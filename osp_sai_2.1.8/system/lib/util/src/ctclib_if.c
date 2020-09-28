#include "sal.h"
#include "genlog.h"
#include "glb_tempfile_define.h"
#include "glb_const.h"
#include "glb_phy_define.h"
#include "glb_if_define.h"
#include "ctclib_if.h"
#include "ctclib_memory.h"

cli_phy_port_master_t *g_phy_port_master = NULL;

/*Modify by weij for support phy port name self-define. bug 31712*/
static int32
_if_phy_port_master_init()
{
    FILE *fp = NULL;
    int num = 0;
    int port_type = 0;
    int slot_id = 0;
    int port_id = 0;
    int sub_id = 0;
    int split_type = 0;
    int split_port_type = 0;
    char port_connection = 0;
    char sub_port_connection = 0;
    char buf[256];
    char prefix[GLB_PHY_PORT_PREFIX_SZ];
    char full_prefix[GLB_PHY_PORT_FULL_PREFIX_SZ];
    cli_phy_port_master_t *p_master = NULL;

    if (NULL != g_phy_port_master)
    {
        return 0;
    }
    
    p_master = XCALLOC(MEM_TEMP, sizeof(cli_phy_port_master_t));
    if (NULL == p_master)
    {
        return 0;
    }
    
    fp = sal_fopen(GLB_PORT_INFO_FILE, SAL_OPEN_RO);
    if (fp)
    {
        while (sal_fgets(buf, 128, fp))
        {
            if (4 == sal_sscanf(buf, "port %d %d %d %d", &port_type, &slot_id, &port_id, &sub_id))
            {
                p_master->valid_num++;
                p_master->port[num].port_type = port_type;
                p_master->port[num].slot_id = slot_id;
                p_master->port[num].port_id = port_id;
                p_master->port[num].sub_id = sub_id;
                p_master->port[num].ifindex = p_master->valid_num;
                /*the type is real existed*/
                p_master->port_prefix[port_type-1].valid = TRUE;
                num++;
            }
        }
        sal_fclose(fp);
    }

    port_type = 0;
    p_master->port_connection = IF_DEF_PORT_CONN_CHAR;
    p_master->sub_port_connection = IF_DEF_SUBPORT_CONN_CHAR;
    fp = sal_fopen(GLB_PORT_CLI_INFO_FILE, SAL_OPEN_RO);
    if (fp)
    {
        while (sal_fgets(buf, 128, fp))
        {
            /*update*/
            if (2 == sal_sscanf(buf, "port_connection %c sub_port_connection %c", &port_connection, &sub_port_connection))
            {
                p_master->port_connection = port_connection;
                p_master->sub_port_connection = sub_port_connection;
            }
            if (2 == sal_sscanf(buf, "split_type %d %d", &split_type, &split_port_type))
            {
                p_master->split_type[split_type] = split_port_type;
            }
            else if (2 == sal_sscanf(buf, "prefix %s full_prefix %s", prefix, full_prefix))
            {
                sal_snprintf(p_master->port_prefix[port_type].prefix, GLB_PHY_PORT_PREFIX_SZ, prefix);
                sal_snprintf(p_master->port_prefix[port_type].full_prefix, GLB_PHY_PORT_FULL_PREFIX_SZ, full_prefix);
                sal_snprintf(p_master->port_prefix[port_type].desc, CLI_ARGV_MAX_LEN, "%s interface", prefix);
                port_type++;
            }
        }
        sal_fclose(fp);
    }
    
    /*default prefix*/
    if (0 == port_type)
    {
        /*default*/
        sal_snprintf(p_master->port_prefix[0].prefix, GLB_PHY_PORT_PREFIX_SZ, CLI_DEF_PHY_PORT_PREFIX);
        sal_snprintf(p_master->port_prefix[0].full_prefix, GLB_PHY_PORT_FULL_PREFIX_SZ, CLI_DEF_PHY_PORT_FULL_PREFIX);
        sal_snprintf(p_master->port_prefix[0].desc, CLI_ARGV_MAX_LEN, "Physical interface");
        p_master->port_prefix[0].valid = TRUE;
        p_master->is_default = TRUE;
    }

    for (port_type = 0; port_type < CLI_PHT_PORT_TYPE_MAX; port_type++)
    {
        if (p_master->port_prefix[port_type].valid && !sal_strlen(p_master->port_prefix[port_type].prefix))
        {
            log_sys(M_MOD_LIB, E_ERROR, "%s fail: port type %d existed without prefix", __FUNCTION__, port_type + 1);
        }
    }

    g_phy_port_master = p_master;

    return 0;
}

