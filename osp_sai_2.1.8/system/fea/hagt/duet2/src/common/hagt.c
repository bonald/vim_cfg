
#include "hagt_inc.h"
#include "glb_stm_define.h"
#include "hagt_lcmtlk.h"
#include "hagt_intr.h"
#include "hagt_cpu_traffic.h"
#include "hagt_if.h"
#include "hagt_agg.h"
#include "hagt_vlan.h"
#include "hagt_l2mc.h"
#include "hagt_qos.h"
#include "hagt_stats.h"
#include "hagt_bridge.h"
#include "hagt_ipuc.h"
#include "hagt_ecmp.h"
#include "hagt_ratelimit.h"
#include "hagt_openflow.h"
#include "hagt_mirror.h"
#include "hagt_sflow.h"
#include "hagt_tap.h"

#include "dal.h"
#include "glb_distribute_system_define.h"
#include "ctc_init.h"

/* for bug 35982 copy rom sdk by liuyang 2015-11-4*/
struct ctc_init_chip_info_s
{
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
    uint8 rsv[2];
};
typedef struct ctc_init_chip_info_s ctc_init_chip_info_t;

extern int32 mem_mgr_init (void);
extern int sal_init();
extern int32 ctc_master_cli(int32 is_cli_merge);
extern int32
hagt_packet_init(hagt_master_t* pst_master);
extern int32 drv_chip_write(uint8 lchip, uint32 offset, uint32 value);
extern int32 drv_chip_read(uint8 lchip, uint32 offset, uint32* p_value);


hagt_master_t *g_pst_hagt_master = NULL;
uint32 route_internal_port = 0;
hagt2lcm_callback_t hagt2lcm_cb[HAGT2LCM_CB_TYPE_MAX] = {0};
uint32 is_openflow_ipv6_profile = 0;

#undef CTC_APP_DBG_OUT
#define CTC_APP_DBG_OUT(level, FMT, ...) \
    do { \
        HAGT_LOG_ERR(FMT, ##__VA_ARGS__); \
    } while (0)
#undef CTC_INIT_DBG_OUT
#define CTC_INIT_DBG_OUT(level, FMT, ...) \
    do { \
        HAGT_LOG_ERR(FMT, ##__VA_ARGS__); \
    } while (0)

/**
 @brief Define board type
*/
enum hagt_ctc_board_type
{
    HAGT_CTC_BOARD_24PLUG8,
    HAGT_CTC_BOARD_48PLUG4,
    HAGT_CTC_BOARD_EMU_960_1,
    HAGT_CTC_BOARD_EMU_720_2,
    HAGT_CTC_BOARD_EMU_880_2,

    MAX_HAGT_CTC_BOARD_TYPE
};
typedef enum hagt_ctc_board_type hagt_ctc_board_type_e;

struct ctc_phy_info_s
{
    uint8 mdio_bus;
    uint8 phy_addr;
};
typedef struct ctc_phy_info_s ctc_phy_info_t;

/**
 @brief Define board type
*/
enum ctc_board_type
{
    CTC_BOARD_24PLUS8,
    CTC_BOARD_48PLUS8,
    CTC_BOARD_48PLUS4,

    MAX_CTC_BOARD_TYPE
};
typedef enum ctc_board_type ctc_board_type_e;

/**
 @brief define chip type
*/
enum ctc_init_chip_type_e
{
    CTC_INIT_CHIP_TYPE_GREATBELT,
    CTC_INIT_CHIP_TYPE_RAMA,
    CTC_INIT_CHIP_TYPE_RIALTO,
    CTC_INIT_CHIP_TYPE_HUMBER,
    CTC_INIT_CHIP_TYPE_GOLDENGATE,
    CTC_INIT_CHIP_TYPE_MAX
};
typedef enum ctc_init_chip_type_e ctc_init_chip_type_t;

#define CTC_INIT_MAX_FEATURE_IDX                            20

#if 0
/* SDK init array, 1 means enable, 0 means disable */
static uint8 ctc_init_arr[CTC_INIT_CHIP_TYPE_MAX][CTC_INIT_MAX_FEATURE_IDX] =
{
/*CHIP_NAME*/ /*MIRROR L2_FDB STP STATS ACL IPUC MPLS IPMC LEARNING_AGING APS SECURITY OAM PTP SYNC_ETH INTERRUPT STACKING BPE IPFIX MONITOR OVERLAY*/
{/* GB       */   1,     1,     1,  1,    1,  1,   1,   1,   1,             1,  1,       1,  1,  1,       1,        1,       1,     0,      0,      0},
{/* RAMA   */     1,     1,     1,  1,    1,  1,   1,   1,   1,             1,  1,       1,  1,  1,       1,        1,       0,     0,      0,      0},
{/* RIALTO */     1,     1,     1,  1,    1,  1,   0,   1,   1,             0,  1,       0,  0,  0,       1,        1,       0,     0,      0,      0},
{/* HB       */   1,     1,     1,  1,    1,  1,   1,   1,   1,             1,  1,       1,  1,  1,       1,        1,       0,     0,      0,      0},
{/* GG       */   1,     1,     1,  1,    1,  1,   1,   1,   1,             1,  1,       1,  1,  1,       1,        0,       1,     1,      1,      1}

};
#endif

#define COUNTOF(_array_)    ((uint32)(sizeof((_array_)) / sizeof((_array_)[0])))

/* chip port to phy mapping */
static ctc_phy_info_t g_port_phy_mapping[MAX_CTC_BOARD_TYPE][CTC_MAX_PHY_PORT] =
{
    /* for 24+8 board type */
    {
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff}

    },

    /* for 48+8 board type */
    {
        {   0,    16},  {   0,    17},  {   0,    18},  {   0,    19},
        {   0,     4},  {   0,     5},  {   0,     6},  {   0,     7},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {   1,    16},  {   1,    17},  {   1,    18},  {   1,    19},
        {   1,     4},  {   1,     5},  {   1,     6},  {   1,     7},
        {   0,    12},  {   0,    13},  {   0,    14},  {   0,    15},
        {   0,     0},  {   0,     1},  {   0,     2},  {   0,     3},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {   1,    12},  {   1,    13},  {   1,    14},  {   1,    15},
        {   1,     0},  {   1,     1},  {   1,     2},  {   1,     3},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},
        {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff},  {0xff,  0xff}
    },

    /* for 48+4 board type */
    {
        {    1,   12},  {    1,   13},  {    1,   14},  {    1,   15},
        {    1,    4},  {    1,    5},  {    1,    6},  {    1,    7},
        {    0,    0},  {    0,    1},  {    0,    2},  {    0,    3},
        {    0,    8},  {    0,    9},  {    0,   10},  {    0,   11},
        {    0,   16},  {    0,   17},  {    0,   18},  {    0,   19},
        {    1,   20},  {    1,   21},  {    1,   22},  {    1,   23},
        {    1,   16},  {    1,   17},  {    1,   18},  {    1,   19},
        {    1,    8},  {    1,    9},  {    1,   10},  {    1,   11},
        {    0,    4},  {    0,    5},  {    0,    6},  {    0,    7},
        {    0,   12},  {    0,   13},  {    0,   14},  {    0,   15},
        {    0,   20},  {    0,   21},  {    0,   22},  {    0,   23},
        {    1,    0},  {    1,    1},  {    1,    2},  {    1,    3},
        { 0xff, 0xff},  { 0xff, 0xff},  { 0xff, 0xff},  { 0xff, 0xff},
        { 0xff, 0xff},  { 0xff, 0xff},  { 0xff, 0xff},  { 0xff, 0xff},
        { 0xff, 0xff},  { 0xff, 0xff},  { 0xff, 0xff},  { 0xff, 0xff}
    }
};

