/**
 @file sys_usw_qos_api.h

 @date 2010-01-13

 @version v2.0

 The file defines queue api
*/

#ifndef _SYS_USW_QOS_API_H
#define _SYS_USW_QOS_API_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_const.h"
#include "ctc_parser.h"
#include "ctc_qos.h"

/*********************************************************************
*
* Data Structure
*
*********************************************************************/

#define SYS_MAX_QUEUE_OFFSET_PER_NETWORK_CHANNEL  12
#define SYS_MAX_QUEUE_OFFSET_PER_EXT_GROUP  4

enum sys_qos_policer_type_e
{
    SYS_QOS_POLICER_TYPE_PORT,
    SYS_QOS_POLICER_TYPE_VLAN,
    SYS_QOS_POLICER_TYPE_FLOW,
    SYS_QOS_POLICER_TYPE_MACRO_FLOW,
    SYS_QOS_POLICER_TYPE_SERVICE,
    SYS_QOS_POLICER_TYPE_COPP,
    SYS_QOS_POLICER_TYPE_MFP
};
typedef enum sys_qos_policer_type_e  sys_qos_policer_type_t;

struct sys_qos_policer_param_s
{
    uint8  level;
    uint8  is_bwp;
    uint16 policer_ptr;
    uint8  triple_play;
    uint8  dir;
    uint16 rsv;
};
typedef struct sys_qos_policer_param_s sys_qos_policer_param_t;

struct sys_qos_shape_profile_s
{
    uint8  chan_shp_tokenThrd;
    uint8  chan_shp_tokenThrdShift;
    uint8  ext_grp_shp_profileId;
    uint8  rsv;
    uint8  queue_shp_profileId[SYS_MAX_QUEUE_OFFSET_PER_NETWORK_CHANNEL];
    uint8  ext_queue_shp_profileId[SYS_MAX_QUEUE_OFFSET_PER_EXT_GROUP];
};
typedef struct sys_qos_shape_profile_s sys_qos_shape_profile_t;

struct sys_cpu_reason_info_s
{
    uint16 reason_id;
    uint8  fatal_excp_valid;
    uint16 fatal_excp_index;
    uint8  exception_index;
    uint8  exception_subIndex;
    uint16 gid_for_acl_key;
    uint8  gid_valid;
    uint8  acl_key_valid;
};
typedef struct sys_cpu_reason_info_s sys_cpu_reason_info_t;

/************************* qos sys api *************************/
extern int32
sys_usw_qos_policer_index_get(uint8 lchip, uint16 policer_id, uint8 sys_policer_type,
                                                sys_qos_policer_param_t* p_policer_param);
extern int32
sys_usw_qos_policer_map_token_rate_user_to_hw(uint8 lchip, uint8 is_pps,
                                             uint32 user_rate, /*kb/s*/
                                             uint32 *hw_rate,
                                             uint16 bit_with,
                                             uint32 gran,
                                             uint32 upd_freq);
extern int32
sys_usw_qos_policer_map_token_rate_hw_to_user(uint8 lchip, uint8 is_pps,
                                                    uint32 hw_rate, uint32 *user_rate, uint32 upd_freq);
extern int32
sys_usw_qos_policer_mfp_alloc(uint8 lchip, uint32 rate, uint32 threshold, uint8* p_profile_id);
extern int32
sys_usw_qos_policer_mfp_free(uint8 lchip, uint8 profile_id);
extern int32
sys_usw_queue_get_enq_mode(uint8 lchip, uint8 *enq_mode);
extern int32
sys_usw_get_channel_by_port(uint8 lchip, uint32 gport, uint8 *channel);
extern int32
sys_usw_add_port_to_channel(uint8 lchip, uint16 lport, uint8 channel);
extern int32
sys_usw_remove_port_from_channel(uint8 lchip, uint16 lport, uint8 channel);
extern int32
sys_usw_qos_add_extend_port_to_channel(uint8 lchip, uint16 lport, uint8 channel);
extern int32
sys_usw_qos_remove_extend_port_to_channel(uint8 lchip, uint16 lport, uint8 channel);
extern int32
sys_usw_qos_add_mcast_queue_to_channel(uint8 lchip, uint16 lport, uint8 channel);
extern int32
sys_usw_qos_remove_mcast_queue_to_channel(uint8 lchip, uint16 lport, uint8 channel);
extern int32
sys_usw_qos_map_token_thrd_user_to_hw(uint8 lchip, uint32  user_bucket_thrd,
                                            uint32 *hw_bucket_thrd,
                                            uint8 shift_bits,
                                            uint32 max_thrd);
extern int32
sys_usw_queue_set_port_drop_en(uint8 lchip, uint32 gport, bool enable, sys_qos_shape_profile_t* p_shp_profile);
extern int32
sys_usw_queue_get_profile_from_hw(uint8 lchip, uint32 gport, sys_qos_shape_profile_t* p_shp_profile);
extern int32
sys_usw_set_dma_channel_drop_en(uint8 lchip, bool enable);
extern int32
sys_usw_queue_get_port_depth(uint8 lchip, uint32 gport, uint32* p_depth);
extern int32
sys_usw_get_sub_queue_id_by_cpu_reason(uint8 lchip, uint16 reason_id, uint8* sub_queue_id);
extern int32
sys_usw_cpu_reason_alloc_exception_index(uint8 lchip, uint8 dir, sys_cpu_reason_info_t* p_cpu_rason_info);
extern int32
sys_usw_cpu_reason_free_exception_index(uint8 lchip, uint8 dir, sys_cpu_reason_info_t* p_cpu_rason_info);
extern int32
sys_usw_cpu_reason_get_info(uint8 lchip, uint16 reason_id, uint32 *destmap);
extern int32
sys_usw_cpu_reason_get_reason_info(uint8 lchip, uint8 dir, sys_cpu_reason_info_t* p_cpu_rason_info);
extern int32
sys_usw_cpu_reason_alloc_dsfwd_offset(uint8 lchip, uint16 reason_id,
                                           uint32 *dsfwd_offset,
                                           uint32 *dsnh_offset,
                                           uint32 *dest_port);
