
#ifndef __TBL_NAT_POOL_H__
#define __TBL_NAT_POOL_H__

int32
tbl_nat_pool_add_nat_pool_sync(tbl_nat_pool_t *p_nat_pool, uint32 sync);

int32
tbl_nat_pool_del_nat_pool_sync(tbl_nat_pool_key_t *p_nat_pool_key, uint32 sync);

int32
tbl_nat_pool_set_nat_pool_field_sync(tbl_nat_pool_t *p_nat_pool, tbl_nat_pool_field_id_t field_id, uint32 sync);

int32
tbl_nat_pool_add_nat_pool(tbl_nat_pool_t *p_nat_pool);

int32
tbl_nat_pool_del_nat_pool(tbl_nat_pool_key_t *p_nat_pool_key);

int32
tbl_nat_pool_set_nat_pool_field(tbl_nat_pool_t *p_nat_pool, tbl_nat_pool_field_id_t field_id);

tbl_nat_pool_t*
tbl_nat_pool_get_nat_pool(tbl_nat_pool_key_t *p_nat_pool_key);

char*
tbl_nat_pool_key_val2str(tbl_nat_pool_t *p_nat_pool, char *str, uint32 str_len);

int32
tbl_nat_pool_key_str2val(char *str, tbl_nat_pool_t *p_nat_pool);

int32
tbl_nat_pool_dump_one(tbl_nat_pool_t *p_nat_pool, tbl_iter_args_t *pargs);

char*
tbl_nat_pool_key_name_dump(tbl_nat_pool_t *p_nat_pool, char* str);

char*
tbl_nat_pool_key_value_dump(tbl_nat_pool_t *p_nat_pool, char* str);

char*
tbl_nat_pool_field_name_dump(tbl_nat_pool_t *p_nat_pool, int32 field_id, char* str);

char*
tbl_nat_pool_field_value_dump(tbl_nat_pool_t *p_nat_pool, int32 field_id, char* str);

char**
tbl_nat_pool_table_dump(tbl_nat_pool_t *p_nat_pool, char** str);

int32
tbl_nat_pool_table_parser(char** array, char* key_value, tbl_nat_pool_t *p_nat_pool);

int32
tbl_nat_pool_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_nat_pool_master_t*
tbl_nat_pool_get_master();

tbl_nat_pool_master_t*
tbl_nat_pool_init_nat_pool();

#endif /* !__TBL_NAT_POOL_H__ */

