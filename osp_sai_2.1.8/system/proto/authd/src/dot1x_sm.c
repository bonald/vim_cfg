/****************************************************************************
*  dot1x state machine process
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
#include "dot1x.h"
#include "dot1x_api.h"
#include "dot1x_packet.h"

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

/* Figure 8-11 */
void
dot1x_reauth_initialize (tbl_dot1x_port_t *p_db_dot1x_port)
{
    p_db_dot1x_port->reAuthWhen = p_db_dot1x_port->reAuthPeriod;

    return;
}

void
dot1x_reauth_reauthenticate (tbl_dot1x_port_t *p_db_dot1x_port)
{
    p_db_dot1x_port->reAuthenticate = TRUE;

    dot1x_reauth_initialize (p_db_dot1x_port);

    return;
}

/* Execute the Reauthentication Timer state machine transitions */
void
dot1x_reauth_sm (tbl_dot1x_port_t *p_db_dot1x_port)
{
    if(AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
    {
        return;
    }
    
    if ((AUTH_PORT_CTRL_AUTO != p_db_dot1x_port->portControl) 
      || p_db_dot1x_port->initialize 
      || (AUTH_PORT_STATUS_UNAUTHORIZED == p_db_dot1x_port->portStatus)
      || (!p_db_dot1x_port->reAuthEnabled))
    {
        dot1x_reauth_initialize (p_db_dot1x_port);
    }
    else if (0 == p_db_dot1x_port->reAuthWhen)
    {
        dot1x_reauth_reauthenticate (p_db_dot1x_port);
    }

    return;
}

void dot1x_reauth_timer_fn(void *p_data)
{
    tbl_dot1x_mac_t      *p_db_dot1x_mac = NULL;
    tbl_dot1x_port_t     *p_db_dot1x_port = NULL;
    tbl_dot1x_port_t     dot1x_port;

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));

    p_db_dot1x_mac = (tbl_dot1x_mac_t*)p_data;
    if (!p_db_dot1x_mac)
    {
        DOT1X_LOG_ERR("dot1x_reauth_timer_fn dot1x mac is null when reauth timer timeout\n");
        return;
    }

    p_db_dot1x_mac->t_reauth = NULL;

    dot1x_port.key.ifindex = p_db_dot1x_mac->key.ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        DOT1X_LOG_ERR("dot1x_reauth_timer_fn dot1x port is null ");
        return;
    }
    
    if (!p_db_dot1x_port->reAuthEnabled)
    {
        return;
    }
    
    if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_BYPASS)
        && GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT))
    {
        GLB_UNSET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT);
        GLB_SET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT);

        dot1x_set_fea_add_mac(p_db_dot1x_mac); 

        p_db_dot1x_mac->t_reauth= ctc_task_add_timer(TASK_PRI_NORMAL, dot1x_reauth_timer_fn, 
                    p_db_dot1x_mac, AUTH_MAC_ENTRY_WAITING_TIME); 
    }
    else if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_BYPASS)
        && GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT))
    {
        dot1x_api_user_off_line(p_db_dot1x_port, p_db_dot1x_mac);
    }
    else if (!GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_BYPASS))
    {
        if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_EAPOL_REAUTH))
        {
            dot1x_api_user_off_line(p_db_dot1x_port, p_db_dot1x_mac);
            return;
        }

        GLB_SET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_EAPOL_REAUTH);
        p_db_dot1x_port->reAuthenticate = TRUE;

        sal_memcpy(p_db_dot1x_port->dest_mac, p_db_dot1x_mac->key.mac, sizeof(mac_addr_t));

        p_db_dot1x_mac->t_reauth= ctc_task_add_timer(TASK_PRI_NORMAL, dot1x_reauth_timer_fn, 
                    p_db_dot1x_mac, AUTH_MAC_ENTRY_WAITING_TIME);
    }     

    return;
}

void dot1x_reject_timer_fn(void *p_data)
{
    tbl_dot1x_mac_t       *p_db_dot1x_mac = NULL;
    tbl_dot1x_port_t      *p_db_dot1x_port = NULL;
    tbl_dot1x_port_t      dot1x_port;

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));

    p_db_dot1x_mac = (tbl_dot1x_mac_t*)p_data;
    if (!p_db_dot1x_mac)
    {
        DOT1X_LOG_ERR("dot1x_reject_timer_fn dot1x mac is null when reauth timer timeout\n");
        return;
    }

    p_db_dot1x_mac->t_reject = NULL;

    dot1x_port.key.ifindex = p_db_dot1x_mac->key.ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        DOT1X_LOG_ERR("dot1x_reauth_proc_timer_fn dot1x port is null ");
        return;
    }

    if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT))
    {
        dot1x_api_user_off_line(p_db_dot1x_port, p_db_dot1x_mac);
    }

    return;
}

static void
dot1x_key_xmit_state(tbl_dot1x_port_t *p_db_dot1x_port)
{
    /* txKey.  
      these codes are invalid, key is not initialized */
    //auth_send_eapol_key (port, &key);

    p_db_dot1x_port->auth_key_xmit_keyAvailable = 0;
    p_db_dot1x_port->auth_key_xmit_state = AUTH_KEY_XMIT;

    return;
}

static void
dot1x_key_no_xmit_state(tbl_dot1x_port_t *p_db_dot1x_port)
{  
    p_db_dot1x_port->auth_key_xmit_state = AUTH_NO_KEY_XMIT;

    return;
}

/* Perform authentication key transmit state machine transitions */
void
dot1x_key_xmit_sm(tbl_dot1x_port_t *p_db_dot1x_port)
{
    if ((p_db_dot1x_port->initialize) 
        || (p_db_dot1x_port->portControl != AUTH_PORT_CTRL_AUTO))
    { 
        dot1x_key_no_xmit_state(p_db_dot1x_port);
    }
    else if (AUTH_NO_KEY_XMIT == p_db_dot1x_port->auth_key_xmit_state)
    {
        if (p_db_dot1x_port->auth_key_xmit_keyTxEnabled 
            && p_db_dot1x_port->auth_key_xmit_keyAvailable
            && p_db_dot1x_port->auth_key_xmit_KeyRun)
        {
            dot1x_key_xmit_state(p_db_dot1x_port);
        }
    }
    else if (AUTH_KEY_XMIT == p_db_dot1x_port->auth_key_xmit_state)
    {
        if (p_db_dot1x_port->auth_key_xmit_keyAvailable)
        {
            dot1x_key_xmit_state(p_db_dot1x_port);
        }
        else if (!p_db_dot1x_port->auth_key_xmit_keyTxEnabled  
            || !p_db_dot1x_port->auth_key_xmit_KeyRun)
        {
            dot1x_key_no_xmit_state(p_db_dot1x_port);
        }
    }

    return;
}

static void
dot1x_force_both(tbl_dot1x_port_t *p_db_dot1x_port)
{
    p_db_dot1x_port->auth_ctrl_dir_operControlledDirections = AUTH_CTRL_DIR_BOTH;

    /* Notify FEA the control direction.  */
    dot1x_set_fea_port_ctrl(p_db_dot1x_port);

    return;
}

void
dot1x_in_or_both(tbl_dot1x_port_t *p_db_dot1x_port)
{
    p_db_dot1x_port->auth_ctrl_dir_operControlledDirections
        = p_db_dot1x_port->auth_ctrl_dir_adminControlledDirections;

    /* Notify NSM the control direction.  */
    dot1x_set_fea_port_ctrl(p_db_dot1x_port);

    return;
}

void
dot1x_ctrl_dir_sm(tbl_dot1x_port_t *p_db_dot1x_port)
{
    if (p_db_dot1x_port->initialize) 
    {
        dot1x_in_or_both(p_db_dot1x_port);
        p_db_dot1x_port->auth_ctrl_dir_state = IN_OR_BOTH;
    }
    else 
    {
        switch (p_db_dot1x_port->auth_ctrl_dir_state) 
        {
        case IN_OR_BOTH:
            if (p_db_dot1x_port->auth_ctrl_dir_operControlledDirections 
                != p_db_dot1x_port->auth_ctrl_dir_adminControlledDirections) 
            {
                break;
            }
            else 
            { 
                if (!p_db_dot1x_port->portEnabled || p_db_dot1x_port->auth_ctrl_dir_bridgeDetected) 
                {
                    dot1x_force_both(p_db_dot1x_port);
                    p_db_dot1x_port->auth_ctrl_dir_state = FORCE_BOTH;
                }
            }
            break;

        case FORCE_BOTH:
            if (p_db_dot1x_port->portEnabled && !p_db_dot1x_port->auth_ctrl_dir_bridgeDetected) 
            {
                dot1x_in_or_both(p_db_dot1x_port);
                p_db_dot1x_port->auth_ctrl_dir_state = IN_OR_BOTH;
            }
            break;

        default:
            break;
        }
    }

    return;
}

/* 802.1X-2001 8.5.8.8 IDLE.  */
void
dot1x_be_idle(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "BE[%s]: State IDLE", get_intf_print_name(p_db_dot1x_port->name));
 
    p_db_dot1x_port->auth_be_state = AUTH_BE_STATE_IDLE;
    p_db_dot1x_port->authStart = FALSE;
    p_db_dot1x_port->auth_be_reqCount = 0;

    return;
}

/* 802.1X-2001 8.5.8.3 REQUEST.  */
void
dot1x_be_request(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "BE[%s]: State REQUEST", get_intf_print_name(p_db_dot1x_port->name));
    DOT1X_LOG_DEBUG(DOT1X_FLAG_TIMER, "[%s]: Start supp-timeout timer %d", get_intf_print_name(p_db_dot1x_port->name), 
        p_db_dot1x_port->auth_be_suppTimeout);

    if (AUTH_BE_STATE_RESPONSE == p_db_dot1x_port->auth_be_state)
    {
        p_db_dot1x_port->currentId = p_db_dot1x_port->auth_be_idFromServer;
        dot1x_send_eapol_req(p_db_dot1x_port);
    }

    p_db_dot1x_port->eapReq = FALSE;
    p_db_dot1x_port->aWhile = p_db_dot1x_port->auth_be_suppTimeout;
    p_db_dot1x_port->auth_be_backend_otherrequest_to_supplicant++;
    p_db_dot1x_port->auth_be_state = AUTH_BE_STATE_REQUEST;

    return;
}

/* 802.1X-2001 8.5.8.4 RESPONSE.  */
void
dot1x_be_response(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "BE[%s]: State RESPONSE", get_intf_print_name(p_db_dot1x_port->name));
    DOT1X_LOG_DEBUG(DOT1X_FLAG_TIMER, "[%s]: Start server-timeout timer %d", get_intf_print_name(p_db_dot1x_port->name), 
        p_db_dot1x_port->auth_be_serverTimeout);

    p_db_dot1x_port->auth_be_state = AUTH_BE_STATE_RESPONSE;
    p_db_dot1x_port->eapReq = FALSE;
    p_db_dot1x_port->eapSuccess = FALSE;
    p_db_dot1x_port->eapNoReq = FALSE;
    p_db_dot1x_port->authTimeout = FALSE;
    p_db_dot1x_port->eapResp = FALSE;
    p_db_dot1x_port->aWhile =  p_db_dot1x_port->auth_be_serverTimeout;
    p_db_dot1x_port->auth_be_reqCount = 0;
  
    /* 802.1X-2001 Section 8.5.8.1.3.b) sendRespToServer.  */
    dot1x_send_radius_eap(p_db_dot1x_port);
  
    p_db_dot1x_port->auth_be_backend_responses++;

    return;
}

/* 802.1X-2001 8.5.8.5 SUCCESS.  */
void
dot1x_be_success(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "BE[%s]: State SUCCESS", get_intf_print_name(p_db_dot1x_port->name));

    p_db_dot1x_port->auth_be_state = AUTH_BE_STATE_SUCCESS;
    p_db_dot1x_port->currentId = p_db_dot1x_port->auth_be_idFromServer;

    p_db_dot1x_port->auth_be_backend_auth_success++;

    /* 802.1X-2001 Section 8.5.8.1.3.c) txCannedSuccess.  */
    dot1x_send_eapol_success(p_db_dot1x_port);

    p_db_dot1x_port->authSuccess = TRUE;
    p_db_dot1x_port->auth_key_xmit_KeyRun = TRUE;
    
    dot1x_be_idle(p_db_dot1x_port);

    return;
}

/* 802.1X-2001 8.5.8.6 FAIL.  */
void
dot1x_be_fail(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "BE[%s]: State FAIL", get_intf_print_name(p_db_dot1x_port->name));

    p_db_dot1x_port->auth_be_state = AUTH_BE_STATE_FAIL;
    p_db_dot1x_port->currentId = p_db_dot1x_port->auth_be_idFromServer;

    p_db_dot1x_port->auth_be_backend_auth_fails++;

    /* 802.1X-2001 Section 8.5.8.1.3.d) txCannedFail.  */
    dot1x_send_eapol_fail(p_db_dot1x_port);

    p_db_dot1x_port->authFail = TRUE;
    dot1x_be_idle(p_db_dot1x_port);

    return;
}

/* 802.1X-2001 8.5.8.7 TIMEOUT.  */
void
dot1x_be_timeout(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "BE[%s]: State TIMEOUT", get_intf_print_name(p_db_dot1x_port->name));

    p_db_dot1x_port->auth_be_state = AUTH_BE_STATE_TIMEOUT;

    p_db_dot1x_port->authTimeout = TRUE;

    /* Delete the port from the RADIUS server association.  */
    dot1x_radius_end_association(p_db_dot1x_port);
    dot1x_delete_session(p_db_dot1x_port->key.ifindex);
    p_db_dot1x_port->current_radius = NULL;

    dot1x_be_idle(p_db_dot1x_port);

    return;
}

/* 802.1X-2001 8.5.8.1.3.e */
void
dot1x_be_abortAuth(tbl_dot1x_port_t *p_db_dot1x_port)
{
    tbl_dot1x_global_t *p_db_glb = NULL;

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        DOT1X_LOG_ERR("Tbl_dot1x_global not found");
        return;
    }

    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "BE[%s]: abortAuth", get_intf_print_name(p_db_dot1x_port->name));

    sal_memset(p_db_dot1x_port->user_name, 0, AUTH_USER_NAMLEN);

    sal_memcpy(p_db_dot1x_port->dest_mac, p_db_glb->eapol_group_address, sizeof(mac_addr_t));

    return;
}

void
dot1x_be_ignore(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "BE[%s]: state IGNORE", get_intf_print_name(p_db_dot1x_port->name));
    DOT1X_LOG_DEBUG(DOT1X_FLAG_TIMER, "[%s]: Start supp-timeout timer %d", get_intf_print_name(p_db_dot1x_port->name), 
        p_db_dot1x_port->auth_be_suppTimeout);

    p_db_dot1x_port->auth_be_state = AUTH_BE_STATE_IGNORE;
    p_db_dot1x_port->eapNoReq = FALSE;
    p_db_dot1x_port->aWhile = p_db_dot1x_port->auth_be_suppTimeout;

    return;
}

