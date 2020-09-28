#include "proto.h"
#include "cdbclient.h"
#include "glb_l2_define.h"
#include "glb_if_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"

typedef struct
{
    uint32 ifindex;
    uint32 ageing_timeout;
} cdbctl_arp_ageing_time_entry_t;


static int32
_cdbctl_show_arp_get_ageing_timeout(uint32 ifindex, cdbctl_arp_ageing_time_entry_t *array, uint32 array_size)
{
    uint32 i = 0;

    for (i = 0; i < array_size; i++)
    {
        if (array[i].ifindex == ifindex)
        {
            return array[i].ageing_timeout;
        }
    }

    return PM_E_FAIL;
}

static int32
_cdbctl_show_arp_one(char *line, cdbctl_arp_ageing_time_entry_t *array, uint32 array_size)
{
    cfg_parse_get_info_t info;
    char *ifname = "";
    char ifname_ext[IFNAME_EXT_SIZE];
    char *macaddr = NULL;
    uint32 ifindex = 0;
    char buf[IFNAME_SIZE];
    char age_min[CMD_BUF_16];
    int32 ageing_timeout = 0;
    int32 ageing_delay = 0;
    
    int32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);

    ifindex = cdb_int_str2val(info.value[TBL_ARP_FLD_IFINDEX], &rc);
    if (GLB_INVALID_IFINDEX != ifindex)
    {
        rc = tbl_interface_get_name_by_ifindex(ifindex, buf, IFNAME_SIZE);
        if (PM_E_NONE == rc)
        {
            ifname = buf;
        }
    }

    macaddr = info.value[TBL_ARP_FLD_MAC_ADDR];
    if (0 == sal_strcmp(info.value[TBL_ARP_FLD_TYPE], "static"))
    {
        sal_sprintf(age_min, "-");
    }
    else
    {
        /* refresh entry should not be showed */
        if (0 == sal_strcmp(info.value[TBL_ARP_FLD_STATUS], "refresh"))
        {
            ageing_delay = cdb_int_str2val(info.value[TBL_ARP_FLD_AGING_DELAY], &rc);
            if (rc < 0)
            {
                return PM_E_NONE;
            }
            ageing_timeout = _cdbctl_show_arp_get_ageing_timeout(ifindex, array, array_size);
            if (ageing_timeout < 0)
            {
                return PM_E_NONE;
            }
            
            sal_sprintf(age_min, "%d", (ageing_timeout-ageing_delay)/60);

            sal_printf("%d - %d = %d", ageing_timeout, ageing_delay, (ageing_timeout-ageing_delay)/60);
        }
        else if (0 == sal_strcmp(info.value[TBL_ARP_FLD_STATUS], "reachable"))
        {
            ageing_delay = cdb_int_str2val(info.value[TBL_ARP_FLD_AGING_DELAY], &rc);
            if (rc < 0)
            {
                return PM_E_NONE;
            }
            ageing_timeout = _cdbctl_show_arp_get_ageing_timeout(ifindex, array, array_size);
            if (ageing_timeout < 0)
            {
                return PM_E_NONE;
            }
            
            sal_sprintf(age_min, "%d", (ageing_timeout-ageing_delay)/60);

            sal_printf("%d - %d = %d", ageing_timeout, ageing_delay, (ageing_timeout-ageing_delay)/60);
        }
        else if (0 == sal_strcmp(info.value[TBL_ARP_FLD_STATUS], "incomplete"))
        {
            sal_sprintf(age_min, "0");
            macaddr = "Incomplete";
        }
    }

    IFNAME_ETH2SHORT(ifname, ifname_ext);
    sal_printf("%-10s %-16s %-10s %-16s %-10s\n", 
        "Internet",
        info.value[TBL_ARP_FLD_KEY],
        age_min,
        macaddr,
        ifname_ext);
    
    return PM_E_NONE;
}

