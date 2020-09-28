/**
 @file ctc_usw_stats_cli.h

 @date 2010-06-09

 @version v2.0

 This file declares extern function for stats internal cli module

*/

#ifndef _CTC_USW_STATS_CLI_H
#define _CTC_USW_STATS_CLI_H
#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk stats internal module CLIs
*/
extern int32
ctc_usw_stats_cli_init(void);

extern int32
ctc_usw_stats_cli_deinit(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_USW_STATS_CLI_H */

