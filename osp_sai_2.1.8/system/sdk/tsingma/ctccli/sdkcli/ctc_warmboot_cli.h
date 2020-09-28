/**
 @file ctc_warmboot_cli.h

 @date 2016-6-15

 @version v1.0

---file comments----
*/

#ifndef _CTC_WARMBOOT_CLI_H
#define _CTC_WARMBOOT_CLI_H
#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk ipuc module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/

#ifdef __cplusplus
extern "C" {
#endif

extern int32
ctc_warmboot_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_WARMBOOT_CLI_H */

