#include "sal.h"

#ifdef _SAL_DEBUG
void sal_assert_failed(const char *expr, const char *file, int line)
{
    sal_log(SAL_LL_FATAL, file, line, "Assertion failed: %s", expr);
    abort();
}
#endif

