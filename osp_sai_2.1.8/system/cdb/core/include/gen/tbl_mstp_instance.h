
#ifndef __TBL_MSTP_INSTANCE_H__
#define __TBL_MSTP_INSTANCE_H__

int32
tbl_mstp_instance_add_mstp_instance_sync(tbl_mstp_instance_t *p_inst, uint32 sync);

int32
tbl_mstp_instance_del_mstp_instance_sync(tbl_mstp_instance_key_t *p_inst_key, uint32 sync);

int32
tbl_mstp_instance_set_mstp_instance_field_sync(tbl_mstp_instance_t *p_inst, tbl_mstp_instance_field_id_t field_id, uint32 sync);

int32
tbl_mstp_instance_add_mstp_instance(tbl_mstp_instance_t *p_inst);

int32
tbl_mstp_instance_del_mstp_instance(tbl_mstp_instance_key_t *p_inst_key);

int32
tbl_mstp_instance_set_mstp_instance_field(tbl_mstp_instance_t *p_inst, tbl_mstp_instance_field_id_t field_id);

tbl_mstp_instance_t*
tbl_mstp_instance_get_mstp_instance(tbl_mstp_instance_key_t *p_inst_key);

char*
tbl_mstp_instance_key_val2str(tbl_mstp_instance_t *p_inst, char *str, uint32 str_len);

int32
tbl_mstp_instance_key_str2val(char *str, tbl_mstp_instance_t *p_inst);

int32
tbl_mstp_instance_dump_one(tbl_mstp_instance_t *p_inst, tbl_iter_args_t *pargs);

char*
tbl_mstp_instance_key_name_dump(tbl_mstp_instance_t *p_inst, char* str);

char*
tbl_mstp_instance_key_value_dump(tbl_mstp_instance_t *p_inst, char* str);

char*
tbl_mstp_instance_field_name_dump(tbl_mstp_instance_t *p_inst, int32 field_id, char* str);

char*
tbl_mstp_instance_field_value_dump(tbl_mstp_instance_t *p_inst, int32 field_id, char* str);

char**
tbl_mstp_instance_table_dump(tbl_mstp_instance_t *p_inst, char** str);

int32
tbl_mstp_instance_table_parser(char** array, char* key_value, tbl_mstp_instance_t *p_inst);

int32
tbl_mstp_instance_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_mstp_instance_master_t*
tbl_mstp_instance_get_master();

tbl_mstp_instance_master_t*
tbl_mstp_instance_init_mstp_instance();

int32
tbl_mstp_instance_set_port_list_sync(tbl_mstp_instance_t *p_inst, uint32 add, void *p_object, uint32 sync);

int32
tbl_mstp_instance_set_port_list(tbl_mstp_instance_t *p_inst, uint32 add, void *p_object);

#endif /* !__TBL_MSTP_INSTANCE_H__ */

