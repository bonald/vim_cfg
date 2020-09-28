#ifndef __SAL_FILE_H__
#define __SAL_FILE_H__

/**
 * @file sal_file.h
 */

/**
 * @defgroup file Files
 * @{
 */


#if defined(_SAL_LINUX_KM)

#include <linux/fs.h>

/** File Object */
typedef struct sal_file* sal_file_t;
typedef mode_t sal_file_mode_t;
#define SAL_FILE_NULL NULL

#elif defined(_SAL_LINUX_UM)

/** File Object */
typedef FILE* sal_file_t;
typedef int sal_file_mode_t;
#define SAL_FILE_NULL   -1
#elif defined(_SAL_VXWORKS)

#include <stdio.h>
/** File Object */
typedef FILE* sal_file_t;
typedef int sal_file_mode_t;
#define SAL_FILE_NULL   -1

#endif

#ifdef __cplusplus
extern "C"
{
#endif


#if defined(_SAL_LINUX_KM)

/**
 * Open file
 *
 * @param[in]  pathname
 * @param[in]  flags
 * @param[in]  mode
 *
 * @return     sal_file_t
 */
sal_file_t ctc_sal_open(const char *pathname, int flags, sal_file_mode_t mode);

sal_file_t ctc_sal_fopen(const char *file, const char *mode);
/**
 * Close file
 *
 * @param[in] fp
 * @return    int
 */
int ctc_sal_fclose(sal_file_t ft);

/**
 * Read file
 *
 * @param[in]  ft
 * @param[out] buf
 * @param[in]  count
 * @return    ssize_t
 */
ssize_t ctc_sal_read(sal_file_t ft, void *buf, size_t count);

/**
 * Write file
 *
 * @param[in] ft
 * @param[in] buf
 * @param[in] count
 * @return    ssize_t
 */
ssize_t ctc_sal_write(sal_file_t ft, void *buf, size_t count);

/**
 * Get a string from a file
 *
 * @param[in] buf
 * @param[in] size
 * @param[in] ft
 * @return    char
 */
char *ctc_sal_fgets(char *buf, int size, sal_file_t ft);

ssize_t ctc_sal_eof(sal_file_t ft);

int ctc_sal_fseek(sal_file_t ft, long offset, int fromwhere);

long ctc_sal_ftell(sal_file_t ft);


#endif

#ifdef __cplusplus
}
#endif

/**@}*/ /* End of @defgroup task */

#endif /* !__SAL_FILE_H__ */

