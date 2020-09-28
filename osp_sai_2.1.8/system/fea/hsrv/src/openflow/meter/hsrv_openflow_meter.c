/* Added by weizj for meter */
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_msg_openflow.h"
#include "hsrv_openflow_meter_priv.h"
#include "hsrv_openflow_meter.h"


#define ____METER_MSG_API____
/*************************************************************************************************
 * Name         : hsrv_openflow_msg_meter_add_meter_entry
 * Purpose      : Hsrv layer openflow meter send msg for add meter entry
 * Input        : p_meter_info: glb_openflow_meter_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
hsrv_openflow_msg_meter_add_meter_entry(glb_openflow_meter_t* p_meter_info)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowMeterEntryKey_t* p_req = NULL;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddMeterEntry;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddMeterEntry;
    p_req->meterId = p_meter_info->meter_id;
    p_req->statsEn = p_meter_info->stats_enable;
    p_req->meterBand.rate = p_meter_info->rate;
    p_req->meterBand.burst = p_meter_info->burst_size;
    p_req->meterBand.type = p_meter_info->band_type;

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    p_req = NULL;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_msg_meter_delete_meter_entry
 * Purpose      : Hsrv layer openflow meter send msg for delete meter entry
 * Input        : meter_id: uint32
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
hsrv_openflow_msg_meter_delete_meter_entry(uint32 meter_id)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowMeterEntryKey_t* p_req = NULL;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelMeterEntry;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelMeterEntry;
    p_req->meterId = meter_id;

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    p_req = NULL;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_msg_meter_add_stats_db
 * Purpose      : Hsrv layer openflow meter send msg for add stats db
 * Input        : meter_id: uint32, stats_en: uint8
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
hsrv_openflow_msg_meter_add_stats_db(uint32 meter_id, uint8 stats_en)
{
    hsrv_of_stats_t stats_info;

    sal_memset(&stats_info, 0, sizeof(hsrv_of_stats_t));

    if (stats_en)
    {
        stats_info.resource_type = HSRV_OPENFLOW_STATS_RESOURCE_TYPE_METER;
        stats_info.stats_id = 0;
        stats_info.stats.resource_id = meter_id;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_create_stats(&stats_info));
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_msg_meter_delete_stats_db
 * Purpose      : Hsrv layer openflow meter send msg for delete stats db
 * Input        : meter_id: uint32, stats_en: uint8
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
hsrv_openflow_msg_meter_delete_stats_db(uint32 meter_id, uint8 stats_en)
{
    hsrv_of_stats_t stats_info;

    sal_memset(&stats_info, 0, sizeof(hsrv_of_stats_t));

    if (stats_en)
    {
        stats_info.resource_type = HSRV_OPENFLOW_STATS_RESOURCE_TYPE_METER;
        stats_info.stats_id = 0;
        stats_info.stats.resource_id = meter_id;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_del_stats(&stats_info));
    }

    return HSRV_E_NONE;
}


/*************************************************************************************************
 * Name         : hsrv_openflow_msg_meter_update_meter_ipg_status
 * Purpose      : Hsrv layer openflow meter send msg for update meter ipg status
 * Input        : meter_id: uint32
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
hsrv_openflow_msg_meter_update_meter_ipg_status(uint8 enable)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowMeterEntryKey_t* p_req = NULL;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowUpdateMeterIpgStatus;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowUpdateMeterIpgStatus;
    p_req->ipgEn = enable;

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    p_req = NULL;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_msg_meter_clear_stats
 * Purpose      : Hsrv layer openflow meter send msg for clear stats
 * Input        : meter_id: uint32
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
hsrv_openflow_msg_meter_clear_stats(uint32 meter_id)
{
    hsrv_of_stats_t stats;

    sal_memset(&stats, 0, sizeof(hsrv_of_stats_t));

    stats.resource_type = HSRV_OPENFLOW_STATS_RESOURCE_TYPE_METER;
    stats.stats_id = 0;
    stats.stats.resource_id = meter_id;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_clear_stats(&stats));

    return HSRV_E_NONE;
}

#define ____METER_INTERNAL_API____

#define ____METER_HAL_APIs____
/*************************************************************************************************
 * Name         : fei_api_openflow_add_meter
 * Purpose      : Hsrv layer openflow meter handle add meter entry
 * Input        : pst_req: fei_openflow_add_meter_req_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32 
fei_api_openflow_add_meter(fei_openflow_add_meter_req_t* pst_req)
{
    glb_openflow_meter_t* p_meter_info = NULL;
    uint32 meter_id;
    uint8  stats_enable;

    p_meter_info = &pst_req->meter;
    meter_id = p_meter_info->meter_id;
    stats_enable = p_meter_info->stats_enable;

    /* send msg to hagt */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_meter_add_meter_entry(p_meter_info));

    /* send msg to hagt for add to stats db */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_meter_add_stats_db(meter_id, stats_enable));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : fei_api_openflow_mod_meter
 * Purpose      : Hsrv layer openflow meter handle update meter entry
 * Input        : pst_req: fei_openflow_mod_meter_req_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
fei_api_openflow_mod_meter(fei_openflow_mod_meter_req_t* pst_req)
{
    glb_openflow_meter_t* p_meter_info = NULL;
    uint32 meter_id;
    uint8  stats_enable;
    uint8  old_stats_enable;

    p_meter_info = &pst_req->meter;
    meter_id = p_meter_info->meter_id;
    stats_enable = p_meter_info->stats_enable;
    old_stats_enable = pst_req->old_stats_enable;

    /* send msg to hagt for delete from stats db */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_meter_delete_stats_db(meter_id, old_stats_enable));

    /* send msg to hagt */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_meter_add_meter_entry(p_meter_info));
    /* send msg to hagt for add to stats db */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_meter_add_stats_db(meter_id, stats_enable));
 
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : fei_api_openflow_del_meter
 * Purpose      : Hsrv layer openflow meter handle delete meter entry
 * Input        : pst_req: fei_openflow_del_meter_req_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
fei_api_openflow_del_meter(fei_openflow_del_meter_req_t* pst_req)
{
    uint32 meter_id;
    uint8  stats_enable;

    meter_id = pst_req->meter_id;
    stats_enable = pst_req->stats_enable;

    /* send msg to hagt for delete from stats db */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_meter_delete_stats_db(meter_id, stats_enable));

    /* send msg to hagt */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_meter_delete_meter_entry(meter_id));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : fei_api_openflow_mod_meter_ipg_status
 * Purpose      : Hsrv layer openflow meter handle update meter ipg status
 * Input        : pst_req: fei_openflow_mod_meter_ipg_req_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
fei_api_openflow_mod_meter_ipg_status(fei_openflow_mod_meter_ipg_req_t* pst_req)
{
    uint8  enable;

    enable = pst_req->enable;

    /* send msg to hagt */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_meter_update_meter_ipg_status(enable));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : fei_api_openflow_clear_meter_stats
 * Purpose      : Hsrv layer openflow meter handle clear meter stats
 * Input        : pst_req: fei_openflow_clear_stats_req_t *
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
fei_api_openflow_clear_meter_stats(fei_openflow_clear_stats_req_t *pst_req)
{
    /* delete to meter stats hash */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_meter_clear_stats(pst_req->id));

    return HSRV_E_NONE;
}

