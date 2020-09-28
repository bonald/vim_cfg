
#ifndef __TBL_LLDP_IF_H__
#define __TBL_LLDP_IF_H__

#include "gen/ds_lldp_neighbour.h"

int32
tbl_lldp_if_add_lldp_if_sync(tbl_lldp_if_t *p_lldp_if, uint32 sync);

int32
tbl_lldp_if_del_lldp_if_sync(tbl_lldp_if_key_t *p_lldp_if_key, uint32 sync);

int32
tbl_lldp_if_set_lldp_if_field_sync(tbl_lldp_if_t *p_lldp_if, tbl_lldp_if_field_id_t field_id, uint32 sync);

int32
tbl_lldp_if_add_lldp_if(tbl_lldp_if_t *p_lldp_if);

int32
tbl_lldp_if_del_lldp_if(tbl_lldp_if_key_t *p_lldp_if_key);

int32
tbl_lldp_if_set_lldp_if_field(tbl_lldp_if_t *p_lldp_if, tbl_lldp_if_field_id_t field_id);

tbl_lldp_if_t*
tbl_lldp_if_get_lldp_if(tbl_lldp_if_key_t *p_lldp_if_key);

char*
tbl_lldp_if_key_val2str(tbl_lldp_if_t *p_lldp_if, char *str, uint32 str_len);

int32
tbl_lldp_if_key_str2val(char *str, tbl_lldp_if_t *p_lldp_if);

int32
tbl_lldp_if_dump_one(tbl_lldp_if_t *p_lldp_if, tbl_iter_args_t *pargs);

char*
tbl_lldp_if_key_name_dump(tbl_lldp_if_t *p_lldp_if, char* str);

char*
tbl_lldp_if_key_value_dump(tbl_lldp_if_t *p_lldp_if, char* str);

char*
tbl_lldp_if_field_name_dump(tbl_lldp_if_t *p_lldp_if, int32 field_id, char* str);

char*
tbl_lldp_if_field_value_dump(tbl_lldp_if_t *p_lldp_if, int32 field_id, char* str);

char**
tbl_lldp_if_table_dump(tbl_lldp_if_t *p_lldp_if, char** str);

int32
tbl_lldp_if_table_parser(char** array, char* key_value, tbl_lldp_if_t *p_lldp_if);

int32
tbl_lldp_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_lldp_if_master_t*
tbl_lldp_if_get_master();

tbl_lldp_if_master_t*
tbl_lldp_if_init_lldp_if();

#endif /* !__TBL_LLDP_IF_H__ */

