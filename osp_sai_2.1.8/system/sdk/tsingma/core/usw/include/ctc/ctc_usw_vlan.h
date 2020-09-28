/**
 @file ctc_usw_vlan.h

 @date 2009-10-17

 @version v2.0

This chapter describes VLAN implementation, including basic vlan, vlan classification and vlan mapping function.
 \d Basic vlan
 \d Vlan classification
 \d Vlan mapping

 \b
 \B Basic vlan
 \p
Basic VLAN configuration allows for the creation and destruction of 802.1Q VLAN definitions. This allows for
binding an 802.1Q VLAN ID to a set of ports and managing the member ports. The API maintains
consistency between trunking, VLAN configuration and port-based VLAN configuration.
The Module support convenient way to create vlan by ctc_vlan_create_uservlan() which will automatically create vlan default entry.
Users don't need call API ctc_l2_add_default_entry() again.
To be compatible with old chip, SDK also support old way by ctc_vlan_create_vlan()
This different mode can be set when vlan init. If mode is CTC_VLANPTR_MODE_VLANID, it is old way, otherwise it is new way.
If vlan mode is CTC_VLANPTR_MODE_VLANID, when vlan is create, receive and transmit of the vlan is enabled,
and fid is equal to vlan_id. If vlan mode is CTC_VLANPTR_MODE_USER_DEFINE1, fid will be the value you set.

 \b
 \B Vlan classfication
 \p
 The VLAN APIs can support vlan class by:
 \pp
 \dd    policy--flex in vlan class structure;
 \dd    mac--macsa or macda;
 \dd    ip--ipv4sa or ipv6sa;
 \dd    protocol--ether-type;
 \dd    port based vlan class, it can be supported by default vlan configuration on port.
 \pp
 Vlan classification priority: mac/ipv4/ipv6/policy > protocol > port(default).
 \b
 \pp
 Vlan class function refer to ctc_vlan_add_vlan_class() and ctc_vlan_remove_vlan_class().
 \pp
 And it can configure default vlan class action if mismatch any entry by API ctc_vlan_add_default_vlan_class().

 \b
 \B Vlan mapping
 \p
 VLAN APIs can support vlan mapping in direction ingress or egress.
 Ingress vlan mapping support the following application:
 \pp
 \dd    QinQ
 \dd    vlan translation
 \dd    vlan stacking
 \dd    vlan switching
 \pp
 The ingress vlan mapping function refer to APIs ctc_vlan_add_vlan_mapping() and ctc_vlan_update_vlan_mapping(), ctc_vlan_remove_vlan_mapping().
 \pp
 And it can configure default ingress vlan mapping action if mismatch any entry by ctc_vlan_add_default_vlan_mapping().
 \b
 \pp
 VLAN APIs can support vlan tag(s or c) /vlan id /cos action type in ingress vlan mapping as following:
 \p
 \t  |   stag action(svid:scos)  |     NONE    |  REP_OR_ADD |     APPEND   |     DEL     |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    none:none   |      1      |      1      |       1      |      1      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    none:swap   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    none:new    |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  none:default  |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    swap:none   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    swap:swap   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    swap:new    |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  swap:default  |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |     new:none   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |     new:swap   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |     new:new    |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |   new:default  |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  default:none  |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  default:swap  |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  default:new   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |default:default |      0      |      1      |       1      |      0      |
 \p
 \p
 \t  |   ctag action(cvid:ccos)  |     NONE    |  REP_OR_ADD |     APPEND   |     DEL     |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    none:none   |      1      |      1      |       1      |      1      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    none:swap   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    none:new    |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  none:default  |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    swap:none   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    swap:swap   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    swap:new    |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  swap:default  |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |     new:none   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |     new:swap   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |     new:new    |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |   new:default  |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  default:none  |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  default:swap  |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  default:new   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |default:default |      0      |      1      |       1      |      0      |
 \b
 \pp
 Egress vlan mapping support the following application: QinQ, vlan translation. The egress vlan mapping function refer to add/remove entry by API
 ctc_vlan_add_egress_vlan_mapping() and ctc_vlan_remove_egress_vlan_mapping(). And it can configure default egress vlan
 mapping action if mismatch any entry, by ctc_vlan_add_default_egress_vlan_mapping().
 \b
 \pp
 VLAN APIs can support vlan tag(s or c) /vlan id /cos action type in egress vlan mapping as following:
 \p
 \t  |   stag action(svid:scos)  |     NONE    |  REP_OR_ADD |     APPEND   |     DEL     |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    none:none   |      1      |      1      |       1      |      1      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    none:swap   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    none:new    |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  none:mapped   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    swap:none   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    swap:swap   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    swap:new    |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  swap:mapped   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |     new:none   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |     new:swap   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |     new:new    |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |   new:mapped   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  default:none  |      0      |      0      |       0      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  default:swap  |      0      |      0      |       0      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  default:new   |      0      |      0      |       0      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  | default:mapped |      0      |      0      |       0      |      0      |
 \p
 \p
 \t  |   ctag action(cvid:ccos)  |     NONE    |  REP_OR_ADD |     APPEND   |     DEL     |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    none:none   |      1      |      1      |       1      |      1      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    none:swap   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    none:new    |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  none:mapped   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    swap:none   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    swap:swap   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |    swap:new    |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  swap:mapped   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |     new:none   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |     new:swap   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |     new:new    |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |   new:mapped   |      0      |      1      |       1      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  default:none  |      0      |      0      |       0      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  default:swap  |      0      |      0      |       0      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  |  default:new   |      0      |      0      |       0      |      0      |
 \t  |----------------|-------------|-------------|------------- |-------------|
 \t  | default:mapped |      0      |      0      |       0      |      0      |


\S ctc_vlan.h:ctc_vlan_class_type_t
\S ctc_vlan.h:ctc_vlan_class_t
\S ctc_vlan.h:ctc_vlan_mapping_key_t
\S ctc_vlan.h:ctc_vlan_mapping_t
\S ctc_vlan.h:ctc_egress_vlan_mapping_key_t
\S ctc_vlan.h:ctc_egress_vlan_mapping_t

*/

#ifndef _CTC_USW_VLAN_H
#define _CTC_USW_VLAN_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_vlan.h"
#include "ctc_const.h"
#include "ctc_parser.h"
#include "ctc_stats.h"
/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
/**
 @addtogroup vlan VLAN
 @{
*/

/**
 @brief Init the vlan module

 @param[in] lchip    local chip id

 @param[in] vlan_global_cfg         vlan module global config

 @remark[D2.TM]
        Initialize device tables and driver structures for VLAN management functions:
        \p
        (1)basic vlan
        \p
        (2)adv-vlan: vlan class and vlan mapping.
        \p
        By default, all vlan transmit and receive is disable except vlan 0.
        Arp packet and dhcp packet is forward.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_vlan_init(uint8 lchip, ctc_vlan_global_cfg_t* vlan_global_cfg);

/**
 @brief De-Initialize vlan module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the vlan configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_deinit(uint8 lchip);

/**
 @brief  Create a vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @remark[D2.TM]
        Allocate and configure a VLAN on the Centec device in vlanptr_mode = CTC_VLANPTR_MODE_VLANID.
        Create a new VLAN with the given vlan_id. Then, vlanptr, vlanid, vsi is the same value.
        By default, transmit and receive of this vlan is enable, and fid is equal to vlan_id.
        Arp packet and dhcp packet is forward.
        0~127 port is tagged for this VLAN.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_vlan_create_vlan(uint8 lchip, uint16 vlan_id);

/**
 @brief  Create a user vlan

 @param[in] lchip    local chip id

 @param[in] user_vlan         user vlan info

 @remark[D2.TM]
        Allocate and configure a VLAN on the Centec device in vlanptr_mode = CTC_VLANPTR_MODE_USER_DEFINE1.
        Create a new VLAN with the given vlan_id, given vlanptr and given fid(vsi).
        By default, transmit and receive of this vlan is enable.
        Arp packet and dhcp packet is forward.
        0~127 port is tagged for this VLAN.
        L2 flooding entry added by default, fid is l2mc group id.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_vlan_create_uservlan(uint8 lchip, ctc_vlan_uservlan_t* user_vlan);


/**
 @brief  Remove the vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @remark[D2.TM]
        To deallocate the VLAN previously created, API ctc_vlan_destroy_vlan must be used.
        By default, this vlan transmit and receive is disable.
        Arp packet and dhcp packet is forward.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_vlan_destroy_vlan(uint8 lchip, uint16 vlan_id);

/**
 @brief  Add member ports to a vlan

 @param[in] lchip_id    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] port_bitmap     bits map of port in vlan

 @remark[D2.TM]
        Adds the selected ports to the VLAN. At the same time allow vlan on the specify port.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_add_ports(uint8 lchip_id, uint16 vlan_id, ctc_port_bitmap_t port_bitmap);

/**
 @brief  Add member port to a vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] gport     Global port of the local chip

 @remark[D2.TM]
        Adds the selected port to the VLAN. At the same time allow vlan on the specify port.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_add_port(uint8 lchip, uint16 vlan_id, uint32 gport);

/**
 @brief  Show vlan's member port

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] gchip         global chip id

 @param[out] port_bitmap    bits map of port in vlan

 @remark[D2.TM]
        Get info about whether ports are added in the given vlan_id VLAN.
        *port_bitmap is the bitmap of 0~127 port in this vlan_id.
        If the bit of the port is set, it means that the port is added in the vlan;
        otherwise, the port is not added.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_vlan_get_ports(uint8 lchip, uint16 vlan_id, uint8 gchip, ctc_port_bitmap_t port_bitmap);

/**
 @brief  Remove member port from a vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] gport           Global port of the local chip

 @remark[D2.TM]
        Removes the specified ports from the given VLAN. If the requested ports is not already
        members of the VLAN, the routine returns CTC_E_MEMBER_PORT_NOT_EXIST.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_remove_port(uint8 lchip, uint16 vlan_id, uint32 gport);

/**
 @brief  Remove member ports from a vlan

 @param[in] lchip_id    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] port_bitmap          bits map of port in vlan

 @remark[D2.TM]
        Removes the specified ports from the given VLAN. If the requested ports is not already
        members of the VLAN, the routine returns CTC_E_MEMBER_PORT_NOT_EXIST.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_remove_ports(uint8 lchip_id, uint16 vlan_id, ctc_port_bitmap_t port_bitmap);

/**
 @brief  Set tagged port on vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] gport           Global port of the local chip

 @param[in] tagged          Vlan allow tagged on this port

 @remark[D2.TM]
   Set whether the vlan is tagged or not when output from the specify port, mainly used on hyrpid port

 @return CTC_E_XXX

*/
extern int32
ctc_usw_vlan_set_tagged_port(uint8 lchip, uint16 vlan_id, uint32 gport, bool tagged);
/**
 @brief  Set tagged ports on vlan

 @param[in] lchip_id    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] port_bitmap          bits map of port in vlan

 @remark[D2.TM]
   Set whether the vlan is tagged or not when output from the specify port, mainly used on hyrpid port

 @return CTC_E_XXX

*/
extern int32
ctc_usw_vlan_set_tagged_ports(uint8 lchip_id, uint16 vlan_id, ctc_port_bitmap_t port_bitmap);
/**
 @brief  Get tagged port on vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] gchip         global chip id

 @param[out] port_bitmap     Bitmap of port tagged or not in this vlan

 @remark[D2.TM]
        Get the ports from which vlan is tagged or not.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_vlan_get_tagged_ports(uint8 lchip, uint16 vlan_id, uint8 gchip, ctc_port_bitmap_t port_bitmap);

/**
 @brief  Set receive enable on vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] enable          A boolean value denote receive is enable

 @remark[D2.TM]
        If enable is set TRUE, receive packet will be enable in this vlan_id.
        Otherwise, it is disable in this vlan_id.
        When a vlan is created, receive is enable in the vlan by default.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_vlan_set_receive_en(uint8 lchip, uint16 vlan_id, bool enable);

/**
 @brief  Get receive on vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[out] enable          A boolean value denote receive is enable

 @remark[D2.TM]

        Get info about whether receive is enable or not in this vlan.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_get_receive_en(uint8 lchip, uint16 vlan_id, bool* enable);

/**
 @brief  Set tranmit enable on vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] enable          A boolean value denote transmit is enable

 @remark[D2.TM]
        If enable is set TRUE, transmit packet will be enable in this vlan_id.
        Otherwise, it is disable in this vlan_id.
        When a vlan is created, transmit is enable in the vlan by default.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_vlan_set_transmit_en(uint8 lchip, uint16 vlan_id, bool enable);

/**
 @brief  Get tranmit on vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[out] enable          A boolean value denote transmit is enable

 @remark[D2.TM]
        Get info about whether transmit is enable or not in this vlan.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_get_transmit_en(uint8 lchip, uint16 vlan_id, bool* enable);

/**
 @brief  Set bridge enable on vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] enable          A boolean value denote bridge is enable

 @remark[D2.TM]
        If set TRUE, bridging is enabled on this VLAN.
        Otherwise, it is disable in this vlan_id.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_set_bridge_en(uint8 lchip, uint16 vlan_id, bool enable);

/**
 @brief  Get bridge on vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[out] enable          A boolean value denote bridge is enable

 @remark[D2.TM]

        Get info about whether bridge is enable or not in this vlan.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_get_bridge_en(uint8 lchip, uint16 vlan_id, bool* enable);

/**
 @brief Set fid of vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] fid             Fid used in bridge

 @remark[D2.TM]
     VLAN mapping to FID in 802.1D VLAN bridge
     \p
     1. One to one mapping of VLAN ID and FID in IVL bridge ,(VLAN:FID = 1:1)
     \p
     2. many to one mapping of VLAN ID and FID in SVL bridge

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_set_fid(uint8 lchip, uint16 vlan_id, uint16 fid);

/**
 @brief Get fid of vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[out] fid            Fid used in bridge

 @remark[D2.TM]
        Get info about fid of the VLAN given used in bridge

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_get_fid(uint8 lchip, uint16 vlan_id, uint16* fid);

/**
 @brief Set vlan base MAC authentication enable

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] enable     enable vlan mac authen

 @remark[D2.TM]  Set vlan mac authen enable. Used with CTC_L2_DFT_VLAN_FLAG_VLAN_MAC_AUTH.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_vlan_set_mac_auth(uint8 lchip, uint16 vlan_id, bool enable);

/**
 @brief Get vlan base MAC authentication

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[out] enable   vlan mac authentication enable

 @remark[D2.TM]  Get vlan mac authentication. Used with CTC_L2_DFT_VLAN_FLAG_VLAN_MAC_AUTH.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_vlan_get_mac_auth(uint8 lchip, uint16 vlan_id, bool* enable);

/**
 @brief Set mac learning enable/disable on the vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] enable          A boolean val denote learning enable/disable

 @remark[D2.TM]
        If set TRUE, learning in this vlan is enabled.
        Otherwise, it is disabled in this vlan.
        When a vlan is created, learning is enable in this vlan_id by default.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_set_learning_en(uint8 lchip, uint16 vlan_id, bool enable);

/**
 @brief Get mac learning enable/disable on the vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[out] enable         A boolean val denote learning enable/disable

 @remark[D2.TM]

        Get info about whether learning is enabled or not in this vlan.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_get_learning_en(uint8 lchip, uint16 vlan_id, bool* enable);

/**
 @brief  Set igmp snooping enable on the vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] enable         A boolean val denote learning enable/disable

 @remark[D2.TM]
        If set TRUE, igmp snooping in this vlan is enabled.
        Otherwise, it is disabled in this vlan.
        When a vlan is created, igmp snooping is disable in this vlan_id by default.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_set_igmp_snoop_en(uint8 lchip, uint16 vlan_id, bool enable);

/**
 @brief  Get igmp snooping enable of the vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[out] enable         A boolean val denote learning enable/disable

 @remark[D2.TM]
        Get info about whether igmp snooping is enabled or not in this vlan.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_get_igmp_snoop_en(uint8 lchip, uint16 vlan_id, bool* enable);

/**
 @brief  Set dhcp exception action of the vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] type            exception type, refer to data structure of ctc_exception_type_s

 @remark[D2.TM]
        Dhcp packet forwarding with this vlan_id will do action just as enum ctc_exception_type_t.
        When a vlan created, dhcp packet will be forward by default.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_set_dhcp_excp_type(uint8 lchip, uint16 vlan_id, ctc_exception_type_t type);

/**
 @brief  Get dhcp exception action of the vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[out] type            exception type, refer to data structure of ctc_exception_type_s

 @remark[D2.TM]
        Get info about which mode of action will dhcp packet forwarding with this vlan_id do.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_get_dhcp_excp_type(uint8 lchip, uint16 vlan_id, ctc_exception_type_t* type);

/**
 @brief  Set arp exception action of the vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] type            exception type, refer to data structure of ctc_exception_type_s

 @remark[D2.TM]
        Arp packet forwarding with this vlan_id will do action just as enum ctc_exception_type_t.
        When a vlan created, arp packet will beforward by default.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_set_arp_excp_type(uint8 lchip, uint16 vlan_id, ctc_exception_type_t type);

/**
 @brief  Get arp exception action of the vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[out] type            exception type, refer to data structure of ctc_exception_type_s

 @remark[D2.TM]
        Get info about which mode of action will arp packet forwarding with this vlan_id do.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_get_arp_excp_type(uint8 lchip, uint16 vlan_id, ctc_exception_type_t* type);

/**
 @brief  Set various properties of the vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id/user_vlanptr

 @param[in] vlan_prop       vlan property

 @param[in] value           value need set

 @remark[D2.TM]
        The parameter vlan_id is vlanid in vlanptr_mode = CTC_VLANPTR_MODE_VLANID;
        \p
        The parameter vlan_id is vlanptr in vlanptr_mode = CTC_VLANPTR_MODE_USER_DEFINE1.
        \p
        Set property configuration as enum ctc_vlan_property_t with the uint32 value in the VLAN given.
        \p
        vlan_prop:
        \d CTC_VLAN_PROP_RECEIVE_EN:                vlan receive enabel.
        \d CTC_VLAN_PROP_BRIDGE_EN:                 vlan bridge enabel.
        \d CTC_VLAN_PROP_TRANSMIT_EN:              vlan transmit enabel.
        \d CTC_VLAN_PROP_LEARNING_EN:               vlan learning enabel.
        \d CTC_VLAN_PROP_DHCP_EXCP_TYPE:            dhcp packet processing type.
        \d CTC_VLAN_PROP_ARP_EXCP_TYPE:             arp packet processing type.
        \d CTC_VLAN_PROP_IGMP_SNOOP_EN:             igmp snooping enable.
        \d CTC_VLAN_PROP_FID:                       fid for general l2 bridge, VSIID for L2VPN.
        \d CTC_VLAN_PROP_IPV4_BASED_L2MC:           Ipv4 based l2mc.
        \d CTC_VLAN_PROP_IPV6_BASED_L2MC:           Ipv6 based l2mc.
        \d CTC_VLAN_PROP_DROP_UNKNOWN_UCAST_EN:     vlan drop unknown ucast packet enable.
        \d CTC_VLAN_PROP_DROP_UNKNOWN_MCAST_EN:     vlan drop unknown mcast packet enable.
        \d CTC_VLAN_PROP_DROP_UNKNOWN_BCAST_EN:     vlan drop unknown bcast packet enable.
        \p
        When a vlan is created, by default, unknown ucast packet, unknown mcast packet and unknown bcast packet forwarding with this vlan_id is not to dropped.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_set_property(uint8 lchip, uint16 vlan_id, ctc_vlan_property_t vlan_prop, uint32 value);

/**
 @brief  Set various properties of the vlan given at a time

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id/user_vlanptr

 @param[in] vlan_prop       vlan properties array

 @param[in|out] array_num    num of vlan properties

 @remark[D2.TM]
        The parameter vlan_id is vlanid in vlanptr_mode = CTC_VLANPTR_MODE_VLANID;
        \p
        The parameter vlan_id is vlanptr in vlanptr_mode = CTC_VLANPTR_MODE_USER_DEFINE1.
        \p
        Set some properties configuration as enum ctc_vlan_property_t with the uint32 value in the VLAN given.
        \p
        vlan_prop:
        \d CTC_VLAN_PROP_RECEIVE_EN:                vlan receive enabel.
        \d CTC_VLAN_PROP_BRIDGE_EN:                 vlan bridge enabel.
        \d CTC_VLAN_PROP_TRANSMIT_EN:              vlan transmit enabel.
        \d CTC_VLAN_PROP_LEARNING_EN:               vlan learning enabel.
        \d CTC_VLAN_PROP_DHCP_EXCP_TYPE:            dhcp packet processing type.
        \d CTC_VLAN_PROP_ARP_EXCP_TYPE:             arp packet processing type.
        \d CTC_VLAN_PROP_IGMP_SNOOP_EN:             igmp snooping enable.
        \d CTC_VLAN_PROP_FID:                       fid for general l2 bridge, VSIID for L2VPN.
        \d CTC_VLAN_PROP_IPV4_BASED_L2MC:           Ipv4 based l2mc.
        \d CTC_VLAN_PROP_IPV6_BASED_L2MC:           Ipv6 based l2mc.
        \d CTC_VLAN_PROP_DROP_UNKNOWN_UCAST_EN:     vlan drop unknown ucast packet enable.
        \d CTC_VLAN_PROP_DROP_UNKNOWN_MCAST_EN:     vlan drop unknown mcast packet enable.
        \d CTC_VLAN_PROP_DROP_UNKNOWN_BCAST_EN:     vlan drop unknown bcast packet enable.
        \p
        When a vlan is created, by default, unknown ucast packet, unknown mcast packet and unknown bcast packet forwarding with this vlan_id is not to dropped.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_set_property_array(uint8 lchip, uint16 vlan_id, ctc_property_array_t* vlan_prop, uint16* array_num);

/**
 @brief  Get various properties of the vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id/user_vlanptr

 @param[in] vlan_prop       vlan property

 @param[out] value           value need get

 @remark[D2.TM]
        Get info about property configuration of this vlan.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_get_property(uint8 lchip, uint16 vlan_id, ctc_vlan_property_t vlan_prop, uint32* value);


/**
 @brief  Set various properties of the vlan with direction

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id/user_vlanptr

 @param[in] vlan_prop       vlan property with direction

 @param[in] dir             direction

 @param[in] value           set_value

 @remark[D2.TM]
        Set property configuration as enum ctc_vlan_direction_property_t with the uint32 value in the VLAN given.
        \p
        vlan_prop:
            \d CTC_VLAN_DIR_PROP_ACL_EN:                vlan acl enable, refer CTC_ACL_EN_XXX
            \d CTC_VLAN_DIR_PROP_ACL_CLASSID:           vlan classid
            \d CTC_VLAN_DIR_PROP_ACL_ROUTED_PKT_ONLY:   vlan routed packet only
            \d CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_0:   acl tcam lookup type.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_set_direction_property(uint8 lchip, uint16 vlan_id, ctc_vlan_direction_property_t vlan_prop, ctc_direction_t dir, uint32 value);

/**
 @brief  Get various properties of the vlan with direction

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id/user_vlanptr

 @param[in] vlan_prop       vlan property with direction

 @param[in] dir             direction

 @param[out] value           get_value

 @remark[D2.TM]

        Get info about property configuration of this vlan.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_get_direction_property(uint8 lchip, uint16 vlan_id, ctc_vlan_direction_property_t vlan_prop, ctc_direction_t dir, uint32* value);

/**
 @brief Set acl properties of the vlan

 @param[in] lchip            local chip id

 @param[in] vlan_id          802.1q vlan id/user_vlanptr

 @param[in] p_prop             acl property of vlan

 @remark[D2.TM]
       Set acl properties configuration of this vlan.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_set_acl_property(uint8 lchip, uint16 vlan_id, ctc_acl_property_t* p_prop);

/**
 @brief Get acl properties of the vlan

 @param[in] lchip            local chip id

 @param[in] vlan_id          802.1q vlan id/user_vlanptr

 @param[out] p_prop            acl property of vlan

 @remark[D2.TM]
       Get acl properties configuration of this vlan.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_get_acl_property(uint8 lchip, uint16 vlan_id, ctc_acl_property_t* p_prop);

/**
 @addtogroup VlanClassification VlanClassification
 @{
*/

/**
 @brief  Add one vlan classification rule

 @param[in] lchip    local chip id

 @param[in] p_vlan_class    Vlan classification info, include key and policy

 @remark[D2.TM]
     Add an association from MAC, IPV4 subnet, IPV6 subnet, POLICY or PROTOCOL to use for assigning a vlan tag to untag or priority packet.
     Type in p_vlan_class will be the mode of vlan class:
     \p
     1. vlan class by policy(packet feature) according to :
     \pp
     (1) flex_ipv4:ipv4sa|ipv4da|macsa|macda|l3-type|l4-type|l4-src-port|l4-dest-port
     \pp
     (2) flex_ipv6:ipv6sa|ipv6da|macsa|macda|l4-type|l4-src-port|l4-dest-port
     if policy used, set flag CTC_VLAN_CLASS_FLAG_USE_FLEX.
     \p
     2. vlan class by mac according to :
     \pp
     mac:macsa or macda (if macda, set flag CTC_VLAN_CLASS_FLAG_USE_MACDA)
     \p
     3. vlan class by ip according to :
     \pp
     (1) ipv4:ipv4_sa
     \pp
     (2) ipv6:ipv6_sa
     \p
     4. vlan class by protocol according to :
     ether-type: the ether type is a 16-bit value that matches the ethertype field in the received packet.
     \p
     \b
     Further more, PRI: mac/ipv4/ipv6/policy > protocol > port(default).
     You can configure vlan_id as the new vlan id. If output new cos, flag CTC_VLAN_CLASS_FLAG_OUTPUT_COS must be set.
     Enable the function on port by API ctc_port_set_scl_property() with key type relative, tcam default entry should use class id 62.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_add_vlan_class(uint8 lchip, ctc_vlan_class_t* p_vlan_class);

/**
 @brief Remove on vlan classification rule

 @param[in] lchip    local chip id

 @param[in] p_vlan_class    Vlan classification info, include key and policy

 @remark[D2.TM]
     Remove an association from MAC, IPV4 subnet, IPV6 subnet, POLICY or PROTOCOL to vlan.
     Type in p_vlan_class will be the mode of vlan class:
     \p
     1. vlan class by policy(packet feature) according to :
     \pp
     (1) flex_ipv4:ipv4sa|ipv4da|macsa|macda|l3-type|l4-type|l4-src-port|l4-dest-port
     \pp
     (2) flex_ipv6:ipv6sa|ipv6da|macsa|macda|l4-type|l4-src-port|l4-dest-port
     if policy used, set flag CTC_VLAN_CLASS_FLAG_USE_FLEX.
     \p
     2. vlan class by mac according to :
     \pp
     mac:macsa or macda (if macda, set flag CTC_VLAN_CLASS_FLAG_USE_MACDA)
     \p
     3. vlan class by ip according to :
     \pp
     (1) ipv4:ipv4_sa
     \pp
     (2) ipv6:ipv6_sa
     \p
     4. vlan class by protocol according to :
     \pp
     ether-type: the ether type is a 16-bit value that matches the ethertype field in the received packet.
     \p
     Disable the function on port by API ctc_port_set_scl_property() with key type relative.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_remove_vlan_class(uint8 lchip, ctc_vlan_class_t* p_vlan_class);

/**
 @brief Flush vlan classification by type

 @param[in] lchip    local chip id

 @param[in] type    Vlan classification type, like mac-based or ip-based.

 @remark[D2.TM]
     Remove all associations from MAC, IPV4, IPV6 or PROTOCOL to vlan for vlan class function
     (1)Remove all associations from IPV4 addresses to VLAN.
     (2)Remove all associations from IPV6 addresses to VLAN.
     (3)Remove all associations from MAC addresses to VLAN.
     (4)Remove all associations from PROTOCOL addresses to VLAN

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_remove_all_vlan_class(uint8 lchip, ctc_vlan_class_type_t type);

/**
 @brief Add vlan class default entry per label

 @param[in] lchip    local chip id

 @param[in] type            Vlan class type, mac-based ipv4-based and ipv6-based

 @param[in] p_action        Default entry action

 @remark[D2.TM]
     Configure vlan class default action, if not matching any entry for MAC, IPV4, IPV6 to vlan, the packet will do this action
     as flag in struct ctc_vlan_miss_t
     Type will be the mode of vlan class, see enum ctc_vlan_class_type_t.
     By default, do nothing will be the action.
 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_add_default_vlan_class(uint8 lchip, ctc_vlan_class_type_t type, ctc_vlan_miss_t* p_action);
/**
 @brief Remove vlan class default entry per label

 @param[in] lchip    local chip id

 @param[in] type            Vlan class type, mac-based ipv4-based and ipv6-based

 @remark[D2.TM]
    Recover configuration of vlan class default action to do nothing
    Type will be the mode of vlan class, see enum ctc_vlan_class_type_t.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_remove_default_vlan_class(uint8 lchip, ctc_vlan_class_type_t type);

/**@} end of @addtogroup VlanClassification*/

/**
 @addtogroup VlanMapping VlanMapping
 @{
*/

/**
 @brief  Create a vlan range

 @param[in] lchip    local chip id

 @param[in] vrange_info   vlan range info

 @param[in] is_svlan      vlan range used for svlan

 @remark[D2.TM]
      Create vlan range group including three element: vrange_grpid, direction (see struct ctc_vlan_range_info_t) and is_svlan.
      is_svlan denote whether the vlan range group property svlan or cvlan.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_create_vlan_range_group(uint8 lchip, ctc_vlan_range_info_t* vrange_info, bool is_svlan);

/**
 @brief  Get vlan range type

 @param[in] lchip    local chip id

 @param[in] vrange_info   vlan range info

 @param[out] is_svlan      vlan range used for svlan

 @remark[D2.TM]
     Get info about whether the vlan range group is svlan or not, according to two elements: vrange_grpid and direction of the vlan range group.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_get_vlan_range_type(uint8 lchip, ctc_vlan_range_info_t* vrange_info, bool* is_svlan);

/**
 @brief  Delete a vlan range

 @param[in] lchip    local chip id

 @param[in] vrange_info   vlan range info

 @remark[D2.TM]
     Delete vlan range group with two element: vrange_grpid and direction.
     Then 8 entries in that vlan range group will be reset.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_destroy_vlan_range_group(uint8 lchip, ctc_vlan_range_info_t* vrange_info);

/**
 @brief  Add a vlan range member to a vrange_group

 @param[in] lchip    local chip id

 @param[in] vrange_info   vlan range info

 @param[in] vlan_range    vlan_start and vlan_end in vlan range

 @remark[D2.TM]
      Add vlan range entry member into one vlan range group, according to vrange_grpid & direction of the vlan range group.
      \p
      Note: 1. vlan range group must be create before this, or will reply error.
      \p
            2. one vlan range group only can have 8 vlan range members.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_add_vlan_range(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_t* vlan_range);

/**
 @brief  Remove a vlan range member to a vrange_group

 @param[in] lchip    local chip id

 @param[in] vrange_info   vlan range info

 @param[in] vlan_range    vlan_start and vlan_end in vlan range

 @remark[D2.TM]
      Remove vlan range entry from vlan range group, according to vrange_grpid & direction of the vlan range group.
      Note: vlan range group must be create and this vlan range entry must be added before, or will reply error.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_remove_vlan_range(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_t* vlan_range);

/**
 @brief  Get all vlan range members from a vlan range

 @param[in] lchip    local chip id

 @param[in]  vrange_info   vlan range info

 @param[out] vrange_group    all vlan_start vlan_end in a group

 @param[out] count      valid range count

 @remark[D2.TM]
      Get info about vlan range entries and count in the vlan range group,according to vrange_grpid & direction

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_get_vlan_range(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_group_t* vrange_group, uint8* count);

/**
 @brief  Add one vlan mapping entry on the port in IPE

 @param[in] lchip    local chip id

 @param[in] gport           Global phyical port of system

 @param[in] p_vlan_mapping  Vlan mapping info, include key and mapped action

 @remark[D2.TM]
       Configure ingress vlan mapping on gport, expecting packet received from this gport(with some special vid or vrange_grpid or cos or not) to do special action
       Key type(any combination) will be the mapping key, see key in struct ctc_vlan_mapping_t.
       If not using cvid/svid/scos/ccos as key, CTC_VLAN_MAPPING_KEY_NONE will be set(only source port as key).
       If any entry is matched, it will do the action as action in struct ctc_vlan_mapping_t.
       \p
       Except the key and action you must set, you must configure as follow:
       \p
       1. stag_op(stag special action) & ctag_op(ctag special action) will be set configuration to stag & ctag, seeing enum ctc_vlan_tag_op_t.
       \p
       2. svid_sl(svid special action) & scos_sl(scos special action) & cvid_sl(cvid special action) & ccos_sl(ccos special action)  will be set configuration to
       svlan id, scos, cvlan id, ccos, seeing enum ctc_vlan_tag_sl_t.
       \p
       \b
       for example:
       \p
       ingress vlan translation(for example:s+c->s'+c'):
       \d (1)stag_op:CTC_VLAN_TAG_OP_REP_OR_ADD;
       \d (2)ctag_op:CTC_VLAN_TAG_OP_REP_OR_ADD;
       \d (3)svid_sl:CTC_VLAN_TAG_SL_NEW;
       \d (4)cvid_sl:CTC_VLAN_TAG_SL_NEW;
       \p
       QinQ(c->c+s):
       \d (1)stag_op:CTC_VLAN_TAG_OP_REP_OR_ADD
       \d (2)svid_sl:CTC_VLAN_TAG_SL_NEW, new svid must be configured (if having no svid, 0 is used);
       \p
       vlan stacking(s->s+s'):
       \d (1)stag_op:CTC_VLAN_TAG_OP_ADD;
       \d (2)svid_sl:CTC_VLAN_TAG_SL_NEW, new svid must be configured (if having no svid, 0 is used);

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_add_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping);

/**
 @brief  Get one vlan mapping entry

 @param[in] lchip    local chip id

 @param[in] gport           Global phyical port of system

 @param[in] p_vlan_mapping  Vlan mapping info

 @remark[D2.TM]
       Get one vlan mapping entry based on the given key.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_get_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping);

/**
 @brief  Update one vlan mapping entry on the port in IPE

 @param[in] lchip    local chip id

 @param[in] gport           Global phyical port of system

 @param[in] p_vlan_mapping  Vlan mapping info

 @remark[D2.TM]
       Update configuration ingress vlan mapping on gport.
       How to use it is the same as API ctc_vlan_add_vlan_mapping().


 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_update_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping);

/**
 @brief  Remove one vlan mapping entry on the port in IPE

 @param[in] lchip    local chip id

 @param[in] gport           Global phyical port of system

 @param[in] p_vlan_mapping  Vlan mapping info, include key info to be deleted

 @remark[D2.TM]
       Delete configuration ingress vlan mapping on gport(with some special vid or vrange_grpid or cos or not).
       Key type(any combination) will be the mapping key, see key in struct ctc_vlan_mapping_t.
       If not using cvid/svid/scos/ccos as key, CTC_VLAN_MAPPING_KEY_NONE will be set(only source port as key).
       If any entry is matched, it will do the action as action in struct ctc_vlan_mapping_t.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_remove_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping);

/**
 @brief  Add vlan mapping default entry on the port

 @param[in] lchip    local chip id

 @param[in] gport           Global phyical port of system

 @param[in] p_action        Default entry action

 @remark[D2.TM]
      Configure ingress vlan mapping default action on gport for packet received from this gport(direction ingress).
      If not matching any entry,packet will do this action just as flag in struct ctc_vlan_miss_t.
      By default, if no configure default action, packet will do nothing.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_add_default_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_miss_t* p_action);

/**
 @brief  Remove vlan mapping miss match default entry of port

 @param[in] lchip    local chip id

 @param[in] gport           Global phyical port of system

 @remark[D2.TM]
      Recover configuration of ingress vlan mapping default action to do nothing on gport

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_remove_default_vlan_mapping(uint8 lchip, uint32 gport);

/**
 @brief  Remove all vlan mapping hash entries on port

 @param[in] lchip    local chip id

 @param[in] gport           Global phyical port of system

 @remark[D2.TM]
      Remove all ingress vlan mapping hash configuration for this gport(direction ingress).

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_remove_all_vlan_mapping_by_port(uint8 lchip, uint32 gport);

/**@} end of @addtogroup VlanMapping*/

/**
 @addtogroup EgressVlanEdit EgressVlanEdit
 @{
*/

/**
 @brief  Add one egress vlan mapping entry on the port in EPE

 @param[in] lchip    local chip id

 @param[in] gport           Global phyical port of system

 @param[in] p_vlan_mapping  Egress vlan mapping info, include key and mapped action

 @remark[D2.TM]
       Configure egress vlan mapping on gport, expecting packet transmitted from this gport(with some special vid or vrange_grpid or cos or not) to do special action
       Key type(any combination) will be the egress mapping key, see key in struct ctc_egress_vlan_mapping_t.
       If not using cvid/svid/scos/ccos as key, CTC_EGRESS_VLAN_MAPPING_KEY_NONE will be set(only source port as key).
       If any entry is matched, it will do the action as action in struct ctc_egress_vlan_mapping_action_t.
       \p
       \b
       Except the key and action you must set, you must configure as follow:
       \p
       1. stag_op(stag special action) & ctag_op(ctag special action) will be set configuration to stag & ctag, seeing enum ctc_vlan_tag_op_t.
       \p
       2. svid_sl(svid special action) & scos_sl(scos special action) & cvid_sl(cvid special action) & ccos_sl(ccos special action)  will be set configuration to
       svlan id, scos, cvlan id, ccos, seeing enum ctc_vlan_tag_sl_t.
       \p
       \b
       for example:
       \p
       egress vlan translation(for example:s+c->s'+c'):
           \d (1)stag_op:CTC_VLAN_TAG_OP_REP_OR_ADD;
           \d (2)ctag_op:CTC_VLAN_TAG_OP_REP_OR_ADD;
           \d (3)svid_sl:CTC_VLAN_TAG_SL_NEW;
           \d (4)cvid_sl:CTC_VLAN_TAG_SL_NEW;
       \p
       QinQ(c+s->c):
           \d (1)stag_op:CTC_VLAN_TAG_OP_DEL;
           \d (2)svid_sl:CTC_VLAN_TAG_SL_AS_RECEIVE;

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_add_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_egress_vlan_mapping_t* p_vlan_mapping);

/**
 @brief  Remove one egress vlan mapping entry on the port in EPE

 @param[in] lchip    local chip id

 @param[in] gport           Global phyical port of system

 @param[in] p_vlan_mapping  Egress vlan mapping info, include key info to be deleted

 @remark[D2.TM]
       Delete configuration egress vlan mapping on gport with some special vid or vrange_grpid or cos or not
       Key type(any combination) will be the egress mapping key, see key in struct ctc_egress_vlan_mapping_t.
       If not using cvid/svid/scos/ccos as key, CTC_EGRESS_VLAN_MAPPING_KEY_NONE will be set(only source port as key).
       If any entry is matched, it will do the action as action in struct ctc_egress_vlan_mapping_action_t.
       \p
       \b
       Except the key and action you must set, you must configure as follow:
       \d 1. stag_op(stag special action) & ctag_op(ctag special action) will be set configuration to stag & ctag, seeing enum ctc_vlan_tag_op_t.
       \d 2. svid_sl(svid special action) & scos_sl(scos special action) & cvid_sl(cvid special action) & ccos_sl(ccos special action)  will be set configuration to
       svlan id, scos, cvlan id, ccos, seeing enum ctc_vlan_tag_sl_t.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_remove_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_egress_vlan_mapping_t* p_vlan_mapping);
/**
 @brief  Get one egress vlan mapping entry

 @param[in] lchip    local chip id

 @param[in] gport           Global phyical port of system

 @param[in] p_vlan_mapping  Egress vlan mapping info

 @remark[D2.TM]
       Get one egress vlan mapping entry based on the given key.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_get_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_egress_vlan_mapping_t* p_vlan_mapping);

/**
 @brief  Add egress vlan mapping default entry on the port

 @param[in] lchip    local chip id

 @param[in] gport           Global phyical port of system

 @param[in] p_action        Default egress entry action

 @remark[D2.TM]
      Configure egress vlan mapping default action on gport for packet transmitted from this gport(direction egress).
      If not matching any entry,packet will do this action just as flag in struct ctc_vlan_miss_t, only supporting two action: discard and do nothing.
      By default, if no configure default action, packet will do nothing.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_add_default_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_miss_t* p_action);

/**
 @brief  Remove egress vlan mapping default entry of port

 @param[in] lchip    local chip id

 @param[in] gport           Global phyical port of system

 @remark[D2.TM]
      Recover configuration of egress vlan mapping default action to do nothing.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_remove_default_egress_vlan_mapping(uint8 lchip, uint32 gport);

/**
 @brief  Remove all egress vlan mapping entries on port

 @param[in] lchip    local chip id

 @param[in] gport           Global phyical port of system

 @remark[D2.TM]
      Remove all egress vlan mapping configuration for this gport(direction egress).

 @return CTC_E_XXX
*/
extern int32
ctc_usw_vlan_remove_all_egress_vlan_mapping_by_port(uint8 lchip, uint32 gport);

/**@} end of @addtogroup EgressVlanEdit EgressVlanEdit*/

/**@} end of @addtogroup vlan VLAN*/

#ifdef __cplusplus
}
#endif

#endif

