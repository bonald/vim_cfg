
#include "proto.h"
#include "sal.h"
#include "sal_types.h"
#include "glb_ip_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_mrt_define.h"
#include "gen/tbl_mrt.h"
#include "gen/tbl_ipmc_global_define.h"
#include "gen/tbl_ipmc_global.h"
#include "gen/tbl_ipmc_intf_define.h"
#include "gen/tbl_ipmc_intf.h"
#include "gen/tbl_pim_static_rp_define.h"
#include "gen/tbl_pim_static_rp.h"
#include "ipmc.h"
#include "route.h"


static int32
_ipmc_api_get_ipmc_global_cmdstr(char *cmd_str,tbl_ipmc_global_t *p_db_glb, int32 field_id, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (field_id)
    {
    case TBL_IPMC_GLOBAL_FLD_MCAST_ROUTE_EN:
        if (PIM_SPT_INFINITY == p_db_glb->mcast_route_en)
        {
            sal_sprintf(cmd_str, "ip multicast-routing");
        }
        else
        {
            sal_sprintf(cmd_str, "no ip multicast-routing");
        }
        break;

    case TBL_IPMC_GLOBAL_FLD_REGISTER_SUPPRESS_TIME:
        sal_sprintf(cmd_str, "ip pim register-suppress-time %d", p_db_glb->register_suppress_time);
        break;

    case TBL_IPMC_GLOBAL_FLD_KEEP_ALIVE_TIME:
        sal_sprintf(cmd_str, "ip pim keep-alive-timer %d", p_db_glb->keep_alive_time);
        break;

    case TBL_IPMC_GLOBAL_FLD_JOIN_PRUNE_INTERVAL:
        sal_sprintf(cmd_str, "ip pim join-prune-interval %d", p_db_glb->join_prune_interval);
        break;

    case TBL_IPMC_GLOBAL_FLD_PIM_PACKET_PROCESS:
        sal_sprintf(cmd_str, "ip pim packets %d", p_db_glb->pim_packet_process);
        break;

    case TBL_IPMC_GLOBAL_FLD_SWITCHOVER_MODE:
        if (PIM_SPT_INFINITY == p_db_glb->switchover_mode)
        {
            sal_sprintf(cmd_str, "ip pim spt-switchover infinity-and-beyond");
        }
        else
        {
            sal_sprintf(cmd_str, "no ip pim spt-switchover infinity-and-beyond");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Unknown support changed field");
        break;
    }
    
    return rc;
}

int32
ipmc_api_set_ipmc_global_field(tbl_ipmc_global_t *p_db_glb, int32 field_id, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char  cmd_str[128] = {0};

    PM_E_RETURN(tbl_ipmc_global_set_ipmc_global_field(p_db_glb, field_id));

    sal_memset(cmd_str, 0, sizeof(cmd_str));
    _ipmc_api_get_ipmc_global_cmdstr(cmd_str, p_db_glb, field_id, para);
    rt_vty_config_cmd(cmd_str);
    return rc;
}

int32
ipmc_start()
{
    cdb_register_cfg_tbl_cb(TBL_IPMC_GLOBAL, ipmc_cmd_process_global);
    cdb_register_cfg_tbl_cb(TBL_IPMC_INTF, ipmc_cmd_process_intf);
    cdb_register_cfg_tbl_cb(TBL_PIM_STATIC_RP, ipmc_cmd_process_static_rp);
    cdb_register_cfg_tbl_cb(TBL_MRT, ipmc_cmd_process_mrt);

	/*add by zhw for ipmc*/
	cdb_register_cfg_tbl_cb(TBL_PIM_SM,rt_cmd_process_pim_sm);
	cdb_register_cfg_tbl_cb(TBL_PIM_SM_BSR,rt_cmd_process_pim_sm_bsr);
	cdb_register_cfg_tbl_cb(TBL_PIM_SM_RP,rt_cmd_process_pim_sm_rp);
	cdb_register_cfg_tbl_cb(TBL_PIM_SM_RP_GP,rt_cmd_process_pim_sm_rp_gp);
	cdb_register_cfg_tbl_cb(TBL_PIM_DM,rt_cmd_process_pim_dm);	
	cdb_register_cfg_tbl_cb(TBL_IGMP_INTF,rt_cmd_process_igmp_intf);
	cdb_register_cfg_act_cb(ACT_SHOW_IP_MROUTE,rt_cmd_process_show_ip_mroute);
	/*end add zhw*/
    
	return PM_E_NONE;
}

