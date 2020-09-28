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
#include "ctc_sai_vlanclass.h"
#include "ctc_sai_debug.h"

static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_VLANCLASS_GROUP_ATTR_RULE_LIST,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};


#define ________SAI_VLANCLASS_DEBUG_FUNC

sai_status_t
ctc_sai_create_vlanclass_group_member_debug_param(
    _Out_ sai_object_id_t* group_member_oid,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    const sai_attribute_t *attr = NULL;
    char              buf[CTC_IPV6_ADDR_STR_LEN] = "";
    uint32_t          attr_idx = 0; 
    uint32_t          tempip = 0;

    CTC_SAI_DEBUG("out: group_member_oid 0x%llx", (*group_member_oid));
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        switch (attr->id)
        {
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_GROUP_ID:
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_GROUP_ID 0x%llx", attr->value.oid);
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_RULE_ID:
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_RULE_ID u%", attr->value.s32);
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_ENTRY_TYPE:
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_ENTRY_TYPE u%", attr->value.s32);
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_VLAN_ID:
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_VLAN_ID u%", attr->value.s16);
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_MAC_ADDRESS:
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_MAC_ADDRESS %02X:%02X:%02X:%02X:%02X:%02X", 
                attr->value.mac[0], attr->value.mac[1], attr->value.mac[2],
                attr->value.mac[3], attr->value.mac[4], attr->value.mac[5]);
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_IPV4_ADDRESS:
            tempip = sal_ntohl(attr->value.ipaddr.addr.ip4);
            sal_inet_ntop(AF_INET, &tempip, buf, CTC_IPV6_ADDR_STR_LEN);
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_IPV4_ADDRESS %s", buf);
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_IPV6_ADDRESS:
            sal_inet_ntop(AF_INET6, attr->value.ipaddr.addr.ip6, buf, CTC_IPV6_ADDR_STR_LEN);
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_IPV6_ADDRESS %s", buf);
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_PROTOCOL_TYPE:
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_PROTOCOL_TYPE u%", attr->value.s32);
            break;
         default:
            break;
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_delete_vlanclass_group_debug_param(
    _In_ sai_object_id_t  group_oid)
{
    CTC_SAI_DEBUG("in: group_oid 0x%llx", group_oid);
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_get_vlanclass_group_member_attribute_debug_param(
    _In_ sai_object_id_t group_member_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{   
    const sai_attribute_t *attr = NULL;
    char              buf[CTC_IPV6_ADDR_STR_LEN] = "";
    uint32_t          attr_idx = 0; 
    uint32_t          tempip = 0;

    CTC_SAI_DEBUG("out: group_member_id 0x%llx", group_member_id);
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        switch (attr->id)
        {
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_GROUP_ID:
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_GROUP_ID 0x%llx", attr->value.oid);
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_RULE_ID:
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_RULE_ID u%", attr->value.s32);
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_ENTRY_TYPE:
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_ENTRY_TYPE u%", attr->value.s32);
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_VLAN_ID:
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_VLAN_ID u%", attr->value.s16);
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_MAC_ADDRESS:
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_MAC_ADDRESS %02X:%02X:%02X:%02X:%02X:%02X", 
                attr->value.mac[0], attr->value.mac[1], attr->value.mac[2],
                attr->value.mac[3], attr->value.mac[4], attr->value.mac[5]);
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_IPV4_ADDRESS:
            tempip = sal_ntohl(attr->value.ipaddr.addr.ip4);
            sal_inet_ntop(AF_INET, &tempip, buf, CTC_IPV6_ADDR_STR_LEN);
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_IPV4_ADDRESS %s", buf);
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_IPV6_ADDRESS:
            sal_inet_ntop(AF_INET6, attr->value.ipaddr.addr.ip6, buf, CTC_IPV6_ADDR_STR_LEN);
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_IPV6_ADDRESS %s", buf);
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_PROTOCOL_TYPE:
            CTC_SAI_DEBUG("out: SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_PROTOCOL_TYPE u%", attr->value.s32);
            break;
         default:
            break;
        }
    }
    return SAI_STATUS_SUCCESS;
}


sai_status_t
ctc_sai_get_vlanclass_group_attribute_debug_param(
    _In_ sai_object_id_t group_oid,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    sai_status_t            ret = SAI_STATUS_SUCCESS;
    sai_attribute_t *       attr = NULL;
    uint32_t                attr_idx = 0; 

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        ret = ctc_sai_attr_check_read_attr(g_sai_attr_entries,attr);

        if(ret != SAI_STATUS_SUCCESS)
        {
            return SAI_STATUS_SUCCESS;
        }

        switch(attr->id)
        {
        case SAI_VLANCLASS_GROUP_ATTR_RULE_LIST:
            CTC_SAI_DEBUG("in:SAI_VLANCLASS_GROUP_ATTR_RULE_LIST group_id 0x%llx", group_oid);
            break;
        default: 
            break;
        }

        if(ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlanclass_db_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    p_sai_glb->vlan_class_max_count = CTC_SAI_VALNCLASS_GROUP_MAX;

    if (0 != ctclib_opb_create(&p_sai_glb->vlan_class_opb, 0, p_sai_glb->vlan_class_max_count, "vlanclass"))
    {
        return SAI_STATUS_NO_MEMORY;
    }
    
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlanclass_get_protocol_type(uint32 protocol_type)
{
    if (SAI_VLAN_CLASS_PROTOCOL_TYPE_IPV4 == protocol_type)
    {
        return CTC_VLANCLASS_P_IPV4;
    }
    else if (SAI_VLAN_CLASS_PROTOCOL_TYPE_IPV6 == protocol_type)
    {
        return CTC_VLANCLASS_P_IPV6;
    }
    else if (SAI_VLAN_CLASS_PROTOCOL_TYPE_MPLS == protocol_type)
    {
        return CTC_VLANCLASS_P_MPLS;
    }
    else if (SAI_VLAN_CLASS_PROTOCOL_TYPE_MPLS_MCAST == protocol_type)
    {
        return CTC_VLANCLASS_P_MPLSMCAST;
    }
    else if (SAI_VLAN_CLASS_PROTOCOL_TYPE_ARP == protocol_type)
    {
        return CTC_VLANCLASS_P_IPV4ARP;
    }
    else if (SAI_VLAN_CLASS_PROTOCOL_TYPE_RARP == protocol_type)
    {
        return CTC_VLANCLASS_P_IPV4RARP;
    }
    else if (SAI_VLAN_CLASS_PROTOCOL_TYPE_PPPOE == protocol_type)
    {
        return CTC_VLANCLASS_P_PPPOE;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlanclass_group_create(
    _Out_ sai_object_id_t* group_oid)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    uint32_t  opf_index = 0;
    if (0 != ctclib_opb_alloc_offset(&p_sai_glb->vlan_class_opb, &opf_index))
    {
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    *group_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_VLAN_CLASS_GROUP, opf_index);
    
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_vlanclass_group_delete(
    _In_  sai_object_id_t group_oid)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    uint32_t opf_index = 0;
    if (SAI_OBJECT_TYPE_VLAN_CLASS_GROUP != CTC_SAI_OBJECT_TYPE_GET(group_oid))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    } 
    opf_index = CTC_SAI_OBJECT_INDEX_GET(group_oid);

    if (0 != ctclib_opb_free_offset(&p_sai_glb->vlan_class_opb, opf_index))
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    return SAI_STATUS_SUCCESS;   
}

sai_status_t
ctc_sai_create_vlanclass_group(
        _Out_ sai_object_id_t *group_oid,
        _In_  uint32_t attr_count,
        _In_  sai_attribute_t *attr_list)
{
    int32 ret = SAI_STATUS_SUCCESS;
    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(group_oid);
    ret = ctc_sai_vlanclass_group_create(group_oid);
    return ret;
}

sai_status_t
ctc_sai_delete_vlanclass_group(
    _In_ sai_object_id_t  group_oid)
{
    int32 ret = SAI_STATUS_SUCCESS;
    CTC_SAI_DEBUG_FUNC();
    ctc_sai_delete_vlanclass_group_debug_param(group_oid);

    ret = ctc_sai_vlanclass_group_delete(group_oid);
    return ret;
}

sai_status_t
ctc_sai_vlanclass_creat_vlanclass_group_member(
         _Out_ sai_object_id_t *group_member_oid,
        _In_  uint32_t attr_count,
        _In_  sai_attribute_t *attr_list)
{
    stbl_vlan_class_t vclass;
    stbl_vlan_class_t *p_sdb_vclass = NULL;
    sai_status_t      ret = SAI_STATUS_SUCCESS;
    uint32_t          attr_idx = 0; 
    sai_object_id_t   group_oid = 0;
    sai_attribute_t*  attr = NULL;
    ctc_vlan_class_t vlan_class;
    sai_mac_t        mac;
    sai_vlan_id_t    vlan_id = 0;
    ip_addr_t        ipv4_addr;
    ipv6_addr_t      ipv6_addr;
    uint32           mask_len =0;
    int32            rule_id = 0;
    uint32           entry_type = 0;
    uint32           proto_type = 0;
    uint32           ctc_proto_type = 0;

    sal_memset(&vclass, 0, sizeof(vclass));
    sal_memset(&vlan_class, 0, sizeof(vlan_class));
    sal_memset(&mac, 0, sizeof(mac)); 
    sal_memset(&ipv4_addr, 0, sizeof(ipv4_addr));
    sal_memset(&ipv6_addr, 0, sizeof(ipv6_addr)); 
    
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = (sai_attribute_t*)attr_list + attr_idx;
        switch(attr->id)
        {
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_GROUP_ID:
            group_oid = attr->value.oid;
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_RULE_ID:
            rule_id = attr->value.s32; 
            *group_member_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_VLAN_CLASS_GROUP_MEMBER, rule_id);     
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_ENTRY_TYPE:
            entry_type = attr->value.s32;
            if (SAI_VLAN_CLASS_ENTRY_TYPE_MAC == entry_type)
            {
                vlan_class.type = CTC_VLAN_CLASS_MAC;             
            }
            else if (SAI_VLAN_CLASS_ENTRY_TYPE_IPV4 == entry_type)
            {
                vlan_class.type = CTC_VLAN_CLASS_IPV4;
            }
            else if (SAI_VLAN_CLASS_ENTRY_TYPE_IPV6 == entry_type)
            {
                vlan_class.type = CTC_VLAN_CLASS_IPV6;
            }
            else if (SAI_VLAN_CLASS_ENTRY_TYPE_PROTOCOL == entry_type)
            {
                vlan_class.type = CTC_VLAN_CLASS_PROTOCOL;
            }  
            #ifndef USW
            //vlan_class.stag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;/*prepare for bug 47421*/
            #endif
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_VLAN_ID:
            vlan_id = attr->value.s16;
            vlan_class.vlan_id = vlan_id;
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_MAC_ADDRESS:
            sal_memcpy(mac, attr->value.mac, sizeof(mac_addr_t));
            sal_memcpy(vlan_class.vlan_class.mac, mac, sizeof(mac_addr_t));  
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_IPV4_ADDRESS:
            sal_memcpy(&ipv4_addr, &attr->value.ipaddr.addr.ip4, sizeof(sai_ip4_t));
            mask_len = CTC_SAI_MAX_IPV4_MASK_LEN;
            sal_memcpy(&(vlan_class.vlan_class.flex_ipv4.ipv4_sa), &ipv4_addr, sizeof(sai_ip4_t));
            IPV4_LEN_TO_MASK(vlan_class.vlan_class.flex_ipv4.ipv4_sa_mask, mask_len);
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_IPV6_ADDRESS:
            sal_memcpy(ipv6_addr, attr->value.ipaddr.addr.ip6, sizeof(sai_ip6_t));
            mask_len = CTC_SAI_MAX_IPV6_MASK_LEN;
            sal_memcpy(vlan_class.vlan_class.flex_ipv6.ipv6_sa, ipv6_addr, sizeof(sai_ip6_t));
            IPV6_LEN_TO_MASK(vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask, mask_len);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[0] = ipv6_addr[3];
            vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[1] = ipv6_addr[2];
            vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[2] = ipv6_addr[1];
            vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[3] = ipv6_addr[0];
            break;
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_PROTOCOL_TYPE:
            proto_type = attr->value.s32;
            ctc_proto_type = ctc_sai_vlanclass_get_protocol_type(proto_type);
            vlan_class.vlan_class.ether_type = ctc_proto_type;
            break; 
         default:
            break;
        }
        
    }

    vclass.key.rule_id = rule_id;
    p_sdb_vclass = stbl_vlan_class_get_vlan_class(&vclass.key);
    if (NULL == p_sdb_vclass)
    {
        stbl_vlan_class_add_vlan_class(&vclass);
        p_sdb_vclass = stbl_vlan_class_get_vlan_class(&vclass.key);
        if (NULL == p_sdb_vclass)
        {
            return SAI_STATUS_NO_MEMORY;
        }
    }
    p_sdb_vclass->group_oid = group_oid;
    p_sdb_vclass->member_oid = *group_member_oid;
    p_sdb_vclass->entry_type = entry_type;
    p_sdb_vclass->proto_type = proto_type;
    p_sdb_vclass->vlan_id = vlan_id;
    sal_memcpy(p_sdb_vclass->mac, mac, sizeof(mac_addr_t));
    if (SAI_VLAN_CLASS_ENTRY_TYPE_IPV4 == entry_type)
    {
        p_sdb_vclass->ipaddr.family = AF_INET;
        sal_memcpy(&p_sdb_vclass->ipaddr.u.prefix4, &ipv4_addr, sizeof(ipv4_addr));
    }
    else if (SAI_VLAN_CLASS_ENTRY_TYPE_IPV6 == entry_type)
    {
        p_sdb_vclass->ipaddr.family = AF_INET6;
        sal_memcpy(&p_sdb_vclass->ipaddr.u.prefix6, &ipv6_addr, sizeof(ipv6_addr));
    }

    /*if entry type is protocol and protocol type is pppoe, modify the ether_type*/
    if (SAI_VLAN_CLASS_ENTRY_TYPE_PROTOCOL == entry_type &&
        SAI_VLAN_CLASS_PROTOCOL_TYPE_PPPOE == proto_type)
    {
        vlan_class.vlan_class.ether_type = 0x8863;
        ret = ctc_vlan_add_vlan_class(&vlan_class);
        if (ret < 0)
        {
            return ret;
        }
        vlan_class.vlan_class.ether_type = 0x8864;
        ret = ctc_vlan_add_vlan_class(&vlan_class);
        if (ret < 0)
        {
            return ret;
        }
        return SAI_STATUS_SUCCESS;  
    }
    /*if entry type is ipv6 and ret is CTC_E_SCL_HASH_CONFLICT, set the vlan_class.flag*/
    if (SAI_VLAN_CLASS_ENTRY_TYPE_IPV6 == entry_type)
    {
         ret = ctc_vlan_add_vlan_class(&vlan_class);
         if (ret == CTC_E_SCL_HASH_CONFLICT)
         {
             vlan_class.flag = CTC_VLAN_CLASS_FLAG_USE_FLEX;
             ret = ctc_vlan_add_vlan_class(&vlan_class);
             if (ret < 0)
             {
                 return ret;
             }
             return SAI_STATUS_SUCCESS;
         }
         else
         {
            return ctc_sai_get_error_from_sdk_error(ret);
         }
    }
    
    ret = ctc_vlan_add_vlan_class(&vlan_class);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_vlanclass_delete_vlanclass_group_member(
    _In_ sai_object_id_t  group_member_oid)
{
    stbl_vlan_class_key_t vclass_key;
    stbl_vlan_class_t *p_sdb_vclass = NULL;
    sai_status_t      ret = SAI_STATUS_SUCCESS;
    ctc_vlan_class_t vlan_class;
    mac_addr_t       mac;
    ip_addr_t        ipv4_addr;
    ipv6_addr_t      ipv6_addr;
    sai_vlan_id_t    vlan_id;
    uint32           rule_id = 0;
    uint32           mask_len = 0;
    uint32           entry_type = 0;
    uint32           proto_type = 0;
    uint32           ctc_proto_type = 0;

    sal_memset(&vlan_class, 0, sizeof(ctc_vlan_class_t));
    sal_memset(&mac, 0, sizeof(mac_addr_t)); 
    sal_memset(&ipv4_addr, 0, sizeof(ip_addr_t));
    sal_memset(&ipv6_addr, 0, sizeof(ipv6_addr_t));
    
    if (SAI_OBJECT_TYPE_VLAN_CLASS_GROUP_MEMBER != CTC_SAI_OBJECT_TYPE_GET(group_member_oid))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    rule_id = CTC_SAI_OBJECT_INDEX_GET(group_member_oid);
    vclass_key.rule_id = rule_id;
    p_sdb_vclass = stbl_vlan_class_get_vlan_class(&vclass_key);
    if (NULL == p_sdb_vclass)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    vlan_id = p_sdb_vclass->vlan_id;
    sal_memcpy(mac, p_sdb_vclass->mac, sizeof(mac_addr_t));
    sal_memcpy(&ipv4_addr, &p_sdb_vclass->ipaddr.u.prefix4, sizeof(ip_addr_t));
    sal_memcpy(ipv6_addr, &p_sdb_vclass->ipaddr.u.prefix6, sizeof(ipv6_addr_t));
    entry_type = p_sdb_vclass->entry_type;
    proto_type = p_sdb_vclass->proto_type;

    stbl_vlan_class_del_vlan_class(&vclass_key);

    if (SAI_VLAN_CLASS_ENTRY_TYPE_MAC == entry_type)
    {
        vlan_class.type = CTC_VLAN_CLASS_MAC;
        vlan_class.vlan_id = vlan_id;
        sal_memcpy(vlan_class.vlan_class.mac, mac, sizeof(mac_addr_t));
        ret = ctc_vlan_remove_vlan_class(&vlan_class);
    }
    else if (SAI_VLAN_CLASS_ENTRY_TYPE_IPV4 == entry_type)
    {
        vlan_class.type = CTC_VLAN_CLASS_IPV4;
        vlan_class.vlan_id = vlan_id;
        sal_memcpy(&(vlan_class.vlan_class.flex_ipv4.ipv4_sa), &ipv4_addr, sizeof(ip_addr_t));
        mask_len = CTC_SAI_MAX_IPV4_MASK_LEN;
        IPV4_LEN_TO_MASK(vlan_class.vlan_class.flex_ipv4.ipv4_sa_mask, mask_len);
        ret = ctc_vlan_remove_vlan_class(&vlan_class);
    }
    else if (SAI_VLAN_CLASS_ENTRY_TYPE_IPV6 == entry_type)
    {
        vlan_class.type = CTC_VLAN_CLASS_IPV6;
        vlan_class.vlan_id = vlan_id;
        sal_memcpy((vlan_class.vlan_class.flex_ipv6.ipv6_sa), ipv6_addr, sizeof(ipv6_addr_t));
        mask_len = CTC_SAI_MAX_IPV6_MASK_LEN;
        IPV6_LEN_TO_MASK(vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask, mask_len);
        vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[0] = ipv6_addr[3];
        vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[1] = ipv6_addr[2];
        vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[2] = ipv6_addr[1];
        vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[3] = ipv6_addr[0];
        ret = ctc_vlan_remove_vlan_class(&vlan_class);
        if (ret == CTC_E_SCL_HASH_CONFLICT)
        {
            vlan_class.flag = CTC_VLAN_CLASS_FLAG_USE_FLEX;
            ret = ctc_vlan_remove_vlan_class(&vlan_class);
        }        
    }
    else if (SAI_VLAN_CLASS_ENTRY_TYPE_PROTOCOL == entry_type)
    {
        vlan_class.type = CTC_VLAN_CLASS_PROTOCOL;
        vlan_class.vlan_id = vlan_id;
        ctc_proto_type = ctc_sai_vlanclass_get_protocol_type(proto_type);
        vlan_class.vlan_class.ether_type = ctc_proto_type;
        if (SAI_VLAN_CLASS_PROTOCOL_TYPE_PPPOE == proto_type)
        {
            vlan_class.vlan_class.ether_type = 0x8863;
            ret = ctc_vlan_remove_vlan_class(&vlan_class);
            if (ret < 0)
            {
                return ctc_sai_get_error_from_sdk_error(ret);
            }
            vlan_class.vlan_class.ether_type = 0x8864;
            ret = ctc_vlan_remove_vlan_class(&vlan_class);
        }
        else
        {
            ret = ctc_vlan_remove_vlan_class(&vlan_class);
        }
    }
    else
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_create_vlanclass_group_member(
        _Out_ sai_object_id_t *group_member_oid,
        _In_  uint32_t attr_count,
        _In_  sai_attribute_t *attr_list)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    
    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr_list);
    CTC_SAI_PTR_VALID_CHECK(group_member_oid);

    ctc_sai_create_vlanclass_group_member_debug_param(group_member_oid, attr_count, attr_list);
    
    ret = ctc_sai_vlanclass_creat_vlanclass_group_member(group_member_oid, attr_count, attr_list);
    if (SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }
    
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_delete_vlanclass_group_member(
    _In_ sai_object_id_t  group_member_oid)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    CTC_SAI_DEBUG_FUNC();

    ret = ctc_sai_vlanclass_delete_vlanclass_group_member(group_member_oid);
    if (SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_set_vlanclass_group_attribute(
        _In_ sai_object_id_t group_oid,
        _In_ const  sai_attribute_t *attr)
{
    CTC_SAI_DEBUG_FUNC();
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_get_vlanclass_group_attribute(
        _In_ sai_object_id_t group_oid,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    CTC_SAI_DEBUG_FUNC();
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_set_vlanclass_group_member_attribute(
    _In_ sai_object_id_t  group_member_oid,
    _In_ const sai_attribute_t *attr)
{
    CTC_SAI_DEBUG_FUNC();
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_get_vlanclass_group_member_attribute(
    _In_ sai_object_id_t group_member_oid,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    stbl_vlan_class_key_t vclass_key;
    stbl_vlan_class_t *p_sdb_vclass = NULL;
    uint32_t          attr_idx = 0;
    uint32            rule_id = 0;
    sai_attribute_t*  attr = NULL;

    CTC_SAI_DEBUG_FUNC();
    ctc_sai_get_vlanclass_group_member_attribute_debug_param(group_member_oid, attr_count, attr_list);

    if (SAI_OBJECT_TYPE_VLAN_CLASS_GROUP_MEMBER != CTC_SAI_OBJECT_TYPE_GET(group_member_oid))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    rule_id = CTC_SAI_OBJECT_INDEX_GET(group_member_oid);
    vclass_key.rule_id = rule_id;
    CTC_SAI_PTR_VALID_CHECK(attr_list); 

    p_sdb_vclass = stbl_vlan_class_get_vlan_class(&vclass_key);
    if (NULL == p_sdb_vclass)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if ((0 == p_sdb_vclass->member_oid) || (0 == p_sdb_vclass->group_oid))
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        switch(attr->id)
        {
        case SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_GROUP_ID:
            attr->value.oid = p_sdb_vclass->group_oid;
            break; 
        case  SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_RULE_ID:
            attr->value.s32 = p_sdb_vclass->key.rule_id;
            break;
        case  SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_ENTRY_TYPE:
            attr->value.s32 = p_sdb_vclass->entry_type;
            break;
        case  SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_VLAN_ID:
            attr->value.s16 = p_sdb_vclass->vlan_id;
            break;
        case  SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_MAC_ADDRESS:
            sal_memcpy(attr->value.mac, p_sdb_vclass->mac, sizeof(mac_addr_t)); 
            break;
        case  SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_IPV4_ADDRESS: 
            sal_memcpy(&attr->value.ipaddr.addr.ip4, &p_sdb_vclass->ipaddr.u.prefix4, sizeof(ip_addr_t));
            break;
        case  SAI_VLAN_CLASS_GROUP_MEMBER_ATTR_PROTOCOL_TYPE:
            attr->value.s32 = p_sdb_vclass->proto_type;
            break;    
        default: /*not support other field */
            break;
        }
    }
    
    return SAI_STATUS_SUCCESS;
}

static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    ctc_sai_vlanclass_db_init();
    if(SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }
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
static sai_vlanclass_api_t g_sai_api_func = {
    .create_vlanclass_group              = ctc_sai_create_vlanclass_group,
    .delete_vlanclass_group             = ctc_sai_delete_vlanclass_group,
    .set_vlanclass_group_attribute      = ctc_sai_set_vlanclass_group_attribute,
    .get_vlanclass_group_attribute      = ctc_sai_get_vlanclass_group_attribute,
    .create_vlanclass_group_member     = ctc_sai_create_vlanclass_group_member,
    .delete_vlanclass_group_member     = ctc_sai_delete_vlanclass_group_member,
    .set_vlanclass_group_member_attribute = ctc_sai_set_vlanclass_group_member_attribute,
    .get_vlanclass_group_member_attribute = ctc_sai_get_vlanclass_group_member_attribute,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id         = SAI_API_VLAN_CLASS,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_VLANCLASS_OUTER_FUNC

sai_status_t
ctc_sai_vlanclass_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}

