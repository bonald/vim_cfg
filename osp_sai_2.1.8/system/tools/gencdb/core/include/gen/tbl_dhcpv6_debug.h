
#ifndef __TBL_DHCPV6_DEBUG_H__
#define __TBL_DHCPV6_DEBUG_H__

int32
tbl_dhcpv6_debug_set_dhcpv6_debug_field_sync(tbl_dhcpv6_debug_t *p_dhcpv6_debug, tbl_dhcpv6_debug_field_id_t field_id, uint32 sync);

int32
tbl_dhcpv6_debug_set_dhcpv6_debug_field(tbl_dhcpv6_debug_t *p_dhcpv6_debug, tbl_dhcpv6_debug_field_id_t field_id);

tbl_dhcpv6_debug_t*
tbl_dhcpv6_debug_get_dhcpv6_debug();

char*
tbl_dhcpv6_debug_key_val2str(tbl_dhcpv6_debug_t *p_dhcpv6_debug, char *str, uint32 str_len);

int32
tbl_dhcpv6_debug_key_str2val(char *str, tbl_dhcpv6_debug_t *p_dhcpv6_debug);

int32
tbl_dhcpv6_debug_dump_one(tbl_dhcpv6_debug_t *p_dhcpv6_debug, tbl_iter_args_t *pargs);

char*
tbl_dhcpv6_debug_key_name_dump(tbl_dhcpv6_debug_t *p_dhcpv6_debug, char* str);

char*
tbl_dhcpv6_debug_key_value_dump(tbl_dhcpv6_debug_t *p_dhcpv6_debug, char* str);

char*
tbl_dhcpv6_debug_field_name_dump(tbl_dhcpv6_debug_t *p_dhcpv6_debug, int32 field_id, char* str);

char*
tbl_dhcpv6_debug_field_value_dump(tbl_dhcpv6_debug_t *p_dhcpv6_debug, int32 field_id, char* str);

char**
tbl_dhcpv6_debug_table_dump(tbl_dhcpv6_debug_t *p_dhcpv6_debug, char** str);

int32
tbl_dhcpv6_debug_table_parser(char** array, char* key_value, tbl_dhcpv6_debug_t *p_dhcpv6_debug);

int32
tbl_dhcpv6_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_dhcpv6_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_dhcpv6_debug_t*
tbl_dhcpv6_debug_init_dhcpv6_debug();

#endif /* !__TBL_DHCPV6_DEBUG_H__ */

