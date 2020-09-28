
#ifndef __TBL_RIP_PASSIVE_IF_H__
#define __TBL_RIP_PASSIVE_IF_H__

int32
tbl_rip_passive_if_add_rip_passive_if_sync(tbl_rip_passive_if_t *p_rip_passive_if, uint32 sync);

int32
tbl_rip_passive_if_del_rip_passive_if_sync(tbl_rip_passive_if_key_t *p_rip_passive_if_key, uint32 sync);

int32
tbl_rip_passive_if_set_rip_passive_if_field_sync(tbl_rip_passive_if_t *p_rip_passive_if, tbl_rip_passive_if_field_id_t field_id, uint32 sync);

int32
tbl_rip_passive_if_add_rip_passive_if(tbl_rip_passive_if_t *p_rip_passive_if);

int32
tbl_rip_passive_if_del_rip_passive_if(tbl_rip_passive_if_key_t *p_rip_passive_if_key);

int32
tbl_rip_passive_if_set_rip_passive_if_field(tbl_rip_passive_if_t *p_rip_passive_if, tbl_rip_passive_if_field_id_t field_id);

tbl_rip_passive_if_t*
tbl_rip_passive_if_get_rip_passive_if(tbl_rip_passive_if_key_t *p_rip_passive_if_key);

char*
tbl_rip_passive_if_key_val2str(tbl_rip_passive_if_t *p_rip_passive_if, char *str, uint32 str_len);

int32
tbl_rip_passive_if_key_str2val(char *str, tbl_rip_passive_if_t *p_rip_passive_if);

int32
tbl_rip_passive_if_dump_one(tbl_rip_passive_if_t *p_rip_passive_if, tbl_iter_args_t *pargs);

char*
tbl_rip_passive_if_key_name_dump(tbl_rip_passive_if_t *p_rip_passive_if, char* str);

char*
tbl_rip_passive_if_key_value_dump(tbl_rip_passive_if_t *p_rip_passive_if, char* str);

char*
tbl_rip_passive_if_field_name_dump(tbl_rip_passive_if_t *p_rip_passive_if, int32 field_id, char* str);

char*
tbl_rip_passive_if_field_value_dump(tbl_rip_passive_if_t *p_rip_passive_if, int32 field_id, char* str);

char**
tbl_rip_passive_if_table_dump(tbl_rip_passive_if_t *p_rip_passive_if, char** str);

int32
tbl_rip_passive_if_table_parser(char** array, char* key_value, tbl_rip_passive_if_t *p_rip_passive_if);

int32
tbl_rip_passive_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_rip_passive_if_master_t*
tbl_rip_passive_if_get_master();

tbl_rip_passive_if_master_t*
tbl_rip_passive_if_init_rip_passive_if();

#endif /* !__TBL_RIP_PASSIVE_IF_H__ */

