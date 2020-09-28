#include "sal.h"
#include "ctc_api.h"
#include "ctc_port.h"
#include "sal_error.h"
#include "sai.h"
#include "saitypes.h"
#include "saistatus.h"
#include "ctc_sai_sai.h"
#include "ctc_sai_common.h"
#include "ctc_sai_common_attr.h"
#include "ctc_sai_port.h"
#include "ctc_sai_vlanmap.h"
#include <ctc_sai_nexthop.h>
#include "ctc_sai_debug.h"


sai_status_t 
ctc_sai_vlanmap_add_nexthop(uint16 gportid, uint16 raw_vid)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    ctc_vlan_edit_nh_param_t vlan_nh = {0};

    /* need to store nexthopid connected with raw vid */
    sai_uint32_t nexthopid;

    CTC_ERROR_RETURN(ctc_sai_nexthop_alloc_offset(&nexthopid));

    sal_memset(&vlan_nh, 0, sizeof(vlan_nh));

    vlan_nh.vlan_edit_info.output_svid = raw_vid;
    GLB_SET_FLAG(vlan_nh.vlan_edit_info.edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
    vlan_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
    vlan_nh.gport_or_aps_bridge_id = gportid;
    
    ret = ctc_nh_add_xlate(nexthopid, &vlan_nh);

    return ret;
}

sai_status_t 
ctc_sai_vlanmap_del_nexthop(uint16 gportid, uint16 raw_vid)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    ctc_vlan_edit_nh_param_t vlan_nh = {0};
    sai_uint32_t nexthopid;

    CTC_ERROR_RETURN(ctc_sai_nexthop_alloc_offset(&nexthopid));

    sal_memset(&vlan_nh, 0, sizeof(vlan_nh));

    vlan_nh.vlan_edit_info.output_svid = raw_vid;
    GLB_SET_FLAG(vlan_nh.vlan_edit_info.edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
    vlan_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
    vlan_nh.gport_or_aps_bridge_id = gportid;
    
    ret = ctc_nh_add_xlate(nexthopid, &vlan_nh);

    return ret;
}

sai_status_t 
ctc_sai_vlanmap_set_basic_qinq_default_entry(sai_vlanmap_entry_t* vm_entry, uint32 is_add)
{
    ctc_vlan_mapping_t vlan_mapping;
    stbl_port_t *p_sdb_port = NULL;
    uint32       gport = 0;
    ctc_sai_port_objectid_to_gport(vm_entry->port_id, &gport);

    p_sdb_port = ctc_sai_port_entry_get_by_portid(vm_entry->port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_SUCCESS;
    }

    sal_memset(&vlan_mapping, 0, sizeof(vlan_mapping));

    if (is_add)
    {
        if (vm_entry->replace)
        {
            vlan_mapping.key = CTC_VLAN_MAPPING_KEY_NONE;
            /* remove vlan mapping default entry */
            ctc_vlan_remove_vlan_mapping(gport, &vlan_mapping);
        }

        sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));
        vlan_mapping.key= CTC_VLAN_MAPPING_KEY_NONE;
        vlan_mapping.scos_sl= CTC_VLAN_TAG_SL_AS_PARSE;
        vlan_mapping.svid_sl = CTC_VLAN_TAG_SL_NEW;
        vlan_mapping.stag_op = CTC_VLAN_TAG_OP_ADD;
        vlan_mapping.action = CTC_VLAN_MAPPING_OUTPUT_SVID;
        vlan_mapping.new_svid =  vm_entry->def_vid;
        CTC_ERROR_RETURN(ctc_vlan_add_vlan_mapping(gport, &vlan_mapping));
    }
    else
    {
        vlan_mapping.key= CTC_VLAN_MAPPING_KEY_NONE;
        ctc_vlan_remove_vlan_mapping(gport, &vlan_mapping);
    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t 
ctc_sai_vlanmap_set_default_entry(uint32 gportid, uint16 vid, uint8 flag, uint8 ingress)
{
    ctc_vlan_miss_t ctc_vlan_miss;
    
    sal_memset(&ctc_vlan_miss, 0, sizeof(ctc_vlan_miss));
    ctc_vlan_miss.flag = flag;
    ctc_vlan_miss.new_svid = vid;
    if (ingress)
    {
        CTC_ERROR_RETURN(ctc_vlan_add_default_vlan_mapping(gportid, &ctc_vlan_miss));
    }
    else
    {
        CTC_ERROR_RETURN(ctc_vlan_add_default_egress_vlan_mapping(gportid, &ctc_vlan_miss));
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlanmap_set_type_none(sai_object_id_t port_id)
{
    ctc_egress_vlan_mapping_t egress_vlan_mapping;
    ctc_port_scl_property_t   prop;
    ctc_vlan_mapping_t vlan_mapping;
    stbl_port_t *p_sdb_port = NULL;
    uint16       vlan_id = 0;
    uint32       gport = 0;
    
    ctc_sai_port_objectid_to_gport(port_id, &gport);
    if (CTC_IS_LINKAGG_PORT(gport))
    {
        goto DEFAULT_ENTRY;
    }

    CTC_ERROR_RETURN(ctc_sai_vlanmap_set_default_entry(gport, 0, CTC_VLAN_MISS_ACTION_DO_NOTHING, TRUE));
    CTC_ERROR_RETURN(ctc_sai_vlanmap_set_default_entry(gport, 0, CTC_VLAN_MISS_ACTION_DO_NOTHING, FALSE));

    sal_memset(&egress_vlan_mapping, 0, sizeof(ctc_egress_vlan_mapping_t));
    sal_memset(&prop, 0, sizeof(ctc_port_scl_property_t));

    egress_vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
    egress_vlan_mapping.stag_op = CTC_VLAN_TAG_OP_NONE;
    egress_vlan_mapping.old_svid = 0;
    ctc_vlan_remove_egress_vlan_mapping(gport, &egress_vlan_mapping); 

    /* unset vlan mapping enable */
#if defined(GOLDENGATE) || defined(USW)
    prop.scl_id = 0;
    prop.direction = CTC_INGRESS;
    prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
    prop.class_id_en = 0;
    CTC_ERROR_RETURN(ctc_port_set_scl_property(gport, &prop));
    prop.direction = CTC_EGRESS;
    CTC_ERROR_RETURN(ctc_port_set_scl_property(gport, &prop));
    prop.scl_id = 1;
    CTC_ERROR_RETURN(ctc_port_set_scl_property(gport, &prop));
#else
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_INGRESS, 0, CTC_SCL_KEY_TYPE_DISABLE));
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_EGRESS, 0, CTC_SCL_KEY_TYPE_DISABLE));
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_EGRESS, 1, CTC_SCL_KEY_TYPE_DISABLE));
#endif

    ctc_port_set_vlan_domain(gport, CTC_PORT_VLAN_DOMAIN_SVLAN);

    ctc_port_get_default_vlan(gport, &vlan_id);
    CTC_ERROR_RETURN(ctc_port_set_default_vlan(gport, vlan_id));

