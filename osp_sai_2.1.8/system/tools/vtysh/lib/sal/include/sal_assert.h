#ifndef __SAL_ASSERT_H__
#define __SAL_ASSERT_H__

/**
 * @file sal_assert.h
 */

/**
 * @defgroup assert Assertion Macros
 * @{
 */

/**
 * Abort the program if assertion failed
 *
 * @param[in] e
 */
#ifdef _SAL_DEBUG
#define SAL_ASSERT(e) if (!(e)) sal_assert_failed(#e, __FILE__, __LINE__)
#else
#define SAL_ASSERT(e) (void)0
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _SAL_DEBUG
void sal_assert_failed(const char *expr, const char *file, int line);
#endif

#ifdef __cplusplus
}
#endif

/**@}*/ /* End of @defgroup assert */

#endif /* !__SAL_ASSERT_H__ */

