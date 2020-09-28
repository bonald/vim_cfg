/**
 @file ctc_pdu_cli.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-1-11

 @version v2.0

This file contains pdu module cli info
*/

#ifndef _CTC_PDU_CLI_H
#define _CTC_PDU_CLI_H
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
ctc_pdu_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif

