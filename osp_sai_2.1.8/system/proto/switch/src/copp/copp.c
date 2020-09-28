#include "proto.h"
#include "switch.h"
#include "copp.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_copp_cfg_define.h"
#include "gen/tbl_copp_cfg.h"
#include "lib_acl.h"

int32
copp_cfg_global_set_total_rate(uint32 rate)
{
    int32 rc = PM_E_NONE;
    tbl_copp_cfg_t copp_glb_cfg;
    sal_memset(&copp_glb_cfg, 0 ,sizeof(copp_glb_cfg));
    
    copp_glb_cfg.copp_total_rate = rate;
    PM_E_RETURN(tbl_copp_cfg_set_copp_cfg_field(&copp_glb_cfg, TBL_COPP_CFG_FLD_COPP_TOTAL_RATE));
    return PM_E_NONE;
}

int32
copp_init(void)
{
    int32 rc = PM_E_NONE;
    PM_E_RETURN(copp_cfg_global_set_total_rate(GLB_CPU_TRAFFIC_DEFAULT_RATE));
    
    return PM_E_NONE;
}

int32
copp_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_COPP_CFG, copp_cfg_cmd_process);
    
    return PM_E_NONE;
}

int32
copp_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_COPP_CFG, NULL);

    return PM_E_NONE;
}

