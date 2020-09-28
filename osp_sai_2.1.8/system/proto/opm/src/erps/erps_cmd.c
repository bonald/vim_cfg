
#include "proto.h"

#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_erps_domain_define.h"
#include "gen/tbl_erps_domain.h"
#include "gen/tbl_erps_ring_define.h"
#include "gen/tbl_erps_ring.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"
#include "erps_define.h"
#include "erps.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_opm_global_define.h"
#include "gen/tbl_opm_global.h"
#include "lib_fs.h"

extern int32
erps_set_erps_domain_instance_num(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain);

int32
erps_fdb_set_flush_fdb(ds_flush_fdb_t *p_flush_fdb)
{
    tbl_l2_action_t   *p_l2_action = NULL; 
    ds_flush_fdb_t    *p_db_flush_fdb = NULL;
    int32             rc = PM_E_NONE;
    
    p_l2_action = tbl_l2_action_get_l2_action();
    p_db_flush_fdb = ds_flush_fdb_get_flush_fdb(p_l2_action);

    if (NULL == p_db_flush_fdb)
    {
        rc = ds_flush_fdb_add_flush_fdb(p_l2_action, p_flush_fdb);
    }
    rc = ds_flush_fdb_set_flush_fdb_field(p_l2_action, p_flush_fdb, DS_FLUSH_FDB_FLD_MAX);

    return rc;
}

int32
erps_fdb_flush_dynamic_fdb_port(uint32 ifindex)
{
    ds_flush_fdb_t flush_fdb;

    sal_memset(&flush_fdb, 0, sizeof(flush_fdb));
    flush_fdb.type = FLUSH_FDB_TYPE_FDB;
    flush_fdb.subtype = FLUSH_FDB_SUBTYPE_DYNAMIC;
    flush_fdb.mode = FLUSH_FDB_MODE_PORT;
    flush_fdb.ifindex = ifindex;
    return erps_fdb_set_flush_fdb(&flush_fdb);
}

int32
erps_fdb_flush_dynamic_fdb_all(void)
{
    ds_flush_fdb_t flush_fdb;

    sal_memset(&flush_fdb, 0, sizeof(flush_fdb));
    flush_fdb.type = FLUSH_FDB_TYPE_FDB;
    flush_fdb.subtype = FLUSH_FDB_SUBTYPE_DYNAMIC;
    flush_fdb.mode = FLUSH_FDB_MODE_ERPS;
    return erps_fdb_set_flush_fdb(&flush_fdb);
}

static bool
_erps_domain_check_two_domain_share_one_port(tbl_erps_domain_t *p_domain1, tbl_erps_domain_t *p_domain2)
{
    tbl_erps_ring_t            *p_erps_ring1 = NULL;
    ctclib_slistnode_t         *listnode1 = NULL;  
    tbl_erps_ring_t            *p_erps_ring2 = NULL;
    ctclib_slistnode_t         *listnode2 = NULL; 

    CTCLIB_SLIST_LOOP(p_domain1->erps_ring_list.obj_list, p_erps_ring1, listnode1)
    {
        CTCLIB_SLIST_LOOP(p_domain2->erps_ring_list.obj_list, p_erps_ring2, listnode2)
        {
            if ((p_erps_ring1->primary_port_if == p_erps_ring2->primary_port_if)
                || (p_erps_ring1->primary_port_if == p_erps_ring2->secondary_port_if)
                || (p_erps_ring1->secondary_port_if == p_erps_ring2->primary_port_if)
                || (p_erps_ring1->secondary_port_if == p_erps_ring2->secondary_port_if))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

static bool
_erps_domain_check_used_by_other_domain_with_same_instance(tbl_erps_ring_t *p_db_ring, void  *port_if)
{
    tbl_erps_domain_master_t   *p_master = NULL;
    uint32                     domain_id = 0;
    tbl_erps_domain_t          *p_db_domain = NULL;
    tbl_erps_domain_t          *pst_domain = NULL;
    tbl_erps_ring_t            *p_erps_ring = NULL;
    ctclib_slistnode_t         *listnode = NULL;
    uint32                     index1 = 0;
    uint32                     index2 = 0;

    p_master = tbl_erps_domain_get_master();
    pst_domain = p_master->domain_array[p_db_ring->key.domain_id];
    if (NULL == pst_domain)
    {
        return FALSE;
    }

    if (0 == pst_domain->instance_id_num)
    {
        return FALSE;    
    }

    for (domain_id = 1; domain_id < GLB_MAX_ERPS_DOMAIN; domain_id++)
    {
        if (domain_id == p_db_ring->key.domain_id)
        {
            continue;
        }

        p_db_domain = p_master->domain_array[domain_id];
        if (NULL == p_db_domain)
        {
            continue;
        }

        if (0 == p_db_domain->instance_id_num)
        {
            continue;    
        }

        for (index1 = 0; index1 < pst_domain->instance_id_num; index1++)
        {
            for (index2 = 0; index2 < p_db_domain->instance_id_num; index2++)
            {
                if (p_db_domain->instance_id[index2] != pst_domain->instance_id[index1])
                {
                    continue;
                }
        
                CTCLIB_SLIST_LOOP(p_db_domain->erps_ring_list.obj_list, p_erps_ring, listnode)
                {
                    if ((p_erps_ring->primary_port_if == port_if)
                        || (p_erps_ring->secondary_port_if == port_if))
                    {
                        return TRUE;
                    }
                }
            }
        }
    }

    return FALSE;
}

static bool
_erps_port_check_used_by_this_domain(tbl_erps_domain_t *p_db_domain, void  *port_if)
{   
    tbl_erps_ring_t           *p_erps_ring = NULL;
    ctclib_slistnode_t        *listnode = NULL;  
    
    CTCLIB_SLIST_LOOP(p_db_domain->erps_ring_list.obj_list, p_erps_ring, listnode)
    {  
        if (ERPS_RING_PRIMARY == p_erps_ring->erps_ring_level)
        {
            continue;
        }

        if ((p_erps_ring->primary_port_if == port_if)
            || (p_erps_ring->secondary_port_if == port_if))
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

int32
_erps_port_reset_pkt_mode(tbl_interface_t* p_if, tbl_erps_ring_t *p_db_ring)
{
    int32                     rc = PM_E_NONE;
    tbl_erps_ring_master_t    *p_master = tbl_erps_ring_get_master();
    tbl_erps_ring_t           *p_erps_ring = NULL;
    ctclib_slistnode_t        *listnode = NULL; 
    tbl_interface_t           port;
    ds_lag_t                  *p_lag = NULL;
    tbl_interface_t           *p_if_mem = NULL;
    int32                     memrc = PM_E_NONE;

    if (!p_if)
    {
        return rc;
    }

    sal_memset(&port, 0, sizeof(tbl_interface_t));
    port.erps_pdu_action = GLB_PDU_L2PDU_PORT_ACTION_FWD;

    CTCLIB_SLIST_LOOP(p_master->erps_ring_list, p_erps_ring, listnode)
    {
        if (((p_erps_ring->primary_port_if == p_if)
            || (p_erps_ring->secondary_port_if == p_if))
            && (p_erps_ring != p_db_ring))
        {
            if ((ERPS_NODE_MASTER == p_erps_ring->node_role)
                || (ERPS_NODE_EDGE_UNKNOWN != p_erps_ring->edge_node_role))
            {
                port.erps_pdu_action = GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU;
                break;
            }
        }
    }

    if (GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU != port.erps_pdu_action)
    {
        CTCLIB_SLIST_LOOP(p_master->erps_ring_list, p_erps_ring, listnode)
        {
            if (((p_erps_ring->primary_port_if == p_if)
                || (p_erps_ring->secondary_port_if == p_if))
                && (p_erps_ring != p_db_ring))
            {
                if (ERPS_NODE_TRANSIT == p_erps_ring->node_role)
                {
                    port.erps_pdu_action = GLB_PDU_L2PDU_PORT_ACTION_COPY_TO_CPU;
                    break;
                }
            }
        }
    }

    if (INTERFACE_IS_LAG_GROUP(p_if))
    {
        p_lag = p_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            sal_memcpy(port.key.name, p_if_mem->key.name, IFNAME_SIZE);  

            memrc = tbl_interface_set_interface_field(&port, TBL_INTERFACE_FLD_ERPS_PDU_ACTION);
            if (memrc)
            {
                ERPS_LOG_ERR("tbl_interface_set_interface_field set erps pdu action fail");
                return memrc;
            }
        }
    }

    sal_memcpy(port.key.name, p_if->key.name, IFNAME_SIZE);  
    
    rc = tbl_interface_set_interface_field(&port, TBL_INTERFACE_FLD_ERPS_PDU_ACTION);
    if (rc < 0)
    {
        ERPS_LOG_ERR("tbl_interface_set_interface_field set erps pdu action fail");
        return rc;
    }

    return rc;
}

int32
erps_port_lacp_add_member(tbl_interface_t* p_if, ds_lag_t* p_lagif)
{
    int32                   rc = PM_E_NONE;
    tbl_interface_t         *p_db_if_lag = NULL;
    tbl_interface_key_t     if_key;
    tbl_interface_t         port;
    tbl_erps_ring_master_t  *p_master = tbl_erps_ring_get_master();
    tbl_erps_ring_t         *p_erps_ring = NULL;
    ctclib_slistnode_t      *listnode = NULL;
    tbl_interface_t         *primary_port_if = NULL;
    tbl_interface_t         *secondary_port_if = NULL;
    bool                    blocked = FALSE;
    bool                    blocked_primary = FALSE;
    
    if (!p_if || !p_lagif)
    {
        return rc;
    }
    
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_lagif->lag_id);
    p_db_if_lag = tbl_interface_get_interface(&if_key);
    if (!p_db_if_lag)
    {
        return rc;
    }

    if (FALSE == p_db_if_lag->erps_enable)
    {
        return rc;
    }

    /* set pdu action */
    sal_memset(&port, 0, sizeof(tbl_interface_t));
    port.erps_pdu_action = p_db_if_lag->erps_pdu_action; 
    sal_memcpy(port.key.name, p_if->key.name, IFNAME_SIZE);  
    
    rc = tbl_interface_set_interface_field(&port, TBL_INTERFACE_FLD_ERPS_PDU_ACTION);
    if (rc < 0)
    {
        ERPS_LOG_ERR("tbl_interface_set_interface_field set erps pdu action fail");
        return rc;
    }

    /* set port blocking */ 
    CTCLIB_SLIST_LOOP(p_master->erps_ring_list, p_erps_ring, listnode)
    {
        primary_port_if = p_erps_ring->primary_port_if;
        secondary_port_if = p_erps_ring->secondary_port_if;

        if ((primary_port_if && (primary_port_if->ifindex == p_db_if_lag->ifindex))
            || (secondary_port_if && (secondary_port_if->ifindex == p_db_if_lag->ifindex)))
        {
            blocked = p_erps_ring->blocked;

            if (blocked && (p_erps_ring->primary_port_if == p_erps_ring->blocked_port_if))
            {
                blocked_primary = TRUE;   
            }
            
            p_erps_ring->blocked_port_if = p_if; 
            p_erps_ring->blocked_port_ifindex = p_if->ifindex;
            erps_set_erps_ring_blocked_interface(p_erps_ring, p_erps_ring, TRUE);

            p_erps_ring->blocked = blocked;
            if (blocked)
            {
                if (blocked_primary)
                {
                    p_erps_ring->blocked_port_if = p_erps_ring->primary_port_if; 
                    p_erps_ring->blocked_port_ifindex = primary_port_if->ifindex;
                }
                else
                {
                    p_erps_ring->blocked_port_if = p_erps_ring->secondary_port_if; 
                    p_erps_ring->blocked_port_ifindex = secondary_port_if->ifindex;              
                }
            }
            else
            {
                p_erps_ring->blocked_port_if = NULL;
                p_erps_ring->blocked_port_ifindex = 0;
            }            
        }
    }

    return rc;
}

int32
erps_port_lacp_delete_member(tbl_interface_t* p_if, ds_lag_t* p_lagif)
{
    int32                   rc = PM_E_NONE;
    tbl_interface_t         *p_db_if_lag = NULL;
    tbl_interface_key_t     if_key;
    tbl_interface_t         port;
    tbl_erps_ring_master_t  *p_master = tbl_erps_ring_get_master();
    tbl_erps_ring_t         *p_erps_ring = NULL;
    ctclib_slistnode_t      *listnode = NULL;
    tbl_interface_t         *primary_port_if = NULL;
    tbl_interface_t         *secondary_port_if = NULL;
    bool                    blocked = FALSE;
    bool                    blocked_primary = FALSE;
    
    if (!p_if || !p_lagif)
    {
        return rc;
    }
    
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_lagif->lag_id);
    p_db_if_lag = tbl_interface_get_interface(&if_key);
    if (!p_db_if_lag)
    {
        return rc;
    }

    if (FALSE == p_db_if_lag->erps_enable)
    {
        return rc;
    }

    /* set pdu action */
    sal_memset(&port, 0, sizeof(tbl_interface_t));
    port.erps_pdu_action = GLB_PDU_L2PDU_PORT_ACTION_FWD; 
    sal_memcpy(port.key.name, p_if->key.name, IFNAME_SIZE);  
    
    rc = tbl_interface_set_interface_field(&port, TBL_INTERFACE_FLD_ERPS_PDU_ACTION);
    if (rc < 0)
    {
        ERPS_LOG_ERR("tbl_interface_set_interface_field set erps pdu action fail");
        return rc;
    }

    /* set port unblocking */ 
    CTCLIB_SLIST_LOOP(p_master->erps_ring_list, p_erps_ring, listnode)
    {
        primary_port_if = p_erps_ring->primary_port_if;
        secondary_port_if = p_erps_ring->secondary_port_if;

        if ((primary_port_if && (primary_port_if->ifindex == p_db_if_lag->ifindex))
            || (secondary_port_if && (secondary_port_if->ifindex == p_db_if_lag->ifindex)))
        {
            blocked = p_erps_ring->blocked;

            if (blocked && (p_erps_ring->primary_port_if == p_erps_ring->blocked_port_if))
            {
                blocked_primary = TRUE;   
            }
            
            p_erps_ring->blocked_port_if = p_if; 
            p_erps_ring->blocked_port_ifindex = p_if->ifindex;
            erps_set_erps_ring_blocked_interface(p_erps_ring, p_erps_ring, FALSE);

            p_erps_ring->blocked = blocked;
            if (blocked)
            {
                if (blocked_primary)
                {
                    p_erps_ring->blocked_port_if = p_erps_ring->primary_port_if; 
                    p_erps_ring->blocked_port_ifindex = primary_port_if->ifindex;
                }
                else
                {
                    p_erps_ring->blocked_port_if = p_erps_ring->secondary_port_if; 
                    p_erps_ring->blocked_port_ifindex = secondary_port_if->ifindex;              
                }
            }
            else
            {
                p_erps_ring->blocked_port_if = NULL;
                p_erps_ring->blocked_port_ifindex = 0;
            }            
        }
    }

    return rc;
}

int32
erps_port_lacp_attach_member(tbl_interface_t* p_if, ds_lag_t* p_lagif)
{
    int32                   rc = PM_E_NONE;
    tbl_interface_t         *p_db_if_lag = NULL;
    tbl_interface_key_t     if_key;
    tbl_erps_ring_master_t  *p_master = tbl_erps_ring_get_master();
    tbl_erps_ring_t         *p_erps_ring = NULL;
    ctclib_slistnode_t      *listnode = NULL;
    tbl_interface_t         *primary_port_if = NULL;
    tbl_interface_t         *secondary_port_if = NULL;
    bool                    blocked = FALSE;
    bool                    blocked_primary = FALSE;
    tbl_erps_domain_t       *p_db_domain = NULL;
    bool                    blocked_member = FALSE;
    
    if (!p_if || !p_lagif)
    {
        return rc;
    }
    
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_lagif->lag_id);
    p_db_if_lag = tbl_interface_get_interface(&if_key);
    if (!p_db_if_lag)
    {
        return rc;
    }

    if (FALSE == p_db_if_lag->erps_enable)
    {
        return rc;
    }

    /* set port unblocking */ 
    CTCLIB_SLIST_LOOP(p_master->erps_ring_list, p_erps_ring, listnode)
    {
        primary_port_if = p_erps_ring->primary_port_if;
        secondary_port_if = p_erps_ring->secondary_port_if;

        if ((primary_port_if && (primary_port_if->ifindex == p_db_if_lag->ifindex))
            || (secondary_port_if && (secondary_port_if->ifindex == p_db_if_lag->ifindex)))
        {
            blocked = p_erps_ring->blocked;

            if (blocked && (p_erps_ring->primary_port_if == p_erps_ring->blocked_port_if))
            {
                blocked_primary = TRUE;   
            }
            
            if (blocked && (p_erps_ring->blocked_port_ifindex == p_db_if_lag->ifindex))
            {
                blocked_member = TRUE;
            }
            else
            {
                blocked_member = FALSE;
            }
            
            p_erps_ring->blocked_port_if = p_if; 
            p_erps_ring->blocked_port_ifindex = p_if->ifindex;
            erps_set_erps_ring_blocked_interface(p_erps_ring, p_erps_ring, blocked_member);

            p_erps_ring->blocked = blocked;
            if (blocked)
            {
                p_db_domain = (tbl_erps_domain_t*)p_erps_ring->p_erps_domain;
                if (!p_db_domain)
                {
                    ERPS_LOG_ERR("NULL domain in erps_port_lacp_attach_member func");
                    continue;
                } 
                
                if (blocked_primary)
                {
                    p_erps_ring->blocked_port_if = p_erps_ring->primary_port_if; 
                    p_erps_ring->blocked_port_ifindex = primary_port_if->ifindex;
                }
                else
                {
                    p_erps_ring->blocked_port_if = p_erps_ring->secondary_port_if; 
                    p_erps_ring->blocked_port_ifindex = secondary_port_if->ifindex;              
                }

                /*if (p_erps_ring->pre_forwarding_timer)
                {
                    ctc_task_delete_timer(p_erps_ring->pre_forwarding_timer);
                    p_erps_ring->pre_forwarding_timer = NULL;
                }*/
                CTC_TASK_STOP_TIMER(p_erps_ring->pre_forwarding_timer);
    
                p_erps_ring->pre_forwarding_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_pre_forwarding_timer_fn, 
                            p_erps_ring, p_db_domain->pre_forwarding_timer_interval);
                        
                if (!p_erps_ring->pre_forwarding_timer)
                {
                    ERPS_LOG_ERR("Preforwarding timer set fail");
                }
            }
            else
            {
                p_erps_ring->blocked_port_if = NULL;
                p_erps_ring->blocked_port_ifindex = 0;
            }            
        }
    }

    return rc;
}

int32
erps_port_lacp_detach_member(tbl_interface_t* p_if, ds_lag_t* p_lagif)
{
    int32                   rc = PM_E_NONE;
    tbl_interface_t         *p_db_if_lag = NULL;
    tbl_interface_key_t     if_key;
    tbl_erps_ring_master_t  *p_master = tbl_erps_ring_get_master();
    tbl_erps_ring_t         *p_erps_ring = NULL;
    ctclib_slistnode_t      *listnode = NULL;
    tbl_interface_t         *primary_port_if = NULL;
    tbl_interface_t         *secondary_port_if = NULL;
    bool                    blocked = FALSE;
    bool                    blocked_primary = FALSE;
    
    if (!p_if || !p_lagif)
    {
        return rc;
    }
    
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_lagif->lag_id);
    p_db_if_lag = tbl_interface_get_interface(&if_key);
    if (!p_db_if_lag)
    {
        return rc;
    }

    if (FALSE == p_db_if_lag->erps_enable)
    {
        return rc;
    }

    /* set port unblocking */ 
    CTCLIB_SLIST_LOOP(p_master->erps_ring_list, p_erps_ring, listnode)
    {
        primary_port_if = p_erps_ring->primary_port_if;
        secondary_port_if = p_erps_ring->secondary_port_if;

        if ((primary_port_if && (primary_port_if->ifindex == p_db_if_lag->ifindex))
            || (secondary_port_if && (secondary_port_if->ifindex == p_db_if_lag->ifindex)))
        {
            blocked = p_erps_ring->blocked;

            if (blocked && (p_erps_ring->primary_port_if == p_erps_ring->blocked_port_if))
            {
                blocked_primary = TRUE;   
            }
            
            p_erps_ring->blocked_port_if = p_if; 
            p_erps_ring->blocked_port_ifindex = p_if->ifindex;
            erps_set_erps_ring_blocked_interface(p_erps_ring, p_erps_ring, TRUE);

            p_erps_ring->blocked = blocked;
            if (blocked)
            {
                if (blocked_primary)
                {
                    p_erps_ring->blocked_port_if = p_erps_ring->primary_port_if; 
                    p_erps_ring->blocked_port_ifindex = primary_port_if->ifindex;
                }
                else
                {
                    p_erps_ring->blocked_port_if = p_erps_ring->secondary_port_if; 
                    p_erps_ring->blocked_port_ifindex = secondary_port_if->ifindex;              
                }
            }
            else
            {
                p_erps_ring->blocked_port_if = NULL;
                p_erps_ring->blocked_port_ifindex = 0;
            }            
        }
    }

    return rc;
}


int32
erps_ring_update_port(tbl_erps_ring_t *p_db_ring, tbl_interface_t* p_old_if, tbl_interface_t* p_if)
{  
    tbl_interface_t          port;
    bool                     erps_enable = FALSE;
    tbl_erps_ring_master_t   *p_master = tbl_erps_ring_get_master();
    tbl_erps_ring_t          *p_erps_ring = NULL;
    ctclib_slistnode_t       *listnode = NULL;  
    int32                    rc = PM_E_NONE;

    if (p_old_if) 
    {
        CTCLIB_SLIST_LOOP(p_master->erps_ring_list, p_erps_ring, listnode)
        {
            if (((p_erps_ring->primary_port_if == p_old_if)
                || (p_erps_ring->secondary_port_if == p_old_if))
                && (p_erps_ring != p_db_ring))
            {
                erps_enable = TRUE;
                break;
            }
        }

        /* set interface erps enable to FALSE */
        if (!erps_enable)
        {        
            sal_memset(&port, 0, sizeof(tbl_interface_t));
            
            if (p_old_if->key.name)
            {
                sal_memcpy(port.key.name, p_old_if->key.name, IFNAME_SIZE);  
            }
            
            rc = tbl_interface_set_interface_field(&port, TBL_INTERFACE_FLD_ERPS_ENABLE);
            if (rc < 0)
            {
                ERPS_LOG_ERR("tbl_interface_set_interface_field set erps enable fail");
                return rc;
            }

            usleep(20);
        }

        _erps_port_reset_pkt_mode(p_old_if, p_db_ring);
    }   

    if (p_if)
    {  
        sal_memset(&port, 0, sizeof(tbl_interface_t));
        
        if (p_if->key.name)
        {
            sal_memcpy(port.key.name, p_if->key.name, IFNAME_SIZE);  
        }
        
        port.erps_enable = TRUE;
        
        rc = tbl_interface_set_interface_field(&port, TBL_INTERFACE_FLD_ERPS_ENABLE);
        if (rc < 0)
        {
            ERPS_LOG_ERR("tbl_interface_set_interface_field set erps enable fail");
            return rc;
        }

        usleep(20);
    
        _erps_port_reset_pkt_mode(p_if, NULL);
    }

    return PM_E_NONE;
}

char*
erps_desc_node_role(erps_node_role_t role)
{
    switch (role)
    {
    case ERPS_NODE_MASTER:
        return "master";
    case ERPS_NODE_TRANSIT:
        return "transit";
    default:
        return "unknown";
    }
}

char*
erps_desc_node_edge_role(erps_edge_node_role_t edge_node_role)
{
    switch (edge_node_role)
    {
    case ERPS_NODE_EDGE:
        return "edge";
    case ERPS_NODE_ASSI_EDGE: 
        return "assistant-edge";
    default:
        return "unknown";
    }
}

char*
erps_desc_node_state(erps_state_t state)
{
    switch (state)
    {
    case ERPS_IDLE:
        return "idle";
    case ERPS_COMPLETE:
        return "complete";
    case ERPS_FAILED:
        return "failed";
    case ERPS_LINK_UP:
        return "linkup";
    case ERPS_LINK_DOWN:
        return "linkdown";
    case ERPS_PREFORWORDING:
        return "preforwarding";
    default:
        return "init";
    }
}

int32 erps_check_domain_name(const char *name, int nLen, cfg_cmd_para_t *para)
{
    char *p = NULL;
    
    /* check the string length */
    if (sal_strlen(name) > nLen)
    {
        CFG_FUNC_ERR_RET("ERPS domain name length should not exceed 15");
    }
    
    /*check the first character*/    
    if(!sal_char_isalpha(*name))
    {
        CFG_FUNC_ERR_RET("The first character should be a-z or A-Z");
    }
    
    /* check the chars */
    p = (char *)(name + 1);
    while ('\0' != *p)
    {
        if ((sal_char_isdigit(*p)) 
            || (sal_char_isalpha(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p))
        {
            p++;
            continue;
        }
        
        CFG_FUNC_ERR_RET("ERPS domain name character should be \"0-9A-Za-z.-_\"");
    }

    /*check the last character*/
    p--;
    if(!(sal_char_isdigit(*p) || sal_char_isalpha(*p)))
    {
        CFG_FUNC_ERR_RET("The last character should be 0-9 or a-z or A-Z");
    }
    
    return PM_E_NONE;
}

static int32
_erps_cmd_add_erps_domain(uint16 domain_id, char* name, int32 argc, cfg_cmd_para_t *para)
{
    int32                   rc = PM_E_NONE;
    tbl_erps_domain_t       *p_db_domain = NULL;
    tbl_erps_domain_key_t   key; 

    key.domain_id = domain_id;
    
    p_db_domain = tbl_erps_domain_get_erps_domain(&key);
    if (p_db_domain)
    {
        if ((argc > 1) && sal_memcmp(p_db_domain->name, name, sal_strlen(p_db_domain->name)))
        {
            CFG_FUNC_ERR_RET("ERPS domain %d has been created", domain_id);
        }
        else
        {
            return rc;
        }
    }

    rc = erps_check_domain_name(name, (ERPS_MAX_NAME_LEN - 1), para);
    if (rc)
    {
        return rc;
    }

    if (erps_get_erps_domain_num() >= GLB_ERPS_MAX_DOMAIN)
    {
        CFG_FUNC_ERR_RET("Can't add ERPS domain: reach max number %d", GLB_ERPS_MAX_DOMAIN);
    }

    rc = erps_add_erps_domain(domain_id, name);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add erps domain fail, %s", pm_err_desc(rc));
    }
    
    return rc;
}

static int32
_erps_cmd_del_erps_domain(uint16 domain_id, cfg_cmd_para_t *para)
{
    int32                  rc = PM_E_NONE;
    tbl_erps_domain_t      *p_db_domain = NULL;
    tbl_erps_domain_key_t  key; 
    tbl_erps_ring_t        *p_erps_ring = NULL;
    ctclib_slistnode_t     *listnode1 = NULL;
    ctclib_slistnode_t     *listnode2 = NULL;
    uint16                 ring_id = 0;
    tbl_erps_domain_t      domain;

    sal_memset(&domain, 0, sizeof(tbl_erps_domain_t));
    
    key.domain_id = domain_id;
    p_db_domain = tbl_erps_domain_get_erps_domain(&key);
    if (!p_db_domain)
    {
        CFG_FUNC_ERR_RET("ERPS domain %d hasn't been created", domain_id);
    }

    CTCLIB_SLIST_LOOP_DEL(p_db_domain->erps_ring_list.obj_list, p_erps_ring, listnode1, listnode2)
    {
        ring_id = p_erps_ring->key.ring_id;
        rc = erps_cmd_del_erps_ring(p_db_domain, ring_id, para);        
        if (rc)
        {
            CFG_FUNC_ERR_RET("delete erps ring %d failure when delete domain", ring_id);    
        }
    }

    domain.key.domain_id = domain_id;
    rc = erps_set_erps_domain_primary_control_vlan(&domain, p_db_domain);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Del erps domain promary control vlan fail, %s", pm_err_desc(rc));
    }

    rc = erps_set_erps_domain_sub_control_vlan(&domain, p_db_domain);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Del erps domain sub control vlan fail, %s", pm_err_desc(rc));
    }

    rc = erps_del_erps_domain(domain_id);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Del erps domain fail, %s", pm_err_desc(rc));
    }
    return rc;
}

static int32 _erps_cmd_set_erps_domain_hello_time(tbl_erps_domain_t *p_domain,
    tbl_erps_domain_t *p_db_domain, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 ring_check = 0;

    /* modified by wangjj for erps bug 41817, 2016-12-03 */
    ring_check = erps_get_check_erps_domain_ring_running(p_db_domain);
    if (ring_check)
    {
        CFG_FUNC_ERR_RET("ERPS ring %d is running, can't change", ring_check);
    }

    if (GLB_ERPS_DOMAIN_HELLO_TIME_DEFAULT != p_domain->hello_timer_interval)
    {
        if ((3 * p_domain->hello_timer_interval) > p_db_domain->failure_timer_interval)
        {
            CFG_FUNC_ERR_RET("Fail timer interval should be at least 3 times of hello timer interval");
        }
    }
    
    rc = erps_set_erps_domain_hello_time(p_domain, p_db_domain);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set erps domain hello time fail, %s", pm_err_desc(rc));
    }
    
    return rc;
}

static int32 _erps_cmd_set_erps_domain_fail_time(tbl_erps_domain_t *p_domain, 
    tbl_erps_domain_t *p_db_domain, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 ring_check = 0;

    /* modified by wangjj for erps bug 41817, 2016-12-03 */
    ring_check = erps_get_check_erps_domain_ring_running(p_db_domain);
    if (ring_check)
    {
        CFG_FUNC_ERR_RET("ERPS ring %d is running, can't change.\n", ring_check);
    }

    if (GLB_ERPS_DOMAIN_FAILURE_TIME_DEFAULT != p_domain->failure_timer_interval)
    {
        if ((3 * p_db_domain->hello_timer_interval) > p_domain->failure_timer_interval)
        {
            CFG_FUNC_ERR_RET("Fail timer interval should be at least 3 times of hello timer interval");
        }
    }
    else
    {
        if (GLB_ERPS_DOMAIN_HELLO_TIME_DEFAULT != p_db_domain->hello_timer_interval)
        {
            CFG_FUNC_ERR_RET("Please reset hellotime first");
        }
    }

    rc = erps_set_erps_domain_fail_time(p_domain, p_db_domain);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set erps domain failure time fail, %s", pm_err_desc(rc));
    }
    
    return rc;
}

int32 erps_check_one_domain_instnace(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain,
    uint32 instance_id, bool add)
{
    tbl_erps_domain_master_t *p_master = NULL;
    uint32                   domain_id = 0;
    tbl_erps_domain_t        *p_other_domain = NULL;
    tbl_mstp_instance_key_t  mstp_instance_key;
    int32                    rc = PM_E_NONE;
    uint32                   index = 0;
    uint32                   instance_id_bak[ERPS_MAX_INSTANCE];
    uint32                   instance_num_bak = 0;
    
    if (add)
    {
        /* check instance exist */
        mstp_instance_key.instance = instance_id;
        if (!tbl_mstp_instance_get_mstp_instance(&mstp_instance_key))
        {
            //CFG_FUNC_ERR_RET("MSTP instance %d hasn't been created", mstp_instance_key.instance);
            return PM_E_ERPS_INSTANCE_NOT_EXIST;
        }
    
        /*make sure the instance is not used by other domain*/ 
        p_master = tbl_erps_domain_get_master();
        for (domain_id = 1; domain_id < GLB_MAX_ERPS_DOMAIN; domain_id++)
        {
            p_other_domain = p_master->domain_array[domain_id];
            if (NULL == p_other_domain)
            {
                continue;
            }
    
            if (p_other_domain->key.domain_id == p_db_domain->key.domain_id)
            {
                continue;    
            }

            for (index = 0; index < p_other_domain->instance_id_num; index++)
            {
                if (p_other_domain->instance_id[index] == instance_id)
                {
                    /* check two domain share same port */
                    if (_erps_domain_check_two_domain_share_one_port(p_other_domain, p_db_domain))
                    {
                        //CFG_FUNC_ERR_RET("This instance is used by other domain, which shares the same interface");
                        return PM_E_ERPS_INSTANCE_USED_BY_OTHER_DOMAIN;
                    }
                }
            }
        }

        /* reserve erps instance */
        if (0 == p_domain->instance_id_num)
        {
            p_domain->instance_id[0] = instance_id;
            p_domain->instance_id_num++;
        }
        else
        {
            for (index = 0; index < p_domain->instance_id_num; index++)
            {
                if (p_domain->instance_id[index] < instance_id)
                {
                    continue;
                }
                else if (p_domain->instance_id[index] == instance_id)
                {
                    break;
                }
                else
                {
                    if (p_domain->instance_id_num > index)
                    {
                        instance_num_bak = p_domain->instance_id_num - index;
                        sal_memcpy(instance_id_bak, &p_domain->instance_id[index], 
                            sizeof(uint32) * instance_num_bak); 
                    }
    
                    p_domain->instance_id[index] = instance_id;
                    p_domain->instance_id_num++;
    
                    if (instance_num_bak)
                    {
                        sal_memcpy(&p_domain->instance_id[index+1], instance_id_bak, 
                            sizeof(uint32) * instance_num_bak);
                    }
                    
                    break;
                }
            }

            if (index == p_domain->instance_id_num)
            {
                p_domain->instance_id[index] = instance_id;
                p_domain->instance_id_num++;    
            }
        }
    }
    else
    {
        if (GLB_ERPS_DOMAIN_INSTANCE_ID_DEFAULT == instance_id)
        {
            sal_memset(p_domain->instance_id, 0, sizeof(uint32) * ERPS_MAX_INSTANCE);
            p_domain->instance_id_num = 0;
        }
        else
        {
            for (index = 0; index < p_domain->instance_id_num; index++)
            {
                if (p_domain->instance_id[index] != instance_id)
                {
                    continue;
                }

                if (p_domain->instance_id_num == (index+1))
                {
                    p_domain->instance_id[index] = 0;
                    p_domain->instance_id_num--;
                }
                else
                {
                    instance_num_bak = p_domain->instance_id_num - index - 1;
                    sal_memcpy(instance_id_bak, &p_domain->instance_id[index + 1], 
                        sizeof(uint32) * instance_num_bak); 
                
                    sal_memcpy(&p_domain->instance_id[index], instance_id_bak, 
                        sizeof(uint32) * instance_num_bak);
                    p_domain->instance_id[p_domain->instance_id_num - 1] = 0;
                    p_domain->instance_id_num--;
                }
                    
                break;
            }
        }
    }

    return rc;
}

static int32 _erps_cmd_set_erps_domain_instnace(tbl_erps_domain_t *p_domain, 
    tbl_erps_domain_t *p_db_domain, cfg_cmd_para_t *para, char* inst_str, bool add)
{
    int32                    rc = PM_E_NONE;
    tbl_erps_ring_t          *p_erps_ring = NULL;
    ctclib_slistnode_t       *listnode = NULL;
    char                     **ppTokArr = NULL;
    unsigned long            nToks = 0;  
    uint32                   instanceStart = 0;
    uint32                   instanceEnd = 0;
    uint32                   idx = 0;
    char                     **ppInstanceArr = NULL;
    unsigned long            nInstanceToks = 0;
    uint32                   nInstanceIdx = 0;
    char                     *p = NULL;
    uint32                   nCnt = 0;

    p_domain->instance_id_num = p_db_domain->instance_id_num;
    sal_memcpy(p_domain->instance_id, p_db_domain->instance_id, sizeof(uint32)*p_db_domain->instance_id_num);
      
    if (sal_strspn(inst_str, "0123456789-,") != sal_strlen(inst_str))
    {
        CFG_FUNC_ERR_RET("Invalid characters in the instance parameter\n");
    }

    /* get instance segments */
    if (split_tokens(inst_str, sal_strlen(inst_str), 16, ",", &nToks, &ppTokArr) != 0)
    {
        CFG_FUNC_ERR_RET("Invalid instance parameter\n");
    }
    
    CTCLIB_SLIST_LOOP(p_db_domain->erps_ring_list.obj_list, p_erps_ring, listnode)
    {
        if (ERPS_IDLE != p_erps_ring->erps_ring_state)
        {
            if (NULL != ppTokArr)
            {
                free_tokens(&ppTokArr);
            }
            CFG_FUNC_ERR_RET("ERPS domain is running, can not change ERPS configuration");
        }
    }

    
    for (idx = 0; idx < nToks; idx++)
    {
        ppInstanceArr = NULL;
        nInstanceToks = 0;
        nInstanceIdx = 0;
        p = NULL;
        nCnt = 0;
        
        /*1. check whether the instance or instance range is valid*/
        p = ppTokArr[idx];
        while (*p != '\0')
        {
            if (*p == '-')
            {
                if (nCnt != 0)
                {
                    if (NULL != ppTokArr)
                    {
                        free_tokens(&ppTokArr);
                    }
                    CFG_FUNC_ERR_RET("Invalid Instance range: %s\n", ppTokArr[idx]);
                }
                ++nCnt;
            }
            ++p;
        }

        /*2. process single Instance*/
        if (sal_strchr(ppTokArr[idx], '-') == NULL)
        {
             instanceStart = sal_atoi(ppTokArr[idx]);
             if (instanceStart < 0 || ((instanceStart > ERPS_MAX_INSTANCE_NO)
                && (instanceStart != GLB_ERPS_DOMAIN_INSTANCE_ID_DEFAULT)))
             {
                  if (NULL != ppTokArr)
                  {
                      free_tokens(&ppTokArr);
                  }
                  CFG_FUNC_ERR_RET("Invalid instance number: %d\n", instanceStart);
             }

             rc = erps_check_one_domain_instnace(p_domain, p_db_domain, instanceStart, add);
             if (rc)
             {
                 if (NULL != ppTokArr)
                 {
                     free_tokens(&ppTokArr);
                 }

                 if (PM_E_ERPS_INSTANCE_NOT_EXIST == rc)
                 {
                     CFG_FUNC_ERR_RET("MSTP instance %d hasn't been created", instanceStart);
                 }
                 else if (PM_E_ERPS_INSTANCE_USED_BY_OTHER_DOMAIN == rc)
                 {
                    CFG_FUNC_ERR_RET("This instance %d is used by other domain, which shares the same interface", instanceStart);
                 }
                 
                 return rc;   
             }
              
             continue;
        }

        /*3. process instance range */
        if (split_tokens(ppTokArr[idx], sal_strlen(ppTokArr[idx]), 
                         2, "-", &nInstanceToks, &ppInstanceArr) == 0 && 2 == nInstanceToks)
        {
            instanceStart = sal_atoi(ppInstanceArr[0]);
            instanceEnd = sal_atoi(ppInstanceArr[1]);
            free_tokens(&ppInstanceArr);
            //for bug 38302
            if (instanceStart< 0 || instanceStart> ERPS_MAX_INSTANCE_NO
                ||  instanceEnd < 1 ||  instanceEnd > ERPS_MAX_INSTANCE_NO
                || instanceStart >=  instanceEnd)
            {
                if (NULL != ppTokArr)
                {
                    free_tokens(&ppTokArr);
                }
                CFG_FUNC_ERR_RET("Invalid instance range: %d-%d", instanceStart, instanceEnd);
            }
          
            for (nInstanceIdx = instanceStart; nInstanceIdx <= instanceEnd; nInstanceIdx++)
            {
                rc = erps_check_one_domain_instnace(p_domain, p_db_domain, nInstanceIdx, add);
                if (rc)
                {
                    if (NULL != ppTokArr)
                    {
                        free_tokens(&ppTokArr);
                    }

                    if (PM_E_ERPS_INSTANCE_NOT_EXIST == rc)
                    {
                        CFG_FUNC_ERR_RET("MSTP instance %d hasn't been created", nInstanceIdx);
                    }
                    else if (PM_E_ERPS_INSTANCE_USED_BY_OTHER_DOMAIN == rc)
                    {
                        CFG_FUNC_ERR_RET("This instance %d is used by other domain, which shares the same interface", 
                            nInstanceIdx);
                    }
                    
                    return rc;   
                }                 
            }
        }
        else
        {
            if (NULL != ppTokArr)
            {
                free_tokens(&ppTokArr);
            }
            
            if (NULL != ppInstanceArr)
            {
                free_tokens(&ppInstanceArr);
            }
            CFG_FUNC_ERR_RET("Invalid instance range\n");
        }
    }

    rc = erps_set_erps_domain_instance(p_domain, p_db_domain);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set erps domain instance fail, %s", pm_err_desc(rc));
    }

    rc = erps_set_erps_domain_instance_num(p_domain, p_db_domain);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set erps domain instance num fail, %s", pm_err_desc(rc));
    }
    
    return rc;
}

static int32 _erps_cmd_set_erps_domain_enable(tbl_erps_domain_t *p_domain, 
    tbl_erps_domain_t *p_db_domain, cfg_cmd_para_t *para)
{
    int32                   rc = PM_E_NONE;
    tbl_erps_ring_t         *p_erps_ring = NULL;
    ctclib_slistnode_t      *listnode = NULL;
    tbl_erps_ring_t         erps_ring;
    tbl_erps_ring_t         erps_ring_update;

    sal_memset(&erps_ring, 0, sizeof(tbl_erps_ring_t));
    if (p_domain->erps_enable)
    {
        erps_ring.erps_ring_state = ERPS_INIT;
    }
    else
    {
        erps_ring.erps_ring_state = ERPS_IDLE;
    }
    
    CTCLIB_SLIST_LOOP(p_db_domain->erps_ring_list.obj_list, p_erps_ring, listnode)
    {
        erps_ring.key.domain_id = p_erps_ring->key.domain_id;
        erps_ring.key.ring_id = p_erps_ring->key.ring_id;
        /* modified by wangjj for fix erps bug 41438, 2016-12-08 */
        sal_memset(&erps_ring_update, 0, sizeof(tbl_erps_ring_t));
        sal_memcpy(&erps_ring_update, &erps_ring, sizeof(tbl_erps_ring_t));
        rc = erps_cmd_set_erps_ring_enable(&erps_ring_update, p_erps_ring, para);
        if (rc)
        {
            return rc;
        }
    }

    rc = erps_set_erps_domain_enable(p_domain, p_db_domain);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set erps domain enable fail, %s", pm_err_desc(rc));
    }
    
    return rc;
}

static int32 _erps_cmd_set_erps_domain_primary_control_vlan(tbl_erps_domain_t *p_domain, 
    tbl_erps_domain_t *p_db_domain, cfg_cmd_para_t *para)
{
    int32             rc = PM_E_NONE;
    tbl_vlan_t        *p_db_vlan = NULL;
    tbl_vlan_key_t    key;
    tbl_erps_ring_t   *p_ring = NULL;

    if (0 != p_domain->primary_control_vlan_id)
    {
        /* check primary control vlan can't equal sub control vlan */
        if (p_domain->primary_control_vlan_id == p_db_domain->sub_control_vlan_id)
        {
            CFG_FUNC_ERR_RET("Can not set the primary control vlan the same as sub control vlan");
        }

        /* check vlan 1 */
        if (1 == p_domain->primary_control_vlan_id)
        {
            CFG_FUNC_ERR_RET("Invalid primary control vlan_id %d", p_domain->primary_control_vlan_id);
        }

        /* check vlan exist */
        key.vid = p_domain->primary_control_vlan_id;  
        p_db_vlan = tbl_vlan_get_vlan(&key);
        if (!p_db_vlan)
        {
            CFG_FUNC_ERR_RET("Can't find VLAN %d", key.vid);
        }

        /* check vlan is already control vlan */
        /* modified by wangjj for fix erps bug 41056, 2016-11-11 */
        if (p_db_vlan->erps_domain_id && p_db_vlan->erps_domain_id != p_db_domain->key.domain_id)
        {
            CFG_FUNC_ERR_RET("This vlan is already set as control vlan of other domain");
        }

        /* modified by wangjj for fix erps bug 41473, 2016-12-03 */
        if (erps_get_erps_control_vlan_igsp_enable(p_domain->primary_control_vlan_id))
        {
            CFG_FUNC_ERR_RET("This VLAN has beed used by IGMP Snooping, please disable IGMP Snooping of this vlan");
        }
    }

    if (p_db_domain->erps_primary_ring)
    {
        p_ring = p_db_domain->erps_primary_ring;
        if (ERPS_IDLE != p_ring->erps_ring_state)
        {
            CFG_FUNC_ERR_RET("Can't reset the primary control vlan for ERPS domain %u, because ring %u is running",
                p_ring->key.domain_id, p_ring->key.ring_id);        
        }
    }

    rc = erps_set_erps_domain_primary_control_vlan(p_domain, p_db_domain);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set erps domain control vlan fail, %s", pm_err_desc(rc));
    }
    
    return rc;
}

static int32 _erps_cmd_set_erps_domain_sub_control_vlan(tbl_erps_domain_t *p_domain, 
    tbl_erps_domain_t *p_db_domain, cfg_cmd_para_t *para)
{
    int32                 rc = PM_E_NONE;
    tbl_vlan_t            *p_db_vlan = NULL;
    tbl_vlan_key_t        key;
    tbl_erps_ring_t       *p_erps_ring = NULL;
    ctclib_slistnode_t    *listnode = NULL;  

    if (0 != p_domain->sub_control_vlan_id)
    {
        /* check primary control vlan can't equal sub control vlan */
        if (p_domain->sub_control_vlan_id == p_db_domain->primary_control_vlan_id)
        {
            CFG_FUNC_ERR_RET("Can not set the sub control vlan the same as primary control vlan");
        }

        /* check vlan 1 */
        if (1 == p_domain->sub_control_vlan_id)
        {
            CFG_FUNC_ERR_RET("Invalid sub control vlan_id %d", p_domain->sub_control_vlan_id);
        }

        /* check vlan exist */     
        key.vid = p_domain->sub_control_vlan_id;  
        p_db_vlan = tbl_vlan_get_vlan(&key);
        if (!p_db_vlan)
        {
            CFG_FUNC_ERR_RET("Can't find VLAN %d", key.vid);
        }

        /* check vlan is already control vlan */
        /* modified by wangjj for fix erps bug 41056, 2016-11-11 */
        if (p_db_vlan->erps_domain_id && p_db_vlan->erps_domain_id != p_db_domain->key.domain_id)
        {
            CFG_FUNC_ERR_RET("This vlan is already set as control vlan of other domain");
        }

        /* modified by wangjj for fix erps bug 41473, 2016-12-03 */
        if (erps_get_erps_control_vlan_igsp_enable(p_domain->sub_control_vlan_id))
        {
            CFG_FUNC_ERR_RET("This VLAN has beed used by IGMP Snooping, please disable IGMP Snooping of this vlan");
        }
    }

    CTCLIB_SLIST_LOOP(p_db_domain->erps_ring_list.obj_list, p_erps_ring, listnode)
    {
        if ((ERPS_RING_SUB == p_erps_ring->erps_ring_level)
            && (ERPS_IDLE != p_erps_ring->erps_ring_state))
        {
            CFG_FUNC_ERR_RET("Can't reset the sub control vlan for ERPS domain %u, because ring %u is running",
                            p_erps_ring->key.domain_id, p_erps_ring->key.ring_id);   
        }
    }

    rc = erps_set_erps_domain_sub_control_vlan(p_domain, p_db_domain);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set erps domain control vlan fail, %s", pm_err_desc(rc));
    }
    
    return rc;
}

static int32
_erps_cmd_get_domain(uint32  domain_id, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                    *fp = NULL;
    tbl_erps_domain_key_t   key;
    tbl_erps_domain_t       *p_db_domain = NULL;
    tbl_iter_args_t         args;    

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (0 == domain_id)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_erps_domain_dump_one, &args);
    }
    else
    {
        key.domain_id = domain_id; 
        p_db_domain = tbl_erps_domain_get_erps_domain(&key);
        if (!p_db_domain)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("domain %u does not exist", domain_id);
        }

        tbl_erps_domain_dump_one(p_db_domain, &args);
    }
    
    sal_fclose(fp);
    return PM_E_NONE;
}

int32
erps_cmd_process_erps_domain(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t              *p_node = NULL;
    tbl_erps_domain_t       erps_domain;
    tbl_erps_domain_t       *p_db_domain = NULL;
    tbl_erps_domain_key_t   key;
    int32                   rc = PM_E_NONE;
    uint32                  domain_id = 0;
    char                    name[ERPS_MAX_NAME_LEN];
    int32                   i = 0;
    cdb_field_t             *p_field = NULL;
    int32                   field_id = 0;
    field_parse_t           field;
    bool                    add = FALSE;
    
    CFG_DBG_FUNC();
    
    p_node = cdb_get_tbl(TBL_ERPS_DOMAIN);
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_GET_UINT(domain_id, argv[0], para->p_rs);

        sal_memset(name, 0, ERPS_MAX_NAME_LEN);
        if (argc > 1)
        {
            /* modified by wangjj for fix erps bug 41047, 2016-11-10 */
            if (sal_strlen(argv[1]) > (ERPS_MAX_NAME_LEN - 1))
            {
                CFG_CONFLICT_RET("Erps domain name length should not exceed %u", ERPS_MAX_NAME_LEN - 1);
            }
            sal_strncpy(name, argv[1], ERPS_MAX_NAME_LEN);
        }
        else
        {
            sal_snprintf(name, ERPS_MAX_NAME_LEN, "ERPS%03d", domain_id);
        }
        rc = _erps_cmd_add_erps_domain(domain_id, name, argc, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_GET_UINT(domain_id, argv[0], para->p_rs);
        rc = _erps_cmd_del_erps_domain(domain_id, para);
        break;
    case CDB_OPER_SET:
        PM_CFG_GET_UINT(domain_id, argv[0], para->p_rs);

        i++;
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        
        key.domain_id = domain_id;        
        p_db_domain = tbl_erps_domain_get_erps_domain(&key);
        if (!p_db_domain)
        {
            return PM_E_NONE;
        }

        /* modified by wangjj for fix erps bug 41210, 2016-11-10 */
        sal_memset(&erps_domain, 0x0, sizeof(erps_domain));
        erps_domain.key.domain_id = domain_id;

        switch (field_id)
        {
        case TBL_ERPS_DOMAIN_FLD_PRIMARY_CONTROL_VLAN_ID:
            i++;
            PM_CFG_GET_UINT(erps_domain.primary_control_vlan_id, argv[i], para->p_rs);
            rc = _erps_cmd_set_erps_domain_primary_control_vlan(&erps_domain, p_db_domain, para);
            break;
        case TBL_ERPS_DOMAIN_FLD_SUB_CONTROL_VLAN_ID:
            i++;
            PM_CFG_GET_UINT(erps_domain.sub_control_vlan_id, argv[i], para->p_rs);
            rc = _erps_cmd_set_erps_domain_sub_control_vlan(&erps_domain, p_db_domain, para);
            break;
        case TBL_ERPS_DOMAIN_FLD_HELLO_TIMER_INTERVAL:
            i++;
            PM_CFG_GET_UINT(erps_domain.hello_timer_interval, argv[i], para->p_rs);
            rc = _erps_cmd_set_erps_domain_hello_time(&erps_domain, p_db_domain, para);
            break;
        case TBL_ERPS_DOMAIN_FLD_FAILURE_TIMER_INTERVAL:
            i++;
            PM_CFG_GET_UINT(erps_domain.failure_timer_interval, argv[i], para->p_rs);
            rc = _erps_cmd_set_erps_domain_fail_time(&erps_domain, p_db_domain, para);
            break;
        case TBL_ERPS_DOMAIN_FLD_INSTANCE_ID:
            i++;
            if ((argc > i + 1) && (0 == sal_memcmp(argv[i+1], "add", 3)))
            {
                add = TRUE;
            }
                
            //PM_CFG_GET_UINT(erps_domain.instance_id[0], argv[i], para->p_rs);
            rc = _erps_cmd_set_erps_domain_instnace(&erps_domain, p_db_domain, para, argv[i], add);
            break;
        case TBL_ERPS_DOMAIN_FLD_ERPS_ENABLE:
            i++;
            PM_CFG_GET_UINT(erps_domain.erps_enable, argv[i], para->p_rs);
            rc = _erps_cmd_set_erps_domain_enable(&erps_domain, p_db_domain, para);
            break;
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            PM_CFG_GET_UINT(domain_id, argv[0], para->p_rs);
        }
        
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        rc = _erps_cmd_get_domain(domain_id, para, p_node, &field);
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_erps_cmd_add_erps_ring(tbl_erps_domain_t *p_db_domain, uint32 ring_id, uint32 level, cfg_cmd_para_t *para)
{
    int32               rc = PM_E_NONE;
    uint32              erps_ring_num = 0;

    /* check ring level */
    switch (level)
    {
    case ERPS_RING_PRIMARY:
        if (p_db_domain->erps_primary_ring)
        {
            CFG_FUNC_ERR_RET("Only one primary ring is allowed to be configured per domain");
        }
        break;

    case ERPS_RING_SUB:
        if (p_db_domain->erps_ring_list.obj_list)
        {
            erps_ring_num = p_db_domain->erps_ring_list.obj_list->count;

            if (p_db_domain->erps_primary_ring)
            {
                erps_ring_num--;        
            }
        }
        
        if (erps_ring_num >= GLB_ERPS_MAX_SUB_RING_PER_DOMAIN)
        {
            CFG_FUNC_ERR_RET("Can't add ERPS ring: reach max sub ring numbers %d", GLB_ERPS_MAX_SUB_RING_PER_DOMAIN);
        }    
        break;

    default:
        CFG_FUNC_ERR_RET("Invalid ring Level");
    }

    rc = erps_add_erps_ring(p_db_domain, ring_id, level);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add erps domain fail, %s", pm_err_desc(rc));
    }
    return rc;
}

int32
erps_cmd_del_erps_ring(tbl_erps_domain_t *p_db_domain, uint32 ring_id, cfg_cmd_para_t *para)
{
    int32               rc = PM_E_NONE;
    tbl_erps_ring_t     erps_ring;
    tbl_erps_ring_t     *p_db_ring = NULL;

    sal_memset(&erps_ring, 0, sizeof(tbl_erps_ring_t));
    
    erps_ring.key.domain_id = p_db_domain->key.domain_id;
    erps_ring.key.ring_id = ring_id;
    
    p_db_ring = tbl_erps_ring_get_erps_ring(&erps_ring.key);
    if (!p_db_ring)
    {
        CFG_FUNC_ERR_RET("ERPS ring %u hasn't been created", ring_id);
    }

    erps_ring.erps_ring_state = ERPS_IDLE;
    rc = erps_cmd_set_erps_ring_enable(&erps_ring, p_db_ring, para);
    if (rc)
    {
        CFG_FUNC_ERR_RET("set ring disable failure when delete erps ring %d\n", ring_id);
    }
    
    if (p_db_ring->primary_port_if)
    {
        erps_ring_update_port(p_db_ring, p_db_ring->primary_port_if, NULL);
    }

    if (p_db_ring->secondary_port_if)
    {
        erps_ring_update_port(p_db_ring, p_db_ring->secondary_port_if, NULL);
    }

    rc = erps_del_erps_ring(p_db_domain, p_db_ring);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add erps domain fail, %s", pm_err_desc(rc));
    }
    return rc;
}

static int32 _erps_cmd_set_erps_ring_node_role(tbl_erps_ring_t *p_ring, 
    tbl_erps_ring_t *p_db_ring, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* check ring edge mode exist */
    if (ERPS_NODE_EDGE_UNKNOWN != p_db_ring->edge_node_role)
    {
        CFG_FUNC_ERR_RET("ERPS ring has already set to edge/assistant-edge node");
    }

    /* check ring running */
    if (ERPS_IDLE != p_db_ring->erps_ring_state)
    {
        CFG_FUNC_ERR_RET("ERPS ring is running, can not change ERPS configuration");
    }

    /* check node mode coherence when set mode */
    /* modified by wangjj for fix erps bug 41336, 2016-11-10 */
    //if (ERPS_NODE_EDGE_UNKNOWN != p_ring->edge_node_role)
    if (ERPS_NODE_UNKNOWN != p_ring->node_role)
    {
        switch (p_ring->node_role)
        {
        case ERPS_NODE_MASTER:
            /* check node role conflict */
            if (erps_ring_check_edge_node_role_mux(ERPS_NODE_EDGE, ERPS_NODE_CHECK_EDGE))
            {
                CFG_FUNC_ERR_RET("Can't set a node to master node when this node has been used as edge/assistant-edge mode");
            } 

            /* check node role conflict */
            if (erps_ring_check_edge_node_role_mux(ERPS_NODE_ASSI_EDGE, ERPS_NODE_CHECK_EDGE))
            {
                CFG_FUNC_ERR_RET("Can't set a node to master node when this node has been used as edge/assistant-edge mode");
            } 
            
            break;
        case ERPS_NODE_TRANSIT:
            /* no check */
            break;
        default:
            CFG_FUNC_ERR_RET("Please set the mode for ring %u first", p_ring->key.ring_id);
        }
    }
    
    rc = erps_set_erps_ring_node_role(p_ring, p_db_ring);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set erps ring node role fail, %s", pm_err_desc(rc));
    }

    _erps_port_reset_pkt_mode(p_db_ring->primary_port_if, NULL);
    _erps_port_reset_pkt_mode(p_db_ring->secondary_port_if, NULL);
    
    return rc;
}

static int32 _erps_cmd_set_erps_ring_edge_node_role(tbl_erps_ring_t *p_ring, 
    tbl_erps_ring_t *p_db_ring, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* check ring running */
    if (ERPS_IDLE != p_db_ring->erps_ring_state)
    {
        CFG_FUNC_ERR_RET("ERPS ring is running, can not change ERPS configuration");
    }

    /* check ring level is primary */
    if (ERPS_RING_PRIMARY == p_db_ring->erps_ring_level)
    {
        CFG_FUNC_ERR_RET("ERPS ring %u is a primary ring, can not set this node to edge/assistant-edge mode", 
            p_db_ring->key.ring_id);
    }

    /* check node role conflict */
    if (erps_ring_check_edge_node_role_mux(ERPS_NODE_MASTER, ERPS_NODE_CHECK_MASTER))
    {
        CFG_FUNC_ERR_RET("Can't set a node to edge/assistant-edge node when this node has been used as master mode");
    }

    rc = erps_set_erps_ring_edge_node_role(p_ring, p_db_ring);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set erps ring edge node role fail, %s", pm_err_desc(rc));
    }

    _erps_port_reset_pkt_mode(p_db_ring->primary_port_if, NULL);
    _erps_port_reset_pkt_mode(p_db_ring->secondary_port_if, NULL);
    
    return rc;
}

static int32 _erps_cmd_set_erps_ring_port(int32 field_id, char* ifname, tbl_erps_ring_t *p_ring, 
    tbl_erps_ring_t *p_db_ring, cfg_cmd_para_t *para)
{
    int32               rc = PM_E_NONE;
    tbl_interface_t     *p_if = NULL;
    tbl_interface_t     *p_old_if = NULL;
    tbl_mstp_port_t     *p_mstp_port = NULL;
    tbl_mstp_port_key_t  mstp_port_key;
    
    /* check erps is running */
    if (ERPS_IDLE != p_db_ring->erps_ring_state)
    {
        CFG_FUNC_ERR_RET("ERPS ring is running, can not change ERPS configuration");       
    }

    /* check ring edge mode exist */
    if (ERPS_NODE_EDGE_UNKNOWN != p_db_ring->edge_node_role)
    {
        CFG_FUNC_ERR_RET("ERPS ring has already set to edge/assistant-edge node");
    }
 
    /* check interface exist*/
    if (0 != sal_memcmp(ifname, "null", 4))
    {
        p_if = tbl_interface_get_interface_by_name(ifname);
        if (!p_if)
        {
            CFG_FUNC_ERR_RET("Interface has been set as aggregated member port or does not exist at all");
        }

        sal_memset(&mstp_port_key, 0, sizeof(tbl_mstp_port_key_t));
        sal_memcpy(mstp_port_key.name, p_if->key.name, sal_strlen(p_if->key.name));
        p_mstp_port = tbl_mstp_port_get_mstp_port(&mstp_port_key);
        if (p_mstp_port && p_mstp_port->port_enable)
        {
            CFG_FUNC_ERR_RET("STP has been enabled on this port, please disable stp on this port");
        }
        
        if (p_if->current_g8032_ring_count)
        {
            CFG_FUNC_ERR_RET("G8032 has been configured,  please disable G8032 on this port");
        }
        /* check interface is not switchport port */
        if (GLB_IF_MODE_L2 != p_if->mode)
        {
            CFG_FUNC_ERR_RET("Interface is not a switchport");
        }

        /* check interface is aggregregated */   
        if ((NULL != p_if->lag) && sal_memcmp(ifname, "agg", sal_strlen("agg")))
        {
            CFG_FUNC_ERR_RET("Can't set an aggregated member port as the ERPS interface");
        }
        
        /* check interface is not trunk and not qinq */
        if ((GLB_VLAN_PORT_TYPE_TRUNK != p_if->vlan_type)
            && (GLB_VLAN_PORT_TYPE_QINQ != p_if->vlan_type))
        {
            CFG_FUNC_ERR_RET("This interface is not a vlan trunk port or dot1q-tunnel port");    
        }

        if (_erps_domain_check_used_by_other_domain_with_same_instance(p_db_ring, (void*)p_if))
        {
            CFG_FUNC_ERR_RET("The interface has been used by other domain with the same instance");                
        }

        /* check whether there is other sub ring exist in this domain */
        if (ERPS_RING_SUB == p_db_ring->erps_ring_level)
        {
            if (_erps_port_check_used_by_this_domain((tbl_erps_domain_t*)p_db_ring->p_erps_domain, (void*)p_if))
            {
                CFG_FUNC_ERR_RET("The interface has been used by this domain's sub ring");
            }
        }
        
        switch (field_id)
        {
        case TBL_ERPS_RING_FLD_PRIMARY_PORT_IFINDEX:
            if (p_if == p_db_ring->secondary_port_if)
            {
                CFG_FUNC_ERR_RET("Can't set the same interface as primary interface");
            }
            p_old_if = p_db_ring->primary_port_if;
            p_db_ring->primary_port_if = p_if;

            rc = erps_ring_update_port(p_db_ring, p_old_if, p_if);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Set erps ring primary interface fail, %s", pm_err_desc(rc));
            }           
            
            p_ring->primary_port_ifindex = p_if->ifindex;
            rc = erps_set_erps_ring_primary_interface(p_ring, p_db_ring);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Set erps ring primary interface fail, %s", pm_err_desc(rc));
            }
            break;
    
        case TBL_ERPS_RING_FLD_SECONDARY_PORT_IFINDEX:
            if (p_if == p_db_ring->primary_port_if)
            {
                CFG_FUNC_ERR_RET("Can't set the same interface as secondary interface");
            }
            p_old_if = p_db_ring->secondary_port_if;
            p_db_ring->secondary_port_if = p_if;
            
            rc = erps_ring_update_port(p_db_ring, p_old_if, p_if);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Set erps ring secondary interface fail, %s", pm_err_desc(rc));
            } 
            
            p_ring->secondary_port_ifindex = p_if->ifindex;
            rc = erps_set_erps_ring_secondary_interface(p_ring, p_db_ring);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Set erps ring secondary interface fail, %s", pm_err_desc(rc));
            }
            break;
    
        default:
            CFG_FUNC_ERR_RET("Invalid ERPS interface type");
        }      
    }
    else
    {
        switch (field_id)
        {
        case TBL_ERPS_RING_FLD_PRIMARY_PORT_IFINDEX:
            p_old_if = p_db_ring->primary_port_if;
            p_db_ring->primary_port_if = NULL;
            
            rc = erps_ring_update_port(p_db_ring, p_old_if, NULL);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Set erps ring primary interface fail, %s", pm_err_desc(rc));
            }
            
            p_ring->primary_port_ifindex = 0;
            rc = erps_set_erps_ring_primary_interface(p_ring, p_db_ring);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Set erps ring primary interface fail, %s", pm_err_desc(rc));
            }
            break;
    
        case TBL_ERPS_RING_FLD_SECONDARY_PORT_IFINDEX:
            p_old_if = p_db_ring->secondary_port_if;
            p_db_ring->secondary_port_if = NULL;
            
            rc = erps_ring_update_port(p_db_ring, p_old_if, NULL);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Set erps ring secondary interface fail, %s", pm_err_desc(rc));
            }

            p_ring->secondary_port_ifindex = 0;
            rc = erps_set_erps_ring_secondary_interface(p_ring, p_db_ring);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Set erps ring secondary interface fail, %s", pm_err_desc(rc));
            }
            break;
    
        default:
            CFG_FUNC_ERR_RET("Invalid ERPS interface type");
        }   
    }       
    
    return rc;
}

static int32 _erps_cmd_set_erps_ring_edge_port(int32 field_id, char* ifname, tbl_erps_ring_t *p_ring, 
    tbl_erps_ring_t *p_db_ring, cfg_cmd_para_t *para)
{
    int32              rc = PM_E_NONE;
    tbl_interface_t    *p_if = NULL;
    tbl_interface_t    *p_old_if = NULL;
    tbl_mstp_port_t    *p_mstp_port = NULL;
    tbl_mstp_port_key_t  mstp_port_key;
    
    /* check erps is running */
    if (ERPS_IDLE != p_db_ring->erps_ring_state)
    {
        CFG_FUNC_ERR_RET("ERPS ring is running, can not change ERPS configuration");       
    }

    /* check ring edge mode exist */
    if (ERPS_NODE_EDGE_UNKNOWN == p_db_ring->edge_node_role)
    {
        CFG_FUNC_ERR_RET("This command is only applicable to edge/assistant-edge node");
    }
 
    /* check interface exist*/
    if (0 != sal_memcmp(ifname, "null", 4))
    {
        p_if = tbl_interface_get_interface_by_name(ifname);
        if (!p_if)
        {
            CFG_FUNC_ERR_RET("Interface has been set as aggregated member port or does not exist at all");
        }

        sal_memset(&mstp_port_key, 0, sizeof(tbl_mstp_port_key_t));
        sal_memcpy(mstp_port_key.name, p_if->key.name, sal_strlen(p_if->key.name));
        p_mstp_port = tbl_mstp_port_get_mstp_port(&mstp_port_key);  
        if (p_mstp_port && p_mstp_port->port_enable)
        {
            CFG_FUNC_ERR_RET("STP has been enabled on this port, please disable stp on this port");
        }

        /* check interface is not switchport port */
        if (GLB_IF_MODE_L2 != p_if->mode)
        {
            CFG_FUNC_ERR_RET("Interface is not a switchport");
        }

        /* check interface is aggregregated */   
        if ((NULL != p_if->lag) && sal_memcmp(ifname, "agg", sal_strlen("agg")))
        {
            CFG_FUNC_ERR_RET("Can't set an aggregated member port as the ERPS interface");
        }

        if (p_if->current_g8032_ring_count)
        {
            CFG_FUNC_ERR_RET("G8032 has been configured, can't configure monitor destination");
        }
        /* check interface is not trunk and not qinq */
        if ((GLB_VLAN_PORT_TYPE_TRUNK != p_if->vlan_type)
            && (GLB_VLAN_PORT_TYPE_QINQ != p_if->vlan_type))
        {
            CFG_FUNC_ERR_RET("This interface is not a vlan trunk port or dot1q-tunnel port");    
        }

        if (_erps_domain_check_used_by_other_domain_with_same_instance(p_db_ring, (void*)p_if))
        {
            CFG_FUNC_ERR_RET("The interface has been used by other domain with the same instance");                
        }
        
        switch (field_id)
        {
        case TBL_ERPS_RING_FLD_EDGE_PORT_IFINDEX:
            if (p_if == p_db_ring->secondary_port_if)
            {
                CFG_FUNC_ERR_RET("Can't set the same interface as common interface");
            }
            
            p_old_if = p_db_ring->primary_port_if;
            p_db_ring->primary_port_if = p_if;
            
            rc = erps_ring_update_port(p_db_ring, p_old_if, p_if);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Set erps ring edge interface fail, %s", pm_err_desc(rc));
            }

            p_ring->primary_port_ifindex = p_if->ifindex;            
            rc = erps_set_erps_ring_primary_interface(p_ring, p_db_ring);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Reserve erps ring edge interface fail, %s", pm_err_desc(rc));
            }
            break;
    
        case TBL_ERPS_RING_FLD_COMMON_PORT_IFINDEX:
            if (p_if == p_db_ring->primary_port_if)
            {
                CFG_FUNC_ERR_RET("Can't set the same interface as edge interface");
            }
            
            p_old_if = p_db_ring->secondary_port_if;
            p_db_ring->secondary_port_if = p_if;
            
            rc = erps_ring_update_port(p_db_ring, p_old_if, p_if);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Set erps ring common interface fail, %s", pm_err_desc(rc));
            }

            p_ring->secondary_port_ifindex = p_if->ifindex;            
            rc = erps_set_erps_ring_secondary_interface(p_ring, p_db_ring);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Reserve erps ring common interface fail, %s", pm_err_desc(rc));
            }
            break;
    
        default:
            CFG_FUNC_ERR_RET("Invalid ERPS interface type");
        }      
    }
    else
    {
        switch (field_id)
        {
        case TBL_ERPS_RING_FLD_EDGE_PORT_IFINDEX:
            p_old_if = p_db_ring->primary_port_if;
            p_db_ring->primary_port_if = NULL;
            
            rc = erps_ring_update_port(p_db_ring, p_old_if, NULL);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Set erps ring edge interface fail, %s", pm_err_desc(rc));
            }

            p_ring->primary_port_ifindex = 0;
            rc = erps_set_erps_ring_primary_interface(p_ring, p_db_ring);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Set erps ring edge interface fail, %s", pm_err_desc(rc));
            }
            break;
    
        case TBL_ERPS_RING_FLD_COMMON_PORT_IFINDEX:
            p_old_if = p_db_ring->secondary_port_if;
            p_db_ring->secondary_port_if = NULL;
            
            rc = erps_ring_update_port(p_db_ring, p_old_if, NULL);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Set erps ring common interface fail, %s", pm_err_desc(rc));
            }

            p_ring->secondary_port_ifindex = 0;
            rc = erps_set_erps_ring_secondary_interface(p_ring, p_db_ring);
            if (rc < 0)
            {
                CFG_FUNC_ERR_RET("Set erps ring common interface fail, %s", pm_err_desc(rc));
            }
            break;
    
        default:
            CFG_FUNC_ERR_RET("Invalid ERPS interface type");
        }   
    }       
    
    return rc;
}

int32 erps_cmd_set_erps_ring_enable(tbl_erps_ring_t *p_ring, 
    tbl_erps_ring_t *p_db_ring, cfg_cmd_para_t *para)
{
    int32                rc = PM_E_NONE;
    tbl_erps_domain_t    *p_db_domain = NULL;
    tbl_interface_t      *p_if = NULL;

    p_db_domain = (tbl_erps_domain_t*)p_db_ring->p_erps_domain;

    /*  check duplicate enable or disable */
    if ((ERPS_INIT == p_ring->erps_ring_state) && para)
    {
        /* modified by wangjj for fix erps bug 41438, 2016-12-08 */
        #if 0
        if (ERPS_IDLE != p_db_ring->erps_ring_state)
        {
            return rc;
        }
        #endif
        if ((p_db_ring->erps_ring_level == ERPS_RING_PRIMARY) && (!p_db_domain->primary_control_vlan_id))
        {
            CFG_FUNC_ERR_RET("Please config the primary control vlan first");
        }
        else if ((p_db_ring->erps_ring_level == ERPS_RING_SUB) && (!p_db_domain->sub_control_vlan_id))
        {
            CFG_FUNC_ERR_RET("Please config the sub control vlan first");
        }
    
        if (ERPS_NODE_UNKNOWN == p_db_ring->node_role)
        {
            CFG_FUNC_ERR_RET("Please set the mode for ring %u first", p_db_ring->key.ring_id);
        }

        /* modified by liwh for bug 45369, 2017-10-16 */
        if (0 == p_db_domain->instance_id_num)
        // if (GLB_ERPS_DOMAIN_INSTANCE_ID_DEFAULT == p_db_domain->instance_id[0])
        /* liwh end */
        {
            CFG_FUNC_ERR_RET("Please specify protected instance");
        }
    
        if (ERPS_NODE_EDGE_UNKNOWN == p_db_ring->edge_node_role)
        {
            if (!p_db_ring->primary_port_if)
            {
                CFG_FUNC_ERR_RET("Please set the primary port for ring %u first", p_db_ring->key.ring_id);
            }
    
            if (!p_db_ring->secondary_port_if)
            {
                CFG_FUNC_ERR_RET("Please set the secondary port for ring %u first", p_db_ring->key.ring_id);
            }
        }
        else
        {
            if (!p_db_ring->primary_port_if)
            {
                CFG_FUNC_ERR_RET("Please set the edge port for ring %u first", p_db_ring->key.ring_id);
            }
            if (!p_db_ring->secondary_port_if)
            {
                CFG_FUNC_ERR_RET("Please set the common port for ring %u first", p_db_ring->key.ring_id);
            }
        }

        if (ERPS_RING_PRIMARY == p_db_ring->erps_ring_level)
        {
            p_if = (tbl_interface_t*)p_db_ring->primary_port_if;
            /* modified by wangjj for fix erps bug 41130, 2016-11-11 */
            if (!GLB_BMP_ISSET(p_if->brgif->allowed_vlan.vlan_bmp, p_db_domain->primary_control_vlan_id))
            {
                CFG_CONFLICT_RET("Primary port, secondary port or edge port, common port for ring %u is not a member of primary control vlan %u",
                    p_db_ring->key.ring_id, p_db_domain->primary_control_vlan_id);
            }

            p_if = (tbl_interface_t*)p_db_ring->secondary_port_if;
            /* modified by wangjj for fix erps bug 41130, 2016-11-11 */
            if (!GLB_BMP_ISSET(p_if->brgif->allowed_vlan.vlan_bmp, p_db_domain->primary_control_vlan_id))
            {
                CFG_CONFLICT_RET("Primary port, secondary port or edge port, common port for ring %u is not a member of primary control vlan %u",
                    p_db_ring->key.ring_id, p_db_domain->primary_control_vlan_id);
            }
        }

        if (ERPS_RING_SUB == p_db_ring->erps_ring_level)
        {
            p_if = (tbl_interface_t*)p_db_ring->primary_port_if;
            /* modified by wangjj for fix erps bug 41130, 2016-11-11 */
            if (!GLB_BMP_ISSET(p_if->brgif->allowed_vlan.vlan_bmp, p_db_domain->sub_control_vlan_id))
            {
                CFG_CONFLICT_RET("Primary port, secondary port or edge port, common port for ring %u is not a member of sub control vlan %u",
                    p_db_ring->key.ring_id, p_db_domain->sub_control_vlan_id);
            }

            p_if = (tbl_interface_t*)p_db_ring->secondary_port_if;
            /* modified by wangjj for fix erps bug 41130, 2016-11-11 */
            if (!GLB_BMP_ISSET(p_if->brgif->allowed_vlan.vlan_bmp, p_db_domain->sub_control_vlan_id))
            {
                CFG_CONFLICT_RET("Primary port, secondary port or edge port, common port for ring %u is not a member of sub control vlan %u",
                    p_db_ring->key.ring_id, p_db_domain->sub_control_vlan_id);
            }
        }

        /* modified by wangjj for fix erps bug 41438, 2016-12-08 */
        if (ERPS_IDLE != p_db_ring->erps_ring_state)
        {
            return rc;
        }

        if (ERPS_NODE_MASTER == p_db_ring->node_role)
        {
            /* modified by liwh for bug 43100, 2017-03-27 */
            /*if (p_db_ring->hello_timer)
            {
                ctc_task_delete_timer(p_db_ring->hello_timer);
                p_db_ring->hello_timer = NULL;
            }*/
            CTC_TASK_STOP_TIMER(p_db_ring->hello_timer);
            /* liwh end */
            
            p_db_ring->hello_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_hello_timer_fn, p_db_ring, 
                p_db_domain->hello_timer_interval);

            if (!p_db_ring->hello_timer)
            {
                CFG_CONFLICT_RET("hello timer set fail");
            }

            if (tbl_interface_is_running((tbl_interface_t*)p_db_ring->primary_port_if)
                && tbl_interface_is_running((tbl_interface_t*)p_db_ring->secondary_port_if))
            {
                p_db_ring->failure_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_failure_timeout_pre_fn, 
                    p_db_ring, (p_db_domain->failure_timer_interval - 1));

                if (!p_db_ring->failure_timer)
                {
                    CFG_CONFLICT_RET("failure timer set fail");
                }
    
                /* use blocked_port_if in master node to remember the state of secondary port interface */
                p_db_ring->blocked_port_if = p_db_ring->secondary_port_if; 
                p_db_ring->blocked_port_ifindex = p_db_ring->secondary_port_ifindex;
                erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, TRUE);

                /* flush fdb */
                erps_fdb_flush_dynamic_fdb_port(p_db_ring->primary_port_ifindex);              
                erps_fdb_flush_dynamic_fdb_port(p_db_ring->secondary_port_ifindex);

                /* send packets */
                erps_msg_send_pkt(ERPS_HEALTH_PKT, (tbl_interface_t*)p_db_ring->primary_port_if, 
                    p_db_domain, p_db_ring);
                erps_msg_send_pkt(ERPS_RING_UP_FLUSH_FDB_PKT, (tbl_interface_t*)p_db_ring->primary_port_if, 
                    p_db_domain, p_db_ring);
                
                p_ring->erps_ring_state = ERPS_INIT;
            }
            else
            {   
                p_ring->erps_ring_state = ERPS_FAILED;
            }    
        }
        else if(ERPS_NODE_TRANSIT == p_db_ring->node_role)
        {
            /* both links are up, enter link up */
            if (tbl_interface_is_running((tbl_interface_t*)p_db_ring->primary_port_if)
                && tbl_interface_is_running((tbl_interface_t*)p_db_ring->secondary_port_if))
            {
                p_ring->erps_ring_state = ERPS_LINK_UP;
            }
            /* one or two link(s) is down, etner link down */
            else
            {
                p_ring->erps_ring_state = ERPS_LINK_DOWN;
            }
    
            if ((ERPS_NODE_EDGE == p_db_ring->edge_node_role) && (p_db_ring->erps_srpt_enable))
            {
                p_db_ring->edge_hello_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_edge_hello_timer_fn, 
                    p_db_ring, (p_db_domain->edge_hello_timer_interval));

                if (!p_db_ring->edge_hello_timer)
                {
                    CFG_CONFLICT_RET("Edge hello timer set fail");
                }
            }   
    
            if ((ERPS_NODE_ASSI_EDGE == p_db_ring->edge_node_role) && (p_db_ring->erps_srpt_enable))
            {
                p_db_ring->edge_failure_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_edge_failure_timer_fn, 
                    p_db_ring, p_db_domain->edge_failure_timer_interval); 

                if (!p_db_ring->edge_failure_timer)
                {
                    CFG_CONFLICT_RET("Edge f timer set fail");
                }
            }       
        }
    }
    else if (ERPS_IDLE == p_ring->erps_ring_state)
    {
        if (ERPS_IDLE == p_db_ring->erps_ring_state)
        {
            return rc;
        }

        switch (p_db_ring->erps_ring_state)
        {   
            case ERPS_INIT:
            case ERPS_COMPLETE:
                /*if (p_db_ring->hello_timer)
                {
                    ctc_task_delete_timer(p_db_ring->hello_timer);
                    p_db_ring->hello_timer = NULL;
                }*/
                CTC_TASK_STOP_TIMER(p_db_ring->hello_timer);

                /*if (p_db_ring->failure_timer)
                {
                    ctc_task_delete_timer(p_db_ring->failure_timer);
                    p_db_ring->failure_timer = NULL;
                }*/
                CTC_TASK_STOP_TIMER(p_db_ring->failure_timer);
                
                /*if (p_db_ring->flush_timer)
                {
                    ctc_task_delete_timer(p_db_ring->flush_timer);
                    p_db_ring->flush_timer = NULL;
                }*/
                CTC_TASK_STOP_TIMER(p_db_ring->flush_timer);
                
                erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE); 
                p_db_ring->blocked_port_if = NULL; 
                p_db_ring->blocked_port_ifindex = 0;
                break;
                
            case ERPS_FAILED:
                /*if (p_db_ring->failure_timer)
                {
                    ctc_task_delete_timer(p_db_ring->failure_timer);
                    p_db_ring->failure_timer = NULL;           
                }*/
                CTC_TASK_STOP_TIMER(p_db_ring->failure_timer);

                /*if (p_db_ring->hello_timer)
                {
                    ctc_task_delete_timer(p_db_ring->hello_timer);
                    p_db_ring->hello_timer = NULL;
                }*/
                CTC_TASK_STOP_TIMER(p_db_ring->hello_timer);
                
                /*if (p_db_ring->flush_timer)
                {
                    ctc_task_delete_timer(p_db_ring->flush_timer);
                    p_db_ring->flush_timer = NULL;
                }*/
                CTC_TASK_STOP_TIMER(p_db_ring->flush_timer);
  
                erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);
                p_db_ring->blocked_port_if = NULL; 
                p_db_ring->blocked_port_ifindex = 0;

                /*if (p_db_ring->pre_forwarding_timer)
                {
                    ctc_task_delete_timer(p_db_ring->pre_forwarding_timer);
                    p_db_ring->pre_forwarding_timer = NULL;
                }*/
                CTC_TASK_STOP_TIMER(p_db_ring->pre_forwarding_timer);
                break;
                
            case ERPS_LINK_UP:  
                break;
                
            case ERPS_LINK_DOWN:           
                break;
                
            case ERPS_PREFORWORDING:             
                erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);
                p_db_ring->blocked_port_if = NULL;                 
                p_db_ring->blocked_port_ifindex = 0;

                /*if (p_db_ring->pre_forwarding_timer)
                {
                    ctc_task_delete_timer(p_db_ring->pre_forwarding_timer);
                    p_db_ring->pre_forwarding_timer = NULL;   
                }*/
                CTC_TASK_STOP_TIMER(p_db_ring->pre_forwarding_timer);
                break;
                
            default:
                break;
        }

        p_db_ring->erps_ring_state = ERPS_IDLE;
    
        if (ERPS_NODE_EDGE_UNKNOWN != p_db_ring->edge_node_role)
        {
            /*if (p_db_ring->edge_hello_timer)
            {
                ctc_task_delete_timer(p_db_ring->edge_hello_timer);
                p_db_ring->edge_hello_timer = NULL;
            }*/
            CTC_TASK_STOP_TIMER(p_db_ring->edge_hello_timer);
            
            /*if (p_db_ring->edge_failure_timer)
            {
                ctc_task_delete_timer(p_db_ring->edge_failure_timer);
                p_db_ring->edge_failure_timer = NULL;
            }*/
            CTC_TASK_STOP_TIMER(p_db_ring->edge_failure_timer);

            /*if (p_db_ring->edge_block_timer)
            {
                ctc_task_delete_timer(p_db_ring->edge_block_timer);
                p_db_ring->edge_block_timer = NULL;
            }*/
            CTC_TASK_STOP_TIMER(p_db_ring->edge_block_timer);
            
            erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);  
            p_db_ring->blocked_port_if = NULL;    
            p_db_ring->blocked_port_ifindex = 0;
        }
    }

    rc = erps_set_erps_ring_state(p_ring, p_db_ring);
    if (rc < 0)
    {
        ERPS_LOG_ERR("Set erps ring enable fail, %s", pm_err_desc(rc));
    }
    
    return rc;
}

static int32 _erps_cmd_set_erps_ring_srpt_enable(tbl_erps_ring_t *p_ring, 
    tbl_erps_ring_t *p_db_ring, cfg_cmd_para_t *para)
{
    int32               rc = PM_E_NONE;
    tbl_erps_domain_t   *p_db_domain = NULL;

    if (ERPS_NODE_EDGE_UNKNOWN == p_db_ring->edge_node_role)
    {
        CFG_FUNC_ERR_RET("This command is only applicable to edge/assistant-edge node");
    }

    p_db_domain = (tbl_erps_domain_t*)p_db_ring->p_erps_domain;

    if (ERPS_NODE_EDGE == p_db_ring->edge_node_role)  
    {
        if (p_db_ring->erps_srpt_enable != p_ring->erps_srpt_enable)
        {
            if (!p_ring->erps_srpt_enable)
            {
                /*if (p_db_ring->edge_hello_timer)
                {
                    ctc_task_delete_timer(p_db_ring->edge_hello_timer);
                    p_db_ring->edge_hello_timer = NULL;
                }*/
                CTC_TASK_STOP_TIMER(p_db_ring->edge_hello_timer);
                
                if (p_db_ring->blocked_port_if == p_db_ring->primary_port_if)
                {                    
                    erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);
                    p_db_ring->blocked_port_if = NULL;                 
                    p_db_ring->blocked_port_ifindex = 0;
                }
            }
            else
            {
                if ((!p_db_ring->edge_hello_timer) && (ERPS_IDLE != p_db_ring->erps_ring_state))
                {
                    p_db_ring->edge_hello_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_edge_hello_timer_fn, 
                        p_db_ring, p_db_domain->edge_hello_timer_interval);

                    if (!p_db_ring->edge_hello_timer)
                    {
                        CFG_FUNC_ERR_RET("edge hello timer set fail in enable srpt func");
                    }
                }
            }
        }
    }
    else if (ERPS_NODE_ASSI_EDGE == p_db_ring->edge_node_role)  
    {
        if (p_db_ring->erps_srpt_enable != p_ring->erps_srpt_enable)
        {
            if (!p_ring->erps_srpt_enable)
            {
                /*if (p_db_ring->edge_failure_timer)
                {
                    ctc_task_delete_timer(p_db_ring->edge_failure_timer);
                    p_db_ring->edge_failure_timer = NULL;
                }*/
                CTC_TASK_STOP_TIMER(p_db_ring->edge_failure_timer);
            }
            else
            {
                if (ERPS_IDLE != p_db_ring->erps_ring_state)
                {             
                    p_db_ring->edge_failure_timer = ctc_task_add_timer(TASK_PRI_NORMAL, 
                        erps_timer_edge_failure_timer_fn, p_db_ring, p_db_domain->edge_failure_timer_interval);

                    if (!p_db_ring->edge_failure_timer)
                    {
                        CFG_FUNC_ERR_RET("edge failure timer set fail in enable srpt func");
                    }
                }
            }
        }
    }
    
    rc = erps_set_erps_ring_srpt_enable(p_ring, p_db_ring);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Set erps ring srpt fail, %s", pm_err_desc(rc));
    }
    
    return rc;
}

int32
erps_cmd_parse_ring_field(char **argv, int32 argc, cdb_node_t* p_node, field_parse_t *field, uint32 with_key)
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

static int32
_erps_cmd_get_ring(uint32  domain_id, uint32 ring_id, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                  *fp = NULL;
    tbl_erps_ring_key_t   key;
    tbl_erps_ring_t       *p_db_ring = NULL;
    tbl_iter_args_t       args;    

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if ((0 == domain_id) || (0 == ring_id))
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_erps_ring_dump_one, &args);
    }
    else
    {
        key.domain_id = domain_id; 
        key.ring_id = ring_id; 
        p_db_ring = tbl_erps_ring_get_erps_ring(&key);
        if (!p_db_ring)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("ring %u/%u does not exist", domain_id, ring_id);
        }

        tbl_erps_ring_dump_one(p_db_ring, &args);
    }
    
    sal_fclose(fp);
    return PM_E_NONE;
}

int32
erps_cmd_process_erps_ring(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t               *p_node = NULL;
    tbl_erps_domain_t        *p_db_domain = NULL;
    tbl_erps_ring_t          erps_ring;
    tbl_erps_ring_t          *p_db_ring = NULL;
    tbl_erps_domain_key_t    domain_key;
    int32                    rc = PM_E_NONE;
    uint32                   domain_id = 0;
    uint32                   ring_id = 0;
    uint32                   level = 0;
    int32                    i = 0;
    cdb_field_t              *p_field = NULL;
    int32                    field_id = 0;
    field_parse_t            field;
    int32                    role_value = 0;
    
    CFG_DBG_FUNC();

    sal_memset(&erps_ring, 0, sizeof(tbl_erps_ring_t));
    sal_memset(&domain_key, 0, sizeof(tbl_erps_domain_key_t));
    sal_memset(&field, 0, sizeof(field_parse_t));
    
    p_node = cdb_get_tbl(TBL_ERPS_RING);
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_GET_UINT(domain_id, argv[0], para->p_rs);
 
        domain_key.domain_id = domain_id;        
        p_db_domain = tbl_erps_domain_get_erps_domain(&domain_key);
        if (!p_db_domain)
        {
            CFG_FUNC_ERR_RET("ERPS domain %d hasn't been created", domain_id);
        }

        i++;
        PM_CFG_GET_UINT(ring_id, argv[i], para->p_rs);

        erps_ring.key.domain_id = domain_id;
        erps_ring.key.ring_id = ring_id;
        p_db_ring = tbl_erps_ring_get_erps_ring(&erps_ring.key);
        if ((p_db_ring) && (argc == (i+1)))
        {
            return PM_E_NONE;
        }

        /* modified by wangjj for fix erps bug 41035, 2016-11-26 */
        if ((!p_db_ring) && (argc == (i+1)))
        {
            CFG_FUNC_ERR_RET("ERPS ring %u hasn't been created", ring_id);
        }

        if (argc == (i+1))
        {
            CFG_INVALID_PARAM_RET("Unknown ring level");
        }

        i++;
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }

        if (TBL_ERPS_RING_FLD_ERPS_RING_LEVEL != field_id)
        {
            CFG_INVALID_PARAM_RET("Unknown ring level");
        }

        i++;
        level = cdb_enum_str2val(erps_ring_level_strs, ERPS_RING_LEVEL_MAX, argv[i]);
        if (level < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid type parameter %s", argv[i]);
        }

        if (p_db_ring)
        {
            if (p_db_ring->erps_ring_level != level) 
            {
                CFG_INVALID_PARAM_RET("ERPS ring is existed, can not set ring");
            }
            else
            {
                return PM_E_NONE;    
            }
        }

        rc = _erps_cmd_add_erps_ring(p_db_domain, ring_id, level, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_GET_UINT(domain_id, argv[0], para->p_rs);
 
        domain_key.domain_id = domain_id;        
        p_db_domain = tbl_erps_domain_get_erps_domain(&domain_key);
        if (!p_db_domain)
        {
            CFG_FUNC_ERR_RET("ERPS domain %d hasn't been created", domain_id);
        }

        i++;
        PM_CFG_GET_UINT(ring_id, argv[i], para->p_rs);
        rc = erps_cmd_del_erps_ring(p_db_domain, ring_id, para);
        break;
    case CDB_OPER_SET:  
        i = 1;
        sal_sscanf(argv[i], "%u-%u", &domain_id, &ring_id);  
        domain_key.domain_id = domain_id;        
        p_db_domain = tbl_erps_domain_get_erps_domain(&domain_key);
        if (!p_db_domain)
        {
            CFG_FUNC_ERR_RET("ERPS domain %d hasn't been created", domain_id);
        }

        i++;
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }

        erps_ring.key.domain_id = domain_id;
        erps_ring.key.ring_id = ring_id;

        p_db_ring = tbl_erps_ring_get_erps_ring(&erps_ring.key);
        if (!p_db_ring)
        {
            CFG_FUNC_ERR_RET("ERPS ring %u hasn't been created", ring_id);
        }
        
        switch (field_id)
        {
        case TBL_ERPS_RING_FLD_NODE_ROLE:
            i++;
            role_value = cdb_enum_str2val(erps_node_role_strs, ERPS_NODE_ROLE_MAX, argv[i]);
            if (role_value < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid type parameter %s", argv[i]);
            }
            erps_ring.node_role = role_value;
            rc = _erps_cmd_set_erps_ring_node_role(&erps_ring, p_db_ring, para);
            break;

        case TBL_ERPS_RING_FLD_EDGE_NODE_ROLE:
            i++;
            role_value = cdb_enum_str2val(erps_edge_node_role_strs, ERPS_EDGE_NODE_ROLE_MAX, argv[i]);
            if (role_value < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid type parameter %s", argv[i]);
            }
            erps_ring.edge_node_role = role_value;
            rc = _erps_cmd_set_erps_ring_edge_node_role(&erps_ring, p_db_ring, para);
            break;

        case TBL_ERPS_RING_FLD_ERPS_SRPT_ENABLE:
            i++;
            PM_CFG_GET_UINT(erps_ring.erps_srpt_enable, argv[i], para->p_rs);
            rc = _erps_cmd_set_erps_ring_srpt_enable(&erps_ring, p_db_ring, para);
            break;

        case TBL_ERPS_RING_FLD_ERPS_RING_STATE:
            i++;
            PM_CFG_GET_UINT(erps_ring.erps_ring_state, argv[i], para->p_rs);
            rc = erps_cmd_set_erps_ring_enable(&erps_ring, p_db_ring, para);
            break;

        case TBL_ERPS_RING_FLD_PRIMARY_PORT_IFINDEX:
            i++;
            rc = _erps_cmd_set_erps_ring_port(field_id, argv[i], &erps_ring, p_db_ring, para);
            break;

        case TBL_ERPS_RING_FLD_SECONDARY_PORT_IFINDEX:
            i++;            
            rc = _erps_cmd_set_erps_ring_port(field_id, argv[i], &erps_ring, p_db_ring, para);
            break;

        case TBL_ERPS_RING_FLD_EDGE_PORT_IFINDEX:
            i++;
            rc = _erps_cmd_set_erps_ring_edge_port(field_id, argv[i], &erps_ring, p_db_ring, para);
            break;

        case TBL_ERPS_RING_FLD_COMMON_PORT_IFINDEX:
            i++;
            rc = _erps_cmd_set_erps_ring_edge_port(field_id, argv[i], &erps_ring, p_db_ring, para);
            break;    
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;
    case CDB_OPER_GET:
        if (2 == argc)
        {
            PM_CFG_GET_UINT(domain_id, argv[0], para->p_rs);
            PM_CFG_GET_UINT(ring_id, argv[1], para->p_rs);
        }
        
        rc = erps_cmd_parse_ring_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        rc = _erps_cmd_get_ring(domain_id, ring_id, para, p_node, &field);
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
erps_cmd_show_erps_list(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    tbl_erps_domain_master_t  *p_master = NULL;
    uint32                    domain_id = 0;
    tbl_erps_domain_t         *p_db_domain = NULL;
    FILE                      *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "ERPS Domain List \n");
    sal_fprintf(fp, "ID        Name   \n");
    sal_fprintf(fp, "============================== \n");

    p_master = tbl_erps_domain_get_master();
    for (domain_id = 1; domain_id < GLB_MAX_ERPS_DOMAIN; domain_id++)
    {
        p_db_domain = p_master->domain_array[domain_id];
        if (NULL == p_db_domain)
        {
            continue;
        }

        sal_fprintf(fp, "%-9d %s\n", p_db_domain->key.domain_id, p_db_domain->name);
    }

    sal_fclose(fp);
    fp = NULL;
        
    return rc;
}

#define M_ERPS_STR_BUFSIZE  1600

