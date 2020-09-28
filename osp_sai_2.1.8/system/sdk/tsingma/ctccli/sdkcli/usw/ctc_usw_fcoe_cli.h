#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_ipfix_cli.h

 @date 2015-10-22

 @version v2.0

 This file declares extern function for fcoe cli module

*/

#ifndef _CTC_USW_FCOE_CLI_H
#define _CTC_USW_FCOE_CLI_H
#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk fcoe module CLIs
*/
extern int32
ctc_usw_fcoe_cli_init(void);

extern int32
ctc_usw_fcoe_cli_deinit(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_USW_CLI_H */

#endif

