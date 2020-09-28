/**
 @file sys_usw_stats_api.h

 @date 2009-12-15

 @version v2.0

*/
#ifndef _SYS_USW_STATS_API_H
#define _SYS_USW_STATS_API_H
#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_const.h"
#include "ctc_macro.h"
#include "ctc_stats.h"

#define SYS_STATS_IPE_DISCARD_TYPE_START            0x1
#define SYS_STATS_IPE_DISCARD_TYPE_END              0x40
#define SYS_STATS_IPE_DISCARD_TYPE_MAX              (SYS_STATS_IPE_DISCARD_TYPE_END - SYS_STATS_IPE_DISCARD_TYPE_START + 1)
#define SYS_STATS_EPE_DISCARD_TYPE_START            0x41
#define SYS_STATS_EPE_DISCARD_TYPE_END              0x78
#define SYS_STATS_EPE_DISCARD_TYPE_MAX              (SYS_STATS_EPE_DISCARD_TYPE_END - SYS_STATS_EPE_DISCARD_TYPE_START + 1)
#define SYS_STATS_DISCARD_TYPE_MAX                  ((SYS_STATS_IPE_DISCARD_TYPE_MAX >= SYS_STATS_EPE_DISCARD_TYPE_MAX) ? SYS_STATS_IPE_DISCARD_TYPE_MAX: SYS_STATS_EPE_DISCARD_TYPE_MAX)

#define SYS_STATS_IS_ACL_PRIVATE(ptr)   (DRV_IS_DUET2(lchip) ? ((ptr)& 0x8000) : ((ptr)& 0x10000))
#define SYS_STATS_DECODE_CHIP_PTR_OFFSET(ptr) (DRV_IS_DUET2(lchip) ? ((ptr)& 0xFFF) : ((ptr)& 0x1FFF))

/* Define sys stats type */
enum sys_stats_type_e
{
    SYS_STATS_TYPE_QUEUE,
    SYS_STATS_TYPE_VLAN,
    SYS_STATS_TYPE_INTF,
    SYS_STATS_TYPE_VRF,
    SYS_STATS_TYPE_SCL,
    SYS_STATS_TYPE_TUNNEL,
    SYS_STATS_TYPE_LSP,
    SYS_STATS_TYPE_PW,
    SYS_STATS_TYPE_ACL,
    SYS_STATS_TYPE_FWD_NH,
    SYS_STATS_TYPE_FIB,
    SYS_STATS_TYPE_POLICER0,
    SYS_STATS_TYPE_POLICER1,
    SYS_STATS_TYPE_ECMP,
    SYS_STATS_TYPE_PORT,
    SYS_STATS_TYPE_FLOW_HASH,
    SYS_STATS_TYPE_MAC,
    SYS_STATS_TYPE_SDC,
    SYS_STATS_TYPE_MAX
};
typedef enum sys_stats_type_e sys_stats_type_t;

/*policing storage structor*/
struct sys_stats_policing_s
{
    uint64 policing_confirm_pkts;
    uint64 policing_confirm_bytes;
    uint64 policing_exceed_pkts;
    uint64 policing_exceed_bytes;
    uint64 policing_violate_pkts;
    uint64 policing_violate_bytes;
};
typedef struct sys_stats_policing_s sys_stats_policing_t;

/*queue storage structor*/
struct sys_stats_queue_s
{
    uint64 packets;
    uint64 bytes;
};
typedef struct sys_stats_queue_s sys_stats_queue_t;

struct sys_stats_statsid_s
{
    uint32 stats_id;
    uint32 stats_ptr;
    uint16 hw_stats_ptr;
    uint8  stats_id_type:6;
    uint8  dir:1;
    uint8  color_aware:1;

    union
    {
        uint8  acl_priority;
        uint8  is_vc_label;
    }u;
};
typedef struct sys_stats_statsid_s sys_stats_statsid_t;
/***************************************************************
 *
 *  flow stats api
 *
 ***************************************************************/
