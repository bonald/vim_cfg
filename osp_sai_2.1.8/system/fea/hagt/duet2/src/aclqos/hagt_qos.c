/****************************************************************************************
 * hagt_qos.c   :  Hal agent qos related function
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       rongl
 * Date         :       2010-09-25
 * Reason       :       First Create.
 * Revision     :       R0.02
 * Author       :       yanxa
 * Date         :       2011-07-18
 * Reason       :       Modified for E330 phase3
****************************************************************************************/

/****************************************************************************************
 *
 * Header Files 
 *
 ****************************************************************************************/
#include "ctc_api.h"
#include "glb_macro.h"
#include "glb_const.h"
#include "glb_qos_define.h"
#include "glb_flow_define.h"

#include "ctclib_show.h"
#include "ctclib_opf.h"
#include "hagt_inc.h"

#include "hagt.h"
#include "hagt_stats.h"
#include "hagt_flow.h"
#include "ctc_hash.h"
#include "hagt_qos_priv.h"
#include "hagt_qos.h"
#include "hagt_chip.h"
#include "hagt_stats.h"
#include "hagt_if.h"/*added by ychen for bug 25183 in 2013-10-12*/


/****************************************************************************************
 *  
 * Globals
 *
 ****************************************************************************************/

extern hagt_master_t *g_pst_hagt_master;

static hagt_qos_master_t *g_pst_hagt_qos_master = NULL;
#define HAGT_QOS_PORT_QUEUE_NUM g_pst_hagt_qos_master->port_queue_num

extern uint32 hagt_ipuc_get_ipv6_enabled();/*added by ychen for bug 25183 in 2013-10-12*/
/****************************************************************************************
 *  
 * Function
 *
 ****************************************************************************************/

 /* ~~~~~~~~~~~~~~~~~~~~~ global configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
 
/****************************************************************************************
 * Name         : _hagt_qos_set_global_enable_flags
 * Purpose      : Set QoS global enable flags
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_set_global_enable_flags(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosSetGlobalEnableFlagsReq_t *req;
    ctc_qos_glb_cfg_t glb_cfg;
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_PROCESS_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetGlobalFlag;
 
    /* 2. check global flags */
    if (GLB_FLAG_ISSET(req->flags, GLB_QOS_GLOBAL_FLAG_POLICER_EN))
    {
        /* no need to memset */
        //sal_memset (&glb_cfg, 0, sizeof (glb_cfg));
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_EN;
        glb_cfg.u.value = req->enable;
        HAGT_IF_ERROR_RETURN(ctc_qos_set_global_config(&glb_cfg));
        HAGT_QOS_PROCESS_DEBUG_INFO("policer global enable: %ld", req->enable);

        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_SEQENCE_EN;
        glb_cfg.u.value = req->enable;        
        HAGT_IF_ERROR_RETURN(ctc_qos_set_global_config(&glb_cfg));
        
        HAGT_QOS_PROCESS_DEBUG_INFO("policer seqence global enable: %ld", req->enable);
    }
    if (GLB_FLAG_ISSET(req->flags, GLB_QOS_GLOBAL_FLAG_FLOW_FIRST))
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_FLOW_FIRST_EN;
        /* high 16bit is dir, low 16bit is enable value*/
        glb_cfg.u.value = CTC_BOTH_DIRECTION << 16 | req->enable;
        HAGT_IF_ERROR_RETURN(ctc_qos_set_global_config(&glb_cfg));
    
        HAGT_QOS_PROCESS_DEBUG_INFO("flow policer first: %ld", req->enable);
    }
    /* global queue stats is not supported now */
    #if 0
    if (GLB_FLAG_ISSET(req->flags, GLB_QOS_GLOBAL_FLAG_QUEUE_STATS_EN))
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_QUE_STATS_EN;
        glb_cfg.u.value = req->enable;        
        HAGT_IF_ERROR_RETURN(ctc_qos_set_global_config(&glb_cfg));
    
        HAGT_QOS_PROCESS_DEBUG_INFO("queue stats global enable: %d", req->enable);
    }
    #endif
    if (GLB_FLAG_ISSET(req->flags, GLB_QOS_GLOBAL_FLAG_POLICER_STATS_EN))
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_STATS_EN;
        glb_cfg.u.value = req->enable;        
        HAGT_IF_ERROR_RETURN(ctc_qos_set_global_config(&glb_cfg));
    
        HAGT_QOS_PROCESS_DEBUG_INFO("policer stats global enable: %ld", req->enable);
    }   
    if (GLB_FLAG_ISSET(req->flags, GLB_QOS_GLOBAL_FLAG_POLICER_IPG_EN))
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_IPG_EN;
        glb_cfg.u.value = req->enable;        
        HAGT_IF_ERROR_RETURN(ctc_qos_set_global_config(&glb_cfg));
    
        HAGT_QOS_PROCESS_DEBUG_INFO("policer ipg global enable: %ld", req->enable);
    }   
    if (GLB_FLAG_ISSET(req->flags, GLB_QOS_GLOBAL_FLAG_SHAPING_IPG_EN))
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_SHAPE_IPG_EN;
        glb_cfg.u.value = req->enable;        
        HAGT_IF_ERROR_RETURN(ctc_qos_set_global_config(&glb_cfg));
    
        HAGT_QOS_PROCESS_DEBUG_INFO("queue ipg global enable: %ld", req->enable);
    }     
    if (GLB_FLAG_ISSET(req->flags, GLB_QOS_GLOBAL_FLAG_GLB_EN))
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_RESRC_MGR_EN;
        glb_cfg.u.value = req->enable;        
        HAGT_IF_ERROR_RETURN(ctc_qos_set_global_config(&glb_cfg));
    
        HAGT_QOS_PROCESS_DEBUG_INFO("QoS global enable: %ld", req->enable);
    }
#ifdef GG_TODO
    /*modify by chenxw for bug 25492, 2013-11-29.*/
    if (GLB_FLAG_ISSET(req->flags, GLB_QOS_GLOBAL_FLAG_SMART_BUFFER_EN))
    {
        glb_cfg.cfg_type = CTC_QOS_GLB_CFG_SMART_BUFFER_EN;
        glb_cfg.u.value = req->enable;        
        HAGT_IF_ERROR_RETURN(ctc_qos_set_global_config(&glb_cfg));
    
        HAGT_QOS_PROCESS_DEBUG_INFO("QoS smart buffer enable: %ld", req->enable);
    }    
#endif
    // TODO: CTC_QOS_GLB_CFG_QUE_SHAPE_EN(done in sdk init)
    // TODO: CTC_QOS_GLB_CFG_GROUP_SHAPE_EN(done in sdk init)
    // TODO: CTC_QOS_GLB_CFG_PORT_SHAPE_EN(done in sdk init)
    // TODO: CTC_QOS_GLB_CFG_SHAPE_IPG_SIZE
    
    return HAGT_E_NONE;
}



 /* ~~~~~~~~~~~~~~~~~~~~~ classification ~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

 /****************************************************************************************
 * Name         : _hagt_qos_set_pri_color_map_table
 * Purpose      : map domain table to ctc domain table
 * Input        : req    - message data from hsrv to hagt
 * Output       : p_domain_map
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_map_domain_table(HalMsgQosSetPriColorMapTableReq_t *req,
                                            HalMsgQosPriColorMap_t * p_value, 
                                            ctc_qos_domain_map_t* p_domain_map)
{
    HAGT_QOS_PTR_CHECK(req);
    HAGT_QOS_PTR_CHECK(p_value);
    HAGT_QOS_PTR_CHECK(p_domain_map);

    HAGT_QOS_CLASS_DEBUG_FUNC();

    p_domain_map->domain_id = p_value->domain;
    p_domain_map->priority = p_value->pri;
    p_domain_map->color = p_value->color;

    
    switch (req->type)
    {
        case GLB_QOS_PRI_COLOR_MAP_TYPE_COS:
            p_domain_map->hdr_pri.dot1p.cos = p_value->value;
            p_domain_map->type = 
                (req->dir == GLB_INGRESS) ? CTC_QOS_DOMAIN_MAP_IGS_COS_TO_PRI_COLOR : CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_COS;
            break;
            
        case GLB_QOS_PRI_COLOR_MAP_TYPE_DSCP:
            p_domain_map->hdr_pri.tos.dscp = p_value->value;
            p_domain_map->type = 
                (req->dir == GLB_INGRESS) ? CTC_QOS_DOMAIN_MAP_IGS_DSCP_TO_PRI_COLOR : CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_DSCP;        
            break;
            
        case GLB_QOS_PRI_COLOR_MAP_TYPE_IP_PREC:
            p_domain_map->hdr_pri.ip_prec = p_value->value;
            p_domain_map->type = 
                (req->dir == GLB_INGRESS) ? CTC_QOS_DOMAIN_MAP_IGS_IP_PREC_TO_PRI_COLOR : CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_DSCP;        
            break;
        
        case GLB_QOS_PRI_COLOR_MAP_TYPE_EXP:
            p_domain_map->hdr_pri.exp = p_value->value;
            p_domain_map->type = 
                (req->dir == GLB_INGRESS) ? CTC_QOS_DOMAIN_MAP_IGS_EXP_TO_PRI_COLOR : CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_EXP;        
            break;
        case GLB_QOS_PRI_COLOR_MAP_TYPE_TOS:
	     p_domain_map->hdr_pri.tos.dscp = p_value->value >> 2;
	     p_domain_map->hdr_pri.tos.ecn = p_value->value & 0x3;
            p_domain_map->type = 
                (req->dir == GLB_INGRESS) ? CTC_QOS_DOMAIN_MAP_IGS_DSCP_TO_PRI_COLOR : CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_DSCP;        
            break;
            
        default :
            return CTC_E_INVALID_PARAM;
            break;
    }
    
    return HAGT_E_NONE;
}

// TODO: need consider mcast queue and ucast queue, Now just consider ucast queue for sdk has not finished
/****************************************************************************************
 * Name         : _hagt_qos_set_pri_color_qid_and_tid_map
 * Purpose      : Map priority and color to mpls-exp
 * Input        : pp_map_tbl    - pointer to map table
                  size          - map table size
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_set_pri_color_qid_and_tid_map(HalMsgQosPriColorMap_t** pp_map_tbl, int32 size)
{
    uint8  queue_shfit;
    uint8  qid;
    uint8 thresholdid;
    int32  index;
    ctc_qos_queue_cfg_t que_cfg;
    
    HAGT_QOS_CLASS_DEBUG_FUNC();
    
    HAGT_QOS_PTR_CHECK(pp_map_tbl);

    HAGT_QOS_CLASS_DEBUG_INFO("%-6s  %-8s  %-6s  %-8s", "priority", "color", "qid", "tid");
    HAGT_QOS_CLASS_DEBUG_INFO("----------------------------------");

    for (index = 0; index < size; index++)
    {
        sal_memset(&que_cfg, 0, sizeof(que_cfg));

        /* domain indicates queue num(uc or mc) on port or service */
        // TODO: need more consideration for mcast queue(Now we just support modify ucast queue)
        que_cfg.type = CTC_QOS_QUEUE_CFG_PRI_MAP;
        que_cfg.value.pri_map.priority = pp_map_tbl[index]->pri;
        que_cfg.value.pri_map.color = pp_map_tbl[index]->color;
        /*qid have the higher 5 bits, tid have the lower 3bits*/
        qid      = (pp_map_tbl[index]->value >> GLB_QOS_THRESHOLD_BIT_NUMBER) & GLB_QOS_QID_BIT_MASK;
        thresholdid = pp_map_tbl[index]->value&GLB_QOS_THRESHOLD_BIT_MASK;
        
        que_cfg.value.pri_map.drop_precedence = thresholdid;
        /* qid should left shift queue_shfit, for ASCI implementation */
        //queue_shfit = (queue_num / 2);  /* queue number must be 2 exponent */
        queue_shfit = 3;
        que_cfg.value.pri_map.queue_select = qid << queue_shfit;
        
        
        HAGT_IF_ERROR_RETURN(ctc_qos_set_queue(&que_cfg));
        
        HAGT_QOS_CLASS_DEBUG_INFO("%-8d  %-6d  %-8d %-6d", que_cfg.value.pri_map.priority, que_cfg.value.pri_map.color, qid, thresholdid);
    }

    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_map_trust_state 
 * Purpose      : Map qos trust state from glb to ctc
 * Input        : from_trust    - GLB_QOS_TRUST_XXX
 * Output       : p_to_trust    - pointer to CTC_QOS_TRUST_XXX value
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_map_trust_state(uint8 from_trust, ctc_qos_trust_type_t *p_to_trust)
{
    HAGT_QOS_PTR_CHECK(p_to_trust);
    
    switch (from_trust)
    {
        case GLB_QOS_TRUST_PORT:
            *p_to_trust = CTC_QOS_TRUST_PORT;
            break;
        case GLB_QOS_TRUST_OUTER:
            *p_to_trust = CTC_QOS_TRUST_OUTER;
            break;
        case GLB_QOS_TRUST_COS:
        case GLB_QOS_TRUST_COS_OUTER:
            *p_to_trust = CTC_QOS_TRUST_STAG_COS;
            break;
        case GLB_QOS_TRUST_COS_INNER:
            *p_to_trust = CTC_QOS_TRUST_CTAG_COS;
            break;
	 case GLB_QOS_TRUST_TOS:
        case GLB_QOS_TRUST_DSCP:
            *p_to_trust = CTC_QOS_TRUST_DSCP;
            break;
        case GLB_QOS_TRUST_IP_PREC:
            *p_to_trust = CTC_QOS_TRUST_IP;
            break;
        default:
            return HAGT_E_INVALID_PARAM;
    }

    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_set_pri_color_map_table
 * Purpose      : Set ingress/egress priority color mapping table
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_set_pri_color_map_table(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosSetPriColorMapTableReq_t *req;
    HalMsgQosPriColorMap_t **pp_array = NULL;
    int32  count = 0;
    int32 index = 0;
    int32 ecn_index = 0;
    ctc_qos_domain_map_t domain_map = { 0 };
    
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_CLASS_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetPriColorMapTable;
    pp_array = p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetPriColorMapTable.priColorList.list.array;
    count    = p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetPriColorMapTable.priColorList.list.count;

    /* pri ,color --> qid, tid */
    if (GLB_QOS_PRI_COLOR_MAP_TYPE_QID_TID == req->type)
    {
        HAGT_IF_ERROR_RETURN(_hagt_qos_set_pri_color_qid_and_tid_map(pp_array, count));
        return HAGT_E_NONE;
    }

    /* cos/dscp/exp/prec <--> pri, color */
    for (index = 0; index < count; index++)
    {
        sal_memset(&domain_map, 0, sizeof(domain_map));
        HAGT_IF_ERROR_RETURN(_hagt_qos_map_domain_table(req, pp_array[index], &domain_map));
        HAGT_IF_ERROR_RETURN(ctc_qos_set_domain_map(&domain_map));

        /*modify by chenxw for bug 22441, 2013-05-08.*/ 
        /*modify by chenxw for bug 22441, 2013-07-23.*/
        if(GLB_QOS_PRI_COLOR_MAP_TYPE_COS == req->type && GLB_INGRESS == req->dir)
        {
            domain_map.hdr_pri.dot1p.dei = 1;
            HAGT_IF_ERROR_RETURN(ctc_qos_set_domain_map(&domain_map));        
        }

        /* modified by chenxw for bug 25293, 2013-10-15.*/
        if (CTC_QOS_DOMAIN_MAP_IGS_DSCP_TO_PRI_COLOR == domain_map.type &&
	      GLB_QOS_PRI_COLOR_MAP_TYPE_DSCP == req->type)
        {
            for (ecn_index = 1; ecn_index < GLB_QOS_ECN_NUM; ecn_index++)
            {
                domain_map.hdr_pri.tos.ecn = ecn_index;
                HAGT_IF_ERROR_RETURN(ctc_qos_set_domain_map(&domain_map));
            }
        }

	  if ((CTC_QOS_DOMAIN_MAP_IGS_DSCP_TO_PRI_COLOR == domain_map.type) &&
	  	GLB_QOS_PRI_COLOR_MAP_TYPE_TOS == req->type)
        {
            HAGT_IF_ERROR_RETURN(ctc_qos_set_domain_map(&domain_map));
        }
        
        
        HAGT_QOS_CLASS_DEBUG_INFO("%-6ld  %-8ld  %-8ld  %-6ld  %-6ld  %-8ld", req->type, req->dir, pp_array[index]->domain, pp_array[index]->pri, 
                        pp_array[index]->color, pp_array[index]->value);
    }
  
    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_set_port_attribute 
 * Purpose      : Set port QoS attribute
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_set_port_attribute(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosSetPortAttributeReq_t *req;
    ctc_qos_trust_type_t trust;

    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_CLASS_DEBUG_FUNC();

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetPortAttribute;
    if (!hagt_chip_port_is_local(req->port))
    {
        return HAGT_E_NONE;
    }
    
    switch (req->type)
    {
        case GLB_QOS_PORT_ATTR_TYPE_TRUST_STATE:
            /* modified by rongl for bug 13990, 2011-02-11 */
            HAGT_IF_ERROR_RETURN(_hagt_qos_map_trust_state(req->value, &trust));
            HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_QOS_POLICY, trust));
            HAGT_QOS_CLASS_DEBUG_INFO("gport = %ld, trust = %ld", req->port, req->value);
            break;
            
        case GLB_QOS_PORT_ATTR_TYPE_DEFAULT_COS:
            HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_DEFAULT_PCP, req->value));
            HAGT_QOS_CLASS_DEBUG_INFO("gport = %ld, cos = %ld", req->port, req->value);
            break;
            
        case GLB_QOS_PORT_ATTR_TYPE_REPLACE_COS:
        // TODO: replace stag-cos, ctag-cos???        
            HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_REPLACE_STAG_COS, req->value));
            HAGT_QOS_CLASS_DEBUG_INFO("gport = %ld, replace_cos = %ld", req->port, req->value);
            break;
        case GLB_QOS_PORT_ATTR_TYPE_REPLACE_CTAG_COS:
        // TODO: replace stag-cos, ctag-cos???        
            HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_REPLACE_CTAG_COS, req->value));
            HAGT_QOS_CLASS_DEBUG_INFO("gport = %ld, replace_ctag-cos = %ld", req->port, req->value);
            break;                        
        case GLB_QOS_PORT_ATTR_TYPE_REPLACE_DSCP:
            HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_REPLACE_DSCP_EN, req->value));
            HAGT_QOS_CLASS_DEBUG_INFO("gport = %ld, replace_dscp = %ld", req->port, req->value);
            break;
            
        case GLB_QOS_PORT_ATTR_TYPE_DOMAIN:
            HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->port, CTC_PORT_DIR_PROP_QOS_DOMAIN, CTC_INGRESS, req->value));
            HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->port, CTC_PORT_DIR_PROP_QOS_DOMAIN, CTC_EGRESS, req->value));
            HAGT_QOS_CLASS_DEBUG_INFO("gport = %ld, domain = %ld", req->port, req->value);
            break;
            
        default:
            return HAGT_E_INVALID_PARAM;
    }
    
    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_set_cfi_enable
 * Purpose      : Set CFI of qos domain for cos map
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_set_cfi_enable(Hsrv2AgtMsg_t* p_msg_data)
{
    uint8 domain;
    bool enable;
    ctc_qos_domain_map_t domain_map = { 0 };
    uint8 cos;
    uint8 pri;
    
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_CLASS_DEBUG_FUNC();

    /* 1. get message data */
    domain = p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetCfiEnable.domain;
    enable = p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetCfiEnable.enable ? TRUE : FALSE;

    HAGT_QOS_CLASS_DEBUG_INFO("%s cfi for domain %d", (enable ? "enable" : "disable"), domain);

    /* set igs cfi : cos + cfi(0/1) -> pri + green/yellow */
    for (cos = 0; cos < GLB_QOS_COS_NUM; cos++)
    {
        sal_memset(&domain_map, 0, sizeof(domain_map));

        domain_map.domain_id = domain;
        domain_map.type = CTC_QOS_DOMAIN_MAP_IGS_COS_TO_PRI_COLOR;
        domain_map.hdr_pri.dot1p.cos = cos;
        
        /* get cfi = 0 map table */
        domain_map.hdr_pri.dot1p.dei = 0;        
        HAGT_IF_ERROR_RETURN(ctc_qos_get_domain_map(&domain_map));
        
        if (enable)
        {
            /* if set cfi enable, set cfi=1 map table with modify green to yellow*/
            if (domain_map.color == CTC_QOS_COLOR_GREEN)
            {
                domain_map.color = CTC_QOS_COLOR_YELLOW;                 
            }
            domain_map.hdr_pri.dot1p.dei = 1; 
        }
        else
        {
            /* if unset cfi enable, get cfi = 0 map table, set cfi=1 map table same as cif=0*/
            domain_map.hdr_pri.dot1p.dei = 1;            
        }
        
        HAGT_IF_ERROR_RETURN(ctc_qos_set_domain_map(&domain_map));
        
    }

    /* set egs cfi :  pri + yellow -> cos + cfi(0/1)*/
    for (pri = 0; pri < GLB_QOS_PRIORITY_NUM; pri++)
    {
        sal_memset(&domain_map, 0, sizeof(domain_map));

        domain_map.domain_id = domain;
        domain_map.type = CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_COS;
        domain_map.priority = pri;
        domain_map.color = CTC_QOS_COLOR_YELLOW;
        
        /* get yellow to cos map table */
        HAGT_IF_ERROR_RETURN(ctc_qos_get_domain_map(&domain_map));
        
        if (enable)
        {
            /* if set cfi enable, set pri + yellow -> cos + cfi(1)*/
            domain_map.hdr_pri.dot1p.dei = 1; 
        }
        else
        {
            /* if unset cfi enable, pri + yellow -> cos + cfi(0)*/
            domain_map.hdr_pri.dot1p.dei = 0;            
        }        
        HAGT_IF_ERROR_RETURN(ctc_qos_set_domain_map(&domain_map));
        
    }    

    

    return HAGT_E_NONE;
}


 /* ~~~~~~~~~~~~~~~~~~~~~ policer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
 
/****************************************************************************************
 * Name         : _hagt_qos_map_qos_policer 
 * Purpose      : Map qos policer from hagt to ctc
 * Input        : p_hagt_policer    - pointer to hagt policer
 * Output       : p_ctc_policer     - pointer to ctc policer
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_map_qos_policer(HalMsgQosPolicer_t* p_hagt_policer, ctc_qos_policer_param_t* p_ctc_policer)
{
    HAGT_QOS_PTR_CHECK(p_hagt_policer);
    HAGT_QOS_PTR_CHECK(p_ctc_policer);

    HAGT_QOS_POLICER_DEBUG_FUNC();

#ifndef DUET2
    p_ctc_policer->is_srtcm       = p_hagt_policer->isSrTcm;
#endif
    p_ctc_policer->is_color_aware = !p_hagt_policer->isColorBlind;
    p_ctc_policer->use_l3_length  = p_hagt_policer->useL3Length;
    p_ctc_policer->drop_color     = p_hagt_policer->dropColor;
    /*Added by yanxa, for bug 17215, 2011-12-31*/
    p_ctc_policer->stats_en    = p_hagt_policer->isStatsEn;

