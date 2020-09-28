
#ifndef __TBL_KERNEL_IF_H__
#define __TBL_KERNEL_IF_H__

int32
tbl_kernel_if_add_kernel_if_sync(tbl_kernel_if_t *p_kernel_if, uint32 sync);

int32
tbl_kernel_if_del_kernel_if_sync(tbl_kernel_if_key_t *p_kernel_if_key, uint32 sync);

int32
tbl_kernel_if_set_kernel_if_field_sync(tbl_kernel_if_t *p_kernel_if, tbl_kernel_if_field_id_t field_id, uint32 sync);

int32
tbl_kernel_if_add_kernel_if(tbl_kernel_if_t *p_kernel_if);

int32
tbl_kernel_if_del_kernel_if(tbl_kernel_if_key_t *p_kernel_if_key);

int32
tbl_kernel_if_set_kernel_if_field(tbl_kernel_if_t *p_kernel_if, tbl_kernel_if_field_id_t field_id);

tbl_kernel_if_t*
tbl_kernel_if_get_kernel_if(tbl_kernel_if_key_t *p_kernel_if_key);

char*
tbl_kernel_if_key_val2str(tbl_kernel_if_t *p_kernel_if, char *str, uint32 str_len);

int32
tbl_kernel_if_key_str2val(char *str, tbl_kernel_if_t *p_kernel_if);

int32
tbl_kernel_if_dump_one(tbl_kernel_if_t *p_kernel_if, tbl_iter_args_t *pargs);

char*
tbl_kernel_if_key_name_dump(tbl_kernel_if_t *p_kernel_if, char* str);

char*
tbl_kernel_if_key_value_dump(tbl_kernel_if_t *p_kernel_if, char* str);

char*
tbl_kernel_if_field_name_dump(tbl_kernel_if_t *p_kernel_if, int32 field_id, char* str);

char*
tbl_kernel_if_field_value_dump(tbl_kernel_if_t *p_kernel_if, int32 field_id, char* str);

char**
tbl_kernel_if_table_dump(tbl_kernel_if_t *p_kernel_if, char** str);

int32
tbl_kernel_if_table_parser(char** array, char* key_value, tbl_kernel_if_t *p_kernel_if);

int32
tbl_kernel_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_kernel_if_master_t*
tbl_kernel_if_get_master();

tbl_kernel_if_master_t*
tbl_kernel_if_init_kernel_if();

#define CTC_IPV4_ADDR_LEN_IN_BIT        32          /**< IPv4 address length in bit */

#define IPV4_LEN_TO_MASK(mask, len)  \
    {                           \
        (mask) = (len) ? ~((1 << (CTC_IPV4_ADDR_LEN_IN_BIT - (len))) - 1) : 0; \
    }

tbl_kernel_if_t * 
tbl_kernel_if_get_by_name(char *name);

tbl_kernel_if_t *
tbl_kernel_if_check_by_list_loop(addr_ipv4_t *prefix);

tbl_kernel_if_t *
tbl_kernel_if_check_by_ipmask_list_loop(addr_ipv4_t *ip_mask);

#endif /* !__TBL_KERNEL_IF_H__ */

