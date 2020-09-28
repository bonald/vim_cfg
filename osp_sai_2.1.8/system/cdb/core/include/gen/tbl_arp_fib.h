
#ifndef __TBL_ARP_FIB_H__
#define __TBL_ARP_FIB_H__

int32
tbl_arp_fib_add_arp_fib_sync(tbl_arp_fib_t *p_arp_fib, uint32 sync);

int32
tbl_arp_fib_del_arp_fib_sync(tbl_arp_fib_key_t *p_arp_fib_key, uint32 sync);

int32
tbl_arp_fib_set_arp_fib_field_sync(tbl_arp_fib_t *p_arp_fib, tbl_arp_fib_field_id_t field_id, uint32 sync);

int32
tbl_arp_fib_add_arp_fib(tbl_arp_fib_t *p_arp_fib);

int32
tbl_arp_fib_del_arp_fib(tbl_arp_fib_key_t *p_arp_fib_key);

int32
tbl_arp_fib_set_arp_fib_field(tbl_arp_fib_t *p_arp_fib, tbl_arp_fib_field_id_t field_id);

tbl_arp_fib_t*
tbl_arp_fib_get_arp_fib(tbl_arp_fib_key_t *p_arp_fib_key);

char*
tbl_arp_fib_key_val2str(tbl_arp_fib_t *p_arp_fib, char *str, uint32 str_len);

int32
tbl_arp_fib_key_str2val(char *str, tbl_arp_fib_t *p_arp_fib);

int32
tbl_arp_fib_dump_one(tbl_arp_fib_t *p_arp_fib, tbl_iter_args_t *pargs);

char*
tbl_arp_fib_key_name_dump(tbl_arp_fib_t *p_arp_fib, char* str);

char*
tbl_arp_fib_key_value_dump(tbl_arp_fib_t *p_arp_fib, char* str);

char*
tbl_arp_fib_field_name_dump(tbl_arp_fib_t *p_arp_fib, int32 field_id, char* str);

char*
tbl_arp_fib_field_value_dump(tbl_arp_fib_t *p_arp_fib, int32 field_id, char* str);

char**
tbl_arp_fib_table_dump(tbl_arp_fib_t *p_arp_fib, char** str);

int32
tbl_arp_fib_table_parser(char** array, char* key_value, tbl_arp_fib_t *p_arp_fib);

int32
tbl_arp_fib_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_arp_fib_master_t*
tbl_arp_fib_get_master();

tbl_arp_fib_master_t*
tbl_arp_fib_init_arp_fib();

#endif /* !__TBL_ARP_FIB_H__ */

