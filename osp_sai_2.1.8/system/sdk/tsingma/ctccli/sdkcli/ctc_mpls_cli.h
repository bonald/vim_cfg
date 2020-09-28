/**
 @file ctc_mpls_cli.h

 @date 2010-03-16

 @version v2.0

---file comments----
*/

#ifndef _CTC_MPLS_CLI_H
#define _CTC_MPLS_CLI_H
#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk mpls module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/

#ifdef __cplusplus
extern "C" {
#endif

int32
ctc_mpls_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_MPLS_CLI_H */

