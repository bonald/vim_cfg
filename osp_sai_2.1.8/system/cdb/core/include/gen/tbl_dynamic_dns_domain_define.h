
#ifndef __TBL_DYNAMIC_DNS_DOMAIN_DEFINE_H__
#define __TBL_DYNAMIC_DNS_DOMAIN_DEFINE_H__

/* TBL_DYNAMIC_DNS_DOMAIN field defines */
typedef enum
{
    TBL_DYNAMIC_DNS_DOMAIN_FLD_KEY                  = 0 ,  /* RW */
    TBL_DYNAMIC_DNS_DOMAIN_FLD_MAX                  = 1 
} tbl_dynamic_dns_domain_field_id_t;

/* TBL_DYNAMIC_DNS_DOMAIN defines */
typedef struct
{
    char                 name[DNS_HOSTNAME_SIZE+1];
} tbl_dynamic_dns_domain_key_t;

typedef struct
{
    tbl_dynamic_dns_domain_key_t key;
} tbl_dynamic_dns_domain_t;

typedef struct
{
    ctclib_slist_t       *dynamic_domain_list; /* no sort */
} tbl_dynamic_dns_domain_master_t;

#endif /* !__TBL_DYNAMIC_DNS_DOMAIN_DEFINE_H__ */

