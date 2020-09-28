
#ifndef __TBL_TRACK_OBJECT_DEFINE_H__
#define __TBL_TRACK_OBJECT_DEFINE_H__

/* TBL_TRACK_OBJECT field defines */
typedef enum
{
    TBL_TRACK_OBJECT_FLD_KEY                  = 0 ,  /* READ */
    TBL_TRACK_OBJECT_FLD_TYPE                 = 1 ,  /* RW */
    TBL_TRACK_OBJECT_FLD_STATE                = 2 ,  /* RW */
    TBL_TRACK_OBJECT_FLD_TRACK_IF             = 3 ,  /* RW */
    TBL_TRACK_OBJECT_FLD_RTR_ENTRYID          = 4 ,  /* RW */
    TBL_TRACK_OBJECT_FLD_DELAY_UP             = 5 ,  /* RW */
    TBL_TRACK_OBJECT_FLD_RUNNING_DELAY_UP     = 6 ,  /* RW */
    TBL_TRACK_OBJECT_FLD_DELAY_DOWN           = 7 ,  /* RW */
    TBL_TRACK_OBJECT_FLD_RUNNING_DELAY_DOWN   = 8 ,  /* RW */
    TBL_TRACK_OBJECT_FLD_TRACKED_VRID_BMP     = 9 ,  /* RW */
    TBL_TRACK_OBJECT_FLD_STATIC_RT_REFCNT     = 10,  /* RW */
    TBL_TRACK_OBJECT_FLD_MAX                  = 11
} tbl_track_object_field_id_t;

/* TBL_TRACK_OBJECT defines */
typedef struct
{
    uint16               trackobj_id;
} tbl_track_object_key_t;

typedef struct
{
    tbl_track_object_key_t key;                 /* object id */
    uint8                type;                /* object type */
    uint8                state;               /* object state */
    char                 track_if[IFNAME_SIZE]; /* Interface name */
    uint16               rtr_entryid;         /* ipsla entry id */
    uint8                delay_up;            /* delay up*/
    uint8                running_delay_up;
    uint8                delay_down;          /* delay down*/
    uint8                running_delay_down;
    vrid_bmp_t           tracked_vrid_bmp;    /* tracked by vrrp session id */
    uint32               static_rt_refcnt;
} tbl_track_object_t;

typedef struct
{
    tbl_track_object_t   *trackobj_array[GLB_TRACK_OBJECT_MAX+1];
} tbl_track_object_master_t;

#endif /* !__TBL_TRACK_OBJECT_DEFINE_H__ */

