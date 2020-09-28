/****************************************************************************
* $Id$
*  Centec nat information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2017-09-27
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
//#include "hsrv_if.h"
#include "gen/tbl_nat_rule_define.h"
#include "gen/tbl_nat_rule.h"
#include "gen/tbl_nat_session_define.h"
#include "gen/tbl_nat_session.h"
#include "gen/tbl_nat_global_define.h"
#include "gen/tbl_nat_global.h"
#include "hsrv_msg.h"
#include "hsrv_message.h"
#include "hsrv_debug.h"
#include "sai.h"

#if _DESC("nat forward entry process funtions")
/***************************************************************************************************
 * Name         : hsrv_nat_fwd_entry_add
 * Purpose      : 
 * Input        : 
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_nat_fwd_entry_add(sai_nat_forward_entry_t *p_fwd, uint32 iif_index)
{
    sai_nat_api_t *nat_api = NULL;
    sai_attribute_t attr;
    tbl_rif_key_t rif_key;
    tbl_rif_t* p_rif = NULL;

    HSRV_NAT_SESSION_DEBUG("Enter into %s.", __FUNCTION__);
    sal_memset(&rif_key, 0, sizeof(rif_key));

    if (tbl_interface_get_name_by_ifindex(iif_index, rif_key.name, IFNAME_SIZE))
    {
        HSRV_NAT_SESSION_DEBUG("Failed to get iif name by index %x.", iif_index);
        return HSRV_E_FAIL;
    }

    p_rif = tbl_rif_get_rif(&rif_key);
    HSRV_PTR_CHECK(p_rif);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NAT,(void**)&nat_api));

    sal_memset(&attr, 0, sizeof(attr));
    attr.id = SAI_NAT_ATTR_INSIDE_INTERFACE;
    attr.value.oid = p_rif->rif_id;

    return nat_api->create_nat_forward_entry((const sai_nat_forward_entry_t *)p_fwd, 1, &attr);
}


/***************************************************************************************************
 * Name         : hsrv_nat_add_del_forward
 * Purpose      : delete forward entry
 * Input        : sai_nat_forward_entry_t *p_fwd, uint32 iif_index
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_nat_fwd_entry_del(sai_nat_forward_entry_t *p_fwd, uint32 iif_index)
{
    sai_nat_api_t *nat_api = NULL;

    HSRV_NAT_SESSION_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NAT,(void**)&nat_api));

    return nat_api->remove_nat_forward_entry((const sai_nat_forward_entry_t *)p_fwd);
}

/***************************************************************************************************
 * Name         : hsrv_nat_fwd_entry_hit_get
 * Purpose      : get entry hit flag
 * Input        : sai_nat_forward_entry_t *p_fwd
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_nat_fwd_entry_hit_get(sai_nat_forward_entry_t *p_fwd, uint8 *p_if_hit)
{
    sai_nat_api_t *nat_api = NULL;
    sai_attribute_t attr;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NAT,(void**)&nat_api));

    sal_memset(&attr, 0, sizeof(attr));
    attr.id = SAI_NAT_ATTR_ENTRY_HIT;
    if (nat_api->get_nat_forward_attribute((const sai_nat_forward_entry_t *)p_fwd, &attr))
    {
        HSRV_NAT_SESSION_DEBUG("entry hit get function return error.");
        *p_if_hit = FALSE;
        return HSRV_E_FAIL;
    }

    *p_if_hit = attr.value.u8;
    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_nat_fwd_entry_hit_set
 * Purpose      : clear entry hit flag
 * Input        : sai_nat_forward_entry_t *p_fwd, uint8 if_hit
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_nat_fwd_entry_hit_set(sai_nat_forward_entry_t *p_fwd, uint8 if_hit)
{
    sai_nat_api_t *nat_api = NULL;
    sai_attribute_t attr;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NAT,(void**)&nat_api));

    sal_memset(&attr, 0, sizeof(attr));
    attr.id = SAI_NAT_ATTR_ENTRY_HIT;
    attr.value.u8 = if_hit;

    return nat_api->set_nat_forward_attribute((const sai_nat_forward_entry_t *)p_fwd, &attr);
}

/***************************************************************************************************
 * Name         : hsrv_nat_fwd_entry_translate_from_session
 * Purpose      : convert nat session to sai nat forward entry data structure
 * Input        : tbl_nat_session_t *p_session, sai_nat_forward_entry_t *p_fwd
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
void
hsrv_nat_fwd_entry_translate_from_session(tbl_nat_session_t *p_session, sai_nat_forward_entry_t *p_fwd)
{

    p_fwd->inside_local_ip.addr.ip4 = sal_htonl(p_session->key.inside_local_ip.u.prefix4.s_addr);
    p_fwd->inside_local_ip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    p_fwd->inside_local_port = p_session->key.proto_info.l4_port.inside_local;

    p_fwd->inside_global_ip.addr.ip4 = sal_htonl(p_session->key.inside_global_ip.u.prefix4.s_addr);
    p_fwd->inside_global_ip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    p_fwd->inside_global_port = p_session->key.proto_info.l4_port.inside_global;
    p_fwd->vrf_id = GLB_DEF_VRFID;

    if (IPPROTO_TCP == p_session->key.ip_protocol)
    {
        p_fwd->flags |= SAI_NAT_FWD_FLAG_USE_TCP_PORT;
    }

    return;
}

/***************************************************************************************************
 * Name         : hsrv_nat_fwd_entry_translate_from_rule
 * Purpose      : convert nat static rule to sai nat forward entry data structure.
                  static basic NAT add sai forward entry when NAT rule configured.
 * Input        : bl_nat_rule_t *p_rule
 * Output       : sai_nat_forward_entry_t *p_fwd
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
void
hsrv_nat_fwd_entry_translate_from_rule(tbl_nat_rule_t *p_rule, sai_nat_forward_entry_t *p_fwd)
{
    ds_connected_t *ifc_primary = NULL;

    HSRV_NAT_RULE_DEBUG("Enter into %s.", __FUNCTION__);

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_OUTSIDE_IF)
    {
        tbl_route_if_key_t rt_if_key;
        tbl_route_if_t *p_rf_if = NULL;
        ctclib_slistnode_t *listnode = NULL;

        sal_memset(&rt_if_key, 0, sizeof(rt_if_key));
        sal_memcpy(rt_if_key.name, p_rule->key.oif, GLB_IFNAME_SZ);
        p_rf_if = tbl_route_if_get_route_if(&rt_if_key);
        if (!p_rf_if)
        {
            HSRV_NAT_RULE_DEBUG("Failed to get outside if. name:%s.", rt_if_key.name);
            return;
        }

        /* get IF primary IP, easy IP nat do not support secondary IP address.*/
        CTCLIB_SLIST_LOOP(p_rf_if->ifc_ipv4.obj_list, ifc_primary, listnode)
        {
            if (!GLB_FLAG_ISSET(ifc_primary->flags, RT_IF_ADDR_SECONDARY))
            {
                break;
            }
        }

        if (!ifc_primary)
        {
            HSRV_NAT_RULE_DEBUG("Have no primary IP. If name:%s.", rt_if_key.name);
            return;
        }
    }
    
    sal_memset(p_fwd, 0, sizeof(sai_nat_forward_entry_t));

    p_fwd->inside_local_ip.addr.ip4 = sal_htonl(p_rule->key.in_local_ip.u.prefix4.s_addr);
    p_fwd->inside_local_ip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    p_fwd->inside_local_port = 0;
    p_fwd->inside_global_ip.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    p_fwd->inside_global_port = 0;
    p_fwd->vrf_id = GLB_DEF_VRFID;

    if ((p_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO)
     && (IPPROTO_TCP == p_rule->key.protocol))
    {
        p_fwd->flags |= SAI_NAT_FWD_FLAG_USE_TCP_PORT;
    }

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_OUTSIDE_IF)
    {
        p_fwd->inside_global_ip.addr.ip4 = sal_htonl(ifc_primary->key.address.u.prefix4.s_addr);
    }
    else
    {
        p_fwd->inside_global_ip.addr.ip4 = sal_htonl(p_rule->key.in_glb_ip.u.prefix4.s_addr);
    }

    return;
}
#endif

#if _DESC("nat rule process funtions")
/***************************************************************************************************
 * Name         : hsrv_nat_rule_oper
 * Purpose      : static basic NAT add sai forward entry when NAT rule configured.
 * Input        : tbl_nat_rule_t *p_rule
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_nat_rule_oper(tbl_nat_rule_t *p_rule, int32 (*fn) (sai_nat_forward_entry_t *, uint32))
{
    int32 ret = 0;
    sai_nat_forward_entry_t fwd_entry;
    tbl_route_if_key_t rt_if_key;
    tbl_route_if_t *p_rf_if = NULL;

    HSRV_NAT_RULE_DEBUG("Enter into %s.", __FUNCTION__);

    /* care about inside static basic nat*/
    if (!(p_rule->key.flags & GLB_NAT_RULE_FLAG_INSIDE)
     || !(p_rule->key.flags & GLB_NAT_RULE_FLAG_SNAT)
     || !(p_rule->key.flags & GLB_NAT_RULE_FLAG_STATIC)
     || !(p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_IP))
    {
        return HSRV_E_NONE;
    }

    /* ignore NAPT, maybe ALG flows exist, should be processed by kernel*/
    if ((p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_PORT)
     || (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT)
     || (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT_HI))
    {
        return HSRV_E_NONE;
    }

    if (0 == p_rule->refer_iif[0])
    {
        HSRV_NAT_RULE_DEBUG("hsrv_nat_rule_oper, no iif");
        return HSRV_E_NONE;
    }

    /* get iif index */
    sal_memset(&rt_if_key, 0, sizeof(rt_if_key));
    sal_memcpy(rt_if_key.name, p_rule->refer_iif, GLB_IFNAME_SZ);
    p_rf_if = tbl_route_if_get_route_if(&rt_if_key);
    if (!p_rf_if)
    {
        HSRV_NAT_RULE_DEBUG("Interface %s does not exist", p_rule->refer_iif);
        return HSRV_E_NONE;
    }

    hsrv_nat_fwd_entry_translate_from_rule(p_rule, &fwd_entry);

    ret = fn(&fwd_entry, p_rf_if->ifindex);

    HSRV_NAT_RULE_DEBUG("hsrv_nat_rule_oper get ret:%u", ret);
    return ret;
}

/***************************************************************************************************
 * Name         : hsrv_nat_rule_sync 
 * Purpose      : load nat rule cdb to sai          
 * Input        : p_tbl_node: table node pointer
                  p_ds_node: ds node pointer
                  oper: operation code
                  field_id: field id with operation
                  p_tbl: cdb table pointer
                  p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_nat_rule_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    /* deleted for bug52815, SDK can not support TCP flows without port configured, delete static NAT rule processing in hsrv.
       the first packet always be sent to CPU kernel for creation NFCT, regardless static NAT rule or dynamic NAT rule configured.*/
#if 0 
    tbl_nat_rule_t   *p_nat_rule = NULL;
    tbl_nat_rule_t   *p_db_nat_rule = NULL;
    HSRV_PTR_CHECK(p_tbl);

    HSRV_NAT_RULE_DEBUG("Enter into %s.", __FUNCTION__);

    /*3. process tbl*/  
    switch (oper)
    {
        case CDB_OPER_ADD:
            p_nat_rule = (tbl_nat_rule_t*)p_tbl;
            HSRV_PTR_CHECK(p_nat_rule);
            HSRV_IF_ERROR_RETURN(hsrv_nat_rule_oper(p_nat_rule, hsrv_nat_fwd_entry_add));
            break;

        case CDB_OPER_DEL:
            p_nat_rule = (tbl_nat_rule_t*)p_tbl;
            HSRV_PTR_CHECK(p_nat_rule);

            p_db_nat_rule = tbl_nat_rule_get_nat_rule(&p_nat_rule->key);
            HSRV_PTR_CHECK(p_db_nat_rule);

            HSRV_IF_ERROR_RETURN(hsrv_nat_rule_oper(p_db_nat_rule, hsrv_nat_fwd_entry_del));
            break;

        case CDB_OPER_SET:
            break;

        default:
            return HSRV_E_INVALID_PARAM;
    }
#endif
    return HSRV_E_NONE;
}



/***************************************************************************************************
 * Name         : hsrv_nat_rule_after_sync
 * Purpose      : load nat rule cdb to sai          
 * Input        : p_tbl_node: table node pointer
                  p_ds_node: ds node pointer
                  oper: operation code
                  field_id: field id with operation
                  p_tbl: cdb table pointer
                  p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_nat_rule_after_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    
    /*3. process tbl*/  
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}
#endif

#if _DESC("nat session process funtions")
void hsrv_nat_session_dump(tbl_nat_session_t *p_session)
{
    //int seconds;
    //int seconds, minutes, hours;
    char     sz_addr1[64];
    char     sz_addr2[64];
    char     sz_ttlc[64];

    sal_memset(sz_addr1, 0, sizeof(sz_addr1));
    sal_memset(sz_addr2, 0, sizeof(sz_addr2));
    sal_memset(sz_ttlc, 0, sizeof(sz_ttlc));
    printf("dump pkt: \n");

    //seconds = p_session->ttl;
    //minutes = seconds/60;
    //hours = minutes/60;
    //minutes = minutes%60;
    //seconds = seconds%60;

    //snprintf(sz_ttlc,11, "%3i:%02i:%02i", hours, minutes, seconds);
    
    printf(" l4proto:%u, time:%u, ttl:%u\n", p_session->key.ip_protocol, (uint32)p_session->timestamp, p_session->ttl);
    printf(" orig = ip:%s, sport:%u <===> new = ip:%s, dport:%u\n",
              cdb_addr_ipv4_val2str(sz_addr1, 64, &p_session->key.inside_local_ip.u.prefix4), 
              p_session->key.proto_info.l4_port.inside_local,
              cdb_addr_ipv4_val2str(sz_addr2, 64, &p_session->key.inside_global_ip.u.prefix4),
              p_session->key.proto_info.l4_port.inside_global);
    return;
}

/***************************************************************************************************
 * Name         : hsrv_nat_session_oper
 * Purpose      : add or delete nat forward rules.
 * Input        : tbl_nat_session_t *p_session
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_nat_session_oper(tbl_nat_session_t *p_session, int32 (*fn) (sai_nat_forward_entry_t *, uint32))
{
    sai_nat_forward_entry_t fwd_entry;

    if (TRUE == p_session->slow_path_fwd)
    {
        /* ICMP and ALG NAT flows forward by kernel, no need to set forward entry to CHIP*/
        return HSRV_E_NONE;
    }

    HSRV_NAT_SESSION_DEBUG("Enter into %s.", __FUNCTION__);
    sal_memset(&fwd_entry, 0, sizeof(fwd_entry));
    hsrv_nat_fwd_entry_translate_from_session(p_session, &fwd_entry);

    return fn(&fwd_entry, p_session->iif_index);
}

/***************************************************************************************************
 * Name         : hsrv_nat_session_aging_timer_func
 * Purpose      : aging timer, read entry hit flag, and process aging
 * Input        : void *p_data
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
void
hsrv_nat_session_aging_timer_func(void *p_data)
{
    tbl_nat_global_t         *p_nat_global = NULL;
    tbl_nat_session_master_t *p_master = NULL;
    ctclib_slistnode_t       *listnode = NULL;
    ctclib_slistnode_t       *next = NULL;
    tbl_nat_session_t        *p_session = NULL;
    uint32 id_min = 0;
    uint32 id_max = 0;
    char str[256];
    uint8 if_hit = FALSE;
    int32 ret = PM_E_NONE;
    sai_nat_forward_entry_t fwd_entry;    
    sal_time_t time_curr = 0;

    p_nat_global = tbl_nat_global_get_nat_global();
    if (!p_nat_global)
    {
        return;    
    }

    p_master = tbl_nat_session_get_master();
    if (!p_master)
    {
        return;    
    }

    id_min = p_nat_global->timer_mark * NAT_SESSION_AGING_TIMER_PROC_NUM_ONCE;
    if (id_min >= NAT_SESSION_SPEC)
    {
        p_nat_global->timer_mark = 0;
        id_min = 0;
    }

    id_max = id_min + NAT_SESSION_AGING_TIMER_PROC_NUM_ONCE - 1;
    if (id_max >= NAT_SESSION_SPEC)
    {
        id_max = NAT_SESSION_SPEC - 1;
    }

    time_curr = ctc_time_boottime_sec(NULL);
    CTCLIB_SLIST_LOOP_DEL(p_master->nat_session_list, p_session, listnode, next)
    {
        if (TRUE == p_session->slow_path_fwd)
        {
            continue;
        }

        if ((p_session->id < id_min) || (p_session->id > id_max))
        {
            continue;
        }

        if (GLB_NAT_SET_SAI_STATUS_FAILED == p_session->set_sai_status)
        {
            /* entries failed to set to SAI, aging it and create again.*/
            if_hit = FALSE;
        }
        else
        {
            /* get entry hit and check ttl*/
            sal_memset(&fwd_entry, 0, sizeof(fwd_entry));
            hsrv_nat_fwd_entry_translate_from_session(p_session, &fwd_entry);

            ret = hsrv_nat_fwd_entry_hit_get(&fwd_entry, &if_hit);
            if (ret)
            {
                HSRV_NAT_TIMER_DEBUG("sai return error. ret:%d", ret);
            }
        }

        if (TRUE == if_hit)
        {
            hsrv_nat_fwd_entry_hit_set(&fwd_entry, FALSE);

            p_session->hit = time_curr;
        }
        else
        {
            HSRV_NAT_TIMER_DEBUG("session:%s, Entry not hit.", cdb_nat_session_key_val2str(str, 256, &p_session->key));
        }

        if ((p_session->hit + p_session->ttl) <= time_curr)
        {
            HSRV_NAT_TIMER_DEBUG("Session timeout, Aging entry. hit:%u, ttl:%u, curr:%u", p_session->hit, p_session->ttl, time_curr);
            HSRV2ROUTED_MSG_SEND_NOTIFY(ROUTED_FEI_NAT_SESSION_AGING_DEL, p_session, sizeof(tbl_nat_session_t));
        }
        else if ((p_session->upd_kernel_ttl_time + NAT_NFCT_FAST_PATH_FWD_TTL_REFRESH_MIN) <= time_curr)
        {
            /* if session live another 8 minutes, and not aging. notice: maybe hit value not updated.*/
            p_session->upd_kernel_ttl_time = time_curr;
            tbl_nat_session_set_nat_session_field(p_session, TBL_NAT_SESSION_FLD_HIT);/* notify hit, update kernel ttl.*/
        }
    }

    p_nat_global->timer_mark++;
    if (p_nat_global->timer_mark >= NAT_SESSION_AGING_TIMER_MARK_MAX)
    {
        p_nat_global->timer_mark = 0;
    }

    p_nat_global->aging_timer = ctc_task_add_timer(TASK_PRI_NORMAL, hsrv_nat_session_aging_timer_func, NULL, NAT_SESSION_AGING_TIMER_LEN);
    return;
}

void
hsrv_nat_session_cnt_increase(tbl_nat_session_t *p_session)
{
    tbl_nat_global_t *p_nat_global = tbl_nat_global_get_nat_global();

    switch (p_session->creator)
    {
        case GLB_NAT_SESSION_CREATOR_INSIDE:
        {
            p_nat_global->session_cnt_upstream++;
            break;
        }
        case GLB_NAT_SESSION_CREATOR_OUTSIDE:
        {
            p_nat_global->session_cnt_dnstream++;
            break;
        }
    }
    return;
}

void
hsrv_nat_session_cnt_decrease(tbl_nat_session_t *p_session)
{
    tbl_nat_global_t *p_nat_global = tbl_nat_global_get_nat_global();

    switch (p_session->creator)
    {
        case GLB_NAT_SESSION_CREATOR_INSIDE:
        {
            /* upstream total session count, for stm show.*/
            if (p_nat_global->session_cnt_upstream > 0)
            {
                p_nat_global->session_cnt_upstream--;
            }
            break;
        }
        case GLB_NAT_SESSION_CREATOR_OUTSIDE:
        {
            if (p_nat_global->session_cnt_dnstream > 0)
            {
                p_nat_global->session_cnt_dnstream--;
            }
            break;
        }
    }
    return;
}

void
hsrv_nat_session_fast_path_fwd_cnt_increase(tbl_nat_session_t *p_session)
{
    tbl_nat_global_t *p_nat_global = tbl_nat_global_get_nat_global();

    if (TRUE == p_session->slow_path_fwd)
    {
        return;
    }

    switch (p_session->creator)
    {
        case GLB_NAT_SESSION_CREATOR_INSIDE:
        {
            p_nat_global->session_fast_fwd_cnt_upstream++;
            break;
        }
        case GLB_NAT_SESSION_CREATOR_OUTSIDE:
        {
            p_nat_global->session_fast_fwd_cnt_dnstream++;
            break;
        }
    }
    return;
}

void
hsrv_nat_session_fast_path_fwd_cnt_decrease(tbl_nat_session_t *p_session)
{
    tbl_nat_global_t *p_nat_global = tbl_nat_global_get_nat_global();

    if (TRUE == p_session->slow_path_fwd)
    {
        return;
    }

    switch (p_session->creator)
    {
        case GLB_NAT_SESSION_CREATOR_INSIDE:
        {
            if (p_nat_global->session_fast_fwd_cnt_upstream > 0)
            {
                p_nat_global->session_fast_fwd_cnt_upstream--;
            }
            break;
        }
        case GLB_NAT_SESSION_CREATOR_OUTSIDE:
        {
            if (p_nat_global->session_fast_fwd_cnt_dnstream > 0)
            {
                p_nat_global->session_fast_fwd_cnt_dnstream--;
            }
            break;
        }
    }
    return;
}

