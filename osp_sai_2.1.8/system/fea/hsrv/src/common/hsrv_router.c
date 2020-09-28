/****************************************************************************
* $Id$
*  Centec neighbor information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-07-10
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "cdb_const.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_nexthop_group_define.h"
#include "gen/tbl_nexthop_group.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/

extern int32
hsrv_route_update_ecmp_rt_entry(tbl_iproute_node_t* p_rt_node, uint32 act_add);

/******************************************************************************
* Defines and Macros
******************************************************************************/

uint32
hsrv_router_ecmp_count_get_max()
{
    tbl_sys_spec_t  *p_sys_spec = NULL;
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis();
    if (NULL == p_chassis) {
        return 0;
    }

    type = p_chassis->cur_stm_mode;
    if (type < GLB_STM_MODE_MAX) {
        p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
        if(NULL != p_sys_spec) {
            #ifdef OFPRODUCT
                return p_sys_spec->ecmp_routes -GLB_OPENFLOW_SELECT_GROUP_MAX;
            #else
                return p_sys_spec->ecmp_routes;
            #endif
        }
    }

    return 0;
}

/***************************************************************************************************
 * Name         : hsrv_router_nexthop_sync 
 * Purpose      : load nexthop cdb to sai          
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
hsrv_router_nexthop_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_fea_nexthop_t   *p_fea_nexthop  = NULL;
    tbl_nexthop_t       *p_nexthop      = NULL;
    tbl_fea_nexthop_t   *p_fea_nh_check = NULL;
    tbl_rif_t           *p_rif          = NULL;
    sai_next_hop_api_t  *nexthop_api    = NULL;
    tbl_fea_nexthop_t fea_nexthop;
    tbl_rif_t rif;
    sai_attribute_t attr[3];
    sai_object_id_t nhid = 0;

    HSRV_NEXTHOP_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);
    sal_memset(&fea_nexthop, 0, sizeof(fea_nexthop));
    sal_memset(&rif, 0, sizeof(rif));
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NEXT_HOP,(void**)&nexthop_api));

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_nexthop = (tbl_nexthop_t*)p_tbl;
        HSRV_PTR_CHECK(p_nexthop);

        /* support nd modified by liwh for bug 47547, 2018-07-06 */
        /*if (NH_TYPE_IPV4 != p_nexthop->key.type)
        {
            return HSRV_E_NONE;
        }*/
        /* liwh end */

        sal_memcpy(rif.key.name, p_nexthop->key.ifname,sizeof(rif.key.name));
        p_rif = tbl_rif_get_rif(&rif.key);
        HSRV_PTR_CHECK(p_rif);

        sal_memset(attr, 0, sizeof(attr));
        attr[0].id = SAI_NEXT_HOP_ATTR_TYPE;
        attr[0].value.s32 = SAI_NEXT_HOP_IP;
        attr[1].id = SAI_NEXT_HOP_ATTR_IP;

        if (NH_TYPE_IPV4 == p_nexthop->key.type)
        {
            attr[1].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
            attr[1].value.ipaddr.addr.ip4 = sal_htonl(p_nexthop->key.u.ipv4.s_addr);
        }
        /* support nd modified by liwh for bug 47547, 2018-07-06 */
        else 
        {
            attr[1].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
            sal_memcpy(&attr[1].value.ipaddr.addr.ip6,  &p_nexthop->key.u.ipv6, GLB_IPV6_MAX_BYTELEN);
        }
        /* liwh end */

        attr[2].id = SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID;
        attr[2].value.oid = p_rif->rif_id;

        HSRV_IF_ERROR_RETURN(nexthop_api->create_next_hop(&nhid, 3, attr));

        sal_memcpy(&fea_nexthop.key, &p_nexthop->key, sizeof(fea_nexthop.key));
        fea_nexthop.nhid = nhid;

        p_fea_nh_check = tbl_fea_nexthop_get_fea_nexthop(&fea_nexthop.key);
        if (NULL != p_fea_nh_check)
        {
            tbl_fea_nexthop_del_fea_nexthop(&p_fea_nh_check->key);
        }

        HSRV_IF_ERROR_RETURN(tbl_fea_nexthop_add_fea_nexthop(&fea_nexthop));   
        break;
        
    case CDB_OPER_DEL:
        p_nexthop = (tbl_nexthop_t*)p_tbl;
        HSRV_PTR_CHECK(p_nexthop);

        /* support nd modified by liwh for bug 47547, 2018-07-06 */
        /*if (NH_TYPE_IPV4 != p_nexthop->key.type)
        {
            return HSRV_E_NONE;
        }*/
        /* liwh end */
        
        p_fea_nexthop = tbl_fea_nexthop_get_fea_nexthop(&p_nexthop->key);
        HSRV_PTR_CHECK(p_fea_nexthop);
        
        HSRV_IF_ERROR_RETURN(nexthop_api->remove_next_hop(p_fea_nexthop->nhid));
        HSRV_IF_ERROR_RETURN(tbl_fea_nexthop_del_fea_nexthop(&p_fea_nexthop->key));
        
        break;

    case CDB_OPER_SET:

        break;

    case CDB_OPER_GET:

        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_router_nexthop_group_set_ecmp_hash 
 * Purpose      : set nexthop group ecmp hash type to sai  
 * Input        : nhg
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_router_nexthop_group_set_ecmp_hash(tbl_nexthop_group_t* nhg)
{
    sai_next_hop_group_api_t   *nhg_api;
    sai_attribute_t             attr[2];    

    HSRV_NEXTHOP_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(nhg);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NEXT_HOP_GROUP,(void**)&nhg_api));

    sal_memset(attr, 0, sizeof(attr));

    attr[0].id          = SAI_NEXT_HOP_GROUP_ATTR_ECMP_HSAH;
    attr[1].id          = SAI_NEXT_HOP_GROUP_ATTR_ECMP_FAILOVER_EN;
    attr[1].value.booldata =  FALSE;
    if (GLB_ECMP_STATIC_LOAD_BALANCE == nhg->ecmp_hash)
    {
        attr[0].value.u32 = CTC_NH_ECMP_TYPE_STATIC;
    }
    else if (GLB_ECMP_DYNAMIC_LOAD_BALANCE == nhg->ecmp_hash)
    {
        attr[0].value.u32 = CTC_NH_ECMP_TYPE_DLB;
    }
    else if (GLB_ECMP_ROUND_ROBIN_LOAD_BALANCE == nhg->ecmp_hash)
    {
        attr[0].value.u32 = CTC_NH_ECMP_TYPE_RR;
    }
    else if (GLB_ECMP_SELF_HEALING_LOAD_BALANCE == nhg->ecmp_hash)
    {
        attr[0].value.u32 = CTC_NH_ECMP_TYPE_STATIC;
        attr[1].value.booldata =  TRUE;
    }

    HSRV_IF_ERROR_RETURN(nhg_api->set_next_hop_group_attribute(nhg->nhg_oid, attr));

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_router_nexthop_group_add 
 * Purpose      : load nexthop group to sai  
 * Input        : nhg
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_router_nexthop_group_add(tbl_nexthop_group_t* nhg)
{
    tbl_fea_nexthop_t   *p_fea_nh           = NULL;
    sai_next_hop_group_api_t    *nhg_api;
    sai_attribute_t             attr[4];    
    sai_object_id_t             nhg_oid;
    sai_object_list_t           obj_list; 
    sai_object_id_t             nh_obj[IP_ECMP_MAX_PATH];
    uint32                      nh_num = 0;
    uint32_t                    i = 0;
    /* GB only support 8 ecmp nexthops, modified by liwh for bug 45595, 2017-11-17 */
    uint32                      valid_nh_num = 0;
    /* liwh end */

    HSRV_NEXTHOP_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(nhg);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NEXT_HOP_GROUP,(void**)&nhg_api));
    HSRV_NEXTHOP_DEBUG("Nexthop group nexthop member number is %d\n, "
        "member is %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
        nhg->key.nh_num,
        nhg->key.nh[0].u.ipv4.s_addr, nhg->key.nh[1].u.ipv4.s_addr, nhg->key.nh[2].u.ipv4.s_addr, nhg->key.nh[3].u.ipv4.s_addr, 
        nhg->key.nh[4].u.ipv4.s_addr, nhg->key.nh[5].u.ipv4.s_addr, nhg->key.nh[6].u.ipv4.s_addr, nhg->key.nh[7].u.ipv4.s_addr, 
        nhg->key.nh[8].u.ipv4.s_addr, nhg->key.nh[9].u.ipv4.s_addr, nhg->key.nh[10].u.ipv4.s_addr, nhg->key.nh[11].u.ipv4.s_addr, 
        nhg->key.nh[12].u.ipv4.s_addr, nhg->key.nh[13].u.ipv4.s_addr, nhg->key.nh[14].u.ipv4.s_addr, nhg->key.nh[15].u.ipv4.s_addr

    );

    sal_memset(&obj_list, 0, sizeof(obj_list));
    sal_memset(attr, 0, sizeof(attr));
    sal_memset(nh_obj, 0, sizeof(nh_obj));
    nh_num = nhg->key.nh_num;

    attr[0].id          = SAI_NEXT_HOP_GROUP_ATTR_TYPE;
    attr[0].value.s32   = SAI_NEXT_HOP_GROUP_ECMP;

    for(i = 0; i < nh_num; i++)
    {
        p_fea_nh = tbl_fea_nexthop_get_fea_nexthop(&nhg->key.nh[i]);
        if (NULL == p_fea_nh)
        {
            continue;
        }
        else
        {
            nh_obj[i] = p_fea_nh->nhid;
            /* GB only support 8 ecmp nexthops, modified by liwh for bug 45595, 2017-11-17 */
            valid_nh_num++;
            /* liwh end */
        }
    }

    /* GB only support 8 ecmp nexthops, modified by liwh for bug 45595, 2017-11-17 */
    if (0 == valid_nh_num)
    {
        return PM_E_NONE;
    }
    /* liwh end */

    obj_list.list           = nh_obj;
    /* GB only support 8 ecmp nexthops, modified by liwh for bug 45595, 2017-11-17 */
    //obj_list.count           = nh_num;
    obj_list.count          = valid_nh_num;
    /* liwh end */
    attr[1].id              = SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST;   
    sal_memcpy(&attr[1].value.objlist, &obj_list, sizeof(obj_list));

    attr[2].id          = SAI_NEXT_HOP_GROUP_ATTR_ECMP_HSAH;
    attr[3].id          = SAI_NEXT_HOP_GROUP_ATTR_ECMP_FAILOVER_EN;
    attr[3].value.booldata =  FALSE;
    if (GLB_ECMP_STATIC_LOAD_BALANCE == nhg->ecmp_hash)
    {
        attr[2].value.u32 = CTC_NH_ECMP_TYPE_STATIC;
    }
    else if (GLB_ECMP_DYNAMIC_LOAD_BALANCE == nhg->ecmp_hash)
    {
        attr[2].value.u32 = CTC_NH_ECMP_TYPE_DLB;
    }
    else if (GLB_ECMP_ROUND_ROBIN_LOAD_BALANCE == nhg->ecmp_hash)
    {
        attr[2].value.u32 = CTC_NH_ECMP_TYPE_RR;
    }
    else if (GLB_ECMP_SELF_HEALING_LOAD_BALANCE == nhg->ecmp_hash)
    {
        attr[2].value.u32 = CTC_NH_ECMP_TYPE_STATIC;
        attr[3].value.booldata =  TRUE;
    }

    HSRV_IF_ERROR_RETURN(nhg_api->create_next_hop_group(&nhg_oid, 4, attr));
    HSRV_NEXTHOP_DEBUG("create nexthop group success, oid is %ld", nhg_oid);


    /* Save it to cdb fea tbl */
    nhg->nhg_oid = nhg_oid;
    HSRV_IF_ERROR_RETURN(tbl_nexthop_group_set_nexthop_group_field(nhg, TBL_NEXTHOP_GROUP_FLD_NHG_OID));
      
    return PM_E_NONE;
}



/***************************************************************************************************
 * Name         : hsrv_router_nexthop_group_del
 * Purpose      : delete nexthop group to sai
 * Input        : p_rt_node
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_router_nexthop_group_del(tbl_nexthop_group_t* nhg)
{

    sai_next_hop_group_api_t    *nhg_api;

    HSRV_NEXTHOP_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(nhg);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NEXT_HOP_GROUP,(void**)&nhg_api));
    HSRV_NEXTHOP_DEBUG("Nexthop group member number is %d, nh_oid is %"PRIu64"", nhg->key.nh_num, nhg->nhg_oid);

    HSRV_NEXTHOP_DEBUG("Nexthop group nexthop member number is %d\n, "
        "member is %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n", 
        nhg->key.nh_num,
        nhg->key.nh[0].u.ipv4.s_addr, nhg->key.nh[1].u.ipv4.s_addr, nhg->key.nh[2].u.ipv4.s_addr, nhg->key.nh[3].u.ipv4.s_addr, 
        nhg->key.nh[4].u.ipv4.s_addr, nhg->key.nh[5].u.ipv4.s_addr, nhg->key.nh[6].u.ipv4.s_addr, nhg->key.nh[7].u.ipv4.s_addr, 
        nhg->key.nh[8].u.ipv4.s_addr, nhg->key.nh[9].u.ipv4.s_addr, nhg->key.nh[10].u.ipv4.s_addr, nhg->key.nh[11].u.ipv4.s_addr, 
        nhg->key.nh[12].u.ipv4.s_addr, nhg->key.nh[13].u.ipv4.s_addr, nhg->key.nh[14].u.ipv4.s_addr, nhg->key.nh[15].u.ipv4.s_addr

    );

    HSRV_IF_ERROR_RETURN(nhg_api->remove_next_hop_group(nhg->nhg_oid));
      
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_router_nexthop_group_sync
 * Purpose      : syc nexthop group
 * Input        : 
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_router_nexthop_group_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_nexthop_group_t* p_nhg = NULL;
    tbl_nexthop_group_t* p_nhg_db = NULL;
    
    HSRV_NEXTHOP_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    switch (oper)
    {
    case CDB_OPER_ADD:
        break;
        
    case CDB_OPER_DEL:
        p_nhg = p_tbl;
        p_nhg_db = tbl_nexthop_group_get_nexthop_group(&p_nhg->key);
        HSRV_PTR_CHECK(p_nhg_db);
        HSRV_IF_ERROR_RETURN(hsrv_router_nexthop_group_del(p_nhg_db));
        break;

    case CDB_OPER_SET:
        p_nhg = p_tbl;
        switch(field_id)
        {
            case TBL_NEXTHOP_GROUP_FLD_NH_GROUP:
                break;
            default:
                break;
        }
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_router_nexthop_group_sync_after
 * Purpose      : syc nexthop group after db is stored.
 * Input        : 
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_router_nexthop_group_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_nexthop_group_t* p_nhg = NULL;
    
    HSRV_NEXTHOP_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_nhg = p_tbl;
        HSRV_IF_ERROR_RETURN(hsrv_router_nexthop_group_add(p_nhg));
        break;
        
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_nhg = p_tbl;
        switch(field_id)
        {
        case TBL_NEXTHOP_GROUP_FLD_NH_GROUP:
            break;
        case TBL_NEXTHOP_GROUP_FLD_ECMP_HASH:
            HSRV_IF_ERROR_RETURN(hsrv_router_nexthop_group_set_ecmp_hash(p_nhg));
            break;
        default:
            break;
        }
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return PM_E_NONE;
}


int32
_fea_nexthop_cmd_read_session_all(cfg_cmd_para_t *para)
{
    field_parse_t *p_field  = NULL;
    FILE          *fp       = NULL;    
    field_parse_t field;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_fea_nexthop_iterate((TBL_ITER_CB_FUNC)tbl_fea_nexthop_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
fea_nexthop_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_GET:
        _fea_nexthop_cmd_read_session_all(para);
        break;

    default:
        break;
    }

    return PM_E_NONE;
}



