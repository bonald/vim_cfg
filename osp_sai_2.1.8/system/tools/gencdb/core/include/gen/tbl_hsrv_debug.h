
#ifndef __TBL_HSRV_DEBUG_H__
#define __TBL_HSRV_DEBUG_H__

int32
tbl_hsrv_debug_set_hsrv_debug_field_sync(tbl_hsrv_debug_t *p_hsrvdbg, tbl_hsrv_debug_field_id_t field_id, uint32 sync);

int32
tbl_hsrv_debug_set_hsrv_debug_field(tbl_hsrv_debug_t *p_hsrvdbg, tbl_hsrv_debug_field_id_t field_id);

tbl_hsrv_debug_t*
tbl_hsrv_debug_get_hsrv_debug();

char*
tbl_hsrv_debug_key_val2str(tbl_hsrv_debug_t *p_hsrvdbg, char *str, uint32 str_len);

int32
tbl_hsrv_debug_key_str2val(char *str, tbl_hsrv_debug_t *p_hsrvdbg);

int32
tbl_hsrv_debug_dump_one(tbl_hsrv_debug_t *p_hsrvdbg, tbl_iter_args_t *pargs);

char*
tbl_hsrv_debug_key_name_dump(tbl_hsrv_debug_t *p_hsrvdbg, char* str);

char*
tbl_hsrv_debug_key_value_dump(tbl_hsrv_debug_t *p_hsrvdbg, char* str);

char*
tbl_hsrv_debug_field_name_dump(tbl_hsrv_debug_t *p_hsrvdbg, int32 field_id, char* str);

char*
tbl_hsrv_debug_field_value_dump(tbl_hsrv_debug_t *p_hsrvdbg, int32 field_id, char* str);

char**
tbl_hsrv_debug_table_dump(tbl_hsrv_debug_t *p_hsrvdbg, char** str);

int32
tbl_hsrv_debug_table_parser(char** array, char* key_value, tbl_hsrv_debug_t *p_hsrvdbg);

int32
tbl_hsrv_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_hsrv_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_hsrv_debug_t*
tbl_hsrv_debug_init_hsrv_debug();

#endif /* !__TBL_HSRV_DEBUG_H__ */

