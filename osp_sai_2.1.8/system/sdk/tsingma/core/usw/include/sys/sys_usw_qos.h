/**
 @file sys_usw_qos.h

 @date 2010-01-13

 @version v2.0

 The file defines queue api
*/

#ifndef _SYS_USW_QOS_H
#define _SYS_USW_QOS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_const.h"
#include "ctc_debug.h"
#include "ctc_vector.h"
#include "ctc_spool.h"
#include "ctc_hash.h"
#include "ctc_stats.h"
#include "ctc_parser.h"
#include "ctc_packet.h"
#include "ctc_qos.h"

#include "sys_usw_qos_api.h"

/*********************************************************************
*
* Data Structure
*
*********************************************************************/
/****************************** queue enq ******************************/
#define SYS_QOS_QUEUE_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(qos, queue, QOS_QUE_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0);

#define SYS_QOS_GET_SHAPE_TOKEN_THRD(is_pps, token_thrd) {\
   if (is_pps)\
   {\
      token_thrd = token_thrd/MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES); /*pps*/ \
   }\
   else\
   {\
      token_thrd = token_thrd/125; /*kb*/ \
   }\
}

#define SYS_MAX_CHANNEL_NUM  84
#define SYS_C2C_QUEUE_NUM 8
#define SYS_LCPU_QUEUE_NUM           ((p_usw_queue_master[lchip]->max_dma_rx_num == 1) ? 64 : 128)
#define SYS_C2C_QUEUE_MODE(lchip)  (0 == p_usw_queue_master[lchip]->enq_mode)
#define SYS_STACKING_PORT_NUM        4
#define SYS_GRP_SHP_CBUCKET_NUM       8
/*at lease one packet's mtu*/
#define SYS_WFQ_DEFAULT_WEIGHT    1
#define SYS_WFQ_DEFAULT_SHIFT      2
#define SYS_SHP_GRAN_RANAGE_NUM   7
#define SYS_SHAPE_MAX_TOKEN_THRD    ((1 << 8) -1)
#define SYS_RESRC_MAX_CONGEST_LEVEL_NUM 4


#define SYS_GROUP_ID(queue_id)  ((queue_id < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC)) ? (queue_id/8) : (MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_GRP_NUM) + ((queue_id-MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT))/MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP))))
#define SYS_EXT_GROUP_ID(queue_id, ext_grp_id)  \
    if(((queue_id) >= MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT))) \
        { (ext_grp_id) = (((queue_id)-MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT))/MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP));}

#define SYS_IS_EXT_GROUP(group_id) ((group_id >= MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_GRP_NUM)) && (group_id <  MCHIP_CAP(SYS_CAP_QOS_GROUP_NUM)))
#define SYS_IS_BASE_GROUP(group_id) (group_id < MCHIP_CAP(SYS_CAP_QOS_BASE_QUEUE_GRP_NUM))

#define SYS_IS_NETWORK_BASE_QUEUE(queue_id) (queue_id < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC))
#define SYS_IS_MISC_CHANNEL_QUEUE(queue_id) ((queue_id >= MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC)) && (queue_id < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP)))
#define SYS_IS_NETWORK_CTL_QUEUE(queue_id) ((queue_id >= MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC)) && (queue_id < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT)))
#define SYS_IS_CPU_QUEUE(queue_id) ((queue_id >= MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP)) && (queue_id < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC)))
#define SYS_IS_EXT_QUEUE(queue_id) ((queue_id >= MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT)) && (queue_id < MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM)))
#define SYS_IS_NETWORK_CHANNEL(channel) (channel < MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
#define SYS_IS_DMA_CHANNEL(channel) ((channel >= MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0)) && (channel <= MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3)))
#define SYS_RESRC_IGS_MAX_PORT_MIN_PROFILE   7

#define SYS_RESRC_PROFILE_QUEUE 0
#define SYS_RESRC_PROFILE_FC 1
#define SYS_RESRC_PROFILE_PFC 2
#define SYS_RESRC_PROFILE_MAX 3

enum sys_extend_que_type_e
{
    SYS_EXTEND_QUE_TYPE_SERVICE,   /**< service queue */
    SYS_EXTEND_QUE_TYPE_BPE,       /**< bpe queue */
    SYS_EXTEND_QUE_TYPE_C2C,       /**< c2c queue */
    SYS_EXTEND_QUE_TYPE_LOGIC_PORT, /**< logic port */
    SYS_EXTEND_QUE_TYPE_MCAST,     /**< mcast queue */
    SYS_EXTEND_QUE_TYPE_MAX
};
typedef enum sys_extend_que_type_e sys_extend_que_type_t;

enum sys_queue_type_e
{
    SYS_QUEUE_TYPE_NORMAL,     /**< normal en queue */
    SYS_QUEUE_TYPE_EXCP,       /**< exception to cpu by DMA*/
    SYS_QUEUE_TYPE_EXTEND,     /**< extend queue */
    SYS_QUEUE_TYPE_MAX
};
typedef enum sys_queue_type_e sys_queue_type_t;

enum sys_shape_type_e
{
    SYS_SHAPE_TYPE_PORT,
    SYS_SHAPE_TYPE_QUEUE,
    SYS_SHAPE_TYPE_MAX
};
typedef enum sys_shape_type_e sys_shape_type_t;

#define SYS_QOS_SHAPE_TYPE_CHAN 0
#define SYS_QOS_SHAPE_TYPE_QUEUE 1
#define SYS_QOS_SHAPE_TYPE_MAX 2

enum sys_queue_grp_shp_cir_type_e
{
    SYS_QUEUE_GRP_SHP_CIR_TYPE_CIR_FAIL,
    SYS_QUEUE_GRP_SHP_CIR_TYPE_CIR_USER,
    SYS_QUEUE_GRP_SHP_CIR_TYPE_MAX
};
typedef enum sys_queue_grp_shp_cir_type_e sys_queue_grp_shp_cir_type_t;

enum sys_queue_spool_type_e
{
    SYS_QUEUE_PROFILE,
    SYS_QUEUE_METER_PROFILE,
    SYS_QUEUE_DROP_WTD_PROFILE,
    SYS_QUEUE_DROP_WRED_PROFILE,
    SYS_QUEUE_GROUP_PROFILE,
    SYS_QUEUE_GROUP_METER_PROFILE,
    SYS_QUEUE_FC_PROFILE,
    SYS_QUEUE_PFC_PROFILE,
    SYS_QUEUE_FC_DROP_PROFILE,
    SYS_QUEUE_PFC_DROP_PROFILE,
    SYS_QUEUE_SPOOL_PORFILE_MAX
};
typedef enum sys_queue_spool_type_e sys_queue_spool_type_t;

struct sys_queue_shp_grp_info_s
{
    uint32   rate;
    uint16   thrd;
    uint8    cir_type;
    uint8    bucket_sel;
};
typedef struct sys_queue_shp_grp_info_s sys_queue_shp_grp_info_t;

struct sys_group_meter_profile_s
{
    uint8  profile_id;
    uint8  rsv[3];
    sys_queue_shp_grp_info_t c_bucket[SYS_GRP_SHP_CBUCKET_NUM];
};
typedef struct sys_group_meter_profile_s sys_group_meter_profile_t;

/**
 @brief En-queue configurations.
*/
struct sys_enqueue_info_s
{
    uint8  queue_num;
    uint8  subq_num;
    uint8  queue_type;
    uint8  ucast;
    uint8  mirror;
    uint16 queue_base;
    uint16 dest_base;
    uint16 dest_num;
    uint8 stacking;
};
typedef struct sys_enqueue_info_s sys_enqueue_info_t;

struct sys_cpu_reason_s
{
    uint8    dest_type;
    uint8    user_define_mode;   /*0-none;1-DsFwd/ misc nexthop;2.alloc excep ID*/
    uint16   excp_id;
    uint32   dsfwd_offset;
    uint16   sub_queue_id;
    uint16   ref_cnt;
    uint32   dest_port;
    uint32   dest_map;
    uint8    dir;
};
typedef struct sys_cpu_reason_s sys_cpu_reason_t;

struct sys_queue_meter_profile_s
{
    uint8   profile_id;
    uint8   rsv;
    uint16  bucket_thrd;  /*[12:5] rate : [4:0] thrd_shift */
    uint32  bucket_rate;  /*[29:8] rate : [7:0] rate_frac */
};
typedef struct sys_queue_meter_profile_s sys_queue_meter_profile_t;

struct sys_queue_shp_profile_s
{
    uint8   profile_id;
    uint8   queue_type;
    uint16  bucket_thrd;  /*[12:5] rate : [4:0] thrd_shift */
    uint32  bucket_rate;  /*[29:8] rate : [7:0] rate_frac */
};
typedef struct sys_queue_shp_profile_s sys_queue_shp_profile_t;

struct sys_group_shp_profile_s
{
    uint8   profile_id;
    uint8   rsv;
    uint16  bucket_thrd;
    uint32  bucket_rate;
};
typedef struct sys_group_shp_profile_s sys_group_shp_profile_t;

struct sys_queue_drop_ecn_s
{
    uint8 discard;
    uint8 exception_en;
    uint8 mapped_ecn;
    uint8 rsv;
};
typedef struct sys_queue_drop_ecn_s sys_queue_drop_ecn_t;

/**
 @brief Queue drop profile
*/
struct sys_queue_drop_profile_data_s
{
   uint32    ecn_mark_thrd0;
   uint32    ecn_mark_thrd1;
   uint32    ecn_mark_thrd2;
   uint32    ecn_mark_thrd3;
   uint32    wred_max_thrd3;
   uint32    wred_max_thrd2;
   uint32    wred_max_thrd1;
   uint32    wred_max_thrd0;
   uint8    drop_thrd_factor3;
   uint8    drop_thrd_factor2;
   uint8    drop_thrd_factor1;
   uint8    drop_thrd_factor0;
};
typedef struct sys_queue_drop_profile_data_s sys_queue_drop_profile_data_t;

struct sys_queue_drop_wtd_profile_s
{
    uint8    profile_id;
    uint8    rsv[3];
    sys_queue_drop_profile_data_t profile[SYS_RESRC_MAX_CONGEST_LEVEL_NUM];
};
typedef struct sys_queue_drop_wtd_profile_s sys_queue_drop_wtd_profile_t;