cli_phy_port_master_t*
if_get_phy_port_master()
{
    _if_phy_port_master_init();
    return g_phy_port_master;
}

int32
if_get_port_num_by_slot_port(uint8 slot_id, uint8 port_id, uint8 sub_port_id, uint8 *p_port_num)
{    
    cli_phy_port_master_t *p_master = if_get_phy_port_master();
    uint16 i = 0;

    if (NULL == p_master)
    {
        return -1;
    }
    
    for (i = 0; i < p_master->valid_num; i++)
    {
        if ((p_master->port[i].slot_id == slot_id)
            &&(p_master->port[i].port_id == port_id) 
            && (p_master->port[i].sub_id == sub_port_id))
        {
            *p_port_num = i;
            return 0;
        }
    }

    return -1;
}

int32
if_build_port_shortname_by_slot_port(char *ifname, uint16 slot_id, uint16 port_id, uint16 sub_port_id)
{
#ifndef GREATBELT
    cli_phy_port_master_t *p_master = if_get_phy_port_master();
    uint8 port_num = 0;
    uint8 port_type = 0;
    
    if (NULL == p_master)
    {
        return -1;
    }
    
    if (if_get_port_num_by_slot_port(slot_id, port_id, sub_port_id, &port_num))
    {
        return -1;
    }
    port_type = p_master->port[port_num].port_type - 1;
    if (sub_port_id)
    {
        sal_snprintf(ifname, GLB_PHY_PORT_FULL_NAME_SZ, "%s%d%c%d%c%d", p_master->port_prefix[port_type].prefix, 
                                        slot_id, p_master->port_connection,
                                        port_id, p_master->sub_port_connection, sub_port_id);
    }
    else
    {
        sal_snprintf(ifname, GLB_PHY_PORT_FULL_NAME_SZ, "%s%d%c%d", p_master->port_prefix[port_type].prefix, 
                                        slot_id, p_master->port_connection, port_id);
    }
    return 0;
#else
    return if_build_ethname_by_slot_port(ifname, slot_id, port_id, sub_port_id);
#endif
}

int32
if_build_port_fullname_by_slot_port(char *ifname, uint16 slot_id, uint16 port_id, uint16 sub_port_id)
{
#ifndef GREATBELT
    cli_phy_port_master_t *p_master = if_get_phy_port_master();
    uint8 port_num = 0;
    uint8 port_type = 0;
    
    if (NULL == p_master)
    {
        return -1;
    }
    
    if (if_get_port_num_by_slot_port(slot_id, port_id, sub_port_id, &port_num))
    {
        return -1;
    }
    port_type = p_master->port[port_num].port_type - 1;
    if (sub_port_id)
    {
        sal_snprintf(ifname, GLB_PHY_PORT_FULL_NAME_SZ, "%s%d%c%d%c%d", p_master->port_prefix[port_type].full_prefix, 
                                        slot_id, p_master->port_connection,
                                        port_id, p_master->sub_port_connection, sub_port_id);
    }
    else
    {
        sal_snprintf(ifname, GLB_PHY_PORT_FULL_NAME_SZ, "%s%d%c%d", p_master->port_prefix[port_type].full_prefix, 
                                        slot_id, p_master->port_connection, port_id);
    }
    return 0;
#else
    return if_build_ethname_by_slot_port(ifname, slot_id, port_id, sub_port_id);
#endif
}

int32
if_check_phy_port_by_shortname(char *ifname)
{
    cli_phy_port_master_t *p_master = if_get_phy_port_master();
    uint16 i = 0;

    if (NULL == p_master)
    {
        return -1;
    }
    
    for (i = 0; i < CLI_PHT_PORT_TYPE_MAX; i++)
    {
        if (!p_master->port_prefix[i].valid)
        {
            continue;
        }
        if (sal_strncmp(ifname, p_master->port_prefix[i].prefix, sal_strlen(p_master->port_prefix[i].prefix)))
        {
            continue;
        }
        return 1;
    }
    return 0;
}

int32
if_check_phy_port_by_fullname(char *ifname)
{
    cli_phy_port_master_t *p_master = if_get_phy_port_master();
    uint16 i = 0;

    if (NULL == p_master)
    {
        return -1;
    }
    
    for (i = 0; i < CLI_PHT_PORT_TYPE_MAX; i++)
    {
        if (!p_master->port_prefix[i].valid)
        {
            continue;
        }
        if (sal_strncmp(ifname, p_master->port_prefix[i].full_prefix, sal_strlen(p_master->port_prefix[i].full_prefix)))
        {
            continue;
        }
        return 1;
    }
    return 0;
}

int32
if_check_phy_port_by_name(char *ifname)
{
    if (if_check_phy_port_by_shortname(ifname))
    {
        return 1;
    }
    if (if_check_phy_port_by_fullname(ifname))
    {
        return 1;
    }
    return 0;
}

int32
if_get_slot_port_by_fullname(char *ifname, uint8 *slot_id, uint8 *port_id, uint8 *sub_port_id)
{
#ifndef GREATBELT
    cli_phy_port_master_t *p_master = if_get_phy_port_master();
    char *p = NULL;
    uint8 i = 0;

    if (NULL == p_master)
    {
        return -1;
    }
    
    for (i = 0; i < CLI_PHT_PORT_TYPE_MAX; i++)
    {
        if (!p_master->port_prefix[i].valid)
        {
            continue;
        }
        if (sal_strncmp(ifname, p_master->port_prefix[i].full_prefix, sal_strlen(p_master->port_prefix[i].full_prefix)))
        {
            continue;
        }
        p = ifname + sal_strlen(p_master->port_prefix[i].full_prefix);
        if (!*p && !sal_isdigit(*p))
        {
            log_sys(M_MOD_LIB, E_ERROR, "Interface %s parse slot fail", ifname);
            return -1;
        }
        *slot_id = sal_atoi(p);
        while (*p && sal_isdigit(*p))
        {
            p++;
        }
        if (!*p)
        {
            log_sys(M_MOD_LIB, E_ERROR, "Interface %s parse port fail", ifname);
            return -1;
        }
        //goto panel_port_no
        p++;
        *port_id = sal_atoi(p);

        //deal sub_port
        while (*p && sal_isdigit(*p))
        {
            p++;
        }
        if (!*p || ('\n' == *p))
        {
            *sub_port_id = 0;
            return 0;
        }
        //goto sub_panel_port_no
        p++;
        *sub_port_id = sal_atoi(p);
        return 0;
    }
    return -1;
#else
    return if_get_slot_port_by_ethname(ifname, slot_id, port_id, sub_port_id);
#endif
}

int32
if_get_slot_port_by_shortname(char *ifname, uint8 *slot_id, uint8 *port_id, uint8 *sub_port_id)
{
#ifndef GREATBELT
    cli_phy_port_master_t *p_master = if_get_phy_port_master();
    char *p = NULL;
    uint16 i = 0;

    if (NULL == p_master)
    {
        return -1;
    }
    
    for (i = 0; i < CLI_PHT_PORT_TYPE_MAX; i++)
    {
        if (!p_master->port_prefix[i].valid)
        {
            continue;
        }
        if (sal_strncmp(ifname, p_master->port_prefix[i].prefix, sal_strlen(p_master->port_prefix[i].prefix)))
        {
            continue;
        }
        p = ifname + sal_strlen(p_master->port_prefix[i].prefix);
        if (!*p && !sal_isdigit(*p))
        {
            log_sys(M_MOD_LIB, E_ERROR, "Interface %s parse slot fail", ifname);
            return -1;
        }
        *slot_id = sal_atoi(p);
        while (*p && sal_isdigit(*p))
        {
            p++;
        }
        if (!*p)
        {
            log_sys(M_MOD_LIB, E_ERROR, "Interface %s parse port fail", ifname);
            return -1;
        }
        //goto panel_port_no
        p++;
        *port_id = sal_atoi(p);

        //deal sub_port
        while (*p && sal_isdigit(*p))
        {
            p++;
        }
        if (!*p || ('\n' == *p))
        {
            *sub_port_id = 0;
            return 0;
        }
        //goto panel_sub_port_no
        p++;
        *sub_port_id = sal_atoi(p);
        return 0;
    }
    return 0;
#else
    return if_get_slot_port_by_ethname(ifname, slot_id, port_id, sub_port_id);
#endif
}

int32
if_get_slot_port_by_name(char *ifname, uint8 *slot_id, uint8 *port_id, uint8 *sub_port_id)
{
    if (!if_get_slot_port_by_fullname(ifname, slot_id, port_id, sub_port_id))
    {
        return 0;
    }
    if (!if_get_slot_port_by_shortname(ifname, slot_id, port_id, sub_port_id))
    {
        return 0;
    }
    return -1;
}

char*
if_get_port_fullname_name(char *ifname)
{
    uint8 slot_id = 0;
    uint8 port_id = 0;
    uint8 sub_port_id = 0;
    static char full_name[GLB_PHY_PORT_FULL_NAME_SZ] = {};
    if (!if_check_phy_port_by_name(ifname))
    {
        return ifname;
    }
    if_get_slot_port_by_name (ifname, &slot_id, &port_id, &sub_port_id);
    if_build_port_fullname_by_slot_port(full_name, slot_id, port_id, sub_port_id);
    return full_name;
}

char*
if_get_port_shortname_name(char *ifname)
{
    uint8 slot_id = 0;
    uint8 port_id = 0;
    uint8 sub_port_id = 0;
    static char short_name[GLB_PHY_PORT_FULL_NAME_SZ] = {};
    if (!if_check_phy_port_by_name(ifname))
    {
        return ifname;
    }
    if_get_slot_port_by_name (ifname, &slot_id, &port_id, &sub_port_id);
    if_build_port_shortname_by_slot_port(short_name, slot_id, port_id, sub_port_id);
    return short_name;
}

int32
if_build_ethname_by_slot_port(char *ifname, uint8 slot_no, uint8 port_no, uint8 sub_port_no)
{
#ifndef GREATBELT
    uint8 port_num = 0;

    _if_phy_port_master_init();
    
    if (if_get_port_num_by_slot_port(slot_no, port_no, sub_port_no, &port_num))
    {
        return -1;
    }
#endif
    if (sub_port_no)
    {
        sal_snprintf(ifname, GLB_PHY_PORT_FULL_NAME_SZ, GLB_IFNAME_ETH_PREFIX"%u-%u_%u",
                                        slot_no, port_no, sub_port_no);
    }
    else
    {
        sal_snprintf(ifname, GLB_PHY_PORT_FULL_NAME_SZ, GLB_IFNAME_ETH_PREFIX"%u-%u",
                                        slot_no, port_no);
    }
    return 0;
}

int32
if_get_slot_port_by_ethname(char *ifname, uint8 *slot_no, uint8 *port_no, uint8 *sub_port_no)
{
    char *p = NULL;

    _if_phy_port_master_init();

    if (sal_strncmp(ifname, GLB_IFNAME_ETH_PREFIX, sal_strlen(GLB_IFNAME_ETH_PREFIX)))
    {
        return -1;
    }
    p = ifname + sal_strlen(GLB_IFNAME_ETH_PREFIX);
    if (!*p && !sal_isdigit(*p))
    {
        log_sys(M_MOD_LIB, E_ERROR, "Interface %s parse slot fail", ifname);
        return -1;
    }
    *slot_no = sal_atoi(p);
    while (*p && sal_isdigit(*p))
    {
        p++;
    }
    if (!*p)
    {
        log_sys(M_MOD_LIB, E_ERROR, "Interface %s parse port fail", ifname);
        return -1;
    }
    //goto panel_port_no
    p++;
    *port_no = sal_atoi(p);

    //deal sub_port
    while (*p && sal_isdigit(*p))
    {
        p++;
    }
    if (!*p || ('\n' == *p))
    {
        *sub_port_no = 0;
        return 0;
    }
    //goto sub_panel_port_no
    p++;
    *sub_port_no = sal_atoi(p);

    return 0;
}

int32
if_get_ifindex_by_name(const char *ifname)
{
#ifndef GREATBELT
    cli_phy_port_master_t *p_master = if_get_phy_port_master();
    cli_phy_port_t *p_port = NULL;
    uint32 slot_id = 0;
    uint32 port_id = 0;
    uint32 sub_id = 0;
    int32 port_idx = 0;
    int32 ifindex = 0;
    int32 rc = 0;

    if (NULL == p_master)
    {
        return -1;
    }
    
    if (sal_strchr(ifname, '_'))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_ETH_PREFIX"%u-%u_%u", &slot_id, &port_id, &sub_id);
        if (rc != 3)
        {
            return -1;
        }
    }
    else
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_ETH_PREFIX"%u-%u", &slot_id, &port_id);
        if (rc != 2)
        {
            return -1;
        }
    }

    for (port_idx = (port_id-1); port_idx < CLI_PHY_PORT_NUM_MAX; port_idx++)
    {
        p_port = &(p_master->port[port_idx]);
        if (p_port->slot_id == slot_id
         && p_port->port_id == port_id
         && p_port->sub_id == sub_id)
        {
            ifindex = GLB_PHY_IFINDEX_MIN + (slot_id * GLB_PORT_NUM_PER_SLOT + (port_idx+1));
            return ifindex;
        }
    }
    
    return -1;
#else
    int32 ifindex = 0;
    int32 slot_id = 0;
    int32 rc = 0;
    
    rc = sal_sscanf(ifname, GLB_IFNAME_ETH_PREFIX"%u-%u", &slot_id, &ifindex);
    if (rc != 2)
    {
        return -1;
    }    
    ifindex = GLB_PHY_IFINDEX_MIN + (slot_id * GLB_PORT_NUM_PER_SLOT + ifindex);
    return ifindex;
#endif
}

int32
if_get_name_by_ifindex(uint32 ifindex, char *name, uint32 len)
{
#ifndef GREATBELT
    cli_phy_port_master_t *p_master = if_get_phy_port_master();
    cli_phy_port_t *p_port = NULL;
    int32 port_idx = 0;

    if (NULL == p_master)
    {
        return -1;
    }

    port_idx = ifindex - 1;
    if (port_idx < 0 || port_idx >= CLI_PHY_PORT_NUM_MAX)
    {
        return -1;
    }

    p_port = &(p_master->port[port_idx]);

    if (p_port->sub_id)
    {
        sal_snprintf(name, len, GLB_IFNAME_ETH_PREFIX"%u-%u_%u", p_port->slot_id, p_port->port_id, p_port->sub_id);
    }
    else
    {
        sal_snprintf(name, len, GLB_IFNAME_ETH_PREFIX"%u-%u", p_port->slot_id, p_port->port_id);
    }
#else
    int32 slot_id = 0;
    int32 index = 0;
    slot_id = ifindex / GLB_PORT_NUM_PER_SLOT;
    index = ifindex % GLB_PORT_NUM_PER_SLOT;
    sal_snprintf(name, len, GLB_IFNAME_ETH_PREFIX"%u-%u", slot_id, index);
#endif
    return 0;
}

static int32
_if_get_info_by_ethname(char *ethname, cli_phy_port_t **pp_port, cli_phy_port_prefix_t **pp_prefix)
{
    cli_phy_port_master_t *p_master = g_phy_port_master;
    uint32 slot = 0;
    uint32 port = 0;
    uint32 sub_port = 0;
    uint32 port_type = 0;
    uint8 port_num = 0;
    char *p = NULL;
    int32 rc = 0;

    *pp_port = NULL;
    *pp_prefix = NULL;

    if (NULL == p_master)
    {
        return -1;
    }
    
    p = sal_strchr(ethname, '_');
    if (p)
    {
        rc = sal_sscanf(ethname, GLB_IFNAME_ETH_PREFIX"%u-%u_%u", &slot, &port, &sub_port);
        if (3 != rc)
        {
            return -1;
        }
    }
    else
    {
        rc = sal_sscanf(ethname, GLB_IFNAME_ETH_PREFIX"%u-%u", &slot, &port);
        if (2 != rc)
        {
            return -1;
        }
    }

    if (if_get_port_num_by_slot_port(slot, port, sub_port, &port_num))
    {
        return -1;
    }

    port_type = p_master->port[port_num].port_type - 1;
    *pp_port = &(p_master->port[port_num]);
    *pp_prefix = &(p_master->port_prefix[port_type]);

    return 0;
}

char*
if_ethname_to_shortname(char *ethname, char *shortname, uint32 buf_len)
{
    cli_phy_port_master_t *p_master = if_get_phy_port_master();
    cli_phy_port_t *p_port = NULL;
    cli_phy_port_prefix_t *p_prefix = NULL;
    char *p = NULL;
    shortname[0] = '\0';
    int32 rc = 0;

    if (NULL == p_master)
    {
        sal_strncpy(shortname, ethname, buf_len);
        return shortname;
    }
    
    if (0 == sal_strncmp(GLB_IFNAME_ETH_PREFIX, ethname, GLB_IFNAME_ETH_PREFIX_LEN))
    {
        if (!p_master || p_master->is_default)
        {
            sal_strncpy(shortname, ethname, buf_len);
            p = sal_strchr(shortname, '_');
            if (p)
            {
                p[0] = '/';
            }
        }
        else
        {
            rc = _if_get_info_by_ethname(ethname, &p_port, &p_prefix);
            if (rc < 0)
            {
                sal_strncpy(shortname, ethname, buf_len);
                p = sal_strchr(shortname, '_');
                if (p)
                {
                    p[0] = '/';
                }
            }
            if (p_port->sub_id)
            {
                sal_snprintf(shortname, buf_len, "%s%u%c%u%c%u", p_prefix->prefix, 
                                                p_port->slot_id, p_master->port_connection,
                                                p_port->port_id, p_master->sub_port_connection, p_port->sub_id);
            }
            else
            {
                sal_snprintf(shortname, buf_len, "%s%u%c%u", p_prefix->prefix, 
                                                p_port->slot_id, p_master->port_connection,
                                                p_port->port_id);
            }
        }
    }
    else
    {
        sal_strncpy(shortname, ethname, buf_len);
    }
    
    return shortname;
}

char*
if_ethname_to_fullname(char *ethname, char *fullname, uint32 buf_len)
{
    cli_phy_port_master_t *p_master = if_get_phy_port_master();
    cli_phy_port_t *p_port = NULL;
    cli_phy_port_prefix_t *p_prefix = NULL;
    char *p = NULL;
    fullname[0] = '\0';
    int32 rc = 0;

    if (NULL == p_master)
    {
        sal_strncpy(fullname, ethname, buf_len);
        return fullname;
    }
    
    if (0 == sal_strncmp(GLB_IFNAME_ETH_PREFIX, ethname, GLB_IFNAME_ETH_PREFIX_LEN))
    {
        if (!p_master || p_master->is_default)
        {
            sal_strncpy(fullname, ethname, buf_len);
            p = sal_strchr(fullname, '_');
            if (p)
            {
                p[0] = '/';
            }
        }
        else
        {
            rc = _if_get_info_by_ethname(ethname, &p_port, &p_prefix);
            if (rc < 0)
            {
                sal_strncpy(fullname, ethname, buf_len);
                p = sal_strchr(fullname, '_');
                if (p)
                {
                    p[0] = '/';
                }
            }
            else
            {
                if (p_port->sub_id)
                {
                    sal_snprintf(fullname, buf_len, "%s%u%c%u%c%u", p_prefix->full_prefix, 
                                                    p_port->slot_id, p_master->port_connection,
                                                    p_port->port_id, p_master->sub_port_connection, p_port->sub_id);
                }
                else
                {
                    sal_snprintf(fullname, buf_len, "%s%u%c%u", p_prefix->full_prefix, 
                                                    p_port->slot_id, p_master->port_connection,
                                                    p_port->port_id);
                }
            }
        }
    }
    else
    {
        sal_strncpy(fullname, ethname, buf_len);
    }

    return fullname;
}

/* Convert user visible name to internal ethname. 
   TODO: this api should take shortname/fullname into account. */
char*
if_exname_to_ethname(char *exname, char *name, uint32 buf_len)    
{
    char *p = NULL;
    name[0] = '\0';

    sal_strncpy(name, exname, buf_len);
    p = sal_strchr(name, '/');
    if (p)
    {
        p[0] = '_';
    }

    return name;
}

int32
ctclib_if_init()
{
    _if_phy_port_master_init();
    return 0;
}