#if 0
static int32
_cdbctl_show_arp_one(char *line, cdbctl_arp_ageing_time_entry_t *array, uint32 array_size)
{
    cfg_parse_get_info_t info;
    char ifname_ext[IFNAME_EXT_SIZE];
    char *macaddr = NULL;
    char *ifname = NULL;
    char *ae_status = NULL;
    
    cfg_get_parse(line, &info, 0);

    ifname = info.value[TBL_ARP_FLD_IFNAME];
    macaddr = info.value[TBL_ARP_FLD_MAC_ADDR];
    IFNAME_ETH2SHORT(ifname, ifname_ext);
    ae_status = info.value[TBL_ARP_FLD_STATUS];



    uint32 ifindex = 0;
    char buf[IFNAME_SIZE];
    char age_min[CMD_BUF_16];
    int32 ageing_timeout = 0;
    int32 ageing_delay = 0;
    int32 rc = PM_E_NONE;

    ifindex = cdb_int_str2val(info.value[TBL_ARP_FLD_IFINDEX], &rc);
    if (GLB_INVALID_IFINDEX != ifindex)
    {
        rc = tbl_interface_get_name_by_ifindex(ifindex, buf, IFNAME_SIZE);
        if (PM_E_NONE == rc)
        {
            ifname = buf;
        }
    }

    macaddr = info.value[TBL_ARP_FLD_MAC_ADDR];
    if (0 == sal_strcmp(info.value[TBL_ARP_FLD_TYPE], "static"))
    {
        sal_sprintf(age_min, "-");
    }
    else
    {
        /* refresh entry should not be showed */
        if (0 == sal_strcmp(info.value[TBL_ARP_FLD_STATUS], "refresh"))
        {
            //return PM_E_NONE;
        }
    
        if (0 == sal_strcmp(info.value[TBL_ARP_FLD_STATUS], "reachable"))
        {
            ageing_delay = cdb_int_str2val(info.value[TBL_ARP_FLD_AGING_DELAY], &rc);
            if (rc < 0)
            {
                return PM_E_NONE;
            }
            ageing_timeout = _cdbctl_show_arp_get_ageing_timeout(ifindex, array, array_size);
            if (ageing_timeout < 0)
            {
                return PM_E_NONE;
            }
            sal_sprintf(age_min, "%d", (ageing_timeout-ageing_delay)/60);
        }
        else
        {
            sal_sprintf(age_min, "0");
            macaddr = info.value[TBL_ARP_FLD_STATUS];
        }
    }

    return PM_E_NONE;

    /*
    if (0 == sal_strcmp("imcomplete", info.value[TBL_ARP_FLD_TYPE]))
    {
        if (0 == sal_strcmp("static", info.value[TBL_ARP_FLD_IM_TYPE]))
        {
            sal_printf("%-10s %-10s %-16s %-16s %-10s\n", 
                "Static",
                "Internet",
                info.value[TBL_ARP_FLD_KEY],
                "imcomplete",
                ifname_ext);
        }
        else if (0 == sal_strcmp("dynamic", info.value[TBL_ARP_FLD_IM_TYPE]))
        {
            sal_printf("%-10s %-10s %-16s %-16s %-10s\n", 
                "Dynamic",
                "Internet",
                info.value[TBL_ARP_FLD_KEY],
                "imcomplete",
                ifname_ext);
        }
    }
    else if (0 == sal_strcmp("static", info.value[TBL_ARP_FLD_TYPE]))
    {        
        sal_printf("%-10s %-10s %-16s %-16s %-10s\n", 
            "Static",
            "Internet",
            info.value[TBL_ARP_FLD_KEY],
            macaddr,
            ifname_ext);
    }
    else if (0 == sal_strcmp("dynamic", info.value[TBL_ARP_FLD_TYPE]) && ae_status)
    {

        switch(*ae_status - '0')
        {
        case AS_REACHABLE:
        case AS_REFRESH:
            sal_printf("%-10s %-10s %-16s %-16s %-10s\n", 
                "Dynamic",
                "Internet",
                info.value[TBL_ARP_FLD_KEY],
                macaddr,
                ifname_ext);
            break;
            
        case AS_INCOMPLETE:
            sal_printf("%-10s %-10s %-16s %-16s %-10s\n", 
                "Dynamic",
                "Internet",
                info.value[TBL_ARP_FLD_KEY],
                "Incomplete",
                ifname_ext);
            break;   

        default:
            break;
        }

        //(cli, " %-10s  %-15s  %-8s  %-14s  %-10s\n", "Protocol", "Address", "Age (min)", "Hardware Addr", "Interface")
    }
    else
    {
        return PM_E_NONE;
    }
    */
}

