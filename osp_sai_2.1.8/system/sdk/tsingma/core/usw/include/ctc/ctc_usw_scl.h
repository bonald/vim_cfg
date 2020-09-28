#if 1
/**
    @file ctc_usw_scl.h

    @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

    @date 2013-02-21

    @version v2.0

    The CTC SCL module provides the Service Classification List features. SCL
    can be used to support the following application:
\p
\d    vlan classification
\d    QinQ
\d    vlan translation
\d    vlan stacking
\d    vlan switching
\d    vpls
\d    vpws
\d    ip source guard
\d    tunnel
\d    open flow
\b
    Based on these classifications, various actions can be taken, such as:
\p
\d        Discarding the packet
\d        Sending the packet to the CPU
\d        Redirect the packet
\d        Output fid and uservlanptr
\d        Output metadata and enable acl lookup
\d        Changing the ToS, VLAN or other fields
\b
    Group-id:        This represents a scl group which has many scl entries. One group can have Mac Vlan Ipv4 Ipv6 mixed.
                Some Group-id are reserved, following hash group have created already when sdk init:
\d                    CTC_SCL_GROUP_ID_HASH_PORT                     0xFFFF0001
\d                    CTC_SCL_GROUP_ID_HASH_PORT_CVLAN          0xFFFF0002
\d                    CTC_SCL_GROUP_ID_HASH_PORT_SVLAN           0xFFFF0003
\d                    CTC_SCL_GROUP_ID_HASH_PORT_2VLAN           0xFFFF0004
\d                    CTC_SCL_GROUP_ID_HASH_PORT_CVLAN_COS   0xFFFF0005
\d                    CTC_SCL_GROUP_ID_HASH_PORT_SVLAN_COS   0xFFFF0006
\d                    CTC_SCL_GROUP_ID_HASH_MAC                        0xFFFF0007
\d                    CTC_SCL_GROUP_ID_HASH_PORT_MAC              0xFFFF0008
\d                    CTC_SCL_GROUP_ID_HASH_IPV4                       0xFFFF0009
\d                    CTC_SCL_GROUP_ID_HASH_PORT_IPV4             0xFFFF000A
\d                    CTC_SCL_GROUP_ID_HASH_IPV6                       0xFFFF000B
\d                    CTC_SCL_GROUP_ID_HASH_L2                          0xFFFF000C
\d                    CTC_SCL_GROUP_ID_HASH_PORT_IPV6             0xFFFF000D
\b
\p
    Group-priority:  This represents tcam scl lookup ID.
                     There are 2 scl tcam lookup<0-1>, and each has its own feature and block.
                     The priority is block_id, the naming comes from that the actions
                     from block_id=0 take higher priority than block_id=1 when these block
                     have conflict actions.
\b
\p
    Entry-id:        This represents a scl entry, and is a global concept. In other words,
                     the entry-id is unique for all scl groups.
\b
\p
    Entry-priority:  This represents a tcam entry priority within a block.
                     The bigger the higher priority. Different entry-type has
                     nothing to compare.
\b
                     Ex. group id=0, group prio=0. There are 3 entries:
\p
\d                        entry id=0, entry prio=10, type= mac
\d                        entry id=1, entry prio=20, type= mac
\d                        entry id=2, entry prio=20, type= ipv4
\d                        The priority of eid1 > eid0.
\d                        The priority of eid2 has nothing to do with eid0.

\S ctc_scl.h:ctc_scl_key_type_t
\S ctc_scl.h:ctc_scl_key_t
\S ctc_scl.h:ctc_scl_action_type_t
\S ctc_scl.h:ctc_scl_action_t
\S ctc_scl.h:ctc_scl_igs_action_t
\S ctc_scl.h:ctc_scl_egs_action_t
\S ctc_scl.h:ctc_scl_flow_action_t
\S ctc_scl.h:ctc_scl_entry_t
\S ctc_scl.h:ctc_scl_group_type_t
\S ctc_scl.h:ctc_scl_group_info_t

*/

#ifndef _CTC_USW_SCL_H
#define _CTC_USW_SCL_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_const.h"
#include "ctc_scl.h"
#include "ctc_stats.h"

/**
 @addtogroup scl SCL
 @{
*/

/**
 @brief     Init scl module.

 @param[in] lchip    local chip id

 @param[in] scl_global_cfg      Init parameter

 @remark[D2.TM]    Init scl module.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_init(uint8 lchip, void* scl_global_cfg);

/**
 @brief     DeInit scl module.

 @param[in] lchip    local chip id

 @remark[D2.TM]    Init scl module.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_deinit(uint8 lchip);

/**
 @brief     Create a scl group.

 @param[in] lchip    local chip id

 @param[in] group_id      SCL group ID
 @param[in] group_info    SCL group info

 @remark[D2.TM]    Creates a scl group with a specified type and priority.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_create_group(uint8 lchip, uint32 group_id, ctc_scl_group_info_t* group_info);

/**
 @brief     Destroy a scl group.

 @param[in] lchip    local chip id

 @param[in] group_id      SCL group ID

 @remark[D2.TM]    Destroy a scl group.
            All entries that uses this group must have been removed before calling this routine,
            or the call will fail.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_destroy_group(uint8 lchip, uint32 group_id);

/**
 @brief     Install all entries of a scl group into the hardware tables.

 @param[in] lchip    local chip id

 @param[in] group_id      SCL group ID
 @param[in] group_info    SCL group info

 @remark[D2.TM]    Installs a group of entries into the hardware tables.
            Will silently reinstall entries already in the hardware tables.
            If the group is empty, nothing is installed but the group_info is saved.
            Only after the group_info saved, ctc_scl_install_entry() will work.
            If group info is NULL, just install entries that are only in software table.



 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_install_group(uint8 lchip, uint32 group_id, ctc_scl_group_info_t* group_info);

/**
 @brief     Uninstall all entries of a scl group from the hardware table.

 @param[in] lchip    local chip id

 @param[in] group_id      SCL group ID

 @remark[D2.TM]    Uninstall all entries of a group from the hardware tables.
            Will silently ignore entries that are not in the hardware tables.
            This does not destroy the group or remove its entries,
            it only uninstalls them from the hardware tables.
            Remove an entry by using  ctc_scl_remove_entry(),
            and destroy a group by using ctc_scl_destroy_group().

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_uninstall_group(uint8 lchip, uint32 group_id);

/**
 @brief      Get group_info of a scl group.

 @param[in] lchip    local chip id

 @param[in]  group_id            SCL group ID
 @param[out] group_info          SCL group information

 @remark[D2.TM]     Get group_info of a scl group.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_get_group_info(uint8 lchip, uint32 group_id, ctc_scl_group_info_t* group_info);

/**
 @brief     Add an scl entry to a specified scl group.

 @param[in] lchip    local chip id

 @param[in] group_id      SCL group ID
 @param[in] scl_entry     SCL entry info.

 @remark[D2.TM]    Add an scl entry to a specified scl group.
            The entry cantains: key, action, entry_id, priority, refer to ctc_scl_entry_t.
            Remove an entry by using ctc_scl_remove_entry().
            It only add entry to the software table, install to hardware table
            by using ctc_scl_install_entry().
            If scl entry is hash entry, you must install this entry immediately when you add a entry.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_add_entry(uint8 lchip, uint32 group_id, ctc_scl_entry_t* scl_entry);

/**
 @brief     Remove an scl entry.

 @param[in] lchip    local chip id

 @param[in] entry_id      SCL entry ID

 @remark[D2.TM]    Remove an scl entry from software table,
            uninstall from hardware table by calling ctc_scl_uninstall_entry().

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_remove_entry(uint8 lchip, uint32 entry_id);

/**
 @brief     Installs an entry into the hardware tables.

 @param[in] lchip    local chip id

 @param[in] entry_id      SCL entry ID

 @remark[D2.TM]    Installs an entry into the hardware tables.
            Must add an entry to software tablse first, by calling ctc_scl_add_entry().

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_install_entry(uint8 lchip, uint32 entry_id);

/**
 @brief     Uninstall an scl entry from the hardware tables.

 @param[in] lchip    local chip id

 @param[in] entry_id      SCL entry ID

 @remark[D2.TM]    Uninstall an scl entry from the hardware tables.
            The object of this entry is still in software table, thus can install again.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_uninstall_entry(uint8 lchip, uint32 entry_id);

/**
 @brief     Remove all scl entries of a scl group.

 @param[in] lchip    local chip id

 @param[in] group_id      SCL group ID

 @remark[D2.TM]    Remove all scl entries of a scl group from software table, must uninstall all
            entries first.
            Uninstall one entry from hardware table by using ctc_scl_uninstall_entry().
            Uninstall a whole group from hardware table by using ctc_scl_uninstall_group().

 @return    CTC_E_XXX
*/
extern int32
ctc_usw_scl_remove_all_entry(uint8 lchip, uint32 group_id);

/**
 @brief     set an entry priority.

 @param[in] lchip    local chip id

 @param[in] entry_id      SCL entry ID

 @param[in] priority      SCL entry priority

 @remark[D2.TM]    Sets the priority of an entry.
            Entries with higher priority values take precedence over entries with lower values.
            Since TCAM lookups start at low indexes, precedence within a physical block is the
            reverse of this.
            The effect of this is that entries with the greatest priority will have the lowest TCAM index.
            Entries should have only positive priority values. If 2 entry priority set equally,
            the latter is lower than former.
            Currently there are 1 predefined cases:
            FPA_PRIORITY_LOWEST  - Lowest possible priority
            1 is the default value.
            Hash entry cannot set priority.

@return CTC_E_XXX
*/
extern int32
ctc_usw_scl_set_entry_priority(uint8 lchip, uint32 entry_id, uint32 priority);

/**
 @brief      Get an array of entry IDs in use in a group

 @param[in] lchip    local chip id

 @param[in|out]  query     Pointer to query structure.

 @remark[D2.TM]     Fills an array with the entry IDs for the specified group. This should first be called with an  entry_size  of 0
to get the number of entries to be returned, so that an appropriately-sized array can be allocated.

@return CTC_E_XXX
*/
extern int32
ctc_usw_scl_get_multi_entry(uint8 lchip, ctc_scl_query_t* query);

/**
 @brief      Set scl default action

 @param[in] lchip    local chip id

 @param[in]  def_action       New default action

 @remark[D2.TM]     Update scl action on specific gport. This action will overwrite the old action, and it
             will be installed to hardware immediately.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_set_default_action(uint8 lchip, ctc_scl_default_action_t* def_action);

/**
 @brief      Set scl hash field select.

 @param[in] lchip    local chip id

 @param[in]  field_sel     Pointer to ctc_scl_hash_field_sel_t struct

 @remark[D2.TM]     Set hash key's hash field select. Key has a variety of hash field, which
             indicate the fields that hash key cares.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_set_hash_field_sel(uint8 lchip, ctc_scl_hash_field_sel_t* field_sel);

/**
 @brief      Add scl key field.

 @param[in] lchip    local chip id

 @param[in] entry_id      SCL entry ID

 @param[in]  p_field_key     Pointer to ctc_field_key_t struct

 @remark[D2.TM]    Add an scl key field to a specified scl entry.
            The key field cantains: type, data, mask, ext_data, ext_mask refer to ctc_scl_key_field_t.
            Remove an key field by using ctc_scl_remove_key_field().
            It only add entry to the software table, install to hardware table
            by using ctc_scl_install_entry().
            If scl entry is tcam entry, you must set mask.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_add_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* p_field_key);

/**
 @brief     Remove scl key field.

 @param[in] lchip    local chip id

 @param[in] entry_id      SCL entry ID

 @param[in]  p_field_key     Pointer to ctc_field_key_t struct

 @remark[D2.TM]    Remove an scl key field from software table,
            uninstall from hardware table by calling ctc_scl_uninstall_entry().

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_remove_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* p_field_key);

/**
 @brief      Add scl action field.

 @param[in] lchip    local chip id

 @param[in] entry_id      SCL entry ID

 @param[in]  p_field_action     Pointer to ctc_scl_field_action_t struct

 @remark[D2.TM]    Add an scl action field to a specified scl entry.
            The key field cantains: type, data0, data1, ext_data refer to ctc_scl_action_field_t.
            Remove an action field by using ctc_scl_remove_action_field().
            It only add entry to the software table, install to hardware table
            by using ctc_scl_install_entry().

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_add_action_field(uint8 lchip, uint32 entry_id, ctc_scl_field_action_t* p_field_action);

/**
 @brief     Remove scl action field.

 @param[in] lchip    local chip id

 @param[in] entry_id      SCL entry ID

 @param[in]  p_field_action     Pointer to ctc_scl_field_action_t struct

 @remark[D2.TM]    Remove an scl action field from software table,
            uninstall from hardware table by calling ctc_scl_uninstall_entry().

 @return CTC_E_XXX
*/
extern int32
ctc_usw_scl_remove_action_field(uint8 lchip, uint32 entry_id, ctc_scl_field_action_t* p_field_action);
/**@} end of @addgroup   */

#ifdef __cplusplus
}
#endif

#endif

#endif
