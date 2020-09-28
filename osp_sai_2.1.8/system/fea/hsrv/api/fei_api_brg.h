
#ifndef __HAL_API_BRG_H__
#define __HAL_API_BRG_H__

int32
fei_api_brg_add_fdb(fei_brg_add_fdb_req_t *pst_req);

int32
fei_api_brg_del_fdb(fei_brg_del_fdb_req_t *pst_req);

int32
fei_api_brg_add_macfilter(fei_brg_add_macfilter_req_t *pst_req);

int32
fei_api_brg_del_macfilter(fei_brg_del_macfilter_req_t *pst_req);

int32
fei_api_brg_set_aging_time(fei_brg_set_aging_time_t *pst_req);

int32
fei_api_brg_flush_fdb_port(fei_brg_flush_fdb_port_req_t *pst_req);

int32
fei_api_brg_flush_fdb_mac(fei_brg_flush_fdb_mac_req_t *pst_req);

int32
fei_api_brg_set_port_state(fei_brg_set_port_state_req_t *pst_req);

int32
fei_api_brg_set_hw_learning_en(uint32 *pst_req);

#endif /* !__HAL_API_BRG_H__ */
