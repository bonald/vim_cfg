/**
 @file ctc_usw_mirror.h

 @date 2009-10-21

 @version v2.0

 \p
 Mirror feature allows packet or flow be duplicated and capture on destination port for the purposes of debugging
 or sniffing. The mirroring module allows network application developers to manage mirror configuration.
 \p
 \b
 According to mirror source and mirror destination on the same or different device, mirror module can provide local
 mirror, remote mirror.
 \p
 \d  Local mirror: the source ports and the destination port all locate on the same device(switch).
 \d  Remote mirror: source ports and destination port are not located on the same device(switch). This is an advanced
 feature that requires a special VLAN to carry the packet being monitored by SPAN between different switches.
 \p
 \b
 According to the packet's direction, mirror module can provide ingress mirror and egress mirror.
 Meanwhile, mirror module support mirror source based on port, vlan and ACL.
 \p
 \b
 Based on port, packet sent by the port or received by port can be duplicated to another port for debugging or
 sniffing purposes. You must enable mirror function based port by API ctc_mirror_set_port_en().
 \p
 \b
 Based on vlan, if the packet forwarding vlan is source vlan, the packet will be duplicated to another port for
 debugging or sniffing purposes. And you must enable mirror function based vlan by API ctc_mirror_set_vlan_en().
 \p
 \b
 Based on ACL, traffic through ACL will be duplicated to another port for debugging or sniffing purposes.
 You must enable mirror function based ACL by API ctc_acl_add_entry().
 \p
 \b
 Mirror module also provides some other function:
 \p
 \d  mirror Filtering
 \d  mirror discard packet or not
 \p
 \b
 The ctc_mirror_dest_t structure contains mirror physical destination and encapsulation info. You can see struct
 ctc_mirror_dest_t. Configuration destinations and encapsulation info by API ctc_mirror_add_session(), using struct
 ctc_mirror_dest_t. If remote mirror configured, another vlan tag will be added. You can use rspan.vlan_id directly.
 Otherwise, you can configure rspan.nh_id first, by API ctc_nh_add_rspan(). And you can use rspan.nh_id to configure
 encapsulation info. GRE mirror, you must create nexthop first.
 \p
 \b
 Mirror filtering function will not do mirror to packets with special mac destination address configured by API
 ctc_mirror_set_escape_mac().This function must be enabled by API ctc_mirror_set_escape_en().
 \p
 \b
 For some special type of mirror(based port, vlan, acl),it can still do mirror by API ctc_mirror_set_mirror_discard,
 even if the packet is discard.

 \S ctc_mirror.h:ctc_mirror_session_type_t
 \S ctc_mirror.h:ctc_mirror_erspan_psc_t
 \S ctc_mirror.h:ctc_mirror_discard_t
 \S ctc_mirror.h:ctc_mirror_info_type_t
 \S ctc_mirror.h:ctc_mirror_dest_t
 \S ctc_mirror.h:ctc_mirror_rspan_escape_t

*/

#ifndef _CTC_USW_MIRROR_H
#define _CTC_USW_MIRROR_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "ctc_mirror.h"

/****************************************************************************
 *
* Function
*
*****************************************************************************/
/**
 @addtogroup mirror MIRROR
 @{
*/

