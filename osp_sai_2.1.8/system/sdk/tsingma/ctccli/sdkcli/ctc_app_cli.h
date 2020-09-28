/**
 @file ctc_wlan_cli.h

 @author  Copyright (C) 2017 Centec Networks Inc.  All rights reserved.

 @date 2017-06-20

 @version v2.0

 This file contains sdk app cli
*/
#ifndef _CTC_CLI_APP_H
#define _CTC_CLI_APP_H

#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk app module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/

#ifdef __cplusplus
extern "C" {
#endif

extern int32
ctc_app_api_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif

