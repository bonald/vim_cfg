
#ifndef __TBL_SENSOR_DEFINE_H__
#define __TBL_SENSOR_DEFINE_H__

/* TBL_SENSOR field defines */
typedef enum
{
    TBL_SENSOR_FLD_KEY                  = 0 ,  /* READ */
    TBL_SENSOR_FLD_SLOT                 = 1 ,  /* READ */
    TBL_SENSOR_FLD_TEMPERATURE          = 2 ,  /* READ */
    TBL_SENSOR_FLD_NAME                 = 3 ,  /* READ */
    TBL_SENSOR_FLD_POSITION             = 4 ,  /* READ */
    TBL_SENSOR_FLD_STATUS               = 5 ,  /* READ */
    TBL_SENSOR_FLD_TMPR_LOW             = 6 ,  /* READ */
    TBL_SENSOR_FLD_TMPR_HIGH            = 7 ,  /* READ */
    TBL_SENSOR_FLD_TMPR_CRITICAL        = 8 ,  /* READ */
    TBL_SENSOR_FLD_IS_CHIP              = 9 ,  /* READ */
    TBL_SENSOR_FLD_MAX                  = 10
} tbl_sensor_field_id_t;

/* TBL_SENSOR defines */
typedef struct
{
    int32                id;
} tbl_sensor_key_t;

typedef struct
{
    tbl_sensor_key_t     key;
    int32                slot;
    int32                temperature;
    char                 name[GLB_NAME_INFO_STR_MAX];
    char                 position[GLB_NAME_INFO_STR_MAX];
    char                 status[GLB_NAME_INFO_STR_MAX];
    int32                tmpr_low;
    int32                tmpr_high;
    int32                tmpr_critical;
    uint32               is_chip;
} tbl_sensor_t;

typedef struct
{
    ctclib_hash_t        *sensor_hash;
    ctclib_slist_t       *sensor_list;
} tbl_sensor_master_t;

#endif /* !__TBL_SENSOR_DEFINE_H__ */

