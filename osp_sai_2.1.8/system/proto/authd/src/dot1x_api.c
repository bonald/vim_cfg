/****************************************************************************
*  dot1x process
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
#include "gen/tbl_auth_cfg_define.h"
#include "gen/tbl_auth_cfg.h"
#include "dot1x.h"
#include "dot1x_api.h"
/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/


/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/


/****************************************************************************
 *
* Function
* 
****************************************************************************/
/* modified by liwh for bug 46709, 2018-05-24 */
extern void auth_reactivate_debug_info(tbl_auth_server_t   *pItem);
/* liwh end */

char *get_intf_print_name(char *pszIfname) 
{
    static char ifname_ext[IFNAME_EXT_SIZE];

    IFNAME_ETH2FULL(pszIfname, ifname_ext);
    return ifname_ext;
}

bool dot1x_debug_is_on(uint32 flag)
{
    tbl_dot1x_global_t *p_db_glb = NULL;
    
    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (p_db_glb)
    {
        if (GLB_FLAG_ISSET(p_db_glb->dot1x, flag))
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

void dot1x_port_timer_fn(void *p_data)
{
    tbl_dot1x_port_t      *p_db_dot1x_port = NULL;

    p_db_dot1x_port = (tbl_dot1x_port_t*)p_data;
    if (!p_db_dot1x_port)
    {
        DOT1X_LOG_ERR("dot1x_port_timer_fn null dot1x port in port timer timeout\n");
        return;
    }

    p_db_dot1x_port->t_timer = NULL;

    if (1 == p_db_dot1x_port->quietWhile)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_TIMER, "[%s]: quiet-period timer timeout", get_intf_print_name(p_db_dot1x_port->name));
    }

    if (1 == p_db_dot1x_port->txWhen)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_TIMER, "[%s]: tx-period timer timeout", get_intf_print_name(p_db_dot1x_port->name));
    }

    if (1 == p_db_dot1x_port->reAuthWhen)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_TIMER, "[%s]: reAuthPeriod timer timeout", get_intf_print_name(p_db_dot1x_port->name));
    }

    if (1 == p_db_dot1x_port->aWhile)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_TIMER, "[%s]: supp-timeout or server-timeout timer timeout", get_intf_print_name(p_db_dot1x_port->name));
    }
    
    DEC(p_db_dot1x_port->aWhile);
    DEC(p_db_dot1x_port->authWhile);
    DEC(p_db_dot1x_port->quietWhile);
    DEC(p_db_dot1x_port->reAuthWhen);
    DEC(p_db_dot1x_port->txWhen);

    if (AUTH_PORT_STATUS_AUTHORIZED == p_db_dot1x_port->portStatus)
    {
        p_db_dot1x_port->session_time++;
    }

    dot1x_run_sm(p_db_dot1x_port);
    
    p_db_dot1x_port->t_timer = ctc_task_add_timer(TASK_PRI_NORMAL, dot1x_port_timer_fn, 
                    p_db_dot1x_port, DOT1X_PORT_TIMER_INTERVAL); 

    return;
}

int32 dot1x_port_check_dot1x_interface(tbl_dot1x_port_t *p_db_dot1x_port, cfg_cmd_para_t *para) 
{
    tbl_interface_t     *p_if = NULL;

    p_if = tbl_interface_get_interface_by_name(p_db_dot1x_port->name);
    if (!p_if)
    {
        CFG_FUNC_ERR_RET("Invalid interface or interface does not exist! must be access switch port or routed port!");
    }

    /*check l2protocol uplink*/
    if (p_if->l2pro_uplink)
    {
        CFG_FUNC_ERR_RET("The interface is configured as l2 protocol uplink port");    
    }
    
    /* check interface is not switchport port */
    if ((GLB_IF_MODE_L3 == p_if->mode) && (GLB_IF_TYPE_PORT_IF != p_if->hw_type))
    {
        CFG_FUNC_ERR_RET("Invalid interface or interface does not exist! must be access switch port or routed port!");
    }

    /* check interface is aggregregated or agg */   
    if (NULL != p_if->lag)
    {
        CFG_FUNC_ERR_RET("Invalid interface or interface does not exist! must be access switch port or routed port!");
    }
    
    /* check interface is trunk and not qinq */
    if ((GLB_IF_MODE_L2 == p_if->mode) && 
        ((GLB_VLAN_PORT_TYPE_TRUNK == p_if->vlan_type)
        || (GLB_VLAN_PORT_TYPE_QINQ == p_if->vlan_type)))
    {
        CFG_FUNC_ERR_RET("Invalid interface or interface does not exist! must be access switch port or routed port!");    
    }

    return PM_E_NONE;
}

int32
dot1x_port_clear_dead_radius(void)
{
    int32                     rc = PM_E_NONE;    
    ctclib_slistnode_t       *listnode = NULL;
    tbl_auth_server_master_t *p_master = NULL;
    tbl_auth_server_t        *pItem = NULL;   

    p_master = tbl_auth_server_get_master();
    if (!p_master)
    {
        DOT1X_LOG_ERR("dot1x_port_clear_dead_radius can't get tbl_auth_server master"); 
        return rc;
    }
    
    CTCLIB_SLIST_LOOP(p_master->auth_servers, pItem, listnode) 
    {
        if (pItem->dead && (AUTH_SERVER_TYPE_RADIUS == pItem->key.type))
        {
            pItem->dead = FALSE;

            /* modified by liwh for bug 46709, 2018-05-24 */
            auth_reactivate_debug_info(pItem);
            /* liwh end */
        }
    }

    return rc;
}

void
dot1x_port_active_radius_server(tbl_dot1x_radius_t  *p_db_dot1x_radius)
{
    auth_server_key_t   auth_server_key;
    tbl_auth_server_t   *p_auth_server = NULL;

    sal_memset(&auth_server_key, 0, sizeof(auth_server_key_t));

    sal_memcpy(&auth_server_key.addr, &p_db_dot1x_radius->key.addr, sizeof(addr_t));
    auth_server_key.sin_port = p_db_dot1x_radius->key.sin_port;
    auth_server_key.type = AUTH_SERVER_TYPE_RADIUS;
    
    p_auth_server = tbl_auth_server_get_auth_server(&auth_server_key);
    if (p_auth_server)
    {
        p_auth_server->dead = FALSE;

        /* modified by liwh for bug 46709, 2018-05-24 */
        auth_reactivate_debug_info(p_auth_server);
        /* liwh end */
    }
    else
    {
        DOT1X_LOG_ERR("RADIUS host %s does not exist", p_db_dot1x_radius->server_string); 
    }

    return; 
}

void
dot1x_set_auth_cfg_retry(tbl_dot1x_global_t *p_glb)
{
    tbl_auth_cfg_t *p_db_item = NULL;

    p_db_item = tbl_auth_cfg_get_auth_cfg();
    if (p_db_item)       
    {
        p_db_item->retries = p_glb->radius_default_retry;
        tbl_auth_cfg_set_auth_cfg_field(p_db_item, TBL_AUTH_CFG_FLD_RETRIES);
    }
    else
    {
        DOT1X_LOG_ERR("dot1x_set_auth_cfg_retry p_db_item is null ptr"); 
    }

    return;
}

void
dot1x_set_auth_cfg_timeout(tbl_dot1x_global_t *p_glb)
{
    tbl_auth_cfg_t *p_db_item = NULL;

    p_db_item = tbl_auth_cfg_get_auth_cfg();
    if (p_db_item)       
    {
        p_db_item->timeout = p_glb->radius_default_timeout;
        tbl_auth_cfg_set_auth_cfg_field(p_db_item, TBL_AUTH_CFG_FLD_TIMEOUT);
    }
    else
    {
        DOT1X_LOG_ERR("dot1x_set_auth_cfg_retry p_db_item is null ptr"); 
    }

    return;
}

