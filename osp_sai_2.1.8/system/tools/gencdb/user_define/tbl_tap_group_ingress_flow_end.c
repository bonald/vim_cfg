int32
tbl_tap_group_ingress_flow_get_if_exist(const char* tap_grp, const char* if_name)
{
    tbl_tap_group_ingress_flow_master_t *p_master = _g_p_tbl_tap_group_ingress_flow_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_tap_group_ingress_flow_t *p_db_tap_group_ingress_flow = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->tap_group_ingress_list, p_db_tap_group_ingress_flow, listnode, next)
    {
        if (0 == sal_memcmp(p_db_tap_group_ingress_flow->key.if_name, if_name, (sizeof(char))*IFNAME_SIZE))
        {
            return PM_E_EXIST;
        }
    }

    return rc;
}

int32
tbl_tap_group_ingress_flow_get_group_if_exist(const char* tap_grp, const char* if_name)
{
    tbl_tap_group_ingress_flow_master_t *p_master = _g_p_tbl_tap_group_ingress_flow_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_tap_group_ingress_flow_t *p_db_tap_group_ingress_flow = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->tap_group_ingress_list, p_db_tap_group_ingress_flow, listnode, next)
    {
        if (0 == sal_memcmp(p_db_tap_group_ingress_flow->key.if_name, if_name, (sizeof(char))*IFNAME_SIZE)
            && 0 == sal_memcmp(p_db_tap_group_ingress_flow->key.tap_group, tap_grp, (sizeof(char))*TAP_NAME_SIZE))
        {
            return PM_E_EXIST;
        }
    }

    return rc;
}

int32
tbl_tap_group_ingress_flow_get_if_flow_exist(const char* if_name, const char* flow_name)
{
    tbl_tap_group_ingress_flow_master_t *p_master = _g_p_tbl_tap_group_ingress_flow_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_tap_group_ingress_flow_t *p_db_tap_group_ingress_flow = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->tap_group_ingress_list, p_db_tap_group_ingress_flow, listnode, next)
    {
        if (0 == sal_memcmp(p_db_tap_group_ingress_flow->key.if_name, if_name, (sizeof(char))*IFNAME_SIZE)
            && 0 == sal_strcmp(p_db_tap_group_ingress_flow->key.flow_name, flow_name))
        {
            return PM_E_EXIST;
        }
    }

    return rc;
}