struct sys_queue_drop_wred_profile_s
{
    uint32    wred_min_thrd3;
    uint32    wred_min_thrd2;
    uint32    wred_min_thrd1;
    uint32    wred_min_thrd0;
    uint32    wred_max_thrd3;
    uint32    wred_max_thrd2;
    uint32    wred_max_thrd1;
    uint32    wred_max_thrd0;
    uint8     factor0;
    uint8     factor1;
    uint8     factor2;
    uint8     factor3;
    uint8    profile_id;
    uint8    rsv[3];
};
typedef struct sys_queue_drop_wred_profile_s sys_queue_drop_wred_profile_t;

struct sys_queue_drop_profile_s
{
    sys_queue_drop_wtd_profile_t* p_drop_wtd_profile;
    sys_queue_drop_wred_profile_t* p_drop_wred_profile;
};
typedef struct sys_queue_drop_profile_s sys_queue_drop_profile_t;

struct sys_queue_node_s
{
    uint8  type;        /* refer to ctc_queue_type_t */
    uint8  offset;
    uint8  shp_en;
    uint8  rsv;
    uint16 queue_id;   /*0~1280*/
    sys_queue_drop_profile_t drop_profile;
    sys_queue_shp_profile_t* p_shp_profile;
    sys_queue_meter_profile_t* p_meter_profile;
};
typedef struct sys_queue_node_s sys_queue_node_t;

struct sys_queue_group_node_s
{
    uint16   group_id;  /* ext group id; 0~63.*/
    uint16   shp_bitmap;
    uint8    chan_id;   /*0~63*/
    uint8    rsv[3];
    sys_group_shp_profile_t* p_shp_profile;
    sys_group_meter_profile_t* p_meter_profile;
};
typedef struct sys_queue_group_node_s sys_queue_group_node_t;

struct sys_qos_rate_granularity_s
{
    uint32 max_rate;        /* unit is Mbps */
    uint32 granularity;     /* unit is Kbps */
};
typedef struct sys_qos_rate_granularity_s sys_qos_rate_granularity_t;

struct sys_qos_resc_igs_port_min_s
{
    uint16 ref;
    uint16 min;
};
typedef struct sys_qos_resc_igs_port_min_s sys_qos_resc_igs_port_min_t;

struct sys_qos_resc_egs_pool_s
{
    uint8 egs_congest_level_num;
    uint8 default_profile_id;
};
typedef struct sys_qos_resc_egs_pool_s sys_qos_resc_egs_pool_t;

struct sys_qos_fc_profile_s
{
    uint16 xon;
    uint16 xoff;
    uint16 thrd;
    uint8  profile_id;
    uint8  type;/* 0:flow control profile 1:drop thrd profile*/
};
typedef struct sys_qos_fc_profile_s sys_qos_fc_profile_t;

struct sys_queue_master_s
{
    sys_cpu_reason_t  cpu_reason[CTC_PKT_CPU_REASON_MAX_COUNT];
    ctc_vector_t*  queue_vec;
    ctc_vector_t*  group_vec;
    ctc_spool_t* p_queue_profile_pool;
    ctc_spool_t* p_queue_meter_profile_pool;
    ctc_spool_t* p_drop_wtd_profile_pool;
    ctc_spool_t* p_drop_wred_profile_pool;
    ctc_spool_t* p_group_profile_pool;
    ctc_spool_t* p_group_profile_meter_pool;
    ctc_spool_t* p_fc_profile_pool;
    ctc_spool_t* p_pfc_profile_pool;
    ctc_hash_t* p_port_extender_hash;
    sys_qos_fc_profile_t *p_fc_profile[SYS_MAX_CHANNEL_NUM];
    sys_qos_fc_profile_t *p_pfc_profile[SYS_MAX_CHANNEL_NUM][8];
    uint8 igs_resrc_mode;
    uint8 egs_resrc_mode;
    uint8 store_chan_shp_en;
    uint8 store_que_shp_en;
    uint8 monitor_drop_en;
    uint8 queue_pktadj_len[7];
    uint8 shp_pps_en;
    uint8 have_lcpu_by_eth;
    uint8 queue_num_per_chanel;
    uint8 queue_num_per_extend_port;
    uint8 max_chan_per_slice;
    uint8 cpu_eth_chan_id;
    uint8 max_dma_rx_num;
    uint32 chan_shp_update_freq;
    uint32 que_shp_update_freq;
    sys_qos_rate_granularity_t que_shp_gran[SYS_SHP_GRAN_RANAGE_NUM];
    uint8 support_stacking;
    uint8 queue_num_for_cpu_reason;
    uint16 wrr_weight_mtu;
    uint8 opf_type_group_id;
    uint8 opf_type_tcam_index;
    uint8 opf_type_queue_shape;
    uint8 opf_type_queue_meter;
    uint8 opf_type_group_shape;
    uint8 opf_type_group_meter;
    uint8 opf_type_queue_drop_wtd;
    uint8 opf_type_queue_drop_wred;
    uint8 opf_type_resrc_fc;
    uint8 opf_type_resrc_pfc;
    uint8 opf_type_excp_index;
    sys_qos_resc_egs_pool_t egs_pool[CTC_QOS_EGS_RESRC_POOL_MAX];
    uint8 enq_mode;  /* 0: 8(uc+mc)+8(c2c)+1(cpuTx) mode, channel based;
                        1: 8+4(mc)ext queue+1(log) mode, channel based;
                        2: 8+1(mc)+1(log) mode, channel based;*/
    uint8 queue_thrd_factor;
    uint8 opf_type_resrc_fc_dropthrd;
    uint8 opf_type_resrc_pfc_dropthrd;
    ctc_spool_t* p_fc_dropthrd_profile_pool;
    ctc_spool_t* p_pfc_dropthrd_profile_pool;
    sys_qos_fc_profile_t *p_dropthrd_fc_profile[SYS_MAX_CHANNEL_NUM];
    sys_qos_fc_profile_t *p_dropthrd_pfc_profile[SYS_MAX_CHANNEL_NUM][8];
    uint8 resrc_check_mode; /* 1:DISCRETE  0:CONTINUOUS */
    ctc_hash_t* p_service_hash;
    ctc_hash_t* p_logicport_service_hash;
    uint8 service_queue_mode; /* 1:service id+dstport  0:logic src port+dstport 2:logic port +dsport enq,logic port can be src and dst*/
};
typedef struct sys_queue_master_s sys_queue_master_t;

