/**
 @file ctc_learning_aging_cli.h

 @date 2010-3-1

 @version v2.0

*/

#ifndef _CTC_LEARNING_AGING_CLI_H
#define _CTC_LEARNING_AGING_CLI_H
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
ctc_learning_aging_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif

