
#ifndef __TBL_ERPS_DOMAIN_H__
#define __TBL_ERPS_DOMAIN_H__

int32
tbl_erps_domain_add_erps_domain_sync(tbl_erps_domain_t *p_erps_domain, uint32 sync);

int32
tbl_erps_domain_del_erps_domain_sync(tbl_erps_domain_key_t *p_erps_domain_key, uint32 sync);

int32
tbl_erps_domain_set_erps_domain_field_sync(tbl_erps_domain_t *p_erps_domain, tbl_erps_domain_field_id_t field_id, uint32 sync);

int32
tbl_erps_domain_add_erps_domain(tbl_erps_domain_t *p_erps_domain);

int32
tbl_erps_domain_del_erps_domain(tbl_erps_domain_key_t *p_erps_domain_key);

int32
tbl_erps_domain_set_erps_domain_field(tbl_erps_domain_t *p_erps_domain, tbl_erps_domain_field_id_t field_id);

tbl_erps_domain_t*
tbl_erps_domain_get_erps_domain(tbl_erps_domain_key_t *p_erps_domain_key);

char*
tbl_erps_domain_key_val2str(tbl_erps_domain_t *p_erps_domain, char *str, uint32 str_len);

int32
tbl_erps_domain_key_str2val(char *str, tbl_erps_domain_t *p_erps_domain);

int32
tbl_erps_domain_dump_one(tbl_erps_domain_t *p_erps_domain, tbl_iter_args_t *pargs);

char*
tbl_erps_domain_key_name_dump(tbl_erps_domain_t *p_erps_domain, char* str);

char*
tbl_erps_domain_key_value_dump(tbl_erps_domain_t *p_erps_domain, char* str);

char*
tbl_erps_domain_field_name_dump(tbl_erps_domain_t *p_erps_domain, int32 field_id, char* str);

char*
tbl_erps_domain_field_value_dump(tbl_erps_domain_t *p_erps_domain, int32 field_id, char* str);

char**
tbl_erps_domain_table_dump(tbl_erps_domain_t *p_erps_domain, char** str);

int32
tbl_erps_domain_table_parser(char** array, char* key_value, tbl_erps_domain_t *p_erps_domain);

int32
tbl_erps_domain_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_erps_domain_master_t*
tbl_erps_domain_get_master();

tbl_erps_domain_master_t*
tbl_erps_domain_init_erps_domain();

int32
tbl_erps_domain_set_erps_ring_list_sync(tbl_erps_domain_t *p_erps_domain, uint32 add, void *p_object, uint32 sync);

int32
tbl_erps_domain_set_erps_ring_list(tbl_erps_domain_t *p_erps_domain, uint32 add, void *p_object);

#endif /* !__TBL_ERPS_DOMAIN_H__ */

