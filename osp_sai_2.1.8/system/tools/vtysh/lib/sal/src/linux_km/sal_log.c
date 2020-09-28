#include "sal.h"

static const char *ll2str[] =
{
    KERN_CRIT    "FATAL %s(%d): %s\n",
    KERN_ERR     "ERROR %s(%d): %s\n",
    KERN_WARNING "WARN  %s(%d): %s\n",
    KERN_INFO    "INFO  %s(%d): %s\n",
    KERN_DEBUG   "DEBUG %s(%d): %s\n",
};

int sal_log(int log_level,
            const char *file,
            int line,
            const char *fmt,
            ...)
{
    char fmtbuf[0x100];
    va_list ap;

    if (log_level < SAL_LL_FATAL)
        log_level = SAL_LL_FATAL;
    else if (log_level > SAL_LL_DEBUG)
        log_level = SAL_LL_DEBUG;

    if (log_level < SAL_LL_INFO)
    {
        snprintf(fmtbuf, 0x100, ll2str[log_level], file, line, fmt);
        fmtbuf[0xFF] = 0;
    }
    else
    {
        strncpy(fmtbuf, fmt, 0x100);
        fmtbuf[0xFF] = 0;
    }

    va_start(ap, fmt);
    vprintk(fmtbuf, ap);
    va_end(ap);

    return 0;
}

EXPORT_SYMBOL(sal_log);