static int32
_cdbctl_show_arp_interface(char *line, char *if_name)
{
    cfg_parse_get_info_t info;
    char ifname_ext[IFNAME_EXT_SIZE];
    char *macaddr = NULL;
    char *ifname = NULL;
    char ifname_cp[IFNAME_EXT_SIZE];
    
    cfg_get_parse(line, &info, 0);

    ifname = info.value[TBL_ARP_FLD_IFNAME];
    macaddr = info.value[TBL_ARP_FLD_MAC_ADDR];
    sal_memset(ifname_cp, 0, sizeof(ifname_cp));

    if (sal_strstr(ifname, GLB_IFNAME_ETH_PREFIX))
    {
        IFNAME_ETH2SHORT(ifname, ifname_ext);
        sal_strcpy(ifname_cp, ifname_ext);
    }
    else
    {
        sal_strcpy(ifname_cp, ifname);
    }
    
    if (0 != sal_strncmp(if_name, ifname_cp, strlen(ifname_cp)))
    {
        return PM_E_NOT_EXIST;
    }
    
    if (0 == sal_strcmp("imcomplete", info.value[TBL_ARP_FLD_TYPE]))
    {
        if (0 == sal_strcmp("static", info.value[TBL_ARP_FLD_IM_TYPE]))
        {
            sal_printf("%-10s %-10s %-16s %-16s %-10s\n", 
                "Static",
                "Internet",
                info.value[TBL_ARP_FLD_KEY],
                "imcomplete",
                ifname_cp);
        }
        else if (0 == sal_strcmp("dynamic", info.value[TBL_ARP_FLD_IM_TYPE]))
        {
            sal_printf("%-10s %-10s %-16s %-16s %-10s\n", 
                "Dynamic",
                "Internet",
                info.value[TBL_ARP_FLD_KEY],
                "imcomplete",
                ifname_cp);
        }
    }
    else if (0 == sal_strcmp("static", info.value[TBL_ARP_FLD_TYPE]))
    {
        sal_printf("%-10s %-10s %-16s %-16s %-10s\n", 
            "Static",
            "Internet",
            info.value[TBL_ARP_FLD_KEY],
            macaddr,
            ifname_cp);
    }
    else if (0 == sal_strcmp("dynamic", info.value[TBL_ARP_FLD_TYPE]))
    {
        sal_printf("%-10s %-10s %-16s %-16s %-10s\n", 
            "Dynamic",
            "Internet",
            info.value[TBL_ARP_FLD_KEY],
            macaddr,
            ifname_cp);
    }
    else
    {
        return PM_E_NONE;
    }
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_arp_or_arp_interface(char *line, char *if_name)
{
    char ifname[IFNAME_EXT_SIZE];
    uint32 rc = PM_E_NONE;

    sal_memset(ifname, 0, sizeof(ifname));
    if (NULL != if_name && 0 != sal_memcmp(if_name, ifname, sizeof(IFNAME_EXT_SIZE)))
    {        
        if (NULL == sal_strstr(ifname, GLB_IFNAME_ETH_PREFIX))
        {
            IFNAME_ETH2SHORT(if_name, ifname);
        }

        rc = _cdbctl_show_arp_interface(line, ifname);
        return rc;
    }

    if (NULL != if_name && 0 == sal_memcmp(if_name, ifname, sizeof(char)*IFNAME_EXT_SIZE))
    {        
        rc =  _cdbctl_show_arp_one(line);
        return rc;
    }
    
    return PM_E_NONE;
}
#endif

static int32
_cdbctl_show_ip_interface_one(char *line)
{
    char *ipaddr    = NULL;
    char *ifname    = NULL;
    char *masklen   = NULL;
    char buf[CMD_BUF_256];    
    cfg_parse_get_info_t info;
    char ifname_ext[IFNAME_EXT_SIZE];

    sal_memset(buf, 0, sizeof(buf));
    cfg_get_parse(line, &info, 0);

    ifname = info.value[TBL_KERNEL_IF_FLD_NAME];
    ipaddr = info.value[TBL_KERNEL_IF_FLD_IP_ADDR];
    masklen = info.value[TBL_KERNEL_IF_FLD_MASKLEN];
    
    IFNAME_ETH2SHORT(ifname, ifname_ext);
    
    if (0 == sal_strcmp(ipaddr, "0.0.0.0") || 0 >= sal_atoi(masklen) ||  32 < sal_atoi(masklen))
    {
        return PM_E_INVALID_PARAM;
    }
    else
    {
        sal_printf("%-16s %-16s\n", ifname, ipaddr);
    }
    
    return PM_E_NONE;
}

int32
cdbctl_show_get_arp_ageing_timeout(cdbctl_arp_ageing_time_entry_t *array, uint32 *array_size)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    FILE *fp = NULL;
    char path[MAX_CMD_SHOW_FNAME_LEN];
    uint32 i = 0;
    uint32 ifindex = 0;
    uint32 ageing_timeout = 0;
    int32 rc = PM_E_NONE;
    
    rc = cdbclt_get_tbl(TBL_ROUTE_IF, NULL, 0, path);
    if (rc < 0)
    {
        return PM_E_FAIL;
    }
    
    fp = sal_fopen(path, "r");
    if (!fp)
    {
        return PM_E_FAIL;
    }
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        ifindex = cdb_int_str2val(info.value[TBL_ROUTE_IF_FLD_IFINDEX], &rc);
        if (rc < 0)
        {
            continue;
        }
        ageing_timeout = cdb_int_str2val(info.value[TBL_ROUTE_IF_FLD_ARP_TIMEOUT], &rc);
        if (rc < 0)
        {
            continue;
        }
        array[i].ifindex = ifindex;
        array[i].ageing_timeout = ageing_timeout;
        i++;
    }

    *array_size = i;
    sal_fclose(fp);
    sal_unlink(path);

    return PM_E_NONE;
}

