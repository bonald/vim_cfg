
#include "proto.h"
#include "ncs.h"
#include "ncs_api.h"

int32
ncs_api_openflow_set_datapath_id(cdb_cfg_t cfg, ncs_switch_t *p_switch)
{
    char cmd[CMD_BUF_256];
    char yang_datapath_id[24];
    char datapath_id[24];
    char *ptr = NULL;
    char *ptr2 = datapath_id;

    sal_memset(yang_datapath_id, 0, sizeof(yang_datapath_id));
    sal_memset(datapath_id, 0, sizeof(datapath_id));
    sal_memset(cmd, 0, sizeof(cmd));

    sal_strcpy(yang_datapath_id, p_switch->datapath_id);

    /* change 00:00:0f:00:00:00:00:01 to 00000f0000000001 */
    ptr = sal_strtok(yang_datapath_id, ":");
    while(ptr)
    {
        sal_sprintf(ptr2, "%s", ptr);
        ptr2 = ptr2 + strlen(ptr);
        ptr= sal_strtok(NULL, ":");
    }
    
    /*
     * user set datapath id cdbctl update/cdb/openflow/openflow/dpid/00000f0000000001
     */
    sal_snprintf(cmd, CMD_BUF_256, "dpid/%s", datapath_id);

    return ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_OPENFLOW, cmd, sal_strlen(cmd)+1);
   
}

int32
ncs_api_openflow_add_controller(cdb_cfg_t cfg, ncs_controller_t *p_controller)
{
    char cmd[CMD_BUF_256];
    uint8 set_source_ip = 0;

    sal_memset(cmd, 0, sizeof(cmd));
    if (sal_strlen(p_controller->local_ip_address))
    {
        set_source_ip = 1;
    }
    
    /*
     * ${addr}/${port}/${protocol}/${mgmt-if}/${bind_addr}/${backoff_value}/${probe_value}/${controller_name}/
     */
    sal_snprintf(cmd, CMD_BUF_256, "%s/%d/%s/%s/%s/%s/%s/%s", p_controller->ip_address, p_controller->l4_port, p_controller->protocol?"ssl":"tcp", 
                    p_controller->mng_flag?"mgmt-if":"", set_source_ip?p_controller->local_ip_address:"", "", "", p_controller->id);

    return ncs_sock_cfg_sync(cfg, CDB_OPER_ADD, CDB_NODE_ACT, ACT_OPENFLOW_SET_CONTROLLER, cmd, sal_strlen(cmd)+1);
   
}

int32
ncs_api_openflow_delete_controller(cdb_cfg_t cfg, ncs_controller_t *p_controller)
{
    char cmd[CMD_BUF_256];

    sal_memset(cmd, 0, sizeof(cmd));
    sal_snprintf(cmd, CMD_BUF_256, "%s/%d/%s", p_controller->ip_address, p_controller->l4_port, p_controller->protocol?"ssl":"tcp");

    return ncs_sock_cfg_sync(cfg, CDB_OPER_DEL, CDB_NODE_ACT, ACT_OPENFLOW_SET_CONTROLLER, cmd, sal_strlen(cmd)+1);
}

int32
ncs_api_openflow_add_interface(cdb_cfg_t cfg, ncs_interface_t *p_interface)
{
    char cmd[CMD_BUF_256];
    int32 rc = 0;
    
    sal_memset(cmd, 0, sizeof(cmd));
    if (!p_interface->is_tunnel)
    {
        /* physical interface already create */
    }
    else
    {
        /*
         * for tunnel interface
         */

        /* 1. create tbl_openflow_tunnel_interface -> cdbctl create/cdb/interface/vxlan2 and create tunnel interface */
        sal_snprintf(cmd, CMD_BUF_256, "%s", p_interface->ifname);
        rc += ncs_sock_cfg_sync(cfg, CDB_OPER_ADD, CDB_NODE_TBL, TBL_INTERFACE, cmd, sal_strlen(cmd)+1);
        
       /* 2. set tunnel interface attr -> cdbctl update/cdb/openflow_tunnel_interface/${iface}/remote_ip/${ip} */
        sal_memset(cmd, 0, sizeof(cmd));
        sal_snprintf(cmd, CMD_BUF_256, "%s/source_ip/%s", p_interface->ifname, p_interface->source_ip);
        rc += ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_OPENFLOW_TUNNEL_INTERFACE, cmd, sal_strlen(cmd)+1);

        sal_memset(cmd, 0, sizeof(cmd));
        sal_snprintf(cmd, CMD_BUF_256, "%s/remote_ip/%s", p_interface->ifname, p_interface->remote_ip);
        rc += ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_OPENFLOW_TUNNEL_INTERFACE, cmd, sal_strlen(cmd)+1);

        if (rc)
        {
            /* delete tunnel interface -> cdbctl delete/cdb/interface/vxlan1 and delete tunnel interface */
            sal_memset(cmd, 0, sizeof(cmd));
            sal_snprintf(cmd, CMD_BUF_256, "%s", p_interface->ifname);
            ncs_sock_cfg_sync(cfg, CDB_OPER_DEL, CDB_NODE_TBL, TBL_INTERFACE, cmd, sal_strlen(cmd)+1);
        }
         
    }

    return rc;
}

int32
ncs_api_openflow_delete_interface(cdb_cfg_t cfg, ncs_interface_t *p_interface)
{
    char cmd[CMD_BUF_256];
    int32 rc = 0;

    sal_memset(cmd, 0, sizeof(cmd));

    if (!p_interface->is_tunnel)
    {
        /*
         * cannot delete physical interface
        */
    }
    else
    {
       /* delete tunnel interface -> cdbctl delete/cdb/interface/vxlan1 and delete tunnel interface */
        sal_memset(cmd, 0, sizeof(cmd));
        sal_snprintf(cmd, CMD_BUF_256, "%s", p_interface->ifname);
        rc = ncs_sock_cfg_sync(cfg, CDB_OPER_DEL, CDB_NODE_TBL, TBL_INTERFACE, cmd, sal_strlen(cmd)+1);
    }

    return rc;
}

int32
ncs_api_openflow_interface_set_enable(cdb_cfg_t cfg, ncs_interface_t *p_interface)
{
    char cmd[CMD_BUF_256];
    int rc = 0;

    sal_memset(cmd, 0, sizeof(cmd));
    /*
     *cdbctl update/cdb/openflow_interface/${iface}/openflow_enable/1
     */
    sal_snprintf(cmd, CMD_BUF_256, "%s/openflow_enable/1", p_interface->ifname);
    rc = ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_OPENFLOW_INTERFACE, cmd, sal_strlen(cmd)+1);
    
    return rc;
}

int32
ncs_api_openflow_interface_set_disable(cdb_cfg_t cfg, ncs_interface_t *p_interface)
{
    char cmd[CMD_BUF_256];
    int rc = 0;

    sal_memset(cmd, 0, sizeof(cmd));
    /*
     *cdbctl update/cdb/openflow_interface/${iface}/openflow_enable/0
     */
    sal_snprintf(cmd, CMD_BUF_256, "%s/openflow_enable/0", p_interface->ifname);
    rc = ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_OPENFLOW_INTERFACE, cmd, sal_strlen(cmd)+1);
    
    return rc;
}


