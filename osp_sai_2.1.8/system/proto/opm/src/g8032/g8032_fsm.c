#include "proto.h"
#include "g8032_define.h"
#include "gen/tbl_g8032_ring_define.h"
#include "gen/tbl_g8032_ring.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "g8032_fsm.h"
#include "g8032_pdu.h"

struct
{
   int32 (* func)();
   int32 next_state;
}G8032FSM[G8032_FSM_STATE_MAX][G8032_FSM_EVENT_MAX] =
{

    /* G8032_NODE_STATE_IDLE - 0 */
    {
        { g8032_fsm_raps_nr,      G8032_NODE_STATE_IDLE                                   },
        { g8032_fsm_raps_nr_rb,   G8032_NODE_STATE_IDLE                               },
        { g8032_fsm_ignore,       G8032_NODE_STATE_IDLE                                    },
        { g8032_fsm_ignore,       G8032_NODE_STATE_IDLE                                    },
        { g8032_fsm_raps_sf,      G8032_NODE_STATE_PROTECTION                  },
        { g8032_fsm_ignore,       G8032_NODE_STATE_IDLE                                   },
        { g8032_fsm_local_sf,     G8032_NODE_STATE_PROTECTION                  },
        { g8032_fsm_ignore,       G8032_NODE_STATE_IDLE                                   },
        { g8032_fsm_local_fs,     G8032_NODE_STATE_FORCE_SWITCH            },
        { g8032_fsm_raps_fs,      G8032_NODE_STATE_FORCE_SWITCH            },
        { g8032_fsm_local_ms,     G8032_NODE_STATE_MANUAL_SWITCH     },
        { g8032_fsm_raps_ms,      G8032_NODE_STATE_MANUAL_SWITCH     },
        { g8032_fsm_ignore,       G8032_NODE_STATE_IDLE                                   },
        { g8032_fsm_ignore,       G8032_NODE_STATE_IDLE                                   }
        
    },

    /* G8032_NODE_STATE_PROTECTION - 1 */
    {
        { g8032_fsm_raps_nr,         G8032_NODE_STATE_PENDING      },
        { g8032_fsm_raps_nr_rb,          G8032_NODE_STATE_PENDING      },
        { g8032_fsm_ignore,          G8032_NODE_STATE_PROTECTION   },
        { g8032_fsm_ignore,          G8032_NODE_STATE_PROTECTION   },
        { g8032_fsm_ignore,          G8032_NODE_STATE_PROTECTION   },
        { g8032_fsm_local_clear_sf,  G8032_NODE_STATE_PENDING      },
        { g8032_fsm_local_sf,        G8032_NODE_STATE_PROTECTION   },
        { g8032_fsm_ignore,          G8032_NODE_STATE_PROTECTION   },
        { g8032_fsm_local_fs,        G8032_NODE_STATE_FORCE_SWITCH },
        { g8032_fsm_raps_fs,         G8032_NODE_STATE_FORCE_SWITCH },
        { g8032_fsm_ignore,          G8032_NODE_STATE_PROTECTION   },
        { g8032_fsm_ignore,          G8032_NODE_STATE_PROTECTION   },
        { g8032_fsm_ignore,          G8032_NODE_STATE_PROTECTION   },
        { g8032_fsm_ignore,          G8032_NODE_STATE_PROTECTION   }
        
    },

    /* G8032_NODE_STATE_MANUAL_SWITCH - 2 */
    {
        { g8032_fsm_raps_nr,       G8032_NODE_STATE_PENDING         },
        { g8032_fsm_raps_nr_rb,   G8032_NODE_STATE_PENDING         },
        { g8032_fsm_ignore,       G8032_NODE_STATE_MANUAL_SWITCH   },
        { g8032_fsm_ignore,       G8032_NODE_STATE_MANUAL_SWITCH   },
        { g8032_fsm_raps_sf,      G8032_NODE_STATE_PROTECTION      },
        { g8032_fsm_ignore,       G8032_NODE_STATE_MANUAL_SWITCH   },
        { g8032_fsm_local_sf,     G8032_NODE_STATE_PROTECTION      },
        { g8032_fsm_local_clear,  G8032_NODE_STATE_PENDING         },
        { g8032_fsm_local_fs,     G8032_NODE_STATE_FORCE_SWITCH    },
        { g8032_fsm_raps_fs,      G8032_NODE_STATE_FORCE_SWITCH    },
        { g8032_fsm_ignore,       G8032_NODE_STATE_MANUAL_SWITCH   },
        { g8032_fsm_raps_ms,      G8032_NODE_STATE_PENDING         },
        { g8032_fsm_ignore,       G8032_NODE_STATE_MANUAL_SWITCH   },
        { g8032_fsm_ignore,       G8032_NODE_STATE_MANUAL_SWITCH   }
        
    },

    /* G8032_NODE_STATE_FORCE_SWITCH - 3 */
    {
        { g8032_fsm_raps_nr,       G8032_NODE_STATE_PENDING        },
        { g8032_fsm_raps_nr_rb,        G8032_NODE_STATE_PENDING        },
        { g8032_fsm_ignore,        G8032_NODE_STATE_FORCE_SWITCH   },
        { g8032_fsm_ignore,        G8032_NODE_STATE_FORCE_SWITCH   },
        { g8032_fsm_ignore,        G8032_NODE_STATE_FORCE_SWITCH   },
        { g8032_fsm_ignore,        G8032_NODE_STATE_FORCE_SWITCH   },
        { g8032_fsm_ignore,        G8032_NODE_STATE_FORCE_SWITCH   },
        { g8032_fsm_local_clear,   G8032_NODE_STATE_PENDING        },
        { g8032_fsm_local_fs,      G8032_NODE_STATE_FORCE_SWITCH   },
        { g8032_fsm_ignore,        G8032_NODE_STATE_FORCE_SWITCH   },
        { g8032_fsm_ignore,        G8032_NODE_STATE_FORCE_SWITCH   },
        { g8032_fsm_ignore,        G8032_NODE_STATE_FORCE_SWITCH   },
        { g8032_fsm_ignore,        G8032_NODE_STATE_FORCE_SWITCH   },
        { g8032_fsm_ignore,        G8032_NODE_STATE_FORCE_SWITCH   }
        
    },

        /* G8032_NODE_STATE_PENDING - 4 */
    {
        { g8032_fsm_raps_nr,       G8032_NODE_STATE_PENDING        },
        { g8032_fsm_raps_nr_rb,    G8032_NODE_STATE_IDLE           },
        { g8032_fsm_ignore,        G8032_NODE_STATE_PENDING        },
        { g8032_fsm_wtr_expires,   G8032_NODE_STATE_IDLE           },
        { g8032_fsm_raps_sf,       G8032_NODE_STATE_PROTECTION     },
        { g8032_fsm_ignore,        G8032_NODE_STATE_PENDING        },
        { g8032_fsm_local_sf,      G8032_NODE_STATE_PROTECTION     },
        { g8032_fsm_local_clear,   G8032_NODE_STATE_IDLE           },
        { g8032_fsm_local_fs,      G8032_NODE_STATE_FORCE_SWITCH   },
        { g8032_fsm_raps_fs,       G8032_NODE_STATE_FORCE_SWITCH   },
        { g8032_fsm_local_ms,      G8032_NODE_STATE_MANUAL_SWITCH  },
        { g8032_fsm_raps_ms,       G8032_NODE_STATE_MANUAL_SWITCH  },
        { g8032_fsm_ignore,        G8032_NODE_STATE_PENDING        },
        { g8032_fsm_wtb_expires,   G8032_NODE_STATE_IDLE           }
    }
        
};

/* system modified by liwh for bug 53010, 2019-08-08 */
uint32
g8032_ring_check_cfm_and_interface_state(tbl_interface_t *p_interface, tbl_g8032_ring_t *p_g8032_ring)
{
    if (!p_interface)
    {
        return FALSE;    
    }

    if (!p_g8032_ring)
    {
        return FALSE;    
    }

    if (p_interface->ifindex == p_g8032_ring->east_port_ifindex)
    {
        if (0 != sal_strlen(p_g8032_ring->cfm_md_name))
        {
            if (GLB_CFM_RMEP_INITIAL != p_g8032_ring->east_interface_cfm_state)
            {
                if  (GLB_CFM_RMEP_UP == p_g8032_ring->east_interface_cfm_state)
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
        }
    }
    else if (p_interface->ifindex == p_g8032_ring->west_port_ifindex)
    {
        if (0 != sal_strlen(p_g8032_ring->cfm_md_name))
        {
            if (GLB_CFM_RMEP_INITIAL != p_g8032_ring->west_interface_cfm_state)
            {
                if  (GLB_CFM_RMEP_UP == p_g8032_ring->west_interface_cfm_state)
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
        }
    }

    return tbl_interface_is_running(p_interface);
}
/* liwh end */

void
_g8032_ring_pkt_tx_raps_fs_fn(void *info)
{
    /*transmit a packet every 5 seconds*/
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    raps_pdu_tx_t  raps_pdu_tx_info ;
    if (NULL == info)
    {
        return ;
    }
    sal_memset(&raps_pdu_tx_info, 0, sizeof(raps_pdu_tx_t));
    p_g8032_ring = ( tbl_g8032_ring_t *)info;
    raps_pdu_tx_info.not_flush_fdb = p_g8032_ring->raps_dnf;
    raps_pdu_tx_info.ring_id = p_g8032_ring->key.ring_id;
    
    CTC_TASK_STOP_TIMER(p_g8032_ring->p_tx_raps_fs_while);
     tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE);
     
    g8032_ring_pkt_tx_raps_fs(&raps_pdu_tx_info);
    
    p_g8032_ring->p_tx_raps_fs_while = CTC_TASK_ADD_TIME((_g8032_ring_pkt_tx_raps_fs_fn), info, G8032_RING_DEF_TX_RAPS_INTERVAL);
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE); 

}

void
_g8032_ring_wait_flush_fdb_fn(void *info)
{
        tbl_g8032_ring_t *p_g8032_ring = NULL;
        tbl_g8032_ring_key_t g8032_ring_key;
        if (NULL == info)
        {
            return ;
        }
        sal_memset(&g8032_ring_key, 0, sizeof(g8032_ring_key));
        p_g8032_ring = (tbl_g8032_ring_t *)info;
        CTC_TASK_STOP_TIMER(p_g8032_ring->p_flush_fdb_while);
        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_FLUSH_FDB_WHILE); 
        p_g8032_ring ->aps_protection_en = FALSE;
        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_APS_PROTECTION_EN);
        p_g8032_ring->east_port_flush_fdb = FALSE;
        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_FLUSH_FDB);
        p_g8032_ring->west_port_flush_fdb = FALSE;
        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_FLUSH_FDB);

}

void
_g8032_ring_pkt_tx_raps_nr_fn(void *info)
{
    /*transmit a packet every 5 seconds*/
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    raps_pdu_tx_t  raps_pdu_tx_info ;
    if (NULL == info)
    {
        return ;
    }
    sal_memset(&raps_pdu_tx_info, 0, sizeof(raps_pdu_tx_t));
    p_g8032_ring = ( tbl_g8032_ring_t *)info;
    raps_pdu_tx_info.not_flush_fdb = p_g8032_ring->raps_dnf;
    raps_pdu_tx_info.ring_id = p_g8032_ring->key.ring_id;
    
    CTC_TASK_STOP_TIMER(p_g8032_ring->p_tx_raps_nr_while);
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE); 
    
    g8032_ring_pkt_tx_raps_nr(&raps_pdu_tx_info);

    p_g8032_ring->p_tx_raps_nr_while = CTC_TASK_ADD_TIME((_g8032_ring_pkt_tx_raps_nr_fn), info, G8032_RING_DEF_TX_RAPS_INTERVAL);
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE); 

}

void
_g8032_ring_pkt_tx_raps_nr_rb_fn(void *info)
{
    /*transmit a packet every 5 seconds*/
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    raps_pdu_tx_t  raps_pdu_tx_info ;
    if (NULL == info)
    {
        return ;
    }
    sal_memset(&raps_pdu_tx_info, 0, sizeof(raps_pdu_tx_t));
    p_g8032_ring = ( tbl_g8032_ring_t *)info;
    raps_pdu_tx_info.not_flush_fdb = p_g8032_ring->raps_dnf;
    raps_pdu_tx_info.ring_id = p_g8032_ring->key.ring_id;
    
    CTC_TASK_STOP_TIMER(p_g8032_ring->p_tx_raps_nr_nb_while);
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE); 
    
    g8032_ring_pkt_tx_raps_nr_rb(&raps_pdu_tx_info);

    p_g8032_ring->p_tx_raps_nr_nb_while = CTC_TASK_ADD_TIME((_g8032_ring_pkt_tx_raps_nr_rb_fn), info, G8032_RING_DEF_TX_RAPS_INTERVAL);
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE); 

}

void
_g8032_ring_pkt_tx_raps_sf_fn(void *info)
{
    /*transmit a packet every 5 seconds*/
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    raps_pdu_tx_t  raps_pdu_tx_info ;
    if (NULL == info)
    {
        return ;
    }
    sal_memset(&raps_pdu_tx_info, 0, sizeof(raps_pdu_tx_t));
    p_g8032_ring = ( tbl_g8032_ring_t *)info;
    raps_pdu_tx_info.not_flush_fdb = p_g8032_ring->raps_dnf;
    raps_pdu_tx_info.ring_id = p_g8032_ring->key.ring_id;
    
    CTC_TASK_STOP_TIMER(p_g8032_ring->p_tx_raps_sf_while);
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_SF_WHILE); 
    
    g8032_ring_pkt_tx_raps_sf(&raps_pdu_tx_info);
    
    p_g8032_ring->p_tx_raps_sf_while = CTC_TASK_ADD_TIME(_g8032_ring_pkt_tx_raps_sf_fn, info, G8032_RING_DEF_TX_RAPS_INTERVAL);
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_SF_WHILE); 
}

void
_g8032_ring_guard_timer_fn(void  * info)
{
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    if (NULL == info)
    {
        return ;
    }
    
    p_g8032_ring = (tbl_g8032_ring_t *)( info);
    CTC_TASK_STOP_TIMER (p_g8032_ring->p_guard_while);
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_GUARD_WHILE); 
    G8032_TIMER_DEBUG("G8032 ring %d stop guard timer", p_g8032_ring->key.ring_id);
}

void
_g8032_ring_wtr_timer_fn(void  * info)
{
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    tbl_g8032_ring_key_t g8032_ring_key;
    uint32 req_state = 0;

    if (NULL == info)
    {
        return ;
    }
   p_g8032_ring = (tbl_g8032_ring_t *)info;
   g8032_ring_key.ring_id = p_g8032_ring->key.ring_id;
   p_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring_key);
   CTC_TASK_STOP_TIMER (p_g8032_ring->p_wtr_while);
   tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE); 
   G8032_TIMER_DEBUG("G8032 ring %d stop wtr timer", p_g8032_ring->key.ring_id);

    req_state = G8032_PRI_REQ_WTR_EXPIRES;
    G8032FSM[p_g8032_ring->current_node_state][req_state].func(p_g8032_ring, NULL, req_state, FALSE);
}

void
_g8032_ring_wtb_timer_fn(void  * info)
{
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    uint32 req_state = 0;

    if (NULL == info)
    {
        return ;
    }
    
    p_g8032_ring = (tbl_g8032_ring_t *)info;
    CTC_TASK_STOP_TIMER (p_g8032_ring->p_wtb_while);
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE); 
    G8032_TIMER_DEBUG("G8032 ring %d stop wtb timer", p_g8032_ring->key.ring_id);
    
    req_state = G8032_PRI_REQ_WTB_EXPIRES;
    G8032FSM[p_g8032_ring->current_node_state][req_state].func(p_g8032_ring, NULL, req_state, FALSE);
   
}

void
_g8032_ring_pkt_tx_raps_ms_fn(void *info)
{
    /*transmit a packet every 5 seconds*/
     tbl_g8032_ring_t *p_g8032_ring = NULL;
    raps_pdu_tx_t  raps_pdu_tx_info ;
    if (NULL == info)
    {
        return ;
    }
    sal_memset(&raps_pdu_tx_info, 0, sizeof(raps_pdu_tx_t));
    p_g8032_ring = ( tbl_g8032_ring_t *)info;
    raps_pdu_tx_info.not_flush_fdb = p_g8032_ring->raps_dnf;
    raps_pdu_tx_info.ring_id = p_g8032_ring->key.ring_id;

    CTC_TASK_STOP_TIMER(p_g8032_ring->p_tx_raps_ms_while);
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_MS_WHILE); 
    
    g8032_ring_pkt_tx_raps_ms(&raps_pdu_tx_info);
    
    p_g8032_ring->p_tx_raps_ms_while = CTC_TASK_ADD_TIME((_g8032_ring_pkt_tx_raps_ms_fn), info, 5);
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_MS_WHILE); 
    
}


int32
g8032_fsm_stop_transmit_raps(tbl_g8032_ring_t *p_g8032_ring)
{ 

    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    
    if (p_g8032_ring->p_tx_raps_nr_while)
    {
        CTC_TASK_STOP_TIMER(p_g8032_ring->p_tx_raps_nr_while);
        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE);
    }

    if (p_g8032_ring->p_tx_raps_fs_while)
    {
        CTC_TASK_STOP_TIMER(p_g8032_ring->p_tx_raps_fs_while);
        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE);
    }

     if (p_g8032_ring->p_tx_raps_ms_while)
     {
        CTC_TASK_STOP_TIMER(p_g8032_ring->p_tx_raps_ms_while);
        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_MS_WHILE);
     }
    
    if (p_g8032_ring->p_tx_raps_sf_while)
    {
        CTC_TASK_STOP_TIMER(p_g8032_ring->p_tx_raps_sf_while);  
        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_SF_WHILE);
    }
    
    if (p_g8032_ring->p_tx_raps_nr_nb_while) 
    {
        CTC_TASK_STOP_TIMER(p_g8032_ring->p_tx_raps_nr_nb_while);
        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE);
    }

    return PM_E_NONE;
}

int32
g8032_fsm_state_switch(tbl_g8032_ring_t *p_g8032_ring, uint32 new_sate)
{
     if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    logid_sys(LOG_OAM_4_G8032_STATE_CHANGE, p_g8032_ring->key.ring_id, G8032_FSM_STATE_STR(p_g8032_ring->current_node_state), G8032_FSM_STATE_STR(new_sate));
    p_g8032_ring->current_node_state = new_sate;
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_CURRENT_NODE_STATE);

     return PM_E_NONE;
     
}

int32
g8032_fsm_clear_db_ms_info(tbl_g8032_ring_t *p_g8032_ring)
{
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    p_g8032_ring->east_port_is_manual_switch= FALSE;
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_MANUAL_SWITCH);
    p_g8032_ring->west_port_is_manual_switch= FALSE;
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_MANUAL_SWITCH);
    p_g8032_ring->is_local_ms = FALSE;
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_LOCAL_MS);
    return PM_E_NONE;
}

int32
g8032_fsm_clear_db_fs_info(tbl_g8032_ring_t *p_g8032_ring)
{
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    
    p_g8032_ring->east_port_is_force_switch= FALSE;
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_FORCE_SWITCH);
    p_g8032_ring->west_port_is_force_switch= FALSE;
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_FORCE_SWITCH);
    p_g8032_ring->is_local_fs = FALSE;
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_LOCAL_FS);
    
    return PM_E_NONE;
}

int32
g8032_fsm_block_port(tbl_g8032_ring_t *p_g8032_ring, uint8 is_east)
{
     if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    if (is_east)
    {
        p_g8032_ring->east_port_is_blocked = TRUE;
        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_BLOCKED);
    }
    else
    {
        p_g8032_ring->west_port_is_blocked = TRUE;
        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_BLOCKED);
    }
    
    sal_memset(p_g8032_ring->east_port_rec_node_id, 0, 6);
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID);
    p_g8032_ring->east_port_rec_bpr = 0;
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_BPR);

    sal_memset(p_g8032_ring->west_port_rec_node_id, 0, 6);
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_REC_NODE_ID);
    p_g8032_ring->west_port_rec_bpr = 0;
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_REC_BPR);

     return PM_E_NONE;
}

