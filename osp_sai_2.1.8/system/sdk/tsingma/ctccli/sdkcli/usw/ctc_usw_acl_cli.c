/**
 @date 2009-12-22

 @version v2.0

---file comments----
*/

/****************************************************************************
 *
 * Header files
 *
 *****************************************************************************/
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_acl_cli.h"
#include "ctc_mirror.h"
#include "ctc_field.h"

#include "ctc_port_mapping_cli.h"


extern int32
sys_usw_acl_show_entry(uint8 lchip, uint8 type, uint32 param, uint8 key_type, uint8 detail, ctc_field_key_t* p_grep, uint8 grep_cnt);

extern int32
sys_usw_acl_show_entry_distribution(uint8 lchip, uint8 dir, uint8 priority);

extern int32
sys_usw_acl_show_status(uint8 lchip);

extern int32
sys_usw_acl_set_global_cfg(uint8 lchip, uint8 property, uint32 value);

extern int32
sys_usw_acl_show_tcp_flags(uint8 lchip);

extern int32
sys_usw_acl_show_field_range(uint8 lchip);

extern int32
sys_usw_acl_set_tcam_mode(uint8 lchip, uint8 mode);

extern int32
sys_usw_dma_set_tcam_interval(uint8 lchip, uint32 interval);

extern int32
sys_usw_acl_show_group(uint8 lchip, uint8 type);

extern int32
sys_usw_acl_show_cid_pair(uint8 lchip, uint8 type);

extern int32
sys_usw_acl_show_tcam_alloc_status(uint8 lchip, uint8 dir, uint8 block_id);

extern int32
sys_usw_acl_set_cid_priority(uint8 lchip, uint8 is_src_cid, uint8* p_prio_arry);
extern int32
sys_usw_acl_dump_udf_entry_info(uint8 lchip);
extern int32 sys_usw_acl_set_sort_mode(uint8 lchip);

#define CTC_CLI_ACL_COMMON_VAR \
    mac_addr_t  mac_da_addr = {0};\
    mac_addr_t  mac_da_addr_mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};\
    mac_addr_t  mac_sa_addr = {0};\
    mac_addr_t  mac_sa_addr_mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};\
    mac_addr_t  arp_sender_mac_addr = {0};\
    mac_addr_t  arp_sender_mac_addr_mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};\
    mac_addr_t  arp_target_mac_addr = {0};\
    mac_addr_t  arp_target_mac_addr_mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};\
    mac_addr_t  wlan_radio_mac_addr = {0};\
    mac_addr_t  wlan_radio_mac_addr_mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};\
    ipv6_addr_t ipv6_da_addr = {0};\
    ipv6_addr_t ipv6_da_addr_mask = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};\
    ipv6_addr_t ipv6_sa_addr = {0};\
    ipv6_addr_t ipv6_sa_addr_mask = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};\
    uint32 svrange_gid = 0;\
    uint32 cvrange_gid = 0; \
    uint32 discard_type = 0;
/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/

CTC_CLI(ctc_cli_usw_acl_show_group_info,
        ctc_cli_usw_acl_show_group_info_cmd,
        "show acl group-info (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_ACL_STR,
        "Group info")
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    uint8 type = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_acl_show_group(lchip, type);
    if (ret < 0)
    {
        ctc_cli_out("ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_acl_show_status,
        ctc_cli_usw_acl_show_status_cmd,
        "show acl status (lchip LCHIP|)",
        "Show",
        CTC_CLI_ACL_STR,
        "Status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_acl_show_status(lchip);
    if (ret < 0)
    {
        ctc_cli_out("ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_usw_acl_set_global_cfg,
        ctc_cli_usw_acl_set_global_cfg_cmd,
        "acl global-cfg (ig-service-acl-en | eg-service-acl-en |\
         ig-cid-overwrite-gport | eg-cid-overwrite-gport |\
         vlan-cid-high-priority) value VALUE (lchip LCHIP|)",
        CTC_CLI_ACL_STR,
        "Global configuration",
        "Ingress service acl enable (bitmap 0x00~0xFF)",
        "Egress service acl enable (bitmap 0x0~0x7)",
        "Ingress CategoryId overwrite gport (bitmap 0x00~0xFF)",
        "Egress CategoryId overwrite gport (bitmap 0x0~0x7)",
        "Vlan-cid has high priority for brg-pkt",
        "matched property value",
        "Value",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    int32 value = 0;
    int32 property = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    CTC_CLI_GET_UINT32("Property_value", value, argv[1]);

    if CLI_CLI_STR_EQUAL("ig-service-acl-en", 0)
    {
        property = 0;  /* SYS_ACL_IGS_SERVICE_ACL_EN */
    }
    else if CLI_CLI_STR_EQUAL("eg-service-acl-en", 0)
    {
        property = 1;  /* SYS_ACL_EGS_SERVICE_ACL_EN */
    }
    else if CLI_CLI_STR_EQUAL("ig-cid-overwrite-gport", 0)
    {
        property = 2;  /* SYS_ACL_IGS_CID_OVER_GPORT_EN */
    }
    else if CLI_CLI_STR_EQUAL("eg-cid-overwrite-gport", 0)
    {
        property = 3;  /* SYS_ACL_EGS_CID_OVER_GPORT_EN */
    }
    else if CLI_CLI_STR_EQUAL("vlan-cid-high-priority", 0)
    {
        property = 4;  /*vlan cid high priority*/
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_acl_set_global_cfg(lchip, property, value);

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_usw_acl_show_field_range,
        ctc_cli_usw_acl_show_field_range_cmd,
        "show acl field-range (lchip LCHIP|)",
        "Show",
        CTC_CLI_ACL_STR,
        "Field Range",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_acl_show_field_range(lchip);

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

#define CTC_CLI_ACL_REVERT_U32_ARRAY(d) \
    do\
    {\
        uint32 temp = 0; \
        uint8  i = 0;\
        uint8 len = sizeof(d)/sizeof(uint32);\
        for(i=0; i<len/2; i++)\
        {\
            temp = d[i]; d[i]=d[len-i-1];d[len-i-1]=temp;\
        }\
    }while(0)

#define CTC_CLI_ACL_KEY_FIELD_STR_0 "\
  l2-type (VALUE (MASK|)|) \
| l3-type (VALUE (MASK|)|) \
| l4-type (VALUE (MASK|)|) \
| l4-user-type (VALUE (MASK|)|) \
| field-port (port-class (CLASS_ID (MASK|)|) | vlan-class (CLASS_ID (MASK|)|) | l3if-class  (CLASS_ID (MASK|)|) | pbr-class (CLASS_ID (MASK|)|) | logic-port (LOGICAL_PORT (MASK|)|) | gport (GPORT (MASK|)|) | port-bitmap (LCHIP PORT_BITMAP_3 PORT_BITMAP_2 PORT_BITMAP_1 PORT_BITMAP_0|)) \
| mac-sa (VALUE (MASK|)|) \
| mac-da (VALUE (MASK|)|) \
| stag-valid (VALUE (MASK|)|) \
| svlan-id (VALUE (MASK|)|) \
| stag-cos (VALUE (MASK|)|) \
| stag-cfi (VALUE (MASK|)|) \
| ctag-valid (VALUE (MASK|)|) \
| cvlan-id (VALUE (MASK|)|) \
| ctag-cos (VALUE (MASK|)|) \
| ctag-cfi (VALUE (MASK|)|) \
| svlan-range (MIN MAX vrange-gid GID |)  \
| cvlan-range (MIN MAX vrange-gid GID |) \
| ether-type (VALUE (MASK|)|) \
| vlan-num (VALUE (MASK|)|) \
| vlan-xlate-hit scl-id SCL_ID (VALUE (MASK|)|) \
| ip-sa (VALUE (MASK|)|) \
| ip-da (VALUE (MASK|)|) \
| ipv6-sa (VALUE (MASK|)|) \
| ipv6-da (VALUE (MASK|)|) \
| ipv6-flow-label (VALUE (MASK|)|) \
| ip-protocol (VALUE (MASK|)|) \
| ip-dscp (VALUE (MASK|)|) \
| ip-precedence (VALUE (MASK|)|) \
| ip-ecn (VALUE (MASK|)|) \
| ip-frag (VALUE (MASK|)|) \
| ip-hdr-error (VALUE (MASK|)|) \
| ip-options (VALUE (MASK|)|) \
| ip-pkt-len-range (MIN (MAX|)|) \
| ip-ttl (VALUE (MASK|)|) \
| arp-op-code (VALUE (MASK|)|) \
| arp-protocol-type (VALUE (MASK|)|) \
| arp-sender-ip (VALUE (MASK|)|) \
| arp-target-ip (VALUE (MASK|)|) \
| rarp (VALUE (MASK|)|) \
| l4-dst-port (VALUE (MASK|)|) \
| l4-src-port (VALUE (MASK|)|) \
| l4-src-port-range (MIN (MAX|)|) \
| l4-dst-port-range (MIN (MAX|)|) \
| tcp-ecn (VALUE (MASK|)|) \
| tcp-flags (VALUE (MASK|)|) \
| gre-key (VALUE (MASK|)|) \
| nvgre-key (VALUE (MASK|)|) \
| vn-id (VALUE (MASK|)|) \
| vxlan-flags (VALUE (MASK|)|) \
| vxlan-rsv1 (VALUE (MASK|)|) \
| vxlan-rsv2 (VALUE (MASK|)|) \
| icmp-code (VALUE (MASK|)|) \
| icmp-type (VALUE (MASK|)|) \
| igmp-type (VALUE (MASK|)|) \
| label-num (VALUE (MASK|)|) \
| mpls-label0 (VALUE (MASK|)|) \
| mpls-exp0 (VALUE (MASK|)|) \
| mpls-sbit0 (VALUE (MASK|)|) \
| mpls-ttl0 (VALUE (MASK|)|) \
| arp-sender-mac (VALUE (MASK|)|) \
| arp-target-mac (VALUE (MASK|)|) \
| aware-tunnel-info (grekey (VALUE (MASK|)|) | vnid (VALUE (MASK|)|) | (wlan {radio-mac VALUE (MASK|)|radio-id VALUE (MASK|)|ctl-pkt VALUE (MASK|)|} ))  \
| stp-state (VALUE (MASK|)|) \
| arp-mac-da-chk-fail (VALUE (MASK|)|) \
| arp-mac-sa-chk-fail (VALUE (MASK|)|) \
| arp-senderip-chk-fail (VALUE (MASK|)|) \
| arp-targetip-chk-fail (VALUE (MASK|)|) \
| garp (VALUE (MASK|)|) \
| is-log-pkt (VALUE (MASK|)|) \
| satpdu-pdu-byte (VALUE (MASK|)|) \
| l2-station-move (VALUE (MASK|)|) \
| mac-security-discard (VALUE (MASK|)|) \
| pkt-fwd-type (VALUE (MASK|)|) \
| is-mcast-rpf-chk-fail (VALUE (MASK|)|) \
| is-route-mac (VALUE (MASK|)|) \
"

#define CTC_CLI_ACL_KEY_FIELD_STR_1 "\
  mpls-label1 (VALUE (MASK|)|) \
| mpls-exp1 (VALUE (MASK|)|) \
| mpls-sbit1 (VALUE (MASK|)|) \
| mpls-ttl1 (VALUE (MASK|)|) \
| mpls-label2 (VALUE (MASK|)|) \
| mpls-exp2 (VALUE (MASK|)|) \
| mpls-sbit2 (VALUE (MASK|)|) \
| mpls-ttl2 (VALUE (MASK|)|) \
| nsh-cbit (VALUE (MASK|)|) \
| nsh-obit (VALUE (MASK|)|) \
| nsh-next-protocol (VALUE (MASK|)|) \
| nsh-si (VALUE (MASK|)|) \
| nsh-spi (VALUE (MASK|)|) \
| is-y1731-oam (VALUE (MASK|)|) \
| ether-oam-level (VALUE (MASK|)|) \
| ether-oam-op-code (VALUE (MASK|)|) \
| ether-oam-version (VALUE (MASK|)|) \
| slow-protocol-code (VALUE (MASK|)|) \
| slow-protocol-flags (VALUE (MASK|)|) \
| slow-protocol-sub-type (VALUE (MASK|)|) \
| ptp-message-type (VALUE (MASK|)|) \
| ptp-version (VALUE (MASK|)|) \
| fcoe-dst-fcid (VALUE (MASK|)|) \
| fcoe-src-fcid (VALUE (MASK|)|) \
| wlan-radio-mac (VALUE (MASK|)|) \
| wlan-radio-id (VALUE (MASK|)|) \
| wlan-ctl-pkt (VALUE (MASK|)|) \
| satpdu-mef-oui (VALUE (MASK|)|) \
| satpdu-oui-sub-type (VALUE (MASK|)|) \
| ingress-nickname (VALUE (MASK|)|) \
| egress-nickname (VALUE (MASK|)|) \
| is-esadi (VALUE (MASK|)|) \
| is-trill-channel (VALUE (MASK|)|) \
| trill-inner-vlanid (VALUE (MASK|)|) \
| trill-inner-vlanid-valid (VALUE (MASK|)|) \
| trill-length (VALUE (MASK|)|) \
| trill-multihop (VALUE (MASK|)|) \
| trill-multicast (VALUE (MASK|)|) \
| trill-version (VALUE (MASK|)|) \
| trill-ttl (VALUE (MASK|)|) \
| dst-category-id (VALUE (MASK|)|) \
| src-category-id (VALUE (MASK|)|) \
| udf udf-id UDF_ID {udf-data VALUE MASK| } \
| decap (VALUE (MASK|)|) \
| elephant-pkt (VALUE (MASK|)|) \
| vxlan-pkt (VALUE (MASK|)|) \
| routed-pkt (VALUE (MASK|)|) \
| macsa-lkup (VALUE (MASK|)|) \
| macsa-hit (VALUE (MASK|)|) \
| macda-lkup (VALUE (MASK|)|) \
| macda-hit (VALUE (MASK|)|) \
| ipsa-lkup (VALUE (MASK|)|) \
| ipsa-hit (VALUE (MASK|)|) \
| ipda-lkup (VALUE (MASK|)|) \
| ipda-hit (VALUE (MASK|)|) \
| discard (VALUE (MASK|)|) (discard-type TYPE|) \
| cpu-reason-id (VALUE (MASK|)|) \
| dst-gport (VALUE (MASK|)|) \
| dst-nhid (VALUE (MASK|)|) \
| interface-id (VALUE (MASK|)|) \
| vrfid (VALUE (MASK|)|) \
| metadata (VALUE (MASK|)|) \
| customer-id (VALUE (MASK|)|) \
| hash-valid (VALUE (MASK|)|) \
| dot1ae-es(VALUE (MASK|)|) \
| dot1ae-sc(VALUE (MASK|)|) \
| dot1ae-an(VALUE (MASK|)|) \
| dot1ae-sl(VALUE (MASK|)|) \
| dot1ae-pn(VALUE (MASK|)|) \
| dot1ae-sci(VALUE (MASK|)|) \
| dot1ae-unknow-pkt (VALUE (MASK|)|) \
|dot1ae-cbit(VALUE (MASK|)|) \
|dot1ae-ebit(VALUE (MASK|)|) \
|dot1ae-scb(VALUE (MASK|)|) \
|dot1ae-ver(VALUE (MASK|)|) \
| gem-port (VALUE (MASK|)|) \
| class-id (VALUE (MASK|)|) \
"


#define CTC_CLI_ACL_KEY_FIELD_DESC_0 \
"layer 2 type (ctc_parser_l2_type_t) ", "value", "mask",\
"layer 3 type (ctc_parser_l3_type_t) ", "value", "mask",\
"layer 4 type (ctc_parser_l4_type_t) ", "value", "mask",\
"layer 4 user-type (ctc_parser_l4_usertype_t) ", "value", "mask",\
"port-type","port class id", "value","mask","vlan class id","value", "mask","l3if class id","value", "mask","pbr class id","value", "mask","logic port","value", "mask","global port","value", "mask",\
CTC_CLI_ACL_PORT_BITMAP,CTC_CLI_LCHIP_ID_VALUE,"port bitmap_3 <port96~port127>","port bitmap_2 <port64~port95>","port bitmap_1 <port32~port63>","port bitmap_0 <port0~port31>",\
"source mac address ","value", "mask",\
"destination mac address ", "value", "mask",\
"s-vlan exist ", "value", "mask",\
"s-vlan id ", "value", "mask",\
"stag cos ", "value", "mask",\
"stag cfi ", "value", "mask",\
"c-vlan exist ", "value", "mask",\
"c-vlan id ", "value", "mask",\
"ctag cos ", "value", "mask",\
"ctag cfi ", "value", "mask",\
"s-vlan range", "min", "max", "vlan range group id", "VALUE",\
"c-vlan range", "min", "max", "vlan range group id", "VALUE",\
"ether type ", "value", "mask",\
"vlan tag number", "value", "mask",\
"vlan xlate hit", "scl id", "SCL ID", "value", "mask", \
"source ipv4 address ", "value", "mask",\
"destination ipv4 address ", "value", "mask",\
"source ipv6 address ", "value", "mask",\
"destination ipv6 address ", "value", "mask",\
"ipv6 flow label", "value", "mask",\
"ip protocol ", "value", "mask",\
"dscp ", "value", "mask",\
"precedence ", "value", "mask",\
"ecn ", "value", "mask",\
"ip fragment information ", "value", "mask",\
"ip header error ", "value", "mask",\
"ip options ", "value", "mask",\
"ip packet length range ", "min", "max",\
"ttl ", "value", "mask",\
"opcode field of arp header ", "value", "mask",\
"protocol type of arp header ", "value", "mask",\
"sender ipv4 field of arp header ", "value", "mask",\
"target ipv4 field of arp header ", "value", "mask",\
"rarp", "value", "mask", \
"layer 4 dest port ", "value", "mask",\
"layer 4 src port ", "value", "mask",\
"layer 4 src port range ", "min", "max",\
"layer 4 dest port range ", "min", "max",\
"tcp ecn ", "value", "mask",\
"tcp flags ", "value", "mask",\
"gre key ", "value", "mask",\
"nvgre key ", "value", "mask",\
"vxlan network id ", "value", "mask",\
"vxlan flags", "value", "mask", \
"vxlan reserved 1", "value", "mask", \
"vxlan reserved 2", "value", "mask", \
"icmp code", "value", "mask",\
"icmp type", "value", "mask",\
"igmp type", "value", "mask",\
"mpls label number ", "value", "mask",\
"label field of the mpls label 0 ", "value", "mask",\
"exp field of the mpls label 0", "value", "mask",\
"s-bit field of the mpls label 0", "value", "mask",\
"ttl field of the mpls label 0", "value", "mask",\
"Ethernet Address of sender of ARP Header","value","mask",\
"Ethernet Address of destination of ARP Header","value","mask",\
"Aware Tunnel Info","merge Key gre key","value","mask","merge Key  vxlan vni","value","mask","wlan type","radio-mac","value","mask","radio-id","value","mask","wlan-ctl-pkt","value","mask",\
"Stp Status.Refer to stp_state_e; 0:forward, 1:blocking, and 2:learning","value","mask",\
"Destination MAC Address of ARP Header Check Fail", "value", "mask",\
"Source MAC Address of ARP Header Check Fail", "value", "mask",\
"IP Address of sender of ARP Header Check Fail", "value", "mask",\
"IP Address of destination of ARP Header Check Fail", "value", "mask",\
"Gratuitous ARP","value","mask",\
"Log to Cpu Packet.","value","mask",\
"Satpdu Byte.","value","mask",\
"L2 Station Move.","value","mask",\
"Mac Security Discard.","value","mask",\
"Packet Forwarding Type"," Refer to ctc_acl_pkt_fwd_type_t","mask",\
"Mcast Rpf Check Fail","value","mask",\
"Route Mac","vlaue","mask"


#define CTC_CLI_ACL_KEY_FIELD_DESC_1 \
"label field of the mpls label 1 ", "value", "mask",\
"exp field of the mpls label 1", "value", "mask",\
"s-bit field of the mpls label 1", "value", "mask",\
"ttl field of the mpls label 1", "value", "mask",\
"label field of the mpls label 2 ", "value", "mask",\
"exp field of the mpls label 2", "value", "mask",\
"s-bit field of the mpls label 2", "value", "mask",\
"ttl field of the mpls label 2", "value", "mask",\
"c-bit of the network service header ", "value", "mask",\
"o-bit of the network service header ", "value", "mask",\
"next protocol of the network service header ", "value", "mask",\
"service index of the network service header ", "value", "mask",\
"service path id of the network service header ", "value", "mask",\
"is y1731 oam ", "value", "mask",\
"oam level ", "value", "mask",\
"oam opcode ", "value", "mask",\
"oam version ", "value", "mask",\
"slow protocol code ", "value", "mask",\
"slow protocol flags ", "value", "mask",\
"slow protocol sub type ", "value", "mask",\
"ptp message type ", "value", "mask",\
"ptp message version ", "value", "mask",\
"fcoe did", "value", "mask",\
"fcoe sid", "value", "mask",\
"wlan radio mac", "value", "mask",\
"wlan radio id", "value", "mask",\
"wlan control packet", "value", "mask",\
"satpdu metro ethernet forum oui", "value", "mask",\
"satpdu oui sub type", "value", "mask",\
"ingress nick name ", "value", "mask",\
"egress nick name ", "value", "mask",\
"is esadi ", "value", "mask",\
"is trill channel ", "value", "mask",\
"trill inner vlan id ", "value", "mask",\
"trill inner vlan id valid ", "value", "mask",\
"trill length ", "value", "mask",\
"trill multi-hop ", "value", "mask",\
"trill multicast ", "value", "mask",\
"trill version ", "value", "mask",\
"trill ttl ", "value", "mask",\
"destination category id ", "value", "mask",\
"source category id ", "value", "mask",\
"udf","udf id","udf id value","udf data","value","mask",\
"decapsulation occurred ", "value", "mask",\
"is elephant ", "value", "mask",\
"is vxlan packet ", "value", "mask",\
"is routed packet ", "value", "mask",\
"mac-sa lookup enable ", "value", "mask",\
"mac-sa lookup hit ", "value", "mask",\
"mac-da lookup enable ", "value", "mask",\
"mac-da lookup hit ", "value", "mask",\
"l3-sa lookup enable ", "value", "mask",\
"l3-sa lookup hit ", "value", "mask",\
"l3-da lookup enable ", "value", "mask",\
"l3-da lookup hit ", "value", "mask",\
"packet is flagged to be discarded ", "value", "mask", "discard type", "value",\
"cpu reason id ", "value", "mask",\
"fwd destination port ", "value", "mask",\
"nexthop id ", "value", "mask",\
"l3 interface id ", "value", "mask",\
"vrfid ", "value", "mask",\
"metadata ", "value", "mask",\
"customer id", "value", "mask",\
"hash valid key", "value", "mask",\
"end Station (ES) bit. ", "value", "mask",\
"secure Channel (SC) bit. ", "value", "mask",\
"association Number (AN). ", "value", "mask",\
"short Length (SL).", "value", "mask",\
"packet Number.", "value", "mask",\
"secure Channel Identifier. ", "value", "mask",\
"Unknow 802.1AE packet, SecTag error or Mode not support.","value", "mask", \
"Changed Text (C) bit", "value", "mask", \
"Encryption (E) bit", "value", "mask", \
"Single Copy Broadcast (SCB) bit", "value", "mask", \
"Version Number (V) bit", "value", "mask", \
"Gem Port", "value", "mask",\
"Class ID","value", "mask"


#define CTC_CLI_ACL_KEY_FIELD_SET(key_field, arg, start)\
do{\
    index = CTC_CLI_GET_ARGC_INDEX("l2-type");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_L2_TYPE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l3-type");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_L3_TYPE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-type");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_L4_TYPE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-user-type");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_L4_USER_TYPE;\
        if(is_add)\
        {\
        CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
        if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MAC_SA;\
        if(is_add)\
        {\
            CTC_CLI_GET_MAC_ADDRESS("mac-sa", mac_sa_addr, argv[index + 1]);\
            key_field->ext_data = mac_sa_addr; \
            sal_memset(&mac_sa_addr_mask, 0xFF, sizeof(mac_addr_t));\
            if(index+2<argc){\
            CTC_CLI_GET_MAC_ADDRESS("mac-sa-mask", mac_sa_addr_mask, argv[index + 2]);\
            key_field->ext_mask = mac_sa_addr_mask;} \
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-da");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MAC_DA;\
        if(is_add)\
        {\
            key_field->type = CTC_FIELD_KEY_MAC_DA;\
            CTC_CLI_GET_MAC_ADDRESS("mac-da", mac_da_addr, argv[index + 1]);\
            key_field->ext_data = mac_da_addr; \
            sal_memset(&mac_da_addr_mask, 0xFF, sizeof(mac_addr_t));\
            if(index+2<argc){\
            CTC_CLI_GET_MAC_ADDRESS("mac-da-mask", mac_da_addr_mask, argv[index + 2]);\
            key_field->ext_mask = mac_da_addr_mask;} \
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stag-valid");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_STAG_VALID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("svlan-id");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_SVLAN_ID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if (index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]); \
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_STAG_COS;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stag-cfi");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_STAG_CFI;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ctag-valid");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_CTAG_VALID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cvlan-id");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_CVLAN_ID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_CTAG_COS;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cfi");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_CTAG_CFI;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("svlan-range");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_SVLAN_RANGE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("min", key_field->data, argv[index + 1]);\
            CTC_CLI_GET_UINT32("max", key_field->mask, argv[index + 2]);\
            CTC_CLI_GET_UINT32("vrange-gid", svrange_gid, argv[index + 4]);\
            key_field->ext_data = &svrange_gid;\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cvlan-range");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_CVLAN_RANGE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("min", key_field->data, argv[index + 1]);\
            CTC_CLI_GET_UINT32("max", key_field->mask, argv[index + 2]);\
            CTC_CLI_GET_UINT32("vrange-gid", cvrange_gid, argv[index + 4]);\
            key_field->ext_data = &cvrange_gid;\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ether-type");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_ETHER_TYPE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vlan-num");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_VLAN_NUM;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vlan-xlate-hit");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_VLAN_XLATE_HIT;\
        if(is_add)\
        {\
            index = CTC_CLI_GET_ARGC_INDEX("scl-id");\
            CTC_CLI_GET_UINT8("scl-id", scl_id, argv[index + 1]);\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 2]);\
            if(index+3<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 3]);\
            key_field->ext_data = (void*)&scl_id;\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IP_SA;\
        if(is_add)\
        {\
            CTC_CLI_GET_IPV4_ADDRESS("ipsa", key_field->data, argv[index + 1]);\
            key_field->mask = 0xFFFFFFFF;\
            if(index+2<argc) CTC_CLI_GET_IPV4_ADDRESS("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-da");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IP_DA;\
        if(is_add)\
        {\
            CTC_CLI_GET_IPV4_ADDRESS("ipda", key_field->data, argv[index + 1]);\
            key_field->mask = 0xFFFFFFFF;\
            if(index+2<argc) CTC_CLI_GET_IPV4_ADDRESS("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-sa");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IPV6_SA;\
        if(is_add)\
        {\
            CTC_CLI_GET_IPV6_ADDRESS("ip-sa", ipv6_sa_addr, argv[index + 1]);               \
            ipv6_sa_addr[0] = sal_htonl(ipv6_sa_addr[0]);                                 \
            ipv6_sa_addr[1] = sal_htonl(ipv6_sa_addr[1]);                                 \
            ipv6_sa_addr[2] = sal_htonl(ipv6_sa_addr[2]);                                 \
            ipv6_sa_addr[3] = sal_htonl(ipv6_sa_addr[3]);                                 \
            key_field->ext_data = ipv6_sa_addr;    \
            if(index+2<argc)\
            {\
                if CLI_CLI_STR_EQUAL("HOST", index + 2)                                          \
                {                                                                             \
                    ipv6_sa_addr_mask[0] = 0xFFFFFFFF;                                            \
                    ipv6_sa_addr_mask[1] = 0xFFFFFFFF;                                            \
                    ipv6_sa_addr_mask[2] = 0xFFFFFFFF;                                            \
                    ipv6_sa_addr_mask[3] = 0xFFFFFFFF;                                            \
                    key_field->ext_mask = ipv6_sa_addr_mask;    \
                }                                                                             \
                else                                                                          \
                {                                                                             \
                    CTC_CLI_GET_IPV6_ADDRESS("ip-sa-mask", ipv6_sa_addr_mask, argv[index + 2]);          \
                    ipv6_sa_addr_mask[0] = sal_htonl(ipv6_sa_addr_mask[0]);                                 \
                    ipv6_sa_addr_mask[1] = sal_htonl(ipv6_sa_addr_mask[1]);                                 \
                    ipv6_sa_addr_mask[2] = sal_htonl(ipv6_sa_addr_mask[2]);                                 \
                    ipv6_sa_addr_mask[3] = sal_htonl(ipv6_sa_addr_mask[3]);                                 \
                    key_field->ext_mask = ipv6_sa_addr_mask;    \
                }\
            }\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-da");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IPV6_DA;\
        if(is_add)\
        {\
            CTC_CLI_GET_IPV6_ADDRESS("ip-da", ipv6_da_addr, argv[index + 1]);               \
            ipv6_da_addr[0] = sal_htonl(ipv6_da_addr[0]);                                 \
            ipv6_da_addr[1] = sal_htonl(ipv6_da_addr[1]);                                 \
            ipv6_da_addr[2] = sal_htonl(ipv6_da_addr[2]);                                 \
            ipv6_da_addr[3] = sal_htonl(ipv6_da_addr[3]);                                 \
            key_field->ext_data = ipv6_da_addr;    \
            if(index+2<argc)\
            {\
                if CLI_CLI_STR_EQUAL("HOST", index + 2)                                          \
                {                                                                             \
                    ipv6_da_addr_mask[0] = 0xFFFFFFFF;                                            \
                    ipv6_da_addr_mask[1] = 0xFFFFFFFF;                                            \
                    ipv6_da_addr_mask[2] = 0xFFFFFFFF;                                            \
                    ipv6_da_addr_mask[3] = 0xFFFFFFFF;                                            \
                    key_field->ext_mask = ipv6_da_addr_mask;    \
                }                                                                             \
                else                                                                          \
                {                                                                             \
                    CTC_CLI_GET_IPV6_ADDRESS("ip-da-mask", ipv6_da_addr_mask, argv[index + 2]);          \
                    ipv6_da_addr_mask[0] = sal_htonl(ipv6_da_addr_mask[0]);                                 \
                    ipv6_da_addr_mask[1] = sal_htonl(ipv6_da_addr_mask[1]);                                 \
                    ipv6_da_addr_mask[2] = sal_htonl(ipv6_da_addr_mask[2]);                                 \
                    ipv6_da_addr_mask[3] = sal_htonl(ipv6_da_addr_mask[3]);                                 \
                    key_field->ext_mask = ipv6_da_addr_mask;    \
                } \
            }\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-flow-label");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IPV6_FLOW_LABEL;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-protocol");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IP_PROTOCOL;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-dscp");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IP_DSCP;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-precedence");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IP_PRECEDENCE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-ecn");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IP_ECN;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IP_FRAG;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-hdr-error");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IP_HDR_ERROR;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-options");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IP_OPTIONS;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-pkt-len-range");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IP_PKT_LEN_RANGE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-ttl");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IP_TTL;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("arp-op-code");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_ARP_OP_CODE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("arp-protocol-type");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_ARP_PROTOCOL_TYPE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("arp-sender-ip");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_ARP_SENDER_IP;\
        if(is_add)\
        {\
            CTC_CLI_GET_IPV4_ADDRESS("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_IPV4_ADDRESS("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("arp-target-ip");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_ARP_TARGET_IP;\
        if(is_add)\
        {\
            CTC_CLI_GET_IPV4_ADDRESS("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_IPV4_ADDRESS("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("rarp");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_RARP;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_L4_DST_PORT;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_L4_SRC_PORT;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port-range");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_L4_SRC_PORT_RANGE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port-range");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_L4_DST_PORT_RANGE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("tcp-ecn");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_TCP_ECN;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("tcp-op");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_TCP_OPTIONS;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_TCP_FLAGS;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("gre-key");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_GRE_KEY;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("gre-flags");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_GRE_FLAGS;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("gre-protocol");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_GRE_PROTOCOL_TYPE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("nvgre-key");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_NVGRE_KEY;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vn-id");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_VN_ID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vxlan-flags");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_VXLAN_FLAGS;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vxlan-rsv1");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_VXLAN_RSV1;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("vxlan-rsv1", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vxlan-rsv2");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_VXLAN_RSV2;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("vxlan-rsv2", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("icmp-code");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_ICMP_CODE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("icmp-type");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_ICMP_TYPE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("igmp-type");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IGMP_TYPE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("label-num");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_LABEL_NUM;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label0");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MPLS_LABEL0;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-exp0");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MPLS_EXP0;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-sbit0");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MPLS_SBIT0;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-ttl0");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MPLS_TTL0;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label1");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MPLS_LABEL1;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-exp1");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MPLS_EXP1;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-sbit1");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MPLS_SBIT1;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-ttl1");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MPLS_TTL1;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label2");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MPLS_LABEL2;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-exp2");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MPLS_EXP2;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-sbit2");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MPLS_SBIT2;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-ttl2");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MPLS_TTL2;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("nsh-cbit");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_NSH_CBIT;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("nsh-obit");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_NSH_OBIT;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("nsh-next-protocol");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_NSH_NEXT_PROTOCOL;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("nsh-si");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_NSH_SI;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("nsh-spi");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_NSH_SPI;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("is-y1731-oam");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IS_Y1731_OAM;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ether-oam-level");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_ETHER_OAM_LEVEL;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ether-oam-op-code");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_ETHER_OAM_OP_CODE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ether-oam-version");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_ETHER_OAM_VERSION;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("slow-protocol-code");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_SLOW_PROTOCOL_CODE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("slow-protocol-flags");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("slow-protocol-sub-type");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ptp-message-type");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_PTP_MESSAGE_TYPE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ptp-version");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_PTP_VERSION;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("fcoe-dst-fcid");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_FCOE_DST_FCID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("fcoe-src-fcid");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_FCOE_SRC_FCID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("wlan-radio-mac");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_WLAN_RADIO_MAC;\
        if(is_add)\
        {\
            CTC_CLI_GET_MAC_ADDRESS("mac", wlan_radio_mac_addr, argv[index + 1]);\
            key_field->ext_data = wlan_radio_mac_addr; \
            if(index+2<argc){\
            CTC_CLI_GET_MAC_ADDRESS("mac-mask", wlan_radio_mac_addr_mask, argv[index + 2]);\
            key_field->ext_mask = wlan_radio_mac_addr_mask;} \
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("wlan-radio-id");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_WLAN_RADIO_ID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("wlan-ctl-pkt");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_WLAN_CTL_PKT;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("satpdu-mef-oui");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_SATPDU_MEF_OUI;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("satpdu-oui-sub-type");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_SATPDU_OUI_SUB_TYPE;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ingress-nickname");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_INGRESS_NICKNAME;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("egress-nickname");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_EGRESS_NICKNAME;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("is-esadi");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IS_ESADI;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("is-trill-channel");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IS_TRILL_CHANNEL;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-inner-vlanid");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_TRILL_INNER_VLANID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-inner-vlanid-valid");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-length");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_TRILL_LENGTH;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-multihop");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_TRILL_MULTIHOP;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-multicast");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_TRILL_MULTICAST;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-version");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_TRILL_VERSION;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-ttl");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_TRILL_TTL;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("dst-category-id");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_DST_CID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("src-category-id");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_SRC_CID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_SPECIFIC_INDEX("udf", start);\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_UDF;\
        index = CTC_CLI_GET_ARGC_INDEX("udf-id");\
        if (INDEX_VALID(index))\
        {\
            CTC_CLI_GET_UINT16("udf-id", udf_data.udf_id, argv[index + 1]);         \
        }\
        if(is_add)\
        {\
            index = CTC_CLI_GET_ARGC_INDEX("udf-data");\
            if (INDEX_VALID(index))\
            {\
                uint32 data_num = 0;\
                data_num = sal_sscanf(argv[index+1], "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx", \
                    (uint8*)udf_data.udf,(uint8*)udf_data.udf+1,(uint8*)udf_data.udf+2,(uint8*)udf_data.udf+3,(uint8*)udf_data.udf+4,(uint8*)udf_data.udf+5,(uint8*)udf_data.udf+6,(uint8*)udf_data.udf+7,(uint8*)udf_data.udf+8,\
                    (uint8*)udf_data.udf+9,(uint8*)udf_data.udf+10,(uint8*)udf_data.udf+11,(uint8*)udf_data.udf+12,(uint8*)udf_data.udf+13,(uint8*)udf_data.udf+14,(uint8*)udf_data.udf+15);\
                if (data_num<16) {ctc_cli_out("  %%  Get udf data fail! \n");\
                    return CLI_ERROR;};\
                data_num = sal_sscanf(argv[index+2], "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",\
                    (uint8*)udf_data_mask.udf,(uint8*)udf_data_mask.udf+1,(uint8*)udf_data_mask.udf+2,(uint8*)udf_data_mask.udf+3,(uint8*)udf_data_mask.udf+4,(uint8*)udf_data_mask.udf+5,(uint8*)udf_data_mask.udf+6,(uint8*)udf_data_mask.udf+7,(uint8*)udf_data_mask.udf+8,\
                    (uint8*)udf_data_mask.udf+9,(uint8*)udf_data_mask.udf+10,(uint8*)udf_data_mask.udf+11,(uint8*)udf_data_mask.udf+12,(uint8*)udf_data_mask.udf+13,(uint8*)udf_data_mask.udf+14,(uint8*)udf_data_mask.udf+15);\
                if (data_num<16) {ctc_cli_out("  %%  Get udf data mask fail! \n");\
                    return CLI_ERROR;}\
            }\
            key_field->ext_data = &udf_data;\
            key_field->ext_mask = &udf_data_mask;\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("decap");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_DECAP;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("elephant-pkt");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_ELEPHANT_PKT;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vxlan-pkt");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_VXLAN_PKT;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("routed-pkt");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_ROUTED_PKT;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("pkt-fwd-type");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_PKT_FWD_TYPE; \
        CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
        if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("macsa-lkup");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MACSA_LKUP;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("macsa-hit");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MACSA_HIT;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("macda-lkup");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MACDA_LKUP;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("macda-hit");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_MACDA_HIT;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipsa-lkup");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IPSA_LKUP;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipsa-hit");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IPSA_HIT;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipda-lkup");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IPDA_LKUP;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipda-hit");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_IPDA_HIT;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("discard");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_DISCARD;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if((index+2<argc) && (index+2 != CTC_CLI_GET_ARGC_INDEX("discard-type"))) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
            index = CTC_CLI_GET_ARGC_INDEX("discard-type");\
            if (0xFF != index) \
            {\
                    CTC_CLI_GET_UINT32("discard-type", discard_type, argv[index + 1]);\
                    key_field->ext_data = &discard_type;\
            }\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cpu-reason-id");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_CPU_REASON_ID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("dst-gport");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_DST_GPORT;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("dst-nhid");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_DST_NHID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("interface-id");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_INTERFACE_ID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vrfid");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_VRFID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("metadata");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_METADATA;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("customer-id");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_CUSTOMER_ID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("hash-valid");\
    if (0xFF != index) \
    {\
        key_field->type = CTC_FIELD_KEY_HASH_VALID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("field-port");\
    if(0xFF != index)\
    {\
        key_field->type = CTC_FIELD_KEY_PORT;\
        index = CTC_CLI_GET_ARGC_INDEX("port-class");\
        if (0xFF != index) \
        { \
            port_info.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;\
            if(is_add)\
            {\
                CTC_CLI_GET_UINT16("value", port_info.port_class_id, argv[index + 1]);\
                if(index+2<argc) CTC_CLI_GET_UINT16("mask", port_info_mask.port_class_id, argv[index + 2]);\
                key_field->ext_mask = &port_info_mask;\
            }\
            key_field->ext_data = &port_info;\
            arg;\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("vlan-class");\
        if (0xFF != index) \
        { \
            port_info.type = CTC_FIELD_PORT_TYPE_VLAN_CLASS;\
            if(is_add)\
            {\
                CTC_CLI_GET_UINT16("value", port_info.vlan_class_id, argv[index + 1]);\
                if(index+2<argc) CTC_CLI_GET_UINT16("mask", port_info_mask.vlan_class_id, argv[index + 2]);\
                key_field->ext_mask = &port_info_mask;\
            }\
            key_field->ext_data = &port_info;\
            arg;\
        } \
        index = CTC_CLI_GET_ARGC_INDEX("l3if-class");\
        if (0xFF != index) \
        { \
            port_info.type = CTC_FIELD_PORT_TYPE_L3IF_CLASS;\
            if(is_add)\
            {\
                CTC_CLI_GET_UINT16("value", port_info.l3if_class_id, argv[index + 1]);\
                if(index+2<argc) CTC_CLI_GET_UINT16("mask", port_info_mask.l3if_class_id, argv[index + 2]);\
                key_field->ext_mask = &port_info_mask;\
            }\
            key_field->ext_data = &port_info;\
            arg;\
        } \
        index = CTC_CLI_GET_ARGC_INDEX("pbr-class");\
        if (0xFF != index) \
        { \
            port_info.type = CTC_FIELD_PORT_TYPE_PBR_CLASS;\
            if(is_add)\
            {\
                CTC_CLI_GET_UINT8("value", port_info.pbr_class_id, argv[index + 1]);\
                if(index+2<argc) CTC_CLI_GET_UINT8("mask", port_info_mask.pbr_class_id, argv[index + 2]);\
                key_field->ext_mask = &port_info_mask;\
            }\
            key_field->ext_data = &port_info;\
            arg;\
        } \
        index = CTC_CLI_GET_ARGC_INDEX("gport");\
        if (0xFF != index) \
        { \
            port_info.type = CTC_FIELD_PORT_TYPE_GPORT;\
            if(is_add)\
            {\
                CTC_CLI_GET_UINT32("value", port_info.gport, argv[index + 1]);\
                if(index+2<argc) CTC_CLI_GET_UINT32("mask", port_info_mask.gport, argv[index + 2]);\
                key_field->ext_mask = &port_info_mask;\
            }\
            key_field->ext_data = &port_info;\
            arg;\
        } \
        index = CTC_CLI_GET_ARGC_INDEX("logic-port");\
        if (0xFF != index) \
        { \
            port_info.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;\
            if(is_add)\
            {\
                CTC_CLI_GET_UINT16("value", port_info.logic_port, argv[index + 1]);\
                if(index+2<argc) CTC_CLI_GET_UINT16("mask", port_info_mask.logic_port, argv[index + 2]);\
                key_field->ext_mask = &port_info_mask;\
            }\
            key_field->ext_data = &port_info;\
            arg;\
        } \
        index = CTC_CLI_GET_ARGC_INDEX("port-bitmap");\
        if(INDEX_VALID(index))\
        {\
            port_info.type = CTC_FIELD_PORT_TYPE_PORT_BITMAP;\
            if(is_add)\
            {\
                CTC_CLI_GET_UINT8("lchip", port_info.lchip, argv[index + 1]);\
                CTC_CLI_GET_UINT32("port-bitmap 3", pbmp[3], argv[index + 2]);\
                CTC_CLI_GET_UINT32("port-bitmap 2", pbmp[2], argv[index + 3]);\
                CTC_CLI_GET_UINT32("port-bitmap 1", pbmp[1], argv[index + 4]);\
                CTC_CLI_GET_UINT32("port-bitmap 0", pbmp[0], argv[index + 5]);\
                cli_acl_port_bitmap_mapping(port_info.port_bitmap, pbmp);\
            }\
            key_field->ext_data = &port_info;\
            arg;\
        }\
    }\
       index = CTC_CLI_GET_ARGC_INDEX("arp-sender-mac");\
       if (0xFF != index) \
       {\
           key_field->type = CTC_FIELD_KEY_SENDER_MAC;\
           if(is_add)\
           {\
               CTC_CLI_GET_MAC_ADDRESS("arp-sender-mac", arp_sender_mac_addr, argv[index + 1]);\
               key_field->ext_data = arp_sender_mac_addr; \
               if(index+2<argc){\
               CTC_CLI_GET_MAC_ADDRESS("arp-sender-mac-mask", arp_sender_mac_addr_mask, argv[index + 2]);\
               key_field->ext_mask = arp_sender_mac_addr_mask;} \
            }\
            arg;\
        }\
       index = CTC_CLI_GET_ARGC_INDEX("arp-target-mac");\
       if (0xFF != index) \
       {\
           key_field->type = CTC_FIELD_KEY_TARGET_MAC;\
           if(is_add)\
           {\
               CTC_CLI_GET_MAC_ADDRESS("arp-target-mac", arp_target_mac_addr, argv[index + 1]);\
                key_field->ext_data = arp_target_mac_addr; \
                if(index+2<argc){\
                CTC_CLI_GET_MAC_ADDRESS("arp-target-mac-mask", arp_target_mac_addr_mask, argv[index + 2]);\
                key_field->ext_mask = arp_target_mac_addr_mask;} \
           }\
        arg;\
       }\
           index = CTC_CLI_GET_ARGC_INDEX("aware-tunnel-info");\
       if (0xFF != index) \
       {\
           if(is_add)\
           {\
           index = CTC_CLI_GET_ARGC_INDEX("vnid");\
           if (0xFF != index) \
           { \
            tunnel_data.type = 1;\
            CTC_CLI_GET_UINT32("vxlan-vni", tunnel_data.vxlan_vni, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("vxlan-vni-mask", tunnel_data_mask.vxlan_vni, argv[index + 2]);\
           } \
           index = CTC_CLI_GET_ARGC_INDEX("grekey");\
         if (0xFF != index) \
           { \
            tunnel_data.type = 2;\
            CTC_CLI_GET_UINT32("gre-key", tunnel_data.gre_key, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("gre-key-mask", tunnel_data_mask.gre_key, argv[index + 2]);\
           } \
            index = CTC_CLI_GET_ARGC_INDEX("radio-mac");\
         if (0xFF != index) \
            { \
                 tunnel_data.type = 3;\
                 CTC_CLI_GET_MAC_ADDRESS("radio-mac", tunnel_data.radio_mac, argv[index + 1]);\
                 CTC_CLI_GET_MAC_ADDRESS("radio-mac-mask", tunnel_data_mask.radio_mac, argv[index + 2]);\
             } \
              index = CTC_CLI_GET_ARGC_INDEX("radio-id");\
         if (0xFF != index) \
        { \
            tunnel_data.type = 3;\
            CTC_CLI_GET_UINT8("radio-id", tunnel_data.radio_id, argv[index + 1]);\
            CTC_CLI_GET_UINT8("radio-id-mask", tunnel_data_mask.radio_id, argv[index + 2]);\
        } \
            index = CTC_CLI_GET_ARGC_INDEX("ctl-pkt");\
        if (0xFF != index) \
        { \
            tunnel_data.type = 3;\
            CTC_CLI_GET_UINT8("wlan-ctl-pkt", tunnel_data.wlan_ctl_pkt, argv[index + 1]);\
            CTC_CLI_GET_UINT8("wlan-ctl-pkt-mask", tunnel_data_mask.wlan_ctl_pkt, argv[index + 2]);\
        } \
        } \
            key_field->type = CTC_FIELD_KEY_AWARE_TUNNEL_INFO;\
            key_field->ext_data = &tunnel_data; \
            key_field->ext_mask = &tunnel_data_mask; \
            arg;\
      }\
         index = CTC_CLI_GET_ARGC_INDEX("stp-state");\
         if (0xFF != index) \
        {\
          key_field->type = CTC_FIELD_KEY_STP_STATE;\
          if(is_add)\
          {\
        CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
        if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
           }\
            arg;\
         }\
         index = CTC_CLI_GET_ARGC_INDEX("arp-mac-da-chk-fail");\
         if (0xFF != index) \
        {\
            key_field->type = CTC_FIELD_KEY_ARP_MAC_DA_CHK_FAIL;\
         if(is_add)\
            {\
                CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                 if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
              }\
              arg;\
         }\
          index = CTC_CLI_GET_ARGC_INDEX("arp-mac-sa-chk-fail");\
          if (0xFF != index) \
          {\
               key_field->type = CTC_FIELD_KEY_ARP_MAC_SA_CHK_FAIL;\
           if(is_add)\
                {\
                    CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                     if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                }\
             arg;\
           }\
           index = CTC_CLI_GET_ARGC_INDEX("arp-senderip-chk-fail");\
           if (0xFF != index) \
           {\
               key_field->type = CTC_FIELD_KEY_ARP_SENDERIP_CHK_FAIL;\
          if(is_add)\
              {\
                CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
               }\
          arg;\
            }\
          index = CTC_CLI_GET_ARGC_INDEX("arp-targetip-chk-fail");\
          if (0xFF != index) \
           {\
                key_field->type = CTC_FIELD_KEY_ARP_TARGETIP_CHK_FAIL;\
           if(is_add)\
                {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                }\
                arg;\
             }\
          index = CTC_CLI_GET_ARGC_INDEX("garp");\
          if (0xFF != index) \
          {\
               key_field->type = CTC_FIELD_KEY_GARP;\
               if(is_add)\
                {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                }\
               arg;\
           }\
          index = CTC_CLI_GET_ARGC_INDEX("is-log-pkt");\
          if (0xFF != index) \
          {\
               key_field->type = CTC_FIELD_KEY_IS_LOG_PKT;\
                if(is_add)\
                {\
                    CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                    if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                }\
               arg;\
           }\
           index = CTC_CLI_GET_ARGC_INDEX("satpdu-pdu-byte");\
           if (0xFF != index) \
           {\
               key_field->type = CTC_FIELD_KEY_SATPDU_PDU_BYTE;\
               if(is_add)\
                 {\
                    uint32 data_num = 0;\
                data_num = sal_sscanf(argv[index+1], "0x%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx", \
                                (uint8*)data8, (uint8*)data8+1, (uint8*)data8+2, (uint8*)data8+3, (uint8*)data8+4, (uint8*)data8+5, (uint8*)data8+6, (uint8*)data8+7);\
                if (data_num != 8) {ctc_cli_out("  %%  Get SAT PDU data fail! \n");\
                    return CLI_ERROR;};\
                data_num = sal_sscanf(argv[index+2], "0x%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",\
                    (uint8*)mask8, (uint8*)mask8+1, (uint8*)mask8+2, (uint8*)mask8+3, (uint8*)mask8+4, (uint8*)mask8+5, (uint8*)mask8+6, (uint8*)mask8+7);\
                if (data_num != 8) {ctc_cli_out("  %%  Get SAT PDU mask fail! \n");\
                    return CLI_ERROR;}\
                      key_field->ext_data = (void*)data8;\
                      key_field->ext_mask = (void*)mask8;\
                 }\
               arg;\
             }\
          index = CTC_CLI_GET_ARGC_INDEX("l2-station-move");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_L2_STATION_MOVE;\
              if(is_add)\
              {\
                 CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                 if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
               }\
              arg;\
          }\
           index = CTC_CLI_GET_ARGC_INDEX("mac-security-discard");\
           if (0xFF != index) \
           {\
               key_field->type = CTC_FIELD_KEY_MAC_SECURITY_DISCARD;\
               if(is_add)\
               {\
                   CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                   if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                }\
                arg;\
            }\
            index = CTC_CLI_GET_ARGC_INDEX("dot1ae-es");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_DOT1AE_ES;\
             if(is_add)\
                  {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                  }\
              arg;\
          }\
          index = CTC_CLI_GET_ARGC_INDEX("dot1ae-sc");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_DOT1AE_SC;\
             if(is_add)\
                  {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                  }\
              arg;\
          }\
          index = CTC_CLI_GET_ARGC_INDEX("dot1ae-sl");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_DOT1AE_SL;\
             if(is_add)\
                  {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                  }\
              arg;\
          }\
          index = CTC_CLI_GET_ARGC_INDEX("dot1ae-pn");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_DOT1AE_PN;\
             if(is_add)\
                  {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                  }\
              arg;\
          }\
          index = CTC_CLI_GET_ARGC_INDEX("dot1ae-an");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_DOT1AE_AN;\
             if(is_add)\
                  {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                  }\
              arg;\
          }\
          index = CTC_CLI_GET_ARGC_INDEX("dot1ae-sci");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_DOT1AE_SCI;\
             if(is_add)\
                  {\
                    uint32 data_num = 0;\
                data_num = sal_sscanf(argv[index+1], "0x%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx", \
                                (uint8*)data8, (uint8*)data8+1, (uint8*)data8+2, (uint8*)data8+3, (uint8*)data8+4, (uint8*)data8+5, (uint8*)data8+6, (uint8*)data8+7);\
                if (data_num != 8) {ctc_cli_out("  %%  Get SCI data fail! \n");\
                    return CLI_ERROR;};\
                data_num = sal_sscanf(argv[index+2], "0x%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",\
                    (uint8*)mask8, (uint8*)mask8+1, (uint8*)mask8+2, (uint8*)mask8+3, (uint8*)mask8+4, (uint8*)mask8+5, (uint8*)mask8+6, (uint8*)mask8+7);\
                if (data_num != 8) {ctc_cli_out("  %%  Get SCI mask fail! \n");\
                    return CLI_ERROR;}\
                      key_field->ext_data = (void*)data8;\
                      key_field->ext_mask = (void*)mask8;\
                 }\
              arg;\
          }\
          index = CTC_CLI_GET_ARGC_INDEX("dot1ae-unknow-pkt");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_DOT1AE_UNKNOW_PKT;\
              if(is_add)\
                  {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                  }\
              arg;\
          }\
          index = CTC_CLI_GET_ARGC_INDEX("dot1ae-cbit");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_DOT1AE_CBIT;\
              if(is_add)\
                  {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                  }\
              arg;\
          }\
          index = CTC_CLI_GET_ARGC_INDEX("dot1ae-ebit");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_DOT1AE_EBIT;\
              if(is_add)\
                  {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                  }\
              arg;\
          }\
          index = CTC_CLI_GET_ARGC_INDEX("dot1ae-scb");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_DOT1AE_SCB;\
              if(is_add)\
                  {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                  }\
              arg;\
          }\
          index = CTC_CLI_GET_ARGC_INDEX("dot1ae-ver");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_DOT1AE_VER;\
              if(is_add)\
                  {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                  }\
              arg;\
          }\
          index = CTC_CLI_GET_ARGC_INDEX("is-mcast-rpf-chk-fail");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_MCAST_RPF_CHECK_FAIL;\
             if(is_add)\
                  {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                  }\
              arg;\
          }\
          index = CTC_CLI_GET_ARGC_INDEX("udf-entry-valid");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_UDF_ENTRY_VALID;\
             if(is_add)\
                  {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                  }\
              arg;\
          }\
          index = CTC_CLI_GET_ARGC_INDEX("is-route-mac");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_IS_ROUTER_MAC;\
             if(is_add)\
                  {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                  }\
              arg;\
          }\
          index = CTC_CLI_GET_ARGC_INDEX("gem-port");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_GEM_PORT;\
             if(is_add)\
                  {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                  }\
              arg;\
          }\
          index = CTC_CLI_GET_ARGC_INDEX("class-id");\
          if (0xFF != index) \
          {\
              key_field->type = CTC_FIELD_KEY_CLASS_ID;\
             if(is_add)\
                  {\
                      CTC_CLI_GET_UINT32("value", key_field->data, argv[index + 1]);\
                      if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field->mask, argv[index + 2]);\
                  }\
              arg;\
          }\
}while(0)


#define CTC_CLI_ACL_ADD_FIELD_ACTION_STR "\
    | deny-bridge \
    | deny-learning \
    | deny-route  \
    | stats STATS-ID \
    | random-log SESSION-ID LOG-PERCENT\
    | color COLOR \
    | dscp DSCP \
    | micro-flow-policer POLICER-ID \
    | macro-flow-policer POLICER-ID \
    | cos-hbwp-policer POLICER-ID COS-INDEX \
    | copp POLICER-ID  \
    | (src-cid|dst-cid) CID \
    | truncate-len LENGTH \
    | redirect NHID \
    | redirect-port GPORT \
    | redirect-cancel-pkt-edit\
    | redirect-filter-routed-pkt \
    | discard {all|green|yellow|red|}\
    | cancel-discard{all|green|yellow|red|} \
    | priority (all |green|red|yellow)NEW-PRIORITY\
    | disable-elephant \
    | terminate-cid-hdr \
    | cancel-nat \
    | inter-cn NEW-INTER-CN \
    | disable-ipfix  \
    | deny-ipfix-learning  \
    | replace-lb-hash LB_HASH mode MODE \
    | cancel-dot1ae \
    | dot1ae-permit-plain-pkt \
    | metadata METADATA \
    | enable-ipfix (cfg-profile-id PRO_ID field-sel SEL_ID hash-type HASH_TYPE (use-mapped-vlan|)(use-cvlan|)(mfp POLICER-ID|){lantency-en VALUE|}) \
    | vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (tpid-index TPID_INDEX|) (scfi-sl TAG_SL (new-scfi CFI|)|)|) \
     (ctag-op TAG_OP  (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)| )  \
    | strip-packet start-to TO_HEADER packet-type PKT_TYPE (extra-len EX_LEN|) \
    | critical-pkt \
    | cancel-acl-tcam-en ACL-PRIORITY \
    | logic-port LOGIC_PORT \
    | oam (eth-oam | ip-bfd |flex|twamp) {dest-gchip GCHIP | lmep-index MEP_INDEX | mip-en ENABLE | link-oam-en ENABLE | packet-offset PACKET_OFFSET | mac-da-check-en|is-self_address|time-stamp-en|timestamp-format FORMAT| } \
    | cancel-all\
    | is-span-packet\
    | copy-to-cpu mode MODE (cpu-reason-id REASON|)  \
    | table-map-id TABLE_MAP_ID (cos2cos |cos2dscp | dscp2cos | dscp2dscp|) \
    | disable-ecmp-rr \
    | disable-linkagg-rr \
    | lb-hash-ecmp-profile PROFILE_ID \
    | lb-hash-lag-profile PROFILE_ID \
    | vxlan-rsv-edit MODE {flags VALUE| reserved1 VALUE| reserved2 VALUE|} \
    | timestamp MODE \
    | reflective-bridge-en VALUE \
    | port-isolation-dis VALUE \
    "

#define CTC_CLI_ACL_ADD_FIELD_ACTION_DESC \
    "Deny bridge ", \
    "Deny learning ", \
    "Deny route ", \
    "Packet statistics ","Stats id" , \
    "Random log to cpu ","Session id" , "Log percent",\
    "Change color ","Color(ctc_qos_color_t)" , \
    "Dscp ","Dscp" , \
    "Micor flow policer ","Micro policer id" , \
    "Macor flow policer ","Macro policer id" , \
    "Cos hbwp policer ", "Hbwp policer id" , "Cos index" , \
    CTC_CLI_QOS_COPP, "Policer id", \
    "Source category id","Destination category id","Category id" ,\
    "Enable packet truncation", "Truncate length", \
    "Redirect packet ","Nexthop id" , \
    "Redirect packet to single port ", CTC_CLI_GPORT_DESC,\
    "Packet edit will be canceled for redirect packet",\
    "Filter Routed packet for redirect operation",\
    "Discard","Discard packet of all color ","Discard green packet","Discard yellow packet","Discard red packet",\
    "Cancel discard","Cancel discard packet of all color ", "Cancel discard green packet ", "Cancel discard yellow packet ","Cancel discard red packet ", \
    "New priority","All packet ","Green packet ","Yellow packet ","Red packet ","New priority" , \
    "Disable elephant flow ", \
    "Insert cid header to packet ", \
    "Cancel nat ", \
    "Internal CN ","Internal CN value", \
    "Disable ipfix ", \
    "Disable ipfix learning ", \
    "Replace loadbalance hash value ","Hash value" ,\
    "Replace loadbalance mode ",\
    "Mode:0- linkagg hash;1- ecmp hash;2- linkagg and ecmp hash use same value", \
    "Cancel dot1ae encrypt", \
    "Dot1ae permit plain packet", \
    "Metedata ","metadata value" , \
    "Enable ipfix", "Profile id", "PROFILE ID", "Hash field select id", "SELECT ID", "Hash type", \
    "HASH TYPE", "Use mapped vlan","Use cvlan", "Mic flow policer", "Policer ID",  "Enable lantency", "value", \
    "Vlan edit", \
    "Stag operation type", CTC_CLI_ACL_VLAN_TAG_OP_DESC, "Svid select", CTC_CLI_ACL_VLAN_TAG_SL_DESC, \
    "New stag vlan id", CTC_CLI_VLAN_RANGE_DESC, "Scos select", CTC_CLI_ACL_VLAN_TAG_SCOS_SL_DESC,  \
    "New stag cos", CTC_CLI_COS_RANGE_DESC, "Scfi select",  CTC_CLI_ACL_VLAN_TAG_SL_DESC, \
    "Set svlan tpid index", "Index, the corresponding svlan tpid is in EpeL2TpidCtl",\
    "New stag cfi", "0-1", \
    "Ctag operation type", CTC_CLI_ACL_VLAN_TAG_OP_DESC, "Cvid select", CTC_CLI_ACL_VLAN_TAG_SL_DESC, \
    "New ctag vlan id",  CTC_CLI_VLAN_RANGE_DESC, "Ccos select", CTC_CLI_ACL_VLAN_TAG_SL_DESC, \
    "New ctag cos", CTC_CLI_COS_RANGE_DESC,  "Ccfi select",  CTC_CLI_ACL_VLAN_TAG_SL_DESC,  \
    "New ctag cfi", "0-1",   \
    "Strip packet", "Start to L2/L3/L4 header", "0:None, 1:L2, 2:L3, 3:L4", "Payload packet type", \
    "0:eth, 1:ipv4, 2:mpls, 3:ipv6",  "Extra-length",  "Unit: byte",\
    "Critical Packet",\
    "Cancel ACL Tcam Lookup"," Data0: ACL Priority",\
    "Logical port", "value",\
    "Enable OAM", "Oam type is Ether Oam ","Oam type is IP BFD ","Oam type is FLEX ","Oam type is TWAMP","Global Chip Id","Value","Local Mep index","Value","MIP enable","Value","Link Oam or Section Oam","Value",\
    "Only used for bfd","Value","Mac da check enable", "Is-self-address","Time-stamp-enable","Timestamp format", "Format",\
    "Cancel all action",\
    "Is span packet",\
    "Copy to cpu","Mode","Value: 0 -not cover 1 -cover 2 -cancel copy to cpu","cpu reason id","value", \
    CTC_CLI_QOS_TABLE_MAP_STR, CTC_CLI_QOS_TABLE_MAP_VALUE, "Cos2cos","Cos2dscp","Dscp2cos","Dscp2dscp", \
    "Disable ecmp rr mode", \
    "Disable linkagg rr mode", \
    "Load hash select profile id for ecmp", "Profile id", \
    "Load hash select profile id for linkagg", "Profile id", \
    "Vxlan reserved data edit mode", "0:None 1:Merge 2.Clear 3.Replace", \
    "Flags", "Value", \
    "Reserved1", "Value", \
    "Reserved2", "Value", \
    "Timestamp Mode", "Value", \
    "Reflective Bridge", "Value", \
    "Port Isolation Disable", "Value"

#define CTC_CLI_ACL_ADD_FIELD_ACTION_SET(act_field, arg) \
do{\
    index = CTC_CLI_GET_ARGC_INDEX("deny-bridge");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DENY_BRIDGE;\
        act_field->data0 = 1;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("deny-learning");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DENY_LEARNING;\
        act_field->data0 = 1;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("deny-route");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DENY_ROUTE;\
        act_field->data0 = 1;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stats");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_STATS;\
        CTC_CLI_GET_UINT32("value", act_field->data0, argv[index + 1]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("random-log");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_RANDOM_LOG;\
        CTC_CLI_GET_UINT32("session-id", act_field->data0, argv[index + 1]);\
        CTC_CLI_GET_UINT32("log-percent", act_field->data1, argv[index + 2]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("color");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_COLOR;\
        CTC_CLI_GET_UINT32("value", act_field->data0, argv[index + 1]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("dscp");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DSCP;\
        CTC_CLI_GET_UINT32("value", act_field->data0, argv[index + 1]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("micro-flow-policer");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER;\
        CTC_CLI_GET_UINT32("value", act_field->data0, argv[index + 1]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("macro-flow-policer");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER;\
        CTC_CLI_GET_UINT32("value", act_field->data0, argv[index + 1]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cos-hbwp-policer");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_COS_HBWP_POLICER;\
        CTC_CLI_GET_UINT32("value", act_field->data0, argv[index + 1]);\
        CTC_CLI_GET_UINT32("value", act_field->data1, argv[index + 2]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("copp");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_COPP;\
        CTC_CLI_GET_UINT32("value", act_field->data0, argv[index + 1]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("src-cid");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_SRC_CID;\
        CTC_CLI_GET_UINT32("value", act_field->data0, argv[index + 1]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DEST_CID;\
        CTC_CLI_GET_UINT32("value", act_field->data0, argv[index + 1]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("truncate-len");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_TRUNCATED_LEN;\
        CTC_CLI_GET_UINT32("value", act_field->data0, argv[index + 1]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("redirect");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_REDIRECT;\
        CTC_CLI_GET_UINT32("value", act_field->data0, argv[index + 1]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("redirect-port");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_REDIRECT_PORT;\
        CTC_CLI_GET_UINT32("value", act_field->data0, argv[index + 1]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("redirect-cancel-pkt-edit");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_REDIRECT_CANCEL_PKT_EDIT;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("redirect-filter-routed-pkt");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_REDIRECT_FILTER_ROUTED_PKT;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("discard");\
    if (0xFF != index) \
    {\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("green"))\
        {\
            CTC_BIT_SET(act_field->data0, CTC_QOS_COLOR_GREEN);\
        }\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("red"))\
        {\
            CTC_BIT_SET(act_field->data0, CTC_QOS_COLOR_RED);\
        }\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("yellow"))\
        {\
            CTC_BIT_SET(act_field->data0, CTC_QOS_COLOR_YELLOW);\
        }\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("all"))\
        {\
            act_field->data0 = CTC_QOS_COLOR_NONE;\
        }\
        act_field->type = CTC_ACL_FIELD_ACTION_DISCARD;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cancel-discard");\
    if (0xFF != index) \
    {\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("green"))\
        {\
            CTC_BIT_SET(act_field->data0, CTC_QOS_COLOR_GREEN);\
        }\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("red"))\
        {\
            CTC_BIT_SET(act_field->data0, CTC_QOS_COLOR_RED);\
        }\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("yellow"))\
        {\
            CTC_BIT_SET(act_field->data0, CTC_QOS_COLOR_YELLOW);\
        }\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("all"))\
        {\
            act_field->data0 = CTC_QOS_COLOR_NONE;\
        }\
        act_field->type = CTC_ACL_FIELD_ACTION_CANCEL_DISCARD;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("priority");\
    if (0xFF != index) \
    {\
    if(0xFF != CTC_CLI_GET_ARGC_INDEX("all"))\
    	{\
        act_field->type = CTC_ACL_FIELD_ACTION_PRIORITY;\
        act_field->data0 = CTC_QOS_COLOR_NONE;\
        CTC_CLI_GET_UINT32("value", act_field->data1, argv[index + 2]);\
        arg;\
         }\
    else if(0xFF != CTC_CLI_GET_ARGC_INDEX("green"))\
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_PRIORITY;\
        act_field->data0 = CTC_QOS_COLOR_GREEN;\
        CTC_CLI_GET_UINT32("value", act_field->data1, argv[index + 2]);\
        arg;\
    }\
   else if(0xFF != CTC_CLI_GET_ARGC_INDEX("red"))\
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_PRIORITY;\
        act_field->data0 = CTC_QOS_COLOR_RED;\
        CTC_CLI_GET_UINT32("value", act_field->data1, argv[index + 2]);\
        arg;\
    }\
   else if(0xFF != CTC_CLI_GET_ARGC_INDEX("yellow"))\
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_PRIORITY;\
        act_field->data0 = CTC_QOS_COLOR_YELLOW;\
        CTC_CLI_GET_UINT32("value", act_field->data1, argv[index + 2]);\
        arg;\
    }\
	}\
    index = CTC_CLI_GET_ARGC_INDEX("disable-elephant");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DISABLE_ELEPHANT_LOG;\
        act_field->data0 = 1;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("terminate-cid-hdr");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_TERMINATE_CID_HDR;\
        act_field->data0 = 1;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cancel-nat");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_CANCEL_NAT;\
        act_field->data0 = 1;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("inter-cn");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_INTER_CN;\
        CTC_CLI_GET_UINT8("inter-cn", inter_cn.inter_cn, argv[index + 1]);\
        act_field->ext_data = &inter_cn;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("disable-ipfix");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_CANCEL_IPFIX;\
        act_field->data0 = 1;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("deny-ipfix-learning");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_CANCEL_IPFIX_LEARNING;\
        act_field->data0 = 1;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("replace-lb-hash");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_REPLACE_LB_HASH;\
        act_field->ext_data = &lb_hash;\
        CTC_CLI_GET_UINT8("lb_hash", lb_hash.lb_hash, argv[index + 1]);\
        CTC_CLI_GET_UINT8("mode", lb_hash.mode, argv[index + 3]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cancel-dot1ae");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_CANCEL_DOT1AE;\
        act_field->data0 = 1;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("dot1ae-permit-plain-pkt");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DOT1AE_PERMIT_PLAIN_PKT;\
        act_field->data0 = 1;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("metadata");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_METADATA;\
        CTC_CLI_GET_UINT32("value", act_field->data0, argv[index + 1]);\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("enable-ipfix"); \
    if (0xFF != index)  \
    {  \
        index = CTC_CLI_GET_ARGC_INDEX("cfg-profile-id");\
        if (0xFF != index) \
        {   \
            CTC_CLI_GET_UINT8("cfg-profile-id", ipfix.flow_cfg_id, argv[index + 1]);         \
        }   \
        index = CTC_CLI_GET_ARGC_INDEX("field-sel");\
        if (0xFF != index) \
        {   \
            CTC_CLI_GET_UINT8("field-sel", ipfix.field_sel_id, argv[index + 1]);         \
        }   \
        index = CTC_CLI_GET_ARGC_INDEX("hash-type");\
        if (0xFF != index) \
        {   \
            CTC_CLI_GET_UINT8("hash-type", ipfix.hash_type, argv[index + 1]);         \
        }   \
        index = CTC_CLI_GET_ARGC_INDEX("use-mapped-vlan");\
        if (0xFF != index) \
        {   \
            ipfix.use_mapped_vlan = 1;    \
        }   \
        index = CTC_CLI_GET_ARGC_INDEX("use-cvlan");\
        if (0xFF != index) \
        {   \
            ipfix.use_cvlan = 1;    \
        }   \
        index = CTC_CLI_GET_ARGC_INDEX("mfp");\
        if (0xFF != index) \
        {   \
            CTC_CLI_GET_UINT32("mfp", ipfix.policer_id, argv[index + 1]);         \
        }   \
        index = CTC_CLI_GET_ARGC_INDEX("lantency-en");\
        if (0xFF != index) \
        {   \
            CTC_CLI_GET_UINT8("lantency-en", ipfix.lantency_en, argv[index + 1]);         \
        }   \
        act_field->type = CTC_ACL_FIELD_ACTION_IPFIX;\
        act_field->ext_data = &ipfix;\
        arg;  \
    }  \
    index = CTC_CLI_GET_ARGC_INDEX("vlan-edit");                                            \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("stag-op");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("stag-op", vlan_edit.stag_op, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("svid-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("svid-sl", vlan_edit.svid_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-svid");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT16("new-svid", vlan_edit.svid_new, argv[index + 1]);      \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("scos-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("scos-sl", vlan_edit.scos_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-scos");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("new-scos", vlan_edit.scos_new, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("scfi-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("scfi-sl", vlan_edit.scfi_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-scfi");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("new-scfi", vlan_edit.scfi_new, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("ctag-op");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("ctag-op", vlan_edit.ctag_op, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("cvid-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("cvid-sl", vlan_edit.cvid_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-cvid");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT16("new-cvid", vlan_edit.cvid_new, argv[index + 1]);      \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("ccos-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("ccos-sl", vlan_edit.ccos_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-ccos");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("new-ccos", vlan_edit.ccos_new, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("ccfi-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("ccfi-sl", vlan_edit.ccfi_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-ccfi");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("new-ccfi", vlan_edit.ccfi_new, argv[index + 1]);         \
        }                                                                                        \
        index = CTC_CLI_GET_ARGC_INDEX("tpid-index");                                            \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("tpid-index", vlan_edit.tpid_index, argv[index + 1]);         \
        }                                                                                   \
        act_field->type = CTC_ACL_FIELD_ACTION_VLAN_EDIT;\
        act_field->ext_data = &vlan_edit;\
        arg;  \
    }\
    index = CTC_CLI_GET_ARGC_INDEX("strip-packet");\
    if (0xFF != index) \
    {\
        index = CTC_CLI_GET_ARGC_INDEX("start-to");                               \
        if (INDEX_VALID(index))                                                             \
        {                                                                                   \
            CTC_CLI_GET_UINT8("start-to", packet_strip.start_packet_strip, argv[index + 1]);     \
        }                                                                               \
        index = CTC_CLI_GET_ARGC_INDEX("extra-len");                               \
        if (INDEX_VALID(index))                                                             \
        {                                                                                   \
            CTC_CLI_GET_UINT8("extra-len", packet_strip.strip_extra_len, argv[index + 1]);     \
        }                                                                               \
        index = CTC_CLI_GET_ARGC_INDEX("packet-type");                               \
        if (INDEX_VALID(index))                                                             \
        {                                                                                   \
            CTC_CLI_GET_UINT8("packet-type", packet_strip.packet_type, argv[index + 1]);     \
        }                                                                               \
        act_field->type = CTC_ACL_FIELD_ACTION_STRIP_PACKET;\
        act_field->ext_data = &packet_strip;\
        arg;\
    }\
         index = CTC_CLI_GET_ARGC_INDEX("critical-pkt");\
         if (0xFF != index) \
        {\
            act_field->type = CTC_ACL_FIELD_ACTION_CRITICAL_PKT;\
            arg;\
         }\
          index = CTC_CLI_GET_ARGC_INDEX("cancel-acl-tcam-en");\
          if (0xFF != index) \
          {\
              act_field->type = CTC_ACL_FIELD_ACTION_CANCEL_ACL_TCAM_EN;\
              CTC_CLI_GET_UINT32("value", act_field->data0, argv[index + 1]);\
              arg;\
           }\
           index = CTC_CLI_GET_ARGC_INDEX("logic-port");\
           if (0xFF != index) \
          {\
              act_field->type = CTC_ACL_FIELD_ACTION_LOGIC_PORT;\
               CTC_CLI_GET_UINT32("value", act_field->data0, argv[index + 1]);\
               arg;\
           }\
    index = CTC_CLI_GET_ARGC_INDEX("oam");\
    if (0xFF != index) \
    {   \
        act_field->type = CTC_ACL_FIELD_ACTION_OAM;\
        index = CTC_CLI_GET_ARGC_INDEX("eth-oam");\
        if (0xFF != index) \
        { \
            oam_t.oam_type = CTC_ACL_OAM_TYPE_ETH_OAM;\
        } \
        index = CTC_CLI_GET_ARGC_INDEX("ip-bfd");\
        if (0xFF != index) \
        { \
            oam_t.oam_type = CTC_ACL_OAM_TYPE_IP_BFD;\
        } \
        index = CTC_CLI_GET_ARGC_INDEX("flex");\
        if (0xFF != index) \
        { \
            oam_t.oam_type = CTC_ACL_OAM_TYPE_FLEX;\
        } \
        index = CTC_CLI_GET_ARGC_INDEX("twamp");\
        if (0xFF != index) \
        { \
            oam_t.oam_type = CTC_ACL_OAM_TYPE_TWAMP;\
        } \
        index = CTC_CLI_GET_ARGC_INDEX("dest-gchip");\
        if (0xFF != index) \
        {   \
            CTC_CLI_GET_UINT8("dest-gchip", oam_t.dest_gchip, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("lmep-index");\
        if (0xFF != index) \
        {   \
            CTC_CLI_GET_UINT16("lmep-index", oam_t.lmep_index, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("mip-en");\
        if (0xFF != index) \
        {   \
            CTC_CLI_GET_UINT8("mip-en", oam_t.mip_en, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("mac-da-check-en");\
        if (0xFF != index) \
        {   \
            oam_t.mac_da_check_en = 1;\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("is-self_address");\
        if (0xFF != index) \
        {   \
            oam_t.is_self_address = 1;\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("time-stamp-en");\
        if (0xFF != index) \
        {   \
            oam_t.time_stamp_en = 1;\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("timestamp-format");\
        if (0xFF != index) \
        {   \
            CTC_CLI_GET_UINT8("timestamp format", oam_t.timestamp_format, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("link-oam-en");\
        if (0xFF != index) \
        {   \
            CTC_CLI_GET_UINT8("link-oam-en", oam_t.link_oam_en, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("packet-offset");\
        if (0xFF != index) \
        {   \
            CTC_CLI_GET_UINT8("packet-offset",oam_t.packet_offset, argv[index + 1]);\
        }\
        act_field->ext_data = &oam_t;\
        arg;\
     }  \
     index = CTC_CLI_GET_ARGC_INDEX("cancel-all");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_CANCEL_ALL;\
        act_field->data0 = 1;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("is-span-packet");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_SPAN_FLOW;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("copy-to-cpu");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_CP_TO_CPU;\
        index = CTC_CLI_GET_ARGC_INDEX("mode");\
        if (0xFF != index) \
        {\
            CTC_CLI_GET_UINT8("mode",cp_to_cpu.mode, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("cpu-reason-id");\
        if (0xFF != index) \
        {\
            CTC_CLI_GET_UINT16("cpu-reason-id",cp_to_cpu.cpu_reason_id, argv[index + 1]);\
        }\
        act_field->ext_data = &cp_to_cpu;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("table-map-id");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_QOS_TABLE_MAP;\
        CTC_CLI_GET_UINT8("table-map-id", table_map.table_map_id, argv[index + 1]);\
        index = CTC_CLI_GET_ARGC_INDEX("cos2cos");\
        if (0xFF != index)\
        {\
            table_map.table_map_type = CTC_QOS_TABLE_MAP_IGS_COS_TO_COS;\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("cos2dscp");\
        if (0xFF != index)\
        {\
            table_map.table_map_type = CTC_QOS_TABLE_MAP_IGS_COS_TO_DSCP;\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("dscp2cos");\
        if (0xFF != index)\
        {\
            table_map.table_map_type = CTC_QOS_TABLE_MAP_IGS_DSCP_TO_COS;\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("dscp2dscp");\
        if (0xFF != index)\
        {\
            table_map.table_map_type = CTC_QOS_TABLE_MAP_IGS_DSCP_TO_DSCP;\
        }\
        act_field->ext_data = &table_map;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("disable-ecmp-rr");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DISABLE_ECMP_RR;\
        arg;\
    } \
    index = CTC_CLI_GET_ARGC_INDEX("disable-linkagg-rr");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DISABLE_LINKAGG_RR;\
        arg;\
    } \
    index = CTC_CLI_GET_ARGC_INDEX("lb-hash-ecmp-profile");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_LB_HASH_ECMP_PROFILE;\
        CTC_CLI_GET_UINT8("lb-hash-ecmp-profile", act_field->data0, argv[index + 1]);\
        arg;\
    } \
    index = CTC_CLI_GET_ARGC_INDEX("lb-hash-lag-profile");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_LB_HASH_LAG_PROFILE;\
        CTC_CLI_GET_UINT8("lb-hash-lag-profile", act_field->data0, argv[index + 1]);\
        arg;\
    } \
    index = CTC_CLI_GET_ARGC_INDEX("vxlan-rsv-edit");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_VXLAN_RSV_EDIT;\
        CTC_CLI_GET_UINT8("vxlan-rsv-edit", vxlan_rsv_edit.edit_mode, argv[index + 1]);\
        index = CTC_CLI_GET_ARGC_INDEX("flags");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("flags", vxlan_rsv_edit.vxlan_flags, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("reserved1");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT32("reserved1", vxlan_rsv_edit.vxlan_rsv1, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("reserved2");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("reserved2", vxlan_rsv_edit.vxlan_rsv2, argv[index + 1]);\
        }\
        act_field->ext_data = (void*)&vxlan_rsv_edit;\
        arg;\
    } \
    index = CTC_CLI_GET_ARGC_INDEX("timestamp");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_TIMESTAMP;\
        CTC_CLI_GET_UINT8("value", timestamp.mode, argv[index + 1]);\
        act_field->ext_data = (void*)&timestamp; \
        arg;\
    } \
    index = CTC_CLI_GET_ARGC_INDEX("reflective-bridge-en"); \
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_REFLECTIVE_BRIDGE_EN;\
        CTC_CLI_GET_UINT8("value", act_field->data0, argv[index + 1]);\
        arg;\
    } \
    index = CTC_CLI_GET_ARGC_INDEX("port-isolation-dis"); \
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_PORT_ISOLATION_DIS;\
        CTC_CLI_GET_UINT8("value", act_field->data0, argv[index + 1]);\
        arg;\
    } \
	}while(0)

#define CTC_CLI_ACL_REMOVE_FIELD_ACTION_STR "\
      deny-bridge \
    | deny-learning \
    | deny-route  \
    | stats  \
    | random-log \
    | color  \
    | dscp  \
    | micro-flow-policer \
    | macro-flow-policer \
    | cos-hbwp-policer \
    | copp \
    | (src-cid|dst-cid) \
    | truncate-len  \
    | redirect  \
    | redirect-port  \
    | redirect-cancel-pkt-edit\
    | redirect-filter-routed-pkt \
    | discard (all|green|yellow|red|)\
    | cancel-discard(all|green|yellow|red|) \
    | priority (all |green|red|yellow) \
    | disable-elephant \
    | terminate-cid-hdr \
    | cancel-nat \
    | inter-cn \
    | disable-ipfix  \
    | deny-ipfix-learning  \
    | replace-lb-hash  \
    | cancel-dot1ae \
    | dot1ae-permit-plain-pkt \
    | metadata  \
    | enable-ipfix \
    | vlan-edit  \
    | strip-packet \
    | critical-pkt \
    | cancel-acl-tcam-en \
    | logic-port \
    | oam \
    | cancel-all\
    | is-span-packet\
    | copy-to-cpu \
    | table-map-id \
    | disable-ecmp-rr \
    | disable-linkagg-rr \
    | lb-hash-ecmp-profile \
    | lb-hash-lag-profile \
    | vxlan-rsv-edit \
    | timestamp \
    | reflective-bridge-en \
    | port-isolation-dis \
    "

#define CTC_CLI_ACL_REMOVE_FIELD_ACTION_DESC \
    "Deny bridge ", \
    "Deny learning ", \
    "Deny route ", \
    "Packet statistics ", \
    "Random log to cpu ",\
    "Change color ", \
    "Dscp ", \
    "Micor flow policer ", \
    "Macor flow policer ", \
    "Cos hbwp policer", \
    CTC_CLI_QOS_COPP,  \
    "Source category id","Destination category id",\
    "Enable packet truncation", \
    "Redirect packet ", \
    "Redirect packet to single port ",\
    "Packet edit will be canceled for redirect packet",\
    "Filter Routed packet for redirect operation",\
    "Discard","Discard packet of all color ","Discard green packet","Discard yellow packet","Discard red packet",\
    "Cancel discard","Cancel discard packet of all color ", "Cancel discard green packet ", "Cancel discard yellow packet ","Cancel discard red packet ", \
    "New priority","All packet ","Green packet ","Yellow packet ","Red packet ", \
    "Disable elephant flow ", \
    "Terminate cid header to packet", \
    "Cancel nat ", \
    "Internal CN ", \
    "Disable ipfix ", \
    "Disable ipfix learning ", \
    "Replace loadbalance hash value ",\
    "Cancel dot1ae encrypt", \
    "Dot1ae permit plain packet", \
    "Metedata ", \
    "Enable ipfix",\
    "Vlan edit", \
    "Strip packet", \
    "Critical Packet",\
    "Cancel ACL Tcam Lookup",\
    "Logical port", \
    "Enable OAM",\
    "Cancel all action",\
    "Is span packet",\
    "Copy to cpu",\
    CTC_CLI_QOS_TABLE_MAP_STR, \
    "Disable ecmp rr mode", \
    "Disable linkagg rr mode", \
    "Load hash select profile id for ecmp", \
    "Load hash select profile id for linkagg", \
    "Vxlan reserved edit mode", \
    "Timestamp Mode", \
    "Reflective Bridge", \
    "Port Isolation Disable"


#define CTC_CLI_ACL_REMOVE_FIELD_ACTION_SET(act_field, arg) \
do{\
    index = CTC_CLI_GET_ARGC_INDEX("deny-bridge");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DENY_BRIDGE;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("deny-learning");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DENY_LEARNING;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("deny-route");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DENY_ROUTE;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stats");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_STATS;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("random-log");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_RANDOM_LOG;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("color");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_COLOR;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("dscp");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DSCP;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("micro-flow-policer");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("macro-flow-policer");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cos-hbwp-policer");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_COS_HBWP_POLICER;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("copp");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_COPP;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("src-cid");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_SRC_CID;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DEST_CID;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("truncate-len");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_TRUNCATED_LEN;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("redirect");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_REDIRECT;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("redirect-port");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_REDIRECT_PORT;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("redirect-cancel-pkt-edit");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_REDIRECT_CANCEL_PKT_EDIT;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("redirect-filter-routed-pkt");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_REDIRECT_FILTER_ROUTED_PKT;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("discard");\
    if (0xFF != index) \
    {\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("green"))\
        {\
            CTC_BIT_SET(act_field->data0, CTC_QOS_COLOR_GREEN);\
        }\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("red"))\
        {\
            CTC_BIT_SET(act_field->data0, CTC_QOS_COLOR_RED);\
        }\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("yellow"))\
        {\
            CTC_BIT_SET(act_field->data0, CTC_QOS_COLOR_YELLOW);\
        }\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("all"))\
        {\
            act_field->data0 = CTC_QOS_COLOR_NONE;\
        }\
        act_field->type = CTC_ACL_FIELD_ACTION_DISCARD;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cancel-discard");\
    if (0xFF != index) \
    {\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("green"))\
        {\
            CTC_BIT_SET(act_field->data0, CTC_QOS_COLOR_GREEN);\
        }\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("red"))\
        {\
            CTC_BIT_SET(act_field->data0, CTC_QOS_COLOR_RED);\
        }\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("yellow"))\
        {\
            CTC_BIT_SET(act_field->data0, CTC_QOS_COLOR_YELLOW);\
        }\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("all"))\
        {\
            act_field->data0 = CTC_QOS_COLOR_NONE;\
        }\
        act_field->type = CTC_ACL_FIELD_ACTION_CANCEL_DISCARD;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("priority");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_PRIORITY;\
        if(0xFF != CTC_CLI_GET_ARGC_INDEX("all"))\
    	{\
            act_field->data0 = CTC_QOS_COLOR_NONE;\
         }\
        else if(0xFF != CTC_CLI_GET_ARGC_INDEX("green"))\
        {\
            act_field->data0 = CTC_QOS_COLOR_GREEN;\
        }\
       else if(0xFF != CTC_CLI_GET_ARGC_INDEX("red"))\
        {\
            act_field->data0 = CTC_QOS_COLOR_RED;\
        }\
       else if(0xFF != CTC_CLI_GET_ARGC_INDEX("yellow"))\
        {\
            act_field->data0 = CTC_QOS_COLOR_YELLOW;\
        }\
        arg;\
	}\
    index = CTC_CLI_GET_ARGC_INDEX("disable-elephant");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DISABLE_ELEPHANT_LOG;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("terminate-cid-hdr");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_TERMINATE_CID_HDR;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cancel-nat");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_CANCEL_NAT;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("inter-cn");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_INTER_CN;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("disable-ipfix");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_CANCEL_IPFIX;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("deny-ipfix-learning");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_CANCEL_IPFIX_LEARNING;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("replace-lb-hash");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_REPLACE_LB_HASH;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cancel-dot1ae");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_CANCEL_DOT1AE;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("dot1ae-permit-plain-pkt");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DOT1AE_PERMIT_PLAIN_PKT;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("metadata");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_METADATA;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("enable-ipfix"); \
    if (0xFF != index)  \
    {  \
        act_field->type = CTC_ACL_FIELD_ACTION_IPFIX;\
        arg;  \
    }  \
    index = CTC_CLI_GET_ARGC_INDEX("vlan-edit"); \
    if (INDEX_VALID(index))   \
    {  \
        act_field->type = CTC_ACL_FIELD_ACTION_VLAN_EDIT;\
        arg;  \
    }\
    index = CTC_CLI_GET_ARGC_INDEX("strip-packet");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_STRIP_PACKET;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("critical-pkt");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_CRITICAL_PKT;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cancel-acl-tcam-en");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_CANCEL_ACL_TCAM_EN;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("logic-port");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_LOGIC_PORT;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("oam");\
    if (0xFF != index) \
    {   \
        act_field->type = CTC_ACL_FIELD_ACTION_OAM;\
        arg;\
    }  \
    index = CTC_CLI_GET_ARGC_INDEX("cancel-all");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_CANCEL_ALL;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("is-span-packet");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_SPAN_FLOW;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("copy-to-cpu");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_CP_TO_CPU;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("table-map-id");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_QOS_TABLE_MAP;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("disable-ecmp-rr");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DISABLE_ECMP_RR;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("disable-linkagg-rr");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_DISABLE_LINKAGG_RR;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("lb-hash-ecmp-profile");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_LB_HASH_ECMP_PROFILE;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("lb-hash-lag-profile");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_LB_HASH_LAG_PROFILE;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vxlan-rsv-edit");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_VXLAN_RSV_EDIT;\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("timestamp");\
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_TIMESTAMP;\
        arg;\
    } \
    index = CTC_CLI_GET_ARGC_INDEX("reflective-bridge-en"); \
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_REFLECTIVE_BRIDGE_EN;\
        arg;\
    } \
    index = CTC_CLI_GET_ARGC_INDEX("port-isolation-dis"); \
    if (0xFF != index) \
    {\
        act_field->type = CTC_ACL_FIELD_ACTION_PORT_ISOLATION_DIS;\
        arg;\
    } \
}while(0);

#define CTC_CLI_ACL_HASH_KEY_TYPE_SET \
    index = CTC_CLI_GET_ARGC_INDEX("mac");\
    if (0xFF != index)\
    {\
        key_type = CTC_ACL_KEY_HASH_MAC;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv4");\
    if (0xFF != index)\
    {\
        key_type = CTC_ACL_KEY_HASH_IPV4;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6");\
    if (0xFF != index)\
    {\
        key_type = CTC_ACL_KEY_HASH_IPV6;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l2l3");\
    if (0xFF != index)\
    {\
        key_type = CTC_ACL_KEY_HASH_L2_L3;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls");\
    if (0xFF != index)\
    {\
        key_type = CTC_ACL_KEY_HASH_MPLS;\
    }

CTC_CLI(ctc_cli_usw_acl_show_entry_info,
        ctc_cli_usw_acl_show_entry_info_cmd,
        "show acl entry-info "\
    "(all (divide-by-group|) | entry ENTRY_ID |group GROUP_ID |priority PRIORITY |lookup-level VALUE |)" \
    "(type (mac|ipv4|ipv6|cid|intf|fwd|udf)|)"\
    "(grep {"CTC_CLI_ACL_KEY_FIELD_TYPE_GREP_STR"} |)(detail|)(lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_ACL_STR,
        "Entry info",
        "All entries",
        "All entrise divide by group",
        "By entry",
        CTC_CLI_ACL_ENTRY_ID_VALUE,
        "By group",
        CTC_CLI_ACL_B_GROUP_ID_VALUE,
        "By group priority",
        CTC_CLI_ACL_GROUP_PRIO_VALUE,
        "By lookup level",
        "Lookup level",
        "By type",
        "ACL Mac-entry",
        "ACL Ipv4-entry",
        "ACL Ipv6-entry",
        "ACL Cid-entry",
        "ACL Intf-entry",
        "ACL Fwd-entry",
        "ACL UDF-entry",
        "Grep",
        CTC_CLI_ACL_KEY_FIELD_TYPE_GREP_DESC,
        "Detail",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    uint8 type = 0;
    uint8 detail = 0;
    uint8 grep_cnt = 0;
    uint8 grep_i = 0;
    uint8 is_add = 1;
    uint8* p_null = NULL;
     /*uint8 scl_id = 0;*/
    uint32 param = 0;
    ctc_acl_key_type_t key_type = CTC_ACL_KEY_NUM;
    ctc_field_key_t grep_key[CTC_CLI_ACL_KEY_FIELED_GREP_NUM];
    ctc_field_port_t port_info;
    ctc_field_port_t port_info_mask;
     /*ctc_port_bitmap_t pbmp = {0};*/
    uint32 data64[2]={0};
    uint32 mask64[2]={0};
    ctc_acl_tunnel_data_t tunnel_data;
    ctc_acl_tunnel_data_t tunnel_data_mask;
    ctc_acl_udf_t udf_data;
    ctc_acl_udf_t udf_data_mask;
     /*CTC_CLI_ACL_COMMON_VAR;*/
    mac_addr_t  mac_da_addr = {0};
    mac_addr_t  mac_da_addr_mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    mac_addr_t  mac_sa_addr = {0};
    mac_addr_t  mac_sa_addr_mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    ipv6_addr_t ipv6_da_addr = {0};
    ipv6_addr_t ipv6_da_addr_mask = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    ipv6_addr_t ipv6_sa_addr = {0};
    ipv6_addr_t ipv6_sa_addr_mask = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};


    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_info_mask, 0xFF, sizeof(ctc_field_port_t));
    sal_memset(&tunnel_data, 0, sizeof(ctc_acl_tunnel_data_t));
    sal_memset(&tunnel_data_mask, 0xFF, sizeof(ctc_acl_tunnel_data_t));
    sal_memset(&udf_data, 0, sizeof(udf_data));
    sal_memset(&udf_data_mask, 0xFF, sizeof(udf_data_mask));
    sal_memset(data64, 0, sizeof(data64));
    sal_memset(mask64, 0xFF, sizeof(mask64));

    sal_memset(grep_key, 0, sizeof(grep_key));
    for (grep_i = 0; grep_i < CTC_CLI_ACL_KEY_FIELED_GREP_NUM; grep_i++)
    {
        grep_key[grep_i].mask = 0xFFFFFFFF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (INDEX_VALID(index))
    {
        type = 0;

        index = CTC_CLI_GET_ARGC_INDEX("divide-by-group");
        if (INDEX_VALID(index))
        { /*0 = by priority. 1 = by group*/
            param = 1;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("entry");
    if (INDEX_VALID(index))
    {
        type = 1;
        CTC_CLI_GET_UINT32("entry id", param, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("group");
    if (INDEX_VALID(index))
    {
        type = 2;
        CTC_CLI_GET_UINT32("group id", param, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if (INDEX_VALID(index))
    {
        type = 3;
        CTC_CLI_GET_UINT32("priority", param, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lookup-level");
    if (INDEX_VALID(index))
    {
        type = 4;
        CTC_CLI_GET_UINT32("lookup-level", param, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("type");
    if (INDEX_VALID(index))
    {
        if CLI_CLI_STR_EQUAL("mac", index + 1)
        {
            key_type = CTC_ACL_KEY_MAC;
        }
        else if CLI_CLI_STR_EQUAL("ipv4", index + 1)
        {
            key_type = CTC_ACL_KEY_IPV4;
        }
        else if CLI_CLI_STR_EQUAL("ipv6", index + 1)
        {
            key_type = CTC_ACL_KEY_IPV6;
        }
        else if CLI_CLI_STR_EQUAL("cid", index + 1)
        {
            key_type = CTC_ACL_KEY_CID;
        }
        else if CLI_CLI_STR_EQUAL("intf", index + 1)
        {
            key_type = CTC_ACL_KEY_INTERFACE;
        }
        else if CLI_CLI_STR_EQUAL("fwd", index + 1)
        {
            key_type = CTC_ACL_KEY_FWD;
        }
        else if CLI_CLI_STR_EQUAL("udf", index + 1)
        {
            key_type = CTC_ACL_KEY_UDF;
        }
    }
    else /*all type*/
    {
        key_type = CTC_ACL_KEY_NUM;
    }

    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (INDEX_VALID(index))
    {
        detail = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

/*
    index = CTC_CLI_GET_ARGC_INDEX("grep");
    if (0xFF != index)
    {
        uint8 start = index;
        CTC_CLI_ACL_KEY_FIELD_SET(grep_key[grep_cnt], CTC_CLI_ACL_KEY_ARG_CHECK(grep_cnt, CTC_CLI_ACL_KEY_FIELED_GREP_NUM), start);
    }
*/
    CTC_CLI_ACL_KEY_FIELD_GREP_SET(grep_key[grep_cnt],CTC_CLI_ACL_KEY_ARG_CHECK(grep_cnt,CTC_CLI_ACL_KEY_FIELED_GREP_NUM, p_null));

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_acl_show_entry(lchip, type, param, key_type, detail, grep_key, grep_cnt);
    if (ret < 0)
    {
        ctc_cli_out("ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_acl_set_hash_mac_field_sel,
        ctc_cli_usw_acl_set_hash_mac_field_sel_cmd,
        "acl (set|unset) hash-key-type mac field-sel-id SEL_ID field-mode (" CTC_CLI_ACL_KEY_FIELD_STR_0 "|"CTC_CLI_ACL_KEY_FIELD_STR_1 ")",
        CTC_CLI_ACL_STR,
        "Set",
        "Unset",
        "hash Key type",
        "MAC Hash Key",
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        CTC_CLI_FIELD_SEL_ID_FILED_MODE_DESC,
        CTC_CLI_ACL_KEY_FIELD_DESC_0,
        CTC_CLI_ACL_KEY_FIELD_DESC_1)
{
    int32 ret = CLI_SUCCESS;
    uint8 key_type = 0;
    uint8 arg_cnt = 0;
    uint8 field_sel_id;
    uint8 index = 0;
    uint8 is_add = 0;
    uint8 scl_id = 0;
    uint8* p_null = NULL;
    ctc_field_port_t port_info;
    ctc_field_port_t port_info_mask;
    ctc_port_bitmap_t pbmp = {0};
    ctc_field_key_t key_field;
    uint8  data8[8]={0};
    uint8  mask8[8]={0};
    ctc_acl_tunnel_data_t tunnel_data;
    ctc_acl_tunnel_data_t tunnel_data_mask;
    ctc_acl_udf_t udf_data;
    ctc_acl_udf_t udf_data_mask;
    CTC_CLI_ACL_COMMON_VAR;

    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_info_mask, 0, sizeof(ctc_field_port_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    sal_memset(&tunnel_data, 0, sizeof(ctc_acl_tunnel_data_t));
    sal_memset(&tunnel_data_mask, 0, sizeof(ctc_acl_tunnel_data_t));

    key_type = CTC_ACL_KEY_HASH_MAC;
    CTC_CLI_GET_UINT32("field-sel-id", field_sel_id, argv[1]);
   /*parser field*/
    CTC_CLI_ACL_KEY_FIELD_SET((&key_field),CTC_CLI_ACL_KEY_ARG_CHECK(arg_cnt,1, p_null), 0);

    index = CTC_CLI_GET_ARGC_INDEX("set");
    key_field.data =  (index != 0xFF) ? 1 :0;   /*set/unset*/

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_set_field_to_hash_field_sel(g_api_lchip,key_type,field_sel_id ,&key_field);
    }
    else
    {
        ret = ctc_acl_set_field_to_hash_field_sel(key_type,field_sel_id,&key_field);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_acl_set_hash_ipv4_field_sel,
        ctc_cli_usw_acl_set_hash_ipv4_field_sel_cmd,
        "acl (set|unset) hash-key-type ipv4 field-sel-id SEL_ID field-mode" \
        "(" CTC_CLI_ACL_KEY_FIELD_STR_0 "|"CTC_CLI_ACL_KEY_FIELD_STR_1 ")",
        CTC_CLI_ACL_STR,
        "Set",
        "Unset",
        "hash Key type",
        "IPV4 Hash Key",
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        CTC_CLI_FIELD_SEL_ID_FILED_MODE_DESC,
        CTC_CLI_ACL_KEY_FIELD_DESC_0,
        CTC_CLI_ACL_KEY_FIELD_DESC_1)
{
    int32 ret = CLI_SUCCESS;
    uint8 key_type = 0;
    uint8 arg_cnt = 0;
    uint8 field_sel_id;
    uint8 index = 0;
    uint8 is_add = 0;
    uint8* p_null = NULL;
    uint8 scl_id = 0;
    ctc_field_port_t port_info;
    ctc_field_port_t port_info_mask;
    ctc_port_bitmap_t pbmp = {0};
    uint8  data8[8]={0};
    uint8  mask8[8]={0};
    ctc_field_key_t key_field;
    ctc_acl_tunnel_data_t tunnel_data;
    ctc_acl_tunnel_data_t tunnel_data_mask;
    ctc_acl_udf_t udf_data;
    ctc_acl_udf_t udf_data_mask;
    CTC_CLI_ACL_COMMON_VAR;

    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_info_mask, 0, sizeof(ctc_field_port_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    sal_memset(&tunnel_data, 0, sizeof(ctc_acl_tunnel_data_t));
    sal_memset(&tunnel_data_mask, 0, sizeof(ctc_acl_tunnel_data_t));

    key_type = CTC_ACL_KEY_HASH_IPV4;
    CTC_CLI_GET_UINT32("field-sel-id", field_sel_id, argv[1]);

    /*parser field*/
    CTC_CLI_ACL_KEY_FIELD_SET((&key_field),CTC_CLI_ACL_KEY_ARG_CHECK(arg_cnt,1, p_null), 0);

    index = CTC_CLI_GET_ARGC_INDEX("set");
    key_field.data = (index != 0xFF) ? 1 :0;  /*set/unset*/
    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_set_field_to_hash_field_sel(g_api_lchip,key_type,field_sel_id ,&key_field);
    }
    else
    {
        ret = ctc_acl_set_field_to_hash_field_sel(key_type,field_sel_id,&key_field);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_usw_acl_set_hash_ipv6_field_sel,
        ctc_cli_usw_acl_set_hash_ipv6_field_sel_cmd,
        "acl (set|unset) hash-key-type ipv6 field-sel-id SEL_ID field-mode" \
        "(" CTC_CLI_ACL_KEY_FIELD_STR_0 "|"CTC_CLI_ACL_KEY_FIELD_STR_1 ")",
        CTC_CLI_ACL_STR,
        "Set",
        "Unset",
        "hash Key type",
        "IPV6 Hash Key",
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        CTC_CLI_FIELD_SEL_ID_FILED_MODE_DESC,
        CTC_CLI_ACL_KEY_FIELD_DESC_0,
        CTC_CLI_ACL_KEY_FIELD_DESC_1)
{

    int32 ret = CLI_SUCCESS;
    uint8 key_type = 0;
    uint8 arg_cnt = 0;
    uint8 field_sel_id;
    uint8 index = 0;
    uint8 is_add = 0;
    uint8* p_null = NULL;
    uint8 scl_id = 0;
    ctc_field_port_t port_info;
    ctc_field_port_t port_info_mask;
    ctc_port_bitmap_t pbmp = {0};
    uint8  data8[8]={0};
    uint8  mask8[8]={0};
    ctc_field_key_t key_field;
    ctc_acl_tunnel_data_t tunnel_data;
    ctc_acl_tunnel_data_t tunnel_data_mask;
    ctc_acl_udf_t udf_data;
    ctc_acl_udf_t udf_data_mask;
    CTC_CLI_ACL_COMMON_VAR;

    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_info_mask, 0, sizeof(ctc_field_port_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    sal_memset(&tunnel_data, 0, sizeof(ctc_acl_tunnel_data_t));
    sal_memset(&tunnel_data_mask, 0, sizeof(ctc_acl_tunnel_data_t));

    key_type = CTC_ACL_KEY_HASH_IPV6;

    CTC_CLI_GET_UINT32("field-sel-id", field_sel_id, argv[1]);

    /*parser field*/
    CTC_CLI_ACL_KEY_FIELD_SET((&key_field),CTC_CLI_ACL_KEY_ARG_CHECK(arg_cnt,1, p_null), 0);
    index = CTC_CLI_GET_ARGC_INDEX("set");
    key_field.data =  (index != 0xFF) ? 1 :0;  /*set/unset*/
    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_set_field_to_hash_field_sel(g_api_lchip,key_type,field_sel_id ,&key_field);
    }
    else
    {
        ret = ctc_acl_set_field_to_hash_field_sel(key_type,field_sel_id,&key_field);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_acl_set_hash_mpls_field_sel,
        ctc_cli_usw_acl_set_hash_mpls_field_sel_cmd,
        "acl (set|unset) hash-key-type mpls field-sel-id SEL_ID field-mode" \
        "(" CTC_CLI_ACL_KEY_FIELD_STR_0 "|"CTC_CLI_ACL_KEY_FIELD_STR_1 ")",
        CTC_CLI_ACL_STR,
        "Set",
        "Unset",
        "hash Key type",
        "MPLS Hash Key",
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        CTC_CLI_FIELD_SEL_ID_FILED_MODE_DESC,
        CTC_CLI_ACL_KEY_FIELD_DESC_0,
        CTC_CLI_ACL_KEY_FIELD_DESC_1)
{
    int32 ret = CLI_SUCCESS;
    uint8 key_type = 0;
    uint8 arg_cnt = 0;
    uint8 field_sel_id;
    uint8 index = 0;
    uint8 is_add = 0;
    uint8* p_null = NULL;
    uint8 scl_id = 0;
    ctc_field_port_t port_info;
    ctc_field_port_t port_info_mask;
    ctc_port_bitmap_t pbmp = {0};
    uint8  data8[8]={0};
    uint8  mask8[8]={0};
    ctc_field_key_t key_field;
    ctc_acl_tunnel_data_t tunnel_data;
    ctc_acl_tunnel_data_t tunnel_data_mask;
    ctc_acl_udf_t udf_data;
    ctc_acl_udf_t udf_data_mask;
    CTC_CLI_ACL_COMMON_VAR;

    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_info_mask, 0, sizeof(ctc_field_port_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    sal_memset(&tunnel_data, 0, sizeof(ctc_acl_tunnel_data_t));
    sal_memset(&tunnel_data_mask, 0, sizeof(ctc_acl_tunnel_data_t));

    key_type = CTC_ACL_KEY_HASH_MPLS;
    CTC_CLI_GET_UINT32("field-sel-id", field_sel_id, argv[1]);

     /*parser field*/
    CTC_CLI_ACL_KEY_FIELD_SET((&key_field),CTC_CLI_ACL_KEY_ARG_CHECK(arg_cnt,1, p_null), 0);
    index = CTC_CLI_GET_ARGC_INDEX("set");
    key_field.data =  (index != 0xFF) ? 1 :0;  /*set/unset*/

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_set_field_to_hash_field_sel(g_api_lchip,key_type,field_sel_id ,&key_field);
    }
    else
    {
        ret = ctc_acl_set_field_to_hash_field_sel(key_type,field_sel_id,&key_field);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_usw_acl_set_hash_l2_l3_field_sel,
        ctc_cli_usw_acl_set_hash_l2_l3_field_sel_cmd,
        "acl (set|unset) hash-key-type l2-l3 field-sel-id SEL_ID field-mode" \
        "(" CTC_CLI_ACL_KEY_FIELD_STR_0 "|"CTC_CLI_ACL_KEY_FIELD_STR_1 ")",
        CTC_CLI_ACL_STR,
        "Set",
        "Unset",
        "hash Key type",
        "L2_L3 Hash Key",
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        CTC_CLI_FIELD_SEL_ID_FILED_MODE_DESC,
        CTC_CLI_ACL_KEY_FIELD_DESC_0,
        CTC_CLI_ACL_KEY_FIELD_DESC_1)
{
    int32 ret = CLI_SUCCESS;
    uint8 key_type = 0;
    uint8 arg_cnt = 0;
    uint8 field_sel_id;
    uint8 index = 0;
    uint8 is_add = 0;
    uint8* p_null = NULL;
    uint8 scl_id = 0;
    ctc_field_port_t port_info;
    ctc_field_port_t port_info_mask;
    ctc_port_bitmap_t pbmp = {0};
    ctc_field_key_t key_field;
    uint8  data8[8]={0};
    uint8  mask8[8]={0};
    ctc_acl_tunnel_data_t tunnel_data;
    ctc_acl_tunnel_data_t tunnel_data_mask;
    ctc_acl_udf_t udf_data;
    ctc_acl_udf_t udf_data_mask;
    CTC_CLI_ACL_COMMON_VAR;

    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_info_mask, 0, sizeof(ctc_field_port_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    sal_memset(&tunnel_data, 0, sizeof(ctc_acl_tunnel_data_t));
    sal_memset(&tunnel_data_mask, 0, sizeof(ctc_acl_tunnel_data_t));

    key_type = CTC_ACL_KEY_HASH_L2_L3;

    CTC_CLI_GET_UINT32("field-sel-id", field_sel_id, argv[1]);

     /*parser field*/
    CTC_CLI_ACL_KEY_FIELD_SET((&key_field),CTC_CLI_ACL_KEY_ARG_CHECK(arg_cnt,1, p_null), 0);
    index = CTC_CLI_GET_ARGC_INDEX("set");
    key_field.data =  (index != 0xFF) ? 1 :0;  /*set/unset*/
    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_set_field_to_hash_field_sel(g_api_lchip,key_type,field_sel_id ,&key_field);
    }
    else
    {
        ret = ctc_acl_set_field_to_hash_field_sel(key_type,field_sel_id,&key_field);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_usw_acl_add_key_field_0,
        ctc_cli_usw_acl_add_key_field_cmd_0,
        "acl entry ENTRY_ID (add|remove) key-field {"CTC_CLI_ACL_KEY_FIELD_STR_0 "}",
        CTC_CLI_ACL_STR,
        "Acl entry id",
        "ENTRY_ID",
        "add","remove",
        "key field",
        CTC_CLI_ACL_KEY_FIELD_DESC_0)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 is_add = 0;
    uint8 arg_cnt = 0;
    uint32 field_cnt = 0;
    uint32 entry_id = 0;
    uint32 scl_id = 0;
    ctc_field_port_t port_info;
    ctc_field_port_t port_info_mask;
    ctc_port_bitmap_t pbmp = {0};
    ctc_field_key_t key_field[CTC_CLI_ACL_KEY_FIELED_GREP_NUM];
    ctc_field_key_t* p_field = &key_field[0];
    uint8  data8[8]={0};
    uint8  mask8[8]={0};
    ctc_acl_tunnel_data_t tunnel_data;
    ctc_acl_tunnel_data_t tunnel_data_mask;
    ctc_acl_udf_t udf_data;
    ctc_acl_udf_t udf_data_mask;
    CTC_CLI_ACL_COMMON_VAR;

    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_info_mask, 0xFF, sizeof(ctc_field_port_t));
    sal_memset(key_field, 0, CTC_CLI_ACL_KEY_FIELED_GREP_NUM * sizeof(ctc_field_key_t));
    sal_memset(&tunnel_data, 0, sizeof(ctc_acl_tunnel_data_t));
    sal_memset(&tunnel_data_mask, 0xFF, sizeof(ctc_acl_tunnel_data_t));
    sal_memset(&udf_data, 0, sizeof(ctc_acl_udf_t));
    sal_memset(&udf_data_mask, 0xFF, sizeof(ctc_acl_udf_t));

    for (index=0; index<CTC_CLI_ACL_KEY_FIELED_GREP_NUM; index+=1)
    {
        key_field[index].mask = 0xFFFFFFFF;
    }

    CTC_CLI_GET_UINT32("entry-id", entry_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (0xFF != index)
    {
        is_add = 1;
    }

    CTC_CLI_ACL_KEY_FIELD_SET(p_field,CTC_CLI_ACL_KEY_ARG_CHECK(arg_cnt,CTC_CLI_ACL_KEY_FIELED_GREP_NUM, p_field), 0);
    field_cnt = arg_cnt;

    if(g_ctcs_api_en)
    {
        if(is_add)
        {
            ret = field_cnt>1? ctcs_acl_add_key_field_list(g_api_lchip, entry_id, key_field, &field_cnt): ctcs_acl_add_key_field(g_api_lchip, entry_id, key_field);
        }
        else
        {
            ret = field_cnt>1? ctcs_acl_remove_key_field_list(g_api_lchip, entry_id, key_field, &field_cnt): ctcs_acl_remove_key_field(g_api_lchip, entry_id, key_field);
        }
    }
    else
    {
        if(is_add)
        {
            ret = field_cnt>1? ctc_acl_add_key_field_list(entry_id, key_field, &field_cnt): ctc_acl_add_key_field(entry_id, key_field);
        }
        else
        {
            ret = field_cnt>1? ctc_acl_remove_key_field_list(entry_id, key_field, &field_cnt): ctc_acl_remove_key_field(entry_id, key_field);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        if (1<arg_cnt)
        {
            ctc_cli_out("%% %6s %u field successfully! \n", is_add? "add": "remove", field_cnt);
        }
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_acl_add_key_field_1,
        ctc_cli_usw_acl_add_key_field_cmd_1,
        "acl entry ENTRY_ID (add|remove) key-field {" CTC_CLI_ACL_KEY_FIELD_STR_1 "}",
        CTC_CLI_ACL_STR,
        "Acl entry id",
        "ENTRY_ID",
        "add","remove",
        "key field",
        CTC_CLI_ACL_KEY_FIELD_DESC_1)
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 is_add = 0;
    uint8 arg_cnt = 0;
    uint8 scl_id = 0;
    uint32 field_cnt = 0;
    uint32 entry_id = 0;
    ctc_port_bitmap_t pbmp = {0};
    ctc_field_key_t key_field[CTC_CLI_ACL_KEY_FIELED_GREP_NUM];
    ctc_field_key_t* p_field = &key_field[0];
    ctc_field_port_t port_info;
    ctc_field_port_t port_info_mask;
    uint8  data8[8] = {0};
    uint8  mask8[8] = {0};
    ctc_acl_tunnel_data_t tunnel_data;
    ctc_acl_tunnel_data_t tunnel_data_mask;
    ctc_acl_udf_t udf_data;
    ctc_acl_udf_t udf_data_mask;
    CTC_CLI_ACL_COMMON_VAR;

    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_info_mask, 0xFF, sizeof(ctc_field_port_t));
    sal_memset(key_field, 0, CTC_CLI_ACL_KEY_FIELED_GREP_NUM * sizeof(ctc_field_key_t));
    sal_memset(&tunnel_data, 0, sizeof(ctc_acl_tunnel_data_t));
    sal_memset(&tunnel_data_mask, 0xFF, sizeof(ctc_acl_tunnel_data_t));
    sal_memset(&udf_data, 0, sizeof(ctc_acl_udf_t));
    sal_memset(&udf_data_mask, 0xFF, sizeof(ctc_acl_udf_t));

    for (index=0; index<CTC_CLI_ACL_KEY_FIELED_GREP_NUM; index+=1)
    {
        key_field[index].mask = 0xFFFFFFFF;
    }

    CTC_CLI_GET_UINT32("entry-id", entry_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (0xFF != index)
    {
        is_add = 1;
    }
    CTC_CLI_ACL_KEY_FIELD_SET(p_field,CTC_CLI_ACL_KEY_ARG_CHECK(arg_cnt,CTC_CLI_ACL_KEY_FIELED_GREP_NUM, p_field), 0);
    field_cnt = arg_cnt;
    if(g_ctcs_api_en)
    {
        if(is_add)
        {
            ret = field_cnt>1? ctcs_acl_add_key_field_list(g_api_lchip, entry_id, key_field, &field_cnt): ctcs_acl_add_key_field(g_api_lchip, entry_id, key_field);
        }
        else
        {
            ret = field_cnt>1? ctcs_acl_remove_key_field_list(g_api_lchip, entry_id, key_field, &field_cnt): ctcs_acl_remove_key_field(g_api_lchip, entry_id, key_field);
        }
    }
    else
    {
        if(is_add)
        {
            ret = field_cnt>1? ctc_acl_add_key_field_list(entry_id, key_field, &field_cnt): ctc_acl_add_key_field(entry_id, key_field);
        }
        else
        {
            ret = field_cnt>1? ctc_acl_remove_key_field_list(entry_id, key_field, &field_cnt): ctc_acl_remove_key_field(entry_id, key_field);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        if (1<arg_cnt)
        {
            ctc_cli_out("%% %6s %u field successfully! \n", is_add? "add": "remove", field_cnt);
        }
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_acl_add_action_field,
        ctc_cli_usw_acl_add_action_field_cmd,
        "acl entry ENTRY_ID add action-field {" CTC_CLI_ACL_ADD_FIELD_ACTION_STR "}",
        CTC_CLI_ACL_STR,
        "Acl entry id",
        "ENTRY_ID",
        "add one action field to an entry",
        "action field",
        CTC_CLI_ACL_ADD_FIELD_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 arg_cnt = 0;
    uint32 field_cnt = 0;
    uint32 entry_id = 0;
    ctc_acl_field_action_t act_field[CTC_CLI_ACL_KEY_FIELED_GREP_NUM];
    ctc_acl_field_action_t* p_field = &act_field[0];
    ctc_acl_vlan_edit_t vlan_edit;
    ctc_acl_packet_strip_t packet_strip;
    ctc_acl_ipfix_t ipfix;
    ctc_acl_lb_hash_t lb_hash;
    ctc_acl_oam_t oam_t;
    ctc_acl_to_cpu_t cp_to_cpu;
    ctc_acl_table_map_t table_map;
    ctc_acl_inter_cn_t inter_cn;
    ctc_acl_vxlan_rsv_edit_t vxlan_rsv_edit;
    ctc_acl_timestamp_t timestamp;

    sal_memset(&vxlan_rsv_edit, 0, sizeof(ctc_acl_vxlan_rsv_edit_t));
    sal_memset(&vlan_edit, 0, sizeof(ctc_acl_vlan_edit_t));
    sal_memset(act_field, 0, CTC_CLI_ACL_KEY_FIELED_GREP_NUM * sizeof(ctc_acl_field_action_t));
    sal_memset(&packet_strip, 0, sizeof(ctc_acl_packet_strip_t));
    sal_memset(&ipfix, 0, sizeof(ctc_acl_ipfix_t));
    sal_memset(&oam_t, 0, sizeof(ctc_acl_oam_t));
    sal_memset(&cp_to_cpu, 0, sizeof(ctc_acl_to_cpu_t));
    sal_memset(&lb_hash, 0, sizeof(lb_hash));
    sal_memset(&table_map, 0, sizeof(ctc_acl_table_map_t));
    sal_memset(&inter_cn, 0, sizeof(ctc_acl_inter_cn_t));
    sal_memset(&timestamp, 0, sizeof(ctc_acl_timestamp_t));

    CTC_CLI_GET_UINT32("entry-id", entry_id, argv[0]);
    CTC_CLI_ACL_ADD_FIELD_ACTION_SET(p_field,CTC_CLI_ACL_KEY_ARG_CHECK(arg_cnt,CTC_CLI_ACL_KEY_FIELED_GREP_NUM,p_field));
    field_cnt = arg_cnt;
    if(g_ctcs_api_en)
    {
        ret = field_cnt>1? ctcs_acl_add_action_field_list(g_api_lchip, entry_id, act_field, &field_cnt): ctcs_acl_add_action_field(g_api_lchip, entry_id, act_field);
    }
    else
    {

        ret = field_cnt>1? ctc_acl_add_action_field_list(entry_id, act_field, &field_cnt): ctc_acl_add_action_field(entry_id, act_field);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        if (1<arg_cnt)
        {
            ctc_cli_out("%% add %u field successfully! \n", field_cnt);
        }
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_acl_remove_action_field,
        ctc_cli_usw_acl_remove_action_field_cmd,
        "acl entry ENTRY_ID remove action-field {" CTC_CLI_ACL_REMOVE_FIELD_ACTION_STR "}",
        CTC_CLI_ACL_STR,
        "Acl entry id",
        "ENTRY_ID",
        "remove one action field to an entry",
        "action field",
        CTC_CLI_ACL_REMOVE_FIELD_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 arg_cnt = 0;
    uint32 field_cnt = 0;
    uint32 entry_id = 0;
    ctc_acl_field_action_t act_field[CTC_CLI_ACL_KEY_FIELED_GREP_NUM];
    ctc_acl_field_action_t* p_field = &act_field[0];

    sal_memset(act_field, 0, CTC_CLI_ACL_KEY_FIELED_GREP_NUM * sizeof(ctc_acl_field_action_t));

    CTC_CLI_GET_UINT32("entry-id", entry_id, argv[0]);
    CTC_CLI_ACL_REMOVE_FIELD_ACTION_SET(p_field,CTC_CLI_ACL_KEY_ARG_CHECK(arg_cnt, CTC_CLI_ACL_KEY_FIELED_GREP_NUM, p_field));
    field_cnt = arg_cnt;
    if(g_ctcs_api_en)
    {
        ret = field_cnt>1? ctcs_acl_remove_action_field_list(g_api_lchip, entry_id, act_field, &field_cnt): ctcs_acl_remove_action_field(g_api_lchip, entry_id, act_field);
    }
    else
    {
        ret = field_cnt>1? ctc_acl_remove_action_field_list(entry_id, act_field, &field_cnt): ctc_acl_remove_action_field(entry_id, act_field);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        if (1<arg_cnt)
        {
            ctc_cli_out("%% remove %u field successfully! \n", field_cnt);
        }
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_acl_add_entry,
        ctc_cli_usw_acl_add_entry_cmd,
        "acl add group GROUP_ID (entry ENTRY_ID) (entry-priority PRIORITY|)  \
        (" CTC_CLI_ACL_ENTRY_TYPE_STR ") field-mode",
        CTC_CLI_ACL_ADD_ENTRY_DESC,
        CTC_CLI_ACL_ENTRY_TYPE_DESC,
        "Use key and action field to add entry")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = 0;
    ctc_acl_entry_t acl_entry;

    sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));

    CTC_CLI_GET_UINT32("group id", gid, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("entry");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("entry id", acl_entry.entry_id, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("entry-priority");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("entry-priority", acl_entry.priority, argv[index + 1]);
        acl_entry.priority_valid = 1;
    }
    acl_entry.mode = 1;


    CTC_CLI_ACL_KEY_TYPE_SET

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_add_entry(g_api_lchip, gid, &acl_entry);
    }
    else
    {
        ret = ctc_acl_add_entry(gid, &acl_entry);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_usw_acl_add_hash_entry,
        ctc_cli_usw_acl_add_hash_entry_cmd,
        "acl add group GROUP_ID (entry ENTRY_ID) (entry-priority PRIORITY|)  \
        (" CTC_CLI_ACL_HASH_ENTRY_TYPE_STR ")  field-mode (field-sel-id SEL_ID)",
        CTC_CLI_ACL_ADD_ENTRY_DESC,
        CTC_CLI_ACL_HASH_ENTRY_TYPE_DESC,
        "Use key and action field to add entry",
        "Hash key field select id ",
        "Field select ID")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = 0;
    ctc_acl_entry_t acl_entry;

    sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));

    index = CTC_CLI_GET_ARGC_INDEX("entry");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("entry id", acl_entry.entry_id, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("entry-priority");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("entry-priority", acl_entry.priority, argv[index + 1]);
        acl_entry.priority_valid = 1;
    }

    acl_entry.mode = 1;

    index = CTC_CLI_GET_ARGC_INDEX("field-sel-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("field-sel-id", acl_entry.hash_field_sel_id, argv[index + 1]);
    }

    CTC_CLI_ACL_KEY_TYPE_SET
    CTC_CLI_GET_UINT32("group id", gid, argv[0]);
    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_add_entry(g_api_lchip, gid, &acl_entry);
    }
    else
    {
        ret = ctc_acl_add_entry(gid, &acl_entry);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_usw_acl_add_cid_pair,
        ctc_cli_usw_acl_add_cid_pair_cmd,
        "acl (add|remove) (hash-cid-pair|tcam-cid-pair) {src-cid SRC_CID | dst-cid DST_CID | } \
         (permit | deny| overwrite-acl PRIORITY class-id CLASS_ID lkup-type (l2 | l3 |l2-l3 | vlan | l3-ext | l2-l3-ext | \
        cid | interface | forward | forward-ext |udf) (use-mapped-vlan|) |)",
        CTC_CLI_ACL_STR,
        "add", "remove",
        "Hash CategoryId Pair", "Tcam CategoryId Pair",
        "Src CategoryId", "value",
        "Dst CategoryId", "value",
        "Permit", "Discard",
        "Overwrite", "acl priority",
        "class-id", "CLASS_ID",
        "tcam lookup type",
        "L2 key", "L3 key", "L2 L3 key", "Vlan key", "L3 extend key",
        "L2 L3 extend key", "Cid key", "Interface key", "Forward key", "Forward extend Key", "Udf key",
        "use mapped vlan")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint8 is_add = 0;
    ctc_acl_cid_pair_t cid_pair;

    sal_memset(&cid_pair, 0, sizeof(ctc_acl_cid_pair_t));

    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (0xFF != index)
    {
        is_add = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("tcam-cid-pair");
    if (0xFF != index)
    {
        cid_pair.flag |= CTC_ACL_CID_PAIR_FLAG_FLEX;
    }
    index = CTC_CLI_GET_ARGC_INDEX("src-cid");
    if (0xFF != index)
    {
        cid_pair.flag |= CTC_ACL_CID_PAIR_FLAG_SRC_CID;
        CTC_CLI_GET_UINT32("src cid", cid_pair.src_cid, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");
    if (0xFF != index)
    {
        cid_pair.flag |= CTC_ACL_CID_PAIR_FLAG_DST_CID;
        CTC_CLI_GET_UINT32("dst cid", cid_pair.dst_cid, argv[index + 1]);
    }
    if(is_add)
    {
        index = CTC_CLI_GET_ARGC_INDEX("permit");
        if (0xFF != index)
        {
            cid_pair.action_mode = CTC_ACL_CID_PAIR_ACTION_PERMIT;
        }
        index = CTC_CLI_GET_ARGC_INDEX("deny");
        if (0xFF != index)
        {
            cid_pair.action_mode = CTC_ACL_CID_PAIR_ACTION_DISCARD;
        }
        index = CTC_CLI_GET_ARGC_INDEX("overwrite-acl");
        if (0xFF != index)
        {
            cid_pair.action_mode = CTC_ACL_CID_PAIR_ACTION_OVERWRITE_ACL;
            CTC_CLI_GET_UINT8("acl priority", cid_pair.acl_prop.acl_priority, argv[index + 1]);
            index = CTC_CLI_GET_ARGC_INDEX("lkup-type");
            if(0xFF != index)
            {
                if(CTC_CLI_STR_EQUAL_ENHANCE("l2",index + 1))
                {
                    cid_pair.acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L2;
                }
                else if(CTC_CLI_STR_EQUAL_ENHANCE("l3",index + 1))
                {
                    cid_pair.acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L3;
                }
                else if(CTC_CLI_STR_EQUAL_ENHANCE("l2-l3",index + 1))
                {
                    cid_pair.acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L2_L3;
                }
                else if(CLI_CLI_STR_EQUAL("vlan",index + 1))
                {
                    cid_pair.acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_VLAN;
                }
                else if(CTC_CLI_STR_EQUAL_ENHANCE("l3-ext",index + 1))
                {
                    cid_pair.acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L3_EXT;
                }
                else if(CTC_CLI_STR_EQUAL_ENHANCE("l2-l3-ext",index + 1 ))
                {
                    cid_pair.acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L2_L3_EXT;
                }
                else if(CLI_CLI_STR_EQUAL("cid",index + 1 ))
                {
                    cid_pair.acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_CID;
                }
                else if(CLI_CLI_STR_EQUAL("interface",index + 1 ))
                {
                    cid_pair.acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_INTERFACE;
                }
                else if(CTC_CLI_STR_EQUAL_ENHANCE("forward",index + 1 ))
                {
                    cid_pair.acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_FORWARD;
                }
                else if(CTC_CLI_STR_EQUAL_ENHANCE("forward-ext",index + 1 ))
                {
                    cid_pair.acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_FORWARD_EXT;
                }
                else if(CTC_CLI_STR_EQUAL_ENHANCE("udf",index + 1 ))
                {
                    cid_pair.acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_UDF;
                }
            }
            index = CTC_CLI_GET_ARGC_INDEX("class-id");
            if(0xFF != index)
            {
                CTC_CLI_GET_UINT8("class-id", cid_pair.acl_prop.class_id, argv[index + 1]);
            }
            index = CTC_CLI_GET_ARGC_INDEX("use-mapped-vlan");
            if(0xFF != index)
            {
                CTC_SET_FLAG(cid_pair.acl_prop.flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN);
            }
        }
    }

    if(g_ctcs_api_en)
    {
        if(is_add)
        {
            ret = ctcs_acl_add_cid_pair(g_api_lchip, &cid_pair);
        }
        else
        {
            ret = ctcs_acl_remove_cid_pair(g_api_lchip, &cid_pair);
        }
    }
    else
    {
        if(is_add)
        {
            ret = ctc_acl_add_cid_pair(&cid_pair);
        }
        else
        {
            ret = ctc_acl_remove_cid_pair(&cid_pair);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_acl_show_cid_pair,
        ctc_cli_usw_acl_show_cid_pair_cmd,
        "show acl cid-pair (hash|tcam|both) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_ACL_STR,
        "CategoryId Pair",
        "hash cid pair",
        "tcam cid pair",
        "both hash and tcam",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint8 type = 0;
    uint8 lchip = 0;
    ctc_acl_cid_pair_t cid_pair;

    sal_memset(&cid_pair, 0, sizeof(ctc_acl_cid_pair_t));

    index = CTC_CLI_GET_ARGC_INDEX("hash");
    if (0xFF != index)
    {
        type = 0;
    }
    index = CTC_CLI_GET_ARGC_INDEX("tcam");
    if (0xFF != index)
    {
        type = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("both");
    if (0xFF != index)
    {
        type = 2;
    }
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = sys_usw_acl_show_cid_pair(lchip, type);

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_acl_create_group_ext,
        ctc_cli_usw_acl_create_group_ext_cmd,
        "acl create group GROUP_ID {priority PRIORITY |key-size SIZE|} direction (ingress|egress) (lchip LCHIP|) \
        (l3if-class CLASS_ID | hash )",
        CTC_CLI_ACL_STR,
        "Create ACL group",
        CTC_CLI_ACL_GROUP_ID_STR,
        CTC_CLI_ACL_S_GROUP_ID_VALUE,
        CTC_CLI_ACL_GROUP_PRIO_STR,
        CTC_CLI_ACL_GROUP_PRIO_VALUE,
        "Key size",
        "Key size value",
        "Direction",
        "Ingress",
        "Egress",
        "Lchip",
        CTC_CLI_LCHIP_ID_VALUE,
        "L3if class acl",
        "l3if class id",
        "Hash type")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 group_id = 0;
    ctc_acl_group_info_t ginfo;

    sal_memset(&ginfo, 0, sizeof(ctc_acl_group_info_t));

    CTC_CLI_GET_UINT32("group id", group_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("group prioirty", ginfo.priority, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("key-size");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("key size", ginfo.key_size, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (INDEX_VALID(index))
    {
        ginfo.dir = CTC_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (INDEX_VALID(index))
    {
        ginfo.dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("both");
    if (INDEX_VALID(index))
    {
        ginfo.dir = CTC_BOTH_DIRECTION;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("lchip id", ginfo.lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l3if-class");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("l3if class id", ginfo.un.l3if_class_id, argv[index + 1]);
        ginfo.type  = CTC_ACL_GROUP_TYPE_L3IF_CLASS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("hash");
    if (INDEX_VALID(index))
    {
        ginfo.type  = CTC_ACL_GROUP_TYPE_HASH;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_create_group(g_api_lchip, group_id, &ginfo);
    }
    else
    {
        ret = ctc_acl_create_group(group_id, &ginfo);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_usw_acl_show_block_status,
        ctc_cli_usw_acl_show_block_status_cmd,
        "show acl (block-id BLOCK_ID) (direction (ingress|egress)|) status (lchip LCHIP|)",
        "Show",
        CTC_CLI_ACL_STR,
        "block id",
        "BLOCK ID",
        "direction",
        "ingress",
        "egress",
        "status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    uint8 dir = CTC_INGRESS;
    uint8 block_id = 0;

    index = CTC_CLI_GET_ARGC_INDEX("block-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("block-id", block_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (0xFF != index)
    {
        dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = sys_usw_acl_show_tcam_alloc_status(lchip, dir, block_id);

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_usw_acl_set_cid_prio,
        ctc_cli_usw_acl_set_cid_prio_cmd,
        "acl set (src-cid-prio pkt-cid VALUE0 i2e-cid VALUE1 global-cid VALUE2 static-cid VALUE3 dynamic-cid VALUE4  \
        if-cid VALUE5 default-cid VALUE6 | dst-cid-prio dynamic-cid VALUE0 dsflow-cid VALUE1 static-cid VALUE2       \
        default-cid VALUE3) (lchip LCHIP|)",
        CTC_CLI_ACL_STR,
        "Set",
        "Src cid priority", "cid from packet", "<0-7>", "cid from header", "<0-7>", "global cid", "<0-7>",
        "static-cid from DsUserId, DsScl, DsFlow", "<0-7>", "dynamic-cid from DsMacSa, DsIpSa", "<0-7>",
        "if-cid from port, vlan, if, tunnel", "<0-7>", "default cid", "<0-7>",
        "Dest cid priority", "dynamic-cid from DsMacDa, DsIpDa", "<0-3>", "cid from DsFlow", "<0-3>",
        "static-cid from DsUserId, DsScl", "<0-3>", "default cid", "<0-3>",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    uint8 array[7] = {0};
    uint8 is_src = 0;

    index = CTC_CLI_GET_ARGC_INDEX("src-cid-prio");
    if (0xFF != index)
    {
        is_src = 1;
        CTC_CLI_GET_UINT8("pkt-cid",     array[0], argv[index + 2]);
        CTC_CLI_GET_UINT8("i2e-cid",     array[1], argv[index + 4]);
        CTC_CLI_GET_UINT8("global-cid",  array[2], argv[index + 6]);
        CTC_CLI_GET_UINT8("static-cid",  array[3], argv[index + 8]);
        CTC_CLI_GET_UINT8("dynamic-cid", array[4], argv[index + 10]);
        CTC_CLI_GET_UINT8("if-cid",      array[5], argv[index + 12]);
        CTC_CLI_GET_UINT8("default-cid", array[6], argv[index + 14]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dst-cid-prio");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("static-cid",  array[0], argv[index + 2]);
        CTC_CLI_GET_UINT8("dsflow-cid",  array[1], argv[index + 4]);
        CTC_CLI_GET_UINT8("static-cid",  array[2], argv[index + 6]);
        CTC_CLI_GET_UINT8("default-cid", array[3], argv[index + 8]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = sys_usw_acl_set_cid_priority(lchip, is_src, array);

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}
CTC_CLI(ctc_cli_usw_acl_add_udf_entry,
        ctc_cli_usw_acl_add_udf_entry_cmd,
        "acl add udf-entry UDF-ID priority PRIORITY (offset-type TYPE | {offset0-type TYPE|offset1-type TYPE|offset2-type TYPE|offset3-type TYPE}) \
        {offset0 OFFSET0|offset1 OFFSET1| offset2 OFFSET2 |offset3 OFFSET3} (tcp-op|)",
        CTC_CLI_ACL_STR,
        "Add",
        "UDF Entry",
        "UDF ID",
        "Priority",
        "Priority ID",
        "Start offset-type",
        "TYPE, refer to ctc_acl_udf_offset_type_t",
        "Start offset0-type", "TYPE, refer to ctc_acl_udf_offset_type_t",
        "Start offset1-type", "TYPE, refer to ctc_acl_udf_offset_type_t",
        "Start offset2-type", "TYPE, refer to ctc_acl_udf_offset_type_t",
        "Start offset3-type", "TYPE, refer to ctc_acl_udf_offset_type_t",
        "Offset0",
        "Offset0 Value",
        "Offset1",
        "Offset1 Value",
        "Offset2",
        "Offset2 Value",
        "Offset3",
        "Offset3 Value",
        "tcp options")
{

    int32 ret = CLI_SUCCESS;

    uint8 index = 0;
    ctc_acl_classify_udf_t udf_entry;
	sal_memset(&udf_entry,0,sizeof(udf_entry));

    CTC_CLI_GET_UINT32("udf-id", udf_entry.udf_id, argv[0]);
    CTC_CLI_GET_UINT16("priority", udf_entry.priority, argv[1]);
    index = CTC_CLI_GET_ARGC_INDEX("offset-type");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("offset-type", udf_entry.offset_type, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("offset0-type");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("offset0-type", udf_entry.offset_type_array[0], argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("offset1-type");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("offset1-type", udf_entry.offset_type_array[1], argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("offset2-type");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("offset2-type", udf_entry.offset_type_array[2], argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("offset3-type");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("offset3-type", udf_entry.offset_type_array[3], argv[index + 1]);
    }
    udf_entry.offset_num = 0;
    index = CTC_CLI_GET_ARGC_INDEX("offset0");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("offset0", udf_entry.offset[0], argv[index + 1]);
		udf_entry.offset_num +=1;
    }
	index = CTC_CLI_GET_ARGC_INDEX("offset1");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("offset1", udf_entry.offset[1], argv[index + 1]);
		udf_entry.offset_num +=1;
    }
	index = CTC_CLI_GET_ARGC_INDEX("offset2");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("offset2", udf_entry.offset[2], argv[index + 1]);
		udf_entry.offset_num +=1;
    }
	index = CTC_CLI_GET_ARGC_INDEX("offset3");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("offset3", udf_entry.offset[3], argv[index + 1]);
		udf_entry.offset_num +=1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("tcp-op");
    if (0xFF != index)
    {
       udf_entry.tcp_option_en = 1;
    }

     if(g_ctcs_api_en)
    {
        ret = ctcs_acl_add_udf_entry(g_api_lchip, &udf_entry);
    }
    else
    {
        ret = ctc_acl_add_udf_entry(&udf_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_usw_acl_remove_udf_entry,
        ctc_cli_usw_acl_remove_udf_entry_cmd,
        "acl remove udf-entry UDF-ID",
        CTC_CLI_ACL_STR,
        "Remove",
        "UDF Entry",
        "UDF ID")
{

    int32 ret = CLI_SUCCESS;

    ctc_acl_classify_udf_t udf_entry;
	sal_memset(&udf_entry,0,sizeof(udf_entry));

    CTC_CLI_GET_UINT32("udf-id", udf_entry.udf_id, argv[0]);


     if(g_ctcs_api_en)
    {
        ret = ctcs_acl_remove_udf_entry(g_api_lchip, &udf_entry);
    }
    else
    {
        ret = ctc_acl_remove_udf_entry(&udf_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}



#define CTC_CLI_ACL_UDF_KEY_FIELD_STR "\
  l2-type (VALUE (MASK|)|)\
| l3-type (VALUE (MASK|)|) \
| l4-type (VALUE (MASK|)|) \
| l4-user-type (VALUE (MASK|)|) \
| field-port (port-class (CLASS_ID MASK |) | vlan-class (CLASS_ID MASK |) | l3if-class  (CLASS_ID MASK |) | pbr-class (CLASS_ID MASK |) | logic-port (LOGICAL_PORT MASK|) | gport (GPORT MASK|) | port-bitmap (LCHIP PORT_BITMAP_3 PORT_BITMAP_2 PORT_BITMAP_1 PORT_BITMAP_0|)) \
| ether-type (VALUE (MASK|)|) \
| vlan-num (VALUE (MASK|)|) \
| ip-protocol (VALUE (MASK|)|) \
| ip-frag (VALUE (MASK|)|) \
| ip-options (VALUE (MASK|)|) \
| l4-dst-port (VALUE (MASK|)|) \
| l4-src-port (VALUE (MASK|)|) \
| tcp-op (VALUE (MASK|)|) \
| gre-flags (VALUE (MASK|)|) \
| gre-protocol (VALUE (MASK|)|) \
| label-num (VALUE (MASK|)|) \
| udf-entry-valid (VALUE (MASK|)|) \
"



#define CTC_CLI_ACL_UDF_KEY_FIELD_DESC \
"layer 2 type (ctc_parser_l2_type_t) ", "value", "mask",\
"layer 3 type (ctc_parser_l3_type_t) ", "value", "mask", \
"layer 4 type (ctc_parser_l4_type_t) ", "value", "mask",\
"layer 4 user-type (ctc_parser_l4_usertype_t) ", "value", "mask",\
"port-type","port class id", "value","mask","vlan class id","value", "mask","l3if class id","value", "mask","pbr class id","value", "mask","logic port","value", "mask","global port","value", "mask",\
CTC_CLI_ACL_PORT_BITMAP,CTC_CLI_LCHIP_ID_VALUE,"port bitmap_3 <port96~port127>","port bitmap_2 <port64~port95>","port bitmap_1 <port32~port63>","port bitmap_0 <port0~port31>",\
"ether type ", "value", "mask",\
"vlan tag number", "value", "mask",\
"ip protocol ", "value", "mask",\
"ip fragment information ", "value", "mask",\
"ip options ", "value", "mask",\
"layer 4 dest port ", "value", "mask",\
"layer 4 src port ", "value", "mask",\
"tcp options ", "value", "mask",\
"gre flags ", "value", "mask",\
"gre protocol ", "value", "mask",\
"mpls label number ", "value", "mask",\
"UDF Entry valid ", "value", "mask"

CTC_CLI(ctc_cli_usw_acl_add_remove_udf_entry_key_field,
        ctc_cli_usw_acl_add_remove_udf_entry_key_field_cmd,
        "acl udf-entry UDF_ID (add|remove) key-field(" CTC_CLI_ACL_UDF_KEY_FIELD_STR")",
        CTC_CLI_ACL_STR,
        "UDF Entry",
        "UDF entry id",
        "add",
        "remove",
        "key field",
        CTC_CLI_ACL_UDF_KEY_FIELD_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 arg_cnt = 0;
    uint8 is_add = 0;
    uint8* p_null = NULL;
    uint8 scl_id = 0;
    uint32 entry_id = 0;
    ctc_field_port_t port_info;
    ctc_field_port_t port_info_mask;
    ctc_port_bitmap_t pbmp = {0};
    ctc_field_key_t key_field;
    uint8  data8[8]={0};
    uint8  mask8[8]={0};
    ctc_acl_tunnel_data_t tunnel_data;
    ctc_acl_tunnel_data_t tunnel_data_mask;
    ctc_acl_udf_t udf_data;
    ctc_acl_udf_t udf_data_mask;
    CTC_CLI_ACL_COMMON_VAR;

    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_info_mask, 0, sizeof(ctc_field_port_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    sal_memset(&tunnel_data, 0, sizeof(ctc_acl_tunnel_data_t));
    sal_memset(&tunnel_data_mask, 0, sizeof(ctc_acl_tunnel_data_t));
    key_field.mask = 0xFFFFFFFF;

    CTC_CLI_GET_UINT32("entry-id", entry_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (0xFF != index)
    {
        is_add = 1;
    }

    CTC_CLI_ACL_KEY_FIELD_SET((&key_field),CTC_CLI_ACL_KEY_ARG_CHECK(arg_cnt,1, p_null), 3);

    if(g_ctcs_api_en)
    {
        if(is_add)
        {
            ret = ctcs_acl_add_udf_entry_key_field(g_api_lchip, entry_id, &key_field);
        }
        else
        {
            ret = ctcs_acl_remove_udf_entry_key_field(g_api_lchip, entry_id, &key_field);
        }
    }
    else
    {
        if(is_add)
        {
            ret = ctc_acl_add_udf_entry_key_field(entry_id, &key_field);
        }
        else
        {
            ret = ctc_acl_remove_udf_entry_key_field(entry_id, &key_field);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_acl_show_udf_entry,
        ctc_cli_usw_acl_show_udf_entry_cmd,
        "show acl udf-entry (udf-id UDF-ID | priority PRIORITY)",
        "Show",
        CTC_CLI_ACL_STR,
        "UDF-Entry",
        "UDF entry id",
        "UDF entry id",
        "UDF entry Priority",
        "UDF entry Priority")
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
	uint8 loop = 0;

    ctc_acl_classify_udf_t udf_entry;
	sal_memset(&udf_entry,0,sizeof(udf_entry));


    index = CTC_CLI_GET_ARGC_INDEX("udf-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("udf-id", udf_entry.udf_id, argv[index + 1]);
		udf_entry.query_type = 0;
    }
    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("priority", udf_entry.priority, argv[index + 1]);
		udf_entry.query_type = 1;
    }
    if(g_ctcs_api_en)
    {
       ret = ctcs_acl_get_udf_entry(g_api_lchip, &udf_entry);

    }
    else
    {

       ret = ctc_acl_get_udf_entry(&udf_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("%-13s : %d\n","udf-id", udf_entry.udf_id);
    ctc_cli_out("%-13s : %d\n","priority", udf_entry.priority);
    ctc_cli_out("%-13s : %d\n","valid", udf_entry.valid);
    ctc_cli_out("%-13s : %d\n","tcp_option_en", udf_entry.tcp_option_en);
    ctc_cli_out("%-13s : %d\n","offset_type", udf_entry.offset_type);
    ctc_cli_out("%-13s : %d\n","offset0_type", udf_entry.offset_type_array[0]);
    ctc_cli_out("%-13s : %d\n","offset1_type", udf_entry.offset_type_array[1]);
    ctc_cli_out("%-13s : %d\n","offset2_type", udf_entry.offset_type_array[2]);
    ctc_cli_out("%-13s : %d\n","offset3_type", udf_entry.offset_type_array[3]);
    ctc_cli_out("%-13s : %d\n","offset_num", udf_entry.offset_num);
    for(loop= 0; loop < 4;loop++)
    {
        ctc_cli_out("%s%d%6s : %d\n","offset",loop,"", udf_entry.offset[loop]);
    }
    return ret;
}
CTC_CLI(ctc_cli_usw_acl_show_udf_entry_status,
        ctc_cli_usw_acl_show_udf_entry_status_cmd,
        "show acl udf-entry status (lchip LCHIP|)",
        "Show",
        CTC_CLI_ACL_STR,
        "UDF-Entry",
        "UDF Overall status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_acl_dump_udf_entry_info(lchip);

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_acl_show_league_mode,
        ctc_cli_usw_acl_show_league_mode_cmd,
        "show acl league-mode",
        "Show",
        CTC_CLI_ACL_STR,
        "League mode")
{
    int32 ret = 0;
    uint8 prio = 0;
    uint8 idx = 0;
    ctc_acl_league_t acl_league;

    ctc_cli_out("%-15s%-15s%-15s\n","Dir","Prio","League-Mode");
    ctc_cli_out("-------------------------------------------------------------\n");

    for (prio=0; prio<8; prio++)
    {
        sal_memset(&acl_league, 0, sizeof(ctc_acl_league_t));
        acl_league.acl_priority = prio;
        acl_league.dir = CTC_INGRESS;

        if(g_ctcs_api_en)
        {
            ret = ctcs_acl_get_league_mode(g_api_lchip, &acl_league);
        }
        else
        {
            ret = ctc_acl_get_league_mode(&acl_league);
        }
        if (ret < 0)
        {
            ctc_cli_out("ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-15s%-15u","Ingress",prio);

        for (idx=0; idx<8; idx++)
        {
            ctc_cli_out("%u ",(CTC_IS_BIT_SET(acl_league.lkup_level_bitmap, idx) ? 1: 0));
        }

        ctc_cli_out("\n");
    }

    for (prio=0; prio<3; prio++)
    {
        sal_memset(&acl_league, 0, sizeof(ctc_acl_league_t));
        acl_league.acl_priority = prio;
        acl_league.dir = CTC_EGRESS;

        if(g_ctcs_api_en)
        {
            ret = ctcs_acl_get_league_mode(g_api_lchip, &acl_league);
        }
        else
        {
            ret = ctc_acl_get_league_mode(&acl_league);
        }
        if (ret < 0)
        {
            ctc_cli_out("ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-15s%-15u%","Egress",prio);

        for (idx=0; idx<3; idx++)
        {
            ctc_cli_out("%u ",(CTC_IS_BIT_SET(acl_league.lkup_level_bitmap, idx) ? 1: 0));
        }

        ctc_cli_out("\n");
    }

    return ret;

}

CTC_CLI(ctc_cli_usw_acl_show_entry_distribution,
        ctc_cli_usw_acl_show_entry_distribution_cmd,
        "show acl entry-info distribution priority PRIORITY (ingress | egress) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_ACL_STR,
        "Entry info",
        "Entry distribution",
        "ACL priority",
        "ACL priority value",
        "Ingress",
        "Egress",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    uint8 dir = CTC_INGRESS;
    uint8 prio = 0;

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (index != 0xFF)
    {
        dir = CTC_INGRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (index != 0xFF)
    {
        dir = CTC_EGRESS;
    }

    CTC_CLI_GET_UINT8_RANGE("priority", prio, argv[0], 0, (dir == CTC_INGRESS ? 7: 2));

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_acl_show_entry_distribution(lchip, dir, prio);
    if (ret < 0)
    {
        ctc_cli_out("ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_usw_acl_set_sort_mode,
            ctc_cli_usw_acl_set_sort_mode_cmd,
            "acl enable user-sort-mode (lchip LCHIP|)",
            CTC_CLI_ACL_STR,
            "Enable",
            "Sort by user mode",
            CTC_CLI_LCHIP_ID_STR,
            CTC_CLI_LCHIP_ID_VALUE)
{
    int ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0xFF;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_acl_set_sort_mode(lchip);
    if (ret < 0)
    {
        ctc_cli_out("ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
int32
ctc_usw_acl_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_entry_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_group_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_status_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_acl_set_global_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_field_range_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_udf_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_remove_udf_entry_cmd);
	install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_remove_udf_entry_key_field_cmd);
	install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_udf_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_udf_entry_status_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_key_field_cmd_0);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_key_field_cmd_1);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_action_field_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_remove_action_field_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_hash_entry_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_set_hash_mac_field_sel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_set_hash_ipv4_field_sel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_set_hash_mpls_field_sel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_set_hash_ipv6_field_sel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_set_hash_l2_l3_field_sel_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_cid_pair_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_cid_pair_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_create_group_ext_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_block_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_set_cid_prio_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_league_mode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_entry_distribution_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_acl_set_sort_mode_cmd);
    return 0;
}

int32
ctc_usw_acl_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_entry_info_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_group_info_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_status_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_acl_set_global_cfg_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_field_range_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_udf_entry_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_remove_udf_entry_key_field_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_udf_entry_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_remove_udf_entry_cmd);
	uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_udf_entry_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_key_field_cmd_0);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_key_field_cmd_1);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_action_field_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_remove_action_field_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_entry_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_hash_entry_cmd);

	uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_set_hash_mac_field_sel_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_set_hash_ipv4_field_sel_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_set_hash_mpls_field_sel_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_set_hash_ipv6_field_sel_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_set_hash_l2_l3_field_sel_cmd);


    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_add_cid_pair_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_cid_pair_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_create_group_ext_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_block_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_set_cid_prio_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_acl_show_entry_distribution_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_acl_set_sort_mode_cmd);
    return 0;
}

