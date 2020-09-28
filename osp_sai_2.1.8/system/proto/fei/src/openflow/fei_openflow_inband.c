/* Added by weizj for inband */
#include "sal.h"
#include "glb_macro.h"
#include "glb_const.h"
#include "proto.h"
#include "glb_openflow_define.h"
#include "lib.h"

#include "fei.h"

#include "hsrv_msg.h"
#include "hsrv_msg_openflow.h"

#include "fei_openflow_inband.h"


/*************************************************************************************************
 * Name         : fei_inband_update_stag
 * Purpose      : Fei layer inband update stag
 * Input        : p_config: glb_openflow_inband_t*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
fei_inband_update_stag(glb_openflow_inband_t* p_config)
{
    fei_openflow_inband_set_config_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_inband_set_config_req_t));
    sal_memcpy(&req.config, p_config, sizeof(glb_openflow_inband_t));

    return fei_talk(FEI_OPENFLOW_INBAND_SET_CONFIG, &req, sizeof(fei_openflow_inband_set_config_req_t));
}

/*************************************************************************************************
 * Name         : fei_inband_create_vif
 * Purpose      : Fei layer inband create vif
 * Input        : ifindex: uint32, name: char*, gport: uint16
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
fei_inband_create_vif(uint32 ifindex, char* name, uint16 gport)
{
    fei_openflow_inband_vif_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_inband_vif_req_t));
    sal_memcpy(req.name, name, HAL_NAME_LEN);
    req.ifindex = ifindex;
    req.gport = gport;

    return fei_talk(FEI_OPENFLOW_INBAND_VIF_ADD, &req, sizeof(fei_openflow_inband_vif_req_t));
}

/*************************************************************************************************
 * Name         : fei_inband_destroy_vif
 * Purpose      : Fei layer inband destroy vif
 * Input        : ifindex: uint32, name: char*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
fei_inband_destroy_vif(uint32 ifindex, char* name)
{
    fei_openflow_inband_vif_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_inband_vif_req_t));
    sal_memcpy(req.name, name, HAL_NAME_LEN);
    req.ifindex = ifindex;

    return fei_talk(FEI_OPENFLOW_INBAND_VIF_DEL, &req, sizeof(fei_openflow_inband_vif_req_t));
}

