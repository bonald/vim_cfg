
#ifndef __STBL_HOSTIF_DEFINE_H__
#define __STBL_HOSTIF_DEFINE_H__

/* STBL_HOSTIF field defines */
typedef enum
{
    STBL_HOSTIF_FLD_KEY                  = 0 ,  /* READ */
    STBL_HOSTIF_FLD_PORT_OID             = 1 ,  /* READ */
    STBL_HOSTIF_FLD_VID                  = 2 ,  /* READ */
    STBL_HOSTIF_FLD_FD                   = 3 ,  /* READ */
    STBL_HOSTIF_FLD_NEXTHOP_PTR          = 4 ,  /* READ */
    STBL_HOSTIF_FLD_IFNAME               = 5 ,  /* READ */
    STBL_HOSTIF_FLD_MAX                  = 6 
} stbl_hostif_field_id_t;

/* STBL_HOSTIF defines */
typedef struct
{
    sai_object_id_t      hif_oid;
} stbl_hostif_key_t;

typedef struct
{
    stbl_hostif_key_t    key;
    sai_object_id_t      port_oid;            /* for port interface */
    vid_t                vid;                 /* for VLAN interface */
    int32                fd;                  /* for VLAN interface */
    uint32               nexthop_ptr;         /* for VLAN interface send packet */
    char                 ifname[GLB_IFNAME_SZ];
} stbl_hostif_t;

typedef struct
{
    ctclib_hash_t        *hif_hash;
    ctclib_slist_t       *if_list;
} stbl_hostif_master_t;

#endif /* !__STBL_HOSTIF_DEFINE_H__ */

