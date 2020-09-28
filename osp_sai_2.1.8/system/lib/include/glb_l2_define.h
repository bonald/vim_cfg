#ifndef __GLB_L2_DEFINE_H__
#define __GLB_L2_DEFINE_H__

/****************************************************************************
 * glb_l2_define.h :  Macro, data structure for global used layer struct, macro, flags, etc;
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision :   R0.01
 * Author   :   xgu
 * Date     :   2009-2-7
 ****************************************************************************/
#include "glb_const.h"

#define GLB_BR_PORT_STATE_DISABLED         0
#define GLB_BR_PORT_STATE_LISTENING        1
#define GLB_BR_PORT_STATE_LEARNING         2
#define GLB_BR_PORT_STATE_FORWARDING       3
#define GLB_BR_PORT_STATE_BLOCKING         4
#define GLB_BR_PORT_STATE_MAX              5

#define GLB_FDB_FLAG_STATIC               (1 << 0)
#define GLB_FDB_FLAG_SRC_DISC_EXCEPT      (1 << 1)
#define GLB_FDB_FLAG_COPY_TOCPU           (1 << 2)
#define GLB_FDB_FLAG_DISC                 (1 << 3)
#define GLB_FDB_FLAG_KP_EMPTY_MCAST_ENT   (1 << 4)  /* when a L2 Mcast entry has no any members,
                                                       still keep the entry in ASIC so that all 
                                                          the matched traffic will be dropped */ 
#define GLB_FDB_FLAG_FWD_TOCPU            (1 << 5)
#define GLB_FDB_FLAG_EGS_VLAN_TRANS       (1 << 6)
#define GLB_FDB_FLAG_MAC_SECURITY_EN      (1 << 7)
#define GLB_FDB_FLAG_INVALID              (1 << 8) /* only use for static arp on vlan interface, no out port */
#define GLB_FDB_FLAG_INVALID_STATIC       (1 << 9) /*for bug 25344, GLB_FDB_FLAG_INVALID+GLB_FDB_FLAG_STATIC */
/* modify for vxlan by chenxw, 2014-05-29. */
#define GLB_FDB_FLAG_OVERLAY_LOGIC_PORT   (1 << 10) /**<[GG] fdb is learn by logic port */    
#define GLB_FDB_FLAG_MLAG                 (1 << 11) /* add by dingyi for mlag, 2014-08. */
#define GLB_FDB_FLAG_MLAG_SYNC_OUT        (1 << 12) /* add by dingyi for mlag, 2014-08 */
#define GLB_FDB_FLAG_MLAG_SYNC_IN         (1 << 13) /* add by dingyi for mlag, 2014-08 */
#define GLB_FDB_FLAG_ARP                  (1 << 14) 
#define GLB_FDB_FLAG_HYBRID_FLOW          (1 << 15) 

#define GLB_VLAN_MAP_TYPE_NONE             (1 << 0) 
#define GLB_VLAN_MAP_TYPE_BASIC_QINQ       (1 << 1)
#define GLB_VLAN_MAP_TYPE_SELECTIVE_QINQ   (1 << 2)
#define GLB_VLAN_MAP_TYPE_VLAN_REPLACE     (1 << 3)

#define GLB_VLAN_CLASS_MAC_BASED            0x1
#define GLB_VLAN_CLASS_PROTOCOL_BASED       0x2
#define GLB_VLAN_CLASS_IP_BASED             0x4

#define GLB_VLAN_CLASS_RULE_ACTION_ADD      0x1
#define GLB_VLAN_CLASS_RULE_ACTION_DEL      0X2

#define GLB_VLAN_OP_ACTION_ADD              0x1
#define GLB_VLAN_OP_ACTION_REMOVE           0X2

/* Filter type */
#define GLB_VLAN_CLASSIFIER_MAC       1  /* filter on source MAC */
#define GLB_VLAN_CLASSIFIER_PROTOCOL  2  /* filter on protocol */
#define GLB_VLAN_CLASSIFIER_IPV4      4  /* filter on src IPv4 subnet */
#define GLB_VLAN_CLASSIFIER_IPV6      8  /* filter on src IPv6 subnet */

#define GLB_IP_BINDING_IP        (1 << 0)
#define GLB_IP_BINDING_IPV6      (1 << 1)
#define GLB_IP_BINDING_PORT      (1 << 2)
#define GLB_IP_BINDING_MAC       (1 << 3)
#define GLB_IP_BINDING_VLAN      (1 << 4)
/*added by huwx for ip source guard, 2012.01.21*/
#define GLB_IP_BINDING_SCL_TYPE_IP    1
#define GLB_IP_BINDING_SCL_TYPE_MAC   2

/*modified by wangjj for ip source guard, 2016.03.29 */
#define IPSOUR_DEFAULT_NUMBER  10
#define IPSOUR_MAX_BINDING_NUM_DEF   1024 

#define GLB_CPU_MIRROR_STRATEGY_REPLACE 0
#define GLB_CPU_MIRROR_STRATEGY_DROP    1

#define GLB_CPU_MIRROR_BUF_SIZE_DEF         1000 

#define GLB_VLAN_RANGE_ENTRY_NUM            8
#define GLB_VLAN_RANGE_GROUP_NUM            64

#define GLB_VLAN_MAX                 4094              /* Max VLANs.   */
#define GLB_VLAN_ALL                 (GLB_VLAN_MAX + 1)/* All VLANs.   */
#define GLB_VLAN_NONE                0                 /* No VLANs.    */
#define GLB_VLAN_DEFAULT_VID         1                 /* Default VID. */

#define GLB_VLAN_MAPPING_ENTRY_REV    16
#define GLB_VLAN_MAPPING_ENTRY_MAX    1024
#define GLB_VLAN_MAPPING_TAB_NAME_MAX 16
#define GLB_VLAN_MAX_VLAN_MAP_TABLE   64
#define GLB_VLAN_EVC_NAME_LEN         16
#define GLB_VLAN_EVC_MAX_NUM          4094

#define GLB_VLAN_EVC_INVALID_U16 0xFFFF
#define GLB_VLAN_EVC_TYPE_NULL 0
#define GLB_VLAN_EVC_TYPE_VLAN 1
#define GLB_VLAN_EVC_TYPE_VPLS 2
#define GLB_VLAN_EVC_TYPE_VPWS 3

