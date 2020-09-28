#if !defined (__SAI_SWITCH_H_)
#define __SAI_SWITCH_H_

#include <saiswitch.h>
#include <ctc_mix.h>

struct ctc_sai_sdk_phy_maping_s
{
    uint16 mac_idx;                  /* mac index of ASIC chip */
    uint16 phy_addr;                  /* phy_addr for mido scan */
    uint16 mdio_bus_addr;             /* mdio_bus_addr for mido scan */
};
typedef struct ctc_sai_sdk_phy_maping_s ctc_sai_sdk_phy_maping_t;

struct ctc_sai_sdk_init_info_s
{

    
    uint8 ftm_profile_type; 
    uint8 stacking_en;
    uint8 datapath_config_file[256]; 
    uint8 ftm_config_file[256]; 



    uint8 gchip[CTC_MAX_LOCAL_CHIP_NUM];
    uint8 local_chip_num;
    uint8 port_mapping_en;

    uint8 queue_num_per_network_port;
    uint8 queue_num_per_static_int_port;
    uint8 queue_num_per_fabric;
    uint8 max_internal_port_id;

    uint8 queue_num_per_internal_port;
    uint8 board_type;
    uint8 queue_num_per_ingress_service;
    uint8 queue_num_per_egress_service;

    uint8 profile_type;
    uint8 nh_dedit_mode;
    uint8 fdb_hw_learning_mode;

    uint16 policer_num;
    uint16 logic_port_num;

    uint16 ext_nexthop_num;
    uint16 mpls_tunnel_num;
    uint32 mpls_entry_num[4];

    uint8 stats_queue_deq_en;
    uint8 stats_queue_drop_en;
    uint8 stats_flow_policer_en;
    uint8 stats_vlan_en;

    uint8 stats_vrf_en;
    uint8 port_phy_mapping_en;
    uint8 cut_through_speed;
    uint8 bfd_vccv_with_ip_en;

    uint16 bfd_mep_num;
    uint16 stats_policer_num;

    uint8 interrupt_mode;
    uint8  bpe_br_pe_en;
    uint16 bpe_br_port_base;

    uint16 bpe_br_uc_max_ecid;
    uint16 bpe_br_mc_max_ecid;

    uint16 cpu_port;
    uint8 cpu_port_en;
    uint8 ecc_recover_en;

    uint8 tcam_scan_en;
    uint8 queue_num_for_cpu_reason;
    uint8 stp_mode;
    uint8 fabric_mode;

    uint32 init_flag;
    uint8 irq_type_exception;
    uint8 irq1;
    uint8 irq2;
    /* Added by liuht to support disable ptp, 2017-04-14 */
    uint8 disable_ptp;
    /* Added by tongzb to support configure synce, 2019-07-10 */
    uint8 disable_synce;

    /* modified by liwh for bug 49424, 2018-10-15 for cut through */
    uint32  cut_through_speed_bitmap; 
    /* liwh end */
    uint16                      port_num;
    ctc_sai_sdk_phy_maping_t*       phy_map;
};
typedef struct ctc_sai_sdk_init_info_s ctc_sai_sdk_init_info_t;

sai_status_t
ctc_sai_switch_get_svlan_tpid(_Inout_  sai_attribute_t *attr);

sai_status_t ctc_sai_switch_init();


#endif

