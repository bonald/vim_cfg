/**
 @file sys_usw_cpu_reason.c

 @date 2010-01-13

 @version v2.0

*/

/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/

#include "ctc_debug.h"
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_macro.h"
#include "ctc_warmboot.h"

#include "ctc_pdu.h"
#include "sys_usw_chip.h"
#include "sys_usw_qos.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_common.h"
#include "sys_usw_opf.h"
#include "sys_usw_register.h"
#include "sys_usw_dmps.h"
#include "sys_usw_packet.h"

#include "drv_api.h"


#define SYS_REASON_NORMAL_EXCP 0
#define SYS_REASON_FATAL_EXCP 1
#define SYS_REASON_OAM_EXCP 2
#define SYS_REASON_FWD_EXCP 3
#define SYS_REASON_MISC_NH_EXCP 4

#define SYS_REASON_ENCAP_EXCP(type, excp, sub_excp)  ((type << 9) | ((excp) + (sub_excp)))
#define SYS_REASON_GET_EXCP_TYPE(exception) (exception >> 9)
#define SYS_REASON_GET_EXCP_INDEX(exception) (exception & 0x1FF)


/* IPE module fatal exception meanings definition */
enum sys_fatal_exception_e
{
    SYS_FATAL_EXCP_UC_IP_HDR_ERR_OR_IP_MARTION_ADDR = 0,
    SYS_FATAL_EXCP_UC_IP_OPTIONS                    = 1,
    SYS_FATAL_EXCP_UC_GRE_UNKNOWN_OPTION_OR_PTL     = 2,
    SYS_FATAL_EXCP_UC_ISATAP_SRC_ADD_CHECK_FAIL     = 3,
    SYS_FATAL_EXCP_UC_IP_TTL_CHECK_FAIL             = 4,
    SYS_FATAL_EXCP_UC_RPF_CHECK_FAIL                = 5,
    SYS_FATAL_EXCP_UC_OR_MC_MORE_RPF                = 6,
    SYS_FATAL_EXCP_UC_LINK_ID_CHECK_FAIL            = 7,
    SYS_FATAL_EXCP_MPLS_LABEL_OUT_OF_RANGE          = 8,
    SYS_FATAL_EXCP_MPLS_SBIT_ERROR                  = 9,
    SYS_FATAL_EXCP_MPLS_TTL_CHECK_FAIL              = 10,
    SYS_FATAL_EXCP_FCOE_VIRTUAL_LINK_CHECK_FAIL     = 11,
    SYS_FATAL_EXCP_IGMP_SNOOPED_PACKET              = 12,
    SYS_FATAL_EXCP_TRILL_OPTION                     = 13,
    SYS_FATAL_EXCP_TRILL_TTL_CHECK_FAIL             = 14,
    SYS_FATAL_EXCP_VXLAN_OR_NVGRE_CHK_FAIL          = 15
};
typedef enum sys_fatal_exception_e sys_fatal_exception_t;

enum exception_type_ipe_e
{
    SYS_NORMAL_EXCP_TYPE_LAYER2             = 0,
    SYS_NORMAL_EXCP_TYPE_LAYER3             = 64,
    SYS_NORMAL_EXCP_TYPE_IPE_OTHER          = 128,
    SYS_NORMAL_EXCP_TYPE_EPE_OTHER          = 192,

    SYS_NORMAL_EXCP_TYPE_IPE_OTHER_UNUSED0  = 146,
    SYS_NORMAL_EXCP_TYPE_IPE_OTHER_UNUSED1  = 149,
    SYS_NORMAL_EXCP_TYPE_IPE_OTHER_UNUSED2  = 151,
    SYS_NORMAL_EXCP_TYPE_IPE_OTHER_UNUSED3  = 158,

    SYS_NORMAL_EXCP_TYPE_EPE_L2SPAN_BASE    = 224,
    SYS_NORMAL_EXCP_TYPE_EPE_PORT_LOG       = 244,
    SYS_NORMAL_EXCP_TYPE_EPE_IPFIX_LOG   = 245,
    SYS_NORMAL_EXCP_TYPE_EPE_IPFIX_CONFLICT = 246,
    SYS_NORMAL_EXCP_TYPE_EPE_DISCARD        = 247,
    SYS_NORMAL_EXCP_TYPE_BUFFER_LOG = 248,
    SYS_NORMAL_EXCP_TYPE_LATENCY_LOG   = 249,

    SYS_NORMAL_EXCP_TYPE_IPE_PORT_LOG = 296,
    SYS_NORMAL_EXCP_TYPE_IPE_IPFIX_LOG = 297,
    SYS_NORMAL_EXCP_TYPE_IPE_IPFIX_CONFLICT = 298,
    SYS_NORMAL_EXCP_TYPE_IPE_DEBUG_SESSION = 300,

    SYS_NORMAL_EXCP_TYPE_IPE_DISCARD = 302,
    SYS_NORMAL_EXCP_TYPE_IPE_ELEPHANT_FLOW_LOG = 303
};
typedef enum exception_type_ipe_e exception_type_ipe_t;


enum sys_normal_excp2_sub_type_e
{
    /* 0 -- 31 User define,32~47 reserved */

    SYS_NORMAL_EXCP_SUB_TYPE_l2_UNUSED_END          = 47,  /*The end of unused l2 exceptionSubIndex, use this flag for alloc unused exceptionSubIndex*/
    SYS_NORMAL_EXCP_PON_PT_PKT                      = 48,
    SYS_NORMAL_EXCP_SUB_TYPE_IGMP_SNOOPED_PACKET    = 49,  /*bridge packet used*/
    SYS_NORMAL_EXCP_SUB_TYPE_PIM_SNOOPED_PACKET        = 50,
    SYS_NORMAL_EXCP_SUB_TYPE_UNKNOWN_DOT11_PKT      = 51,
    SYS_NORMAL_EXCP_SUB_TYPE_LEARNING_CONFLICT      = 52,
    SYS_NORMAL_EXCP_SUB_TYPE_MACDA_SELF_ADRESS      = 53,
    SYS_NORMAL_EXCP_SUB_TYPE_MACSA_SELF_ADRESS      = 54,
    SYS_NORMAL_EXCP_SUB_TYPE_SYS_SECURITY           = 55,
    SYS_NORMAL_EXCP_SUB_TYPE_PARSER_LEN_ERR_EXCP    = 56,
    SYS_NORMAL_EXCP_SUB_TYPE_PTP_EXCP               = 57,
    SYS_NORMAL_EXCP_SUB_TYPE_STORM_CTL         = 58,
    SYS_NORMAL_EXCP_SUB_TYPE_VSI_SECURITY      = 59,
    SYS_NORMAL_EXCP_SUB_TYPE_PROFILE_SECURITY  = 60,
    SYS_NORMAL_EXCP_SUB_TYPE_PORT_SECURITY     = 61,
    SYS_NORMAL_EXCP_SUB_TYPE_SRC_MISMATCH      = 62,
    SYS_NORMAL_EXCP_SUB_TYPE_MAC_SA            = 63
};
typedef enum sys_normal_excp2_sub_type_e sys_normal_excp2_sub_type_t;

enum sys_normal_excp3_sub_type_e
{
    SYS_NORMAL_EXCP_SUB_TYPE_l3_UNUSED_END    = 49,  /*The end of unused l3 exceptionSubIndex, use this flag for alloc unused exceptionSubIndex*/

    SYS_NORMAL_EXCP_UC_IP_OPTIONS             = 50,
    SYS_NORMAL_EXCP_UC_IP_TTL_CHECK_FAIL      = 51,
    SYS_NORMAL_EXCP_UC_LINK_ID_CHECK_FAIL     = 52,
    SYS_NORMAL_EXCP_MPLS_TTL_CHECK_FAIL       = 53,
    SYS_NORMAL_EXCP_IGMP_SNOOPED_PACKET       = 54,

    SYS_NORMAL_EXCP_SUB_TYPE_ICMP_ERR_MSG     = 55,
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_PKT_ERR     = 56,
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_TUNNEL_ERR  = 57,
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_FRAG_EXCP   = 58,
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_KEEP_ALIVE  = 59,
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DTLS  = 60,
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_CTL  = 61,
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_CLIENT_ERR = 62,
    SYS_NORMAL_EXCP_SUB_TYPE_IP_DA            = 63
};
typedef enum sys_normal_excp3_sub_type_e sys_normal_excp3_sub_type_t;

enum sys_normal_excp_ipe_other_sub_type_e
{
    SYS_NORMAL_EXCP_SUB_TYPE_SGMAC_CTL_MSG         = 0,
    SYS_NORMAL_EXCP_SUB_TYPE_IPE_PARSER_LEN_ERR    = 1,
    SYS_NORMAL_EXCP_SUB_TYPE_PBB_TCI_NCA           = 2,
    SYS_NORMAL_EXCP_SUB_TYPE_PTP_EXCP_TO_CPU       = 3,
    SYS_NORMAL_EXCP_SUB_TYPE_PTP_FATAL_EXCP_TO_CPU = 4,
    SYS_NORMAL_EXCP_SUB_TYPE_PLD_OFFSET_ERR        = 5,
    SYS_NORMAL_EXCP_SUB_TYPE_INGRESS_ACL           = 6,
    SYS_NORMAL_EXCP_SUB_TYPE_ENTROPY_LABEL_ERROR   = 7,
    SYS_NORMAL_EXCP_SUB_TYPE_ETHERNET_OAM_ERR      = 8,
    SYS_NORMAL_EXCP_SUB_TYPE_MPLS_OAM_TTL_CHK_FAIL = 9,
    SYS_NORMAL_EXCP_SUB_TYPE_MPLS_TMPLS_OAM        = 10,
    SYS_NORMAL_EXCP_SUB_TYPE_DCN                   = 11,
    SYS_NORMAL_EXCP_SUB_TYPE_VXLAN_NVGRE_INNER_VTAG_CHK = 12,
    SYS_NORMAL_EXCP_SUB_TYPE_ACH_ERR               = 13,
    SYS_NORMAL_EXCP_SUB_TYPE_SECTION_OAM_EXP       = 14,
    SYS_NORMAL_EXCP_SUB_TYPE_INGRESS_OAM_HASH_CONFLICT   = 15,
    SYS_NORMAL_EXCP_SUB_TYPE_NO_MEP_MIP_DIS        = 16,
    SYS_NORMAL_EXCP_SUB_TYPE_MPLS_ALERT_LABEL      = 17,
    /* 18 reserved */
    SYS_NORMAL_EXCP_SUB_TYPE_NEW_ELEPHANT_FLOW     = 19,
    SYS_NORMAL_EXCP_SUB_TYPE_VXLAN_FLAG_CHK_ERROR = 20,
    SYS_NORMAL_EXCP_SUB_TYPE_QUE_DROP_SPAN_EXCP = 21,

    SYS_NORMAL_EXCP_SUB_TYPE_GENEVE_PKT_CPU_PROCESS    = 22,
    /* 23 reserved */
    SYS_NORMAL_EXCP_SUB_TYPE_USERID_EXCEPTION = 24,
    SYS_NORMAL_EXCP_SUB_TYPE_TUNNELID_EXCEPTION = 25,
    SYS_NORMAL_EXCP_SUB_TYPE_PROTOCOL_VLAN = 26,
    SYS_NORMAL_EXCP_SUB_TYPE_LEARNING_CACHE_FULL = 27,
    SYS_NORMAL_EXCP_SUB_TYPE_ICMP_REDIRECT = 28,
    SYS_NORMAL_EXCP_SUB_TYPE_ROUTE_MCAST_RPF_FAIL = 29,
    /* 30 reserved */
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_ENCRYPT_GENERAL_ERROR      = 31,
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DECRYPT_GENERAL_ERROR      = 32,
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DECRYPT_MAC_CHECK_ERROR    = 33,
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DECRYPT_SEQ_CHECK_ERROR    = 34,
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DECRYPT_EPOCH_CHECK_ERROR  = 35,
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_REASSEMBLE_GENERAL_ERROR   = 36,
    SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_ENCRYPT_GENERAL_ERROR    = 37,
    SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_DECRYPT_UNKNOWN_PACKET   = 38,
    SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_DECRYPT_ICV_CHECK_ERROR  = 39,
    SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_DECRYPT_CIPHER_MODE_MIS  = 40,
    SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_DECRYPT_PN_CHECK_ERROR   = 41,
    SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_PACKET_SCI_ERROR         = 42,
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DECRYPT_SN_THRESHOLD_MATCH = 43,
    SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_DECRYPT_REACH_PN_THRD    = 44,
    SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_DECRYPT_ICV_CHECK_FAIL   = 45,
    SYS_NORMAL_EXCP_SUB_TYPE_DEST_MAP_PROFILE_EXCEPTION = 46,
    SYS_NORMAL_EXCP_SUB_TYPE_IPE_OTHER_UNUSED_BEGIN     = 47,  /*The begin of unused ipe other exceptionSubIndex, use this flag for alloc unused exceptionSubIndex*/
    /* 47~56 reserved */
    SYS_NORMAL_EXCP_SUB_TYPE_IPE_OTHER_UNUSED_END       = 56,  /*The end of unused ipe other exceptionSubIndex, use this flag for alloc unused exceptionSubIndex*/
    SYS_NORMAL_EXCP_UNKNOWN_GEM_PORT                    = 57,
    SYS_NORMAL_EXCP_SUB_TYPE_UNKNOWN_DOT1AE_PKT         = 58,
    SYS_NORMAL_EXCP_SUB_TYPE_UNKNOWN_DOT1AE_SCI         = 59,
    SYS_NORMAL_EXCP_SUB_TYPE_MCAST_TUNNEL_FRAG          = 60,
    SYS_NORMAL_EXCP_SUB_TYPE_TUNNEL_ECN_MIS             = 61,
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DOT11_MGR             = 62,
    SYS_NORMAL_EXCP_SUB_TYPE_WLAN_CIPHER_MISMATCH       = 63
};
typedef enum sys_normal_excp_ipe_other_sub_type_e sys_normal_excp_ipe_other_sub_type_t;

enum sys_normal_excp_epe_other_sub_type_e
{
    SYS_NORMAL_EXCP_SUB_TYPE_EGRESS_VLANXLATE_EXCEPTION = 0,
    SYS_NORMAL_EXCP_SUB_TYPE_EPE_PARSER_LEN_ERR = 1,
    SYS_NORMAL_EXCP_SUB_TYPE_TTL_CHECK_FAIL = 2,
    SYS_NORMAL_EXCP_SUB_TYPE_EGRESS_OAM_HASH_CONFLICT = 3,
    SYS_NORMAL_EXCP_SUB_TYPE_MTU_CHECK_FAIL = 4,
    SYS_NORMAL_EXCP_SUB_TYPE_STRIP_OFFSET_ERROR = 5,
    SYS_NORMAL_EXCP_SUB_TYPE_ICMP_ERROR_MESSAGE = 6,
    SYS_NORMAL_EXCP_SUB_TYPE_EGRESS_ACL = 7,
    SYS_NORMAL_EXCP_SUB_TYPE_UNUSER0 = 8,
    SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_ENCRYPT_REACH_PN_THRD = 9,
    SYS_NORMAL_EXCP_SUB_TYPE_ARP_MIS = 10,
    SYS_NORMAL_EXCP_SUB_TYPE_EPE_UNKNOWN_DOT11_PKT      = 11,
    SYS_NORMAL_EXCP_SUB_TYPE_EPE_OTHER_UNUSED_BEGIN     = 12,  /*The begin of unused epe other exceptionSubIndex, use this flag for alloc unused exceptionSubIndex*/

    SYS_NORMAL_EXCP_SUB_TYPE_EPE_OTHER_UNUSED_END       = 31   /*The end of unused epe other exceptionSubIndex, use this flag for alloc unused exceptionSubIndex*/
};
typedef enum sys_normal_excp_epe_other_sub_type_e sys_normal_excp_epe_other_sub_type_t;

#define SYS_CPU_OAM_EXCP_NUM 37

extern sys_queue_master_t* p_usw_queue_master[CTC_MAX_LOCAL_CHIP_NUM];
extern int32 sys_usw_nh_update_entry_dsfwd(uint8 lchip, uint32 *offset,uint32 dest_map,uint32 dsnh_offset,
                                                     uint8 dsnh_8w,uint8 del,uint8 critical_packet);

int32
sys_usw_cpu_reason_get_nexthop_info(uint8 lchip, uint16 reason_id,
                                           uint32* nexthop_ptr,uint8* dsnh_8w)
{
    sys_cpu_reason_t* p_reason = NULL;
    uint8 dest_type = 0;
    uint32 nhid = 0;
    sys_nh_info_dsnh_t nh_info;

    sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));
    p_reason = &p_usw_queue_master[lchip]->cpu_reason[reason_id];
    dest_type = p_reason->dest_type;
    if(dest_type == CTC_PKT_CPU_REASON_TO_LOCAL_PORT)
    {
        CTC_ERROR_RETURN(sys_usw_l2_get_ucast_nh(lchip, p_reason->dest_port, CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS, &nhid));
        /*get nexthop info from nexthop id */
        CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, nhid, &nh_info, 1));
    }
    else if(dest_type == CTC_PKT_CPU_REASON_TO_NHID)
    {
        CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, p_reason->dest_port, &nh_info, 1));
    }
    else
    {
        *nexthop_ptr = CTC_PKT_CPU_REASON_BUILD_NHPTR(reason_id, 1);
    }
    if(dest_type == CTC_PKT_CPU_REASON_TO_LOCAL_PORT || dest_type == CTC_PKT_CPU_REASON_TO_NHID)
    {
        *nexthop_ptr = nh_info.dsnh_offset;
        CTC_BIT_UNSET(*nexthop_ptr, 17); /* bit 17 used for bypass ingress edit */
        *dsnh_8w = nh_info.nexthop_ext;
    }

    return CTC_E_NONE;
}
int32
sys_usw_queue_get_excp_idx_by_reason(uint8 lchip, uint16 reason_id,
                                           uint16* excp_idx,
                                           uint8* excp_num)
{
    sys_cpu_reason_t *p_cpu_reason = NULL;
    uint8 idx = 0;
    uint8 i = 0;
    uint8 oam_exception[SYS_CPU_OAM_EXCP_NUM] = /*indicate the value in ctc_oam_exceptions_t*/
    {
       0, /*SYS_OAM_EXCP_NONE             = 0,  for relay all to cpu, use DsOamExcp 0*/
       0, /*SYS_OAM_EXCP_ETH_INVALID      = 1,  CTC_OAM_EXCP_INVALID_OAM_PDU*/
       2, /*SYS_OAM_EXCP_ETH_LB           = 2,  CTC_OAM_EXCP_LBM*/
      24, /*SYS_OAM_EXCP_ETH_LT           = 3,  CTC_OAM_EXCP_EQUAL_LTM_LTR_TO_CPU*/
      27, /*SYS_OAM_EXCP_ETH_LM           = 4,  CTC_OAM_EXCP_LM_TO_CPU*/
      18, /*SYS_OAM_EXCP_ETH_DM           = 5,  CTC_OAM_EXCP_DM_TO_CPU*/
      10, /*SYS_OAM_EXCP_ETH_TST          = 6,  CTC_OAM_EXCP_ETH_TST_TO_CPU*/
      16, /*SYS_OAM_EXCP_ETH_APS          = 7,  CTC_OAM_EXCP_APS_PDU_TO_CPU*/
      31, /*SYS_OAM_EXCP_ETH_SCC          = 8,  CTC_OAM_EXCP_SCC_PDU_TO_CPU*/
      22, /*SYS_OAM_EXCP_ETH_MCC          = 9,  CTC_OAM_EXCP_MCC_PDU_TO_CPU*/
      20, /*SYS_OAM_EXCP_ETH_CSF          = 10, CTC_OAM_EXCP_CSF_TO_CPU*/
      12, /*SYS_OAM_EXCP_ETH_BIG_CCM      = 11, CTC_OAM_EXCP_BIG_INTERVAL_OR_SW_MEP_TO_CPU*/
      19, /*SYS_OAM_EXCP_ETH_LEARN_CCM    = 12, CTC_OAM_EXCP_LEARNING_CCM_TO_CPU*/
       1, /*SYS_OAM_EXCP_ETH_DEFECT       = 13, CTC_OAM_EXCP_ALL_DEFECT*/
      23, /*SYS_OAM_EXCP_PBX_OAM          = 14, CTC_OAM_EXCP_PBT_MM_DEFECT_OAM_PDU*/
       3, /*SYS_OAM_EXCP_ETH_HIGH_VER     = 15, CTC_OAM_EXCP_HIGH_VER_OAM_TO_CPU*/
       8, /*SYS_OAM_EXCP_ETH_TLV          = 16, CTC_OAM_EXCP_OPTION_CCM_TLV*/
      17, /*SYS_OAM_EXCP_ETH_OTHER        = 17, CTC_OAM_EXCP_UNKNOWN_PDU*/
       0, /*SYS_OAM_EXCP_BFD_INVALID      = 18, CTC_OAM_EXCP_INVALID_OAM_PDU*/
      28, /*SYS_OAM_EXCP_BFD_LEARN        = 19, CTC_OAM_EXCP_LEARNING_BFD_TO_CPU*/
      12, /*SYS_OAM_EXCP_BIG_BFD          = 20, CTC_OAM_EXCP_BIG_INTERVAL_OR_SW_MEP_TO_CPU*/
      30, /*SYS_OAM_EXCP_BFD_TIMER_NEG    = 21, CTC_OAM_EXCP_BFD_TIMER_NEGOTIATION*/
      31, /*SYS_OAM_EXCP_TP_SCC           = 22, CTC_OAM_EXCP_SCC_PDU_TO_CPU*/
      22, /*SYS_OAM_EXCP_TP_MCC           = 23, CTC_OAM_EXCP_MCC_PDU_TO_CPU*/
       5, /*SYS_OAM_EXCP_TP_CSF           = 24, CTC_OAM_EXCP_TP_CSF*/
       4, /*SYS_OAM_EXCP_TP_LB            = 25, CTC_OAM_EXCP_TP_LBM*/
      14, /*SYS_OAM_EXCP_TP_DLM           = 26, CTC_OAM_EXCP_MPLS_TP_DLM_TO_CPU*/
      15, /*SYS_OAM_EXCP_TP_DM            = 27, CTC_OAM_EXCP_MPLS_TP_DM_DLMDM_TO_CPU*/
       9, /*SYS_OAM_EXCP_TP_FM            = 28, CTC_OAM_EXCP_TP_FM*/
      17, /*SYS_OAM_EXCP_BFD_OTHER        = 29, CTC_OAM_EXCP_UNKNOWN_PDU*/
      21, /*SYS_OAM_EXCP_BFD_DISC_MISMATCH= 30, CTC_OAM_EXCP_BFD_DISC_MISMATCH*/
      11, /*SYS_OAM_EXCP_ETH_SM           = 31, CTC_OAM_EXCP_SM*/
      13, /*SYS_OAM_EXCP_TP_CV            = 32, CTC_OAM_EXCP_TP_BFD_CV*/
      29, /*SYS_OAM_EXCP_TWAMP            = 33, CTC_OAM_EXCP_TWAMP_TO_CPU*/
      6, /*SYS_OAM_EXCP_ETH_SC           = 34, CTC_OAM_EXCP_ETH_SC_TO_CPU*/
      7, /*SYS_OAM_EXCP_ETH_LL           = 35, CTC_OAM_EXCP_ETH_LL_TO_CPU*/
      26  /*SYS_OAM_EXCP_MAC_FAIL          = 36  CTC_OAM_EXCP_MACDA_CHK_FAIL*/
    };

    switch (reason_id)
    {
    case CTC_PKT_CPU_REASON_IGMP_SNOOPING:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_SUB_TYPE_IGMP_SNOOPED_PACKET);

        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                SYS_FATAL_EXCP_IGMP_SNOOPED_PACKET,
                                                0);

        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER3,
                                                SYS_NORMAL_EXCP_IGMP_SNOOPED_PACKET);


        break;

    case CTC_PKT_CPU_REASON_PIM_SNOOPING:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_SUB_TYPE_PIM_SNOOPED_PACKET);
        break;

    case CTC_PKT_CPU_REASON_PTP:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_SUB_TYPE_PTP_EXCP);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_PTP_EXCP_TO_CPU);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_PTP_FATAL_EXCP_TO_CPU);
        break;

    case CTC_PKT_CPU_REASON_MPLS_OAM:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_MPLS_TMPLS_OAM);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_MPLS_OAM_TTL_CHK_FAIL);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER ,
                                                SYS_NORMAL_EXCP_SUB_TYPE_ACH_ERR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER ,
                                                SYS_NORMAL_EXCP_SUB_TYPE_MPLS_ALERT_LABEL);
        break;

    case CTC_PKT_CPU_REASON_DCN:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_DCN);
        break;

    case CTC_PKT_CPU_REASON_SCL_MATCH:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_USERID_EXCEPTION);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_TUNNELID_EXCEPTION);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_EPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_EGRESS_VLANXLATE_EXCEPTION);
        break;

    case CTC_PKT_CPU_REASON_ACL_MATCH:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_INGRESS_ACL);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_EPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_EGRESS_ACL);
        break;

    case CTC_PKT_CPU_REASON_SFLOW_SOURCE:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_PORT_LOG,
                                                0);
        break;

    case CTC_PKT_CPU_REASON_SFLOW_DEST:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_EPE_PORT_LOG,
                                                0);
        break;

    case CTC_PKT_CPU_REASON_L2_SYSTEM_LEARN_LIMIT:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_SUB_TYPE_SYS_SECURITY);
        break;

    case CTC_PKT_CPU_REASON_L2_VLAN_LEARN_LIMIT:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_SUB_TYPE_VSI_SECURITY);
        break;

    case CTC_PKT_CPU_REASON_L2_PORT_LEARN_LIMIT:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_SUB_TYPE_PORT_SECURITY);
        break;

    case CTC_PKT_CPU_REASON_L2_MOVE:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_SUB_TYPE_SRC_MISMATCH);
        break;

    case CTC_PKT_CPU_REASON_L2_CPU_LEARN:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_LEARNING_CACHE_FULL);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_SUB_TYPE_LEARNING_CONFLICT);
        break;

    case CTC_PKT_CPU_REASON_L2_COPY_CPU:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_SUB_TYPE_MAC_SA);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_SUB_TYPE_MACDA_SELF_ADRESS);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_SUB_TYPE_MACSA_SELF_ADRESS);
        break;

    case CTC_PKT_CPU_REASON_L3_URPF_FAIL:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                SYS_FATAL_EXCP_UC_RPF_CHECK_FAIL,
                                                0);
        break;

    case CTC_PKT_CPU_REASON_L3_MTU_FAIL:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_EPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_MTU_CHECK_FAIL);
        break;

    case CTC_PKT_CPU_REASON_L3_MC_RPF_FAIL:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_ROUTE_MCAST_RPF_FAIL);
        break;

    case CTC_PKT_CPU_REASON_TUNNEL_MORE_RPF:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                SYS_FATAL_EXCP_UC_OR_MC_MORE_RPF,
                                                0);
        break;

    case CTC_PKT_CPU_REASON_L3_IP_OPTION:
        if (DRV_IS_DUET2(lchip))
        {
            excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                    SYS_FATAL_EXCP_UC_IP_OPTIONS,
                                                    0);
        }
        else
        {
            excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                    SYS_NORMAL_EXCP_TYPE_LAYER3,
                                                    SYS_NORMAL_EXCP_UC_IP_OPTIONS);
        }
        break;

    case CTC_PKT_CPU_REASON_L3_ICMP_REDIRECT:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_ICMP_REDIRECT);
        break;

    case CTC_PKT_CPU_REASON_L3_COPY_CPU:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER3,
                                                SYS_NORMAL_EXCP_SUB_TYPE_IP_DA);
        break;

    case CTC_PKT_CPU_REASON_L3_MARTIAN_ADDR:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                SYS_FATAL_EXCP_UC_IP_HDR_ERR_OR_IP_MARTION_ADDR,
                                                0);
        break;

    case CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL:
        if (DRV_IS_DUET2(lchip))
        {
            excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                    SYS_FATAL_EXCP_UC_IP_TTL_CHECK_FAIL,
                                                    0);
        }
        else
        {
            excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                    SYS_NORMAL_EXCP_TYPE_LAYER3,
                                                    SYS_NORMAL_EXCP_UC_IP_TTL_CHECK_FAIL);
        }
        break;

    case CTC_PKT_CPU_REASON_MPLS_TTL_CHECK_FAIL:
        if (DRV_IS_DUET2(lchip))
        {
            excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                    SYS_FATAL_EXCP_MPLS_TTL_CHECK_FAIL,
                                                    0);
        }
        else
        {
            excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                    SYS_NORMAL_EXCP_TYPE_LAYER3,
                                                    SYS_NORMAL_EXCP_MPLS_TTL_CHECK_FAIL);
        }
        break;

    case CTC_PKT_CPU_REASON_EPE_TTL_CHECK_FAIL:  /* equal to CTC_PKT_CPU_REASON_IPMC_TTL_CHECK_FAIL*/
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_EPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_TTL_CHECK_FAIL);
        break;

    case CTC_PKT_CPU_REASON_GRE_UNKNOWN:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                SYS_FATAL_EXCP_UC_GRE_UNKNOWN_OPTION_OR_PTL,
                                                0);
        break;

    case CTC_PKT_CPU_REASON_LABEL_MISS:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                SYS_FATAL_EXCP_MPLS_LABEL_OUT_OF_RANGE,
                                                0);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                SYS_FATAL_EXCP_MPLS_SBIT_ERROR,
                                                0);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER ,
                                                SYS_NORMAL_EXCP_SUB_TYPE_ENTROPY_LABEL_ERROR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER ,
                                                SYS_NORMAL_EXCP_SUB_TYPE_SECTION_OAM_EXP);
        break;

    case CTC_PKT_CPU_REASON_LINK_ID_FAIL:
        if (DRV_IS_DUET2(lchip))
        {
            excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                    SYS_FATAL_EXCP_UC_LINK_ID_CHECK_FAIL,
                                                    0);
        }
        else
        {
            excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                    SYS_NORMAL_EXCP_TYPE_LAYER3,
                                                    SYS_NORMAL_EXCP_UC_LINK_ID_CHECK_FAIL);
        }
        break;

    case CTC_PKT_CPU_REASON_OAM_HASH_CONFLICT:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_INGRESS_OAM_HASH_CONFLICT);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_EPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_EGRESS_OAM_HASH_CONFLICT);
        break;

    case CTC_PKT_CPU_REASON_NEW_ELEPHANT_FLOW:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_NEW_ELEPHANT_FLOW);
        break;

    case CTC_PKT_CPU_REASON_VXLAN_NVGRE_CHK_FAIL:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                SYS_FATAL_EXCP_VXLAN_OR_NVGRE_CHK_FAIL,
                                                0);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_VXLAN_NVGRE_INNER_VTAG_CHK);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_VXLAN_FLAG_CHK_ERROR);
        break;

    case CTC_PKT_CPU_REASON_L2_STORM_CTL:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_SUB_TYPE_STORM_CTL);
        break;

    case CTC_PKT_CPU_REASON_MONITOR_BUFFER_LOG:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_BUFFER_LOG,
                                                0);
        break;

    case CTC_PKT_CPU_REASON_MONITOR_LATENCY_LOG:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LATENCY_LOG,
                                                0);
        break;

    case CTC_PKT_CPU_REASON_QUEUE_DROP_PKT:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_EPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_QUE_DROP_SPAN_EXCP);
        break;

    case CTC_PKT_CPU_REASON_GENEVE_PKT:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_GENEVE_PKT_CPU_PROCESS);

         break;

    case CTC_PKT_CPU_REASON_OAM_DEFECT_MESSAGE:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_OAM_EXCP,
                                                reason_id - CTC_PKT_CPU_REASON_OAM,
                                                MCHIP_CAP(SYS_CAP_CPU_REASON_OAM_DEFECT_MESSAGE_BASE));
        break;

    case CTC_PKT_CPU_REASON_ARP_MISS:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_DEST_MAP_PROFILE_EXCEPTION);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_EPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_ARP_MIS);
        break;

    case CTC_PKT_CPU_REASON_WLAN_KEPP_ALIVE:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER3,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_KEEP_ALIVE);
        break;

    case CTC_PKT_CPU_REASON_WLAN_DTLS:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER3,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DTLS);
        break;

    case CTC_PKT_CPU_REASON_WLAN_CTL:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER3,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_CTL);
        break;

    case CTC_PKT_CPU_REASON_WLAN_DOT11_MGR:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DOT11_MGR);
        break;

    case CTC_PKT_CPU_REASON_WLAN_CHECK_ERR:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_CIPHER_MISMATCH);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER3,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_CLIENT_ERR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_SUB_TYPE_UNKNOWN_DOT11_PKT);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER3,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_FRAG_EXCP);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER3,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_TUNNEL_ERR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER3,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_PKT_ERR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_ENCRYPT_GENERAL_ERROR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DECRYPT_GENERAL_ERROR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DECRYPT_MAC_CHECK_ERROR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DECRYPT_SEQ_CHECK_ERROR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DECRYPT_EPOCH_CHECK_ERROR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_REASSEMBLE_GENERAL_ERROR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DECRYPT_SN_THRESHOLD_MATCH);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_EPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_EPE_UNKNOWN_DOT11_PKT);
        break;

    case CTC_PKT_CPU_REASON_IPFIX_HASH_CONFLICT:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_IPFIX_CONFLICT,
                                                0);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_EPE_IPFIX_CONFLICT,
                                                0);
        break;

    case CTC_PKT_CPU_REASON_ICMP_ERR_MSG:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER3,
                                                SYS_NORMAL_EXCP_SUB_TYPE_ICMP_ERR_MSG);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_EPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_ICMP_ERROR_MESSAGE);
        break;

    case CTC_PKT_CPU_REASON_TUNNEL_ECN_MIS:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_TUNNEL_ECN_MIS);
        break;

    case CTC_PKT_CPU_REASON_MCAST_TUNNEL_FRAG:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_MCAST_TUNNEL_FRAG);
        break;

    case CTC_PKT_CPU_REASON_DOT1AE_CHECK_ERR:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_UNKNOWN_DOT1AE_SCI);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_UNKNOWN_DOT1AE_PKT);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_ENCRYPT_GENERAL_ERROR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_DECRYPT_UNKNOWN_PACKET);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_DECRYPT_ICV_CHECK_ERROR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_DECRYPT_CIPHER_MODE_MIS);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_DECRYPT_PN_CHECK_ERROR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_PACKET_SCI_ERROR);
        break;

    case CTC_PKT_CPU_REASON_DOT1AE_REACH_PN_THRD:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_DECRYPT_REACH_PN_THRD);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_EPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_ENCRYPT_REACH_PN_THRD);
        break;

    case CTC_PKT_CPU_REASON_DOT1AE_DECRYPT_ICV_CHK_FAIL:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_DOT1AE_DECRYPT_ICV_CHECK_FAIL);
        break;

    case CTC_PKT_CPU_REASON_DKITS_CAPTURE_PKT:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_DEBUG_SESSION,
                                                0);
        break;

    case CTC_PKT_CPU_REASON_ELEPHANT_FLOW_LOG:
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_ELEPHANT_FLOW_LOG,
                                                0);
        break;
    case CTC_PKT_CPU_REASON_DIAG_DISCARD_PKT:
               excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_DISCARD,
                                                0);
               excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_EPE_DISCARD,
                                                0);
        break;
    case CTC_PKT_CPU_REASON_UNKNOWN_GEM_PORT:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_UNKNOWN_GEM_PORT);
        break;
    case CTC_PKT_CPU_REASON_PON_PT_PKT:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_PON_PT_PKT);
        break;
    default:
        break;
    }

    /*IPE l2Pdu (exception2)*/
    if (reason_id >= CTC_PKT_CPU_REASON_L2_PDU &&
        reason_id < (CTC_PKT_CPU_REASON_L2_PDU + CTC_PKT_CPU_REASON_L2_PDU_RESV))
    {
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                reason_id - CTC_PKT_CPU_REASON_L2_PDU);
    }

    /*IPE l3Pdu (exception3)*/
    if (reason_id >= CTC_PKT_CPU_REASON_L3_PDU &&
        reason_id < (CTC_PKT_CPU_REASON_L3_PDU + CTC_PKT_CPU_REASON_L3_PDU_RESV))
    {
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER3,
                                                reason_id - CTC_PKT_CPU_REASON_L3_PDU);
    }

    /* oam exception*/
    if (reason_id >= CTC_PKT_CPU_REASON_OAM &&
        reason_id < (CTC_PKT_CPU_REASON_OAM + CTC_PKT_CPU_REASON_OAM_PDU_RESV))
    {
        for (i = 0; i < SYS_CPU_OAM_EXCP_NUM; i++)
        {
            if ((reason_id - CTC_PKT_CPU_REASON_OAM) == oam_exception[i])
            {
                excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_OAM_EXCP,
                                                        i,
                                                        0);
            }
        }

        if (idx == 0)
        {
            return CTC_E_NONE;
        }
    }

    /* forward to cpu*/
    if (reason_id == CTC_PKT_CPU_REASON_FWD_CPU)
    {
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FWD_EXCP,
                                                0,
                                                0);
    }

    if (reason_id == CTC_PKT_CPU_REASON_DROP)
    {
        /*fatal*/
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                SYS_FATAL_EXCP_UC_ISATAP_SRC_ADD_CHECK_FAIL,
                                                0);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                SYS_FATAL_EXCP_FCOE_VIRTUAL_LINK_CHECK_FAIL,
                                                0);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                SYS_FATAL_EXCP_TRILL_OPTION,
                                                0);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_FATAL_EXCP,
                                                SYS_FATAL_EXCP_TRILL_TTL_CHECK_FAIL,
                                                0);
        /*normal*/
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_PROTOCOL_VLAN);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_IPE_PARSER_LEN_ERR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_EPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_EPE_PARSER_LEN_ERR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_EPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_STRIP_OFFSET_ERROR);
        /*exception =2 */
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_SUB_TYPE_PARSER_LEN_ERR_EXCP);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_LAYER2,
                                                SYS_NORMAL_EXCP_SUB_TYPE_PROFILE_SECURITY);
        /*other */
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_SGMAC_CTL_MSG);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_PBB_TCI_NCA);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_PLD_OFFSET_ERR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_ETHERNET_OAM_ERR);
        excp_idx[idx++] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,
                                                SYS_NORMAL_EXCP_TYPE_IPE_OTHER,
                                                SYS_NORMAL_EXCP_SUB_TYPE_NO_MEP_MIP_DIS);
    }

   if (reason_id >= CTC_PKT_CPU_REASON_CUSTOM_BASE && reason_id < CTC_PKT_CPU_REASON_MAX_COUNT)
   {
        p_cpu_reason = &p_usw_queue_master[lchip]->cpu_reason[reason_id];
        if (p_cpu_reason->user_define_mode == 1)
        {
            excp_idx[0] = SYS_REASON_ENCAP_EXCP(SYS_REASON_MISC_NH_EXCP, 0, 0);
            idx = 1;
        }
        else if ((p_cpu_reason->user_define_mode == 2) && (p_cpu_reason->excp_id != 0))
        {
            excp_idx[0] = SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP, p_cpu_reason->excp_id, 0);
            idx = 1;
        }
   }

   *excp_num = idx;
   SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO,  "excp_num = %d, excp_idx[0] = %d \n", *excp_num, excp_idx[0]);

   if ((0 == idx) && (reason_id < CTC_PKT_CPU_REASON_CUSTOM_BASE))
   {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}

/*****************************************************************************
 Description  : update cpu exception include normal and fatal
*****************************************************************************/
int32
sys_usw_cpu_reason_update_exception(uint8 lchip, uint16 reason_id,
                                          uint32 dest_map,
                                          uint32 nexthop_ptr,
                                          uint32 dsnh_8w,
                                          uint8 dest_type)
{
    uint32 cmd         = 0;
    uint8 idx          = 0;
    uint16 exception   = 0;
    uint16 excp_idx    = 0;
    uint8 excp_num     = 0;
    uint8 excp_type    = 0;
    uint16 excp_array[32] = {0};
    sys_cpu_reason_t* p_reason = NULL;
    uint32 dsfwd_offset;
    uint32 offset = 0;
    uint8 critical_packet = 0;
    uint32 value = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if ((CTC_PKT_CPU_REASON_TO_LOCAL_CPU == dest_type)
            || (CTC_PKT_CPU_REASON_TO_REMOTE_CPU == dest_type)
            || (CTC_PKT_CPU_REASON_TO_LOCAL_CPU_ETH == dest_type))
    {
        critical_packet = 1;
    }

    CTC_ERROR_RETURN(sys_usw_queue_get_excp_idx_by_reason(lchip, reason_id,
                                                                excp_array,
                                                                &excp_num));
    p_reason = &p_usw_queue_master[lchip]->cpu_reason[reason_id];
    for (idx = 0; idx < excp_num; idx++)
    {
        exception = excp_array[idx];

        excp_type = SYS_REASON_GET_EXCP_TYPE(exception);
        excp_idx = SYS_REASON_GET_EXCP_INDEX(exception);

        if (SYS_REASON_FWD_EXCP == excp_type)
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, CTC_NH_RESERVED_NHID_FOR_TOCPU,
                                                                &dsfwd_offset, 0));
            critical_packet = 1;
            CTC_ERROR_RETURN(sys_usw_nh_update_dsfwd(lchip, &dsfwd_offset, dest_map, nexthop_ptr, dsnh_8w, 0, critical_packet));
        }
        else if (SYS_REASON_MISC_NH_EXCP == excp_type)
        {
            /*none*/
        }
        else if (SYS_REASON_OAM_EXCP == excp_type)
        {
            if (excp_idx >=MCHIP_CAP(SYS_CAP_CPU_REASON_OAM_DEFECT_MESSAGE_BASE))
            {
                OamUpdateApsCtl_m aps_ctl;
                cmd = DRV_IOR(OamUpdateApsCtl_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aps_ctl));
                cmd = DRV_IOW(OamUpdateApsCtl_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aps_ctl));
            }
            else
            {
                DsOamExcp_m oam_excp;
                cmd = DRV_IOR(DsOamExcp_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_idx, cmd, &oam_excp));
                SetDsOamExcp(V, destMap_f, &oam_excp, dest_map);
                SetDsOamExcp(V, nextHopPtr_f, &oam_excp, nexthop_ptr);
                cmd = DRV_IOW(DsOamExcp_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_idx, cmd, &oam_excp));
            }
        }
        else if (SYS_REASON_FATAL_EXCP == excp_type)
        {
            /*get fatal exception resver offset*/
            CTC_ERROR_RETURN(sys_usw_nh_get_fatal_excp_dsnh_offset(lchip, &offset));
            offset = offset + excp_idx * 2;
            critical_packet = 1;
            CTC_ERROR_RETURN(sys_usw_nh_update_dsfwd(lchip, &offset, dest_map, nexthop_ptr, dsnh_8w, 0, critical_packet));
        }
        else
        {
            cmd = DRV_IOW(DsMetFifoExcp_t, DsMetFifoExcp_destMap_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_idx, cmd, &dest_map));

            cmd = DRV_IOW(DsMetFifoExcp_t, DsMetFifoExcp_nextHopExt_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_idx, cmd, &dsnh_8w));

            if ((reason_id == CTC_PKT_CPU_REASON_DIAG_DISCARD_PKT)
                && (excp_idx ==  SYS_REASON_ENCAP_EXCP(SYS_REASON_NORMAL_EXCP,SYS_NORMAL_EXCP_TYPE_EPE_DISCARD, 0)))
            {
                /* Special Process for Distinguish EPE Discard Pkt toCPU*/
                value = nexthop_ptr + 1;
                cmd = DRV_IOW(DsBufRetrvExcp_t, DsBufRetrvExcp_nextHopPtr_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_idx, cmd, &value));
            }
            else
            {
                cmd = DRV_IOW(DsBufRetrvExcp_t, DsBufRetrvExcp_nextHopPtr_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_idx, cmd, &nexthop_ptr));
            }

            value = 1;
            cmd = DRV_IOW(DsBufRetrvExcp_t, DsBufRetrvExcp_forceEgressEdit_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_idx, cmd, &value));
        }
    }

    /*CTC_OAM_EXCP_ALL_TO_CPU*/
    if (reason_id == (CTC_PKT_CPU_REASON_OAM+25))
    {
        OamHeaderEditCtl_m hdr_edit_ctl;
        cmd = DRV_IOR(OamHeaderEditCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hdr_edit_ctl));
        SetOamHeaderEditCtl(V, relayAllToCpuDestmap_f, &hdr_edit_ctl, dest_map);
        SetOamHeaderEditCtl(V, relayAllToCpuNexthopPtr_f, &hdr_edit_ctl, nexthop_ptr);
        cmd = DRV_IOW(OamHeaderEditCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hdr_edit_ctl));
    }

    if (reason_id == CTC_PKT_CPU_REASON_ARP_MISS)
    {
        /*destmap profile tocpu for metfifo*/
        cmd = DRV_IOW(MetFifoCtl_t, MetFifoCtl_destMapProfileDiscardDestMap_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dest_map));

        value = (nexthop_ptr<<5);
        cmd = DRV_IOW(MetFifoCtl_t, MetFifoCtl_destMapProfileDiscardReplicationCtl_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    }

    if (reason_id >= CTC_PKT_CPU_REASON_CUSTOM_BASE)  /*the cpu reason is user-defined reason*/
    {
        /*alloc*/
        if (p_reason->user_define_mode == 2)
        {
            return CTC_E_NONE;
        }

        if ((p_reason->user_define_mode == 1) && dest_type != CTC_PKT_CPU_REASON_TO_DROP)
        {
            CTC_ERROR_RETURN(sys_usw_nh_update_dsfwd(lchip, &p_reason->dsfwd_offset, dest_map, nexthop_ptr, dsnh_8w, 0, critical_packet));
        }
        else if ((p_reason->user_define_mode == 1) && p_reason->dsfwd_offset)
        {
            CTC_ERROR_RETURN(sys_usw_nh_update_dsfwd(lchip, &p_reason->dsfwd_offset, dest_map, nexthop_ptr, dsnh_8w, 1, critical_packet));
            p_reason->dsfwd_offset = 0;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_cpu_reason_get_info(uint8 lchip, uint16 reason_id,
                                   uint32 *destmap)
{
    sys_cpu_reason_t* p_reason = NULL;
    CTC_MAX_VALUE_CHECK(reason_id, CTC_PKT_CPU_REASON_MAX_COUNT - 1);
    p_reason = &p_usw_queue_master[lchip]->cpu_reason[reason_id];

    *destmap = p_reason->dest_map;

    return CTC_E_NONE;
}

int32
sys_usw_cpu_reason_get_excp_index_by_excp_id(uint8 lchip, uint16 exception_id,
                                                                     uint8* exception_index, uint8* exception_sub_index)
{
    CTC_PTR_VALID_CHECK(exception_index);
    CTC_PTR_VALID_CHECK(exception_sub_index);

    if (exception_id < SYS_NORMAL_EXCP_TYPE_LAYER3)
    {
        *exception_index = 1;
        *exception_sub_index = exception_id;
    }
    else if (exception_id < SYS_NORMAL_EXCP_TYPE_IPE_OTHER)
    {
        *exception_index = 2;
        *exception_sub_index = exception_id - SYS_NORMAL_EXCP_TYPE_LAYER3;
    }
    else if (exception_id < SYS_NORMAL_EXCP_TYPE_EPE_OTHER)
    {
        *exception_index = 3;
        *exception_sub_index = exception_id - SYS_NORMAL_EXCP_TYPE_IPE_OTHER;
    }
    else if (exception_id < (SYS_NORMAL_EXCP_TYPE_EPE_OTHER + 32))
    {
        *exception_index = 0;
        *exception_sub_index = exception_id - SYS_NORMAL_EXCP_TYPE_EPE_OTHER;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

/*only used to process ACL KEY And AD
  ACL key: excp group id (excp index) || faltal exception index
  ACL AD: excption index & sub index
*/
int32
_sys_usw_cpu_reason_get_reason_info(uint8 lchip, uint8 dir, sys_cpu_reason_info_t* p_cpu_rason_info)
{
    uint16 excp_array[32] = {0};
    uint8 use_group = 0;
    uint8 fatal_excp_id = 0;
    uint8 normal_excp_id = 0;
    uint8  fatal_excp_valid = 0;
    uint8  normal_excp_valid = 0;
    uint8 excp_num = 0;
    uint8 exception_index = 0;
    uint8 exception_subIndex = 0;
    uint8 idx = 0;
    uint32 excp_id = 0;
    uint32 cmd = 0;
    uint8 dir_is_match = 0;
    uint8 excp_type = 0;

    CTC_PTR_VALID_CHECK(p_cpu_rason_info);

    if (p_cpu_rason_info->reason_id >= CTC_PKT_CPU_REASON_MAX_COUNT)
    {
        return CTC_E_BADID;
    }

    CTC_ERROR_RETURN(sys_usw_queue_get_excp_idx_by_reason(lchip,  p_cpu_rason_info->reason_id,
                                                      excp_array,
                                                      &excp_num));

    for (idx = 0; idx < excp_num; idx++)
    {
        excp_type = SYS_REASON_GET_EXCP_TYPE(excp_array[idx]);
        excp_id = SYS_REASON_GET_EXCP_INDEX(excp_array[idx]);

        if (excp_type !=SYS_REASON_NORMAL_EXCP && excp_type !=SYS_REASON_FATAL_EXCP)
        {
            continue;
        }

        if ((CTC_INGRESS == dir) && (excp_type == SYS_REASON_FATAL_EXCP))
        {
            fatal_excp_id = excp_id;
            dir_is_match = 1;
            fatal_excp_valid = 1;
        }
        else if ((CTC_INGRESS == dir) && (excp_id < SYS_NORMAL_EXCP_TYPE_EPE_OTHER))
        {
            dir_is_match = 1;
            use_group = 1;
            normal_excp_id = excp_id;
             normal_excp_valid = 1;
        }
        else if ((CTC_EGRESS == dir) && (excp_id >= SYS_NORMAL_EXCP_TYPE_EPE_OTHER)
            && (excp_id < SYS_NORMAL_EXCP_TYPE_EPE_L2SPAN_BASE))
        {
            dir_is_match = 1;
            use_group = 1;
            normal_excp_id = excp_id;
            normal_excp_valid = 1;
        }

        if (normal_excp_valid && fatal_excp_valid)
        {
            break;
        }
    }


    if (fatal_excp_valid)
    {
        p_cpu_rason_info->fatal_excp_valid = 1;
        p_cpu_rason_info->fatal_excp_index = fatal_excp_id;
    }

    /*CTC_PKT_CPU_REASON_VXLAN_NVGRE_CHK_FAIL/CTC_PKT_CPU_REASON_LABEL_MISS as acl key,
    only fatal exception can do as key*/

    if(normal_excp_valid)
    {
       sys_usw_cpu_reason_get_excp_index_by_excp_id(lchip, normal_excp_id,&exception_index, &exception_subIndex);
       p_cpu_rason_info->exception_index = exception_index;
       p_cpu_rason_info->exception_subIndex = exception_subIndex;

       if (CTC_INGRESS == dir)
       {
           uint32 value  = 0;
           cmd = DRV_IOR(IpeFwdExcepGroupMap_t, IpeFwdExcepGroupMap_array_0_exceptionGid_f + (normal_excp_id & 0x3));
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, (normal_excp_id >> 2), cmd, &value));

           p_cpu_rason_info->gid_for_acl_key = value & 0xFFFF;
           /*must be asic researved exception index & sub index, acl remap exception can't do as acl key*/
           p_cpu_rason_info->gid_valid = (p_cpu_rason_info->reason_id < CTC_PKT_CPU_REASON_CUSTOM_BASE)
                           && dir_is_match && use_group;
       }
       else
       {
           uint32 value  = 0;
           excp_id = normal_excp_id - 192;
           excp_id = excp_id & 0xF;
           cmd = DRV_IOR(EpeAclQosCtl_t, EpeAclQosCtl_gExcepGidMap_0_exceptionGid_f + excp_id);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip,0, cmd, &value));
           p_cpu_rason_info->gid_for_acl_key= value & 0xF;
           /*must be asic researved exception index & sub index, acl remap exception can't do as acl key*/
           p_cpu_rason_info->gid_valid = (p_cpu_rason_info->reason_id < CTC_PKT_CPU_REASON_CUSTOM_BASE)
                                                               && dir_is_match && use_group;
       }
    }


    if (fatal_excp_valid)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"reason_id:%d,excp_id:%d  fatal_excp_index = %d\n",
                                                            p_cpu_rason_info->reason_id,excp_id, p_cpu_rason_info->fatal_excp_index);
    }
    if (normal_excp_valid)
    {

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"reason_id:%d,excp_id:%d exception index:%d,subIndex:%d,gid:%d gid_valid:%d\n",
                                                            p_cpu_rason_info->reason_id,excp_id,exception_index,exception_subIndex,
                                                            p_cpu_rason_info->gid_for_acl_key,p_cpu_rason_info->gid_valid);
    }
    if(!p_cpu_rason_info->acl_key_valid  && (!normal_excp_valid || !dir_is_match ))
    {
        return CTC_E_BADID;
    }
    return CTC_E_NONE;
}

int32
_sys_usw_cpu_reason_alloc_exception_index(uint8 lchip, uint8 dir, sys_cpu_reason_info_t* p_cpu_rason_info)
{
    sys_usw_opf_t opf;
    uint8 exception_index = 0;
    uint8 exception_subIndex = 0;
    uint32 excp_id = 0;
    sys_cpu_reason_t *p_cpu_reason;
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 nexthop_ptr = 0;
    uint8 dsnh_8w = 0;

    if (p_cpu_rason_info->reason_id >= CTC_PKT_CPU_REASON_MAX_COUNT)
    {
        return CTC_E_BADID;
    }

    if (p_cpu_rason_info->reason_id < CTC_PKT_CPU_REASON_CUSTOM_BASE)
    {
        CTC_ERROR_RETURN(_sys_usw_cpu_reason_get_reason_info(lchip,  dir, p_cpu_rason_info));
        return CTC_E_NONE;
    }

    p_cpu_reason = &p_usw_queue_master[lchip]->cpu_reason[p_cpu_rason_info->reason_id];
    if (p_cpu_reason->user_define_mode == 1)
    {
        return CTC_E_IN_USE;
    }

    if (p_cpu_reason->user_define_mode == 0)
    {
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type  = p_usw_queue_master[lchip]->opf_type_excp_index;
        opf.pool_index = dir;
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, 1, &excp_id));
        p_cpu_reason->excp_id = (excp_id & 0xFFFF);
        p_cpu_reason->ref_cnt = 0;
        p_cpu_reason->dir = dir;
    }

    ret = sys_usw_cpu_reason_get_excp_index_by_excp_id(lchip, p_cpu_reason->excp_id, &exception_index, &exception_subIndex);
    if (ret != CTC_E_NONE)
    {
        if(p_cpu_reason->user_define_mode == 0)
        {
            sys_usw_opf_free_offset(lchip, &opf, 1, excp_id);
            p_cpu_reason->excp_id = 0;
        }

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Alloc new exception_index can't get exception_index and exception_subIndex :reason_id:%d,excp_id:%d\n",
                                                        p_cpu_rason_info->reason_id, excp_id);

        return ret;
    }
    p_cpu_rason_info->exception_index = exception_index;
    p_cpu_rason_info->exception_subIndex = exception_subIndex;
    p_cpu_rason_info->fatal_excp_valid = 0;
    p_cpu_rason_info->gid_valid = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Alloc new exception_index:reason_id:%d,excp_id:%d exception_index = %d, exception_subIndex = %d\n",
                                                        p_cpu_rason_info->reason_id, excp_id,exception_index, exception_subIndex);

    p_cpu_reason->user_define_mode = 2;
    p_cpu_reason->ref_cnt += 1;

    CTC_ERROR_RETURN(sys_usw_cpu_reason_get_nexthop_info(lchip, p_cpu_rason_info->reason_id, &nexthop_ptr, &dsnh_8w));
    value = p_cpu_reason->dest_map;
    cmd = DRV_IOW(DsMetFifoExcp_t, DsMetFifoExcp_destMap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cpu_reason->excp_id, cmd, &value));

    value = dsnh_8w;
    cmd = DRV_IOW(DsMetFifoExcp_t, DsMetFifoExcp_nextHopExt_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cpu_reason->excp_id, cmd, &value));

    cmd = DRV_IOW(DsBufRetrvExcp_t, DsBufRetrvExcp_nextHopPtr_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cpu_reason->excp_id, cmd, &nexthop_ptr));

    value = 1;
    cmd = DRV_IOW(DsBufRetrvExcp_t, DsBufRetrvExcp_forceEgressEdit_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_cpu_reason->excp_id, cmd, &value));
    return CTC_E_NONE;
}

int32
_sys_usw_cpu_reason_free_exception_index(uint8 lchip, uint8 dir, sys_cpu_reason_info_t* p_cpu_rason_info)
{
    sys_usw_opf_t opf;
    sys_cpu_reason_t *p_cpu_reason;

    if (p_cpu_rason_info->reason_id < CTC_PKT_CPU_REASON_CUSTOM_BASE)
    {
        return CTC_E_NONE;
    }

    if (p_cpu_rason_info->reason_id >= CTC_PKT_CPU_REASON_MAX_COUNT)
    {
        return CTC_E_INVALID_PARAM;
    }

    p_cpu_reason = &p_usw_queue_master[lchip]->cpu_reason[p_cpu_rason_info->reason_id];
    if (p_cpu_reason->user_define_mode != 2)
    {
        return CTC_E_NONE;
    }

    p_cpu_reason->ref_cnt = p_cpu_reason->ref_cnt ? (p_cpu_reason->ref_cnt-1) : 0;
    if (p_cpu_reason->excp_id != 0 && (p_cpu_reason->ref_cnt == 0))
    {
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type  = p_usw_queue_master[lchip]->opf_type_excp_index;
        opf.pool_index = dir;
        sys_usw_opf_free_offset(lchip, &opf, 1, p_cpu_reason->excp_id);

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Free old exception_index: reason_id = %d, excp_id :%d\n",
                              p_cpu_rason_info->reason_id, p_cpu_reason->excp_id);

        p_cpu_reason->excp_id = 0;
        p_cpu_reason->user_define_mode = 0;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_cpu_reason_alloc_dsfwd_offset(uint8 lchip, uint16 reason_id,
                                           uint32 *dsfwd_offset,
                                           uint32 *dsnh_offset,
                                           uint32 *dest_port)
{
    sys_cpu_reason_t* p_reason = NULL;
    uint8 critical_packet = 0;
    uint32 nexthop_ptr = 0;
    uint8 dsnh_8w = 0;
    uint8 del = (dsfwd_offset == NULL)?1:0;

    CTC_MIN_VALUE_CHECK(reason_id, CTC_PKT_CPU_REASON_CUSTOM_BASE);
    CTC_MAX_VALUE_CHECK(reason_id, CTC_PKT_CPU_REASON_MAX_COUNT - 1);

    p_reason = &p_usw_queue_master[lchip]->cpu_reason[reason_id];
    if (p_reason->user_define_mode == 2)
    {
        return CTC_E_IN_USE;
    }
    if ((CTC_PKT_CPU_REASON_TO_LOCAL_CPU == p_reason->dest_type)|| (CTC_PKT_CPU_REASON_TO_REMOTE_CPU == p_reason->dest_type)
         || (CTC_PKT_CPU_REASON_TO_LOCAL_CPU_ETH == p_reason->dest_type))
    {
        critical_packet = 1;
    }

    CTC_ERROR_RETURN(sys_usw_cpu_reason_get_nexthop_info(lchip, reason_id, &nexthop_ptr, &dsnh_8w));
    CTC_ERROR_RETURN(sys_usw_nh_update_entry_dsfwd(lchip, &p_reason->dsfwd_offset, p_reason->dest_map, nexthop_ptr, dsnh_8w, del, critical_packet));
    if(del)
    {
        p_reason->dsfwd_offset = 0;
        p_reason->user_define_mode = 0;
    }
    else
    {
        *dsfwd_offset = p_reason->dsfwd_offset;
        *dsnh_offset = CTC_PKT_CPU_REASON_BUILD_NHPTR(reason_id, 0);
        if (dest_port)
        {
            *dest_port = p_reason->dest_port;
        }
        p_reason->user_define_mode = 1;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_cpu_reason_set_discard_cancel_to_cpu(uint8 lchip, uint16 reason_id, uint32 cancel_en)
{
    uint32 cmd         = 0;
    uint8 idx          = 0;
    uint16 exception   = 0;
    uint16 excp_idx    = 0;
    uint8 excp_num     = 0;
    uint8 excp_type    = 0;
    uint16 excp_array[32] = {0};
    IpeFwdCtl_m fwd_ctl;


    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&fwd_ctl, 0, sizeof(IpeFwdCtl_m));
    CTC_ERROR_RETURN(sys_usw_queue_get_excp_idx_by_reason(lchip, reason_id,
                                                                excp_array,
                                                                &excp_num));
    for (idx = 0; idx < excp_num; idx++)
    {
        exception = excp_array[idx];
        excp_type = SYS_REASON_GET_EXCP_TYPE(exception);
        excp_idx = SYS_REASON_GET_EXCP_INDEX(exception);

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "excp_idx:%d\n", excp_idx);

        /*only support normal exception IPE*/
        if ((SYS_REASON_NORMAL_EXCP == excp_type) && (excp_idx < SYS_NORMAL_EXCP_TYPE_EPE_OTHER)) /*192*/
        {
            cmd = DRV_IOR(IpeFwdCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fwd_ctl));
            /**D2:gDiscardPktClearExcep_0_exceptionDisable_f;*
             **TM:gClearExcep_0_exceptionDisable_f,For adaption of Driver: same to D2*/
            SetIpeFwdCtl(V, gDiscardPktClearExcep_0_exceptionDisable_f + excp_idx, &fwd_ctl, cancel_en ? 1 : 0);
            cmd = DRV_IOW(IpeFwdCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fwd_ctl));
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_cpu_reason_set_drop(uint8 lchip, uint16 reason_id,uint8 drop_en)
{
    uint32 cmd         = 0;
    uint8 idx          = 0;
    uint16 exception   = 0;
    uint16 excp_idx    = 0;
    uint8 excp_num     = 0;
    uint8 excp_type    = 0;
    uint16 excp_array[32] = {0};
    IpeFwdExceptionCtl_m  igs_ctl;
    EpeHeaderEditExceptionCtl_m egs_ctl;
    IpeFwdCtl_m ipe_fwd_ctl;
    uint32 ipe_excep_bitmap = 0;
    uint32 epe_excep_bitmap = 0;
    uint32 fatal_excp_bitmap = 0;

    sal_memset(&igs_ctl, 0, sizeof(IpeFwdExceptionCtl_m));
    sal_memset(&egs_ctl, 0, sizeof(EpeHeaderEditExceptionCtl_m));
    sal_memset(&ipe_fwd_ctl, 0, sizeof(IpeFwdCtl_m));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_queue_get_excp_idx_by_reason(lchip, reason_id,
                                                                excp_array,
                                                                &excp_num));
    if (!excp_num)
    {
        return CTC_E_NONE;
    }
    cmd = DRV_IOR(BufferStoreCtl_t, BufferStoreCtl_exceptionVectorEgressBitmap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_excep_bitmap));

    cmd = DRV_IOR(BufferStoreCtl_t, BufferStoreCtl_exceptionVectorIngressBitmap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_excep_bitmap));

    if (DRV_IS_DUET2(lchip))
    {
        cmd = DRV_IOR(IpeFwdCtl_t, IpeFwdCtl_discardFatal_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fatal_excp_bitmap));
    }
    else
    {
        cmd = DRV_IOR(IpeAclQosCtl_t, IpeAclQosCtl_discardFatal_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fatal_excp_bitmap));
    }

    for (idx = 0; idx < excp_num; idx++)
    {
        exception = excp_array[idx];
        excp_type = SYS_REASON_GET_EXCP_TYPE(exception);
        excp_idx = SYS_REASON_GET_EXCP_INDEX(exception);
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "excp_idx:%d\n", excp_idx);

        /*cpu-rx mirror do not processing*/
        if ((SYS_REASON_OAM_EXCP == excp_type) || (SYS_REASON_FWD_EXCP == excp_type) || (SYS_REASON_MISC_NH_EXCP == excp_type))
        {
            continue;
        }

        /*fatal exception discard*/
        if (SYS_REASON_FATAL_EXCP == excp_type)
        {
           if (drop_en)
            {
                CTC_BIT_SET(fatal_excp_bitmap, excp_idx);
            }
            else
            {
                CTC_BIT_UNSET(fatal_excp_bitmap, excp_idx);
            }
        }
        /*normal exception IPE*/
        if ((SYS_REASON_NORMAL_EXCP == excp_type) && (excp_idx < SYS_NORMAL_EXCP_TYPE_EPE_OTHER)) /*192*/
        {
            cmd = DRV_IOR(IpeFwdExceptionCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &igs_ctl));
            SetIpeFwdExceptionCtl(V,g_0_clearException_f + excp_idx*2, &igs_ctl, (drop_en?1:0));
            cmd = DRV_IOW(IpeFwdExceptionCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &igs_ctl));

            continue;
        }

        /*normal exception EPE*/
        if ((SYS_REASON_NORMAL_EXCP == excp_type) && (excp_idx >= SYS_NORMAL_EXCP_TYPE_EPE_OTHER) && (excp_idx < SYS_NORMAL_EXCP_TYPE_EPE_L2SPAN_BASE))
        {
            cmd = DRV_IOR(EpeHeaderEditExceptionCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &egs_ctl));
            SetEpeHeaderEditExceptionCtl(V,a1_0_clearException_f + excp_idx - SYS_NORMAL_EXCP_TYPE_EPE_OTHER,
                                                &egs_ctl, (drop_en?1:0));

            cmd = DRV_IOW(EpeHeaderEditExceptionCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &egs_ctl));

            continue;
        }

        /*exceptionVetcor high bits*/
        if ((SYS_REASON_NORMAL_EXCP == excp_type) && (excp_idx >= SYS_NORMAL_EXCP_TYPE_EPE_L2SPAN_BASE)) /*224*/
        {
            switch (excp_idx)
            {
            case SYS_NORMAL_EXCP_TYPE_EPE_PORT_LOG: /*244*/
                if (drop_en)
                {
                    CTC_BIT_UNSET(epe_excep_bitmap, 6);
                }
                else
                {
                    CTC_BIT_SET(epe_excep_bitmap, 6);
                }
                break;

            case SYS_NORMAL_EXCP_TYPE_EPE_IPFIX_CONFLICT: /*246*/
                if (drop_en)
                {
                    CTC_BIT_UNSET(epe_excep_bitmap, 8);
                }
                else
                {
                    CTC_BIT_SET(epe_excep_bitmap, 8);
                }
                break;

            case SYS_NORMAL_EXCP_TYPE_BUFFER_LOG: /*248*/
                if (drop_en)
                {
                    CTC_BIT_UNSET(epe_excep_bitmap, 10);
                }
                else
                {
                    CTC_BIT_SET(epe_excep_bitmap, 10);
                }
                break;

            case SYS_NORMAL_EXCP_TYPE_LATENCY_LOG: /*249*/
                if (drop_en)
                {
                    CTC_BIT_UNSET(epe_excep_bitmap, 11);
                }
                else
                {
                    CTC_BIT_SET(epe_excep_bitmap, 11);
                }
                break;

            case SYS_NORMAL_EXCP_TYPE_IPE_PORT_LOG: /*296*/
                if (drop_en)
                {
                    CTC_BIT_UNSET(ipe_excep_bitmap, 11);
                }
                else
                {
                    CTC_BIT_SET(ipe_excep_bitmap, 11);
                }
                break;

            case SYS_NORMAL_EXCP_TYPE_IPE_IPFIX_CONFLICT: /*298*/
                if (drop_en)
                {
                    CTC_BIT_UNSET(ipe_excep_bitmap, 13);
                }
                else
                {
                    CTC_BIT_SET(ipe_excep_bitmap, 13);
                }
                break;

            case SYS_NORMAL_EXCP_TYPE_IPE_DEBUG_SESSION: /*300*/
                if (drop_en)
                {
                    CTC_BIT_UNSET(ipe_excep_bitmap, 15);
                }
                else
                {
                    CTC_BIT_SET(ipe_excep_bitmap, 15);
                }
                break;

            case SYS_NORMAL_EXCP_TYPE_IPE_ELEPHANT_FLOW_LOG: /*303*/
                if (drop_en)
                {
                    CTC_BIT_UNSET(ipe_excep_bitmap, 18);
                }
                else
                {
                    CTC_BIT_SET(ipe_excep_bitmap, 18);
                }
                break;

            default:
                break;
            }
        }
    }

    cmd = DRV_IOW(BufferStoreCtl_t, BufferStoreCtl_exceptionVectorEgressBitmap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_excep_bitmap));
    cmd = DRV_IOW(BufferStoreCtl_t, BufferStoreCtl_exceptionVectorIngressBitmap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_excep_bitmap));
    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_discardFatal_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fatal_excp_bitmap));
    cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_discardFatal_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fatal_excp_bitmap));

    return CTC_E_NONE;
}

int32
sys_usw_cpu_reason_set_map(uint8 lchip, uint16 reason_id,
                                  uint8 sub_queue_id,
                                  uint8 group_id)
{
    uint8  dest_type = 0;
    uint32 dest_port = 0;
    uint16 cpu_queue_id = 0;

    CTC_MAX_VALUE_CHECK(group_id, (SYS_USW_MAX_CPU_REASON_GROUP_NUM - 1));
    CTC_MAX_VALUE_CHECK(sub_queue_id, (SYS_USW_MAX_QNUM_PER_GROUP - 1));
    CTC_MAX_VALUE_CHECK(reason_id, CTC_PKT_CPU_REASON_MAX_COUNT - 1);

    group_id &= (p_usw_queue_master[lchip]->queue_num_for_cpu_reason/SYS_USW_MAX_QNUM_PER_GROUP - 1);
    dest_type = p_usw_queue_master[lchip]->cpu_reason[reason_id].dest_type;
    dest_port = p_usw_queue_master[lchip]->cpu_reason[reason_id].dest_port;
    sub_queue_id = group_id * SYS_USW_MAX_QNUM_PER_GROUP + sub_queue_id;

    if (dest_type != CTC_PKT_CPU_REASON_TO_LOCAL_CPU_ETH
        && dest_type != CTC_PKT_CPU_REASON_TO_LOCAL_CPU
        && dest_type != CTC_PKT_CPU_REASON_TO_REMOTE_CPU)
    {
        return CTC_E_NONE;
    }

    if (dest_type == CTC_PKT_CPU_REASON_TO_LOCAL_CPU_ETH)
    {
        if (!p_usw_queue_master[lchip]->have_lcpu_by_eth ||
            (sub_queue_id > p_usw_queue_master[lchip]->queue_num_per_chanel))
        {
            return CTC_E_INVALID_PARAM;
        }
    }


    if (CTC_PKT_CPU_REASON_C2C_PKT == reason_id)
    {
        uint32 cmd = 0;
        uint8 gchip_id = 0;
        uint32 field_val = 0;
        IpeFwdCtl_m ipe_fwd_ctl;
        sal_memset(&ipe_fwd_ctl, 0, sizeof(IpeFwdCtl_m));

        /*c2c pkt need 2 group*/
        CTC_MAX_VALUE_CHECK(group_id, (MCHIP_CAP(SYS_CAP_QOS_QUEUE_GRP_NUM_FOR_CPU_REASON) - 2));
        sub_queue_id = group_id * MCHIP_CAP(SYS_CAP_REASON_GRP_QUEUE_NUM);

        /*2 reason group sp schedule*/
        cpu_queue_id = MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP) + group_id * MCHIP_CAP(SYS_CAP_REASON_GRP_QUEUE_NUM);
        CTC_ERROR_RETURN(sys_usw_queue_sch_set_c2c_group_sched(lchip, cpu_queue_id, 2));
        cpu_queue_id = MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP) + (group_id + 1) * MCHIP_CAP(SYS_CAP_REASON_GRP_QUEUE_NUM);
        CTC_ERROR_RETURN(sys_usw_queue_sch_set_c2c_group_sched(lchip, cpu_queue_id, 3));

        /*to cpu based on prio for point to point stacking*/
        sys_usw_get_gchip_id(lchip, &gchip_id);
        field_val =  SYS_ENCODE_EXCP_DESTMAP_GRP(gchip_id, (group_id+1));
        cmd = DRV_IOR(IpeFwdCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl));
        SetIpeFwdCtl(V, neighborDiscoveryDestMap_f, &ipe_fwd_ctl, field_val);
        cmd = DRV_IOW(IpeFwdCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl));
    }

    p_usw_queue_master[lchip]->cpu_reason[reason_id].sub_queue_id = sub_queue_id;

    if ((CTC_PKT_CPU_REASON_MIRRORED_TO_CPU == reason_id) || (CTC_PKT_CPU_REASON_C2C_PKT == reason_id))
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_usw_cpu_reason_set_dest(lchip,  reason_id, dest_type, dest_port));

    return CTC_E_NONE;
}

int32
_sys_usw_get_sub_queue_id_by_cpu_reason(uint8 lchip, uint16 reason_id, uint8* sub_queue_id)
{
    CTC_MAX_VALUE_CHECK(reason_id, CTC_PKT_CPU_REASON_MAX_COUNT - 1);
    CTC_PTR_VALID_CHECK(sub_queue_id);

    *sub_queue_id = p_usw_queue_master[lchip]->cpu_reason[reason_id].sub_queue_id;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_cpu_reason_update_resrc_pool(uint8 lchip, uint16 reason_id,
                                             uint8 dest_type)
{
    uint8 pool = 0;

    if ((dest_type == CTC_PKT_CPU_REASON_TO_LOCAL_CPU ||
        dest_type == CTC_PKT_CPU_REASON_TO_LOCAL_CPU_ETH ||
        dest_type == CTC_PKT_CPU_REASON_TO_REMOTE_CPU)
        && (CTC_PKT_CPU_REASON_NEW_ELEPHANT_FLOW != reason_id))
    {
        pool = CTC_QOS_EGS_RESRC_CONTROL_POOL;
    }
    else
    {
        pool = CTC_QOS_EGS_RESRC_DEFAULT_POOL;
    }

    if (dest_type != CTC_PKT_CPU_REASON_TO_REMOTE_CPU)
    {
        CTC_ERROR_RETURN(sys_usw_queue_set_cpu_queue_egs_pool_classify(lchip, reason_id, pool));
    }

    return CTC_E_NONE;
}

int32
sys_usw_cpu_reason_set_dest(uint8 lchip,
                                  uint16 reason_id,
                                  uint8 dest_type,
                                  uint32 dest_port)
{
    uint32 dest_map = 0;
    uint8  gchip = 0;
    uint16 lport = 0;
    uint32 nexthop_ptr = 0;
    uint16 sub_queue_id = 0;
    uint8  old_dest_type = 0;
    uint8  dsnh_8w = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO,  "reason_id = %d, dest_type = %d, dest_port = 0x%x\n",
                       reason_id, dest_type, dest_port);

    CTC_MAX_VALUE_CHECK(reason_id, CTC_PKT_CPU_REASON_MAX_COUNT - 1);
    CTC_MAX_VALUE_CHECK(dest_type, CTC_PKT_CPU_REASON_TO_MAX_COUNT-1);

    sys_usw_get_gchip_id(lchip, &gchip);
    old_dest_type = p_usw_queue_master[lchip]->cpu_reason[reason_id].dest_type;
    sub_queue_id = p_usw_queue_master[lchip]->cpu_reason[reason_id].sub_queue_id;

    switch (dest_type)
    {
    case CTC_PKT_CPU_REASON_TO_LOCAL_CPU:
        if (old_dest_type != CTC_PKT_CPU_REASON_TO_LOCAL_CPU
           && sub_queue_id == 0xFF)
        {
            sub_queue_id  = 0;   /*use default sub queue id*/
        }
        dest_map = SYS_ENCODE_EXCP_DESTMAP(gchip,sub_queue_id);
        nexthop_ptr = CTC_PKT_CPU_REASON_BUILD_NHPTR(reason_id, 0);
        dest_port = CTC_GPORT_RCPU(gchip);
        break;

    case CTC_PKT_CPU_REASON_TO_LOCAL_CPU_ETH:
        {
            if (!p_usw_queue_master[lchip]->have_lcpu_by_eth)
            {
                return CTC_E_INVALID_PARAM;
            }
            lport = sys_usw_dmps_get_lport_with_chan(lchip, p_usw_queue_master[lchip]->cpu_eth_chan_id);
            dest_map = SYS_ENCODE_DESTMAP(gchip, lport);
            nexthop_ptr = CTC_PKT_CPU_REASON_BUILD_NHPTR(reason_id, 0);
            dest_port = CTC_MAP_LPORT_TO_GPORT(gchip, lport);
            sub_queue_id = 0xFF;
        }
        break;

    case CTC_PKT_CPU_REASON_TO_LOCAL_PORT:
        {
            uint32 nhid = 0;
            sys_nh_info_dsnh_t nh_info;
            sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));
            gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(dest_port);
            lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(dest_port);
            dest_map = SYS_ENCODE_DESTMAP( gchip, lport);
            CTC_ERROR_RETURN(sys_usw_l2_get_ucast_nh(lchip, dest_port, CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS, &nhid));
            /*get nexthop info from nexthop id */
            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, nhid, &nh_info, 0));
            nexthop_ptr = nh_info.dsnh_offset;
            CTC_BIT_UNSET(nexthop_ptr, 17); /* bit 17 used for bypass ingress edit */
            dsnh_8w = nh_info.nexthop_ext;
            sub_queue_id = 0xFF;
        }
        break;

    case CTC_PKT_CPU_REASON_TO_NHID:
        {
            sys_nh_info_dsnh_t nhinfo;
            sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, dest_port, &nhinfo, 0));
            if (nhinfo.aps_en ||
                (nhinfo.nh_entry_type != SYS_NH_TYPE_BRGUC
                && nhinfo.nh_entry_type != SYS_NH_TYPE_IPUC
                && nhinfo.nh_entry_type != SYS_NH_TYPE_ILOOP
                && nhinfo.nh_entry_type != SYS_NH_TYPE_ELOOP))
            {
                return CTC_E_INVALID_PARAM;
            }

            dest_map = nhinfo.dest_map;
            nexthop_ptr = nhinfo.dsnh_offset;
            CTC_BIT_UNSET(nexthop_ptr, 17); /* bit 17 used for bypass ingress edit */
            dsnh_8w = nhinfo.nexthop_ext;
            sub_queue_id = 0xFF;
        }
        break;

    case CTC_PKT_CPU_REASON_TO_REMOTE_CPU:
        if (old_dest_type != CTC_PKT_CPU_REASON_TO_REMOTE_CPU
           && sub_queue_id == 0xFF)
        {
            sub_queue_id  = 0;   /*use default sub queue id*/
        }
        gchip = CTC_MAP_GPORT_TO_GCHIP(dest_port);
        dest_map = SYS_ENCODE_EXCP_DESTMAP(gchip,sub_queue_id);
        nexthop_ptr = CTC_PKT_CPU_REASON_BUILD_NHPTR(reason_id, 0);
        break;

    case CTC_PKT_CPU_REASON_TO_DROP:
        sub_queue_id = 0xFF;
        CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
        dest_port = CTC_MAP_LPORT_TO_GPORT(gchip, SYS_RSV_PORT_DROP_ID);
        dest_map = SYS_ENCODE_DESTMAP(gchip, SYS_RSV_PORT_DROP_ID);
        nexthop_ptr = CTC_PKT_CPU_REASON_BUILD_NHPTR(reason_id, 0);
        break;

    case CTC_PKT_CPU_REASON_DISCARD_CANCEL_TO_CPU:
        CTC_ERROR_RETURN(_sys_usw_cpu_reason_set_discard_cancel_to_cpu(lchip, reason_id, dest_port));
        return CTC_E_NONE;

    default:
        break;
    }

    /*For CPU Rx drop mirror*/
    if (dest_type == CTC_PKT_CPU_REASON_TO_DROP)
    {
        _sys_usw_cpu_reason_set_drop(lchip, reason_id, TRUE);
    }
    else
    {
        _sys_usw_cpu_reason_set_drop(lchip, reason_id, FALSE);
    }

    CTC_ERROR_RETURN(sys_usw_cpu_reason_update_exception(lchip, reason_id,
                                                                dest_map,
                                                                nexthop_ptr,
                                                                dsnh_8w,
                                                                dest_type));
    p_usw_queue_master[lchip]->cpu_reason[reason_id].dest_type = dest_type;
    p_usw_queue_master[lchip]->cpu_reason[reason_id].sub_queue_id = sub_queue_id;
    p_usw_queue_master[lchip]->cpu_reason[reason_id].dest_port = dest_port;
    p_usw_queue_master[lchip]->cpu_reason[reason_id].dest_map = dest_map;
    if (CTC_PKT_CPU_REASON_C2C_PKT == reason_id)
    {
        sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_C2C_SUB_QUEUE_ID, sub_queue_id, 0);
    }
    else if (CTC_PKT_CPU_REASON_FWD_CPU == reason_id)
    {
        sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_FWD_CPU_SUB_QUEUE_ID, sub_queue_id, 0);
    }

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO,  "gchip = %d, dest_map = 0x%x\n",  gchip, dest_map);

    CTC_ERROR_RETURN(_sys_usw_cpu_reason_update_resrc_pool(lchip, reason_id, dest_type));

    return CTC_E_NONE;
}

int32
_sys_usw_cpu_reason_register_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint16 val_ipetunnel = 0;
    uint16 val_iperoute  = 0;
    IpeUserIdCtl_m  user_id_ctl;
    IpeTunnelIdCtl_m  tunnel_id_ctl;
    IpeTunnelDecapCtl_m tunnel_decap_ctl;
    IpeException3Ctl_m ipe_excp_ctl;
    IpeRouteCtl_m ipe_route_ctl;
    IpeLookupCtl_m ipe_lookup_ctl;
    IpeBridgeCtl_m ipe_bridge_ctl;
    IpeLearningCtl_m ipe_learn_ctl;
    IpeHeaderAdjustCtl_m ipe_hdr_adjust_ctl;
    IpeTunnelDecapFatalExcepCtl_m ipe_tunnel_decap_ftl_ctl;
    IpeRouteFatalExcepCtl_m ipe_router_ftl_ctl;
    IpeMplsFatalExcepCtl_m  ipe_mpls_ftl_ctl;
    IpePreLookupProtocolCtl_m ipe_pre_lookup_proto_ctl;

    sal_memset(&user_id_ctl,0,sizeof(user_id_ctl));
    sal_memset(&tunnel_id_ctl,0,sizeof(tunnel_id_ctl));
    sal_memset(&tunnel_decap_ctl,0,sizeof(tunnel_decap_ctl));
    sal_memset(&ipe_excp_ctl,0,sizeof(ipe_excp_ctl));
    sal_memset(&ipe_route_ctl,0,sizeof(IpeRouteCtl_m));
    sal_memset(&ipe_lookup_ctl,0,sizeof(IpeLookupCtl_m));
    sal_memset(&ipe_bridge_ctl,0,sizeof(IpeBridgeCtl_m));
    sal_memset(&ipe_learn_ctl,0,sizeof(IpeLearningCtl_m));
    sal_memset(&ipe_hdr_adjust_ctl,0,sizeof(IpeHeaderAdjustCtl_m));
    sal_memset(&ipe_tunnel_decap_ftl_ctl,0,sizeof(IpeTunnelDecapFatalExcepCtl_m));
    sal_memset(&ipe_router_ftl_ctl,0,sizeof(IpeRouteFatalExcepCtl_m));
    sal_memset(&ipe_mpls_ftl_ctl,0,sizeof(IpeMplsFatalExcepCtl_m));
    sal_memset(&ipe_pre_lookup_proto_ctl,0,sizeof(IpePreLookupProtocolCtl_m));

    cmd = DRV_IOR(IpeUserIdCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&user_id_ctl));
    cmd = DRV_IOR(IpeTunnelIdCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&tunnel_id_ctl));
    cmd = DRV_IOR(IpeTunnelDecapCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&tunnel_decap_ctl));
    cmd = DRV_IOR(IpeException3Ctl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_excp_ctl));
    cmd = DRV_IOR(IpeRouteCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_route_ctl));
    cmd = DRV_IOR(IpeLookupCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_lookup_ctl));
    cmd = DRV_IOR(IpeBridgeCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_bridge_ctl));
    cmd = DRV_IOR(IpeLearningCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_learn_ctl));
    cmd = DRV_IOR(IpeHeaderAdjustCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_hdr_adjust_ctl));
    cmd = DRV_IOR(IpeTunnelDecapFatalExcepCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_tunnel_decap_ftl_ctl));
    cmd = DRV_IOR(IpeRouteFatalExcepCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_router_ftl_ctl));
    cmd = DRV_IOR(IpeMplsFatalExcepCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_mpls_ftl_ctl));
    cmd = DRV_IOR(IpePreLookupProtocolCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_pre_lookup_proto_ctl));

    SetIpeUserIdCtl(V,staStatusCheckErrorException_f,&user_id_ctl,1);
    SetIpeUserIdCtl(V,staStatusCheckErrorExceptionSubIndex_f,&user_id_ctl,SYS_NORMAL_EXCP_SUB_TYPE_WLAN_CLIENT_ERR);
    SetIpeUserIdCtl(V,roamingForwardErrorException_f,&user_id_ctl,0);
    SetIpeUserIdCtl(V,unknownDot11PacketExceptionEn_f,&user_id_ctl,1);
    SetIpeUserIdCtl(V,unknownDot11PacketExceptionSubIndex_f,&user_id_ctl,SYS_NORMAL_EXCP_SUB_TYPE_UNKNOWN_DOT11_PKT);

    SetIpeUserIdCtl(V,mcastTunnelFragSubIndex_f,&user_id_ctl,SYS_NORMAL_EXCP_SUB_TYPE_MCAST_TUNNEL_FRAG);

    SetIpeUserIdCtl(V,unknownDot1AeSciException_f,&user_id_ctl,1);
    SetIpeUserIdCtl(V,unknownDot1AeSciExceptionSubIndex_f,&user_id_ctl,SYS_NORMAL_EXCP_SUB_TYPE_UNKNOWN_DOT1AE_SCI);

    SetIpeUserIdCtl(V,unknownDot1AePacketException_f,&user_id_ctl,1);
    SetIpeUserIdCtl(V,unknownDot1AePacketExceptionSubIndex_f,&user_id_ctl,SYS_NORMAL_EXCP_SUB_TYPE_UNKNOWN_DOT1AE_PKT);

    /*DRV_IS_TSINGMA:START*/
    SetIpeUserIdCtl(V, onuPassThroughExceptionIndex_f, &user_id_ctl, SYS_NORMAL_EXCP_PON_PT_PKT);

    SetIpeHeaderAdjustCtl(V, gemPortErrorDiscard_f, &ipe_hdr_adjust_ctl, 1);
    SetIpeHeaderAdjustCtl(V, gemPortErrorDiscardType_f, &ipe_hdr_adjust_ctl, 0x10);/*IPEDISCARDTYPE_VLAN_FILTER_DIS*/
    SetIpeHeaderAdjustCtl(V, gemPortErrorExceptionEn_f, &ipe_hdr_adjust_ctl, 1);
    SetIpeHeaderAdjustCtl(V, gemPortErrorExceptionSubIndex_f, &ipe_hdr_adjust_ctl, SYS_NORMAL_EXCP_UNKNOWN_GEM_PORT);
    /*DRV_IS_TSINGMA:END*/

    SetIpeTunnelIdCtl(V,capwapFragException_f,&tunnel_id_ctl,1);
    SetIpeTunnelIdCtl(V,capwapFragExceptionSubIndex_f,&tunnel_id_ctl,SYS_NORMAL_EXCP_SUB_TYPE_WLAN_FRAG_EXCP);
    SetIpeTunnelIdCtl(V,capwapTunnelErrorException_f,&tunnel_id_ctl,1);
    SetIpeTunnelIdCtl(V,capwapTunnelErrExceptionSubIndex_f,&tunnel_id_ctl,SYS_NORMAL_EXCP_SUB_TYPE_WLAN_TUNNEL_ERR);

    SetIpeTunnelDecapCtl(V,capwapCipherStatusMismatchException_f,&tunnel_decap_ctl,1);
    SetIpeTunnelDecapCtl(V,capwapCipherStatusMismatchExceptionSubIndex_f,&tunnel_decap_ctl,SYS_NORMAL_EXCP_SUB_TYPE_WLAN_CIPHER_MISMATCH);
    SetIpeTunnelDecapCtl(V,capwapDot11MgrCtlPktException_f,&tunnel_decap_ctl,1);
    SetIpeTunnelDecapCtl(V,capwapDot11MgrCtlPktExceptionSubIndex_f,&tunnel_decap_ctl,SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DOT11_MGR);
    SetIpeTunnelDecapCtl(V,capwapDtlsControlPacketException_f,&tunnel_decap_ctl,1);
    SetIpeTunnelDecapCtl(V,capwapDtlsControlPacketExceptionSubIndex_f,&tunnel_decap_ctl,SYS_NORMAL_EXCP_SUB_TYPE_WLAN_DTLS);
    SetIpeTunnelDecapCtl(V,capwapKeepAliveException_f,&tunnel_decap_ctl,1);
    SetIpeTunnelDecapCtl(V,capwapKeepAliveExceptionSubIndex_f,&tunnel_decap_ctl,SYS_NORMAL_EXCP_SUB_TYPE_WLAN_KEEP_ALIVE);
    SetIpeTunnelDecapCtl(V,capwapPaketErrorException_f,&tunnel_decap_ctl,1);
    SetIpeTunnelDecapCtl(V,capwapPacketErrorExceptionSubIndex_f,&tunnel_decap_ctl,SYS_NORMAL_EXCP_SUB_TYPE_WLAN_PKT_ERR);

    SetIpeException3Ctl(V,capwapControlPacketExceptionEn_f,&ipe_excp_ctl,1);
    SetIpeException3Ctl(V,capwapControlPacketExceptionSubIndex_f,&ipe_excp_ctl,SYS_NORMAL_EXCP_SUB_TYPE_WLAN_CTL);

    /*DRV_IS_TSINGMA:START*/
    SetIpeException3Ctl(V, igmpExceptionEn_f, &ipe_excp_ctl, 1);
    SetIpeException3Ctl(V, igmpQueryExceptionEn_f, &ipe_excp_ctl, 0);
    SetIpeException3Ctl(V, igmpQueryExceptionSubIndex_f, &ipe_excp_ctl, CTC_L3PDU_ACTION_INDEX_IGMP);
    SetIpeException3Ctl(V, igmpRepLevExceptionEn_f, &ipe_excp_ctl, 0);
    SetIpeException3Ctl(V, igmpRepLevExceptionSubIndex_f, &ipe_excp_ctl, CTC_L3PDU_ACTION_INDEX_IGMP);
    SetIpeException3Ctl(V, igmpUnknownExceptionEn_f, &ipe_excp_ctl, 0);
    SetIpeException3Ctl(V, igmpUnknownExceptionSubIndex_f, &ipe_excp_ctl, CTC_L3PDU_ACTION_INDEX_IGMP);

    SetIpeException3Ctl(V, mldExceptionEn_f, &ipe_excp_ctl, 1);
    SetIpeException3Ctl(V, mldQueryExceptionEn_f, &ipe_excp_ctl, 0);
    SetIpeException3Ctl(V, mldQueryExceptionSubIndex_f, &ipe_excp_ctl, CTC_L3PDU_ACTION_INDEX_MLD);
    SetIpeException3Ctl(V, mldRepDoneExceptionEn_f, &ipe_excp_ctl, 0);
    SetIpeException3Ctl(V, mldRepAndDoneExceptionSubIndex_f, &ipe_excp_ctl, CTC_L3PDU_ACTION_INDEX_MLD);
    SetIpeException3Ctl(V, mldUnknownExceptionEn_f, &ipe_excp_ctl, 0);
    SetIpeException3Ctl(V, mldUnknownExceptionSubIndex_f, &ipe_excp_ctl, CTC_L3PDU_ACTION_INDEX_MLD);

    SetIpeException3Ctl(V, isisExceptionEn14_f, &ipe_excp_ctl, 0);
    SetIpeException3Ctl(V, isisExceptionEn15_f, &ipe_excp_ctl, 0);
    SetIpeException3Ctl(V, isisExceptionSubIndex_f, &ipe_excp_ctl, CTC_L3PDU_ACTION_INDEX_ISIS);
    SetIpeException3Ctl(V, isisDiscardEn_f, &ipe_excp_ctl, 0);
    SetIpeException3Ctl(V, isisDiscardType_f, &ipe_excp_ctl, 0x1e);/*IPEDISCARDTYPE_L3_EXCEP_DIS*/

    SetIpePreLookupProtocolCtl(V, v4IgmpQueryFwdGlobalCtlEn_f, &ipe_pre_lookup_proto_ctl, 1);
    SetIpePreLookupProtocolCtl(V, v6MldQueryFwdGlobalCtlEn_f, &ipe_pre_lookup_proto_ctl, 1);
    SetIpePreLookupProtocolCtl(V, forceV4IgmpQueryFlooding_f, &ipe_pre_lookup_proto_ctl, 1);
    SetIpePreLookupProtocolCtl(V, forceV6MldQueryFlooding_f, &ipe_pre_lookup_proto_ctl, 1);
    /*DRV_IS_TSINGMA:END*/

    SetIpeRouteCtl(V, icmpErrMsgExceptionSubIndex_f, &ipe_route_ctl, SYS_NORMAL_EXCP_SUB_TYPE_ICMP_ERR_MSG);
    /*igmp use fatal exception to cpu by default*/
    SetIpeRouteCtl(V, igmpSnoopedMode_f, &ipe_route_ctl, 0);
    SetIpeRouteCtl(V, nonBrgPktIgmpSnoopedDiscardType_f, &ipe_route_ctl, 0x1e);/*IPEDISCARDTYPE_L3_EXCEP_DIS*/
    SetIpeRouteCtl(V, nonBrgPktIgmpSnoopedExceptionIndex_f ,&ipe_route_ctl, 1);
    SetIpeRouteCtl(V, nonBrgPktIgmpSnoopedExceptionSubIndex_f ,&ipe_route_ctl,SYS_NORMAL_EXCP_SUB_TYPE_IGMP_SNOOPED_PACKET);

    SetIpeBridgeCtl(V, igmpSnoopedDonotEscape_f, &ipe_bridge_ctl, 0);
    SetIpeBridgeCtl(V, igmpSnoopedDiscardControl_f, &ipe_bridge_ctl, 0x6e);

    SetIpeLookupCtl(V, tunnelEcnMisExceptionSubIndex_f, &ipe_lookup_ctl, SYS_NORMAL_EXCP_SUB_TYPE_TUNNEL_ECN_MIS);
    /*igmp use fatal exception to cpu by default*/
    SetIpeBridgeCtl(V, ipv6MrdSnoopingEn_f, &ipe_bridge_ctl, 1);
    SetIpeBridgeCtl(V, igmpSnoopEnCtlMode_f, &ipe_bridge_ctl, 1);
    SetIpeBridgeCtl(V, igmpSnoopedDiscardType_f, &ipe_bridge_ctl, 0x1e);/*IPEDISCARDTYPE_L3_EXCEP_DIS*/
    SetIpeBridgeCtl(V, igmpSnoopedExceptionIndex_f, &ipe_bridge_ctl, 1);
    SetIpeBridgeCtl(V, igmpSnoopedExceptionSubIndex_f, &ipe_bridge_ctl, SYS_NORMAL_EXCP_SUB_TYPE_IGMP_SNOOPED_PACKET);
    SetIpeBridgeCtl(V, pimSnoopedDiscardType_f, &ipe_bridge_ctl, 0x1e);/*IPEDISCARDTYPE_L3_EXCEP_DIS*/
    SetIpeBridgeCtl(V, pimSnoopedDonotEscape_f, &ipe_bridge_ctl, 1);
    SetIpeBridgeCtl(V, pimSnoopedExceptionIndex_f, &ipe_bridge_ctl, 1);
    SetIpeBridgeCtl(V, pimSnoopedExceptionSubIndex_f, &ipe_bridge_ctl, SYS_NORMAL_EXCP_SUB_TYPE_PIM_SNOOPED_PACKET);

    /*unknownMC/BC/UC exception to CPU*/
    SetIpeBridgeCtl(V, unknownUcExceptionSubIndexDelta_f, &ipe_bridge_ctl, 0);/*TM*/
    SetIpeBridgeCtl(V, bcExceptionSubIndexDelta_f, &ipe_bridge_ctl, 0);/*TM*/

    SetIpeLearningCtl(V, portAuthDiscardType_f, &ipe_learn_ctl, 0x15);/*IPEDISCARDTYPE_LEARNING_DIS*/
    SetIpeLearningCtl(V, portAuthFailedExceptionIndex_f, &ipe_learn_ctl, 1);
    SetIpeLearningCtl(V, portAuthFailedExceptionSubIndex_f, &ipe_learn_ctl, SYS_NORMAL_EXCP_SUB_TYPE_SRC_MISMATCH);


    /*TSINGMA fatal exception replace process:only replace which default action is to local cpu*/
    /*DRV_IS_TSINGMA:START*/
    /*SYS_FATAL_EXCP_UC_IP_OPTIONS*/
    CTC_BIT_SET(val_ipetunnel,2);/*set fatalExceptionMode_f bit*/
    SetIpeTunnelDecapFatalExcepCtl(V, gFatalExcepMode1_2_discard_f, &ipe_tunnel_decap_ftl_ctl, 1);
    SetIpeTunnelDecapFatalExcepCtl(V, gFatalExcepMode1_2_discardType_f, &ipe_tunnel_decap_ftl_ctl, 0x18);/*IPEDISCARDTYPE_FATAL_EXCEP_DIS*/
    SetIpeTunnelDecapFatalExcepCtl(V, gFatalExcepMode1_2_exceptionIndex_f, &ipe_tunnel_decap_ftl_ctl, 2);
    SetIpeTunnelDecapFatalExcepCtl(V, gFatalExcepMode1_2_exceptionSubIndex_f, &ipe_tunnel_decap_ftl_ctl, SYS_NORMAL_EXCP_UC_IP_OPTIONS);

    CTC_BIT_SET(val_iperoute,1);/*set fatalExceptionMode_f bit*/
    SetIpeRouteFatalExcepCtl(V, gRouteFatalExcepMode1_1_discard_f, &ipe_router_ftl_ctl, 1);
    SetIpeRouteFatalExcepCtl(V, gRouteFatalExcepMode1_1_discardType_f, &ipe_router_ftl_ctl, 0x18);/*IPEDISCARDTYPE_FATAL_EXCEP_DIS*/
    SetIpeRouteFatalExcepCtl(V, gRouteFatalExcepMode1_1_exceptionIndex_f, &ipe_router_ftl_ctl, 2);
    SetIpeRouteFatalExcepCtl(V, gRouteFatalExcepMode1_1_exceptionSubIndex_f, &ipe_router_ftl_ctl, SYS_NORMAL_EXCP_UC_IP_OPTIONS);

    /*SYS_FATAL_EXCP_UC_IP_TTL_CHECK_FAIL*/
    CTC_BIT_SET(val_ipetunnel,7);/*set fatalExceptionMode_f bit*/
    SetIpeTunnelDecapFatalExcepCtl(V, gFatalExcepMode1_7_discard_f, &ipe_tunnel_decap_ftl_ctl, 1);
    SetIpeTunnelDecapFatalExcepCtl(V, gFatalExcepMode1_7_discardType_f, &ipe_tunnel_decap_ftl_ctl, 0x18);/*IPEDISCARDTYPE_FATAL_EXCEP_DIS*/
    SetIpeTunnelDecapFatalExcepCtl(V, gFatalExcepMode1_7_exceptionIndex_f, &ipe_tunnel_decap_ftl_ctl, 2);
    SetIpeTunnelDecapFatalExcepCtl(V, gFatalExcepMode1_7_exceptionSubIndex_f, &ipe_tunnel_decap_ftl_ctl, SYS_NORMAL_EXCP_UC_IP_TTL_CHECK_FAIL);

    CTC_BIT_SET(val_iperoute,4);/*set fatalExceptionMode_f bit*/
    SetIpeRouteFatalExcepCtl(V, gRouteFatalExcepMode1_4_discard_f, &ipe_router_ftl_ctl, 1);
    SetIpeRouteFatalExcepCtl(V, gRouteFatalExcepMode1_4_discardType_f, &ipe_router_ftl_ctl, 0x18);/*IPEDISCARDTYPE_FATAL_EXCEP_DIS*/
    SetIpeRouteFatalExcepCtl(V, gRouteFatalExcepMode1_4_exceptionIndex_f, &ipe_router_ftl_ctl, 2);
    SetIpeRouteFatalExcepCtl(V, gRouteFatalExcepMode1_4_exceptionSubIndex_f, &ipe_router_ftl_ctl, SYS_NORMAL_EXCP_UC_IP_TTL_CHECK_FAIL);

    /*SYS_FATAL_EXCP_UC_LINK_ID_CHECK_FAIL*/
    CTC_BIT_SET(val_iperoute,7);/*set fatalExceptionMode_f bit*/
    SetIpeRouteFatalExcepCtl(V, gRouteFatalExcepMode1_7_discard_f, &ipe_router_ftl_ctl, 1);
    SetIpeRouteFatalExcepCtl(V, gRouteFatalExcepMode1_7_discardType_f, &ipe_router_ftl_ctl, 0x18);/*IPEDISCARDTYPE_FATAL_EXCEP_DIS*/
    SetIpeRouteFatalExcepCtl(V, gRouteFatalExcepMode1_7_exceptionIndex_f, &ipe_router_ftl_ctl, 2);
    SetIpeRouteFatalExcepCtl(V, gRouteFatalExcepMode1_7_exceptionSubIndex_f, &ipe_router_ftl_ctl, SYS_NORMAL_EXCP_UC_LINK_ID_CHECK_FAIL);

    /*SYS_FATAL_EXCP_MPLS_TTL_CHECK_FAIL*/
    SetIpeMplsFatalExcepCtl(V, gMplsFatalExcepMode1_2_discard_f, &ipe_mpls_ftl_ctl, 1);
    SetIpeMplsFatalExcepCtl(V, gMplsFatalExcepMode1_2_discardType_f, &ipe_mpls_ftl_ctl, 0x18);/*IPEDISCARDTYPE_FATAL_EXCEP_DIS*/
    SetIpeMplsFatalExcepCtl(V, gMplsFatalExcepMode1_2_exceptionIndex_f, &ipe_mpls_ftl_ctl, 2);
    SetIpeMplsFatalExcepCtl(V, gMplsFatalExcepMode1_2_exceptionSubIndex_f, &ipe_mpls_ftl_ctl, SYS_NORMAL_EXCP_MPLS_TTL_CHECK_FAIL);
    SetIpeMplsFatalExcepCtl(V, mplsFatalExceptionMode1EscapEn_f, &ipe_mpls_ftl_ctl, 0x3);/*b011*/

    SetIpeTunnelDecapFatalExcepCtl(V, fatalExceptionMode_f, &ipe_tunnel_decap_ftl_ctl, val_ipetunnel);
    SetIpeRouteFatalExcepCtl(V, routeFatalExceptionMode_f, &ipe_router_ftl_ctl, val_iperoute);
    SetIpeMplsFatalExcepCtl(V, mplsFatalExceptionMode_f, &ipe_mpls_ftl_ctl, 0x4);/*b100*/
    /*DRV_IS_TSINGMA:END*/

    cmd = DRV_IOW(IpeUserIdCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&user_id_ctl));
    cmd = DRV_IOW(IpeTunnelIdCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&tunnel_id_ctl));
    cmd = DRV_IOW(IpeTunnelDecapCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&tunnel_decap_ctl));
    cmd = DRV_IOW(IpeException3Ctl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_excp_ctl));
    cmd = DRV_IOW(IpeRouteCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_route_ctl));
    cmd = DRV_IOW(IpeLookupCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_lookup_ctl));
    cmd = DRV_IOW(IpeBridgeCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_bridge_ctl));
    cmd = DRV_IOW(IpeLearningCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_learn_ctl));
    cmd = DRV_IOW(IpeHeaderAdjustCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_hdr_adjust_ctl));
    cmd = DRV_IOW(IpeTunnelDecapFatalExcepCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_tunnel_decap_ftl_ctl));
    cmd = DRV_IOW(IpeRouteFatalExcepCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_router_ftl_ctl));
    cmd = DRV_IOW(IpeMplsFatalExcepCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_mpls_ftl_ctl));
    cmd = DRV_IOW(IpePreLookupProtocolCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&ipe_pre_lookup_proto_ctl));

    /*Arp Miss destmap to cpu*/
    field_val = 1;
	cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_destMapDiscardException_f);/*DUET2*/
	CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
	cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_destMapDiscardException_f);/*TSINGMA*/
	CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /*epe unknownDot11Packet to cpu*/
    field_val = 1;
    cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_unknownDot11PacketExceptionEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /*discardfatalexcption*/
    field_val = 0xEB6D;/*toCPU:0, discard:1; 1110 1011 0110 1101*/
    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_discardFatal_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_discardFatal_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

char*
sys_usw_reason_2Str(uint16 reason_id)
{
    static char  reason_desc[64] = {0};

    if (reason_id >= CTC_PKT_CPU_REASON_L2_PDU && reason_id < CTC_PKT_CPU_REASON_L3_PDU)
    {
        sal_sprintf(reason_desc,"L2_PDU(%d)",reason_id-CTC_PKT_CPU_REASON_L2_PDU);
        return reason_desc;
    }
    else if (reason_id >= CTC_PKT_CPU_REASON_L3_PDU && reason_id < CTC_PKT_CPU_REASON_IGMP_SNOOPING)
    {
        sal_sprintf(reason_desc,"L3_PDU(%d)",reason_id-CTC_PKT_CPU_REASON_L3_PDU);
        return reason_desc;
    }
    else if (reason_id >= CTC_PKT_CPU_REASON_OAM && reason_id < CTC_PKT_CPU_REASON_OAM_DEFECT_MESSAGE)
    {
        char* OAM_CPU_Reason[32] =
        {
            "INDIVIDUAL_OAM_PDU", "All_DEFECT_TO_CPU", "LBM/LBR", "HIGHER_VERSION_OAM", "MPLS-TP_LBM_PDU",
            "MPLS-TP_CSF", "ETH_SC_PDU", "ETH_LL_PDU", "CCM_HAS_OPTIONAL_TLV",
            "MPLS-TP_FM_PDU ", "Test_PDU", "Ether_SLM/SLR_PDU", "BIGInterval/MEPonSW",
            "TP_BFD_CV_PDU", "MPLS-TP_DLM", "MPLS-TP DM/DLMDM", "APS_PDU",
            "UNKNOWEN_PDU", "DM_PDU", "LEARNING_CCM_TO_CPU", "Y1731_CSF_PDU",
            "BFD_DISC_MISMATCH", "Y1731_MCC_PDU", "PBT_mmDefect_OAM", "LTM/LTR_PDU",
            "ALL_OAM_PDU_TO_CPU", "MACDA_CHK_FAIL", "LM_PDU", "LEARNING_BFD_TO_CPU",
            "TWAMP_PDU", "BFD_TIMER_NEGOTIATE", "Y1731_SCC_PDU"
        };

        return OAM_CPU_Reason[reason_id-CTC_PKT_CPU_REASON_OAM];
    }
   switch (reason_id)
   {
   case CTC_PKT_CPU_REASON_IGMP_SNOOPING:/* [GB] IGMP-Snooping*/
       return "IGMP_SNOOPING";
   case CTC_PKT_CPU_REASON_PTP:                 /* [GB.GG] PTP */
       return "PTP";
   case CTC_PKT_CPU_REASON_MPLS_OAM:            /* [GB.GG] TMPLS or MPLS protocol OAM */
       return "MPLS_OAM";
   case CTC_PKT_CPU_REASON_DCN:                /* [GB.GG] DCN */
       return "DCN";
   case CTC_PKT_CPU_REASON_SCL_MATCH:           /* [GB.GG] SCL entry match */
       return "SCL_MATCH";
   case CTC_PKT_CPU_REASON_ACL_MATCH:           /* [GB.GG] ACL entry match */
       return "ACL_MATCH";
   case CTC_PKT_CPU_REASON_SFLOW_SOURCE:        /* [GB.GG] SFlow source */
       return "SFLOW_SOURCE";
   case CTC_PKT_CPU_REASON_SFLOW_DEST:         /* [GB.GG] SFlow destination */
       return "SFLOW_DEST";
   case CTC_PKT_CPU_REASON_L2_PORT_LEARN_LIMIT:     /* [GB.GG] MAC learn limit base on port */
       return "PORT_LEARN_LIMIT";
   case CTC_PKT_CPU_REASON_L2_VLAN_LEARN_LIMIT:     /* [GB.GG] MAC learn limit base on vlan/vsi */
       return "VLAN_LEARN_LIMIT";
   case CTC_PKT_CPU_REASON_L2_SYSTEM_LEARN_LIMIT:   /* [GB.GG] MAC learn limit base on system */
       return "SYSTEM_LEARN_LIMIT";
   case CTC_PKT_CPU_REASON_L2_MOVE:             /* [GB.GG] MAC SA station move */
       return "L2_MOVE";
   case CTC_PKT_CPU_REASON_L2_CPU_LEARN:        /* [GB.GG] MAC SA CPU learning */
       return "L2_CPU_LEARN";
   case CTC_PKT_CPU_REASON_L2_COPY_CPU:         /* [GB.GG] MAC DA copy to CPU */
       return "L2_COPY_CPU";
   case CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL:   /* [GB.GG] IP TTL check fail */
       return "IP_TTL_CHECK_FAIL";
   case CTC_PKT_CPU_REASON_L3_MTU_FAIL:         /* [GB.GG] IP MTU check fail */
       return "L3_MTU_FAIL";
   case CTC_PKT_CPU_REASON_L3_MC_RPF_FAIL:     /* [GB.GG] IP multicast RPF check fail */
       return "L3_MC_RPF_FAIL";
   case CTC_PKT_CPU_REASON_L3_MC_MORE_RPF:      /* [GB.GG] IP multicast more RPF */
       return "L3_MC_MORE_RPF";
   case CTC_PKT_CPU_REASON_L3_IP_OPTION:        /* [GB.GG] IP option */
       return "L3_IP_OPTION";
   case CTC_PKT_CPU_REASON_L3_ICMP_REDIRECT:    /* [GB.GG] IP ICMP redirect */
       return "L3_ICMP_REDIRECT";
   case CTC_PKT_CPU_REASON_L3_COPY_CPU:        /* [GB.GG] IP DA copy to CPU */
       return "L3_COPY_CPU";
   case CTC_PKT_CPU_REASON_L3_MARTIAN_ADDR:     /* [GB.GG] IP martian address */
       return "L3_MARTIAN_ADDR";
   case CTC_PKT_CPU_REASON_L3_URPF_FAIL:        /* [GB.GG] IP URPF fail */
       return "L3_URPF_FAIL";
   case CTC_PKT_CPU_REASON_EPE_TTL_CHECK_FAIL: /* equal to CTC_PKT_CPU_REASON_IPMC_TTL_CHECK_FAIL*/
       return "TTL_CHECK_FAIL";
   case CTC_PKT_CPU_REASON_MPLS_TTL_CHECK_FAIL: /* [GB.GG] MPLS ttl check fail */
       return "MPLS_TTL_CHECK_FAIL";
   case CTC_PKT_CPU_REASON_GRE_UNKNOWN:         /* [GB.GG] GRE unknown */
       return "GRE_UNKNOWN";
   case CTC_PKT_CPU_REASON_LABEL_MISS:          /* [GB.GG] MPLS label out of range */
       return "LABEL_MISS";
   case CTC_PKT_CPU_REASON_LINK_ID_FAIL:        /* [GB.GG] IP Link id check fail */
       return "LINK_ID_FAIL";
   case CTC_PKT_CPU_REASON_OAM_HASH_CONFLICT:   /* [GB.GG] OAM hash conflict */
       return "OAM_HASH_CONFLICT";
   case CTC_PKT_CPU_REASON_IPFIX_HASH_CONFLICT:     /* [GG] Ipfix conflict*/
       return "IPFIX_HASH_CONFLICT";
   case CTC_PKT_CPU_REASON_VXLAN_NVGRE_CHK_FAIL:   /* [GG] Vxlan or NvGre check fail */
       return "VXLAN_NVGRE_CHK_FAIL";
   case CTC_PKT_CPU_REASON_OAM_DEFECT_MESSAGE: /*OAM Defect message from oam engine */
       return "OAM_DEFECT_MESSAGE";
   case CTC_PKT_CPU_REASON_FWD_CPU:              /* packet redirect forward to CPU */
       return "FWD_CPU";
   case CTC_PKT_CPU_REASON_NEW_ELEPHANT_FLOW:     /* [GG] new elephant flow  */
       return "NEW_ELEPHANT_FLOW";
   case CTC_PKT_CPU_REASON_L2_STORM_CTL:
       return "L2_STORM_CTL";  /*[GB.GG] storm control log*/
   case CTC_PKT_CPU_REASON_MONITOR_BUFFER_LOG:
       return "MONITOR_BUFFER"; /*[GG] Monitor buffer log*/
   case CTC_PKT_CPU_REASON_MONITOR_LATENCY_LOG:
       return "MONITOR_LATENCY"; /*[GG] Monitor buffer log*/
   case CTC_PKT_CPU_REASON_QUEUE_DROP_PKT:
       return "QUEUE_DROP_PKT"; /*[GG] queue drop packet*/
   case CTC_PKT_CPU_REASON_GENEVE_PKT:
       return "GENEVE_PKT"; /*[GG] Gnveve option packet */
   case CTC_PKT_CPU_REASON_MIRRORED_TO_CPU:
       return "MIRRORED_TO_CPU"; /*[GG] MIRRORED_TO_CPU */
   case CTC_PKT_CPU_REASON_ARP_MISS:
       return "ARP_MISS";
   case CTC_PKT_CPU_REASON_WLAN_CTL:
       return "WLAN_CTL";
   case CTC_PKT_CPU_REASON_WLAN_DTLS:
       return "WLAN_DTLS";
   case CTC_PKT_CPU_REASON_WLAN_DOT11_MGR:
       return "WLAN_DOT11_MGR";
   case CTC_PKT_CPU_REASON_WLAN_KEPP_ALIVE:
       return "WLAN_KEEP_ALIVE";
   case CTC_PKT_CPU_REASON_WLAN_CHECK_ERR:
       return "WLAN_CHECK_ERROR";
   case CTC_PKT_CPU_REASON_DOT1AE_CHECK_ERR:
       return "DOT1AE_CHECK_ERR";
   case CTC_PKT_CPU_REASON_DOT1AE_REACH_PN_THRD:
       return "DOT1AE_REACH_PN_THRD";
   case CTC_PKT_CPU_REASON_DOT1AE_DECRYPT_ICV_CHK_FAIL:
       return "DOT1AE_ICV_CHK_FAIL";
   case CTC_PKT_CPU_REASON_MCAST_TUNNEL_FRAG:
       return "MCAST_TUNNEL_FRAG";
   case CTC_PKT_CPU_REASON_TUNNEL_ECN_MIS:
       return "TUNNEL_ECN_MIS";
   case CTC_PKT_CPU_REASON_ICMP_ERR_MSG:
       return "ICMP_ERR_MSG";
   case CTC_PKT_CPU_REASON_C2C_PKT:                       /**< [GB.GG.D2] CPU-to-CPU packet */
        return "C2C_PKT";
   case  CTC_PKT_CPU_REASON_PIM_SNOOPING:                   /**< [D2] PIM-Snooping*/
        return "PIM_SNOOPING";
   case CTC_PKT_CPU_REASON_DKITS_CAPTURE_PKT:              /**< [D2] captured packet,used to dkis application*/
        return "DKIT_CAPTURE_PKT";
   case CTC_PKT_CPU_REASON_ELEPHANT_FLOW_LOG:              /**< [D2] elephant flow log */
        return "ELEPHANT_FLOW_LOG";
   case CTC_PKT_CPU_REASON_DIAG_DISCARD_PKT:
        return "DIAG_DISCARD_PKT";
   case CTC_PKT_CPU_REASON_UNKNOWN_GEM_PORT:
        return "UNKNOWN_GEM_PORT";
   case CTC_PKT_CPU_REASON_PON_PT_PKT:
        return "PON_PASS_THROUGH_PKT";
   default:
       sal_sprintf(reason_desc, "Reason(%d)", reason_id);
       return reason_desc;
   }
}