void
dot1x_set_auth_cfg_deadtime(tbl_dot1x_global_t *p_glb)
{
    tbl_auth_cfg_t *p_db_item = NULL;

    p_db_item = tbl_auth_cfg_get_auth_cfg();
    if (p_db_item)       
    {
        p_db_item->deadtime = p_glb->radius_default_deadtime * 60;
        tbl_auth_cfg_set_auth_cfg_field(p_db_item, TBL_AUTH_CFG_FLD_DEADTIME);
    }
    else
    {
        DOT1X_LOG_ERR("dot1x_set_auth_cfg_retry p_db_item is null ptr"); 
    }

    return;
}

void
dot1x_set_auth_cfg_shared_secret(tbl_dot1x_global_t *p_glb)
{
    tbl_auth_cfg_t *p_db_item = NULL;

    p_db_item = tbl_auth_cfg_get_auth_cfg();
    if (p_db_item)       
    {
        sal_memcpy(p_db_item->secret, p_glb->radius_default_shared_secret, GLB_AUTH_SECRET_SIZE + 1);
        sal_memcpy(p_db_item->enc_secret, p_glb->radius_default_shared_enc_secret, PASSWD_SIZE);

        tbl_auth_cfg_set_auth_cfg_field(p_db_item, TBL_AUTH_CFG_FLD_SECRET);
        tbl_auth_cfg_set_auth_cfg_field(p_db_item, TBL_AUTH_CFG_FLD_ENC_SECRET);
    }
    else
    {
        DOT1X_LOG_ERR("dot1x_set_auth_cfg_retry p_db_item is null ptr"); 
    }

    return;
}

void
dot1x_set_auth_server_retry(tbl_dot1x_radius_t  *p_db_dot1x_radius)
{
    auth_server_key_t   auth_server_key;
    tbl_auth_server_t   *p_auth_server = NULL;

    sal_memset(&auth_server_key, 0, sizeof(auth_server_key_t));

    sal_memcpy(&auth_server_key.addr, &p_db_dot1x_radius->key.addr, sizeof(addr_t));
    auth_server_key.sin_port = p_db_dot1x_radius->key.sin_port;
    auth_server_key.type = AUTH_SERVER_TYPE_RADIUS;
    
    p_auth_server = tbl_auth_server_get_auth_server(&auth_server_key);
    if (p_auth_server)
    {
        p_auth_server->retries = p_db_dot1x_radius->max_retry;
        tbl_auth_server_set_auth_server_field(p_auth_server, TBL_AUTH_SERVER_FLD_RETRIES);
    }
    else
    {
        DOT1X_LOG_ERR("RADIUS host %s does not exist", p_db_dot1x_radius->server_string); 
    }

    return; 
}

void
dot1x_set_auth_server_timeout(tbl_dot1x_radius_t  *p_db_dot1x_radius)
{
    auth_server_key_t   auth_server_key;
    tbl_auth_server_t   *p_auth_server = NULL;

    sal_memset(&auth_server_key, 0, sizeof(auth_server_key_t));

    sal_memcpy(&auth_server_key.addr, &p_db_dot1x_radius->key.addr, sizeof(addr_t));
    auth_server_key.sin_port = p_db_dot1x_radius->key.sin_port;
    auth_server_key.type = AUTH_SERVER_TYPE_RADIUS;
    
    p_auth_server = tbl_auth_server_get_auth_server(&auth_server_key);
    if (p_auth_server)
    {
        p_auth_server->timeout = p_db_dot1x_radius->timeout;
        tbl_auth_server_set_auth_server_field(p_auth_server, TBL_AUTH_SERVER_FLD_TIMEOUT);
    }
    else
    {
        DOT1X_LOG_ERR("RADIUS host %s does not exist", p_db_dot1x_radius->server_string); 
    }

    return; 
}