int32
g8032_fsm_unblock_port(tbl_g8032_ring_t *p_g8032_ring, uint8 is_east)
{
     if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
   if (is_east)
    {
        if (p_g8032_ring->east_port_is_blocked)
        {
            p_g8032_ring->east_port_is_blocked = FALSE;
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_BLOCKED);
        }
    }
    else
    {
        if (p_g8032_ring->west_port_is_blocked)
        {
            p_g8032_ring->west_port_is_blocked = FALSE;
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_BLOCKED);
        }
    }

     return PM_E_NONE;
}

int32
g8032_fsm_change_port_state(tbl_g8032_ring_t *p_g8032_ring, uint8 is_east, uint8 block)
{
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    if (block)
    {
        g8032_fsm_block_port(p_g8032_ring, is_east);
    }
    else
    {
        g8032_fsm_unblock_port(p_g8032_ring, is_east);
    }

    return PM_E_NONE;
}


int32
g8032_fsm_stop_timer_with_type(tbl_g8032_ring_t *p_g8032_ring, uint32 type)
{
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    switch(type)
    {
        case TBL_G8032_RING_FLD_P_WTR_WHILE:
            if (p_g8032_ring->p_wtr_while)
            {
                CTC_TASK_STOP_TIMER (p_g8032_ring->p_wtr_while);
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE);  
                G8032_TIMER_DEBUG("G8032 ring %d stop wtr timer", p_g8032_ring->key.ring_id);
            }
            break;
            
        case TBL_G8032_RING_FLD_P_WTB_WHILE:
            if (p_g8032_ring->p_wtb_while)
            {
                CTC_TASK_STOP_TIMER (p_g8032_ring->p_wtb_while);
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);  
                G8032_TIMER_DEBUG("G8032 ring %d stop wtb timer", p_g8032_ring->key.ring_id);
            }
            break;
            
        case TBL_G8032_RING_FLD_P_GUARD_WHILE: 
            if (p_g8032_ring->p_guard_while)
            {
                CTC_TASK_STOP_TIMER (p_g8032_ring->p_guard_while);
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_GUARD_WHILE);  
                G8032_TIMER_DEBUG("G8032 ring %d stop guard timer", p_g8032_ring->key.ring_id);
            }
            break;
        default :
            break;
    }

    return PM_E_NONE;
}

int32
g8032_fsm_add_timer_with_type(tbl_g8032_ring_t *p_g8032_ring, uint32 type)
{
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    switch(type)
    {
        case TBL_G8032_RING_FLD_P_WTR_WHILE:
            p_g8032_ring->p_wtr_while = CTC_TASK_ADD_TIME(_g8032_ring_wtr_timer_fn, p_g8032_ring,  p_g8032_ring->wtr_timeout_val*60);
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE); 
            G8032_TIMER_DEBUG("G8032 ring %d start wtr timer", p_g8032_ring->key.ring_id);
            break;
            
        case TBL_G8032_RING_FLD_P_WTB_WHILE:
            p_g8032_ring->p_wtb_while = CTC_TASK_ADD_TIME_MSEC(_g8032_ring_wtb_timer_fn, p_g8032_ring,  p_g8032_ring->wtb_timeout_val);
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE); 
            G8032_TIMER_DEBUG("G8032 ring %d start wtb timer", p_g8032_ring->key.ring_id);
            break;
            
        case TBL_G8032_RING_FLD_P_GUARD_WHILE: 
            p_g8032_ring->p_guard_while = CTC_TASK_ADD_TIME_MSEC(_g8032_ring_guard_timer_fn, p_g8032_ring,  p_g8032_ring->guard_timeout_val);
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_GUARD_WHILE); 
            G8032_TIMER_DEBUG("G8032 ring %d start guard timer", p_g8032_ring->key.ring_id);
            break;
            
        case TBL_G8032_RING_FLD_P_FLUSH_FDB_WHILE:
            p_g8032_ring ->p_flush_fdb_while =  CTC_TASK_ADD_TIME(_g8032_ring_wait_flush_fdb_fn, p_g8032_ring, G8032_RING_DEF_FLUSH_FDB_INTERVAL);
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_FLUSH_FDB_WHILE);
            break;
            
        default :  
            break;
    }

    return PM_E_NONE;
}

int32
g8032_fsm_set_aps_protection_en(tbl_g8032_ring_t *p_g8032_ring)
{
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    p_g8032_ring ->aps_protection_en = TRUE;
    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_APS_PROTECTION_EN);
   
    g8032_fsm_add_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_FLUSH_FDB_WHILE);  

    return PM_E_NONE;
}

int32
g8032_fsm_flush_port_fdb(tbl_g8032_ring_t *p_g8032_ring, uint8 flush_fdb)
{
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }

    if (flush_fdb)
    {
        
            p_g8032_ring->east_port_flush_fdb = TRUE;
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_FLUSH_FDB);
            if (p_g8032_ring->west_port_ifindex)
            {
                p_g8032_ring->west_port_flush_fdb = TRUE;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_FLUSH_FDB);
            }
    }
    else
    {
        
            p_g8032_ring->east_port_flush_fdb = FALSE;
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_FLUSH_FDB);
            if (p_g8032_ring->west_port_ifindex)
            {
                p_g8032_ring->west_port_flush_fdb = FALSE;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_FLUSH_FDB);
            }

    }

    return PM_E_NONE;
    
}


int32
g8032_ring_pkt_tx_raps(tbl_g8032_ring_t *p_g8032_ring, uint32 type, uint8 not_flush_fdb)
{
    int32 i = 0;
    raps_pdu_tx_t  raps_pdu_tx_info ;
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    sal_memset(&raps_pdu_tx_info, 0, sizeof(raps_pdu_tx_t));
    switch (type)
    {
        case TBL_G8032_RING_FLD_P_TX_RAPS_SF_WHILE:
            g8032_fsm_stop_transmit_raps(p_g8032_ring);
            p_g8032_ring->raps_dnf = not_flush_fdb;
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_RAPS_DNF);
            raps_pdu_tx_info.not_flush_fdb = not_flush_fdb;
            raps_pdu_tx_info.ring_id = p_g8032_ring->key.ring_id;
            for (i = 0; i<3; i++)
            {
                g8032_ring_pkt_tx_raps_sf(&raps_pdu_tx_info); 
            }
            p_g8032_ring->p_tx_raps_sf_while = CTC_TASK_ADD_TIME(_g8032_ring_pkt_tx_raps_sf_fn, (p_g8032_ring),G8032_RING_DEF_TX_RAPS_INTERVAL);
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_SF_WHILE); 
            break;
            
        case TBL_G8032_RING_FLD_P_TX_RAPS_MS_WHILE:
            g8032_fsm_stop_transmit_raps(p_g8032_ring);
            p_g8032_ring->raps_dnf = not_flush_fdb;
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_RAPS_DNF);
            raps_pdu_tx_info.not_flush_fdb = not_flush_fdb;
            raps_pdu_tx_info.ring_id = p_g8032_ring->key.ring_id;
            for (i = 0; i<3; i++)
            {
                g8032_ring_pkt_tx_raps_ms(&raps_pdu_tx_info); 
            }
            p_g8032_ring->p_tx_raps_ms_while = CTC_TASK_ADD_TIME(_g8032_ring_pkt_tx_raps_ms_fn, (p_g8032_ring), G8032_RING_DEF_TX_RAPS_INTERVAL);
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_MS_WHILE); 
            break;
            
        case TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE: 
            g8032_fsm_stop_transmit_raps(p_g8032_ring);
            p_g8032_ring->raps_dnf = not_flush_fdb;
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_RAPS_DNF);
            raps_pdu_tx_info.not_flush_fdb = not_flush_fdb;
            raps_pdu_tx_info.ring_id = p_g8032_ring->key.ring_id;
            for (i = 0; i<3; i++)
            {
                g8032_ring_pkt_tx_raps_fs(&raps_pdu_tx_info); 
            }
            p_g8032_ring->p_tx_raps_fs_while = CTC_TASK_ADD_TIME(_g8032_ring_pkt_tx_raps_fs_fn, (p_g8032_ring), G8032_RING_DEF_TX_RAPS_INTERVAL);
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE); 
            break;
            
        case TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE:
            g8032_fsm_stop_transmit_raps(p_g8032_ring);
            p_g8032_ring->raps_dnf = not_flush_fdb;
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_RAPS_DNF);
            raps_pdu_tx_info.not_flush_fdb = not_flush_fdb;
            raps_pdu_tx_info.ring_id = p_g8032_ring->key.ring_id;
            for (i = 0; i<3; i++)
            {
                g8032_ring_pkt_tx_raps_nr(&raps_pdu_tx_info); 
            }
            p_g8032_ring->p_tx_raps_nr_while = CTC_TASK_ADD_TIME(_g8032_ring_pkt_tx_raps_nr_fn, (p_g8032_ring), G8032_RING_DEF_TX_RAPS_INTERVAL);
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE); 
            break;
            
        case TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE:
            g8032_fsm_stop_transmit_raps(p_g8032_ring);
            p_g8032_ring->raps_dnf = not_flush_fdb;
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_RAPS_DNF);
            raps_pdu_tx_info.not_flush_fdb = not_flush_fdb;
            raps_pdu_tx_info.ring_id = p_g8032_ring->key.ring_id;
            for (i = 0; i<3; i++)
            {
                g8032_ring_pkt_tx_raps_nr_rb(&raps_pdu_tx_info); 
            }
            p_g8032_ring->p_tx_raps_nr_nb_while = CTC_TASK_ADD_TIME(_g8032_ring_pkt_tx_raps_nr_rb_fn, (p_g8032_ring), G8032_RING_DEF_TX_RAPS_INTERVAL);
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE); 
            break;
        default:
            break;
    }   
    
    return PM_E_NONE;
    
}


