#include "sal.h"

#ifdef _SAL_DEBUG
void sal_assert_failed(const char *expr, const char *file, int line)
{
    panic("%s:%d: Assertion failed: %s", file, line, expr);
}
#endif