void
dot1x_set_auth_server_shared_secret(tbl_dot1x_radius_t  *p_db_dot1x_radius)
{
    auth_server_key_t   auth_server_key;
    tbl_auth_server_t   *p_auth_server = NULL;

    sal_memset(&auth_server_key, 0, sizeof(auth_server_key_t));

    sal_memcpy(&auth_server_key.addr, &p_db_dot1x_radius->key.addr, sizeof(addr_t));
    auth_server_key.sin_port = p_db_dot1x_radius->key.sin_port;
    auth_server_key.type = AUTH_SERVER_TYPE_RADIUS;
    
    p_auth_server = tbl_auth_server_get_auth_server(&auth_server_key);
    if (p_auth_server)
    {
        sal_memcpy(p_auth_server->secret, p_db_dot1x_radius->shared_secret, GLB_AUTH_SECRET_SIZE + 1);
        sal_memcpy(p_auth_server->enc_secret, p_db_dot1x_radius->shared_enc_secret, PASSWD_SIZE);

        tbl_auth_server_set_auth_server_field(p_auth_server, TBL_AUTH_SERVER_FLD_SECRET);
        tbl_auth_server_set_auth_server_field(p_auth_server, TBL_AUTH_SERVER_FLD_ENC_SECRET);
    }
    else
    {
        DOT1X_LOG_ERR("RADIUS host %s does not exist", p_db_dot1x_radius->server_string); 
    }

    return; 
}

void
dot1x_set_fea_port_authorize(tbl_dot1x_port_t *p_db_dot1x_port)
{
    tbl_interface_t       *p_db_if = NULL;
    int32                 rc = PM_E_NONE;

    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "dot1x_set_fea_port_authorize is called for ifindex %d", p_db_dot1x_port->key.ifindex);
    
    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_dot1x_port->key.ifindex);
    if (!p_db_if)
    {
        DOT1X_LOG_ERR("dot1x_set_port_authorize fail because Invalid interface or interface does not exist");
        return;
    }  

    GLB_UNSET_FLAG(p_db_if->dot1x_flag, GLB_IF_DOT1X_UNAUTHED);
    GLB_UNSET_FLAG(p_db_if->dot1x_flag, GLB_IF_DOT1X_DIR_IN);
    GLB_UNSET_FLAG(p_db_if->dot1x_flag, GLB_IF_DOT1X_DIR_OUT);
    
    rc = tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_DOT1X_UNAUTHORIZED);
    if (rc)
    {
        DOT1X_LOG_ERR("dot1x_set_port_authorize set TBL_INTERFACE_FLD_DOT1X_UNAUTHORIZED return %d", rc);
        return;     
    }
    
    return;
}

void
dot1x_set_fea_port_unauthorize(tbl_dot1x_port_t *p_db_dot1x_port)
{
    tbl_interface_t       *p_db_if = NULL;
    int32                 rc = PM_E_NONE;
    
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "dot1x_set_fea_port_unauthorize is called for ifindex %d", p_db_dot1x_port->key.ifindex);

    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_dot1x_port->key.ifindex);
    if (!p_db_if)
    {
        DOT1X_LOG_ERR("dot1x_set_port_authorize fail because Invalid interface or interface does not exist");
        return;
    }  

    GLB_SET_FLAG(p_db_if->dot1x_flag, GLB_IF_DOT1X_UNAUTHED);
    GLB_SET_FLAG(p_db_if->dot1x_flag, GLB_IF_DOT1X_DIR_IN);
    if (AUTH_CTRL_DIR_BOTH == p_db_dot1x_port->auth_ctrl_dir_operControlledDirections)
    {
        GLB_SET_FLAG(p_db_if->dot1x_flag, GLB_IF_DOT1X_DIR_OUT);
    }
    else
    {
        GLB_UNSET_FLAG(p_db_if->dot1x_flag, GLB_IF_DOT1X_DIR_OUT);
    }

    if ((AUTH_PORT_CTRL_AUTO == p_db_dot1x_port->portControl)
        && p_db_dot1x_port->guest_vlan_valid)
    {
        dot1x_set_fea_port_guest_vlan(p_db_dot1x_port);  
    }
    else
    {
        rc = tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_DOT1X_UNAUTHORIZED);
        if (rc)
        {
            DOT1X_LOG_ERR("dot1x_set_port_authorize set TBL_INTERFACE_FLD_DOT1X_UNAUTHORIZED return %d", rc);
            return;     
        }

        rc = tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_DOT1X_DIR_OUT);
        if (rc)
        {
            DOT1X_LOG_ERR("dot1x_set_port_authorize set TBL_INTERFACE_FLD_DOT1X_DIR_OUT return %d", rc);
            return;     
        }
    }

    return;
}

