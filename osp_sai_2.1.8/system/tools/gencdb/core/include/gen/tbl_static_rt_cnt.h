
#ifndef __TBL_STATIC_RT_CNT_H__
#define __TBL_STATIC_RT_CNT_H__

int32
tbl_static_rt_cnt_add_static_rt_cnt_sync(tbl_static_rt_cnt_t *p_rt_cnt, uint32 sync);

int32
tbl_static_rt_cnt_del_static_rt_cnt_sync(tbl_static_rt_cnt_key_t *p_rt_cnt_key, uint32 sync);

int32
tbl_static_rt_cnt_set_static_rt_cnt_field_sync(tbl_static_rt_cnt_t *p_rt_cnt, tbl_static_rt_cnt_field_id_t field_id, uint32 sync);

int32
tbl_static_rt_cnt_add_static_rt_cnt(tbl_static_rt_cnt_t *p_rt_cnt);

int32
tbl_static_rt_cnt_del_static_rt_cnt(tbl_static_rt_cnt_key_t *p_rt_cnt_key);

int32
tbl_static_rt_cnt_set_static_rt_cnt_field(tbl_static_rt_cnt_t *p_rt_cnt, tbl_static_rt_cnt_field_id_t field_id);

tbl_static_rt_cnt_t*
tbl_static_rt_cnt_get_static_rt_cnt(tbl_static_rt_cnt_key_t *p_rt_cnt_key);

char*
tbl_static_rt_cnt_key_val2str(tbl_static_rt_cnt_t *p_rt_cnt, char *str, uint32 str_len);

int32
tbl_static_rt_cnt_key_str2val(char *str, tbl_static_rt_cnt_t *p_rt_cnt);

int32
tbl_static_rt_cnt_dump_one(tbl_static_rt_cnt_t *p_rt_cnt, tbl_iter_args_t *pargs);

char*
tbl_static_rt_cnt_key_name_dump(tbl_static_rt_cnt_t *p_rt_cnt, char* str);

char*
tbl_static_rt_cnt_key_value_dump(tbl_static_rt_cnt_t *p_rt_cnt, char* str);

char*
tbl_static_rt_cnt_field_name_dump(tbl_static_rt_cnt_t *p_rt_cnt, int32 field_id, char* str);

char*
tbl_static_rt_cnt_field_value_dump(tbl_static_rt_cnt_t *p_rt_cnt, int32 field_id, char* str);

char**
tbl_static_rt_cnt_table_dump(tbl_static_rt_cnt_t *p_rt_cnt, char** str);

int32
tbl_static_rt_cnt_table_parser(char** array, char* key_value, tbl_static_rt_cnt_t *p_rt_cnt);

int32
tbl_static_rt_cnt_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_static_rt_cnt_master_t*
tbl_static_rt_cnt_get_master();

tbl_static_rt_cnt_master_t*
tbl_static_rt_cnt_init_static_rt_cnt();

#endif /* !__TBL_STATIC_RT_CNT_H__ */

