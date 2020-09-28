
#ifndef __TBL_ISOLATION_H__
#define __TBL_ISOLATION_H__

int32
tbl_isolation_add_isolation_sync(tbl_isolation_t *p_iso, uint32 sync);

int32
tbl_isolation_del_isolation_sync(tbl_isolation_key_t *p_iso_key, uint32 sync);

int32
tbl_isolation_set_isolation_field_sync(tbl_isolation_t *p_iso, tbl_isolation_field_id_t field_id, uint32 sync);

int32
tbl_isolation_add_isolation(tbl_isolation_t *p_iso);

int32
tbl_isolation_del_isolation(tbl_isolation_key_t *p_iso_key);

int32
tbl_isolation_set_isolation_field(tbl_isolation_t *p_iso, tbl_isolation_field_id_t field_id);

tbl_isolation_t*
tbl_isolation_get_isolation(tbl_isolation_key_t *p_iso_key);

char*
tbl_isolation_key_val2str(tbl_isolation_t *p_iso, char *str, uint32 str_len);

int32
tbl_isolation_key_str2val(char *str, tbl_isolation_t *p_iso);

int32
tbl_isolation_dump_one(tbl_isolation_t *p_iso, tbl_iter_args_t *pargs);

char*
tbl_isolation_key_name_dump(tbl_isolation_t *p_iso, char* str);

char*
tbl_isolation_key_value_dump(tbl_isolation_t *p_iso, char* str);

char*
tbl_isolation_field_name_dump(tbl_isolation_t *p_iso, int32 field_id, char* str);

char*
tbl_isolation_field_value_dump(tbl_isolation_t *p_iso, int32 field_id, char* str);

char**
tbl_isolation_table_dump(tbl_isolation_t *p_iso, char** str);

int32
tbl_isolation_table_parser(char** array, char* key_value, tbl_isolation_t *p_iso);

int32
tbl_isolation_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_isolation_master_t*
tbl_isolation_get_master();

tbl_isolation_master_t*
tbl_isolation_init_isolation();

int32
tbl_isolation_set_ports_sync(tbl_isolation_t *p_iso, uint32 add, void *p_object, uint32 sync);

int32
tbl_isolation_set_ports(tbl_isolation_t *p_iso, uint32 add, void *p_object);

#endif /* !__TBL_ISOLATION_H__ */

