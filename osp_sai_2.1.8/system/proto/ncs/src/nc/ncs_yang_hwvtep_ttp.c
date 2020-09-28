
#ifdef HAVE_NETCONF

#include "proto.h"
#include "ncutil.h"
#include "ncs.h"
#include "ncs_api.h"

#define CMD_BUF_256 256
#define TTP_NETCONF_TMP_FILE "/tmp/ttp_netconf_tmp_file"

extern sr_session_ctx_t *sr_session;
static uint32 switch_num = 0;
ncs_switch_master_t *switch_master = NULL;
uint32 first_add = 1;

bool 
ncs_sxpath_in_dxpath(char *s_xpath, char *d_xpath)
{
    sr_xpath_ctx_t state = {0};
    char *res = NULL;
    char tmp_xpath[1024] = {0};
    char *p = tmp_xpath;

    if (!s_xpath || !d_xpath)
    {
        return FALSE;
    }

    res = sr_xpath_next_node_with_ns(d_xpath, &state);
    if (!res)
    {
        return FALSE;
    }

    do
    {
        sal_sprintf(p, "/%s", res);
        p += sal_strlen(res) + 1;
    }while((res = sr_xpath_next_node_with_ns(NULL, &state)) != NULL);

    /* recover */
    sr_xpath_recover(&state);

    if (0 != sal_strcmp(s_xpath, tmp_xpath))
    {
        return FALSE;
    }

    return TRUE;
}

#define ____CONFIG_MODULE____

int
ncs_hwvtep_ttp_module_change_cb(sr_session_ctx_t *srs, const char *module_name,
        sr_notif_event_t event, void *private_ctx)
{
    return SR_ERR_OK;
}

#define ____CONFIG_SWITCH_FUN____

int
ncs_hwvtep_ttp_switch_create_capable_switch()
{
    char xpath[1024];
    int rc;

    sal_memset(xpath, 0, sizeof(xpath));

    sal_sprintf(xpath, "/hwvtep-ttp:capable-switch/id");

    
    //set switch datapath id
    if ((rc = ctc_nc_set_item_str(NULL, xpath, "capable-switch-0", SR_EDIT_DEFAULT)) == SR_ERR_OK) 
    {
        rc = ctc_nc_commit_no_verify(NULL);
    }

    return rc;
}

int
ncs_hwvtep_ttp_switch_create_capable_switch_port_resource(char *full_if_name)
{
    char xpath[1024];
    int rc;

    sal_memset(xpath, 0, sizeof(xpath));

    sal_sprintf(xpath, "/hwvtep-ttp:capable-switch/resources/port[resource-id='%s']", full_if_name);

    
    //set switch datapath id
    if ((rc = ctc_nc_set_item_str(NULL, xpath, NULL, SR_EDIT_DEFAULT)) == SR_ERR_OK) 
    {
        rc = ctc_nc_commit_no_verify(NULL);
    }

    return rc;
}

static void
ncs_hwvtep_ttp_switch_set_datapath_id(void *p_data)
{
    ncs_switch_t *p_switch = (ncs_switch_t*)p_data;
    char xpath[1024];
    int rc;
    FILE *fp = NULL;
    char cmd[CMD_BUF_256];
    char datapath_id[24];
    char yang_datapath_id[24];
    int32 i = 0, j = 0;
    
    sal_memset(cmd, 0, sizeof(cmd));
    sal_memset(datapath_id, 0, sizeof(datapath_id));
    sal_memset(yang_datapath_id, 0, sizeof(yang_datapath_id));
    sal_memset(xpath, 0, sizeof(xpath));
    
    sal_snprintf(cmd, CMD_BUF_256, "ovs-vsctl  --columns=datapath_id find bridge name=br0 | awk '{print $3}' > %s  2>&1", TTP_NETCONF_TMP_FILE);
    sal_system(cmd);

    fp = sal_fopen(TTP_NETCONF_TMP_FILE, "r");
    //"0000f0000000001"\n include ""
    sal_fgets(datapath_id, 24, fp);
    sal_fclose(fp);
    sal_unlink(TTP_NETCONF_TMP_FILE);

    sal_sprintf(xpath, "/hwvtep-ttp:capable-switch/logical-switches/switch[id='%s']/datapath-id", p_switch->id);

    if (!sal_strlen(datapath_id))
    {
        return;
    }
    /*trim '\n' in datapath_id*/
    datapath_id[sal_strlen(datapath_id)-1] = '\0';

    //change 00000f0000000001 to 00:00:0f:00:00:00:00:01
    for (i = 1; i < sal_strlen(datapath_id)-1; i++)
    {
        if (i%2 == 1 && i != 1)
        {
            yang_datapath_id[j++] = ':';
        }
        yang_datapath_id[j] = datapath_id[i];
        j ++;
    }
    
    //set switch datapath id
    if ((rc = ctc_nc_set_item_str(NULL, xpath, yang_datapath_id, SR_EDIT_DEFAULT)) == SR_ERR_OK) 
        {
            rc = ctc_nc_commit_no_verify(NULL);
        }

    sal_free(p_data);
}


static int
ncs_hwvtep_ttp_switch_change_process(sr_session_ctx_t *srs,sr_notif_event_t ev, sr_change_oper_t op,
    sr_val_t *old_val, sr_val_t *new_val, ncs_switch_t *p_switch)
{
    int rc = SR_ERR_OK;
    sr_val_t *tmp;

    switch(op) {
    case SR_OP_CREATED:
        if (NULL != new_val) {
           tmp = new_val;
        }
        break;
    case SR_OP_DELETED:
        if (NULL != old_val) {
            tmp = old_val;
        }
        break;
    case SR_OP_MODIFIED:
        //not support now
        //return SR_ERR_UNSUPPORTED;
        if (NULL != old_val && NULL != new_val) {
            tmp = new_val;
        }
        break;
    case SR_OP_MOVED:
        //not support now
        return SR_ERR_UNSUPPORTED;
        if (NULL != new_val) {
        }
        break;
    default:
        rc = SR_ERR_INVAL_ARG;
        break;
    }

    if (NULL != tmp && (SR_OP_CREATED == op || SR_OP_DELETED == op || SR_OP_MODIFIED == op))
    {
        if (ncs_sxpath_in_dxpath("/hwvtep-ttp:capable-switch/logical-switches/switch/id", tmp->xpath))
        {
            sal_strcpy(p_switch->id, tmp->data.string_val);
        }
        else if (ncs_sxpath_in_dxpath("/hwvtep-ttp:capable-switch/logical-switches/switch/datapath-id", tmp->xpath))
        {
            sal_strcpy(p_switch->datapath_id, tmp->data.string_val);
        }
        else if(ncs_sxpath_in_dxpath("/hwvtep-ttp:capable-switch/logical-switches/switch/lost-connection-behavior", tmp->xpath))
        {
            if (sal_strcmp(tmp->data.enum_val, "failSecureMode") == 0)
            {
                //failSecureMode
                p_switch->lost_connection_behavior = 0;
            }
            else
            {
                //failStandaloneMode
                p_switch->lost_connection_behavior = 1;
            }
        }
    }

    return rc;
}

