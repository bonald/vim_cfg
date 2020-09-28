
#ifndef __TBL_FAN_DEFINE_H__
#define __TBL_FAN_DEFINE_H__

/* TBL_FAN field defines */
typedef enum
{
    TBL_FAN_FLD_KEY                  = 0 ,  /* READ */
    TBL_FAN_FLD_SLOT                 = 1 ,  /* READ */
    TBL_FAN_FLD_TRAY                 = 2 ,  /* READ */
    TBL_FAN_FLD_PERCENT              = 3 ,  /* RW */
    TBL_FAN_FLD_ABSENT               = 4 ,  /* RW */
    TBL_FAN_FLD_NAME                 = 5 ,  /* RW */
    TBL_FAN_FLD_DIRECTION            = 6 ,  /* RW */
    TBL_FAN_FLD_STATUS               = 7 ,  /* RW */
    TBL_FAN_FLD_MODE                 = 8 ,  /* RW */
    TBL_FAN_FLD_MAX                  = 9 
} tbl_fan_field_id_t;

/* TBL_FAN defines */
typedef struct
{
    int32                id;
} tbl_fan_key_t;

typedef struct
{
    tbl_fan_key_t        key;
    int32                slot;
    int32                tray;
    int32                percent;
    int32                absent;
    char                 name[GLB_NAME_INFO_STR_MAX];
    char                 direction[GLB_NAME_INFO_STR_MAX];
    char                 status[GLB_NAME_INFO_STR_MAX];
    char                 mode[GLB_NAME_INFO_STR_MAX];
} tbl_fan_t;

typedef struct
{
    ctclib_hash_t        *fan_hash;
    ctclib_slist_t       *fan_list;
} tbl_fan_master_t;

#endif /* !__TBL_FAN_DEFINE_H__ */

