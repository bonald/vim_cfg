/**
 @file sys_usw_mirror.h

 @date 2009-10-21

 @version v2.0

*/

#ifndef _SYS_USW_MIRROR_H
#define _SYS_USW_MIRROR_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"

#include "ctc_const.h"
#include "ctc_mirror.h"
#include "ctc_debug.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

enum sys_mirror_discard_e
{
    SYS_MIRROR_L2SPAN_DISCARD = 0x1,                 /**<mirror port session drop packet*/
    SYS_MIRROR_L3SPAN_DISCARD = 0x2,                 /**<mirror vlan session drop packet*/
    SYS_MIRROR_INGRESS_ACLLOG_PRI_DISCARD = 0x3FC,    /**<mirror ingress acl priority 0 session drop packet*/
    SYS_MIRROR_EGRESS_ACLLOG_PRI_DISCARD = 0x1C,    /**<mirror egress acl priority 0 session drop packet*/
    SYS_MIRROR_INGRESS_IPFIX_MIRROR_DISCARD = 0x800,   /**<mirror ingress ipfix mirror drop packet*/
    SYS_MIRROR_EGRESS_IPFIX_MIRROR_DISCARD = 0x40   /**<mirror egress ipfix mirror drop packet*/
};
typedef enum sys_mirror_discard_e sys_mirror_discard_t;

#define SYS_MIRROR_DISCARD_CHECK(flag) \
    if (0 == (flag & (CTC_MIRROR_IPFIX_DISCARD | \
                      CTC_MIRROR_ACLLOG_PRI_DISCARD | \
                      CTC_MIRROR_L3SPAN_DISCARD | CTC_MIRROR_L2SPAN_DISCARD))) \
        return CTC_E_INVALID_PARAM

#define SYS_MIRROR_DBG_OUT(level, FMT, ...)                          \
    do                                                     \
    {                                                      \
        CTC_DEBUG_OUT(mirror, mirror, MIRROR_SYS, level, FMT, ##__VA_ARGS__);  \
    } while (0)


/****************************************************************************
*
* Function
*
*****************************************************************************/
extern int32
sys_usw_mirror_init(uint8 lchip);

extern int32
sys_usw_mirror_set_port_en(uint8 lchip, uint32 gport, ctc_direction_t dir, bool enable, uint8 session_id);

extern int32
sys_usw_mirror_get_port_info(uint8 lchip, uint32 gport, ctc_direction_t dir, bool* enable, uint8* session_id);

extern int32
sys_usw_mirror_set_vlan_en(uint8 lchip, uint16 vlan_id, ctc_direction_t dir, bool enable, uint8 session_id);

extern int32
sys_usw_mirror_get_vlan_info(uint8 lchip, uint16 vlan_id, ctc_direction_t dir, bool* enable, uint8* session_id);

extern int32
sys_usw_mirror_set_dest(uint8 lchip, ctc_mirror_dest_t* mirror);

extern int32
sys_usw_mirror_rspan_escape_en(uint8 lchip, bool enable);

extern int32
sys_usw_mirror_rspan_escape_mac(uint8 lchip, ctc_mirror_rspan_escape_t* escape);

extern int32
sys_usw_mirror_unset_dest(uint8 lchip, ctc_mirror_dest_t* mirror);

extern int32
sys_usw_mirror_set_mirror_discard(uint8 lchip, ctc_direction_t dir, uint16 discard_flag, bool enable);

extern int32
sys_usw_mirror_get_mirror_discard(uint8 lchip, ctc_direction_t dir, ctc_mirror_discard_t discard_flag, bool* enable);

extern int32
sys_usw_mirror_set_erspan_psc(uint8 lchip, ctc_mirror_erspan_psc_t* psc);

extern int32
sys_usw_mirror_show_info(uint8 lchip, ctc_mirror_info_type_t type, uint32 value);

extern int32
sys_usw_mirror_deinit(uint8 lchip);
#ifdef __cplusplus
}
#endif

#endif

