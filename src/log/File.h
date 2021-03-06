#ifndef SQUID_SRC_LOG_FILE_H
#define SQUID_SRC_LOG_FILE_H

#include "dlink.h"

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

class logfile_buffer_t
{
public:
    char *buf;
    int size;
    int len;
    int written_len;
    dlink_node node;
};

class Logfile;

typedef void LOGLINESTART(Logfile *);
typedef void LOGWRITE(Logfile *, const char *, size_t len);
typedef void LOGLINEEND(Logfile *);
typedef void LOGFLUSH(Logfile *);
typedef void LOGROTATE(Logfile *);
typedef void LOGCLOSE(Logfile *);

class Logfile
{

public:
    char path[MAXPATHLEN];

    struct {
        unsigned int fatal;
    } flags;

    int64_t sequence_number;  ///< Unique sequence number per log line.

public:
    void *data;

    LOGLINESTART *f_linestart;
    LOGWRITE *f_linewrite;
    LOGLINEEND *f_lineend;
    LOGFLUSH *f_flush;
    LOGROTATE *f_rotate;
    LOGCLOSE *f_close;
};

/* Legacy API */
extern Logfile *logfileOpen(const char *path, size_t bufsz, int);
extern void logfileClose(Logfile * lf);
extern void logfileRotate(Logfile * lf);
extern void logfileWrite(Logfile * lf, char *buf, size_t len);
extern void logfileFlush(Logfile * lf);
extern void logfilePrintf(Logfile * lf, const char *fmt,...) PRINTF_FORMAT_ARG2;
extern void logfileLineStart(Logfile * lf);
extern void logfileLineEnd(Logfile * lf);

#endif /* SQUID_SRC_LOG_FILE_H */
