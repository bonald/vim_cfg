
#ifndef __TBL_VCM_H__
#define __TBL_VCM_H__

int32
tbl_vcm_add_vcm_sync(tbl_vcm_t *p_vcm, uint32 sync);

int32
tbl_vcm_del_vcm_sync(tbl_vcm_key_t *p_vcm_key, uint32 sync);

int32
tbl_vcm_set_vcm_field_sync(tbl_vcm_t *p_vcm, tbl_vcm_field_id_t field_id, uint32 sync);

int32
tbl_vcm_add_vcm(tbl_vcm_t *p_vcm);

int32
tbl_vcm_del_vcm(tbl_vcm_key_t *p_vcm_key);

int32
tbl_vcm_set_vcm_field(tbl_vcm_t *p_vcm, tbl_vcm_field_id_t field_id);

tbl_vcm_t*
tbl_vcm_get_vcm(tbl_vcm_key_t *p_vcm_key);

char*
tbl_vcm_key_val2str(tbl_vcm_t *p_vcm, char *str, uint32 str_len);

int32
tbl_vcm_key_str2val(char *str, tbl_vcm_t *p_vcm);

int32
tbl_vcm_dump_one(tbl_vcm_t *p_vcm, tbl_iter_args_t *pargs);

char*
tbl_vcm_key_name_dump(tbl_vcm_t *p_vcm, char* str);

char*
tbl_vcm_key_value_dump(tbl_vcm_t *p_vcm, char* str);

char*
tbl_vcm_field_name_dump(tbl_vcm_t *p_vcm, int32 field_id, char* str);

char*
tbl_vcm_field_value_dump(tbl_vcm_t *p_vcm, int32 field_id, char* str);

char**
tbl_vcm_table_dump(tbl_vcm_t *p_vcm, char** str);

int32
tbl_vcm_table_parser(char** array, char* key_value, tbl_vcm_t *p_vcm);

int32
tbl_vcm_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_vcm_master_t*
tbl_vcm_get_master();

tbl_vcm_master_t*
tbl_vcm_init_vcm();

#endif /* !__TBL_VCM_H__ */

