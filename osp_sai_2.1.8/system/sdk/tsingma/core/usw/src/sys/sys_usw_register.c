/**
 @file sys_usw_register.c

 @date 2009-11-6

 @version v2.0


*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctcs_api.h"
#include "ctc_error.h"
#include "ctc_warmboot.h"
#include "ctc_packet.h"
#include "ctc_l2.h"

#include "sys_usw_common.h"
#include "sys_usw_pdu.h"
#include "sys_usw_chip.h"
#include "sys_usw_register.h"
#include "sys_usw_ftm.h"
#include "sys_usw_parser.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_interrupt.h"
#include "sys_usw_qos_api.h"
#include "ctc_register.h"
#include "sys_usw_dma.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_opf.h"
#include "sys_usw_l2_fdb.h"

#include "drv_api.h"
#include "sal_mutex.h"

/*IPE Discard Type*/
#define IPEDISCARDTYPE_RESERVED0                          0x0
#define IPEDISCARDTYPE_LPBK_HDR_ADJ_RM_ERR                0x1
#define IPEDISCARDTYPE_PARSER_LEN_ERR                     0x2
#define IPEDISCARDTYPE_UC_TO_LAG_GROUP_NO_MEMBER          0x3
#define IPEDISCARDTYPE_EXCEP2_DIS                         0x4
#define IPEDISCARDTYPE_DS_USERID_DIS                      0x5
#define IPEDISCARDTYPE_RECEIVE_DIS                        0x6
#define IPEDISCARDTYPE_MICROFLOW_POLICING_FAIL_DROP       0x7
#define IPEDISCARDTYPE_PROTOCOL_VLAN_DIS                  0x8
#define IPEDISCARDTYPE_AFT_DIS                            0x9
#define IPEDISCARDTYPE_L2_ILLEGAL_PKT_DIS                 0xa
#define IPEDISCARDTYPE_STP_DIS                            0xb
#define IPEDISCARDTYPE_DEST_MAP_PROFILE_DISCARD           0xc
#define IPEDISCARDTYPE_STACK_REFLECT_DISCARD              0xd
#define IPEDISCARDTYPE_ARP_DHCP_DIS                       0xe
#define IPEDISCARDTYPE_DS_PHYPORT_SRCDIS                  0xf
#define IPEDISCARDTYPE_VLAN_FILTER_DIS                    0x10
#define IPEDISCARDTYPE_DS_SCL_DIS                         0x11
#define IPEDISCARDTYPE_ROUTE_ERROR_PKT_DIS                0x12
#define IPEDISCARDTYPE_SECURITY_CHK_DIS                   0x13
#define IPEDISCARDTYPE_STORM_CTL_DIS                      0x14
#define IPEDISCARDTYPE_LEARNING_DIS                       0x15
#define IPEDISCARDTYPE_NO_FORWARDING_PTR                  0x16
#define IPEDISCARDTYPE_IS_DIS_FORWARDING_PTR              0x17
#define IPEDISCARDTYPE_FATAL_EXCEP_DIS                    0x18
#define IPEDISCARDTYPE_APS_DIS                            0x19
#define IPEDISCARDTYPE_DS_FWD_DESTID_DIS                  0x1a
#define IPEDISCARDTYPE_LOOPBACK_DIS                       0x1b
#define IPEDISCARDTYPE_DISCARD_PACKET_LOG_ALL_TYPE        0x1c
#define IPEDISCARDTYPE_PORT_MAC_CHECK_DIS                 0x1d
#define IPEDISCARDTYPE_L3_EXCEP_DIS                       0x1e
#define IPEDISCARDTYPE_STACKING_HDR_CHK_ERR               0x1f
#define IPEDISCARDTYPE_TUNNEL_DECAP_MARTIAN_ADD           0x20
#define IPEDISCARDTYPE_TUNNELID_FWD_PTR_DIS               0x21
#define IPEDISCARDTYPE_VXLAN_FLAG_CHK_ERROR_DISCARD       0x22
#define IPEDISCARDTYPE_VXLAN_NVGRE_INNER_VTAG_CHK_DIS     0x23
#define IPEDISCARDTYPE_VXLAN_NVGRE_CHK_FAIL               0x24
#define IPEDISCARDTYPE_GENEVE_PAKCET_DISCARD              0x25
#define IPEDISCARDTYPE_ICMP_REDIRECT_DIS                  0x26
#define IPEDISCARDTYPE_ICMP_ERR_MSG_DIS                   0x27
#define IPEDISCARDTYPE_PTP_PKT_DIS                        0x28
#define IPEDISCARDTYPE_MUX_PORT_ERR                       0x29
#define IPEDISCARDTYPE_HW_ERROR_DISCARD                   0x2a
#define IPEDISCARDTYPE_USERID_BINDING_DIS                 0x2b
#define IPEDISCARDTYPE_DS_PLC_DIS                          0x2c
#define IPEDISCARDTYPE_DS_ACL_DIS                          0x2d
#define IPEDISCARDTYPE_DOT1AE_CHK                         0x2e
#define IPEDISCARDTYPE_OAM_DISABLE                        0x2f
#define IPEDISCARDTYPE_OAM_NOT_FOUND                      0x30
#define IPEDISCARDTYPE_CFLAX_SRC_ISOLATE_DIS              0x31
#define IPEDISCARDTYPE_OAM_ETH_VLAN_CHK                   0x32
#define IPEDISCARDTYPE_OAM_BFD_TTL_CHK                    0x33
#define IPEDISCARDTYPE_OAM_FILTER_DIS                     0x34
#define IPEDISCARDTYPE_TRILL_CHK                          0x35
#define IPEDISCARDTYPE_WLAN_CHK                           0x36
#define IPEDISCARDTYPE_TUNNEL_ECN_DIS                     0x37
#define IPEDISCARDTYPE_EFM_DIS                            0x38
#define IPEDISCARDTYPE_ILOOP_DIS                          0x39
#define IPEDISCARDTYPE_MPLS_ENTROPY_LABEL_CHK             0x3a
#define IPEDISCARDTYPE_MPLS_TP_MCC_SCC_DIS                0x3b
#define IPEDISCARDTYPE_MPLS_MC_PKT_ERROR                  0x3c
#define IPEDISCARDTYPE_L2_EXCPTION_DIS                    0x3d
#define IPEDISCARDTYPE_NAT_PT_CHK                         0x3e
#define IPEDISCARDTYPE_SD_CHECK_DIS                          0x3f


/*EPE Discard Type*/


struct sys_register_cethertype_s
{
    uint16 ether_type;

    uint8  lsv;
    uint8  cethertype_index;
};
typedef struct sys_register_cethertype_s sys_register_cethertype_t;

struct sys_usw_register_tcat_prof_s
{
    uint8  shift;
    uint8  length;

    uint8  profile_id;
    uint8  rsv;
};
typedef struct sys_usw_register_tcat_prof_s sys_usw_register_tcat_prof_t;

