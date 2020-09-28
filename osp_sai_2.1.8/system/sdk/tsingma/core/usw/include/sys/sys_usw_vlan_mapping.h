/**
 @file sys_usw_vlan_mapping.h

 @date 2009-12-11

 @version v2.0


*/

#ifndef _SYS_USW_VLAN_MAPPING_H
#define _SYS_USW_VLAN_MAPPING_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_vlan.h"
/**************************************************************
*
* Macro and Defines
*
***************************************************************/

#define VLAN_RANGE_ENTRY_NUM            64
typedef struct
{
    uint16  vrange[VLAN_RANGE_ENTRY_NUM * CTC_BOTH_DIRECTION];
    int32   igs_step ;
    int32   egs_step ;
    sal_mutex_t* mutex;
    uint16   vrange_mem_use_count[VLAN_RANGE_ENTRY_NUM * CTC_BOTH_DIRECTION][8];
}sys_vlan_mapping_master_t;

/**************************************************************
*
* Function
*
***************************************************************/
extern int32
sys_usw_vlan_mapping_init(uint8 lchip);
extern int32
sys_usw_vlan_mapping_deinit(uint8 lchip);
extern int32
sys_usw_vlan_add_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping);
extern int32
sys_usw_vlan_update_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping);
extern int32
sys_usw_vlan_get_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping);
extern int32
sys_usw_vlan_remove_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping);
extern int32
sys_usw_vlan_add_default_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_miss_t* p_action);
extern int32
sys_usw_vlan_remove_default_vlan_mapping(uint8 lchip, uint32 gport);
extern int32
sys_usw_vlan_remove_all_vlan_mapping_by_port(uint8 lchip, uint32 gport);
extern int32
sys_usw_vlan_add_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_egress_vlan_mapping_t* p_vlan_mapping);
extern int32
sys_usw_vlan_remove_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_egress_vlan_mapping_t* p_vlan_mapping);
extern int32
sys_usw_vlan_get_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_egress_vlan_mapping_t*  p_vlan_mapping);
extern int32
sys_usw_vlan_add_default_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_miss_t* p_action);
extern int32
sys_usw_vlan_remove_default_egress_vlan_mapping(uint8 lchip, uint32 gport);
extern int32
sys_usw_vlan_remove_all_egress_vlan_mapping_by_port(uint8 lchip, uint32 gport);
extern int32
sys_usw_vlan_get_vlan_range_type(uint8 lchip, ctc_vlan_range_info_t* vrange_info, bool* is_svlan);
extern int32
sys_usw_vlan_create_vlan_range(uint8 lchip, ctc_vlan_range_info_t* vrange_info, bool is_svlan);
extern int32
sys_usw_vlan_destroy_vlan_range(uint8 lchip, ctc_vlan_range_info_t* vrange_info);
extern int32
sys_usw_vlan_add_vlan_range_member(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_t* vlan_range);
extern int32
sys_usw_vlan_remove_vlan_range_member(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_t* vlan_range);
extern int32
sys_usw_vlan_get_vlan_range_member(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_group_t* vrange_group, uint8* count);
extern int32
sys_usw_vlan_get_maxvid_from_vrange(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_t* vlan_range, bool is_svlan, uint16* p_max_vid);
extern int32
sys_usw_vlan_mapping_get_master(uint8 lchip, sys_vlan_mapping_master_t** p_vlan_mapping_master);

#ifdef __cplusplus
}
#endif

#endif

