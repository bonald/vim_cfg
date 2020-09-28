
#ifndef __TBL_OPENFLOW_H__
#define __TBL_OPENFLOW_H__

int32
tbl_openflow_set_openflow_field_sync(tbl_openflow_t *p_openflow, tbl_openflow_field_id_t field_id, uint32 sync);

int32
tbl_openflow_set_openflow_field(tbl_openflow_t *p_openflow, tbl_openflow_field_id_t field_id);

tbl_openflow_t*
tbl_openflow_get_openflow();

char*
tbl_openflow_key_val2str(tbl_openflow_t *p_openflow, char *str, uint32 str_len);

int32
tbl_openflow_key_str2val(char *str, tbl_openflow_t *p_openflow);

int32
tbl_openflow_dump_one(tbl_openflow_t *p_openflow, tbl_iter_args_t *pargs);

char*
tbl_openflow_key_name_dump(tbl_openflow_t *p_openflow, char* str);

char*
tbl_openflow_key_value_dump(tbl_openflow_t *p_openflow, char* str);

char*
tbl_openflow_field_name_dump(tbl_openflow_t *p_openflow, int32 field_id, char* str);

char*
tbl_openflow_field_value_dump(tbl_openflow_t *p_openflow, int32 field_id, char* str);

char**
tbl_openflow_table_dump(tbl_openflow_t *p_openflow, char** str);

int32
tbl_openflow_table_parser(char** array, char* key_value, tbl_openflow_t *p_openflow);

int32
tbl_openflow_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_openflow_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_openflow_t*
tbl_openflow_init_openflow();

#endif /* !__TBL_OPENFLOW_H__ */

