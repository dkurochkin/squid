/*
 * -----------------------------------------------------------------------------
 *
 * Author: Markus Moeller (markus_moeller at compuserve.com)
 *
 * Copyright (C) 2007 Markus Moeller. All rights reserved.
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
 *   As a special exemption, M Moeller gives permission to link this program
 *   with MIT, Heimdal or other GSS/Kerberos libraries, and distribute
 *   the resulting executable, without including the source code for
 *   the Libraries in the source distribution.
 *
 * -----------------------------------------------------------------------------
 */
/*
 * Hosted at http://sourceforge.net/projects/squidkerbauth
 */
#include "config.h"
#include "helpers/defines.h"
#include "util.h"

#ifdef HAVE_LDAP

#include "support.h"
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

void
init_args(struct main_args *margs)
{
    margs->nlist = NULL;
    margs->glist = NULL;
    margs->llist = NULL;
    margs->ulist = NULL;
    margs->tlist = NULL;
    margs->luser = NULL;
    margs->lpass = NULL;
    margs->lbind = NULL;
    margs->lurl = NULL;
    margs->ssl = NULL;
    margs->rc_allow = 0;
    margs->AD = 0;
    margs->mdepth = 5;
    margs->ddomain = NULL;
    margs->groups = NULL;
    margs->ndoms = NULL;
    margs->lservs = NULL;
}

void clean_gd(struct gdstruct *gdsp);
void clean_nd(struct ndstruct *ndsp);
void clean_ls(struct ndstruct *lssp);

void
clean_gd(struct gdstruct *gdsp)
{
    struct gdstruct *p = NULL, *pp = NULL;

    p = gdsp;
    while (p) {
        while (p->next) {
            pp = p;
            p = p->next;
        }
        if (p->group) {
            xfree(p->group);
            p->group = NULL;
        }
        if (p->domain) {
            xfree(p->domain);
            p->domain = NULL;
        }
        if (pp && pp->next) {
            xfree(pp->next);
            pp->next = NULL;
        }
        if (p == gdsp) {
            xfree(gdsp);
            gdsp = NULL;
        }
        p = gdsp;
    }
}

void
clean_nd(struct ndstruct *ndsp)
{
    struct ndstruct *p = NULL, *pp = NULL;

    p = ndsp;
    while (p) {
        while (p->next) {
            pp = p;
            p = p->next;
        }
        if (p->netbios) {
            xfree(p->netbios);
            p->netbios = NULL;
        }
        if (p->domain) {
            xfree(p->domain);
            p->domain = NULL;
        }
        if (pp && pp->next) {
            xfree(pp->next);
            pp->next = NULL;
        }
        if (p == ndsp) {
            xfree(ndsp);
            ndsp = NULL;
        }
        p = ndsp;
    }
}

void
clean_ls(struct lsstruct *lssp)
{
    struct lsstruct *p = NULL, *pp = NULL;

    p = lssp;
    while (p) {
        while (p->next) {
            pp = p;
            p = p->next;
        }
        if (p->lserver) {
            xfree(p->lserver);
            p->lserver = NULL;
        }
        if (p->domain) {
            xfree(p->domain);
            p->domain = NULL;
        }
        if (pp && pp->next) {
            xfree(pp->next);
            pp->next = NULL;
        }
        if (p == lssp) {
            xfree(lssp);
            lssp = NULL;
        }
        p = lssp;
    }
}

void
clean_args(struct main_args *margs)
{
    if (margs->glist) {
        xfree(margs->glist);
        margs->glist = NULL;
    }
    if (margs->ulist) {
        xfree(margs->ulist);
        margs->ulist = NULL;
    }
    if (margs->tlist) {
        xfree(margs->tlist);
        margs->tlist = NULL;
    }
    if (margs->nlist) {
        xfree(margs->nlist);
        margs->nlist = NULL;
    }
    if (margs->llist) {
        xfree(margs->llist);
        margs->llist = NULL;
    }
    if (margs->luser) {
        xfree(margs->luser);
        margs->luser = NULL;
    }
    if (margs->lpass) {
        xfree(margs->lpass);
        margs->lpass = NULL;
    }
    if (margs->lbind) {
        xfree(margs->lbind);
        margs->lbind = NULL;
    }
    if (margs->lurl) {
        xfree(margs->lurl);
        margs->lurl = NULL;
    }
    if (margs->ssl) {
        xfree(margs->ssl);
        margs->ssl = NULL;
    }
    if (margs->ddomain) {
        xfree(margs->ddomain);
        margs->ddomain = NULL;
    }
    if (margs->groups) {
        clean_gd(margs->groups);
        margs->groups = NULL;
    }
    if (margs->ndoms) {
        clean_nd(margs->ndoms);
        margs->ndoms = NULL;
    }
    if (margs->lservs) {
        clean_ls(margs->lservs);
        margs->lservs = NULL;
    }
}

