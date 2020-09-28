/****************************************************************************
*  dot1x command process
*
* Modify History:
* Revision      : R0.01
* Author        : Liwh
* Date          : 2016-03-14 11:07
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <netinet/ip6.h>
#include <assert.h>

#include "proto.h"
#include "param_check.h"
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
#include "gen/tbl_auth_session_define.h"
#include "gen/tbl_auth_session.h"
#include "gen/tbl_auth_server_define.h"
#include "gen/tbl_auth_server.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_auth_session_define.h"
#include "gen/tbl_auth_session.h"
#include "dot1x.h"
#include "dot1x_api.h"
#include "dot1x_packet.h"
#include "rsa_derhex.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
char auth_port_ctrl_msg[][20] =
{
  {"Invalid"},
  {"Force Unauthorized"},
  {"Force Authorized"},
  {"Auto"}
};

char auth_port_status_msg[][20] =
{
  {"Unknown"},
  {"Authorized"},
  {"Unauthorized"}
};

char auth_pae_state_msg[][20] =
{
  {"Down"},
  {"Init"},
  {"Disconnected"},
  {"Connecting"},
  {"Authenticating"},
  {"Authenticated"},
  {"Aborting"},
  {"Held"},
  {"Force Authorized"},
  {"Force Unauthorized"},
  {"Restart"},
};

char auth_be_state_msg[][20] =
{
  {"Invalid"},
  {"Request"},
  {"Response"},
  {"Success"},
  {"Fail"},
  {"Timeout"},
  {"Idle"},
  {"Initialize"},
  {"Ignore"},
};


char auth_sesssion_terminate_cause[][40] =
{
  {"Port admin disabled or force authorized"},
  {"Supplicant logoff"},
  {"Port failure"},
  {"Supplicant restart"},
  {"Reauth failure"},
  {"Set to force unauthorized"},
  {"Port reinitialize"},
  {"Port admin disabled or force authorized"},
  {"Not terminated yet"},
};

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/
extern int32 
auth_server_dot1x_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int app_is_password_encryption();

/****************************************************************************
 *
* Function
* 
****************************************************************************/
void dot1x_set_port_mode(tbl_dot1x_global_t  *p_db_glb, tbl_dot1x_port_t   *p_db_dot1x_port)
{
    if (!p_db_glb)
    {
        return;
    }
    
    if (p_db_glb->global_auth_enable)
    {
        dot1x_set_fea_mode(p_db_dot1x_port);
        
        if (p_db_dot1x_port->portControl != p_db_dot1x_port->authControlledPortControl)
        {  
            p_db_dot1x_port->portControl = p_db_dot1x_port->authControlledPortControl;

            /* Run the PAE state machine.  */
            dot1x_run_sm(p_db_dot1x_port);
        }
    }
    else
    {
        dot1x_unset_fea_mode(p_db_dot1x_port);
        
        if (p_db_dot1x_port->portControl != AUTH_PORT_CTRL_FORCE_AUTHORIZED)
        {
            p_db_dot1x_port->portControl = AUTH_PORT_CTRL_FORCE_AUTHORIZED;

            /* Run the PAE state machine.  */
            dot1x_run_sm(p_db_dot1x_port);
        }
    }

    return;
}

static int32 _dot1x_cmd_set_global_auth_enable(tbl_dot1x_global_t *p_glb, 
    tbl_dot1x_global_t *p_db_glb, int32 field_id, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    tbl_dot1x_port_master_t   *p_glb_port = NULL;
    tbl_dot1x_port_t          *p_db_dot1x_port = NULL;
    ctclib_slistnode_t        *listnode = NULL; 
    tbl_interface_t           *p_db_if = NULL;
    
    /* check exist */
    p_glb_port = tbl_dot1x_port_get_master();

    if (!p_glb_port)
    {
        CFG_FUNC_ERR_RET("tbl_dot1x_global_set_dot1x_global_field fail because tbl_dot1x_port is not exist");
    }

    CTCLIB_SLIST_LOOP(p_glb_port->dot1x_port_list, p_db_dot1x_port, listnode)
    {
        rc = dot1x_port_check_dot1x_interface(p_db_dot1x_port, para); 
        if (rc)
        {
            continue;
        }

        p_db_if = tbl_interface_get_interface_by_ifindex(p_db_dot1x_port->key.ifindex);
        if (!p_db_if)
        {
            continue;
        }

        if (tbl_interface_is_up(p_db_if) || (!p_glb->global_auth_enable))
        {
            dot1x_set_port_mode(p_glb, p_db_dot1x_port);
        }
    }

    rc = tbl_dot1x_global_set_dot1x_global_field(p_glb, field_id);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("tbl_dot1x_global_set_dot1x_global_field set dot1x enable fail");
        return rc;
    }
        
    return rc;
}

static int32 _dot1x_cmd_set_radius_default_retry(tbl_dot1x_global_t *p_glb, 
    tbl_dot1x_global_t *p_db_glb, int32 field_id, cfg_cmd_para_t *para)
{
    int32                       rc = PM_E_NONE;
    tbl_dot1x_radius_master_t   *p_glb_radius = NULL;
    tbl_dot1x_radius_t          *p_db_dot1x_radius = NULL;
    ctclib_slistnode_t          *listnode = NULL;  

    rc = tbl_dot1x_global_set_dot1x_global_field(p_glb, field_id);
    if (rc)
    {
        return rc;
    }
        
    /* check exist */
    p_glb_radius = tbl_dot1x_radius_get_master();

    if (!p_glb_radius)
    {
        CFG_FUNC_ERR_RET("_dot1x_cmd_set_radius_default_retry fail because tbl_dot1x_radius is not exist");
    }

    dot1x_set_auth_cfg_retry(p_glb);

    CTCLIB_SLIST_LOOP(p_glb_radius->radius_list, p_db_dot1x_radius, listnode)
    {
        if (GLB_FLAG_ISSET(p_db_dot1x_radius->radius_config, AUTH_RADIUS_CONFIG_DEFAULT_RETRANSMIT))
        {
            p_db_dot1x_radius->max_retry = p_glb->radius_default_retry;
            rc = tbl_dot1x_radius_set_dot1x_radius_field(p_db_dot1x_radius, TBL_DOT1X_RADIUS_FLD_MAX_RETRY);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius max retry valid failed");
            }
            
            dot1x_port_active_radius_server(p_db_dot1x_radius);

            dot1x_set_auth_server_retry(p_db_dot1x_radius);
        }
    }
        
    return rc;
}

static int32 _dot1x_cmd_set_radius_default_timeout(tbl_dot1x_global_t *p_glb, 
    tbl_dot1x_global_t *p_db_glb, int32 field_id, cfg_cmd_para_t *para)
{
    int32                       rc = PM_E_NONE;
    tbl_dot1x_radius_master_t   *p_glb_radius = NULL;
    tbl_dot1x_radius_t          *p_db_dot1x_radius = NULL;
    ctclib_slistnode_t          *listnode = NULL;  

    rc = tbl_dot1x_global_set_dot1x_global_field(p_glb, field_id);
    if (rc)
    {
        return rc;
    }
    
    /* check exist */
    p_glb_radius = tbl_dot1x_radius_get_master();

    if (!p_glb_radius)
    {
        CFG_FUNC_ERR_RET("_dot1x_cmd_set_radius_default_retry fail because tbl_dot1x_radius is not exist");
    }

    dot1x_set_auth_cfg_timeout(p_glb);

    CTCLIB_SLIST_LOOP(p_glb_radius->radius_list, p_db_dot1x_radius, listnode)
    {
        if (GLB_FLAG_ISSET(p_db_dot1x_radius->radius_config, AUTH_RADIUS_CONFIG_DEFAULT_TIMEOUT))
        {
            p_db_dot1x_radius->timeout = p_glb->radius_default_timeout;
            rc = tbl_dot1x_radius_set_dot1x_radius_field(p_db_dot1x_radius, TBL_DOT1X_RADIUS_FLD_TIMEOUT);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius timeout failed");
            }
            
            dot1x_port_active_radius_server(p_db_dot1x_radius);

            dot1x_set_auth_server_timeout(p_db_dot1x_radius);
        }
    }
        
    return rc;
}

static int32 _dot1x_cmd_set_radius_default_deadtime(tbl_dot1x_global_t *p_glb, 
    tbl_dot1x_global_t *p_db_glb, int32 field_id, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    
    if (0 == p_glb->radius_default_deadtime)
    {
        p_glb->radius_default_deadtime = RADIUS_DEADTIME_DEFAULT;

        dot1x_port_clear_dead_radius();
    }

    dot1x_set_auth_cfg_deadtime(p_glb);
    
    rc = tbl_dot1x_global_set_dot1x_global_field(p_glb, field_id);

    /* modified by liwh for bug 50172, 2019-01-15 */
    CFG_PROMPT_RET("Warning: Tacacs-server deadtime will also be changed by this cli");
    /* liwh end */
        
    return rc;
}

static int32 _dot1x_cmd_set_radius_default_shared_secret(tbl_dot1x_global_t *p_glb, 
    tbl_dot1x_global_t *p_db_glb, int32 field_id, cfg_cmd_para_t *para)
{
    int32                       rc = PM_E_NONE;
    tbl_dot1x_radius_master_t   *p_glb_radius = NULL;
    tbl_dot1x_radius_t          *p_db_dot1x_radius = NULL;
    ctclib_slistnode_t          *listnode = NULL;  

    rc = tbl_dot1x_global_set_dot1x_global_field(p_glb, TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET_VALID);
    if (rc)
    {
        return rc;
    }
    
    rc = tbl_dot1x_global_set_dot1x_global_field(p_glb, field_id);
    if (rc)
    {
        return rc;
    }

    dot1x_set_auth_cfg_shared_secret(p_glb);

    /* check exist */
    p_glb_radius = tbl_dot1x_radius_get_master();

    if (!p_glb_radius)
    {
        CFG_FUNC_ERR_RET("_dot1x_cmd_set_radius_default_retry fail because tbl_dot1x_radius is not exist");
    }

    CTCLIB_SLIST_LOOP(p_glb_radius->radius_list, p_db_dot1x_radius, listnode)
    {
        if (GLB_FLAG_ISSET(p_db_dot1x_radius->radius_config, AUTH_RADIUS_CONFIG_DEFAULT_KEY))
        {
#if 0
            p_db_dot1x_radius->shared_secret_valid = p_glb->radius_default_shared_secret_valid;
            rc = tbl_dot1x_radius_set_dot1x_radius_field(p_db_dot1x_radius, TBL_DOT1X_RADIUS_FLD_SHARED_SECRET_VALID);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius default shared secret valid failed");
            }
    
            if (p_glb->radius_default_shared_secret_valid)
            {
                sal_memcpy(p_db_dot1x_radius->shared_secret, p_glb->radius_default_shared_secret, 
                    sizeof(p_glb->radius_default_shared_secret));  
            }
            else
            {
                sal_memset(p_db_dot1x_radius->shared_secret, 0, RADIUS_SHARED_SECRET_LEN+1);   
            }
            
            rc = tbl_dot1x_radius_set_dot1x_radius_field(p_db_dot1x_radius, TBL_DOT1X_RADIUS_FLD_SHARED_SECRET);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius default shared secret failed");
            }
#endif
            if (p_glb->radius_default_shared_secret_valid)
            {
                sal_memcpy(p_db_dot1x_radius->shared_secret, p_glb->radius_default_shared_secret, 
                    RADIUS_SHARED_SECRET_LEN+1);
                sal_memcpy(p_db_dot1x_radius->shared_enc_secret, p_glb->radius_default_shared_enc_secret, 
                    PASSWD_SIZE);
            }
            else
            {
                sal_memset(p_db_dot1x_radius->shared_secret, 0, RADIUS_SHARED_SECRET_LEN+1);   
                sal_memset(p_db_dot1x_radius->shared_enc_secret, 0, PASSWD_SIZE);   
            }
            
            dot1x_set_auth_server_shared_secret(p_db_dot1x_radius);
            
            dot1x_port_active_radius_server(p_db_dot1x_radius);
        }
    }

    return rc;
}

int
dot1x_radius_check_shared_key (char *key, cfg_cmd_para_t *para)
{
    char              *p = key;
    int32             rc = PM_E_NONE;
    
    if (sal_strlen(p) > RADIUS_SHARE_KEY_MAX_LEN || sal_strlen(p) < RADIUS_SHARE_KEY_MIN_LEN)
    {
        CFG_FUNC_ERR_RET("Invalid shared key! its length range should be [1, 64]");
    }
    
    /* check the chars */
    while ('\0' != *p)
    {
        if ((*p == '%')||(*p == '`'))
        {
            CFG_FUNC_ERR_RET("The key can include 0-9A-Za-z~!@#$^&*()_+-={}[]|:;<>,.");
        }     
       
        p++;      
    }
    
    return rc;
}

int
dot1x_radius_check_enc_shared_key (char *key, cfg_cmd_para_t *para)
{
    char              *p = key;
    int32             rc = PM_E_NONE;
    char              *q = NULL;
    if (sal_strlen(p) > PASSWD_SIZE|| sal_strlen(p) < RADIUS_SHARE_KEY_MIN_LEN)
    {
        CFG_FUNC_ERR_RET("Invalid encrypt shared key! its length range should be [1, 256]");
    }
    char szTmp[PASSWD_SIZE];
    if (do_decrypt(M_DEFAULT_PW_STR, key, sal_strlen(key),
        szTmp, PASSWD_SIZE) != 0) {
        CFG_FUNC_ERR_RET("Invalid encrypt password");
    }
    q = szTmp;
    while ('\0' != *q)
    {
        if (!isprint(*q)) 
        {
            CFG_FUNC_ERR_RET("Invalid password string");
        }
        q++;
    }
    return rc;
}

int32
_dot1x_cmd_get_global(char **argv, int32 argc, cfg_cmd_para_t *para, cdb_node_t  *p_node)
{
    FILE                      *fp = NULL;
    tbl_iter_args_t           args;     
    tbl_dot1x_global_t        *p_db_glb = NULL;
    int32                     field_id = 0;
    field_parse_t             field;

     sal_memset(&args, 0 ,sizeof(args));

    /* check exist */
    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_CONFLICT_RET("Tbl_dot1x_global not found");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    args.argv[0] = &field;
    args.argv[1] = fp;
    
    field_id = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
    if (field_id < 0)
    {
        CFG_DEL_SHOW_PATH(para->p_show->path, fp);
        CFG_INVALID_PARAM_RET("Invalid field");
    }

    tbl_dot1x_global_dump_one(p_db_glb, &args);
            
    sal_fclose(fp);
    return PM_E_NONE;
}

int32
dot1x_cmd_process_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t                *p_node = NULL;
    tbl_dot1x_global_t        dot1x_glb;
    int32                     field_id = 0;
    cdb_field_t               *p_field = NULL;
    int32                     value = 0;
    int32                     i = 0;
    tbl_dot1x_global_t        *p_db_glb = NULL;
    int32                     rc = PM_E_NONE;

    CFG_DBG_FUNC();

    sal_memset(&dot1x_glb, 0, sizeof(dot1x_glb));

    p_node = cdb_get_tbl(TBL_DOT1X_GLOBAL);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        /* check exist */
        p_db_glb = tbl_dot1x_global_get_dot1x_global();
        if (!p_db_glb)
        {
            CFG_CONFLICT_RET("Tbl_dot1x_global not found");
        }

        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch (field_id)
        {
        case TBL_DOT1X_GLOBAL_FLD_GLOBAL_AUTH_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);

            dot1x_glb.global_auth_enable = value;
            rc = _dot1x_cmd_set_global_auth_enable(&dot1x_glb, p_db_glb, field_id, para);
            break;

        case TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_RETRY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            dot1x_glb.radius_default_retry = value;
            rc = _dot1x_cmd_set_radius_default_retry(&dot1x_glb, p_db_glb, field_id, para);
            break;

        case TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_TIMEOUT:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            dot1x_glb.radius_default_timeout = value;
            rc = _dot1x_cmd_set_radius_default_timeout(&dot1x_glb, p_db_glb, field_id, para);
            break;

        case TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_DEADTIME:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            dot1x_glb.radius_default_deadtime = value;
            rc = _dot1x_cmd_set_radius_default_deadtime(&dot1x_glb, p_db_glb, field_id, para);
            break;

        case TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET:
            if (argc == 2)
            {                
                rc = dot1x_radius_check_shared_key (argv[i+1], para);
                if (rc)
                {
                    return rc;    
                }

                dot1x_glb.radius_default_shared_secret_valid = TRUE;
                
                if (app_is_password_encryption())
                {
                    char szTmp[PASSWD_SIZE];
                    gen_passwd_crypt(argv[i+1], szTmp, PASSWD_SIZE);
                    snprintf(dot1x_glb.radius_default_shared_enc_secret, PASSWD_SIZE, "%s", szTmp);
                    sal_memset(dot1x_glb.radius_default_shared_secret, 0, RADIUS_SHARED_SECRET_LEN);

                    rc = _dot1x_cmd_set_radius_default_shared_secret(&dot1x_glb, p_db_glb, 
                        TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_ENC_SECRET, para);
                    break;
                }
                else
                {
                    sal_memcpy(dot1x_glb.radius_default_shared_secret, argv[i+1], sal_strlen(argv[i+1]));
                    sal_memset(dot1x_glb.radius_default_shared_enc_secret, 0, PASSWD_SIZE);
                }
            }
            else
            {
                dot1x_glb.radius_default_shared_secret_valid = FALSE;
            }
            rc = _dot1x_cmd_set_radius_default_shared_secret(&dot1x_glb, p_db_glb, field_id, para);
            break; 

        case TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_ENC_SECRET:
            if (argc == 2)
            {
                rc = dot1x_radius_check_enc_shared_key (argv[i+1], para);
                if (rc)
                {
                    return rc;    
                }

                dot1x_glb.radius_default_shared_secret_valid = TRUE;

                sal_memcpy(dot1x_glb.radius_default_shared_enc_secret, argv[i+1], sal_strlen(argv[i+1]));
                sal_memset(dot1x_glb.radius_default_shared_secret, 0, RADIUS_SHARED_SECRET_LEN);

                rc = tbl_dot1x_global_set_dot1x_global_field(&dot1x_glb, TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET);
                rc = _dot1x_cmd_set_radius_default_shared_secret(&dot1x_glb, p_db_glb, 
                    TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_ENC_SECRET, para);
                break;
            }
            break;

        case TBL_DOT1X_GLOBAL_FLD_DOT1X_ALL:
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            GLB_SET_FLAG_COND(value, dot1x_glb.dot1x, DOT1X_FLAG_ALL);
            rc = tbl_dot1x_global_set_dot1x_global_field(&dot1x_glb, field_id);
            break;

        case TBL_DOT1X_GLOBAL_FLD_DOT1X_PACKET:
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            GLB_SET_FLAG_COND(value, dot1x_glb.dot1x, DOT1X_FLAG_PACKET);
            rc = tbl_dot1x_global_set_dot1x_global_field(&dot1x_glb, field_id);
            break;

        case TBL_DOT1X_GLOBAL_FLD_DOT1X_TIMER:
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            GLB_SET_FLAG_COND(value, dot1x_glb.dot1x, DOT1X_FLAG_TIMER);
            rc = tbl_dot1x_global_set_dot1x_global_field(&dot1x_glb, field_id);
            break;

        case TBL_DOT1X_GLOBAL_FLD_DOT1X_PROTOCOL:
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            GLB_SET_FLAG_COND(value, dot1x_glb.dot1x, DOT1X_FLAG_PROTO);
            rc = tbl_dot1x_global_set_dot1x_global_field(&dot1x_glb, field_id);
            break;  
            
        case TBL_DOT1X_GLOBAL_FLD_DOT1X_EVENT:
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            GLB_SET_FLAG_COND(value, dot1x_glb.dot1x, DOT1X_FLAG_EVENTS);
            rc = tbl_dot1x_global_set_dot1x_global_field(&dot1x_glb, field_id);
            break;
        
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        rc = _dot1x_cmd_get_global(argv, argc, para, p_node);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

void
dot1x_radius_key_encryption(void)
{
    tbl_dot1x_global_t        *p_db_glb = NULL;
    char szTmp[PASSWD_SIZE];
    
    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (p_db_glb && p_db_glb->radius_default_shared_secret_valid
        && ('\0' != p_db_glb->radius_default_shared_secret[0]))
    {
        gen_passwd_crypt((char*)p_db_glb->radius_default_shared_secret, szTmp, PASSWD_SIZE);
        snprintf(p_db_glb->radius_default_shared_enc_secret, PASSWD_SIZE, "%s", szTmp);

        sal_memset(p_db_glb->radius_default_shared_secret, 0, RADIUS_SHARED_SECRET_LEN+1);

        tbl_dot1x_global_set_dot1x_global_field(p_db_glb, TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET);
        tbl_dot1x_global_set_dot1x_global_field(p_db_glb, TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_ENC_SECRET);
    }

    return;
}

int32
dot1x_port_set_defaults(tbl_dot1x_port_t  *p_db_dot1x_port, cfg_cmd_para_t *para)
{
    tbl_dot1x_global_t *p_db_glb = NULL;
    int32              rc = PM_E_NONE;

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_INVALID_PARAM_RET("Tbl_dot1x_global not found");
    }
        
    p_db_dot1x_port->authControlledPortControl = AUTH_PORT_CTRL_AUTO;

    if (p_db_glb->global_auth_enable)
    {
        p_db_dot1x_port->portControl = p_db_dot1x_port->authControlledPortControl;
    }
    else 
    {
        p_db_dot1x_port->portControl = AUTH_PORT_CTRL_FORCE_AUTHORIZED;
    }

    /* set be defaults  */
    dot1x_be_initialize(p_db_dot1x_port);
    p_db_dot1x_port->auth_be_suppTimeout = DOT1X_DEFAULT_BE_SUPP_TIMEOUT;
    rc = tbl_dot1x_port_set_dot1x_port_field(p_db_dot1x_port, TBL_DOT1X_PORT_FLD_AUTH_BE_SUPPTIMEOUT);
    if (rc)        
    {            
        CFG_INVALID_PARAM_RET("Set dot1x port failed");        
    }
    
    p_db_dot1x_port->auth_be_serverTimeout = DOT1X_DEFAULT_BE_SERVER_TIMEOUT;
    rc = tbl_dot1x_port_set_dot1x_port_field(p_db_dot1x_port, TBL_DOT1X_PORT_FLD_AUTH_BE_SERVERTIMEOUT);
    if (rc)        
    {            
        CFG_INVALID_PARAM_RET("Set dot1x port failed");        
    }

    /* set pae defaults */
    p_db_dot1x_port->auth_pae_quietPeriod = DOT1X_DEFAULT_PAE_QUIETPERIOD;
    rc = tbl_dot1x_port_set_dot1x_port_field(p_db_dot1x_port, TBL_DOT1X_PORT_FLD_AUTH_PAE_QUIETPERIOD);
    if (rc)        
    {            
        CFG_INVALID_PARAM_RET("Set dot1x port failed");        
    }
    
    p_db_dot1x_port->auth_pae_reAuthMax = DOT1X_DEFAULT_PAE_REAUTHMAX;
    rc = tbl_dot1x_port_set_dot1x_port_field(p_db_dot1x_port, TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHMAX);
    if (rc)        
    {            
        CFG_INVALID_PARAM_RET("Set dot1x port failed");        
    }
    
    p_db_dot1x_port->auth_pae_txPeriod = DOT1X_DEFAULT_PAE_TXPERIOD;
    rc = tbl_dot1x_port_set_dot1x_port_field(p_db_dot1x_port, TBL_DOT1X_PORT_FLD_AUTH_PAE_TXPERIOD);
    if (rc)        
    {            
        CFG_INVALID_PARAM_RET("Set dot1x port failed");        
    }

    /* set ctrl dir defaults */
    p_db_dot1x_port->auth_ctrl_dir_adminControlledDirections = TRUE;
    rc = tbl_dot1x_port_set_dot1x_port_field(p_db_dot1x_port,             
        TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_ADMINCONTROLLEDDIRECTIONS);        
    if (rc)        
    {            
        CFG_INVALID_PARAM_RET("Set dot1x port failed");        
    }
    
    dot1x_in_or_both(p_db_dot1x_port);
    p_db_dot1x_port->auth_ctrl_dir_state = IN_OR_BOTH;

    /* set reauth defaults */
    p_db_dot1x_port->reAuthPeriod = DOT1X_DEFAULT_REAUTH_PERIOD;
    rc = tbl_dot1x_port_set_dot1x_port_field(p_db_dot1x_port, TBL_DOT1X_PORT_FLD_REAUTHPERIOD);
    if (rc)        
    {            
        CFG_INVALID_PARAM_RET("Set dot1x port failed");        
    }
    
    /*the default authentication method should be remote server*/
    p_db_dot1x_port->session_authentication_method = AUTH_REMOTE_AUTHN_SERVER;

    return rc;
}

int32
dot1x_add_port(tbl_dot1x_port_t *p_dot1x_port, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    tbl_dot1x_port_t      *p_db_dot1x_port = NULL;
    int32                 rc = PM_E_NONE;
    
    rc = tbl_dot1x_port_add_dot1x_port(p_dot1x_port);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Create dot1x port failure");
    }

    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&p_dot1x_port->key);
    if (!p_db_dot1x_port)
    {
        CFG_FUNC_ERR_RET("Dot1x port is not created");
    }

    p_db_dot1x_port->sessiontype = SESSION_TYPE_8021X;

    rc = dot1x_port_set_defaults(p_db_dot1x_port, para);
    if (rc)
    {
        return rc;    
    }

    p_db_dot1x_port->p_buf = p_db_dot1x_port->buf;

    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, TBL_DOT1X_PORT_FLD_NAME);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port name return %d", rc);
    }

    sal_memcpy(&p_dot1x_port->src_mac, p_db_if->mac_addr, sizeof(mac_addr_t));
    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, TBL_DOT1X_PORT_FLD_SRC_MAC);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port src_mac return %d", rc);
    }
    
    p_dot1x_port->protocol_version = AUTH_EAPOL_PROTOCOL_VERSION_2;
    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, TBL_DOT1X_PORT_FLD_PROTOCOL_VERSION);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x dot1x port protocol_version failed");
    }  

    if (tbl_interface_is_up(p_db_if))
    {
        p_dot1x_port->portEnabled = TRUE;
        rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, TBL_DOT1X_PORT_FLD_PORTENABLED);
        if (rc)
        {
            CFG_FUNC_ERR_RET("Set dot1x portEnabled return %d", rc);
        }

        p_dot1x_port->radius_msg_id = INVALID_RADIUS_MSG_ID;
        rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, TBL_DOT1X_PORT_FLD_RADIUS_MSG_ID);
        if (rc)
        {
            CFG_FUNC_ERR_RET("Set dot1x port radius_msg_id return %d", rc);
        }

        if (!p_db_dot1x_port->t_timer)
        {
            p_db_dot1x_port->t_timer = ctc_task_add_timer(TASK_PRI_NORMAL, dot1x_port_timer_fn, 
                    p_db_dot1x_port, DOT1X_PORT_TIMER_INTERVAL); 
        }
    }

    return rc;
}

static int32
_dot1x_cmd_add_port(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_interface_t       intf;
    tbl_interface_t       *p_db_if = NULL;
    int32                 rc = PM_E_NONE;
    tbl_dot1x_port_t      dot1x_port;
    tbl_dot1x_port_t      *p_db_dot1x_port = NULL;
    int32                 field_id = 0;
    cdb_node_t            *p_node = NULL;
    cdb_field_t           *p_field = NULL;
    uint8                 ctrl = 0;
    tbl_dot1x_global_t    *p_db_glb = NULL;

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    
    /* 1. get ifindex from ifname */
    sal_memset(&intf, 0, sizeof(tbl_interface_t));    
    sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);
    
    p_db_if = tbl_interface_get_interface(&intf.key);
    if (!p_db_if)
    {
        CFG_FUNC_ERR_RET("Invalid interface or interface does not exist! must be access switch port or routed port!");
    }

    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);

    sal_strncpy(dot1x_port.name, argv[0], IFNAME_SIZE);
    rc = dot1x_port_check_dot1x_interface(&dot1x_port, para); 
    if (rc)
    {
        return rc;
    }

    dot1x_port.key.ifindex = p_db_if->ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        rc = dot1x_add_port(&dot1x_port, p_db_if, para);
        if (rc)
        {
            return rc;
        }

        p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
        if (!p_db_dot1x_port)
        {
            CFG_FUNC_ERR_RET("Dot1x port is not created");
        }

        p_db_dot1x_port->p_if = p_db_if;
    }

    p_node = cdb_get_tbl(TBL_DOT1X_PORT);

    field_id = cdb_get_field_id(argv[1], p_node, &p_field);
    if (field_id < 0)
    {
        CFG_FUNC_ERR_RET("Unknown field %s", argv[1]);
    }

    if (TBL_DOT1X_PORT_FLD_AUTHCONTROLLEDPORTCONTROL != field_id)
    {
        CFG_FUNC_ERR_RET("Set dot1x port failed for invalid parameter");
    }
    
    if (sal_strncmp (argv[2], "a", 1) == 0)
    {
        ctrl = AUTH_PORT_CTRL_AUTO;
    }
    else if (sal_strncmp (argv[2], "force-u", 7) == 0)
    {
        ctrl = AUTH_PORT_CTRL_FORCE_UNAUTHORIZED;
    }
    else if (sal_strncmp (argv[2], "force-a", 7) == 0)
    {
        ctrl = AUTH_PORT_CTRL_FORCE_AUTHORIZED;
    }

    if ((AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
           && (AUTH_PORT_CTRL_AUTO != ctrl)) 
    {
        CFG_FUNC_ERR_RET("Dot1x mac mode is conflict with force-authorized or force-unauthorized.");
    }

    dot1x_port.authControlledPortControl = ctrl;
    rc = tbl_dot1x_port_set_dot1x_port_field(&dot1x_port, TBL_DOT1X_PORT_FLD_AUTHCONTROLLEDPORTCONTROL);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port failed");
    }    

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_CONFLICT_RET("DOT1X_GLOBAL not found");
    }

    if (p_db_glb->global_auth_enable)
    {
        dot1x_port.portControl = ctrl;
    }
    else
    {
        dot1x_port.portControl = AUTH_PORT_CTRL_FORCE_AUTHORIZED;    
    }
    rc = tbl_dot1x_port_set_dot1x_port_field(&dot1x_port, TBL_DOT1X_PORT_FLD_PORTCONTROL);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port control failed");
    }

    rc = dot1x_set_fea_port_enable(p_db_dot1x_port);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("tbl_interface_set_interface_field set dot1x enable fail");
    }

    if ((AUTH_PORT_STATUS_UNAUTHORIZED == p_db_dot1x_port->portStatus)
          && (p_db_dot1x_port->portControl != AUTH_PORT_CTRL_AUTO)
          && (p_db_dot1x_port->guest_vlan_valid)&& (p_db_dot1x_port->guest_vlan))
    {  
        DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PORT[%s]: portChange to %s, Remove port from guest vlan(%d)", 
              get_intf_print_name(p_db_dot1x_port->name),((AUTH_PORT_CTRL_FORCE_AUTHORIZED == p_db_dot1x_port->portControl)? "ForceAuth" : "ForceUnauth"),
              p_db_dot1x_port->guest_vlan);
        
        p_db_dot1x_port->guest_vlan_valid = FALSE; /*guest vlan invalid*/    
        dot1x_set_fea_port_guest_vlan(p_db_dot1x_port);   
    }

    /* Run the state machine with change in portControl.  */
    dot1x_run_sm(p_db_dot1x_port);
    
    return rc;
}

int32
dot1x_port_delete(tbl_dot1x_port_t *p_db_dot1x_port, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32                rc = PM_E_NONE;
    
    if (tbl_interface_is_up(p_db_if))
    {
        p_db_dot1x_port->portEnabled = FALSE;
        rc = tbl_dot1x_port_set_dot1x_port_field(p_db_dot1x_port, TBL_DOT1X_PORT_FLD_PORTENABLED);
        if (rc)
        {
            CFG_FUNC_ERR_RET("Set dot1x portEnabled return %d", rc);
        }

        if (p_db_dot1x_port->t_timer)
        {
            ctc_task_delete_timer(p_db_dot1x_port->t_timer);
            p_db_dot1x_port->t_timer = NULL;
        }
    }

    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PORT[%s]: Delete the port", get_intf_print_name(p_db_dot1x_port->name));

    if (p_db_dot1x_port->t_wait)
    {
        ctc_task_delete_timer(p_db_dot1x_port->t_wait);
        p_db_dot1x_port->t_wait = NULL;
    }

    dot1x_radius_end_association(p_db_dot1x_port);

    dot1x_delete_session(p_db_dot1x_port->key.ifindex);

    return rc;
}

static int32
_dot1x_cmd_delete_port(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_interface_t       intf;
    tbl_interface_t       *p_db_if = NULL;
    int32                 rc = PM_E_NONE;
    tbl_dot1x_port_t      dot1x_port;
    tbl_dot1x_port_t      *p_db_dot1x_port = NULL;

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    
    /* 1. get ifindex from ifname */
    sal_memset(&intf, 0, sizeof(tbl_interface_t));    
    sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);
    
    p_db_if = tbl_interface_get_interface(&intf.key);
    if (!p_db_if)
    {
        return PM_E_NONE;
    }

    dot1x_port.key.ifindex = p_db_if->ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        CFG_FUNC_ERR_RET("802.1x not configured on this interface!");
    }

    if (AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
    {
        dot1x_api_user_off_line_whole_port(p_db_dot1x_port);
    }

    if (0 != p_db_dot1x_port->guest_vlan)
    {
        rc = dot1x_cmd_set_port_guest_vlan(&dot1x_port, p_db_dot1x_port, p_db_if, 
            TBL_DOT1X_PORT_FLD_GUEST_VLAN, para, FALSE);
        if (rc)
        {
            return rc;
        }
    }

    if (AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
    {
        dot1x_port.mac_auth_bypass = FALSE;
        rc = dot1x_cmd_set_port_mac_auth_bypass(&dot1x_port, p_db_dot1x_port, 
            TBL_DOT1X_PORT_FLD_MAC_AUTH_BYPASS, para);
        if (rc)
        {
            return rc;
        }
        
        dot1x_port.auth_mode = AUTH_PORT_MODE_PORT;
        rc = dot1x_cmd_set_port_authmode(&dot1x_port, p_db_dot1x_port, 
            TBL_DOT1X_PORT_FLD_AUTH_MODE, para, FALSE);
        if (rc)
        {
            return rc;
        }
    }

    dot1x_set_fea_port_authorize(p_db_dot1x_port);
    dot1x_set_fea_port_disable(p_db_dot1x_port);

    dot1x_port_delete(p_db_dot1x_port, p_db_if, para);

    dot1x_port.key.ifindex = p_db_if->ifindex;
    rc = tbl_dot1x_port_del_dot1x_port(&dot1x_port.key);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Port does not exist");
    }
       
    return rc;
}

static int32
_dot1x_cmd_set_port_control_dir(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    int32 field_id, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if ((AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode) 
        && (!p_dot1x_port->auth_ctrl_dir_adminControlledDirections))
    {
        CFG_FUNC_ERR_RET("Dot1x mac based mode do not support control direction both.");
    }

    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, field_id);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x control dir failed");
    }

    p_dot1x_port->auth_ctrl_dir_operControlledDirections
        = p_dot1x_port->auth_ctrl_dir_adminControlledDirections; 
    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_OPERCONTROLLEDDIRECTIONS);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x control dir failed");
    }

    dot1x_set_fea_port_ctrl(p_db_dot1x_port);

    /* Run the state machine with change in adminControlledDirection.  */
    dot1x_run_sm(p_db_dot1x_port);

    return rc;
}

static int32
_dot1x_cmd_set_port_initialize(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    tbl_interface_t   *p_db_if, int32 field_id, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_dot1x_global_t    *p_db_glb = NULL;
    
    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("Tbl_dot1x_global is not found");
    }

    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, field_id);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port initialize failed");
    }

    if (tbl_interface_is_up(p_db_if))
    {
        dot1x_port_init(p_db_dot1x_port);
    }

    if (p_db_glb->global_auth_enable)
    {
        /*set to data plane*/
        dot1x_set_fea_mode(p_db_dot1x_port);
    }

    return rc;
}

static int32
_dot1x_cmd_set_port_auth_pae_reAuthMax(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    int32 field_id, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, field_id);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port auth pae reAuthMax failed");
    }

    return rc;
}

static int32
_dot1x_cmd_set_port_protocol_version(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    int32 field_id, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, field_id);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port protocol version failed");
    }

    return rc;
}

static int32
_dot1x_cmd_set_port_reauth_enabled(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    int32 field_id, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, field_id);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port reauth enabled failed");
    }

    return rc;
}

static int32
_dot1x_cmd_set_port_reauthenticate(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    int32 field_id, cfg_cmd_para_t *para)
{
    int32                   rc = PM_E_NONE;
    tbl_dot1x_mac_master_t  *p_glb_dot1x_mac = NULL;
    tbl_dot1x_mac_t         *p_db_dot1x_mac = NULL;
    ctclib_slistnode_t      *listnode = NULL;  

    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, field_id);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port reauthenticate failed");
    }

    p_glb_dot1x_mac = tbl_dot1x_mac_get_master();

    if (!p_glb_dot1x_mac)
    {
        CFG_FUNC_ERR_RET("_dot1x_cmd_set_port_reauthenticate fail because tbl_dot1x_mac is not exist");
    }

    if(AUTH_PORT_MODE_MAC == p_dot1x_port->auth_mode)
    {
        p_dot1x_port->share_reauth = TRUE;

        CTCLIB_SLIST_LOOP(p_glb_dot1x_mac->dot1x_mac_list, p_db_dot1x_mac, listnode)
        {
            if (p_db_dot1x_mac->key.ifindex == p_dot1x_port->key.ifindex)
            {
                if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT))
                {
                    if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_BYPASS))
                    {
                        GLB_UNSET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT);
                        GLB_SET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT);
    
                        dot1x_set_fea_add_mac(p_db_dot1x_mac); 
                    }
                    else
                    {
                        if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_EAPOL_REAUTH))
                        {
                            continue;    
                        }
                        GLB_SET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_EAPOL_REAUTH);
                    }
        
                    if (p_db_dot1x_mac->t_reauth)
                    {
                        ctc_task_delete_timer(p_db_dot1x_mac->t_reauth);
                        p_db_dot1x_mac->t_reauth = NULL;
                    }
    
                    p_db_dot1x_mac->t_reauth= ctc_task_add_timer(TASK_PRI_NORMAL, dot1x_reauth_timer_fn, 
                        p_db_dot1x_mac, AUTH_MAC_ENTRY_WAITING_TIME);
                }
            }
        }  
    }

    return rc;
}

int32
dot1x_cmd_set_port_guest_vlan(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    tbl_interface_t  *p_db_if, int32 field_id, cfg_cmd_para_t *para, bool check)
{
    int32          rc = PM_E_NONE;
    uint16         old_guest_vlan = 0;
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t     *p_db_new_vlan = NULL;
    tbl_vlan_t     *p_db_old_vlan = NULL;

    sal_memset(&vlan_key, 0, sizeof(tbl_vlan_key_t));

    if ((GLB_IF_MODE_L2 != p_db_if->mode) 
        || (GLB_VLAN_PORT_TYPE_ACCESS != p_db_if->vlan_type))
    {
        CFG_FUNC_ERR_RET("Invalid interface! must be access port.");
    }

    if ((p_db_dot1x_port->guest_vlan != p_dot1x_port->guest_vlan)
        && (AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode) && check)
    {
        if ((p_db_dot1x_port->current_user != 0))
        {
            CFG_FUNC_ERR_RET("There are users on line, cannot change configure.");
        }
        else if ((AUTH_PORT_STATUS_AUTHORIZED == p_db_dot1x_port->portStatus)
            && (AUTH_PORT_CTRL_AUTO == p_db_dot1x_port->portControl))
        {
            CFG_FUNC_ERR_RET("There are users on line, cannot change configure.");
        }
    }

    /* modified by liwh for bug 40620, 2017-01-22 */
    if (p_db_if->brgif && (p_db_if->brgif->pvid == p_dot1x_port->guest_vlan))
    {
        CFG_CONFLICT_RET("Can't configure guest VLAN same as native VLAN");    
    }
    /* liwh end */

    if (p_dot1x_port->guest_vlan)
    {
        vlan_key.vid = p_dot1x_port->guest_vlan;
        p_db_new_vlan = tbl_vlan_get_vlan(&vlan_key);
        if (!p_db_new_vlan)
        {
            CFG_FUNC_ERR_RET("VLAN is not configured");    
        }
    }

    if (p_db_dot1x_port->guest_vlan)
    {
        vlan_key.vid = p_db_dot1x_port->guest_vlan;
        p_db_old_vlan = tbl_vlan_get_vlan(&vlan_key);
        if (!p_db_old_vlan)
        {
            CFG_FUNC_ERR_RET("VLAN is not configured");    
        }
    }

    if (p_db_new_vlan)
    {
        p_db_new_vlan->dot1x_guest_vlan_num++;
        tbl_vlan_set_vlan_field(p_db_new_vlan, TBL_VLAN_FLD_DOT1X_GUEST_VLAN_NUM);
    }

    if (p_db_old_vlan && p_db_old_vlan->dot1x_guest_vlan_num > 0)
    {
        p_db_old_vlan->dot1x_guest_vlan_num--;
        tbl_vlan_set_vlan_field(p_db_old_vlan, TBL_VLAN_FLD_DOT1X_GUEST_VLAN_NUM);
    }

    old_guest_vlan = p_db_dot1x_port->guest_vlan;

    p_db_dot1x_port->guest_vlan = p_dot1x_port->guest_vlan;
    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, TBL_DOT1X_PORT_FLD_GUEST_VLAN);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x control dir failed");
    }    

    /* modified by liwh for bug 40620, 2017-01-22 
       when dot1x is not enabled, guest_vlan will not be set to p_db_if->dot1x_guest_vlan,
       therefore it will be reserved in dot1x_cfg_guest_vlan */
    p_db_if->dot1x_cfg_guest_vlan = p_dot1x_port->guest_vlan;
    rc = tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_DOT1X_CFG_GUEST_VLAN);
    if (rc)
    {
        DOT1X_LOG_ERR("dot1x_set_port_authorize set TBL_INTERFACE_FLD_DOT1X_GUEST_VLAN return %d", rc);
        return PM_E_FAIL;
    }
    /* liwh end */

    if ((AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
        && (AUTH_PORT_CTRL_AUTO == p_db_dot1x_port->portControl))
    {
        dot1x_set_fea_port_mac_enable(p_db_dot1x_port);

        /* add port to guest vlan */
        if (p_db_dot1x_port->guest_vlan)
        {
            p_db_dot1x_port->guest_vlan_valid = TRUE; 
        }
        else
        {
            p_db_dot1x_port->guest_vlan_valid = FALSE; 
        }
        dot1x_set_fea_mac_guest_vlan(p_db_dot1x_port);  
        return rc;
    }
    
    if ((AUTH_PORT_STATUS_UNAUTHORIZED == p_db_dot1x_port->portStatus)
            && (AUTH_PORT_CTRL_AUTO == p_db_dot1x_port->portControl)
            && (old_guest_vlan != p_db_dot1x_port->guest_vlan)
            && p_db_dot1x_port->guest_vlan_valid)
    { 
        DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PORT[%s]: GuestVlan change/delete, Remove port from guest vlan(%d)", 
            get_intf_print_name(p_db_dot1x_port->name), old_guest_vlan);
        
        p_db_dot1x_port->guest_vlan_valid = FALSE; /*guest vlan invalid*/
        dot1x_set_fea_port_guest_vlan(p_db_dot1x_port);        
    }

    return rc;
}

static int32
_dot1x_cmd_set_port_pae_quietperiod(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    int32 field_id, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, field_id);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port pae quietperiod failed");
    }

    p_dot1x_port->quietWhile = p_dot1x_port->auth_pae_quietPeriod;
    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, TBL_DOT1X_PORT_FLD_QUIETWHILE);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port quiet while failed");
    }

    return rc;
}

static int32
_dot1x_cmd_set_port_pae_txperiod(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    int32 field_id, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, field_id);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port pae txperiod failed");
    }

    p_dot1x_port->txWhen = p_dot1x_port->auth_pae_txPeriod;
    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, TBL_DOT1X_PORT_FLD_TXWHEN);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port txwhile failed");
    }

    return rc;
}

static int32
_dot1x_cmd_set_port_reauthperiod(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    int32 field_id, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, field_id);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port reauthperiod failed");
    }

    p_dot1x_port->reAuthWhen = p_dot1x_port->reAuthPeriod;
    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, TBL_DOT1X_PORT_FLD_REAUTHWHEN);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port reauthwhen failed");
    }

    return rc;
}

static int32
_dot1x_cmd_set_port_be_supptimeout(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    int32 field_id, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, field_id);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port be supptimeout failed");
    }

    return rc;
}

static int32
_dot1x_cmd_set_port_be_servertimeout(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    int32 field_id, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, field_id);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x be servertimeout failed");
    }

    return rc;
}

bool
dot1x_if_conflict_check(tbl_interface_t  *p_db_if)
{
    if (!p_db_if)
    {
        return FALSE;
    }

    if (p_db_if->ip_source_type)
    {
        return TRUE;    
    }

    if (p_db_if->vlanclass_active_type)
    {
        return TRUE;
    }

    /*modified by yejl for bug48461, 2018-08-13*/
    if (p_db_if->brgif)
    {
        if (p_db_if->brgif->vlan_translation_en)
        {
            return TRUE;
        }
    }

    return FALSE;
}

int32
dot1x_cmd_set_port_authmode(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    int32 field_id, cfg_cmd_para_t *para, bool check)
{
    int32                 rc = PM_E_NONE;
    tbl_dot1x_global_t    *p_db_glb = NULL;
    tbl_interface_t       *p_db_if = NULL;

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("Tbl_dot1x_global is not found");
    }

    /* no change */
    if (p_dot1x_port->auth_mode == p_db_dot1x_port->auth_mode)
    {
        return rc;    
    }

    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_dot1x_port->key.ifindex);
    if (!p_db_if)
    {
        CFG_FUNC_ERR_RET("dot1x_cmd_set_port_authmode fail because Invalid interface or interface does not exist");
    }

    if (!p_db_glb->global_auth_enable)
    {
        CFG_FUNC_ERR_RET("Dot1x mac based mode can't be configured when dot1x is not enabled globally");    
    }
    
    /*user on line */
    if (check)
    {
        if ((0 != p_db_dot1x_port->current_user))
        {
            CFG_FUNC_ERR_RET("There are users on line, cannot change configure.");
        }
        else if((AUTH_PORT_MODE_PORT == p_db_dot1x_port->auth_mode)
            && (AUTH_PORT_STATUS_AUTHORIZED == p_db_dot1x_port->portStatus)         
            && (AUTH_PORT_CTRL_AUTO == p_db_dot1x_port->portControl))
        {
            CFG_FUNC_ERR_RET("There are users on line, cannot change configure.");
        }
    
        if ((AUTH_PORT_MODE_MAC == p_dot1x_port->auth_mode)
            && (AUTH_CTRL_DIR_BOTH == p_db_dot1x_port->auth_ctrl_dir_adminControlledDirections))
        {
            CFG_FUNC_ERR_RET("Dot1x mac based mode do not support control direction both.");
        } 
    
        if (AUTH_PORT_MODE_MAC == p_dot1x_port->auth_mode)
        {
            if (dot1x_if_conflict_check(p_db_if))
            {
                CFG_FUNC_ERR_RET("Ip source guard, vlan classifier , vlan translation  or other function is enable on this interface.");    
            }
        }

        if ((AUTH_PORT_MODE_PORT == p_dot1x_port->auth_mode)
           && (p_db_dot1x_port->mac_auth_bypass))
        {
            CFG_FUNC_ERR_RET("Mac auth bypass is enabled, not support change to port mode.");
        }

        if ((AUTH_PORT_MODE_MAC == p_dot1x_port->auth_mode)
           && (AUTH_PORT_CTRL_AUTO != p_db_dot1x_port->authControlledPortControl)) 
        {
            CFG_FUNC_ERR_RET("Dot1x mac mode is conflict with force-authorized or force-unauthorized.");
        }
    }

    if (p_db_dot1x_port->guest_vlan_valid)
    {
        if (AUTH_PORT_MODE_PORT == p_db_dot1x_port->auth_mode)
        {   
            /* remove port from guest vlan, change pvid to configuration */
            p_db_dot1x_port->guest_vlan_valid = FALSE; 
            dot1x_set_fea_port_guest_vlan(p_db_dot1x_port); 
        }
        else
        {
            /* remove port from guest vlan */
            p_db_dot1x_port->guest_vlan_valid = FALSE; 
            dot1x_set_fea_mac_guest_vlan(p_db_dot1x_port); 
        }
    }
    
    p_db_dot1x_port->auth_mode = p_dot1x_port->auth_mode;
    rc = tbl_dot1x_port_set_dot1x_port_field(p_db_dot1x_port, field_id);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port auth mode failed");
    }

    /* set auth mode to fea */
    if (AUTH_PORT_MODE_PORT == p_dot1x_port->auth_mode)
    {
        GLB_UNSET_FLAG(p_db_if->dot1x_flag, GLB_IF_DOT1X_MODE_MAC);
    }
    else
    {
        GLB_SET_FLAG(p_db_if->dot1x_flag, GLB_IF_DOT1X_MODE_MAC);
    }
    
    rc = tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_DOT1X_MODE_MAC);
    if (rc)
    {
        DOT1X_LOG_ERR("dot1x_cmd_set_port_authmode set TBL_INTERFACE_FLD_DOT1X_MODE_MAC return %d", rc);
        return rc;     
    }     

    if(AUTH_PORT_MODE_MAC == p_dot1x_port->auth_mode)
    {       
        p_db_glb->mac_port_count_current++;
    }
    else
    {
        p_db_glb->mac_port_count_current--;
    }

    if (tbl_interface_is_up(p_db_if))
    {
        dot1x_port_init(p_db_dot1x_port);
        dot1x_set_port_mode(p_db_glb, p_db_dot1x_port);
    }

    return rc;
}

int32
dot1x_cmd_set_port_mac_auth_bypass(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    int32 field_id, cfg_cmd_para_t *para)
{
    int32                 rc = PM_E_NONE;
    tbl_dot1x_global_t    *p_db_glb = NULL;
    tbl_interface_t       *p_db_if = NULL;

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("Tbl_dot1x_global is not found");
    }

    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_dot1x_port->key.ifindex);
    if (!p_db_if)
    {
        CFG_FUNC_ERR_RET("dot1x_cmd_set_port_mac_auth_bypass fail because Invalid interface or interface does not exist");
    }

    /* no change */
    if (p_dot1x_port->mac_auth_bypass == p_db_dot1x_port->mac_auth_bypass)
    {
        return rc;    
    }

    if(AUTH_PORT_MODE_MAC != p_db_dot1x_port->auth_mode)
    {
        CFG_FUNC_ERR_RET("Port auth mode is port, can not support mac auth bypass now.");
    }

    /*user on line */
    if (0 != p_db_dot1x_port->current_user)
    {
        CFG_FUNC_ERR_RET("There are users on line, cannot change configure.");
    }

    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, field_id);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port mac auth bypass failed");
    }

    if (tbl_interface_is_up(p_db_if))
    {
        dot1x_set_port_mode(p_db_glb, p_db_dot1x_port);
    }

    return rc;
}

static int32
_dot1x_cmd_set_port_max_user(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    int32 field_id, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_dot1x_global_t *p_db_glb = NULL;
    
    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("Tbl_dot1x_global is not found");
    }

    /* no change */
    if (p_dot1x_port->max_user == p_db_dot1x_port->max_user)
    {
        return rc;    
    }

    /* user on line */
    if (0 != p_dot1x_port->max_user
        && ((p_db_dot1x_port->current_user > p_dot1x_port->max_user) 
        || (p_dot1x_port->max_user > DEFAULT_MAC_USER_COUNT_MAX)))
    {
        CFG_FUNC_ERR_RET("Current user count is large than the configure value, or global resource is limited.");
    }
    
    /* default value */
    if (p_dot1x_port->max_user == p_db_glb->mac_user_count_max)
    {
        p_dot1x_port->max_user = 0; /*uncontrol*/
    }

    rc = tbl_dot1x_port_set_dot1x_port_field(p_dot1x_port, field_id);
    if (rc)
    {
        CFG_FUNC_ERR_RET("Set dot1x port mac auth bypass failed");
    }

    return rc;
}