static int32
_cdbctl_show_arp_entry(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cdbctl_arp_ageing_time_entry_t array[GLB_MAX_L3IF_NUM];
    uint32 array_size = 0;
    char  line[FILE_READ_LINE_LEN];

    sal_memset(line, 0, sizeof(line));
    cdbctl_show_get_arp_ageing_timeout(array, &array_size);
    
    sal_printf("%-10s %-16s %-10s %-16s %-10s\n", "Protocol", "Address", "Age (min)", "Hardware Addr", "Interface");
    sal_printf("----------+----------------+----------+----------------+----------\n");
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbctl_show_arp_one(line, array, array_size);
    }
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_ip_interface_entry(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];

    sal_printf("%-16s %-16s\n", "Interface", "IP-Address");
    sal_printf("----------------+----------------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbctl_show_ip_interface_one(line);
    }
        
    return PM_E_NONE;
}

int32
cdbctl_show_arp(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    return _cdbctl_show_arp_entry(p_data, data_len, fp, p_info);
}

int32
cdbctl_show_ip_interface(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    return _cdbctl_show_ip_interface_entry(p_data, data_len, fp, p_info);
}

static int32
_cdbctl_show_rt_entry_one(char *line)
{
    cfg_parse_get_info_t info;
    char ifname_ext[IFNAME_EXT_SIZE];
    char *macaddr = NULL;
    char *ifname = NULL;
    
    cfg_get_parse(line, &info, 0);

    ifname = info.value[TBL_ARP_FLD_IFNAME];
    macaddr = info.value[TBL_ARP_FLD_MAC_ADDR];
    IFNAME_ETH2SHORT(ifname, ifname_ext);

    if (0 == sal_strcmp("imcomplete", info.value[TBL_ARP_FLD_TYPE]))
    {
        sal_printf("%-10s %-16s %-16s %-10s\n", 
            "Internet",
            info.value[TBL_ARP_FLD_KEY],
            "imcomplete",
            ifname_ext);
    }
    else
    {
        sal_printf("%-10s %-16s %-16s %-10s\n", 
            "Internet",
            info.value[TBL_ARP_FLD_KEY],
            macaddr,
            ifname_ext);
    }
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_route_entry(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];

    sal_printf("%-10s %-16s %-16s %-10s\n", "Protocol", "Address", "Hardware Addr", "Interface");
    sal_printf("----------+----------------+----------+----------------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbctl_show_rt_entry_one(line);
    }

    return PM_E_NONE;
}

int32
cdbctl_show_iproute_node(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    return _cdbctl_show_route_entry(p_data, data_len, fp, p_info);
}


int32
cdbctl_show_nat_pool_node(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    return PM_E_NONE;
}



int32
cdbctl_show_nat_rule_node(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    return PM_E_NONE;
}


int32
cdbctl_show_nat_session_node(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    return PM_E_NONE;
}


int32
cdbctl_show_nat_session_limit_node(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    return PM_E_NONE;
}

