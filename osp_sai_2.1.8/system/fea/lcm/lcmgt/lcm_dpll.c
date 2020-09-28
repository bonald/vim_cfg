/****************************************************************************
* $Id$
*  line card dpll information
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : lylandris
* Date          : 2011-12-15
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
 * Header Files 
 *
 ****************************************************************************/
#ifdef CNOS_OS
#include "sal.h"
#include "laiinc.h"
#else
#include "sal_common.h"
#endif
#include "glb_hw_define.h"
#include "glb_synce_define.h"
#include "glb_macro.h"
#include "lcm_debug.h"
#include "lcm_error.h"
#include "clock_api.h"
#include "lcapi.h"
#include "lcm_hagttlk.h"

/****************************************************************************
 *  
 * Defines and Macros
 *
 *****************************************************************************/
char *g_lcm_dpll_state_string[] = 
{
    "000 = Unknown",
    "001 = Free-run",
    "010 = Holdover",
    "011 = Unknown",
    "100 = Locked",
    "101 = Prelocked 2",
    "110 = Prelocked",
    "111 = Loss-of-lock",
};
/****************************************************************************
 *  
 * Global and Declaration
 *
 *****************************************************************************/

/****************************************************************************
 *  
 * Function
 *
 *****************************************************************************/

/***************************************************************************************************
 * Name         : lcm_dpll_status_intr 
 * Purpose      : handle interrupt caused by dpll stauts change          
 * Input        : N/A         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
 ***************************************************************************************************/
int32
lcm_dpll_status_intr(void)
{
    lcapi_lcm_get_clock_status_t clock_status;
    uint32 dpll_state = 0;

#ifndef _GLB_UML_SYSTEM_
    clock_get_status(0, &dpll_state);
    clock_intr_fixup(0);
#endif
    clock_status.dpll_state = dpll_state;
    if (clock_status.dpll_state!= GLB_SYNCE_STATE_PRELOCKED)
    {
        log_sys(M_MOD_LCM, E_NOTICE, "DPLL switch to %s state.\n", g_lcm_dpll_state_string[clock_status.dpll_state]);
    }
    else
    {
        log_sys(M_MOD_LCM, E_NOTICE, "DPLL switch to %x state.\n", dpll_state);
    }

    /* modified by yaom for bug 18501 20110306 */  
    if (lcm2hagt_cb[LCM2HAGT_CB_UPDATE_DPLL_STATE])
    {
#ifdef CNOS_OS        
        lai_synce_info_t lai_synce;
        sal_memset(&lai_synce, 0, sizeof(lai_synce));
        lai_synce.dpll_state = dpll_state;
        if (p_lai_card_noti && p_lai_card_noti->on_synce_dpll_state_event)
        {
            p_lai_card_noti->on_synce_dpll_state_event(LAI_SYNCE_UPDATE_DPLL_STATE, &lai_synce);    
        }
#else        
        LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_UPDATE_DPLL_STATE, &clock_status);
#endif
    }
    return LCM_E_SUCCESS;
}

