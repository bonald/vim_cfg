
#ifndef __TBL_PIM_SM_BSR_H__
#define __TBL_PIM_SM_BSR_H__

int32
tbl_pim_sm_bsr_add_pim_sm_bsr_sync(tbl_pim_sm_bsr_t *p_pim_sm_bsr, uint32 sync);

int32
tbl_pim_sm_bsr_del_pim_sm_bsr_sync(tbl_pim_sm_bsr_key_t *p_pim_sm_bsr_key, uint32 sync);

int32
tbl_pim_sm_bsr_set_pim_sm_bsr_field_sync(tbl_pim_sm_bsr_t *p_pim_sm_bsr, tbl_pim_sm_bsr_field_id_t field_id, uint32 sync);

int32
tbl_pim_sm_bsr_add_pim_sm_bsr(tbl_pim_sm_bsr_t *p_pim_sm_bsr);

int32
tbl_pim_sm_bsr_del_pim_sm_bsr(tbl_pim_sm_bsr_key_t *p_pim_sm_bsr_key);

int32
tbl_pim_sm_bsr_set_pim_sm_bsr_field(tbl_pim_sm_bsr_t *p_pim_sm_bsr, tbl_pim_sm_bsr_field_id_t field_id);

tbl_pim_sm_bsr_t*
tbl_pim_sm_bsr_get_pim_sm_bsr(tbl_pim_sm_bsr_key_t *p_pim_sm_bsr_key);

char*
tbl_pim_sm_bsr_key_val2str(tbl_pim_sm_bsr_t *p_pim_sm_bsr, char *str, uint32 str_len);

int32
tbl_pim_sm_bsr_key_str2val(char *str, tbl_pim_sm_bsr_t *p_pim_sm_bsr);

int32
tbl_pim_sm_bsr_dump_one(tbl_pim_sm_bsr_t *p_pim_sm_bsr, tbl_iter_args_t *pargs);

char*
tbl_pim_sm_bsr_key_name_dump(tbl_pim_sm_bsr_t *p_pim_sm_bsr, char* str);

char*
tbl_pim_sm_bsr_key_value_dump(tbl_pim_sm_bsr_t *p_pim_sm_bsr, char* str);

char*
tbl_pim_sm_bsr_field_name_dump(tbl_pim_sm_bsr_t *p_pim_sm_bsr, int32 field_id, char* str);

char*
tbl_pim_sm_bsr_field_value_dump(tbl_pim_sm_bsr_t *p_pim_sm_bsr, int32 field_id, char* str);

char**
tbl_pim_sm_bsr_table_dump(tbl_pim_sm_bsr_t *p_pim_sm_bsr, char** str);

int32
tbl_pim_sm_bsr_table_parser(char** array, char* key_value, tbl_pim_sm_bsr_t *p_pim_sm_bsr);

int32
tbl_pim_sm_bsr_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_pim_sm_bsr_master_t*
tbl_pim_sm_bsr_get_master();

tbl_pim_sm_bsr_master_t*
tbl_pim_sm_bsr_init_pim_sm_bsr();

#endif /* !__TBL_PIM_SM_BSR_H__ */

