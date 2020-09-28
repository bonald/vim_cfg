#include "glb_tempfile_define.h"
#include "hsrv_inc.h"
#include "hsrv_fdb.h"
#include "hsrv_g8032.h"
#include "sdb_defines.h"
#include "saifdb.h"
#include "saistp.h"
#include "saiapsgroup.h"
#include "glb_openflow_define.h"
#include "hsrv_openflow_nexthop.h"
#include "hsrv_openflow_tpoam.h"
extern int32
ctc_sai_mgroup_id_opf_alloc(uint32* p_index);

extern sai_status_t
ctc_sai_port_objectid_to_gport(sai_object_id_t port_oid, uint32 *pgport);

extern sai_status_t
ctc_sai_l2mc_free_offset(uint32_t opf_index);


int32 hsrv_g8032_mgroup_update_member_port(uint32 ifindex, uint32 mcast_group_id, uint8 is_add)
{
    int32 rc = HSRV_E_NONE;
    sai_object_id_t port = 0;
    sai_aps_group_api_t    *aps_group_api = NULL;
    sai_attribute_t attr[2];
    tbl_interface_key_t if_key;
    uint32 gport = 0;
    tbl_interface_t *p_db_interface = NULL;

    sal_memset(&if_key, 0, sizeof(if_key));
    p_db_interface = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_db_interface)
    {
       return rc; 
    }
    
    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(ifindex, &port));

    ctc_sai_port_objectid_to_gport(port,&gport); 
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_APS_GROUP,(void**)&aps_group_api));
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_APS_GROUP_MGROUP_TYPE_ADD;
    attr[0].value.s8 = is_add;
    attr[1].id = SAI_APS_GROUP_MGROUP_TYPE_ID;
    attr[1].value.s16 = mcast_group_id;
    
    HSRV_IF_ERROR_RETURN(aps_group_api->aps_mgroup_update_member_port(gport, 2, attr));
    return rc;
}

int32
hsrv_g8032_set_fea_ring(uint32 instance_id, uint32 ifindex, uint32 nh_offset, uint32 nh_id, uint32 aps_group_id)
{
    tbl_fea_g8032_ring_t  fea_g8032_ring;
    sal_memset(&fea_g8032_ring, 0, sizeof(fea_g8032_ring));
    fea_g8032_ring.key.instance_id= instance_id;
    fea_g8032_ring.key.ifindex= ifindex;
    fea_g8032_ring.nhoffset = nh_offset;
    fea_g8032_ring.nh_id = nh_id;
    fea_g8032_ring.aps_group_id = aps_group_id;
    tbl_fea_g8032_ring_add_fea_g8032_ring(&fea_g8032_ring);
    return HSRV_E_NONE;
}