struct sys_extend_que_node_s
{
    uint8  type;     /** sys_extend_que_type_t */
    uint8  lchip;
    uint16 service_id;
    uint16 lport;
    uint8  channel;
    uint8  rsv;
    uint16 logic_src_port;
    uint16 rsv1;
    uint16 logic_dst_port;
    uint16 rsv2;
    uint16 group_id;
    uint16 key_index;
};
typedef struct sys_extend_que_node_s sys_extend_que_node_t;

struct sys_qos_logicport_service_s
{
    uint16  logic_src_port;
    uint16  service_id;
};
typedef struct sys_qos_logicport_service_s sys_qos_logicport_service_t;

struct sys_qos_logicport_s
{
    ctc_slistnode_t head;
    uint16 logic_port;
};
typedef struct sys_qos_logicport_s sys_qos_logicport_t;

struct sys_qos_destport_s
{
    ctc_slistnode_t head;
    uint8 lchip;
    uint8 lport;          /*dest port list*/
};
typedef struct sys_qos_destport_s sys_qos_destport_t;

struct sys_service_node_s
{
    uint16 service_id;               /*service id*/
    uint16 rsv;
    ctc_slist_t* p_logic_port_list; /*source logic port list*/
    ctc_slist_t* p_dest_port_list;  /*dest port list*/
};
typedef struct sys_service_node_s sys_service_node_t;

/****************************** queue drop ******************************/
#define SYS_DROP_COUNT_GE       0x38
#define SYS_DROP_DELTA_GE       0x4
#define SYS_DROP_COUNT_XGE      0x40
#define SYS_DROP_DELTA_XGE      0x10

#define SYS_DEFAULT_TAIL_DROP_PROFILE 0
#define SYS_DEFAULT_GRP_DROP_PROFILE  0

/*********************************************************************
*
* Function Declaration
*
*********************************************************************/
/****************************** qos class ******************************/
extern int32
sys_usw_qos_domain_map_set(uint8 lchip, ctc_qos_domain_map_t* p_domain_map);
extern int32
sys_usw_qos_domain_map_get(uint8 lchip, ctc_qos_domain_map_t* p_domain_map);
extern int32
sys_usw_qos_table_map_set(uint8 lchip, ctc_qos_table_map_t* p_domain_map);
extern int32
sys_usw_qos_table_map_get(uint8 lchip, ctc_qos_table_map_t* p_domain_map);
extern int32
_sys_usw_qos_domain_map_dump(uint8 lchip, uint8 domain, uint8 type);
extern int32
_sys_usw_qos_table_map_dump(uint8 lchip, uint8 table_map_id, uint8 type);
extern int32
sys_usw_qos_class_init(uint8 lchip);
extern int32
sys_usw_qos_class_deinit(uint8 lchip);

/****************************** qos policer ******************************/
extern int32
sys_usw_qos_policer_set(uint8 lchip, ctc_qos_policer_t* p_policer);
extern int32
sys_usw_qos_policer_get(uint8 lchip, ctc_qos_policer_t* p_policer);
extern int32
sys_usw_qos_lookup_policer(uint8 lchip, uint8 sys_policer_type, uint8 dir, uint16 policer_id);
extern int32
_sys_usw_qos_policer_index_get(uint8 lchip, uint16 policer_id, uint8 sys_policer_type,
                                                sys_qos_policer_param_t* p_policer_param);
extern int32
_sys_usw_qos_policer_map_token_rate_user_to_hw(uint8 lchip, uint8 is_pps, uint32 user_rate,
                                                    uint32 *hw_rate, uint16 bit_with, uint32 gran, uint32 upd_freq );
extern int32
_sys_usw_qos_policer_map_token_rate_hw_to_user(uint8 lchip, uint8 is_pps,
                                                                     uint32 hw_rate,uint32 *user_rate, uint32 upd_freq);
