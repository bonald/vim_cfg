
#ifndef __TBL_PIM_SM_RP_GP_H__
#define __TBL_PIM_SM_RP_GP_H__

int32
tbl_pim_sm_rp_gp_add_pim_sm_rp_gp_sync(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, uint32 sync);

int32
tbl_pim_sm_rp_gp_del_pim_sm_rp_gp_sync(tbl_pim_sm_rp_gp_key_t *p_pim_sm_rp_gp_key, uint32 sync);

int32
tbl_pim_sm_rp_gp_set_pim_sm_rp_gp_field_sync(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, tbl_pim_sm_rp_gp_field_id_t field_id, uint32 sync);

int32
tbl_pim_sm_rp_gp_add_pim_sm_rp_gp(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp);

int32
tbl_pim_sm_rp_gp_del_pim_sm_rp_gp(tbl_pim_sm_rp_gp_key_t *p_pim_sm_rp_gp_key);

int32
tbl_pim_sm_rp_gp_set_pim_sm_rp_gp_field(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, tbl_pim_sm_rp_gp_field_id_t field_id);

tbl_pim_sm_rp_gp_t*
tbl_pim_sm_rp_gp_get_pim_sm_rp_gp(tbl_pim_sm_rp_gp_key_t *p_pim_sm_rp_gp_key);

char*
tbl_pim_sm_rp_gp_key_val2str(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, char *str, uint32 str_len);

int32
tbl_pim_sm_rp_gp_key_str2val(char *str, tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp);

int32
tbl_pim_sm_rp_gp_dump_one(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, tbl_iter_args_t *pargs);

char*
tbl_pim_sm_rp_gp_key_name_dump(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, char* str);

char*
tbl_pim_sm_rp_gp_key_value_dump(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, char* str);

char*
tbl_pim_sm_rp_gp_field_name_dump(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, int32 field_id, char* str);

char*
tbl_pim_sm_rp_gp_field_value_dump(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, int32 field_id, char* str);

char**
tbl_pim_sm_rp_gp_table_dump(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, char** str);

int32
tbl_pim_sm_rp_gp_table_parser(char** array, char* key_value, tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp);

int32
tbl_pim_sm_rp_gp_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_pim_sm_rp_gp_master_t*
tbl_pim_sm_rp_gp_get_master();

tbl_pim_sm_rp_gp_master_t*
tbl_pim_sm_rp_gp_init_pim_sm_rp_gp();

#endif /* !__TBL_PIM_SM_RP_GP_H__ */