int32
_dot1x_cmd_get_port(char **argv, int32 argc, cfg_cmd_para_t *para, cdb_node_t  *p_node)
{
    FILE                      *fp = NULL;
    tbl_iter_args_t           args;     
    tbl_dot1x_port_master_t   *p_glb_port = NULL;
    tbl_dot1x_port_t          *p_db_dot1x_port = NULL;
    ctclib_slistnode_t        *listnode = NULL;  
    tbl_dot1x_port_t          dot1x_port;
    tbl_interface_t           *p_db_if = NULL;
    tbl_interface_t           intf;
    int32                     field_id = 0;
    field_parse_t             field;

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    sal_memset(&args, 0 ,sizeof(args));

    /* check exist */
    p_glb_port = tbl_dot1x_port_get_master();

    if (!p_glb_port)
    {
        CFG_FUNC_ERR_RET("dot1x_show_all fail because tbl_dot1x_port is not exist");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    args.argv[0] = &field;
    args.argv[1] = fp;
    
    field_id = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
    if (field_id < 0)
    {
        CFG_DEL_SHOW_PATH(para->p_show->path, fp);
        CFG_INVALID_PARAM_RET("Invalid field");
    }

    if (0 == argc)
    {
        CTCLIB_SLIST_LOOP(p_glb_port->dot1x_port_list, p_db_dot1x_port, listnode)
        {
            tbl_dot1x_port_dump_one(p_db_dot1x_port, &args);
        }
    }
    else
    {
        sal_memset(&intf, 0, sizeof(tbl_interface_t));    
        sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);
        
        p_db_if = tbl_interface_get_interface(&intf.key);
        if (!p_db_if)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            return PM_E_NONE;
        }
        
        dot1x_port.key.ifindex = p_db_if->ifindex;
        p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
        if (!p_db_dot1x_port)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_FUNC_ERR_RET("802.1x not configured on this interface!");
        }
        
        tbl_dot1x_port_dump_one(p_db_dot1x_port, &args);
    }
            
    sal_fclose(fp);
    return PM_E_NONE;
}

int32
dot1x_cmd_process_port(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t              *p_node = NULL;
    int32                   rc = PM_E_NONE;
    tbl_dot1x_port_t        dot1x_port;
    tbl_dot1x_port_t        *p_db_dot1x_port = NULL;
    int32                   i = 0;
    cdb_field_t             *p_field = NULL;
    int32                   field_id = 0;
    tbl_interface_t         intf;
    tbl_interface_t         *p_db_if = NULL;

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    
    CFG_DBG_FUNC();
    
    p_node = cdb_get_tbl(TBL_DOT1X_PORT);
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (argc < 3)
        {
            CFG_INVALID_PARAM_RET("Invalid action");
        }
        rc = _dot1x_cmd_add_port(argv, argc, para);
        break;

    case CDB_OPER_DEL:
        rc = _dot1x_cmd_delete_port(argv, argc, para);
        break;
        
    case CDB_OPER_SET:
        /* 1. get ifindex from ifname */
        sal_memset(&intf, 0, sizeof(tbl_interface_t));    
        sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);
        
        p_db_if = tbl_interface_get_interface(&intf.key);
        if (!p_db_if)
        {
            return PM_E_NONE;
        }

        field_id = cdb_get_field_id(argv[1], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[1]);
        }
        i = 2;

        if ((TBL_DOT1X_PORT_FLD_INITIALIZE == field_id)
            || (TBL_DOT1X_PORT_FLD_AUTH_MODE == field_id)
            || (TBL_DOT1X_PORT_FLD_MAC_AUTH_BYPASS == field_id)
            || (TBL_DOT1X_PORT_FLD_MAX_USER == field_id))
        {
            sal_strncpy(dot1x_port.name, intf.key.name, IFNAME_SIZE);
            rc = dot1x_port_check_dot1x_interface(&dot1x_port, para); 
            if (rc)
            {
                return rc;
            }
        }
    
        dot1x_port.key.ifindex = p_db_if->ifindex;
        p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
        if (!p_db_dot1x_port)
        {
            CFG_FUNC_ERR_RET("802.1x not configured on this interface!");
        }

        switch (field_id)
        {
        case TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_ADMINCONTROLLEDDIRECTIONS:
            PM_CFG_GET_UINT(dot1x_port.auth_ctrl_dir_adminControlledDirections, argv[i], para->p_rs);
            rc = _dot1x_cmd_set_port_control_dir(&dot1x_port, p_db_dot1x_port, field_id, para);
            break;

        case TBL_DOT1X_PORT_FLD_INITIALIZE:
            PM_CFG_GET_UINT(dot1x_port.initialize, argv[i], para->p_rs);
            rc = _dot1x_cmd_set_port_initialize(&dot1x_port, p_db_dot1x_port, p_db_if, field_id, para);
            break;

        case TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHMAX:
            PM_CFG_GET_UINT(dot1x_port.auth_pae_reAuthMax, argv[i], para->p_rs);
            rc = _dot1x_cmd_set_port_auth_pae_reAuthMax(&dot1x_port, p_db_dot1x_port, field_id, para);
            break;

        case TBL_DOT1X_PORT_FLD_PROTOCOL_VERSION:
            PM_CFG_GET_UINT(dot1x_port.protocol_version, argv[i], para->p_rs);
            rc = _dot1x_cmd_set_port_protocol_version(&dot1x_port, p_db_dot1x_port, field_id, para);
            break;

        case TBL_DOT1X_PORT_FLD_REAUTHENABLED:
            PM_CFG_GET_UINT(dot1x_port.reAuthEnabled, argv[i], para->p_rs);
            rc = _dot1x_cmd_set_port_reauth_enabled(&dot1x_port, p_db_dot1x_port, field_id, para);
            break;

        case TBL_DOT1X_PORT_FLD_REAUTHENTICATE:   
            PM_CFG_GET_UINT(dot1x_port.reAuthenticate, argv[i], para->p_rs);
            rc = _dot1x_cmd_set_port_reauthenticate(&dot1x_port, p_db_dot1x_port, field_id, para);
            break;

        case TBL_DOT1X_PORT_FLD_GUEST_VLAN:
            PM_CFG_GET_UINT(dot1x_port.guest_vlan, argv[i], para->p_rs);

            if (0 != dot1x_port.guest_vlan)
            {
                dot1x_port.guest_vlan_valid = TRUE;
            }
            
            rc = dot1x_cmd_set_port_guest_vlan(&dot1x_port, p_db_dot1x_port, p_db_if, field_id, para, TRUE);
            break;

        case TBL_DOT1X_PORT_FLD_AUTH_PAE_QUIETPERIOD:
            PM_CFG_GET_UINT(dot1x_port.auth_pae_quietPeriod, argv[i], para->p_rs);
            rc = _dot1x_cmd_set_port_pae_quietperiod(&dot1x_port, p_db_dot1x_port, field_id, para);
            break;

        case TBL_DOT1X_PORT_FLD_AUTH_PAE_TXPERIOD:
            PM_CFG_GET_UINT(dot1x_port.auth_pae_txPeriod, argv[i], para->p_rs);
            rc = _dot1x_cmd_set_port_pae_txperiod(&dot1x_port, p_db_dot1x_port, field_id, para);
            break;

        case TBL_DOT1X_PORT_FLD_REAUTHPERIOD:
            PM_CFG_GET_UINT(dot1x_port.reAuthPeriod, argv[i], para->p_rs);
            rc = _dot1x_cmd_set_port_reauthperiod(&dot1x_port, p_db_dot1x_port, field_id, para);
            break;

        case TBL_DOT1X_PORT_FLD_AUTH_BE_SUPPTIMEOUT:
            PM_CFG_GET_UINT(dot1x_port.auth_be_suppTimeout, argv[i], para->p_rs);
            rc = _dot1x_cmd_set_port_be_supptimeout(&dot1x_port, p_db_dot1x_port, field_id, para);
            break;

        case TBL_DOT1X_PORT_FLD_AUTH_BE_SERVERTIMEOUT:
            PM_CFG_GET_UINT(dot1x_port.auth_be_serverTimeout, argv[i], para->p_rs);
            rc = _dot1x_cmd_set_port_be_servertimeout(&dot1x_port, p_db_dot1x_port, field_id, para);
            break;

        case TBL_DOT1X_PORT_FLD_AUTH_MODE:
            PM_CFG_GET_UINT(dot1x_port.auth_mode, argv[i], para->p_rs);
            rc = dot1x_cmd_set_port_authmode(&dot1x_port, p_db_dot1x_port, field_id, para, TRUE);
            break;

        case TBL_DOT1X_PORT_FLD_MAC_AUTH_BYPASS:
            PM_CFG_GET_UINT(dot1x_port.mac_auth_bypass, argv[i], para->p_rs);
            rc = dot1x_cmd_set_port_mac_auth_bypass(&dot1x_port, p_db_dot1x_port, field_id, para);
            break;

        case TBL_DOT1X_PORT_FLD_MAX_USER:
            PM_CFG_GET_UINT(dot1x_port.max_user, argv[i], para->p_rs);
            rc = _dot1x_cmd_set_port_max_user(&dot1x_port, p_db_dot1x_port, field_id, para);
            break;
            
        default:
            CFG_FUNC_ERR_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        rc = _dot1x_cmd_get_port(argv, argc, para, p_node);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int
dot1x_radius_server_check_ip(addr_t *pAddr)
{
    u_int32_t network;
    
    if (AF_INET == pAddr->family) {
        u_char *p =  (u_char *)&pAddr->u.prefix4;
        
        /* filtrate ip with 255.255.255.255 */
        if (255 == p[0] && 255 == p[1] && 255 == p[2] && 255 == p[3]) {
            return -1;
        }
        
        network = ntohl (*(int*)&pAddr->u.prefix4);
        if (network == 0 || IN_BADCLASS(network) || IN_MULTICAST (network)
            || IN_EXPERIMENTAL(network) || IPV4_NET127(network)
            || IPV4_NET0(network)) {
            return -1;
        }
    } else if (IN6_IS_ADDR_MULTICAST (&pAddr->u.prefix6)
               || IN6_IS_ADDR_LOOPBACK (&pAddr->u.prefix6)
               || IN6_IS_ADDR_UNSPECIFIED (&pAddr->u.prefix6)) {
        return -1;
    }
    
    return 0;
}

int32
dot1x_get_ip_str2val(char *str, addr_t *addr)
{
    if (1 == sal_inet_pton(AF_INET, str, &addr->u.prefix4))
    {  
        addr->family = AF_INET;
        return PM_E_NONE;
    }
    else if (1 == sal_inet_pton(AF_INET6, str, &addr->u.prefix6))
    {
        addr->family = AF_INET6;
        return PM_E_NONE;
    }
    
    return PM_E_FAIL;
}


static int32
_dot1x_cmd_add_radius(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                       rc = PM_E_NONE;
    cdb_node_t                  *p_node = NULL;
    uint8                       i = 0;
    int32                       field_id = 0;
    int32                       is_inband = 0;    
    cdb_field_t                 *p_field = NULL;
    tbl_dot1x_radius_t          *p_db_dot1x_radius = NULL;
    tbl_dot1x_radius_t          dot1x_radius;
    uint32                      value = 0;
    tbl_dot1x_global_t          *p_db_glb = NULL;
    tbl_dot1x_radius_master_t   *p_glb_radius = NULL;
    char                        input[11][256];
    char                        *p_input[11];
    
    sal_memset(&dot1x_radius, 0, sizeof(tbl_dot1x_radius_t));
    
    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("DOT1X_GLOBAL not found");
    }

    /* check exist */
    p_glb_radius = tbl_dot1x_radius_get_master();
    if (!p_glb_radius)
    {
        CFG_FUNC_ERR_RET("_dot1x_cmd_add_radius fail because tbl_dot1x_radius is not exist");
    }

    p_node = cdb_get_tbl(TBL_DOT1X_RADIUS);

    field_id = cdb_get_field_id(argv[i], p_node, &p_field);
    if (field_id < 0)
    {
        CFG_FUNC_ERR_RET("Unknown field %s", argv[i]);
    }

    if (TBL_DOT1X_RADIUS_FLD_KEY == field_id)
    {
        i++;
        rc = dot1x_get_ip_str2val(argv[i], &dot1x_radius.key.addr);
        if (PM_E_NONE != rc)
        {
             CFG_INVALID_PARAM_RET("Invalid address!");    
        }
        sal_memcpy(dot1x_radius.server_string, argv[i], sal_strlen(argv[i]));

        i++;
        PM_CFG_GET_UINT(dot1x_radius.key.sin_port, argv[i], para->p_rs);

        if (dot1x_radius_server_check_ip(&dot1x_radius.key.addr) != 0) 
        {
            CFG_INVALID_PARAM_RET("Invalid address!");
        }

        if (sal_strcmp(argv[4], ""))
        {
            if (0 != sal_strcmp(argv[3], "shared_enc_secret"))
            {
                rc = dot1x_radius_check_shared_key (argv[4], para);
                if (rc)
                {
                    return rc;    
                }
            }
            else
            {
                rc = dot1x_radius_check_enc_shared_key (argv[4], para);
                if (rc)
                {
                    return rc;    
                }
            }
        }
        else
        {
            if (0 == sal_strcmp(argv[3], "shared_enc_secret"))
            {
                CFG_FUNC_ERR_RET("Invalid key string");
            }
        }

        if (p_glb_radius->radius_list->count >= RADIUS_SERVER_MAX_COUNT)
        {
            CFG_FUNC_ERR_RET("No more than 3 RADIUS servers are supported.");
        }
        
        p_db_dot1x_radius = tbl_dot1x_radius_get_dot1x_radius(&dot1x_radius.key);
        if (!p_db_dot1x_radius)
        {
            rc = tbl_dot1x_radius_add_dot1x_radius(&dot1x_radius);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Create dot1x radius %s failure", argv[1]);
            }       
        }
        else
        {
            CFG_FUNC_ERR_RET("Create radius server %s auth_port %d failure when it is exist", argv[1], dot1x_radius.key.sin_port);
        }

        p_db_dot1x_radius = tbl_dot1x_radius_get_dot1x_radius(&dot1x_radius.key);
        if (!p_db_dot1x_radius)
        {
            CFG_FUNC_ERR_RET("Create dot1x radius %s failure", argv[1]);
        } 

        rc = tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_SERVER_STRING);
        if (rc)
        {
            CFG_FUNC_ERR_RET("Set dot1x radius server string failed");
        }  
    }

    i++;
    field_id = cdb_get_field_id(argv[i], p_node, &p_field);
    if (field_id < 0)
    {
        CFG_FUNC_ERR_RET("Unknown field %s", argv[i]);
    }

    if ((TBL_DOT1X_RADIUS_FLD_SHARED_SECRET == field_id)
        || (TBL_DOT1X_RADIUS_FLD_SHARED_ENC_SECRET == field_id))
    {
        i++;
        if (sal_strcmp(argv[i], ""))
        {    
            value = FALSE;
            GLB_SET_FLAG_COND(value, dot1x_radius.radius_config, AUTH_RADIUS_CONFIG_DEFAULT_KEY);
            rc = tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_KEY);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius default shared secret failed");
            }

            if (TBL_DOT1X_RADIUS_FLD_SHARED_SECRET == field_id)
            {
                if (app_is_password_encryption())
                {
                    char szTmp[PASSWD_SIZE];
                    gen_passwd_crypt(argv[i], szTmp, PASSWD_SIZE);
                    snprintf(dot1x_radius.shared_enc_secret, PASSWD_SIZE, "%s", szTmp);
                    sal_memset(&dot1x_radius.shared_secret, 0, RADIUS_SHARED_SECRET_LEN + 1);
                }
                else
                {
                    sal_memcpy(dot1x_radius.shared_secret, argv[i], sal_strlen(argv[i]));
                    sal_memset(&dot1x_radius.shared_enc_secret, 0, PASSWD_SIZE);
                }
            }
            else
            {
                sal_memcpy(dot1x_radius.shared_enc_secret, argv[i], sal_strlen(argv[i]));
                sal_memset(&dot1x_radius.shared_secret, 0, RADIUS_SHARED_SECRET_LEN + 1);
            }

#if 0    
            dot1x_radius.shared_secret_valid = TRUE;
     
            rc = tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_SHARED_SECRET);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius default shared secret failed");
            }
    
            rc = tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_SHARED_SECRET_VALID);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius default shared secret valid failed");
            }
#endif
        }
        else
        {
            value = TRUE;
            GLB_SET_FLAG_COND(value, dot1x_radius.radius_config, AUTH_RADIUS_CONFIG_DEFAULT_KEY);
            rc = tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_KEY);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius default shared secret failed");
            }

            if (p_db_glb->radius_default_shared_secret_valid)
            {
                
                if ('\0' != p_db_glb->radius_default_shared_secret[0])
                {
                    sal_memcpy(dot1x_radius.shared_secret, p_db_glb->radius_default_shared_secret, 
                        sizeof(p_db_glb->radius_default_shared_secret)); 
                    sal_memset(dot1x_radius.shared_enc_secret, 0, PASSWD_SIZE);
                }
                else
                {
                    sal_memcpy(dot1x_radius.shared_enc_secret, p_db_glb->radius_default_shared_enc_secret, 
                        sizeof(p_db_glb->radius_default_shared_enc_secret)); 
                    sal_memset(dot1x_radius.shared_secret, 0, RADIUS_SHARED_SECRET_LEN + 1);
                }
                
            }

#if 0
            dot1x_radius.shared_secret_valid = p_db_glb->radius_default_shared_secret_valid;
            rc = tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_SHARED_SECRET_VALID);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius default shared secret valid failed");
            } 

            rc = tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_SHARED_SECRET);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius default shared secret failed");
            }
