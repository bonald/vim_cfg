/**
 @file ctc_usw_l2.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-10-28

 @version v2.0

The module provides L2UC/L2MC/FID(vlan) based default entry management functions.
\p
1) L2UC operation
\p
 When creating a new L2 entry, you need three information: MAC/FID/destination information.
 The Forwarding Instance ID (FID) is wide concept in the CTC SDK. As the L2 forwarding look up is based on MAC+FID  in the Module,
 the basic information of the packet such as VLAN value, VSI value should be mapped to the FID before further operation. Up to 16K FIDs
 are supported in the Module. Several mappings to FID can be done and are described as followed.
\p
    VLAN mapping to FID in 802.1D VLAN bridge
\d        One to one mapping of VLAN ID and FID in IVL bridge
\d        Many to one mapping of VLAN ID and FID in SVL bridge
\p
    VSI mapping to FID in VPLS bridge
\p
    S+C/S/C mapping to FID (VLAN mapping)
\p
 The allocation of the FID is in high-level application, below is an example for the allocation of FID.
\p

\t   |      Type                |  Number     |   FID         |
\t   |--------------------------|-------------|---------------|
\t   |  802.1q bridge           |      4k     | 0~4095        |
\t   |--------------------------|-------------|---------------|
\t   |  VPLS bridge             |      1k     | 4096-5119     |
\p
\p
 About logical port learning
 You must create a nexthop and bind logical port with nexthop by the API ctc_l2_set_nhid_by_logic_port if you want to
 learn use logical port.
\p
\t   | Logical port for learning   |         Other logical port      |
\t   |-----------------------------|---------------------------------|
\t   | 0~Max Logical Port Number-1 |  Max Logical Port Number~0x3FFF |
\p
 The Max Logical Port Number is specified by the API ctc_l2_fdb_init and
 the value of logical port is DsMac index, so each logical port will occupy one DsMac resource.
\p
The destination information in ctc_l2_addr_t can either be the gport or NHID. generally, basic bridge will use ctc_l2_add_fdb()
  to add a MAC address to FDB table with the given FID and the given port.
  If the sent packet need do packet edit from the given port, such as VPLS/APS application, firstly, you must create a nexthop
 ,then use NHID to add a MAC address to FDB table by ctc_l2_add_fdb_with_nexthop().
\b
2) L2MC and default entry operation
\p
 When creating a new L2MC group , you need three information: Multicast MAC/FID/Group ID and the search key
 is the Multicast MAC  + FID.
 In The Module, L2MC, IPMC, VLAN based broadcast and VSI based broadcast are using multicast replication, and
 each multicast group has a value to identify it, and it is called as multicast group ID. In Centec chipset,
 multicast entries are stored in MET which is organized as link list. The multicast group ID
 indicates the offset of the first Ds_MET entry, then all other nodes in this group's link list can be retrieved
 by one member field "nextMetPtr" in the MET entry, The allocation of the Mcast group ID is in high-level application
 , below is an example for the allocation of the Mcast group ID.
 \p

 \t  |     Mcast Type           |    Number   |Mcast Group ID |
 \t  |--------------------------|-------------|---------------|
 \t  |  Vlan based Broadcast    |      4k     | 0~4095        |
 \t  |--------------------------|-------------|---------------|
 \t  |  VSI based Broadcast     |      1k     | 4096-5119     |
 \t  |--------------------------|-------------|---------------|
 \t  |       L2MC               |      1k     |  5120~6143    |
 \t  |--------------------------|-------------|---------------|
 \t  |      IPMC                |      1k     |  6144~7167    |

\S ctc_l2.h:ctc_l2_flag_t
\S ctc_l2.h:ctc_l2_addr_t
\S ctc_l2.h:ctc_l2_mcast_addr_t
\S ctc_l2.h:ctc_l2dflt_addr_t

*/

#ifndef _CTC_USW_L2_H
#define _CTC_USW_L2_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_l2.h"

/**********************************************************************************
                      Define API function interfaces
***********************************************************************************/
/**
 @addtogroup fdb FDB
 @{
*/

