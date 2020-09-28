/**
 @file ctc_parser_cli.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-12-28

 @version v2.0

This file contains parser module cli init info
*/

#ifndef _CTC_PARSER_CLI_H
#define _CTC_PARSER_CLI_H
#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk Mirror module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/

#ifdef __cplusplus
extern "C" {
#endif

int32
ctc_parser_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif

