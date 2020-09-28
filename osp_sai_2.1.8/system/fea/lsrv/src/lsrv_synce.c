#include "lsrv_inc.h"
#include "ctc_lai_synce.h"
#include "gen/tbl_ssm_global_define.h"
#include "gen/tbl_ssm_global.h"
#include "gen/tbl_ssm_port_define.h"
#include "gen/tbl_ssm_port.h"

lsrv_synce_t g_p_lsrv_synce;

extern char *g_lcm_dpll_state_string[];

extern int32
hsrv_ifdb_get_ifindex_by_gportid(uint16 gportid, uint32* p_ifindex);

extern int32
hsrv_ifdb_get_portid(uint32 ifindex, uint16* p_portid);

static inline char*
_lsrv_synce_desc(uint32 value)
{
    return (value) ? "true" : "false";
}

void
lsrv_synce_dpll_state_event_cb(lai_synce_event_t event, lai_synce_info_t* p_synce_info)
{
    tbl_ssm_global_t* p_ssm_glb = tbl_ssm_global_get_ssm_global();

    LSRV_SYNCE_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_synce_info)
    {
       log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_synce_dpll_state_event_cb, synce_info is null!\n");
       return ;
    }
    if (event == LAI_SYNCE_UPDATE_DPLL_STATE)
    {
        LSRV_SYNCE_DEBUG("Synce insert in, dpll state is %s\n", g_lcm_dpll_state_string[p_synce_info->dpll_state]);
        if (p_ssm_glb->dpll_current_status != p_synce_info->dpll_state)
        {
            p_ssm_glb->dpll_current_status = p_synce_info->dpll_state;
            tbl_ssm_global_set_ssm_global_field(p_ssm_glb, TBL_SSM_GLOBAL_FLD_DPLL_CURRENT_STATUS);
        }
    }
    return;
}

int32
lsrv_synce_select_port(uint16 gport, uint32 output)
{
    int32 rc = 0;
    
    uint8 slot = 0;
    uint8 pannelport = 0;
    uint8 pannelsubport = 0;
    uint32 p_ifindex = 0;
    char eth_name[IFNAME_SIZE];
    lai_synce_info_t synce_info;
    lai_synce_api_t*  synce_api;

    LSRV_IF_ERROR_RETURN(hsrv_ifdb_get_ifindex_by_gportid(gport, &p_ifindex));
    LSRV_IF_ERROR_RETURN(if_get_name_by_ifindex(p_ifindex,eth_name,IFNAME_SIZE));
    LSRV_IF_ERROR_RETURN(if_get_slot_port_by_ethname(eth_name, &slot, &pannelport, &pannelsubport));

    sal_memset(&synce_info, 0, sizeof(synce_info));
    synce_info.id = LAI_SYNCE_SELECTED_IFINDEX;
    synce_info.slot_idx = slot;
    synce_info.port_idx = pannelport;
    synce_info.subport_idx = pannelsubport;
    synce_info.status = output;
    synce_info.gport = gport;
    
    PM_E_RETURN(lai_api_query(LAI_API_SYNCE,(void**)&synce_api));
    PM_E_RETURN(synce_api->set_synce_attribute(&synce_info));
    
    return PM_E_NONE;
}


int32
lsrv_synce_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_ssm_global_t *p_ssm_glb = NULL;
    int32 rc = 0;
    uint16 gport = 0;
    lai_synce_api_t*  synce_api;
    lai_synce_info_t  synce_info;
    
    LSRV_SYNCE_DEBUG("Enter into %s.", __FUNCTION__);
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_ssm_glb = (tbl_ssm_global_t*)p_tbl;
        sal_memset(&synce_info, 0, sizeof(synce_info));
        switch (field_id)
        {
            case TBL_SSM_GLOBAL_FLD_SYNCE_ENABLE:
                PM_E_RETURN(lai_api_query(LAI_API_SYNCE,(void**)&synce_api));
                /*1. dump informations*/
                LSRV_SYNCE_DEBUG("p_ssm_glb->synce_enable: %s, g_p_lsrv_synce->global_enable: %s", 
                _lsrv_synce_desc(p_ssm_glb->synce_enable), _lsrv_synce_desc(g_p_lsrv_synce.global_enable));

                /*2. set to ctc_lai_synce*/
                synce_info.id = LAI_SYNCE_GLOBAL_ENBALE;
                synce_info.synce_enable = p_ssm_glb->synce_enable;
                PM_E_RETURN(synce_api->set_synce_attribute(&synce_info));

                /*3. restore*/
                g_p_lsrv_synce.global_enable = p_ssm_glb->synce_enable;
                break;

            case TBL_SSM_GLOBAL_FLD_SELECT_IFINDEX:
                /*1. dump informations*/
                LSRV_SYNCE_DEBUG("p_ssm_glb->select_ifindex: %d, g_p_lsrv_synce->recovery_port: %d", 
                p_ssm_glb->select_ifindex, g_p_lsrv_synce.recovery_port);

                /*2. set to ctc_lai_synce*/
                if (g_p_lsrv_synce.is_recovering && g_p_lsrv_synce.recovery_port)
                {
                    lsrv_synce_select_port(g_p_lsrv_synce.recovery_port, FALSE);
                    g_p_lsrv_synce.is_recovering = FALSE;
                    g_p_lsrv_synce.recovery_port = 0;
                }
                LSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_ssm_glb->select_ifindex, &gport));
                lsrv_synce_select_port(gport, TRUE);

                /*3. restore*/
                g_p_lsrv_synce.is_recovering = TRUE;
                g_p_lsrv_synce.recovery_port = gport;
                break;

            case TBL_SSM_GLOBAL_FLD_HOLDOVER_EN:
                PM_E_RETURN(lai_api_query(LAI_API_SYNCE,(void**)&synce_api));

                /*1. dump informations*/
                LSRV_SYNCE_DEBUG("p_ssm_glb->holdover_en: %s, g_p_lsrv_synce->is_recovering: %s", 
                _lsrv_synce_desc(p_ssm_glb->holdover_en), _lsrv_synce_desc(g_p_lsrv_synce.is_recovering));

                /*2. set to ctc_lai_synce*/
                if (g_p_lsrv_synce.is_recovering && g_p_lsrv_synce.recovery_port)
                {
                    lsrv_synce_select_port(g_p_lsrv_synce.recovery_port, FALSE);
                    g_p_lsrv_synce.is_recovering = FALSE;
                    g_p_lsrv_synce.recovery_port = 0;
                }
                break;

            case TBL_SSM_GLOBAL_FLD_CLOCK_RECOVERY_SELECT:
                PM_E_RETURN(lai_api_query(LAI_API_SYNCE,(void**)&synce_api));
                synce_info.id = LAI_SYNCE_CLOCK_RECOVERY_SELECT_ENABLE;
                synce_info.clock_recovery_select_en = p_ssm_glb->clock_recovery_select;
                PM_E_RETURN(synce_api->set_synce_attribute(&synce_info));
                break;

            default:
                break;
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
lsrv_synce_start()
{
    cdb_pm_subscribe_tbl(TBL_SSM_GLOBAL, lsrv_synce_sync, NULL);
    return PM_E_NONE;
}