int
ncs_hwvtep_ttp_switch_list_change_cb(sr_session_ctx_t *srs, const char *xpath, 
    sr_notif_event_t event, void *private_ctx) 
{
    int rc;
    sr_change_iter_t *it = NULL;
    sr_val_t *new_val = NULL;
    sr_val_t *old_val = NULL;
    sr_change_oper_t op;
    ncs_switch_t p_switch;
    ncs_switch_t *task_switch;

    sal_memset(&p_switch, 0, sizeof(ncs_switch_t));

    rc = ctc_nc_get_changes_iter(srs, "/hwvtep-ttp:capable-switch/logical-switches/switch", &it);
    if (SR_ERR_OK != rc) {
        //puts("sr get changes iter failed");
        goto err_out;
    }

    while (SR_ERR_OK == (rc = ctc_nc_get_change_next(srs,
            it, &op, &old_val, &new_val))) {
        rc = ncs_hwvtep_ttp_switch_change_process(srs, event, op, old_val, new_val, &p_switch);
        if (SR_ERR_OK != rc) {
            goto err_out;
        }
        if (NULL != old_val) {
            sr_free_val(old_val);
            old_val = NULL;
        }
        if (NULL != new_val) {
            sr_free_val(new_val);
            new_val = NULL;
        }
    } /* while */

    if (SR_EV_VERIFY == event && (SR_OP_CREATED == op || SR_OP_MODIFIED == op))
    {
        if (0 != sal_strlen(p_switch.datapath_id))
        {
            //user set datapath id cdbctl update/cdb/openflow/openflow/dpid/00000f0000000001
            rc = ncs_api_openflow_set_datapath_id(CDB_CFG_NC_CHECK, &p_switch);
            if (rc)
            {
                rc = SR_ERR_INVAL_ARG;
                goto err_out;
            }
        }
    }
    else if (SR_EV_APPLY == event && (SR_OP_CREATED == op || SR_OP_MODIFIED == op))
    {
        if (0 != sal_strlen(p_switch.datapath_id))
        {
            //user set datapath id cdbctl update/cdb/openflow/openflow/dpid/00000f0000000001
            rc = ncs_api_openflow_set_datapath_id(CDB_CFG_NC_SET, &p_switch);
            if (rc)
            {
                rc = SR_ERR_INVAL_ARG;
                goto err_out;
            }
        }
    }

    if (SR_EV_VERIFY == event && SR_OP_CREATED == op) 
    {
        //create switch if switch id not null
        if (0 != sal_strlen(p_switch.id))
        {
            //only support create 1 switch now
            if (switch_num + 1 > 1)
            {
                rc = SR_ERR_OPERATION_FAILED;
                goto err_out;
            }

            if (0 != sal_strlen(p_switch.datapath_id))
            {
                #if 0
                //user set datapath id cdbctl update/cdb/openflow/openflow/dpid/00000f0000000001
                rc = ncs_api_openflow_set_datapath_id(CDB_CFG_NC_CHECK, &p_switch);
                if (rc)
                {
                    rc = SR_ERR_INVAL_ARG;
                    goto err_out;
                }
                #endif
            }
        }
    }
    else if (SR_EV_VERIFY == event && SR_OP_DELETED == op)
    {

    }
    else if (SR_EV_APPLY == event && SR_OP_CREATED == op)
    {
        if (0 != sal_strlen(p_switch.id))
        {
            switch_num ++;
            if (0 == sal_strlen(p_switch.datapath_id))
            {
                //if user does not set datapath id, return datapath id
                task_switch = (ncs_switch_t*)sal_malloc(sizeof(ncs_switch_t));
                sal_memset(task_switch, 0 , sizeof(ncs_switch_t));
                sal_strcpy(task_switch->id, p_switch.id);
                ctc_task_add_timer_msec(TASK_PRI_LOW, ncs_hwvtep_ttp_switch_set_datapath_id, (void*)task_switch, 500);
            }
            else
            {
                #if 0
                //user set datapath id cdbctl update/cdb/openflow/openflow/dpid/00000f0000000001
                rc = ncs_api_openflow_set_datapath_id(CDB_CFG_NC_SET, &p_switch);
                if (rc)
                {
                    rc = SR_ERR_INVAL_ARG;
                    goto err_out;
                }
                #endif
            }
        }
    }
    else if (SR_EV_APPLY == event && SR_OP_DELETED == op)
    {
        //delete switch if switch id not null
        if (0 != sal_strlen(p_switch.id))
        {
            if (switch_num > 0)
            {
                switch_num --;
            }
        }
    }
    else if (SR_EV_ABORT == event)
    {
        #if 0
         if (switch_num > 0)
         {
            switch_num --;
         }
         #endif
    }
    else if (SR_EV_ENABLED == event) 
    {
        /* Startup stage: subscription establish event
         *   XXX: copy from startup config to running config
         */
    }

    rc = SR_ERR_OK;

err_out:
    if (NULL != old_val) {
        sr_free_val(old_val);
        old_val = NULL;
    }
    if (NULL != new_val) {
        sr_free_val(new_val);
        new_val = NULL;
    }
    if (NULL != it) {
        sr_free_change_iter(it);
    }
    return rc;
    
}


#define ____CONFIG_CONTROLLER_FUN____

static int
ncs_hwvtep_ttp_controller_change_process(sr_session_ctx_t *srs,sr_notif_event_t ev, sr_change_oper_t op,
    sr_val_t *old_val, sr_val_t *new_val, ncs_controller_t *p_controller, char *next_controller_name)
{
    int rc = SR_ERR_OK;
    sr_val_t *tmp;

#if 1
    if (NULL != new_val)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "operation:%s, event:%s, xpath:%s\n", op==SR_OP_CREATED?"create":op==SR_OP_MODIFIED?"modify":op==SR_OP_DELETED?"delete":op==SR_OP_MOVED?"move":"unknown"
            , ev==SR_EV_VERIFY?"verify":ev==SR_EV_APPLY?"apply":ev==SR_EV_ABORT?"abort":"unknown" ,new_val->xpath);
    }