/**
 @brief Init fdb module

 @param[in] lchip    local chip id

 @param[in]  l2_fdb_global_cfg l2 fdb global config information

 @remark[D2.TM] Initialize the The Module fdb subsystem, including global variable, soft table, asic table, etc.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_l2_fdb_init(uint8 lchip, void* l2_fdb_global_cfg);

/**
 @brief De-Initialize l2 module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the l2 configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_l2_fdb_deinit(uint8 lchip);

/**
 @brief Add a fdb entry

 @param[in] lchip    local chip id

 @param[in] l2_addr       device-independent L2 unicast address

 @remark[D2.TM]  Add a MAC address to FDB table with the given FID and parameters.
 \p
 \d                Use the CTC_L2_FLAG_IS_STATIC flag to add a static fdb;
  \d               Use the CTC_L2_FLAG_COPY_TO_CPU flag to send a copy to the cpu;
  \d               Use the CTC_L2_FLAG_DISCARD & CTC_L2_FLAG_SRC_DISCARD flag to add a blackbone MAC;
  \d               Use the CTC_L2_FLAG_PROTOCOL_ENTRY to add a protocol entry and paser L2PDU by L3Type;
 @return CTC_E_XXX

*/
extern int32
ctc_usw_l2_add_fdb(uint8 lchip, ctc_l2_addr_t* l2_addr);

/**
 @brief Remove a fdb entry

 @param[in] lchip    local chip id

 @param[in] l2_addr      device-independent L2 unicast address

 @remark[D2.TM]  Delete the given MAC address with the specified FID from the device.

 @return CTC_E_XXX


 Delete the given MAC address with the specified VLAN ID from the device.

*/
extern int32
ctc_usw_l2_remove_fdb(uint8 lchip, ctc_l2_addr_t* l2_addr);

/**
 @brief Get fdb entry by index

 @param[in] lchip    local chip id

 @param[in] index  fdb node index

 @param[out] l2_addr      device-independent L2 unicast address

 @remark[D2.TM]  Given a MAC address and VLAN ID by FDB index, check if the entry is present in the FDB table,
         and if so, return all associated information.
 @return CTC_E_XXX

*/
extern int32
ctc_usw_l2_get_fdb_by_index(uint8 lchip, uint32 index, ctc_l2_addr_t* l2_addr);

/**
 @brief Delete fdb entry by index

 @param[in] lchip    local chip id

 @param[in] index  fdb node index

 @remark[D2.TM]  Delete the given FDB index associated FDB Entry, check if the entry
           is present in the FDB table,and if so, delete all associated information.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_l2_remove_fdb_by_index(uint8 lchip, uint32 index);

/**
 @brief Add a fdb entry with nexthop ID

 @param[in] lchip    local chip id

 @param[in] l2_addr        device-independent L2 unicast address,when packet will edited by nh_id,l2_addr.gport is invalid.

 @param[in] nhp_id         the specified nhp_id

 @remark[D2.TM]   Add a MAC address to FDB table with nhp_id,generally it is used to VPLS networks.
           The given nexthop will be created according to the nexthop's edit information in
           Nexthop Module,and the FDB's destination port will be got from the given nexthop.


 @return CTC_E_XXX

*/
extern int32
ctc_usw_l2_add_fdb_with_nexthop(uint8 lchip, ctc_l2_addr_t* l2_addr, uint32 nhp_id);

/**
 @brief Get fdb count according to specified query condition

 @param[in] lchip    local chip id

 @param[in|out] pQuery        query condition

 @remark[D2.TM]   Get fdb count according to the given query condition.

 @return CTC_E_XXX

*/

extern int32
ctc_usw_l2_get_fdb_count(uint8 lchip, ctc_l2_fdb_query_t* pQuery);