/* Bit defines only for HalMsgPortState_t.state */
#define GLB_VLAN_FLAG_DEF_ENTRY                   (1 << 0)

/* Bit defines only for hsrv2AgtMsgL2VlanSync.flag */
#define GLB_VLAN_FLAG_STATE_ENABLE                   (1 << 0)
#define GLB_VLAN_FLAG_BRIDGE_ENABLE                   (1 << 1)
#define GLB_VLAN_FLAG_REPLACE_DSCP                   (1 << 2)
#define GLB_VLAN_FLAG_LEARNING_ENABLE                   (1 << 3)

#define GLB_ISOLATE_GRP_MAX             16
#define GLB_PORT_ISOLATE_GROUP_MAX      30  /* 30 and 31 is reserved for MLAG */
#define GLB_PORT_ISOLATE_GROUP_INVALID  0

#define GLB_STP_INSTANCE_DEF            0
/*Modified by xgu for bug 25713, 2013-11-7*/
#define GLB_STP_INSTANCE_MAX            64
/*modified by weij for bug 18234, 2012-03-06*/
#define GLB_STP_INSTANCE_ALL          0xFF

#define GLB_COPP_TRAP_MAX               0x6000  /* max of sai_hostif_trap_id_t */
#define GLB_COPP_GROUP_MAX              256
#define GLB_COPP_REASON_MAX             1024
#define GLB_COPP_ENTRY_MAX              64

#define GLB_MIRROR_MAX_SESSION_ID  4
#define GLB_MIRROR_MIN_SESSION_ID  1
#define GLB_MIRROR_MAX_SESSION_NUM  GLB_MIRROR_MAX_SESSION_ID+1
#define GLB_MIRROR_DIR_IGS          (1 << 0)
#define GLB_MIRROR_DIR_EGS          (1 << 1)
#define GLB_MIRROR_DIR_BOTH         ((1 << 0) | (1 << 1))
#define GLB_MIRROR_MAC_ESCAPE_MAX  2
#define GLB_MIRROR_ACLLOG_DEFAULT_BLOCK  1
#define GLB_MIRROR_MULTI_DEST_GROUP_MAX  32
 
#define GLB_DOT1X_AUTHED    1
#define GLB_DOT1X_UNAUTHED  2
#define GLB_DOT1X_UNCTRL    3
#define GLB_DOT1X_DIR_IN    1
#define GLB_DOT1X_DIR_BOTH  2

#define GLB_CFM_MAID_LENGTH_48BYTES       48
#define GLB_CFM_MAID_LENGTH_16BYTES       16
#define GLB_CFM_UP_MEP             1
#define GLB_CFM_DOWN_MEP           0

#define GLB_CFM_ETHERTYPE        0x8902
#define GLB_CFM_OPERATION_TYPE      3
#define GLB_CFM_MAX_ERROR_CACHE_NUM     16

#define GLB_CFM_INVALID_LEVEL       8
#define GLB_CFM_LEVEL_MIN               0
#define GLB_CFM_LEVEL_MAX               7

#define GLB_CFM_MODE_DOT1AG             0
#define GLB_CFM_MODE_Y1731              1

#define GLB_CFM_MAX_LMEP_ERROR_NUM      10
#define GLB_CFM_MAX_RMEP_ERROR_NUM      10

#define GLB_CFM_MAX_LM_DUAL_COUNTER_CACHE_NUM      10

#define GLB_CFM_MAX_LM_SD_CACHE_NUM      10

#define GLB_CFM_THROUGHPUT_LBM    1
#define GLB_CFM_THROUGHPUT_TST    2

#define GLB_CFM_TRPT_TRANSMIT_MODE_FIXED       0
#define GLB_CFM_TRPT_TRANSMIT_MODE_CONTINUOUS  1

#define GLB_CFM_TRPT_TLV_NONE     0
#define GLB_CFM_TRPT_TLV_DATA     1
#define GLB_CFM_TRPT_TLV_TEST     2

#define GLB_CFM_TRPT_PKT_SIZE_MIN      64
#define GLB_CFM_TRPT_PKT_SIZE_DEFAULT  512
#define GLB_CFM_TRPT_PKT_SIZE_MAX      16027

#define GBL_CFM_TRPT_TEST_TLV_PATTERN_TYPE_NULL_NO_CRC   0
#define GBL_CFM_TRPT_TEST_TLV_PATTERN_TYPE_PRBS_NO_CRC   2
#define GBL_CFM_TRPT_TEST_TLV_PATTERN_TYPE_RESERVED      4

/* IEEE 802.1ag 2007, Table 21-19 Maintenance Domain Name Format */
#define  GLB_CFM_NO_MD_NAME                    1
#define  GLB_CFM_DNS_NAME                      2
#define  GLB_CFM_MAC_ADDR_TWO_OCTET_INTEGER    3
#define  GLB_CFM_MD_CHAR_STRING                4
#define  GLB_CFM_MD_TYPE_ITU_T                 32

#define GLB_CFM_MD_MA_NAME_LENGTH_8021AG       44
#define GLB_CFM_MD_MA_NAME_LENGTH_Y1731         13

#define GLB_CFM_MA_CHAR_STRING                      2
#define GLB_CFM_MA_ICC_BASED_FORMAT            32

#define  GLB_CFM_PRIO_MIN     0
#define  GLB_CFM_PRIO_MAX     7

#define  GLB_CFM_MEP_INDEX_MIN          2050       /* MEP index: start from 2050 */
#define  GLB_CFM_MEP_NUM_MAX            1024

#define  GLB_CFM_MAID_MAX_SIZE          170

#define GLB_CFM_MAX_MEP_NUM_ON_CHANNEL    4

/* modified by yaom for bug 15216, 2011.6.17 */
#define GLB_FDB_FLOOD_ALL           (1 << 1)
#define GLB_FDB_FLOOD_UNKOWN_UCAST  (1 << 2)
#define GLB_FDB_FLOOD_MCAST         (1 << 3)
#define GLB_FDB_FLOOD_BROADCAST     (1 << 4)

