#if (FEATURE_MODE == 0)
/**
 @file ctc_dot1ae_cli.h

 @author  Copyright (C) 2017 Centec Networks Inc.  All rights reserved.

 @date 2017-08-22

 @version v2.0

 This file contains dot1ae cli
*/
#ifndef _CTC_CLI_DOT1AE_H
#define _CTC_CLI_DOT1AE_H

#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk dot1ae module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/

#ifdef __cplusplus
extern "C" {
#endif

extern int32
ctc_dot1ae_cli_init(void);


#ifdef __cplusplus
}
#endif

#endif

#endif