/**
 @brief Query fdb enery according to specified query condition

 @param[in] lchip    local chip id

 @param[in] pQuery           query condition

 @param[in|out] query_rst  query results

 @remark[D2.TM]   Query all fdb entries according to the given query condition.
           The query condition include  the query type ctc_l2_fdb_query_type_t
           and the query flag ctc_l2_fdb_query_flag_t.\p

\d           If it is the first query,  query_rst.start_index must be equal to 0,
               else must is equal to the last next_query_index;
\d           If query_rst.is_end == 1,indicate the end of query.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_l2_get_fdb_entry(uint8 lchip, ctc_l2_fdb_query_t* pQuery, ctc_l2_fdb_query_rst_t* query_rst);

/**
 @brief Flush fdb entry by specified type(based on mac, port, fid) and specified flag(static,dynamic,all)

 @param[in] lchip    local chip id

 @param[in] pFlush      flush FDB entries data structure

 @remark[D2.TM]   flush all fdb entries according to the given flush condition.
           The flush condition include  the query type ctc_l2_flush_fdb_flag_t
           and the query flag ctc_l2_flush_fdb_flag_t.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_l2_flush_fdb(uint8 lchip, ctc_l2_flush_fdb_t* pFlush);

/**@} end of @addtogroup fdb FDB*/

/**
 @addtogroup l2mcast L2Mcast
 @{
*/

/**
 @brief Create a layer2 multicast group

 @param[in] lchip    local chip id

 @param[in] l2mc_addr     l2MC  data structure

 @remark[D2.TM]   Create a layer2 multicast group with MAC/FID and multicast group id,and the search key
           is the  MAC  + FID,and the function only create a multicast group with empty member.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_l2mcast_add_addr(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr);

/**
 @brief This Remove a layer2 multicast group

 @param[in] lchip    local chip id

 @param[in] l2mc_addr     l2MC  data structure

 @remark[D2.TM]   Remove a layer2 multicast group with MAC/FID.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_l2mcast_remove_addr(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr);

/**
 @brief Add a member port into a existed multicast group

 @param[in] lchip    local chip id

 @param[in] l2mc_addr     l2MC  data structure

 @remark[D2.TM]  Add member to an existing entry corresponding to the multicast address/VLAN ID in the multicast group.
 @return CTC_E_XXX

*/
extern int32
ctc_usw_l2mcast_add_member(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr);

/**
 @brief Remove a member port into a existed multicast group

 @param[in] lchip    local chip id

 @param[in] l2mc_addr     l2MC  data structure

 @remark[D2.TM]  Remove member from an existing entry corresponding to the multicast address/VLAN ID in the multicast group.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_l2mcast_remove_member(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr);

/**
 @brief Add a default entry

 @param[in] lchip    local chip id

 @param[in] l2dflt_addr   point to ctc_l2dflt_addr_t

 @remark[D2.TM]   Create a default entry  with FID and multicast group id  by multicast method ,and the search key
           is the FID,and the function only create a multicast group with empty member.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_l2_add_default_entry(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr);

/**
 @brief Remove default entry

 @param[in] lchip    local chip id

 @param[in] l2dflt_addr   point to ctc_l2dflt_addr_t

 @remark[D2.TM]   Remove a default entry with FID .


 @return CTC_E_XXX
*/
extern int32
ctc_usw_l2_remove_default_entry(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr);

/**
 @brief Add a port into default entry

 @param[in] lchip    local chip id

 @param[in] l2dflt_addr   point to ctc_l2dflt_addr_t

 @remark[D2.TM] Add member to an existing entry corresponding to the multicast address/VLAN ID in the multicast group.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_l2_add_port_to_default_entry(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr);

/**
 @brief Remove a port from default entry

 @param[in] lchip    local chip id

 @param[in] l2dflt_addr   point to ctc_l2dflt_addr_t

 @remark[D2.TM] Remove member from an existing entry corresponding to the multicast address/VLAN ID in the multicast group.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_l2_remove_port_from_default_entry(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr);

/**
 @brief Set default entry features

 @param[in] lchip    local chip id

 @param[in] l2dflt_addr  point to ctc_l2dflt_addr_t

 @remark[D2.TM] Configure default entry's property .
 \p
 \d    Not support CTC_L2_DFT_VLAN_FLAG_UNKOWN_UCAST_DROP flag to drop Unknown unicast packet in this interface, must using ctc_l2_set_fid_property instead;
 \d    Not support CTC_L2_DFT_VLAN_FLAG_UNKOWN_MCAST_DROP flag to drop Unknown multicast packet in this interface, must using ctc_l2_set_fid_property instead;
  \d   Use the CTC_L2_DFT_VLAN_FLAG_PROTOCOL_EXCP_TOCPU flag to Protocol exception to cpu;
 \d    Not support CTC_L2_DFT_VLAN_FLAG_LEARNING_DISABLE flag to drop Unknown multicast packet in this interface, must using ctc_l2_set_fid_property instead;

  \p


 @return CTC_E_XXX
*/
extern int32
ctc_usw_l2_set_default_entry_features(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr);

