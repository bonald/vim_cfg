
#ifndef __TBL_L2_ACTION_H__
#define __TBL_L2_ACTION_H__

#include "gen/ds_flush_fdb.h"

int32
tbl_l2_action_set_l2_action_field_sync(tbl_l2_action_t *p_l2_act, tbl_l2_action_field_id_t field_id, uint32 sync);

int32
tbl_l2_action_set_l2_action_field(tbl_l2_action_t *p_l2_act, tbl_l2_action_field_id_t field_id);

tbl_l2_action_t*
tbl_l2_action_get_l2_action();

char*
tbl_l2_action_key_val2str(tbl_l2_action_t *p_l2_act, char *str, uint32 str_len);

int32
tbl_l2_action_key_str2val(char *str, tbl_l2_action_t *p_l2_act);

int32
tbl_l2_action_dump_one(tbl_l2_action_t *p_l2_act, tbl_iter_args_t *pargs);

char*
tbl_l2_action_key_name_dump(tbl_l2_action_t *p_l2_act, char* str);

char*
tbl_l2_action_key_value_dump(tbl_l2_action_t *p_l2_act, char* str);

char*
tbl_l2_action_field_name_dump(tbl_l2_action_t *p_l2_act, int32 field_id, char* str);

char*
tbl_l2_action_field_value_dump(tbl_l2_action_t *p_l2_act, int32 field_id, char* str);

char**
tbl_l2_action_table_dump(tbl_l2_action_t *p_l2_act, char** str);

int32
tbl_l2_action_table_parser(char** array, char* key_value, tbl_l2_action_t *p_l2_act);

int32
tbl_l2_action_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_l2_action_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_l2_action_t*
tbl_l2_action_init_l2_action();

#endif /* !__TBL_L2_ACTION_H__ */

