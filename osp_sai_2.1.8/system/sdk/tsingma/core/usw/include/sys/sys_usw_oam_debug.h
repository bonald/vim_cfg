#if (FEATURE_MODE == 0)
/**
 @file sys_usw_oam_debug.h

 @date 2010-1-19

 @version v2.0

  The file defines Macro, stored data structure for  Ethernet OAM module
*/
#ifndef _SYS_USW_OAM_DEBUG_H
#define _SYS_USW_OAM_DEBUG_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************************
 *
 * Header Files
 *
****************************************************************************/

#include "ctc_debug.h"

/****************************************************************************
*
* Defines and Macros
*
****************************************************************************/

#define SYS_OAM_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(oam, oam, OAM_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0);

#define SYS_OAM_DBG_DUMP(FMT, ...) SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, FMT, ##__VA_ARGS__)
/****************************************************************************
*
* Defines and Macros
*
****************************************************************************/
#define SYS_OAM_DBG_ETH_OAM         1
#define SYS_OAM_DBG_TP_Y1731_OAM    2
#define SYS_OAM_DBG_BFD             3
#define SYS_OAM_DBG_ALL             4

/****************************************************************************
*
* Global and Declaration
*
*****************************************************************************/

/****************************************************************************
 *
* Function
*
****************************************************************************/
extern int32
sys_usw_oam_show_oam_property(uint8 lchip, ctc_oam_property_t* p_prop);
extern int32
sys_usw_oam_internal_property(uint8 lchip);

extern int32
sys_usw_oam_internal_maid_property(uint8 lchip, ctc_oam_maid_len_format_t maid_len);

extern int32
sys_usw_oam_tp_section_init(uint8 lchip, uint8 use_port);

extern int32
sys_usw_isr_oam_process_isr(uint8 gchip, void* p_data);
extern int32
sys_usw_oam_show_oam_status(uint8 lchip);
extern int32
sys_usw_oam_show_oam_defect_status(uint8 lchip);
extern int32
sys_usw_oam_show_oam_mep_detail(uint8 lchip, ctc_oam_key_t *p_oam_key);
extern int32
sys_usw_oam_show_oam_mep(uint8 lchip, uint8 oam_type);
extern int32
sys_usw_oam_show_oam_cpu_reseaon(uint8 lchip);

#ifdef __cplusplus
}
#endif

#endif

#endif