sys_register_master_t* p_usw_register_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define SYS_REGISTER_DBG_OUT(level, FMT, ...) \
    CTC_DEBUG_OUT(register, register, REGISTER_SYS, level, FMT, ## __VA_ARGS__)

#define SYS_USW_REGISTER_LOCK(lchip) \
    if (p_usw_register_master[lchip]->register_mutex) sal_mutex_lock(p_usw_register_master[lchip]->register_mutex)

#define SYS_USW_REGISTER_UNLOCK(lchip) \
    if (p_usw_register_master[lchip]->register_mutex) sal_mutex_unlock(p_usw_register_master[lchip]->register_mutex)

#define SYS_REGISTER_INIT_CHECK         \
    {                                \
        SYS_LCHIP_CHECK_ACTIVE(lchip);          \
        if (!p_usw_register_master[lchip]) {           \
            SYS_REGISTER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n"); \
			return CTC_E_NOT_INIT; \
 } \
    }
extern sys_chip_master_t* p_usw_chip_master[CTC_MAX_LOCAL_CHIP_NUM];
/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/
 extern int32
sys_usw_datapath_wb_sync(uint8 lchip);

extern int32
sys_usw_mac_wb_sync(uint8 lchip);

extern int32
sys_usw_ftm_wb_sync(uint8 lchip);

extern int32
sys_usw_internal_port_wb_sync(uint8 lchip);

extern int32
sys_usw_acl_set_global_cfg(uint8 lchip, uint8 property, uint32 value);

extern int32
sys_usw_mac_stats_wb_sync(uint8 lchip);

extern int32
sys_usw_mdio_init(uint8 lchip);

extern int32
sys_usw_inband_init(uint8 lchip);
extern int32
sys_usw_inband_deinit(uint8 lchip);

 STATIC int32
 _sys_usw_discard_type_init(uint8 lchip)
 {

     uint32 cmd = 0;
     uint32 field_val = 0;


     /*NOT USED*/
     field_val = IPEDISCARDTYPE_RESERVED0;
     cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_pbbCheckFailDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_pbbOamDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_pbbCheckDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_pbbDecapDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeMplsCtl_t, IpeMplsCtl_tmplsOamDiscardType_f); /*TMPLS OAM (14 label)*/
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     /*MAC SECURITY (DOT1AE)*/
     field_val = IPEDISCARDTYPE_DOT1AE_CHK;
     cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_unencryptedPktReceivedDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_dot1AeUnknownSciDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_dot1AeUnknownPktDiscard_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     /**< [TM] DOT1AE discard type */
     cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_dot1AePktRxOnDisabledPort_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     /**< [TM] OAM discard type  */
     field_val = IPEDISCARDTYPE_OAM_FILTER_DIS;
     cmd = DRV_IOW(IpeLookupCtl_t, IpeLookupCtl_sBfdReflectorSrcPortCheckDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     /*ETH OAM CHK (LinkOam/ServiceOam vlan check)*/
     field_val = IPEDISCARDTYPE_OAM_ETH_VLAN_CHK;
     cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_ethLinkOamDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_ethServOamDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     /*BFD TLL check */
     field_val = IPEDISCARDTYPE_OAM_BFD_TTL_CHK;
     cmd = DRV_IOW(IpeLookupCtl_t, IpeLookupCtl_mplsBfdTtlCheckDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeOamCtl_t, IpeOamCtl_ipBfdTtlCheckDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = IPEDISCARDTYPE_OAM_FILTER_DIS;
     cmd = DRV_IOW(IpeOamCtl_t, IpeOamCtl_oamFilterDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = IPEDISCARDTYPE_OAM_NOT_FOUND;
     cmd = DRV_IOW(IpeOamCtl_t, IpeOamCtl_oamNotFindDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeOamCtl_t, IpeOamCtl_noMepMipDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = IPEDISCARDTYPE_OAM_DISABLE;
     cmd = DRV_IOW(IpeMplsCtl_t, IpeMplsCtl_oamDisableDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     /*TRILL check*/
     field_val = IPEDISCARDTYPE_TRILL_CHK;
     cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_trillFilterDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeLookupCtl_t, IpeLookupCtl_trillInnverVlanCheckDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_trillOamDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_trillRpfCheckDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_trillVersionBfdCheckDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTrillCtl_t, IpeTrillCtl_trillVersionErrorDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_trillEsadiLoopbackDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTrillCtl_t, IpeTrillCtl_mcastAddressCheckDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_trillNonPortMacDiscard_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));


     /*WLAN check*/
     field_val = IPEDISCARDTYPE_WLAN_CHK;
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_capwapCipherStatusMisDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_capwapDtlsControlPacketDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_capwapKeepAlivePacketDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_dot11MgrCtlPktDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTunnelIdCtl_t, IpeTunnelIdCtl_capwapCanNotReassembleDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_capwapPacketErrorDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTunnelIdCtl_t, IpeTunnelIdCtl_capwapTunnelErrorDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_capwapStaStatusErrorDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_unknownDot11PacketDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_unknownCapwapPacketDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     /**< [TM] START */
     cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_unknownCapwapPacketDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     /**< [TM] END */
     cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_capwapRoamingFwdErrorDiscard_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     cmd = DRV_IOW(IpeTunnelIdCtl_t, IpeTunnelIdCtl_capwapFragPacketDiscard_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = IPEDISCARDTYPE_TUNNEL_ECN_DIS;
     cmd = DRV_IOW(IpeLookupCtl_t, IpeLookupCtl_tunnelEcnMisDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = IPEDISCARDTYPE_EFM_DIS;
     cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_efmDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = IPEDISCARDTYPE_ILOOP_DIS;
     cmd = DRV_IOW(IpeLoopbackHeaderAdjustCtl_t, IpeLoopbackHeaderAdjustCtl_bufRetrvDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = IPEDISCARDTYPE_MPLS_ENTROPY_LABEL_CHK;
     cmd = DRV_IOW(IpeMplsCtl_t, IpeMplsCtl_entropyIsReservedLabelDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = IPEDISCARDTYPE_MPLS_TP_MCC_SCC_DIS;
     cmd = DRV_IOW(IpeMplsCtl_t, IpeMplsCtl_mplstpMccSccDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = IPEDISCARDTYPE_MPLS_MC_PKT_ERROR;
     cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_mplsMcPacketErrorDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = IPEDISCARDTYPE_L2_EXCPTION_DIS;
     cmd = DRV_IOW(IpeRouteCtl_t, IpeRouteCtl_l2ExceptionDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = IPEDISCARDTYPE_NAT_PT_CHK;
     cmd = DRV_IOW(IpeRouteCtl_t, IpeRouteCtl_ptErrorDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = IPEDISCARDTYPE_DS_ACL_DIS;
     cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_aclDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = IPEDISCARDTYPE_DS_PLC_DIS;
     cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_plcDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = IPEDISCARDTYPE_SD_CHECK_DIS;
     cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_sdChkDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = IPEDISCARDTYPE_CFLAX_SRC_ISOLATE_DIS;
     cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_cFlexSrcIsolateDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = MCHIP_CAP(SYS_CAP_EPEDISCARDTYPE_DS_ACL_DIS);
     cmd = DRV_IOW(EpeClassficationCtl_t, EpeClassficationCtl_aclDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

     field_val = MCHIP_CAP(SYS_CAP_EPEDISCARDTYPE_DS_PLC_DIS);
     cmd = DRV_IOW(EpeClassficationCtl_t, EpeClassficationCtl_plcDiscardType_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));


     return CTC_E_NONE;
 }



/* mode 0: txThrd0=3, txThrd1=4; mode 1: txThrd0=5, txThrd1=11; if cut through enable, default use mode 1 */
int32 sys_usw_net_tx_threshold_cfg(uint8 lchip, uint16 mode)
{
#if 0
    uint32 cmd = 0;
    NetTxTxThrdCfg0_m net_tx_cfg;

    if(1 == mode)
    {
        cmd = DRV_IOR(NetTxTxThrdCfg0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_tx_cfg));
        SetNetTxTxThrdCfg0(V, txThrd0_f, &net_tx_cfg, 5);
        SetNetTxTxThrdCfg0(V, txThrd1_f, &net_tx_cfg, 11);
        cmd = DRV_IOW(NetTxTxThrdCfg0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_tx_cfg));

        cmd = DRV_IOR(NetTxTxThrdCfg1_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_tx_cfg));
        SetNetTxTxThrdCfg0(V, txThrd0_f, &net_tx_cfg, 5);
        SetNetTxTxThrdCfg0(V, txThrd1_f, &net_tx_cfg, 11);
        cmd = DRV_IOW(NetTxTxThrdCfg1_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_tx_cfg));
    }
    else
    {
        cmd = DRV_IOR(NetTxTxThrdCfg0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_tx_cfg));
        SetNetTxTxThrdCfg0(V, txThrd0_f, &net_tx_cfg, 3);
        SetNetTxTxThrdCfg0(V, txThrd1_f, &net_tx_cfg, 4);
        cmd = DRV_IOW(NetTxTxThrdCfg0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_tx_cfg));

        cmd = DRV_IOR(NetTxTxThrdCfg1_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_tx_cfg));
        SetNetTxTxThrdCfg0(V, txThrd0_f, &net_tx_cfg, 3);
        SetNetTxTxThrdCfg0(V, txThrd1_f, &net_tx_cfg, 4);
        cmd = DRV_IOW(NetTxTxThrdCfg1_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_tx_cfg));
    }
#endif
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_lkp_ctl_init(uint8 lchip)
{

    CTC_ERROR_RETURN(sys_usw_ftm_lkp_register_init(lchip));

    return CTC_E_NONE;
}

int32
sys_usw_lkup_ttl_index(uint8 lchip, uint8 ttl, uint32* ttl_index)
{
    uint32 cmd;
    uint8 index = 0;
    uint32 mpls_ttl = 0;
    uint8 bfind = 0;
    SYS_REGISTER_INIT_CHECK;

    for (index = 0; index < 16; index++)
    {
        cmd = DRV_IOR(EpeL3EditMplsTtl_t, EpeL3EditMplsTtl_array_0_ttl_f +  index);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mpls_ttl));

        if (mpls_ttl >= ttl)
        {
            *ttl_index = index;
            bfind = 1;
            break;
        }
    }

    if (bfind)
    {
        return CTC_E_NONE;
    }

    return CTC_E_INVALID_PARAM;
}

int32
_sys_usw_misc_ctl_init(uint8 lchip)
{
    uint32 cmd = 0;
    ds_t ds;
    uint32 max_index = 0;
    uint8 lgchip = 0;
    uint32 field_val = 0;
    uint16 i = 0;
    uint32 truncate_en[2] = {0};
    NetTxTruncateEnableCtl_m net_tx_truncate_en_ctl;
    IpeFwdCtl_m ipe_fwd_ctl;
    IpeAclQosCtl_m ipe_acl_qos_ctl;

    sys_usw_get_gchip_id(lchip, &lgchip);

    field_val = 0x11;
    cmd = DRV_IOW(NetTxMiscCtl_t, NetTxMiscCtl_cfgMinPktLen0_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);

    cmd = DRV_IOW(EpeScheduleMiscCtl_t, EpeScheduleMiscCtl_toMiscMinPktLen_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);

    cmd = DRV_IOW(EpeScheduleMiscCtl_t, EpeScheduleMiscCtl_toNetMinPktLen_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(IpeHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetIpeHeaderAdjustCtl(V, packetHeaderBypassAll_f, ds, 0);
    SetIpeHeaderAdjustCtl(V, gemPortBitType_f, ds, 0);
    cmd = DRV_IOW(IpeHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(IpeUserIdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetIpeUserIdCtl(V, parserLengthErrorMode_f, ds, 0);
        /* SYSTEM MODIFIED for tap */
#ifdef TAPPRODUCT
    SetIpeUserIdCtl(V, parserLengthErrorMode_f, ds, 2);
#endif
    SetIpeUserIdCtl(V, layer2ProtocolTunnelLmDisable_f, ds, 1);
    SetIpeUserIdCtl(V, useBypassIngressEdit_f, ds, 1);
    /**< [TM] Hash match default entry while not match ip da */
    SetIpeUserIdCtl(V, ipdaIndexMismatchHashType0Mode_f, ds, 1);
    SetIpeUserIdCtl(V, ipdaIndexMismatchHashType1Mode_f, ds, 1);
    /*SetIpeUserIdCtl(V, tcamLookup2NotImpactPopDefaultEntry_f, ds, 1);
    SetIpeUserIdCtl(V, tcamLookup3ForRouterMac_f, ds, 1);*/

    /**< [TM] Skip IP Binding Check for L3 ARP Packet */
    SetIpeUserIdCtl(V, arpPktSkipIpBindingCheck_f, ds, 0);
    /**< [TM] Wlan Module: Whether do hash lookup for capwap packet */
    SetIpeUserIdCtl(V, splitMacTunnel1LookupEn_f, ds, 1);
    /**< [TM] "0x01"=UserId{2,0,1,3}        "0x02"=UserId{2,3,0,1}
                   "0x03"=UserId{0,2,1,3}        "0x04"=UserId{2,0,3,1}
                   "0x00"=UserId{0,1,2,3}        keep compatible with D2       */
    SetIpeUserIdCtl(V, userIdResultPriorityMode_f, ds, 0x00);

    /*
    randomLogShift is 5, so random log percent 0~100 % and step is 0.0031%
    */
    SetIpeUserIdCtl(V, randomLogShift_f, ds, 5);
    cmd = DRV_IOW(IpeUserIdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

	sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(IpeSclFlowCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetIpeSclFlowCtl(V, followGreProtocol_f, ds, 1);
    cmd = DRV_IOW(IpeSclFlowCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetIpeIntfMapperCtl(V, allowMcastMacSa_f,                ds, 0);
/* SYSTEM MODIFIED by hansf 2018/11/05: for physical routed port, hybrid set arp broadcast copy to cpu */
#ifdef OFPRODUCT
    SetIpeIntfMapperCtl(V, arpBroadcastRoutedPortDiscard_f,  ds, 0);
#else
    SetIpeIntfMapperCtl(V, arpBroadcastRoutedPortDiscard_f,  ds, 1);
#endif
    SetIpeIntfMapperCtl(V, arpCheckExceptionEn_f,            ds, 0);
    SetIpeIntfMapperCtl(V, arpDestMacCheckEn_f,              ds, 0);
    SetIpeIntfMapperCtl(V, arpExceptionSubIndex_f,           ds, CTC_L3PDU_ACTION_INDEX_ARP);
    SetIpeIntfMapperCtl(V, arpIpCheckEn_f,                   ds, 0);
    SetIpeIntfMapperCtl(V, arpSrcMacCheckEn_f,               ds, 0);
    SetIpeIntfMapperCtl(V, arpUnicastDiscard_f,              ds, 0);
    SetIpeIntfMapperCtl(V, arpUnicastExceptionDisable_f,     ds, 1);
    SetIpeIntfMapperCtl(V, bypassAllDisableLearning_f,       ds, 0);
    SetIpeIntfMapperCtl(V, bypassPortCrossConnectDisable_f,  ds, 0);
    SetIpeIntfMapperCtl(V, dhcpBroadcastRoutedPortDiscard_f, ds, 0);
    SetIpeIntfMapperCtl(V, dhcpExceptionSubIndex_f,          ds, CTC_L3PDU_ACTION_INDEX_DHCP);
    SetIpeIntfMapperCtl(V, dhcpUnicastDiscard_f,             ds, 0);
    SetIpeIntfMapperCtl(V, dhcpUnicastExceptionDisable_f,    ds, 1);
    SetIpeIntfMapperCtl(V, discardSameIpAddr_f,              ds, 0);
    SetIpeIntfMapperCtl(V, discardSameMacAddr_f,             ds, 1);
    SetIpeIntfMapperCtl(V, fipExceptionSubIndex_f,          ds, CTC_L2PDU_ACTION_INDEX_FIP);
    SetIpeIntfMapperCtl(V, addDefaultSvlanUpdateTpid_f,          ds, 1);
    SetIpeIntfMapperCtl(V, learningDisableMode_f,          ds, 1);
    cmd = DRV_IOW(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(IpeMplsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetIpeMplsCtl(V, minInterfaceLabel_f, ds, 0);
    SetIpeMplsCtl(V, oamAlertLabel0_f, ds, 14);
    SetIpeMplsCtl(V, oamAlertLabel1_f, ds, 13);
    SetIpeMplsCtl(V, mplsOffsetBytesShift_f, ds, 2);
    SetIpeMplsCtl(V, useFirstLabelTtl_f, ds, 0);
    SetIpeMplsCtl(V, galSbitCheckEn_f, ds, 1);
    SetIpeMplsCtl(V, galTtlCheckEn_f, ds, 1);
    SetIpeMplsCtl(V, entropyLabelTtlCheckEn_f, ds, 1);
    SetIpeMplsCtl(V, mplsUseBypassIngressEdit_f, ds, 1);
    SetIpeMplsCtl(V, allAchAsOamEn_f, ds, 0);
    SetIpeMplsCtl(V, aclQosUseOuterInfoMode_f, ds, 0);
    SetIpeMplsCtl(V, metaDataEnForAll_f, ds, 0);
    SetIpeMplsCtl(V, trustMplsTcForce_f, ds, 0);
    cmd = DRV_IOW(IpeMplsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(IpeTunnelIdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetIpeTunnelIdCtl(V, routeObeyStp_f, ds, 1);
    SetIpeTunnelIdCtl(V, tunnelforceSecondParserEn_f, ds, 0x3ff);
    cmd = DRV_IOW(IpeTunnelIdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(IpeLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetIpeLookupCtl(V, globalVrfIdLookupEn_f, ds, 1);
    SetIpeLookupCtl(V, pimSnoopingEn_f, ds, 1);
    SetIpeLookupCtl(V, stpBlockBridgeDisable_f, ds, 0);
    SetIpeLookupCtl(V, noIpMcastMacLookup_f, ds, 0);
    SetIpeLookupCtl(V, stpBlockLayer3_f, ds, 0);
    SetIpeLookupCtl(V, trillMbitCheckDisable_f, ds, 0);
    SetIpeLookupCtl(V, routedPortDisableBcastBridge_f, ds, 0);
    SetIpeLookupCtl(V, greFlexProtocol_f, ds, 0);
    SetIpeLookupCtl(V, greFlexPayloadPacketType_f, ds, 0);
    SetIpeLookupCtl(V, trillInnerVlanCheck_f, ds, 1);
    SetIpeLookupCtl(V, trillVersion_f, ds, 0);
    SetIpeLookupCtl(V, trillUseInnerVlan_f, ds, 1);
    SetIpeLookupCtl(V, metadataSendFid_f, ds, 1);
    SetIpeLookupCtl(V, upMepMacDaLkupMode_f, ds, 1);
    SetIpeLookupCtl(V, ecnAware_f, ds, 6); /*only TCP and UDP ECN Aware*/
    SetIpeLookupCtl(V, arpExceptionSubIndex_f, ds, CTC_L3PDU_ACTION_INDEX_ARP);

	SetIpeLookupCtl(V, flowHashPrBusGlobalMode_f, ds, 0);
    SetIpeLookupCtl(V, hashFieldSrcLocalPortMode_f, ds, 1);
    SetIpeLookupCtl(V, macSaIsAllZeroDisableMacSaLookup_f, ds, 0);

    cmd = DRV_IOW(IpeLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    cmd = DRV_IOR(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl));
    SetIpeFwdCtl(V, ecnPriorityCritical_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, chipId_f, &ipe_fwd_ctl, lgchip);
    SetIpeFwdCtl(V, discardFatal_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, headerHashMode_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, linkOamColor_f, &ipe_fwd_ctl, 0);
    /*SYSTEM MODIFIED by chenc for bug 52789, 2019-08-01*/
    SetIpeFwdCtl(V, linkOamPrio_f, &ipe_fwd_ctl, 12);
    /*chenc end*/
    SetIpeFwdCtl(V, logOnDiscard_f, &ipe_fwd_ctl, 0x3FFFF);
    SetIpeFwdCtl(V, rxEtherOamCritical_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, portExtenderMcastEn_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, forceExceptLocalPhyPort_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, cutThroughCareChannelState_f, &ipe_fwd_ctl, 1);
    SetIpeFwdCtl(V, cutThroughDisableChannel_f, &ipe_fwd_ctl, 64);
    SetIpeFwdCtl(V, elephantFlowColor_f, &ipe_fwd_ctl, CTC_QOS_COLOR_GREEN);
    SetIpeFwdCtl(V, oamObeyAclDiscard_f, &ipe_fwd_ctl, 1);
    SetIpeFwdCtl(V, aclRedirectForceMacKnown_f, &ipe_fwd_ctl, 1);
    SetIpeFwdCtl(V, iloopBheaderSourePortMode_f, &ipe_fwd_ctl, 1);
    cmd = DRV_IOW(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl));
    /**< [TM] START */
    cmd = DRV_IOR(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_acl_qos_ctl));
    SetIpeAclQosCtl(V, logOnDiscard_f, &ipe_acl_qos_ctl, 0x3FFFF);
    SetIpeAclQosCtl(V, discardFatal_f, &ipe_acl_qos_ctl, 0);
    SetIpeAclQosCtl(V, oamBypassPolicingOp_f, &ipe_acl_qos_ctl, 1);
    SetIpeAclQosCtl(V, oamBypassStormCtlOp_f, &ipe_acl_qos_ctl, 1);
    SetIpeAclQosCtl(V, fromCpuOrOamBypassPolicing_f, &ipe_acl_qos_ctl, 1);
    SetIpeAclQosCtl(V, fromCpuOrOamBypassStorm_f, &ipe_acl_qos_ctl, 1);
    SetIpeAclQosCtl(V, fatalExceptionForceMacKnown_f, &ipe_acl_qos_ctl, 1);
    SetIpeAclQosCtl(V, forceBackClearSrcCidValidEn_f, &ipe_acl_qos_ctl, 1);
    SetIpeAclQosCtl(V, vplsDecapsPktForwardingTypeMode_f, &ipe_acl_qos_ctl, 1);
    cmd = DRV_IOW(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_acl_qos_ctl));

    field_val = 1;
    cmd = DRV_IOW(EpeClassficationCtl_t, EpeClassficationCtl_fromCpuOrOamBypassPolicing_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(IpeUserIdFlowCtl_t, IpeUserIdFlowCtl_seedSelectEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    /**< [TM] END */
    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(EpeHdrAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetEpeHdrAdjustCtl(V, mplsOfModeAutoDetectIp_f, ds, 1);
    SetEpeHdrAdjustCtl(V, portExtenderMcastEn_f, ds, 0);
    if (!DRV_IS_DUET2(lchip))
    {
         SetEpeHdrAdjustCtl(V, mplsSbfdEn_f, ds, 0); /*spec bug 107664, need set 0 to support TP-Y1731*/
        /*SYSTEM MODIFIED, Added by yangl for TP-Y1731, 2019-09-03*/
        //SetEpeHdrAdjustCtl(V, mplsSbfdEn_f, ds, 1);
    }
    SetEpeHdrAdjustCtl(V, mplsOfModeEn_f, ds, 1);
    /*SYSTEM MODIFIED, Added by hansf for qos, 2018-09-25*/
    SetEpeHdrAdjustCtl(V, ingressAclPhbLowPriority_f, ds, 1);
    cmd = DRV_IOW(EpeHdrAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(IpeTunnelDecapCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetIpeTunnelDecapCtl(V, greFlexPayloadPacketType_f, ds, 0);
    SetIpeTunnelDecapCtl(V, greFlexProtocol_f, ds, 0x0800);
    SetIpeTunnelDecapCtl(V, greOption2CheckEn_f, ds, 1);
    SetIpeTunnelDecapCtl(V, ipTtlLimit_f, ds, 1);
    SetIpeTunnelDecapCtl(V, offsetByteShift_f, ds, (DRV_IS_DUET2(lchip)) ? 1 : 0);
    SetIpeTunnelDecapCtl(V, tunnelMartianAddressCheckEn_f, ds, 1);
    SetIpeTunnelDecapCtl(V, mcastAddressMatchCheckDisable_f, ds, 0);
    SetIpeTunnelDecapCtl(V, trillTtl_f, ds, 1);
    SetIpeTunnelDecapCtl(V, trillversionCheckEn_f, ds, 1);
    SetIpeTunnelDecapCtl(V, trillVersion_f, ds, 0);
    SetIpeTunnelDecapCtl(V, trillBfdCheckEn_f, ds, 1);
    SetIpeTunnelDecapCtl(V, trillBfdHopCount_f, ds, 0x30);
    SetIpeTunnelDecapCtl(V, ipOptionTunnelFatalException_f, ds, 1);
    SetIpeTunnelDecapCtl(V, greDecapSkipOptionValid_f, ds, 0xF);
    SetIpeTunnelDecapCtl(V, mplsSnoopMode_f, ds, 0);
    cmd = DRV_IOW(IpeTunnelDecapCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(IpeLearningCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetIpeLearningCtl(V, vsiSecurityExceptionEn_f, ds, 1);
    SetIpeLearningCtl(V, pendingEntrySecurityDisable_f, ds, 0);
    SetIpeLearningCtl(V, srcMismatchDiscardException_f, ds, 1);
    SetIpeLearningCtl(V, aclCareMacSecurityEn_f, ds, 0x1);      /*only enable black hole mac discard*/
    cmd = DRV_IOW(IpeLearningCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(IpeTrillCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetIpeTrillCtl(V, versionCheckEn_f, ds, 1);
    SetIpeTrillCtl(V, trillBypassDenyRoute_f, ds, 0);
    cmd = DRV_IOW(IpeTrillCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(MetFifoCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetMetFifoCtl(V, chipId_f, ds, lgchip);
    SetMetFifoCtl(V, portCheckEn_f, ds, 1);
    SetMetFifoCtl(V, discardMetLoop_f, ds, 1);
    SetMetFifoCtl(V, portBitmapNextHopPtr_f, ds, 1);
    SetMetFifoCtl(V, portBitmapLagMode_f, ds, 0);
    cmd = DRV_IOW(MetFifoCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(EpeL2EtherType_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetEpeL2EtherType(V, array_0_etherType_f, ds, 0);
    SetEpeL2EtherType(V, array_1_etherType_f, ds, 0);
    cmd = DRV_IOW(EpeL2EtherType_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    /*Temple disable hyrid port*/
    /*SYSTEM MODIFIED by yejl for tsingma tap, 2019-08-06*/
  #ifdef TAPPRODUCT
    SetEpeNextHopCtl(V, tagPortBitMapEn_f, ds, 0);
  #else
    SetEpeNextHopCtl(V, tagPortBitMapEn_f, ds, 1);
  #endif
    /*ended*/
    SetEpeNextHopCtl(V, discardReflectiveBridge_f, ds, 1);
    SetEpeNextHopCtl(V, forceBridgeL3Match_f, ds, 1);
    SetEpeNextHopCtl(V, discardLogicTunnelMatch_f, ds, 1);
    SetEpeNextHopCtl(V, routeObeyIsolate_f, ds, 0);
    SetEpeNextHopCtl(V, mirrorObeyDiscard_f, ds, 0);
    SetEpeNextHopCtl(V, denyDuplicateMirror_f, ds, 1);
    SetEpeNextHopCtl(V, parserErrorIgnore_f, ds, 0);
    SetEpeNextHopCtl(V, discardSameIpAddr_f, ds, 0);
    SetEpeNextHopCtl(V, discardSameMacAddr_f, ds, 1);
    SetEpeNextHopCtl(V, cbpTciRes2CheckEn_f, ds, 0);
    SetEpeNextHopCtl(V, pbbBsiOamOnEgsCbpEn_f, ds, 0);
    SetEpeNextHopCtl(V, terminatePbbBvOamOnEgsPip_f, ds, 0);
    SetEpeNextHopCtl(V, pbbBvOamOnEgsPipEn_f, ds, 0);
    SetEpeNextHopCtl(V, routeObeyStp_f, ds, 1);
    SetEpeNextHopCtl(V, parserLengthErrorMode_f, ds, 3);
    SetEpeNextHopCtl(V, oamIgnorePayloadOperation_f, ds, 1);
    SetEpeNextHopCtl(V, etherOamUsePayloadOperation_f, ds, 1);
    SetEpeNextHopCtl(V, bridgeOpMapping_f, ds, 0x1c);
    SetEpeNextHopCtl(V, bypassDisableInsertCidTagBitmap_f, ds, 0x2);
    /*SYSTEM Modify by yangbs for sdk bug 102807, 2017-12-05*/
    SetEpeNextHopCtl(V, spanPktDestVlanStats_f, ds, 1);
    SetEpeNextHopCtl(V, oamSdChkEn_f, ds, 0x1);
    /*
    randomLogShift is 5, so random log percent 0~100 % and step is 0.0031%
    */
    SetEpeNextHopCtl(V, randomLogShift_f, ds, 5);
    SetEpeNextHopCtl(V, seedSelectEn_f, ds, 1);
    SetEpeNextHopCtl(V, useGlbDstPortDefaultVid_f, ds, 1);
    /*SYSTEM MODIFIED by hansf for qos , 2018-09-25*/
    SetEpeNextHopCtl(V, dscpRemarkModeFromPortFirst_f, ds, 1);

	cmd = DRV_IOW(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(EpeHeaderEditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetEpeHeaderEditCtl(V, logOnDiscard_f, ds, 0x7F);
    SetEpeHeaderEditCtl(V, rxEtherOamCritical_f, ds, 0);
    SetEpeHeaderEditCtl(V, stackingEn_f, ds, 0);
    SetEpeHeaderEditCtl(V, evbTpid_f, ds, 0);
    SetEpeHeaderEditCtl(V, muxLengthTypeEn_f, ds, 1);
    SetEpeHeaderEditCtl(V, loopbackUseSourcePort_f, ds, 0);
    SetEpeHeaderEditCtl(V, srcVlanInfoEn_f, ds, 1);
    SetEpeHeaderEditCtl(V, chipId_f, ds, lgchip);

    SetEpeHeaderEditCtl(V, oamDestId_f, ds, SYS_RSV_PORT_OAM_CPU_ID);

    SetEpeHeaderEditCtl(V, cidHdrEtherType_f, ds, 0x8909);
    SetEpeHeaderEditCtl(V, cidHdrVersion_f, ds, 1);
    SetEpeHeaderEditCtl(V, cidHdrLength_f, ds, 1);
    SetEpeHeaderEditCtl(V, cidOptionLen_f, ds, 0);
    SetEpeHeaderEditCtl(V, cidOptionType_f, ds, 1);
    SetEpeHeaderEditCtl(V, skipSpanPacketLoop_f, ds, 1);
    /*SYSTEM MODIFIED by yangl for bug 54029*/
    SetEpeHeaderEditCtl(V, discardPacketBypassEdit_f, ds, 0);
    #if 0
    SetEpeHeaderEditCtl(V, discardPacketBypassEdit_f, ds, 1);
    #endif
    /*yangl end*/
    SetEpeHeaderEditCtl(V, discardPacketLengthRecover_f, ds, 1);

    SetEpeHeaderEditCtl(V, forceHeaderCarryPortMode_f, ds, 0);
    SetEpeHeaderEditCtl(V, loopbackPktPermitIpfix_f, ds, 0);
    SetEpeHeaderEditCtl(V, e2iLoopPktPermitIpfix_f, ds, 0);
    SetEpeHeaderEditCtl(V, packetHeaderEnPktPermitIpfix_f, ds, 0);
    SetEpeHeaderEditCtl(V, loopbackPktPermitMfp_f, ds, 0);
    SetEpeHeaderEditCtl(V, e2iLoopPktPermitMfp_f, ds, 0);
    SetEpeHeaderEditCtl(V, packetHeaderEnPktPermitMfp_f, ds, 0);

    cmd = DRV_IOW(EpeHeaderEditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(EpePktProcCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetEpePktProcCtl(V, discardRouteTtl0_f, ds, 1);
    SetEpePktProcCtl(V, discardMplsTtl0_f, ds, 1);
    SetEpePktProcCtl(V, discardMplsTagTtl0_f, ds, 1);
    SetEpePktProcCtl(V, discardTunnelTtl0_f, ds, 1);
    SetEpePktProcCtl(V, discardTrillTtl0_f, ds, 1);
    SetEpePktProcCtl(V, useGlobalCtagCos_f, ds, 0);
    SetEpePktProcCtl(V, mirrorEscapeCamEn_f, ds, 0);
    SetEpePktProcCtl(V, mplsDeriveLabelDisable_f, ds, 1);
    SetEpePktProcCtl(V, icmpErrMsgCheckEn_f, ds, 1);
    SetEpePktProcCtl(V, globalCtagCos_f, ds, 0);
    SetEpePktProcCtl(V, ptMulticastAddressEn_f, ds, 0);
    SetEpePktProcCtl(V, ptUdpChecksumZeroDiscard_f, ds, 1);
    SetEpePktProcCtl(V, nextHopStagCtlEn_f, ds, 1);
    SetEpePktProcCtl(V, mplsDeriveLabelDisable_f, ds, 1);
    /*-TBD  SetEpePktProcCtl(V, mplsStatsMode_f, ds, 2);*/
    SetEpePktProcCtl(V, ipIdIncreaseType_f, ds, 0);
    SetEpePktProcCtl(V, ecnIgnoreCheck_f, ds, 0); /*ECN mark enable*/
    SetEpePktProcCtl(V, ucastTtlFailExceptionEn_f, ds, 1);
    SetEpePktProcCtl(V, mcastTtlFailExceptionEn_f, ds, 1);
    SetEpePktProcCtl(V, oamTtlOne_f, ds, 1);
    SetEpePktProcCtl(V, pwPipeTtlObeyOam_f, ds, 1);
    /*SYSTEM MODIFIED by yangl for tpoam , 2019-09-20*/
    SetEpePktProcCtl(V, lspPipeTtlObeyOam_f, ds, 0);
    SetEpePktProcCtl(V, spmePipeTtlObeyOam_f, ds, 1);
    SetEpePktProcCtl(V, oamUseLspMapTtlMode_f, ds, 0);


    SetEpePktProcCtl(V, ipv4FlexTunnelUpdateL3Type_f, ds, 1);
    SetEpePktProcCtl(V, ipv6FlexTunnelUpdateL3Type_f, ds, 1);
    SetEpePktProcCtl(V, ipv4FlexTunnelUpdateL4Type_f, ds, 1);
    SetEpePktProcCtl(V, ipv4FlexTunnelUpdateL4TypeValue_f, ds, 0);
    SetEpePktProcCtl(V, ipv6FlexTunnelUpdateL4Type_f, ds, 1);
    SetEpePktProcCtl(V, ipv6FlexTunnelUpdateL4TypeValue_f, ds, 0);

    SetEpePktProcCtl(V, prUdfUseFlexHdrUpdateUdfIndex_f, ds, 0);



    cmd = DRV_IOW(EpePktProcCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(DsEgressPortMac_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetDsEgressPortMac(V, portMac_f, ds, 0);
    cmd = DRV_IOW(DsEgressPortMac_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
 /*TBD*/
#if 0
    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(EpeClassificationCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetEpeClassificationCtl(V, portPolicerBase_f, ds, 0);
    cmd = DRV_IOW(EpeClassificationCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
#endif
    field_val = 48;
    cmd = DRV_IOW(EpeGlobalChannelMap_t, EpeGlobalChannelMap_maxNetworkPortChannel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetEpeAclQosCtl(V, oamObeyAclDiscard_f, ds, 0x1);
    cmd = DRV_IOW(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(FibEngineLookupResultCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetFibEngineLookupResultCtl(V, gMacSaLookupResultCtl_defaultEntryEn_f, ds, 1);
    SetFibEngineLookupResultCtl(V, gMacSaLookupResultCtl_defaultEntryBase_f, ds, 0);
    SetFibEngineLookupResultCtl(V, gMacDaLookupResultCtl_defaultEntryEn_f, ds, 1);
    SetFibEngineLookupResultCtl(V, gMacDaLookupResultCtl_defaultEntryBase_f, ds, 0);
    SetFibEngineLookupResultCtl(V, gIpv4UcastLookupResultCtl_defaultEntryEn_f, ds, 1);
    SetFibEngineLookupResultCtl(V, gIpv4UcastLookupResultCtl_defaultEntryBase_f, ds, 12);
    SetFibEngineLookupResultCtl(V, gIpv4McastLookupResultCtl_defaultEntryEn_f, ds, 1);
    SetFibEngineLookupResultCtl(V, gIpv4McastLookupResultCtl_defaultEntryBase_f, ds, 13);
    SetFibEngineLookupResultCtl(V, gIpv6UcastLookupResultCtl_defaultEntryEn_f, ds, 1);
    SetFibEngineLookupResultCtl(V, gIpv6UcastLookupResultCtl_defaultEntryBase_f, ds, 14);
    SetFibEngineLookupResultCtl(V, gIpv6McastLookupResultCtl_defaultEntryEn_f, ds, 1);
    SetFibEngineLookupResultCtl(V, gIpv6McastLookupResultCtl_defaultEntryBase_f, ds, 15);
    SetFibEngineLookupResultCtl(V, gIpv4RpfLookupResultCtl_defaultEntryEn_f, ds, 1);
    SetFibEngineLookupResultCtl(V, gIpv4RpfLookupResultCtl_defaultEntryBase_f, ds, 12);
    SetFibEngineLookupResultCtl(V, gIpv6RpfLookupResultCtl_defaultEntryEn_f, ds, 1);
    SetFibEngineLookupResultCtl(V, gIpv6RpfLookupResultCtl_defaultEntryBase_f, ds, 14);
    SetFibEngineLookupResultCtl(V, gFcoeDaLookupResultCtl_defaultEntryEn_f, ds, 1);
    SetFibEngineLookupResultCtl(V, gFcoeDaLookupResultCtl_defaultEntryPtr_f, ds, 1536);
    SetFibEngineLookupResultCtl(V, gFcoeSaLookupResultCtl_defaultEntryEn_f, ds, 1);
    SetFibEngineLookupResultCtl(V, gFcoeSaLookupResultCtl_defaultEntryPtr_f, ds, 1600);
    SetFibEngineLookupResultCtl(V, gTrillUcastLookupResultCtl_defaultEntryEn_f, ds, 1);
    SetFibEngineLookupResultCtl(V, gTrillUcastLookupResultCtl_defaultEntryPtr_f, ds, 1792);
    SetFibEngineLookupResultCtl(V, gTrillMcastLookupResultCtl_defaultEntryEn_f, ds, 1);
    SetFibEngineLookupResultCtl(V, gTrillMcastLookupResultCtl_defaultEntryPtr_f, ds, 1856);
    cmd = DRV_IOW(FibEngineLookupResultCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(BufferStoreCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetBufferStoreCtl(V, chipId_f, ds, lgchip);
    SetBufferStoreCtl(V, mcastMetFifoEnable_f, ds, 1);
    SetBufferStoreCtl(V, cpuRxExceptionEn0_f, ds, 0);
    SetBufferStoreCtl(V, cpuRxExceptionEn1_f, ds, 0);
    SetBufferStoreCtl(V, cpuTxExceptionEn_f, ds, 0);

    SetBufferStoreCtl(V, e2iErrorDiscardCtl_f, ds, 0x7FFF);
    /*except errorCode:
      E2ILOOPERROR_DOT1AE_DECRYPT_CIPHER_MODE_MISMATCH:0xa -1
      E2ILOOPERROR_DOT1AE_DECRYPT_ICV_CHECK_ERROR:0x9 -1    */
    SetBufferStoreCtl(V, e2iErrorExceptionCtl_f, ds, 0x7CFF);

    SetBufferStoreCtl(V, mplsOfModeEnable_f, ds, 1);
    SetBufferStoreCtl(V, spanPktBypassSrcCheckDisable_f, ds, 0);

    cmd = DRV_IOW(BufferStoreCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(BufferRetrieveCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
     /*-TBD SetBufferRetrieveCtl(V, colorMapEn_f, ds, 0);*/
     /*-TBD SetBufferRetrieveCtl(V, chipId_f, ds, lgchip);*/
    cmd = DRV_IOW(BufferRetrieveCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(MetFifoCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetMetFifoCtl(V, chipId_f, ds, lgchip);
    SetMetFifoCtl(V, portCheckEn_f, ds, 1);
    SetMetFifoCtl(V, discardMetLoop_f, ds, 1);
    cmd = DRV_IOW(MetFifoCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(QWriteCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetQWriteCtl(V, chipId_f, ds, lgchip);
    SetQWriteCtl(V, loopbackFromFabricDisable_f, ds, 0);
    cmd = DRV_IOW(QWriteCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOR(LagEngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetLagEngineCtl(V, chipId_f, ds, lgchip);

    SetLagEngineCtl(V, chanOrCflexSlbHashValBasedOnIngrPortMode_f, ds, 0);
    SetLagEngineCtl(V, chanRrProtectDisable_f, ds, 0);
    SetLagEngineCtl(V, useLoopBackHashCfgEn_f, ds, 0);
    SetLagEngineCtl(V, hashOffsetForSlbNonUcPortLag_f, ds, 0);
    SetLagEngineCtl(V, hashOffsetNonUcCflexLag_f, ds, 0);
    SetLagEngineCtl(V, hashOffsetNonUcChanLag_f, ds, 0);
    SetLagEngineCtl(V, hashSelectForLoopback_f, ds, 0);
    SetLagEngineCtl(V, hashSelectForSlbNonUcPortLag_f, ds, 0);
    SetLagEngineCtl(V, hashOffsetForSlbNonUcPortLag_f, ds, 0);
    SetLagEngineCtl(V, nonUcCflexLagHashSelectGlobalCtlEn_f, ds, 0);
    SetLagEngineCtl(V, nonUcChanLagHashSelectGlobalCtlEn_f, ds, 0);
    SetLagEngineCtl(V, nonUcPortLagHashSelectGlobalCtlEn_f, ds, 0);
    SetLagEngineCtl(V, portLagIloopUseGlobalHashCfgEn_f, ds, 0);
    SetLagEngineCtl(V, portSlbHashValBasedOnIngrPortMode_f, ds, 0);
    SetLagEngineCtl(V, useLoopBackHashCfgEn_f, ds, 0);

    cmd = DRV_IOW(LagEngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    /*bug32120*/
    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOW(OamUpdateCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOW(DsBfdV6Addr_t, DRV_ENTRY_FLAG);
    sys_usw_ftm_query_table_entry_num(lchip, DsBfdV6Addr_t, &max_index);

    for (i = 0; i < max_index; i++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, ds));
    }

    sys_usw_ftm_query_table_entry_num(lchip, DsPortProperty_t, &max_index);
    sal_memset(ds, 0, sizeof(ds));
    cmd = DRV_IOW(DsPortProperty_t, DRV_ENTRY_FLAG);
    for (i = 0; i < max_index; i++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, ds));
    }

    /*Truncate Length cfg*/
    /*
    sal_memset(ds, 0, sizeof(ds));
    for(i = 1; i < 16; i++)
    {
        cmd = DRV_IOW(DsTruncationProfile_t, DRV_ENTRY_FLAG);
        SetDsTruncationProfile(V, lengthShift_f, ds, 3);
        SetDsTruncationProfile(V, length_f, ds, 8 * i);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, ds));
    }*/
    field_val = 2;   /*DMA unit 16bytes, so 16 << 2 = 64*/
    cmd = DRV_IOW(BufferRetrieveCtl_t, BufferRetrieveCtl_truncationLenShift_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    field_val = 1;
    cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_networkCpuTruncationMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    field_val = 1;
    cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_networkCpuAdjustTunnel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    field_val = 6;   /*1<<6 = 64*/
    cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_cpuTruncationShift_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    field_val = 1;
    cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_toCpuRecoverTruncation_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));


    if (DRV_IS_DUET2(lchip))
    {
        field_val = 1;
        cmd = DRV_IOW(DmaPktRxDropCfg_t, DmaPktRxDropCfg_cfgPktRxTruncEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }

    sal_memset(&net_tx_truncate_en_ctl, 0, sizeof(NetTxTruncateEnableCtl_m));
    sal_memset(truncate_en, 0xFF, 2*sizeof(uint32));
    SetNetTxTruncateEnableCtl(A, truncateEn_f, &net_tx_truncate_en_ctl, truncate_en);
    cmd = DRV_IOW(NetTxTruncateEnableCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_tx_truncate_en_ctl));

    /*Eco*/
    field_val = 3;
    cmd = DRV_IOW(IpeFwdReserved_t, IpeFwdReserved_reserved_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    /**< [TM] START */
    field_val = 1;
    cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_fromCpuOrOamBypassPolicing_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_fromCpuOrOamBypassStorm_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    /**< [TM] END */
    field_val = 1;
    cmd = DRV_IOW(EpeAclOamReserved_t, EpeAclOamReserved_reserved_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 1;
    cmd = DRV_IOW(IpePktProcReserved_t, IpePktProcReserved_reserved_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 1;
    cmd = DRV_IOW(ErmMiscCtl_t, ErmMiscCtl_scExcludeGuarantee_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = DRV_IS_DUET2(lchip) ? 1 : 0;
    cmd = DRV_IOW(IpeIntfMapReserved_t, IpeIntfMapReserved_reserved_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &field_val));

    field_val = 2;
    cmd = DRV_IOW(IpeCapwapFragCtl_t, IpeCapwapFragCtl_fragTableMgrMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /* cfg iloop use default index 7 to check pkt length*/
    if (DRV_IS_DUET2(lchip))
    {
        field_val = CTC_FRAME_SIZE_7;
        cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMinLenSelId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 11, cmd, &field_val));

        field_val = CTC_FRAME_SIZE_7;
        cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMaxLenSelId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 11, cmd, &field_val));
    }
    else
    {
        field_val = SYS_USW_ILOOP_MIN_FRAMESIZE_DEFAULT_VALUE;
        cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMinLen_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 11, cmd, &field_val));

        field_val = SYS_USW_MAX_FRAMESIZE_MAX_VALUE;
        cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMaxLen_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 11, cmd, &field_val));
    }

    field_val = 1;
    cmd = DRV_IOW(OamHeaderAdjustCtl_t, OamHeaderAdjustCtl_relayAllToCpu_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_ctl_init(uint8 lchip)
{
    uint32 cmd = 0;
    ds_t ds;
    uint32 value = 0;

    cmd = DRV_IOW(EpeL3EditMplsTtl_t, DRV_ENTRY_FLAG);
    sal_memset(ds, 0, sizeof(ds));

    SetEpeL3EditMplsTtl(V, array_0_ttl_f, ds, 0);
    SetEpeL3EditMplsTtl(V, array_1_ttl_f, ds, 1);
    SetEpeL3EditMplsTtl(V, array_2_ttl_f, ds, 2);
    SetEpeL3EditMplsTtl(V, array_3_ttl_f, ds, 8);
    SetEpeL3EditMplsTtl(V, array_4_ttl_f, ds, 15);
    SetEpeL3EditMplsTtl(V, array_5_ttl_f, ds, 16);
    SetEpeL3EditMplsTtl(V, array_6_ttl_f, ds, 31);
    SetEpeL3EditMplsTtl(V, array_7_ttl_f, ds, 32);
    SetEpeL3EditMplsTtl(V, array_8_ttl_f, ds, 60);
    SetEpeL3EditMplsTtl(V, array_9_ttl_f, ds, 63);
    SetEpeL3EditMplsTtl(V, array_10_ttl_f, ds, 64);
    SetEpeL3EditMplsTtl(V, array_11_ttl_f, ds, 65);
    SetEpeL3EditMplsTtl(V, array_12_ttl_f, ds, 127);
    SetEpeL3EditMplsTtl(V, array_13_ttl_f, ds, 128);
    SetEpeL3EditMplsTtl(V, array_14_ttl_f, ds, 254);
    SetEpeL3EditMplsTtl(V, array_15_ttl_f, ds, 255);

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    value = 1;
    cmd = DRV_IOW(IpeMplsTtlThrd_t, IpeMplsTtlThrd_array_0_ttlThreshold_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(BufferStoreCtl_t, BufferStoreCtl_mplsOfModeEnable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_mplsOfModeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_excp_ctl_init(uint8 lchip)
{
    /* POP DS */
    uint32 cmd = 0;
    uint32 excp_idx = 0;
    uint32 value = 0;
    uint8 gchip  = 0;
    ds_t ds;
    sys_usw_get_gchip_id(lchip, &gchip);
    for (excp_idx = 0; excp_idx < 320; excp_idx++)
    {
        sal_memset(ds, 0, sizeof(ds));
        SetDsMetFifoExcp(V, destMap_f, ds, SYS_ENCODE_DESTMAP(gchip, SYS_RSV_PORT_DROP_ID));
        cmd = DRV_IOW(DsMetFifoExcp_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_idx, cmd, ds));
    }

    value = 1;
    cmd = DRV_IOW(BufRetrvExcpCtl_t, BufRetrvExcpCtl_exceptionEditMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
	value = 0;
    cmd = DRV_IOW(BufRetrvExcpCtl_t, BufRetrvExcpCtl_cflexExcepRestOffsetSkipCfHdr_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}

#if 0
STATIC int32
_sys_usw_net_rx_tx_init(uint8 lchip)
{
    return CTC_E_NONE;
}
#endif

STATIC int32
_sys_usw_ipe_loopback_hdr_adjust_ctl_init(uint8 lchip)
{
    uint32 cmd = 0;
    ds_t ds_data;
    uint32 field_val = 0;

    sal_memset(&ds_data, 0, sizeof(ds_t));
    SetIpeLoopbackHeaderAdjustCtl(V, loopbackInternalPortBaseValid0_f, ds_data, 0);
    SetIpeLoopbackHeaderAdjustCtl(V, loopbackInternalPortBaseValid1_f, ds_data, 0);
    SetIpeLoopbackHeaderAdjustCtl(V, iloopDisableSourceHashInfo_f, ds_data, 0);
    SetIpeLoopbackHeaderAdjustCtl(V, iloopFromCpuDisableSourceHashInfo_f, ds_data, 0);
    SetIpeLoopbackHeaderAdjustCtl(V, loopbackFromFabricEn_f, ds_data, 0);
    SetIpeLoopbackHeaderAdjustCtl(V, alwaysUseHeaderLogicPort_f, ds_data, 1);
    SetIpeLoopbackHeaderAdjustCtl(V, iloopVrfidEn_f, ds_data, 0);  /* This field MUST be set 0. Asic not supports that outer vrfid covers inner vrfid.*/
    SetIpeLoopbackHeaderAdjustCtl(V, internalPortBase_f, ds_data, 0);
    SetIpeLoopbackHeaderAdjustCtl(V, loopbackClearSrcCidValidEn_f, ds_data, 1);

    cmd = DRV_IOW(IpeLoopbackHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_data));

    field_val = 1;
    cmd = DRV_IOW(IpeE2iLoopCtl_t, IpeE2iLoopCtl_useHeaderLogicPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipe_route_ctl_init(uint8 lchip)
{
    uint32 cmd = 0;
    ds_t ipe_lookup_route_ctl;
    ds_t ipe_route_martian_addr;
    ds_t ipe_route_ctl;
    ParserIpChecksumCtl_m checksum_ctl;

    sal_memset(&ipe_route_ctl, 0, sizeof(ipe_route_ctl));
    cmd = DRV_IOR(IpeRouteCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_route_ctl));

    SetIpeRouteCtl(V, rpfTypeForMcast_f, ipe_route_ctl, 0);
    SetIpeRouteCtl(V, exception2Discard_f, ipe_route_ctl, 0);
    SetIpeRouteCtl(V, greOption2CheckEn_f, ipe_route_ctl, 0);
    if(DRV_IS_DUET2(lchip))
    {
        SetIpeRouteCtl(V, exception3DiscardDisable_f, ipe_route_ctl, 1);
    }
    else
    {
        uint32 l3exp_discard_disable[2] = {0xFFFFFFFF, 0xFFFFFFFF};
        CTC_BIT_SET(l3exp_discard_disable[CTC_L3PDU_ACTION_INDEX_IPDA/32], CTC_L3PDU_ACTION_INDEX_IPDA%32);
        SetIpeRouteCtl(A, exception3DiscardDisable_f, ipe_route_ctl, l3exp_discard_disable);
    }

    SetIpeRouteCtl(V, exceptionSubIndex_f, ipe_route_ctl, 1);
    SetIpeRouteCtl(V, mcastEscapeToCpu_f, ipe_route_ctl, 0);
    SetIpeRouteCtl(V, mcastAddressMatchCheckDisable_f, ipe_route_ctl, 0);
    SetIpeRouteCtl(V, ipOptionsEscapeDisable_f, ipe_route_ctl, 0);
    SetIpeRouteCtl(V, routeOffsetByteShift_f, ipe_route_ctl, 0);
    SetIpeRouteCtl(V, ipTtlLimit_f, ipe_route_ctl, 2);
    SetIpeRouteCtl(V, icmpCheckRpfEn_f, ipe_route_ctl, 1);
    SetIpeRouteCtl(V, iviUseDaInfo_f, ipe_route_ctl, 1);
    SetIpeRouteCtl(V, ptIcmpEscape_f, ipe_route_ctl, 1);
    SetIpeRouteCtl(V, igmpPacketDecideMode_f, ipe_route_ctl, 1);
    SetIpeRouteCtl(V, ipBfdEscapeCtl_f, ipe_route_ctl, 1);
    SetIpeRouteCtl(V, l4CheckErrorDiscard_f, ipe_route_ctl, 1);
    SetIpeRouteCtl(V, routeUseBypassIngressEdit_f, ipe_route_ctl, 1);

    sal_memset(&ipe_route_martian_addr, 0, sizeof(IpeRouteMartianAddr_m));
    cmd = DRV_IOR(IpeRouteMartianAddr_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ipe_route_martian_addr));
    SetIpeRouteMartianAddr(V, flex0Mask_f, ipe_route_martian_addr, 0);
    SetIpeRouteMartianAddr(V, flex0Value_f, ipe_route_martian_addr, 0);
    SetIpeRouteMartianAddr(V, flex1Mask_f, ipe_route_martian_addr, 0);
    SetIpeRouteMartianAddr(V, flex1Value_f, ipe_route_martian_addr, 0);

    sal_memset(&ipe_lookup_route_ctl, 0, sizeof(IpeLookupRouteCtl_m));
    SetIpeLookupRouteCtl(V, martianCheckEn_f, ipe_lookup_route_ctl, 0xF);
    SetIpeLookupRouteCtl(V, martianAddressCheckDisable_f, ipe_lookup_route_ctl, 0);

    sal_memset(&checksum_ctl, 0, sizeof(checksum_ctl));
    SetParserIpChecksumCtl(V, ipChecksumCheckWithF_f, &checksum_ctl, 1);
    SetParserIpChecksumCtl(V, ipChecksumCheckWithZero_f, &checksum_ctl, 1);


    cmd = DRV_IOW(IpeRouteCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_route_ctl));

    cmd = DRV_IOW(IpeLookupRouteCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_lookup_route_ctl));

    cmd = DRV_IOW(IpeRouteMartianAddr_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_route_martian_addr));

    cmd = DRV_IOW(ParserIpChecksumCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &checksum_ctl));

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_ipe_brg_ctl_init(uint8 lchip)
{

    int32 step = 0;
    uint8 idx = 0;
    uint32 cmd = 0;
    IpeBridgeCtl_m ipe_bridge_ctl;

    sal_memset(&ipe_bridge_ctl, 0, sizeof(IpeBridgeCtl_m));

    step = IpeBridgeCtl_array_1_protocolExceptionSubIndex_f
           - IpeBridgeCtl_array_0_protocolExceptionSubIndex_f;

    for (idx = 0; idx < 16; idx++)
    {
        DRV_SET_FIELD_V(lchip, IpeBridgeCtl_t, IpeBridgeCtl_array_0_protocolExceptionSubIndex_f + idx * step,
                        &ipe_bridge_ctl, 32);
    }

    SetIpeBridgeCtl(V, bridgeOffsetByteShift_f, &ipe_bridge_ctl, 0);
    SetIpeBridgeCtl(V, discardForceBridge_f, &ipe_bridge_ctl, 0);
    SetIpeBridgeCtl(V, macDaIsPortMacException_f, &ipe_bridge_ctl, 0);
    SetIpeBridgeCtl(V, pbbMode_f, &ipe_bridge_ctl, 0);
    SetIpeBridgeCtl(V, protocolException_f, &ipe_bridge_ctl, 0x0440);
    SetIpeBridgeCtl(V, discardForceBridge_f, &ipe_bridge_ctl, 0);
    SetIpeBridgeCtl(V, bridgeExceptionSubIndexBase_f, &ipe_bridge_ctl, 0xf);
    SetIpeBridgeCtl(V, bridgeUseBypassIngressEdit_f, &ipe_bridge_ctl, 1);
    SetIpeBridgeCtl(V, bridgeControlLayer3ExceptionEn_f, &ipe_bridge_ctl, 1);

    cmd = DRV_IOW(IpeBridgeCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_bridge_ctl));

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_ipe_fwd_ctl_init(uint8 lchip)
{
    uint32 cmd = 0;
    IpeFwdCtl_m ipe_fwd_ctl;
    IpePreLookupCtl_m pre_lkp_ctl;
    EpeHdrEditMiscCtl_m misc_ctl;

    sal_memset(&ipe_fwd_ctl, 0, sizeof(IpeFwdCtl_m));
    sal_memset(&pre_lkp_ctl, 0, sizeof(IpePreLookupCtl_m));
    sal_memset(&misc_ctl, 0, sizeof(EpeHdrEditMiscCtl_m));


    cmd = DRV_IOR(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl));
    SetIpeFwdCtl(V, oamDestId_f, &ipe_fwd_ctl, SYS_RSV_PORT_OAM_CPU_ID);
    SetIpeFwdCtl(V, forceBackClearIngressAction_f, &ipe_fwd_ctl, 1);
    SetIpeFwdCtl(V, skipSpanPacketLoop_f, &ipe_fwd_ctl, 1);
    SetIpeFwdCtl(V, oamSdChkEn_f, &ipe_fwd_ctl, 1);
    cmd = DRV_IOW(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl));

    cmd = DRV_IOR(IpePreLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pre_lkp_ctl));
    SetIpePreLookupCtl(V, routeObeyStp_f, &pre_lkp_ctl, 1);
    SetIpePreLookupCtl(V, innerL3McastObeyFlowForceRouteCtl_f, &pre_lkp_ctl, 1);
    SetIpePreLookupCtl(V, innerL3McastObeyForceCheckCtl_f, &pre_lkp_ctl, 1);
    SetIpePreLookupCtl(V, innerL3McastObeyL3IfMcastEnCtl_f, &pre_lkp_ctl, 0);
    SetIpePreLookupCtl(V, innerL3McastObeyStpCheckCtl_f, &pre_lkp_ctl, 1);
    SetIpePreLookupCtl(V, innerL3UcastObeyFlowForceRouteCtl_f, &pre_lkp_ctl, 1);
    SetIpePreLookupCtl(V, innerL3UcastObeyForceUcastCtl_f, &pre_lkp_ctl, 1);
    SetIpePreLookupCtl(V, innerL3UcastObeyL3IfUcastEnCtl_f, &pre_lkp_ctl, 0);
    SetIpePreLookupCtl(V, innerL3UcastObeyStpCheckCtl_f, &pre_lkp_ctl, 1);

    cmd = DRV_IOW(IpePreLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pre_lkp_ctl));

    cmd = DRV_IOR(EpeHdrEditMiscCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &misc_ctl));
    SetEpeHdrEditMiscCtl(V, cfgL3HdrLenCorrectEn_f, &misc_ctl, 0);
    cmd = DRV_IOW(EpeHdrEditMiscCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &misc_ctl));

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_buffer_retrieve_ctl_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    QWriteRsvChanCtl_m ctl;
    BufStorePktSizeChkCtl_m pkt_size;
    EpeHdrAdjustMiscCtl_m epe_ctl;
    uint32 max_len = 0;

    /* enable dma channel */
    tbl_id = QWriteRsvChanCtl_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ctl));
    SetQWriteRsvChanCtl(V,reservedChannelIdValid_f, &ctl, 1);
    SetQWriteRsvChanCtl(V,reservedChannelId_f, &ctl, MCHIP_CAP(SYS_CAP_CHANID_DROP));
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ctl));

    max_len = 16127;
    tbl_id = BufStorePktSizeChkCtl_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pkt_size));
    SetBufStorePktSizeChkCtl(V, minEopLen_f, &pkt_size, 10);
    SetBufStorePktSizeChkCtl(V, maxBufCnt_f, &pkt_size, (max_len/288));
    SetBufStorePktSizeChkCtl(V, maxEopLen_f, &pkt_size, (max_len%288));
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pkt_size));

    tbl_id = EpeHdrAdjustMiscCtl_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_ctl));
    SetEpeHdrAdjustMiscCtl(V, minPktLen_f, &epe_ctl, 10);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_bufferstoreCpuRxLogChannelMap_init(uint8 lchip)
{
    uint32 cmd = 0;
    BufferStoreCpuRxLogChannelMap_m cpu_rx_channel_map;

    sal_memset(&cpu_rx_channel_map,0,sizeof(BufferStoreCpuRxLogChannelMap_m));
    SetBufferStoreCpuRxLogChannelMap(V,g_0_cpuChanId_f,&cpu_rx_channel_map,0x2f);
    SetBufferStoreCpuRxLogChannelMap(V,g_1_cpuChanId_f,&cpu_rx_channel_map,MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0));
    SetBufferStoreCpuRxLogChannelMap(V,g_2_cpuChanId_f,&cpu_rx_channel_map,MCHIP_CAP(SYS_CAP_CHANID_DMA_RX1));
    SetBufferStoreCpuRxLogChannelMap(V,g_3_cpuChanId_f,&cpu_rx_channel_map,MCHIP_CAP(SYS_CAP_CHANID_DMA_RX2));
    SetBufferStoreCpuRxLogChannelMap(V,g_4_cpuChanId_f,&cpu_rx_channel_map,MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3));

    cmd = DRV_IOW(BufferStoreCpuRxLogChannelMap_t,DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip,0,cmd,&cpu_rx_channel_map));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qmgr_ctl_init(uint8 lchip)
{
    uint32 cmd = 0;
    QWritePktLenCtl_m  length_adjust;
    LagEngineCtl_m lag_engine_ctl;
	sal_memset(&length_adjust,0,sizeof(QWritePktLenCtl_m));

    cmd = DRV_IOR(QWritePktLenCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &length_adjust));

    SetQWritePktLenCtl(V, g0_0_packetLengthAdjustType_f , &length_adjust, 0);
    SetQWritePktLenCtl(V, g0_0_packetLengthAdjust_f , &length_adjust, 0);
	SetQWritePktLenCtl(V, g0_1_packetLengthAdjustType_f , &length_adjust, 0);
    SetQWritePktLenCtl(V, g0_1_packetLengthAdjust_f , &length_adjust, 4);
	SetQWritePktLenCtl(V, g0_2_packetLengthAdjustType_f , &length_adjust, 0);
    SetQWritePktLenCtl(V, g0_2_packetLengthAdjust_f , &length_adjust, 8);
	SetQWritePktLenCtl(V, g0_3_packetLengthAdjustType_f , &length_adjust, 0);
    SetQWritePktLenCtl(V, g0_3_packetLengthAdjust_f , &length_adjust, 12);
	SetQWritePktLenCtl(V, g0_4_packetLengthAdjustType_f , &length_adjust, 0);
    SetQWritePktLenCtl(V, g0_4_packetLengthAdjust_f , &length_adjust, 14);
	SetQWritePktLenCtl(V, g0_5_packetLengthAdjustType_f , &length_adjust, 0);
    SetQWritePktLenCtl(V, g0_5_packetLengthAdjust_f , &length_adjust, 16);
	SetQWritePktLenCtl(V, g0_6_packetLengthAdjustType_f , &length_adjust, 0);
    SetQWritePktLenCtl(V, g0_6_packetLengthAdjust_f , &length_adjust, 18);
	SetQWritePktLenCtl(V, g0_7_packetLengthAdjustType_f , &length_adjust, 0);
    SetQWritePktLenCtl(V, g0_7_packetLengthAdjust_f , &length_adjust, 22);

    cmd = DRV_IOW(QWritePktLenCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &length_adjust));

    /*Lag Engine Ctl init*/
    sal_memset(&lag_engine_ctl, 0, sizeof(LagEngineCtl_m));
    cmd = DRV_IOR(LagEngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lag_engine_ctl));
    SetLagEngineCtl(V, sgmacEn_f, &lag_engine_ctl, 1);
    SetLagEngineCtl(V, discardUnkownSgmacGroup_f, &lag_engine_ctl, 1);
    cmd = DRV_IOW(LagEngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lag_engine_ctl));
    return CTC_E_NONE;

}

int32
sys_usw_lkup_adjust_len_index(uint8 lchip, uint8 adjust_len, uint8* len_index)
{
    uint32 cmd;
    uint8 index = 0;
    uint32 length = 0;
    uint8 bfind = 0;

    SYS_REGISTER_INIT_CHECK;

    for (index = 0; index < 8; index++)
    {
        cmd = DRV_IOR(QWritePktLenCtl_t, QWritePktLenCtl_g0_0_packetLengthAdjust_f + index*2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &length));

        if (length >= adjust_len)
        {
            *len_index = index;
            bfind = 1;
            break;
        }
    }
	if(!bfind)
	{
	   *len_index  = 7;
	}

    return CTC_E_NONE;
}

/* add default entry adindex=0 for ipuc */
int32
_sys_usw_mac_default_entry_init(uint8 lchip)
{
#ifdef NEVER
    ds_mac_t macda;
    uint32 cmd = 0;
    uint8 chip_num = 0;
    uint8 chip_id = 0;

    sal_memset(&macda, 0, sizeof(ds_mac_t));

    macda.ds_fwd_ptr = 0xFFFF;

    /* write default entry */
    chip_num = sys_usw_get_local_chip_num(lchip);

    for (chip_id = 0; chip_id < chip_num; chip_id++)
    {
        /* build DsMac entry */

        cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(chip_id, 0, cmd, &macda));
    }

    return CTC_E_NONE;
#endif /* never */
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_timestamp_engine_init(uint8 lchip)
{
#ifdef NEVER
    uint8 lchip_num = 0;

    uint32 cmd = 0;
    ptp_frc_ctl_t ptp_frc_ctl;

    sal_memset(&ptp_frc_ctl, 0, sizeof(ptp_frc_ctl));
    lchip_num = sys_usw_get_local_chip_num(lchip);

    if(TRUE)
    {

        ptp_frc_ctl.frc_en_ref = 1;
        cmd = DRV_IOW(PtpFrcCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ptp_frc_ctl));
    }

    return CTC_E_NONE;
#endif /* never */
    return CTC_E_NONE;
}


STATIC int32
_sys_usw_ipg_init(uint8 lchip)
{
    uint32 cmd = 0;
    IpeIpgCtl_m  ipe_ipg_ctl;
    EpeIpgCtl_m  epe_ipg_ctl;

    sal_memset(&ipe_ipg_ctl, 0, sizeof(ipe_ipg_ctl));
    sal_memset(&epe_ipg_ctl, 0, sizeof(epe_ipg_ctl));

    SetIpeIpgCtl(V, array_0_ipg_f, &ipe_ipg_ctl, 20);
    SetIpeIpgCtl(V, array_1_ipg_f, &ipe_ipg_ctl, 20);
    SetIpeIpgCtl(V, array_2_ipg_f, &ipe_ipg_ctl, 20);
    SetIpeIpgCtl(V, array_3_ipg_f, &ipe_ipg_ctl, 20);

    SetEpeIpgCtl(V, array_0_ipg_f, &epe_ipg_ctl, 20);
    SetEpeIpgCtl(V, array_1_ipg_f, &epe_ipg_ctl, 20);
    SetEpeIpgCtl(V, array_2_ipg_f, &epe_ipg_ctl, 20);
    SetEpeIpgCtl(V, array_3_ipg_f, &epe_ipg_ctl, 20);

    cmd = DRV_IOW(IpeIpgCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_ipg_ctl));

    cmd = DRV_IOW(EpeIpgCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_ipg_ctl));

    return CTC_E_NONE;

}

/** < [TM] reserved */
int32
_sys_usw_set_glb_scl_property(uint8 lchip, uint8 scl_id, uint8 key_share_mode)
{
    uint32 cmd = 0;
    IpeUserIdTcamCtl_m ipe_userid_tcam_ctl;

    CTC_MAX_VALUE_CHECK(scl_id, 3);
    CTC_MAX_VALUE_CHECK(key_share_mode, 3);

    if (0 == scl_id)
    {
        SetIpeUserIdTcamCtl(V, lookupLevel_0_udf320KeyShareType_f, &ipe_userid_tcam_ctl, key_share_mode);
    }
    else if (1 == scl_id)
    {
        SetIpeUserIdTcamCtl(V, lookupLevel_1_udf320KeyShareType_f, &ipe_userid_tcam_ctl, key_share_mode);
    }
    else if (2 == scl_id)
    {
        SetIpeUserIdTcamCtl(V, lookupLevel_2_udf320KeyShareType_f, &ipe_userid_tcam_ctl, key_share_mode);
    }
    else
    {
        SetIpeUserIdTcamCtl(V, lookupLevel_3_udf320KeyShareType_f, &ipe_userid_tcam_ctl, key_share_mode);
    }
    cmd = DRV_IOW(IpeUserIdTcamCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_userid_tcam_ctl));

    return CTC_E_NONE;
}

int32
_sys_usw_get_glb_scl_property(uint8 lchip, uint8 scl_id, uint8* key_share_mode)
{
    uint32 cmd = 0;
    IpeUserIdTcamCtl_m ipe_userid_tcam_ctl;

    CTC_MAX_VALUE_CHECK(scl_id, 3);

    cmd = DRV_IOR(IpeUserIdTcamCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_userid_tcam_ctl));

    if (0 == scl_id)
    {
        *key_share_mode = GetIpeUserIdTcamCtl(V, lookupLevel_0_udf320KeyShareType_f, &ipe_userid_tcam_ctl);
    }
    else if (1 == scl_id)
    {
        *key_share_mode = GetIpeUserIdTcamCtl(V, lookupLevel_1_udf320KeyShareType_f, &ipe_userid_tcam_ctl);
    }
    else if (2 == scl_id)
    {
        *key_share_mode = GetIpeUserIdTcamCtl(V, lookupLevel_2_udf320KeyShareType_f, &ipe_userid_tcam_ctl);
    }
    else
    {
        *key_share_mode= GetIpeUserIdTcamCtl(V, lookupLevel_3_udf320KeyShareType_f, &ipe_userid_tcam_ctl);
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_set_glb_acl_property(uint8 lchip, ctc_global_acl_property_t* pacl_property)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint8  step = 0;
    uint8 loop = 0;
    uint8 fwd_ext_key_u1_bmp = 0;
    FlowTcamLookupCtl_m  flow_ctl;
    EpeAclQosCtl_m       epe_acl_ctl;
    IpeFwdAclCtl_m       ipe_fwd_acl;
    EpePktProcCtl_m      epe_pkt_proc_ctl;
    CTC_PTR_VALID_CHECK(pacl_property);
    CTC_MAX_VALUE_CHECK(pacl_property->dir,CTC_BOTH_DIRECTION);
    CTC_MAX_VALUE_CHECK(pacl_property->key_cid_en,1);
    CTC_MAX_VALUE_CHECK(pacl_property->discard_pkt_lkup_en,1);
    CTC_MAX_VALUE_CHECK(pacl_property->key_ipv6_da_addr_mode,CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_16);
    CTC_MAX_VALUE_CHECK(pacl_property->key_ipv6_sa_addr_mode,CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_16);
    CTC_MAX_VALUE_CHECK(pacl_property->cid_key_ipv6_da_addr_mode,CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_16);
    CTC_MAX_VALUE_CHECK(pacl_property->cid_key_ipv6_sa_addr_mode,CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_16);

    if(pacl_property->dir == CTC_BOTH_DIRECTION)
    {
        CTC_MAX_VALUE_CHECK(pacl_property->lkup_level,2);
    }
    cmd = DRV_IOR(FlowTcamLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flow_ctl));

    if(pacl_property->dir == CTC_INGRESS || pacl_property->dir == CTC_BOTH_DIRECTION )
    {
        IpePreLookupAclCtl_m ipe_acl_ctl;
        IpeAclQosCtl_m       ipe_aclqos_ctl;

        CTC_MAX_VALUE_CHECK(pacl_property->lkup_level,7);
        CTC_MAX_VALUE_CHECK(pacl_property->random_log_pri, 7);

        /*discard_pkt_lkup_en*/
        cmd = DRV_IOR(IpeFwdAclCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_acl));
	    value = GetIpeFwdAclCtl(V, dropPktDisableAcl_f,&ipe_fwd_acl);
        if(!pacl_property->discard_pkt_lkup_en)
        {
           CTC_BIT_SET(value ,pacl_property->lkup_level);
        }
        else
        {
            CTC_BIT_UNSET(value ,pacl_property->lkup_level);
        }
		SetIpeFwdAclCtl(V, dropPktDisableAcl_f, &ipe_fwd_acl,value);
        for(loop = 0;loop <MAX_CTC_PARSER_L3_TYPE;loop++ )
        {
          if(pacl_property->copp_key_use_ext_mode[loop])
          {
             CTC_BIT_SET(value, loop);
          }
          else
		  {
             CTC_BIT_UNSET(value, loop);
          }
		}
	    SetIpeFwdAclCtl(V, useCopp640bitsKey_f, &ipe_fwd_acl,value);

		for(loop = 0;loop <MAX_CTC_PARSER_L3_TYPE;loop++ )
        {
          if(pacl_property->l2l3_ext_key_use_l2l3_key[loop])
          {
             CTC_BIT_SET(value, loop);
          }
          else
		  {
             CTC_BIT_UNSET(value, loop);
          }
		}
	    SetIpeFwdAclCtl(V, aclForceL2L3ExtKeyToL2L3Key_f, &ipe_fwd_acl,value);

		SetIpeFwdAclCtl(V, aclForceL3BasicV6ToL3ExtV6_f, &ipe_fwd_acl,pacl_property->l3_key_ipv6_use_compress_addr?0:1);

        cmd = DRV_IOW(IpeFwdAclCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_acl));

        /*v6 basic key*/
        step = FlowTcamLookupCtl_gIngrAcl_1_v6BasicKeyMode0IpAddr1Encode_f -
                FlowTcamLookupCtl_gIngrAcl_0_v6BasicKeyMode0IpAddr1Encode_f;
        SetFlowTcamLookupCtl(V, gIngrAcl_0_v6BasicKeyMode0IpAddr1Encode_f + pacl_property->lkup_level *step,
                                  &flow_ctl, pacl_property->key_ipv6_da_addr_mode);
        SetFlowTcamLookupCtl(V, gIngrAcl_0_v6BasicKeyMode0IpAddr2Encode_f + pacl_property->lkup_level *step,
                                  &flow_ctl, pacl_property->key_ipv6_sa_addr_mode);

        /*fwd basic key*/
        step = FlowTcamLookupCtl_gIngrAcl_1_v6ForwardBasicKeyMode0IpAddr1Encode_f -
                FlowTcamLookupCtl_gIngrAcl_0_v6ForwardBasicKeyMode0IpAddr1Encode_f;
        SetFlowTcamLookupCtl(V, gIngrAcl_0_v6ForwardBasicKeyMode0IpAddr1Encode_f + pacl_property->lkup_level *step,
                                  &flow_ctl, pacl_property->key_ipv6_da_addr_mode);
        SetFlowTcamLookupCtl(V, gIngrAcl_0_v6ForwardBasicKeyMode0IpAddr2Encode_f + pacl_property->lkup_level *step,
                                  &flow_ctl, pacl_property->key_ipv6_sa_addr_mode);
         /*cid key*/
        SetFlowTcamLookupCtl(V, ingrSgaclKeyIpv6DaAddrEncode_f,&flow_ctl, pacl_property->cid_key_ipv6_da_addr_mode);
        SetFlowTcamLookupCtl(V, ingrSgaclKeyIpv6SaAddrEncode_f,&flow_ctl, pacl_property->cid_key_ipv6_sa_addr_mode);

        /*CID enable*/
        step = FlowTcamLookupCtl_gIngrAcl_1_categoryIdFieldValid_f -
                FlowTcamLookupCtl_gIngrAcl_0_categoryIdFieldValid_f;
        SetFlowTcamLookupCtl(V, gIngrAcl_0_categoryIdFieldValid_f + pacl_property->lkup_level *step,
                                  &flow_ctl, pacl_property->key_cid_en);

        cmd = DRV_IOR(IpePreLookupAclCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_acl_ctl));
        SetIpePreLookupAclCtl(V, bridgePktAclObeyStpBlock_f, &ipe_acl_ctl, (pacl_property->stp_blocked_pkt_lkup_en)?0:1);
        SetIpePreLookupAclCtl(V, nonBridgePktAclObeyStpBlock_f, &ipe_acl_ctl, (pacl_property->stp_blocked_pkt_lkup_en)?0:1);
        SetIpePreLookupAclCtl(V, layer2TypeUsedAsVlanNum_f, &ipe_acl_ctl, (pacl_property->l2_type_as_vlan_num)?1:0);
        cmd = DRV_IOW(IpePreLookupAclCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_acl_ctl));

        /*random log priority*/
        step = IpeAclQosCtl_aclLogSelectMap_1_logBlockNum_f - IpeAclQosCtl_aclLogSelectMap_0_logBlockNum_f;
        cmd = DRV_IOR(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_aclqos_ctl));
        SetIpeAclQosCtl(V, aclLogSelectMap_0_logBlockNum_f+step*pacl_property->lkup_level, &ipe_aclqos_ctl, pacl_property->random_log_pri);
        cmd = DRV_IOW(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_aclqos_ctl));

   }

    SetFlowTcamLookupCtl(V, acl80bitKeyUseCvlan_f,&flow_ctl, pacl_property->key_ctag_en? 1 : 0);
    SetFlowTcamLookupCtl(V, aclForwardingBasicKeyUseCvlan_f,&flow_ctl, pacl_property->key_ctag_en? 1 : 0);
    SetFlowTcamLookupCtl(V, coppBasicKeyUseCtag_f,&flow_ctl, pacl_property->key_ctag_en? 1 : 0);
    SetFlowTcamLookupCtl(V, coppExtKeyUseCtag_f,&flow_ctl, pacl_property->key_ctag_en? 1 : 0);
    fwd_ext_key_u1_bmp = GetFlowTcamLookupCtl(V, forwardingAclKeyUnion1Type_f, &flow_ctl);
    if (pacl_property->fwd_ext_key_l2_hdr_en)
    {
        CTC_BIT_UNSET(fwd_ext_key_u1_bmp, pacl_property->lkup_level);
    }
    else
    {
        CTC_BIT_SET(fwd_ext_key_u1_bmp, pacl_property->lkup_level);
    }
    SetFlowTcamLookupCtl(V, forwardingAclKeyUnion1Type_f, &flow_ctl, fwd_ext_key_u1_bmp);

    if(pacl_property->dir == CTC_EGRESS || pacl_property->dir == CTC_BOTH_DIRECTION)
    {
        CTC_MAX_VALUE_CHECK(pacl_property->lkup_level,2);
        CTC_MAX_VALUE_CHECK(pacl_property->random_log_pri, 2);
        cmd = DRV_IOR(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_ctl));
        cmd = DRV_IOR(EpePktProcCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_pkt_proc_ctl));
        value = GetEpeAclQosCtl(V, dropPktDisableAcl_f,&epe_acl_ctl);
        if(!pacl_property->discard_pkt_lkup_en)
        {
            CTC_BIT_SET(value ,pacl_property->lkup_level);
        }
        else
        {
            CTC_BIT_UNSET(value ,pacl_property->lkup_level);
        }
        SetEpeAclQosCtl(V, dropPktDisableAcl_f, &epe_acl_ctl,value);

	   value = 0;
	   for(loop = 0;loop <MAX_CTC_PARSER_L3_TYPE;loop++ )
        {
          if(pacl_property->l2l3_ext_key_use_l2l3_key[loop])
          {
             CTC_BIT_SET(value, loop);
          }
          else
		  {
             CTC_BIT_UNSET(value, loop);
          }
		}
	    SetEpeAclQosCtl(V, aclForceL2L3ExtKeyToL2L3Key_f, &epe_acl_ctl,value);
		SetEpeAclQosCtl(V, aclForceL3BasicV6ToL3ExtV6_f, &epe_acl_ctl,pacl_property->l3_key_ipv6_use_compress_addr?0:1);
        /*random log priority*/
        step = EpeAclQosCtl_aclLogSelectMap_1_logBlockNum_f - EpeAclQosCtl_aclLogSelectMap_0_logBlockNum_f;
        SetEpeAclQosCtl(V, aclLogSelectMap_0_logBlockNum_f+step*pacl_property->lkup_level, &epe_acl_ctl, pacl_property->random_log_pri);

        cmd = DRV_IOW(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_ctl));

        /*v6 basic key*/
        step = FlowTcamLookupCtl_gEgrAcl_1_v6BasicKeyMode0IpAddr1Encode_f -
                FlowTcamLookupCtl_gEgrAcl_0_v6BasicKeyMode0IpAddr1Encode_f;
        SetFlowTcamLookupCtl(V, gEgrAcl_0_v6BasicKeyMode0IpAddr1Encode_f + pacl_property->lkup_level *step,
                                  &flow_ctl, pacl_property->key_ipv6_da_addr_mode);
        SetFlowTcamLookupCtl(V, gEgrAcl_0_v6BasicKeyMode0IpAddr2Encode_f + pacl_property->lkup_level *step,
                                  &flow_ctl, pacl_property->key_ipv6_sa_addr_mode);
        /*fwd basic key*/
        step = FlowTcamLookupCtl_gEgrAcl_1_v6ForwardBasicKeyMode0IpAddr1Encode_f -
                FlowTcamLookupCtl_gEgrAcl_0_v6ForwardBasicKeyMode0IpAddr1Encode_f;
        SetFlowTcamLookupCtl(V, gEgrAcl_0_v6ForwardBasicKeyMode0IpAddr1Encode_f + pacl_property->lkup_level *step,
                                  &flow_ctl, pacl_property->key_ipv6_da_addr_mode);
        SetFlowTcamLookupCtl(V, gEgrAcl_0_v6ForwardBasicKeyMode0IpAddr2Encode_f + pacl_property->lkup_level *step,
                                  &flow_ctl, pacl_property->key_ipv6_sa_addr_mode);
        /*cid key*/
        SetFlowTcamLookupCtl(V, egrSgaclKeyIpv6DaAddrEncode_f,&flow_ctl, pacl_property->cid_key_ipv6_da_addr_mode);
        SetFlowTcamLookupCtl(V, egrSgaclKeyIpv6SaAddrEncode_f,&flow_ctl, pacl_property->cid_key_ipv6_sa_addr_mode);

        /*CID enable*/
        step = FlowTcamLookupCtl_gEgrAcl_1_categoryIdFieldValid_f -
                FlowTcamLookupCtl_gEgrAcl_0_categoryIdFieldValid_f;

        SetFlowTcamLookupCtl(V, gEgrAcl_0_categoryIdFieldValid_f + pacl_property->lkup_level *step,
                                  &flow_ctl, pacl_property->key_cid_en? 1 : 0);
        SetEpePktProcCtl(V, layer2TypeUsedAsVlanNum_f, &epe_pkt_proc_ctl, pacl_property->l2_type_as_vlan_num?1:0);
        cmd = DRV_IOW(EpePktProcCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_pkt_proc_ctl));
  }
    cmd = DRV_IOW(FlowTcamLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flow_ctl));

    return CTC_E_NONE;
}

int32
sys_usw_get_glb_acl_property(uint8 lchip, ctc_global_acl_property_t* pacl_property)
{

    uint32 cmd = 0;
	uint8  loop = 0;
    uint32 value = 0;
    uint8 step = 0;
    uint8 fwd_ext_key_u1_bmp = 0;
    FlowTcamLookupCtl_m  flow_ctl;
	EpeAclQosCtl_m       epe_acl_ctl;
    IpeFwdAclCtl_m       ipe_fwd_acl;

    SYS_REGISTER_INIT_CHECK;
    CTC_PTR_VALID_CHECK(pacl_property);
    cmd = DRV_IOR(FlowTcamLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flow_ctl));
    pacl_property->key_ctag_en =  GetFlowTcamLookupCtl(V, acl80bitKeyUseCvlan_f,&flow_ctl);
    fwd_ext_key_u1_bmp = GetFlowTcamLookupCtl(V, forwardingAclKeyUnion1Type_f, &flow_ctl);
    pacl_property->fwd_ext_key_l2_hdr_en = CTC_IS_BIT_SET(fwd_ext_key_u1_bmp, pacl_property->lkup_level)? 0: 1;
    if(pacl_property->dir == CTC_INGRESS )
    {
        IpePreLookupAclCtl_m ipe_acl_ctl;
        IpeAclQosCtl_m       ipe_aclqos_ctl;

        CTC_MAX_VALUE_CHECK(pacl_property->lkup_level,7);
        cmd = DRV_IOR(IpeFwdAclCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_acl));

		value = GetIpeFwdAclCtl(V,dropPktDisableAcl_f,&ipe_fwd_acl);
        pacl_property->discard_pkt_lkup_en = !CTC_IS_BIT_SET(value,pacl_property->lkup_level);
        value = GetIpeFwdAclCtl(V, useCopp640bitsKey_f, &ipe_fwd_acl);
        for(loop = 0;loop <MAX_CTC_PARSER_L3_TYPE;loop++ )
        {
            pacl_property->copp_key_use_ext_mode[loop] = CTC_IS_BIT_SET(value,loop);
		}
		value = GetIpeFwdAclCtl(V, aclForceL2L3ExtKeyToL2L3Key_f, &ipe_fwd_acl);
		for(loop = 0;loop <MAX_CTC_PARSER_L3_TYPE;loop++ )
        {
            pacl_property->l2l3_ext_key_use_l2l3_key[loop] =  CTC_IS_BIT_SET(value,loop);
		}

		pacl_property->l3_key_ipv6_use_compress_addr = !GetIpeFwdAclCtl(V, aclForceL3BasicV6ToL3ExtV6_f, &ipe_fwd_acl);

        /*v6 basic key*/
        step = FlowTcamLookupCtl_gIngrAcl_1_v6BasicKeyMode0IpAddr1Encode_f -
                FlowTcamLookupCtl_gIngrAcl_0_v6BasicKeyMode0IpAddr1Encode_f;
        pacl_property->key_ipv6_da_addr_mode = GetFlowTcamLookupCtl(V, gIngrAcl_0_v6BasicKeyMode0IpAddr1Encode_f + pacl_property->lkup_level *step,
                                                &flow_ctl);
        pacl_property->key_ipv6_sa_addr_mode = GetFlowTcamLookupCtl(V, gIngrAcl_0_v6BasicKeyMode0IpAddr2Encode_f + pacl_property->lkup_level *step,
                                                &flow_ctl);
        /*fwd basic key*/
        step = FlowTcamLookupCtl_gIngrAcl_1_v6ForwardBasicKeyMode0IpAddr1Encode_f -
                FlowTcamLookupCtl_gIngrAcl_0_v6ForwardBasicKeyMode0IpAddr1Encode_f;
        pacl_property->key_ipv6_da_addr_mode = GetFlowTcamLookupCtl(V, gIngrAcl_0_v6ForwardBasicKeyMode0IpAddr1Encode_f + pacl_property->lkup_level *step,
                                                &flow_ctl);
        pacl_property->key_ipv6_sa_addr_mode = GetFlowTcamLookupCtl(V, gIngrAcl_0_v6ForwardBasicKeyMode0IpAddr2Encode_f + pacl_property->lkup_level *step,
                                                &flow_ctl);
        /*cid key*/
        pacl_property->cid_key_ipv6_da_addr_mode = GetFlowTcamLookupCtl(V, ingrSgaclKeyIpv6DaAddrEncode_f,&flow_ctl);
        pacl_property->cid_key_ipv6_sa_addr_mode = GetFlowTcamLookupCtl(V, ingrSgaclKeyIpv6SaAddrEncode_f,&flow_ctl );

        /*CID enable*/
        step = FlowTcamLookupCtl_gIngrAcl_1_categoryIdFieldValid_f -
                FlowTcamLookupCtl_gIngrAcl_0_categoryIdFieldValid_f;
        pacl_property->key_cid_en = GetFlowTcamLookupCtl(V, gIngrAcl_0_categoryIdFieldValid_f + pacl_property->lkup_level *step,
                                      &flow_ctl );

        cmd = DRV_IOR(IpePreLookupAclCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_acl_ctl));
        pacl_property->stp_blocked_pkt_lkup_en = GetIpePreLookupAclCtl(V, bridgePktAclObeyStpBlock_f, &ipe_acl_ctl)?0:1;
        pacl_property->l2_type_as_vlan_num = GetIpePreLookupAclCtl(V, layer2TypeUsedAsVlanNum_f, &ipe_acl_ctl)?1:0;

        /*random log priority*/
        step = IpeAclQosCtl_aclLogSelectMap_1_logBlockNum_f - IpeAclQosCtl_aclLogSelectMap_0_logBlockNum_f;
        cmd = DRV_IOR(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_aclqos_ctl));
        pacl_property->random_log_pri = GetIpeAclQosCtl(V, aclLogSelectMap_0_logBlockNum_f+step*pacl_property->lkup_level, &ipe_aclqos_ctl);

    }
    else if(pacl_property->dir == CTC_EGRESS )
    {
        EpePktProcCtl_m      epe_pkt_proc_ctl;
        CTC_MAX_VALUE_CHECK(pacl_property->lkup_level,2);
        cmd = DRV_IOR(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_ctl));
        value = GetEpeAclQosCtl(V,dropPktDisableAcl_f,&epe_acl_ctl);
        pacl_property->discard_pkt_lkup_en = !CTC_IS_BIT_SET(value,pacl_property->lkup_level);
        pacl_property->l3_key_ipv6_use_compress_addr = !GetEpeAclQosCtl(V, aclForceL3BasicV6ToL3ExtV6_f, &epe_acl_ctl);
        value = GetEpeAclQosCtl(V, aclForceL2L3ExtKeyToL2L3Key_f, &epe_acl_ctl);
		for(loop = 0;loop <MAX_CTC_PARSER_L3_TYPE;loop++ )
        {
            pacl_property->l2l3_ext_key_use_l2l3_key[loop] = CTC_IS_BIT_SET(value,loop);
		}

        /*v6 basic key*/
        step = FlowTcamLookupCtl_gEgrAcl_1_v6BasicKeyMode0IpAddr1Encode_f -
                FlowTcamLookupCtl_gEgrAcl_0_v6BasicKeyMode0IpAddr1Encode_f;
        pacl_property->key_ipv6_da_addr_mode = GetFlowTcamLookupCtl(V, gEgrAcl_0_v6BasicKeyMode0IpAddr1Encode_f + pacl_property->lkup_level *step,
                            &flow_ctl);
        pacl_property->key_ipv6_sa_addr_mode = GetFlowTcamLookupCtl(V, gEgrAcl_0_v6BasicKeyMode0IpAddr2Encode_f + pacl_property->lkup_level *step,
                            &flow_ctl);
        /*fwd basic key*/
        step = FlowTcamLookupCtl_gEgrAcl_1_v6ForwardBasicKeyMode0IpAddr1Encode_f -
                FlowTcamLookupCtl_gEgrAcl_0_v6ForwardBasicKeyMode0IpAddr1Encode_f;
        pacl_property->key_ipv6_da_addr_mode = GetFlowTcamLookupCtl(V, gEgrAcl_0_v6ForwardBasicKeyMode0IpAddr1Encode_f + pacl_property->lkup_level *step,
                                                &flow_ctl);
        pacl_property->key_ipv6_sa_addr_mode = GetFlowTcamLookupCtl(V, gEgrAcl_0_v6ForwardBasicKeyMode0IpAddr2Encode_f + pacl_property->lkup_level *step,
                                                &flow_ctl);
         /*cid key*/
        pacl_property->cid_key_ipv6_da_addr_mode = GetFlowTcamLookupCtl(V, egrSgaclKeyIpv6DaAddrEncode_f,&flow_ctl);
        pacl_property->cid_key_ipv6_sa_addr_mode = GetFlowTcamLookupCtl(V, egrSgaclKeyIpv6SaAddrEncode_f,&flow_ctl );

        /*CID enable*/
        step = FlowTcamLookupCtl_gEgrAcl_1_categoryIdFieldValid_f -
        FlowTcamLookupCtl_gEgrAcl_0_categoryIdFieldValid_f;
        pacl_property->key_cid_en = GetFlowTcamLookupCtl(V, gEgrAcl_0_categoryIdFieldValid_f + pacl_property->lkup_level *step,
                                 &flow_ctl);

        /*random log priority*/
        step = EpeAclQosCtl_aclLogSelectMap_1_logBlockNum_f - EpeAclQosCtl_aclLogSelectMap_0_logBlockNum_f;
        pacl_property->random_log_pri = GetEpeAclQosCtl(V, aclLogSelectMap_0_logBlockNum_f+step*pacl_property->lkup_level, &epe_acl_ctl);
        cmd = DRV_IOR(EpePktProcCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_pkt_proc_ctl));
        pacl_property->l2_type_as_vlan_num = GetEpePktProcCtl(V, layer2TypeUsedAsVlanNum_f, &epe_pkt_proc_ctl)?1:0;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_set_glb_cid_property(uint8 lchip, ctc_global_cid_property_t* pcid_property)
{
    ParserEthernetCtl_m     cid_parser_ctl;
    IpeFwdCategoryCtl_m     cid_glb_ctl;
	EpeHeaderEditCtl_m      epe_edit_ctl;
	EpeNextHopCtl_m         epe_nh_ctl;
    uint32 cmd = 0;
    uint8 step = 0,loop =0 ;
	uint32 field_val = 0;
	CTC_PTR_VALID_CHECK(pcid_property);

	if(pcid_property->global_cid_en)
	{
        SYS_USW_CID_CHECK(lchip,pcid_property->global_cid);
	}
	if(pcid_property->reassign_cid_pri_en)
    {
       if(!pcid_property->is_dst_cid)
       {
        CTC_MAX_VALUE_CHECK(pcid_property->pkt_cid_pri,7);
        CTC_MAX_VALUE_CHECK(pcid_property->iloop_cid_pri,7);
        CTC_MAX_VALUE_CHECK(pcid_property->global_cid_pri,7);
        CTC_MAX_VALUE_CHECK(pcid_property->flow_table_cid_pri,7);
        CTC_MAX_VALUE_CHECK(pcid_property->fwd_table_cid_pri,7);
        CTC_MAX_VALUE_CHECK(pcid_property->if_cid_pri,7);
        CTC_MAX_VALUE_CHECK(pcid_property->default_cid_pri,7);
       }
        else
        {
            CTC_MAX_VALUE_CHECK(pcid_property->flow_table_cid_pri,3);
            CTC_MAX_VALUE_CHECK(pcid_property->fwd_table_cid_pri,3);
            CTC_MAX_VALUE_CHECK(pcid_property->default_cid_pri,3);
        }
	}

	field_val = (pcid_property->cross_chip_cid_en) ? 1:0;
	cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_extCidValid_f);
	CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOR(ParserEthernetCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cid_parser_ctl));
    SetParserEthernetCtl(V,supportMetaDataHeader_f,&cid_parser_ctl,pcid_property->cmd_parser_en?1:0);
    SetParserEthernetCtl(V,metaDataEtherType_f,&cid_parser_ctl,pcid_property->cmd_ethtype);

    cmd = DRV_IOW(ParserEthernetCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cid_parser_ctl));

    cmd = DRV_IOR(EpeHeaderEditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_edit_ctl));
    SetEpeHeaderEditCtl(V,cidHdrEtherType_f,&epe_edit_ctl,pcid_property->cmd_ethtype);
    cmd = DRV_IOW(EpeHeaderEditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_edit_ctl));

	cmd = DRV_IOR(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_nh_ctl));
    SetEpeNextHopCtl(V,bypassInsertCidThreValid_f,&epe_nh_ctl,1);
	SetEpeNextHopCtl(V,bypassInsertCidThre_f,&epe_nh_ctl,pcid_property->insert_cid_hdr_en ?255:0);
    cmd = DRV_IOW(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_nh_ctl));

    cmd = DRV_IOR(IpeFwdCategoryCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cid_glb_ctl));
    SetIpeFwdCategoryCtl(V,globalSrcCategoryIdValid_f,&cid_glb_ctl,pcid_property->global_cid_en?1:0);
    SetIpeFwdCategoryCtl(V,globalSrcCategoryId_f,&cid_glb_ctl,pcid_property->global_cid);
    SetIpeFwdCategoryCtl(V,categoryIdPairLookupEn_f,&cid_glb_ctl,pcid_property->cid_pair_en?1:0);
    if(pcid_property->reassign_cid_pri_en)
    {
       if(!pcid_property->is_dst_cid)
       {
          step = IpeFwdCategoryCtl_gSrcCidPri_1_pktSrcCategoryIdPri_f -
                        IpeFwdCategoryCtl_gSrcCidPri_0_pktSrcCategoryIdPri_f;
          for(loop = 0; loop < 6 ;loop++)
          {
             SetIpeFwdCategoryCtl(V, gSrcCidPri_0_pktSrcCategoryIdPri_f + step *loop,     &cid_glb_ctl, pcid_property->pkt_cid_pri);
             SetIpeFwdCategoryCtl(V, gSrcCidPri_0_i2eSrcCategoryIdPri_f + step *loop,     &cid_glb_ctl, pcid_property->iloop_cid_pri);
             SetIpeFwdCategoryCtl(V, gSrcCidPri_0_globalSrcCategoryIdPri_f + step *loop,  &cid_glb_ctl, pcid_property->global_cid_pri);
             SetIpeFwdCategoryCtl(V, gSrcCidPri_0_staticSrcCategoryIdPri_f + step *loop,  &cid_glb_ctl, pcid_property->flow_table_cid_pri);
             SetIpeFwdCategoryCtl(V, gSrcCidPri_0_dynamicSrcCategoryIdPri_f + step *loop, &cid_glb_ctl, pcid_property->fwd_table_cid_pri);
             SetIpeFwdCategoryCtl(V, gSrcCidPri_0_ifSrcCategoryIdPri_f + step *loop,      &cid_glb_ctl, pcid_property->if_cid_pri);
             SetIpeFwdCategoryCtl(V, gSrcCidPri_0_defaultSrcCategoryIdPri_f+ step *loop, &cid_glb_ctl, pcid_property->default_cid_pri);
          }
       }
       else
       {
         SetIpeFwdCategoryCtl(V, gDstCidPri_defaultDstCategoryIdPri_f, &cid_glb_ctl, pcid_property->default_cid_pri);
         SetIpeFwdCategoryCtl(V, gDstCidPri_dsflowDstCategoryIdPri_f,  &cid_glb_ctl, pcid_property->flow_table_cid_pri);
         SetIpeFwdCategoryCtl(V, gDstCidPri_dynamicDstCategoryIdPri_f, &cid_glb_ctl, pcid_property->fwd_table_cid_pri);
         SetIpeFwdCategoryCtl(V, gDstCidPri_staticDstCategoryIdPri_f,  &cid_glb_ctl, pcid_property->flow_table_cid_pri);
       }

    }

  cmd = DRV_IOW(IpeFwdCategoryCtl_t, DRV_ENTRY_FLAG);
  CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cid_glb_ctl));

  return CTC_E_NONE;
}

