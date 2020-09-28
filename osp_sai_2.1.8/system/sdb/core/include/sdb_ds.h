
#ifndef __SDB_DS_H__
#define __SDB_DS_H__

//#include <assert.h>
#include "sdb_include.h"

#define SDB_MAX_CMD_STR_LEN         1024 * 32

#define SDB_FLD_MAX_DEPTH   2
#define SDB_FLD_ID_ALL      (-1)

#define SDB_FLD_MATCH(_fld_id, _id)  ((_fld_id == _id) || (SDB_FLD_ID_ALL == _id))

extern const char *sai_packet_action_strs[];

char*
sdb_bitmap_val2str(char *str, uint32 str_len, const uint32 *bmp, const uint32 bmp_len);
char*
sdb_mac_addr_val2str(char *str, uint32 str_len, const uint8 *mac);
char*
sdb_ipuc_val2str(char *str, uint32 str_len, const sai_unicast_route_entry_t *ipuc);
char*
sdb_l2mc_val2str(char *str, uint32 str_len, const sai_l2mc_entry_t *l2mc);
char*
sdb_ipmc_val2str(char *str, uint32 str_len, const sai_ipmc_entry_t *ipmc);
char*
sdb_neighbor_val2str(char *str, uint32 str_len, const sai_neighbor_entry_t *neighbor);
char*
sdb_ipaddr_val2str(char *str, uint32 str_len, const sai_ip_address_t *ip);
char*
sdb_addr_val2str(char *str, uint32 str_len, const addr_t *addr);
char*
sdb_sai_acl_field_val2str(char *str, uint32 str_len, const sai_acl_field_data_t *acl_field);
char*
sdb_sai_acl_tap_val2str(char *str, uint32 str_len, const ctc_sai_acl_tap_t *tap);
char*
sdb_sai_acl_field_list_val2str(char *str, uint32 str_len, const ctc_sai_acl_field_list_t *list);
char*
sdb_sai_acl_action_list_val2str(char *str, uint32 str_len, const ctc_sai_acl_action_list_t *list);
char*
sdb_sai_acl_action_attrlist_val2str(char *str, uint32 str_len, const ctc_sai_acl_action_attrlist_t *list);
char*
sdb_sai_acl_tunnel_key_val2str(char *str, uint32 str_len, const ctc_sai_acl_tunnel_key_t *key);
char*
sdb_uint8_array_val2str(char *str, uint32 str_len, const uint8 *array, uint32 len);
char*
sdb_uint32_array_val2str(char *str, uint32 str_len, const uint32 *array, uint32 len);
char*
sdb_uint64_array_val2str(char *str, uint32 str_len, const uint64 *array, uint32 len);
char*
sdb_opb_val2str(char *str, uint32 str_len, const ctclib_opb_t *opb);
char*
sdb_object_id_val2str(char *str, uint32 str_len, const sai_object_id_t *oid);
char*
sdb_mac_rx_stats_ptr_val2str(char *str, uint32 str_len, const sdb_mac_rx_stats_ptr_t *rx_ptr);
char*
sdb_mac_tx_stats_ptr_val2str(char *str, uint32 str_len, const sdb_mac_tx_stats_ptr_t *tx_ptr);

char*
sdb_reference_list_val2str(char *str, uint32 str_len, sdb_reference_list_t *ref_list);
int32
sdb_reference_list_init(sdb_reference_list_t *ref_list, uint16 type, uint16 id, CTCLIB_SLIST_CMP_FN cmp);
int32
sdb_reference_list_deinit(sdb_reference_list_t *ref_list);
int32
sdb_reference_list_add_object(sdb_reference_list_t *ref_list, void *p_object);
int32
sdb_reference_list_del_object(sdb_reference_list_t *ref_list, void *p_object);

typedef char* (*SDB_MAC_RX_VAL2STR_CB_FUNC)(char *str, uint32 str_len, const sdb_mac_rx_stats_ptr_t *rx_ptr);
typedef char* (*SDB_MAC_TX_VAL2STR_CB_FUNC)(char *str, uint32 str_len, const sdb_mac_tx_stats_ptr_t *tx_ptr);
int32
sdb_mac_rx_stats_ptr_val2str_register_fn(SDB_MAC_RX_VAL2STR_CB_FUNC fn);
int32
sdb_mac_tx_stats_ptr_val2str_register_fn(SDB_MAC_TX_VAL2STR_CB_FUNC fn);

char*
sdb_nat_forward_val2str(char *str, uint32 str_len, const sai_nat_forward_entry_t *p_data);


#endif /* !__SDB_DS_H__ */