/* 802.1X-2001 8.5.8.9 INITIALIZE.  */
void
dot1x_be_initialize(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "BE[%s]: State INITIALIZE", get_intf_print_name(p_db_dot1x_port->name));

    p_db_dot1x_port->auth_be_state = AUTH_BE_STATE_INITIALIZE;
    dot1x_be_abortAuth(p_db_dot1x_port);
    p_db_dot1x_port->authAbort = FALSE;
    p_db_dot1x_port->eapNoReq = FALSE;

    dot1x_be_idle(p_db_dot1x_port);

    return;
}

/* 802.1X-2001 8.5.8 Backend Authentication State Machine.  */
void
dot1x_be_sm(tbl_dot1x_port_t *p_db_dot1x_port)
{
    /* Initialize */
    /* The spec says to reinitialize if portControl != auto
     in the state machine diagram (Figure 8-12),
     but does not contain this condition in the text (Section 8.5.8.9).
     If we follow the digram, then setting portControl to forceUnauthorized
     causes auth_be_initialize to be called every second. This is not
     a good thing!
     */

    if (p_db_dot1x_port->initialize || p_db_dot1x_port->authAbort)
    {
        dot1x_be_initialize(p_db_dot1x_port);
    }
    else
    {
        switch(p_db_dot1x_port->auth_be_state)
        {
        case AUTH_BE_STATE_REQUEST:
            if (p_db_dot1x_port->eapResp)
            {
                p_db_dot1x_port->auth_be_backend_nonnak_responses_from_supplicant++; 
                dot1x_be_response(p_db_dot1x_port);
            }
            else if (p_db_dot1x_port->eapReq)
            {
                dot1x_be_request(p_db_dot1x_port);
            }
            else if (0 == p_db_dot1x_port->aWhile)
            {
                dot1x_be_timeout(p_db_dot1x_port);
            }
            break;

        case AUTH_BE_STATE_RESPONSE:
            if (0 == p_db_dot1x_port->aWhile)
            {
                dot1x_be_timeout(p_db_dot1x_port);
            }
            else if (p_db_dot1x_port->eapFail)
            {
                dot1x_be_fail(p_db_dot1x_port);
            }
            else if (p_db_dot1x_port->eapSuccess)
            {
                dot1x_be_success(p_db_dot1x_port);
            }
            else if (p_db_dot1x_port->eapNoReq)
            {
                dot1x_be_ignore(p_db_dot1x_port);
            }
            else if (p_db_dot1x_port->eapReq)
            {
                p_db_dot1x_port->auth_be_backend_access_challenges++;
                dot1x_be_request(p_db_dot1x_port);
            }
            break;

        case AUTH_BE_STATE_SUCCESS: 
        case AUTH_BE_STATE_FAIL:
        case AUTH_BE_STATE_TIMEOUT:
        case AUTH_BE_STATE_INITIALIZE:
            /* This really shouldn't happen as 
               the states transition themselves on UCT. */
            dot1x_be_idle(p_db_dot1x_port);
            break;
            
        case AUTH_BE_STATE_IDLE:
            if (p_db_dot1x_port->authStart && p_db_dot1x_port->eapFail)
            {
                dot1x_be_fail(p_db_dot1x_port);
            }
            else if (p_db_dot1x_port->authStart && p_db_dot1x_port->eapSuccess)
            {
                dot1x_be_success(p_db_dot1x_port);
            }
            else if (p_db_dot1x_port->authStart && p_db_dot1x_port->eapReq)
            {
                dot1x_be_request(p_db_dot1x_port);
            }
            break;
            
        case AUTH_BE_STATE_INVALID:
        case AUTH_BE_STATE_IGNORE:
            if (p_db_dot1x_port->eapResp)
            {
                dot1x_be_response(p_db_dot1x_port);
            }
            else if (p_db_dot1x_port->eapReq)
            {
                dot1x_be_request(p_db_dot1x_port);
            }
            break;

        default:
            dot1x_be_initialize(p_db_dot1x_port);
            break;
        }
    }

    return;
}

void
dot1x_port_restart(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "port[%s]:Higher layer restart", get_intf_print_name(p_db_dot1x_port->name));

    if (p_db_dot1x_port->eaprestart)
    {
        dot1x_radius_end_association(p_db_dot1x_port);
        dot1x_delete_session(p_db_dot1x_port->key.ifindex);
        p_db_dot1x_port->current_radius = NULL;
        
        p_db_dot1x_port->eaprestart = FALSE;
        p_db_dot1x_port->eapSuccess = FALSE;
        p_db_dot1x_port->eapFail = FALSE;
    }

    return;
}

void
dot1x_port_authorize(tbl_dot1x_port_t *p_db_dot1x_port)
{
    if (AUTH_PORT_STATUS_AUTHORIZED == p_db_dot1x_port->portStatus)
    {
        p_db_dot1x_port->session_terminate_cause = AUTH_NOT_TERMINATED_YET;
        return;
    }

    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PORT[%s]: Authorize the port", get_intf_print_name(p_db_dot1x_port->name));

    if (p_db_dot1x_port->guest_vlan_valid)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PORT[%s]: portAuthorized, leave the guest vlan %d", get_intf_print_name(p_db_dot1x_port->name), 
            p_db_dot1x_port->guest_vlan);

        p_db_dot1x_port->guest_vlan_valid = FALSE;
        dot1x_set_fea_port_guest_vlan(p_db_dot1x_port);
    }

    /* Set the port state.  */
    p_db_dot1x_port->portStatus = AUTH_PORT_STATUS_AUTHORIZED;
  
    /* Notify NSM the port is authorized.  */
    dot1x_set_fea_port_authorize(p_db_dot1x_port);
  
    /* 9.4.4.1.3.h */
    p_db_dot1x_port->session_time = 0;
    p_db_dot1x_port->session_terminate_cause = AUTH_NOT_TERMINATED_YET;
  
    return;
}

