/**
 @file ctc_usw_security.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-2-4

 @version v2.0

 \p
 This file contains the API designed to support implementation of security.
 \p
 It is including function as follows:
 \p
 \d    mac security
 \d    ip source guard
 \d    storm control
 \d    port isolation
 \d    private vlan
 \d    blocking based port
 \p
 The mac security will contains two parts: port security and mac limit.
 \p
 Enable or disable port security on a port, referring to API ctc_mac_security_set_port_security(). This function takes
 into account whether the source port enabled the function packet from is mismatch with FDB. If mismatch, then the packet will
 be reported as a security violation event to discard. Otherwise the packet will be forwarding normally.
 \b
 \p
 Enable or disable mac limit on a port, a vlan or system. Based on port, the function will be enabled on port by API
 ctc_mac_security_set_port_mac_limit(). When the function enabled on port, the packet received on port will do action
 as enum ctc_maclimit_action_t defined. Based on vlan, the function will be enabled by API ctc_mac_security_set_vlan_mac_limit().
 When the function enabled on vlan, the packet with the vlan forwarding will do action as enum ctc_maclimit_action_t defined.
 Based on system, the function will be enabled by API ctc_mac_security_set_system_mac_limit(). When the function
 enabled on system, any packet will do action as enum ctc_maclimit_action_t defined.
 \b
 \p
 Enable or disable ip source guard function. This function will takes into account whether the packet parameters including mac
 source address, ip source address, vlan id and their any combination with source port equal to the entry configured by API
 ctc_ip_source_guard_add_entry(). If compared correctly, the packet will be forwarding normally. Otherwise, the packet
 will be reported as a security violation event to discard. Enable or disable the function on port by API ctc_port_set_scl_property()
 with key type relative, tcam default entry should use class id 63.
 \b
 \p
 Storm control is used to limit the rate of bridge packet include unicast and mcast. Usually, unknown unicast and mcast bridge packet rate is
 limited on edge point of network. The Module support port and vlan base storm control and total is 63 controls entry which port and vlan shares
 storm control can limit rate by bps and bps and its grain is 1kbps or pbs. Before use api set storm control limit rate, it's recommend to set global
 configure by ctc_storm_ctl_set_global_cfg().
 \b
 \p
 Port isolation can be configured on port. It will be configured to set isolated group id on port by API ctc_port_set_restriction(),
 The function will take into account whether source port and destination port the packet referred to are in the same isolated group.
 If they are in the same isolated group, the source port and destination port cannot communicate with each other.
 Furthermore, isolation type of packet can be including: unknown ucast, known ucast, unknown mcast, known mcast, bcast. You can set type in struct ctc_port_restriction_t.
 Otherwise, they can speak with each other. If the function can take effect in route forwarding, the API ctc_port_isolation_set_route_obey_isolated_en
 will be configured.
 \b
 \p
 Private VLAN is a security feature that enhances VLAN scalability, IP Address conservation and the security of the switched Layer 2 network.
 Private VLAN architecture is based on the IETF RFC 5517.Private VLAN provides higher degree of Layer 2 isolation between ports.
 An "isolated" port can only talk to the promiscuous port and none else. The community ports can talk to each other of the same community.
 Any communication to an isolated port has to go via the "promiscuous" port.
 \b
 \p
 There are two ways to support PVlan in The Module.
 \pp
 \dd    The first way is introduced in Greatbelt and Goldengate, which is easy to configure and will not consume extra resource. But the limitation is that it can't support trunk port.
        You must configure port type that can be promiscuous, isolated, community and none(normal port) by API ctc_port_set_restriction(). Then, if the port is
        community port, the isolated group id must be set as isolated_id by API ctc_port_set_restriction().
 \dd    The other way is the same with Bay and Humber, which can support trunk port but with more complicated configuration, including that it requires asymmetric vlan,
        egress vlan filter, etc. The second way is using egress vlan filtering to support PVLAN in GreatBelt and Goldengate. But isolated port only support that incoming
        packet is untag packet.
 \b
 \p
 Blocking based on port is a security feature. By default, the switch floods all packets including unknown unicast, unknown multicast, known unicast, known multicast and broadcast. In certain instance
 unknown unicast, unknown multicast, known unicast, known multicast or broadcast traffic is forwarded to a protected port, there could be security issues. To
 prevent unknown unicast, unknown multicast, known unicast, known multicast or broadcast traffic from being forwarded from one port to another, you can use
 API ctc_port_set_restriction(). You can set type as the special traffic prevented from being forwarded from one port to another.
 \p
 \b
 \S ctc_security.h:ctc_maclimit_action_t
 \S ctc_security.h:ctc_security_ip_source_guard_type_t
 \S ctc_security.h:ctc_security_storm_ctl_op_t
 \S ctc_security.h:ctc_security_storm_ctl_type_t
 \S ctc_security.h:ctc_security_storm_ctl_mode_t
 \S ctc_security.h:ctc_security_storm_ctl_granularity_t
 \S ctc_security.h:ctc_security_learn_limit_type_t
 \S ctc_security.h:ctc_security_learn_limit_t
 \S ctc_security.h:ctc_security_ip_source_guard_elem_t
 \S ctc_security.h:ctc_security_stmctl_cfg_t
 \S ctc_security.h:ctc_security_stmctl_glb_cfg_t
*/

#ifndef _CTC_USW_SECURITY_H
#define _CTC_USW_SECURITY_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_security.h"
#include "ctc_l2.h"

/**********************************************************************************
                      Define API function interfaces
***********************************************************************************/
/**
 @addtogroup security SECURITY
 @{
*/

/**
 @brief  Init security module

 @param[in] lchip    local chip id

 @param[in] security_global_cfg   security module global config

 @remark[D2.TM]
      Initialize security feature module, when init, having no need to give global configuration.
      When Initialized, port isolate function will be enabled.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_security_init(uint8 lchip, void* security_global_cfg);

/**
 @brief De-Initialize security module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the security configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_security_deinit(uint8 lchip);

/**
 @brief  Configure port based  security

 @param[in] lchip    local chip id

 @param[in] gport  global port

 @param[in] enable  if set,packet will be discard when srcport is mismatch with FDB

 @remark[D2.TM]
       The API is configuring enable or not function port security on this port.
       If function enable on the port, the packet will be reported as a security violation event to discard
       and not learning this mac address again when the source port is mismatch with FDB .
       The function will take effect, also referred to API ctc_l2_add_fdb_with_nexthop() when
       configure static fdb. The parameter flag will be set CTC_L2_FLAG_BIND_PORT in struct ctc_l2_addr_t.
       By default, port security is enable on port when port module Initialized.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_mac_security_set_port_security(uint8 lchip, uint32 gport, bool enable);

/**
 @brief  Get configure of port based security

 @param[in] lchip    local chip id

 @param[in] gport  global port

 @param[out] p_enable  if set,packet will be discard when srcport is mismatch with FDB

 @remark[D2.TM]
       Get info about whether port security was enable on this port or not.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_mac_security_get_port_security(uint8 lchip, uint32 gport, bool* p_enable);

/**
 @brief Configure port based  mac limit

 @param[in] lchip    local chip id

 @param[in] gport       global port

 @param[in] action      refer to ctc_maclimit_action_t

 @remark[D2.TM]
       The API configure mac limit on a gport.
       When the function enabled on gport, the packet received on port will do action as enum ctc_maclimit_action_t.
       \p
       action type:
       \p
              CTC_MACLIMIT_ACTION_NONE:         packet will forwarding and learning mac if reach mac learning limitation.
              \p
              CTC_MACLIMIT_ACTION_FWD:          packet will forwarding but no learning mac if reach mac learning limitation.
              \p
              CTC_MACLIMIT_ACTION_DISCARD:      packet will discard and no learning mac if reach mac learning limitation.
              \p
              CTC_MACLIMIT_ACTION_TOCPU:        packet will discard and no learning, redirect to cpu if reach mac learning limitation.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mac_security_set_port_mac_limit(uint8 lchip, uint32 gport, ctc_maclimit_action_t action);
/**
 @brief Get the configure of port based  mac limit

 @param[in] lchip    local chip id

 @param[in] gport       global port

 @param[out] action      refer to ctc_maclimit_action_t

 @remark[D2.TM]
        Get info about action of mac limit based on port function on this gport.
 @return CTC_E_XXX
*/
extern int32
ctc_usw_mac_security_get_port_mac_limit(uint8 lchip, uint32 gport, ctc_maclimit_action_t* action);

/**
 @brief Configure vlan based  mac limit

 @param[in] lchip    local chip id

 @param[in] vlan_id     vlan id

 @param[in] action      refer to ctc_maclimit_action_t

 @remark[D2.TM]
       The API configure mac limit on a vlan.
       When the function enabled on this vlan, the packet forwarding as this vlan id will do action as enum ctc_maclimit_action_t.
       \p
       action type:
       \p
              CTC_MACLIMIT_ACTION_NONE:         packet will forwarding and learning mac if reach mac learning limitation.
              \p
              CTC_MACLIMIT_ACTION_FWD:          packet will forwarding but no learning mac if reach mac learning limitation.
              \p
              CTC_MACLIMIT_ACTION_DISCARD:      packet will discard and no learning mac if reach mac learning limitation.
              \p
              CTC_MACLIMIT_ACTION_TOCPU:        packet will discard and no learning, redirect to cpu if reach mac learning limitation.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mac_security_set_vlan_mac_limit(uint8 lchip, uint16 vlan_id, ctc_maclimit_action_t action);

/**
 @brief Get the configure of vlan based  mac limit

 @param[in] lchip    local chip id

 @param[in] vlan_id     vlan id

 @param[out] action     refer to ctc_maclimit_action_t

 @remark[D2.TM]
        Get info about action of mac limit based on vlan function on this vlan.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mac_security_get_vlan_mac_limit(uint8 lchip, uint16 vlan_id, ctc_maclimit_action_t* action);

/**
 @brief Configure system based  mac limit

 @param[in] lchip    local chip id

 @param[in] action      refer to ctc_maclimit_action_t

 @remark[D2.TM]
     The API configure mac limit on system.
     When the function enabled on system, the packet forwarding as this vlan id will do action as enum ctc_maclimit_action_t.
     \p
     action type:
     \p
            CTC_MACLIMIT_ACTION_NONE:         packet will forwarding and learning mac if reach mac learning limitation.
            \p
            CTC_MACLIMIT_ACTION_FWD:          packet will forwarding but no learning mac if reach mac learning limitation.
            \p
            CTC_MACLIMIT_ACTION_DISCARD:      packet will discard and no learning mac if reach mac learning limitation.
            \p
            CTC_MACLIMIT_ACTION_TOCPU:        packet will discard and no learning, redirect to cpu if reach mac learning limitation.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mac_security_set_system_mac_limit(uint8 lchip, ctc_maclimit_action_t action);
/**
 @brief Get the configure of system based  mac limit

 @param[in] lchip    local chip id

 @param[out] action      refer to ctc_maclimit_action_t

 @remark[D2.TM]
        Get info about action of mac limit based on system function on system.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mac_security_get_system_mac_limit(uint8 lchip, ctc_maclimit_action_t* action);

/**
 @brief Configure learn limit

 @param[in] lchip    local chip id

 @param[in] p_learn_limit    refer to ctc_security_learn_limit_t

 @remark[D2.TM]
       The API configure learn limit for a gport or vlan or global system.
       The learn limit type is assigned by type as enum ctc_security_learn_limit_type_t.
       \p
       type:
       \p
            CTC_SECURITY_LEARN_LIMIT_TYPE_PORT:          port based learn limit.
            \p
            CTC_SECURITY_LEARN_LIMIT_TYPE_VLAN:          vlan based learn limit.
            \p
            CTC_SECURITY_LEARN_LIMIT_TYPE_SYSTEM:        system global learn limit.
       When the function enabled on gport, the union gport is valid.
       When the function enabled on vlan, the union vlan_id is valid.
       If learnt entries reach the number defined by limit (0 means disable learn limit),
       the packet received on port/vlan/global system will do action as enum ctc_security_learn_limit_action_t.
       \p
       action type:
       \p
              CTC_SECURITY_LEARN_LIMIT_ACTION_FWD:          packet will forwarding but no learning if reach mac learn limitation.
              \p
              CTC_SECURITY_LEARN_LIMIT_ACTION_DISCARD:      packet will discard and no learning if reach mac learn limitation.
              \p
              CTC_SECURITY_LEARN_LIMIT_ACTION_TOCPU:        packet will discard and no learning, redirect to cpu if reach mac learn limitation.
     Notice: If mac limit function is counted by software, set mac limit number in this interface will be useless.
 @return CTC_E_XXX
*/
extern int32
ctc_usw_mac_security_set_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learn_limit);

/**
 @brief Get the configure learn limit

 @param[in] lchip    local chip id

 @param[in|out] p_learn_limit    refer to ctc_security_learn_limit_t

 @remark[D2.TM]
       Get info about configuration and running counter of learn limit.
 @return CTC_E_XXX
*/
extern int32
ctc_usw_mac_security_get_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learn_limit);


/**
 @brief  Add ip source guard entry

 @param[in] lchip    local chip id

 @param[in] p_elem  element which bind to port

 @remark[D2.TM]
     This API can add ip source guard configuration.
     6 kind of ip source guard check mode is support in GG. The mode can be configured by parameter ip_src_guard_type.
     \p
     CTC_SECURITY_BINDING_TYPE_IP:                 ipv4_sa and gport must be configured.
     \p
     CTC_SECURITY_BINDING_TYPE_IP_VLAN:            ipv4_sa and vid and gport must be configured.
     \p
     CTC_SECURITY_BINDING_TYPE_IP_MAC:             ipv4_sa and mac_sa and gport must be configured.
     \p
     CTC_SECURITY_BINDING_TYPE_IP_MAC_VLAN:        ipv4_sa and mac_sa and vid and gport must be configured.
     \p
     CTC_SECURITY_BINDING_TYPE_MAC:                mac_sa and gport must be configured.
     \p
     CTC_SECURITY_BINDING_TYPE_MAC_VLAN:           mac_sa and vid and gport must be configured.
     \p
     And you must enable ip source guard function on port by API ctc_port_set_scl_property(), according to ip source guard check mode.
     You can see enum ctc_port_igs_scl_hash_type_t note(CTC_PORT_IGS_SCL_HASH_TYPE_PORT_MAC_SA and CTC_PORT_IGS_SCL_HASH_TYPE_PORT_IP_SA).

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ip_source_guard_add_entry(uint8 lchip, ctc_security_ip_source_guard_elem_t* p_elem);

/**
 @brief  Remove ip source guard entry

 @param[in] lchip    local chip id

 @param[in] p_elem  element which bind to port

 @remark[D2.TM]
     This API can remove ip source guard configuration.
     6 kind of ip source guard check mode is support in GG. The mode can be configured by parameter ip_src_guard_type.
     \p
     CTC_SECURITY_BINDING_TYPE_IP:                 ipv4_sa and gport must be configured.
     \p
     CTC_SECURITY_BINDING_TYPE_IP_VLAN:            ipv4_sa and vid and gport must be configured.
     \p
     CTC_SECURITY_BINDING_TYPE_IP_MAC:             ipv4_sa and mac_sa and gport must be configured.
     \p
     CTC_SECURITY_BINDING_TYPE_IP_MAC_VLAN:        ipv4_sa and mac_sa and vid and gport must be configured.
     \p
     CTC_SECURITY_BINDING_TYPE_MAC:                mac_sa and gport must be configured.
     \p
     CTC_SECURITY_BINDING_TYPE_MAC_VLAN:           mac_sa and vid and gport must be configured.
     \p
     If you want to disable ip source guard function on the gport, it must be set disable by API ctc_port_set_scl_property().
     You can see enum ctc_port_igs_scl_hash_type_t note(CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE).

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ip_source_guard_remove_entry(uint8 lchip, ctc_security_ip_source_guard_elem_t* p_elem);

/**
 @brief  Set configure of storm ctl

 @param[in] lchip    local chip id

 @param[in] stmctl_cfg   configure of storm ctl

 @remark[D2.TM]
  Storm control can support port base or vlan base refer to ctc_security_storm_ctl_op_t
  can control packets such as:
  \p
  CTC_SECURITY_STORM_CTL_BCAST        : broadcast storm control.
  \p
  CTC_SECURITY_STORM_CTL_KNOWN_MCAST  : known multicast storm control.
  \p
  CTC_SECURITY_STORM_CTL_UNKNOWN_MCAST: unknown multicast storm control.
  \p
  CTC_SECURITY_STORM_CTL_ALL_MCAST    : known and unknown multicast storm control.
  \p
  CTC_SECURITY_STORM_CTL_KNOWN_UCAST  : known unicast storm control.
  \p
  CTC_SECURITY_STORM_CTL_UNKNOWN_UCAST: unknown unicast storm control.
  \p
  CTC_SECURITY_STORM_CTL_ALL_UCAST    : known and unknown unicast storm control.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_storm_ctl_set_cfg(uint8 lchip, ctc_security_stmctl_cfg_t* stmctl_cfg);

/**
 @brief  Get configure of storm ctl

 @param[in] lchip    local chip id

 @param[in|out] stmctl_cfg  configure of storm ctl

 @remark[D2.TM]
  Get storm contrl configure of type and mode.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_storm_ctl_get_cfg(uint8 lchip, ctc_security_stmctl_cfg_t* stmctl_cfg);

/**
 @brief  Set global configure of storm ctl

 @param[in] lchip    local chip id

 @param[in] p_glb_cfg  global configure of storm ctl

 @remark[D2.TM]
  Set configure ipg, known or unknown unicast,mcast mode.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_storm_ctl_set_global_cfg(uint8 lchip, ctc_security_stmctl_glb_cfg_t* p_glb_cfg);

/**
 @brief  Get global configure of storm ctl

 @param[in] lchip    local chip id

 @param[out] p_glb_cfg  global configure of storm ctl

 @remark[D2.TM]
  Get configure ipg, known or unknown unicast,mcast mode.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_storm_ctl_get_global_cfg(uint8 lchip, ctc_security_stmctl_glb_cfg_t* p_glb_cfg);

/**
 @brief  Set route is obey isolated enable

 @param[in] lchip    local chip id

 @param[in] enable  a boolean value denote route-obey-iso is enable

 @remark[D2.TM]
     If set enable, packet forwarding by route can obey isolation.

 @return CTC_E_XXX


*/
extern int32
ctc_usw_port_isolation_set_route_obey_isolated_en(uint8 lchip, bool enable);

/**
 @brief  Get route is obey isolated enable

 @param[in] lchip    local chip id

 @param[out] p_enable  a boolean value denote route-obey-iso is enable

 @remark[D2.TM]
     Get info whether packet forwarding by route obey isolation or not.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_port_isolation_get_route_obey_isolated_en(uint8 lchip, bool* p_enable);


#ifdef __cplusplus
}
#endif

#endif

