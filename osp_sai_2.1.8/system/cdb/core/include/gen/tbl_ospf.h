
#ifndef __TBL_OSPF_H__
#define __TBL_OSPF_H__

int32
tbl_ospf_add_ospf_sync(tbl_ospf_t *p_ospf, uint32 sync);

int32
tbl_ospf_del_ospf_sync(tbl_ospf_key_t *p_ospf_key, uint32 sync);

int32
tbl_ospf_set_ospf_field_sync(tbl_ospf_t *p_ospf, tbl_ospf_field_id_t field_id, uint32 sync);

int32
tbl_ospf_add_ospf(tbl_ospf_t *p_ospf);

int32
tbl_ospf_del_ospf(tbl_ospf_key_t *p_ospf_key);

int32
tbl_ospf_set_ospf_field(tbl_ospf_t *p_ospf, tbl_ospf_field_id_t field_id);

tbl_ospf_t*
tbl_ospf_get_ospf(tbl_ospf_key_t *p_ospf_key);

char*
tbl_ospf_key_val2str(tbl_ospf_t *p_ospf, char *str, uint32 str_len);

int32
tbl_ospf_key_str2val(char *str, tbl_ospf_t *p_ospf);

int32
tbl_ospf_dump_one(tbl_ospf_t *p_ospf, tbl_iter_args_t *pargs);

char*
tbl_ospf_key_name_dump(tbl_ospf_t *p_ospf, char* str);

char*
tbl_ospf_key_value_dump(tbl_ospf_t *p_ospf, char* str);

char*
tbl_ospf_field_name_dump(tbl_ospf_t *p_ospf, int32 field_id, char* str);

char*
tbl_ospf_field_value_dump(tbl_ospf_t *p_ospf, int32 field_id, char* str);

char**
tbl_ospf_table_dump(tbl_ospf_t *p_ospf, char** str);

int32
tbl_ospf_table_parser(char** array, char* key_value, tbl_ospf_t *p_ospf);

int32
tbl_ospf_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ospf_master_t*
tbl_ospf_get_master();

tbl_ospf_master_t*
tbl_ospf_init_ospf();

#endif /* !__TBL_OSPF_H__ */

