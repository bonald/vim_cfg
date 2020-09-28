
#include "proto.h"
#include "glb_if_define.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "gen/tbl_erps_domain_define.h"
#include "gen/tbl_erps_domain.h"
#include "gen/tbl_erps_ring_define.h"
#include "gen/tbl_erps_ring.h"
#include "erps_define.h"
#include "erps.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_opm_debug_define.h"
#include "gen/tbl_opm_debug.h"
#include "gen/tbl_opm_global_define.h"
#include "gen/tbl_opm_global.h"
#include "gen/tbl_igsp_intf_define.h"
#include "gen/tbl_igsp_intf.h"

tbl_interface_t* flush_p_if = NULL;
tbl_interface_t* flush_s_if = NULL;
tbl_interface_t* flush_p_p_if = NULL;
tbl_interface_t* flush_p_s_if = NULL;

static const uint8 erps_addr1[6] = {0x01, 0x80, 0x63, 0x07, 0x00, 0x00};
static const uint8 erps_addr2[6] = {0x00, 0x01, 0x7A, 0x4F, 0x48, 0x26};

static const uint8 erps_addr1_hw[6] = {0x00, 0x0f, 0xE2, 0x07, 0x82, 0x17};
static const uint8 erps_addr2_hw[6] = {0x00, 0x0f, 0xE2, 0x07, 0x82, 0x57};
static const uint8 erps_addr3_hw[6] = {0x00, 0x0f, 0xE2, 0x07, 0x82, 0x97};
static const uint8 erps_addr4_hw[6] = {0x00, 0x0f, 0xE2, 0x07, 0x82, 0xD6};

int32
erps_add_erps_domain(uint16 domain_id, char* name)
{
    tbl_erps_domain_t     domain;    
    int32                 rc = PM_E_NONE;

    sal_memset(&domain, 0, sizeof(domain));
    domain.key.domain_id = domain_id;
    sal_memcpy(domain.name, name, ERPS_MAX_NAME_LEN);
    
    /* 2. add to DB */
    PM_E_RETURN(tbl_erps_domain_add_erps_domain(&domain));

    /* set fld to default value */
    domain.hello_timer_interval = GLB_ERPS_DOMAIN_HELLO_TIME_DEFAULT;
    PM_E_RETURN(tbl_erps_domain_set_erps_domain_field(&domain, TBL_ERPS_DOMAIN_FLD_HELLO_TIMER_INTERVAL));

    domain.failure_timer_interval = GLB_ERPS_DOMAIN_FAILURE_TIME_DEFAULT;
    PM_E_RETURN(tbl_erps_domain_set_erps_domain_field(&domain, TBL_ERPS_DOMAIN_FLD_FAILURE_TIMER_INTERVAL));

    domain.pre_forwarding_timer_interval = GLB_ERPS_DOMAIN_PRE_FORWARDING_TIMER_DEFAULT;   
    PM_E_RETURN(tbl_erps_domain_set_erps_domain_field(&domain, TBL_ERPS_DOMAIN_FLD_PRE_FORWARDING_TIMER_INTERVAL));

    domain.flush_timer_interval = GLB_ERPS_DOMAIN_FLUSH_TIMER_DEFAULT;  
    PM_E_RETURN(tbl_erps_domain_set_erps_domain_field(&domain, TBL_ERPS_DOMAIN_FLD_FLUSH_TIMER_INTERVAL));

    domain.edge_hello_timer_interval = GLB_ERPS_DOMAIN_HELLO_TIME_DEFAULT;  
    PM_E_RETURN(tbl_erps_domain_set_erps_domain_field(&domain, TBL_ERPS_DOMAIN_FLD_EDGE_HELLO_TIMER_INTERVAL));

    domain.edge_failure_timer_interval = GLB_ERPS_DOMAIN_HELLO_TIME_DEFAULT * 2;  
    PM_E_RETURN(tbl_erps_domain_set_erps_domain_field(&domain, TBL_ERPS_DOMAIN_FLD_EDGE_FAILURE_TIMER_INTERVAL));

    return rc;
}

int32
erps_del_erps_domain(uint16 domain_id)
{
    tbl_erps_domain_t     domain;
    int32                 rc = PM_E_NONE;

    sal_memset(&domain, 0, sizeof(domain));
    domain.key.domain_id = domain_id;
    
    /* 2. add to DB */
    PM_E_RETURN(tbl_erps_domain_del_erps_domain(&domain.key));

    return rc;
}

int32
erps_get_check_erps_domain_ring_running(tbl_erps_domain_t *p_db_domain)
{
    ctclib_slistnode_t  *listnode       = NULL;
    ctclib_slistnode_t  *next           = NULL;
    tbl_erps_ring_t     *p_db_erps_ring = NULL;

    if (NULL == p_db_domain)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_db_domain->erps_ring_list.obj_list, p_db_erps_ring, listnode, next)
    {
        if (ERPS_IDLE != p_db_erps_ring->erps_ring_state)
        {
            return p_db_erps_ring->key.ring_id;
        }
    }

    return PM_E_NONE;
}

int32
erps_set_erps_domain_hello_time(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain)
{
    int32 rc = PM_E_NONE;

    /* update local CDB */
    PM_E_RETURN(tbl_erps_domain_set_erps_domain_field(p_domain, TBL_ERPS_DOMAIN_FLD_HELLO_TIMER_INTERVAL));
    
    return rc;
}

int32
erps_set_erps_domain_fail_time(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain)
{
    int32 rc = PM_E_NONE;

    /* update local CDB */
    PM_E_RETURN(tbl_erps_domain_set_erps_domain_field(p_domain, TBL_ERPS_DOMAIN_FLD_FAILURE_TIMER_INTERVAL));
    
    return rc;
}

int32
erps_set_erps_domain_instance(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain)
{
    int32 rc = PM_E_NONE;

    /* update local CDB */
    PM_E_RETURN(tbl_erps_domain_set_erps_domain_field(p_domain, TBL_ERPS_DOMAIN_FLD_INSTANCE_ID));

    return rc;
}

int32
erps_set_erps_domain_instance_num(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain)
{
    int32 rc = PM_E_NONE;

    /* update local CDB */
    PM_E_RETURN(tbl_erps_domain_set_erps_domain_field(p_domain, TBL_ERPS_DOMAIN_FLD_INSTANCE_ID_NUM));

    return rc;
}


/* modified by wangjj for fix erps bug 41473, 2016-12-03 */
int32
erps_get_erps_control_vlan_igsp_enable(vid_t vlanid)
{
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    tbl_igsp_intf_key_t igsp_if_key;

    sal_memset(&igsp_if_key, 0, sizeof(igsp_if_key));
    igsp_if_key.vid = vlanid;
    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&igsp_if_key);
    if (NULL != p_db_igsp_if && p_db_igsp_if->enable)
    {
        return TRUE;
    }

    return FALSE;
}

int32
erps_set_erps_domain_primary_control_vlan(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain)
{
    int32       rc = PM_E_NONE;
    tbl_vlan_t  vlan;

    sal_memset(&vlan, 0, sizeof(tbl_vlan_t));

    /* update vlan tbl */
    if (p_db_domain->primary_control_vlan_id)
    {
        vlan.key.vid = p_db_domain->primary_control_vlan_id;
        vlan.erps_domain_id = 0;
        PM_E_RETURN(tbl_vlan_set_vlan_field(&vlan, TBL_VLAN_FLD_ERPS_DOMAIN_ID));
    }

    if (p_domain->primary_control_vlan_id)
    {
        vlan.key.vid = p_domain->primary_control_vlan_id;
        /* modified by wangjj for fix erps bug 41056, 2016-11-11 */
        vlan.erps_domain_id = p_domain->key.domain_id;
        //vlan.erps_control_vlan = p_domain->primary_control_vlan_id;
        PM_E_RETURN(tbl_vlan_set_vlan_field(&vlan, TBL_VLAN_FLD_ERPS_DOMAIN_ID));
    }

    /* update erps domain tbl */
    PM_E_RETURN(tbl_erps_domain_set_erps_domain_field(p_domain, TBL_ERPS_DOMAIN_FLD_PRIMARY_CONTROL_VLAN_ID));

    usleep(20);

    return rc;
}

int32
erps_set_erps_domain_sub_control_vlan(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain)
{
    int32        rc = PM_E_NONE;
    tbl_vlan_t   vlan;
    
    sal_memset(&vlan, 0, sizeof(tbl_vlan_t));

    /* update vlan tbl */
    if (p_db_domain->sub_control_vlan_id)
    {
        vlan.key.vid = p_db_domain->sub_control_vlan_id;
        vlan.erps_domain_id = 0;
        PM_E_RETURN(tbl_vlan_set_vlan_field(&vlan, TBL_VLAN_FLD_ERPS_DOMAIN_ID));
    }

    if (p_domain->sub_control_vlan_id)
    {
        vlan.key.vid = p_domain->sub_control_vlan_id;
        /* modified by wangjj for fix erps bug 41056, 2016-11-11 */
        //vlan.erps_control_vlan = p_domain->sub_control_vlan_id;
        vlan.erps_domain_id = p_domain->key.domain_id;
        PM_E_RETURN(tbl_vlan_set_vlan_field(&vlan, TBL_VLAN_FLD_ERPS_DOMAIN_ID));
    }

    /* update local CDB */
    PM_E_RETURN(tbl_erps_domain_set_erps_domain_field(p_domain, TBL_ERPS_DOMAIN_FLD_SUB_CONTROL_VLAN_ID));

    usleep(20);

    return rc;
}

int32
erps_set_erps_domain_enable(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain)
{
    int32 rc = PM_E_NONE;

    /* update local CDB */
    PM_E_RETURN(tbl_erps_domain_set_erps_domain_field(p_domain, TBL_ERPS_DOMAIN_FLD_ERPS_ENABLE));
    
    return rc;
}

int32
erps_clear_erps_domain_instnace(uint32 instance_id)
{
    int32                      rc = PM_E_NONE;
    tbl_erps_domain_master_t   *p_master = NULL;
    uint32                     domain_id = 0;
    tbl_erps_domain_t          *p_db_domain = NULL;
    tbl_erps_domain_t          domain;    
    tbl_erps_ring_t            *p_erps_ring = NULL;
    ctclib_slistnode_t         *listnode = NULL;
    tbl_erps_ring_t            erps_ring;
    uint32                     index = 0;

    sal_memset(&domain, 0, sizeof(tbl_erps_domain_t));  
    sal_memset(&erps_ring, 0, sizeof(tbl_erps_ring_t));   
    
    /* when instance is deleted, it should be cleared in erps domain */ 
    p_master = tbl_erps_domain_get_master();
    for (domain_id = 1; domain_id < GLB_MAX_ERPS_DOMAIN; domain_id++)
    {
        p_db_domain = p_master->domain_array[domain_id];
        if (NULL == p_db_domain)
        {
            continue;
        }

        CTCLIB_SLIST_LOOP(p_db_domain->erps_ring_list.obj_list, p_erps_ring, listnode)
        {
            if (ERPS_IDLE != p_erps_ring->erps_ring_state)
            {
                erps_ring.key.domain_id = p_erps_ring->key.domain_id;
                erps_ring.key.ring_id = p_erps_ring->key.ring_id;
                erps_ring.erps_ring_state = ERPS_IDLE;
                
                rc = erps_cmd_set_erps_ring_enable(&erps_ring, p_erps_ring, NULL);
                if (rc)
                {
                    ERPS_LOG_ERR("Disable erps ring %u %u fail", p_erps_ring->key.domain_id,
                        p_erps_ring->key.ring_id);   
                }
            }
        }

        for (index = 0; index < p_db_domain->instance_id_num; index++)
        {
            if (p_db_domain->instance_id[index] == instance_id)
            {
                erps_check_one_domain_instnace(p_db_domain, p_db_domain,
                    instance_id, FALSE);
                
                rc = erps_set_erps_domain_instance(p_db_domain, p_db_domain);
                if (rc < 0)
                {
                    ERPS_LOG_ERR("Set erps domain instance fail, %s", pm_err_desc(rc));
                }  

                rc = erps_set_erps_domain_instance_num(p_db_domain, p_db_domain);
                if (rc < 0)
                {
                    ERPS_LOG_ERR("Set erps domain instance num fail, %s", pm_err_desc(rc));
                }

                break;
            }
        }
    }

    return rc;
}

uint32
erps_get_erps_domain_num(void)
{
    uint32                     num = 0;
    tbl_erps_domain_master_t   *p_master = NULL;
    uint32                     domain_id = 0;
    tbl_erps_domain_t          *p_db_domain = NULL;
    
    /*make sure the instance is not used by other domain*/ 
    p_master = tbl_erps_domain_get_master();
    for (domain_id = 1; domain_id < GLB_MAX_ERPS_DOMAIN; domain_id++)
    {
        p_db_domain = p_master->domain_array[domain_id];
        if (NULL == p_db_domain)
        {
            continue;
        }

        num++;
    }

    return num;
}

int32
erps_add_erps_ring(tbl_erps_domain_t *p_db_domain, uint32 ring_id, uint32 level)
{
    tbl_erps_ring_t     ring;
    tbl_erps_ring_t     *p_db_ring = NULL;
    int32               rc = PM_E_NONE;

    sal_memset(&ring, 0, sizeof(ring));
    ring.key.domain_id = p_db_domain->key.domain_id;
    ring.key.ring_id = ring_id;
 
    /* add to DB */
    PM_E_RETURN(tbl_erps_ring_add_erps_ring(&ring));

    p_db_ring = tbl_erps_ring_get_erps_ring(&ring.key);
    if (!p_db_ring)
    {
        return PM_E_NOT_EXIST;
    }

    /* set fld to default value */
    ring.erps_ring_level = level;
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(&ring, TBL_ERPS_RING_FLD_ERPS_RING_LEVEL));

    p_db_ring->p_erps_domain = (void*)p_db_domain;

    /* add erps ring to domain ring list */
    tbl_erps_domain_set_erps_ring_list(p_db_domain, TRUE, p_db_ring);

    if (ERPS_RING_PRIMARY == p_db_ring->erps_ring_level)
    {
        p_db_domain->erps_primary_ring = p_db_ring;
    }
    
    return rc;
}

int32
erps_del_erps_ring(tbl_erps_domain_t *p_db_domain, tbl_erps_ring_t *p_db_ring)
{
    int32 rc = PM_E_NONE;

    /* set erps ing for domain */
    tbl_erps_domain_set_erps_ring_list(p_db_domain, FALSE, p_db_ring);

    if (ERPS_RING_PRIMARY == p_db_ring->erps_ring_level)
    {
        p_db_domain->erps_primary_ring = NULL;
    }

    p_db_ring->p_erps_domain = NULL;
    
    /* 2. delete to DB */
    PM_E_RETURN(tbl_erps_ring_del_erps_ring(&p_db_ring->key));

    return rc;
}

int32
erps_set_erps_ring_node_role(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring)
{    
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_ring, TBL_ERPS_RING_FLD_NODE_ROLE));

    return rc;
}

int32
erps_set_erps_ring_edge_node_role(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring)
{    
    int32 rc = PM_E_NONE;

    if (ERPS_NODE_EDGE_UNKNOWN != p_ring->edge_node_role)
    {
        p_ring->node_role = ERPS_NODE_TRANSIT;     
        PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_ring, TBL_ERPS_RING_FLD_NODE_ROLE));

        p_ring->erps_srpt_enable = TRUE;
        PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_ring, TBL_ERPS_RING_FLD_ERPS_SRPT_ENABLE));
    }
    else
    {
        p_ring->node_role = ERPS_NODE_UNKNOWN;     
        PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_ring, TBL_ERPS_RING_FLD_NODE_ROLE));    
        
        p_ring->erps_srpt_enable = FALSE;
        PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_ring, TBL_ERPS_RING_FLD_ERPS_SRPT_ENABLE));
    }
    
    /* update local CDB */
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_ring, TBL_ERPS_RING_FLD_EDGE_NODE_ROLE));

    return rc;
}

int32
erps_set_erps_ring_srpt_enable(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring)
{    
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_ring, TBL_ERPS_RING_FLD_ERPS_SRPT_ENABLE));

    return rc;
}

int32
erps_set_erps_ring_state(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring)
{    
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_ring, TBL_ERPS_RING_FLD_ERPS_RING_STATE));

    return rc;
}

int32
erps_set_erps_ring_primary_interface(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring)
{    
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_ring, TBL_ERPS_RING_FLD_PRIMARY_PORT_IFINDEX));

    return rc;
}

int32
erps_set_erps_ring_secondary_interface(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring)
{    
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_ring, TBL_ERPS_RING_FLD_SECONDARY_PORT_IFINDEX));

    return rc;
}

int32
erps_set_erps_ring_edge_interface(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring)
{    
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_ring, TBL_ERPS_RING_FLD_EDGE_PORT_IFINDEX));

    return rc;
}

int32
erps_set_erps_ring_common_interface(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring)
{    
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_ring, TBL_ERPS_RING_FLD_COMMON_PORT_IFINDEX));

    return rc;
}

int32
erps_set_erps_ring_blocked_interface(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring, bool blocked)
{
    int32              rc = PM_E_NONE;
    tbl_erps_domain_t  *p_db_domain = NULL;
    tbl_interface_t    *p_db_if = NULL;
    char        ifname_ext[IFNAME_EXT_SIZE];

    sal_memset(ifname_ext, 0, sizeof(ifname_ext));
    p_db_domain = (tbl_erps_domain_t*)p_ring->p_erps_domain;

    if (p_db_domain)
    {
        if (p_db_domain->instance_id_num)
        {
            sal_memcpy(p_ring->instance_id, p_db_domain->instance_id, sizeof(uint32) * p_db_domain->instance_id_num);
            p_ring->instance_id_num = p_db_domain->instance_id_num;
        }
    }
    else
    {
        p_ring->instance_id[0] = GLB_ERPS_DOMAIN_INSTANCE_ID_DEFAULT;
        p_ring->instance_id_num = 0;
    }

    p_ring->blocked = blocked;
    p_db_if = p_ring->blocked_port_if;
    if (p_db_if)
    {
        /* modified by wangjj for fix erps bug 41043, 2016-11-10 */
        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
        if (blocked)
        {
            logid_sys(LOG_ERPS_4_BLOCK, ifname_ext, p_ring->instance_id[0], "blocking");
        }
        else
        {
            logid_sys(LOG_ERPS_4_BLOCK, ifname_ext, p_ring->instance_id[0], "unblock");
        }
    }

    /* set stp instance to fea */
    if (0 != p_ring->instance_id_num)
    {
        PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_ring, TBL_ERPS_RING_FLD_BLOCKED_PORT_IFINDEX));
    }
    
    /* update CDB */
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_ring, TBL_ERPS_RING_FLD_INSTANCE_ID));
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_ring, TBL_ERPS_RING_FLD_BLOCKED));

    return rc;
}

int32
erps_clear_erps_ring_counters(tbl_erps_ring_t  *p_db_ring)
{
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_db_ring, TBL_ERPS_RING_FLD_RCV_HELLO_CNT));
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_db_ring, TBL_ERPS_RING_FLD_RCV_RING_UP_FLUSH_FDB_CNT));
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_db_ring, TBL_ERPS_RING_FLD_RCV_RING_DOWN_FLUSH_FDB_CNT));
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_db_ring, TBL_ERPS_RING_FLD_RCV_LINK_DOWN_CNT));
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_db_ring, TBL_ERPS_RING_FLD_RCV_EDGE_HELLO_CNT));
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_db_ring, TBL_ERPS_RING_FLD_RCV_MAJOR_FAULT_CNT));

    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_db_ring, TBL_ERPS_RING_FLD_SEND_HELLO_CNT));
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_db_ring, TBL_ERPS_RING_FLD_SEND_RING_UP_FLUSH_FDB_CNT));
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_db_ring, TBL_ERPS_RING_FLD_SEND_RING_DOWN_FLUSH_FDB_CNT));
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_db_ring, TBL_ERPS_RING_FLD_SEND_LINK_DOWN_CNT));
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_db_ring, TBL_ERPS_RING_FLD_SEND_EDGE_HELLO_CNT));
    PM_E_RETURN(tbl_erps_ring_set_erps_ring_field(p_db_ring, TBL_ERPS_RING_FLD_SEND_MAJOR_FAULT_CNT));

    return rc;
}

bool
erps_ring_check_edge_node_role_mux(uint8 role, erps_node_check_t check_type)
{
    tbl_erps_ring_master_t  *p_master = tbl_erps_ring_get_master();
    tbl_erps_ring_t         *p_erps_ring = NULL;
    ctclib_slistnode_t      *listnode = NULL;  

    CTCLIB_SLIST_LOOP(p_master->erps_ring_list, p_erps_ring, listnode)
    {
        if ((ERPS_NODE_CHECK_MASTER == check_type) && (role == p_erps_ring->node_role))
        {
            return TRUE;    
        }

        if ((ERPS_NODE_CHECK_EDGE == check_type) && (role == p_erps_ring->edge_node_role))
        {
            return TRUE;    
        }
    }

    return FALSE;   
}

void erps_timer_hello_timer_fn(void *p_data)
{
    tbl_erps_ring_t    *p_db_ring = (tbl_erps_ring_t*)p_data;
    tbl_erps_domain_t  *p_db_domain = NULL;

    if (!p_db_ring)
    {
        ERPS_LOG_ERR("erps_timer_hello_timer_fn null ring in hello timer timeout\n");
        return;
    }
    p_db_ring->hello_timer = NULL;
    
    p_db_domain = (tbl_erps_domain_t*)p_db_ring->p_erps_domain;

    if (!p_db_domain)
    {
        ERPS_LOG_ERR("erps_timer_hello_timer_fn null domain in hello timer timeout\n");
        return;
    }

    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_TIMER, "Hello timer timeout for domain %u ring %u %s node in %s state\n",    
        p_db_ring->key.domain_id, p_db_ring->key.ring_id,
        /* modified by wangjj for fix erps bug 41093, 2016-11-21 */
        erps_desc_node_role(p_db_ring->node_role),
        //erps_desc_node_role(p_db_ring->edge_node_role),
        erps_desc_node_state(p_db_ring->erps_ring_state));
    
    erps_msg_send_pkt(ERPS_HEALTH_PKT, p_db_ring->primary_port_if, p_db_domain, p_db_ring);

    p_db_ring->hello_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_hello_timer_fn, p_db_ring, 
                        p_db_domain->hello_timer_interval);
    if (!p_db_ring->hello_timer)
    {
        ERPS_LOG_ERR("Add hello timer fail for domain %u ring %u", p_db_ring->key.domain_id, p_db_ring->key.ring_id);
    }
    
    return;
}

void erps_timer_failure_timer_fn(void *p_data)
{
    tbl_erps_ring_t     *p_db_ring = (tbl_erps_ring_t*)p_data;
    tbl_erps_domain_t   *p_db_domain = NULL;
    int32               rc = PM_E_NONE;

    if (!p_db_ring)
    {
        ERPS_LOG_ERR("erps_timer_failure_timer_fn null ring in failure timer timeout\n");
        return;
    }
    p_db_ring->failure_timer = NULL;
    
    p_db_domain = (tbl_erps_domain_t*)p_db_ring->p_erps_domain;
    if (!p_db_domain)
    {
        ERPS_LOG_ERR("erps_timer_failure_timer_fn null domain in failure timer timeout\n");
        return;
    }
    
    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_TIMER, "Failure timer timeout for domain %u ring %u %s node in %s state\n",    
        p_db_ring->key.domain_id, p_db_ring->key.ring_id, 
        /* modified by wangjj for fix erps bug 41093, 2016-11-21 */
        erps_desc_node_role(p_db_ring->node_role),
        //erps_desc_node_role(p_db_ring->edge_node_role),
        erps_desc_node_state(p_db_ring->erps_ring_state));

    switch (p_db_ring->erps_ring_state)
    {
    case ERPS_INIT:
    case ERPS_COMPLETE :
        /* unblock secondary port */  
        if (p_db_ring->blocked_port_if)
        {
            erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);
            p_db_ring->blocked_port_if = NULL; 
            p_db_ring->blocked_port_ifindex = 0;
        }
        
        /* flush fdb */
        //erps_fdb_flush_dynamic_fdb_port(p_db_ring->primary_port_ifindex);              
        //erps_fdb_flush_dynamic_fdb_port(p_db_ring->secondary_port_ifindex);
        erps_fdb_flush_dynamic_fdb_all();

        /* send flush fdb packet */
        erps_msg_send_pkt(ERPS_RING_DOWN_FLUSH_FDB_PKT, p_db_ring->primary_port_if, p_db_domain, p_db_ring);
        erps_msg_send_pkt(ERPS_RING_DOWN_FLUSH_FDB_PKT, p_db_ring->secondary_port_if, p_db_domain, p_db_ring);            

        /* change erps ring state to failed */
        p_db_ring->erps_ring_state = ERPS_FAILED;
        rc = erps_set_erps_ring_state(p_db_ring, p_db_ring);
        if (rc < 0)
        {
            ERPS_LOG_ERR("erps_timer_failure_timer_fn set ring %d failed\n", p_db_ring->key.ring_id);
        }
        break;
        
    default:
        break;
    }      
    
    return;
}

void
erps_timer_failure_timeout_pre_fn(void *p_data)
{
    tbl_erps_ring_t     *p_db_ring = (tbl_erps_ring_t*)p_data;
    tbl_erps_domain_t   *p_db_domain = NULL;

    if (!p_db_ring)
    {
        ERPS_LOG_ERR("erps_timer_failure_timeout_pre_fn null ring in failure timer timeout\n");
        return;
    }
    p_db_ring->failure_timer = NULL;
    
    p_db_domain = (tbl_erps_domain_t*)p_db_ring->p_erps_domain;
    if (!p_db_domain)
    {
        ERPS_LOG_ERR("erps_timer_failure_timeout_pre_fn null domain in failure timer timeout\n");
        return;
    }
  
    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_TIMER, "Failure pre-timer timeout for domain %u ring %u %s node in %s state\n",    
        p_db_ring->key.domain_id, p_db_ring->key.ring_id,
        /* modified by wangjj for fix erps bug 41093, 2016-11-21 */
        erps_desc_node_role(p_db_ring->node_role),
        //erps_desc_node_role(p_db_ring->edge_node_role),
        erps_desc_node_state(p_db_ring->erps_ring_state));

    p_db_ring->failure_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_failure_timer_fn, 
                    p_db_ring, 1);

    return;
}

void erps_timer_edge_hello_timer_fn(void *p_data)
{
    tbl_erps_ring_t     *p_db_ring = (tbl_erps_ring_t*)p_data;
    tbl_erps_domain_t   *p_db_domain = NULL;
    tbl_erps_ring_t     *p_primary_ring = NULL;

    if (!p_db_ring)
    {
        ERPS_LOG_ERR("erps_timer_edge_hello_timer_fn null ring in edge hello timer timeout\n");
        return;
    }
    p_db_ring->edge_hello_timer = NULL;
    
    p_db_domain = (tbl_erps_domain_t*)p_db_ring->p_erps_domain;
    if (!p_db_domain)
    {
        ERPS_LOG_ERR("erps_timer_edge_hello_timer_fn null domain in edge hello timer timeout\n");
        return;
    }
   
    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_TIMER, "Edge hello timer timeout for domain %u ring %u %s node in %s state\n",    
            p_db_ring->key.domain_id, p_db_ring->key.ring_id, erps_desc_node_role(p_db_ring->edge_node_role),
            erps_desc_node_state(p_db_ring->erps_ring_state));

    if ((ERPS_NODE_EDGE == p_db_ring->edge_node_role) && (p_db_ring->erps_srpt_enable))
    {
        if (p_db_domain->erps_primary_ring)
        {
            p_primary_ring = (tbl_erps_ring_t*)p_db_domain->erps_primary_ring;

            if (p_primary_ring->primary_port_if)
            {
                erps_msg_send_pkt(ERPS_EDGE_HELLO_PKT, p_primary_ring->primary_port_if, p_db_domain, p_db_ring);
            }

            if (p_primary_ring->secondary_port_if)
            {
                erps_msg_send_pkt(ERPS_EDGE_HELLO_PKT, p_primary_ring->secondary_port_if, p_db_domain, p_db_ring);
            }
        }

        p_db_ring->edge_hello_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_edge_hello_timer_fn, p_db_ring, 
                                p_db_domain->edge_hello_timer_interval);

        if (!p_db_ring->edge_hello_timer)
        {
            ERPS_LOG_ERR("Add edge hello timer fail for domain %u ring %u", p_db_ring->key.domain_id, p_db_ring->key.ring_id);
            return;
        }
    }
    
    return;
}

void erps_timer_edge_failure_timer_fn(void *p_data)
{
    tbl_erps_ring_t    *p_db_ring = (tbl_erps_ring_t*)p_data;
    tbl_erps_domain_t  *p_db_domain = NULL;

    if (!p_db_ring)
    {
        ERPS_LOG_ERR("erps_timer_edge_failure_timer_fn null ring in edge failure timer timeout\n");
        return;
    }
    p_db_ring->edge_failure_timer = NULL;
    
    p_db_domain = (tbl_erps_domain_t*)p_db_ring->p_erps_domain;
    if (!p_db_domain)
    {
        ERPS_LOG_ERR("erps_timer_edge_failure_timer_fn null domain in edge failure timer timeout\n");
        return;
    }
    
    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_TIMER, "Edge failure timer timeout for domain %u ring %u %s node in %s state\n",    
            p_db_ring->key.domain_id, p_db_ring->key.ring_id, erps_desc_node_role(p_db_ring->edge_node_role),
            erps_desc_node_state(p_db_ring->erps_ring_state));

    if ((ERPS_NODE_ASSI_EDGE == p_db_ring->edge_node_role)&& (p_db_ring->erps_srpt_enable))
    {
        erps_msg_send_pkt(ERPS_MAJOR_FAULT_PKT, p_db_ring->primary_port_if, p_db_domain, p_db_ring);
        erps_msg_send_pkt(ERPS_MAJOR_FAULT_PKT, p_db_ring->secondary_port_if, p_db_domain, p_db_ring);
         
        p_db_ring->edge_failure_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_edge_failure_timer_fn, p_db_ring, 
            p_db_domain->edge_failure_timer_interval); 

        if (!p_db_ring->edge_failure_timer)
        {
            ERPS_LOG_ERR("Add edge failure timer fail for domain %u ring %u", 
                p_db_ring->key.domain_id, p_db_ring->key.ring_id);
            return;
        }
    }
    
    return;
}

void erps_timer_edge_block_timer_fn(void *p_data)
{
    tbl_erps_ring_t     *p_db_ring = (tbl_erps_ring_t*)p_data;
    tbl_erps_domain_t   *p_db_domain = NULL;

    if (!p_db_ring)
    {
        ERPS_LOG_ERR("erps_timer_edge_failure_timer_fn null ring in edge failure timer timeout\n");
        return;
    }
    p_db_ring->edge_block_timer = NULL;
    
    p_db_domain = (tbl_erps_domain_t*)p_db_ring->p_erps_domain;
    if (!p_db_domain)
    {
        ERPS_LOG_ERR("erps_timer_edge_failure_timer_fn null domain in edge failure timer timeout\n");
        return;
    }
   
    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_TIMER, "Edge block timer timeout for domain %u ring %u %s node in %s state\n",    
            p_db_ring->key.domain_id, p_db_ring->key.ring_id, erps_desc_node_role(p_db_ring->edge_node_role),
            erps_desc_node_state(p_db_ring->erps_ring_state));

    if (p_db_ring->blocked_port_if)
    {
        erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);
        p_db_ring->blocked_port_if = NULL; 
        p_db_ring->blocked_port_ifindex = 0;
    }
    
    return;
}

void erps_timer_pre_forwarding_timer_fn(void *p_data)
{
    tbl_erps_ring_t     *p_db_ring = (tbl_erps_ring_t*)p_data;
    tbl_erps_domain_t   *p_db_domain = NULL;

    if (!p_db_ring)
    {
        ERPS_LOG_ERR("erps_timer_pre_forwarding_timer_fn null ring in preforwarding timer timeout\n");
        return;
    }
    p_db_ring->pre_forwarding_timer = NULL;
    
    p_db_domain = (tbl_erps_domain_t*)p_db_ring->p_erps_domain;
    if (!p_db_domain)
    {
        ERPS_LOG_ERR("erps_timer_pre_forwarding_timer_fn null domain in preforwarding timer timeout\n");
        return;
    }
    
    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_TIMER, "Pre forwarding timer timeout for domain %u ring %u %s node in %s state\n",    
            p_db_ring->key.domain_id, p_db_ring->key.ring_id, erps_desc_node_role(p_db_ring->edge_node_role),
            erps_desc_node_state(p_db_ring->erps_ring_state));

    switch (p_db_ring->erps_ring_state)
    { 
    case ERPS_PREFORWORDING :
        if (p_db_ring->blocked_port_if)
        {
            erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);
            p_db_ring->blocked_port_if = NULL; 
            p_db_ring->blocked_port_ifindex = 0;
        }
        
        p_db_ring->erps_ring_state = ERPS_LINK_UP;
        
        ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PROTO, "ERPS domain %d ring %d state switch to %s\n", p_db_ring->key.domain_id,
                p_db_ring->key.ring_id, erps_desc_node_state(p_db_ring->erps_ring_state));
        break;

    case ERPS_FAILED:
        if (p_db_ring->blocked_port_if)
        {
            erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);
            p_db_ring->blocked_port_if = NULL; 
            p_db_ring->blocked_port_ifindex = 0;
        }
        break;
    default:
        break;
    }

    return;
}

/* send erps msg to specific interface  */
void
erps_msg_send_pkt(erps_msg_type_t type, tbl_interface_t* p_if, tbl_erps_domain_t *p_domain, tbl_erps_ring_t *p_ring)
{
    erps_message_t     erps_msg;   
    tbl_sys_global_t   *pGlobal = tbl_sys_global_get_sys_global();

    if (!p_if)
    {
        ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PACKET, "invalid port when send erps pkt");
        return;
    }

    if (!tbl_interface_is_running(p_if))
    {
        ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PACKET, "send msg through port %d fail for domain %u ring %u, the port is not up.", 
            p_if->ifindex, p_domain->key.domain_id, p_ring->key.ring_id);
        return;
    }
    
    sal_memset(&erps_msg, 0, sizeof(erps_message_t));

    /* construct erps msg */
    if (!erps_mode_is_huawei())
    {
        if ((ERPS_HEALTH_PKT == type) || (ERPS_LINK_DOWN_PKT == type)
            || (ERPS_RING_DOWN_FLUSH_FDB_PKT == type) || (ERPS_RING_UP_FLUSH_FDB_PKT == type))
        {
            
            sal_memcpy(&erps_msg.dst_mac, erps_addr1, sizeof(erps_msg.dst_mac));
        }
        else if ((ERPS_EDGE_HELLO_PKT == type) || (ERPS_MAJOR_FAULT_PKT == type))
        {
            sal_memcpy(&erps_msg.dst_mac, erps_addr2, sizeof(erps_msg.dst_mac));
        }
    }
    else
    {
        if (ERPS_HEALTH_PKT == type)
        {
            sal_memcpy(&erps_msg.dst_mac, erps_addr1_hw, sizeof(erps_msg.dst_mac));        
        }
        else if (ERPS_LINK_DOWN_PKT == type)
        {
            sal_memcpy(&erps_msg.dst_mac, erps_addr2_hw, sizeof(erps_msg.dst_mac));        
        }
        else if ((ERPS_RING_DOWN_FLUSH_FDB_PKT == type) || (ERPS_RING_UP_FLUSH_FDB_PKT == type))
        {
            sal_memcpy(&erps_msg.dst_mac, erps_addr3_hw, sizeof(erps_msg.dst_mac));
        }
        else if ((ERPS_EDGE_HELLO_PKT == type) || (ERPS_MAJOR_FAULT_PKT == type))
        {
            sal_memcpy(&erps_msg.dst_mac, erps_addr4_hw, sizeof(erps_msg.dst_mac));
        }     
    }
    
    sal_memcpy(&erps_msg.src_mac, p_if->mac_addr, sizeof(erps_msg.src_mac));
    erps_msg.eth_type = sal_hton16(0x8100);
    if (ERPS_RING_PRIMARY == p_ring->erps_ring_level)
    {
        erps_msg.pri_vlan_id = sal_hton16((((uint16_t)ERPS_PRI_MAX)<<12) 
            + ((p_domain->primary_control_vlan_id) & 0x0fff));
    }  
    else
    {
        erps_msg.pri_vlan_id = sal_hton16((((uint16_t)ERPS_PRI_MAX)<<12) 
            + ((p_domain->sub_control_vlan_id) & 0x0fff));
    }    
    erps_msg.frame_len = sal_hton16(0x48);
    erps_msg.dsap = sal_hton16(0xaaaa);
    erps_msg.control = 0x03;
    erps_msg.oui[0] = 0x00;
    erps_msg.oui[1] = 0xe0;
    erps_msg.oui[2] = 0x2b;
    erps_msg.rub1 = sal_hton32(0x00bb990b);
    erps_msg.erps_len = sal_hton16(0x0040);
    erps_msg.erps_ver = 0x01;
    erps_msg.erps_type = (uint8_t)type;      
    erps_msg.domain_id = sal_hton16(p_domain->key.domain_id);
    erps_msg.ring_id = sal_hton16(p_ring->key.ring_id);
    sal_memcpy(&erps_msg.sys_mac, pGlobal->route_mac, sizeof(erps_msg.sys_mac));
    erps_msg.h_timer = ERPS_SECS_TO_TICKS(sal_hton16(p_domain->hello_timer_interval));
    erps_msg.f_timer = ERPS_SECS_TO_TICKS(sal_hton16(p_domain->failure_timer_interval));
    erps_msg.state = 0;
    erps_msg.level = (uint8)(p_ring->erps_ring_level);
    erps_msg.h_seq = sal_hton16(p_ring->hello_seq);
    
    /* send pkt through erps socket to specific interface */
    if (ipc_pkt_tx_erps((uint8*)&erps_msg, sizeof(erps_message_t), p_if->ifindex))
    {
        ERPS_LOG_ERR("send msg through port %d fail for domain %u ring %u", 
            p_if->ifindex, p_domain->key.domain_id, p_ring->key.ring_id);
        return;
    }   

    p_ring->hello_seq++;

    switch (type)
    {
    case ERPS_HEALTH_PKT:
        p_ring->send_hello_cnt++;
        break;
    case ERPS_RING_UP_FLUSH_FDB_PKT:
        p_ring->send_ring_up_flush_fdb_cnt++;
        break;
    case ERPS_RING_DOWN_FLUSH_FDB_PKT:
        p_ring->send_ring_down_flush_fdb_cnt++;
        break;
    case ERPS_LINK_DOWN_PKT:
        p_ring->send_link_down_cnt++;
        break;    
    case ERPS_EDGE_HELLO_PKT:
        p_ring->send_edge_hello_cnt++;
        break;
    case ERPS_MAJOR_FAULT_PKT:
        p_ring->send_major_fault_cnt++;
        break;      
    default:
        break;
    }

    if (ERPS_LINK_DOWN_PKT == type)
    {
        usleep(20000); /*sleep 20ms*/
    }
        
    return;
}


/* rcv health check pkt */
static void
_erps_msg_rcv_health_pkt(tbl_erps_domain_t  *p_db_domain, tbl_erps_ring_t  *p_db_ring, u_int32_t hello_timer_interval)
{
    if (!p_db_domain)
    {
        ERPS_LOG_ERR("null domain in rcv health check func");
        return;
    } 

    if (!p_db_ring)
    {
        ERPS_LOG_ERR("null ring in rcv health check func");
        return;
    } 

    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PACKET, "receive health check msg for domain %u ring %u %s node in %s state", 
        p_db_domain->key.domain_id, p_db_ring->key.ring_id,
        erps_desc_node_role(p_db_ring->node_role),
        erps_desc_node_state(p_db_ring->erps_ring_state)); 

    p_db_ring->rcv_hello_cnt++;    

    if (ERPS_NODE_EDGE_UNKNOWN != p_db_ring->edge_node_role)
    {
        p_db_domain->edge_hello_timer_interval = hello_timer_interval;
        if (0 == p_db_domain->edge_hello_timer_interval)
        {
            p_db_domain->edge_hello_timer_interval = GLB_ERPS_DOMAIN_HELLO_TIME_DEFAULT;
        }    
        p_db_domain->edge_failure_timer_interval = p_db_domain->edge_hello_timer_interval * 2;
    }

    switch (p_db_ring->erps_ring_state)
    {   
    case ERPS_INIT:
        p_db_ring->erps_ring_state = ERPS_COMPLETE; 
        
    case ERPS_COMPLETE:
        /*if (p_db_ring->failure_timer)
        {
            ctc_task_delete_timer(p_db_ring->failure_timer);
            p_db_ring->failure_timer = NULL;
        }*/

        CTC_TASK_STOP_TIMER(p_db_ring->failure_timer);
        
        p_db_ring->failure_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_failure_timer_fn, 
            p_db_ring, (p_db_domain->failure_timer_interval - 1));
        
        if (!p_db_ring->failure_timer)
        {
            ERPS_LOG_ERR("failure timer set fail");
        }
        break;
     
    case ERPS_FAILED :
        /* check master node link state before transitting into complete state */
        if ((!tbl_interface_is_running((tbl_interface_t*)p_db_ring->primary_port_if))
            || (!tbl_interface_is_running((tbl_interface_t*)p_db_ring->secondary_port_if)))
        {
            ERPS_LOG_ERR("ERPS receive hello pkt in failed state for domain %d ring %d", p_db_ring->key.domain_id,
                p_db_ring->key.ring_id);
            break;
        }
        
        /* there may be one port blocked in failed state to avoid broadcasting storm */
        /*unblock when the port is down : old -----if (erps_ring->erps_node.tmp_blocked_port_if) */
        if (p_db_ring->blocked_port_if && (p_db_ring->blocked_port_if != p_db_ring->secondary_port_if))
        {
            /*if primary port from block to unblock and the second port is unblock also , that may cause loop*/
            /*so first block second port ,then unblock primary port */
            /* use blocked_port_if in master node to remember the state of secondary port interface */
            p_db_ring->blocked_port_if = p_db_ring->secondary_port_if; 
            p_db_ring->blocked_port_ifindex = p_db_ring->secondary_port_ifindex;
            erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, TRUE);

            p_db_ring->blocked_port_if = p_db_ring->primary_port_if; 
            p_db_ring->blocked_port_ifindex = p_db_ring->primary_port_ifindex;
            erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);
        }

        /*if (p_db_ring->pre_forwarding_timer)
        {
            ctc_task_delete_timer(p_db_ring->pre_forwarding_timer);
            p_db_ring->pre_forwarding_timer = NULL;
        }*/
        CTC_TASK_STOP_TIMER(p_db_ring->pre_forwarding_timer);
        
        p_db_ring->blocked_port_if = p_db_ring->secondary_port_if; 
        p_db_ring->blocked_port_ifindex = p_db_ring->secondary_port_ifindex;
        erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, TRUE);
        
        /* block after send flush fdb pkt on high cpu usage */
        usleep(5000); /*sleep 5ms*/
        
        erps_msg_send_pkt(ERPS_RING_UP_FLUSH_FDB_PKT, p_db_ring->primary_port_if, p_db_domain, p_db_ring);
        erps_msg_send_pkt(ERPS_RING_UP_FLUSH_FDB_PKT, p_db_ring->secondary_port_if, p_db_domain, p_db_ring);

        //usleep(5000); /*sleep 5ms*/

        /* flush fdb */
        //erps_fdb_flush_dynamic_fdb_port(p_db_ring->primary_port_ifindex);              
        //erps_fdb_flush_dynamic_fdb_port(p_db_ring->secondary_port_ifindex);
        erps_fdb_flush_dynamic_fdb_all();

        p_db_ring->failure_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_failure_timer_fn, 
                    p_db_ring, (p_db_domain->failure_timer_interval - 1));
        if (!p_db_ring->failure_timer)
        {
            ERPS_LOG_ERR("failure timer set fail");
        }

        p_db_ring->erps_ring_state = ERPS_COMPLETE;

        ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PROTO, "ERPS domain %d ring %d state switch to %s\n", p_db_ring->key.domain_id,
                p_db_ring->key.ring_id, erps_desc_node_state(p_db_ring->erps_ring_state));
        break;
        
    default:
        break;
    }   

    return;                                                                
}

/* rcv ring up flush fdb pkt */
static void
_erps_msg_rcv_ring_up_flush_fdb_pkt(tbl_erps_domain_t  *p_db_domain, tbl_erps_ring_t  *p_db_ring)
{
    if (!p_db_domain)
    {
        ERPS_LOG_ERR("NULL domain in rcv ring up flush fdb func");
        return;
    } 

    if (!p_db_ring)
    {
        ERPS_LOG_ERR("NULL ring in rcv ring up flush fdb func");
        return;
    }

    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PACKET, "receive ring up flush fdb msg for domain %u ring %u %s node in %s state", 
        p_db_domain->key.domain_id, p_db_ring->key.ring_id,
        erps_desc_node_role(p_db_ring->node_role),
        erps_desc_node_state(p_db_ring->erps_ring_state)); 

    p_db_ring->rcv_ring_up_flush_fdb_cnt++;

    switch (p_db_ring->erps_ring_state)
    {
    case ERPS_LINK_UP:       
        if (ERPS_NODE_EDGE == p_db_ring->edge_node_role)
        {
            if (p_db_ring->blocked_port_if == p_db_ring->primary_port_if)
            {
                erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);
                p_db_ring->blocked_port_if = NULL; 
                p_db_ring->blocked_port_ifindex = 0;
            }
        }    

        //flush_p_if = p_db_ring->primary_port_if;
        //flush_s_if = p_db_ring->secondary_port_if;
        erps_fdb_flush_dynamic_fdb_all();
        break;
        
    case ERPS_PREFORWORDING :
        /*if (p_db_ring->pre_forwarding_timer)
        {
            ctc_task_delete_timer(p_db_ring->pre_forwarding_timer);
            p_db_ring->pre_forwarding_timer = NULL;
        }*/
        CTC_TASK_STOP_TIMER(p_db_ring->pre_forwarding_timer);

        if (p_db_ring->blocked_port_if)
        {
            erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);
            p_db_ring->blocked_port_if = NULL; 
            p_db_ring->blocked_port_ifindex = 0;
        }

        //flush_p_if = p_db_ring->primary_port_if;
        //flush_s_if = p_db_ring->secondary_port_if;
        erps_fdb_flush_dynamic_fdb_all();

        p_db_ring->erps_ring_state = ERPS_LINK_UP;
        
        ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PROTO, "ERPS domain %d ring %d state switch to %s\n", p_db_ring->key.domain_id,
                p_db_ring->key.ring_id, erps_desc_node_state(p_db_ring->erps_ring_state));
        break;

    /*Master node receive ring_up_flush_fdb in preforwarding state*/
    case ERPS_FAILED : 
        if (NULL == p_db_ring->blocked_port_if)
        {
            break;
        }

        /*if (p_db_ring->pre_forwarding_timer)
        {
            ctc_task_delete_timer(p_db_ring->pre_forwarding_timer);
            p_db_ring->pre_forwarding_timer = NULL;
        }*/
        CTC_TASK_STOP_TIMER(p_db_ring->pre_forwarding_timer);

        if (p_db_ring->blocked_port_if)
        {
            erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);
            p_db_ring->blocked_port_if = NULL; 
            p_db_ring->blocked_port_ifindex = 0;
        }

        //flush_p_if = p_db_ring->primary_port_if;
        //flush_s_if = p_db_ring->secondary_port_if;
        erps_fdb_flush_dynamic_fdb_all();

        p_db_ring->erps_ring_state = ERPS_COMPLETE;
        
        ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PROTO, "ERPS domain %d ring %d state switch to %s\n", p_db_ring->key.domain_id,
                p_db_ring->key.ring_id, erps_desc_node_state(p_db_ring->erps_ring_state));
        break;
        
    case ERPS_COMPLETE :
        break;
         
    default:
        break;
    }
 
    return;
}

/* master node rcv link down pkt from transit node*/
static void
_erps_msg_rcv_link_down_pkt(tbl_erps_domain_t  *p_db_domain, tbl_erps_ring_t  *p_db_ring)
{
    if (!p_db_domain)
    {
        ERPS_LOG_ERR("NULL domain in rcv link down pkt func");
        return;
    } 

    if (!p_db_ring)
    {
        ERPS_LOG_ERR("NULL ring in rcv link down pkt func");
        return;
    }

    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PACKET, "receive link down msg for domain %u ring %u %s node in %s state", 
        p_db_domain->key.domain_id, p_db_ring->key.ring_id,
        erps_desc_node_role(p_db_ring->node_role),
        erps_desc_node_state(p_db_ring->erps_ring_state)); 

    p_db_ring->rcv_link_down_cnt++;

    switch (p_db_ring->erps_ring_state)
    {   
        case ERPS_INIT:
        case ERPS_COMPLETE :
            /* need to stop failure timer before transiting into failed state */
            /*if (p_db_ring->failure_timer)
            {
                ctc_task_delete_timer(p_db_ring->failure_timer);
                p_db_ring->failure_timer = NULL;
            }*/
            CTC_TASK_STOP_TIMER(p_db_ring->failure_timer);

            if (p_db_ring->blocked_port_if)
            {
                erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);
                p_db_ring->blocked_port_if = NULL; 
                p_db_ring->blocked_port_ifindex = 0;
            }
       
            erps_msg_send_pkt(ERPS_RING_DOWN_FLUSH_FDB_PKT, p_db_ring->primary_port_if, p_db_domain, p_db_ring);
            erps_msg_send_pkt(ERPS_RING_DOWN_FLUSH_FDB_PKT, p_db_ring->secondary_port_if, p_db_domain, p_db_ring);

            //usleep(5000);/*sleep 5ms*/

            /* flush fdb */
            //erps_fdb_flush_dynamic_fdb_port(p_db_ring->primary_port_ifindex);              
            //erps_fdb_flush_dynamic_fdb_port(p_db_ring->secondary_port_ifindex);       
            erps_fdb_flush_dynamic_fdb_all();
            
            p_db_ring->erps_ring_state = ERPS_FAILED;
            
            ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PROTO, "ERPS domain %d ring %d state switch to %s\n", p_db_ring->key.domain_id,
                p_db_ring->key.ring_id, erps_desc_node_state(p_db_ring->erps_ring_state));
   
            break;
            
        default:
            break;
               
    }       
    
    return;
}

/* rcv ring down flush fdb pkt */
static void
_erps_msg_rcv_ring_down_flush_fdb_pkt(tbl_erps_domain_t  *p_db_domain, tbl_erps_ring_t  *p_db_ring)
{
    //tbl_erps_ring_t  *p_ring = NULL;
    
    if (!p_db_domain)
    {
        ERPS_LOG_ERR("NULL domain in rcv ring down flush pkt func");
        return;
    } 

    if (!p_db_ring)
    {
        ERPS_LOG_ERR("NULL ring in rcv ring down flush pkt func");
        return;
    }

    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PACKET, "receive ring down flush fdb msg for domain %u ring %u %s node in %s state", 
        p_db_domain->key.domain_id, p_db_ring->key.ring_id,
        erps_desc_node_role(p_db_ring->node_role),
        erps_desc_node_state(p_db_ring->erps_ring_state)); 

    p_db_ring->rcv_ring_down_flush_fdb_cnt++;
 
    switch (p_db_ring->erps_ring_state)
    {
    case ERPS_LINK_DOWN:
    case ERPS_LINK_UP:
        /* flush fdb */
        erps_fdb_flush_dynamic_fdb_all();
#if 0
        erps_fdb_flush_dynamic_fdb_port(p_db_ring->primary_port_ifindex);              
        erps_fdb_flush_dynamic_fdb_port(p_db_ring->secondary_port_ifindex);       
            
        if ((ERPS_NODE_EDGE_UNKNOWN != p_db_ring->edge_node_role) 
            && (p_db_domain->erps_primary_ring))
        {
            p_ring = (tbl_erps_ring_t*)p_db_domain->erps_primary_ring;
            if (ERPS_IDLE != p_ring->erps_ring_state)
            {
                /*need to flush primary ring's fdb*/
                erps_fdb_flush_dynamic_fdb_port(p_ring->primary_port_ifindex);              
                erps_fdb_flush_dynamic_fdb_port(p_ring->secondary_port_ifindex);                              
            }
        }
#endif
        break;
        
    default:
        break;
    }
    
    return;
}

/* rcv edge hello pkt */
static void
_erps_msg_rcv_edge_hello_pkt(tbl_erps_domain_t  *p_db_domain, tbl_erps_ring_t  *p_db_ring)
{
    if (!p_db_domain)
    {
        ERPS_LOG_ERR("null domain in rcv edge hello func");
        return;
    } 

    if (!p_db_ring)
    {
        ERPS_LOG_ERR("null ring in rcv edge hello func");
        return;
    }

    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PACKET, "receive edge hello msg for domain %u ring %u %s node in %s state", 
            p_db_domain->key.domain_id, p_db_ring->key.ring_id,
            /* modified by wangjj for fix erps bug 41093, 2016-11-21 */
            erps_desc_node_role(p_db_ring->edge_node_role),
            //erps_desc_node_role(p_db_ring->node_role),
            erps_desc_node_state(p_db_ring->erps_ring_state)); 

    if (ERPS_NODE_ASSI_EDGE == p_db_ring->edge_node_role)
    {
        p_db_ring->rcv_edge_hello_cnt++;
        
        if (!p_db_ring->erps_srpt_enable)
        {
            return;
        }

        /*if (p_db_ring->edge_failure_timer)
        {
            ctc_task_delete_timer(p_db_ring->edge_failure_timer);
            p_db_ring->edge_failure_timer = NULL;
        }*/
        CTC_TASK_STOP_TIMER(p_db_ring->edge_failure_timer);

        p_db_ring->edge_failure_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_edge_failure_timer_fn, 
            p_db_ring, p_db_domain->edge_failure_timer_interval); 

        if (!p_db_ring->edge_failure_timer)
        {
            ERPS_LOG_ERR("Add edge failure timer fail for domain %u ring %u", 
                p_db_ring->key.domain_id, p_db_ring->key.ring_id);
            return;
        }
    }

    return;
}

/* rcv edge hello pkt */
static void
_erps_msg_rcv_major_fault_pkt(tbl_erps_domain_t  *p_db_domain, tbl_erps_ring_t  *p_db_ring)
{
    if (!p_db_domain)
    {
        ERPS_LOG_ERR("null domain in rcv edge hello func");
        return;
    } 

    if (!p_db_ring)
    {
        ERPS_LOG_ERR("null ring in rcv edge hello func");
        return;
    }

    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PACKET, "receive major fault msg for domain %u ring %u %s node in %s state", 
            p_db_domain->key.domain_id, p_db_ring->key.ring_id,
            /* modified by wangjj for fix erps bug 41093, 2016-11-21 */
            erps_desc_node_role(p_db_ring->edge_node_role),
            //erps_desc_node_role(p_db_ring->node_role),
            erps_desc_node_state(p_db_ring->erps_ring_state)); 

    if (ERPS_NODE_EDGE == p_db_ring->edge_node_role)
    {
        p_db_ring->rcv_major_fault_cnt++;

        if (!p_db_ring->erps_srpt_enable)
        {
            return;
        }
        
        /* block edge port and set timeout timer */
        if (!p_db_ring->blocked_port_if)
        {
            p_db_ring->blocked_port_if = p_db_ring->primary_port_if; 
            p_db_ring->blocked_port_ifindex = p_db_ring->primary_port_ifindex;
            erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, TRUE);
            /* modified by liwh for bug 43154, 2017-03-27 */
            erps_fdb_flush_dynamic_fdb_port(p_db_ring->primary_port_ifindex);
            /* liwh end */
        }
        
        /*if (p_db_ring->edge_block_timer)
        {
            ctc_task_delete_timer(p_db_ring->edge_block_timer);
            p_db_ring->edge_block_timer = NULL;
        }*/ 
        CTC_TASK_STOP_TIMER(p_db_ring->edge_block_timer);

        p_db_ring->edge_block_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_edge_block_timer_fn, 
            p_db_ring, p_db_domain->pre_forwarding_timer_interval);

        if (!p_db_ring->edge_block_timer)
        {
            ERPS_LOG_ERR("Add edge block timer fail for domain %u ring %u", 
                p_db_ring->key.domain_id, p_db_ring->key.ring_id);
            return;
        }
    }

    return;
}

