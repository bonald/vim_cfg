
#ifndef __TBL_MCFDB_H__
#define __TBL_MCFDB_H__

int32
tbl_mcfdb_add_mcfdb_sync(tbl_mcfdb_t *p_mcfdb, uint32 sync);

int32
tbl_mcfdb_del_mcfdb_sync(tbl_mcfdb_key_t *p_mcfdb_key, uint32 sync);

int32
tbl_mcfdb_set_mcfdb_field_sync(tbl_mcfdb_t *p_mcfdb, tbl_mcfdb_field_id_t field_id, uint32 sync);

int32
tbl_mcfdb_add_mcfdb(tbl_mcfdb_t *p_mcfdb);

int32
tbl_mcfdb_del_mcfdb(tbl_mcfdb_key_t *p_mcfdb_key);

int32
tbl_mcfdb_set_mcfdb_field(tbl_mcfdb_t *p_mcfdb, tbl_mcfdb_field_id_t field_id);

tbl_mcfdb_t*
tbl_mcfdb_get_mcfdb(tbl_mcfdb_key_t *p_mcfdb_key);

char*
tbl_mcfdb_key_val2str(tbl_mcfdb_t *p_mcfdb, char *str, uint32 str_len);

int32
tbl_mcfdb_key_str2val(char *str, tbl_mcfdb_t *p_mcfdb);

int32
tbl_mcfdb_dump_one(tbl_mcfdb_t *p_mcfdb, tbl_iter_args_t *pargs);

char*
tbl_mcfdb_key_name_dump(tbl_mcfdb_t *p_mcfdb, char* str);

char*
tbl_mcfdb_key_value_dump(tbl_mcfdb_t *p_mcfdb, char* str);

char*
tbl_mcfdb_field_name_dump(tbl_mcfdb_t *p_mcfdb, int32 field_id, char* str);

char*
tbl_mcfdb_field_value_dump(tbl_mcfdb_t *p_mcfdb, int32 field_id, char* str);

char**
tbl_mcfdb_table_dump(tbl_mcfdb_t *p_mcfdb, char** str);

int32
tbl_mcfdb_table_parser(char** array, char* key_value, tbl_mcfdb_t *p_mcfdb);

int32
tbl_mcfdb_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_mcfdb_master_t*
tbl_mcfdb_get_master();

tbl_mcfdb_master_t*
tbl_mcfdb_init_mcfdb();

#endif /* !__TBL_MCFDB_H__ */