STATIC int32
_sys_usw_get_glb_cid_property(uint8 lchip, ctc_global_cid_property_t* pcid_property)
{
	ParserEthernetCtl_m     cid_parser_ctl;
	IpeFwdCategoryCtl_m      cid_glb_ctl;
	EpeNextHopCtl_m     epe_nh_ctl;
	uint32 cmd = 0;
	uint32 field_val = 0;
	CTC_PTR_VALID_CHECK(pcid_property);

	cmd = DRV_IOR(ParserEthernetCtl_t, DRV_ENTRY_FLAG);
	CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cid_parser_ctl));
	pcid_property->cmd_parser_en = GetParserEthernetCtl(V,supportMetaDataHeader_f,&cid_parser_ctl);
	pcid_property->cmd_ethtype = GetParserEthernetCtl(V,metaDataEtherType_f,&cid_parser_ctl);

	cmd = DRV_IOR(IpeFwdCategoryCtl_t, DRV_ENTRY_FLAG);
	CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cid_glb_ctl));
	pcid_property->global_cid_en = GetIpeFwdCategoryCtl(V,globalSrcCategoryIdValid_f,&cid_glb_ctl);
	pcid_property->global_cid = GetIpeFwdCategoryCtl(V,globalSrcCategoryId_f,&cid_glb_ctl);
	pcid_property->cid_pair_en = GetIpeFwdCategoryCtl(V,categoryIdPairLookupEn_f,&cid_glb_ctl);
	cmd = DRV_IOR(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
	CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_nh_ctl));
	pcid_property->insert_cid_hdr_en = GetEpeNextHopCtl(V,bypassInsertCidThre_f,&epe_nh_ctl)?1:0;

	cmd = DRV_IOR(EpeHeaderEditCtl_t, EpeHeaderEditCtl_extCidValid_f);
	CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
	pcid_property->cross_chip_cid_en = field_val;

	if(!pcid_property->is_dst_cid)
	{
	  pcid_property->pkt_cid_pri = GetIpeFwdCategoryCtl(V, gSrcCidPri_0_pktSrcCategoryIdPri_f,&cid_glb_ctl);
	  pcid_property->iloop_cid_pri = GetIpeFwdCategoryCtl(V, gSrcCidPri_0_i2eSrcCategoryIdPri_f,&cid_glb_ctl);
	  pcid_property->global_cid_pri = GetIpeFwdCategoryCtl(V, gSrcCidPri_0_globalSrcCategoryIdPri_f, &cid_glb_ctl);
	  pcid_property->flow_table_cid_pri = GetIpeFwdCategoryCtl(V, gSrcCidPri_0_staticSrcCategoryIdPri_f,&cid_glb_ctl);
	  pcid_property->fwd_table_cid_pri = GetIpeFwdCategoryCtl(V, gSrcCidPri_0_dynamicSrcCategoryIdPri_f,&cid_glb_ctl);
	  pcid_property->if_cid_pri = GetIpeFwdCategoryCtl(V, gSrcCidPri_0_ifSrcCategoryIdPri_f,&cid_glb_ctl);
	  pcid_property->default_cid_pri = GetIpeFwdCategoryCtl(V, gSrcCidPri_0_defaultSrcCategoryIdPri_f, &cid_glb_ctl);
	}
	else
	{
	  pcid_property->default_cid_pri = GetIpeFwdCategoryCtl(V, gDstCidPri_defaultDstCategoryIdPri_f, &cid_glb_ctl);
	  pcid_property->flow_table_cid_pri = GetIpeFwdCategoryCtl(V, gDstCidPri_dsflowDstCategoryIdPri_f,&cid_glb_ctl);
	  pcid_property->fwd_table_cid_pri = GetIpeFwdCategoryCtl(V, gDstCidPri_dynamicDstCategoryIdPri_f,&cid_glb_ctl);
	  pcid_property->flow_table_cid_pri = GetIpeFwdCategoryCtl(V, gDstCidPri_staticDstCategoryIdPri_f,&cid_glb_ctl);
	}

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_set_glb_ipmc_property(uint8 lchip, ctc_global_ipmc_property_t* pipmc_property)
{
    uint32 cmd = 0;
    uint32 value = 0;

    CTC_PTR_VALID_CHECK(pipmc_property);
    CTC_MAX_VALUE_CHECK(pipmc_property->ip_l2mc_mode, 1);
    value = pipmc_property->ip_l2mc_mode;
    cmd = DRV_IOW(FibEngineLookupCtl_t, FibEngineLookupCtl_l2mcMacIpParallel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_set_glb_acl_lkup_property(uint8 lchip, ctc_acl_property_t* acl_prop)
{
    uint32 cmd = 0;
    uint8  lkup_type = 0;
    uint8  gport_type = 0;
    uint8  step = 0;
    uint8  prio = 0;
    IpePreLookupAclCtl_m ipe_pre_acl_ctl;
    EpeAclQosCtl_m       epe_acl_ctl;

    CTC_PTR_VALID_CHECK(acl_prop);
    SYS_USW_GLOBAL_ACL_PROP_CHK(acl_prop);

    if(CTC_ACL_TCAM_LKUP_TYPE_VLAN == acl_prop->tcam_lkup_type)
    {
        return CTC_E_NOT_SUPPORT;
    }
    lkup_type = sys_usw_map_acl_tcam_lkup_type(lchip, acl_prop->tcam_lkup_type);
    prio = acl_prop->acl_priority;
    if(CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_HASH_LKUP))
    {
        return CTC_E_NOT_SUPPORT;
    }

    if(CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP))
    {
        gport_type = DRV_FLOWPORTTYPE_BITMAP;
    }
    else if(CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_METADATA))
    {
        gport_type = DRV_FLOWPORTTYPE_METADATA;
    }
    else if(CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT))
    {
        gport_type = DRV_FLOWPORTTYPE_LPORT;
    }
    else
    {
        gport_type = DRV_FLOWPORTTYPE_GPORT;
    }

    if(CTC_INGRESS == acl_prop->direction )
    {
        CTC_MAX_VALUE_CHECK(prio, MCHIP_CAP(SYS_CAP_ACL_INGRESS_ACL_LKUP)-1);
        sal_memset(&ipe_pre_acl_ctl, 0, sizeof(ipe_pre_acl_ctl));
        cmd = DRV_IOR(IpePreLookupAclCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_pre_acl_ctl));

        step = IpePreLookupAclCtl_gGlbAcl_1_aclEnable_f - IpePreLookupAclCtl_gGlbAcl_0_aclEnable_f;
        if(acl_prop->acl_en)
        {
            SetIpePreLookupAclCtl(V, gGlbAcl_0_aclEnable_f + step*prio,         &ipe_pre_acl_ctl, acl_prop->acl_en? 1 : 0);
            SetIpePreLookupAclCtl(V, gGlbAcl_0_aclLabel_f + step*prio,          &ipe_pre_acl_ctl, acl_prop->class_id);
            SetIpePreLookupAclCtl(V, gGlbAcl_0_aclLookupType_f + step*prio,     &ipe_pre_acl_ctl, lkup_type);
            SetIpePreLookupAclCtl(V, gGlbAcl_0_aclGlobalPortType_f + step*prio, &ipe_pre_acl_ctl, gport_type);
            SetIpePreLookupAclCtl(V, gGlbAcl_0_aclUseCapwapInfo_f + step*prio,  &ipe_pre_acl_ctl,
                                                CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_WLAN)?1:0);
            SetIpePreLookupAclCtl(V, gGlbAcl_0_aclUsePIVlan_f + step*prio,      &ipe_pre_acl_ctl,
                                                CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN)?1:0);
        }
        else
        {
            SetIpePreLookupAclCtl(V, gGlbAcl_0_aclEnable_f + step*prio,         &ipe_pre_acl_ctl, 0);
            SetIpePreLookupAclCtl(V, gGlbAcl_0_aclLabel_f + step*prio,          &ipe_pre_acl_ctl, 0);
            SetIpePreLookupAclCtl(V, gGlbAcl_0_aclLookupType_f + step*prio,     &ipe_pre_acl_ctl, 0);
            SetIpePreLookupAclCtl(V, gGlbAcl_0_aclGlobalPortType_f + step*prio, &ipe_pre_acl_ctl, 0);
            SetIpePreLookupAclCtl(V, gGlbAcl_0_aclUseCapwapInfo_f + step*prio,  &ipe_pre_acl_ctl, 0);
            SetIpePreLookupAclCtl(V, gGlbAcl_0_aclUsePIVlan_f + step*prio,      &ipe_pre_acl_ctl, 0);
        }

        cmd = DRV_IOW(IpePreLookupAclCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_pre_acl_ctl));
    }

    else if(CTC_EGRESS == acl_prop->direction )
    {
       CTC_MAX_VALUE_CHECK(prio, MCHIP_CAP(SYS_CAP_ACL_EGRESS_LKUP)-1);
       sal_memset(&epe_acl_ctl, 0, sizeof(epe_acl_ctl));
        cmd = DRV_IOR(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_ctl));

        step = EpeAclQosCtl_gGlbAcl_1_aclEnable_f - EpeAclQosCtl_gGlbAcl_0_aclEnable_f;
        if(acl_prop->acl_en)
        {
            SetEpeAclQosCtl(V, gGlbAcl_0_aclEnable_f + step*prio,         &epe_acl_ctl, acl_prop->acl_en? 1 : 0);
            SetEpeAclQosCtl(V, gGlbAcl_0_aclLabel_f + step*prio,          &epe_acl_ctl, acl_prop->class_id);
            SetEpeAclQosCtl(V, gGlbAcl_0_aclLookupType_f + step*prio,     &epe_acl_ctl, lkup_type);
            SetEpeAclQosCtl(V, gGlbAcl_0_aclGlobalPortType_f + step*prio, &epe_acl_ctl, gport_type);
            SetEpeAclQosCtl(V, gGlbAcl_0_aclUsePIVlan_f + step*prio,      &epe_acl_ctl,
                                                CTC_FLAG_ISSET(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN)?1:0);
        }
        else
        {
            SetEpeAclQosCtl(V, gGlbAcl_0_aclEnable_f + step*prio,         &epe_acl_ctl, 0);
            SetEpeAclQosCtl(V, gGlbAcl_0_aclLabel_f + step*prio,          &epe_acl_ctl, 0);
            SetEpeAclQosCtl(V, gGlbAcl_0_aclLookupType_f + step*prio,     &epe_acl_ctl, 0);
            SetEpeAclQosCtl(V, gGlbAcl_0_aclGlobalPortType_f + step*prio, &epe_acl_ctl, 0);
            SetEpeAclQosCtl(V, gGlbAcl_0_aclUsePIVlan_f + step*prio,      &epe_acl_ctl, 0);
        }

        cmd = DRV_IOW(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_ctl));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_get_glb_acl_lkup_property(uint8 lchip, ctc_acl_property_t* acl_prop)
{
    uint32 cmd = 0;
    uint8  lkup_type = 0;
    uint8  gport_type = 0;
    uint8  step = 0;
    uint8  prio = 0;
    uint8  use_mapped_vlan = 0;
    uint8  use_wlan = 0;
    IpePreLookupAclCtl_m ipe_pre_acl_ctl;
    EpeAclQosCtl_m       epe_acl_ctl;

    CTC_PTR_VALID_CHECK(acl_prop);
    CTC_MAX_VALUE_CHECK(acl_prop->direction, CTC_EGRESS);
    prio = acl_prop->acl_priority;
    if(CTC_INGRESS == acl_prop->direction)
    {
        CTC_MAX_VALUE_CHECK(prio, MCHIP_CAP(SYS_CAP_ACL_INGRESS_ACL_LKUP)-1);
        sal_memset(&ipe_pre_acl_ctl, 0, sizeof(ipe_pre_acl_ctl));
        cmd = DRV_IOR(IpePreLookupAclCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_pre_acl_ctl));

        step = IpePreLookupAclCtl_gGlbAcl_1_aclEnable_f - IpePreLookupAclCtl_gGlbAcl_0_aclEnable_f;
        acl_prop->acl_en = GetIpePreLookupAclCtl(V, gGlbAcl_0_aclEnable_f + step*prio,  &ipe_pre_acl_ctl);
        acl_prop->class_id = GetIpePreLookupAclCtl(V, gGlbAcl_0_aclLabel_f + step*prio, &ipe_pre_acl_ctl);
        lkup_type = GetIpePreLookupAclCtl(V, gGlbAcl_0_aclLookupType_f + step*prio,     &ipe_pre_acl_ctl);
        gport_type = GetIpePreLookupAclCtl(V, gGlbAcl_0_aclGlobalPortType_f + step*prio, &ipe_pre_acl_ctl);
        use_wlan = GetIpePreLookupAclCtl(V, gGlbAcl_0_aclUseCapwapInfo_f + step*prio,  &ipe_pre_acl_ctl);
        use_mapped_vlan = GetIpePreLookupAclCtl(V, gGlbAcl_0_aclUsePIVlan_f + step*prio, &ipe_pre_acl_ctl);
    }
    else  if(CTC_EGRESS == acl_prop->direction)
    {
        CTC_MAX_VALUE_CHECK(prio, MCHIP_CAP(SYS_CAP_ACL_EGRESS_LKUP)-1);
        sal_memset(&epe_acl_ctl, 0, sizeof(epe_acl_ctl));
        cmd = DRV_IOR(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_ctl));

        step = EpeAclQosCtl_gGlbAcl_1_aclEnable_f - EpeAclQosCtl_gGlbAcl_0_aclEnable_f;
        acl_prop->acl_en = GetEpeAclQosCtl(V, gGlbAcl_0_aclEnable_f + step*prio,  &epe_acl_ctl);
        acl_prop->class_id = GetEpeAclQosCtl(V, gGlbAcl_0_aclLabel_f + step*prio, &epe_acl_ctl);
        lkup_type = GetEpeAclQosCtl(V, gGlbAcl_0_aclLookupType_f + step*prio,     &epe_acl_ctl);
        gport_type = GetEpeAclQosCtl(V, gGlbAcl_0_aclGlobalPortType_f + step*prio, &epe_acl_ctl);
        use_mapped_vlan = GetEpeAclQosCtl(V, gGlbAcl_0_aclUsePIVlan_f + step*prio, &epe_acl_ctl);
    }

    acl_prop->tcam_lkup_type  = sys_usw_unmap_acl_tcam_lkup_type(lchip, lkup_type);
    if(use_mapped_vlan)
    {
        CTC_SET_FLAG(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN);
    }
    if(use_wlan)
    {
        CTC_SET_FLAG(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_WLAN);
    }

    switch(gport_type)
    {
        case DRV_FLOWPORTTYPE_BITMAP:
            CTC_SET_FLAG(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP);
            break;
        case DRV_FLOWPORTTYPE_LPORT:
            CTC_SET_FLAG(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT);
            break;
        case DRV_FLOWPORTTYPE_METADATA:
            CTC_SET_FLAG(acl_prop->flag, CTC_ACL_PROP_FLAG_USE_METADATA);
            break;
        default:
            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_get_glb_ipmc_property(uint8 lchip, ctc_global_ipmc_property_t* ipmc_prop)
{
    uint32 cmd = 0;
    uint32 value = 0;

    cmd = DRV_IOR(FibEngineLookupCtl_t, FibEngineLookupCtl_l2mcMacIpParallel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    ipmc_prop->ip_l2mc_mode = value?1:0;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_set_glb_flow_property(uint8 lchip, ctc_global_flow_property_t* p_flow_prop)
{
    uint32 field_val = 0;
    uint32 cmd = 0;
    IpeFwdCategoryCtl_m ipe_fwd_cid_ctl;

    sal_memset(&ipe_fwd_cid_ctl, 0, sizeof(IpeFwdCategoryCtl_m));

    CTC_MAX_VALUE_CHECK(p_flow_prop->igs_vlan_range_mode, CTC_GLOBAL_VLAN_RANGE_MODE_MAX-1);
    CTC_MAX_VALUE_CHECK(p_flow_prop->egs_vlan_range_mode, CTC_GLOBAL_VLAN_RANGE_MODE_MAX-1);

    if (CTC_GLOBAL_VLAN_RANGE_MODE_SCL == p_flow_prop->igs_vlan_range_mode)
    {
        field_val = 0;
    }
    else if (CTC_GLOBAL_VLAN_RANGE_MODE_ACL == p_flow_prop->igs_vlan_range_mode)
    {
        field_val = 1;
    }
    else if (CTC_GLOBAL_VLAN_RANGE_MODE_SHARE == p_flow_prop->igs_vlan_range_mode)
    {
        field_val = 2;
    }
    cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_vlanRangeMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    if (CTC_GLOBAL_VLAN_RANGE_MODE_SCL == p_flow_prop->egs_vlan_range_mode)
    {
        field_val = 0;
    }
    else if (CTC_GLOBAL_VLAN_RANGE_MODE_ACL == p_flow_prop->egs_vlan_range_mode)
    {
        field_val = 1;
    }
    else if (CTC_GLOBAL_VLAN_RANGE_MODE_SHARE == p_flow_prop->egs_vlan_range_mode)
    {
        field_val = 2;
    }
    cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_vlanRangeMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_get_glb_flow_property(uint8 lchip, ctc_global_flow_property_t* p_flow_prop)
{
    uint32 field_val = 0;
    uint32 cmd = 0;
    IpeFwdCategoryCtl_m ipe_fwd_cid_ctl;

    sal_memset(&ipe_fwd_cid_ctl, 0, sizeof(IpeFwdCategoryCtl_m));

    cmd = DRV_IOR(IpeUserIdCtl_t, IpeUserIdCtl_vlanRangeMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    if (0 == field_val)
    {
        p_flow_prop->igs_vlan_range_mode = CTC_GLOBAL_VLAN_RANGE_MODE_SCL;
    }
    else if (1 == field_val)
    {
        p_flow_prop->igs_vlan_range_mode = CTC_GLOBAL_VLAN_RANGE_MODE_ACL;
    }
    else if (2 == field_val)
    {
        p_flow_prop->igs_vlan_range_mode = CTC_GLOBAL_VLAN_RANGE_MODE_SHARE;
    }

    cmd = DRV_IOR(EpeNextHopCtl_t, EpeNextHopCtl_vlanRangeMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    if (0 == field_val)
    {
        p_flow_prop->egs_vlan_range_mode = CTC_GLOBAL_VLAN_RANGE_MODE_SCL;
    }
    else if (1 == field_val)
    {
        p_flow_prop->egs_vlan_range_mode = CTC_GLOBAL_VLAN_RANGE_MODE_ACL;
    }
    else if (2 == field_val)
    {
        p_flow_prop->egs_vlan_range_mode = CTC_GLOBAL_VLAN_RANGE_MODE_SHARE;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_global_ctl_set_arp_check_en(uint8 lchip, ctc_global_control_type_t type, uint32* value)
{
    uint32 cmd = 0;
    switch (type)
    {
        case CTC_GLOBAL_ARP_MACDA_CHECK_EN:
            cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_arpDestMacCheckEn_f);
            break;
        case CTC_GLOBAL_ARP_MACSA_CHECK_EN:
            cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_arpSrcMacCheckEn_f);
            break;
        case CTC_GLOBAL_ARP_IP_CHECK_EN:
            cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_arpIpCheckEn_f);
            break;
        case CTC_GLOBAL_ARP_CHECK_FAIL_TO_CPU:
            cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_arpCheckExceptionEn_f);
            break;
        default:
            return CTC_E_NOT_SUPPORT;
    }
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, value));

    return CTC_E_NONE;

}


STATIC int32
_sys_usw_global_ctl_get_arp_check_en(uint8 lchip, ctc_global_control_type_t type, uint32* value)
{
    uint32 cmd = 0;

    switch (type)
    {
        case CTC_GLOBAL_ARP_MACDA_CHECK_EN:
            cmd = DRV_IOR(IpeIntfMapperCtl_t, IpeIntfMapperCtl_arpDestMacCheckEn_f);
            break;
        case CTC_GLOBAL_ARP_MACSA_CHECK_EN:
            cmd = DRV_IOR(IpeIntfMapperCtl_t, IpeIntfMapperCtl_arpSrcMacCheckEn_f);
            break;
        case CTC_GLOBAL_ARP_IP_CHECK_EN:
            cmd = DRV_IOR(IpeIntfMapperCtl_t, IpeIntfMapperCtl_arpIpCheckEn_f);
            break;
        case CTC_GLOBAL_ARP_CHECK_FAIL_TO_CPU:
            cmd = DRV_IOR(IpeIntfMapperCtl_t, IpeIntfMapperCtl_arpCheckExceptionEn_f);
            break;
        default:
            return CTC_E_NOT_SUPPORT;
    }
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, value));

    return CTC_E_NONE;

}
/*igmg use fatal exception to cpu by default*/
STATIC int32
_sys_usw_global_ctl_set_igmp_mode(uint8 lchip, ctc_global_control_type_t type, uint32* value)
{

    uint32 cmd = 0;
    uint32 dsfwd_offset = 0;
    uint32 dest_map = 0;
    uint32 nexthop_ptr = 0;
    uint8 gchip = 0;
    uint8 sub_queue_id = 64;  /*CTC_PKT_CPU_REASON_IGMP_SNOOPING default to 64*/
    uint32 field_val = (*value == CTC_GLOBAL_IGMP_SNOOPING_MODE_2) ?1 :0;
    CTC_MAX_VALUE_CHECK(*value, CTC_GLOBAL_IGMP_SNOOPING_MODE_2);

    cmd = DRV_IOW(IpeRouteCtl_t, IpeRouteCtl_igmpSnoopedMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(IpeBridgeCtl_t, IpeBridgeCtl_igmpSnoopedDonotEscape_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(IpeRouteCtl_t, IpeRouteCtl_nonBrgPktIgmpSnoopedExceptionEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(IpeRouteCtl_t, IpeRouteCtl_nonBrgPktIgmpSnoopedDiscardEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    /*MODE_0 :igmp will be identified as fatal exception,send fatal exception to CPU--can't do copp & vlan flooding */
    /*MODE_1 :igmp will be identified as fatal exception,must use acl to classify it as normal exception--can do copp */
    /*MODE_2: igmp from vlan if will be identified as normal exception, -- can do vlan flooding & copp
              igmp from phy if/sub if must enable bridge */
    dest_map = (*value == CTC_GLOBAL_IGMP_SNOOPING_MODE_0) ?
         SYS_ENCODE_EXCP_DESTMAP(gchip, sub_queue_id) : SYS_ENCODE_DESTMAP(gchip, SYS_RSV_PORT_DROP_ID);
    nexthop_ptr = CTC_PKT_CPU_REASON_BUILD_NHPTR(CTC_PKT_CPU_REASON_IGMP_SNOOPING, 0);
    CTC_ERROR_RETURN(sys_usw_nh_get_fatal_excp_dsnh_offset(lchip, &dsfwd_offset));
    dsfwd_offset = dsfwd_offset + 12 * 2; /* SYS_FATAL_EXCP_IGMP_SNOOPED_PACKET   = 12 */
    CTC_ERROR_RETURN(sys_usw_nh_update_dsfwd(lchip, &dsfwd_offset, dest_map, nexthop_ptr, 0, 0, 1));

    return CTC_E_NONE;

}
STATIC int32
_sys_usw_global_ctl_get_igmp_mode(uint8 lchip, ctc_global_control_type_t type, uint32* value)
{
   uint32 cmd = 0;
   uint32 field_val = 0;
   uint32 dsfwd_offset = 0;
   DsFwd_m dsfwd;
   DsFwdHalf_m dsfwd_half;
   uint32 dest_map = 0;

   cmd = DRV_IOR(IpeRouteCtl_t, IpeRouteCtl_igmpSnoopedMode_f);
   CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

   CTC_ERROR_RETURN(sys_usw_nh_get_fatal_excp_dsnh_offset(lchip, &dsfwd_offset));
   /* SYS_FATAL_EXCP_IGMP_SNOOPED_PACKET   = 12 */
   dsfwd_offset = dsfwd_offset + 12 * 2;
   cmd = DRV_IOR(DsFwd_t, DRV_ENTRY_FLAG);
   DRV_IOCTL(lchip, dsfwd_offset / 2, cmd, &dsfwd);
   if (dsfwd_offset % 2 == 0)
   {
       GetDsFwdDualHalf(A, g_0_dsFwdHalf_f,   &dsfwd,   &dsfwd_half);
   }
   else
   {
       GetDsFwdDualHalf(A, g_1_dsFwdHalf_f,   &dsfwd,   &dsfwd_half);
   }
   dest_map = GetDsFwdHalf(V, destMap_f, &dsfwd_half);
   if (dest_map >> 16 && (field_val == 0) )  /*SYS_ENCODE_EXCP_DESTMAP*/
   {  /*to cpu*/
       *value  = CTC_GLOBAL_IGMP_SNOOPING_MODE_0;
   }
   else if(field_val == 1)
   {
       *value  = CTC_GLOBAL_IGMP_SNOOPING_MODE_2;
   }
   else
   {
       *value  = CTC_GLOBAL_IGMP_SNOOPING_MODE_1;
   }

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_set_dump_db(uint8 lchip, ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;
    sal_file_t p_file = NULL;
    char cfg_file[300] = {0};
    char line_buf[121] = {0};
    uint16 str_len = 0;
    uint8 feature = 0;

    CTC_PTR_VALID_CHECK(p_dump_param);
    LCHIP_CHECK(lchip);
    if(sal_strlen(p_dump_param->file) != 0)
    {
        sal_sprintf(cfg_file, "%s%s", p_dump_param->file, ".txt");
        p_file = sal_fopen(cfg_file, "wb+");

        if (NULL == p_file)
        {
            SYS_REGISTER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR," Store file: %s failed!\n\n", cfg_file);
            goto CTC_DUMP_DB_END;
        }
    }

    str_len += sal_sprintf(line_buf + str_len, "%s", "---------------------------------------------------**** lchip: ");
    str_len += sal_sprintf(line_buf + str_len, "%d", lchip);
    str_len += sal_sprintf(line_buf + str_len, "%s", " ****---------------------------------------------------");
    SYS_DUMP_DB_LOG(p_file, "%s\n", line_buf);
    SYS_DUMP_DB_LOG(p_file, "%s\n", "************************************************* CTC SDK DEBUG DUMP ***************************************************");
    SYS_DUMP_DB_LOG(p_file, "%s %s\n", "SDK Version       :", CTC_SDK_VERSION_STR);
    SYS_DUMP_DB_LOG(p_file, "%s %s\n", "SDK Release Date  :", CTC_SDK_RELEASE_DATE);
    SYS_DUMP_DB_LOG(p_file, "%s %s\n", "SDK Copyright Time:", CTC_SDK_COPYRIGHT_TIME);
    SYS_DUMP_DB_LOG(p_file, "%s %s\n", "Chip Series       :",g_ctcs_api_en ? ctcs_get_chip_name(lchip):ctc_get_chip_name());
    SYS_DUMP_DB_LOG(p_file, "%s %s %s\n", "Compile time      :",__DATE__,__TIME__);
    SYS_DUMP_DB_LOG(p_file, "%s\n", "************************************************* CTC SDK DEBUG DUMP ***************************************************");
    for (feature = 0; feature < CTC_FEATURE_MAX; feature++)
    {
        if (CTC_BMP_ISSET(p_dump_param->bit_map, feature) && p_usw_chip_master[lchip]->dump_cb[feature])
        {
            CTC_ERROR_GOTO(p_usw_chip_master[lchip]->dump_cb[feature](lchip, p_file, p_dump_param),ret,CTC_DUMP_DB_END);
        }
    }

CTC_DUMP_DB_END:
    if (NULL != p_file)
    {
        sal_fclose(p_file);
    }
    return CTC_E_NONE;
}

int32
_sys_usw_set_warmboot_sync(uint8 lchip)
{
    uint8 i;
    /*sync up all data to memory */
    CTC_ERROR_RETURN(sys_usw_chip_wb_sync(lchip));
    CTC_ERROR_RETURN(sys_usw_datapath_wb_sync(lchip));
    CTC_ERROR_RETURN(sys_usw_mac_wb_sync(lchip));
    CTC_ERROR_RETURN(sys_usw_ftm_wb_sync(lchip));
    CTC_ERROR_RETURN(sys_usw_internal_port_wb_sync(lchip));
    CTC_ERROR_RETURN(sys_usw_mac_stats_wb_sync(lchip));

    for (i = 0; i < CTC_FEATURE_MAX; i++)
    {
        if (p_usw_register_master[lchip]->wb_sync_cb[i])
        {
            CTC_ERROR_RETURN(p_usw_register_master[lchip]->wb_sync_cb[i](lchip));
        }
    }

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_register_hash_make_cethertype(sys_register_cethertype_t* pa)
{
    uint32 size = sizeof(uint16);
    return ctc_hash_caculate(size, &pa->ether_type);
}

/*acl cethertype spool*/
STATIC bool
_sys_usw_register_hash_compare_cethertype(sys_register_cethertype_t* pa0,
                                           sys_register_cethertype_t* pa1)
{
    if (!pa0 || !pa1)
    {
        return FALSE;
    }

    if (pa0->ether_type == pa1->ether_type)
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_sys_usw_register_build_cethertype_index(sys_register_cethertype_t* pa, uint8* p_lchip)
{
    sys_usw_opf_t opf;
    uint32               value_32 = 0;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_type  = p_usw_register_master[*p_lchip]->opf_type_cethertype;
    if (CTC_WB_ENABLE && CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, pa->cethertype_index));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        pa->cethertype_index = value_32 & 0x3F;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_register_free_cethertype_index(sys_register_cethertype_t* pa, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_type = p_usw_register_master[*p_lchip]->opf_type_cethertype;

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, pa->cethertype_index));

    return CTC_E_NONE;
}

int32
_sys_usw_register_cethertype_spool_init(uint8 lchip)
{
    uint32 cethertype_num = 0;
    ctc_spool_t spool;
    sys_usw_opf_t opf;

    sal_memset(&opf, 0, sizeof(opf));
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, EtherTypeCompressCam_t, &cethertype_num));
    if(cethertype_num)
    {
        CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &p_usw_register_master[lchip]->opf_type_cethertype, 1, "opf-register-cethertype"));
        opf.pool_index = 0;
        opf.pool_type  = p_usw_register_master[lchip]->opf_type_cethertype;
        CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 0, cethertype_num));
    }

    /*spool for cethertype*/
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = cethertype_num;
    spool.max_count = cethertype_num;
    spool.user_data_size = sizeof(sys_register_cethertype_t);
    spool.spool_key = (hash_key_fn) _sys_usw_register_hash_make_cethertype;
    spool.spool_cmp = (hash_cmp_fn) _sys_usw_register_hash_compare_cethertype;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_register_build_cethertype_index;
    spool.spool_free = (spool_free_fn)_sys_usw_register_free_cethertype_index;
    p_usw_register_master[lchip]->cethertype_spool = ctc_spool_create(&spool);

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_register_hash_make_tcat_prof(sys_usw_register_tcat_prof_t* pa)
{
    return ctc_hash_caculate(sizeof(pa->length) + sizeof(pa->shift), &pa->shift);
}

