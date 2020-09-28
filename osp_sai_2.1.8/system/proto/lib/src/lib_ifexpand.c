/****************************************************************************
* $Id$
*  Expand interface for CLI
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2008-08-14 10:25
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include <limits.h> /* for ULONG_MAX. */

#include "proto.h"
#include "glb_tempfile_define.h"
#include "lib_ifexpand_ex.h"
#include "lib_ifexpand.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
#ifndef DELIMITER
#define WHITE_SPACE(C)     ((C) == ' ' || (C) == '\t' || (C) == '\n')
#define DELIMITER(C)       (WHITE_SPACE(C) || (C) == '\0')
#endif

#ifndef MIN
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef MAX
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#endif

//#define PHY_IFNAME_CONNECTION      '-'
//#define PHY_SUB_PORT_CONNECTION      '/'

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/
/* 
 * Interface expand configurations
 */
#define M_INTFCFG_CNTS      (sizeof(g_stIntfCfg) / sizeof(g_stIntfCfg[0]))
/* XXX: 
 * 1) pszName should not start with the same alpha 
 */
SysIntfCfg_t g_stIntfCfg[] = 
{
    {
        .flags = M_EXP_FLAG_COMMON | M_EXP_FLAG_SLOTPORT,
        .type = E_INTF_PHYSICAL,
        .pszToken = "IFPHYSICAL",
        .pszName = "eth", /* not use for physical interface */
        .pszDesc = "Physical interface",
        .M_PHYINTF_STRUCT = 
        {
            .flags = M_PHYSICAL_FLAG_HAVE_GE
        },
        .M_COMINTF_SLOT_MIN = 0,
        .M_COMINTF_SLOT_MAX = 0,
        .M_COMINTF_PORT_MIN = 1,
        .M_COMINTF_PORT_MAX = 52
    },

    {
        .flags = (M_EXP_FLAG_COMMON | M_EXP_FLAG_PORTONLY),
        .type = E_INTF_AGG,
        .pszToken = "IFAGG",
        .pszName = "agg",
        .pszDesc = "Aggregation interface",
        .M_COMINTF_PORT_MIN = 1,
        .M_COMINTF_PORT_MAX = 55
    },

    {
        .flags = (M_EXP_FLAG_COMMON | M_EXP_FLAG_PORTONLY),
        .type = E_INTF_LOOPBACK,
        .pszToken = "IFLOOPBACK",
        .pszName = "loopback",
        .pszDesc = "Loopback interface",
        .M_COMINTF_PORT_MIN = 0,
        .M_COMINTF_PORT_MAX = 255
    },

    {
        .flags = (M_EXP_FLAG_COMMON | M_EXP_FLAG_PORTONLY),
        .type = E_INTF_VLAN,
        .pszToken = "IFVLAN",
        .pszName = "vlan",
        .pszDesc = "Vlan interface",
        .M_COMINTF_PORT_MIN = 1,
        .M_COMINTF_PORT_MAX = 4094 
    },

    {
        .flags = (M_EXP_FLAG_COMMON | M_EXP_FLAG_PORTONLY),
        .type = E_INTF_L2GRE,
        .pszToken = "IFL2GRE",
        .pszName = "l2gre",
        .pszDesc = "L2gre interface",
        .M_COMINTF_PORT_MIN = 1,
        .M_COMINTF_PORT_MAX = 200
    },

    /* modified by liwh for bug 47983, 2019-03-06 */
    /*{
        .flags = (M_EXP_FLAG_COMMON | M_EXP_FLAG_PORTONLY),
        .type = E_INTF_L3GRE,
        .pszToken = "IFL3GRE",
        .pszName = "l3gre",
        .pszDesc = "L3gre interface",
        .M_COMINTF_PORT_MIN = 1,
        .M_COMINTF_PORT_MAX = 1000
    },*/
    /* liwh end */
    
    {
        .flags = (M_EXP_FLAG_COMMON | M_EXP_FLAG_PORTONLY),
        .type = E_INTF_NVGRE,
        .pszToken = "IFNVGRE",
        .pszName = "nvgre",
        .pszDesc = "Nvgre interface",
        .M_COMINTF_PORT_MIN = 1,
        .M_COMINTF_PORT_MAX = 200
    },
    
    {
        .flags = (M_EXP_FLAG_COMMON | M_EXP_FLAG_PORTONLY),
        .type = E_INTF_VXLAN,
        .pszToken = "IFVXLAN",
        .pszName = "vxlan",
        .pszDesc = "Vxlan interface",
        .M_COMINTF_PORT_MIN = 1,
        .M_COMINTF_PORT_MAX = 200
    },
/* OSP does not support these intf types currently. */
#if 0
    {
        .flags = (M_EXP_FLAG_NOTINUSE | M_EXP_FLAG_PORTONLY),
        .type = E_INTF_TUNNEL,
        .pszToken = "IFTUNNEL",
        .pszName = "tunnel",
        .pszDesc = "Tunnel interface",
        .M_COMINTF_PORT_MIN = 0,
        .M_COMINTF_PORT_MAX = 1023 
    },

    {
        .flags = (M_EXP_FLAG_NOTINUSE | M_EXP_FLAG_PORTONLY),
        .type = E_INTF_NULL,
        .pszToken = "IFNULL",
        .pszName = "null",
        .pszDesc = "NULL interface",
        .M_COMINTF_PORT_MIN = 0,
        .M_COMINTF_PORT_MAX = 0 
    },
#endif
};

int g_insert_append_character = 1;
int g_intf_expand_last_portid = 0;
SysIntfCfg_t *g_stlastmatch = NULL;
static int interface_cfg_inited = 0;
/* E600MERGE begin */
static int g_phy_if_inited = 0;
/* E600MERGE end */

int glb_cli_have_phy_name = FALSE;

ifexpand_phy_port_master_t *ifexpand_phy_port_master = NULL;
    
/****************************************************************************
 *
* Function
* 
****************************************************************************/