extern int32
sys_usw_qos_policer_copp_alloc(uint8 lchip, uint32 rate, uint32 threshold,
                                               uint8 pps_mode, uint8* p_policer_ptr);
extern int32
sys_usw_qos_policer_copp_free(uint8 lchip, uint8 policer_ptr);
extern int32
sys_usw_qos_policer_mfp_get_profile(uint8 lchip, uint8 profile_id, uint32* p_rate, uint32* p_threshold);
extern int32
sys_usw_qos_set_policer_update_enable(uint8 lchip, uint8 enable);
extern int32
sys_usw_qos_set_phb(uint8 lchip, ctc_qos_phb_map_t *p_phb_map);
extern int32
sys_usw_qos_get_phb(uint8 lchip, ctc_qos_phb_map_t *p_phb_map);
extern int32
sys_usw_qos_set_policer_ipg_enable(uint8 lchip, uint8 enable);
extern int32
sys_usw_qos_policer_stats_query(uint8 lchip, ctc_qos_policer_stats_t* p_stats);
extern int32
sys_usw_qos_policer_stats_clear(uint8 lchip, ctc_qos_policer_stats_t* p_stats);
extern int32
sys_usw_qos_set_policer_level_select(uint8 lchip, ctc_qos_policer_level_select_t policer_level);
extern int32
sys_usw_qos_get_policer_level_select(uint8 lchip, ctc_qos_policer_level_select_t* policer_level);
extern int32
_sys_usw_qos_policer_dump(uint8 type, uint8 dir, uint8 lchip, uint16 start, uint16 end, uint8 detail);
extern int32
sys_usw_qos_policer_wb_sync(uint8 lchip, ctc_wb_data_t *p_wb_data);
extern int32
sys_usw_qos_policer_wb_restore(uint8 lchip, ctc_wb_query_t *p_wb_query);
extern int32
sys_usw_qos_policer_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param);
extern int32
sys_usw_qos_policer_init(uint8 lchip, ctc_qos_global_cfg_t *p_glb_parm);
extern int32
sys_usw_qos_policer_deinit(uint8 lchip);
extern int32
_sys_usw_qos_policer_ingress_vlan_get(uint8 lchip, uint16* ingress_vlan_policer_num);
extern int32
_sys_usw_qos_policer_egress_vlan_get(uint8 lchip, uint16* egress_vlan_policer_num);
/****************************** queue enq ******************************/
extern sys_extend_que_node_t*
sys_usw_port_extender_lookup(uint8 lchip, sys_extend_que_node_t* p_node);
extern int32
sys_usw_qos_get_mux_port_enable(uint8 lchip, uint32 gport, uint8* enable);
extern int32
sys_usw_queue_get_queue_id(uint8 lchip, ctc_qos_queue_id_t* p_queue,
                                   uint16* queue_id);
extern uint8
_sys_usw_queue_get_enq_mode(uint8 lchip);
extern int32
sys_usw_queue_get_queue_type_by_queue_id(uint8 lchip, uint16 queue_id, uint8 *queue_type);
extern int32
sys_usw_get_channel_by_queue_id(uint8 lchip, uint16 queue_id, uint8 *channel);
extern int32
sys_usw_queue_get_enqueue_info(uint8 lchip, uint16* port_num, uint8 *destid_mode);
extern int32
sys_usw_qos_queue_set(uint8 lchip, ctc_qos_queue_cfg_t* p_que_cfg);
extern int32
sys_usw_set_local_switch_enable(uint8 lchip, bool enable);
extern int32
sys_usw_get_local_switch_enable(uint8 lchip, bool* p_enable);
extern int32
sys_usw_port_queue_add(uint8 lchip,
                             uint8 queue_type,
                             uint8 lport);
extern int32
sys_usw_port_queue_remove(uint8 lchip,
                                uint8 queue_type,
                                uint8 lport);
extern int32
_sys_usw_qos_add_extend_port_to_channel(uint8 lchip, uint16 lport, uint8 channel);
extern int32
_sys_usw_qos_remove_extend_port_to_channel(uint8 lchip, uint16 lport, uint8 channel);
extern int32
_sys_usw_qos_add_mcast_queue_to_channel(uint8 lchip, uint16 lport, uint8 channel);
extern int32
_sys_usw_qos_remove_mcast_queue_to_channel(uint8 lchip, uint16 lport, uint8 channel);
extern int32
_sys_usw_queue_add_for_stacking(uint8 lchip, uint32 gport);
extern int32
_sys_usw_queue_remove_for_stacking(uint8 lchip, uint32 gport);
extern uint8
_sys_usw_queue_get_service_queue_mode(uint8 lchip);
extern int32
_sys_usw_qos_bind_service_logic_dstport(uint8 lchip, uint16 service_id, uint16 logic_dst_port);
extern int32
_sys_usw_qos_unbind_service_logic_dstport(uint8 lchip, uint16 service_id, uint16 logic_dst_port);
extern int32
_sys_usw_qos_bind_service_logic_srcport(uint8 lchip, uint16 service_id, uint16 logic_port);
extern int32
_sys_usw_qos_unbind_service_logic_srcport(uint8 lchip, uint16 service_id, uint16 logic_port);
extern int32
sys_usw_get_channel_agg_group(uint8 lchip, uint16 chan_id, uint8* chanagg_id);
extern int32
sys_usw_add_port_to_channel_agg(uint8 lchip, uint16 lport, uint8 channel_agg);
extern int32
sys_usw_remove_port_from_channel_agg(uint8 lchip, uint16 lport);
extern int32
_sys_usw_get_channel_by_port(uint8 lchip, uint32 gport, uint8 *channel);
extern int32
_sys_usw_add_port_to_channel(uint8 lchip, uint32 lport, uint8 channel);
extern int32
_sys_usw_remove_port_from_channel(uint8 lchip, uint32 lport, uint8 channel);
extern int32
sys_usw_set_priority_queue_select(uint8 lchip,
                                        uint8 priority,
                                        uint8 queue_select);