/**
 @brief Get default entry features

 @param[in] lchip    local chip id

 @param[in|out] l2dflt_addr   point to ctc_l2dflt_addr_t

 @remark[D2.TM] Get default entry's property

 @return CTC_E_XXX
*/
extern int32
ctc_usw_l2_get_default_entry_features(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr);

/**
 @brief Set nhid logic_port mapping table

 @param[in] lchip    local chip id

 @param[in]   logic_port   logic_port
 @param[in]   nhp_id       nexthop_id


 @remark[D2.TM] Set nhid to logic-port mapping.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_l2_set_nhid_by_logic_port(uint8 lchip, uint16 logic_port, uint32 nhp_id);


/**
 @brief Get nhid by logic_port from mapping table

 @param[in] lchip    local chip id

 @param[in] logic_port logic port

 @param[out] nhp_id    nexthop id

 @remark[D2.TM] Get nhid to logic-port mapping by logic-port.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_l2_get_nhid_by_logic_port(uint8 lchip, uint16 logic_port, uint32* nhp_id);



/**
 @brief Set various properties of the fid

 @param[in] lchip    local chip id

 @param[in] fid_id          fid id

 @param[in] property        fid property

 @param[in] value           value need set

 @remark[D2.TM]
        Set property configuration as enum ctc_l2_fid_property_t with the uint32 value in the fid given.\p

 @return CTC_E_XXX
*/
extern int32
ctc_usw_l2_set_fid_property(uint8 lchip, uint16 fid_id, ctc_l2_fid_property_t property, uint32 value);

/**
 @brief Get various properties of the fid

 @param[in] lchip    local chip id

 @param[in] fid_id          fid id

 @param[in] property        fid property

 @param[out] value           value need get

 @remark[D2.TM]
        Get info about property configuration of this fid.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_l2_get_fid_property(uint8 lchip, uint16 fid_id, ctc_l2_fid_property_t property, uint32* value);

/**
 @brief Set aging status of the fdb, means hit or not

 @param[in] lchip    local chip id

 @param[in] l2_addr  l2 address

 @param[in] hit      entry hit status

 @remark[D2.TM]
        Set the FDB entry hit or not by user.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_l2_fdb_set_entry_hit(uint8 lchip, ctc_l2_addr_t* l2_addr, uint8 hit);

/**
 @brief Get aging status of the fdb, means hit or not

 @param[in] lchip    local chip id

 @param[in] l2_addr  l2 address

 @param[out] hit     entry hit status

 @remark[D2.TM]
        Get the FDB entry hit or not.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_l2_fdb_get_entry_hit(uint8 lchip, ctc_l2_addr_t* l2_addr, uint8* hit);

/**
 @brief Replace a fdb entry

 @param[in] lchip    local chip id

 @param[in] p_replace       device-independent l2 fdb replace data struct

 @remark[D2.TM]
        Replace a fdb entry by the match entry. The match entry always need to be get with CTC_L2_FDB_ENTRY_CONFLICT.
        If hit the given fdb entry, the match entry will not be removed, the given fdb entry will be overwite; If hit
        the match fdb entry and they are conflict entries, the match fdb entry will be removed and the given fdb entry
        will add to device; if not hit the match fdb entry, the function likes add fdb.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_l2_replace_fdb(uint8 lchip, ctc_l2_replace_t* p_replace);

/**@} end of @addtogroup l2mcast L2Mcast*/

#ifdef __cplusplus
}
#endif

#endif /*end of _CTC_USW_L2_H*/