extern int32
sys_usw_flow_stats_ram_assign(uint8 lchip, uint16 ram_bmp[][CTC_BOTH_DIRECTION], uint16 acl_ram_bmp[][CTC_BOTH_DIRECTION]);
extern int32
sys_usw_flow_stats_init(uint8 lchip, ctc_stats_global_cfg_t* stats_global_cfg);
extern int32
sys_usw_flow_stats_deinit(uint8 lchip);
extern int32
sys_usw_flow_stats_set_drop_packet_stats_en(uint8 lchip, ctc_stats_discard_t bitmap, bool enable);
extern int32
sys_usw_flow_stats_get_drop_packet_stats_en(uint8 lchip, ctc_stats_discard_t bitmap, bool* enable);
extern int32
sys_usw_flow_stats_get_igs_port_log_stats(uint8 lchip, uint32 gport, ctc_stats_basic_t* p_stats);
extern int32
sys_usw_flow_stats_clear_igs_port_log_stats(uint8 lchip, uint32 gport);
extern int32
sys_usw_flow_stats_get_egs_port_log_stats(uint8 lchip, uint32 gport, ctc_stats_basic_t* p_stats);
extern int32
sys_usw_flow_stats_clear_egs_port_log_stats(uint8 lchip, uint32 gport);
extern int32
sys_usw_flow_stats_enable_igs_port_discard_stats(uint8 lchip, uint8 enable);
extern int32
sys_usw_flow_stats_get_igs_port_discard_stats(uint8 lchip, uint32 gport, ctc_stats_basic_t* p_stats);
extern int32
sys_usw_flow_stats_clear_igs_port_discard_stats(uint8 lchip, uint32 gport);
extern int32
sys_usw_flow_stats_enable_egs_port_discard_stats(uint8 lchip, uint8 enable);
extern int32
sys_usw_flow_stats_get_egs_port_discard_stats(uint8 lchip, uint32 gport, ctc_stats_basic_t* p_stats);
extern int32
sys_usw_flow_stats_clear_egs_port_discard_stats(uint8 lchip, uint32 gport);
extern int32
sys_usw_flow_stats_alloc_statsptr(uint8 lchip, ctc_direction_t dir, uint8 stats_num, uint8 stats_type, uint32* p_stats_ptr);
extern int32
sys_usw_flow_stats_free_statsptr(uint8 lchip, ctc_direction_t dir, uint8 stats_num, uint8 stats_type, uint32* p_stats_ptr);
extern int32
sys_usw_flow_stats_get_policing_stats(uint8 lchip, ctc_direction_t dir, uint32 stats_ptr, uint8 stats_type,sys_stats_policing_t* p_stats);
extern int32
sys_usw_flow_stats_clear_policing_stats(uint8 lchip, ctc_direction_t dir, uint32 stats_ptr, uint8 stats_type);
extern int32
sys_usw_flow_stats_get_queue_stats(uint8 lchip, uint32 stats_ptr, sys_stats_queue_t* p_stats);
extern int32
sys_usw_flow_stats_clear_queue_stats(uint8 lchip, uint32 stats_ptr);
extern int32
sys_usw_flow_stats_set_saturate_en(uint8 lchip, ctc_stats_type_t stats_type, bool enable);
extern int32
sys_usw_flow_stats_get_saturate_en(uint8 lchip, ctc_stats_type_t stats_type, bool* p_enable);
extern int32
sys_usw_flow_stats_set_hold_en(uint8 lchip, ctc_stats_type_t stats_type, bool enable);
extern int32
sys_usw_flow_stats_get_hold_en(uint8 lchip, ctc_stats_type_t stats_type, bool* p_enable);
extern int32
sys_usw_flow_stats_set_clear_after_read_en(uint8 lchip, ctc_stats_type_t stats_type, bool enable);
extern int32
sys_usw_flow_stats_get_clear_after_read_en(uint8 lchip, ctc_stats_type_t stats_type, bool* p_enable);
extern int32
sys_usw_flow_stats_get_statsptr(uint8 lchip, uint32 stats_id, uint32* p_cache_ptr);
extern int32
sys_usw_flow_stats_get_statsptr_with_check(uint8 lchip, sys_stats_statsid_t* p_statsid, uint32* p_cache_ptr);
extern int32
sys_usw_flow_stats_create_statsid(uint8 lchip, ctc_stats_statsid_t* statsid);
extern int32
sys_usw_flow_stats_destroy_statsid(uint8 lchip, uint32 stats_id);
extern int32
sys_usw_flow_stats_lookup_statsid(uint8 lchip, ctc_stats_statsid_type_t type, uint32 stats_ptr, uint32* p_statsid, uint8 dir);
extern int32
sys_usw_flow_stats_get_stats(uint8 lchip, uint32 stats_id, ctc_stats_basic_t* p_stats);
extern int32
sys_usw_flow_stats_clear_stats(uint8 lchip, uint32 stats_id);
extern bool
sys_usw_flow_stats_is_color_aware_en(uint8 lchip, uint32 stats_id);

