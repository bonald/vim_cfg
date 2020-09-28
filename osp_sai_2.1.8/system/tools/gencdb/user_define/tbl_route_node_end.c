
/* Unlock node. */
int32
rt_route_delete_node(tbl_route_node_t *node)
{
    tbl_route_table_t *table = NULL;
    int32 rc = PM_E_NONE;
    
    table = rt_route_table_get_table(node->key.p.family, node->key.vrf_id);
    SAL_ASSERT(table);
    SAL_ASSERT(CTCLIB_SLISTCOUNT(node->rib.obj_list) == 0);
    _rt_route_node_delete(table, node);
    
    return rc;
}

tbl_route_node_t*
rt_route_table_top(tbl_route_table_t *table)
{
    if (table == NULL || table->top == NULL)
    {
        return NULL;
    }

    return table->top;
}

tbl_route_node_t*
rt_route_next(tbl_route_node_t *node)
{
    tbl_route_node_t *next = NULL;
    tbl_route_node_t *start = NULL;

    if (node->l_left)
    {
        next = node->l_left;
        return next;
    }

    if (node->l_right)
    {
        next = node->l_right;
        return next;
    }

    start = node;
    while (node->parent)
    {
        if (node->parent->l_left == node && node->parent->l_right)
        {
            next = node->parent->l_right;
            return next;
        }
        node = node->parent;
    }

    return NULL;
}

int32
rt_route_table_add_table(uint8 family, vrf_id_t vrf_id)
{
    tbl_route_node_master_t *p_master = tbl_route_node_get_master();
    tbl_route_table_t *p_table = NULL;

    p_table = rt_route_table_get_table(family, vrf_id);
    if (p_table)
    {
        return PM_E_NONE;
    }

    if (vrf_id > GLB_MAX_VRFID)
    {
        return PM_E_INVALID_PARAM;
    }
    
    if (AF_INET == family)
    {
        p_master->ipv4[vrf_id] = XMALLOC(MEM_PM_RT_TABLE, sizeof(tbl_route_table_t));
        if (NULL == p_master->ipv4[vrf_id])
        {
            return PM_E_NO_MEMORY;
        }
    }
    else if (AF_INET6 == family)
    {
        p_master->ipv6[vrf_id] = XMALLOC(MEM_PM_RT_TABLE, sizeof(tbl_route_table_t));
        if (NULL == p_master->ipv6[vrf_id])
        {
            return PM_E_NO_MEMORY;
        }
    }
    else
    {
        return PM_E_INVALID_PARAM;
    }

    return PM_E_NONE;
}

int32
rt_route_table_del_table(uint8 family, vrf_id_t vrf_id)
{
    tbl_route_node_master_t *p_master = tbl_route_node_get_master();
    tbl_route_table_t *p_table = NULL;

    p_table = rt_route_table_get_table(family, vrf_id);
    if (NULL == p_table)
    {
        return PM_E_NOT_EXIST;
    }

    if (vrf_id > GLB_MAX_VRFID)
    {
        return PM_E_INVALID_PARAM;
    }
    
    if (AF_INET == family)
    {
        XFREE(MEM_PM_RT_TABLE, p_master->ipv4[vrf_id]);
        p_master->ipv4[vrf_id] = NULL;
    }
    else if (AF_INET6 == family)
    {
        XFREE(MEM_PM_RT_TABLE, p_master->ipv4[vrf_id]);
        p_master->ipv6[vrf_id] = NULL;
    }
    else
    {
        return PM_E_INVALID_PARAM;
    }

    return PM_E_NONE;
}

tbl_route_table_t*
rt_route_table_get_table(uint8 family, vrf_id_t vrf_id)
{
    tbl_route_node_master_t *p_master = tbl_route_node_get_master();

    if (vrf_id > GLB_MAX_VRFID)
    {
        return NULL;
    }
    
    if (AF_INET == family)
    {
        return p_master->ipv4[vrf_id];
    }
    else if (AF_INET6 == family)
    {
        return p_master->ipv6[vrf_id];
    }
    else
    {
        return NULL;
    }
}

int32
tbl_route_node_iterate_vrf(uint8 family, vrf_id_t vrf_id, TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_route_table_t *p_table = NULL;
    tbl_route_node_t* node = NULL;
    int32 rc = PM_E_NONE;

    if (vrf_id > GLB_MAX_VRFID)
    {
        return PM_E_INVALID_PARAM;
    }
    
    if (AF_INET != family && AF_INET6 != family)
    {
        return PM_E_INVALID_PARAM;
    }

    p_table = rt_route_table_get_table(family, vrf_id);
    if (NULL == p_table)
    {
        return PM_E_NOT_EXIST;
    }
    
    node = rt_route_table_top(p_table);
    for (; node; node = rt_route_next(node))
    {
        rc |= fn(node, pargs);
    }
    
    return rc;
}

int32
tbl_route_node_iterate_v4(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_route_table_t *p_table = NULL;
    tbl_route_node_t* node = NULL;
    vrf_id_t vrf_id = 0;
    int32 rc = PM_E_NONE;

    for (vrf_id = 0; vrf_id < GLB_MAX_VRFID+1; vrf_id++)
    {
        p_table = rt_route_table_get_table(AF_INET, vrf_id);
        if (NULL == p_table)
        {
            continue;
        }

        node = rt_route_table_top(p_table);
        for (; node; node = rt_route_next(node))
        {
            rc |= fn(node, pargs);
        }        
    }

    return rc;
}

int32
tbl_route_node_iterate_v6(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_route_table_t *p_table = NULL;
    tbl_route_node_t* node = NULL;
    vrf_id_t vrf_id = 0;
    int32 rc = PM_E_NONE;

    for (vrf_id = 0; vrf_id < GLB_MAX_VRFID+1; vrf_id++)
    {
        p_table = rt_route_table_get_table(AF_INET6, vrf_id);
        if (NULL == p_table)
        {
            continue;
        }

        node = rt_route_table_top(p_table);
        for (; node; node = rt_route_next(node))
        {
            rc |= fn(node, pargs);
        }        
    }

    return rc;
}

