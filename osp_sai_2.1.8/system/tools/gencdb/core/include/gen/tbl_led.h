
#ifndef __TBL_LED_H__
#define __TBL_LED_H__

int32
tbl_led_add_led_sync(tbl_led_t *p_led, uint32 sync);

int32
tbl_led_del_led_sync(tbl_led_key_t *p_led_key, uint32 sync);

int32
tbl_led_set_led_field_sync(tbl_led_t *p_led, tbl_led_field_id_t field_id, uint32 sync);

int32
tbl_led_add_led(tbl_led_t *p_led);

int32
tbl_led_del_led(tbl_led_key_t *p_led_key);

int32
tbl_led_set_led_field(tbl_led_t *p_led, tbl_led_field_id_t field_id);

tbl_led_t*
tbl_led_get_led(tbl_led_key_t *p_led_key);

char*
tbl_led_key_val2str(tbl_led_t *p_led, char *str, uint32 str_len);

int32
tbl_led_key_str2val(char *str, tbl_led_t *p_led);

int32
tbl_led_dump_one(tbl_led_t *p_led, tbl_iter_args_t *pargs);

char*
tbl_led_key_name_dump(tbl_led_t *p_led, char* str);

char*
tbl_led_key_value_dump(tbl_led_t *p_led, char* str);

char*
tbl_led_field_name_dump(tbl_led_t *p_led, int32 field_id, char* str);

char*
tbl_led_field_value_dump(tbl_led_t *p_led, int32 field_id, char* str);

char**
tbl_led_table_dump(tbl_led_t *p_led, char** str);

int32
tbl_led_table_parser(char** array, char* key_value, tbl_led_t *p_led);

int32
tbl_led_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_led_master_t*
tbl_led_get_master();

tbl_led_master_t*
tbl_led_init_led();

#endif /* !__TBL_LED_H__ */

