#define CTC_IFINDEX_VALID_CHECK(_ifindex, _min, _max, _name)  \
if (_ifindex < _min || _ifindex > _max)\
{\
    return (-1);\
}

int32
tbl_interface_get_ifindex_by_name(const char *ifname)
{
    int32 ifindex = 0;
    int32 rc = PM_E_NONE;

    if (0 == sal_memcmp(ifname, GLB_IFNAME_ETH_PREFIX, GLB_IFNAME_ETH_PREFIX_LEN))
    {
#if 0
        int32 slot_index = 0;
        rc = sal_sscanf(ifname, GLB_IFNAME_ETH_PREFIX"%u-%u", &slot_index, &ifindex);
        ifindex = GLB_PHY_IFINDEX_MIN + (slot_index * GLB_PORT_NUM_PER_SLOT + ifindex);
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_PHY_IFINDEX_MIN, GLB_PHY_IFINDEX_MAX, ifname);
#else
        ifindex = if_get_ifindex_by_name(ifname);
#endif
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_AGG_PREFIX, GLB_IFNAME_AGG_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_AGG_PREFIX"%u", &ifindex);
        ifindex += GLB_AGG_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_AGG_IFINDEX_MIN, GLB_AGG_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_VLAN_PREFIX, GLB_IFNAME_VLAN_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_VLAN_PREFIX"%u", &ifindex);
        ifindex += GLB_VLAN_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_VLAN_IFINDEX_MIN, GLB_VLAN_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_TUNNEL_PREFIX, GLB_IFNAME_TUNNEL_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_TUNNEL_PREFIX"%u", &ifindex);
        ifindex += GLB_TUNNEL_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_TUNNEL_IFINDEX_MIN, GLB_TUNNEL_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_LOOPBACK_PREFIX, GLB_IFNAME_LOOPBACK_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_LOOPBACK_PREFIX"%u", &ifindex);
        ifindex += GLB_LB_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_LB_IFINDEX_MIN, GLB_LB_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_L2GRE_PREFIX, GLB_IFNAME_L2GRE_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_L2GRE_PREFIX"%u", &ifindex);
        ifindex += GLB_L2GRE_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_L2GRE_IFINDEX_MIN, GLB_L2GRE_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_L3GRE_PREFIX, GLB_IFNAME_L3GRE_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_L3GRE_PREFIX"%u", &ifindex);
        ifindex += GLB_L3GRE_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_L3GRE_IFINDEX_MIN, GLB_L3GRE_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_NVGRE_PREFIX, GLB_IFNAME_NVGRE_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_NVGRE_PREFIX"%u", &ifindex);
        ifindex += GLB_NVGRE_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_NVGRE_IFINDEX_MIN, GLB_NVGRE_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_VXLAN_PREFIX, GLB_IFNAME_VXLAN_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_VXLAN_PREFIX"%u", &ifindex);
        ifindex += GLB_VXLAN_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_VXLAN_IFINDEX_MIN, GLB_VXLAN_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_CPU_PREFIX, GLB_IFNAME_CPU_PREFIX_LEN))
    {
        ifindex = GLB_CPU_IFINDEX_MIN;
        return ifindex;
    }
    else
    {
        return PM_E_INVALID_PARAM;
    }
    
    (void)rc;
    return PM_E_NONE;
}

uint32
tbl_interface_get_type_by_ifindex(uint32 ifindex)
{
    if (GLB_PHY_IFINDEX_MIN < ifindex && ifindex < GLB_PHY_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_PORT_IF;
    }
    else if (GLB_AGG_IFINDEX_MIN < ifindex && ifindex < GLB_AGG_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_LINKAGG_IF;
    }
    else if (GLB_VLAN_IFINDEX_MIN < ifindex && ifindex < GLB_VLAN_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_VLAN_IF;
    }
    else if (GLB_TUNNEL_IFINDEX_MIN < ifindex && ifindex < GLB_TUNNEL_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_TUNNEL_IF;
    }
    else if (GLB_LB_IFINDEX_MIN <= ifindex && ifindex < GLB_LB_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_LOOPBACK_IF;
    }
    else if (GLB_L2GRE_IFINDEX_MIN < ifindex && ifindex < GLB_L2GRE_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_L2GRE_IF;
    }
    else if (GLB_L3GRE_IFINDEX_MIN < ifindex && ifindex < GLB_L3GRE_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_L3GRE_IF;
    }
    else if (GLB_NVGRE_IFINDEX_MIN < ifindex && ifindex < GLB_NVGRE_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_NVGRE_IF;
    }
    else if (GLB_VXLAN_IFINDEX_MIN < ifindex && ifindex < GLB_VXLAN_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_VXLAN_IF;
    }
    else if (GLB_CPU_IFINDEX_MIN == ifindex)
    {
        return GLB_IF_TYPE_CPU_IF;
    }
    else
    {
        return GLB_IF_TYPE_NULL;
    }
    
    return GLB_IF_TYPE_NULL;
}

vid_t
tbl_interface_get_vid_by_ifindex(uint32 ifindex)
{
    if (GLB_VLAN_IFINDEX_MIN < ifindex && ifindex < GLB_VLAN_IFINDEX_MAX)
    {
        return (ifindex - GLB_VLAN_IFINDEX_MIN);
    }
    else
    {
        return GLB_ALL_VLAN_ID;
    }
}

