
#include <unistd.h>

#include "proto.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_auth_server_define.h"
#include "gen/tbl_auth_server.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_authd_debug_define.h"
#include "gen/tbl_authd_debug.h"
#include "gen/tbl_auth_session_define.h"
#include "gen/tbl_auth_session.h"
#include "fei.h"
#include "genlog.h"
#include "ctc_task.h"
#include "lib_fs.h"
#include "authd.h"
#include "gen/tbl_dot1x_global_define.h"
#include "gen/tbl_dot1x_global.h"
#include "gen/tbl_dot1x_port_define.h"
#include "gen/tbl_dot1x_port.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_dot1x_radius_define.h"
#include "gen/tbl_dot1x_radius.h"
#include "gen/tbl_dot1x_mac_define.h"
#include "gen/tbl_dot1x_mac.h"
#include "gen/tbl_login_define.h"
#include "gen/tbl_login.h"
#include "gen/tbl_vty_define.h"
#include "gen/tbl_vty.h"
#include "gen/tbl_author_define.h"
#include "gen/tbl_author.h"
#include "gen/tbl_account_define.h"
#include "gen/tbl_account.h"
#include "gen/tbl_accountcmd_define.h"
#include "gen/tbl_accountcmd.h"
#include "dot1x.h"
#include "dot1x_api.h"
#include "authd_debug.h"

DEBUG_DEFINE(authd_debug);

int32 authd_db_act_process(char **argv, int32 argc, cfg_cmd_para_t *para);
int32 auth_cfg_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);
int32 auth_server_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);
int32 auth_session_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);
int32 authdbg_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);
//extern int32 authd_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);
extern int authd_start();
extern void
authd_send_account_stop(tbl_login_t  *p_db_login, tbl_vty_t  *p_vty);
extern tbl_login_t *
auth_get_login_by_vty(tbl_vty_t  *p_vty);
extern void
authd_send_account_commands(tbl_login_t  *p_db_login, tbl_vty_t  *p_vty);
/* modified by liwh for bug 44993, 2017-08-17 */
extern int
syscmd_file_exist(char *filename);
/* liwh end */

#if 0
extern void authd_if_delete_cb(char *pszIntfName);
extern int32
authd_process_intf_msg_after(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds);

extern int32
authd_process_route_msg_after(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds);
#endif

int _is_startup_done()
{
    tbl_sys_global_t* pGlobal = tbl_sys_global_get_sys_global();
    if (pGlobal && pGlobal->startup_done) {
        return 1;
    }
    return 0;
}

int app_is_password_encryption()
{
    tbl_sys_global_t* pGlobal = tbl_sys_global_get_sys_global();
    if (pGlobal && pGlobal->service_password_encryption) {
        return 1;
    }
    return 0;
}

int authd_proc_init_done(void)
{
    authd_tbl_init();

    return 0;
}

int authd_proc_startup_done(void)
{
    dot1x_init();

    /* modified by liwh for bug 45931, 2017-12-20 */
    authd_start();
    /* liwh end */

    return 0;
}

extern int authd_password_encryption(int onoff);

static int32
authd_process_sys_global(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;
    tbl_sys_global_t *pGlobal = (tbl_sys_global_t *)p_tbl;

    CFG_DBG_FUNC();

    switch (oper)
    {
    case CDB_OPER_SET:
        if (TBL_SYS_GLOBAL_FLD_INIT_DONE == field_id) {
            authd_proc_init_done();
        } else if (TBL_SYS_GLOBAL_FLD_STARTUP_DONE == field_id) {
            authd_proc_startup_done();
        } else if (TBL_SYS_GLOBAL_FLD_HOSTNAME == field_id) {
            if ('\0' != pGlobal->hostname[0]) {
            }
        } else if (TBL_SYS_GLOBAL_FLD_SERVICE_PASSWORD_ENCRYPTION == field_id) {
            authd_password_encryption(pGlobal->service_password_encryption);
        }
        break;

    case CDB_OPER_GET:
        break;

    default:
        break;
    }
    
    return rc;
}