DEFAULT_ENTRY:
    /* get port flags */ 
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_SUCCESS;
    }

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));
    vlan_mapping.key= CTC_VLAN_MAPPING_KEY_NONE;
    ctc_vlan_remove_vlan_mapping(gport, &vlan_mapping);

    //ctc_sai_port_set_port_flag(gport, SAI_PORT_FLAG_VLAN_TRANSLATION_ENABLE, FALSE);
    //ctc_sai_port_set_port_flag(gport, SAI_PORT_FLAG_VLAN_MAPPING_QINQ_ENABLE, FALSE);
    CTC_ERROR_RETURN(ctc_port_set_property(gport, CTC_PORT_PROP_DOT1Q_TYPE, CTC_DOT1Q_TYPE_BOTH));

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlanmap_set_type_replace(sai_object_id_t port_id)
{
    ctc_egress_vlan_mapping_t egress_vlan_mapping;
    ctc_vlan_mapping_t vlan_mapping;
    stbl_port_t *p_sdb_port = NULL;
    ctc_port_scl_property_t   prop;
    uint32       gport = 0;
    
    ctc_sai_port_objectid_to_gport(port_id, &gport);
    if (CTC_IS_LINKAGG_PORT(gport))
    {
        goto DEFAULT_ENTRY;
    }

    /* control forward or drop of the unmaped-vlan packets */
    CTC_ERROR_RETURN(ctc_sai_vlanmap_set_default_entry(gport, 0, CTC_VLAN_MISS_ACTION_DISCARD, TRUE));
    CTC_ERROR_RETURN(ctc_sai_vlanmap_set_default_entry(gport, 0, CTC_VLAN_MISS_ACTION_DISCARD, FALSE));

    sal_memset(&egress_vlan_mapping, 0, sizeof(ctc_egress_vlan_mapping_t));
    sal_memset(&prop, 0, sizeof(ctc_port_scl_property_t));

    egress_vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
    egress_vlan_mapping.stag_op = CTC_VLAN_TAG_OP_NONE;
    egress_vlan_mapping.old_svid = 0;
    CTC_ERROR_RETURN(ctc_vlan_add_egress_vlan_mapping(gport, &egress_vlan_mapping));

    /* set vlan mapping enable */
#if defined(GOLDENGATE) || defined(USW)
    prop.scl_id = 0;
    prop.direction = CTC_INGRESS;
    prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN;
    prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
    prop.class_id_en = 0;
    CTC_ERROR_RETURN(ctc_port_set_scl_property(gport, &prop));

    prop.direction = CTC_EGRESS;
    prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT_SVLAN;
    CTC_ERROR_RETURN(ctc_port_set_scl_property(gport, &prop));
#else
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_INGRESS, 0, CTC_SCL_KEY_TYPE_DISABLE));
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_INGRESS, 0, CTC_SCL_KEY_TYPE_VLAN_MAPPING_SVID));
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_EGRESS, 0, CTC_SCL_KEY_TYPE_DISABLE));
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_EGRESS, 0, CTC_SCL_KEY_TYPE_VLAN_MAPPING_SVID));
#endif

DEFAULT_ENTRY:
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_SUCCESS;
    }

    /* remove vlan mapping default entry */
    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));
    vlan_mapping.key= CTC_VLAN_MAPPING_KEY_NONE;
    CTC_ERROR_RETURN(ctc_vlan_remove_vlan_mapping(gport, &vlan_mapping));

    CTC_ERROR_RETURN(ctc_port_set_property(gport, CTC_PORT_PROP_DOT1Q_TYPE, CTC_DOT1Q_TYPE_BOTH));
    //ctc_sai_port_set_port_flag(gport, SAI_PORT_FLAG_VLAN_TRANSLATION_ENABLE, TRUE);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlanmap_set_type_basic_qinq(sai_object_id_t port_id)
{
    ctc_port_scl_property_t   prop;
    ctc_vlan_mapping_t vlan_mapping;
    stbl_port_t *p_sdb_port = NULL;
    uint16_t     vlan_id = 0;
    uint32       gport = 0;

    ctc_sai_port_objectid_to_gport(port_id, &gport);
    if (CTC_IS_LINKAGG_PORT(gport))
    {
        goto DEFAULT_ENTRY;
    }
    
    /* for untag packet */
    CTC_ERROR_RETURN(ctc_port_set_property(gport, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, TRUE));
    CTC_ERROR_RETURN(ctc_port_set_vlan_domain(gport, CTC_PORT_VLAN_DOMAIN_SVLAN));

    /* set vlan mapping enable */
    sal_memset(&prop, 0, sizeof(ctc_port_scl_property_t));
#if defined(GOLDENGATE) || defined(USW)
    prop.scl_id = 0;
    prop.direction = CTC_INGRESS;
    prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT;
    prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
    prop.class_id_en = 0;
    CTC_ERROR_RETURN(ctc_port_set_scl_property(gport, &prop));
    prop.direction = CTC_EGRESS;
    prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT;
    CTC_ERROR_RETURN(ctc_port_set_scl_property(gport, &prop));
    prop.scl_id = 1;
    prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_DISABLE;
    CTC_ERROR_RETURN(ctc_port_set_scl_property(gport, &prop));
#else
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_INGRESS, 0, CTC_SCL_KEY_TYPE_DISABLE));
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_INGRESS, 0, CTC_SCL_KEY_TYPE_VLAN_MAPPING_PORT));
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_EGRESS, 0, CTC_SCL_KEY_TYPE_DISABLE));
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_EGRESS, 0, CTC_SCL_KEY_TYPE_VLAN_MAPPING_PORT));
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_EGRESS, 1, CTC_SCL_KEY_TYPE_DISABLE));
#endif

    /* set default entry: don't drop unmatch entry */
    CTC_ERROR_RETURN(ctc_sai_vlanmap_set_default_entry(gport, 0, CTC_VLAN_MISS_ACTION_DO_NOTHING, TRUE));

DEFAULT_ENTRY:
    /* get port flags */ 
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_SUCCESS;
    }
    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));
    /* get default vlan */
    ctc_port_get_default_vlan(gport, &vlan_id);
    vlan_mapping.key= CTC_VLAN_MAPPING_KEY_NONE;
    vlan_mapping.scos_sl= CTC_VLAN_TAG_SL_AS_PARSE;
    vlan_mapping.svid_sl = CTC_VLAN_TAG_SL_NEW;
    vlan_mapping.stag_op = CTC_VLAN_TAG_OP_ADD;
    vlan_mapping.action = CTC_VLAN_MAPPING_OUTPUT_SVID;
    vlan_mapping.new_svid = vlan_id;
    CTC_ERROR_RETURN(ctc_vlan_add_vlan_mapping(gport, &vlan_mapping));

    //ctc_sai_port_set_port_flag(port_id, SAI_PORT_FLAG_VLAN_MAPPING_QINQ_SELECTIVE, FALSE);
    /* set dot1q type cvlan */
    //ctc_sai_port_set_port_flag(port_id, SAI_PORT_FLAG_VLAN_MAPPING_QINQ_ENABLE, TRUE);
    CTC_ERROR_RETURN(ctc_port_set_property(gport, CTC_PORT_PROP_DOT1Q_TYPE, CTC_DOT1Q_TYPE_CVLAN));

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlanmap_set_type_selective_qinq(sai_object_id_t port_id)
{
    ctc_port_scl_property_t   prop;
    ctc_vlan_mapping_t vlan_mapping;
    stbl_port_t *p_sdb_port = NULL;
    uint32       gport = 0;

    ctc_sai_port_objectid_to_gport(port_id, &gport);
    if (CTC_IS_LINKAGG_PORT(gport))
    {
        goto DEFAULT_ENTRY;
    }

    /* for untag packet */
    CTC_ERROR_RETURN(ctc_port_set_property(gport, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, FALSE));
    CTC_ERROR_RETURN(ctc_port_set_vlan_domain(gport, CTC_PORT_VLAN_DOMAIN_SVLAN));

    /* set port lookup flag */
    sal_memset(&prop, 0, sizeof(ctc_port_scl_property_t));
#if defined(GOLDENGATE) || defined(USW)
    prop.scl_id = 0;
    prop.direction = CTC_INGRESS;
    prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN;
    prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
    prop.class_id_en = 0;
    CTC_ERROR_RETURN(ctc_port_set_scl_property(gport, &prop));
    /* first scl lookup used svlan+cvlan as scl key */
    prop.direction = CTC_EGRESS;
    prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT_2VLAN;
    CTC_ERROR_RETURN(ctc_port_set_scl_property(gport, &prop));
    /* second scl lookup used svlan as scl key */
    prop.scl_id = 1;
    prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT_SVLAN;
    CTC_ERROR_RETURN(ctc_port_set_scl_property(gport, &prop));
#else
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_INGRESS, 0, CTC_SCL_KEY_TYPE_DISABLE));
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_INGRESS, 0, CTC_SCL_KEY_TYPE_VLAN_MAPPING_SVID));
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_EGRESS, 0, CTC_SCL_KEY_TYPE_DISABLE));
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_EGRESS, 0, CTC_SCL_KEY_TYPE_VLAN_MAPPING_DVID));
    CTC_ERROR_RETURN(ctc_port_set_scl_key_type(gport, CTC_EGRESS, 1, CTC_SCL_KEY_TYPE_VLAN_MAPPING_SVID));
#endif
    /* set default forward entry: drop unmatch entry */
    CTC_ERROR_RETURN(ctc_sai_vlanmap_set_default_entry(gport, 0, CTC_VLAN_MISS_ACTION_DISCARD, TRUE));

DEFAULT_ENTRY:
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_SUCCESS;
    }
    /* remove vlan mapping default entry */
    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));
    vlan_mapping.key= CTC_VLAN_MAPPING_KEY_NONE;
    ctc_vlan_remove_vlan_mapping(gport, &vlan_mapping);

    /* set selective qinq type flag */
    //ctc_sai_port_set_port_flag(port_id, SAI_PORT_FLAG_VLAN_MAPPING_QINQ_SELECTIVE, TRUE);
    /* output packet can carry both vlan when mismatch for egress qinq */
    CTC_ERROR_RETURN(ctc_port_set_property(gport, CTC_PORT_PROP_DOT1Q_TYPE, CTC_DOT1Q_TYPE_BOTH));

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_sdk_replace_entry_add(const sai_vlanmap_entry_t* vm_entry)
{
    int32 ret = SAI_STATUS_SUCCESS;
    ctc_vlan_mapping_t vlan_mapping;
    ctc_egress_vlan_mapping_t egress_vlan_mapping;
    uint32       gport = 0;
    ctc_sai_port_objectid_to_gport(vm_entry->port_id, &gport);

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));
    sal_memset(&egress_vlan_mapping, 0, sizeof(ctc_egress_vlan_mapping_t));

    if ((vm_entry->mapped_svlan == vm_entry->def_vid) && (vm_entry->change_defvid))
    {
        /* update egress entry */
        egress_vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
        egress_vlan_mapping.old_svid= vm_entry->mapped_svlan;
        ret = ctc_vlan_remove_egress_vlan_mapping(gport, &egress_vlan_mapping);
        if (SAI_STATUS_SUCCESS != ret)
        { 
            CTC_SAI_DEBUG("ctc_vlan_remove_egress_vlan_mapping gPort %ld, rawVid %ld, mapSvid %ld, ret %d, line = %d", 
                                            gport, vm_entry->raw_vid,  vm_entry->mapped_svlan, ret, __LINE__);
        }

        if (vm_entry->untag_pvid)
        {
            egress_vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
            egress_vlan_mapping.stag_op = CTC_VLAN_TAG_OP_DEL;
            egress_vlan_mapping.old_svid = vm_entry->mapped_svlan;
        }
        else
        {
            egress_vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
            egress_vlan_mapping.svid_sl = CTC_VLAN_TAG_SL_NEW;
            egress_vlan_mapping.stag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;
            egress_vlan_mapping.action = CTC_VLAN_MAPPING_OUTPUT_SVID;
            egress_vlan_mapping.old_svid = vm_entry->mapped_svlan;
            egress_vlan_mapping.new_svid = vm_entry->raw_vid;
        }
        ctc_vlan_add_egress_vlan_mapping(gport, &egress_vlan_mapping);
        return SAI_STATUS_SUCCESS;
    }

    /* ingress */
    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
    vlan_mapping.svid_sl = CTC_VLAN_TAG_SL_NEW;
    vlan_mapping.stag_op =CTC_VLAN_TAG_OP_REP_OR_ADD;
    vlan_mapping.action = CTC_VLAN_MAPPING_OUTPUT_SVID;
    vlan_mapping.old_svid = vm_entry->raw_vid;
    vlan_mapping.new_svid = vm_entry->mapped_svlan;
    ret = ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
    if (CTC_E_NONE != ret)
    {
        if (CTC_E_SCL_HASH_CONFLICT == ret)
        {
            vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_FLEX;
            ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
        }
        else
        {        
            CTC_SAI_DEBUG("_hagt_vlan_mapping_replace_entry_add glPort %ld, rawVid %ld, mapSvid %ld, ret %d, line = %d", 
                                        gport, vm_entry->raw_vid,  vm_entry->mapped_svlan, ret, __LINE__);
        }    
    }

    /* egress */
    egress_vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
    egress_vlan_mapping.old_svid = vm_entry->mapped_svlan;
    if (vm_entry->mapped_svlan == vm_entry->def_vid)
    {
        egress_vlan_mapping.stag_op = CTC_VLAN_TAG_OP_DEL;
    }
    else
    {
        egress_vlan_mapping.svid_sl = CTC_VLAN_TAG_SL_NEW;
        egress_vlan_mapping.stag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;
        egress_vlan_mapping.action = CTC_VLAN_MAPPING_OUTPUT_SVID;
        egress_vlan_mapping.new_svid = vm_entry->raw_vid;
    }
    ctc_vlan_add_egress_vlan_mapping(gport, &egress_vlan_mapping);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_sdk_replace_entry_del(const sai_vlanmap_entry_t* vm_entry)
{
    int32 ret = SAI_STATUS_SUCCESS;
    ctc_vlan_mapping_t vlan_mapping;
    ctc_egress_vlan_mapping_t egress_vlan_mapping;
    uint32       gport = 0;
    ctc_sai_port_objectid_to_gport(vm_entry->port_id, &gport);

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));
    sal_memset(&egress_vlan_mapping, 0, sizeof(ctc_egress_vlan_mapping_t));
    if ((vm_entry->mapped_svlan == vm_entry->def_vid) && (vm_entry->change_defvid))
    {
        /* update egress entry */
        egress_vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
        egress_vlan_mapping.old_svid= vm_entry->mapped_svlan;
        ret = ctc_vlan_remove_egress_vlan_mapping(gport, &egress_vlan_mapping);
        if (SAI_STATUS_SUCCESS != ret)
        { 
            CTC_SAI_DEBUG("ctc_vlan_remove_egress_vlan_mapping port_id %ld, raw_vid %ld, mapped_svlan %ld, ret %d, line = %d", 
                                            vm_entry->port_id, vm_entry->raw_vid,  vm_entry->mapped_svlan, ret, __LINE__);
        }

        egress_vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
        egress_vlan_mapping.svid_sl = CTC_VLAN_TAG_SL_NEW;
        egress_vlan_mapping.stag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;
        egress_vlan_mapping.action = CTC_VLAN_MAPPING_OUTPUT_SVID;
        egress_vlan_mapping.old_svid = vm_entry->mapped_svlan;
        egress_vlan_mapping.new_svid = vm_entry->raw_vid;
        ctc_vlan_add_egress_vlan_mapping(gport, &egress_vlan_mapping);
        return SAI_STATUS_SUCCESS;
    }

    /* remove vlan mapping entry */
    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
    vlan_mapping.old_svid= vm_entry->raw_vid;
    ret = ctc_vlan_remove_vlan_mapping(gport, &vlan_mapping);
    if (SAI_STATUS_SUCCESS != ret)
    { 
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_FLEX;
        ret = ctc_vlan_remove_vlan_mapping(gport, &vlan_mapping);
        if (SAI_STATUS_SUCCESS != ret)
        {        
            CTC_SAI_DEBUG("ctc_vlan_remove_vlan_mapping port_id %ld, raw_vid %ld, mapped_svlan %ld, ret %d, line = %d", 
                                        vm_entry->port_id, vm_entry->raw_vid,  vm_entry->mapped_svlan, ret, __LINE__);
        }
    }

    egress_vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
    egress_vlan_mapping.old_svid= vm_entry->mapped_svlan;
    ret = ctc_vlan_remove_egress_vlan_mapping(gport, &egress_vlan_mapping);
    if (SAI_STATUS_SUCCESS != ret)
    { 
        CTC_SAI_DEBUG("ctc_vlan_remove_egress_vlan_mapping port_id %ld, raw_vid %ld, mapped_svlan %ld, ret %d, line = %d", 
                                        vm_entry->port_id, vm_entry->raw_vid,  vm_entry->mapped_svlan, ret, __LINE__);
    }
    return SAI_STATUS_SUCCESS;
}


sai_status_t
ctc_sai_sdk_append_entry_add(const sai_vlanmap_entry_t* vm_entry)
{
    int32 ret = SAI_STATUS_SUCCESS;
    ctc_vlan_mapping_t vlan_mapping;

    uint32       gport = 0;
    ctc_sai_port_objectid_to_gport(vm_entry->port_id, &gport);

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));

    /* out-of-range <--> stag */
    if (GLB_MAX_VLAN_ID == vm_entry->raw_vid)
    {
        /*use default vlan to add svlan*/
        if (!CTC_IS_LINKAGG_PORT(gport))
        {
            CTC_ERROR_RETURN(ctc_sai_vlanmap_set_default_entry(gport, vm_entry->mapped_svlan, CTC_VLAN_MISS_ACTION_APPEND_STAG, TRUE));
        }
        return SAI_STATUS_SUCCESS;
    }

    /* untagged <--> ctag + stag */
    if (vm_entry->mapped_cvlan != GLB_VLAN_NONE)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_CVID;
        vlan_mapping.new_cvid = vm_entry->mapped_cvlan;
        vlan_mapping.cvid_sl = CTC_VLAN_TAG_SL_NEW;
        vlan_mapping.ctag_op = CTC_VLAN_TAG_OP_ADD;
        vlan_mapping.ccos_sl = CTC_VLAN_TAG_SL_ALTERNATIVE;
    }

    /* untagged/tagged <--> stag */
    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
    vlan_mapping.svid_sl = CTC_VLAN_TAG_SL_NEW;
    vlan_mapping.old_svid = vm_entry->raw_vid;
    vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_SVID;
    vlan_mapping.new_svid = vm_entry->mapped_svlan;
    if(vm_entry->raw_vid)
    {
        vlan_mapping.stag_op = CTC_VLAN_TAG_OP_ADD;
    }
    else
    {
        vlan_mapping.stag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;
    }

    ret = ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
    if (SAI_STATUS_SUCCESS != ret)
    {
        if (CTC_E_SCL_HASH_CONFLICT == ret)
        {
            vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_FLEX;
            ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
        }
        else
        {        
            CTC_SAI_DEBUG("_hagt_vlan_mapping_append_entry_add port_id %ld, raw_vid %ld, mapped_svlan %ld, ret %d, line = %d", 
                                        vm_entry->port_id, vm_entry->raw_vid,  vm_entry->mapped_svlan, ret, __LINE__);
        }  

        return ret;
    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t
ctc_sai_sdk_append_entry_del(const sai_vlanmap_entry_t* vm_entry)
{
    ctc_vlan_mapping_t vlan_mapping;
    int32 ret = SAI_STATUS_SUCCESS;
    uint32       gport = 0;
    ctc_sai_port_objectid_to_gport(vm_entry->port_id, &gport);
  
    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));
  
    /* out-of-range <--> stag */
    if (GLB_MAX_VLAN_ID == vm_entry->raw_vid)
    {      
        if (!CTC_IS_LINKAGG_PORT(gport))
        {
            if (vm_entry->default_fwd)
            {
                CTC_ERROR_RETURN(ctc_sai_vlanmap_set_default_entry(gport, 0, CTC_VLAN_MISS_ACTION_DO_NOTHING, TRUE));
            }
            else
            {
                CTC_ERROR_RETURN(ctc_sai_vlanmap_set_default_entry(gport, 0, CTC_VLAN_MISS_ACTION_DISCARD, TRUE));
            }
        }
    }
    else
    {
        vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
        vlan_mapping.old_svid = vm_entry->raw_vid;
        ret = ctc_vlan_remove_vlan_mapping(gport, &vlan_mapping);
        if (SAI_STATUS_SUCCESS != ret)
        {
            vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_FLEX;
            ret = ctc_vlan_remove_vlan_mapping(gport, &vlan_mapping);
            if (SAI_STATUS_SUCCESS != ret)
            { 
                CTC_SAI_DEBUG("ctc_vlan_remove_vlan_mapping_entry port_id %ld, raw_vid %ld, mapped_svlan %ld, ret %d, line = %d", 
                                vm_entry->port_id, vm_entry->raw_vid, vm_entry->mapped_svlan, ret, __LINE__);
            }
        }
    }
    return SAI_STATUS_SUCCESS;
}