char*
sys_usw_reason_destType2Str(uint8 dest_type)
{
    switch (dest_type)
    {
    case CTC_PKT_CPU_REASON_TO_LOCAL_CPU:
        return "LCPU";
        break;

    case CTC_PKT_CPU_REASON_TO_LOCAL_PORT:
        return "LPort";
        break;

    case CTC_PKT_CPU_REASON_TO_REMOTE_CPU:
        return "RCPU";
        break;

    case CTC_PKT_CPU_REASON_TO_DROP:
        return "Drop";
        break;

    case CTC_PKT_CPU_REASON_TO_NHID:
        return "NHID";
        break;

    case CTC_PKT_CPU_REASON_TO_LOCAL_CPU_ETH:
        return "LCPU_ETH";
        break;

    default:
        break;
    }

    return "-";
}

char*
sys_usw_exception_type_str(uint8 lchip, uint16 exception_index,uint8 *sub_index)
{
    uint8 excp_type = 0;

    excp_type =  SYS_REASON_GET_EXCP_TYPE(exception_index);
    exception_index = SYS_REASON_GET_EXCP_INDEX(exception_index);

    if (excp_type == SYS_REASON_OAM_EXCP)
    {
        *sub_index =  0;
        return "OAM exception";
    }
    else if (excp_type == SYS_REASON_FWD_EXCP)
    {
        *sub_index =  0;
        return "Fwd exception";
    }
    else if (excp_type == SYS_REASON_MISC_NH_EXCP)
    {
        *sub_index =  0;
        return "Misc Nexthop exception";
    }
    else if (excp_type == SYS_REASON_FATAL_EXCP)
    {
        *sub_index =  0;
        return "Fatal exception";
    }
    else if (exception_index < 24)
    {    /*unexpect*/
        *sub_index = 0;
        return "Ingress mirror";
    }
    else if (exception_index < 32)
    {
        *sub_index = 0;
        return "Normal exception";
    }
    else if (exception_index < 44)  /*32~43*/
    {    /*unexpect*/
        *sub_index = 0;
        return "Egress mirror";
    }
    else if (exception_index < 56)  /*44-63*/
    {
        *sub_index = 0;
        return "Normal exception";
    }
    else if (exception_index < 64)  /*56-63*/
    {
        *sub_index = exception_index - 56;
        return "Normal exception(epe)";
    }
    else if (exception_index < 128)  /*64~127 */
    {
        *sub_index = exception_index - 64;
        return "Normal exception(2)";
    }
    else if (exception_index < 192)  /*128~191 */
    {
        *sub_index = exception_index - 128;
        return "Normal exception(3)";
    }
    else if (exception_index < 224)  /*192~223 */
    {
        *sub_index = exception_index - 192;
        return "Normal exception(other)";
    }
    else if (exception_index < 255)
    {
        *sub_index = 0;
        return "Normal exception";
    }

    *sub_index = 0;
    return "Other exception";
}