static uint16
_erps_msg_dispatcher(ctc_msg_t *p_msg)
{
    uint16                 vlan_id = 0;
    erps_message_t         *p_erps_msg = NULL;
    tbl_erps_domain_t      *p_db_domain = NULL;
    tbl_erps_domain_key_t  domain_key;
    tbl_erps_ring_key_t    ring_key;
    tbl_erps_ring_t        *p_db_ring = NULL;
    tbl_interface_t        *p_rcv_if = NULL;
    tbl_erps_ring_t        *p_domain_primary_ring = NULL;
    tbl_sys_global_t       *pGlobal = tbl_sys_global_get_sys_global();
    erps_state_t           old_state = 0;
    tbl_interface_t        *p_db_if_lag = NULL;
    tbl_interface_key_t    if_key;
    char    buf[CMD_BUF_256];
    char    buf1[CMD_BUF_256];

    p_erps_msg = (erps_message_t*)p_msg->data;

    vlan_id = sal_ntoh16(p_erps_msg->pri_vlan_id) & 0x0FFF;

    p_rcv_if = tbl_interface_get_interface_by_ifindex(p_msg->u_hdr.pkt.ifindex);
    if (!p_rcv_if)
    {
        ERPS_LOG_ERR("_erps_msg_dispatcher interface %d does not exist at all", p_msg->u_hdr.pkt.ifindex);
        return PM_E_NONE;
    }

    /* modified by wangjj for fix erps bug 41093, 2016-12-01 */
    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf1, 0, sizeof(buf1));

    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PACKET, "ERPS receive msg from interface %s", p_rcv_if->key.name);
    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PACKET, "dst mac is %s, src mac is %s, eth_type is is %x, vlan id is %u, frame_len is %u, erps_len is %u",
            cdb_mac_addr_val2str(buf, CMD_BUF_256, p_erps_msg->dst_mac),
            cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_erps_msg->src_mac), 
            sal_ntoh16(p_erps_msg->eth_type), vlan_id, sal_ntoh16(p_erps_msg->frame_len), sal_ntoh16(p_erps_msg->erps_len));

    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PACKET, "erps_ver is %u, erps_type is %u, domain_id is %u, ring_id is %u",
            p_erps_msg->erps_ver, p_erps_msg->erps_type, sal_ntoh16(p_erps_msg->domain_id), sal_ntoh16(p_erps_msg->ring_id));

    sal_memset(buf, 0, sizeof(buf));
    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PACKET, "sys_mac is %s, state is %u, level is %u", 
            cdb_mac_addr_val2str(buf, CMD_BUF_256, p_erps_msg->sys_mac), p_erps_msg->state, p_erps_msg->level);

    /* get domain */
    domain_key.domain_id = sal_ntoh16(p_erps_msg->domain_id);
    p_db_domain = tbl_erps_domain_get_erps_domain(&domain_key);
    if (!p_db_domain)
    {
        return PM_E_NONE;
    }

    if (p_rcv_if->lag)
    {
        sal_memset(&if_key, 0, sizeof(if_key));
        sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_rcv_if->lag->lag_id);
        p_db_if_lag = tbl_interface_get_interface(&if_key);
        if (p_db_if_lag)
        {
            p_rcv_if = p_db_if_lag;
        }
    }

    ring_key.domain_id = sal_ntoh16(p_erps_msg->domain_id);
    ring_key.ring_id = sal_ntoh16(p_erps_msg->ring_id);
    p_db_ring = tbl_erps_ring_get_erps_ring(&ring_key);
    if (p_db_ring)
    {
        if (p_db_ring->erps_ring_level != p_erps_msg->level)
        {
            ERPS_LOG_ERR("_erps_msg_dispatcher invalid erps ring level");
            return PM_E_NONE;
        }
        
        if (ERPS_IDLE == p_db_ring->erps_ring_state)
        {
            return PM_E_NONE;
        }

        if ((p_db_ring->primary_port_if != p_rcv_if)
            && (p_db_ring->secondary_port_if != p_rcv_if))
        {
            p_domain_primary_ring = (tbl_erps_ring_t*)p_db_domain->erps_primary_ring;
            if (!p_domain_primary_ring 
                || (ERPS_IDLE == p_domain_primary_ring->erps_ring_state) 
                || ((p_domain_primary_ring->primary_port_if != p_rcv_if)
                  && (p_domain_primary_ring->secondary_port_if != p_rcv_if)))
            {
                ERPS_LOG_ERR("_erps_msg_dispatcher recv erps pkt in error port");
                return PM_E_NONE;
            }
        }    
    }
    else
    {
        if (p_db_domain->sub_control_vlan_id == vlan_id)
        {   
            if (p_db_domain->erps_primary_ring)
            {
                p_domain_primary_ring = (tbl_erps_ring_t*)p_db_domain->erps_primary_ring;   
                if (ERPS_IDLE == p_domain_primary_ring->erps_ring_state)
                {
                    return PM_E_NONE;
                }

                /* when node is edge or edge assistant, if no port is bloced for primary ring in this switch
                   forwarding erps pkts in sub vlan */
                if ((GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU == p_rcv_if->erps_pdu_action) 
                    && (!p_domain_primary_ring->blocked_port_if))
                {
                    return vlan_id;
                }
                return PM_E_NONE;
            }

            ERPS_LOG_ERR("_erps_msg_dispatcher ERPS primary ring is not configured for domain %u\n", p_db_domain->key.domain_id); 
            return PM_E_NONE;
        }    

        ERPS_LOG_ERR("_erps_msg_dispatcher ERPS packet received in mis-match vlan or in nonexistent ring\n");
        return PM_E_NONE;
    }

    if (ERPS_IDLE == p_db_ring->erps_ring_state)
    {
        return PM_E_NONE;
    } 

    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PACKET, "receive message of type %d for domain %u ring %u %s node in %s state", 
            p_erps_msg->erps_type, p_db_domain->key.domain_id, p_db_ring->key.ring_id,
            erps_desc_node_role(p_db_ring->node_role),
            erps_desc_node_state(p_db_ring->erps_ring_state)); 

    switch (p_erps_msg->erps_type)
    {
        case ERPS_HEALTH_PKT:
            if (ERPS_NODE_MASTER == p_db_ring->node_role) 
            {
                if (p_db_ring->primary_port_if == p_rcv_if)
                {
                    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PACKET, "ERPS receive hello message from primary interface in domain %u ring %u\n", 
                        p_db_ring->key.domain_id, p_db_ring->key.ring_id);
                    return PM_E_NONE;
                }

                if (sal_memcmp(p_erps_msg->sys_mac, pGlobal->route_mac, GLB_ETH_ADDR_LEN))
                {
                    ERPS_LOG_ERR("ERPS receive hello message with invalid system mac %02x:%02x:%02x:%02x:%02x:%02x\n",
                        p_erps_msg->sys_mac[0], p_erps_msg->sys_mac[1], p_erps_msg->sys_mac[2],
                        p_erps_msg->sys_mac[3], p_erps_msg->sys_mac[4], p_erps_msg->sys_mac[5]);
                }
            }

            _erps_msg_rcv_health_pkt(p_db_domain, p_db_ring, ERPS_TICKS_TO_SECS(sal_ntoh16(p_erps_msg->h_timer)));

            /*for edge/edge-asistant port*/
            if (ERPS_NODE_EDGE_UNKNOWN != p_db_ring->edge_node_role)
            {
                if (p_db_ring->primary_port_if == p_rcv_if)
                {
                    break;
                }
                
                if (p_db_ring->secondary_port_if == p_rcv_if)
                {
                    ipc_pkt_tx_erps((uint8*)p_msg->data, p_msg->hdr.length, p_db_ring->primary_port_ifindex);
                    return PM_E_NONE;
                }
                
                /*from other port: the line of s_port_if is down*/
                if (GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU == p_rcv_if->erps_pdu_action)
                {
                    ipc_pkt_tx_erps((uint8*)p_msg->data, p_msg->hdr.length, p_db_ring->primary_port_ifindex);
                    return PM_E_NONE;
                }
            }
            
            /*one port is blocked*/
            if (ERPS_PREFORWORDING == p_db_ring->erps_ring_state)
            {
                break;
            }
            
            /* self is transit, but have maset, need send the packet */
            if (ERPS_NODE_TRANSIT == p_db_ring->node_role)
            {
                if (GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU == p_rcv_if->erps_pdu_action)
                {
                    break;
                }
            }
            return PM_E_NONE;


        case ERPS_RING_UP_FLUSH_FDB_PKT:
            old_state = p_db_ring->erps_ring_state;
            
            _erps_msg_rcv_ring_up_flush_fdb_pkt(p_db_domain, p_db_ring);

            /*for edge/edge-asistant port*/
            if (ERPS_NODE_EDGE_UNKNOWN != p_db_ring->edge_node_role)
            {
                if (p_db_ring->primary_port_if == p_rcv_if)
                {
                    break;
                }
                
                if (p_db_ring->secondary_port_if == p_rcv_if)
                {
                    ipc_pkt_tx_erps((uint8*)p_msg->data, p_msg->hdr.length, p_db_ring->primary_port_ifindex);
                    return PM_E_NONE;
                }
                
                /*from other port: the line of s_port_if is down*/
                if (GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU == p_rcv_if->erps_pdu_action)
                {
                    ipc_pkt_tx_erps((uint8*)p_msg->data, p_msg->hdr.length, p_db_ring->primary_port_ifindex);
                    return PM_E_NONE;
                }
            }

            /* self is transit, but have maset, need send the packet */
            if (ERPS_NODE_TRANSIT == p_db_ring->node_role)
            {
                /* need to check by wangjj for erps pkt forwarding, 2016-11-23 */
                if ((GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU == p_rcv_if->erps_pdu_action)
                    && (ERPS_PREFORWORDING != old_state))
                {
                    break;
                }
            }
            return PM_E_NONE;

        case ERPS_RING_DOWN_FLUSH_FDB_PKT:
            _erps_msg_rcv_ring_down_flush_fdb_pkt(p_db_domain, p_db_ring);

            /*for edge/edge-asistant port*/
            if (ERPS_NODE_EDGE_UNKNOWN != p_db_ring->edge_node_role)
            {
                if (p_db_ring->primary_port_if == p_rcv_if)
                {
                    break;
                }

                if (p_db_ring->secondary_port_if == p_rcv_if)
                {
                    ipc_pkt_tx_erps((uint8*)p_msg->data, p_msg->hdr.length, p_db_ring->primary_port_ifindex);
                    return PM_E_NONE;
                }
                
                /*from other port: the line of s_port_if is down*/
                if (GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU == p_rcv_if->erps_pdu_action)
                {
                    ipc_pkt_tx_erps((uint8*)p_msg->data, p_msg->hdr.length, p_db_ring->primary_port_ifindex);
                    return PM_E_NONE;
                }
            }
            
            if (ERPS_PREFORWORDING == p_db_ring->erps_ring_state)
            {
                break;
            }
            
            if (ERPS_NODE_TRANSIT == p_db_ring->node_role)
            {
                if (GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU == p_rcv_if->erps_pdu_action)
                {
                    break;
                }
            }
            return PM_E_NONE;

        case ERPS_LINK_DOWN_PKT:           
            /*for edge/edge-asistant port*/
            if (ERPS_NODE_EDGE_UNKNOWN != p_db_ring->edge_node_role)
            {
                if (p_db_ring->primary_port_if == p_rcv_if)
                {
                    break;
                }

                if (p_db_ring->secondary_port_if == p_rcv_if)
                {
                    ipc_pkt_tx_erps((uint8*)p_msg->data, p_msg->hdr.length, p_db_ring->primary_port_ifindex);
                    return PM_E_NONE;
                }
                
                /*from other port: the line of s_port_if is down*/
                if (GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU == p_rcv_if->erps_pdu_action)
                {
                    ipc_pkt_tx_erps((uint8*)p_msg->data, p_msg->hdr.length, p_db_ring->primary_port_ifindex);
                    return PM_E_NONE;
                }
            }

            _erps_msg_rcv_link_down_pkt(p_db_domain, p_db_ring);
            if (ERPS_PREFORWORDING == p_db_ring->erps_ring_state)
            {
                break;
            }
            if (ERPS_NODE_TRANSIT == p_db_ring->node_role)
            {
                if (GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU == p_rcv_if->erps_pdu_action)
                {
                    break;
                }
            }
            return PM_E_NONE;

        case ERPS_EDGE_HELLO_PKT:
            _erps_msg_rcv_edge_hello_pkt(p_db_domain, p_db_ring);

            if (ERPS_NODE_EDGE_UNKNOWN != p_db_ring->edge_node_role)
            {
                return PM_E_NONE;
            }
            if (ERPS_PREFORWORDING == p_db_ring->erps_ring_state)
            {
                break;
            }

            if (GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU == p_rcv_if->erps_pdu_action)
            {
                break;
            }
            return PM_E_NONE;
            
        case ERPS_MAJOR_FAULT_PKT:
            _erps_msg_rcv_major_fault_pkt(p_db_domain, p_db_ring);

            if (ERPS_NODE_EDGE_UNKNOWN != p_db_ring->edge_node_role)
            {
                return PM_E_NONE;
            }
            
            if (ERPS_PREFORWORDING == p_db_ring->erps_ring_state)
            {
                break;
            }

            if (GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU == p_rcv_if->erps_pdu_action)
            {
                break;
            }
            return PM_E_NONE;
           
        default:
            ERPS_LOG_ERR("Invalid erps type in msg\n");
            return PM_E_NONE;
    }
    return vlan_id;
}

int32
erps_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    uint8            *p_rcv_buf = NULL;
    uint16           vlan_id = 0;
    tbl_vlan_key_t   key;
    tbl_vlan_t       *p_db_vlan = NULL;
    int32            rc = PM_E_NONE;
    tbl_interface_t  *p_db_if = NULL;
    uint32           ifindex = 0;
    uint32           rcv_ifindex = 0;

    p_rcv_buf = (uint8*)p_msg->data;
    rcv_ifindex = p_msg->u_hdr.pkt.ifindex;

    /*pass to msg dispatcher*/
    vlan_id = _erps_msg_dispatcher(p_msg);

    /* forward the pkt in application layer */
    if (vlan_id)
    {
        ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PACKET, "Transmit erps msg from vlan %u", vlan_id);

        key.vid = vlan_id;

        /* check exist */
        p_db_vlan = tbl_vlan_get_vlan(&key);
        if (!p_db_vlan)
        {
            return PM_E_NONE;
        }

        /* send to all members */
        GLB_BMP_ITER_BEGIN(p_db_vlan->member_port, ifindex)
        {
            p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
            if (!p_db_if)
            {
                continue;
            }

            /* modified by wangjj for fix erps bug 41465, 2016-12-05 */
            //if (tbl_interface_is_running(p_db_if))
            if (ifindex != rcv_ifindex && tbl_interface_is_running(p_db_if))
            {
                rc = ipc_pkt_tx_erps((uint8*)p_rcv_buf, p_msg->hdr.length, p_db_if->ifindex);
                if (rc)
                {
                    ERPS_LOG_ERR("send msg through port %d fail\n", p_db_if->ifindex);
                    return rc;
                }   
            }
        }
        GLB_BMP_ITER_END(p_db_vlan->member_port, ifindex);
    }

    /* let packet flooding */
#if 0    
    if (flush_p_if)
    {
        erps_fdb_flush_dynamic_fdb_port(flush_p_if->ifindex);
        flush_p_if = NULL;
    }
    if (flush_s_if)
    {
        erps_fdb_flush_dynamic_fdb_port(flush_s_if->ifindex);
        flush_s_if = NULL;
    }
#endif
    return PM_E_NONE;
}

static bool
_erps_port_check_link_up(tbl_erps_ring_t  *p_db_ring, tbl_interface_t *p_if)
{
    if ((ERPS_NODE_EDGE_UNKNOWN != p_db_ring->edge_node_role)
               && (p_db_ring->secondary_port_if == p_if))
    {
        /* when subring common port, the linkup/down is invalid, should be treated as UP */
        return TRUE;
    }

    return (tbl_interface_is_running(p_if));
}

static void
_erps_port_report_link_up(tbl_erps_ring_t  *p_db_ring, tbl_interface_t *p_if)
{
    tbl_erps_domain_t  *p_db_domain = NULL;

    if (!p_db_ring)
    {
        ERPS_LOG_ERR("NULL ring in port report link up func");
        return;
    }

    p_db_domain = (tbl_erps_domain_t*)p_db_ring->p_erps_domain;
    if (!p_db_domain)
    {
        ERPS_LOG_ERR("NULL domain in port report link up func");
        return;
    } 
    
    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_EVENTS, "receive link up for domain %u ring %u %s node in %s state", 
        p_db_domain->key.domain_id, p_db_ring->key.ring_id, 
        erps_desc_node_role(p_db_ring->node_role),
        erps_desc_node_state(p_db_ring->erps_ring_state)); 

    switch (p_db_ring->erps_ring_state)
    { 
    case ERPS_LINK_DOWN :
        if (_erps_port_check_link_up(p_db_ring, p_db_ring->primary_port_if) 
            && _erps_port_check_link_up(p_db_ring, p_db_ring->secondary_port_if))
        {
            /* there may be one port blocked in failed state to avoid broadcasting storm, for bug 12783 */ 
            if (!p_db_ring->blocked_port_if)
            {
                if (p_if->ifindex == p_db_ring->primary_port_ifindex)
                {
                    p_db_ring->blocked_port_if = p_db_ring->primary_port_if; 
                    p_db_ring->blocked_port_ifindex = p_db_ring->primary_port_ifindex;
                }
                else
                {
                    p_db_ring->blocked_port_if = p_db_ring->secondary_port_if; 
                    p_db_ring->blocked_port_ifindex = p_db_ring->secondary_port_ifindex;                    
                }
                erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, TRUE);           
            }

            /*if (p_db_ring->pre_forwarding_timer)
            {
                ctc_task_delete_timer(p_db_ring->pre_forwarding_timer);
                p_db_ring->pre_forwarding_timer = NULL;
            }*/
            CTC_TASK_STOP_TIMER(p_db_ring->pre_forwarding_timer);

            p_db_ring->pre_forwarding_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_pre_forwarding_timer_fn, 
                        p_db_ring, p_db_domain->pre_forwarding_timer_interval);
                    
            if (!p_db_ring->pre_forwarding_timer)
            {
                ERPS_LOG_ERR("Preforwarding timer set fail");
            }

            p_db_ring->erps_ring_state = ERPS_PREFORWORDING;
            
            ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PROTO, "ERPS domain %d ring %d state switch to %s\n", p_db_ring->key.domain_id,
                    p_db_ring->key.ring_id, erps_desc_node_state(p_db_ring->erps_ring_state));
        }
        break;

    /* there may be one port blocked in failed state to avoid broadcasting storm */ 
    case ERPS_FAILED:  
        if (_erps_port_check_link_up(p_db_ring, p_db_ring->primary_port_if)  
            && _erps_port_check_link_up(p_db_ring, p_db_ring->secondary_port_if))
        {   
            /* there may be one port blocked in failed state to avoid broadcasting storm */ 
            if (!p_db_ring->blocked_port_if)
            {
                if (p_if->ifindex == p_db_ring->primary_port_ifindex)
                {
                    p_db_ring->blocked_port_if = p_db_ring->primary_port_if; 
                    p_db_ring->blocked_port_ifindex = p_db_ring->primary_port_ifindex;
                }
                else
                {
                    p_db_ring->blocked_port_if = p_db_ring->secondary_port_if; 
                    p_db_ring->blocked_port_ifindex = p_db_ring->secondary_port_ifindex;                    
                }
                erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, TRUE);           
            }

            /*if (p_db_ring->pre_forwarding_timer)
            {
                ctc_task_delete_timer(p_db_ring->pre_forwarding_timer);
                p_db_ring->pre_forwarding_timer = NULL;
            }*/
            CTC_TASK_STOP_TIMER(p_db_ring->pre_forwarding_timer);

            p_db_ring->pre_forwarding_timer = ctc_task_add_timer(TASK_PRI_NORMAL, erps_timer_pre_forwarding_timer_fn, 
                        p_db_ring, p_db_domain->pre_forwarding_timer_interval);
                    
            if (!p_db_ring->pre_forwarding_timer)
            {
                ERPS_LOG_ERR("Preforwarding timer set fail");
            }             
        }

        /* modified by liwh for bug 47130, 2019-02-18 */
        if (!tbl_interface_is_running(p_db_ring->primary_port_if)
            && (p_db_ring->secondary_port_ifindex == p_if->ifindex))
        {
            if ((p_db_ring->blocked_port_if) && (p_db_ring->secondary_port_ifindex == p_db_ring->blocked_port_ifindex))
            {
                erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);
                p_db_ring->blocked_port_if = NULL; 
                p_db_ring->blocked_port_ifindex = 0;
            }
            
            return;
        }
        /* liwh end */
        
        /* send one hello pkt to accelerate the restoration time */        
        erps_msg_send_pkt(ERPS_HEALTH_PKT, p_db_ring->primary_port_if, p_db_domain, p_db_ring);
        break;

    default:
        break;
    }

    return;
}

static void
_erps_port_report_link_down(tbl_erps_ring_t  *p_db_ring, tbl_interface_t *p_if)
{
    tbl_erps_domain_t  *p_db_domain = NULL;
    tbl_erps_ring_t    *p_erps_ring = NULL;
    ctclib_slistnode_t *listnode = NULL; 
    bool               need_sleep = FALSE;
    tbl_interface_t    *primary_port_if = NULL;

    if (!p_db_ring)
    {
        ERPS_LOG_ERR("NULL ring in port report link down func");
        return;
    }

    p_db_domain = (tbl_erps_domain_t*)p_db_ring->p_erps_domain;
    if (!p_db_domain)
    {
        ERPS_LOG_ERR("NULL domain in port report link down func");
        return;
    } 

    ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_EVENTS, "receive report link down msg for domain %u ring %u %s node in %s state", 
        p_db_domain->key.domain_id, p_db_ring->key.ring_id, 
        erps_desc_node_role(p_db_ring->node_role),
        erps_desc_node_state(p_db_ring->erps_ring_state));

    /* for dianxing xiangjiaohuan topo */
    if (ERPS_RING_PRIMARY == p_db_ring->erps_ring_level)
    {
        if (!(tbl_interface_is_running((tbl_interface_t*)p_db_ring->primary_port_if)) 
            && !(tbl_interface_is_running((tbl_interface_t*)p_db_ring->secondary_port_if)))
        {
            CTCLIB_SLIST_LOOP(p_db_domain->erps_ring_list.obj_list, p_erps_ring, listnode)
            {
                primary_port_if = (tbl_interface_t*)p_erps_ring->primary_port_if;
                if (primary_port_if && (primary_port_if->ifindex == p_if->ifindex))
                {
                    continue;
                }
                
                erps_msg_send_pkt(ERPS_LINK_DOWN_PKT, p_db_ring->primary_port_if, p_db_domain, p_db_ring);
                need_sleep = TRUE;  
            }

            if (need_sleep)
            {
                usleep(5000); /*sleep 5ms*/
                need_sleep = FALSE;
            }
        }
    }
    /* for dianxing xiangjiaohuan topo */

    switch (p_db_ring->erps_ring_state)
    {    
    case ERPS_LINK_UP:
        /*the status is also up when common if is down?*/
        if (ERPS_NODE_EDGE_UNKNOWN != p_db_ring->edge_node_role)
        {
            if (p_db_domain && p_db_domain->erps_primary_ring)
            {
                p_erps_ring = (tbl_erps_ring_t*)p_db_domain->erps_primary_ring;
                
                if (p_erps_ring->primary_port_if 
                    && (p_erps_ring->primary_port_if != p_erps_ring->blocked_port_if))
                {
                    erps_msg_send_pkt(ERPS_LINK_DOWN_PKT, p_erps_ring->primary_port_if, p_db_domain, p_db_ring);
                    need_sleep = TRUE; 
                }

                if (p_erps_ring->secondary_port_if
                    && (p_erps_ring->secondary_port_if != p_erps_ring->blocked_port_if))
                {
                    erps_msg_send_pkt(ERPS_LINK_DOWN_PKT, p_erps_ring->secondary_port_if, p_db_domain, p_db_ring);
                    need_sleep = TRUE; 
                }
            }
            if (need_sleep)
            {
                usleep(5000); /*sleep 5ms*/
            }
        }
        /*the status is also up then common if is down?*/

        if (tbl_interface_is_running((tbl_interface_t*)p_db_ring->primary_port_if))
        {
            erps_msg_send_pkt(ERPS_LINK_DOWN_PKT, p_db_ring->primary_port_if, p_db_domain, p_db_ring);
            //usleep(5000); /*sleep 5ms*/
            //erps_fdb_flush_dynamic_fdb_port(p_db_ring->secondary_port_ifindex);              
        }
        else if (tbl_interface_is_running((tbl_interface_t*)p_db_ring->secondary_port_if))
        {
            erps_msg_send_pkt(ERPS_LINK_DOWN_PKT, p_db_ring->secondary_port_if, p_db_domain, p_db_ring);            
            //usleep(5000); /*sleep 5ms*/
            //erps_fdb_flush_dynamic_fdb_port(p_db_ring->primary_port_ifindex);  
        }
        //erps_fdb_flush_dynamic_fdb_port(p_if->ifindex);  

        erps_fdb_flush_dynamic_fdb_all();
        
        p_db_ring->erps_ring_state = ERPS_LINK_DOWN;
        
        ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PROTO, "ERPS domain %d ring %d state switch to %s\n", p_db_ring->key.domain_id,
                p_db_ring->key.ring_id, erps_desc_node_state(p_db_ring->erps_ring_state));
        break;

    case ERPS_LINK_DOWN :
        if (ERPS_NODE_EDGE_UNKNOWN != p_db_ring->edge_node_role)
        {
            if (p_db_domain && p_db_domain->erps_primary_ring)
            {
                p_erps_ring = (tbl_erps_ring_t*)p_db_domain->erps_primary_ring;
                
                if (p_erps_ring->primary_port_if 
                    && (p_erps_ring->primary_port_if != p_erps_ring->blocked_port_if))
                {
                    erps_msg_send_pkt(ERPS_RING_DOWN_FLUSH_FDB_PKT, p_erps_ring->primary_port_if, p_db_domain, p_db_ring);
                }

                if (p_erps_ring->secondary_port_if
                    && (p_erps_ring->secondary_port_if != p_erps_ring->blocked_port_if))
                {
                    erps_msg_send_pkt(ERPS_RING_DOWN_FLUSH_FDB_PKT, p_erps_ring->secondary_port_if, p_db_domain, p_db_ring);
                }
                usleep(5000); /*sleep 5ms*/
            }
        }
        //erps_fdb_flush_dynamic_fdb_port(p_if->ifindex);  
        erps_fdb_flush_dynamic_fdb_all();
        break;

    case ERPS_PREFORWORDING :    
        /*if (p_db_ring->pre_forwarding_timer)
        {
            ctc_task_delete_timer(p_db_ring->pre_forwarding_timer);
            p_db_ring->pre_forwarding_timer = NULL;
        }*/
        CTC_TASK_STOP_TIMER(p_db_ring->pre_forwarding_timer);

        if (p_db_ring->blocked_port_if)
        {
            erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);
            p_db_ring->blocked_port_if = NULL; 
            p_db_ring->blocked_port_ifindex = 0;
        }

        p_db_ring->erps_ring_state = ERPS_LINK_DOWN;
        
        ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PROTO, "ERPS domain %d ring %d state switch to %s\n", p_db_ring->key.domain_id,
                p_db_ring->key.ring_id, erps_desc_node_state(p_db_ring->erps_ring_state));
        break;

    case ERPS_INIT:
    case ERPS_COMPLETE :
        /* need to stop failure timer before transiting into failed state */ 
        /*if (p_db_ring->failure_timer)
        {
            ctc_task_delete_timer(p_db_ring->failure_timer);
            p_db_ring->failure_timer = NULL;
        }*/
        CTC_TASK_STOP_TIMER(p_db_ring->failure_timer);
     
        /*need not to unblock port, when the port is down*/
        if (p_if->ifindex != p_db_ring->blocked_port_ifindex)
        {
            erps_set_erps_ring_blocked_interface(p_db_ring, p_db_ring, FALSE);
            p_db_ring->blocked_port_if = NULL; 
            p_db_ring->blocked_port_ifindex = 0; 
        }

        erps_msg_send_pkt(ERPS_RING_DOWN_FLUSH_FDB_PKT, p_db_ring->primary_port_if, p_db_domain, p_db_ring);
        erps_msg_send_pkt(ERPS_RING_DOWN_FLUSH_FDB_PKT, p_db_ring->secondary_port_if, p_db_domain, p_db_ring);

        //usleep(5000); /*sleep 5ms*/
        //erps_fdb_flush_dynamic_fdb_port(p_db_ring->primary_port_ifindex);  
        //erps_fdb_flush_dynamic_fdb_port(p_db_ring->secondary_port_ifindex); 
        erps_fdb_flush_dynamic_fdb_all();
        
        p_db_ring->erps_ring_state = ERPS_FAILED;
      
        ERPS_LOG_DEBUG(OPMDBG_FLAG_ERPS_PROTO, "ERPS domain %d ring %d state switch to %s\n", p_db_ring->key.domain_id,
                p_db_ring->key.ring_id, erps_desc_node_state(p_db_ring->erps_ring_state));
        break;          
    default:
        break;
    }

    return;
}

int32 erps_port_report_link_state(tbl_interface_t *p_if)
{
    bool                     up  = FALSE;
    tbl_erps_ring_master_t   *p_master = tbl_erps_ring_get_master();
    tbl_erps_ring_t          *p_ring = NULL;
    ctclib_slistnode_t       *listnode = NULL;  
    tbl_interface_t          *primary_port_if = NULL;
    tbl_interface_t          *secondary_port_if = NULL;

    if (tbl_interface_is_running(p_if))
    {
        up = TRUE;
    }
    
    CTCLIB_SLIST_LOOP(p_master->erps_ring_list, p_ring, listnode)
    {
        primary_port_if = (tbl_interface_t*)p_ring->primary_port_if;
        secondary_port_if = (tbl_interface_t*)p_ring->secondary_port_if;
        
        if ((primary_port_if && (primary_port_if->ifindex == p_if->ifindex))
            || (secondary_port_if && (secondary_port_if->ifindex == p_if->ifindex)))
        {          
            if ((ERPS_NODE_EDGE_UNKNOWN != p_ring->edge_node_role)
               && (primary_port_if->ifindex != p_if->ifindex))
            {
                continue;
            }
            
            if (ERPS_IDLE == p_ring->erps_ring_state)
            {
                erps_fdb_flush_dynamic_fdb_port(p_if->ifindex);
                continue;
            }

            if(up)
            {
                _erps_port_report_link_up(p_ring, p_if);
            }
            else
            {
                _erps_port_report_link_down(p_ring, p_if);
            }       
        }
    }
    
    return PM_E_NONE;
}

bool erps_debug_is_on(uint32 flag)
{
    tbl_opm_debug_t *p_db_dbg = NULL;

    /* check exist */
    p_db_dbg = tbl_opm_debug_get_opm_debug();
    if (p_db_dbg)
    {
        if (GLB_FLAG_ISSET(p_db_dbg->erps, flag))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*
int32
erps_cmd_get(tbl_opm_debug_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_opm_debug_dump_one(pitem, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}
*/

/*
#define DBG_OPM_TYPE_FIELD     0
#define DBG_OPM_TYPE_ALL       1

static int32
erps_cmd_process_opm_debug(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_opm_debug_t dbg;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    int32 i = 0;
    tbl_opm_debug_t *p_db_dbg = NULL;
    int32 rc = PM_E_NONE;
    field_parse_t field;
    uint32 multi_type = DBG_OPM_TYPE_FIELD;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_OPM_DEBUG);

    
    p_db_dbg = tbl_opm_debug_get_opm_debug();
    if (!p_db_dbg)
    {
        CFG_CONFLICT_RET("OPM_DEBUG not found");
    }

    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&dbg, 0, sizeof(dbg));
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            if (0 == sal_strcmp(argv[i], "all"))
            {
                multi_type = DBG_OPM_TYPE_ALL;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
            }
        }
        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);

        if (DBG_OPM_TYPE_FIELD == multi_type)
        {
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }

            switch (field_id)
            {
            case TBL_OPM_DEBUG_FLD_ERPS_ALL:
                GLB_SET_FLAG_COND(value, dbg.erps, OPMDBG_FLAG_ERPS_ALL);
                break;
            case TBL_OPM_DEBUG_FLD_ERPS_PACKET:
                GLB_SET_FLAG_COND(value, dbg.erps, OPMDBG_FLAG_ERPS_PACKET);
                break;
            case TBL_OPM_DEBUG_FLD_ERPS_TIMER:
                GLB_SET_FLAG_COND(value, dbg.erps, OPMDBG_FLAG_ERPS_TIMER);
                break;
            case TBL_OPM_DEBUG_FLD_ERPS_PROTOCOL:
                GLB_SET_FLAG_COND(value, dbg.erps, OPMDBG_FLAG_ERPS_PROTO);
                break;
            case TBL_OPM_DEBUG_FLD_ERPS_EVENT:
                GLB_SET_FLAG_COND(value, dbg.erps, OPMDBG_FLAG_ERPS_EVENTS);
                break;
            default:
                CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
                break;
            }
            rc = tbl_opm_debug_set_opm_debug_field(&dbg, field_id);
        }
        else if (DBG_OPM_TYPE_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, dbg.erps, OPMDBG_FLAG_ERPS_ALL);
            GLB_SET_FLAG_COND(value, dbg.erps, OPMDBG_FLAG_ERPS_PACKET);
            GLB_SET_FLAG_COND(value, dbg.erps, OPMDBG_FLAG_ERPS_TIMER);
            GLB_SET_FLAG_COND(value, dbg.erps, OPMDBG_FLAG_ERPS_PROTO);
            GLB_SET_FLAG_COND(value, dbg.erps, OPMDBG_FLAG_ERPS_EVENTS);
            rc = tbl_opm_debug_set_opm_debug_field(&dbg, TBL_OPM_DEBUG_FLD_ERPS_ALL);
            rc = tbl_opm_debug_set_opm_debug_field(&dbg, TBL_OPM_DEBUG_FLD_ERPS_PACKET);
            rc = tbl_opm_debug_set_opm_debug_field(&dbg, TBL_OPM_DEBUG_FLD_ERPS_TIMER);
            rc = tbl_opm_debug_set_opm_debug_field(&dbg, TBL_OPM_DEBUG_FLD_ERPS_PROTOCOL);
            rc = tbl_opm_debug_set_opm_debug_field(&dbg, TBL_OPM_DEBUG_FLD_ERPS_EVENT);
        }
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = erps_cmd_get(p_db_dbg, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}
*/
bool erps_mode_is_huawei()
{
    tbl_opm_global_t *p_db_glb = NULL;

    /* check exist */
    p_db_glb = tbl_opm_global_get_opm_global();
    if (p_db_glb)
    {
        if (GLB_FLAG_ISSET(p_db_glb->erps, OPMGLB_FLAG_ERPS_MODE_UNRELOAD))
        {
            if (!GLB_FLAG_ISSET(p_db_glb->erps, OPMGLB_FLAG_ERPS_MODE_HUAWEI))
            {
                return TRUE;
            }
        }
        else
        {
            if (GLB_FLAG_ISSET(p_db_glb->erps, OPMGLB_FLAG_ERPS_MODE_HUAWEI))
            {
                return TRUE;
            }
        }
    }

    return FALSE; 
}

static int32
erps_cmd_process_opm_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_opm_global_t opm_glb;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    int32 i = 0;
    tbl_opm_global_t *p_db_glb = NULL;
    int32 rc = PM_E_NONE;
    tbl_sys_global_t* pGlobal = tbl_sys_global_get_sys_global();

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_OPM_GLOBAL);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&opm_glb, 0, sizeof(opm_glb));

        /* check exist */
        p_db_glb = tbl_opm_global_get_opm_global();
        if (!p_db_glb)
        {
            CFG_CONFLICT_RET("OPM_GLOBAL not found");
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

        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);

        if (value == GLB_FLAG_ISSET(p_db_glb->erps,OPMGLB_FLAG_ERPS_MODE_HUAWEI))
        {
            return rc;
        }
        
        switch (field_id)
        {
        case TBL_OPM_GLOBAL_FLD_ERPS_MODE:
            GLB_SET_FLAG_COND(value, opm_glb.erps, OPMGLB_FLAG_ERPS_MODE_HUAWEI);
            break;
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        rc = tbl_opm_global_set_opm_global_field(&opm_glb, field_id);

        if (pGlobal && pGlobal->startup_done) {

            if (GLB_FLAG_ISSET(p_db_glb->erps, OPMGLB_FLAG_ERPS_MODE_UNRELOAD))
            {
                GLB_SET_FLAG_COND(FALSE, opm_glb.erps, OPMGLB_FLAG_ERPS_MODE_UNRELOAD);
                tbl_opm_global_set_opm_global_field(&opm_glb, TBL_OPM_GLOBAL_FLD_ERPS_UNRELOAD);
            }
            else
            {
                GLB_SET_FLAG_COND(TRUE, opm_glb.erps, OPMGLB_FLAG_ERPS_MODE_UNRELOAD);
                tbl_opm_global_set_opm_global_field(&opm_glb, TBL_OPM_GLOBAL_FLD_ERPS_UNRELOAD);
            }
            
            CFG_CONFLICT_RET("Erps mode change will take effect until next reload.");
        }
        break;

    case CDB_OPER_GET:
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
erps_start(void)
{
    cdb_register_cfg_tbl_cb(TBL_OPM_GLOBAL,  erps_cmd_process_opm_global);
    cdb_register_cfg_tbl_cb(TBL_ERPS_DOMAIN, erps_cmd_process_erps_domain);
    cdb_register_cfg_tbl_cb(TBL_ERPS_RING,   erps_cmd_process_erps_ring);  

    cdb_register_cfg_act_cb(ACT_SHOW_ERPS_LIST, erps_cmd_show_erps_list);
    cdb_register_cfg_act_cb(ACT_SHOW_ERPS, erps_cmd_show_erps);
//    cdb_register_cfg_tbl_cb(TBL_OPM_DEBUG, erps_cmd_process_opm_debug);
    cdb_register_cfg_act_cb(ACT_CLEAR_ERPS_COUNTERS, erps_cmd_clear_erps_counters);
    ipc_register_pkt_rx_fn(GLB_PKT_ERPS, erps_pdu_rx);
    
    return PM_E_NONE;
}

int32
erps_stop(void)
{
    cdb_register_cfg_tbl_cb(TBL_ERPS_DOMAIN, NULL);

    return PM_E_NONE;
}

