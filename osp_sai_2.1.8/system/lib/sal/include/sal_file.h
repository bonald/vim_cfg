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
sal_file_t sal_open(const char *pathname, int flags, sal_file_mode_t mode);

sal_file_t sal_fopen(const char *file, const char *mode);
/**
 * Close file
 *
 * @param[in] fp
 * @return    int
 */
int sal_fclose(sal_file_t ft);

/**
 * Read file
 *
 * @param[in]  ft
 * @param[out] buf
 * @param[in]  count
 * @return    ssize_t
 */
ssize_t sal_read(sal_file_t ft, void *buf, size_t count);

/**
 * Write file
 *
 * @param[in] ft
 * @param[in] buf
 * @param[in] count
 * @return    ssize_t
 */
ssize_t sal_write(sal_file_t ft, void *buf, size_t count);

/**
 * Get a string from a file
 *
 * @param[in] buf
 * @param[in] size
 * @param[in] ft
 * @return    char
 */
char *sal_fgets(char *buf, int size, sal_file_t ft);

int sal_fseek(sal_file_t ft, long offset, int fromwhere);

long sal_ftell(sal_file_t ft);

#else

/* file operation */
#ifndef sal_open
#define sal_open open
#endif

#ifndef sal_close
#define sal_close close
#endif

#ifndef sal_fopen
#define sal_fopen fopen
#endif

#ifndef sal_fclose
#define sal_fclose fclose
#endif

#ifndef sal_read
#define sal_read read
#endif

#ifndef sal_write
#define sal_write write
#endif

#ifndef sal_fread
#define sal_fread fread
#endif

#ifndef sal_fwrite
#define sal_fwrite fwrite
#endif

#ifndef sal_fprintf
#define sal_fprintf fprintf
#endif

#ifndef sal_fgets
#define sal_fgets fgets
#endif

#ifndef sal_fseek
#define sal_fseek fseek
#endif

#ifndef sal_ftell
#define sal_ftell ftell
#endif

#ifndef sal_feof
#define sal_feof feof
#endif

#endif

#ifdef __cplusplus
}
#endif

/**@}*/ /* End of @defgroup task */

#endif /* !__SAL_FILE_H__ */

