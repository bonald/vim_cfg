/**********************************************************
 * ctc_api.c
 * Date:
 * Author: auto generate from include file
 **********************************************************/
/**********************************************************
 * 
 * Header file
 * 
 **********************************************************/
#include "ctc_api.h"
#include "ctcs_api.h"
#include "dal.h"
/**********************************************************
 * 
 * Defines and macros
 * 
 **********************************************************/
/**********************************************************
 * 
 * Global and Declaration
 * 
 **********************************************************/
ctcs_api_t *ctc_api = NULL;
uint8 ctc_chip_type = 0;;
#ifdef GREATBELT
extern ctcs_api_t ctc_greatbelt_api;
#endif
#ifdef GOLDENGATE
extern ctcs_api_t ctc_goldengate_api;
#endif
#ifdef USW
extern ctcs_api_t ctc_usw_api;
#endif
/**********************************************************
 * 
 * Functions
 * 
 **********************************************************/
int32 ctc_install_api()
{
    uint32 dev_id = 0;
    dal_get_chip_dev_id(0, &dev_id);
    switch (dev_id)
    {
#ifdef GREATBELT
        case DAL_GREATBELT_DEVICE_ID:
            ctc_api = &ctc_greatbelt_api;
            ctc_chip_type = CTC_CHIP_GREATBELT;
            break;
#endif
#ifdef GOLDENGATE
        case DAL_GOLDENGATE_DEVICE_ID:
        case DAL_GOLDENGATE_DEVICE_ID1:
            ctc_api = &ctc_goldengate_api;
            ctc_chip_type = CTC_CHIP_GOLDENGATE;
            break;
#endif
#ifdef USW
        case DAL_DUET2_DEVICE_ID:
            ctc_api = &ctc_usw_api;
            ctc_chip_type = CTC_CHIP_DUET2;
            break;
        case DAL_TSINGMA_DEVICE_ID:
            ctc_api = &ctc_usw_api;
            ctc_chip_type = CTC_CHIP_TSINGMA;
            break;
#endif
        default:
            return CTC_E_INVALID_PARAM;
            break;
    }
    return CTC_E_NONE;
} 
int32 ctc_uninstall_api()
{
    ctc_api = NULL;
    return CTC_E_NONE;
}
char* ctc_get_chip_name()
{
    char* chip_name[] = {NULL, "Greatbelt", "Goldengate", "Duet2", "TsingMa"};
    return chip_name[ctc_chip_type];
}
CTC_EXPORT_SYMBOL(ctc_get_chip_name);
uint8 ctc_get_chip_type()
{
    return ctc_chip_type;
}
CTC_EXPORT_SYMBOL(ctc_get_chip_type);
/*##acl##*/
int32 ctc_acl_add_action_field_list(uint32 entry_id, ctc_acl_field_action_t* p_field_list, uint32* p_field_cnt)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_add_action_field_list, lchip, entry_id, p_field_list, p_field_cnt);
}
CTC_EXPORT_SYMBOL(ctc_acl_add_action_field_list);
 
int32 ctc_acl_add_action_field(uint32 entry_id, ctc_acl_field_action_t* action_field)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_add_action_field, lchip, entry_id, action_field);
}
CTC_EXPORT_SYMBOL(ctc_acl_add_action_field);
 
int32 ctc_acl_add_cid_pair(ctc_acl_cid_pair_t* cid_pair)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_add_cid_pair, lchip, cid_pair);
}
CTC_EXPORT_SYMBOL(ctc_acl_add_cid_pair);
 
int32 ctc_acl_add_entry(uint32 group_id, ctc_acl_entry_t* acl_entry)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_add_entry, lchip, group_id, acl_entry);
}
CTC_EXPORT_SYMBOL(ctc_acl_add_entry);
 
int32 ctc_acl_add_key_field_list(uint32 entry_id, ctc_field_key_t* p_field_list, uint32* p_field_cnt)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_add_key_field_list, lchip, entry_id, p_field_list, p_field_cnt);
}
CTC_EXPORT_SYMBOL(ctc_acl_add_key_field_list);
 
int32 ctc_acl_add_key_field(uint32 entry_id, ctc_field_key_t* key_field)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_add_key_field, lchip, entry_id, key_field);
}
CTC_EXPORT_SYMBOL(ctc_acl_add_key_field);
 
int32 ctc_acl_add_udf_entry_key_field(uint32 udf_id, ctc_field_key_t* key_field)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_add_udf_entry_key_field, lchip, udf_id, key_field);
}
CTC_EXPORT_SYMBOL(ctc_acl_add_udf_entry_key_field);
 
int32 ctc_acl_add_udf_entry(ctc_acl_classify_udf_t* udf_entry)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_add_udf_entry, lchip, udf_entry);
}
CTC_EXPORT_SYMBOL(ctc_acl_add_udf_entry);
 
int32 ctc_acl_copy_entry(ctc_acl_copy_entry_t* copy_entry)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_copy_entry, lchip, copy_entry);
}
CTC_EXPORT_SYMBOL(ctc_acl_copy_entry);
 
int32 ctc_acl_create_group(uint32 group_id, ctc_acl_group_info_t* group_info)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_create_group, lchip, group_id, group_info);
}
CTC_EXPORT_SYMBOL(ctc_acl_create_group);
 
int32 ctc_acl_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_acl_deinit);
 
int32 ctc_acl_destroy_group(uint32 group_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_destroy_group, lchip, group_id);
}
CTC_EXPORT_SYMBOL(ctc_acl_destroy_group);
 
int32 ctc_acl_get_group_info(uint32 group_id, ctc_acl_group_info_t* group_info)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_get_group_info, lchip, group_id, group_info);
}
CTC_EXPORT_SYMBOL(ctc_acl_get_group_info);
 
int32 ctc_acl_get_league_mode(ctc_acl_league_t* league)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_get_league_mode, lchip, league);
}
CTC_EXPORT_SYMBOL(ctc_acl_get_league_mode);
 
int32 ctc_acl_get_multi_entry(ctc_acl_query_t* query)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_get_multi_entry, lchip, query);
}
CTC_EXPORT_SYMBOL(ctc_acl_get_multi_entry);
 
int32 ctc_acl_get_udf_entry(ctc_acl_classify_udf_t* udf_entry)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_get_udf_entry, lchip, udf_entry);
}
CTC_EXPORT_SYMBOL(ctc_acl_get_udf_entry);
 
int32 ctc_acl_init(ctc_acl_global_cfg_t* acl_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_acl_init, lchip, acl_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_acl_init);
 
int32 ctc_acl_install_entry(uint32 entry_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_install_entry, lchip, entry_id);
}
CTC_EXPORT_SYMBOL(ctc_acl_install_entry);
 
int32 ctc_acl_install_group(uint32 group_id, ctc_acl_group_info_t* group_info)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_install_group, lchip, group_id, group_info);
}
CTC_EXPORT_SYMBOL(ctc_acl_install_group);
 
int32 ctc_acl_remove_action_field_list(uint32 entry_id, ctc_acl_field_action_t* p_field_list, uint32* p_field_cnt)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_remove_action_field_list, lchip, entry_id, p_field_list, p_field_cnt);
}
CTC_EXPORT_SYMBOL(ctc_acl_remove_action_field_list);
 
int32 ctc_acl_remove_action_field(uint32 entry_id, ctc_acl_field_action_t* action_field)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_remove_action_field, lchip, entry_id, action_field);
}
CTC_EXPORT_SYMBOL(ctc_acl_remove_action_field);
 
int32 ctc_acl_remove_all_entry(uint32 group_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_remove_all_entry, lchip, group_id);
}
CTC_EXPORT_SYMBOL(ctc_acl_remove_all_entry);
 
int32 ctc_acl_remove_cid_pair(ctc_acl_cid_pair_t* cid_pair)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_remove_cid_pair, lchip, cid_pair);
}
CTC_EXPORT_SYMBOL(ctc_acl_remove_cid_pair);
 
int32 ctc_acl_remove_entry(uint32 entry_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_remove_entry, lchip, entry_id);
}
CTC_EXPORT_SYMBOL(ctc_acl_remove_entry);
 
int32 ctc_acl_remove_key_field_list(uint32 entry_id, ctc_field_key_t* p_field_list, uint32* p_field_cnt)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_remove_key_field_list, lchip, entry_id, p_field_list, p_field_cnt);
}
CTC_EXPORT_SYMBOL(ctc_acl_remove_key_field_list);
 
int32 ctc_acl_remove_key_field(uint32 entry_id, ctc_field_key_t* key_field)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_remove_key_field, lchip, entry_id, key_field);
}
CTC_EXPORT_SYMBOL(ctc_acl_remove_key_field);
 
int32 ctc_acl_remove_udf_entry_key_field(uint32 udf_id, ctc_field_key_t* key_field)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_remove_udf_entry_key_field, lchip, udf_id, key_field);
}
CTC_EXPORT_SYMBOL(ctc_acl_remove_udf_entry_key_field);
 
int32 ctc_acl_remove_udf_entry(ctc_acl_classify_udf_t* udf_entry)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_remove_udf_entry, lchip, udf_entry);
}
CTC_EXPORT_SYMBOL(ctc_acl_remove_udf_entry);
 
int32 ctc_acl_reorder_entry(ctc_acl_reorder_t* reorder)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_reorder_entry, lchip, reorder);
}
CTC_EXPORT_SYMBOL(ctc_acl_reorder_entry);
 
int32 ctc_acl_set_entry_priority(uint32 entry_id, uint32 priority)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_set_entry_priority, lchip, entry_id, priority);
}
CTC_EXPORT_SYMBOL(ctc_acl_set_entry_priority);
 
int32 ctc_acl_set_field_to_hash_field_sel(uint8 key_type, uint8 field_sel_id, ctc_field_key_t* sel_field)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_set_field_to_hash_field_sel, lchip, key_type, field_sel_id, sel_field);
}
CTC_EXPORT_SYMBOL(ctc_acl_set_field_to_hash_field_sel);
 
int32 ctc_acl_set_hash_field_sel(ctc_acl_hash_field_sel_t* field_sel)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_set_hash_field_sel, lchip, field_sel);
}
CTC_EXPORT_SYMBOL(ctc_acl_set_hash_field_sel);
 
int32 ctc_acl_set_league_mode(ctc_acl_league_t* league)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_set_league_mode, lchip, league);
}
CTC_EXPORT_SYMBOL(ctc_acl_set_league_mode);
 
int32 ctc_acl_uninstall_entry(uint32 entry_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_uninstall_entry, lchip, entry_id);
}
CTC_EXPORT_SYMBOL(ctc_acl_uninstall_entry);
 
int32 ctc_acl_uninstall_group(uint32 group_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_uninstall_group, lchip, group_id);
}
CTC_EXPORT_SYMBOL(ctc_acl_uninstall_group);
 
int32 ctc_acl_update_action(uint32 entry_id, ctc_acl_action_t* action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_acl_update_action, lchip, entry_id, action);
}
CTC_EXPORT_SYMBOL(ctc_acl_update_action);
 
/*##aps##*/
int32 ctc_aps_create_aps_bridge_group(uint16 aps_bridge_group_id, ctc_aps_bridge_group_t* aps_group)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aps_create_aps_bridge_group, lchip, aps_bridge_group_id, aps_group);
}
CTC_EXPORT_SYMBOL(ctc_aps_create_aps_bridge_group);
 
int32 ctc_aps_create_raps_mcast_group(uint16 group_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aps_create_raps_mcast_group, lchip, group_id);
}
CTC_EXPORT_SYMBOL(ctc_aps_create_raps_mcast_group);
 
int32 ctc_aps_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aps_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_aps_deinit);
 
int32 ctc_aps_destroy_aps_bridge_group(uint16 aps_bridge_group_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aps_destroy_aps_bridge_group, lchip, aps_bridge_group_id);
}
CTC_EXPORT_SYMBOL(ctc_aps_destroy_aps_bridge_group);
 
int32 ctc_aps_destroy_raps_mcast_group(uint16 group_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aps_destroy_raps_mcast_group, lchip, group_id);
}
CTC_EXPORT_SYMBOL(ctc_aps_destroy_raps_mcast_group);
 
int32 ctc_aps_get_aps_bridge_protection_path(uint16 aps_bridge_group_id, uint32* gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aps_get_aps_bridge_protection_path, lchip, aps_bridge_group_id, gport);
}
CTC_EXPORT_SYMBOL(ctc_aps_get_aps_bridge_protection_path);
 
int32 ctc_aps_get_aps_bridge(uint16 aps_bridge_group_id, bool* protect_en)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aps_get_aps_bridge, lchip, aps_bridge_group_id, protect_en);
}
CTC_EXPORT_SYMBOL(ctc_aps_get_aps_bridge);
 
int32 ctc_aps_get_aps_bridge_working_path(uint16 aps_bridge_group_id, uint32* gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aps_get_aps_bridge_working_path, lchip, aps_bridge_group_id, gport);
}
CTC_EXPORT_SYMBOL(ctc_aps_get_aps_bridge_working_path);
 
int32 ctc_aps_get_aps_select(uint16 aps_select_group_id, bool* protect_en)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aps_get_aps_select, lchip, aps_select_group_id, protect_en);
}
CTC_EXPORT_SYMBOL(ctc_aps_get_aps_select);
 
int32 ctc_aps_init(void* aps_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_aps_init, lchip, aps_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_aps_init);
 
int32 ctc_aps_set_aps_bridge_group(uint16 aps_bridge_group_id, ctc_aps_bridge_group_t* aps_group)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aps_set_aps_bridge_group, lchip, aps_bridge_group_id, aps_group);
}
CTC_EXPORT_SYMBOL(ctc_aps_set_aps_bridge_group);
 
int32 ctc_aps_set_aps_bridge_protection_path(uint16 aps_bridge_group_id, uint32 gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aps_set_aps_bridge_protection_path, lchip, aps_bridge_group_id, gport);
}
CTC_EXPORT_SYMBOL(ctc_aps_set_aps_bridge_protection_path);
 
int32 ctc_aps_set_aps_bridge(uint16 aps_bridge_group_id, bool protect_en)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aps_set_aps_bridge, lchip, aps_bridge_group_id, protect_en);
}
CTC_EXPORT_SYMBOL(ctc_aps_set_aps_bridge);
 
int32 ctc_aps_set_aps_bridge_working_path(uint16 aps_bridge_group_id, uint32 gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aps_set_aps_bridge_working_path, lchip, aps_bridge_group_id, gport);
}
CTC_EXPORT_SYMBOL(ctc_aps_set_aps_bridge_working_path);
 
int32 ctc_aps_set_aps_select(uint16 aps_select_group_id, bool protect_en)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aps_set_aps_select, lchip, aps_select_group_id, protect_en);
}
CTC_EXPORT_SYMBOL(ctc_aps_set_aps_select);
 
int32 ctc_aps_update_raps_mcast_member(ctc_raps_member_t* p_raps_mem)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aps_update_raps_mcast_member, lchip, p_raps_mem);
}
CTC_EXPORT_SYMBOL(ctc_aps_update_raps_mcast_member);
 
/*##bpe##*/
int32 ctc_bpe_add_8021br_forward_entry(ctc_bpe_8021br_forward_t* p_extender_fwd)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_bpe_add_8021br_forward_entry, lchip, p_extender_fwd);
}
CTC_EXPORT_SYMBOL(ctc_bpe_add_8021br_forward_entry);
 
int32 ctc_bpe_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_bpe_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_bpe_deinit);
 
int32 ctc_bpe_get_intlk_en(bool* enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_bpe_get_intlk_en, lchip, enable);
}
CTC_EXPORT_SYMBOL(ctc_bpe_get_intlk_en);
 
int32 ctc_bpe_get_port_extender(uint32 gport, bool* enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_bpe_get_port_extender, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_bpe_get_port_extender);
 
int32 ctc_bpe_init(void* bpe_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_bpe_init, lchip, bpe_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_bpe_init);
 
int32 ctc_bpe_remove_8021br_forward_entry(ctc_bpe_8021br_forward_t* p_extender_fwd)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_bpe_remove_8021br_forward_entry, lchip, p_extender_fwd);
}
CTC_EXPORT_SYMBOL(ctc_bpe_remove_8021br_forward_entry);
 
int32 ctc_bpe_set_intlk_en(bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_bpe_set_intlk_en, lchip, enable);
}
CTC_EXPORT_SYMBOL(ctc_bpe_set_intlk_en);
 
int32 ctc_bpe_set_port_extender(uint32 gport, ctc_bpe_extender_t* p_extender)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_bpe_set_port_extender, lchip, gport, p_extender);
}
CTC_EXPORT_SYMBOL(ctc_bpe_set_port_extender);
 
/*##chip##*/
int32 ctc_chip_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_chip_deinit);
 
int32 ctc_chip_get_access_type(ctc_chip_access_type_t* p_access_type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_get_access_type, lchip, p_access_type);
}
CTC_EXPORT_SYMBOL(ctc_chip_get_access_type);
 
int32 ctc_chip_get_gpio_input(uint8 gpio_id, uint8* in_value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_get_gpio_input, lchip, gpio_id, in_value);
}
CTC_EXPORT_SYMBOL(ctc_chip_get_gpio_input);
 
int32 ctc_chip_get_mdio_clock(ctc_chip_mdio_type_t type, uint16* p_freq)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_get_mdio_clock, lchip, type, p_freq);
}
CTC_EXPORT_SYMBOL(ctc_chip_get_mdio_clock);
 
int32 ctc_chip_get_phy_mapping(ctc_chip_phy_mapping_para_t* phy_mapping_para)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_get_phy_mapping, lchip, phy_mapping_para);
}
CTC_EXPORT_SYMBOL(ctc_chip_get_phy_mapping);
 
int32 ctc_chip_get_property(uint8 lchip_id, ctc_chip_property_t chip_prop, void* p_value)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_chip_get_property, lchip_id, chip_prop, p_value);
}
CTC_EXPORT_SYMBOL(ctc_chip_get_property);
 
int32 ctc_chip_i2c_read(ctc_chip_i2c_read_t* p_i2c_para)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_i2c_read, lchip, p_i2c_para);
}
CTC_EXPORT_SYMBOL(ctc_chip_i2c_read);
 
int32 ctc_chip_i2c_write(ctc_chip_i2c_write_t* i2c_para)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_i2c_write, lchip, i2c_para);
}
CTC_EXPORT_SYMBOL(ctc_chip_i2c_write);
 
int32 ctc_chip_init(uint8 lchip_num)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_chip_init, lchip, lchip_num);
}
CTC_EXPORT_SYMBOL(ctc_chip_init);
 
int32 ctc_chip_mdio_read(uint8 lchip_id, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_mdio_read, lchip_id, type, p_para);
}
CTC_EXPORT_SYMBOL(ctc_chip_mdio_read);
 
int32 ctc_chip_mdio_write(uint8 lchip_id, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_mdio_write, lchip_id, type, p_para);
}
CTC_EXPORT_SYMBOL(ctc_chip_mdio_write);
 
int32 ctc_chip_read_gephy_reg(uint32 gport, ctc_chip_gephy_para_t* gephy_para)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_read_gephy_reg, lchip, gport, gephy_para);
}
CTC_EXPORT_SYMBOL(ctc_chip_read_gephy_reg);
 
int32 ctc_chip_read_i2c_buf(ctc_chip_i2c_scan_read_t* p_i2c_scan_read)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_read_i2c_buf, lchip, p_i2c_scan_read);
}
CTC_EXPORT_SYMBOL(ctc_chip_read_i2c_buf);
 
int32 ctc_chip_read_xgphy_reg(uint32 gport, ctc_chip_xgphy_para_t* xgphy_para)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_read_xgphy_reg, lchip, gport, xgphy_para);
}
CTC_EXPORT_SYMBOL(ctc_chip_read_xgphy_reg);
 
int32 ctc_chip_set_access_type(ctc_chip_access_type_t access_type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_access_type, lchip, access_type);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_access_type);
 
int32 ctc_chip_set_gephy_scan_special_reg(ctc_chip_ge_opt_reg_t* p_scan_opt, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_gephy_scan_special_reg, lchip, p_scan_opt, enable);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_gephy_scan_special_reg);
 
int32 ctc_chip_set_gpio_mode(uint8 gpio_id, ctc_chip_gpio_mode_t mode)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_gpio_mode, lchip, gpio_id, mode);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_gpio_mode);
 
int32 ctc_chip_set_gpio_output(uint8 gpio_id, uint8 out_para)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_gpio_output, lchip, gpio_id, out_para);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_gpio_output);
 
int32 ctc_chip_set_hss12g_enable(uint8 lchip_id, uint8 macro_id, ctc_chip_serdes_mode_t mode, uint8 enable)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_hss12g_enable, lchip_id, macro_id, mode, enable);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_hss12g_enable);
 
int32 ctc_chip_set_i2c_scan_en(bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_i2c_scan_en, lchip, enable);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_i2c_scan_en);
 
int32 ctc_chip_set_i2c_scan_para(ctc_chip_i2c_scan_t* p_i2c_para)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_i2c_scan_para, lchip, p_i2c_para);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_i2c_scan_para);
 
int32 ctc_chip_set_mac_led_en(bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_mac_led_en, lchip, enable);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_mac_led_en);
 
int32 ctc_chip_set_mac_led_mapping(ctc_chip_mac_led_mapping_t* p_led_map)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_mac_led_mapping, lchip, p_led_map);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_mac_led_mapping);
 
int32 ctc_chip_set_mac_led_mode(ctc_chip_led_para_t* p_led_para, ctc_chip_mac_led_type_t led_type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_mac_led_mode, lchip, p_led_para, led_type);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_mac_led_mode);
 
int32 ctc_chip_set_mdio_clock(ctc_chip_mdio_type_t type, uint16 freq)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_mdio_clock, lchip, type, freq);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_mdio_clock);
 
int32 ctc_chip_set_phy_mapping(ctc_chip_phy_mapping_para_t* phy_mapping_para)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_phy_mapping, lchip, phy_mapping_para);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_phy_mapping);
 
int32 ctc_chip_set_phy_scan_en(bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_phy_scan_en, lchip, enable);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_phy_scan_en);
 
