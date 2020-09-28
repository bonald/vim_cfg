
#ifndef __TBL_OSPF6_H__
#define __TBL_OSPF6_H__

int32
tbl_ospf6_add_ospf6_sync(tbl_ospf6_t *p_ospf6, uint32 sync);

int32
tbl_ospf6_del_ospf6_sync(tbl_ospf6_key_t *p_ospf6_key, uint32 sync);

int32
tbl_ospf6_set_ospf6_field_sync(tbl_ospf6_t *p_ospf6, tbl_ospf6_field_id_t field_id, uint32 sync);

int32
tbl_ospf6_add_ospf6(tbl_ospf6_t *p_ospf6);

int32
tbl_ospf6_del_ospf6(tbl_ospf6_key_t *p_ospf6_key);

int32
tbl_ospf6_set_ospf6_field(tbl_ospf6_t *p_ospf6, tbl_ospf6_field_id_t field_id);

tbl_ospf6_t*
tbl_ospf6_get_ospf6(tbl_ospf6_key_t *p_ospf6_key);

char*
tbl_ospf6_key_val2str(tbl_ospf6_t *p_ospf6, char *str, uint32 str_len);

int32
tbl_ospf6_key_str2val(char *str, tbl_ospf6_t *p_ospf6);

int32
tbl_ospf6_dump_one(tbl_ospf6_t *p_ospf6, tbl_iter_args_t *pargs);

char*
tbl_ospf6_key_name_dump(tbl_ospf6_t *p_ospf6, char* str);

char*
tbl_ospf6_key_value_dump(tbl_ospf6_t *p_ospf6, char* str);

char*
tbl_ospf6_field_name_dump(tbl_ospf6_t *p_ospf6, int32 field_id, char* str);

char*
tbl_ospf6_field_value_dump(tbl_ospf6_t *p_ospf6, int32 field_id, char* str);

char**
tbl_ospf6_table_dump(tbl_ospf6_t *p_ospf6, char** str);

int32
tbl_ospf6_table_parser(char** array, char* key_value, tbl_ospf6_t *p_ospf6);

int32
tbl_ospf6_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ospf6_master_t*
tbl_ospf6_get_master();

tbl_ospf6_master_t*
tbl_ospf6_init_ospf6();

#endif /* !__TBL_OSPF6_H__ */

