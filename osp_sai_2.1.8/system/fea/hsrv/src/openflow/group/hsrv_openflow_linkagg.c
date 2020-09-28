/****************************************************************************************
 * hsrv_openflow_linkagg.c   :  Hal server openflow linkagg related function
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Yan Xing'an
 * Date         :       2014-12-15
 * Reason       :       First Create.
 ****************************************************************************************/

#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_msg_openflow.h"
#include "hsrv_openflow_linkagg.h"


/*************************
 * Global and Declaration
 *************************/
hsrv_openflow_linkagg_master_t* g_pst_openflow_linkagg_master;


#define _______INTERNAL_API_________
/*************************************************************************************************
 * Name         : _hsrv_openflow_linkagg_create_send_msg
 * Purpose      : Fea layer group link agg create msg send to hagt
 * Input        : tid: uint8
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_linkagg_create_send_msg(uint32 tid)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowCreateLinkAggReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowCreateLinkAgg;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowCreateLinkAgg;
    p_req->tid = tid;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_linkagg_delete_send_msg
 * Purpose      : Fea layer group link agg delete msg send to hagt
 * Input        : tid: uint8
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_linkagg_delete_send_msg(uint32 tid)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowDelLinkAggReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelLinkAgg;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelLinkAgg;
    p_req->tid = tid;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_linkagg_add_member_send_msg
 * Purpose      : Fea layer group link agg add port
 * Input        : tid: uint8, gport: uint16
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_linkagg_add_member_send_msg(uint32 tid, uint16 gport)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowAddLinkAggMemberReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLinkAggMember;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddLinkAggMember;
    p_req->tid = tid;
    p_req->gport = gport;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_linkagg_delete_member_send_msg
 * Purpose      : Fea layer group link agg delete port
 * Input        : tid: uint8, gport: uint16
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_linkagg_delete_member_send_msg(uint32 tid, uint16 gport)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowDelLinkAggMemberReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelLinkAggMember;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelLinkAggMember;
    p_req->tid = tid;
    p_req->gport = gport;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

#define _______LINKAGG_APIS__________
/*************************************************************************************************
 * Name         : hsrv_openflow_linkagg_offset_alloc
 * Purpose      : Fea layer group link agg alloc offset
 * Input        : p_tid: uint32*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
hsrv_openflow_linkagg_offset_alloc(uint32* p_tid)
{
    uint32 tid = 0;

    HSRV_OPENFLOW_PTR_CHECK(p_tid);

    tid = ctclib_opf_alloc_offset(g_pst_openflow_linkagg_master->pst_linkagg_tid, 0, 1);
    if (CTCLIB_OPF_INVALID_OFFSET == tid)
    {
        return HSRV_E_OPENFLOW_RESOURCE_FULL;
    }

    *p_tid = tid;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_linkagg_offset_dealloc
 * Purpose      : Fea layer group link agg dealloc offset
 * Input        : p_tid: uint32*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
hsrv_openflow_linkagg_offset_dealloc(uint32 tid)
{
    ctclib_opf_free_offset(g_pst_openflow_linkagg_master->pst_linkagg_tid, 0,1, tid);

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_linkagg_create
 * Purpose      : Fea layer group link agg create
 * Input        : p_tid: uint32*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
hsrv_openflow_linkagg_create(uint32 tid)
{
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_linkagg_create_send_msg(tid));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_linkagg_delete
 * Purpose      : Fea layer group link agg delete
 * Input        : p_tid: uint32*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
hsrv_openflow_linkagg_delete(uint32 tid)
{
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_linkagg_delete_send_msg(tid));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_linkagg_add_port
 * Purpose      : Fea layer group link agg add port
 * Input        : p_tid: uint32*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
hsrv_openflow_linkagg_add_port(uint32 tid, uint16 gport)
{
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_linkagg_add_member_send_msg(tid, gport));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_linkagg_del_port
 * Purpose      : Fea layer group link agg delete port
 * Input        : p_tid: uint32*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/

int32
hsrv_openflow_linkagg_del_port(uint32 tid, uint16 gport)
{
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_linkagg_delete_member_send_msg(tid, gport));

    return HSRV_E_NONE;
}

#define _______INIT_APIS__________
/*************************************************************************************************
 * Name         : hsrv_openflow_linkagg_start
 * Purpose      : Fea layer group link agg start
 * Input        : N/A
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
hsrv_openflow_linkagg_start(void)
{
    if (NULL == g_pst_openflow_linkagg_master)
    {
        g_pst_openflow_linkagg_master = HSRV_OPENFLOW_MALLOC(sizeof(hsrv_openflow_linkagg_master_t));
        HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_linkagg_master);
        sal_memset(g_pst_openflow_linkagg_master, 0, sizeof(hsrv_openflow_linkagg_master_t));
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_linkagg_init
 * Purpose      : Fea layer group link agg init
 * Input        : p_system_info: hsrv_system_spec_info_t*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
hsrv_openflow_linkagg_init(hsrv_system_spec_info_t* p_system_info)
{
    g_pst_openflow_linkagg_master->pst_linkagg_tid = ctclib_opf_init(1);

    ctclib_opf_init_offset(g_pst_openflow_linkagg_master->pst_linkagg_tid,
                           0,
                           p_system_info->res_info.openflow_linkagg_interface_max_tid,
                           32 - p_system_info->res_info.openflow_linkagg_interface_max_tid + 1);

    return HSRV_E_NONE;
}