#endif

    switch(op) {
    case SR_OP_CREATED:
        if (NULL != new_val) {
           tmp = new_val;
        }
        break;
    case SR_OP_DELETED:
        if (NULL != old_val) {
            tmp = old_val;
        }
        break;
    case SR_OP_MODIFIED:
        //not support now
        return SR_ERR_UNSUPPORTED;
        if (NULL != old_val && NULL != new_val) {
        }
        break;
    case SR_OP_MOVED:
        //not support now
        return SR_ERR_UNSUPPORTED;
        if (NULL != new_val) {
        }
        break;
    default:
        rc = SR_ERR_INVAL_ARG;
        break;
    }

    if (NULL != tmp && (SR_OP_CREATED == op || SR_OP_DELETED == op))
    {
        if (ncs_sxpath_in_dxpath("/hwvtep-ttp:capable-switch/logical-switches/switch/controllers/controller/id", tmp->xpath))
        {
            if (sal_strlen(p_controller->id))
            {
                //indicate that another controller content begin
                sal_strcpy(next_controller_name, tmp->data.string_val);
            }
            else
            {
                sal_strcpy(p_controller->id, tmp->data.string_val);
            }
        }
        else if (ncs_sxpath_in_dxpath("/hwvtep-ttp:capable-switch/logical-switches/switch/controllers/controller/role", tmp->xpath))
        {
            if (sal_strcmp(tmp->data.enum_val, "master") == 0)
            {
                p_controller->role = 0;
            }
            else if (sal_strcmp(tmp->data.enum_val, "slave") == 0)
            {
                p_controller->role = 1;
            }
            else if (sal_strcmp(tmp->data.enum_val, "equal") == 0)
            {
                p_controller->role = 2;
            }
        }
        else if (ncs_sxpath_in_dxpath("/hwvtep-ttp:capable-switch/logical-switches/switch/controllers/controller/ip-address", tmp->xpath))
        {
            sal_strcpy(p_controller->ip_address, tmp->data.string_val);
        }
        else if (ncs_sxpath_in_dxpath("/hwvtep-ttp:capable-switch/logical-switches/switch/controllers/controller/port", tmp->xpath))
        {
            p_controller->l4_port = tmp->data.uint16_val;
        }
        else if (ncs_sxpath_in_dxpath("/hwvtep-ttp:capable-switch/logical-switches/switch/controllers/controller/local-ip-address", tmp->xpath))
        {
            sal_strcpy(p_controller->local_ip_address, tmp->data.string_val);
        }
        else if (ncs_sxpath_in_dxpath("/hwvtep-ttp:capable-switch/logical-switches/switch/controllers/controller/local-port", tmp->xpath))
        {
            p_controller->local_l4_port = tmp->data.uint16_val;
        }
        else if (ncs_sxpath_in_dxpath("/hwvtep-ttp:capable-switch/logical-switches/switch/controllers/controller/protocol", tmp->xpath))
        {
            if (sal_strcmp(tmp->data.enum_val, "tcp") == 0)
            {
                p_controller->protocol = 0;
            }
            else if (sal_strcmp(tmp->data.enum_val, "tls") == 0)
            {
                p_controller->protocol = 1;
            }
        }
        else if (ncs_sxpath_in_dxpath("/hwvtep-ttp:capable-switch/logical-switches/switch/controllers/controller/mng-flag", tmp->xpath))
        {
            if (tmp->data.bool_val)
            {
                //manage port
                p_controller->mng_flag = 1;
            }
            else
            {
                //data port
                p_controller->mng_flag = 0;
            }
        }

    }
    return rc;
}

int
ncs_hwvtep_ttp_controller_list_change_cb(sr_session_ctx_t *srs, const char *xpath, 
    sr_notif_event_t event, void *private_ctx) 
{
    int rc;
    sr_change_iter_t *it = NULL;
    sr_val_t *new_val = NULL;
    sr_val_t *old_val = NULL;
    sr_change_oper_t op;
    ncs_controller_t p_controller;
    char next_controller_name[64];
    int is_end = 0;

    sal_memset(&p_controller, 0, sizeof(ncs_controller_t));
    sal_memset(next_controller_name, 0, sizeof(next_controller_name));

    rc = ctc_nc_get_changes_iter(srs, "/hwvtep-ttp:capable-switch/logical-switches/switch/controllers/controller", &it);
    if (SR_ERR_OK != rc) {
        //puts("sr get changes iter failed");
        goto err_out;
    }

    while (!is_end)
    {
        rc = ctc_nc_get_change_next(srs,it, &op, &old_val, &new_val);
        if (SR_ERR_OK == rc)
        {
            rc = ncs_hwvtep_ttp_controller_change_process(srs, event, op, old_val, new_val, &p_controller, next_controller_name);
        }
        else
        {
            is_end = 1;
            rc = SR_ERR_OK;
        }
        
        if (SR_ERR_OK != rc) {
            goto err_out;
        }
        if (NULL != old_val) {
            sr_free_val(old_val);
            old_val = NULL;
        }
        if (NULL != new_val) {
            sr_free_val(new_val);
            new_val = NULL;
        }
        if (sal_strlen(next_controller_name) || is_end)
        {
            if (SR_EV_VERIFY == event && SR_OP_CREATED == op)
            {
                /* if controller id not null, create controller */
                if (0 != sal_strlen(p_controller.id))
                {
                    rc = ncs_api_openflow_add_controller(CDB_CFG_NC_CHECK, &p_controller);
                    if (rc)
                    {
                        rc = SR_ERR_INVAL_ARG;
                        goto err_out;
                    }
                }
                else
                {
                    /* not support other create action in controller container with no controller id  */
                    rc = SR_ERR_INVAL_ARG;
                    goto err_out;
                }
            } 
            else if (SR_EV_VERIFY == event && SR_OP_DELETED == op) 
            {
                if (0 != sal_strlen(p_controller.id))
                {
                    rc = ncs_api_openflow_delete_controller(CDB_CFG_NC_CHECK, &p_controller);
                    if (rc)
                    {
                        rc = SR_ERR_INVAL_ARG;
                        goto err_out;
                    }
                }
                else
                {
                    /* not support other delete action in controller container with no controller id  */
                    rc = SR_ERR_INVAL_ARG;
                    goto err_out;
                }
            }
            else if (SR_EV_APPLY == event && SR_OP_CREATED == op) 
            {
                if (0 != sal_strlen(p_controller.id))
                {
                    rc = ncs_api_openflow_add_controller(CDB_CFG_NC_SET, &p_controller);
                    if (rc)
                    {
                        rc = SR_ERR_OPERATION_FAILED;
                        goto err_out;
                    }
                }
            }
            else if (SR_EV_APPLY == event && SR_OP_DELETED == op)
            {
                if (0 != sal_strlen(p_controller.id))
                {
                    rc = ncs_api_openflow_delete_controller(CDB_CFG_NC_SET, &p_controller);
                    if (rc)
                    {
                        rc = SR_ERR_OPERATION_FAILED;
                        goto err_out;
                    }
                }
                
            }
            else if (SR_EV_ENABLED == event) 
            {
                /* Startup stage: subscription establish event
                 *   XXX: copy from startup config to running config
                 */
            }
            sal_memset(&p_controller, 0, sizeof(ncs_controller_t));
            sal_strcpy(p_controller.id, next_controller_name);
            sal_memset(next_controller_name, 0, sizeof(next_controller_name));
        }
    } /* while */

    rc = SR_ERR_OK;

err_out:
    if (NULL != old_val) {
        sr_free_val(old_val);
        old_val = NULL;
    }
    if (NULL != new_val) {
        sr_free_val(new_val);
        new_val = NULL;
    }
    if (NULL != it) {
        sr_free_change_iter(it);
    }
    return rc;
    
}

#define ____CONFIG_INTERFACE_RESOURCE_FUN____

/*
*if resource_id==NULL, return physical interface that not used
*if resource_id!=NULL, retrun interface by resource_id
*/
static ncs_interface_t*
ncs_hwvtep_ttp_get_interface_by_resource_id(char *resource_id)
{
    ctclib_list_node_t *p_node = NULL, *p_next = NULL;
    ctclib_list_t *p_list = NULL;
    ncs_interface_t *p_db_interface = NULL;

    if (!switch_master)
    {
        return NULL;
    }
    p_list = &switch_master->if_list;
    
    ctclib_list_for_each_safe(p_node, p_next, p_list)
    {
        p_db_interface = ctclib_container_of(p_node, ncs_interface_t, node);
        if (resource_id)
        {
            if (0 == sal_strcmp(p_db_interface->resource_id, resource_id))
            {
                return p_db_interface;
            }
        }
        else
        {
            if (!p_db_interface->is_tunnel && !p_db_interface->has_used)
            {
                return p_db_interface;
            }
        }
     }

    return NULL;
}

