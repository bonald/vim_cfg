
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_global.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"
#include "switch.h"
#include "brg.h"
#include "mstp.h"
#include "mstp_define.h"
#include "mstp_api.h"
#include "mstp_pdu.h"
#include "mstp_sm.h"

#ifdef CONFIG_MSTP

int32
mstp_init_global()
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_mstp_global_t *p_glb = tbl_mstp_global_get_mstp_global();
    int32 rc = PM_E_NONE;

    p_glb->type = MSTP_DEFAULT_TYPE;
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_TYPE));
    
    p_glb->bridge_maxAge        = MSTP_TIMER_DEF_MAX_AGE;
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXAGE));
    
    p_glb->bridge_helloTime     = MSTP_TIMER_DEF_HELLO_TIME;
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_BRIDGE_HELLOTIME));
    
    p_glb->bridge_fwdDelay      = MSTP_TIMER_DEF_FWD_DELAY;
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_BRIDGE_FWDDELAY));
    
    p_glb->bridge_maxHops       = MSTP_DEF_MAX_HOPS;
    mstp_api_set_glb_max_hops(p_glb);

    p_glb->bridge_priority      = MSTP_DEF_BRIDGE_PRIORITY;
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_BRIDGE_PRIORITY));

    mstp_make_bridge_id_priority(&p_glb->bridge_id, (uint16)p_glb->bridge_priority, 0);
    sal_memcpy(p_glb->bridge_id.address, p_sys_glb->route_mac, MAC_ADDR_LEN);

    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_BRIDGE_ID));
    
    p_glb->last_tc_time = ctc_time_boottime_sec(NULL);
    p_glb->topology_change_detected = FALSE;
    p_glb->bpduguard            = FALSE;
    p_glb->bpdufilter           = FALSE;
    p_glb->admin_cisco          = FALSE;
    p_glb->oper_cisco           = FALSE;
    p_glb->txHoldCount          = MSTP_TX_HOLD_COUNT;
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_TXHOLDCOUNT));
    
    p_glb->tc_protection        = FALSE;
    p_glb->pathcost_standard    = MSTP_DEF_PATH_COST;
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_PATHCOST_STANDARD));

    p_glb->recv_tcn_threshold   = MSTP_DEF_TCN_THRESHOLD;
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_RECV_TCN_THRESHOLD));

    brg_set_brg_max_id(CDB_ID_INSTANCE, GLB_STP_INSTANCE_MAX);

    return PM_E_NONE;
}

int32
mstp_init()
{
    mstp_init_global();
    mstp_sm_init();
    return PM_E_NONE;
}

int32
mstp_start(switch_master_t *master)
{
    tbl_mstp_global_t *p_glb = tbl_mstp_global_get_mstp_global();

    /* forceVersion need this type */    
    p_glb->type = MSTP_DEFAULT_TYPE;
    tbl_mstp_global_set_mstp_global_field(p_glb, TBL_MSTP_GLOBAL_FLD_TYPE);
    
    cdb_register_cfg_tbl_cb(TBL_MSTP_PORT, mstp_cmd_process_mstp_port);
    cdb_register_cfg_tbl_cb(TBL_MSTI_PORT, mstp_cmd_process_msti_port);
    cdb_register_cfg_tbl_cb(TBL_MSTP_INSTANCE, mstp_cmd_process_mstp_instance);
    cdb_register_cfg_tbl_cb(TBL_MSTP_GLOBAL, mstp_cmd_process_mstp_global);
    cdb_register_cfg_act_cb(ACT_CLEAR_STP, mstp_cmd_process_mstp_clear_stp);
    cdb_register_cfg_act_cb(ACT_SHOW_MSTP, mstp_cmd_process_mstp_show_mstp);
    ipc_register_pkt_rx_fn(GLB_PKT_BPDU, mstp_pdu_rx);

    return PM_E_NONE;
}

int32
mstp_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_MSTP_PORT, NULL);
    cdb_register_cfg_tbl_cb(TBL_MSTP_GLOBAL, NULL);
    cdb_register_cfg_act_cb(ACT_CLEAR_STP, NULL);
    ipc_register_pkt_rx_fn(GLB_PKT_BPDU, NULL);

    return PM_E_NONE;
}

#endif /* CONFIG_MSTP */