#endif
        }

        i++;
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_FUNC_ERR_RET("Unknown field %s", argv[i]);
        }
    }
    
    if (TBL_DOT1X_RADIUS_FLD_MAX_RETRY == field_id)
    {
        i++;
        PM_CFG_GET_UINT(dot1x_radius.max_retry, argv[i], para->p_rs); 

        if (0 == dot1x_radius.max_retry)
        {
            value = TRUE;
            GLB_SET_FLAG_COND(value, dot1x_radius.radius_config, AUTH_RADIUS_CONFIG_DEFAULT_RETRANSMIT);
            rc = tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_RETRANSMIT);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius default retransmit failed");
            }

            if (0 != p_db_glb->radius_default_retry)
            {
                dot1x_radius.max_retry = p_db_glb->radius_default_retry;
            }
            else
            {
                dot1x_radius.max_retry = RADIUS_RETRANSMIT_DEFAULT;
            }
            rc = tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_MAX_RETRY);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius retransmit failed");
            } 
        }
        else
        {
            value = FALSE;
            GLB_SET_FLAG_COND(value, dot1x_radius.radius_config, AUTH_RADIUS_CONFIG_DEFAULT_RETRANSMIT);
            rc = tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_RETRANSMIT);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius default retransmit failed");
            }

            rc = tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_MAX_RETRY);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius retransmit failed");
            } 
        }

        i++;
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_FUNC_ERR_RET("Unknown field %s", argv[i]);
        }
    }

    if (TBL_DOT1X_RADIUS_FLD_TIMEOUT == field_id)
    {
        i++;
        PM_CFG_GET_UINT(dot1x_radius.timeout, argv[i], para->p_rs); 

        if (0 == dot1x_radius.timeout)
        {
            value = TRUE;
            GLB_SET_FLAG_COND(value, dot1x_radius.radius_config, AUTH_RADIUS_CONFIG_DEFAULT_TIMEOUT);
            rc = tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_TIMEOUT);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius default timeout failed");
            }

            if (0 != p_db_glb->radius_default_timeout)
            {
                dot1x_radius.timeout = p_db_glb->radius_default_timeout;
            }
            else
            {
                dot1x_radius.timeout = RADIUS_TIMEOUT_DEFAULT;
            }          
            rc = tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_TIMEOUT);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius timeout failed");
            } 
        }
        else
        {
            value = FALSE;
            GLB_SET_FLAG_COND(value, dot1x_radius.radius_config, AUTH_RADIUS_CONFIG_DEFAULT_TIMEOUT);
            rc = tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_TIMEOUT);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius default timeout failed");
            }

            rc = tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_TIMEOUT);
            if (rc)
            {
                CFG_FUNC_ERR_RET("Set dot1x radius timeout failed");
            } 
        }
        i++;
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_FUNC_ERR_RET("Unknown field %s", argv[i]);
        }
    }
    
    if (TBL_DOT1X_RADIUS_FLD_IS_INBAND == field_id)
    {
        i++;
        is_inband = (0 == sal_strcmp(argv[i], "inband")) ? TRUE : FALSE;
    }
    
    sal_memset(input, 0, sizeof(input));
    for (i = 0; i < 11; i++)
    {
        p_input[i] = (char*)&input[i];    
    }
    
    sprintf(input[0], "%s#radius#%s", argv[1], argv[2]);
    sprintf(input[1], "%s", "port");
    sprintf(input[2], "%s", argv[2]); 
    sprintf(input[3], "%s", "retries");
    sprintf(input[4], "%d", dot1x_radius.max_retry);
    sprintf(input[5], "%s", "timeout");
    sprintf(input[6], "%d", dot1x_radius.timeout);
    sprintf(input[7], "%s", "is_inband");
    sprintf(input[8], "%s", is_inband ? "inband" : "");
    if ('\0' != dot1x_radius.shared_secret[0])
    {
        sprintf(input[9], "%s", "secret");
        sprintf(input[10], "%s", dot1x_radius.shared_secret);
    }
    else if ('\0' != dot1x_radius.shared_enc_secret[0])
    {
        sprintf(input[9], "%s", "enc_secret");
        sprintf(input[10], "%s", dot1x_radius.shared_enc_secret);
    }
    else
    {
        sprintf(input[9], "%s", "secret");
    }
    
    rc = auth_server_dot1x_cmd_process((char**)&p_input[0], 11, para);
    if (rc)
    {
        tbl_dot1x_radius_del_dot1x_radius(&dot1x_radius.key);
        return rc;
    }
    
    dot1x_radius.is_inband = is_inband;
    tbl_dot1x_radius_set_dot1x_radius_field(&dot1x_radius, TBL_DOT1X_RADIUS_FLD_IS_INBAND);

    return rc;
}

static int32
_dot1x_cmd_delete_radius(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    cdb_node_t                *p_node = NULL;
    uint8                     i = 0;
    tbl_dot1x_radius_t        dot1x_radius;
    int32                     field_id = 0;
    cdb_field_t               *p_field = NULL;
    tbl_dot1x_radius_t        *p_db_dot1x_radius = NULL;
    tbl_dot1x_port_t          dot1x_port;
    tbl_dot1x_port_t          *p_db_dot1x_port = NULL;
    tbl_dot1x_port_master_t   *p_glb_port = NULL;
    ctclib_slistnode_t        *listnode = NULL;  
    char                      input[9][128];
    char                      *p_input[9];
    tbl_dot1x_mac_master_t    *p_glb_dot1x_mac = NULL;
    tbl_dot1x_mac_t           *p_db_dot1x_mac = NULL;
    uint32 is_inband = FALSE;
    
    sal_memset(&dot1x_radius, 0, sizeof(tbl_dot1x_radius_t));
    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    
    p_node = cdb_get_tbl(TBL_DOT1X_RADIUS);

    PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
    
    field_id = cdb_get_field_id(argv[i], p_node, &p_field);
    if (field_id < 0)
    {
        CFG_FUNC_ERR_RET("Unknown field %s", argv[i]);
    }

    if (TBL_DOT1X_RADIUS_FLD_KEY == field_id)
    {
        i++;
        rc = dot1x_get_ip_str2val(argv[i], &dot1x_radius.key.addr);
        if (PM_E_NONE != rc)
        {
             CFG_INVALID_PARAM_RET("Please check the ip address of ntp server!");    
        }
        sal_memcpy(dot1x_radius.server_string, argv[i], sal_strlen(argv[i]));

        i++;
        is_inband = (0 == sal_strcmp("inband", argv[i])) ? TRUE : FALSE;
        
        i++;
        PM_CFG_GET_UINT(dot1x_radius.key.sin_port, argv[i], para->p_rs);
       
        if (dot1x_radius_server_check_ip(&dot1x_radius.key.addr) != 0) 
        {
            CFG_INVALID_PARAM_RET("Invalid address!");
        }

        p_db_dot1x_radius = tbl_dot1x_radius_get_dot1x_radius(&dot1x_radius.key);
        if (!p_db_dot1x_radius)
        {
            CFG_FUNC_ERR_RET("RADIUS host does not exist");     
        }
        
        if (is_inband != p_db_dot1x_radius->is_inband)
        {
            CFG_INVALID_PARAM_RET("The mgmt-if is mismatch");
        }
        
        p_glb_dot1x_mac = tbl_dot1x_mac_get_master();     
        if (p_glb_dot1x_mac)
        { 
            CTCLIB_SLIST_LOOP(p_glb_dot1x_mac->dot1x_mac_list, p_db_dot1x_mac, listnode)
            {
                if ((p_db_dot1x_mac->is_auth_bypass)
                    && (p_db_dot1x_mac->current_radius == p_db_dot1x_radius)) 
                {
                    dot1x_port.key.ifindex = p_db_dot1x_mac->key.ifindex;
                    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
                    if (p_db_dot1x_port)
                    {
                        dot1x_mac_access_reject(p_db_dot1x_port, p_db_dot1x_mac);
                    }
                    else
                    {
                        DOT1X_LOG_ERR("802.1x not configured on this interface!");
                    }
                }
            }
        }
        else
        {
            DOT1X_LOG_ERR("_dot1x_cmd_delete_radius fail because tbl_dot1x_mac is not exist");
        }
 
        /* check exist */
        p_glb_port = tbl_dot1x_port_get_master();
    
        if (!p_glb_port)
        {
            CFG_FUNC_ERR_RET("_dot1x_cmd_delete_radius fail because tbl_dot1x_port is not exist");
        }
            
        CTCLIB_SLIST_LOOP(p_glb_port->dot1x_port_list, p_db_dot1x_port, listnode)
        {
            if (p_db_dot1x_port->current_radius == p_db_dot1x_radius)
            {
                if (p_db_dot1x_port->t_wait)
                {
                    ctc_task_delete_timer(p_db_dot1x_port->t_wait);
                    p_db_dot1x_port->t_wait = NULL;
                }

                dot1x_radius_end_association(p_db_dot1x_port);

                dot1x_delete_session(p_db_dot1x_port->key.ifindex);

                p_db_dot1x_port->radius_msg_id = INVALID_RADIUS_MSG_ID;
                rc = tbl_dot1x_port_set_dot1x_port_field(p_db_dot1x_port, TBL_DOT1X_PORT_FLD_RADIUS_MSG_ID);
                if (rc)
                {
                    CFG_FUNC_ERR_RET("Set dot1x port radius_msg_id return %d", rc);
                }

                p_db_dot1x_port->current_radius = NULL;
            }
        }  

        sal_memset(input, 0, 2*128);
        for (i = 0; i < 2; i++)
        {
            p_input[i] = (char*)&input[i];    
        }
        
        sprintf(input[0], "%s#radius#%s", argv[1], argv[3]);
        
        auth_server_dot1x_cmd_process((char**)&p_input[0], 1, para);

        rc = tbl_dot1x_radius_del_dot1x_radius(&dot1x_radius.key);
        if (rc)
        {
            CFG_FUNC_ERR_RET("Delete dot1x radius %s failure", argv[1]);
        } 
    }

    return rc;
}

static int32
_dot1x_cmd_set_radius(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    cdb_node_t                *p_node = NULL;
    uint8                     i = 0;
    int32                     field_id = 0;
    cdb_field_t               *p_field = NULL;  
    tbl_dot1x_radius_t        dot1x_radius;
    tbl_dot1x_radius_t        *p_db_dot1x_radius = NULL;

    sal_memset(&dot1x_radius, 0, sizeof(tbl_dot1x_radius_t));
    
    p_node = cdb_get_tbl(TBL_DOT1X_RADIUS);

    field_id = cdb_get_field_id(argv[i], p_node, &p_field);
    if (field_id < 0)
    {
        CFG_FUNC_ERR_RET("Unknown field %s", argv[i]);
    }

    if (TBL_DOT1X_RADIUS_FLD_REACTIVATE == field_id)
    {
        i++;
        if ((0 == sal_strcmp(argv[i], "all")) || (0 == sal_strcmp(argv[i], "")))
        {
            dot1x_port_clear_dead_radius();      
        }
        else if (0 == sal_strcmp(argv[i], "host"))
        {
            i++;
            rc = dot1x_get_ip_str2val(argv[i], &dot1x_radius.key.addr);
            if (PM_E_NONE != rc)
            {
                 CFG_INVALID_PARAM_RET("Invalid address!");    
            }
    
            i++;
            PM_CFG_GET_UINT(dot1x_radius.key.sin_port, argv[i], para->p_rs);             
            
            p_db_dot1x_radius = tbl_dot1x_radius_get_dot1x_radius(&dot1x_radius.key);
            if (!p_db_dot1x_radius)
            {
                CFG_FUNC_ERR_RET("Radius server %s:%d is not exist!", argv[i-1], dot1x_radius.key.sin_port);
            }
            else
            {
                dot1x_port_active_radius_server(p_db_dot1x_radius);  
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Re-activate radius server %s failure", argv[i]);
        }
    }

    return rc;
}

int32
dot1x_cmd_parse_radius_field(char **argv, int32 argc, cdb_node_t* p_node, field_parse_t *field, uint32 with_key)
{
    cdb_tbl_info_t    *p_tbl_info = p_node->tbl_info;
    uint32            field_id = FLD_ID_ALL;
    uint32            key_id = 0;

    if (with_key)
    {
        key_id = 2;
    }
    
    sal_memset(field, FLD_ID_ALL, sizeof(field_parse_t));
    if (key_id >= argc)
    {
        return PM_E_NONE;
    }
    
    for (field_id = 0; field_id < p_tbl_info->field_num; field_id++)
    {
        if (!sal_strcmp(argv[key_id], p_tbl_info->field[field_id].name))
        {
            field->field_id[0] = field_id;
            return PM_E_NONE;
        }
    }
    
    return PM_E_FAIL;
}


int32
_dot1x_cmd_get_radius(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_dot1x_radius_master_t   *p_glb_radius = NULL;
    tbl_dot1x_radius_t          *p_db_dot1x_radius = NULL;
    ctclib_slistnode_t          *listnode = NULL;  
    FILE                        *fp = NULL;
    tbl_iter_args_t             args;     
    int32                       field_id = 0;
    field_parse_t               field;
    cdb_node_t                  *p_node = NULL;
    tbl_dot1x_radius_t          dot1x_radius;
    int32                       rc = PM_E_NONE;

    sal_memset(&args, 0, sizeof(args));
    sal_memset(&dot1x_radius, 0, sizeof(tbl_dot1x_radius_t));

    /* check exist */
    p_glb_radius = tbl_dot1x_radius_get_master();

    if (!p_glb_radius)
    {
        CFG_FUNC_ERR_RET("_dot1x_cmd_set_radius_default_retry fail because tbl_dot1x_radius is not exist");
    }

    args.argv[0] = &field;

    p_node = cdb_get_tbl(TBL_DOT1X_RADIUS);
    
    field_id = dot1x_cmd_parse_radius_field(argv, argc, p_node, &field, TRUE);
    if (field_id < 0)
    {
        CFG_INVALID_PARAM_RET("Invalid field");
    }

    if (0 == argc)
    { 
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        args.argv[1] = fp;

        CTCLIB_SLIST_LOOP(p_glb_radius->radius_list, p_db_dot1x_radius, listnode)
        {
            tbl_dot1x_radius_dump_one(p_db_dot1x_radius, &args);
        }

        sal_fclose(fp);
    }
    else if (argc >= 2)
    {
        CFG_PARSER_IP_ADDR(argv[0], &dot1x_radius.key.addr.u.prefix4, para->p_rs);
        PM_CFG_GET_UINT(dot1x_radius.key.sin_port, argv[1], para->p_rs);
        dot1x_radius.key.addr.family = AF_INET;
        
        p_db_dot1x_radius = tbl_dot1x_radius_get_dot1x_radius(&dot1x_radius.key);
        if (!p_db_dot1x_radius)
        {
            CFG_FUNC_ERR_RET("Radius server %s:%d is not exist!", argv[0], dot1x_radius.key.sin_port);
        }
        
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        args.argv[1] = fp;

        tbl_dot1x_radius_dump_one(p_db_dot1x_radius, &args);

        sal_fclose(fp);
    }

    return PM_E_NONE;
}

int32
dot1x_cmd_process_radius(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                   rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (argc < 11)
        {
            CFG_INVALID_PARAM_RET("Invalid action");
        }
        rc = _dot1x_cmd_add_radius(argv, argc, para);
        break;

    case CDB_OPER_SET:
        rc = _dot1x_cmd_set_radius(argv, argc, para);
        break;
        break;

    case CDB_OPER_DEL:
        rc = _dot1x_cmd_delete_radius(argv, argc, para);
        break;

    case CDB_OPER_GET:
        rc = _dot1x_cmd_get_radius(argv, argc, para);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}


int32 
dot1x_cmd_show_port_radius_server(tbl_dot1x_port_t  *p_db_dot1x_port, cfg_cmd_para_t *para, FILE                     *fp)
{
    int32                    rc = PM_E_NONE;
    tbl_dot1x_radius_t       *p_db_dot1x_radius = NULL;

    sal_fprintf(fp, "=====================================\n");
    sal_fprintf(fp, "802.1X session on interface %s:\n", get_intf_print_name(p_db_dot1x_port->name));
    sal_fprintf(fp, "current radius server:\n");

    if (p_db_dot1x_port->current_radius)
    {
        p_db_dot1x_radius = (tbl_dot1x_radius_t*)p_db_dot1x_port->current_radius;
        sal_fprintf(fp, " retransmit count  : %d\n", p_db_dot1x_port->retry);
        sal_fprintf(fp, " server address    : %s:%d\n", 
            p_db_dot1x_radius->server_string, p_db_dot1x_radius->key.sin_port); 
    }
    else
    {
        sal_fprintf(fp, " N/A\n");    
    }

    return rc;
}

int32
dot1x_cmd_show_radius_server(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                    rc = PM_E_NONE;    
    tbl_interface_t          intf;
    tbl_interface_t          *p_db_if = NULL;
    tbl_dot1x_port_t         dot1x_port;
    tbl_dot1x_port_t         *p_db_dot1x_port = NULL;
    tbl_dot1x_port_master_t  *p_glb_dot1x_port = NULL;
    ctclib_slistnode_t       *listnode = NULL;
    FILE                     *fp = NULL;
    uint8                    dead_radius_count = 0;
    tbl_auth_server_master_t *p_master = tbl_auth_server_get_master();
    tbl_auth_server_t        *pItem = NULL;
    time_t                    tCurrent = time(NULL);
    char                     buf[CMD_BUF_256];

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "=====================================\n");
    sal_fprintf(fp, "radius servers in dead list:\n");
    
    CTCLIB_SLIST_LOOP(p_master->auth_servers, pItem, listnode) 
    {
        if (pItem->dead && (AUTH_SERVER_TYPE_RADIUS == pItem->key.type))
        {
            sal_fprintf(fp, " server address    : %s:%d\n", 
                cdb_addr_val2str(buf, CMD_BUF_256, &pItem->key.addr),
                pItem->key.sin_port); 

            sal_fprintf(fp, " dead timer        : %ld\n", (pItem->reawake_time - tCurrent));   

            dead_radius_count++;
        }
    }
    
    if(0 == dead_radius_count)
    {
        sal_fprintf(fp, " N/A\n");
    }

    if ((argc > 0) && (0 == sal_memcmp(argv[0], "interface", sal_strlen("interface"))))
    {
        sal_memset(&intf, 0, sizeof(tbl_interface_t));    
        sal_strncpy(intf.key.name, argv[1], IFNAME_SIZE);
    
        p_db_if = tbl_interface_get_interface(&intf.key);
        if (!p_db_if)
        {
            CFG_FUNC_ERR_RET("Invalid interface or interface does not exist! must be access switch port or routed port!");
        }

        dot1x_port.key.ifindex = p_db_if->ifindex;
        p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
        if (!p_db_dot1x_port)
        {
            CFG_FUNC_ERR_RET("802.1x not configured on %s", argv[1]);
        }

        dot1x_cmd_show_port_radius_server(p_db_dot1x_port, para, fp);
    }
    else
    {
        /* check exist */
        p_glb_dot1x_port = tbl_dot1x_port_get_master();
    
        if (!p_glb_dot1x_port)
        {
            CFG_FUNC_ERR_RET("dot1x_cmd_show_radius_server fail because tbl_dot1x_port is not exist");
        }
    
        CTCLIB_SLIST_LOOP(p_glb_dot1x_port->dot1x_port_list, p_db_dot1x_port, listnode)
        {
            dot1x_cmd_show_port_radius_server(p_db_dot1x_port, para, fp);
        }
    }
    
    sal_fprintf(fp, "=====================================\n");
    sal_fclose(fp);
    fp = NULL;

    return rc;
}

