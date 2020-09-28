#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_global.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"
#include "gen/tbl_msti_port_define.h"
#include "gen/tbl_msti_port.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_mcfdb.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_fdb.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_macfilter.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "gen/tbl_psfdb_define.h"
#include "gen/tbl_psfdb.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "switch.h"
#include "brg.h"
#include "mstp_define.h"
#include "mstp_api.h"
#include "mstp_sm.h"
#include "msti_sm.h"
#include "mstp_pdu.h"
#include "mstp.h"
#include "brg_fdb.h"
#include "switch_api.h"
#include "auth_md5.h"

#ifdef CONFIG_MSTP

int32
mstp_api_add_msti_port(tbl_interface_t *p_db_if, tbl_mstp_port_t *p_db_port, uint32 instance);

static char CONFIGURATION_DIGEST_SIGNATURE_KEY[MSTP_DIGEST_LEN] =
{ 
    0x13, 0xAC, 0x06, 0xA6, 0x2E, 0x47, 0xFD, 0x51,
    0xF9, 0x5D, 0x2B, 0xA2, 0x43, 0xCD, 0x03, 0x46
};

static int32
_mstp_api_reinit_enable()
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    uint32 stp_enable = FALSE;

    stp_enable = p_db_glb->enable;

    if (!stp_enable)
    {
        return PM_E_NONE;
    }

    p_db_glb->enable = FALSE;
    mstp_api_set_glb_enable(p_db_glb, TRUE);

    p_db_glb->enable = TRUE;
    mstp_api_set_glb_enable(p_db_glb, FALSE);
    
    return PM_E_NONE;
}

static int32
_mstp_api_update_port_operP2PMac(tbl_mstp_port_t *p_db_port)
{
    tbl_interface_t *p_db_if = NULL;

    if (MSTP_LINK_AUTO == p_db_port->adminP2PMac)
    {
        p_db_if = (tbl_interface_t*)p_db_port->p_if;

        /* modified by liwh for bug 37629, 2016-05-09 */
        /*p_db_port->operP2PMac = (GLB_DUPLEX_FULL == p_db_if->oper_duplex) ? TRUE : FALSE;*/
        p_db_port->operP2PMac = (GLB_DUPLEX_HALF == p_db_if->oper_duplex) ? FALSE : TRUE;
        /* liwh end */
        
    }

    return PM_E_NONE;
}

static int32
_mstp_api_set_default_disabled_msti_port(tbl_msti_port_t *p_db_port)
{
    p_db_port->disputed             = FALSE;
    p_db_port->fdbFlush             = FALSE;
    p_db_port->forward              = FALSE;
    p_db_port->forwarding           = FALSE;
    p_db_port->infoIs               = MSTP_INFO_DISABLED;
    p_db_port->learn                = FALSE;
    p_db_port->learning             = FALSE;
    p_db_port->newInfo              = FALSE;
    sal_memset(&p_db_port->port_rootId, 0, sizeof(mstp_brg_id_t));
    p_db_port->port_rootPathCost    = 0;
    sal_memset(&p_db_port->port_designatedBridgeId, 0, sizeof(mstp_brg_id_t));
    p_db_port->port_designatedPortId= 0;
    p_db_port->port_bridgePortId    = 0;
    p_db_port->port_messageAge      = 0;
    p_db_port->port_fwdDelay        = 0;
    p_db_port->port_helloTime       = 0;
    p_db_port->port_maxAge          = 0;
    p_db_port->proposed             = FALSE;
    p_db_port->proposing            = FALSE;
    p_db_port->rcvdBpdu             = FALSE;
    p_db_port->role                 = ROLE_DISABLED;
    p_db_port->selected             = FALSE;
    p_db_port->selectedRole         = ROLE_DISABLED;
    p_db_port->sync                 = FALSE;
    p_db_port->synced               = FALSE;
    p_db_port->tcAck                = FALSE;
    p_db_port->tcProp               = FALSE;
    p_db_port->updtInfo             = FALSE;
    p_db_port->reRooted             = FALSE;
    p_db_port->oper_rootguard       = FALSE;
    p_db_port->oper_loopguard       = FALSE;
    p_db_port->ppm_state            = 0;
    p_db_port->pim_state            = PIM_DISABLED;
    p_db_port->prt_state            = PRT_INIT_PORT;
    p_db_port->pst_state            = PST_DISCARDING;
    p_db_port->tcm_state            = TCM_INACTIVE;
    p_db_port->bpdu_loop_count      = 0;

    return PM_E_NONE;
}

static int32
_mstp_api_set_default_disabled_port(tbl_mstp_port_t *p_db_port)
{
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    p_db_port->disputed             = FALSE;
    p_db_port->fdbFlush             = FALSE;
    p_db_port->forward              = FALSE;
    p_db_port->forwarding           = FALSE;
    p_db_port->infoIs               = MSTP_INFO_DISABLED;
    p_db_port->learn                = FALSE;
    p_db_port->learning             = FALSE;
    p_db_port->newInfo              = FALSE;
    sal_memset(&p_db_port->port_rootId, 0, sizeof(mstp_brg_id_t));
    p_db_port->port_rootPathCost    = 0;
    sal_memset(&p_db_port->port_regionRootId, 0, sizeof(mstp_brg_id_t));
    p_db_port->port_intRootPathCost = 0;
    sal_memset(&p_db_port->port_designatedBridgeId, 0, sizeof(mstp_brg_id_t));
    p_db_port->port_designatedPortId= 0;
    p_db_port->port_bridgePortId    = 0;
    p_db_port->port_messageAge      = 0;
    p_db_port->port_fwdDelay        = 0;
    p_db_port->port_helloTime       = 0;
    p_db_port->port_maxAge          = 0;
    p_db_port->proposed             = FALSE;
    p_db_port->proposing            = FALSE;
    p_db_port->rcvdBpdu             = FALSE;
    p_db_port->rcvdInternal         = FALSE;
    p_db_port->rcvdInternalChange   = FALSE;
    p_db_port->role                 = ROLE_DISABLED;
    p_db_port->selected             = FALSE;
    p_db_port->selectedRole         = ROLE_DISABLED;
    p_db_port->sendRstp             = FALSE;
    p_db_port->sync                 = FALSE;
    p_db_port->synced               = FALSE;
    p_db_port->tcAck                = FALSE;
    p_db_port->tcProp               = FALSE;
    p_db_port->updtInfo             = FALSE;
    p_db_port->reRooted             = FALSE;
    p_db_port->oper_rootguard       = FALSE;
    p_db_port->oper_loopguard       = FALSE;
    if (p_db_port->forceVersion >= BPDU_VERSION_RSTP)
    {
        p_db_port->rstpVersion = TRUE;
        p_db_port->stpVersion  = FALSE;
    }
    else
    {
        p_db_port->rstpVersion = FALSE;
        p_db_port->stpVersion  = TRUE;
    }
    p_db_port->sendRstp = p_db_port->rstpVersion;
    p_db_port->prx_state            = PRX_DISCARD;
    p_db_port->ppm_state            = 0;
    p_db_port->bdm_state            = BDM_NOT_EDGE;
    p_db_port->ptx_state            = PTX_TRANSMIT_INIT;
    p_db_port->pim_state            = PIM_DISABLED;
    p_db_port->prt_state            = PRT_INIT_PORT;
    p_db_port->pst_state            = PST_DISCARDING;
    p_db_port->tcm_state            = TCM_INACTIVE;
    p_db_port->bpdu_loop_count      = 0;
    p_db_port->txCount              = 0;
    p_db_port->rcvdSTP              = FALSE;
    p_db_port->rcvdRSTP             = FALSE;
    p_db_port->rcvdMSTP             = FALSE;
    
    _mstp_api_update_port_operP2PMac(p_db_port);

    CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
    {
        _mstp_api_set_default_disabled_msti_port(p_db_msti_port);
    }
    
    return PM_E_NONE;
}


static inline uint32
_mstp_api_get_port_path_cost_agg(tbl_interface_t *p_db_if, uint32 bandwidth)
{
#ifdef TODO
    /*bug 30444. huangxt, 2014-11-10*/
    if (p_db_if->mlagid != 0)
        return 20000;
#endif

    if (bandwidth == 0)
        return 200000000;
    else if (bandwidth <= 100)                  /* [0, 100K) */
        return 200000000/bandwidth;
    else if (bandwidth < 10000)                 /* [100K, 10M) */
        return 20000000/(bandwidth/100);
    else if (bandwidth < 100000)                /* [10M, 100M) */
        return 2000000/(bandwidth/10000);
    else if (bandwidth < 1000000)               /* [100M, 1000M) */
        return 200000/(bandwidth/100000);
    else if (bandwidth < 10000000)              /* [1000M, 10G) */
        return 20000/(bandwidth/1000000);
    else if (bandwidth < 100000000)             /* [10G, 100G) */
        return 2000/(bandwidth/10000000);
    else if (bandwidth < 1000000000)            /* [100G, 1T) */
        return 200/(bandwidth/100000000);
    else
        return 20/(bandwidth/1000000000);      
    
}

static int32
_mstp_api_set_start_port_default(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_mstp_port_t port;
    int32 rc = PM_E_NONE;

    /* update port timers */
    p_db_port->port_fwdDelay     = p_db_glb->root_fwdDelay;
    p_db_port->port_helloTime    = p_db_glb->root_helloTime;
    p_db_port->port_maxAge       = p_db_glb->root_maxAge;
    p_db_port->bpdu_loop_count   = 0;
    p_db_port->hopCount          = p_db_glb->bridge_maxHops;
    p_db_port->oper_rootguard    = FALSE;
    p_db_port->oper_loopguard    = FALSE;
    p_db_port->txCount           = 0;
    p_db_port->rcvdSTP           = FALSE;
    p_db_port->rcvdRSTP          = FALSE;
    p_db_port->rcvdMSTP          = FALSE;
    
    /* update forceVersion */
    sal_strncpy(port.key.name, p_db_port->key.name, IFNAME_SIZE);
    port.ifindex = p_db_port->ifindex;
    port.adminForceVersion = p_db_port->adminForceVersion;
    rc = mstp_api_set_port_admin_force_version(&port);

    return rc;
}