/*acl cethertype spool*/
STATIC bool
_sys_usw_register_hash_compare_tcat_prof(sys_usw_register_tcat_prof_t* pa0,
                                           sys_usw_register_tcat_prof_t* pa1)
{
    if (!pa0 || !pa1)
    {
        return FALSE;
    }

    if ((pa0->shift == pa1->shift) && (pa0->length == pa1->length))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_sys_usw_register_build_tcat_prof_index(sys_usw_register_tcat_prof_t* pa, uint8* p_lchip)
{
    sys_usw_opf_t opf;
    uint32               value_32 = 0;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_type  = p_usw_register_master[*p_lchip]->opf_type_tcat_prof;
    if (CTC_WB_ENABLE && CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, pa->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        pa->profile_id = value_32 & 0xF;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_register_free_tcat_prof_index(sys_usw_register_tcat_prof_t* pa, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_type = p_usw_register_master[*p_lchip]->opf_type_tcat_prof;

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, pa->profile_id));

    return CTC_E_NONE;
}

/*Truncation profile spool init*/
int32
_sys_usw_register_tcat_prof_spool_init(uint8 lchip)
{
    uint32 tcat_prof_num = 0;
    ctc_spool_t spool;
    sys_usw_opf_t opf;
    int32  ret = 0;

    sal_memset(&opf, 0, sizeof(opf));
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsTruncationProfile_t, &tcat_prof_num));

    if(tcat_prof_num)
    {
        CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &p_usw_register_master[lchip]->opf_type_tcat_prof, 1, "opf-register-truncation-profile"));
        opf.pool_index = 0;
        opf.pool_type  = p_usw_register_master[lchip]->opf_type_tcat_prof;
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, tcat_prof_num - 1), ret, error_return);
    }

    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = tcat_prof_num - 1;
    spool.max_count = tcat_prof_num - 1;
    spool.user_data_size = sizeof(sys_usw_register_tcat_prof_t);
    spool.spool_key = (hash_key_fn) _sys_usw_register_hash_make_tcat_prof;
    spool.spool_cmp = (hash_cmp_fn) _sys_usw_register_hash_compare_tcat_prof;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_register_build_tcat_prof_index;
    spool.spool_free = (spool_free_fn)_sys_usw_register_free_tcat_prof_index;
    p_usw_register_master[lchip]->tcat_prof_spool = ctc_spool_create(&spool);
    if (NULL == p_usw_register_master[lchip]->tcat_prof_spool)
    {
        ret = CTC_E_NO_MEMORY;
        goto error_return;
    }

    return CTC_E_NONE;

error_return:
    sys_usw_opf_deinit(lchip, p_usw_register_master[lchip]->opf_type_tcat_prof);
    return ret;
}


int32
sys_usw_register_wb_sync(uint8 lchip)
{
    uint8 loop = 1;
    uint16 length = 0;
    int32 ret = CTC_E_NONE;
    uint32 max_entry_cnt = 0;
    uint8  shift = 0;
    uint32 cmd = 0;
    sys_wb_register_master_t* p_wb_register_master = NULL;
    ctc_wb_data_t wb_data;
    sys_wb_register_trunc_pool_t wb_trunc_pool;
    sys_usw_register_tcat_prof_t tcat_prof_new;

    DsTruncationProfile_m truncation_prof;

    sal_memset(&wb_trunc_pool, 0, sizeof(sys_wb_register_trunc_pool_t));

    /*sync up master*/
    wb_data.buffer = mem_malloc(MEM_SYSTEM_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    p_wb_register_master = (sys_wb_register_master_t*)wb_data.buffer;
    p_wb_register_master->lchip = lchip;
    p_wb_register_master->version = SYS_WB_VERSION_REGISTER;
    p_wb_register_master->oam_coexist_mode = p_usw_register_master[lchip]->tpoam_vpws_coexist;

    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_register_master_t, CTC_FEATURE_REGISTER, SYS_WB_APPID_REGISTER_SUBID_MASTER);
    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /*sync TruncationProfile*/
    /*for different kinds of data stored, the wb_data must be reinited*/
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_register_trunc_pool_t, CTC_FEATURE_REGISTER, SYS_WB_APPID_REGISTER_SUBID_TRUNC_POOL);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data.key_len + wb_data.data_len);

    cmd = DRV_IOR(DsTruncationProfile_t, DRV_ENTRY_FLAG);
    do
    {
        sal_memset(&truncation_prof, 0, sizeof(truncation_prof));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, loop, cmd, &truncation_prof));

        shift = GetDsTruncationProfile(V, lengthShift_f, &truncation_prof);
        length = GetDsTruncationProfile(V, length_f, &truncation_prof);

        if ((0 == shift) && (0 == length))
        {
            continue;
        }

        sal_memset(&tcat_prof_new, 0, sizeof(tcat_prof_new));
        tcat_prof_new.shift = shift;
        tcat_prof_new.length = length;

        wb_trunc_pool.profile_id = loop;
        wb_trunc_pool.ref_cnt = ctc_spool_get_refcnt(p_usw_register_master[lchip]->tcat_prof_spool, &tcat_prof_new);
        if (0 == wb_trunc_pool.ref_cnt)
        {
            ret = CTC_E_NOT_EXIST;
            goto done;
        }

        sal_memcpy( (uint8*)wb_data.buffer + wb_data.valid_cnt * sizeof(sys_wb_register_trunc_pool_t), (uint8*)&wb_trunc_pool, sizeof(sys_wb_register_trunc_pool_t));

        if (++wb_data.valid_cnt == max_entry_cnt)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
            wb_data.valid_cnt = 0;
        }
    }while ((++loop) <= 15);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }
    return ret;
}

int32 sys_usw_register_wb_restore(uint8 lchip)
{
    uint16 length = 0;
    uint8  shift = 0;
    uint32 cmd = 0;
    uint32 loop = 0;
    int32 ret = CTC_E_NONE;
    uint16 entry_cnt = 0;

    sys_usw_register_tcat_prof_t tcat_prof_new;
    sys_wb_register_master_t wb_register_master = {0};
    ctc_wb_query_t    wb_query;
    sys_wb_register_trunc_pool_t   wb_trunc_pool = {0};
    DsTruncationProfile_m truncation_prof;

    wb_query.buffer = mem_malloc(MEM_SYSTEM_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        SYS_REGISTER_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_register_master_t, CTC_FEATURE_REGISTER, SYS_WB_APPID_REGISTER_SUBID_MASTER);

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);

    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query register master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_INVALID_CONFIG;
        goto done;
    }

    sal_memcpy((uint8*)&wb_register_master, (uint8*)wb_query.buffer, wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_REGISTER, wb_register_master.version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }
    p_usw_register_master[lchip]->tpoam_vpws_coexist = wb_register_master.oam_coexist_mode;

    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_register_trunc_pool_t, CTC_FEATURE_REGISTER, SYS_WB_APPID_REGISTER_SUBID_TRUNC_POOL);


    cmd = DRV_IOR(DsTruncationProfile_t, DRV_ENTRY_FLAG);

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_trunc_pool, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        sal_memset(&truncation_prof, 0, sizeof(truncation_prof));
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, wb_trunc_pool.profile_id, cmd, &truncation_prof),ret,done);

        shift = GetDsTruncationProfile(V, lengthShift_f, &truncation_prof);
        length = GetDsTruncationProfile(V, length_f, &truncation_prof);

        sal_memset(&tcat_prof_new, 0, sizeof(tcat_prof_new));
        tcat_prof_new.shift = shift;
        tcat_prof_new.length = length;
        tcat_prof_new.profile_id = wb_trunc_pool.profile_id;

        for (loop = 0; loop < wb_trunc_pool.ref_cnt; loop++)
        {
            CTC_ERROR_GOTO(ctc_spool_add(p_usw_register_master[lchip]->tcat_prof_spool, &tcat_prof_new, NULL, NULL),ret,done);
        }

    CTC_WB_QUERY_ENTRY_END((&wb_query));

done:
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

   return ret;
}

int32
sys_usw_register_init(uint8 lchip)
{
    if (p_usw_register_master[lchip])
    {
        return CTC_E_NONE;
    }

    p_usw_register_master[lchip] = mem_malloc(MEM_SYSTEM_MODULE, sizeof(sys_register_master_t));
    if (NULL == p_usw_register_master[lchip])
    {
        return CTC_E_NO_MEMORY;
    }

    sal_memset(p_usw_register_master[lchip], 0, sizeof(sys_register_master_t));

    sal_mutex_create(&p_usw_register_master[lchip]->register_mutex);
    if (NULL == p_usw_register_master[lchip]->register_mutex)
    {
        return CTC_E_NO_MEMORY;
    }

    p_usw_register_master[lchip]->p_register_api = (sys_register_cb_api_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(sys_register_cb_api_t));
    if (NULL == p_usw_register_master[lchip]->p_register_api)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_usw_register_master[lchip]->p_register_api, 0, sizeof(sys_register_cb_api_t));
    CTC_ERROR_RETURN(_sys_usw_register_cethertype_spool_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_register_tcat_prof_spool_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_discard_type_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_lkp_ctl_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_misc_ctl_init(lchip));

    /* mpls */
    CTC_ERROR_RETURN(_sys_usw_mpls_ctl_init(lchip));

    /* exception */
    CTC_ERROR_RETURN(_sys_usw_excp_ctl_init(lchip));

    /* ipe loopback hdr adjust ctl */
    CTC_ERROR_RETURN(_sys_usw_ipe_loopback_hdr_adjust_ctl_init(lchip));

    /* ipe route ctl */
    CTC_ERROR_RETURN(_sys_usw_ipe_route_ctl_init(lchip));

    /* ipe brige ctl */
    CTC_ERROR_RETURN(_sys_usw_ipe_brg_ctl_init(lchip));

    /* ipe fwd ctl */
    CTC_ERROR_RETURN(_sys_usw_ipe_fwd_ctl_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_mac_default_entry_init(lchip));

    /* buf retrive ctl , now only process slice0 TODO slice1*/
    CTC_ERROR_RETURN(_sys_usw_buffer_retrieve_ctl_init(lchip));

    /* Timestamp engine */
    CTC_ERROR_RETURN(_sys_usw_timestamp_engine_init(lchip));

    /* Ipg init */
    CTC_ERROR_RETURN(_sys_usw_ipg_init(lchip));

    /* MDIO init */
    CTC_ERROR_RETURN(sys_usw_mdio_init(lchip));

	/* lengthAdjust init */
	CTC_ERROR_RETURN(_sys_usw_qmgr_ctl_init(lchip));

    /*init bufferstoreCpuRxLogChannelMap*/
    CTC_ERROR_RETURN(_sys_usw_bufferstoreCpuRxLogChannelMap_init(lchip));

    CTC_ERROR_RETURN(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_REGISTER, sys_usw_register_wb_sync));

    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_register_wb_restore(lchip));
    }

    CTC_ERROR_RETURN(sys_usw_inband_init(lchip));
    return CTC_E_NONE;
}

int32
sys_usw_register_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);

    if (NULL == p_usw_register_master[lchip])
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_usw_inband_deinit(lchip));
    /*free cethertype_spool node */
    ctc_spool_free(p_usw_register_master[lchip]->cethertype_spool);
    ctc_spool_free(p_usw_register_master[lchip]->tcat_prof_spool);

    /*opf free*/
    sys_usw_opf_deinit(lchip, p_usw_register_master[lchip]->opf_type_cethertype);
    sys_usw_opf_deinit(lchip, p_usw_register_master[lchip]->opf_type_tcat_prof);


    sal_mutex_destroy(p_usw_register_master[lchip]->register_mutex);

    mem_free(p_usw_register_master[lchip]->p_register_api);
    mem_free(p_usw_register_master[lchip]);

    return CTC_E_NONE;
}

int32
sys_usw_register_add_compress_ether_type(uint8 lchip, uint16 new_ether_type, uint16 old_ether_type,uint8* o_cether_type, uint8* o_cether_type_index)
{
    uint32 cmd = 0;
    EtherTypeCompressCam_m    cam_cethertype;
    sys_register_cethertype_t new_cethertype_t = {0};
    sys_register_cethertype_t old_cethertype_t = {0};
    sys_register_cethertype_t*    pcether_get  = NULL;

    new_cethertype_t.ether_type = new_ether_type;
    new_cethertype_t.cethertype_index = *o_cether_type_index;
    old_cethertype_t.ether_type = old_ether_type;

    SYS_REGISTER_INIT_CHECK;

    SYS_USW_REGISTER_LOCK(lchip);
    CTC_ERROR_RETURN_WITH_UNLOCK(ctc_spool_add(p_usw_register_master[lchip]->cethertype_spool,&new_cethertype_t,&old_cethertype_t, &pcether_get), p_usw_register_master[lchip]->register_mutex);
    SYS_USW_REGISTER_UNLOCK(lchip);

    if(o_cether_type)
    {
        *o_cether_type = (1 << 6 | (pcether_get->cethertype_index&0x3F));

        SetEtherTypeCompressCam(V, etherType_f, &cam_cethertype, pcether_get->ether_type);
        SetEtherTypeCompressCam(V, compressEtherType_f, &cam_cethertype, pcether_get->cethertype_index);
        SetEtherTypeCompressCam(V, valid_f, &cam_cethertype, 1);

        cmd = DRV_IOW(EtherTypeCompressCam_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, pcether_get->cethertype_index, cmd, &cam_cethertype));
    }
        *o_cether_type_index = pcether_get->cethertype_index&0x3F;

    return CTC_E_NONE;
}

int32
sys_usw_register_remove_compress_ether_type(uint8 lchip, uint16 ether_type)
{
    sys_register_cethertype_t cethertype_t = {0};
    cethertype_t.ether_type = ether_type;

    SYS_REGISTER_INIT_CHECK;

    SYS_USW_REGISTER_LOCK(lchip);
    CTC_ERROR_RETURN_WITH_UNLOCK(ctc_spool_remove(p_usw_register_master[lchip]->cethertype_spool, &cethertype_t, NULL), p_usw_register_master[lchip]->register_mutex);
    SYS_USW_REGISTER_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
_sys_usw_register_show_cethertype_status(uint8 lchip)
{
    SYS_REGISTER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Cethertype Status:" " %s %u\n","max_count:",p_usw_register_master[lchip]->cethertype_spool->max_count);
    SYS_REGISTER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"                   %s %u\n","used_cout:",p_usw_register_master[lchip]->cethertype_spool->count);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_register_show_tcat_prof_status(uint8 lchip)
{
    SYS_REGISTER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Truncation Status:" " %s %u\n","max_count:",p_usw_register_master[lchip]->tcat_prof_spool->max_count);
    SYS_REGISTER_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"                   %s %u\n","used_cout:",p_usw_register_master[lchip]->tcat_prof_spool->count);
    return CTC_E_NONE;
}

int32
sys_usw_register_show_status(uint8 lchip)
{
    SYS_REGISTER_INIT_CHECK;

    SYS_USW_REGISTER_LOCK(lchip);
    _sys_usw_register_show_cethertype_status(lchip);
    _sys_usw_register_show_tcat_prof_status(lchip);
    SYS_USW_REGISTER_UNLOCK(lchip);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_global_get_chip_capability(uint8 lchip, uint32* p_capability)
{
    CTC_PTR_VALID_CHECK(p_capability);

    p_capability[CTC_GLOBAL_CAPABILITY_LOGIC_PORT_NUM] = MCHIP_CAP(SYS_CAP_SPEC_LOGIC_PORT_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_MAX_FID] = MCHIP_CAP(SYS_CAP_SPEC_MAX_FID);
    p_capability[CTC_GLOBAL_CAPABILITY_MAX_VRFID] = MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID);
    p_capability[CTC_GLOBAL_CAPABILITY_MCAST_GROUP_NUM] = MCHIP_CAP(SYS_CAP_SPEC_MCAST_GROUP_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_VLAN_NUM] = MCHIP_CAP(SYS_CAP_SPEC_VLAN_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_VLAN_RANGE_GROUP_NUM] = MCHIP_CAP(SYS_CAP_SPEC_VLAN_RANGE_GROUP_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_STP_INSTANCE_NUM] = MCHIP_CAP(SYS_CAP_SPEC_STP_INSTANCE_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_LINKAGG_GROUP_NUM] = MCHIP_CAP(SYS_CAP_SPEC_LINKAGG_GROUP_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_LINKAGG_MEMBER_NUM] = MCHIP_CAP(SYS_CAP_SPEC_LINKAGG_MEMBER_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_LINKAGG_DLB_FLOW_NUM] = MCHIP_CAP(SYS_CAP_SPEC_LINKAGG_DLB_FLOW_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_LINKAGG_DLB_MEMBER_NUM] = MCHIP_CAP(SYS_CAP_SPEC_LINKAGG_DLB_MEMBER_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_LINKAGG_DLB_GROUP_NUM] = MCHIP_CAP(SYS_CAP_SPEC_LINKAGG_DLB_GROUP_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ECMP_GROUP_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ECMP_GROUP_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ECMP_MEMBER_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ECMP_MEMBER_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ECMP_DLB_FLOW_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ECMP_DLB_FLOW_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_EXTERNAL_NEXTHOP_NUM] = MCHIP_CAP(SYS_CAP_SPEC_EXTERNAL_NEXTHOP_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_GLOBAL_DSNH_NUM] = MCHIP_CAP(SYS_CAP_SPEC_GLOBAL_DSNH_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_MPLS_TUNNEL_NUM] = MCHIP_CAP(SYS_CAP_SPEC_MPLS_TUNNEL_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ARP_ID_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ARP_ID_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_L3IF_NUM] = MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_OAM_SESSION_NUM] = MCHIP_CAP(SYS_CAP_SPEC_OAM_SESSION_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_NPM_SESSION_NUM] = MCHIP_CAP(SYS_CAP_SPEC_NPM_SESSION_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_APS_GROUP_NUM] = MCHIP_CAP(SYS_CAP_SPEC_APS_GROUP_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_TOTAL_POLICER_NUM] = MCHIP_CAP(SYS_CAP_SPEC_TOTAL_POLICER_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_POLICER_NUM] = MCHIP_CAP(SYS_CAP_SPEC_POLICER_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_TOTAL_STATS_NUM] = MCHIP_CAP(SYS_CAP_SPEC_TOTAL_STATS_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_QUEUE_STATS_NUM] = MCHIP_CAP(SYS_CAP_SPEC_QUEUE_STATS_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_POLICER_STATS_NUM] = MCHIP_CAP(SYS_CAP_SPEC_POLICER_STATS_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_SHARE1_STATS_NUM] = MCHIP_CAP(SYS_CAP_SPEC_SHARE1_STATS_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_SHARE2_STATS_NUM] = MCHIP_CAP(SYS_CAP_SPEC_SHARE2_STATS_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_SHARE3_STATS_NUM] = MCHIP_CAP(SYS_CAP_SPEC_SHARE3_STATS_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_SHARE4_STATS_NUM] = MCHIP_CAP(SYS_CAP_SPEC_SHARE4_STATS_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL0_IGS_STATS_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL0_IGS_STATS_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL1_IGS_STATS_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL1_IGS_STATS_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL2_IGS_STATS_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL2_IGS_STATS_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL3_IGS_STATS_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL3_IGS_STATS_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL0_EGS_STATS_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL0_EGS_STATS_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ECMP_STATS_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ECMP_STATS_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ROUTE_MAC_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ROUTE_MAC_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_MAC_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_MAC_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_BLACK_HOLE_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_L2_BLACK_HOLE_ENTRY);
    p_capability[CTC_GLOBAL_CAPABILITY_HOST_ROUTE_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_HOST_ROUTE_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_LPM_ROUTE_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_LPM_ROUTE_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_IPMC_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_IPMC_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_MPLS_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_MPLS_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_TUNNEL_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_TUNNEL_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_L2PDU_L2HDR_PROTO_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_L2PDU_BASED_L2HDR_PTL_ENTRY);
    p_capability[CTC_GLOBAL_CAPABILITY_L2PDU_MACDA_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_L2PDU_BASED_MACDA_ENTRY);
    p_capability[CTC_GLOBAL_CAPABILITY_L2PDU_MACDA_LOW24_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_L2PDU_BASED_MACDA_LOW24_ENTRY);
    p_capability[CTC_GLOBAL_CAPABILITY_L2PDU_L2CP_MAX_ACTION_INDEX] = MCHIP_CAP(SYS_CAP_L2PDU_PER_PORT_ACTION_INDEX);
    p_capability[CTC_GLOBAL_CAPABILITY_L3PDU_L3HDR_PROTO_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_L3PDU_BASED_L3HDR_PROTO);
    p_capability[CTC_GLOBAL_CAPABILITY_L3PDU_L4PORT_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_L3PDU_BASED_PORT);
    p_capability[CTC_GLOBAL_CAPABILITY_L3PDU_IPDA_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_L3PDU_BASED_IPDA);
    p_capability[CTC_GLOBAL_CAPABILITY_L3PDU_MAX_ACTION_INDEX] = MCHIP_CAP(SYS_CAP_L3PDU_ACTION_INDEX);
    p_capability[CTC_GLOBAL_CAPABILITY_SCL_HASH_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_SCL_HASH_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_SCL1_HASH_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_SCL1_HASH_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_SCL_TCAM_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_SCL0_IGS_TCAM_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_SCL1_TCAM_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_SCL1_IGS_TCAM_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_SCL2_TCAM_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_SCL2_IGS_TCAM_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_SCL3_TCAM_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_SCL3_IGS_TCAM_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL_HASH_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL_HASH_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL0_IGS_TCAM_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL0_IGS_TCAM_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL1_IGS_TCAM_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL1_IGS_TCAM_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL2_IGS_TCAM_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL2_IGS_TCAM_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL3_IGS_TCAM_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL3_IGS_TCAM_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL4_IGS_TCAM_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL4_IGS_TCAM_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL5_IGS_TCAM_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL5_IGS_TCAM_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL6_IGS_TCAM_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL6_IGS_TCAM_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL7_IGS_TCAM_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL7_IGS_TCAM_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL0_EGS_TCAM_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL0_EGS_TCAM_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL1_EGS_TCAM_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL1_EGS_TCAM_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_ACL2_EGS_TCAM_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_ACL2_EGS_TCAM_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_CID_PAIR_NUM] = MCHIP_CAP(SYS_CAP_ACL_HASH_CID_KEY)+ MCHIP_CAP(SYS_CAP_ACL_TCAM_CID_PAIR);
    p_capability[CTC_GLOBAL_CAPABILITY_UDF_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_ACL_UDF_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_IPFIX_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_IPFIX_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_EFD_FLOW_ENTRY_NUM] = MCHIP_CAP(SYS_CAP_SPEC_EFD_FLOW_ENTRY_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_MAX_LCHIP_NUM] = MCHIP_CAP(SYS_CAP_SPEC_MAX_LCHIP_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_MAX_PHY_PORT_NUM] = MCHIP_CAP(SYS_CAP_SPEC_MAX_PHY_PORT_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM] = MCHIP_CAP(SYS_CAP_SPEC_MAX_PORT_NUM);
    p_capability[CTC_GLOBAL_CAPABILITY_MAX_CHIP_NUM] = MCHIP_CAP(SYS_CAP_SPEC_MAX_CHIP_NUM);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_global_ctl_l4_check_en(uint8 lchip, ctc_global_control_type_t type, uint32* value, uint8 is_set)
{
    uint32 field_val = 0;
    uint32 cmd = 0;
    uint8 bit = 0;

    switch (type)
    {
        case CTC_GLOBAL_DISCARD_TCP_SYN_0_PKT:
            bit = 0;
            break;
        case CTC_GLOBAL_DISCARD_TCP_NULL_PKT:
            bit = 1;
            break;
        case CTC_GLOBAL_DISCARD_TCP_XMAS_PKT:
            bit = 2;
            break;
        case CTC_GLOBAL_DISCARD_TCP_SYN_FIN_PKT:
            bit = 3;
            break;
        case CTC_GLOBAL_DISCARD_SAME_L4_PORT_PKT:
            bit = 4;
            break;
        case CTC_GLOBAL_DISCARD_ICMP_FRAG_PKT:
            bit = 5;
            break;
        default:
            return CTC_E_NOT_SUPPORT;
    }

    cmd = DRV_IOR(IpeIntfMapperCtl_t, IpeIntfMapperCtl_layer4SecurityCheckEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    if (is_set)
    {
        if (*value)
        {
            CTC_BIT_SET(field_val, bit);
        }
        else
        {
            CTC_BIT_UNSET(field_val, bit);
        }
        cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_layer4SecurityCheckEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }
    else
    {
        *value = CTC_IS_BIT_SET(field_val, bit)? 1 : 0;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_global_get_panel_ports(uint8 lchip, ctc_global_panel_ports_t* phy_info)
{
    uint16 lport = 0;
    uint8 count = 0;
    int32 ret = 0;
    uint8 gchip = 0;
    uint32 port_type = 0;
    uint32 gport = 0;

    lchip = phy_info->lchip;
    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    for (lport = 0; lport < MCHIP_CAP(SYS_CAP_SPEC_MAX_PORT_NUM); lport++)
    {
        gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
        ret = sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_PORT_TYPE, &port_type);
        if (ret < 0)
        {
            continue;
        }
        if (port_type != SYS_DMPS_NETWORK_PORT)
        {
            continue;
        }
        phy_info->lport[count++] = lport;
    }
    phy_info->count = count;
    return CTC_E_NONE;
}
STATIC int32
_sys_usw_global_wait_all_queues_to_empty(uint8 lchip)
{
    int32  ret = 0;
    sys_qos_shape_profile_t* shp_profile = NULL;
    uint32 depth;
    uint16 index;
    uint16  loop;
    uint32 gport;
    uint8  gchip;
    uint8  chan_id;
    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    shp_profile = mem_malloc(MEM_SYSTEM_MODULE, sizeof(sys_qos_shape_profile_t)*MCHIP_CAP(SYS_CAP_SPEC_MAX_PHY_PORT_NUM));
    sal_memset(shp_profile, 0, sizeof(sys_qos_shape_profile_t)*MCHIP_CAP(SYS_CAP_SPEC_MAX_PHY_PORT_NUM));
    for(loop=0; loop < MCHIP_CAP(SYS_CAP_SPEC_MAX_PHY_PORT_NUM); loop++)
    {
        gport = CTC_MAP_LPORT_TO_GPORT(gchip, loop);
        chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
        if (0xFF == chan_id)
        {
            continue;
        }
        CTC_ERROR_GOTO(sys_usw_queue_get_profile_from_hw(lchip, gport, shp_profile+loop), ret, end_0);
        CTC_ERROR_GOTO(sys_usw_queue_set_port_drop_en(lchip, gport, 1, shp_profile+loop), ret, end_0);
    }

    for(loop=0; loop < MCHIP_CAP(SYS_CAP_SPEC_MAX_PHY_PORT_NUM); loop++)
    {
        gport = CTC_MAP_LPORT_TO_GPORT(gchip, loop);
        chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
        if (0xFF == chan_id)
        {
            continue;
        }
        index = 0;
        CTC_ERROR_GOTO(sys_usw_queue_get_port_depth(lchip, gport, &depth), ret, end_0);
        while (depth)
        {
            if ((index++) > 100)
            {
                return CTC_E_HW_TIME_OUT;
            }
            CTC_ERROR_GOTO(sys_usw_queue_get_port_depth(lchip, gport, &depth), ret, end_0);
        }
    }
end_0:
    for(loop=0; loop < MCHIP_CAP(SYS_CAP_SPEC_MAX_PHY_PORT_NUM); loop++)
    {
        gport = CTC_MAP_LPORT_TO_GPORT(gchip, loop);
        chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
        if (0xFF == chan_id)
        {
            continue;
        }
        sys_usw_queue_set_port_drop_en(lchip, gport, 0, shp_profile+loop);
    }
    if(shp_profile)
    {
        mem_free(shp_profile);
    }
    return ret;
}
STATIC int32
_sys_usw_global_net_rx_enable(uint8 lchip, uint32 enable)
{
    uint32 field_val = enable?1:0;
    uint8  loop;
    uint8  loop1;
    uint32 base_table = 0;
    uint32 cmd = 0;

    if(!DRV_IS_TSINGMA(lchip))
    {
        return CTC_E_NOT_SUPPORT;
    }
    for(loop1=0; loop1 < 4; loop1++)
    {
        switch(loop1)
        {
            case 0:
               base_table = Sgmac0RxCfg0_t;
               break;
            case 1:
               base_table = Sgmac1RxCfg0_t;
               break;
            case 2:
               base_table = Sgmac2RxCfg0_t;
               break;
            default:
               base_table = Sgmac3RxCfg0_t;
               break;
        }
        for(loop=0; loop < 18; loop++)
        {
            cmd = DRV_IOW(base_table+loop, Sgmac0RxCfg0_cfgSgmac0RxPktEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        }
    }

    if(!enable)
    {
        CTC_ERROR_RETURN(_sys_usw_global_wait_all_queues_to_empty(lchip));
    }
    return CTC_E_NONE;
}
int32
sys_usw_global_ctl_set(uint8 lchip, ctc_global_control_type_t type, void* value)
{
    uint32 field_val = 0;
    uint32 val = 0;
    uint8 enable = 0;
    uint8 chan_id = 0;
    uint32 cmd = 0;
    ds_t   ds;
    ctc_global_overlay_decap_mode_t* p_decap_mode = (ctc_global_overlay_decap_mode_t*)value;

    SYS_REGISTER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_REGISTER_INIT_CHECK;
    CTC_PTR_VALID_CHECK(value);

    switch(type)
    {
    case CTC_GLOBAL_DISCARD_SAME_MACDASA_PKT:
        field_val = *(bool *)value ? 1 : 0;

        cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_discardSameMacAddr_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_discardSameMacAddr_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_GLOBAL_DISCARD_SAME_IPDASA_PKT:
        field_val = *(bool *)value ? 1 : 0;

        cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_discardSameIpAddr_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_discardSameIpAddr_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_GLOBAL_DISCARD_TTL_0_PKT:
        field_val = *(bool *)value ? 1 : 0;

        sal_memset(ds, 0, sizeof(ds));
        cmd = DRV_IOR(EpePktProcCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
        SetEpePktProcCtl(V, discardMplsTagTtl0_f, ds, field_val);
        SetEpePktProcCtl(V, discardMplsTtl0_f, ds, field_val);
        SetEpePktProcCtl(V, discardRouteTtl0_f, ds, field_val);
        SetEpePktProcCtl(V, discardTrillTtl0_f, ds, field_val);
        SetEpePktProcCtl(V, discardTunnelTtl0_f, ds, field_val);
        SetEpePktProcCtl(V, ofTtlFailDiscard_f, ds, field_val);
        cmd = DRV_IOW(EpePktProcCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
        break;

    case CTC_GLOBAL_DISCARD_MCAST_SA_PKT:
        field_val = *(bool *)value ? 0 : 1;

        cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_allowMcastMacSa_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        break;

    case CTC_GLOBAL_ACL_CHANGE_COS_ONLY:
        /*Dt2 using vlan profile for this function*/
        #if 0
        field_val = *(bool *)value ? 1 : 0;
        cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_aclMergeVlanAction_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        #endif

        break;

    case CTC_GLOBAL_ECMP_DLB_MODE:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, CTC_GLOBAL_ECMP_DLB_MODE_MAX - 1);

        if (CTC_GLOBAL_ECMP_DLB_MODE_ELEPHANT == *(uint32*)value)
        {
            field_val = 1;
        }
        else if (CTC_GLOBAL_ECMP_DLB_MODE_TCP == *(uint32*)value)
        {
            field_val = 3;
        }
        else if (CTC_GLOBAL_ECMP_DLB_MODE_ALL == *(uint32*)value)
        {
            field_val = 2;
        }

        cmd = DRV_IOW(IpeEcmpCtl_t, IpeEcmpCtl_dlbEnableMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_GLOBAL_ECMP_REBALANCE_MODE:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, CTC_GLOBAL_ECMP_REBALANCE_MODE_MAX - 1);

        if (CTC_GLOBAL_ECMP_REBALANCE_MODE_NORMAL == *(uint32*)value)
        {
            field_val = 2;
        }
        else if (CTC_GLOBAL_ECMP_REBALANCE_MODE_FIRST == *(uint32*)value)
        {
            field_val = 0;
        }
        else if (CTC_GLOBAL_ECMP_REBALANCE_MODE_PACKET == *(uint32*)value)
        {
            field_val = 1;
        }

        cmd = DRV_IOW(DlbEngineCtl_t, DlbEngineCtl_rebalanceMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

     case CTC_GLOBAL_ECMP_FLOW_AGING_INTERVAL:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 4294967);
        field_val = ((*(uint32*)value) * 250) & 0x3FFFFFFF;
        cmd = DRV_IOW(DlbEngineTimerCtl_t, DlbEngineTimerCtl_cfgRefDivAgingRstPulse_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_GLOBAL_ECMP_FLOW_INACTIVE_INTERVAL:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 0xFFFFFFFF);
        field_val = ((*(uint32*)value) >> 3) & 0x3FFFFFFF;
        cmd = DRV_IOW(DlbEngineTimerCtl_t, DlbEngineTimerCtl_cfgRefDivInactiveRstPulse_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_GLOBAL_ECMP_FLOW_PKT_INTERVAL:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 0xFFFF);

        if((*(uint32*)value) <= 256)        /* every 256 packets */
        {
            field_val = 1;
        }
        else if((*(uint32*)value) <= 1024)  /* every 1024 packets */
        {
            field_val = 0;
        }
        else if((*(uint32*)value) <= 8192)  /* every 8192 packets */
        {
            field_val = 2;
        }
        else                                 /* every 32768 packets */
        {
            field_val = 3;
        }

        cmd = DRV_IOW(DlbEngineCtl_t, DlbEngineCtl_reorderPktIntervalMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_GLOBAL_LINKAGG_FLOW_INACTIVE_INTERVAL:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 0xFF);
        field_val = (*(uint32*)value);
        cmd = DRV_IOW(LagEngineTimerCtl0_t, LagEngineTimerCtl0_tsThreshold0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_GLOBAL_STACKING_TRUNK_FLOW_INACTIVE_INTERVAL:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 0xFF);
        field_val = (*(uint32*)value);
        cmd = DRV_IOW(LagEngineTimerCtl1_t, LagEngineTimerCtl1_tsThreshold1_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_GLOBAL_WARMBOOT_STATUS:               /**< [D2] set warmboot status */
        drv_set_warmboot_status(lchip, *(uint32*)value);
        if (*(uint32*)value == CTC_WB_STATUS_DONE)
        {
            /*enable interrupt*/
            sys_usw_interrupt_set_group_en(lchip, TRUE);

            /*enable failover after warmboot done*/
            sys_usw_global_failover_en(lchip, TRUE);

            CTC_ERROR_RETURN(sys_usw_set_dma_channel_drop_en(lchip, FALSE));

        }else if (*(uint32*)value == CTC_WB_STATUS_SYNC)
        {
            CTC_ERROR_RETURN(_sys_usw_set_warmboot_sync(lchip));
        }
        break;

    case CTC_GLOBAL_WARMBOOT_CPU_RX_EN:
        enable = *(bool *)value ? 1 : 0;

        for(chan_id = 0; chan_id < SYS_DMA_MAX_CHAN_NUM; chan_id++)
        {
            if(chan_id == SYS_DMA_TBL_RD_CHAN_ID || chan_id == SYS_DMA_HASHKEY_CHAN_ID)
            {
                continue;
            }
            CTC_ERROR_RETURN(sys_usw_dma_set_chan_en(lchip, chan_id, enable));
        }

        CTC_ERROR_RETURN(sys_usw_interrupt_set_group_en(lchip, enable));
        break;
    case CTC_GLOBAL_DISCARD_TCP_SYN_0_PKT:
    case CTC_GLOBAL_DISCARD_TCP_NULL_PKT:
    case CTC_GLOBAL_DISCARD_TCP_XMAS_PKT:
    case CTC_GLOBAL_DISCARD_TCP_SYN_FIN_PKT:
    case CTC_GLOBAL_DISCARD_SAME_L4_PORT_PKT:
    case CTC_GLOBAL_DISCARD_ICMP_FRAG_PKT:
        CTC_ERROR_RETURN(_sys_usw_global_ctl_l4_check_en(lchip, type, ((uint32*)value), 1));
        break;

    case CTC_GLOBAL_ARP_MACDA_CHECK_EN:
    case CTC_GLOBAL_ARP_MACSA_CHECK_EN:
    case CTC_GLOBAL_ARP_IP_CHECK_EN:
    case CTC_GLOBAL_ARP_CHECK_FAIL_TO_CPU:
        CTC_ERROR_RETURN(_sys_usw_global_ctl_set_arp_check_en(lchip, type, ((uint32*)value)));
        break;
   case CTC_GLOBAL_IGMP_SNOOPING_MODE:
        CTC_ERROR_RETURN(_sys_usw_global_ctl_set_igmp_mode(lchip, type, ((uint32*)value)));
        break;

    case CTC_GLOBAL_FLOW_PROPERTY:
        CTC_ERROR_RETURN(_sys_usw_set_glb_flow_property(lchip, (ctc_global_flow_property_t*)value));
        break;

    case CTC_GLOBAL_ACL_LKUP_PROPERTY:
        CTC_ERROR_RETURN(_sys_usw_set_glb_acl_lkup_property(lchip, (ctc_acl_property_t*)value));
        break;
    case CTC_GLOBAL_ELOOP_USE_LOGIC_DESTPORT:
        field_val = (*(uint32*)value)?1:0;
        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_ingressEditUseLogicPortSelect_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_ACL_PROPERTY:
        CTC_ERROR_RETURN(_sys_usw_set_glb_acl_property(lchip, (ctc_global_acl_property_t*)value));
        break;
    case CTC_GLOBAL_PIM_SNOOPING_MODE:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 1);
        field_val = *(uint32 *)value ? 0 : 1;
        cmd = DRV_IOW(IpeLookupCtl_t, IpeLookupCtl_pimSnoopingEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_IPMC_PROPERTY:
        CTC_ERROR_RETURN(_sys_usw_set_glb_ipmc_property(lchip, (ctc_global_ipmc_property_t*)value));
        break;

    case CTC_GLOBAL_CID_PROPERTY:
        CTC_ERROR_RETURN(_sys_usw_set_glb_cid_property(lchip, (ctc_global_cid_property_t*)value));
        break;

    case CTC_GLOBAL_VXLAN_UDP_DEST_PORT:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 0xFFFF);
        field_val = *(uint32*)value;
        if (0 == field_val)
        {
            field_val = 4789;
        }
        cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_vxlanV4UdpDestPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_vxlanV6UdpDestPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(ParserLayer4AppCtl_t, ParserLayer4AppCtl_vxlanUdpDestPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(ParserLayer4AppCtl_t, ParserLayer4AppCtl_evxlanUdpDestPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_GENEVE_UDP_DEST_PORT:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 0xFFFF);
        field_val = *(uint32*)value;
        if (0 == field_val)
        {
            field_val = 6081;
        }
        cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_geneveV4UdpDestPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_geneveV6UdpDestPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(ParserLayer4AppCtl_t, ParserLayer4AppCtl_geneveVxlanUdpDestPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_NH_FORCE_BRIDGE_DISABLE:
        field_val = (*(uint32*)value)?0:1;
        cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_forceBridgeL3Match_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_NH_MCAST_LOGIC_REP_EN:
        enable = *(bool *)value ? 1 : 0;
        CTC_ERROR_RETURN(sys_usw_nh_set_ipmc_logic_replication(lchip, enable));
        break;

    case CTC_GLOBAL_LB_HASH_KEY:
        if ((NULL == MCHIP_API(lchip)) || (NULL == MCHIP_API(lchip)->hash_set_cfg))
        {
            return CTC_E_NOT_SUPPORT;
        }
        CTC_ERROR_RETURN(MCHIP_API(lchip)->hash_set_cfg(lchip, value));
        break;
    case CTC_GLOBAL_LB_HASH_OFFSET_PROFILE:
        if ((NULL == MCHIP_API(lchip)) || (NULL == MCHIP_API(lchip)->hash_set_offset))
        {
            return CTC_E_NOT_SUPPORT;
        }
        CTC_ERROR_RETURN(MCHIP_API(lchip)->hash_set_offset(lchip, value));

        break;
    case CTC_GLOBAL_OAM_POLICER_EN:
        val = (*(uint32*)value)?0:1;
        if (DRV_IS_DUET2(lchip))
        {
            cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_oamBypassPolicingOp_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &val));
            cmd = DRV_IOR(IpeFwdReserved_t, IpeFwdReserved_reserved_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            val ? CTC_BIT_SET(field_val, 0) : CTC_BIT_UNSET(field_val, 0);
            cmd = DRV_IOW(IpeFwdReserved_t, IpeFwdReserved_reserved_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            cmd = DRV_IOW(EpeClassficationCtl_t, EpeClassficationCtl_oamBypassPolicingOp_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &val));
            cmd = DRV_IOR(EpeAclOamReserved_t, EpeAclOamReserved_reserved_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            val ? CTC_BIT_SET(field_val, 0) : CTC_BIT_UNSET(field_val, 0);
            cmd = DRV_IOW(EpeAclOamReserved_t, EpeAclOamReserved_reserved_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        }
        else
        {
            cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_oamBypassPolicingOp_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &val));
            cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_fromCpuOrOamBypassPolicing_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &val));
            cmd = DRV_IOW(EpeClassficationCtl_t, EpeClassficationCtl_oamBypassPolicingOp_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &val));
            cmd = DRV_IOW(EpeClassficationCtl_t, EpeClassficationCtl_fromCpuOrOamBypassPolicing_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &val));
        }

        cmd = DRV_IOW(EpeClassficationCtl_t, EpeClassficationCtl_oamBypassPolicingOp_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &val));
        cmd = DRV_IOR(EpeAclOamReserved_t, EpeAclOamReserved_reserved_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        val ? CTC_BIT_SET(field_val, 0) : CTC_BIT_UNSET(field_val, 0);
        cmd = DRV_IOW(EpeAclOamReserved_t, EpeAclOamReserved_reserved_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_PARSER_OUTER_ALWAYS_CVLAN_EN:
        field_val = (*(uint32*)value)? 1 : 0;
        cmd = DRV_IOW(ParserEthernetCtl_t, ParserEthernetCtl_outerAlwaysCvlan_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_VPWS_SNOOPING_PARSER:
        val = (*(bool *)value) ? 1 : 0;
        cmd = DRV_IOW(IpeTunnelIdCtl_t, IpeTunnelIdCtl_vpwsDecapsForceSecondParser_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &val));
        break;
    case CTC_GLOBAL_ARP_VLAN_CLASS_EN:
        field_val = (*(uint32*)value)? 1 : 0;
        cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_arpForceIpv4_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_DISCARD_MACSA_0_PKT:
        field_val = *(bool *)value ? 1 : 0;
        cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_discardAllZeroMacSa_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_NH_ARP_MACDA_DERIVE_MODE:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, CTC_GLOBAL_MACDA_DERIVE_FROM_NH_ROUTE2);

        field_val = (*(uint32*)value)? 1 : 0;
        cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_hostRouteNextHopSaveEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_hostRouteNextHopSaveEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));


        field_val = (*(uint32*)value == CTC_GLOBAL_MACDA_DERIVE_FROM_NH_ROUTE2)? 0 : 1;
        cmd = DRV_IOW(IpeRouteCtl_t, IpeRouteCtl_cloudSecUseShareBit_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        /*cfg derive mode*/
        field_val = *(uint32*)value;
        cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_hostRouteNextHopSaveMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_VXLAN_CRYPT_UDP_DEST_PORT:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 0xffff);
        field_val = *(uint32*)value;
        cmd = DRV_IOW(ParserLayer4AppCtl_t, ParserLayer4AppCtl_encryptVxlanUdpDestPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(ParserLayer4AppCtl_t, ParserLayer4AppCtl_decryptVxlanUdpDestPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_encryptVxlanUdpDestPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST:
        if (!DRV_IS_DUET2(lchip))
        {
            return CTC_E_NOT_SUPPORT;
        }
        field_val = *(uint32 *)value ? 1 : 0;
        p_usw_register_master[lchip]->tpoam_vpws_coexist = field_val;
        break;
    case CTC_GLOBAL_VXLAN_POLICER_GROUP_ID_BASE:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 0xff00);
        field_val = *(uint32*)value;
        cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_vxlanPolicyGidBase_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_INBAND_CPU_TRAFFIC_TIMER:
        if (p_usw_register_master[lchip]->p_register_api->inband_timer)
        {
            CTC_ERROR_RETURN(p_usw_register_master[lchip]->p_register_api->inband_timer(lchip, *(uint32*)value));
        }
        break;
    case CTC_GLOBAL_DUMP_DB:
        CTC_ERROR_RETURN(_sys_usw_set_dump_db(lchip, (ctc_global_dump_db_t*)value));
        break;
    case CTC_GLOBAL_IGS_RANDOM_LOG_SHIFT:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 7);
        field_val = *(uint32*)value;
        cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_randomLogShift_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_EGS_RANDOM_LOG_SHIFT:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 7);
        field_val = *(uint32*)value;
        cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_randomLogShift_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_OVERLAY_DECAP_MODE:
        CTC_MAX_VALUE_CHECK(p_decap_mode->scl_id, 1);
        cmd = DRV_IOW(IpeUserIdCtl_t, DRV_IS_DUET2(lchip) ? \
            ((p_decap_mode->scl_id)? IpeUserIdCtl_vxlanTunnelHash2LkpMode_f: IpeUserIdCtl_vxlanTunnelHash1LkpMode_f):\
            ((p_decap_mode->scl_id)? IpeUserIdCtl_vxlanTunnelHash1LkpMode_f: IpeUserIdCtl_vxlanTunnelHash0LkpMode_f));
        field_val = p_decap_mode->vxlan_mode ? 0:1;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(IpeUserIdCtl_t, DRV_IS_DUET2(lchip) ? \
            ((p_decap_mode->scl_id)? IpeUserIdCtl_nvgreTunnelHash2LkpMode_f: IpeUserIdCtl_nvgreTunnelHash1LkpMode_f):
            ((p_decap_mode->scl_id)? IpeUserIdCtl_nvgreTunnelHash1LkpMode_f: IpeUserIdCtl_nvgreTunnelHash0LkpMode_f));
        field_val = p_decap_mode->nvgre_mode ? 0:1;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_EGS_STK_ACL_DIS:
        cmd = DRV_IOW(EpeAclQosCtl_t, EpeAclQosCtl_stackingDisableAcl_f);
        field_val = (*(bool*)value)? 1:0;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
   case CTC_GLOBAL_STK_WITH_IGS_PKT_HDR_EN:
        cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_toRemoteCpuAttachHeader_f);
        field_val = (*(bool*)value)? 1:0;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;
    case CTC_GLOBAL_NET_RX_EN:
        field_val = (*(bool*)value)? 1:0;
        CTC_ERROR_RETURN(_sys_usw_global_net_rx_enable(lchip, field_val));

        /*clear change memory status*/
        if(field_val)
        {
            uint8 work_status = 0;
            CTC_ERROR_RETURN(sys_usw_ftm_set_working_status(lchip, work_status));
        }
        break;
    default:
        return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}

int32
sys_usw_global_ctl_get(uint8 lchip, ctc_global_control_type_t type, void* value)
{
    uint32 field_val = 0;
    uint8 enable = 0;
    uint32 cmd = 0;
    ctc_global_overlay_decap_mode_t* p_decap_mode = (ctc_global_overlay_decap_mode_t*)value;

    SYS_REGISTER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(value);
    SYS_REGISTER_INIT_CHECK;

    switch(type)
    {
    case CTC_GLOBAL_DISCARD_SAME_MACDASA_PKT:
        cmd = DRV_IOR(IpeIntfMapperCtl_t, IpeIntfMapperCtl_discardSameMacAddr_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool *)value = field_val?TRUE:FALSE;
        break;

    case CTC_GLOBAL_DISCARD_SAME_IPDASA_PKT:
        cmd = DRV_IOR(IpeIntfMapperCtl_t, IpeIntfMapperCtl_discardSameIpAddr_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool *)value = field_val?TRUE:FALSE;
        break;

    case CTC_GLOBAL_DISCARD_TTL_0_PKT:
        cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_discardMplsTagTtl0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool *)value = field_val?TRUE:FALSE;
        break;

    case CTC_GLOBAL_DISCARD_MCAST_SA_PKT:
        cmd = DRV_IOR(IpeIntfMapperCtl_t, IpeIntfMapperCtl_allowMcastMacSa_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool *)value = field_val?FALSE:TRUE;
        break;

    case CTC_GLOBAL_ACL_CHANGE_COS_ONLY:
        #if 0
        cmd = DRV_IOR(IpeAclQosCtl_t, IpeAclQosCtl_aclMergeVlanAction_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool *)value = field_val ? TRUE:FALSE;
        #endif
        break;

    case CTC_GLOBAL_ECMP_DLB_MODE:
        cmd = DRV_IOR(IpeEcmpCtl_t, IpeEcmpCtl_dlbEnableMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        if (1 == field_val)
        {
            *(uint32*)value = CTC_GLOBAL_ECMP_DLB_MODE_ELEPHANT;
        }
        else if (3 == field_val)
        {
            *(uint32*)value = CTC_GLOBAL_ECMP_DLB_MODE_TCP;
        }
        else if (2 == field_val)
        {
            *(uint32*)value = CTC_GLOBAL_ECMP_DLB_MODE_ALL;
        }
        break;

    case CTC_GLOBAL_ECMP_REBALANCE_MODE:
        cmd = DRV_IOR(DlbEngineCtl_t, DlbEngineCtl_rebalanceMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        if (0 == field_val)
        {
            *(uint32*)value = CTC_GLOBAL_ECMP_REBALANCE_MODE_FIRST;
        }
        else if (1 == field_val)
        {
            *(uint32*)value = CTC_GLOBAL_ECMP_REBALANCE_MODE_PACKET;
        }
        else if (2 == field_val)
        {
            *(uint32*)value = CTC_GLOBAL_ECMP_REBALANCE_MODE_NORMAL;
        }
        break;

     case CTC_GLOBAL_ECMP_FLOW_AGING_INTERVAL:
        cmd = DRV_IOR(DlbEngineTimerCtl_t, DlbEngineTimerCtl_cfgRefDivAgingRstPulse_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32*)value = field_val / 250;
        break;

    case CTC_GLOBAL_ECMP_FLOW_INACTIVE_INTERVAL:
        cmd = DRV_IOR(DlbEngineTimerCtl_t, DlbEngineTimerCtl_cfgRefDivInactiveRstPulse_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32*)value = field_val << 3;
        break;

    case CTC_GLOBAL_ECMP_FLOW_PKT_INTERVAL:
        cmd = DRV_IOR(DlbEngineCtl_t, DlbEngineCtl_reorderPktIntervalMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        if (0 == field_val)
        {
            *(uint32*)value = 1024;
        }
        else if (1 == field_val)
        {
            *(uint32*)value = 256;
        }
        else if (2 == field_val)
        {
            *(uint32*)value = 8192;
        }
        else if (3 == field_val)
        {
            *(uint32*)value = 32768;
        }
        break;

    case CTC_GLOBAL_LINKAGG_FLOW_INACTIVE_INTERVAL:
        cmd = DRV_IOR(LagEngineTimerCtl0_t, LagEngineTimerCtl0_tsThreshold0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32*)value = field_val;
        break;

    case CTC_GLOBAL_STACKING_TRUNK_FLOW_INACTIVE_INTERVAL:
        cmd = DRV_IOR(LagEngineTimerCtl1_t, LagEngineTimerCtl1_tsThreshold1_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32*)value = field_val;
        break;

    case CTC_GLOBAL_DISCARD_TCP_SYN_0_PKT:
    case CTC_GLOBAL_DISCARD_TCP_NULL_PKT:
    case CTC_GLOBAL_DISCARD_TCP_XMAS_PKT:
    case CTC_GLOBAL_DISCARD_TCP_SYN_FIN_PKT:
    case CTC_GLOBAL_DISCARD_SAME_L4_PORT_PKT:
    case CTC_GLOBAL_DISCARD_ICMP_FRAG_PKT:
        CTC_ERROR_RETURN(_sys_usw_global_ctl_l4_check_en(lchip, type, ((uint32*)value), 0));
        break;

    case CTC_GLOBAL_ARP_MACDA_CHECK_EN:
    case CTC_GLOBAL_ARP_MACSA_CHECK_EN:
    case CTC_GLOBAL_ARP_IP_CHECK_EN:
    case CTC_GLOBAL_ARP_CHECK_FAIL_TO_CPU:
        CTC_ERROR_RETURN(_sys_usw_global_ctl_get_arp_check_en(lchip, type, ((uint32*)value)));
        break;
    case CTC_GLOBAL_IGMP_SNOOPING_MODE:
        CTC_ERROR_RETURN(_sys_usw_global_ctl_get_igmp_mode(lchip, type, ((uint32*)value)));
        break;
    case CTC_GLOBAL_CHIP_CAPABILITY:
        CTC_ERROR_RETURN(_sys_usw_global_get_chip_capability(lchip, (uint32*)value));
        break;

    case CTC_GLOBAL_FLOW_PROPERTY:
        CTC_ERROR_RETURN(_sys_usw_get_glb_flow_property(lchip, (ctc_global_flow_property_t*)value));
        break;

    case CTC_GLOBAL_ACL_LKUP_PROPERTY:
        CTC_ERROR_RETURN(_sys_usw_get_glb_acl_lkup_property(lchip, (ctc_acl_property_t*)value));
        break;
    case CTC_GLOBAL_ACL_PROPERTY:
        CTC_ERROR_RETURN(sys_usw_get_glb_acl_property(lchip, (ctc_global_acl_property_t*)value));
        break;
    case CTC_GLOBAL_PIM_SNOOPING_MODE:
        cmd = DRV_IOR(IpeLookupCtl_t, IpeLookupCtl_pimSnoopingEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32 *)value = field_val ? 0 : 1;
        break;
    case CTC_GLOBAL_IPMC_PROPERTY:
        CTC_ERROR_RETURN(_sys_usw_get_glb_ipmc_property(lchip, (ctc_global_ipmc_property_t*)value));
        break;
    case CTC_GLOBAL_CID_PROPERTY:
        CTC_ERROR_RETURN(_sys_usw_get_glb_cid_property(lchip, (ctc_global_cid_property_t*)value));
         break;

    case CTC_GLOBAL_VXLAN_UDP_DEST_PORT:
        cmd = DRV_IOR(ParserLayer4AppCtl_t, ParserLayer4AppCtl_vxlanUdpDestPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32*)value = field_val;
        break;
    case CTC_GLOBAL_GENEVE_UDP_DEST_PORT:
        cmd = DRV_IOR(ParserLayer4AppCtl_t, ParserLayer4AppCtl_geneveVxlanUdpDestPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32*)value = field_val;
		break;
    case CTC_GLOBAL_WARMBOOT_CPU_RX_EN:
        CTC_ERROR_RETURN(sys_usw_interrupt_get_group_en(lchip, &enable));
        *(bool *)value = (enable)?TRUE:FALSE;
        break;
    case CTC_GLOBAL_ELOOP_USE_LOGIC_DESTPORT:
        cmd = DRV_IOR(EpeHeaderEditCtl_t, EpeHeaderEditCtl_ingressEditUseLogicPortSelect_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool *)value = (field_val)?TRUE:FALSE;
        break;
    case CTC_GLOBAL_NH_FORCE_BRIDGE_DISABLE:
        cmd = DRV_IOR(EpeNextHopCtl_t, EpeNextHopCtl_forceBridgeL3Match_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool *)value = (field_val)?FALSE:TRUE;
        break;

    case CTC_GLOBAL_NH_MCAST_LOGIC_REP_EN:
        *(bool *)value = sys_usw_nh_is_ipmc_logic_rep_enable(lchip);
        break;

	case CTC_GLOBAL_LB_HASH_KEY:
        if ((NULL == MCHIP_API(lchip)) || (NULL == MCHIP_API(lchip)->hash_get_cfg))
        {
            return CTC_E_NOT_SUPPORT;
        }
        CTC_ERROR_RETURN(MCHIP_API(lchip)->hash_get_cfg(lchip, value));

		break;
    case CTC_GLOBAL_LB_HASH_OFFSET_PROFILE:
        if ((NULL == MCHIP_API(lchip)) || (NULL == MCHIP_API(lchip)->hash_get_offset))
        {
            return CTC_E_NOT_SUPPORT;
        }
        CTC_ERROR_RETURN(MCHIP_API(lchip)->hash_get_offset(lchip, value));
		break;
    case CTC_GLOBAL_OAM_POLICER_EN:
        if (DRV_IS_DUET2(lchip))
        {
            cmd = DRV_IOR(IpeFwdCtl_t, IpeFwdCtl_oamBypassPolicingOp_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        }
        else
        {
            cmd = DRV_IOR(IpeAclQosCtl_t, IpeAclQosCtl_oamBypassPolicingOp_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        }
        *(bool *)value = (field_val)?FALSE:TRUE;
        break;
    case CTC_GLOBAL_PANEL_PORTS:
        _sys_usw_global_get_panel_ports(lchip, (ctc_global_panel_ports_t*)value);
        break;
	case CTC_GLOBAL_PARSER_OUTER_ALWAYS_CVLAN_EN:
        cmd = DRV_IOR(ParserEthernetCtl_t, ParserEthernetCtl_outerAlwaysCvlan_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool *)value = (field_val)? TRUE : FALSE;
        break;
    case CTC_GLOBAL_VPWS_SNOOPING_PARSER:
        cmd = DRV_IOR(IpeTunnelIdCtl_t, IpeTunnelIdCtl_vpwsDecapsForceSecondParser_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool *)value = (field_val) ? TRUE : FALSE;
        break;

    case CTC_GLOBAL_ARP_VLAN_CLASS_EN:
        cmd = DRV_IOR(IpeUserIdCtl_t, IpeUserIdCtl_arpForceIpv4_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32*)value = field_val;
        break;
    case CTC_GLOBAL_NH_ARP_MACDA_DERIVE_MODE:
        cmd = DRV_IOR(EpeNextHopCtl_t, EpeNextHopCtl_hostRouteNextHopSaveMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32 *)value =field_val;
        break;
    case CTC_GLOBAL_VXLAN_CRYPT_UDP_DEST_PORT:
        cmd = DRV_IOR(ParserLayer4AppCtl_t, ParserLayer4AppCtl_encryptVxlanUdpDestPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32 *)value =field_val;
        break;

    case CTC_GLOBAL_DISCARD_MACSA_0_PKT:
        cmd = DRV_IOR(IpeIntfMapperCtl_t, IpeIntfMapperCtl_discardAllZeroMacSa_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool*)value = field_val;
        break;
    case CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST:
        if (!DRV_IS_DUET2(lchip))
        {
            return CTC_E_NOT_SUPPORT;
        }
        *(uint32 *)value = p_usw_register_master[lchip]->tpoam_vpws_coexist ? 1 : 0;
        break;
    case CTC_GLOBAL_VXLAN_POLICER_GROUP_ID_BASE:
        cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_vxlanPolicyGidBase_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32 *)value =field_val;
        break;
    case CTC_GLOBAL_IGS_RANDOM_LOG_SHIFT:
        cmd = DRV_IOR(IpeUserIdCtl_t, IpeUserIdCtl_randomLogShift_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32*)value = field_val;
        break;
    case CTC_GLOBAL_EGS_RANDOM_LOG_SHIFT:
        cmd = DRV_IOR(EpeNextHopCtl_t, EpeNextHopCtl_randomLogShift_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32*)value = field_val;
        break;
    case CTC_GLOBAL_OVERLAY_DECAP_MODE:
        CTC_MAX_VALUE_CHECK(p_decap_mode->scl_id, 1);
        cmd = DRV_IOR(IpeUserIdCtl_t, DRV_IS_DUET2(lchip) ? \
            ((p_decap_mode->scl_id)? IpeUserIdCtl_vxlanTunnelHash2LkpMode_f: IpeUserIdCtl_vxlanTunnelHash1LkpMode_f):\
            ((p_decap_mode->scl_id)? IpeUserIdCtl_vxlanTunnelHash1LkpMode_f: IpeUserIdCtl_vxlanTunnelHash0LkpMode_f));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        p_decap_mode->vxlan_mode = field_val ? 0:1;
        cmd = DRV_IOR(IpeUserIdCtl_t, DRV_IS_DUET2(lchip) ? \
            ((p_decap_mode->scl_id)? IpeUserIdCtl_nvgreTunnelHash2LkpMode_f: IpeUserIdCtl_nvgreTunnelHash1LkpMode_f):\
            ((p_decap_mode->scl_id)? IpeUserIdCtl_nvgreTunnelHash1LkpMode_f: IpeUserIdCtl_nvgreTunnelHash0LkpMode_f));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        p_decap_mode->nvgre_mode = field_val ? 0:1;
        break;
    case CTC_GLOBAL_EGS_STK_ACL_DIS:
        cmd = DRV_IOR(EpeAclQosCtl_t, EpeAclQosCtl_stackingDisableAcl_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool*)value = field_val;
        break;
   case CTC_GLOBAL_STK_WITH_IGS_PKT_HDR_EN:
        cmd = DRV_IOR(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_toRemoteCpuAttachHeader_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool*)value = field_val;
        break;
    default:
        return CTC_E_NOT_SUPPORT;

    }


    return CTC_E_NONE;
}

int32
sys_usw_wb_sync_register_cb(uint8 lchip, uint8 module, sys_wb_sync_fn fn)
{
    if (NULL == p_usw_register_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }

    CTC_MAX_VALUE_CHECK(module, CTC_FEATURE_MAX - 1);

    p_usw_register_master[lchip]->wb_sync_cb[module] = fn;

    return CTC_E_NONE;
}

int32
sys_usw_dump_db_register_cb(uint8 lchip, uint8 module, CTC_DUMP_MASTER_FUNC fn)
{
    if (NULL == p_usw_chip_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }

    CTC_MAX_VALUE_CHECK(module, CTC_FEATURE_MAX - 1);

    p_usw_chip_master[lchip]->dump_cb[module] = fn;

    return CTC_E_NONE;
}

int32
sys_usw_register_api_cb(uint8 lchip, uint8 type, void* cb)
{
    if (NULL == p_usw_register_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }

    switch(type)
    {
        case SYS_REGISTER_CB_TYPE_DOT1AE_BIND_SEC_CHAN:
            p_usw_register_master[lchip]->p_register_api->dot1ae_bind_sec_chan = cb;
            break;

        case SYS_REGISTER_CB_TYPE_DOT1AE_UNBIND_SEC_CHAN:
            p_usw_register_master[lchip]->p_register_api->dot1ae_unbind_sec_chan = cb;
            break;

        case SYS_REGISTER_CB_TYPE_DOT1AE_GET_BIND_SEC_CHAN:
            p_usw_register_master[lchip]->p_register_api->dot1ae_get_bind_sec_chan = cb;
            break;

        case SYS_REGISTER_CB_TYPE_STK_GET_STKHDR_LEN:
            p_usw_register_master[lchip]->p_register_api->stacking_get_stkhdr_len = cb;
            break;

        case SYS_REGISTER_CB_TYPE_STK_GET_RSV_TRUNK_NUM:
            p_usw_register_master[lchip]->p_register_api->stacking_get_rsv_trunk_number = cb;
            break;

        case SYS_REGISTER_CB_TYPE_STK_GET_MCAST_PROFILE_MET_OFFSET:
            p_usw_register_master[lchip]->p_register_api->stacking_get_mcast_profile_met_offset = cb;
            break;

        case SYS_REGISTER_CB_TYPE_INBAND_PORT_EN:
            p_usw_register_master[lchip]->p_register_api->inband_port_en = cb;
            break;

        case SYS_REGISTER_CB_TYPE_INBAND_TIMER:
            p_usw_register_master[lchip]->p_register_api->inband_timer = cb;
            break;

        case SYS_REGISTER_CB_TYPE_INBAND_TX:
            p_usw_register_master[lchip]->p_register_api->inband_tx = cb;
            break;

        case SYS_REGISTER_CB_TYPE_INBAND_RX:
            p_usw_register_master[lchip]->p_register_api->inband_rx = cb;
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}
int32
sys_usw_global_set_logic_destport_en(uint8 lchip, uint8 enable)
{
    uint32 cmd = 0;
    uint32 value = 0;

    if (NULL == p_usw_register_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }
    value = enable?1:0;
    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_headerCarryLogicDestPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_headerCarryLogicDestPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(QMgrCtl_t, QMgrCtl_queueEntryCarryLogicDestPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(QMgrCtl_t, QMgrCtl_logicDestPort64kMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(QWriteCtl_t, QWriteCtl_queueEntryCarryLogicDestPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(QWriteCtl_t, QWriteCtl_logicDestPort64kMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(BufferRetrieveCtl_t, BufferRetrieveCtl_queueEntryCarryLogicDestPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(BufferStoreCtl_t, BufferStoreCtl_headerCarryLogicDestPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}

int32
sys_usw_global_get_logic_destport_en(uint8 lchip, uint8* enable)
{
    uint32 cmd = 0;
    uint32 value = 0;

    cmd = DRV_IOR(BufferStoreCtl_t, BufferStoreCtl_headerCarryLogicDestPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    *enable = value;

    return CTC_E_NONE;
}
int32
sys_usw_global_set_gint_en(uint8 lchip, uint8 enable)
{
    if (NULL == p_usw_register_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }
    p_usw_register_master[lchip]->gint_en = enable;

    return CTC_E_NONE;
}

int32
sys_usw_global_get_gint_en(uint8 lchip, uint8* enable)
{
    if (NULL == p_usw_register_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }

    CTC_PTR_VALID_CHECK(enable);


    *enable = p_usw_register_master[lchip]->gint_en;

    return CTC_E_NONE;
}

int32
sys_usw_global_set_xgpon_en(uint8 lchip, uint8 enable)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 offset = 0;
    IpeBridgeCtl_m ipe_bridge_ctl;

    if (NULL == p_usw_register_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }

    CTC_ERROR_RETURN(sys_usw_acl_set_global_cfg(lchip, 5, enable));     /*Gem port enable*/

    if (DRV_IS_DUET2(lchip))
    {
        value = enable?0:1;
        cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_hwVsiMacLimitEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        cmd = DRV_IOR(IpeUserIdCtl_t, IpeUserIdCtl_macPortTypeMacSaFieldShareType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        value = enable?(value|0x1):(value&0x2);
        cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_macPortTypeMacSaFieldShareType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        value = enable?1:0;
        cmd = DRV_IOW(IpeLoopbackHeaderAdjustCtl_t, IpeLoopbackHeaderAdjustCtl_customerLabelMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        value = enable?1:0;
        cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_srcMismatchDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        value = enable?1:0;
        cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_srcMismatchDiscardException_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        value = enable?1:0;
        cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_srcMismatchNoDiscardException_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        value = enable?1:0;
        cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_srcMismatchException_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        value = enable?0:1;
        cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_discardMplsTagTtl0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        value = enable?0:1;
        cmd = DRV_IOW(FlowTcamLookupCtl_t, FlowTcamLookupCtl_coppKeyIpAddrShareType0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    }

    if (DRV_IS_TSINGMA(lchip))
    {
        value = enable?1:0;
        cmd = DRV_IOW(IpeLoopbackHeaderAdjustCtl_t, IpeLoopbackHeaderAdjustCtl_logicSrcPort64kMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        cmd = DRV_IOW(BufferStoreCtl_t, BufferStoreCtl_logicSrcPort64kMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_dot1AeFieldShareMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_ponAppEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        cmd = DRV_IOW(IpeBridgeCtl_t, IpeBridgeCtl_ponAppEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        cmd = DRV_IOW(FibAccelerationCtl_t, FibAccelerationCtl_ponAppEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        cmd = DRV_IOW(FibEngineLookupCtl_t, FibEngineLookupCtl_ponAppEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_gemPortRecoverEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        /*cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_gemPortRecoverFromNexthopEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));*/

        cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_nhpOuterEditReadValid_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_voltEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        /*set oun flex>onu>pon flex>pon*/
        value = enable?4:0;
        cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_userIdResultPriorityMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        value = enable?0:1;
        cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_hwVsiMacLimitEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        sal_memset(&ipe_bridge_ctl, 0, sizeof(IpeBridgeCtl_m));

        CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BRIDGE_NH,
                                                                   &offset));
        cmd = DRV_IOR(IpeBridgeCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_bridge_ctl));
        SetIpeBridgeCtl(V, macDaPolicerControlBit_f, &ipe_bridge_ctl, 1);
        SetIpeBridgeCtl(V, ponNextHop_0_ponBypassIngressEdit_f, &ipe_bridge_ctl, 1);
        SetIpeBridgeCtl(V, ponNextHop_0_ponLengthAdjustType_f, &ipe_bridge_ctl, 0);
        SetIpeBridgeCtl(V, ponNextHop_0_ponNextHopExt_f, &ipe_bridge_ctl, 0);
        SetIpeBridgeCtl(V, ponNextHop_0_ponNextHopPtr_f, &ipe_bridge_ctl, offset);
        SetIpeBridgeCtl(V, ponNextHop_1_ponBypassIngressEdit_f, &ipe_bridge_ctl, 1);
        SetIpeBridgeCtl(V, ponNextHop_1_ponLengthAdjustType_f, &ipe_bridge_ctl, 0);
        SetIpeBridgeCtl(V, ponNextHop_1_ponNextHopExt_f, &ipe_bridge_ctl, 0);
        SetIpeBridgeCtl(V, ponNextHop_1_ponNextHopPtr_f, &ipe_bridge_ctl, offset);
        SetIpeBridgeCtl(V, macDaPolicerLevel_f, &ipe_bridge_ctl, 0);
        SetIpeBridgeCtl(V, macDaPolicerPhbEn_f, &ipe_bridge_ctl, 1);
        cmd = DRV_IOW(IpeBridgeCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_bridge_ctl));

        CTC_ERROR_RETURN(sys_usw_global_set_logic_destport_en(lchip, enable));
        /* xgpon use logic dst port enq*/
        value = 0;
        cmd = DRV_IOW(QWriteCtl_t, QWriteCtl_useNextHopPtr_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    }
    p_usw_register_master[lchip]->xgpon_en = enable;

    return CTC_E_NONE;
}

int32
sys_usw_global_get_xgpon_en(uint8 lchip, uint8* enable)
{
    if (NULL == p_usw_register_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }

    CTC_PTR_VALID_CHECK(enable);


    *enable = p_usw_register_master[lchip]->xgpon_en;

    return CTC_E_NONE;
}

int32
sys_usw_register_add_truncation_profile(uint8 lchip, uint16 length, uint8 old_profile_id, uint8* profile_id)
{
    uint8 shift = 0;
    uint32 cmd = 0;
    DsTruncationProfile_m truncation_prof;
    sys_usw_register_tcat_prof_t tcat_prof_new;
    sys_usw_register_tcat_prof_t  tcat_prof_old;
    sys_usw_register_tcat_prof_t* p_tcat_prof_old = NULL;
    sys_usw_register_tcat_prof_t* tcat_prof_out = NULL;

    SYS_REGISTER_INIT_CHECK;
    CTC_PTR_VALID_CHECK(profile_id);

    CTC_MIN_VALUE_CHECK(length,1);
    CTC_MAX_VALUE_CHECK(length,MCHIP_CAP(SYS_CAP_PKT_TRUNCATED_LEN));

    shift = (length >> 8) & 0x7;
    if (IS_BIT_SET(shift, 2))
    {
        shift = 3;
    }
    else if (IS_BIT_SET(shift, 1))
    {
        shift = 2;
    }
    else if (IS_BIT_SET(shift, 0))
    {
        shift = 1;
    }
    else
    {
        shift = 0;
    }

    SYS_USW_REGISTER_LOCK(lchip);
    sal_memset(&tcat_prof_new, 0, sizeof(tcat_prof_new));

    tcat_prof_new.shift = shift;
    tcat_prof_new.length = (length) / (1 << shift);

    if (old_profile_id)
    {
        sal_memset(&tcat_prof_old, 0, sizeof(tcat_prof_old));
        sal_memset(&truncation_prof, 0, sizeof(truncation_prof));
        cmd = DRV_IOR(DsTruncationProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, old_profile_id , cmd, &truncation_prof), p_usw_register_master[lchip]->register_mutex);

        tcat_prof_old.shift = GetDsTruncationProfile(V, lengthShift_f, &truncation_prof);
        tcat_prof_old.length = GetDsTruncationProfile(V, length_f, &truncation_prof);
        p_tcat_prof_old = &tcat_prof_old;
    }

    CTC_ERROR_RETURN_WITH_UNLOCK(ctc_spool_add(p_usw_register_master[lchip]->tcat_prof_spool, &tcat_prof_new, p_tcat_prof_old, &tcat_prof_out), p_usw_register_master[lchip]->register_mutex);
    if (NULL == tcat_prof_out)
    {
        SYS_USW_REGISTER_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    if (ctc_spool_get_refcnt(p_usw_register_master[lchip]->tcat_prof_spool, &tcat_prof_new) == 1)
    {
        sal_memset(&truncation_prof, 0, sizeof(truncation_prof));
        cmd = DRV_IOW(DsTruncationProfile_t, DRV_ENTRY_FLAG);
        SetDsTruncationProfile(V, lengthShift_f, &truncation_prof, tcat_prof_new.shift);
        SetDsTruncationProfile(V, length_f, &truncation_prof, tcat_prof_new.length);
        CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, tcat_prof_out->profile_id, cmd, &truncation_prof), p_usw_register_master[lchip]->register_mutex);
    }
    *profile_id = tcat_prof_out->profile_id;
    SYS_USW_REGISTER_UNLOCK(lchip);
    return CTC_E_NONE;
}

/*
mode 0 : remove by length
mode 1 : remove by profile_id
*/
int32
sys_usw_register_remove_truncation_profile(uint8 lchip, uint8 mode, uint16 len_or_profile_id)
{
    uint8  shift = 0;
    uint16  length = 0;
    uint32 cmd = 0;
    DsTruncationProfile_m truncation_prof;
    sys_usw_register_tcat_prof_t tcat_prof;
    sys_usw_register_tcat_prof_t* tcat_prof_get = NULL;

    SYS_REGISTER_INIT_CHECK;
    if (len_or_profile_id == 0)
    {
        return CTC_E_NONE;
    }

    if (mode == 0)
    {
        /* remove by length */
        CTC_MIN_VALUE_CHECK(len_or_profile_id, 1);
        CTC_MAX_VALUE_CHECK(len_or_profile_id, MCHIP_CAP(SYS_CAP_PKT_TRUNCATED_LEN));

        shift = (len_or_profile_id >> 8) & 0x7;
        if (IS_BIT_SET(shift, 2))
        {
            shift = 3;
        }
        else if (IS_BIT_SET(shift, 1))
        {
            shift = 2;
        }
        else if (IS_BIT_SET(shift, 0))
        {
            shift = 1;
        }
        else
        {
            shift = 0;
        }
        length = (len_or_profile_id) / (1 << shift);
    }
    else
    {
        /* remove by profile_id */
        CTC_MAX_VALUE_CHECK(len_or_profile_id, 0xF);
        sal_memset(&truncation_prof, 0, sizeof(truncation_prof));
        cmd = DRV_IOR(DsTruncationProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, len_or_profile_id , cmd, &truncation_prof));

        shift = GetDsTruncationProfile(V, lengthShift_f, &truncation_prof);
        length = GetDsTruncationProfile(V, length_f, &truncation_prof);
    }

    SYS_USW_REGISTER_LOCK(lchip);
    sal_memset(&tcat_prof, 0, sizeof(tcat_prof));
    tcat_prof.shift = shift;
    tcat_prof.length = length;

    CTC_ERROR_RETURN_WITH_UNLOCK(ctc_spool_remove(p_usw_register_master[lchip]->tcat_prof_spool, &tcat_prof, &tcat_prof_get), p_usw_register_master[lchip]->register_mutex);

    if (NULL == tcat_prof_get)
    {
        SYS_USW_REGISTER_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    if (ctc_spool_get_refcnt(p_usw_register_master[lchip]->tcat_prof_spool, &tcat_prof) == 0)
    {
        sal_memset(&truncation_prof, 0, sizeof(truncation_prof));
        cmd = DRV_IOW(DsTruncationProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, tcat_prof.profile_id, cmd, &truncation_prof), p_usw_register_master[lchip]->register_mutex);
    }

    SYS_USW_REGISTER_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_global_failover_en(uint8 lchip, uint32 enable)
{
    uint32 cmd = 0;
    uint32 value = 0;

    value = enable?1:0;

    /* linkagg failover */
    cmd = DRV_IOW(LagEngineCtl_t, LagEngineCtl_linkChangeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    /* aps failover aps */
    cmd = DRV_IOW(MetFifoCtl_t, MetFifoCtl_linkChangeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}

