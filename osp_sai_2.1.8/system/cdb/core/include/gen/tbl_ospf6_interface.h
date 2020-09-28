
#ifndef __TBL_OSPF6_INTERFACE_H__
#define __TBL_OSPF6_INTERFACE_H__

int32
tbl_ospf6_interface_add_ospf6_interface_sync(tbl_ospf6_interface_t *p_ospf6_interface, uint32 sync);

int32
tbl_ospf6_interface_del_ospf6_interface_sync(tbl_ospf6_interface_key_t *p_ospf6_interface_key, uint32 sync);

int32
tbl_ospf6_interface_set_ospf6_interface_field_sync(tbl_ospf6_interface_t *p_ospf6_interface, tbl_ospf6_interface_field_id_t field_id, uint32 sync);

int32
tbl_ospf6_interface_add_ospf6_interface(tbl_ospf6_interface_t *p_ospf6_interface);

int32
tbl_ospf6_interface_del_ospf6_interface(tbl_ospf6_interface_key_t *p_ospf6_interface_key);

int32
tbl_ospf6_interface_set_ospf6_interface_field(tbl_ospf6_interface_t *p_ospf6_interface, tbl_ospf6_interface_field_id_t field_id);

tbl_ospf6_interface_t*
tbl_ospf6_interface_get_ospf6_interface(tbl_ospf6_interface_key_t *p_ospf6_interface_key);

char*
tbl_ospf6_interface_key_val2str(tbl_ospf6_interface_t *p_ospf6_interface, char *str, uint32 str_len);

int32
tbl_ospf6_interface_key_str2val(char *str, tbl_ospf6_interface_t *p_ospf6_interface);

int32
tbl_ospf6_interface_dump_one(tbl_ospf6_interface_t *p_ospf6_interface, tbl_iter_args_t *pargs);

char*
tbl_ospf6_interface_key_name_dump(tbl_ospf6_interface_t *p_ospf6_interface, char* str);

char*
tbl_ospf6_interface_key_value_dump(tbl_ospf6_interface_t *p_ospf6_interface, char* str);

char*
tbl_ospf6_interface_field_name_dump(tbl_ospf6_interface_t *p_ospf6_interface, int32 field_id, char* str);

char*
tbl_ospf6_interface_field_value_dump(tbl_ospf6_interface_t *p_ospf6_interface, int32 field_id, char* str);

char**
tbl_ospf6_interface_table_dump(tbl_ospf6_interface_t *p_ospf6_interface, char** str);

int32
tbl_ospf6_interface_table_parser(char** array, char* key_value, tbl_ospf6_interface_t *p_ospf6_interface);

int32
tbl_ospf6_interface_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ospf6_interface_master_t*
tbl_ospf6_interface_get_master();

tbl_ospf6_interface_master_t*
tbl_ospf6_interface_init_ospf6_interface();

#endif /* !__TBL_OSPF6_INTERFACE_H__ */