int32 ctc_chip_set_phy_scan_para(ctc_chip_phy_scan_ctrl_t* p_scan_para)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_phy_scan_para, lchip, p_scan_para);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_phy_scan_para);
 
int32 ctc_chip_set_property(uint8 lchip_id, ctc_chip_property_t chip_prop, void* p_value)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_chip_set_property, lchip_id, chip_prop, p_value);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_property);
 
int32 ctc_chip_set_serdes_mode(uint8 lchip_id, ctc_chip_serdes_info_t* p_serdes_info)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_serdes_mode, lchip_id, p_serdes_info);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_serdes_mode);
 
int32 ctc_chip_set_xgphy_scan_special_reg(ctc_chip_xg_opt_reg_t* scan_opt, uint8 enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_set_xgphy_scan_special_reg, lchip, scan_opt, enable);
}
CTC_EXPORT_SYMBOL(ctc_chip_set_xgphy_scan_special_reg);
 
int32 ctc_chip_write_gephy_reg(uint32 gport, ctc_chip_gephy_para_t* gephy_para)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_write_gephy_reg, lchip, gport, gephy_para);
}
CTC_EXPORT_SYMBOL(ctc_chip_write_gephy_reg);
 
int32 ctc_chip_write_xgphy_reg(uint32 gport, ctc_chip_xgphy_para_t* xgphy_para)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_chip_write_xgphy_reg, lchip, gport, xgphy_para);
}
CTC_EXPORT_SYMBOL(ctc_chip_write_xgphy_reg);
 
int32 ctc_datapath_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_datapath_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_datapath_deinit);
 
int32 ctc_datapath_init(uint8 lchip_id, void* p_global_cfg)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_datapath_init, lchip_id, p_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_datapath_init);
 
int32 ctc_datapath_sim_init()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_datapath_sim_init, lchip);
}
CTC_EXPORT_SYMBOL(ctc_datapath_sim_init);
 
int32 ctc_get_chip_clock(uint16* freq)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_get_chip_clock, lchip, freq);
}
CTC_EXPORT_SYMBOL(ctc_get_chip_clock);
 
int32 ctc_get_chip_sensor(uint8 lchip_id, ctc_chip_sensor_type_t type, uint32* p_value)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_get_chip_sensor, lchip_id, type, p_value);
}
CTC_EXPORT_SYMBOL(ctc_get_chip_sensor);
 
int32 ctc_get_gchip_id(uint8 lchip_id, uint8* gchip_id)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_get_gchip_id, lchip_id, gchip_id);
}
CTC_EXPORT_SYMBOL(ctc_get_gchip_id);
 
int32 ctc_get_local_chip_num(uint8* lchip_num)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_get_local_chip_num, lchip, lchip_num);
}
CTC_EXPORT_SYMBOL(ctc_get_local_chip_num);
 
int32 ctc_init_pll_hss()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_init_pll_hss, lchip);
}
CTC_EXPORT_SYMBOL(ctc_init_pll_hss);
 
int32 ctc_parse_datapath(char* datapath_config_file)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parse_datapath, lchip, datapath_config_file);
}
CTC_EXPORT_SYMBOL(ctc_parse_datapath);
 
int32 ctc_set_chip_global_cfg(ctc_chip_global_cfg_t* chip_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_set_chip_global_cfg, lchip, chip_cfg);
}
CTC_EXPORT_SYMBOL(ctc_set_chip_global_cfg);
 
int32 ctc_set_gchip_id(uint8 lchip_id, uint8 gchip_id)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_set_gchip_id, lchip_id, gchip_id);
}
CTC_EXPORT_SYMBOL(ctc_set_gchip_id);
 
/*##diag##*/
int32 ctc_diag_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_diag_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_diag_deinit);
 
int32 ctc_diag_get_drop_info(uint8 lchip_id, ctc_diag_drop_t* p_drop)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_diag_get_drop_info, lchip_id, p_drop);
}
CTC_EXPORT_SYMBOL(ctc_diag_get_drop_info);
 
int32 ctc_diag_get_pkt_trace(uint8 lchip_id, ctc_diag_pkt_trace_result_t* p_rslt)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_diag_get_pkt_trace, lchip_id, p_rslt);
}
CTC_EXPORT_SYMBOL(ctc_diag_get_pkt_trace);
 
int32 ctc_diag_get_property(uint8 lchip_id, ctc_diag_property_t prop, void* p_value)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_diag_get_property, lchip_id, prop, p_value);
}
CTC_EXPORT_SYMBOL(ctc_diag_get_property);
 
int32 ctc_diag_init(void* init_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_diag_init, lchip, init_cfg);
}
CTC_EXPORT_SYMBOL(ctc_diag_init);
 
int32 ctc_diag_set_property(uint8 lchip_id, ctc_diag_property_t prop, void* p_value)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_diag_set_property, lchip_id, prop, p_value);
}
CTC_EXPORT_SYMBOL(ctc_diag_set_property);
 
int32 ctc_diag_trigger_pkt_trace(uint8 lchip_id, ctc_diag_pkt_trace_t* p_trace)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_diag_trigger_pkt_trace, lchip_id, p_trace);
}
CTC_EXPORT_SYMBOL(ctc_diag_trigger_pkt_trace);
 
/*##dma##*/
int32 ctc_dma_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_dma_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_dma_deinit);
 
int32 ctc_dma_init(ctc_dma_global_cfg_t* dma_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_dma_init, lchip, dma_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_dma_init);
 
int32 ctc_dma_rw_table(ctc_dma_tbl_rw_t* tbl_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_dma_rw_table, lchip, tbl_cfg);
}
CTC_EXPORT_SYMBOL(ctc_dma_rw_table);
 
int32 ctc_dma_tx_pkt(ctc_pkt_tx_t* tx_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_dma_tx_pkt, lchip, tx_cfg);
}
CTC_EXPORT_SYMBOL(ctc_dma_tx_pkt);
 
/*##dot1ae##*/
int32 ctc_dot1ae_add_sec_chan(uint8 lchip_id, ctc_dot1ae_sc_t* p_chan)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_dot1ae_add_sec_chan, lchip_id, p_chan);
}
CTC_EXPORT_SYMBOL(ctc_dot1ae_add_sec_chan);
 
int32 ctc_dot1ae_clear_stats(uint8 lchip_id, uint32 chan_id, uint8 an)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_dot1ae_clear_stats, lchip_id, chan_id, an);
}
CTC_EXPORT_SYMBOL(ctc_dot1ae_clear_stats);
 
int32 ctc_dot1ae_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_dot1ae_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_dot1ae_deinit);
 
int32 ctc_dot1ae_get_global_cfg(ctc_dot1ae_glb_cfg_t* p_glb_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_dot1ae_get_global_cfg, lchip, p_glb_cfg);
}
CTC_EXPORT_SYMBOL(ctc_dot1ae_get_global_cfg);
 
int32 ctc_dot1ae_get_sec_chan(uint8 lchip_id, ctc_dot1ae_sc_t* p_chan)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_dot1ae_get_sec_chan, lchip_id, p_chan);
}
CTC_EXPORT_SYMBOL(ctc_dot1ae_get_sec_chan);
 
int32 ctc_dot1ae_get_stats(uint8 lchip_id, uint32 chan_id, ctc_dot1ae_stats_t* p_stats)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_dot1ae_get_stats, lchip_id, chan_id, p_stats);
}
CTC_EXPORT_SYMBOL(ctc_dot1ae_get_stats);
 
int32 ctc_dot1ae_init(void* dot1ae_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_dot1ae_init, lchip, dot1ae_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_dot1ae_init);
 
int32 ctc_dot1ae_remove_sec_chan(uint8 lchip_id, ctc_dot1ae_sc_t* p_chan)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_dot1ae_remove_sec_chan, lchip_id, p_chan);
}
CTC_EXPORT_SYMBOL(ctc_dot1ae_remove_sec_chan);
 
int32 ctc_dot1ae_set_global_cfg(ctc_dot1ae_glb_cfg_t* p_glb_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_dot1ae_set_global_cfg, lchip, p_glb_cfg);
}
CTC_EXPORT_SYMBOL(ctc_dot1ae_set_global_cfg);
 
int32 ctc_dot1ae_update_sec_chan(uint8 lchip_id, ctc_dot1ae_sc_t* p_chan)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_dot1ae_update_sec_chan, lchip_id, p_chan);
}
CTC_EXPORT_SYMBOL(ctc_dot1ae_update_sec_chan);
 
/*##efd##*/
int32 ctc_efd_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_efd_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_efd_deinit);
 
int32 ctc_efd_get_global_ctl(ctc_efd_global_control_type_t type, void* value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_efd_get_global_ctl, lchip, type, value);
}
CTC_EXPORT_SYMBOL(ctc_efd_get_global_ctl);
 
int32 ctc_efd_init(void* p_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_efd_init, lchip, p_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_efd_init);
 
int32 ctc_efd_register_cb(ctc_efd_fn_t callback, void* userdata)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_efd_register_cb, lchip, callback, userdata);
}
CTC_EXPORT_SYMBOL(ctc_efd_register_cb);
 
int32 ctc_efd_set_global_ctl(ctc_efd_global_control_type_t type, void* value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_efd_set_global_ctl, lchip, type, value);
}
CTC_EXPORT_SYMBOL(ctc_efd_set_global_ctl);
 
/*##fcoe##*/
int32 ctc_fcoe_add_route(ctc_fcoe_route_t* p_fcoe_route)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_fcoe_add_route, lchip, p_fcoe_route);
}
CTC_EXPORT_SYMBOL(ctc_fcoe_add_route);
 
int32 ctc_fcoe_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_fcoe_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_fcoe_deinit);
 
int32 ctc_fcoe_init(void* fcoe_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_fcoe_init, lchip, fcoe_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_fcoe_init);
 
int32 ctc_fcoe_remove_route(ctc_fcoe_route_t* p_fcoe_route)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_fcoe_remove_route, lchip, p_fcoe_route);
}
CTC_EXPORT_SYMBOL(ctc_fcoe_remove_route);
 
/*##ftm##*/
int32 ctc_ftm_get_hash_poly(ctc_ftm_hash_poly_t* hash_poly)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ftm_get_hash_poly, lchip, hash_poly);
}
CTC_EXPORT_SYMBOL(ctc_ftm_get_hash_poly);
 
int32 ctc_ftm_get_profile_specs(uint32 spec_type, uint32* value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ftm_get_profile_specs, lchip, spec_type, value);
}
CTC_EXPORT_SYMBOL(ctc_ftm_get_profile_specs);
 
int32 ctc_ftm_mem_alloc(ctc_ftm_profile_info_t* ctc_profile_info)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_ftm_mem_alloc, lchip, ctc_profile_info);
}
CTC_EXPORT_SYMBOL(ctc_ftm_mem_alloc);
 
int32 ctc_ftm_mem_change(ctc_ftm_change_profile_t* p_profile)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ftm_mem_change, lchip, p_profile);
}
CTC_EXPORT_SYMBOL(ctc_ftm_mem_change);
 
int32 ctc_ftm_mem_free()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ftm_mem_free, lchip);
}
CTC_EXPORT_SYMBOL(ctc_ftm_mem_free);
 
int32 ctc_ftm_set_hash_poly(ctc_ftm_hash_poly_t* hash_poly)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ftm_set_hash_poly, lchip, hash_poly);
}
CTC_EXPORT_SYMBOL(ctc_ftm_set_hash_poly);
 
int32 ctc_ftm_set_profile_specs(uint32 spec_type, uint32 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ftm_set_profile_specs, lchip, spec_type, value);
}
CTC_EXPORT_SYMBOL(ctc_ftm_set_profile_specs);
 
/*##internal_port##*/
int32 ctc_alloc_internal_port(ctc_internal_port_assign_para_t* port_assign)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_alloc_internal_port, lchip, port_assign);
}
CTC_EXPORT_SYMBOL(ctc_alloc_internal_port);
 
int32 ctc_free_internal_port(ctc_internal_port_assign_para_t* port_assign)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_free_internal_port, lchip, port_assign);
}
CTC_EXPORT_SYMBOL(ctc_free_internal_port);
 
int32 ctc_internal_port_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_internal_port_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_internal_port_deinit);
 
int32 ctc_internal_port_init(void* p_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_internal_port_init, lchip, p_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_internal_port_init);
 
int32 ctc_set_internal_port(ctc_internal_port_assign_para_t* port_assign)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_set_internal_port, lchip, port_assign);
}
CTC_EXPORT_SYMBOL(ctc_set_internal_port);
 
/*##interrupt##*/
int32 ctc_interrupt_clear_status(uint8 lchip_id, ctc_intr_type_t* p_type)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_interrupt_clear_status, lchip_id, p_type);
}
CTC_EXPORT_SYMBOL(ctc_interrupt_clear_status);
 
int32 ctc_interrupt_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_interrupt_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_interrupt_deinit);
 
int32 ctc_interrupt_get_en(uint8 lchip_id, ctc_intr_type_t* p_type, uint32* p_enable)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_interrupt_get_en, lchip_id, p_type, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_interrupt_get_en);
 
int32 ctc_interrupt_get_status(uint8 lchip_id, ctc_intr_type_t* p_type, ctc_intr_status_t* p_status)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_interrupt_get_status, lchip_id, p_type, p_status);
}
CTC_EXPORT_SYMBOL(ctc_interrupt_get_status);
 
int32 ctc_interrupt_init(void* p_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_interrupt_init, lchip, p_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_interrupt_init);
 
int32 ctc_interrupt_register_event_cb(ctc_interrupt_event_t event, CTC_INTERRUPT_EVENT_FUNC cb)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_interrupt_register_event_cb, lchip, event, cb);
}
CTC_EXPORT_SYMBOL(ctc_interrupt_register_event_cb);
 
int32 ctc_interrupt_set_en(uint8 lchip_id, ctc_intr_type_t* p_type, uint32 enable)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_interrupt_set_en, lchip_id, p_type, enable);
}
CTC_EXPORT_SYMBOL(ctc_interrupt_set_en);
 
/*##ipfix##*/
int32 ctc_ipfix_add_entry(ctc_ipfix_data_t* p_data)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipfix_add_entry, lchip, p_data);
}
CTC_EXPORT_SYMBOL(ctc_ipfix_add_entry);
 
int32 ctc_ipfix_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipfix_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_ipfix_deinit);
 
int32 ctc_ipfix_delete_entry(ctc_ipfix_data_t* p_data)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipfix_delete_entry, lchip, p_data);
}
CTC_EXPORT_SYMBOL(ctc_ipfix_delete_entry);
 
int32 ctc_ipfix_get_flow_cfg(ctc_ipfix_flow_cfg_t* flow_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipfix_get_flow_cfg, lchip, flow_cfg);
}
CTC_EXPORT_SYMBOL(ctc_ipfix_get_flow_cfg);
 
int32 ctc_ipfix_get_global_cfg(ctc_ipfix_global_cfg_t* ipfix_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipfix_get_global_cfg, lchip, ipfix_cfg);
}
CTC_EXPORT_SYMBOL(ctc_ipfix_get_global_cfg);
 
int32 ctc_ipfix_get_port_cfg(uint32 gport,ctc_ipfix_port_cfg_t* ipfix_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipfix_get_port_cfg, lchip, gport, ipfix_cfg);
}
CTC_EXPORT_SYMBOL(ctc_ipfix_get_port_cfg);
 
int32 ctc_ipfix_init(void* p_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_ipfix_init, lchip, p_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_ipfix_init);
 
int32 ctc_ipfix_register_cb(ctc_ipfix_fn_t callback,void* userdata)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipfix_register_cb, lchip, callback, userdata);
}
CTC_EXPORT_SYMBOL(ctc_ipfix_register_cb);
 
int32 ctc_ipfix_set_flow_cfg(ctc_ipfix_flow_cfg_t* flow_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipfix_set_flow_cfg, lchip, flow_cfg);
}
CTC_EXPORT_SYMBOL(ctc_ipfix_set_flow_cfg);
 
int32 ctc_ipfix_set_global_cfg(ctc_ipfix_global_cfg_t* ipfix_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipfix_set_global_cfg, lchip, ipfix_cfg);
}
CTC_EXPORT_SYMBOL(ctc_ipfix_set_global_cfg);
 
int32 ctc_ipfix_set_hash_field_sel(ctc_ipfix_hash_field_sel_t* field_sel)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipfix_set_hash_field_sel, lchip, field_sel);
}
CTC_EXPORT_SYMBOL(ctc_ipfix_set_hash_field_sel);
 
int32 ctc_ipfix_set_port_cfg(uint32 gport,ctc_ipfix_port_cfg_t* ipfix_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipfix_set_port_cfg, lchip, gport, ipfix_cfg);
}
CTC_EXPORT_SYMBOL(ctc_ipfix_set_port_cfg);
 
int32 ctc_ipfix_traverse_remove(ctc_ipfix_traverse_remove_cmp fn, ctc_ipfix_traverse_t* p_data)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipfix_traverse_remove, lchip, fn, p_data);
}
CTC_EXPORT_SYMBOL(ctc_ipfix_traverse_remove);
 
int32 ctc_ipfix_traverse(ctc_ipfix_traverse_fn fn, ctc_ipfix_traverse_t* p_data)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipfix_traverse, lchip, fn, p_data);
}
CTC_EXPORT_SYMBOL(ctc_ipfix_traverse);
 
/*##ipmc##*/
int32 ctc_ipmc_add_default_entry(uint8 ip_version, ctc_ipmc_default_action_t type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipmc_add_default_entry, lchip, ip_version, type);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_add_default_entry);
 
int32 ctc_ipmc_add_group(ctc_ipmc_group_info_t* p_group)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipmc_add_group, lchip, p_group);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_add_group);
 
int32 ctc_ipmc_add_member(ctc_ipmc_group_info_t* p_group)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipmc_add_member, lchip, p_group);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_add_member);
 
int32 ctc_ipmc_add_rp_intf(ctc_ipmc_rp_t* p_rp)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipmc_add_rp_intf, lchip, p_rp);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_add_rp_intf);
 
int32 ctc_ipmc_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipmc_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_deinit);
 
int32 ctc_ipmc_get_entry_hit(ctc_ipmc_group_info_t* p_group, uint8* hit)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipmc_get_entry_hit, lchip, p_group, hit);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_get_entry_hit);
 
int32 ctc_ipmc_get_group_info(ctc_ipmc_group_info_t* p_group)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipmc_get_group_info, lchip, p_group);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_get_group_info);
 
int32 ctc_ipmc_get_mcast_force_route(ctc_ipmc_force_route_t* p_data)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipmc_get_mcast_force_route, lchip, p_data);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_get_mcast_force_route);
 
int32 ctc_ipmc_init(void* ipmc_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_ipmc_init, lchip, ipmc_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_init);
 
int32 ctc_ipmc_remove_group(ctc_ipmc_group_info_t* p_group)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipmc_remove_group, lchip, p_group);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_remove_group);
 
int32 ctc_ipmc_remove_member(ctc_ipmc_group_info_t* p_group)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipmc_remove_member, lchip, p_group);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_remove_member);
 
int32 ctc_ipmc_remove_rp_intf(ctc_ipmc_rp_t* p_rp)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipmc_remove_rp_intf, lchip, p_rp);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_remove_rp_intf);
 
int32 ctc_ipmc_set_entry_hit(ctc_ipmc_group_info_t* p_group, uint8 hit)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipmc_set_entry_hit, lchip, p_group, hit);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_set_entry_hit);
 
int32 ctc_ipmc_set_mcast_force_route(ctc_ipmc_force_route_t* p_data)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipmc_set_mcast_force_route, lchip, p_data);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_set_mcast_force_route);
 
int32 ctc_ipmc_traverse(uint8 ip_ver, ctc_ipmc_traverse_fn fn, void* user_data)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipmc_traverse, lchip, ip_ver, fn, user_data);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_traverse);
 
int32 ctc_ipmc_update_rpf(ctc_ipmc_group_info_t* p_group)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipmc_update_rpf, lchip, p_group);
}
CTC_EXPORT_SYMBOL(ctc_ipmc_update_rpf);
 
/*##ipuc##*/
int32 ctc_ipuc_add_default_entry(uint8 ip_ver, uint32 nh_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_add_default_entry, lchip, ip_ver, nh_id);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_add_default_entry);
 
int32 ctc_ipuc_add_nat_sa(ctc_ipuc_nat_sa_param_t* p_ipuc_nat_sa_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_add_nat_sa, lchip, p_ipuc_nat_sa_param);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_add_nat_sa);
 
int32 ctc_ipuc_add_tunnel(ctc_ipuc_tunnel_param_t* p_ipuc_tunnel_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_add_tunnel, lchip, p_ipuc_tunnel_param);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_add_tunnel);
 
int32 ctc_ipuc_add(ctc_ipuc_param_t* p_ipuc_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_add, lchip, p_ipuc_param);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_add);
 
int32 ctc_ipuc_arrange_fragment(void* p_arrange_info)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_arrange_fragment, lchip, p_arrange_info);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_arrange_fragment);
 
int32 ctc_ipuc_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_deinit);
 
int32 ctc_ipuc_get_entry_hit(ctc_ipuc_param_t* p_ipuc_param, uint8* hit)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_get_entry_hit, lchip, p_ipuc_param, hit);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_get_entry_hit);
 
int32 ctc_ipuc_get_natsa_entry_hit(ctc_ipuc_nat_sa_param_t* p_ipuc_nat_sa_param, uint8* hit)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_get_natsa_entry_hit, lchip, p_ipuc_nat_sa_param, hit);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_get_natsa_entry_hit);
 
int32 ctc_ipuc_get(ctc_ipuc_param_t* p_ipuc_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_get, lchip, p_ipuc_param);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_get);
 
int32 ctc_ipuc_init(void* ipuc_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_ipuc_init, lchip, ipuc_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_init);
 
int32 ctc_ipuc_remove_nat_sa(ctc_ipuc_nat_sa_param_t* p_ipuc_nat_sa_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_remove_nat_sa, lchip, p_ipuc_nat_sa_param);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_remove_nat_sa);
 