//    p_ctc_policer->cir = HAGT_QOS_MAP_RATE_KBIT2BYTE(p_hagt_policer->cir);
    p_ctc_policer->cir = p_hagt_policer->cir;
//    p_ctc_policer->cbs = p_hagt_policer->cbs;
    p_ctc_policer->cbs = HAGT_QOS_MAP_RATE_BYTE2KBIT(p_hagt_policer->cbs);
//    p_ctc_policer->pir = HAGT_QOS_MAP_RATE_KBIT2BYTE(p_hagt_policer->pir);
    p_ctc_policer->pir = p_hagt_policer->pir;
//    p_ctc_policer->pbs = p_hagt_policer->pbs;
    p_ctc_policer->pbs = HAGT_QOS_MAP_RATE_BYTE2KBIT(p_hagt_policer->pbs);

    p_ctc_policer->policer_mode = p_hagt_policer->mode;

    /*modify by chenxw for bug 22441, 2013-07-29.*/
    if(CTC_QOS_POLICER_MODE_MEF_BWP == p_ctc_policer->policer_mode)
    {
        p_ctc_policer->cf_en = 1;
    }

    HAGT_QOS_POLICER_DEBUG_INFO("is_stats_en    = %d", p_ctc_policer->stats_en);
    HAGT_QOS_POLICER_DEBUG_INFO("is_srtcm       = %d", p_ctc_policer->is_srtcm);
    HAGT_QOS_POLICER_DEBUG_INFO("is_color_blind = %d", !p_ctc_policer->is_color_aware);
    HAGT_QOS_POLICER_DEBUG_INFO("use_l3_length  = %d", p_ctc_policer->use_l3_length);
    HAGT_QOS_POLICER_DEBUG_INFO("drop_color     = %d", p_ctc_policer->drop_color);
    
//    HAGT_QOS_POLICER_DEBUG_INFO("cir = %u (Byte/s)", p_ctc_policer->cir);
    HAGT_QOS_POLICER_DEBUG_INFO("cir = %u (kbit/s)", p_ctc_policer->cir);
//    HAGT_QOS_POLICER_DEBUG_INFO("cbs = %u ",          p_ctc_policer->cbs);
    HAGT_QOS_POLICER_DEBUG_INFO("cbs = %u (kbit)",          p_ctc_policer->cbs);
//    HAGT_QOS_POLICER_DEBUG_INFO("pir = %u (Byte/s)", p_ctc_policer->pir);
    HAGT_QOS_POLICER_DEBUG_INFO("pir = %u (kbit/s)", p_ctc_policer->pir);
//    HAGT_QOS_POLICER_DEBUG_INFO("pbs = %u ",          p_ctc_policer->pbs);
    HAGT_QOS_POLICER_DEBUG_INFO("pbs = %u (kbit)",          p_ctc_policer->pbs);
    HAGT_QOS_POLICER_DEBUG_INFO("mode = %d",          p_ctc_policer->policer_mode);

    

    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_flow_policer_op 
 * Purpose      : QoS flow policer operation: create/remove/upate
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_flow_policer_op(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosFlowPolicerOpReq_t *req;
    ctc_qos_policer_t ctc_policer;
    int ret;

    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_POLICER_DEBUG_FUNC();

    sal_memset(&ctc_policer, 0, sizeof(ctc_qos_policer_t));

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosFlowPolicerOp;

    ctc_policer.id.policer_id = req->policerId;
    ctc_policer.type = CTC_QOS_POLICER_TYPE_FLOW;
    /* for flow policer, policer is determinated by policer_id, so we set flow policer dir as BOTH */
    /* in _sys_greatbelt_qos_policer_policer_id_check(), BOTH is invalid, so we set dir as CTC_INGRESS*/
    ctc_policer.dir = CTC_INGRESS;
    switch (req->opCode)
    {
        /* create flow policer */
        case HalMsgQosFlowPolicerOp_create:
            ctc_policer.enable = 1;
            HAGT_IF_ERROR_RETURN(_hagt_qos_map_qos_policer(&req->policer, &ctc_policer.policer));
            HAGT_IF_ERROR_RETURN(ctc_qos_set_policer(&ctc_policer));
            HAGT_QOS_POLICER_DEBUG_INFO("create flow policer, policer_id = %lu", req->policerId);
            break;

        /* remove flow policer */
        case HalMsgQosFlowPolicerOp_remove:
            ctc_policer.enable = 0;
            HAGT_IF_ERROR_RETURN(ctc_qos_set_policer(&ctc_policer));
            HAGT_QOS_POLICER_DEBUG_INFO("remove flow policer, policer_id = %lu", req->policerId);
            break;

        /* update flow policer */
        case HalMsgQosFlowPolicerOp_update:
            ctc_policer.enable = 1;
            HAGT_IF_ERROR_RETURN(_hagt_qos_map_qos_policer(&req->policer, &ctc_policer.policer));
            ret = ctc_qos_set_policer(&ctc_policer);
            if (ret)
            {
                HAGT_QOS_LOG_ERR("Failed to update flow policer, policer_id = %lu", req->policerId);
            }
            else
            {
                HAGT_QOS_POLICER_DEBUG_INFO("update flow policer, policer_id = %lu", req->policerId);
            }
            break;

        default:
            return HAGT_E_INVALID_PARAM;
    }
    
    return HAGT_E_NONE;
}


/****************************************************************************************
 * Name         : _hagt_qos_set_port_policer
 * Purpose      : Set policer on a port at the specified direction
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_set_port_policer(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosSetPortPolicerReq_t *req;
    ctc_qos_policer_t ctc_policer = { 0 };
    
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_POLICER_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetPortPolicer;

    /* 2. map qos policer from hagt to ctc */
    //sal_memset(&ctc_policer, 0, sizeof(ctc_qos_policer_t));
    HAGT_IF_ERROR_RETURN(_hagt_qos_map_qos_policer(&req->policer, &ctc_policer.policer));

    ctc_policer.dir = req->dir;
    ctc_policer.type = CTC_QOS_POLICER_TYPE_PORT;
    ctc_policer.id.policer_id = req->port;
    ctc_policer.enable = TRUE;
    
    /* 3. set port policer */
    HAGT_IF_ERROR_RETURN(ctc_qos_set_policer(&ctc_policer));

    HAGT_QOS_POLICER_DEBUG_INFO("set port policer, gport = %ld, dir = %ld", req->port, req->dir);
    
    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_unset_port_policer
 * Purpose      : Cancel policer on a port at the specified direction
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_unset_port_policer(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosUnsetPortPolicerReq_t *req;
    ctc_qos_policer_t ctc_policer = { 0 };
    
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_POLICER_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosUnsetPortPolicer;

    /* 2. unset port policer */
    
    ctc_policer.dir = req->dir;
    ctc_policer.type = CTC_QOS_POLICER_TYPE_PORT;
    ctc_policer.id.policer_id = req->port;
    ctc_policer.enable = FALSE;
    
    HAGT_IF_ERROR_RETURN(ctc_qos_set_policer(&ctc_policer));

    HAGT_QOS_POLICER_DEBUG_INFO("unset port policer, gport = %ld, dir = %ld", req->port, req->dir);
    
    return HAGT_E_NONE;
}

 /* ~~~~~~~~~~~~~~~~~~~~~ queue ~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/****************************************************************************************
 * Name         : _hagt_qos_map_queue_drop
 * Purpose      : Map queue drop from hagt to ctc
 * Input        : p_hagt_drop   - pointer to hagt queue drop
 * Output       : p_ctc_drop    - pointer to ctc queue drop
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_map_queue_drop(HalMsgQosQueueDrop_t* p_hagt_drop, ctc_qos_queue_drop_t* p_ctc_drop)
{    
    HAGT_QOS_PTR_CHECK(p_hagt_drop);
    HAGT_QOS_PTR_CHECK(p_ctc_drop);

    HAGT_QOS_QUEUE_DEBUG_FUNC();

    p_ctc_drop->mode = p_hagt_drop->mode;
    
    p_ctc_drop->min_th[0] = p_hagt_drop->minThresh0;
    p_ctc_drop->min_th[1] = p_hagt_drop->minThresh1;
    p_ctc_drop->min_th[2] = p_hagt_drop->minThresh2;
    p_ctc_drop->min_th[3] = p_hagt_drop->minThresh3;
    
    p_ctc_drop->max_th[0] = p_hagt_drop->maxThresh0;
    p_ctc_drop->max_th[1] = p_hagt_drop->maxThresh1;
    p_ctc_drop->max_th[2] = p_hagt_drop->maxThresh2;
    p_ctc_drop->max_th[3] = p_hagt_drop->maxThresh3;

/* modified by chenxw for bug 25816, 2013-08-28 */
#ifdef _CENTEC_
    p_ctc_drop->drop_prob[0] = GLB_QOS_MAX_DROP_PROBABILITY_CTC * p_hagt_drop->dropProb0 / GLB_QOS_MAX_DROP_PROBABILITY_VALUE;
    p_ctc_drop->drop_prob[1] = GLB_QOS_MAX_DROP_PROBABILITY_CTC * p_hagt_drop->dropProb1 / GLB_QOS_MAX_DROP_PROBABILITY_VALUE;
    p_ctc_drop->drop_prob[2] = GLB_QOS_MAX_DROP_PROBABILITY_CTC * p_hagt_drop->dropProb2 / GLB_QOS_MAX_DROP_PROBABILITY_VALUE;
    p_ctc_drop->drop_prob[3] = GLB_QOS_MAX_DROP_PROBABILITY_CTC * p_hagt_drop->dropProb3 / GLB_QOS_MAX_DROP_PROBABILITY_VALUE;
#else /*_CENTEC_*/
    /*modified by xgu for bug 16265, 2011-11-09*/
    // TODO: need check this mapping, added by cuixl
    HAGT_QOS_MAP_WRED_DROP_PROB(p_ctc_drop->drop_prob[0], p_hagt_drop->dropProb0,
            (p_ctc_drop->max_th[0] - p_ctc_drop->min_th[0]));
    HAGT_QOS_MAP_WRED_DROP_PROB(p_ctc_drop->drop_prob[1], p_hagt_drop->dropProb1,
            (p_ctc_drop->max_th[1] - p_ctc_drop->min_th[1]));
    HAGT_QOS_MAP_WRED_DROP_PROB(p_ctc_drop->drop_prob[2], p_hagt_drop->dropProb2,
            (p_ctc_drop->max_th[2] - p_ctc_drop->min_th[2]));
    HAGT_QOS_MAP_WRED_DROP_PROB(p_ctc_drop->drop_prob[3], p_hagt_drop->dropProb3,
            (p_ctc_drop->max_th[3] - p_ctc_drop->min_th[3]));
#endif

#ifndef DUET2
    p_ctc_drop->weight = p_hagt_drop->weight;