#if 0
static int
ncs_hwvtep_ttp_interface_check_resource(sr_change_oper_t op, ncs_interface_t *p_interface)
{
    ncs_interface_t *p_db_interface = NULL;
    char tunnel_name[IFNAME_SIZE];
    int i = 0;

    if (!switch_master)
    {
        return SR_ERR_INTERNAL;
    }
    sal_memset(tunnel_name, 0, sizeof(tunnel_name));

    if (SR_OP_CREATED == op)
    {
        if (p_interface->is_tunnel)
        {
            //tunnel interface
            for (i = 0; i < NCS_TUNNEL_NUM; i++)
            {
                if (switch_master->tunnel_bitmap[i] == 0)
                {
                    sal_sprintf(tunnel_name, "vxlan%d", i+1);
                    sal_strcpy(p_interface->ifname, tunnel_name);
                    return SR_ERR_OK;
                }
            }
            return SR_ERR_INVAL_ARG;
        }
        else
        {
            //physical interface 
            p_db_interface = ncs_hwvtep_ttp_get_interface_by_resource_id(NULL);
            if (!p_db_interface)
            {
                return SR_ERR_INVAL_ARG;
            }
            sal_strcpy(p_interface->ifname, p_db_interface->ifname);
        }
    }
    else if (SR_OP_DELETED == op)
    {
        p_db_interface = ncs_hwvtep_ttp_get_interface_by_resource_id(p_interface->resource_id);
        if (!p_db_interface)
        {
            return SR_ERR_INVAL_ARG;
        }
        sal_strcpy(p_interface->ifname, p_db_interface->ifname);
    }

    return SR_ERR_OK;
}
#endif

static int
ncs_hwvtep_ttp_interface_alloc_resource(ncs_interface_t *p_interface)
{
    ncs_interface_t *p_db_interface = NULL;
    ncs_interface_t *p_tunnel_interface = NULL;
    char tunnel_name[IFNAME_SIZE];
    int i = 0;

    if (!switch_master)
    {
        return SR_ERR_INTERNAL;
    }
    sal_memset(tunnel_name, 0, sizeof(tunnel_name));
        
    if (p_interface->is_tunnel)
    {
        //tunnel interface
        for (i = 0; i < NCS_TUNNEL_NUM; i++)
        {
            if (switch_master->tunnel_bitmap[i] == 0)
            {
                sal_sprintf(tunnel_name, "vxlan%d", i+1);
                sal_strcpy(p_interface->ifname, tunnel_name);
                /* alloc tunnel interface */
                p_tunnel_interface = sal_malloc(sizeof(ncs_interface_t));
                sal_memset(p_tunnel_interface, 0, sizeof(ncs_interface_t));
                sal_strcpy(p_tunnel_interface->resource_id, p_interface->resource_id);
                sal_strcpy(p_tunnel_interface->ifname, p_interface->ifname);
                sal_strcpy(p_tunnel_interface->source_ip, p_interface->source_ip);
                sal_strcpy(p_tunnel_interface->remote_ip, p_interface->remote_ip);
                p_tunnel_interface->is_tunnel = 1;
                p_tunnel_interface->tunnel_index = i;
                ctclib_list_insert_tail(&switch_master->if_list, &p_tunnel_interface->node);

                switch_master->tunnel_bitmap[i] = 1;
                
                return SR_ERR_OK;
            }
        }
        return SR_ERR_INVAL_ARG;
    }
    else
    {
        //physical interface 
        p_db_interface = ncs_hwvtep_ttp_get_interface_by_resource_id(NULL);
        if (!p_db_interface)
        {
            return SR_ERR_INVAL_ARG;
        }
        sal_strcpy(p_interface->ifname, p_db_interface->ifname);
        p_db_interface->has_used = 1;
        sal_strcpy(p_db_interface->resource_id, p_interface->resource_id);
    }

    return SR_ERR_OK;
}


static int
ncs_hwvtep_ttp_interface_release_resource(ncs_interface_t *p_interface)
{
    ctclib_list_t *p_list = NULL;
    ncs_interface_t *p_db_interface = NULL;
    
    if (!switch_master)
    {
        return SR_ERR_INTERNAL;
    }

    p_list = &switch_master->if_list;

    p_db_interface = ncs_hwvtep_ttp_get_interface_by_resource_id(p_interface->resource_id);
    if (!p_db_interface)
    {
        return SR_ERR_INVAL_ARG;
    }

    sal_strcpy(p_interface->ifname, p_db_interface->ifname);
    if (p_db_interface->is_tunnel)
    {
        //tunnel interface
        switch_master->tunnel_bitmap[p_db_interface->tunnel_index] = 0;
        ctclib_list_delete(p_list, &p_db_interface->node);
        sal_free(p_db_interface);
    }
    else
    {
        //physical interface
        p_db_interface->has_used = 0;
        sal_memset(p_db_interface->resource_id, 0, sizeof(p_db_interface->resource_id));
    }
    
    return SR_ERR_OK;
}

static int
ncs_hwvtep_ttp_interface_change_process(sr_session_ctx_t *srs,sr_notif_event_t ev, sr_change_oper_t op,
    sr_val_t *old_val, sr_val_t *new_val, ncs_interface_t *p_interface, char *next_interface_name)
{
    int rc = SR_ERR_OK;
    sr_val_t *tmp;

    switch(op) {
    case SR_OP_CREATED:
        if (NULL != new_val) {
           tmp = new_val;
        }
        break;
    case SR_OP_DELETED:
        if (NULL != old_val) {
            tmp = old_val;
        }
        break;
    case SR_OP_MODIFIED:
        //not support now
        return SR_ERR_UNSUPPORTED;
        if (NULL != old_val && NULL != new_val) {
        }
        break;
    case SR_OP_MOVED:
        //not support now
        return SR_ERR_UNSUPPORTED;
        if (NULL != new_val) {
        }
        break;
    default:
        rc = SR_ERR_INVAL_ARG;
        break;
    }

    if (NULL != tmp && (SR_OP_CREATED == op || SR_OP_DELETED == op))
    {
        if (ncs_sxpath_in_dxpath("/hwvtep-ttp:capable-switch/resources/port/resource-id", tmp->xpath))
        {
            if (sal_strlen(p_interface->resource_id))
            {
                sal_strcpy(next_interface_name, tmp->data.string_val);
            }
            else
            {
                sal_strcpy(p_interface->resource_id, tmp->data.string_val);
            }
        }
        else if (ncs_sxpath_in_dxpath("/hwvtep-ttp:capable-switch/resources/port/vxlan-tunnel/local-endpoint-ipv4-adress", tmp->xpath))
        {
            sal_strcpy(p_interface->source_ip, tmp->data.string_val);
            p_interface->is_tunnel = 1;
        }
        else if (ncs_sxpath_in_dxpath("/hwvtep-ttp:capable-switch/resources/port/vxlan-tunnel/remote-endpoint-ipv4-adress", tmp->xpath))
        {
            sal_strcpy(p_interface->remote_ip, tmp->data.string_val);
        }
    }
        
    return rc;
}

