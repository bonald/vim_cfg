
#ifndef __TBL_FEA_QOS_DROP_PROFILE_H__
#define __TBL_FEA_QOS_DROP_PROFILE_H__

int32
tbl_fea_qos_drop_profile_add_fea_qos_drop_profile_sync(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, uint32 sync);

int32
tbl_fea_qos_drop_profile_del_fea_qos_drop_profile_sync(tbl_fea_qos_drop_profile_key_t *p_fea_qos_drop_profile_key, uint32 sync);

int32
tbl_fea_qos_drop_profile_set_fea_qos_drop_profile_field_sync(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, tbl_fea_qos_drop_profile_field_id_t field_id, uint32 sync);

int32
tbl_fea_qos_drop_profile_add_fea_qos_drop_profile(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile);

int32
tbl_fea_qos_drop_profile_del_fea_qos_drop_profile(tbl_fea_qos_drop_profile_key_t *p_fea_qos_drop_profile_key);

int32
tbl_fea_qos_drop_profile_set_fea_qos_drop_profile_field(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, tbl_fea_qos_drop_profile_field_id_t field_id);

tbl_fea_qos_drop_profile_t*
tbl_fea_qos_drop_profile_get_fea_qos_drop_profile(tbl_fea_qos_drop_profile_key_t *p_fea_qos_drop_profile_key);

char*
tbl_fea_qos_drop_profile_key_val2str(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, char *str, uint32 str_len);

int32
tbl_fea_qos_drop_profile_key_str2val(char *str, tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile);

int32
tbl_fea_qos_drop_profile_dump_one(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, tbl_iter_args_t *pargs);

char*
tbl_fea_qos_drop_profile_key_name_dump(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, char* str);

char*
tbl_fea_qos_drop_profile_key_value_dump(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, char* str);

char*
tbl_fea_qos_drop_profile_field_name_dump(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, int32 field_id, char* str);

char*
tbl_fea_qos_drop_profile_field_value_dump(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, int32 field_id, char* str);

char**
tbl_fea_qos_drop_profile_table_dump(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, char** str);

int32
tbl_fea_qos_drop_profile_table_parser(char** array, char* key_value, tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile);

int32
tbl_fea_qos_drop_profile_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fea_qos_drop_profile_master_t*
tbl_fea_qos_drop_profile_get_master();

tbl_fea_qos_drop_profile_master_t*
tbl_fea_qos_drop_profile_init_fea_qos_drop_profile();

#endif /* !__TBL_FEA_QOS_DROP_PROFILE_H__ */

