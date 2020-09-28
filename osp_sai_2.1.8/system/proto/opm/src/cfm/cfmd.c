
#include "proto.h"
#include "glb_if_define.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_cfm_global_define.h"
#include "gen/tbl_cfm_global.h"
#include "gen/tbl_cfm_md_define.h"
#include "gen/tbl_cfm_md.h"
#include "gen/tbl_opm_debug_define.h"
#include "gen/tbl_opm_debug.h"
#include "cfmd.h"

bool cfm_debug_is_on(uint32 flag)
{
    tbl_opm_debug_t *p_db_dbg = NULL;

    /* check exist */
    p_db_dbg = tbl_opm_debug_get_opm_debug();
    if (p_db_dbg)
    {
        if (GLB_FLAG_ISSET(p_db_dbg->cfm, flag))
        {
            return TRUE;
        }
    }

    return FALSE;
}

int32
cfm_start(void)
{
    cdb_register_cfg_tbl_cb(TBL_CFM_GLOBAL,  cfm_cmd_process_cfm_global);
    cdb_register_cfg_tbl_cb(TBL_CFM_MD,  cfm_cmd_process_cfm_md);
    cdb_register_cfg_tbl_cb(TBL_CFM_MA,  cfm_cmd_process_cfm_ma);
    cdb_register_cfg_tbl_cb(TBL_CFM_MAID, cfm_cmd_process_cfm_maid);

    cdb_register_cfg_tbl_cb(TBL_CFM_LMEP, cfm_cmd_process_cfm_lmep);
    cdb_register_cfg_tbl_cb(TBL_CFM_RMEP, cfm_cmd_process_cfm_rmep);

    /* modified by liwh for bug 53922, 2019-10-15 */
    cdb_register_cfg_act_cb(ACT_SHOW_CFM, cfm_cmd_show_cfm);
    /* liwh end */

    cdb_register_cfg_act_cb(ACT_SHOW_CFM_DOMAIN, cfm_cmd_show_cfm_domain);
    cdb_register_cfg_act_cb(ACT_SHOW_CFM_CC_CONFIG, cfm_cmd_show_cfm_cc_config);
    cdb_register_cfg_act_cb(ACT_SHOW_CFM_ERRORS, cfm_cmd_show_cfm_errors);
    cdb_register_cfg_act_cb(ACT_CLEAR_CFM_ERRORS, cfm_cmd_clear_cfm_errors);
    cdb_register_cfg_act_cb(ACT_SHOW_CFM_MEP, cfm_cmd_show_cfm_mep);

    cdb_pm_subscribe_tbl(TBL_CFM_RMEP, cfm_cmd_pm_process_cfm_rmep, NULL);
    cdb_pm_subscribe_tbl(TBL_CFM_LMEP, cfm_cmd_pm_process_cfm_lmep, NULL);

    cdb_pm_subscribe_tbl(TBL_VLAN, cfm_vlan_sync, NULL);

    return PM_E_NONE;
}

int32
cfm_stop(void)
{
    return PM_E_NONE;
}