ctc_intr_mapping_t hagt_sdk_intr_config[] =
{
    {1,    CTC_INTR_CHIP_FATAL                 },
    {0,    CTC_INTR_CHIP_NORMAL                },
    {INVG,  CTC_INTR_FUNC_PTP_TS_CAPTURE        },
    {INVG,  CTC_INTR_FUNC_PTP_MAC_TX_TS_CAPTURE },
    {INVG,  CTC_INTR_FUNC_PTP_TOD_PULSE_IN      },
    {INVG,  CTC_INTR_FUNC_PTP_TOD_CODE_IN_RDY   },
    {INVG,  CTC_INTR_FUNC_PTP_SYNC_PULSE_IN     },
    {INVG,  CTC_INTR_FUNC_PTP_SYNC_CODE_IN_RDY  },
    {0,       CTC_INTR_FUNC_OAM_DEFECT_CACHE      },
    {INVG,  CTC_INTR_FUNC_MDIO_CHANGE           },
#if defined (GREATBELT) || defined (RAMA) || defined (RIALTO)
    {0,    CTC_INTR_FUNC_MDIO_1G_CHANGE        },
    {0,    CTC_INTR_DMA_NORMAL                 },
#endif
    {INVG, CTC_INTR_FUNC_LINKAGG_FAILOVER      },
    {0,      CTC_INTR_FUNC_IPE_LEARN_CACHE       },
    {0,      CTC_INTR_FUNC_LINK_CHAGNE       },
    {0,      CTC_INTR_FUNC_IPE_AGING_FIFO        },
    {INVG, CTC_INTR_FUNC_STATS_FIFO            },
    {INVG, CTC_INTR_FUNC_APS_FAILOVER          },
    {1,      CTC_INTR_DMA_FUNC                   },
};

ctc_intr_mapping_t hagt_sdk_intr_config_msi[] =
{
    {0,    CTC_INTR_CHIP_FATAL                 },
    {0,    CTC_INTR_CHIP_NORMAL                },
    {INVG,  CTC_INTR_FUNC_PTP_TS_CAPTURE        },
    {INVG,  CTC_INTR_FUNC_PTP_MAC_TX_TS_CAPTURE },
    {INVG,  CTC_INTR_FUNC_PTP_TOD_PULSE_IN      },
    {INVG,  CTC_INTR_FUNC_PTP_TOD_CODE_IN_RDY   },
    {INVG,  CTC_INTR_FUNC_PTP_SYNC_PULSE_IN     },
    {INVG,  CTC_INTR_FUNC_PTP_SYNC_CODE_IN_RDY  },
    {0,       CTC_INTR_FUNC_OAM_DEFECT_CACHE      },
    {INVG,  CTC_INTR_FUNC_MDIO_CHANGE           },
#if defined (GREATBELT) || defined (RAMA) || defined (RIALTO)
    {0,    CTC_INTR_FUNC_MDIO_1G_CHANGE        },
    {0,    CTC_INTR_DMA_NORMAL                 },
#endif
    {INVG, CTC_INTR_FUNC_LINKAGG_FAILOVER      },
    {0,      CTC_INTR_FUNC_LINK_CHAGNE       },
    {0,      CTC_INTR_FUNC_IPE_LEARN_CACHE       },
    {0,      CTC_INTR_FUNC_IPE_AGING_FIFO        },
    {INVG, CTC_INTR_FUNC_STATS_FIFO            },
    {INVG, CTC_INTR_FUNC_APS_FAILOVER          },
    {0,      CTC_INTR_DMA_FUNC                   },
};

/* default interrupt group configuration, need to change the IRQ and priority of groups based on your requirement
 * the priority range is [1, 139] for linux, thereinto [1, 99] is realtime schedule; and [0, 255] for vxworks
 */
ctc_intr_group_t hagt_sdk_intr_group_config[] =
{
    {0, 16, SAL_TASK_PRIO_DEF,  "other interrupt group"},
    {1, 17, 100,                "DMA and abnormal group"}
};

/* default interrupt group configuration, need to change the IRQ and priority of groups based on your requirement
 * the priority range is [1, 139] for linux, thereinto [1, 99] is realtime schedule; and [0, 255] for vxworks
 */
ctc_intr_group_t hagt_sdk_intr_group_config_msi[] =
{
    {0, 0, SAL_TASK_PRIO_DEF,  "msi interrupt group"},
};

#ifdef _GLB_UML_SYSTEM_
extern int32 sram_model_initialize(uint8 chip_id);
extern int32 tcam_model_initialize(uint8 chip_id);
extern int32 sim_model_init();
extern int32 sram_model_allocate_sram_tbl_info(void);
extern int32 sys_goldengate_interrupt_dispatch(void* p_data);
extern int32 cm_com_intr_register_cb(int32 (*)(uint32));

/*added by huangxt in for IP BFD */
extern int32
ctc_oam_cfm_run_error_cache_timer(bool enable);
extern uint32
ctc_oam_cfm_run_ccm(bool enable);
#endif
extern uint32
chip_agent_get_mode();

#if 0
static int32
_hagt_get_port_queue_num(ctc_qos_queue_number_t *p_queue_num)
{
    uint32 queue_num = 8;
    p_queue_num->queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    p_queue_num->queue_number = queue_num;
    return 0;  
}
#endif

bool
hagt_gb_get_feature_enabled(uint32 fea)
{
    if (HAGT_FEA_OAM_ENABLED == fea)
        return g_pst_hagt_master->fea_chk.is_oam_supported;
    else if (HAGT_FEA_IPV6_ENABLED == fea)
        return g_pst_hagt_master->fea_chk.is_ipv6_supported;
    else
        return FALSE;
}

#if 0
static uint32
_hagt_gb_ftm_mapping_key_media(uint32 key_media)
{
    switch(key_media)
    {
        case GLB_STM_INT_TCAM:
            return CTC_FTM_INT_TCAM;

        case GLB_STM_EXT_TCAM:
            return CTC_FTM_EXT_TCAM;

        case GLB_STM_HASH:
            return CTC_FTM_HASH;

        default:
            break;
    }

    return CTC_FTM_EXT_TCAM; /*must have something wrong*/
}

static uint32
_hagt_gb_ftm_mapping_key_id(uint32 key_id)
{
    switch(key_id)
    {

    case GLB_STM_KEY_TYPE_IPV6_ACL0:
        return CTC_FTM_KEY_TYPE_IPV6_ACL0;

    case GLB_STM_KEY_TYPE_IPV6_ACL1:
        return CTC_FTM_KEY_TYPE_IPV6_ACL1;

    case GLB_STM_KEY_TYPE_ACL0:
        return CTC_FTM_KEY_TYPE_ACL0;

    case GLB_STM_KEY_TYPE_ACL1:
        return CTC_FTM_KEY_TYPE_ACL1;

    case GLB_STM_KEY_TYPE_ACL2:
        return CTC_FTM_KEY_TYPE_ACL2;

    case GLB_STM_KEY_TYPE_ACL3:
        return CTC_FTM_KEY_TYPE_ACL3;

    case GLB_STM_KEY_TYPE_IPV4_MCAST:
        return CTC_FTM_KEY_TYPE_IPV4_MCAST;

    case GLB_STM_KEY_TYPE_IPV6_MCAST:
        return CTC_FTM_KEY_TYPE_IPV6_MCAST;

    case GLB_STM_KEY_TYPE_VLAN_SCL:
        return CTC_FTM_KEY_TYPE_VLAN_SCL;
        
    case GLB_STM_KEY_TYPE_MAC_SCL:
        return CTC_FTM_KEY_TYPE_MAC_SCL;

    case GLB_STM_KEY_TYPE_IPV4_SCL:
        return CTC_FTM_KEY_TYPE_IPV4_SCL;

    case GLB_STM_KEY_TYPE_IPV6_SCL:
        return CTC_FTM_KEY_TYPE_IPV6_SCL;

    case GLB_STM_KEY_TYPE_FDB:
        return CTC_FTM_KEY_TYPE_FDB;
        
    case GLB_STM_KEY_TYPE_IPV4_UCAST:
        return CTC_FTM_KEY_TYPE_IPV4_UCAST;
             
    case GLB_STM_KEY_TYPE_IPV6_UCAST:
        return CTC_FTM_KEY_TYPE_IPV6_UCAST;

    case GLB_STM_KEY_TYPE_IPV4_NAT:
        return CTC_FTM_KEY_TYPE_IPV4_NAT;

    case GLB_STM_KEY_TYPE_IPV6_NAT:
        return CTC_FTM_KEY_TYPE_IPV6_NAT;

    case GLB_STM_KEY_TYPE_IPV4_PBR:
        return CTC_FTM_KEY_TYPE_IPV4_PBR;

    case GLB_STM_KEY_TYPE_IPV6_PBR:
        return CTC_FTM_KEY_TYPE_IPV6_PBR;

    case GLB_STM_KEY_TYPE_IPV4_TUNNEL:
        return CTC_FTM_KEY_TYPE_IPV4_TUNNEL;

    case GLB_STM_KEY_TYPE_IPV6_TUNNEL:
        return CTC_FTM_KEY_TYPE_IPV6_TUNNEL;


    default:
        break;
    }

    return CTC_FTM_KEY_TYPE_FDB; /*must have something wrong*/
}

