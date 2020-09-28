
#ifndef __TBL_LED_DEFINE_H__
#define __TBL_LED_DEFINE_H__

/* TBL_LED field defines */
typedef enum
{
    TBL_LED_FLD_KEY                  = 0 ,  /* READ */
    TBL_LED_FLD_SLOT                 = 1 ,  /* READ */
    TBL_LED_FLD_NAME                 = 2 ,  /* READ */
    TBL_LED_FLD_STATUS               = 3 ,  /* RW */
    TBL_LED_FLD_MODE                 = 4 ,  /* RW */
    TBL_LED_FLD_MAX                  = 5 
} tbl_led_field_id_t;

/* TBL_LED defines */
typedef struct
{
    int32                id;
} tbl_led_key_t;

typedef struct
{
    tbl_led_key_t        key;
    int32                slot;
    char                 name[GLB_NAME_INFO_STR_MAX];
    char                 status[GLB_NAME_INFO_STR_MAX];
    char                 mode[GLB_NAME_INFO_STR_MAX];
} tbl_led_t;

typedef struct
{
    ctclib_hash_t        *led_hash;
    ctclib_slist_t       *led_list;
} tbl_led_master_t;

#endif /* !__TBL_LED_DEFINE_H__ */

