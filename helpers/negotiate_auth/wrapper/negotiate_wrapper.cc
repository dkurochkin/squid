/*
 * -----------------------------------------------------------------------------
 *
 * Author: Markus Moeller (markus_moeller at compuserve.com)
 *
 * Copyright (C) 2011 Markus Moeller. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * -----------------------------------------------------------------------------
 */
/*
 * Hosted at http://sourceforge.net/projects/squidkerbauth
 */

#include "config.h"
#include "base64.h"

#if HAVE_STRING_H
#include <string.h>
#endif
#if HAVE_STDIO_H
#include <stdio.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_TIME_H
#include <time.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_ERRNO_H
#include <errno.h>
#endif

#if !defined(HAVE_DECL_XMALLOC) || !HAVE_DECL_XMALLOC
#define xmalloc malloc
#endif
#if !defined(HAVE_DECL_XSTRDUP) || !HAVE_DECL_XSTRDUP
#define xstrdup strdup
#endif
#if !defined(HAVE_DECL_XFREE) || !HAVE_DECL_XFREE
#define xfree free
#endif

#undef PROGRAM
#define PROGRAM "negotiate_wrapper"
#undef VERSION
#define VERSION "1.0.1"

#ifndef MAX_AUTHTOKEN_LEN
#define MAX_AUTHTOKEN_LEN   65535
#endif

static const unsigned char ntlmProtocol[] = {'N', 'T', 'L', 'M', 'S', 'S', 'P', 0};

static const char *
LogTime()
{
    struct timeval now;
    static time_t last_t = 0;
    static char buf[128];

    gettimeofday(&now, NULL);
    if (now.tv_sec != last_t) {
        time_t *tmp = (time_t *) & now.tv_sec;
        struct tm *tm = localtime(tmp);
        strftime(buf, 127, "%Y/%m/%d %H:%M:%S", tm);
        last_t = now.tv_sec;
    }
    return buf;
}

void usage(void)
{
    fprintf(stderr, "Usage: \n");
    fprintf(stderr, "negotiate_wrapper [-h] [-d] --ntlm ntlm helper + arguments --kerberos kerberos helper + arguments\n");
    fprintf(stderr, "-h help\n");
    fprintf(stderr, "-d full debug\n");
    fprintf(stderr, "--ntlm full ntlm helper path with arguments\n");
    fprintf(stderr, "--kerberos full kerberos helper path with arguments\n");
}