int32
tbl_interface_get_name_by_ifindex(uint32 ifindex, char *name, uint32 len)
{
    int32 index = 0;

    sal_memset(name, 0, len);
    if (GLB_PHY_IFINDEX_MIN < ifindex && ifindex < GLB_PHY_IFINDEX_MAX)
    {
#if 0
        int32 slot_index = 0;
        slot_index = ifindex / GLB_PORT_NUM_PER_SLOT;
        index = ifindex % GLB_PORT_NUM_PER_SLOT;
        sal_snprintf(name, len, GLB_IFNAME_ETH_PREFIX"%u-%u", slot_index, index);
#else
        if_get_name_by_ifindex(ifindex, name, len);
#endif
    }
    else if (GLB_AGG_IFINDEX_MIN < ifindex && ifindex < GLB_AGG_IFINDEX_MAX)
    {
        index = ifindex - GLB_AGG_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_AGG_PREFIX"%u", index);
    }
    else if (GLB_VLAN_IFINDEX_MIN < ifindex && ifindex < GLB_VLAN_IFINDEX_MAX)
    {
        index = ifindex - GLB_VLAN_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_VLAN_PREFIX"%u", index);
    }
    else if (GLB_TUNNEL_IFINDEX_MIN < ifindex && ifindex < GLB_TUNNEL_IFINDEX_MAX)
    {
        index = ifindex - GLB_TUNNEL_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_TUNNEL_PREFIX"%u", index);
    }
    else if (GLB_LB_IFINDEX_MIN < ifindex && ifindex < GLB_LB_IFINDEX_MAX)
    {
        index = ifindex - GLB_LB_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_LOOPBACK_PREFIX"%u", index);
    }
    else if (GLB_L2GRE_IFINDEX_MIN < ifindex && ifindex < GLB_L2GRE_IFINDEX_MAX)
    {
        index = ifindex - GLB_L2GRE_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_L2GRE_PREFIX"%u", index);
    }
    else if (GLB_L3GRE_IFINDEX_MIN < ifindex && ifindex < GLB_L3GRE_IFINDEX_MAX)
    {
        index = ifindex - GLB_L3GRE_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_L3GRE_PREFIX"%u", index);
    }
    else if (GLB_NVGRE_IFINDEX_MIN < ifindex && ifindex < GLB_NVGRE_IFINDEX_MAX)
    {
        index = ifindex - GLB_NVGRE_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_NVGRE_PREFIX"%u", index);
    }
    else if (GLB_VXLAN_IFINDEX_MIN < ifindex && ifindex < GLB_VXLAN_IFINDEX_MAX)
    {
        index = ifindex - GLB_VXLAN_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_VXLAN_PREFIX"%u", index);
    }
    else if (GLB_CPU_IFINDEX_MIN == ifindex)
    {
        sal_snprintf(name, len, GLB_IFNAME_CPU_PREFIX);
    }
    else
    {
        return PM_E_INVALID_PARAM;
    }

    return PM_E_NONE;
}

tbl_interface_t*
tbl_interface_get_interface_by_name(const char* ifname)
{
    tbl_interface_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.name, ifname, IFNAME_SIZE);
    return tbl_interface_get_interface(&key);
}

tbl_interface_t*
tbl_interface_get_interface_by_ifindex(uint32 ifindex)
{
    tbl_interface_key_t key;
    int32 rc = PM_E_NONE;

    sal_memset(&key, 0, sizeof(key));
    rc = tbl_interface_get_name_by_ifindex(ifindex, key.name, IFNAME_SIZE);
    if (rc < 0)
    {
        return NULL;
    }

    return tbl_interface_get_interface(&key);
}

uint32
tbl_interface_is_up(tbl_interface_t *p_if)
{
    return (p_if->flags & GLB_IFF_UP);
}

uint32
tbl_interface_is_running(tbl_interface_t *p_if)
{
    if (GLB_IF_TYPE_LOOPBACK_IF == p_if->hw_type)
    {
        return tbl_interface_is_up(p_if);
    }

    return (p_if->flags & (GLB_IFF_UP|GLB_IFF_RUNNING)) == (GLB_IFF_UP|GLB_IFF_RUNNING);
}

tbl_interface_t*
tbl_interface_get_interface_by_port_id(uint64 port_id)
{
    tbl_interface_master_t *p_master = _g_p_tbl_interface_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_interface_t *p_db_if = NULL;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, listnode, next)
    {
        if(p_db_if->portid == port_id)
        {
            return p_db_if;
        }
    }

    return NULL;
}

int32
tbl_interface_get_lag_1st_member_ifindex(uint32 lag_ifindex, uint32 *mem_ifindex)
{
    tbl_interface_t *p_db_lag = NULL;
    tbl_interface_t *p_db_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    p_db_lag = tbl_interface_get_interface_by_ifindex(lag_ifindex);
    if ((NULL == p_db_lag) || (NULL == p_db_lag->lag))
    {
        return PM_E_FAIL; 
    }
    if (GLB_IF_TYPE_LINKAGG_IF != p_db_lag->hw_type)
    {
        return PM_E_FAIL; 
    }
    
    CTCLIB_SLIST_LOOP(p_db_lag->lag->member_ports.obj_list, p_db_mem, listnode)
    {
        if (NULL == p_db_mem->lag)
        {
            continue;
        }
        if (p_db_mem->lag->oper_state)
        {
            *mem_ifindex = p_db_mem->ifindex;
            return PM_E_NONE;
        }
    }
    
    return PM_E_FAIL;
}

