#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_trill_cli.h

 @date 2015-10-26

 @version v3.0

*/

#ifndef _CTC_USW_TRILL_CLI_H
#define _CTC_USW_TRILL_CLI_H
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
extern int32
ctc_usw_trill_cli_init(void);

extern int32
ctc_usw_trill_cli_deinit(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_USW_TRILL_CLI_H */

#endif

