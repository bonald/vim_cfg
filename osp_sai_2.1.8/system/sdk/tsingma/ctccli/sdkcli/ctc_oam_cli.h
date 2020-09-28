/**
 @file ctc_oam_cli.h

 @date 2010-03-23

 @version v2.0

 This file declares extern function for oam cli module

*/

#ifndef _CTC_OAM_CLI_H
#define _CTC_OAM_CLI_H
#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk oam module CLIs
*/
extern int32
ctc_oam_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_OAM_CLI_H */

