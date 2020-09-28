
#ifndef __SDB_DIR_PRIV_H__
#define __SDB_DIR_PRIV_H__

sdb_node_t g_sdb_dir_node_info[SDIR_MAX] =
{
    {
        SDIR_SDB,
        SDB_NODE_DIR,
        SDB_PRODUCT_ALL,
        "sdb",
        "/sdb",
        NULL,
    },
    {
        SDIR_SAI,
        SDB_NODE_DIR,
        SDB_PRODUCT_ALL,
        "sai",
        "/sdb/sai",
        &g_sdb_dir_node_info[SDIR_SDB],
    },
    {
        SDIR_OF,
        SDB_NODE_DIR,
        SDB_PRODUCT_ALL,
        "of",
        "/sdb/of",
        &g_sdb_dir_node_info[SDIR_SDB],
    },
    {
        SDIR_COMMON,
        SDB_NODE_DIR,
        SDB_PRODUCT_ALL,
        "common",
        "/sdb/sai/common",
        &g_sdb_dir_node_info[SDIR_SAI],
    },
    {
        SDIR_L2,
        SDB_NODE_DIR,
        SDB_PRODUCT_ALL,
        "l2",
        "/sdb/sai/l2",
        &g_sdb_dir_node_info[SDIR_SAI],
    },
    {
        SDIR_L3,
        SDB_NODE_DIR,
        SDB_PRODUCT_ALL,
        "l3",
        "/sdb/sai/l3",
        &g_sdb_dir_node_info[SDIR_SAI],
    },
    {
        SDIR_ACL,
        SDB_NODE_DIR,
        SDB_PRODUCT_ALL,
        "acl",
        "/sdb/sai/acl",
        &g_sdb_dir_node_info[SDIR_SAI],
    },
    {
        SDIR_QOS,
        SDB_NODE_DIR,
        SDB_PRODUCT_ALL,
        "qos",
        "/sdb/sai/qos",
        &g_sdb_dir_node_info[SDIR_SAI],
    },
};

#endif /* !__SDB_DIR_PRIV_H__ */

