/**
 @file ctc_sync_ether_cli.h

 @date 2012-10-18

 @version v2.0

 This file declares extern function for SyncE cli module

*/

#ifndef _CTC_SYNC_ETHER_CLI_H
#define _CTC_SYNC_ETHER_CLI_H
#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk SynE module CLIs
*/

#ifdef __cplusplus
extern "C" {
#endif

extern int32
ctc_sync_ether_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_SYNC_ETHER_CLI_H */