#define GLB_APS_TYPE_PERNANENT     1
#define GLB_APS_TYPE_NON_PERNANENT 2
#define GLB_APS_STATE_BLOCKING     (1 << 0)
//#define GLB_APS_STATE_SELECT       (1 << 1)

#define GLB_VOICE_VLAN_OUI_DEF_NUM            5
#define GLB_VOICE_VLAN_OUI_NUM                16
#define GLB_VOICE_VLAN_OUI_DEFAULT         16
#define GLB_VOICE_VLAN_OUI_DESC               128

#define GLB_FDB_DEFAULT_AGING_TIME          300

/* Port type flags. */
#define GLB_LAG_LOAD_BALANCE_SRC_MAC        (1 << 0)
#define GLB_LAG_LOAD_BALANCE_DST_MAC        (1 << 1)
#define GLB_LAG_LOAD_BALANCE_SRC_IP         (1 << 2)
#define GLB_LAG_LOAD_BALANCE_DST_IP         (1 << 3)
#define GLB_LAG_LOAD_BALANCE_IP_PROTOCOL    (1 << 4)
#define GLB_LAG_LOAD_BALANCE_SRC_PORT       (1 << 5)
#define GLB_LAG_LOAD_BALANCE_DST_PORT       (1 << 6)
#define GLB_LAG_LOAD_BALANCE_VXLAN_VNI      (1 << 7)
#define GLB_LAG_LOAD_BALANCE_INNER_SRC_MAC  (1 << 8)
#define GLB_LAG_LOAD_BALANCE_INNER_DST_MAC  (1 << 9)
#define GLB_LAG_LOAD_BALANCE_INNER_SRC_IP   (1 << 10)
#define GLB_LAG_LOAD_BALANCE_INNER_DST_IP   (1 << 11)
#define GLB_LAG_LOAD_BALANCE_INNER_IP_PROTOCOL   (1 << 12)
#define GLB_LAG_LOAD_BALANCE_INNER_SRC_PORT_L4   (1 << 13)
#define GLB_LAG_LOAD_BALANCE_INNER_DST_PORT_L4   (1 << 14)
#define GLB_LAG_LOAD_BALANCE_GRE_KEY        (1 << 15)
#define GLB_LAG_LOAD_BALANCE_NVGRE_VNI      (1 << 16)
#define GLB_LAG_LOAD_BALANCE_NVGRE_FLOW_ID  (1 << 17)

enum ethernet_evc_type_e
{
    ETHERNET_EVC_TYPE_NONE = 0,
    ETHERNET_EVC_TYPE_SINGLE_SVLAN,
    ETHERNET_EVC_TYPE_DOUBLE_VLAN,
    ETHERNET_EVC_TYPE_MAX
};
typedef enum ethernet_evc_type_e ethernet_evc_type_t;

typedef enum glb_lag_hash_algo_inner_mode_e
{
    GLB_LAG_TUNNEL_HASH_MODE_OUTER,  /* hash-calc only outer-field */
    GLB_LAG_TUNNEL_HASH_MODE_BOTH,	/* hash-calc inner and outer-field, default type*/
    GLB_LAG_TUNNEL_HASH_MODE_INNER,  /* hash-calc tunnel only inner, else outer */
}glb_lag_hash_algo_inner_mode_t;

#ifdef TAPPRODUCT
/* TAP */
#define GLB_AGG_LOAD_BALANCE_CTC_DEFAULT    (GLB_LAG_LOAD_BALANCE_SRC_IP | GLB_LAG_LOAD_BALANCE_DST_IP | GLB_LAG_LOAD_BALANCE_DST_PORT | GLB_LAG_LOAD_BALANCE_SRC_PORT)
#define GLB_AGG_LOAD_BALANCE_INNER_FIELD_EN    GLB_LAG_TUNNEL_HASH_MODE_BOTH

#else
/* L2+ */
#ifdef GREATBELT
#define GLB_AGG_LOAD_BALANCE_CTC_DEFAULT    (GLB_LAG_LOAD_BALANCE_SRC_IP | GLB_LAG_LOAD_BALANCE_DST_IP)
#else
#define GLB_AGG_LOAD_BALANCE_CTC_DEFAULT    (GLB_LAG_LOAD_BALANCE_SRC_MAC | GLB_LAG_LOAD_BALANCE_DST_MAC | GLB_LAG_LOAD_BALANCE_SRC_IP | GLB_LAG_LOAD_BALANCE_DST_IP)
#endif
#define GLB_AGG_LOAD_BALANCE_INNER_FIELD_EN    GLB_LAG_TUNNEL_HASH_MODE_OUTER
#endif

#define GLB_G8032_MAX_SUB_RINGS    1   /* For one major ring, the number of sub ring should be limited */ 

typedef enum glb_lag_hash_algo_e
{
    GLB_LAG_HASH_ALGO_XOR,     /*default type*/
    GLB_LAG_HASH_ALGO_CRC
}glb_lag_hash_algo_t;

typedef enum glb_lag_load_balance_mode_e
{
    GLB_LAG_LOAD_BALANCE_MODE_STATIC,
    GLB_LAG_LOAD_BALANCE_DLB,
    GLB_LAG_LOAD_BALANCE_RR,
    GLB_LAG_LOAD_BALANCE_RESILIENT
}glb_lag_load_balance_mode_t;

typedef enum glb_fdb_type_e
{
    GLB_FDB_TYPE_UCAST_STATIC,
    GLB_FDB_TYPE_UCAST_DYNAMIC,
    GLB_FDB_TYPE_UCAST_ALL,
    GLB_FDB_TYPE_MULTICAST,
    GLB_FDB_TYPE_ALL
} glb_fdb_type_t;

enum glb_oam_trpt_pattern_type_e
{
    GLB_OAM_TRPT_PATTERN_REPEAT_TYPE,       /**< [GB] repeat mode */
    GLB_OAM_TRPT_PATTERN_RANDOM_TYPE,       /**< [GB] random mode */
    GLB_OAM_TRPT_PATTERN_INC_BYTE_TYPE,     /**< [GB] increace by byte mode */
    GLB_OAM_TRPT_PATTERN_DEC_BYTE_TYPE,     /**< [GB] dec by byte mode */
    GLB_OAM_TRPT_PATTERN_INC_WORD_TYPE,     /**< [GB] increace by word mode */
    GLB_OAM_TRPT_PATTERN_DEC_WORD_TYPE,     /**< [GB] increace by word mode */

