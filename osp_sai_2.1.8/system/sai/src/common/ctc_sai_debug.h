#ifndef _CTC_SAI_DEBUG_CLI_H_
#define _CTC_SAI_DEBUG_CLI_H_

#ifdef __cplusplus
extern C {
#endif

#include <sal.h>
#include "genlog.h"

extern uint32 g_sai_debug_flag;

#define SAI_DEBUG_FLAG_FUNC         0x01
#define SAI_DEBUG_FLAG_PARAM        0x02
#define SAI_DEBUG_FLAG_PACKET       0x04

#define CTC_SAI_DEBUG_FUNC() \
do { \
    if (g_sai_debug_flag & SAI_DEBUG_FLAG_FUNC) \
        log_sys(M_MOD_SAI, E_DEBUG, "Enter into %s", __FUNCTION__); \
} while (0)

#define CTC_SAI_DEBUG_PACKET(fmt, args...) \
do { \
    if (g_sai_debug_flag & SAI_DEBUG_FLAG_PACKET) \
        log_sys(M_MOD_SAI, E_DEBUG, fmt, ##args); \
} while (0)

#define CTC_SAI_DEBUG(fmt, args...) \
do { \
    if (g_sai_debug_flag & SAI_DEBUG_FLAG_PARAM) \
        log_sys(M_MOD_SAI, E_DEBUG, fmt, ##args); \
} while (0)
    
extern int32 ctc_sai_debug_cli_init(uint8 cli_tree_mode);

#ifdef __cplusplus
}
#endif

#endif
