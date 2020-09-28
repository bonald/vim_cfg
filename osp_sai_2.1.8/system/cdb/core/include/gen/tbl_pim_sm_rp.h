
#ifndef __TBL_PIM_SM_RP_H__
#define __TBL_PIM_SM_RP_H__

int32
tbl_pim_sm_rp_add_pim_sm_rp_sync(tbl_pim_sm_rp_t *p_pim_sm_rp, uint32 sync);

int32
tbl_pim_sm_rp_del_pim_sm_rp_sync(tbl_pim_sm_rp_key_t *p_pim_sm_rp_key, uint32 sync);

int32
tbl_pim_sm_rp_set_pim_sm_rp_field_sync(tbl_pim_sm_rp_t *p_pim_sm_rp, tbl_pim_sm_rp_field_id_t field_id, uint32 sync);

int32
tbl_pim_sm_rp_add_pim_sm_rp(tbl_pim_sm_rp_t *p_pim_sm_rp);

int32
tbl_pim_sm_rp_del_pim_sm_rp(tbl_pim_sm_rp_key_t *p_pim_sm_rp_key);

int32
tbl_pim_sm_rp_set_pim_sm_rp_field(tbl_pim_sm_rp_t *p_pim_sm_rp, tbl_pim_sm_rp_field_id_t field_id);

tbl_pim_sm_rp_t*
tbl_pim_sm_rp_get_pim_sm_rp(tbl_pim_sm_rp_key_t *p_pim_sm_rp_key);

char*
tbl_pim_sm_rp_key_val2str(tbl_pim_sm_rp_t *p_pim_sm_rp, char *str, uint32 str_len);

int32
tbl_pim_sm_rp_key_str2val(char *str, tbl_pim_sm_rp_t *p_pim_sm_rp);

int32
tbl_pim_sm_rp_dump_one(tbl_pim_sm_rp_t *p_pim_sm_rp, tbl_iter_args_t *pargs);

char*
tbl_pim_sm_rp_key_name_dump(tbl_pim_sm_rp_t *p_pim_sm_rp, char* str);

char*
tbl_pim_sm_rp_key_value_dump(tbl_pim_sm_rp_t *p_pim_sm_rp, char* str);

char*
tbl_pim_sm_rp_field_name_dump(tbl_pim_sm_rp_t *p_pim_sm_rp, int32 field_id, char* str);

char*
tbl_pim_sm_rp_field_value_dump(tbl_pim_sm_rp_t *p_pim_sm_rp, int32 field_id, char* str);

char**
tbl_pim_sm_rp_table_dump(tbl_pim_sm_rp_t *p_pim_sm_rp, char** str);

int32
tbl_pim_sm_rp_table_parser(char** array, char* key_value, tbl_pim_sm_rp_t *p_pim_sm_rp);

int32
tbl_pim_sm_rp_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_pim_sm_rp_master_t*
tbl_pim_sm_rp_get_master();

tbl_pim_sm_rp_master_t*
tbl_pim_sm_rp_init_pim_sm_rp();

#endif /* !__TBL_PIM_SM_RP_H__ */