int32
g8032_fsm_ignore(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east)
{ 
    return PM_E_NONE;
}

int32
g8032_fsm_raps_nr(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east)
{
   
    tbl_interface_t *p_db_interface = NULL;
    tbl_sys_global_t *p_sys_global = NULL;

    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    G8032_EVENT_DEBUG("G8032 ring %d enter into raps_nr event ", p_g8032_ring->key.ring_id);
    p_sys_global = tbl_sys_global_get_sys_global();
    switch(p_g8032_ring->current_node_state)
    {

        case G8032_NODE_STATE_IDLE:
            if (!p_g8032_ring->rpl_owner && !p_g8032_ring->rpl_neighbor)
            {
                
                if (NULL == p_sys_global)
                {
                     return PM_E_NONE;
                }
                if (0 > sal_memcmp(p_sys_global->route_mac, p_raps_pdu_all->node_id, 6))
                {
                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);

                    /* system modified by liwh for bug 53010, 2019-08-08 */
                    /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                    if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                    /* liwh end */
                    {
                        g8032_fsm_change_port_state(p_g8032_ring, TRUE, FALSE);
                    }
    				
                    if (p_g8032_ring->west_port_ifindex)
                    {
                        p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                        /* system modified by liwh for bug 53010, 2019-08-08 */
                        /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                        if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                        /* liwh end */
                        {
                            g8032_fsm_change_port_state(p_g8032_ring,  FALSE, FALSE);
                        }
                    
                    }
                    
                    g8032_fsm_stop_transmit_raps(p_g8032_ring);

                }
            }
            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_IDLE);
            break;
            
       case G8032_NODE_STATE_PROTECTION:

            if (p_g8032_ring->east_port_is_fail  || p_g8032_ring->west_port_is_fail )
            {
                return PM_E_NONE;
            }
             if (p_g8032_ring->rpl_owner && p_g8032_ring->mode_revertive)
            {
                g8032_fsm_add_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE); 
            }

            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PENDING);
            
            break; 
       case G8032_NODE_STATE_MANUAL_SWITCH:
            if (p_g8032_ring->east_port_is_manual_switch|| p_g8032_ring->west_port_is_manual_switch)
            {
                return PM_E_NONE;
            }
            if (p_g8032_ring->rpl_owner && p_g8032_ring->mode_revertive)
            {
               g8032_fsm_add_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE); 
            }

            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PENDING);

            break;  
       case G8032_NODE_STATE_FORCE_SWITCH:

            if (p_g8032_ring->east_port_is_force_switch|| p_g8032_ring->west_port_is_force_switch)
            {
                return PM_E_NONE; 
            }
            
            if (p_g8032_ring->rpl_owner && p_g8032_ring->mode_revertive)
            {
                g8032_fsm_add_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE); 
            }
            
            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PENDING);
            
            break;  
            
        case G8032_NODE_STATE_PENDING:
             if (0 > sal_memcmp(p_sys_global->route_mac, p_raps_pdu_all->node_id, 6))
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);

                /* system modified by liwh for bug 53010, 2019-08-08 */
                /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                /* liwh end */
                {
                    g8032_fsm_change_port_state(p_g8032_ring,  TRUE, FALSE);
                }
    			
                if (p_g8032_ring->west_port_ifindex)
                {
                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                    /* system modified by liwh for bug 53010, 2019-08-08 */
                    /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                    if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                    /* liwh end */
                    {
                        g8032_fsm_change_port_state(p_g8032_ring,  FALSE, FALSE);
                    }
                
                }
                g8032_fsm_stop_transmit_raps(p_g8032_ring);
             }
             
            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PENDING);

            break;

    }
    return PM_E_NONE;
}
 
int32
g8032_fsm_raps_nr_rb(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east)
{
    tbl_interface_t *p_db_interface = NULL;
    tbl_sys_global_t *p_sys_global = NULL;
    
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    G8032_EVENT_DEBUG("G8032 ring %d enter into raps_nr_rb event ", p_g8032_ring->key.ring_id);
    p_sys_global = tbl_sys_global_get_sys_global();

    if (p_g8032_ring->rpl_owner== TRUE)
    {
       if (sal_memcmp(p_sys_global->route_mac, p_raps_pdu_all->node_id, 6))
       {
            logid_sys(LOG_OAM_4_G8032_CONFIG_ALARM, p_g8032_ring->key.ring_id);
            return PM_E_NONE;
       }
    }
    switch(p_g8032_ring->current_node_state)
    {
        case G8032_NODE_STATE_IDLE:
            if (p_g8032_ring->rpl_owner== TRUE)
            {
                return PM_E_NONE;
            }
            if (!p_g8032_ring->rpl_owner && !p_g8032_ring->rpl_neighbor)
            {
                if ((p_g8032_ring->east_port_is_blocked== TRUE))
                {
                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                    /* system modified by liwh for bug 53010, 2019-08-08 */
                    /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                    if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                    /* liwh end */
                    {
                        g8032_fsm_unblock_port(p_g8032_ring, TRUE);
                    }
                    
                }
                if (p_g8032_ring->west_port_ifindex && (p_g8032_ring->west_port_is_blocked== TRUE) )
                {
                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                    /* system modified by liwh for bug 53010, 2019-08-08 */
                    /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                    if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                    /* liwh end */
                    {
                        g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                    }
                    
                }
            }
            if (!p_g8032_ring->rpl_owner)
            {
               if ((!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
                || (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
                || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && p_g8032_ring->is_fwdtocpu 
                && p_g8032_ring->virtual_channel))
                {
                    p_g8032_ring->is_fwdtocpu = FALSE;
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
                }
                g8032_fsm_stop_transmit_raps(p_g8032_ring);
            }

            //g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_IDLE);
            break;
            
        case G8032_NODE_STATE_PROTECTION:
            if (p_g8032_ring->east_port_is_fail  || p_g8032_ring->west_port_is_fail )
            {
                return PM_E_NONE;
            }
            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PENDING);
            break;
            
        case G8032_NODE_STATE_MANUAL_SWITCH:
            if (p_g8032_ring->east_port_is_manual_switch|| p_g8032_ring->west_port_is_manual_switch)
            {
                return PM_E_NONE;  
            }
            
            if ((!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
            || (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
            || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && p_g8032_ring->is_fwdtocpu 
            && p_g8032_ring->virtual_channel))
            {
                p_g8032_ring->is_fwdtocpu = FALSE;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
            }
            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PENDING);
            
            break;
            
        case G8032_NODE_STATE_FORCE_SWITCH:
            if (p_g8032_ring->east_port_is_force_switch || p_g8032_ring->west_port_is_force_switch)
            {
                return PM_E_NONE; 
            }
            
            if ((!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
            || (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
            || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && p_g8032_ring->is_fwdtocpu 
            && p_g8032_ring->virtual_channel))
            {
                p_g8032_ring->is_fwdtocpu = FALSE;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
            }
            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PENDING);
            
            
            break;
            
        case G8032_NODE_STATE_PENDING:
            if (p_g8032_ring->rpl_owner)
            {
                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE);

                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE); 

                if ((!p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu)
                || (p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel))
                {
                    p_g8032_ring->is_fwdtocpu = TRUE;
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
                }
            }

             if (!p_g8032_ring->rpl_owner && !p_g8032_ring->rpl_neighbor)
            {
              
                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                    /* system modified by liwh for bug 53010, 2019-08-08 */
                    /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                    if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                    /* liwh end */
                    {
                       g8032_fsm_unblock_port(p_g8032_ring, TRUE);
                    }
                    
                if (p_g8032_ring->west_port_ifindex )
                {
                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                    /* system modified by liwh for bug 53010, 2019-08-08 */
                    /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                    if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                    /* liwh end */
                    {
                        g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                    }
                    
                }
                
                g8032_fsm_stop_transmit_raps(p_g8032_ring);

                if ((!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
                || (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
                || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && p_g8032_ring->is_fwdtocpu 
                && p_g8032_ring->virtual_channel))
                {
                    p_g8032_ring->is_fwdtocpu = FALSE;
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
                }
            }

             if (p_g8032_ring->rpl_neighbor)
             {
                if (p_g8032_ring->east_port_is_rpl_neighbor)
                {
                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                    if (p_db_interface )
                    {
                        g8032_fsm_block_port(p_g8032_ring, TRUE);
                    }
                    
                    if (p_g8032_ring->west_port_ifindex )
                    {
                        p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                        /* system modified by liwh for bug 53010, 2019-08-08 */
                        /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                        if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                        /* liwh end */
                        {
                            g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                        }
                        
                    }
                }

                if(p_g8032_ring->west_port_ifindex && p_g8032_ring->west_port_is_rpl_neighbor)
                {
                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                    if (p_db_interface )
                    {
                        g8032_fsm_unblock_port(p_g8032_ring, TRUE);
                    }

                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                    if (p_db_interface )
                    {
                        g8032_fsm_block_port(p_g8032_ring, FALSE);
                    }
                }
                 g8032_fsm_stop_transmit_raps(p_g8032_ring);
                if ((!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
                || (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
                || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && p_g8032_ring->is_fwdtocpu 
                && p_g8032_ring->virtual_channel))
                {
                    p_g8032_ring->is_fwdtocpu = FALSE;
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
                }
             }
             
            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_IDLE);
        break;
    }

    return PM_E_NONE;
}


int32
g8032_fsm_wtr_expires(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east)
{
   
    tbl_interface_t *p_db_interface = NULL;
    tbl_g8032_ring_t *p_g8032_majorring = NULL;
    tbl_g8032_ring_key_t g8032_ring_key;
    uint32 i = 0;

    sal_memset(&g8032_ring_key, 0, sizeof(g8032_ring_key));

    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
     G8032_EVENT_DEBUG("G8032 ring %d enter into wtr_expires event ", p_g8032_ring->key.ring_id);
    switch(p_g8032_ring->current_node_state)
    {
        case G8032_NODE_STATE_IDLE:
            break;
        case G8032_NODE_STATE_PROTECTION:
            break;  
        case G8032_NODE_STATE_MANUAL_SWITCH:
            break;
        case G8032_NODE_STATE_FORCE_SWITCH:
            break;
        case G8032_NODE_STATE_PENDING:
            
            if (p_g8032_ring->rpl_owner)
            {            
                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);     

                if (p_g8032_ring->east_port_is_rpl)
                {
                    if (p_g8032_ring->east_port_is_blocked)
                    {
                        if (p_g8032_ring->west_port_ifindex)
                        {
                            p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                            /* system modified by liwh for bug 53010, 2019-08-08 */
                            /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                            if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                            /* liwh end */
                            {
                                g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                            }
                        }

                        g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE, TRUE);
                    }
                    else
                    {
                        p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                        /* system modified by liwh for bug 53010, 2019-08-08 */
                        /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                        if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                        /* liwh end */
                        {   
                            if (p_g8032_ring->is_sub_ring && p_g8032_ring->g8032_major_ring_id)
                            {
                                g8032_ring_key.ring_id = p_g8032_ring->g8032_major_ring_id;
                                p_g8032_majorring = tbl_g8032_ring_get_g8032_ring(&g8032_ring_key);

                                if (p_g8032_majorring)
                                {
                                    for (i = 0; i<3; i++)
                                    {
                                        g8032_ring_pkt_tx_raps_event(p_g8032_majorring);
                                    }
                                } 
                            }
                            g8032_fsm_block_port(p_g8032_ring, TRUE);
                        }
                         if (p_g8032_ring->east_port_is_blocked)
                        {
                              if (p_g8032_ring->west_port_ifindex)
                            {
                                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                                /* system modified by liwh for bug 53010, 2019-08-08 */
                                /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                                if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                                /* liwh end */
                                {
                                    g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                                }
                            } 
                        }

                        g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE, FALSE);

                        g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);
                        
                    }
                }

                if (p_g8032_ring->west_port_ifindex && (p_g8032_ring->west_port_is_rpl))
                {
                    if (p_g8032_ring->west_port_is_blocked)
                    {
                        p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                        /* system modified by liwh for bug 53010, 2019-08-08 */
                        /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                        if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                        /* liwh end */
                        {
                            g8032_fsm_unblock_port(p_g8032_ring, TRUE);
                        }

                        g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE, TRUE);
                    }
                    else
                    {
                        p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                        /* system modified by liwh for bug 53010, 2019-08-08 */
                        /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                        if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                        /* liwh end */
                        {
                            g8032_fsm_block_port(p_g8032_ring, FALSE);
                        }
                        p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                        /* system modified by liwh for bug 53010, 2019-08-08 */
                        /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                        if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                        /* liwh end */
                        {
                            g8032_fsm_unblock_port(p_g8032_ring, TRUE);
                        }
                        
                        g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE, FALSE);
                        g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);

                    }
                }
                if ((!p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
                    || (p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
                    || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && !p_g8032_ring->is_fwdtocpu 
                    && p_g8032_ring->virtual_channel))
                {
                    p_g8032_ring->is_fwdtocpu = TRUE;
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
                }
            }
            
            g8032_fsm_clear_db_ms_info(p_g8032_ring);
            g8032_fsm_clear_db_fs_info(p_g8032_ring);
            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_IDLE);
            break;
         }
    return PM_E_NONE;
}


int32
g8032_fsm_raps_sf(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east)
{
   
    tbl_interface_t *p_db_interface = NULL;
    
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    
    G8032_EVENT_DEBUG("G8032 ring %d enter into raps_sf event ", p_g8032_ring->key.ring_id);
    
    g8032_fsm_set_aps_protection_en(p_g8032_ring);
    switch(p_g8032_ring->current_node_state)
    {
        case G8032_NODE_STATE_IDLE:
        case G8032_NODE_STATE_MANUAL_SWITCH:
            p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
            /* system modified by liwh for bug 53010, 2019-08-08 */
            /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
            if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
            /* liwh end */
            {
                g8032_fsm_unblock_port(p_g8032_ring, TRUE);
            }
            if (p_g8032_ring->west_port_ifindex)
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                /* system modified by liwh for bug 53010, 2019-08-08 */
                /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                /* liwh end */
                {
                    g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                }
            
            }
            
            g8032_fsm_stop_transmit_raps(p_g8032_ring);

            if ((!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
            || (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
            || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && p_g8032_ring->is_fwdtocpu 
            && p_g8032_ring->virtual_channel))
            {
                p_g8032_ring->is_fwdtocpu = FALSE;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
            }
            
            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PROTECTION);

            g8032_fsm_clear_db_ms_info(p_g8032_ring);

            break;
            
        case G8032_NODE_STATE_PROTECTION:    
            break;  
        case G8032_NODE_STATE_FORCE_SWITCH:
            break;
        case G8032_NODE_STATE_PENDING:
            p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
            /* system modified by liwh for bug 53010, 2019-08-08 */
            /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
            if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
            /* liwh end */
            {
                g8032_fsm_unblock_port(p_g8032_ring, TRUE);
            }
            
            if (p_g8032_ring->west_port_ifindex)
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                /* system modified by liwh for bug 53010, 2019-08-08 */
                /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                /* liwh end */    
                {
                    g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                }
            }
            
            g8032_fsm_stop_transmit_raps(p_g8032_ring);
            if (p_g8032_ring->rpl_owner)
            {
                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE);

                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);     
            }

            if ((!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
            || (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
            || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && p_g8032_ring->is_fwdtocpu 
            && p_g8032_ring->virtual_channel))
            {
                p_g8032_ring->is_fwdtocpu = FALSE;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
            }
            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PROTECTION);
            break;
     }
    
    return PM_E_NONE;
}


static int32 
_g8032_fsm_local_sf_state_idle(tbl_g8032_ring_t *p_g8032_ring, uint8 is_east)
{
    tbl_interface_t *p_db_interface = NULL;
    tbl_g8032_ring_t *p_g8032_majorring = NULL;
    tbl_g8032_ring_key_t g8032_ring_key;
    uint32 i = 0;

    sal_memset(&g8032_ring_key, 0, sizeof(g8032_ring_key));
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }

    if(is_east)
    {
        if (p_g8032_ring->east_port_is_fail)
        {

            if (p_g8032_ring->east_port_is_blocked)
            {
                if (p_g8032_ring->west_port_ifindex)
                {
                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                    if (!p_g8032_ring->west_port_is_fail)
                    {
                        g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                    }
                
                }

                g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_SF_WHILE, TRUE);
                        
                g8032_fsm_flush_port_fdb(p_g8032_ring, FALSE);
             }
             else
             {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                if (p_g8032_ring->is_sub_ring && p_g8032_ring->g8032_major_ring_id)
                {
                    g8032_ring_key.ring_id = p_g8032_ring->g8032_major_ring_id;
                    p_g8032_majorring = tbl_g8032_ring_get_g8032_ring(&g8032_ring_key);

                    if (p_g8032_majorring)
                    {
                        for (i = 0; i<3; i++)
                        {
                            g8032_ring_pkt_tx_raps_event(p_g8032_majorring);
                        }
                    } 
                }
                if (p_db_interface)
                {
                    g8032_fsm_block_port(p_g8032_ring, TRUE);
                }
               
                if (p_g8032_ring->west_port_ifindex)
                {
                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                    if (!p_g8032_ring->west_port_is_fail)
                    {
                        g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                    }
                
                }
                g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_SF_WHILE, FALSE);
                
                g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);
             }
        }
    }
    else
    {
        if (p_g8032_ring->west_port_is_fail)
        {
            if (p_g8032_ring->west_port_is_blocked)
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);//
                if (!p_g8032_ring->east_port_is_fail)
                {
                    g8032_fsm_unblock_port(p_g8032_ring, TRUE);
                }

                g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_SF_WHILE, TRUE);
                g8032_fsm_flush_port_fdb(p_g8032_ring, FALSE);
            }
            else
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                if (p_db_interface)
                {
                    g8032_fsm_block_port(p_g8032_ring, FALSE);
                }
                
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                if (!p_g8032_ring->east_port_is_fail)
                {
                    g8032_fsm_unblock_port(p_g8032_ring, TRUE);
                }

                g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_SF_WHILE, FALSE);

                g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);
            }
        }
    }
    
    g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PROTECTION);
    
    return PM_E_NONE;
    
}

int32
g8032_fsm_local_sf(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east)
{
   
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    G8032_EVENT_DEBUG("G8032 ring %d enter into local_sf event ", p_g8032_ring->key.ring_id);
    g8032_fsm_set_aps_protection_en(p_g8032_ring);
    
    switch(p_g8032_ring->current_node_state)
    {
        case G8032_NODE_STATE_IDLE:
        case G8032_NODE_STATE_PROTECTION:
        case G8032_NODE_STATE_MANUAL_SWITCH:
            _g8032_fsm_local_sf_state_idle(p_g8032_ring, is_east);
            g8032_fsm_clear_db_ms_info(p_g8032_ring);
            if ((!p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
                || (p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
                || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && !p_g8032_ring->is_fwdtocpu 
                && p_g8032_ring->virtual_channel))
            {
                p_g8032_ring->is_fwdtocpu = TRUE;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
            }
            break;
            
        case G8032_NODE_STATE_FORCE_SWITCH:
            break;
            
        case G8032_NODE_STATE_PENDING:
            _g8032_fsm_local_sf_state_idle(p_g8032_ring, is_east);
            if ((!p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
                || (p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
                || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && !p_g8032_ring->is_fwdtocpu 
                && p_g8032_ring->virtual_channel))
            {
                p_g8032_ring->is_fwdtocpu = TRUE;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
            }
            if (p_g8032_ring->rpl_owner)
            {
                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE);

                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);     
            }
            break;
     }
     
    return PM_E_NONE;
}


int32
g8032_fsm_local_clear(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east)
{

    tbl_interface_t *p_db_interface = NULL;
    tbl_g8032_ring_t *p_g8032_majorring = NULL;
    tbl_g8032_ring_key_t g8032_ring_key;
    uint32 i = 0;

    sal_memset(&g8032_ring_key, 0, sizeof(g8032_ring_key));
    if (NULL == p_g8032_ring)
    {
    return PM_E_NONE;
    }
    G8032_EVENT_DEBUG("G8032 ring %d enter into local_clear event ", p_g8032_ring->key.ring_id);
    switch(p_g8032_ring->current_node_state)
    {
    case G8032_NODE_STATE_IDLE:
        break;
        
    case G8032_NODE_STATE_PROTECTION:
        break;  
        
    case G8032_NODE_STATE_MANUAL_SWITCH:
        if (p_g8032_ring->east_port_is_blocked ||p_g8032_ring->west_port_is_blocked)
        {
            g8032_fsm_add_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_GUARD_WHILE); 

             g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE, FALSE);
            
            if (p_g8032_ring->rpl_owner && p_g8032_ring->mode_revertive)
            {
                 g8032_fsm_add_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);   
            }
        }
         g8032_fsm_clear_db_ms_info(p_g8032_ring);

         g8032_fsm_clear_db_fs_info(p_g8032_ring);

         g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PENDING);

        break;
        
    case G8032_NODE_STATE_FORCE_SWITCH:
        
        g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PENDING);
        if (p_g8032_ring->east_port_is_blocked ||p_g8032_ring->west_port_is_blocked)
        {
            g8032_fsm_add_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_GUARD_WHILE); 

            g8032_fsm_clear_db_ms_info(p_g8032_ring);

            g8032_fsm_clear_db_fs_info(p_g8032_ring);

            g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE, FALSE);

            if (p_g8032_ring->east_port_is_fail  || p_g8032_ring->west_port_is_fail )
            {
                  if (p_g8032_ring->east_port_is_fail )
                 {
                    G8032FSM[p_g8032_ring->current_node_state][G8032_PRI_REQ_L_SF].func(p_g8032_ring, NULL, G8032_PRI_REQ_L_SF, TRUE);
                 }

                  if (p_g8032_ring->west_port_is_fail )
                 {
                    G8032FSM[p_g8032_ring->current_node_state][G8032_PRI_REQ_L_SF].func(p_g8032_ring, NULL, G8032_PRI_REQ_L_SF, FALSE);
                 }
            }
            else
            {
                if (p_g8032_ring->rpl_owner && p_g8032_ring->mode_revertive)
                {
                    g8032_fsm_add_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);   
                }
            }
        }     
        break;

    case G8032_NODE_STATE_PENDING:
        g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_GUARD_WHILE); 
        if (p_g8032_ring->rpl_owner)
        {            
            g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);     

            g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE);  
            if (p_g8032_ring->east_port_is_rpl)
            {
                if (p_g8032_ring->east_port_is_blocked)
                {
                    if (p_g8032_ring->west_port_ifindex)
                    {
                        p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                        if (p_db_interface)
                        {
                            g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                        }
                    }

                    g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE, TRUE);

                }
                else
                {
                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                    if (p_db_interface)
                    {
                        if (p_g8032_ring->is_sub_ring && p_g8032_ring->g8032_major_ring_id)
                        {
                            g8032_ring_key.ring_id = p_g8032_ring->g8032_major_ring_id;
                            p_g8032_majorring = tbl_g8032_ring_get_g8032_ring(&g8032_ring_key);

                            if (p_g8032_majorring)
                            {
                                for (i = 0; i<3; i++)
                                {
                                    g8032_ring_pkt_tx_raps_event(p_g8032_majorring);
                                }
                            } 
                        }
                        g8032_fsm_block_port(p_g8032_ring, TRUE);
                    }
                    if (p_g8032_ring->west_port_ifindex)
                    {
                        p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                        if (p_db_interface)
                        {
                            g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                        }
                    } 

                    g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE, FALSE);

                    g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);

                }
            }

            if (p_g8032_ring->west_port_ifindex && (p_g8032_ring->west_port_is_rpl))
            {
                if (p_g8032_ring->west_port_is_blocked)
                {
                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                    if (p_db_interface)
                    {
                        g8032_fsm_unblock_port(p_g8032_ring, TRUE);
                    }

                    g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE, TRUE);
                }
                else
                {
                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                    if (p_db_interface)
                    {
                        g8032_fsm_block_port(p_g8032_ring, FALSE);
                    }
                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                    if (p_db_interface)
                    {
                        g8032_fsm_unblock_port(p_g8032_ring, TRUE);
                    }
                    g8032_fsm_stop_transmit_raps(p_g8032_ring);

                    g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE, TRUE);

                    g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);
                }
            }

            if ((!p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
                || (p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
                || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && !p_g8032_ring->is_fwdtocpu 
                && p_g8032_ring->virtual_channel))
            {
                p_g8032_ring->is_fwdtocpu = TRUE;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
            }
        }
        g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_IDLE);
        break;
    }
    return PM_E_NONE;
}

static int32 
_g8032_fsm_local_fs_state(tbl_g8032_ring_t *p_g8032_ring)
{
    tbl_interface_t *p_db_interface = NULL;
    tbl_g8032_ring_t *p_g8032_majorring = NULL;
    tbl_g8032_ring_key_t g8032_ring_key;
    uint32 i = 0;

    sal_memset(&g8032_ring_key, 0, sizeof(g8032_ring_key));

    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
  
    if (p_g8032_ring->east_port_is_force_switch)
    {
        if (p_g8032_ring->east_port_is_blocked)
        {
            if (p_g8032_ring->west_port_ifindex)
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                if (p_db_interface)
                {
                    g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                }
            }
            g8032_fsm_stop_transmit_raps(p_g8032_ring);//

             g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE, TRUE);

            g8032_fsm_flush_port_fdb(p_g8032_ring, FALSE);
            
        }
        else
        {
            p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
            if (p_db_interface)
            {
                g8032_fsm_block_port(p_g8032_ring, TRUE);
            }

            if (p_g8032_ring->west_port_ifindex)
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                if (p_db_interface )
                {
                    g8032_fsm_unblock_port(p_g8032_ring, FALSE);//check
                }
             }
            g8032_fsm_stop_transmit_raps(p_g8032_ring);
            g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE, FALSE);

            g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);
             if (p_g8032_ring->is_sub_ring && p_g8032_ring->g8032_major_ring_id)
            {
                g8032_ring_key.ring_id = p_g8032_ring->g8032_major_ring_id;
                p_g8032_majorring = tbl_g8032_ring_get_g8032_ring(&g8032_ring_key);

                if (p_g8032_majorring)
                {
                    for (i = 0; i<3; i++)
                    {
                        g8032_ring_pkt_tx_raps_event(p_g8032_majorring);
                    }
                } 
            }
            
            }
        }
        
        if (p_g8032_ring->west_port_ifindex && p_g8032_ring->west_port_is_force_switch)
        {
            if (p_g8032_ring->west_port_is_blocked)
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                if (p_db_interface)
                {
                    g8032_fsm_unblock_port(p_g8032_ring, TRUE);
                }
                g8032_fsm_stop_transmit_raps(p_g8032_ring);
                g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE, TRUE);
  
                g8032_fsm_flush_port_fdb(p_g8032_ring, FALSE);
            }
            else
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                if (p_db_interface)
                {
                    g8032_fsm_block_port(p_g8032_ring, FALSE);
                }

                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                if (p_db_interface )
                {
                    g8032_fsm_unblock_port(p_g8032_ring, TRUE);
                }
                g8032_fsm_stop_transmit_raps(p_g8032_ring);
                g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE, FALSE);
                
                g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);
        }
    
    }

    g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_FORCE_SWITCH);

    return PM_E_NONE;
}

int32
g8032_fsm_local_fs(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east)
{
    tbl_interface_t *p_db_interface = NULL;
    tbl_g8032_ring_t *p_g8032_majorring = NULL;
    tbl_g8032_ring_key_t g8032_ring_key;
    uint32 i = 0;

    sal_memset(&g8032_ring_key, 0, sizeof(g8032_ring_key));
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    G8032_EVENT_DEBUG("G8032 ring %d enter into local_fs event ", p_g8032_ring->key.ring_id);
    g8032_fsm_set_aps_protection_en(p_g8032_ring);
    if ((!p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
        || (p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
        || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && !p_g8032_ring->is_fwdtocpu 
        && p_g8032_ring->virtual_channel))
    {
        p_g8032_ring->is_fwdtocpu = TRUE;
        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
    }
    switch(p_g8032_ring->current_node_state)
    {
        case G8032_NODE_STATE_IDLE:
        case G8032_NODE_STATE_PROTECTION:
        case G8032_NODE_STATE_MANUAL_SWITCH:
            _g8032_fsm_local_fs_state(p_g8032_ring);
            g8032_fsm_clear_db_ms_info(p_g8032_ring);
            break;
            
        case G8032_NODE_STATE_FORCE_SWITCH:
            g8032_fsm_clear_db_ms_info(p_g8032_ring);
            if (is_east)
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);

                if(!p_g8032_ring->east_port_is_blocked)
                {
                    if (p_g8032_ring->is_sub_ring && p_g8032_ring->g8032_major_ring_id)
                    {
                        g8032_ring_key.ring_id = p_g8032_ring->g8032_major_ring_id;
                        p_g8032_majorring = tbl_g8032_ring_get_g8032_ring(&g8032_ring_key);

                        if (p_g8032_majorring)
                        {
                            for (i = 0; i<3; i++)
                            {
                                g8032_ring_pkt_tx_raps_event(p_g8032_majorring);
                            }
                        } 
                    }
                }
                if (p_db_interface)
                {
                    g8032_fsm_block_port(p_g8032_ring, TRUE);
                }
                
                g8032_fsm_stop_transmit_raps(p_g8032_ring);

                g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE, FALSE);

                g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);

                
             }
            else 
            {
                if (p_g8032_ring->west_port_ifindex && p_g8032_ring->west_port_is_force_switch)
                {
                    p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                    if (p_db_interface)
                    {
                        g8032_fsm_block_port(p_g8032_ring, FALSE);
                    }
                    g8032_fsm_stop_transmit_raps(p_g8032_ring);

                    g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE, FALSE);
                    
                    g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);
                }
                
            }
            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_FORCE_SWITCH);
            break; 
            
        case G8032_NODE_STATE_PENDING:
             _g8032_fsm_local_fs_state(p_g8032_ring);
             if (p_g8032_ring->rpl_owner)
            {
                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE);

                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);     
            }
            break;
            
        default:
            break;
     }
    return PM_E_NONE;
}


int32
g8032_fsm_raps_ms(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east)
{
   tbl_interface_t *p_db_interface = NULL;
    
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    
    G8032_EVENT_DEBUG("G8032 ring %d enter into raps_ms event ", p_g8032_ring->key.ring_id);
    g8032_fsm_set_aps_protection_en(p_g8032_ring);
    switch(p_g8032_ring->current_node_state)
    {
        case G8032_NODE_STATE_IDLE:
            p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
            /* system modified by liwh for bug 53010, 2019-08-08 */
            /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
            if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
            /* liwh end */
            {
                g8032_fsm_unblock_port(p_g8032_ring, TRUE);
            }
            if (p_g8032_ring->west_port_ifindex)
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                /* system modified by liwh for bug 53010, 2019-08-08 */
                /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                /* liwh end */
                {
                    g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                }
            
            }
            
            g8032_fsm_stop_transmit_raps(p_g8032_ring);

            if ((!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
            || (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
            || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && p_g8032_ring->is_fwdtocpu 
            && p_g8032_ring->virtual_channel))
            {
                p_g8032_ring->is_fwdtocpu = FALSE;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
            }
            
            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_MANUAL_SWITCH);
            break;
        case G8032_NODE_STATE_PROTECTION:
            break;  
        case G8032_NODE_STATE_MANUAL_SWITCH:
            if (p_g8032_ring->east_port_is_blocked ||p_g8032_ring->west_port_is_blocked)
            {
                g8032_fsm_clear_db_ms_info(p_g8032_ring);
                
                g8032_fsm_add_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_GUARD_WHILE); 
                
                g8032_fsm_stop_transmit_raps(p_g8032_ring);

                g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE, FALSE);
                
                if (p_g8032_ring->rpl_owner && p_g8032_ring->mode_revertive)
                {
                    g8032_fsm_add_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);   
                }
                g8032_fsm_clear_db_ms_info(p_g8032_ring);
                g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PENDING);
            }
            else
            {
                g8032_fsm_clear_db_ms_info(p_g8032_ring);
                g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_MANUAL_SWITCH);
            }
            
            break;
        case G8032_NODE_STATE_FORCE_SWITCH:
            break;
            
        case G8032_NODE_STATE_PENDING:
            p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
            /* system modified by liwh for bug 53010, 2019-08-08 */
            /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
            if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
            /* liwh end */
            {
                g8032_fsm_unblock_port(p_g8032_ring, TRUE);
            }
            if (p_g8032_ring->west_port_ifindex)
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                /* system modified by liwh for bug 53010, 2019-08-08 */
                /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                /* liwh end */
                {
                    g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                }
            
            }
             if (p_g8032_ring->rpl_owner)
            {
                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE);

                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);     
            }
            g8032_fsm_stop_transmit_raps(p_g8032_ring);

             if ((!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
            || (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
            || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && p_g8032_ring->is_fwdtocpu 
            && p_g8032_ring->virtual_channel))
            {
                p_g8032_ring->is_fwdtocpu = FALSE;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
            }
            
            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_MANUAL_SWITCH);
            break;
     }
    return PM_E_NONE;
}

static int32 
_g8032_fsm_local_ms_state_idle(tbl_g8032_ring_t *p_g8032_ring)
{

    tbl_interface_t *p_db_interface = NULL;
    tbl_g8032_ring_t *p_g8032_majorring = NULL;
    tbl_g8032_ring_key_t g8032_ring_key;
    uint32 i = 0;

    sal_memset(&g8032_ring_key, 0, sizeof(g8032_ring_key));
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    
    if (p_g8032_ring->east_port_is_manual_switch)
    {
        if (p_g8032_ring->east_port_is_blocked)
        {
            
            if (p_g8032_ring->west_port_ifindex)
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                if (p_db_interface)
                {
                    g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                }
            }
            g8032_fsm_stop_transmit_raps(p_g8032_ring);
            g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_MS_WHILE, TRUE);
            
            g8032_fsm_flush_port_fdb(p_g8032_ring, FALSE);
         }
         else
         {
            p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
            if (p_db_interface )
            {
                g8032_fsm_block_port(p_g8032_ring, TRUE);
                
            }
           
            if (p_g8032_ring->west_port_ifindex)
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                if (p_db_interface)
                {
                    g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                }
            
            }
            g8032_fsm_stop_transmit_raps(p_g8032_ring);
            
            g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_MS_WHILE, FALSE);
            
            g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);

            if (p_g8032_ring->is_sub_ring && p_g8032_ring->g8032_major_ring_id)
            {
                g8032_ring_key.ring_id = p_g8032_ring->g8032_major_ring_id;
                p_g8032_majorring = tbl_g8032_ring_get_g8032_ring(&g8032_ring_key);

                if (p_g8032_majorring)
                {
                    for (i = 0; i<3; i++)
                    {
                        g8032_ring_pkt_tx_raps_event(p_g8032_majorring);
                    }
                } 
            }
            

         }
    }
    
    if (p_g8032_ring->west_port_ifindex && p_g8032_ring->west_port_is_manual_switch)
    {

        if (p_g8032_ring->west_port_is_blocked)
        {
            p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
            
            if (p_db_interface)
            {
                g8032_fsm_unblock_port(p_g8032_ring, TRUE);
            }
            g8032_fsm_stop_transmit_raps(p_g8032_ring);

            g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_MS_WHILE, TRUE);
            
            g8032_fsm_flush_port_fdb(p_g8032_ring, FALSE);
        }
        else
        {
            p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
            /* system modified by liwh for bug 53010, 2019-08-08 */
            /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
            if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
            /* liwh end */
            {
                g8032_fsm_block_port(p_g8032_ring, FALSE);
            }

            p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
            if (p_db_interface )
            {
                g8032_fsm_unblock_port(p_g8032_ring, TRUE);
            }
            g8032_fsm_stop_transmit_raps(p_g8032_ring);
            
            g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_MS_WHILE, FALSE);
            
            g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);

        }
    
    }
   
    if ((!p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
        || (p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
        || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && !p_g8032_ring->is_fwdtocpu 
        && p_g8032_ring->virtual_channel))
    {
        p_g8032_ring->is_fwdtocpu = TRUE;
        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
    }
    g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_MANUAL_SWITCH);
    
    return PM_E_NONE;
}

int32
g8032_fsm_local_ms(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east)
{
   
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    G8032_EVENT_DEBUG("G8032 ring %d enter into local_ms event ", p_g8032_ring->key.ring_id);
    g8032_fsm_set_aps_protection_en(p_g8032_ring); 
    switch(p_g8032_ring->current_node_state)
    {
        case G8032_NODE_STATE_IDLE:
            _g8032_fsm_local_ms_state_idle(p_g8032_ring);
            break;
        case G8032_NODE_STATE_PROTECTION:
            break;  
        case G8032_NODE_STATE_MANUAL_SWITCH:
            break;
        case G8032_NODE_STATE_FORCE_SWITCH:
            break;
        case G8032_NODE_STATE_PENDING:
             if (p_g8032_ring->rpl_owner)
            {
                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE);

                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);     
            }
              _g8032_fsm_local_ms_state_idle(p_g8032_ring);
            break;
       default:
            break;
     }
    return PM_E_NONE;
}


int32
g8032_fsm_wtb_expires(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east)
{
   
   tbl_interface_t *p_db_interface = NULL;
   tbl_g8032_ring_t *p_g8032_majorring = NULL;
   tbl_g8032_ring_key_t g8032_ring_key;
   uint32 i = 0;

    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    G8032_EVENT_DEBUG("G8032 ring %d enter into wtb_expires event ", p_g8032_ring->key.ring_id);
    switch(p_g8032_ring->current_node_state)
    {
        case G8032_NODE_STATE_IDLE:
            break;
        case G8032_NODE_STATE_PROTECTION:
            break;  
        case G8032_NODE_STATE_MANUAL_SWITCH:
            break;
        case G8032_NODE_STATE_FORCE_SWITCH:
            break;
        case G8032_NODE_STATE_PENDING:
         g8032_fsm_clear_db_ms_info(p_g8032_ring);

         g8032_fsm_clear_db_fs_info(p_g8032_ring);
            
           if (p_g8032_ring->rpl_owner)
            {            
                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE);     

                if (p_g8032_ring->east_port_is_rpl)
                {
                    if (p_g8032_ring->east_port_is_blocked)
                    {
                        if (p_g8032_ring->west_port_ifindex)
                        {
                            p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                            /* system modified by liwh for bug 53010, 2019-08-08 */
                            /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                            if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                            /* liwh end */
                            {
                               g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                            }
                        }
                        g8032_fsm_stop_transmit_raps(p_g8032_ring);
                        g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE, TRUE);
                    }
                    else
                    {
                        p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                        /* system modified by liwh for bug 53010, 2019-08-08 */
                        /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                        if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                        /* liwh end */    
                        {
                            if (p_g8032_ring->is_sub_ring && p_g8032_ring->g8032_major_ring_id)
                            {
                                g8032_ring_key.ring_id = p_g8032_ring->g8032_major_ring_id;
                                p_g8032_majorring = tbl_g8032_ring_get_g8032_ring(&g8032_ring_key);

                                if (p_g8032_majorring)
                                {
                                    for (i = 0; i<3; i++)
                                    {
                                        g8032_ring_pkt_tx_raps_event(p_g8032_majorring);
                                    }
                                } 
                            }
                            g8032_fsm_block_port(p_g8032_ring, TRUE);
                        }
                         if (p_g8032_ring->east_port_is_blocked)
                        {
                              if (p_g8032_ring->west_port_ifindex)
                            {
                                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                                /* system modified by liwh for bug 53010, 2019-08-08 */
                                /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                                if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                                /* liwh end */  
                                {
                                    g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                                }
                            } 
                     }
                    g8032_fsm_stop_transmit_raps(p_g8032_ring);
                    g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE, FALSE);

                    g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);

                    }
                }

                if (p_g8032_ring->west_port_ifindex && (p_g8032_ring->west_port_is_rpl))
                {
                    if (p_g8032_ring->west_port_is_blocked)
                    {
                        p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                        /* system modified by liwh for bug 53010, 2019-08-08 */
                        /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                        if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                        /* liwh end */      
                        {
                            g8032_fsm_unblock_port(p_g8032_ring, TRUE);
                        }
                        g8032_fsm_stop_transmit_raps(p_g8032_ring);
                        g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE, TRUE);
                    }
                    else
                    {
                        p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                        /* system modified by liwh for bug 53010, 2019-08-08 */
                        /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                        if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                        /* liwh end */  
                        {
                            g8032_fsm_block_port(p_g8032_ring, FALSE);
                        }
                        p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                        /* system modified by liwh for bug 53010, 2019-08-08 */
                        /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                        if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                        /* liwh end */  
                        {
                            g8032_fsm_unblock_port(p_g8032_ring, TRUE);
                        }
                        g8032_fsm_stop_transmit_raps(p_g8032_ring);
                        g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE, FALSE);

                        g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);
                    }
                }
                if ((!p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
                    || (p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
                    || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && !p_g8032_ring->is_fwdtocpu 
                    && p_g8032_ring->virtual_channel))
                {
                    p_g8032_ring->is_fwdtocpu = TRUE;
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
                }
            }

            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_IDLE);
            break;
     }
    return PM_E_NONE;
}

