/**
 @file sys_usw_aps.h

 @date 2010-3-10

 @version v2.0

*/

#ifndef _SYS_USW_APS_H
#define _SYS_USW_APS_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_aps.h"
#include "ctc_const.h"
#include "ctc_vector.h"
#include "ctc_linklist.h"
#include "ctc_debug.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

#define CTC_APS_GROUP_BLOCK 16

#define SYS_APS_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(aps, aps, APS_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_APS_DUMP(FMT, ...)                    \
    do                                                       \
    {                                                        \
        CTC_DEBUG_OUT(aps, aps, APS_SYS, CTC_DEBUG_LEVEL_DUMP, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_APS_BRIDGE_DUMP(group_id, p_bridge_info) \
    do { \
        SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_bridge_info :: \n" \
                        "     aps_bridge_id = %d, protect_en = %d, physical_isolated=%d,\n" \
                        "     working_gport =%d, protection_gport =%d, w_l3if_id=%d, p_l3if_id=%d,\n" \
                        "     next_w_aps_en=%d, next_p_aps_en=%d, next_w_aps=%d, next_p_aps=%d, \n" \
                        "     raps_en=%d, raps_group_id=%d\n", \
                        group_id, p_bridge_info->protect_en, p_bridge_info->physical_isolated, \
                        p_bridge_info->working_gport, p_bridge_info->protection_gport, p_bridge_info->w_l3if_id, p_bridge_info->p_l3if_id, \
                        p_bridge_info->next_w_aps_en, p_bridge_info->next_p_aps_en, \
                        p_bridge_info->next_aps.w_group_id, p_bridge_info->next_aps.p_group_id, \
                        p_bridge_info->raps_en, p_bridge_info->raps_group_id); \
    } while (0)

#define SYS_APS_GROUP_ID_CHECK(aps_group_id)  \
    { \
        uint32 entry_num = 0;  \
        sys_usw_ftm_query_table_entry_num(0, DsApsBridge_t, &entry_num); \
        if (aps_group_id >= entry_num){ \
            return CTC_E_INVALID_PARAM; } \
    }
#ifndef PACKET_TX_USE_SPINLOCK
#define APS_LOCK(lchip) \
    do { \
        if (p_usw_aps_master[lchip]->p_aps_mutex){ \
            sal_mutex_lock(p_usw_aps_master[lchip]->p_aps_mutex); } \
    } while (0)

#define APS_UNLOCK(lchip) \
   do { \
        if (p_usw_aps_master[lchip]->p_aps_mutex){ \
            sal_mutex_unlock(p_usw_aps_master[lchip]->p_aps_mutex); } \
    } while (0)
#else
#define APS_LOCK(lchip) \
    do { \
        if (p_usw_aps_master[lchip]->p_aps_mutex){ \
            sal_spinlock_lock((sal_spinlock_t*)p_usw_aps_master[lchip]->p_aps_mutex); } \
    } while (0)

#define APS_UNLOCK(lchip) \
   do { \
        if (p_usw_aps_master[lchip]->p_aps_mutex){ \
            sal_spinlock_unlock((sal_spinlock_t*)p_usw_aps_master[lchip]->p_aps_mutex); } \
    } while (0)
#endif

typedef struct
{
  uint32  mpls_tunnel :1;
  uint32  working_path :1;
  uint32  next_aps_en:1;
  uint32  destmap_profile:1;
  uint32  edit_ptr_valid :1;
  uint32  spme_en :1;
  uint32  l3if_id:16;
  uint32  different_nh:1;
  uint32  edit_ptr_type:2;
  uint32  edit_ptr_location:1;
  uint32  rsv:6;

  uint32  edit_ptr;
  uint32  dest_id;

  uint32  nh_id;  /*if mpls_tunnel set,mpls tunnel id,else nh_id */

}sys_aps_bind_nh_param_t;

/****************************************************************
*
* Functions
*
****************************************************************/
extern int32
sys_usw_aps_init(uint8 lchip);
extern int32
sys_usw_aps_deinit(uint8 lchip);
extern int32
sys_usw_aps_create_bridge_group(uint8 lchip, uint16 group_id, ctc_aps_bridge_group_t* aps_group);
extern int32
sys_usw_aps_update_bridge_group(uint8 lchip, uint16 group_id, ctc_aps_bridge_group_t* aps_group);
extern int32
sys_usw_aps_remove_bridge_group(uint8 lchip, uint16 group_id);
extern int32
sys_usw_aps_set_bridge_path(uint8 lchip, uint16 group_id, uint32 gport, bool is_working);
extern int32
sys_usw_aps_get_bridge_path(uint8 lchip, uint16 group_id, uint32* gport, bool is_working);
extern int32
sys_usw_aps_set_protection(uint8 lchip, uint16 group_id, bool protect_en);
extern int32
sys_usw_aps_get_protection(uint8 lchip, uint16 group_id, bool* protect_en);

extern int32
sys_usw_aps_create_raps_mcast_group(uint8 lchip, uint16 group_id);
extern int32
sys_usw_aps_remove_raps_mcast_group(uint8 lchip, uint16 group_id);
extern int32
sys_usw_aps_update_raps_mcast_member(uint8 lchip, ctc_raps_member_t* p_raps_mem);

extern int32
sys_usw_aps_set_share_nh(uint8 lchip, uint16 group_id, bool share_nh);

extern int32
sys_usw_aps_get_share_nh(uint8 lchip, uint16 group_id, bool* share_nh);

extern int32
sys_usw_aps_get_ports(uint8 lchip, uint16 group_id, ctc_aps_bridge_group_t* aps_group);



extern int32
sys_usw_aps_get_next_group(uint8 lchip, uint16 group_id, uint16* next_group_id, bool is_working);
#if 0
extern int32
sys_usw_aps_get_current_working_port(uint8 lchip, uint16 group_id, uint32* gport);
#endif
extern int32
sys_usw_aps_bind_nexthop(uint8 lchip,uint16 group_id,sys_aps_bind_nh_param_t *p_bind_nh);

extern int32
sys_usw_aps_unbind_nexthop(uint8 lchip,uint16 group_id,sys_aps_bind_nh_param_t *p_bind_nh);

extern int32
sys_usw_aps_deinit(uint8 lchip);

#ifdef __cplusplus
}
#endif

#endif