extern int32
sys_usw_flow_stats_get_statsid(uint8 lchip, sys_stats_statsid_t* p_statsid);
extern int32
sys_usw_flow_stats_get_stats_num(uint8 lchip, uint32 stats_id, uint32* p_stats_num);
/***************************************************************
 *
 *  mac stats api
 *
 ***************************************************************/
extern int32
sys_usw_mac_stats_init(uint8 lchip, ctc_stats_global_cfg_t* stats_global_cfg);
extern int32
sys_usw_mac_stats_deinit(uint8 lchip);
extern int32
sys_usw_mac_stats_set_property_mtu1(uint8 lchip, uint32 gport, uint16 length);
extern int32
sys_usw_mac_stats_get_property_mtu1(uint8 lchip, uint32 gport, uint16* p_length);
extern int32
sys_usw_mac_stats_set_property_mtu2(uint8 lchip, uint32 gport, uint16 length);
extern int32
sys_usw_mac_stats_get_property_mtu2(uint8 lchip, uint32 gport, uint16* p_length);
extern int32
sys_usw_mac_stats_set_property_pfc(uint8 lchip, uint32 gport, uint8 enable);
extern int32
sys_usw_mac_stats_get_property_pfc(uint8 lchip, uint32 gport, uint8* p_enable);
extern int32
sys_usw_mac_stats_set_global_property_hold(uint8 lchip, bool enable);
extern int32
sys_usw_mac_stats_get_global_property_hold(uint8 lchip, bool* p_enable);
extern int32
sys_usw_mac_stats_set_global_property_saturate(uint8 lchip, bool enable);
extern int32
sys_usw_mac_stats_get_global_property_saturate(uint8 lchip, bool* p_enable);
extern int32
sys_usw_mac_stats_set_global_property_clear_after_read(uint8 lchip, bool enable);
extern int32
sys_usw_mac_stats_get_global_property_clear_after_read(uint8 lchip, bool* p_enable);
extern int32
sys_usw_mac_stats_get_rx_stats(uint8 lchip, uint32 gport, ctc_mac_stats_t* p_stats);
extern int32
sys_usw_mac_stats_clear_rx_stats(uint8 lchip, uint32 gport);
extern int32
sys_usw_mac_stats_get_tx_stats(uint8 lchip, uint32 gport, ctc_mac_stats_t* p_stats);
extern int32
sys_usw_mac_stats_clear_tx_stats(uint8 lchip, uint32 gport);
extern void
sys_usw_mac_stats_get_port_rate(uint8 lchip, uint32 gport, uint64* p_throughput);
#ifdef __cplusplus
}
#endif

#endif