extern int32
sys_usw_queue_add_for_stacking(uint8 lchip, uint32 gport);
extern int32
sys_usw_queue_remove_for_stacking(uint8 lchip, uint32 gport);
extern int32
sys_usw_qos_policer_ingress_vlan_get(uint8 lchip, uint16* ingress_vlan_policer_num);
extern int32
sys_usw_qos_policer_egress_vlan_get(uint8 lchip, uint16* egress_vlan_policer_num);
extern int32
sys_usw_qos_bind_service_logic_dstport(uint8 lchip, uint16 service_id, uint16 logic_dst_port);
extern int32
sys_usw_qos_unbind_service_logic_dstport(uint8 lchip, uint16 service_id, uint16 logic_dst_port);
extern int32
sys_usw_queue_get_service_queue_mode(uint8 lchip, uint8 *enq_mode);
extern int32
sys_usw_qos_bind_service_logic_srcport(uint8 lchip, uint16 service_id, uint16 logic_port);
extern int32
sys_usw_qos_unbind_service_logic_srcport(uint8 lchip, uint16 service_id, uint16 logic_port);

/************************* qos show api *************************/
extern int32
sys_usw_qos_domain_map_dump(uint8 lchip, uint8 domain, uint8 type);
extern int32
sys_usw_qos_table_map_dump(uint8 lchip, uint8 table_map_id, uint8 type);
extern int32
sys_usw_qos_policer_dump(uint8 type, uint8 dir, uint8 lchip, uint16 start, uint16 end, uint8 detail);
extern int32
sys_usw_qos_queue_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail);
extern int32
sys_usw_qos_group_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail);
extern int32
sys_usw_qos_port_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail);
extern int32
sys_usw_qos_cpu_reason_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail);
extern int32
sys_usw_qos_dump_status(uint8 lchip);

/************************* qos ctc api *************************/
/*policer*/
extern int32
sys_usw_qos_set_policer(uint8 lchip, ctc_qos_policer_t* p_policer);
extern int32
sys_usw_qos_get_policer(uint8 lchip, ctc_qos_policer_t* p_policer);
extern int32
sys_usw_qos_set_global_config(uint8 lchip, ctc_qos_glb_cfg_t* p_glb_cfg);
extern int32
sys_usw_qos_get_global_config(uint8 lchip, ctc_qos_glb_cfg_t* p_glb_cfg);
/*mapping*/
extern int32
sys_usw_qos_set_domain_map(uint8 lchip, ctc_qos_domain_map_t* p_domain_map);
extern int32
sys_usw_qos_get_domain_map(uint8 lchip, ctc_qos_domain_map_t* p_domain_map);
/*queue*/
extern int32
sys_usw_qos_set_queue(uint8 lchip, ctc_qos_queue_cfg_t* p_que_cfg);
/*shape*/
extern int32
sys_usw_qos_set_shape(uint8 lchip, ctc_qos_shape_t* p_shape);
extern int32
sys_usw_qos_get_shape(uint8 lchip, ctc_qos_shape_t* p_shape);
/*schedule*/
extern int32
sys_usw_qos_set_sched(uint8 lchip, ctc_qos_sched_t* p_sched);
extern int32
sys_usw_qos_get_sched(uint8 lchip, ctc_qos_sched_t* p_sched);
/*drop*/
extern int32
sys_usw_qos_set_drop_scheme(uint8 lchip, ctc_qos_drop_t* p_drop);
extern int32
sys_usw_qos_get_drop_scheme(uint8 lchip, ctc_qos_drop_t* p_drop);
/*resrc*/
extern int32
sys_usw_qos_set_resrc(uint8 lchip, ctc_qos_resrc_t* p_resrc);
extern int32
sys_usw_qos_get_resrc(uint8 lchip, ctc_qos_resrc_t* p_resrc);
extern int32
sys_usw_qos_query_pool_stats(uint8 lchip, ctc_qos_resrc_pool_stats_t* p_stats);
/*stats*/
extern int32
sys_usw_qos_query_queue_stats(uint8 lchip, ctc_qos_queue_stats_t* p_queue_stats);
extern int32
sys_usw_qos_clear_queue_stats(uint8 lchip, ctc_qos_queue_stats_t* p_queue_stats);
extern int32
sys_usw_qos_query_policer_stats(uint8 lchip, ctc_qos_policer_stats_t* p_policer_stats);
extern int32
sys_usw_qos_clear_policer_stats(uint8 lchip, ctc_qos_policer_stats_t* p_policer_stats);
/*init*/
extern int32
sys_usw_qos_init(uint8 lchip, void* p_gbl_parm);
/*deinit*/
extern int32
sys_usw_qos_deinit(uint8 lchip);

#ifdef __cplusplus
}
#endif

#endif

