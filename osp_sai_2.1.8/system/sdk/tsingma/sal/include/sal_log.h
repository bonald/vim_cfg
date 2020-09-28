#ifndef __SAL_LOG_H__
#define __SAL_LOG_H__

/**
 * @file sal_log.h
 */
#include "sal_types.h"
#include "sal_file.h"
/**
 * @defgroup log Message Logging
 * @{
 */

/** Log levels */
typedef enum sal_loglevel
{
    SAL_LL_FATAL,    /**< Fatal log level */
    SAL_LL_ERROR,    /**< Error log level */
    SAL_LL_WARN,     /**< Warning log level */
    SAL_LL_INFO,      /**< Information log level */
    SAL_LL_DEBUG,   /**< Debug log level */
    SAL_LL_OFF        /**< Off log level */
} sal_loglevel_t;

extern sal_loglevel_t debug_level_flag;

/**
 * @brief Log fatal error message
 *
 * The macro prototype is the same as printf()'s.
 * The output has "FATAL <file>(<line>): " prefix and auto-newline.
 */
#define SAL_LOG_FATAL(fmt, args...) \
    ctc_sal_log(SAL_LL_FATAL, __FILE__, __LINE__, fmt, ##args)

/**
 * @brief Log error message
 *
 * The macro prototype is the same as printf()'s.
 * The output has "ERROR <file>(<line>): " prefix and auto-newline.
 */
#define SAL_LOG_ERROR(fmt, args...) \
    ctc_sal_log(SAL_LL_ERROR, __FILE__, __LINE__, fmt, ##args)

/**
 * @brief Log warning message
 *
 * The macro prototype is the same as printf()'s.
 * The output has "WARN  <file>(<line>): " prefix and auto-newline.
 */
#define SAL_LOG_WARN(fmt, args...) \
    ctc_sal_log(SAL_LL_WARN, __FILE__, __LINE__, fmt, ##args)

/**
 * @brief Log informational message
 *
 * The macro prototype is the same as printf()'s.
 * The output has no prefix and auto-newline.
 */
#define SAL_LOG_INFO(fmt, args...) \
    ctc_sal_log(SAL_LL_INFO, __FILE__, __LINE__, fmt, ##args)

/**
 * @brief Log debug message
 *
 * The macro prototype is the same as printf()'s.
 * The output has no prefix and auto-newline.
 */
#ifdef _SAL_DEBUG
#define SAL_LOG_DEBUG(fmt, args...) \
    ctc_sal_log(SAL_LL_DEBUG, __FILE__, __LINE__, fmt, ##args)
#else
#define SAL_LOG_DEBUG(fmt, args...)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define _SAL_LOG_ATTR __attribute__((format(printf, 4, 5)))

/**
 * @brief Internal logging function
 *
 * @param[in] log_level
 * @param[in] file
 * @param[in] line
 * @param[in] fmt
 * @param[in] ...
 *
 * @return
 */


int ctc_sal_log(int log_level,
            const char* file, int32 line, const char* fmt, ...);

int ctc_sal_print_func(const char* fmt,  ...);

#if defined(_SAL_LINUX_KM)
int ctc_sal_fprintf(sal_file_t pf, const char* fmt,  ...);
#endif

#ifdef __cplusplus
}
#endif

/**@}*/ /* End of @defgroup log */

#endif /* !__SAL_LOG_H__ */

