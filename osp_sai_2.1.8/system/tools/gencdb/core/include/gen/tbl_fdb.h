
#ifndef __TBL_FDB_H__
#define __TBL_FDB_H__

int32
tbl_fdb_add_fdb_sync(tbl_fdb_t *p_fdb, uint32 sync);

int32
tbl_fdb_del_fdb_sync(tbl_fdb_key_t *p_fdb_key, uint32 sync);

int32
tbl_fdb_set_fdb_field_sync(tbl_fdb_t *p_fdb, tbl_fdb_field_id_t field_id, uint32 sync);

int32
tbl_fdb_add_fdb(tbl_fdb_t *p_fdb);

int32
tbl_fdb_del_fdb(tbl_fdb_key_t *p_fdb_key);

int32
tbl_fdb_set_fdb_field(tbl_fdb_t *p_fdb, tbl_fdb_field_id_t field_id);

tbl_fdb_t*
tbl_fdb_get_fdb(tbl_fdb_key_t *p_fdb_key);

char*
tbl_fdb_key_val2str(tbl_fdb_t *p_fdb, char *str, uint32 str_len);

int32
tbl_fdb_key_str2val(char *str, tbl_fdb_t *p_fdb);

int32
tbl_fdb_dump_one(tbl_fdb_t *p_fdb, tbl_iter_args_t *pargs);

char*
tbl_fdb_key_name_dump(tbl_fdb_t *p_fdb, char* str);

char*
tbl_fdb_key_value_dump(tbl_fdb_t *p_fdb, char* str);

char*
tbl_fdb_field_name_dump(tbl_fdb_t *p_fdb, int32 field_id, char* str);

char*
tbl_fdb_field_value_dump(tbl_fdb_t *p_fdb, int32 field_id, char* str);

char**
tbl_fdb_table_dump(tbl_fdb_t *p_fdb, char** str);

int32
tbl_fdb_table_parser(char** array, char* key_value, tbl_fdb_t *p_fdb);

int32
tbl_fdb_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fdb_master_t*
tbl_fdb_get_master();

tbl_fdb_master_t*
tbl_fdb_init_fdb();

#endif /* !__TBL_FDB_H__ */