#endif

    HAGT_QOS_QUEUE_DEBUG_INFO("mode   = %d", p_ctc_drop->mode);
    HAGT_QOS_QUEUE_DEBUG_INFO("weight = %u", p_ctc_drop->weight);
    HAGT_QOS_QUEUE_DEBUG_INFO("min threshold = %6d %6d %6d %6d", p_ctc_drop->min_th[0], 
                              p_ctc_drop->min_th[1], p_ctc_drop->min_th[2], p_ctc_drop->min_th[3]);
    HAGT_QOS_QUEUE_DEBUG_INFO("max threshold = %6d %6d %6d %6d", p_ctc_drop->max_th[0], 
                              p_ctc_drop->max_th[1], p_ctc_drop->max_th[2], p_ctc_drop->max_th[3]);
    HAGT_QOS_QUEUE_DEBUG_INFO("drop prob factor = %6d %6d %6d %6d", p_ctc_drop->drop_prob[0], 
                              p_ctc_drop->drop_prob[1], p_ctc_drop->drop_prob[2], p_ctc_drop->drop_prob[3]);

    return HAGT_E_NONE;
}
/****************************************************************************************
 * Name         : _hagt_qos_set_port_queue_drop
 * Purpose      : Set port queue drop scheme
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_set_port_queue_drop(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosSetPortQueueDropReq_t *req;
    ctc_qos_drop_t ctc_drop;
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_QUEUE_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetPortQueueDrop;
    if (!hagt_chip_port_is_local(req->port))
    {
        return HAGT_E_NONE;
    }

    /* 2. map queue drop from hagt to ctc */
    sal_memset(&ctc_drop, 0, sizeof(ctc_drop));
    HAGT_IF_ERROR_RETURN(_hagt_qos_map_queue_drop(&req->drop, &ctc_drop.drop));

    // TODO:  merge sdk 2.1.13
    ctc_drop.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    ctc_drop.queue.gport = req->port;
    ctc_drop.queue.queue_id = req->qid;
    
    /* 3. set port queue drop */
    HAGT_QOS_QUEUE_DEBUG_INFO("set port queue drop, gport = 0x%lx, qid = %ld", req->port, req->qid);

    HAGT_IF_ERROR_RETURN(ctc_qos_set_drop_scheme(&ctc_drop));
    
    
    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_set_port_queue_stats
 * Purpose      : Set port queue drop scheme
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_set_port_queue_stats(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosSetPortQueueStatsReq_t *req;
    ctc_qos_queue_cfg_t que_cfg = { 0 };
    uint8  qid = 0;
    
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_QUEUE_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetPortQueueStats;

    /* 2. map queue stats from hagt to ctc */

    que_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN;
    // TODO:  merge sdk 2.1.13
    que_cfg.value.stats.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;

    que_cfg.value.stats.stats_en = req->enable;
    que_cfg.value.stats.queue.gport = req->port;
    que_cfg.value.stats.queue.queue_id = req->qid;
    
    /* 3. set port queue drop */
    HAGT_QOS_QUEUE_DEBUG_INFO("set port queue stats, gport = 0x%lx, qid = %ld, enable = %ld", req->port, req->qid, req->enable);

    if (req->qid == GLB_QOS_ALL_QUEUE_ID)
    {
        for (qid = 0; qid < HAGT_QOS_PORT_QUEUE_NUM; qid++)
        {
            que_cfg.value.stats.queue.queue_id = qid;
            HAGT_IF_ERROR_RETURN(ctc_qos_set_queue(&que_cfg));
        }
    }
    else
    {
        HAGT_IF_ERROR_RETURN(ctc_qos_set_queue(&que_cfg));
    }
        
    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_set_port_queue_class
 * Purpose      : Map port queue to the specified class
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_set_port_queue_class(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosSetPortQueueClassReq_t *req;
    int32 ret ;
    ctc_qos_sched_t queue_sched;
    
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_QUEUE_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetPortQueueClass;

    /* 2. set port queue class */
    HAGT_QOS_QUEUE_DEBUG_INFO("set port queue class, gport = 0x%lx, qid = %ld, class = %ld, force = %ld",
                              req->port, req->qid, req->Class, req->force);

    if (!hagt_chip_port_is_local(req->port))
    {
        return HAGT_E_NONE;
    }

    sal_memset(&queue_sched, 0, sizeof(queue_sched));
    queue_sched.type = CTC_QOS_SCHED_QUEUE;
    queue_sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    queue_sched.sched.queue_sched.queue.gport = req->port;
    queue_sched.sched.queue_sched.queue.queue_id = req->qid;    
    queue_sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
    queue_sched.sched.queue_sched.exceed_class = req->Class;
    /*modified by yanxa, for bug 14875, 2011-06-21*/
    // TODO: need confirm req->force anymore ??
    if (req->force)
    {
        HAGT_IF_ERROR_RETURN(ctc_qos_set_sched(&queue_sched));
        return HAGT_E_NONE;
    }

    ret = ctc_qos_set_sched(&queue_sched);
    if (CTC_E_NONE != ret && CTC_E_QUEUE_DEPTH_NOT_EMPTY != ret)
    {
        HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__);
    }

    //_hagt_qos_notify_queue_class_result(req->port, req->qid, req->Class, (CTC_E_NONE == ret));
    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_calculate_queue_wdrr_weight_gcd
 * Purpose      : Calculate greatest common divisorqueue for wdrr weight
 * Input        : pp_weights    - weights for wdrr
                  len           - length for wdrr weights array
 * Output       : p_gcd         - greatest common divisorqueue
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32
_hagt_qos_calculate_queue_wdrr_weight_gcd(uint32** pp_weights, uint32 len, uint32* p_gcd)
{
    HAGT_QOS_PTR_CHECK(p_gcd);
    HAGT_QOS_PTR_CHECK(pp_weights);
    HAGT_QOS_PTR_CHECK((*pp_weights));

    switch (len)
    {
    case 0:
        return HAGT_E_INVALID_PARAM;
    case 1:
        *p_gcd = *(pp_weights[0]);
        return HAGT_E_NONE;
    default:
        break;
    }

    uint32 minimum = (*pp_weights[0]);
    int32 i = 0;
    for (i = 0; i < len; i++)
    {
        minimum = (minimum <= (*pp_weights[i])) ? minimum : (*pp_weights[i]);
    }

    uint32 divisorqueue;
    bool is_found = FALSE;
    for (divisorqueue = minimum; divisorqueue > 1; divisorqueue--)
    {
        for (i = 0; i < len; i++)
        {
            if (0 != ((*pp_weights[i]) % divisorqueue))
            {
                is_found = FALSE;
                break;
            }
            else
            {
                is_found = TRUE;
            }
        }

        if (is_found)
        {
            break;
        }
    }

    if (0 == divisorqueue)
    {
        return HAGT_E_NOT_FOUND;
    }

    *p_gcd = divisorqueue;
    return HAGT_E_NONE;
}
/****************************************************************************************
 * Name         : _hagt_qos_set_port_queue_wdrr_weight
 * Purpose      : Set port queue wdrr weight
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_set_port_queue_wdrr_weight(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosSetPortQueueWdrrWeightReq_t *req;
    uint32 **pp_array;
    int32  qid, count = 0;
    uint32 gcd = 0;
    uint32 weight = 0;
    ctc_qos_sched_t queue_sched;

    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_QUEUE_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetPortQueueWdrrWeight;
    
    pp_array = (uint32 **)req->weightList.list.array;
    count    = req->weightList.list.count;
    GLB_EQUAL_CHECK(count, HAGT_QOS_PORT_QUEUE_NUM, HAGT_E_INVALID_PARAM);

    HAGT_IF_ERROR_RETURN(_hagt_qos_calculate_queue_wdrr_weight_gcd(pp_array, HAGT_QOS_PORT_QUEUE_NUM, &gcd));
    HAGT_QOS_QUEUE_DEBUG_INFO("calculate wdrr weight gcd %u", gcd);

    sal_memset(&queue_sched, 0, sizeof(queue_sched));
    
    queue_sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    queue_sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_WEIGHT;
    queue_sched.sched.queue_sched.queue.gport = req->port;
    queue_sched.type = CTC_QOS_SCHED_QUEUE;
    
    /* 2. set port queue wdrr weight */
    for (qid = 0; qid < HAGT_QOS_PORT_QUEUE_NUM; qid++)    
    {
        weight = *pp_array[qid] / gcd;    
        queue_sched.sched.queue_sched.exceed_weight = weight;
        queue_sched.sched.queue_sched.queue.queue_id = qid;
        HAGT_QOS_QUEUE_DEBUG_INFO("set port queue wdrr weight, gport = 0x%lx, qid = %d, weight = %d(from %d)", 
                                  req->port, qid, weight, *pp_array[qid]);

        HAGT_IF_ERROR_RETURN(ctc_qos_set_sched(&queue_sched));
        
    }
    
    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_map_queue_shape
 * Purpose      : Map queue shape from hagt to ctc
 * Input        : p_hagt_shape  - pointer to hagt queue shape
 * Output       : p_ctc_shape   - pointer to ctc queue shape
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_map_queue_shape(HalMsgQosQueueShape_t* p_hagt_shape, ctc_qos_shape_queue_t* p_ctc_shape)
{
    HAGT_QOS_PTR_CHECK(p_hagt_shape);
    HAGT_QOS_PTR_CHECK(p_ctc_shape);

    HAGT_QOS_QUEUE_DEBUG_FUNC();

    p_ctc_shape->cir = HAGT_QOS_MAP_RATE_BYTE2KBIT(p_hagt_shape->cir);
    //p_ctc_shape->cbs = HAGT_QOS_MAP_RATE_BYTE2KBIT(p_hagt_shape->cbs);
    p_ctc_shape->cbs = CTC_QOS_SHP_TOKE_THRD_DEFAULT;
    p_ctc_shape->pir = HAGT_QOS_MAP_RATE_BYTE2KBIT(p_hagt_shape->pir);
    //p_ctc_shape->pbs = HAGT_QOS_MAP_RATE_BYTE2KBIT(p_hagt_shape->pbs);
    p_ctc_shape->pbs = CTC_QOS_SHP_TOKE_THRD_DEFAULT;

    HAGT_QOS_QUEUE_DEBUG_INFO("cir = %u (kbits/s)", p_ctc_shape->cir);
    HAGT_QOS_QUEUE_DEBUG_INFO("cbs = %u (kbit)", p_ctc_shape->cbs);
    HAGT_QOS_QUEUE_DEBUG_INFO("pir = %u (kbits/s)", p_ctc_shape->pir);
    HAGT_QOS_QUEUE_DEBUG_INFO("pbs = %u (kbit)", p_ctc_shape->pbs);
    
    return HAGT_E_NONE;
}


/****************************************************************************************
 * Name         : _hagt_qos_set_port_queue_shape
 * Purpose      : Set queue shaping for the specified port
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_set_port_queue_shape(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosSetPortQueueShapeReq_t *req;
    ctc_qos_shape_t ctc_shape = { 0 };
    
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_QUEUE_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetPortQueueShape;

    /* 2. map queue shape from hagt to ctc */
//    sal_memset(&ctc_shape, 0, sizeof(ctc_shape));
    HAGT_IF_ERROR_RETURN(_hagt_qos_map_queue_shape(&req->shape, &ctc_shape.shape.queue_shape));

    /* 3. set port queue shape */
    ctc_shape.type = CTC_QOS_SHAPE_QUEUE;
    ctc_shape.shape.queue_shape.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    ctc_shape.shape.queue_shape.queue.queue_id = req->qid;
    ctc_shape.shape.queue_shape.queue.gport = req->port;
    ctc_shape.shape.queue_shape.enable = TRUE;
    
    HAGT_IF_ERROR_RETURN(ctc_qos_set_shape(&ctc_shape));

    HAGT_QOS_QUEUE_DEBUG_INFO("set port queue shape, gport = 0x%lx, qid = %ld", req->port, req->qid);
    
    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_unset_port_queue_shape
 * Purpose      : Unset queue shaping for the specified port
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_unset_port_queue_shape(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosUnsetPortQueueShapeReq_t *req;
    ctc_qos_shape_t ctc_shape;

    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_QUEUE_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosUnsetPortQueueShape;
    
    ctc_shape.type = CTC_QOS_SHAPE_QUEUE;
    ctc_shape.shape.queue_shape.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    ctc_shape.shape.queue_shape.queue.queue_id = req->qid;
    ctc_shape.shape.queue_shape.queue.gport = req->port;
    ctc_shape.shape.queue_shape.enable = FALSE;

    /* 2. unset port queue shape */
    HAGT_IF_ERROR_RETURN(ctc_qos_set_shape(&ctc_shape));

    HAGT_QOS_QUEUE_DEBUG_INFO("unset port queue shape, gport = 0x%lx, qid = %ld", req->port, req->qid);
    
    return HAGT_E_NONE;
}



/****************************************************************************************
 * Name         : _hagt_qos_init_qos_port_cfg
 * Purpose      : Initialize QoS port config
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_init_qos_port_cfg(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosInitPortCfgReq_t *req;
    ctc_qos_drop_t ctc_drop;
    uint8 qid;
    ctc_qos_sched_t queue_sched;
    uint8  queue_class;        

    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_PROCESS_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosInitPortCfg;
    if (!hagt_chip_port_is_local(req->port))
    {
        return HAGT_E_NONE;
    }

    /* 2. port qos domain */
    HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->port, CTC_PORT_DIR_PROP_QOS_DOMAIN, 
                        CTC_INGRESS, req->domain));
    HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->port, CTC_PORT_DIR_PROP_QOS_DOMAIN, 
                        CTC_EGRESS, req->domain));                        

    /* 3. port trust state */
    HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_QOS_POLICY, req->trust));

    /* 4. port default cos */
    HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_DEFAULT_PCP, req->cos));

    /* 5. port replace cos & dscp */
    // TODO: confirm replace ctag-cos or stag-cos
    HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_REPLACE_STAG_COS,  req->replaceCos));
    HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_REPLACE_DSCP_EN, req->replaceDscp));

    /* 7. queue drop */
    sal_memset(&ctc_drop, 0, sizeof(ctc_drop));
    HAGT_IF_ERROR_RETURN(_hagt_qos_map_queue_drop(&req->drop, &ctc_drop.drop));
    ctc_drop.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    ctc_drop.queue.gport = req->port;

