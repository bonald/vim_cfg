
#ifndef __TBL_FEA_FDB_H__
#define __TBL_FEA_FDB_H__

int32
tbl_fea_fdb_add_fea_fdb_sync(tbl_fea_fdb_t *p_fdb, uint32 sync);

int32
tbl_fea_fdb_del_fea_fdb_sync(tbl_fea_fdb_key_t *p_fdb_key, uint32 sync);

int32
tbl_fea_fdb_set_fea_fdb_field_sync(tbl_fea_fdb_t *p_fdb, tbl_fea_fdb_field_id_t field_id, uint32 sync);

int32
tbl_fea_fdb_add_fea_fdb(tbl_fea_fdb_t *p_fdb);

int32
tbl_fea_fdb_del_fea_fdb(tbl_fea_fdb_key_t *p_fdb_key);

int32
tbl_fea_fdb_set_fea_fdb_field(tbl_fea_fdb_t *p_fdb, tbl_fea_fdb_field_id_t field_id);

tbl_fea_fdb_t*
tbl_fea_fdb_get_fea_fdb(tbl_fea_fdb_key_t *p_fdb_key);

char*
tbl_fea_fdb_key_val2str(tbl_fea_fdb_t *p_fdb, char *str, uint32 str_len);

int32
tbl_fea_fdb_key_str2val(char *str, tbl_fea_fdb_t *p_fdb);

int32
tbl_fea_fdb_dump_one(tbl_fea_fdb_t *p_fdb, tbl_iter_args_t *pargs);

char*
tbl_fea_fdb_key_name_dump(tbl_fea_fdb_t *p_fdb, char* str);

char*
tbl_fea_fdb_key_value_dump(tbl_fea_fdb_t *p_fdb, char* str);

char*
tbl_fea_fdb_field_name_dump(tbl_fea_fdb_t *p_fdb, int32 field_id, char* str);

char*
tbl_fea_fdb_field_value_dump(tbl_fea_fdb_t *p_fdb, int32 field_id, char* str);

char**
tbl_fea_fdb_table_dump(tbl_fea_fdb_t *p_fdb, char** str);

int32
tbl_fea_fdb_table_parser(char** array, char* key_value, tbl_fea_fdb_t *p_fdb);

int32
tbl_fea_fdb_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fea_fdb_master_t*
tbl_fea_fdb_get_master();

tbl_fea_fdb_master_t*
tbl_fea_fdb_init_fea_fdb();

#endif /* !__TBL_FEA_FDB_H__ */

