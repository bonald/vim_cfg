
#ifndef __TBL_ACLQOS_IF_H__
#define __TBL_ACLQOS_IF_H__

int32
tbl_aclqos_if_add_aclqos_if_sync(tbl_aclqos_if_t *p_aclqos_if, uint32 sync);

int32
tbl_aclqos_if_del_aclqos_if_sync(tbl_aclqos_if_key_t *p_aclqos_if_key, uint32 sync);

int32
tbl_aclqos_if_set_aclqos_if_field_sync(tbl_aclqos_if_t *p_aclqos_if, tbl_aclqos_if_field_id_t field_id, uint32 sync);

int32
tbl_aclqos_if_add_aclqos_if(tbl_aclqos_if_t *p_aclqos_if);

int32
tbl_aclqos_if_del_aclqos_if(tbl_aclqos_if_key_t *p_aclqos_if_key);

int32
tbl_aclqos_if_set_aclqos_if_field(tbl_aclqos_if_t *p_aclqos_if, tbl_aclqos_if_field_id_t field_id);

tbl_aclqos_if_t*
tbl_aclqos_if_get_aclqos_if(tbl_aclqos_if_key_t *p_aclqos_if_key);

char*
tbl_aclqos_if_key_val2str(tbl_aclqos_if_t *p_aclqos_if, char *str, uint32 str_len);

int32
tbl_aclqos_if_key_str2val(char *str, tbl_aclqos_if_t *p_aclqos_if);

int32
tbl_aclqos_if_dump_one(tbl_aclqos_if_t *p_aclqos_if, tbl_iter_args_t *pargs);

char*
tbl_aclqos_if_key_name_dump(tbl_aclqos_if_t *p_aclqos_if, char* str);

char*
tbl_aclqos_if_key_value_dump(tbl_aclqos_if_t *p_aclqos_if, char* str);

char*
tbl_aclqos_if_field_name_dump(tbl_aclqos_if_t *p_aclqos_if, int32 field_id, char* str);

char*
tbl_aclqos_if_field_value_dump(tbl_aclqos_if_t *p_aclqos_if, int32 field_id, char* str);

char**
tbl_aclqos_if_table_dump(tbl_aclqos_if_t *p_aclqos_if, char** str);

int32
tbl_aclqos_if_table_parser(char** array, char* key_value, tbl_aclqos_if_t *p_aclqos_if);

int32
tbl_aclqos_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_aclqos_if_master_t*
tbl_aclqos_if_get_master();

tbl_aclqos_if_master_t*
tbl_aclqos_if_init_aclqos_if();

tbl_aclqos_if_t*
tbl_aclqos_if_get_aclqos_if_by_name(const char* ifname);
#endif /* !__TBL_ACLQOS_IF_H__ */