int32
hsrv_g8032_delete_aps_group(uint32 ring_id, uint32 ifindex, uint32 mcast_group_id)
{
    int32 rc = HSRV_E_NONE;
    sai_aps_group_api_t    *aps_group_api = NULL;
    uint32 nh_offset = 0;
    hsrv_of_nh_offset_t hsrv_nh_offset;
    uint32 aps_offset = 0;
    uint32 nh_id = 0;
    sai_object_id_t port = 0;
    uint32 gport = 0;
    int32 vid = 0;
    int32 i = 0;
    uint32 instance_id = 0;
    tbl_mstp_instance_key_t inst_key;
    tbl_interface_t *p_db_interface = NULL;
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    tbl_mstp_instance_t *p_db_mstp_instance = NULL;
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_fea_g8032_ring_key_t fea_g8032_ring_key;
    tbl_fea_g8032_ring_t *p_fea_g8032_ring = NULL;

    sal_memset(&fea_g8032_ring_key, 0, sizeof(fea_g8032_ring_key));
    sal_memset(&hsrv_nh_offset, 0, sizeof(hsrv_nh_offset));
    p_db_interface = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_db_interface)
    {
       return rc; 
    }
    p_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t *)(& ring_id));
    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(ifindex, &port));
    ctc_sai_port_objectid_to_gport(port,&gport); 
    sal_memset(&nh_offset, 0, sizeof(nh_offset));
    hsrv_nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_MCAST; 

    instance_id = p_g8032_ring->instance[0];
    inst_key.instance = instance_id;
    p_db_mstp_instance = tbl_mstp_instance_get_mstp_instance(&inst_key);

    do {                                                        
             for ((vid) = 0; (vid) < GLB_BMP_BITS((p_db_mstp_instance->vlan)); (vid)++)  
             {
                if (GLB_BMP_ISSET(p_db_mstp_instance->vlan, vid))
                {
                    vlan_key.vid = vid;
                    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
                    if (NULL == p_db_vlan)
                    {
                        continue;
                    }
                    fea_g8032_ring_key.ifindex = ifindex;
                    fea_g8032_ring_key.instance_id = instance_id;
                    p_fea_g8032_ring = tbl_fea_g8032_ring_get_fea_g8032_ring(&fea_g8032_ring_key);
                    if (p_fea_g8032_ring)
                    {
                        break;
                    }
                   
                }
             }
        }while(0);
    if (!p_fea_g8032_ring)
    {
        return rc; 
    }
    aps_offset = p_fea_g8032_ring->aps_group_id;
    
    nh_id = p_fea_g8032_ring->nh_id;
    nh_offset = p_fea_g8032_ring->nhoffset;

    hsrv_nh_offset.nhid = nh_id;
    hsrv_nh_offset.offset= nh_offset;
    hsrv_nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_MCAST;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_APS_GROUP,(void**)&aps_group_api));
    HSRV_IF_ERROR_RETURN(aps_group_api->delete_aps_group(aps_offset, nh_id));
    hsrv_aps_group_offset_release(aps_offset);
    hsrv_openflow_nexthop_offset_release(&hsrv_nh_offset);

    for (i = 0; i < p_g8032_ring->instance_count; i++)
    {
        fea_g8032_ring_key.ifindex = ifindex;
        fea_g8032_ring_key.instance_id = p_g8032_ring->instance[i];
        tbl_fea_g8032_ring_del_fea_g8032_ring(&fea_g8032_ring_key);
    }

    return rc; 
    

}

int32
hsrv_g8032_add_aps_group(uint32 ring_id, uint32 ifindex, uint32 mcast_group_id)
{
    int32 rc = HSRV_E_NONE;
    sai_aps_group_api_t    *aps_group_api = NULL;
    hsrv_of_nh_offset_t nh_offset;

    uint32 aps_offset = 0;
    uint32 nh_id = 0;
    sai_object_id_t port = 0;
    uint32 gport = 0;
    int32 i = 0;
    uint32 instance_id = 0;
    sai_attribute_t attr[4];
    tbl_mstp_instance_key_t inst_key;
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_interface = NULL;
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    tbl_mstp_instance_t *p_db_mstp_instance = NULL;

    
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_memset(&attr, 0, sizeof(attr));
    p_db_interface = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_db_interface)
    {
       return rc; 
    }
    p_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t *)(& ring_id));
    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(ifindex, &port));
    ctc_sai_port_objectid_to_gport(port,&gport); 
    sal_memset(&nh_offset, 0, sizeof(nh_offset));
    nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_MCAST;  
    hsrv_openflow_nexthop_offset_alloc(&nh_offset);
    hsrv_aps_group_offset_alloc(&aps_offset);
    nh_id = nh_offset.nhid;
    HSRV_G8032_EVENT_DEBUG("g8032 start");
    for (i = 0; i < p_g8032_ring->instance_count; i++)
    {
        instance_id = p_g8032_ring->instance[i];
        inst_key.instance = instance_id;
        p_db_mstp_instance = tbl_mstp_instance_get_mstp_instance(&inst_key);

        if (p_db_mstp_instance)
        {
            hsrv_g8032_set_fea_ring(instance_id, ifindex, nh_offset.offset, nh_id, aps_offset);
        }
    }
     HSRV_G8032_EVENT_DEBUG("g8032 end");
    
    attr[0].id = SAI_APS_GROUP_MGROUP_TYPE_ID;
    attr[0].value.s16 = mcast_group_id ;
    attr[1].id = SAI_APS_GROUP_TYPE_ID;
    attr[1].value.s32 = aps_offset;
    attr[2].id = SAI_APS_GROUP_TYPE_NH_ID;
    attr[2].value.s32 = nh_offset.nhid;
    attr[3].id = SAI_APS_GROUP_TYPE_NH_OFFSET;
    attr[3].value.s32 = nh_offset.offset;
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_APS_GROUP,(void**)&aps_group_api));
    HSRV_IF_ERROR_RETURN(aps_group_api->add_aps_group(gport, 4, attr));
    return rc;
}

int32
hsrv_g8032_creat_aps_mgroup(uint32 mcast_group_id)
{
    int32 rc = HSRV_E_NONE;
    sai_aps_group_api_t    *aps_group_api = NULL;
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_APS_GROUP,(void**)&aps_group_api));
    HSRV_IF_ERROR_RETURN(aps_group_api->create_raps_mc_group(mcast_group_id ));
    return rc;
}

int32
hsrv_g8032_destory_aps_mgroup(uint32 mcast_group_id)
{
    int32 rc = HSRV_E_NONE;
    sai_aps_group_api_t    *aps_group_api = NULL;
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_APS_GROUP,(void**)&aps_group_api));
    HSRV_IF_ERROR_RETURN(aps_group_api->destroy_raps_mc_group(mcast_group_id));
    return rc;
}

int32
hsrv_l2mc_alloc_mgroupid(uint32 *mcast_group_id)
{
    int32 rc = HSRV_E_NONE;
    uint32_t             nhg_id;
    ctc_sai_mgroup_id_opf_alloc(&nhg_id);
    *mcast_group_id = nhg_id;
    return rc;
}

int32
hsrv_l2mc_free_mgroupid(uint32 ring_id, uint32 mcast_group_id)
{
    int32 rc = HSRV_E_NONE;
    tbl_fea_g8032_ring_mc_group_t  *p_fea_g8032_ring_mc_group = NULL;
    p_fea_g8032_ring_mc_group = tbl_fea_g8032_ring_mc_group_get_fea_g8032_ring_mc_group((tbl_fea_g8032_ring_mc_group_key_t *)&ring_id);
    if (p_fea_g8032_ring_mc_group)
    {
        tbl_fea_g8032_ring_mc_group_del_fea_g8032_ring_mc_group((tbl_fea_g8032_ring_mc_group_key_t *)&ring_id);
    }
    ctc_sai_l2mc_free_offset(mcast_group_id);
    
    return rc;
}

int32
hsrv_g8032_delete_control_vlan(tbl_g8032_ring_t *p_g8032_ring)
{
    sai_fdb_api_t*  fdb_api = NULL;
    sai_fdb_entry_t l2_addr;
    uint8_t mac_addr[6] = {0x01, 0x19, 0xA7, 0x00, 0x00, 0x01};
    sai_l2mc_api_t*  l2mc_api = NULL;
    sai_l2mc_entry_t l2mc_addr;
    int32 rc = HSRV_E_NONE;
    
    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api));

    if (!p_g8032_ring)
    {
        return HSRV_E_NONE;
    }

    if (!p_g8032_ring->control_vlan)
    {
         return HSRV_E_NONE;
    }
    if (p_g8032_ring->is_fwdtocpu)
    {
        sal_memcpy(l2_addr.mac_address, mac_addr, sizeof(mac_addr));
        l2_addr.vlan_id = p_g8032_ring->control_vlan;
        HSRV_IF_ERROR_RETURN(fdb_api->remove_fdb_entry(&l2_addr));
    }
    else
    {
            sal_memset(&l2mc_addr, 0, sizeof(l2mc_addr));
            HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
            HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_L2MC, (void**)&l2mc_api));
            sal_memcpy(l2mc_addr.mac_address, mac_addr, sizeof(mac_addr_t));
            l2mc_addr.vlan_id = p_g8032_ring->control_vlan;
            rc = l2mc_api->remove_l2mc_entry(&l2mc_addr);
            return rc;
    }

    return HSRV_E_NONE;
}

int32
hsrv_g8032_add_control_vlan(tbl_g8032_ring_t *p_g8032_ring)
{
    int32 rc = HSRV_E_NONE;
    uint8_t mac_addr[6] = {0x01, 0x19, 0xA7, 0x00, 0x00, 0x01};
    sai_fdb_entry_t fdb_entry;
    tbl_g8032_ring_t *p_major_g8032_ring = NULL;
    sai_object_id_t port = 0;
    sai_fdb_api_t *p_fdb_api = NULL;
    sai_attribute_t attr[3];
    sai_l2mc_api_t*  l2mc_api = NULL;
    sai_l2mc_entry_t l2mc_addr;
    sai_object_id_t port_oid[1];
    sai_attribute_t attr_i[1] =
    {
        [0] = {
           .id = SAI_L2MC_ATTR_PORT_LIST,
        }
    };
    
    sal_memset(&fdb_entry, 0, sizeof(fdb_entry));
    sal_memcpy(fdb_entry.mac_address, mac_addr, sizeof(mac_addr));
    //fdb_entry.mac_address= mac_addr;
    fdb_entry.vlan_id= p_g8032_ring->control_vlan;
    HSRV_MEM_CHECK(p_g8032_ring);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB, (void**)&p_fdb_api));
    
    sal_memset(attr, 0, sizeof(attr));

    /* modified by liwh for bug 53553, 2019-10-23 */
    //hsrv_fdb_db_flush_vlan(p_g8032_ring->control_vlan);
    hsrv_fdb_flush_fdb_vlan(p_g8032_ring->control_vlan);
    /* liwh end */
    
    if (TRUE == p_g8032_ring->is_fwdtocpu)
    {
        HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_g8032_ring->east_port_ifindex, &port));
        attr[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
        attr[0].value.u32 = SAI_FDB_ENTRY_STATIC;
        attr[2].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION;
        attr[2].value.u32 = SAI_PACKET_ACTION_TRAP;
        attr[1].id = SAI_FDB_ENTRY_ATTR_PORT_ID;
        attr[1].value.oid= 0;
        HSRV_IF_ERROR_RETURN(p_fdb_api->create_fdb_entry(&fdb_entry, 3, attr));
    }
    else
    {
            sal_memset(&l2mc_addr, 0, sizeof(l2mc_addr));
            HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
            HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_L2MC, (void**)&l2mc_api));
            sal_memcpy(l2mc_addr.mac_address, mac_addr, sizeof(mac_addr_t));
            l2mc_addr.vlan_id = p_g8032_ring->control_vlan;

            rc = l2mc_api->create_l2mc_entry_with_flag(&l2mc_addr, 0, NULL, CTC_L2_FLAG_COPY_TO_CPU);

            attr_i[0].value.objlist.count = 1;
            attr_i[0].value.objlist.list = port_oid;
            HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_g8032_ring->east_port_ifindex, port_oid));
            rc = l2mc_api->add_ports_to_l2mc(&l2mc_addr, 1, attr_i);

            if (p_g8032_ring->west_port_ifindex)
            {
                attr_i[0].value.objlist.count = 1;
                attr_i[0].value.objlist.list = port_oid;
                HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_g8032_ring->west_port_ifindex, port_oid));
                rc = l2mc_api->add_ports_to_l2mc(&l2mc_addr, 1, attr_i);
            }
            else if(!p_g8032_ring->west_port_ifindex && p_g8032_ring->g8032_major_ring_id 
                            && !p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
            {
                p_major_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t * )&p_g8032_ring->g8032_major_ring_id);
                if (p_major_g8032_ring)
                {
                    attr_i[0].value.objlist.count = 1;
                    attr_i[0].value.objlist.list = port_oid;
                    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_major_g8032_ring->east_port_ifindex, port_oid));
                    rc = l2mc_api->add_ports_to_l2mc(&l2mc_addr, 1, attr_i);

                    if (p_major_g8032_ring->west_port_ifindex)
                    {
                        attr_i[0].value.objlist.count = 1;
                        attr_i[0].value.objlist.list = port_oid;
                        HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_major_g8032_ring->west_port_ifindex, port_oid));
                        rc = l2mc_api->add_ports_to_l2mc(&l2mc_addr, 1, attr_i);
                    }
                    
                }
            }
            
    }

    return rc;
}

int32
hsrv_g8032_bind_instance_to_ring(tbl_g8032_ring_t *p_g8032_ring)
{
    
    HSRV_MEM_CHECK(p_g8032_ring);
    int32 rc = HSRV_E_NONE;
    uint32 i = 0;
    uint32 instance_id = 0;
    uint32 vid = 0;
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_mstp_instance = NULL;
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_g8032_ring_key_t g8032_key;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    
    sal_memset(&inst_key, 0, sizeof(inst_key));
    sal_memset(&vlan_key, 0, sizeof(vlan_key));
    if((0 == p_g8032_ring->g8032_major_ring_id) && (0 != p_g8032_ring->west_port_ifindex))
    {
        for (i = 0; i < p_g8032_ring->instance_count; i++)
        {
            instance_id = p_g8032_ring->instance[i];
            inst_key.instance = instance_id;
            p_db_mstp_instance = tbl_mstp_instance_get_mstp_instance(&inst_key);        
            do {                                                                
                for ((vid) = 0; (vid) < GLB_BMP_BITS((p_db_mstp_instance->vlan)); (vid)++)  
                {
                     if (GLB_BMP_ISSET((p_db_mstp_instance->vlan), (vid)))
                     {
                        vlan_key.vid = vid;
                        p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
                        if (NULL == p_db_vlan)
                        {
                            continue;
                        }

                        if (!GLB_BMP_ISSET(p_db_vlan->member_port, p_g8032_ring->east_port_ifindex)
                            ||!GLB_BMP_ISSET(p_db_vlan->member_port, p_g8032_ring->west_port_ifindex))
                        {
                            continue;
                        }
                        hsrv_fdb_flush_fdb_port_vlan(p_g8032_ring->east_port_ifindex, vid);
                        hsrv_fdb_flush_fdb_port_vlan(p_g8032_ring->west_port_ifindex, vid);
                        if (p_g8032_ring->sub_ring_id[0])
                        {
                            sal_memset(&g8032_key, 0, sizeof(g8032_key));
                            g8032_key.ring_id = p_g8032_ring->sub_ring_id[0];
                            p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_key);
                            if (p_db_g8032_ring)
                            {
                                 hsrv_fdb_flush_fdb_port_vlan(p_db_g8032_ring->east_port_ifindex, vid);
                            }
                        }          
                     }
                }
            }while (0);

        }
     }
     return rc;

}

int32
hsrv_g8032_ring_set_port_state(tbl_g8032_ring_t *p_g8032_ring, uint8 is_east)
{
    sai_stp_api_t    *g8032_stp_api = NULL;
    sai_object_id_t  port_oid = 0;
    sai_object_id_t inst_oid = 0;
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    int i = 0;
    if (NULL == p_g8032_ring)
    {
        return HSRV_E_NONE;
    }
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_STP, (void**)&g8032_stp_api));
    for (i = 0; i < p_g8032_ring->instance_count; i++)
    {
        if (GLB_DEFAULT_INSTANCE == p_g8032_ring->instance[i])
        {
            inst_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_STP_INSTANCE, p_g8032_ring->instance[i]);
        }
        else
        {
            inst_key.instance = p_g8032_ring->instance[i];
            p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
            if (NULL == p_db_inst)
            {
                return HSRV_E_INVALID_PTR;
            }
            inst_oid = p_db_inst->instance_oid;
        }
        if (is_east)
        {
            HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_g8032_ring->east_port_ifindex, &port_oid));
            if (p_g8032_ring->east_port_is_blocked)
            {
                HSRV_IF_ERROR_RETURN(g8032_stp_api->set_stp_port_state(inst_oid, port_oid, SAI_PORT_APS_STATE_BLOCKING));
            }
            else
            {
                HSRV_IF_ERROR_RETURN(g8032_stp_api->set_stp_port_state(inst_oid, port_oid, SAI_PORT_APS_STATE_FORWARDING));
            }
        }
        else
        {
            HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_g8032_ring->west_port_ifindex, &port_oid));
            if (p_g8032_ring->west_port_is_blocked)
            {
                HSRV_IF_ERROR_RETURN(g8032_stp_api->set_stp_port_state(inst_oid, port_oid, SAI_PORT_APS_STATE_BLOCKING));
            }
            else
            {
                HSRV_IF_ERROR_RETURN(g8032_stp_api->set_stp_port_state(inst_oid, port_oid, SAI_PORT_APS_STATE_FORWARDING));
            }
        }
        
    }
    return HSRV_E_NONE;

}

int32
hsrv_g8032_ring_set_aps_protection_en(tbl_g8032_ring_t *p_g8032_ring, uint8_t enable)
{
    tbl_fea_g8032_ring_t  fea_g8032_ring;
    tbl_fea_g8032_ring_t  *p_fea_g8032_ring = NULL;
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_mstp_instance = NULL;
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    uint32 instance_id = 0;
    uint32 vid = 0;
    sai_aps_group_api_t    *aps_group_api = NULL;

     if (NULL == p_g8032_ring)
    {
        return HSRV_E_NONE;
    }
     
    sal_memset(&inst_key, 0, sizeof(inst_key));
    sal_memset(&fea_g8032_ring, 0, sizeof(fea_g8032_ring)); 

    instance_id = p_g8032_ring->instance[0];
    inst_key.instance = instance_id;
    p_db_mstp_instance = tbl_mstp_instance_get_mstp_instance(&inst_key);
    do {                                                                
                for ((vid) = 0; (vid) < GLB_BMP_BITS((p_db_mstp_instance->vlan)); (vid)++)  
                {
                     if (GLB_BMP_ISSET((p_db_mstp_instance->vlan), (vid)))
                     {
                        vlan_key.vid = vid;
                        p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
                        if (NULL != p_db_vlan)
                        {
                                fea_g8032_ring.key.instance_id= instance_id;
                                fea_g8032_ring.key.ifindex= p_g8032_ring->east_port_ifindex;
                                p_fea_g8032_ring = tbl_fea_g8032_ring_get_fea_g8032_ring(&fea_g8032_ring.key);
                                if (p_fea_g8032_ring)
                                {
                                    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_APS_GROUP,(void**)&aps_group_api));
                                    HSRV_IF_ERROR_RETURN(aps_group_api->set_protection_en(p_fea_g8032_ring->aps_group_id, enable));
                                }
                                break;
                        }
                     }
                }
    } while(0);
    
    if ( p_g8032_ring->west_port_ifindex)
    {
         do {                                                                
                for ((vid) = 0; (vid) < GLB_BMP_BITS((p_db_mstp_instance->vlan)); (vid)++)  
                {
                     if (GLB_BMP_ISSET((p_db_mstp_instance->vlan), (vid)))
                     {
                        vlan_key.vid = vid;
                        p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
                        if (NULL != p_db_vlan)
                        {
                                fea_g8032_ring.key.instance_id= instance_id;
                                fea_g8032_ring.key.ifindex= p_g8032_ring->west_port_ifindex;
                                p_fea_g8032_ring = tbl_fea_g8032_ring_get_fea_g8032_ring(&fea_g8032_ring.key);
                                if (p_fea_g8032_ring)
                                {
                                    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_APS_GROUP,(void**)&aps_group_api));
                                    HSRV_IF_ERROR_RETURN(aps_group_api->set_protection_en(p_fea_g8032_ring->aps_group_id, enable));
                                }
                                break;
                        }
                     }
                }
        } while(0);
    }
    return HSRV_E_NONE;
}

int32
hsrv_g8032_ring_flush_fdb_by_port(tbl_g8032_ring_t *p_g8032_ring, uint8 is_east)
{
     int32 rc = HSRV_E_NONE;
     HSRV_MEM_CHECK(p_g8032_ring);
    uint32 i = 0;
    uint32 instance_id = 0;
    uint32 vid = 0;
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_mstp_instance = NULL;
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    
    sal_memset(&inst_key, 0, sizeof(inst_key));
    sal_memset(&vlan_key, 0, sizeof(vlan_key));
    if (is_east)
    {
        for (i = 0; i < p_g8032_ring->instance_count; i++)
        {
            instance_id = p_g8032_ring->instance[i];
            inst_key.instance = instance_id;
            p_db_mstp_instance = tbl_mstp_instance_get_mstp_instance(&inst_key);
            do {                                                                
                        for ((vid) = 0; (vid) < GLB_BMP_BITS((p_db_mstp_instance->vlan)); (vid)++)  
                        {
                             if (GLB_BMP_ISSET((p_db_mstp_instance->vlan), (vid)))
                             {
                                vlan_key.vid = vid;
                                p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
                                if (NULL == p_db_vlan)
                                {
                                    continue;
                                }
                                
                                if (!GLB_BMP_ISSET(p_db_vlan->member_port, p_g8032_ring->east_port_ifindex) )
                                {
                                    continue;
                                }
                                hsrv_fdb_flush_fdb_port_vlan(p_g8032_ring->east_port_ifindex, vid);
                             }
                        }
                    }while(0);
        }
    }
    else
    {
        if (p_g8032_ring->west_port_ifindex)
        {
             for (i = 0; i < p_g8032_ring->instance_count; i++)
            {
                instance_id = p_g8032_ring->instance[i];
                inst_key.instance = instance_id;
                p_db_mstp_instance = tbl_mstp_instance_get_mstp_instance(&inst_key);
               do {                                                                
                        for ((vid) = 0; (vid) < GLB_BMP_BITS((p_db_mstp_instance->vlan)); (vid)++)  
                        {
                             if (GLB_BMP_ISSET((p_db_mstp_instance->vlan), (vid)))
                             {
                                vlan_key.vid = vid;
                                p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
                                if (NULL == p_db_vlan)
                                {
                                    continue;
                                }
                                
                                if (!GLB_BMP_ISSET(p_db_vlan->member_port, p_g8032_ring->west_port_ifindex) )
                                {
                                    continue;
                                }
                                hsrv_fdb_flush_fdb_port_vlan(p_g8032_ring->west_port_ifindex, vid);
                             }
                        }
                    }while(0);
            }
        }
    }
     return rc;
}

int32
hsrv_g8032_ring_enable(tbl_g8032_ring_t *p_g8032_ring)
{
    int32 rc = HSRV_E_NONE;
    uint32 mcast_group_id;
    tbl_fea_g8032_ring_mc_group_t  *p_fea_g8032_ring_mc_group = NULL;
    tbl_fea_g8032_ring_mc_group_t   fea_g8032_ring_mc_group ;

     sal_memset(&fea_g8032_ring_mc_group, 0, sizeof(fea_g8032_ring_mc_group));
   if (p_g8032_ring->config_sync)
   {
       if (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring)
       {
            p_fea_g8032_ring_mc_group = tbl_fea_g8032_ring_mc_group_get_fea_g8032_ring_mc_group((tbl_fea_g8032_ring_mc_group_key_t *)(&(p_g8032_ring->g8032_major_ring_id)));
            if (p_fea_g8032_ring_mc_group)
            {
               mcast_group_id = p_fea_g8032_ring_mc_group->mc_group_id;
               hsrv_g8032_mgroup_update_member_port(p_g8032_ring->east_port_ifindex, mcast_group_id, TRUE);
               hsrv_g8032_add_aps_group(p_g8032_ring->key.ring_id, p_g8032_ring->east_port_ifindex, mcast_group_id);
            }
       }
       else
       {
           hsrv_l2mc_alloc_mgroupid(&mcast_group_id); 
           
           fea_g8032_ring_mc_group.key.ring_id = p_g8032_ring->key.ring_id;
           fea_g8032_ring_mc_group.mc_group_id = mcast_group_id;
           tbl_fea_g8032_ring_mc_group_add_fea_g8032_ring_mc_group(&fea_g8032_ring_mc_group);
           hsrv_g8032_creat_aps_mgroup(mcast_group_id); 
           hsrv_g8032_mgroup_update_member_port(p_g8032_ring->east_port_ifindex, mcast_group_id, TRUE);
           hsrv_g8032_mgroup_update_member_port(p_g8032_ring->west_port_ifindex, mcast_group_id, TRUE);
           hsrv_g8032_add_aps_group(p_g8032_ring->key.ring_id, p_g8032_ring->east_port_ifindex, mcast_group_id);
           hsrv_g8032_add_aps_group(p_g8032_ring->key.ring_id, p_g8032_ring->west_port_ifindex, mcast_group_id);
       }
        
   }
   else
   {
        if (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring)
        {
              p_fea_g8032_ring_mc_group = tbl_fea_g8032_ring_mc_group_get_fea_g8032_ring_mc_group((tbl_fea_g8032_ring_mc_group_key_t *)(&(p_g8032_ring->g8032_major_ring_id)));
              if (p_fea_g8032_ring_mc_group)
              {
                  hsrv_g8032_mgroup_update_member_port(p_g8032_ring->east_port_ifindex, p_fea_g8032_ring_mc_group->mc_group_id, FALSE);
                  hsrv_g8032_delete_aps_group(p_g8032_ring->key.ring_id, p_g8032_ring->east_port_ifindex, p_fea_g8032_ring_mc_group->mc_group_id);
              }
        }
        else
        {
           p_fea_g8032_ring_mc_group = tbl_fea_g8032_ring_mc_group_get_fea_g8032_ring_mc_group((tbl_fea_g8032_ring_mc_group_key_t *)(&(p_g8032_ring->key.ring_id)));
           if (p_fea_g8032_ring_mc_group)
           {
               hsrv_g8032_mgroup_update_member_port(p_g8032_ring->east_port_ifindex, p_fea_g8032_ring_mc_group->mc_group_id, FALSE);
               hsrv_g8032_mgroup_update_member_port(p_g8032_ring->west_port_ifindex, p_fea_g8032_ring_mc_group->mc_group_id, FALSE);
               hsrv_g8032_delete_aps_group(p_g8032_ring->key.ring_id, p_g8032_ring->east_port_ifindex, p_fea_g8032_ring_mc_group->mc_group_id);
               hsrv_g8032_delete_aps_group(p_g8032_ring->key.ring_id, p_g8032_ring->west_port_ifindex, p_fea_g8032_ring_mc_group->mc_group_id);
               hsrv_g8032_destory_aps_mgroup(p_fea_g8032_ring_mc_group->mc_group_id);
               hsrv_l2mc_free_mgroupid(p_g8032_ring->key.ring_id, p_fea_g8032_ring_mc_group->mc_group_id);
           }
        }
   }
   
   return rc;
}

