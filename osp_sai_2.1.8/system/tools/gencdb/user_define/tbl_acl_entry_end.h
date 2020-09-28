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