static int32
_mstp_api_set_start_msti_port_default(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    int32 rc = PM_E_NONE;

    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (NULL == p_db_inst)
    {
        return PM_E_NONE;
    }
    
    /* update port timers */
    p_db_port->port_fwdDelay     = p_db_inst->root_fwdDelay;
    p_db_port->port_helloTime    = p_db_inst->root_helloTime;
    p_db_port->port_maxAge       = p_db_inst->root_maxAge;
    p_db_port->bpdu_loop_count   = 0;
    p_db_port->hopCount          = p_db_glb->bridge_maxHops;
    p_db_port->oper_rootguard    = FALSE;
    p_db_port->oper_loopguard    = FALSE;
    
    return rc;
}

/* check and add a msti port */
int32
mstp_api_port_check_add_instance(tbl_interface_t *p_db_if, tbl_mstp_port_t *p_db_port, uint32 new_instance)
{
    tbl_msti_port_t *p_db_msti = NULL;
    tbl_msti_port_t msti;
    int32 rc = PM_E_NONE;
    
    sal_memset(&msti, 0, sizeof(msti));
    sal_memcpy(msti.key.name, p_db_port->key.name, IFNAME_SIZE);
    msti.key.instance = new_instance;

    p_db_msti = tbl_msti_port_get_msti_port(&msti.key);
    if (p_db_msti)
    {
        return PM_E_NONE;
    }

    rc = mstp_api_add_msti_port(p_db_if, p_db_port, new_instance);
    return rc;
}

/* This function calculates the default path cost for a port
   based on the link speed.  See Table 8-5 802.1t.  */
uint32
mstp_api_get_port_path_cost(tbl_interface_t *p_db_if, uint32 bandwidth)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    uint32 bandwidth_kbyte = 0;

    bandwidth_kbyte = bandwidth;
    
    switch (p_db_glb->pathcost_standard)
    {
    case MSTP_PATH_COST_1D_1998:
        {
            /* Bandwidth is in bytes/sec - Table 8-5 802.1d 1998. */
            if (bandwidth_kbyte <= 100)              /* [0, 100K) */
                return 65535;    
            else if (bandwidth_kbyte < 10000)        /* [100K, 10M) */
                return 1000;  
            else if (bandwidth_kbyte < 20000)        /* [10M, 20M) */
                return 100;
            else if (bandwidth_kbyte < 100000)       /* [20M, 100M) */
                return 95;
            else if (bandwidth_kbyte < 200000)       /* [100M, 200M) */
                return 19;       
            else if (bandwidth_kbyte < 1000000)      /* [200M, 1000M) */
                return 15;
            else if (bandwidth_kbyte < 2000000)      /* [1000M, 2000M) */
                return 4;
            else if (bandwidth_kbyte < 10000000)     /* [2000M, 10G) */
                return 3;
/* modified by liwh for bug 37702, 2016-05-09 */
#if 0
            else if (bandwidth_kbyte < 20000000)     /* [10G, 20G) */
                return 2;
#endif
            else if (bandwidth_kbyte < 40000000)     /* [10G, 40G) */
                return 2;
            else if (bandwidth_kbyte < 100000000)    /* [40G, 100G) */
                return 2;
/* liwh end */
            else                                     /* [100G, ...) */
                return 1;
        }
        break;
        
    case MSTP_PATH_COST_1T:
        {
            if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
            {
                 return _mstp_api_get_port_path_cost_agg(p_db_if, bandwidth_kbyte);
            }
            
            /* Bandwidth is in bytes/sec - Table 8-5 is in bits/sec.
             N.B. We handle floating port roundoff by adding a fudge factor
             to the constants. Values 2 thru 5 should be changed to 65535
             to support bridges which only support 16 bit path cost values. */
            if (bandwidth_kbyte <= 100)                  /* [0, 100K) */
                return 200000000;
            else if (bandwidth_kbyte < 10000)            /* [100K, 10M) */
                return 20000000;
            else if (bandwidth_kbyte < 100000)           /* [10M, 100M) */
                return 2000000;
            else if (bandwidth_kbyte < 1000000)          /* [100M, 1000M) */
                return 200000;
            else if (bandwidth_kbyte < 10000000)         /* [1000M, 10G) */
                return 20000;
/* modified by liwh for bug 37702, 2016-05-09 */
#if 0
            else if (bandwidth_kbyte < 100000000)        /* [10G, 100G) */
                return 2000;
            else if (bandwidth_kbyte < 1000000000)       /* [100G, 1T) */
                return 200;
#endif
            else if (bandwidth_kbyte < 40000000)         /* [10G, 40G) */
                return 2000;
            else if (bandwidth_kbyte < 100000000)        /* [40G, 100G) */
                return 500;
            else if (bandwidth_kbyte < 1000000000)       /* [100G, 1T) */
                return 200;
/* liwh end */
            else
                return 20;        
        }
        break;
        
    default:
        return 20000;
        break;
    }
}

uint16
mstp_api_get_port_id(tbl_mstp_port_t *p_port)
{
    return (uint16)((p_port->ifindex & 0x0fff) | ((p_port->priority << 8) & 0xf000));
}

uint16
mstp_api_get_msti_port_id(tbl_mstp_port_t *p_port, tbl_msti_port_t *p_msti_port)
{
    return (uint16)((p_port->ifindex & 0x0fff) | ((p_msti_port->priority << 8) & 0xf000));
}

static int32
_mstp_api_set_port_default(tbl_interface_t *p_db_if, tbl_mstp_port_t *p_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    p_port->portPathCost    = mstp_api_get_port_path_cost(p_db_if, p_db_if->oper_bandwidth);
    p_port->priority    = MSTP_DEF_PORT_PRIORITY;
    p_port->portId          = mstp_api_get_port_id(p_port);
    // forward_transitions = 0;
    // tx_bpdu_count = 0;
    // tx_error_count = 0;
    // rx_bpdu_count = 0;
    // rx_error_count = 0;
    // newInfo = 0;
    // newInfoMsti = 0;
    p_port->port_enable     = TRUE;
    // bpdu_loop_count = 0;
    p_port->restricted_role = FALSE;
    p_port->restricted_tcn  = FALSE;
    p_port->admin_bpduguard = MSTP_EDGEPORT_DEFAULT;
    p_port->admin_bpdufilter = MSTP_EDGEPORT_DEFAULT;
    p_port->admin_rootguard = FALSE;
    p_port->oper_rootguard  = FALSE;
    p_port->admin_loopguard = FALSE;
    p_port->oper_loopguard  = FALSE;
    p_port->rcvdBpdu        = FALSE;
    p_port->rcvdInternal    = FALSE;
    // cisco_cfg_format_id = 0;

    p_port->adminP2PMac     = MSTP_LINK_AUTO;

    /* modified by liwh for bug 37629, 2016-05-09 */
    /*p_port->operP2PMac      = (GLB_DUPLEX_FULL == p_db_if->oper_duplex) ? TRUE : FALSE;*/
    p_port->operP2PMac = (GLB_DUPLEX_HALF == p_db_if->oper_duplex) ? FALSE : TRUE;
    /* liwh end */
    
    p_port->adminForceVersion = BPDU_VERSION_DEFAULT;
    p_port->forceVersion    = 0;
    p_port->sendRstp        = FALSE;
    p_port->proposed        = FALSE;
    p_port->proposing       = FALSE;                
    p_port->agree           = FALSE;
    p_port->agreed          = FALSE;
    p_port->sync            = FALSE;
    p_port->synced          = FALSE;
    p_port->tcAck           = FALSE;
    p_port->tcProp          = FALSE;
    p_port->tcAck           = FALSE;
    p_port->mcheck          = FALSE;
    p_port->role            = ROLE_DISABLED;
    p_port->hopCount        = p_db_glb->bridge_maxHops;
    p_port->txCount         = 0;

    return PM_E_NONE;
}

#define _MSTP_MSTI_PORT_API_

int32
mstp_make_bridge_id(mstp_brg_id_t *bridge_id, uint8 *address, uint16 new_priority, uint32 instance_id)
{
    /* Settable priority comprises only of four most sigificant bits */
    sal_memcpy(bridge_id->address, address, MAC_ADDR_LEN);
    bridge_id->priority = ((new_priority & 0xF000) | (instance_id & 0x0FFF));
    
    return PM_E_NONE;
}

int32
mstp_make_bridge_id_priority(mstp_brg_id_t *bridge_id, uint16 new_priority, uint32 instance_id)
{
    /* Settable priority comprises only of four most sigificant bits */
    bridge_id->priority = (new_priority & 0xF000) | (instance_id & 0x0FFF);
    
    return PM_E_NONE;
}

int32
_mstp_api_set_port_msti_default(tbl_interface_t *p_db_if, tbl_mstp_port_t *p_db_port, tbl_msti_port_t *p_msti)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    
    p_msti->portPathCost    = mstp_api_get_port_path_cost(p_db_if, p_db_if->oper_bandwidth);
    p_msti->priority        = MSTP_DEF_PORT_PRIORITY;
    p_msti->portId          = mstp_api_get_msti_port_id(p_db_port, p_msti);
    p_msti->restricted_role = FALSE;
    p_msti->restricted_tcn  = FALSE;
    p_msti->proposed        = FALSE;
    p_msti->proposing       = FALSE;                
    p_msti->agree           = FALSE;
    p_msti->agreed          = FALSE;
    p_msti->sync            = FALSE;
    p_msti->synced          = FALSE;
    p_msti->tcAck           = FALSE;
    p_msti->tcProp          = FALSE;
    p_msti->tcAck           = FALSE;
    p_msti->role            = ROLE_DISABLED;
    p_msti->hopCount        = p_db_glb->bridge_maxHops;
    
    return PM_E_NONE;
}

static int32
_mstp_api_start_msti_port(tbl_msti_port_t *p_db_port)
{
    int32 rc = PM_E_NONE;

    _mstp_api_set_start_msti_port_default(p_db_port);
    
    msti_sm_port_start_pst(p_db_port);  /* CIST/MSTI port, should before BDM */
    msti_sm_port_start_pim(p_db_port);  /* CIST/MSTI port */
    msti_sm_port_start_prt(p_db_port);  /* CIST/MSTI port */
    msti_sm_port_start_tcm(p_db_port);  /* CIST/MSTI port */

    /* modified by liwh for bug 37722, 2016-08-16 */
    msti_newTcWhile(p_db_port);
    /* liwh end */

    mstp_api_reselect_msti_port(p_db_port);
    
    return rc;
}

static int32
_mstp_api_msti_port_del_timer(tbl_msti_port_t *p_db_port)
{
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    CTC_TASK_STOP_TIMER(p_db_port->mdelayWhile_timer);
    CTC_TASK_STOP_TIMER(p_db_port->rbWhile_timer);
    CTC_TASK_STOP_TIMER(p_db_port->rcvdInfoWhile_timer);
    CTC_TASK_STOP_TIMER(p_db_port->rrWhile_timer);
    CTC_TASK_STOP_TIMER(p_db_port->tcWhile_timer);
    CTC_TASK_STOP_TIMER(p_db_port->hold_timer);
    
    return PM_E_NONE;
}

static int32
_mstp_api_stop_msti_port(tbl_msti_port_t *p_db_port, uint32 force)
{
    int32 rc = PM_E_NONE;

    msti_sm_port_stop_pim(p_db_port);
    msti_sm_port_stop_prt(p_db_port);
    msti_sm_port_stop_pst(p_db_port);
    if (force)
    {
        /* down port should also run TCM state */
        msti_sm_port_stop_tcm(p_db_port);
    }

    _mstp_api_msti_port_del_timer(p_db_port);
    
    _mstp_api_set_default_disabled_msti_port(p_db_port);

    return rc;
}

int32
mstp_api_add_msti_port(tbl_interface_t *p_db_if, tbl_mstp_port_t *p_db_port, uint32 instance)
{
    tbl_mstp_global_t *p_db_global = tbl_mstp_global_get_mstp_global();
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    tbl_msti_port_t *p_db_msti_port = NULL;
    tbl_msti_port_t msti;

    inst_key.instance = instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (NULL == p_db_inst)
    {
        return PM_E_NONE;
    }
    
    sal_memset(&msti, 0, sizeof(msti));
    sal_memcpy(msti.key.name, p_db_port->key.name, IFNAME_SIZE);
    msti.key.instance = instance;
    _mstp_api_set_port_msti_default(p_db_if, p_db_port, &msti);
    msti.p_if = p_db_if;
    msti.p_port = p_db_port;
    msti.ifindex = p_db_port->ifindex;
    tbl_msti_port_add_msti_port(&msti);

    p_db_msti_port = tbl_msti_port_get_msti_port(&msti.key);
    if (NULL == p_db_msti_port)
    {
        return PM_E_NONE;
    }
    tbl_mstp_port_set_msti_list(p_db_port, TRUE, p_db_msti_port);
    tbl_mstp_instance_set_port_list(p_db_inst, TRUE, p_db_msti_port);
    p_db_msti_port->bpdu = NULL;

    if (p_db_port->portEnabled)
    {
        _mstp_api_start_msti_port(p_db_msti_port);
    }
    else
    {
        /* if global_disable or port_disable, need set to FEA stp state to forward */
        if (!p_db_global->enable || !p_db_port->port_enable)
        {
            mstp_api_set_msti_fei_state(p_db_msti_port, GLB_BR_PORT_STATE_DISABLED);
        }
    }

    return PM_E_NONE;
}

int32
mstp_api_del_msti_port(tbl_interface_t *p_db_if, tbl_mstp_port_t *p_db_port, uint32 instance)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    tbl_msti_port_t *p_db_msti_port = NULL;
    tbl_msti_port_key_t msti_key;
    int32 rc = PM_E_NONE;
    
    inst_key.instance = instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (NULL == p_db_inst)
    {
        return PM_E_NONE;
    }
    
    sal_memset(&msti_key, 0, sizeof(msti_key));
    sal_memcpy(msti_key.name, p_db_port->key.name, IFNAME_SIZE);
    msti_key.instance = instance;

    p_db_msti_port = tbl_msti_port_get_msti_port(&msti_key);
    if (NULL == p_db_msti_port)
    {
        return PM_E_NONE;
    }

    _mstp_api_stop_msti_port(p_db_msti_port, TRUE);
    p_db_msti_port->bpdu = NULL;
    tbl_mstp_port_set_msti_list(p_db_port, FALSE, p_db_msti_port);
    tbl_mstp_instance_set_port_list(p_db_inst, FALSE, p_db_msti_port);
    PM_E_RETURN(tbl_msti_port_del_msti_port(&p_db_msti_port->key));

    mstp_api_msti_reselect(p_db_inst);

    return PM_E_NONE;
}

static int32
_mstp_api_msti_set_default(tbl_mstp_instance_t *p_db_inst)
{
    int32 rc = PM_E_NONE;

    p_db_inst->bridge_priority      = MSTP_DEF_BRIDGE_PRIORITY;
    PM_E_RETURN(tbl_mstp_instance_set_mstp_instance_field(p_db_inst, TBL_MSTP_INSTANCE_FLD_BRIDGE_PRIORITY));

    return PM_E_NONE;
}

tbl_msti_port_t*
mstp_api_get_msti_port(tbl_mstp_port_t *p_db_port, uint16 instance_id)
{
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
    {
        if (p_db_msti_port->key.instance == instance_id)
        {
            return p_db_msti_port;
        }
    }

    return NULL;
}

int32
mstp_api_set_msti_port_path_cost(tbl_msti_port_t *p_port, tbl_msti_port_t *p_db_port)
{
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. process */
    if (p_port->adminPortPathCost)
    {
        p_port->portPathCost = p_port->adminPortPathCost;
    }
    else
    {
        sal_memset(&if_key, 0, sizeof(if_key));
        sal_strncpy(if_key.name, p_port->key.name, IFNAME_SIZE);
        p_db_if = tbl_interface_get_interface(&if_key);
        if (!p_db_if)
        {
            SAL_ASSERT(0);
        }
        p_port->portPathCost = mstp_api_get_port_path_cost(p_db_if, p_db_if->oper_bandwidth);
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_msti_port_set_msti_port_field(p_port, TBL_MSTI_PORT_FLD_ADMINPORTPATHCOST));
    PM_E_RETURN(tbl_msti_port_set_msti_port_field(p_port, TBL_MSTI_PORT_FLD_PORTPATHCOST));

    mstp_api_reselect_msti_port(p_db_port);

    return rc;
}

int32
mstp_api_set_msti_port_priority(tbl_msti_port_t *p_port, tbl_msti_port_t *p_db_port)
{
    tbl_mstp_port_t *p_mstp_port = NULL;
    int32 rc = PM_E_NONE;

    /* 1. process */
    p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    p_port->portId = mstp_api_get_msti_port_id(p_mstp_port, p_port);

    /* 2. update local CDB */
    PM_E_RETURN(tbl_msti_port_set_msti_port_field(p_port, TBL_MSTI_PORT_FLD_PRIORITY));
    PM_E_RETURN(tbl_msti_port_set_msti_port_field(p_port, TBL_MSTI_PORT_FLD_PORTID));
    
    mstp_api_reselect_msti_port(p_db_port);
    
    return rc;    
}

int32
mstp_api_set_msti_port_restricted_role(tbl_msti_port_t *p_port)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_msti_port_set_msti_port_field(p_port, TBL_MSTI_PORT_FLD_RESTRICTED_ROLE));
    return rc;    
}

int32
mstp_api_set_msti_port_restricted_tcn(tbl_msti_port_t *p_port)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_msti_port_set_msti_port_field(p_port, TBL_MSTI_PORT_FLD_RESTRICTED_TCN));
    return rc;    
}

#define _MSTP_MSTI_API_

int32
_mstp_api_msti_init(tbl_mstp_instance_t *p_db_inst)
{
    _mstp_api_msti_set_default(p_db_inst);
    msti_sm_port_start_prs(p_db_inst);
    //msti_updtRolesTree(p_db_inst);
    
    return PM_E_NONE;
}

tbl_mstp_instance_t*
mstp_api_add_msti(tbl_mstp_instance_t *p_inst)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_mstp_instance_t default_inst;
    tbl_mstp_instance_t *p_db_inst = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&default_inst, 0, sizeof(default_inst));
    default_inst.key.instance = p_inst->key.instance;
    sal_memcpy(&default_inst.bridge_id, &p_db_glb->bridge_id, sizeof(mstp_brg_id_t));
    mstp_make_bridge_id(&default_inst.bridge_id, p_db_glb->bridge_id.address, MSTP_DEF_BRIDGE_PRIORITY, default_inst.key.instance);
    rc = tbl_mstp_instance_add_mstp_instance(&default_inst);
    if (rc < 0)
    {
        return NULL;
    }
    
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&p_inst->key);
    if (NULL == p_db_inst)
    {
        return NULL;
    }

    _mstp_api_msti_init(p_db_inst);

    return p_db_inst;
}

int32
mstp_api_del_msti(tbl_mstp_instance_t *p_inst)
{
    tbl_mstp_instance_t *p_db_inst = NULL;
    int32 rc = PM_E_NONE;

    /* not remove default instance */
    if (MSTP_DEFAULT_INSTANCE == p_inst->key.instance)
    {
        return PM_E_NONE;        
    }

    p_db_inst = tbl_mstp_instance_get_mstp_instance(&p_inst->key);
    if (NULL == p_db_inst)
    {
        return PM_E_NONE;
    }
    
    PM_E_RETURN(tbl_mstp_instance_del_mstp_instance(&p_db_inst->key));

    return PM_E_NONE;
}

int32
mstp_api_set_msti_priority(tbl_mstp_instance_t *p_db_inst, uint32 priority)
{
    int32 rc = PM_E_NONE;
    
    p_db_inst->bridge_priority = priority;
    tbl_mstp_instance_set_mstp_instance_field(p_db_inst, TBL_MSTP_INSTANCE_FLD_BRIDGE_PRIORITY);

    /* update bridgeID */
    mstp_make_bridge_id_priority(&p_db_inst->bridge_id, (uint16)p_db_inst->bridge_priority, p_db_inst->key.instance);
    PM_E_RETURN(tbl_mstp_instance_set_mstp_instance_field(p_db_inst, TBL_MSTP_INSTANCE_FLD_BRIDGE_ID));

    mstp_api_msti_reselect(p_db_inst);

    return PM_E_NONE;
}

#define _MSTP_PORT_API_

int32
mstp_api_set_port_admin_force_version(tbl_mstp_port_t *p_port)
{
    int32 rc = PM_E_NONE;

    /* 1. update local CDB */
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_ADMINFORCEVERSION));
    PM_E_RETURN(mstp_api_set_port_force_version(p_port));
    
    return rc;
}

int32
mstp_api_set_port_force_version(tbl_mstp_port_t *p_port)
{
    tbl_mstp_global_t *p_glb = tbl_mstp_global_get_mstp_global();
    tbl_mstp_port_t *p_db_port = NULL;
    int32 rc = PM_E_NONE;

    p_db_port = tbl_mstp_port_get_mstp_port(&p_port->key);
    if (NULL == p_db_port)
    {
        return PM_E_NOT_EXIST;
    }

    /* calc forceVersion */
    if (BPDU_VERSION_DEFAULT == p_port->adminForceVersion)
    {
        switch (p_glb->type)
        {
        case MSTP_TYPE_STP:
            p_port->forceVersion = BPDU_VERSION_STP;
            break;
        case MSTP_TYPE_RSTP:
            p_port->forceVersion = BPDU_VERSION_RSTP;
            break;
        case MSTP_TYPE_MSTP:
            p_port->forceVersion = BPDU_VERSION_MSTP;
            break;
        default:
            break;
        }
    }
    else
    {
        p_port->forceVersion = p_port->adminForceVersion;
    }
    
    /* 1. update local CDB */
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_FORCEVERSION));

    /* 2. process */
    if (p_port->forceVersion >= BPDU_VERSION_RSTP)
    {
        p_port->rstpVersion = TRUE;
        p_port->stpVersion  = FALSE;
    }
    else
    {
        p_port->rstpVersion = FALSE;
        p_port->stpVersion  = TRUE;
    }

    /* update rstpVersion/stpVersion */
    if (p_port->rstpVersion != p_db_port->rstpVersion)
    {
        PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_RSTPVERSION));
        PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_STPVERSION));

        /* only when portEnabled, should start ppm; otherwise maybe result in wild-pointer when mstp_port removed */
        if (p_db_port->portEnabled)
        {
            mstp_sm_port_stop_ppm(p_db_port);
            mstp_sm_port_start_ppm(p_db_port);
        }
    }

    return rc;
}

int32
mstp_api_set_port_path_cost(tbl_mstp_port_t *p_port)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. process */
    if (p_port->adminPortPathCost)
    {
        p_port->portPathCost = p_port->adminPortPathCost;
    }
    else
    {
        p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&p_port->key);
        if (!p_db_if)
        {
            SAL_ASSERT(0);
        }
        p_port->portPathCost = mstp_api_get_port_path_cost(p_db_if, p_db_if->oper_bandwidth);
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_ADMINPORTPATHCOST));
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_PORTPATHCOST));

    mstp_api_reselect();
    mstp_api_port_reselect_all_msti(p_port);

    mstp_api_all_port_tx_pdu();
    
    return rc;
}

static int32
_mstp_api_start_port(tbl_mstp_port_t *p_db_port)
{
    int32 rc = PM_E_NONE;

    _mstp_api_set_start_port_default(p_db_port);
    _mstp_api_update_port_operP2PMac(p_db_port);
    
    mstp_sm_port_start_pti(p_db_port);  /* only CIST port */
    mstp_sm_port_start_prx(p_db_port);  /* only CIST port */
    mstp_sm_port_start_ppm(p_db_port);  /* only CIST port */
    mstp_sm_port_start_pst(p_db_port);  /* CIST/MSTI port, should before BDM */
    mstp_sm_port_start_bdm(p_db_port);  /* only CIST port */
    mstp_sm_port_start_pim(p_db_port);  /* CIST/MSTI port */
    mstp_sm_port_start_prt(p_db_port);  /* CIST/MSTI port */
    mstp_sm_port_start_tcm(p_db_port);  /* CIST/MSTI port */
    mstp_sm_port_start_ptx(p_db_port);  /* only CIST port */

    /* modified by liwh for bug 37722, 2016-08-16 */
    newTcWhile(p_db_port);
    /* liwh end */

    return rc;
}

static int32
_mstp_api_port_del_timer(tbl_mstp_port_t *p_db_port)
{
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    CTC_TASK_STOP_TIMER(p_db_port->port_timer);
    CTC_TASK_STOP_TIMER(p_db_port->edgeDelayWhile_timer);
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    CTC_TASK_STOP_TIMER(p_db_port->helloWhen_timer);
    CTC_TASK_STOP_TIMER(p_db_port->mdelayWhile_timer);
    CTC_TASK_STOP_TIMER(p_db_port->rbWhile_timer);
    CTC_TASK_STOP_TIMER(p_db_port->rcvdInfoWhile_timer);
    CTC_TASK_STOP_TIMER(p_db_port->rrWhile_timer);
    CTC_TASK_STOP_TIMER(p_db_port->tcWhile_timer);
    CTC_TASK_STOP_TIMER(p_db_port->hold_timer);

    CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
    {
        _mstp_api_msti_port_del_timer(p_db_msti_port);
    }

    return PM_E_NONE;
}

static int32
_mstp_api_stop_port(tbl_mstp_port_t *p_db_port, uint32 force)
{
    int32 rc = PM_E_NONE;

    mstp_sm_port_stop_pti(p_db_port);
    mstp_sm_port_stop_prx(p_db_port);
    mstp_sm_port_stop_ppm(p_db_port);
    mstp_sm_port_stop_bdm(p_db_port);
    mstp_sm_port_stop_ptx(p_db_port);
    mstp_sm_port_stop_pim(p_db_port);
    mstp_sm_port_stop_prt(p_db_port);
    mstp_sm_port_stop_pst(p_db_port);
    if (force)
    {
        /* down port should also run TCM state */
        mstp_sm_port_stop_tcm(p_db_port);
    }

    _mstp_api_set_default_disabled_port(p_db_port);
    return rc;
}

static uint32
_mstp_api_calc_port_enabled(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_interface_t *p_db_if = (tbl_interface_t*)p_db_port->p_if;

    /*
     portEnabled need 3 conditions
     1) global enable
     2) port enable
     3) port running
     */
    if (p_db_glb->enable 
     && p_db_port->port_enable
     && GLB_FLAG_ISSET(p_db_if->flags, GLB_IFF_RUNNING)
     && !INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

int32
mstp_api_update_port_enabled(tbl_mstp_port_t *p_db_port, uint32 force)
{
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;    
    uint32 old_portEnabled = FALSE;
    uint32 new_portEnabled = FALSE;

    old_portEnabled = p_db_port->portEnabled;
    new_portEnabled = _mstp_api_calc_port_enabled(p_db_port);

    if (old_portEnabled == new_portEnabled)
    {
        return PM_E_NONE;
    }

    p_db_port->portEnabled = new_portEnabled;
    tbl_mstp_port_set_mstp_port_field(p_db_port, TBL_MSTP_PORT_FLD_PORTENABLED);

    if (new_portEnabled)
    {
        _mstp_api_start_port(p_db_port);
#if 0
        /* Removed by kcao for bug 42762 2017-03-14, msti pst has begin twice times, remove it */
        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
        {
            _mstp_api_start_msti_port(p_db_msti_port);
        }
#endif
    }
    else
    {
        /* Added by kcao 2019-02-21 for bug 51045, if port link down trigger other ports to send TC */
        if (PST_DISCARDING != p_db_port->pst_state)
        {
            setTcPropTree(p_db_port);
        }

        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
        {
            _mstp_api_stop_msti_port(p_db_msti_port, force);
        }
        
        _mstp_api_stop_port(p_db_port, force);
    }

    mstp_api_reselect();
    mstp_api_port_reselect_all_msti(p_db_port);

    return PM_E_NONE;
}

int32
_mstp_api_set_port_all_msti_fei_state(tbl_mstp_port_t *p_db_port, uint32 state)
{
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
    {
        mstp_api_set_msti_fei_state(p_db_msti_port, state);
    }

    return PM_E_NONE;
}
        
int32
mstp_api_set_port_enable(tbl_mstp_port_t *p_db_port, uint32 enable, uint32 del_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    uint32 is_enable = FALSE;
    int32 rc = PM_E_NONE;

    p_db_port->port_enable = enable;
    /* 1. update local CDB */
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_db_port, TBL_MSTP_PORT_FLD_PORT_ENABLE));

    is_enable = (p_db_glb->enable && p_db_port->port_enable);
    if (is_enable)
    {
        mstp_api_set_port_fei_state(p_db_port, GLB_BR_PORT_STATE_BLOCKING);
        _mstp_api_set_port_all_msti_fei_state(p_db_port, GLB_BR_PORT_STATE_BLOCKING);
    }

    /* 2. process */
    rc = mstp_api_update_port_enabled(p_db_port, TRUE);

    if (!is_enable)
    {
        /* delete port, keep port state for LAG member */
        if (!del_port)
        {
            mstp_api_set_port_fei_state(p_db_port, GLB_BR_PORT_STATE_FORWARDING);
            _mstp_api_set_port_all_msti_fei_state(p_db_port, GLB_BR_PORT_STATE_FORWARDING);
        }
    }

    return rc;    
}

int32
mstp_api_set_port_priority(tbl_mstp_port_t *p_port)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    p_port->portId = mstp_api_get_port_id(p_port);

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_PRIORITY));
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_PORTID));
    
    mstp_api_reselect();
    mstp_api_port_reselect_all_msti(p_port);
    
    return rc;    
}

int32
mstp_api_set_port_bpdufilter(tbl_mstp_port_t *p_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    int32 rc = PM_E_NONE;

    /* 1. process */
    if (MSTP_EDGEPORT_DEFAULT == p_port->admin_bpdufilter)
    {
        p_port->oper_bpdufilter = p_db_glb->bpdufilter ? MSTP_EDGEPORT_ENABLED : MSTP_EDGEPORT_DISABLED;
    }
    else
    {
        p_port->oper_bpdufilter = p_port->admin_bpdufilter;
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_ADMIN_BPDUFILTER));
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_OPER_BPDUFILTER));
    return rc;    
}

int32
mstp_api_set_port_bpduguard(tbl_mstp_port_t *p_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    int32 rc = PM_E_NONE;

    /* 1. process */
    if (MSTP_EDGEPORT_DEFAULT == p_port->admin_bpduguard)
    {
        p_port->oper_bpduguard = p_db_glb->bpduguard ? MSTP_EDGEPORT_ENABLED : MSTP_EDGEPORT_DISABLED;
    }
    else
    {
        p_port->oper_bpduguard = p_port->admin_bpduguard;
    }
    
    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_ADMIN_BPDUGUARD));
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_OPER_BPDUGUARD));
    return rc;    
}

int32
mstp_api_set_port_rootguard(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port)
{
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_ADMIN_ROOTGUARD));

    /* modified by liwh for bug 36699/37625/37767, 2016-05-11 */
    if (p_db_port->admin_rootguard)
    {
        if ((ROLE_ROOTPORT == p_db_port->role)
            || (ROLE_ALTERNATE == p_db_port->role)
            || (ROLE_BACKUP == p_db_port->role))
        {
            p_db_port->oper_rootguard = TRUE;
            p_db_port->infoIs = MSTP_INFO_AGED;
            p_db_port->reselect = TRUE;
            p_db_port->selected = FALSE;
            if (PST_FORWARDING == p_db_port->pst_state)
            {
                mstp_sm_pst_process_event(p_db_port, PST_EVT_NOTFORWARD);
            }
            mstp_api_reselect();
            mstp_api_port_reselect_all_msti(p_db_port);
        }

        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
        {
            if ((ROLE_ROOTPORT == p_db_msti_port->role)
                || (ROLE_ALTERNATE == p_db_msti_port->role)
                || (ROLE_BACKUP == p_db_msti_port->role)
                || (ROLE_MASTERPORT == p_db_msti_port->role))
            {
                p_db_msti_port->oper_rootguard = TRUE;
                p_db_msti_port->infoIs = MSTP_INFO_AGED;
                p_db_msti_port->reselect = TRUE;
                p_db_msti_port->selected = FALSE;
                if (PST_FORWARDING == p_db_msti_port->pst_state)
                {
                    msti_sm_pst_process_event(p_db_msti_port, PST_EVT_NOTFORWARD);
                }
                mstp_api_reselect_msti_port(p_db_msti_port);
            }
        }
    }
    else
    {
        if (p_db_port->oper_rootguard)
        {
            p_db_port->oper_rootguard = FALSE;
            mstp_api_reselect();
            mstp_api_port_reselect_all_msti(p_db_port);
        }

        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
        {
            if (p_db_msti_port->oper_rootguard)
            {
                p_db_msti_port->oper_rootguard = FALSE;
                mstp_api_reselect_msti_port(p_db_msti_port);
            }
        }        
    }
    /* liwh end */
    
    return rc;    
}

extern int32
_mstp_sm_PRT_DESIGNATED_DISCARD(tbl_mstp_port_t *p_db_port, char *ifname_ext);

int32
mstp_api_set_port_loopguard(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    char msti_port_name[IFNAME_EXT_SIZE];
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    int32 rc = PM_E_NONE;

    /* 1. process */
    IFNAME_ETH2FULL(p_port->key.name, ifname_ext);

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_ADMIN_LOOPGUARD));

    if (p_db_port->oper_loopguard)
    {
        _mstp_sm_PRT_DESIGNATED_DISCARD(p_db_port, ifname_ext);
        p_db_port->oper_loopguard = FALSE;

        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
        {
            if (p_db_msti_port->oper_loopguard)
            {
                MSTP_MSTI_PORT_STR(msti_port_name, p_db_msti_port);
                msti_sm_PRT_DESIGNATED_DISCARD(p_db_msti_port, msti_port_name);
                p_db_msti_port->oper_loopguard = FALSE;
            }
        }
        mstp_api_reselect();
        mstp_api_port_reselect_all_msti(p_db_port);
    }
    
    return rc;    
}

int32
mstp_api_set_port_admin_edge(tbl_mstp_port_t *p_port)
{
    tbl_mstp_port_t *p_db_port = NULL;
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_ADMINEDGE));

    p_db_port = tbl_mstp_port_get_mstp_port(&p_port->key);
    if (p_db_port)
    {
        if (p_port->adminEdge)
        {
            mstp_sm_bdm_process_event(p_db_port, BDM_EVT_BEGIN_ADMINEDGE);
        }
        else
        {
            mstp_sm_bdm_process_event(p_db_port, BDM_EVT_BEGIN_NOTADMINEDGE);
        }
    }
    
    return rc;    
}

int32
mstp_api_set_port_admin_p2pmac(tbl_mstp_port_t *p_port)
{
    tbl_mstp_port_t *p_db_port = NULL;
    int32 rc = PM_E_NONE;

    /* 1. update local CDB */
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_ADMINP2PMAC));

    /* 2. process */
    if (MSTP_LINK_AUTO == p_port->adminP2PMac)
    {
        p_db_port = tbl_mstp_port_get_mstp_port(&p_port->key);
        if (p_db_port)
        {
            _mstp_api_update_port_operP2PMac(p_db_port);
        }
    }
    else
    {
        p_port->operP2PMac = (MSTP_LINK_P2P == p_port->adminP2PMac) ? TRUE : FALSE;
        PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_OPERP2PMAC));
    }

    return rc;    
}

int32
mstp_api_set_port_restricted_role(tbl_mstp_port_t *p_port)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_RESTRICTED_ROLE));
    return rc;    
}

int32
mstp_api_set_port_restricted_tcn(tbl_mstp_port_t *p_port)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_port, TBL_MSTP_PORT_FLD_RESTRICTED_TCN));
    return rc;    
}

int32
mstp_api_check_add_msti_port(tbl_interface_t *p_db_if)
{
    ds_brgif_t *p_db_brgif = NULL; 
    tbl_mstp_port_t *p_db_port = NULL;
    vid_t vid = 0;
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;

    p_db_brgif = p_db_if->brgif;
    if (NULL == p_db_brgif)
    {
        return PM_E_NONE;
    }

    p_db_port = p_db_if->mstp_port;
    if (NULL == p_db_port)
    {
        return PM_E_NONE;
    }

    if (GLB_VLAN_PORT_TYPE_ACCESS == p_db_if->vlan_type)
    {
        vlan_key.vid = p_db_brgif->pvid;
        p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
        if (p_db_vlan)
        {
            if (GLB_DEFAULT_INSTANCE != p_db_vlan->instance)
            {
                mstp_api_port_check_add_instance(p_db_if, p_db_port, p_db_vlan->instance);
            }
        }
    }
    else if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
    {
        GLB_BMP_ITER_BEGIN(p_db_brgif->allowed_vlan.vlan_bmp, vid)
        {
            vlan_key.vid = vid;
            p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
            if (p_db_vlan)
            {
                if (GLB_DEFAULT_INSTANCE != p_db_vlan->instance)
                {
                    mstp_api_port_check_add_instance(p_db_if, p_db_port, p_db_vlan->instance);
                }
            }
        }
        GLB_BMP_ITER_END(p_db_brgif->allowed_vlan.vlan_bmp, vid);
    }
    
    return PM_E_NONE;    
}

int32
mstp_api_add_port(tbl_interface_t *p_db_if)
{
    tbl_mstp_port_t port;
    tbl_mstp_port_t *p_db_port = NULL;

    sal_memset(&port, 0, sizeof(port));
    sal_strcpy(port.key.name, p_db_if->key.name);
    port.ifindex = p_db_if->ifindex;

    _mstp_api_set_port_default(p_db_if, &port);
    tbl_mstp_port_add_mstp_port(&port);
    p_db_port = tbl_mstp_port_get_mstp_port(&port.key);
    p_db_port->bpdu = XCALLOC(MEM_MSTP_PDU, sizeof(bpdu_t));
    p_db_if->mstp_port = p_db_port;
    p_db_port->p_if = p_db_if;

    mstp_api_set_port_enable(p_db_port, TRUE, FALSE);
    _mstp_api_set_start_port_default(p_db_port);

    mstp_api_check_add_msti_port(p_db_if);

    return PM_E_NONE;
}

int32
mstp_api_del_port(tbl_interface_t *p_db_if)
{
    tbl_mstp_port_t *p_db_port = NULL;
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    uint32 instance = 0;
    
    p_db_port = (tbl_mstp_port_t*)p_db_if->mstp_port;
    if (NULL == p_db_port)
    {
        return PM_E_NONE;
    }

    if (p_db_port->portEnabled)
    {
        _mstp_api_stop_port(p_db_port, TRUE);
    }
    _mstp_api_port_del_timer(p_db_port);

    mstp_api_set_port_enable(p_db_port, FALSE, TRUE);
    XFREE(MEM_MSTP_PDU, p_db_port->bpdu);
    p_db_if->mstp_port = NULL;
    p_db_port->bpdu = NULL;

    CTCLIB_SLIST_LOOP_DEL(p_db_port->msti_list.obj_list, p_db_msti_port, listnode, next)
    {
        instance = p_db_msti_port->key.instance;
        mstp_api_del_msti_port(p_db_if, p_db_port, instance);
    }
    
    p_db_port->p_if = NULL;
    return tbl_mstp_port_del_mstp_port(&p_db_port->key);
}

int32
mstp_api_clear_all_disabled_port()
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    tbl_interface_t *p_db_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    int32 rc = 0;

    /* 2. update all portEnabled state */
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        /* Added by kcao for bug 41317 2016-10-25, bypass MLAG peer link */
        rc = swth_api_is_mlag_peer_link(p_db_port->ifindex);
        if (TRUE == rc)
        {
            continue;
        }
        p_db_if = (tbl_interface_t*)p_db_port->p_if;
        if (p_db_if && p_db_if->erps_enable)
        {
            continue;
        }
        mstp_api_set_port_enable(p_db_port, TRUE, FALSE);
    }
    
    return PM_E_NONE;
}

int32
mstp_api_clear_counters(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    ctclib_slistnode_t *listnode = NULL;

    if (p_db_port)
    {
        p_db_port->tx_bpdu_count = 0;
        p_db_port->tx_error_count = 0;
        p_db_port->rx_bpdu_count = 0;
        p_db_port->rx_error_count = 0;
    }
    else
    {
        /* 2. update all portEnabled state */
        CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
        {
            p_db_port->tx_bpdu_count = 0;
            p_db_port->tx_error_count = 0;
            p_db_port->rx_bpdu_count = 0;
            p_db_port->rx_error_count = 0;
        }
    }
    
    return PM_E_NONE;
}

int32
mstp_api_set_port_mcheck(tbl_mstp_port_t *p_db_port)
{
#if 0
    /* remove for case mstp_oam_38 */
    /* Setting mcheck has no effect if stpVersion (17.20.12) is TRUE */
    if (p_db_port->stpVersion)
    {
        return PM_E_NONE;
    }
#endif

    p_db_port->mcheck = TRUE;
    mstp_sm_ppm_process_event(p_db_port, PPM_EVT_MCHECK);
    mstp_sm_port_do_mcheck(p_db_port);
    p_db_port->mcheck = FALSE;

    return PM_E_NONE;
}

int32
mstp_api_update_if_running(tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    tbl_mstp_port_t *p_db_port = NULL;

    p_db_port = (tbl_mstp_port_t*)p_db_if->mstp_port;

    if (NULL == p_db_port)
    {
        return PM_E_NONE;
    }

    /* 2. process */
    rc = mstp_api_update_port_enabled(p_db_port, FALSE);
    return rc;
}

int32
mstp_api_update_if_path_cost(tbl_interface_t *p_db_if)
{
    tbl_mstp_port_t *p_db_port = p_db_if->mstp_port;
    tbl_msti_port_t *p_db_msti_port = NULL;
    uint32 default_pathcost = 0;
    ctclib_slistnode_t *listnode = NULL;    
    int32 rc = PM_E_NONE;

    /* 1. process */
    if (NULL == p_db_port)
    {
        return PM_E_NONE;
    }

    /* 2. update local CDB */
    default_pathcost = mstp_api_get_port_path_cost(p_db_if, p_db_if->oper_bandwidth);
    if (0 == p_db_port->adminPortPathCost)
    {
        p_db_port->portPathCost = default_pathcost;
        PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_db_port, TBL_MSTP_PORT_FLD_PORTPATHCOST));
    }

    CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
    {
        if (0 == p_db_msti_port->adminPortPathCost)
        {
            p_db_msti_port->portPathCost = default_pathcost;
            PM_E_RETURN(tbl_msti_port_set_msti_port_field(p_db_msti_port, TBL_MSTI_PORT_FLD_PORTPATHCOST));
        }
    }

    mstp_api_reselect();
    mstp_api_port_reselect_all_msti(p_db_port);
    
    return PM_E_NONE;
}

int32
mstp_api_update_if_p2p_mac(tbl_interface_t *p_db_if)
{
    tbl_mstp_port_t *p_db_port = p_db_if->mstp_port;

    /* only process adminP2PMac is auto */
    if (NULL == p_db_port || MSTP_LINK_AUTO != p_db_port->adminP2PMac)
    {
        return PM_E_NONE;
    }

    return _mstp_api_update_port_operP2PMac(p_db_port);
}

int32
mstp_api_flush_fdb(tbl_mstp_port_t *p_db_port)
{
    char ifname_ext[IFNAME_SIZE];
    
    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    MSTP_DEBUG(STP_EVENT, "Interface %s flush FDB", ifname_ext);

    return brg_fdb_flush_dynamic_fdb_port(p_db_port->ifindex);
}

#define _MSTP_INSTANCE_API_

static int32
_mstp_gen_digest(uint8 *data, uint32 len, uint8 *digest)
{
    auth_hmac_md5(data, len, CONFIGURATION_DIGEST_SIGNATURE_KEY, MSTP_DIGEST_LEN, digest);

    return PM_E_NONE;
}

/* Called if instance/vlan mapping changes */
int32
mstp_api_update_digest()
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_mstp_instance_master_t *p_db_master = tbl_mstp_instance_get_master();
    tbl_mstp_instance_t *p_db_inst = NULL;
    ctclib_slistnode_t *listnode = NULL;
    mstp_digest_t new_digest;
    uint16 config_table[4096];
    uint16 instance_id = 0;
    vid_t vid = 0;
    
    /*Initialize -set all vids to 0 */
    sal_memset(config_table, 0, sizeof(config_table));
    sal_memset(&new_digest, 0, sizeof(new_digest));

    CTCLIB_SLIST_LOOP(p_db_master->inst_list, p_db_inst, listnode)
    {
        instance_id = (uint16)p_db_inst->key.instance;
        GLB_BMP_ITER_BEGIN(p_db_inst->vlan, vid)
        {
            config_table[vid] = sal_hton16(instance_id);
        }
        GLB_BMP_ITER_END(p_db_inst->vlan, vid);
    }

    _mstp_gen_digest((uint8*)config_table, 4096*2, (uint8*)new_digest.data);

    if (sal_memcmp(p_db_glb->mstp_digest.data, new_digest.data, MSTP_DIGEST_LEN))
    {
        sal_memcpy(p_db_glb->mstp_digest.data, new_digest.data, MSTP_DIGEST_LEN);
        _mstp_api_reinit_enable();
    }

    return PM_E_NONE;
}

/* check a port is belong to a instance, the removed_vid is will removed in future and need bypassed */
int32
mstp_api_port_is_this_instance(tbl_interface_t *p_db_if, uint32 instance, vid_t removed_vid)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    ds_brgif_t *p_db_brgif = p_db_if->brgif;
    vid_t vid = 0;

    inst_key.instance = instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (NULL == p_db_inst)
    {
        return PM_E_FAIL;
    }

    GLB_BMP_ITER_BEGIN(p_db_inst->vlan, vid)
    {
        if (GLB_BMP_ISSET(p_db_brgif->allowed_vlan.vlan_bmp, vid))
        {
            if (vid == removed_vid)
            {
                continue;
            }
            return PM_E_NONE;
        }
    }
    GLB_BMP_ITER_END(p_db_inst->vlan, vid);

    return PM_E_FAIL;
}

/* check and remove a msti port */
int32
mstp_api_port_check_remove_instance(tbl_interface_t *p_db_if, tbl_mstp_port_t *p_db_port, uint32 old_instance, vid_t removed_vid)
{
    int32 rc = PM_E_NONE;
    
    rc = mstp_api_port_is_this_instance(p_db_if, old_instance, removed_vid);
    if (rc < 0)
    {
        rc = mstp_api_del_msti_port(p_db_if, p_db_port, old_instance);
    }
    
    return rc;
}

static int32
_mstp_api_instance_vlan_change(tbl_vlan_t *p_db_vlan, uint32 old_instance)
{
    tbl_mstp_port_t *p_db_port = NULL;
    tbl_interface_t *p_db_if = NULL;
    uint32 ifindex = 0;

    /* send to all members */
    GLB_BMP_ITER_BEGIN(p_db_vlan->member_port, ifindex)
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
        if (!p_db_if)
        {
            continue;
        }

        if (INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            continue;
        }
        
        p_db_port = tbl_mstp_port_get_mstp_port((tbl_mstp_port_key_t*)&p_db_if->key);
        if (!p_db_port)
        {
            continue;
        }

        if (MSTP_DEFAULT_INSTANCE != old_instance)
        {
            mstp_api_port_check_remove_instance(p_db_if, p_db_port, old_instance, p_db_vlan->key.vid);
        }

        if (MSTP_DEFAULT_INSTANCE != p_db_vlan->instance)
        {
            mstp_api_port_check_add_instance(p_db_if, p_db_port, p_db_vlan->instance);
        }
    }
    GLB_BMP_ITER_END(p_db_vlan->member_port, ifindex);

    return PM_E_NONE;
}

tbl_mstp_instance_t*
mstp_api_instance_lookup_vlan(vid_t vid)
{
    tbl_mstp_instance_master_t *p_master = tbl_mstp_instance_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_mstp_instance_t *p_db_inst = NULL;

    CTCLIB_SLIST_LOOP(p_master->inst_list, p_db_inst, listnode)
    {
        if (GLB_BMP_ISSET(p_db_inst->vlan, vid))
        {
            return p_db_inst;
        }
    }

    return NULL;
}

/* vlan create process */
int32
mstp_api_instance_add_vlan(vid_t vid)
{
    tbl_mstp_instance_t inst;
    tbl_mstp_instance_t *p_db_inst = NULL;

    inst.key.instance = MSTP_DEFAULT_INSTANCE;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst.key);
    if (NULL == p_db_inst)
    {
        return PM_E_FAIL;
    }

    /* set instance DB */
    GLB_BMP_SET(p_db_inst->vlan, vid);
    tbl_mstp_instance_set_mstp_instance_field(p_db_inst, TBL_MSTP_INSTANCE_FLD_VLAN);

    mstp_api_update_digest();

    return PM_E_NONE;
}

/* vlan delete process */
int32
mstp_api_instance_del_vlan(vid_t vid)
{
    tbl_mstp_instance_t *p_db_inst = NULL;
    uint32 has_vlan = FALSE;

    p_db_inst = mstp_api_instance_lookup_vlan(vid);
    if (NULL == p_db_inst)
    {
        return PM_E_FAIL;
    }

//    _mstp_api_instance_vlan_change(p_db_vlan, p_db_inst->key.instance);
    
    /* clear instance DB */
    GLB_BMP_UNSET(p_db_inst->vlan, vid);
    tbl_mstp_instance_set_mstp_instance_field(p_db_inst, TBL_MSTP_INSTANCE_FLD_VLAN);

    GLB_BMP_ITER_BEGIN(p_db_inst->vlan, vid)
    {
        has_vlan = TRUE;
    }
    GLB_BMP_ITER_END(p_db_inst->vlan, vid);
    
    if (!has_vlan)
    {
        mstp_api_del_msti(p_db_inst);
    }
    mstp_api_update_digest();

    return PM_E_NONE;
}

/* vlan delete process */
int32
mstp_api_of_del_vlan(vid_t vid)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    int32 rc = PM_E_NONE;

    if (GLB_BMP_ISSET(p_db_glb->of_include_vlan_bmp, vid))
    {
        GLB_BMP_UNSET(p_db_glb->of_include_vlan_bmp, vid);
        PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_db_glb, TBL_MSTP_GLOBAL_FLD_OF_INCLUDE_VLAN_BMP));
    }
    
    return PM_E_NONE;
}

/* vlan change instance process */
int32
mstp_api_instance_change_to_vlan(vid_t vid, tbl_mstp_instance_t *p_db_inst_new)
{
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_mstp_instance_t *p_db_inst = NULL;
    tbl_mstp_instance_t *p_old_db_inst = NULL;
    tbl_mstp_instance_key_t old_inst_key;
    uint32 old_instance = 0;
    uint32 has_vlan = FALSE;

    p_db_inst = mstp_api_instance_lookup_vlan(vid);
    if (NULL == p_db_inst)
    {
        return PM_E_FAIL;
    }
    vlan_key.vid = vid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        return PM_E_FAIL;
    }
    
    /* clear instance DB, set is in bitmap */
    GLB_BMP_UNSET(p_db_inst->vlan, vid);
    tbl_mstp_instance_set_mstp_instance_field(p_db_inst, TBL_MSTP_INSTANCE_FLD_VLAN);

    old_instance = p_db_vlan->instance;
    p_db_vlan->instance = p_db_inst_new->key.instance;
    tbl_vlan_set_vlan_field(p_db_vlan, TBL_VLAN_FLD_INSTANCE);

    _mstp_api_instance_vlan_change(p_db_vlan, old_instance);

    /* Added by kcao 2016-12-15 for bug 42131 */
    if (GLB_DEFAULT_INSTANCE != old_instance)
    {
        old_inst_key.instance = old_instance;
        p_old_db_inst = tbl_mstp_instance_get_mstp_instance(&old_inst_key);
        if (p_old_db_inst)
        {
            GLB_BMP_ITER_BEGIN(p_db_inst->vlan, vid)
            {
                has_vlan = TRUE;
            }
            GLB_BMP_ITER_END(p_db_inst->vlan, vid);
            if (!has_vlan)
            {
                mstp_api_del_msti(p_old_db_inst);
            }
        }
    }

    return PM_E_NONE;
}

/* vlan change instance process */
int32
mstp_api_instance_change_to_default_vlan(vid_t vid, tbl_mstp_instance_t *p_db_inst_old)
{
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_mstp_instance_key_t key;
    tbl_mstp_instance_t *p_db_inst_default = NULL;
    uint32 old_instance = 0;

    key.instance = MSTP_DEFAULT_INSTANCE;
    p_db_inst_default = tbl_mstp_instance_get_mstp_instance(&key);
    if (NULL == p_db_inst_default)
    {
        return PM_E_FAIL;
    }
    vlan_key.vid = vid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        return PM_E_FAIL;
    }
   
    /* set instance DB, clear is in bitmap */
    GLB_BMP_SET(p_db_inst_default->vlan, vid);
    tbl_mstp_instance_set_mstp_instance_field(p_db_inst_default, TBL_MSTP_INSTANCE_FLD_VLAN);
    
    old_instance = p_db_vlan->instance;
    p_db_vlan->instance = MSTP_DEFAULT_INSTANCE;
    tbl_vlan_set_vlan_field(p_db_vlan, TBL_VLAN_FLD_INSTANCE);

    _mstp_api_instance_vlan_change(p_db_vlan, old_instance);

    /* should clear vid from old instance */
    GLB_BMP_UNSET(p_db_inst_old->vlan, vid);
    
    return PM_E_NONE;
}

int32
mstp_api_instance_set_vlan_db(tbl_mstp_instance_t *p_inst)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_instance_set_mstp_instance_field(p_inst, TBL_MSTP_INSTANCE_FLD_VLAN));

    mstp_api_update_digest();

    return PM_E_NONE;
}

#define _MSTP_GLOBAL_API_

int32
mstp_api_set_glb_tc_protection(tbl_mstp_global_t *p_glb)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_TC_PROTECTION));
    return rc;  
}

int32
mstp_api_set_glb_tx_hold_count(tbl_mstp_global_t *p_glb)
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    int32 rc = PM_E_NONE;

    /* 1. process */
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        p_db_port->txCount = 0;
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_TXHOLDCOUNT));
    return rc;  
}

int32
mstp_api_tx(tbl_mstp_port_t *p_db_src_port)
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (p_db_port->portEnabled)
        {
            if (p_db_port->newInfo)
            {
                if (p_db_port->sendRstp)
                {
                    mstp_sm_send_bpdu(p_db_port);
                }
                else
                {
                    mstp_sm_ptx_process_event(p_db_port, PTX_EVT_SENDCONFIG);
                }
            }
        }
    }

    return PM_E_NONE;
}

int32
mstp_api_all_port_tx_pdu()
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (p_db_port->portEnabled)
        {
            if (p_db_port->sendRstp)
            {
                mstp_sm_send_bpdu(p_db_port);
            }
            else
            {
                mstp_sm_ptx_process_event(p_db_port, PTX_EVT_SENDCONFIG);
            }
        }
    }

    return PM_E_NONE;
}

int32
mstp_api_set_glb_priority(tbl_mstp_global_t *p_glb)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    int32 rc = PM_E_NONE;

    /* 1. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_BRIDGE_PRIORITY));

    /* update bridgeID */
    mstp_make_bridge_id_priority(&p_db_glb->bridge_id, (uint16)p_glb->bridge_priority, 0);
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_db_glb, TBL_MSTP_GLOBAL_FLD_BRIDGE_ID));

    if (p_db_glb->enable)
    {
        /* trigger reslect */
        mstp_api_reselect();
        mstp_api_reselect_all_msti();
    }
    else
    {
        /* update priority */
        updtRolesTree();
    }

    mstp_api_all_port_tx_pdu();
    
    return rc;
}

static int32
_mstp_api_update_all_ports_pathcost()
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_interface_t *p_db_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_mstp_port_t *p_db_port = NULL;
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode_msti = NULL;
    uint32 portPathCost = 0;
    
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        p_db_port->adminPortPathCost = 0;
        p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&p_db_port->key);
        if (!p_db_if)
        {
            SAL_ASSERT(0);
        }
        portPathCost = mstp_api_get_port_path_cost(p_db_if, p_db_if->oper_bandwidth);
        p_db_port->portPathCost = portPathCost;
        tbl_mstp_port_set_mstp_port_field(p_db_port, TBL_MSTP_PORT_FLD_PORTPATHCOST);
        tbl_mstp_port_set_mstp_port_field(p_db_port, TBL_MSTP_PORT_FLD_ADMINPORTPATHCOST);

        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode_msti)
        {
            p_db_msti_port->adminPortPathCost = 0;
            p_db_msti_port->portPathCost = portPathCost;
            tbl_msti_port_set_msti_port_field(p_db_msti_port, TBL_MSTI_PORT_FLD_PORTPATHCOST);
            tbl_msti_port_set_msti_port_field(p_db_msti_port, TBL_MSTI_PORT_FLD_ADMINPORTPATHCOST);
        }
    }

    mstp_api_reselect();
    mstp_api_reselect_all_msti();
    
    return PM_E_NONE;
}

int32
mstp_api_set_glb_pathcost_standard(tbl_mstp_global_t *p_glb)
{
    int32 rc = PM_E_NONE;

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_PATHCOST_STANDARD));

    _mstp_api_update_all_ports_pathcost();
    
    return rc;  
}

int32
mstp_api_set_glb_fwd_delay(tbl_mstp_global_t *p_glb)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_BRIDGE_FWDDELAY));

    mstp_api_reselect();
    mstp_api_reselect_all_msti();

    return rc;  
}

int32
mstp_api_set_glb_hello_time(tbl_mstp_global_t *p_glb)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_BRIDGE_HELLOTIME));
    
    mstp_api_reselect();
    mstp_api_reselect_all_msti();
    
    return rc;  
}

int32
mstp_api_set_glb_max_age(tbl_mstp_global_t *p_glb)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXAGE));
    
    mstp_api_reselect();
    mstp_api_reselect_all_msti();
    
    return rc;  
}

int32
_mstp_api_set_glb_max_hops()
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_mstp_port_master_t *p_port_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    tbl_msti_port_t *p_db_msti_port = NULL;
    tbl_mstp_instance_master_t *p_inst_master = tbl_mstp_instance_get_master();
    tbl_mstp_instance_t *p_db_inst = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *listnode1 = NULL;

    if (NULL == p_db_glb->root_port)
    {
        p_db_glb->hopCount = p_db_glb->bridge_maxHops;
        CTCLIB_SLIST_LOOP(p_port_master->port_list, p_db_port, listnode)
        {
            p_db_port->hopCount = p_db_glb->bridge_maxHops;
        }
    }

    CTCLIB_SLIST_LOOP(p_inst_master->inst_list, p_db_inst, listnode)
    {
        if (GLB_DEFAULT_INSTANCE == p_db_inst->key.instance)
        {
            continue;
        }
        
        if (NULL == p_db_inst->root_port)
        {
            p_db_inst->hopCount = p_db_glb->bridge_maxHops;
            CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_msti_port, listnode1)
            {
                p_db_msti_port->hopCount = p_db_glb->bridge_maxHops;
            }
        }
    }

    return PM_E_NONE;
}

int32
mstp_api_set_glb_max_hops(tbl_mstp_global_t *p_glb)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXHOPS));

    _mstp_api_set_glb_max_hops();
    
    return rc;  
}

int32
mstp_api_set_glb_bpduguard(tbl_mstp_global_t *p_glb)
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    int32 rc = PM_E_NONE;

    /* 1. process */
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (MSTP_EDGEPORT_DEFAULT == p_db_port->admin_bpduguard)
        {
            p_db_port->oper_bpduguard = p_glb->bpduguard ? MSTP_EDGEPORT_ENABLED : MSTP_EDGEPORT_DISABLED;
            PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_db_port, TBL_MSTP_PORT_FLD_OPER_BPDUGUARD));
        }
    }
    
    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_BPDUGUARD));
    return rc;  
}

int32
mstp_api_set_glb_bpdufilter(tbl_mstp_global_t *p_glb)
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    int32 rc = PM_E_NONE;

    /* 1. process */
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (MSTP_EDGEPORT_DEFAULT == p_db_port->admin_bpdufilter)
        {
            p_db_port->oper_bpdufilter = p_glb->bpdufilter ? MSTP_EDGEPORT_ENABLED : MSTP_EDGEPORT_DISABLED;
            PM_E_RETURN(tbl_mstp_port_set_mstp_port_field(p_db_port, TBL_MSTP_PORT_FLD_OPER_BPDUFILTER));
        }
    }
    
    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_BPDUFILTER));
    return rc;  
}

static int32
_mstp_api_set_glb_type()
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_mstp_port_t *p_db_port = NULL;
    tbl_mstp_port_t port;
    tbl_mstp_port_t *p_port = &port;
    uint32 stp_enable = p_db_glb->enable;

    if (stp_enable)
    {
        /* if enable, stop old state-machine */
        p_db_glb->enable = FALSE;
        mstp_api_set_glb_enable(p_db_glb, TRUE);
    }
    
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        sal_memcpy(&p_port->key, &p_db_port->key, sizeof(tbl_mstp_port_key_t));
        p_port->adminForceVersion = BPDU_VERSION_DEFAULT;
        mstp_api_set_port_admin_force_version(p_port);

    }

    if (stp_enable)
    {
        /* if enable, start new state-machine */
        p_db_glb->enable = stp_enable;
        mstp_api_set_glb_enable(p_db_glb, FALSE);
    }

    return PM_E_NONE;
}

