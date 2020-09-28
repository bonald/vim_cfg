
#ifndef __TBL_REBOOT_INFO_DEFINE_H__
#define __TBL_REBOOT_INFO_DEFINE_H__

/* TBL_REBOOT_INFO field defines */
typedef enum
{
    TBL_REBOOT_INFO_FLD_KEY                  = 0 ,  /* READ */
    TBL_REBOOT_INFO_FLD_REBOOT_TYPE          = 1 ,  /* READ */
    TBL_REBOOT_INFO_FLD_REBOOT_TIME          = 2 ,  /* READ */
    TBL_REBOOT_INFO_FLD_MAX                  = 3 
} tbl_reboot_info_field_id_t;

/* TBL_REBOOT_INFO defines */
typedef struct
{
    uint32               index;
} tbl_reboot_info_key_t;

typedef struct
{
    tbl_reboot_info_key_t key;
    char                 reboot_type[REBOOT_STR_SIZE];
    char                 reboot_time[REBOOT_STR_SIZE];
} tbl_reboot_info_t;

typedef struct
{
    ctclib_hash_t        *reboot_hash;
    ctclib_slist_t       *reboot_list;
} tbl_reboot_info_master_t;

#endif /* !__TBL_REBOOT_INFO_DEFINE_H__ */

