/**
 @file ctc_linkagg_cli.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-11-18

 @version v2.0

 This file contains linkagg cli
 */

#ifndef _CTC_LINKAGG_CLI_H
#define _CTC_LINKAGG_CLI_H

#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk linkagg module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/

#ifdef __cplusplus
extern "C" {
#endif

extern int32
ctc_linkagg_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif

