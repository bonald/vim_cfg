
#ifndef __TBL_STATIC_DNS_DEFINE_H__
#define __TBL_STATIC_DNS_DEFINE_H__

/* TBL_STATIC_DNS field defines */
typedef enum
{
    TBL_STATIC_DNS_FLD_KEY                  = 0 ,  /* RW */
    TBL_STATIC_DNS_FLD_IP_ADDR              = 1 ,  /* RW */
    TBL_STATIC_DNS_FLD_MAX                  = 2 
} tbl_static_dns_field_id_t;

/* TBL_STATIC_DNS defines */
typedef struct
{
    char                 name[DNS_HOSTNAME_SIZE+1];
} tbl_static_dns_key_t;

typedef struct
{
    tbl_static_dns_key_t key;
    addr_t               ip_addr;             /* read-only or read-write */
} tbl_static_dns_t;

typedef struct
{
    ctclib_slist_t       *static_list;        /* no sort */
} tbl_static_dns_master_t;

#endif /* !__TBL_STATIC_DNS_DEFINE_H__ */

