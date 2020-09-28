
#ifndef __TBL_RIP_H__
#define __TBL_RIP_H__

int32
tbl_rip_add_rip_sync(tbl_rip_t *p_rip, uint32 sync);

int32
tbl_rip_del_rip_sync(tbl_rip_key_t *p_rip_key, uint32 sync);

int32
tbl_rip_set_rip_field_sync(tbl_rip_t *p_rip, tbl_rip_field_id_t field_id, uint32 sync);

int32
tbl_rip_add_rip(tbl_rip_t *p_rip);

int32
tbl_rip_del_rip(tbl_rip_key_t *p_rip_key);

int32
tbl_rip_set_rip_field(tbl_rip_t *p_rip, tbl_rip_field_id_t field_id);

tbl_rip_t*
tbl_rip_get_rip(tbl_rip_key_t *p_rip_key);

char*
tbl_rip_key_val2str(tbl_rip_t *p_rip, char *str, uint32 str_len);

int32
tbl_rip_key_str2val(char *str, tbl_rip_t *p_rip);

int32
tbl_rip_dump_one(tbl_rip_t *p_rip, tbl_iter_args_t *pargs);

char*
tbl_rip_key_name_dump(tbl_rip_t *p_rip, char* str);

char*
tbl_rip_key_value_dump(tbl_rip_t *p_rip, char* str);

char*
tbl_rip_field_name_dump(tbl_rip_t *p_rip, int32 field_id, char* str);

char*
tbl_rip_field_value_dump(tbl_rip_t *p_rip, int32 field_id, char* str);

char**
tbl_rip_table_dump(tbl_rip_t *p_rip, char** str);

int32
tbl_rip_table_parser(char** array, char* key_value, tbl_rip_t *p_rip);

int32
tbl_rip_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_rip_master_t*
tbl_rip_get_master();

tbl_rip_master_t*
tbl_rip_init_rip();

#endif /* !__TBL_RIP_H__ */