void strup(char *s);

int
main(int argc, char *const argv[])
{
    char buf[6400];
    char *user, *domain;
    char *nuser, *nuser8 = NULL, *netbios;
    char *c;
    int opt;
    struct main_args margs;

    setbuf(stdout, NULL);
    setbuf(stdin, NULL);

    init_args(&margs);

    while (-1 != (opt = getopt(argc, argv, "diasg:D:N:S:u:U:t:T:p:l:b:m:h"))) {
        switch (opt) {
        case 'd':
            debug_enabled = 1;
            break;
        case 'i':
            log_enabled = 1;
            break;
        case 'a':
            margs.rc_allow = 1;
            break;
        case 's':
            margs.ssl = (char *) "yes";
            break;
        case 'g':
            margs.glist = xstrdup(optarg);
            break;
        case 'D':
            margs.ddomain = xstrdup(optarg);
            break;
        case 'N':
            margs.nlist = xstrdup(optarg);
            break;
        case 'u':
            margs.luser = xstrdup(optarg);
            break;
        case 'U':
            margs.ulist = xstrdup(optarg);
            break;
        case 't':
            margs.ulist = xstrdup(optarg);
            break;
        case 'T':
            margs.tlist = xstrdup(optarg);
            break;
        case 'p':
            margs.lpass = xstrdup(optarg);
            /* Hide Password */
            memset(optarg, 'X', strlen(optarg));
            break;
        case 'l':
            margs.lurl = xstrdup(optarg);
            break;
        case 'b':
            margs.lbind = xstrdup(optarg);
            break;
        case 'm':
            margs.mdepth = atoi(optarg);
            break;
        case 'S':
            margs.llist = xstrdup(optarg);
            break;
        case 'h':
            fprintf(stderr, "Usage: \n");
            fprintf(stderr, "squid_kerb_ldap [-d] [-i] -g group list [-D domain] [-N netbios domain map] [-s] [-u ldap user] [-p ldap user password] [-l ldap url] [-b ldap bind path] [-a] [-m max depth] [-h]\n");
            fprintf(stderr, "-d full debug\n");
            fprintf(stderr, "-i informational messages\n");
            fprintf(stderr, "-g group list\n");
            fprintf(stderr, "-t group list (only group name hex UTF-8 format)\n");
            fprintf(stderr, "-T group list (all in hex UTF-8 format - except seperator @)\n");
            fprintf(stderr, "-D default domain\n");
            fprintf(stderr, "-N netbios to dns domain map\n");
            fprintf(stderr, "-S ldap server to dns domain map\n");
            fprintf(stderr, "-u ldap user\n");
            fprintf(stderr, "-p ldap user password\n");
            fprintf(stderr, "-l ldap url\n");
            fprintf(stderr, "-b ldap bind path\n");
            fprintf(stderr, "-s use SSL encryption with Kerberos authentication\n");
            fprintf(stderr, "-a allow SSL without cert verification\n");
            fprintf(stderr, "-m maximal depth for recursive searches\n");
            fprintf(stderr, "-h help\n");
            fprintf(stderr, "The ldap url, ldap user and ldap user password details are only used if the kerberised\n");
            fprintf(stderr, "access fails(e.g. unknown domain) or if the username does not contain a domain part\n");
            fprintf(stderr, "and no default domain is provided.\n");
            fprintf(stderr, "If the ldap url starts with ldaps:// it is either start_tls or simple SSL\n");
            fprintf(stderr, "The group list can be:\n");
            fprintf(stderr, "group   - In this case group can be used for all keberised and non kerberised ldap servers\n");
            fprintf(stderr, "group@  - In this case group can be used for all keberised ldap servers\n");
            fprintf(stderr, "group@domain  - In this case group can be used for ldap servers of domain domain\n");
            fprintf(stderr, "group1@domain1:group2@domain2:group3@:group4  - A list is build with a colon as seperator\n");
            fprintf(stderr, "Group membership is determined with AD servers through the users memberof attribute which\n");
            fprintf(stderr, "is followed to the top (e.g. if the group is a member of a group)\n");
            fprintf(stderr, "Group membership is determined with non AD servers through the users memberuid (assuming\n");
            fprintf(stderr, "PosixGroup) or primary group membership (assuming PosixAccount)\n");
            fprintf(stderr, "The ldap server list can be:\n");
            fprintf(stderr, "server - In this case server can be used for all Kerberos domains\n");
            fprintf(stderr, "server@  - In this case server can be used for all Kerberos domains\n");
            fprintf(stderr, "server@domain  - In this case server can be used for Kerberos domain domain\n");
            fprintf(stderr, "server1a@domain1:server1b@domain1:server2@domain2:server3@:server4 - A list is build with a colon as seperator\n");
            clean_args(&margs);
            exit(0);
        default:
            warn((char *) "%s| %s: WARNING: unknown option: -%c.\n", LogTime(), PROGRAM, opt);
        }
    }

    debug((char *) "%s| %s: INFO: Starting version %s\n", LogTime(), PROGRAM, KERBEROS_LDAP_GROUP_VERSION);
    if (create_gd(&margs)) {
        debug((char *) "%s| %s: FATAL: Error in group list: %s\n", LogTime(), PROGRAM, margs.glist ? margs.glist : "NULL");
        SEND_ERR("");
        clean_args(&margs);
        exit(1);
    }
    if (create_nd(&margs)) {
        debug((char *) "%s| %s: FATAL: Error in netbios list: %s\n", LogTime(), PROGRAM, margs.nlist ? margs.nlist : "NULL");
        SEND_ERR("");
        clean_args(&margs);
        exit(1);
    }
    if (create_ls(&margs)) {
        debug((char *) "%s| %s: Error in ldap server list: %s\n", LogTime(), PROGRAM, margs.llist ? margs.llist : "NULL");
        SEND_ERR("");
        clean_args(&margs);
        exit(1);
    }
    while (1) {
        if (fgets(buf, sizeof(buf) - 1, stdin) == NULL) {
            if (ferror(stdin)) {
                debug((char *) "%s| %s: FATAL: fgets() failed! dying..... errno=%d (%s)\n", LogTime(), PROGRAM, ferror(stdin),
                      strerror(ferror(stdin)));

                SEND_ERR("");
                clean_args(&margs);
                exit(1);	/* BIIG buffer */
            }
            SEND_ERR("");
            clean_args(&margs);
            exit(0);
        }
        c = (char *) memchr(buf, '\n', sizeof(buf) - 1);
        if (c) {
            *c = '\0';
        } else {
            SEND_ERR("");
            debug((char *) "%s| %s: ERR\n", LogTime(), PROGRAM);
            continue;
        }

        user = buf;
        nuser = strchr(user, '\\');
        if (!nuser)
            nuser8 = strstr(user, "%5C");
        if (!nuser && !nuser8)
            nuser8 = strstr(user, "%5c");
        domain = strrchr(user, '@');
        if (nuser || nuser8) {
            if (nuser) {
                *nuser = '\0';
                nuser++;
            } else {
                *nuser8 = '\0';
                nuser = nuser8 + 3;
            }
            netbios = user;
            if (debug_enabled)
                debug((char *) "%s| %s: INFO: Got User: %s Netbios Name: %s\n", LogTime(), PROGRAM, nuser, netbios);
            else
                log((char *) "%s| %s: INFO: Got User: %s Netbios Name: %s\n", LogTime(), PROGRAM, nuser, netbios);
            domain = get_netbios_name(&margs, netbios);
            user = nuser;
        } else if (domain) {
            strup(domain);
            *domain = '\0';
            domain++;
        }
        if (!domain && margs.ddomain) {
            domain = xstrdup(margs.ddomain);
            if (debug_enabled)
                debug((char *) "%s| %s: INFO: Got User: %s set default domain: %s\n", LogTime(), PROGRAM, user, domain);
            else
                log((char *) "%s| %s: INFO: Got User: %s set default domain: %s\n", LogTime(), PROGRAM, user, domain);
        }
        if (debug_enabled)
            debug((char *) "%s| %s: INFO: Got User: %s Domain: %s\n", LogTime(), PROGRAM, user, domain ? domain : "NULL");
        else
            log((char *) "%s| %s: INFO: Got User: %s Domain: %s\n", LogTime(), PROGRAM, user, domain ? domain : "NULL");

        if (!strcmp(user, "QQ") && domain && !strcmp(domain, "QQ")) {
            clean_args(&margs);
            exit(-1);
        }
        if (check_memberof(&margs, user, domain)) {
            SEND_OK("");
            debug((char *) "%s| %s: DEBUG: OK\n", LogTime(), PROGRAM);
        } else {
            SEND_ERR("");
            debug((char *) "%s| %s: DEBUG: ERR\n", LogTime(), PROGRAM);
        }
    }


}

void
strup(char *s)
{
    while (*s) {
        *s = toupper((unsigned char) *s);
        s++;
    }
}

#else
#include <stdio.h>
#include <stdlib.h>
int
main(int argc, char *const argv[])
{
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);
    char buf[6400];
    while (1) {
        if (fgets(buf, sizeof(buf) - 1, stdin) == NULL) {
        }
        fprintf(stdout, "ERR\n");
        fprintf(stderr, "LDAP group authorisation not supported\n");
    }
}
#endif
