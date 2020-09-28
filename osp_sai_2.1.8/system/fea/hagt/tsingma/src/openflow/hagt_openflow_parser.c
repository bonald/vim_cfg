#include "hagt_inc.h"

#include "glb_openflow_define.h"
#include "glb_ip_define.h"
#include "hagt_openflow.h"
#include "hagt_openflow_parser.h"

#define _____INTERNAL_API_____
/**
 * Set ecmp hash keys
 * @param hash_keys                     Pointer to bitmap of adpt_parser_ecmp_hash_key_t
 * @return OFP_ERR_XXX
 */
static int32
_hagt_parser_set_ecmp_hash_key(uint32 hash_keys)
{
    ctc_parser_ecmp_hash_ctl_t hash_ctl;

    memset(&hash_ctl, 0, sizeof(hash_ctl));
    GLB_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L2);
    GLB_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_IP);
    GLB_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4);

    /* 0. set l2 hash key */
    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_MACSA))
    {
        GLB_SET_FLAG(hash_ctl.l2_flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACSA);
    }

    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_MACDA))
    {
        GLB_SET_FLAG(hash_ctl.l2_flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACDA);
    }
    
    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_ETHERTYPE))
    {
        GLB_SET_FLAG(hash_ctl.l2_flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_ETHERTYPE);
    }

    /* 1. set ip hash key */
    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_IPSA))
    {
        GLB_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPSA);
    }

    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_IPDA))
    {
        GLB_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPDA);
    }

    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_IPPRO))
    {
        GLB_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_PROTOCOL);
    }

    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_DSCP))
    {
        GLB_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_DSCP);
    }

    /* 2. set layer4 hash key */
    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_SRC_PORT))
    {
        GLB_SET_FLAG(hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_SRC_PORT);
        /* Added by weizj for tunnel bug 33501 */
        GLB_SET_FLAG(hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_VXLAN_L4_SRC_PORT);
    }

    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_DST_PORT))
    {
        GLB_SET_FLAG(hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_DST_PORT);
    }

    /* 4. set hash ctl to sdk */
    HAGT_IF_ERROR_RETURN(ctc_parser_set_ecmp_hash_field(&hash_ctl));

    /*set inner hash key*/
    memset(&hash_ctl, 0, sizeof(hash_ctl));
    GLB_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L2);
    GLB_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_IP);
    GLB_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4);
    GLB_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_INNER);
    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_INNER_MACSA))
    {
        GLB_SET_FLAG(hash_ctl.l2_flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACSA);
    }

    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_INNER_MACDA))
    {
        GLB_SET_FLAG(hash_ctl.l2_flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACDA);
    }

    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_INNER_IPSA))
    {
        GLB_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPSA);
    }

    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_INNER_IPDA))
    {
        GLB_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPDA);
    }

    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_INNER_IP_PROTOCOL))
    {
        GLB_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_PROTOCOL);
    }

    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_INNER_SRC_PORT))
    {
        GLB_SET_FLAG(hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_SRC_PORT);
    }

    if (GLB_FLAG_ISSET(hash_keys, GLB_ECMP_HASH_KEY_INNER_DST_PORT))
    {
        GLB_SET_FLAG(hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_DST_PORT);
    }
    
    HAGT_IF_ERROR_RETURN(ctc_parser_set_ecmp_hash_field(&hash_ctl));

    return HAGT_E_NONE;
}

/**
 * Set linkagg hash key
 * @param[in] hash_use                  Bitmap of adpt_parser_linkagg_hash_use_t
 * @param[in] hash_key                  Bitmap of adpt_parser_linkagg_hash_key_t
 * @return OFP_ERR_XXX
 */
