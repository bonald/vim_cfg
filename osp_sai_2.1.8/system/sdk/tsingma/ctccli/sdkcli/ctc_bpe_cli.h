/**
 @file ctc_bpe_cli.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-11-14

 @version v2.0

 This file contains bpe cli
*/
#ifndef _CTC_CLI_BPE_H
#define _CTC_CLI_BPE_H

#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk bpe module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/

#ifdef __cplusplus
extern "C" {
#endif

extern int32
ctc_bpe_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif
