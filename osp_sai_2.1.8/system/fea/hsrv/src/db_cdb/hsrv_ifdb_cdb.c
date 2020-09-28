#include "hsrv_inc.h"
#include "glb_openflow_define.h"
#include "gen/stbl_acl_entry_define.h"
#include "ctc_sai_acl.h"
#include "hsrv_openflow.h"

extern int32 hsrv_openflow_msg_tunnel_port_decap_en(fei_openflow_port_decap_req_t* pst_req);

extern int32
ctc_routerintf_alloc_l3if_id(uint32_t *l3if_id);

extern sai_status_t
ctc_sai_port_objectid_to_classid(sai_object_id_t port_oid, uint16 *pclassid);

extern int32_t
ctc_sai_acl_get_group_id_by_class_id(sai_uint32_t class_id, uint32 dirction);

extern sai_status_t
ctc_sai_port_objectid_to_gport(sai_object_id_t port_oid, uint32 *pgport);

extern sai_status_t
ctc_sai_port_gport_to_objectid(uint32 gport, sai_object_id_t *pst_port_oid);

extern int32
ctc_sai_mgroup_id_opf_alloc(uint32* p_index);

extern int32
ctc_sai_mgroup_id_opf_free(uint32 opf_index);

extern int32
ctc_sai_nexthop_free_offset(uint32 index);

extern int32
ctc_sai_nexthop_alloc_offset(uint32* p_index);

int32
hsrv_ifdb_get_portid(uint32 ifindex, uint16* p_portid)
{
    sai_object_id_t port_oid;
    uint32  gport = 0;
    int32 rc = 0;
    rc = hsrv_interface_ifindex_to_objectid(ifindex, &port_oid);
    if (rc < 0)
    {
        *p_portid = 0xFFFF;
        return HSRV_E_FAIL;
    }
    ctc_sai_port_objectid_to_gport(port_oid, &gport);
    *p_portid = gport;

    return HSRV_E_NONE;
}

int32
hsrv_ifdb_get_agg_portid(uint32 ifindex, uint16* p_portid)
{  
    sai_object_id_t port_oid;
    uint32  gport = 0;
    hsrv_interface_ifindex_to_objectid(ifindex, &port_oid);
    ctc_sai_port_objectid_to_gport(port_oid, &gport);
    *p_portid = gport;

    return HSRV_E_NONE;
}

int32
hsrv_ifdb_get_ifindex_by_gportid(uint16 gportid, uint32* p_ifindex)
{
    sai_object_id_t cdb_port_id = 0;
    tbl_interface_t* p_tbl_interface = NULL;

    ctc_sai_port_gport_to_objectid(gportid, &cdb_port_id);
    hsrv_interface_get_if_by_oid(cdb_port_id, &p_tbl_interface);
    if(p_tbl_interface)
    {
        *p_ifindex = p_tbl_interface->ifindex;
    }
    return HSRV_E_NONE;
}

uint32
hsrv_ifdb_get_agg_gport_by_tid(uint8 tid)
{
    uint32 bond_ifindex;
    sai_object_id_t port_oid;
    uint32 gport = 0;
    bond_ifindex =tid + GLB_BOND_IFINDEX_BASE;

    hsrv_interface_ifindex_to_objectid(bond_ifindex, &port_oid);

    ctc_sai_port_objectid_to_gport(port_oid, &gport);

    return gport;
}

int32
hsrv_ifdb_new_l3ifid(glb_if_type_t iftype, uint32* p_l3ifid)
{
    ctc_routerintf_alloc_l3if_id(p_l3ifid);

    return HSRV_E_NONE;
}

#ifndef TTPPRODUCT
int32
hsrv_openflow_cdb_bond_set_member_tunnel_type(uint32 ifindex, void *user_data)
{
    tbl_interface_master_t *p_master = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_interface_t *p_db_if = NULL;
    uint32 tid = 0;
    int32 ret = 0;
    fei_openflow_port_decap_req_t* pst_req = user_data;

    tid = ifindex - GLB_BOND_IFINDEX_BASE;
    p_master = tbl_interface_get_master();
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, listnode, next)
    {
        if(p_db_if->ifindex < GLB_BOND_IFINDEX_BASE && NULL != p_db_if->lag)
        {
            if(p_db_if->lag->lag_id == tid)
            {
                pst_req->ifindex = p_db_if->ifindex;
                hsrv_openflow_msg_tunnel_port_decap_en(pst_req);
            }
        }
    }

    return ret;
}
#endif

int32
hsrv_openflow_cdb_bond_update_slave(uint32 bond_ifindex, uint32 slave_ifindex, bool enable)
{
    tbl_interface_t* p_tbl_interface = NULL;
    char cmd[MAX_CMD_LINE_LEN] = {0};
    uint32 tid = 0;
    FILE * fp_console = NULL; 
    fp_console = fopen("/dev/console", "a+"); 

    tid = bond_ifindex - GLB_BOND_IFINDEX_BASE;
    p_tbl_interface = tbl_interface_get_interface_by_ifindex(slave_ifindex);
    if(NULL == p_tbl_interface)
    {
        return -1;
    }
    if(enable)
    {
        sal_sprintf(cmd, "cdbctl create/cdb/interface/%s/lag/%d/mode/static", p_tbl_interface->key.name, tid);
        fprintf(fp_console, "cli:%s\n",cmd);
        sal_system(cmd);
    }
    else
    {
        sal_sprintf(cmd, "cdbctl delete/cdb/interface/%s/lag/static", p_tbl_interface->key.name);
        fprintf(fp_console, "cli:%s\n",cmd);
        sal_system(cmd);
    }
    fclose(fp_console); 
    return 0;
}

int32
hsrv_openflow_cdb_bond_remove(uint32 ifindex)
{
    tbl_interface_master_t *p_master = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_interface_t *p_db_if = NULL;
    uint32 tid = 0;
    char cmd[MAX_CMD_LINE_LEN] = {0};
    int32 ret = 0;

    tid = ifindex - GLB_BOND_IFINDEX_BASE;
    p_master = tbl_interface_get_master();
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, listnode, next)
    {
        if(NULL != p_db_if->lag)
        {
            if(p_db_if->lag->lag_id == tid)
            {
                sal_sprintf(cmd, "cdbctl delete/cdb/interface/%s/lag/static", p_db_if->key.name);
                ret |= sal_system(cmd);
            }
        }
    }

    return ret;
}

bool
hsrv_openflow_cdb_port_is_bond(uint32 ifindex)
{
    tbl_interface_t* p_interface = NULL;
    if((ifindex > GLB_AGG_IFINDEX_MIN) && (ifindex < GLB_AGG_IFINDEX_MAX))
    {
        p_interface = tbl_interface_get_interface_by_ifindex(ifindex);
        if(p_interface)
        {
            return TRUE;
        }
    }

    return FALSE;
}

int32
hsrv_openflow_port_get_label(uint32 ifindex, uint32* p_label)
{
    tbl_interface_t* p_tbl_interface = NULL;
    uint16 label = 0;
    int ret = HSRV_E_NONE;
    sai_object_id_t port_oid = 0;

    hsrv_interface_ifindex_to_objectid(ifindex, &port_oid);
    p_tbl_interface = tbl_interface_get_interface_by_ifindex(ifindex);
    if(p_tbl_interface)
    {
       ret = ctc_sai_port_objectid_to_classid(port_oid, &label);
       if(ret)
       {
           return ret;
       }
    }
    *p_label = label;
    return HSRV_E_NONE;
}

int32
_hsrv_openflow_get_group_id_by_ifindex_class_id(uint32 ifindex, uint32 class_id, uint32 dirction)
{
    uint32 group_id = 0;
    uint16  gport = 0;
    
    if(class_id ==HSRV_OPENFLOW_GLOBAL_LABEL_BASE)
    {
        return SAI_ACL_WORM_FILTER_GROUP;
    }
    if(class_id < 64) /*CTC_MAX_LINKAGG_GROUP_NUM*/
    {
        hsrv_ifdb_get_agg_portid(ifindex, &gport);
        group_id = CTC_GPORT_LINKAGG_ID(gport);
        return (dirction == CTC_INGRESS) ? (group_id) : (group_id + 64); /*CTC_SAI_PORT_GROUP_LAG_NUM*/
    }
    return ctc_sai_acl_get_group_id_by_class_id(class_id, dirction);
}

uint32 
hsrv_openflow_phyport_get_link_status(uint32 ifindex)
{
    tbl_interface_t* p_tbl_interface = NULL;
    p_tbl_interface = tbl_interface_get_interface_by_ifindex(ifindex);
    if(p_tbl_interface)
    {
        return tbl_interface_is_running(p_tbl_interface);
    }

    return HSRV_E_NONE;
}

uint32 
hsrv_openflow_agg_get_link_status(uint32 ifindex)
{
    tbl_interface_t* p_tbl_interface = NULL;
    p_tbl_interface = tbl_interface_get_interface_by_ifindex(ifindex);
    if(p_tbl_interface)
    {
        return tbl_interface_is_running(p_tbl_interface);
    }

    return HSRV_E_NONE;
}

uint32
hsrv_openflow_phyport_is_openflow_port(uint32 ifindex)
{
    tbl_interface_t* p_tbl_interface = NULL;
    tbl_openflow_interface_t* p_tbl_openflow_interface = NULL;
    tbl_openflow_interface_key_t openflow_if_key;

    p_tbl_interface = tbl_interface_get_interface_by_ifindex(ifindex);
    if(p_tbl_interface)
    {
        sal_memset(&openflow_if_key, 0x0, sizeof(tbl_openflow_interface_key_t));
        sal_strcpy(openflow_if_key.name, p_tbl_interface->key.name);
        p_tbl_openflow_interface = tbl_openflow_interface_get_openflow_interface(&openflow_if_key);
        if(p_tbl_openflow_interface)
        {
            return p_tbl_openflow_interface->openflow_instance_enable;
        }
    }

    return FALSE;
}

uint32
hsrv_openflow_phyport_is_openflow_port_by_portid(uint64 port_id)
{
    tbl_interface_t* p_tbl_interface = NULL;
    tbl_openflow_interface_t* p_tbl_openflow_interface = NULL;
    tbl_openflow_interface_key_t openflow_if_key;

    p_tbl_interface = tbl_interface_get_interface_by_port_id(port_id);
    if(p_tbl_interface)
    {
        sal_memset(&openflow_if_key, 0x0, sizeof(tbl_openflow_interface_key_t));
        sal_strcpy(openflow_if_key.name, p_tbl_interface->key.name);
        p_tbl_openflow_interface = tbl_openflow_interface_get_openflow_interface(&openflow_if_key);
        if(p_tbl_openflow_interface)
        {
            return p_tbl_openflow_interface->openflow_instance_enable;
        }
    }

    return FALSE;
}



