
#ifndef __TBL_POE_H__
#define __TBL_POE_H__

int32
tbl_poe_add_poe_sync(tbl_poe_t *p_poe, uint32 sync);

int32
tbl_poe_del_poe_sync(tbl_poe_key_t *p_poe_key, uint32 sync);

int32
tbl_poe_set_poe_field_sync(tbl_poe_t *p_poe, tbl_poe_field_id_t field_id, uint32 sync);

int32
tbl_poe_add_poe(tbl_poe_t *p_poe);

int32
tbl_poe_del_poe(tbl_poe_key_t *p_poe_key);

int32
tbl_poe_set_poe_field(tbl_poe_t *p_poe, tbl_poe_field_id_t field_id);

tbl_poe_t*
tbl_poe_get_poe(tbl_poe_key_t *p_poe_key);

char*
tbl_poe_key_val2str(tbl_poe_t *p_poe, char *str, uint32 str_len);

int32
tbl_poe_key_str2val(char *str, tbl_poe_t *p_poe);

int32
tbl_poe_dump_one(tbl_poe_t *p_poe, tbl_iter_args_t *pargs);

char*
tbl_poe_key_name_dump(tbl_poe_t *p_poe, char* str);

char*
tbl_poe_key_value_dump(tbl_poe_t *p_poe, char* str);

char*
tbl_poe_field_name_dump(tbl_poe_t *p_poe, int32 field_id, char* str);

char*
tbl_poe_field_value_dump(tbl_poe_t *p_poe, int32 field_id, char* str);

char**
tbl_poe_table_dump(tbl_poe_t *p_poe, char** str);

int32
tbl_poe_table_parser(char** array, char* key_value, tbl_poe_t *p_poe);

int32
tbl_poe_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_poe_master_t*
tbl_poe_get_master();

tbl_poe_master_t*
tbl_poe_init_poe();

#endif /* !__TBL_POE_H__ */