void
intf_expand_phy_port_master_init (ifexpand_phy_port_master_t *ifexpand_phy_port_master)
{
    FILE *fd;
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
    
    fd = sal_fopen(GLB_PORT_INFO_FILE, SAL_OPEN_RO);
    if (fd)
    {
        while (sal_fgets(buf, 128, fd))
        {
            if (4 == sal_sscanf(buf, "port %d %d %d %d", &port_type, &slot_id, &port_id, &sub_id))
            {
                ifexpand_phy_port_master->valid_num++;
                ifexpand_phy_port_master->port[num].port_type = port_type;
                ifexpand_phy_port_master->port[num].slot_id = slot_id;
                ifexpand_phy_port_master->port[num].port_id = port_id;
                ifexpand_phy_port_master->port[num].sub_id = sub_id;
                /*the type is real existed*/
                ifexpand_phy_port_master->port_prefix[port_type-1].valid = TRUE;
                num++;
            }
        }
        sal_fclose(fd);
    }

    port_type = 0;
    ifexpand_phy_port_master->port_connection = '-';
    ifexpand_phy_port_master->sub_port_connection = '/';
    fd = sal_fopen(GLB_PORT_CLI_INFO_FILE, SAL_OPEN_RO);
    if (fd)
    {
        while (sal_fgets(buf, 128, fd))
        {
            /*update*/
            if (2 == sal_sscanf(buf, "port_connection %c sub_port_connection %c", &port_connection, &sub_port_connection))
            {
                ifexpand_phy_port_master->port_connection = port_connection;
                ifexpand_phy_port_master->sub_port_connection = sub_port_connection;
            }
            if (2 == sal_sscanf(buf, "split_type %d %d", &split_type, &split_port_type))
            {
                ifexpand_phy_port_master->split_type[split_type] = split_port_type;
            }
            else if (2 == sal_sscanf(buf, "prefix %s full_prefix %s", prefix, full_prefix))
            {
                sal_snprintf(ifexpand_phy_port_master->port_prefix[port_type].prefix, GLB_PHY_PORT_PREFIX_SZ, prefix);
                sal_snprintf(ifexpand_phy_port_master->port_prefix[port_type].full_prefix, GLB_PHY_PORT_FULL_PREFIX_SZ, full_prefix);
                sal_snprintf(ifexpand_phy_port_master->port_prefix[port_type].desc, IFEXPAND_ARGV_MAX_LEN, "%s interface", prefix);
                port_type++;
            }
        }
        sal_fclose(fd);
    }
    /*default prefix*/
    if (0 == port_type)
    {
        /*default*/
        sal_snprintf(ifexpand_phy_port_master->port_prefix[0].prefix, GLB_PHY_PORT_PREFIX_SZ, GLB_DEF_PHY_PORT_PREFIX);
        sal_snprintf(ifexpand_phy_port_master->port_prefix[0].full_prefix, GLB_PHY_PORT_FULL_PREFIX_SZ, GLB_DEF_PHY_PORT_FULL_PREFIX);
        sal_snprintf(ifexpand_phy_port_master->port_prefix[0].desc, IFEXPAND_ARGV_MAX_LEN, "Physical interface");
        ifexpand_phy_port_master->port_prefix[0].valid = TRUE;
    }

    for (port_type = 0; port_type < GLB_PHT_PORT_TYPE_MAX; port_type++)
    {
        if (ifexpand_phy_port_master->port_prefix[port_type].valid && !sal_strlen(ifexpand_phy_port_master->port_prefix[port_type].prefix))
        {
            log_sys(M_MOD_LIB, E_ERROR, "%s fail: port type %d existed without prefix", __FUNCTION__, port_type + 1);
        }
    }
    return;
}

ifexpand_phy_port_master_t *
intf_expand_get_master()
{
    /* Init ifexpand master if not ready. */
    if (NULL == ifexpand_phy_port_master)
    {
        ifexpand_phy_port_master = sal_malloc(sizeof(ifexpand_phy_port_master_t));
        sal_memset(ifexpand_phy_port_master, 0, sizeof(ifexpand_phy_port_master_t));
        intf_expand_phy_port_master_init(ifexpand_phy_port_master);
    }

    return ifexpand_phy_port_master;
}