    GLB_OAM_TRPT_PATTERN_MAX_TYPE
};

enum glb_oam_trpt_time_type_e
{
    GLB_OAM_TRPT_TIME_1S_TYPE = 1,       /**< [GB] 1 second */
    GLB_OAM_TRPT_TIME_1M_TYPE = 60,      /**< [GB] 1 minute */
    GLB_OAM_TRPT_TIME_15M_TYPE,          /**< [GB] 15 minutes */
    GLB_OAM_TRPT_TIME_2H_TYPE,           /**< [GB] 2 hours */
    GLB_OAM_TRPT_TIME_24H_TYPE,          /**< [GB] 24 hours */

    GLB_OAM_TRPT_TIME_MAX_TYPE
};

enum glb_oam_tst_type_e
{
    GLB_OAM_TST_TX       = 1,
    GLB_OAM_TST_RX       = 2,
    GLB_OAM_TST_TX_RX    = 3,

    GLB_OAM_TST_MAX
};

/* OAM exceptions in packet info */
enum glb_oam_exception_e
{
    GLB_OAM_EXCEPTION_OAM_PDU_INVALID                        = 0,
    GLB_OAM_EXCEPTION_SOME_RDI_DEFECT                        = 1,
    GLB_OAM_EXCEPTION_SOME_MAC_STATUS_DEFECT                 = 2,
    GLB_OAM_EXCEPTION_HIGH_VERSION_ETH_OAM_TO_CPU            = 3,
    GLB_OAM_EXCEPTION_ERROR_CCM_DEFECT_RMEP_NOT_FOUND        = 4,
    GLB_OAM_EXCEPTION_XCON_CCM_DEFECT                        = 5,
    GLB_OAM_EXCEPTION_CCM_SEQ_NUM_ERROR                      = 6,
    GLB_OAM_EXCEPTION_DS_OR_CTRL_REG_CONFIG_ERROR            = 7, 
    GLB_OAM_EXCEPTION_ETH_CCM_TLV_OPTION                     = 8,
    GLB_OAM_EXCEPTION_SLOW_OAM_PDU_TO_CPU                    = 9, 
    GLB_OAM_EXCEPTION_RESERVED0                              = 10,
    GLB_OAM_EXCEPTION_HIGHER_CCM_ON_CBP_PIP                  = 11,
    GLB_OAM_EXCEPTION_OAM_LOOKUP_NOT_EN_TO_CPU               = 12, 
    GLB_OAM_EXCEPTION_SOURCE_MAC_MISMATCH                    = 13,
    GLB_OAM_EXCEPTION_FDI_TO_CPU                             = 14,
    GLB_OAM_EXCEPTION_BDI_TO_CPU                             = 15,
    GLB_OAM_EXCEPTION_APS_PDU_TO_CPU                         = 16,
    GLB_OAM_EXCEPTION_ERROR_CCM_DEFECT_CCM_INTERVAL_MISMATCH = 17,
    GLB_OAM_EXCEPTION_DM_TO_CPU                              = 18,
    GLB_OAM_EXCEPTION_MD_LEVEL_LOWER_THAN_PASSIVE_MEP        = 19,
    GLB_OAM_EXCEPTION_MPLS_OR_PBT_MEP_CONFIG_ERROR           = 20,
    GLB_OAM_EXCEPTION_MPLS_OR_PBT_CHECK_ERROR                = 21,
    GLB_OAM_EXCEPTION_ALL_PBB_BSI_PDU_TO_CPU                 = 22,
    GLB_OAM_EXCEPTION_PBT_MM_DEFECT_PDU_TO_CPU               = 23,
    GLB_OAM_EXCEPTION_MPLS_CV0_TO_CPU                        = 24,
    GLB_OAM_EXCEPTION_EQUAL_LBR_TO_CPU                       = 25,
    GLB_OAM_EXCEPTION_LBM_MAC_DA_CHECK_FAIL                  = 26,
    GLB_OAM_EXCEPTION_LM_TO_CPU                              = 27,
    GLB_OAM_EXCEPTION_LEARNING_BFD_TO_CPU                    = 28,
    GLB_OAM_EXCEPTION_ERROR_BFD_TO_CPU                       = 29,
    GLB_OAM_EXCEPTION_RESERVED1                              = 30,
    GLB_OAM_EXCEPTION_ERROR_CACHE_FULL                       = 31,
};

enum glb_cfm_error_reason_e
{
    GLB_CFM_RMEP_SRC_MAC_MISMATCH                        = 0,
    GLB_CFM_SOME_RDI_DEFECT                              = 1,
    GLB_CFM_SOME_RDI_DEFECT_CLEAR                        = 2,
    GLB_CFM_SOME_MAC_STATUS_DEFECT                       = 3,
    GLB_CFM_SOME_RMEP_CCM_DEFECT_DLOC                    = 4,
    GLB_CFM_SOME_RMEP_CCM_DEFECT_DLOC_CLEAR              = 5,
    GLB_CFM_ERROR_CCM_DEFECT_RMEP_NOT_FOUND              = 6,
    GLB_CFM_ERROR_CCM_DEFECT_RMEP_NOT_FOUND_CLEAR        = 7,
    GLB_CFM_ERROR_CCM_DEFECT_CCM_INTERVAL_MISMATCH       = 8,
    GLB_CFM_ERROR_CCM_DEFECT_CCM_INTERVAL_MISMATCH_CLEAR = 9,
    GLB_CFM_XCON_CCM_DEFECT_CCM_MAID_MISMATCH            = 10,
    GLB_CFM_XCON_CCM_DEFECT_CCM_MAID_MISMATCH_CLEAR      = 11,
    GLB_CFM_XCON_CCM_DEFECT_CCM_LOW_CCM                  = 12,
    GLB_CFM_XCON_CCM_DEFECT_CCM_LOW_CCM_CLEAR            = 13,
    GLB_CFM_AIS_DEFECT_AIS_RECEIVE                       = 14,
    GLB_CFM_AIS_DEFECT_AIS_RECEIVE_CLEAR                 = 15,
    GLB_CFM_LCK_RECEIVE_CAUSE_AIS                        = 16,
    GLB_CFM_LCK_RECEIVE_CAUSE_AIS_CLEAR                  = 17,
    GLB_CFM_ENTER_LCK_CONDITION                          = 18,
    GLB_CFM_EXIT_LCK_CONDITION                           = 19,
    GLB_CFM_ENTER_CSF_CONDITION                          = 20,
    GLB_CFM_EXIT_CSF_CONDITION                           = 21,
    GLB_CFM_RMEP_FIRST_PACKET_RECEIVE                    = 22,
    GLB_CFM_UNKNOWN_REASON,
};
typedef enum glb_cfm_error_reason_e glb_cfm_error_reason_t;