static uint32
_hagt_gb_ftm_mapping_tbl_id(uint32 tbl_id)
{
    switch(tbl_id)
    {
        case GLB_STM_TBL_TYPE_FIB_HASH_KEY:
            return CTC_FTM_TBL_TYPE_FIB_HASH_KEY;
            
        case GLB_STM_TBL_TYPE_SCL_HASH_KEY:
            return CTC_FTM_TBL_TYPE_SCL_HASH_KEY;
            
        case GLB_STM_TBL_TYPE_LPM_HASH_KEY:
            return CTC_FTM_TBL_TYPE_LPM_HASH_KEY;
            
        case GLB_STM_TBL_TYPE_FIB_HASH_AD:
            return CTC_FTM_TBL_TYPE_FIB_HASH_AD;
            
        case GLB_STM_TBL_TYPE_SCL_HASH_AD:
            return CTC_FTM_TBL_TYPE_SCL_HASH_AD;
            
        case GLB_STM_TBL_TYPE_LPM_PIPE1:
            return CTC_FTM_TBL_TYPE_LPM_PIPE1;
            
        case GLB_STM_TBL_TYPE_LPM_PIPE2:
            return CTC_FTM_TBL_TYPE_LPM_PIPE2;
            
        case GLB_STM_TBL_TYPE_LPM_PIPE0:
            return CTC_FTM_TBL_TYPE_LPM_PIPE0;
            
        case GLB_STM_TBL_TYPE_LPM_PIPE3:
            return CTC_FTM_TBL_TYPE_LPM_PIPE3;
            
        case GLB_STM_TBL_TYPE_MPLS:
            return CTC_FTM_TBL_TYPE_MPLS;
            
        case GLB_STM_TBL_TYPE_NEXTHOP:
            return CTC_FTM_TBL_TYPE_NEXTHOP;
            
        case GLB_STM_TBL_TYPE_FWD:
            return CTC_FTM_TBL_TYPE_FWD;
            
        case GLB_STM_TBL_TYPE_MET:
            return CTC_FTM_TBL_TYPE_MET;
            
        case GLB_STM_TBL_TYPE_EDIT:
            return CTC_FTM_TBL_TYPE_EDIT;
            
        case GLB_STM_TBL_TYPE_OAM_MEP:
            return CTC_FTM_TBL_TYPE_OAM_MEP;
            
        case GLB_STM_TBL_TYPE_STATS:
            return CTC_FTM_TBL_TYPE_STATS;
            
        case GLB_STM_TBL_TYPE_LM:
            return CTC_FTM_TBL_TYPE_LM;
    }
    return CTC_FTM_TBL_TYPE_FIB_HASH_KEY; /*must have something wrong*/
}

static int32
_hagt_gb_ftm_profile_mapping(void* pv_in, void* pv_out)
{
    glb_stm_profile_info_t* pst_stm_info = NULL;
    ctc_ftm_profile_info_t* p_profile_info;
    uint32 index;
    uint32 aclv4_block0_number = 0;
    uint32 aclv4_block1_number = 0;
    uint32 aclv6_block0_number = 0;
    uint32 aclv6_block1_number = 0;

    pst_stm_info = (glb_stm_profile_info_t*)pv_in;
    p_profile_info = (ctc_ftm_profile_info_t*)pv_out;
    if (NULL == pst_stm_info || NULL == p_profile_info)
    {
        HAGT_LOG_ERR("Invalid pointer for stm profile mapping\n");
        return HAGT_E_NOT_INIT;
    }
    
    p_profile_info->misc_info.ecmp_number = pst_stm_info->ecmp_number;
    p_profile_info->misc_info.glb_nexthop_number = pst_stm_info->glb_nexthop_number;
    p_profile_info->misc_info.mcast_group_number = pst_stm_info->mcast_group_number;
    p_profile_info->misc_info.vsi_number = pst_stm_info->vsi_number;
    
    g_pst_hagt_master->fea_chk.max_external_nhid = 
        pst_stm_info->glb_nexthop_number +
        pst_stm_info->ecmp_number;

    /*key info*/
    p_profile_info->key_info_size = pst_stm_info->key_info_size;
    for (index = 0; index < pst_stm_info->key_info_size; index++)
    {
        p_profile_info->key_info[index].key_id =
            _hagt_gb_ftm_mapping_key_id(pst_stm_info->key_info[index].key_id);
        p_profile_info->key_info[index].key_size = pst_stm_info->key_info[index].key_size;
        p_profile_info->key_info[index].key_media =
            _hagt_gb_ftm_mapping_key_media(pst_stm_info->key_info[index].key_media);
        p_profile_info->key_info[index].max_key_index = pst_stm_info->key_info[index].max_key_index;
        /*added by ychen in 2013-10-10 for using the max number block to normal acl/qos flow*/
        if (CTC_FTM_KEY_TYPE_IPV6_ACL0 == p_profile_info->key_info[index].key_id)
        {
            aclv6_block0_number = p_profile_info->key_info[index].max_key_index;
            g_pst_hagt_master->fea_chk.is_ipv6_supported = TRUE;
        }
        if (CTC_FTM_KEY_TYPE_IPV6_ACL1 == p_profile_info->key_info[index].key_id)
        {
            aclv6_block1_number = p_profile_info->key_info[index].max_key_index;
            g_pst_hagt_master->fea_chk.is_ipv6_supported = TRUE;
        }
        if (CTC_FTM_KEY_TYPE_ACL0 == p_profile_info->key_info[index].key_id)
        {
            aclv4_block0_number = p_profile_info->key_info[index].max_key_index;           
        }
        if (CTC_FTM_KEY_TYPE_ACL1 == p_profile_info->key_info[index].key_id)
        {
            aclv4_block1_number = p_profile_info->key_info[index].max_key_index;           
        }
    }

    /*table info*/
    p_profile_info->tbl_info_size = pst_stm_info->tbl_info_size;
    for (index = 0; index < pst_stm_info->tbl_info_size; index++)
    {
        p_profile_info->tbl_info[index].tbl_id = 
            _hagt_gb_ftm_mapping_tbl_id(pst_stm_info->tbl_info[index].tbl_id);
        p_profile_info->tbl_info[index].mem_bitmap = pst_stm_info->tbl_info[index].mem_bitmap;
        sal_memcpy(p_profile_info->tbl_info[index].mem_start_offset, pst_stm_info->tbl_info[index].mem_start_offset,
            sizeof(uint32)*GLB_STM_SRAM_MAX);
        sal_memcpy(p_profile_info->tbl_info[index].mem_entry_num, pst_stm_info->tbl_info[index].mem_entry_num,
            sizeof(uint32)*GLB_STM_SRAM_MAX);
        /*added by ychen in 2013-09-04 for feature enabled check*/
        if (CTC_FTM_TBL_TYPE_OAM_MEP == p_profile_info->tbl_info[index].tbl_id)
        {
            g_pst_hagt_master->fea_chk.is_oam_supported = TRUE;
        }
    }

    /*added by ychen in 2013-10-10 for using the max number block to normal acl/qos flow*/
    if (aclv4_block0_number >= aclv4_block1_number)
    {
        g_pst_hagt_master->fea_chk.use_block0_for_aclv4 = 1;
    }
    else
    {
        g_pst_hagt_master->fea_chk.use_block0_for_aclv4 = 0;
    }
    if (aclv6_block0_number >= aclv6_block1_number)
    {
        g_pst_hagt_master->fea_chk.use_block0_for_aclv6 = 1;
    }
    else
    {
        g_pst_hagt_master->fea_chk.use_block0_for_aclv6 = 0;
    }

    return HAGT_E_NONE;
}