int32
g8032_fsm_raps_fs(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east)
{
   
    tbl_interface_t *p_db_interface = NULL;
    
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    G8032_EVENT_DEBUG("G8032 ring %d enter into raps_fs event ", p_g8032_ring->key.ring_id);
    g8032_fsm_set_aps_protection_en(p_g8032_ring);
    switch(p_g8032_ring->current_node_state)
    {
    
        case G8032_NODE_STATE_IDLE:
        case G8032_NODE_STATE_PROTECTION:
        case G8032_NODE_STATE_MANUAL_SWITCH:
            g8032_fsm_clear_db_ms_info(p_g8032_ring);
            p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
            if (p_db_interface)
            {
                g8032_fsm_unblock_port(p_g8032_ring, TRUE);
            }
            
            if (p_g8032_ring->west_port_ifindex)
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                if (p_db_interface )
                {
                   g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                }
            
            }
            g8032_fsm_stop_transmit_raps(p_g8032_ring);
            
            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_FORCE_SWITCH);

            g8032_fsm_clear_db_ms_info(p_g8032_ring);

            if ((!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
            || (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
            || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && p_g8032_ring->is_fwdtocpu 
            && p_g8032_ring->virtual_channel))
            {
                p_g8032_ring->is_fwdtocpu = FALSE;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
            }

            break; 
        
        case G8032_NODE_STATE_FORCE_SWITCH:
            break;
        case G8032_NODE_STATE_PENDING:
          
            p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
            /* system modified by liwh for bug 53010, 2019-08-08 */
            /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
            if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
            /* liwh end */  
            {
                g8032_fsm_unblock_port(p_g8032_ring, TRUE);
            }
            if (p_g8032_ring->west_port_ifindex)
            {
                p_db_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
                /* system modified by liwh for bug 53010, 2019-08-08 */
                /*if (p_db_interface && tbl_interface_is_running(p_db_interface))*/
                if (g8032_ring_check_cfm_and_interface_state(p_db_interface, p_g8032_ring))
                /* liwh end */  
                {
                   g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                }
            
            }
            
            g8032_fsm_stop_transmit_raps(p_g8032_ring);
            if (p_g8032_ring->rpl_owner)
            {
                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE);

                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);     
            }

            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_FORCE_SWITCH);

            if ((!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && !p_g8032_ring->is_sub_ring)
            || (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_fwdtocpu && p_g8032_ring->virtual_channel)
            || (!p_g8032_ring->g8032_major_ring_id && p_g8032_ring->is_sub_ring && p_g8032_ring->is_fwdtocpu 
            && p_g8032_ring->virtual_channel))
            {
                p_g8032_ring->is_fwdtocpu = FALSE;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_IS_FWDTOCPU);  
            }
            
            break;
     }
    return PM_E_NONE;
}

int32
g8032_fsm_local_clear_sf(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east)
{
   
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    G8032_EVENT_DEBUG("G8032 ring %d enter into local_clear_sf ", p_g8032_ring->key.ring_id);
    switch(p_g8032_ring->current_node_state)
    {
        case G8032_NODE_STATE_IDLE:
            break;
            
        case G8032_NODE_STATE_PROTECTION:
            g8032_fsm_stop_transmit_raps(p_g8032_ring);
            g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE, FALSE);

            g8032_fsm_add_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_GUARD_WHILE); 

            if (p_g8032_ring->rpl_owner && p_g8032_ring->mode_revertive)
            {
                  g8032_fsm_add_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE); 
            }

            g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PENDING);

             if (p_g8032_ring->east_port_is_fail  || p_g8032_ring->west_port_is_fail )
            {
                g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE); 
                if (p_g8032_ring->east_port_is_fail )
                {
                    G8032FSM[p_g8032_ring->current_node_state][G8032_PRI_REQ_L_SF].func(p_g8032_ring, NULL, G8032_PRI_REQ_L_SF, TRUE);
                }

                if (p_g8032_ring->west_port_is_fail )
                {
                    G8032FSM[p_g8032_ring->current_node_state][G8032_PRI_REQ_L_SF].func(p_g8032_ring, NULL, G8032_PRI_REQ_L_SF, FALSE);
                }
            }
            break;  
        case G8032_NODE_STATE_MANUAL_SWITCH:
            break;
        case G8032_NODE_STATE_FORCE_SWITCH:
            break;
        case G8032_NODE_STATE_PENDING:
            break;
     }
    return PM_E_NONE;
}


int32
g8032_fsm_init (tbl_g8032_ring_t *p_g8032_ring)
{
    tbl_interface_t  *p_east_interface = NULL;
    tbl_interface_t  *p_west_interface = NULL;
     if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }

    
    g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE);
    g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTB_WHILE);
    g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_GUARD_WHILE);

    if (p_g8032_ring->rpl_owner)
    {
         if (p_g8032_ring->east_port_is_rpl)
         {
                g8032_fsm_block_port(p_g8032_ring, TRUE);
                if (p_g8032_ring->west_port_ifindex)
                {
                    g8032_fsm_unblock_port(p_g8032_ring, FALSE);
                }
         }
         else if (p_g8032_ring->west_port_is_rpl)
         {
                g8032_fsm_unblock_port(p_g8032_ring, TRUE);
                g8032_fsm_block_port(p_g8032_ring, FALSE);
                
         }

        g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE, FALSE);
        
        if (p_g8032_ring->mode_revertive)
        {
              g8032_fsm_add_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE); 
        }
        
        g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PENDING);
     
    }
    else if (p_g8032_ring->rpl_neighbor)
    {
         if (p_g8032_ring->east_port_is_rpl_neighbor)
         {
            g8032_fsm_block_port(p_g8032_ring, TRUE); 
            if (p_g8032_ring->west_port_ifindex)
            {
                g8032_fsm_unblock_port(p_g8032_ring, FALSE);
            }
         }
         else if (p_g8032_ring->west_port_is_rpl_neighbor)
         {
              g8032_fsm_block_port(p_g8032_ring, FALSE);
              g8032_fsm_unblock_port(p_g8032_ring, TRUE);
         }

        g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE, FALSE);

        g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PENDING);
     
    }
    else
    {
        g8032_fsm_block_port(p_g8032_ring, TRUE);

        if (p_g8032_ring->west_port_ifindex)
        {
            g8032_fsm_unblock_port(p_g8032_ring, FALSE);
        }
        g8032_ring_pkt_tx_raps(p_g8032_ring, TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE, FALSE);

        g8032_fsm_state_switch(p_g8032_ring, G8032_NODE_STATE_PENDING);
         
    }
    p_east_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
    if (p_east_interface)
    {   
        /* system modified by liwh for bug 53010, 2019-08-08 */
        /*if (!tbl_interface_is_running(p_east_interface))*/
        if (!g8032_ring_check_cfm_and_interface_state(p_east_interface, p_g8032_ring))
        /* liwh end */
        {
            p_g8032_ring->east_port_is_fail = TRUE;
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_FAIL);
            g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE); 
 
            G8032FSM[p_g8032_ring->current_node_state][G8032_PRI_REQ_L_SF].func(p_g8032_ring, NULL, G8032_PRI_REQ_L_SF, TRUE);

        }
        else
        {
            p_g8032_ring->east_port_is_fail = FALSE;
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_IS_FAIL);
        }
    }
    p_west_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
    if (p_west_interface)
    {
        /* system modified by liwh for bug 53010, 2019-08-08 */
        /*if (!tbl_interface_is_running(p_west_interface))*/
        if (!g8032_ring_check_cfm_and_interface_state(p_west_interface, p_g8032_ring))
        /* liwh end */
        {
            p_g8032_ring->west_port_is_fail = TRUE;
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_FAIL);
            g8032_fsm_stop_timer_with_type(p_g8032_ring, TBL_G8032_RING_FLD_P_WTR_WHILE); 
           
            G8032FSM[p_g8032_ring->current_node_state][G8032_PRI_REQ_L_SF].func(p_g8032_ring, NULL, G8032_PRI_REQ_L_SF, FALSE);
        }
        else
        {
            p_g8032_ring->west_port_is_fail = FALSE;
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_IS_FAIL);
        }
    }
    
    return PM_E_NONE;
}

