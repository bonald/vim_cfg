
#ifndef __TBL_DYNAMIC_DNS_SERVER_DEFINE_H__
#define __TBL_DYNAMIC_DNS_SERVER_DEFINE_H__

/* TBL_DYNAMIC_DNS_SERVER field defines */
typedef enum
{
    TBL_DYNAMIC_DNS_SERVER_FLD_KEY                  = 0 ,  /* RW */
    TBL_DYNAMIC_DNS_SERVER_FLD_MAX                  = 1 
} tbl_dynamic_dns_server_field_id_t;

/* TBL_DYNAMIC_DNS_SERVER defines */
typedef struct
{
    addr_t               ip_addr;
} tbl_dynamic_dns_server_key_t;

typedef struct
{
    tbl_dynamic_dns_server_key_t key;
} tbl_dynamic_dns_server_t;

typedef struct
{
    ctclib_slist_t       *dynamic_server_list; /* no sort */
} tbl_dynamic_dns_server_master_t;

#endif /* !__TBL_DYNAMIC_DNS_SERVER_DEFINE_H__ */