int32
hsrv_ifdb_do_listloop(glb_if_type_t iftype, HSRV_IFDB_LISTLOOP_CB_FUNC func, void* pv_arg)
{
    tbl_interface_master_t *p_master = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    p_master = tbl_interface_get_master();
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, listnode, next)
    {
        rc |= func(p_db_if->ifindex , pv_arg);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

bool
hsrv_openflow_port_is_slave(uint32 ifindex)
{
    tbl_interface_t* p_tbl_interface = NULL;

    p_tbl_interface = tbl_interface_get_interface_by_ifindex(ifindex);
    if(p_tbl_interface)
    {
        if(p_tbl_interface->lag)
        {
            return TRUE;
        }
    }

    return FALSE;
}

int32
hsrv_openflow_port_set_instance_enable(uint32 ifindex, uint32 instance_enable)
{
    tbl_interface_t* p_tbl_interface = NULL;
    tbl_openflow_interface_t* p_tbl_openflow_interface = NULL;
    tbl_openflow_interface_key_t openflow_if_key;

    p_tbl_interface = tbl_interface_get_interface_by_ifindex(ifindex);
    if(p_tbl_interface)
    {
        sal_memset(&openflow_if_key, 0x0, sizeof(tbl_openflow_interface_key_t));
        sal_strcpy(openflow_if_key.name, p_tbl_interface->key.name);
        p_tbl_openflow_interface = tbl_openflow_interface_get_openflow_interface(&openflow_if_key);
        if(p_tbl_openflow_interface)
        {
            if(p_tbl_openflow_interface->openflow_instance_enable != instance_enable)
            {
                p_tbl_openflow_interface->openflow_instance_enable = instance_enable;
                tbl_openflow_interface_set_openflow_interface_field(p_tbl_openflow_interface, TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_INSTANCE_ENABLE);
            }
        }
    }

    return 0;
}

int32
hsrv_openflow_port_add_to_bridge_by_cdb(uint32 ifindex, uint32 enable)
{
    tbl_interface_t* p_tbl_interface = NULL;
    tbl_openflow_interface_t* p_tbl_openflow_interface = NULL;
    tbl_openflow_interface_key_t openflow_if_key;

    p_tbl_interface = tbl_interface_get_interface_by_ifindex(ifindex);
    if(p_tbl_interface)
    {
        sal_memset(&openflow_if_key, 0x0, sizeof(tbl_openflow_interface_key_t));
        sal_strcpy(openflow_if_key.name, p_tbl_interface->key.name);
        p_tbl_openflow_interface = tbl_openflow_interface_get_openflow_interface(&openflow_if_key);
        if(p_tbl_openflow_interface)
        {
            if(p_tbl_openflow_interface->is_add_to_br0 != enable)
            {
                p_tbl_openflow_interface->is_add_to_br0 = enable;
                tbl_openflow_interface_set_openflow_interface_field(p_tbl_openflow_interface, TBL_OPENFLOW_INTERFACE_FLD_IS_ADD_TO_BR0);
            }
        }
    }

    return 0;
}

int32
hsrv_openflow_nexthop_id_opf_alloc_by_sai(uint32* p_nexthop_id)
{
    return ctc_sai_nexthop_alloc_offset(p_nexthop_id);
}

int32
hsrv_openflow_nexthop_id_opf_release_by_sai(uint32 nexthop_id)
{
    return ctc_sai_nexthop_free_offset(nexthop_id);
}

int32
hsrv_openflow_mgroup_id_opf_alloc_by_sai(uint32* p_group_id)
{
    return ctc_sai_mgroup_id_opf_alloc(p_group_id);
}

int32
hsrv_openflow_mgroup_id_opf_free_by_sai(uint32 group_id)
{
    return ctc_sai_mgroup_id_opf_free(group_id);
}

int32
hsrv_openflow_get_port_mac_by_gport(uint32 gport, uint8 *mac)
{
    sai_object_id_t cdb_port_id = 0;
    tbl_interface_t* p_tbl_interface = NULL;

    ctc_sai_port_gport_to_objectid(gport, &cdb_port_id);

    hsrv_interface_get_if_by_oid(cdb_port_id, &p_tbl_interface);
    if (NULL == p_tbl_interface)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    sal_memcpy(mac, &p_tbl_interface->hw_mac_addr, sizeof(mac_addr_t));

    return HSRV_E_NONE;
}

void
hsrv_openflow_get_vxlan_src_port(uint32* p_is_set, uint32* p_vxlan_srcport)
{
     tbl_openflow_t* p_openflow = tbl_openflow_get_openflow();
     if(p_openflow->vxlan_srcport_is_set)
     {
        *p_is_set = 1;
        *p_vxlan_srcport = p_openflow->vxlan_srcport;
     }
}

void
hsrv_openflow_get_vxlan_dest_port(uint32* p_is_set, uint32* p_vxlan_destport)
{
     tbl_openflow_t* p_openflow = tbl_openflow_get_openflow();
     if(p_openflow->vxlan_destport_is_set)
     {
        *p_is_set = 1;
        *p_vxlan_destport = p_openflow->vxlan_destport;
     }
     else
    {
        *p_is_set = 0;
        *p_vxlan_destport = 4789;
    }
}

int32
hsrv_openflow_flow_is_mac_learning_enable(void)
{
    tbl_openflow_t*   p_glb_openflow = NULL;
    p_glb_openflow = tbl_openflow_get_openflow();
    if(p_glb_openflow)
    {
        return p_glb_openflow->openflow_mac_learning_enable;
    }
    return 0;
}


