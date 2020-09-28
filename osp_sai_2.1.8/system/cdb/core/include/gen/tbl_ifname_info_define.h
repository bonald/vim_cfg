
#ifndef __TBL_IFNAME_INFO_DEFINE_H__
#define __TBL_IFNAME_INFO_DEFINE_H__

/* TBL_IFNAME_INFO field defines */
typedef enum
{
    TBL_IFNAME_INFO_FLD_CONNECTION           = 0 ,  /* RW */
    TBL_IFNAME_INFO_FLD_SUB_CONNECTION       = 1 ,  /* RW */
    TBL_IFNAME_INFO_FLD_PREFIX_NUM           = 2 ,  /* RW */
    TBL_IFNAME_INFO_FLD_PREFIX               = 3 ,  /* RW */
    TBL_IFNAME_INFO_FLD_FULL_PREFIX          = 4 ,  /* RW */
    TBL_IFNAME_INFO_FLD_MAX                  = 5 
} tbl_ifname_info_field_id_t;

/* TBL_IFNAME_INFO defines */
typedef struct
{
    char                 connection;
    char                 sub_connection;
    char                 prefix_num;
    ifname_info_t        prefix[GLB_PHT_PORT_TYPE_MAX];
    ifname_info_t        full_prefix[GLB_PHT_PORT_TYPE_MAX];
} tbl_ifname_info_t;

typedef struct
{
    tbl_ifname_info_t    *ifname_info_array[GLB_PHT_PORT_TYPE_MAX];
} tbl_ifname_info_master_t;

#endif /* !__TBL_IFNAME_INFO_DEFINE_H__ */

