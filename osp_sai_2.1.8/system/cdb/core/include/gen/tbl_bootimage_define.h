
#ifndef __TBL_BOOTIMAGE_DEFINE_H__
#define __TBL_BOOTIMAGE_DEFINE_H__

/* TBL_BOOTIMAGE field defines */
typedef enum
{
    TBL_BOOTIMAGE_FLD_MODE                 = 0 ,  /* RW */
    TBL_BOOTIMAGE_FLD_FAST                 = 1 ,  /* RW */
    TBL_BOOTIMAGE_FLD_IMAGE                = 2 ,  /* RW */
    TBL_BOOTIMAGE_FLD_SERVERIP             = 3 ,  /* RW */
    TBL_BOOTIMAGE_FLD_IPADDR               = 4 ,  /* RW */
    TBL_BOOTIMAGE_FLD_BOOTCMD              = 5 ,  /* RW */
    TBL_BOOTIMAGE_FLD_CURRENT_IMAGE        = 6 ,  /* RW */
    TBL_BOOTIMAGE_FLD_MAX                  = 7 
} tbl_bootimage_field_id_t;

/* TBL_BOOTIMAGE defines */
typedef struct
{
    uint32               mode;                /* boot_mode_t */
    uint32               fast;                /* 1: boot fast, will decompression ramfs to memory ; 0: boot system */
    char                 image[SYS_INFO_SIZE];
    char                 serverip[SYS_INFO_SIZE];
    char                 ipaddr[SYS_INFO_SIZE];
    char                 bootcmd[M_FULLPATH_MAX_LEN];
    char                 current_image[M_FULLPATH_MAX_LEN]; /*Note, this field must be the last one, because it include '/'*/
} tbl_bootimage_t;

#endif /* !__TBL_BOOTIMAGE_DEFINE_H__ */