enum glb_cfm_state_for_g8032_e
{
    GLB_CFM_RMEP_INITIAL,
    GLB_CFM_RMEP_UP,
    GLB_CFM_RMEP_DOWN,
};
typedef enum glb_cfm_state_for_g8032_e glb_cfm_state_for_g8032_t;

struct glb_fdb_info_s
{
    uint8_t mac[6];
    uint16_t fid;
    uint16_t gl_port;    
    uint16_t flag;
    uint16_t l3_type;
    uint16_t is_valid;
    uint32_t nhid;

    uint8_t   gsrcport_type;
    uint16_t  virtual_port;       /**< virtual Port Number */

    /* modified by liwh to support cfm for bug 14710 */
    uint8_t is_ether_oam;        /* If set, indicate packet is Ethernet OAM */
    uint8_t ether_oam_md_level;  /* Ethernet OAM MD Levl */
    uint16_t  reserved1;
    uint8_t   reserved2;
    
    /* modified by yaom for bug 15157, 2011.6.16 */
    bool by_name;
    uint32_t vpls_peer_ip; /*for show mac addr vpls only*/
    char    vpls_peer_name[21]; /*for show mac addr vpls only*/
};
typedef struct glb_fdb_info_s glb_fdb_info_t;

enum glb_port_security_mode_e
{
    GLB_PORT_SECURITY_MODE_NONE = 0,
    GLB_PORT_SECURITY_MODE_PORT,
    GLB_PORT_SECURITY_MODE_VLAN,
    GLB_PORT_SECURITY_MODE_VLAN_EX
};
typedef enum glb_port_security_mode_e glb_port_security_mode_t;

enum glb_port_security_violation_mode_e
{
    GLB_PORT_SECURITY_VIOLATION_MODE_PROTECT = 0,
    GLB_PORT_SECURITY_VIOLATION_MODE_RESTRICT,
    GLB_PORT_SECURITY_VIOLATION_MODE_SHUTDOWN,
    GLB_PORT_SECURITY_VIOLATION_MODE_MAX
};
typedef enum glb_port_security_violation_mode_e glb_port_security_violation_mode_t;

#define GLB_PORT_SECURITY_VIO_TYPE_MACSA_MISMATCH   1
#define GLB_PORT_SECURITY_VIO_TYPE_PORT_MACLIMIT_DISCARD  2
#define GLB_PORT_SECURITY_VIO_TYPE_VLAN_MACLIMIT_DISCARD  3

enum glb_port_security_enable_e
{
    GLB_PORT_SECURITY_DISABLE = 0,
    GLB_PORT_SECURITY_ENABLE,
    GLB_PORT_SECURITY_MAX
};
typedef enum glb_port_security_enable_e glb_port_security_enable_t;

enum glb_port_security_config_bmp_e
{
    GLB_PORT_SECURITY_CONFIG_SECURITY_EN = 1 << 0,
    GLB_PORT_SECURITY_CONFIG_VIO_MODE    = 1 << 1,
    GLB_PORT_SECURITY_CONFIG_MAX_MAC_NUM = 1 << 2,
    GLB_PORT_SECURITY_CONFIG_MAX
};
typedef enum glb_port_security_config_bmp_e glb_port_security_config_bmp_t;

enum glb_vlan_security_action_e
{
    GLB_VLAN_SECURITY_ACTION_NONE,
    GLB_VLAN_SECURITY_ACTION_DISCARD,
    GLB_VLAN_SECURITY_ACTION_WARN,
    GLB_VLAN_SECURITY_ACTION_FORWARD,
    GLB_VLAN_SECURITY_ACTION_MAX
};
typedef enum glb_vlan_security_action_e glb_vlan_security_action_t;

enum glb_acl_config_type_e{
    GLB_ACL_CONFIG_TYPE_NORMAL = 0,
    GLB_ACL_CONFIG_TYPE_FLOW_MATCH,
    GLB_ACL_CONFIG_TYPE_FLOW,
    GLB_ACL_CONFIG_TYPE_IPV6,
    GLB_ACL_CONFIG_TYPE_MAC,
    GLB_ACL_CONFIG_TYPE_MPLS,
    GLB_ACL_CONFIG_TYPE_MAX,
};
typedef enum glb_acl_config_type_e glb_acl_config_type_t;


#define GLB_VLAN_SECURITY_MAX_MSG_VLAN  16
#define GLB_VLAN_SEC_ACT_TO_STR(action)                         \
    (action == GLB_VLAN_SECURITY_ACTION_NONE ? "None" :         \
    action == GLB_VLAN_SECURITY_ACTION_DISCARD ? "Discard" :    \
     action == GLB_VLAN_SECURITY_ACTION_WARN ? "Warn" :         \
     action == GLB_VLAN_SECURITY_ACTION_FORWARD ? "Forward" :   \
     "error action")
#define GLB_VLAN_SEC_SPEC_MAX_MAC_COUNT 0xffffffff

#define GLB_PORT_SECURITY_MAC_DEFAULT_MAXIMUM  1

#define GLB_ERPS_DOMAIN_HELLO_TIME_DEFAULT     1
#define GLB_ERPS_DOMAIN_FAILURE_TIME_DEFAULT   3
#define GLB_ERPS_DOMAIN_PRE_FORWARDING_TIMER_DEFAULT 6
#define GLB_ERPS_DOMAIN_FLUSH_TIMER_DEFAULT 10
#define GLB_ERPS_DOMAIN_INSTANCE_ID_DEFAULT    0xffff


#define GLB_ERPS_MAX_DOMAIN                    16
#define GLB_ERPS_MAX_SUB_RING_PER_DOMAIN       2