int32
dot1x_set_fea_port_enable(tbl_dot1x_port_t *p_db_dot1x_port)
{
    tbl_interface_t       *p_db_if = NULL;
    int32                 rc = PM_E_NONE;
    
    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_dot1x_port->key.ifindex);
    if (!p_db_if)
    {
        DOT1X_LOG_ERR("dot1x_set_port_authorize fail because Invalid interface or interface does not exist");
        return PM_E_FAIL;
    }  

    p_db_if->dot1x_enable = TRUE;    
    rc = tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_DOT1X_ENABLE);
    if (rc)
    {
        DOT1X_LOG_ERR("dot1x_set_port_authorize set TBL_INTERFACE_FLD_DOT1X_ENABLE return %d", rc);
    }
    
    return rc;
}

int32
dot1x_set_fea_port_disable(tbl_dot1x_port_t *p_db_dot1x_port)
{
    tbl_interface_t       *p_db_if = NULL;
    int32                 rc = PM_E_NONE;
    
    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_dot1x_port->key.ifindex);
    if (!p_db_if)
    {
        DOT1X_LOG_ERR("dot1x_set_port_authorize fail because Invalid interface or interface does not exist");
        return PM_E_FAIL;
    }  

    p_db_if->dot1x_enable = FALSE;    
    rc = tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_DOT1X_ENABLE);
    if (rc)
    {
        DOT1X_LOG_ERR("dot1x_set_port_authorize set TBL_INTERFACE_FLD_DOT1X_ENABLE return %d", rc);
        return rc;     
    }
    
    return rc;
}

void
dot1x_set_fea_port_ctrl(tbl_dot1x_port_t *p_db_dot1x_port)
{
    if (AUTH_PORT_STATUS_UNAUTHORIZED == p_db_dot1x_port->portStatus)
    {
        dot1x_set_fea_port_unauthorize(p_db_dot1x_port);
    }
    else
    {
        dot1x_set_fea_port_authorize(p_db_dot1x_port);    
    }
   
    return;
}

void
dot1x_set_fea_mac_guest_vlan(tbl_dot1x_port_t *p_db_dot1x_port)
{
    tbl_interface_t       *p_db_if = NULL;
    int32                 rc = PM_E_NONE;
    
    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_dot1x_port->key.ifindex);
    if (!p_db_if)
    {
        DOT1X_LOG_ERR("dot1x_set_port_authorize fail because Invalid interface or interface does not exist");
        return;
    }

    if (p_db_dot1x_port->guest_vlan_valid)
    {
        if (p_db_dot1x_port->guest_vlan == p_db_if->dot1x_guest_vlan)
        {
            return;    
        }
    }
    else
    {
        if (0 == p_db_if->dot1x_guest_vlan)
        {
            return;    
        }
    }        
        
    if (!p_db_dot1x_port->guest_vlan_valid)
    {
        p_db_if->dot1x_guest_vlan = 0;   
    }
    else
    {
        p_db_if->dot1x_guest_vlan = p_db_dot1x_port->guest_vlan;
    }

    rc = tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_DOT1X_GUEST_VLAN);
    if (rc)
    {
        DOT1X_LOG_ERR("dot1x_set_port_authorize set TBL_INTERFACE_FLD_DOT1X_GUEST_VLAN return %d", rc);
        return;     
    }

    return;
}

void
dot1x_set_fea_port_guest_vlan(tbl_dot1x_port_t *p_db_dot1x_port)
{
    int32                 rc = PM_E_NONE;
    tbl_interface_t       *p_db_if = NULL;

    if (AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
    {
        return;
    }

    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_dot1x_port->key.ifindex);
    if (!p_db_if)
    {
        DOT1X_LOG_ERR("dot1x_set_port_authorize fail because Invalid interface or interface does not exist");
        return;
    }

    if (p_db_dot1x_port->guest_vlan_valid)
    {
        if (p_db_dot1x_port->guest_vlan == p_db_if->dot1x_guest_vlan)
        {
            return;    
        }
    }
    else
    {
        if (0 == p_db_if->dot1x_guest_vlan)
        {
            return;    
        }
    }        
        
    if (!p_db_dot1x_port->guest_vlan_valid)
    {
        p_db_if->dot1x_guest_vlan = 0;   

        if (AUTH_PORT_STATUS_UNAUTHORIZED == p_db_dot1x_port->portStatus)
        {
            dot1x_set_fea_port_unauthorize(p_db_dot1x_port);
        }
    }
    else
    {
        p_db_if->dot1x_guest_vlan = p_db_dot1x_port->guest_vlan;

        dot1x_set_fea_port_authorize(p_db_dot1x_port);
    }

    rc = tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_DOT1X_GUEST_VLAN);
    if (rc)
    {
        DOT1X_LOG_ERR("dot1x_set_port_authorize set TBL_INTERFACE_FLD_DOT1X_GUEST_VLAN return %d", rc);
        return;     
    }

    return;
}

int32
dot1x_port_init(tbl_dot1x_port_t *p_db_dot1x_port)
{
    int32 rc = PM_E_NONE;

    p_db_dot1x_port->initialize = TRUE;

    if(AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
    {
        dot1x_api_user_off_line_whole_port(p_db_dot1x_port);
    }

    p_db_dot1x_port->session_terminate_cause = AUTH_PORT_REINIT;
    dot1x_run_sm(p_db_dot1x_port);

    p_db_dot1x_port->initialize = FALSE;

    p_db_dot1x_port->radius_msg_id = INVALID_RADIUS_MSG_ID;

    p_db_dot1x_port->current_radius = NULL;

    p_db_dot1x_port->retry = 0;

    rc = dot1x_port_clear_dead_radius();
    
    return rc;
}

int32
dot1x_port_up(tbl_dot1x_port_t *p_db_dot1x_port)
{
    tbl_dot1x_global_t    *p_db_glb = NULL;
    
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PORT[%s]: Up the port", get_intf_print_name(p_db_dot1x_port->name));
        
    p_db_dot1x_port->portEnabled = TRUE;

    dot1x_port_init(p_db_dot1x_port);

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (p_db_glb)
    {
        dot1x_set_port_mode(p_db_glb, p_db_dot1x_port);
    }

    if (!p_db_dot1x_port->t_timer)
    {
        p_db_dot1x_port->t_timer = ctc_task_add_timer(TASK_PRI_NORMAL, dot1x_port_timer_fn, 
                    p_db_dot1x_port, DOT1X_PORT_TIMER_INTERVAL); 
    }
    
    return PM_E_NONE;
}

int32
dot1x_port_down(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PORT[%s]: Down the port", get_intf_print_name(p_db_dot1x_port->name));
        
    p_db_dot1x_port->portEnabled = FALSE;

    dot1x_run_sm(p_db_dot1x_port);

    if (p_db_dot1x_port->t_timer)
    {
        ctc_task_delete_timer(p_db_dot1x_port->t_timer);
        p_db_dot1x_port->t_timer = NULL;
    }

    return PM_E_NONE;
}

int32 dot1x_port_report_link_state(tbl_interface_t *p_if)
{
    tbl_dot1x_port_t        dot1x_port;
    tbl_dot1x_port_t        *p_db_dot1x_port = NULL;

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    
    dot1x_port.key.ifindex = p_if->ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "802.1x not configured on this interface!");
        return PM_E_NONE;
    }
    
    if (tbl_interface_is_running(p_if))
    {
        dot1x_port_up(p_db_dot1x_port);
    }
    else
    {
        dot1x_api_user_off_line_whole_port(p_db_dot1x_port);
        dot1x_port_down(p_db_dot1x_port);
    }
    
    return PM_E_NONE;
}

void
dot1x_set_fea_port_mac_enable(tbl_dot1x_port_t *p_db_dot1x_port)
{
    int32                 rc = PM_E_NONE;

    rc = tbl_dot1x_port_set_dot1x_port_field(p_db_dot1x_port, TBL_DOT1X_PORT_FLD_AUTH_MODE);
    if (rc)
    {
        DOT1X_LOG_ERR("dot1x_set_fea_port_mac_enable set TBL_DOT1X_PORT_FLD_AUTH_MODE return %d", rc);
        return;     
    }
    
    return;
}

void
dot1x_set_fea_port_mac_disable(tbl_dot1x_port_t *p_db_dot1x_port)
{
    int32                 rc = PM_E_NONE;

    p_db_dot1x_port->auth_mode = AUTH_PORT_MODE_PORT;
    p_db_dot1x_port->mac_auth_bypass = FALSE;

    rc = tbl_dot1x_port_set_dot1x_port_field(p_db_dot1x_port, TBL_DOT1X_PORT_FLD_AUTH_MODE);
    if (rc)
    {
        DOT1X_LOG_ERR("dot1x_set_fea_port_mac_disable set TBL_DOT1X_PORT_FLD_AUTH_MODE return %d", rc);
        return;     
    }

    rc = tbl_dot1x_port_set_dot1x_port_field(p_db_dot1x_port, TBL_DOT1X_PORT_FLD_MAC_AUTH_BYPASS);
    if (rc)
    {
        DOT1X_LOG_ERR("dot1x_set_fea_port_mac_disable set TBL_DOT1X_PORT_FLD_MAC_AUTH_BYPASS return %d", rc);
        return;     
    }
    
    return;
}

