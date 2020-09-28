#ifndef __HAGT_FLOW_H__
#define __HAGT_FLOW_H__   

#include "glb_flow_define.h"

extern int32
hagt_flow_map_mac_key(HalMsgFlowMacRule_t* p_from_mac, ctc_acl_mac_key_t* p_to_mac);
/*add by chenzh for bug 15103, 2011-07-02*/
extern int32
hagt_flow_map_pbr_ipv4_key(HalMsgFlowIpv4Rule_t* p_from_ipv4, ctc_acl_pbr_ipv4_key_t* p_to_ipv4);
extern int32
hagt_flow_map_pbr_ipv6_key(HalMsgFlowIpv6Rule_t* p_from_ipv6, ctc_acl_pbr_ipv6_key_t* p_to_ipv6);
extern int32
hagt_flow_map_ipv4_key(HalMsgFlowIpv4Rule_t* p_from_ipv4, ctc_acl_ipv4_key_t* p_to_ipv4);
extern int32
hagt_flow_map_ipv6_key(HalMsgFlowIpv6Rule_t* p_from_ipv6, ctc_acl_ipv6_key_t* p_to_ipv6);
extern int32
hagt_flow_map_key_mac2ipv4(HalMsgFlowMacRule_t* p_from_mac, ctc_acl_ipv4_key_t* p_to_ipv4, bool  mac_acl);
extern int32
hagt_flow_map_key_mac2ipv6(HalMsgFlowMacRule_t* p_from_mac, ctc_acl_ipv6_key_t* p_to_ipv6);
extern int32
hagt_flow_map_key_mac2mpls(HalMsgFlowMacRule_t* p_from_mac, ctc_acl_mpls_key_t* p_to_mpls);
extern int32
hagt_flow_map_rule(HalMsgFlowRule_t* p_hagt_rule,
                   ctc_acl_key_t* p_ctc_key);
extern int32
hagt_flow_map_rule2(HalMsgFlowRule_t* p_from_flow,
                   ctc_acl_key_t* p_to_ctc_key,
                   ctc_acl_key_t* p_to_ctc_key2,
                   uint8* has_ext);
/*add by chenzh for bug 15103, pbr develop, 2011-07-02*/
extern int32
hagt_flow_map_pbr_rule(HalMsgFlowRule_t* p_hagt_rule, ctc_acl_key_t* p_ctc_key);
extern int32
hagt_flow_map_rule_type(uint32 hagt_type, ctc_acl_key_type_t* p_ctc_type);
#endif
