/* Utility mask array. */
static const uint8 maskbit[] =
{
    0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff
};

/* Common prefix route genaration. */
static int32
_route_common(prefix_t *n, prefix_t *p, prefix_t *new)
{
    int32 i = 0;
    uint8 diff = 0;
    uint8 mask = 0;
    uint8 *np = (uint8*)&n->u.prefix;
    uint8 *pp = (uint8*)&p->u.prefix;
    uint8 *newp = (uint8*)&new->u.prefix;

    for (i = 0; i < p->prefixlen / 8; i++)
    {
        if (np[i] == pp[i])
            newp[i] = np[i];
        else
            break;
    }

    new->prefixlen = i * 8;

    if (new->prefixlen != p->prefixlen)
    {
        diff = np[i] ^ pp[i];
        mask = 0x80;
        while (new->prefixlen < p->prefixlen && !(mask & diff))
        {
            mask >>= 1;
            new->prefixlen++;
        }
        newp[i] = np[i] & maskbit[new->prefixlen % 8];
    }

    return PM_E_NONE;
}

static void
_set_link(tbl_route_node_t *node, tbl_route_node_t *new_node)
{
    uint32 bit = prefix_bit(&new_node->key.p.u.prefix, node->key.p.prefixlen);

    node->link[bit] = new_node;
    new_node->parent = node;
}

/* Check bit of the prefix. */
static int32
_check_bit(uint8 *prefix, int8 prefixlen)
{
    int32 offset = 0;
    int32 shift = 0;
    uint8 *p = (uint8*)prefix;

    /*  assert (prefixlen <= 128); */
    offset = prefixlen / 8;
    shift = 7 - (prefixlen % 8);
    
    return (p[offset] >> shift & 1);
}

static inline tbl_route_node_t*
_rt_route_node_create()
{
    tbl_route_node_t *node = NULL;
    
    node = XCALLOC(MEM_TBL_ROUTE_NODE, sizeof(tbl_route_node_t));
    if (NULL == node)
    {
        return NULL;
    }
    cdb_reference_list_init(&node->rib, CDB_NODE_DS, DS_RIB, ds_rib_cmp);

    return node;
}

static inline int32
_rt_route_node_destory(tbl_route_node_t *node)
{
    int32 rc = PM_E_NONE;
    PM_E_RETURN(cdb_reference_list_deinit(&node->rib));
    XFREE(MEM_TBL_ROUTE_NODE, node);

    return PM_E_NONE;
}

static tbl_route_node_t*
_rt_route_node_set(tbl_route_table_t *table, prefix_t *p)
{
    tbl_route_node_t *node = NULL;
    
    node = _rt_route_node_create();
    if (NULL == node)
    {
        return NULL;
    }
    prefix_copy(&node->key.p, p);
    
    return node;
}

static int32
_rt_route_node_delete(tbl_route_table_t *table, tbl_route_node_t *node)
{
    tbl_route_node_t *child = NULL;
    tbl_route_node_t *parent = NULL;

    SAL_ASSERT(node->lock == 0);
    SAL_ASSERT(CTCLIB_SLISTCOUNT(node->rib.obj_list) == 0);

    if (node->l_left && node->l_right)
    {
        return PM_E_NONE;
    }

    if (node->l_left)
    {
        child = node->l_left;
    }
    else
    {
        child = node->l_right;
    }

    parent = node->parent;

    if (child)
    {
        child->parent = parent;
    }

    if (parent)
    {
        if (parent->l_left == node)
        {
            parent->l_left = child;
        }
        else
        {
            parent->l_right = child;
        }
    }
    else
    {
        table->top = child;
    }

    table->count--;

    _rt_route_node_destory(node);

    /* If parent node is stub then delete it also. */
    if (parent && parent->lock == 0)
    {
        _rt_route_node_delete(table, parent);
    }

    return PM_E_NONE;
}

tbl_route_node_t*
rt_route_node_get(tbl_route_node_key_t *p_rn_key)
{
    tbl_route_table_t *table = NULL;
    tbl_route_node_t *new_node = NULL;
    tbl_route_node_t *node = NULL;
    tbl_route_node_t *match = NULL;
    uint8 prefixlen = p_rn_key->p.prefixlen;
    const uint8 *prefix = &p_rn_key->p.u.prefix;

    table = rt_route_table_get_table(p_rn_key->p.family, p_rn_key->vrf_id);
    if (NULL == table)
    {
        return NULL;
    }

    match = NULL;
    node = table->top;

    while (node && node->key.p.prefixlen <= prefixlen && prefix_match(&node->key.p, &p_rn_key->p))
    {
        if (node->key.p.prefixlen == prefixlen)
        {
            return rt_route_lock_node(node);
        }

        match = node;
        node = node->link[prefix_bit(prefix, node->key.p.prefixlen)];
    }

    if (NULL == node)
    {
        new_node = _rt_route_node_set(table, &p_rn_key->p);
        if (match)
        {
            _set_link(match, new_node);
        }
        else
        {
            table->top = new_node;
        }
    }
    else
    {
        new_node = _rt_route_node_create();
        if (NULL == new_node)
        {
            return NULL;
        }
        _route_common(&node->key.p, &p_rn_key->p, &new_node->key.p);
        new_node->key.p.family = p_rn_key->p.family;
        new_node->key.vrf_id = p_rn_key->vrf_id;
        _set_link(new_node, node);

        if (match)
        {
            _set_link(match, new_node);
        }
        else
        {
            table->top = new_node;
        }
    
        if (new_node->key.p.prefixlen != p_rn_key->p.prefixlen)
        {
            match = new_node;
            new_node = _rt_route_node_set(table, &p_rn_key->p);
            _set_link(match, new_node);
            table->count++;
        }
    }
    
    table->count++;
    rt_route_lock_node(new_node);
  
    return new_node;
}
