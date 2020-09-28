
#ifndef __HAL_API_VLAN_H__
#define __HAL_API_VLAN_H__

int32
fei_api_vlan_add(fei_vlan_add_req_t *pst_req);

int32
fei_api_vlan_delete(fei_vlan_del_req_t *pst_req);

int32
fei_api_vlan_set_en(fei_vlan_set_enable_req_t *pst_req);

int32
fei_api_vlan_set_instance(fei_vlan_set_instance_req_t *pst_req);

int32
fei_api_vlan_add_vid_to_port(fei_vlan_add_vid_to_port_req_t *pst_req);

int32
fei_api_vlan_delete_vid_from_port(fei_vlan_del_vid_from_port_req_t *pst_req);

int32
fei_api_vlan_set_mac_learn_en(fei_vlan_set_mac_learn_en_req_t *pst_req);

#endif /* !__HAL_API_VLAN_H__ */

