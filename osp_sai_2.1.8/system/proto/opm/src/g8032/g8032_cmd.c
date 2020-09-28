#include "proto.h"
#include "g8032_define.h"
#include "g8032_fsm.h"
#include "g8032_pdu.h"
#include "g8032.h"
#include "gen/tbl_g8032_ring_define.h"
#include "gen/tbl_g8032_ring.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_fdb.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_igsp_intf_define.h"
#include "gen/tbl_igsp_intf.h"
#include "gen/tbl_cfm_md_define.h"
#include "gen/tbl_cfm_md.h"
#include "gen/tbl_cfm_ma_define.h"
#include "gen/tbl_cfm_ma.h"
#include "gen/tbl_cfm_lmep_define.h"
#include "gen/tbl_cfm_lmep.h"
#include "gen/tbl_cfm_rmep_define.h"
#include "gen/tbl_cfm_rmep.h"
#include "cfm_api.h"

extern struct
{
    int32 (* func)(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east);
    int32 next_state;
}G8032FSM[G8032_FSM_STATE_MAX][G8032_FSM_EVENT_MAX] ;

/* system modified by liwh for bug 53010, 2019-08-07 */
int32 g8032_delete_cfm_association(tbl_g8032_ring_t *p_db_g8032_ring);
/* liwh end */
    
static int32
_g8032_fdb_get_condition_vlan(tbl_fdb_t *p_fdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];
    vid_t vid = *(vid_t*)pargs->argv[2];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    if (vid == p_fdb->key.fid)
    {
        p_get->node_array[p_get->index] = p_fdb;
        p_get->index++;
    }

    return PM_E_NONE;
}

int32 g8032_delete_static_fdb_by_control_vlan(uint32 vid)
{
    TBL_ITER_CB_FUNC get_func;
    cdb_get_node_t  get_node;
    tbl_iter_args_t args;
    tbl_fdb_t *p_fdb = NULL;
    uint32 i = 0;
    uint32 hash_index = 0;
    uint32 ifindex = 0;
    tbl_fdb_t *p_db_fdb = NULL;
    tbl_fdb_master_t *p_master = NULL;
    int32 rc = PM_E_NONE;
    vid_t vlan_id = 0xFFFF & vid;
    get_func = (TBL_ITER_CB_FUNC)_g8032_fdb_get_condition_vlan;
    sal_memset(&args, 0, sizeof(args));
    ifindex = GLB_IFINDEX_ALL;
    args.argv[0] = &get_node;
    args.argv[1] = &ifindex;
    args.argv[2] = &vlan_id;
    args.argv[3] = &hash_index;
    do 
    {
        sal_memset(&get_node, 0, sizeof(get_node));
        p_master = tbl_fdb_get_master();
        ctclib_hash_iterate2_cont(p_master->fdb_hash, hash_index, tbl_hash_iter_adpt_fn_cont, get_func, &args);
        for (i = 0; i < get_node.index; i++)
        {
            p_fdb = get_node.node_array[i];

            p_db_fdb = tbl_fdb_get_fdb(&p_fdb->key);
            if (NULL == p_db_fdb)
            {
            return PM_E_NOT_EXIST;
            }

            PM_E_RETURN(tbl_fdb_del_fdb(&p_db_fdb->key));
        }
    } while (CDB_GET_COUNT == get_node.index);

    return rc;
}

int32 g8032_delete_control_vlan(uint32 ring_id, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t g8032_major_ring;   
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_vlan_key_t vlan_key;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    sal_memset(&g8032_major_ring, 0, sizeof(tbl_g8032_ring_t));
    sal_memset(&vlan_key, 0, sizeof(tbl_vlan_key_t));
    
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);

    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }
    
    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't delete contro-vlan in running state");
    }
    
    vlan_key.vid = p_db_g8032_ring->control_vlan & 0xFFFF;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);

    if (p_db_vlan)
    {
        p_db_vlan->g8032_ring_id = 0;
        tbl_vlan_set_vlan_field(p_db_vlan, TBL_VLAN_FLD_G8032_RING_ID);
    }

    p_db_g8032_ring->control_vlan = 0;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_CONTROL_VLAN);
    return rc;   
}

int32 g8032_delete_instance(uint32 ring_id, uint32 instance_id, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    tbl_g8032_ring_t *p_sub_g8032_ring = NULL;
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_mstp_instance = NULL;
    uint8 find_instance = FALSE;
    int32 j = 0;
    int32 i = 0;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }
    
    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't delete instance in running state");
    }

    for (i = 0; i < p_db_g8032_ring->instance_count; i++)
    {
        if (p_db_g8032_ring->instance[i] == instance_id)
        { 
            find_instance = TRUE;
            break;
        }
    }
    
    if (!find_instance)
    {
        CFG_CONFLICT_RET("Instance %d has not been bound to G8032 ring %d ", instance_id, p_db_g8032_ring->key.ring_id);
    }
    
    if (p_db_g8032_ring->sub_ring_id[0])
    {
        p_sub_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t * )p_db_g8032_ring->sub_ring_id);
        if (p_sub_g8032_ring)
        {
             for (i = 0; i < p_sub_g8032_ring->instance_count; i++)
            {
                if (p_sub_g8032_ring->instance[i] == instance_id)
                { 
                    CFG_CONFLICT_RET("Instance %d has been bound to G8032 sub-ring %d", instance_id, p_sub_g8032_ring->key.ring_id);
                }
             }
        }
    }

    for (i = 0; i < p_db_g8032_ring->instance_count; i++)
    {
        if (p_db_g8032_ring->instance[i] == instance_id)
        {   
            for (j = i; j < (p_db_g8032_ring->instance_count - 1); j++)
            {
                 p_db_g8032_ring->instance[j] = p_db_g8032_ring->instance[j+1];
            }
            p_db_g8032_ring->instance[p_db_g8032_ring->instance_count - 1] = 0;
            p_db_g8032_ring->instance_count--;
        }
    }

    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_INSTANCE_COUNT);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_INSTANCE);
    
    inst_key.instance = instance_id;
    p_db_mstp_instance = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (!p_db_g8032_ring->g8032_major_ring_id && p_db_mstp_instance)
    {
        p_db_mstp_instance->g8032_ring_id = 0;
        tbl_mstp_instance_set_mstp_instance_field(p_db_mstp_instance, TBL_MSTP_INSTANCE_FLD_G8032_RING_ID);
    }
    
    return rc; 
}


int32 g8032_delete_ring(uint32 ring_id, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t g8032_major_ring;   
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    tbl_g8032_ring_t *p_db_major_g8032_ring = NULL;
    tbl_interface_t  *p_db_interface = NULL;
    int32 current_instance_cnt = 0;
    int32 i= 0;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    sal_memset(&g8032_major_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }
    
    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't delete ring in running state");
    }
    
    if (p_db_g8032_ring->sub_ring_id[0])
    {
        CFG_CONFLICT_RET("Please delete sub-ring first");
    }
    
    if (p_db_g8032_ring->config_sync)
    {
        p_db_g8032_ring->config_sync= 0;
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_CONFIG_SYNC);
    }

    if (p_db_g8032_ring->control_vlan)
    {
        g8032_delete_control_vlan( ring_id, para);
    }

    /* system modified by liwh for bug 53010, 2019-08-07 */
    if (0 != sal_strlen(p_db_g8032_ring->cfm_md_name))
    {
        g8032_delete_cfm_association(p_db_g8032_ring);
    }
    /* liwh end */

    if (p_db_g8032_ring->instance_count)
    {
        current_instance_cnt = p_db_g8032_ring->instance_count;
        for (i = current_instance_cnt; i > 0; i--)
        {
            g8032_delete_instance(ring_id, p_db_g8032_ring->instance[i-1], para);
        }
    }
    
    p_db_interface = tbl_interface_get_interface_by_ifindex(p_db_g8032_ring->east_port_ifindex);
    if (p_db_interface)
    {
        if (p_db_interface ->current_g8032_ring_count)
        {
            for (i = 0; i < p_db_interface ->current_g8032_ring_count; i++)
            {
                if (p_db_interface->g8032_ring_first[i] == ring_id)
                {
                    if (!i)
                    {
                        p_db_interface->g8032_ring_first[0] = p_db_interface->g8032_ring_first[p_db_interface ->current_g8032_ring_count-1];
                        p_db_interface->g8032_ring_first[p_db_interface ->current_g8032_ring_count-1] = 0;
                        p_db_interface ->current_g8032_ring_count--;
                        tbl_interface_set_interface_field(p_db_interface, TBL_INTERFACE_FLD_G8032_RING_FIRST);
                        tbl_interface_set_interface_field(p_db_interface, TBL_INTERFACE_FLD_CURRENT_G8032_RING_COUNT);
                    }
                    else
                    {
                        p_db_interface->g8032_ring_first[i] = 0;
                        p_db_interface ->current_g8032_ring_count--;
                        tbl_interface_set_interface_field(p_db_interface, TBL_INTERFACE_FLD_G8032_RING_FIRST);
                        tbl_interface_set_interface_field(p_db_interface, TBL_INTERFACE_FLD_CURRENT_G8032_RING_COUNT);
                    }

                }
            }
        }
    }

    p_db_interface = tbl_interface_get_interface_by_ifindex(p_db_g8032_ring->west_port_ifindex);
    if (p_db_interface)
    {
        if (p_db_interface ->current_g8032_ring_count)
        {
            for (i = 0; i < p_db_interface ->current_g8032_ring_count; i++)
            {
                if (p_db_interface->g8032_ring_first[i] == ring_id)
                {
                    if (!i)
                    {
                        p_db_interface->g8032_ring_first[0] = p_db_interface->g8032_ring_first[p_db_interface ->current_g8032_ring_count-1];
                        p_db_interface->g8032_ring_first[p_db_interface ->current_g8032_ring_count-1] = 0;
                        p_db_interface ->current_g8032_ring_count--;
                        tbl_interface_set_interface_field(p_db_interface, TBL_INTERFACE_FLD_G8032_RING_FIRST);
                        tbl_interface_set_interface_field(p_db_interface, TBL_INTERFACE_FLD_CURRENT_G8032_RING_COUNT);
                    }
                    else
                    {
                        p_db_interface->g8032_ring_first[i] = 0;
                        p_db_interface ->current_g8032_ring_count--;
                        tbl_interface_set_interface_field(p_db_interface, TBL_INTERFACE_FLD_G8032_RING_FIRST);
                        tbl_interface_set_interface_field(p_db_interface, TBL_INTERFACE_FLD_CURRENT_G8032_RING_COUNT);
                    }
                }
            }
        }
    }

    if (p_db_g8032_ring->g8032_major_ring_id)
    {
        g8032_major_ring.key.ring_id = p_db_g8032_ring->g8032_major_ring_id;
        p_db_major_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_major_ring.key);
        if (p_db_major_g8032_ring)
        {
            p_db_major_g8032_ring->sub_ring_id[0] = 0;
            tbl_g8032_ring_set_g8032_ring_field(p_db_major_g8032_ring, TBL_G8032_RING_FLD_SUB_RING_ID);

            g8032_fsm_stop_transmit_raps(p_db_g8032_ring);

            p_db_g8032_ring->east_port_is_blocked = FALSE;
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_BLOCKED);

            CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_wtr_while);
            CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_wtb_while);
            CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_hold_off_while);
            CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_guard_while);
            CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_flush_fdb_while);
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE);
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_HOLD_OFF_WHILE);
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_GUARD_WHILE);
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_FLUSH_FDB_WHILE);

            tbl_g8032_ring_del_g8032_ring(&g8032_ring.key);
        }
        else
        {
            g8032_fsm_stop_transmit_raps(p_db_g8032_ring);
            p_db_g8032_ring->east_port_is_blocked = FALSE;
            p_db_g8032_ring->west_port_is_blocked = FALSE;
            p_db_g8032_ring->protection_en= 0;
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_BLOCKED);
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_BLOCKED);
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_PROTECTION_EN);
            CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_wtr_while);
            CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_wtb_while);
            CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_hold_off_while);
            CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_guard_while);
            CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_flush_fdb_while);
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE);
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_HOLD_OFF_WHILE);
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_GUARD_WHILE);
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_FLUSH_FDB_WHILE);

            if (p_db_g8032_ring->config_sync)
            {
                p_db_g8032_ring->config_sync= 0;
                tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_CONFIG_SYNC);
            }
            
            tbl_g8032_ring_del_g8032_ring(&g8032_ring.key);
        }
    }
    else
    {
        
        g8032_fsm_stop_transmit_raps(p_db_g8032_ring);
        p_db_g8032_ring->east_port_is_blocked = FALSE;
        p_db_g8032_ring->west_port_is_blocked = FALSE;
        p_db_g8032_ring->protection_en= 0;
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_BLOCKED);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_BLOCKED);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_PROTECTION_EN);
        CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_wtr_while);
        CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_wtb_while);
        CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_hold_off_while);
        CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_guard_while);
        CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_flush_fdb_while);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_HOLD_OFF_WHILE);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_GUARD_WHILE);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_FLUSH_FDB_WHILE);

        if (p_db_g8032_ring->config_sync)
        {
            p_db_g8032_ring->config_sync= 0;
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_CONFIG_SYNC);
        }

        tbl_g8032_ring_del_g8032_ring(&g8032_ring.key);
    
        return rc;
    
    }

    return rc;
}


int32 g8032_delete_rpl_owner(uint32 ring_id, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }
    
    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't change rpl node role in running state");
    }

    p_db_g8032_ring->rpl_owner = 0;
    p_db_g8032_ring->east_port_is_rpl = 0;
    p_db_g8032_ring->west_port_is_rpl = 0;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_RPL_OWNER);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_RPL);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_RPL);

    if ((p_db_g8032_ring->rpl_owner && !p_db_g8032_ring->g8032_major_ring_id && !p_db_g8032_ring->is_sub_ring) 
    || ((0 != p_db_g8032_ring->g8032_major_ring_id) && !p_db_g8032_ring->virtual_channel)
    ||(p_db_g8032_ring->rpl_owner && p_db_g8032_ring->virtual_channel && p_db_g8032_ring->is_sub_ring
    &&  !p_db_g8032_ring->g8032_major_ring_id))
    {
        p_db_g8032_ring->is_fwdtocpu = TRUE;
    }
    else
    {
        p_db_g8032_ring->is_fwdtocpu = FALSE;
    }
    
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
    
    return rc;

}

int32 g8032_delete_rpl_neighbor(uint32 ring_id, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }
    
    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't change rpl node role in running state");
    }

    p_db_g8032_ring->rpl_neighbor = 0;
    p_db_g8032_ring->east_port_is_rpl_neighbor = 0;
    p_db_g8032_ring->west_port_is_rpl_neighbor = 0;

    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_RPL_NEIGHBOR);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEIGHBOR);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEIGHBOR);

    return rc;   
}

int32 g8032_set_rpl_owner(uint32 ring_id, uint8 rlp_owner_flag, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }
    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't change rpl node role in running state");
    }

    if (!rlp_owner_flag && !p_db_g8032_ring->west_port_ifindex)
    {
        CFG_CONFLICT_RET("G8032 ring %d has no west port", ring_id);
    }

    if (p_db_g8032_ring->rpl_owner)
    {
        CFG_CONFLICT_RET("The device has already been configured as rpl owner node of g8032 ring %d", ring_id);
    }

    if (p_db_g8032_ring->rpl_neighbor)
    {
        CFG_CONFLICT_RET("The device has already been configured as rpl neighbor node of g8032 ring %d", ring_id);
    }

    p_db_g8032_ring->rpl_owner = 1;

    if (rlp_owner_flag)
    {
        p_db_g8032_ring->east_port_is_rpl = 1;
        p_db_g8032_ring->west_port_is_rpl = 0;
    }
    else
    {
        p_db_g8032_ring->east_port_is_rpl = 0;
        p_db_g8032_ring->west_port_is_rpl = 1;
    }
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_RPL_OWNER);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_RPL);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_RPL);

    
    /*Set the way that RAPS packets send to CPU:
        Forward to cpu
        1.The owner node of major ring
        2.The sub-ring with no west-interface, and the mode is non-virtual-channel
        3.The sub-ring with  west-interface, and the mode is virtual-channel
    */
    if ((p_db_g8032_ring->rpl_owner && !p_db_g8032_ring->g8032_major_ring_id && !p_db_g8032_ring->is_sub_ring) 
    || ((0 != p_db_g8032_ring->g8032_major_ring_id) && !p_db_g8032_ring->virtual_channel)
    ||(p_db_g8032_ring->rpl_owner && p_db_g8032_ring->virtual_channel && p_db_g8032_ring->is_sub_ring 
    && !p_db_g8032_ring->g8032_major_ring_id))
    {
        p_db_g8032_ring->is_fwdtocpu = TRUE;
    }
    else
    {
        p_db_g8032_ring->is_fwdtocpu = FALSE;
    }
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
    

    return rc;

}

int32 g8032_delete_rpl_next_neighbor(uint32 ring_id, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }
    
    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't change rpl node role in running state");
    }

    p_db_g8032_ring->rpl_next_neighbor = 0;
    p_db_g8032_ring->east_port_is_rpl_next_neighbor = 0;
    p_db_g8032_ring->west_port_is_rpl_next_neighbor = 0;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_RPL_NEXT_NEIGHBOR);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEXT_NEIGHBOR);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEXT_NEIGHBOR);

    return rc;
}

int32 g8032_set_rpl_neighbor(uint32 ring_id, uint8 rlp_neighbor_flag, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);

    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }
    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't change rpl owner in running state");
    }

    if (!rlp_neighbor_flag && !p_db_g8032_ring->west_port_ifindex)
    {
        CFG_CONFLICT_RET("G8032 ring %d has no west port", ring_id);
    }

       if (p_db_g8032_ring->rpl_owner)
    {
        CFG_CONFLICT_RET("The device has already been configured as rpl owner node of g8032 ring %d", ring_id);
    }

    if (p_db_g8032_ring->rpl_neighbor)
    {
        CFG_CONFLICT_RET("The device has already been configured as rpl neighbor node of g8032 ring %d", ring_id);
    }
    p_db_g8032_ring->rpl_owner = 0;
    p_db_g8032_ring->rpl_neighbor = 1;
    p_db_g8032_ring->rpl_next_neighbor = 0;
    p_db_g8032_ring->east_port_is_rpl = 0;
    p_db_g8032_ring->west_port_is_rpl = 0;
    p_db_g8032_ring->east_port_is_rpl_next_neighbor = 0;
    p_db_g8032_ring->west_port_is_rpl_next_neighbor = 0;
    if (rlp_neighbor_flag)
    {
        p_db_g8032_ring->east_port_is_rpl_neighbor = 1;
        p_db_g8032_ring->west_port_is_rpl_neighbor = 0;

    }
    else
    {
        p_db_g8032_ring->east_port_is_rpl_neighbor = 0;
        p_db_g8032_ring->west_port_is_rpl_neighbor = 1;
    }
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_RPL_OWNER);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_RPL_NEIGHBOR);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_RPL);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_RPL);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_RPL_NEXT_NEIGHBOR);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEIGHBOR);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEXT_NEIGHBOR);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEIGHBOR);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEXT_NEIGHBOR);

    return rc;

}

int32 g8032_set_rpl_next_neighbor(uint32 ring_id, uint8 rlp_next_neighbor_flag, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }
    
    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't change rpl owner in running state");
    }
    p_db_g8032_ring->rpl_owner = 0;
    p_db_g8032_ring->rpl_neighbor = 0;
    p_db_g8032_ring->rpl_next_neighbor = 1;
    p_db_g8032_ring->east_port_is_rpl = 0;
    p_db_g8032_ring->west_port_is_rpl = 0;
    p_db_g8032_ring->east_port_is_rpl_neighbor = 0;
    p_db_g8032_ring->west_port_is_rpl_neighbor = 0;

    if (rlp_next_neighbor_flag)
    {
        p_db_g8032_ring->east_port_is_rpl_next_neighbor = 1;
        p_db_g8032_ring->west_port_is_rpl_next_neighbor = 0;
    }
    else
    {
        p_db_g8032_ring->east_port_is_rpl_next_neighbor = 0;
        p_db_g8032_ring->west_port_is_rpl_next_neighbor = 1;
    }
    
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_RPL_OWNER);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_RPL_NEIGHBOR);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_RPL);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_RPL);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_RPL_NEXT_NEIGHBOR);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEIGHBOR);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEXT_NEIGHBOR);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEIGHBOR);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEXT_NEIGHBOR);

    return rc;

}

int32 g8032_set_virtual_channel(uint32 ring_id, uint8 virtual_channel, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    tbl_g8032_ring_t *p_major_g8032_ring = NULL;
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_mstp_instance = NULL;
    uint32 vid = 0;
    uint32 i = 0;
    uint8 vlan_find = 0;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }

    if (!p_db_g8032_ring->is_sub_ring)
    {
        CFG_CONFLICT_RET("Can't configure virtual channel on major ring");
    }
    
    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't change virtual channel mode in running state");
    }


    if (virtual_channel && p_db_g8032_ring->control_vlan)
    {
        if (p_db_g8032_ring->g8032_major_ring_id && p_db_g8032_ring->is_sub_ring)
        {
            p_major_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t * )(&p_db_g8032_ring->g8032_major_ring_id));
            if (!p_major_g8032_ring)
            {
                CFG_CONFLICT_RET("Can not find major ring of this sub-ring");
            }

              for (i = 0; i < p_major_g8032_ring->instance_count; i++)
              {
                    inst_key.instance = p_major_g8032_ring->instance[i];
                    p_db_mstp_instance = tbl_mstp_instance_get_mstp_instance(&inst_key);
                    do {                                                                
                        for (vid= 0; vid < GLB_BMP_BITS(p_db_mstp_instance->vlan); vid++) 
                        {
                            if (GLB_BMP_ISSET((p_db_mstp_instance->vlan), (vid)))
                            {
                                if (p_db_g8032_ring->control_vlan == vid)
                                {
                                    vlan_find = TRUE;
                                    break;
                                }
                            }
                        }
                    }while (0);
                }
                
                if (!vlan_find)
                {
                    CFG_CONFLICT_RET("The control vlan of sub-ring must belong to the instance of major ring");
                }
            }

        }
    
    p_db_g8032_ring->virtual_channel = virtual_channel;    
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_VIRTUAL_CHANNEL);

   if ((p_db_g8032_ring->rpl_owner && !p_db_g8032_ring->g8032_major_ring_id && !p_db_g8032_ring->is_sub_ring) 
    || ((0 != p_db_g8032_ring->g8032_major_ring_id) && !p_db_g8032_ring->virtual_channel)
    ||(p_db_g8032_ring->rpl_owner && p_db_g8032_ring->virtual_channel && p_db_g8032_ring->is_sub_ring 
    && !p_db_g8032_ring->g8032_major_ring_id))
    {
        p_db_g8032_ring->is_fwdtocpu = TRUE;
    }
    else
    {
        p_db_g8032_ring->is_fwdtocpu = FALSE;
    }
    
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  

    return rc;
}

int32 g8032_set_revertive_mode(uint32 ring_id, uint8 revertive_mode, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't change revertive mode in running state");
    }
    
    p_db_g8032_ring->mode_revertive = revertive_mode;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_MODE_REVERTIVE);

    return rc;
}

int32 g8032_add_instance(uint32 ring_id, uint32 instance_id, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    tbl_g8032_ring_t *p_major_g8032_ring = NULL;
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_mstp_instance = NULL;
    int32 rc = PM_E_NONE;
    int32 i = 0;
    uint32 vid = 0;
    uint8 find_indtance = 0;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    sal_memset(&inst_key, 0, sizeof(inst_key));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }
    
    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't add or remove instance in running state");
    }

    for (i = 0; i< p_db_g8032_ring->instance_count; i++)
   {
        if (p_db_g8032_ring->instance[i] == instance_id)
        {
            return rc;
        }
   }
    
    if (p_db_g8032_ring->instance_count ==G8032_MAX_INSTANCE)
    {
        CFG_CONFLICT_RET("Can't add more than 4 instance to a ring");
    }

    inst_key.instance = instance_id;
    p_db_mstp_instance = tbl_mstp_instance_get_mstp_instance(&inst_key);

    if(!p_db_mstp_instance)
    {
        CFG_CONFLICT_RET("The instance does not exist, please create it first");
    }

    if (p_db_mstp_instance->g8032_ring_id && !p_db_g8032_ring->is_sub_ring 
        && p_db_mstp_instance->g8032_ring_id != ring_id)
    {
        CFG_CONFLICT_RET("The instance has been bound to g8032 ring %d", p_db_mstp_instance->g8032_ring_id );
    }

    do {                                                                
            for (vid= 0; vid < GLB_BMP_BITS(p_db_mstp_instance->vlan); vid++) 
            {
                if (GLB_BMP_ISSET((p_db_mstp_instance->vlan), (vid)))
                {
                    if (p_db_g8032_ring->control_vlan == vid)
                    {
                        CFG_CONFLICT_RET("The control vlan of ring can not belong to the instance of ring");
                        break;
                    }
                }
            }
        }while (0);

    if (p_db_g8032_ring->is_sub_ring && p_db_g8032_ring->g8032_major_ring_id)
    {
        p_major_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t * )& p_db_g8032_ring->g8032_major_ring_id);
        if (p_major_g8032_ring)
        {
            for (i = 0; i<p_major_g8032_ring->instance_count; i++)
            {
                if (instance_id != p_major_g8032_ring->instance[i])
                {
                    continue;
                }
                find_indtance = TRUE;
            }
        }
        if (!find_indtance)
        {
            CFG_CONFLICT_RET("The instance of sub-ring must belong to the protection instance of major ring");
        }
    }

    p_db_g8032_ring->instance_count++;
    p_db_g8032_ring->instance[p_db_g8032_ring->instance_count-1] = instance_id;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_INSTANCE_COUNT);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_INSTANCE);
    if (!p_db_g8032_ring->g8032_major_ring_id)
    {
        p_db_mstp_instance->g8032_ring_id = p_db_g8032_ring->key.ring_id;
        tbl_mstp_instance_set_mstp_instance_field(p_db_mstp_instance, TBL_MSTP_INSTANCE_FLD_G8032_RING_ID);
    }

    return rc;
}


int32 g8032_set_control_vlan(uint32 ring_id, uint32 vlan_id, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    tbl_g8032_ring_t *p_major_g8032_ring = NULL;
    int32 rc = PM_E_NONE;
    uint32  i =0;
    uint32 vid = 0;
    uint8 vlan_find = 0;
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_mstp_instance = NULL;
    tbl_g8032_ring_master_t *p_g8032_master = NULL;
    ctclib_slistnode_t *listnode = NULL;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    sal_memset(&inst_key, 0, sizeof(inst_key));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);

    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }

    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't set control vlan in running state");
    }

    sal_memset(&vlan_key, 0, sizeof(vlan_key));
    vlan_key.vid = vlan_id & 0xFFFF;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        CFG_CONFLICT_RET("VLAN %d does not exist", vlan_id);
    }

    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf((tbl_igsp_intf_key_t * )&(vlan_key.vid));
    if (p_db_igsp_if && p_db_igsp_if->enable)
    {
        CFG_CONFLICT_RET("The igmp snooping has been enabled on VLAN %d, please disable it first", vlan_id);
    }


    p_g8032_master = tbl_g8032_ring_get_master();
    CTCLIB_SLIST_LOOP(p_g8032_master->g8032_ring_list, p_g8032_ring, listnode)
    {
        if (p_g8032_ring->control_vlan == vlan_id)
        {
            CFG_CONFLICT_RET("The VLAN %d has been configured as control-vlan of other ring", vlan_id);
            break;
        }
    }

    if (p_db_g8032_ring->g8032_major_ring_id && p_db_g8032_ring->is_sub_ring)
    {
        p_major_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t * )(&p_db_g8032_ring->g8032_major_ring_id));
        if (!p_major_g8032_ring)
        {
            CFG_CONFLICT_RET("Can not find major ring of this sub-ring");
        }

        if (p_db_g8032_ring->virtual_channel)
        {
            for (i = 0; i < p_major_g8032_ring->instance_count; i++)
            {
                inst_key.instance = p_major_g8032_ring->instance[i];
                p_db_mstp_instance = tbl_mstp_instance_get_mstp_instance(&inst_key);
                do {                                                                
                    for (vid= 0; vid < GLB_BMP_BITS(p_db_mstp_instance->vlan); vid++) 
                    {
                        if (GLB_BMP_ISSET((p_db_mstp_instance->vlan), (vid)))
                        {
                            if (vlan_id == vid)
                            {
                                vlan_find = TRUE;
                                break;
                            }
                        }
                    }
                }while (0);
            }
            
            if (!vlan_find)
            {
                CFG_CONFLICT_RET("The control vlan of sub-ring must belong to the instance of major ring in virtual_channel mode");
            }
        }

       
        for (i = 0; i < p_db_g8032_ring->instance_count; i++)
        {
            inst_key.instance = p_db_g8032_ring->instance[i];
            p_db_mstp_instance = tbl_mstp_instance_get_mstp_instance(&inst_key);
            do {                                                                
                    for (vid= 0; vid < GLB_BMP_BITS(p_db_mstp_instance->vlan); vid++) 
                    {
                        if (GLB_BMP_ISSET((p_db_mstp_instance->vlan), (vid)))
                        {
                            if (vlan_id == vid)
                            {
                                CFG_CONFLICT_RET("The control vlan of sub-ring can not belong to the instance of sub-ring");
                                break;
                            }
                        }
                    }
            }while (0);
        }

    }
    else
    {
         for (i = 0; i < p_db_g8032_ring->instance_count; i++)
        {
            inst_key.instance = p_db_g8032_ring->instance[i];
            p_db_mstp_instance = tbl_mstp_instance_get_mstp_instance(&inst_key);
            do {                                                                
                        for (vid= 0; vid < GLB_BMP_BITS(p_db_mstp_instance->vlan); vid++) 
                        {
                            if (GLB_BMP_ISSET((p_db_mstp_instance->vlan), (vid)))
                            {
                                if (vlan_id == vid)
                                {
                                    CFG_CONFLICT_RET("The control vlan of ring can not belong to the instance of ring");
                                    break;
                                }
                            }
                        }
            }while (0);
        }
    }

    g8032_delete_static_fdb_by_control_vlan(vlan_key.vid);
    p_db_g8032_ring->control_vlan = vlan_id;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_CONTROL_VLAN);

    p_db_vlan->g8032_ring_id = p_db_g8032_ring->key.ring_id;    
    tbl_vlan_set_vlan_field(p_db_vlan, TBL_VLAN_FLD_G8032_RING_ID);

    if ((p_db_g8032_ring->rpl_owner && !p_db_g8032_ring->g8032_major_ring_id && !p_db_g8032_ring->is_sub_ring) 
    || ((0 != p_db_g8032_ring->g8032_major_ring_id) && !p_db_g8032_ring->virtual_channel)
    ||(p_db_g8032_ring->rpl_owner && p_db_g8032_ring->virtual_channel && p_db_g8032_ring->is_sub_ring &&  !p_db_g8032_ring->g8032_major_ring_id))
    {
        p_db_g8032_ring->is_fwdtocpu = TRUE;
    }
    else
    {
        p_db_g8032_ring->is_fwdtocpu = FALSE;
    }
    
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  

    g8032_delete_static_fdb_by_control_vlan(vlan_key.vid);
    
    return rc;
}

int32 g8032_set_wtr_timer_interval(uint32 ring_id, uint32 interval, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }
    
    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't change wtr timer interval value in running state");
    }

    p_db_g8032_ring->wtr_timeout_val = interval;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WTR_TIMEOUT_VAL);

    return rc;
}

int32 g8032_set_hold_off_timer_interval(uint32 ring_id, uint32 interval, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }
    
    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't change hold-off timer interval value in running state");
    }

    p_db_g8032_ring->hold_off_timeout_val= interval;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_HOLD_OFF_TIMEOUT_VAL);

    return rc;
}

int32 g8032_set_guard_timer_interval(uint32 ring_id, uint32 interval, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }
    
    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't change guard timer interval value in running state");
    }

    p_db_g8032_ring->guard_timeout_val = interval;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_GUARD_TIMEOUT_VAL);
    p_db_g8032_ring->wtb_timeout_val = p_db_g8032_ring->guard_timeout_val + 5000;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_GUARD_TIMEOUT_VAL);

    return rc;
}

int32 g8032_set_ring_disable(tbl_g8032_ring_t *p_db_g8032_ring)
{
    if (!p_db_g8032_ring)
    {
        return PM_E_NONE;
    }

    if (!p_db_g8032_ring->protection_en)
    {
        return PM_E_NONE;
    }
    
    g8032_fsm_stop_transmit_raps(p_db_g8032_ring);
    p_db_g8032_ring->east_port_is_blocked = FALSE;
    if (p_db_g8032_ring->west_port_ifindex)
    {
        p_db_g8032_ring->west_port_is_blocked = FALSE;
    }
    p_db_g8032_ring->protection_en= FALSE;
    p_db_g8032_ring->current_node_state = G8032_FSM_STATE_MAX;
  
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_BLOCKED);
    if (p_db_g8032_ring->west_port_ifindex)
    {
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_BLOCKED);
    }

    p_db_g8032_ring->east_port_is_fail = FALSE;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_FAIL);

    p_db_g8032_ring->west_port_is_fail = FALSE;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_FAIL);
            
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_PROTECTION_EN);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_CURRENT_NODE_STATE);

    g8032_fsm_clear_db_ms_info(p_db_g8032_ring);
    g8032_fsm_clear_db_fs_info(p_db_g8032_ring);

    sal_memset(p_db_g8032_ring->east_port_rec_node_id, 0, 6);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID);
    p_db_g8032_ring->east_port_rec_bpr = 0;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_BPR);

    sal_memset(p_db_g8032_ring->west_port_rec_node_id, 0, 6);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_REC_NODE_ID);
    p_db_g8032_ring->west_port_rec_bpr = 0;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_REC_BPR);
    CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_wtr_while);
    CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_wtb_while);
    CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_hold_off_while);
    CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_guard_while);
    CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_flush_fdb_while);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_HOLD_OFF_WHILE);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_GUARD_WHILE);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_FLUSH_FDB_WHILE);

    return PM_E_NONE;
}

int32 g8032_set_ring_enable(uint32 ring_id, uint8 ring_enable, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    tbl_g8032_ring_t *p_major_g8032_ring = NULL;
    tbl_g8032_ring_t *p_sub_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }

    if (!p_db_g8032_ring->control_vlan)
    {
        CFG_CONFLICT_RET("No control-vlan has been configured on ring %d ", g8032_ring.key.ring_id);
    }

    if (!p_db_g8032_ring->instance_count)
    {
        CFG_CONFLICT_RET("No instance has been configured on ring %d", g8032_ring.key.ring_id);
    }

    if (ring_enable)
    {
        if (p_db_g8032_ring->is_sub_ring && p_db_g8032_ring->g8032_major_ring_id)
        {
            g8032_ring.key.ring_id = p_db_g8032_ring->g8032_major_ring_id;
            p_major_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
            if (p_major_g8032_ring && !p_major_g8032_ring->protection_en)
            {
                CFG_CONFLICT_RET("The major ring of this sub-ring has not been enabled, please enable it first");
            }
        }
        if ((p_db_g8032_ring->rpl_owner && !p_db_g8032_ring->g8032_major_ring_id && !p_db_g8032_ring->is_sub_ring) 
        || ((0 != p_db_g8032_ring->g8032_major_ring_id) && !p_db_g8032_ring->virtual_channel)
        ||(p_db_g8032_ring->rpl_owner && p_db_g8032_ring->virtual_channel && p_db_g8032_ring->is_sub_ring 
        && !p_db_g8032_ring->g8032_major_ring_id)
        || (p_db_g8032_ring->rpl_owner && p_db_g8032_ring->virtual_channel && p_db_g8032_ring->g8032_major_ring_id))
        {
            p_db_g8032_ring->is_fwdtocpu = TRUE;
        }
        else
        {
            p_db_g8032_ring->is_fwdtocpu = FALSE;
        }
        
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU); 
        if (!p_db_g8032_ring->config_sync && !p_db_g8032_ring->protection_en)
        {
            p_db_g8032_ring->protection_en= ring_enable;
            p_db_g8032_ring->config_sync = 1;
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_PROTECTION_EN);
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_CONFIG_SYNC);
            g8032_fsm_init (p_db_g8032_ring);
        }
        else if (p_db_g8032_ring->config_sync && !p_db_g8032_ring->protection_en)
        {
            p_db_g8032_ring->protection_en= ring_enable;
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_PROTECTION_EN);
            g8032_fsm_init (p_db_g8032_ring);
        }
        
    }
    else
    {
        if (!p_db_g8032_ring->is_sub_ring)
        {
            if (p_db_g8032_ring->sub_ring_id[0])
            {
                g8032_ring.key.ring_id = p_db_g8032_ring->sub_ring_id[0];
                p_sub_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
                if (p_sub_g8032_ring && p_sub_g8032_ring->protection_en)
                {
                    CFG_CONFLICT_RET("The sub-ring of this major ring has not been disabled, please disable it first");
                }
            }
        }
        
        if (!p_db_g8032_ring->protection_en)
        {
            CFG_CONFLICT_RET("Ring %d has not been enabled", p_db_g8032_ring->key.ring_id);
        }

        g8032_set_ring_disable(p_db_g8032_ring);
    }
    
    return rc;
}

int32 g8032_set_ring_level_id(uint32 ring_id, uint32 level_id, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);

    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }

    if (p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Can't change wait to ring level in running state");
    }
    p_db_g8032_ring->ring_level_id = level_id;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_RING_LEVEL_ID);

    return rc;
}

