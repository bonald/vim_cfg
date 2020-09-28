
#ifndef __DS_CFM_ERROR_H__
#define __DS_CFM_ERROR_H__

int32
ds_cfm_error_add_cfm_error_sync(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, uint32 sync);

int32
ds_cfm_error_del_cfm_error_sync(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, uint32 sync);

int32
ds_cfm_error_set_cfm_error_field_sync(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, ds_cfm_error_field_id_t field_id, uint32 sync);

int32
ds_cfm_error_add_cfm_error(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error);

int32
ds_cfm_error_del_cfm_error(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error);

int32
ds_cfm_error_set_cfm_error_field(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, ds_cfm_error_field_id_t field_id);

ds_cfm_error_t*
ds_cfm_error_get_cfm_error(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error);

int32
ds_cfm_error_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error);

char*
ds_cfm_error_key_name_dump(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, char* str);

char*
ds_cfm_error_key_value_dump(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, char* str);

char*
ds_cfm_error_field_name_dump(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, int32 field_id, char* str);

char*
ds_cfm_error_field_value_dump(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, int32 field_id, char* str);

char**
ds_cfm_error_table_dump(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, char** str);

int32
ds_cfm_error_field_value_parser(char* str, int32 field_id, tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error);

int32
ds_cfm_error_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error);

int32
ds_cfm_error_dump_one(ds_cfm_error_t *p_cfm_error, tbl_iter_args_t *pargs);

#endif /* !__DS_CFM_ERROR_H__ */

