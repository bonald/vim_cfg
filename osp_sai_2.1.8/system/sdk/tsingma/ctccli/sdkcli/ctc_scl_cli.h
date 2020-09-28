/**
 @file ctc_scl_cli.h

 @date 2009-12-22

 @version v2.0

*/

#ifndef _CTC_SCL_CLI_H_
#define _CTC_SCL_CLI_H_

#include "sal.h"
#include "ctc_cli.h"

#define CHECK_FLAG(V, F)      ((V)&(F))
#define SET_FLAG(V, F)        (V) = (V) | (F)
#define UNSET_FLAG(V, F)      (V) = (V)&~(F)

#define SCL_IGMP_TYPE_HOST_QUERY                 0x11
#define SCL_IGMP_TYPE_HOST_REPORT                0x12
#define SCL_IGMP_TYPE_HOST_DVMRP                 0x13
#define SCL_IGMP_TYPE_PIM                        0x14
#define SCL_IGMP_TYPE_TRACE                      0x15
#define SCL_IGMP_TYPE_V2_REPORT                  0x16
#define SCL_IGMP_TYPE_V2_LEAVE                   0x17
#define SCL_IGMP_TYPE_MTRACE                     0x1f
#define SCL_IGMP_TYPE_MTRACE_RESPONSE            0x1e
#define SCL_IGMP_TYPE_PRECEDENCE                 0          /*temp, maybe change later*/
#define SCL_IGMP_TYPE_V3_REPORT                  0x22

#ifdef __cplusplus
extern "C" {
#endif

extern int32
ctc_scl_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif

