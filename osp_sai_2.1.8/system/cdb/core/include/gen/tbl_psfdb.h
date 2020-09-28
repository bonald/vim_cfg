
#ifndef __TBL_PSFDB_H__
#define __TBL_PSFDB_H__

int32
tbl_psfdb_add_psfdb_sync(tbl_psfdb_t *p_psfdb, uint32 sync);

int32
tbl_psfdb_del_psfdb_sync(tbl_psfdb_key_t *p_psfdb_key, uint32 sync);

int32
tbl_psfdb_set_psfdb_field_sync(tbl_psfdb_t *p_psfdb, tbl_psfdb_field_id_t field_id, uint32 sync);

int32
tbl_psfdb_add_psfdb(tbl_psfdb_t *p_psfdb);

int32
tbl_psfdb_del_psfdb(tbl_psfdb_key_t *p_psfdb_key);

int32
tbl_psfdb_set_psfdb_field(tbl_psfdb_t *p_psfdb, tbl_psfdb_field_id_t field_id);

tbl_psfdb_t*
tbl_psfdb_get_psfdb(tbl_psfdb_key_t *p_psfdb_key);

char*
tbl_psfdb_key_val2str(tbl_psfdb_t *p_psfdb, char *str, uint32 str_len);

int32
tbl_psfdb_key_str2val(char *str, tbl_psfdb_t *p_psfdb);

int32
tbl_psfdb_dump_one(tbl_psfdb_t *p_psfdb, tbl_iter_args_t *pargs);

char*
tbl_psfdb_key_name_dump(tbl_psfdb_t *p_psfdb, char* str);

char*
tbl_psfdb_key_value_dump(tbl_psfdb_t *p_psfdb, char* str);

char*
tbl_psfdb_field_name_dump(tbl_psfdb_t *p_psfdb, int32 field_id, char* str);

char*
tbl_psfdb_field_value_dump(tbl_psfdb_t *p_psfdb, int32 field_id, char* str);

char**
tbl_psfdb_table_dump(tbl_psfdb_t *p_psfdb, char** str);

int32
tbl_psfdb_table_parser(char** array, char* key_value, tbl_psfdb_t *p_psfdb);

int32
tbl_psfdb_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_psfdb_master_t*
tbl_psfdb_get_master();

tbl_psfdb_master_t*
tbl_psfdb_init_psfdb();

#endif /* !__TBL_PSFDB_H__ */

