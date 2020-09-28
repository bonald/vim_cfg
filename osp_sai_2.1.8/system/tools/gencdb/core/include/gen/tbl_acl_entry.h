
#ifndef __TBL_ACL_ENTRY_H__
#define __TBL_ACL_ENTRY_H__

int32
tbl_acl_entry_add_acl_entry_sync(tbl_acl_entry_t *p_acl_entry, uint32 sync);

int32
tbl_acl_entry_del_acl_entry_sync(tbl_acl_entry_key_t *p_acl_entry_key, uint32 sync);

int32
tbl_acl_entry_set_acl_entry_field_sync(tbl_acl_entry_t *p_acl_entry, tbl_acl_entry_field_id_t field_id, uint32 sync);

int32
tbl_acl_entry_add_acl_entry(tbl_acl_entry_t *p_acl_entry);

int32
tbl_acl_entry_del_acl_entry(tbl_acl_entry_key_t *p_acl_entry_key);

int32
tbl_acl_entry_set_acl_entry_field(tbl_acl_entry_t *p_acl_entry, tbl_acl_entry_field_id_t field_id);

tbl_acl_entry_t*
tbl_acl_entry_get_acl_entry(tbl_acl_entry_key_t *p_acl_entry_key);

char*
tbl_acl_entry_key_val2str(tbl_acl_entry_t *p_acl_entry, char *str, uint32 str_len);

int32
tbl_acl_entry_key_str2val(char *str, tbl_acl_entry_t *p_acl_entry);

int32
tbl_acl_entry_dump_one(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs);

char*
tbl_acl_entry_key_name_dump(tbl_acl_entry_t *p_acl_entry, char* str);

char*
tbl_acl_entry_key_value_dump(tbl_acl_entry_t *p_acl_entry, char* str);

char*
tbl_acl_entry_field_name_dump(tbl_acl_entry_t *p_acl_entry, int32 field_id, char* str);

char*
tbl_acl_entry_field_value_dump(tbl_acl_entry_t *p_acl_entry, int32 field_id, char* str);

char**
tbl_acl_entry_table_dump(tbl_acl_entry_t *p_acl_entry, char** str);

int32
tbl_acl_entry_table_parser(char** array, char* key_value, tbl_acl_entry_t *p_acl_entry);

int32
tbl_acl_entry_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_acl_entry_master_t*
tbl_acl_entry_get_master();

tbl_acl_entry_master_t*
tbl_acl_entry_init_acl_entry();

int32
tbl_acl_entry_get_acl_entry_count();

int32
tbl_acl_entry_get_count(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs);

int32
tbl_acl_entry_get_entry_dir_count(uint32* cnt_ingress, uint32* cnt_egress, uint32* cnt_worm_filter);

typedef struct acl_entry_count_s{
	uint32	cur_ingress;
	uint32  cur_ingress6;
	uint32  cur_egress;
	uint32  cur_egress6;
	uint32  cur_worm_filter;
	uint32  cur_worm_filter6;
}acl_entry_count_t;

int32
tbl_acl_entry_get_entry_dir_count_1(acl_entry_count_t *entry_count);

#endif /* !__TBL_ACL_ENTRY_H__ */