static int32
_mstp_api_clear_mstp_config(tbl_mstp_global_t *p_db_glb)
{
    tbl_mstp_instance_master_t *p_inst_master = tbl_mstp_instance_get_master();
    tbl_mstp_instance_t *p_db_inst = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_mstp_global_t glb;
    vid_t vid = 0;
    int32 rc = PM_E_NONE;

    glb.mstp_revision_level = MSTP_REVISION_LEVEL_DEF;
    sal_memset(glb.mstp_region_name, 0, sizeof(glb.mstp_region_name));
    mstp_api_set_glb_revision_level(&glb);
    mstp_api_set_glb_region_name(&glb);

    CTCLIB_SLIST_LOOP_DEL(p_inst_master->inst_list, p_db_inst, listnode, next)
    {
        if (MSTP_DEFAULT_INSTANCE == p_db_inst->key.instance)
        {
            continue;
        }
        GLB_BMP_ITER_BEGIN(p_db_inst->vlan, vid)
        {
            mstp_api_instance_change_to_default_vlan(vid, p_db_inst);
        }
        GLB_BMP_ITER_END(p_db_inst->vlan, vid);

        sal_memset(&p_db_inst->vlan, 0, sizeof(vlan_bmp_t));
        PM_E_RETURN(mstp_api_instance_set_vlan_db(p_db_inst));
        PM_E_RETURN(mstp_api_del_msti(p_db_inst));
    }
    
    return PM_E_NONE;
}

int32
mstp_api_clear_mst_config()
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    
    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        _mstp_api_clear_mstp_config(p_db_glb);
    }

    return PM_E_NONE;
}

int32
mstp_api_set_glb_type(tbl_mstp_global_t *p_glb)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    mstp_api_clear_mst_config();

    /* 2. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_TYPE));

    PM_E_RETURN(_mstp_api_set_glb_type());
    
    return rc;  
}

int32
mstp_api_start_sm(uint32 enable)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_mstp_instance_master_t *p_inst_master = tbl_mstp_instance_get_master();
    tbl_mstp_instance_t *p_db_inst = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    if (enable)
    {
        mstp_sm_port_start_prs(p_db_glb);
    }
    else
    {
        mstp_sm_port_stop_prs(p_db_glb);
    }

    CTCLIB_SLIST_LOOP(p_inst_master->inst_list, p_db_inst, listnode)
    {
        if (enable)
        {
            msti_sm_port_start_prs(p_db_inst);
        }
        else
        {
            msti_sm_port_stop_prs(p_db_inst);
        }
    }
        
    return PM_E_NONE;
}

int32
mstp_api_set_glb_enable(tbl_mstp_global_t *p_glb, uint32 force_blocking)
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    int32 rc = PM_E_NONE;

    /* 1. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_ENABLE));

    /* 2. update all portEnabled state */
    if (p_glb->enable)
    {
        mstp_api_set_global_enable_fei(p_glb->enable, FALSE);
    }
    
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        mstp_api_update_port_enabled(p_db_port, TRUE);
    }

    if (!p_glb->enable)
    {
        mstp_api_set_global_enable_fei(p_glb->enable, force_blocking);
    }
    
    /* 3. start Port Role Selection Machine */
    mstp_api_start_sm(p_glb->enable);

    /* 4. clear topo change state */
    if (!p_glb->enable)
    {
        mstp_sm_update_topo_change();
    }

    return rc;
}

int32
mstp_api_set_glb_region_name(tbl_mstp_global_t *p_glb)
{
    int32 rc = PM_E_NONE;

    /* 1. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_MSTP_REGION_NAME));

    _mstp_api_reinit_enable();

    return rc;
}

int32
mstp_api_set_glb_revision_level(tbl_mstp_global_t *p_glb)
{
    int32 rc = PM_E_NONE;

    /* 1. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_MSTP_REVISION_LEVEL));

    _mstp_api_reinit_enable();

    return rc;
}

int32
mstp_api_set_glb_of_exclude_all_vlan(tbl_mstp_global_t *p_glb)
{
    int32 rc = PM_E_NONE;

    /* 1. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_OF_EXCLUDE_ALL_VLAN));

    return rc;
}

int32
mstp_api_update_system_mac(uint8 *system_mac)
{
    tbl_mstp_global_t *p_glb = tbl_mstp_global_get_mstp_global();
    
    sal_memcpy(p_glb->bridge_id.address, system_mac, MAC_ADDR_LEN);

    return PM_E_NONE;
}

/*
 reselect when config params changes or port join/leave STP calculation,
 should be called after the CDB params change
 */
int32
mstp_api_reselect()
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();

    return mstp_sm_prs_process_event(p_db_glb, PRS_EVT_RESELECT);
}

int32
mstp_api_reselect_all_msti()
{
    tbl_mstp_instance_master_t *p_master = tbl_mstp_instance_get_master();
    tbl_mstp_instance_t *p_db_inst = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_master->inst_list, p_db_inst, listnode)
    {
        if (GLB_DEFAULT_INSTANCE != p_db_inst->key.instance)
        {
            msti_sm_prs_process_event(p_db_inst, PRS_EVT_RESELECT);
        }
    }
    
    return PM_E_NONE;
}

int32
mstp_api_port_reselect_all_msti(tbl_mstp_port_t *p_port)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    tbl_mstp_port_t *p_db_port = NULL;
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;

    p_db_port = tbl_mstp_port_get_mstp_port(&p_port->key);
    if (NULL == p_db_port)
    {
        return PM_E_NONE;
    }
    
    CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
    {
        inst_key.instance = p_db_msti_port->key.instance;
        p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
        if (p_db_inst)
        {
            if (GLB_DEFAULT_INSTANCE != p_db_inst->key.instance)
            {
                msti_sm_prs_process_event(p_db_inst, PRS_EVT_RESELECT);
            }
        }
    }

    return PM_E_NONE;
}

int32
mstp_api_reselect_msti_port(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;

    inst_key.instance = p_db_port->key.instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (p_db_inst)
    {
        if (GLB_DEFAULT_INSTANCE != p_db_inst->key.instance)
        {
            msti_sm_prs_process_event(p_db_inst, PRS_EVT_RESELECT);
        }
    }

    return PM_E_NONE;
}

/*
 reselect when config params changes or port join/leave STP calculation,
 should be called after the CDB params change
 */
int32
mstp_api_msti_reselect(tbl_mstp_instance_t *p_db_inst)
{
    return msti_sm_prs_process_event(p_db_inst, PRS_EVT_RESELECT);
}

int32
mstp_api_set_port_fei_state(tbl_mstp_port_t *p_db_port, uint32 state)
{
    char ifname_ext[IFNAME_SIZE];
    
    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    MSTP_DEBUG(STP_EVENT, "Interface %s change to %s state",
        ifname_ext, 
        cdb_enum_val2str(glb_br_port_state_strs, GLB_BR_PORT_STATE_MAX, state));

    p_db_port->fea_state = state;
    return tbl_mstp_port_set_mstp_port_field(p_db_port, TBL_MSTP_PORT_FLD_FEA_STATE);
#if 0    
    return fei_brg_set_port_state(p_db_if->ifindex, instance, state);
#endif
}

int32
mstp_api_set_msti_fei_state(tbl_msti_port_t *p_db_port, uint32 state)
{
    char ifname_ext[IFNAME_SIZE];
    
    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    MSTP_DEBUG(STP_EVENT, "Interface %s instance %u change to %s state",
        ifname_ext, 
        p_db_port->key.instance,
        cdb_enum_val2str(glb_br_port_state_strs, GLB_BR_PORT_STATE_MAX, state));

    p_db_port->fea_state = state;
    return tbl_msti_port_set_msti_port_field(p_db_port, TBL_MSTI_PORT_FLD_FEA_STATE);
#if 0    
    return fei_brg_set_port_state(p_db_if->ifindex, instance, state);
#endif
}

int32
mstp_api_set_global_enable_fei(uint32 enable, uint32 force_blocking)
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *listnode1 = NULL;
    uint32 is_enable = FALSE;
    uint32 fea_state = 0;

    /* 2. update all portEnabled state */
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        is_enable = (enable && p_db_port->port_enable);

        if (is_enable)
        {
            fea_state = GLB_BR_PORT_STATE_BLOCKING;
        }
        else
        {
            fea_state = GLB_BR_PORT_STATE_FORWARDING;
            if (force_blocking)
            {
                fea_state = GLB_BR_PORT_STATE_BLOCKING;
            }
        }
        
        mstp_api_set_port_fei_state(p_db_port, fea_state);

        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode1)
        {
            mstp_api_set_msti_fei_state(p_db_msti_port, fea_state);
        }
    }

    return PM_E_NONE;
}

int32
mstp_api_set_mlag_enable(uint32 mlag_enable)
{
    tbl_mstp_global_t *p_mstp_glb = tbl_mstp_global_get_mstp_global();

    p_mstp_glb->mlag_enable = mlag_enable;
    tbl_mstp_global_set_mstp_global_field(p_mstp_glb, TBL_MSTP_GLOBAL_FLD_MLAG_ENABLE);
    
    return PM_E_NONE;
}

int32
mstp_api_set_mlag_role(mlag_role_t mlag_role)
{
    tbl_mstp_global_t *p_mstp_glb = tbl_mstp_global_get_mstp_global();

    p_mstp_glb->mlag_role = mlag_role;
    tbl_mstp_global_set_mstp_global_field(p_mstp_glb, TBL_MSTP_GLOBAL_FLD_MLAG_ROLE);
    
    return PM_E_NONE;
}

int32
mstp_api_allowed_vlan_change_change_cb(tbl_interface_t *p_db_if, vid_t vid, uint32 is_add)
{
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_mstp_port_t *p_db_port = NULL;

    vlan_key.vid = vid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        return PM_E_FAIL;
    }

    p_db_port = tbl_mstp_port_get_mstp_port((tbl_mstp_port_key_t*)&p_db_if->key);
    if (!p_db_port)
    {
        return PM_E_FAIL;
    }

    /* bypass default instance */
    if (MSTP_DEFAULT_INSTANCE == p_db_vlan->instance)
    {
        return PM_E_NONE;
    }
    
    if (is_add)
    {
        mstp_api_port_check_add_instance(p_db_if, p_db_port, p_db_vlan->instance);
    }
    else
    {
        mstp_api_port_check_remove_instance(p_db_if, p_db_port, p_db_vlan->instance, vid);
    }

    return PM_E_NONE;
}

#endif /* CONFIG_MSTP */