extern int32
_sys_usw_set_dma_channel_drop_en(uint8 lchip, bool enable);
extern int32
_sys_usw_qos_set_aqmscan_high_priority_en(uint8 lchip, bool enable);
extern int32
sys_usw_queue_stats_query(uint8 lchip, ctc_qos_queue_stats_t* p_stats);
extern int32
sys_usw_queue_stats_clear(uint8 lchip, ctc_qos_queue_stats_t* p_stats);
extern int32
sys_usw_qos_set_monitor_drop_queue_id(uint8 lchip,  ctc_qos_queue_drop_monitor_t* drop_monitor);
extern int32
sys_usw_qos_get_monitor_drop_queue_id(uint8 lchip,  ctc_qos_queue_drop_monitor_t* drop_monitor);
extern int32
sys_usw_qos_map_token_rate_hw_to_user(uint8 lchip, uint8 is_pps,
                                            uint32 hw_rate,
                                            uint32 *user_rate,
                                            uint16 bit_with,
                                            uint32 upd_freq,
                                            uint16 pps_pkt_byte);
extern int32
sys_usw_qos_map_token_rate_user_to_hw(uint8 lchip, uint8 is_pps,
                                            uint32 user_rate, /*kb/s*/
                                            uint32 *hw_rate,
                                            uint16 bit_with,
                                            uint32 gran,
                                            uint32 upd_freq,
                                            uint16 pps_pkt_byte);
extern int32
_sys_usw_qos_map_token_thrd_hw_to_user(uint8 lchip, uint32  *user_bucket_thrd,
                                            uint32 hw_bucket_thrd,
                                            uint8 shift_bits);
extern int32
_sys_usw_qos_map_token_thrd_user_to_hw(uint8 lchip, uint32  user_bucket_thrd,
                                            uint32 *hw_bucket_thrd,
                                            uint8 shift_bits,
                                            uint32 max_thrd);
extern bool
sys_usw_queue_shp_get_channel_pps_enbale(uint8 lchip, uint8 chand_id);
extern int32
sys_usw_qos_queue_dump_status(uint8 lchip);
extern int32
sys_usw_qos_sched_dump_status(uint8 lchip);
extern int32
sys_usw_qos_policer_dump_status(uint8 lchip);
extern int32
sys_usw_qos_drop_dump_status(uint8 lchip);
extern int32
sys_usw_qos_shape_dump_status(uint8 lchip);
extern int32
_sys_usw_qos_queue_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail);
extern int32
_sys_usw_qos_group_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail);
extern int32
_sys_usw_qos_port_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail);
extern int32
_sys_usw_qos_service_dump(uint8 lchip, uint16 service_id, uint32 gport);
extern int32
_sys_usw_qos_dump_status(uint8 lchip);
extern int32
sys_usw_queue_wb_sync(uint8 lchip, ctc_wb_data_t *p_wb_data);
extern int32
sys_usw_queue_wb_restore(uint8 lchip, ctc_wb_query_t *p_wb_query);
extern int32
sys_usw_qos_queue_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param);
extern int32
sys_usw_queue_enq_init(uint8 lchip, void* p_glb_parm);
extern int32
sys_usw_queue_enq_deinit(uint8 lchip);

/****************************** queue shape ******************************/
extern int32
_sys_usw_queue_shp_alloc_profileId(sys_queue_shp_profile_t* p_node, uint8* p_lchip);
extern int32
_sys_usw_queue_shp_restore_profileId(sys_queue_shp_profile_t* p_node, uint8* p_lchip);
extern int32
_sys_usw_queue_meter_alloc_profileId(sys_queue_meter_profile_t* p_node, uint8* p_lchip);
extern int32
_sys_usw_queue_meter_restore_profileId(sys_queue_meter_profile_t* p_node, uint8* p_lchip);
extern int32
_sys_usw_group_shp_alloc_profileId(sys_group_shp_profile_t* p_node, uint8* p_lchip);
extern int32
_sys_usw_group_shp_restore_profileId(sys_group_shp_profile_t* p_node, uint8* p_lchip);
extern int32
_sys_usw_group_meter_alloc_profileId(sys_group_meter_profile_t* p_node, uint8* p_lchip);
extern int32
_sys_usw_group_meter_restore_profileId(sys_group_meter_profile_t* p_node, uint8* p_lchip);
extern int32
_sys_usw_queue_shp_get_queue_shape_profile_from_asic(uint8 lchip,
                                               sys_queue_shp_profile_t* p_sys_profile);
