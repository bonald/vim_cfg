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
