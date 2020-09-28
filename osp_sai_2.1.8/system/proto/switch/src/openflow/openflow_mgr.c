
#include "openflow_mgr.h"

#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ns_route_define.h"
#include "gen/tbl_ns_route.h"
#include "gen/tbl_inband_snat_define.h"
#include "gen/tbl_inband_snat.h"
#include "if_agg.h"

#define TMP_FNAME_UUID "/tmp/of_uuid"

int32
openflow_agg_create(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    if ((NULL == p_db_agg_if) || (NULL == p_db_mem_if))
    {
        return PM_E_NONE;
    }

    //fei_openflow_port_agg_create(p_db_agg_if->ifindex, TRUE);

    return PM_E_NONE;
}

int32
openflow_agg_destroy(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{

    if (NULL == p_db_agg_if)
    {
        return PM_E_NONE;
    }

    //fei_openflow_port_agg_create(p_db_agg_if->ifindex, FALSE);

    return PM_E_NONE;
}

/* port join agg will effect ingress interface */
int32
openflow_agg_attach_update(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    int32 rc = PM_E_NONE;

    if ((NULL == p_db_agg_if) || (NULL == p_db_mem_if))
    {
        return rc;
    }

    //fei_openflow_port_agg_update_member(p_db_agg_if->ifindex, p_db_mem_if->ifindex, TRUE);

    return rc;
}

/* port join agg will effect ingress interface */
int32
openflow_agg_detach_update(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    int32 rc = PM_E_NONE;

    if ((NULL == p_db_agg_if) || (NULL == p_db_mem_if))
    {
        return rc;
    }

   // fei_openflow_port_agg_update_member(p_db_agg_if->ifindex, p_db_mem_if->ifindex, FALSE);

    return rc;
}

uint32
openflow_get_controller_count()
{
    tbl_ns_route_master_t *p_master = tbl_ns_route_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_ns_route_t *p_db_ns_route = NULL;
    uint32 count = 0;

    CTCLIB_SLIST_LOOP(p_master->ns_route_list, p_db_ns_route, listnode)
    {
        if (NS_ROUTE_FLAG_OPENFLOW_CONTROLLER == p_db_ns_route->flag)
        {
            count++;
        }
    }

    return count;
}

extern int32
ns_route_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);
extern int32
inband_snat_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
_openflow_cmd_add_controller(char *addr, char *port, char *protocol, char *src_ip_addr, char *backoff, char *probe, uint32 inband, char *controller_name, cfg_cmd_para_t *para)
{
    tbl_ns_route_key_t ns_route_key;
    tbl_ns_route_t *p_db_ns_route = NULL;
    char key_str[CMD_BUF_64];
    char *argv[7];
    char cmd[CMD_BUF_256];
    int32 rc = PM_E_NONE;

    /* 1. check */
    sal_memset(&ns_route_key, 0, sizeof(ns_route_key));

    if (0 == sal_strcmp(protocol, "ssl"))
    {
        ns_route_key.proto = NS_ROUTE_SSL;
    }
    else if (0 == sal_strcmp(protocol, "tcp"))
    {
        ns_route_key.proto = NS_ROUTE_TCP;
    }
    else
    {
        CFG_CONFLICT_RET("Invalid protocol");
    }
    PM_CFG_GET_UINT(ns_route_key.port, port, para->p_rs);
    rc = cdb_addr_ipv4_str2val(&ns_route_key.ip.u.prefix4, addr, sal_strlen(addr));
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Invalid IP address");
    }

#if 0
	if [ ${protocol} = "ssl" ]; then
		ssl_key=`ovs-vsctl list ssl | grep "_uuid"`
		if [ -z "$ssl_key" ]; then
			echo "Ssl key hasn't been configurated yet!"
		fi
	fi
