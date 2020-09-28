/* Added by weizj for meter */
#include "sal.h"
#include "glb_macro.h"
#include "glb_const.h"
#include "proto.h"
#include "glb_openflow_define.h"
#include "lib.h"

#include "fei.h"

#include "hsrv_msg.h"
#include "hsrv_msg_openflow.h"

#include "fei_openflow_meter.h"


/*************************************************************************************************
 * Name         : fei_meter_add_meter_entry
 * Purpose      : Fei layer add meter entry
 * Input        : p_meter_info: glb_openflow_meter_t*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
fei_meter_add_meter_entry(glb_openflow_meter_t* p_meter_info)
{
    fei_openflow_add_meter_req_t* p_req;
    uint32 req_len;
    int32  rc;

    req_len = sizeof(fei_openflow_add_meter_req_t);
    p_req = XMALLOC(MEM_PM_OPENFLOW_METER, req_len);
    sal_memcpy(&p_req->meter, p_meter_info, req_len);

    rc = fei_talk(FEI_OPENFLOW_METER_ADD, p_req, req_len);
    XFREE(MEM_PM_OPENFLOW_METER, p_req);

    return rc;
}

/*************************************************************************************************
 * Name         : fei_meter_update_meter_entry
 * Purpose      : Fei layer update meter entry
 * Input        : p_meter_info: glb_openflow_meter_t*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
fei_meter_update_meter_entry(glb_openflow_meter_t* p_meter_info, uint8 old_stats_enable)
{
    fei_openflow_mod_meter_req_t* p_req;
    uint32 req_len;
    int32  rc;

    req_len = sizeof(fei_openflow_mod_meter_req_t);
    p_req = XMALLOC(MEM_PM_OPENFLOW_METER, req_len);
    sal_memcpy(&p_req->meter, p_meter_info, req_len);
    p_req->old_stats_enable = old_stats_enable;

    rc = fei_talk(FEI_OPENFLOW_METER_MOD, p_req, req_len);
    XFREE(MEM_PM_OPENFLOW_METER, p_req);

    return rc;
}

/*************************************************************************************************
 * Name         : fei_meter_delete_meter_entry
 * Purpose      : Fei layer delete meter entry
 * Input        : p_meter_info: glb_openflow_meter_t*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
fei_meter_delete_meter_entry(glb_openflow_meter_t* p_meter_info)
{
    fei_openflow_del_meter_req_t* p_req;
    uint32 req_len;
    int32  rc;

    req_len = sizeof(fei_openflow_del_meter_req_t);
    p_req = XMALLOC(MEM_PM_OPENFLOW_METER, req_len);
    p_req->meter_id = p_meter_info->meter_id;
    p_req->stats_enable = p_meter_info->stats_enable;

    rc = fei_talk(FEI_OPENFLOW_METER_DEL, p_req, req_len);
    XFREE(MEM_PM_OPENFLOW_METER, p_req);

    return rc;
}

/*************************************************************************************************
 * Name         : fei_meter_update_meter_ipg_status
 * Purpose      : Fei layer update meter ipg status
 * Input        : status: glb_openflow_meter_ipg_t*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
fei_meter_update_meter_ipg_status(glb_openflow_meter_ipg_t* p_status)
{
    fei_openflow_mod_meter_ipg_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_mod_meter_ipg_req_t));

    req.enable = p_status->enable;

    return fei_talk(FEI_OPENFLOW_METER_MOD_IPG, &req, sizeof(req));
}

/*************************************************************************************************
 * Name         : fei_meter_clear_all_stats
 * Purpose      : Fei layer clear meter stats
 * Input        : meter_id: uint32
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
fei_meter_clear_all_stats(uint32 meter_id)
{
    fei_openflow_clear_stats_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_clear_stats_req_t));

    req.res_type = FEI_OPENFLOW_STATS_RES_TYPE_METER;
    req.id = meter_id;

    return fei_talk(FEI_OPENFLOW_STATS_CLEAR, &req, sizeof(req));
}
