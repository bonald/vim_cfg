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
void dot1x_db_clear_mac_timer_fn(void *p_data)
{
    tbl_dot1x_mac_t        *p_db_dot1x_mac = NULL;
    tbl_dot1x_port_t       *p_db_dot1x_port = NULL;
    tbl_dot1x_port_t       dot1x_port;

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));

    p_db_dot1x_mac = (tbl_dot1x_mac_t*)p_data;
    if (!p_db_dot1x_mac)
    {
        DOT1X_LOG_ERR("dot1x_db_clear_mac_timer p_data is NULL\n");
        return;
    }

    p_db_dot1x_mac->t_delete = NULL;

    dot1x_port.key.ifindex = p_db_dot1x_mac->key.ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        DOT1X_LOG_ERR("802.1x not configured on this interface for ifindex %d", p_db_dot1x_mac->key.ifindex);
        return;
    }

    if (!GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT)
        && !GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT)
        && !GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT))
    {
        dot1x_db_del_mac(p_db_dot1x_port, p_db_dot1x_mac);
    }

    return;
}

tbl_dot1x_mac_t * 
dot1x_db_add_mac(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_dot1x_mac_t *p_dot1x_mac)
{
    tbl_dot1x_mac_t       *p_db_dot1x_mac = NULL;
    int32                 rc = PM_E_NONE;
    tbl_dot1x_global_t    *p_db_glb = NULL;

    rc = tbl_dot1x_mac_add_dot1x_mac(p_dot1x_mac);
    if (rc)
    {
        DOT1X_LOG_ERR("Create dot1x_mac_entry fail for entry %02x%02x.%02x%02x.%02x%02x",
            p_dot1x_mac->key.mac[0],  p_dot1x_mac->key.mac[1], p_dot1x_mac->key.mac[2],            
            p_dot1x_mac->key.mac[3],  p_dot1x_mac->key.mac[4], p_dot1x_mac->key.mac[5]);
        
        return NULL;
    }

    p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&p_dot1x_mac->key);
    if (!p_db_dot1x_mac)
    {
        DOT1X_LOG_ERR("Create dot1x_mac_entry fail for entry %02x%02x.%02x%02x.%02x%02x",
            p_dot1x_mac->key.mac[0],  p_dot1x_mac->key.mac[1], p_dot1x_mac->key.mac[2],            
            p_dot1x_mac->key.mac[3],  p_dot1x_mac->key.mac[4], p_dot1x_mac->key.mac[5]);
        
        return NULL;
    }

    if (!p_db_dot1x_mac->t_delete)
    {
        p_db_dot1x_mac->t_delete = ctc_task_add_timer(TASK_PRI_NORMAL, dot1x_db_clear_mac_timer_fn, 
                    p_db_dot1x_mac, AUTH_MAC_ENTRY_WAITING_TIME); 
    }

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (p_db_glb)
    {
        p_db_glb->mac_user_count_db++;
    }
        
    p_db_dot1x_port->current_user_count_db++;
    
    return p_db_dot1x_mac;
}

void
dot1x_db_del_mac(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_dot1x_mac_t *p_dot1x_mac)
{
    tbl_dot1x_mac_t        *p_db_dot1x_mac = NULL;
    tbl_dot1x_global_t     *p_db_glb = NULL;
    int32                  rc = PM_E_NONE;

    p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&p_dot1x_mac->key);
    if (!p_db_dot1x_mac)
    {
        DOT1X_LOG_ERR("Delete dot1x_mac_entry fail for entry %02x%02x.%02x%02x.%02x%02x not exist src()",
            p_dot1x_mac->key.mac[0],  p_dot1x_mac->key.mac[1], p_dot1x_mac->key.mac[2],            
            p_dot1x_mac->key.mac[3],  p_dot1x_mac->key.mac[4], p_dot1x_mac->key.mac[5]);
        
        return;
    }

    if (p_db_dot1x_mac->t_delete)
    {
        ctc_task_delete_timer(p_db_dot1x_mac->t_delete);
        p_db_dot1x_mac->t_delete = NULL;
    }

    if (p_db_dot1x_mac->t_reject)
    {
        ctc_task_delete_timer(p_db_dot1x_mac->t_reject);
        p_db_dot1x_mac->t_reject = NULL;
    }

    if (p_db_dot1x_mac->t_reauth)
    {
        ctc_task_delete_timer(p_db_dot1x_mac->t_reauth);
        p_db_dot1x_mac->t_reauth = NULL;
    }

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (p_db_glb && p_db_glb->mac_user_count_db > 0)
    {
        p_db_glb->mac_user_count_db--;
    }

    if (p_db_dot1x_port->current_user_count_db > 0)
    {
        p_db_dot1x_port->current_user_count_db--;
    }

    /* modified by liwh for bug 47489, 2018-05-31 
        maybe dot1x mac is cleared, therefore auth session should be deleted */
    if (INVALID_RADIUS_MSG_ID != p_dot1x_mac->session_id)
    {
        dot1x_radius_end_mac_association(p_dot1x_mac);
    }
    /* liwh end */

    rc = tbl_dot1x_mac_del_dot1x_mac(&p_dot1x_mac->key);
    if (rc)
    {
        DOT1X_LOG_ERR("Delete dot1x mac %2x:%2x:%2x:%2x:%2x:%2x ifindex %d failure",
            p_dot1x_mac->key.mac[0], p_dot1x_mac->key.mac[1], p_dot1x_mac->key.mac[2],
            p_dot1x_mac->key.mac[3], p_dot1x_mac->key.mac[4], p_dot1x_mac->key.mac[5], 
            p_dot1x_mac->key.ifindex);
        
        return;
    }

    return;
}

void
dot1x_api_user_off_line(tbl_dot1x_port_t *p_db_dot1x_port, tbl_dot1x_mac_t  *p_db_dot1x_mac)
{
    if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT)
        || GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT)
        || GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT))
    {
        dot1x_set_fea_delete_mac(p_db_dot1x_mac);
        dot1x_db_mac_dec_count(p_db_dot1x_port);
    }
        
    dot1x_db_del_mac(p_db_dot1x_port, p_db_dot1x_mac);
    
    return;
}

void
dot1x_api_user_off_line_whole_port(tbl_dot1x_port_t *p_db_dot1x_port)
{
    tbl_dot1x_mac_master_t  *p_glb_dot1x_mac = NULL;    
    tbl_dot1x_mac_t         *p_db_dot1x_mac = NULL;
    ctclib_slistnode_t      *listnode = NULL; 
    ctclib_slistnode_t      *listnode1 = NULL; 
    
    p_glb_dot1x_mac = tbl_dot1x_mac_get_master();     
    if (!p_glb_dot1x_mac)
    {
        DOT1X_LOG_ERR("dot1x_clear_interface fail because tbl_dot1x_mac is not exist");
        return;
    }

    CTCLIB_SLIST_LOOP_DEL(p_glb_dot1x_mac->dot1x_mac_list, p_db_dot1x_mac, listnode, listnode1)
    {
        if (p_db_dot1x_mac->key.ifindex == p_db_dot1x_port->key.ifindex)
        {
            dot1x_api_user_off_line(p_db_dot1x_port, p_db_dot1x_mac);
        }
    }
    
    return;
}

void
dot1x_db_mac_inc_count(tbl_dot1x_port_t  *p_db_dot1x_port)
{
    tbl_dot1x_global_t *p_db_glb = NULL;

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (p_db_glb)
    {
        p_db_glb->mac_user_count_current++;
        tbl_dot1x_global_set_dot1x_global_field(p_db_glb, TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_CURRENT);
    }
        
    p_db_dot1x_port->current_user++;

    return;
}

void
dot1x_db_mac_dec_count(tbl_dot1x_port_t  *p_db_dot1x_port)
{
    tbl_dot1x_global_t *p_db_glb = NULL;

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (p_db_glb && p_db_glb->mac_user_count_current > 0)
    {
        p_db_glb->mac_user_count_current--;       
        tbl_dot1x_global_set_dot1x_global_field(p_db_glb, TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_CURRENT);
    }
    else
    {
        DOT1X_LOG_ERR("dot1x_db_mac_dec_count database error system count [%d]!",
            p_db_glb->mac_user_count_current);
        
        p_db_glb->mac_user_count_current = 0;            
        tbl_dot1x_global_set_dot1x_global_field(p_db_glb, TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_CURRENT);
    }

    if (p_db_dot1x_port->current_user > 0)
    {
        p_db_dot1x_port->current_user--;
    }
    else
    {
        DOT1X_LOG_ERR("dot1x_db_mac_dec_count database error port count[%d]",
            p_db_dot1x_port->current_user);
        
        p_db_dot1x_port->current_user = 0;       
    }

    return;
}

int32
dot1x_db_check_count(tbl_dot1x_port_t  *p_db_dot1x_port, bool check_sw_db)
{
    tbl_dot1x_global_t *p_db_glb = NULL;

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        return PM_E_FAIL;
    }

    if (check_sw_db)/*check software mac_entry count*/
    {
        if ((p_db_dot1x_port->max_user != 0) 
            && (p_db_dot1x_port->current_user_count_db >= (p_db_dot1x_port->max_user * 2)))
        {
            DOT1X_LOG_INFO("reach dot1x mac based user count limitation on on interface %s", get_intf_print_name(p_db_dot1x_port->name));
            return PM_E_FAIL;
        }

        if (p_db_glb->mac_user_count_db >= (p_db_glb->mac_user_count_max * 2))
        {
            DOT1X_LOG_INFO("reach dot1x mac based user count limitation on this system");
            return PM_E_FAIL;
        }
    }
    else /*check tcam count*/
    {
        if (p_db_dot1x_port->max_user != 0 && p_db_dot1x_port->current_user >= p_db_dot1x_port->max_user)
        {
            DOT1X_LOG_INFO("reach dot1x mac based user count limitation (hardware entry) on interface %s", 
                get_intf_print_name(p_db_dot1x_port->name));
            return PM_E_FAIL;
        }

        if ((p_db_glb->mac_user_count_current >= p_db_glb->mac_user_count_max))
        {
            DOT1X_LOG_INFO("reach dot1x mac based user count limitation (hardware entry) on this system");
            return PM_E_FAIL;
        }
    }
    
    return PM_E_NONE;
}

void
dot1x_mac_connect_session(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_auth_session_t  *p_auth_session)
{
    tbl_dot1x_mac_t *p_db_dot1x_mac = NULL;
    tbl_dot1x_mac_t dot1x_mac_entry;
 
    sal_memset(&dot1x_mac_entry, 0, sizeof(tbl_dot1x_mac_t));
    
    sal_memcpy(&dot1x_mac_entry.key.mac, p_db_dot1x_port->dest_mac, sizeof(mac_addr_t));
    dot1x_mac_entry.key.ifindex = p_db_dot1x_port->key.ifindex;

    p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&dot1x_mac_entry.key);
    if (!p_db_dot1x_mac)
    {
        return;
    }

    p_db_dot1x_mac->session_id = p_auth_session->uCurSessId;

    sal_memcpy(p_auth_session->dot1x_mac_addr, p_db_dot1x_mac->key.mac, MAC_ADDR_LEN);
    p_auth_session->dot1x_mac_ifindex = dot1x_mac_entry.key.ifindex;

    return;
}

