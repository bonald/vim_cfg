/**
 @file ctc_greatbel_l2_cli.h

 @date 2011-11-25

 @version v2.0

---file comments----
*/

#ifndef _CTC_USW_L2_CLI_H
#define _CTC_USW_L2_CLI_H
#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk L2 module CLIs, only for show function

 @param[in/out]

 @return CTC_E_XXX

*/
extern int32
ctc_usw_l2_cli_init(void);

extern int32
ctc_usw_l2_cli_deinit(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_USW_L2_CLI_H */

