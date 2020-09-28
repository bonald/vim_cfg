
#ifndef __DS_BRGIF_H__
#define __DS_BRGIF_H__

int32
ds_brgif_add_brgif_sync(tbl_interface_t *p_if, ds_brgif_t *p_brgif, uint32 sync);

int32
ds_brgif_del_brgif_sync(tbl_interface_t *p_if, uint32 sync);

int32
ds_brgif_set_brgif_field_sync(tbl_interface_t *p_if, ds_brgif_t *p_brgif, ds_brgif_field_id_t field_id, uint32 sync);

int32
ds_brgif_add_brgif(tbl_interface_t *p_if, ds_brgif_t *p_brgif);

int32
ds_brgif_del_brgif(tbl_interface_t *p_if);

int32
ds_brgif_set_brgif_field(tbl_interface_t *p_if, ds_brgif_t *p_brgif, ds_brgif_field_id_t field_id);

ds_brgif_t*
ds_brgif_get_brgif(tbl_interface_t *p_if);

int32
ds_brgif_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_brgif_t *p_brgif);

char*
ds_brgif_key_name_dump(tbl_interface_t *p_if, ds_brgif_t *p_brgif, char* str);

char*
ds_brgif_key_value_dump(tbl_interface_t *p_if, ds_brgif_t *p_brgif, char* str);

char*
ds_brgif_field_name_dump(tbl_interface_t *p_if, ds_brgif_t *p_brgif, int32 field_id, char* str);

char*
ds_brgif_field_value_dump(tbl_interface_t *p_if, ds_brgif_t *p_brgif, int32 field_id, char* str);

char**
ds_brgif_table_dump(tbl_interface_t *p_if, ds_brgif_t *p_brgif, char** str);

int32
ds_brgif_field_value_parser(char* str, int32 field_id, tbl_interface_t *p_if, ds_brgif_t *p_brgif);

int32
ds_brgif_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_brgif_t *p_brgif);

int32
ds_brgif_dump_one(ds_brgif_t *p_brgif, tbl_iter_args_t *pargs);

#endif /* !__DS_BRGIF_H__ */

