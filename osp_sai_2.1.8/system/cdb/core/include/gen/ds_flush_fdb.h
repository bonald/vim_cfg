
#ifndef __DS_FLUSH_FDB_H__
#define __DS_FLUSH_FDB_H__

int32
ds_flush_fdb_add_flush_fdb_sync(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, uint32 sync);

int32
ds_flush_fdb_del_flush_fdb_sync(tbl_l2_action_t *p_l2_act, uint32 sync);

int32
ds_flush_fdb_set_flush_fdb_field_sync(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, ds_flush_fdb_field_id_t field_id, uint32 sync);

int32
ds_flush_fdb_add_flush_fdb(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb);

int32
ds_flush_fdb_del_flush_fdb(tbl_l2_action_t *p_l2_act);

int32
ds_flush_fdb_set_flush_fdb_field(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, ds_flush_fdb_field_id_t field_id);

ds_flush_fdb_t*
ds_flush_fdb_get_flush_fdb(tbl_l2_action_t *p_l2_act);

int32
ds_flush_fdb_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb);

char*
ds_flush_fdb_key_name_dump(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, char* str);

char*
ds_flush_fdb_key_value_dump(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, char* str);

char*
ds_flush_fdb_field_name_dump(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, int32 field_id, char* str);

char*
ds_flush_fdb_field_value_dump(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, int32 field_id, char* str);

char**
ds_flush_fdb_table_dump(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, char** str);

int32
ds_flush_fdb_field_value_parser(char* str, int32 field_id, tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb);

int32
ds_flush_fdb_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb);

int32
ds_flush_fdb_dump_one(ds_flush_fdb_t *p_flush_fdb, tbl_iter_args_t *pargs);

#endif /* !__DS_FLUSH_FDB_H__ */