void
dot1x_show_system (cfg_cmd_para_t *para, FILE *fp)
{
    
    tbl_dot1x_global_t         *p_db_glb = NULL;
    tbl_dot1x_radius_master_t  *p_glb_radius = NULL;
    tbl_dot1x_radius_t         *p_db_dot1x_radius = NULL;
    ctclib_slistnode_t         *listnode = NULL;

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        return;
    }

    sal_fprintf(fp, "802.1X Port-Based Authentication %s\n", 
        p_db_glb->global_auth_enable ? "Enabled" : "Disabled");
        
    /* check exist */
    p_glb_radius = tbl_dot1x_radius_get_master();
    if (!p_glb_radius)
    {
        sal_fclose(fp);
        fp = NULL;
        return;
    }

    CTCLIB_SLIST_LOOP(p_glb_radius->radius_list, p_db_dot1x_radius, listnode)
    {
        sal_fprintf(fp, "  RADIUS server address: %s:%d\n", 
            p_db_dot1x_radius->server_string, p_db_dot1x_radius->key.sin_port);

        sal_fprintf(fp, "  Next radius message ID: %d\n", p_db_dot1x_radius->identifier);
    }
    
    return; 
}

void 
dot1x_show_port_details(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_dot1x_global_t *p_db_glb,
    tbl_dot1x_mac_master_t  *p_glb_dot1x_mac, cfg_cmd_para_t *para, FILE  *fp)
{  
    tbl_dot1x_mac_t           *p_db_dot1x_mac = NULL;
    ctclib_slistnode_t        *listnode = NULL;         
  
    sal_fprintf(fp, "802.1X info for interface %s\n", get_intf_print_name(p_db_dot1x_port->name));
    
    if (AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
    {
        CTCLIB_SLIST_LOOP(p_glb_dot1x_mac->dot1x_mac_list, p_db_dot1x_mac, listnode)
        {
            if (p_db_dot1x_mac->key.ifindex == p_db_dot1x_port->key.ifindex)
            {
                if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT))
                {
                    sal_fprintf(fp, "  Supplicant name:");

                    if (p_db_dot1x_mac->is_auth_bypass)
                    {
                        sal_fprintf(fp, "  %s ", "bypass user");  
                    }
                    else
                    {
                        sal_fprintf(fp, "  %s ", p_db_dot1x_mac->user_name);   
                    }

                    if (sal_memcmp(p_db_dot1x_mac->key.mac, p_db_glb->eapol_group_address, sizeof(mac_addr_t)))
                    {
                        sal_fprintf(fp, "  (%.02x%.02x.%.02x%.02x.%.02x%.02x)",
                            p_db_dot1x_mac->key.mac[0], p_db_dot1x_mac->key.mac[1],
                            p_db_dot1x_mac->key.mac[2], p_db_dot1x_mac->key.mac[3],
                            p_db_dot1x_mac->key.mac[4], p_db_dot1x_mac->key.mac[5]);    
                    }

                    sal_fprintf(fp, "\n");
                }
            }
        }
    }
    else
    {
        if (p_db_dot1x_port->user_name[0] != '\0')
        {
            sal_fprintf(fp, "  Supplicant name: %s\n", p_db_dot1x_port->user_name);
        }

        if (sal_memcmp(p_db_dot1x_port->dest_mac, p_db_glb->eapol_group_address, sizeof(mac_addr_t)))
        {
            sal_fprintf(fp, "  Supplicant address: %.02x%.02x.%.02x%.02x.%.02x%.02x\n",
                p_db_dot1x_port->dest_mac[0], p_db_dot1x_port->dest_mac[1],
                p_db_dot1x_port->dest_mac[2], p_db_dot1x_port->dest_mac[3],
                p_db_dot1x_port->dest_mac[4], p_db_dot1x_port->dest_mac[5]);
        }
    }

    sal_fprintf(fp, "  portEnabled: %s - portControl: %s\n",
        p_db_dot1x_port->portEnabled ? "true" : "false",    
        auth_port_ctrl_msg[p_db_dot1x_port->portControl]);

    if (AUTH_PORT_MODE_PORT == p_db_dot1x_port->auth_mode)
    {
        sal_fprintf(fp, "  portStatus: %s - currentId: %d\n",
            auth_port_status_msg[p_db_dot1x_port->portStatus], p_db_dot1x_port->currentId);
    }

    sal_fprintf(fp, "  reAuthenticate: %s\n", 
        p_db_dot1x_port->reAuthEnabled ? "enabled" : "disabled");

    sal_fprintf(fp, "  reAuthPeriod: %u\n", p_db_dot1x_port->reAuthPeriod);

    if (p_db_dot1x_port->guest_vlan && (p_db_dot1x_port->auth_mode == AUTH_PORT_MODE_PORT))
    {
        sal_fprintf(fp, "  Guest VLAN:%d%s", p_db_dot1x_port->guest_vlan,
            p_db_dot1x_port->guest_vlan_valid? ("(Port Authorized by guest vlan)\n") :("\n"));     
    }

    sal_fprintf(fp, "  Auth mode        :  %s\n",
         AUTH_PORT_MODE_PORT == p_db_dot1x_port->auth_mode? "Port based":"mac based");

    if (AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
    {
        sal_fprintf(fp, "  Mac Auth bypass  :  %s\n",
            p_db_dot1x_port->mac_auth_bypass? "enabled":"disabled");
        sal_fprintf(fp, "  Max mac user     :  %d\n", p_db_dot1x_port->max_user ? 
            p_db_dot1x_port->max_user : p_db_glb->mac_user_count_max);
        sal_fprintf(fp, "  Current mac user :  %d\n", p_db_dot1x_port->current_user);
    }

    if (AUTH_PORT_MODE_PORT == p_db_dot1x_port->auth_mode)
   {
        if (p_db_dot1x_port->portEnabled)
        {
            sal_fprintf(fp, "  abort:%c fail:%c start:%c timeout:%c success:%c\n",
                p_db_dot1x_port->authAbort ? 'T' : 'F',
                p_db_dot1x_port->authFail ? 'T' : 'F',
                p_db_dot1x_port->authStart ? 'T' : 'F',
                p_db_dot1x_port->authTimeout ? 'T' : 'F',
                p_db_dot1x_port->authSuccess ? 'T' : 'F');

            sal_fprintf(fp, "  PAE: state: %s - portMode: %s\n",
                auth_pae_state_msg[p_db_dot1x_port->auth_pae_state],
                auth_port_ctrl_msg[p_db_dot1x_port->auth_pae_portMode]);

            sal_fprintf(fp, "  PAE: reAuthCount: %d - rxRespId: %d\n",
                p_db_dot1x_port->auth_pae_reAuthCount, p_db_dot1x_port->auth_pae_rxRespId);

            sal_fprintf(fp, "  PAE: quietPeriod: %d - reqMax: %d - txPeriod: %d\n",
                p_db_dot1x_port->auth_pae_quietPeriod, p_db_dot1x_port->auth_pae_reAuthMax, 
                p_db_dot1x_port->auth_pae_txPeriod);

            sal_fprintf(fp, "  BE: state: %s - reqCount: %d - idFromServer: %d\n",
                auth_be_state_msg[p_db_dot1x_port->auth_be_state],
                p_db_dot1x_port->auth_be_reqCount, p_db_dot1x_port->auth_be_idFromServer);

            sal_fprintf(fp, "  BE: suppTimeout: %d - serverTimeout: %d\n", 
                p_db_dot1x_port->auth_be_suppTimeout, p_db_dot1x_port->auth_be_serverTimeout);
        }

        sal_fprintf(fp, "  CD: adminControlledDirections: %s - operControlledDirections: %s\n",
            p_db_dot1x_port->auth_ctrl_dir_adminControlledDirections ? "in" : "both",
            p_db_dot1x_port->auth_ctrl_dir_operControlledDirections ? "in" : "both");

        sal_fprintf(fp, "  CD: bridgeDetected: %s\n",
            p_db_dot1x_port->auth_ctrl_dir_bridgeDetected ? "true" : "false");
    }

    return; 
}

int32
dot1x_show_all(cfg_cmd_para_t *para, FILE  *fp)
{
    tbl_dot1x_port_master_t   *p_glb_port = NULL;
    tbl_dot1x_port_t          *p_db_dot1x_port = NULL;
    ctclib_slistnode_t        *listnode = NULL;  
    tbl_dot1x_mac_master_t    *p_glb_dot1x_mac = NULL;
    tbl_dot1x_global_t        *p_db_glb = NULL;

    /* check exist */
    p_glb_port = tbl_dot1x_port_get_master();

    if (!p_glb_port)
    {
        CFG_FUNC_ERR_RET("dot1x_show_all fail because tbl_dot1x_port is not exist");
    }

    p_glb_dot1x_mac = tbl_dot1x_mac_get_master();

    if (!p_glb_dot1x_mac)
    {
        CFG_FUNC_ERR_RET("dot1x_show_all fail because tbl_dot1x_mac is not exist");
    }

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("dot1x_show_all fail because tbl_dot1x_global is not exist");
    }
        
    CTCLIB_SLIST_LOOP(p_glb_port->dot1x_port_list, p_db_dot1x_port, listnode)
    {
        dot1x_show_port_details(p_db_dot1x_port, p_db_glb, p_glb_dot1x_mac, para, fp);
    }
    
    return PM_E_NONE; 
}

int32 dot1x_show_interface(char *name, cfg_cmd_para_t *para, FILE  *fp)
{
    int32                   rc = PM_E_NONE;
    tbl_dot1x_port_t        dot1x_port;
    tbl_dot1x_port_t        *p_db_dot1x_port = NULL;
    tbl_interface_t         intf;
    tbl_interface_t         *p_db_if = NULL;
    tbl_dot1x_mac_master_t  *p_glb_dot1x_mac = NULL;
    tbl_dot1x_global_t      *p_db_glb = NULL;

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    sal_memset(&intf, 0, sizeof(tbl_interface_t));    

    sal_strncpy(intf.key.name, name, IFNAME_SIZE);
    
    p_db_if = tbl_interface_get_interface(&intf.key);
    if (!p_db_if)
    {
        CFG_FUNC_ERR_RET("Invalid interface: %s", name);
    }

    sal_strncpy(dot1x_port.name, name, IFNAME_SIZE);
    rc = dot1x_port_check_dot1x_interface(&dot1x_port, para); 
    if (rc)
    {
        return rc;
    }
    
    dot1x_port.key.ifindex = p_db_if->ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        CFG_FUNC_ERR_RET("802.1x not configured on this interface!");
    }
    
    p_glb_dot1x_mac = tbl_dot1x_mac_get_master();

    if (!p_glb_dot1x_mac)
    {
        CFG_FUNC_ERR_RET("dot1x_show_interface fail because tbl_dot1x_mac is not exist");
    }

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("dot1x_show_interface fail because tbl_dot1x_global is not exist");
    }

    dot1x_show_port_details(p_db_dot1x_port, p_db_glb, p_glb_dot1x_mac, para, fp);
        
    return rc;
}

void 
dot1x_show_port_stats(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_dot1x_global_t *p_db_glb,
    tbl_dot1x_mac_master_t  *p_glb_dot1x_mac, cfg_cmd_para_t *para, FILE  *fp)
{  
    sal_fprintf(fp, "802.1X statistics for interface %s\n", get_intf_print_name(p_db_dot1x_port->name));

    sal_fprintf(fp, "  EAPOL Frames Rx: %u - EAPOL Frames Tx: %u\n",
        p_db_dot1x_port->eapol_frames_rx, p_db_dot1x_port->eapol_frames_tx);

    sal_fprintf(fp, "  EAPOL Start Frames Rx: %u - EAPOL Logoff Frames Rx: %u\n",
        p_db_dot1x_port->eapol_start_frames_rx, p_db_dot1x_port->eapol_logoff_frames_rx);

    sal_fprintf(fp, "  EAP Rsp/Id Frames Rx: %u - EAP Response Frames Rx: %u\n",
        p_db_dot1x_port->eap_respid_frames_rx, p_db_dot1x_port->eap_resp_frames_rx);
    
    sal_fprintf(fp, "  EAP Rsp/Id Frames Rx: %u - EAP Response Frames Rx: %u\n",
        p_db_dot1x_port->eap_respid_frames_rx, p_db_dot1x_port->eap_resp_frames_rx);

    sal_fprintf(fp, "  EAP Req/Id Frames Tx: %u - EAP Request Frames Tx: %u\n",
        p_db_dot1x_port->eap_reqid_frames_tx, p_db_dot1x_port->eap_req_frames_tx);

    sal_fprintf(fp, "  Invalid EAPOL Frames Rx: %u "
        "- EAP Length Error Frames Rx: %u\n",
        p_db_dot1x_port->eapol_invalid_frames_rx,
        p_db_dot1x_port->eap_len_error_frames_rx);

    sal_fprintf(fp, "  EAPOL Last Frame Version Rx: %u - "
        "EAPOL Last Frame Src: %02x%02x.%02x%02x.%02x%02x\n",
        p_db_dot1x_port->eapol_last_frame_version, 
        p_db_dot1x_port->eapol_last_frame_source[0],
        p_db_dot1x_port->eapol_last_frame_source[1],
        p_db_dot1x_port->eapol_last_frame_source[2],
        p_db_dot1x_port->eapol_last_frame_source[3],
        p_db_dot1x_port->eapol_last_frame_source[4],
        p_db_dot1x_port->eapol_last_frame_source[5]);
    
    return;
}

int32
dot1x_show_all_stats(cfg_cmd_para_t *para)
{
    tbl_dot1x_port_master_t   *p_glb_port = NULL;
    tbl_dot1x_port_t          *p_db_dot1x_port = NULL;
    ctclib_slistnode_t        *listnode = NULL;  
    tbl_dot1x_mac_master_t    *p_glb_dot1x_mac = NULL;
    tbl_dot1x_global_t        *p_db_glb = NULL;
    FILE                      *fp = NULL; 
    
    /* check exist */
    p_glb_port = tbl_dot1x_port_get_master();

    if (!p_glb_port)
    {
        CFG_FUNC_ERR_RET("dot1x_show_all_stats fail because tbl_dot1x_port is not exist");
    }

    p_glb_dot1x_mac = tbl_dot1x_mac_get_master();

    if (!p_glb_dot1x_mac)
    {
        CFG_FUNC_ERR_RET("dot1x_show_all_stats fail because tbl_dot1x_mac is not exist");
    }

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("dot1x_show_all_stats fail because tbl_dot1x_global is not exist");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
        
    CTCLIB_SLIST_LOOP(p_glb_port->dot1x_port_list, p_db_dot1x_port, listnode)
    {
        dot1x_show_port_stats(p_db_dot1x_port, p_db_glb, p_glb_dot1x_mac, para, fp);
    }
    
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE; 
}

int32 dot1x_show_interface_stats(char *name, cfg_cmd_para_t *para)
{
    int32                   rc = PM_E_NONE;
    tbl_dot1x_port_t        dot1x_port;
    tbl_dot1x_port_t        *p_db_dot1x_port = NULL;
    tbl_interface_t         intf;
    tbl_interface_t         *p_db_if = NULL;
    tbl_dot1x_mac_master_t  *p_glb_dot1x_mac = NULL;
    tbl_dot1x_global_t      *p_db_glb = NULL;
    FILE                    *fp = NULL; 

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    sal_memset(&intf, 0, sizeof(tbl_interface_t));    

    sal_strncpy(intf.key.name, name, IFNAME_SIZE);
    
    p_db_if = tbl_interface_get_interface(&intf.key);
    if (!p_db_if)
    {
        CFG_FUNC_ERR_RET("Invalid interface: %s", name);
    }

    sal_strncpy(dot1x_port.name, name, IFNAME_SIZE);
    rc = dot1x_port_check_dot1x_interface(&dot1x_port, para); 
    if (rc)
    {
        return rc;
    }
    
    dot1x_port.key.ifindex = p_db_if->ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        CFG_FUNC_ERR_RET("802.1x not configured on this interface!");
    }
    
    p_glb_dot1x_mac = tbl_dot1x_mac_get_master();

    if (!p_glb_dot1x_mac)
    {
        CFG_FUNC_ERR_RET("dot1x_show_interface_status fail because tbl_dot1x_mac is not exist");
    }

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("dot1x_show_interface_status fail because tbl_dot1x_global is not exist");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    dot1x_show_port_stats(p_db_dot1x_port, p_db_glb, p_glb_dot1x_mac, para, fp);

    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

void 
dot1x_show_port_session_stats(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_dot1x_global_t *p_db_glb,
    tbl_dot1x_mac_master_t  *p_glb_dot1x_mac, cfg_cmd_para_t *para, FILE *fp)
{  
    sal_fprintf(fp, "802.1X session statistics for interface %s\n", get_intf_print_name(p_db_dot1x_port->name));

    sal_fprintf(fp, "session authentication method: %s\n", (p_db_dot1x_port->session_authentication_method
        ? "Remote server" : "Local server"));
   
    sal_fprintf(fp, "session time: %d secs \n", p_db_dot1x_port->session_time);

    sal_fprintf(fp, "session user name: %s\n", p_db_dot1x_port->user_name);

    sal_fprintf(fp, "session terminate cause: %s\n",
        auth_sesssion_terminate_cause[p_db_dot1x_port->session_terminate_cause]);

    return;
}