void dot1x_set_fea_mode(tbl_dot1x_port_t *p_db_dot1x_port)
{
    if(AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
    {
        dot1x_set_fea_port_authorize(p_db_dot1x_port);        
        dot1x_set_fea_port_mac_enable(p_db_dot1x_port);

        /* add port to guest vlan */
        p_db_dot1x_port->guest_vlan_valid = TRUE; 
        dot1x_set_fea_mac_guest_vlan(p_db_dot1x_port); 
    }
    /*remove mac*/
    else if(AUTH_PORT_MODE_PORT == p_db_dot1x_port->auth_mode)
    {
        dot1x_api_user_off_line_whole_port(p_db_dot1x_port);
        dot1x_set_fea_port_mac_disable(p_db_dot1x_port); 

        if (AUTH_PORT_CTRL_FORCE_AUTHORIZED != p_db_dot1x_port->authControlledPortControl)
        {
            dot1x_set_fea_port_unauthorize(p_db_dot1x_port);
            dot1x_run_sm(p_db_dot1x_port);
        }
    }
    
    return;
}

void dot1x_unset_fea_mode(tbl_dot1x_port_t *p_db_dot1x_port)
{
    dot1x_api_user_off_line_whole_port(p_db_dot1x_port);

    dot1x_set_fea_port_mac_disable(p_db_dot1x_port);
    
    return;
}

void dot1x_set_fea_add_mac(tbl_dot1x_mac_t *p_dot1x_mac)
{
    tbl_dot1x_mac_t        *p_db_dot1x_mac = NULL;
    tbl_dot1x_mac_t        dot1x_mac;
    int32                  rc = PM_E_NONE;
    int32                  field_id = 0;

    sal_memset(&dot1x_mac, 0, sizeof(tbl_dot1x_mac_t));

    sal_memcpy(&dot1x_mac.key, &p_dot1x_mac->key, sizeof(tbl_dot1x_mac_key_t));
    p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&dot1x_mac.key);
    if (!p_db_dot1x_mac)
    {
        rc = tbl_dot1x_mac_add_dot1x_mac(&dot1x_mac);
        if (rc)
        {
            DOT1X_LOG_ERR("Create dot1x mac %2x:%2x:%2x:%2x:%2x:%2x ifindex %d failure",
                dot1x_mac.key.mac[0], dot1x_mac.key.mac[1], dot1x_mac.key.mac[2],
                dot1x_mac.key.mac[3], dot1x_mac.key.mac[4], dot1x_mac.key.mac[5], dot1x_mac.key.ifindex);
            return;
        }
    }

    if (GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT))
    {
        field_id = TBL_DOT1X_MAC_FLD_AUTH_MAC_ACCEPT;
    }
    else if (GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT))
    {
        field_id = TBL_DOT1X_MAC_FLD_AUTH_MAC_REJECT;
    }
    else if (GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT))
    {
        field_id = TBL_DOT1X_MAC_FLD_AUTH_MAC_REAUTH_ACCEPT;
    }
    else
    {
        /*do nothing to other flags*/
        return;
    } 

    rc = tbl_dot1x_mac_set_dot1x_mac_field(p_dot1x_mac, field_id);
    if (rc)
    {
        DOT1X_LOG_ERR("Set dot1x control dir failed");
    }

    return;
}

void dot1x_set_fea_delete_mac(tbl_dot1x_mac_t *p_dot1x_mac)
{
    tbl_dot1x_mac_t        *p_db_dot1x_mac = NULL;
    tbl_dot1x_mac_t        dot1x_mac;

    sal_memset(&dot1x_mac, 0, sizeof(tbl_dot1x_mac_t));

    sal_memcpy(&dot1x_mac.key, &p_dot1x_mac->key, sizeof(tbl_dot1x_mac_key_t));
    p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&dot1x_mac.key);
    if (!p_db_dot1x_mac)
    {       
        DOT1X_LOG_ERR("Delete dot1x mac %2x:%2x:%2x:%2x:%2x:%2x ifindex %d failure because it is not exist",
            dot1x_mac.key.mac[0], dot1x_mac.key.mac[1], dot1x_mac.key.mac[2],
            dot1x_mac.key.mac[3], dot1x_mac.key.mac[4], dot1x_mac.key.mac[5], dot1x_mac.key.ifindex);
        
        return;        
    }

    return;
}

uint32 
dot1x_allocate_session_id(void)
{
    tbl_dot1x_global_t *p_db_glb = NULL;
    uint32             i = 0;

    /* check exist */
    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        return RADIUS_SESSION_BUFFER;
    }

    if (p_db_glb->last_session_id < RADIUS_SESSION_BUFFER - 1)
    {
        p_db_glb->last_session_id++;    
    }
    else
    {
        p_db_glb->last_session_id = 0;
    }

    for (i = p_db_glb->last_session_id; i <= RADIUS_SESSION_BUFFER; i++)
    {
        if (!p_db_glb->session_id_allocated[i])
        {
            p_db_glb->session_id_allocated[i] = TRUE;
            p_db_glb->last_session_id = i;
            return i;
        }
    }
    
    return RADIUS_SESSION_BUFFER;  
}

void 
dot1x_free_session(uint32 session_id)
{
    tbl_dot1x_global_t *p_db_glb = NULL;
     
    /* check exist */
    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        return;
    }

    if (session_id >= RADIUS_SESSION_BUFFER)
    {
        return;
    }

    p_db_glb->session_id_allocated[session_id] = FALSE;

    return;  
}