#if 0
int32
authd_process_intf_msg_after(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_interface_key_t *p_if_key = NULL;
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    p_if_key = (tbl_interface_key_t*)p_tbl;
    if (NULL != p_if_key) {
        p_db_if = tbl_interface_get_interface(p_if_key);
    }
    
    switch (oper) {
    case CDB_OPER_ADD:
    case CDB_OPER_SET:
        break;

    case CDB_OPER_DEL:
        if (p_ds_node) {
            /* delete sub DS */
            break;
        }
        if (p_if_key && '\0' != p_if_key->name[0]) {
            authd_if_delete_cb(p_if_key->name);
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}
#endif

int32
dot1x_interface_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_interface_t        *p_if = NULL;
    int32                  rc = PM_E_NONE;

    /* modified by liwh for bug 40784, 2016-09-14 */
    if (p_ds_node)
    {
        return PM_E_NONE;
    }
    /* liwh end */

    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:        
        p_if = (tbl_interface_t*)p_tbl;
        DOT1X_PTR_CHECK(p_if);

        if (p_if->hw_type == GLB_IF_TYPE_PORT_IF)
        {
            switch (field_id)
            {
            case TBL_INTERFACE_FLD_RUNNING:
                rc = dot1x_port_report_link_state(p_if);
                break;

            default:
                break;
            }
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

int32
authd_login_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_login_t        *p_login = NULL;
    int32              rc = PM_E_NONE;
    tbl_login_t        *p_db_login = NULL;
    tbl_vty_t          vty;
    tbl_vty_t          *p_vty = NULL;
    int                vty_id = 0;
    
    sal_memset(&vty, 0, sizeof(tbl_vty_t));
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        p_login = (tbl_login_t*)p_tbl;
        DOT1X_PTR_CHECK(p_login);

        p_db_login = tbl_login_get_login(&p_login->key);

        if (p_db_login && (sal_sscanf(p_db_login->vty, "vty%d", &vty_id) != 1))
        {
            return PM_E_INVALID_PARAM;
        }

        vty.key.id = vty_id;

        AUTHD_EVENT_DEBUG(
            "authd_login_sync is called, vty.key.id is %d user_name %s key_name %s", vty.key.id, p_db_login->user.name, p_db_login->key.name);

        p_vty = tbl_vty_get_vty(&vty.key);
        //if (p_vty && (p_db_login->user.name[0] != '\0'))
        if (p_vty)
        {
            /* modified by liwh for bug 41338/42848, 2017-03-23 
              when ssh logoff, shouldn,'t change privilege to configure value 
              when return vty, change privilege to configure value*/
            if ((0 == sal_memcmp(p_db_login->key.name, "ssh", 3))
               && (p_db_login->user.name[0] == '\0'))
            {
                return PM_E_NONE;    
            }
            /* liwh end */

            /* modified by liwh for bug 46677, 2018-04-02 
                should get acct stop privilege from vty privilege */
            if(syscmd_file_exist("/tmp/aaa_new_model"))
            {
                p_vty->acct_stop_privilege = p_vty->privilege;
            }
            /* liwh end */
            
            if (0 != p_vty->config_privilege)
            { 
                if ((p_vty->config_privilege != p_vty->privilege))
                {
                    p_vty->privilege = p_vty->config_privilege;
                    tbl_vty_set_vty_field(p_vty, TBL_VTY_FLD_PRIVILEGE);
                }
            }
            
            if (0 != p_vty->aaa_privilege)
            {
                p_vty->aaa_privilege = 0;
                tbl_vty_set_vty_field(p_vty, TBL_VTY_FLD_AAA_PRIVILEGE);
            }

            if (0 != p_vty->config_privilege)
            {
                p_vty->config_privilege = 0;  
                tbl_vty_set_vty_field(p_vty, TBL_VTY_FLD_CONFIG_PRIVILEGE);
            }
            
            /* modified by liwh for bug 44993, 2017-08-17 */
            if(syscmd_file_exist("/tmp/aaa_new_model"))
            /* liwh end */
            {           
                p_vty->acct_stop_pid = p_db_login->pid;
                sal_memcpy(p_vty->acct_stop_ipaddr, p_db_login->ipaddr, sal_strlen(p_db_login->ipaddr));
                authd_send_account_stop(p_db_login, p_vty);
            }
        }
        break;

    case CDB_OPER_SET:        
        
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

int32
authd_vty_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    int32              rc = PM_E_NONE;
    tbl_vty_t          vty;
    tbl_vty_t          *p_vty = NULL;
    tbl_vty_t          *p_db_vty = NULL;
    tbl_login_t        *p_db_login = NULL;
    
    sal_memset(&vty, 0, sizeof(tbl_vty_t));
        
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:               
        p_vty = (tbl_vty_t*)p_tbl;
        DOT1X_PTR_CHECK(p_vty);

        p_db_vty = tbl_vty_get_vty(&p_vty->key);
        if (p_db_vty && ('\0' != p_db_vty->accountcmd_method[0]) 
            && ('\0' != p_db_vty->cmd_buf[0])
            && sal_strcmp((char*)p_db_vty->cmd_buf, (char*)p_db_vty->old_cmd_buf))
        {
            p_db_login = auth_get_login_by_vty(p_db_vty);

            if (p_db_login)
            {
                authd_send_account_commands(p_db_login, p_vty);   
            }

            sal_memcpy(p_db_vty->old_cmd_buf, p_db_vty->cmd_buf, GLB_MAX_CMD_BUFSZ); 
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

int32
authd_cmd_show_tacacs_server(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                    rc = PM_E_NONE;    
    ctclib_slistnode_t       *listnode = NULL;
    FILE                     *fp = NULL;
    tbl_auth_server_master_t *p_master = tbl_auth_server_get_master();
    tbl_auth_server_t        *pItem = NULL;
    char                     buf[CMD_BUF_256];
    char                     buf_addr[CMD_BUF_256];

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "================================================================\n");
    sal_fprintf(fp, "%-16s%-6s%-8s%-8s%-5s%s\n", "Host", "Port", "Timeout", "Retries", "Dead", "Secret");
    sal_fprintf(fp, "================================================================\n");
    
    CTCLIB_SLIST_LOOP(p_master->auth_servers, pItem, listnode) 
    {
        if  (AUTH_SERVER_TYPE_TACPLUS == pItem->key.type)
        {
            cdb_addr_val2str(buf, CMD_BUF_256, &pItem->key.addr);
            sal_memcpy(buf_addr, buf, 15);
            buf_addr[15] = '\0';
                
            sal_fprintf(fp, "%-16s%-6d%-8d%-8d%-5d", 
                buf_addr, pItem->key.sin_port, pItem->timeout, pItem->retries, pItem->dead); 

            if (sal_strlen(pItem->enc_secret) > 0)
            {
                sal_fprintf(fp, "%s\n", pItem->enc_secret);    
            }
            else
            {
                sal_fprintf(fp, "%s\n", pItem->secret);        
            }
            
            if (sal_strlen(buf) > 15)
            {
                sal_fprintf(fp, " Host: %s\n", buf);
            }
        }
    }
    
    sal_fprintf(fp, "================================================================\n");
    sal_fclose(fp);
    fp = NULL;

    return rc;
}

int32
authd_module_init()
{
    //ipc_register_pkt_rx_fn(GLB_PKT_DHCP, authd_pdu_rx);
    
    cdb_register_cfg_tbl_cb(TBL_AUTH_SERVER, auth_server_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_AUTH_SESSION, auth_session_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_AUTHD_DEBUG, authdbg_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_AUTH_CFG, auth_cfg_cmd_process);
    
    //cdb_register_cfg_act_cb(ACT_AUTHD_DB, authd_db_act_process);    

    cdb_pm_subscribe_tbl(TBL_SYS_GLOBAL, NULL, authd_process_sys_global);

    /* interface */
    cdb_pm_subscribe_tbl(TBL_INTERFACE, NULL, dot1x_interface_sync);
    //cdb_pm_subscribe_tbl(TBL_INTERFACE, NULL, authd_process_intf_msg_after);

    cdb_pm_subscribe_tbl(TBL_LOGIN, authd_login_sync, NULL);

    cdb_pm_subscribe_tbl(TBL_VTY, NULL, authd_vty_sync);

    /* modified by liwh for bug 45931, 2017-12-20 */
    //authd_start();
    /* liwh end */

   cdb_register_cfg_act_cb(ACT_SHOW_TACACS_SERVER, authd_cmd_show_tacacs_server);

    dot1x_start();
    
    return PM_E_NONE;
}

int32
authd_init()
{     
    lib_master_t *lib_master = lib_init(PM_ID_AUTHD);
    lib_master->task_mst = ctc_task_init();
    lib_master->proto_mst = NULL;
    pm_id_t pmid = M_MOD_AUTHD;

    DEBUG_INIT(authd_debug, pmid);
#ifdef LDB_SYNC_TO_RDB
    rdb_connect();
#endif
    ipc_connect_bhm();
    
    /* client to CDS */
    ipc_connect_ccs_cds(0);

    /* client to HAL */
    fei_init(lib_get_master(), NULL, FALSE, 0);

    /* client to Packet */
    ipc_pkt_init(0);

    authd_module_init();

    log_sys(pmid, E_INFORMATIONAL, "Ready to service.");
    ctc_task_main();
    return PM_E_NONE;
}

static void usage()
{
    sal_printf("Usage: authd [-d] [-v] [-h]\n");
    sal_printf("    -d: daemon mode\n");
    sal_printf("    -v: show lib version\n");
    sal_printf("    -h: print this\n");
    return;
}

int
main(int argc, char **argv)
{
    char *p;
    char *progname;
    uint16 daemon_mode = 0;
    static const char *shortopts = "dhv";
 
    /* preserve my name */
    progname = ((p = sal_strrchr(argv[0], '/')) ? ++p : argv[0]);

    while (1) {   
         int32 opt;
         opt = getopt(argc, argv, shortopts);
         if (EOF == opt) {   
             break;
         }
     
         switch (opt) {
         case 'd':
             daemon_mode = TRUE;
             break;
         case 'v':
             lib_print_version(progname);
             exit(0);
             break;
         case 'h':
         default:
             usage();
             exit(1);
         }
    }

    if (daemon_mode) {
        daemon(1, 1);
    }
 
    authd_init();
    exit(0);
}


