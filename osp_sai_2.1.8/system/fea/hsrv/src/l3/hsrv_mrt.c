/****************************************************************************
* $Id$
*  Centec mrt information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2018-03-15
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "gen/tbl_mrt_define.h"
#include "gen/tbl_mrt.h"
#include "gen/tbl_ipmc_intf_define.h"
#include "gen/tbl_ipmc_intf.h"
#include "ctc_api.h"
#include "hsrv_mrt.h"
#include "ctc_task.h"

ctc_task_t *g_p_mrt_stats_scan_timer = NULL; 

/*add by zhw for mc*/
int32
hsrv_mrt_add_entry(tbl_mrt_t *p_mrt)
{
    sai_ipmc_api_t     *mrt_api          = NULL;
	sai_next_hop_group_api_t     *nhg_api          = NULL;
	tbl_interface_t     *p_if = NULL;
	sai_object_id_t    nhg_oid = 0;
	sai_object_id_t    next_rif_oid[GLB_IPV4_MROUTE_O_MEMBER_MAX] = {0};
    sai_attribute_t attr[3];
    tbl_rif_t       *rif = NULL;
	tbl_rif_key_t   rif_key;
	sai_ipmc_entry_t  mrt_entry;
	int32 oifindex = 0;
	int32 id = 0;
	uint32 no_oif = TRUE;
	sai_status_t rc;


    /*1. debug out infor*/
    HSRV_IPMC_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_mrt);

    HSRV_IPMC_DEBUG("mrt add, vrf %d, source %x, group %x\n"
        "    rpf_if %d, nhg_oid %x\n",
        p_mrt->key.vrf_id, p_mrt->key.source.s_addr, p_mrt->key.group.s_addr,
        p_mrt->rpf_ifindex, p_mrt->nhg_oid
        );

	HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NEXT_HOP_GROUP, (void**)&nhg_api));
	HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_IPMC, (void**)&mrt_api));	

	/*1, get ipmc nexthop group info and set to sdk*/

	attr[0].id = SAI_NEXT_HOP_GROUP_ATTR_TYPE;
	attr[0].value.oid = SAI_NEXT_HOP_GROUP_L3MC;
	attr[1].id = SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST;
	attr[1].value.objlist.count = 0;
	attr[1].value.objlist.list = next_rif_oid;
	
	GLB_BMP_CHECK_ALL_ZERO(p_mrt->oif_index_bmp,no_oif);
	if(!no_oif)
	{
	    GLB_BMP_ITER_BEGIN(p_mrt->oif_index_bmp, oifindex)
	    {
	        if(GLB_BMP_ISSET(p_mrt->oif_index_bmp, oifindex))
	        {
	            p_if = tbl_interface_get_interface_by_ifindex(oifindex);
				if(p_if)
				{
					HSRV_IPMC_DEBUG("    mrt out if member %s\n", p_if->key.name);
					
					sal_memset(&rif_key, 0, sizeof(rif_key));
					sal_memcpy(rif_key.name, p_if->key.name, sizeof(p_if->key.name));
					rif = tbl_rif_get_rif(&rif_key);
					if(NULL == rif)
					{
					    HSRV_IPMC_DEBUG( "rif not exit for interface %s!\n", p_if->key.name);
						continue;
					}
					else   
					{
						HSRV_IPMC_DEBUG( "mrt add member rif_oid %x\n",rif->rif_id);
					    next_rif_oid[id] = rif->rif_id;
						id ++;
					}
				}   
			    else
			    {
			        log_sys(M_MOD_IPMC, E_WARNING, "pif not exit for interface %d!\n", oifindex);
					continue;
   			    }  
   	            
	    	}
	    }
		
		GLB_BMP_ITER_END(p_mrt->oif_index_bmp, oifindex);
	}
	else
	{
	    log_sys(M_MOD_IPMC, E_ERROR, "There are no out member interface for ipmc group !\n");
		return PM_E_NOT_EXIST;
	}
	attr[1].value.objlist.count = id;
	/*create ipmc entry in asic, and return nhgoid to us*/
	HSRV_IF_ERROR_RETURN(nhg_api->create_next_hop_group(&nhg_oid, 2, attr));
	HSRV_IPMC_DEBUG("    mrt nhgid  %llx \n", nhg_oid);

    /*2, get ipmc infor and set to sdk*/
    sal_memset(&mrt_entry, 0, sizeof(mrt_entry));
	mrt_entry.vrf_id =  CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, p_mrt->key.vrf_id);
	mrt_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
	mrt_entry.destination.addr.ip4 = sal_ntohl(p_mrt->key.group.s_addr);
	mrt_entry.source.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
	mrt_entry.source.addr.ip4 = sal_ntohl(p_mrt->key.source.s_addr);
	if(0 == mrt_entry.source.addr.ip4)
	{
	    mrt_entry.type = SAI_IPMC_TYPE_XG;
	}
	else
	{
	    mrt_entry.type = SAI_IPMC_TYPE_SG;
	}

	sal_memset(&rif_key, 0, sizeof(rif_key));
	p_if = tbl_interface_get_interface_by_ifindex(p_mrt->rpf_ifindex);
	if(NULL == p_if)
	{
		log_sys(M_MOD_IPMC, E_ERROR, "rpfif not exit for interface %d!\n",p_mrt->rpf_ifindex);
		return PM_E_NOT_EXIST;
	}
	sal_memcpy(rif_key.name, p_if->key.name, sizeof(p_if->key.name));
	rif = tbl_rif_get_rif(&rif_key);
	if(NULL == rif)
	{
	    log_sys(M_MOD_IPMC, E_ERROR, "rpfif not exit for interface %s!\n", rif->key.name);
		return PM_E_NOT_EXIST;
	}
	HSRV_IPMC_DEBUG("mrt add  mrt rif_id  %x \n", rif->rif_id);
	attr[0].id = SAI_IPMC_ATTR_NEXT_HOP_GROUP_ID;
	attr[0].value.oid = nhg_oid;
	attr[1].id = SAI_IPMC_ATTR_RPF_ROUTER_INTERFACE;
	attr[1].value.oid = rif->rif_id;
	HSRV_IF_ERROR_RETURN(mrt_api->create_ipmc_entry(&mrt_entry, 2, attr));

	p_mrt->nhg_oid = nhg_oid;
	tbl_mrt_set_mrt_field(p_mrt, TBL_MRT_FLD_NHG_OID);
	HSRV_IPMC_DEBUG("mrt add finish  nhoid %llx \n",p_mrt->nhg_oid);
	
    return HSRV_E_NONE;
}

int32
hsrv_mrt_remove_entry(tbl_mrt_t* p_mrt)
{
	sai_ipmc_api_t * mrt_api = NULL;
	sai_next_hop_group_api_t* nhg_api = NULL;
	sai_ipmc_entry_t mrt_entry;
	tbl_mrt_t* p_local_mrt = NULL;
	tbl_mrt_key_t *p_mrt_key;
	sai_status_t rc;

    HSRV_PTR_CHECK(p_mrt);
	HSRV_IPMC_DEBUG("Enter into %s. \n", __FUNCTION__);
	HSRV_IPMC_DEBUG("mrt remove entry,vrfif =%d, group %x, source %x",
		p_mrt->key.vrf_id, p_mrt->key.group.s_addr, p_mrt->key.source.s_addr);

	HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_IPMC, (void**)&mrt_api));
	HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NEXT_HOP_GROUP, (void**)&nhg_api));

	sal_memset(&mrt_entry, 0, sizeof(mrt_entry));
	mrt_entry.vrf_id =  CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, p_mrt->key.vrf_id);
	mrt_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
	mrt_entry.destination.addr.ip4 = sal_ntohl(p_mrt->key.group.s_addr);
	mrt_entry.source.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
	mrt_entry.source.addr.ip4 = sal_ntohl(p_mrt->key.source.s_addr);
	if(0 == mrt_entry.source.addr.ip4)
	{
	    mrt_entry.type = SAI_IPMC_TYPE_XG;
	}
	else
	{
	    mrt_entry.type = SAI_IPMC_TYPE_SG;
	}

	p_mrt_key = &p_mrt->key;
    p_local_mrt = tbl_mrt_get_mrt(p_mrt_key);
	HSRV_PTR_CHECK(p_local_mrt);
	
    HSRV_IPMC_DEBUG("mrt set, local entry,  vrf %d, source %x, group %x\n"
        "    rpf_if %d, nhg_oid %llx\n",
        p_local_mrt->key.vrf_id, p_local_mrt->key.source.s_addr, p_local_mrt->key.group.s_addr,
        p_local_mrt->rpf_ifindex, p_local_mrt->nhg_oid);

	HSRV_IF_ERROR_RETURN(mrt_api->remove_ipmc_entry(&mrt_entry));
	HSRV_IF_ERROR_RETURN(nhg_api->remove_next_hop_group(p_local_mrt->nhg_oid));

    return HSRV_E_NONE;
}

int32
hsrv_mrt_sai_add_entry(tbl_mrt_t *p_mrt) 
{
    sai_status_t rc = 0;
    sai_next_hop_group_api_t*   nexthop_group_api = NULL;
    sai_ipmc_api_t*             ipmc_api = NULL;
    sai_object_id_t             nexthop_group_object;
    sai_object_id_t             nexthop_list_obj[64] = {0};
    sai_attribute_t             attr[3] = {};
    tbl_rif_key_t               rif_key = {};
    tbl_rif_t                   *rif = NULL;
    sai_ipmc_entry_t            ipmc_entry;
    int32                       oifindex  = 0;
    int32                       idx = 0;
    uint32                      oif_index_bmp_zero = TRUE;

    HSRV_IPMC_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NEXT_HOP_GROUP, (void**)&nexthop_group_api));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_IPMC, (void**)&ipmc_api));

    /* 1. create ipmc group */
    attr[0].id = SAI_NEXT_HOP_GROUP_ATTR_TYPE;
    attr[0].value.oid = SAI_NEXT_HOP_GROUP_L3MC;

    attr[1].id =  SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST;
    attr[1].value.objlist.count = 0;
    attr[1].value.objlist.list = nexthop_list_obj;

    GLB_BMP_CHECK_ALL_ZERO(p_mrt->oif_index_bmp, oif_index_bmp_zero);
    if(!oif_index_bmp_zero)
    {
        GLB_BMP_ITER_BEGIN(p_mrt->oif_index_bmp, oifindex)
        {
            if (GLB_BMP_ISSET(p_mrt->oif_index_bmp, oifindex))
            {
                sal_memset(&rif_key, 0, sizeof(rif_key));
                tbl_interface_get_name_by_ifindex(oifindex,rif_key.name,sizeof(rif_key.name));
                rif = tbl_rif_get_rif(&rif_key);
                nexthop_list_obj[idx] = rif->rif_id;
                idx++;
            }
        }
        attr[1].value.objlist.count = idx;
        GLB_BMP_ITER_END(p_mrt->oif_index_bmp, oifindex);
    }

    rc = nexthop_group_api->create_next_hop_group(&nexthop_group_object, 2, attr);
    if (!rc)
    {
        p_mrt->nhg_oid = nexthop_group_object;
    }

    /* 2. create ipmc entry */
    sal_memset(&ipmc_entry, 0, sizeof(ipmc_entry));
    ipmc_entry.vrf_id = tbl_fea_global_get_fea_global()->router_id;
    ipmc_entry.type = SAI_IPMC_TYPE_XG;
    ipmc_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    ipmc_entry.destination.addr.ip4 = sal_ntohl(p_mrt->key.group.s_addr);
    ipmc_entry.source.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    ipmc_entry.source.addr.ip4 = sal_ntohl(p_mrt->key.source.s_addr);
    if (0 == ipmc_entry.source.addr.ip4)
    {
        ipmc_entry.type = SAI_IPMC_TYPE_XG;
    }
    else
    {
        ipmc_entry.type = SAI_IPMC_TYPE_SG;
    }

    attr[0].id = SAI_IPMC_ATTR_PACKET_ACTION;
    attr[0].value.s32 = SAI_PACKET_ACTION_FORWARD;

    attr[1].id = SAI_IPMC_ATTR_NEXT_HOP_GROUP_ID;
    attr[1].value.oid = p_mrt->nhg_oid;

    sal_memset(&rif_key, 0, sizeof(rif_key));
    tbl_interface_get_name_by_ifindex(p_mrt->rpf_ifindex,rif_key.name,sizeof(rif_key.name));
    rif = tbl_rif_get_rif(&rif_key);
    attr[2].id = SAI_IPMC_ATTR_RPF_ROUTER_INTERFACE;
    attr[2].value.oid = rif->rif_id;

    rc = ipmc_api->create_ipmc_entry(&ipmc_entry, 3, attr);

    return rc;
}

int32
hsrv_mrt_sai_del_entry(tbl_mrt_t *p_mrt) 
{
    sai_status_t rc = 0;
    sai_ipmc_api_t*             ipmc_api = NULL;
    sai_next_hop_group_api_t*   nexthop_group_api = NULL;
    sai_ipmc_entry_t            ipmc_entry;


    HSRV_IPMC_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_IPMC, (void**)&ipmc_api));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NEXT_HOP_GROUP, (void**)&nexthop_group_api));

    sal_memset(&ipmc_entry, 0, sizeof(ipmc_entry));
    ipmc_entry.vrf_id = tbl_fea_global_get_fea_global()->router_id;
    ipmc_entry.type = SAI_IPMC_TYPE_XG;
    ipmc_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    ipmc_entry.destination.addr.ip4 = sal_ntohl(p_mrt->key.group.s_addr);
    ipmc_entry.source.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    ipmc_entry.source.addr.ip4 = sal_ntohl(p_mrt->key.source.s_addr);
    if (0 == ipmc_entry.source.addr.ip4)
    {
        ipmc_entry.type = SAI_IPMC_TYPE_XG;
    }
    else
    {
        ipmc_entry.type = SAI_IPMC_TYPE_SG;
    }

    rc = ipmc_api->remove_ipmc_entry(&ipmc_entry);
    rc = nexthop_group_api->remove_next_hop_group(p_mrt->nhg_oid);

    return rc;
}

int32
hsrv_mrt_set_rpf(tbl_mrt_t *p_mrt) 
{
    sai_status_t rc = 0;

    return rc;
}

int32
hsrv_mrt_set_oif(tbl_mrt_t *p_mrt) 
{
    sai_status_t    rc = 0;
    sai_next_hop_group_api_t*   nexthop_group_api = NULL;
    sai_ipmc_api_t*             ipmc_api = NULL;
    sai_ipmc_entry_t            ipmc_entry;
    sai_attribute_t             attr[1] = {};
    sai_object_id_t             nexthop_list_obj[64] = {0};
    tbl_rif_key_t               rif_key = {};
    tbl_rif_t                  *rif = NULL;
    int32                       oifindex  = 0;
    int32                       idx = 0;
    uint32                      oif_index_bmp_zero = TRUE;
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_NEXT_HOP_GROUP, (void**)&nexthop_group_api));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_IPMC, (void**)&ipmc_api));

    attr[0].id =  SAI_IPMC_ATTR_IPMC_MEMBER;
    attr[0].value.objlist.count = 0;
    attr[0].value.objlist.list = nexthop_list_obj;

    GLB_BMP_CHECK_ALL_ZERO(p_mrt->oif_index_bmp, oif_index_bmp_zero);
    if(!oif_index_bmp_zero)
    {
        GLB_BMP_ITER_BEGIN(p_mrt->oif_index_bmp, oifindex)
        {
            if (GLB_BMP_ISSET(p_mrt->oif_index_bmp, oifindex))
            {
                sal_memset(&rif_key, 0, sizeof(rif_key));
                tbl_interface_get_name_by_ifindex(oifindex,rif_key.name,sizeof(rif_key.name));
                rif = tbl_rif_get_rif(&rif_key);
                nexthop_list_obj[idx] = rif->rif_id;
                idx++;
            }
        }
        attr[0].value.objlist.count = idx;
        GLB_BMP_ITER_END(p_mrt->oif_index_bmp, oifindex);
    }

    sal_memset(&ipmc_entry, 0, sizeof(ipmc_entry));
    ipmc_entry.vrf_id = tbl_fea_global_get_fea_global()->router_id;
    ipmc_entry.type = SAI_IPMC_TYPE_XG;
    ipmc_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    ipmc_entry.destination.addr.ip4 = sal_ntohl(p_mrt->key.group.s_addr);
    ipmc_entry.source.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    ipmc_entry.source.addr.ip4 = sal_ntohl(p_mrt->key.source.s_addr);
    if (0 == ipmc_entry.source.addr.ip4)
    {
        ipmc_entry.type = SAI_IPMC_TYPE_XG;
    }
    else
    {
        ipmc_entry.type = SAI_IPMC_TYPE_SG;
    }

    rc = ipmc_api->set_ipmc_entry_attribute(&ipmc_entry, attr);

    return rc;
}

