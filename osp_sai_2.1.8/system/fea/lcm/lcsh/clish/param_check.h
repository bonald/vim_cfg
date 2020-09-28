/****************************************************************************
 * param_check.h: common functions to check parameter
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     : R0.01
 * Author       : Alexander Liu
 * Date         : 2006-11-27
 * Reason       : First Create.
 ****************************************************************************/

#include <stdint.h>

/* key name */
#define M_MAX_KEYNAME_LEN       32
/* for bug 28065 modified by liuyang 2014-4-10*/
#define M_KEYNAME_LIMIT_STR     "starting with alphabetic and can only include [0-9a-zA-Z.-_]. Its length range is [1, 32)."
#ifdef _CENTEC_
#define M_COMMUNITY_LIMIT_STR     "starting with alphanumeric and can only include [0-9a-zA-Z.-_]. Its length range is [1, 256)."
#endif
/* username */
#define M_MAX_USERNAME_LEN      32
/* for bug 28065 modified by liuyang 2014-4-10*/
#define M_USERNAME_LIMIT_STR    "can only include [0-9a-zA-Z.-_], and must starting with alphabetic. Its length range is [1, 32)."
#define M_MAX_USER_COUNTER 256

/* password length limit */
#define M_MAX_PASSWORD_LEN      64
#define M_PASSWORD_BUFSIZE      256
#define M_PASSWORD_LIMIT_STR    "can only include printable characters and its length range is [1, 64)."
/* password process flags */
#define E_PASSWORD_ENCRYPT      0x1
#define E_PASSWORD_DECRYPT      0x0
/* full path name length */
#define M_FULLPATH_MAX_LEN      256
#define M_FULLPATH_DEPTH_MAX    32
/*VPS name*/
#define M_MAX_VPSNAME_LEN M_MAX_KEYNAME_LEN
#define M_MAX_VPS_COUNTER 16
#define M_MAX_MD5_PASSWORD_LEN      (M_MAX_PASSWORD_LEN*3)

/* validation filename */
int check_filename(const char *pszFilename);

/* process tokens */
int split_tokens(char *pszBuf, size_t nLen, unsigned long nMaxToks, 
                 char *pszDelimiters, unsigned long *pnToks, char ***pppTokArr);
int free_tokens(char ***pppTokArr);
int signal_app(const char *pszPidFile, int signal);
int check_image_file(char *filename, char *errmsg, size_t size, int bCRC);