int32 ctc_ipuc_remove_tunnel(ctc_ipuc_tunnel_param_t* p_ipuc_tunnel_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_remove_tunnel, lchip, p_ipuc_tunnel_param);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_remove_tunnel);
 
int32 ctc_ipuc_remove(ctc_ipuc_param_t* p_ipuc_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_remove, lchip, p_ipuc_param);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_remove);
 
int32 ctc_ipuc_set_entry_hit(ctc_ipuc_param_t* p_ipuc_param, uint8 hit)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_set_entry_hit, lchip, p_ipuc_param, hit);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_set_entry_hit);
 
int32 ctc_ipuc_set_global_property(ctc_ipuc_global_property_t* p_global_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_set_global_property, lchip, p_global_prop);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_set_global_property);
 
int32 ctc_ipuc_set_natsa_entry_hit(ctc_ipuc_nat_sa_param_t* p_ipuc_nat_sa_param, uint8 hit)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_set_natsa_entry_hit, lchip, p_ipuc_nat_sa_param, hit);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_set_natsa_entry_hit);
 
int32 ctc_ipuc_traverse(uint8 ip_ver, void* fn, void* data)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ipuc_traverse, lchip, ip_ver, fn, data);
}
CTC_EXPORT_SYMBOL(ctc_ipuc_traverse);
 
/*##l2##*/
int32 ctc_l2_add_default_entry(ctc_l2dflt_addr_t* l2dflt_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_add_default_entry, lchip, l2dflt_addr);
}
CTC_EXPORT_SYMBOL(ctc_l2_add_default_entry);
 
int32 ctc_l2_add_fdb(ctc_l2_addr_t* l2_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_add_fdb, lchip, l2_addr);
}
CTC_EXPORT_SYMBOL(ctc_l2_add_fdb);
 
int32 ctc_l2_add_fdb_with_nexthop(ctc_l2_addr_t* l2_addr, uint32 nhp_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_add_fdb_with_nexthop, lchip, l2_addr, nhp_id);
}
CTC_EXPORT_SYMBOL(ctc_l2_add_fdb_with_nexthop);
 
int32 ctc_l2_add_port_to_default_entry(ctc_l2dflt_addr_t* l2dflt_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_add_port_to_default_entry, lchip, l2dflt_addr);
}
CTC_EXPORT_SYMBOL(ctc_l2_add_port_to_default_entry);
 
int32 ctc_l2_fdb_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_fdb_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_l2_fdb_deinit);
 
int32 ctc_l2_fdb_get_entry_hit(ctc_l2_addr_t* l2_addr, uint8* hit)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_fdb_get_entry_hit, lchip, l2_addr, hit);
}
CTC_EXPORT_SYMBOL(ctc_l2_fdb_get_entry_hit);
 
int32 ctc_l2_fdb_init(void* l2_fdb_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_l2_fdb_init, lchip, l2_fdb_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_l2_fdb_init);
 
int32 ctc_l2_fdb_set_entry_hit(ctc_l2_addr_t* l2_addr, uint8 hit)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_fdb_set_entry_hit, lchip, l2_addr, hit);
}
CTC_EXPORT_SYMBOL(ctc_l2_fdb_set_entry_hit);
 
int32 ctc_l2_flush_fdb(ctc_l2_flush_fdb_t* pFlush)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_flush_fdb, lchip, pFlush);
}
CTC_EXPORT_SYMBOL(ctc_l2_flush_fdb);
 
int32 ctc_l2_get_default_entry_features(ctc_l2dflt_addr_t* l2dflt_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_get_default_entry_features, lchip, l2dflt_addr);
}
CTC_EXPORT_SYMBOL(ctc_l2_get_default_entry_features);
 
int32 ctc_l2_get_fdb_by_index(uint32 index, ctc_l2_addr_t* l2_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_get_fdb_by_index, lchip, index, l2_addr);
}
CTC_EXPORT_SYMBOL(ctc_l2_get_fdb_by_index);
 
int32 ctc_l2_get_fdb_count(ctc_l2_fdb_query_t* pQuery)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_get_fdb_count, lchip, pQuery);
}
CTC_EXPORT_SYMBOL(ctc_l2_get_fdb_count);
 
int32 ctc_l2_get_fdb_entry(ctc_l2_fdb_query_t* pQuery, ctc_l2_fdb_query_rst_t* query_rst)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_get_fdb_entry, lchip, pQuery, query_rst);
}
CTC_EXPORT_SYMBOL(ctc_l2_get_fdb_entry);
 
int32 ctc_l2_get_fid_property(uint16 fid_id, ctc_l2_fid_property_t property, uint32* value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_get_fid_property, lchip, fid_id, property, value);
}
CTC_EXPORT_SYMBOL(ctc_l2_get_fid_property);
 
int32 ctc_l2_get_nhid_by_logic_port(uint16 logic_port, uint32* nhp_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_get_nhid_by_logic_port, lchip, logic_port, nhp_id);
}
CTC_EXPORT_SYMBOL(ctc_l2_get_nhid_by_logic_port);
 
int32 ctc_l2mcast_add_addr(ctc_l2_mcast_addr_t* l2mc_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2mcast_add_addr, lchip, l2mc_addr);
}
CTC_EXPORT_SYMBOL(ctc_l2mcast_add_addr);
 
int32 ctc_l2mcast_add_member(ctc_l2_mcast_addr_t* l2mc_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2mcast_add_member, lchip, l2mc_addr);
}
CTC_EXPORT_SYMBOL(ctc_l2mcast_add_member);
 
int32 ctc_l2mcast_remove_addr(ctc_l2_mcast_addr_t* l2mc_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2mcast_remove_addr, lchip, l2mc_addr);
}
CTC_EXPORT_SYMBOL(ctc_l2mcast_remove_addr);
 
int32 ctc_l2mcast_remove_member(ctc_l2_mcast_addr_t* l2mc_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2mcast_remove_member, lchip, l2mc_addr);
}
CTC_EXPORT_SYMBOL(ctc_l2mcast_remove_member);
 
int32 ctc_l2_remove_default_entry(ctc_l2dflt_addr_t* l2dflt_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_remove_default_entry, lchip, l2dflt_addr);
}
CTC_EXPORT_SYMBOL(ctc_l2_remove_default_entry);
 
int32 ctc_l2_remove_fdb_by_index(uint32 index)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_remove_fdb_by_index, lchip, index);
}
CTC_EXPORT_SYMBOL(ctc_l2_remove_fdb_by_index);
 
int32 ctc_l2_remove_fdb(ctc_l2_addr_t* l2_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_remove_fdb, lchip, l2_addr);
}
CTC_EXPORT_SYMBOL(ctc_l2_remove_fdb);
 
int32 ctc_l2_remove_port_from_default_entry(ctc_l2dflt_addr_t* l2dflt_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_remove_port_from_default_entry, lchip, l2dflt_addr);
}
CTC_EXPORT_SYMBOL(ctc_l2_remove_port_from_default_entry);
 
int32 ctc_l2_replace_fdb(ctc_l2_replace_t* p_replace)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_replace_fdb, lchip, p_replace);
}
CTC_EXPORT_SYMBOL(ctc_l2_replace_fdb);
 
int32 ctc_l2_set_default_entry_features(ctc_l2dflt_addr_t* l2dflt_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_set_default_entry_features, lchip, l2dflt_addr);
}
CTC_EXPORT_SYMBOL(ctc_l2_set_default_entry_features);
 
int32 ctc_l2_set_fid_property(uint16 fid_id, ctc_l2_fid_property_t property, uint32 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_set_fid_property, lchip, fid_id, property, value);
}
CTC_EXPORT_SYMBOL(ctc_l2_set_fid_property);
 
int32 ctc_l2_set_nhid_by_logic_port(uint16 logic_port, uint32 nhp_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2_set_nhid_by_logic_port, lchip, logic_port, nhp_id);
}
CTC_EXPORT_SYMBOL(ctc_l2_set_nhid_by_logic_port);
 
/*##l3if##*/
int32 ctc_l3if_add_ecmp_if_member(uint16 ecmp_if_id, uint16 l3if_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_add_ecmp_if_member, lchip, ecmp_if_id, l3if_id);
}
CTC_EXPORT_SYMBOL(ctc_l3if_add_ecmp_if_member);
 
int32 ctc_l3if_add_vmac_low_8bit(uint16 l3if_id, ctc_l3if_vmac_t* p_l3if_vmac)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_add_vmac_low_8bit, lchip, l3if_id, p_l3if_vmac);
}
CTC_EXPORT_SYMBOL(ctc_l3if_add_vmac_low_8bit);
 
int32 ctc_l3if_create_ecmp_if(ctc_l3if_ecmp_if_t* p_ecmp_if)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_create_ecmp_if, lchip, p_ecmp_if);
}
CTC_EXPORT_SYMBOL(ctc_l3if_create_ecmp_if);
 
int32 ctc_l3if_create(uint16 l3if_id, ctc_l3if_t* p_l3_if)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_create, lchip, l3if_id, p_l3_if);
}
CTC_EXPORT_SYMBOL(ctc_l3if_create);
 
int32 ctc_l3if_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_l3if_deinit);
 
int32 ctc_l3if_destory_ecmp_if(uint16 ecmp_if_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_destory_ecmp_if, lchip, ecmp_if_id);
}
CTC_EXPORT_SYMBOL(ctc_l3if_destory_ecmp_if);
 
int32 ctc_l3if_destory(uint16 l3if_id, ctc_l3if_t* p_l3_if)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_destory, lchip, l3if_id, p_l3_if);
}
CTC_EXPORT_SYMBOL(ctc_l3if_destory);
 
int32 ctc_l3if_get_acl_property(uint16 l3if_id, ctc_acl_property_t* acl_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_get_acl_property, lchip, l3if_id, acl_prop);
}
CTC_EXPORT_SYMBOL(ctc_l3if_get_acl_property);
 
int32 ctc_l3if_get_interface_router_mac(uint16 l3if_id, ctc_l3if_router_mac_t* router_mac)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_get_interface_router_mac, lchip, l3if_id, router_mac);
}
CTC_EXPORT_SYMBOL(ctc_l3if_get_interface_router_mac);
 
int32 ctc_l3if_get_l3if_id(ctc_l3if_t* p_l3_if, uint16* l3if_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_get_l3if_id, lchip, p_l3_if, l3if_id);
}
CTC_EXPORT_SYMBOL(ctc_l3if_get_l3if_id);
 
int32 ctc_l3if_get_property(uint16 l3if_id, ctc_l3if_property_t l3if_prop, uint32* p_value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_get_property, lchip, l3if_id, l3if_prop, p_value);
}
CTC_EXPORT_SYMBOL(ctc_l3if_get_property);
 
int32 ctc_l3if_get_router_mac(mac_addr_t mac_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_get_router_mac, lchip, mac_addr);
}
CTC_EXPORT_SYMBOL(ctc_l3if_get_router_mac);
 
int32 ctc_l3if_get_vmac_low_8bit(uint16 l3if_id, ctc_l3if_vmac_t* p_l3if_vmac)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_get_vmac_low_8bit, lchip, l3if_id, p_l3if_vmac);
}
CTC_EXPORT_SYMBOL(ctc_l3if_get_vmac_low_8bit);
 
int32 ctc_l3if_get_vmac_prefix(uint32 prefix_idx, mac_addr_t mac_40bit)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_get_vmac_prefix, lchip, prefix_idx, mac_40bit);
}
CTC_EXPORT_SYMBOL(ctc_l3if_get_vmac_prefix);
 
int32 ctc_l3if_init(void* l3if_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_l3if_init, lchip, l3if_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_l3if_init);
 
int32 ctc_l3if_remove_ecmp_if_member(uint16 ecmp_if_id, uint16 l3if_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_remove_ecmp_if_member, lchip, ecmp_if_id, l3if_id);
}
CTC_EXPORT_SYMBOL(ctc_l3if_remove_ecmp_if_member);
 
int32 ctc_l3if_remove_vmac_low_8bit(uint16 l3if_id, ctc_l3if_vmac_t* p_l3if_vmac)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_remove_vmac_low_8bit, lchip, l3if_id, p_l3if_vmac);
}
CTC_EXPORT_SYMBOL(ctc_l3if_remove_vmac_low_8bit);
 
int32 ctc_l3if_set_acl_property(uint16 l3if_id, ctc_acl_property_t* acl_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_set_acl_property, lchip, l3if_id, acl_prop);
}
CTC_EXPORT_SYMBOL(ctc_l3if_set_acl_property);
 
int32 ctc_l3if_set_interface_router_mac(uint16 l3if_id, ctc_l3if_router_mac_t router_mac)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_set_interface_router_mac, lchip, l3if_id, router_mac);
}
CTC_EXPORT_SYMBOL(ctc_l3if_set_interface_router_mac);
 
int32 ctc_l3if_set_property_array(uint16 l3if_id, ctc_property_array_t* l3if_prop, uint16* array_num)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_set_property_array, lchip, l3if_id, l3if_prop, array_num);
}
CTC_EXPORT_SYMBOL(ctc_l3if_set_property_array);
 
int32 ctc_l3if_set_property(uint16 l3if_id, ctc_l3if_property_t l3if_prop, uint32 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_set_property, lchip, l3if_id, l3if_prop, value);
}
CTC_EXPORT_SYMBOL(ctc_l3if_set_property);
 
int32 ctc_l3if_set_router_mac(mac_addr_t mac_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_set_router_mac, lchip, mac_addr);
}
CTC_EXPORT_SYMBOL(ctc_l3if_set_router_mac);
 
int32 ctc_l3if_set_vmac_prefix(uint32 prefix_idx, mac_addr_t mac_40bit)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_set_vmac_prefix, lchip, prefix_idx, mac_40bit);
}
CTC_EXPORT_SYMBOL(ctc_l3if_set_vmac_prefix);
 
int32 ctc_l3if_set_vrf_stats_en(ctc_l3if_vrf_stats_t* p_vrf_stats)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3if_set_vrf_stats_en, lchip, p_vrf_stats);
}
CTC_EXPORT_SYMBOL(ctc_l3if_set_vrf_stats_en);
 
/*##learning_aging##*/
int32 ctc_aging_get_property(ctc_aging_table_type_t tbl_type, ctc_aging_prop_t aging_prop, uint32* value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aging_get_property, lchip, tbl_type, aging_prop, value);
}
CTC_EXPORT_SYMBOL(ctc_aging_get_property);
 
int32 ctc_aging_read_aging_fifo(ctc_aging_fifo_info_t* fifo_info)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aging_read_aging_fifo, lchip, fifo_info);
}
CTC_EXPORT_SYMBOL(ctc_aging_read_aging_fifo);
 
int32 ctc_aging_set_property(ctc_aging_table_type_t tbl_type, ctc_aging_prop_t aging_prop, uint32 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_aging_set_property, lchip, tbl_type, aging_prop, value);
}
CTC_EXPORT_SYMBOL(ctc_aging_set_property);
 
int32 ctc_get_learning_action(ctc_learning_action_info_t* p_learning_action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_get_learning_action, lchip, p_learning_action);
}
CTC_EXPORT_SYMBOL(ctc_get_learning_action);
 
int32 ctc_learning_aging_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_learning_aging_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_learning_aging_deinit);
 
int32 ctc_learning_aging_init(void* global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_learning_aging_init, lchip, global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_learning_aging_init);
 
int32 ctc_learning_clear_learning_cache(uint16 entry_vld_bitmap)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_learning_clear_learning_cache, lchip, entry_vld_bitmap);
}
CTC_EXPORT_SYMBOL(ctc_learning_clear_learning_cache);
 
int32 ctc_learning_get_cache_entry_valid_bitmap(uint16* entry_vld_bitmap)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_learning_get_cache_entry_valid_bitmap, lchip, entry_vld_bitmap);
}
CTC_EXPORT_SYMBOL(ctc_learning_get_cache_entry_valid_bitmap);
 
int32 ctc_learning_read_learning_cache(uint16 entry_vld_bitmap, ctc_learning_cache_t* l2_lc)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_learning_read_learning_cache, lchip, entry_vld_bitmap, l2_lc);
}
CTC_EXPORT_SYMBOL(ctc_learning_read_learning_cache);
 
int32 ctc_set_learning_action(ctc_learning_action_info_t* p_learning_action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_set_learning_action, lchip, p_learning_action);
}
CTC_EXPORT_SYMBOL(ctc_set_learning_action);
 
/*##linkagg##*/
int32 ctc_linkagg_add_port(uint8 tid, uint32 gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_linkagg_add_port, lchip, tid, gport);
}
CTC_EXPORT_SYMBOL(ctc_linkagg_add_port);
 
int32 ctc_linkagg_create(ctc_linkagg_group_t* p_linkagg_grp)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_linkagg_create, lchip, p_linkagg_grp);
}
CTC_EXPORT_SYMBOL(ctc_linkagg_create);
 
int32 ctc_linkagg_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_linkagg_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_linkagg_deinit);
 
int32 ctc_linkagg_destroy(uint8 tid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_linkagg_destroy, lchip, tid);
}
CTC_EXPORT_SYMBOL(ctc_linkagg_destroy);
 
int32 ctc_linkagg_get_1st_local_port(uint8 lchip_id, uint8 tid, uint32* p_gport, uint16* local_cnt)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_linkagg_get_1st_local_port, lchip_id, tid, p_gport, local_cnt);
}
CTC_EXPORT_SYMBOL(ctc_linkagg_get_1st_local_port);
 
int32 ctc_linkagg_get_max_mem_num(uint16* max_num)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_linkagg_get_max_mem_num, lchip, max_num);
}
CTC_EXPORT_SYMBOL(ctc_linkagg_get_max_mem_num);
 
int32 ctc_linkagg_get_member_ports(uint8 tid, uint32* p_gports, uint16* cnt)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_linkagg_get_member_ports, lchip, tid, p_gports, cnt);
}
CTC_EXPORT_SYMBOL(ctc_linkagg_get_member_ports);
 
int32 ctc_linkagg_get_property(uint8 tid, ctc_linkagg_property_t linkagg_prop, uint32* p_value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_linkagg_get_property, lchip, tid, linkagg_prop, p_value);
}
CTC_EXPORT_SYMBOL(ctc_linkagg_get_property);
 
int32 ctc_linkagg_get_psc(ctc_linkagg_psc_t* psc)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_linkagg_get_psc, lchip, psc);
}
CTC_EXPORT_SYMBOL(ctc_linkagg_get_psc);
 
int32 ctc_linkagg_init(void* linkagg_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_linkagg_init, lchip, linkagg_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_linkagg_init);
 
int32 ctc_linkagg_remove_port(uint8 tid, uint32 gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_linkagg_remove_port, lchip, tid, gport);
}
CTC_EXPORT_SYMBOL(ctc_linkagg_remove_port);
 
int32 ctc_linkagg_replace_ports(uint8 tid, uint32* gport, uint16 mem_num)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_linkagg_replace_ports, lchip, tid, gport, mem_num);
}
CTC_EXPORT_SYMBOL(ctc_linkagg_replace_ports);
 
int32 ctc_linkagg_set_property(uint8 tid, ctc_linkagg_property_t linkagg_prop, uint32 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_linkagg_set_property, lchip, tid, linkagg_prop, value);
}
CTC_EXPORT_SYMBOL(ctc_linkagg_set_property);
 
int32 ctc_linkagg_set_psc(ctc_linkagg_psc_t* psc)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_linkagg_set_psc, lchip, psc);
}
CTC_EXPORT_SYMBOL(ctc_linkagg_set_psc);
 
/*##mirror##*/
int32 ctc_mirror_add_session(ctc_mirror_dest_t* mirror)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mirror_add_session, lchip, mirror);
}
CTC_EXPORT_SYMBOL(ctc_mirror_add_session);
 
int32 ctc_mirror_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mirror_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_mirror_deinit);
 
int32 ctc_mirror_get_mirror_discard(ctc_direction_t dir, ctc_mirror_discard_t discard_flag, bool* enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mirror_get_mirror_discard, lchip, dir, discard_flag, enable);
}
CTC_EXPORT_SYMBOL(ctc_mirror_get_mirror_discard);
 
int32 ctc_mirror_get_port_info(uint32 gport, ctc_direction_t dir, bool* enable, uint8* session_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mirror_get_port_info, lchip, gport, dir, enable, session_id);
}
CTC_EXPORT_SYMBOL(ctc_mirror_get_port_info);
 
int32 ctc_mirror_get_vlan_info(uint16 vlan_id, ctc_direction_t dir, bool* enable, uint8* session_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mirror_get_vlan_info, lchip, vlan_id, dir, enable, session_id);
}
CTC_EXPORT_SYMBOL(ctc_mirror_get_vlan_info);
 
int32 ctc_mirror_init(void* mirror_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_mirror_init, lchip, mirror_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_mirror_init);
 
int32 ctc_mirror_remove_session(ctc_mirror_dest_t* mirror)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mirror_remove_session, lchip, mirror);
}
CTC_EXPORT_SYMBOL(ctc_mirror_remove_session);
 
int32 ctc_mirror_set_erspan_psc(ctc_mirror_erspan_psc_t* psc)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mirror_set_erspan_psc, lchip, psc);
}
CTC_EXPORT_SYMBOL(ctc_mirror_set_erspan_psc);
 
int32 ctc_mirror_set_escape_en(bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mirror_set_escape_en, lchip, enable);
}
CTC_EXPORT_SYMBOL(ctc_mirror_set_escape_en);
 
int32 ctc_mirror_set_escape_mac(ctc_mirror_rspan_escape_t* escape)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mirror_set_escape_mac, lchip, escape);
}
CTC_EXPORT_SYMBOL(ctc_mirror_set_escape_mac);
 
int32 ctc_mirror_set_mirror_discard(ctc_direction_t dir, uint16 discard_flag, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mirror_set_mirror_discard, lchip, dir, discard_flag, enable);
}
CTC_EXPORT_SYMBOL(ctc_mirror_set_mirror_discard);
 
int32 ctc_mirror_set_port_en(uint32 gport, ctc_direction_t dir, bool enable, uint8 session_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mirror_set_port_en, lchip, gport, dir, enable, session_id);
}
CTC_EXPORT_SYMBOL(ctc_mirror_set_port_en);
 
int32 ctc_mirror_set_vlan_en(uint16 vlan_id, ctc_direction_t dir, bool enable, uint8 session_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mirror_set_vlan_en, lchip, vlan_id, dir, enable, session_id);
}
CTC_EXPORT_SYMBOL(ctc_mirror_set_vlan_en);
 
/*##monitor##*/
int32 ctc_monitor_clear_watermark(ctc_monitor_watermark_t* p_watermark)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_monitor_clear_watermark, lchip, p_watermark);
}
CTC_EXPORT_SYMBOL(ctc_monitor_clear_watermark);
 
int32 ctc_monitor_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_monitor_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_monitor_deinit);
 
int32 ctc_monitor_get_config(ctc_monitor_config_t* p_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_monitor_get_config, lchip, p_cfg);
}
CTC_EXPORT_SYMBOL(ctc_monitor_get_config);
 
int32 ctc_monitor_get_watermark(ctc_monitor_watermark_t* p_watermark)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_monitor_get_watermark, lchip, p_watermark);
}
CTC_EXPORT_SYMBOL(ctc_monitor_get_watermark);
 
int32 ctc_monitor_init(void* p_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_monitor_init, lchip, p_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_monitor_init);
 
int32 ctc_monitor_register_cb(ctc_monitor_fn_t callback,void* userdata)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_monitor_register_cb, lchip, callback, userdata);
}
CTC_EXPORT_SYMBOL(ctc_monitor_register_cb);
 
int32 ctc_monitor_set_config(ctc_monitor_config_t* p_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_monitor_set_config, lchip, p_cfg);
}
CTC_EXPORT_SYMBOL(ctc_monitor_set_config);
 
int32 ctc_monitor_set_global_config(ctc_monitor_glb_cfg_t* p_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_monitor_set_global_config, lchip, p_cfg);
}
CTC_EXPORT_SYMBOL(ctc_monitor_set_global_config);
 
/*##mpls##*/
int32 ctc_mpls_add_ilm(ctc_mpls_ilm_t* p_mpls_ilm)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mpls_add_ilm, lchip, p_mpls_ilm);
}
CTC_EXPORT_SYMBOL(ctc_mpls_add_ilm);
 
int32 ctc_mpls_add_l2vpn_pw(ctc_mpls_l2vpn_pw_t* p_mpls_pw)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mpls_add_l2vpn_pw, lchip, p_mpls_pw);
}
CTC_EXPORT_SYMBOL(ctc_mpls_add_l2vpn_pw);
 
int32 ctc_mpls_add_stats(ctc_mpls_stats_index_t* stats_index)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mpls_add_stats, lchip, stats_index);
}
CTC_EXPORT_SYMBOL(ctc_mpls_add_stats);
 
int32 ctc_mpls_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mpls_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_mpls_deinit);
 
int32 ctc_mpls_del_ilm(ctc_mpls_ilm_t* p_mpls_ilm)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mpls_del_ilm, lchip, p_mpls_ilm);
}
CTC_EXPORT_SYMBOL(ctc_mpls_del_ilm);
 
int32 ctc_mpls_del_l2vpn_pw(ctc_mpls_l2vpn_pw_t* p_mpls_pw)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mpls_del_l2vpn_pw, lchip, p_mpls_pw);
}
CTC_EXPORT_SYMBOL(ctc_mpls_del_l2vpn_pw);
 
int32 ctc_mpls_get_ilm(uint32* nh_id, ctc_mpls_ilm_t* p_mpls_ilm)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mpls_get_ilm, lchip, nh_id, p_mpls_ilm);
}
CTC_EXPORT_SYMBOL(ctc_mpls_get_ilm);
 
int32 ctc_mpls_init(ctc_mpls_init_t* p_mpls_info)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_mpls_init, lchip, p_mpls_info);
}
CTC_EXPORT_SYMBOL(ctc_mpls_init);
 
int32 ctc_mpls_remove_stats(ctc_mpls_stats_index_t* stats_index)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mpls_remove_stats, lchip, stats_index);
}
CTC_EXPORT_SYMBOL(ctc_mpls_remove_stats);
 
int32 ctc_mpls_set_ilm_property(ctc_mpls_property_t* p_mpls_pro)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mpls_set_ilm_property, lchip, p_mpls_pro);
}
CTC_EXPORT_SYMBOL(ctc_mpls_set_ilm_property);
 
int32 ctc_mpls_update_ilm(ctc_mpls_ilm_t* p_mpls_ilm)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mpls_update_ilm, lchip, p_mpls_ilm);
}
CTC_EXPORT_SYMBOL(ctc_mpls_update_ilm);
 
/*##nexthop##*/
int32 ctc_nexthop_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nexthop_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_nexthop_deinit);
 
int32 ctc_nexthop_init(ctc_nh_global_cfg_t* nh_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_nexthop_init, lchip, nh_cfg);
}
CTC_EXPORT_SYMBOL(ctc_nexthop_init);
 
int32 ctc_nh_add_ecmp(uint32 nhid, ctc_nh_ecmp_nh_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_add_ecmp, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_add_ecmp);
 
int32 ctc_nh_add_iloop(uint32 nhid, ctc_loopback_nexthop_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_add_iloop, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_add_iloop);
 
int32 ctc_nh_add_ip_tunnel(uint32 nhid, ctc_ip_tunnel_nh_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_add_ip_tunnel, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_add_ip_tunnel);
 
int32 ctc_nh_add_ipuc(uint32 nhid, ctc_ip_nh_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_add_ipuc, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_add_ipuc);
 
int32 ctc_nh_add_l2uc(uint32 gport, ctc_nh_param_brguc_sub_type_t nh_type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_add_l2uc, lchip, gport, nh_type);
}
CTC_EXPORT_SYMBOL(ctc_nh_add_l2uc);
 
int32 ctc_nh_add_mcast(uint32 nhid, ctc_mcast_nh_param_group_t* p_nh_mcast_group)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_add_mcast, lchip, nhid, p_nh_mcast_group);
}
CTC_EXPORT_SYMBOL(ctc_nh_add_mcast);
 
int32 ctc_nh_add_misc(uint32 nhid, ctc_misc_nh_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_add_misc, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_add_misc);
 
int32 ctc_nh_add_mpls_tunnel_label(uint16 tunnel_id, ctc_mpls_nexthop_tunnel_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_add_mpls_tunnel_label, lchip, tunnel_id, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_add_mpls_tunnel_label);
 
int32 ctc_nh_add_mpls(uint32 nhid, ctc_mpls_nexthop_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_add_mpls, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_add_mpls);
 
int32 ctc_nh_add_nexthop_mac(uint16 arp_id, ctc_nh_nexthop_mac_param_t* p_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_add_nexthop_mac, lchip, arp_id, p_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_add_nexthop_mac);
 
int32 ctc_nh_add_rspan(uint32 nhid, ctc_rspan_nexthop_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_add_rspan, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_add_rspan);
 
int32 ctc_nh_add_trill(uint32 nhid, ctc_nh_trill_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_add_trill, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_add_trill);
 
int32 ctc_nh_add_udf_profile(ctc_nh_udf_profile_param_t* p_edit)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_add_udf_profile, lchip, p_edit);
}
CTC_EXPORT_SYMBOL(ctc_nh_add_udf_profile);
 
int32 ctc_nh_add_wlan_tunnel(uint32 nhid, ctc_nh_wlan_tunnel_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_add_wlan_tunnel, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_add_wlan_tunnel);
 
int32 ctc_nh_add_xlate(uint32 nhid, ctc_vlan_edit_nh_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_add_xlate, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_add_xlate);
 
int32 ctc_nh_get_l2uc(uint32 gport, ctc_nh_param_brguc_sub_type_t nh_type, uint32* nhid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_get_l2uc, lchip, gport, nh_type, nhid);
}
CTC_EXPORT_SYMBOL(ctc_nh_get_l2uc);
 
int32 ctc_nh_get_max_ecmp(uint16* max_ecmp)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_get_max_ecmp, lchip, max_ecmp);
}
CTC_EXPORT_SYMBOL(ctc_nh_get_max_ecmp);
 
int32 ctc_nh_get_mcast_nh(uint32 group_id, uint32* p_nhid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_get_mcast_nh, lchip, group_id, p_nhid);
}
CTC_EXPORT_SYMBOL(ctc_nh_get_mcast_nh);
 
int32 ctc_nh_get_nh_info(uint32 nhid, ctc_nh_info_t* p_nh_info)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_get_nh_info, lchip, nhid, p_nh_info);
}
CTC_EXPORT_SYMBOL(ctc_nh_get_nh_info);
 
int32 ctc_nh_get_resolved_dsnh_offset(ctc_nh_reserved_dsnh_offset_type_t type, uint32* p_offset)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_get_resolved_dsnh_offset, lchip, type, p_offset);
}
CTC_EXPORT_SYMBOL(ctc_nh_get_resolved_dsnh_offset);
 
int32 ctc_nh_remove_ecmp(uint32 nhid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_remove_ecmp, lchip, nhid);
}
CTC_EXPORT_SYMBOL(ctc_nh_remove_ecmp);
 
int32 ctc_nh_remove_iloop(uint32 nhid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_remove_iloop, lchip, nhid);
}
CTC_EXPORT_SYMBOL(ctc_nh_remove_iloop);
 
int32 ctc_nh_remove_ip_tunnel(uint32 nhid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_remove_ip_tunnel, lchip, nhid);
}
CTC_EXPORT_SYMBOL(ctc_nh_remove_ip_tunnel);
 
int32 ctc_nh_remove_ipuc(uint32 nhid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_remove_ipuc, lchip, nhid);
}
CTC_EXPORT_SYMBOL(ctc_nh_remove_ipuc);
 
int32 ctc_nh_remove_l2uc(uint32 gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_remove_l2uc, lchip, gport);
}
CTC_EXPORT_SYMBOL(ctc_nh_remove_l2uc);
 
int32 ctc_nh_remove_mcast(uint32 nhid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_remove_mcast, lchip, nhid);
}
CTC_EXPORT_SYMBOL(ctc_nh_remove_mcast);
 
int32 ctc_nh_remove_misc(uint32 nhid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_remove_misc, lchip, nhid);
}
CTC_EXPORT_SYMBOL(ctc_nh_remove_misc);
 
int32 ctc_nh_remove_mpls_tunnel_label(uint16 tunnel_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_remove_mpls_tunnel_label, lchip, tunnel_id);
}
CTC_EXPORT_SYMBOL(ctc_nh_remove_mpls_tunnel_label);
 
int32 ctc_nh_remove_mpls(uint32 nhid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_remove_mpls, lchip, nhid);
}
CTC_EXPORT_SYMBOL(ctc_nh_remove_mpls);
 
int32 ctc_nh_remove_nexthop_mac(uint16 arp_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_remove_nexthop_mac, lchip, arp_id);
}
CTC_EXPORT_SYMBOL(ctc_nh_remove_nexthop_mac);
 
int32 ctc_nh_remove_rspan(uint32 nhid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_remove_rspan, lchip, nhid);
}
CTC_EXPORT_SYMBOL(ctc_nh_remove_rspan);
 
int32 ctc_nh_remove_trill(uint32 nhid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_remove_trill, lchip, nhid);
}
CTC_EXPORT_SYMBOL(ctc_nh_remove_trill);
 
int32 ctc_nh_remove_udf_profile(uint8 profile_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_remove_udf_profile, lchip, profile_id);
}
CTC_EXPORT_SYMBOL(ctc_nh_remove_udf_profile);
 
int32 ctc_nh_remove_wlan_tunnel(uint32 nhid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_remove_wlan_tunnel, lchip, nhid);
}
CTC_EXPORT_SYMBOL(ctc_nh_remove_wlan_tunnel);
 
int32 ctc_nh_remove_xlate(uint32 nhid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_remove_xlate, lchip, nhid);
}
CTC_EXPORT_SYMBOL(ctc_nh_remove_xlate);
 
int32 ctc_nh_set_global_config(ctc_nh_global_cfg_t* p_glb_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_set_global_config, lchip, p_glb_cfg);
}
CTC_EXPORT_SYMBOL(ctc_nh_set_global_config);
 
int32 ctc_nh_set_max_ecmp(uint16 max_ecmp)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_set_max_ecmp, lchip, max_ecmp);
}
CTC_EXPORT_SYMBOL(ctc_nh_set_max_ecmp);
 
int32 ctc_nh_set_nh_drop(uint32 nhid, ctc_nh_drop_info_t* p_nh_drop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_set_nh_drop, lchip, nhid, p_nh_drop);
}
CTC_EXPORT_SYMBOL(ctc_nh_set_nh_drop);
 
int32 ctc_nh_swap_mpls_tunnel_label(uint16 old_tunnel_id,uint16 new_tunnel_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_swap_mpls_tunnel_label, lchip, old_tunnel_id, new_tunnel_id);
}
CTC_EXPORT_SYMBOL(ctc_nh_swap_mpls_tunnel_label);
 
int32 ctc_nh_traverse_mcast(ctc_nh_mcast_traverse_fn fn, ctc_nh_mcast_traverse_t* p_data)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_traverse_mcast, lchip, fn, p_data);
}
CTC_EXPORT_SYMBOL(ctc_nh_traverse_mcast);
 
int32 ctc_nh_update_ecmp(uint32 nhid, ctc_nh_ecmp_nh_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_update_ecmp, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_update_ecmp);
 
int32 ctc_nh_update_ip_tunnel(uint32 nhid, ctc_ip_tunnel_nh_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_update_ip_tunnel, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_update_ip_tunnel);
 
int32 ctc_nh_update_ipuc(uint32 nhid, ctc_ip_nh_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_update_ipuc, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_update_ipuc);
 
int32 ctc_nh_update_mcast(uint32 nhid, ctc_mcast_nh_param_group_t* p_nh_mcast_group)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_update_mcast, lchip, nhid, p_nh_mcast_group);
}
CTC_EXPORT_SYMBOL(ctc_nh_update_mcast);
 
int32 ctc_nh_update_misc(uint32 nhid, ctc_misc_nh_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_update_misc, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_update_misc);
 
int32 ctc_nh_update_mpls_tunnel_label(uint16 tunnel_id, ctc_mpls_nexthop_tunnel_param_t* p_new_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_update_mpls_tunnel_label, lchip, tunnel_id, p_new_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_update_mpls_tunnel_label);
 
int32 ctc_nh_update_mpls(uint32 nhid, ctc_mpls_nexthop_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_update_mpls, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_update_mpls);
 
int32 ctc_nh_update_nexthop_mac(uint16 arp_id, ctc_nh_nexthop_mac_param_t* p_new_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_update_nexthop_mac, lchip, arp_id, p_new_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_update_nexthop_mac);
 
int32 ctc_nh_update_trill(uint32 nhid, ctc_nh_trill_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_update_trill, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_update_trill);
 
int32 ctc_nh_update_wlan_tunnel(uint32 nhid, ctc_nh_wlan_tunnel_param_t* p_nh_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_nh_update_wlan_tunnel, lchip, nhid, p_nh_param);
}
CTC_EXPORT_SYMBOL(ctc_nh_update_wlan_tunnel);
 
/*##npm##*/
int32 ctc_npm_clear_stats(uint8 lchip_id, uint8 session_id)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_npm_clear_stats, lchip_id, session_id);
}
CTC_EXPORT_SYMBOL(ctc_npm_clear_stats);
 
int32 ctc_npm_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_npm_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_npm_deinit);
 
int32 ctc_npm_get_global_config(uint8 lchip_id, ctc_npm_global_cfg_t* p_npm)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_npm_get_global_config, lchip_id, p_npm);
}
CTC_EXPORT_SYMBOL(ctc_npm_get_global_config);
 
int32 ctc_npm_get_stats(uint8 lchip_id, uint8 session_id, ctc_npm_stats_t* p_stats)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_npm_get_stats, lchip_id, session_id, p_stats);
}
CTC_EXPORT_SYMBOL(ctc_npm_get_stats);
 
int32 ctc_npm_init(void* npm_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_npm_init, lchip, npm_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_npm_init);
 
int32 ctc_npm_set_config(uint8 lchip_id, ctc_npm_cfg_t* p_cfg)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_npm_set_config, lchip_id, p_cfg);
}
CTC_EXPORT_SYMBOL(ctc_npm_set_config);
 
int32 ctc_npm_set_global_config(uint8 lchip_id, ctc_npm_global_cfg_t* p_npm)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_npm_set_global_config, lchip_id, p_npm);
}
CTC_EXPORT_SYMBOL(ctc_npm_set_global_config);
 
int32 ctc_npm_set_transmit_en(uint8 lchip_id, uint8 session_id, uint8 enable)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_npm_set_transmit_en, lchip_id, session_id, enable);
}
CTC_EXPORT_SYMBOL(ctc_npm_set_transmit_en);
 
/*##oam##*/
int32 ctc_oam_add_lmep(ctc_oam_lmep_t* p_lmep)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_add_lmep, lchip, p_lmep);
}
CTC_EXPORT_SYMBOL(ctc_oam_add_lmep);
 
int32 ctc_oam_add_maid(ctc_oam_maid_t* p_maid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_add_maid, lchip, p_maid);
}
CTC_EXPORT_SYMBOL(ctc_oam_add_maid);
 
int32 ctc_oam_add_mip(ctc_oam_mip_t* p_mip)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_add_mip, lchip, p_mip);
}
CTC_EXPORT_SYMBOL(ctc_oam_add_mip);
 
int32 ctc_oam_add_rmep(ctc_oam_rmep_t* p_rmep)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_add_rmep, lchip, p_rmep);
}
CTC_EXPORT_SYMBOL(ctc_oam_add_rmep);
 
int32 ctc_oam_clear_trpt_stats(uint8 gchip, uint8 session_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_clear_trpt_stats, lchip, gchip, session_id);
}
CTC_EXPORT_SYMBOL(ctc_oam_clear_trpt_stats);
 
int32 ctc_oam_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_oam_deinit);
 
int32 ctc_oam_get_defect_info(uint8 lchip_id, void* p_defect_info)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_get_defect_info, lchip_id, p_defect_info);
}
CTC_EXPORT_SYMBOL(ctc_oam_get_defect_info);
 
int32 ctc_oam_get_mep_info(uint8 lchip_id, ctc_oam_mep_info_t* p_mep_info)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_get_mep_info, lchip_id, p_mep_info);
}
CTC_EXPORT_SYMBOL(ctc_oam_get_mep_info);
 
int32 ctc_oam_get_mep_info_with_key(ctc_oam_mep_info_with_key_t* p_mep_info)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_get_mep_info_with_key, lchip, p_mep_info);
}
CTC_EXPORT_SYMBOL(ctc_oam_get_mep_info_with_key);
 
int32 ctc_oam_get_stats(ctc_oam_stats_info_t* p_stat_info)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_get_stats, lchip, p_stat_info);
}
CTC_EXPORT_SYMBOL(ctc_oam_get_stats);
 
int32 ctc_oam_get_trpt_stats(uint8 gchip, uint8 session_id, ctc_oam_trpt_stats_t* p_trpt_stats)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_get_trpt_stats, lchip, gchip, session_id, p_trpt_stats);
}
CTC_EXPORT_SYMBOL(ctc_oam_get_trpt_stats);
 
int32 ctc_oam_init(ctc_oam_global_t* p_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_oam_init, lchip, p_cfg);
}
CTC_EXPORT_SYMBOL(ctc_oam_init);
 
int32 ctc_oam_remove_lmep(ctc_oam_lmep_t* p_lmep)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_remove_lmep, lchip, p_lmep);
}
CTC_EXPORT_SYMBOL(ctc_oam_remove_lmep);
 
int32 ctc_oam_remove_maid(ctc_oam_maid_t* p_maid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_remove_maid, lchip, p_maid);
}
CTC_EXPORT_SYMBOL(ctc_oam_remove_maid);
 
int32 ctc_oam_remove_mip(ctc_oam_mip_t* p_mip)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_remove_mip, lchip, p_mip);
}
CTC_EXPORT_SYMBOL(ctc_oam_remove_mip);
 
int32 ctc_oam_remove_rmep(ctc_oam_rmep_t* p_rmep)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_remove_rmep, lchip, p_rmep);
}
CTC_EXPORT_SYMBOL(ctc_oam_remove_rmep);
 
int32 ctc_oam_set_property(ctc_oam_property_t* p_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_set_property, lchip, p_prop);
}
CTC_EXPORT_SYMBOL(ctc_oam_set_property);
 
int32 ctc_oam_set_trpt_cfg(ctc_oam_trpt_t* p_throughput)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_set_trpt_cfg, lchip, p_throughput);
}
CTC_EXPORT_SYMBOL(ctc_oam_set_trpt_cfg);
 
int32 ctc_oam_set_trpt_en(uint8 gchip, uint8 session_id, uint8 enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_set_trpt_en, lchip, gchip, session_id, enable);
}
CTC_EXPORT_SYMBOL(ctc_oam_set_trpt_en);
 
int32 ctc_oam_update_lmep(ctc_oam_update_t* p_lmep)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_update_lmep, lchip, p_lmep);
}
CTC_EXPORT_SYMBOL(ctc_oam_update_lmep);
 
int32 ctc_oam_update_rmep(ctc_oam_update_t* p_rmep)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_oam_update_rmep, lchip, p_rmep);
}
CTC_EXPORT_SYMBOL(ctc_oam_update_rmep);
 
/*##overlay_tunnel##*/
int32 ctc_overlay_tunnel_add_tunnel(ctc_overlay_tunnel_param_t* p_tunnel_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_overlay_tunnel_add_tunnel, lchip, p_tunnel_param);
}
CTC_EXPORT_SYMBOL(ctc_overlay_tunnel_add_tunnel);
 
int32 ctc_overlay_tunnel_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_overlay_tunnel_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_overlay_tunnel_deinit);
 
int32 ctc_overlay_tunnel_get_fid(uint32 vn_id, uint16* p_fid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_overlay_tunnel_get_fid, lchip, vn_id, p_fid);
}
CTC_EXPORT_SYMBOL(ctc_overlay_tunnel_get_fid);
 
int32 ctc_overlay_tunnel_get_vn_id(uint16 fid, uint32* p_vn_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_overlay_tunnel_get_vn_id, lchip, fid, p_vn_id);
}
CTC_EXPORT_SYMBOL(ctc_overlay_tunnel_get_vn_id);
 
int32 ctc_overlay_tunnel_init(void* param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_overlay_tunnel_init, lchip, param);
}
CTC_EXPORT_SYMBOL(ctc_overlay_tunnel_init);
 
int32 ctc_overlay_tunnel_remove_tunnel(ctc_overlay_tunnel_param_t* p_tunnel_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_overlay_tunnel_remove_tunnel, lchip, p_tunnel_param);
}
CTC_EXPORT_SYMBOL(ctc_overlay_tunnel_remove_tunnel);
 
int32 ctc_overlay_tunnel_set_fid(uint32 vn_id, uint16 fid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_overlay_tunnel_set_fid, lchip, vn_id, fid);
}
CTC_EXPORT_SYMBOL(ctc_overlay_tunnel_set_fid);
 
int32 ctc_overlay_tunnel_update_tunnel(ctc_overlay_tunnel_param_t* p_tunnel_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_overlay_tunnel_update_tunnel, lchip, p_tunnel_param);
}
CTC_EXPORT_SYMBOL(ctc_overlay_tunnel_update_tunnel);
 
/*##packet##*/
int32 ctc_packet_create_netif(uint8 lchip_id, ctc_pkt_netif_t* p_netif)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_packet_create_netif, lchip_id, p_netif);
}
CTC_EXPORT_SYMBOL(ctc_packet_create_netif);
 
int32 ctc_packet_decap(ctc_pkt_rx_t* p_pkt_rx)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_packet_decap, lchip, p_pkt_rx);
}
CTC_EXPORT_SYMBOL(ctc_packet_decap);
 
int32 ctc_packet_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_packet_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_packet_deinit);
 
int32 ctc_packet_destory_netif(uint8 lchip_id, ctc_pkt_netif_t* p_netif)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_packet_destory_netif, lchip_id, p_netif);
}
CTC_EXPORT_SYMBOL(ctc_packet_destory_netif);
 
int32 ctc_packet_encap(ctc_pkt_tx_t* p_pkt_tx)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_packet_encap, lchip, p_pkt_tx);
}
CTC_EXPORT_SYMBOL(ctc_packet_encap);
 
int32 ctc_packet_get_netif(uint8 lchip_id, ctc_pkt_netif_t* p_netif)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_packet_get_netif, lchip_id, p_netif);
}
CTC_EXPORT_SYMBOL(ctc_packet_get_netif);
 
int32 ctc_packet_init(void* p_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_packet_init, lchip, p_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_packet_init);
 
int32 ctc_packet_set_tx_timer(uint8 lchip_id, ctc_pkt_tx_timer_t* p_pkt_timer)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_packet_set_tx_timer, lchip_id, p_pkt_timer);
}
CTC_EXPORT_SYMBOL(ctc_packet_set_tx_timer);
 
int32 ctc_packet_tx_alloc(uint32 size, void** p_addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_packet_tx_alloc, lchip, size, p_addr);
}
CTC_EXPORT_SYMBOL(ctc_packet_tx_alloc);
 
int32 ctc_packet_tx_array(ctc_pkt_tx_t** p_pkt_array, uint32 count, ctc_pkt_callback all_done_cb, void* cookie)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_packet_tx_array, lchip, p_pkt_array, count, all_done_cb, cookie);
}
CTC_EXPORT_SYMBOL(ctc_packet_tx_array);
 
int32 ctc_packet_tx_free(void* addr)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_packet_tx_free, lchip, addr);
}
CTC_EXPORT_SYMBOL(ctc_packet_tx_free);
 
int32 ctc_packet_tx(ctc_pkt_tx_t* p_pkt_tx)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_packet_tx, lchip, p_pkt_tx);
}
CTC_EXPORT_SYMBOL(ctc_packet_tx);
 
/*##parser##*/
int32 ctc_parser_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_parser_deinit);
 
int32 ctc_parser_enable_l3_type(ctc_parser_l3_type_t l3_type, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_enable_l3_type, lchip, l3_type, enable);
}
CTC_EXPORT_SYMBOL(ctc_parser_enable_l3_type);
 
int32 ctc_parser_get_ecmp_hash_field(ctc_parser_ecmp_hash_ctl_t* hash_ctl)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_get_ecmp_hash_field, lchip, hash_ctl);
}
CTC_EXPORT_SYMBOL(ctc_parser_get_ecmp_hash_field);
 
int32 ctc_parser_get_efd_hash_field(ctc_parser_efd_hash_ctl_t* hash_ctl)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_get_efd_hash_field, lchip, hash_ctl);
}
CTC_EXPORT_SYMBOL(ctc_parser_get_efd_hash_field);
 
int32 ctc_parser_get_global_cfg(ctc_parser_global_cfg_t* global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_get_global_cfg, lchip, global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_parser_get_global_cfg);
 
int32 ctc_parser_get_l2_flex_header(ctc_parser_l2flex_header_t* l2flex_header)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_get_l2_flex_header, lchip, l2flex_header);
}
CTC_EXPORT_SYMBOL(ctc_parser_get_l2_flex_header);
 
int32 ctc_parser_get_l3_flex_header(ctc_parser_l3flex_header_t* l3flex_header)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_get_l3_flex_header, lchip, l3flex_header);
}
CTC_EXPORT_SYMBOL(ctc_parser_get_l3_flex_header);
 
int32 ctc_parser_get_l4_app_ctl(ctc_parser_l4app_ctl_t* l4app_ctl)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_get_l4_app_ctl, lchip, l4app_ctl);
}
CTC_EXPORT_SYMBOL(ctc_parser_get_l4_app_ctl);
 
int32 ctc_parser_get_l4_app_data_ctl(uint8 index, ctc_parser_l4_app_data_entry_t* entry)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_get_l4_app_data_ctl, lchip, index, entry);
}
CTC_EXPORT_SYMBOL(ctc_parser_get_l4_app_data_ctl);
 
int32 ctc_parser_get_l4_flex_header(ctc_parser_l4flex_header_t* l4flex_header)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_get_l4_flex_header, lchip, l4flex_header);
}
CTC_EXPORT_SYMBOL(ctc_parser_get_l4_flex_header);
 
int32 ctc_parser_get_max_length_filed(uint16* max_length)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_get_max_length_filed, lchip, max_length);
}
CTC_EXPORT_SYMBOL(ctc_parser_get_max_length_filed);
 
int32 ctc_parser_get_pbb_header(ctc_parser_pbb_header_t* pbb_header)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_get_pbb_header, lchip, pbb_header);
}
CTC_EXPORT_SYMBOL(ctc_parser_get_pbb_header);
 
int32 ctc_parser_get_tpid(ctc_parser_l2_tpid_t type, uint16* tpid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_get_tpid, lchip, type, tpid);
}
CTC_EXPORT_SYMBOL(ctc_parser_get_tpid);
 
int32 ctc_parser_get_trill_header(ctc_parser_trill_header_t* trill_header)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_get_trill_header, lchip, trill_header);
}
CTC_EXPORT_SYMBOL(ctc_parser_get_trill_header);
 
int32 ctc_parser_get_udf(uint32 index, ctc_parser_udf_t* udf)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_get_udf, lchip, index, udf);
}
CTC_EXPORT_SYMBOL(ctc_parser_get_udf);
 
int32 ctc_parser_get_vlan_parser_num(uint8* vlan_num)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_get_vlan_parser_num, lchip, vlan_num);
}
CTC_EXPORT_SYMBOL(ctc_parser_get_vlan_parser_num);
 
int32 ctc_parser_init(void* parser_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_parser_init, lchip, parser_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_parser_init);
 
int32 ctc_parser_map_l3_type(uint8 index, ctc_parser_l2_protocol_entry_t* entry)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_map_l3_type, lchip, index, entry);
}
CTC_EXPORT_SYMBOL(ctc_parser_map_l3_type);
 
int32 ctc_parser_map_l4_type(uint8 index, ctc_parser_l3_protocol_entry_t* entry)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_map_l4_type, lchip, index, entry);
}
CTC_EXPORT_SYMBOL(ctc_parser_map_l4_type);
 
int32 ctc_parser_set_ecmp_hash_field(ctc_parser_ecmp_hash_ctl_t* hash_ctl)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_set_ecmp_hash_field, lchip, hash_ctl);
}
CTC_EXPORT_SYMBOL(ctc_parser_set_ecmp_hash_field);
 
int32 ctc_parser_set_efd_hash_field(ctc_parser_efd_hash_ctl_t* hash_ctl)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_set_efd_hash_field, lchip, hash_ctl);
}
CTC_EXPORT_SYMBOL(ctc_parser_set_efd_hash_field);
 
int32 ctc_parser_set_global_cfg(ctc_parser_global_cfg_t* global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_set_global_cfg, lchip, global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_parser_set_global_cfg);
 
int32 ctc_parser_set_l2_flex_header(ctc_parser_l2flex_header_t* l2flex_header)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_set_l2_flex_header, lchip, l2flex_header);
}
CTC_EXPORT_SYMBOL(ctc_parser_set_l2_flex_header);
 
int32 ctc_parser_set_l3_flex_header(ctc_parser_l3flex_header_t* l3flex_header)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_set_l3_flex_header, lchip, l3flex_header);
}
CTC_EXPORT_SYMBOL(ctc_parser_set_l3_flex_header);
 
int32 ctc_parser_set_l4_app_ctl(ctc_parser_l4app_ctl_t* l4app_ctl)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_set_l4_app_ctl, lchip, l4app_ctl);
}
CTC_EXPORT_SYMBOL(ctc_parser_set_l4_app_ctl);
 
int32 ctc_parser_set_l4_app_data_ctl(uint8 index, ctc_parser_l4_app_data_entry_t* entry)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_set_l4_app_data_ctl, lchip, index, entry);
}
CTC_EXPORT_SYMBOL(ctc_parser_set_l4_app_data_ctl);
 
int32 ctc_parser_set_l4_flex_header(ctc_parser_l4flex_header_t* l4flex_header)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_set_l4_flex_header, lchip, l4flex_header);
}
CTC_EXPORT_SYMBOL(ctc_parser_set_l4_flex_header);
 
int32 ctc_parser_set_max_length_field(uint16 max_length)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_set_max_length_field, lchip, max_length);
}
CTC_EXPORT_SYMBOL(ctc_parser_set_max_length_field);
 
int32 ctc_parser_set_pbb_header(ctc_parser_pbb_header_t* pbb_header)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_set_pbb_header, lchip, pbb_header);
}
CTC_EXPORT_SYMBOL(ctc_parser_set_pbb_header);
 
int32 ctc_parser_set_tpid(ctc_parser_l2_tpid_t type, uint16 tpid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_set_tpid, lchip, type, tpid);
}
CTC_EXPORT_SYMBOL(ctc_parser_set_tpid);
 
int32 ctc_parser_set_trill_header(ctc_parser_trill_header_t* trill_header)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_set_trill_header, lchip, trill_header);
}
CTC_EXPORT_SYMBOL(ctc_parser_set_trill_header);
 
int32 ctc_parser_set_udf(uint32 index, ctc_parser_udf_t* udf)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_set_udf, lchip, index, udf);
}
CTC_EXPORT_SYMBOL(ctc_parser_set_udf);
 
int32 ctc_parser_set_vlan_parser_num(uint8 vlan_num)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_set_vlan_parser_num, lchip, vlan_num);
}
CTC_EXPORT_SYMBOL(ctc_parser_set_vlan_parser_num);
 
int32 ctc_parser_unmap_l3_type(uint8 index)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_unmap_l3_type, lchip, index);
}
CTC_EXPORT_SYMBOL(ctc_parser_unmap_l3_type);
 
int32 ctc_parser_unmap_l4_type(uint8 index)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_parser_unmap_l4_type, lchip, index);
}
CTC_EXPORT_SYMBOL(ctc_parser_unmap_l4_type);
 
/*##pdu##*/
int32 ctc_l2pdu_classify_l2pdu(ctc_pdu_l2pdu_type_t l2pdu_type, uint8 index, ctc_pdu_l2pdu_key_t* key)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2pdu_classify_l2pdu, lchip, l2pdu_type, index, key);
}
CTC_EXPORT_SYMBOL(ctc_l2pdu_classify_l2pdu);
 
int32 ctc_l2pdu_get_classified_key(ctc_pdu_l2pdu_type_t l2pdu_type, uint8 index, ctc_pdu_l2pdu_key_t* key)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2pdu_get_classified_key, lchip, l2pdu_type, index, key);
}
CTC_EXPORT_SYMBOL(ctc_l2pdu_get_classified_key);
 
int32 ctc_l2pdu_get_global_action(ctc_pdu_l2pdu_type_t l2pdu_type, uint8 index, ctc_pdu_global_l2pdu_action_t* action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2pdu_get_global_action, lchip, l2pdu_type, index, action);
}
CTC_EXPORT_SYMBOL(ctc_l2pdu_get_global_action);
 
int32 ctc_l2pdu_get_port_action(uint32 gport, uint8 action_index, ctc_pdu_port_l2pdu_action_t* action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2pdu_get_port_action, lchip, gport, action_index, action);
}
CTC_EXPORT_SYMBOL(ctc_l2pdu_get_port_action);
 
int32 ctc_l2pdu_set_global_action(ctc_pdu_l2pdu_type_t l2pdu_type, uint8 index, ctc_pdu_global_l2pdu_action_t* action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2pdu_set_global_action, lchip, l2pdu_type, index, action);
}
CTC_EXPORT_SYMBOL(ctc_l2pdu_set_global_action);
 
int32 ctc_l2pdu_set_port_action(uint32 gport, uint8 action_index, ctc_pdu_port_l2pdu_action_t action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l2pdu_set_port_action, lchip, gport, action_index, action);
}
CTC_EXPORT_SYMBOL(ctc_l2pdu_set_port_action);
 
int32 ctc_l3pdu_classify_l3pdu(ctc_pdu_l3pdu_type_t l3pdu_type, uint8 index, ctc_pdu_l3pdu_key_t* key)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3pdu_classify_l3pdu, lchip, l3pdu_type, index, key);
}
CTC_EXPORT_SYMBOL(ctc_l3pdu_classify_l3pdu);
 
int32 ctc_l3pdu_get_classified_key(ctc_pdu_l3pdu_type_t l3pdu_type, uint8 index, ctc_pdu_l3pdu_key_t* key)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3pdu_get_classified_key, lchip, l3pdu_type, index, key);
}
CTC_EXPORT_SYMBOL(ctc_l3pdu_get_classified_key);
 
int32 ctc_l3pdu_get_global_action(ctc_pdu_l3pdu_type_t l3pdu_type, uint8 index, ctc_pdu_global_l3pdu_action_t* action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3pdu_get_global_action, lchip, l3pdu_type, index, action);
}
CTC_EXPORT_SYMBOL(ctc_l3pdu_get_global_action);
 
int32 ctc_l3pdu_get_l3if_action(uint16 l3ifid, uint8 action_index, ctc_pdu_l3if_l3pdu_action_t* action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3pdu_get_l3if_action, lchip, l3ifid, action_index, action);
}
CTC_EXPORT_SYMBOL(ctc_l3pdu_get_l3if_action);
 
int32 ctc_l3pdu_set_global_action(ctc_pdu_l3pdu_type_t l3pdu_type, uint8 index, ctc_pdu_global_l3pdu_action_t* action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3pdu_set_global_action, lchip, l3pdu_type, index, action);
}
CTC_EXPORT_SYMBOL(ctc_l3pdu_set_global_action);
 
int32 ctc_l3pdu_set_l3if_action(uint16 l3ifid, uint8 action_index, ctc_pdu_l3if_l3pdu_action_t action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_l3pdu_set_l3if_action, lchip, l3ifid, action_index, action);
}
CTC_EXPORT_SYMBOL(ctc_l3pdu_set_l3if_action);
 
int32 ctc_pdu_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_pdu_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_pdu_deinit);
 
int32 ctc_pdu_init(void* pdu_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_pdu_init, lchip, pdu_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_pdu_init);
 
/*##port##*/
int32 ctc_get_ipg_size(ctc_ipg_size_t index, uint8* p_size)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_get_ipg_size, lchip, index, p_size);
}
CTC_EXPORT_SYMBOL(ctc_get_ipg_size);
 
int32 ctc_get_max_frame_size(ctc_frame_size_t index, uint16* p_value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_get_max_frame_size, lchip, index, p_value);
}
CTC_EXPORT_SYMBOL(ctc_get_max_frame_size);
 
int32 ctc_get_min_frame_size(ctc_frame_size_t index, uint16* p_value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_get_min_frame_size, lchip, index, p_value);
}
CTC_EXPORT_SYMBOL(ctc_get_min_frame_size);
 
int32 ctc_port_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_port_deinit);
 
int32 ctc_port_get_acl_property(uint32 gport, ctc_acl_property_t* p_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_acl_property, lchip, gport, p_prop);
}
CTC_EXPORT_SYMBOL(ctc_port_get_acl_property);
 
int32 ctc_port_get_bpe_property(uint32 gport, ctc_port_bpe_property_t* p_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_bpe_property, lchip, gport, p_prop);
}
CTC_EXPORT_SYMBOL(ctc_port_get_bpe_property);
 
int32 ctc_port_get_bridge_en(uint32 gport, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_bridge_en, lchip, gport, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_bridge_en);
 
int32 ctc_port_get_capability(uint32 gport, ctc_port_capability_type_t type, void* p_value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_capability, lchip, gport, type, p_value);
}
CTC_EXPORT_SYMBOL(ctc_port_get_capability);
 
int32 ctc_port_get_cpu_mac_en(bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_cpu_mac_en, lchip, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_cpu_mac_en);
 
int32 ctc_port_get_cross_connect(uint32 gport, uint32* p_value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_cross_connect, lchip, gport, p_value);
}
CTC_EXPORT_SYMBOL(ctc_port_get_cross_connect);
 
int32 ctc_port_get_default_vlan(uint32 gport, uint16* p_vid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_default_vlan, lchip, gport, p_vid);
}
CTC_EXPORT_SYMBOL(ctc_port_get_default_vlan);
 
int32 ctc_port_get_direction_property(uint32 gport, ctc_port_direction_property_t port_prop, ctc_direction_t dir, uint32* p_value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_direction_property, lchip, gport, port_prop, dir, p_value);
}
CTC_EXPORT_SYMBOL(ctc_port_get_direction_property);
 
int32 ctc_port_get_dot1q_type(uint32 gport, ctc_dot1q_type_t* p_type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_dot1q_type, lchip, gport, p_type);
}
CTC_EXPORT_SYMBOL(ctc_port_get_dot1q_type);
 
int32 ctc_port_get_flow_ctl_en(ctc_port_fc_prop_t* p_fc_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_flow_ctl_en, lchip, p_fc_prop);
}
CTC_EXPORT_SYMBOL(ctc_port_get_flow_ctl_en);
 
int32 ctc_port_get_ipg(uint32 gport, ctc_ipg_size_t* p_index)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_ipg, lchip, gport, p_index);
}
CTC_EXPORT_SYMBOL(ctc_port_get_ipg);
 
int32 ctc_port_get_isolation(uint8 lchip_id, ctc_port_isolation_t* p_port_isolation)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_isolation, lchip_id, p_port_isolation);
}
CTC_EXPORT_SYMBOL(ctc_port_get_isolation);
 
int32 ctc_port_get_learning_en(uint32 gport, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_learning_en, lchip, gport, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_learning_en);
 
int32 ctc_port_get_mac_auth(uint32 gport, bool* enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_mac_auth, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_mac_auth);
 
int32 ctc_port_get_mac_en(uint32 gport, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_mac_en, lchip, gport, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_mac_en);
 
int32 ctc_port_get_mac_link_up(uint32 gport, bool* p_is_up)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_mac_link_up, lchip, gport, p_is_up);
}
CTC_EXPORT_SYMBOL(ctc_port_get_mac_link_up);
 
int32 ctc_port_get_max_frame(uint32 gport, uint32* p_value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_max_frame, lchip, gport, p_value);
}
CTC_EXPORT_SYMBOL(ctc_port_get_max_frame);
 
int32 ctc_port_get_min_frame_size(uint32 gport, uint8* p_size)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_min_frame_size, lchip, gport, p_size);
}
CTC_EXPORT_SYMBOL(ctc_port_get_min_frame_size);
 
int32 ctc_port_get_pading_en(uint32 gport, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_pading_en, lchip, gport, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_pading_en);
 
int32 ctc_port_get_phy_if_en(uint32 gport, uint16* p_l3if_id, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_phy_if_en, lchip, gport, p_l3if_id, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_phy_if_en);
 
int32 ctc_port_get_port_check_en(uint32 gport, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_port_check_en, lchip, gport, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_port_check_en);
 
int32 ctc_port_get_port_en(uint32 gport, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_port_en, lchip, gport, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_port_en);
 
int32 ctc_port_get_port_mac(uint32 gport, mac_addr_t* p_port_mac)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_port_mac, lchip, gport, p_port_mac);
}
CTC_EXPORT_SYMBOL(ctc_port_get_port_mac);
 
int32 ctc_port_get_preamble(uint32 gport, uint8* p_pre_bytes)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_preamble, lchip, gport, p_pre_bytes);
}
CTC_EXPORT_SYMBOL(ctc_port_get_preamble);
 
int32 ctc_port_get_property(uint32 gport, ctc_port_property_t port_prop, uint32* p_value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_property, lchip, gport, port_prop, p_value);
}
CTC_EXPORT_SYMBOL(ctc_port_get_property);
 
int32 ctc_port_get_protocol_vlan_en(uint32 gport, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_protocol_vlan_en, lchip, gport, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_protocol_vlan_en);
 
int32 ctc_port_get_random_log_en(uint32 gport, ctc_direction_t dir, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_random_log_en, lchip, gport, dir, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_random_log_en);
 
int32 ctc_port_get_random_log_percent(uint32 gport, ctc_direction_t dir, uint8* p_percent)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_random_log_percent, lchip, gport, dir, p_percent);
}
CTC_EXPORT_SYMBOL(ctc_port_get_random_log_percent);
 
int32 ctc_port_get_receive_en(uint32 gport, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_receive_en, lchip, gport, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_receive_en);
 
int32 ctc_port_get_reflective_bridge_en(uint32 gport, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_reflective_bridge_en, lchip, gport, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_reflective_bridge_en);
 
int32 ctc_port_get_restriction(uint32 gport, ctc_port_restriction_t* p_restriction)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_restriction, lchip, gport, p_restriction);
}
CTC_EXPORT_SYMBOL(ctc_port_get_restriction);
 
int32 ctc_port_get_scl_key_type(uint32 gport, ctc_direction_t dir, uint8 scl_id, ctc_port_scl_key_type_t* p_type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_scl_key_type, lchip, gport, dir, scl_id, p_type);
}
CTC_EXPORT_SYMBOL(ctc_port_get_scl_key_type);
 
int32 ctc_port_get_scl_property(uint32 gport, ctc_port_scl_property_t* prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_scl_property, lchip, gport, prop);
}
CTC_EXPORT_SYMBOL(ctc_port_get_scl_property);
 
int32 ctc_port_get_speed(uint32 gport, ctc_port_speed_t* p_speed_mode)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_speed, lchip, gport, p_speed_mode);
}
CTC_EXPORT_SYMBOL(ctc_port_get_speed);
 
int32 ctc_port_get_srcdiscard_en(uint32 gport, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_srcdiscard_en, lchip, gport, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_srcdiscard_en);
 
int32 ctc_port_get_stag_tpid_index(uint32 gport, ctc_direction_t dir, uint8* p_index)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_stag_tpid_index, lchip, gport, dir, p_index);
}
CTC_EXPORT_SYMBOL(ctc_port_get_stag_tpid_index);
 
int32 ctc_port_get_sub_if_en(uint32 gport, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_sub_if_en, lchip, gport, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_sub_if_en);
 
int32 ctc_port_get_transmit_en(uint32 gport, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_transmit_en, lchip, gport, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_transmit_en);
 
int32 ctc_port_get_untag_dft_vid(uint32 gport, bool* p_enable, bool* p_untag_svlan)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_untag_dft_vid, lchip, gport, p_enable, p_untag_svlan);
}
CTC_EXPORT_SYMBOL(ctc_port_get_untag_dft_vid);
 
int32 ctc_port_get_use_outer_ttl(uint32 gport, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_use_outer_ttl, lchip, gport, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_use_outer_ttl);
 
int32 ctc_port_get_vlan_ctl(uint32 gport, ctc_vlantag_ctl_t* p_mode)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_vlan_ctl, lchip, gport, p_mode);
}
CTC_EXPORT_SYMBOL(ctc_port_get_vlan_ctl);
 
int32 ctc_port_get_vlan_domain(uint32 gport, ctc_port_vlan_domain_type_t* p_vlan_domain)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_vlan_domain, lchip, gport, p_vlan_domain);
}
CTC_EXPORT_SYMBOL(ctc_port_get_vlan_domain);
 
int32 ctc_port_get_vlan_filter_en(uint32 gport, ctc_direction_t dir, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_vlan_filter_en, lchip, gport, dir, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_vlan_filter_en);
 
int32 ctc_port_get_vlan_range(uint32 gport, ctc_vlan_range_info_t* p_vrange_info, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_get_vlan_range, lchip, gport, p_vrange_info, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_get_vlan_range);
 
int32 ctc_port_init(void* p_port_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_port_init, lchip, p_port_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_port_init);
 
int32 ctc_port_set_acl_property(uint32 gport, ctc_acl_property_t* p_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_acl_property, lchip, gport, p_prop);
}
CTC_EXPORT_SYMBOL(ctc_port_set_acl_property);
 
int32 ctc_port_set_auto_neg(uint32 gport, uint32 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_auto_neg, lchip, gport, value);
}
CTC_EXPORT_SYMBOL(ctc_port_set_auto_neg);
 
int32 ctc_port_set_bpe_property(uint32 gport, ctc_port_bpe_property_t* p_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_bpe_property, lchip, gport, p_prop);
}
CTC_EXPORT_SYMBOL(ctc_port_set_bpe_property);
 
int32 ctc_port_set_bridge_en(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_bridge_en, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_bridge_en);
 
int32 ctc_port_set_cpu_mac_en(bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_cpu_mac_en, lchip, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_cpu_mac_en);
 
int32 ctc_port_set_cross_connect(uint32 gport, uint32 nhid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_cross_connect, lchip, gport, nhid);
}
CTC_EXPORT_SYMBOL(ctc_port_set_cross_connect);
 
int32 ctc_port_set_default_cfg(uint32 gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_default_cfg, lchip, gport);
}
CTC_EXPORT_SYMBOL(ctc_port_set_default_cfg);
 
int32 ctc_port_set_default_vlan(uint32 gport, uint16 vid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_default_vlan, lchip, gport, vid);
}
CTC_EXPORT_SYMBOL(ctc_port_set_default_vlan);
 
int32 ctc_port_set_direction_property(uint32 gport, ctc_port_direction_property_t port_prop, ctc_direction_t dir, uint32 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_direction_property, lchip, gport, port_prop, dir, value);
}
CTC_EXPORT_SYMBOL(ctc_port_set_direction_property);
 
int32 ctc_port_set_dot1q_type(uint32 gport, ctc_dot1q_type_t type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_dot1q_type, lchip, gport, type);
}
CTC_EXPORT_SYMBOL(ctc_port_set_dot1q_type);
 
int32 ctc_port_set_flow_ctl_en(ctc_port_fc_prop_t* p_fc_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_flow_ctl_en, lchip, p_fc_prop);
}
CTC_EXPORT_SYMBOL(ctc_port_set_flow_ctl_en);
 
int32 ctc_port_set_interface_mode(uint32 gport, ctc_port_if_mode_t* if_mode)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_interface_mode, lchip, gport, if_mode);
}
CTC_EXPORT_SYMBOL(ctc_port_set_interface_mode);
 
int32 ctc_port_set_ipg(uint32 gport, ctc_ipg_size_t index)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_ipg, lchip, gport, index);
}
CTC_EXPORT_SYMBOL(ctc_port_set_ipg);
 
int32 ctc_port_set_isolation(uint8 lchip_id, ctc_port_isolation_t* p_port_isolation)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_isolation, lchip_id, p_port_isolation);
}
CTC_EXPORT_SYMBOL(ctc_port_set_isolation);
 
int32 ctc_port_set_learning_en(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_learning_en, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_learning_en);
 
int32 ctc_port_set_link_intr(uint32 gport, uint32 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_link_intr, lchip, gport, value);
}
CTC_EXPORT_SYMBOL(ctc_port_set_link_intr);
 
int32 ctc_port_set_loopback(ctc_port_lbk_param_t* p_port_lbk)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_loopback, lchip, p_port_lbk);
}
CTC_EXPORT_SYMBOL(ctc_port_set_loopback);
 
int32 ctc_port_set_mac_auth(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_mac_auth, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_mac_auth);
 
int32 ctc_port_set_mac_en(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_mac_en, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_mac_en);
 
int32 ctc_port_set_max_frame(uint32 gport, uint32 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_max_frame, lchip, gport, value);
}
CTC_EXPORT_SYMBOL(ctc_port_set_max_frame);
 
int32 ctc_port_set_min_frame_size(uint32 gport, uint8 size)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_min_frame_size, lchip, gport, size);
}
CTC_EXPORT_SYMBOL(ctc_port_set_min_frame_size);
 
int32 ctc_port_set_pading_en(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_pading_en, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_pading_en);
 
int32 ctc_port_set_phy_if_en(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_phy_if_en, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_phy_if_en);
 
int32 ctc_port_set_port_check_en(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_port_check_en, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_port_check_en);
 
int32 ctc_port_set_port_en(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_port_en, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_port_en);
 
int32 ctc_port_set_port_mac_postfix(uint32 gport, ctc_port_mac_postfix_t* p_port_mac)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_port_mac_postfix, lchip, gport, p_port_mac);
}
CTC_EXPORT_SYMBOL(ctc_port_set_port_mac_postfix);
 
int32 ctc_port_set_port_mac_prefix(ctc_port_mac_prefix_t* p_port_mac)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_port_mac_prefix, lchip, p_port_mac);
}
CTC_EXPORT_SYMBOL(ctc_port_set_port_mac_prefix);
 
int32 ctc_port_set_preamble(uint32 gport, uint8 pre_bytes)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_preamble, lchip, gport, pre_bytes);
}
CTC_EXPORT_SYMBOL(ctc_port_set_preamble);
 
int32 ctc_port_set_property_array(uint32 gport, ctc_property_array_t* port_prop, uint16* array_num)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_property_array, lchip, gport, port_prop, array_num);
}
CTC_EXPORT_SYMBOL(ctc_port_set_property_array);
 
int32 ctc_port_set_property(uint32 gport, ctc_port_property_t port_prop, uint32 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_property, lchip, gport, port_prop, value);
}
CTC_EXPORT_SYMBOL(ctc_port_set_property);
 
int32 ctc_port_set_protocol_vlan_en(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_protocol_vlan_en, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_protocol_vlan_en);
 
int32 ctc_port_set_random_log_en(uint32 gport, ctc_direction_t dir, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_random_log_en, lchip, gport, dir, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_random_log_en);
 
int32 ctc_port_set_random_log_percent(uint32 gport, ctc_direction_t dir, uint8 percent)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_random_log_percent, lchip, gport, dir, percent);
}
CTC_EXPORT_SYMBOL(ctc_port_set_random_log_percent);
 
int32 ctc_port_set_receive_en(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_receive_en, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_receive_en);
 
int32 ctc_port_set_reflective_bridge_en(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_reflective_bridge_en, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_reflective_bridge_en);
 
int32 ctc_port_set_restriction(uint32 gport, ctc_port_restriction_t* p_restriction)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_restriction, lchip, gport, p_restriction);
}
CTC_EXPORT_SYMBOL(ctc_port_set_restriction);
 
int32 ctc_port_set_scl_key_type(uint32 gport, ctc_direction_t dir, uint8 scl_id, ctc_port_scl_key_type_t type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_scl_key_type, lchip, gport, dir, scl_id, type);
}
CTC_EXPORT_SYMBOL(ctc_port_set_scl_key_type);
 
int32 ctc_port_set_scl_property(uint32 gport, ctc_port_scl_property_t* prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_scl_property, lchip, gport, prop);
}
CTC_EXPORT_SYMBOL(ctc_port_set_scl_property);
 
int32 ctc_port_set_speed(uint32 gport, ctc_port_speed_t speed_mode)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_speed, lchip, gport, speed_mode);
}
CTC_EXPORT_SYMBOL(ctc_port_set_speed);
 
int32 ctc_port_set_srcdiscard_en(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_srcdiscard_en, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_srcdiscard_en);
 
int32 ctc_port_set_stag_tpid_index(uint32 gport, ctc_direction_t dir, uint8 index)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_stag_tpid_index, lchip, gport, dir, index);
}
CTC_EXPORT_SYMBOL(ctc_port_set_stag_tpid_index);
 
int32 ctc_port_set_sub_if_en(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_sub_if_en, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_sub_if_en);
 
int32 ctc_port_set_transmit_en(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_transmit_en, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_transmit_en);
 
int32 ctc_port_set_untag_dft_vid(uint32 gport, bool enable, bool untag_svlan)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_untag_dft_vid, lchip, gport, enable, untag_svlan);
}
CTC_EXPORT_SYMBOL(ctc_port_set_untag_dft_vid);
 
int32 ctc_port_set_use_outer_ttl(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_use_outer_ttl, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_use_outer_ttl);
 
int32 ctc_port_set_vlan_ctl(uint32 gport, ctc_vlantag_ctl_t mode)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_vlan_ctl, lchip, gport, mode);
}
CTC_EXPORT_SYMBOL(ctc_port_set_vlan_ctl);
 
int32 ctc_port_set_vlan_domain(uint32 gport, ctc_port_vlan_domain_type_t vlan_domain)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_vlan_domain, lchip, gport, vlan_domain);
}
CTC_EXPORT_SYMBOL(ctc_port_set_vlan_domain);
 
int32 ctc_port_set_vlan_filter_en(uint32 gport, ctc_direction_t dir, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_vlan_filter_en, lchip, gport, dir, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_vlan_filter_en);
 
int32 ctc_port_set_vlan_range(uint32 gport, ctc_vlan_range_info_t* p_vrange_info, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_set_vlan_range, lchip, gport, p_vrange_info, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_set_vlan_range);
 
int32 ctc_set_ipg_size(ctc_ipg_size_t index, uint8 size)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_set_ipg_size, lchip, index, size);
}
CTC_EXPORT_SYMBOL(ctc_set_ipg_size);
 
int32 ctc_set_max_frame_size(ctc_frame_size_t index, uint16 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_set_max_frame_size, lchip, index, value);
}
CTC_EXPORT_SYMBOL(ctc_set_max_frame_size);
 
int32 ctc_set_min_frame_size(ctc_frame_size_t index, uint16 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_set_min_frame_size, lchip, index, value);
}
CTC_EXPORT_SYMBOL(ctc_set_min_frame_size);
 
/*##ptp##*/
int32 ctc_ptp_add_device_clock(ctc_ptp_clock_t* clock)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_add_device_clock, lchip, clock);
}
CTC_EXPORT_SYMBOL(ctc_ptp_add_device_clock);
 
int32 ctc_ptp_adjust_clock_offset(uint8 lchip_id, ctc_ptp_time_t* p_offset)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_adjust_clock_offset, lchip_id, p_offset);
}
CTC_EXPORT_SYMBOL(ctc_ptp_adjust_clock_offset);
 
int32 ctc_ptp_clear_sync_intf_code()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_clear_sync_intf_code, lchip);
}
CTC_EXPORT_SYMBOL(ctc_ptp_clear_sync_intf_code);
 
int32 ctc_ptp_clear_tod_intf_code()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_clear_tod_intf_code, lchip);
}
CTC_EXPORT_SYMBOL(ctc_ptp_clear_tod_intf_code);
 
int32 ctc_ptp_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_ptp_deinit);
 
int32 ctc_ptp_get_adjust_delay(uint32 gport, ctc_ptp_adjust_delay_type_t type, int64* value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_get_adjust_delay, lchip, gport, type, value);
}
CTC_EXPORT_SYMBOL(ctc_ptp_get_adjust_delay);
 
int32 ctc_ptp_get_captured_ts(uint8 lchip_id, ctc_ptp_capured_ts_t* p_captured_ts)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_get_captured_ts, lchip_id, p_captured_ts);
}
CTC_EXPORT_SYMBOL(ctc_ptp_get_captured_ts);
 
int32 ctc_ptp_get_clock_drift(uint8 lchip_id, ctc_ptp_time_t* p_drift)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_get_clock_drift, lchip_id, p_drift);
}
CTC_EXPORT_SYMBOL(ctc_ptp_get_clock_drift);
 
int32 ctc_ptp_get_clock_timestamp(uint8 lchip_id, ctc_ptp_time_t* p_timestamp)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_get_clock_timestamp, lchip_id, p_timestamp);
}
CTC_EXPORT_SYMBOL(ctc_ptp_get_clock_timestamp);
 
int32 ctc_ptp_get_device_type(ctc_ptp_device_type_t* p_device_type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_get_device_type, lchip, p_device_type);
}
CTC_EXPORT_SYMBOL(ctc_ptp_get_device_type);
 
int32 ctc_ptp_get_global_property(ctc_ptp_global_prop_t property, uint32* value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_get_global_property, lchip, property, value);
}
CTC_EXPORT_SYMBOL(ctc_ptp_get_global_property);
 
int32 ctc_ptp_get_sync_intf_code(uint8 lchip_id, ctc_ptp_sync_intf_code_t* p_time_code)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_get_sync_intf_code, lchip_id, p_time_code);
}
CTC_EXPORT_SYMBOL(ctc_ptp_get_sync_intf_code);
 
int32 ctc_ptp_get_sync_intf_toggle_time(uint8 lchip_id, ctc_ptp_time_t* p_toggle_time)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_get_sync_intf_toggle_time, lchip_id, p_toggle_time);
}
CTC_EXPORT_SYMBOL(ctc_ptp_get_sync_intf_toggle_time);
 
int32 ctc_ptp_get_sync_intf(uint8 lchip_id, ctc_ptp_sync_intf_cfg_t* p_sync_cfg)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_get_sync_intf, lchip_id, p_sync_cfg);
}
CTC_EXPORT_SYMBOL(ctc_ptp_get_sync_intf);
 
int32 ctc_ptp_get_tod_intf_rx_code(uint8 lchip_id, ctc_ptp_tod_intf_code_t* p_rx_code)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_get_tod_intf_rx_code, lchip_id, p_rx_code);
}
CTC_EXPORT_SYMBOL(ctc_ptp_get_tod_intf_rx_code);
 
int32 ctc_ptp_get_tod_intf_tx_code(uint8 lchip_id, ctc_ptp_tod_intf_code_t* p_tx_code)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_get_tod_intf_tx_code, lchip_id, p_tx_code);
}
CTC_EXPORT_SYMBOL(ctc_ptp_get_tod_intf_tx_code);
 
int32 ctc_ptp_get_tod_intf(uint8 lchip_id, ctc_ptp_tod_intf_cfg_t* p_tod_cfg)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_get_tod_intf, lchip_id, p_tod_cfg);
}
CTC_EXPORT_SYMBOL(ctc_ptp_get_tod_intf);
 
int32 ctc_ptp_init(ctc_ptp_global_config_t* p_ptp_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_ptp_init, lchip, p_ptp_cfg);
}
CTC_EXPORT_SYMBOL(ctc_ptp_init);
 
int32 ctc_ptp_remove_device_clock(ctc_ptp_clock_t* clock)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_remove_device_clock, lchip, clock);
}
CTC_EXPORT_SYMBOL(ctc_ptp_remove_device_clock);
 
int32 ctc_ptp_set_adjust_delay(uint32 gport, ctc_ptp_adjust_delay_type_t type, int64 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_set_adjust_delay, lchip, gport, type, value);
}
CTC_EXPORT_SYMBOL(ctc_ptp_set_adjust_delay);
 
int32 ctc_ptp_set_clock_drift(uint8 lchip_id, ctc_ptp_time_t* p_drift)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_set_clock_drift, lchip_id, p_drift);
}
CTC_EXPORT_SYMBOL(ctc_ptp_set_clock_drift);
 
int32 ctc_ptp_set_device_type(ctc_ptp_device_type_t device_type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_set_device_type, lchip, device_type);
}
CTC_EXPORT_SYMBOL(ctc_ptp_set_device_type);
 
int32 ctc_ptp_set_global_property(ctc_ptp_global_prop_t property, uint32 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_set_global_property, lchip, property, value);
}
CTC_EXPORT_SYMBOL(ctc_ptp_set_global_property);
 
int32 ctc_ptp_set_sync_intf_toggle_time(uint8 lchip_id, ctc_ptp_time_t* p_toggle_time)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_set_sync_intf_toggle_time, lchip_id, p_toggle_time);
}
CTC_EXPORT_SYMBOL(ctc_ptp_set_sync_intf_toggle_time);
 
int32 ctc_ptp_set_sync_intf(uint8 lchip_id, ctc_ptp_sync_intf_cfg_t* p_sync_cfg)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_set_sync_intf, lchip_id, p_sync_cfg);
}
CTC_EXPORT_SYMBOL(ctc_ptp_set_sync_intf);
 
int32 ctc_ptp_set_tod_intf_tx_code(uint8 lchip_id, ctc_ptp_tod_intf_code_t* p_tx_code)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_set_tod_intf_tx_code, lchip_id, p_tx_code);
}
CTC_EXPORT_SYMBOL(ctc_ptp_set_tod_intf_tx_code);
 
int32 ctc_ptp_set_tod_intf(uint8 lchip_id, ctc_ptp_tod_intf_cfg_t* p_tod_cfg)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ptp_set_tod_intf, lchip_id, p_tod_cfg);
}
CTC_EXPORT_SYMBOL(ctc_ptp_set_tod_intf);
 
/*##qos##*/
int32 ctc_qos_clear_policer_stats(ctc_qos_policer_stats_t* p_policer_stats)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_clear_policer_stats, lchip, p_policer_stats);
}
CTC_EXPORT_SYMBOL(ctc_qos_clear_policer_stats);
 
int32 ctc_qos_clear_queue_stats(ctc_qos_queue_stats_t* p_queue_stats)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_clear_queue_stats, lchip, p_queue_stats);
}
CTC_EXPORT_SYMBOL(ctc_qos_clear_queue_stats);
 
int32 ctc_qos_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_qos_deinit);
 
int32 ctc_qos_get_domain_map(ctc_qos_domain_map_t* p_domain_map)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_get_domain_map, lchip, p_domain_map);
}
CTC_EXPORT_SYMBOL(ctc_qos_get_domain_map);
 
int32 ctc_qos_get_drop_scheme(ctc_qos_drop_t* p_drop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_get_drop_scheme, lchip, p_drop);
}
CTC_EXPORT_SYMBOL(ctc_qos_get_drop_scheme);
 
int32 ctc_qos_get_global_config(ctc_qos_glb_cfg_t* p_glb_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_get_global_config, lchip, p_glb_cfg);
}
CTC_EXPORT_SYMBOL(ctc_qos_get_global_config);
 
int32 ctc_qos_get_policer(ctc_qos_policer_t* p_policer)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_get_policer, lchip, p_policer);
}
CTC_EXPORT_SYMBOL(ctc_qos_get_policer);
 
int32 ctc_qos_get_queue(ctc_qos_queue_cfg_t* p_que_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_get_queue, lchip, p_que_cfg);
}
CTC_EXPORT_SYMBOL(ctc_qos_get_queue);
 
int32 ctc_qos_get_resrc(ctc_qos_resrc_t* p_resrc)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_get_resrc, lchip, p_resrc);
}
CTC_EXPORT_SYMBOL(ctc_qos_get_resrc);
 
int32 ctc_qos_get_sched(ctc_qos_sched_t* p_sched)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_get_sched, lchip, p_sched);
}
CTC_EXPORT_SYMBOL(ctc_qos_get_sched);
 
int32 ctc_qos_get_shape(ctc_qos_shape_t* p_shape)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_get_shape, lchip, p_shape);
}
CTC_EXPORT_SYMBOL(ctc_qos_get_shape);
 
int32 ctc_qos_init(void* p_glb_parm)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_qos_init, lchip, p_glb_parm);
}
CTC_EXPORT_SYMBOL(ctc_qos_init);
 
int32 ctc_qos_query_policer_stats(ctc_qos_policer_stats_t* p_policer_stats)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_query_policer_stats, lchip, p_policer_stats);
}
CTC_EXPORT_SYMBOL(ctc_qos_query_policer_stats);
 
int32 ctc_qos_query_pool_stats(ctc_qos_resrc_pool_stats_t* p_stats)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_query_pool_stats, lchip, p_stats);
}
CTC_EXPORT_SYMBOL(ctc_qos_query_pool_stats);
 
int32 ctc_qos_query_queue_stats(ctc_qos_queue_stats_t* p_queue_stats)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_query_queue_stats, lchip, p_queue_stats);
}
CTC_EXPORT_SYMBOL(ctc_qos_query_queue_stats);
 
int32 ctc_qos_set_domain_map(ctc_qos_domain_map_t* p_domain_map)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_set_domain_map, lchip, p_domain_map);
}
CTC_EXPORT_SYMBOL(ctc_qos_set_domain_map);
 
int32 ctc_qos_set_drop_scheme(ctc_qos_drop_t* p_drop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_set_drop_scheme, lchip, p_drop);
}
CTC_EXPORT_SYMBOL(ctc_qos_set_drop_scheme);
 
int32 ctc_qos_set_global_config(ctc_qos_glb_cfg_t* p_glb_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_set_global_config, lchip, p_glb_cfg);
}
CTC_EXPORT_SYMBOL(ctc_qos_set_global_config);
 
int32 ctc_qos_set_policer(ctc_qos_policer_t* p_policer)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_set_policer, lchip, p_policer);
}
CTC_EXPORT_SYMBOL(ctc_qos_set_policer);
 
int32 ctc_qos_set_queue(ctc_qos_queue_cfg_t* p_que_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_set_queue, lchip, p_que_cfg);
}
CTC_EXPORT_SYMBOL(ctc_qos_set_queue);
 
int32 ctc_qos_set_resrc(ctc_qos_resrc_t* p_resrc)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_set_resrc, lchip, p_resrc);
}
CTC_EXPORT_SYMBOL(ctc_qos_set_resrc);
 
int32 ctc_qos_set_sched(ctc_qos_sched_t* p_sched)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_set_sched, lchip, p_sched);
}
CTC_EXPORT_SYMBOL(ctc_qos_set_sched);
 
int32 ctc_qos_set_shape(ctc_qos_shape_t* p_shape)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_qos_set_shape, lchip, p_shape);
}
CTC_EXPORT_SYMBOL(ctc_qos_set_shape);
 
/*##register##*/
int32 ctc_global_ctl_get(ctc_global_control_type_t type, void* value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_global_ctl_get, lchip, type, value);
}
CTC_EXPORT_SYMBOL(ctc_global_ctl_get);
 
int32 ctc_global_ctl_set(ctc_global_control_type_t type, void* value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_global_ctl_set, lchip, type, value);
}
CTC_EXPORT_SYMBOL(ctc_global_ctl_set);
 
int32 ctc_register_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_register_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_register_deinit);
 
int32 ctc_register_init(void* p_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_register_init, lchip, p_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_register_init);
 
/*##scl##*/
int32 ctc_scl_add_action_field(uint32 entry_id, ctc_scl_field_action_t* p_field_action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_add_action_field, lchip, entry_id, p_field_action);
}
CTC_EXPORT_SYMBOL(ctc_scl_add_action_field);
 
int32 ctc_scl_add_entry(uint32 group_id, ctc_scl_entry_t* scl_entry)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_add_entry, lchip, group_id, scl_entry);
}
CTC_EXPORT_SYMBOL(ctc_scl_add_entry);
 
int32 ctc_scl_add_key_field(uint32 entry_id, ctc_field_key_t* p_field_key)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_add_key_field, lchip, entry_id, p_field_key);
}
CTC_EXPORT_SYMBOL(ctc_scl_add_key_field);
 
int32 ctc_scl_copy_entry(ctc_scl_copy_entry_t* copy_entry)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_copy_entry, lchip, copy_entry);
}
CTC_EXPORT_SYMBOL(ctc_scl_copy_entry);
 
int32 ctc_scl_create_group(uint32 group_id, ctc_scl_group_info_t* group_info)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_create_group, lchip, group_id, group_info);
}
CTC_EXPORT_SYMBOL(ctc_scl_create_group);
 
int32 ctc_scl_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_scl_deinit);
 
int32 ctc_scl_destroy_group(uint32 group_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_destroy_group, lchip, group_id);
}
CTC_EXPORT_SYMBOL(ctc_scl_destroy_group);
 
int32 ctc_scl_get_group_info(uint32 group_id, ctc_scl_group_info_t* group_info)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_get_group_info, lchip, group_id, group_info);
}
CTC_EXPORT_SYMBOL(ctc_scl_get_group_info);
 
int32 ctc_scl_get_multi_entry(ctc_scl_query_t* query)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_get_multi_entry, lchip, query);
}
CTC_EXPORT_SYMBOL(ctc_scl_get_multi_entry);
 
int32 ctc_scl_init(void* scl_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_scl_init, lchip, scl_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_scl_init);
 
int32 ctc_scl_install_entry(uint32 entry_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_install_entry, lchip, entry_id);
}
CTC_EXPORT_SYMBOL(ctc_scl_install_entry);
 
int32 ctc_scl_install_group(uint32 group_id, ctc_scl_group_info_t* group_info)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_install_group, lchip, group_id, group_info);
}
CTC_EXPORT_SYMBOL(ctc_scl_install_group);
 
int32 ctc_scl_remove_action_field(uint32 entry_id, ctc_scl_field_action_t* p_field_action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_remove_action_field, lchip, entry_id, p_field_action);
}
CTC_EXPORT_SYMBOL(ctc_scl_remove_action_field);
 
int32 ctc_scl_remove_all_entry(uint32 group_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_remove_all_entry, lchip, group_id);
}
CTC_EXPORT_SYMBOL(ctc_scl_remove_all_entry);
 
int32 ctc_scl_remove_entry(uint32 entry_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_remove_entry, lchip, entry_id);
}
CTC_EXPORT_SYMBOL(ctc_scl_remove_entry);
 
int32 ctc_scl_remove_key_field(uint32 entry_id, ctc_field_key_t* p_field_key)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_remove_key_field, lchip, entry_id, p_field_key);
}
CTC_EXPORT_SYMBOL(ctc_scl_remove_key_field);
 
int32 ctc_scl_set_default_action(ctc_scl_default_action_t* def_action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_set_default_action, lchip, def_action);
}
CTC_EXPORT_SYMBOL(ctc_scl_set_default_action);
 
int32 ctc_scl_set_entry_priority(uint32 entry_id, uint32 priority)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_set_entry_priority, lchip, entry_id, priority);
}
CTC_EXPORT_SYMBOL(ctc_scl_set_entry_priority);
 
int32 ctc_scl_set_hash_field_sel(ctc_scl_hash_field_sel_t* field_sel)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_set_hash_field_sel, lchip, field_sel);
}
CTC_EXPORT_SYMBOL(ctc_scl_set_hash_field_sel);
 
int32 ctc_scl_uninstall_entry(uint32 entry_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_uninstall_entry, lchip, entry_id);
}
CTC_EXPORT_SYMBOL(ctc_scl_uninstall_entry);
 
int32 ctc_scl_uninstall_group(uint32 group_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_uninstall_group, lchip, group_id);
}
CTC_EXPORT_SYMBOL(ctc_scl_uninstall_group);
 
int32 ctc_scl_update_action(uint32 entry_id, ctc_scl_action_t* action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_scl_update_action, lchip, entry_id, action);
}
CTC_EXPORT_SYMBOL(ctc_scl_update_action);
 
/*##security##*/
int32 ctc_ip_source_guard_add_entry(ctc_security_ip_source_guard_elem_t* p_elem)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ip_source_guard_add_entry, lchip, p_elem);
}
CTC_EXPORT_SYMBOL(ctc_ip_source_guard_add_entry);
 
int32 ctc_ip_source_guard_remove_entry(ctc_security_ip_source_guard_elem_t* p_elem)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_ip_source_guard_remove_entry, lchip, p_elem);
}
CTC_EXPORT_SYMBOL(ctc_ip_source_guard_remove_entry);
 
int32 ctc_mac_security_get_learn_limit(ctc_security_learn_limit_t* p_learn_limit)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mac_security_get_learn_limit, lchip, p_learn_limit);
}
CTC_EXPORT_SYMBOL(ctc_mac_security_get_learn_limit);
 
int32 ctc_mac_security_get_port_mac_limit(uint32 gport, ctc_maclimit_action_t* action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mac_security_get_port_mac_limit, lchip, gport, action);
}
CTC_EXPORT_SYMBOL(ctc_mac_security_get_port_mac_limit);
 
int32 ctc_mac_security_get_port_security(uint32 gport, bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mac_security_get_port_security, lchip, gport, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_mac_security_get_port_security);
 
int32 ctc_mac_security_get_system_mac_limit(ctc_maclimit_action_t* action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mac_security_get_system_mac_limit, lchip, action);
}
CTC_EXPORT_SYMBOL(ctc_mac_security_get_system_mac_limit);
 
int32 ctc_mac_security_get_vlan_mac_limit(uint16 vlan_id, ctc_maclimit_action_t* action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mac_security_get_vlan_mac_limit, lchip, vlan_id, action);
}
CTC_EXPORT_SYMBOL(ctc_mac_security_get_vlan_mac_limit);
 
int32 ctc_mac_security_set_learn_limit(ctc_security_learn_limit_t* p_learn_limit)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mac_security_set_learn_limit, lchip, p_learn_limit);
}
CTC_EXPORT_SYMBOL(ctc_mac_security_set_learn_limit);
 
int32 ctc_mac_security_set_port_mac_limit(uint32 gport, ctc_maclimit_action_t action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mac_security_set_port_mac_limit, lchip, gport, action);
}
CTC_EXPORT_SYMBOL(ctc_mac_security_set_port_mac_limit);
 
int32 ctc_mac_security_set_port_security(uint32 gport, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mac_security_set_port_security, lchip, gport, enable);
}
CTC_EXPORT_SYMBOL(ctc_mac_security_set_port_security);
 
int32 ctc_mac_security_set_system_mac_limit(ctc_maclimit_action_t action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mac_security_set_system_mac_limit, lchip, action);
}
CTC_EXPORT_SYMBOL(ctc_mac_security_set_system_mac_limit);
 
int32 ctc_mac_security_set_vlan_mac_limit(uint16 vlan_id, ctc_maclimit_action_t action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_mac_security_set_vlan_mac_limit, lchip, vlan_id, action);
}
CTC_EXPORT_SYMBOL(ctc_mac_security_set_vlan_mac_limit);
 
int32 ctc_port_isolation_get_route_obey_isolated_en(bool* p_enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_isolation_get_route_obey_isolated_en, lchip, p_enable);
}
CTC_EXPORT_SYMBOL(ctc_port_isolation_get_route_obey_isolated_en);
 
int32 ctc_port_isolation_set_route_obey_isolated_en(bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_port_isolation_set_route_obey_isolated_en, lchip, enable);
}
CTC_EXPORT_SYMBOL(ctc_port_isolation_set_route_obey_isolated_en);
 
int32 ctc_security_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_security_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_security_deinit);
 
int32 ctc_security_init(void* security_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_security_init, lchip, security_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_security_init);
 
int32 ctc_storm_ctl_get_cfg(ctc_security_stmctl_cfg_t* stmctl_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_storm_ctl_get_cfg, lchip, stmctl_cfg);
}
CTC_EXPORT_SYMBOL(ctc_storm_ctl_get_cfg);
 
int32 ctc_storm_ctl_get_global_cfg(ctc_security_stmctl_glb_cfg_t* p_glb_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_storm_ctl_get_global_cfg, lchip, p_glb_cfg);
}
CTC_EXPORT_SYMBOL(ctc_storm_ctl_get_global_cfg);
 
int32 ctc_storm_ctl_set_cfg(ctc_security_stmctl_cfg_t* stmctl_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_storm_ctl_set_cfg, lchip, stmctl_cfg);
}
CTC_EXPORT_SYMBOL(ctc_storm_ctl_set_cfg);
 
int32 ctc_storm_ctl_set_global_cfg(ctc_security_stmctl_glb_cfg_t* p_glb_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_storm_ctl_set_global_cfg, lchip, p_glb_cfg);
}
CTC_EXPORT_SYMBOL(ctc_storm_ctl_set_global_cfg);
 
/*##stacking##*/
int32 ctc_stacking_add_trunk_port(ctc_stacking_trunk_t* p_trunk)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_add_trunk_port, lchip, p_trunk);
}
CTC_EXPORT_SYMBOL(ctc_stacking_add_trunk_port);
 
int32 ctc_stacking_add_trunk_rchip(ctc_stacking_trunk_t* p_trunk)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_add_trunk_rchip, lchip, p_trunk);
}
CTC_EXPORT_SYMBOL(ctc_stacking_add_trunk_rchip);
 
int32 ctc_stacking_create_keeplive_group(uint16 group_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_create_keeplive_group, lchip, group_id);
}
CTC_EXPORT_SYMBOL(ctc_stacking_create_keeplive_group);
 
int32 ctc_stacking_create_trunk(ctc_stacking_trunk_t* p_trunk)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_create_trunk, lchip, p_trunk);
}
CTC_EXPORT_SYMBOL(ctc_stacking_create_trunk);
 
int32 ctc_stacking_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_stacking_deinit);
 
int32 ctc_stacking_destroy_keeplive_group(uint16 group_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_destroy_keeplive_group, lchip, group_id);
}
CTC_EXPORT_SYMBOL(ctc_stacking_destroy_keeplive_group);
 
int32 ctc_stacking_destroy_trunk(ctc_stacking_trunk_t* p_trunk)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_destroy_trunk, lchip, p_trunk);
}
CTC_EXPORT_SYMBOL(ctc_stacking_destroy_trunk);
 
int32 ctc_stacking_get_member_ports(uint8 lchip_id, uint8 trunk_id, uint32* p_gports, uint8* cnt)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_get_member_ports, lchip_id, trunk_id, p_gports, cnt);
}
CTC_EXPORT_SYMBOL(ctc_stacking_get_member_ports);
 
int32 ctc_stacking_get_property(ctc_stacking_property_t* p_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_get_property, lchip, p_prop);
}
CTC_EXPORT_SYMBOL(ctc_stacking_get_property);
 
int32 ctc_stacking_get_trunk_mcast_profile(ctc_stacking_trunk_mcast_profile_t* p_mcast_profile)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_get_trunk_mcast_profile, lchip, p_mcast_profile);
}
CTC_EXPORT_SYMBOL(ctc_stacking_get_trunk_mcast_profile);
 
int32 ctc_stacking_get_trunk_rchip(ctc_stacking_trunk_t* p_trunk)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_get_trunk_rchip, lchip, p_trunk);
}
CTC_EXPORT_SYMBOL(ctc_stacking_get_trunk_rchip);
 
int32 ctc_stacking_init(void* p_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_stacking_init, lchip, p_cfg);
}
CTC_EXPORT_SYMBOL(ctc_stacking_init);
 
int32 ctc_stacking_keeplive_add_member(ctc_stacking_keeplive_t* p_keeplive)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_keeplive_add_member, lchip, p_keeplive);
}
CTC_EXPORT_SYMBOL(ctc_stacking_keeplive_add_member);
 
int32 ctc_stacking_keeplive_get_members(ctc_stacking_keeplive_t* p_keeplive)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_keeplive_get_members, lchip, p_keeplive);
}
CTC_EXPORT_SYMBOL(ctc_stacking_keeplive_get_members);
 
int32 ctc_stacking_keeplive_remove_member(ctc_stacking_keeplive_t* p_keeplive)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_keeplive_remove_member, lchip, p_keeplive);
}
CTC_EXPORT_SYMBOL(ctc_stacking_keeplive_remove_member);
 
int32 ctc_stacking_remove_trunk_port(ctc_stacking_trunk_t* p_trunk)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_remove_trunk_port, lchip, p_trunk);
}
CTC_EXPORT_SYMBOL(ctc_stacking_remove_trunk_port);
 
int32 ctc_stacking_remove_trunk_rchip(ctc_stacking_trunk_t* p_trunk)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_remove_trunk_rchip, lchip, p_trunk);
}
CTC_EXPORT_SYMBOL(ctc_stacking_remove_trunk_rchip);
 
int32 ctc_stacking_replace_trunk_ports(ctc_stacking_trunk_t* p_trunk, uint32* gports, uint16 mem_ports)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_replace_trunk_ports, lchip, p_trunk, gports, mem_ports);
}
CTC_EXPORT_SYMBOL(ctc_stacking_replace_trunk_ports);
 
int32 ctc_stacking_set_property(ctc_stacking_property_t* p_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_set_property, lchip, p_prop);
}
CTC_EXPORT_SYMBOL(ctc_stacking_set_property);
 
int32 ctc_stacking_set_trunk_mcast_profile(ctc_stacking_trunk_mcast_profile_t* p_mcast_profile)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stacking_set_trunk_mcast_profile, lchip, p_mcast_profile);
}
CTC_EXPORT_SYMBOL(ctc_stacking_set_trunk_mcast_profile);
 
/*##stats##*/
int32 ctc_stats_clear_cpu_mac_stats(uint32 gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_clear_cpu_mac_stats, lchip, gport);
}
CTC_EXPORT_SYMBOL(ctc_stats_clear_cpu_mac_stats);
 
int32 ctc_stats_clear_mac_stats(uint32 gport, ctc_mac_stats_dir_t dir)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_clear_mac_stats, lchip, gport, dir);
}
CTC_EXPORT_SYMBOL(ctc_stats_clear_mac_stats);
 
int32 ctc_stats_clear_port_log_stats(uint32 gport, ctc_direction_t dir)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_clear_port_log_stats, lchip, gport, dir);
}
CTC_EXPORT_SYMBOL(ctc_stats_clear_port_log_stats);
 
int32 ctc_stats_clear_stats(uint32 stats_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_clear_stats, lchip, stats_id);
}
CTC_EXPORT_SYMBOL(ctc_stats_clear_stats);
 
int32 ctc_stats_create_statsid(ctc_stats_statsid_t* statsid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_create_statsid, lchip, statsid);
}
CTC_EXPORT_SYMBOL(ctc_stats_create_statsid);
 
int32 ctc_stats_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_stats_deinit);
 
int32 ctc_stats_destroy_statsid(uint32 stats_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_destroy_statsid, lchip, stats_id);
}
CTC_EXPORT_SYMBOL(ctc_stats_destroy_statsid);
 
int32 ctc_stats_get_cpu_mac_stats(uint32 gport, ctc_cpu_mac_stats_t* p_stats)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_get_cpu_mac_stats, lchip, gport, p_stats);
}
CTC_EXPORT_SYMBOL(ctc_stats_get_cpu_mac_stats);
 
int32 ctc_stats_get_drop_packet_stats_en(ctc_stats_discard_t bitmap, bool* enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_get_drop_packet_stats_en, lchip, bitmap, enable);
}
CTC_EXPORT_SYMBOL(ctc_stats_get_drop_packet_stats_en);
 
int32 ctc_stats_get_global_cfg(ctc_stats_property_param_t stats_param, ctc_stats_property_t* p_stats_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_get_global_cfg, lchip, stats_param, p_stats_prop);
}
CTC_EXPORT_SYMBOL(ctc_stats_get_global_cfg);
 
int32 ctc_stats_get_mac_stats_cfg(uint32 gport, ctc_mac_stats_prop_type_t mac_stats_prop_type, ctc_mac_stats_property_t* p_prop_data)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_get_mac_stats_cfg, lchip, gport, mac_stats_prop_type, p_prop_data);
}
CTC_EXPORT_SYMBOL(ctc_stats_get_mac_stats_cfg);
 
int32 ctc_stats_get_mac_stats(uint32 gport, ctc_mac_stats_dir_t dir, ctc_mac_stats_t* p_stats)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_get_mac_stats, lchip, gport, dir, p_stats);
}
CTC_EXPORT_SYMBOL(ctc_stats_get_mac_stats);
 
int32 ctc_stats_get_port_log_stats(uint32 gport, ctc_direction_t dir, ctc_stats_basic_t* p_stats)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_get_port_log_stats, lchip, gport, dir, p_stats);
}
CTC_EXPORT_SYMBOL(ctc_stats_get_port_log_stats);
 
int32 ctc_stats_get_stats(uint32 stats_id, ctc_stats_basic_t* p_stats)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_get_stats, lchip, stats_id, p_stats);
}
CTC_EXPORT_SYMBOL(ctc_stats_get_stats);
 
int32 ctc_stats_init(void* stats_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_stats_init, lchip, stats_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_stats_init);
 
int32 ctc_stats_intr_callback_func(uint8* gchip)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_intr_callback_func, lchip, gchip);
}
CTC_EXPORT_SYMBOL(ctc_stats_intr_callback_func);
 
int32 ctc_stats_register_cb(ctc_stats_sync_fn_t cb, void* userdata)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_register_cb, lchip, cb, userdata);
}
CTC_EXPORT_SYMBOL(ctc_stats_register_cb);
 
int32 ctc_stats_set_drop_packet_stats_en(ctc_stats_discard_t bitmap, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_set_drop_packet_stats_en, lchip, bitmap, enable);
}
CTC_EXPORT_SYMBOL(ctc_stats_set_drop_packet_stats_en);
 
