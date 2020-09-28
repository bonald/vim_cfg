#include "sal.h"
#include "sal_types.h"
#include "hsrv_inc.h"

#include "hsrv_openflow.h"
#include "hsrv_openflow_parser_priv.h"
#include "hsrv_openflow_parser.h"

#define _____HAL_APIs_____
int32
fei_api_openflow_set_ecmp_hash_key(fei_openflow_parser_set_ecmp_hash_key_req_t* pst_req)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowSetEcmpHashKeyReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowSetEcmpHashKey;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetEcmpHashKey;
    p_req->hashKey.hashKey = pst_req->hash_key;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    return HSRV_E_NONE;
}

int32
fei_api_openflow_set_linkagg_hash_key(fei_openflow_parser_set_linkagg_hash_key_req_t* pst_req)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowSetLinkAggHashKeyReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowSetLinkAggHashKey;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetLinkAggHashKey;
    p_req->linkAggHashKey.hashUse = pst_req->hash_use;
    p_req->hashKey.hashKey = pst_req->hash_key;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    return HSRV_E_NONE;
}

int32
fei_api_openflow_set_mpls_parser(fei_openflow_parser_set_mpls_parser_req_t* pst_req)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowSetMplsParserReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowSetMplsParser;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetMplsParser;
    p_req->enable = pst_req->enable;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    return HSRV_E_NONE;
}

#define _____END_LINE_____
