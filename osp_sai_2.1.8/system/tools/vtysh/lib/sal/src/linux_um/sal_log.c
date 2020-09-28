#include "sal.h"

static const char *ll2str[] =
{
    "FATAL",
    "ERROR",
    "WARN ",
    "INFO ",
    "DEBUG"
};

int sal_log(int log_level,
            const char *file,
            int line,
            const char *fmt,
            ...)
{
    va_list ap;

    if (log_level < SAL_LL_FATAL)
        log_level = SAL_LL_FATAL;
    else if (log_level > SAL_LL_DEBUG)
        log_level = SAL_LL_DEBUG;

    if (log_level < SAL_LL_INFO)
        printf("%s %s:%d: ", ll2str[log_level], file, line);

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    if (log_level < SAL_LL_INFO)
        printf("\n");

    return 0;
}
