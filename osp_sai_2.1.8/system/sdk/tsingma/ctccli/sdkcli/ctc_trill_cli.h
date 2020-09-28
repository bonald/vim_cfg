/**
 @file ctc_trill_cli.h

 @date 2013-10-25

 @version v3.0

---file comments----
*/

#ifndef _CTC_TRILL_CLI_H
#define _CTC_TRILL_CLI_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk trill module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/
int32
ctc_trill_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif

