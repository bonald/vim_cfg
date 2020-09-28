/**
 @file ctc_mpls_cli.h

 @date 2010-03-16

 @version v2.0

---file comments----
*/

#ifndef _CTC_USW_APS_CLI_H
#define _CTC_USW_APS_CLI_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk aps module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/
extern int32
ctc_usw_aps_cli_init(void);

extern int32
ctc_usw_aps_cli_deinit(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_USW_APS_CLI_H */