#define GLB_CFM_MD_MAX                              32
#define GLB_CFM_MA_MAX                              170
#define GLB_CFM_LMEP_RMEP_MAX               1024

#define GLB_G8032_RING_MAX                              128

/*Added by xgu for hardware learning, 2013-9-16*/
enum glb_l2_fdb_entry_op_type_e
{
    GLB_L2_FDB_ENTRY_OP_BY_VID,       /**<[HB,GB] operation BY FID(VLAN/VSI)*/
    GLB_L2_FDB_ENTRY_OP_BY_PORT,      /**<[HB,GB] operation BY PORT(VLAN/VSI)*/
    GLB_L2_FDB_ENTRY_OP_BY_MAC,       /**<[HB,GB] operation BY MA(VLAN/VSI)*/
    GLB_L2_FDB_ENTRY_OP_BY_PORT_VLAN, /**<[HB,GB] operation BY PORT + FID(VLAN/VSI)*/
    GLB_L2_FDB_ENTRY_OP_BY_MAC_VLAN,  /**<[HB,GB] operation BY MAC+FID(VLAN/VSI)*/
    GLB_L2_FDB_ENTRY_OP_ALL,          /**<[HB,GB] operation BY ALL(VLAN/VSI)*/
    GLB_L2_FDB_ENTRY_OP_GET_COUNT,
    MAX_GLB_L2_FDB_ENTRY_OP
};
typedef enum glb_l2_fdb_entry_op_type_e glb_l2_fdb_query_type_t;

enum glb_l2_fdb_entry_flag_e
{
    GLB_L2_FDB_ENTRY_STATIC,        /**<[HB,GB] all static fdb record */
    GLB_L2_FDB_ENTRY_DYNAMIC,       /**<[HB,GB] all dynamic fdb record */
    GLB_L2_FDB_ENTRY_LOCAL_DYNAMIC, /**<[HB,GB] all local chip's dynamic fdb record */
    GLB_L2_FDB_ENTRY_ALL,           /**<[HB,GB] all fdb record */
    MAX_GLB_L2_FDB_ENTRY_FLAG
};
typedef enum glb_l2_fdb_entry_flag_e glb_l2_fdb_query_flag_t;

/*modified by yanxa, for release 2 action-index, bug 15434, 2011-07-13*/
#define GLB_L2_PROTOCOL_MAC_MAX (GLB_PROTO_FULL_MAC00+1-GLB_PROTO_MASK_MAC00)           /*add the last array member for ful-mac type, modify by zhangjx for bug 20488, 20120905*/
#define GLB_L2_PROTOCOL_FULL_MAC_MAX   (GLB_PROTO_FULL_MAC00 - GLB_PROTO_MAC04)
#define GLB_L2_PORTOCOL_PPPOE_TYPE  13

/*L2 Protocol Tunnel*/
#define GLB_L2_PROTOCOL_NAME_SIZE 20
#define GLB_L2_PROTOCOL_DEFAULT_COS 0
#define GLB_L2_PROTOCOL_TUNNEL_DMAC_NUM 4
#define GLB_L2_PROTOCOL_NUM 8
#define GLB_L2_PROTOCOL_MAC_NUM 4
enum glb_l2_proto_e
{ 
    GLB_PROTO_LLDP = 1,
    GLB_PROTO_BPDU,
    GLB_PROTO_LACP,
    GLB_PROTO_EAPOL,
    GLB_PROTO_MAX
};
typedef enum glb_l2_proto_e glb_l2_proto_t;

enum glb_l2_proto_action_e
{
    GLB_L2_PROTO_PEER = 0,
    GLB_L2_PROTO_TUNNEL,
    GLB_L2_PROTO_DISCARD,
    GLB_L2_PROTO_FORWARD,
    GLB_L2_PROTO_PROCESS_MAX,
};
typedef enum glb_l2_proto_action_e glb_l2_proto_action_t;

enum glb_l2_proto_index_e
{
    GLB_L2_PROTO_INDEX_0 = 0, /*l2protocol dot1x*/
    GLB_L2_PROTO_INDEX_1,     /*l2protocol lldp*/    
    GLB_L2_PROTO_INDEX_2,     /*l2protocol slow-proto*/
    GLB_L2_PROTO_INDEX_3,     /*l2protocol stp*/
    GLB_L2_PROTO_INDEX_4,     /*l2protocol user-defined*/
    GLB_L2_PROTO_INDEX_5,     /*l2protocol user-defined*/
    GLB_L2_PROTO_INDEX_6,     /*l2protocol user-defined*/
    GLB_L2_PROTO_INDEX_7,     /*l2protocol user-defined*/
    GLB_L2_PROTO_INDEX_MAX,
};
typedef enum glb_l2_proto_index_e glb_l2_proto_index_t;

enum glb_dhcp_pkt_excp_type_e
{
    GLB_DHCPPKT_FORWORD_CPU = 0,
    GLB_DHCPPKT_FORWORD,
    GLB_DHCPPKT_CPU,
    GLB_DHCPPKT_DISGARD
};
typedef enum glb_dhcp_pkt_excp_type_e glb_dhcp_pkt_excp_type_t;

struct l2pro_if_action_vlan_s
{
    uint32  l2action[GLB_L2_PROTOCOL_NUM];
    vid_t   l2vlan[GLB_L2_PROTOCOL_NUM];
};
typedef struct l2pro_if_action_vlan_s l2pro_if_action_vlan_t;

enum glb_arp_excp_type_e
{
    GLB_ARPPKT_FORWORD_CPU = 0,
    GLB_ARPPKT_FORWORD,
    GLB_ARPPKT_CPU,
    GLB_ARPPKT_DISGARD
};
typedef enum glb_arp_excp_type_e glb_arp_excp_type_t;

enum glb_efm_mux_action_s
{
    GLB_MUX_ACTION_FWD,
    GLB_MUX_ACTION_DISCARD,  
    GLB_MUX_ACTION_INVALID,
};
typedef enum glb_efm_mux_action_s glb_efm_mux_action_t;

