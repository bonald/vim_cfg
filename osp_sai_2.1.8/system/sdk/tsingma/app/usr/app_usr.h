/**
 @file app_usr.h

 @date 2010-06-09

 @version v2.0

 User define code
 */

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#ifndef _APP_USR_H
#define _APP_USR_H
#ifdef __cplusplus
extern "C" {
#endif

#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_app.h"

#define START_UP_CONFIG         "./start_up.cfg"

extern int32
ctc_app_usr_init(void);

#ifdef __cplusplus
}
#endif

#endif

