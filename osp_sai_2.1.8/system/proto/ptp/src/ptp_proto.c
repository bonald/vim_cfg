
#include "proto.h"

#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"

#include "ptp_error.h"
#include "ptp_ops.h"
#include "ptp_adpt.h"
#include "ptp_util.h"
#include "ptp_desc.h"
#include "ptp_msg_handle.h"
#include "ptp_msg_issue.h"
#include "ptp_bmc.h"
#include "ptp_proto.h"

/* modified by yaom for bug 18501 20110306 */
extern int32 ptp_snmp_state_change_trap(tbl_ptp_port_t* p_port);

static void
_ptp_announce_receipt_time_trigger(void* p_data)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    tbl_ptp_port_t *p_port = p_data ? (tbl_ptp_port_t*)p_data : NULL;
    char ifname_null[IFNAME_SIZE] = {0};
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    
    sal_memset(ifname_null, 0, sizeof(ifname_null));

    if (NULL == g_p_master)
    {
        PTP_LOG_ERR("Ptp global does not exist");
        return ;
    }
    
    if (NULL == p_port)
    {
        PTP_LOG_ERR("Input parameter ptp port is NULL");
        return;
    }

    /*TMR_ANNOUNCE_RECEIPT*/
    PTP_DBG_TMR("Announce-receipt timer expired for %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));

    p_port->ptm_announce_receipt_timer = NULL;
    p_port->bmc_expires = PTP_BMC_EXPIRES_ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES;

    if (ptp_ops_is_oc())
    {
        if (g_p_master->default_ds.slave_only)
        {
            p_port->recommended_state = PTP_STATE_LISTENING;
            ptp_proto_to_state(p_port);
        }
        else
        {
            ptp_bmc_update_m1(p_port);
            p_port->recommended_state = PTP_STATE_MASTER;
            ptp_proto_to_state(p_port);
        }
    }
    else
    {
        if (sal_memcmp(g_p_master->current_slave_port, ifname_null, sizeof(g_p_master->current_slave_port)))
        {
            ptp_bmc_update_m1(p_port);
            p_port->recommended_state = PTP_STATE_MASTER;
            ptp_proto_to_state(p_port);
        }
        else
        {
            ptp_bmc_update_m3(p_port);
            p_port->recommended_state = PTP_STATE_MASTER;
            ptp_proto_to_state(p_port);
        }
    }

    return;
}

static void
_ptp_qualification_trigger(void* p_data)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    tbl_ptp_port_t *p_port = p_data ? (tbl_ptp_port_t*)p_data : NULL;
    if (NULL == p_port)
    {
        PTP_LOG_ERR("Input parameter ptp port is NULL");
        return;
    }

    /*TMR_QUALIFY*/
    PTP_DBG_TMR("Qualification timer expired for %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));

    p_port->ptm_qualification_timer = NULL;
    p_port->bmc_expires = PTP_BMC_EXPIRES_QUALIFICATION_TIMEOUT_EXPIRES;
    p_port->recommended_state = PTP_STATE_MASTER;
    ptp_proto_to_state(p_port);

    return ;
}

void
ptp_announce_receipt_time_restart(tbl_ptp_port_t* p_port)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    
    if (NULL == p_port)
    {
        PTP_LOG_ERR("Input parameter ptp port is NULL");
        return;
    }

    /*TMR_ANNOUNCE_RECEIPT*/
    PTP_DBG_TMR("Restart announce-receipt timer for %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    ptp_util_cancel_timer(&p_port->ptm_announce_receipt_timer);
    p_port->ptm_announce_receipt_timer = ptp_util_add_timer_log_window(
                                    _ptp_announce_receipt_time_trigger,
                                    p_port,
                                    p_port->port_ds.log_announce_interval,
                                    p_port->port_ds.announce_receipt_timeout);

    return;
}

void
ptp_qualification_start(tbl_ptp_port_t* p_port)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    uint8 times = 0;
    
    if (NULL == p_port)
    {
        PTP_LOG_ERR("Input parameter ptp port is NULL");
        return;
    }

    /*TMR_QUALIFY*/
    PTP_DBG_TMR("Start qualification timer for %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    
    ptp_util_cancel_timer(&p_port->ptm_qualification_timer);
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    if (NULL == g_p_master)
    {
        PTP_LOG_ERR("Ptp global does not exist");
        return ;
    }
    
    if (PTP_BMC_CODE_M3 == p_port->bmc_code)
    {
        times = g_p_master->current_ds.steps_removed + 1;
    }

    ptp_util_cancel_timer(&p_port->ptm_qualification_timer);
    p_port->ptm_qualification_timer = ptp_util_add_timer_log_window(_ptp_qualification_trigger, p_port,
                                    p_port->port_ds.log_announce_interval, times);

    return ;
}

