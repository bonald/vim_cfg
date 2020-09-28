
#ifndef __SDB_DEFINE_H__
#define __SDB_DEFINE_H__

typedef enum
{
    SDIR_SDB             = 0, /* NULL */
    SDIR_SAI             = 1, /* SDIR_SDB */
    SDIR_OF              = 2, /* SDIR_SDB */
    SDIR_COMMON          = 3, /* SDIR_SAI */
    SDIR_L2              = 4, /* SDIR_SAI */
    SDIR_L3              = 5, /* SDIR_SAI */
    SDIR_ACL             = 6, /* SDIR_SAI */
    SDIR_QOS             = 7, /* SDIR_SAI */
    SDIR_MAX
} sdb_dir_t;

typedef enum
{
    STBL_SAI_GLB         = 0, /* SDIR_SAI */
    STBL_PORT_GLB        = 1, /* SDIR_SAI */
    STBL_PORT            = 2, /* SDIR_SAI */
    STBL_HOSTIF          = 3, /* SDIR_SAI */
    STBL_LAG_MEMPORT     = 4, /* SDIR_SAI */
    STBL_VR              = 5, /* SDIR_L3 */
    STBL_RIF             = 6, /* SDIR_SAI */
    STBL_NEXTHOP         = 7, /* SDIR_SAI */
    STBL_NEXTHOP_GROUP   = 8, /* SDIR_SAI */
    STBL_NEIGHBOR        = 9, /* SDIR_L3 */
    STBL_VLAN            = 10, /* SDIR_L2 */
    STBL_VLAN_CLASS      = 11, /* SDIR_L2 */
    STBL_STP             = 12, /* SDIR_L2 */
    STBL_L2MC            = 13, /* SDIR_L2 */
    STBL_ROUTE           = 14, /* SDIR_L3 */
    STBL_VRRP_VIP        = 15, /* SDIR_L3 */
    STBL_COPP_TRAP       = 16, /* SDIR_SAI */
    STBL_COPP_GROUP      = 17, /* SDIR_SAI */
    STBL_COPP_REASON     = 18, /* SDIR_SAI */
    STBL_COPP_ENTRY      = 19, /* SDIR_SAI */
    STBL_ACL_GLB         = 20, /* SDIR_SAI */
    STBL_ACL_PORT        = 21, /* SDIR_SAI */
    STBL_ACL_TBL         = 22, /* SDIR_SAI */
    STBL_ACL_ENTRY       = 23, /* SDIR_SAI */
    STBL_ACL_TUNNEL      = 24, /* SDIR_SAI */
    STBL_ACL_UDF_ENTRY   = 25, /* SDIR_SAI */
    STBL_IPMC            = 26, /* SDIR_SAI */
    STBL_NAT_FORWARD     = 27, /* SDIR_SAI */
    STBL_QOS_MPLS_DOMAIN = 28, /* SDIR_SAI */
    STBL_QOS_MPLS_FLOW   = 29, /* SDIR_OF */
    STBL_CFM_LMEP        = 30, /* SDIR_SAI */
    STBL_CFM_RMEP        = 31, /* SDIR_SAI */
    STBL_MAX
} sdb_tbl_t;

#endif /* !__SDB_DEFINE_H__ */

