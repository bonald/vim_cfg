#include "hagt_inc.h"

#include "hagt_openflow.h"
#include "hagt_openflow_linkagg.h"


#define _____GROUP_LINKAGG_APIS_____
/*************************************************************************************************
 * Name         : hagt_openflow_create_linkagg
 * Purpose      : Hagt layer group create linkagg
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
hagt_openflow_create_linkagg(Hsrv2AgtMsg_t* p_msg_data)
{
    ctc_linkagg_group_t linkagg_grp;
    HalMsgOpenFlowCreateLinkAggReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowCreateLinkAgg;
    sal_memset(&linkagg_grp, 0, sizeof(linkagg_grp));

    linkagg_grp.tid      = (uint8)req->tid;
    linkagg_grp.linkagg_mode = CTC_LINKAGG_MODE_STATIC;
    HAGT_IF_ERROR_RETURN(ctc_linkagg_create(&linkagg_grp));

    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : hagt_openflow_del_linkagg
 * Purpose      : Hagt layer group delete linkagg
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
hagt_openflow_del_linkagg(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDelLinkAggReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelLinkAgg;

    HAGT_IF_ERROR_RETURN(ctc_linkagg_destroy((uint8)req->tid));

    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : hagt_openflow_add_linkagg_member
 * Purpose      : Hagt layer group add linkagg member
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
hagt_openflow_add_linkagg_member(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowAddLinkAggMemberReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddLinkAggMember;

    HAGT_IF_ERROR_RETURN(ctc_linkagg_add_port((uint8)req->tid, (uint16)req->gport));

    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : hagt_openflow_del_linkagg_member
 * Purpose      : Hagt layer group delete linkagg member
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
hagt_openflow_del_linkagg_member(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDelLinkAggMemberReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelLinkAggMember;

    HAGT_IF_ERROR_RETURN(ctc_linkagg_remove_port((uint8)req->tid, (uint16)req->gport));

    return HAGT_E_NONE;
}

#define _____GROUP_LINKAGG_INIT_____
/*************************************************************************************************
 * Name         : hagt_openflow_linkagg_init
 * Purpose      : Hagt layer group linkagg init
 * Input        : N/A
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
hagt_openflow_linkagg_init(void)
{
    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : hagt_openflow_linkagg_register_callback
 * Purpose      : Hagt layer group linkagg callback
 * Input        : N/A
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
hagt_openflow_linkagg_register_callback(void)
{
#define Reg(a, b)\
    hagt_message_set_msg_callback(a, b)

    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowCreateLinkAgg, hagt_openflow_create_linkagg);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelLinkAgg, hagt_openflow_del_linkagg);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddLinkAggMember, hagt_openflow_add_linkagg_member);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelLinkAggMember, hagt_openflow_del_linkagg_member);

#undef Reg
    return HAGT_E_NONE;
}