extern int32
_sys_usw_queue_shp_get_queue_meter_profile_from_asic(uint8 lchip,
                                               sys_queue_meter_profile_t* p_sys_profile);
extern int32
_sys_usw_queue_shp_get_group_shape_profile_from_asic(uint8 lchip,
                                               sys_group_shp_profile_t* p_sys_profile);
extern int32
_sys_usw_queue_shp_get_group_meter_profile_from_asic(uint8 lchip,
                                               sys_group_meter_profile_t* p_sys_meter_profile);
extern int32
_sys_usw_qos_set_shape(uint8 lchip, ctc_qos_shape_t* p_shape);
extern int32
_sys_usw_qos_get_shape(uint8 lchip, ctc_qos_shape_t* p_shape);
extern int32
_sys_usw_queue_shp_set_group_shape(uint8 lchip, ctc_qos_shape_group_t* p_shape);
extern int32
sys_usw_qos_set_port_shape_enable(uint8 lchip, uint8 enable);
extern int32
sys_usw_qos_set_queue_shape_enable(uint8 lchip, uint8 enable);
extern int32
sys_usw_qos_set_group_shape_enable(uint8 lchip, uint8 enable);
extern int32
sys_usw_qos_get_port_shape_enable(uint8 lchip, uint32* p_enable);
extern int32
sys_usw_qos_get_queue_shape_enable(uint8 lchip, uint32* p_enable);
extern int32
sys_usw_qos_get_group_shape_enable(uint8 lchip, uint32* p_enable);
extern int32
sys_usw_qos_set_shape_ipg_enable(uint8 lchip, uint8 enable);
extern int32
sys_usw_qos_set_reason_shp_base_pkt_en(uint8 lchip, uint8 enable);
extern int32
sys_usw_qos_set_port_shp_base_pkt_en(uint8 lchip, uint8 enable);
extern int32
sys_usw_qos_set_shape_mode(uint8 lchip, uint8 mode);
extern int32
_sys_usw_qos_get_port_shape_profile(uint8 lchip, uint32 gport, uint32* rate, uint32* thrd, uint8* p_shp_en);
extern int32
sys_usw_queue_shape_init(uint8 lchip);
extern int32
sys_usw_queue_shape_deinit(uint8 lchip);

/****************************** queue sch ******************************/
extern int32
_sys_usw_qos_set_sched(uint8 lchip, ctc_qos_sched_t* p_sched);
extern int32
_sys_usw_qos_get_sched(uint8 lchip, ctc_qos_sched_t* p_sched);
extern int32
sys_usw_queue_sch_get_group_sched(uint8 lchip, uint16 sub_grp,ctc_qos_sched_group_t* p_sched);
extern int32
sys_usw_queue_sch_get_queue_sched(uint8 lchip, uint16 queue_id ,ctc_qos_sched_queue_t* p_sched);
extern int32
sys_usw_queue_sch_set_c2c_group_sched(uint8 lchip, uint16 queue_id, uint8 class_priority);
extern int32
sys_usw_qos_set_sch_wrr_enable(uint8 lchip, uint8 enable);
extern int32
sys_usw_queue_sch_init(uint8 lchip);
extern int32
sys_usw_queue_sch_deinit(uint8 lchip);

/****************************** queue drop ******************************/
extern int32
_sys_usw_queue_drop_wtd_alloc_profileId(sys_queue_drop_wtd_profile_t* p_node, uint8* p_lchip);;
extern int32
_sys_usw_queue_drop_wred_alloc_profileId(sys_queue_drop_wred_profile_t* p_node, uint8* p_lchip);
extern int32
_sys_usw_queue_drop_wred_restore_profileId(sys_queue_drop_wred_profile_t* p_node, uint8* p_lchip);
extern int32
_sys_usw_qos_fc_alloc_profileId(sys_qos_fc_profile_t* p_node, uint8* p_lchip);
extern int32
_sys_usw_qos_pfc_alloc_profileId(sys_qos_fc_profile_t* p_node, uint8* p_lchip);
extern int32
_sys_usw_queue_get_profile_from_hw(uint8 lchip, uint32 gport, sys_qos_shape_profile_t* p_shp_profile);
extern int32
_sys_usw_queue_set_port_drop_en(uint8 lchip, uint32 gport, bool enable, sys_qos_shape_profile_t* p_shp_profile);
extern int32
_sys_usw_queue_get_port_depth(uint8 lchip, uint32 gport, uint32* p_depth);
extern int32
sys_usw_qos_flow_ctl_profile(uint8 lchip, uint32 gport, uint8 cos, uint8 is_pfc,  uint8 enable);
extern int32
_sys_usw_queue_drop_read_profile_from_asic(uint8 lchip, uint8 wred_mode,
                                             sys_queue_node_t* p_sys_queue,
                                             sys_queue_drop_profile_t* p_sys_profile);
