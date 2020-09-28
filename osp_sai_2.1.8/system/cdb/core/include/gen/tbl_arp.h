
#ifndef __TBL_ARP_H__
#define __TBL_ARP_H__

#include "gen/ds_static_route_dup_ipv4_nh.h"

int32
tbl_arp_add_arp_sync(tbl_arp_t *p_arp, uint32 sync);

int32
tbl_arp_del_arp_sync(tbl_arp_key_t *p_arp_key, uint32 sync);

int32
tbl_arp_set_arp_field_sync(tbl_arp_t *p_arp, tbl_arp_field_id_t field_id, uint32 sync);

int32
tbl_arp_add_arp(tbl_arp_t *p_arp);

int32
tbl_arp_del_arp(tbl_arp_key_t *p_arp_key);

int32
tbl_arp_set_arp_field(tbl_arp_t *p_arp, tbl_arp_field_id_t field_id);

tbl_arp_t*
tbl_arp_get_arp(tbl_arp_key_t *p_arp_key);

char*
tbl_arp_key_val2str(tbl_arp_t *p_arp, char *str, uint32 str_len);

int32
tbl_arp_key_str2val(char *str, tbl_arp_t *p_arp);

int32
tbl_arp_dump_one(tbl_arp_t *p_arp, tbl_iter_args_t *pargs);

char*
tbl_arp_key_name_dump(tbl_arp_t *p_arp, char* str);

char*
tbl_arp_key_value_dump(tbl_arp_t *p_arp, char* str);

char*
tbl_arp_field_name_dump(tbl_arp_t *p_arp, int32 field_id, char* str);

char*
tbl_arp_field_value_dump(tbl_arp_t *p_arp, int32 field_id, char* str);

char**
tbl_arp_table_dump(tbl_arp_t *p_arp, char** str);

int32
tbl_arp_table_parser(char** array, char* key_value, tbl_arp_t *p_arp);

int32
tbl_arp_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_arp_master_t*
tbl_arp_get_master();

tbl_arp_master_t*
tbl_arp_init_arp();

#endif /* !__TBL_ARP_H__ */

