/****************************************************************************
* $Id$
*  Centec function information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-12-01
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "hsrv_msg_if.h"
#include "glb_ip_define.h"
#include "lcm.h"
#include "hsrv_interface.h"
#include "hsrv_if.h"
#include "glb_cpu_traffic_define.h"
#include "ctc_pdu.h"
#include "ctc_sai_port.h"
#include "sal.h"
#include "gen/tbl_lldp_global_define.h"
#include "gen/tbl_lldp_global.h"
#include "gen/tbl_lldp_if_define.h"
#include "gen/tbl_lldp_if.h"

#define HSRV_L2PDU_KEY_INDEX_LLDP 0

static int32
hsrv_lldp_global_set_enable(tbl_lldp_global_t* p_lldp_glb)
{
    int32 ret = 0;
    tbl_lldp_global_t* p_db_lldp_glb = NULL;
    ctc_pdu_l2pdu_type_t  l2pdu_type;
    uint8 key_index = 0;
    ctc_pdu_global_l2pdu_action_t l2_action;

    sal_memset(&l2pdu_type, 0, sizeof(l2pdu_type));
    sal_memset(&l2_action, 0, sizeof(l2_action));
    
    p_db_lldp_glb = tbl_lldp_global_get_lldp_global();
    p_db_lldp_glb->enable = p_lldp_glb->enable;
    if (p_db_lldp_glb->enable)
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_LLDP;
        l2_action.action_index = CTC_L2PDU_ACTION_INDEX_LLDP;
#ifndef USW
        l2_action.bypass_all = 1;
#endif
        l2_action.copy_to_cpu = 1;
        l2_action.entry_valid = 1;
        key_index = HSRV_L2PDU_KEY_INDEX_LLDP;
    }
    else
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_LLDP;
        l2_action.action_index = CTC_L2PDU_ACTION_INDEX_LLDP;
#ifndef USW
        l2_action.bypass_all = 0;
#endif
        l2_action.copy_to_cpu = 0;
        l2_action.entry_valid = 0;
        key_index = HSRV_L2PDU_KEY_INDEX_LLDP;
    }
    ret = ctc_l2pdu_set_global_action(l2pdu_type, key_index, &l2_action);
    return ret; 
}

static int32
hsrv_lldp_if_set_mode(sai_object_id_t port_oid, tbl_lldp_if_t* p_lldp_if)
{
    int32 ret = 0;
    uint32 gport = 0;
    ctc_pdu_port_l2pdu_action_t l2_action;
    sal_memset(&l2_action, 0, sizeof(l2_action));
    
    HSRV_PTR_CHECK(p_lldp_if);

    ctc_sai_port_objectid_to_gport(port_oid, &gport);
    if (p_lldp_if->mode)
    {
        l2_action = CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU;
    }
    else
    {
        l2_action = CTC_PDU_L2PDU_ACTION_TYPE_DISCARD;
    }
    ret =  ctc_l2pdu_set_port_action(gport, CTC_L2PDU_ACTION_INDEX_LLDP, l2_action);
    return ret;
}

int32
hsrv_lldp_global_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_lldp_global_t* p_lldp_glb = NULL;
    
    HSRV_CPU_TRAFFIC_RX_DEBUG("Enter into %s.", __FUNCTION__);  
    HSRV_PTR_CHECK(p_tbl);
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_lldp_glb = (tbl_lldp_global_t*)p_tbl;
        switch (field_id)
        {
        case TBL_LLDP_GLOBAL_FLD_ENABLE:
            hsrv_lldp_global_set_enable(p_lldp_glb);
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

int32
hsrv_lldp_if_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_lldp_if_t* p_lldp_if = NULL;
    uint32 ifindex = 0;
    sai_object_id_t port_oid = 0;
    
    HSRV_CPU_TRAFFIC_RX_DEBUG("Enter into %s.", __FUNCTION__);  
    switch (oper)
    {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        p_lldp_if = (tbl_lldp_if_t*)p_tbl;
        HSRV_PTR_CHECK(p_lldp_if);
        ifindex = p_lldp_if->key.ifindex;
        hsrv_interface_ifindex_to_objectid(ifindex, &port_oid);
        hsrv_lldp_if_set_mode(port_oid, p_lldp_if); 
        break;

    case CDB_OPER_SET:
        p_lldp_if = (tbl_lldp_if_t*)p_tbl;
        HSRV_PTR_CHECK(p_lldp_if);
        switch (field_id)
        {
        case TBL_LLDP_IF_FLD_MODE:
            ifindex = p_lldp_if->key.ifindex;
            hsrv_interface_ifindex_to_objectid(ifindex, &port_oid);
            hsrv_lldp_if_set_mode(port_oid, p_lldp_if);
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
