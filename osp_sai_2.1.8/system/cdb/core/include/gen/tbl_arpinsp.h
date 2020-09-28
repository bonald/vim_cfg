
#ifndef __TBL_ARPINSP_H__
#define __TBL_ARPINSP_H__

int32
tbl_arpinsp_add_arpinsp_sync(tbl_arpinsp_t *p_arpinsp, uint32 sync);

int32
tbl_arpinsp_del_arpinsp_sync(tbl_arpinsp_key_t *p_arpinsp_key, uint32 sync);

int32
tbl_arpinsp_set_arpinsp_field_sync(tbl_arpinsp_t *p_arpinsp, tbl_arpinsp_field_id_t field_id, uint32 sync);

int32
tbl_arpinsp_add_arpinsp(tbl_arpinsp_t *p_arpinsp);

int32
tbl_arpinsp_del_arpinsp(tbl_arpinsp_key_t *p_arpinsp_key);

int32
tbl_arpinsp_set_arpinsp_field(tbl_arpinsp_t *p_arpinsp, tbl_arpinsp_field_id_t field_id);

tbl_arpinsp_t*
tbl_arpinsp_get_arpinsp(tbl_arpinsp_key_t *p_arpinsp_key);

char*
tbl_arpinsp_key_val2str(tbl_arpinsp_t *p_arpinsp, char *str, uint32 str_len);

int32
tbl_arpinsp_key_str2val(char *str, tbl_arpinsp_t *p_arpinsp);

int32
tbl_arpinsp_dump_one(tbl_arpinsp_t *p_arpinsp, tbl_iter_args_t *pargs);

char*
tbl_arpinsp_key_name_dump(tbl_arpinsp_t *p_arpinsp, char* str);

char*
tbl_arpinsp_key_value_dump(tbl_arpinsp_t *p_arpinsp, char* str);

char*
tbl_arpinsp_field_name_dump(tbl_arpinsp_t *p_arpinsp, int32 field_id, char* str);

char*
tbl_arpinsp_field_value_dump(tbl_arpinsp_t *p_arpinsp, int32 field_id, char* str);

char**
tbl_arpinsp_table_dump(tbl_arpinsp_t *p_arpinsp, char** str);

int32
tbl_arpinsp_table_parser(char** array, char* key_value, tbl_arpinsp_t *p_arpinsp);

int32
tbl_arpinsp_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_arpinsp_master_t*
tbl_arpinsp_get_master();

tbl_arpinsp_master_t*
tbl_arpinsp_init_arpinsp();

#endif /* !__TBL_ARPINSP_H__ */

