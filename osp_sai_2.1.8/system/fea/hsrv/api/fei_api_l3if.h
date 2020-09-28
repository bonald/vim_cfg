
#ifndef __HAL_API_L3IF_H__
#define __HAL_API_L3IF_H__

int32
fei_api_l3if_set_ip_en(fei_l3if_ip_en_req_t *pst_req);

int32
fei_api_l3if_set_vrfid(fei_l3if_bind_fib_req_t *pst_req);

int32
fei_api_l3if_set_mtu(fei_l3if_mtu_req_t *pst_req);

int32
fei_api_l3if_create_vlan_if(fei_l3if_svi_create_req_t *pst_req, uint32 *p_kernel_ifindex);

int32
fei_api_l3if_destroy_vlanif(fei_l3if_svi_delete_req_t *pst_req);

int32
fei_api_l3if_create_loopback_if(fei_l3if_loopback_if_create_req_t *pst_req, uint32 *p_kernel_ifindex);

int32
fei_api_l3if_destroy_loopback_if(fei_l3if_loopback_if_op_t *pst_req);

int32
fei_api_l3if_set_route_mac(fei_l3if_routemac_req_t *pst_req);

#endif /* !__HAL_API_L3IF_H__ */

