/****************************************************************************
* $Id$
*  file related operations
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2014-11-20 17:47
* Reason        : First Create.
****************************************************************************/

#ifndef _FNCONVERT_H_
#define _FNCONVERT_H_

#define SKIP_WHITE_SPACE(P) \
  do { \
      if (NULL != P) \
       while (*P != '\0' && isspace ((int) * P)) \
         P++; \
   } while (0)

typedef enum {
    E_FNC_CMD_NONE,
    /* convert to show name, eg: /mnt/flash/xx --> flash:/xx */
    E_FNC_CMD_TO_SHOWNAME,
    /* convert to access name, eg: flash:/xx --> /mnt/flash/xx */
    E_FNC_CMD_TO_ACCESSNAME,

    E_FNC_CMD_FILE_PROC_CONFIRM,    /* file overwrite/delete confirm */

    E_FNC_CMD_ISBINFILE,    /* is binary file check */

    E_FNC_CMD_YESNO_NO,     /* confirm yes or no, default no */
    E_FNC_CMD_CONFIRM,      /* confirm yes or no, default yes */

    E_FNC_CMD_CONVERT_RSA_KEY,      /* convert rsa key */
    E_FNC_CMD_IMPORT_RSA_KEY,       /* import rsa key */
    E_FNC_CMD_URL,  /* send/recv file by URL */
    E_FNC_CMD_FILE_COPY, /* copy file with progress */
    E_FNC_CMD_B64_PTON, /* b64 decoding */
    E_FNC_CMD_B64_NTOP, /* b64 encoding */
    E_FNC_CMD_LOGIN, /* user/pin check */
    E_FNC_CMD_SSH, /* ssh client */
    E_FNC_CMD_ENCRYPT, /* encrypt password */
	E_FNC_CMD_DECRYPT  /* djd: add for decrypt password */
} fnc_cmd_e;


int gen_file_proc_confirm(char *action, char *file, char *origfile);
extern int do_filename_convert(fnc_cmd_e cmd, char *pszPath);

#endif /* !_FNCONVERT_H_ */