#if 0
int32
hsrv_mrt_del_member(tbl_mrt_t *p_mrt, uint32 ifindex)
{
    sai_ipmc_api_t *mrt_api = NULL;
	sai_ipmc_entry_t mrt_entry;
	int rc;
	tbl_interface_t     *p_if = NULL;
    tbl_rif_t       *rif = NULL;
	tbl_rif_key_t   rif_key;
	
    HSRV_PTR_CHECK(p_mrt);
	HSRV_IPMC_DEBUG("Enter into %s. \n", __FUNCTION__);
	HSRV_IPMC_DEBUG("mrt del member,vrfif =%d, group %x, source %x",
		p_mrt->key.vrf_id, p_mrt->key.group.s_addr, p_mrt->key.source.s_addr);
	
	HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_IPMC, (void**)&mrt_api));

	sal_memset(&mrt_entry, 0, sizeof(mrt_entry));
	mrt_entry.vrf_id =  CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, p_mrt->key.vrf_id);
	mrt_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
	mrt_entry.destination.addr.ip4 = sal_ntohl(p_mrt->key.group.s_addr);
	mrt_entry.source.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
	mrt_entry.source.addr.ip4 = sal_ntohl(p_mrt->key.source.s_addr);
	if(0 == mrt_entry.source.addr.ip4)
	{
	    mrt_entry.type = SAI_IPMC_TYPE_XG;
	}
	else
	{
	    mrt_entry.type = SAI_IPMC_TYPE_SG;
	}

	p_if = tbl_interface_get_interface_by_ifindex(ifindex);
	if(p_if)
	{
		HSRV_IPMC_DEBUG("	 mrt add member if %s\n", p_if->key.name);
		sal_memset(&rif_key, 0, sizeof(rif_key));
		sal_memcpy(rif_key.name, p_if->key.name, sizeof(p_if->key.name));
		rif = tbl_rif_get_rif(&rif_key);
		if(NULL == rif)
		{
			HSRV_IPMC_DEBUG( "rif not exit for interface %s!\n", p_if->key.name);
			return HSRV_E_INVALID_PARAM;
		}
		else   
		{
			HSRV_IPMC_DEBUG( "mrt del member rif_oid %x\n",rif->rif_id);
			HSRV_IF_ERROR_RETURN(mrt_api->del_ipmc_member(&mrt_entry, rif->rif_id));	
		}
	}	
	else
	{
		log_sys(M_MOD_IPMC, E_WARNING, "pif not exit for interface %d!\n", ifindex);
		return HSRV_E_INVALID_PARAM;
	} 
    return PM_E_NONE;
}

int32
hsrv_mrt_add_member(tbl_mrt_t *p_mrt, uint32 ifindex)
{
    sai_ipmc_api_t *mrt_api = NULL;
	sai_ipmc_entry_t mrt_entry;
	int rc;
	tbl_interface_t     *p_if = NULL;
    tbl_rif_t       *rif = NULL;
	tbl_rif_key_t   rif_key;
	
    HSRV_PTR_CHECK(p_mrt);
	HSRV_IPMC_DEBUG("Enter into %s. \n", __FUNCTION__);
	HSRV_IPMC_DEBUG("mrt add member,vrfif =%d, group %x, source %x",
		p_mrt->key.vrf_id, p_mrt->key.group.s_addr, p_mrt->key.source.s_addr);
	
	HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_IPMC, (void**)&mrt_api));

	sal_memset(&mrt_entry, 0, sizeof(mrt_entry));
	mrt_entry.vrf_id =  CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, p_mrt->key.vrf_id);
	mrt_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
	mrt_entry.destination.addr.ip4 = sal_ntohl(p_mrt->key.group.s_addr);
	mrt_entry.source.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
	mrt_entry.source.addr.ip4 = sal_ntohl(p_mrt->key.source.s_addr);
	if(0 == mrt_entry.source.addr.ip4)
	{
	    mrt_entry.type = SAI_IPMC_TYPE_XG;
	}
	else
	{
	    mrt_entry.type = SAI_IPMC_TYPE_SG;
	}

	p_if = tbl_interface_get_interface_by_ifindex(ifindex);
	if(p_if)
	{
		HSRV_IPMC_DEBUG("	 mrt add member if %s\n", p_if->key.name);
		sal_memset(&rif_key, 0, sizeof(rif_key));
		sal_memcpy(rif_key.name, p_if->key.name, sizeof(p_if->key.name));
		rif = tbl_rif_get_rif(&rif_key);
		if(NULL == rif)
		{
			HSRV_IPMC_DEBUG( "rif not exit for interface %s!\n", p_if->key.name);
			return HSRV_E_INVALID_PARAM;
		}
		else   
		{
			HSRV_IPMC_DEBUG( "mrt add member rif_oid %x\n",rif->rif_id);
			HSRV_IF_ERROR_RETURN(mrt_api->add_ipmc_member(&mrt_entry, rif->rif_id));	
		}
	}	
	else
	{
		log_sys(M_MOD_IPMC, E_WARNING, "pif not exit for interface %d!\n", ifindex);
		return HSRV_E_INVALID_PARAM;
	} 
    return PM_E_NONE;
}


int32
hsrv_mrt_set_pim_oif(tbl_mrt_t* p_mrt, tbl_mrt_t* p_db_mrt)
{
    int i;

    /*1. debug out infor*/
	sai_status_t rc;
    HSRV_IPMC_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_mrt);

    HSRV_IPMC_DEBUG("mrt set, vrf %d, source %x, group %x\n"
        "    rpf_if %d, nhg_oid %llx\n",
        p_mrt->key.vrf_id, p_mrt->key.source.s_addr, p_mrt->key.group.s_addr,
        p_mrt->rpf_ifindex, p_mrt->nhg_oid);

    /*add new member*/
	for ((i) = 0; (i) < GLB_BMP_BITS((p_mrt->oif_index_bmp)); (i)++)
    {
        if((GLB_BMP_ISSET(p_db_mrt->oif_index_bmp, i)) && (!GLB_BMP_ISSET(p_mrt->oif_index_bmp, i)))
        {
			/*old exist, new not exist, del the member*/
			HSRV_IF_ERROR_RETURN(hsrv_mrt_del_member(p_mrt, i));
        }
        else if((!GLB_BMP_ISSET(p_db_mrt->oif_index_bmp, i)) && (GLB_BMP_ISSET(p_mrt->oif_index_bmp, i)))
        {
			/*old exist, new not exist, del the member*/
			HSRV_IF_ERROR_RETURN(hsrv_mrt_add_member(p_mrt, i));
        }
    }        
	return HSRV_E_NONE;
}
#endif
int32
hsrv_mrt_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_mrt_t *        p_mrt = NULL;
    tbl_mrt_t * p_db_mrt = NULL;
	
    HSRV_IPMC_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    p_mrt = (tbl_mrt_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        HSRV_IPMC_DEBUG("MRT add entry");
        HSRV_IF_ERROR_RETURN(hsrv_mrt_add_entry(p_mrt));
        break;

    case CDB_OPER_DEL:
        HSRV_IPMC_DEBUG("MRT del entry");
        HSRV_IF_ERROR_RETURN(hsrv_mrt_remove_entry(p_mrt));
        break;

    case CDB_OPER_SET:
        switch (field_id)
        {
        /*
        case TBL_MRT_FLD_RPF_IFINDEX:
            HSRV_IF_ERROR_RETURN(hsrv_mrt_set_rpf(p_mrt));
            break;
          */  
        case TBL_MRT_FLD_OIF_INDEX_BMP:
			HSRV_IPMC_DEBUG("MRT set entry");
            HSRV_IF_ERROR_RETURN(hsrv_mrt_set_oif(p_mrt));
            break;

        default:
            break;
        }

    default:
        return HSRV_E_INVALID_PARAM;
    }
    return HSRV_E_NONE;
}

int32
hsrv_ipmc_set_mcast_admin_state(tbl_ipmc_intf_t* p_ipmc_inft, uint32 isadd)
{
    sai_router_interface_api_t* rif_api = NULL;  
    tbl_rif_t       *p_rif  = NULL;
    sai_attribute_t attr;
    tbl_rif_t       rif; 
   
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_ipmc_inft);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE,(void**)&rif_api));

    sal_memcpy(rif.key.name, p_ipmc_inft->key.name, sizeof(p_ipmc_inft->key.name));   
    p_rif = tbl_rif_get_rif(&rif.key);
    HSRV_PTR_CHECK(p_rif);

    attr.id = SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4MCAST_STATE;
    attr.value.booldata = (isadd) ? TRUE : FALSE;
    HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(p_rif->rif_id, &attr));