/* sdk will process mcast queue and ucast queue */
    for (qid = 0; qid < HAGT_QOS_PORT_QUEUE_NUM; qid++)
    {
        ctc_drop.queue.queue_id = qid;    
        HAGT_IF_ERROR_RETURN(ctc_qos_set_drop_scheme(&ctc_drop));
    }

    /*8. set queue confirm class to 7, modify by chenxw for bug 22441, 2013-05-14.*/
    sal_memset(&queue_sched, 0, sizeof(queue_sched));
    queue_sched.type = CTC_QOS_SCHED_QUEUE;
    queue_sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
    queue_sched.sched.queue_sched.confirm_class = 7;
    queue_sched.sched.queue_sched.queue.gport = req->port;
    queue_sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;

    for (qid = 0; qid < HAGT_QOS_PORT_QUEUE_NUM; qid++)
    {
        queue_sched.sched.queue_sched.queue.queue_id = qid;
        HAGT_IF_ERROR_RETURN(ctc_qos_set_sched(&queue_sched));
    }

    /*9. group class, added by chenxw for bug22441, 2013-05-14.*/    
    sal_memset(&queue_sched, 0, sizeof(queue_sched));
    queue_sched.type = CTC_QOS_SCHED_GROUP;
    queue_sched.sched.group_sched.class_priority = 3;
    queue_sched.sched.group_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
    queue_sched.sched.group_sched.gport = req->port;
    queue_sched.sched.group_sched.group_type = CTC_QOS_SCHED_GROUP_PORT;
 
    for (queue_class = 0; queue_class < 8; queue_class++)
    {
        queue_sched.sched.group_sched.queue_class = queue_class;
        HAGT_IF_ERROR_RETURN(ctc_qos_set_sched(&queue_sched));
    }
    


    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_sync_qos_port_cfg
 * Purpose      : Initialize QoS port config
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_sync_qos_port_cfg(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosSyncPortCfgReq_t *req;
    ctc_qos_drop_t ctc_drop;
    uint8 qid;
    uint32 gcd = 0;   
    uint32 weight = 0;    
    ctc_qos_sched_t queue_sched;
    ctc_qos_policer_t ctc_policer = { 0 };    
    ctc_qos_shape_t ctc_shape = { 0 };    
    ctc_qos_queue_cfg_t que_cfg = { 0 };    
    uint8  group_class;       

    uint32 **wdrr_weight; 
    uint32 **queue_class;     
    HalMsgQosQueueDrop_t **pp_drop_array = NULL;
    HalMsgQosQueueShape_t **pp_queue_shape_array = NULL;    
    
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_PROCESS_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSyncPortCfg;
    if (!hagt_chip_port_is_local(req->port))
    {
        return HAGT_E_NONE;
    }

    wdrr_weight = (uint32 **)req->queuewdrrWeight.list.array;
    queue_class = (uint32 **)req->queueclass.list.array;
    pp_drop_array = req->drop.list.array;
    pp_queue_shape_array = req->queueshape.list.array;

    GLB_EQUAL_CHECK(req->queuewdrrWeight.list.count, HAGT_QOS_PORT_QUEUE_NUM, HAGT_E_INVALID_PARAM);    
    GLB_EQUAL_CHECK(req->queueclass.list.count, HAGT_QOS_PORT_QUEUE_NUM, HAGT_E_INVALID_PARAM);    
    GLB_EQUAL_CHECK(req->drop.list.count, HAGT_QOS_PORT_QUEUE_NUM, HAGT_E_INVALID_PARAM);    
    GLB_EQUAL_CHECK(req->queueshape.list.count, HAGT_QOS_PORT_QUEUE_NUM, HAGT_E_INVALID_PARAM);       

    /* 2. port qos domain */
    HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->port, CTC_PORT_DIR_PROP_QOS_DOMAIN, 
                        CTC_INGRESS, req->domain));
    HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->port, CTC_PORT_DIR_PROP_QOS_DOMAIN, 
                        CTC_EGRESS, req->domain));                        

    /* 3. port trust state */
    HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_QOS_POLICY, req->trust));

    /* 4. port default cos */
    HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_DEFAULT_PCP, req->cos));

    /* 5. port replace cos & dscp */
    HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_REPLACE_STAG_COS,  req->replaceCos));
    HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_REPLACE_CTAG_COS,  req->replaceCtagCos));    
    HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_REPLACE_DSCP_EN, req->replaceDscp));

    if (req->qosenable && req->inputportpolicer.mode != -1)
    {
        sal_memset(&ctc_policer, 0 , sizeof(ctc_qos_policer_t));    
        HAGT_IF_ERROR_RETURN(_hagt_qos_map_qos_policer(&req->inputportpolicer, &ctc_policer.policer));
        ctc_policer.dir = GLB_INGRESS;
        ctc_policer.type = CTC_QOS_POLICER_TYPE_PORT;
        ctc_policer.id.policer_id = req->port;
        ctc_policer.enable = TRUE;   
        HAGT_IF_ERROR_RETURN(ctc_qos_set_policer(&ctc_policer));
    }

    if (req->qosenable && req->outputportpolicer.mode != -1)
    {    
        sal_memset(&ctc_policer, 0 , sizeof(ctc_qos_policer_t));
        HAGT_IF_ERROR_RETURN(_hagt_qos_map_qos_policer(&req->outputportpolicer, &ctc_policer.policer));
        ctc_policer.dir = GLB_EGRESS;
        ctc_policer.type = CTC_QOS_POLICER_TYPE_PORT;
        ctc_policer.id.policer_id = req->port;
        ctc_policer.enable = TRUE;   
        HAGT_IF_ERROR_RETURN(ctc_qos_set_policer(&ctc_policer));  
    }

    if (req->qosenable && req->portshape.pir != -1)
    { 
        /* 2. set port shape */
        sal_memset(&ctc_shape, 0, sizeof(ctc_shape));       
        ctc_shape.type = CTC_QOS_SHAPE_PORT;
        ctc_shape.shape.port_shape.gport = req->port;
        ctc_shape.shape.port_shape.pir = HAGT_QOS_MAP_RATE_BYTE2KBIT(req->portshape.pir);
        ctc_shape.shape.port_shape.pbs = CTC_QOS_SHP_TOKE_THRD_DEFAULT;
        ctc_shape.shape.port_shape.enable = TRUE;
        HAGT_IF_ERROR_RETURN(ctc_qos_set_shape(&ctc_shape));
    }

    /*modify by chenxw for bug 27372, 2014-03-03.*/
    if (req->qosenable)
    {
        que_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN;
        que_cfg.value.stats.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;

        que_cfg.value.stats.stats_en = req->stats;
        que_cfg.value.stats.queue.gport = req->port;
    
        /* 3. set port queue drop */
        for (qid = 0; qid < HAGT_QOS_PORT_QUEUE_NUM; qid++)
        {
            que_cfg.value.stats.queue.queue_id = qid;
            HAGT_IF_ERROR_RETURN(ctc_qos_set_queue(&que_cfg));
        }    
    }
    

    if (req->qosenable)
    {
        HAGT_IF_ERROR_RETURN(_hagt_qos_calculate_queue_wdrr_weight_gcd(wdrr_weight, HAGT_QOS_PORT_QUEUE_NUM, &gcd));

        sal_memset(&queue_sched, 0, sizeof(queue_sched));   
        queue_sched.type = CTC_QOS_SCHED_QUEUE;    
        queue_sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
        queue_sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_WEIGHT;
        queue_sched.sched.queue_sched.queue.gport = req->port;
        for (qid = 0; qid < HAGT_QOS_PORT_QUEUE_NUM; qid++)    
        {
            weight = *wdrr_weight[qid] / gcd;    
            queue_sched.sched.queue_sched.exceed_weight = weight;
            queue_sched.sched.queue_sched.queue.queue_id = qid;
            HAGT_IF_ERROR_RETURN(ctc_qos_set_sched(&queue_sched));        
        }
    }

    sal_memset(&queue_sched, 0, sizeof(queue_sched));      
    queue_sched.type = CTC_QOS_SCHED_QUEUE;
    queue_sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
    queue_sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;    
    queue_sched.sched.queue_sched.queue.gport = req->port;
    for (qid = 0; qid < HAGT_QOS_PORT_QUEUE_NUM; qid++)    
    {         
        queue_sched.sched.queue_sched.exceed_class = *queue_class[qid];
        queue_sched.sched.queue_sched.queue.queue_id = qid;
        HAGT_IF_ERROR_RETURN(ctc_qos_set_sched(&queue_sched));        
    }

    /* 7. queue drop */
    sal_memset(&ctc_drop, 0, sizeof(ctc_drop));
    ctc_drop.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    ctc_drop.queue.gport = req->port;
    for (qid = 0; qid < req->queuenum; qid++)
    {
        HAGT_IF_ERROR_RETURN(_hagt_qos_map_queue_drop(pp_drop_array[qid], &ctc_drop.drop));
        ctc_drop.queue.queue_id = qid;    
        HAGT_IF_ERROR_RETURN(ctc_qos_set_drop_scheme(&ctc_drop));
    }


    sal_memset(&ctc_shape, 0, sizeof(ctc_shape));
    ctc_shape.type = CTC_QOS_SHAPE_QUEUE;
    ctc_shape.shape.queue_shape.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    ctc_shape.shape.queue_shape.queue.gport = req->port;
    ctc_shape.shape.queue_shape.enable = TRUE;
    for (qid = 0; qid < req->queuenum; qid++)
    {
        if (req->qosenable && pp_queue_shape_array[qid]->cir != -1)
        {
            ctc_shape.shape.queue_shape.queue.queue_id = qid;
            HAGT_IF_ERROR_RETURN(_hagt_qos_map_queue_shape(pp_queue_shape_array[qid], &ctc_shape.shape.queue_shape));
            HAGT_IF_ERROR_RETURN(ctc_qos_set_shape(&ctc_shape));     
           
        }
        
    }
    
    if (req->qosenable)
    {
        /*8. set queue confirm class to 7, modify by chenxw for bug 22441, 2013-05-14.*/
        sal_memset(&queue_sched, 0, sizeof(queue_sched));
        queue_sched.type = CTC_QOS_SCHED_QUEUE;
        queue_sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
        queue_sched.sched.queue_sched.confirm_class = 7;
        queue_sched.sched.queue_sched.queue.gport = req->port;
        queue_sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;

        for (qid = 0; qid < HAGT_QOS_PORT_QUEUE_NUM; qid++)
        {
            queue_sched.sched.queue_sched.queue.queue_id = qid;
            HAGT_IF_ERROR_RETURN(ctc_qos_set_sched(&queue_sched));
        }

        /*9. group class, added by chenxw for bug22441, 2013-05-14.*/    
        sal_memset(&queue_sched, 0, sizeof(queue_sched));
        queue_sched.type = CTC_QOS_SCHED_GROUP;
        queue_sched.sched.group_sched.class_priority = 3;
        queue_sched.sched.group_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
        queue_sched.sched.group_sched.gport = req->port;
        queue_sched.sched.group_sched.group_type = CTC_QOS_SCHED_GROUP_PORT;
 
        for (group_class = 0; group_class < 8; group_class++)
        {
            queue_sched.sched.group_sched.queue_class = group_class;
            HAGT_IF_ERROR_RETURN(ctc_qos_set_sched(&queue_sched));
        }
    }
    


    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_deinit_qos_port_cfg
 * Purpose      : De-initialize QoS port config
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_deinit_qos_port_cfg(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosDeinitPortCfgReq_t *req;
    uint8 qid;
    ctc_qos_drop_t ctc_drop;
    ctc_qos_shape_t ctc_shape = { 0 };
    ctc_qos_policer_t ctc_policer = { 0 };
    ctc_qos_sched_t queue_sched = { 0 };
    uint8  queue_class;      
    
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_PROCESS_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosDeinitPortCfg;
    if (!hagt_chip_port_is_local(req->port))
    {
        return HAGT_E_NONE;
    }
    /* 2. check flag to deinit qos port cfg */
    if (GLB_FLAG_ISSET(req->flag, GLB_QOS_PORT_CFG_FLAG_DOMAIN))
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->port, CTC_PORT_DIR_PROP_QOS_DOMAIN, 
                                CTC_INGRESS, req->domain));
        HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->port, CTC_PORT_DIR_PROP_QOS_DOMAIN, 
                                CTC_EGRESS, req->domain));
    }
    if (GLB_FLAG_ISSET(req->flag, GLB_QOS_PORT_CFG_FLAG_TRUST))
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_QOS_POLICY, req->trust));
    }
    if (GLB_FLAG_ISSET(req->flag, GLB_QOS_PORT_CFG_FLAG_COS))
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->port, CTC_PORT_PROP_DEFAULT_PCP, req->cos));
    }
    if (GLB_FLAG_ISSET(req->flag, GLB_QOS_PORT_CFG_FLAG_REPLACE_COS))
    {
        HAGT_IF_ERROR_RETURN(
            ctc_port_set_property(req->port, CTC_PORT_PROP_REPLACE_STAG_COS, FALSE));
    }
    if (GLB_FLAG_ISSET(req->flag, GLB_QOS_PORT_CFG_FLAG_REPLACE_DSCP))
    {
        HAGT_IF_ERROR_RETURN(
            ctc_port_set_property(req->port, CTC_PORT_PROP_REPLACE_DSCP_EN, FALSE));
    }
    if (GLB_FLAG_ISSET(req->flag, GLB_QOS_PORT_CFG_FLAG_IGS_PORT_POLICER))
    {
        ctc_policer.dir = CTC_INGRESS;
        ctc_policer.type = CTC_QOS_POLICER_TYPE_PORT;
        ctc_policer.id.policer_id = req->port;
        ctc_policer.enable = FALSE;

        HAGT_IF_ERROR_RETURN(ctc_qos_set_policer(&ctc_policer));
    }
    if (GLB_FLAG_ISSET(req->flag, GLB_QOS_PORT_CFG_FLAG_EGS_PORT_POLICER))
    {
        ctc_policer.dir = CTC_EGRESS;
        ctc_policer.type = CTC_QOS_POLICER_TYPE_PORT;
        ctc_policer.id.policer_id = req->port;
        ctc_policer.enable = FALSE;
    
        HAGT_IF_ERROR_RETURN(ctc_qos_set_policer(&ctc_policer));
    }
    if (GLB_FLAG_ISSET(req->flag, GLB_QOS_PORT_CFG_FLAG_PORT_SHAPE))
    {
        ctc_shape.type = CTC_QOS_SHAPE_PORT;
        ctc_shape.shape.port_shape.gport = req->port;

        ctc_shape.shape.port_shape.enable = FALSE;

        /* set default shape for the port */
        HAGT_IF_ERROR_RETURN(ctc_qos_set_shape(&ctc_shape));
    }

    sal_memset(&ctc_drop, 0, sizeof(ctc_qos_drop_t));
    if (GLB_FLAG_ISSET(req->flag, GLB_QOS_PORT_CFG_FLAG_QUEUE_DROP))
    {
        HAGT_IF_ERROR_RETURN(_hagt_qos_map_queue_drop(&req->queueDrop, &ctc_drop.drop));
    }

    for (qid = 0; qid < HAGT_QOS_PORT_QUEUE_NUM; qid++)
    {
        if (GLB_FLAG_ISSET(req->flag, GLB_QOS_PORT_CFG_FLAG_QUEUE_DROP) && 
            GLB_IS_BIT_SET(req->queueDropBmp, qid))
        {
            ctc_drop.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
            ctc_drop.queue.gport = req->port;
            ctc_drop.queue.queue_id = qid;
            HAGT_IF_ERROR_RETURN(ctc_qos_set_drop_scheme(&ctc_drop));
        }

        if (GLB_FLAG_ISSET(req->flag, GLB_QOS_PORT_CFG_FLAG_QUEUE_CLASS) && 
            GLB_IS_BIT_SET(req->queueClassBmp, qid))
        {
            queue_sched.type = CTC_QOS_SCHED_QUEUE;
            queue_sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
            queue_sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
            // TODO: use ((req->queueClassId >> qid) & 0x7) ???
            queue_sched.sched.queue_sched.exceed_class = ((req->queueClassId >> qid) & 0x7);
            /*modify by chenxw for bug22441, 2013-05-27.*/
            queue_sched.sched.queue_sched.queue.queue_id = qid;
            queue_sched.sched.queue_sched.queue.gport = req->port;
            HAGT_IF_ERROR_RETURN(ctc_qos_set_sched(&queue_sched));
        }

        if (GLB_FLAG_ISSET(req->flag, GLB_QOS_PORT_CFG_FLAG_WDRR_WEIGHT) && 
            GLB_IS_BIT_SET(req->wdrrWeightBmp, qid))
        {
            sal_memset (&queue_sched, 0 ,sizeof(queue_sched));
            queue_sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
            queue_sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_WEIGHT;
            queue_sched.sched.queue_sched.queue.queue_id = qid;
            /*modify by chenxw for bug22441, 2013-05-27.*/
            queue_sched.sched.queue_sched.queue.gport= req->port;
            queue_sched.type = CTC_QOS_SCHED_QUEUE;
            // TODO: use req->wdrrWeight directly ???
            queue_sched.sched.queue_sched.exceed_weight = req->wdrrWeight;

            HAGT_IF_ERROR_RETURN(ctc_qos_set_sched(&queue_sched));
        }

        if (GLB_FLAG_ISSET(req->flag, GLB_QOS_PORT_CFG_FLAG_QUEUE_SHAPE) && 
            GLB_IS_BIT_SET(req->queueShapeBmp, qid))
        {
            sal_memset (&ctc_shape, 0 ,sizeof(ctc_shape));
            
            ctc_shape.type = CTC_QOS_SHAPE_QUEUE;
            ctc_shape.shape.queue_shape.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
            ctc_shape.shape.queue_shape.queue.queue_id = qid;
            ctc_shape.shape.queue_shape.queue.gport = req->port;
            ctc_shape.shape.queue_shape.enable = FALSE;
        
            HAGT_IF_ERROR_RETURN(ctc_qos_set_shape(&ctc_shape));
        }
    }

    /*8. set queue confirm class to 7, modify by chenxw for bug 22441, 2013-05-14.*/
    sal_memset(&queue_sched, 0, sizeof(queue_sched));
    queue_sched.type = CTC_QOS_SCHED_QUEUE;
    queue_sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
    queue_sched.sched.queue_sched.queue.gport = req->port;
    queue_sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;

    for (qid = 0; qid < HAGT_QOS_PORT_QUEUE_NUM; qid++)
    {
        queue_sched.sched.queue_sched.queue.queue_id = qid;
        queue_sched.sched.queue_sched.confirm_class = qid;        
        HAGT_IF_ERROR_RETURN(ctc_qos_set_sched(&queue_sched));
    }

    /*9. group class, added by chenxw for bug22441, 2013-05-14.*/    
    sal_memset(&queue_sched, 0, sizeof(queue_sched));
    queue_sched.type = CTC_QOS_SCHED_GROUP;
    queue_sched.sched.group_sched.class_priority = 3;
    queue_sched.sched.group_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
    queue_sched.sched.group_sched.gport = req->port;
    queue_sched.sched.group_sched.group_type = CTC_QOS_SCHED_GROUP_PORT;
 
    for (queue_class = 0; queue_class < 8; queue_class++)
    {
        queue_sched.sched.group_sched.queue_class = queue_class/2;
        HAGT_IF_ERROR_RETURN(ctc_qos_set_sched(&queue_sched));
    }    
    
    return HAGT_E_NONE;
}

 /* ~~~~~~~~~~~~~~~~~~~~~ stats ~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/****************************************************************************************
 * Name         : _hagt_qos_get_flow_policer_stats
 * Purpose      : Get flow policer statistics that collected on the specified interface
 * Input        : p_msg_data    - message data from hal to hsrv
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_get_flow_policer_stats(HalMsgStatsKey_t* p_stats_key, glb_stats_result_t* p_stats)
{
    HalMsgFlowPolicerKey_t *p_plc_key;
    ctc_qos_policer_stats_t policer_stats = { 0 };

    HAGT_QOS_PTR_CHECK(p_stats_key);
    HAGT_QOS_PTR_CHECK(p_stats);

    HAGT_QOS_POLICER_DEBUG_FUNC();

    p_plc_key = &p_stats_key->flowPlcKey;

    HAGT_QOS_POLICER_DEBUG_INFO("get flow policer stats, policer_id = %ld\n", 
                                p_plc_key->policerId);

    /* 1. get stats */
    /* for flow policer, policer is determinated by policer_id, so we set flow policer dir as BOTH */
    /* in _sys_greatbelt_qos_policer_policer_id_check(), BOTH is invalid, so we set dir as CTC_INGRESS*/        
    policer_stats.dir = CTC_INGRESS;
    policer_stats.type = CTC_QOS_POLICER_TYPE_FLOW;
    policer_stats.id.policer_id = p_plc_key->policerId;
    
    HAGT_IF_ERROR_RETURN(ctc_qos_query_policer_stats(&policer_stats));

    /* 2. output stats */
    p_stats->policer.confirm_stats.packet_count = policer_stats.stats.confirm_pkts;
    p_stats->policer.confirm_stats.byte_count   = policer_stats.stats.confirm_bytes;
    p_stats->policer.exceed_stats.packet_count  = policer_stats.stats.exceed_pkts;
    p_stats->policer.exceed_stats.byte_count    = policer_stats.stats.exceed_bytes;
    p_stats->policer.violate_stats.packet_count = policer_stats.stats.violate_pkts;
    p_stats->policer.violate_stats.byte_count   = policer_stats.stats.violate_bytes;
    p_stats->policer.support_flag               = g_pst_hagt_qos_master->policer_stats_support_flag;

    HAGT_QOS_POLICER_DEBUG_INFO("flow policer stats result:\n");
    HAGT_QOS_POLICER_DEBUG_INFO("--------------------------------------\n");
    HAGT_QOS_POLICER_DEBUG_INFO("confirm stats: pkts (%llu), bytes (%llu)\n", 
                                policer_stats.stats.confirm_pkts, policer_stats.stats.confirm_bytes);
    HAGT_QOS_POLICER_DEBUG_INFO("exceed stats : pkts (%llu), bytes (%llu)\n", 
                                policer_stats.stats.exceed_pkts, policer_stats.stats.exceed_bytes);
    HAGT_QOS_POLICER_DEBUG_INFO("violate stats: pkts (%llu), bytes (%llu)\n", 
                                policer_stats.stats.violate_pkts, policer_stats.stats.violate_bytes);
    
    return HAGT_E_NONE;
}


/****************************************************************************************
 * Name         : _hagt_qos_clear_flow_policer_stats
 * Purpose      : Clear flow policer statistics on the specified interface
 * Input        : p_msg_data    - message data from hal to hsrv
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_clear_flow_policer_stats(HalMsgStatsKey_t* p_stats_key)
{
    HalMsgFlowPolicerKey_t *p_plc_key;
    ctc_qos_policer_stats_t policer_stats = { 0 };
    
    HAGT_QOS_PTR_CHECK(p_stats_key);

    HAGT_QOS_POLICER_DEBUG_FUNC();

    p_plc_key = &p_stats_key->flowPlcKey;
    /* for flow policer, policer is determinated by policer_id, so we set flow policer dir as BOTH */
    /* in _sys_greatbelt_qos_policer_policer_id_check(), BOTH is invalid, so we set dir as CTC_INGRESS*/    
    policer_stats.dir = CTC_INGRESS;
    policer_stats.type = CTC_QOS_POLICER_TYPE_FLOW;
    policer_stats.id.policer_id = p_plc_key->policerId;

    
    HAGT_QOS_POLICER_DEBUG_INFO("clear flow policer stats, policer_id = %ld\n", 
                                p_plc_key->policerId);

    /* 1. clear stats */
    ctc_qos_clear_policer_stats(&policer_stats);
    
    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_get_port_policer_stats
 * Purpose      : Get port policer statistics
 * Input        : p_stats_key   - pointer to stats key
 * Output       : p_stats       - pointer to port policer stats
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_get_port_policer_stats(HalMsgStatsKey_t* p_stats_key, glb_stats_result_t* p_stats)
{
    HalMsgPortPolicerKey_t *p_plc_key;
    ctc_qos_policer_stats_t policer_stats = { 0 };
    
    HAGT_QOS_PTR_CHECK(p_stats_key);
    HAGT_QOS_PTR_CHECK(p_stats);

    HAGT_QOS_POLICER_DEBUG_FUNC();

    p_plc_key = &p_stats_key->portPlcKey;

    HAGT_QOS_POLICER_DEBUG_INFO("get port policer stats, port = 0x%lx, dir = %ld\n", 
                                p_plc_key->port, p_plc_key->dir);

    /* 1. get stats */

    policer_stats.dir = p_plc_key->dir;
    policer_stats.type =CTC_QOS_POLICER_TYPE_PORT;
    policer_stats.id.policer_id = p_plc_key->port;
    
    HAGT_IF_ERROR_RETURN(ctc_qos_query_policer_stats(&policer_stats));

    /* 2. output stats */
    p_stats->policer.confirm_stats.packet_count = policer_stats.stats.confirm_pkts;
    p_stats->policer.confirm_stats.byte_count   = policer_stats.stats.confirm_bytes;
    p_stats->policer.exceed_stats.packet_count  = policer_stats.stats.exceed_pkts;
    p_stats->policer.exceed_stats.byte_count    = policer_stats.stats.exceed_bytes;
    p_stats->policer.violate_stats.packet_count = policer_stats.stats.violate_pkts;
    p_stats->policer.violate_stats.byte_count   = policer_stats.stats.violate_bytes;
    p_stats->policer.support_flag               = g_pst_hagt_qos_master->policer_stats_support_flag;

    HAGT_QOS_POLICER_DEBUG_INFO("port policer stats result:\n");
    HAGT_QOS_POLICER_DEBUG_INFO("--------------------------------------\n");
    HAGT_QOS_POLICER_DEBUG_INFO("confirm stats: pkts (%llu), bytes (%llu)\n", 
                                policer_stats.stats.confirm_pkts, policer_stats.stats.confirm_bytes);
    HAGT_QOS_POLICER_DEBUG_INFO("exceed stats : pkts (%llu), bytes (%llu)\n", 
                                policer_stats.stats.exceed_pkts, policer_stats.stats.exceed_bytes);
    HAGT_QOS_POLICER_DEBUG_INFO("violate stats: pkts (%llu), bytes (%llu)\n", 
                                policer_stats.stats.violate_pkts, policer_stats.stats.violate_bytes);
    
    return HAGT_E_NONE;
}


/****************************************************************************************
 * Name         : _hagt_qos_clear_port_policer_stats
 * Purpose      : Clear port policer statistics
 * Input        : p_msg_data    - message data from hal to hsrv
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_clear_port_policer_stats(HalMsgStatsKey_t* p_stats_key)
{
    HalMsgPortPolicerKey_t *p_plc_key;
    ctc_qos_policer_stats_t policer_stats = { 0 };
    
    HAGT_QOS_PTR_CHECK(p_stats_key);

    HAGT_QOS_POLICER_DEBUG_FUNC();

    p_plc_key = &p_stats_key->portPlcKey;

    HAGT_QOS_POLICER_DEBUG_INFO("clear port policer stats, port = 0x%lx, dir = %ld\n", 
                                p_plc_key->port, p_plc_key->dir);

    policer_stats.type = CTC_QOS_POLICER_TYPE_PORT;
    policer_stats.dir = p_plc_key->dir;
    policer_stats.id.policer_id = p_plc_key->port;
    
    /* 1. clear stats */
    HAGT_IF_ERROR_RETURN(
        ctc_qos_clear_policer_stats(&policer_stats));    
    
    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_get_port_queue_stats
 * Purpose      : Get port queue statistics
 * Input        : p_msg_data    - message data from hal to hsrv
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_get_port_queue_stats(HalMsgStatsKey_t* p_stats_key, glb_stats_result_t* p_stats)
{
    HalMsgPortQueueKey_t *p_que_key;
    ctc_qos_queue_stats_t queue_stats;
    
    HAGT_QOS_PTR_CHECK(p_stats_key);
    HAGT_QOS_PTR_CHECK(p_stats);

    HAGT_QOS_QUEUE_DEBUG_FUNC();

    p_que_key = &p_stats_key->portQueueKey;

    HAGT_QOS_POLICER_DEBUG_INFO("get port queue stats, port = 0x%lx, qid = %ld\n", 
                                p_que_key->port, p_que_key->qid);

    sal_memset(&queue_stats, 0, sizeof(ctc_qos_queue_stats_t));

    /* 1. get stats */

    queue_stats.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    queue_stats.queue.queue_id = p_que_key->qid;
    queue_stats.queue.gport = p_que_key->port;
    
    HAGT_IF_ERROR_RETURN(ctc_qos_query_queue_stats(&queue_stats));

/*added by wangl for openflow queue stats*/
#if 0
    /* 2. output stats */
    p_stats->queue.deq_stats.packet_count  = queue_stats.stats.deq_packets;
    p_stats->queue.deq_stats.byte_count    = queue_stats.stats.deq_bytes;
    p_stats->queue.drop_stats.packet_count = queue_stats.stats.drop_packets;
    p_stats->queue.drop_stats.byte_count   = queue_stats.stats.drop_bytes;
    p_stats->queue.support_flag            = g_pst_hagt_qos_master->queue_stats_support_flag;

    HAGT_QOS_POLICER_DEBUG_INFO("port queue stats result:\n");
    HAGT_QOS_POLICER_DEBUG_INFO("--------------------------------------\n");
    HAGT_QOS_POLICER_DEBUG_INFO("deq stats : pkts (%llu), bytes (%llu)\n", 
                                queue_stats.stats.deq_packets, queue_stats.stats.deq_bytes);
    HAGT_QOS_POLICER_DEBUG_INFO("drop stats: pkts (%llu), bytes (%llu)\n", 
                                queue_stats.stats.drop_packets, queue_stats.stats.drop_bytes);
 #endif
    return HAGT_E_NONE;
}


/****************************************************************************************
 * Name         : _hagt_qos_clear_port_queue_stats
 * Purpose      : Clear port queue statistics
 * Input        : p_msg_data    - message data from hal to hsrv
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_clear_port_queue_stats(HalMsgStatsKey_t* p_stats_key)
{
    HalMsgPortQueueKey_t *p_que_key;
    uint8  qid = 0;
    ctc_qos_queue_stats_t queue_stats;

    HAGT_QOS_PTR_CHECK(p_stats_key);

    HAGT_QOS_QUEUE_DEBUG_FUNC();

    p_que_key = &p_stats_key->portQueueKey;

    HAGT_QOS_POLICER_DEBUG_INFO("clear port queue stats, port = 0x%lx, qid = %ld\n", 
                                p_que_key->port, p_que_key->qid);
    
    sal_memset(&queue_stats, 0, sizeof(ctc_qos_queue_stats_t));
    
    queue_stats.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    queue_stats.queue.queue_id = p_que_key->qid;
    queue_stats.queue.gport = p_que_key->port;

    /* 1. clear stats */
    if (p_que_key->qid == GLB_QOS_ALL_QUEUE_ID)
    {
        for (qid = 0; qid < HAGT_QOS_PORT_QUEUE_NUM; qid++)
        {
            queue_stats.queue.queue_id = qid;
            ctc_qos_clear_queue_stats(&queue_stats);
        }
    }
    else
    {
        ctc_qos_clear_queue_stats(&queue_stats);
    }
    
    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_map_flow_action 
 * Purpose      : Map flow action from hagt to ctc
 * Input        : p_hagt_action - pointer to hagt flow action
 * Output       : p_ctc_action  - pointer to ctc flow action
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_map_flow_action(HalMsgQosFlowAction_t* p_hagt_action, ctc_acl_action_t* p_ctc_action, uint32 label)
{
    HAGT_QOS_PTR_CHECK(p_hagt_action);
    HAGT_QOS_PTR_CHECK(p_ctc_action);
    uint32 fwd_nh_id = 0;

    HAGT_QOS_POLICER_DEBUG_FUNC();

    /* trust */
    if (GLB_FLAG_ISSET(p_hagt_action->flag, GLB_QOS_FLOW_ACTION_FLAG_TRUST))
    {
        GLB_SET_FLAG(p_ctc_action->flag, CTC_ACL_ACTION_FLAG_TRUST);
        p_ctc_action->trust = p_hagt_action->trust;

        HAGT_QOS_POLICER_DEBUG_INFO("flow action, trust = %d", p_ctc_action->trust);
    }

    /* priority and color */
    if (GLB_FLAG_ISSET(p_hagt_action->flag, GLB_QOS_FLOW_ACTION_FLAG_PRIORITY))
    {
        GLB_SET_FLAG(p_ctc_action->flag, CTC_ACL_ACTION_FLAG_PRIORITY_AND_COLOR);
        p_ctc_action->priority = p_hagt_action->priority;
        p_ctc_action->color    = p_hagt_action->color;

        HAGT_QOS_POLICER_DEBUG_INFO("flow action, priority = %d, color = %d", 
                                    p_ctc_action->priority, p_ctc_action->color);
    }

    /* flow policer */
    if (GLB_FLAG_ISSET(p_hagt_action->flag, GLB_QOS_FLOW_ACTION_FLAG_POLICER))
    {
        GLB_SET_FLAG(p_ctc_action->flag, CTC_ACL_ACTION_FLAG_MICRO_FLOW_POLICER);
        p_ctc_action->micro_policer_id = p_hagt_action->policerId;

        HAGT_QOS_POLICER_DEBUG_INFO("flow action, policer_id = %u", p_ctc_action->micro_policer_id);
    }
    
        /* agg flow policer */
    if (GLB_FLAG_ISSET(p_hagt_action->flag, GLB_QOS_FLOW_ACTION_AGG_POLICER))
    {
        GLB_SET_FLAG(p_ctc_action->flag, CTC_ACL_ACTION_FLAG_MACRO_FLOW_POLICER);
        p_ctc_action->macro_policer_id = p_hagt_action->agpId;

        HAGT_QOS_POLICER_DEBUG_INFO("flow action, policer_id = %u", p_ctc_action->macro_policer_id);
    }

    /* stats */
    if (GLB_FLAG_ISSET(p_hagt_action->flag, GLB_QOS_FLOW_ACTION_FLAG_STATS))
    {
        GLB_SET_FLAG(p_ctc_action->flag, CTC_ACL_ACTION_FLAG_STATS);
        GLB_QOS_ACL_GET_ENTRY_ID(p_hagt_action->statsId, label, p_ctc_action->stats_id);        
    }

    /* redirect*/
    if (GLB_FLAG_ISSET(p_hagt_action->flag, GLB_QOS_FLOW_ACTION_FLAG_REDIRECT))
    {
        GLB_SET_FLAG(p_ctc_action->flag, CTC_ACL_ACTION_FLAG_REDIRECT);
        /* Modified by yanxa, for bug 16738, 2011-10-27*/
        HAGT_IF_ERROR_RETURN(ctc_nh_get_l2uc(p_hagt_action->fwdGport, CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS, &fwd_nh_id));
        p_ctc_action->nh_id = fwd_nh_id;
    }

    /* redirect*/
    if (GLB_FLAG_ISSET(p_hagt_action->flag, GLB_QOS_FLOW_ACTION_TAP))
    {
        GLB_SET_FLAG(p_ctc_action->flag, CTC_ACL_ACTION_FLAG_REDIRECT);
        p_ctc_action->nh_id = p_hagt_action->nexthopId;

        if (0 != p_hagt_action->vlanid)
        {
            GLB_SET_FLAG(p_ctc_action->flag, CTC_ACL_ACTION_FLAG_VLAN_EDIT);
            p_ctc_action->vlan_edit.stag_op = CTC_ACL_VLAN_TAG_OP_ADD;
            p_ctc_action->vlan_edit.svid_sl = CTC_ACL_VLAN_TAG_SL_NEW;
            p_ctc_action->vlan_edit.svid_new = p_hagt_action->vlanid;
        }
    }

    /* mirror*/
    if (GLB_FLAG_ISSET(p_hagt_action->flag, GLB_QOS_FLOW_ACTION_MIRROR))
    {
        GLB_SET_FLAG(p_ctc_action->flag, CTC_ACL_ACTION_FLAG_RANDOM_LOG);
        p_ctc_action->log_session_id = p_hagt_action->nexthopId;
        p_ctc_action->log_percent = CTC_LOG_PERCENT_POWER_NEGATIVE_0;
    }

    /* log */
    if (GLB_FLAG_ISSET(p_hagt_action->flag, GLB_QOS_FLOW_ACTION_FLAG_LOG))
    {
        GLB_SET_FLAG(p_ctc_action->flag, CTC_ACL_ACTION_FLAG_RANDOM_LOG);
        p_ctc_action->log_session_id = p_hagt_action->logId;
        p_ctc_action->log_percent = p_hagt_action->logWeight;
    }
/* modified by cuixl for bug 22441, 2013-03-19 */
/*modification start*/
    if (GLB_FLAG_ISSET(p_hagt_action->flag, GLB_QOS_FLOW_ACTION_DOMAIN))
    {
        GLB_SET_FLAG(p_ctc_action->flag, CTC_ACL_ACTION_FLAG_QOS_DOMAIN);
        p_ctc_action->qos_domain = p_hagt_action->domain;
    }
    if (GLB_FLAG_ISSET(p_hagt_action->flag, GLB_QOS_FLOW_ACTION_DSCP))
    {
        GLB_SET_FLAG(p_ctc_action->flag, CTC_ACL_ACTION_FLAG_DSCP);
        p_ctc_action->dscp = p_hagt_action->dscp;
    }
    // TODO: need test this feature
    if (GLB_FLAG_ISSET(p_hagt_action->flag, GLB_QOS_FLOW_ACTION_STAG_COS))
    {
        GLB_SET_FLAG(p_ctc_action->flag, CTC_ACL_ACTION_FLAG_VLAN_EDIT);
        p_ctc_action->vlan_edit.stag_op = CTC_ACL_VLAN_TAG_OP_REP;
        p_ctc_action->vlan_edit.scos_sl = CTC_ACL_VLAN_TAG_SL_NEW;
        p_ctc_action->vlan_edit.scos_new = p_hagt_action->cos;
    }
    if (GLB_FLAG_ISSET(p_hagt_action->flag, GLB_QOS_FLOW_ACTION_CTAG_COS))
    {
        GLB_SET_FLAG(p_ctc_action->flag, CTC_ACL_ACTION_FLAG_VLAN_EDIT);
        p_ctc_action->vlan_edit.ctag_op = CTC_ACL_VLAN_TAG_OP_REP;
        p_ctc_action->vlan_edit.ccos_sl = CTC_ACL_VLAN_TAG_SL_NEW;
        p_ctc_action->vlan_edit.ccos_new = p_hagt_action->cos;
    }
    
/*modification end*/
    
    return HAGT_E_NONE;
}

static int32
hagt_qos_acl_add_stats(uint32 stats_id)
{
/* modified by cuixl for bug 27208, 2014-04-01 */
/* modification start */
/* Ingress ACL and Egress ACL used different stats ptr in GG,
   so, we need specify ACL directy, otherwise errors would be occur*/
    ctc_stats_statsid_t statsid;
    ctc_acl_group_info_t group_info = { 0 };
    uint8 group_id = 0;
    
    GLB_QOS_ACL_GET_GROUP_ID(stats_id, group_id);

    HAGT_IF_ERROR_RETURN(ctc_acl_get_group_info(group_id, &group_info));
    
    sal_memset(&statsid, 0x0, sizeof(ctc_stats_statsid_t));
    statsid.type = CTC_STATS_STATSID_TYPE_ACL;
    statsid.dir = group_info.dir;
    statsid.stats_id = stats_id;     
    statsid.statsid.acl_priority = group_info.priority; 
    HAGT_IF_ERROR_RETURN(ctc_stats_create_statsid(&statsid));  

/* modification end */

    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_update_flow_action 
 * Purpose      : Update flow action
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_update_flow_action(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosUpdateFlowActionReq_t *req;
    HalMsgQosFlowEntryId_t **pp_array;
    int32  index, count = 0;
    ctc_acl_action_t ctc_action;
    uint32 entry_id;
    ctc_stats_statsid_t statsid;
 

    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_POLICER_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosUpdateFlowAction;
    pp_array = req->updateEntryIdList.list.array;
    count    = req->updateEntryIdList.list.count;

    /* 2. invoke flow action op callback */
    for (index = 0; index < count; index++)
    {
        sal_memset(&ctc_action, 0, sizeof(ctc_acl_action_t));
        sal_memset(&statsid, 0x0, sizeof(ctc_stats_statsid_t));
        req->action.statsId = pp_array[index]->statsId;
        _hagt_qos_map_flow_action(&req->action, &ctc_action, req->label);
        
        if(GLB_QOS_ACL_STATS_UPDATE_DEL == req->action.delStats)
        {
            /*added by ychen for bug 25350 in 2013-10-17*/
            GLB_QOS_ACL_GET_ENTRY_ID(req->action.statsId, req->label, statsid.stats_id);
            HAGT_IF_ERROR_RETURN(ctc_stats_destroy_statsid(statsid.stats_id));
        }
        else if(GLB_QOS_ACL_STATS_UPDATE_ADD == req->action.delStats)
        {
            hagt_qos_acl_add_stats(ctc_action.stats_id);
        }
        
        GLB_QOS_ACL_GET_ENTRY_ID(pp_array[index]->ruleId, req->label, entry_id);
        if(pp_array[index]->discard)
        {
            GLB_UNSET_FLAG(ctc_action.flag, CTC_ACL_ACTION_FLAG_RANDOM_LOG);
            GLB_SET_FLAG(ctc_action.flag, CTC_ACL_ACTION_FLAG_DISCARD);
/* modified by cuixl for bug 27208, 2014-04-01 */
/* modification start */
/* Egress ACL doesn't support deny leaning and SDK would return error code*/
            ctc_acl_group_info_t group_info = { 0 };
            HAGT_IF_ERROR_RETURN(ctc_acl_get_group_info(req->label, &group_info));
            if (group_info.dir == CTC_INGRESS)
            {            
                GLB_SET_FLAG(ctc_action.flag, CTC_ACL_ACTION_FLAG_DENY_LEARNING);
            }
/* modification end */
            
        }
        HAGT_IF_ERROR_RETURN(ctc_acl_update_action(entry_id, &ctc_action));

        if (GLB_FLOW_RULE_TYPE_MAC == pp_array[index]->ruleType)
        {
            ctc_acl_update_action(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV6, &ctc_action);
            ctc_acl_update_action(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_MPLS, &ctc_action);
            ctc_acl_update_action(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV4, &ctc_action);
        }

        switch(req->opAction)
        {
            case GLB_QOS_FLOW_ACTION_OP_SET_PRI_COLOR:
                 HAGT_QOS_POLICER_DEBUG_INFO("set flow pri + color, label = %lu, entry_id = %u, "
                     "rule_type = %lu, prioiry = %lu, color = %lu", req->label, entry_id, 
                      pp_array[index]->ruleType, req->action.priority, req->action.color);
                 break;
            case GLB_QOS_FLOW_ACTION_OP_UNSET_PRI_COLOR:
                HAGT_QOS_POLICER_DEBUG_INFO("unset flow pri + color, label = %lu, entry_id = %u, rule_type = %lu", 
                                req->label, entry_id, pp_array[index]->ruleType);
                 break;
            case GLB_QOS_FLOW_ACTION_OP_SET_TRUST:
                HAGT_QOS_POLICER_DEBUG_INFO("set flow trust, label = %lu, entry_id = %u, "
                    "rule_type = %lu, trust = %lu", req->label, entry_id, 
                    pp_array[index]->ruleType, req->action.trust);
                 break;
            case GLB_QOS_FLOW_ACTION_OP_UNSET_TRUST:
                HAGT_QOS_POLICER_DEBUG_INFO("unset flow trust, label = %lu, entry_id = %u, "
                    "rule_type = %lu", req->label, entry_id, pp_array[index]->ruleType);
                break;
            case GLB_QOS_FLOW_ACTION_OP_ADD_IND_POLICER:
                HAGT_QOS_POLICER_DEBUG_INFO("add flow policer, label = %lu, entry_id = %u, "
                    "rule_type = %lu, policer_id = %lu", req->label, entry_id, 
                    pp_array[index]->ruleType, req->action.policerId);
                break;
            case GLB_QOS_FLOW_ACTION_OP_DEL_IND_POLICER:
                HAGT_QOS_POLICER_DEBUG_INFO("del flow policer, label = %lu, entry_id = %u, "
                    "rule_type = %lu", req->label, entry_id, pp_array[index]->ruleType);
                break;
            case GLB_QOS_FLOW_ACTION_OP_SET_LOG:
                HAGT_QOS_POLICER_DEBUG_INFO("set flow monitor, label = %lu, entry_id = %u, "
                    "rule_type = %lu, log_id = %lu, log_weight= %lu", req->label, entry_id, 
                    pp_array[index]->ruleType, req->action.logId, req->action.logWeight);
               break;
            case GLB_QOS_FLOW_ACTION_OP_UNSET_LOG:
                HAGT_QOS_POLICER_DEBUG_INFO("unset flow monitor, label = %lu, entry_id = %u, "
                    "rule_type = %lu,", req->label, entry_id, 
                    pp_array[index]->ruleType);
               break;
            case GLB_QOS_FLOW_ACTION_OP_ENABLE_STATS:
                HAGT_QOS_POLICER_DEBUG_INFO("enable flow statistics, label = %lu, entry_id = %u, "
                    "rule_type = %lu", req->label, entry_id, 
                    pp_array[index]->ruleType);
               break;
            case GLB_QOS_FLOW_ACTION_OP_DISABLE_STATS:
                HAGT_QOS_POLICER_DEBUG_INFO("disable flow statistics, label = %lu, entry_id = %u, "
                    "rule_type = %lu", req->label, entry_id, 
                    pp_array[index]->ruleType);
               break;
            case GLB_QOS_FLOW_ACTION_OP_SET_REDIRECT:
                HAGT_QOS_POLICER_DEBUG_INFO("set flow redirect, label = %lu, entry_id = %u, "
                    "rule_type = %lu, redirect to gport = %lu", req->label, entry_id, 
                    pp_array[index]->ruleType, req->action.fwdGport);
               break;
            case GLB_QOS_FLOW_ACTION_OP_UNSET_REDIRECT:
                HAGT_QOS_POLICER_DEBUG_INFO("unset flow redirect, label = %lu, entry_id = %u, "
                    "rule_type = %lu", req->label, entry_id, pp_array[index]->ruleType);
               break; 
/* modified by cuixl for bug 22441, 2013-03-15 */
/*modification start*/
            case GLB_QOS_FLOW_ACTION_OP_SET_DOMAIN:
                HAGT_QOS_POLICER_DEBUG_INFO("set flow domain, label = %lu, entry_id = %u, "
                    "rule_type = %lu, domain = %lu", req->label, entry_id, 
                    pp_array[index]->ruleType, req->action.domain);
                 break;
            case GLB_QOS_FLOW_ACTION_OP_UNSET_DOMAIN:
                HAGT_QOS_POLICER_DEBUG_INFO("unset flow domain, label = %lu, entry_id = %u, "
                    "rule_type = %lu", req->label, entry_id, pp_array[index]->ruleType);
                break;
            case GLB_QOS_FLOW_ACTION_OP_SET_DSCP:
                HAGT_QOS_POLICER_DEBUG_INFO("set flow dscp, label = %lu, entry_id = %u, "
                    "rule_type = %lu, dscp = %lu", req->label, entry_id, 
                    pp_array[index]->ruleType, req->action.dscp);
                 break;
            case GLB_QOS_FLOW_ACTION_OP_UNSET_DSCP:
                HAGT_QOS_POLICER_DEBUG_INFO("unset flow dscp, label = %lu, entry_id = %u, "
                    "rule_type = %lu", req->label, entry_id, pp_array[index]->ruleType);
                break;
            case GLB_QOS_FLOW_ACTION_OP_SET_STAG_COS:
                HAGT_QOS_POLICER_DEBUG_INFO("set flow stag-cos, label = %lu, entry_id = %u, "
                    "rule_type = %lu, cos = %lu", req->label, entry_id, 
                    pp_array[index]->ruleType, req->action.cos);
                 break;
            case GLB_QOS_FLOW_ACTION_OP_SET_CTAG_COS:
                HAGT_QOS_POLICER_DEBUG_INFO("set flow ctag-cos, label = %lu, entry_id = %u, "
                    "rule_type = %lu, cos = %lu", req->label, entry_id, 
                    pp_array[index]->ruleType, req->action.cos);
                 break;
            case GLB_QOS_FLOW_ACTION_OP_UNSET_COS:
                HAGT_QOS_POLICER_DEBUG_INFO("unset flow cos, label = %lu, entry_id = %u, "
                    "rule_type = %lu", req->label, entry_id, pp_array[index]->ruleType);
                 break;                                
            case GLB_QOS_FLOW_ACTION_OP_ADD_AGG_POLICER:
                HAGT_QOS_POLICER_DEBUG_INFO("add aggregate flow policer, label = %lu, entry_id = %u, "
                    "rule_type = %lu, policer_id = %lu", req->label, entry_id, 
                    pp_array[index]->ruleType, req->action.policerId);
                break;
            case GLB_QOS_FLOW_ACTION_OP_DEL_AGG_POLICER:
                HAGT_QOS_POLICER_DEBUG_INFO("del aggregate flow policer, label = %lu, entry_id = %u, "
                    "rule_type = %lu", req->label, entry_id, pp_array[index]->ruleType);
                break;

/*modification end*/
               
            default:

               return HAGT_E_INVALID_PARAM;
        }

    }
    
    return HAGT_E_NONE;
}


/****************************************************************************************
 * Name         : _hagt_qos_label_op 
 * Purpose      : QoS label operation: create/remove/apply/unapply
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_label_op(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosLabelOpReq_t *req;
    ctc_acl_group_info_t group_info;
    int32 lchip;
    
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_POLICER_DEBUG_FUNC();

    sal_memset(&group_info, 0, sizeof(ctc_acl_group_info_t));

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosLabelOp;
    group_info.dir = req->dir;
    switch (req->opCode)
    {
        /* apply qos label to gport */
        case HalMsgQosLabelOp_apply:
            HAGT_QOS_POLICER_DEBUG_INFO("apply qos label, %s = %ld, dir = %ld, label = %lu", 
                                        req->isVlan ? "vlan-id" : "gport",
                                        req->gportOrVlanId, req->dir, req->label);
#if 0
            /*for h qos policer label*/
            if(req->logicPort)
            {
                group_info.type = CTC_ACL_GROUP_TYPE_SERVICE_ACL;
                group_info.un.service_id = req->logicPort;
                group_info.lchip = 0;
                group_info.priority = GLB_QOS_ACL_TCAM_BLOCK;
                HAGT_IF_ERROR_RETURN(ctc_acl_create_group(req->label, &group_info));
                HAGT_IF_ERROR_RETURN(ctc_acl_install_group(req->label, &group_info));
            }
            else
#endif
            {
            if(req->isVlan)
            { 
/* modified by cuixl for bug 27208, 2014-04-02 */
/* modification start */     
/* use acl lookup1 as vlan acl lookup */
                HAGT_IF_ERROR_RETURN(ctc_vlan_set_direction_property(req->gportOrVlanId, CTC_VLAN_DIR_PROP_ACL_EN, req->dir, (CTC_INGRESS == req->dir) ? CTC_ACL_EN_1 : CTC_ACL_EN_0));
                HAGT_IF_ERROR_RETURN(ctc_vlan_set_direction_property(req->gportOrVlanId, (CTC_INGRESS == req->dir) ? CTC_VLAN_DIR_PROP_ACL_CLASSID_1 : CTC_VLAN_DIR_PROP_ACL_CLASSID, req->dir, req->label));
                HAGT_IF_ERROR_RETURN(ctc_vlan_set_direction_property(req->gportOrVlanId, (CTC_INGRESS == req->dir) ? CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_1 : CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, req->dir, CTC_ACL_TCAM_LKUP_TYPE_L2_L3));
/* modification end */                
                group_info.type = CTC_ACL_GROUP_TYPE_VLAN_CLASS;
                group_info.un.vlan_class_id = req->label;
                group_info.lchip = 0; // TODO:  merge sdk 2.1.13
            }
            else
            {
            /* modified by cuixl for bug 27208, 2014-03-28 */
/* modification start */
/* for egress acl, only block 0 is supported on GG*/
                //HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->gportOrVlanId, CTC_PORT_DIR_PROP_ACL_EN, req->dir, CTC_ACL_EN_0));                
                HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->gportOrVlanId, CTC_PORT_DIR_PROP_ACL_CLASSID, req->dir, req->label));
/* we set aclLookup0 as CTC_ACL_TCAM_LKUP_TYPE_L2_L3*/
                HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->gportOrVlanId, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, req->dir, CTC_ACL_TCAM_LKUP_TYPE_L2_L3));
/* modification end */
                
                group_info.type = CTC_ACL_GROUP_TYPE_PORT_CLASS;
                group_info.un.port_class_id = req->label;
                lchip = hagt_chip_glb2local(req->gchip); // TODO:  merge sdk 2.1.13
                if (lchip < 0)
                {
                    return HAGT_E_NONE;
                }
                
                group_info.lchip = lchip;
                /*added by ychen for bug 25183 in 2013-10-12*/
                HAGT_IF_ERROR_RETURN (hagt_ifdb_set_port_flag(req->gportOrVlanId, HAGT_IF_PORT_FLAG_ACL_APPLY, 1));
            }
           if(req->isFirstMember)
           {
            /*modify by chenxw for qos res, bug24095, 2013-10-12.*/
            /*use bigger block to install acl entry*/
/* modified by cuixl for bug 27208, 2014-04-02 */
/* modification start */
/* aclLookup0 is used by portACL; aclLookup1 is used by VlanACL;*/
            if (CTC_INGRESS == req->dir)
            {
                group_info.priority = req->isVlan ? HAGT_QACL_TCAM_VLAN_BLOCK: HAGT_QACL_TCAM_PORT_BLOCK;
            } 
            else
            {
                /* egress acl has only 1 lookup*/
                group_info.priority = HAGT_QACL_TCAM_PORT_BLOCK;
            }
/* modification end */

            HAGT_IF_ERROR_RETURN(ctc_acl_create_group(req->label, &group_info));
            HAGT_IF_ERROR_RETURN(ctc_acl_install_group(req->label, &group_info));
               }
           }
            break;

        /* unapply qos label from gport */
        case HalMsgQosLabelOp_unapply:
            HAGT_QOS_POLICER_DEBUG_INFO("unapply qos label, %s = %lu, dir = %ld", 
                                        req->isVlan ? "vlan-id" : "gport",
                                        req->gportOrVlanId, req->dir);
#if 0
            if(req->logicPort)
            {
                HAGT_IF_ERROR_RETURN(ctc_acl_destroy_group(req->label));
            }
            else
#endif
            {
            if (req->isVlan)
            {
                HAGT_IF_ERROR_RETURN(ctc_vlan_set_direction_property(req->gportOrVlanId, CTC_VLAN_DIR_PROP_ACL_EN, req->dir, 0));  
            }
            else
            {
                //HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->gportOrVlanId, CTC_PORT_DIR_PROP_ACL_EN, req->dir, 0));
                /*modified by wangl for multicast 2013-7-2, modified by ychen in 2013-10-11 for bug 25183*/
#ifdef V580_OPENFLOW
                if (hagt_ipuc_get_ipv6_enabled())
                {
                    //HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->gportOrVlanId, 
                    //    CTC_PORT_DIR_PROP_ACL_EN, req->dir, 0xf));
                }
#endif
                /*added by ychen for bug 25183 in 2013-10-12*/
                HAGT_IF_ERROR_RETURN (hagt_ifdb_set_port_flag(req->gportOrVlanId, HAGT_IF_PORT_FLAG_ACL_APPLY, 0));
            }
            if(req->isLastMember)
                HAGT_IF_ERROR_RETURN(ctc_acl_destroy_group(req->label));
            }
            break;

        default: 
            return HAGT_E_INVALID_PARAM;
    }
    
    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_map_flow_entry 
 * Purpose      : Map flow entry from hagt to ctc
 * Input        : p_hagt_entry  - pointer to hagt flow entry
 * Output       : p_ctc_entry   - pointer to ctc flow entry
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32
_hagt_qos_map_flow_entry(HalMsgQosFlowEntry_t* p_hagt_entry, ctc_acl_entry_t* p_ctc_entry, uint32 label)
{
    HAGT_QOS_PTR_CHECK(p_hagt_entry);
    HAGT_QOS_PTR_CHECK(p_ctc_entry);
    
    HAGT_QOS_POLICER_DEBUG_FUNC();

    /* 1. map flow action from hagt to ctc */
    HAGT_IF_ERROR_RETURN(_hagt_qos_map_flow_action(&p_hagt_entry->action, &p_ctc_entry->action, label));
    if (p_hagt_entry->rule.discard)
    {
        GLB_UNSET_FLAG(p_ctc_entry->action.flag, CTC_ACL_ACTION_FLAG_RANDOM_LOG);
        GLB_SET_FLAG(p_ctc_entry->action.flag, CTC_ACL_ACTION_FLAG_DISCARD);
        /*Added by yanxa, for bug 17132, no fdb learnt if discarded by flow rule. 2011-11-29*/
/* modified by cuixl for bug 27208, 2014-04-01 */
/* modification start */
/* Egress ACL doesn't support deny leaning and SDK would return error code*/
            ctc_acl_group_info_t group_info = { 0 };
            HAGT_IF_ERROR_RETURN(ctc_acl_get_group_info(label, &group_info));
            if (group_info.dir == CTC_INGRESS)
            {            
                GLB_SET_FLAG(p_ctc_entry->action.flag, CTC_ACL_ACTION_FLAG_DENY_LEARNING);
            }
/* modification end */        
    }
    /*
    if (p_hagt_entry->rule.invalid)
    {
        GLB_SET_FLAG(p_ctc_entry->action.flag, CTC_ACLQOS_ACTION_INVALID_FLAG);
    }
    */

    /* 2. map flow rule from hagt to ctc */
   /*entry_id combine*/
    GLB_QOS_ACL_GET_ENTRY_ID(p_hagt_entry->ruleId, label, p_ctc_entry->entry_id);
    HAGT_IF_ERROR_RETURN(hagt_flow_map_rule(&p_hagt_entry->rule, &p_ctc_entry->key));
    return HAGT_E_NONE;
}
#if 0
static inline int32
_hagt_qos_need_add_ipv4_key_with_l2type(HalMsgFlowRule_t* mac)
{
    HAGT_QOS_PTR_CHECK(mac);

    if (mac->ruleType == GLB_FLOW_RULE_TYPE_MAC)
    {
        if (GLB_FLAG_ISSET(mac->rule.macRule.flag, GLB_FLOW_MAC_RULE_FLAG_L2TYPE))
        {
            return TRUE;
        }
    }
    
    return FALSE;
}
#endif
static inline int32
_hagt_qos_need_add_ipv6_key(HalMsgFlowRule_t* mac)
{
    if (GLB_FLAG_ISSET(g_pst_hagt_qos_master->flow_proc_flag, GLB_QOS_FLOW_PROCESSING_IPV6)
        && mac && mac->ruleType == GLB_FLOW_RULE_TYPE_MAC)
    {
        if(GLB_FLAG_ISSET(mac->rule.macRule.flag, GLB_FLOW_MAC_RULE_FLAG_ARP_OP_CODE))
        {
            return FALSE;
        }
        if (!GLB_FLAG_ISSET(mac->rule.macRule.flag, GLB_FLOW_MAC_RULE_FLAG_ETHTYPE))
        {
            return TRUE;
        }
        if ((mac->rule.macRule.ethType & ((~mac->rule.macRule.ethTypeMask)&0xffff)) == 
            (GLB_FLOW_ETHER_TYPE_IPV6 & ((~mac->rule.macRule.ethTypeMask)&0xffff)))
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

/*modified by yanxa, for bug 17674, 2012-03-06*/
static inline int32
_hagt_qos_need_add_mpls_key(HalMsgFlowRule_t* mac)
{
/* modified by cuixl for bug 27869, 2014-03-28 */
/* modification start */
/* we don't need mpls key in GG, so return false anyway*/
    return FALSE;
/* modification end */

    if (GLB_FLAG_ISSET(g_pst_hagt_qos_master->flow_proc_flag, GLB_QOS_FLOW_PROCESSING_MPLS)
        && mac && mac->ruleType == GLB_FLOW_RULE_TYPE_MAC)
    {
        if(GLB_FLAG_ISSET(mac->rule.macRule.flag, GLB_FLOW_MAC_RULE_FLAG_ARP_OP_CODE))
        {
            return FALSE;
        }
        if (!GLB_FLAG_ISSET(mac->rule.macRule.flag, GLB_FLOW_MAC_RULE_FLAG_ETHTYPE))
        {
            return TRUE;
        }
        if ((mac->rule.macRule.ethType & ((~mac->rule.macRule.ethTypeMask)&0xffff)) == 
            (GLB_FLOW_ETHER_TYPE_MPLS  & ((~mac->rule.macRule.ethTypeMask)&0xffff)))
        {
            return TRUE;
        }
        if ((mac->rule.macRule.ethType & ((~mac->rule.macRule.ethTypeMask)&0xffff)) == 
            (GLB_FLOW_ETHER_TYPE_MMPLS & ((~mac->rule.macRule.ethTypeMask)&0xffff)))
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

/****************************************************************************************
 * Name         : _hagt_qos_set_flow_processing_flag 
 * Purpose      : Set weather the mac rules should merge to mpls or ipv6 rules
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_set_flow_processing_flag(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosSetFlowProcFlagReq_t *req;
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_POLICER_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetFlowProcFlag;

    if (GLB_FLAG_ISSET(req->flag, GLB_QOS_FLOW_PROCESSING_IPV6))
    {
        GLB_SET_FLAG(g_pst_hagt_qos_master->flow_proc_flag, GLB_QOS_FLOW_PROCESSING_IPV6);
    }

    if (GLB_FLAG_ISSET(req->flag, GLB_QOS_FLOW_PROCESSING_MPLS))
    {
        GLB_SET_FLAG(g_pst_hagt_qos_master->flow_proc_flag, GLB_QOS_FLOW_PROCESSING_MPLS);
    }

    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_add_flow_entry 
 * Purpose      : Add a flow entry to QoS label
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_add_flow_entry(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosAddFlowEntryReq_t *req;
    HalMsgQosFlowEntry_t **pp_array;
    ctc_acl_entry_t   ctc_entry;
    ctc_acl_entry_t   ctc_merge_ipv6_entry;
    ctc_acl_entry_t   ctc_merge_mpls_entry;
    int32  index, count = 0;
    uint32 entry_priority;
    
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_POLICER_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosAddFlowEntry;

    pp_array = req->entryList.list.array;
    count    = req->entryList.list.count;
    for (index = 0; index < count; index++)
    {
        sal_memset(&ctc_entry, 0, sizeof(ctc_acl_entry_t));

        HAGT_QOS_POLICER_DEBUG_INFO("add flow rule, type = %lu,  rule_id = %lu", 
            pp_array[index]->rule.ruleType, (pp_array[index]->ruleId * 2) + (req->label << 22));
  
        /* 2. map flow entry from hagt to ctc */
        _hagt_qos_map_flow_entry(pp_array[index], &ctc_entry, req->label);
        entry_priority = GLB_QOS_FLOW_RULE_PRIORITY_MAX - ((unsigned int)(pp_array[index]->rulePriorityHig) << 25) - pp_array[index]->rulePriorityLOW;

        if(ctc_entry.action.flag & CTC_ACL_ACTION_FLAG_STATS)
        {
            hagt_qos_acl_add_stats(ctc_entry.action.stats_id);
        }

        /* 3. add flow entry to sdk */
        HAGT_IF_ERROR_RETURN(ctc_acl_add_entry(req->label, &ctc_entry));
        HAGT_IF_ERROR_RETURN(ctc_acl_set_entry_priority(ctc_entry.entry_id, entry_priority));
        if(! pp_array[index]->rule.invalid)
        {
            HAGT_IF_ERROR_RETURN(ctc_acl_install_entry(ctc_entry.entry_id));
        }

        /* 4. if need merge flow entry, add it to sdk */
#if 0
        if(_hagt_qos_need_add_ipv4_key_with_l2type(&pp_array[index]->rule))
        {
            sal_memcpy(&ctc_merge_ipv4_entry, &ctc_entry, sizeof(ctc_acl_entry_t));
            GLB_UNSET_FLAG(ctc_merge_ipv4_entry.key.u.ipv4_key.flag, CTC_ACL_IPV4_KEY_FLAG_L2_TYPE);
            ctc_merge_ipv4_entry.entry_id = ctc_entry.entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV4;
            HAGT_IF_ERROR_RETURN(ctc_acl_add_entry(req->label, &ctc_merge_ipv4_entry));
            HAGT_IF_ERROR_RETURN(ctc_acl_set_entry_priority(ctc_merge_ipv4_entry.entry_id, entry_priority));
            if(! pp_array[index]->rule.invalid)
            {
                 HAGT_IF_ERROR_RETURN(ctc_acl_install_entry(ctc_merge_ipv4_entry.entry_id));
            }
            HAGT_QOS_POLICER_DEBUG_INFO("add mac2ipv4 flow rule, rule_id = %u", ctc_merge_ipv4_entry.entry_id);
        }
#endif
        if (_hagt_qos_need_add_ipv6_key(&pp_array[index]->rule))
        {
            sal_memset(&ctc_entry.key, 0, sizeof(ctc_acl_key_t));

            ctc_entry.key.type = CTC_ACL_KEY_IPV6;
            hagt_flow_map_key_mac2ipv6(&pp_array[index]->rule.rule.macRule, 
                                  &ctc_entry.key.u.ipv6_key);
            sal_memcpy(&ctc_merge_ipv6_entry, &ctc_entry, sizeof(ctc_acl_entry_t));
            ctc_merge_ipv6_entry.entry_id = ctc_entry.entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV6;
          
            HAGT_IF_ERROR_RETURN(ctc_acl_add_entry(req->label, &ctc_merge_ipv6_entry));
            HAGT_IF_ERROR_RETURN(ctc_acl_set_entry_priority(ctc_merge_ipv6_entry.entry_id, entry_priority));
            if(! pp_array[index]->rule.invalid)
            {
                 HAGT_IF_ERROR_RETURN(ctc_acl_install_entry(ctc_merge_ipv6_entry.entry_id));
            }

            HAGT_QOS_POLICER_DEBUG_INFO("add mac2ipv6 flow rule, rule_id = %u", ctc_merge_ipv6_entry.entry_id);
        }
        if (_hagt_qos_need_add_mpls_key(&pp_array[index]->rule))
        {
            sal_memset(&ctc_entry.key, 0, sizeof(ctc_acl_key_t));

            ctc_entry.key.type = CTC_ACL_KEY_MPLS;
            hagt_flow_map_key_mac2mpls(&pp_array[index]->rule.rule.macRule, 
                                  &ctc_entry.key.u.mpls_key);
            sal_memcpy(&ctc_merge_mpls_entry, &ctc_entry, sizeof(ctc_acl_entry_t));
            ctc_merge_mpls_entry.entry_id = ctc_entry.entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_MPLS;
            
            HAGT_IF_ERROR_RETURN(ctc_acl_add_entry(req->label, &ctc_merge_mpls_entry));
            HAGT_IF_ERROR_RETURN(ctc_acl_set_entry_priority(ctc_merge_mpls_entry.entry_id, entry_priority));
            if(! pp_array[index]->rule.invalid)
            {
                HAGT_IF_ERROR_RETURN(ctc_acl_install_entry(ctc_merge_mpls_entry.entry_id));
            }

            HAGT_QOS_POLICER_DEBUG_INFO("add mac2mpls flow rule, rule_id = %u", ctc_merge_mpls_entry.entry_id);
        }
    }
    
    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_update_flow_entry_priority
 * Purpose      : update priority of flow entry 
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_update_flow_entry_priority(Hsrv2AgtMsg_t* p_msg_data)
{
    uint32 rule_id;
    uint32 entry_id;
    uint32 rule_priority;
    uint32 rule_priority_low;
    uint32 rule_priority_hig;
    uint32 label;

    label = p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosUpdatePriority.label;
    rule_id = p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosUpdatePriority.ruleId;
    rule_priority_low = p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosUpdatePriority.rulePriorityLow;
    rule_priority_hig = p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosUpdatePriority.rulePriorityHig;
    rule_priority = GLB_QOS_FLOW_RULE_PRIORITY_MAX - ((unsigned int)(rule_priority_hig) << 25) - rule_priority_low;
    /*the 10 bits of the beginning of "real_rule_id" is meaning label*/
    GLB_QOS_ACL_GET_ENTRY_ID(rule_id, label, entry_id);

    HAGT_IF_ERROR_RETURN(ctc_acl_set_entry_priority(entry_id, rule_priority));
    if(p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosUpdatePriority.mergeIpv6)
    {
        HAGT_IF_ERROR_RETURN(ctc_acl_set_entry_priority(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV6, rule_priority));
    }
    if(p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosUpdatePriority.mergeMpls)
    {
        HAGT_IF_ERROR_RETURN(ctc_acl_set_entry_priority(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_MPLS, rule_priority));
    }
    if(p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosUpdatePriority.mergeIpv4)
    {
        HAGT_IF_ERROR_RETURN(ctc_acl_set_entry_priority(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV4, rule_priority));
    }
    return HAGT_E_NONE;
}


/****************************************************************************************
 * Name         : _hagt_qos_remove_flow_entry 
 * Purpose      : Delete the specified flow entry from QoS label
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_remove_flow_entry(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosDelFlowEntryReq_t *req;
    HalMsgQosFlowEntryId_t **pp_array;
    int32  index, count = 0;
    uint32 entry_id;
    uint32 stats_id;

    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_POLICER_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosDelFlowEntry;
    pp_array = req->entryIdList.list.array;
    count    = req->entryIdList.list.count;

    for (index = 0; index < count; index++)
    {
        GLB_QOS_ACL_GET_ENTRY_ID(pp_array[index]->ruleId, req->label, entry_id);
        HAGT_QOS_POLICER_DEBUG_INFO("remove flow rule, rule_id = %u", entry_id);
        if(GLB_QOS_ACL_STATS_UPDATE_DEL == pp_array[index]->delStats)
        {
            GLB_QOS_ACL_GET_ENTRY_ID(pp_array[index]->statsId, req->label, stats_id);
            ctc_stats_destroy_statsid(stats_id);
        }

        /* 3. remove flow rule from sdk */
        HAGT_IF_ERROR_RETURN(ctc_acl_uninstall_entry(entry_id));
        HAGT_IF_ERROR_RETURN(ctc_acl_remove_entry(entry_id));
        /* 4. if is extended flow rule, remove it from sdk */
        if (pp_array[index]->mergeIpv6)
        {
            HAGT_IF_ERROR_RETURN(ctc_acl_uninstall_entry(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV6));
            HAGT_IF_ERROR_RETURN(ctc_acl_remove_entry(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV6));
        }
/* modified by cuixl for bug 27869, 2014-03-28 */
/* modification start */
/* we don't use mplskey in GG, so comment off*/
#if 0        
        if(pp_array[index]->mergeMpls)
        {
            HAGT_IF_ERROR_RETURN(ctc_acl_uninstall_entry(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_MPLS));
            HAGT_IF_ERROR_RETURN(ctc_acl_remove_entry(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_MPLS));
        }
#endif        
/* modification end */
        
 #if 0
        if(pp_array[index]->mergeIpv4)
        {
            HAGT_IF_ERROR_RETURN(ctc_acl_uninstall_entry(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV4));
            HAGT_IF_ERROR_RETURN(ctc_acl_remove_entry(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV4));
            hagt_qos_acl_del_stats(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV4);
        }
#endif
    }
    
    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_invalid_flow_entry 
 * Purpose      : Invalid the specified flow entry from QoS label
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_invalid_flow_entry(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosInvalidFlowEntryReq_t *req;
    bool invalid;
    uint32 entry_id;

    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_POLICER_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosInvalidFlowEntry;
    invalid = req->invalid;
    GLB_QOS_ACL_GET_ENTRY_ID(req->entryId.ruleId, req->label, entry_id);

    HAGT_QOS_POLICER_DEBUG_INFO("invalid flow rule, type = %lu, rule_id = %u", 
        req->entryId.ruleType, entry_id);
    
    
    /* 3. remove flow rule from sdk */
    if(invalid)
    {
        HAGT_IF_ERROR_RETURN(ctc_acl_uninstall_entry(entry_id));
    }
    else
    {
        HAGT_IF_ERROR_RETURN(ctc_acl_install_entry(entry_id));
    }
   

    /* 4. if is extended flow rule, remove it from sdk */
    if(req->entryId.mergeIpv4)
    {
        if(invalid)
        {
            ctc_acl_uninstall_entry(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV4);
        }
        else
        {
            ctc_acl_install_entry(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV4);
        }
    }
    if(req->entryId.mergeIpv6)
    {
        if(invalid)
        {
            HAGT_IF_ERROR_RETURN(ctc_acl_uninstall_entry(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV6));
        }
        else
        {
            HAGT_IF_ERROR_RETURN(ctc_acl_install_entry(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV6));
        }
    }
    if(req->entryId.mergeMpls)
    {
        if(invalid)
        {
            HAGT_IF_ERROR_RETURN(ctc_acl_uninstall_entry(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_MPLS));
        }
        else
        {
            HAGT_IF_ERROR_RETURN(ctc_acl_install_entry(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_MPLS));
        }
    }
    
    return HAGT_E_NONE;
}


/****************************************************************************************
 * Name         : _hagt_qos_remove_all_flow_entry 
 * Purpose      : Delete all flow entry from QoS label
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_remove_all_flow_entry(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosDelAllFlowEntryReq_t *req;
    
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_POLICER_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosDelAllFlowEntry;

    HAGT_QOS_POLICER_DEBUG_INFO("remove all flow rules, label = %lu", req->label);

    /* 2. remove all flow rules from sdk */
    /*don't return error because remove agg will do twice*/
    ctc_acl_uninstall_group(req->label);   
    ctc_acl_remove_all_entry(req->label);
    
    return HAGT_E_NONE;
}

static int32
_hagt_qos_remove_all_flow_stats(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosDelAllFlowStatsReq_t *req;
    uint32 stats_id;

    HAGT_QOS_PTR_CHECK(p_msg_data);

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosDelAllFlowStats;
    GLB_QOS_ACL_GET_ENTRY_ID(req->statsId, req->label, stats_id);
    /*may be can not find stats id*/
    ctc_stats_destroy_statsid(stats_id);

    return HAGT_E_NONE;
}
/****************************************************************************************
 * Name         : _hagt_qos_set_port_queue_wdrr_weight_mtu
 * Purpose      : Set port queue wdrr weight mtu
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_set_port_queue_wdrr_weight_mtu(Hsrv2AgtMsg_t* p_msg_data)
{   
    ctc_qos_queue_cfg_t que_cfg = { 0 };
    
    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_QUEUE_DEBUG_FUNC();

    /* 1. get message data */
    que_cfg.type = CTC_QOS_QUEUE_CFG_SCHED_WDRR_MTU;
    
    que_cfg.value.value_32 = p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetPortQueueWdrrWeightMtu.mtu;

    HAGT_QOS_QUEUE_DEBUG_INFO("queue wdrr weight mtu: %u", que_cfg.value.value_32);
    HAGT_IF_ERROR_RETURN(ctc_qos_set_queue(&que_cfg));

    return HAGT_E_NONE;
}

/****************************************************************************************
 * Name         : _hagt_qos_set_port_shape
 * Purpose      : Set port shaping for the specified port
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_set_port_shape(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosSetPortShapeReq_t *req;
    ctc_qos_shape_t ctc_shape = { 0 };

    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_QUEUE_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosSetPortShape;

    /* 2. set port shape */
    HAGT_QOS_QUEUE_DEBUG_INFO("set port shape, gport = 0x%lx", req->port);

    ctc_shape.type = CTC_QOS_SHAPE_PORT;
    ctc_shape.shape.port_shape.gport = req->port;
//    ctc_shape.shape.port_shape.pir = req->shape.pir;
    ctc_shape.shape.port_shape.pir = HAGT_QOS_MAP_RATE_BYTE2KBIT(req->shape.pir);
    /*modify by chenxw for bug 25023, 2013-10-11.*/
    //ctc_shape.shape.port_shape.pbs = req->shape.pbs;
    ctc_shape.shape.port_shape.pbs = CTC_QOS_SHP_TOKE_THRD_DEFAULT;
//    ctc_shape.shape.port_shape.pbs = HAGT_QOS_MAP_RATE_BYTE2KBIT(req->shape.pbs);

    ctc_shape.shape.port_shape.enable = TRUE;
#if 0    
    /* GLB_QOS_ESPECAIL_BURST means disable port shape */
    if (req->shape.pbs == GLB_QOS_ESPECAIL_BURST)
    {
        ctc_shape.shape.port_shape.enable = FALSE;
    }
#endif    
    HAGT_QOS_QUEUE_DEBUG_INFO("pir = %u (kbits/s)", ctc_shape.shape.port_shape.pir);
    HAGT_QOS_QUEUE_DEBUG_INFO("pbs = %u (kbit)", ctc_shape.shape.port_shape.pbs);
    HAGT_IF_ERROR_RETURN(ctc_qos_set_shape(&ctc_shape));

    return HAGT_E_NONE;
}


/****************************************************************************************
 * Name         : _hagt_qos_unset_port_shape
 * Purpose      : Unset port shaping for the specified port
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_unset_port_shape(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosUnsetPortShapeReq_t *req;
    ctc_qos_shape_t ctc_shape = { 0 };

    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_QUEUE_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosUnsetPortShape;
    
    if (!hagt_chip_port_is_local(req->port))
    {
        return HAGT_E_NONE;
    }

    /* 2. unset port shape */
    HAGT_QOS_QUEUE_DEBUG_INFO("unset port shape, gport = 0x%lx", req->port);
    ctc_shape.type = CTC_QOS_SHAPE_PORT;
    ctc_shape.shape.port_shape.gport = req->port;
    ctc_shape.shape.port_shape.enable = FALSE;
    HAGT_IF_ERROR_RETURN(ctc_qos_set_shape(&ctc_shape));
    
    return HAGT_E_NONE;
}


/****************************************************************************************
 * Name         : _hagt_qos_get_port_queue_depth
 * Purpose      : Get port queue depth information
 * Input        : p_msg_data    - message data from hsrv to hagt
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_get_port_queue_depth(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgQosGetPortQueueDepthReq_t *req;
    Hagt2SrvMsg_t hagt_msg;
  //  HalMsgQosQueueDepthEntry_t depthInfo[GLB_QOS_PORT_QUEUE_NUM];
    //ctc_queue_depth_t queue_depth;
//    int32 queueid;

    HAGT_QOS_PTR_CHECK(p_msg_data);

    HAGT_QOS_QUEUE_DEBUG_FUNC();

    /* 1. get message data */
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgQosGetPortQueueDepth;

    /* 2. unset port shape */
    HAGT_QOS_QUEUE_DEBUG_INFO("get port queue depth, gport = 0x%lx", req->port);

    sal_memset(&hagt_msg, 0, sizeof(hagt_msg));
    hagt_msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgQosGetPortQueueDepthResp;
    // TODO: sdk api is not supported Now
    #if 0
    for (queueid = 0; queueid < HAGT_QOS_PORT_QUEUE_NUM; queueid++)
    {
    
        ret = ctc_get_port_queue_depth(req->port, queueid, &queue_depth);
        if (CTC_E_NONE != ret)
        {
            goto FINALLY;
        }

        depthInfo[queueid].avgDepth = queue_depth.queue_avg_depth;
        depthInfo[queueid].instDepth = queue_depth.queue_inst_depth;

        ASN_SET_ADD(&(hagt_msg.opHagt2Srv.hagt2SrvMsgQosGetPortQueueDepthResp.depthInfo.list), &depthInfo[queueid]); 
    }
#endif
    hagt_msg.opHagt2Srv.hagt2SrvMsgQosGetPortQueueDepthResp.success = TRUE;
    hagt_msg.opHagt2Srv.hagt2SrvMsgQosGetPortQueueDepthResp.port = req->port;

//FINALLY:
    HAGT2SRV_MSG_SEND(&hagt_msg);
    asn_set_empty(&(hagt_msg.opHagt2Srv.hagt2SrvMsgQosGetPortQueueDepthResp.depthInfo.list));

    return HAGT_E_NONE;
}


/****************************************************************************************
 * Name         : _hagt_qos_get_flow_rule_stats
 * Purpose      : Get flow rule stats which matching the rules in policy-map
 * Input        : p_msg_data    - message data from hal to hsrv
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_get_flow_rule_stats(HalMsgStatsKey_t* key, glb_stats_result_t* p_stats)
{
    ctc_stats_basic_t ctc_stats;
    uint16 label;
    uint8  ace_type;
    uint32 entry_id;
    char pkt_str[GLB_INT64_DEC_LENGTH];
    char byte_str[GLB_INT64_DEC_LENGTH];
    uint32 stats_id;

    (void)entry_id;
    
    /*1.check & dump message*/
    HAGT_QOS_POLICER_DEBUG_FUNC();
    GLB_PTR_VALID_CHECK(key, HAGT_E_INVALID_PTR);
    GLB_PTR_VALID_CHECK(p_stats, HAGT_E_INVALID_PTR);
    HAGT_QOS_POLICER_DEBUG_INFO("get flow rule stats, label = %ld, type = %ld, entry_id = %ld\n", 
                                key->flowRuleKey.label, key->flowRuleKey.aceType, key->flowRuleKey.entryId);
    label = key->flowRuleKey.label;
    ace_type = key->flowRuleKey.aceType;
    GLB_QOS_ACL_GET_ENTRY_ID(key->flowRuleKey.entryId, label, entry_id);
    GLB_QOS_ACL_GET_ENTRY_ID(key->flowRuleKey.statsId, label, stats_id);
    /*2.get stats result*/
    sal_memset(p_stats, 0, sizeof(glb_stats_result_t));
    sal_memset(&ctc_stats, 0, sizeof(ctc_stats_basic_t));

    switch(ace_type)
    {
        case GLB_FLOW_RULE_TYPE_MAC:
            HAGT_IF_ERROR_RETURN(ctc_stats_get_stats(stats_id, &ctc_stats));
 #if 0  /*removed by wangl for SDK stats mege 2013-8-8*/
            p_stats->basic.byte_count   += ctc_stats.byte_count;
            p_stats->basic.packet_count += ctc_stats.packet_count;
            /*may be merge ipv4 key*/
            sal_memset(&ctc_stats, 0, sizeof(ctc_stats_basic_t));
            stats_id = hagt_qos_acl_lk_stats(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV4);
            if(HAGT_E_ENTRY_NOT_EXIST != stats_id)
            {
                HAGT_IF_ERROR_RETURN(ctc_stats_get_stats(stats_id, &ctc_stats));
                p_stats->basic.byte_count   += ctc_stats.byte_count;
                p_stats->basic.packet_count += ctc_stats.packet_count;
            }    
            /*may be merge ipv6 key*/
            if (GLB_FLAG_ISSET(g_pst_hagt_qos_master->flow_proc_flag, GLB_QOS_FLOW_PROCESSING_IPV6))
            {
                
                sal_memset(&ctc_stats, 0, sizeof(ctc_stats_basic_t));
                stats_id = hagt_qos_acl_lk_stats(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV6);
                if(HAGT_E_ENTRY_NOT_EXIST != stats_id)
                {
                    HAGT_IF_ERROR_RETURN(ctc_stats_get_stats(stats_id, &ctc_stats));
                    p_stats->basic.byte_count   += ctc_stats.byte_count;
                    p_stats->basic.packet_count += ctc_stats.packet_count;
                }   
            }
            /*may be merge mpls key*/
            sal_memset(&ctc_stats, 0, sizeof(ctc_stats_basic_t));
            stats_id = hagt_qos_acl_lk_stats(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_MPLS);
            if(HAGT_E_ENTRY_NOT_EXIST != stats_id)
            {
                HAGT_IF_ERROR_RETURN(ctc_stats_get_stats(stats_id, &ctc_stats));
                /*add stats at last*/
            }  
 #endif       
            break;
        case GLB_FLOW_RULE_TYPE_IPV4:
            HAGT_IF_ERROR_RETURN(ctc_stats_get_stats(stats_id, &ctc_stats));
            break;
        case GLB_FLOW_RULE_TYPE_IPV6:
            HAGT_IF_ERROR_RETURN(ctc_stats_get_stats(stats_id, &ctc_stats));
            break;
#if 0/*modified by yanxa, for bug 17674, 2012-03-06*/
        case GLB_FLOW_RULE_TYPE_EXIP:
            ctc_qos_entry_stats_get(label, CTC_QOS_LABEL, CTC_ACLQOS_MAC_KEY, entry_id, &ctc_stats);
            p_stats->basic.byte_count   = ctc_stats.byte_count;
            p_stats->basic.packet_count = ctc_stats.packet_count;

            sal_memset(&ctc_stats, 0, sizeof(ctc_stats_basic_t));
            ctc_qos_entry_stats_get(label, CTC_QOS_LABEL, CTC_ACLQOS_IPV4_KEY, entry_id, &ctc_stats);
            break;
        case GLB_FLOW_RULE_TYPE_EXIPV6:
            ctc_qos_entry_stats_get(label, CTC_QOS_LABEL, CTC_ACLQOS_IPV6_KEY, entry_id, &ctc_stats);
            break;
#endif
        /*Added by yanxa, for mpls bug 17222, 2011-12-09*/
        case GLB_FLOW_RULE_TYPE_MPLS:
            HAGT_IF_ERROR_RETURN(ctc_stats_get_stats(stats_id, &ctc_stats));
            break;
        default:
            return HAGT_E_INVALID_PARAM;
    }
    p_stats->basic.byte_count   += ctc_stats.byte_count;
    p_stats->basic.packet_count += ctc_stats.packet_count;
    UINT64_TO_STR(p_stats->basic.packet_count, pkt_str);
    UINT64_TO_STR(p_stats->basic.byte_count, byte_str);
    HAGT_QOS_POLICER_DEBUG_INFO("flow rule stats result: pkts = %s, byte = %s\n",
                                pkt_str, byte_str);

    return HAGT_E_NONE;
}


