
#ifndef __TBL_OAM_MPLS_APS_CHECK_H__
#define __TBL_OAM_MPLS_APS_CHECK_H__

int32
tbl_oam_mpls_aps_check_add_oam_mpls_aps_check_sync(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, uint32 sync);

int32
tbl_oam_mpls_aps_check_del_oam_mpls_aps_check_sync(tbl_oam_mpls_aps_check_key_t *p_oam_mpls_aps_check_key, uint32 sync);

int32
tbl_oam_mpls_aps_check_set_oam_mpls_aps_check_field_sync(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, tbl_oam_mpls_aps_check_field_id_t field_id, uint32 sync);

int32
tbl_oam_mpls_aps_check_add_oam_mpls_aps_check(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check);

int32
tbl_oam_mpls_aps_check_del_oam_mpls_aps_check(tbl_oam_mpls_aps_check_key_t *p_oam_mpls_aps_check_key);

int32
tbl_oam_mpls_aps_check_set_oam_mpls_aps_check_field(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, tbl_oam_mpls_aps_check_field_id_t field_id);

tbl_oam_mpls_aps_check_t*
tbl_oam_mpls_aps_check_get_oam_mpls_aps_check(tbl_oam_mpls_aps_check_key_t *p_oam_mpls_aps_check_key);

char*
tbl_oam_mpls_aps_check_key_val2str(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, char *str, uint32 str_len);

int32
tbl_oam_mpls_aps_check_key_str2val(char *str, tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check);

int32
tbl_oam_mpls_aps_check_dump_one(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, tbl_iter_args_t *pargs);

char*
tbl_oam_mpls_aps_check_key_name_dump(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, char* str);

char*
tbl_oam_mpls_aps_check_key_value_dump(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, char* str);

char*
tbl_oam_mpls_aps_check_field_name_dump(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, int32 field_id, char* str);

char*
tbl_oam_mpls_aps_check_field_value_dump(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, int32 field_id, char* str);

char**
tbl_oam_mpls_aps_check_table_dump(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, char** str);

int32
tbl_oam_mpls_aps_check_table_parser(char** array, char* key_value, tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check);

int32
tbl_oam_mpls_aps_check_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_oam_mpls_aps_check_master_t*
tbl_oam_mpls_aps_check_get_master();

tbl_oam_mpls_aps_check_master_t*
tbl_oam_mpls_aps_check_init_oam_mpls_aps_check();

#endif /* !__TBL_OAM_MPLS_APS_CHECK_H__ */

