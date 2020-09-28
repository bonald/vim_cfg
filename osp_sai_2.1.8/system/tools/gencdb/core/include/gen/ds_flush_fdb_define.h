
#ifndef __DS_FLUSH_FDB_DEFINE_H__
#define __DS_FLUSH_FDB_DEFINE_H__

/* DS_FLUSH_FDB field defines */
typedef enum
{
    DS_FLUSH_FDB_FLD_TYPE                 = 0 ,  /* RW */
    DS_FLUSH_FDB_FLD_SUBTYPE              = 1 ,  /* RW */
    DS_FLUSH_FDB_FLD_MODE                 = 2 ,  /* RW */
    DS_FLUSH_FDB_FLD_IFINDEX              = 3 ,  /* RW */
    DS_FLUSH_FDB_FLD_VID                  = 4 ,  /* RW */
    DS_FLUSH_FDB_FLD_MAC                  = 5 ,  /* RW */
    DS_FLUSH_FDB_FLD_MAX                  = 6 
} ds_flush_fdb_field_id_t;

/* DS_FLUSH_FDB defines */
typedef struct
{
    uint32               type;                /* flush_fdb_type_t */
    uint32               subtype;             /* flush_fdb_subtype_t */
    uint32               mode;                /* flush_fdb_mode_t */
    uint32               ifindex;
    vid_t                vid;
    mac_addr_t           mac;
} ds_flush_fdb_t;

#endif /* !__DS_FLUSH_FDB_DEFINE_H__ */

