#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ssm_global_define.h"
#include "gen/tbl_ssm_global.h"
#include "gen/tbl_ssm_port_define.h"
#include "gen/tbl_ssm_port.h"
#include "glb_if_define.h"
#include "ssm.h"
#include "ssm_api.h"
#include "ssm_opts.h"
#include "ctc_task.h"
#include "ssm_pdu.h"
#include "ssm_fsm.h"
#include "ssm_desc.h"

char *
ssm_desc_clockid(char *clock_id_str, ssm_clock_id_t clockid)
{
    sal_memset(clock_id_str, 0x00, SSM_CLOCKID_STR_LEN);
    sal_snprintf(clock_id_str, SSM_CLOCKID_STR_LEN, "%.2hhX:%.2hhX:%.2hhX:%.2hhX:%.2hhX:%.2hhX:%.2hhX:%.2hhX",
                clockid[0], clockid[1], clockid[2], clockid[3], clockid[4], clockid[5], clockid[6], clockid[7]);
    return clock_id_str;
}

char *
ssm_desc_enable(uint32 value)
{
    return (value) ? "enable" : "disable";
}

char *
ssm_desc_quality(uint32 value, uint8 desc_mode)
{
    tbl_ssm_global_t* p_ssm_master = tbl_ssm_global_get_ssm_global();
    
    switch (value)
    {
    case SSM_QL_STU:
         return "STU";
    case SSM_QL_PRS:
         return "PRS";
    case SSM_QL_PRC:
         return "PRC";
    case SSM_QL_TNC:
        if (SSM_OPTION_MODE_1 == p_ssm_master->option_mode)
        {
            return "SSU-A";
        }
        else
        {
            return "TNC";
        }
    case SSM_QL_ST2:
         return "ST2";
    case SSM_QL_SSUB:
         return "SSU-B";
    case SSM_QL_EEC2:
         if (1 == desc_mode)
         {
             return "EEC2";
         }
         else
         {
             return "EEC2/ST3";
         }
    case SSM_QL_EEC1:
         if (1 == desc_mode)
         {
             return "EEC1";
         }
         else
         {
             return "EEC1/SEC";
         }
    case SSM_QL_SMC:
         return "SMC";
    case SSM_QL_ST3E:
         return "ST3E";
    case SSM_QL_PROV:
         return "PROV";
    case SSM_QL_DNU:
         if (SSM_OPTION_MODE_1 == p_ssm_master->option_mode)
         {
             return "DNU";
         }
         else
         {
             return "DUS";
         }
    case SSM_QL_INV3:  /*Invalid Flag to show N/A*/
         return "N/A";
    default:
        return "DNU";
    }
}

char *
ssm_desc_select_interface(char *ifname_ext, uint32 is_next)
{
    tbl_ssm_global_t* p_ssm_master = tbl_ssm_global_get_ssm_global();
    char ifname[IFNAME_SIZE];

    if (is_next)
    {
        if (p_ssm_master->next_selected_ifindex)
        {
            tbl_interface_get_name_by_ifindex(p_ssm_master->next_selected_ifindex, ifname, IFNAME_SIZE);
            IFNAME_ETH2FULL(ifname, ifname_ext); 
            return ifname_ext;
        }
        else
        {
            return "N/A";
        }
    }
    else
    {
        if (p_ssm_master->select_ifindex)
        {
            tbl_interface_get_name_by_ifindex(p_ssm_master->select_ifindex, ifname, IFNAME_SIZE);
            IFNAME_ETH2FULL(ifname, ifname_ext); 
            return ifname_ext;
        }
        else
        {
            return "N/A";
        }
    }
}

char *
ssm_desc_device_status(tbl_ssm_global_t* p_ssm_glb)
{
    if (SSM_DEVICE_MAST == p_ssm_glb->device_role)
    {
        return "Mast-Clock";
    }
    if (p_ssm_glb->freerun_en)
    {
        return "FreeRun";
    }
    if (p_ssm_glb->holdover_en)
    {
        return "Holdover";
    }
    else
    {
        return "Lock";
    }
}

char *
ssm_desc_role(uint32 value)
{
    switch (value)
    {
     case SSM_DEVICE_MAST:
         return "Master-only";
     case SSM_DEVICE_SLAV:
         return "Slave-only";
     case SSM_DEVICE_STAND:
         return "Default";
     default:
         return "Default";
    }
}

char *
ssm_desc_dpll_selected_if_status(tbl_ssm_global_t* p_ssm_glb)
{    
    if (SSM_CLOCK_LOCK == p_ssm_glb->dpll_current_status)
    {
        return "Valid";
    }

    return "Invalid";
}

char *
ssm_desc_dpll_status(uint32 value)
{
    switch (value)
    {
    case SSM_CLOCK_LOCK:
        return "Lock";
    case SSM_CLOCK_HOLDOVER:
        return "Holdover";
    case SSM_CLOCK_FREERUN:
        return "FreeRun";
    default:
        return "Temporary status";
    }
}

char *
ssm_desc_if_pdu(uint32 value)
{
    switch (value)
    {
    case SSM_PDU_ALL:
         return "ESMC TX & RX";
    case SSM_PDU_RX:
         return "ESMC RX";
    case SSM_PDU_TX:
         return "ESMC TX";
    case SSM_PDU_NONE:
         return "disable";
    default:
         return "ESMC TX & RX";
    }
}
 