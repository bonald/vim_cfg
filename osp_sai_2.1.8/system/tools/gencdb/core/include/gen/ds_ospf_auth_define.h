
#ifndef __DS_OSPF_AUTH_DEFINE_H__
#define __DS_OSPF_AUTH_DEFINE_H__

/* DS_OSPF_AUTH field defines */
typedef enum
{
    DS_OSPF_AUTH_FLD_KEY                  = 0 ,  /* READ */
    DS_OSPF_AUTH_FLD_OSPF_AUTH_MD5_KEY    = 1 ,  /* RW */
    DS_OSPF_AUTH_FLD_MAX                  = 2 
} ds_ospf_auth_field_id_t;

/* DS_OSPF_AUTH defines */
typedef struct
{
    uint32               id;
} ds_ospf_auth_key_t;

typedef struct
{
    ds_ospf_auth_key_t   key;
    char                 ospf_auth_md5_key[17];
} ds_ospf_auth_t;

#endif /* !__DS_OSPF_AUTH_DEFINE_H__ */