void
ptp_proto_to_state(tbl_ptp_port_t* p_port)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    int32 rc = 0;
    tbl_interface_t *p_db_if = NULL;
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    if (NULL == g_p_master)
    {
        PTP_LOG_ERR("Ptp global does not exist");
        return;
    }

    if (NULL == p_port)
    {
        PTP_LOG_ERR("Input parameter ptp port is NULL");
        return;
    }

    IFNAME_ETH2FULL(p_port->key.name, ifname_ext);
    
    if (p_port->recommended_state != p_port->port_ds.port_state)
    {
        PTP_DBG_BMC("Set %s from %s to %s",
                    ifname_ext,
                    ptp_desc_state(p_port->port_ds.port_state),
                    ptp_desc_state(p_port->recommended_state));
    }

    if (PTP_STATE_MASTER == p_port->recommended_state && PTP_STATE_MASTER != p_port->port_ds.port_state)
    {
        if (PTP_BMC_EXPIRES_NONE == p_port->bmc_expires)
        {
            p_port->recommended_state = PTP_STATE_PRE_MASTER;
            PTP_DBG_BMC("Set %s's recommended state from master to %s for bmc expires none",
                    ifname_ext, ptp_desc_state(p_port->recommended_state));
        }

        p_port->bmc_expires = PTP_BMC_EXPIRES_NONE;
    }

    if (PTP_STATE_SLAVE == p_port->recommended_state)
    {
        sal_memcpy(g_p_master->current_slave_port, p_port->key.name, sizeof(p_port->key.name));
        if (PTP_STATE_UNCALIBRATED == p_port->port_ds.port_state)
        {
            if (!p_port->master_clock_selected)
            {
                p_port->recommended_state = PTP_STATE_UNCALIBRATED;
                PTP_DBG_BMC("Set %s's recommended state from slave to %s for not master clock selected",
                    ifname_ext, ptp_desc_state(p_port->recommended_state));
            }
            else if (g_p_master->set_system_time)
            {
                /* change to slave from uncalibrated so we can update the system time
                   Do not care return value because we want the update not affect the further things */
                ptp_adpt_update_system_time(g_p_master->time_prop_ds.current_utc_offset);
            }
        }
        else if (PTP_STATE_SLAVE == p_port->port_ds.port_state)
        {
            if (p_port->slave_change_master || p_port->synchronization_fault)
            {
                p_port->recommended_state = PTP_STATE_UNCALIBRATED;
                PTP_DBG_BMC("Set %s's recommended state from slave to %s for current state is slave, slave_change_master: %d, synchronization_fault: %d",
                    ifname_ext, ptp_desc_state(p_port->recommended_state), p_port->slave_change_master, p_port->synchronization_fault);
            }
        }
        else
        {
            p_port->recommended_state = PTP_STATE_UNCALIBRATED;
            PTP_DBG_BMC("Set %s's recommended state from slave to %s for current state is %s",
                    ifname_ext, ptp_desc_state(p_port->recommended_state), ptp_desc_state(p_port->port_ds.port_state));
        }
    }
    else
    {
        p_port->uncalibrated_update_offset_times = 0;
        p_port->uncalibrated_update_limit_times = 0;
        if (!sal_memcmp(g_p_master->current_slave_port, p_port->key.name, sizeof(g_p_master->current_slave_port)))
        {
            sal_memset(g_p_master->current_slave_port, 0, sizeof(g_p_master->current_slave_port));
        }
    }

    if (PTP_STATE_SLAVE != p_port->recommended_state)
    {
        sal_memset(&g_p_master->c, 0x00, sizeof(g_p_master->c));
        if (PTP_DELAY_MECHANISM_E2E == p_port->port_ds.delay_mechanism)
        {
            sal_memset(&p_port->c, 0x00, sizeof(p_port->c));
        }
    }

    if (p_port->port_ds.port_state == p_port->recommended_state)
    {
        PTP_DBG_BMC("Exit state change for interface %s, because current state is equal to recommended state: %s", ifname_ext, ptp_desc_state(p_port->recommended_state));
        return;
    }
    else
    {
        PTP_DBG_BMC("%s is changing from %s to %s",
                    ifname_ext,
                    ptp_desc_state(p_port->port_ds.port_state),
                    ptp_desc_state(p_port->recommended_state));
        
        /* modified by yaom for bug 18501 20110306 */
        rc = ptp_snmp_state_change_trap(p_port);
        PTP_DBG_BMC("Ptp state change trap ret %d", rc);
    
        p_db_if = tbl_interface_get_interface_by_ifindex(p_port->ifindex);
        if (NULL == p_db_if)
        {
            PTP_DBG_BMC("Exit state change for interface %s does not exist", ifname_ext);
            return;
        }

        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
        logid_sys(LOG_PTP_4_PTP_STATE_CHANGE, ifname_ext, ptp_desc_state(p_port->port_ds.port_state), ptp_desc_state(p_port->recommended_state));
    }

    ptp_trigger_register(p_port);
    ptp_handle_register(p_port);
    ptp_issue_register(p_port);

    /* entering state tasks */
    switch (p_port->recommended_state)
    {
    case PTP_STATE_INITIALIZING:
        ptp_ops_port_init_prop(p_port);
        p_port->port_ds.port_state = p_port->recommended_state;
        p_port->recommended_state = PTP_STATE_LISTENING;
        ptp_proto_to_state(p_port);
        break;

    case PTP_STATE_FAULTY:
    case PTP_STATE_DISABLED:
        ptp_ops_port_init_prop(p_port);
        p_port->port_ds.port_state = p_port->recommended_state;
        break;

    case PTP_STATE_UNCALIBRATED:
        p_port->slave_change_master = FALSE;
        p_port->synchronization_fault = FALSE;
        p_port->master_clock_selected = FALSE;
        p_port->port_ds.port_state = p_port->recommended_state;
        break;

    case PTP_STATE_LISTENING:
    case PTP_STATE_MASTER:
    case PTP_STATE_PRE_MASTER:
    case PTP_STATE_PASSIVE:
    case PTP_STATE_SLAVE:
    default:
        p_port->port_ds.port_state = p_port->recommended_state;
        break;

    }
}

