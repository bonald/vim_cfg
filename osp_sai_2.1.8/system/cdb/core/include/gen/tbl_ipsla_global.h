
#ifndef __TBL_IPSLA_GLOBAL_H__
#define __TBL_IPSLA_GLOBAL_H__

int32
tbl_ipsla_global_set_ipsla_global_field_sync(tbl_ipsla_global_t *p_ipsla_glb, tbl_ipsla_global_field_id_t field_id, uint32 sync);

int32
tbl_ipsla_global_set_ipsla_global_field(tbl_ipsla_global_t *p_ipsla_glb, tbl_ipsla_global_field_id_t field_id);

tbl_ipsla_global_t*
tbl_ipsla_global_get_ipsla_global();

char*
tbl_ipsla_global_key_val2str(tbl_ipsla_global_t *p_ipsla_glb, char *str, uint32 str_len);

int32
tbl_ipsla_global_key_str2val(char *str, tbl_ipsla_global_t *p_ipsla_glb);

int32
tbl_ipsla_global_dump_one(tbl_ipsla_global_t *p_ipsla_glb, tbl_iter_args_t *pargs);

char*
tbl_ipsla_global_key_name_dump(tbl_ipsla_global_t *p_ipsla_glb, char* str);

char*
tbl_ipsla_global_key_value_dump(tbl_ipsla_global_t *p_ipsla_glb, char* str);

char*
tbl_ipsla_global_field_name_dump(tbl_ipsla_global_t *p_ipsla_glb, int32 field_id, char* str);

char*
tbl_ipsla_global_field_value_dump(tbl_ipsla_global_t *p_ipsla_glb, int32 field_id, char* str);

char**
tbl_ipsla_global_table_dump(tbl_ipsla_global_t *p_ipsla_glb, char** str);

int32
tbl_ipsla_global_table_parser(char** array, char* key_value, tbl_ipsla_global_t *p_ipsla_glb);

int32
tbl_ipsla_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_ipsla_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ipsla_global_t*
tbl_ipsla_global_init_ipsla_global();

#endif /* !__TBL_IPSLA_GLOBAL_H__ */

