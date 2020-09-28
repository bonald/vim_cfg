#include "sal.h"
#include "glb_macro.h"
#include "glb_const.h"
#include "proto.h"
#include "glb_openflow_define.h"
#include "lib.h"

#include "fei.h"

#include "hsrv_msg.h"
#include "hsrv_msg_openflow.h"

#include "fei_openflow_flow.h"

int32
fei_flow_add_flow(glb_openflow_flow_t* p_flow)
{
    fei_openflow_add_flow_req_t* p_req;
    uint32 req_len;
    uint32 action_len;
    int32  rc;

    req_len = sizeof(fei_openflow_add_flow_req_t);
    action_len = sizeof(glb_openflow_flow_action_t) * p_flow->actlist.action_count;
    p_req = malloc(req_len + action_len);
    sal_memcpy(&p_req->flow, p_flow, req_len);

    p_req->flow.actlist.actlist = NULL;

    if (action_len)
    {
        /* copy action list to end of request message */
        sal_memcpy(p_req + 1, p_flow->actlist.actlist, action_len);
    }
    rc = fei_talk(FEI_OPENFLOW_FLOW_ADD, p_req, req_len + action_len);
    free(p_req);
    return rc;
}

int32
fei_flow_packet_out(glb_openflow_packet_out_param_t* param, void* pkt, uint32 pkt_len)
{
    fei_openflow_packet_out_req_t* p_req;
    uint32 req_len;
    int32  rc = 0;
    if(pkt_len > 9600)
    {
        return -1;
    }
    req_len = sizeof(glb_openflow_packet_out_param_t);
    p_req = malloc(req_len + pkt_len);

    sal_memcpy(&p_req->packet_out, param, req_len);
    p_req->packet_out.packet_len = pkt_len;
    
    sal_memcpy(p_req + 1, pkt, pkt_len);
    rc = fei_talk(FEI_OPENFLOW_PACKET_OUT, p_req, req_len + pkt_len);
    free(p_req);
    return rc;
}

int32
fei_flow_mod_flow(glb_openflow_flow_t* p_flow)
{
    fei_openflow_mod_flow_req_t* p_req;
    uint32 req_len;
    uint32 action_len;
    int32  rc;

    req_len = sizeof(fei_openflow_mod_flow_req_t);
    action_len = sizeof(glb_openflow_flow_action_t) * p_flow->actlist.action_count;
    p_req = malloc(req_len + action_len);
    sal_memcpy(&p_req->flow, p_flow, req_len);
    
    p_req->flow.actlist.actlist = NULL;
    
    if (action_len)
    {
        /* copy action list to end of request message */
        sal_memcpy(p_req + 1, p_flow->actlist.actlist, action_len);
    }    
    rc = fei_talk(FEI_OPENFLOW_FLOW_MOD, p_req, req_len + action_len);
    free(p_req);
    return rc;
}

int32
fei_flow_del_flow(uint32 flow_id)
{
    fei_openflow_del_flow_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_del_flow_req_t));
    req.flow_id = flow_id;
   
    return fei_talk(FEI_OPENFLOW_FLOW_DEL, &req, sizeof(req));
}

int32
fei_flow_reset_flow(void)
{
    fei_openflow_reset_flow_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_reset_flow_req_t));
   
    return fei_talk(FEI_OPENFLOW_FLOW_RESET, &req, sizeof(req));
}

int32
fei_flow_clear_stats(uint32 flow_id)
{
    fei_openflow_clear_stats_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_clear_stats_req_t));

    req.res_type = FEI_OPENFLOW_STATS_RES_TYPE_FLOW;
    req.id = flow_id;

    return fei_talk(FEI_OPENFLOW_STATS_CLEAR, &req, sizeof(req));
}

/* Added by weizj for tunnel bug 33724 */
/* Modifed by weizj for bug 35135 */
int32
fei_flow_set_drop_ingress(uint8 is_group, uint8 is_drop)
{
    fei_openflow_set_drop_ingress req;

    sal_memset(&req, 0, sizeof(fei_openflow_set_drop_ingress));

    req.is_group = is_group;
    req.is_drop = is_drop;

    return fei_talk(FEI_OPENFLOW_FLOW_DROP_INGRESS, &req, sizeof(req));
}

int32
fei_openflow_update_all_action_list(uint32 ifindex, bool enable)
{
    fei_openflow_update_all_action_list_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_update_all_action_list_req_t));
    req.ifindex = ifindex;
    req.enable = enable;
    
    return fei_talk(FEI_OPENFLOW_FLOW_UPDATE_ALL_ACTION_LIST, &req, sizeof(req));
}

/* Added by weizj for udf */
int32
fei_flow_set_udf_enable(uint8 enable)
{
    fei_openflow_set_udf_parser req;

    sal_memset(&req, 0, sizeof(fei_openflow_set_udf_parser));

    req.enable = enable;

    return fei_talk(FEI_OPENFLOW_FLOW_UDF_ENABLE, &req, sizeof(req));
}

int32
fei_flow_set_e2e_enable(uint8 enable)
{
    fei_openflow_set_e2e_parser req;

    sal_memset(&req, 0, sizeof(fei_openflow_set_e2e_parser));

    req.enable = enable;

    return fei_talk(FEI_OPENFLOW_FLOW_E2E_ENABLE, &req, sizeof(req));
}
int32
fei_flow_set_udf_parser(glb_openflow_udf_parser_t *p_info)
{
    fei_openflow_set_udf_parser req;

    sal_memset(&req, 0, sizeof(fei_openflow_set_udf_parser));

    sal_memcpy(&req.parser, p_info, sizeof(fei_openflow_set_udf_parser));

    return fei_talk(FEI_OPENFLOW_FLOW_SET_UDF_PARSER, &req, sizeof(req));
}
/* End by weizj for udf */

