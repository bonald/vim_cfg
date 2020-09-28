
#ifndef __TBL_PSU_H__
#define __TBL_PSU_H__

int32
tbl_psu_add_psu_sync(tbl_psu_t *p_psu, uint32 sync);

int32
tbl_psu_del_psu_sync(tbl_psu_key_t *p_psu_key, uint32 sync);

int32
tbl_psu_set_psu_field_sync(tbl_psu_t *p_psu, tbl_psu_field_id_t field_id, uint32 sync);

int32
tbl_psu_add_psu(tbl_psu_t *p_psu);

int32
tbl_psu_del_psu(tbl_psu_key_t *p_psu_key);

int32
tbl_psu_set_psu_field(tbl_psu_t *p_psu, tbl_psu_field_id_t field_id);

tbl_psu_t*
tbl_psu_get_psu(tbl_psu_key_t *p_psu_key);

char*
tbl_psu_key_val2str(tbl_psu_t *p_psu, char *str, uint32 str_len);

int32
tbl_psu_key_str2val(char *str, tbl_psu_t *p_psu);

int32
tbl_psu_dump_one(tbl_psu_t *p_psu, tbl_iter_args_t *pargs);

char*
tbl_psu_key_name_dump(tbl_psu_t *p_psu, char* str);

char*
tbl_psu_key_value_dump(tbl_psu_t *p_psu, char* str);

char*
tbl_psu_field_name_dump(tbl_psu_t *p_psu, int32 field_id, char* str);

char*
tbl_psu_field_value_dump(tbl_psu_t *p_psu, int32 field_id, char* str);

char**
tbl_psu_table_dump(tbl_psu_t *p_psu, char** str);

int32
tbl_psu_table_parser(char** array, char* key_value, tbl_psu_t *p_psu);

int32
tbl_psu_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_psu_master_t*
tbl_psu_get_master();

tbl_psu_master_t*
tbl_psu_init_psu();

#endif /* !__TBL_PSU_H__ */