enum glb_efm_par_action_s
{
    GLB_PAR_ACTION_FWD,
    GLB_PAR_ACTION_LB,
    GLB_PAR_ACTION_DISCARD,
    GLB_PAR_ACTION_INVALID,
};
typedef enum glb_efm_par_action_s glb_efm_par_action_t;

struct glb_efm_par_mux_action_s
{
    u_int8_t  par_action;
    u_int8_t  mux_action;
    u_int16_t reserved2;
};
typedef struct glb_efm_par_mux_action_s glb_efm_par_mux_action_t;

enum glb_l2_fdb_port_type_e
{
    GLB_L2_FDB_NORMAL = 0, /**< from normal port */
    GLB_L2_FDB_TYPE_VPLS = 1,   /**< from vpls port/qinq service queue appication */
    GLB_L2_FDB_TYPE_PBB = 2,    /**< from pbb PIP */
    MAX_L2_FDB_TYPE,
};
typedef enum glb_l2_fdb_port_type_e glb_l2_fdb_port_type_t;

struct glb_l2_delete_static_fdb_s
{
    uint16_t    vid;
    uint8_t     mac[6];
    uint32_t    ifindex;
    uint32_t    flag;
};
typedef struct glb_l2_delete_static_fdb_s glb_l2_delete_static_fdb_t;

#define GLB_MAX_OAM_FDB_ENTRIES 32

struct glb_l2_oam_fdb_s
{
    uint16_t    vid;
    uint8_t     mac[6];
    uint8_t     level;
    uint32_t    ifindex;
    uint8_t     bstatic;
    uint8_t     reserved[2];
};
typedef struct glb_l2_oam_fdb_s glb_l2_oam_fdb_t;

struct glb_l2_oam_fdb_entries_s
{
    uint32_t            count;
    glb_l2_oam_fdb_t    oam_fdb_entry[GLB_MAX_OAM_FDB_ENTRIES]; 
};
typedef struct glb_l2_oam_fdb_entries_s glb_l2_oam_fdb_entries_t;

struct glb_cfm_error_cache_entry_s
{
    uint16_t mep_id;
    uint16_t rmep_id;
    uint16_t vlan_id;
    uint8_t  level;
    uint8_t  resv1;
    uint8_t  present_rdi;
    uint8_t  d_unexp_mep;
    uint8_t  d_mismerge;
    uint8_t  d_meg_lvl;
    uint8_t  first_pkt_rx;
    uint8_t  last_rdi;
    uint8_t  d_loc;
    uint8_t  d_unexp_period;
    uint8_t  d_csf;
    uint8_t  ma_sa_mismatch;
    uint8_t  mac_status_change;
    uint8_t  resv2;     
};
typedef struct glb_cfm_error_cache_entry_s glb_cfm_error_cache_entry_t;

struct glb_cfm_error_cache_s
{
    uint8_t error_num;
    glb_cfm_error_cache_entry_t error_entry[GLB_CFM_MAX_ERROR_CACHE_NUM];
};
typedef struct glb_cfm_error_cache_s glb_cfm_error_cache_t;

struct glb_cfm_lmep_error_entry_s
{
    uint8_t  level;
    uint16_t vlan_id;
    uint16_t mep_id; 
    
    uint8_t   present_rdi;
    uint8_t   d_unexp_mep;
    uint8_t   d_mismerge;
    uint8_t   d_meg_lvl; 
    uint8_t   reserved[3];
};
typedef struct glb_cfm_lmep_error_entry_s glb_cfm_lmep_error_entry_t;

struct glb_cfm_lmep_error_cache_s
{
    uint8_t num;
    glb_cfm_lmep_error_entry_t entry[GLB_CFM_MAX_LMEP_ERROR_NUM];
};
typedef struct glb_cfm_lmep_error_cache_s glb_cfm_lmep_error_cache_t;

#define GLB_CFM_MAX_LMEP_LM_DUAL_NUM      10
struct glb_cfm_lmep_lm_dual_entry_s
{
    uint8_t  level;
    uint16_t vlan_id;
    uint16_t mep_id; 
    
    uint32_t  TxFCf;
    uint32_t  RxFCb;
    uint32_t  TxFCb;
    uint32_t  RxFCl;

    uint8_t   ccm_cos;
    uint16_t   reserved;
};
typedef struct glb_cfm_lmep_lm_dual_entry_s glb_cfm_lmep_lm_dual_entry_t;

struct glb_cfm_lmep_lm_dual_cache_s
{
    uint32_t num;
    glb_cfm_lmep_lm_dual_entry_t entry[GLB_CFM_MAX_LMEP_LM_DUAL_NUM];
};
typedef struct glb_cfm_lmep_lm_dual_cache_s glb_cfm_lmep_lm_dual_cache_t;

struct glb_cfm_rmep_error_entry_s
{
    uint8_t  level;
    uint16_t vlan_id;
    uint16_t mep_id; 
    uint16_t rmep_id;
    
    uint8_t   last_rdi;
    uint8_t   d_loc;
    uint8_t   d_unexp_period;
    uint16_t   reserved;
};
typedef struct glb_cfm_rmep_error_entry_s glb_cfm_rmep_error_entry_t;

struct glb_cfm_rmep_error_cache_s
{
    uint8_t num;
    glb_cfm_rmep_error_entry_t entry[GLB_CFM_MAX_RMEP_ERROR_NUM];
};
typedef struct glb_cfm_rmep_error_cache_s glb_cfm_rmep_error_cache_t;

struct glb_cfm_lm_dual_counter_s
{
    uint8_t   level;
    uint16_t  vlan_id;
    uint16_t  mep_id;
    
    uint32_t  TxFCf;
    uint32_t  RxFCb;
    uint32_t  TxFCb;
    uint32_t  RxFCl;

    u_int8_t  ccm_cos_value;
    u_int16_t reserved;
};
typedef struct glb_cfm_lm_dual_counter_s glb_cfm_lm_dual_counter_t;

struct glb_cfm_lm_dual_counter_cache_s
{
    uint8_t num;
    glb_cfm_lm_dual_counter_t entry[GLB_CFM_MAX_LM_DUAL_COUNTER_CACHE_NUM];
};
typedef struct glb_cfm_lm_dual_counter_cache_s glb_cfm_lm_dual_counter_cache_t;

