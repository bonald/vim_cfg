#if (FEATURE_MODE == 0)
/**
 @file ctc_wlan_cli.h

 @author  Copyright (C) 2016 Centec Networks Inc.  All rights reserved.

 @date 2016-02-22

 @version v2.0

 This file contains wlan cli
*/
#ifndef _CTC_CLI_WLAN_H
#define _CTC_CLI_WLAN_H

#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk wlan module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/

#ifdef __cplusplus
extern "C" {
#endif

extern int32
ctc_wlan_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif

#endif