#if defined GOLDENGATE
    attr.id = SAI_ROUTER_INTERFACE_ATTR_IGMP_SNOOP_EN;
    attr.value.booldata = (isadd) ? TRUE : FALSE;
    HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(p_rif->rif_id, &attr));
#endif

    return HSRV_E_NONE;
}

int32
hsrv_ipmc_intf_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_ipmc_intf_t *   p_ipmc_intf = NULL;
    
    HSRV_IPMC_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    p_ipmc_intf = (tbl_ipmc_intf_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        HSRV_IF_ERROR_RETURN(hsrv_ipmc_set_mcast_admin_state(p_ipmc_intf, TRUE));
        break;

    case CDB_OPER_DEL:
        HSRV_IF_ERROR_RETURN(hsrv_ipmc_set_mcast_admin_state(p_ipmc_intf, FALSE));
        break;

    case CDB_OPER_SET:
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }
    return HSRV_E_NONE;
}
		
int32
hsrv_mrt_update_stats(tbl_mrt_t *p_mrt)
{
	sai_ipmc_api_t *mrt_api = NULL;
	sai_attribute_t attr[2];
	sai_ipmc_entry_t mrt_entry;
	int rc;
			
	HSRV_PTR_CHECK(p_mrt);
	HSRV_IPMC_DEBUG("Enter into %s. \n", __FUNCTION__);
	HSRV_IPMC_DEBUG("mrt update entry stats,vrfif =%d, group %x, source %x",
	p_mrt->key.vrf_id, p_mrt->key.group.s_addr, p_mrt->key.source.s_addr);
	
	sal_memset(attr, 0, sizeof(attr));
	attr[0].id = SAI_IPMC_COUNTER_ATTR_COUNTER_PACKETS;
	attr[1].id = SAI_IPMC_COUNTER_ATTR_COUNTER_BYTES;
	HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_IPMC,(void**)&mrt_api));
			
	sal_memset(&mrt_entry, 0, sizeof(mrt_entry));
	mrt_entry.vrf_id =	CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, p_mrt->key.vrf_id);
	mrt_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
	mrt_entry.destination.addr.ip4 = sal_ntohl(p_mrt->key.group.s_addr);
	mrt_entry.source.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
	mrt_entry.source.addr.ip4 = sal_ntohl(p_mrt->key.source.s_addr);
	if(0 == mrt_entry.source.addr.ip4)
	{
		mrt_entry.type = SAI_IPMC_TYPE_XG;
	}
	else
	{
		mrt_entry.type = SAI_IPMC_TYPE_SG;
	}
		
	HSRV_IF_ERROR_RETURN(mrt_api->get_ipmc_entry_attribute(&mrt_entry, 2, attr));
	HSRV_IPMC_DEBUG("mrt update entry stats, packet %llu, bytes %llu \n",
		attr[0].value.u64, attr[1].value.u64);
		
	p_mrt->packet_match = attr[0].value.u64;
	p_mrt->byte_match = attr[1].value.u64;
	tbl_mrt_set_mrt_field(p_mrt,TBL_MRT_FLD_PACKET_MATCH);
	tbl_mrt_set_mrt_field(p_mrt,TBL_MRT_FLD_BYTE_MATCH);
		
	return PM_E_NONE;
}

int32
hsrv_mrt_update_stats_all(void* args)
{
	tbl_mrt_iterate((TBL_ITER_CB_FUNC)hsrv_mrt_update_stats, args);
	g_p_mrt_stats_scan_timer = ctc_task_add_timer(TASK_PRI_NORMAL, hsrv_mrt_update_stats_all, NULL, GLB_MRT_POLLING_STATS_INTERVAL);
			
	return PM_E_NONE;
}

int32
hsrv_mrt_start()
{
    CTC_TASK_STOP_TIMER(g_p_mrt_stats_scan_timer);
    g_p_mrt_stats_scan_timer = ctc_task_add_timer(TASK_PRI_NORMAL, hsrv_mrt_update_stats_all, NULL, GLB_MRT_POLLING_STATS_INTERVAL);
	return PM_E_NONE;
}