int
ncs_hwvtep_ttp_interface_list_change_cb(sr_session_ctx_t *srs, const char *xpath, 
    sr_notif_event_t event, void *private_ctx)
{
    int rc;
    sr_change_iter_t *it = NULL;
    sr_val_t *new_val = NULL;
    sr_val_t *old_val = NULL;
    sr_change_oper_t op;
    ncs_interface_t p_interface;
    char next_interface_name[64];
    int is_end = 0;

    sal_memset(&p_interface, 0, sizeof(ncs_interface_t));
    sal_memset(next_interface_name, 0, sizeof(next_interface_name));

    rc = ctc_nc_get_changes_iter(srs, "/hwvtep-ttp:capable-switch/resources/port", &it);
    if (SR_ERR_OK != rc) {
        //puts("sr get changes iter failed");
        goto err_out;
    }

    while (!is_end)
    {
        rc = ctc_nc_get_change_next(srs,it, &op, &old_val, &new_val);
        if (SR_ERR_OK == rc)
        {
            rc = ncs_hwvtep_ttp_interface_change_process(srs, event, op, old_val, new_val, &p_interface, next_interface_name);
        }
        else
        {
            is_end = 1;
            rc = SR_ERR_OK;
        }
        
        if (SR_ERR_OK != rc) {
            goto err_out;
        }
        if (NULL != old_val) {
            sr_free_val(old_val);
            old_val = NULL;
        }
        if (NULL != new_val) {
            sr_free_val(new_val);
            new_val = NULL;
        }

#if 0
        if (sal_strlen(next_interface_name) || is_end)
        {
            if (SR_EV_VERIFY == event && SR_OP_CREATED == op)
            {
                /* if interface resources id not null, create interface */
                if (0 != sal_strlen(p_interface.resource_id))
                {
                    rc = 
                        (op, &p_interface);
                    if (rc)
                    {
                        goto err_out;
                    }
                    
                    rc = ncs_api_openflow_add_interface(CDB_CFG_NC_CHECK, &p_interface);
                    if (rc)
                    {
                        rc = SR_ERR_INVAL_ARG;
                        goto err_out;
                    }
                }
                else
                {
                    /* not support other create action in port container with no resource id  */
                    rc = SR_ERR_INVAL_ARG;
                    goto err_out;
                }
            } 
            else if (SR_EV_VERIFY == event && SR_OP_DELETED == op)
            {
               if (0 != sal_strlen(p_interface.resource_id))
                {
                    rc = ncs_hwvtep_ttp_interface_check_resource(op, &p_interface);
                    if (rc)
                    {
                        goto err_out;
                    }
                    
                    rc = ncs_api_openflow_delete_interface(CDB_CFG_NC_CHECK, &p_interface);
                    if (rc)
                    {
                        rc = SR_ERR_INVAL_ARG;
                        goto err_out;
                    }
                }
                else
                {
                    /* not support other create action in port container with no resource id  */
                    rc = SR_ERR_INVAL_ARG;
                    goto err_out;
                }
            }
            else if (SR_EV_APPLY == event && SR_OP_CREATED == op) 
            {
               if (0 != sal_strlen(p_interface.resource_id))
                {
                    rc = ncs_hwvtep_ttp_interface_alloc_resource(&p_interface);
                    if (rc)
                    {
                        goto err_out;
                    }
                    
                    rc = ncs_api_openflow_add_interface(CDB_CFG_NC_SET, &p_interface);
                    if (rc)
                    {
                        rc = SR_ERR_INVAL_ARG;
                        goto err_out;
                    }
                }
            }
            else if (SR_EV_APPLY == event && SR_OP_DELETED == op) 
            {
                if (0 != sal_strlen(p_interface.resource_id))
                {
                    rc = ncs_hwvtep_ttp_interface_release_resource(&p_interface);
                    if (rc)
                    {
                        goto err_out;
                    }
                    
                    rc = ncs_api_openflow_delete_interface(CDB_CFG_NC_SET, &p_interface);
                    if (rc)
                    {
                        rc = SR_ERR_INVAL_ARG;
                        goto err_out;
                    }
                }       
            }
            else if (SR_EV_ENABLED == event) 
            {
                /* Startup stage: subscription establish event
                 *   XXX: copy from startup config to running config
                 */
            }
            sal_memset(&p_interface, 0, sizeof(ncs_interface_t));
            sal_strcpy(p_interface.resource_id, next_interface_name);
            sal_memset(next_interface_name, 0, sizeof(next_interface_name));
        }
#endif
        
        if (sal_strlen(next_interface_name) || is_end)
        {
            if (SR_EV_VERIFY == event && SR_OP_CREATED == op)
            {
                /* if interface resources id not null, create interface */
                if (0 != sal_strlen(p_interface.resource_id))
                {
                    rc = ncs_hwvtep_ttp_interface_alloc_resource(&p_interface);
                    if (rc)
                    {
                        goto err_out;
                    }
                    
                    rc = ncs_api_openflow_add_interface(CDB_CFG_NC_SET, &p_interface);
                    if (rc)
                    {
                        rc = SR_ERR_INVAL_ARG;
                        goto err_out;
                    }
                }
                else
                {
                    /* not support other create action in port container with no resource id  */
                    rc = SR_ERR_INVAL_ARG;
                    goto err_out;
                }
            } 
            else if (SR_EV_VERIFY == event && SR_OP_DELETED == op)
            {
               if (0 != sal_strlen(p_interface.resource_id))
                {
                    rc = ncs_hwvtep_ttp_interface_release_resource(&p_interface);
                    if (rc)
                    {
                        goto err_out;
                    }
                    
                    rc = ncs_api_openflow_delete_interface(CDB_CFG_NC_SET, &p_interface);
                    if (rc)
                    {
                        rc = SR_ERR_INVAL_ARG;
                        goto err_out;
                    }
                }
               else
               {
                    /* not support other create action in port container with no resource id  */
                    rc = SR_ERR_INVAL_ARG;
                    goto err_out;
               }
            }
            else if (SR_EV_APPLY == event && SR_OP_CREATED == op) 
            {
                
            }
            else if (SR_EV_APPLY == event && SR_OP_DELETED == op) 
            {
                
            }
            else if (SR_EV_ABORT == event) 
            {
                if (0 != sal_strlen(p_interface.resource_id))
                {
                    rc = ncs_hwvtep_ttp_interface_release_resource(&p_interface);
                    if (rc)
                    {
                        goto err_out;
                    }
                    
                    rc = ncs_api_openflow_delete_interface(CDB_CFG_NC_SET, &p_interface);
                    if (rc)
                    {
                        rc = SR_ERR_INVAL_ARG;
                        goto err_out;
                    }
                }
               else
               {
                    /* not support other create action in port container with no resource id  */
                    rc = SR_ERR_INVAL_ARG;
                    goto err_out;
               }
            }
            else if (SR_EV_ENABLED == event) 
            {
                /* Startup stage: subscription establish event
                 *   XXX: copy from startup config to running config
                 */
            }
            sal_memset(&p_interface, 0, sizeof(ncs_interface_t));
            sal_strcpy(p_interface.resource_id, next_interface_name);
            sal_memset(next_interface_name, 0, sizeof(next_interface_name));
        }
    } /* while */

    rc = SR_ERR_OK;

err_out:
    if (NULL != old_val) {
        sr_free_val(old_val);
        old_val = NULL;
    }
    if (NULL != new_val) {
        sr_free_val(new_val);
        new_val = NULL;
    }
    if (NULL != it) {
        sr_free_change_iter(it);
    }
    return rc;
}

