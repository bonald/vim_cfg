
#ifndef __TBL_FEA_BRG_IF_DEFINE_H__
#define __TBL_FEA_BRG_IF_DEFINE_H__

/* TBL_FEA_BRG_IF field defines */
typedef enum
{
    TBL_FEA_BRG_IF_FLD_KEY                  = 0 ,  /* READ */
    TBL_FEA_BRG_IF_FLD_FLAGS                = 1 ,  /* READ */
    TBL_FEA_BRG_IF_FLD_LAG_ID               = 2 ,  /* READ */
    TBL_FEA_BRG_IF_FLD_MAC_LEARNING_EN      = 3 ,  /* READ */
    TBL_FEA_BRG_IF_FLD_PORT_SECURITY_EN     = 4 ,  /* READ */
    TBL_FEA_BRG_IF_FLD_MAX_MAC              = 5 ,  /* READ */
    TBL_FEA_BRG_IF_FLD_CURR_MAC             = 6 ,  /* READ */
    TBL_FEA_BRG_IF_FLD_CURR_MAC_STATIC      = 7 ,  /* READ */
    TBL_FEA_BRG_IF_FLD_MLAG_ID              = 8 ,  /* READ */
    TBL_FEA_BRG_IF_FLD_MLAG_PEER_CONF       = 9 ,  /* READ */
    TBL_FEA_BRG_IF_FLD_MLAG_PEER_IF_UP      = 10,  /* READ */
    TBL_FEA_BRG_IF_FLD_MLAG_IS_GROUP        = 11,  /* READ */
    TBL_FEA_BRG_IF_FLD_IS_MIRROR_DEST       = 12,  /* READ */
    TBL_FEA_BRG_IF_FLD_HW_TYPE              = 13,  /* READ */
    TBL_FEA_BRG_IF_FLD_PORTID               = 14,  /* READ */
    TBL_FEA_BRG_IF_FLD_DISABLE_LEARN        = 15,  /* READ */
    TBL_FEA_BRG_IF_FLD_MAX                  = 16
} tbl_fea_brg_if_field_id_t;

/* TBL_FEA_BRG_IF defines */
typedef struct
{
    uint32               ifindex;
} tbl_fea_brg_if_key_t;

typedef struct
{
    tbl_fea_brg_if_key_t key;
    uint32               flags;               /* copy from TBL_INTERFACE flags */
    uint32               lag_id;              /* non-zero indicate port is LAG's member port */
    uint32               mac_learning_en;
    uint32               port_security_en;    /* port security enable */
    uint32               max_mac;             /* port security max MAC count */
    int32                curr_mac;            /* port security current MAC count */
    int32                curr_mac_static;     /* port security current static MAC count */
    uint32               mlag_id;             /* mlag ID */
    uint32               mlag_peer_conf;      /* mlag peer conf */
    uint32               mlag_peer_if_up;     /* mlag peer if up */
    uint32               mlag_is_group;       /* mlag is group, if set, FDB on this port should with MLAG flag */
    uint32               is_mirror_dest;      /* whether is mirror dest port*/
    uint32               hw_type;             /* glb_if_type_t */
    uint64               portid;              /* LAGID or PORTID */
    uint8                disable_learn[GLB_STP_INSTANCE_MAX]; /* MSTP instance state for FDB check */
    ctclib_slist_t       *fdb_list;           /* for store fdb in this interface */
    fea_fdb_tailq_head_t fdb_queue;
} tbl_fea_brg_if_t;

typedef struct
{
    ctclib_hash_t        *brgif_hash;
} tbl_fea_brg_if_master_t;

#endif /* !__TBL_FEA_BRG_IF_DEFINE_H__ */

