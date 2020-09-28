
#ifndef __TBL_FEA_BRG_IF_H__
#define __TBL_FEA_BRG_IF_H__

int32
tbl_fea_brg_if_add_fea_brg_if_sync(tbl_fea_brg_if_t *p_brgif, uint32 sync);

int32
tbl_fea_brg_if_del_fea_brg_if_sync(tbl_fea_brg_if_key_t *p_brgif_key, uint32 sync);

int32
tbl_fea_brg_if_set_fea_brg_if_field_sync(tbl_fea_brg_if_t *p_brgif, tbl_fea_brg_if_field_id_t field_id, uint32 sync);

int32
tbl_fea_brg_if_add_fea_brg_if(tbl_fea_brg_if_t *p_brgif);

int32
tbl_fea_brg_if_del_fea_brg_if(tbl_fea_brg_if_key_t *p_brgif_key);

int32
tbl_fea_brg_if_set_fea_brg_if_field(tbl_fea_brg_if_t *p_brgif, tbl_fea_brg_if_field_id_t field_id);

tbl_fea_brg_if_t*
tbl_fea_brg_if_get_fea_brg_if(tbl_fea_brg_if_key_t *p_brgif_key);

char*
tbl_fea_brg_if_key_val2str(tbl_fea_brg_if_t *p_brgif, char *str, uint32 str_len);

int32
tbl_fea_brg_if_key_str2val(char *str, tbl_fea_brg_if_t *p_brgif);

int32
tbl_fea_brg_if_dump_one(tbl_fea_brg_if_t *p_brgif, tbl_iter_args_t *pargs);

char*
tbl_fea_brg_if_key_name_dump(tbl_fea_brg_if_t *p_brgif, char* str);

char*
tbl_fea_brg_if_key_value_dump(tbl_fea_brg_if_t *p_brgif, char* str);

char*
tbl_fea_brg_if_field_name_dump(tbl_fea_brg_if_t *p_brgif, int32 field_id, char* str);

char*
tbl_fea_brg_if_field_value_dump(tbl_fea_brg_if_t *p_brgif, int32 field_id, char* str);

char**
tbl_fea_brg_if_table_dump(tbl_fea_brg_if_t *p_brgif, char** str);

int32
tbl_fea_brg_if_table_parser(char** array, char* key_value, tbl_fea_brg_if_t *p_brgif);

int32
tbl_fea_brg_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fea_brg_if_master_t*
tbl_fea_brg_if_get_master();

tbl_fea_brg_if_master_t*
tbl_fea_brg_if_init_fea_brg_if();

#endif /* !__TBL_FEA_BRG_IF_H__ */