int32
dot1x_show_all_session_stats(cfg_cmd_para_t *para)
{
    tbl_dot1x_port_master_t   *p_glb_port = NULL;
    tbl_dot1x_port_t          *p_db_dot1x_port = NULL;
    ctclib_slistnode_t        *listnode = NULL;  
    tbl_dot1x_mac_master_t    *p_glb_dot1x_mac = NULL;
    tbl_dot1x_global_t        *p_db_glb = NULL;
    FILE                      *fp = NULL; 
    
    /* check exist */
    p_glb_port = tbl_dot1x_port_get_master();

    if (!p_glb_port)
    {
        CFG_FUNC_ERR_RET("dot1x_show_all_session_stats fail because tbl_dot1x_port is not exist");
    }

    p_glb_dot1x_mac = tbl_dot1x_mac_get_master();

    if (!p_glb_dot1x_mac)
    {
        CFG_FUNC_ERR_RET("dot1x_show_all_session_stats fail because tbl_dot1x_mac is not exist");
    }

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("dot1x_show_all_session_stats fail because tbl_dot1x_global is not exist");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
        
    CTCLIB_SLIST_LOOP(p_glb_port->dot1x_port_list, p_db_dot1x_port, listnode)
    {
        dot1x_show_port_session_stats(p_db_dot1x_port, p_db_glb, p_glb_dot1x_mac, para, fp);
    }
    
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE; 
}

int32 dot1x_show_interface_session_stats(char *name, cfg_cmd_para_t *para)
{
    int32                   rc = PM_E_NONE;
    tbl_dot1x_port_t        dot1x_port;
    tbl_dot1x_port_t        *p_db_dot1x_port = NULL;
    tbl_interface_t         intf;
    tbl_interface_t         *p_db_if = NULL;
    tbl_dot1x_mac_master_t  *p_glb_dot1x_mac = NULL;
    tbl_dot1x_global_t      *p_db_glb = NULL;
    FILE                    *fp = NULL; 

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    sal_memset(&intf, 0, sizeof(tbl_interface_t));    

    sal_strncpy(intf.key.name, name, IFNAME_SIZE);
    
    p_db_if = tbl_interface_get_interface(&intf.key);
    if (!p_db_if)
    {
        CFG_FUNC_ERR_RET("Invalid interface: %s", name);
    }

    sal_strncpy(dot1x_port.name, name, IFNAME_SIZE);
    rc = dot1x_port_check_dot1x_interface(&dot1x_port, para); 
    if (rc)
    {
        return rc;
    }
    
    dot1x_port.key.ifindex = p_db_if->ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        CFG_FUNC_ERR_RET("802.1x not configured on this interface!");
    }
    
    p_glb_dot1x_mac = tbl_dot1x_mac_get_master();

    if (!p_glb_dot1x_mac)
    {
        CFG_FUNC_ERR_RET("dot1x_show_interface_session_stats fail because tbl_dot1x_mac is not exist");
    }

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("dot1x_show_interface_session_stats fail because tbl_dot1x_global is not exist");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    dot1x_show_port_session_stats(p_db_dot1x_port, p_db_glb, p_glb_dot1x_mac, para, fp);

    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

void 
dot1x_show_port_diagnostics(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_dot1x_global_t *p_db_glb,
    tbl_dot1x_mac_master_t  *p_glb_dot1x_mac, cfg_cmd_para_t *para, FILE *fp)
{    
    sal_fprintf(fp, "802.1X Diagnostics for interface %s\n", get_intf_print_name(p_db_dot1x_port->name));

    sal_fprintf(fp, " authEnterConnecting: %d\n", p_db_dot1x_port->auth_pae_enter_connecting);

    sal_fprintf(fp, " authEaplogoffWhileConnecting: %d\n", p_db_dot1x_port->auth_pae_eaplogoff_while_connecting);

    sal_fprintf(fp, " authEnterAuthenticating: %d\n", p_db_dot1x_port->auth_pae_enter_authenticating);

    sal_fprintf(fp, " authSuccessWhileAuthenticating: %d\n", p_db_dot1x_port->auth_pae_success_while_authenticating);

    sal_fprintf(fp, " authTimeoutWhileAuthenticating: %d\n", p_db_dot1x_port->auth_pae_timeout_while_authenticating);

    sal_fprintf(fp, " authFailWhileAuthenticating: %d\n", p_db_dot1x_port->auth_pae_fail_while_authenticating);

    sal_fprintf(fp, " authEapstartWhileAuthenticating: %d\n", p_db_dot1x_port->auth_pae_eapstart_while_authenticating);

    sal_fprintf(fp, " authEaplogoggWhileAuthenticating: %d\n", 
        p_db_dot1x_port->auth_pae_eaplogoff_while_authenticating);

    sal_fprintf(fp, " authAuthReauthsWhileAuthenticating: %d\n", 
        p_db_dot1x_port->auth_pae_reauths_while_authenticating);

    sal_fprintf(fp, " authReauthsWhileAuthenticated: %d\n", 
        p_db_dot1x_port->auth_pae_reauths_while_authenticated);

    sal_fprintf(fp, " authEapstartWhileAuthenticated: %d\n", 
        p_db_dot1x_port->auth_pae_eapstart_while_authenticated);

    sal_fprintf(fp, " authEaplogoffWhileAuthenticated: %d\n", 
        p_db_dot1x_port->auth_pae_eaplogoff_while_authenticated);

    sal_fprintf(fp, " BackendResponses: %d\n", 
        p_db_dot1x_port->auth_be_backend_responses);
    
    sal_fprintf(fp, " BackendAccessChallenges: %d\n", 
        p_db_dot1x_port->auth_be_backend_access_challenges);

    sal_fprintf(fp, " BackendOtherrequestToSupplicant: %d\n", 
        p_db_dot1x_port->auth_be_backend_otherrequest_to_supplicant);

    sal_fprintf(fp, " BackendAuthSuccess: %d\n", 
        p_db_dot1x_port->auth_be_backend_auth_success);

    sal_fprintf(fp, " BackendAuthFails: %d\n", 
        p_db_dot1x_port->auth_be_backend_auth_fails);

    sal_fprintf(fp, " BackendNonNakResponsesFromSupplicant: %d\n", 
        p_db_dot1x_port->auth_be_backend_nonnak_responses_from_supplicant);

    return;
}

int32
dot1x_show_all_diagnostics(cfg_cmd_para_t *para)
{
    tbl_dot1x_port_master_t   *p_glb_port = NULL;
    tbl_dot1x_port_t          *p_db_dot1x_port = NULL;
    ctclib_slistnode_t        *listnode = NULL;  
    tbl_dot1x_mac_master_t    *p_glb_dot1x_mac = NULL;
    tbl_dot1x_global_t        *p_db_glb = NULL;
    FILE                      *fp = NULL; 

    /* check exist */
    p_glb_port = tbl_dot1x_port_get_master();

    if (!p_glb_port)
    {
        CFG_FUNC_ERR_RET("dot1x_show_all_diagnostics fail because tbl_dot1x_port is not exist");
    }

    p_glb_dot1x_mac = tbl_dot1x_mac_get_master();

    if (!p_glb_dot1x_mac)
    {
        CFG_FUNC_ERR_RET("dot1x_show_all_diagnostics fail because tbl_dot1x_mac is not exist");
    }

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("dot1x_show_all_diagnostics fail because tbl_dot1x_global is not exist");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
        
    CTCLIB_SLIST_LOOP(p_glb_port->dot1x_port_list, p_db_dot1x_port, listnode)
    {
        dot1x_show_port_diagnostics(p_db_dot1x_port, p_db_glb, p_glb_dot1x_mac, para, fp);
    }
    
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE; 
}

int32 dot1x_show_interface_diagnostics(char *name, cfg_cmd_para_t *para)
{
    int32                   rc = PM_E_NONE;
    tbl_dot1x_port_t        dot1x_port;
    tbl_dot1x_port_t        *p_db_dot1x_port = NULL;
    tbl_interface_t         intf;
    tbl_interface_t         *p_db_if = NULL;
    tbl_dot1x_mac_master_t  *p_glb_dot1x_mac = NULL;
    tbl_dot1x_global_t      *p_db_glb = NULL;
    FILE                    *fp = NULL;

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    sal_memset(&intf, 0, sizeof(tbl_interface_t));    

    sal_strncpy(intf.key.name, name, IFNAME_SIZE);
    
    p_db_if = tbl_interface_get_interface(&intf.key);
    if (!p_db_if)
    {
        CFG_FUNC_ERR_RET("Invalid interface: %s", name);
    }

    sal_strncpy(dot1x_port.name, name, IFNAME_SIZE);
    rc = dot1x_port_check_dot1x_interface(&dot1x_port, para); 
    if (rc)
    {
        return rc;
    }
    
    dot1x_port.key.ifindex = p_db_if->ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        CFG_FUNC_ERR_RET("802.1x not configured on this interface!");
    }
    
    p_glb_dot1x_mac = tbl_dot1x_mac_get_master();

    if (!p_glb_dot1x_mac)
    {
        CFG_FUNC_ERR_RET("dot1x_show_interface_diagnostics fail because tbl_dot1x_mac is not exist");
    }

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("dot1x_show_interface_diagnostics fail because tbl_dot1x_global is not exist");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    dot1x_show_port_diagnostics(p_db_dot1x_port, p_db_glb, p_glb_dot1x_mac, para, fp);

    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

void 
dot1x_show_port_mac(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_dot1x_global_t *p_db_glb,
    tbl_dot1x_mac_master_t  *p_glb_dot1x_mac, cfg_cmd_para_t *para, FILE  *fp)
{  
    tbl_dot1x_mac_t           *p_db_dot1x_mac = NULL;
    ctclib_slistnode_t        *listnode = NULL; 
    char                      guest_vlan_str[20];
    uint32                    timer = 0;

    sal_memset(&guest_vlan_str, 0, 20);

    CTCLIB_SLIST_LOOP(p_glb_dot1x_mac->dot1x_mac_list, p_db_dot1x_mac, listnode)
    {
        if (p_db_dot1x_port->key.ifindex != p_db_dot1x_mac->key.ifindex)
        {
            continue;    
        }

        /* modified by liwh for bug 38872, 2016-06-06 */
        timer = 0;
        /* liwh end */
        
        if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_EAPOL_REAUTH))
        {
            if (p_db_dot1x_mac->t_reauth)
            {
                timer = ctc_timer_get_remain_sec(p_db_dot1x_mac->t_reauth); 
            }
        }
        else if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT))
        {
            if (p_db_dot1x_mac->t_reject)
            {
                timer = ctc_timer_get_remain_sec(p_db_dot1x_mac->t_reject); 
            }
            else
            {
                DOT1X_LOG_ERR("dot1x_show_port_mac p_db_dot1x_mac->t_reject is null\n");
            }
        }
        else if(GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT) 
            && GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_BYPASS))
        {
            /*accept && by pass*/
            if (p_db_dot1x_mac->t_reauth)
            {
                timer = ctc_timer_get_remain_sec(p_db_dot1x_mac->t_reauth);
            }
        }
        else if(GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT) 
            && GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_BYPASS))
        {
            /*accept && by pass*/
            if (p_db_dot1x_mac->t_reauth)
            {
                timer = ctc_timer_get_remain_sec(p_db_dot1x_mac->t_reauth);
            }
        }
        else if(!GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT) 
            && !GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT)
            && !GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT))
        {
            /*waiting*/
            if (p_db_dot1x_mac->t_delete)
            {
                timer = ctc_timer_get_remain_sec(p_db_dot1x_mac->t_delete);
            }
            else
            {
                DOT1X_LOG_ERR("dot1x_show_port_mac p_db_dot1x_mac->t_delete is null\n");    
            }
        }
        else
        {
            /*accept*/
            if (p_db_dot1x_mac->t_reauth)
            {
                timer = ctc_timer_get_remain_sec(p_db_dot1x_mac->t_reauth);
            }
        }
        
        if (p_db_dot1x_port->guest_vlan)
        {  
            if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT)
                || GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT)
                || GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_EAPOL_REAUTH))
            {
                sal_strncpy(guest_vlan_str, "No", sizeof(guest_vlan_str));
            }
            else
            {
                /*reject and waiting entries are in the guest vlan*/
                sal_snprintf(guest_vlan_str, sizeof(guest_vlan_str), "Yes/%d", 
                             p_db_dot1x_port->guest_vlan);
            }
        }
        else
        {
            sal_strncpy(guest_vlan_str, "N/A", sizeof(guest_vlan_str));
        }

        sal_fprintf(fp, "%-10s %.04hx.%.04hx.%.04hx %-8s %-8s %-6d %-13s",
            get_intf_print_name(p_db_dot1x_port->name),
            sal_ntohs(((unsigned short *)p_db_dot1x_mac->key.mac)[0]),
            sal_ntohs(((unsigned short *)p_db_dot1x_mac->key.mac)[1]),
            sal_ntohs(((unsigned short *)p_db_dot1x_mac->key.mac)[2]),
            GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_EAPOL_REAUTH) ? "REAUTH":
            GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT)? "REJECT" :
            GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT)? "ACCEPT" :
            GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT) ? "REAUTH":
                "WAITING",
            GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_BYPASS)? "TRUE":"FALSE",
            timer,  guest_vlan_str);
    
        sal_fprintf(fp,  "\n");
    }

    return;
}

int32
dot1x_show_all_mac(cfg_cmd_para_t *para)
{
    tbl_dot1x_port_master_t   *p_glb_port = NULL;
    tbl_dot1x_port_t          *p_db_dot1x_port = NULL;
    ctclib_slistnode_t        *listnode = NULL;  
    tbl_dot1x_mac_master_t    *p_glb_dot1x_mac = NULL;
    tbl_dot1x_global_t        *p_db_glb = NULL;
    FILE                      *fp = NULL; 
    
    /* check exist */
    p_glb_port = tbl_dot1x_port_get_master();

    if (!p_glb_port)
    {
        CFG_FUNC_ERR_RET("dot1x_show_port_mac fail because tbl_dot1x_port is not exist");
    }

    p_glb_dot1x_mac = tbl_dot1x_mac_get_master();

    if (!p_glb_dot1x_mac)
    {
        CFG_FUNC_ERR_RET("dot1x_show_port_mac fail because tbl_dot1x_mac is not exist");
    }

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("dot1x_show_port_mac fail because tbl_dot1x_global is not exist");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    sal_fprintf(fp, "MAC based dot1x port count:%d\n", p_db_glb->mac_port_count_current);

    sal_fprintf(fp, "System user count (hardware accept or reject entries): %d/%d\n", 
        p_db_glb->mac_user_count_current, p_db_glb->mac_user_count_max);

    sal_fprintf(fp, "System user count (include waiting entries): %d/%d\n", 
        p_db_glb->mac_user_count_db, (p_db_glb->mac_user_count_max * 2));
    
    sal_fprintf(fp, "--------------------------------------------------------------------------------\n");
     
    sal_fprintf(fp, "%-10s %-14s %-8s %-8s %-6s %-13s\n", 
                "interface", "mac address", "state", "bypass", "timer", "in guest vlan");

    CTCLIB_SLIST_LOOP(p_glb_port->dot1x_port_list, p_db_dot1x_port, listnode)
    {
        dot1x_show_port_mac(p_db_dot1x_port, p_db_glb, p_glb_dot1x_mac, para, fp);
    }

    sal_fprintf(fp, "--------------------------------------------------------------------------------\n");    

    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE; 
}

int32 dot1x_show_interface_mac(char *name, cfg_cmd_para_t *para)
{
    int32                   rc = PM_E_NONE;
    tbl_dot1x_port_t        dot1x_port;
    tbl_dot1x_port_t        *p_db_dot1x_port = NULL;
    tbl_interface_t         intf;
    tbl_interface_t         *p_db_if = NULL;
    tbl_dot1x_mac_master_t  *p_glb_dot1x_mac = NULL;
    tbl_dot1x_global_t      *p_db_glb = NULL;
    FILE                    *fp = NULL; 

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    sal_memset(&intf, 0, sizeof(tbl_interface_t));    

    sal_strncpy(intf.key.name, name, IFNAME_SIZE);
    
    p_db_if = tbl_interface_get_interface(&intf.key);
    if (!p_db_if)
    {
        CFG_FUNC_ERR_RET("Invalid interface: %s", name);
    }

    sal_strncpy(dot1x_port.name, name, IFNAME_SIZE);
    rc = dot1x_port_check_dot1x_interface(&dot1x_port, para); 
    if (rc)
    {
        return rc;
    }
    
    dot1x_port.key.ifindex = p_db_if->ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        CFG_FUNC_ERR_RET("802.1x not configured on this interface!");
    }
    
    p_glb_dot1x_mac = tbl_dot1x_mac_get_master();

    if (!p_glb_dot1x_mac)
    {
        CFG_FUNC_ERR_RET("dot1x_show_port_mac fail because tbl_dot1x_mac is not exist");
    }

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("dot1x_show_port_mac fail because tbl_dot1x_global is not exist");
    }

    if(AUTH_PORT_MODE_MAC != p_db_dot1x_port->auth_mode)
    {
        CFG_FUNC_ERR_RET("Mac based 802.1x not configured on %s", name);
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    sal_fprintf(fp, "Port user count (hardware accept or reject entries): %d/%d\n", p_db_dot1x_port->current_user,
        (p_db_dot1x_port->max_user ? p_db_dot1x_port->max_user : p_db_glb->mac_user_count_max));
    
    sal_fprintf(fp, "Port user count (include waiting entries): %d/%d\n", p_db_dot1x_port->current_user_count_db,
            ((p_db_dot1x_port->max_user ? p_db_dot1x_port->max_user : p_db_glb->mac_user_count_max) * 2));
    
    sal_fprintf(fp, "--------------------------------------------------------------------------------\n");
     
    sal_fprintf(fp, "%-10s %-14s %-8s %-8s %-6s %-13s\n", 
                "interface", "mac address", "state", "bypass", "timer", "in guest vlan");

    dot1x_show_port_mac(p_db_dot1x_port, p_db_glb, p_glb_dot1x_mac, para, fp);
    
    sal_fprintf(fp, "--------------------------------------------------------------------------------\n");    

    sal_fclose(fp);
    fp = NULL;
        
    return rc;
}

