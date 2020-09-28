#include "drv_api.h"
#include "ctc_cli.h"
#include "ctc_dkit.h"
#include "ctc_usw_dkit_tcam.h"
#include "ctc_usw_dkit_dump_tbl.h"

tbls_id_t usw_auto_genernate_tbl[] =
{
    /* ... */
    MaxTblId_t
};

tbls_id_t usw_fib_hash_bridge_key_tbl[] =
{
    DsFibHost0MacHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_fib_hash_ipuc_ipv4_key_tbl[] =
{
    DsFibHost0Ipv4HashKey_t,
    DsFibHost1Ipv4NatDaPortHashKey_t,
    DsFibHost1Ipv4NatSaPortHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_fib_hash_ipuc_ipv6_key_tbl[] =
{
    DsFibHost0Ipv6UcastHashKey_t,
    DsFibHost1Ipv6NatDaPortHashKey_t,
    DsFibHost1Ipv6NatSaPortHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_fib_hash_ipmc_ipv4_key_tbl[] =
{
    DsFibHost1Ipv4McastHashKey_t,
    DsFibHost1MacIpv4McastHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_fib_hash_ipmc_ipv6_key_tbl[] =
{
    DsFibHost0Ipv6McastHashKey_t,
    DsFibHost0MacIpv6McastHashKey_t,
    DsFibHost1Ipv6McastHashKey_t,
    DsFibHost1MacIpv6McastHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_fib_hash_fcoe_key_tbl[] =
{
    DsFibHost0FcoeHashKey_t,
    DsFibHost1FcoeRpfHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_fib_hash_trill_key_tbl[] =
{
    DsFibHost0TrillHashKey_t,
    DsFibHost1TrillMcastVlanHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_scl_hash_bridge_key_tbl[] =
{
    DsUserIdCvlanCosPortHashKey_t,
    DsUserIdCvlanPortHashKey_t,
    DsUserIdDoubleVlanPortHashKey_t,
    DsUserIdMacHashKey_t,
    DsUserIdMacPortHashKey_t,
    DsUserIdPortHashKey_t,
    DsUserIdSvlanCosPortHashKey_t,
    DsUserIdSvlanHashKey_t,
    DsUserIdSvlanMacSaHashKey_t,
    DsUserIdSvlanPortHashKey_t,
    DsUserIdSclFlowL2HashKey_t,
    MaxTblId_t
};

tbls_id_t usw_scl_hash_ipv4_key_tbl[] =
{
    DsUserIdIpv4PortHashKey_t,
    DsUserIdIpv4SaHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_scl_hash_ipv6_key_tbl[] =
{
    DsUserIdIpv6PortHashKey_t,
    DsUserIdIpv6SaHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_scl_hash_iptunnel_key_tbl[] =
{
    DsUserIdTunnelIpv4DaHashKey_t,
    DsUserIdTunnelIpv4GreKeyHashKey_t,
    DsUserIdTunnelIpv4HashKey_t,
    DsUserIdTunnelIpv4RpfHashKey_t,
    DsUserIdTunnelIpv4UdpHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_scl_hash_nvgre_tunnel_key_tbl[] =
{
    DsUserIdTunnelIpv4NvgreMode1HashKey_t,
    DsUserIdTunnelIpv4McNvgreMode0HashKey_t,
    DsUserIdTunnelIpv4UcNvgreMode0HashKey_t,
    DsUserIdTunnelIpv4UcNvgreMode1HashKey_t,
    DsUserIdTunnelIpv6McNvgreMode0HashKey_t,
    DsUserIdTunnelIpv6McNvgreMode1HashKey_t,
    DsUserIdTunnelIpv6UcNvgreMode0HashKey_t,
    DsUserIdTunnelIpv6UcNvgreMode1HashKey_t,
    MaxTblId_t
};

tbls_id_t usw_scl_hash_vxlan_tunnel_key_tbl[] =
{
    DsUserIdTunnelIpv4McVxlanMode0HashKey_t,
    DsUserIdTunnelIpv4UcVxlanMode0HashKey_t,
    DsUserIdTunnelIpv4UcVxlanMode1HashKey_t,
    DsUserIdTunnelIpv4VxlanMode1HashKey_t,
    DsUserIdTunnelIpv6McVxlanMode0HashKey_t,
    DsUserIdTunnelIpv6McVxlanMode1HashKey_t,
    DsUserIdTunnelIpv6UcVxlanMode0HashKey_t,
    DsUserIdTunnelIpv6UcVxlanMode1HashKey_t,
    MaxTblId_t
};

tbls_id_t usw_scl_hash_wlan_key_tbl[] =
{
    DsUserIdTunnelCapwapRmacHashKey_t,
    DsUserIdTunnelCapwapRmacRidHashKey_t,
    DsUserIdTunnelIpv4CapwapHashKey_t,
    DsUserIdTunnelIpv6CapwapHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_scl_hash_mpls_tunnel_key_tbl[] =
{
    DsUserIdTunnelMplsHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_scl_hash_pbb_tunnel_key_tbl[] =
{
    DsUserIdTunnelPbbHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_scl_hash_trill_tunnel_key_tbl[] =
{
    DsUserIdTunnelTrillMcAdjHashKey_t,
    DsUserIdTunnelTrillMcDecapHashKey_t,
    DsUserIdTunnelTrillMcRpfHashKey_t,
    DsUserIdTunnelTrillUcDecapHashKey_t,
    DsUserIdTunnelTrillUcRpfHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_ipfix_hash_bridge_key_tbl[] =
{
    DsIpfixL2HashKey_t,
    MaxTblId_t
};

tbls_id_t usw_ipfix_hash_l2l3_key_tbl[] =
{
    DsIpfixL2L3HashKey_t,
    MaxTblId_t
};

tbls_id_t usw_ipfix_hash_ipv4_key_tbl[] =
{
    DsIpfixL3Ipv4HashKey_t,
    MaxTblId_t
};

tbls_id_t usw_ipfix_hash_ipv6_key_tbl[] =
{
    DsIpfixL3Ipv6HashKey_t,
    MaxTblId_t
};

tbls_id_t usw_ipfix_hash_mpls_key_tbl[] =
{
    DsIpfixL3MplsHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_flow_hash_bridge_key_tbl[] =
{
    DsFlowL2HashKey_t,
    MaxTblId_t
};

tbls_id_t usw_flow_hash_l2l3_key_tbl[] =
{
    DsFlowL2L3HashKey_t,
    MaxTblId_t
};

tbls_id_t usw_flow_hash_ipv4_key_tbl[] =
{
    DsFlowL3Ipv4HashKey_t,
    MaxTblId_t
};

tbls_id_t usw_flow_hash_ipv6_key_tbl[] =
{
    DsFlowL3Ipv6HashKey_t,
    MaxTblId_t
};

tbls_id_t usw_flow_hash_mpls_key_tbl[] =
{
    DsFlowL3MplsHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_xcoam_hash_scl_tbl[] =
{
    DsEgressXcOamPortCrossHashKey_t,
    DsEgressXcOamPortHashKey_t,
    DsEgressXcOamPortVlanCrossHashKey_t,
    DsEgressXcOamCvlanCosPortHashKey_t,
    DsEgressXcOamCvlanPortHashKey_t,
    DsEgressXcOamDoubleVlanPortHashKey_t,
    DsEgressXcOamSvlanCosPortHashKey_t,
    DsEgressXcOamSvlanPortHashKey_t,
    DsEgressXcOamSvlanPortMacHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_xcoam_hash_tunnel_tbl[] =
{
    DsEgressXcOamTunnelPbbHashKey_t,
    MaxTblId_t
};

tbls_id_t usw_xcoam_hash_oam_tbl[] =
{
    DsEgressXcOamBfdHashKey_t,
    DsEgressXcOamEthHashKey_t,
    DsEgressXcOamMplsLabelHashKey_t,
    DsEgressXcOamMplsSectionHashKey_t,
    DsEgressXcOamRmepHashKey_t,
    MaxTblId_t
};

#if 0
tbls_id_t usw_oam_tbl[] =
{
    /* APS */
    DsApsBridge_t,
    /* LM */
    DsOamLmStats_t,
    /* MEP */
    DsBfdMep_t,
    DsBfdRmep_t,
    DsEthMep_t,
    DsEthRmep_t,
    /* MA */
    DsMa_t,
    /* MA Name */
    DsMaName_t,

    MaxTblId_t
};
#endif

ctc_dkits_tcam_key_inf_t usw_acl_tcam_ipv4_key_inf[] =
{
    {DsAclQosL3Key160Ing0_t, DKITS_TCAM_ACL_KEY_L3160},
    {DsAclQosL3Key160Ing1_t, DKITS_TCAM_ACL_KEY_L3160},
    {DsAclQosL3Key160Ing2_t, DKITS_TCAM_ACL_KEY_L3160},
    {DsAclQosL3Key160Ing3_t, DKITS_TCAM_ACL_KEY_L3160},
    {DsAclQosL3Key160Ing4_t, DKITS_TCAM_ACL_KEY_L3160},
    {DsAclQosL3Key160Ing5_t, DKITS_TCAM_ACL_KEY_L3160},
    {DsAclQosL3Key160Ing6_t, DKITS_TCAM_ACL_KEY_L3160},
    {DsAclQosL3Key160Ing7_t, DKITS_TCAM_ACL_KEY_L3160},

    {DsAclQosL3Key320Ing0_t, DKITS_TCAM_ACL_KEY_L3320},
    {DsAclQosL3Key320Ing1_t, DKITS_TCAM_ACL_KEY_L3320},
    {DsAclQosL3Key320Ing2_t, DKITS_TCAM_ACL_KEY_L3320},
    {DsAclQosL3Key320Ing3_t, DKITS_TCAM_ACL_KEY_L3320},
    {DsAclQosL3Key320Ing4_t, DKITS_TCAM_ACL_KEY_L3320},
    {DsAclQosL3Key320Ing5_t, DKITS_TCAM_ACL_KEY_L3320},
    {DsAclQosL3Key320Ing6_t, DKITS_TCAM_ACL_KEY_L3320},
    {DsAclQosL3Key320Ing7_t, DKITS_TCAM_ACL_KEY_L3320},

    {DsAclQosL3Key160Egr0_t, DKITS_TCAM_ACL_KEY_L3160},
    {DsAclQosL3Key160Egr1_t, DKITS_TCAM_ACL_KEY_L3160},
    {DsAclQosL3Key160Egr2_t, DKITS_TCAM_ACL_KEY_L3160},

    {DsAclQosL3Key320Egr0_t, DKITS_TCAM_ACL_KEY_L3320},
    {DsAclQosL3Key320Egr1_t, DKITS_TCAM_ACL_KEY_L3320},
    {DsAclQosL3Key320Egr2_t, DKITS_TCAM_ACL_KEY_L3320},

    {MaxTblId_t,             DKITS_TCAM_ACL_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_acl_tcam_l2l3_key_inf[] =
{
    {DsAclQosMacL3Key320Ing0_t, DKITS_TCAM_ACL_KEY_MACL3320},
    {DsAclQosMacL3Key320Ing1_t, DKITS_TCAM_ACL_KEY_MACL3320},
    {DsAclQosMacL3Key320Ing2_t, DKITS_TCAM_ACL_KEY_MACL3320},
    {DsAclQosMacL3Key320Ing3_t, DKITS_TCAM_ACL_KEY_MACL3320},
    {DsAclQosMacL3Key320Ing4_t, DKITS_TCAM_ACL_KEY_MACL3320},
    {DsAclQosMacL3Key320Ing5_t, DKITS_TCAM_ACL_KEY_MACL3320},
    {DsAclQosMacL3Key320Ing6_t, DKITS_TCAM_ACL_KEY_MACL3320},
    {DsAclQosMacL3Key320Ing7_t, DKITS_TCAM_ACL_KEY_MACL3320},

    {DsAclQosMacL3Key640Ing0_t, DKITS_TCAM_ACL_KEY_MACL3640},
    {DsAclQosMacL3Key640Ing1_t, DKITS_TCAM_ACL_KEY_MACL3640},
    {DsAclQosMacL3Key640Ing2_t, DKITS_TCAM_ACL_KEY_MACL3640},
    {DsAclQosMacL3Key640Ing3_t, DKITS_TCAM_ACL_KEY_MACL3640},
    {DsAclQosMacL3Key640Ing4_t, DKITS_TCAM_ACL_KEY_MACL3640},
    {DsAclQosMacL3Key640Ing5_t, DKITS_TCAM_ACL_KEY_MACL3640},
    {DsAclQosMacL3Key640Ing6_t, DKITS_TCAM_ACL_KEY_MACL3640},
    {DsAclQosMacL3Key640Ing7_t, DKITS_TCAM_ACL_KEY_MACL3640},

    {DsAclQosMacIpv6Key640Ing0_t, DKITS_TCAM_ACL_KEY_MACIPV6640},
    {DsAclQosMacIpv6Key640Ing1_t, DKITS_TCAM_ACL_KEY_MACIPV6640},
    {DsAclQosMacIpv6Key640Ing2_t, DKITS_TCAM_ACL_KEY_MACIPV6640},
    {DsAclQosMacIpv6Key640Ing3_t, DKITS_TCAM_ACL_KEY_MACIPV6640},
    {DsAclQosMacIpv6Key640Ing4_t, DKITS_TCAM_ACL_KEY_MACIPV6640},
    {DsAclQosMacIpv6Key640Ing5_t, DKITS_TCAM_ACL_KEY_MACIPV6640},
    {DsAclQosMacIpv6Key640Ing6_t, DKITS_TCAM_ACL_KEY_MACIPV6640},
    {DsAclQosMacIpv6Key640Ing7_t, DKITS_TCAM_ACL_KEY_MACIPV6640},

    {DsAclQosMacL3Key320Egr0_t,   DKITS_TCAM_ACL_KEY_MACL3320},
    {DsAclQosMacL3Key320Egr1_t,   DKITS_TCAM_ACL_KEY_MACL3320},
    {DsAclQosMacL3Key320Egr2_t,   DKITS_TCAM_ACL_KEY_MACL3320},

    {DsAclQosMacL3Key640Egr0_t,   DKITS_TCAM_ACL_KEY_MACL3640},
    {DsAclQosMacL3Key640Egr1_t,   DKITS_TCAM_ACL_KEY_MACL3640},
    {DsAclQosMacL3Key640Egr2_t,   DKITS_TCAM_ACL_KEY_MACL3640},

    {DsAclQosMacIpv6Key640Egr0_t, DKITS_TCAM_ACL_KEY_MACIPV6640},
    {DsAclQosMacIpv6Key640Egr1_t, DKITS_TCAM_ACL_KEY_MACIPV6640},
    {DsAclQosMacIpv6Key640Egr2_t, DKITS_TCAM_ACL_KEY_MACIPV6640},

    {MaxTblId_t,                  DKITS_TCAM_ACL_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_acl_tcam_category_key_inf[] =
{
    {DsAclQosCidKey160Ing0_t,   DKITS_TCAM_ACL_KEY_CID160},
    {DsAclQosCidKey160Ing1_t,   DKITS_TCAM_ACL_KEY_CID160},
    {DsAclQosCidKey160Ing2_t,   DKITS_TCAM_ACL_KEY_CID160},
    {DsAclQosCidKey160Ing3_t,   DKITS_TCAM_ACL_KEY_CID160},
    {DsAclQosCidKey160Ing4_t,   DKITS_TCAM_ACL_KEY_CID160},
    {DsAclQosCidKey160Ing5_t,   DKITS_TCAM_ACL_KEY_CID160},
    {DsAclQosCidKey160Ing6_t,   DKITS_TCAM_ACL_KEY_CID160},
    {DsAclQosCidKey160Ing7_t,   DKITS_TCAM_ACL_KEY_CID160},
    {MaxTblId_t,                DKITS_TCAM_ACL_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_acl_tcam_interface_key_inf[] =
{
    {DsAclQosKey80Ing0_t, DKITS_TCAM_ACL_KEY_SHORT80},
    {DsAclQosKey80Ing1_t, DKITS_TCAM_ACL_KEY_SHORT80},
    {DsAclQosKey80Ing2_t, DKITS_TCAM_ACL_KEY_SHORT80},
    {DsAclQosKey80Ing3_t, DKITS_TCAM_ACL_KEY_SHORT80},
    {DsAclQosKey80Ing4_t, DKITS_TCAM_ACL_KEY_SHORT80},
    {DsAclQosKey80Ing5_t, DKITS_TCAM_ACL_KEY_SHORT80},
    {DsAclQosKey80Ing6_t, DKITS_TCAM_ACL_KEY_SHORT80},
    {DsAclQosKey80Ing7_t, DKITS_TCAM_ACL_KEY_SHORT80},

    {DsAclQosKey80Egr0_t, DKITS_TCAM_ACL_KEY_SHORT80},
    {DsAclQosKey80Egr1_t, DKITS_TCAM_ACL_KEY_SHORT80},
    {DsAclQosKey80Egr2_t, DKITS_TCAM_ACL_KEY_SHORT80},

    {MaxTblId_t,          DKITS_TCAM_ACL_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_acl_tcam_fwd_key_inf[] =
{
    {DsAclQosForwardKey320Ing0_t, DKITS_TCAM_ACL_KEY_FORWARD320},
    {DsAclQosForwardKey320Ing1_t, DKITS_TCAM_ACL_KEY_FORWARD320},
    {DsAclQosForwardKey320Ing2_t, DKITS_TCAM_ACL_KEY_FORWARD320},
    {DsAclQosForwardKey320Ing3_t, DKITS_TCAM_ACL_KEY_FORWARD320},
    {DsAclQosForwardKey320Ing4_t, DKITS_TCAM_ACL_KEY_FORWARD320},
    {DsAclQosForwardKey320Ing5_t, DKITS_TCAM_ACL_KEY_FORWARD320},
    {DsAclQosForwardKey320Ing6_t, DKITS_TCAM_ACL_KEY_FORWARD320},
    {DsAclQosForwardKey320Ing7_t, DKITS_TCAM_ACL_KEY_FORWARD320},

    {DsAclQosForwardKey640Ing0_t, DKITS_TCAM_ACL_KEY_FORWARD640},
    {DsAclQosForwardKey640Ing1_t, DKITS_TCAM_ACL_KEY_FORWARD640},
    {DsAclQosForwardKey640Ing2_t, DKITS_TCAM_ACL_KEY_FORWARD640},
    {DsAclQosForwardKey640Ing3_t, DKITS_TCAM_ACL_KEY_FORWARD640},
    {DsAclQosForwardKey640Ing4_t, DKITS_TCAM_ACL_KEY_FORWARD640},
    {DsAclQosForwardKey640Ing5_t, DKITS_TCAM_ACL_KEY_FORWARD640},
    {DsAclQosForwardKey640Ing6_t, DKITS_TCAM_ACL_KEY_FORWARD640},
    {DsAclQosForwardKey640Ing7_t, DKITS_TCAM_ACL_KEY_FORWARD640},

    {MaxTblId_t,                  DKITS_TCAM_ACL_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_acl_tcam_ipv6_key_inf[] =
{
    {DsAclQosIpv6Key320Ing0_t, DKITS_TCAM_ACL_KEY_IPV6320},
    {DsAclQosIpv6Key320Ing1_t, DKITS_TCAM_ACL_KEY_IPV6320},
    {DsAclQosIpv6Key320Ing2_t, DKITS_TCAM_ACL_KEY_IPV6320},
    {DsAclQosIpv6Key320Ing3_t, DKITS_TCAM_ACL_KEY_IPV6320},
    {DsAclQosIpv6Key320Ing4_t, DKITS_TCAM_ACL_KEY_IPV6320},
    {DsAclQosIpv6Key320Ing5_t, DKITS_TCAM_ACL_KEY_IPV6320},
    {DsAclQosIpv6Key320Ing6_t, DKITS_TCAM_ACL_KEY_IPV6320},
    {DsAclQosIpv6Key320Ing7_t, DKITS_TCAM_ACL_KEY_IPV6320},

    {DsAclQosIpv6Key640Ing0_t, DKITS_TCAM_ACL_KEY_IPV6640},
    {DsAclQosIpv6Key640Ing1_t, DKITS_TCAM_ACL_KEY_IPV6640},
    {DsAclQosIpv6Key640Ing2_t, DKITS_TCAM_ACL_KEY_IPV6640},
    {DsAclQosIpv6Key640Ing3_t, DKITS_TCAM_ACL_KEY_IPV6640},
    {DsAclQosIpv6Key640Ing4_t, DKITS_TCAM_ACL_KEY_IPV6640},
    {DsAclQosIpv6Key640Ing5_t, DKITS_TCAM_ACL_KEY_IPV6640},
    {DsAclQosIpv6Key640Ing6_t, DKITS_TCAM_ACL_KEY_IPV6640},
    {DsAclQosIpv6Key640Ing7_t, DKITS_TCAM_ACL_KEY_IPV6640},

    {DsAclQosIpv6Key320Egr0_t, DKITS_TCAM_ACL_KEY_IPV6320},
    {DsAclQosIpv6Key320Egr1_t, DKITS_TCAM_ACL_KEY_IPV6320},
    {DsAclQosIpv6Key320Egr2_t, DKITS_TCAM_ACL_KEY_IPV6320},

    {DsAclQosIpv6Key640Egr0_t, DKITS_TCAM_ACL_KEY_IPV6640},
    {DsAclQosIpv6Key640Egr1_t, DKITS_TCAM_ACL_KEY_IPV6640},
    {DsAclQosIpv6Key640Egr2_t, DKITS_TCAM_ACL_KEY_IPV6640},

    {MaxTblId_t,               DKITS_TCAM_ACL_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_acl_tcam_bridge_key_inf[] =
{
    {DsAclQosMacKey160Ing0_t,  DKITS_TCAM_ACL_KEY_MAC160},
    {DsAclQosMacKey160Ing1_t,  DKITS_TCAM_ACL_KEY_MAC160},
    {DsAclQosMacKey160Ing2_t,  DKITS_TCAM_ACL_KEY_MAC160},
    {DsAclQosMacKey160Ing3_t,  DKITS_TCAM_ACL_KEY_MAC160},
    {DsAclQosMacKey160Ing4_t,  DKITS_TCAM_ACL_KEY_MAC160},
    {DsAclQosMacKey160Ing5_t,  DKITS_TCAM_ACL_KEY_MAC160},
    {DsAclQosMacKey160Ing6_t,  DKITS_TCAM_ACL_KEY_MAC160},
    {DsAclQosMacKey160Ing7_t,  DKITS_TCAM_ACL_KEY_MAC160},

    {DsAclQosMacKey160Egr0_t,  DKITS_TCAM_ACL_KEY_MAC160},
    {DsAclQosMacKey160Egr1_t,  DKITS_TCAM_ACL_KEY_MAC160},
    {DsAclQosMacKey160Egr2_t,  DKITS_TCAM_ACL_KEY_MAC160},

    {MaxTblId_t,               DKITS_TCAM_ACL_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_scl_tcam_ipv4_key_inf[] =
{
    {DsScl0L3Key160_t,      DKITS_TCAM_SCL_KEY_L3_IPV4},
    {DsScl1L3Key160_t,      DKITS_TCAM_SCL_KEY_L3_IPV4},
    {MaxTblId_t,            DKITS_TCAM_SCL_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_scl_tcam_l2l3_key_inf[] =
{
    {DsScl0MacL3Key320_t,   DKITS_TCAM_SCL_KEY_L2L3_IPV4},
    {DsScl1MacL3Key320_t,   DKITS_TCAM_SCL_KEY_L2L3_IPV4},
    {DsScl0MacIpv6Key640_t, DKITS_TCAM_SCL_KEY_L2L3_IPV6},
    {DsScl1MacIpv6Key640_t, DKITS_TCAM_SCL_KEY_L2L3_IPV6},
    {MaxTblId_t,            DKITS_TCAM_SCL_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_scl_tcam_ipv6_key_inf[] =
{
    {DsScl0Ipv6Key320_t,    DKITS_TCAM_SCL_KEY_L3_IPV6},
    {DsScl1Ipv6Key320_t,    DKITS_TCAM_SCL_KEY_L3_IPV6},
    {MaxTblId_t,            DKITS_TCAM_SCL_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_scl_tcam_bridge_key_inf[] =
{
    {DsScl0MacKey160_t,     DKITS_TCAM_SCL_KEY_L2},
    {DsScl1MacKey160_t,     DKITS_TCAM_SCL_KEY_L2},
    {MaxTblId_t,            DKITS_TCAM_SCL_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_scl_tcam_userid_key_inf[] =
{
    {DsUserId0TcamKey80_t,  DKITS_TCAM_SCL_KEY_USERID_3W},
    {DsUserId1TcamKey80_t,  DKITS_TCAM_SCL_KEY_USERID_3W},
    {DsUserId0TcamKey160_t, DKITS_TCAM_SCL_KEY_USERID_6W},
    {DsUserId1TcamKey160_t, DKITS_TCAM_SCL_KEY_USERID_6W},
    {DsUserId0TcamKey320_t, DKITS_TCAM_SCL_KEY_USERID_12W},
    {DsUserId1TcamKey320_t, DKITS_TCAM_SCL_KEY_USERID_12W},
    {MaxTblId_t,            DKITS_TCAM_SCL_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_ip_tcam_prefix_key_inf[] =
{
    {DsLpmTcamIpv4HalfKeyLookup1_t,         DKITS_TCAM_IP_KEY_IPV4UC},
    {DsLpmTcamIpv6DoubleKey0Lookup1_t,      DKITS_TCAM_IP_KEY_IPV6UC},
    {DsLpmTcamIpv4SaHalfKeyLookup1_t,       DKITS_TCAM_IP_KEY_IPV4UC},
    {DsLpmTcamIpv6SaDoubleKey0Lookup1_t,    DKITS_TCAM_IP_KEY_IPV6UC},
    {DsLpmTcamIpv4DaPubHalfKeyLookup1_t,    DKITS_TCAM_IP_KEY_IPV4UC},
    {DsLpmTcamIpv6DaPubDoubleKey0Lookup1_t, DKITS_TCAM_IP_KEY_IPV6UC},
    {DsLpmTcamIpv4SaPubHalfKeyLookup1_t,    DKITS_TCAM_IP_KEY_IPV4UC},
    {DsLpmTcamIpv6SaPubDoubleKey0Lookup1_t, DKITS_TCAM_IP_KEY_IPV6UC},
    {DsLpmTcamIpv6SingleKey_t,              DKITS_TCAM_IP_KEY_IPV6UC},
    {DsLpmTcamIpv6SaSingleKey_t,            DKITS_TCAM_IP_KEY_IPV6UC},
    {DsLpmTcamIpv6DaPubSingleKey_t,         DKITS_TCAM_IP_KEY_IPV6UC},
    {DsLpmTcamIpv6SaPubSingleKey_t,         DKITS_TCAM_IP_KEY_IPV6UC},

    {DsLpmTcamIpv4HalfKeyLookup2_t,         DKITS_TCAM_IP_KEY_IPV4UC},
    {DsLpmTcamIpv6DoubleKey0Lookup2_t,      DKITS_TCAM_IP_KEY_IPV6UC},
    {DsLpmTcamIpv4SaHalfKeyLookup2_t,       DKITS_TCAM_IP_KEY_IPV4UC},
    {DsLpmTcamIpv6SaDoubleKey0Lookup2_t,    DKITS_TCAM_IP_KEY_IPV6UC},
    {DsLpmTcamIpv4DaPubHalfKeyLookup2_t,    DKITS_TCAM_IP_KEY_IPV4UC},
    {DsLpmTcamIpv6DaPubDoubleKey0Lookup2_t, DKITS_TCAM_IP_KEY_IPV6UC},
    {DsLpmTcamIpv4SaPubHalfKeyLookup2_t,    DKITS_TCAM_IP_KEY_IPV4UC},
    {DsLpmTcamIpv6SaPubDoubleKey0Lookup2_t, DKITS_TCAM_IP_KEY_IPV6UC},
    {MaxTblId_t,                            DKITS_TCAM_IP_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_ip_tcam_pbr_ipv4_key_inf[] =
{
    {DsLpmTcamIpv4PbrDoubleKey_t, DKITS_TCAM_NATPBR_KEY_IPV4PBR},
    {MaxTblId_t,                  DKITS_TCAM_NATPBR_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_ip_tcam_pbr_ipv6_key_inf[] =
{
    {DsLpmTcamIpv6QuadKey_t,      DKITS_TCAM_NATPBR_KEY_IPV6PBR},
    {MaxTblId_t,                  DKITS_TCAM_NATPBR_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_ip_tcam_nat_ipv4_key_inf[] =
{
    {DsLpmTcamIpv4NatDoubleKey_t, DKITS_TCAM_NATPBR_KEY_IPV4NAT},
    {MaxTblId_t,                  DKITS_TCAM_NATPBR_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_ip_tcam_nat_ipv6_key_inf[] =
{
    {DsLpmTcamIpv6DoubleKey1_t,   DKITS_TCAM_NATPBR_KEY_IPV6NAT},
    {MaxTblId_t,                  DKITS_TCAM_NATPBR_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_static_cid_tcam_key_inf[] =
{
    {DsCategoryIdPairTcamKey_t,   0},
    {MaxTblId_t,                  DKITS_TCAM_NATPBR_KEY_NUM}
};

ctc_dkits_tcam_key_inf_t usw_static_qmgr_tcam_key_inf[] =
{
    {DsQueueMapTcamKey_t,         0},
    {MaxTblId_t,                  DKITS_TCAM_NATPBR_KEY_NUM}
};

tbls_id_t usw_nh_fwd_tbl[] =
{
    DsFwd_t,
    DsFwdDualHalf_t,
    DsFwdHalf_t,
    MaxTblId_t
};

tbls_id_t usw_nh_met_tbl[] =
{
    DsMetEntry3W_t,
    DsMetEntry6W_t,
    MaxTblId_t
};

tbls_id_t usw_nh_nexthop_tbl[] =
{
    DsNextHop4W_t,
    DsNextHop8W_t,
    MaxTblId_t
};

tbls_id_t usw_nh_edit_tbl[] =
{
    DsL23Edit3W_t,
    DsL23Edit12W_t,
    DsL23Edit6W_t,
    DsL2EditEth3W_t,
    DsL2EditEth6W_t,
    DsL2EditInnerSwap_t,
    DsL2EditFlex_t,
    DsL2EditLoopback_t,
    DsL2EditPbb4W_t,
    DsL2EditPbb8W_t,
    DsL2EditSwap_t,
    DsL2EditOf_t,
    DsL3EditFlex_t,
    DsL3EditLoopback_t,
    DsL3EditMpls3W_t,
    DsL3EditNat3W_t,
    DsL3EditNat6W_t,
    DsL3EditOf6W_t,
    DsL3EditOf12W_t,
    DsL3EditOf12WIpv6Only_t,
    DsL3EditOf12WArpData_t,
    DsL3EditOf12WIpv6L4_t,
    DsL3EditTrill_t,
    DsL3EditTunnelV4_t,
    DsL3EditTunnelV6_t,
    DsL3Edit12W1st_t,
    DsL3Edit12W2nd_t,
    DsL2Edit12WInner_t,
    DsL2Edit12WShare_t,
    DsL2Edit6WShare_t,
    DsL2EditDsLite_t,
    TempInnerEdit12W_t,
    TempOuterEdit12W_t,
    MaxTblId_t
};

#if 0
tbls_id_t usw_lpm_lookup_key_tbl[] =
{
    MaxTblId_t
};

tbls_id_t usw_alias_tbl[] =
{
    MaxTblId_t
};

tbls_id_t usw_stats_tbl[] =
{
    MaxTblId_t
};
#endif
tbls_id_t dt2_serdes_tbl[] =
{
    HsCfg0_t,
    HsCfg1_t,
    HsCfg2_t,
    HsClkTreeCfg0_t,
    HsClkTreeCfg1_t,
    HsClkTreeCfg2_t,
    HsMacroCfg0_t,
    HsMacroCfg1_t,
    HsMacroCfg2_t,
    CsCfg0_t,
    CsCfg1_t,
    CsCfg2_t,
    CsCfg3_t,
    CsClkTreeCfg0_t,
    CsClkTreeCfg1_t,
    CsClkTreeCfg2_t,
    CsClkTreeCfg3_t,
    CsMacroCfg0_t,
    CsMacroCfg1_t,
    CsMacroCfg2_t,
    CsMacroCfg3_t,
    RlmCsCtlReset_t,
    RlmHsCtlReset_t,
    MaxTblId_t
};
tbls_id_t tm_serdes_tbl[] =
{
    Hss12GGlbCfg0_t,
    Hss12GGlbCfg1_t,
    Hss12GGlbCfg2_t,
    Hss12GClkTreeCfg0_t,
    Hss12GClkTreeCfg1_t,
    Hss12GClkTreeCfg2_t,
    Hss12GMacroCfg0_t,
    Hss12GMacroCfg1_t,
    Hss12GMacroCfg2_t,
    Hss28GCfg_t,
    Hss28GClkTreeCfg_t,
    Hss28GMacroCfg_t,
    RlmCsCtlReset_t,
    RlmHsCtlReset_t,
    Hss12GCmuCfg0_t,
    Hss12GCmuCfg1_t,
    Hss12GCmuCfg2_t,
    Hss12GLaneMiscCfg0_t,
    Hss12GLaneMiscCfg1_t,
    Hss12GLaneMiscCfg2_t,
    MaxTblId_t
};


tbls_id_t usw_exclude_tbl[] =
{
    AnethDebugStatus0_t,
    AnethDebugStatus1_t,
    AnethDebugStatus2_t,
    AnethDebugStatus3_t,
    AnethDebugStatus4_t,
    AnethDebugStatus5_t,
    AnethDebugStatus6_t,
    AnethDebugStatus7_t,
    AnethDebugStatus8_t,
    AnethDebugStatus9_t,
    AnethDebugStatus10_t,
    AnethDebugStatus11_t,
    AnethDebugStatus12_t,
    AnethDebugStatus13_t,
    AnethDebugStatus14_t,
    AnethDebugStatus15_t,
    AnethDebugStatus16_t,
    AnethDebugStatus17_t,
    AnethDebugStatus18_t,
    AnethDebugStatus19_t,
    AnethDebugStatus20_t,
    AnethDebugStatus21_t,
    AnethDebugStatus22_t,
    AnethDebugStatus23_t,
    AnethDebugStatus24_t,
    AnethDebugStatus25_t,
    AnethDebugStatus26_t,
    AnethDebugStatus27_t,
    AnethDebugStatus28_t,
    AnethDebugStatus29_t,
    AnethDebugStatus30_t,
    AnethDebugStatus31_t,
    AnethDebugStatus32_t,
    AnethDebugStatus33_t,
    AnethDebugStatus34_t,
    AnethDebugStatus35_t,
    AnethDebugStatus36_t,
    AnethDebugStatus37_t,
    AnethDebugStatus38_t,
    AnethDebugStatus39_t,
    BufRetrvInputDebugStats_t,
    BufRetrvIntfMemAddrDebug_t,
    BufRetrvMiscDebugStats_t,
    BufRetrvOutputPktDebugStats_t,
    BufRetrvPktMiscWeightDebug_t,
    BufRetrvReassembleDebug_t,
    BufRetrvWrrDebugCtl_t,
    BufRetrvWrrDebugStatus_t,
    DbgFwdBufRetrvHeaderInfo_t,
    DbgFwdBufRetrvInfo_t,
    BufStoreDebugCam_t,
    BufStoreDebugCtl_t,
    BufStoreDebugFifoDepth_t,
    BufStoreInputDebugStats_t,
    BufStoreIpeInputDebugStats_t,
    BufStoreMiscBufStats_t,
    BufStoreOutputDebugStats_t,
    BufStoreStatsCtl_t,
    DbgFwdBufStoreInfo_t,
    DbgFwdBufStoreInfo2_t,
    DbgIrmResrcAllocInfo_t,
    PktErrStatsMem_t,
    CapwapCipherDebugStats_t,
    DbgWlanDecryptEngineInfo_t,
    DbgWlanEncryptAndFragmentEngineInfo_t,
    CapwapCipherInputStats_t,
    CapwapProcDebugStats_t,
    DbgWlanReassembleEngineInfo_t,
    CoppDebugStats_t,
    DbgDlbEngineInfo_t,
    DlbDebugStats_t,
    DlbLoadDebug_t,
    DmaCtlDebugStats_t,
    DmaDescStats_t,
    DmaFifoDepthStats_t,
    DmaInfoAsyncStats_t,
    DmaIntfDebugStats_t,
    DmaPktRxErrorStats_t,
    DmaPktRxStats_t,
    DmaPktStatsCfg_t,
    DmaPktTxStats_t,
    DmaRegDebugStats_t,
    DmaTsDebugStats_t,
    DsAgingDebugInfo_t,
    DsAgingDebugStats_t,
    DynamicAdDebugStats_t,
    DynamicEditDebugStats_t,
    DbgOamApsSwitch_t,
    DynamicKeyDebugStats_t,
    DsOamLmStatsRtl_t,
    DbgEfdEngineInfo_t,
    EfdDebugStats_t,
    DbgEgrXcOamHash0EngineFromEpeNhpInfo_t,
    DbgEgrXcOamHash1EngineFromEpeNhpInfo_t,
    DbgEgrXcOamHashEngineFromEpeOam0Info_t,
    DbgEgrXcOamHashEngineFromEpeOam1Info_t,
    DbgEgrXcOamHashEngineFromEpeOam2Info_t,
    DbgEgrXcOamHashEngineFromIpeOam0Info_t,
    DbgEgrXcOamHashEngineFromIpeOam1Info_t,
    DbgEgrXcOamHashEngineFromIpeOam2Info_t,
    EgrOamHashDebugStats_t,
    DbgEpeAclInfo_t,
    DbgEpeClassificationInfo_t,
    DbgEpeOamInfo_t,
    EpeAclQosDebugStats_t,
    EpeClaDebugStats_t,
    EpeOamDebugStats_t,
    DbgEpeHdrAdjInfo_t,
    EpeHdrAdjDebugStats_t,
    EpeHdrAdjustDebugCam_t,
    DbgEpeHdrEditCflexHdrInfo_t,
    DbgEpeHdrEditInfo_t,
    EpeHdrEditDebugInfo_t,
    EpeHdrEditDebugStats_t,
    EpeHdrEditStatsCtl_t,
    DsEgressDiscardStats_t,
    EpeHdrEditChanFlowStatsRa_t,
    DbgEpeEgressEditInfo_t,
    DbgEpeInnerL2EditInfo_t,
    DbgEpeL3EditInfo_t,
    DbgEpeOuterL2EditInfo_t,
    DbgEpePayLoadInfo_t,
    DbgParserFromEpeHdrAdjInfo_t,
    EpeHdrProcDebugStats_t,
    DbgEpeNextHopInfo_t,
    EpeNextHopDebugStatus_t,
    EpeScheduleDebugInfo_t,
    EpeScheduleDebugStats_t,
    FibAccDebugInfo_t,
    FibAccDebugStats_t,
    DbgFibLkpEngineFlowHashInfo_t,
    DbgFibLkpEngineHostUrpfHashInfo_t,
    DbgFibLkpEngineMacDaHashInfo_t,
    DbgFibLkpEngineMacSaHashInfo_t,
    DbgFibLkpEnginel3DaHashInfo_t,
    DbgFibLkpEnginel3SaHashInfo_t,
    DbgLpmPipelineLookupResultInfo_t,
    FibEngineDebugStats_t,
    FibHashDebugStats_t,
    DbgIpfixAccEgrInfo_t,
    DbgIpfixAccIngInfo_t,
    FlowAccDebugInfo_t,
    FlowAccDebugStats_t,
    FlowHashDebugStats_t,
    DbgFlowTcamEngineEpeAclInfo0_t,
    DbgFlowTcamEngineEpeAclInfo1_t,
    DbgFlowTcamEngineEpeAclInfo2_t,
    DbgFlowTcamEngineEpeAclKeyInfo0_t,
    DbgFlowTcamEngineEpeAclKeyInfo1_t,
    DbgFlowTcamEngineEpeAclKeyInfo2_t,
    DbgFlowTcamEngineIpeAclInfo0_t,
    DbgFlowTcamEngineIpeAclInfo1_t,
    DbgFlowTcamEngineIpeAclInfo2_t,
    DbgFlowTcamEngineIpeAclInfo3_t,
    DbgFlowTcamEngineIpeAclInfo4_t,
    DbgFlowTcamEngineIpeAclInfo5_t,
    DbgFlowTcamEngineIpeAclInfo6_t,
    DbgFlowTcamEngineIpeAclInfo7_t,
    DbgFlowTcamEngineIpeAclKeyInfo0_t,
    DbgFlowTcamEngineIpeAclKeyInfo1_t,
    DbgFlowTcamEngineIpeAclKeyInfo2_t,
    DbgFlowTcamEngineIpeAclKeyInfo3_t,
    DbgFlowTcamEngineIpeAclKeyInfo4_t,
    DbgFlowTcamEngineIpeAclKeyInfo5_t,
    DbgFlowTcamEngineIpeAclKeyInfo6_t,
    DbgFlowTcamEngineIpeAclKeyInfo7_t,
    DbgFlowTcamEngineUserIdInfo0_t,
    DbgFlowTcamEngineUserIdInfo1_t,
    DbgFlowTcamEngineUserIdInfo2_t,
    DbgFlowTcamEngineUserIdKeyInfo0_t,
    DbgFlowTcamEngineUserIdKeyInfo1_t,
    DbgFlowTcamEngineUserIdKeyInfo2_t,
    FlowTcamDbgInfoCtl_t,
    FlowTcamDebugStats_t,
    DsStatsEdramRefreshCtl_t,
    GlobalStatsCacheRamOverflowVector_t,
    GlobalStatsCreditCtl_t,
    GlobalStatsCreditStatus_t,
    GlobalStatsCtl_t,
    GlobalStatsDebugStats_t,
    GlobalStatsEEECalendar_t,
    GlobalStatsEEECtl_t,
    GlobalStatsEdramInit_t,
    GlobalStatsEdramInitDone_t,
    GlobalStatsInit_t,
    GlobalStatsInitDone_t,
    GlobalStatsParityCtl_t,
    GlobalStatsParityStatus_t,
    GlobalStatsRamChkRec_t,
    GlobalStatsReqDropCnt_t,
    GlobalStatsReserved_t,
    GlobalStatsRxLpiState_t,
    GlobalStatsSatuAddrFifoDepth_t,
    GlobalStatsSatuInterruptThreshold_t,
    GlobalStatsTimeoutCtl_t,
    GlobalStatsUpdateEnCtl_t,
    StatsCacheBasePtr_t,
    StatsUpdateThrdCtl_t,
    DsStats_t,
    DsStatsEgressACL0_t,
    DsStatsEgressGlobal0_t,
    DsStatsEgressGlobal1_t,
    DsStatsEgressGlobal2_t,
    DsStatsEgressGlobal3_t,
    DsStatsIngressACL0_t,
    DsStatsIngressACL1_t,
    DsStatsIngressACL2_t,
    DsStatsIngressACL3_t,
    DsStatsIngressGlobal0_t,
    DsStatsIngressGlobal1_t,
    DsStatsIngressGlobal2_t,
    DsStatsIngressGlobal3_t,
    DsStatsQueue_t,
    EEEStatsRam_t,
    GlobalStatsDsStatsSatuAddr_t,
    GlobalStatsInterruptFatal_t,
    GlobalStatsInterruptNormal_t,
    RxInbdStatsRam_t,
    StatsCacheFifo_t,
    StatsScanResFifo_t,
    TxInbdStatsEpeRam_t,
    TxInbdStatsPauseRam_t,
    I2CMasterDebugState0_t,
    I2CMasterDebugState1_t,
    DbgIpeAclProcInfo_t,
    DbgIpeEcmpProcInfo_t,
    DbgIpeFwdCoppInfo_t,
    DbgIpeFwdInputHeaderInfo_t,
    DbgIpeFwdPolicingInfo_t,
    DbgIpeFwdProcessInfo_t,
    DbgIpeFwdStormCtlInfo_t,
    IpeFwdDebugStats_t,
    IpeFwdDiscardStatsCtl_t,
    IpeFwdFlowTcamDebugStats_t,
    IpeFwdIpfixStats_t,
    IpeFwdStatsCtl_t,
    DsIngressDiscardStats_t,
    IpeFwdEcmpStatsFifo_t,
    IpeFwdStatsSopMem_t,
    IpeHdrAdjDebugStats_t,
    DbgIpeIntfMapperInfo_t,
    DbgIpeMplsDecapInfo_t,
    DbgIpeUserIdInfo_t,
    DbgParserFromIpeHdrAdjInfo_t,
    DebugSessionStatusCtl_t,
    IpeIntfMapDebugStats_t,
    DbgIpeLkpMgrInfo_t,
    DbgParserFromIpeIntfInfo_t,
    IpeLkupMgrDebugStats_t,
    DbgIpeFcoeInfo_t,
    DbgIpeFlowProcessInfo_t,
    DbgIpeIpRoutingInfo_t,
    DbgIpeMacBridgingInfo_t,
    DbgIpeMacLearningInfo_t,
    DbgIpeOamInfo_t,
    DbgIpePacketProcessInfo_t,
    DbgIpePerHopBehaviorInfo_t,
    DbgIpeTrillInfo_t,
    IpePktProcDebugStats_t,
    DsOamLmStatsFifo_t,
    IpfixHashDebugStats_t,
    DbgLagEngineInfoFromBsrEnqueue_t,
    DbgLagEngineInfoFromIpeFwd_t,
    LinkAggDebugStats_t,
    DbgLpmTcamEngineResult0Info_t,
    DbgLpmTcamEngineResult1Info_t,
    LpmTcamDebugStats_t,
    DbgMACSecDecryptEngineInfo_t,
    DbgMACSecEncryptEngineInfo_t,
    MacSecEngineDebugStats_t,
    McuDbgAddrReg0_t,
    McuDbgAddrReg1_t,
    McuDbgCmdReg0_t,
    McuDbgCmdReg1_t,
    McuDbgData0Reg0_t,
    McuDbgData0Reg1_t,
    McuDbgData10Reg0_t,
    McuDbgData10Reg1_t,
    McuDbgData11Reg0_t,
    McuDbgData11Reg1_t,
    McuDbgData12Reg0_t,
    McuDbgData12Reg1_t,
    McuDbgData13Reg0_t,
    McuDbgData13Reg1_t,
    McuDbgData14Reg0_t,
    McuDbgData14Reg1_t,
    McuDbgData15Reg0_t,
    McuDbgData15Reg1_t,
    McuDbgData1Reg0_t,
    McuDbgData1Reg1_t,
    McuDbgData2Reg0_t,
    McuDbgData2Reg1_t,
    McuDbgData3Reg0_t,
    McuDbgData3Reg1_t,
    McuDbgData4Reg0_t,
    McuDbgData4Reg1_t,
    McuDbgData5Reg0_t,
    McuDbgData5Reg1_t,
    McuDbgData6Reg0_t,
    McuDbgData6Reg1_t,
    McuDbgData7Reg0_t,
    McuDbgData7Reg1_t,
    McuDbgData8Reg0_t,
    McuDbgData8Reg1_t,
    McuDbgData9Reg0_t,
    McuDbgData9Reg1_t,
    McuDbgMiscCtlReg0_t,
    McuDbgMiscCtlReg1_t,
    McuSupStats0_t,
    McuSupStats1_t,
    DbgFwdMetFifoInfo1_t,
    DbgFwdMetFifoInfo2_t,
    MetFifoDebugCam_t,
    MetFifoDebugCtl_t,
    MetFifoDebugStats_t,
    MetFifoDepthStats_t,
    MetFifoLinkUpdateStats_t,
    NetRxDebugStats_t,
    NetRxDebugStatsTable_t,
    NetTxCalDebugInfo_t,
    NetTxDebugStats_t,
    OamAutoGenPktDebugStats_t,
    AutoGenPktRxPktStats_t,
    AutoGenPktTxPktStats_t,
    DbgOamHdrEdit_t,
    OamFwdDebugStats_t,
    DbgOamHdrAdj_t,
    DbgOamParser_t,
    OamParserDebugStats_t,
    DbgOamDefectProc_t,
    DbgOamRxProc_t,
    DbgOamTxProc_t,
    OamProcDebugStats_t,
    OamProcRxDsDebug_t,
    OamProcTxDsDebug_t,
    OamTxProcDebugStats_t,
    OobFcDebugStats_t,
    OobFcErrorStats_t,
    ParserDebugStats1_t,
    ParserDebugStats2_t,
    ParserDebugStats_t,
    PbCtlDebugStats_t,
    PcieDebugPtr_t,
    Pcie0DebugMem_t,
    PolicingEpeDebugStats_t,
    PolicingIpeDebugStats_t,
    QMgrDeqDebugStats_t,
    QMgrDeqWrrDebugInfo_t,
    DbgErmResrcAllocInfo_t,
    DbgFwdQWriteInfo_t,
    QMgrEnqDebugInfo_t,
    QMgrEnqDebugStats_t,
    QMgrMsgStoreDebugStats_t,
    QsgmiiPcsK281PositionDbg0_t,
    QsgmiiPcsK281PositionDbg1_t,
    QsgmiiPcsK281PositionDbg2_t,
    QsgmiiPcsK281PositionDbg3_t,
    QsgmiiPcsK281PositionDbg4_t,
    QsgmiiPcsK281PositionDbg5_t,
    QsgmiiPcsK281PositionDbg6_t,
    QsgmiiPcsK281PositionDbg7_t,
    QsgmiiPcsK281PositionDbg8_t,
    QsgmiiPcsK281PositionDbg9_t,
    QsgmiiPcsK281PositionDbg10_t,
    QsgmiiPcsK281PositionDbg11_t,
    QuadSgmacDebugStatus0_t,
    QuadSgmacDebugStatus1_t,
    QuadSgmacDebugStatus2_t,
    QuadSgmacDebugStatus3_t,
    QuadSgmacDebugStatus4_t,
    QuadSgmacDebugStatus5_t,
    QuadSgmacDebugStatus6_t,
    QuadSgmacDebugStatus7_t,
    QuadSgmacDebugStatus8_t,
    QuadSgmacDebugStatus9_t,
    QuadSgmacDebugStatus10_t,
    QuadSgmacDebugStatus11_t,
    QuadSgmacDebugStatus12_t,
    QuadSgmacDebugStatus13_t,
    QuadSgmacDebugStatus14_t,
    QuadSgmacDebugStatus15_t,
    QuadSgmacStatsCfg0_t,
    QuadSgmacStatsCfg1_t,
    QuadSgmacStatsCfg2_t,
    QuadSgmacStatsCfg3_t,
    QuadSgmacStatsCfg4_t,
    QuadSgmacStatsCfg5_t,
    QuadSgmacStatsCfg6_t,
    QuadSgmacStatsCfg7_t,
    QuadSgmacStatsCfg8_t,
    QuadSgmacStatsCfg9_t,
    QuadSgmacStatsCfg10_t,
    QuadSgmacStatsCfg11_t,
    QuadSgmacStatsCfg12_t,
    QuadSgmacStatsCfg13_t,
    QuadSgmacStatsCfg14_t,
    QuadSgmacStatsCfg15_t,
    QuadSgmacStatsOverWrite0_t,
    QuadSgmacStatsOverWrite1_t,
    QuadSgmacStatsOverWrite2_t,
    QuadSgmacStatsOverWrite3_t,
    QuadSgmacStatsOverWrite4_t,
    QuadSgmacStatsOverWrite5_t,
    QuadSgmacStatsOverWrite6_t,
    QuadSgmacStatsOverWrite7_t,
    QuadSgmacStatsOverWrite8_t,
    QuadSgmacStatsOverWrite9_t,
    QuadSgmacStatsOverWrite10_t,
    QuadSgmacStatsOverWrite11_t,
    QuadSgmacStatsOverWrite12_t,
    QuadSgmacStatsOverWrite13_t,
    QuadSgmacStatsOverWrite14_t,
    QuadSgmacStatsOverWrite15_t,
    QuadSgmacStatsRam0_t,
    QuadSgmacStatsRam1_t,
    QuadSgmacStatsRam2_t,
    QuadSgmacStatsRam3_t,
    QuadSgmacStatsRam4_t,
    QuadSgmacStatsRam5_t,
    QuadSgmacStatsRam6_t,
    QuadSgmacStatsRam7_t,
    QuadSgmacStatsRam8_t,
    QuadSgmacStatsRam9_t,
    QuadSgmacStatsRam10_t,
    QuadSgmacStatsRam11_t,
    QuadSgmacStatsRam12_t,
    QuadSgmacStatsRam13_t,
    QuadSgmacStatsRam14_t,
    QuadSgmacStatsRam15_t,
    StormCtlDebugStats_t,
    DebugMode_t,
    ResetDebugClock_t,
    TsEngineDebugStats_t,
    DbgUserIdHashEngineForMpls0Info_t,
    DbgUserIdHashEngineForMpls1Info_t,
    DbgUserIdHashEngineForMpls2Info_t,
    DbgUserIdHashEngineForUserId0Info_t,
    DbgUserIdHashEngineForUserId1Info_t,
    UserIdHashDebugStats_t,
    DsL2EditOfTemp_t,
    DsL3EditTunnelV4Temp_t,
    DsL3EditTunnelV6Temp_t,
    TempInnerEdit12W_t,
    TempOuterEdit12W_t,
    DsOamLmStats_t,
    AclEgressStatsPI_t,
    AclIngressStatsPI_t,
    CFHeaderExtTemp2_t,
    CFHeaderExtTemp3_t,
    CFHeaderExtTemp4_t,
    DsLinkAggregateChannelMemberTemp_t,
    DsLinkAggregateMemberTemp_t,
    DsStatsCache_t,
    EpeHdrAdjustDebugCamPR_t,
    FwdEgressStatsPI_t,
    FwdIngressStatsPI_t,
    MsQWriteToStats_t,
    PRTemp_t,
    ParserInfoTemp_t,
    ParserResultTemp_t,
    TempAclKeyOtherInfo_t,
    TempDlbEcmpLinkState_t,
    TempDmaQueUpdateInfo_t,
    TempDot1AeSecTag_t,
    TempDsDestMap1_t,
    TempDsFibHost0340Key_t,
    TempDsFlow12wHashKey_t,
    TempDsFlow6wHashKey_t,
    TempDsOamChan0_t,
    TempDsOamChan1_t,
    TempDsOamChan2_t,
    TempDsSclAclControlProfile_t,
    TempDsSclAclControlProfile1_t,
    TempDsSclAclControlProfile2_t,
    TempDsSclAclControlProfile3_t,
    TempDsSclAclControlProfile4_t,
    TempDsSclAclControlProfile5_t,
    TempDsSclAclControlProfile6_t,
    TempDsVsi_t,
    TempEgrAclLkpLevel_t,
    TempEgrAclLookupInfo_t,
    TempEgrIntfAclControl_t,
    TempInfoBufferStore_t,
    TempInfoEpeAcl_t,
    TempInfoEpeClassification_t,
    TempInfoEpeEgressEdit_t,
    TempInfoEpeHdrEdit_t,
    TempInfoEpeInnerL2Edit_t,
    TempInfoEpeL3Edit_t,
    TempInfoEpeOam_t,
    TempInfoEpeOuterL2Edit_t,
    TempInfoEpePayloadOp_t,
    TempInfoIpeFwd_t,
    TempInfoIpeHdrAdj_t,
    TempInfoIpeTunnelDecaps_t,
    TempInfoIpeUserId_t,
    TempInfoMetFifoDebugLookup_t,
    TempInfoMetFifoException_t,
    TempInfoMetFifoIsolation_t,
    TempIngAclLkpLevel_t,
    TempIngAclLookupInfo_t,
    TempIngrIntfAclControl_t,
    TempIpfixSamplingCount_t,
    TempIpfixSamplingCtl_t,
    TempIpfixSamplingProfile_t,
    TempL2NewHeader_t,
    TempL2NewHeaderOuter_t,
    TempL3EditInfo_t,
    TempL3NewHeader_t,
    TempL3NewHeaderExtra_t,
    TempL3NewHeaderOuter_t,
    TempLpmTcamInfo_t,
    TempMfpMeterRefresh_t,
    TempMfpRecordDs_t,
    TempNetGroupUpdateInfo_t,
    TempParserL3NewHeader_t,
    TempParserResult_t,
    TempParserUdfKey_t,
    TempQMgrBaseSchInfo_t,
    TempQMgrEopInfo_t,
    TempQMgrExtSchInfo_t,
    TempQMgrSchDmaInfo_t,
    TempQMgrSchMiscInfo_t,
    TempQWriteQcnInfo_t,
    TempQcnProcInfo_t,
    TempSclTcamData_t,
    TempSclTcamLookupKey0_t,
    TempSclTcamLookupKey1_t,
    TempTcamEngine_t,
    BufStoreDebugCamPR_t,
    CFHeaderExtTemp1_t,
    DsFwdTemp_t,
    MsStatsUpdate_t,
    TempBsrShareInfo_t,
    TempDbgParserInfo_t,
    TempDebugDsReplicationCtl_t,
    TempDsAcl_t,
    TempDsAclVlanActionProfile_t,
    TempDsDestMap_t,
    TempDsEthOamChan_t,
    TempDsEthOamChanFunc_t,
    TempDsFibHost0FcoeHashKey_t,
    TempDsFibHost0Ipv4McastHashKey_t,
    TempDsFibHost0Ipv4UcastHashKey_t,
    TempDsFibHost0Ipv6McastHashKey_t,
    TempDsFibHost0Ipv6UcastHashKey_t,
    TempDsFibHost0MacDaHashKey_t,
    TempDsFibHost0MacHashKey_t,
    TempDsFibHost0MacIpv4McastHashKey_t,
    TempDsFibHost0MacIpv6McastHashKey_t,
    TempDsFibHost0MacSaHashKey_t,
    TempDsFibHost0TrillMcastHashKey_t,
    TempDsFibHost0TrillUcastHashKey_t,
    TempDsFibHost1FcoeRpfHashKey_t,
    TempDsFibHost1Ipv4McastHashKey_t,
    TempDsFibHost1Ipv4NatDaPortHashKey_t,
    TempDsFibHost1Ipv4NatSaPortHashKey_t,
    TempDsFibHost1Ipv6McastHashKey_t,
    TempDsFibHost1Ipv6NatDaPortHashKey_t,
    TempDsFibHost1Ipv6NatSaPortHashKey_t,
    TempDsFibHost1MacIpv4McastHashKey_t,
    TempDsFibHost1MacIpv6McastHashKey_t,
    TempDsFibHost1TrillMcastVlanHashKey_t,
    TempDsFlowL2HashKey_t,
    TempDsFlowL2L3HashKey_t,
    TempDsFlowL3Ipv4HashKey_t,
    TempDsFlowL3Ipv6HashKey_t,
    TempDsFlowL3MplsHashKey_t,
    TempDsIpfixL2HashKey_t,
    TempDsIpfixL2L3HashKey_t,
    TempDsIpfixL3Ipv4HashKey_t,
    TempDsIpfixL3Ipv6HashKey_t,
    TempDsIpfixL3MplsHashKey_t,
    TempDsLmChan0_t,
    TempDsLmChan1_t,
    TempDsLmChan2_t,
    TempDsLpmTcamIpv4NatDoubleKey_t,
    TempDsLpmTcamIpv4PbrDoubleKey_t,
    TempDsLpmTcamIpv4RpfHalfKey_t,
    TempDsLpmTcamIpv4UcHalfKey_t,
    TempDsLpmTcamIpv6NatDoubleKey_t,
    TempDsLpmTcamIpv6PbrQuadKey_t,
    TempDsLpmTcamIpv6RpfDoubleKey_t,
    TempDsLpmTcamIpv6RpfSingleKey_t,
    TempDsLpmTcamIpv6UcDoubleKey_t,
    TempDsLpmTcamIpv6UcSingleKey_t,
    TempDsOamChan_t,
    TempDsReplicationCtl_t,
    TempDsSclFlow_t,
    TempDsSclVlanActionProfile_t,
    TempDsUserId_t,
    TempDsVlanActionProfile_t,
    TempErmMbInfo_t,
    TempErmResrcMonInfo_t,
    TempFibDsFcoeDa_t,
    TempFibDsFcoeSa_t,
    TempFibDsFlow_t,
    TempFibDsIpDa_t,
    TempFibDsIpSaNat_t,
    TempFibDsIpSaPbr_t,
    TempFibDsIpSaRpf_t,
    TempFibDsMacDa_t,
    TempFibDsMacSa_t,
    TempFibDsTrillDa_t,
    TempInfoMetFifo_t,
    TempInfoMetFifoCommProc_t,
    TempInfoMeterEngine_t,
    TempInfoMeterRefresh_t,
    TempIpfixConfig_t,
    TempIrmResrcMonInfo_t,
    TempLmInfo_t,
    TempMepDownInfo_t,
    TempMepUpInfo_t,
    TempPacketHeader_t,
    TempQMgrInfo_t,
    TempQMgrSchNetInfo_t,
    TempQWriteInfo_t,
    TempQWriteRtnInfo_t,
    TempResrcGrpIdInfo_t,
    /*D2 new start*/
    TempStackingHeader_t,
    BufRetrvFirstFragRam_t,
    BufRetrvLinkListRam_t,
    BufRetrvMsgParkMem_t,
    BufRetrvPktMsgMem_t,
    McuSupProgMem0_t,
    McuSupProgMem1_t,
    DsMsgUsedList_t,
    DsMsgFreePtr_t,
    QMgrMsgStoreInterruptFatal_t,
    EpeScheduleBodyRam_t,
    EpeScheduleResidualRam_t,
    EpeScheduleInterruptNormal_t,
    EpeScheduleInterruptFatal_t,
    McuSupDataMem0_t,
    McuSupDataMem1_t,
    NetTxInterruptFatal_t,
    NetTxInterruptNormal_t,
    NetTxPktHdr0_t,
    NetTxPktHdr1_t,
    NetTxPktHdr2_t,
    NetTxPktHdr3_t,
    DsMepOamInputFifo_t,
    EgrOamHashInterruptFatal_t,
    EgrOamHashInterruptNormal_t,
    QuadSgmacInterruptNormal0_t,
    EpeScheduleSopRam_t,
    OamDsMaData_t,
    OamDsMaNameData_t,
    OamDsMpDataMask_t,
    OamDsMpData_t,
    DlbEngineCurrentTsCtl_t,
    DmaDescCache_t,
    DmaDynInfo_t,
    DmaIntfRdLog_t,
    DmaIntrVecRec_t,
    DmaLogTime_t,
    DmaRdReqFifo_t,
    DmaRegRd2Fifo_t,
    DmaRegRdResFifo_t,
    DmaWrReqAttrFifo_t,
    DmaWrReqDataFifo_t,
    DsIpeStormCtl0CountXFrac_t,
    DsQMgrDmaQueTokenFrac_t,
    Gbif0UtlReg_t,
    OamProcCreditStatus_t,
    OamUpdateStatus_t,
    QMgrDmaChanShpToken_t,
    QMgrNetBaseQueMeterFracState_t,
    TsEngineRefRc_t,
    /*D2 new end*/
    MaxTblId_t
};

