
#ifndef __DS_CIRCUIT_ID_H__
#define __DS_CIRCUIT_ID_H__

int32
ds_circuit_id_add_circuit_id_sync(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, uint32 sync);

int32
ds_circuit_id_del_circuit_id_sync(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, uint32 sync);

int32
ds_circuit_id_set_circuit_id_field_sync(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, ds_circuit_id_field_id_t field_id, uint32 sync);

int32
ds_circuit_id_add_circuit_id(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id);

int32
ds_circuit_id_del_circuit_id(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id);

int32
ds_circuit_id_set_circuit_id_field(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, ds_circuit_id_field_id_t field_id);

ds_circuit_id_t*
ds_circuit_id_get_circuit_id(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id);

int32
ds_circuit_id_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id);

char*
ds_circuit_id_key_name_dump(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, char* str);

char*
ds_circuit_id_key_value_dump(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, char* str);

char*
ds_circuit_id_field_name_dump(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, int32 field_id, char* str);

char*
ds_circuit_id_field_value_dump(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, int32 field_id, char* str);

char**
ds_circuit_id_table_dump(tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id, char** str);

int32
ds_circuit_id_field_value_parser(char* str, int32 field_id, tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id);

int32
ds_circuit_id_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_circuit_id_t *p_circuit_id);

int32
ds_circuit_id_dump_one(ds_circuit_id_t *p_circuit_id, tbl_iter_args_t *pargs);

#endif /* !__DS_CIRCUIT_ID_H__ */