int32 ctc_stats_set_global_cfg(ctc_stats_property_param_t stats_param, ctc_stats_property_t stats_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_set_global_cfg, lchip, stats_param, stats_prop);
}
CTC_EXPORT_SYMBOL(ctc_stats_set_global_cfg);
 
int32 ctc_stats_set_mac_stats_cfg(uint32 gport, ctc_mac_stats_prop_type_t mac_stats_prop_type, ctc_mac_stats_property_t prop_data)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_set_mac_stats_cfg, lchip, gport, mac_stats_prop_type, prop_data);
}
CTC_EXPORT_SYMBOL(ctc_stats_set_mac_stats_cfg);
 
int32 ctc_stats_set_syncup_cb_internal(uint32 stats_interval)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stats_set_syncup_cb_internal, lchip, stats_interval);
}
CTC_EXPORT_SYMBOL(ctc_stats_set_syncup_cb_internal);
 
/*##stp##*/
int32 ctc_stp_clear_all_inst_state(uint32 gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stp_clear_all_inst_state, lchip, gport);
}
CTC_EXPORT_SYMBOL(ctc_stp_clear_all_inst_state);
 
int32 ctc_stp_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stp_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_stp_deinit);
 
int32 ctc_stp_get_state(uint32 gport, uint8 stpid, uint8* state)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stp_get_state, lchip, gport, stpid, state);
}
CTC_EXPORT_SYMBOL(ctc_stp_get_state);
 
int32 ctc_stp_get_vlan_stpid(uint16 vlan_id, uint8* stpid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stp_get_vlan_stpid, lchip, vlan_id, stpid);
}
CTC_EXPORT_SYMBOL(ctc_stp_get_vlan_stpid);
 
int32 ctc_stp_init(void* stp_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_stp_init, lchip, stp_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_stp_init);
 
int32 ctc_stp_set_state(uint32 gport, uint8 stpid, uint8 state)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stp_set_state, lchip, gport, stpid, state);
}
CTC_EXPORT_SYMBOL(ctc_stp_set_state);
 
int32 ctc_stp_set_vlan_stpid(uint16 vlan_id, uint8 stpid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_stp_set_vlan_stpid, lchip, vlan_id, stpid);
}
CTC_EXPORT_SYMBOL(ctc_stp_set_vlan_stpid);
 
/*##sync_ether##*/
int32 ctc_sync_ether_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_sync_ether_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_sync_ether_deinit);
 
int32 ctc_sync_ether_get_cfg(uint8 lchip_id, uint8 sync_ether_clock_id, ctc_sync_ether_cfg_t* p_sync_ether_cfg)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_sync_ether_get_cfg, lchip_id, sync_ether_clock_id, p_sync_ether_cfg);
}
CTC_EXPORT_SYMBOL(ctc_sync_ether_get_cfg);
 
int32 ctc_sync_ether_init(void* sync_ether_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_sync_ether_init, lchip, sync_ether_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_sync_ether_init);
 
int32 ctc_sync_ether_set_cfg(uint8 lchip_id, uint8 sync_ether_clock_id, ctc_sync_ether_cfg_t* p_sync_ether_cfg)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_sync_ether_set_cfg, lchip_id, sync_ether_clock_id, p_sync_ether_cfg);
}
CTC_EXPORT_SYMBOL(ctc_sync_ether_set_cfg);
 
/*##trill##*/
int32 ctc_trill_add_route(ctc_trill_route_t* p_trill_route)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_trill_add_route, lchip, p_trill_route);
}
CTC_EXPORT_SYMBOL(ctc_trill_add_route);
 
int32 ctc_trill_add_tunnel(ctc_trill_tunnel_t* p_trill_tunnel)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_trill_add_tunnel, lchip, p_trill_tunnel);
}
CTC_EXPORT_SYMBOL(ctc_trill_add_tunnel);
 
int32 ctc_trill_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_trill_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_trill_deinit);
 
int32 ctc_trill_init(void* trill_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_trill_init, lchip, trill_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_trill_init);
 
int32 ctc_trill_remove_route(ctc_trill_route_t* p_trill_route)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_trill_remove_route, lchip, p_trill_route);
}
CTC_EXPORT_SYMBOL(ctc_trill_remove_route);
 
int32 ctc_trill_remove_tunnel(ctc_trill_tunnel_t* p_trill_tunnel)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_trill_remove_tunnel, lchip, p_trill_tunnel);
}
CTC_EXPORT_SYMBOL(ctc_trill_remove_tunnel);
 
int32 ctc_trill_update_tunnel(ctc_trill_tunnel_t* p_trill_tunnel)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_trill_update_tunnel, lchip, p_trill_tunnel);
}
CTC_EXPORT_SYMBOL(ctc_trill_update_tunnel);
 
/*##vlan##*/
int32 ctc_vlan_add_default_egress_vlan_mapping(uint32 gport, ctc_vlan_miss_t* p_action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_add_default_egress_vlan_mapping, lchip, gport, p_action);
}
CTC_EXPORT_SYMBOL(ctc_vlan_add_default_egress_vlan_mapping);
 
int32 ctc_vlan_add_default_vlan_class(ctc_vlan_class_type_t type, ctc_vlan_miss_t* p_action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_add_default_vlan_class, lchip, type, p_action);
}
CTC_EXPORT_SYMBOL(ctc_vlan_add_default_vlan_class);
 
int32 ctc_vlan_add_default_vlan_mapping(uint32 gport, ctc_vlan_miss_t* p_action)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_add_default_vlan_mapping, lchip, gport, p_action);
}
CTC_EXPORT_SYMBOL(ctc_vlan_add_default_vlan_mapping);
 
int32 ctc_vlan_add_egress_vlan_mapping(uint32 gport, ctc_egress_vlan_mapping_t* p_vlan_mapping)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_add_egress_vlan_mapping, lchip, gport, p_vlan_mapping);
}
CTC_EXPORT_SYMBOL(ctc_vlan_add_egress_vlan_mapping);
 
int32 ctc_vlan_add_ports(uint8 lchip_id, uint16 vlan_id, ctc_port_bitmap_t port_bitmap)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_add_ports, lchip_id, vlan_id, port_bitmap);
}
CTC_EXPORT_SYMBOL(ctc_vlan_add_ports);
 
int32 ctc_vlan_add_port(uint16 vlan_id, uint32 gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_add_port, lchip, vlan_id, gport);
}
CTC_EXPORT_SYMBOL(ctc_vlan_add_port);
 
int32 ctc_vlan_add_vlan_class(ctc_vlan_class_t* p_vlan_class)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_add_vlan_class, lchip, p_vlan_class);
}
CTC_EXPORT_SYMBOL(ctc_vlan_add_vlan_class);
 
int32 ctc_vlan_add_vlan_mapping(uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_add_vlan_mapping, lchip, gport, p_vlan_mapping);
}
CTC_EXPORT_SYMBOL(ctc_vlan_add_vlan_mapping);
 
int32 ctc_vlan_add_vlan_range(ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_t* vlan_range)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_add_vlan_range, lchip, vrange_info, vlan_range);
}
CTC_EXPORT_SYMBOL(ctc_vlan_add_vlan_range);
 
int32 ctc_vlan_create_uservlan(ctc_vlan_uservlan_t* user_vlan)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_create_uservlan, lchip, user_vlan);
}
CTC_EXPORT_SYMBOL(ctc_vlan_create_uservlan);
 
int32 ctc_vlan_create_vlan_range_group(ctc_vlan_range_info_t* vrange_info, bool is_svlan)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_create_vlan_range_group, lchip, vrange_info, is_svlan);
}
CTC_EXPORT_SYMBOL(ctc_vlan_create_vlan_range_group);
 
int32 ctc_vlan_create_vlan(uint16 vlan_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_create_vlan, lchip, vlan_id);
}
CTC_EXPORT_SYMBOL(ctc_vlan_create_vlan);
 
int32 ctc_vlan_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_vlan_deinit);
 
int32 ctc_vlan_destroy_vlan_range_group(ctc_vlan_range_info_t* vrange_info)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_destroy_vlan_range_group, lchip, vrange_info);
}
CTC_EXPORT_SYMBOL(ctc_vlan_destroy_vlan_range_group);
 
int32 ctc_vlan_destroy_vlan(uint16 vlan_id)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_destroy_vlan, lchip, vlan_id);
}
CTC_EXPORT_SYMBOL(ctc_vlan_destroy_vlan);
 
int32 ctc_vlan_get_acl_property(uint16 vlan_id, ctc_acl_property_t* p_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_acl_property, lchip, vlan_id, p_prop);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_acl_property);
 
int32 ctc_vlan_get_arp_excp_type(uint16 vlan_id, ctc_exception_type_t* type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_arp_excp_type, lchip, vlan_id, type);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_arp_excp_type);
 
int32 ctc_vlan_get_bridge_en(uint16 vlan_id, bool* enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_bridge_en, lchip, vlan_id, enable);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_bridge_en);
 
int32 ctc_vlan_get_dhcp_excp_type(uint16 vlan_id, ctc_exception_type_t* type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_dhcp_excp_type, lchip, vlan_id, type);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_dhcp_excp_type);
 
int32 ctc_vlan_get_direction_property(uint16 vlan_id, ctc_vlan_direction_property_t vlan_prop, ctc_direction_t dir, uint32* value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_direction_property, lchip, vlan_id, vlan_prop, dir, value);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_direction_property);
 
int32 ctc_vlan_get_egress_vlan_mapping(uint32 gport, ctc_egress_vlan_mapping_t* p_vlan_mapping)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_egress_vlan_mapping, lchip, gport, p_vlan_mapping);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_egress_vlan_mapping);
 
int32 ctc_vlan_get_fid(uint16 vlan_id, uint16* fid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_fid, lchip, vlan_id, fid);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_fid);
 
int32 ctc_vlan_get_igmp_snoop_en(uint16 vlan_id, bool* enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_igmp_snoop_en, lchip, vlan_id, enable);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_igmp_snoop_en);
 
int32 ctc_vlan_get_learning_en(uint16 vlan_id, bool* enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_learning_en, lchip, vlan_id, enable);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_learning_en);
 
int32 ctc_vlan_get_mac_auth(uint16 vlan_id, bool* enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_mac_auth, lchip, vlan_id, enable);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_mac_auth);
 
int32 ctc_vlan_get_ports(uint16 vlan_id, uint8 gchip, ctc_port_bitmap_t port_bitmap)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_ports, lchip, vlan_id, gchip, port_bitmap);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_ports);
 
int32 ctc_vlan_get_property(uint16 vlan_id, ctc_vlan_property_t vlan_prop, uint32* value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_property, lchip, vlan_id, vlan_prop, value);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_property);
 
int32 ctc_vlan_get_receive_en(uint16 vlan_id, bool* enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_receive_en, lchip, vlan_id, enable);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_receive_en);
 
int32 ctc_vlan_get_tagged_ports(uint16 vlan_id, uint8 gchip, ctc_port_bitmap_t port_bitmap)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_tagged_ports, lchip, vlan_id, gchip, port_bitmap);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_tagged_ports);
 
int32 ctc_vlan_get_transmit_en(uint16 vlan_id, bool* enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_transmit_en, lchip, vlan_id, enable);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_transmit_en);
 
int32 ctc_vlan_get_vlan_mapping(uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_vlan_mapping, lchip, gport, p_vlan_mapping);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_vlan_mapping);
 
int32 ctc_vlan_get_vlan_range_type(ctc_vlan_range_info_t* vrange_info, bool* is_svlan)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_vlan_range_type, lchip, vrange_info, is_svlan);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_vlan_range_type);
 
int32 ctc_vlan_get_vlan_range(ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_group_t* vrange_group, uint8* count)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_get_vlan_range, lchip, vrange_info, vrange_group, count);
}
CTC_EXPORT_SYMBOL(ctc_vlan_get_vlan_range);
 
int32 ctc_vlan_init(ctc_vlan_global_cfg_t* vlan_global_cfg)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_vlan_init, lchip, vlan_global_cfg);
}
CTC_EXPORT_SYMBOL(ctc_vlan_init);
 
int32 ctc_vlan_remove_all_egress_vlan_mapping_by_port(uint32 gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_remove_all_egress_vlan_mapping_by_port, lchip, gport);
}
CTC_EXPORT_SYMBOL(ctc_vlan_remove_all_egress_vlan_mapping_by_port);
 
int32 ctc_vlan_remove_all_vlan_class(ctc_vlan_class_type_t type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_remove_all_vlan_class, lchip, type);
}
CTC_EXPORT_SYMBOL(ctc_vlan_remove_all_vlan_class);
 
int32 ctc_vlan_remove_all_vlan_mapping_by_port(uint32 gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_remove_all_vlan_mapping_by_port, lchip, gport);
}
CTC_EXPORT_SYMBOL(ctc_vlan_remove_all_vlan_mapping_by_port);
 
int32 ctc_vlan_remove_default_egress_vlan_mapping(uint32 gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_remove_default_egress_vlan_mapping, lchip, gport);
}
CTC_EXPORT_SYMBOL(ctc_vlan_remove_default_egress_vlan_mapping);
 
int32 ctc_vlan_remove_default_vlan_class(ctc_vlan_class_type_t type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_remove_default_vlan_class, lchip, type);
}
CTC_EXPORT_SYMBOL(ctc_vlan_remove_default_vlan_class);
 
int32 ctc_vlan_remove_default_vlan_mapping(uint32 gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_remove_default_vlan_mapping, lchip, gport);
}
CTC_EXPORT_SYMBOL(ctc_vlan_remove_default_vlan_mapping);
 
int32 ctc_vlan_remove_egress_vlan_mapping(uint32 gport, ctc_egress_vlan_mapping_t* p_vlan_mapping)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_remove_egress_vlan_mapping, lchip, gport, p_vlan_mapping);
}
CTC_EXPORT_SYMBOL(ctc_vlan_remove_egress_vlan_mapping);
 
int32 ctc_vlan_remove_ports(uint8 lchip_id, uint16 vlan_id, ctc_port_bitmap_t port_bitmap)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_remove_ports, lchip_id, vlan_id, port_bitmap);
}
CTC_EXPORT_SYMBOL(ctc_vlan_remove_ports);
 
int32 ctc_vlan_remove_port(uint16 vlan_id, uint32 gport)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_remove_port, lchip, vlan_id, gport);
}
CTC_EXPORT_SYMBOL(ctc_vlan_remove_port);
 
int32 ctc_vlan_remove_vlan_class(ctc_vlan_class_t* p_vlan_class)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_remove_vlan_class, lchip, p_vlan_class);
}
CTC_EXPORT_SYMBOL(ctc_vlan_remove_vlan_class);
 
int32 ctc_vlan_remove_vlan_mapping(uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_remove_vlan_mapping, lchip, gport, p_vlan_mapping);
}
CTC_EXPORT_SYMBOL(ctc_vlan_remove_vlan_mapping);
 
int32 ctc_vlan_remove_vlan_range(ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_t* vlan_range)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_remove_vlan_range, lchip, vrange_info, vlan_range);
}
CTC_EXPORT_SYMBOL(ctc_vlan_remove_vlan_range);
 
int32 ctc_vlan_set_acl_property(uint16 vlan_id, ctc_acl_property_t* p_prop)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_set_acl_property, lchip, vlan_id, p_prop);
}
CTC_EXPORT_SYMBOL(ctc_vlan_set_acl_property);
 
int32 ctc_vlan_set_arp_excp_type(uint16 vlan_id, ctc_exception_type_t type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_set_arp_excp_type, lchip, vlan_id, type);
}
CTC_EXPORT_SYMBOL(ctc_vlan_set_arp_excp_type);
 
int32 ctc_vlan_set_bridge_en(uint16 vlan_id, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_set_bridge_en, lchip, vlan_id, enable);
}
CTC_EXPORT_SYMBOL(ctc_vlan_set_bridge_en);
 
int32 ctc_vlan_set_dhcp_excp_type(uint16 vlan_id, ctc_exception_type_t type)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_set_dhcp_excp_type, lchip, vlan_id, type);
}
CTC_EXPORT_SYMBOL(ctc_vlan_set_dhcp_excp_type);
 
int32 ctc_vlan_set_direction_property(uint16 vlan_id, ctc_vlan_direction_property_t vlan_prop, ctc_direction_t dir, uint32 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_set_direction_property, lchip, vlan_id, vlan_prop, dir, value);
}
CTC_EXPORT_SYMBOL(ctc_vlan_set_direction_property);
 
int32 ctc_vlan_set_fid(uint16 vlan_id, uint16 fid)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_set_fid, lchip, vlan_id, fid);
}
CTC_EXPORT_SYMBOL(ctc_vlan_set_fid);
 
int32 ctc_vlan_set_igmp_snoop_en(uint16 vlan_id, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_set_igmp_snoop_en, lchip, vlan_id, enable);
}
CTC_EXPORT_SYMBOL(ctc_vlan_set_igmp_snoop_en);
 
int32 ctc_vlan_set_learning_en(uint16 vlan_id, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_set_learning_en, lchip, vlan_id, enable);
}
CTC_EXPORT_SYMBOL(ctc_vlan_set_learning_en);
 
int32 ctc_vlan_set_mac_auth(uint16 vlan_id, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_set_mac_auth, lchip, vlan_id, enable);
}
CTC_EXPORT_SYMBOL(ctc_vlan_set_mac_auth);
 
int32 ctc_vlan_set_property_array(uint16 vlan_id, ctc_property_array_t* vlan_prop, uint16* array_num)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_set_property_array, lchip, vlan_id, vlan_prop, array_num);
}
CTC_EXPORT_SYMBOL(ctc_vlan_set_property_array);
 
int32 ctc_vlan_set_property(uint16 vlan_id, ctc_vlan_property_t vlan_prop, uint32 value)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_set_property, lchip, vlan_id, vlan_prop, value);
}
CTC_EXPORT_SYMBOL(ctc_vlan_set_property);
 
int32 ctc_vlan_set_receive_en(uint16 vlan_id, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_set_receive_en, lchip, vlan_id, enable);
}
CTC_EXPORT_SYMBOL(ctc_vlan_set_receive_en);
 
int32 ctc_vlan_set_tagged_ports(uint8 lchip_id, uint16 vlan_id, ctc_port_bitmap_t port_bitmap)
{
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_set_tagged_ports, lchip_id, vlan_id, port_bitmap);
}
CTC_EXPORT_SYMBOL(ctc_vlan_set_tagged_ports);
 
int32 ctc_vlan_set_tagged_port(uint16 vlan_id, uint32 gport, bool tagged)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_set_tagged_port, lchip, vlan_id, gport, tagged);
}
CTC_EXPORT_SYMBOL(ctc_vlan_set_tagged_port);
 
int32 ctc_vlan_set_transmit_en(uint16 vlan_id, bool enable)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_set_transmit_en, lchip, vlan_id, enable);
}
CTC_EXPORT_SYMBOL(ctc_vlan_set_transmit_en);
 
int32 ctc_vlan_update_vlan_mapping(uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_vlan_update_vlan_mapping, lchip, gport, p_vlan_mapping);
}
CTC_EXPORT_SYMBOL(ctc_vlan_update_vlan_mapping);
 
/*##wlan##*/
int32 ctc_wlan_add_client(ctc_wlan_client_t* p_client_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_wlan_add_client, lchip, p_client_param);
}
CTC_EXPORT_SYMBOL(ctc_wlan_add_client);
 
int32 ctc_wlan_add_tunnel(ctc_wlan_tunnel_t* p_tunnel_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_wlan_add_tunnel, lchip, p_tunnel_param);
}
CTC_EXPORT_SYMBOL(ctc_wlan_add_tunnel);
 
int32 ctc_wlan_deinit()
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_wlan_deinit, lchip);
}
CTC_EXPORT_SYMBOL(ctc_wlan_deinit);
 
int32 ctc_wlan_get_crypt(ctc_wlan_crypt_t* p_crypt_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_wlan_get_crypt, lchip, p_crypt_param);
}
CTC_EXPORT_SYMBOL(ctc_wlan_get_crypt);
 
int32 ctc_wlan_get_global_cfg(ctc_wlan_global_cfg_t* p_glb_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_wlan_get_global_cfg, lchip, p_glb_param);
}
CTC_EXPORT_SYMBOL(ctc_wlan_get_global_cfg);
 
int32 ctc_wlan_init(ctc_wlan_global_cfg_t* p_glb_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_INIT_ERROR_RETURN(ctc_api->ctcs_wlan_init, lchip, p_glb_param);
}
CTC_EXPORT_SYMBOL(ctc_wlan_init);
 
int32 ctc_wlan_remove_client(ctc_wlan_client_t* p_client_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_wlan_remove_client, lchip, p_client_param);
}
CTC_EXPORT_SYMBOL(ctc_wlan_remove_client);
 
int32 ctc_wlan_remove_tunnel(ctc_wlan_tunnel_t* p_tunnel_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_wlan_remove_tunnel, lchip, p_tunnel_param);
}
CTC_EXPORT_SYMBOL(ctc_wlan_remove_tunnel);
 
int32 ctc_wlan_set_crypt(ctc_wlan_crypt_t* p_crypt_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_wlan_set_crypt, lchip, p_crypt_param);
}
CTC_EXPORT_SYMBOL(ctc_wlan_set_crypt);
 
int32 ctc_wlan_set_global_cfg(ctc_wlan_global_cfg_t* p_glb_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_wlan_set_global_cfg, lchip, p_glb_param);
}
CTC_EXPORT_SYMBOL(ctc_wlan_set_global_cfg);
 
int32 ctc_wlan_update_client(ctc_wlan_client_t* p_client_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_wlan_update_client, lchip, p_client_param);
}
CTC_EXPORT_SYMBOL(ctc_wlan_update_client);
 
int32 ctc_wlan_update_tunnel(ctc_wlan_tunnel_t* p_tunnel_param)
{
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(ctc_api);
    CTC_API_ERROR_RETURN(ctc_api->ctcs_wlan_update_tunnel, lchip, p_tunnel_param);
}
CTC_EXPORT_SYMBOL(ctc_wlan_update_tunnel);
 
 
 
