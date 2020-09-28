
#ifndef __TBL_OPENFLOW_MANAGER_H__
#define __TBL_OPENFLOW_MANAGER_H__

int32
tbl_openflow_manager_add_openflow_manager_sync(tbl_openflow_manager_t *p_openflow_manager, uint32 sync);

int32
tbl_openflow_manager_del_openflow_manager_sync(tbl_openflow_manager_key_t *p_openflow_manager_key, uint32 sync);

int32
tbl_openflow_manager_set_openflow_manager_field_sync(tbl_openflow_manager_t *p_openflow_manager, tbl_openflow_manager_field_id_t field_id, uint32 sync);

int32
tbl_openflow_manager_add_openflow_manager(tbl_openflow_manager_t *p_openflow_manager);

int32
tbl_openflow_manager_del_openflow_manager(tbl_openflow_manager_key_t *p_openflow_manager_key);

int32
tbl_openflow_manager_set_openflow_manager_field(tbl_openflow_manager_t *p_openflow_manager, tbl_openflow_manager_field_id_t field_id);

tbl_openflow_manager_t*
tbl_openflow_manager_get_openflow_manager(tbl_openflow_manager_key_t *p_openflow_manager_key);

char*
tbl_openflow_manager_key_val2str(tbl_openflow_manager_t *p_openflow_manager, char *str, uint32 str_len);

int32
tbl_openflow_manager_key_str2val(char *str, tbl_openflow_manager_t *p_openflow_manager);

int32
tbl_openflow_manager_dump_one(tbl_openflow_manager_t *p_openflow_manager, tbl_iter_args_t *pargs);

char*
tbl_openflow_manager_key_name_dump(tbl_openflow_manager_t *p_openflow_manager, char* str);

char*
tbl_openflow_manager_key_value_dump(tbl_openflow_manager_t *p_openflow_manager, char* str);

char*
tbl_openflow_manager_field_name_dump(tbl_openflow_manager_t *p_openflow_manager, int32 field_id, char* str);

char*
tbl_openflow_manager_field_value_dump(tbl_openflow_manager_t *p_openflow_manager, int32 field_id, char* str);

char**
tbl_openflow_manager_table_dump(tbl_openflow_manager_t *p_openflow_manager, char** str);

int32
tbl_openflow_manager_table_parser(char** array, char* key_value, tbl_openflow_manager_t *p_openflow_manager);

int32
tbl_openflow_manager_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_openflow_manager_master_t*
tbl_openflow_manager_get_master();

tbl_openflow_manager_master_t*
tbl_openflow_manager_init_openflow_manager();

#endif /* !__TBL_OPENFLOW_MANAGER_H__ */