#endif

    if (inband)
    {
        if (sal_strlen(src_ip_addr))
        {
            sal_snprintf(cmd, CMD_BUF_256, "cdbctl update/cdb/l3/check_l3if_addr/%s", src_ip_addr);
            rc = sal_system(cmd);
            if (rc != 0)
            {
                CFG_CONFLICT_RET("IP address %s not configured", src_ip_addr);
            }
        }
    }
    else
    {
        sal_snprintf(cmd, CMD_BUF_256, "cdbctl update/cdb/l3/check_mgmt_if_ip/%s", addr);
        rc = sal_system(cmd);
        if (rc != 0)
        {
            CFG_CONFLICT_RET("Address %s on mgmt-if overlaps with inband interfaces", addr);
        }

        if (sal_strlen(src_ip_addr))
        {
            CFG_CONFLICT_RET("Mgmt-if not support set source IP address");
        }
    }

    p_db_ns_route = tbl_ns_route_get_ns_route(&ns_route_key);
    if (p_db_ns_route)
    {
        CFG_CONFLICT_RET("The controller is exist!");
    }
    
    if (openflow_get_controller_count() >= 7)
    {
        CFG_CONFLICT_RET("The number of controller should not exceed 7");
    }
    
    if (CDB_CFG_NC_CHECK == para->cfg)
    {
        return PM_E_NONE;
    }

    /* 2. config */

    /* 2.1 add ns_route */
    cdb_ns_route_key_val2str(key_str, CMD_BUF_64, &ns_route_key);
    argv[0] = key_str;
    argv[1] = "1";
    argv[2] = inband ? "inband" : "outband";
    rc = ns_route_cmd_process(argv, 3, para);
    
    /* 2.2 add inband_snat */
    if (sal_strlen(src_ip_addr))
    {
        argv[0] = key_str;
        argv[1] = "type";
        argv[2] = "1";
        argv[3] = "ifname";
        argv[4] = "";
        argv[5] = "ip";
        argv[6] = src_ip_addr;
        rc = inband_snat_cmd_process(argv, 7, para);
    }

    sal_snprintf(cmd, CMD_BUF_256, "ovs-vsctl -- add bridge br0 controller @m -- --id=@m create controller target=\"%s\\:%s\\:%s\" >/dev/null 2>&1", protocol, addr, port);
    sal_system(cmd);

    sal_task_sleep(500);
    sal_memset(cmd, 0 , sizeof(cmd));
    sal_snprintf(cmd, CMD_BUF_256, "cdbctl create/cdb/openflow/controller_attr/%s/%s/%s/%s/%s/%s >/dev/null 2>&1", addr, port, protocol, backoff, probe, controller_name);
//	cdbctl create/cdb/openflow/controller_attr/${addr}/${port}/${protocol}/${backoff_value}/${probe_value}/controller_name
    sal_system(cmd);

#ifdef HAVE_NETCONF
    /* 3. commit to NETCONF */
    if (CDB_CFG_CLI == para->cfg)
    {
        if (PM_E_NONE == rc)
        {
        }
    }
#endif /* !HAVE_NETCONF */

    return rc;
}

int32
_openflow_cmd_del_controller(char *addr, char *port, char *protocol, cfg_cmd_para_t *para)
{
    tbl_inband_snat_key_t snat_key;
    tbl_ns_route_key_t ns_route_key;
    tbl_inband_snat_t *p_db_snat = NULL;
    tbl_ns_route_t *p_db_ns_route = NULL;
    char key_str[CMD_BUF_64];
    char uuid_str[CMD_BUF_256];
    char *p_uuid = NULL;
    char *argv[1];
    char cmd[CMD_BUF_256];
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    /* 1. check */
    sal_memset(&ns_route_key, 0, sizeof(ns_route_key));
    sal_memset(&snat_key, 0, sizeof(snat_key));

    if (0 == sal_strcmp(protocol, "ssl"))
    {
        ns_route_key.proto = NS_ROUTE_SSL;
        snat_key.proto = NS_ROUTE_SSL;
    }
    else if (0 == sal_strcmp(protocol, "tcp"))
    {
        ns_route_key.proto = NS_ROUTE_TCP;
        snat_key.proto = NS_ROUTE_TCP;
    }
    else
    {
        CFG_CONFLICT_RET("Invalid protocol");
    }
    PM_CFG_GET_UINT(ns_route_key.port, port, para->p_rs);
    rc = cdb_addr_ipv4_str2val(&ns_route_key.ip.u.prefix4, addr, sal_strlen(addr));
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Invalid IP address");
    }

    snat_key.proto = ns_route_key.proto;
    snat_key.port = ns_route_key.port;
    snat_key.ip = ns_route_key.ip.u.prefix4;
    p_db_ns_route = tbl_ns_route_get_ns_route(&ns_route_key);
    if (NULL == p_db_ns_route)
    {
        CFG_CONFLICT_RET("The controller is not exist!");
    }
    if (NS_ROUTE_FLAG_OPENFLOW_CONTROLLER != p_db_ns_route->flag)
    {
        CFG_CONFLICT_RET("The controller is not exist!");
    }
    
    if (CDB_CFG_NC_CHECK == para->cfg)
    {
        return PM_E_NONE;
    }

    /* 2. config */

    /* 2.1 add ns_route */
    cdb_ns_route_key_val2str(key_str, CMD_BUF_64, &ns_route_key);
    argv[0] = key_str;
    rc = ns_route_cmd_process(argv, 1, para);

    /* 2.2 add inband_snat */
    p_db_snat = tbl_inband_snat_get_inband_snat(&snat_key);
    if (p_db_snat)
    {
        argv[0] = key_str;
        inband_snat_cmd_process(argv, 1, para);
    }
    
    sal_snprintf(cmd, CMD_BUF_256, "ovs-vsctl  --columns=_uuid find controller target=\"%s\\:%s\\:%s\" > %s 2>&1", protocol, addr, port, TMP_FNAME_UUID);
    sal_system(cmd);

    fp = sal_fopen(TMP_FNAME_UUID, "r");
    sal_fgets(uuid_str, CMD_BUF_256, fp);
    p_uuid = sal_strstr(uuid_str, ": ");
    if (p_uuid)
    {
        p_uuid += sal_strlen(": ");
        sal_snprintf(cmd, CMD_BUF_256, "ovs-vsctl -- remove bridge br0 controller %s >/dev/null 2>&1", p_uuid);
        sal_system(cmd);
    }
    sal_fclose(fp);
    sal_unlink(TMP_FNAME_UUID);

    /* delete controller attr */
    sal_memset(cmd, 0, sizeof(cmd));
    sal_snprintf(cmd, CMD_BUF_256, "cdbctl delete/cdb/openflow/controller_attr/%s/%s/%s >/dev/null 2>&1", addr, port, protocol);
    sal_system(cmd);
    
