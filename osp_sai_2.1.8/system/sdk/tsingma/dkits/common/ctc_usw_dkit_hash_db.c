#include "sal.h"
#include "drv_api.h"
#include "usw/include/drv_common.h"
#include "ctc_cli.h"
#include "ctc_dkit.h"
#include "ctc_dkit_cli.h"
#include "ctc_dkit_common.h"
#include "ctc_usw_dkit_hash_db.h"

ctc_dkits_hash_db_info_t ctc_dkits_hash_tbl_egs_xc_oam[] =
{
    {
        EGRESSXCOAMHASHTYPE_BFD,
        DsEgressXcOamBfdHashKey_t,
        MaxTblId_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        EGRESSXCOAMHASHTYPE_CVLANCOSPORT,
        DsEgressXcOamCvlanCosPortHashKey_t,
        MaxTblId_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        EGRESSXCOAMHASHTYPE_CVLANPORT,
        DsEgressXcOamCvlanPortHashKey_t,
        MaxTblId_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        EGRESSXCOAMHASHTYPE_DOUBLEVLANPORT,
        DsEgressXcOamDoubleVlanPortHashKey_t,
        MaxTblId_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        EGRESSXCOAMHASHTYPE_ETH,
        DsEgressXcOamEthHashKey_t,
        MaxTblId_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        EGRESSXCOAMHASHTYPE_MPLSLABEL,
        DsEgressXcOamMplsLabelHashKey_t,
        MaxTblId_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        EGRESSXCOAMHASHTYPE_MPLSSECTION,
        DsEgressXcOamMplsSectionHashKey_t,
        MaxTblId_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        EGRESSXCOAMHASHTYPE_PORTCROSS,
        DsEgressXcOamPortCrossHashKey_t,
        MaxTblId_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        EGRESSXCOAMHASHTYPE_PORT,
        DsEgressXcOamPortHashKey_t,
        MaxTblId_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        EGRESSXCOAMHASHTYPE_PORTVLANCROSS,
        DsEgressXcOamPortVlanCrossHashKey_t,
        MaxTblId_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        EGRESSXCOAMHASHTYPE_RMEP,
        DsEgressXcOamRmepHashKey_t,
        MaxTblId_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        EGRESSXCOAMHASHTYPE_SVLANCOSPORT,
        DsEgressXcOamSvlanCosPortHashKey_t,
        MaxTblId_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        EGRESSXCOAMHASHTYPE_SVLANPORT,
        DsEgressXcOamSvlanPortHashKey_t,
        MaxTblId_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        EGRESSXCOAMHASHTYPE_SVLANPORTMAC,
        DsEgressXcOamSvlanPortMacHashKey_t,
        MaxTblId_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        EGRESSXCOAMHASHTYPE_TUNNELPBB,
        DsEgressXcOamTunnelPbbHashKey_t,
        MaxTblId_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
};

ctc_dkits_hash_db_info_t ctc_dkits_hash_tbl_fib_host0[] =
{
    {
        FIBHOST0HASHTYPE_FCOE,
        DsFibHost0FcoeHashKey_t,
        DsFcoeDa_t,
        DsFibHost0FcoeHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        FIBHOST0HASHTYPE_IPV4,
        DsFibHost0Ipv4HashKey_t,
        DsIpDa_t,
        DsFibHost0Ipv4HashKey_dsAdIndex_f,
        85,
        1
    },
    {
        FIBHOST0HASHTYPE_IPV6MCAST,
        DsFibHost0Ipv6McastHashKey_t,
        DsIpDa_t,
        DsFibHost0Ipv6McastHashKey_dsAdIndex_f,
        85,
        2
    },
    {
        FIBHOST0HASHTYPE_IPV6UCAST,
        DsFibHost0Ipv6UcastHashKey_t,
        DsIpDa_t,
        DsFibHost0Ipv6UcastHashKey_dsAdIndex_f,
        85,
        2
    },
    {
        FIBHOST0HASHTYPE_MAC,
        DsFibHost0MacHashKey_t,
        DsMac_t,
        DsFibHost0MacHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        FIBHOST0HASHTYPE_MACIPV6MCAST,
        DsFibHost0MacIpv6McastHashKey_t,
        DsMac_t,
        DsFibHost0MacIpv6McastHashKey_dsAdIndex_f,
        85,
        2
    },
    {
        FIBHOST0HASHTYPE_TRILL,
        DsFibHost0TrillHashKey_t,
        DsTrillDa_t,
        DsFibHost0TrillHashKey_dsAdIndex_f,
        85,
        1
    }
};

ctc_dkits_hash_db_info_t ctc_dkits_hash_tbl_fib_host1[] =
{
    {
        FIBHOST1HASHTYPE_FCOERPF,
        DsFibHost1FcoeRpfHashKey_t,
        DsFcoeSa_t,
        DsFibHost1FcoeRpfHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        FIBHOST1HASHTYPE_IPV4MCAST,
        DsFibHost1Ipv4McastHashKey_t,
        DsIpDa_t,
        DsFibHost1Ipv4McastHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        FIBHOST1HASHTYPE_IPV4NATDAPORT,
        DsFibHost1Ipv4NatDaPortHashKey_t,
        DsIpSaNat_t,
        DsFibHost1Ipv4NatDaPortHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        FIBHOST1HASHTYPE_IPV4NATSAPORT,
        DsFibHost1Ipv4NatSaPortHashKey_t,
        DsIpSaNat_t,
        DsFibHost1Ipv4NatSaPortHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        FIBHOST1HASHTYPE_IPV6MCAST,
        DsFibHost1Ipv6McastHashKey_t,
        DsIpDa_t,
        DsFibHost1Ipv6McastHashKey_dsAdIndex_f,
        85,
        2
    },
    {
        FIBHOST1HASHTYPE_IPV6NATDAPORT,
        DsFibHost1Ipv6NatDaPortHashKey_t,
        DsIpSaNat_t,
        DsFibHost1Ipv6NatDaPortHashKey_dsAdIndex_f,
        85,
        2
    },
    {
        FIBHOST1HASHTYPE_IPV6NATSAPORT,
        DsFibHost1Ipv6NatSaPortHashKey_t,
        DsIpSaNat_t,
        DsFibHost1Ipv6NatSaPortHashKey_dsAdIndex_f,
        85,
        2
    },
    {
        FIBHOST1HASHTYPE_MACIPV4MCAST,
        DsFibHost1MacIpv4McastHashKey_t,
        DsMac_t,
        DsFibHost1MacIpv4McastHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        FIBHOST1HASHTYPE_MACIPV6MCAST,
        DsFibHost1MacIpv6McastHashKey_t,
        DsMac_t,
        DsFibHost1MacIpv6McastHashKey_dsAdIndex_f,
        85,
        2
    },
    {
        FIBHOST1HASHTYPE_TRILLMCASTVLAN,
        DsFibHost1TrillMcastVlanHashKey_t,
        DsTrillDa_t,
        DsFibHost1TrillMcastVlanHashKey_dsAdIndex_f,
        85,
        1
    }
};

ctc_dkits_hash_db_info_t ctc_dkits_hash_tbl_flow[] =
{
    {
        FLOWHASHTYPE_L2,
        DsFlowL2HashKey_t,
        DsFlow_t,
        DsFlowL2HashKey_dsAdIndex_f,
        85,
        2
    },
    {
        FLOWHASHTYPE_L2L3,
        DsFlowL2L3HashKey_t,
        DsFlow_t,
        DsFlowL2L3HashKey_dsAdIndex_f,
        85,
        4
    },
    {
        FLOWHASHTYPE_L3IPV4,
        DsFlowL3Ipv4HashKey_t,
        DsFlow_t,
        DsFlowL3Ipv4HashKey_dsAdIndex_f,
        85,
        2
    },
    {
        FLOWHASHTYPE_L3IPV6,
        DsFlowL3Ipv6HashKey_t,
        DsFlow_t,
        DsFlowL3Ipv6HashKey_dsAdIndex_f,
        85,
        4
    },
    {
        FLOWHASHTYPE_L3MPLS,
        DsFlowL3MplsHashKey_t,
        DsFlow_t,
        DsFlowL3MplsHashKey_dsAdIndex_f,
        85,
        2
    }
};

ctc_dkits_hash_db_info_t ctc_dkits_hash_tbl_ipfix[] =
{
    {
        IPFIXHASHTYPE_L2,
        DsIpfixL2HashKey_t,
        DsIpfixSessionRecord_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        IPFIXHASHTYPE_L2L3,
        DsIpfixL2L3HashKey_t,
        DsIpfixSessionRecord_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        4
    },
    {
        IPFIXHASHTYPE_L3IPV4,
        DsIpfixL3Ipv4HashKey_t,
        DsIpfixSessionRecord_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    },
    {
        IPFIXHASHTYPE_L3IPV6,
        DsIpfixL3Ipv6HashKey_t,
        DsIpfixSessionRecord_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        4
    },
    {
        IPFIXHASHTYPE_L3MPLS,
        DsIpfixL3MplsHashKey_t,
        DsIpfixSessionRecord_t,
        CTC_DKITS_TBL_INVALID_FLDID,
        85,
        2
    }
};

ctc_dkits_hash_db_info_t ctc_dkits_hash_tbl_userid[] =
{
    {
        USERIDHASHTYPE_CAPWAPMACDAFORWARD,
        DsUserIdCapwapMacDaForwardHashKey_t,
        DsUserId_t,
        DsUserIdCapwapMacDaForwardHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_CAPWAPSTASTATUS,
        DsUserIdCapwapStaStatusHashKey_t,
        DsUserId_t,
        DsUserIdCapwapStaStatusHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_CAPWAPSTASTATUSMC,
        DsUserIdCapwapStaStatusMcHashKey_t,
        DsUserId_t,
        DsUserIdCapwapStaStatusMcHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_CAPWAPVLANFORWARD,
        DsUserIdCapwapVlanForwardHashKey_t,
        DsUserId_t,
        DsUserIdCapwapVlanForwardHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_CVLANCOSPORT,
        DsUserIdCvlanCosPortHashKey_t,
        DsUserId_t,
        DsUserIdCvlanCosPortHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_CVLANPORT,
        DsUserIdCvlanPortHashKey_t,
        DsUserId_t,
        DsUserIdCvlanPortHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_DOUBLEVLANPORT,
        DsUserIdDoubleVlanPortHashKey_t,
        DsUserId_t,
        DsUserIdDoubleVlanPortHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_ECIDNAMESPACE,
        DsUserIdEcidNameSpaceHashKey_t,
        DsUserId_t,
        DsUserIdEcidNameSpaceHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_INGECIDNAMESPACE,
        DsUserIdIngEcidNameSpaceHashKey_t,
        DsUserId_t,
        DsUserIdIngEcidNameSpaceHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_IPV4PORT,
        DsUserIdIpv4PortHashKey_t,
        DsUserId_t,
        DsUserIdIpv4PortHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_IPV4SA,
        DsUserIdIpv4SaHashKey_t,
        DsUserId_t,
        DsUserIdIpv4SaHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_IPV6PORT,
        DsUserIdIpv6PortHashKey_t,
        DsUserId_t,
        DsUserIdIpv6PortHashKey_dsAdIndex_f,
        85,
        4
    },
    {
        USERIDHASHTYPE_IPV6SA,
        DsUserIdIpv6SaHashKey_t,
        DsUserId_t,
        DsUserIdIpv6SaHashKey_dsAdIndex_f,
        85,
        2
    },
    {
        USERIDHASHTYPE_MAC,
        DsUserIdMacHashKey_t,
        DsUserId_t,
        DsUserIdMacHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_MACPORT,
        DsUserIdMacPortHashKey_t,
        DsUserId_t,
        DsUserIdMacPortHashKey_dsAdIndex_f,
        85,
        2
    },
    {
        USERIDHASHTYPE_PORT,
        DsUserIdPortHashKey_t,
        DsUserId_t,
        DsUserIdPortHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        0x35,
        DsUserIdSclFlowL2HashKey_t,
        DsSclFlow_t,
        DsUserIdSclFlowL2HashKey_dsAdIndex_f,
        85,
        2
    },
    {
        USERIDHASHTYPE_SVLANCOSPORT,
        DsUserIdSvlanCosPortHashKey_t,
        DsUserId_t,
        DsUserIdSvlanCosPortHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_SVLAN,
        DsUserIdSvlanHashKey_t,
        DsUserId_t,
        DsUserIdSvlanHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_SVLANMACSA,
        DsUserIdSvlanMacSaHashKey_t,
        DsUserId_t,
        DsUserIdSvlanMacSaHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_SVLANPORT,
        DsUserIdSvlanPortHashKey_t,
        DsUserId_t,
        DsUserIdSvlanPortHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELCAPWAPRMAC,
        DsUserIdTunnelCapwapRmacHashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelCapwapRmacHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELCAPWAPRMACRID,
        DsUserIdTunnelCapwapRmacRidHashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelCapwapRmacRidHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELIPV4CAPWAP,
        DsUserIdTunnelIpv4CapwapHashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv4CapwapHashKey_dsAdIndex_f,
        85,
        2
    },
    {
        0x33,
        DsUserIdTunnelIpv4DaHashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv4DaHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELIPV4GREKEY,
        DsUserIdTunnelIpv4GreKeyHashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv4GreKeyHashKey_dsAdIndex_f,
        85,
        2
    },
    {
        USERIDHASHTYPE_TUNNELIPV4,
        DsUserIdTunnelIpv4HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv4HashKey_dsAdIndex_f,
        85,
        2
    },
    {
        USERIDHASHTYPE_TUNNELIPV4MCNVGREMODE0,
        DsUserIdTunnelIpv4McNvgreMode0HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv4McNvgreMode0HashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELIPV4MCVXLANMODE0,
        DsUserIdTunnelIpv4McVxlanMode0HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv4McVxlanMode0HashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELIPV4NVGREMODE1,
        DsUserIdTunnelIpv4NvgreMode1HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv4NvgreMode1HashKey_dsAdIndex_f,
        85,
        2
    },
    {
        USERIDHASHTYPE_TUNNELIPV4RPF,
        DsUserIdTunnelIpv4RpfHashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv4RpfHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE0,
        DsUserIdTunnelIpv4UcNvgreMode0HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv4UcNvgreMode0HashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE1,
        DsUserIdTunnelIpv4UcNvgreMode1HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv4UcNvgreMode1HashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE0,
        DsUserIdTunnelIpv4UcVxlanMode0HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv4UcVxlanMode0HashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE1,
        DsUserIdTunnelIpv4UcVxlanMode1HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv4UcVxlanMode1HashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELIPV4UDP,
        DsUserIdTunnelIpv4UdpHashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv4UdpHashKey_dsAdIndex_f,
        85,
        2
    },
    {
        USERIDHASHTYPE_TUNNELIPV4VXLANMODE1,
        DsUserIdTunnelIpv4VxlanMode1HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv4VxlanMode1HashKey_dsAdIndex_f,
        85,
        2
    },
    {
        USERIDHASHTYPE_TUNNELIPV6CAPWAP,
        DsUserIdTunnelIpv6CapwapHashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv6CapwapHashKey_dsAdIndex_f,
        85,
        4
    },
    {
        USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE0,
        DsUserIdTunnelIpv6McNvgreMode0HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv6McNvgreMode0HashKey_dsAdIndex_f,
        85,
        4
    },
    {
        USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE1,
        DsUserIdTunnelIpv6McNvgreMode1HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv6McNvgreMode1HashKey_dsAdIndex_f,
        85,
        4
    },
    {
        USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE0,
        DsUserIdTunnelIpv6McVxlanMode0HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv6McVxlanMode0HashKey_dsAdIndex_f,
        85,
        4
    },
    {
        USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE1,
        DsUserIdTunnelIpv6McVxlanMode1HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv6McVxlanMode1HashKey_dsAdIndex_f,
        85,
        4
    },
    {
        USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE0,
        DsUserIdTunnelIpv6UcNvgreMode0HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv6UcNvgreMode0HashKey_dsAdIndex_f,
        85,
        4
    },
    {
        USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE1,
        DsUserIdTunnelIpv6UcNvgreMode1HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv6UcNvgreMode1HashKey_dsAdIndex_f,
        85,
        4
    },
    {
        USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE0,
        DsUserIdTunnelIpv6UcVxlanMode0HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv6UcVxlanMode0HashKey_dsAdIndex_f,
        85,
        4
    },
    {
        USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE1,
        DsUserIdTunnelIpv6UcVxlanMode1HashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelIpv6UcVxlanMode1HashKey_dsAdIndex_f,
        85,
        4
    },
    {
        USERIDHASHTYPE_TUNNELMPLS,
        DsUserIdTunnelMplsHashKey_t,
        DsMpls_t,
        DsUserIdTunnelMplsHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELPBB,
        DsUserIdTunnelPbbHashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelPbbHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELTRILLMCADJ,
        DsUserIdTunnelTrillMcAdjHashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelTrillMcAdjHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELTRILLMCDECAP,
        DsUserIdTunnelTrillMcDecapHashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelTrillMcDecapHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELTRILLMCRPF,
        DsUserIdTunnelTrillMcRpfHashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelTrillMcRpfHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELTRILLUCDECAP,
        DsUserIdTunnelTrillUcDecapHashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelTrillUcDecapHashKey_dsAdIndex_f,
        85,
        1
    },
    {
        USERIDHASHTYPE_TUNNELTRILLUCRPF,
        DsUserIdTunnelTrillUcRpfHashKey_t,
        DsTunnelId_t,
        DsUserIdTunnelTrillUcRpfHashKey_dsAdIndex_f,
        85,
        1
    }
};

ctc_dkits_hash_level_t  ctc_dkits_hash_egress_xc_oam_level[] =
{
    {4096, 2},
    {2048, 2},
    {32,   2}
};

ctc_dkits_hash_level_t  ctc_dkits_hash_fib_host0_level[] =
{
    {4096, 4},
    {4096, 4},
    {4096, 4},
    {2048, 4},
    {2048, 4},
    {2048, 4},
    {8,    4}
};

ctc_dkits_hash_level_t  ctc_dkits_hash_fib_host1_level[] =
{
    {2048, 4},
    {2048, 4},
    {1024, 4},
    {1024, 4},
    {512,  4}
};

ctc_dkits_hash_level_t  ctc_dkits_hash_flow_level[] =
{
    {4096, 2},
    {4096, 2},
    {4096, 2},
    {2048, 2},
    {2048, 2},
    {8,    2}
};

ctc_dkits_hash_level_t  ctc_dkits_hash_ipfix_level[] =
{
    {2048, 2},
    {2048, 2},
    {1024, 2},
    {1024, 2}
};

ctc_dkits_hash_level_t  ctc_dkits_hash_userid_level[] =
{
    {2048, 4},
    {2048, 4},
    {1024, 4},
    {1024, 4},
    {512,  4}
};

ctc_dkits_hash_db_t ctc_dkits_hash_db[] =
{
    {   /* EgressXcOam */
        15,
        2,
        ctc_dkits_hash_tbl_egs_xc_oam,
        ctc_dkits_hash_egress_xc_oam_level,
        DsEgressXcOamHashCam_t
    },
    {   /* FibHost0 */
        7,
        6,
        ctc_dkits_hash_tbl_fib_host0,
        ctc_dkits_hash_fib_host0_level,
        DsFibHost0HashCam_t
    },
    {   /* FibHost1 */
        10,
        5,
        ctc_dkits_hash_tbl_fib_host1,
        ctc_dkits_hash_fib_host1_level,
        DsFibHost1HashCam_t
    },
    {   /* Flow */
        5,
        5,
        ctc_dkits_hash_tbl_flow,
        ctc_dkits_hash_flow_level,
        DsFlowHashCam_t
    },
    {   /* Ipfix */
        5,
        4,
        ctc_dkits_hash_tbl_ipfix,
        ctc_dkits_hash_ipfix_level,
        MaxTblId_t
    },
    {   /* UserId */
        53,
        5,
        ctc_dkits_hash_tbl_userid,
        ctc_dkits_hash_userid_level,
        DsUserIdHashCam_t
    }
};

int32
ctc_usw_dkits_hash_db_level_en(uint8 lchip, ctc_dkits_hash_db_module_t module, uint32* p_level_en)
{
    int32  ret = CLI_SUCCESS;
    ds_t   ds = {0};
    uint32 cmd = 0;

    if (CTC_DKITS_HASH_DB_MODULE_FIB_HOST0 == module)
    {
        cmd = DRV_IOR(FibHost0HashLookupCtl_t, DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &ds);

        GetFibHost0HashLookupCtl(A, fibHost0Level0HashEn_f, &ds, &p_level_en[0]);
        GetFibHost0HashLookupCtl(A, fibHost0Level1HashEn_f, &ds, &p_level_en[1]);
        GetFibHost0HashLookupCtl(A, fibHost0Level2HashEn_f, &ds, &p_level_en[2]);
        GetFibHost0HashLookupCtl(A, fibHost0Level3HashEn_f, &ds, &p_level_en[3]);
        GetFibHost0HashLookupCtl(A, fibHost0Level4HashEn_f, &ds, &p_level_en[4]);
        GetFibHost0HashLookupCtl(A, fibHost0Level5HashEn_f, &ds, &p_level_en[5]);
    }
    else if (CTC_DKITS_HASH_DB_MODULE_FIB_HOST1 == module)
    {
        cmd = DRV_IOR(FibHost1HashLookupCtl_t, DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &ds);

        GetFibHost1HashLookupCtl(A, fibHost1Level0HashEn_f, &ds, &p_level_en[0]);
        GetFibHost1HashLookupCtl(A, fibHost1Level1HashEn_f, &ds, &p_level_en[1]);
        GetFibHost1HashLookupCtl(A, fibHost1Level2HashEn_f, &ds, &p_level_en[2]);
        GetFibHost1HashLookupCtl(A, fibHost1Level3HashEn_f, &ds, &p_level_en[3]);
        GetFibHost1HashLookupCtl(A, fibHost1Level4HashEn_f, &ds, &p_level_en[4]);
    }
    else if (CTC_DKITS_HASH_DB_MODULE_EGRESS_XC == module)
    {
        cmd = DRV_IOR(EgressXcOamHashLookupCtl_t, DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &ds);

        GetEgressXcOamHashLookupCtl(A, egressXcOamLevel0HashEn_f, &ds, &p_level_en[0]);
        GetEgressXcOamHashLookupCtl(A, egressXcOamLevel1HashEn_f, &ds, &p_level_en[1]);
    }
    else if (CTC_DKITS_HASH_DB_MODULE_FLOW == module)
    {
        cmd = DRV_IOR(FlowHashLookupCtl_t, DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &ds);

        GetFlowHashLookupCtl(A, flowLevel0HashEn_f, &ds, &p_level_en[0]);
        GetFlowHashLookupCtl(A, flowLevel1HashEn_f, &ds, &p_level_en[1]);
        GetFlowHashLookupCtl(A, flowLevel2HashEn_f, &ds, &p_level_en[2]);
        GetFlowHashLookupCtl(A, flowLevel3HashEn_f, &ds, &p_level_en[3]);
        GetFlowHashLookupCtl(A, flowLevel4HashEn_f, &ds, &p_level_en[4]);
    }
    else if (CTC_DKITS_HASH_DB_MODULE_USERID == module)
    {
        cmd = DRV_IOR(UserIdHashLookupCtl_t, DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &ds);

        GetUserIdHashLookupCtl(A, userIdLevel0HashEn_f, &ds, &p_level_en[0]);
        GetUserIdHashLookupCtl(A, userIdLevel1HashEn_f, &ds, &p_level_en[1]);
        GetUserIdHashLookupCtl(A, userIdLevel2HashEn_f, &ds, &p_level_en[2]);
        GetUserIdHashLookupCtl(A, userIdLevel3HashEn_f, &ds, &p_level_en[3]);
        GetUserIdHashLookupCtl(A, userIdLevel4HashEn_f, &ds, &p_level_en[4]);
    }
    else if (CTC_DKITS_HASH_DB_MODULE_IPFIX == module)
    {
        cmd = DRV_IOR(IpfixHashLookupCtl_t, DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &ds);

        GetIpfixHashLookupCtl(A, ipfixLevel0HashEn_f, &ds, &p_level_en[0]);
        GetIpfixHashLookupCtl(A, ipfixLevel1HashEn_f, &ds, &p_level_en[1]);
        GetIpfixHashLookupCtl(A, ipfixLevel2HashEn_f, &ds, &p_level_en[2]);
        GetIpfixHashLookupCtl(A, ipfixLevel3HashEn_f, &ds, &p_level_en[3]);
    }

    return (ret < 0) ? CLI_ERROR : CLI_SUCCESS;
}

int32
ctc_usw_dkits_hash_db_get_couple_mode(uint8 lchip, ctc_dkits_hash_db_module_t module, uint32* p_couple_mode)
{
    int32  ret = CLI_SUCCESS;
    ds_t   ds = {0};
    int32  cmd = 0;

    switch (module)
    {
        case CTC_DKITS_HASH_DB_MODULE_EGRESS_XC:
            cmd = DRV_IOR(EgressXcOamHashLookupCtl_t, DRV_ENTRY_FLAG);
            ret = DRV_IOCTL(lchip, 0, cmd, &ds);

            GetEgressXcOamHashLookupCtl(A, egressXcOamLevel0Couple_f, &ds, &p_couple_mode[0]);
            GetEgressXcOamHashLookupCtl(A, egressXcOamLevel1Couple_f, &ds, &p_couple_mode[1]);
            break;

        case CTC_DKITS_HASH_DB_MODULE_FIB_HOST0:
            cmd = DRV_IOR(FibHost0HashLookupCtl_t, DRV_ENTRY_FLAG);
            ret = DRV_IOCTL(lchip, 0, cmd, &ds);

            GetFibHost0HashLookupCtl(A, fibHost0Level0Couple_f, &ds, &p_couple_mode[0]);
            GetFibHost0HashLookupCtl(A, fibHost0Level1Couple_f, &ds, &p_couple_mode[1]);
            GetFibHost0HashLookupCtl(A, fibHost0Level2Couple_f, &ds, &p_couple_mode[2]);
            GetFibHost0HashLookupCtl(A, fibHost0Level3Couple_f, &ds, &p_couple_mode[3]);
            GetFibHost0HashLookupCtl(A, fibHost0Level4Couple_f, &ds, &p_couple_mode[4]);
            GetFibHost0HashLookupCtl(A, fibHost0Level5Couple_f, &ds, &p_couple_mode[5]);
            break;

        case CTC_DKITS_HASH_DB_MODULE_FIB_HOST1:
            cmd = DRV_IOR(FibHost1HashLookupCtl_t, DRV_ENTRY_FLAG);
            ret = DRV_IOCTL(lchip, 0, cmd, &ds);

            GetFibHost1HashLookupCtl(A, fibHost1Level0Couple_f, &ds, &p_couple_mode[0]);
            GetFibHost1HashLookupCtl(A, fibHost1Level1Couple_f, &ds, &p_couple_mode[1]);
            GetFibHost1HashLookupCtl(A, fibHost1Level2Couple_f, &ds, &p_couple_mode[2]);
            GetFibHost1HashLookupCtl(A, fibHost1Level3Couple_f, &ds, &p_couple_mode[3]);
            GetFibHost1HashLookupCtl(A, fibHost1Level4Couple_f, &ds, &p_couple_mode[4]);
            break;

        case CTC_DKITS_HASH_DB_MODULE_FLOW:
            cmd = DRV_IOR(FlowHashLookupCtl_t, DRV_ENTRY_FLAG);
            ret = DRV_IOCTL(lchip, 0, cmd, &ds);

            GetFlowHashLookupCtl(A, flowLevel0Couple_f, &ds, &p_couple_mode[0]);
            GetFlowHashLookupCtl(A, flowLevel1Couple_f, &ds, &p_couple_mode[1]);
            GetFlowHashLookupCtl(A, flowLevel2Couple_f, &ds, &p_couple_mode[2]);
            GetFlowHashLookupCtl(A, flowLevel3Couple_f, &ds, &p_couple_mode[3]);
            GetFlowHashLookupCtl(A, flowLevel4Couple_f, &ds, &p_couple_mode[4]);
            break;

        case CTC_DKITS_HASH_DB_MODULE_IPFIX:
            cmd = DRV_IOR(IpfixHashLookupCtl_t, DRV_ENTRY_FLAG);
            ret = DRV_IOCTL(lchip, 0, cmd, &ds);

            GetIpfixHashLookupCtl(A, ipfixMemCouple_f, &ds, &p_couple_mode[0]);
            GetIpfixHashLookupCtl(A, ipfixMemCouple_f, &ds, &p_couple_mode[1]);
            GetIpfixHashLookupCtl(A, ipfixMemCouple_f, &ds, &p_couple_mode[2]);
            GetIpfixHashLookupCtl(A, ipfixMemCouple_f, &ds, &p_couple_mode[3]);
            break;

        case CTC_DKITS_HASH_DB_MODULE_USERID:
            cmd = DRV_IOR(UserIdHashLookupCtl_t, DRV_ENTRY_FLAG);
            ret = DRV_IOCTL(lchip, 0, cmd, &ds);

            GetUserIdHashLookupCtl(A, userIdLevel0Couple_f, &ds, &p_couple_mode[0]);
            GetUserIdHashLookupCtl(A, userIdLevel1Couple_f, &ds, &p_couple_mode[1]);
            GetUserIdHashLookupCtl(A, userIdLevel2Couple_f, &ds, &p_couple_mode[2]);
            GetUserIdHashLookupCtl(A, userIdLevel3Couple_f, &ds, &p_couple_mode[3]);
            GetUserIdHashLookupCtl(A, userIdLevel4Couple_f, &ds, &p_couple_mode[4]);
            break;

        default:
            break;
    }

    return (ret < 0) ? CLI_ERROR : CLI_SUCCESS;
}

int32
ctc_usw_dkits_hash_db_key_index_base(uint8 lchip, ctc_dkits_hash_db_module_t module, uint32 level, uint32* p_index_base)
{
    int32  ret = CLI_SUCCESS;
    ds_t   ds = {0};
    uint32 depth = 0, level_en[MAX_HASH_LEVEL] = {0};
    int    i = 0;
    int32  cmd = 0;
    uint32 hash_size_mode[MAX_HASH_LEVEL] = {0};

    switch (module)
    {
        case CTC_DKITS_HASH_DB_MODULE_EGRESS_XC:
            cmd = DRV_IOR(EgressXcOamHashLookupCtl_t, DRV_ENTRY_FLAG);
            ret = DRV_IOCTL(lchip, 0, cmd, &ds);

            GetEgressXcOamHashLookupCtl(A, egressXcOamLevel0Couple_f, &ds, &hash_size_mode[0]);
            GetEgressXcOamHashLookupCtl(A, egressXcOamLevel1Couple_f, &ds, &hash_size_mode[1]);
            break;

        case CTC_DKITS_HASH_DB_MODULE_FIB_HOST0:
            cmd = DRV_IOR(FibHost0HashLookupCtl_t, DRV_ENTRY_FLAG);
            ret = DRV_IOCTL(lchip, 0, cmd, &ds);

            GetFibHost0HashLookupCtl(A, fibHost0Level0Couple_f, &ds, &hash_size_mode[0]);
            GetFibHost0HashLookupCtl(A, fibHost0Level1Couple_f, &ds, &hash_size_mode[1]);
            GetFibHost0HashLookupCtl(A, fibHost0Level2Couple_f, &ds, &hash_size_mode[2]);
            GetFibHost0HashLookupCtl(A, fibHost0Level3Couple_f, &ds, &hash_size_mode[3]);
            GetFibHost0HashLookupCtl(A, fibHost0Level4Couple_f, &ds, &hash_size_mode[4]);
            GetFibHost0HashLookupCtl(A, fibHost0Level5Couple_f, &ds, &hash_size_mode[5]);
            break;

        case CTC_DKITS_HASH_DB_MODULE_FIB_HOST1:
            cmd = DRV_IOR(FibHost1HashLookupCtl_t, DRV_ENTRY_FLAG);
            ret = DRV_IOCTL(lchip, 0, cmd, &ds);

            GetFibHost1HashLookupCtl(A, fibHost1Level0Couple_f, &ds, &hash_size_mode[0]);
            GetFibHost1HashLookupCtl(A, fibHost1Level1Couple_f, &ds, &hash_size_mode[1]);
            GetFibHost1HashLookupCtl(A, fibHost1Level2Couple_f, &ds, &hash_size_mode[2]);
            GetFibHost1HashLookupCtl(A, fibHost1Level3Couple_f, &ds, &hash_size_mode[3]);
            GetFibHost1HashLookupCtl(A, fibHost1Level4Couple_f, &ds, &hash_size_mode[4]);
            break;

        case CTC_DKITS_HASH_DB_MODULE_FLOW:
            cmd = DRV_IOR(FlowHashLookupCtl_t, DRV_ENTRY_FLAG);
            ret = DRV_IOCTL(lchip, 0, cmd, &ds);

            GetFlowHashLookupCtl(A, flowLevel0Couple_f, &ds, &hash_size_mode[0]);
            GetFlowHashLookupCtl(A, flowLevel1Couple_f, &ds, &hash_size_mode[1]);
            GetFlowHashLookupCtl(A, flowLevel2Couple_f, &ds, &hash_size_mode[2]);
            GetFlowHashLookupCtl(A, flowLevel3Couple_f, &ds, &hash_size_mode[3]);
            GetFlowHashLookupCtl(A, flowLevel4Couple_f, &ds, &hash_size_mode[4]);
            break;

        case CTC_DKITS_HASH_DB_MODULE_IPFIX:
            cmd = DRV_IOR(IpfixHashLookupCtl_t, DRV_ENTRY_FLAG);
            ret = DRV_IOCTL(lchip, 0, cmd, &ds);

            GetIpfixHashLookupCtl(A, ipfixMemCouple_f, &ds, &hash_size_mode[0]);
            GetIpfixHashLookupCtl(A, ipfixMemCouple_f, &ds, &hash_size_mode[1]);
            GetIpfixHashLookupCtl(A, ipfixMemCouple_f, &ds, &hash_size_mode[2]);
            GetIpfixHashLookupCtl(A, ipfixMemCouple_f, &ds, &hash_size_mode[3]);
            break;

        case CTC_DKITS_HASH_DB_MODULE_USERID:
            cmd = DRV_IOR(UserIdHashLookupCtl_t, DRV_ENTRY_FLAG);
            ret = DRV_IOCTL(lchip, 0, cmd, &ds);

            GetUserIdHashLookupCtl(A, userIdLevel0Couple_f, &ds, &hash_size_mode[0]);
            GetUserIdHashLookupCtl(A, userIdLevel1Couple_f, &ds, &hash_size_mode[1]);
            GetUserIdHashLookupCtl(A, userIdLevel2Couple_f, &ds, &hash_size_mode[2]);
            GetUserIdHashLookupCtl(A, userIdLevel3Couple_f, &ds, &hash_size_mode[3]);
            GetUserIdHashLookupCtl(A, userIdLevel4Couple_f, &ds, &hash_size_mode[4]);
            break;

        default:
            break;
    }

    ret = ret ? ret : ctc_usw_dkits_hash_db_level_en(lchip, module, level_en);

    for (i = 0; i < level; i++)
    {
        if (level_en[i])
        {
            if ((module == CTC_DKITS_HASH_DB_MODULE_FLOW)
               || (module == CTC_DKITS_HASH_DB_MODULE_IPFIX)
               || (module == CTC_DKITS_HASH_DB_MODULE_EGRESS_XC))
            {
                depth = ctc_dkits_hash_db[module].p_level[i].depth * 2;
            }
            else
            {
                depth = ctc_dkits_hash_db[module].p_level[i].depth;
            }
            *p_index_base += depth * ctc_dkits_hash_db[module].p_level[i].bucket_num << hash_size_mode[i];
        }
    }

    return (ret < 0) ? CLI_ERROR : CLI_SUCCESS;
}

int32
ctc_usw_dkits_hash_db_entry_valid(uint8 lchip, ctc_dkits_hash_db_module_t module, uint8 type, uint32* p_data_entry, uint32* p_valid)
{
    uint8  word = 0, unit = 0, key_type = 0;

    if (module >= CTC_DKITS_HASH_DB_MODULE_NUM)
    {
        CTC_DKIT_PRINT_DEBUG("%s %d, Invalid hash module type %u!\n", __FUNCTION__, __LINE__, module);
        return CLI_ERROR;
    }

    for (key_type = 0; key_type < CTC_DKITS_HASH_DB_KEY_NUM(module); key_type++)
    {
        if (type == CTC_DKITS_HASH_DB_HASH_TYPE(module, key_type))
        {
            break;
        }
    }

    if (key_type == CTC_DKITS_HASH_DB_KEY_NUM(module))
    {
        if (0 == type)
        {
            *p_valid = 0;
            return CLI_SUCCESS;
        }
        else
        {
            CTC_DKIT_PRINT_DEBUG("%s %d, Hash module:%u's invalid hash type:%u!\n", __FUNCTION__, __LINE__, module, type);
            return CLI_ERROR;
        }
    }

    unit = CTC_DKITS_HASH_DB_KEY_ALIGN(module, key_type);
    word = CTC_DKITS_HASH_DB_BYTE2WORD(CTC_DKITS_HASH_DB_BIT2BYTE(CTC_DKITS_HASH_DB_KEY_UNIT(module, key_type)));

    if ((CTC_DKITS_HASH_DB_MODULE_FIB_HOST0 == module)
       || (CTC_DKITS_HASH_DB_MODULE_FIB_HOST1 == module)
       || (CTC_DKITS_HASH_DB_MODULE_USERID == module))
    {
        if (1 == unit)
        {
            *p_valid  = p_data_entry[0] & 0x1;
        }
        else if (2 == unit)
        {
            *p_valid  = p_data_entry[0] & 0x1;
            *p_valid &= p_data_entry[word*1] & 0x1;
        }
        else if (4 == unit)
        {
            *p_valid  = p_data_entry[0] & 0x1;
            *p_valid &= p_data_entry[word*1] & 0x1;
            *p_valid &= p_data_entry[word*2] & 0x1;
            *p_valid &= p_data_entry[word*3] & 0x1;
        }
    }
    else if (CTC_DKITS_HASH_DB_MODULE_EGRESS_XC == module)
    {
        if (2 == unit)
        {
            *p_valid = p_data_entry[0] & 0x1;
        }
        else
        {
            CTC_DKIT_PRINT_DEBUG("%s %d, EgressXcOam Hash align_units should only be 2, not %d!\n",
                                 __FUNCTION__, __LINE__, unit);
            return CLI_ERROR;
        }
    }
    else if (CTC_DKITS_HASH_DB_MODULE_FLOW == module)
    {
        if (2 == unit)
        {
            *p_valid = p_data_entry[0] & 0x7;
        }
        else if (4 == unit)
        {
            *p_valid  = p_data_entry[0] & 0x7;
            *p_valid &= p_data_entry[word*2] & 0x7;
        }
    }
    else if (CTC_DKITS_HASH_DB_MODULE_IPFIX == module)
    {
        if (2 == unit)
        {
            *p_valid = p_data_entry[0] & 0x7;
        }
        else if (4 == unit)
        {
            *p_valid  = p_data_entry[0] & 0x7;
            *p_valid &= p_data_entry[word*2] & 0x7;
        }
    }

    return CLI_SUCCESS;
}

