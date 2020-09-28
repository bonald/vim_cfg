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


/*************************************************************************************************
 * Name         : fei_queue_add_queue_shape
 * Purpose      : Fei layer add meter entry
 * Input        : p_meter_info: glb_openflow_meter_t*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
fei_queue_add_queue_shape(glb_openflow_queue_t* p_queue_info)
{
    fei_openflow_add_queue_shape_req_t* p_req;
    uint32 req_len;
    int32  rc;

    req_len = sizeof(fei_openflow_add_queue_shape_req_t);
    p_req = XMALLOC(MEM_PM_OPENFLOW_COMMON, req_len);
    sal_memcpy(&p_req->queue, p_queue_info, req_len);

    rc = fei_talk(FEI_OPENFLOW_QUEUE_SHAPE_ADD, p_req, req_len);
    XFREE(MEM_PM_OPENFLOW_COMMON, p_req);

    return rc;
}