int32 g8032_set_fs_port(uint32 ring_id, uint8 is_east, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    uint32 req_state = 0;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);

    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }

    if (!p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Ring %d has not been enabled", g8032_ring.key.ring_id);
    }

    p_db_g8032_ring->is_local_fs = TRUE;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_IS_LOCAL_FS);

    if (is_east)
    {

        p_db_g8032_ring->east_port_is_force_switch = TRUE;
        p_db_g8032_ring->west_port_is_force_switch = FALSE;
        p_db_g8032_ring->east_port_is_manual_switch = FALSE;
        p_db_g8032_ring->west_port_is_manual_switch = FALSE;
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_FORCE_SWITCH);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_FORCE_SWITCH);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_MANUAL_SWITCH);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_MANUAL_SWITCH);

        p_db_g8032_ring->is_local_ms = FALSE;
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_IS_LOCAL_MS);

        req_state = G8032_PRI_REQ_L_FS;
        G8032FSM[p_db_g8032_ring->current_node_state][req_state].func(p_db_g8032_ring, NULL, req_state, TRUE);
    }
    else
    {
        if(!p_db_g8032_ring->west_port_ifindex)
        {
            CFG_CONFLICT_RET("Ring %d has no west-interface", g8032_ring.key.ring_id);
        }

        p_db_g8032_ring->west_port_is_force_switch = TRUE;
        p_db_g8032_ring->east_port_is_force_switch = FALSE;
        p_db_g8032_ring->east_port_is_manual_switch = FALSE;
        p_db_g8032_ring->west_port_is_manual_switch = FALSE;
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_FORCE_SWITCH);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_FORCE_SWITCH);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_MANUAL_SWITCH);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_MANUAL_SWITCH);

        req_state = G8032_PRI_REQ_L_FS;
        G8032FSM[p_db_g8032_ring->current_node_state][req_state].func(p_db_g8032_ring, NULL, req_state,FALSE);
    }

    return rc;
}

int32 
g8032_set_local_clear(uint32 ring_id, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    uint32 req_state = 0;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }

    if (!p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Ring %d has not been enabled", g8032_ring.key.ring_id);
    }

    if (p_db_g8032_ring->east_port_is_manual_switch)
    {
        p_db_g8032_ring->east_port_is_manual_switch = FALSE;
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_MANUAL_SWITCH);
    }

    if (p_db_g8032_ring->east_port_is_force_switch)
    {
        p_db_g8032_ring->east_port_is_manual_switch = FALSE;
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_FORCE_SWITCH);
    }

    if (p_db_g8032_ring->west_port_is_manual_switch);
    {
        p_db_g8032_ring->west_port_is_manual_switch = FALSE;
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_MANUAL_SWITCH);
    }

    if (p_db_g8032_ring->west_port_is_force_switch);
    {
        p_db_g8032_ring->west_port_is_force_switch = FALSE;
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_FORCE_SWITCH);
    }

    p_db_g8032_ring->is_local_ms = FALSE;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_IS_LOCAL_MS);
    p_db_g8032_ring->is_local_fs = FALSE;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_IS_LOCAL_FS);
    req_state = G8032_PRI_REQ_L_CLEAR;
    G8032FSM[p_db_g8032_ring->current_node_state][req_state].func(p_db_g8032_ring, NULL, req_state, FALSE);

    return rc;
}

int32 
g8032_set_ms_port(uint32 ring_id, uint8 is_east, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    uint32 req_state = 0;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);

    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }

    if (!p_db_g8032_ring->protection_en)
    {
        CFG_CONFLICT_RET("Ring %d has not been enabled", g8032_ring.key.ring_id);
    }

    if (!is_east)
    {
        if(!p_db_g8032_ring->west_port_ifindex)
        {
            CFG_CONFLICT_RET("Ring %d has no west-interface", g8032_ring.key.ring_id);
        } 
    }
    if (p_db_g8032_ring->current_node_state == G8032_NODE_STATE_PROTECTION 
    || p_db_g8032_ring->current_node_state == G8032_NODE_STATE_FORCE_SWITCH
    || p_db_g8032_ring->current_node_state == G8032_NODE_STATE_MANUAL_SWITCH )
    {
        return rc;
    }

    if (is_east)
    {
        if (p_db_g8032_ring->current_node_state != G8032_NODE_STATE_PROTECTION 
        && p_db_g8032_ring->current_node_state != G8032_NODE_STATE_FORCE_SWITCH
        &&  p_db_g8032_ring->current_node_state != G8032_NODE_STATE_MANUAL_SWITCH )
        {
            p_db_g8032_ring->is_local_ms = TRUE;
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_IS_LOCAL_MS);
            p_db_g8032_ring->east_port_is_manual_switch = TRUE;
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_MANUAL_SWITCH);
        }

        req_state = G8032_PRI_REQ_L_MS;
        G8032FSM[p_db_g8032_ring->current_node_state][req_state].func(p_db_g8032_ring, NULL, req_state, TRUE);
    }
    else
    {
        if (p_db_g8032_ring->current_node_state != G8032_NODE_STATE_PROTECTION 
        && p_db_g8032_ring->current_node_state != G8032_NODE_STATE_FORCE_SWITCH
        &&  p_db_g8032_ring->current_node_state != G8032_NODE_STATE_MANUAL_SWITCH )
        {
            p_db_g8032_ring->is_local_ms = TRUE;
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_IS_LOCAL_MS);
            p_db_g8032_ring->west_port_is_manual_switch = TRUE;
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_MANUAL_SWITCH);
        }

        req_state = G8032_PRI_REQ_L_MS;
        G8032FSM[p_db_g8032_ring->current_node_state][req_state].func(p_db_g8032_ring, NULL, req_state, FALSE);
    }
    
    return rc;    
}

int32 g8032_ring_add_sub_ring(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_east_interface = NULL;
    tbl_interface_t *p_west_interface = NULL;
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    tbl_route_if_t *p_db_rtif = NULL;
    tbl_mstp_port_t * p_mstp_port = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    p_east_interface = tbl_interface_get_interface_by_name(argv[3]); 
    g8032_ring.key.ring_id = sal_atoi(argv[1]);
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);

    if (p_db_g8032_ring)
    {
        if (!p_db_g8032_ring->is_sub_ring)
        {
            CFG_INVALID_PARAM_RET("G8032 ring %d has been configured as major ring", p_db_g8032_ring->key.ring_id);
        }
        
        p_east_interface = tbl_interface_get_interface_by_ifindex(p_db_g8032_ring->east_port_ifindex);
        if (NULL == p_east_interface)
        {
            CFG_INVALID_PARAM_RET("The east-interface and west-interface can not be modified");
        }

        if (sal_strcmp((p_east_interface->key.name),argv[3]))
        {
            CFG_INVALID_PARAM_RET("The east-interface and west-interface can not be modified");
        }

        if ((p_db_g8032_ring->g8032_major_ring_id != sal_atoi(argv[5])))
        {
            CFG_INVALID_PARAM_RET("The major ring-id of sub-ring can not be modified");
        }

        return rc;
    }

    if (NULL == p_east_interface)
    {
        CFG_INVALID_PARAM_RET("The interface does not exist");
    }

    if ((GLB_VLAN_PORT_TYPE_TRUNK != p_east_interface->vlan_type) && (GLB_VLAN_PORT_TYPE_QINQ != p_east_interface->vlan_type))
    {
        CFG_INVALID_PARAM_RET("Interface is not a trunk port or dot1q-tunnel port");
    }

    p_mstp_port = tbl_mstp_port_get_mstp_port((tbl_mstp_port_key_t * )(p_east_interface->key.name));
    if (p_mstp_port && p_mstp_port->port_enable)
    {
        CFG_INVALID_PARAM_RET("Can't configure on this interface, MSTP,ERPS, vlan translation has been configured on it");
    }

    if (p_east_interface->mstp_port ||  p_east_interface->erps_enable ||p_east_interface-> brgif->vlan_translation_en)
    {
        CFG_INVALID_PARAM_RET("Can't configure on this interface, MSTP,ERPS, vlan translation has been configured on it");
    }

    if (p_east_interface->mirror_dest_id )
    {
        CFG_INVALID_PARAM_RET("Can't configure on this interface, the interface has been configured as mirror destination");
    }

    if (INTERFACE_IS_LAG_MEMBER(p_east_interface))
    {
        CFG_INVALID_PARAM_RET("East interface is aggregator member port");
    }
    
    g8032_ring.key.ring_id = sal_atoi(argv[5]);
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);         
    if (NULL == p_db_g8032_ring)
    {
        CFG_INVALID_PARAM_RET("The major ring does not exist, please create it first");
    }

    if (p_db_g8032_ring->sub_ring_id[0])
    {
        CFG_INVALID_PARAM_RET("Can not configure more than one sub-ring in a major ring");
    }

    if (p_db_g8032_ring->is_sub_ring)
    {
        CFG_INVALID_PARAM_RET("Can not configure sub-ring in a sub-ring");
    }

    p_east_interface = tbl_interface_get_interface_by_ifindex(p_db_g8032_ring->east_port_ifindex);
    p_west_interface = tbl_interface_get_interface_by_ifindex(p_db_g8032_ring->west_port_ifindex);
    if (!sal_strcmp(p_east_interface->key.name, argv[3]) || !sal_strcmp(p_west_interface->key.name, argv[3]))
    {
        CFG_INVALID_PARAM_RET("The interface of sub-ring can not be same as the interface of major ring");
    }

    p_east_interface = tbl_interface_get_interface_by_name(argv[3]);
    if (NULL == p_east_interface)
    {
        CFG_INVALID_PARAM_RET("The interface %s does not exist", argv[3]);
    }

    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)p_east_interface->key.name);
    if (p_db_rtif)
    {
        CFG_INVALID_PARAM_RET("The interface is L3 interface");
    }

    g8032_ring.key.ring_id = sal_atoi(argv[1]);
    g8032_ring.east_port_ifindex = p_east_interface->ifindex;
    g8032_ring.g8032_major_ring_id = sal_atoi(argv[5]);
    g8032_ring.is_sub_ring = 1;
    g8032_ring.guard_timeout_val = G8032_RING_DEFAULT_GUARD_TIMER_INTERVAL_MS;
    g8032_ring.wtb_timeout_val = G8032_RING_DEFAULT_GUARD_TIMER_INTERVAL_MS + 5000;
    g8032_ring.hold_off_timeout_val = G8032_RING_HOLD_MIN_TIMEOUT_MS;
    g8032_ring.wtr_timeout_val = G8032_RING_DEFAULT_WTR_TIMER_INTERVAL_IN_MIN;
    g8032_ring.ring_level_id = G8032_DEF_RAPS_MEL_VALUE;
    g8032_ring.mode_revertive = TRUE;
    g8032_ring.current_node_state = G8032_FSM_STATE_MAX;
    tbl_g8032_ring_add_g8032_ring(&g8032_ring);

    g8032_ring.key.ring_id = sal_atoi(argv[5]);
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (p_db_g8032_ring)
    {
        p_db_g8032_ring->sub_ring_id[0] = sal_atoi(argv[1]);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_SUB_RING_ID);
    }
    p_east_interface->current_g8032_ring_count++;
    p_east_interface->g8032_ring_first[p_east_interface->current_g8032_ring_count-1] = sal_atoi(argv[1]);
    tbl_interface_set_interface_field(p_east_interface, TBL_INTERFACE_FLD_G8032_RING_FIRST);
    tbl_interface_set_interface_field(p_east_interface, TBL_INTERFACE_FLD_CURRENT_G8032_RING_COUNT);
    return rc;
}

int32 g8032_ring_add_ring(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_east_interface = NULL;
    tbl_interface_t *p_west_interface = NULL;
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    tbl_route_if_t *p_db_rtif = NULL;
    tbl_mstp_port_t * p_mstp_port = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = sal_atoi(argv[0]);
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (p_db_g8032_ring)
    {
        p_east_interface = tbl_interface_get_interface_by_ifindex(p_db_g8032_ring->east_port_ifindex);
        p_west_interface = tbl_interface_get_interface_by_ifindex(p_db_g8032_ring->west_port_ifindex);

        if (NULL == p_east_interface || NULL == p_west_interface)
        {
            CFG_INVALID_PARAM_RET("The east-interface and west-interface can not be modified");
        }

        if (sal_strcmp((p_east_interface->key.name),argv[2]) || sal_strcmp((p_west_interface->key.name),argv[4]))
        {
            CFG_INVALID_PARAM_RET("The east-interface and west-interface can not be modified");
        }

        if (6 == argc && !p_db_g8032_ring->is_sub_ring)
        {
            CFG_INVALID_PARAM_RET("Can not change major ring to sub-ring");
        }
        return rc;
    }

    p_east_interface = tbl_interface_get_interface_by_name(argv[2]);
    p_west_interface = tbl_interface_get_interface_by_name(argv[4]);

    if (NULL == p_east_interface)
    {
        CFG_INVALID_PARAM_RET("The interface %s does not exist", argv[2]); 
    }

    if (NULL == p_west_interface)
    {
        CFG_INVALID_PARAM_RET("The interface %s does not exist", argv[4]); 
    }

    if (p_east_interface->ifindex == p_west_interface->ifindex)
    {
        CFG_INVALID_PARAM_RET("East interface and west interface are same"); 
    }

    if (p_east_interface->current_g8032_ring_count >= G8032_RING_MAX_VALUE_ON_PORT)
    {
        CFG_INVALID_PARAM_RET("Can't configure more than 2 g8032 ring on one port");
    }

    if (p_west_interface->current_g8032_ring_count >= G8032_RING_MAX_VALUE_ON_PORT)
    {
        CFG_INVALID_PARAM_RET("Can't configure more than 2 g8032 ring on one port");
    }
    
    if (INTERFACE_IS_LAG_MEMBER(p_east_interface))
    {
        CFG_INVALID_PARAM_RET("East interface is aggregator member port");
    }
    
    if (INTERFACE_IS_LAG_MEMBER(p_west_interface))
    {
        CFG_INVALID_PARAM_RET("West interface is aggregator member port");
    }
        
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)p_east_interface->key.name);
    if (p_db_rtif)
    {
        CFG_INVALID_PARAM_RET("The east interface is L3 interface");
    }

    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)p_west_interface->key.name);
    if (p_db_rtif)
    {
        CFG_INVALID_PARAM_RET("The west interface is L3 interface");
    }

    if ((GLB_VLAN_PORT_TYPE_TRUNK != p_east_interface->vlan_type) && (GLB_VLAN_PORT_TYPE_QINQ != p_east_interface->vlan_type))
    {
        CFG_INVALID_PARAM_RET("Interface is not a trunk port or dot1q-tunnel port");
    }

    if ((GLB_VLAN_PORT_TYPE_TRUNK != p_west_interface->vlan_type) && (GLB_VLAN_PORT_TYPE_QINQ != p_west_interface->vlan_type))
    {
        CFG_INVALID_PARAM_RET("Interface is not a trunk port or dot1q-tunnel port");
    }

    p_mstp_port = tbl_mstp_port_get_mstp_port((tbl_mstp_port_key_t * )(p_east_interface->key.name));
    if (p_mstp_port && p_mstp_port->port_enable)
    {
        CFG_INVALID_PARAM_RET("Can't configure on this interface, MSTP,ERPS, vlan translation has been configured on it");
    }

    p_mstp_port = tbl_mstp_port_get_mstp_port((tbl_mstp_port_key_t * )(p_west_interface->key.name));
    if (p_mstp_port && p_mstp_port->port_enable)
    {
        CFG_INVALID_PARAM_RET("Can't configure on this interface, MSTP,ERPS, vlan translation has been configured on it");
    }

    if (p_east_interface->mstp_port || p_west_interface->mstp_port || p_east_interface->erps_enable ||  p_west_interface->erps_enable 
    ||p_east_interface-> brgif->vlan_translation_en || p_west_interface-> brgif->vlan_translation_en)
    {
        CFG_INVALID_PARAM_RET("Can't configure on this interface, MSTP,ERPS, vlan translation has been configured on it");
    }

    if (p_east_interface->mirror_dest_id || p_west_interface->mirror_dest_id)
    {
        CFG_INVALID_PARAM_RET("Can't configure on this interface, the interface has been configured as mirror destination");
    }

    g8032_ring.east_port_ifindex = p_east_interface->ifindex;
    g8032_ring.west_port_ifindex = p_west_interface->ifindex;
    if (6 == argc)
    {
        g8032_ring.is_sub_ring = 1;
    }

    g8032_ring.guard_timeout_val = G8032_RING_DEFAULT_GUARD_TIMER_INTERVAL_MS;
    g8032_ring.wtb_timeout_val = G8032_RING_DEFAULT_GUARD_TIMER_INTERVAL_MS + 5000;
    g8032_ring.hold_off_timeout_val = G8032_RING_HOLD_MIN_TIMEOUT_MS;
    g8032_ring.wtr_timeout_val = G8032_RING_DEFAULT_WTR_TIMER_INTERVAL_IN_MIN;
    g8032_ring.ring_level_id = G8032_DEF_RAPS_MEL_VALUE;
    g8032_ring.mode_revertive = TRUE;
    g8032_ring.current_node_state = G8032_FSM_STATE_MAX;
    tbl_g8032_ring_add_g8032_ring(&g8032_ring);
    p_east_interface->current_g8032_ring_count++;
    p_east_interface->g8032_ring_first[p_east_interface->current_g8032_ring_count-1] = g8032_ring.key.ring_id;
    tbl_interface_set_interface_field(p_east_interface, TBL_INTERFACE_FLD_G8032_RING_FIRST);
    tbl_interface_set_interface_field(p_east_interface, TBL_INTERFACE_FLD_CURRENT_G8032_RING_COUNT);
    p_west_interface->current_g8032_ring_count++;
    p_west_interface->g8032_ring_first[p_west_interface->current_g8032_ring_count-1] = g8032_ring.key.ring_id;    
    tbl_interface_set_interface_field(p_west_interface, TBL_INTERFACE_FLD_G8032_RING_FIRST);
    tbl_interface_set_interface_field(p_west_interface, TBL_INTERFACE_FLD_CURRENT_G8032_RING_COUNT);

    return rc;
}

int32 g8032_ring_get_cmd_operation_field(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t g8032_ring;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    uint32 ring_id = 0;
    int32 rc = PM_E_NONE;

    ring_id = sal_atoi(argv[0]);
    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (NULL == p_db_g8032_ring)
    {
        CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
    }
    
    if (1 == argc)
    {
        rc = TBL_G8032_RING_FLD_KEY;
    }
    if (2 == argc)
    {
        if (!sal_strcmp(argv[1], "rpl_owner")) 
        {
        rc = TBL_G8032_RING_FLD_RPL_OWNER;
        }
        if (!sal_strcmp(argv[1], "rpl_next_neighbor")) 
        {
        rc = TBL_G8032_RING_FLD_RPL_NEXT_NEIGHBOR;
        }

        if (!sal_strcmp(argv[1], "rpl_neighbor")) 
        {
        rc = TBL_G8032_RING_FLD_RPL_NEIGHBOR;
        }

        if (!sal_strcmp(argv[1], "control-vlan")) 
        {
        rc = TBL_G8032_RING_FLD_CONTROL_VLAN;
        }
    }
    
    if (3 == argc)
    {
        if (!sal_strcmp(argv[1], "virtual-channel"))
        {
        rc = TBL_G8032_RING_FLD_VIRTUAL_CHANNEL;
        }

        if (!sal_strcmp(argv[1], "revertive-mode"))
        {
        rc = TBL_G8032_RING_FLD_MODE_REVERTIVE;
        }

        if (!sal_strcmp(argv[2], "rpl_owner")) 
        {
            if (!sal_strcmp(argv[1], "east-interface"))
            {
            rc = TBL_G8032_RING_FLD_EAST_PORT_IS_RPL;
            }

            if (!sal_strcmp(argv[1], "west-interface"))
            {
            rc = TBL_G8032_RING_FLD_WEST_PORT_IS_RPL;
            }
        }
        if (!sal_strcmp(argv[2], "rpl_next_neighbor")) 
        {
            if (!sal_strcmp(argv[1], "east-interface"))
            {
            rc = TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEXT_NEIGHBOR;
            }

            if (!sal_strcmp(argv[1], "west-interface"))
            {
            rc = TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEXT_NEIGHBOR;
            }
        }


        if (!sal_strcmp(argv[2], "rpl_neighbor")) 
        {
            if (!sal_strcmp(argv[1], "east-interface"))
            {
            rc = TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEIGHBOR;
            }

            if (!sal_strcmp(argv[1], "west-interface"))
            {
                rc = TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEIGHBOR;
            }
        }

        if (!sal_strcmp(argv[1], "instance_id"))  
        {
            rc = TBL_G8032_RING_FLD_INSTANCE; 
        }

        if (!sal_strcmp(argv[1], "control-vlan")) 
        {
            rc = TBL_G8032_RING_FLD_CONTROL_VLAN; 
        }

        if (!sal_strcmp(argv[1], "wait_to_restore")) 
        {
            rc = TBL_G8032_RING_FLD_WTR_TIMEOUT_VAL; 
        }

        if (!sal_strcmp(argv[1], "hold_off")) 
        {
            rc = TBL_G8032_RING_FLD_HOLD_OFF_TIMEOUT_VAL; 
        }

        if (!sal_strcmp(argv[1], "guard_time")) 
        {
            rc = TBL_G8032_RING_FLD_GUARD_TIMEOUT_VAL; 
        }

        if (!sal_strcmp(argv[1], "protection_en")) 
        {
            rc = TBL_G8032_RING_FLD_PROTECTION_EN; 
        }

        if (!sal_strcmp(argv[1], "level_id")) 
        {
            rc = TBL_G8032_RING_FLD_RING_LEVEL_ID; 
        }

        if (!sal_strcmp(argv[1], "fs_port") && !sal_strcmp(argv[2], "east-interface") ) 
        {
            rc = TBL_G8032_RING_FLD_EAST_PORT_IS_FORCE_SWITCH; 
        }

        if (!sal_strcmp(argv[1], "fs_port") && !sal_strcmp(argv[2], "west-interface") ) 
        {
            rc = TBL_G8032_RING_FLD_WEST_PORT_IS_FORCE_SWITCH; 
        }

        if (!sal_strcmp(argv[1], "ms_port") && !sal_strcmp(argv[2], "east-interface") ) 
        {
            rc = TBL_G8032_RING_FLD_EAST_PORT_IS_MANUAL_SWITCH; 
        }

        if (!sal_strcmp(argv[1], "ms_port") && !sal_strcmp(argv[2], "west-interface") ) 
        {
            rc = TBL_G8032_RING_FLD_WEST_PORT_IS_MANUAL_SWITCH; 
        }
    } 

    if (4 == argc)
    {
        if (!sal_strcmp(argv[1], "control-vlan")) 
        {
            rc = TBL_G8032_RING_FLD_CONTROL_VLAN; 
        }
    }

    /* modified by liwh for bug 53010, 2019-07-22 */
    if ((argc >= 2) && (!sal_strcmp(argv[1], "cfm_md_name")))
    {
        rc = TBL_G8032_RING_FLD_CFM_MD_NAME; 
    }
    /* liwh end */
    
    return rc ;
}

/* system modified by liwh for bug 53010, 2019-08-07 */
int32 g8032_delete_cfm_association(tbl_g8032_ring_t *p_db_g8032_ring)
{
    tbl_cfm_lmep_key_t           lmep_key;
    tbl_cfm_lmep_t                  *p_east_lmep = NULL;
    tbl_cfm_lmep_t                  *p_west_lmep = NULL;
    
    /* delete east mep ring_id */
    lmep_key.level = p_db_g8032_ring->level;
    lmep_key.vlan_id = p_db_g8032_ring->vlan_id;
    lmep_key.ifindex = p_db_g8032_ring->east_port_ifindex;    
    p_east_lmep = tbl_cfm_lmep_get_cfm_lmep(&lmep_key);
    if (p_east_lmep)
    {
        /* clear ring id for east mep */
        p_east_lmep->ring_id = 0;
        tbl_cfm_lmep_set_cfm_lmep_field(p_east_lmep, TBL_CFM_LMEP_FLD_RING_ID);
    }

    /* delete west mep ring_id */
    lmep_key.ifindex = p_db_g8032_ring->west_port_ifindex;    
    p_west_lmep = tbl_cfm_lmep_get_cfm_lmep(&lmep_key);
    if (p_west_lmep)
    {   
        /* clear ring id for west mep */
        p_west_lmep->ring_id = 0;
        tbl_cfm_lmep_set_cfm_lmep_field(p_west_lmep, TBL_CFM_LMEP_FLD_RING_ID); 
    }

    /* clear md/ma name */
    sal_memset(p_db_g8032_ring->cfm_md_name, 0, GLB_CFM_MD_MA_NAME_LENGTH_8021AG); 
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_CFM_MD_NAME);

    p_db_g8032_ring->level = 0;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_LEVEL);

    sal_memset(p_db_g8032_ring->cfm_ma_name, 0, GLB_CFM_MD_MA_NAME_LENGTH_8021AG); 
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_CFM_MA_NAME);

    p_db_g8032_ring->vlan_id = 0;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_VLAN_ID);

    p_db_g8032_ring->east_interface_cfm_state = 0;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_INTERFACE_CFM_STATE);

    p_db_g8032_ring->west_interface_cfm_state = 0;
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_INTERFACE_CFM_STATE);

    return PM_E_NONE;
}

int32 g8032_cfm_association(tbl_g8032_ring_t *p_db_g8032_ring, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                   rc = PM_E_NONE;
    tbl_cfm_md_t       md;
    tbl_cfm_md_t       *p_md = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    tbl_cfm_ma_t       *p_ma = NULL;
    tbl_cfm_ma_master_t   *p_db_glb_ma = NULL;
    tbl_cfm_lmep_key_t           lmep_key;
    tbl_cfm_lmep_t                  *p_east_lmep = NULL;
    tbl_cfm_lmep_t                  *p_west_lmep = NULL;
    bool                     ma_exist = FALSE;

    if (5 == argc)
    {     
        /* modified by liwh for bug 53859, 2019-10-08 */
        if (p_db_g8032_ring->protection_en)
        {
            CFG_CONFLICT_RET("Can't change MD/MA in running state");
        }
        /* liwh end */
                
        /* check ring is associated by  cfm */
        if (0 != sal_strlen(p_db_g8032_ring->cfm_md_name))
        {
            CFG_CONFLICT_RET("Ring already associated MD/MA");    
        }
        
        /* check md exist */
        sal_memcpy(md.key.name, argv[2], sal_strlen(argv[2]));
        p_md = tbl_cfm_md_get_cfm_md(&md.key);
        if (!p_md)
        {   
            CFG_CONFLICT_RET("MD Not found");
        }

        /* check ma exist */
        p_db_glb_ma = tbl_cfm_ma_get_master();
        if (!p_db_glb_ma)
        {
            CFG_CONFLICT_RET("MA Not found");
        }

        CTCLIB_SLIST_LOOP(p_db_glb_ma->cfm_ma_list, p_ma, listnode)
        {
            if ((0 == sal_strcmp((char*)p_ma->key.name, (char*)p_md->key.name))
                && (0 == sal_strcmp(p_ma->name, argv[4])))
            {
                ma_exist = TRUE;
                break;    
            }
        }

        if (!ma_exist)
        {   
            CFG_CONFLICT_RET("MA Not found");
        }
        
        /* check east mep exist */
        lmep_key.level = p_md->level;
        lmep_key.vlan_id = p_ma->key.vlan_id;
        lmep_key.ifindex = p_db_g8032_ring->east_port_ifindex;    
        p_east_lmep = tbl_cfm_lmep_get_cfm_lmep(&lmep_key);
        if (!p_east_lmep)
        {   
            CFG_FUNC_ERR_RET("Local mep is not found for east interface"); 
        }

        if (GLB_CFM_UP_MEP == p_east_lmep->direction)
        {
            CFG_FUNC_ERR_RET("Local mep is up mep for east interface, can't associate with g8032"); 
        }

        if (0 == p_east_lmep->rmep_num)
        {
            CFG_FUNC_ERR_RET("Remote mep is not found for east interface");    
        }

        if (p_east_lmep->rmep_num > 1)
        {
            CFG_FUNC_ERR_RET("Local mep and remote mep are not point to point for east interface");    
        }

        /* check west mep is exist */
        if (0 != p_db_g8032_ring->west_port_ifindex) 
        {
            lmep_key.ifindex = p_db_g8032_ring->west_port_ifindex;    
            p_west_lmep = tbl_cfm_lmep_get_cfm_lmep(&lmep_key);
            if (!p_west_lmep)
            {   
                CFG_FUNC_ERR_RET("Local mep not found for west interface"); 
            }

            if (GLB_CFM_UP_MEP == p_west_lmep->direction)
            {
                CFG_FUNC_ERR_RET("Local mep is up mep for west interface, can't associate with g8032"); 
            }
    
            if (0 == p_west_lmep->rmep_num)
            {
                CFG_FUNC_ERR_RET("Remote mep is not found for west interface");    
            }
    
            if (p_west_lmep->rmep_num > 1)
            {
                CFG_FUNC_ERR_RET("Local mep and remote mep are not point to point for west interface");    
            }
        }

        /* reserve cfm md/ma for g8032 ring */    
        sal_strcpy(p_db_g8032_ring->cfm_md_name, argv[2]);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_CFM_MD_NAME);

        p_db_g8032_ring->level = p_md->level;
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_LEVEL);

        sal_strcpy(p_db_g8032_ring->cfm_ma_name, argv[4]);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_CFM_MA_NAME);

        p_db_g8032_ring->vlan_id = p_ma->key.vlan_id;
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_VLAN_ID);
  
        /* reserve ring id for east mep */
        p_east_lmep->ring_id = p_db_g8032_ring->key.ring_id;
        tbl_cfm_lmep_set_cfm_lmep_field(p_east_lmep, TBL_CFM_LMEP_FLD_RING_ID);

        if (0 != p_db_g8032_ring->west_port_ifindex) 
        {
            /* reserve ring id for west mep */
            p_west_lmep->ring_id = p_db_g8032_ring->key.ring_id;
            tbl_cfm_lmep_set_cfm_lmep_field(p_west_lmep, TBL_CFM_LMEP_FLD_RING_ID);
        }
        
        /* reserve east cfm state */
        p_db_g8032_ring->east_interface_cfm_state = cfm_api_get_cfm_state_for_g8032(p_east_lmep);
        tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_INTERFACE_CFM_STATE);

        /* reserve west cfm state */
        if (0 != p_db_g8032_ring->west_port_ifindex) 
        {
            p_db_g8032_ring->west_interface_cfm_state = cfm_api_get_cfm_state_for_g8032(p_west_lmep);
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_INTERFACE_CFM_STATE);
        }
    }
    /* clear cfm md/ma for g8032 ring */
    else
    {
        /* check ring is associated by  cfm */
        if (0 == sal_strlen(p_db_g8032_ring->cfm_md_name))
        {
            CFG_CONFLICT_RET("MD/MA not associated");    
        }

        if (p_db_g8032_ring->protection_en)
        {
            CFG_CONFLICT_RET("Can't change MD/MA in running state");
        }

        g8032_delete_cfm_association(p_db_g8032_ring);
    }

    return rc ;
}

int32 g8032_delete_association_by_cfm(uint32  ring_id)
{
    tbl_g8032_ring_t    *p_db_g8032_ring = NULL;
    tbl_g8032_ring_t    g8032_ring;
    
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (p_db_g8032_ring)
    {
        /* set g8032 ring disable */
        g8032_set_ring_disable(p_db_g8032_ring);

        /* delete g8032 cfm association */
        g8032_delete_cfm_association(p_db_g8032_ring);
    }

    return PM_E_NONE;
}
/* liwh end */

static int32
g8032_ring_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    //int32 field_id = 0;
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    //cdb_field_t *p_field = NULL;
    p_node = cdb_get_tbl(TBL_G8032_RING);
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    tbl_g8032_ring_t g8032_ring;
    tbl_interface_t *p_east_interface = NULL;
    //tbl_interface_t *p_west_interface = NULL;
    int32 operation_field = 0;
    uint8 ring_enable = 0;
    uint8 virtual_channel = 0;
    uint8 revertive_mode = 0;
    uint32 instance_id = 0;
    uint32 ring_id = 0;
    uint32 control_vlan_id = 0;
    uint32 interval = 0;
    uint32 level_id = 0;

    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    CFG_DBG_FUNC();
    switch (para->oper)
    {
        case CDB_OPER_ADD:
            ring_id = sal_atoi(argv[0]);
            if (1 == argc)
            {
                g8032_ring.key.ring_id = ring_id;
                p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
                if (NULL == p_db_g8032_ring)
                {
                    CFG_CONFLICT_RET("Ring %d does not exist", ring_id);
                }
                else
                {
                    return rc; 
                }
            }
            else if(5 == argc || (6 == argc && !sal_strcmp(argv[5],"is_sub_ring")))
            {
                rc = g8032_ring_add_ring(argv, argc, para); 
                if (rc)
                {
                    return rc;
                }
            }
            else if (6 == argc && !sal_strcmp(argv[0],"sub-ring-id"))
            {
                g8032_ring.key.ring_id = ring_id;
                p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
                if (p_db_g8032_ring)
                {
                p_east_interface = tbl_interface_get_interface_by_ifindex(p_db_g8032_ring->east_port_ifindex);
                if (sal_strcmp((p_east_interface->key.name),argv[3]))
                {
                    CFG_INVALID_PARAM_RET("The east-interface and west-interface can not be modified");
                }
                else
                {
                    return rc;
                }
                }
                else
                {
                    rc = g8032_ring_add_sub_ring(argv, argc, para);
                    if (rc)
                    {
                        return rc;
                    }
                } 
            } 
            break;

        case CDB_OPER_DEL:
            g8032_ring.key.ring_id = sal_atoi(argv[0]);
            p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
            if (NULL == p_db_g8032_ring)
            {
                CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
            }
            
            operation_field = g8032_ring_get_cmd_operation_field(argv, argc, para);
            switch(operation_field)
            {
                case TBL_G8032_RING_FLD_KEY:
                    rc = g8032_delete_ring(g8032_ring.key.ring_id, para);
                    break;

                case TBL_G8032_RING_FLD_INSTANCE:
                    instance_id = sal_atoi(argv[2]);
                    rc = g8032_delete_instance(g8032_ring.key.ring_id, instance_id, para);
                    break;

                case TBL_G8032_RING_FLD_RPL_OWNER:
                    rc = g8032_delete_rpl_owner(g8032_ring.key.ring_id, para);
                    break;

                case TBL_G8032_RING_FLD_RPL_NEIGHBOR:
                    rc = g8032_delete_rpl_neighbor(g8032_ring.key.ring_id, para);
                    break;

                case TBL_G8032_RING_FLD_CONTROL_VLAN:
                    rc = g8032_delete_control_vlan(g8032_ring.key.ring_id, para);
                    break;

                default:
                    CFG_INVALID_PARAM_RET("Invalid action");
                    break;
            }
            break;

        case CDB_OPER_SET:
            g8032_ring.key.ring_id = sal_atoi(argv[0]);
            p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
            if (NULL == p_db_g8032_ring)
            {
                CFG_CONFLICT_RET("Ring %d does not exist", g8032_ring.key.ring_id);
            }

            if (2 == argc && !sal_strcmp(argv[1],"clear_port"))
            {
                g8032_set_local_clear(g8032_ring.key.ring_id, para); 
            }
            else
            {
                operation_field = g8032_ring_get_cmd_operation_field(argv, argc, para);
                switch(operation_field)
                {
                    case TBL_G8032_RING_FLD_VIRTUAL_CHANNEL:
                        virtual_channel = sal_atoi(argv[2]);
                        rc = g8032_set_virtual_channel(g8032_ring.key.ring_id, virtual_channel, para);
                        break;

                    case TBL_G8032_RING_FLD_MODE_REVERTIVE:
                        revertive_mode = sal_atoi(argv[2]);
                        rc = g8032_set_revertive_mode(g8032_ring.key.ring_id, revertive_mode, para);
                        break;

                    case TBL_G8032_RING_FLD_EAST_PORT_IS_RPL:
                        rc = g8032_set_rpl_owner(g8032_ring.key.ring_id, TRUE, para);
                        break; 

                    case TBL_G8032_RING_FLD_WEST_PORT_IS_RPL:
                        rc = g8032_set_rpl_owner(g8032_ring.key.ring_id, FALSE, para);
                        break;

                    case TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEXT_NEIGHBOR:
                        rc = g8032_set_rpl_next_neighbor(g8032_ring.key.ring_id, TRUE, para);
                        break;

                    case TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEXT_NEIGHBOR:
                        rc = g8032_set_rpl_next_neighbor(g8032_ring.key.ring_id, FALSE, para);
                        break;

                    case TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEIGHBOR:
                        rc = g8032_set_rpl_neighbor(g8032_ring.key.ring_id, TRUE, para);
                        break;

                    case TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEIGHBOR:
                        rc = g8032_set_rpl_neighbor(g8032_ring.key.ring_id, FALSE, para);
                        break;

                    case TBL_G8032_RING_FLD_INSTANCE:
                        instance_id = sal_atoi(argv[2]);
                        rc = g8032_add_instance(g8032_ring.key.ring_id, instance_id, para);
                        break;

                    case TBL_G8032_RING_FLD_CONTROL_VLAN:
                        control_vlan_id = sal_atoi(argv[2]);
                        rc = g8032_set_control_vlan(g8032_ring.key.ring_id, control_vlan_id, para);
                        break;

                    case TBL_G8032_RING_FLD_WTR_TIMEOUT_VAL:
                        interval = sal_atoi(argv[2]);
                        rc = g8032_set_wtr_timer_interval(g8032_ring.key.ring_id, interval, para);
                        break;

                    case TBL_G8032_RING_FLD_HOLD_OFF_TIMEOUT_VAL:
                        interval = sal_atoi(argv[2]);
                        if(interval%100)
                        {
                            CFG_CONFLICT_RET("Hold-off timer interval must be the multiples of 100"); 
                        }
                        rc = g8032_set_hold_off_timer_interval(g8032_ring.key.ring_id, interval, para);
                        break;

                    case TBL_G8032_RING_FLD_GUARD_TIMEOUT_VAL:
                        interval = sal_atoi(argv[2]);
                        rc = g8032_set_guard_timer_interval(g8032_ring.key.ring_id, interval, para);
                        break;

                    case TBL_G8032_RING_FLD_PROTECTION_EN:
                        ring_enable = sal_atoi(argv[2]);
                        rc = g8032_set_ring_enable(g8032_ring.key.ring_id, ring_enable, para);
                        break;

                    case TBL_G8032_RING_FLD_RING_LEVEL_ID:
                        level_id = sal_atoi(argv[2]);
                        rc = g8032_set_ring_level_id(g8032_ring.key.ring_id, level_id, para);
                        break;
                        
                    case TBL_G8032_RING_FLD_EAST_PORT_IS_FORCE_SWITCH:
                        rc = g8032_set_fs_port(g8032_ring.key.ring_id, TRUE, para);
                        break;
                    
                    case TBL_G8032_RING_FLD_WEST_PORT_IS_FORCE_SWITCH:
                        rc = g8032_set_fs_port(g8032_ring.key.ring_id, FALSE, para);
                        break;
                    
                    case TBL_G8032_RING_FLD_EAST_PORT_IS_MANUAL_SWITCH:
                        rc = g8032_set_ms_port(g8032_ring.key.ring_id, TRUE, para);
                        break;
                    
                    case TBL_G8032_RING_FLD_WEST_PORT_IS_MANUAL_SWITCH:
                        rc = g8032_set_ms_port(g8032_ring.key.ring_id, FALSE, para);
                        break;

                    case TBL_G8032_RING_FLD_CFM_MD_NAME:
                        rc = g8032_cfm_association(p_db_g8032_ring, argv, argc, para);
                        break;
                    
                    default:
                        rc = CFG_INVALID_PARAM_RET("Invalid action");
                        break;
            }
        }
        break;

        case CDB_OPER_GET:
            if (0 == argc)
            {
                ring_id = 0;
            }
            else if (1 == argc)
            {
                ring_id = cdb_uint_str2val(argv[0], &rc);
                if (rc || (ring_id > G8032_RING_ID_MAXIMUM) || (ring_id <= 0))
                {
                    CFG_INVALID_PARAM_RET("Invalid ring ID");
                }
            }
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
            rc = g8032_ring_cmd_get_ring(ring_id, para, p_node, &field);
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }
    
    return rc;
}

void 
g8032_ring_hold_off_expired_fn(void* p_info)
{
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;

    if (!p_info)
    {
        return ;
    }

    p_db_g8032_ring = (tbl_g8032_ring_t *)p_info;
    
    CTC_TASK_STOP_TIMER(p_db_g8032_ring->p_hold_off_while);
    tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_P_HOLD_OFF_WHILE);
    G8032_TIMER_DEBUG("G8032 ring %d stop hold-off timer", p_db_g8032_ring->key.ring_id);

    if (p_db_g8032_ring->east_port_is_fail)
    {
        G8032FSM[p_db_g8032_ring->current_node_state][G8032_PRI_REQ_L_SF].func(p_db_g8032_ring, NULL, G8032_PRI_REQ_L_SF, TRUE);
    }

    if (p_db_g8032_ring->west_port_is_fail)
    {
        G8032FSM[p_db_g8032_ring->current_node_state][G8032_PRI_REQ_L_SF].func(p_db_g8032_ring, NULL, G8032_PRI_REQ_L_SF, FALSE);
    }


}

int32
g8032_ring_switch_trigged_by_cfm(uint32  ring_id, uint32  ifindex, bool isUp)
{
    tbl_g8032_ring_t    *p_db_g8032_ring = NULL;
    tbl_g8032_ring_t    g8032_ring;
    uint32                    req_state = 0;
    tbl_interface_t  *p_interface = NULL;

    p_interface = tbl_interface_get_interface_by_ifindex(ifindex);
    if (!p_interface)
    {
        G8032_EVENT_DEBUG("%s %d ifindex is invalid\n", __func__, __LINE__, ifindex);
        return PM_E_NOT_EXIST;
    }
    
    if (isUp)
    {
        req_state = G8032_PRI_REQ_L_CLEAR_SF;    
    }
    else
    {
        req_state = G8032_PRI_REQ_L_SF;    
    }
    
    g8032_ring.key.ring_id = ring_id;
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
    if (p_db_g8032_ring)
    {
        if (p_interface->ifindex == p_db_g8032_ring->east_port_ifindex)
        {
            /* system modified by liwh for bug 53010, 2019-08-08 */
            if (isUp)
            {
                p_db_g8032_ring->east_interface_cfm_state = GLB_CFM_RMEP_UP;
                tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_INTERFACE_CFM_STATE);
            }
            else
            {
                p_db_g8032_ring->east_interface_cfm_state = GLB_CFM_RMEP_DOWN;
                tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_INTERFACE_CFM_STATE);    
            }
            /* liwh end */
            
            if (req_state == G8032_PRI_REQ_L_SF)
            {
                p_db_g8032_ring->east_port_is_fail = TRUE;
            }

            if (req_state == G8032_PRI_REQ_L_CLEAR_SF)
            {
                p_db_g8032_ring->east_port_is_fail = FALSE;
            }

            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_FAIL);
            if (p_db_g8032_ring->protection_en)
            {
                if (req_state == G8032_PRI_REQ_L_SF)
                {
                    if (p_db_g8032_ring->hold_off_timeout_val)
                    {
                        p_db_g8032_ring->p_hold_off_while=  CTC_TASK_ADD_TIME_MSEC((g8032_ring_hold_off_expired_fn), p_db_g8032_ring, p_db_g8032_ring->hold_off_timeout_val);
                    }
                    else
                    {
                        G8032FSM[p_db_g8032_ring->current_node_state][req_state].func(p_db_g8032_ring, NULL, req_state, TRUE);
                    }
                }

                if (req_state == G8032_PRI_REQ_L_CLEAR_SF)
                {
                    G8032FSM[p_db_g8032_ring->current_node_state][req_state].func(p_db_g8032_ring, NULL, req_state, TRUE);
                }
            }
        }

        if (p_interface->ifindex == p_db_g8032_ring->west_port_ifindex)
        {
            /* system modified by liwh for bug 53010, 2019-08-08 */
            if (isUp)
            {
                p_db_g8032_ring->west_interface_cfm_state = GLB_CFM_RMEP_UP;
                tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_INTERFACE_CFM_STATE);
            }
            else
            {
                p_db_g8032_ring->west_interface_cfm_state = GLB_CFM_RMEP_DOWN;
                tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_INTERFACE_CFM_STATE);    
            }
            /* liwh end */
            
            if (req_state == G8032_PRI_REQ_L_SF)
            {
                p_db_g8032_ring->west_port_is_fail = TRUE;
            }
            if (req_state == G8032_PRI_REQ_L_CLEAR_SF)
            {
                p_db_g8032_ring->west_port_is_fail = FALSE;
            }
            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_FAIL);
             if (p_db_g8032_ring->protection_en)
            {
                if (req_state == G8032_PRI_REQ_L_SF)
                {
                    if (p_db_g8032_ring->hold_off_timeout_val)
                    {
                        p_db_g8032_ring->p_hold_off_while=  CTC_TASK_ADD_TIME_MSEC((g8032_ring_hold_off_expired_fn), p_db_g8032_ring, p_db_g8032_ring->hold_off_timeout_val);
                        G8032_TIMER_DEBUG("G8032 ring %d start hold-off timer", p_db_g8032_ring->key.ring_id);
                    }
                    else
                    {
                        G8032FSM[p_db_g8032_ring->current_node_state][req_state].func(p_db_g8032_ring, NULL, req_state, FALSE);
                    }
                }

                if (req_state == G8032_PRI_REQ_L_CLEAR_SF)
                {
                    G8032FSM[p_db_g8032_ring->current_node_state][req_state].func(p_db_g8032_ring, NULL, req_state, FALSE);
                }
            } 
        }
    }

    return PM_E_NONE;
}
              
int32
g8032_ring_process_link_state_sync(cdb_node_t* p_tbl_node, 
cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;
    tbl_interface_t  *p_interface = NULL;
    tbl_interface_t  *p_db_interface = NULL;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    tbl_g8032_ring_t g8032_ring;
    uint32 i = 0;
    uint32 req_state = 0;
    if (!p_tbl)
    {
        return rc;
    }
    sal_memset(&g8032_ring, 0, sizeof(tbl_g8032_ring_t));
    p_interface = (tbl_interface_t  *)p_tbl;
    p_db_interface = tbl_interface_get_interface_by_name(p_interface->key.name);
    switch (oper)
    {
        case CDB_OPER_ADD:
            break;
            
        case CDB_OPER_DEL:
            break;
            
        case CDB_OPER_SET: 
            if (TBL_INTERFACE_FLD_UP == field_id ||TBL_INTERFACE_FLD_RUNNING== field_id )
            {
                if (tbl_interface_is_running(p_db_interface))
                {
                    if (!tbl_interface_is_running(p_interface))
                    {
                        req_state = G8032_PRI_REQ_L_SF;
                    } 
                }

                if (!tbl_interface_is_running(p_db_interface))
                {
                    if (tbl_interface_is_running(p_interface))
                    {
                        req_state = G8032_PRI_REQ_L_CLEAR_SF; 
                    } 
                }

                for (i = 0; i<p_interface->current_g8032_ring_count; i++)
                {
                    g8032_ring.key.ring_id = p_interface->g8032_ring_first[i];
                    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring.key);
                    if (p_db_g8032_ring)
                    {
                        if (p_interface->ifindex == p_db_g8032_ring->east_port_ifindex)
                        {
                            /* modified by liwh for bug 53010, 2019-08-07 */
                            /* check cfm state */
                            if (0 != sal_strlen(p_db_g8032_ring->cfm_md_name))
                            {
                                if (GLB_CFM_RMEP_INITIAL != p_db_g8032_ring->east_interface_cfm_state)
                                {
                                    continue;    
                                }
                            }
                            /* liwh end */
                        
                            if (req_state == G8032_PRI_REQ_L_SF)
                            {
                                p_db_g8032_ring->east_port_is_fail = TRUE;
                            }

                            if (req_state == G8032_PRI_REQ_L_CLEAR_SF)
                            {
                                p_db_g8032_ring->east_port_is_fail = FALSE;
                            }

                            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_FAIL);
                            if (p_db_g8032_ring->protection_en)
                            {
                                if (req_state == G8032_PRI_REQ_L_SF )
                                {
                                    if (p_db_g8032_ring->hold_off_timeout_val)
                                    {
                                        G8032_TIMER_DEBUG("G8032 ring %d start hold-off timer", p_db_g8032_ring->key.ring_id);

                                        p_db_g8032_ring->p_hold_off_while=  CTC_TASK_ADD_TIME_MSEC((g8032_ring_hold_off_expired_fn), p_db_g8032_ring, p_db_g8032_ring->hold_off_timeout_val);
                                    }
                                    else
                                    {
                                        G8032FSM[p_db_g8032_ring->current_node_state][req_state].func(p_db_g8032_ring, NULL, req_state, TRUE);
                                    }
                                }

                                if (req_state == G8032_PRI_REQ_L_CLEAR_SF)
                                {
                                    G8032FSM[p_db_g8032_ring->current_node_state][req_state].func(p_db_g8032_ring, NULL, req_state, TRUE);
                                }
                            }
                        }

                        if (p_interface->ifindex == p_db_g8032_ring->west_port_ifindex)
                        {
                            /* modified by liwh for bug 53010, 2019-08-07 */
                            /* check cfm state */
                            if (0 != sal_strlen(p_db_g8032_ring->cfm_md_name))
                            {
                                if (GLB_CFM_RMEP_INITIAL != p_db_g8032_ring->west_interface_cfm_state)
                                {
                                    continue;
                                }
                            }
                            /* liwh end */
                        
                            if (req_state == G8032_PRI_REQ_L_SF)
                            {
                                p_db_g8032_ring->west_port_is_fail = TRUE;
                            }
                            if (req_state == G8032_PRI_REQ_L_CLEAR_SF)
                            {
                                p_db_g8032_ring->west_port_is_fail = FALSE;
                            }
                            tbl_g8032_ring_set_g8032_ring_field(p_db_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_FAIL);
                             if (p_db_g8032_ring->protection_en)
                            {
                                if (req_state == G8032_PRI_REQ_L_SF)
                                {
                                    if (p_db_g8032_ring->hold_off_timeout_val)
                                    {
                                        p_db_g8032_ring->p_hold_off_while=  CTC_TASK_ADD_TIME_MSEC((g8032_ring_hold_off_expired_fn), p_db_g8032_ring, p_db_g8032_ring->hold_off_timeout_val);
                                        G8032_TIMER_DEBUG("G8032 ring %d start hold-off timer", p_db_g8032_ring->key.ring_id);
                                    }
                                    else
                                    {
                                        G8032FSM[p_db_g8032_ring->current_node_state][req_state].func(p_db_g8032_ring, NULL, req_state, FALSE);
                                    }
                                }

                                if (req_state == G8032_PRI_REQ_L_CLEAR_SF)
                                {
                                    G8032FSM[p_db_g8032_ring->current_node_state][req_state].func(p_db_g8032_ring, NULL, req_state, FALSE);
                                }
                            } 
                        }
                    }
                }
            }
            break;

        default:
            break;
    }

    return rc;
}

uint32
g8032_ring_get_node_role(tbl_g8032_ring_t *p_g8032_ring)
{
    if (!p_g8032_ring)
    {
        return G8032_NODE_ROLE_NA;
    }

    if (p_g8032_ring->rpl_owner)
    {
        return G8032_NODE_ROLE_RPL_OWNER;
    }

    if (p_g8032_ring->rpl_neighbor)
    {
        return G8032_NODE_ROLE_RPL_NEIGHBOR;
    }

    if (p_g8032_ring->rpl_next_neighbor)
    {
        return G8032_NODE_ROLE_RPL_NEXT_NEIGHBOR;
    }
    
    return  G8032_NODE_ROLE_NA;
}

static int32
g8032_cmd_show_g8032_interface(tbl_interface_t *p_interface, FILE *fp, cfg_cmd_para_t *para)
{
    uint32 i = 0;
    tbl_g8032_ring_t *p_g8032_ring = NULL;

    if (!p_interface)
    {
        CFG_INVALID_PARAM_RET("Interface does not exist");
    }

    for (i = 0; i < p_interface->current_g8032_ring_count; i++)
    {        
        p_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t *)(&p_interface->g8032_ring_first[i]));
        if (!p_g8032_ring)
        {
            CFG_INVALID_PARAM_RET("G8032 ring %d that bound to the interface does not exist", p_interface->g8032_ring_first[i]);
        }
    }
    sal_fprintf(fp, "%-25s : %s\n", "Interface State", tbl_interface_is_running(p_interface) ? "Up" : "Down");
    sal_fprintf(fp, "==================================================\n");
    for (i = 0; i < p_interface->current_g8032_ring_count; i++)
    {        
        p_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t *)(&p_interface->g8032_ring_first[i]));
        if (!p_g8032_ring)
        {
            continue;
        }

        if (p_interface->ifindex == p_g8032_ring->east_port_ifindex)
        {
            sal_fprintf(fp, "%-25s : %d\n", "Ring ID", p_interface->g8032_ring_first[i]);
            sal_fprintf(fp, "%-25s\n", "Flush Logic");
            sal_fprintf(fp, "    %-21s : %02x%02x.%02x%02x.%02x%02x\n", "Remote Node ID",
                p_g8032_ring->east_port_rec_node_id[0],p_g8032_ring->east_port_rec_node_id[1],p_g8032_ring->east_port_rec_node_id[2],
                p_g8032_ring->east_port_rec_node_id[3],p_g8032_ring->east_port_rec_node_id[4],p_g8032_ring->east_port_rec_node_id[5]);
            sal_fprintf(fp, "    %-21s : %d\n", "Remote BPR",p_g8032_ring->east_port_rec_bpr);
        }

        if (p_interface->ifindex == p_g8032_ring->west_port_ifindex)
        {
            sal_fprintf(fp, "%-25s : %d\n", "Ring ID", p_interface->g8032_ring_first[i]);
            sal_fprintf(fp, "%-25s\n", "Flush Logic");
            sal_fprintf(fp, "    %-21s : %02x%02x.%02x%02x.%02x%02x\n", "Remote Node ID",
                p_g8032_ring->west_port_rec_node_id[0],p_g8032_ring->west_port_rec_node_id[1],p_g8032_ring->west_port_rec_node_id[2],
                p_g8032_ring->west_port_rec_node_id[3],p_g8032_ring->west_port_rec_node_id[4],p_g8032_ring->west_port_rec_node_id[5]);
            sal_fprintf(fp, "    %-21s : %d\n", "Remote BPR",p_g8032_ring->east_port_rec_bpr);
        }

        sal_fprintf(fp, "==================================================\n");
        
    }
    return PM_E_NONE;
}

static int32
g8032_cmd_show_one_ring(tbl_g8032_ring_t *p_g8032_ring, FILE *fp )
{
    tbl_interface_t *p_east_interface = NULL;
    tbl_interface_t *p_west_interface = NULL;
    uint32 role = 0;
    uint32 i = 0;
    char ifname_ext_east[IFNAME_EXT_SIZE];
    char ifname_ext_west[IFNAME_EXT_SIZE];
   

    if (!p_g8032_ring)
    {
        return PM_E_NONE;
    }
    role = g8032_ring_get_node_role(p_g8032_ring);
    sal_fprintf(fp, "%-7s %-10s %-15s %-11s %-10s %-11s %-10s\n", "RingID",  "MajorRing", "State",  "East", "Status", "West", "Status");
    sal_fprintf(fp, "================================================================================\n");
    if (p_g8032_ring->west_port_ifindex)
    {
        p_east_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
        p_west_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
        IFNAME_ETH2FULL(p_east_interface->key.name, ifname_ext_east);
        IFNAME_ETH2FULL(p_west_interface->key.name, ifname_ext_west);
        if (p_g8032_ring->g8032_major_ring_id)
        {
            sal_fprintf(fp, "%-7d %-10d %-15s %-11s %-10s %-11s %-10s \n", p_g8032_ring->key.ring_id, 
            p_g8032_ring->g8032_major_ring_id, 
            (p_g8032_ring->protection_en) ? G8032_FSM_STATE_STR(p_g8032_ring->current_node_state) :"N/A" ,
            ifname_ext_east, 
            (p_g8032_ring->east_port_is_blocked) ? "Blocked" : "Unblocked",
             ifname_ext_west,
            (p_g8032_ring->west_port_is_blocked) ? "Blocked" : "Unblocked");
        }
        else
        {
            sal_fprintf(fp, "%-7d %-10s %-15s %-11s %-10s %-11s %-10s\n", p_g8032_ring->key.ring_id, 
            "N/A", (p_g8032_ring->protection_en) ? G8032_FSM_STATE_STR(p_g8032_ring->current_node_state) :"N/A" ,
            ifname_ext_east, 
            (p_g8032_ring->east_port_is_blocked) ? "Blocked" : "Unblocked",
            ifname_ext_west,
            (p_g8032_ring->west_port_is_blocked) ? "Blocked" : "Unblocked");
        }
    }
    else
    {
        p_east_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
        IFNAME_ETH2FULL(p_east_interface->key.name, ifname_ext_east);
        sal_fprintf(fp, "%-7d %-10d %-15s %-11s %-10s %-11s %-10s \n", p_g8032_ring->key.ring_id, 
        p_g8032_ring->g8032_major_ring_id, 
        (p_g8032_ring->protection_en) ? G8032_FSM_STATE_STR(p_g8032_ring->current_node_state) :"N/A" ,
        ifname_ext_east, 
        (p_g8032_ring->east_port_is_blocked) ? "Blocked" : "Unblocked", 
        "N/A", "N/A");
    }

    sal_fprintf(fp,"\n");

    if (p_g8032_ring->control_vlan)
    {
        sal_fprintf(fp,"%-25s: %d\n", "Control Vlan", p_g8032_ring->control_vlan);
    }
    else
    {
        sal_fprintf(fp,"%-25s: %s\n", "Control Vlan", "N/A");
    }

    /* modified by liwh for bug 53010, 2019-08-07 */
    if (0 != sal_strlen(p_g8032_ring->cfm_md_name))
    {
        sal_fprintf(fp, "%-25s: %s\n", "MD Name", p_g8032_ring->cfm_md_name);
        sal_fprintf(fp, "%-25s: %s\n", "Service Id", p_g8032_ring->cfm_ma_name);
    }
    /* liwh end */

    if (p_g8032_ring->protection_en)
    {
        sal_fprintf(fp,"%-25s: %s\n", "Is Enable", "Yes");
    }
    else
    {
        sal_fprintf(fp,"%-25s: %s\n", "Is Enable", "No");
    }

    sal_fprintf(fp,"%-25s: %s\n", "Node Role", G8032_FSM_ROLE_STR(role) );

    if (p_g8032_ring->is_sub_ring)
    {
        sal_fprintf(fp,"%-25s: %s\n", "Is Sub_ring","Yes");
    }
    else
    {
        sal_fprintf(fp,"%-25s: %s\n","Is Sub_ring",  "No");
    }

    sal_fprintf(fp,"%-25s: %d\n", "Protect Instance Count", p_g8032_ring->instance_count);

    if (p_g8032_ring->instance_count)
    {
        sal_fprintf(fp,"%-25s:", "Protect Instance");
        for (i = 0; i<p_g8032_ring->instance_count; i++)
        {
        sal_fprintf(fp," %d  ", p_g8032_ring->instance[i]);
        }
        sal_fprintf(fp,"\n");
    }
    else
    {
        sal_fprintf(fp,"%-25s: %s\n", "Protect Instance", "N/A");
    }

    if (p_g8032_ring->is_local_fs)
    {
        sal_fprintf(fp,"%-25s: %s\n", "Force Switch Interface", p_g8032_ring->east_port_is_force_switch ? "east-interface" :(p_g8032_ring->west_port_is_force_switch ? "west-interface" : "N/A"));
    }
    else
    {
        sal_fprintf(fp,"%-25s: %s\n", "Force Switch Interface", "N/A");
    }

    if (p_g8032_ring->is_local_ms)
    {
        sal_fprintf(fp,"%-25s: %s\n", "Manual Switch Interface", p_g8032_ring->east_port_is_manual_switch ? "east-interface" :(p_g8032_ring->west_port_is_manual_switch ? "west-interface" : "N/A"));
    }
    else
    {
        sal_fprintf(fp,"%-25s: %s\n", "Manual Switch Interface", "N/A");
    }

    sal_fprintf(fp, "%-25s: %d(msec)\n", "Guard Time", p_g8032_ring->guard_timeout_val);
    sal_fprintf(fp, "%-25s: %d(msec)\n", "Hold-off Time", p_g8032_ring->hold_off_timeout_val);
    sal_fprintf(fp, "%-25s: %d(min)\n", "Wait-to-restore Time ", p_g8032_ring->wtr_timeout_val);
    sal_fprintf(fp, "%-25s: %d(msec)\n", "Wait-to-block Time ", p_g8032_ring->wtb_timeout_val);
    sal_fprintf(fp, "%-25s: %s\n", "Revertive mode", p_g8032_ring->mode_revertive ? "Yes" : "No");
    sal_fprintf(fp, "%-25s: %s\n", "Virtual-channel", p_g8032_ring->virtual_channel? "Yes" : "No");
    sal_fprintf(fp, "%-25s: %d\n", "Ring level", p_g8032_ring->ring_level_id);
    sal_fprintf(fp, "================================================================================\n");
    sal_fprintf(fp, "\n");
    return PM_E_NONE;

}

static int32
g8032_cmd_show_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    tbl_interface_t *p_interface = NULL;
    uint32 ring_id = 0;
    FILE *fp = NULL;
    tbl_g8032_ring_master_t *p_g8032_master = NULL;
    ctclib_slistnode_t *listnode = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];


    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (argc > 2)
    {
        return PM_E_NONE;
    }

    if (1 == argc)
    {
        ring_id = sal_atoi(argv[0]);
        p_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t * )&ring_id );

        if (!p_g8032_ring)
        {
            CFG_INVALID_PARAM_RET("G8032 ring %d does not exist", ring_id);
        }

        g8032_cmd_show_one_ring(p_g8032_ring, fp);
    }

    if (2 == argc && !sal_strcmp(argv[0],"interface"))
    {
        p_interface = tbl_interface_get_interface_by_name(argv[1]);
        if (!p_interface)
        {
            CFG_INVALID_PARAM_RET("Interface does not exist");
        }

        if (!p_interface->current_g8032_ring_count)
        {
            IFNAME_ETH2FULL(p_interface->key.name, ifname_ext);
            CFG_INVALID_PARAM_RET("Interface %s has not been bound to g8032 ring", ifname_ext);
        }

        g8032_cmd_show_g8032_interface(p_interface, fp, para);
        
    }
    
    if (!argc)
    {
        p_g8032_master = tbl_g8032_ring_get_master();
        CTCLIB_SLIST_LOOP(p_g8032_master->g8032_ring_list, p_g8032_ring, listnode)
        {
            g8032_cmd_show_one_ring(p_g8032_ring, fp);
        }
    }

    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;

}

int32
g8032_start(void)
{
    cdb_register_cfg_tbl_cb(TBL_G8032_RING, g8032_ring_cmd_process);
    cdb_pm_subscribe_tbl(TBL_INTERFACE, g8032_ring_process_link_state_sync,NULL);
    cdb_register_cfg_act_cb(ACT_SHOW_G8032, g8032_cmd_show_process);
    ipc_register_pkt_rx_fn(GLB_PKT_RAPS, g8032_ring_pkt_fun);

    return PM_E_NONE;
}