#define ____CONFIG_INTERFACE_REF_FUN____

static int
ncs_hwvtep_ttp_interface_ref_change_process(sr_session_ctx_t *srs,sr_notif_event_t ev, sr_change_oper_t op,
    sr_val_t *old_val, sr_val_t *new_val, char *resource_id)
{
    int rc = SR_ERR_OK;
    sr_val_t *tmp;

    switch(op) {
    case SR_OP_CREATED:
        if (NULL != new_val) {
           tmp = new_val;
        }
        break;
    case SR_OP_DELETED:
        if (NULL != old_val) {
            tmp = old_val;
        }
        break;
    case SR_OP_MODIFIED:
        //not support now
        return SR_ERR_UNSUPPORTED;
        if (NULL != old_val && NULL != new_val) {
        }
        break;
    case SR_OP_MOVED:
        //not support now
        return SR_ERR_UNSUPPORTED;
        if (NULL != new_val) {
        }
        break;
    default:
        rc = SR_ERR_INVAL_ARG;
        break;
    }

    if (NULL != tmp && (SR_OP_CREATED == op || SR_OP_DELETED == op))
    {
        if (ncs_sxpath_in_dxpath("/hwvtep-ttp:capable-switch/logical-switches/switch/resources/port", tmp->xpath))
        {
            sal_strcpy(resource_id, tmp->data.string_val);
        }
    }
        
    return rc;
}


/*
 *leaf list doesn't care next resource id 
*/
int
ncs_hwvtep_ttp_interface_ref_list_change_cb(sr_session_ctx_t *srs, const char *xpath, 
    sr_notif_event_t event, void *private_ctx)
{
    int rc;
    sr_change_iter_t *it = NULL;
    sr_val_t *new_val = NULL;
    sr_val_t *old_val = NULL;
    sr_change_oper_t op;
    ncs_interface_t *p_interface;
    char resource_id[64];

    sal_memset(resource_id, 0, sizeof(resource_id));

    rc = ctc_nc_get_changes_iter(srs, "/hwvtep-ttp:capable-switch/logical-switches/switch/resources/port", &it);
    if (SR_ERR_OK != rc) {
        //puts("sr get changes iter failed");
        goto err_out;
    }

    while (SR_ERR_OK == (rc = ctc_nc_get_change_next(srs,
            it, &op, &old_val, &new_val))) {
        rc = ncs_hwvtep_ttp_interface_ref_change_process(srs, event, op, old_val, new_val, resource_id);
        if (SR_ERR_OK != rc) {
            goto err_out;
        }
        if (NULL != old_val) {
            sr_free_val(old_val);
            old_val = NULL;
        }
        if (NULL != new_val) {
            sr_free_val(new_val);
            new_val = NULL;
        }
        /* get ncs interface by resource_id */
        p_interface = ncs_hwvtep_ttp_get_interface_by_resource_id(resource_id);
        if (!p_interface)
        {
            rc = SR_ERR_INVAL_ARG;
            goto err_out;
        }

        if (SR_EV_VERIFY == event && SR_OP_CREATED == op)
        {
            rc = ncs_api_openflow_interface_set_enable(CDB_CFG_NC_CHECK, p_interface);
            if (rc)
            {
                rc = SR_ERR_INVAL_ARG;
                goto err_out;
            }
        } 
        else if (SR_EV_VERIFY == event && SR_OP_DELETED == op)
        {
            rc = ncs_api_openflow_interface_set_disable(CDB_CFG_NC_CHECK, p_interface);
            if (rc)
            {
                rc = SR_ERR_INVAL_ARG;
                goto err_out;
            }
        }
        else if (SR_EV_APPLY == event && SR_OP_CREATED == op) 
        {
            rc = ncs_api_openflow_interface_set_enable(CDB_CFG_NC_SET, p_interface);
            if (rc)
            {
                rc = SR_ERR_INVAL_ARG;
                goto err_out;
            }
        }
        else if (SR_EV_APPLY == event && SR_OP_DELETED == op) 
        {
            rc = ncs_api_openflow_interface_set_disable(CDB_CFG_NC_SET, p_interface);
            if (rc)
            {
                rc = SR_ERR_INVAL_ARG;
                goto err_out;
            }  
        }
        else if (SR_EV_ENABLED == event) 
        {
            /* Startup stage: subscription establish event
             *   XXX: copy from startup config to running config
             */
        }
        p_interface = NULL;

    } /* while */

    rc = SR_ERR_OK;

err_out:
    if (NULL != old_val) {
        sr_free_val(old_val);
        old_val = NULL;
    }
    if (NULL != new_val) {
        sr_free_val(new_val);
        new_val = NULL;
    }
    if (NULL != it) {
        sr_free_change_iter(it);
    }
    return rc;
}

#define ____DATA_PROVIDER_CONTROLLER_FUN____

int32
ncs_hwvtep_ttp_get_controller_key_cb(void *obj, tbl_iter_args_t *pargs)
{
    tbl_controller_attr_t *p_controller = (tbl_controller_attr_t*)obj;
    char *controller_name = (char*)pargs->argv[0];
    
    if (0 == sal_strcmp(controller_name, p_controller->controller_name))
    {
        sal_memcpy(pargs->argv[1], &p_controller->key, sizeof(tbl_ns_route_key_t));
    }
    
    return 0;
}

static int
ncs_hwvtep_ttp_provide_controller_connection_state(char *controller_name, int *controller_status)
{
    tbl_ns_route_key_t  ns_route_key, tmp_ns_route_key;
    tbl_iter_args_t args;
    char cmd[CMD_BUF_256];
    char ip_adress[INET_ADDRSTRLEN];
    char connection_status[16];
    FILE *fp = NULL;
    uint8 is_tcp = 0;
    
    sal_memset(&ns_route_key, 0, sizeof(tbl_ns_route_key_t));
    sal_memset(&tmp_ns_route_key, 0, sizeof(tbl_ns_route_key_t));
    sal_memset(ip_adress, 0, sizeof(ip_adress));
    sal_memset(cmd, 0, sizeof(cmd));
    sal_memset(connection_status, 0, sizeof(connection_status));

    /* get controller attr */
    args.argv[0] = controller_name;
    args.argv[1] = &ns_route_key;
    /* get ns_route_key */
    tbl_controller_attr_iterate(ncs_hwvtep_ttp_get_controller_key_cb, &args);

    /* ns route not found */
    if (0 == sal_memcmp(&ns_route_key, &tmp_ns_route_key, sizeof(tbl_ns_route_key_t)))
    {
        return SR_ERR_INVAL_ARG;
    }

    if (NS_ROUTE_TCP == ns_route_key.proto)
    {
        is_tcp = 1;
    }

    sal_inet_ntop(AF_INET, &ns_route_key.ip, ip_adress, INET_ADDRSTRLEN);
    sal_snprintf(cmd, CMD_BUF_256, "ovs-vsctl  --columns=is_connected find controller target=\"%s\\:%s\\:%d\" | awk '{print $3}' > %s  2>&1", 
                    is_tcp?"tcp":"ssl", ip_adress, ns_route_key.port, TTP_NETCONF_TMP_FILE);
    sal_system(cmd);

    fp = sal_fopen(TTP_NETCONF_TMP_FILE, "r");
    sal_fgets(connection_status, 16, fp);
    sal_fclose(fp);
    sal_unlink(TTP_NETCONF_TMP_FILE);

    if (!strlen(connection_status))
    {
        return SR_ERR_INVAL_ARG;
    }
    
    /*trim '\n' in connection_status*/
    connection_status[sal_strlen(connection_status)-1] = '\0';
    if (0 == sal_strcmp(connection_status, "true"))
    {
        *controller_status = 1;
    }
    else
    {
        *controller_status = 0;
    }

    
    return SR_ERR_OK;
}

