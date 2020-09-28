/* Added by weizj for tunnel */
#include "proto.h"
#include "fei.h"

#include "glb_openflow_define.h"
#include "hsrv_msg.h"
#include "hsrv_msg_openflow.h"
#include "fei_openflow_tunnel.h"

/*************************
 * Global and Declaration
 *************************/
extern int32
fei_openflow_port_do_barrier_by_ovs(uint32 ifindex);

static int32 fei_openflow_port_wait_exist_by_ovs(uint32 ifindex)
{
    uint32 cnt = 0;
    while(1)
    {
        if(fei_openflow_port_do_barrier_by_ovs(ifindex))
        {
            return 1;
        }
        cnt ++;
        if(100 == cnt)
        {
            return 0;
        }
        usleep(1000000);
    }

    return 0;
}

#define ____FEI_APIs____
/*************************************************************************************************
 * Name         : fei_openflow_tunnel_add_port
 * Purpose      : Fei layer tunnel add port
 * Input        : p_data: glb_openflow_tunnel_info_t*
 * Output       : N/A 
 * Return       : PM_E_XXX
**************************************************************************************************/
int32
fei_openflow_tunnel_add_port(glb_openflow_tunnel_info_t* p_data)
{
    fei_openflow_add_tunnel_req_t req;
    uint32 req_len;

    if (NULL == p_data)
    {
        return PM_E_INVALID_PARAM;
    }
    if(!p_data->is_dynamic)
    {
        fei_openflow_port_wait_exist_by_ovs(p_data->bond_port_ifindex);
    }
    req_len = sizeof(fei_openflow_add_tunnel_req_t);
    sal_memset(&req, 0, sizeof(fei_openflow_add_tunnel_req_t));
    sal_memcpy(&req.tunnel_info, p_data, sizeof(glb_openflow_tunnel_info_t));

    return fei_talk(FEI_OPENFLOW_TUNNEL_ADD, &req, req_len);
}

/*************************************************************************************************
 * Name         : adpt_common_tunnel_get_type_by_ofport
 * Purpose      : Fei layer tunnel delete port
 * Input        : ifindex: uint32
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
fei_openflow_tunnel_del_port(uint32 ifindex, glb_openflow_tunnel_del_type_t type)
{
    fei_openflow_del_tunnel_req_t req;
    uint32 req_len;

    req_len = sizeof(fei_openflow_del_tunnel_req_t);
    sal_memset(&req, 0, sizeof(fei_openflow_del_tunnel_req_t));
    req.ifindex = ifindex;
    req.tunnel_del_type = type;

    return fei_talk(FEI_OPENFLOW_TUNNEL_DEL, &req, req_len);
}

/*************************************************************************************************
 * Name         : fei_openflow_tunnel_update_decap_mode
 * Purpose      : Fei layer openflow tunnel update decap mode
 * Input        : decap_mode: uint8
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
fei_openflow_tunnel_update_decap_mode(uint8 decap_mode)
{
    fei_openflow_tunnel_decap_mode_req_t req;
    uint32 req_len;

    req_len = sizeof(fei_openflow_tunnel_decap_mode_req_t);
    sal_memset(&req, 0, sizeof(fei_openflow_tunnel_decap_mode_req_t));
    req.decap_mode = decap_mode;

    return fei_talk(FEI_OPENFLOW_TUNNEL_DECAP_MODE_MOD, &req, req_len);
}

/* Added by weizj for tunnel bug 33958 */
/*************************************************************************************************
 * Name         : fei_openflow_tunnel_update_vtep_ip
 * Purpose      : Fei layer openflow tunnel update vtep ip
 * Input        : vtep_ip: uint32
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
fei_openflow_tunnel_update_vtep_ip(uint32 vtep_ip)
{
    fei_openflow_vtep_ip_req_t req;
    uint32 req_len;

    req_len = sizeof(fei_openflow_vtep_ip_req_t);
    sal_memset(&req, 0, sizeof(fei_openflow_vtep_ip_req_t));
    req.ip_addr = vtep_ip;

    return fei_talk(FEI_OPENFLOW_TUNNEL_VTEP_IP_MOD, &req, req_len);
}
/* End by weizj for tunnel */

