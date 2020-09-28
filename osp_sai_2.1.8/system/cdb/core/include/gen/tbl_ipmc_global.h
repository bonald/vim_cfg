
#ifndef __TBL_IPMC_GLOBAL_H__
#define __TBL_IPMC_GLOBAL_H__

int32
tbl_ipmc_global_set_ipmc_global_field_sync(tbl_ipmc_global_t *p_pim_glb, tbl_ipmc_global_field_id_t field_id, uint32 sync);

int32
tbl_ipmc_global_set_ipmc_global_field(tbl_ipmc_global_t *p_pim_glb, tbl_ipmc_global_field_id_t field_id);

tbl_ipmc_global_t*
tbl_ipmc_global_get_ipmc_global();

char*
tbl_ipmc_global_key_val2str(tbl_ipmc_global_t *p_pim_glb, char *str, uint32 str_len);

int32
tbl_ipmc_global_key_str2val(char *str, tbl_ipmc_global_t *p_pim_glb);

int32
tbl_ipmc_global_dump_one(tbl_ipmc_global_t *p_pim_glb, tbl_iter_args_t *pargs);

char*
tbl_ipmc_global_key_name_dump(tbl_ipmc_global_t *p_pim_glb, char* str);

char*
tbl_ipmc_global_key_value_dump(tbl_ipmc_global_t *p_pim_glb, char* str);

char*
tbl_ipmc_global_field_name_dump(tbl_ipmc_global_t *p_pim_glb, int32 field_id, char* str);

char*
tbl_ipmc_global_field_value_dump(tbl_ipmc_global_t *p_pim_glb, int32 field_id, char* str);

char**
tbl_ipmc_global_table_dump(tbl_ipmc_global_t *p_pim_glb, char** str);

int32
tbl_ipmc_global_table_parser(char** array, char* key_value, tbl_ipmc_global_t *p_pim_glb);

int32
tbl_ipmc_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_ipmc_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ipmc_global_t*
tbl_ipmc_global_init_ipmc_global();

#endif /* !__TBL_IPMC_GLOBAL_H__ */

