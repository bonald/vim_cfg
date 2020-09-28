
#ifndef __TBL_RIF_H__
#define __TBL_RIF_H__

int32
tbl_rif_add_rif_sync(tbl_rif_t *p_rif, uint32 sync);

int32
tbl_rif_del_rif_sync(tbl_rif_key_t *p_rif_key, uint32 sync);

int32
tbl_rif_set_rif_field_sync(tbl_rif_t *p_rif, tbl_rif_field_id_t field_id, uint32 sync);

int32
tbl_rif_add_rif(tbl_rif_t *p_rif);

int32
tbl_rif_del_rif(tbl_rif_key_t *p_rif_key);

int32
tbl_rif_set_rif_field(tbl_rif_t *p_rif, tbl_rif_field_id_t field_id);

tbl_rif_t*
tbl_rif_get_rif(tbl_rif_key_t *p_rif_key);

char*
tbl_rif_key_val2str(tbl_rif_t *p_rif, char *str, uint32 str_len);

int32
tbl_rif_key_str2val(char *str, tbl_rif_t *p_rif);

int32
tbl_rif_dump_one(tbl_rif_t *p_rif, tbl_iter_args_t *pargs);

char*
tbl_rif_key_name_dump(tbl_rif_t *p_rif, char* str);

char*
tbl_rif_key_value_dump(tbl_rif_t *p_rif, char* str);

char*
tbl_rif_field_name_dump(tbl_rif_t *p_rif, int32 field_id, char* str);

char*
tbl_rif_field_value_dump(tbl_rif_t *p_rif, int32 field_id, char* str);

char**
tbl_rif_table_dump(tbl_rif_t *p_rif, char** str);

int32
tbl_rif_table_parser(char** array, char* key_value, tbl_rif_t *p_rif);

int32
tbl_rif_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_rif_master_t*
tbl_rif_get_master();

tbl_rif_master_t*
tbl_rif_init_rif();

#endif /* !__TBL_RIF_H__ */