int
main(int argc, char *const argv[])
{
    char buf[MAX_AUTHTOKEN_LEN];
    char tbuff[MAX_AUTHTOKEN_LEN];
    char buff[MAX_AUTHTOKEN_LEN+2];
    char *c;
    int debug = 0;
    int length;
    int nstart = 0, kstart = 0;
    int nend = 0, kend = 0;
    char *token;
    char **nargs, **kargs;
    int i,j;
    int fpid;
    FILE *FDKIN,*FDKOUT;
    FILE *FDNIN,*FDNOUT;
    int pkin[2];
    int pkout[2];
    int pnin[2];
    int pnout[2];

    setbuf(stdout, NULL);
    setbuf(stdin, NULL);

    if (argc ==1 || !strncasecmp(argv[1],"-h",2)) {
        usage();
        return 0;
    }

    j = 1;
    if (!strncasecmp(argv[1],"-d",2)) {
        debug = 1;
        j = 2;
    }

    for (i=j; i<argc; i++) {
        if (!strncasecmp(argv[i],"--ntlm",6))
            nstart = i;
        if (!strncasecmp(argv[i],"--kerberos",10))
            kstart = i;
    }
    if (nstart > kstart) {
        kend = nstart-1;
        nend = argc-1;
    } else {
        kend = argc-1;
        nend = kstart-1;
    }
    if (nstart == 0 || kstart == 0 || kend-kstart <= 0 || nend-nstart <= 0 ) {
        usage();
        return 0;
    }

    if (debug)
        fprintf(stderr, "%s| %s: Starting version %s\n", LogTime(), PROGRAM,
                VERSION);

    if ((nargs = (char **)xmalloc((nend-nstart+1)*sizeof(char *))) == NULL) {
        fprintf(stderr, "%s| %s: Error allocating memory for ntlm helper\n", LogTime(), PROGRAM);
        return 1;
    }
    memcpy(nargs,argv+nstart+1,(nend-nstart)*sizeof(char *));
    nargs[nend-nstart]=NULL;
    if (debug) {
        fprintf(stderr, "%s| %s: NTLM command: ", LogTime(), PROGRAM);
        for (i=0; i<nend-nstart; i++)
            fprintf(stderr, "%s ", nargs[i]);
        fprintf(stderr, "\n");
    }
    if ((kargs = (char **)xmalloc((kend-kstart+1)*sizeof(char *))) == NULL) {
        fprintf(stderr, "%s| %s: Error allocating memory for kerberos helper\n", LogTime(), PROGRAM);
        return 1;
    }
    memcpy(kargs,argv+kstart+1,(kend-kstart)*sizeof(char *));
    kargs[kend-kstart]=NULL;
    if (debug) {
        fprintf(stderr, "%s| %s: Kerberos command: ", LogTime(), PROGRAM);
        for (i=0; i<kend-kstart; i++)
            fprintf(stderr, "%s ", kargs[i]);
        fprintf(stderr, "\n");
    }
    /*
       Fork Kerberos helper and NTLM helper and manage IO to send NTLM requests
       to the right helper. squid must keep session state
    */

    if (pipe(pkin) < 0) {
        fprintf(stderr, "%s| %s: Could not assign streams for pkin\n", LogTime(), PROGRAM);
        return 1;
    }
    if (pipe(pkout) < 0) {
        fprintf(stderr, "%s| %s: Could not assign streams for pkout\n", LogTime(), PROGRAM);
        return 1;
    }


    if  (( fpid = vfork()) < 0 ) {
        fprintf(stderr, "%s| %s: Failed first fork\n", LogTime(), PROGRAM);
        return 1;
    }

    if ( fpid == 0 ) {
        /* First Child for Kerberos helper */

        close(pkin[1]);
        dup2(pkin[0],STDIN_FILENO);
        close(pkin[0]);

        close(pkout[0]);
        dup2(pkout[1],STDOUT_FILENO);
        close(pkout[1]);

        setbuf(stdin, NULL);
        setbuf(stdout, NULL);

        execv(kargs[0], kargs);
        fprintf(stderr, "%s| %s: Failed execv for %s: %s\n", LogTime(), PROGRAM, kargs[0], strerror(errno));
        return 1;

    }

    close(pkin[0]);
    close(pkout[1]);

    if (pipe(pnin) < 0) {
        fprintf(stderr, "%s| %s: Could not assign streams for pnin\n", LogTime(), PROGRAM);
        return 1;
    }
    if (pipe(pnout) < 0) {
        fprintf(stderr, "%s| %s: Could not assign streams for pnout\n", LogTime(), PROGRAM);
        return 1;
    }

    if  (( fpid = vfork()) < 0 ) {
        fprintf(stderr, "%s| %s: Failed second fork\n", LogTime(), PROGRAM);
        return 1;
    }

    if ( fpid == 0 ) {
        /* Second Child for NTLM helper */

        close(pnin[1]);
        dup2(pnin[0],STDIN_FILENO);
        close(pnin[0]);

        close(pnout[0]);
        dup2(pnout[1],STDOUT_FILENO);
        close(pnout[1]);

        setbuf(stdin, NULL);
        setbuf(stdout, NULL);

        execv(nargs[0], nargs);
        fprintf(stderr, "%s| %s: Failed execv for %s: %s\n", LogTime(), PROGRAM, nargs[0], strerror(errno));
        return 1;
    }

    close(pnin[0]);
    close(pnout[1]);

    FDKIN=fdopen(pkin[1],"w");
    FDKOUT=fdopen(pkout[0],"r");

    FDNIN=fdopen(pnin[1],"w");
    FDNOUT=fdopen(pnout[0],"r");

    if (!FDKIN || !FDKOUT || !FDNIN || !FDNOUT) {
        fprintf(stderr, "%s| %s: Could not assign streams for FDKIN/FDKOUT/FDNIN/FDNOUT\n", LogTime(), PROGRAM);
        return 1;
    }

    setbuf(FDKIN, NULL);
    setbuf(FDKOUT, NULL);
    setbuf(FDNIN, NULL);
    setbuf(FDNOUT, NULL);


    while (1) {
        if (fgets(buf, sizeof(buf) - 1, stdin) == NULL) {
            if (ferror(stdin)) {
                if (debug)
                    fprintf(stderr,
                            "%s| %s: fgets() failed! dying..... errno=%d (%s)\n",
                            LogTime(), PROGRAM, ferror(stdin),
                            strerror(ferror(stdin)));

                fprintf(stdout, "BH input error\n");
                return 1;        /* BIIG buffer */
            }
            fprintf(stdout, "BH input error\n");
            return 0;
        }
        c = static_cast<char*>(memchr(buf, '\n', sizeof(buf) - 1));
        if (c) {
            *c = '\0';
            length = c - buf;
            if (debug)
                fprintf(stderr, "%s| %s: Got '%s' from squid (length: %d).\n",
                        LogTime(), PROGRAM, buf, length);
        } else {
            if (debug)
                fprintf(stderr, "%s| %s: Oversized message\n", LogTime(),
                        PROGRAM);
            fprintf(stdout, "BH Oversized message\n");
            continue;
        }

        if (buf[0] == '\0') {
            if (debug)
                fprintf(stderr, "%s| %s: Invalid request\n", LogTime(),
                        PROGRAM);
            fprintf(stdout, "BH Invalid request\n");
            continue;
        }
        if (strlen(buf) < 2) {
            if (debug)
                fprintf(stderr, "%s| %s: Invalid request [%s]\n", LogTime(),
                        PROGRAM, buf);
            fprintf(stdout, "BH Invalid request\n");
            continue;
        }
        if (!strncmp(buf, "QQ", 2)) {
            fprintf(stdout, "BH quit command\n");
            return 0;
        }
        if (strncmp(buf, "YR", 2) && strncmp(buf, "KK", 2)) {
            if (debug)
                fprintf(stderr, "%s| %s: Invalid request [%s]\n", LogTime(),
                        PROGRAM, buf);
            fprintf(stdout, "BH Invalid request\n");
            continue;
        }
        if (strlen(buf) <= 3) {
            if (debug)
                fprintf(stderr, "%s| %s: Invalid negotiate request [%s]\n",
                        LogTime(), PROGRAM, buf);
            fprintf(stdout, "BH Invalid negotiate request\n");
            continue;
        }
        length = base64_decode_len(buf+3);
        if (debug)
            fprintf(stderr, "%s| %s: Decode '%s' (decoded length: %d).\n",
                    LogTime(), PROGRAM, buf + 3, (int) length);

        if ((token = (char *)xmalloc(length)) == NULL) {
            fprintf(stderr, "%s| %s: Error allocating memory for token\n", LogTime(), PROGRAM);
            return 1;
        }

        length = base64_decode(token, length, buf+3);

        if ((static_cast<size_t>(length) >= sizeof(ntlmProtocol) + 1) &&
                (!memcmp(token, ntlmProtocol, sizeof ntlmProtocol))) {
            free(token);
            if (debug)
                fprintf(stderr, "%s| %s: received type %d NTLM token\n",
                        LogTime(), PROGRAM, (int) *((unsigned char *) token +
                                                    sizeof ntlmProtocol));
            fprintf(FDNIN, "%s\n",buf);
            if (fgets(tbuff, sizeof(tbuff) - 1, FDNOUT) == NULL) {
                if (ferror(FDNOUT)) {
                    fprintf(stderr,
                            "fgets() failed! dying..... errno=%d (%s)\n",
                            ferror(FDNOUT), strerror(ferror(FDNOUT)));
                    return 1;
                }
                fprintf(stderr, "%s| %s: Error reading NTLM helper response\n",
                        LogTime(), PROGRAM);
                return 0;
            }
            /*
                   Need to translate NTLM reply to Negotiate reply
                   AF user => AF blob user
               NA reason => NA blob reason
               Set blob to '='
                */
            if (strlen(tbuff) >= 3 && (!strncmp(tbuff,"AF ",3) || !strncmp(tbuff,"NA ",3))) {
                strncpy(buff,tbuff,3);
                buff[3]='=';
                for (unsigned int i=2; i<=strlen(tbuff); i++)
                    buff[i+2] = tbuff[i];
            } else {
                strcpy(buff,tbuff);
            }
        } else {
            free(token);
            if (debug)
                fprintf(stderr, "%s| %s: received Kerberos token\n",
                        LogTime(), PROGRAM);

            fprintf(FDKIN, "%s\n",buf);
            if (fgets(buff, sizeof(buff) - 1, FDKOUT) == NULL) {
                if (ferror(FDKOUT)) {
                    fprintf(stderr,
                            "fgets() failed! dying..... errno=%d (%s)\n",
                            ferror(FDKOUT), strerror(ferror(FDKOUT)));
                    return 1;
                }
                fprintf(stderr, "%s| %s: Error reading Kerberos helper response\n",
                        LogTime(), PROGRAM);
                return 0;
            }
        }
        fprintf(stdout,"%s",buff);
        if (debug)
            fprintf(stderr, "%s| %s: Return '%s'\n",
                    LogTime(), PROGRAM, buff);
    }

    return 1;
}
