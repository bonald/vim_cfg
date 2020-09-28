
#ifndef __TBL_NS_PORT_FORWARDING_DEFINE_H__
#define __TBL_NS_PORT_FORWARDING_DEFINE_H__

/* TBL_NS_PORT_FORWARDING field defines */
typedef enum
{
    TBL_NS_PORT_FORWARDING_FLD_KEY                  = 0 ,  /* READ */
    TBL_NS_PORT_FORWARDING_FLD_IP                   = 1 ,  /* RW */
    TBL_NS_PORT_FORWARDING_FLD_MAX                  = 2 
} tbl_ns_port_forwarding_field_id_t;

/* TBL_NS_PORT_FORWARDING defines */
typedef ns_port_forwarding_key_t tbl_ns_port_forwarding_key_t;

typedef struct
{
    tbl_ns_port_forwarding_key_t key;
    addr_ipv4_t          ip;
} tbl_ns_port_forwarding_t;

typedef struct
{
    ctclib_hash_t        *ns_port_forwarding_hash;
    ctclib_slist_t       *ns_port_forwarding_list;
} tbl_ns_port_forwarding_master_t;

#endif /* !__TBL_NS_PORT_FORWARDING_DEFINE_H__ */

