
/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "hsrv_wb.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
* Defines and Macros
******************************************************************************/

int32
hsrv_wb_cmd_process_act_wb(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = HSRV_E_NONE;
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (argc >= 1)
        {
            if (0 == ctc_sai_get_wb_restoring())
            {
                CFG_INVALID_PARAM_RET("warmboot is not restoring");
            }
            hsrv_wb_sdb_restore_done();
        }
        else
        {
            //rc = hsrv_wb_do_reboot();
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
hsrv_wb_cmd_init()
{
    cdb_register_cfg_act_cb(ACT_WARMBOOT,   hsrv_wb_cmd_process_act_wb);

    return HSRV_E_NONE;
}