void 
erps_get_instance_str(tbl_erps_domain_t *p_db_domain, char *str)
{
    uint32           index = 0;
    uint32           start = 0xffff;
    uint32           end = 0;
    char             *p = NULL;

    for (index = 0; index < p_db_domain->instance_id_num; index++)
    {
        if (0xffff == start)
        { 
            start = p_db_domain->instance_id[index];
            end = p_db_domain->instance_id[index];
            continue;
        }
        else
        {
            if ((end + 1) == p_db_domain->instance_id[index])
            {
                end = p_db_domain->instance_id[index];
                continue;
            }
        }

        p = str;
        p = p + sal_strlen(str);

        if (start < end)
        {
            sal_snprintf(p, M_ERPS_STR_BUFSIZE, "%d-%d,", start, end);
        }
        else
        {
            sal_snprintf(p, M_ERPS_STR_BUFSIZE, "%d,", start);
        }
        start = p_db_domain->instance_id[index];
        end = p_db_domain->instance_id[index];
    }

    p = str;
    p = p + sal_strlen(str);
    
    if (start < end)
    {
        sal_snprintf(p, M_ERPS_STR_BUFSIZE, "%d-%d", start, end);
    }
    else if (start == end)
    {
        sal_snprintf(p, M_ERPS_STR_BUFSIZE, "%d", start);
    }

    return;
}


int32
erps_cmd_show_erps(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_erps_domain_t        *p_db_domain = NULL;
    tbl_erps_domain_key_t    key;
    int32                    rc = PM_E_NONE;
    uint32                   domain_id = 0;
    FILE                     *fp = NULL;
    tbl_erps_ring_t          *p_erps_ring = NULL;
    ctclib_slistnode_t       *listnode = NULL;
    uint8                    erps_node_mode[8];
    uint8                    erps_node_state[16];
    tbl_interface_t          *p_if = NULL;
    char                     ifname_ext[IFNAME_EXT_SIZE];
    char                     str[M_ERPS_STR_BUFSIZE] = {'\0'};

    PM_CFG_GET_UINT(domain_id, argv[0], para->p_rs);

    key.domain_id = domain_id;
    p_db_domain = tbl_erps_domain_get_erps_domain(&key);
    if (!p_db_domain)
    {
        /* modified by wangjj for fix erps bug 41297, 2016-11-10 */
        CFG_FUNC_ERR_RET("ERPS domain %d hasn't been created", domain_id);
        //return PM_E_NONE;
    }

    /* modified by wangjj for fix erps bug 41211, 2016-11-21 */
    uint32              erps_mode_is_rrpp   = 0;
    
    erps_mode_is_rrpp = erps_mode_is_huawei();

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "ERPS domain ID: %d\n", p_db_domain->key.domain_id);
    sal_fprintf(fp, "ERPS domain name: %s\n", p_db_domain->name);
    /* modified by wangjj for fix erps bug 41211, 2016-11-21 */
    sal_fprintf(fp, "ERPS domain mode: %s\n", erps_mode_is_rrpp ? "RRPP":"normal");
    sal_fprintf(fp, "ERPS domain primary control VLAN ID: %d\n", p_db_domain->primary_control_vlan_id);
    sal_fprintf(fp, "ERPS domain sub control VLAN ID: %d\n", p_db_domain->sub_control_vlan_id);
    sal_fprintf(fp, "ERPS domain hello timer interval: %u second(s)\n", p_db_domain->hello_timer_interval);
    sal_fprintf(fp, "ERPS domain fail timer interval: %u second(s)\n", p_db_domain->failure_timer_interval);

    /* modified by wangjj for fix erps bug 41211, 2016-12-02 */
    if (0 != p_db_domain->instance_id_num)
    {
        erps_get_instance_str(p_db_domain, str);
        sal_fprintf(fp, "ERPS domain protected mstp instance: %s\n", str);
    }

    CTCLIB_SLIST_LOOP(p_db_domain->erps_ring_list.obj_list, p_erps_ring, listnode)
    {
        sal_memset(erps_node_mode, 0, sizeof(erps_node_mode));
        sal_memset(erps_node_state, 0, sizeof(erps_node_state));

        sal_fprintf(fp, "ERPS ring ID: %d\n", p_erps_ring->key.ring_id);
        sal_fprintf(fp, "ERPS ring level: %s\n", (p_erps_ring->erps_ring_level == ERPS_RING_PRIMARY) ? "primary":"sub");
    
        switch(p_erps_ring->node_role)
        {
            case ERPS_NODE_UNKNOWN:
                sal_strncpy((char*)erps_node_mode, "unknown", sizeof(erps_node_mode)-1);
                break;
            case ERPS_NODE_MASTER:
                sal_strncpy((char*)erps_node_mode, "master", sizeof(erps_node_mode)-1);
                break;
            case ERPS_NODE_TRANSIT:
                sal_strncpy((char*)erps_node_mode, "transit", sizeof(erps_node_mode)-1);
                break;
            default:
                ERPS_LOG_ERR("wrong node mode!");
                break;
        }
    
        sal_fprintf(fp, "ERPS ring %u node mode: %s\n", p_erps_ring->key.ring_id, erps_node_mode);
    
        if (ERPS_NODE_EDGE_UNKNOWN != p_erps_ring->edge_node_role)
        {
            sal_fprintf(fp, "ERPS ring %u edge node mode: %s\n", p_erps_ring->key.ring_id, \
                (ERPS_NODE_EDGE == p_erps_ring->edge_node_role)  ? "edge":"assistant edge");
        }
    
        switch(p_erps_ring->erps_ring_state)
        {
            case ERPS_IDLE:
                sal_strncpy((char*)erps_node_state, "idle", sizeof(erps_node_state)-1);
                break;
            case ERPS_COMPLETE:
                sal_strncpy((char*)erps_node_state, "complete", sizeof(erps_node_state)-1);
                break;
            case ERPS_INIT:
                sal_strncpy((char*)erps_node_state, "init", sizeof(erps_node_state)-1);
                break;
            case ERPS_FAILED:
                sal_strncpy((char*)erps_node_state, "failed", sizeof(erps_node_state)-1);
                break;
            case ERPS_LINK_UP:
                sal_strncpy((char*)erps_node_state, "link up", sizeof(erps_node_state)-1);
                break;
            case ERPS_LINK_DOWN:
                sal_strncpy((char*)erps_node_state, "link down", sizeof(erps_node_state)-1);
                break;
            case ERPS_PREFORWORDING:
                sal_strncpy((char*)erps_node_state, "preforwording", sizeof(erps_node_state)-1);
                break;
            default:
                ERPS_LOG_ERR("wrong node state");
                break;
        }
    
        sal_fprintf(fp, "ERPS ring %u node state: %s\n", p_erps_ring->key.ring_id, erps_node_state);
    
        if (ERPS_NODE_EDGE_UNKNOWN != p_erps_ring->edge_node_role)
        {
            p_if = p_erps_ring->primary_port_if;
            if(p_erps_ring->primary_port_if && p_if->key.name)
            {    
                IFNAME_ETH2FULL(p_if->key.name, ifname_ext);
                sal_fprintf(fp, "ERPS ring %u edge interface name: %-10s     state: %s\n", 
                    p_erps_ring->key.ring_id, ifname_ext, 
                    (p_erps_ring->primary_port_if == p_erps_ring->blocked_port_if) ? "block":"unblock");
            }
            else
            {
                sal_fprintf(fp, "ERPS ring %u edge interface name: not configured\n", p_erps_ring->key.ring_id);
            }

            p_if = p_erps_ring->secondary_port_if;
            if(p_erps_ring->secondary_port_if && p_if->key.name)
            {
                IFNAME_ETH2FULL(p_if->key.name, ifname_ext);
                sal_fprintf(fp, "ERPS ring %u common interface name: %-8s     state: %s\n", 
                    p_erps_ring->key.ring_id, ifname_ext, 
                    (p_erps_ring->secondary_port_if == p_erps_ring->blocked_port_if) ? "block":"unblock");
            }
            else
            {
                sal_fprintf(fp, "ERPS ring %u common interface name: not configured\n", p_erps_ring->key.ring_id);
            }
        }
        else
        {     
            p_if = p_erps_ring->primary_port_if;
            if(p_erps_ring->primary_port_if && p_if->key.name)
            {
                IFNAME_ETH2FULL(p_if->key.name, ifname_ext);
                sal_fprintf(fp, "ERPS ring %u primary interface name: %-10s     state:%s\n", 
                    p_erps_ring->key.ring_id, ifname_ext, 
                    (p_erps_ring->primary_port_if == p_erps_ring->blocked_port_if) ? "block":"unblock");
            }
            else
            {
                sal_fprintf(fp, "ERPS ring %u primary interface name: not configured\n", p_erps_ring->key.ring_id);
            }

            p_if = p_erps_ring->secondary_port_if;
            if(p_erps_ring->secondary_port_if && p_if->key.name)
            {
                IFNAME_ETH2FULL(p_if->key.name, ifname_ext);
                sal_fprintf(fp, "ERPS ring %u secondary interface name: %-8s     state:%s\n", 
                    p_erps_ring->key.ring_id, ifname_ext, 
                    (p_erps_ring->secondary_port_if == p_erps_ring->blocked_port_if) ? "block":"unblock");
            }
            else
            {
                sal_fprintf(fp, "ERPS ring %u secondary interface name: not configured\n", p_erps_ring->key.ring_id);
            }
        }
    
        if (ERPS_NODE_EDGE_UNKNOWN != p_erps_ring->edge_node_role)
        {
            sal_fprintf(fp, "EPRS ring %u SRPT is %s\n", p_erps_ring->key.ring_id, 
                p_erps_ring->erps_srpt_enable ? "enabled":"disabled");
        }
    
        sal_fprintf(fp, "ERPS ring %u stats:\n", p_erps_ring->key.ring_id);
    
        sal_fprintf(fp, "Sent:\n");      
        sal_fprintf(fp, "  total packets:%u\n", p_erps_ring->send_hello_cnt + p_erps_ring->send_ring_down_flush_fdb_cnt 
            + p_erps_ring->send_ring_up_flush_fdb_cnt + p_erps_ring->send_link_down_cnt
            + p_erps_ring->send_edge_hello_cnt + p_erps_ring->send_major_fault_cnt);
        sal_fprintf(fp, "  hello packets:%-20u", p_erps_ring->send_hello_cnt);
        sal_fprintf(fp, "  ring-up-flush-fdb packets:%u\n", p_erps_ring->send_ring_up_flush_fdb_cnt);
        sal_fprintf(fp, "  ring-down-flush-fdb packets:%-6u", p_erps_ring->send_ring_down_flush_fdb_cnt);
        sal_fprintf(fp, "  link-down packets:%u\n", p_erps_ring->send_link_down_cnt);
        sal_fprintf(fp, "  edge-hello packets:%-15u", p_erps_ring->send_edge_hello_cnt);
        sal_fprintf(fp, "  major-fault packets:%u\n", p_erps_ring->send_major_fault_cnt);
    
        sal_fprintf(fp, "Received:\n");
        sal_fprintf(fp, "  total packets:%u\n", p_erps_ring->rcv_hello_cnt + p_erps_ring->rcv_ring_down_flush_fdb_cnt 
            + p_erps_ring->rcv_ring_up_flush_fdb_cnt + p_erps_ring->rcv_link_down_cnt
            + p_erps_ring->rcv_edge_hello_cnt + p_erps_ring->rcv_major_fault_cnt);
        sal_fprintf(fp, "  hello packets:%-20u", p_erps_ring->rcv_hello_cnt);
        sal_fprintf(fp, "  ring-up-flush-fdb packets:%u\n", p_erps_ring->rcv_ring_up_flush_fdb_cnt);
        sal_fprintf(fp, "  ring-down-flush-fdb packets:%-6u", p_erps_ring->rcv_ring_down_flush_fdb_cnt);
        sal_fprintf(fp, "  link-down packets:%u\n", p_erps_ring->rcv_link_down_cnt);
        sal_fprintf(fp, "  edge-hello packets:%-15u", p_erps_ring->rcv_edge_hello_cnt);
        sal_fprintf(fp, "  major-fault packets:%u\n", p_erps_ring->rcv_major_fault_cnt);
    }

    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

int32
erps_cmd_clear_erps_counters(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                      rc = PM_E_NONE;
    tbl_erps_domain_master_t   *p_master = NULL;
    uint32                     domain_id = 0;
    uint32                     ring_id = 0;
    tbl_erps_domain_t          *p_db_domain = NULL;
    tbl_erps_ring_t            *p_erps_ring = NULL;
    ctclib_slistnode_t         *listnode = NULL;
    tbl_erps_ring_t            ring;

    sal_memset(&ring, 0, sizeof(ring));
    /* modified by wangjj for fix erps bug 41186, 2016-11-11 */
    p_master = tbl_erps_domain_get_master();
    if (NULL == p_master)
    {
        return PM_E_FAIL;
    }

    if (2 == argc)
    {
        PM_CFG_GET_UINT(domain_id, argv[0], para->p_rs);
        PM_CFG_GET_UINT(ring_id, argv[1], para->p_rs);

        ring.key.domain_id = domain_id;
        ring.key.ring_id = ring_id;
        p_db_domain = p_master->domain_array[domain_id];
        if (NULL == p_db_domain)
        {
            CFG_INVALID_PARAM_RET("ERPS domain %d hasn't been created", domain_id);
        }

        p_erps_ring = tbl_erps_ring_get_erps_ring(&ring.key);
        if (NULL == p_erps_ring)
        {
            CFG_INVALID_PARAM_RET("ERPS ring %u hasn't been created", ring_id);
        }

        rc = erps_clear_erps_ring_counters(&ring);
    }
    else if (1 == argc)
    {
        for (domain_id = 1; domain_id < GLB_MAX_ERPS_DOMAIN; domain_id++)
        {
            p_db_domain = p_master->domain_array[domain_id];
            if (NULL == p_db_domain)
            {
                continue;
            }

            CTCLIB_SLIST_LOOP(p_db_domain->erps_ring_list.obj_list, p_erps_ring, listnode)
            {
                sal_memset(&ring, 0, sizeof(ring));
                ring.key.domain_id = p_db_domain->key.domain_id;
                ring.key.ring_id = p_erps_ring->key.ring_id;

                rc = erps_clear_erps_ring_counters(&ring);
            }
        }
    }

    return rc;
}