/**
 @brief Initialize the mirror module

 @param[in] lchip    local chip id

 @param[in] mirror_global_cfg   mirror module global config

 @remark[D2.TM]
     Initialize mirror feature module, when init, having no need to give global configuration.
     When initialized, mirroring is disabled on the device.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mirror_init(uint8 lchip, void* mirror_global_cfg);

/**
 @brief De-Initialize mirror module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the mirror configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mirror_deinit(uint8 lchip);

/**
 @brief Set mirror enable on port

 @param[in] lchip    local chip id

 @param[in] gport           global phyical port of system

 @param[in] dir             mirror direction, ingress or egress flow

 @param[in] enable          mirror enable or not on port

 @param[in] session_id      mirror session id

 @remark[D2.TM]
     Enable mirror on port. Monitoring packet input the gport or output the gport.
     \p
     1. gport: specify the mirror global source port; when source port is equal to CTC_LPORT_CPU, it means cpu mirror which miror packet to cpu or from cpu.
     \p
     2. dir:
            \p
            ingress denote monitoring packets received in this port;
            \p
            egress denote monitoring packets transmitted from this port.
     \p
     3. enable: enable or disable mirror function in this port.
     \p
     4. session_id: monitoring the packets to which session; when mirror source is cpu port, the value is meaningless.
     \p
     Note: support 4 sessions for mirror based port.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mirror_set_port_en(uint8 lchip, uint32 gport, ctc_direction_t dir, bool enable, uint8 session_id);

/**
 @brief Get the information of port mirror

 @param[in] lchip    local chip id

 @param[in] gport           global phyical port of system

 @param[in] dir             mirror direction, ingress or egress flow

 @param[out] enable          a return value denote whether mirror enable on port

 @param[out] session_id      a return value denote mirror session id if mirror enable on this port

 @remark[D2.TM]
     Get info about mirror source port
     \p
     1. gport: specify the mirror global source port; when source port is equal to CTC_LPORT_CPU, it means cpu mirror which miror packet to cpu or from cpu.
     \p
     2. dir:
     \p
            ingress denote monitoring packets received in this port;
            \p
            egress denote monitoring packets tranmitted from this port.
     \p
     3. enable: get enable or disable info mirror function in this port.
     \p
     4. session_id: monitoring the packets to which session, get sesson_id info; when mirror source is cpu port, the value is meaningless.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mirror_get_port_info(uint8 lchip, uint32 gport, ctc_direction_t dir, bool* enable, uint8* session_id);

/**
 @brief Set mirror enable on vlan

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] dir             mirror direction, ingress or egress flow

 @param[in] enable          mirror enable or not on vlan

 @param[in] session_id      mirror session id

 @remark[D2.TM]
     Enable mirror on vlan.
     \p
     1. dir:
     \p
           ingress denote monitoring packets received with this vlan;
           \p
           egress denote monitoring packets tranmitted with this vlan.
           \p
     2. enable: enable or disable mirror function in this vlan.
     \p
     3. session_id: monitoring the packets to which session.
     \p
     Note: support 4 sessions for mirror based vlan.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mirror_set_vlan_en(uint8 lchip, uint16 vlan_id, ctc_direction_t dir, bool enable, uint8 session_id);

/**
 @brief Get the information of vlan mirror

 @param[in] lchip    local chip id

 @param[in] vlan_id         802.1q vlan id

 @param[in] dir             mirror direction, ingress or egress flow

 @param[out] enable          a return value denote whether mirror enable on port

 @param[out] session_id      a return value denote mirror session id if mirror enable on this port

 @remark[D2.TM]
     Get info about mirror source vlan
     \p
     1. dir:
     \p
           ingress denote monitoring packets received with this vlan;
           \p
           egress denote monitoring packets tranmitted with this vlan.
           \p
     2. enable: get enable or disable info mirror function in this vlan.
     \p
     3. session_id: monitoring the packets to which session, get sesson_id info.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mirror_get_vlan_info(uint8 lchip, uint16 vlan_id, ctc_direction_t dir, bool* enable, uint8* session_id);

/**
 @brief Set local mirror destination port by add session

 @param[in] lchip    local chip id

 @param[in] mirror          mirror destination information

 @remark[D2.TM]
     Configure the session destinations and encapsulation info, see struct ctc_mirror_dest_t:
     \p
     1. session_id:  mirror session id, when type is CTC_MIRROR_CPU_SESSION its value is meaningless.
     \p
     2. dest_gport:  destinations.
     \p
     3. is_rspan:    if set, this session will be used remote mirror/GRE mirror, else be local mirror
     \p
     4. vlan_valid:  if it set, it can be configure vlan_id(vlan tag add when rspan over l2) directly, or
     you will use nh_id (you must create nh_id by API ctc_nh_add_rspan() first) to configure
     encapsulation info.
     \p
     5.type: port mirror or vlan mirror.
     \p
           CTC_MIRROR_L2SPAN_SESSION:       mirror l2span session
           \p
           CTC_MIRROR_L3SPAN_SESSION:       mirror l3span session
           \p
           CTC_MIRROR_ACLLOG_SESSION:       mirror acllog session
           \p
           CTC_MIRROR_CPU_SESSION:          mirror cpu session
           \p
     6. acl_priority: acl priority id refer to acl moudule

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mirror_add_session(uint8 lchip, ctc_mirror_dest_t* mirror);

/**
 @brief Discard some special mirrored packet if enable

 @param[in] lchip    local chip id

 @param[in] enable  boolean value denote the function enable or not

 @remark[D2.TM]
     Enable mac mirror filting function.
     When packet with this special macsa won't be duplicated on another port for debugging or sniffing purposes.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mirror_set_escape_en(uint8 lchip, bool enable);

/**
 @brief Config mac info indicate the mirrored packet is special

 @param[in] lchip    local chip id

 @param[in] escape  please refer to ctc_mirror_rspan_escape_t

 @remark[D2.TM]
     Configure mac destination address, with which packet won't be mirrored, for example useful for bpdu.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mirror_set_escape_mac(uint8 lchip, ctc_mirror_rspan_escape_t* escape);

/**
 @brief Remove mirror destination port by remove session

 @param[in] lchip    local chip id

 @param[in] mirror          mirror destination information

 @remark[D2.TM]
     Remove configure the session with the following feature:
     \p
     1.session_id: mirror session id, when type is CTC_MIRROR_CPU_SESSION its value is meaningless.
     \p
     2.type: port mirror or vlan mirror.
     \p
           CTC_MIRROR_L2SPAN_SESSION:       mirror l2span session
           \p
           CTC_MIRROR_L3SPAN_SESSION:       mirror l3span session
           \p
           CTC_MIRROR_ACLLOG_SESSION:       mirror acllog session
           \p
           CTC_MIRROR_CPU_SESSION:          mirror cpu session
           \p
     3.dir: ingress or egress(in or out).

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mirror_remove_session(uint8 lchip, ctc_mirror_dest_t* mirror);

/**
 @brief Set that whether packet is mirrored when it has been droped

 @param[in] lchip    local chip id

 @param[in] dir             direciton ingress or egress

 @param[in] discard_flag    indicate port mirror or vlan mirror

 @param[in] enable          enable/disable

 @remark[D2.TM]
     Some special mirror type packet can be mirror, even if the packet was discard.
     \p
     You can set discard_flag :
     \p
           CTC_MIRROR_L2SPAN_DISCARD:          mirror qos session drop packet.
           \p
           CTC_MIRROR_L3SPAN_DISCARD:          mirror vlan session drop packet.
           \p
           CTC_MIRROR_ACLLOG_PRI_DISCARD:      mirror acl priority session drop packet.
           \p
     enable: TRUE enable this function, or FALSE disable this function.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mirror_set_mirror_discard(uint8 lchip, ctc_direction_t dir, uint16 discard_flag, bool enable);

/**
 @brief Get that whether packet is mirrored when it has been droped

 @param[in] lchip    local chip id

 @param[in] dir             direction ingress or egress

 @param[in] discard_flag    indicate port mirror or vlan mirror

 @param[out] enable         a return value denote enable/disable

 @remark[D2.TM]
     Get info about which mirror type can be still take effect, even if the packet was discard.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mirror_get_mirror_discard(uint8 lchip, ctc_direction_t dir, ctc_mirror_discard_t discard_flag, bool* enable);

/**
 @brief Set erspan psc

 @param[in] lchip    local chip id

 @param[in] psc             pointer to psc

 @remark[D2.TM]
     Set selection of erspan symmetric hash field.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mirror_set_erspan_psc(uint8 lchip, ctc_mirror_erspan_psc_t* psc);

/**@} end of @addtogroup mirror MIRROR */

#ifdef __cplusplus
}
#endif

#endif