sai_status_t
ctc_sai_sdk_append_group_add(const sai_vlanmap_entry_t* vm_entry)
{
    ctc_vlan_mapping_t vlan_mapping;
    uint32       gport = 0;
    ctc_sai_port_objectid_to_gport(vm_entry->port_id, &gport);

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));
    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
    vlan_mapping.svid_sl = CTC_VLAN_TAG_SL_NEW;
    vlan_mapping.stag_op = CTC_VLAN_TAG_OP_ADD;
    vlan_mapping.old_svid = vm_entry->raw_vid;
    vlan_mapping.svlan_end = vm_entry->raw_vid_max;
    vlan_mapping.action = CTC_VLAN_MAPPING_OUTPUT_SVID;
    vlan_mapping.new_svid = vm_entry->mapped_svlan;
    vlan_mapping.vrange_grpid = vm_entry->group_id - 1;
    ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_sdk_append_group_del(const sai_vlanmap_entry_t* vm_entry)
{
    int32 ret = SAI_STATUS_SUCCESS;
    ctc_vlan_mapping_t vlan_mapping;
    uint32       gport = 0;
    ctc_sai_port_objectid_to_gport(vm_entry->port_id, &gport);

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));
    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
    vlan_mapping.old_svid = vm_entry->raw_vid;
    vlan_mapping.svlan_end = vm_entry->raw_vid_max;
    vlan_mapping.vrange_grpid = vm_entry->group_id - 1;
    ret = ctc_vlan_remove_vlan_mapping(gport, &vlan_mapping);
    if (SAI_STATUS_SUCCESS != ret)
    { 
        CTC_SAI_DEBUG("ctc_vlan_remove_vlan_mapping_entry glPort %ld, rawVid %ld, mapSvid %ld, ret %d, line = %d",
                      vm_entry->port_id, vm_entry->raw_vid, vm_entry->mapped_svlan, ret, __LINE__);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_sdk_egress_entry_add(const sai_vlanmap_entry_t* vm_entry)
{
    int32 ret = SAI_STATUS_SUCCESS;
    uint32       gport = 0;
    ctc_sai_port_objectid_to_gport(vm_entry->port_id, &gport);
    ctc_egress_vlan_mapping_t egress_vlan_mapping;
    sal_memset(&egress_vlan_mapping, 0, sizeof(ctc_egress_vlan_mapping_t));

    /* cvlan not change */
    if (GLB_INVALID_VLANID == vm_entry->raw_vid)
    {
        egress_vlan_mapping.ctag_op = CTC_VLAN_TAG_OP_NONE;
    }
    /* del cvlan */
    else if (GLB_VLAN_NONE == vm_entry->raw_vid)
    {
        egress_vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CVID;
        egress_vlan_mapping.action |= CTC_EGRESS_VLAN_MAPPING_OUTPUT_CVID;
        egress_vlan_mapping.ctag_op = CTC_VLAN_TAG_OP_DEL;
        egress_vlan_mapping.old_cvid = vm_entry->mapped_cvlan;
    }
    /* modify cvlan */
    else
    {
        egress_vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CVID;
        egress_vlan_mapping.action |= CTC_EGRESS_VLAN_MAPPING_OUTPUT_CVID;
        egress_vlan_mapping.ctag_op = CTC_VLAN_TAG_OP_REP;
        egress_vlan_mapping.cvid_sl = CTC_VLAN_TAG_SL_NEW;
        egress_vlan_mapping.old_cvid = vm_entry->mapped_cvlan;
        egress_vlan_mapping.new_cvid = vm_entry->raw_vid;
    }

    egress_vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_SVID;
    egress_vlan_mapping.action |= CTC_EGRESS_VLAN_MAPPING_OUTPUT_SVID;
    egress_vlan_mapping.stag_op = CTC_VLAN_TAG_OP_DEL;
    egress_vlan_mapping.old_svid = vm_entry->mapped_svlan;

    CTC_ERROR_RETURN(ctc_vlan_add_egress_vlan_mapping(gport, &egress_vlan_mapping));

    return ret;
}

sai_status_t
ctc_sai_sdk_egress_entry_del(const sai_vlanmap_entry_t* vm_entry)
{
    int32 ret = SAI_STATUS_SUCCESS;
    uint32       gport = 0;
    ctc_sai_port_objectid_to_gport(vm_entry->port_id, &gport);
    ctc_egress_vlan_mapping_t egress_vlan_mapping;
    sal_memset(&egress_vlan_mapping, 0, sizeof(ctc_egress_vlan_mapping_t));

    if (GLB_INVALID_VLANID != vm_entry->mapped_svlan)
    {
        egress_vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_SVID;
        egress_vlan_mapping.old_svid= vm_entry->mapped_svlan;
    }

    if (GLB_INVALID_VLANID != vm_entry->mapped_cvlan)
    {
        egress_vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CVID;
        egress_vlan_mapping.old_cvid= vm_entry->mapped_cvlan;
    }

    ret = ctc_vlan_remove_egress_vlan_mapping(gport, &egress_vlan_mapping);

    return ret;
}

sai_status_t
ctc_sai_create_vlanmap_entry(
    _In_ sai_vlanmap_entry_t* vm_entry,
    _In_ sai_attribute_t *attr_list)
{
    CTC_SAI_DEBUG("ctc_sai_create_vlanmap_entry gl_port: 0x%lx, raw_vid: %ld, mapped_svid: %ld, mapped_cvid: %ld, add: %ld, replace: %ld", 
                   vm_entry->port_id, vm_entry->raw_vid, vm_entry->mapped_svlan, vm_entry->mapped_cvlan, 1, vm_entry->replace);

    if (vm_entry->default_entry)
    {
        ctc_sai_vlanmap_set_basic_qinq_default_entry(vm_entry, TRUE);
        return SAI_STATUS_SUCCESS;
    }
    
    if (vm_entry->replace)
    {
        ctc_sai_sdk_replace_entry_add(vm_entry);
    }
    else
    {
        /* add for selctive qinq egress scl */
        if (vm_entry->egress)
        {
            ctc_sai_sdk_egress_entry_add(vm_entry);
            return SAI_STATUS_SUCCESS;
        }
    
        if (!vm_entry->raw_vid_max)
        {
            ctc_sai_sdk_append_entry_add(vm_entry);
        }
        else
        {
            ctc_sai_sdk_append_group_add(vm_entry);
        }
    }
    
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_delete_vlanmap_entry(
    _In_ sai_vlanmap_entry_t* vm_entry,
    _In_ sai_attribute_t *attr_list)
{
    CTC_SAI_DEBUG("ctc_sai_create_vlanmap_entry gl_port: 0x%lx, raw_vid: %ld, mapped_svid: %ld, mapped_cvid: %ld, add: %ld, replace: %ld", 
                   vm_entry->port_id, vm_entry->raw_vid, vm_entry->mapped_svlan, vm_entry->mapped_cvlan, 0, vm_entry->replace);

    if (vm_entry->default_entry)
    {
        /* delete default entry for basic qinq join agg */
        ctc_sai_vlanmap_set_basic_qinq_default_entry(vm_entry, FALSE);
        return SAI_STATUS_SUCCESS;
    }

    if (vm_entry->replace)
    {
        ctc_sai_sdk_replace_entry_del(vm_entry);
    }
    else
    {
        /* add for selctive qinq egress scl */
        if (vm_entry->egress)
        {
            ctc_sai_sdk_egress_entry_del(vm_entry);
            return SAI_STATUS_SUCCESS;
        }

        if (!vm_entry->raw_vid_max)
        {
            ctc_sai_sdk_append_entry_del(vm_entry);
        }
        else
        {
            ctc_sai_sdk_append_group_del(vm_entry);
        }
    }
    
    return SAI_STATUS_SUCCESS;   
}

sai_status_t
ctc_sai_create_vlan_group(
        _In_ sai_vlan_group_t *vlan_group)
{
    int32 ret = SAI_STATUS_SUCCESS;
    int16 group_id = 0;
    ctc_vlan_range_info_t vrange_info;
    ctc_vlan_range_t vlan_range;
    uint32 i = 0;


    group_id = vlan_group->group_id;
    if(group_id < 1)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    vrange_info.vrange_grpid = group_id - 1;
    vrange_info.direction = CTC_INGRESS;

    ctc_vlan_create_vlan_range_group(&vrange_info, TRUE);

    for (i = 0; i < 8; ++i)
    {
        if (vlan_group->vlanmin[i])
        {
            vlan_range.vlan_start = vlan_group->vlanmin[i];
            vlan_range.vlan_end = vlan_group->vlanmax[i];
            ctc_vlan_add_vlan_range(&vrange_info, &vlan_range);
        }
    }

    return ret;
}

sai_status_t
ctc_sai_delete_vlan_group(
    _In_ sai_object_id_t group_id)
{
    int32 ret = SAI_STATUS_SUCCESS;
    ctc_vlan_range_info_t vrange_info;

    if(group_id < 1)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    vrange_info.vrange_grpid = group_id - 1;
    vrange_info.direction = CTC_INGRESS;
    
    ctc_vlan_destroy_vlan_range_group(&vrange_info);
    return ret;
}

static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    preg->init_status = INITIALIZED;

    return ret;
}

static sai_status_t
__exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    
    preg->init_status = UNINITIALIZED;

    return ret;
}

/* define sai  */
static sai_vlanmap_api_t g_sai_api_func = {
    .create_vlanmap_entry     = ctc_sai_create_vlanmap_entry,
    .delete_vlanmap_entry     = ctc_sai_delete_vlanmap_entry,
    .create_vlan_group        = ctc_sai_create_vlan_group,
    .delete_vlan_group        = ctc_sai_delete_vlan_group,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id         = SAI_API_VLANMAP,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_VLANMAP_OUTER_FUNC

sai_status_t
ctc_sai_vlanmap_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}

