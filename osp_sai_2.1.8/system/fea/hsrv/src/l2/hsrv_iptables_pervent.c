/****************************************************************************
* $Id$
*  Centec fdb information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2016-03-10
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
* Defines and Macros
******************************************************************************/



/***************************************************************************************************
 * Name         : hsrv_iptables_prevent_sync 
 * Purpose      : load switch global table cdb to sai          
 * Input        : p_tbl_node: table node pointer
                  p_ds_node: ds node pointer
                  oper: operation code
                  field_id: field id with operation
                  p_tbl: cdb table pointer
                  p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_iptables_prevent_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_iptables_prevent_t *p_ipt_prevent = NULL;
    bool enable;

    HSRV_SWITCH_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    //CTCLIB_PRINT_CON("hsrv_iptables_prevent_sync, OPER = %d, field = %d", oper, field_id);

    p_ipt_prevent = (tbl_iptables_prevent_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:

        switch (field_id)
        {
            case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_IPEQ:
                enable = GLB_FLAG_ISSET(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_IPEQ);
                HSRV_IF_ERROR_RETURN(ctc_global_ctl_set(CTC_GLOBAL_DISCARD_SAME_IPDASA_PKT, &enable));
                break;
            case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_MACEQ:
                enable = GLB_FLAG_ISSET(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_MACEQ);
                HSRV_IF_ERROR_RETURN(ctc_global_ctl_set(CTC_GLOBAL_DISCARD_SAME_MACDASA_PKT, &enable));
                break;

            default:
                break;
        }

        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}
