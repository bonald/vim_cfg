
#ifndef __TBL_BHM_MODULE_H__
#define __TBL_BHM_MODULE_H__

int32
tbl_bhm_module_add_bhm_module_sync(tbl_bhm_module_t *p_bhm_module, uint32 sync);

int32
tbl_bhm_module_del_bhm_module_sync(tbl_bhm_module_key_t *p_bhm_module_key, uint32 sync);

int32
tbl_bhm_module_set_bhm_module_field_sync(tbl_bhm_module_t *p_bhm_module, tbl_bhm_module_field_id_t field_id, uint32 sync);

int32
tbl_bhm_module_add_bhm_module(tbl_bhm_module_t *p_bhm_module);

int32
tbl_bhm_module_del_bhm_module(tbl_bhm_module_key_t *p_bhm_module_key);

int32
tbl_bhm_module_set_bhm_module_field(tbl_bhm_module_t *p_bhm_module, tbl_bhm_module_field_id_t field_id);

tbl_bhm_module_t*
tbl_bhm_module_get_bhm_module(tbl_bhm_module_key_t *p_bhm_module_key);

char*
tbl_bhm_module_key_val2str(tbl_bhm_module_t *p_bhm_module, char *str, uint32 str_len);

int32
tbl_bhm_module_key_str2val(char *str, tbl_bhm_module_t *p_bhm_module);

int32
tbl_bhm_module_dump_one(tbl_bhm_module_t *p_bhm_module, tbl_iter_args_t *pargs);

char*
tbl_bhm_module_key_name_dump(tbl_bhm_module_t *p_bhm_module, char* str);

char*
tbl_bhm_module_key_value_dump(tbl_bhm_module_t *p_bhm_module, char* str);

char*
tbl_bhm_module_field_name_dump(tbl_bhm_module_t *p_bhm_module, int32 field_id, char* str);

char*
tbl_bhm_module_field_value_dump(tbl_bhm_module_t *p_bhm_module, int32 field_id, char* str);

char**
tbl_bhm_module_table_dump(tbl_bhm_module_t *p_bhm_module, char** str);

int32
tbl_bhm_module_table_parser(char** array, char* key_value, tbl_bhm_module_t *p_bhm_module);

int32
tbl_bhm_module_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_bhm_module_master_t*
tbl_bhm_module_get_master();

tbl_bhm_module_master_t*
tbl_bhm_module_init_bhm_module();

#endif /* !__TBL_BHM_MODULE_H__ */