static int32
_hagt_parser_set_linkagg_hash_key(uint32 hash_use, uint32 hash_keys)
{
    ctc_linkagg_psc_t hash_ctl;

    memset(&hash_ctl, 0, sizeof(hash_ctl));

    /* 1. set hash use */
    GLB_SET_FLAG(hash_ctl.psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_L2);
    GLB_SET_FLAG(hash_ctl.psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_IP);
    GLB_SET_FLAG(hash_ctl.psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_L4);

    /* 2. set mac hash key */
    if (GLB_FLAG_ISSET(hash_use, GLB_PARSER_LAG_HASH_USE_MAC))
    {
        if (GLB_FLAG_ISSET(hash_keys, GLB_PARSER_LAG_HASH_KEY_PORT))
        {
            GLB_SET_FLAG(hash_ctl.l2_flag, CTC_LINKAGG_PSC_L2_PORT);
        }

        if (GLB_FLAG_ISSET(hash_keys, GLB_PARSER_LAG_HASH_KEY_MACSA))
        {
            GLB_SET_FLAG(hash_ctl.l2_flag, CTC_LINKAGG_PSC_L2_MACSA);
        }

        if (GLB_FLAG_ISSET(hash_keys, GLB_PARSER_LAG_HASH_KEY_MACDA))
        {
            GLB_SET_FLAG(hash_ctl.l2_flag, CTC_LINKAGG_PSC_L2_MACDA);
        }

        if (GLB_FLAG_ISSET(hash_keys, GLB_PARSER_LAG_HASH_KEY_VID))
        {
            GLB_SET_FLAG(hash_ctl.l2_flag, CTC_LINKAGG_PSC_L2_VLAN);
        }

        if (GLB_FLAG_ISSET(hash_keys, GLB_PARSER_LAG_HASH_KEY_COS))
        {
            GLB_SET_FLAG(hash_ctl.l2_flag, CTC_LINKAGG_PSC_L2_COS);
        }

        if (GLB_FLAG_ISSET(hash_keys, GLB_PARSER_LAG_HASH_KEY_L2PRO))
        {
            GLB_SET_FLAG(hash_ctl.l2_flag, CTC_LINKAGG_PSC_L2_ETHERTYPE);
        }
    }

    /* 3. set ip hash key */
    if (GLB_FLAG_ISSET(hash_use, GLB_PARSER_LAG_HASH_USE_IP))
    {
        GLB_SET_FLAG(hash_ctl.ip_flag, CTC_LINKAGG_PSC_USE_IP);
        if (GLB_FLAG_ISSET(hash_keys, GLB_PARSER_LAG_HASH_KEY_IPSA))
        {
            GLB_SET_FLAG(hash_ctl.ip_flag, CTC_LINKAGG_PSC_IP_IPSA);
        }

        if (GLB_FLAG_ISSET(hash_keys, GLB_PARSER_LAG_HASH_KEY_IPDA))
        {
            GLB_SET_FLAG(hash_ctl.ip_flag, CTC_LINKAGG_PSC_IP_IPDA);
        }

        if (GLB_FLAG_ISSET(hash_keys, GLB_PARSER_LAG_HASH_KEY_IPPRO))
        {
            GLB_SET_FLAG(hash_ctl.ip_flag, CTC_LINKAGG_PSC_IP_PROTOCOL);
        }
    }

    /* 4. set layer4 hash key */
    if (GLB_FLAG_ISSET(hash_use, GLB_PARSER_LAG_HASH_USE_L4))
    {
        GLB_SET_FLAG(hash_ctl.l4_flag, CTC_LINKAGG_PSC_USE_L4);
        if (GLB_FLAG_ISSET(hash_keys, GLB_PARSER_LAG_HASH_KEY_SRC_PORT))
        {
            GLB_SET_FLAG(hash_ctl.l4_flag, CTC_LINKAGG_PSC_L4_SRC_PORT);
        }

        if (GLB_FLAG_ISSET(hash_keys, GLB_PARSER_LAG_HASH_KEY_DST_PORT))
        {
            GLB_SET_FLAG(hash_ctl.l4_flag, CTC_LINKAGG_PSC_L4_DST_PORT);
        }
    }

    /* 5. set hash ctl to sdk */
    HAGT_IF_ERROR_RETURN(ctc_linkagg_set_psc(&hash_ctl));

    return HAGT_E_NONE;
}

#define _____MESSAGE_CB_______
int32
hagt_openflow_set_ecmp_hash_key(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowSetEcmpHashKeyReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowSetEcmpHashKey;

    HAGT_IF_ERROR_RETURN(_hagt_parser_set_ecmp_hash_key(req->hashKey.hashKey));

    return HAGT_E_NONE;
}

int32
hagt_openflow_set_linkagg_hash_key(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowSetLinkAggHashKeyReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowSetLinkAggHashKey;

    HAGT_IF_ERROR_RETURN(_hagt_parser_set_linkagg_hash_key(req->linkAggHashKey.hashUse, req->hashKey.hashKey));

    return HAGT_E_NONE;
}

int32
hagt_openflow_set_mpls_parser(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowSetMplsParserReq_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowSetMplsParser;

    HAGT_IF_ERROR_RETURN(ctc_parser_enable_l3_type(CTC_PARSER_L3_TYPE_MPLS, (req->enable ? TRUE : FALSE)));

    return HAGT_E_NONE;
}

#define _____INIT_____
int32
hagt_openflow_parser_register_callback(void)
{   
#define Reg(a, b)\
    hagt_message_set_msg_callback(a, b)

    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetEcmpHashKey, hagt_openflow_set_ecmp_hash_key);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetLinkAggHashKey, hagt_openflow_set_linkagg_hash_key);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetMplsParser, hagt_openflow_set_mpls_parser);

#undef Reg    
    return HAGT_E_NONE;
}