int32
dot1x_show_timers(cfg_cmd_para_t *para)
{
    tbl_dot1x_global_t        *p_db_glb = NULL;
    FILE                      *fp = NULL; 
    tbl_auth_session_master_t *pmaster = NULL;
    tbl_dot1x_port_master_t   *p_glb_port = NULL;
        tbl_dot1x_port_t          *p_db_dot1x_port = NULL;
        ctclib_slistnode_t        *listnode = NULL;  

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("tbl_dot1x_global is not exist");
    }

    pmaster = tbl_auth_session_get_master();
    if (!pmaster)
    {
        CFG_FUNC_ERR_RET("tbl_auth_session is not exist");;
    }

    p_glb_port = tbl_dot1x_port_get_master();
    if (!p_glb_port)
    {
        CFG_FUNC_ERR_RET("tbl_dot1x_port is not exist");
    }  

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "802.1X running session number: %d\n", pmaster->session_cnt);
    sal_fprintf(fp, "802.1X port timers:\n");

    CTCLIB_SLIST_LOOP(p_glb_port->dot1x_port_list, p_db_dot1x_port, listnode)
    {
        sal_fprintf(fp, "  802.1X timers info for interface %s\n", get_intf_print_name(p_db_dot1x_port->name));

        sal_fprintf(fp, "    quiet-period timer %d\n", p_db_dot1x_port->quietWhile);
        sal_fprintf(fp, "    tx-period timer %d\n", p_db_dot1x_port->txWhen);
        sal_fprintf(fp, "    reAuthPeriod timer %d\n", p_db_dot1x_port->reAuthWhen);
        sal_fprintf(fp, "    supp-timeout or server-timeout timer %d\n", p_db_dot1x_port->aWhile);
    }

    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE; 
}

#define DBG_PRINT_BANNER() \
do { \
    if (banner) \
    { \
        sal_fprintf(fp, "%-12s %-16s %-20s %-6s\n", "Module", "Feature", "Type", "Status"); \
        sal_fprintf(fp, "------------+----------------+--------------------+------\n"); \
        banner = FALSE; \
    } \
} while (0)

int32
dot1x_show_debugging_all(uint32 detail, cfg_cmd_para_t *para)
{
    tbl_dot1x_global_t        *p_db_glb = NULL;
    FILE                      *fp = NULL;  
    bool                      first = TRUE;
    bool                      banner = TRUE;
    uint32                    is_on = FALSE;
    char                      *mod_str = "dot1x";
    char                      *sub_mod_str = "dot1x";

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        CFG_FUNC_ERR_RET("tbl_dot1x_global is not exist");
    } 

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    is_on = dot1x_debug_is_on(DOT1X_FLAG_EVENTS);
    if (detail || is_on)
    {
        DBG_PRINT_BANNER();
        sal_fprintf(fp, "%-12s %-16s %-20s %-6s\n", mod_str, sub_mod_str, "event", is_on ? "on" : "off");
        first = FALSE;
    }

    is_on = dot1x_debug_is_on(DOT1X_FLAG_PACKET);
    if (detail || is_on)
    {
        DBG_PRINT_BANNER();    
        if (first)
        {
            sal_fprintf(fp, "%-12s %-16s %-20s %-6s\n", mod_str, sub_mod_str, "packet", is_on ? "on" : "off");
            first = FALSE;
        }
        else
        {
            sal_fprintf(fp, "%-12s %-16s %-20s %-6s\n", " ", sub_mod_str, "packet", is_on ? "on" : "off");
        }
    }

    is_on = dot1x_debug_is_on(DOT1X_FLAG_PROTO);
    if (detail || is_on)
    {
        DBG_PRINT_BANNER();    
        if (first)
        {
            sal_fprintf(fp, "%-12s %-16s %-20s %-6s\n", mod_str, sub_mod_str, "protocol", is_on ? "on" : "off");
            first = FALSE;
        }
        else
        {
            sal_fprintf(fp, "%-12s %-16s %-20s %-6s\n", " ", sub_mod_str, "protocol", is_on ? "on" : "off");
        }
    }

    is_on = dot1x_debug_is_on(DOT1X_FLAG_TIMER);
    if (detail || is_on)
    {
        DBG_PRINT_BANNER();    
        if (first)
        {
            sal_fprintf(fp, "%-12s %-16s %-20s %-6s\n", mod_str, sub_mod_str, "timer", is_on ? "on" : "off");
        }
        else
        {
            sal_fprintf(fp, "%-12s %-16s %-20s %-6s\n", " ", sub_mod_str, "timer", is_on ? "on" : "off");
        }
    }

    if (!first)
    {
        sal_fprintf(fp, "\n");
    }

    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE; 
}

int32
dot1x_cmd_show_dot1x(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32                   detail = FALSE;
    int32                    rc = PM_E_NONE;
    FILE                     *fp = NULL;
    
    if (0 == argc)
    {  
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
    
        dot1x_show_system(para, fp);
        
        sal_fclose(fp);
        fp = NULL;
    }
    else if ((1 == argc) && (0 == sal_memcmp(argv[0], "debugging", sal_strlen("debugging"))))
    {
        dot1x_show_debugging_all(detail, para);
    }
    else if ((2 == argc) && (0 == sal_memcmp(argv[0], "debugging", sal_strlen("debugging"))))
    {
        if (0 == sal_memcmp(argv[1], "detail", sal_strlen("detail")))
        {
            detail = TRUE;
        }
        dot1x_show_debugging_all(detail, para);
    }
    else if ((1 == argc) && (0 == sal_memcmp(argv[0], "timers", sal_strlen("timers"))))
    {
        dot1x_show_timers(para);
    }
    else if ((1 == argc) && (0 == sal_memcmp(argv[0], "all", sal_strlen("all"))))
    {
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
    
        dot1x_show_system(para, fp);
        dot1x_show_all(para, fp);  

        sal_fclose(fp);
        fp = NULL;
    }
    else if ((2 == argc) && (0 == sal_memcmp(argv[0], "interface", sal_strlen("interface"))))
    {
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
    
        rc = dot1x_show_interface(argv[1], para, fp);

        sal_fclose(fp);
        fp = NULL;
    }
    else if ((1 == argc) && (0 == sal_memcmp(argv[0], "statistics", sal_strlen("statistics"))))
    {
        dot1x_show_all_stats(para);    
    }
    else if ((3 == argc) && (0 == sal_memcmp(argv[0], "statistics", sal_strlen("statistics")))
        && (0 == sal_memcmp(argv[1], "interface", sal_strlen("interface"))))
    {
        dot1x_show_interface_stats(argv[2], para);    
    }
    else if ((1 == argc) && (0 == sal_memcmp(argv[0], "session-statistics", sal_strlen("session-statistics"))))
    {
        dot1x_show_all_session_stats(para);    
    }
    else if ((3 == argc) && (0 == sal_memcmp(argv[0], "session-statistics", sal_strlen("session-statistics")))
        && (0 == sal_memcmp(argv[1], "interface", sal_strlen("interface"))))
    {
        rc = dot1x_show_interface_session_stats(argv[2], para);    
    }
    else if ((1 == argc) && (0 == sal_memcmp(argv[0], "diagnostics", sal_strlen("diagnostics"))))
    {
        dot1x_show_all_diagnostics(para);    
    }
    else if ((3 == argc) && (0 == sal_memcmp(argv[0], "diagnostics", sal_strlen("diagnostics")))
        && (0 == sal_memcmp(argv[1], "interface", sal_strlen("interface"))))
    {
        rc = dot1x_show_interface_diagnostics(argv[2], para);    
    }
    else if ((1 == argc) && (0 == sal_memcmp(argv[0], "mac", sal_strlen("mac"))))
    {
        rc = dot1x_show_all_mac(para);    
    }
    else if ((3 == argc) && (0 == sal_memcmp(argv[0], "mac", sal_strlen("mac")))
        && (0 == sal_memcmp(argv[1], "interface", sal_strlen("interface"))))
    {
        rc = dot1x_show_interface_mac(argv[2], para);    
    }
    
    return rc;
}

int32
dot1x_clear_stats(cfg_cmd_para_t *para)
{
    tbl_dot1x_port_master_t   *p_glb_port = NULL;
    tbl_dot1x_port_t          *p_db_dot1x_port = NULL;
    ctclib_slistnode_t        *listnode = NULL;  

    /* check exist */
    p_glb_port = tbl_dot1x_port_get_master();

    if (!p_glb_port)
    {
        CFG_FUNC_ERR_RET("dot1x_clear_stats fail because tbl_dot1x_port is not exist");
    }
        
    CTCLIB_SLIST_LOOP(p_glb_port->dot1x_port_list, p_db_dot1x_port, listnode)
    {
        p_db_dot1x_port->eapol_frames_rx = 0;
        p_db_dot1x_port->eapol_frames_tx = 0;
        p_db_dot1x_port->eapol_start_frames_rx = 0;
        p_db_dot1x_port->eapol_logoff_frames_rx = 0;
        p_db_dot1x_port->eap_respid_frames_rx = 0;
        p_db_dot1x_port->eap_resp_frames_rx = 0;
        p_db_dot1x_port->eap_reqid_frames_tx = 0;
        p_db_dot1x_port->eap_req_frames_tx = 0;
        p_db_dot1x_port->eapol_invalid_frames_rx = 0;
        p_db_dot1x_port->eap_len_error_frames_rx = 0;
        p_db_dot1x_port->eapol_last_frame_version = 0;
        sal_memset(p_db_dot1x_port->eapol_last_frame_source, 0, GLB_ETH_ADDR_LEN);
    }
    
    return PM_E_NONE; 
}

int32
dot1x_clear_session_stats(cfg_cmd_para_t *para)
{
    tbl_dot1x_port_master_t   *p_glb_port = NULL;
    tbl_dot1x_port_t          *p_db_dot1x_port = NULL;
    ctclib_slistnode_t        *listnode = NULL;  

    /* check exist */
    p_glb_port = tbl_dot1x_port_get_master();

    if (!p_glb_port)
    {
        CFG_FUNC_ERR_RET("dot1x_clear_session_stats fail because tbl_dot1x_port is not exist");
    }
        
    CTCLIB_SLIST_LOOP(p_glb_port->dot1x_port_list, p_db_dot1x_port, listnode)
    {
        p_db_dot1x_port->session_time = 0;
        p_db_dot1x_port->session_authentication_method = 0;
        p_db_dot1x_port->session_terminate_cause = 0;

        sal_memset(p_db_dot1x_port->session_user_name, 0, (AUTH_USER_NAMLEN+1));
    }
    
    return PM_E_NONE; 
}

int32
dot1x_clear_interface(char *name, char *mac_addr, cfg_cmd_para_t *para)
{
    int32                   rc = PM_E_NONE;
    tbl_dot1x_port_t        dot1x_port;
    tbl_dot1x_port_t        *p_db_dot1x_port = NULL;
    tbl_interface_t         intf;
    tbl_interface_t         *p_db_if = NULL;
    tbl_dot1x_mac_key_t     dot1x_mac_key;
    mac_addr_t              user_mac;
    tbl_dot1x_mac_t         *p_db_dot1x_mac = NULL;

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    sal_memset(&intf, 0, sizeof(tbl_interface_t)); 
    sal_memset(&dot1x_mac_key, 0, sizeof(tbl_dot1x_mac_key_t));

    sal_strncpy(intf.key.name, name, IFNAME_SIZE);
    
    p_db_if = tbl_interface_get_interface(&intf.key);
    if (!p_db_if)
    {
        CFG_FUNC_ERR_RET("Invalid interface: %s", name);
    }
    
    dot1x_port.key.ifindex = p_db_if->ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        CFG_FUNC_ERR_RET("802.1x not configured on this interface!");
    }

    if (mac_addr)
    {
        rc = cdb_mac_addr_str2val(user_mac, mac_addr);
        if (rc < 0)
        {
            CFG_FUNC_ERR_RET("Unable to translate mac address user");
        }

        dot1x_mac_key.ifindex = p_db_if->ifindex;
        sal_memcpy(&dot1x_mac_key.mac, user_mac, sizeof(mac_addr_t));
        
        p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&dot1x_mac_key);
        if (p_db_dot1x_mac)
        {
            dot1x_api_user_off_line(p_db_dot1x_port, p_db_dot1x_mac);  
        }
    }
    else
    {        
        dot1x_api_user_off_line_whole_port(p_db_dot1x_port);
    }

    return rc;

}

int32
dot1x_cmd_clear_dot1x(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                   rc = PM_E_NONE;

    if ((1 == argc) && (0 == sal_memcmp(argv[0], "Statistics", sal_strlen("Statistics"))))
    {
        dot1x_clear_stats(para);    
    }
    else if ((1 == argc) && (0 == sal_memcmp(argv[0], "session-statistics", sal_strlen("session-statistics"))))
    {
        dot1x_clear_session_stats(para);    
    }
    else if ((3 == argc) && (0 == sal_memcmp(argv[0], "interface", sal_strlen("interface"))))
    {
        dot1x_clear_interface(argv[1], NULL, para);    
    }
    else if ((4 == argc) && (0 == sal_memcmp(argv[0], "interface", sal_strlen("interface"))))
    {
        dot1x_clear_interface(argv[1], argv[3], para);    
    }

    return rc;
}

int32
dot1x_init(void)
{
    tbl_dot1x_global_t dot1x_glb;
    int32 rc = PM_E_NONE;
    tbl_dot1x_global_t        *p_db_glb = NULL;

    sal_memset(&dot1x_glb, 0, sizeof(dot1x_glb));

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        DOT1X_LOG_ERR("Tbl_dot1x_global not found");
        return PM_E_FAIL;
    }

    if (0 == p_db_glb->radius_default_retry)
    {
        dot1x_glb.radius_default_retry = RADIUS_RETRANSMIT_DEFAULT;
        rc = tbl_dot1x_global_set_dot1x_global_field(&dot1x_glb, TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_RETRY);
        if (rc)
        {
            DOT1X_LOG_ERR("dot1x_init set radius defalut retry return %d", rc);
        }
    }

    if (0 == p_db_glb->radius_default_timeout)
    {
        dot1x_glb.radius_default_timeout = RADIUS_TIMEOUT_DEFAULT;
        rc = tbl_dot1x_global_set_dot1x_global_field(&dot1x_glb, TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_TIMEOUT);
        if (rc)
        {
            DOT1X_LOG_ERR("dot1x_init set radius defalut timeout return %d", rc);
        }
    }

    if (0 == p_db_glb->radius_default_deadtime)
    {
        dot1x_glb.radius_default_deadtime = RADIUS_DEADTIME_DEFAULT;
        rc = tbl_dot1x_global_set_dot1x_global_field(&dot1x_glb, TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_DEADTIME);
        if (rc)
        {
            DOT1X_LOG_ERR("dot1x_init set radius defalut deadtime return %d", rc);
        }
    }

    /* 802.1X-2001 Section 7.8 PAE group address assignment.  */
    dot1x_glb.eapol_group_address[0] = 0x01;
    dot1x_glb.eapol_group_address[1] = 0x80;
    dot1x_glb.eapol_group_address[2] = 0xC2;
    dot1x_glb.eapol_group_address[3] = 0x00;
    dot1x_glb.eapol_group_address[4] = 0x00;
    dot1x_glb.eapol_group_address[5] = 0x03;

    rc = tbl_dot1x_global_set_dot1x_global_field(&dot1x_glb, TBL_DOT1X_GLOBAL_FLD_EAPOL_GROUP_ADDRESS);   
    if (rc)
    {
        DOT1X_LOG_ERR("dot1x_init set eapol group address return %d", rc);
    }

    dot1x_glb.mac_user_count_max = DEFAULT_MAC_USER_COUNT_MAX;
    rc = tbl_dot1x_global_set_dot1x_global_field(&dot1x_glb, TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_MAX);   
    if (rc)
    {
        DOT1X_LOG_ERR("dot1x_init set mac user count max return %d", rc);
    }
    
    dot1x_glb.last_session_id = RADIUS_SESSION_BUFFER - 1;
    rc = tbl_dot1x_global_set_dot1x_global_field(&dot1x_glb, TBL_DOT1X_GLOBAL_FLD_LAST_SESSION_ID);   
    if (rc)
    {
        DOT1X_LOG_ERR("dot1x_init set last session id return %d", rc);
    }

    return PM_E_NONE;
}

int32
dot1x_start()
{
    cdb_register_cfg_tbl_cb(TBL_DOT1X_GLOBAL,   dot1x_cmd_process_global); 
    cdb_register_cfg_tbl_cb(TBL_DOT1X_PORT,   dot1x_cmd_process_port); 
    cdb_register_cfg_tbl_cb(TBL_DOT1X_RADIUS,   dot1x_cmd_process_radius); 

    cdb_register_cfg_act_cb(ACT_SHOW_RADIUS_SERVER, dot1x_cmd_show_radius_server);
    cdb_register_cfg_act_cb(ACT_SHOW_DOT1X, dot1x_cmd_show_dot1x);  
    cdb_register_cfg_act_cb(ACT_CLEAR_DOT1X, dot1x_cmd_clear_dot1x);

    ipc_register_pkt_rx_fn(GLB_PKT_EAPOL, dot1x_pdu_rx);
    ipc_register_pkt_rx_fn(GLB_PKT_MAC_BY_PASS, dot1x_mac_by_pass_pdu_rx);

    return 0;
}