int32
hsrv_g8032_ring_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = HSRV_E_NONE;
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    switch (oper)
    {
        case CDB_OPER_ADD:
            break;
        case CDB_OPER_DEL:
            break;
        case CDB_OPER_SET:
            p_g8032_ring = (tbl_g8032_ring_t *)p_tbl;
            HSRV_PTR_CHECK(p_g8032_ring);
            switch (field_id)
            {
                case TBL_G8032_RING_FLD_CONFIG_SYNC:
                    hsrv_g8032_ring_enable(p_g8032_ring);
                    break;
                        
                case TBL_G8032_RING_FLD_INSTANCE:
                    hsrv_g8032_bind_instance_to_ring(p_g8032_ring);
                    break;
                    
                case TBL_G8032_RING_FLD_EAST_PORT_IS_BLOCKED:
                    hsrv_g8032_ring_set_port_state(p_g8032_ring, TRUE);
                    break;
                    
                case TBL_G8032_RING_FLD_WEST_PORT_IS_BLOCKED:
                    hsrv_g8032_ring_set_port_state(p_g8032_ring, FALSE);
                    break;
                    
                 case TBL_G8032_RING_FLD_EAST_PORT_FLUSH_FDB:
                     if (p_g8032_ring->east_port_flush_fdb)
                     {
                         hsrv_g8032_ring_flush_fdb_by_port(p_g8032_ring, TRUE);
                     }
                    break;
                    
                 case TBL_G8032_RING_FLD_WEST_PORT_FLUSH_FDB:
                     if (p_g8032_ring->west_port_flush_fdb)
                     {
                         hsrv_g8032_ring_flush_fdb_by_port(p_g8032_ring, FALSE);
                     }
                    break;
                    
                case TBL_G8032_RING_FLD_CONTROL_VLAN:
                    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&(p_g8032_ring->key));
                    if (p_db_g8032_ring->control_vlan && !p_g8032_ring->control_vlan)
                    {
                        hsrv_g8032_delete_control_vlan(p_db_g8032_ring);
                    }
                    else if (!p_db_g8032_ring->control_vlan && p_g8032_ring->control_vlan)
                    {
                        hsrv_g8032_add_control_vlan(p_g8032_ring);
                    }
                    else if(p_db_g8032_ring->control_vlan && p_g8032_ring->control_vlan)
                    {
                        hsrv_g8032_delete_control_vlan(p_db_g8032_ring);
                        hsrv_g8032_add_control_vlan(p_g8032_ring);
                    }
                    
                    break;

                case TBL_G8032_RING_FLD_IS_FWDTOCPU:
                    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&(p_g8032_ring->key));
                    if(p_g8032_ring->control_vlan)
                    {
                        hsrv_g8032_delete_control_vlan(p_db_g8032_ring);
                        hsrv_g8032_add_control_vlan(p_g8032_ring);
                    }
                    break;

                 case TBL_G8032_RING_FLD_APS_PROTECTION_EN:
                   hsrv_g8032_ring_set_aps_protection_en(p_g8032_ring, p_g8032_ring->aps_protection_en);
                    break; 
                default: 
                    break;
            }
            break;
        default:
            break;
    }
    return rc;
}


