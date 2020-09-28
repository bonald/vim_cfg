/****************************************************************************
 * $Id$
 *  HAGT Interrupt
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : Kun Cao
 * Date          : 2010-07-24 16:00
 * Reason        : First Create.
 ****************************************************************************/

/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/

#include "hagt_inc.h"
#include "hagt_intr.h"
#include "hagt_lcmtlk.h"
#include "lcapi.h"
#include "glb_hw_define.h"

/*huangxt for ecc 2015-04-16*/
#if 0
#define HAGT_LOG_DEBUG_ECC(fmt, args...)\
    HAGT_LOG_DEBUG(ecc, ecc, DEBUG_ECC, fmt, ##args); 
#else
#define HAGT_LOG_DEBUG_ECC(fmt, args...)\
    HAGT_LOG_ERR_CONSOLE(fmt, ##args); 
#endif

/*Support log abnormal int, bug25780. 2013-11-12*/
static int32
_hagt_abnormal_int_process_isr(uint8 gchip, void* p_data)
{
    ctc_interrupt_abnormal_status_t* p_status = NULL;
    lcapi_lcm_asic_abnormal_t asic_abnormal;
#define MAX_PRINT_STR_LEN 512  
    char log_buf[MAX_PRINT_STR_LEN];
    
    if(p_data == NULL)
    {
        return HAGT_E_INVALID_PTR;
    }
    p_status = (ctc_interrupt_abnormal_status_t*)p_data;
    if(p_status->type.intr == CTC_INTR_GB_CHIP_NORMAL)
    {
        sal_snprintf(log_buf, MAX_PRINT_STR_LEN, "Asic log interrupt %d.%d=[0x%x][0x%x][0x%x]\n", p_status->type.intr, 
                     p_status->type.sub_intr, p_status->status.bmp[0],p_status->status.bmp[1],p_status->status.bmp[2]);
        asic_abnormal.fatal_type = GLB_ASIC_FATAL_INTR_LOG;
        asic_abnormal.buf = (uint8*)log_buf;
        asic_abnormal.size = MAX_PRINT_STR_LEN;
        HAGT_LOG_ERR_LOGSYS("%s", log_buf);
        //HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_ASIC_FATAL, &asic_abnormal);
    }
    else if(p_status->type.intr == CTC_INTR_GB_CHIP_FATAL)
    {
        sal_snprintf(log_buf, MAX_PRINT_STR_LEN, "Asic log interrupt %d.%d=[0x%x][0x%x][0x%x]\n", p_status->type.intr, 
                 p_status->type.sub_intr, p_status->status.bmp[0],p_status->status.bmp[1],p_status->status.bmp[2]);
            asic_abnormal.fatal_type = GLB_ASIC_FATAL_INTR_LOG;
            asic_abnormal.buf = (uint8*)log_buf;
            asic_abnormal.size = MAX_PRINT_STR_LEN;
            HAGT_LOG_ERR_LOGSYS("%s", log_buf);
            //HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_ASIC_FATAL, &asic_abnormal);
    }
    (void)asic_abnormal;
    
    return HAGT_E_NONE;
}

/*huangxt for ecc 2015-04-30*/
static int
_hagt_intr_ecc_send_cache(ctc_interrupt_ecc_t* pst_info)
{
#if 0
    Hagt2SrvMsg_t st_hagt_msg = { 0 };
    
    HAGT_CHECK_PTR(pst_info);
     
    st_hagt_msg.agentId = 0;
    st_hagt_msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgEccCacheEntry;
    st_hagt_msg.opHagt2Srv.hagt2SrvMsgEccCacheEntry.type = pst_info->type;
    st_hagt_msg.opHagt2Srv.hagt2SrvMsgEccCacheEntry.tblid = pst_info->tbl_id;
    st_hagt_msg.opHagt2Srv.hagt2SrvMsgEccCacheEntry.tblindex = pst_info->tbl_index;
    st_hagt_msg.opHagt2Srv.hagt2SrvMsgEccCacheEntry.action = pst_info->action;
    st_hagt_msg.opHagt2Srv.hagt2SrvMsgEccCacheEntry.recover = pst_info->recover;

    HAGT2SRV_MSG_SEND(&st_hagt_msg);
#endif
    return HAGT_E_NONE;
}


/*huangxt for ecc 2015-04-16*/
static int32
_hagt_ecc_error_process_isr(uint8 gchip, void* p_data)
{
    char* ecc_error_type[] = {"single bit error", "multiple bit error", "parity error", "tcam error"};
    char* ecc_error_action[] = {"NoAction", "Log", "Reset"};

    ctc_interrupt_ecc_t* p_ecc = NULL;
    char action_str[20];
    char* p_time_str = NULL;
    sal_time_t tm;

    /*huangxt for ecc record to log_sys*/
    char log_buffer[1024] = "";
    char log_temp[128] = "";

    sal_memset(action_str, 0, sizeof(action_str));
    HAGT_CHECK_PTR(p_data);
    p_ecc = (ctc_interrupt_ecc_t*)p_data;

    sal_time(&tm);
    p_time_str = sal_ctime(&tm);

    HAGT_LOG_DEBUG_ECC("\n");
    HAGT_LOG_DEBUG_ECC(" EccError Time:   %s", p_time_str);
    HAGT_LOG_DEBUG_ECC(" ---------------------------------------- \n");

    sal_sprintf(log_temp, "EccError detected! "); /*log_sys has its own time, so do not need to print again*/
    sal_strcat(log_buffer, log_temp);

    HAGT_LOG_DEBUG_ECC(" EccErrorType :   %s\n",
                   (p_ecc->type >= CTC_INTERRUPT_ECC_TYPE_NUM)
                   ? "Unknown" : ecc_error_type[p_ecc->type]);

    sal_sprintf(log_temp, "Type:%s, ", (p_ecc->type >= CTC_INTERRUPT_ECC_TYPE_NUM)
                   ? "Unknown" : ecc_error_type[p_ecc->type]);
    sal_strcat(log_buffer, log_temp);

    if (0xFD000000 == (p_ecc->tbl_id & 0xFD000000))
    {
        HAGT_LOG_DEBUG_ECC(" TcamAdRamId  :   %u\n",(p_ecc->tbl_id & 0xFFFFFF));
        HAGT_LOG_DEBUG_ECC(" Offset       :   %u\n", p_ecc->tbl_index);

        sal_sprintf(log_temp, "TcamAdRamId:%u, Offset:%u, ", (p_ecc->tbl_id & 0xFFFFFF), p_ecc->tbl_index); 
        sal_strcat(log_buffer, log_temp);
    }
    else if (0xFF000000 == (p_ecc->tbl_id & 0xFF000000))
    {
        HAGT_LOG_DEBUG_ECC(" SramID       :   %u\n", (p_ecc->tbl_id & 0xFFFFFF));
        HAGT_LOG_DEBUG_ECC(" Offset(3W)   :   %u\n", p_ecc->tbl_index);

        sal_sprintf(log_temp, "SramID:%u, Offset(3W):%u, ", (p_ecc->tbl_id & 0xFFFFFF), p_ecc->tbl_index); 
        sal_strcat(log_buffer, log_temp);
    }
    else
    {
        HAGT_LOG_DEBUG_ECC(" TableID      :   %u\n", p_ecc->tbl_id);
        HAGT_LOG_DEBUG_ECC(" TableIndex   :   %u\n", p_ecc->tbl_index);

        sal_sprintf(log_temp, "TableID:%u, TableIndex:%u, ", p_ecc->tbl_id, p_ecc->tbl_index); 
        sal_strcat(log_buffer, log_temp);
    }

    HAGT_LOG_DEBUG_ECC(" EccAction    :   %s\n",
                   (p_ecc->action > CTC_INTERRUPT_FATAL_INTR_RESET)
                   ? "Unknown" : ecc_error_action[p_ecc->action]);

    sal_sprintf(log_temp, "EccAction:%s, ", (p_ecc->action > CTC_INTERRUPT_FATAL_INTR_RESET)
                   ? "Unknown" : ecc_error_action[p_ecc->action]);
    sal_strcat(log_buffer, log_temp);

    HAGT_LOG_DEBUG_ECC(" ErrorRecover :   %s\n", p_ecc->recover ? "Yes" : "No");
    HAGT_LOG_DEBUG_ECC("\n");

    sal_sprintf(log_temp, "ErrorRecover:%s, ", p_ecc->recover ? "Yes" : "No");
    sal_strcat(log_buffer, log_temp);

    /*huangxt for ecc record to log_sys*/    
    HAGT_LOG_ERR_LOGSYS("%s", log_buffer);

    /*send to hsrv for show*/
    _hagt_intr_ecc_send_cache(p_ecc);

    return CTC_E_NONE;
}

int32
hagt_abnormal_int_init()
{
    ctc_interrupt_register_event_cb(CTC_EVENT_ABNORMAL_INTR, _hagt_abnormal_int_process_isr);

    /*huangxt for ecc 2015-04-16*/
    ctc_interrupt_register_event_cb(CTC_EVENT_ECC_ERROR, _hagt_ecc_error_process_isr);
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : hagt_intr_init
 * Purpose : This function used to init hagt interrupt module
 * Input   : hagt_master_t* p_master    -  hal server master point
 * Output  : N/A
 * Return  : HAGT_E_XXX - HAGT error code
 * Note    : 
*********************************************************************/
int32
hagt_intr_init(hagt_master_t* p_master)
{
    return HAGT_E_NOT_INIT;
}
