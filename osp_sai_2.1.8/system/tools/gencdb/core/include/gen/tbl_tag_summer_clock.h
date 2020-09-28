
#ifndef __TBL_TAG_SUMMER_CLOCK_H__
#define __TBL_TAG_SUMMER_CLOCK_H__

int32
tbl_tag_summer_clock_set_tag_summer_clock_field_sync(tbl_tag_summer_clock_t *p_tag_summer_clk, tbl_tag_summer_clock_field_id_t field_id, uint32 sync);

int32
tbl_tag_summer_clock_set_tag_summer_clock_field(tbl_tag_summer_clock_t *p_tag_summer_clk, tbl_tag_summer_clock_field_id_t field_id);

tbl_tag_summer_clock_t*
tbl_tag_summer_clock_get_tag_summer_clock();

char*
tbl_tag_summer_clock_key_val2str(tbl_tag_summer_clock_t *p_tag_summer_clk, char *str, uint32 str_len);

int32
tbl_tag_summer_clock_key_str2val(char *str, tbl_tag_summer_clock_t *p_tag_summer_clk);

int32
tbl_tag_summer_clock_dump_one(tbl_tag_summer_clock_t *p_tag_summer_clk, tbl_iter_args_t *pargs);

char*
tbl_tag_summer_clock_key_name_dump(tbl_tag_summer_clock_t *p_tag_summer_clk, char* str);

char*
tbl_tag_summer_clock_key_value_dump(tbl_tag_summer_clock_t *p_tag_summer_clk, char* str);

char*
tbl_tag_summer_clock_field_name_dump(tbl_tag_summer_clock_t *p_tag_summer_clk, int32 field_id, char* str);

char*
tbl_tag_summer_clock_field_value_dump(tbl_tag_summer_clock_t *p_tag_summer_clk, int32 field_id, char* str);

char**
tbl_tag_summer_clock_table_dump(tbl_tag_summer_clock_t *p_tag_summer_clk, char** str);

int32
tbl_tag_summer_clock_table_parser(char** array, char* key_value, tbl_tag_summer_clock_t *p_tag_summer_clk);

int32
tbl_tag_summer_clock_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_tag_summer_clock_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_tag_summer_clock_t*
tbl_tag_summer_clock_init_tag_summer_clock();

#endif /* !__TBL_TAG_SUMMER_CLOCK_H__ */