/***************************************************************************************************
 * Name         : hsrv_nat_session_sync 
 * Purpose      : load nat session cdb to sai          
 * Input        : p_tbl_node: table node pointer
                  p_ds_node: ds node pointer
                  oper: operation code
                  field_id: field id with operation
                  p_tbl: cdb table pointer
                  p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_nat_session_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    int32 ret = 0;
    tbl_nat_session_t *p_session = NULL;
    tbl_nat_session_t *p_db_session = NULL;
    tbl_nat_global_t  *p_nat_global = tbl_nat_global_get_nat_global();

    p_session = (tbl_nat_session_t*)p_tbl;
    HSRV_PTR_CHECK(p_session);

    switch (oper)
    {
        case CDB_OPER_ADD:
            p_db_session = tbl_nat_session_get_nat_session(&p_session->key);

            if ((p_db_session) && (GLB_NAT_SET_SAI_STATUS_SUCCESS == p_db_session->set_sai_status))
            {
                /* already set*/
                return HSRV_E_NONE;
            }

            /* total session count*/
            hsrv_nat_session_cnt_increase(p_session);

            if (p_session->slow_path_fwd)
            {
                return HSRV_E_NONE;
            }

            if (p_nat_global->session_fast_fwd_cnt_dnstream + p_nat_global->session_fast_fwd_cnt_upstream >= NAT_SESSION_SPEC)
            {
                /*beyond hardware limit*/
                p_session->set_sai_status = GLB_NAT_SET_SAI_STATUS_FAILED;
                tbl_nat_session_set_nat_session_field(p_session, TBL_NAT_SESSION_FLD_SET_SAI_STATUS);
                return HSRV_E_NONE;
            }

            /* set to lsw*/
            ret = hsrv_nat_session_oper(p_session, hsrv_nat_fwd_entry_add);
            if (ret)
            {
                p_session->set_sai_status = GLB_NAT_SET_SAI_STATUS_FAILED;
                tbl_nat_session_set_nat_session_field(p_session, TBL_NAT_SESSION_FLD_SET_SAI_STATUS);
                return HSRV_E_NONE;
            }

            /* set sai ok, increase success cnt*/
            p_session->set_sai_status = GLB_NAT_SET_SAI_STATUS_SUCCESS;
            hsrv_nat_session_fast_path_fwd_cnt_increase(p_session);
            break;

        case CDB_OPER_DEL:
            p_db_session = tbl_nat_session_get_nat_session(&p_session->key);
            HSRV_PTR_CHECK(p_db_session);

            hsrv_nat_session_cnt_decrease(p_db_session);

            if (GLB_NAT_SET_SAI_STATUS_SUCCESS != p_db_session->set_sai_status)
            {
                /* not set to sai yet*/
                return HSRV_E_NONE;
            }

            if (p_db_session->slow_path_fwd)
            {
                return HSRV_E_NONE;
            }

            hsrv_nat_session_oper(p_db_session, hsrv_nat_fwd_entry_del);

            hsrv_nat_session_fast_path_fwd_cnt_decrease(p_db_session);

            p_db_session->set_sai_status = GLB_NAT_SET_SAI_STATUS_INVALID;
            break;

        case CDB_OPER_SET:
            p_db_session = tbl_nat_session_get_nat_session(&p_session->key);
            HSRV_PTR_CHECK(p_db_session);
        
            switch(field_id)
            {
                case TBL_NAT_SESSION_FLD_SET_SAI_STATUS:
                {
                    if (p_db_session->slow_path_fwd)
                    {
                        return HSRV_E_NONE;
                    }
                
                    if (GLB_NAT_SET_SAI_AGAIN != p_db_session->set_sai_status)
                    {
                        return HSRV_E_FAIL;
                    }

                    if (p_nat_global->session_fast_fwd_cnt_dnstream + p_nat_global->session_fast_fwd_cnt_upstream >= NAT_SESSION_SPEC)
                    {
                        /*beyond hardware limit*/
                        p_db_session->set_sai_status = GLB_NAT_SET_SAI_STATUS_FAILED;
                        tbl_nat_session_set_nat_session_field(p_db_session, TBL_NAT_SESSION_FLD_SET_SAI_STATUS);
                        return HSRV_E_NONE;
                    }

                    ret = hsrv_nat_session_oper(p_db_session, hsrv_nat_fwd_entry_add);
                    if (ret)
                    {
                        p_db_session->set_sai_status = GLB_NAT_SET_SAI_STATUS_FAILED;
                        tbl_nat_session_set_nat_session_field(p_db_session, TBL_NAT_SESSION_FLD_SET_SAI_STATUS);
                    }
                    else
                    {
                        p_db_session->set_sai_status = GLB_NAT_SET_SAI_STATUS_SUCCESS;
                        hsrv_nat_session_fast_path_fwd_cnt_increase(p_db_session);
                    }
                    break;
                }
            }
            break;

        default:
            return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}
#endif


int32
hsrv_nat_start()
{
#ifdef TSINGMA
#ifndef OFPRODUCT
    tbl_nat_global_t *p_nat_global = NULL;

    /* ====nat global var init===== */
    p_nat_global = tbl_nat_global_get_nat_global();

    p_nat_global->aging_timer = ctc_task_add_timer(TASK_PRI_NORMAL, hsrv_nat_session_aging_timer_func, NULL, NAT_SESSION_AGING_TIMER_LEN);
    p_nat_global->timer_mark = 0;
#endif
#endif
    return HSRV_E_NONE;
}

