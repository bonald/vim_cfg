/**
 @file sys_usw_register.h

 @date 2009-11-3

 @version v2.0

Macro, data structure for system common operations

*/

#ifndef _SYS_USW_REGISTER
#define _SYS_USW_REGISTER
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_register.h"
#include "ctc_spool.h"
#include "sys_usw_chip.h"

typedef int32 (* sys_wb_sync_fn)(uint8 lchip);

struct sys_usw_register_dot1ae_bind_sc_s
{
  uint8  type;  /*0-port, 1-nexthop*/
  uint8  dir;
  uint8  include_sci;
  uint8  sc_index;
  uint32 gport;

  uint32 chan_id;
};
typedef struct sys_usw_register_dot1ae_bind_sc_s sys_usw_register_dot1ae_bind_sc_t;

enum sys_usw_register_api_cb_type_e
{
    SYS_REGISTER_CB_TYPE_DOT1AE_BIND_SEC_CHAN,
    SYS_REGISTER_CB_TYPE_DOT1AE_UNBIND_SEC_CHAN,
    SYS_REGISTER_CB_TYPE_DOT1AE_GET_BIND_SEC_CHAN,
    SYS_REGISTER_CB_TYPE_STK_GET_STKHDR_LEN,
    SYS_REGISTER_CB_TYPE_STK_GET_RSV_TRUNK_NUM,
    SYS_REGISTER_CB_TYPE_STK_GET_MCAST_PROFILE_MET_OFFSET,
    SYS_REGISTER_CB_TYPE_INBAND_PORT_EN,
    SYS_REGISTER_CB_TYPE_INBAND_TIMER,
    SYS_REGISTER_CB_TYPE_INBAND_TX,
    SYS_REGISTER_CB_TYPE_INBAND_RX,

    SYS_REGISTER_CB_MAX_TYPE
};
typedef  enum sys_usw_register_api_cb_type_e sys_usw_register_api_cb_type_t;

struct sys_register_cb_api_s
{
    int32 (*dot1ae_bind_sec_chan)(uint8 lchip, sys_usw_register_dot1ae_bind_sc_t* p_bind_sc);
    int32 (*dot1ae_unbind_sec_chan)(uint8 lchip, sys_usw_register_dot1ae_bind_sc_t* p_bind_sc);
    int32 (*dot1ae_get_bind_sec_chan)(uint8 lchip, sys_usw_register_dot1ae_bind_sc_t* p_bind_sc);
    int32 (*stacking_get_stkhdr_len)(uint8 lchip, uint32 gport, uint8* p_packet_head_en, uint8* p_legacy_en, uint16* p_cloud_hdr_len);
    int32 (*stacking_get_rsv_trunk_number)(uint8 lchip, uint8* number);
    int32 (*stacking_get_mcast_profile_met_offset)(uint8 lchip,  uint16 mcast_profile_id, uint32 *p_stacking_met_offset);
    int32 (*inband_port_en)(uint8 lchip, uint32 gport, uint32 enable);
    int32 (*inband_timer)(uint8 lchip, uint32 timer);
    int32 (*inband_tx)(uint8 lchip, void* cfg);
    int32 (*inband_rx)(uint8 lchip, void* cfg);
};
typedef struct sys_register_cb_api_s sys_register_cb_api_t;

struct sys_register_master_s
{
    sys_wb_sync_fn wb_sync_cb[CTC_FEATURE_MAX];
    sys_register_cb_api_t *p_register_api;
    uint8 xgpon_en;
    uint8 gint_en;
    
    ctc_spool_t*        cethertype_spool;      /* Share pool save cEtherType */
    uint8               opf_type_cethertype;   /*range:0~63*/

    sal_mutex_t*        register_mutex;        /**< mutex for register */
    ctc_spool_t*        tcat_prof_spool;      /* Share pool save DsTruncationProfile */
    uint8               opf_type_tcat_prof;   /*range:0~15*/
    uint8               tpoam_vpws_coexist;
};
typedef struct sys_register_master_s sys_register_master_t;

extern sys_register_master_t *p_usw_register_master[CTC_MAX_LOCAL_CHIP_NUM];

struct  sys_dump_db_traverse_param_s
{
   uint8 lchip;
   void* value0;
   void* value1;
   void* value2;
   void* value3;
   void* value4;
};
typedef struct sys_dump_db_traverse_param_s sys_dump_db_traverse_param_t;

#define REGISTER_API(lchip) p_usw_register_master[lchip]->p_register_api

extern int32
sys_usw_global_ctl_set(uint8 lchip, ctc_global_control_type_t type, void* value);

extern int32
sys_usw_global_ctl_get(uint8 lchip, ctc_global_control_type_t type, void* value);

extern int32
sys_usw_lkup_ttl_index(uint8 lchip, uint8 ttl, uint32* ttl_index);
extern int32
sys_usw_lkup_adjust_len_index(uint8 lchip, uint8 adjust_len, uint8* len_index);

extern int32
sys_usw_lkup_ttl(uint8 lchip, uint8 ttl_index, uint32* ttl);

extern int32
sys_usw_register_init(uint8 lchip);

extern int32
sys_usw_wb_sync_register_cb(uint8 lchip, uint8 module, sys_wb_sync_fn fn);

extern int32
sys_usw_dump_db_register_cb(uint8 lchip, uint8 module, CTC_DUMP_MASTER_FUNC fn);

extern int32
sys_usw_register_api_cb(uint8 lchip, uint8 type, void* cb);

/* mode 0: txThrd0=3, txThrd1=4; mode 1: txThrd0=5, txThrd1=11; if cut through enable, default use mode 1 */
/* Mode 1 can avoid CRC, but will increase latency. Mode 0 can reduce latency, but lead to CRC. */
extern int32
sys_usw_net_tx_threshold_cfg(uint8 lchip, uint16 mode);

extern int32
sys_usw_global_set_chip_capability(uint8 lchip, ctc_global_capability_type_t type, uint32 value);

/*for acl module*/
extern int32
sys_usw_get_glb_acl_property(uint8 lchip, ctc_global_acl_property_t* pacl_property);

extern int32
sys_usw_register_deinit(uint8 lchip);

extern int32
sys_usw_global_set_logic_destport_en(uint8 lchip, uint8 enable);

extern int32
sys_usw_global_get_logic_destport_en(uint8 lchip, uint8* enable);

extern int32
sys_usw_global_set_gint_en(uint8 lchip, uint8 enable);

extern int32
sys_usw_global_get_gint_en(uint8 lchip, uint8* enable);

extern int32
sys_usw_global_set_xgpon_en(uint8 lchip, uint8 enable);

extern int32
sys_usw_global_get_xgpon_en(uint8 lchip, uint8* enable);

extern int32
sys_usw_register_add_compress_ether_type(uint8 lchip, uint16 new_ether_type, uint16 old_ether_type,uint8* o_cether_type, uint8* o_cether_type_index);

extern int32
sys_usw_register_remove_compress_ether_type(uint8 lchip, uint16 ether_type);

extern int32
sys_usw_register_add_truncation_profile(uint8 lchip, uint16 length, uint8 old_profile_id, uint8* profile_id);

extern int32
sys_usw_register_remove_truncation_profile(uint8 lchip, uint8 mode, uint16 len_or_profile_id);

extern int32
sys_usw_global_failover_en(uint8 lchip, uint32 enable);

#ifdef __cplusplus
}
#endif

#endif

