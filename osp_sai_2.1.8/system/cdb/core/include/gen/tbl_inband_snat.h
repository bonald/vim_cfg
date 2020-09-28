
#ifndef __TBL_INBAND_SNAT_H__
#define __TBL_INBAND_SNAT_H__

int32
tbl_inband_snat_add_inband_snat_sync(tbl_inband_snat_t *p_inband_snat, uint32 sync);

int32
tbl_inband_snat_del_inband_snat_sync(tbl_inband_snat_key_t *p_inband_snat_key, uint32 sync);

int32
tbl_inband_snat_set_inband_snat_field_sync(tbl_inband_snat_t *p_inband_snat, tbl_inband_snat_field_id_t field_id, uint32 sync);

int32
tbl_inband_snat_add_inband_snat(tbl_inband_snat_t *p_inband_snat);

int32
tbl_inband_snat_del_inband_snat(tbl_inband_snat_key_t *p_inband_snat_key);

int32
tbl_inband_snat_set_inband_snat_field(tbl_inband_snat_t *p_inband_snat, tbl_inband_snat_field_id_t field_id);

tbl_inband_snat_t*
tbl_inband_snat_get_inband_snat(tbl_inband_snat_key_t *p_inband_snat_key);

char*
tbl_inband_snat_key_val2str(tbl_inband_snat_t *p_inband_snat, char *str, uint32 str_len);

int32
tbl_inband_snat_key_str2val(char *str, tbl_inband_snat_t *p_inband_snat);

int32
tbl_inband_snat_dump_one(tbl_inband_snat_t *p_inband_snat, tbl_iter_args_t *pargs);

char*
tbl_inband_snat_key_name_dump(tbl_inband_snat_t *p_inband_snat, char* str);

char*
tbl_inband_snat_key_value_dump(tbl_inband_snat_t *p_inband_snat, char* str);

char*
tbl_inband_snat_field_name_dump(tbl_inband_snat_t *p_inband_snat, int32 field_id, char* str);

char*
tbl_inband_snat_field_value_dump(tbl_inband_snat_t *p_inband_snat, int32 field_id, char* str);

char**
tbl_inband_snat_table_dump(tbl_inband_snat_t *p_inband_snat, char** str);

int32
tbl_inband_snat_table_parser(char** array, char* key_value, tbl_inband_snat_t *p_inband_snat);

int32
tbl_inband_snat_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_inband_snat_master_t*
tbl_inband_snat_get_master();

tbl_inband_snat_master_t*
tbl_inband_snat_init_inband_snat();

#endif /* !__TBL_INBAND_SNAT_H__ */

