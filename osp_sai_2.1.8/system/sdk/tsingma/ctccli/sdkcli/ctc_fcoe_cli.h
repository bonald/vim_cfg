/**
 @file ctc_fcoe_cli.h

 @date 2015-10-21

 @version v2.0

---file comments----
*/

#ifndef _CTC_FCOE_CLI_H
#define _CTC_FCOE_CLI_H
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"

/**
 @brief  Initialize sdk fcoe module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/

#ifdef __cplusplus
extern "C" {
#endif

extern int32
ctc_fcoe_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_FCOE_CLI_H */

