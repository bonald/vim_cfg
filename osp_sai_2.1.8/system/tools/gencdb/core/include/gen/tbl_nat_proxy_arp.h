
#ifndef __TBL_NAT_PROXY_ARP_H__
#define __TBL_NAT_PROXY_ARP_H__

int32
tbl_nat_proxy_arp_add_nat_proxy_arp_sync(tbl_nat_proxy_arp_t *p_nat_proxy_arp, uint32 sync);

int32
tbl_nat_proxy_arp_del_nat_proxy_arp_sync(tbl_nat_proxy_arp_key_t *p_nat_proxy_arp_key, uint32 sync);

int32
tbl_nat_proxy_arp_set_nat_proxy_arp_field_sync(tbl_nat_proxy_arp_t *p_nat_proxy_arp, tbl_nat_proxy_arp_field_id_t field_id, uint32 sync);

int32
tbl_nat_proxy_arp_add_nat_proxy_arp(tbl_nat_proxy_arp_t *p_nat_proxy_arp);

int32
tbl_nat_proxy_arp_del_nat_proxy_arp(tbl_nat_proxy_arp_key_t *p_nat_proxy_arp_key);

int32
tbl_nat_proxy_arp_set_nat_proxy_arp_field(tbl_nat_proxy_arp_t *p_nat_proxy_arp, tbl_nat_proxy_arp_field_id_t field_id);

tbl_nat_proxy_arp_t*
tbl_nat_proxy_arp_get_nat_proxy_arp(tbl_nat_proxy_arp_key_t *p_nat_proxy_arp_key);

char*
tbl_nat_proxy_arp_key_val2str(tbl_nat_proxy_arp_t *p_nat_proxy_arp, char *str, uint32 str_len);

int32
tbl_nat_proxy_arp_key_str2val(char *str, tbl_nat_proxy_arp_t *p_nat_proxy_arp);

int32
tbl_nat_proxy_arp_dump_one(tbl_nat_proxy_arp_t *p_nat_proxy_arp, tbl_iter_args_t *pargs);

char*
tbl_nat_proxy_arp_key_name_dump(tbl_nat_proxy_arp_t *p_nat_proxy_arp, char* str);

char*
tbl_nat_proxy_arp_key_value_dump(tbl_nat_proxy_arp_t *p_nat_proxy_arp, char* str);

char*
tbl_nat_proxy_arp_field_name_dump(tbl_nat_proxy_arp_t *p_nat_proxy_arp, int32 field_id, char* str);

char*
tbl_nat_proxy_arp_field_value_dump(tbl_nat_proxy_arp_t *p_nat_proxy_arp, int32 field_id, char* str);

char**
tbl_nat_proxy_arp_table_dump(tbl_nat_proxy_arp_t *p_nat_proxy_arp, char** str);

int32
tbl_nat_proxy_arp_table_parser(char** array, char* key_value, tbl_nat_proxy_arp_t *p_nat_proxy_arp);

int32
tbl_nat_proxy_arp_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_nat_proxy_arp_master_t*
tbl_nat_proxy_arp_get_master();

tbl_nat_proxy_arp_master_t*
tbl_nat_proxy_arp_init_nat_proxy_arp();

#endif /* !__TBL_NAT_PROXY_ARP_H__ */