int 
ncs_hwvtep_ttp_controller_provider_cb(const char *xpath, sr_val_t **values, size_t *values_cnt, void *private_ctx)
{
    char *controller_name = NULL;
    char xp[256];
    sr_xpath_ctx_t xp_ctx = {0};
    int rc;
    int controller_status;

    sal_memset(xp, 0, sizeof(xp));
    sal_strcpy(xp, xpath);
    
    *values = sal_calloc(2*sizeof(sr_val_t));
    if (NULL == *values) {
        rc = SR_ERR_NOMEM;
        goto err_out;
    }
    
    controller_name = sr_xpath_key_value(xp, "controller", "id", &xp_ctx);
    if (!controller_name)
    {
        rc = SR_ERR_INVAL_ARG;
        goto err_out;
    }

    rc = ncs_hwvtep_ttp_provide_controller_connection_state(controller_name, &controller_status);
    if (rc)
    {
        goto err_out; 
    }

    /*should recover after used */
    sr_xpath_recover(&xp_ctx);

    if (controller_status)
    {
        (*values)[0].data.enum_val = strdup("up");
    }
    else
    {
        (*values)[0].data.enum_val = strdup("down");
    }

    sal_memset(xp, 0, sizeof(xp));
    sal_sprintf(xp, "%s/connection-state", xpath);
    (*values)[0].xpath = strdup(xp);
    (*values)[0].type = SR_ENUM_T;

    sal_memset(xp, 0, sizeof(xp));
    sal_sprintf(xp, "%s/current-version", xpath);
    (*values)[1].xpath = strdup(xp);
    (*values)[1].type = SR_ENUM_T;
    (*values)[1].data.enum_val = strdup("1.3");

    *values_cnt = 2;
    
    return SR_ERR_OK;

err_out:
    if (NULL != (*values)) {
        sal_free(*values);
        *values = NULL;
    }

    return rc;
}

#define ____DATA_PROVIDER_INTERFACE_FUN____

#if 0
static int
ncs_hwvtep_ttp_provide_interface_number(const char *xpath, sr_val_t **values, size_t *values_cnt, void *private_ctx)
{
    sr_xpath_ctx_t xp_ctx = {0};
    char *xp = strdup(xpath);
    char cmd[CMD_BUF_256];
    char fullname[IFNAME_SIZE];
    char ofport_str[16];
    char *resource_id = NULL;
    uint32 ofport;
    FILE *fp = NULL;
    ncs_interface_t *p_db_interface = NULL;
    int rc;
    
    *values = sal_calloc(sizeof(sr_val_t));
    if (NULL == *values) {
        return SR_ERR_NOMEM;
    }
    sal_memset(cmd, 0, sizeof(cmd));
    sal_memset(fullname, 0, sizeof(fullname));
    sal_memset(ofport_str, 0, sizeof(ofport_str));
    
    resource_id = sr_xpath_key_value(xp, "port", "resource-id", &xp_ctx);
    if (!resource_id)
    {
        rc = SR_ERR_INVAL_ARG;
        goto err_out;
    }
    
    p_db_interface = ncs_hwvtep_ttp_get_interface_by_resource_id(resource_id);
    if (!p_db_interface)
    {
        rc = SR_ERR_INVAL_ARG;
        goto err_out;
    }
    
    /*should recover after used */
    sr_xpath_recover(&xp_ctx);

    sal_snprintf(cmd, CMD_BUF_256, "ovs-vsctl  --columns=ofport find interface name=%s | awk '{print $3}' > %s 2>&1", IFNAME_ETH2FULL(p_db_interface->ifname, fullname), TTP_NETCONF_TMP_FILE);
    sal_system(cmd);

    fp = sal_fopen(TTP_NETCONF_TMP_FILE, "r");
    sal_fgets(ofport_str, 16, fp);
    sal_fclose(fp);
    sal_unlink(TTP_NETCONF_TMP_FILE);

    if (!sal_strlen(ofport_str))
    {
        rc = SR_ERR_INVAL_ARG;
        goto err_out;
    }
    /*trim '\n' in ofport_str*/
    ofport_str[sal_strlen(ofport_str)-1] = '\0';
    ofport = sal_atoi(ofport_str);
    
    (*values)->xpath = xp;
    (*values)->type = SR_UINT32_T;
    (*values)->data.uint32_val = ofport;
    *values_cnt = 1;
    
     return SR_ERR_OK;

err_out:
    if (NULL != (*values)) {
        sr_free_val(*values);
        *values = NULL;
    }
    if (NULL != xp) {
        sal_free(xp);
        xp = NULL;
    }
    return rc;
}
#endif

static int
ncs_hwvtep_ttp_provide_interface_number(const char *xpath, sr_val_t **values, size_t *values_cnt, void *private_ctx)
{
    sr_xpath_ctx_t xp_ctx = {0};
    char *xp = strdup(xpath);
    char *resource_id = NULL;
    ncs_interface_t *p_db_interface = NULL;
    tbl_interface_t *p_db_if = NULL;
    int rc;
    
    *values = sal_calloc(sizeof(sr_val_t));
    if (NULL == *values) {
        return SR_ERR_NOMEM;
    }
    
    resource_id = sr_xpath_key_value(xp, "port", "resource-id", &xp_ctx);
    if (!resource_id)
    {
        rc = SR_ERR_INVAL_ARG;
        goto err_out;
    }
    
    p_db_interface = ncs_hwvtep_ttp_get_interface_by_resource_id(resource_id);
    if (!p_db_interface)
    {
        rc = SR_ERR_INVAL_ARG;
        goto err_out;
    }
    /*should recover after used */
    sr_xpath_recover(&xp_ctx);

    p_db_if = tbl_interface_get_interface_by_name(p_db_interface->ifname);
    if (!p_db_if)
    {
        rc = SR_ERR_INVAL_ARG;
        goto err_out;
    }
    
    (*values)->xpath = xp;
    (*values)->type = SR_UINT32_T;
    (*values)->data.uint32_val = p_db_if->ifindex;
    *values_cnt = 1;
    
     return SR_ERR_OK;

err_out:
    if (NULL != (*values)) {
        sr_free_val(*values);
        *values = NULL;
    }
    if (NULL != xp) {
        sal_free(xp);
        xp = NULL;
    }
    return rc;
}