/****************************************************************************************
 * Name         : _hagt_qos_clear_flow_rule_stats
 * Purpose      : Clear flow rule stats which matching the rules in policy-map
 * Input        : p_msg_data    - message data from hal to hsrv
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32 
_hagt_qos_clear_flow_rule_stats(HalMsgStatsKey_t* p_stats_key)
{
    uint16 label;
    uint8  ace_type;
    uint32 entry_id;
    uint32 stats_id;

    (void)entry_id;
    
    /*1.check & dump message*/
    HAGT_QOS_POLICER_DEBUG_FUNC();
    GLB_PTR_VALID_CHECK(p_stats_key, HAGT_E_INVALID_PTR);
    
    label = p_stats_key->flowRuleKey.label;
    ace_type = p_stats_key->flowRuleKey.aceType;
    GLB_QOS_ACL_GET_ENTRY_ID(p_stats_key->flowRuleKey.entryId, label, entry_id);
    GLB_QOS_ACL_GET_ENTRY_ID(p_stats_key->flowRuleKey.statsId, label, stats_id);
    HAGT_QOS_POLICER_DEBUG_INFO("clear flow rule stats, label = %d, type = %d, entry_id = %u\n", 
                                label, ace_type, entry_id);
 
    switch(ace_type)
    {
        case GLB_FLOW_RULE_TYPE_MAC:
            HAGT_IF_ERROR_RETURN(ctc_stats_clear_stats(stats_id));
  #if 0 /*removed by wangl for SDK stats mege 2013-8-8*/
            /*may be merge ipv4 key*/
            stats_id = hagt_qos_acl_lk_stats(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV4);
            if(HAGT_E_ENTRY_NOT_EXIST != stats_id)
            {
                HAGT_IF_ERROR_RETURN(ctc_stats_clear_stats(stats_id));
            }  
            /*modified by yanxa, for bug 17674, 2012-03-06*/
            /*may be merge mpls key*/
            stats_id = hagt_qos_acl_lk_stats(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_MPLS);
            if(HAGT_E_ENTRY_NOT_EXIST != stats_id)
            {
                HAGT_IF_ERROR_RETURN(ctc_stats_clear_stats(stats_id));
            } 
            if (GLB_FLAG_ISSET(g_pst_hagt_qos_master->flow_proc_flag, GLB_QOS_FLOW_PROCESSING_IPV6))
            {
                stats_id = hagt_qos_acl_lk_stats(entry_id + GLB_QOS_ACL_MAC_KEY_MERGE_IPV6);
                if(HAGT_E_ENTRY_NOT_EXIST != stats_id)
                {
                    HAGT_IF_ERROR_RETURN(ctc_stats_clear_stats(stats_id));
                } 
            }
  #endif
            break;
        case GLB_FLOW_RULE_TYPE_IPV4:
            HAGT_IF_ERROR_RETURN(ctc_stats_clear_stats(stats_id));
            break;
        case GLB_FLOW_RULE_TYPE_IPV6:
            HAGT_IF_ERROR_RETURN(ctc_stats_clear_stats(stats_id));
            break;
#if 0/*modified by yanxa, for bug 17674, 2012-03-06*/
        case GLB_FLOW_RULE_TYPE_EXIP:
            ctc_qos_entry_stats_reset(label, CTC_ACLQOS_MAC_KEY,  entry_id);
            ctc_qos_entry_stats_reset(label, CTC_ACLQOS_IPV4_KEY, entry_id);
            break;
        case GLB_FLOW_RULE_TYPE_EXIPV6:
            ctc_qos_entry_stats_reset(label, CTC_ACLQOS_IPV6_KEY, entry_id);
            break;
#endif
        /*Added by yanxa, for mpls bug 17222, 2011-12-09*/
        case GLB_FLOW_RULE_TYPE_MPLS:
            HAGT_IF_ERROR_RETURN(ctc_stats_clear_stats(stats_id));
            break;
        default:
            return HAGT_E_INVALID_PARAM;
    }

    return HAGT_E_NONE;
}

/* modified by cuixl for bug 22441, 2013-03-18 */
/* We don't do resource check before, so we need store these failed entries when resource is ok*/
/* Now we do resource check, so function is no need any more */
/*modification start*/
#if 0
/****************************************************************************************
 * Name         : hagt_qos_debug_dump
 * Purpose      : Dump Hagt QoS database
 * Input        : N/A
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
int32 
hagt_qos_debug_dump()
{
#ifdef _GLB_ENABLE_DBGSHOW_
    ctclib_list_node_t *p_node;
    hagt_qos_pend_flow_entry_t *p_pend_entry;
    int32 count = 0;

    CTCLIB_DUMP_OUT("-------------------------------------------------------------\n");
    CTCLIB_DUMP_OUT("                  HAGT Humber QoS Pend Flow Entry\n");
    CTCLIB_DUMP_OUT("-------------------------------------------------------------\n\n");

    CTCLIB_DUMP_OUT(" %-6s%-8s%-12s%-8s\n", "", "label", "rule-id", "notified");
    CTCLIB_DUMP_OUT(" ----------------------------------------------\n");
    
    sal_mutex_lock(g_pst_hagt_qos_master->p_pend_flow_entry_mutex);
    ctclib_list_for_each(p_node, &g_pst_hagt_qos_master->pend_flow_entry_list)
    {
        p_pend_entry = ctclib_container_of(p_node, hagt_qos_pend_flow_entry_t, node);

        ++count;
        CTCLIB_DUMP_OUT(" %-6d", count);
        CTCLIB_DUMP_OUT("%-8d", p_pend_entry->label);
        CTCLIB_DUMP_OUT("%-12u", p_pend_entry->rule_id);
        CTCLIB_DUMP_OUT("%-8s\n", p_pend_entry->has_notified ? "Y" : "N");
    }
    sal_mutex_unlock(g_pst_hagt_qos_master->p_pend_flow_entry_mutex);
#endif /* _GLB_ENABLE_DBGSHOW_ */

    return HAGT_E_NONE;
}
#endif



/****************************************************************************************
 * Name         : _hagt_if_register_hagt_message_callback
 * Purpose      : Register hagt message callbacks
 * Input        : N/A
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
static int32
_hagt_qos_register_hagt_message_callback (void)
{   
    /* ~~~~~~~~~~ QoS Global/Intf configuration ~~~~~~~~~~~~*/
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosSetGlobalFlag         , _hagt_qos_set_global_enable_flags);
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosInitPortCfg           , _hagt_qos_init_qos_port_cfg      );
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosDeinitPortCfg         , _hagt_qos_deinit_qos_port_cfg    );
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosSetPortAttribute      , _hagt_qos_set_port_attribute     );
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosSyncPortCfg           , _hagt_qos_sync_qos_port_cfg      );

    /* ~~~~~~~~~~ QoS Classification ~~~~~~~~~~~~*/
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosSetPriColorMapTable   , _hagt_qos_set_pri_color_map_table);
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosSetCfiEnable          , _hagt_qos_set_cfi_enable         );

    /* ~~~~~~~~~~ QoS Policer ~~~~~~~~~~~~*/
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosSetPortPolicer        , _hagt_qos_set_port_policer       );
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosUnsetPortPolicer      , _hagt_qos_unset_port_policer     );
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosFlowPolicerOp         , _hagt_qos_flow_policer_op        );

    /* ~~~~~~~~~~ QoS Drop ~~~~~~~~~~~~*/
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosSetPortQueueDrop      , _hagt_qos_set_port_queue_drop    );

    /* ~~~~~~~~~~ QoS Shape ~~~~~~~~~~~~*/    
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosSetPortQueueShape     , _hagt_qos_set_port_queue_shape   );
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosUnsetPortQueueShape   , _hagt_qos_unset_port_queue_shape );    
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosSetPortShape          , _hagt_qos_set_port_shape         );
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosUnsetPortShape        , _hagt_qos_unset_port_shape       );

    /* stats */
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosSetPortQueueStats      , _hagt_qos_set_port_queue_stats    );

    /* ~~~~~~~~~~ QoS Schedule ~~~~~~~~~~~~*/
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosSetPortQueueClass     , _hagt_qos_set_port_queue_class   );
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosSetPortQueueWdrrWeight,_hagt_qos_set_port_queue_wdrr_weight);
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosSetPortQueueWdrrWeightMtu, _hagt_qos_set_port_queue_wdrr_weight_mtu);
    

    /* ~~~~~~~~~~ Flow ~~~~~~~~~~~~*/
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosLabelOp            , _hagt_qos_label_op               );
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosAddFlowEntry       , _hagt_qos_add_flow_entry         );
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosDelFlowEntry       , _hagt_qos_remove_flow_entry      );
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosInvalidFlowEntry   , _hagt_qos_invalid_flow_entry     );
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosDelAllFlowEntry    , _hagt_qos_remove_all_flow_entry  );
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosDelAllFlowStats,     _hagt_qos_remove_all_flow_stats);
    #if 0 // TODO: Commented by wangl, for compile pass, 2013-1-17(no need any more)
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosSetQosEnable       , _hagt_qos_set_qos_enable         );
    #endif
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosUpdateFlowAction   , _hagt_qos_update_flow_action     );
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosGetPortQueueDepth  , _hagt_qos_get_port_queue_depth   );

    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosSetFlowProcFlag    , _hagt_qos_set_flow_processing_flag);
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgQosUpdatePriority     , _hagt_qos_update_flow_entry_priority);

    return HAGT_E_NONE;
}

int32
hagt_qos_get_port_queue_num(void)
{
    FILE* fp = NULL; 
    char   sz_profile[4] = { 0 };
    char   sz_queue_num[4] = { 0 };
    uint32 queue_num = 8;
    uint8   idx = 0;
       
    fp = sal_fopen(GLB_QOS_PROFILE_FILE, "r");
    if (!fp)
    {
        return queue_num;
    }

    /* first line is profile id, second line is queue number */
    sal_fgets(sz_profile, 4, fp);
    sal_fgets(sz_queue_num, 4, fp);

    queue_num = 0;
    while (sz_queue_num[idx] != '\0')
    {
        queue_num = queue_num * 10 + (sz_queue_num[idx] - 48);
        idx++;
    }
    
    sal_fclose(fp);        
    return queue_num;
   
}

/****************************************************************************************
 * Name         : hagt_qos_init
 * Purpose      : Initialize QoS module in hagt
 * Input        : N/A
 * Output       : N/A
 * Return       : HAGT_E_XXX
 * Note         : N/A
 ****************************************************************************************/
int32
hagt_qos_init (hagt_master_t* p_hagt_master)
{

    hagt_module_info_t* p_qos_module = NULL; 
    hagt_stast_pm_data_t stats_pm_data;
    uint8 index = 0;
    ctc_parser_l3_protocol_entry_t entry = { 0 };
    ctc_qos_queue_cfg_t que_cfg = { 0 };
    
    /* 1. get hagt qos master, allocte memory */
    g_pst_hagt_qos_master = HAGT_QOS_MALLOC(sizeof(hagt_qos_master_t));
    HAGT_QOS_MEM_CHECK(g_pst_hagt_qos_master);    

    /* 2. register message callback */
    _hagt_qos_register_hagt_message_callback();

    /* 3. register stats cb */
    /* 3.1 register port queue stats cb */
    stats_pm_data.get_stats_cb   = _hagt_qos_get_port_queue_stats;
    stats_pm_data.clear_stats_cb = _hagt_qos_clear_port_queue_stats;
    hagt_stats_regrister_pm_data(GLB_STATS_TYPE_PORT_QUEUE, &stats_pm_data);

    /* 3.2 register port policer stats cb */
    stats_pm_data.get_stats_cb   = _hagt_qos_get_port_policer_stats;
    stats_pm_data.clear_stats_cb = _hagt_qos_clear_port_policer_stats;
    hagt_stats_regrister_pm_data(GLB_STATS_TYPE_PORT_POLICER, &stats_pm_data);

    /* 3.3 register flow policer stats cb */
    stats_pm_data.get_stats_cb   = _hagt_qos_get_flow_policer_stats;
    stats_pm_data.clear_stats_cb = _hagt_qos_clear_flow_policer_stats;
    hagt_stats_regrister_pm_data(GLB_STATS_TYPE_FLOW_POLICER, &stats_pm_data);

    stats_pm_data.get_stats_cb   = _hagt_qos_get_flow_policer_stats;
    stats_pm_data.clear_stats_cb = _hagt_qos_clear_flow_policer_stats;
    hagt_stats_regrister_pm_data(GLB_STATS_TYPE_AGGREGATE_POLICER, &stats_pm_data);
    
    /* 3.4 register flow rule stats cb */
    stats_pm_data.get_stats_cb   = _hagt_qos_get_flow_rule_stats;
    stats_pm_data.clear_stats_cb = _hagt_qos_clear_flow_rule_stats;
    hagt_stats_regrister_pm_data(GLB_STATS_TYPE_FLOW_RULE, &stats_pm_data);
    
/* modified by cuixl for bug 22441, 2013-03-14 */
// TODO: should sdk supports API to get stats enable???
/* refer to ctc_greatbelt_stats_init(), now all policer and queue stats is enabled*/
/*modification start*/
    /* 5. init stats flag */
    GLB_SET_FLAG(g_pst_hagt_qos_master->queue_stats_support_flag, GLB_QOS_QUEUE_STATS_SUPPORT_DEQUEUE);
    GLB_SET_FLAG(g_pst_hagt_qos_master->queue_stats_support_flag, GLB_QOS_QUEUE_STATS_SUPPORT_DROP);
    GLB_SET_FLAG(g_pst_hagt_qos_master->policer_stats_support_flag, GLB_QOS_POLICER_STATS_SUPPORT_VIOLATE);
    GLB_SET_FLAG(g_pst_hagt_qos_master->policer_stats_support_flag, GLB_QOS_POLICER_STATS_SUPPORT_EXCEED);
    GLB_SET_FLAG(g_pst_hagt_qos_master->policer_stats_support_flag, GLB_QOS_POLICER_STATS_SUPPORT_CONFIRM);
/*modification end*/

    /* 6. set queue wdrr weight mtu to default GLB_QOS_QUEUE_WDRR_WEIGHT_MTU_DEF */
/* modified by cuixl for bug 22441, 2013-03-19 */
/*modification start*/
/* this is done in sdk init (sys_greatbelt_queue_sch_init ())*/    
#if 0
    que_cfg.type = CTC_QOS_QUEUE_CFG_SCHED_WDRR_MTU;    
    que_cfg.value.value_32 = GLB_QOS_QUEUE_WDRR_WEIGHT_MTU_DEF;
 HAGT_IF_ERROR_RETURN(ctc_qos_set_queue(&que_cfg));
#endif
/*modification end*/

/* modified by cuixl for bug 22441, 2013-03-18 */
/*modification start*/
/* Now system support CLI to enable/disble, so comment off this */
#if 0
    /* Added by yanxa, for bug 15492, 2011-09-12 */
    /* 10. Globally discard small fragment */
    HAGT_IF_ERROR_RETURN(ctc_qos_tiny_fragment_default_entry_add(TRUE));
#endif    
/*modification end*/
    
    /*7. parse all l3 protocol*/
    index = 2;
    entry.l4_type = CTC_PARSER_L4_TYPE_ANY_PROTO;
    HAGT_IF_ERROR_RETURN(ctc_parser_map_l4_type(index, &entry));
    
    /* 8. Init the flow processing flag */
    g_pst_hagt_qos_master->flow_proc_flag = 0;
    
    /* 9. init qos module */
    p_qos_module = HAGT_QOS_CALLOC(sizeof(hagt_module_info_t));
    if (NULL == p_qos_module)
    {
        HAGT_QOS_LOG_ERR("malloc hagt qos module failed!");
        HAGT_QOS_FREE(g_pst_hagt_qos_master);
        return HAGT_E_NO_MEMORY;
    }
    
    p_qos_module->module_id = HAGT_MODULE_QOS;
    p_qos_module->p_data_info = g_pst_hagt_qos_master;
    /* 10. register to hsrv master */
    p_hagt_master->past_module_info[HAGT_MODULE_QOS] = p_qos_module; 

    /* 11. init port queue number */
    // TODO: after sdk api is supported, we init port queue number in hagt_ctc_feature_resource_init
    g_pst_hagt_qos_master->port_queue_num = hagt_qos_get_port_queue_num();
    sal_memset (&que_cfg, 0, sizeof (que_cfg));
    que_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_NUM;
    que_cfg.value.queue_num.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    que_cfg.value.queue_num.queue_number = g_pst_hagt_qos_master->port_queue_num;
    ctc_qos_set_queue(&que_cfg);

    return HAGT_E_NONE;
}



