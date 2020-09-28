#include <sal.h>
#include "ctc_api.h"
#include "sal_error.h"
#include <sai.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_mirror.h>
#include <ctc_sai_port.h>
#include <ctc_sai_stp.h>
#include <ctc_sai_lag.h>
#include <ctc_sai_routerintf.h>
#include <ctc_sai_nexthop.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_router.h>

static ctc_sai_check_u32_range_t g_range_type_limit =
{
    .min = SAI_MIRROR_TYPE_LOCAL,
    .max = SAI_MIRROR_TYPE_ENHANCED_REMOTE,
};

static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_MIRROR_SESSION_ATTR_TYPE,
        .type   = SAI_ATTR_FALG_CREATE  | SAI_ATTR_FALG_READ,
        .check_fn = {
            .func_fn = ctc_sai_check_i32_range_fn,
            .func_parameter = &g_range_type_limit,
        }
    },
    {
        .id     = SAI_MIRROR_SESSION_ATTR_MONITOR_PORT,
        .type   = SAI_ATTR_FALG_CREATE | SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,   
    },
    {
        .id     = SAI_MIRROR_SESSION_ATTR_MULTI_DEST_MEMBER_ADD,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,   
    },
    {
        .id     = SAI_MIRROR_SESSION_ATTR_TC,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_MIRROR_SESSION_ATTR_VLAN_TPID,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_MIRROR_SESSION_ATTR_VLAN_ID,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_MIRROR_SESSION_ATTR_VLAN_PRI,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_MIRROR_SESSION_ATTR_ENCAP_TYPE,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_MIRROR_SESSION_ATTR_TOS,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_MIRROR_SESSION_ATTR_TTL,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     =  SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     =   SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     =   SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};

struct ctc_sai_mirror_info_s g_ctc_sai_mirror_info;


#define ________SAI_SAI_INNER_API_FUNC
sai_status_t
ctc_sai_mirror_db_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    g_ctc_sai_mirror_info.max_count = CTC_SAI_MIRROR_SESSION_MAX;         /*  mirror sesson id*/

    if (0 != ctclib_opb_create(&p_sai_glb->mirror_id_opb, 0, g_ctc_sai_mirror_info.max_count, "mirror_id"))
    {
        return SAI_STATUS_NO_MEMORY;
    }
    
    return SAI_STATUS_SUCCESS;
}

ctc_sai_mirror_entry_t*
ctc_sai_mirror_entry_get(sai_object_id_t session_oid)
{
    uint32 i = 0;

    for (i = 0; i < CTC_SAI_MIRROR_SESSION_MAX; i++)
    {
        if (session_oid == g_ctc_sai_mirror_info.session[i].sid)
        {
            return &g_ctc_sai_mirror_info.session[i];
        }
    }   
    return NULL;
}

sai_status_t
ctc_sai_mirror_alloc_offset(uint32_t *popf_index)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_status_t    ret         = SAI_STATUS_SUCCESS;
    uint32          opf_index   = 0;

    CTC_SAI_ERROR_GOTO_MAPSDKERR2SAI(
        ctclib_opb_alloc_offset(&p_sai_glb->mirror_id_opb, &opf_index),ret,out);
    *popf_index = opf_index;
    
out:
    return ret;
}

sai_status_t
ctc_sai_mirror_free_offset(uint32_t opf_index)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_status_t    ret         = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO_MAPSDKERR2SAI(
        ctclib_opb_free_offset(&p_sai_glb->mirror_id_opb, opf_index),ret,out);
out:
    return ret;
}

sai_object_id_t
ctc_sai_mirror_session_get_monitor_port(uint32 sessionid)
{
    int32 i = 0;
    sai_object_id_t port_oid = 0;
    for (i = 0; i < CTC_SAI_MIRROR_MULTI_DEST_PORT_NUM; i++)
    {
        if (g_ctc_sai_mirror_info.session[sessionid].monitor_port[i])
        {
            port_oid = g_ctc_sai_mirror_info.session[sessionid].monitor_port[i];
            return port_oid;
        }
    }
    return SAI_STATUS_SUCCESS;
}

ctc_sai_mirror_entry_t*
ctc_sai_mirror_session_id_entry_get(uint32 sessionid)
{
    return &g_ctc_sai_mirror_info.session[sessionid];
}

sai_status_t
ctc_sai_mirror_create_mirror_session( 
        sai_object_id_t *session_oid,
        uint32_t attr_count,
        const sai_attribute_t *attr_list)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    uint32_t          opf_index = 0;;
    uint32_t          sessionid = 0;
    uint32_t          mirror_type = 0;
    sai_status_t      ret = SAI_STATUS_SUCCESS;
    uint32_t          attr_idx = 0; 
    sai_attribute_t*  attr = NULL;
    ctc_mirror_dest_t st_dest;
    uint32            is_port = FALSE;
    uint32            is_cpu_port = FALSE;
    uint32            is_multi_dest_mirror = FALSE;
    uint32            dest_gport = 0;
    uint32            nexthop_id = 0;
    sai_vlan_id_t     vlan_id = 0;
#ifdef USW
    uint16             cnt = 0;
#else
    uint8             cnt = 0;
#endif
    uint8             i = 0;
    uint8             tid = CTC_MAX_LINKAGG_GROUP_NUM;
    uint32            p_gports[64];
    ctc_mcast_nh_param_group_t nh_mcast_group;
    ctc_rspan_nexthop_param_t rspan_nh_param;

    sal_memset(&st_dest, 0, sizeof(st_dest));

    if (0 != ctclib_opb_alloc_offset(&p_sai_glb->mirror_id_opb, &opf_index))
    {
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }
    
    *session_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_MIRROR, opf_index);
    sessionid = opf_index;
    
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = (sai_attribute_t*)attr_list + attr_idx;
        switch(attr->id)
        {
        case SAI_MIRROR_SESSION_ATTR_TYPE:
            mirror_type = attr->value.s32;
            g_ctc_sai_mirror_info.session[sessionid].monitor_type = mirror_type;                
            break;
        case SAI_MIRROR_SESSION_ATTR_MONITOR_PORT:
            if (SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(attr->value.objlist.list[0]))
            {
                dest_gport = CTC_SAI_OBJECT_INDEX_GET(attr->value.objlist.list[0]);
                g_ctc_sai_mirror_info.session[sessionid].monitor_port[0] = attr->value.objlist.list[0];
                g_ctc_sai_mirror_info.session[sessionid].monitor_port_count++;
                is_port = TRUE;
            }
            else if (SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(attr->value.objlist.list[0]))
            {
                tid = CTC_SAI_OBJECT_INDEX_GET(attr->value.objlist.list[0]);
                dest_gport = CTC_MAP_TID_TO_GPORT(tid);
                g_ctc_sai_mirror_info.session[sessionid].monitor_port[0] = attr->value.objlist.list[0];
                g_ctc_sai_mirror_info.session[sessionid].monitor_port_count++;
            }
            else if (SAI_OBJECT_TYPE_COPP == CTC_SAI_OBJECT_TYPE_GET(attr->value.objlist.list[0]))
            {
                dest_gport = CTC_LPORT_CPU;
                g_ctc_sai_mirror_info.session[sessionid].monitor_port[0] = attr->value.objlist.list[0];
                g_ctc_sai_mirror_info.session[sessionid].monitor_port_count++;
                is_cpu_port = TRUE;
            }
            else
            {
                return SAI_STATUS_NOT_SUPPORTED;
            }
            break;
            
        case SAI_MIRROR_SESSION_ATTR_MULTI_DEST_MEMBER_ADD:
            if (attr->value.booldata)
            {
                is_multi_dest_mirror = TRUE;
                g_ctc_sai_mirror_info.session[sessionid].is_multi_dest_mirror = TRUE;    
            }
            break;
        case SAI_MIRROR_SESSION_ATTR_TC:
            g_ctc_sai_mirror_info.session[sessionid].tc= attr->value.u8;                
            break;

        case SAI_MIRROR_SESSION_ATTR_VLAN_TPID:
            g_ctc_sai_mirror_info.session[sessionid].tpid = attr->value.u16;                
            break;

        case SAI_MIRROR_SESSION_ATTR_VLAN_ID:
            vlan_id = attr->value.s16;
            g_ctc_sai_mirror_info.session[sessionid].vlanid = attr->value.u16;                
            break;

        case SAI_MIRROR_SESSION_ATTR_VLAN_PRI:
            g_ctc_sai_mirror_info.session[sessionid].vlan_pri = attr->value.u8;                
            break;

        case SAI_MIRROR_SESSION_ATTR_ENCAP_TYPE:
            g_ctc_sai_mirror_info.session[sessionid].encap_type = attr->value.s32;                
            break;

        case SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION:
            g_ctc_sai_mirror_info.session[sessionid].iphdr_version = attr->value.u8;                
            break;
            
        case SAI_MIRROR_SESSION_ATTR_TOS:
            g_ctc_sai_mirror_info.session[sessionid].tos = attr->value.u8;                
            break;

        case SAI_MIRROR_SESSION_ATTR_TTL:
            g_ctc_sai_mirror_info.session[sessionid].ttl = attr->value.u8;                
            break;

        case SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS:
            sal_memcpy(&g_ctc_sai_mirror_info.session[sessionid].src_ip_address, 
                &attr->value.ipaddr, sizeof(sai_ip_address_t));              
            break;

        case SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS:
            sal_memcpy(&g_ctc_sai_mirror_info.session[sessionid].dst_ip_address, 
                &attr->value.ipaddr, sizeof(sai_ip_address_t));              
            break;

        case SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS:
            sal_memcpy(&g_ctc_sai_mirror_info.session[sessionid].src_mac_address, 
                &attr->value.mac, sizeof(sai_mac_t));                 
            break;

        case SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS:
            sal_memcpy(&g_ctc_sai_mirror_info.session[sessionid].dst_mac_address, 
                &attr->value.mac, sizeof(sai_mac_t));                 
            break;
            
        case SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE:
            g_ctc_sai_mirror_info.session[sessionid].gre_protocol_type = attr->value.u16;                
            break;

        default: /*not support other field now*/
            break;

        }

    }
    
    /*this code is just support current local mirror and remote mirror load sequence*/
    if(mirror_type == SAI_MIRROR_TYPE_LOCAL) /*add real local (inlcude multi-dest)mirror session to ASIC*/
    {
        if (is_multi_dest_mirror) /*add real multi-dest mirror session to ASIC*/
        {   
            /*add mcast*/
            sal_memset(&nh_mcast_group, 0, sizeof(nh_mcast_group));
            nh_mcast_group.is_mirror = TRUE;
            nh_mcast_group.mc_grp_id = CTC_SAI_MIRROR_MCAST_GROUP_ID;
            CTC_SAI_ERROR_GOTO(ctc_sai_nexthop_alloc_offset(&nexthop_id),ret,out);
            CTC_SAI_ERROR_GOTO(ctc_nh_add_mcast(nexthop_id, &nh_mcast_group), ret, out);

            /*add l2span session*/
            st_dest.session_id = sessionid;
            st_dest.rspan.nh_id = nexthop_id;
            st_dest.dir = CTC_BOTH_DIRECTION;
            st_dest.is_rspan = TRUE;
            st_dest.vlan_valid = FALSE;
            st_dest.type = CTC_MIRROR_L2SPAN_SESSION;
            CTC_SAI_ERROR_GOTO(ctc_mirror_add_session(&st_dest), ret, out);

            /*add l3span session*/
            st_dest.dir = CTC_BOTH_DIRECTION;
            st_dest.type = CTC_MIRROR_L3SPAN_SESSION;
            CTC_SAI_ERROR_GOTO(ctc_mirror_add_session(&st_dest), ret, out);

            /*update(add) mcast*/
            sal_memset(&nh_mcast_group, 0, sizeof(nh_mcast_group));
            nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_BRGMC_LOCAL;
            nh_mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
            nh_mcast_group.mem_info.destid = dest_gport;
            CTC_SAI_ERROR_GOTO(ctc_nh_update_mcast(nexthop_id, &nh_mcast_group), ret, out);
            g_ctc_sai_mirror_info.session[sessionid].nhid = nexthop_id;
        }
        else  if (dest_gport != CTC_LPORT_CPU)/*add real local mirror session to ASIC*/
        {
            /*add l2span session*/
            st_dest.session_id = sessionid;
            st_dest.dest_gport = dest_gport;
            st_dest.dir = CTC_BOTH_DIRECTION;
            st_dest.type = CTC_MIRROR_L2SPAN_SESSION;
            CTC_SAI_ERROR_GOTO(ctc_mirror_add_session(&st_dest), ret, out);

            /*add l3span session*/
            st_dest.dir = CTC_BOTH_DIRECTION;
            st_dest.type = CTC_MIRROR_L3SPAN_SESSION;
            CTC_SAI_ERROR_GOTO(ctc_mirror_add_session(&st_dest), ret, out);

            //st_dest.dir = CTC_BOTH_DIRECTION;
            //st_dest.type = CTC_MIRROR_CPU_SESSION;
            //CTC_SAI_ERROR_GOTO(ctc_mirror_add_session(&st_dest), ret, out);
        }
        else if (dest_gport == CTC_LPORT_CPU)  /*cpu mirror*/
        {
            /*add  cpu session*/
            st_dest.session_id = sessionid;
            st_dest.dest_gport = dest_gport;

            /*add l2span session*/
            st_dest.dir = CTC_BOTH_DIRECTION;
            st_dest.type = CTC_MIRROR_L2SPAN_SESSION;
            CTC_SAI_ERROR_GOTO(ctc_mirror_add_session(&st_dest), ret, out);

            /*add l3span session*/
            st_dest.dir = CTC_BOTH_DIRECTION;
            st_dest.type = CTC_MIRROR_L3SPAN_SESSION;
            CTC_SAI_ERROR_GOTO(ctc_mirror_add_session(&st_dest), ret, out);
            
            //st_dest.dir = CTC_BOTH_DIRECTION;
            //st_dest.type = CTC_MIRROR_CPU_SESSION;
            //CTC_SAI_ERROR_GOTO(ctc_mirror_add_session(&st_dest), ret, out);
        }

        /*set mirror dest port flag*/
        if (is_port)
        {
            CTC_SAI_ERROR_GOTO(ctc_sai_port_set_port_flag(CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, dest_gport), 
                SAI_PORT_FLAG_MIRROR_DEST, TRUE), ret, out);
        }
        else if (is_cpu_port)
        {
        }
        else
        {
            ctc_linkagg_get_member_ports(tid, p_gports, &cnt);
            for (i = 0; i < cnt; i++)
            {
                CTC_SAI_ERROR_GOTO(ctc_sai_port_set_port_flag(CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_gports[i]), 
                    SAI_PORT_FLAG_MIRROR_DEST, TRUE), ret, out);
            }
        }
    }
    else if (mirror_type == SAI_MIRROR_TYPE_REMOTE) /*add real remote mirror session to ASIC*/
    {   
        /*add rspan*/
        sal_memset(&rspan_nh_param, 0, sizeof(ctc_rspan_nexthop_param_t));
        CTC_SAI_ERROR_GOTO(ctc_sai_nexthop_alloc_offset(&nexthop_id),ret,out);
        rspan_nh_param.rspan_vid = vlan_id;
        CTC_SAI_ERROR_GOTO(ctc_nh_add_rspan(nexthop_id, &rspan_nh_param), ret, out);

        /*add l2span session*/
        st_dest.session_id = sessionid;
        st_dest.dest_gport = dest_gport;
        st_dest.dir = CTC_BOTH_DIRECTION;
        st_dest.rspan.nh_id = nexthop_id;
        st_dest.is_rspan = TRUE;
        st_dest.vlan_valid = FALSE;
        st_dest.type = CTC_MIRROR_L2SPAN_SESSION;
        CTC_SAI_ERROR_GOTO(ctc_mirror_add_session(&st_dest), ret, out);

        /*add l3span session*/
        st_dest.type = CTC_MIRROR_L3SPAN_SESSION;
        st_dest.dir = CTC_BOTH_DIRECTION;
        CTC_SAI_ERROR_GOTO(ctc_mirror_add_session(&st_dest), ret, out);
        
        g_ctc_sai_mirror_info.session[sessionid].nhid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, nexthop_id);
        g_ctc_sai_mirror_info.session[sessionid].vlanid = vlan_id;
    }
    else
    {
        return SAI_STATUS_NOT_SUPPORTED;
    }

    /*save this mirror session to DB*/
    g_ctc_sai_mirror_info.session[sessionid].sid = *session_oid;
    g_ctc_sai_mirror_info.session[sessionid].dest_gport = dest_gport;
    
out:
    return ret;

}

sai_status_t
ctc_sai_mirror_remove_mirror_session(
    sai_object_id_t  session_oid)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    uint32_t          opf_index = 0;
    uint32_t          sessionid = 0;
    sai_status_t      ret = SAI_STATUS_SUCCESS;
    uint32            nexthop_id = 0;
#ifdef USW
    uint16            cnt = 0;
#else
    uint8             cnt = 0;
#endif
    uint8             i    = 0;
    uint8             tid  = 0;
    uint32            p_gports[64];
    sai_object_id_t   port_oid = 0;
    ctc_mirror_dest_t st_dest;
    ctc_mcast_nh_param_group_t nh_mcast_group;
    ctc_sai_mirror_entry_t* p_session = NULL;
    
    sal_memset(&st_dest, 0, sizeof(ctc_mirror_dest_t));
    if (SAI_OBJECT_TYPE_MIRROR != CTC_SAI_OBJECT_TYPE_GET(session_oid))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    } 
    opf_index = CTC_SAI_OBJECT_INDEX_GET(session_oid);

    if (0 != ctclib_opb_free_offset(&p_sai_glb->mirror_id_opb, opf_index))
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    } 
    sessionid = opf_index;

    port_oid = ctc_sai_mirror_session_get_monitor_port(sessionid);
    if (0 == port_oid)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    p_session = ctc_sai_mirror_entry_get(session_oid);
    if(!p_session)
    {
        g_ctc_sai_mirror_info.session[sessionid].sid = 0;
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if(p_session->monitor_type == SAI_MIRROR_TYPE_LOCAL) /*delete local mirror(include multi-dest mirror) session from ASIC*/
    {
        if (p_session->is_multi_dest_mirror) /*delete multi-dest mirror session from ASIC*/
        {
            /*update mcast*/
            sal_memset(&nh_mcast_group, 0, sizeof(nh_mcast_group));
            nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_BRGMC_LOCAL;
            nh_mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
            nexthop_id = p_session->nhid;
            nh_mcast_group.mem_info.destid = CTC_SAI_OBJECT_INDEX_GET(port_oid);
            CTC_SAI_ERROR_GOTO(ctc_nh_update_mcast(nexthop_id, &nh_mcast_group), ret, out);

            /*remove l2span session*/
            st_dest.session_id = sessionid;
            st_dest.dir = CTC_BOTH_DIRECTION;
            st_dest.type = CTC_MIRROR_L2SPAN_SESSION;
            CTC_SAI_ERROR_GOTO(ctc_mirror_remove_session(&st_dest), ret, out);

            /*remove l3span session*/
            st_dest.dir = CTC_BOTH_DIRECTION;
            st_dest.type = CTC_MIRROR_L3SPAN_SESSION;
            CTC_SAI_ERROR_GOTO(ctc_mirror_remove_session(&st_dest), ret, out);

            /*remove mcast*/
            CTC_SAI_ERROR_GOTO(ctc_nh_remove_mcast(nexthop_id), ret, out);
            ctc_sai_nexthop_free_offset(nexthop_id);
            g_ctc_sai_mirror_info.session[sessionid].nhid = 0;
            g_ctc_sai_mirror_info.session[sessionid].is_multi_dest_mirror = FALSE;
        }
        else  /*delete local mirror session from ASIC*/
        {
            /*remove l2span session*/
            st_dest.session_id = sessionid;
            st_dest.dir = CTC_BOTH_DIRECTION;
            st_dest.type = CTC_MIRROR_L2SPAN_SESSION;
            CTC_SAI_ERROR_GOTO(ctc_mirror_remove_session(&st_dest), ret, out);

            /*remove l3span session*/
            st_dest.dir = CTC_BOTH_DIRECTION;
            st_dest.type = CTC_MIRROR_L3SPAN_SESSION;
            CTC_SAI_ERROR_GOTO(ctc_mirror_remove_session(&st_dest), ret, out);

            //st_dest.dir = CTC_BOTH_DIRECTION;
            //st_dest.type = CTC_MIRROR_CPU_SESSION;
            //CTC_SAI_ERROR_GOTO(ctc_mirror_remove_session(&st_dest), ret, out);
        }

        /*set mirror dest port flag*/
        if (SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(port_oid))
        {
            CTC_SAI_ERROR_GOTO(ctc_sai_port_set_port_flag(port_oid, SAI_PORT_FLAG_MIRROR_DEST, FALSE), ret, out);
        }
        else if (SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_oid))
        {
            tid = CTC_SAI_OBJECT_INDEX_GET(port_oid);
            CTC_SAI_ERROR_GOTO(ctc_linkagg_get_member_ports(tid, p_gports, &cnt), ret, out);
            for(i = 0; i < cnt; i++)
            {
                CTC_SAI_ERROR_GOTO(ctc_sai_port_set_port_flag(CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_gports[i]), 
                    SAI_PORT_FLAG_MIRROR_DEST, FALSE), ret, out);
            }
        }
    }
    if (p_session->monitor_type == SAI_MIRROR_TYPE_REMOTE) /*delete remote mirror session from ASIC*/
    {
        /*remove l2span session*/
        st_dest.session_id = sessionid;
        st_dest.dir = CTC_BOTH_DIRECTION;
        st_dest.type = CTC_MIRROR_L2SPAN_SESSION;
        CTC_SAI_ERROR_GOTO(ctc_mirror_remove_session(&st_dest), ret, out);

        /*remove l3span session*/
        st_dest.dir = CTC_BOTH_DIRECTION;
        st_dest.type = CTC_MIRROR_L3SPAN_SESSION;
        CTC_SAI_ERROR_GOTO(ctc_mirror_remove_session(&st_dest), ret, out);

        /*remove rspan*/
        if (p_session->nhid == 0)
        {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
        nexthop_id = p_session->nhid;
        CTC_SAI_ERROR_GOTO(ctc_nh_remove_rspan(nexthop_id), ret, out);
        ctc_sai_nexthop_free_offset(nexthop_id);
        g_ctc_sai_mirror_info.session[sessionid].nhid = 0;
    }
    
    /*set the multi-dest port in soft table to 0*/
    for (i = 0; i< CTC_SAI_MIRROR_MULTI_DEST_PORT_NUM; i++)
    {
        sal_memset(&g_ctc_sai_mirror_info.session[sessionid].monitor_port[i], 0, sizeof(sai_object_id_t));
        if (g_ctc_sai_mirror_info.session[sessionid].monitor_port_count)
        {
            g_ctc_sai_mirror_info.session[sessionid].monitor_port_count--;
        }
    }
    g_ctc_sai_mirror_info.session[sessionid].sid = 0;
    g_ctc_sai_mirror_info.session[sessionid].dest_gport = 0;
    
 out:
    return ret;
    
}

sai_status_t
ctc_sai_mirror_session_set_type(
    sai_object_id_t  session_oid,
       const sai_attribute_t	*attr)
{
     return SAI_STATUS_SUCCESS;
}

sai_status_t
 ctc_sai_mirror_session_set_vlan_id(
       sai_object_id_t  session_oid,
       const sai_attribute_t	*attr)
{     
     return SAI_STATUS_SUCCESS;
}

sai_status_t
 ctc_sai_mirror_session_set_cpu_dest_en(
       sai_object_id_t  session_oid,
       const sai_attribute_t	*attr)
{
    ctc_sai_mirror_entry_t* p_mirror = ctc_sai_mirror_entry_get(session_oid);
    ctc_mirror_dest_t st_dest;
    uint32_t sessionid = 0;
    int32_t ret = 0;

    if (SAI_OBJECT_TYPE_MIRROR != CTC_SAI_OBJECT_TYPE_GET(session_oid))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    } 
    sessionid = CTC_SAI_OBJECT_INDEX_GET(session_oid);

    sal_memset(&st_dest, 0, sizeof(st_dest));
    st_dest.session_id = sessionid;
    st_dest.dir = CTC_BOTH_DIRECTION;
    st_dest.type = CTC_MIRROR_CPU_SESSION;
    if (SAI_MIRROR_TYPE_LOCAL == p_mirror->monitor_type)
    {
        if (p_mirror->is_multi_dest_mirror)
        {
            st_dest.rspan.nh_id = CTC_SAI_OBJECT_INDEX_GET(p_mirror->nhid);
            st_dest.is_rspan = TRUE;
            st_dest.vlan_valid = FALSE;
        }
        else
        {
            st_dest.dest_gport = p_mirror->dest_gport;
        }
    }

    if (attr->value.u32)
    {
        ret = ctc_mirror_add_session(&st_dest);
    }
    else
    {
        ret = ctc_mirror_remove_session(&st_dest);
    }

    return ret;
}

sai_status_t
ctc_sai_mirror_session_set_multi_dest_member_add(
        sai_object_id_t  session_oid,
        const sai_attribute_t *attr)
{
    uint32_t          sessionid = 0;

    if (SAI_OBJECT_TYPE_MIRROR != CTC_SAI_OBJECT_TYPE_GET(session_oid))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    } 
    sessionid = CTC_SAI_OBJECT_INDEX_GET(session_oid);
    
    g_ctc_sai_mirror_info.session[sessionid].add_multidest_member = attr->value.booldata;
    
    return SAI_STATUS_SUCCESS;
    
}

sai_status_t
ctc_sai_mirror_session_set_monitor_port(
    sai_object_id_t  session_oid,
       const sai_attribute_t	*attr)
{
    uint32_t          sessionid = 0;
    uint32            dest_gport = 0;
    uint32            i = 0;
    sai_status_t      ret = SAI_STATUS_SUCCESS;
    sai_object_id_t   port_oid = 0;
    ctc_sai_mirror_entry_t* p_session = NULL;
    ctc_mcast_nh_param_group_t nh_mcast_group;

    sal_memset(&nh_mcast_group, 0, sizeof(nh_mcast_group));
    
    if (SAI_OBJECT_TYPE_MIRROR != CTC_SAI_OBJECT_TYPE_GET(session_oid))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    } 
    sessionid = CTC_SAI_OBJECT_INDEX_GET(session_oid);

    if (SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(attr->value.objlist.list[0]))
    {
        port_oid = attr->value.objlist.list[0];
        dest_gport = CTC_SAI_OBJECT_INDEX_GET(attr->value.objlist.list[0]);
    }
    else
    {
        return SAI_STATUS_NOT_SUPPORTED;
    }
    
    p_session = ctc_sai_mirror_entry_get(session_oid);
    if(!p_session)
    {
        g_ctc_sai_mirror_info.session[sessionid].sid = 0;
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /*add multi-dest mirror member*/
    if (g_ctc_sai_mirror_info.session[sessionid].add_multidest_member)
    {
        /*update multi-cast nhid*/
        nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_BRGMC_LOCAL;
        nh_mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
        nh_mcast_group.mem_info.destid = dest_gport;
        CTC_SAI_ERROR_GOTO(ctc_nh_update_mcast(p_session->nhid, &nh_mcast_group), ret, out);

        /*set port flag: TRUE*/
        CTC_SAI_ERROR_GOTO(ctc_sai_port_set_port_flag(port_oid, SAI_PORT_FLAG_MIRROR_DEST, TRUE), ret, out);

        /*store SAI mirror soft-table*/
        if (g_ctc_sai_mirror_info.session[sessionid].monitor_port_count)
        {
            for (i = 0; i< CTC_SAI_MIRROR_MULTI_DEST_PORT_NUM; i++)
            {
                if (g_ctc_sai_mirror_info.session[sessionid].monitor_port[i] == 0)
                {
                    g_ctc_sai_mirror_info.session[sessionid].monitor_port[i] = attr->value.objlist.list[0];  
                    g_ctc_sai_mirror_info.session[sessionid].monitor_port_count++;
                    break;
                }
            }
        }
    }
    else  /*del multi-dest mirror member*/
    {
        /*update multi-cast nhid*/
        nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_BRGMC_LOCAL;
        nh_mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
        nh_mcast_group.mem_info.destid = dest_gport;
        CTC_SAI_ERROR_GOTO(ctc_nh_update_mcast(p_session->nhid, &nh_mcast_group), ret, out);

        /*set port flag: FALSE*/
        CTC_SAI_ERROR_GOTO(ctc_sai_port_set_port_flag(port_oid, SAI_PORT_FLAG_MIRROR_DEST, FALSE), ret, out);

        /*set the SAI mirror soft-table*/
        if (g_ctc_sai_mirror_info.session[sessionid].monitor_port_count)
        {
            for (i = 0; i< CTC_SAI_MIRROR_MULTI_DEST_PORT_NUM; i++)
            {
                if (g_ctc_sai_mirror_info.session[sessionid].monitor_port[i] == attr->value.objlist.list[0])
                {
                    sal_memset(&g_ctc_sai_mirror_info.session[sessionid].monitor_port[i], 0, sizeof(sai_object_id_t));
                    g_ctc_sai_mirror_info.session[sessionid].monitor_port_count--;
                }
            }
        }
    }
out:
    return ret;
    
}

#define ________SAI_SAI_DEBUG_FUNC
sai_status_t
ctc_sai_create_mirror_session_debug_param(
        _Out_ sai_object_id_t *session_oid,
        _In_  uint32_t attr_count,
        _In_  const sai_attribute_t *attr_list)
{
    const sai_attribute_t *attr = NULL;
    uint32_t          attr_idx = 0; 

    CTC_SAI_DEBUG("in:session_oid 0x%llx", *session_oid);

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = (sai_attribute_t*)attr_list + attr_idx;

        switch(attr->id)
        {
            case SAI_MIRROR_SESSION_ATTR_TYPE:
                CTC_SAI_DEBUG("in:SAI_MIRROR_SESSION_ATTR_TYPE %d", attr->value.s32);
                break;
            case SAI_MIRROR_SESSION_ATTR_MONITOR_PORT:
                CTC_SAI_DEBUG("in:SAI_MIRROR_SESSION_ATTR_MONITOR_PORT %u", attr->value.oid);
                break;
            case SAI_MIRROR_SESSION_ATTR_VLAN_ID:
                CTC_SAI_DEBUG("in:SAI_MIRROR_SESSION_ATTR_VLAN_ID %d", attr->value.s16);
                break;
            case SAI_MIRROR_SESSION_ATTR_MULTI_DEST_MEMBER_ADD:
                CTC_SAI_DEBUG("in:SAI_MIRROR_SESSION_ATTR_MULTI_DEST_MEMBER_ADD %d", attr->value.booldata);
                break;
            default: 
                break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_set_mirror_session_attribute_debug_param(
        _In_ sai_object_id_t session_oid,
        _In_ const  sai_attribute_t *attr)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG("in:session_oid 0x%llx", session_oid);

    if (SAI_OBJECT_TYPE_MIRROR != CTC_SAI_OBJECT_TYPE_GET(session_oid))
    {
         return SAI_STATUS_INVALID_PARAMETER;
    }
    CTC_SAI_PTR_VALID_CHECK(attr);
    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_write_attr(
                            g_sai_attr_entries,attr),
                       ret,out);

    switch(attr->id)
    {
    case SAI_MIRROR_SESSION_ATTR_TYPE:
        CTC_SAI_DEBUG("in:SAI_MIRROR_SESSION_ATTR_TYPE");
        break;
    case SAI_MIRROR_SESSION_ATTR_MONITOR_PORT:
        CTC_SAI_DEBUG("in:SAI_MIRROR_SESSION_ATTR_TYPE");
        break;
    case SAI_MIRROR_SESSION_ATTR_VLAN_ID:
        CTC_SAI_DEBUG("in:SAI_MIRROR_SESSION_ATTR_TYPE");
        break;  
    case SAI_MIRROR_SESSION_ATTR_MULTI_DEST_MEMBER_ADD:
        CTC_SAI_DEBUG("in:SAI_MIRROR_SESSION_ATTR_MULTI_DEST_MEMBER_ADD");
        break;
    default: /*not support other field */
        CTC_SAI_DEBUG("in:NOT SUPPORT");
        break;
    }
out:
    return SAI_STATUS_SUCCESS;
}
sai_status_t
ctc_sai_get_mirror_session_attribute_debug_param(        
        _In_ sai_object_id_t session_oid,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
     sai_status_t ret = SAI_STATUS_SUCCESS;
     uint32_t  attr_idx = 0;
     sai_attribute_t* attr = NULL;

    CTC_SAI_DEBUG("in:session_oid 0x%llx attr_count %u", session_oid, attr_count);
     for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
     {
        attr = attr_list + attr_idx;
        ret = ctc_sai_attr_check_read_attr(g_sai_attr_entries,attr);

        if(ret != SAI_STATUS_SUCCESS)
        {
            return ret + attr_idx;
        }
        switch(attr->id)
        {
        case SAI_MIRROR_SESSION_ATTR_TYPE:
            CTC_SAI_DEBUG("out:SAI_MIRROR_SESSION_ATTR_TYPE %d", attr->value.s32);
            break;
            
        case SAI_MIRROR_SESSION_ATTR_MONITOR_PORT:
            CTC_SAI_DEBUG("out:SAI_MIRROR_SESSION_ATTR_MONITOR_PORT 0x%llx", attr->value.oid);
            break;
            
        case SAI_MIRROR_SESSION_ATTR_VLAN_ID:
            CTC_SAI_DEBUG("out:SAI_MIRROR_SESSION_ATTR_VLAN_ID %d", attr->value.s16);
            break;
            
        case SAI_MIRROR_SESSION_ATTR_MULTI_DEST_MEMBER_ADD:
            CTC_SAI_DEBUG("out:SAI_MIRROR_SESSION_ATTR_MULTI_DEST_MEMBER_ADD %d", attr->value.booldata);
            break;
            
        default: /*not support other field */
            break;
        }
        if (SAI_STATUS_SUCCESS != ret)
        {
            break;
        }
     }

    return SAI_STATUS_SUCCESS;
}
#define ________SAI_SAI_API_FUNC

/**
 * @brief Create mirror session.
 *
 * @param[out] session_id Port mirror session id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Value of attributes
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t
ctc_sai_create_mirror_session(
        _Out_ sai_object_id_t *session_oid,
        _In_  uint32_t attr_count,
        _In_  const sai_attribute_t *attr_list)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;
    ctc_sai_attr_entry_list_t* pattr_entry_list = NULL;
    
    CTC_SAI_DEBUG_FUNC();
    ctc_sai_create_mirror_session_debug_param(session_oid, attr_count, attr_list);

    CTC_SAI_PTR_VALID_CHECK(attr_list);
    CTC_SAI_PTR_VALID_CHECK(session_oid);

    sal_memset(&attr, 0, sizeof(sai_attribute_t));
    
    CTC_SAI_ERROR_GOTO(ctc_sai_attr_alloc_attr_entry_list(g_sai_attr_entries,
                            attr_list,
                            attr_count,
                            &pattr_entry_list),ret,out);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_attr_entry_list(g_sai_attr_entries,
                            pattr_entry_list), ret, out);
    
    ret = ctc_sai_mirror_create_mirror_session(session_oid, attr_count, attr_list);
    if (SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }
    
out:
    if(pattr_entry_list)
    {
        ctc_sai_attr_free_attr_entry_list(pattr_entry_list);
    }
    return ret;

}


/**
 * @brief Remove mirror session.
 *
 * @param[in] session_id Port mirror session id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t
ctc_sai_remove_mirror_session(
   _In_ sai_object_id_t session_oid)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    CTC_SAI_DEBUG_FUNC();

    ret = ctc_sai_mirror_remove_mirror_session(session_oid);
    
    return ret;
}

/**
 * @brief Set mirror session attributes.
 *
 * @param[in] session_id Port mirror session id
 * @param[in] attr Value of attribute
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t
ctc_sai_set_mirror_session_attribute(
        _In_ sai_object_id_t session_oid,
        _In_ const  sai_attribute_t *attr)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;  

    CTC_SAI_DEBUG_FUNC();
    ctc_sai_set_mirror_session_attribute_debug_param(session_oid, attr);

    if (SAI_OBJECT_TYPE_MIRROR != CTC_SAI_OBJECT_TYPE_GET(session_oid))
    {
         return SAI_STATUS_INVALID_PARAMETER;
    }
    CTC_SAI_PTR_VALID_CHECK(attr);
    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_write_attr(
                            g_sai_attr_entries,attr),
                       ret,out);
    switch(attr->id)
    {
    case SAI_MIRROR_SESSION_ATTR_TYPE:
         ret = ctc_sai_mirror_session_set_type(session_oid, attr);
        break;
    case SAI_MIRROR_SESSION_ATTR_MONITOR_PORT:
        ret = ctc_sai_mirror_session_set_monitor_port(session_oid, attr);
        break;
    case SAI_MIRROR_SESSION_ATTR_MULTI_DEST_MEMBER_ADD:
        ret = ctc_sai_mirror_session_set_multi_dest_member_add(session_oid, attr);
        break;
    case SAI_MIRROR_SESSION_ATTR_VLAN_ID:
        ret = ctc_sai_mirror_session_set_vlan_id(session_oid, attr);
        break;
    case SAI_MIRROR_SESSION_ATTR_CPU_DEST_EN:
        ret = ctc_sai_mirror_session_set_cpu_dest_en(session_oid, attr);
        break;
    default: /*not support other field */
        break;
    }
out:
    return ret;
}

/**
 * @brief Get mirror session attributes.
 *
 * @param[in] session_id Port mirror session id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Value of attribute
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */

sai_status_t
ctc_sai_get_mirror_session_attribute(        
        _In_ sai_object_id_t session_oid,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
     uint32_t  sessionid = 0;
     uint32_t  attr_idx = 0;
     uint32_t  index = 0;
     uint32_t  i = 0;
     sai_status_t ret = SAI_STATUS_SUCCESS;
     sai_attribute_t* attr = NULL;
 
     CTC_SAI_DEBUG_FUNC();

     if (SAI_OBJECT_TYPE_MIRROR != CTC_SAI_OBJECT_TYPE_GET(session_oid))
     {
         return SAI_STATUS_INVALID_PARAMETER;
     }
     sessionid = CTC_SAI_OBJECT_INDEX_GET(session_oid);
     CTC_SAI_PTR_VALID_CHECK(attr_list);  
     
     if (0 == g_ctc_sai_mirror_info.session[sessionid].sid)
     {
         return SAI_STATUS_ITEM_NOT_FOUND;
     }
     for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
     {
        attr = attr_list + attr_idx;
        ret = ctc_sai_attr_check_read_attr(g_sai_attr_entries,attr);

        if(ret != SAI_STATUS_SUCCESS)
        {
            return ret + attr_idx;
        }
        switch(attr->id)
        {
        case SAI_MIRROR_SESSION_ATTR_TYPE:
            attr->value.s32 = g_ctc_sai_mirror_info.session[sessionid].monitor_type;
            break;
            
        case SAI_MIRROR_SESSION_ATTR_MONITOR_PORT:
            for (i = 0; i< CTC_SAI_MIRROR_MULTI_DEST_PORT_NUM; i++)
            {
                if (g_ctc_sai_mirror_info.session[sessionid].monitor_port[i] != 0)
                {
                    attr->value.objlist.list[index++] = g_ctc_sai_mirror_info.session[sessionid].monitor_port[i];
                }
            }
            attr->value.objlist.count = index;
            break;
            
        case SAI_MIRROR_SESSION_ATTR_MULTI_DEST_MEMBER_ADD:
            attr->value.booldata = g_ctc_sai_mirror_info.session[sessionid].is_multi_dest_mirror;
            break;

        case SAI_MIRROR_SESSION_ATTR_VLAN_ID:
            attr->value.s16 = g_ctc_sai_mirror_info.session[sessionid].vlanid;
            break;
        default: /*not support other field */
            break;
        }
        if (SAI_STATUS_SUCCESS != ret)
        {
            break;
        }
     }
    ctc_sai_get_mirror_session_attribute_debug_param(session_oid, attr_count, attr_list);
    return SAI_STATUS_SUCCESS;

}


#define ________SAI_INNER_FUNC
static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ret = ctc_sai_mirror_db_init();
    if(SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }
    /*added by yejl to fix bug41942 and bug42056, 2017-01-17*/
    ctc_mirror_set_mirror_discard(CTC_EGRESS, CTC_MIRROR_L2SPAN_DISCARD, FALSE);
    ctc_mirror_set_mirror_discard(CTC_EGRESS, CTC_MIRROR_L3SPAN_DISCARD, FALSE);
    ctc_mirror_set_mirror_discard(CTC_EGRESS, CTC_MIRROR_ACLLOG_PRI_DISCARD, FALSE);

    /*added by yejl to fix bug50951, 2019-01-28*/
    //ctc_mirror_set_mirror_discard(CTC_INGRESS, CTC_MIRROR_L2SPAN_DISCARD, FALSE);
    //ctc_mirror_set_mirror_discard(CTC_INGRESS, CTC_MIRROR_L3SPAN_DISCARD, FALSE);
    
    preg->init_status =  INITIALIZED;
    return ret;
}

static sai_status_t
__exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static sai_mirror_api_t      g_sai_api_func = {
    .create_mirror_session            = ctc_sai_create_mirror_session,
    .remove_mirror_session            = ctc_sai_remove_mirror_session,
    .set_mirror_session_attribute     = ctc_sai_set_mirror_session_attribute,
    .get_mirror_session_attribute     = ctc_sai_get_mirror_session_attribute,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id  		= SAI_API_MIRROR,
        .init_func 	= __init_mode_fn,
        .exit_func 	= __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_FDB_OUTER_FUNC
sai_status_t
ctc_sai_mirror_init()
{
    api_reg_register_fn(&g_api_reg_info);
    
    return SAI_STATUS_SUCCESS;
}
