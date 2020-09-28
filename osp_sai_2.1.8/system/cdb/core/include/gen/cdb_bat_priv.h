
#ifndef __CDB_BAT_PRIV_H__
#define __CDB_BAT_PRIV_H__

cdb_node_t g_bat_node_info[BAT_MAX] =
{
    {
        BAT_VLANRANGE,
        CDB_NODE_BAT,
        CDB_PRODUCT_L2PLUS,
        "vlanrange",
        "/cdb/l2/vlanrange",
        &g_dir_node_info[DIR_L2],
        NULL,
    },
    {
        BAT_INTERFACERANGE,
        CDB_NODE_BAT,
        CDB_PRODUCT_ALL,
        "interfacerange",
        "/cdb/sys/interfacerange",
        &g_dir_node_info[DIR_SYS],
        NULL,
    },
    {
        BAT_QOS_DOMAIN_RANGE,
        CDB_NODE_BAT,
        CDB_PRODUCT_L2PLUS,
        "qos_domain_range",
        "/cdb/qos/qos_domain_range",
        &g_dir_node_info[DIR_QOS],
        NULL,
    },
    {
        BAT_DUMP_CDB_RANGE,
        CDB_NODE_BAT,
        CDB_PRODUCT_ALL,
        "dump_cdb_range",
        "/cdb/debug/dump_cdb_range",
        &g_dir_node_info[DIR_DEBUG],
        NULL,
    },
    {
        BAT_PURE_OPENFLOW,
        CDB_NODE_BAT,
        CDB_PRODUCT_ALL,
        "pure_openflow",
        "/cdb/openflow/pure_openflow",
        &g_dir_node_info[DIR_OPENFLOW],
        NULL,
    },
};

#endif /* !__CDB_BAT_PRIV_H__ */

