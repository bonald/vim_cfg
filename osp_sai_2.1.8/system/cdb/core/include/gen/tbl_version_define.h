
#ifndef __TBL_VERSION_DEFINE_H__
#define __TBL_VERSION_DEFINE_H__

/* TBL_VERSION field defines */
typedef enum
{
    TBL_VERSION_FLD_VERSION              = 0 ,  /* READ */
    TBL_VERSION_FLD_COMPANY              = 1 ,  /* READ */
    TBL_VERSION_FLD_PACKAGE              = 2 ,  /* READ */
    TBL_VERSION_FLD_PRODUCT              = 3 ,  /* READ */
    TBL_VERSION_FLD_HW_TYPE              = 4 ,  /* READ */
    TBL_VERSION_FLD_IS_CST               = 5 ,  /* READ */
    TBL_VERSION_FLD_CST_COMPANY          = 6 ,  /* READ */
    TBL_VERSION_FLD_CST_HW_TYPE          = 7 ,  /* READ */
    TBL_VERSION_FLD_CST_HW_INFO          = 8 ,  /* READ */
    TBL_VERSION_FLD_CST_HW_VER           = 9 ,  /* READ */
    TBL_VERSION_FLD_CST_SW_VER           = 10,  /* READ */
    TBL_VERSION_FLD_MAX                  = 11
} tbl_version_field_id_t;

/* TBL_VERSION defines */
typedef struct
{
    char                 version[SYS_INFO_SIZE];
    char                 company[SYS_INFO_SIZE];
    char                 package[SYS_INFO_SIZE];
    char                 product[SYS_INFO_SIZE];
    char                 hw_type[SYS_INFO_SIZE];
    uint8                is_cst;
    char                 cst_company[SYS_INFO_SIZE];
    char                 cst_hw_type[SYS_INFO_SIZE];
    char                 cst_hw_info[SYS_INFO_SIZE];
    char                 cst_hw_ver[SYS_INFO_SIZE];
    char                 cst_sw_ver[SYS_INFO_SIZE];
} tbl_version_t;

#endif /* !__TBL_VERSION_DEFINE_H__ */

