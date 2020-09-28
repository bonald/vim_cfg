
#include <sal.h>
#include "ctc_api.h"
#include <sai.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_nexthop_group.h>
#include <ctc_sai_port.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_mirror.h>
#include <ctc_sai_lag.h>
#include <ctc_sai_tap_group.h>
#include <ctc_sai_acl.h>
#include <ctc_sai_copp.h>


typedef struct fields_mapping_res_s
{
#define acl u.ctc_acl_entry
#define scl u.ctc_scl_entry

#define acl_key u.ctc_acl_entry.key.u.ipv4_key
#define scl_key u.ctc_scl_entry.key.u.tcam_ipv4_key
#define aclv6_key u.ctc_acl_entry.key.u.ipv6_key
#define sclv6_key u.ctc_scl_entry.key.u.tcam_ipv6_key

    enum {
        FIELD_ACL_ENTRY_TYPE,
        FIELD_SCL_ENTRY_TYPE,
    } mapping_type;
    union {
        ctc_acl_entry_t     ctc_acl_entry;
        ctc_scl_entry_t     ctc_scl_entry;
    } u;
    sai_int32_t             is_ipv6;
} fields_mapping_res_t;


sai_int32_t
__tunnel_decap_alloc_erspan_nexthop (stbl_acl_tunnel_t *tun_info);

sai_int32_t
__tunnel_decap_alloc_erspan_service_id (stbl_acl_tunnel_t *tun_info);

static void 
__mapping_sai_acl_entry_to_ctc_acl_entry(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res);


#define ________SAI_ACL_TUNNEL_MAPPING_FUN
static int32_t
__config_strip_header(
            ctc_sai_acl_field_list_t        *fields,
            strip_header_parameter_t        *strip_parameter,
            fields_mapping_res_t            *res)
{

    int32_t packet_type = PACKET_UNKONW;
#ifdef GOLDENGATE	
    uint8_t l4_protocol = 0;
    //uint16_t l4_dst_port = 0;
    uint8_t is_l4_vxlan_dst_port = 0;

    l4_protocol = 
        fields->ipv4_ip_protocol.data.u8 & 
        ~(fields->ipv4_ip_protocol.mask.u8);

    /* dst_port eq mode, list dst-port eq 4789,so data == mask == 4789 */
    //l4_dst_port = (fields->ipv4_dst_port.data.u16) & (fields->ipv4_dst_port.mask.u16);
    is_l4_vxlan_dst_port = fields->ipv4_vxlan_vni_dst_port.enable;

    /* check parameters */
    if(L4_PROTOCOL_UDP == l4_protocol && (is_l4_vxlan_dst_port == TRUE))
    {
        packet_type = PACKET_VXLAN;
    }
    else if(L4_PROTOCOL_GRE == l4_protocol)
    {    
        packet_type = PACKET_IPV4_GRE;
    }
    else if(L4_PROTOCOL_IPIP == l4_protocol)
    {
        packet_type = PACKET_IPV4_IPIP;
    }
#endif

    /* set strip-header */
    switch(packet_type){
    case PACKET_VXLAN:
        if(res->mapping_type == FIELD_ACL_ENTRY_TYPE){
            if(strip_parameter->strip_header.enable){
                res->u.ctc_acl_entry.action.packet_strip.start_packet_strip  = CTC_ACL_STRIP_START_TO_L4;
                res->u.ctc_acl_entry.action.packet_strip.packet_type         = PKT_TYPE_ETH;
                res->u.ctc_acl_entry.action.packet_strip.strip_extra_len     = 16;

                /* TODO DIY strip offset */
                if(strip_parameter->strip_header_pos.enable){
                    switch(strip_parameter->strip_header_pos.parameter.u32){
                    case SAI_ACL_STRIP_HEADER_POS_L2:
                        res->u.ctc_acl_entry.action.packet_strip.start_packet_strip  = CTC_ACL_STRIP_START_TO_L2;
                        break;
                    case SAI_ACL_STRIP_HEADER_POS_L3:
                        res->u.ctc_acl_entry.action.packet_strip.start_packet_strip  = CTC_ACL_STRIP_START_TO_L3;
                        break;
                    default:
                        break;
                    }
                }

                if(strip_parameter->strip_header_offset.enable){
                    res->u.ctc_acl_entry.action.packet_strip.strip_extra_len  = 
                        strip_parameter->strip_header_offset.parameter.u32;
                }
            }
        }
        break;

    case PACKET_IPV4_GRE:
        if(res->mapping_type == FIELD_ACL_ENTRY_TYPE){
            if(strip_parameter->strip_header.enable){
                res->u.ctc_acl_entry.action.packet_strip.start_packet_strip  = CTC_ACL_STRIP_START_TO_L4;
                res->u.ctc_acl_entry.action.packet_strip.packet_type         = PKT_TYPE_ETH;

                res->u.ctc_acl_entry.action.packet_strip.strip_extra_len     = 4;

                if(SAI_ACL_GRE_NVGRE == 
                   fields->gre_type.data.u32){
                    /* nvgre allows has gre-key */
                    res->u.ctc_acl_entry.action.packet_strip.strip_extra_len += 4;      
                }else{
                    /* if gre has gre-key */
                    if(fields->gre_key.enable){
                        res->u.ctc_acl_entry.action.packet_strip.strip_extra_len  += 4;
                    }
                }

                if(fields->erspan_id.enable)
                {
                    res->u.ctc_acl_entry.action.packet_strip.strip_extra_len     += 8;
                }

                /* TODO DIY strip offset */
                if(strip_parameter->strip_header_pos.enable){
                    switch(strip_parameter->strip_header_pos.parameter.u32){
                    case SAI_ACL_STRIP_HEADER_POS_L2:
                        res->u.ctc_acl_entry.action.packet_strip.start_packet_strip  = CTC_ACL_STRIP_START_TO_L2;
                        break;
                    case SAI_ACL_STRIP_HEADER_POS_L3:
                        res->u.ctc_acl_entry.action.packet_strip.start_packet_strip  = CTC_ACL_STRIP_START_TO_L3;
                        break;
                    default:
                        break;
                    }
                }

                if(strip_parameter->strip_header_offset.enable){
                    res->u.ctc_acl_entry.action.packet_strip.strip_extra_len  = 
                        strip_parameter->strip_header_offset.parameter.u32;
                }
            } 
        }else{
            if(strip_parameter->strip_header.enable){
//#ifndef DUET2
				CTC_SET_FLAG(res->scl.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_STRIP_PACKET);
                res->scl.action.u.flow_action.strip_packet.start_packet_strip  = CTC_SCL_STRIP_START_TO_L4;
                res->scl.action.u.flow_action.strip_packet.packet_type         = PKT_TYPE_ETH;

                res->scl.action.u.flow_action.strip_packet.strip_extra_len     = 4;

                if(SAI_ACL_GRE_NVGRE == 
                   fields->gre_type.data.u32){
                    /* nvgre allows has gre-key */
                    res->scl.action.u.flow_action.strip_packet.strip_extra_len += 4;      
                }else{
                    /* if gre has gre-key */
                    if(fields->gre_key.enable){
                        res->scl.action.u.flow_action.strip_packet.strip_extra_len  += 4;
                    }
                }

                if(fields->erspan_id.enable)
                {
                    res->scl.action.u.flow_action.strip_packet.strip_extra_len     += 8;
                }

                /* TODO DIY strip offset */
                if(strip_parameter->strip_header_pos.enable){
                    switch(strip_parameter->strip_header_pos.parameter.u32){
                    case SAI_ACL_STRIP_HEADER_POS_L2:
                        res->scl.action.u.flow_action.strip_packet.start_packet_strip  = CTC_SCL_STRIP_START_TO_L2;
                        break;
                    case SAI_ACL_STRIP_HEADER_POS_L3:
                        res->scl.action.u.flow_action.strip_packet.start_packet_strip  = CTC_SCL_STRIP_START_TO_L3;
                        break;
                    default:
                        break;
                    }
                }

                if(strip_parameter->strip_header_offset.enable){
                    res->scl.action.u.flow_action.strip_packet.strip_extra_len  = 
                        strip_parameter->strip_header_offset.parameter.u32;
                }
//#endif /* !DUET2 */
            }
        }
        break;

    case PACKET_IPV4_IPIP:
        if(res->mapping_type == FIELD_ACL_ENTRY_TYPE){
            if(strip_parameter->strip_header.enable){
                res->u.ctc_acl_entry.action.packet_strip.start_packet_strip  = CTC_ACL_STRIP_START_TO_L4;
                res->u.ctc_acl_entry.action.packet_strip.packet_type         = PKT_TYPE_IPV4;
                res->u.ctc_acl_entry.action.packet_strip.strip_extra_len     = 0;
            }
        }
        break;
    
    default:
        goto out;
        break;
    }

    return 0;

out:
    return 0;
}

static int32_t 
__check_tunnel_header_valid_and_config(
            ctc_sai_acl_field_list_t        *field_list,
            fields_mapping_res_t            *res)
{

    int32_t packet_type = PACKET_UNKONW;
    uint32_t vni_mask = 0;
    uint32_t gre_mask = 0;

#ifdef GOLDENGATE
	uint8_t l4_protocol = 0;
    uint16_t l4_dst_port = 0;
    uint8_t is_l4_vxlan_dst_port = 0;
	
    l4_protocol = field_list->ipv4_ip_protocol.data.u8 & ~(field_list->ipv4_ip_protocol.mask.u8);

    /* dst_port eq mode, list dst-port eq 4789,so data == mask == 4789 */
    l4_dst_port = field_list->ipv4_dst_port.data.u16 & field_list->ipv4_dst_port.mask.u16;

    is_l4_vxlan_dst_port = field_list->ipv4_vxlan_vni_dst_port.enable;
 
    /* check parameters */
    if(((L4_PROTOCOL_UDP == l4_protocol) && (is_l4_vxlan_dst_port == TRUE)) ||
        ((L4_PROTOCOL_UDP == l4_protocol) && (is_l4_vxlan_dst_port == FALSE) && (L4_PROTOCOL_VXLAN_DEST_PORT == l4_dst_port)))
    {
        packet_type = PACKET_VXLAN;
    }else if(L4_PROTOCOL_GRE == l4_protocol){    
        packet_type = PACKET_IPV4_GRE;
    }
#endif

    /* set strip-header */
    switch(packet_type)
    {
    case PACKET_VXLAN:
        if (res->mapping_type == FIELD_ACL_ENTRY_TYPE)
        {
            if (field_list->is_ipv6)
            {
                res->aclv6_key.sub_flag &= ~(CTC_ACL_IPV6_KEY_SUB_FLAG_L4_SRC_PORT);
                res->aclv6_key.sub_flag &= ~(CTC_ACL_IPV6_KEY_SUB_FLAG_L4_DST_PORT);

                res->aclv6_key.sub_flag |= CTC_ACL_IPV6_KEY_SUB_FLAG_VNI;
            }
            else
            {
                res->acl_key.sub_flag &= ~(CTC_ACL_IPV4_KEY_SUB_FLAG_L4_SRC_PORT);
                res->acl_key.sub_flag &= ~(CTC_ACL_IPV4_KEY_SUB_FLAG_L4_DST_PORT);

                res->acl_key.sub_flag |= CTC_ACL_IPV4_KEY_SUB_FLAG_VNI;
            }

            if (field_list->ipv4_vxlan_vni.enable)
            {
                vni_mask = (~field_list->ipv4_vxlan_vni.mask.u32) & 0xFFFFFF;      /* 24-bit */
                if (field_list->is_ipv6)
                {
                    res->aclv6_key.vni      = field_list->ipv4_vxlan_vni.data.u32;
                    res->aclv6_key.vni_mask = vni_mask;
                }
                else
                {
                    res->acl_key.vni      = field_list->ipv4_vxlan_vni.data.u32;
                    res->acl_key.vni_mask = vni_mask;
                }
            }
            else
            {
                if (field_list->is_ipv6)
                {
                    res->aclv6_key.vni      = 0;
                    res->aclv6_key.vni_mask = 0;

                }
                else
                {
                    res->acl_key.vni      = 0;
                    res->acl_key.vni_mask = 0;
                }
            } 
        }
        else
        {
            CTC_SET_FLAG(res->scl.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_ACLQOS_USE_OUTER_LOOKUP);
            res->scl.action.u.flow_action.aclqos_outer_lookup = 0;

            if (field_list->is_ipv6)
            {
                res->scl.key.u.tcam_ipv6_key.sub_flag &= ~(CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_L4_SRC_PORT);
                res->scl.key.u.tcam_ipv6_key.sub_flag &= ~(CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_L4_DST_PORT);
                res->scl.key.u.tcam_ipv6_key.sub_flag |= CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_VNI;
            }
            else
            {
                res->scl.key.u.tcam_ipv4_key.sub_flag &= ~(CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_SRC_PORT);
                res->scl.key.u.tcam_ipv4_key.sub_flag &= ~(CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_DST_PORT);
                res->scl.key.u.tcam_ipv4_key.sub_flag |= CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_VNI;
            }

            if (field_list->ipv4_vxlan_vni.enable)
            {
                vni_mask = (~field_list->ipv4_vxlan_vni.mask.u32) & 0xFFFFFF;      /* 24-bit */
                if (field_list->is_ipv6)
                {
                    res->scl.key.u.tcam_ipv6_key.vni      = field_list->ipv4_vxlan_vni.data.u32;
                    res->scl.key.u.tcam_ipv6_key.vni_mask = vni_mask;
                }
                else
                {
                    res->scl.key.u.tcam_ipv4_key.vni      = field_list->ipv4_vxlan_vni.data.u32;
                    res->scl.key.u.tcam_ipv4_key.vni_mask = vni_mask;
                }
            }
            else
            {
                if (field_list->is_ipv6)
                {
                    res->scl.key.u.tcam_ipv6_key.vni      = 0;
                    res->scl.key.u.tcam_ipv6_key.vni_mask = 0;
                }
                else
                {
                    res->scl.key.u.tcam_ipv4_key.vni      = 0;
                    res->scl.key.u.tcam_ipv4_key.vni_mask = 0;
                }
            } 
        }
        break;

    case PACKET_IPV4_GRE:
        if (res->mapping_type == FIELD_ACL_ENTRY_TYPE)
        {

            if (field_list->is_ipv6)
            {
                res->aclv6_key.flag |= CTC_ACL_IPV6_KEY_FLAG_L4_PROTOCOL;
                if (SAI_ACL_GRE_NVGRE == field_list->gre_type.data.u32)
                {
                    res->aclv6_key.sub_flag |= CTC_ACL_IPV6_KEY_SUB_FLAG_NVGRE_KEY;
                }
                else
                {
                    res->aclv6_key.sub_flag |= CTC_ACL_IPV6_KEY_SUB_FLAG_GRE_KEY;
                }

                if (field_list->gre_key.enable)
                {               
                    gre_mask = (~(field_list->gre_key.mask.u32)) & 0xFFFFFFFF;
                    res->aclv6_key.gre_key = field_list->gre_key.data.u32;
                    res->aclv6_key.gre_key_mask = gre_mask;
                }
            }
            else
            {
                res->acl_key.flag |= CTC_ACL_IPV4_KEY_FLAG_L4_PROTOCOL;
                if (SAI_ACL_GRE_NVGRE == field_list->gre_type.data.u32)
                {
                    res->acl_key.sub_flag |= CTC_ACL_IPV4_KEY_SUB_FLAG_NVGRE_KEY;
                }
                else
                {
                    res->acl_key.sub_flag |= CTC_ACL_IPV4_KEY_SUB_FLAG_GRE_KEY;
                }

                if (field_list->gre_key.enable)
                {               
                    gre_mask = (~(field_list->gre_key.mask.u32)) & 0xFFFFFFFF;
                    res->acl_key.gre_key = field_list->gre_key.data.u32;
                    res->acl_key.gre_key_mask = gre_mask;
                }
            }   
        }
        else
        {
            CTC_SET_FLAG(res->scl.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_ACLQOS_USE_OUTER_LOOKUP);
            res->scl.action.u.flow_action.aclqos_outer_lookup = 0;

            if (field_list->is_ipv6)
            {
                res->scl.key.u.tcam_ipv6_key.flag |= CTC_SCL_TCAM_IPV6_KEY_FLAG_L4_PROTOCOL;
                if (SAI_ACL_GRE_NVGRE == field_list->gre_type.data.u32)
                {
                    res->scl.key.u.tcam_ipv6_key.sub_flag |= CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_NVGRE_KEY;
                }
                else
                {
                    res->scl.key.u.tcam_ipv6_key.sub_flag |= CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_GRE_KEY;
                }

                if (field_list->gre_key.enable)
                {               
                    gre_mask = (~(field_list->gre_key.mask.u32)) & 0xFFFFFFFF;
                    res->scl.key.u.tcam_ipv6_key.gre_key = field_list->gre_key.data.u32;
                    res->scl.key.u.tcam_ipv6_key.gre_key_mask = gre_mask;
                }
            }
            else
            {
                res->scl.key.u.tcam_ipv4_key.flag |= CTC_SCL_TCAM_IPV4_KEY_FLAG_L4_PROTOCOL;
                if (SAI_ACL_GRE_NVGRE == field_list->gre_type.data.u32)
                {
                    res->scl.key.u.tcam_ipv4_key.sub_flag |= CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_NVGRE_KEY;
                }
                else
                {
                    res->scl.key.u.tcam_ipv4_key.sub_flag |= CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_GRE_KEY;
                }

                if(field_list->gre_key.enable)
                {               
                    gre_mask = (~(field_list->gre_key.mask.u32)) & 0xFFFFFFFF;
                    res->scl.key.u.tcam_ipv4_key.gre_key = field_list->gre_key.data.u32;
                    res->scl.key.u.tcam_ipv4_key.gre_key_mask = gre_mask;
                }
            }
        }
    default:
        break;
    }

    return 0;
}


static stbl_acl_tunnel_t*
__tunnel_decap_alloc_erspan (ctc_sai_acl_tunnel_key_t *key, 
                                        stbl_acl_entry_t  *p_sdb_entry)
{
    stbl_acl_glb_t          *p_acl_glb = stbl_acl_glb_get_acl_glb();
    stbl_acl_tunnel_t       tunnel; 
    stbl_acl_tunnel_t       *p_sdb_tunnel = NULL;
    fields_mapping_res_t    sclentry = {FIELD_SCL_ENTRY_TYPE};
    sai_uint32_t            index = 0;
    uint32_t                group_id = 0;
    uint32                  gport = 0;
	strip_header_parameter_t  strip_header = {};
    
    sai_status_t            ret = SAI_STATUS_SUCCESS;
    
    sal_memset(&tunnel, 0, sizeof(tunnel));
    sal_memcpy(&tunnel.key, key, sizeof(ctc_sai_acl_tunnel_key_t));

    sal_memset(&sclentry, 0, sizeof(fields_mapping_res_t));
    sclentry.mapping_type = FIELD_SCL_ENTRY_TYPE;

    p_sdb_tunnel = stbl_acl_tunnel_get_acl_tunnel(&tunnel.key);
    if (NULL == p_sdb_tunnel)
    {
        stbl_acl_tunnel_add_acl_tunnel(&tunnel);
        p_sdb_tunnel = stbl_acl_tunnel_get_acl_tunnel(&tunnel.key);
        if (NULL == p_sdb_tunnel)
        {
            return NULL;
        }
    }

    /* if exist then ref count */
    if (p_sdb_tunnel->count > 0)
    {
        p_sdb_tunnel->count++;
        CTC_SAI_DEBUG("%s%d: get tunnel decap res inc count:count = %d\n",
            __FUNCTION__,__LINE__, p_sdb_tunnel->count);
        return p_sdb_tunnel;
    }

    /* create res */
    ctclib_opb_alloc_offset(&p_acl_glb->tunnel_opb, &index);
    p_sdb_tunnel->entry_id = index;

	ctclib_opb_alloc_offset(&p_acl_glb->erspan_service_opb, &index);
    p_sdb_tunnel->erspan_service_id = index;

    ctc_sai_nexthop_alloc_offset(&index);
    p_sdb_tunnel->erspan_nexthop_id = index;

    __tunnel_decap_alloc_erspan_nexthop(p_sdb_tunnel);

    __tunnel_decap_alloc_erspan_service_id(p_sdb_tunnel);

    __mapping_sai_acl_entry_to_ctc_acl_entry(
                &p_sdb_entry->outer_fields, 
                p_sdb_entry, 
                &sclentry);

    ctc_sai_port_objectid_to_gport(key->port_oid, &gport);
    group_id = ctc_sai_port_make_acl_group_id(CTC_INGRESS, gport);

    sclentry.scl.entry_id = p_sdb_tunnel->entry_id;
    sclentry.scl.key.type = CTC_SCL_KEY_TCAM_IPV4;
    sclentry.scl.key.u.tcam_ipv4_key.key_size = CTC_SCL_KEY_SIZE_DOUBLE;
    sclentry.scl.priority_valid = TRUE;
    sclentry.scl.priority = p_sdb_entry->priority;

    CTC_SET_FLAG(sclentry.scl.action.type, CTC_SCL_ACTION_FLOW);
    CTC_SET_FLAG(sclentry.scl.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_DENY_BRIDGE);
    CTC_SET_FLAG(sclentry.scl.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_DENY_LEARNING);
    CTC_SET_FLAG(sclentry.scl.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_DENY_ROUTE);

    __check_tunnel_header_valid_and_config(&p_sdb_entry->outer_fields, &sclentry);

    strip_header.strip_header = p_sdb_entry->action.strip_header;
    strip_header.strip_header_pos = p_sdb_entry->action.strip_header_pos;
    strip_header.strip_header_offset = p_sdb_entry->action.strip_header_offset;
    __config_strip_header(&p_sdb_entry->outer_fields, &strip_header, &sclentry);

    CTC_SET_FLAG(sclentry.scl.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_REDIRECT);
    sclentry.scl.action.u.flow_action.nh_id = p_sdb_tunnel->erspan_nexthop_id;

    ret = ctc_scl_add_entry(group_id, &sclentry.scl);
    ret = ctc_scl_install_entry(sclentry.scl.entry_id);
    p_sdb_tunnel->count++;

    (void)ret;
    return p_sdb_tunnel;
}


static stbl_acl_tunnel_t*
__tunnel_decap_alloc_tunnel(ctc_sai_acl_tunnel_key_t *key, 
                                     stbl_acl_entry_t  *p_sdb_entry)
{
    stbl_acl_glb_t *p_acl_glb = stbl_acl_glb_get_acl_glb();
    stbl_acl_tunnel_t   tunnel; 
    stbl_acl_tunnel_t   *p_sdb_tunnel = NULL;
    fields_mapping_res_t    sclentry = {FIELD_SCL_ENTRY_TYPE};
    sai_uint32_t            index = 0;
    uint32_t                group_id = 0;
    uint32                  gport = 0;
    
    sai_status_t            ret = SAI_STATUS_SUCCESS;
    
    sal_memset(&tunnel, 0, sizeof(tunnel));
    sal_memcpy(&tunnel.key, key, sizeof(ctc_sai_acl_tunnel_key_t));

    sal_memset(&sclentry, 0, sizeof(fields_mapping_res_t));
    sclentry.mapping_type = FIELD_SCL_ENTRY_TYPE;

    p_sdb_tunnel = stbl_acl_tunnel_get_acl_tunnel(&tunnel.key);
    if (NULL == p_sdb_tunnel)
    {
        stbl_acl_tunnel_add_acl_tunnel(&tunnel);
        p_sdb_tunnel = stbl_acl_tunnel_get_acl_tunnel(&tunnel.key);
        if (NULL == p_sdb_tunnel)
        {
            return NULL;
        }
    }

    /* if exist then ref count */
    if (p_sdb_tunnel->count > 0)
    {
        p_sdb_tunnel->count++;
        CTC_SAI_DEBUG("%s%d: get tunnel decap res inc count:count = %d\n",
            __FUNCTION__,__LINE__, p_sdb_tunnel->count);
        return p_sdb_tunnel;
    }

    /* create res */
    ctclib_opb_alloc_offset(&p_acl_glb->tunnel_opb, &index);
    p_sdb_tunnel->entry_id = index;

    __mapping_sai_acl_entry_to_ctc_acl_entry(
                &p_sdb_entry->outer_fields, 
                p_sdb_entry, 
                &sclentry);

    ctc_sai_port_objectid_to_gport(key->port_oid, &gport);
    group_id = ctc_sai_port_make_acl_group_id(CTC_INGRESS, gport);

    sclentry.scl.entry_id = p_sdb_tunnel->entry_id;
    if (key->fields.is_ipv6)
    {
        sclentry.scl.key.type = CTC_SCL_KEY_TCAM_IPV6;
    }
    else
    {
        sclentry.scl.key.type = CTC_SCL_KEY_TCAM_IPV4;
        sclentry.scl.key.u.tcam_ipv4_key.key_size = CTC_SCL_KEY_SIZE_DOUBLE;
    }
    
    sclentry.scl.priority_valid = TRUE;
    sclentry.scl.priority = p_sdb_entry->priority;

    CTC_SET_FLAG(sclentry.scl.action.type, CTC_SCL_ACTION_FLOW);
    CTC_SET_FLAG(sclentry.scl.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_DENY_BRIDGE);
    CTC_SET_FLAG(sclentry.scl.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_DENY_LEARNING);
    CTC_SET_FLAG(sclentry.scl.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_DENY_ROUTE);
    //CTC_SET_FLAG(sclentry.scl.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_INNERLOOKUP);
    CTC_SET_FLAG(sclentry.scl.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_METADATA);
    sclentry.scl.action.u.flow_action.metadata = p_sdb_tunnel->entry_id;

    __check_tunnel_header_valid_and_config(&p_sdb_entry->outer_fields, &sclentry);
#if 0
    CTC_SET_FLAG(sclentry.scl.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_STRIP_PACKET);
    sclentry.scl.action.u.flow_action.strip_packet.start_packet_strip = 3;
    sclentry.scl.action.u.flow_action.strip_packet.strip_extra_len    = 8;
    sclentry.scl.action.u.flow_action.strip_packet.packet_type        = 0;
#endif

    ret = ctc_scl_add_entry(group_id, &sclentry.scl);
    ret = ctc_scl_install_entry(sclentry.scl.entry_id);
    p_sdb_tunnel->count++;

    (void)ret;
    return p_sdb_tunnel;
}

#define ________SAI_ACL_L2_MAPPING_FUN
void  
__mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_ovlan(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ether_svlan.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_SVLAN);
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_STAG_VALID);
                res->aclv6_key.stag_valid = TRUE;
                res->aclv6_key.svlan      = field_list->ether_svlan.data.u16;
                res->aclv6_key.svlan_mask = field_list->ether_svlan.mask.u16;
                SAI_ACL_VLAN_WILDCARD_MASK(res->aclv6_key.svlan_mask);
            }
            else
            {
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_SVLAN);
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_STAG_VALID);
                res->acl_key.stag_valid = TRUE;
                res->acl_key.svlan      = field_list->ether_svlan.data.u16;
                res->acl_key.svlan_mask = field_list->ether_svlan.mask.u16;
                SAI_ACL_VLAN_WILDCARD_MASK(res->acl_key.svlan_mask);
            }
        }
        else 
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_SVLAN);
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_STAG_VALID);
                res->sclv6_key.stag_valid = TRUE;
                res->sclv6_key.svlan      = field_list->ether_svlan.data.u16;
                res->sclv6_key.svlan_mask = field_list->ether_svlan.mask.u16;
                SAI_ACL_VLAN_WILDCARD_MASK(res->sclv6_key.svlan_mask);
            }
            else
            {
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_SVLAN);
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_STAG_VALID);
                res->scl_key.stag_valid = TRUE;
                res->scl_key.svlan      = field_list->ether_svlan.data.u16;
                res->scl_key.svlan_mask = field_list->ether_svlan.mask.u16;
                SAI_ACL_VLAN_WILDCARD_MASK(res->scl_key.svlan_mask);
            }
        }
    }
}

void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_ovlan_cos(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ether_svlan_pri.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_STAG_COS);
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_STAG_VALID);
                res->aclv6_key.stag_valid = TRUE;
                res->aclv6_key.stag_cos      = field_list->ether_svlan_pri.data.u8;
                res->aclv6_key.stag_cos_mask = field_list->ether_svlan_pri.mask.u8;
                SAI_ACL_COS_WILDCARD_MASK(res->aclv6_key.stag_cos_mask);
            }
            else
            {
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_STAG_COS);
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_STAG_VALID);
                res->acl_key.stag_valid = TRUE;
                res->acl_key.stag_cos      = field_list->ether_svlan_pri.data.u8;
                res->acl_key.stag_cos_mask = field_list->ether_svlan_pri.mask.u8;
                SAI_ACL_COS_WILDCARD_MASK(res->acl_key.stag_cos_mask);
            }
        }
        else
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_STAG_COS);
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_STAG_VALID);
                res->sclv6_key.stag_valid = TRUE;
                res->sclv6_key.stag_cos      = field_list->ether_svlan_pri.data.u8;
                res->sclv6_key.stag_cos_mask = field_list->ether_svlan_pri.mask.u8;
                SAI_ACL_COS_WILDCARD_MASK(res->sclv6_key.stag_cos_mask);  
            }
            else
            {
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_STAG_COS);
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_STAG_VALID);
                res->scl_key.stag_valid = TRUE;
                res->scl_key.stag_cos      = field_list->ether_svlan_pri.data.u8;
                res->scl_key.stag_cos_mask = field_list->ether_svlan_pri.mask.u8;
                SAI_ACL_COS_WILDCARD_MASK(res->scl_key.stag_cos_mask);
            }
        }
    }
}

void
__mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_ivlan(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ether_cvlan.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_CVLAN);
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_VALID);
                res->aclv6_key.ctag_valid = TRUE;
                res->aclv6_key.cvlan        = field_list->ether_cvlan.data.u16;
                res->aclv6_key.cvlan_mask   = field_list->ether_cvlan.mask.u16;
                SAI_ACL_VLAN_WILDCARD_MASK(res->aclv6_key.cvlan_mask);
            }
            else
            {
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_CVLAN);
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_VALID);
                res->acl_key.ctag_valid = TRUE;
                res->acl_key.cvlan        = field_list->ether_cvlan.data.u16;
                res->acl_key.cvlan_mask   = field_list->ether_cvlan.mask.u16;
                SAI_ACL_VLAN_WILDCARD_MASK(res->acl_key.cvlan_mask);
            }
        }
        else 
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_CVLAN);
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_CTAG_VALID);
                res->sclv6_key.ctag_valid = TRUE;
                res->sclv6_key.cvlan        = field_list->ether_cvlan.data.u16;
                res->sclv6_key.cvlan_mask   = field_list->ether_cvlan.mask.u16;
                SAI_ACL_VLAN_WILDCARD_MASK(res->sclv6_key.cvlan_mask);
            }
            else
            {
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_CVLAN);
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_CTAG_VALID);
                res->scl_key.ctag_valid = TRUE;
                res->scl_key.cvlan        = field_list->ether_cvlan.data.u16;
                res->scl_key.cvlan_mask   = field_list->ether_cvlan.mask.u16;
                SAI_ACL_VLAN_WILDCARD_MASK(res->scl_key.cvlan_mask);
            }
        }
    }
}

void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_ivlan_cos(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ether_cvlan_pri.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_COS);
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_VALID);
                res->aclv6_key.ctag_valid = TRUE;
                res->aclv6_key.ctag_cos      = field_list->ether_cvlan_pri.data.u8;
                res->aclv6_key.ctag_cos_mask = field_list->ether_cvlan_pri.mask.u8;
                SAI_ACL_COS_WILDCARD_MASK(res->aclv6_key.ctag_cos_mask);
            }
            else
            {
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_COS);
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_VALID);
                res->acl_key.ctag_valid = TRUE;
                res->acl_key.ctag_cos      = field_list->ether_cvlan_pri.data.u8;
                res->acl_key.ctag_cos_mask = field_list->ether_cvlan_pri.mask.u8;
                SAI_ACL_COS_WILDCARD_MASK(res->acl_key.ctag_cos_mask);
            }
        }
        else 
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_CTAG_COS);
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_CTAG_VALID);
                res->sclv6_key.ctag_valid = TRUE;
                res->sclv6_key.ctag_cos      = field_list->ether_cvlan_pri.data.u8;
                res->sclv6_key.ctag_cos_mask = field_list->ether_cvlan_pri.mask.u8;
                SAI_ACL_COS_WILDCARD_MASK(res->sclv6_key.ctag_cos_mask);
            }
            else
            {
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_CTAG_COS);
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_CTAG_VALID);
                res->scl_key.ctag_valid = TRUE;
                res->scl_key.ctag_cos      = field_list->ether_cvlan_pri.data.u8;
                res->scl_key.ctag_cos_mask = field_list->ether_cvlan_pri.mask.u8;
                SAI_ACL_COS_WILDCARD_MASK(res->scl_key.ctag_cos_mask);
            }
        }
    }
}

/*added by yejl for bug 49791, 2018-11-14*/
void
__mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_untag_vlan(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ether_untag_vlan.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_STAG_VALID);
                res->aclv6_key.stag_valid = FALSE;
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_VALID);
                res->aclv6_key.ctag_valid = FALSE;
            }
            else
            {
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_STAG_VALID);
                res->acl_key.stag_valid = FALSE;
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_VALID);
                res->acl_key.ctag_valid = FALSE;
            }
        }
        else 
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_STAG_VALID);
                res->sclv6_key.stag_valid = FALSE;
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_CTAG_VALID);
                res->sclv6_key.ctag_valid = FALSE;
            }
            else
            {
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_STAG_VALID);
                res->scl_key.stag_valid = FALSE;
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_CTAG_VALID);
                res->scl_key.ctag_valid = FALSE;
            }
        }
    }
}
/*end*/

void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_ether_macsa(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ether_macsa.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_MAC_SA);
                sal_memcpy(res->aclv6_key.mac_sa,       field_list->ether_macsa.data.mac, sizeof(sai_mac_t));
                sal_memcpy(res->aclv6_key.mac_sa_mask,  field_list->ether_macsa.mask.mac, sizeof(sai_mac_t));
                SAI_ACL_MAC_WILDCARD_MASK(res->aclv6_key.mac_sa_mask);
            }
            else 
            {
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_MAC_SA);
                sal_memcpy(res->acl_key.mac_sa,       field_list->ether_macsa.data.mac, sizeof(sai_mac_t));
                sal_memcpy(res->acl_key.mac_sa_mask,  field_list->ether_macsa.mask.mac, sizeof(sai_mac_t));
                SAI_ACL_MAC_WILDCARD_MASK(res->acl_key.mac_sa_mask);
            }
        }
        else 
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_MAC_SA);
                sal_memcpy(res->sclv6_key.mac_sa,       field_list->ether_macsa.data.mac, sizeof(sai_mac_t));
                sal_memcpy(res->sclv6_key.mac_sa_mask,  field_list->ether_macsa.mask.mac, sizeof(sai_mac_t));
                SAI_ACL_MAC_WILDCARD_MASK(res->sclv6_key.mac_sa_mask);
            }
            else
            {
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_MAC_SA);
                sal_memcpy(res->scl_key.mac_sa,       field_list->ether_macsa.data.mac, sizeof(sai_mac_t));
                sal_memcpy(res->scl_key.mac_sa_mask,  field_list->ether_macsa.mask.mac, sizeof(sai_mac_t));
                SAI_ACL_MAC_WILDCARD_MASK(res->scl_key.mac_sa_mask);
            }
        }
    }
}

void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_ether_macda(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ether_macda.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_MAC_DA);
                sal_memcpy(res->aclv6_key.mac_da,       field_list->ether_macda.data.mac, sizeof(sai_mac_t));
                sal_memcpy(res->aclv6_key.mac_da_mask,  field_list->ether_macda.mask.mac, sizeof(sai_mac_t));
                SAI_ACL_MAC_WILDCARD_MASK(res->aclv6_key.mac_da_mask);
            }
            else
            {
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_MAC_DA);
                sal_memcpy(res->acl_key.mac_da,       field_list->ether_macda.data.mac, sizeof(sai_mac_t));
                sal_memcpy(res->acl_key.mac_da_mask,  field_list->ether_macda.mask.mac, sizeof(sai_mac_t));
                SAI_ACL_MAC_WILDCARD_MASK(res->acl_key.mac_da_mask);
            }
        }
        else
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_MAC_DA);
                sal_memcpy(res->sclv6_key.mac_da,       field_list->ether_macda.data.mac, sizeof(sai_mac_t));
                sal_memcpy(res->sclv6_key.mac_da_mask,  field_list->ether_macda.mask.mac, sizeof(sai_mac_t));
                SAI_ACL_MAC_WILDCARD_MASK(res->sclv6_key.mac_da_mask);
            }
            else
            {
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_MAC_DA);
                sal_memcpy(res->scl_key.mac_da,       field_list->ether_macda.data.mac, sizeof(sai_mac_t));
                sal_memcpy(res->scl_key.mac_da_mask,  field_list->ether_macda.mask.mac, sizeof(sai_mac_t));
                SAI_ACL_MAC_WILDCARD_MASK(res->scl_key.mac_da_mask);
            }
        }
    }
}

void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l2_layer_ether_type(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
#ifdef GOLDENGATE
    struct eth_type_map{
        uint16_t    type;
        uint8_t     l3_type;
    } type2map[] = {
        {0x0800,CTC_PARSER_L3_TYPE_IPV4},
        {0x0806,CTC_PARSER_L3_TYPE_ARP},
        {0x86DD,CTC_PARSER_L3_TYPE_IPV6},
        {0x8847,CTC_PARSER_L3_TYPE_MPLS},
        {0x8848,CTC_PARSER_L3_TYPE_MPLS_MCAST},
        {0x8906,CTC_PARSER_L3_TYPE_FCOE},
        {0x22F3,CTC_PARSER_L3_TYPE_TRILL},
        {0x8902,CTC_PARSER_L3_TYPE_ETHER_OAM},
        {0x88E7,CTC_PARSER_L3_TYPE_CMAC},
        {0x8809,CTC_PARSER_L3_TYPE_SLOW_PROTO},
        {0x88F7,CTC_PARSER_L3_TYPE_PTP},
    };
    uint32_t    idx = 0;
#endif
    uint32_t    hit_l3_type = 0;

    CTC_SAI_DEBUG_FUNC();
    if (field_list->ether_type.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                //GG not support CTC_ACL_IPV6_KEY_FLAG_L3_TYPE
            }
            else
            {
#ifdef GOLDENGATE
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);

                for(idx = 0; idx < sizeof(type2map)/sizeof(type2map[0]); idx++)
                {
                    if (type2map[idx].type == field_list->ether_type.data.u16 && 
                        0 == field_list->ether_type.mask.u16)
                    {
                        res->acl_key.l3_type = type2map[idx].l3_type;
                        res->acl_key.l3_type_mask = 0xf;

                        hit_l3_type = 1;
                        break;
                    }
                }
#else
                if (0x0800 == field_list->ether_type.data.u16 &&
                    0 == field_list->ether_type.mask.u16)
                {
                    CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_IPV4_PACKET);
                    res->acl_key.ipv4_packet = 1;
                    
                    hit_l3_type = 1;
                }
                else if (0x0806 == field_list->ether_type.data.u16 &&
                         0 == field_list->ether_type.mask.u16)
                {
                    CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_ARP_PACKET);
                    res->acl_key.arp_packet = 1;

                    hit_l3_type = 1;
                }
#endif
                if (!hit_l3_type)
                {
                    res->acl_key.l3_type      = CTC_PARSER_L3_TYPE_NONE;
                    res->acl_key.l3_type_mask = 0xf;
                    CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_ETH_TYPE);
                }
                res->acl_key.eth_type      = field_list->ether_type.data.u16;
                res->acl_key.eth_type_mask = field_list->ether_type.mask.u16;
                SAI_ACL_WILDCARD_MASK(res->acl_key.eth_type_mask);
            }
        }
        else 
        {
            if (field_list->is_ipv6)
            {
                //GG not support, GB support CTC_SCL_TCAM_IPV6_KEY_FLAG_L3_TYPE
            }
            else
            {
    #ifdef GOLDENGATE
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_L3_TYPE);

                for(idx = 0; idx < sizeof(type2map)/sizeof(type2map[0]); idx++)
                {
                    if (type2map[idx].type == field_list->ether_type.data.u16 && 
                        0 == field_list->ether_type.mask.u16)
                    {
                        res->scl_key.l3_type = type2map[idx].l3_type;
                        res->scl_key.l3_type_mask = 0xf;

                        hit_l3_type = 1;
                        break;
                    }
                }
    #else
                /* not support GB */
    #endif
                if (!hit_l3_type)
                {
                    res->scl_key.l3_type      = CTC_PARSER_L3_TYPE_NONE;
                    res->scl_key.l3_type_mask = 0xf;
                    CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_ETH_TYPE);
                }
                res->scl_key.eth_type      = field_list->ether_type.data.u16;
                res->scl_key.eth_type_mask = field_list->ether_type.mask.u16;
                SAI_ACL_WILDCARD_MASK(res->scl_key.eth_type_mask);
            }
        }
    }
}

#define ________SAI_ACL_L3_MAPPNG_FUN
void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ipda(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
        
    if (field_list->ipv4_ipda.enable)
    {
#ifdef TAPPRODUCT
        if(IS_EQ_WILDCARD_MASK_32(field_list->ipv4_ipda.mask.ip4))
        {
            return ;
        }
#endif

        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
#ifdef GOLDENGATE
            CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
#endif
            res->acl_key.l3_type = CTC_PARSER_L3_TYPE_IPV4;
            res->acl_key.l3_type_mask = 0xf;
            
            CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_IP_DA);
            res->acl_key.ip_da        = field_list->ipv4_ipda.data.ip4;
            res->acl_key.ip_da_mask   = field_list->ipv4_ipda.mask.ip4;
            SAI_ACL_WILDCARD_MASK(res->acl_key.ip_da_mask);
        }
        else
        {        
            CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_L3_TYPE);
            res->scl_key.l3_type        = CTC_PARSER_L3_TYPE_IPV4;
            res->scl_key.l3_type_mask   = 0xF;

            CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_DA);
            res->scl_key.ip_da        = field_list->ipv4_ipda.data.ip4;
            res->scl_key.ip_da_mask   = field_list->ipv4_ipda.mask.ip4;
            SAI_ACL_WILDCARD_MASK(res->scl_key.ip_da_mask);
        }
    }
    if (field_list->ipv6_ipda.enable)
    {
#ifdef TAPPRODUCT
        if (IS_EQ_WILDCARD_MASK_128(&field_list->ipv6_ipda.mask.ip6))
            return;
#endif

        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_IP_DA);
            sal_memcpy(&res->aclv6_key.ip_da, field_list->ipv6_ipda.data.ip6, sizeof(sai_ip6_t));
            SAI_ACL_IPV6_WILDCARD_MASK(field_list->ipv6_ipda.mask.ip6);
            sal_memcpy(&res->aclv6_key.ip_da_mask, field_list->ipv6_ipda.mask.ip6, sizeof(sai_ip6_t));
        }
        else
        {   
#ifdef GREATBELT
            CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_L3_TYPE);
            res->sclv6_key.l3_type        = CTC_PARSER_L3_TYPE_IPV6;
            res->sclv6_key.l3_type_mask   = 0xF;
#endif
            CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_IP_DA);
            sal_memcpy(&res->sclv6_key.ip_da, field_list->ipv6_ipda.data.ip6, sizeof(sai_ip6_t));
            SAI_ACL_IPV6_WILDCARD_MASK(field_list->ipv6_ipda.mask.ip6);
            sal_memcpy(&res->sclv6_key.ip_da_mask, field_list->ipv6_ipda.mask.ip6, sizeof(sai_ip6_t));
        }
    }
}

void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ipsa(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{

    if (field_list->ipv4_ipsa.enable)
    {
#ifdef TAPPRODUCT
        if(IS_EQ_WILDCARD_MASK_32(field_list->ipv4_ipsa.mask.ip4))
        {
            return ;
        }
#endif

        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
#ifdef GOLDENGATE
            CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
#endif
            res->acl_key.l3_type = CTC_PARSER_L3_TYPE_IPV4;
            res->acl_key.l3_type_mask = 0xf;
            
            CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_IP_SA);
            res->acl_key.ip_sa        = field_list->ipv4_ipsa.data.ip4;
            res->acl_key.ip_sa_mask   = field_list->ipv4_ipsa.mask.ip4;
            SAI_ACL_WILDCARD_MASK(res->acl_key.ip_sa_mask);
        }
        else
        {       
            CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_L3_TYPE);
            res->scl_key.l3_type        = CTC_PARSER_L3_TYPE_IPV4;
            res->scl_key.l3_type_mask   = 0xF;

            CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_SA);
            res->scl_key.ip_sa        = field_list->ipv4_ipsa.data.ip4;
            res->scl_key.ip_sa_mask   = field_list->ipv4_ipsa.mask.ip4;
            SAI_ACL_WILDCARD_MASK(res->scl_key.ip_sa_mask);
        }
    }
    if (field_list->ipv6_ipsa.enable)
    {
#ifdef TAPPRODUCT
        if (IS_EQ_WILDCARD_MASK_128(&field_list->ipv6_ipsa.mask.ip6))
            return;
#endif

        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {    
            CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_IP_SA);
            sal_memcpy(&res->aclv6_key.ip_sa, field_list->ipv6_ipsa.data.ip6, sizeof(sai_ip6_t));
            SAI_ACL_IPV6_WILDCARD_MASK(field_list->ipv6_ipsa.mask.ip6);
            sal_memcpy(&res->aclv6_key.ip_sa_mask, field_list->ipv6_ipsa.mask.ip6, sizeof(sai_ip6_t));
        }
        else
        {  
#ifdef GREATBELT
            CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_L3_TYPE);
            res->sclv6_key.l3_type        = CTC_PARSER_L3_TYPE_IPV6;
            res->sclv6_key.l3_type_mask   = 0xF;
#endif

            CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_IP_SA);
            sal_memcpy(&res->sclv6_key.ip_sa, field_list->ipv6_ipsa.data.ip6, sizeof(sai_ip6_t));
            SAI_ACL_IPV6_WILDCARD_MASK(field_list->ipv6_ipsa.mask.ip6);

            sal_memcpy(&res->sclv6_key.ip_sa_mask, field_list->ipv6_ipsa.mask.ip6, sizeof(sai_ip6_t));
        }
    }
}
void
__mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_flow_label(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ipv6_flow_label.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_FLOW_LABEL);
                res->aclv6_key.flow_label      = field_list->ipv6_flow_label.data.u32;
                res->aclv6_key.flow_label_mask = field_list->ipv6_flow_label.mask.u32;
                SAI_ACLV6_FLOW_LABEL_WILDCARD_MASK(res->aclv6_key.flow_label_mask);
            }
        }
        else 
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_FLOW_LABEL);
                res->sclv6_key.flow_label      = field_list->ipv6_flow_label.data.u32;
                res->sclv6_key.flow_label_mask = field_list->ipv6_flow_label.mask.u32;
                SAI_ACLV6_FLOW_LABEL_WILDCARD_MASK(res->sclv6_key.flow_label_mask);
            }
        }
    }
}


void
__mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ip_protocol(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ipv4_ip_protocol.enable)
    {
#ifdef TAPPRODUCT
        if(IS_EQ_WILDCARD_MASK_8(field_list->ipv4_ip_protocol.mask.u8))
        {
            return ;
        }
#endif

        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                res->aclv6_key.l3_type      = CTC_PARSER_L3_TYPE_IPV6;
                res->aclv6_key.l3_type_mask = 0xf;
                /* if v6 l3 protocol 0 means options bug 46277 */
                if (0 == field_list->ipv4_ip_protocol.data.u8 &&
                    0 == field_list->ipv4_ip_protocol.mask.u8)
                {
                    CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_IP_OPTION);
                    res->aclv6_key.ip_option = field_list->ipv4_ip_options.data.u8;
                }
                else
                {
                    CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_L4_PROTOCOL);
                    res->aclv6_key.l4_protocol      = field_list->ipv4_ip_protocol.data.u8;
                    res->aclv6_key.l4_protocol_mask = field_list->ipv4_ip_protocol.mask.u8;
                    SAI_ACL_WILDCARD_MASK(res->aclv6_key.l4_protocol_mask);
                }
            }
            else
            {
#ifdef GOLDENGATE
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
#endif
                /* SDK bug, if l4_protocol == 0 && l4_protocol_mask == 255 then ,not set flag l4_protocol */
                if(field_list->ipv4_ip_protocol.mask.u8 != 0xFF ||
                   res->acl_key.l4_protocol != 0)
                {
                    res->acl_key.l3_type      = CTC_PARSER_L3_TYPE_IPV4;
                    res->acl_key.l3_type_mask = 0xf;
                    CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_L4_PROTOCOL);
                    
                    res->acl_key.l4_protocol      = field_list->ipv4_ip_protocol.data.u8;
                    res->acl_key.l4_protocol_mask = field_list->ipv4_ip_protocol.mask.u8;
                    SAI_ACL_WILDCARD_MASK(res->acl_key.l4_protocol_mask);
                }
            }
        }
        else 
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_L4_PROTOCOL);
                res->sclv6_key.l3_type      = CTC_PARSER_L3_TYPE_IPV6;
                res->sclv6_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_L4_PROTOCOL);
                
                res->sclv6_key.l4_protocol      = field_list->ipv4_ip_protocol.data.u8;
                res->sclv6_key.l4_protocol_mask = field_list->ipv4_ip_protocol.mask.u8;
                SAI_ACL_WILDCARD_MASK(res->sclv6_key.l4_protocol_mask);
            }
            else
            {
#ifdef GOLDENGATE
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_L4_PROTOCOL);
#endif
                /* SDK bug, if l4_protocol == 0 && l4_protocol_mask == 255 then ,not set flag l4_protocol */
                if(field_list->ipv4_ip_protocol.mask.u8 != 0xFF ||
                   res->scl_key.l4_protocol != 0)
                {
                    res->scl_key.l3_type      = CTC_PARSER_L3_TYPE_IPV4;
                    res->scl_key.l3_type_mask = 0xf;
                    CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_L4_PROTOCOL);
                    
                    res->scl_key.l4_protocol      = field_list->ipv4_ip_protocol.data.u8;
                    res->scl_key.l4_protocol_mask = field_list->ipv4_ip_protocol.mask.u8;
                    SAI_ACL_WILDCARD_MASK(res->scl_key.l4_protocol_mask);
                }
            }
        }
    }
}

void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ip_dscp(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ipv4_dscp.enable)
    {
#ifdef TAPPRODUCT
        if(IS_EQ_WILDCARD_MASK_8(field_list->ipv4_dscp.mask.u8))
        {
            return ;
        }
#endif
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                res->aclv6_key.l3_type = CTC_PARSER_L3_TYPE_IPV6;
                res->aclv6_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_DSCP);
                res->aclv6_key.dscp = field_list->ipv4_dscp.data.u8;
                res->aclv6_key.dscp_mask = field_list->ipv4_dscp.mask.u8;
                SAI_ACL_DSCP_WILDCARD_MASK(res->aclv6_key.dscp_mask);
            }
            else
            {
#ifdef GOLDENGATE
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
#endif
                res->acl_key.l3_type = CTC_PARSER_L3_TYPE_IPV4;
                res->acl_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_DSCP);
                res->acl_key.dscp = field_list->ipv4_dscp.data.u8;
                res->acl_key.dscp_mask = field_list->ipv4_dscp.mask.u8;
                SAI_ACL_DSCP_WILDCARD_MASK(res->acl_key.dscp_mask);
            }
        }
        else 
        {
            if (field_list->is_ipv6)
            {
#ifdef GREATBELT
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_L3_TYPE);
#endif
                res->sclv6_key.l3_type = CTC_PARSER_L3_TYPE_IPV6;
                res->sclv6_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_DSCP);
                res->sclv6_key.dscp = field_list->ipv4_dscp.data.u8;
                res->sclv6_key.dscp_mask = field_list->ipv4_dscp.mask.u8;
                SAI_ACL_DSCP_WILDCARD_MASK(res->sclv6_key.dscp_mask);
            }
            else
            {
#ifdef GOLDENGATE
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_L3_TYPE);
#endif
                res->scl_key.l3_type = CTC_PARSER_L3_TYPE_IPV4;
                res->scl_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_DSCP);
                res->scl_key.dscp = field_list->ipv4_dscp.data.u8;
                res->scl_key.dscp_mask = field_list->ipv4_dscp.mask.u8;
                SAI_ACL_DSCP_WILDCARD_MASK(res->scl_key.dscp_mask);
            }
        }
    }
}


void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ip_precedence(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ipv4_ip_precedence.enable)
    {
#ifdef TAPPRODUCT
        if(IS_EQ_WILDCARD_MASK_8(field_list->ipv4_ip_precedence.mask.u8))
        {
            return ;
        }
#endif     
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                res->aclv6_key.l3_type = CTC_PARSER_L3_TYPE_IPV6;
                res->aclv6_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_PRECEDENCE);
                res->aclv6_key.dscp = field_list->ipv4_ip_precedence.data.u8;
                res->aclv6_key.dscp_mask = field_list->ipv4_ip_precedence.mask.u8;
                SAI_ACL_COS_WILDCARD_MASK(res->aclv6_key.dscp_mask);       
            }
            else
            {
#ifdef GOLDENGATE
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
#endif
                res->acl_key.l3_type = CTC_PARSER_L3_TYPE_IPV4;
                res->acl_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_PRECEDENCE);
                res->acl_key.dscp = field_list->ipv4_ip_precedence.data.u8;
                res->acl_key.dscp_mask = field_list->ipv4_ip_precedence.mask.u8;
                SAI_ACL_COS_WILDCARD_MASK(res->acl_key.dscp_mask);            
            }

        }
        else 
        {
            if (field_list->is_ipv6)
            {
#ifdef GREATBELT
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_L3_TYPE);
#endif
                res->sclv6_key.l3_type = CTC_PARSER_L3_TYPE_IPV6;
                res->sclv6_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_PRECEDENCE);
                res->sclv6_key.dscp = field_list->ipv4_ip_precedence.data.u8;
                res->sclv6_key.dscp_mask = field_list->ipv4_ip_precedence.mask.u8;
                SAI_ACL_COS_WILDCARD_MASK(res->sclv6_key.dscp_mask);   
            }
            else
            {
#ifdef GOLDENGATE
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_L3_TYPE);
#endif
                res->scl_key.l3_type = CTC_PARSER_L3_TYPE_IPV4;
                res->scl_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_PRECEDENCE);
                res->scl_key.dscp = field_list->ipv4_ip_precedence.data.u8;
                res->scl_key.dscp_mask = field_list->ipv4_ip_precedence.mask.u8;
                SAI_ACL_COS_WILDCARD_MASK(res->scl_key.dscp_mask);
            }
        }
    }
}


void
__mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ip_frag(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    ctc_parser_global_cfg_t cfg;

    sal_memset(&cfg,0,sizeof(ctc_parser_global_cfg_t));
    if (field_list->ipv4_ip_frag.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                res->aclv6_key.l3_type = CTC_PARSER_L3_TYPE_IPV6;
                res->aclv6_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_IP_FRAG);
                res->aclv6_key.ip_frag = field_list->ipv4_ip_frag.data.u8;
                if (res->aclv6_key.ip_frag == CTC_IP_FRAG_SMALL)
                {
                    cfg.small_frag_offset = 3;
                    ctc_parser_set_global_cfg(&cfg);
                }
            }
            else
            {
#ifdef GOLDENGATE
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
#endif
                res->acl_key.l3_type = CTC_PARSER_L3_TYPE_IPV4;
                res->acl_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_IP_FRAG);
                res->acl_key.ip_frag = field_list->ipv4_ip_frag.data.u8;
                if (res->acl_key.ip_frag == CTC_IP_FRAG_SMALL)
                {
                    cfg.small_frag_offset = 3;
                    ctc_parser_set_global_cfg(&cfg);
                }
            } 
        }
        else 
        {
            if (field_list->is_ipv6)
            {
#ifdef GREATBELT
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_L3_TYPE);
#endif
                res->sclv6_key.l3_type = CTC_PARSER_L3_TYPE_IPV6;
                res->sclv6_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_IP_FRAG);
                res->sclv6_key.ip_frag = field_list->ipv4_ip_frag.data.u8;
                if (res->sclv6_key.ip_frag == CTC_IP_FRAG_SMALL)
                {
                    cfg.small_frag_offset = 3;
                    ctc_parser_set_global_cfg(&cfg);
                }
            }
            else
            {
#ifdef GOLDENGATE
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_L3_TYPE);
#endif
                res->scl_key.l3_type = CTC_PARSER_L3_TYPE_IPV4;
                res->scl_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_FRAG);
                res->scl_key.ip_frag = field_list->ipv4_ip_frag.data.u8;
                if (res->scl_key.ip_frag == CTC_IP_FRAG_SMALL)
                {
                    cfg.small_frag_offset = 3;
                    ctc_parser_set_global_cfg(&cfg);
                }
            }
        }
    }
}

void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_igmp_type(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ipv4_igmp_type.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                /*Ipv6 not support*/
            }
            else
            {
                CTC_SET_FLAG(res->acl_key.sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_IGMP_TYPE);
                res->acl_key.igmp_type = field_list->ipv4_igmp_type.data.u8;
                res->acl_key.igmp_type_mask = 0xff;
            }
        }
        else 
        {
            if (field_list->is_ipv6)
            {
                 /*Ipv6 not support*/
            }
            else
            {
                CTC_SET_FLAG(res->scl_key.sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_IGMP_TYPE);
                res->scl_key.igmp_type = field_list->ipv4_igmp_type.data.u8;
                res->scl_key.igmp_type_mask = 0xff;
            }
        }
    }
}

void
__mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_icmp_type(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ipv4_icmp_type.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->aclv6_key.sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_ICMP_TYPE);
                res->aclv6_key.icmp_type = field_list->ipv4_icmp_type.data.u8;
                res->aclv6_key.icmp_type_mask = 0xff;
            }
            else
            {
                CTC_SET_FLAG(res->acl_key.sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_TYPE);
                res->acl_key.icmp_type = field_list->ipv4_icmp_type.data.u8;
                res->acl_key.icmp_type_mask = 0xff;
            }
        }
        else 
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->sclv6_key.sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_ICMP_TYPE);
                res->sclv6_key.icmp_type = field_list->ipv4_icmp_type.data.u8;
                res->sclv6_key.icmp_type_mask = 0xff;
            }
            else
            {
                CTC_SET_FLAG(res->scl_key.sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ICMP_TYPE);
                res->scl_key.icmp_type = field_list->ipv4_icmp_type.data.u8;
                res->scl_key.icmp_type_mask = 0xff;
            }
        }
    }
}

void
__mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_icmp_code(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ipv4_icmp_code.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->aclv6_key.sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_ICMP_CODE);
                res->aclv6_key.icmp_code = field_list->ipv4_icmp_code.data.u8;
                res->aclv6_key.icmp_code_mask = 0xff;
            }
            else
            {
                CTC_SET_FLAG(res->acl_key.sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_CODE);
                res->acl_key.icmp_code = field_list->ipv4_icmp_code.data.u8;
                res->acl_key.icmp_code_mask = 0xff;
            }
        }
        else 
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->sclv6_key.sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_ICMP_CODE);
                res->sclv6_key.icmp_code = field_list->ipv4_icmp_code.data.u8;
                res->sclv6_key.icmp_code_mask = 0xff;
            }
            else
            {
                CTC_SET_FLAG(res->scl_key.sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ICMP_CODE);
                res->scl_key.icmp_code = field_list->ipv4_icmp_code.data.u8;
                res->scl_key.icmp_code_mask = 0xff;
            }
        }
    }
}

void
__mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ip_options(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ipv4_ip_options.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                res->aclv6_key.l3_type = CTC_PARSER_L3_TYPE_IPV6;
                res->aclv6_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_IP_OPTION);
                res->aclv6_key.ip_option = field_list->ipv4_ip_options.data.u8;
            }
            else
            {
#ifdef GOLDENGATE
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
#endif
                res->acl_key.l3_type = CTC_PARSER_L3_TYPE_IPV4;
                res->acl_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_IP_OPTION);
                res->acl_key.ip_option = field_list->ipv4_ip_options.data.u8;
            }
        }
        else 
        {
            if (field_list->is_ipv6)
            {
#ifdef GREATBELT
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_L3_TYPE);
#endif
                res->sclv6_key.l3_type = CTC_PARSER_L3_TYPE_IPV6;
                res->sclv6_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->sclv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_IP_OPTION);
                res->sclv6_key.ip_option = field_list->ipv4_ip_options.data.u8;
            }
            else
            {
#ifdef GOLDENGATE
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_L3_TYPE);
#endif
                res->scl_key.l3_type = CTC_PARSER_L3_TYPE_IPV4;
                res->scl_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_OPTION);
                res->scl_key.ip_option = field_list->ipv4_ip_options.data.u8;
            }
        }
    }
}

void
__mapping_sai_acl_entry_to_ctc_acl_entry_l3_layer_ip_routed_packet(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ipv4_routed_packet.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                res->aclv6_key.l3_type = CTC_PARSER_L3_TYPE_IPV6;
                res->aclv6_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->aclv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_ROUTED_PACKET);
                res->aclv6_key.routed_packet = field_list->ipv4_routed_packet.data.u8;   
            }
            else
            {
#ifdef GOLDENGATE
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
#endif
                res->acl_key.l3_type = CTC_PARSER_L3_TYPE_IPV4;
                res->acl_key.l3_type_mask = 0xf;
                CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_ROUTED_PACKET);
                res->acl_key.routed_packet = field_list->ipv4_routed_packet.data.u8;
            }
        }
        else 
        {
            /* not support. routed packet is after scl */
        }
    }
}

#define ________SAI_ACL_L4_MAPPING_FUN
void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l4_layer_src_port(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ipv4_src_port.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->aclv6_key.sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_L4_SRC_PORT);
#ifdef SAI_DEFINE_STANDAND_ACL_CTC
                if (field_list->ipv4_src_port.data.u16 == field_list->ipv4_src_port.mask.u16)
                {
                    res->aclv6_key.l4_src_port_use_mask = 1;
                    res->aclv6_key.l4_src_port_0 = field_list->ipv4_src_port.data.u16;
                    res->aclv6_key.l4_src_port_1 = 0xffff;
                }
                else
                {
                    res->aclv6_key.l4_src_port_use_mask = 0;
                    res->aclv6_key.l4_src_port_0 = field_list->ipv4_src_port.data.u16;
                    res->aclv6_key.l4_src_port_1 = field_list->ipv4_src_port.mask.u16;
                }
#else 
                res->aclv6_key.l4_src_port_use_mask = 1;
                res->aclv6_key.l4_src_port_0 = field_list->ipv4_src_port.data.u16;
                res->aclv6_key.l4_src_port_1 = field_list->ipv4_src_port.mask.u16;
                SAI_ACL_WILDCARD_MASK(res->aclv6_key.l4_src_port_1);
#endif
            }
            else
            {
                CTC_SET_FLAG(res->acl_key.sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_SRC_PORT);
#ifdef SAI_DEFINE_STANDAND_ACL_CTC
                if (field_list->ipv4_src_port.data.u16 == field_list->ipv4_src_port.mask.u16)
                {
                    res->acl_key.l4_src_port_use_mask = 1;
                    res->acl_key.l4_src_port_0 = field_list->ipv4_src_port.data.u16;
                    res->acl_key.l4_src_port_1 = 0xffff;
                }
                else
                {
                    res->acl_key.l4_src_port_use_mask = 0;
                    res->acl_key.l4_src_port_0 = field_list->ipv4_src_port.data.u16;
                    res->acl_key.l4_src_port_1 = field_list->ipv4_src_port.mask.u16;
                }
#else 
                res->acl_key.l4_src_port_use_mask = 1;
                res->acl_key.l4_src_port_0 = field_list->ipv4_src_port.data.u16;
                res->acl_key.l4_src_port_1 = field_list->ipv4_src_port.mask.u16;
                SAI_ACL_WILDCARD_MASK(res->acl_key.l4_src_port_1);
#endif
            }
        }
        else 
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->scl_key.sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_L4_SRC_PORT);
                res->scl_key.l4_src_port = field_list->ipv4_src_port.data.u16;
                res->scl_key.l4_src_port = field_list->ipv4_src_port.mask.u16;
            }
            else
            {
                CTC_SET_FLAG(res->scl_key.sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_SRC_PORT);
                res->scl_key.l4_src_port = field_list->ipv4_src_port.data.u16;
                res->scl_key.l4_src_port = field_list->ipv4_src_port.mask.u16;
                /* src_port eq data == mask */            
            }
        }
    }
}

void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l4_layer_dst_port(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ipv4_dst_port.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->aclv6_key.sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_L4_DST_PORT);
#ifdef SAI_DEFINE_STANDAND_ACL_CTC
                if (field_list->ipv4_dst_port.data.u16 == field_list->ipv4_dst_port.mask.u16)
                {
                    res->aclv6_key.l4_dst_port_use_mask = 1;
                    res->aclv6_key.l4_dst_port_0 = field_list->ipv4_dst_port.data.u16;
                    res->aclv6_key.l4_dst_port_1 = 0xffff;
                }
                else
                {
                    res->aclv6_key.l4_dst_port_use_mask = 0;
                    res->aclv6_key.l4_dst_port_0 = field_list->ipv4_dst_port.data.u16;
                    res->aclv6_key.l4_dst_port_1 = field_list->ipv4_dst_port.mask.u16;
                }
#else 
                res->aclv6_key.l4_dst_port_use_mask = 1;
                res->aclv6_key.l4_dst_port_0 = field_list->ipv4_dst_port.data.u16;
                res->aclv6_key.l4_dst_port_1 = field_list->ipv4_dst_port.mask.u16;
                SAI_ACL_WILDCARD_MASK(res->aclv6_key.l4_dst_port_1);
#endif
            }
            else
            {
                CTC_SET_FLAG(res->acl_key.sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_DST_PORT);
#ifdef SAI_DEFINE_STANDAND_ACL_CTC
                if (field_list->ipv4_dst_port.data.u16 == field_list->ipv4_dst_port.mask.u16)
                {
                    res->acl_key.l4_dst_port_use_mask = 1;
                    res->acl_key.l4_dst_port_0 = field_list->ipv4_dst_port.data.u16;
                    res->acl_key.l4_dst_port_1 = 0xffff;
                }
                else
                {
                    res->acl_key.l4_dst_port_use_mask = 0;
                    res->acl_key.l4_dst_port_0 = field_list->ipv4_dst_port.data.u16;
                    res->acl_key.l4_dst_port_1 = field_list->ipv4_dst_port.mask.u16;
                }
#else 
                res->acl_key.l4_dst_port_use_mask = 1;
                res->acl_key.l4_dst_port_0 = field_list->ipv4_dst_port.data.u16;
                res->acl_key.l4_dst_port_1 = field_list->ipv4_dst_port.mask.u16;
                SAI_ACL_WILDCARD_MASK(res->acl_key.l4_dst_port_1);
#endif
            }
        }
        else 
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->sclv6_key.sub_flag, CTC_SCL_TCAM_IPV6_KEY_SUB_FLAG_L4_DST_PORT);
                res->sclv6_key.l4_dst_port = field_list->ipv4_dst_port.data.u16;
                res->sclv6_key.l4_dst_port = field_list->ipv4_dst_port.mask.u16;
            }
            else
            {
                CTC_SET_FLAG(res->scl_key.sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_L4_DST_PORT);
                res->scl_key.l4_dst_port = field_list->ipv4_dst_port.data.u16;
                res->scl_key.l4_dst_port = field_list->ipv4_dst_port.mask.u16;
                /* dst_port eq data == mask */
            }
        }
    }
}

void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l4_vxlan_vni(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    /* only outer field */
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ipv4_vxlan_vni.enable)
    {
        /*added by yejl for bug54032,54049, 2019-10-22*/
        if (field_list->ipv4_vxlan_vni_dst_port.enable)
        {
            (void)ctc_global_ctl_set(CTC_GLOBAL_VXLAN_UDP_DEST_PORT, &(field_list->ipv4_vxlan_vni_dst_port.data.u32));
        }
        /*ended*/
        p_sdb_entry->tap.l4_vxlan_vni       = field_list->ipv4_vxlan_vni.data.u32;
        p_sdb_entry->tap.l4_vxlan_vni_mask  = field_list->ipv4_vxlan_vni.mask.u32;
        p_sdb_entry->tap.l4_vxlan_vni_en    = TRUE;
    }
}

void 
__mapping_sai_acl_entry_to_ctc_acl_entry_l4_tcp_flags(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->ipv4_tcp_flag.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->aclv6_key.sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_TCP_FLAGS);
                res->aclv6_key.tcp_flags_match_any   = field_list->ipv4_tcp_flag.data.u8;
                res->aclv6_key.tcp_flags             = field_list->ipv4_tcp_flag.mask.u8;
            }
            else
            {
                CTC_SET_FLAG(res->acl_key.sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_TCP_FLAGS);
                res->acl_key.tcp_flags_match_any   = field_list->ipv4_tcp_flag.data.u8;
                res->acl_key.tcp_flags             = field_list->ipv4_tcp_flag.mask.u8;
            }
        }
        else 
        {
            /* SDK not support */
        }
    }
}

#define ________SAI_ACL_ARP_MAPPING_FUN
void
__mapping_sai_acl_entry_to_ctc_acl_entry_arp_layer_op_type(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->arp_op_type.enable)
    {
        if (FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
#ifdef GOLDENGATE
            CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
#endif
            res->acl_key.l3_type      = CTC_PARSER_L3_TYPE_ARP;
            res->acl_key.l3_type_mask = 0xff;
#ifdef GREATBELT
            CTC_SET_FLAG(res->acl_key.flag, CTC_ACL_IPV4_KEY_FLAG_ARP_PACKET);
            res->acl_key.arp_packet = 1;
#endif            

            CTC_SET_FLAG(res->acl_key.sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_OP_CODE);
            res->acl_key.arp_op_code = field_list->arp_op_type.data.u16;
            res->acl_key.arp_op_code_mask = field_list->arp_op_type.mask.u16;
            SAI_ACL_WILDCARD_MASK(res->acl_key.arp_op_code_mask);         
        }
    }
}

void
__mapping_sai_acl_entry_to_ctc_acl_entry_arp_layer_sender_ip(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->arp_sender_ip.enable)
    {
        if (FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {        
            CTC_SET_FLAG(res->acl_key.sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_SENDER_IP);
            res->acl_key.sender_ip = field_list->arp_sender_ip.data.ip4;
            res->acl_key.sender_ip_mask = field_list->arp_sender_ip.mask.ip4;
            SAI_ACL_WILDCARD_MASK(res->acl_key.sender_ip_mask);         
        }
    }
}

void
__mapping_sai_acl_entry_to_ctc_acl_entry_arp_layer_target_ip(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();
    if (field_list->arp_target_ip.enable)
    {
        if (FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {        
            CTC_SET_FLAG(res->acl_key.sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_TARGET_IP);
            res->acl_key.target_ip = field_list->arp_target_ip.data.ip4;
            res->acl_key.target_ip_mask = field_list->arp_target_ip.mask.ip4;
            SAI_ACL_WILDCARD_MASK(res->acl_key.target_ip_mask);         
        }
    }
}

#define ________SAL_ACL_UDF_MAPPING_FUN
void 
__mapping_sai_acl_entry_to_ctc_acl_entry_udf_type(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    CTC_SAI_DEBUG_FUNC();

    if (field_list->udf_type.enable)
    {
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
            }
            else
            {}
            res->acl.key.u.ipv4_key.udf_type = 
                (field_list->udf_type.data.u8 - 1) ? 
                    CTC_PARSER_UDF_TYPE_L4_UDF: CTC_PARSER_UDF_TYPE_L3_UDF;
            
        }
        else 
        {
            /* scl not support udf */
        }
    }
}

void 
__mapping_sai_acl_entry_to_ctc_acl_entry_udf_match(
            ctc_sai_acl_field_list_t *field_list,
            stbl_acl_entry_t        *p_sdb_entry,
            fields_mapping_res_t    *res)
{
    sai_uint8_t i = 0;
    sai_uint32_t udf_value;
    sai_uint32_t udf_mask;
    sai_object_id_t *udf_match = NULL;
    
    CTC_SAI_DEBUG_FUNC();

    if (field_list->udf_match_data.enable)
    {
        udf_match = field_list->udf_match_data.data.objlist.list;
        if (NULL == udf_match)
        {
            return;
        }

        udf_value = udf_match[0] & 0xFFFFFFFF;
        udf_mask  = (~udf_match[1]) & 0xFFFFFFFF;
        if(FIELD_ACL_ENTRY_TYPE == res->mapping_type)
        {
            if (field_list->is_ipv6)
            {
                CTC_SET_FLAG(res->acl.key.u.ipv6_key.flag, CTC_ACL_IPV6_KEY_FLAG_UDF);
                for (i = 0; i < CTC_ACL_UDF_BYTE_NUM; i++)
                {
                    res->acl.key.u.ipv6_key.udf[i] = (udf_value >> (8 * i)) & 0xff;
                    res->acl.key.u.ipv6_key.udf_mask[i] = (udf_mask >> (8 * i)) & 0xff;
                }
            }
            else
            {
                CTC_SET_FLAG(res->acl.key.u.ipv4_key.flag, CTC_ACL_IPV4_KEY_FLAG_UDF);
                for (i = 0; i < CTC_ACL_UDF_BYTE_NUM; i++)
                {
                    res->acl.key.u.ipv4_key.udf[i] = (udf_value >> (8 * i)) & 0xff;
                    res->acl.key.u.ipv4_key.udf_mask[i] = (udf_mask >> (8 * i)) & 0xff;
                }
            }
        }
        else 
        {
            CTC_SET_FLAG(res->scl_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_UDF);

            for (i = 0; i < CTC_ACL_UDF_BYTE_NUM; i++)
            {
                res->scl_key.udf[i] = (udf_value >> (8 * i)) & 0xff;
                res->scl_key.udf_mask[i] = (udf_mask >> (8 * i)) & 0xff;
            }
        }
    }
}

#define ________SAI_ACL_ACTION_MAPPING_FUN
sai_attribute_t*
ctc_sai_acl_get_action_attribute(stbl_acl_entry_t *p_sdb_entry, sai_attr_id_t attr_id, uint32 check_valid)
{
    sai_attribute_t *p_attr = NULL;

    if (SAI_ACL_ENTRY_ATTR_ACTION_END < attr_id)
    {
        return NULL;
    }

    p_attr = &(p_sdb_entry->action_attr.array[attr_id-SAI_ACL_ENTRY_ATTR_ACTION_START]);
    if (!check_valid)
    {
        return p_attr;
    }
    else
    {
        if (p_attr->id == attr_id)
        {
            return p_attr;
        }
        return NULL;
    }
}

static sai_status_t
ctc_sai_acl_action_map_db_ctc(stbl_acl_entry_t *p_sdb_entry, ctc_acl_action_t *action)
{
    sai_attribute_t   *attr_value = NULL;
    uint8_t ctc_sai_set_ctc_color[3] = {CTC_QOS_COLOR_GREEN, CTC_QOS_COLOR_YELLOW, CTC_QOS_COLOR_RED};
    ctc_sai_mirror_entry_t* p_session = NULL;
    ctc_mirror_dest_t mirror_dest;
    int32 ret = SAI_STATUS_SUCCESS;
    uint8 tid = CTC_MAX_LINKAGG_GROUP_NUM;
    uint32 redirect_gport = 0;
    
    attr_value = ctc_sai_acl_get_action_attribute(p_sdb_entry, SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT, TRUE);
    if (attr_value)
    {
        if (SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(attr_value->value.aclaction.parameter.oid))
        {
            redirect_gport = CTC_SAI_OBJECT_INDEX_GET(attr_value->value.aclaction.parameter.oid);
            ctc_nh_get_l2uc(redirect_gport, CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS, &action->nh_id);
            action->flag |= CTC_ACL_ACTION_FLAG_REDIRECT;
        }
        else if (SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(attr_value->value.aclaction.parameter.oid))
        {
            ctc_sai_port_objectid_to_gport(attr_value->value.aclaction.parameter.oid, &redirect_gport);
            ctc_nh_get_l2uc(redirect_gport, CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS, &action->nh_id);
            action->flag |= CTC_ACL_ACTION_FLAG_REDIRECT;
        }
        else if (SAI_OBJECT_TYPE_NEXT_HOP == CTC_SAI_OBJECT_TYPE_GET(attr_value->value.aclaction.parameter.oid))
        {
            action->flag |= CTC_ACL_ACTION_FLAG_REDIRECT;
            action->nh_id = CTC_SAI_OBJECT_INDEX_GET(attr_value->value.aclaction.parameter.oid);
        }
    }
    
    attr_value = ctc_sai_acl_get_action_attribute(p_sdb_entry, SAI_ACL_ENTRY_ATTR_PACKET_ACTION, TRUE);
    if(attr_value)
    {
        if (attr_value->value.aclaction.enable)
        {
            switch(attr_value->value.aclaction.parameter.s32)
            {
            case SAI_PACKET_ACTION_DROP:
                CTC_SET_FLAG(action->flag, CTC_ACL_ACTION_FLAG_DISCARD);
                break;

            case SAI_PACKET_ACTION_FORWARD:
                CTC_UNSET_FLAG(action->flag, CTC_ACL_ACTION_FLAG_DISCARD);
                break;

            case SAI_PACKET_ACTION_TRAP:
                CTC_SET_FLAG(action->flag, CTC_ACL_ACTION_FLAG_REDIRECT);
                action->nh_id = CTC_NH_RESERVED_NHID_FOR_TOCPU;
                break;

            case SAI_PACKET_ACTION_LOG:
                CTC_SET_FLAG(action->flag, CTC_ACL_ACTION_FLAG_COPY_TO_CPU);
                break;

            default:
                break;            
            }
        }
    } 
    
    attr_value = ctc_sai_acl_get_action_attribute(p_sdb_entry, SAI_ACL_ENTRY_ATTR_ACTION_FLOOD, TRUE);
    if (attr_value)
    {
        /*TODO*/
    }
    
    attr_value = ctc_sai_acl_get_action_attribute(p_sdb_entry, SAI_ACL_ENTRY_ATTR_ACTION_COUNTER, TRUE);
    if (attr_value)
    {
        if (attr_value->value.aclaction.enable)
        {
            action->flag |= CTC_ACL_ACTION_FLAG_STATS;
            action->stats_id = CTC_SAI_OBJECT_INDEX_GET(attr_value->value.aclaction.parameter.oid);
        }
    }
    
    attr_value = ctc_sai_acl_get_action_attribute(p_sdb_entry, SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS, TRUE);
    if (attr_value)
    {
        if (attr_value->value.aclaction.enable)
        {
            p_session = ctc_sai_mirror_entry_get(attr_value->value.aclaction.parameter.oid);
            if (p_session && (p_session->sid != 0))
            {
                sal_memset(&mirror_dest, 0, sizeof(mirror_dest));
                mirror_dest.session_id = CTC_SAI_OBJECT_INDEX_GET(p_session->sid);
                mirror_dest.dir = CTC_INGRESS;
                mirror_dest.type = CTC_MIRROR_ACLLOG_SESSION;
                if (p_session->monitor_type == SAI_MIRROR_TYPE_LOCAL)
                {
                    if (p_session->is_multi_dest_mirror)
                    {
                        mirror_dest.rspan.nh_id = CTC_SAI_OBJECT_INDEX_GET(p_session->nhid);
                        mirror_dest.is_rspan = TRUE;
                        mirror_dest.vlan_valid = FALSE; 
                    }
                    else
                    {
                        /* port */
                        if (SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(p_session->monitor_port[0]))
                        {
                            mirror_dest.dest_gport = CTC_SAI_OBJECT_INDEX_GET(p_session->monitor_port[0]);
                        }
                        /* agg */
                        else if (SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(p_session->monitor_port[0]))
                        {
                            tid = CTC_SAI_OBJECT_INDEX_GET(p_session->monitor_port[0]);
                            mirror_dest.dest_gport = CTC_MAP_TID_TO_GPORT(tid);
                        }
                        else if (SAI_OBJECT_TYPE_COPP == CTC_SAI_OBJECT_TYPE_GET(p_session->monitor_port[0]))
                        {
                            mirror_dest.dest_gport = CTC_LPORT_CPU;
                        }
                        else
                        {
                            return SAI_STATUS_NOT_SUPPORTED;
                        }
                    }
                }
                else if (p_session->monitor_type == SAI_MIRROR_TYPE_REMOTE)
                {            
                    mirror_dest.dest_gport = CTC_SAI_OBJECT_INDEX_GET(p_session->monitor_port[0]);
                    mirror_dest.rspan.nh_id = CTC_SAI_OBJECT_INDEX_GET(p_session->nhid);
                    mirror_dest.is_rspan = TRUE;
                    mirror_dest.vlan_valid = FALSE;
                }
                else
                {
                    return SAI_STATUS_NOT_SUPPORTED;
                }
                ret = ctc_mirror_add_session(&mirror_dest);
                if(ret)
                {
                    return ret;
                }
                action->flag |= CTC_ACL_ACTION_FLAG_RANDOM_LOG;
                action->log_percent = CTC_LOG_PERCENT_POWER_NEGATIVE_0;
                action->log_session_id = CTC_SAI_OBJECT_INDEX_GET(attr_value->value.aclaction.parameter.oid);
            }
        }
    }
    
    attr_value = ctc_sai_acl_get_action_attribute(p_sdb_entry, SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS, TRUE);
    if (attr_value)
    {
        if (attr_value->value.aclaction.enable)
        {
            p_session = ctc_sai_mirror_entry_get(attr_value->value.aclaction.parameter.oid);
            if (p_session && (p_session->sid != 0))
            {
                sal_memset(&mirror_dest, 0, sizeof(mirror_dest));
                mirror_dest.session_id = CTC_SAI_OBJECT_INDEX_GET(p_session->sid);
                mirror_dest.dir = CTC_EGRESS;
                mirror_dest.type = CTC_MIRROR_ACLLOG_SESSION;
                if (p_session->monitor_type == SAI_MIRROR_TYPE_LOCAL)
                {
                    if (p_session->is_multi_dest_mirror)
                    {
                        mirror_dest.rspan.nh_id = CTC_SAI_OBJECT_INDEX_GET(p_session->nhid);
                        mirror_dest.is_rspan = TRUE;
                        mirror_dest.vlan_valid = FALSE; 
                    }
                    else
                    {
                        /* port */
                        if (SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(p_session->monitor_port[0]))
                        {
                            mirror_dest.dest_gport = CTC_SAI_OBJECT_INDEX_GET(p_session->monitor_port[0]);
                        }
                        /* agg */
                        else if (SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(p_session->monitor_port[0]))
                        {
                            tid = CTC_SAI_OBJECT_INDEX_GET(p_session->monitor_port[0]);
                            mirror_dest.dest_gport = CTC_MAP_TID_TO_GPORT(tid);
                        }
                        else if (SAI_OBJECT_TYPE_COPP == CTC_SAI_OBJECT_TYPE_GET(p_session->monitor_port[0]))
                        {
                            mirror_dest.dest_gport = CTC_LPORT_CPU;
                        }
                        else
                        {
                            return SAI_STATUS_NOT_SUPPORTED;
                        }
                    }
                }
                else if (p_session->monitor_type == SAI_MIRROR_TYPE_REMOTE)
                {            
                    mirror_dest.dest_gport = CTC_SAI_OBJECT_INDEX_GET(p_session->monitor_port[0]);
                    mirror_dest.rspan.nh_id = CTC_SAI_OBJECT_INDEX_GET(p_session->nhid);
                    mirror_dest.is_rspan = TRUE;
                    mirror_dest.vlan_valid = FALSE;
                }
                else
                {
                    return SAI_STATUS_NOT_SUPPORTED;
                }
                ret = ctc_mirror_add_session(&mirror_dest);
                if(ret)
                {
                    return ret;
                }
                action->flag |= CTC_ACL_ACTION_FLAG_RANDOM_LOG;
                action->log_percent = CTC_LOG_PERCENT_POWER_NEGATIVE_0;
                action->log_session_id = CTC_SAI_OBJECT_INDEX_GET(attr_value->value.aclaction.parameter.oid);
            }
        }
    }
    
    attr_value = ctc_sai_acl_get_action_attribute(p_sdb_entry, SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER, TRUE);
    if(attr_value)
    {
        if (attr_value->value.aclaction.enable)
        {
            action->flag |= CTC_ACL_ACTION_FLAG_MICRO_FLOW_POLICER;
            action->micro_policer_id = CTC_SAI_OBJECT_INDEX_GET(attr_value->value.aclaction.parameter.oid);
        }
    }
    
    attr_value = ctc_sai_acl_get_action_attribute(p_sdb_entry, SAI_ACL_ENTRY_ATTR_ACTION_DECREMENT_TTL, TRUE);
    if(attr_value)
    {
        /*TODO*/    
    }

    attr_value = ctc_sai_acl_get_action_attribute(p_sdb_entry, SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID, TRUE);
    if (attr_value)
    {
        if(attr_value->value.aclaction.enable)
        {
            action->flag |= CTC_ACL_ACTION_FLAG_VLAN_EDIT;
#ifdef GREATBELT
            action->vlan_edit.ctag_op = CTC_ACL_VLAN_TAG_OP_REP;
#else
            action->vlan_edit.ctag_op = CTC_ACL_VLAN_TAG_OP_REP_OR_ADD;
#endif
            action->vlan_edit.cvid_sl = CTC_ACL_VLAN_TAG_SL_NEW;
            action->vlan_edit.cvid_new = attr_value->value.aclaction.parameter.u16;
        }
    }
    
    attr_value = ctc_sai_acl_get_action_attribute(p_sdb_entry, SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI, TRUE);
    if (attr_value)
    {
        if(attr_value->value.aclaction.enable)
        {
            action->flag |= CTC_ACL_ACTION_FLAG_VLAN_EDIT;
#ifdef GREATBELT
            action->vlan_edit.ctag_op = CTC_ACL_VLAN_TAG_OP_REP;
#else
            action->vlan_edit.ctag_op = CTC_ACL_VLAN_TAG_OP_REP_OR_ADD;
#endif
            action->vlan_edit.ccos_sl = CTC_ACL_VLAN_TAG_SL_NEW;
            action->vlan_edit.ccos_new= attr_value->value.aclaction.parameter.u8;
        }
    }
    
    attr_value = ctc_sai_acl_get_action_attribute(p_sdb_entry, SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID, TRUE);
    if (attr_value)
    {
        if(attr_value->value.aclaction.enable)
        {
            action->flag |= CTC_ACL_ACTION_FLAG_VLAN_EDIT;
#ifdef GREATBELT
            action->vlan_edit.stag_op = CTC_ACL_VLAN_TAG_OP_REP;
#else
            action->vlan_edit.stag_op = CTC_ACL_VLAN_TAG_OP_REP_OR_ADD;
#endif
            action->vlan_edit.svid_sl = CTC_ACL_VLAN_TAG_SL_NEW;
            action->vlan_edit.svid_new = attr_value->value.aclaction.parameter.u16;
        }
    }
    
    attr_value = ctc_sai_acl_get_action_attribute(p_sdb_entry, SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI, TRUE);
    if (attr_value)
    {
        if(attr_value->value.aclaction.enable)
        {
            action->flag |= CTC_ACL_ACTION_FLAG_VLAN_EDIT;
#ifdef GREATBELT
            action->vlan_edit.stag_op = CTC_ACL_VLAN_TAG_OP_REP;
#else
            action->vlan_edit.stag_op = CTC_ACL_VLAN_TAG_OP_REP_OR_ADD;
#endif
            action->vlan_edit.scos_sl = CTC_ACL_VLAN_TAG_SL_NEW;
            action->vlan_edit.scos_new= attr_value->value.aclaction.parameter.u8;
        }
    }
    
    attr_value = ctc_sai_acl_get_action_attribute(p_sdb_entry, SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP, TRUE);
    if (attr_value)
    {
        if(attr_value->value.aclaction.enable)
        {
            action->flag |= CTC_ACL_ACTION_FLAG_DSCP;
            action->dscp = attr_value->value.aclaction.parameter.u8;
        }
    }
    
    /* fix gb tc+color bug add by wangqj at 2016/8/11 */
    attr_value = ctc_sai_acl_get_action_attribute(p_sdb_entry, SAI_ACL_ENTRY_ATTR_ACTION_SET_TC_AND_COLOR, TRUE);
    if (attr_value)
    {
        if(attr_value->value.aclaction.enable)
        {
            action->flag |= CTC_ACL_ACTION_FLAG_PRIORITY_AND_COLOR;

            action->priority = (attr_value->value.aclaction.parameter.tcandcolor.tc) * 8;
            action->color = ctc_sai_set_ctc_color[attr_value->value.aclaction.parameter.tcandcolor.color];
        }
    }
    
    return SAI_STATUS_SUCCESS;
}


static sai_status_t 
_acl_action_mirror(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{  
    CTC_SAI_DEBUG_FUNC();

    if (attr_value->value.aclaction.enable)
    {
        acl_entry->action.flag |= CTC_ACL_ACTION_FLAG_RANDOM_LOG;
        acl_entry->action.log_percent = CTC_LOG_PERCENT_POWER_NEGATIVE_0;
        acl_entry->action.log_session_id = CTC_SAI_OBJECT_INDEX_GET(attr_value->value.aclaction.parameter.oid);
    }
    else
    {
        CTC_UNSET_FLAG(acl_entry->action.flag, CTC_ACL_ACTION_FLAG_RANDOM_LOG);
    }

    return SAI_STATUS_SUCCESS;
}


static sai_status_t 
_acl_action_packet_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    CTC_SAI_DEBUG_FUNC();
    if (attr_value->value.aclaction.enable)
    {
        switch(attr_value->value.aclaction.parameter.s32)
        {
        case SAI_PACKET_ACTION_DROP:
            CTC_SET_FLAG(acl_entry->action.flag, CTC_ACL_ACTION_FLAG_DISCARD);
            break;

        case SAI_PACKET_ACTION_LOG:
            CTC_SET_FLAG(acl_entry->action.flag, CTC_ACL_ACTION_FLAG_RANDOM_LOG);
            break;

        case SAI_PACKET_ACTION_FORWARD:
            CTC_UNSET_FLAG(acl_entry->action.flag, CTC_ACL_ACTION_FLAG_DISCARD);
            break;

        case SAI_PACKET_ACTION_TRAP:
            CTC_SET_FLAG(acl_entry->action.flag, CTC_ACL_ACTION_FLAG_REDIRECT);
            acl_entry->action.nh_id = CTC_NH_RESERVED_NHID_FOR_TOCPU;
            break;

        case SAI_PACKET_ACTION_COPY:
            CTC_SET_FLAG(acl_entry->action.flag, CTC_ACL_ACTION_FLAG_COPY_TO_CPU);
            break;

        default:
            break;            
        }
    }

    return SAI_STATUS_SUCCESS;
}




static sai_status_t 
_acl_action_counter(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    CTC_SAI_DEBUG_FUNC();
    if (attr_value->value.aclaction.enable)
    {
        acl_entry->action.flag |= CTC_ACL_ACTION_FLAG_STATS;
        acl_entry->action.stats_id = CTC_SAI_OBJECT_INDEX_GET(attr_value->value.aclaction.parameter.oid);
    }
    else
    {
        CTC_UNSET_FLAG(acl_entry->action.flag, CTC_ACL_ACTION_FLAG_STATS);
    }

    return SAI_STATUS_SUCCESS;
}


static sai_status_t 
_acl_action_settc(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    CTC_SAI_DEBUG_FUNC();
    if (attr_value->value.aclaction.enable)
    {
        acl_entry->action.flag |= CTC_ACL_ACTION_FLAG_PRIORITY;
        acl_entry->action.priority = (attr_value->value.aclaction.parameter.u32)*8 - 1;
    }
    else
    {
        CTC_UNSET_FLAG(acl_entry->action.flag, CTC_ACL_ACTION_FLAG_PRIORITY);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_setcolor(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    CTC_SAI_DEBUG_FUNC();
    if (attr_value->value.aclaction.enable)
    {
        acl_entry->action.flag |= CTC_ACL_ACTION_FLAG_COLOR;
        acl_entry->action.color = (attr_value->value.aclaction.parameter.u32)*8 - 1;
    }
    else
    {
        CTC_UNSET_FLAG(acl_entry->action.flag, CTC_ACL_ACTION_FLAG_COLOR);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_policer(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        acl_entry->action.flag |= CTC_ACL_ACTION_FLAG_MICRO_FLOW_POLICER;
        acl_entry->action.micro_policer_id = CTC_SAI_OBJECT_INDEX_GET(attr_value->value.aclaction.parameter.oid);
    }
    else
    {
        CTC_UNSET_FLAG(acl_entry->action.flag, CTC_ACL_ACTION_FLAG_MICRO_FLOW_POLICER);
    }
    return SAI_STATUS_SUCCESS;
}


static sai_status_t 
_acl_action_set_redirect_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
#ifdef TAPPRODUCT
    ctc_sai_tap_info_t* tap_info = NULL;
#endif
    if (attr_value->value.aclaction.enable)
    { 
#ifdef TAPPRODUCT    
        if(sai_object_type_query(attr_value->value.aclaction.parameter.oid) == SAI_OBJECT_TYPE_TAP_GROUP)
        {
            tap_info = ctc_sai_tap_get_by_oid(attr_value->value.aclaction.parameter.oid);
            if (!tap_info)
            {
                return SAI_STATUS_FAILURE;
            }
            p_sdb_entry->tap.ptap_info = tap_info;
            p_sdb_entry->tap.tap_group_id = attr_value->value.aclaction.parameter.oid;
        }
        else
        {
     
            acl_entry->action.flag |= CTC_ACL_ACTION_FLAG_REDIRECT;
            if(CTC_SAI_OBJECT_TYPE_GET(attr_value->value.aclaction.parameter.oid) == SAI_OBJECT_TYPE_NEXT_HOP)
            {
                acl_entry->action.nh_id = CTC_SAI_OBJECT_INDEX_GET(attr_value->value.aclaction.parameter.oid);
            }
        }
#else
        acl_entry->action.flag |= CTC_ACL_ACTION_FLAG_REDIRECT;
        if(CTC_SAI_OBJECT_TYPE_GET(attr_value->value.aclaction.parameter.oid) == SAI_OBJECT_TYPE_NEXT_HOP)
        {
            acl_entry->action.nh_id = CTC_SAI_OBJECT_INDEX_GET(attr_value->value.aclaction.parameter.oid);
        }
#endif
    }
    else
    {
        CTC_UNSET_FLAG(acl_entry->action.flag, CTC_ACL_ACTION_FLAG_REDIRECT);
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_redirect_truncation(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
#ifdef TAPPRODUCT
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.trunction_en = TRUE;
    }
    else
    {
        p_sdb_entry->tap.trunction_en = FALSE;
    }
#endif
    return SAI_STATUS_SUCCESS;
}


static sai_status_t 
_acl_action_set_redirect_vlan(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        acl_entry->action.flag |= CTC_ACL_ACTION_FLAG_VLAN_EDIT;
        acl_entry->action.vlan_edit.stag_op = CTC_ACL_VLAN_TAG_OP_ADD;
        acl_entry->action.vlan_edit.svid_sl = CTC_ACL_VLAN_TAG_SL_NEW;
        acl_entry->action.vlan_edit.svid_new = attr_value->value.aclaction.parameter.u16;
    }
    else
    {
        CTC_UNSET_FLAG(acl_entry->action.flag, CTC_ACL_ACTION_FLAG_MICRO_FLOW_POLICER);
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_redirect_untag(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        acl_entry->action.flag |= CTC_ACL_ACTION_FLAG_VLAN_EDIT;
        if(SAI_TAP_GROUP_MEMBER_STAGE_UNTAG_MODE_DOUBLE_VLAN == attr_value->value.aclaction.parameter.u32)
        {
            acl_entry->action.vlan_edit.stag_op = CTC_ACL_VLAN_TAG_OP_DEL;
            acl_entry->action.vlan_edit.svid_sl = CTC_ACL_VLAN_TAG_SL_NONE;
            acl_entry->action.vlan_edit.ctag_op = CTC_ACL_VLAN_TAG_OP_DEL;
            acl_entry->action.vlan_edit.cvid_sl = CTC_ACL_VLAN_TAG_SL_NONE;
        }
        else if(SAI_TAP_GROUP_MEMBER_STAGE_UNTAG_MODE_SVLAN == attr_value->value.aclaction.parameter.u32)
        {
            acl_entry->action.vlan_edit.stag_op = CTC_ACL_VLAN_TAG_OP_DEL;
            acl_entry->action.vlan_edit.svid_sl = CTC_ACL_VLAN_TAG_SL_NONE;
            acl_entry->action.vlan_edit.ctag_op = CTC_ACL_VLAN_TAG_OP_NONE;
            acl_entry->action.vlan_edit.cvid_sl = CTC_ACL_VLAN_TAG_SL_NONE;
        }
        else if(SAI_TAP_GROUP_MEMBER_STAGE_UNTAG_MODE_CVLAN == attr_value->value.aclaction.parameter.u32)
        {
            acl_entry->action.vlan_edit.stag_op = CTC_ACL_VLAN_TAG_OP_NONE;
            acl_entry->action.vlan_edit.svid_sl = CTC_ACL_VLAN_TAG_SL_NONE;
            acl_entry->action.vlan_edit.ctag_op = CTC_ACL_VLAN_TAG_OP_DEL;
            acl_entry->action.vlan_edit.cvid_sl = CTC_ACL_VLAN_TAG_SL_NONE;
        }
    }
    else
    {
        CTC_UNSET_FLAG(acl_entry->action.flag, CTC_ACL_ACTION_FLAG_MICRO_FLOW_POLICER);
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_src_mac_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.macsa_en = TRUE;
        sal_memcpy(p_sdb_entry->tap.macsa, attr_value->value.aclaction.parameter.mac, sizeof(sai_mac_t));
    }
    else
    {
        p_sdb_entry->tap.macsa_en = FALSE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_dst_mac_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.macda_en = TRUE;
        sal_memcpy(p_sdb_entry->tap.macda, attr_value->value.aclaction.parameter.mac, sizeof(sai_mac_t));
    }
    else
    {
        p_sdb_entry->tap.macda_en = FALSE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_src_ip_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.ipsa_en = TRUE;
        sal_memcpy(&p_sdb_entry->tap.ipsa, &attr_value->value.aclaction.parameter.ip4, sizeof(sai_ip4_t));
    }
    else
    {
        p_sdb_entry->tap.ipsa_en = FALSE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_dst_ip_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.ipda_en = TRUE;
        sal_memcpy(&p_sdb_entry->tap.ipda, &attr_value->value.aclaction.parameter.ip4, sizeof(sai_ip4_t));
    }
    else
    {
        p_sdb_entry->tap.ipda_en = FALSE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_src_ipv6_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.ipv6sa_en = TRUE;
        sal_memcpy(&p_sdb_entry->tap.ipv6sa, &attr_value->value.aclaction.parameter.ip6, sizeof(sai_ip6_t));
    }
    else
    {
        p_sdb_entry->tap.ipv6sa_en = FALSE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_dst_ipv6_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.ipv6da_en = TRUE;
        sal_memcpy(&p_sdb_entry->tap.ipv6da, &attr_value->value.aclaction.parameter.ip6, sizeof(sai_ip6_t));
    }
    else
    {
        p_sdb_entry->tap.ipv6da_en = FALSE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_strip_header_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.strip_header = attr_value->value.aclaction.parameter.u32;
    }
    else
    {
        p_sdb_entry->tap.strip_header = 0;
    }
    return SAI_STATUS_SUCCESS;
}

/*added by yejl for bug 53475, 2019-09-10*/
static sai_status_t 
_acl_action_set_add_l2gre_header_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.add_l2gre_header = TRUE;
    }
    else
    {
        p_sdb_entry->tap.add_l2gre_header = FALSE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_add_l2gre_src_ip_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        sal_memcpy(&p_sdb_entry->tap.l2gre_ipsa, &attr_value->value.aclaction.parameter.ip4, sizeof(sai_ip4_t));
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_add_l2gre_dst_ip_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        sal_memcpy(&p_sdb_entry->tap.l2gre_ipda, &attr_value->value.aclaction.parameter.ip4, sizeof(sai_ip4_t));
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_add_l2gre_dst_mac_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        sal_memcpy(p_sdb_entry->tap.l2gre_macda, attr_value->value.aclaction.parameter.mac, sizeof(sai_mac_t));
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_add_l2gre_key_num_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.l2gre_key_num = attr_value->value.aclaction.parameter.u32;
    }
    return SAI_STATUS_SUCCESS;
}   

static sai_status_t 
_acl_action_set_add_l2gre_key_len_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.l2gre_key_len = attr_value->value.aclaction.parameter.u32;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_add_l3gre_header_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.add_l3gre_header = TRUE;
    }
    else
    {
        p_sdb_entry->tap.add_l3gre_header = FALSE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_add_l3gre_src_ip_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        sal_memcpy(&p_sdb_entry->tap.l3gre_ipsa, &attr_value->value.aclaction.parameter.ip4, sizeof(sai_ip4_t));
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_add_l3gre_dst_ip_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        sal_memcpy(&p_sdb_entry->tap.l3gre_ipda, &attr_value->value.aclaction.parameter.ip4, sizeof(sai_ip4_t));
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_add_l3gre_dst_mac_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        sal_memcpy(p_sdb_entry->tap.l3gre_macda, attr_value->value.aclaction.parameter.mac, sizeof(sai_mac_t));
    }
    return SAI_STATUS_SUCCESS;
}
/*ended by yejl*/

/*added by guoxd for vxlan*/
static sai_status_t 
_acl_action_set_add_vxlan_header_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.vxlan_add_header = TRUE;
    }
    else
    {
        p_sdb_entry->tap.vxlan_add_header = FALSE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_add_vxlan_dst_mac_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        sal_memcpy(p_sdb_entry->tap.vxlan_macda, attr_value->value.aclaction.parameter.mac, sizeof(sai_mac_t));
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_add_vxlan_src_ip_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        sal_memcpy(&p_sdb_entry->tap.vxlan_ipsa, &attr_value->value.aclaction.parameter.ip4, sizeof(sai_ip4_t));
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_add_vxlan_dest_ip_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        sal_memcpy(&p_sdb_entry->tap.vxlan_ipda, &attr_value->value.aclaction.parameter.ip4, sizeof(sai_ip4_t));
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_add_vxlan_src_port_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.vxlan_portsrc = attr_value->value.aclaction.parameter.u32;
    }
    return SAI_STATUS_SUCCESS;
}   

static sai_status_t 
_acl_action_set_add_vxlan_dest_port_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.vxlan_portdst = attr_value->value.aclaction.parameter.u32;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t 
_acl_action_set_add_vxlan_vni_action(ctc_acl_entry_t* acl_entry, const sai_attribute_t *attr_value, stbl_acl_entry_t *p_sdb_entry)
{
    if (attr_value->value.aclaction.enable)
    {
        p_sdb_entry->tap.vxlan_vni = attr_value->value.aclaction.parameter.u32;
    }
    return SAI_STATUS_SUCCESS;
}

/*ended by guoxd*/