static int
ncs_hwvtep_ttp_provide_interface_name(const char *xpath, sr_val_t **values, size_t *values_cnt, void *private_ctx)
{
    ncs_interface_t *p_db_interface = NULL;
    char *resource_id;
    sr_xpath_ctx_t xp_ctx = {0};
    char *xp = strdup(xpath);
    char fullname[IFNAME_SIZE];
    int rc;

    sal_memset(fullname, 0, sizeof(fullname));
    *values = sal_calloc(sizeof(sr_val_t));
    if (NULL == *values) {
        rc = SR_ERR_NOMEM;
        goto err_out;
    }

    resource_id = sr_xpath_key_value(xp, "port", "resource-id", &xp_ctx);
    if (!resource_id)
    {
        rc = SR_ERR_INVAL_ARG;
        goto err_out;
    }
    p_db_interface = ncs_hwvtep_ttp_get_interface_by_resource_id(resource_id);
    if (!p_db_interface)
    {
        rc = SR_ERR_INVAL_ARG;
        goto err_out;
    }
                
    (*values)->data.string_val = strdup(IFNAME_ETH2FULL(p_db_interface->ifname, fullname));
    
    sr_xpath_recover(&xp_ctx);
    
    (*values)->xpath = xp;
    (*values)->type = SR_STRING_T;
    *values_cnt = 1;

     return SR_ERR_OK;

err_out:
    if (NULL != (*values)) {
        sr_free_val(*values);
        *values = NULL;
    }
    if (NULL != xp) {
        sal_free(xp);
        xp = NULL;
    }
    return rc;
}

int 
ncs_hwvtep_ttp_interface_provider_cb(const char *xpath, sr_val_t **values, size_t *values_cnt, void *private_ctx)
{
    if (sr_xpath_node_name_eq(xpath, "number"))
    {
        return ncs_hwvtep_ttp_provide_interface_number(xpath, values, values_cnt, private_ctx);
    } else if (sr_xpath_node_name_eq(xpath, "name")) {
        return ncs_hwvtep_ttp_provide_interface_name(xpath, values, values_cnt, private_ctx);
    }

    return SR_ERR_OK;
}

#define ____INIT____

int
ncs_rpc_test_cb(const char *xpath, const sr_val_t *input, const size_t input_cnt,
        sr_val_t **output, size_t *output_cnt, void *private_ctx)
{
    sr_val_t *v = NULL;
    char str[256] = {0};

    sr_new_values(1, &v);

    sr_val_set_xpath(v, "/hwvtep-ttp:test-rpc/status");
    sal_sprintf(str, "xpath: %s, content: %s.", input->xpath, input->data.string_val);
    sr_val_set_str_data(v, SR_STRING_T, str);

    *output = v;
    *output_cnt = 1;

    return SR_ERR_OK;
}


void
ncs_hwvtep_ttp_switch_init()
{
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_interface_master_t *p_master =NULL;
    tbl_interface_t *p_db_if = NULL;
    ncs_interface_t *p_interface = NULL;
    char fullname[IFNAME_SIZE];
    int rc = SR_ERR_OK;

    if (!first_add)
    {
        return;
    }
    switch_master = sal_malloc(sizeof(ncs_switch_master_t));

    /* create hwttp capable switch 0 */
    rc = ncs_hwvtep_ttp_switch_create_capable_switch();
    if (SR_ERR_OK != rc) {
        log_sys(M_MOD_INTERNAL, E_ERROR, "hwttp create capable switch 0 failed");
    }
    
    ctclib_list_init(&switch_master->if_list);
    sal_memset(switch_master->tunnel_bitmap, 0, sizeof(switch_master->tunnel_bitmap));

    p_master = tbl_interface_get_master();
    if (NULL == p_master)
    {
        return ;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, listnode, next)
    {
        if(p_db_if && GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
        {
            p_interface = sal_malloc(sizeof(ncs_interface_t));
            sal_memset(p_interface, 0, sizeof(ncs_interface_t));
 
            rc = ncs_hwvtep_ttp_switch_create_capable_switch_port_resource(IFNAME_ETH2FULL(p_db_if->key.name, fullname));
            if (rc != SR_ERR_OK)
            {
                log_sys(M_MOD_INTERNAL, E_ERROR, "hwttp create port resource %s failed!", fullname);
                sal_free(p_interface);
                continue;
            }
            p_interface->is_tunnel = 0;
            p_interface->has_used = 1;
            sal_strcpy(p_interface->ifname, p_db_if->key.name);    
            sal_strcpy(p_interface->resource_id, fullname);    

            ctclib_list_insert_tail(&switch_master->if_list, &p_interface->node);
        }
    }
    first_add = 0;
    
}

void
ncs_hwvtep_ttp_switch_reset()
{
    ctclib_list_node_t *p_node = NULL, *p_next = NULL;
    ctclib_list_t *p_list = NULL;
    ncs_interface_t *p_interface = NULL;

    if (!switch_master)
    {
        return;
    }
    
    p_list = &switch_master->if_list;

    ctclib_list_for_each_safe(p_node, p_next, p_list)
    {
        p_interface = ctclib_container_of(p_node, ncs_interface_t, node);
        /* does not delete physical interface */
        if (p_interface->is_tunnel)
        {
            ctclib_list_delete(p_list, p_node);
            sal_free(p_interface);
        }
        else
        {
            p_interface->has_used = 0;
            sal_memset(p_interface->resource_id, 0, sizeof(p_interface->resource_id));
        }
    }

}

int32
ncs_interface_sync_after_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    return 0;
    tbl_interface_t* p_if = NULL;
    tbl_interface_t* p_db_if = NULL; 
    sr_val_t values[2];
    char fullname[IFNAME_SIZE];
    int32 rc = PM_E_NONE;

    if(p_ds_node)
    {
        return PM_E_NONE;
    }
    
    switch (oper)
    {
    case CDB_OPER_SET:
        p_if = (tbl_interface_t*)p_tbl;
        if(NULL == p_if)
        {
            return PM_E_INVALID_PARAM;
        }
        p_db_if = tbl_interface_get_interface(&p_if->key);
        if(NULL == p_db_if)
        {
            return PM_E_NOT_EXIST;
        }        
        
        switch(field_id)
        {
        case TBL_INTERFACE_FLD_RUNNING:
            sal_memset(values, 0, sizeof(values));
            sal_memset(fullname, 0, sizeof(fullname));
            values[0].xpath = "/hwvtep-ttp:link-up/if-name";
            values[0].type = SR_STRING_T;
            values[0].data.string_val = strdup(IFNAME_ETH2FULL(p_db_if->key.name, fullname));
            values[1].xpath = "/hwvtep-ttp:link-up/status";
            values[1].type = SR_ENUM_T;
            if (GLB_FLAG_ISSET(p_if->flags, GLB_IFF_RUNNING))
            {
                values[1].data.enum_val = "up";
            }
            else
            {
                values[1].data.enum_val = "down";
            }
            rc = ctc_nc_event_notif_send(NULL, "/hwvtep-ttp:link-up", values, 2, SR_EV_NOTIF_DEFAULT);
            if (rc)
            {
                
            }
            break;
        }
        break;
    default:
        return PM_E_NONE;
    }

    return PM_E_NONE;
}

void
ncs_register_cb()
{
    cdb_pm_subscribe_tbl(TBL_INTERFACE, NULL, ncs_interface_sync_after_cb);
}


#endif /* !HAVE_NETCONF */