#ifdef HAVE_NETCONF
    /* 3. commit to NETCONF */
    if (CDB_CFG_CLI == para->cfg)
    {
        if (PM_E_NONE == rc)
        {
        }
    }
#endif /* !HAVE_NETCONF */

    return rc;
}

int32
openflow_cmd_process_set_controller(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char *addr = NULL;
    char *port = NULL;
    char *protocol = NULL;
    char *src_ip_addr = NULL;
    char *backoff = NULL;
    char *probe = NULL;
    char *controller_name = NULL;
    uint32 inband = FALSE;
    
    int32 rc = PM_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        //the last one argument is controller_name
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 8, para->p_rs);
        addr = argv[0];
        port = argv[1];
        protocol = argv[2];
        src_ip_addr = argv[4];
        backoff = argv[5];
        probe = argv[6];
        controller_name = argv[7];
        inband = (0 == sal_strcmp(argv[3], "mgmt-if")) ? FALSE : TRUE;
        PM_E_RETURN(_openflow_cmd_add_controller(addr, port, protocol, src_ip_addr, backoff, probe, inband, controller_name, para));
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
        addr = argv[0];
        port = argv[1];
        protocol = argv[2];
        PM_E_RETURN(_openflow_cmd_del_controller(addr, port, protocol, para));
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }

    return rc;
}

int32
openflow_mgr_start(switch_master_t *master)
{
#if 0
    openflow_init();

    cdb_register_cfg_tbl_cb(TBL_OPENFLOW, openflow_mgr_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_OPENFLOW_FLOW, openflow_mgr_flow_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_OPENFLOW_METER, openflow_mgr_meter_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_OPENFLOW_GROUP, openflow_mgr_group_cmd_process);

    ipc_register_hal_rx_fn(SWITCH_OPENFLOW_HAL_MSG_TYPE_FLOW_STATS, _openflow_mgr_refresh_flow_stats_handle);
    ipc_register_hal_rx_fn(SWITCH_OPENFLOW_HAL_MSG_TYPE_METER_STATS, _openflow_mgr_refresh_meter_stats_handle);
    ipc_register_hal_rx_fn(SWITCH_OPENFLOW_HAL_MSG_TYPE_GROUP_STATS, _openflow_mgr_refresh_group_stats_handle);
#endif

#ifdef OPENFLOW_V580
    if_agg_create_agg_sync_prop_set_cb(IF_AGG_CB_MODULE_OPENFLOW,  openflow_agg_create);
    if_agg_destory_agg_sync_prop_set_cb(IF_AGG_CB_MODULE_OPENFLOW, openflow_agg_destroy);

    if_agg_attach_member_sync_prop_set_cb(IF_AGG_CB_MODULE_OPENFLOW,  openflow_agg_attach_update);
    if_agg_detach_member_sync_prop_set_cb(IF_AGG_CB_MODULE_OPENFLOW,  openflow_agg_detach_update);
#endif /* OFPRODUCT */

    cdb_register_cfg_act_cb(ACT_OPENFLOW_SET_CONTROLLER, openflow_cmd_process_set_controller);

    return PM_E_NONE;
}

int32
openflow_mgr_stop(switch_master_t *master)
{
#if 0
    cdb_register_cfg_tbl_cb(TBL_OPENFLOW, NULL);
    cdb_register_cfg_tbl_cb(TBL_OPENFLOW_FLOW, NULL);
    cdb_register_cfg_tbl_cb(TBL_OPENFLOW_METER, NULL);
    cdb_register_cfg_tbl_cb(TBL_OPENFLOW_GROUP, NULL);

    ipc_register_hal_rx_fn(SWITCH_OPENFLOW_HAL_MSG_TYPE_FLOW_STATS, NULL);
    ipc_register_hal_rx_fn(SWITCH_OPENFLOW_HAL_MSG_TYPE_METER_STATS, NULL);
    ipc_register_hal_rx_fn(SWITCH_OPENFLOW_HAL_MSG_TYPE_GROUP_STATS, NULL);
#endif

    return PM_E_NONE;
}