int32
hagt_ctc_drv_init(glb_sdk_init_info_t *p_sdk_info)
{
    ctc_ftm_profile_info_t ftm_profile;
    glb_stm_profile_info_t* p_glb_stm_info = NULL;
    glb_chip_info_t* p_chip_info;
    int32 ret = 0;
#ifdef _GLB_UML_SYSTEM_
    int32 chip_id = 0;
#endif 

    p_chip_info = p_sdk_info->g_chip_info;// TODO: Should get chip_info from lcm, commented by xgu, 2013-3-4
    p_glb_stm_info = (glb_stm_profile_info_t *)(p_sdk_info->ftm_profile_info);
    
    /*install api  */
    ret = ctc_install_api();
    if (ret != 0)
    {
        sal_printf("ctc_install_api failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /*~~~~~~~2. Init Chip cfg ~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    ret = ctc_chip_init(p_chip_info->local_chip_num);
    ret = ret ? ret : ctc_set_gchip_id(0, p_chip_info->gchip_0);
    sal_printf("lchip:%d gchip:%d\n", 0, p_chip_info->gchip_0);
    if (ret < 0)
    {
         sal_printf("Failed to init chip num\r\n");
         return ret;
     }
    if (p_chip_info->local_chip_num > 1)
    {
        ctc_set_gchip_id(1, p_chip_info->gchip_1);
        sal_printf("lchip:%d gchip:%d\n", 1, p_chip_info->gchip_1);
    }
    
    /*~~~~~~~3. Init dal~~~~~~~~~~~~~~~~~~~~~~~*/
#if DATAPAH
#ifndef _GLB_UML_SYSTEM_
    ret = ctc_set_port_mapping_table(p_chip_info->port_mapping_table);
    if (ret < 0)
    {
         sal_printf("Failed to init port mapping table\r\n");
         return ret;
    }
#endif
#endif

    ret = dal_set_device_access_type(p_sdk_info->dal_access_type);
    if (ret < 0)
    {
        sal_printf("Failed to register dal callback\r\n");
        return ret;
    }
    
//#ifdef SDK_IN_USERMODE    
    ret = dal_op_init(NULL);
    if (ret != 0)
    {
        sal_printf("Failed to register dal callback\r\n");
        return ret;
    }
//#endif /*SDK_IN_USERMODE*/

    /*~~~~~~~4. Init datatapath~~~~~~~~~~~~~~~~~~~~~~~*/
#ifdef _GLB_UML_SYSTEM_
    for (chip_id = 0; chip_id < p_chip_info->local_chip_num; chip_id++)
    {
        ret = ctc_datapath_sim_init();
    }
#else
    /* data_path_init*/
    ret = ctc_parse_datapath(p_sdk_info->datapath_config_file);
    if (ret != 0)
    {
        sal_printf("ctc_parse_datapath failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    ret = ctc_init_pll_hss();
    if (ret != 0)
    {
        sal_printf("ctc_init_pll_hss failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    ret = ctc_datapath_init(NULL);    
    if (ret != 0)
    {
        sal_printf("ctc_data_path_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }    
#endif

    /*~~~~~~~5. FTM~~~~~~~~~~~~~~~~~~~~~~~*/    
    sal_memset(&ftm_profile, 0, sizeof(ctc_ftm_profile_info_t));
    //ftm_profile.profile_type = 0;
    ftm_profile.profile_type = p_sdk_info->ftm_profile_type;
    if(CTC_FTM_PROFILE_USER_DEFINE == ftm_profile.profile_type)
    {
        ftm_profile.key_info = (ctc_ftm_key_info_t*)HAGT_MALLOC(MEM_FEA_HAGT_MODULE, (CTC_FTM_KEY_TYPE_MAX*sizeof(ctc_ftm_key_info_t)));
        if(NULL == ftm_profile.key_info)
        {
            return HAGT_E_NO_MEMORY;
        }
        ftm_profile.tbl_info = (ctc_ftm_tbl_info_t*)HAGT_MALLOC(MEM_FEA_HAGT_MODULE, (CTC_FTM_TBL_TYPE_MAX*sizeof(ctc_ftm_tbl_info_t)));
        if(NULL == ftm_profile.tbl_info)
        {            
            HAGT_FREE(MEM_FEA_HAGT_MODULE, ftm_profile.key_info);
            return HAGT_E_NO_MEMORY;
        } 
        _hagt_gb_ftm_profile_mapping(p_glb_stm_info, &ftm_profile);
    }
    ret = ctc_ftm_mem_alloc(&ftm_profile);
    if (ret != 0)
    {
        sal_printf("ctc_ftm_alloc failed:%s@%d %s\n",  __FUNCTION__, __LINE__,ctc_get_error_desc(ret));
        return ret;
    }
    if (CTC_FTM_PROFILE_USER_DEFINE == ftm_profile.profile_type)
    {
        HAGT_FREE(MEM_FEA_HAGT_MODULE, ftm_profile.key_info);
        HAGT_FREE(MEM_FEA_HAGT_MODULE, ftm_profile.tbl_info);
    }

    return CTC_E_NONE;
}
#endif

#ifdef _GLB_UML_SYSTEM_
int32
hagt_ctc_sim_model_init(uint8 sim_chip_num)
{
    int ret;
    uint8 lchip;

    /*process only to avoid warning*/
    ret = 0;
    lchip = 0;

#if 0// TODO: no need change topo, just use default topo, should modify default top, 20121123, by xgu
    /* software simulation platform,init  testing swemu environment*/
    ret = swemu_environment_setting(NETWORK_CFG_FILE);
    if (ret != 0)
    {
        sal_printf("swemu_environment_setting failed:%s@%d \n",  __FUNCTION__, __LINE__);
        return ret;
    }
#endif

    for (lchip = 0; lchip < sim_chip_num; lchip ++)
    {
        ret = sram_model_initialize(lchip);
        if (ret != 0)
        {
            sal_printf("sram_model_initialize failed:%s@%d \n",  __FUNCTION__, __LINE__);
            return ret;
        }
        ret = tcam_model_initialize(lchip);
        if (ret != 0)
        {
            sal_printf("tcam_model_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
            return ret;
        }
    }
#ifndef DUET2
    ret  = sram_model_allocate_sram_tbl_info();
    if (ret != 0)
    {
        sal_printf("sram_model_allocate_sram_tbl_info failed:%s@%d %s\n",  __FUNCTION__, __LINE__,ctc_get_error_desc(ret));
        return ret;
    }
#endif
    /* modified by liwh for bug 22127, 2013-02-06,
      refer to ctc_greatbelt_sample_init.c 
      sim_model_init(0), the mode should be UM_EMU_MODE_SDK */
    ret = sim_model_init();
    if (ret != 0)
    {
        sal_printf("sim_model_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
        return ret;
    }

    return ret;
}

int32
hagt_ctc_isr_intr_cb(uint32 irq)
{
    #ifdef GOLDENGATE
    HAGT_IF_ERROR_RETURN(sys_goldengate_interrupt_dispatch(&irq));
    #endif

    return CTC_E_NONE;
}

#endif /* _GLB_UML_SYSTEM_ */

#if 0
int32
hagt_ctc_feature_resource_init(glb_sdk_init_info_t *p_sdk_info)
{
    int32  ret = 0, value = 0;
    ctc_chip_global_cfg_t   chip_global_cfg;
    ctc_nh_global_cfg_t    nh_global_cfg;
    ctc_pkt_global_cfg_t packet_cfg;
    ctc_linkagg_global_cfg_t linkagg_global_cfg;
    ctc_stats_global_cfg_t stats_global_cfg;
    uint8  local_chip_num = 1;
    uint8 index = 0;
    uint8 lchip = 0;

    sal_memset(&nh_global_cfg, 0, sizeof(ctc_nh_global_cfg_t));
    sal_memset(&stats_global_cfg, 0, sizeof(stats_global_cfg));


    ctc_get_local_chip_num(&local_chip_num);
     /*  Global chip config  */

    /*Init cpu mac*/ // TODO: it's better to get the cpu mac from lcm, and lcm will pass this config to peth
    for(lchip = 0; lchip  < local_chip_num ;lchip  ++)
    {
        sal_memset(&chip_global_cfg, 0, sizeof(ctc_chip_global_cfg_t));
        chip_global_cfg.lchip = lchip;
        chip_global_cfg.cpu_mac_sa[0] = 0xFE;
        chip_global_cfg.cpu_mac_sa[1] = 0xFD;
        chip_global_cfg.cpu_mac_sa[2] = 0x0;
        chip_global_cfg.cpu_mac_sa[3] = 0x0;
        chip_global_cfg.cpu_mac_sa[4] = 0x0;
        chip_global_cfg.cpu_mac_sa[5] = 0x1;

        /*init CPU MAC DA*/
        for(index = 0; index  < MAX_CTC_CPU_MACDA_NUM ;index ++)
       {
           chip_global_cfg.cpu_mac_da[index][0]= 0xFE;
           chip_global_cfg.cpu_mac_da[index][1]= 0xFD;
           chip_global_cfg.cpu_mac_da[index][2]= 0x00;
           chip_global_cfg.cpu_mac_da[index][3]= 0x00;
           chip_global_cfg.cpu_mac_da[index][4]= 0x00;
           chip_global_cfg.cpu_mac_da[index][5]= 0x00;

        }    
        chip_global_cfg.tpid = 0x5a5a;
        chip_global_cfg.vlanid = 0;
        ctc_set_chip_global_cfg(&chip_global_cfg);

    }

    /*   initializate common table/register */
    ret =  ctc_register_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_register_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }
    /*the following is resource module*/

    value = 0;
    CTC_ERROR_RETURN(ctc_global_ctl_set(CTC_GLOBAL_DISCARD_SAME_MACDASA_PKT, &value));
    CTC_ERROR_RETURN(ctc_global_ctl_set(CTC_GLOBAL_DISCARD_SAME_IPDASA_PKT, &value));

    nh_global_cfg.max_external_nhid = g_pst_hagt_master->fea_chk.max_external_nhid;
    nh_global_cfg.acl_redirect_fwd_ptr_num = 1024;// TODO: tmp value, need consider later
    nh_global_cfg.max_ecmp  = CTC_DEFAULT_MAX_ECMP_NUM;
    nh_global_cfg.nh_edit_mode= 0;// TODO:  1 stand for nexthop egress edit mode;

    /* nexthop  */ /*CAUTION!! Nexthop module should be init before QoS module, 
                            because QoS module will get fatal exception's nh offset , 
                            and this offset is inited in nexthop module, commented by xgu, 2012-12-26*/
    ret = ctc_nexthop_init(&nh_global_cfg);
    if (ret != 0)
    {
        sal_printf("ctc_nexthop_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }
    
#if 0 // TODO: Commented by xgu, 2012-12-8
    /*the following is resource module*/
    /*   interrupt start */
    ret = ctc_interrupt_reg_init_start();
    if (ret != 0)
    {
        sal_printf("ctc_interrupt_reg_init_start failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* qos init */
    if(p_chip_info->queue_num_per_network_port)
    {
        qos_global_cfg.queue_num_per_network_port = p_chip_info->queue_num_per_network_port;
        qos_global_cfg.queue_num_per_static_int_port = p_chip_info->queue_num_per_static_int_port;
        qos_global_cfg.queue_num_per_fabric = p_chip_info->queue_num_per_fabric;
        qos_global_cfg.max_internal_port_id = p_chip_info->max_internal_port_id;
        qos_global_cfg.queue_num_per_internal_port = p_chip_info->queue_num_per_internal_port;
        qos_global_cfg.service_num_with_queue = p_chip_info->service_num;
        qos_global_cfg.queue_num_per_ingress_service = p_chip_info->queue_num_per_ingress_service;
        qos_global_cfg.queue_num_per_egress_service = p_chip_info->queue_num_per_egress_service;
    }
    else
#endif      
/* modified by cuixl for bug 22441, 2013-03-19 */
/*modification start*/
// TODO:now we init port queue number, other type queue(cpu, exception, service) follow this
/* Now sdk qos does not support input parameters */
    ctc_qos_queue_number_t queue_num = { 0 };
    _hagt_get_port_queue_num(&queue_num);
//    ret = ctc_qos_init(&queue_num);
/*modification end*/

    ret = ctc_qos_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_qos_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /*    SCL */
    ret = ctc_scl_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_scl_init failed: %s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /*    VLAN  */
    {
        ctc_vlan_global_cfg_t vlan_global_cfg;
        sal_memset(&vlan_global_cfg,0,sizeof(ctc_vlan_global_cfg_t));
        vlan_global_cfg.vlanptr_mode = CTC_VLANPTR_MODE_VLANID;
        ret =  ctc_vlan_init(&vlan_global_cfg);
        if (ret != 0)
        {
            sal_printf("ctc_vlan_init failed: %s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }
    ret = ctc_l3if_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_l3if_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }
    /*parser*/
    ret = ctc_parser_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_parser_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /*    port  */
    ret = ctc_port_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_port_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    ret = ctc_internal_port_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_internal_port_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }


    linkagg_global_cfg.linkagg_mode = CTC_LINKAGG_MODE_32;
    /* Linkagg */
    ret = ctc_linkagg_init(&linkagg_global_cfg);
    if (ret != 0)
    {
        sal_printf("ctc_linkagg_init failed: %s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    ret = ctc_pdu_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_pdu_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

extern int32
umpeth_rx_skb_proc(ctc_pkt_rx_t* p_pkt_rx);

    packet_cfg.rx_cb = umpeth_rx_skb_proc;
    ret = ctc_packet_init(&packet_cfg);
    if (ret != 0)
    {
        sal_printf("ctc_packet_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* stats init */
    stats_global_cfg.stats_bitmap = CTC_STATS_QUEUE_DEQ_STATS
                            | CTC_STATS_QUEUE_DROP_STATS
                            | CTC_STATS_PORT_POLICER_STATS
                            | CTC_STATS_SERVICE_POLICER_STATS
                            | CTC_STATS_FLOW_POLICER_STATS
                            | CTC_STATS_FLOW_STATS;
    /*modify by chenxw for bug 22441, 2013-07-22.*/
    stats_global_cfg.policer_stats_num = 1024;
    ret = ctc_stats_init(&stats_global_cfg);
    if (ret != 0)
    {
        sal_printf("ctc_stats_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    return 0;
}
#endif

#ifndef _GLB_UML_SYSTEM_   /* added by qicx for e350 system, bug 23786, cr7145, 2013-07-12 */
#define COUNTOF(_array_)    ((uint32)(sizeof((_array_)) / sizeof((_array_)[0])))

/* default interrupt configuration */
ctc_intr_mapping_t ctc_intr_config[] =
{
    
};

/* default interrupt group configuration, need to change the IRQ and priority of groups based on your requirement
 * the priority range is [1, 139] for linux, thereinto [1, 99] is realtime schedule; and [0, 255] for vxworks
 */
 /*fix bug28274, GB INTR, liangf, 2014-06-04*/
#ifdef _CTC_LOONGSON_2F_
ctc_intr_group_t ctc_intr_group_config[] =
{
    {0, 2, SAL_TASK_PRIO_DEF,  "Function normal group"},
    {1, 3, 100,                "Function critical group"},
    {-1, 18, SAL_TASK_PRIO_DEF,  "Function DMA group"},
    {-1, 19, SAL_TASK_PRIO_DEF,  "Chip abnormal group"}
};
#else
ctc_intr_group_t ctc_intr_group_config[] =
{
    {0, 16, SAL_TASK_PRIO_DEF,  "Function normal group"},
    {1, 17, 100,                "Function critical group"},
    /* Modified by liuht for bug 27657, 2014-03-25 */
    /* This irq is assigned to dying gasp */
    {-1, 18, SAL_TASK_PRIO_DEF,  "Function DMA group"},
    {-1, 19, SAL_TASK_PRIO_DEF,  "Chip abnormal group"}
};
#endif


/* get interrupt configuration */
int32
ctc_isr_get_intr_cfg(ctc_intr_global_cfg_t* p_intr_cfg)
{
    uint32 group_count = 0;
    uint32 intr_count = 0;

    /* use default global configuration in sys */
    group_count = COUNTOF(ctc_intr_group_config);
    intr_count = COUNTOF(ctc_intr_config);
    if ((group_count > CTC_INTR_MAX_GROUP) || (group_count > CTC_INTR_GB_MAX_GROUP))
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((intr_count > CTC_INTR_MAX_INTR) || (intr_count > CTC_INTR_GB_MAX_INTR))
    {
        return CTC_E_INVALID_PARAM;
    }

    p_intr_cfg->group_count = group_count;
    p_intr_cfg->intr_count = intr_count;
    p_intr_cfg->p_group = ctc_intr_group_config;
    p_intr_cfg->p_intr = ctc_intr_config;
    //sal_memcpy(&(p_intr_cfg->group), ctc_intr_group_config, sizeof(ctc_intr_group_config));
    //sal_memcpy(&(p_intr_cfg->intr), ctc_intr_config, sizeof(ctc_intr_config));

    return CTC_E_NONE;
}
#endif  /* end of _GLB_UML_SYSTEM_ , added by qicx, 2013-07-12 */

#if 0
int32
hagt_ctc_feature_selective_init(glb_sdk_init_info_t *p_sdk_info)
{
    int32  ret = 0;
    ctc_oam_global_t  oam_global;
    ctc_l2_fdb_global_cfg_t l2_fdb_global_cfg;
    ctc_bpe_global_cfg_t bpe_global_cfg;
    ctc_ptp_global_config_t ptp_global_cfg;
    ctc_acl_global_cfg_t acl_global_cfg;
    ctc_dma_global_cfg_t dma_gbl_cfg;
#ifndef _GLB_UML_SYSTEM_ /* added by qicx, use configurable intr cfg, not the default cfg, 2013-07-04 */
    ctc_intr_global_cfg_t intr_cfg;
    void* p_intr_cfg = NULL;
#endif

    ctc_oam_property_t oam_prop;
    mac_addr_t bridge_mac;

    //sal_memset(&oam_global,0,sizeof(oam_global));
    sal_memset(&l2_fdb_global_cfg, 0, sizeof(l2_fdb_global_cfg));
    sal_memset(&ptp_global_cfg, 0, sizeof(ptp_global_cfg));
    sal_memset(&acl_global_cfg, 0, sizeof(acl_global_cfg));
    sal_memset(&dma_gbl_cfg, 0, sizeof(ctc_dma_global_cfg_t));
    sal_memset(&bpe_global_cfg, 0, sizeof(ctc_bpe_global_cfg_t));

    /*mirror*/
    ret = ctc_mirror_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_mirror_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }


    l2_fdb_global_cfg.logic_port_num = p_sdk_info->g_chip_info->logic_port_num;
    l2_fdb_global_cfg.flush_fdb_cnt_per_loop = 0;
    l2_fdb_global_cfg.default_entry_rsv_num = 4 * 1024 + l2_fdb_global_cfg.logic_port_num;
    l2_fdb_global_cfg.hw_learn_en = p_sdk_info->g_chip_info->fdb_hw_learning_mode;
    ret = ctc_l2_fdb_init(&l2_fdb_global_cfg);
    if (ret != 0)
    {
        sal_printf("ctc_l2_fdb_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* stp init */
    ret = ctc_stp_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_stp_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

#if 0// TODO: Commented by xgu, 2012-12-8
    acl_global_cfg.acl_redirect_fwd_ptr_num = ACL_REDIRECT_FWD_PTR_NUM;
    acl_global_cfg.global_aclqos_entry_head_num = 10;
    acl_global_cfg.global_aclqos_entry_tail_num = 20;
    acl_global_cfg.global_pbr_entry_head_num = 10;
    acl_global_cfg.global_pbr_entry_tail_num = 20;
#endif
    acl_global_cfg.acl_redirect_num = 1024;
    acl_global_cfg.merge_mac_ip = 1;
    acl_global_cfg.priority_bitmap_of_stats = 3;
    /*modify by chenxw for bug 24076, 2013-09-04.*/    
    acl_global_cfg.ingress_use_mapped_vlan = 0;
    ret = ctc_acl_init(&acl_global_cfg);
    if (ret != 0)
    {
        sal_printf("ctc_acl_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    ret = ctc_ipuc_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_ipuc_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

#if 0
    /**mpls init*/
    /**Support 256 mpls interface space, space 0 used as global, space 1-255 for interface. */
    /**User Must set MPLS Profile, at the same time, allocate the interface space by yourself. */
    /**e.g  MPLS_Profile=3K, space0=2K, space1=512,space2=512, allocate as follows:  */
    /**Warning: SDK only support  enum ctc_mpls_label_size_type_e as interface space param!!!*/
    {
        ctc_mpls_init_t mpls_init;
        uint16 mpls_size=  3*1024;
        sal_memset(&mpls_init, 0, sizeof(ctc_mpls_init_t));

        ret = ctc_sample_mpls_init_helper(0,CTC_MPLS_LABEL_NUM_2K_TYPE,mpls_size,&mpls_init);
        ret+= ctc_sample_mpls_init_helper(1,CTC_MPLS_LABEL_NUM_512_TYPE,mpls_size,&mpls_init);
        ret+= ctc_sample_mpls_init_helper(2,CTC_MPLS_LABEL_NUM_512_TYPE,mpls_size,&mpls_init);
        if (ret != 0)
        {
            sal_printf("MPLS SPACE SIZE ALLOC FAILED:%s@%d %s\n", __FUNCTION__, __LINE__, "alloc mpls label space error!!!");
            return ret;
        }

        ret = ctc_mpls_init(&mpls_init);

    }
#endif

    ret = ctc_mpls_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_mpls_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    ret = ctc_ipmc_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_ipmc_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }
    {
        ctc_learn_aging_global_cfg_t cfg;
        sal_memset(&cfg, 0, sizeof(cfg));
        cfg.hw_mac_aging_en = 1;
        cfg.hw_mac_learn_en = 1;
        cfg.scl_aging_en    = 0;
        cfg.hw_scl_aging_en = 0;
        ret = ctc_learning_aging_init((void *)&cfg);
        if (ret != 0)
        {
            sal_printf("ctc_learning_aging_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }
    ret = ctc_aps_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_aps_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    ret = ctc_security_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_security_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /*com oam global param*/
#if 0
    oam_global.common_global.oam_excp_bitmap_to_cpu            = 0xFFFFFFFF;
    oam_global.common_global.trig_aps_msg_flag_rmep = CTC_OAM_TRIG_APS_MSG_FLAG_RMEP_DLOC;
    oam_global.common_global.trig_aps_msg_flag_mep  = 0;
#endif
    /*eth oam global param*/
#if 0
    CTC_SET_FLAG(oam_global.eth_global.flag, CTC_ETH_OAM_GLOBAL_SLOW_OAM_TO_CPU);
#endif
    sal_memset(&oam_global, 0 , sizeof(ctc_oam_global_t));
    oam_global.is_p2p_mode                  = 1;

    /* modified by liwh for bug 24752, 2013-09-29 
      maid_len_format should be decided by hsrv/hagt */
    oam_global.maid_len_format              = 0;  
    
    oam_global.mep_index_alloc_by_sdk       = 1;
    oam_global.tp_section_oam_based_l3if    = 0;
    oam_global.tp_y1731_ach_chan_type       = 0x8902;
#ifdef HAVE_BFD
    /*modified by ychen in 2013-11-04 for bug 24232*/
    oam_global.mep_1ms_num                  = p_sdk_info->mep_1ms_num;
    oam_global.tp_bfd_333ms                 = 0;
#endif
    ret = ctc_oam_init(&oam_global);
    if (ret != 0)
    {
        sal_printf("ctc_eth_oam_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    bridge_mac[0] = 0x0;
    bridge_mac[1] = 0x11;
    bridge_mac[2] = 0x11;
    bridge_mac[3] = 0x11;
    bridge_mac[4] = 0x11;
    bridge_mac[5] = 0x11;
    sal_memset(&oam_prop, 0 ,sizeof(ctc_oam_property_t));
    oam_prop.oam_pro_type       = CTC_OAM_PROPERTY_TYPE_Y1731;
    oam_prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_BRIDGE_MAC;
    oam_prop.u.y1731.p_value  = &bridge_mac;
    ret = ctc_oam_set_property(&oam_prop);
    if ( (0 != ret) && (CTC_E_OAM_NOT_INIT != ret))
    {
        sal_printf("ctc_oam_set_property failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /* for some board like e350 8t12xg don't support ptp, added by jcao for bug 25500, 2013-10-25 */
    if(p_sdk_info->ptp_en)
    {        
        /**ptp init*/
        ptp_global_cfg.ptp_ucast_en = 1;
        ptp_global_cfg.sync_copy_to_cpu_en = 0;
        ptp_global_cfg.signaling_copy_to_cpu_en = 0;
        ptp_global_cfg.management_copy_to_cpu_en = 0;
        ptp_global_cfg.delay_request_process_en = 1;
        ptp_global_cfg.port_based_ptp_en = 1;
        ptp_global_cfg.cache_aging_time = 60;
    
        ret = ctc_ptp_init(&ptp_global_cfg);
        if (ret != 0)
        {
            sal_printf("ctc_ptp_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }
        
    /* SyncE init*/
    ret = ctc_sync_ether_init(NULL);
    if (ret != 0)
    {
        sal_printf("ctc_sync_ether_init failed:%s@%d %s\n", __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    /*   interrupt end */
#ifdef _GLB_UML_SYSTEM_
    ret = ctc_interrupt_init(NULL);
#else
    /* added by qicx, use configurable intr cfg, not the default cfg, 2013-07-04 */
    if(GLB_SERIES_GREATBELT_DEMO == p_sdk_info->board_type.series)
    {  // TODO: can not distinguish board_type in HAGT, should find out another flag, qicx, 2013-07-08
        switch (p_sdk_info->board_type.type)
        {
            case GLB_BOARD_GREATBELT_DEMO:
                p_intr_cfg = NULL;  /* use default configuration */
                break;
            default:
                sal_printf("ctc_isr_get_intr_cfg failed: Unsupport board series %d type %d",
                    p_sdk_info->board_type.series, p_sdk_info->board_type.type);
                return HAGT_E_INVALID_BOARD;
        }
    }
    else if(GLB_SERIES_E350 == p_sdk_info->board_type.series)
    {
        switch (p_sdk_info->board_type.type)
        {
            case GLB_BOARD_E350_48T4X2Q:
            case GLB_BOARD_E350_48T4XG:
            case GLB_BOARD_V350_48P4X:
            case GLB_BOARD_V350_24P4X:
            case GLB_BOARD_E350_8T12XG:  
	     case GLB_BOARD_E350_24T4XG:			
                ctc_isr_get_intr_cfg(&intr_cfg);
                p_intr_cfg = (void*)&intr_cfg;
                break;
            default:
                sal_printf("ctc_isr_get_intr_cfg failed: Unsupport board series %d type %d",
                    p_sdk_info->board_type.series, p_sdk_info->board_type.type);
                return HAGT_E_INVALID_BOARD;
        }
    }
    else
    {
        sal_printf("ctc_isr_get_intr_cfg failed: Unsupport board series %d type %d",
            p_sdk_info->board_type.series, p_sdk_info->board_type.type);
        return HAGT_E_INVALID_BOARD;
    }
    
    ret = ctc_interrupt_init(p_intr_cfg);
#endif
    if (ret != 0)
    {
        sal_printf("ctc_interrupt_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }

    if(p_sdk_info->stacking_en)
    {
        ret = ctc_stacking_init(NULL);
        if (ret != 0)
        {
            sal_printf("ctc_stacking_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }
#if 0 // TODO: Commented by xgu, for compile pass, 2012-12-8

    /*   interrupt end */
    ret = ctc_interrupt_reg_init_end();
    if (ret != 0)
    {
        sal_printf("ctc_interrupt_reg_init_end failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }
#endif

#ifdef HAVE_DMA
    /* init dma */
#ifdef GB_TODO
    dma_gbl_cfg.stats.desc_num          = 16;
    dma_gbl_cfg.stats.priority          = SAL_TASK_PRIO_DEF;
#endif
    dma_gbl_cfg.learning.desc_num       = 1024;
    dma_gbl_cfg.learning.priority       = SAL_TASK_PRIO_DEF;
    dma_gbl_cfg.pkt_rx[0].desc_num      = 512;
    dma_gbl_cfg.pkt_rx[0].priority      = SAL_TASK_PRIO_NICE_HIGH;
    dma_gbl_cfg.pkt_rx[1].desc_num      = 64;
    dma_gbl_cfg.pkt_rx[1].priority      = SAL_TASK_PRIO_DEF;
    dma_gbl_cfg.pkt_rx[2].desc_num      = 64;
    dma_gbl_cfg.pkt_rx[2].priority      = SAL_TASK_PRIO_DEF;
    dma_gbl_cfg.pkt_rx[3].desc_num      = 64;
    dma_gbl_cfg.pkt_rx[3].priority      = SAL_TASK_PRIO_DEF;
    dma_gbl_cfg.pkt_rx_chan_num         = 4;
    dma_gbl_cfg.pkt_tx_desc_num         = 16;
    dma_gbl_cfg.table_r_desc_num        = 1;
    dma_gbl_cfg.table_w_desc_num        = 1;
#ifdef _GLB_UML_SYSTEM_
    dma_gbl_cfg.pkt_rx_size_per_desc    = 256 * 8;
#else
    dma_gbl_cfg.pkt_rx_size_per_desc    = 256 * 40;
#endif
    dma_gbl_cfg.func_en_bitmap          = 0;
    dma_gbl_cfg.learning_proc_func      = hagt_bridge_dma_learning;
    CTC_BIT_SET(dma_gbl_cfg.func_en_bitmap, CTC_DMA_FUNC_PACKET_RX);
    CTC_BIT_SET(dma_gbl_cfg.func_en_bitmap, CTC_DMA_FUNC_PACKET_TX);
    CTC_BIT_SET(dma_gbl_cfg.func_en_bitmap, CTC_DMA_FUNC_TABLE_W);
    CTC_BIT_SET(dma_gbl_cfg.func_en_bitmap, CTC_DMA_FUNC_TABLE_R);
    CTC_BIT_SET(dma_gbl_cfg.func_en_bitmap, CTC_DMA_FUNC_HW_LEARNING);
    ret = ctc_dma_init(&dma_gbl_cfg);
    if (ret != 0)
    {
        sal_printf("ctc_dma_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }
#endif

    bpe_global_cfg.intlk_mode = CTC_BPE_INTLK_PKT_MODE;
    ret = ctc_bpe_init(&bpe_global_cfg);
    if (ret != 0)
    {
        sal_printf("ctc_bpe_init failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
        return ret;
    }
    return ret;
}
#endif

int32
hagt_ctc_feature_timer_init(void)
{
#ifdef GG_TODO
#ifdef _GLB_UML_SYSTEM_
    CTC_ERROR_RETURN(ctc_oam_cfm_run_error_cache_timer(TRUE));
    CTC_ERROR_RETURN(ctc_oam_cfm_run_ccm(TRUE));
#endif
#endif
    return HAGT_E_NONE;
}

#if 0
int32
hagt_ctc_feature_init(glb_sdk_init_info_t *p_sdk_info)
{
    /*init resource module*/
    CTC_ERROR_RETURN(hagt_ctc_feature_resource_init(p_sdk_info));

    /*init selective module*/
    CTC_ERROR_RETURN(hagt_ctc_feature_selective_init(p_sdk_info));
    
    /* EADP client need not start Sim model timer */
    if (1 != chip_agent_get_mode()) /* CHIP_AGT_MODE_CLIENT */
    {
        /*init model timer */
        CTC_ERROR_RETURN(hagt_ctc_feature_timer_init());
    }

    return 0;
}
#endif

/* for bug 35982 added by liuyang 2015-11-4*/
ctc_init_chip_info_t g_chip_info;

int32
hagt_sdk_get_chip_profile(uint8* fname, ctc_init_cfg_t * p_init_config)
{
    /*set chip profile default */
    g_chip_info.local_chip_num = 1;
    g_chip_info.gchip[0] = 0;
#if 0    
enum ctc_board_type
{
    CTC_BOARD_24PLUS8,
    CTC_BOARD_48PLUS8,
    CTC_BOARD_48PLUS4,

    MAX_CTC_BOARD_TYPE
};
typedef enum ctc_board_type ctc_board_type_e;
#endif    
    g_chip_info.board_type = 0;// CTC_BOARD_24PLUS8
    g_chip_info.port_mapping_en = 1;
    g_chip_info.port_phy_mapping_en = 0;
    g_chip_info.interrupt_mode = 0;
    g_chip_info.profile_type = 0;
    g_chip_info.nh_dedit_mode = 0; /* modified by yaom for mpls nexthop egress edit mode 20150519 */
    g_chip_info.ext_nexthop_num = 32768;/*huangxt for bug 34224*/
    g_chip_info.mpls_tunnel_num = 32768; // ZDY: openflow mpls l2vpn will use this as tunnel id limit.
    g_chip_info.mpls_entry_num[0] = 1024;
    g_chip_info.mpls_entry_num[1] = 1024;
    g_chip_info.mpls_entry_num[2] = 1024;
    g_chip_info.mpls_entry_num[3] = 1024;
    g_chip_info.fdb_hw_learning_mode = 0;
    g_chip_info.logic_port_num = 1216; /* for bug 32607 modified by liuyang 2015-4-7*/
    g_chip_info.stats_queue_deq_en = 1;
    g_chip_info.stats_queue_drop_en = 1;
    g_chip_info.stats_flow_policer_en = 1;
    g_chip_info.stats_vlan_en = 1;
    g_chip_info.stats_vrf_en = 1;
    g_chip_info.stats_policer_num = 512;
    g_chip_info.cut_through_speed = 0;
    g_chip_info.bfd_mep_num = 2002;
    g_chip_info.bfd_vccv_with_ip_en = 1;

    return HAGT_E_NONE;
}

int32
hagt_sdk_get_port_map(uint8* fname, ctc_init_chip_info_t* p_chip_info,ctc_chip_phy_mapping_para_t * p_phy_mapping_para)
{
    int32 index;
    for (index = 0; index < CTC_MAX_PHY_PORT; index++)
    {
        p_phy_mapping_para->port_mdio_mapping_tbl[index] = g_port_phy_mapping[p_chip_info->board_type][index].mdio_bus;
        p_phy_mapping_para->port_phy_mapping_tbl[index]  = g_port_phy_mapping[p_chip_info->board_type][index].phy_addr;
    }
    return 0;
}

/**
 @brief get interrupt configuration
*/
#ifdef V580_OPENFLOW
static int32
_hagt_sdk_get_intr_cfg(ctc_intr_global_cfg_t* p_intr_cfg, uint8 interrupt_mode)
{
    uint32 group_count = 0;
    uint32 intr_count = 0;

    /* use default global configuration in sys */
    group_count = COUNTOF(hagt_sdk_intr_group_config);
    intr_count = COUNTOF(hagt_sdk_intr_config);
    if ((group_count > CTC_INTR_MAX_GROUP) || (group_count > CTC_INTR_MAX_GROUP))
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((intr_count > CTC_INTR_MAX_INTR) || (intr_count > CTC_INTR_MAX_INTR))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (interrupt_mode)
    {
        p_intr_cfg->group_count = group_count;
        p_intr_cfg->intr_count = intr_count;
        p_intr_cfg->p_group = hagt_sdk_intr_group_config_msi;
        p_intr_cfg->p_intr = hagt_sdk_intr_config_msi;
        p_intr_cfg->intr_mode = 1;
    }
    else
    {
        p_intr_cfg->group_count = group_count;
        p_intr_cfg->intr_count = intr_count;
        p_intr_cfg->p_group = hagt_sdk_intr_group_config;
        p_intr_cfg->p_intr = hagt_sdk_intr_config;
        p_intr_cfg->intr_mode = 0;
    }
    return CTC_E_NONE;
}

static int32
_hagt_sdk_get_dma_cfg(ctc_dma_global_cfg_t* p_dma_cfg)
{
    return CTC_E_NONE;
}
#endif
int32
hagt_packet_rx_proc_isr(ctc_pkt_rx_t* p_pkt_rx)
{
    return CTC_E_NONE;    
}

int32
hagt_sdk_get_config(ctc_init_cfg_t * p_init_config, void *arg)
{   
    return CTC_E_NONE;
}

int32
hagt_sdk_init(void * p_global_cfg, void *p_sdk_init_info)
{
    return CTC_E_NONE;
}

int32
hagt_goldengate_sdk_init(void* arg)
{
    return CTC_E_NONE;
}

uint16 hagt_get_logic_slotid()
{
    return g_pst_hagt_master->logic_slot_no;
}

void hagt_set_logic_slotid(uint16 logic_slot)
{
    g_pst_hagt_master->logic_slot_no = logic_slot;
    return;
}

uint16 hagt_get_phy_slotid()
{
    return g_pst_hagt_master->phy_slot_no;
}

void hagt_set_phy_slotid(uint16 phy_slot)
{
    g_pst_hagt_master->phy_slot_no = phy_slot;
    return;
}

void hagt_set_gchip_id(uint32 gchip_id)
{
    g_pst_hagt_master->gchip_id = gchip_id;
    return;
}

uint32 hagt_get_gchip_id()
{
    return g_pst_hagt_master->gchip_id;
}

int32
hagt_is_enable_stack()
{
    static int enable = -1;
    struct stat stat_buf;

    if (enable != -1)
    {
        return enable;
    }

    enable = !stat(GLB_STACK_ENABLE_FILE, &stat_buf);
    return enable;
}

int32
hagt2lcm_set_callback(hagt2lcm_cb_msg_type_t msg_type, hagt2lcm_callback_t func)
{
    if(msg_type >= HAGT2LCM_CB_TYPE_MAX)
        return HAGT_E_INVALID_PARAM;

    hagt2lcm_cb[msg_type] = func;

    return HAGT_E_NONE;
}

int32
hagt_modules_init(void *arg)
{

    hagt_stats_init(g_pst_hagt_master);
//    hagt_oam_efm_init(g_pst_hagt_master);
//    hagt_if_init (g_pst_hagt_master);
    //hagt_vrrp_init (g_pst_hagt_master);
    /*2.0 Init debug db*/
    hagt_debug_init();
    /*2.7 Init lacp*/
    hagt_agg_init(g_pst_hagt_master);

    hagt_qos_init(g_pst_hagt_master);
    hagt_sflow_init(g_pst_hagt_master);

        /*Support log abnormal int, bug25780. 2013-11-12*/
    hagt_abnormal_int_init();


    hagt_openflow_init(g_pst_hagt_master);

    return HAGT_E_NONE;
}

int32
hagt_modules_start()
{
    hagt_master_t *p_tmp_hagt = NULL;

    /*1. init hagt db*/
    p_tmp_hagt = HAGT_MALLOC(MEM_FEA_HAGT_MODULE, sizeof(hagt_master_t));
    if (NULL == p_tmp_hagt)
    {
        return HAGT_E_NO_MEMORY;
    }
    sal_memset(p_tmp_hagt, 0, sizeof(hagt_master_t));
    g_pst_hagt_master = p_tmp_hagt;

    /*1.1 Init hagt thread master*/
    /*1.2 Init message module db*/
    HAGT_IF_ERROR_RETURN(hagt_message_module_start(&(p_tmp_hagt->p_messge_info)));
    /*1.3 Init other hagt module db*/

    hagt_modules_init(NULL);
    return HAGT_E_NONE;
}