/*check whether the sub port is existed in specified port*/
int
intf_phy_sub_port_in_range(uint16 port_id, uint16 sub_port_id)
{
    int i = 0;
    ifexpand_phy_port_master_t *p_phyport_master = intf_expand_get_master(); 

    for (i = 0; i < p_phyport_master->valid_num; i++)
    {
        if ((p_phyport_master->port[i].port_id == port_id) && (p_phyport_master->port[i].sub_id == sub_port_id))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*check whether the port is existed in specified type*/
int
intf_phy_port_in_range(uint16 port_type, uint16 port_id)
{
    int i = 0;
    ifexpand_phy_port_master_t *p_phyport_master = intf_expand_get_master(); 

    for (i = 0; i < p_phyport_master->valid_num; i++)
    {
        if (port_type != p_phyport_master->port[i].port_type)
        {
            continue;
        }
        if (p_phyport_master->port[i].port_id == port_id)
        {
            return TRUE;
        }
    }
    return FALSE;
}

int
intf_phy_port_may_in_range(uint16 port_type, uint16 port_id)
{
    int i = 0;
    ifexpand_phy_port_master_t *p_phyport_master = intf_expand_get_master(); 

    if (port_id == 0)
    {
        return TRUE;
    }

    for (i = 0; i < p_phyport_master->valid_num; i++)
    {
        if (port_type != p_phyport_master->port[i].port_type)
        {
            continue;
        }
        if (validate_may_in_range(p_phyport_master->port[i].port_id, p_phyport_master->port[i].port_id, port_id))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*return the port is spited. If port_id is 0, the port_id just use the last match value*/
int
intf_phy_port_in_split(uint16 port_id)
{
    int i = 0;
    ifexpand_phy_port_master_t *p_phyport_master = intf_expand_get_master(); 

    if (!port_id)
    {
        port_id = p_phyport_master->match_port_id;
    }
    
    for (i = 0; i < p_phyport_master->valid_num; i++)
    {
        if ((p_phyport_master->port[i].port_id == port_id) && p_phyport_master->port[i].sub_id)
        {
            return TRUE;
        }
    }
    return FALSE;
}

int
intf_phy_port_get_range(uint16 port_type, uint16 *min_port, uint16 *max_port)
{
    int i = 0;
    ifexpand_phy_port_master_t *p_phyport_master = intf_expand_get_master(); 
   
    for (i = 0; i < p_phyport_master->valid_num; i++)
    {
        if (port_type != p_phyport_master->port[i].port_type)
        {
            continue;
        }
        //first in
        if (0 == *min_port)
        {
            *min_port = p_phyport_master->port[i].port_id;
            *max_port = p_phyport_master->port[i].port_id;
            continue;
        }
        if (p_phyport_master->port[i].port_id < *min_port)
        {
            *min_port = p_phyport_master->port[i].port_id;
            continue;
        }
        if (p_phyport_master->port[i].port_id > *max_port)
        {
            *max_port = p_phyport_master->port[i].port_id;
            continue;
        }
    }
    return 0;
}

int
intf_phy_sub_port_get_range(uint16 port_id, uint16 *min_port, uint16 *max_port)
{
    int i = 0;
    ifexpand_phy_port_master_t *p_phyport_master = intf_expand_get_master(); 
   
    for (i = 0; i < p_phyport_master->valid_num; i++)
    {
        if (port_id != p_phyport_master->port[i].port_id)
        {
            continue;
        }
        //first in
        if (0 == *min_port)
        {
            *min_port = p_phyport_master->port[i].sub_id;
            *max_port = p_phyport_master->port[i].sub_id;
            continue;
        }
        if (p_phyport_master->port[i].sub_id < *min_port)
        {
            *min_port = p_phyport_master->port[i].sub_id;
            continue;
        }
        if (p_phyport_master->port[i].sub_id > *max_port)
        {
            *max_port = p_phyport_master->port[i].sub_id;
            continue;
        }
    }
    return 0;
}

SysIntfCfg_t *
ifrange_get_intfcfg()
{
    return g_stlastmatch;
}
char *
ifrange_endnum_match(char *cp, IntfMatch_e *pmatch)
{
    unsigned long nPortId = 0L;
    SysIntfCfg_t *pCfgmatch = NULL;
    char *sp = NULL;
    IntfMatch_e match;
    int portmin;
    
    /* check digit */
    sp = cp;
    while (!DELIMITER(*sp))
    {
        if (!sal_isdigit((int)*sp))
        {
            goto match_none_return;
        }

        if ((sp - cp) > GLB_PHY_PORT_FULL_NAME_SZ)
        {
            goto match_none_return;
        }

        sp++;
    }

    /* match base on interface name type */
    pCfgmatch = ifrange_get_intfcfg();
    if (pCfgmatch == NULL)
    {
        goto match_none_return;
    }
    
    errno = 0;
    nPortId = sal_strtou32(cp, NULL, 10);
    if ((0 == nPortId && EINVAL == errno) || (ULONG_MAX == nPortId && ERANGE == errno))
    {
        if (errno == ERANGE)
        {
            while (sal_isdigit(*cp))
            {
                cp++;
            }
        }
        goto match_none_return;
    }

    if (g_intf_expand_last_portid >= pCfgmatch->M_COMINTF_PORT_MIN)
    {
        portmin = g_intf_expand_last_portid;
    }
    else
    {
        portmin = pCfgmatch->M_COMINTF_PORT_MIN;
    }
    
    if (portmin <= nPortId
        && nPortId <= pCfgmatch->M_COMINTF_PORT_MAX)
    {
        while (sal_isdigit(*cp))
        {
            cp++;
        }
        match = E_INTF_MATCH_PORT_FULL;
    }
    else if (validate_may_in_range(portmin, 
                pCfgmatch->M_COMINTF_PORT_MAX, nPortId))
    {
        /* if the user input more digit
         * then the number will in the range
         */
        while (sal_isdigit(*cp))
        {
            cp++;
        }
        match = E_INTF_MATCH_PORT_INCOMPLETE;
    }
    else
    {
        goto match_none_return;
    }

    if ('\0' != *cp
        && !WHITE_SPACE(*cp))
    {
        /* invalid chars at the end */
        goto match_none_return;
    }

    if (pmatch)
    {
        *pmatch = match;
    }
    return cp;

match_none_return:
    if (pmatch)
    {
        *pmatch = E_INTF_MATCH_NONE;
    }
    return cp;
}

int
ifrange_get_complete_count()
{
    return 1;
}

void
ifrange_get_complete_string(char *oldtext, char **ptr, int *nCntRet)
{
    *nCntRet = 0;
    return;
}

int
ifrange_get_helpstr_len()
{
    int nLen = 0;
    SysIntfCfg_t *pCfgmatch = NULL;
    char szTmp[IFEXPAND_ARGV_MAX_LEN];

    /* match base on interface name type */
    pCfgmatch = ifrange_get_intfcfg();
    if (pCfgmatch == NULL)
    {
        return 0;
    }

    if ( g_intf_expand_last_portid >= pCfgmatch->M_COMINTF_PORT_MIN)
    {
        nLen = sal_snprintf(szTmp, IFEXPAND_ARGV_MAX_LEN, "<%u-%lu>",
                g_intf_expand_last_portid, pCfgmatch->M_COMINTF_PORT_MAX);
    }
    else
    {
        nLen = sal_snprintf(szTmp, IFEXPAND_ARGV_MAX_LEN, "<%lu-%lu>",
                pCfgmatch->M_COMINTF_PORT_MIN, pCfgmatch->M_COMINTF_PORT_MAX);
    }
     return nLen;
}

extern void intf_expand_output_by_width(struct cli *cli, 
        char *keyword, char *help, int max, int width);

int
ifrange_output_helpstr(struct cli *cli, int max, int width)
{
    SysIntfCfg_t *pCfgmatch = NULL;
    char szTmp[IFEXPAND_ARGV_MAX_LEN];

    /* match base on interface name type */
    pCfgmatch = ifrange_get_intfcfg();
    if (pCfgmatch == NULL)
    {
        return 0;
    }
    
    if ( g_intf_expand_last_portid >= pCfgmatch->M_COMINTF_PORT_MIN)
    {
        sal_snprintf(szTmp, IFEXPAND_ARGV_MAX_LEN, "<%u-%lu>",
                g_intf_expand_last_portid, pCfgmatch->M_COMINTF_PORT_MAX);
    }
    else
    {
        sal_snprintf(szTmp, IFEXPAND_ARGV_MAX_LEN, "<%lu-%lu>",
                pCfgmatch->M_COMINTF_PORT_MIN, pCfgmatch->M_COMINTF_PORT_MAX);
    }

    if (pCfgmatch->type == E_INTF_VLAN)
        intf_expand_output_by_width(cli, szTmp,
            M_HELPSTR_VLAN_NUMBER, max, width);
    else
        intf_expand_output_by_width(cli, szTmp,
            M_HELPSTR_PORT_NUMBER, max, width);

    return 0;
}

void
intf_expand_get_phy_if_info(void)
{
    FILE *fd;
    char buf[1024];
    int min_slot = 0;
    int max_slot = 0;
    int tmp_port = 0;
    
    if(interface_cfg_inited)
        return;

    fd = sal_fopen(GLB_BOARD_INFO_FILE, SAL_OPEN_RO);
    if (fd)
    {
        g_stIntfCfg[0].slot.nMax = 0;
        g_stIntfCfg[0].slot.nMin = 0;
        while (sal_fgets(buf, 256, fd))
        {
            if (sal_sscanf(buf, "slot %d %d port %d", &min_slot, &max_slot, &tmp_port))
            {
                g_stIntfCfg[0].slot.nMin = min_slot;
                g_stIntfCfg[0].slot.nMax = max_slot;
                g_stIntfCfg[0].port.nMax = tmp_port;
            }
        }
        sal_fclose(fd);
        interface_cfg_inited = 1;
    }

    return;
}

void
intf_expand_init(char *ifname)
{
    const char *p = NULL;
    ifexpand_phy_port_master_t *p_phyport_master = intf_expand_get_master(); 
    int str_len = 0;
    
/*Begin: Add by guoyh at 2008-09-21*/
    if (NULL == ifname)
        return;

    intf_expand_get_phy_if_info();

/*End: Add by guoyh at 2008-09-21*/    
    p = ifname;
    while ('\0' != *p && WHITE_SPACE(*p))
    {
        p++;
    }

    str_len = sal_strlen(p) + 1;
    if (str_len >= GLB_PHY_PORT_FULL_NAME_SZ)
    {
        str_len = GLB_PHY_PORT_FULL_NAME_SZ - 1;
    }
    sal_snprintf(p_phyport_master->match_name, str_len, p);
    return;
}

char *
intf_expand_parse_current_ifname(void)
{
    ifexpand_phy_port_master_t *p_phyport_master = intf_expand_get_master(); 
    
    return p_phyport_master->match_name;
}

void
intf_expand_finish()
{
    ifexpand_phy_port_master_t *p_phyport_master = intf_expand_get_master(); 
    
    sal_memset(p_phyport_master->match_name, 0, sizeof(p_phyport_master->match_name));
    return;
}


char *
intf_expand_match_portonly(char *cp, SysIntfCfg_t *pCfg, IntfMatch_e *pmatch)
{
    IntfMatch_e match = E_INTF_MATCH_NONE;
    char szName[GLB_PHY_PORT_FULL_NAME_SZ + 1];
    unsigned long nPortId = 0L;
    char *sp = NULL;
        
    if (NULL == pCfg || !(M_EXP_FLAG_PORTONLY & pCfg->flags))
    {
        sal_assert(0);
        goto match_none_return;
    }

    /* get interface name */
    sp = cp;
    while (!DELIMITER(*cp))
    {
        if (!sal_isalpha((int)*cp))
        {
            /* Added by kcao for l2gre/l3gre */
            if ((0 == sal_strncmp(sp, "l2", 2)) || (0 == sal_strncmp(sp, "l3", 2)))
            {
                if ((cp - sp) > 2)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        
        if ((cp - sp) > GLB_PHY_PORT_FULL_NAME_SZ)
        {
            goto match_none_return;
        }

        cp++;
    }

    if ((cp - sp) <= 0)
    {
        goto match_none_return;
    }

    sal_memcpy(szName, sp, (cp - sp));
    szName[cp - sp] = '\0';
    
    /* check interface name */
    if (sal_strncasecmp(pCfg->pszName, szName, sal_strlen(szName)))
    {
        goto match_none_return;
    }
    if (sal_strlen(pCfg->pszName) == sal_strlen(szName))
    {
        /* interface name string fully matched */
        match = E_INTF_MATCH_NAME_FULL;
    }
    else
    {
        match = E_INTF_MATCH_NAME_INCOMPLETE;
    }

    /* skip white space */
    while (WHITE_SPACE(*cp)) 
    {
        cp++;
    }
    if ('\0' == (*cp))
    {
        goto match_return;
    }

    /* get interface port index */
    errno = 0;
    nPortId = sal_strtou32(cp, NULL, 10);
    if ((0 == nPortId && EINVAL == errno) || (ULONG_MAX == nPortId && ERANGE == errno))
    {
        if (errno == ERANGE)
        {
            while (sal_isdigit(*cp))
            {
                cp++;
            }
        }
        goto match_none_return;
    }
    
    if (pCfg->M_COMINTF_PORT_MIN <= nPortId && nPortId <= pCfg->M_COMINTF_PORT_MAX)
    {
        while (sal_isdigit(*cp))
        {
            cp++;
        }
        match = E_INTF_MATCH_PORT_FULL;
        g_intf_expand_last_portid = nPortId;
    }
    else if (validate_may_in_range(pCfg->M_COMINTF_PORT_MIN, pCfg->M_COMINTF_PORT_MAX, nPortId))
    {
        /* if the user input more digit
         * then the number will in the range
         */
        while (sal_isdigit(*cp))
        {
            cp++;
        }
        match = E_INTF_MATCH_PORT_INCOMPLETE;
    }
    else
    {
        goto match_none_return;
    }

    if ('\0' != *cp && !WHITE_SPACE(*cp))
    {
        /* invalid chars at the end */
        goto match_none_return;
    }

    /* fully matched */
match_return:
    if (pmatch)
    {
        *pmatch = match;
    }
    return cp;

match_none_return:
    if (pmatch)
    {
        *pmatch = E_INTF_MATCH_NONE;
    }
    return cp;
}

char *
intf_expand_match_slotport(char *cp, SysIntfCfg_t *pCfg, IntfMatch_e *pmatch)
{
    int i = 0;
    int match_num = 0;
    IntfMatch_e match = E_INTF_MATCH_NONE;
    char szName[GLB_PHY_PORT_FULL_NAME_SZ + 1];
    unsigned long nSlotId = 0L;
    unsigned long nPortId = 0L;
    char *sp = NULL;
    char *prefix = NULL;
    char *full_prefix = NULL;
    char szTemp[GLB_PHY_PORT_FULL_NAME_SZ + 1];
    ifexpand_phy_port_master_t *p_phyport_master = intf_expand_get_master(); 
    
    if (NULL == pCfg || !(M_EXP_FLAG_SLOTPORT & pCfg->flags))
    {
        sal_assert(0);
        goto match_none_return;
    }

    p_phyport_master->match_port_id = 0;
    p_phyport_master->match_sub_port_id = 0;
    sal_memset(p_phyport_master->match_port_type, 0, sizeof(p_phyport_master->match_port_type));

    /* get interface name */
    sp = cp;
    while (!DELIMITER(*cp))
    {
        //add (cp != '-') by weij, before eth and - are seprated, in new cli we need to put them togather, as other prefix need't -
        if (!sal_isalpha((int)*cp) && (*cp != '-'))
        {
            break;
        }

        if ((cp - sp) > GLB_PHY_PORT_FULL_NAME_SZ)
        {
            goto match_none_return;
        }
        cp++;
    }

    if ((cp - sp) <= 0)
    {
        goto match_none_return;
    }

    sal_memcpy(szName, sp, (cp - sp));
    szName[cp - sp] = '\0';
    
    /* check interface name */
    for (i = 0; i < GLB_PHT_PORT_TYPE_MAX; i++)
    {
        if (!p_phyport_master->port_prefix[i].valid)
        {
            continue;
        }
        full_prefix = p_phyport_master->port_prefix[i].full_prefix;
        //not match this prefix
        if (sal_strncasecmp(full_prefix, szName, sal_strlen(szName)))
        {
            continue;
        }
        //save match phy type
        p_phyport_master->match_port_type[match_num] = i + 1;
        //match all name
        if (sal_strlen(full_prefix) == sal_strlen(szName))
        {
            /* interface name string fully matched */
            match = E_INTF_MATCH_NAME_FULL;
            break;
        }
        /*input valid but is unfinished*/
        match = E_INTF_MATCH_NAME_INCOMPLETE;
        match_num++;
    }

    /* check interface reduced name */
    if (E_INTF_MATCH_NAME_FULL != match)
    {
        match_num = 0;
        for (i = 0; i < GLB_PHT_PORT_TYPE_MAX; i++)
        {
            if (!p_phyport_master->port_prefix[i].valid)
            {
                continue;
            }
            prefix = p_phyport_master->port_prefix[i].prefix;
            //not match this prefix
            if (sal_strncasecmp(prefix, szName, sal_strlen(szName)))
            {
                continue;
            }
            //match all name
            if (sal_strlen(prefix) == sal_strlen(szName))
            {
                //save match phy type
                p_phyport_master->match_port_type[match_num] = i + 1;
                /* interface name string fully matched */
                match = E_INTF_MATCH_NAME_FULL;
                break;
            }
            match_num++;
        }
    }

    if (E_INTF_MATCH_NAME_INCOMPLETE == match)
    {
        goto match_return;
    }

    //no match
    if (GLB_PHT_PORT_TYPE_MAX == i)
    {
        goto match_none_return;
    }

    /*end of input string*/
    if (*cp == '\0')
    {
        goto match_return;
    }
    /*invalid input as the begin char must be digit*/
    if (!sal_isdigit((int)*cp))
    {
        goto match_none_return;
    }

    sp = cp;
    while (!DELIMITER(*cp))
    {
        if (!sal_isdigit((int)*cp))
        {
            break;
        }

        if ((cp - sp) > GLB_PHY_PORT_FULL_NAME_SZ)
        {
            goto match_none_return;
        }
        cp++;
    }
        
    /* get interface slot index */
    errno = 0;
    nSlotId = sal_strtou32(sp, NULL, 10);
    if ((0 == nSlotId && EINVAL == errno) || (ULONG_MAX == nSlotId && ERANGE == errno))
    {
        if (errno == ERANGE)
        {
            while (sal_isdigit(*cp))
            {
                cp++;
            }
        }
        goto match_none_return;
    }

    if (pCfg->M_COMINTF_SLOT_MIN <= nSlotId && nSlotId <= pCfg->M_COMINTF_SLOT_MAX)
    {
        match = E_INTF_MATCH_SLOT_FULL;
    }
    else if (validate_may_in_range(pCfg->M_COMINTF_SLOT_MIN, pCfg->M_COMINTF_SLOT_MAX, nSlotId))
    {
        /* if the user input more digit
         * then the number will in the range
         */
        while (sal_isdigit(*cp))
        {
            cp++;
        }
        match = E_INTF_MATCH_PORT_INCOMPLETE;
    }
    else
    {
        goto match_none_return;
    }

    if (*cp == p_phyport_master->port_connection)
    {
        match = E_INTF_MATCH_PORT_CON_FULL;
    }
    else if (*cp == '\0')
    {
        goto match_return;
    }
    else
    {
        goto match_none_return;
    }
    cp++;

    sp = cp;
    while (!DELIMITER(*cp))
    {
        if (!sal_isdigit((int)*cp))
        {
            break;
        }

        if ((cp - sp) > GLB_PHY_PORT_FULL_NAME_SZ)
        {
            goto match_none_return;
        }
        cp++;
    }
    
    if (cp == sp)
        goto match_return;

    sal_memcpy(szTemp, sp, (cp - sp));
    szName[cp - sp] = '\0';

    /* get interface port index */
    errno = 0;
    nPortId = sal_strtou32(sp, NULL, 10);
    if ((0 == nPortId && EINVAL == errno) || (ULONG_MAX == nPortId && ERANGE == errno))
    {
        if (errno == ERANGE)
        {
            while (sal_isdigit(*cp))
            {
                cp++;
            }
        }
        goto match_none_return;
    }
    if (intf_phy_port_in_range(p_phyport_master->match_port_type[0], nPortId))
    {
        match = E_INTF_MATCH_PORT_FULL;
        p_phyport_master->match_port_id = nPortId;
        g_intf_expand_last_portid = nPortId;
    }
    else if (intf_phy_port_may_in_range(p_phyport_master->match_port_type[0], nPortId))
    {
        /* if the user input more digit then the number will in the range*/
        while (sal_isdigit(*cp))
        {
            cp++;
        }
        match = E_INTF_MATCH_PORT_INCOMPLETE;
    }
    else
    {
        goto match_none_return;
    }

    /*match should finish for unsplit port*/
    if ('\0' != *cp && !WHITE_SPACE(*cp) && !intf_phy_port_in_split(nPortId))
    {
        /* invalid chars at the end */
        goto match_none_return;
    }

    if (intf_phy_port_in_split(nPortId))
    {
        if (*cp == p_phyport_master->sub_port_connection)
        {
            match = E_INTF_MATCH_SUB_PORT_CON_FULL;
        }
        else if (*cp == '\0')
        {
            //just find the father port
            goto match_return;
        }
        else
        {
            goto match_none_return;
        }
        cp++;

        sp = cp;
        while (!DELIMITER(*cp))
        {
            if (!sal_isdigit((int)*cp))
            {
                break;
            }

            if ((cp - sp) > GLB_PHY_PORT_FULL_NAME_SZ)
            {
                goto match_none_return;
            }
            cp++;
        }
        
        if (cp == sp)
            goto match_return;

        /* get interface port index */
        errno = 0;
        nPortId = sal_strtou32(sp, NULL, 10);
        if ((0 == nPortId && EINVAL == errno) || (ULONG_MAX == nPortId && ERANGE == errno))
        {
            if (errno == ERANGE)
            {
                while (sal_isdigit(*cp))
                {
                    cp++;
                }
            }
            goto match_none_return;
        }

        if (intf_phy_sub_port_in_range(p_phyport_master->match_port_id, nPortId))
        {
            p_phyport_master->match_sub_port_id = nPortId;
            match = E_INTF_MATCH_SUB_PORT_FULL;
        }
        else
        {
            goto match_none_return;
        }

        /*match should finish*/
        if ('\0' != *cp && !WHITE_SPACE(*cp))
        {
            /* invalid chars at the end */
            goto match_none_return;
        }
    }

match_return:
    if (pmatch)
    {
        *pmatch = match;
        glb_cli_have_phy_name = TRUE;
    }
    return cp;

match_none_return:
    if (pmatch)
    {
        *pmatch = E_INTF_MATCH_NONE;
    }

    return cp;
}

char *
intf_expand_match(char *cp, IntfType_e intfType, IntfMatch_e *pmatch)
{
    int nCnt = M_INTFCFG_CNTS;
    SysIntfCfg_t *pCfg = NULL;
    IntfMatch_e max_match = E_INTF_MATCH_NONE;
    IntfMatch_e cur_match = E_INTF_MATCH_NONE;
    char *sp = NULL;
    int i = 0;
    char *retp = NULL;
    ifexpand_phy_port_master_t *p_phyport_master = intf_expand_get_master(); 
    
    /* E600MERGE begin */
    if (0 == g_phy_if_inited)
    {
        intf_expand_get_phy_if_info();
        g_phy_if_inited = 1;
    }
    /* E600MERGE end */
    
    /* match base on interface name type */
    for (i = 0; i < nCnt; i++)
    {
        pCfg = &g_stIntfCfg[i];

        if (pCfg->flags & M_EXP_FLAG_NOTINUSE)
        {
            continue;
        }

        if (intfType != E_INTF_ALL)
        {
#if 0
            if (intfType == E_INTF_COMMON)
            {
                if (!(pCfg->flags & M_EXP_FLAG_COMMON))
                {
                    continue;
                }
            }
            else if (intfType != pCfg->type)
            {
                continue;
            }
#else
            if (!(intfType & pCfg->type))
            {
                continue;
            }    
#endif
        }

        cur_match = E_INTF_MATCH_NONE;
        if (pCfg->flags & M_EXP_FLAG_PORTONLY)
        {
            sp = cp;
            /* match port only interfaces (all logical interfaces) */
            sp = intf_expand_match_portonly(sp, pCfg, &cur_match);
        }
        else if (pCfg->flags & M_EXP_FLAG_SLOTPORT)
        {
            sp = cp;
            /* match slot_port interfaces (phy interface) */
            sp = intf_expand_match_slotport(sp, pCfg, &cur_match);
        }
        else
        {
        }
        
        if (E_INTF_MATCH_NONE != cur_match)
        {
            if (NULL == retp)
            {
                retp = sp;
            }
            else if (sp < retp)
            {
                retp = sp;
            }
        }

        if (cur_match > max_match)
        {
            max_match = cur_match;
            g_stlastmatch = pCfg;
        }
    }
    
    if (pmatch)
    {
        *pmatch = max_match;
    }

    /*for bug 13193*/
    if (max_match == E_INTF_MATCH_NAME_FULL && *retp == p_phyport_master->port_connection)
    {
        *pmatch = E_INTF_MATCH_NONE;
        return NULL;
    }
    if (max_match == E_INTF_MATCH_PORT_CON_FULL && *retp == p_phyport_master->port_connection)
    {
        *pmatch = E_INTF_MATCH_NONE;
        return NULL;
    }
    if (max_match == E_INTF_MATCH_SUB_PORT_CON_FULL && *retp == p_phyport_master->sub_port_connection)
    {
        *pmatch = E_INTF_MATCH_NONE;
        return NULL;
    }
    return retp;
}

int
intf_expand_get_complete_count(IntfType_e intfType)
{
    int i = 0;
    char *cp = NULL;
    int nCnt = M_INTFCFG_CNTS;
    SysIntfCfg_t *pCfg = NULL;
    char *pifname = intf_expand_parse_current_ifname();
    IntfMatch_e cur_match = E_INTF_MATCH_NONE;
    int bemptyname = 0;
    int matchcnt = 0;

    if (NULL == pifname)
    {
        return 0;
    }

    if (!sal_strcasecmp(pifname, ""))
    {
        bemptyname = 1;
    }

    /* match base on interface name type */
    for (i = 0; i < nCnt; i++)
    {
        pCfg = &g_stIntfCfg[i];

        if (pCfg->flags & M_EXP_FLAG_NOTINUSE)
        {
            continue;
        }

        if (intfType != E_INTF_ALL)
        {
#if 0
            if (intfType == E_INTF_COMMON)
            {
                if (!(pCfg->flags & M_EXP_FLAG_COMMON))
                {
                    continue;
                }
            }
            else if (intfType != pCfg->type)
            {
                continue;
            }
#else
            if (!(intfType & pCfg->type))
            {
                continue;
            }    
#endif
        }

        cur_match = E_INTF_MATCH_NONE;
        if (pCfg->flags & M_EXP_FLAG_PORTONLY)
        {
            /* match port only interfaces (all logical interfaces) */
            if (bemptyname)
            {
                matchcnt += 1;
            }
            else
            {
                cp = intf_expand_match_portonly(pifname, pCfg, &cur_match);
                if (E_INTF_MATCH_NONE != cur_match && ('\0' == *cp || WHITE_SPACE(*cp)))
                {
                    if (E_INTF_MATCH_NAME_INCOMPLETE == cur_match
                        || E_INTF_MATCH_PORT_FULL == cur_match /* IF_SPLITTING: treat as full match. */)
                    {
                        matchcnt += 1;
                    }
                }
            }
        }
        else if (pCfg->flags & M_EXP_FLAG_SLOTPORT)
        {
            /* match phy interfaces*/
            if (bemptyname)
            {
                matchcnt += 1;
            }
            else
            {
                cp = intf_expand_match_slotport(pifname, pCfg, &cur_match);
                if (E_INTF_MATCH_NONE != cur_match && ('\0' == *cp || WHITE_SPACE(*cp)))
                {
                    if (E_INTF_MATCH_NAME_INCOMPLETE == cur_match
                        ||E_INTF_MATCH_NAME_FULL == cur_match
                        //||E_INTF_MATCH_SLOT_CON_FULL == cur_match
                        ||E_INTF_MATCH_SLOT_INCOMPLETE == cur_match
                        ||E_INTF_MATCH_SLOT_FULL == cur_match
                        ||E_INTF_MATCH_PORT_CON_FULL == cur_match
                        ||E_INTF_MATCH_PORT_FULL == cur_match
                        ||E_INTF_MATCH_SUB_PORT_CON_FULL == cur_match
                        ||E_INTF_MATCH_SUB_PORT_FULL == cur_match)
                    {
                        matchcnt += 1;
                    }
                }
            }
        }
        else 
        {
            continue;
        }
    }
    
    return matchcnt;
}

void
intf_expand_get_complete_string(char *oldtext, IntfType_e intfType,
        char **ptr, int *nCntRet)
{
    int i = 0;
    char *cp = NULL;
    int nCnt = M_INTFCFG_CNTS;
    int port_type = 0;
    SysIntfCfg_t *pCfg = NULL;
    char *pifname = intf_expand_parse_current_ifname();
    IntfMatch_e cur_match = E_INTF_MATCH_NONE;
    int bemptyname = 0;
    char szName[GLB_PHY_PORT_FULL_NAME_SZ + 1];
    ifexpand_phy_port_master_t *p_phyport_master = intf_expand_get_master(); 
    
    if (NULL == pifname)
    {
        return;
    }

    if (!sal_strcasecmp(pifname, ""))
    {
        bemptyname = 1;
    }

    /* match base on interface name type */
    *nCntRet = 0;
    for (i = 0; i < nCnt; i++)
    {
        pCfg = &g_stIntfCfg[i];

        if (pCfg->flags & M_EXP_FLAG_NOTINUSE)
        {
            continue;
        }

        if (intfType != E_INTF_ALL)
        {
#if 0
            if (intfType == E_INTF_COMMON)
            {
                if (!(pCfg->flags & M_EXP_FLAG_COMMON))
                {
                    continue;
                }
            }
            else if (intfType != pCfg->type)
            {
                continue;
            }
#else
            if (!(intfType & pCfg->type))
            {
                continue;
            }    
#endif
        }

        cur_match = E_INTF_MATCH_NONE;
        if (pCfg->flags & M_EXP_FLAG_PORTONLY)
        {
            /* match port only interfaces (all logical interfaces) */
            if (bemptyname)
            {
                ptr[*nCntRet] = sal_strdup(pCfg->pszName);
                if (ptr[*nCntRet])
                {
                    *nCntRet += 1;
                }
            }
            else
            {
                cp = intf_expand_match_portonly(pifname, pCfg, &cur_match);
                if (E_INTF_MATCH_NONE != cur_match && ('\0' == *cp || WHITE_SPACE(*cp)))
                {
                    if (E_INTF_MATCH_NAME_INCOMPLETE == cur_match
                        || E_INTF_MATCH_PORT_FULL == cur_match /* IF_SPLITTING: treat as full match. */)
                    {
                        ptr[*nCntRet] = sal_strdup(pCfg->pszName);
                        if (ptr[*nCntRet])
                        {
                            *nCntRet += 1;
                        }
                    }
                }
            }
        }
        else if (pCfg->flags & M_EXP_FLAG_SLOTPORT )
        {
            /* match phy interfaces*/
            if (bemptyname)
            {
                g_insert_append_character = 0;

                /* save and pring all existed port type */
                for (port_type = 0; port_type < GLB_PHT_PORT_TYPE_MAX; port_type++)
                {
                    if (!p_phyport_master->port_prefix[port_type].valid)
                    {
                        continue;
                    }
                    sal_snprintf(szName, GLB_PHY_PORT_FULL_NAME_SZ, p_phyport_master->port_prefix[port_type].full_prefix);
                    ptr[*nCntRet] = sal_strdup(szName);
                    if (ptr[*nCntRet])
                    {
                        *nCntRet += 1;
                    }
                }
            }
            else
            {
                cp = intf_expand_match_slotport(pifname, pCfg, &cur_match);
                if (E_INTF_MATCH_NONE != cur_match && ('\0' == *cp || WHITE_SPACE(*cp)))
                {
                    if (E_INTF_MATCH_NAME_FULL > cur_match)
                    {
                        /*make just one phy port prefix is matched*/
                        if (0 == p_phyport_master->match_port_type[1])
                        {
                            g_insert_append_character = 0;
                            sal_snprintf (szName, GLB_PHY_PORT_FULL_NAME_SZ, 
                                                p_phyport_master->port_prefix[p_phyport_master->match_port_type[0]-1].full_prefix);
                            ptr[*nCntRet] = sal_strdup(szName);
                            if (ptr[*nCntRet])
                            {
                                *nCntRet += 1;
                            }
                        }
                    }
                    else if (E_INTF_MATCH_SLOT_FULL == cur_match)
                    {
                        g_insert_append_character = 0;
                        sal_snprintf(szName, GLB_PHY_PORT_FULL_NAME_SZ, "%s%c", pifname, p_phyport_master->port_connection);
                        ptr[*nCntRet] = sal_strdup(szName);
                        if (ptr[*nCntRet])
                        {
                            *nCntRet += 1;
                        }
                    }
                    else if (E_INTF_MATCH_PORT_FULL == cur_match)
                    {
                        if ((intf_phy_port_in_split(p_phyport_master->match_port_id)))
                        {
                            g_insert_append_character = 0;
                            sal_snprintf(szName, GLB_PHY_PORT_FULL_NAME_SZ, "%s%c", pifname, p_phyport_master->sub_port_connection);
                            ptr[*nCntRet] = sal_strdup(szName);
                            if (ptr[*nCntRet])
                            {
                                *nCntRet += 1;
                            }
                        }
                        else
                        {
                            ptr[*nCntRet] = sal_strdup(pifname);
                            if (ptr[*nCntRet])
                            {
                                *nCntRet += 1;
                            }
                        }
                    }
                    else if (E_INTF_MATCH_SUB_PORT_FULL == cur_match)
                    {
                        ptr[*nCntRet] = sal_strdup(pifname);
                        if (ptr[*nCntRet])
                        {
                            *nCntRet += 1;
                        }
                    }
                }            
            }
        }
        else
        {
            continue;
        }
    }
    return;
}

extern void gen_output_helpstr_by_width(struct cli *, char *, char *, int, int);
void
intf_expand_output_by_width(struct cli *cli, 
        char *keyword, char *help, int max, int width)
{
    // TO BE DELETED...
    
    return;
}

int
intf_expand_get_helpstr_len(IntfType_e intfType)
{
    int i = 0;
    int nLen = 0;
    int nTmpLen = 0;
    int nRetLen = 0;
    int match_num = 0;
    char *cp = NULL;
    int nCnt = M_INTFCFG_CNTS;
    SysIntfCfg_t *pCfg = NULL;
    char *pifname = intf_expand_parse_current_ifname();
    IntfMatch_e cur_match = E_INTF_MATCH_NONE;
    int bemptyname = 0;
    char szTmp[IFEXPAND_ARGV_MAX_LEN];
    int phy_type = 0;
    ifexpand_phy_port_master_t *p_phyport_master = intf_expand_get_master(); 
    
    if (NULL == pifname)
    {
        return 0;
    }

    if (!sal_strcasecmp(pifname, ""))
    {
        bemptyname = 1;
    }

    /* match base on interface name type */
    for (i = 0; i < nCnt; i++)
    {
        pCfg = &g_stIntfCfg[i];

        if (pCfg->flags & M_EXP_FLAG_NOTINUSE)
        {
            continue;
        }

        if (intfType != E_INTF_ALL)
        {
#if 0
            if (intfType == E_INTF_COMMON)
            {
                if (!(pCfg->flags & M_EXP_FLAG_COMMON))
                {
                    continue;
                }
            }
            else if (intfType != pCfg->type)
            {
                continue;
            }
#else
            if (!(intfType & pCfg->type))
            {
                continue;
            }    
#endif
        }

        cur_match = E_INTF_MATCH_NONE;
        if (pCfg->flags & M_EXP_FLAG_PORTONLY)
        {
            /* match port only interfaces (all logical interfaces) */
            if (bemptyname)
            {
                nLen = sal_strlen(pCfg->pszName);
            }
            else
            {
                cp = intf_expand_match_portonly(pifname, pCfg, &cur_match);
                if (E_INTF_MATCH_NONE != cur_match && ('\0' == *cp || WHITE_SPACE(*cp)))
                {
                    if (cur_match < E_INTF_MATCH_NAME_FULL)
                    {
                        nTmpLen = sal_strlen(pCfg->pszName);
                        nLen = MAX(nLen, nTmpLen);
                    }
                    else
                    {
                        nLen = sal_snprintf(szTmp, IFEXPAND_ARGV_MAX_LEN, "<%lu-%lu>",
                                pCfg->M_COMINTF_PORT_MIN, pCfg->M_COMINTF_PORT_MAX);
                    }
                }
            }
            nRetLen = MAX(nLen, nRetLen);
        }
        else if (pCfg->flags & M_EXP_FLAG_SLOTPORT)
        {
             /* match phy interface */
            if (bemptyname)
            {
                //get the max nLen
                for (phy_type = 0; phy_type < GLB_PHT_PORT_TYPE_MAX; phy_type++)
                {
                    if (p_phyport_master->port_prefix[phy_type].valid)
                    {
                        if (sal_strlen(p_phyport_master->port_prefix[phy_type].full_prefix) > nLen)
                        {
                            nLen = sal_strlen(p_phyport_master->port_prefix[phy_type].full_prefix);
                        }
                    }
                }
            }
            else
            {
                cp = intf_expand_match_slotport(pifname, pCfg, &cur_match);
                if (E_INTF_MATCH_NONE != cur_match && ('\0' == *cp || WHITE_SPACE(*cp)))
                {
                    if (cur_match < E_INTF_MATCH_NAME_FULL)
                    {
                        for (match_num = 0; match_num < GLB_PHT_PORT_TYPE_MAX; match_num++)
                        {
                            if (!p_phyport_master->match_port_type[match_num])
                            {
                                continue;
                            }
                            nTmpLen = sal_strlen(p_phyport_master->port_prefix[p_phyport_master->match_port_type[match_num]-1].full_prefix);
                            nLen = MAX(nLen, nTmpLen);
                        }
                    }
                    else
                    {
                        // TODO: weij
                        nLen = sal_snprintf(szTmp, IFEXPAND_ARGV_MAX_LEN, "<%lu-%lu>",
                                pCfg->M_COMINTF_PORT_MIN, pCfg->M_COMINTF_PORT_MAX);
                    }
                }
            }
#ifdef _CENTEC_  /* added by zhaozh, for bug 7171, 2008-10-29 */
            nRetLen = MAX(nLen, nRetLen);
#endif
        }
        else
        {
            continue;
        }
    }

    return nRetLen;
}

int
intf_expand_generate_help(IFEXPAND_HELP_OUT_FUNC pCB, void *pArg, IntfType_e intfType)
{
    int i = 0;
    int phy_type = 0;
    uint16 min_port = 0;
    uint16 max_port = 0;
    int match_num = 0;
    uint16 port_type = 0;
    char *cp = NULL;
    int nCnt = M_INTFCFG_CNTS;
    SysIntfCfg_t *pCfg = NULL;
    char *pifname = intf_expand_parse_current_ifname();
    IntfMatch_e cur_match = E_INTF_MATCH_NONE;
    int bemptyname = 0;
    char szTmp[IFEXPAND_ARGV_MAX_LEN];
    ifexpand_phy_port_master_t *p_phyport_master = intf_expand_get_master(); 
    
    if (NULL == pifname)
    {
        return 0;
    }

    if (!sal_strcasecmp(pifname, ""))
    {
        bemptyname = 1;
    }

    /* match base on interface name type */
    for (i = 0; i < nCnt; i++)
    {
        pCfg = &g_stIntfCfg[i];

        if (pCfg->flags & M_EXP_FLAG_NOTINUSE)
        {
            continue;
        }

        if (intfType != E_INTF_ALL)
        {
#if 0
            if (intfType == E_INTF_COMMON)
            {
                if (!(pCfg->flags & M_EXP_FLAG_COMMON))
                {
                    continue;
                }
            }
            else if (intfType != pCfg->type)
            {
                continue;
            }
#else
            if (!(intfType & pCfg->type))
            {
                continue;
            }    
#endif
        }

        cur_match = E_INTF_MATCH_NONE;
        if (pCfg->flags & M_EXP_FLAG_PORTONLY)
        {
            /* match port only interfaces (all logical interfaces) */
            if (bemptyname)
            {
                pCB(pArg, pCfg->pszName, pCfg->pszDesc);
            }
            else
            {
                cp = intf_expand_match_portonly(pifname, pCfg, &cur_match);
                if (E_INTF_MATCH_NONE != cur_match && ('\0' == *cp || WHITE_SPACE(*cp)))
                {
                    if (cur_match < E_INTF_MATCH_NAME_FULL)
                    {
                        pCB(pArg, pCfg->pszName, pCfg->pszDesc);
                    }
                    else
                    {
                        sal_snprintf(szTmp, IFEXPAND_ARGV_MAX_LEN, "<%lu-%lu>", pCfg->M_COMINTF_PORT_MIN, pCfg->M_COMINTF_PORT_MAX);
                        if (pCfg->type == E_INTF_VLAN)
                            pCB(pArg, szTmp, M_HELPSTR_VLAN_NUMBER);
                        else
                            pCB(pArg, szTmp, M_HELPSTR_PORT_NUMBER);
                    }
                }
            }
        }
        else if (pCfg->flags & M_EXP_FLAG_SLOTPORT)
        {
            /* match phy interface*/
            if (bemptyname)
            {
                //intf_expand_output_by_width(cli, pCfg->pszName, pCfg->pszDesc, max, width);
                for (phy_type = 0; phy_type < GLB_PHT_PORT_TYPE_MAX; phy_type++)
                {
                    if (p_phyport_master->port_prefix[phy_type].valid)
                    {
                        pCB(pArg, 
                            p_phyport_master->port_prefix[phy_type].full_prefix, 
                            p_phyport_master->port_prefix[phy_type].desc);
                    }
                }
            }
            else
            {
                cp = intf_expand_match_slotport(pifname, pCfg, &cur_match);
                if (E_INTF_MATCH_NONE != cur_match && ('\0' == *cp || WHITE_SPACE(*cp)))
                {
                    if (cur_match < E_INTF_MATCH_NAME_FULL)
                    {

                        for (match_num = 0; match_num < GLB_PHT_PORT_TYPE_MAX; match_num++)
                        {
                            if (!p_phyport_master->match_port_type[match_num])
                            {
                                continue;
                            }
                            port_type = p_phyport_master->match_port_type[match_num];
                            pCB(pArg, 
                                p_phyport_master->port_prefix[port_type-1].full_prefix,
                                p_phyport_master->port_prefix[port_type-1].desc);
                        }
                    }
                    else if (E_INTF_MATCH_NAME_FULL ==  cur_match)
                    {
                        // TODO: weij can still use it
                        sal_snprintf(szTmp, IFEXPAND_ARGV_MAX_LEN, "<%lu-%lu>", pCfg->M_COMINTF_SLOT_MIN, pCfg->M_COMINTF_SLOT_MAX);
                        pCB(pArg, szTmp, M_HELPSTR_SLOT_NUMBER);
                    }
                    else if (E_INTF_MATCH_SLOT_FULL ==  cur_match)
                    {
                        sal_snprintf(szTmp, IFEXPAND_ARGV_MAX_LEN, "%c", p_phyport_master->port_connection);
                        pCB(pArg, szTmp, " ");
                     }
                    else if (E_INTF_MATCH_PORT_CON_FULL ==  cur_match || E_INTF_MATCH_PORT_INCOMPLETE ==  cur_match)
                    {
                        intf_phy_port_get_range(p_phyport_master->match_port_type[0], &min_port, &max_port);
                        sal_snprintf(szTmp, IFEXPAND_ARGV_MAX_LEN, "<%d-%d>", min_port, max_port);
                        pCB(pArg, szTmp, M_HELPSTR_PORT_NUMBER);
                     }
                    else if (E_INTF_MATCH_PORT_FULL ==  cur_match)
                    {
                        if (intf_phy_port_in_split(p_phyport_master->match_port_id))
                        {
                            sal_snprintf(szTmp, IFEXPAND_ARGV_MAX_LEN, "%c", p_phyport_master->sub_port_connection);
                            pCB(pArg, szTmp, " ");
                         }
                        else
                        {
                            intf_phy_port_get_range(p_phyport_master->match_port_type[0], &min_port, &max_port);
                            sal_snprintf(szTmp, IFEXPAND_ARGV_MAX_LEN, "<%d-%d>", min_port, max_port);
                            pCB(pArg, szTmp, M_HELPSTR_PORT_NUMBER);
                         }
                    }
                    else if ((E_INTF_MATCH_SUB_PORT_CON_FULL ==  cur_match) || (E_INTF_MATCH_SUB_PORT_FULL ==  cur_match))
                    {
                        intf_phy_sub_port_get_range(p_phyport_master->match_port_id, &min_port, &max_port);
                        sal_snprintf(szTmp, IFEXPAND_ARGV_MAX_LEN, "<%d-%d>", min_port, max_port);
                        pCB(pArg, szTmp, M_HELPSTR_SUB_PORT_NUMBER);
                     }
                }
            }
        }        
        else
        {
            continue;
        }
    }

    return 0;
}

unsigned long
get_offset_for_value(unsigned long uValue)
{
    unsigned long offset = 1;

    if (uValue == 0)
    {
        return 9;
    }
    
    do
    {
        uValue = uValue / 10;
        offset *= 10;
    }
    while (uValue >= 10);
        
    return (offset - 1);
}

int
validate_may_in_range(unsigned long uMin, unsigned long uMax, unsigned long uValue)
{
    unsigned long uTmp = 0;
    unsigned long i = 0;
    unsigned long offset = 0;

    if (uValue == 0)
    {
        return 1;
    }
        
    if (uValue > uMax)
    {
        return 0;
    }

    if (uMin <= uValue && uValue <= uMax)
    {
        return 1;
    }

    offset = get_offset_for_value(uValue);
    uTmp = uValue * 10;
    for (i = 0; i <= offset; i++)
    {
        if (uMin <= (i + uTmp) && (i + uTmp) <= uMax)
        {
            return 1;
        }
    }

    return validate_may_in_range(uMin, uMax, uTmp);
}

int
intf_expand_transform_ifname(char *cp)
{
    /*transform ifname format.
        "v3" to "vlan3"
        "v0030" to "vlan30"
        "vlan 0030" to "vlan30"
        "eth  -  000  -  0010" to "eth-0-10"
        "agg    0010" to "agg10"*/
    char *space;               
    char *zero;            
    int firstdigit = 0;
    char ifname[IFEXPAND_ARGV_MAX_LEN+1];
    char *name;
    int nCnt = M_INTFCFG_CNTS;
    int i;
    SysIntfCfg_t *pCfg = NULL;
    ifexpand_phy_port_master_t *p_phyport_master = intf_expand_get_master(); 
    
    space = cp;
    while (*space != '\0')
    {
        if (sal_isspace((int)*space))
        {
            sal_strcpy(ifname, space+1);
            sal_strcpy(space, ifname);
            continue;
        }
        space++;
    }

    zero = cp;
    while (!DELIMITER(*zero))
    {
        if (*zero == p_phyport_master->port_connection)
        {
            firstdigit = 0;
        }
        if (*zero == '0' && sal_isdigit((int)*(zero+1)))
        {
            if (firstdigit == 0)
            {
                sal_strcpy(ifname, zero+1);
                sal_strcpy(zero, ifname);
                continue;
            }
        }
        else if (sal_isdigit((int)*(zero)))
        {
            firstdigit = 1;
        }
        zero++;
    }

    name = cp;
    while (*name != '\0')
    {
        if (!sal_isalpha((int)*name))
        {
            break;
        }
        name++;
    }
    if (name == cp || name - cp >= IFEXPAND_ARGV_MAX_LEN)
        return -1;
        
    /* match base on interface name type */
    for (i = 1; i < nCnt; i++)
    {
        pCfg = &g_stIntfCfg[i];
        if (sal_strncasecmp(pCfg->pszName, cp, (name-cp)))
        {
            continue;
        }
        if (sal_strlen(pCfg->pszName) != name-cp)
        {
            sal_strcpy(ifname, name);
            sal_strcpy(cp, pCfg->pszName);
            sal_strcpy(cp+sal_strlen(pCfg->pszName), ifname);
            break;
        }

    }
    return 0;
}