void
dot1x_port_unauthorize(tbl_dot1x_port_t *p_db_dot1x_port)
{
    if (AUTH_PORT_STATUS_UNAUTHORIZED == p_db_dot1x_port->portStatus)
    {
        return;
    }

    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PORT[%s]: Unauthorize the port", get_intf_print_name(p_db_dot1x_port->name));

    /* Set the port state.  */
    p_db_dot1x_port->portStatus = AUTH_PORT_STATUS_UNAUTHORIZED;

    /* Notify NSM the port is unauthorized */
    if (AUTH_PORT_MODE_PORT == p_db_dot1x_port->auth_mode)
    {
        dot1x_set_fea_port_unauthorize(p_db_dot1x_port);
    }

    return;
}

/* IEEE802.1x-2004 8.2.4.5 RESTART */
void
dot1x_pae_restart(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PAE[%s]: state RESTART", get_intf_print_name(p_db_dot1x_port->name));

    p_db_dot1x_port->auth_pae_state = AUTH_PAE_STATE_RESTART;

    if (p_db_dot1x_port->reAuthenticate)
    {
        p_db_dot1x_port->auth_pae_reauths_while_authenticated++; 
        p_db_dot1x_port->session_terminate_cause = AUTH_REAUTH_FAILURE;
    }
    else if (p_db_dot1x_port->auth_pae_eapStart)
    {
        p_db_dot1x_port->auth_pae_eapstart_while_authenticated++;
        p_db_dot1x_port->session_terminate_cause = AUTH_SUPPLICANT_RESTART;
    }
  
    p_db_dot1x_port->eaprestart = TRUE;
  
    dot1x_port_restart(p_db_dot1x_port);

    return;
}

/* 802.1X-2001 8.5.4.5 CONNECTING.  */
void
dot1x_pae_connecting(tbl_dot1x_port_t *p_db_dot1x_port)
{
    tbl_dot1x_mac_master_t  *p_glb_dot1x_mac = NULL;
    tbl_dot1x_mac_t         *p_db_dot1x_mac = NULL;
    ctclib_slistnode_t      *listnode = NULL; 

    p_glb_dot1x_mac = tbl_dot1x_mac_get_master();

    if (!p_glb_dot1x_mac)
    {
        DOT1X_LOG_ERR("_dot1x_cmd_set_port_reauthenticate fail because tbl_dot1x_mac is not exist");
        return;
    }
    
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PAE[%s]: State CONNECTING", get_intf_print_name(p_db_dot1x_port->name));

    p_db_dot1x_port->auth_pae_state = AUTH_PAE_STATE_CONNECTING;
    p_db_dot1x_port->auth_pae_eapStart = FALSE;
    p_db_dot1x_port->reAuthenticate = FALSE;
    p_db_dot1x_port->auth_pae_rxRespId = 0;
    p_db_dot1x_port->txWhen = p_db_dot1x_port->auth_pae_txPeriod;

    DOT1X_LOG_DEBUG(DOT1X_FLAG_TIMER, "[%s]: Start tx-period timer %d", get_intf_print_name(p_db_dot1x_port->name), 
        p_db_dot1x_port->auth_pae_txPeriod);

    if (p_db_dot1x_port->share_reauth)
    {
        CTCLIB_SLIST_LOOP(p_glb_dot1x_mac->dot1x_mac_list, p_db_dot1x_mac, listnode)
        {
            if (p_db_dot1x_mac->key.ifindex == p_db_dot1x_port->key.ifindex)
            {
                sal_memcpy(p_db_dot1x_port->dest_mac, p_db_dot1x_mac->key.mac, sizeof(mac_addr_t));

                dot1x_send_eapol_req_id(p_db_dot1x_port);           
                INC(p_db_dot1x_port->currentId);
            }   
         }
        
        p_db_dot1x_port->share_reauth = 0;
    }
    else
    {
        p_db_dot1x_port->auth_pae_enter_connecting++;
    
        INC(p_db_dot1x_port->auth_pae_reAuthCount);   

        if (p_db_dot1x_port->auth_pae_reAuthCount <= p_db_dot1x_port->auth_pae_reAuthMax)
        {
            dot1x_send_eapol_req_id(p_db_dot1x_port);
        }
    }

    /* If the wait timer is running delete the port from 
       the RADIUS server association. */
    if (INVALID_RADIUS_MSG_ID != p_db_dot1x_port->radius_msg_id)
    {
        dot1x_radius_end_association(p_db_dot1x_port);
        dot1x_delete_session(p_db_dot1x_port->key.ifindex);
        p_db_dot1x_port->current_radius = NULL;
    }

    return;
}

/* 802.1X-2001 8.5.4.4 DISCONNECTED.  */
void
dot1x_pae_disconnected(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PAE[%s]: State DISCONNECTED", get_intf_print_name(p_db_dot1x_port->name));

    if (p_db_dot1x_port->auth_pae_eapLogoff && (AUTH_PAE_STATE_AUTHENTICATED == p_db_dot1x_port->auth_pae_state))
    {
        p_db_dot1x_port->auth_pae_eaplogoff_while_authenticated++;
        p_db_dot1x_port->session_terminate_cause = AUTH_SUPPLICANT_LOGOFF;
    }
    else
    {
        p_db_dot1x_port->auth_pae_eaplogoff_while_connecting++;
    } 

    /* When a guest VLAN is configured, clients that are not IEEE 802.1x-capable are put into the guest
        VLAN when the server does not receive a response to its EAP request/identity frame.*/
    if (!p_db_dot1x_port->auth_pae_eapLogoff 
        && (p_db_dot1x_port->auth_pae_reAuthCount > p_db_dot1x_port->auth_pae_reAuthMax)
        && p_db_dot1x_port->guest_vlan)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PORT[%s]: ReqTimeout, Add port to guest vlan(%d)", 
            get_intf_print_name(p_db_dot1x_port->name), p_db_dot1x_port->guest_vlan);
        
        p_db_dot1x_port->guest_vlan_valid = TRUE; /*port authorized by guest vlan policy*/
        dot1x_set_fea_port_guest_vlan(p_db_dot1x_port);
    }

    p_db_dot1x_port->auth_pae_state = AUTH_PAE_STATE_DISCONNECTED;
    p_db_dot1x_port->auth_pae_eapLogoff = FALSE;
    p_db_dot1x_port->auth_pae_reAuthCount = 0;

    dot1x_send_eapol_fail(p_db_dot1x_port);

    INC(p_db_dot1x_port->currentId);
 
    /* Unauthorize the port.  */
    dot1x_port_unauthorize(p_db_dot1x_port);

    /* Transition unconditionally to restart state */
    if (p_db_dot1x_port->portEnabled)
    {
        dot1x_pae_restart(p_db_dot1x_port);
    }
    else
    {
        p_db_dot1x_port->session_terminate_cause = AUTH_PORT_FAILURE;
    }

    return;
}

/* 802.1X-2001 8.5.4.10 FORCE_AUTH.  */
void
dot1x_pae_force_auth(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PAE[%s]: State FORCE AUTHORIZED", get_intf_print_name(p_db_dot1x_port->name));

    p_db_dot1x_port->auth_pae_state = AUTH_PAE_STATE_FORCE_AUTH;
    p_db_dot1x_port->auth_pae_portMode = AUTH_PORT_CTRL_FORCE_AUTHORIZED;
    p_db_dot1x_port->auth_pae_eapStart = FALSE;

    p_db_dot1x_port->session_terminate_cause = AUTH_PORT_ADMIN_DISABLE;

    /* Authenticate the port.  */
    dot1x_port_authorize(p_db_dot1x_port);

    /* txCannedSuccess.  */
    dot1x_send_eapol_success(p_db_dot1x_port);

    INC(p_db_dot1x_port->currentId);

    /*should disable guest vlan , for authed by radius server
      or no dot1x system auth ctrl. force unauth will be controled in api function auth_port_ctrl_set()*/  
    p_db_dot1x_port->guest_vlan_valid = FALSE; 
    dot1x_set_fea_port_guest_vlan(p_db_dot1x_port);

    return;
} 

/* 802.1X-2001 8.5.4.11 FORCE_UNAUTH.  */
void
dot1x_pae_force_unauth(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PAE[%s]: State FORCE UNAUTHORIZED", get_intf_print_name(p_db_dot1x_port->name));

    p_db_dot1x_port->auth_pae_state = AUTH_PAE_STATE_FORCE_UNAUTH;
    p_db_dot1x_port->auth_pae_portMode = AUTH_PORT_CTRL_FORCE_UNAUTHORIZED;
    p_db_dot1x_port->auth_pae_eapStart = FALSE;
  
    if (AUTH_PORT_STATUS_AUTHORIZED == p_db_dot1x_port->portStatus)
    {
        p_db_dot1x_port->session_terminate_cause = AUTH_SET_TO_FORCE_UNAUTHORIZED;
    }
    
    /* txCannedFail.  */
    dot1x_send_eapol_fail(p_db_dot1x_port);
  
    INC(p_db_dot1x_port->currentId);
  
    /* Unauthenticate the port.  */
    dot1x_port_unauthorize(p_db_dot1x_port);

    return;
}

/* 802.1X-2001 8.5.4.3 INITIALIZE.  */
void
dot1x_pae_initialize(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PAE[%s]: State INITIALIZE", get_intf_print_name(p_db_dot1x_port->name));

    p_db_dot1x_port->auth_pae_state = AUTH_PAE_STATE_INIT;
    p_db_dot1x_port->currentId = 0;
    p_db_dot1x_port->auth_pae_portMode = AUTH_PORT_CTRL_AUTO;

    /* when port initialize, should leave the guest vlan first*/
    if ((AUTH_PORT_STATUS_UNAUTHORIZED == p_db_dot1x_port->portStatus)
        && (p_db_dot1x_port->guest_vlan_valid)&& (p_db_dot1x_port->guest_vlan))
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PORT[%s]: paeInit, Remove port from guest vlan(%d)", get_intf_print_name(p_db_dot1x_port->name), 
            p_db_dot1x_port->guest_vlan);
        
        p_db_dot1x_port->guest_vlan_valid = FALSE; /*guest vlan invalid*/ 
        dot1x_set_fea_port_guest_vlan(p_db_dot1x_port);   
    }

    if (AUTH_PORT_CTRL_AUTO == p_db_dot1x_port->portControl)
    {
        dot1x_pae_disconnected(p_db_dot1x_port);
    }
    else if (AUTH_PORT_CTRL_FORCE_AUTHORIZED == p_db_dot1x_port->portControl)
    {
        dot1x_pae_force_auth(p_db_dot1x_port);
    }
    else if (AUTH_PORT_CTRL_FORCE_UNAUTHORIZED == p_db_dot1x_port->portControl)
    {
        dot1x_pae_force_unauth(p_db_dot1x_port);
    }

    return;
}

/* 802.1X-2001 8.5.4.6 AUTHENTICATING.  */
void
dot1x_pae_authenticating(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PAE[%s]: State AUTHENTICATING", get_intf_print_name(p_db_dot1x_port->name));

    p_db_dot1x_port->auth_pae_state = AUTH_PAE_STATE_AUTHENTICATING;
    p_db_dot1x_port->authSuccess = FALSE;
    p_db_dot1x_port->authFail = FALSE;
    p_db_dot1x_port->authTimeout = FALSE;
    p_db_dot1x_port->authStart = TRUE;
    p_db_dot1x_port->auth_key_xmit_KeyRun = FALSE;
    p_db_dot1x_port->auth_key_xmit_KeyDone = FALSE;
    p_db_dot1x_port->auth_pae_enter_authenticating++;

    return;
}

/* 802.1X-2001 8.5.4.7 AUTHENTICATED.  */
void
dot1x_pae_authenticated(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PAE[%s]: State AUTHENTICATED", get_intf_print_name(p_db_dot1x_port->name));

    p_db_dot1x_port->auth_pae_state = AUTH_PAE_STATE_AUTHENTICATED;
    p_db_dot1x_port->auth_pae_reAuthCount = 0;

    INC(p_db_dot1x_port->currentId);

    p_db_dot1x_port->guest_vlan_valid = FALSE;
    dot1x_set_fea_port_guest_vlan(p_db_dot1x_port); 

    /* Authorize the port.  */
    dot1x_port_authorize(p_db_dot1x_port);

    p_db_dot1x_port->auth_pae_success_while_authenticating++;

    return;
}

/* 802.1X-2001 8.5.4.8 ABORTING.  */
void
dot1x_pae_aborting(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PAE[%s]: State ABORTING", get_intf_print_name(p_db_dot1x_port->name));

    if (p_db_dot1x_port->auth_pae_eapStart)
    {
        p_db_dot1x_port->auth_pae_eapstart_while_authenticating++;
    }
    else if (p_db_dot1x_port->auth_pae_eapLogoff)
    {
        p_db_dot1x_port->auth_pae_eaplogoff_while_authenticating++;
    }
    else if (p_db_dot1x_port->authTimeout)
    {
        p_db_dot1x_port->auth_pae_timeout_while_authenticating++;
    }
    else if (p_db_dot1x_port->reAuthenticate)
    {
        p_db_dot1x_port->auth_pae_reauths_while_authenticating++;
    }

    p_db_dot1x_port->auth_pae_state = AUTH_PAE_STATE_ABORTING;
    p_db_dot1x_port->authAbort = TRUE;

    INC(p_db_dot1x_port->currentId);

    p_db_dot1x_port->auth_key_xmit_KeyRun = FALSE;
    p_db_dot1x_port->auth_key_xmit_KeyDone = FALSE;

    return;
}

/* 802.1X-2001 8.5.4.9 HELD.  */
void
dot1x_pae_held(tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_EVENTS, "PAE[%s]: State HELD", get_intf_print_name(p_db_dot1x_port->name));
    DOT1X_LOG_DEBUG(DOT1X_FLAG_TIMER, "[%s]: Start quiet-period timer %d", get_intf_print_name(p_db_dot1x_port->name), p_db_dot1x_port->auth_pae_quietPeriod);

    p_db_dot1x_port->auth_pae_state = AUTH_PAE_STATE_HELD;
    p_db_dot1x_port->quietWhile = p_db_dot1x_port->auth_pae_quietPeriod;
    p_db_dot1x_port->auth_pae_eapLogoff = FALSE;

    INC(p_db_dot1x_port->currentId);

    /* Unauthorize the port.  */
    dot1x_port_unauthorize(p_db_dot1x_port);

    p_db_dot1x_port->auth_pae_fail_while_authenticating++;

    return;
}

/* 802.1X-2001 8.5.4 Authenticator PAE state machine.  */
void
dot1x_pae_sm(tbl_dot1x_port_t *p_db_dot1x_port)
{
    if (p_db_dot1x_port->initialize || !p_db_dot1x_port->portEnabled)
    {
        dot1x_pae_initialize(p_db_dot1x_port);
    }
    else if (p_db_dot1x_port->auth_pae_portMode != p_db_dot1x_port->portControl)
    {      
        if (AUTH_PORT_CTRL_AUTO == p_db_dot1x_port->portControl)
        {
            dot1x_pae_initialize(p_db_dot1x_port);
        }
        else if (AUTH_PORT_CTRL_FORCE_AUTHORIZED == p_db_dot1x_port->portControl)
        {
            dot1x_pae_force_auth(p_db_dot1x_port);
        }
        else if (AUTH_PORT_CTRL_FORCE_UNAUTHORIZED == p_db_dot1x_port->portControl)
        {
            dot1x_pae_force_unauth(p_db_dot1x_port);
        }
    }
    else
    {
        switch (p_db_dot1x_port->auth_pae_state)
        {
        case AUTH_PAE_STATE_DOWN:
        case AUTH_PAE_STATE_INIT:
        case AUTH_PAE_STATE_DISCONNECTED:
            dot1x_pae_initialize(p_db_dot1x_port);
            break;

        case AUTH_PAE_STATE_CONNECTING:
            if (p_db_dot1x_port->eapReq
                && p_db_dot1x_port->auth_pae_reAuthCount <= p_db_dot1x_port->auth_pae_reAuthMax)
            {
                dot1x_pae_authenticating(p_db_dot1x_port);
            }
            else if (p_db_dot1x_port->auth_pae_eapLogoff
                || p_db_dot1x_port->auth_pae_reAuthCount > p_db_dot1x_port->auth_pae_reAuthMax)
            {
                dot1x_pae_disconnected(p_db_dot1x_port);
            }
            else if (((0 == p_db_dot1x_port->txWhen) 
                || p_db_dot1x_port->auth_pae_eapStart || p_db_dot1x_port->reAuthenticate)
                && (p_db_dot1x_port->auth_pae_reAuthCount <= p_db_dot1x_port->auth_pae_reAuthMax))
            {
                dot1x_pae_connecting(p_db_dot1x_port);
            }
            break;


        case AUTH_PAE_STATE_AUTHENTICATING:
            if (p_db_dot1x_port->authSuccess)
            {
                dot1x_pae_authenticated(p_db_dot1x_port);
            }
            else if (p_db_dot1x_port->authFail)
            {
                dot1x_pae_held(p_db_dot1x_port);
            }
            else if (p_db_dot1x_port->auth_pae_eapStart || p_db_dot1x_port->auth_pae_eapLogoff 
                || p_db_dot1x_port->reAuthenticate || p_db_dot1x_port->authTimeout)
            {
                dot1x_pae_aborting(p_db_dot1x_port);
            }
            break;

        case AUTH_PAE_STATE_AUTHENTICATED:
            if (p_db_dot1x_port->auth_pae_eapLogoff)
            {
                dot1x_pae_disconnected(p_db_dot1x_port);
            }
            else if (p_db_dot1x_port->auth_pae_eapStart || p_db_dot1x_port->reAuthenticate)
            {
                dot1x_pae_restart(p_db_dot1x_port);
            }
            break;

        case AUTH_PAE_STATE_ABORTING:
            if (p_db_dot1x_port->auth_pae_eapLogoff && !p_db_dot1x_port->authAbort)
            {
                dot1x_pae_disconnected(p_db_dot1x_port);
            }
            else if (!p_db_dot1x_port->auth_pae_eapLogoff && !p_db_dot1x_port->authAbort)
            {
                dot1x_pae_restart(p_db_dot1x_port);
            }
            break;

        case AUTH_PAE_STATE_HELD:
            if (0 == p_db_dot1x_port->quietWhile)
            {
                dot1x_pae_restart(p_db_dot1x_port);
            }
            break;

        case AUTH_PAE_STATE_FORCE_AUTH:
            if (p_db_dot1x_port->auth_pae_eapStart)
            {
                dot1x_pae_force_auth(p_db_dot1x_port);
            }
            break;

        case AUTH_PAE_STATE_FORCE_UNAUTH:
            if (p_db_dot1x_port->auth_pae_eapStart)
            {
                dot1x_pae_force_unauth(p_db_dot1x_port);
            }
            break;

        case AUTH_PAE_STATE_RESTART:
            if (!p_db_dot1x_port->eaprestart)
            {
                dot1x_pae_connecting(p_db_dot1x_port);
            }
            break;

        default:
            dot1x_pae_initialize(p_db_dot1x_port);
            break;
        }
    }

    return;
}

void dot1x_run_sm(tbl_dot1x_port_t *p_db_dot1x_port)
{
    dot1x_reauth_sm(p_db_dot1x_port);
    dot1x_key_xmit_sm(p_db_dot1x_port);
    dot1x_ctrl_dir_sm(p_db_dot1x_port);
    dot1x_be_sm(p_db_dot1x_port);    
    dot1x_pae_sm(p_db_dot1x_port);
  
    return;
}