struct glb_cfm_lm_single_counter_s
{
    uint32_t  TxFCf;
    uint32_t  RxFCf;
    uint32_t  TxFCb;
    uint32_t  RxFCl;

    u_int32_t  lmm_cos_value;
};
typedef struct glb_cfm_lm_single_counter_s glb_cfm_lm_single_counter_t;


struct glb_cfm_lm_sd_s
{
    uint8_t   level;
    uint16_t  vlan_id;
    uint16_t  mep_id;
    
    u_int8_t reserved[3];
};
typedef struct glb_cfm_lm_sd_s glb_cfm_lm_sd_t;

struct glb_cfm_lm_sd_cache_s
{
    uint8_t num;
    glb_cfm_lm_sd_t entry[GLB_CFM_MAX_LM_SD_CACHE_NUM];
};
typedef struct glb_cfm_lm_sd_cache_s glb_cfm_lm_sd_cache_t;

enum glb_cfm_lm_type
{
    GLB_CFM_LM_TYPE_NONE = 0,
    GLB_CFM_LM_TYPE_DUAL = 1,
    GLB_CFM_LM_TYPE_SINGLE = 2,
};

/* modified by liwh for bug 17055, 2011-11-21 */
struct glb_get_cfm_tx_ts_s
{  
    u_int32_t  ifindex;
    u_int32_t  type;
};
typedef struct glb_get_cfm_tx_ts_s glb_get_cfm_tx_ts_t;

struct glb_cfm_size_s
{
    uint16_t cfm_local_and_remote_meps;
    uint16_t cfm_lck;
};
typedef struct glb_cfm_size_s glb_cfm_size_t;

typedef struct glb_voice_vlan_oui_s
{
    uint8_t mac[MAC_ADDR_LEN];
    uint8_t mask[MAC_ADDR_LEN];
    char desc[GLB_VOICE_VLAN_OUI_DESC];
    uint8_t enable;
} glb_voice_vlan_oui_t;

typedef struct glb_voice_vlan_s
{   
    uint16_t vid;
    glb_voice_vlan_oui_t oui[GLB_VOICE_VLAN_OUI_NUM];
} glb_voice_vlan_t;


enum glb_l2_fdb_flush_type_e
{
    GLB_L2_FDB_FLUSH_ALL_FDB = 0, 
    GLB_L2_FDB_FLUSH_BY_VID = 1,  
    GLB_L2_FDB_FLUSH_BY_PORT = 2,   
    GLB_L2_FDB_FLUSH_BY_PORT_AND_VID = 3,
    GLB_L2_FDB_FLUSH_BY_MAC = 4,    
    GLB_L2_FDB_FLUSH_ALL_FDB_TOTAL = 5,    /* flush all fdb one time*/
};
typedef enum glb_l2_fdb_flush_type_e glb_l2_fdb_flush_type_t;

struct glb_l2_get_index_by_mac_vid_req_s
{
    uint16_t    vid;
    uint8_t     mac_addr[MAC_ADDR_LEN];
};
typedef struct glb_l2_get_index_by_mac_vid_req_s glb_l2_get_index_by_mac_vid_req_t;

struct glb_l2_get_index_by_mac_vid_resp_s
{
    uint32_t    ifindex;
};
typedef struct glb_l2_get_index_by_mac_vid_resp_s glb_l2_get_index_by_mac_vid_resp_t;

struct glb_l2_get_tst_counters_resp_s
{
    uint64      tx_pkt;
    uint64      tx_oct;
    uint64      rx_pkt;
    uint64      rx_oct;
};
typedef struct glb_l2_get_tst_counters_resp_s glb_l2_get_tst_counters_resp_t;

#define GLB_MLAG_SYNC_COUNT_MAX     100

typedef enum
{
    GLB_MLAG_FDB_SYNC_ADD               = 0,
    GLB_MLAG_FDB_SYNC_DEL               = 1,
    GLB_MLAG_FDB_SYNC_FLUSH_ALL         = 2,
    GLB_MLAG_FDB_SYNC_FLUSH_PORT        = 3,
    GLB_MLAG_FDB_SYNC_FLUSH_VLAN        = 4,
    GLB_MLAG_FDB_SYNC_FLUSH_PORT_VLAN   = 5,
    GLB_MLAG_FDB_SYNC_FLUSH_MAC         = 6,
    GLB_MLAG_FDB_SYNC_MAX               = 7
} glb_mlag_fdb_sync_act_t;

typedef struct
{
    uint8   mac[MAC_ADDR_LEN];
    uint16  fid;
    uint32  mlag_id;    
    uint32  flag;
    uint32  act;
} glb_mlag_fdb_entry_t;

#define FEI_MLAG_FDB_SYNC_LEN(_COUNT_)  (4 + sizeof(glb_mlag_fdb_entry_t) * (_COUNT_))

typedef struct
{
    uint32                  count;
    glb_mlag_fdb_entry_t    array[GLB_MLAG_SYNC_COUNT_MAX+1];   /* add 1 for avoid memory overwrite */
} glb_mlag_fdb_sync_t;

typedef struct
{
    vid_t               fid;
    mac_addr_t          mac;
} fdb_key_t;

#define FLD_MAX_DEPTH   2
#define FLD_ID_ALL      (-1)

#define FLD_MATCH(_fld_id, _id)  ((_fld_id == _id) || (FLD_ID_ALL == _id))

typedef struct
{
    int32   field_id[FLD_MAX_DEPTH];
} field_parse_t;

typedef struct
{
    uint32     raw_vid;
    char       table_name[GLB_VLAN_MAPPING_TAB_NAME_MAX+1];
} vlan_mapping_entry_key_t;

typedef struct
{
    uint32     raw_cvid;
    uint32     raw_svid;
    char       table_name[GLB_VLAN_MAPPING_TAB_NAME_MAX+1];
} egress_mapping_entry_key_t;

typedef struct
{
    char       evc_name[GLB_VLAN_EVC_NAME_LEN+1];
    char       table_name[GLB_VLAN_MAPPING_TAB_NAME_MAX+1];
} mapped_vlan_info_key_t;

typedef struct
{
    mac_addr_t          src_mac;
    mac_addr_t          dest_mac;
    vid_t                    fid;
} l2edit_key_t;

#endif /*__GLB_L2_DEFINE_H__*/