int32
_sys_usw_qos_cpu_reason_dump(uint8 lchip, uint16 start, uint16 end, uint8 detail)
{
    sys_cpu_reason_t* p_reason = NULL;
    ctc_qos_queue_id_t queue = {0};
    sys_queue_node_t      *p_queue_node = NULL;
    uint32 token_rate = 0;
    uint32 token_thrd = 0;
    uint16 queue_id = 0;
    uint32 dest_port = 0;
    uint16 reason_id = 0;
    uint8 is_pps = 0;
    uint8 loop = 0;
    uint8 channel = 0;

    LCHIP_CHECK(lchip);
    CTC_MAX_VALUE_CHECK(start,CTC_PKT_CPU_REASON_MAX_COUNT - 1);
    CTC_MAX_VALUE_CHECK(end,CTC_PKT_CPU_REASON_MAX_COUNT - 1);

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "CPU reason information:\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Shaping mode %s local CPU\n", p_usw_queue_master[lchip]->shp_pps_en ? "PPS" : "BPS");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Queue base %d for local CPU\n", MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP));
    if (p_usw_queue_master[lchip]->have_lcpu_by_eth)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Channel Id: %d for local CPU by network port\n", p_usw_queue_master[lchip]->cpu_eth_chan_id);
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Queue base %d for local CPU by network port\n", p_usw_queue_master[lchip]->cpu_eth_chan_id*8);
    }
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "PIR unit:pps(PPS)/kbps(BPS)\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " \n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "CPU reason Detail information:\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s %-20s %-8s %-7s %-7s %-9s %-9s\n", "Id", "reason" , "queue-id", "chanenl", "PIR", "dest-type", "dest-id");

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------------------------------------------------------\n");
    for (reason_id = start; reason_id <= end && (reason_id < CTC_PKT_CPU_REASON_MAX_COUNT); reason_id++)
    {
        queue_id = 0;
        channel  = 0;
        p_reason = &p_usw_queue_master[lchip]->cpu_reason[reason_id];

        if (p_reason && (CTC_PKT_CPU_REASON_TO_REMOTE_CPU == p_reason->dest_type))
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6d %-20s %-8s %-7s %-7s %9s %5d\n",
                               reason_id, sys_usw_reason_2Str(reason_id), "-" , "-",
                               "-", sys_usw_reason_destType2Str(p_reason->dest_type), p_reason->sub_queue_id);
            continue;
        }

        if (reason_id >= 256 && !p_reason->user_define_mode)
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6d %-20s %-8s %-7s %-7s %9s %5s\n",
                               reason_id, sys_usw_reason_2Str(reason_id), "-" , "-",
                               "-", "-", "-");
            continue;
        }

        queue.cpu_reason = reason_id;
        queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
        sys_usw_queue_get_queue_id(lchip, &queue, &queue_id);
        p_queue_node = ctc_vector_get(p_usw_queue_master[lchip]->queue_vec, queue_id);
        if (!p_reason || !p_queue_node)
        {
            continue;
        }
        CTC_ERROR_RETURN(sys_usw_get_channel_by_queue_id(lchip, queue_id, &channel));
        if (p_reason->dest_type == CTC_PKT_CPU_REASON_TO_DROP)
        {
            channel = MCHIP_CAP(SYS_CAP_CHANID_DROP);
        }
        is_pps = sys_usw_queue_shp_get_channel_pps_enbale(lchip, channel);

        if (p_queue_node->shp_en)
        {
            sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, p_queue_node->p_shp_profile->bucket_rate, &token_rate,
                                                    MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH), p_usw_queue_master[lchip]->que_shp_update_freq, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES));

            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6d %-20s %-8d %-7d %-7u ",
                               reason_id, sys_usw_reason_2Str(reason_id),
                               queue_id, channel, token_rate);
        }
        else
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6d %-20s %-8d %-7d %-7s ",
                               reason_id, sys_usw_reason_2Str(reason_id),
                               queue_id , channel, "-");
        }

        if (p_reason->dest_type == CTC_PKT_CPU_REASON_TO_LOCAL_PORT
            || p_reason->dest_type == CTC_PKT_CPU_REASON_TO_NHID
            || p_reason->dest_type == CTC_PKT_CPU_REASON_TO_DROP
            || p_reason->dest_type == CTC_PKT_CPU_REASON_TO_LOCAL_CPU_ETH)
        {
            dest_port = p_reason->dest_port;
        }
        else
        {
            dest_port = p_reason->sub_queue_id;
        }

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%9s %5d",
                           sys_usw_reason_destType2Str(p_reason->dest_type),
                           dest_port);
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    }

    if (start != end)
    {
        return CTC_E_NONE;
    }

    reason_id = start;
    if ((start == end) && p_queue_node)
    {
        uint32 shp_en = 0;
        uint32 cmd = 0;
        uint32 token_thrd_shift = 0;
        uint32 token_rate_frac = 0;
        QMgrDmaChanShpProfile_m  dma_chan_shp_profile;
        sal_memset(&dma_chan_shp_profile, 0, sizeof(QMgrDmaChanShpProfile_m));

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nQueue shape:\n");
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------\n");

        if (p_queue_node->shp_en)
        {
            sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, p_queue_node->p_shp_profile->bucket_rate, &token_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                                    p_usw_queue_master[lchip]->que_shp_update_freq, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES));
            _sys_usw_qos_map_token_thrd_hw_to_user(lchip, &token_thrd , p_queue_node->p_shp_profile->bucket_thrd, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH));
            SYS_QOS_GET_SHAPE_TOKEN_THRD(is_pps, token_thrd);
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s: %u (%s)\n", "PIR", token_rate, is_pps ?"pps":"kbps");
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s: %u (%s)\n", "PBS", token_thrd, is_pps ?"packet":"kb");
        }
        else
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "No PIR Bucket\n");
        }

        cmd = DRV_IOR(QMgrDmaChanShpProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dma_chan_shp_profile));

        token_thrd = GetQMgrDmaChanShpProfile(V, dmaChanShpTokenThrd_f, &dma_chan_shp_profile);
        token_thrd_shift = GetQMgrDmaChanShpProfile(V, dmaChanShpTokenThrdShift_f, &dma_chan_shp_profile);
        token_rate_frac = GetQMgrDmaChanShpProfile(V, dmaChanShpTokenRateFrac_f, &dma_chan_shp_profile);
        token_rate = GetQMgrDmaChanShpProfile(V, dmaChanShpTokenRate_f, &dma_chan_shp_profile);

        shp_en = (token_thrd_shift == MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT)) ? 0 : 1;
        is_pps = 1;
        sys_usw_qos_map_token_rate_hw_to_user(lchip, is_pps, (token_rate << 8 | token_rate_frac), &token_rate, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_RATE_BIT_WIDTH),
                                              p_usw_queue_master[lchip]->chan_shp_update_freq, MCHIP_CAP(SYS_CAP_QOS_SHP_PPS_PACKET_BYTES));
        _sys_usw_qos_map_token_thrd_hw_to_user(lchip, &token_thrd ,
                                               (token_thrd << MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH) | token_thrd_shift), MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFTS_WIDTH));

        SYS_QOS_GET_SHAPE_TOKEN_THRD(is_pps, token_thrd);

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nPort shape:\n");
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------\n");
        if (shp_en)
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s: %u (%s)\n", "PIR", token_rate, is_pps ?"pps":"kbps");
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s: %u (%s)\n", "PBS", token_thrd, is_pps ?"packet":"kb");
        }
        else
        {
            SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "No PIR Bucket\n");
        }
    }

    if ((start == end) && p_queue_node)
    {
        ctc_qos_sched_queue_t  queue_sched;

        sal_memset(&queue_sched, 0,  sizeof(queue_sched));
        queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;

        CTC_ERROR_RETURN(sys_usw_queue_sch_get_queue_sched(lchip, queue_id , &queue_sched));

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nQueue Scheduler:\n");
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------\n");
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s: %d \n", "Queue Class(Yellow)", queue_sched.exceed_class);
    }

    if (start == end)
    {
        uint16 excp_array[32];
        uint8 excp_num = 0;
        uint32 offset = 0;
        uint8 excp_type = 0;
        uint32 excp_idx = 0;
        uint32 cmd  = 0;
        uint32 value = 0;
        char fwd_str[50] = {0};

        sys_usw_queue_get_excp_idx_by_reason(lchip, start, excp_array, &excp_num);
        p_reason = &p_usw_queue_master[lchip]->cpu_reason[reason_id];

        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nCPU Reason Table:\n");
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------\n");
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s %-10s %-10s %-5s\n",   "Table", "Offset","Type","ExcepGroupId");
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------\n");

        for (loop = 0; loop < excp_num; loop++)
        {
            excp_type = SYS_REASON_GET_EXCP_TYPE(excp_array[loop]);
            excp_idx = SYS_REASON_GET_EXCP_INDEX(excp_array[loop]);
            if (SYS_REASON_FWD_EXCP == excp_type)
            {
                uint32  dsfwd_offset;

                CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, CTC_NH_RESERVED_NHID_FOR_TOCPU,
                                                                    &dsfwd_offset, 0));
                sal_sprintf(fwd_str, "%u%s", dsfwd_offset, "[3w]");
                SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s %-10s %-10s %-5s\n", "DsFwd", fwd_str, "Fwd", "-");
            }
            else if (SYS_REASON_MISC_NH_EXCP == excp_type)
            {
                uint32  dsfwd_offset = p_reason->dsfwd_offset;
                sal_sprintf(fwd_str, "%u%s", dsfwd_offset, "[3w]");
                SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s %-10s %-10s %-5s\n", "DsFwd", fwd_str, "Misc-NH", "-");
            }
            else if (SYS_REASON_OAM_EXCP == excp_type)
            {
                if ((excp_idx + CTC_PKT_CPU_REASON_OAM) == CTC_PKT_CPU_REASON_OAM_DEFECT_MESSAGE)
                {
                    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s %-10d %-10s %-5s\n", "OamUpdateApsCtl_t", 0, "Oam","-");
                }
                else
                {
                    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s %-10d %-10s %-5s\n", "DsOamExcp", excp_idx, "Oam", "-");
                }
            }
            else if (SYS_REASON_FATAL_EXCP == excp_type)
            {
                CTC_ERROR_RETURN(sys_usw_nh_get_fatal_excp_dsnh_offset(lchip, &offset));
                offset = offset + excp_idx * 2;
                sal_sprintf(fwd_str, "%u%s", offset, "[3w]");
                SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s %-10s %-10s %-5s\n", "DsFwd", fwd_str, "FATAL", "-");
            }
            else if (SYS_REASON_NORMAL_EXCP == excp_type)
            {
                SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s %-10d ", "DsMetFifoExcp", excp_idx);

                if (excp_idx <  SYS_NORMAL_EXCP_TYPE_EPE_OTHER)
                {
                    cmd = DRV_IOR(IpeFwdExcepGroupMap_t, IpeFwdExcepGroupMap_array_0_exceptionGid_f + (excp_idx & 0x3));
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (excp_idx >> 2), cmd, &value));
                    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-10s %-5d\n", "IPE", value);
                }
                else if (excp_idx < SYS_NORMAL_EXCP_TYPE_EPE_L2SPAN_BASE)
                {
                    excp_idx = excp_idx -192;
                    excp_idx = excp_idx & 0xF;
                    cmd = DRV_IOR(EpeAclQosCtl_t, EpeAclQosCtl_gExcepGidMap_0_exceptionGid_f + excp_idx);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0, cmd, &value));
                    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-10s %-5d\n", "EPE", value);
                }
                else
                {
                    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-10s %-5s\n", "BSR", "-");
                }
            }
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_cpu_reason_init(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    uint16 reason = 0;
    uint16 idx = 0;
    uint8 queue_id = 0;
    uint8 group_id = 0;
    uint32 max_size    = 0;
    uint32 start_offset = 0;
    uint32 entry_size = 0;
    sys_usw_opf_t opf;
    uint8 reason_grp8[] =
    {
        CTC_PKT_CPU_REASON_IGMP_SNOOPING,
        CTC_PKT_CPU_REASON_L3_ICMP_REDIRECT,
        CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL,
        CTC_PKT_CPU_REASON_L3_MTU_FAIL,
        CTC_PKT_CPU_REASON_L3_MC_RPF_FAIL,
        CTC_PKT_CPU_REASON_L3_IP_OPTION,
        CTC_PKT_CPU_REASON_L3_COPY_CPU,
        CTC_PKT_CPU_REASON_LINK_ID_FAIL
    };
    uint8 reason_grp9[] =
    {
       CTC_PKT_CPU_REASON_MPLS_TTL_CHECK_FAIL,
       CTC_PKT_CPU_REASON_MPLS_OAM,
       CTC_PKT_CPU_REASON_PTP,
       CTC_PKT_CPU_REASON_NEW_ELEPHANT_FLOW,
       CTC_PKT_CPU_REASON_MONITOR_BUFFER_LOG,
       CTC_PKT_CPU_REASON_MONITOR_LATENCY_LOG,
       CTC_PKT_CPU_REASON_GENEVE_PKT
    };
    uint8 reason_grp10[] =
    {
       CTC_PKT_CPU_REASON_SCL_MATCH,
       CTC_PKT_CPU_REASON_ACL_MATCH,
       CTC_PKT_CPU_REASON_SFLOW_SOURCE,
       CTC_PKT_CPU_REASON_L2_PORT_LEARN_LIMIT,
       CTC_PKT_CPU_REASON_L2_MOVE,
       CTC_PKT_CPU_REASON_L2_COPY_CPU,
       CTC_PKT_CPU_REASON_OAM_DEFECT_MESSAGE,
       CTC_PKT_CPU_REASON_IPFIX_HASH_CONFLICT,
       CTC_PKT_CPU_REASON_ARP_MISS
    };
    uint8 reason_drop_queue[] =
    {
        CTC_PKT_CPU_REASON_DROP,
        CTC_PKT_CPU_REASON_L3_MARTIAN_ADDR,
        CTC_PKT_CPU_REASON_L3_URPF_FAIL,
        CTC_PKT_CPU_REASON_EPE_TTL_CHECK_FAIL,  /* equal to CTC_PKT_CPU_REASON_IPMC_TTL_CHECK_FAIL*/
        CTC_PKT_CPU_REASON_DCN,
        CTC_PKT_CPU_REASON_L2_SYSTEM_LEARN_LIMIT,
        CTC_PKT_CPU_REASON_L2_CPU_LEARN,
        CTC_PKT_CPU_REASON_GRE_UNKNOWN,
        CTC_PKT_CPU_REASON_LABEL_MISS,
        CTC_PKT_CPU_REASON_OAM_HASH_CONFLICT,
        CTC_PKT_CPU_REASON_VXLAN_NVGRE_CHK_FAIL,
        CTC_PKT_CPU_REASON_L2_STORM_CTL,
        CTC_PKT_CPU_REASON_WLAN_CHECK_ERR,
        CTC_PKT_CPU_REASON_DOT1AE_CHECK_ERR,
        CTC_PKT_CPU_REASON_ICMP_ERR_MSG,
        CTC_PKT_CPU_REASON_TUNNEL_ECN_MIS,
        CTC_PKT_CPU_REASON_MCAST_TUNNEL_FRAG,
        CTC_PKT_CPU_REASON_DIAG_DISCARD_PKT,
        CTC_PKT_CPU_REASON_DKITS_CAPTURE_PKT,
        CTC_PKT_CPU_REASON_L3_MC_MORE_RPF
    };

    for (idx = 0; idx < CTC_PKT_CPU_REASON_MAX_COUNT; idx++)
    {
        p_usw_queue_master[lchip]->cpu_reason[idx].sub_queue_id = CTC_MAX_UINT16_VALUE;
    }

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_queue_master[lchip]->opf_type_excp_index,
                                                                        2, "opf-exception-index"), ret, error0);
    opf.pool_type = p_usw_queue_master[lchip]->opf_type_excp_index;

    /*Init Ingress free exception index*/
    opf.pool_index = 0;
    start_offset = CTC_PKT_CPU_REASON_L2_PDU_RESV;
    max_size = SYS_NORMAL_EXCP_TYPE_EPE_OTHER - CTC_PKT_CPU_REASON_L2_PDU_RESV + 1;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size),
                                                                                    ret, error1);
    CTC_ERROR_GOTO(sys_usw_opf_alloc_offset_from_position(lchip, &opf, max_size, start_offset),
                                                                                    ret, error1);

    /*free l2pdu index*/
    opf.pool_index = 0;
    start_offset = CTC_PKT_CPU_REASON_L2_PDU_RESV+1;
    entry_size = SYS_NORMAL_EXCP_SUB_TYPE_l2_UNUSED_END - CTC_PKT_CPU_REASON_L2_PDU_RESV;
    CTC_ERROR_GOTO(sys_usw_opf_free_offset(lchip, &opf,  entry_size, start_offset),
                                                                                    ret, error1);

    /*free l3pdu index*/
    opf.pool_index = 0;
    start_offset = SYS_NORMAL_EXCP_TYPE_LAYER3 + CTC_PKT_CPU_REASON_L3_PDU_RESV;
    entry_size = SYS_NORMAL_EXCP_SUB_TYPE_l3_UNUSED_END - CTC_PKT_CPU_REASON_L3_PDU_RESV + 1;
    CTC_ERROR_GOTO(sys_usw_opf_free_offset(lchip, &opf,  entry_size, start_offset),
                                                                                    ret, error1);

    /*free ipe other index*/
    opf.pool_index = 0;
    start_offset = SYS_NORMAL_EXCP_TYPE_IPE_OTHER + SYS_NORMAL_EXCP_SUB_TYPE_IPE_OTHER_UNUSED_BEGIN;
    entry_size = SYS_NORMAL_EXCP_SUB_TYPE_IPE_OTHER_UNUSED_END - SYS_NORMAL_EXCP_SUB_TYPE_IPE_OTHER_UNUSED_BEGIN + 1;
    CTC_ERROR_GOTO(sys_usw_opf_free_offset(lchip, &opf,  entry_size, start_offset),
                                                                                    ret, error1);

    opf.pool_index = 0;
    start_offset = SYS_NORMAL_EXCP_TYPE_IPE_OTHER_UNUSED0;
    entry_size = 1;
    CTC_ERROR_GOTO(sys_usw_opf_free_offset(lchip, &opf,  entry_size, start_offset),
                                                                                    ret, error1);

    opf.pool_index = 0;
    start_offset = SYS_NORMAL_EXCP_TYPE_IPE_OTHER_UNUSED1;
    entry_size = 1;
    CTC_ERROR_GOTO(sys_usw_opf_free_offset(lchip, &opf,  entry_size, start_offset),
                                                                                    ret, error1);

    opf.pool_index = 0;
    start_offset = SYS_NORMAL_EXCP_TYPE_IPE_OTHER_UNUSED2;
    entry_size = 1;
    CTC_ERROR_GOTO(sys_usw_opf_free_offset(lchip, &opf,  entry_size, start_offset),
                                                                                    ret, error1);

    opf.pool_index = 0;
    start_offset = SYS_NORMAL_EXCP_TYPE_IPE_OTHER_UNUSED3;
    entry_size = 1;
    CTC_ERROR_GOTO(sys_usw_opf_free_offset(lchip, &opf,  entry_size, start_offset),
                                                                                    ret, error1);

    /*init Egress free exception index*/
    opf.pool_index = 1;
    start_offset = SYS_NORMAL_EXCP_TYPE_EPE_OTHER;
    max_size = SYS_NORMAL_EXCP_TYPE_EPE_L2SPAN_BASE - SYS_NORMAL_EXCP_TYPE_EPE_OTHER + 1;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, start_offset, max_size),
                                                                                    ret, error1);
    CTC_ERROR_GOTO(sys_usw_opf_alloc_offset_from_position(lchip, &opf,  max_size, start_offset),
                                                                                    ret, error1);

    opf.pool_index = 1;
    start_offset = SYS_NORMAL_EXCP_TYPE_EPE_OTHER + SYS_NORMAL_EXCP_SUB_TYPE_UNUSER0;
    entry_size = 1;
    CTC_ERROR_GOTO(sys_usw_opf_free_offset(lchip, &opf,  entry_size, start_offset),
                                                                                    ret, error1);

    opf.pool_index = 1;
    start_offset = SYS_NORMAL_EXCP_TYPE_EPE_OTHER + SYS_NORMAL_EXCP_SUB_TYPE_EPE_OTHER_UNUSED_BEGIN;
    entry_size = SYS_NORMAL_EXCP_SUB_TYPE_EPE_OTHER_UNUSED_END - SYS_NORMAL_EXCP_SUB_TYPE_EPE_OTHER_UNUSED_BEGIN + 1;
    CTC_ERROR_GOTO(sys_usw_opf_free_offset(lchip, &opf,  entry_size, start_offset),
                                                                                    ret, error1);

    /*L2PDU ,reason group :0~3*/
    for (idx = 0; idx < CTC_PKT_CPU_REASON_L2_PDU_RESV; idx++)
    {
        reason = CTC_PKT_CPU_REASON_L2_PDU + idx;
        group_id = idx / 8;
        queue_id = idx % 8;
        CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip,  reason, queue_id, group_id), ret, error1);
    }

    /*L3PDU ,reason group :4~7*/
    for (idx = 0; idx < CTC_PKT_CPU_REASON_L3_PDU_RESV; idx++)
    {
        reason = CTC_PKT_CPU_REASON_L3_PDU + idx;
        group_id = idx / 8 + 4;
        queue_id = idx % 8;
        CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip,  reason, queue_id, group_id), ret, error1);
    }

    /*L3 packet ,reason group: 8*/
    for (idx = 0; idx < sizeof(reason_grp8) / sizeof(uint8); idx++)
    {
        reason = reason_grp8[idx];
        group_id = 8;
        queue_id = idx % 8;
        CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip,  reason, queue_id, group_id), ret, error1);
    }

    /*mpls & misc packet, reason group: 9*/
    for (idx = 0; idx < sizeof(reason_grp9) / sizeof(uint8); idx++)
    {
        reason = reason_grp9[idx];
        group_id = 9;
        queue_id = idx % 8;
        CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip,  reason, queue_id, group_id), ret, error1);
    }

    reason = CTC_PKT_CPU_REASON_WLAN_KEPP_ALIVE;
    group_id = 9;
    queue_id = 7;
    CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip,  reason, queue_id, group_id), ret, error1);

    reason = CTC_PKT_CPU_REASON_WLAN_DTLS;
    group_id = 9;
    queue_id = 7;
    CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip,  reason, queue_id, group_id), ret, error1);

    reason = CTC_PKT_CPU_REASON_WLAN_CTL;
    group_id = 9;
    queue_id = 7;
    CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip,  reason, queue_id, group_id), ret, error1);

    reason = CTC_PKT_CPU_REASON_WLAN_DOT11_MGR;
    group_id = 9;
    queue_id = 7;
    CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip,  reason, queue_id, group_id), ret, error1);

    if (DRV_IS_TSINGMA(lchip))
    {
        reason = CTC_PKT_CPU_REASON_UNKNOWN_GEM_PORT;
        group_id = 9;
        queue_id = 7;
        CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip,  reason, queue_id, group_id), ret, error1);

        reason = CTC_PKT_CPU_REASON_PON_PT_PKT;
        group_id = 9;
        queue_id = 7;
        CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip,  reason, queue_id, group_id), ret, error1);
    }
    /*l2 packet, reason group: 10*/
    for (idx = 0; idx < sizeof(reason_grp10) / sizeof(uint8); idx++)
    {
        reason = reason_grp10[idx];
        group_id = 10;
        queue_id = idx % 8;
        CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip,  reason, queue_id, group_id), ret, error1);
    }

    reason = CTC_PKT_CPU_REASON_SFLOW_DEST;
    group_id = 10;
    queue_id = 2;
    CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip,  reason, queue_id, group_id), ret, error1);

    reason = CTC_PKT_CPU_REASON_L2_VLAN_LEARN_LIMIT;
    group_id = 10;
    queue_id = 3;
    CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip,  reason, queue_id, group_id), ret, error1);

    /*OAMPDU : reason group :11~12*/
    for (idx = 0; idx < CTC_PKT_CPU_REASON_OAM_PDU_RESV; idx++)
    {
        reason = CTC_PKT_CPU_REASON_OAM + idx;
        group_id = idx / 16 + 11;
        queue_id = idx % 8;
        ret = sys_usw_cpu_reason_set_map(lchip, reason, queue_id, group_id);
        if ((CTC_E_NOT_SUPPORT == ret) || (CTC_E_NONE == ret))
        {
            continue;
        }
        else
        {
            goto error1;
        }
    }

    /*FWD to CPU,reason group :13*/
    reason = CTC_PKT_CPU_REASON_FWD_CPU;
    group_id = 13;
    queue_id = 0;
    CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip, reason, queue_id, group_id), ret, error1);

    /*Mirror to CPU,reason group :13*/
    reason = CTC_PKT_CPU_REASON_MIRRORED_TO_CPU;
    group_id = 13;
    queue_id = 0;
    CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip,  reason, queue_id, group_id), ret, error1);

    reason = CTC_PKT_CPU_REASON_DOT1AE_REACH_PN_THRD;
    group_id = 13;
    queue_id = 1;
    CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip, reason, queue_id, group_id), ret, error1);

    reason = CTC_PKT_CPU_REASON_DOT1AE_DECRYPT_ICV_CHK_FAIL;
    group_id = 13;
    queue_id = 2;
    CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip, reason, queue_id, group_id), ret, error1);

    reason = CTC_PKT_CPU_REASON_PIM_SNOOPING;
    group_id = 8;
    queue_id = 0;
    CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip, reason, queue_id, group_id), ret, error1);

    if (0 == p_usw_queue_master[lchip]->enq_mode)
    {
        /*c2c*/
        reason = CTC_PKT_CPU_REASON_C2C_PKT;
        group_id = 14;
        queue_id = 0;
        CTC_ERROR_GOTO(sys_usw_cpu_reason_set_map(lchip, reason, queue_id, group_id), ret, error1);
    }

    for (idx = 0; idx < sizeof(reason_drop_queue) / sizeof(uint8); idx++)
    {
        reason = reason_drop_queue[idx];
        ret = (sys_usw_cpu_reason_set_dest(lchip, reason,
                                                            CTC_PKT_CPU_REASON_TO_DROP,
                                                            0));
        if ((CTC_E_NOT_SUPPORT == ret) || (CTC_E_NONE == ret))
        {
            continue;
        }
        else
        {
            goto error1;
        }
    }

    /*config corrsponding register */
    CTC_ERROR_GOTO(_sys_usw_cpu_reason_register_init(lchip), ret, error1);

    /*Init Ingress free exception index*/
    /*free l2pdu index*/

    /*free l3pdu index*/

    /*free ipe other index*/

    /*init Egress free exception index*/
    /*free epe other index*/

    /*set excetpion group*/
    for (reason = 0; reason < CTC_PKT_CPU_REASON_CUSTOM_BASE; reason++)
    {
        uint16 excp_array[32] = {0};
        uint8 excp_num = 0;
        uint16 exception_idx = 0;
        uint16 excp_type = 0;
        uint16 excp_idx = 0;
        uint32 cmd = 0;
        uint32 field_val = 0;

        sys_usw_queue_get_excp_idx_by_reason(lchip,  reason,
                                                   excp_array,
                                                   &excp_num);

        /*set exception group for acl*/
        for (idx = 0; idx < excp_num; idx++)
        {
            exception_idx = excp_array[idx];
            excp_type = SYS_REASON_GET_EXCP_TYPE(exception_idx);
            excp_idx = SYS_REASON_GET_EXCP_INDEX(exception_idx);
            field_val = reason;
            if (excp_type != SYS_REASON_NORMAL_EXCP
                || excp_idx >= SYS_NORMAL_EXCP_TYPE_EPE_L2SPAN_BASE)
            {
                /*only ipe & epe normal exceltion support exception group*/
                continue;
            }

            if (excp_type == SYS_REASON_NORMAL_EXCP && excp_idx < SYS_NORMAL_EXCP_TYPE_EPE_OTHER)
            {
                cmd = DRV_IOW(IpeFwdExcepGroupMap_t, IpeFwdExcepGroupMap_array_0_exceptionGid_f + (excp_idx & 0x3));
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, (excp_idx >> 2), cmd, &field_val), ret, error1);
            }
            else
            {
                excp_idx = excp_idx - 192;
                excp_idx = excp_idx & 0xF;
                cmd = DRV_IOW(EpeAclQosCtl_t, EpeAclQosCtl_gExcepGidMap_0_exceptionGid_f + excp_idx);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip,0, cmd, &field_val), ret, error1);
            }
        }
    }

    return CTC_E_NONE;

error1:
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_excp_index);
error0:
    return ret;
}

int32
sys_usw_queue_cpu_reason_deinit(uint8 lchip)
{
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_excp_index);

    return CTC_E_NONE;
}