extern int32
sys_usw_qos_fc_get_profile_from_asic(uint8 lchip, uint8 is_pfc, sys_qos_fc_profile_t *p_sys_fc_profile);
extern int32
sys_usw_qos_fc_get_dropthrd_profile_from_asic(uint8 lchip, uint8 is_pfc, sys_qos_fc_profile_t *p_sys_fc_profile);
extern int32
sys_usw_queue_set_default_drop(uint8 lchip,
                                     uint16 queue_id,
                                     uint8 profile_id);
extern int32
_sys_usw_qos_fc_profile_add_spool(uint8 lchip, uint8 is_pfc, uint8 is_dropthrd, sys_qos_fc_profile_t* p_sys_profile_old,
                                                          sys_qos_fc_profile_t* p_sys_profile_new,
                                                          sys_qos_fc_profile_t** pp_sys_profile_get);
extern int32
_sys_usw_queue_drop_wtd_profile_add_spool(uint8 lchip, sys_queue_drop_wtd_profile_t* p_sys_profile_old,
                                                          sys_queue_drop_wtd_profile_t* p_sys_profile_new,
                                                          sys_queue_drop_wtd_profile_t** pp_sys_profile_get);
extern int32
_sys_usw_queue_drop_wred_profile_add_spool(uint8 lchip, sys_queue_drop_wred_profile_t* p_sys_profile_old,
                                                          sys_queue_drop_wred_profile_t* p_sys_profile_new,
                                                          sys_queue_drop_wred_profile_t** pp_sys_profile_get);
extern int32
sys_usw_queue_get_sc_tc(uint8 lchip, sys_queue_node_t* p_sys_queue, uint8* sc, uint8* tc);

extern int32
sys_usw_queue_set_drop(uint8 lchip, ctc_qos_drop_t* p_drop);
extern int32
sys_usw_queue_get_drop(uint8 lchip, ctc_qos_drop_t* p_drop);
extern int32
sys_usw_queue_set_cpu_queue_egs_pool_classify(uint8 lchip, uint16 reason_id, uint8 pool);
extern int32
sys_usw_queue_set_resrc(uint8 lchip, ctc_qos_resrc_t* p_resrc);
extern int32
sys_usw_queue_get_resrc(uint8 lchip, ctc_qos_resrc_t* p_resrc);
extern int32
sys_usw_queue_query_pool_stats(uint8 lchip, ctc_qos_resrc_pool_stats_t* p_stats);
extern int32
sys_usw_qos_resrc_mgr_en(uint8 lchip, uint8 enable);
extern int32
sys_usw_qos_set_ecn_enable(uint8 lchip, uint32 enable);
extern int32
sys_usw_qos_get_ecn_enable(uint8 lchip, uint32* enable);
extern int32
sys_usw_queue_drop_init(uint8 lchip, void *p_glb_parm);
extern int32
sys_usw_queue_drop_deinit(uint8 lchip);
extern int32
sys_usw_qos_set_resrc_check_mode(uint8 lchip, uint8 mode);
extern int32
sys_usw_qos_set_fcdl_interval(uint8 lchip, uint32 time);
extern int32
sys_usw_qos_set_port_policer_stbm_enable(uint8 lchip, uint8 enable);

/****************************** cpu reason ******************************/
extern int32
sys_usw_queue_get_excp_idx_by_reason(uint8 lchip, uint16 reason_id,
                                            uint16* excp_idx,
                                            uint8* excp_num);
extern int32
_sys_usw_cpu_reason_get_reason_info(uint8 lchip, uint8 dir,
                                          sys_cpu_reason_info_t* p_cpu_rason_info);
extern int32
_sys_usw_cpu_reason_free_exception_index(uint8 lchip, uint8 dir,
                                            sys_cpu_reason_info_t* p_cpu_rason_info);
extern int32
_sys_usw_cpu_reason_alloc_exception_index(uint8 lchip, uint8 dir,
                                            sys_cpu_reason_info_t* p_cpu_rason_info);
extern int32
_sys_usw_cpu_reason_alloc_dsfwd_offset(uint8 lchip, uint16 reason_id,
                                           uint32 *dsfwd_offset,
                                           uint32 *dsnh_offset,
                                           uint32 *dest_port);
extern int32
sys_usw_cpu_reason_set_map(uint8 lchip, uint16 reason_id,
                                  uint8 sub_queue_id,
                                  uint8 group_id);
extern int32
sys_usw_cpu_reason_set_dest(uint8 lchip, uint16 reason_id,
                                  uint8 dest_type,
                                  uint32 dest_port);
extern int32
_sys_usw_get_sub_queue_id_by_cpu_reason(uint8 lchip, uint16 reason_id, uint8* sub_queue_id);
extern int32
sys_usw_cpu_reason_set_misc_param(uint8 lchip, uint16 reason_id, uint32 truncatd_len);

extern int32
_sys_usw_cpu_reason_get_info(uint8 lchip, uint16 reason_id,
                                           uint32 *destmap);
extern int32
_sys_usw_qos_cpu_reason_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail);
extern int32
sys_usw_queue_cpu_reason_init(uint8 lchip);
extern int32
sys_usw_queue_cpu_reason_deinit(uint8 lchip);

#ifdef __cplusplus
}
#endif

#endif

