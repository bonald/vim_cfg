#include <sai.h>
#include <sal.h>
#include "ctc_api.h"
#include <ctc_sai_fdb.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_notifier_call.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_port.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_l2mc.h>
#include <ctc_sai_vlan.h>
#include <ctc_sai_nexthop_group.h>
#include <ctc_sai_routerintf.h>
#include <ctc_sai_notifier_call.h>


stbl_ipmc_t*
ctc_sai_ipmc_db_get(const stbl_ipmc_key_t* p_ipmc_entry)
{
    stbl_ipmc_key_t key;

    sal_memcpy(&key, p_ipmc_entry, sizeof(stbl_ipmc_key_t));
    
    return stbl_ipmc_get_ipmc(&key);
}

sai_status_t
ctc_sai_ipmc_db_add(const stbl_ipmc_t* ipmc_entry)
{
    stbl_ipmc_t ipmc;

    /* 1. lookup entry exist */
    if (ctc_sai_ipmc_db_get(&ipmc_entry->key))
    {
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    sal_memset(&ipmc, 0, sizeof(ipmc));

    /* 3. evaluate db entry */
    sal_memcpy(&ipmc, ipmc_entry, sizeof(stbl_ipmc_t));

    /* 4. add to db */
    stbl_ipmc_add_ipmc(&ipmc);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_ipmc_db_del(const stbl_ipmc_t* ipmc_entry)
{
    stbl_ipmc_t ipmc;
    
    /* 1. lookup entry exist */
    if (!ctc_sai_ipmc_db_get(&ipmc_entry->key))
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    sal_memset(&ipmc, 0, sizeof(ipmc));

    /* 3. evaluate db entry */
    sal_memcpy(&ipmc, ipmc_entry, sizeof(stbl_ipmc_t));

    /* 4. add to db */
    stbl_ipmc_del_ipmc(&ipmc.key);

    return SAI_STATUS_SUCCESS;
}

void 
__mapping_key(const stbl_ipmc_t* ipmc_entry, ctc_ipmc_group_info_t *ipmc)
{
    ipmc->ip_version = (ipmc_entry->key.destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4) ? 
                                CTC_IP_VER_4 : CTC_IP_VER_6;
    if (ipmc->ip_version == CTC_IP_VER_4)
    {
        ipmc->group_ip_mask_len = 32;
        ipmc->src_ip_mask_len = (ipmc_entry->key.type == SAI_IPMC_TYPE_SG) ? 32 : 0;
        ipmc->address.ipv4.group_addr = ipmc_entry->key.destination.addr.ip4;
        ipmc->address.ipv4.src_addr = ipmc_entry->key.source.addr.ip4;
        ipmc->address.ipv4.vrfid = (uint16)CTC_SAI_OBJECT_INDEX_GET(ipmc_entry->key.vrf_id);
    }
    else
    {
        ipmc->group_ip_mask_len = 128;
        ipmc->src_ip_mask_len = (ipmc_entry->key.type == SAI_IPMC_TYPE_SG) ? 128 : 0;
        ipmc->address.ipv6.vrfid = (uint16)CTC_SAI_OBJECT_INDEX_GET(ipmc_entry->key.vrf_id);

        sal_memcpy(ipmc->address.ipv6.group_addr, ipmc_entry->key.destination.addr.ip6, sizeof(sai_ip6_t));
        sal_memcpy(ipmc->address.ipv6.src_addr, ipmc_entry->key.source.addr.ip6, sizeof(sai_ip6_t));
    }
}

sai_status_t
ctc_sai_ipmc_counter_create(uint32 *counter_id)
{
    ctc_stats_statsid_t stats;
    int32_t             sdk_ret  = 0;

    sal_memset(&stats,0,sizeof(stats));

    stats.type                 = CTC_STATS_STATSID_TYPE_IPMC;
    stats.dir = CTC_INGRESS;
    
    CTC_SAI_ERROR_GOTO(ctc_stats_create_statsid(&stats), sdk_ret, out);
    if(SAI_STATUS_SUCCESS == sdk_ret)
    {
        *counter_id = stats.stats_id;
        ctc_stats_clear_stats(stats.stats_id);
    }

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_ipmc_sdk_add_entry(const stbl_ipmc_t* ipmc_entry, uint32* counter_id)
{
    int32_t                 ret = 0;
    ctclib_slistnode_t     *listnode       = NULL;
    stbl_nexthop_group_t   *nhg = NULL;
    stbl_rif_t             *p_sdb_rif  = NULL;
    ctc_ipmc_group_info_t   grp_param  = {0};
    ctc_port_bitmap_t       vlan_ports = {0};
    int32_t                 idx = 0;
    int32_t                 bit_cnt = 0;
	
	log_sys(M_MOD_INTERNAL, E_ALERT, "zhw test: ctc_sai_ipmc_add_entry, start \n");
	
    /* 1. create ipmc group */
    nhg = ctc_nexthop_group_get_by_oid(ipmc_entry->nexthop_group_id);
	
    grp_param.group_id = 0;
    if (nhg)
    {
        grp_param.group_id = nhg->ipmc_group_id;
    }
    grp_param.flag |= CTC_IPMC_FLAG_RPF_CHECK | CTC_IPMC_FLAG_TTL_CHECK | CTC_IPMC_FLAG_STATS;
    __mapping_key(ipmc_entry, &grp_param);

	CTC_ERROR_RETURN(ctc_sai_ipmc_counter_create(counter_id));
	grp_param.stats_id = *counter_id;
	log_sys(M_MOD_INTERNAL, E_ALERT, "zhw test:ctc_sai_ipmc_install_entry, start 2, statsid %d\n", grp_param.stats_id);
	
    ret = ctc_ipmc_add_group(&grp_param);
    if(ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(ret);
    }

    /* 2. add mcast member to ipmc */
    CTCLIB_SLIST_LOOP(nhg->nh_list.obj_list, p_sdb_rif, listnode)
    {   
        sal_memset(&grp_param, 0, sizeof(grp_param));
        __mapping_key(ipmc_entry, &grp_param);
    
        switch(p_sdb_rif->type)
        {
        case SAI_ROUTER_INTERFACE_TYPE_PORT:
            grp_param.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_PHY_IF; 
            ctc_sai_port_objectid_to_gport(p_sdb_rif->port_oid, &grp_param.ipmc_member[0].global_port);
            idx++;
			grp_param.member_number = 1;
            break;
        case SAI_ROUTER_INTERFACE_TYPE_VLAN:
            ctc_vlan_get_ports(p_sdb_rif->vlan_id, 0, vlan_ports);
            for (bit_cnt = 0; bit_cnt < MAX_PORT_NUM_PER_CHIP; bit_cnt++)
            {
                if (CTC_BMP_ISSET(vlan_ports, bit_cnt))
                {
                    grp_param.ipmc_member[idx].l3_if_type = CTC_L3IF_TYPE_VLAN_IF; 
                    grp_param.ipmc_member[idx].vlan_id = p_sdb_rif->vlan_id;
                    grp_param.ipmc_member[idx].global_port = bit_cnt;
                    idx++;
                }
            }
			grp_param.member_number = idx;
            break;
        default:
            break;
        }
		
		log_sys(M_MOD_INTERNAL, E_ALERT, "zhw test:ctc_ipmc_add_member, start \n");
        ret = ctc_ipmc_add_member(&grp_param);
    }

    /* 3. add rpf to ipmc */
    if(0 != ipmc_entry->rpf_router_interface)
    {
        sal_memset(&grp_param, 0, sizeof(grp_param));
        __mapping_key(ipmc_entry, &grp_param);

        grp_param.rpf_intf_valid[0] = 1;
        grp_param.rpf_intf[0] = (ctc_routerintf_get_by_oid(ipmc_entry->rpf_router_interface))->key.rif_id;

        ret = ctc_ipmc_update_rpf(&grp_param);
        if(ret < 0)
        {
            return ctc_sai_get_error_from_sdk_error(ret);
        }
    }
    
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_ipmc_sdk_del_entry(const stbl_ipmc_t* ipmc_entry)
{
    int32_t                 ret = 0;
    stbl_nexthop_group_t    *nhg = NULL;
    ctc_ipmc_group_info_t   grp_param = {0};

    /* 1. create ipmc group */
    nhg = ctc_nexthop_group_get_by_oid(ipmc_entry->nexthop_group_id);

    grp_param.group_id = 0;
    if (nhg)
    {
        grp_param.group_id = nhg->ipmc_group_id;
    }
    grp_param.ip_version = (ipmc_entry->key.destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4) ? 
                                CTC_IP_VER_4 : CTC_IP_VER_6;
    if (grp_param.ip_version == CTC_IP_VER_4)
    {
        grp_param.group_ip_mask_len = 32;
        grp_param.src_ip_mask_len = (ipmc_entry->key.type == SAI_IPMC_TYPE_SG) ? 32 : 0;
        grp_param.address.ipv4.group_addr = ipmc_entry->key.destination.addr.ip4;
        grp_param.address.ipv4.src_addr = ipmc_entry->key.source.addr.ip4;
        grp_param.address.ipv4.vrfid = (uint16)CTC_SAI_OBJECT_INDEX_GET(ipmc_entry->key.vrf_id);
    }
    else
    {
        grp_param.group_ip_mask_len = 128;
        grp_param.src_ip_mask_len = (ipmc_entry->key.type == SAI_IPMC_TYPE_SG) ? 128 : 0;
        grp_param.address.ipv6.vrfid = (uint16)CTC_SAI_OBJECT_INDEX_GET(ipmc_entry->key.vrf_id);

        sal_memcpy(grp_param.address.ipv6.group_addr, ipmc_entry->key.destination.addr.ip6, sizeof(sai_ip6_t));
        sal_memcpy(grp_param.address.ipv6.src_addr, ipmc_entry->key.source.addr.ip6, sizeof(sai_ip6_t));
    }

    ret = ctc_ipmc_remove_group(&grp_param);
    if(ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(ret);
    }

    return ctc_sai_get_error_from_sdk_error(ret);
}


sai_status_t 
ctc_sai_create_ipmc_entry(
    _In_ const sai_ipmc_entry_t* ipmc_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    stbl_ipmc_t sdb_ipmc = {};
    const sai_attribute_t *pattr = NULL;
    sai_status_t    ret = SAI_STATUS_SUCCESS;
	uint32 stats_id = 0;
 
    sal_memset(&sdb_ipmc, 0, sizeof(sdb_ipmc));
    sal_memcpy(&sdb_ipmc.key, ipmc_entry, sizeof(sai_ipmc_entry_t));

    /* 1. check entry exist */
    if(ctc_sai_ipmc_db_get(&sdb_ipmc.key))
    {
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    /* 2. get nexthop ipmc group */
    pattr = ctc_sai_attr_get_attr_by_id(SAI_IPMC_ATTR_NEXT_HOP_GROUP_ID, attr_list, attr_count, NULL);
    if(NULL == pattr){
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    sdb_ipmc.nexthop_group_id = pattr->value.oid;

    pattr = ctc_sai_attr_get_attr_by_id(SAI_IPMC_ATTR_RPF_ROUTER_INTERFACE, attr_list, attr_count, NULL);
    if(NULL != pattr){
        sdb_ipmc.rpf_router_interface = pattr->value.oid;
    }

    /* 3. set chip */
    ctc_sai_ipmc_sdk_add_entry(&sdb_ipmc, &stats_id);
	sdb_ipmc.counter_id = stats_id;
	
    /* 4. add sdb */
    ret = ctc_sai_ipmc_db_add(&sdb_ipmc);
    if (ret < 0)
    {
        return ret;
    }
    
    return SAI_STATUS_SUCCESS;
}


sai_status_t 
ctc_sai_remove_ipmc_entry(
    _In_ const sai_ipmc_entry_t* ipmc_entry)
{
    stbl_ipmc_t sdb_ipmc = {};
    stbl_ipmc_t *psdb_ipmc = NULL;
    sai_status_t    ret = SAI_STATUS_SUCCESS;
 
    sal_memset(&sdb_ipmc, 0, sizeof(sdb_ipmc));
    sal_memcpy(&sdb_ipmc.key, ipmc_entry, sizeof(sai_ipmc_entry_t));

    /* 1. check entry exist */
    if(psdb_ipmc = ctc_sai_ipmc_db_get(&sdb_ipmc.key), psdb_ipmc == NULL)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* 3. set chip */
    ctc_sai_ipmc_sdk_del_entry(psdb_ipmc);

    /* 4. del sdb */
    ret = ctc_sai_ipmc_db_del(psdb_ipmc);
    if (ret < 0)
    {
        return ret;
    }
    
    return SAI_STATUS_SUCCESS;
}



sai_status_t 
ctc_sai_set_ipmc_entry_attribute(
    _In_ const sai_ipmc_entry_t* ipmc_entry,
    _In_ const sai_attribute_t *attr)
{
    stbl_ipmc_t sdb_ipmc = {};
    stbl_ipmc_t *psdb_ipmc = NULL;
    stbl_nexthop_group_t   *nhg = NULL;
    ctc_ipmc_group_info_t   grp_param  = {0};
    stbl_rif_t             *p_sdb_rif  = NULL;
    stbl_rif_t             *p_sdb_rif1  = NULL;
    ctc_port_bitmap_t       vlan_ports = {0};
    uint32                  i = 0;
    uint32                  found = FALSE;
    int32_t                 idx = 0;
    int32_t                 bit_cnt = 0;
    ctclib_slistnode_t     *listnode       = NULL;
    ctclib_slistnode_t     *listnode1      = NULL;
    
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    sal_memset(&sdb_ipmc, 0, sizeof(sdb_ipmc));
    sal_memcpy(&sdb_ipmc.key, ipmc_entry, sizeof(sai_ipmc_entry_t));

    psdb_ipmc = ctc_sai_ipmc_db_get(&sdb_ipmc.key);
    if(NULL == psdb_ipmc)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    nhg = ctc_nexthop_group_get_by_oid(psdb_ipmc->nexthop_group_id);

    sal_memset(&grp_param, 0, sizeof(grp_param));
    __mapping_key(&sdb_ipmc, &grp_param);

    /* del ipmc member */
    CTCLIB_SLIST_LOOP_DEL(nhg->nh_list.obj_list, p_sdb_rif, listnode, listnode1)
    {
        if (0 == attr[0].value.objlist.count)
        {
            switch(p_sdb_rif->type)
            {
            case SAI_ROUTER_INTERFACE_TYPE_PORT:
                grp_param.member_number = 1;
                grp_param.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_PHY_IF; 
                ctc_sai_port_objectid_to_gport(p_sdb_rif->port_oid, &grp_param.ipmc_member[0].global_port);
                ret = ctc_ipmc_remove_member(&grp_param);

                stbl_nexthop_group_set_nh_list(nhg, FALSE, p_sdb_rif);
                break;

            case SAI_ROUTER_INTERFACE_TYPE_VLAN:
                grp_param.member_number = 1;
                ctc_vlan_get_ports(p_sdb_rif->vlan_id, 0, vlan_ports);
                for (bit_cnt = 0; bit_cnt < MAX_PORT_NUM_PER_CHIP; bit_cnt++)
                {
                    if (CTC_BMP_ISSET(vlan_ports, bit_cnt))
                    {
                        grp_param.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_VLAN_IF; 
                        grp_param.ipmc_member[0].vlan_id = p_sdb_rif->vlan_id;
                        grp_param.ipmc_member[0].global_port = bit_cnt;
                        ret = ctc_ipmc_remove_member(&grp_param);
                    }
                }
                stbl_nexthop_group_set_nh_list(nhg, FALSE, p_sdb_rif);
                break;
            default:
                break;
            }

            continue;
        }

        found = FALSE;
        for (i = 0; i < attr[0].value.objlist.count; i++)
        {
            p_sdb_rif1 = ctc_routerintf_get_by_oid(attr[0].value.objlist.list[i]);
            if (NULL == p_sdb_rif1)
            {
                return SAI_STATUS_ITEM_NOT_FOUND;
            }
            if (!sal_memcmp(p_sdb_rif1, p_sdb_rif, sizeof(p_sdb_rif->key)))
            {
                found = TRUE;
                break;
            }
        }

        if (!found)
        {
            switch(p_sdb_rif->type)
            {
            case SAI_ROUTER_INTERFACE_TYPE_PORT:
                grp_param.member_number = 1;
                grp_param.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_PHY_IF; 
                ctc_sai_port_objectid_to_gport(p_sdb_rif->port_oid, &grp_param.ipmc_member[0].global_port);
                ret = ctc_ipmc_remove_member(&grp_param);

                stbl_nexthop_group_set_nh_list(nhg, FALSE, p_sdb_rif);
                break;

            case SAI_ROUTER_INTERFACE_TYPE_VLAN:
                grp_param.member_number = 1;
                ctc_vlan_get_ports(p_sdb_rif->vlan_id, 0, vlan_ports);
                for (bit_cnt = 0; bit_cnt < MAX_PORT_NUM_PER_CHIP; bit_cnt++)
                {
                    if (CTC_BMP_ISSET(vlan_ports, bit_cnt))
                    {
                        grp_param.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_VLAN_IF; 
                        grp_param.ipmc_member[0].vlan_id = p_sdb_rif->vlan_id;
                        grp_param.ipmc_member[0].global_port = bit_cnt;
                        ret = ctc_ipmc_remove_member(&grp_param);
                    }
                }
                stbl_nexthop_group_set_nh_list(nhg, FALSE, p_sdb_rif);
                break;

            default:
                break;
            }
            continue;
        }
    }

    /* add ipmc member */
    for (i = 0; i < attr[0].value.objlist.count; i++)
    {
        found = FALSE;
        p_sdb_rif = ctc_routerintf_get_by_oid(attr[0].value.objlist.list[i]);
        if (NULL == p_sdb_rif)
        {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }

        CTCLIB_SLIST_LOOP(nhg->nh_list.obj_list, p_sdb_rif1, listnode)
        {
            if (!sal_memcmp(p_sdb_rif1, p_sdb_rif, sizeof(p_sdb_rif->key)))
            {
                found = TRUE;
                break;
            }
        }

        if (!found)
        {
            switch(p_sdb_rif->type)
            {
            case SAI_ROUTER_INTERFACE_TYPE_PORT:
                grp_param.member_number = 1;
                grp_param.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_PHY_IF; 
                ctc_sai_port_objectid_to_gport(p_sdb_rif->port_oid, &grp_param.ipmc_member[idx].global_port);
                ret = ctc_ipmc_add_member(&grp_param);
                break;

            case SAI_ROUTER_INTERFACE_TYPE_VLAN:
                ctc_vlan_get_ports(p_sdb_rif->vlan_id, 0, vlan_ports);
                for (bit_cnt = 0; bit_cnt < MAX_PORT_NUM_PER_CHIP; bit_cnt++)
                {
                    if (CTC_BMP_ISSET(vlan_ports, bit_cnt))
                    {
                        grp_param.ipmc_member[idx].l3_if_type = CTC_L3IF_TYPE_VLAN_IF; 
                        grp_param.ipmc_member[idx].vlan_id = p_sdb_rif->vlan_id;
                        grp_param.ipmc_member[idx].global_port = bit_cnt;
                        idx++;
                    }
                }
                grp_param.member_number = idx;

                ret = ctc_ipmc_add_member(&grp_param);
                break;
            
            default:
                break;
            }
        }
        
        stbl_nexthop_group_set_nh_list(nhg, TRUE, p_sdb_rif);
    }

    return ret;
}

sai_status_t 
ctc_sai_get_ipmc_entry_attribute(
    _In_ const sai_ipmc_entry_t* ipmc_entry,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
	stbl_ipmc_t sdb_ipmc = {};
    stbl_ipmc_t *psdb_ipmc = NULL;
	
	sai_attribute_t  *attr = NULL;
    uint32_t          attr_idx = 0;
    sai_status_t      ret      = SAI_STATUS_FAILURE;
    ctc_stats_basic_t stats_count;


	sal_memset(&sdb_ipmc, 0, sizeof(sdb_ipmc));
	
	log_sys(M_MOD_INTERNAL, E_ALERT, "ctc_sai_get_ipmc_entry, ipaddr %x, source %x, vrfid %x,type %x\n",
		 ipmc_entry->destination.addr.ip4, ipmc_entry->source.addr.ip4, ipmc_entry->vrf_id, ipmc_entry->type);

	/*1, check entry exist*/
	sal_memcpy(&sdb_ipmc.key, ipmc_entry, sizeof(sai_ipmc_entry_t));
	psdb_ipmc = ctc_sai_ipmc_db_get(&sdb_ipmc);
	if(psdb_ipmc)
	{
		for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
		{
			attr = attr_list + attr_idx;	
			log_sys(M_MOD_INTERNAL, E_ERROR, "zhw test,ctc_sai_get_ipmc_entry_attributet: %x, attr_count:%d\n", attr->id, attr_count);
			switch(attr->id)
			{
			case SAI_IPMC_ATTR_NEXT_HOP_GROUP_ID:
				break;
		
			case SAI_IPMC_ATTR_RPF_ROUTER_INTERFACE:
				break;
		
			
		
			case SAI_IPMC_COUNTER_ATTR_COUNTER_PACKETS:
				sal_memset(&stats_count, 0, sizeof(stats_count));
				ret = ctc_stats_get_stats(psdb_ipmc->counter_id, &stats_count);
				attr->value.u64 = stats_count.packet_count;
				log_sys(M_MOD_INTERNAL, E_ERROR, "ctc_sai_get_ipmc_entry_attributet, psdb_ipmc->counter_id:%d, stats_count.packet_count:%d\n",psdb_ipmc->counter_id, stats_count.packet_count);
				break;
		
			case SAI_IPMC_COUNTER_ATTR_COUNTER_BYTES:
				sal_memset(&stats_count, 0, sizeof(stats_count));
				ret = ctc_stats_get_stats(psdb_ipmc->counter_id, &stats_count);
				attr->value.u64 = stats_count.byte_count;
				log_sys(M_MOD_INTERNAL, E_ERROR, "ctc_sai_get_ipmc_entry_attributet, psdb_ipmc->counter_id:%d, stats_count.byte_count:%d\n",psdb_ipmc->counter_id, stats_count.byte_count);
				break;
			}
		
			if (ret != SAI_STATUS_SUCCESS)
			{
				log_sys(M_MOD_INTERNAL, E_ERROR, "ctc_sai_get_ipmc_entry_attributet get stats fail, ret = %d :%s@%d \n",  ret, __FUNCTION__, __LINE__);
				break;
			}
		}
		
		return SAI_STATUS_SUCCESS;
	}
	else
	{
		log_sys(M_MOD_INTERNAL, E_ERROR, "ctc_sai_get_ipmc_entry_attribute entry not exist:%s@%d \n",  __FUNCTION__, __LINE__);
		return SAI_STATUS_ITEM_NOT_FOUND;

	}

    return SAI_STATUS_SUCCESS;
}

int32 
__ipmc_add_vlan_member(void *obj, stbl_iter_args_t *pargs)
{
    ctc_sai_vlan_notifier_t  *pvlan_nf        = pargs->argv[0];
    stbl_ipmc_t              *psdb_ipmc        = obj;
    ctclib_slistnode_t       *listnode        = NULL;
    stbl_nexthop_group_t     *nhg              = NULL;
    stbl_rif_t               *p_sdb_rif        = NULL;
    ctc_ipmc_group_info_t    grp_param         = {0};

    nhg = ctc_nexthop_group_get_by_oid(psdb_ipmc->nexthop_group_id);
    if (!nhg)
    {
        return 0;
    }

    CTCLIB_SLIST_LOOP(nhg->nh_list.obj_list, p_sdb_rif, listnode)
    {   
        if(SAI_ROUTER_INTERFACE_TYPE_VLAN != p_sdb_rif->type || 
           p_sdb_rif->vlan_id != pvlan_nf->vlan_id)
        {
            continue;
        }

        sal_memset(&grp_param, 0, sizeof(grp_param));
        __mapping_key(psdb_ipmc, &grp_param);

        grp_param.member_number = 1;
        grp_param.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_VLAN_IF; 
        grp_param.ipmc_member[0].vlan_id = p_sdb_rif->vlan_id;
        ctc_sai_port_objectid_to_gport(pvlan_nf->vlan_member_port_id, &grp_param.ipmc_member[0].global_port);
        
        ctc_ipmc_add_member(&grp_param);
        break;
    }

    return 0;
}

int32 
__ipmc_del_vlan_member(void *obj, stbl_iter_args_t *pargs)
{
    ctc_sai_vlan_notifier_t  *pvlan_nf        = pargs->argv[0];
    stbl_ipmc_t              *psdb_ipmc        = obj;
    ctclib_slistnode_t       *listnode        = NULL;
    stbl_nexthop_group_t     *nhg              = NULL;
    stbl_rif_t               *p_sdb_rif        = NULL;
    ctc_ipmc_group_info_t    grp_param         = {0};

    nhg = ctc_nexthop_group_get_by_oid(psdb_ipmc->nexthop_group_id);
    if (!nhg)
    {
        return 0;
    }

    CTCLIB_SLIST_LOOP(nhg->nh_list.obj_list, p_sdb_rif, listnode)
    {   
        if(SAI_ROUTER_INTERFACE_TYPE_VLAN != p_sdb_rif->type || 
           p_sdb_rif->vlan_id != pvlan_nf->vlan_id)
        {
            continue;
        }

        sal_memset(&grp_param, 0, sizeof(grp_param));
        __mapping_key(psdb_ipmc, &grp_param);

        grp_param.member_number = 1;
        grp_param.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_VLAN_IF; 
        grp_param.ipmc_member[0].vlan_id = p_sdb_rif->vlan_id;
        ctc_sai_port_objectid_to_gport(pvlan_nf->vlan_member_port_id, &grp_param.ipmc_member[0].global_port);
        
        ctc_ipmc_remove_member(&grp_param);
        break;
    }

    return 0;
}

int32_t
ctc_ipmc_vlan_notifier_callback(
    _In_  struct ctc_sai_notifier_block *nb,
    _In_  uint32_t action,
    _In_  void *data)
{
    ctc_sai_vlan_notifier_t  *pvlan_nf        = data;
    stbl_iter_args_t        arg = {};

    arg.argv[0] = data;
    if(CTC_SAI_LAG_NF_NEW_VLAN_MEMBER == pvlan_nf->op)
    {
        stbl_ipmc_iterate(__ipmc_add_vlan_member, &arg);
    }else{
        stbl_ipmc_iterate(__ipmc_del_vlan_member, &arg);
    }
    return 0;
}

static struct ctc_sai_notifier_block nb_vlan_notifier_cb = {
    .notifier_call = ctc_ipmc_vlan_notifier_callback,
};

#define ________SAI_IPMC_INNER_FUNC
static sai_status_t
__init_mode_fn(
    _In_  ctc_sai_api_reg_info_t    *preg,
    _In_  void                      *private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    ctc_sai_vlan_evnet_notifier_register(&nb_vlan_notifier_cb);

    //ctc_ipmc_add_default_entry(CTC_IP_VER_4, CTC_IPMC_DEFAULT_ACTION_TO_CPU);

    preg->init_status =  INITIALIZED;
    return ret;
}

static sai_status_t
__exit_mode_fn(
    _In_  ctc_sai_api_reg_info_t    *preg,
    _In_  void                      *private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static sai_ipmc_api_t      g_ipmc_api_func = {
    .create_ipmc_entry          = ctc_sai_create_ipmc_entry,
    .remove_ipmc_entry          = ctc_sai_remove_ipmc_entry,
    .set_ipmc_entry_attribute   = ctc_sai_set_ipmc_entry_attribute,
    .get_ipmc_entry_attribute   = ctc_sai_get_ipmc_entry_attribute,
};

static ctc_sai_api_reg_info_t g_ipmc_api_reg_info = {
    .id         = SAI_API_IPMC,
    .init_func  = __init_mode_fn,
    .exit_func  = __exit_mode_fn,
    .api_method_table = &g_ipmc_api_func,
    .private_data     = NULL,
};

#define ________SAI_L2MC_OUTER_FUNC

sai_status_t
ctc_sai_ipmc_init()
{
    api_reg_register_fn(&g_ipmc_api_reg_info);

    return SAI_STATUS_SUCCESS;
}
