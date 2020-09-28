/**
 @file ctc_l2_cli.h

 @date 2009-10-30

 @version v2.0

---file comments----
*/

#ifndef _CTC_L2_CLI_H
#define _CTC_L2_CLI_H
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"

/**
 @brief  Initialize sdk L2 module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/

#ifdef __cplusplus
extern "C" {
#endif

int32
ctc_l2_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_L2_CLI_H */

