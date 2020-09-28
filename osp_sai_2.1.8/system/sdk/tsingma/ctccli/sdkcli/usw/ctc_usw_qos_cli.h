/**
 @file ctc_usw_qos_cli.h

 @date 2009-12-22

 @version v2.0

*/

#ifndef _CTC_USW_QOS_CLI_H_
#define _CTC_USW_QOS_CLI_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_cli.h"

#define SET_FLAG(V, F)        (V) = (V) | (F)

extern int32
ctc_usw_qos_cli_init(void);

extern int32
ctc_usw_qos_cli_deinit(void);

#ifdef __cplusplus
}
#endif

#endif

