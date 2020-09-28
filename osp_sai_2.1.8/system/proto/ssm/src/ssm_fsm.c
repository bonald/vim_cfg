#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ssm_global_define.h"
#include "gen/tbl_ssm_global.h"
#include "gen/tbl_ssm_port_define.h"
#include "gen/tbl_ssm_port.h"
#include "ssm.h"
#include "ssm_api.h"
#include "ssm_opts.h"
#include "ctc_task.h"
#include "ssm_pdu.h"
#include "ssm_fsm.h"
#include "ssm_desc.h"


tbl_ssm_port_t *
ssm_get_tbl_ssm_by_ifindex(uint32 ifindex)
{
    int32 rc = PM_E_NONE;
    char db_ifname[IFNAME_SIZE];
    tbl_ssm_port_key_t ssm_port_key;
    rc = tbl_interface_get_name_by_ifindex(ifindex, db_ifname, IFNAME_SIZE);  
    if (rc < 0)
    {
        return NULL;
    }
    sal_memset(&ssm_port_key, 0, sizeof(ssm_port_key));
    sal_strncpy(ssm_port_key.name, db_ifname, IFNAME_SIZE);
    
    return tbl_ssm_port_get_ssm_port(&ssm_port_key);
}

uint32
_ssm_fsm_get_compared_port_quality(tbl_ssm_port_t* p_ssm_if)
{
    return (SSM_QL_INV3 == p_ssm_if->force_rx_quality) ? p_ssm_if->receive_quality : p_ssm_if->force_rx_quality;
}


int32
_ssm_fsm_set_holdoff_timer(tbl_ssm_port_t* p_ssm_if_best)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    int32 rc = PM_E_NONE;
    if (p_ssm_glb->holdoff_send_timer)
    {
        if (p_ssm_glb->next_selected_ifindex)
        {
            if (p_ssm_glb->next_selected_ifindex == p_ssm_if_best->ifindex)
            {
                return PM_E_NONE;
            }
            else
            {
                ctc_task_delete_timer(p_ssm_glb->holdoff_send_timer);
                p_ssm_glb->holdoff_send_timer = NULL;
            }
        }
        else
        {   
            p_ssm_glb->next_selected_ifindex = p_ssm_if_best->ifindex;
            return PM_E_NONE;
        }
    }
    /*set holdover state*/
    PM_E_RETURN(ssm_opts_set_holdover_state(FALSE));
    p_ssm_glb->next_selected_ifindex = p_ssm_if_best->ifindex;
    
    p_ssm_glb->holdoff_send_timer = p_ssm_glb->holdoff_send_timer ? p_ssm_glb->holdoff_send_timer
    : ctc_task_add_timer_msec(TASK_PRI_NORMAL, ssm_opts_holdoff_select_sync_if_trigger, p_ssm_if_best, p_ssm_glb->holdoff_time);
    
    return PM_E_NONE;
}

int32
_ssm_fsm_set_wtr_timer(tbl_ssm_port_t* p_ssm_if_best, tbl_ssm_port_t* p_db_ssm_select_if)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    int32 rc = PM_E_NONE;
    if (p_ssm_glb->wtr_send_timer)
    {
        if (p_ssm_glb->next_selected_ifindex)
        {
            if (p_ssm_glb->next_selected_ifindex == p_ssm_if_best->ifindex)
            {
                if (!ssm_port_is_up(p_db_ssm_select_if))
                {
                    PM_E_RETURN(ssm_opts_set_holdover_state(TRUE));     
                }
                return PM_E_NONE;
            }
            else
            {
                ctc_task_delete_timer(p_ssm_glb->wtr_send_timer);
                p_ssm_glb->wtr_send_timer = NULL;
            }
        }
        else
        {
            p_ssm_glb->next_selected_ifindex = p_ssm_if_best->ifindex;
            return PM_E_NONE;
        }
    }
    p_ssm_glb->next_selected_ifindex = p_ssm_if_best->ifindex;
    p_ssm_glb->wtr_send_timer = p_ssm_glb->wtr_send_timer ? p_ssm_glb->wtr_send_timer
    : ctc_task_add_timer(TASK_PRI_NORMAL, ssm_opts_wtr_select_sync_if_trigger, p_ssm_if_best, p_ssm_glb->wtr_time);

    return PM_E_NONE;
}

static int32
_ssm_fsm_compare_quality(uint32 p_port_quality, uint32 p_temp_quality)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    if (SSM_OPTION_MODE_1 == p_ssm_glb->option_mode)
    {
        if (p_port_quality > p_temp_quality)
        {
            return 1;
        } 
        else if (p_port_quality == p_temp_quality)
        {
            return 0;
        }
        else
        {
            return -1;
        }     
    }
    else
    {
        if (SSM_QL_STU == p_port_quality)
        {
            p_port_quality = p_port_quality + SSM_FSM_ORDER_NUM_STU; /*order by quanlity level*/
        }
        if (SSM_QL_ST2 == p_port_quality)
        {
            p_port_quality = p_port_quality - SSM_FSM_ORDER_NUM_ST2; /*order by quanlity level*/
        }
        
        if (SSM_QL_ST3E == p_port_quality)
        {
            p_port_quality = p_port_quality - SSM_FSM_ORDER_NUM_ST3E; /*order by quanlity level*/
        }
        
        if (SSM_QL_STU == p_temp_quality)
        {
            p_temp_quality = p_temp_quality + SSM_FSM_ORDER_NUM_STU; /*order by quanlity level*/
        }
        
        if (SSM_QL_ST2 == p_temp_quality)
        {
            p_temp_quality = p_temp_quality - SSM_FSM_ORDER_NUM_ST2; /*order by quanlity level*/
        }
        
        if (SSM_QL_ST3E == p_temp_quality)
        {
            p_temp_quality = p_temp_quality - SSM_FSM_ORDER_NUM_ST3E; /*order by quanlity level*/
        }

        if (p_port_quality > p_temp_quality)
        {
            return 1;
        }
        else if (p_port_quality == p_temp_quality)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
}

bool
_ssm_fsm_check_replace_clockid(ssm_clock_id_t clock_id)
{
    tbl_ssm_port_master_t *p_master = tbl_ssm_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_ssm_port_t* p_db_ssm_if = NULL;

    CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
    {
        if (NULL == p_db_ssm_if || !p_db_ssm_if->replace_clockid_en)
        {
            continue;
        }
        else
        {
            if (0 == sal_memcmp(clock_id, p_db_ssm_if->receive_clock_id, SSM_CLOCK_ID_LEN))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

int32
ssm_fsm_select_best_port_holdoff_or_wtr(tbl_ssm_port_t* p_ssm_if_best)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_t* p_db_ssm_select_if = NULL;
    int32 rc = PM_E_NONE;
    
    if (p_ssm_glb->select_ifindex)
    {
        p_db_ssm_select_if = ssm_get_tbl_ssm_by_ifindex (p_ssm_glb->select_ifindex);
        if (NULL == p_db_ssm_select_if)
        {
            /*Event Debug*/
            SSM_EVENT_DEBUG("Select best port holdoff or wtr fail: invalid ifindex %d", p_ssm_glb->select_ifindex);
            return PM_E_FAIL;
        }
        if (p_ssm_if_best->receive_quality > p_db_ssm_select_if->receive_quality)
        {
            if (p_ssm_if_best->ifindex == p_db_ssm_select_if->ifindex)
            {
                SSM_EVENT_DEBUG("Forthcoming port receive quality is more than selected port receive quality and the port numbers are equal");
                PM_E_RETURN(ssm_opts_selected_ifindex(p_ssm_if_best));
            }
            else
            {
                SSM_EVENT_DEBUG("Forthcoming port receive quality is more than selected port receive quality and the port numbers are not equal");
                PM_E_RETURN(_ssm_fsm_set_holdoff_timer(p_ssm_if_best));
            }   
        }
        else if (p_ssm_if_best->receive_quality < p_db_ssm_select_if->receive_quality)
        {
            if (p_ssm_if_best->ifindex == p_db_ssm_select_if->ifindex)
            {
                SSM_EVENT_DEBUG("Forthcoming port receive quality is less than selected port receive quality and the port numbers are equal");
                PM_E_RETURN(ssm_opts_selected_ifindex(p_ssm_if_best));
            }
            else if ((p_ssm_if_best->ifindex != p_db_ssm_select_if->ifindex) && p_ssm_glb->select_if_quality_change_en)
            {
                if (p_ssm_glb->wtr_send_timer)
                {
                    SSM_EVENT_DEBUG("Forthcoming port receive quality is less than selected port receive quality and the port numbers are not equal and WTRtimer exist");
                    PM_E_RETURN(_ssm_fsm_set_wtr_timer(p_ssm_if_best, p_db_ssm_select_if));
                }
                else
                {
                    SSM_EVENT_DEBUG("Forthcoming port receive quality is less than selected port receive quality and the port numbers are not equal and WTRtimer not exist");
                    PM_E_RETURN(_ssm_fsm_set_holdoff_timer(p_ssm_if_best));
                }
            }
            else
            {
                if (p_ssm_glb->holdoff_send_timer)
                {
                    ctc_task_delete_timer(p_ssm_glb->holdoff_send_timer);
                    p_ssm_glb->holdoff_send_timer = NULL;
                }
                SSM_EVENT_DEBUG("Forthcoming port receive quality is less than selected port receive quality");
                PM_E_RETURN(_ssm_fsm_set_wtr_timer(p_ssm_if_best, p_db_ssm_select_if));
            }
        }
        else if (p_ssm_if_best->receive_quality == p_db_ssm_select_if->receive_quality)
        {
            if (p_ssm_if_best->priority > p_db_ssm_select_if->priority)
            {
                SSM_EVENT_DEBUG("Forthcoming port receive quality is equal to selected port receive quality and the priority of forthcoming port is bigger");
                PM_E_RETURN(_ssm_fsm_set_holdoff_timer(p_ssm_if_best));
            }
            else if (p_ssm_if_best->priority < p_db_ssm_select_if->priority)
            {
                if (p_ssm_glb->select_if_priority_change_en)
                {
                    SSM_EVENT_DEBUG("Forthcoming port receive quality is equal to selected port receive quality and the priority of selected port is bigger and changed");
                    PM_E_RETURN(_ssm_fsm_set_holdoff_timer(p_ssm_if_best));
                }
                else
                {
                    if (p_ssm_glb->holdoff_send_timer)
                    {
                        ctc_task_delete_timer(p_ssm_glb->holdoff_send_timer);
                        p_ssm_glb->holdoff_send_timer = NULL;
                    }
                    SSM_EVENT_DEBUG("Forthcoming port receive quality is equal to selected port receive quality and the priority of selected port is bigger and not changed");
                    PM_E_RETURN(_ssm_fsm_set_wtr_timer(p_ssm_if_best, p_db_ssm_select_if));
                }
            }
            else if ((p_ssm_if_best->priority == p_db_ssm_select_if->priority) &&
                     (p_ssm_if_best->ifindex > p_db_ssm_select_if->ifindex))
            {
                SSM_EVENT_DEBUG("Forthcoming port receive quality&priority is equal to selected port receive quality&priority and the forthcoming port number is bigger");
                PM_E_RETURN(_ssm_fsm_set_holdoff_timer(p_ssm_if_best));
            }
            else if ((p_ssm_if_best->priority == p_db_ssm_select_if->priority) &&
                     (p_ssm_if_best->ifindex < p_db_ssm_select_if->ifindex))
            {
                if (p_ssm_glb->holdoff_send_timer)
                {
                    ctc_task_delete_timer(p_ssm_glb->holdoff_send_timer);
                    p_ssm_glb->holdoff_send_timer = NULL;
                }
                SSM_EVENT_DEBUG("Forthcoming port receive quality&priority is equal to selected port receive quality&priority and the selected port number is bigger");
                PM_E_RETURN(_ssm_fsm_set_wtr_timer(p_ssm_if_best, p_db_ssm_select_if));
            }
            else if ((p_ssm_if_best->priority == p_db_ssm_select_if->priority) &&
                     (p_ssm_if_best->ifindex == p_db_ssm_select_if->ifindex))
            {
                if (p_ssm_glb->holdoff_send_timer)
                {
                    ctc_task_delete_timer(p_ssm_glb->holdoff_send_timer);
                    p_ssm_glb->holdoff_send_timer = NULL;

                    p_ssm_glb->select_if_linkup_restore_en = TRUE;
                    p_ssm_glb->next_selected_ifindex = p_ssm_if_best->ifindex;
                    SSM_EVENT_DEBUG("Forthcoming port receive quality&priority&number is equal to selected port receive quality&priority&number and holdoff timer exist");
                    PM_E_RETURN(ssm_opts_selected_ifindex(p_ssm_if_best));

                    p_ssm_glb->select_if_priority_change_en = FALSE;
                    p_ssm_glb->select_if_linkup_restore_en = FALSE;
                    return PM_E_NONE;
                }
                if (p_ssm_glb->wtr_send_timer)
                {
                    ctc_task_delete_timer(p_ssm_glb->wtr_send_timer);
                    p_ssm_glb->wtr_send_timer = NULL;
                }
                p_ssm_glb->next_selected_ifindex = p_ssm_if_best->ifindex;
                PM_E_RETURN(ssm_opts_selected_ifindex(p_ssm_if_best));
                p_ssm_glb->select_if_priority_change_en = FALSE;
            }
        }
    }
    else
    {
        if (p_ssm_glb->wtr_send_timer)
        {
            if (p_ssm_glb->holdover_en)
            {
                if (p_ssm_glb->next_selected_ifindex && 
                    (p_ssm_glb->next_selected_ifindex != p_ssm_if_best->ifindex))
                {
                    SSM_EVENT_DEBUG("Ssm enable and select if is not exist but WTRtimer is exist");
                    ctc_task_delete_timer(p_ssm_glb->wtr_send_timer);
                    p_ssm_glb->wtr_send_timer = NULL;
                    PM_E_RETURN(ssm_opts_selected_ifindex(p_ssm_if_best));
                }
            }
        }
        else
        {
            PM_E_RETURN(ssm_opts_selected_ifindex(p_ssm_if_best));
        }
    }
    
    return PM_E_NONE;  
}

int32
ssm_fsm_select_best_port_holdoff_or_wtr_non_ssm(tbl_ssm_port_t* p_ssm_if_best)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_t* p_db_ssm_select_if = NULL;
    int32 rc = PM_E_NONE;

    if (p_ssm_glb->select_ifindex)
    {
        p_db_ssm_select_if = ssm_get_tbl_ssm_by_ifindex (p_ssm_glb->select_ifindex);
        if (!p_db_ssm_select_if)
        {
            /*Event Debug*/
            SSM_EVENT_DEBUG("Select best port holdoff or wtr non ssm fail: invalid ifindex %d", p_ssm_glb->select_ifindex);
            return PM_E_FAIL;
        }
        if (p_ssm_if_best->priority > p_db_ssm_select_if->priority)
        {
            SSM_EVENT_DEBUG("Forthcoming port priority is more than selected port priority");
            PM_E_RETURN(_ssm_fsm_set_holdoff_timer(p_ssm_if_best));
        }
        else if (p_ssm_if_best->priority < p_db_ssm_select_if->priority)
        {
            if (p_ssm_glb->select_if_priority_change_en)
            {
                SSM_EVENT_DEBUG("Forthcoming port priority is less than selected port priority and the priority of selected port is changed");
                PM_E_RETURN(_ssm_fsm_set_holdoff_timer(p_ssm_if_best));
            }
            else
            {
                if (p_ssm_glb->holdoff_send_timer)
                {
                    ctc_task_delete_timer(p_ssm_glb->holdoff_send_timer);
                    p_ssm_glb->holdoff_send_timer = NULL;
                }
                SSM_EVENT_DEBUG("Forthcoming port priority is less than selected port priority");
                PM_E_RETURN(_ssm_fsm_set_wtr_timer(p_ssm_if_best, p_db_ssm_select_if));
            }
        }
        else if (p_ssm_if_best->priority == p_db_ssm_select_if->priority &&
                 p_ssm_if_best->ifindex < p_db_ssm_select_if->ifindex)
        {
            if (p_ssm_glb->holdoff_send_timer)
            {
                ctc_task_delete_timer(p_ssm_glb->holdoff_send_timer);
                p_ssm_glb->holdoff_send_timer = NULL;
            }
            SSM_EVENT_DEBUG("Forthcoming port priority is equal to selected port priority and the selected port number is bigger");
            PM_E_RETURN(_ssm_fsm_set_wtr_timer(p_ssm_if_best, p_db_ssm_select_if));
        }
        else if (p_ssm_if_best->priority == p_db_ssm_select_if->priority &&
                 p_ssm_if_best->ifindex > p_db_ssm_select_if->ifindex)
        {
            SSM_EVENT_DEBUG("Forthcoming port priority is equal to selected port priority and the forthcoming port number is bigger");
            PM_E_RETURN(_ssm_fsm_set_holdoff_timer(p_ssm_if_best));
        }
        else if (p_ssm_if_best->priority == p_db_ssm_select_if->priority &&
                 p_ssm_if_best->ifindex == p_db_ssm_select_if->ifindex)
        {
            if (p_ssm_glb->holdoff_send_timer)
            {
                SSM_EVENT_DEBUG("Forthcoming port priority&number is equal to selected port priority&number and holdoff timer exist");
                ctc_task_delete_timer(p_ssm_glb->holdoff_send_timer);
                p_ssm_glb->holdoff_send_timer = NULL;
                
                p_ssm_glb->select_if_linkup_restore_en = TRUE;
                p_ssm_glb->next_selected_ifindex = p_ssm_if_best->ifindex;
                PM_E_RETURN(ssm_opts_selected_ifindex(p_ssm_if_best));

                p_ssm_glb->select_if_priority_change_en = FALSE;
                p_ssm_glb->select_if_linkup_restore_en = FALSE;
                return PM_E_NONE;
            }
            if (p_ssm_glb->wtr_send_timer)
            {
                ctc_task_delete_timer(p_ssm_glb->wtr_send_timer);
                p_ssm_glb->wtr_send_timer = NULL;
            }
            p_ssm_glb->next_selected_ifindex = p_ssm_if_best->ifindex;
            PM_E_RETURN(ssm_opts_selected_ifindex(p_ssm_if_best));
            p_ssm_glb->select_if_priority_change_en = FALSE;
        }
    }
    else
    {
        if (p_ssm_glb->wtr_send_timer)
        {
            if (p_ssm_glb->holdover_en)
            {
                if (p_ssm_glb->next_selected_ifindex && 
                    (p_ssm_glb->next_selected_ifindex != p_ssm_if_best->ifindex))
                {
                    SSM_EVENT_DEBUG("Ssm disable and select if is not exist but WTRtimer is exist");
                    ctc_task_delete_timer(p_ssm_glb->wtr_send_timer);
                    p_ssm_glb->wtr_send_timer = NULL;
                    PM_E_RETURN(ssm_opts_selected_ifindex(p_ssm_if_best));
                }
            }
        }
        else
        {
            PM_E_RETURN(ssm_opts_selected_ifindex(p_ssm_if_best));
        }    
    }
    return PM_E_NONE;  
}

int32
_ssm_fsm_select_best_port_slave(void)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_master_t *p_master = tbl_ssm_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_ssm_port_t* p_db_ssm_if = NULL;
    tbl_ssm_port_t* p_ssm_if_best = NULL;
    char ifname_ext[IFNAME_SIZE];
    int32 rc = PM_E_NONE;

    bool replace_clockid = FALSE;
    int32 cmp_quality = 0;

    CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
    {
        if (NULL == p_db_ssm_if || !p_db_ssm_if->synce_enable || !ssm_port_is_up(p_db_ssm_if) ||
            (SSM_QL_INV3 == p_db_ssm_if->force_rx_quality && !p_db_ssm_if->receive_pdu_enable) ||
            (SSM_QL_INV3 != p_db_ssm_if->force_rx_quality && SSM_QL_DNU == p_db_ssm_if->force_rx_quality) ||
            (SSM_QL_INV3 == p_db_ssm_if->force_rx_quality && !p_db_ssm_if->ssm_timeout_timer) || /*for bug 46354, 2018-02-05*/
            (SSM_QL_INV3 == p_db_ssm_if->force_rx_quality && SSM_QL_DNU == p_db_ssm_if->receive_quality) ||
            (p_ssm_glb->ssm_extend_enable && (0 == sal_memcmp(p_ssm_glb->device_clock_id,
                                                      p_db_ssm_if->receive_clock_id, SSM_CLOCK_ID_LEN))))
        {
            continue;
        }

        if (p_ssm_glb->ssm_extend_enable && !p_db_ssm_if->replace_clockid_en)
        {
            replace_clockid = _ssm_fsm_check_replace_clockid(p_db_ssm_if->receive_clock_id);
            if (replace_clockid)
            {
                /*SSM debug*/
                SSM_EVENT_DEBUG("Don't select clock on port %s, received clock id is equal to other port's clock id\n", 
                                IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext));
                continue;
            }
        }
        
        if (NULL == p_ssm_if_best)
        {
            p_ssm_if_best = p_db_ssm_if;
        }
        cmp_quality = _ssm_fsm_compare_quality(_ssm_fsm_get_compared_port_quality(p_db_ssm_if),
                                               _ssm_fsm_get_compared_port_quality(p_ssm_if_best));

        if (-1 == cmp_quality)
        {
            p_ssm_if_best = p_db_ssm_if;
        }
        else if (0 == cmp_quality)
        {
            if (p_db_ssm_if->priority < p_ssm_if_best->priority)
            {
                p_ssm_if_best = p_db_ssm_if;
            }
            else if (p_db_ssm_if->priority == p_ssm_if_best->priority)
            {
                if (p_ssm_glb->ssm_extend_enable)
                {
                    if (p_db_ssm_if->hop < p_ssm_if_best->hop)
                    {
                        p_ssm_if_best = p_db_ssm_if;
                    }
                    else if ((p_db_ssm_if->hop == p_ssm_if_best->hop) && 
                             (p_db_ssm_if->ifindex < p_ssm_if_best->ifindex))
                    {
                        p_ssm_if_best = p_db_ssm_if;
                    }
                }
                else
                {
                    if (p_db_ssm_if->ifindex < p_ssm_if_best->ifindex)
                    {
                        p_ssm_if_best = p_db_ssm_if;
                    }
                }
            }
        }
    }

    if (NULL != p_ssm_if_best)
    {
        PM_E_RETURN(ssm_fsm_select_best_port_holdoff_or_wtr(p_ssm_if_best));
    }
    else
    {
        if (!p_ssm_glb->freerun_en)
        {
            SSM_EVENT_DEBUG("_ssm_fsm_select_best_port_slave: set holdover");
        }
        else
        {
            SSM_EVENT_DEBUG("_ssm_fsm_select_best_port_slave: keep freerun");
        }
        PM_E_RETURN(ssm_opts_set_holdover_state(TRUE));
        if (p_ssm_glb->holdoff_send_timer)
        {
            ctc_task_delete_timer(p_ssm_glb->holdoff_send_timer);
            p_ssm_glb->holdoff_send_timer = NULL;
        }
        if (p_ssm_glb->wtr_send_timer)
        {
            ctc_task_delete_timer(p_ssm_glb->wtr_send_timer);
            p_ssm_glb->wtr_send_timer = NULL;
        }
    }
    return PM_E_NONE;
}

int32
 _ssm_fsm_select_best_port_standard(void)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_master_t *p_master = tbl_ssm_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_ssm_port_t* p_db_ssm_if = NULL;
    tbl_ssm_port_t* p_ssm_if_best = NULL;
    char ifname_ext[IFNAME_SIZE];
    int32 rc = PM_E_NONE;

    bool replace_clockid = FALSE;
    int32 cmp_quality = 0;

    CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
    {
        if (NULL == p_db_ssm_if || !p_db_ssm_if->synce_enable || !ssm_port_is_up(p_db_ssm_if) ||
            (SSM_QL_INV3 == p_db_ssm_if->force_rx_quality && !p_db_ssm_if->receive_pdu_enable) ||
            (SSM_QL_INV3 != p_db_ssm_if->force_rx_quality && SSM_QL_DNU == p_db_ssm_if->force_rx_quality) ||
            (SSM_QL_INV3 == p_db_ssm_if->force_rx_quality && !p_db_ssm_if->ssm_timeout_timer) || /*for bug 46354, 2018-02-05*/
            (SSM_QL_INV3 == p_db_ssm_if->force_rx_quality && SSM_QL_DNU == p_db_ssm_if->receive_quality) ||
            (p_ssm_glb->ssm_extend_enable && (0 == sal_memcmp(p_ssm_glb->device_clock_id,
                                                      p_db_ssm_if->receive_clock_id, SSM_CLOCK_ID_LEN))))
        {
            continue;
        }

        if (p_ssm_glb->ssm_extend_enable && !p_db_ssm_if->replace_clockid_en)
        {
            replace_clockid = _ssm_fsm_check_replace_clockid(p_db_ssm_if->receive_clock_id);
            if (replace_clockid)
            {
                /*SSM debug*/
                SSM_EVENT_DEBUG("Don't select clock on port %s, received clock id is equal to other port's clock id\n", 
                                IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext));
                continue;
            }
        }
        
        if (NULL == p_ssm_if_best)
        {
            p_ssm_if_best = p_db_ssm_if;
        }
        cmp_quality = _ssm_fsm_compare_quality(_ssm_fsm_get_compared_port_quality(p_db_ssm_if),
                                               _ssm_fsm_get_compared_port_quality(p_ssm_if_best));

        if (-1 == cmp_quality)
        {
            p_ssm_if_best = p_db_ssm_if;
        }
        else if (0 == cmp_quality)
        {
            if (p_db_ssm_if->priority < p_ssm_if_best->priority)
            {
                p_ssm_if_best = p_db_ssm_if;
            }
            else if (p_db_ssm_if->priority == p_ssm_if_best->priority)
            {
                if (p_ssm_glb->ssm_extend_enable)
                {
                    if (p_db_ssm_if->hop < p_ssm_if_best->hop)
                    {
                        p_ssm_if_best = p_db_ssm_if;
                    }
                    else if ((p_db_ssm_if->hop == p_ssm_if_best->hop) && 
                             (p_db_ssm_if->ifindex < p_ssm_if_best->ifindex))
                    {
                        p_ssm_if_best = p_db_ssm_if;
                    }
                }
                else
                {
                    if (p_db_ssm_if->ifindex < p_ssm_if_best->ifindex)
                    {
                        p_ssm_if_best = p_db_ssm_if;
                    }
                }
            }
        }
    }

    if (NULL == p_ssm_if_best)
    {
        /*Modified by yejl for bug 46482, 2018-03-02*/
        if (!p_ssm_glb->freerun_en)
        {
            SSM_EVENT_DEBUG("_ssm_fsm_select_best_port_standard: set holdover because no port conforms");
        }
        else
        {
            SSM_EVENT_DEBUG("_ssm_fsm_select_best_port_standard: keep freerun because no port conforms");
        }
        PM_E_RETURN(ssm_opts_set_holdover_state(TRUE));
        return PM_E_NONE;
    }
    cmp_quality = _ssm_fsm_compare_quality(_ssm_fsm_get_compared_port_quality(p_ssm_if_best), p_ssm_glb->system_quality);

    //compare port and device
    if (1 == cmp_quality)
    {
        if (!p_ssm_glb->freerun_en)
        {
            SSM_EVENT_DEBUG("_ssm_fsm_select_best_port_standard: set holdover because the port quality value is bigger than system quality value");
        }
        else
        {
            SSM_EVENT_DEBUG("_ssm_fsm_select_best_port_standard: keep freerun because the port quality value is bigger than system quality value");
        }
        PM_E_RETURN(ssm_opts_set_holdover_state(TRUE));
    }
    else if (0 == cmp_quality)
    {
        if (p_ssm_if_best->priority < p_ssm_glb->local_priority)
        {
            PM_E_RETURN(ssm_fsm_select_best_port_holdoff_or_wtr(p_ssm_if_best));
        }
        else
        {
            if (!p_ssm_glb->freerun_en)
            {
                SSM_EVENT_DEBUG("_ssm_fsm_select_best_port_standard: set holdover because the port priority value is bigger or equal than system priority value");
            }
            else
            {
                SSM_EVENT_DEBUG("_ssm_fsm_select_best_port_standard: keep freerun because the port priority value is bigger or equal than system priority value");
            }
            PM_E_RETURN(ssm_opts_set_holdover_state(TRUE));
            if (p_ssm_glb->holdoff_send_timer)
            {
                ctc_task_delete_timer(p_ssm_glb->holdoff_send_timer);
                p_ssm_glb->holdoff_send_timer = NULL;
            }
            if (p_ssm_glb->wtr_send_timer)
            {
                ctc_task_delete_timer(p_ssm_glb->wtr_send_timer);
                p_ssm_glb->wtr_send_timer = NULL;
            }
        }
    }
    else
    {
        PM_E_RETURN(ssm_fsm_select_best_port_holdoff_or_wtr(p_ssm_if_best));
    }
    return PM_E_NONE;
}


int32
ssm_fsm_select_best_port(uint32 device_role)
{   
    int32 rc = PM_E_NONE;
    if (SSM_DEVICE_SLAV == device_role)
    {
        PM_E_RETURN(_ssm_fsm_select_best_port_slave());
    }
    else if (SSM_DEVICE_STAND == device_role)
    {
        PM_E_RETURN(_ssm_fsm_select_best_port_standard());
    }
     
    return PM_E_NONE; 
}
 

int32
ssm_fsm_select_clock(void)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    int32 rc = PM_E_NONE;
    
    if (SSM_DEVICE_MAST == p_ssm_glb->device_role)
    {
        p_ssm_glb->select_ifindex = 0;
        p_ssm_glb->next_selected_ifindex = 0;
        p_ssm_glb->current_quality = p_ssm_glb->system_quality;
    }
    else
    {   
        PM_E_RETURN(ssm_fsm_select_best_port(p_ssm_glb->device_role));
    }
    return PM_E_NONE; 
}

int32
ssm_fsm_select_non_ssm(uint32 device_role)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_master_t *p_master = tbl_ssm_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_ssm_port_t* p_db_ssm_if = NULL;
    tbl_ssm_port_t* p_ssm_if_best = NULL;
    int32 rc = PM_E_NONE;
    
    if (SSM_DEVICE_SLAV == device_role)
    {
        CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
        {
            if (NULL == p_db_ssm_if || !ssm_port_is_up(p_db_ssm_if) || !p_db_ssm_if->synce_enable)
            {
                continue;    
            }  
            if (NULL == p_ssm_if_best)
            {
                p_ssm_if_best = p_db_ssm_if;
            }
            if (p_db_ssm_if->priority < p_ssm_if_best->priority)
            {
                p_ssm_if_best = p_db_ssm_if;
            }
            else if (p_db_ssm_if->priority == p_ssm_if_best->priority)
            {
                if (p_db_ssm_if->ifindex < p_ssm_if_best->ifindex)
                {
                    p_ssm_if_best = p_db_ssm_if;
                }
            }
        }

        if (NULL != p_ssm_if_best)
        {
            PM_E_RETURN(ssm_opts_selected_ifindex(p_ssm_if_best)); 
        }
        else
        {
            /*set holdover state*/
            if (!p_ssm_glb->freerun_en)
            {
                SSM_EVENT_DEBUG("ssm_fsm_select_non_ssm: set holdover because no port conforms under slave role");
            }
            else
            {
                SSM_EVENT_DEBUG("ssm_fsm_select_non_ssm: keep freerun because no port conforms under slave role");
            }
            PM_E_RETURN(ssm_opts_set_holdover_state(TRUE));
        }
    }
    else if (SSM_DEVICE_MAST == device_role)
    {
        return PM_E_NONE;
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
        {
            if (NULL == p_db_ssm_if || !ssm_port_is_up(p_db_ssm_if) || !p_db_ssm_if->synce_enable)
            {
                continue;    
            }  
            if (NULL == p_ssm_if_best)
            {
                p_ssm_if_best = p_db_ssm_if;
            }
            if (p_db_ssm_if->priority < p_ssm_if_best->priority)
            {
                p_ssm_if_best = p_db_ssm_if;
            }
            else if (p_db_ssm_if->priority == p_ssm_if_best->priority)
            {
                if (p_db_ssm_if->ifindex < p_ssm_if_best->ifindex)
                {
                    p_ssm_if_best = p_db_ssm_if;
                }
            }
        }
        
        //compare port and device
        if (NULL != p_ssm_if_best)
        {
            if (p_ssm_if_best->priority < p_ssm_glb->local_priority)
            {
                PM_E_RETURN(ssm_fsm_select_best_port_holdoff_or_wtr_non_ssm(p_ssm_if_best));
            }
            else
            {
                /*set holdover state*/
                if (!p_ssm_glb->freerun_en)
                {
                    SSM_EVENT_DEBUG("ssm_fsm_select_non_ssm: set holdover because the port priority value is bigger or equal than system priority value");
                }
                else
                {
                    SSM_EVENT_DEBUG("ssm_fsm_select_non_ssm: keep freerun because the port priority value is bigger or equal than system priority value");
                }
                PM_E_RETURN(ssm_opts_set_holdover_state(TRUE));

                if (p_ssm_glb->holdoff_send_timer)
                {
                    ctc_task_delete_timer(p_ssm_glb->holdoff_send_timer);
                    p_ssm_glb->holdoff_send_timer = NULL;
                }
                if (p_ssm_glb->wtr_send_timer)
                {
                     ctc_task_delete_timer(p_ssm_glb->wtr_send_timer);
                     p_ssm_glb->wtr_send_timer = NULL;
                }
            }
        }
        else
        {
            /*set holdover state*/
            if (!p_ssm_glb->freerun_en)
            {
                SSM_EVENT_DEBUG("ssm_fsm_select_non_ssm: set holdover because no port conforms under standard role");
            }
            else
            {
                SSM_EVENT_DEBUG("ssm_fsm_select_non_ssm: keep freerun because no port conforms under standard role");
            }
            PM_E_RETURN(ssm_opts_set_holdover_state(TRUE));

            if (p_ssm_glb->holdoff_send_timer)
            {
                ctc_task_delete_timer(p_ssm_glb->holdoff_send_timer);
                p_ssm_glb->holdoff_send_timer = NULL;
            }
            if (p_ssm_glb->wtr_send_timer)
            {
                 ctc_task_delete_timer(p_ssm_glb->wtr_send_timer);
                 p_ssm_glb->wtr_send_timer = NULL;
            }
        }
    }
    return PM_E_NONE;   
}

