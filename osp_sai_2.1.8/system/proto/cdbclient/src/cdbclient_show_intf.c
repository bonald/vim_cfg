
#include "proto.h"
#include "cdbclient.h"
#include "glb_l2_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_port_stats_define.h"
#include "gen/tbl_port_stats.h"
#include "gen/tbl_port_stats_rate_define.h"
#include "gen/tbl_port_stats_rate.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_lag_global_define.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_qos_queue_shape_profile_define.h"
#include "gen/tbl_qos_drop_profile_define.h"
#include "lacp_define.h"
#include "cJSON.h"

const char*
interface_type_str(const char *mode, const char *vlan_type)
{
    if (!sal_strcmp(mode, "routed"))
    {
        return mode;
    }
    else if (!sal_strcmp(mode, "switch"))
    {
        return vlan_type;
    }
    else
    {
        return "invalid";
    }
}

const char*
interface_get_status(uint32 flags, uint32 is_short, uint32 errdisable_rsn, uint32 mirror_dest_flag)
{
    if (errdisable_rsn != ERRDIS_NO_ERRDIS)
    {
        if (is_short)
        {
            return "errdisable";
        }
        else
        {
            return "Error Disabled";
        }
    }
    else if ((mirror_dest_flag & MIRROR_DEST_FLAG) == MIRROR_DEST_FLAG)
    {
        if (is_short)
        {
            return "monitoring";
        }
        else
        {
            return "Monitoring";
        }
    }
    else if ((flags & (GLB_IFF_UP|GLB_IFF_RUNNING)) == (GLB_IFF_UP|GLB_IFF_RUNNING))
    {
        if (is_short)
        {
            return "up";
        }
        else
        {
            return "UP";
        }
    }
    else
    {
        if (is_short)
        {
            return (flags & GLB_IFF_UP) ? "down" : "admin down";
        }
        else
        {
            return (flags & GLB_IFF_UP) ? "DOWN" : "Administratively DOWN";
        }
    }
}

static int32
_cdbctl_show_interface_detail(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        sal_printf("%s\n", line);
    }

    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_status_one(char *line)
{
#define IF_DESC_WIDTH   16

    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t ds_info;
    uint32 flags = 0;
    uint32 mirror_dest_flag = 0;
    char *key = NULL;
    char *desc = NULL;
    int32 speed = 0;
    int32 oper_speed = 0;
    int32 duplex = 0;
    int32 oper_duplex = 0;
    uint32 errdisable_rsn_val = ERRDIS_NO_ERRDIS;
    char *mode = NULL;
    char *vlan_type = NULL;
    char *up = NULL;
    char *running = NULL;
    char *hw_type = NULL;
    char *phy_type = NULL;
    char *type = NULL;
    char *errdisable_rsn = NULL;
    char *mirror_dest = NULL;
    char new_desc[IF_DESC_WIDTH+1];
    int32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);

    key = info.value[TBL_INTERFACE_FLD_KEY];
    desc = info.value[TBL_INTERFACE_FLD_DESC];
    mode = info.value[TBL_INTERFACE_FLD_MODE];
    vlan_type = info.value[TBL_INTERFACE_FLD_VLAN_TYPE];
    up = info.value[TBL_INTERFACE_FLD_UP];
    running = info.value[TBL_INTERFACE_FLD_RUNNING];
    hw_type = info.value[TBL_INTERFACE_FLD_HW_TYPE];
    phy_type = info.value[TBL_INTERFACE_FLD_PHY_TYPE];
    errdisable_rsn = info.value[TBL_INTERFACE_FLD_ERRDISABLE_REASON];
    mirror_dest = info.value[TBL_INTERFACE_FLD_MIRROR_ENABLE];
        
    speed = cdb_enum_str2val(glb_port_speed_strs, GLB_PORT_SPEED_MAX, info.value[TBL_INTERFACE_FLD_SPEED]);
    oper_speed = cdb_enum_str2val(glb_port_speed_strs, GLB_PORT_SPEED_MAX, info.value[TBL_INTERFACE_FLD_OPER_SPEED]);
    duplex = cdb_enum_str2val(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, info.value[TBL_INTERFACE_FLD_DUPLEX]);
    oper_duplex = cdb_enum_str2val(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, info.value[TBL_INTERFACE_FLD_OPER_DUPLEX]);
    speed = (speed < 0) ? GLB_SPEED_AUTO : speed;
    oper_speed = (oper_speed < 0) ? GLB_SPEED_AUTO : oper_speed;
    duplex = (duplex < 0) ? GLB_DUPLEX_AUTO : duplex;
    oper_duplex = (oper_duplex < 0) ? GLB_DUPLEX_AUTO : oper_duplex;
    errdisable_rsn_val = cdb_int_str2val(errdisable_rsn, &rc);
    
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_BRGIF], &ds_info, 1);

    if (mirror_dest[0] == '1' && sal_strlen(mirror_dest) == 1)
    {
        GLB_SET_FLAG(mirror_dest_flag, MIRROR_DEST_FLAG);
    }
    if (up[0] == '1')
    {
        GLB_SET_FLAG(flags, GLB_IFF_UP);
    }
    if (running[0] == '1')
    {
        GLB_SET_FLAG(flags, GLB_IFF_RUNNING);
    }

    if (0 == sal_strcmp(hw_type, "Port"))
    {
        type = phy_type;
    }
    else if (0 == sal_strcmp(hw_type, "LAG"))
    {
        type = hw_type;
    }
    else
    {
        return rc;
    }

    sal_memset(new_desc, 0, sizeof(new_desc));
    if (sal_strlen(desc) > IF_DESC_WIDTH)
    {
        sal_strncpy(new_desc, desc, IF_DESC_WIDTH);
        new_desc[IF_DESC_WIDTH-3] = '.';
        new_desc[IF_DESC_WIDTH-2] = '.';
        new_desc[IF_DESC_WIDTH-1] = '.';
        new_desc[IF_DESC_WIDTH] = 0;
    }
    else
    {
        sal_strncpy(new_desc, desc, IF_DESC_WIDTH);
    }

    if (!sal_strcmp(vlan_type, "dot1q-tunnel"))
    {
        sal_strcpy(vlan_type, "qinq");
    }

    sal_printf("%-10s %-11s %-8s %-8s %-7s %-12s %-16s\n", 
        IFNAME_ETH2SHORT(key, ifname_ext),
        interface_get_status(flags, TRUE, errdisable_rsn_val, mirror_dest_flag),
        cdb_if_duplex_str(duplex, oper_duplex),
        cdb_if_speed_str(speed, oper_speed),
        interface_type_str(mode, vlan_type),
        type,
        new_desc);

    return rc;
}

static int32
_cdbctl_show_interface_status(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];

    sal_printf("%-10s %-11s %-8s %-8s %-7s %-12s %-16s\n", 
            "Name", "Status", "Duplex", "Speed", "Mode", "Type", "Description");
    sal_printf("----------+-----------+--------+--------+-------+------------+----------------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_status_one(line);
    }

    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_description_one(char *line)
{
#define IF_DESC_WIDTH_57   57

    char left_desc[256];
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    uint32 flags = 0;
    uint32 mirror_dest_flag = 0;
    char *key = NULL;
    char *desc = NULL;
    char *up = NULL;
    char *running = NULL;
    uint32 errdisable_rsn_val = ERRDIS_NO_ERRDIS;
    char *mirror_dest = NULL;
    int32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);

    key = info.value[TBL_INTERFACE_FLD_KEY];
    desc = info.value[TBL_INTERFACE_FLD_DESC];
    mirror_dest = info.value[TBL_INTERFACE_FLD_MIRROR_ENABLE];
    up = info.value[TBL_INTERFACE_FLD_UP];
    running = info.value[TBL_INTERFACE_FLD_RUNNING];
    
    if (0 == sal_strcmp(key, "cpu"))
    {
        return PM_E_NONE;
    }

    if (mirror_dest[0] == '1' && sal_strlen(mirror_dest) == 1)
    {
        GLB_SET_FLAG(mirror_dest_flag, MIRROR_DEST_FLAG);
    }
    if (up[0] == '1')
    {
        GLB_SET_FLAG(flags, GLB_IFF_UP);
    }
    if (running[0] == '1')
    {
        GLB_SET_FLAG(flags, GLB_IFF_RUNNING);
    }

    if (sal_strlen(desc) <= IF_DESC_WIDTH_57)
    {
        sal_printf("%-10s %-11s %-57s\n", 
            IFNAME_ETH2SHORT(key, ifname_ext),
            interface_get_status(flags, TRUE, errdisable_rsn_val, mirror_dest_flag),
            desc);
    }
    else
    {
        sal_memset(left_desc, 0, sizeof(left_desc));
        sal_strncpy(left_desc, desc+IF_DESC_WIDTH_57, 256);
        desc[IF_DESC_WIDTH_57] = CMD_ZERO_CHAR;
        sal_printf("%-10s %-11s %-57s\n", 
            IFNAME_ETH2SHORT(key, ifname_ext),
            interface_get_status(flags, TRUE, errdisable_rsn_val, mirror_dest_flag),
            desc);
        sal_printf("%-22s %-57s\n", "", left_desc);
    }

    return rc;
}

static int32
_cdbctl_show_interface_loopback_one(char *line)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    char dst_ifname_ext[IFNAME_EXT_SIZE];
    char ifname_full[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    uint32 mode = 0;
    uint32 if_index = 0;
    char *key = NULL;
    int32 rc = PM_E_NONE;
    enum loopback_mode_e
    {
        LOOPBACK_LB_NONE,
        LOOPBACK_LB_PHY_FAR,
        LOOPBACK_LB_PHY_NEAR,   
        LOOPBACK_LB_PHY_NEAR_DIAG,
        LOOPBACK_LB_PHY_LOOPBACK,
        LOOPBACK_LB_PORT,
        LOOPBACK_LB_PORT_SWAP,
        LOOPBACK_LB_MAX
    };
    
    cfg_get_parse(line, &info, 0);

    key = info.value[TBL_INTERFACE_FLD_KEY];
    mode = sal_atoi(info.value[TBL_INTERFACE_FLD_PHY_LOOPBACK_MODE]);
    if_index = sal_atoi(info.value[TBL_INTERFACE_FLD_INTERNAL_LP_IFINDEX]);
    if_get_name_by_ifindex(if_index, dst_ifname_ext, IFNAME_EXT_SIZE);
    
    if (LOOPBACK_LB_NONE != mode)
    {
        sal_printf("%-12s", IFNAME_ETH2FULL(key, ifname_ext));
        if (LOOPBACK_LB_PHY_FAR == mode)
        {
            sal_printf("%-12s ", "external");
        }
        else if (LOOPBACK_LB_PHY_NEAR == mode)
        {
            sal_printf("%-12s ", "internal");
            sal_printf("%-12s ", IFNAME_ETH2FULL(dst_ifname_ext, ifname_full));
            sal_printf("%-12s ", "-");
        }
        else if (LOOPBACK_LB_PHY_LOOPBACK == mode)
        {
            sal_printf("%-12s ", "internal");
        }
        else if (LOOPBACK_LB_PORT == mode)
        {
            sal_printf("%-12s ", "port");
            sal_printf("%-12s ", "-");
            sal_printf("%-12s ", "no");
        }
        else if (LOOPBACK_LB_PORT_SWAP == mode)
        {
            sal_printf("%-12s ", "port");
            sal_printf("%-12s ", "-");
            sal_printf("%-12s ", "yes");
        }
        sal_printf("\n");
    }

    return rc;
}


static int32
_cdbctl_show_interface_description(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];

    sal_printf("%-10s %-11s %-57s\n", "Name", "Status", "Description");
    sal_printf("----------+-----------+---------------------------------------------------------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_description_one(line);
    }

    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_loopback(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];
    sal_printf("%-12s %-12s \n", "Interface", "Type");
    sal_printf("------------+------------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_loopback_one(line);
    }

    return PM_E_NONE;
}
    
static int32
_cdbctl_show_interface_key_one(char *line)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    char *key = NULL;
    int32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);

    key = info.value[TBL_INTERFACE_FLD_KEY];
    sal_printf("%s\n", IFNAME_ETH2FULL(key, ifname_ext));
    return rc;
}

static int32
_cdbctl_show_interface_key(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_key_one(line);
    }

    return PM_E_NONE;
}
    
static int32
_cdbctl_show_interface_summary_one(char *line)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t info_stats;
    uint32 is_running = FALSE;
    char *hw_type = NULL;
    char rxbs[32] = {0};
    char rxps[32] = {0};
    char txbs[32] = {0};
    char txps[32] = {0};
    uint64 u_rxbs = 0;
    uint64 u_rxps = 0;
    uint64 u_txbs = 0;
    uint64 u_txps = 0;
    uint8 ifg_stats_en = 0;
    char *ifindex = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    char lines[FILE_READ_LINE_LEN];
    uint32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);

    hw_type = info.value[TBL_INTERFACE_FLD_HW_TYPE];
    if ((0 != sal_strcmp(hw_type, "Port")) && (0 != sal_strcmp(hw_type, "LAG")))
    {
        /* only show Port and  */
        return PM_E_NONE;
    }

    if (info.value[TBL_INTERFACE_FLD_RUNNING][0] == '1')
    {
        is_running = TRUE;
    }

    if (info.value[TBL_INTERFACE_FLD_IFG_STATS_EN][0] == '1')
    {
        ifg_stats_en = TRUE;
    }

    ifindex = info.value[TBL_INTERFACE_FLD_IFINDEX];
    sal_memset(&info_stats, 0, sizeof(info_stats));
    sal_memset(&lines, 0, sizeof(lines));

    PM_E_RETURN(cdbclt_get_tbl_line(TBL_PORT_STATS_RATE, ifindex, (sal_strlen(ifindex)+1),lines,FILE_READ_LINE_LEN));
    cfg_get_parse(lines, &info_stats, 0);

    /* modified by liwh for bug 52146, 2019-05-17 */
    if (0 == info_stats.argc)
    {
        return PM_E_NONE;    
    }
    /* liwh end */
    
    u_rxbs = sal_strtou64(info_stats.value[TBL_PORT_STATS_RATE_FLD_OCTETS_RCV_RATE], NULL, 10);
    u_rxps = sal_strtou64(info_stats.value[TBL_PORT_STATS_RATE_FLD_PKTS_RCV_RATE], NULL, 10);
    u_txbs = sal_strtou64(info_stats.value[TBL_PORT_STATS_RATE_FLD_OCTETS_SEND_RATE], NULL, 10);
    u_txps = sal_strtou64(info_stats.value[TBL_PORT_STATS_RATE_FLD_PKTS_SEND_RATE], NULL, 10);

    if (ifg_stats_en)
    {
        u_rxbs = u_rxbs + u_rxps*GLB_DEF_IPG_LEN*8;
        u_txbs = u_txbs + u_txps*GLB_DEF_IPG_LEN*8;
    }

    sal_sprintf(rxbs, "%"PRIu64, u_rxbs); 
    sal_sprintf(rxps, "%"PRIu64, u_rxps); 
    sal_sprintf(txbs, "%"PRIu64, u_txbs); 
    sal_sprintf(txps, "%"PRIu64, u_txps); 
    

    IFNAME_ETH2SHORT(info.value[TBL_INTERFACE_FLD_KEY], ifname_ext),
    sal_printf("%-10s %-6s %-14s %-14s %-14s %-14s\n", 
            ifname_ext, 
            is_running ? "UP" : "DOWN",
            rxbs,
            rxps,
            txbs,
            txps);
    
    return PM_E_NONE;
}
    
static int32
_cdbctl_show_interface_summary(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];

    sal_printf("RXBS: rx rate (bits/sec)         RXPS: rx rate (pkts/sec)\n");
    sal_printf("TXBS: tx rate (bits/sec)         TXPS: tx rate (pkts/sec)\n");
    sal_printf("\n");
    sal_printf("%-10s %-6s %-14s %-14s %-14s %-14s\n", 
            "Interface", "Link", "RXBS", "RXPS", "TXBS", "TXPS");
    sal_printf("----------+------+--------------+--------------+--------------+--------------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_summary_one(line);
    }

    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_port_security_max_mac_one(char *line)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t ds_info;

    char *hw_type = NULL;
    char *max_mac = "0";

    
    cfg_get_parse(line, &info, 0);

    hw_type = info.value[TBL_INTERFACE_FLD_HW_TYPE];
    if ((0 != sal_strcmp(hw_type, "Port")) && (0 != sal_strcmp(hw_type, "LAG")))
    {
        /* only show Port and  */
        return PM_E_NONE;
    }

    cfg_get_parse(info.value[TBL_INTERFACE_FLD_BRGIF], &ds_info, 1);


    max_mac = ds_info.value[DS_BRGIF_FLD_MAX_MAC];

    sal_printf("Maximum MAC addresse : %-10s\n", max_mac);
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_port_security_max_mac(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_port_security_max_mac_one(line);
    }

    return PM_E_NONE;
}

static int32
_cdbclt_show_interface_allowed_vlan(char *prefix_str, char *vlan_str, uint32 width)
{
    char blank_line[CMD_BUF_256];
    uint32 prefix_width = sal_strlen(prefix_str);
    uint32 line_limit = width - prefix_width;
    char *line = NULL;
    char *ptr = NULL;
    char *p_prefix = prefix_str;
    uint32 first_line = TRUE;

    sal_memset(blank_line, ' ', prefix_width);
    blank_line[prefix_width] = CMD_ZERO_CHAR;

    if (sal_strlen(vlan_str) <= line_limit)
    {
        sal_printf("%s%s\n", prefix_str, vlan_str);
    }
    else
    {
        line = vlan_str;
        while (sal_strlen(line) > line_limit)
        {
            ptr = line + line_limit - 5; /* "xxxx," takes 5 chars */
            for (;;)
            {
                if (CMD_COMMA_CHAR == ptr[0])
                {
                    break;
                }
                ptr++;
            }
            ptr[0] = CMD_ZERO_CHAR;
            sal_printf("%s%s\n", p_prefix, line);
            if (first_line)
            {
                p_prefix = blank_line;
                first_line = FALSE;
            }
            line = ptr + 1;
        }

        if (sal_strlen(line))
        {
            sal_printf("%s%s\n", p_prefix, line);
        }
    }
    
    return PM_E_NONE;
}

static int32
_cdbclt_show_interface_untagged_vlan(char *prefix_str, char *vlan_str, uint32 width)
{
    char blank_line[CMD_BUF_256];
    uint32 prefix_width = sal_strlen(prefix_str);
    uint32 line_limit = width - prefix_width;
    char *line = NULL;
    char *ptr = NULL;
    char *p_prefix = prefix_str;
    uint32 first_line = TRUE;

    sal_memset(blank_line, ' ', prefix_width);
    blank_line[prefix_width] = CMD_ZERO_CHAR;

    if (sal_strlen(vlan_str) <= line_limit)
    {
        sal_printf("%s%s\n", prefix_str, vlan_str);
    }
    else
    {
        line = vlan_str;
        while (sal_strlen(line) > line_limit)
        {
            ptr = line + line_limit - 5; /* "xxxx," takes 5 chars */
            for (;;)
            {
                if (CMD_COMMA_CHAR == ptr[0])
                {
                    break;
                }
                ptr++;
            }
            ptr[0] = CMD_ZERO_CHAR;
            sal_printf("%s%s\n", p_prefix, line);
            if (first_line)
            {
                p_prefix = blank_line;
                first_line = FALSE;
            }
            line = ptr + 1;
        }

        if (sal_strlen(line))
        {
            sal_printf("%s%s\n", p_prefix, line);
        }
    }
    
    return PM_E_NONE;
}


static int32
_cdbctl_show_interface_switchport_one(char *line, uint32 show_one)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t ds_info;
    char *key = NULL;
    char *mode = NULL;
    char *vlan_type = NULL;
    char *qinq_mode = NULL;
    char *pvid = NULL;
    char *inner_pvid = NULL;
    uint32 pvid_num = 0;
    char *ingress_filter = NULL;
    char *allowed_vlan = NULL;
    char *untagged_vlan = NULL;
    char *vlantrans_en = NULL;
    char *vlanmap_table = NULL;
    char *p = NULL;
    uint32 i = 0;
    
    cfg_get_parse(line, &info, 0);

    key = info.value[TBL_INTERFACE_FLD_KEY];
    mode = info.value[TBL_INTERFACE_FLD_MODE];
    vlan_type = info.value[TBL_INTERFACE_FLD_VLAN_TYPE];
    
    if (sal_strcmp("switch", mode))
    {
        if (show_one)
        {
            sal_printf("%% Interface is not a switchport\n");
        }
        return PM_E_NONE;
    }
    
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_BRGIF], &ds_info, 1);
    
    pvid = ds_info.value[DS_BRGIF_FLD_PVID];
    inner_pvid = ds_info.value[DS_BRGIF_FLD_INNER_NATIVE_VID];
    if (NULL != pvid)
    {
        pvid_num = sal_atoi(pvid);
    }
    allowed_vlan = ds_info.value[DS_BRGIF_FLD_ALLOWED_VLAN];
    untagged_vlan = ds_info.value[DS_BRGIF_FLD_UNTAGGED_VLAN];
    ingress_filter = ds_info.value[DS_BRGIF_FLD_INGRESS_FILTER];
    vlantrans_en = ds_info.value[DS_BRGIF_FLD_VLAN_TRANSLATION_EN];
    qinq_mode = ds_info.value[DS_BRGIF_FLD_QINQ_TYPE];
    vlanmap_table = ds_info.value[DS_BRGIF_FLD_VLAN_MAPPING_TABLE];

    sal_printf(" Interface name          : %s\n", IFNAME_ETH2SHORT(key, ifname_ext));
    sal_printf(" Switchport mode         : %s", vlan_type);
    if (!sal_strcmp("dot1q-tunnel", vlan_type))
    {
        sal_printf("(%s)\n", qinq_mode);
    }
    else
    {
        sal_printf("\n");
    }
    if (!sal_strcmp("trunk", vlan_type) && enable_value(vlantrans_en))
    {
        sal_printf(" VLAN translation        : enable\n");
    }
    if (sal_strlen(vlanmap_table))
    {
        sal_printf(" VLAN mapping table      : %s\n", vlanmap_table);
    }
    
    sal_printf(" Ingress filter          : %s\n", enable_str_lower(enable_value(ingress_filter)));

    if (!sal_strcmp("access", vlan_type))
    {
        sal_printf(" Acceptable frame types  : vlan-untagged only\n");
    }
    else if (!sal_strcmp("trunk", vlan_type))
    {
        sal_printf(" Acceptable frame types  : all\n");
    }
    else if (!sal_strcmp("dot1q-tunnel", vlan_type))
    {
        sal_printf(" Acceptable frame types  : all\n");
    }

    if (pvid_num)
    {
        sal_printf(" Default Vlan            : %s\n", pvid);
    }
    else
    {
        sal_printf(" Default Vlan            : %s(inner)\n", inner_pvid);
    }

    p = allowed_vlan;
    for (i = 0; i < sal_strlen(allowed_vlan); i++)
    {
        if (sal_isdigit(p[0]))
        {
            break;
        }
        p++;
    }
    if (0 == sal_strlen(p))
    {
        sal_printf(" Configured Vlans        : N/A\n");
    }
    else
    {            
        _cdbclt_show_interface_allowed_vlan(" Configured Vlans        : ", p, MAX_CMD_LINE_LEN);
    }
    p = untagged_vlan;
    for (i = 0; i < sal_strlen(untagged_vlan); i++)
    {
        if (sal_isdigit(p[0]))
        {
            break;
        }
        p++;
    }
    if (!sal_strcmp("trunk", vlan_type))
    {
        if (0 == sal_strlen(p))
        {
            sal_printf(" Untagged Vlans          : N/A\n");
        }
        else
        {           
            _cdbclt_show_interface_untagged_vlan(" Untagged Vlans          : ", p, MAX_CMD_LINE_LEN);
        }
    }
    sal_printf("\n");
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_switchport(FILE *fp, cfg_parse_info_t *p_info, uint32 show_one)
{
    char  line[FILE_READ_LINE_32K_LEN];

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_switchport_one(line, show_one);
    }

    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_trunk_one(char *line, uint32 part)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t ds_info;
    char *key = NULL;
    char *mode = NULL;
    char *vlan_type = NULL;
    char *pvid = NULL;
    char *allowed_vlan = NULL;
    char *p = NULL;
    uint32 i = 0;
    char prefix_str[MAX_CMD_LINE_LEN];
    
    cfg_get_parse(line, &info, 0);

    key = info.value[TBL_INTERFACE_FLD_KEY];
    IFNAME_ETH2SHORT(key, ifname_ext);
    mode = info.value[TBL_INTERFACE_FLD_MODE];
    vlan_type = info.value[TBL_INTERFACE_FLD_VLAN_TYPE];
    
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_BRGIF], &ds_info, 1);
    
    pvid = ds_info.value[DS_BRGIF_FLD_PVID];
    allowed_vlan = ds_info.value[DS_BRGIF_FLD_ALLOWED_VLAN];

    if (sal_strcmp("switch", mode))
    {
        return PM_E_NONE;
    }

    if (sal_strcmp("trunk", vlan_type))
    {
        return PM_E_NONE;
    }

    if (part)
    {
        p = allowed_vlan;
        for (i = 0; i < sal_strlen(allowed_vlan); i++)
        {
            if (sal_isdigit(p[0]))
            {
                break;
            }
            p++;
        }
        
        sal_snprintf(prefix_str, MAX_CMD_LINE_LEN, "%-10s ", ifname_ext);
        _cdbclt_show_interface_allowed_vlan(prefix_str, p, MAX_CMD_LINE_LEN);
    }
    else
    {
        sal_printf("%-10s %-14s %-10s %-10s\n", ifname_ext, "802.1q", "trunking", pvid);
    }
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_trunk(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];


    sal_printf("\n%-10s %-14s %-10s %-10s\n", "Port", "Encapsulation", "Status", "Native VLAN");
    sal_printf("----------+--------------+----------+-----------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_trunk_one(line, 0);
    }

    sal_fseek(fp, 0, SEEK_SET);
    
    sal_printf("\n%-10s %-s\n", "Port", "Allowed VLANs on Trunk");
    sal_printf("----------+--------------------------------------------------------\n");
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_trunk_one(line, 1);
    }
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lag_one_get_member_ifname_and_state(char *line, char *p_buf, uint32 buf_len)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t lag_info;
    cfg_parse_get_info_t lacp_info;
    char *key = NULL;
    char *running = NULL;
    char* is_active = NULL;
    char *state = "";
    int32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_LAG], &lag_info, 1);
    if (0 == lag_info.argc)
    {
        /* Added by kcao for bug37975, interface maybe removed from LAG when call cdbclt_get_tbl_line()
         * just return
         */
        return PM_E_NONE;
    }
    key = info.value[TBL_INTERFACE_FLD_KEY];
    IFNAME_ETH2SHORT(key, ifname_ext);
    running = info.value[TBL_INTERFACE_FLD_RUNNING];
    is_active = info.value[TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE];

    if (0 == sal_strcmp("static", lag_info.value[DS_LAG_FLD_MODE]))
    {
        if ('0' == running[0])
        {
            state = "(D)";
        }
        else if (('1' == lag_info.value[DS_LAG_FLD_OPER_STATE][0]) && ('1' == is_active[0]))
        {
            state = "(B)";
        }
        else if ('1' == lag_info.value[DS_LAG_FLD_MISMATCH][0])
        {
            state = "(w)";
        }
        else
        {
            state = "(T)";
        }
    }
    else
    {
        cfg_get_parse(info.value[TBL_INTERFACE_FLD_LACP], &lacp_info, 1);

        if ('0' == running[0])
        {
            state = "(D)";
        }
        else if ('1' == lag_info.value[DS_LAG_FLD_OPER_STATE][0])
        {
            state = "(B)";
        }
        else if (0 == sal_strcmp(lacp_info.value[DS_LACP_FLD_SELECTED], "STANDBY"))
        {
            state = "(T)";
        }
        else if (0 == sal_strcmp(lacp_info.value[DS_LACP_FLD_MUX_STATE], "DETACHED"))
        {
            state = "(s)";
        }
        else
        {
            /* WAITING and ATTACHED */
            state = "(w)";
        }
    }

    sal_snprintf(p_buf, buf_len, "%s%s", ifname_ext, state);
    return rc;
}

static int32
_cdbctl_show_interface_lag_one_summary_do_show(char *name_and_state, char *load_balance_mode, char *lag_mode_str, char *member_port_list)
{
    cfg_parse_get_info_t parse_info;
    char *ifname = NULL;
    uint32 i = 0;
    uint32 str_len = 0;
    char  mem_name_and_state[MAX_CMD_LINE_LEN];
    char  line[FILE_READ_LINE_32K_LEN];
    int32 rc = PM_E_NONE;
    
    sal_printf("%-10s %-4s %-8s ", name_and_state, load_balance_mode, lag_mode_str);

    rc = cfg_get_parse_comma(member_port_list, &parse_info);
    if (rc < 0)
    {
        goto error;
    }

    for (i = 0; i < parse_info.argc; i++)
    {
        ifname = (char*)parse_info.argv[i];
        str_len = sal_strlen(ifname);
        rc = cdbclt_get_tbl_line(TBL_INTERFACE, ifname, str_len+1, line, FILE_READ_LINE_32K_LEN);
        if (rc < 0)
        {
            continue;
        }
        rc = _cdbctl_show_interface_lag_one_get_member_ifname_and_state(line, mem_name_and_state, MAX_CMD_LINE_LEN);
        if (rc < 0)
        {
            continue;
        }

        if ((3 == (i % 4)) && (i+1 != parse_info.argc))
        {
            sal_printf("%-13s", mem_name_and_state);
            /* new line if not last member */
            sal_printf("\n%-10s %-4s %-8s ", "", "", "");
        }
        else
        {
            sal_printf("%-14s", mem_name_and_state);
        }
    }

error:
    sal_printf("\n");

    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lag_one_summary(char *line)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t ds_info;
    char *key = NULL;
    char *switch_mode = NULL;
    char *running = NULL;
    char *hw_type = NULL;
    char *member_ports = NULL;
    char *lag_mode_str = NULL;
    uint32 str_len = 0;
    char name_and_state[20];
    char load_balance_mode[10];
    int32 mode = 0;
    int32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);

    key = info.value[TBL_INTERFACE_FLD_KEY];
    IFNAME_ETH2SHORT(key, ifname_ext);
    
    switch_mode = info.value[TBL_INTERFACE_FLD_MODE];
    running = info.value[TBL_INTERFACE_FLD_RUNNING];
    hw_type = info.value[TBL_INTERFACE_FLD_HW_TYPE];
    
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_LAG], &ds_info, 1);
    
    lag_mode_str = (0 == sal_strcmp("static", ds_info.value[DS_LAG_FLD_MODE])) ? 
        "Static" : "LACP";
    member_ports = ds_info.value[DS_LAG_FLD_MEMBER_PORTS];

    sal_memset(load_balance_mode, 0, sizeof(load_balance_mode));
    mode = sal_atoi(ds_info.value[DS_LAG_FLD_LOAD_BALANCE_MODE]);
    if (GLB_LAG_LOAD_BALANCE_DLB == mode)
    {
        sal_strcpy(load_balance_mode, "DLB");
    }
    else if (GLB_LAG_LOAD_BALANCE_RR == mode)
    {
        sal_strcpy(load_balance_mode, "RR");
    }
    else if (GLB_LAG_LOAD_BALANCE_MODE_STATIC == mode)
    {
        sal_strcpy(load_balance_mode, "SLB");
    }

    if (sal_strcmp("LAG", hw_type))
    {
        return PM_E_NONE;
    }

    sal_memset(name_and_state, 0, sizeof(name_and_state));
    sal_strcpy(name_and_state, ifname_ext);
    if (0 == sal_strcmp("switch", switch_mode))
    {
        sal_strcat(name_and_state, "(S");
    }
    else
    {
        sal_strcat(name_and_state, "(R");
    }

    if (0 == sal_strcmp("1", running))
    {
        sal_strcat(name_and_state, "U)");
    }
    else
    {
        sal_strcat(name_and_state, "D)");
    }

    str_len = sal_strlen(member_ports);
    if (str_len > 0 && member_ports[str_len-1] == CMD_DS_RIGHT_BRA_CHAR)
    {
        member_ports[str_len-1] = CMD_ZERO_CHAR;
    }
    str_len = sal_strlen(member_ports);
    if (str_len > 0 && member_ports[str_len-1] == CMD_COMMA_CHAR)
    {
        member_ports[str_len-1] = CMD_ZERO_CHAR;
    }
    
    rc = _cdbctl_show_interface_lag_one_summary_do_show(name_and_state, load_balance_mode, lag_mode_str, member_ports);
    return rc;
}

static const char*
_cdbctl_show_interface_lacp_get_port_state(uint32 is_running, uint8 actor_oper_port_state, uint8 selected_state)
{
    if (!is_running)
    {
        return "down";
    }
    else
    {
        if (GET_COLLECTING(actor_oper_port_state) && GET_DISTRIBUTING(actor_oper_port_state))
        {
            return "In-Bundle";
        }
        else if (SELECTED_STANDBY == selected_state)
        {
            return "standby";
        }
        else
        {
            return "Out-Bundle";
        }
    }

    return "Invalid";
}

static int32
_cdbctl_show_interface_lag_one_member(char *line, char *intend)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t lag_info;
    cfg_parse_get_info_t lacp_info;
    char *running = NULL;
    char *lag_mode_str = NULL;
    char *bundle = NULL;
    char *protocol = NULL;
    char *mode_str = NULL;
    char *partner_oper_system_priority = NULL;
    char *partner_oper_system = NULL;
    char *key = NULL;
    const char *port_state_str = NULL;
    char flags_str[MAX_CMD_LINE_LEN];
    char partner_system_id_str[MAX_CMD_LINE_LEN];
    uint8 actor_oper_port_state = 0;
    uint8 partner_oper_port_state = 0;
    uint8 selected_state = 0;
    uint16 actor_oper_port_key = 0;
    uint16 partner_oper_key = 0;
    uint32 lag_mode = 0;
    uint32 is_running = FALSE;
    int32 rc = PM_E_NONE;
    uint32 priority = 0;

    cfg_get_parse(line, &info, 0);
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_LAG], &lag_info, 1);

    if (0 == lag_info.argc)
    {
        /* not LAG member */
        return PM_E_NONE;
    }
    
    key = info.value[TBL_INTERFACE_FLD_KEY];
    IFNAME_ETH2SHORT(key, ifname_ext);
    if (0 == sal_strcmp("1", info.value[TBL_INTERFACE_FLD_RUNNING]))
    {
        running = "Up ";
        is_running = TRUE;
    }
    else
    {
        running = "Down ";
        is_running = FALSE;
    }

    priority = sal_strtou32(info.value[TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY], NULL, 10);
    if (0 == sal_strcmp("1", lag_info.value[DS_LAG_FLD_OPER_STATE]))
    {
        bundle = "In-Bundle";
        if (0 == sal_strcmp("0", info.value[TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE]))
        {
            bundle = "Standby";
        }
    }
    else
    {
        bundle = "Out-Bundle";
    }
    
    lag_mode_str = lag_info.value[DS_LAG_FLD_MODE];
    if (0 == sal_strcmp(lag_mode_str, "static"))
    {
        lag_mode = GLB_AGG_MODE_STATIC;
    }
    else if (0 == sal_strcmp(lag_mode_str, "active"))
    {
        lag_mode = GLB_AGG_MODE_DYNAMIC_ACTIVE;
        mode_str = "Active";
    }
    else
    {
        lag_mode = GLB_AGG_MODE_DYNAMIC_PASSIVE;
        mode_str = "Passive";
    }

    protocol = (GLB_AGG_MODE_STATIC == lag_mode) ? "static" : "LACP";
    
    sal_printf("%s%-14s: %s\n",               intend, "Port", ifname_ext);
    sal_printf("%s----------------------------\n",        intend);
    sal_printf(INTEND_2_STR"%s%-14s: %s%s\n", intend, "State", running, bundle);
    sal_printf(INTEND_2_STR"%s%-14s: %s\n",   intend, "Channel group", lag_info.value[DS_LAG_FLD_LAG_ID]);
    sal_printf(INTEND_2_STR"%s%-14s: %s\n",   intend, "Protocol", protocol);
    if (mode_str)
    {
        sal_printf(INTEND_2_STR"%s%-14s: %s\n",   intend, "Mode", mode_str);
    }
    sal_printf(INTEND_2_STR"%s%-14s: %s\n",   intend, "Port index", info.value[TBL_INTERFACE_FLD_IFINDEX]);

    if (GLB_AGG_MODE_STATIC == lag_mode)
    {
        sal_printf(INTEND_2_STR"%s%-14s: %u\n",   intend, "Priority", (priority >> 16));
    }

    if (GLB_AGG_MODE_STATIC != lag_mode)
    {
        cfg_get_parse(info.value[TBL_INTERFACE_FLD_LACP], &lacp_info, 1);

        if (0 != sal_strcmp("0", lacp_info.value[DS_LACP_FLD_MLAG_ID]))
        {
            sal_printf(INTEND_2_STR"%s%-14s: %s\n",   intend, "MLAG ID", lacp_info.value[DS_LACP_FLD_MLAG_ID]);
        }
        
        /* actor info */    
        actor_oper_port_state = cdb_int_str2val(lacp_info.value[DS_LACP_FLD_ACTOR_OPER_PORT_STATE], &rc);
        actor_oper_port_key = cdb_int_str2val(lacp_info.value[DS_LACP_FLD_ACTOR_OPER_PORT_KEY], &rc);
        selected_state = cdb_enum_str2val(lacp_selected_strs, SELECTED_STATE_MAX, lacp_info.value[DS_LACP_FLD_SELECTED]);
    
        flags_str[0] = CMD_ZERO_CHAR;
        if (GET_LACP_TIMEOUT(actor_oper_port_state))
        {
            sal_strncat(flags_str, "F", MAX_CMD_LINE_LEN);
        }
        else
        {
            sal_strncat(flags_str, "S", MAX_CMD_LINE_LEN);
        }
        if (GET_LACP_ACTIVITY(actor_oper_port_state))
        {
            sal_strncat(flags_str, "A", MAX_CMD_LINE_LEN);
        }
        else
        {
            sal_strncat(flags_str, "P", MAX_CMD_LINE_LEN);
        }

        port_state_str = _cdbctl_show_interface_lacp_get_port_state(is_running, actor_oper_port_state, selected_state);
        sal_printf(INTEND_2_STR"\n");
        sal_printf(INTEND_2_STR"%sActor's information:\n", intend);
        sal_printf(INTEND_2_STR"%s%-10s %-6s %-8s %-11s %-8s %-8s %-8s %-8s\n", intend,
            "", "", "Port", "", "Admin", "Oper", "Port", "Port");
        sal_printf(INTEND_2_STR"%s%-10s %-6s %-8s %-11s %-8s %-8s %-8s %-8s\n", intend,
            "Port", "Flags", "Priority", "State", "Key", "Key", "Number", "State");
        sal_printf(INTEND_2_STR"%s----------+------+--------+-----------+--------+--------+--------+--------\n", intend);

        sal_printf(INTEND_2_STR"%s%-10s %-6s %-8s %-11s %-8u %-8u %-8s 0x%-6x\n", intend,
            ifname_ext, 
            flags_str, 
            lacp_info.value[DS_LACP_FLD_ACTOR_PORT_PRIORITY],
            port_state_str, 
            actor_oper_port_key, 
            actor_oper_port_key,
            lacp_info.value[DS_LACP_FLD_ACTOR_PORT_NUMBER],
            actor_oper_port_state);

        /* partner info */
        partner_oper_port_state = cdb_int_str2val(lacp_info.value[DS_LACP_FLD_PARTNER_OPER_PORT_STATE], &rc);        
        partner_oper_key = cdb_int_str2val(lacp_info.value[DS_LACP_FLD_PARTNER_OPER_KEY], &rc);
        partner_oper_system_priority = lacp_info.value[DS_LACP_FLD_PARTNER_OPER_SYSTEM_PRIORITY];
        partner_oper_system = lacp_info.value[DS_LACP_FLD_PARTNER_OPER_SYSTEM];
        sal_snprintf(partner_system_id_str, MAX_CMD_LINE_LEN, "%s,%s", partner_oper_system_priority, partner_oper_system);
        
        flags_str[0] = CMD_ZERO_CHAR;
        if (GET_LACP_TIMEOUT(partner_oper_port_state))
        {
            sal_strncat(flags_str, "F", MAX_CMD_LINE_LEN);
        }
        else
        {
            sal_strncat(flags_str, "S", MAX_CMD_LINE_LEN);
        }
        if (GET_LACP_ACTIVITY(partner_oper_port_state))
        {
            sal_strncat(flags_str, "A", MAX_CMD_LINE_LEN);
        }
        else
        {
            sal_strncat(flags_str, "P", MAX_CMD_LINE_LEN);
        }
        
        sal_printf(INTEND_2_STR"\n");
        sal_printf(INTEND_2_STR"%sPartner's information:\n", intend);
        sal_printf(INTEND_2_STR"%s%-10s %-6s %-8s %-20s %-8s %-8s %-8s\n", intend,
            "", "", "Port", "", "Oper", "Port", "Port");
        sal_printf(INTEND_2_STR"%s%-10s %-6s %-8s %-20s %-8s %-8s %-8s\n", intend,
            "ActorPort", "Flags", "Priority", "System ID", "Key", "Number", "State");
        sal_printf(INTEND_2_STR"%s----------+------+--------+--------------------+--------+--------+--------\n", intend);

        sal_printf(INTEND_2_STR"%s%-10s %-6s %-8s %-20s %-8u %-8s 0x%-6x\n", intend,
            ifname_ext, 
            flags_str, 
            lacp_info.value[DS_LACP_FLD_PARTNER_OPER_PORT_PRIORITY],
            partner_system_id_str, 
            partner_oper_key,
            lacp_info.value[DS_LACP_FLD_PARTNER_OPER_PORT_NUMBER],
            partner_oper_port_state);
    }
    sal_printf(INTEND_2_STR"\n");
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lag_one_port_do_show(char *ifname)
{
    char  line[FILE_READ_LINE_32K_LEN];
    int32 rc = PM_E_NONE;

    rc = cdbclt_get_tbl_line(TBL_INTERFACE, ifname, sal_strlen(ifname)+1, line, FILE_READ_LINE_32K_LEN);
    if (rc < 0)
    {
        return PM_E_NONE;
    }

    return _cdbctl_show_interface_lag_one_member(line, INTEND_2_STR);
}

static int32
_cdbctl_show_interface_lag_one_detail(char *line)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t ds_info;
    cfg_parse_get_info_t member_info;
    char *switch_mode = NULL;
    char *hw_type = NULL;
    char *member_ports = NULL;
    char *lag_mode_str = NULL;
    char *protocol = NULL;
    char *member_ifname = NULL;
    uint32 i = 0;
    uint32 str_len = 0;
    uint32 lag_mode = 0;
    int32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);

    switch_mode = info.value[TBL_INTERFACE_FLD_MODE];
    hw_type = info.value[TBL_INTERFACE_FLD_HW_TYPE];
    
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_LAG], &ds_info, 1);
    
    lag_mode_str = ds_info.value[DS_LAG_FLD_MODE];
    member_ports = ds_info.value[DS_LAG_FLD_MEMBER_PORTS];

    if (sal_strcmp("LAG", hw_type))
    {
        return PM_E_NONE;
    }

    str_len = sal_strlen(member_ports);
    if (str_len > 0 && member_ports[str_len-1] == CMD_DS_RIGHT_BRA_CHAR)
    {
        member_ports[str_len-1] = CMD_ZERO_CHAR;
    }
    str_len = sal_strlen(member_ports);
    if (str_len > 0 && member_ports[str_len-1] == CMD_COMMA_CHAR)
    {
        member_ports[str_len-1] = CMD_ZERO_CHAR;
    }

    if (0 == sal_strcmp(lag_mode_str, "static"))
    {
        lag_mode = GLB_AGG_MODE_STATIC;
    }
    else if (0 == sal_strcmp(lag_mode_str, "active"))
    {
        lag_mode = GLB_AGG_MODE_DYNAMIC_ACTIVE;
    }
    else
    {
        lag_mode = GLB_AGG_MODE_DYNAMIC_PASSIVE;
    }

    protocol = (GLB_AGG_MODE_STATIC == lag_mode) ? "static" : "LACP";
        
    rc = cfg_get_parse_comma(member_ports, &member_info);
    if (rc < 0)
    {
        return PM_E_FAIL;
    }
    
    sal_printf(INTEND_2_STR"\n");
    sal_printf("Group: %s\n", ds_info.value[DS_LAG_FLD_LAG_ID]);
    sal_printf("----------------------------\n");
    sal_printf(INTEND_2_STR"%-14s: %s\n", "Mode", switch_mode);
    sal_printf(INTEND_2_STR"%-14s: %u\n", "Ports", member_info.argc);
    sal_printf(INTEND_2_STR"%-14s: %u\n", "Maxports", GLB_LINKAGG_MAX_MEMBER);
    sal_printf(INTEND_2_STR"%-14s: %s\n", "Bundle Ports", ds_info.value[DS_LAG_FLD_BUNDLE_PORTS_COUNT]);
    sal_printf(INTEND_2_STR"%-14s: %s\n", "Protocol", protocol);
    sal_printf(INTEND_2_STR"%-14s: %s/%s\n", "Active/Maxactive", ds_info.value[DS_LAG_FLD_CUR_ACTIVE_NUMBER], ds_info.value[DS_LAG_FLD_MAX_ACTIVE_NUMBER]);
    sal_printf(INTEND_2_STR"\n");
    
    for (i = 0; i < member_info.argc; i++)
    {
        member_ifname = (char*)member_info.argv[i];
        _cdbctl_show_interface_lag_one_port_do_show(member_ifname);
    }

    return rc;
}

static int32
_cdbctl_show_interface_lag_one_port(char *line)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t ds_info;
    cfg_parse_get_info_t member_info;
    char *hw_type = NULL;
    char *member_ports = NULL;
    char *member_ifname = NULL;
    uint32 i = 0;
    uint32 str_len = 0;
    int32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);

    hw_type = info.value[TBL_INTERFACE_FLD_HW_TYPE];
    
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_LAG], &ds_info, 1);
    member_ports = ds_info.value[DS_LAG_FLD_MEMBER_PORTS];

    if (sal_strcmp("LAG", hw_type))
    {
        return PM_E_NONE;
    }

    str_len = sal_strlen(member_ports);
    if (str_len > 0 && member_ports[str_len-1] == CMD_DS_RIGHT_BRA_CHAR)
    {
        member_ports[str_len-1] = CMD_ZERO_CHAR;
    }
    str_len = sal_strlen(member_ports);
    if (str_len > 0 && member_ports[str_len-1] == CMD_COMMA_CHAR)
    {
        member_ports[str_len-1] = CMD_ZERO_CHAR;
    }
        
    rc = cfg_get_parse_comma(member_ports, &member_info);
    if (rc < 0)
    {
        return PM_E_FAIL;
    }
    
    sal_printf(INTEND_2_STR"\n");
    sal_printf("Group: %s\n", ds_info.value[DS_LAG_FLD_LAG_ID]);
    sal_printf("----------------------------\n");

    for (i = 0; i < member_info.argc; i++)
    {
        member_ifname = (char*)member_info.argv[i];
        _cdbctl_show_interface_lag_one_port_do_show(member_ifname);
    }

    return rc;
}

static int32
_cdbctl_show_interface_lag_summary(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];
    cfg_parse_get_info_t info;
    uint32 load_balance = 0;
	uint32 tunnel_hash_mode = 0;
    uint8 hash_arithmetic=GLB_LAG_HASH_ALGO_XOR;
    char load_balance_str[CMD_BUF_256];

    load_balance_str[0] = CMD_ZERO_CHAR;
    cdbclt_get_tbl_line(TBL_LAG_GLOBAL, NULL, 0, line, FILE_READ_LINE_LEN);
    cfg_get_parse(line, &info, 0);
    
    if (info.value[TBL_LAG_GLOBAL_FLD_HASH_ARITHMETIC])
    {
        hash_arithmetic=sal_atoi(info.value[TBL_LAG_GLOBAL_FLD_HASH_ARITHMETIC]);
        if (GLB_LAG_HASH_ALGO_XOR == hash_arithmetic)
        {
            sal_printf("port-channel load-balance hash-arithmetic: xor\n");
        }
        else if (GLB_LAG_HASH_ALGO_CRC == hash_arithmetic)
        {
            sal_printf("port-channel load-balance hash-arithmetic: crc\n");
        }
     }

	if (info.value[TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_INNER_FIELD_EN])
    {
        tunnel_hash_mode = sal_atoi(info.value[TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_INNER_FIELD_EN]);
        switch(tunnel_hash_mode)
        {
		case GLB_LAG_TUNNEL_HASH_MODE_OUTER:
            sal_printf("port-channel load-balance tunnel-hash-mode: outer\n");
			break;
        case GLB_LAG_TUNNEL_HASH_MODE_BOTH:
            sal_printf("port-channel load-balance tunnel-hash-mode: both\n");
			break;
		case GLB_LAG_TUNNEL_HASH_MODE_INNER:
            sal_printf("port-channel load-balance tunnel-hash-mode: inner\n");
			break;
    	}
     }

    if (info.value[TBL_LAG_GLOBAL_FLD_LOAD_BALANCE])
    {
        load_balance = sal_atoi(info.value[TBL_LAG_GLOBAL_FLD_LOAD_BALANCE]);

            
        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_SRC_MAC))
        {
            sal_strcat(load_balance_str, "src-mac ");
        }
        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_DST_MAC))
        {
            sal_strcat(load_balance_str, "dst-mac ");
        }
        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_SRC_IP))
        {
            sal_strcat(load_balance_str, "src-ip ");
        }
        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_DST_IP))
        {
            sal_strcat(load_balance_str, "dst-ip ");
        }
        if(GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_IP_PROTOCOL))
        {
            sal_strcat(load_balance_str, "ip-protocol ");
        }
        if(GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_SRC_PORT))
        {
            sal_strcat(load_balance_str, "src-port-l4 ");
        }
        if(GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_DST_PORT))
        {
            sal_strcat(load_balance_str, "dst-port-l4 ");
        }
        if(GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_VXLAN_VNI))
        {
            sal_strcat(load_balance_str, "vxlan-vni ");
        }
        if(GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_GRE_KEY))
        {
            sal_strcat(load_balance_str, "gre-key ");
        }
        if(GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_NVGRE_VNI))
        {
            sal_strcat(load_balance_str, "nvgre-vsid ");
        }
        if(GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_NVGRE_FLOW_ID))
        {
            sal_strcat(load_balance_str, "nvgre-flow-id ");
        }
        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_INNER_SRC_MAC))
        {
            sal_strcat(load_balance_str, "inner-src-mac ");
        }
        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_INNER_DST_MAC))
        {
            sal_strcat(load_balance_str, "inner-dst-mac ");
        }
        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_INNER_SRC_IP))
        {
            sal_strcat(load_balance_str, "inner-src-ip ");
        }
        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_INNER_DST_IP))
        {
            sal_strcat(load_balance_str, "inner-dst-ip ");
        }
        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_INNER_IP_PROTOCOL))
        {
            sal_strcat(load_balance_str, "inner-ip-protocol ");
        }

        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_INNER_SRC_PORT_L4))
        {
            sal_strcat(load_balance_str, "inner-src-port-l4 ");
        }

        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_INNER_DST_PORT_L4))
        {
            sal_strcat(load_balance_str, "inner-dst-port-l4 ");
        }
        sal_printf("Port-channel load-balance hash-field-select: \n");
        sal_printf("    %s\n", load_balance_str);
    }
        
    sal_printf("Flags:  s - suspend           T - standby\n");
    sal_printf("        w - wait              B - in Bundle\n");
    sal_printf("        R - Layer3            S - Layer2\n");
    sal_printf("        D - down/admin down   U - in use\n\n");

    sal_printf("Mode:  SLB   - static load balance\n");
    sal_printf("       DLB   - dynamic load balance\n");
    sal_printf("       RR    - round robin load balance\n\n");

    sal_printf("%-10s %-4s %-8s %s\n", "Aggregator", "Mode", "Protocol", "Ports");
    sal_printf("----------+----+--------+----------------------------------------------------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_lag_one_summary(line);
    }
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lag_detail(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];

    sal_printf("Flags:  S - Device is requesting Slow LACPDUs\n");
    sal_printf("        F - Device is requesting Fast LACPDUs\n");
    sal_printf("        A - Device is in Active mode\n");
    sal_printf("        P - Device is in Passive mode\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_lag_one_detail(line);
    }
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lag_port(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_lag_one_port(line);
    }
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lag(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    if (sal_strstr(para, "summary"))
    {
        _cdbctl_show_interface_lag_summary(fp, para, p_info);
    }
    else if (sal_strstr(para, "detail"))
    {
        _cdbctl_show_interface_lag_detail(fp, para, p_info);
    }
    else if (sal_strstr(para, "port"))
    {
        _cdbctl_show_interface_lag_port(fp, para, p_info);
    }
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lag_member(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_lag_one_member(line, INTEND_0_STR);
    }

    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lacp_counters_one(char *line, char *pre_lag_id, uint32 len)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t lag_info;
    cfg_parse_get_info_t lacp_info;
    char *key = NULL;
    char *lag_id = NULL;
    char *tx_lacp_count = NULL;
    char *rx_lacp_count = NULL;
    char *tx_error_count = NULL;
    char *rx_error_count = NULL;

    cfg_get_parse(line, &info, 0);
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_LAG], &lag_info, 1);
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_LACP], &lacp_info, 1);

    /* double check */
    if (0 == sal_strcmp(lag_info.value[DS_LAG_FLD_MODE], "static"))
    {
        return PM_E_NONE;
    }

    key = info.value[TBL_INTERFACE_FLD_KEY];
    IFNAME_ETH2SHORT(key, ifname_ext);
    lag_id = lag_info.value[DS_LAG_FLD_LAG_ID];
    tx_lacp_count = lacp_info.value[DS_LACP_FLD_TX_LACP_COUNT];
    rx_lacp_count = lacp_info.value[DS_LACP_FLD_RX_LACP_COUNT];
    tx_error_count = lacp_info.value[DS_LACP_FLD_TX_ERROR_COUNT];
    rx_error_count = lacp_info.value[DS_LACP_FLD_RX_ERROR_COUNT];

    if (sal_strcmp(pre_lag_id, lag_id))
    {
        sal_printf("%s "GLB_IFNAME_AGG_PREFIX"%s:\n", "Channel-group", lag_id);
        sal_strncpy(pre_lag_id, lag_id, len);
    }
    
    sal_printf("%-20s %-10s %-10s %-10s %-10s\n", ifname_ext, 
        tx_lacp_count, rx_lacp_count, tx_error_count, rx_error_count);
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lacp_counters(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    char  pre_lag_id[IFNAME_SIZE];
    uint32 banner = TRUE;

    sal_memset(pre_lag_id, 0, sizeof(pre_lag_id));

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        if (banner)
        {
            sal_printf("%-20s %-21s %-21s\n", "", "    LACPDU", "    Packet Error");
            sal_printf("%-20s %-10s %-10s %-10s %-10s\n", "Port", "Send", "Recv", "Send", "Recv");
            sal_printf("--------------------+----------+----------+----------+----------\n");
            banner = FALSE;
        }
        _cdbctl_show_interface_lacp_counters_one(line, pre_lag_id, sizeof(pre_lag_id));
    }
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lacp_internal_one(char *line, char *pre_lag_id, uint32 len)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t lag_info;
    cfg_parse_get_info_t lacp_info;
    char *key = NULL;
    char *running = NULL;
    char *lag_id = NULL;
    char *actor_port_priority = NULL;
    char *actor_port_number = NULL;
    const char *port_state_str = NULL;
    uint16 actor_oper_port_key = 0;
    uint8 actor_oper_port_state = 0;
    uint8 selected_state = 0;
    uint32 is_running = FALSE;
    char flags_str[MAX_CMD_LINE_LEN];
    int32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_LAG], &lag_info, 1);
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_LACP], &lacp_info, 1);

    /* double check */
    if (0 == sal_strcmp(lag_info.value[DS_LAG_FLD_MODE], "static"))
    {
        return PM_E_NONE;
    }
        
    key = info.value[TBL_INTERFACE_FLD_KEY];
    IFNAME_ETH2SHORT(key, ifname_ext);
    lag_id = lag_info.value[DS_LAG_FLD_LAG_ID];
    
    actor_oper_port_key = cdb_int_str2val(lacp_info.value[DS_LACP_FLD_ACTOR_OPER_PORT_KEY], &rc);
    actor_port_priority = lacp_info.value[DS_LACP_FLD_ACTOR_PORT_PRIORITY];
    actor_port_number = lacp_info.value[DS_LACP_FLD_ACTOR_PORT_NUMBER];
    selected_state = cdb_enum_str2val(lacp_selected_strs, SELECTED_STATE_MAX, lacp_info.value[DS_LACP_FLD_SELECTED]);
    actor_oper_port_state = cdb_int_str2val(lacp_info.value[DS_LACP_FLD_ACTOR_OPER_PORT_STATE], &rc);

    flags_str[0] = CMD_ZERO_CHAR;
    if (GET_LACP_TIMEOUT(actor_oper_port_state))
    {
        sal_strncat(flags_str, "F", MAX_CMD_LINE_LEN);
    }
    else
    {
        sal_strncat(flags_str, "S", MAX_CMD_LINE_LEN);
    }
    if (GET_LACP_ACTIVITY(actor_oper_port_state))
    {
        sal_strncat(flags_str, "A", MAX_CMD_LINE_LEN);
    }
    else
    {
        sal_strncat(flags_str, "P", MAX_CMD_LINE_LEN);
    }

    running = info.value[TBL_INTERFACE_FLD_RUNNING];
    
    if (running[0] == '1')
    {
        is_running = TRUE;
    }

    port_state_str = _cdbctl_show_interface_lacp_get_port_state(is_running, actor_oper_port_state, selected_state);
    
    if (sal_strcmp(pre_lag_id, lag_id))
    {
        sal_printf("\n%s "GLB_IFNAME_AGG_PREFIX"%s:\n", "Channel-group", lag_id);
        sal_printf("Actor's information:\n");
        sal_printf("%-8s %-7s %-20s %-10s %-6s %-6s %-6s %-8s\n",
            "", "", "", "LACP port", "Admin", "Oper", "Port", "Port");
        sal_printf("%-8s %-7s %-20s %-10s %-6s %-6s %-6s %-8s\n",
            "Port", "Flags", "State", "Priority", "Key", "Key", "Number", "State");
        sal_printf("--------+-------+--------------------+----------+------+------+------+--------\n");
        sal_strncpy(pre_lag_id, lag_id, len);
    }

    sal_printf("%-8s %-7s %-20s %-10s %-6u %-6u %-6s 0x%-6x\n",
                ifname_ext, flags_str, port_state_str, actor_port_priority, actor_oper_port_key, actor_oper_port_key, 
                actor_port_number, actor_oper_port_state);
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lacp_internal_one_detail(char *line, char *pre_lag_id, uint32 len, tbl_lag_global_t *p_lag_glb)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t lag_info;
    cfg_parse_get_info_t lacp_info;
    char *key = NULL;
    char *lag_id = NULL;
    char *actor_port_priority = NULL;
    char *actor_port_number = NULL;
    uint16 actor_oper_port_key = 0;
    uint8 actor_oper_port_state = 0;
    char flags_str[MAX_CMD_LINE_LEN];
    char system_id_str[MAX_CMD_LINE_LEN];
    uint32 str_len = 0;
    int32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_LAG], &lag_info, 1);
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_LACP], &lacp_info, 1);

    /* double check */
    if (0 == sal_strcmp(lag_info.value[DS_LAG_FLD_MODE], "static"))
    {
        return PM_E_NONE;
    }
        
    key = info.value[TBL_INTERFACE_FLD_KEY];
    IFNAME_ETH2SHORT(key, ifname_ext);
    lag_id = lag_info.value[DS_LAG_FLD_LAG_ID];
    
    actor_oper_port_key = cdb_int_str2val(lacp_info.value[DS_LACP_FLD_ACTOR_OPER_PORT_KEY], &rc);
    actor_port_priority = lacp_info.value[DS_LACP_FLD_ACTOR_PORT_PRIORITY];
    actor_port_number = lacp_info.value[DS_LACP_FLD_ACTOR_PORT_NUMBER];
    actor_oper_port_state = cdb_int_str2val(lacp_info.value[DS_LACP_FLD_ACTOR_OPER_PORT_STATE], &rc);

    if (0 == sal_strcmp("0", lacp_info.value[DS_LACP_FLD_MLAG_ID]))
    {
        sal_snprintf(system_id_str, MAX_CMD_LINE_LEN, "%u,", p_lag_glb->system_priority);
        str_len = sal_strlen(system_id_str);
        cdb_mac_addr_val2str(&system_id_str[str_len], MAX_CMD_LINE_LEN-str_len, p_lag_glb->system_id);
    }
    else
    {
        sal_snprintf(system_id_str, MAX_CMD_LINE_LEN, "%u,", p_lag_glb->mlag_system_priority);
        str_len = sal_strlen(system_id_str);
        cdb_mac_addr_val2str(&system_id_str[str_len], MAX_CMD_LINE_LEN-str_len, p_lag_glb->mlag_system_id);
    }

    flags_str[0] = CMD_ZERO_CHAR;
    if (GET_LACP_TIMEOUT(actor_oper_port_state))
    {
        sal_strncat(flags_str, "F", MAX_CMD_LINE_LEN);
    }
    else
    {
        sal_strncat(flags_str, "S", MAX_CMD_LINE_LEN);
    }
    if (GET_LACP_ACTIVITY(actor_oper_port_state))
    {
        sal_strncat(flags_str, "A", MAX_CMD_LINE_LEN);
    }
    else
    {
        sal_strncat(flags_str, "P", MAX_CMD_LINE_LEN);
    }

    if (sal_strcmp(pre_lag_id, lag_id))
    {
        sal_printf("\n%s "GLB_IFNAME_AGG_PREFIX"%s:\n", "Channel-group", lag_id);
        sal_printf("Actor's information:\n");
        sal_strncpy(pre_lag_id, lag_id, len);
    }

    sal_printf("%-10s %-20s %-11s %-10s\n",
        "Port", "System ID", "Port Number", "Flags");
    sal_printf("----------+--------------------+-----------+----------\n");
    sal_printf("%-10s %-20s %-11s %-6s\n", ifname_ext, system_id_str, actor_port_number, flags_str);
    sal_printf("\n");
    sal_printf("%-10s %-20s %-11s %-10s\n",
        "", "Port Priority", "Oper Key", "Port State");
    sal_printf("          +--------------------+-----------+----------\n");
    sal_printf("%-10s %-20s %-11u 0x%-8x\n", "", actor_port_priority, actor_oper_port_key, actor_oper_port_state);
    sal_printf("\n");
    sal_printf("          Port State Flags Decode:\n");
    sal_printf("%-10s %-12s %-14s %-14s %-16s\n",
        "", "Activity:", "Timeout:", "Aggregation:", "Synchronization:");
    sal_printf("          +------------+--------------+--------------+---------------\n");
    sal_printf("%-10s %-12s %-14s %-14s %-16s\n", "", 
        GET_LACP_ACTIVITY(actor_oper_port_state) ? "Active" : "Passive",
        GET_LACP_TIMEOUT(actor_oper_port_state) ? "Short" : "Long",
        yes_no_str(GET_AGGREGATION(actor_oper_port_state)),
        yes_no_str(GET_SYNCHRONIZATION(actor_oper_port_state)));
    sal_printf("\n");
    sal_printf("%-10s %-12s %-14s %-14s %-16s\n",
        "", "Collecting:", "Distributing:", "Defaulted:", "Expired:");
    sal_printf("          +------------+--------------+--------------+---------------\n");
    sal_printf("%-10s %-12s %-14s %-14s %-16s\n", "", 
        yes_no_str(GET_COLLECTING(actor_oper_port_state)),
        yes_no_str(GET_DISTRIBUTING(actor_oper_port_state)),
        yes_no_str(GET_DEFAULTED(actor_oper_port_state)),
        yes_no_str(GET_EXPIRED(actor_oper_port_state)));
    sal_printf("\n");
    sal_printf("          Port State Decode:\n");
    sal_printf("%-10s %-12s %-14s %-14s %-16s\n",
        "", "Selected:", "PeriodicTX:", "Receive:", "Mux:");
    sal_printf("          +------------+--------------+--------------+---------------\n");
    sal_printf("%-10s %-12s %-14s %-14s %-16s\n", "",
        lacp_info.value[DS_LACP_FLD_SELECTED],
        lacp_info.value[DS_LACP_FLD_PERIODIC_TX_STATE],
        lacp_info.value[DS_LACP_FLD_RECEIVE_STATE],
        lacp_info.value[DS_LACP_FLD_MUX_STATE]);
    sal_printf("\n");
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lacp_internal(FILE *fp, char *para, cfg_parse_info_t *p_info, uint32 detail)
{
    char  line[FILE_READ_LINE_LEN];
    char  pre_lag_id[IFNAME_SIZE];
    tbl_lag_global_t lag_glb;
    int32 rc = PM_E_NONE;

    sal_memset(pre_lag_id, 0, sizeof(pre_lag_id));

    sal_printf("Flags:  S - Device is requesting Slow LACPDUs\n");
    sal_printf("        F - Device is requesting Fast LACPDUs\n");
    sal_printf("        A - Device is in Active mode\n");
    sal_printf("        P - Device is in Passive mode\n");

    if (detail)
    {
        sal_memset(&lag_glb, 0, sizeof(lag_glb));
        rc = cdbclt_get_tbl_data(TBL_LAG_GLOBAL, NULL, 0, &lag_glb, sizeof(lag_glb));
        if (rc < 0)
        {
            sal_printf("%% Get System ID failed\n");
        }
    }
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        if (detail)
        {
            _cdbctl_show_interface_lacp_internal_one_detail(line, pre_lag_id, sizeof(pre_lag_id), &lag_glb);
        }
        else
        {
            _cdbctl_show_interface_lacp_internal_one(line, pre_lag_id, sizeof(pre_lag_id));
        }
    }
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lacp_neighbor_one(char *line, char *pre_lag_id, uint32 len)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t lag_info;
    cfg_parse_get_info_t lacp_info;
    char *key = NULL;
    char *lag_id = NULL;
    char *partner_oper_port_priority = NULL;
    char *partner_oper_system_priority = NULL;
    char *partner_oper_system = NULL;
    char *partner_oper_port_number = NULL;
    uint16 partner_admin_key = 0;
    uint16 partner_oper_key = 0;
    uint8 partner_oper_port_state = 0;
    char flags_str[MAX_CMD_LINE_LEN];
    char system_id_str[MAX_CMD_LINE_LEN];
    int32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_LAG], &lag_info, 1);
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_LACP], &lacp_info, 1);

    /* double check */
    if (0 == sal_strcmp(lag_info.value[DS_LAG_FLD_MODE], "static"))
    {
        return PM_E_NONE;
    }
        
    key = info.value[TBL_INTERFACE_FLD_KEY];
    IFNAME_ETH2SHORT(key, ifname_ext);
    lag_id = lag_info.value[DS_LAG_FLD_LAG_ID];
    
    partner_admin_key = cdb_int_str2val(lacp_info.value[DS_LACP_FLD_PARTNER_ADMIN_KEY], &rc);
    partner_oper_key = cdb_int_str2val(lacp_info.value[DS_LACP_FLD_PARTNER_OPER_KEY], &rc);
    partner_oper_port_priority = lacp_info.value[DS_LACP_FLD_PARTNER_OPER_PORT_PRIORITY];
    partner_oper_system_priority = lacp_info.value[DS_LACP_FLD_PARTNER_OPER_SYSTEM_PRIORITY];
    partner_oper_system = lacp_info.value[DS_LACP_FLD_PARTNER_OPER_SYSTEM];
    partner_oper_port_number = lacp_info.value[DS_LACP_FLD_PARTNER_OPER_PORT_NUMBER];
    partner_oper_port_state = cdb_int_str2val(lacp_info.value[DS_LACP_FLD_PARTNER_OPER_PORT_STATE], &rc);

    flags_str[0] = CMD_ZERO_CHAR;
    if (GET_LACP_TIMEOUT(partner_oper_port_state))
    {
        sal_strncat(flags_str, "F", MAX_CMD_LINE_LEN);
    }
    else
    {
        sal_strncat(flags_str, "S", MAX_CMD_LINE_LEN);
    }
    if (GET_LACP_ACTIVITY(partner_oper_port_state))
    {
        sal_strncat(flags_str, "A", MAX_CMD_LINE_LEN);
    }
    else
    {
        sal_strncat(flags_str, "P", MAX_CMD_LINE_LEN);
    }

    sal_snprintf(system_id_str, MAX_CMD_LINE_LEN, "%s,%s", partner_oper_system_priority, partner_oper_system);

    if (sal_strcmp(pre_lag_id, lag_id))
    {
        sal_printf("\n%s "GLB_IFNAME_AGG_PREFIX"%s:\n", "Channel-group", lag_id);
        sal_printf("Partner's information:\n");
        sal_printf("%-8s %-7s %-20s %-10s %-6s %-6s %-6s %-8s\n",
            "", "", "", "LACP port", "Admin", "Oper", "Port", "Port");
        sal_printf("%-8s %-7s %-20s %-10s %-6s %-6s %-6s %-8s\n",
            "Port", "Flags", "SystemID", "Priority", "Key", "Key", "Number", "State");
        sal_printf("--------+-------+--------------------+----------+------+------+------+--------\n");
        sal_strncpy(pre_lag_id, lag_id, len);
    }

    sal_printf("%-8s %-7s %-20s %-10s %-6u %-6u %-6s 0x%-6x\n",
                ifname_ext, flags_str, system_id_str, partner_oper_port_priority, partner_admin_key, partner_oper_key, 
                partner_oper_port_number, partner_oper_port_state);
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lacp_neighbor_one_detail(char *line, char *pre_lag_id, uint32 len)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t lag_info;
    cfg_parse_get_info_t lacp_info;
    char *key = NULL;
    char *lag_id = NULL;
    char *partner_oper_port_priority = NULL;
    char *partner_oper_system_priority = NULL;
    char *partner_oper_system = NULL;
    char *partner_oper_port_number = NULL;
    uint16 partner_oper_key = 0;
    uint8 partner_oper_port_state = 0;
    char flags_str[MAX_CMD_LINE_LEN];
    char system_id_str[MAX_CMD_LINE_LEN];
    int32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_LAG], &lag_info, 1);
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_LACP], &lacp_info, 1);

    /* double check */
    if (0 == sal_strcmp(lag_info.value[DS_LAG_FLD_MODE], "static"))
    {
        return PM_E_NONE;
    }
        
    key = info.value[TBL_INTERFACE_FLD_KEY];
    IFNAME_ETH2SHORT(key, ifname_ext);
    lag_id = lag_info.value[DS_LAG_FLD_LAG_ID];
    
    partner_oper_key = cdb_int_str2val(lacp_info.value[DS_LACP_FLD_PARTNER_OPER_KEY], &rc);
    partner_oper_port_priority = lacp_info.value[DS_LACP_FLD_PARTNER_OPER_PORT_PRIORITY];
    partner_oper_system_priority = lacp_info.value[DS_LACP_FLD_PARTNER_OPER_SYSTEM_PRIORITY];
    partner_oper_system = lacp_info.value[DS_LACP_FLD_PARTNER_OPER_SYSTEM];
    partner_oper_port_number = lacp_info.value[DS_LACP_FLD_PARTNER_OPER_PORT_NUMBER];
    partner_oper_port_state = cdb_int_str2val(lacp_info.value[DS_LACP_FLD_PARTNER_OPER_PORT_STATE], &rc);

    sal_snprintf(system_id_str, MAX_CMD_LINE_LEN, "%s,%s", partner_oper_system_priority, partner_oper_system);

    flags_str[0] = CMD_ZERO_CHAR;
    if (GET_LACP_TIMEOUT(partner_oper_port_state))
    {
        sal_strncat(flags_str, "F", MAX_CMD_LINE_LEN);
    }
    else
    {
        sal_strncat(flags_str, "S", MAX_CMD_LINE_LEN);
    }
    if (GET_LACP_ACTIVITY(partner_oper_port_state))
    {
        sal_strncat(flags_str, "A", MAX_CMD_LINE_LEN);
    }
    else
    {
        sal_strncat(flags_str, "P", MAX_CMD_LINE_LEN);
    }

    if (sal_strcmp(pre_lag_id, lag_id))
    {
        sal_printf("\n%s "GLB_IFNAME_AGG_PREFIX"%s:\n", "Channel-group", lag_id);
        sal_printf("Partner's information:\n");
        sal_strncpy(pre_lag_id, lag_id, len);
    }

    sal_printf("%-10s %-20s %-11s %-10s\n",
        "Port", "System ID", "Port Number", "Flags");
    sal_printf("----------+--------------------+-----------+----------\n");
    sal_printf("%-10s %-20s %-11s %-6s\n", ifname_ext, system_id_str, partner_oper_port_number, flags_str);
    sal_printf("\n");
    sal_printf("%-10s %-20s %-11s %-10s\n",
        "", "Port Priority", "Oper Key", "Port State");
    sal_printf("          +--------------------+-----------+----------\n");
    sal_printf("%-10s %-20s %-11u 0x%-8x\n", "", partner_oper_port_priority, partner_oper_key, partner_oper_port_state);
    sal_printf("\n");
    sal_printf("          Port State Flags Decode:\n");
    sal_printf("%-10s %-12s %-14s %-14s %-16s\n",
        "", "Activity:", "Timeout:", "Aggregation:", "Synchronization:");
    sal_printf("          +------------+--------------+--------------+---------------\n");
    sal_printf("%-10s %-12s %-14s %-14s %-16s\n", "", 
        GET_LACP_ACTIVITY(partner_oper_port_state) ? "Active" : "Passive",
        GET_LACP_TIMEOUT(partner_oper_port_state) ? "Short" : "Long",
        yes_no_str(GET_AGGREGATION(partner_oper_port_state)),
        yes_no_str(GET_SYNCHRONIZATION(partner_oper_port_state)));
    sal_printf("\n");
    sal_printf("%-10s %-12s %-14s %-14s %-16s\n",
        "", "Collecting:", "Distributing:", "Defaulted:", "Expired:");
    sal_printf("          +------------+--------------+--------------+---------------\n");
    sal_printf("%-10s %-12s %-14s %-14s %-16s\n", "", 
        yes_no_str(GET_COLLECTING(partner_oper_port_state)),
        yes_no_str(GET_DISTRIBUTING(partner_oper_port_state)),
        yes_no_str(GET_DEFAULTED(partner_oper_port_state)),
        yes_no_str(GET_EXPIRED(partner_oper_port_state)));
    sal_printf("\n");

    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lacp_neighbor(FILE *fp, char *para, cfg_parse_info_t *p_info, uint32 detail)
{
    char  line[FILE_READ_LINE_LEN];
    char  pre_lag_id[IFNAME_SIZE];

    sal_memset(pre_lag_id, 0, sizeof(pre_lag_id));
    sal_printf("Flags:  S - Device is requesting Slow LACPDUs\n");
    sal_printf("        F - Device is requesting Fast LACPDUs\n");
    sal_printf("        A - Device is in Active mode\n");
    sal_printf("        P - Device is in Passive mode\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        if (detail)
        {
            _cdbctl_show_interface_lacp_neighbor_one_detail(line, pre_lag_id, sizeof(pre_lag_id));
        }
        else
        {
            _cdbctl_show_interface_lacp_neighbor_one(line, pre_lag_id, sizeof(pre_lag_id));
        }
    }
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_lacp(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    uint32 detail = FALSE;

    if (sal_strstr(para, "detail"))
    {
        detail = TRUE;
    }

    if (sal_strstr(para, "counters"))
    {
        _cdbctl_show_interface_lacp_counters(fp, para, p_info);
    }
    else if (sal_strstr(para, "internal"))
    {
        _cdbctl_show_interface_lacp_internal(fp, para, p_info, detail);
    }
    else if (sal_strstr(para, "neighbor"))
    {
        _cdbctl_show_interface_lacp_neighbor(fp, para, p_info, detail);
    }
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_port_isolate_ports(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    uint32 isolate_ifindex_array[GLB_PORT_ISOLATE_GROUP_MAX+1][256];
    uint32 isolate_ifindex_count[GLB_PORT_ISOLATE_GROUP_MAX+1];
    char ifname[IFNAME_SIZE];
    char  line[FILE_READ_LINE_32K_LEN];
    uint32 isolate_group_id = 0;
    uint32 ifindex = 0;
    uint32 i = 0;
    uint32 j = 0;
    uint32 member_count = 0;
    cfg_parse_get_info_t info;
    cfg_parse_info_t parse_info;
    uint32 banner = TRUE;
    uint32 group_id = 0;
    int32 rc = PM_E_NONE;

    sal_memset(isolate_ifindex_array, 0, sizeof(isolate_ifindex_array));
    sal_memset(isolate_ifindex_count, 0, sizeof(isolate_ifindex_count));
    
    cfg_cmd_parse_para(para, &parse_info);

    if (parse_info.argc > 1)
    {
        group_id = cdb_int_str2val(parse_info.argv[1], &rc);
    }    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        isolate_group_id = 0;
        cfg_get_parse(line, &info, 0);
        isolate_group_id = cdb_int_str2val(info.value[TBL_INTERFACE_FLD_ISOLATE_GROUP_ID], &rc);
        ifindex = cdb_int_str2val(info.value[TBL_INTERFACE_FLD_IFINDEX], &rc);
        if (0 != sal_strcmp("{}", info.value[TBL_INTERFACE_FLD_LAG]) 
            && 0 != sal_strcmp("LAG", info.value[TBL_INTERFACE_FLD_HW_TYPE]))
        {
            /* bypass LAG member port */
            continue;
        }
        if (isolate_group_id > 0 && isolate_group_id <= GLB_PORT_ISOLATE_GROUP_MAX)
        {
            i = isolate_ifindex_count[isolate_group_id];
            isolate_ifindex_array[isolate_group_id][i] = ifindex;
            isolate_ifindex_count[isolate_group_id]++;
        }
    }
  
    for (i = 1; i <= GLB_PORT_ISOLATE_GROUP_MAX; i++)
    {
        if  ((group_id != 0) && (group_id != i))
        {
            continue;
        }
        member_count = isolate_ifindex_count[i];
        if (0 == member_count)
        {
            continue;
        }

        if (TRUE == banner)
        {
            banner = FALSE;
            sal_printf("--------------------------------------------------------------------------------\n");
            sal_printf("%-9s %s\n", "Group ID", "Member ports");
            sal_printf("---------+----------------------------------------------------------------------\n");
        }

        sal_printf("%-9u ", i);
        for (j = 0; j < member_count; j++)
        {
            ifindex = isolate_ifindex_array[i][j];
            rc = tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_SIZE);
            if (0 == rc)
            {
                if (j && (j % 5) == 0)
                {
                    sal_printf("\n");
                    sal_printf("%-9s ", "");
                }

                sal_printf("%-10s", ifname);
            }
        }
        sal_printf("\n");
        sal_printf("--------------------------------------------------------------------------------\n");
    }
      
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_port_isolate(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    char *mode_str = NULL;

    cdbclt_get_tbl_line(TBL_BRG_GLOBAL, NULL, 0, line, FILE_READ_LINE_LEN);
    cfg_get_parse(line, &info, 0);
    if (0 == sal_strcmp(info.value[TBL_BRG_GLOBAL_FLD_PORT_ISOLATE_MODE], "all"))
    {
        mode_str = "ALL";
    }
    else
    {
        mode_str = "L2";
    }
    
    sal_printf("%-18s : %s\n", "Port Isolate Mode", mode_str);
    
    return _cdbctl_show_interface_port_isolate_ports(fp, para, p_info);   
}

static int32
_cdbctl_show_interface_private_vlan(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    int32 ifindex = 0;
    char ifname[IFNAME_SIZE];
    int32 pvlan_type = 0;
    int32 primary_vid = 0;
    int32 secondary_vid = 0;
    int32 rc = PM_E_NONE;
    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        

        ifindex = cdb_int_str2val(info.value[TBL_INTERFACE_FLD_IFINDEX], &rc);
        pvlan_type = cdb_enum_str2val(glb_if_pvlan_type_strs, GLB_IF_PVLAN_TYPE_MAX, info.value[TBL_INTERFACE_FLD_PVLAN_TYPE]);
        primary_vid = cdb_int_str2val(info.value[TBL_INTERFACE_FLD_PRIVATE_VLAN], &rc);
        secondary_vid = cdb_int_str2val(info.value[TBL_INTERFACE_FLD_COMMUNITY_VLAN], &rc);
    }

    if(GLB_IF_PVLAN_TYPE_NONE == pvlan_type)
    {
        tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_SIZE);
        sal_printf(" Interface %s is not configured as private vlan\n", ifname);
        return PM_E_NONE;
    }
    sal_printf("------------------------------------------------------------------------------\n");
     
    if(GLB_IF_PVLAN_TYPE_PROMISCUOUS == pvlan_type)
    {
        sal_printf( " Private vlan mode: %s\n", "promiscuous");
    }
    else if (GLB_IF_PVLAN_TYPE_ISOLATED == pvlan_type)
    {
        sal_printf(" Private vlan mode: %s\n", "isolate");
    }
    else if(GLB_IF_PVLAN_TYPE_COMMUNITY == pvlan_type)
    {
        sal_printf( " Private vlan mode: %s\n", "community");
    }
    
    sal_printf( " Primary vlan id  : %d\n", primary_vid);
    
    if(GLB_IF_PVLAN_TYPE_COMMUNITY == pvlan_type)
    {
        sal_printf(" Secondary vlan id: %d\n", secondary_vid);
     }   

    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_flowcontrol_one(char *line)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    char *key = NULL;
    char *fc_send = NULL;
    char *fc_recv = NULL;
    char *fc_oper_send = NULL;
    char *fc_oper_recv = NULL;   
    char *pkt_rcv_str = NULL;
    char *pkt_send_str = NULL;    
    int32 rc = PM_E_NONE;
    char *ifindex = NULL;
    cfg_parse_get_info_t stats_info;
    char lines[FILE_READ_LINE_LEN];
    
    cfg_get_parse(line, &info, 0);

    key = info.value[TBL_INTERFACE_FLD_KEY];
    IFNAME_ETH2SHORT(key, ifname_ext);
    fc_send = info.value[TBL_INTERFACE_FLD_FLOWCONTROL_SEND];
    fc_recv = info.value[TBL_INTERFACE_FLD_FLOWCONTROL_RECEIVE];
    fc_oper_send = info.value[TBL_INTERFACE_FLD_FLOWCONTROL_OPER_SEND];
    fc_oper_recv = info.value[TBL_INTERFACE_FLD_FLOWCONTROL_OPER_RECEIVE];
            
    sal_memset(&stats_info, 0, sizeof(stats_info));
    ifindex = info.value[TBL_INTERFACE_FLD_IFINDEX];
    
    PM_E_RETURN(cdbclt_get_tbl_line(TBL_PORT_STATS, ifindex, sal_strlen(ifindex),lines,FILE_READ_LINE_LEN));
    cfg_get_parse(lines, &stats_info, 0);
    
    pkt_rcv_str = stats_info.value[TBL_PORT_STATS_FLD_PAUSE_RCV] ? stats_info.value[TBL_PORT_STATS_FLD_PAUSE_RCV] : 0;
    pkt_send_str = stats_info.value[TBL_PORT_STATS_FLD_PAUSE_SEND] ? stats_info.value[TBL_PORT_STATS_FLD_PAUSE_SEND] : 0;

    sal_printf("%-11s  %-8s %-8s    %-8s %-8s  %-12s %-12s\n", ifname_ext,
             on_off_str_lower(enable_value(fc_recv)), on_off_str_lower(enable_value(fc_oper_recv)),
             on_off_str_lower(enable_value(fc_send)), on_off_str_lower(enable_value(fc_oper_send)),
             pkt_rcv_str, pkt_send_str);
                
    return rc;
}

static int32
_cdbctl_show_interface_flowcontrol(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];
    sal_printf("\n%-11s  %-20s %-17s  %-12s %-12s\n", "Port", "Receive FlowControl", "Send FlowControl",
          "RxPause", "TxPause");
    sal_printf("             %-8s %-8s    %-8s %-8s\n", "admin", "oper", "admin", "oper");
    sal_printf("------------+--------+-----------+--------+---------+------------+-----------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_flowcontrol_one(line);
    }
    sal_printf("\n");

    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_storm_control_one(char *line)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    char *key = NULL;
    int32 rc = PM_E_NONE;
    cfg_parse_get_info_t storm_control_info;
    char *storm_control_mode_str[GLB_RATELIMIT_MAX_MODE] = {"none", "pps", "level", "bps"};
    int32 value = 0;
    
    cfg_get_parse(line, &info, 0);
    
    key = info.value[TBL_INTERFACE_FLD_KEY];
    IFNAME_ETH2SHORT(key, ifname_ext);
    sal_printf("%-11s", ifname_ext);

    if (0 == sal_strcmp("{}", info.value[TBL_INTERFACE_FLD_STORM_CONTROL]))
    {
        sal_printf("Disable             ");
        sal_printf("   ");
        sal_printf("Disable             ");
        sal_printf("   ");
        sal_printf("Disable             ");
        sal_printf("   ");

        sal_printf("\n");
        return PM_E_NONE;
    }
        
    sal_memset(&storm_control_info, 0, sizeof(storm_control_info));
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_STORM_CONTROL], &storm_control_info, 1);

    if (0 != sal_strcmp("0", storm_control_info.value[DS_STORM_CONTROL_FLD_UCAST_MODE]))
    {
        value = sal_atoi(storm_control_info.value[DS_STORM_CONTROL_FLD_UCAST_MODE]);
        if (0 == sal_strcmp(storm_control_mode_str[value], "level"))
        {
            value = sal_atoi(storm_control_info.value[DS_STORM_CONTROL_FLD_UCAST_RATE]);
            sal_printf("Level     %-10.2f",  (float)value/100);
        }
        else if (0 == sal_strcmp(storm_control_mode_str[value], "pps"))
        {
            sal_printf("PPS       %-10s", storm_control_info.value[DS_STORM_CONTROL_FLD_UCAST_RATE]);
        }                
    }
    else
    {
        sal_printf("Disable             ");
    }
    sal_printf("   ");


    if (0 != sal_strcmp("0", storm_control_info.value[DS_STORM_CONTROL_FLD_BCAST_MODE]))
    {
        value = sal_atoi(storm_control_info.value[DS_STORM_CONTROL_FLD_BCAST_MODE]);
        if (0 == sal_strcmp(storm_control_mode_str[value], "level"))
        {
            value = sal_atoi(storm_control_info.value[DS_STORM_CONTROL_FLD_BCAST_RATE]);
            sal_printf("Level     %-10.2f",  (float)value/100);
        }
        else if (0 == sal_strcmp(storm_control_mode_str[value], "pps"))
        {
            sal_printf("PPS       %-10s", storm_control_info.value[DS_STORM_CONTROL_FLD_BCAST_RATE]);
        }                
    }
    else
    {
        sal_printf("Disable             ");
    }
    sal_printf("   ");

    if (0 != sal_strcmp("0", storm_control_info.value[DS_STORM_CONTROL_FLD_MCAST_MODE]))
    {
        value = sal_atoi(storm_control_info.value[DS_STORM_CONTROL_FLD_MCAST_MODE]);
        if (0 == sal_strcmp(storm_control_mode_str[value], "level"))
        {
            value = sal_atoi(storm_control_info.value[DS_STORM_CONTROL_FLD_MCAST_RATE]);
            sal_printf("Level     %-10.2f",  (float)value/100);
        }
        else if (0 == sal_strcmp(storm_control_mode_str[value], "pps"))
        {
            sal_printf("PPS       %-10s", storm_control_info.value[DS_STORM_CONTROL_FLD_MCAST_RATE]);
        }                
    }
    else
    {
        sal_printf("Disable             ");
    }
    sal_printf("   ");

    sal_printf("\n");
    return rc;
}


static int32
_cdbctl_show_interface_storm_control(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];
    sal_printf("Port       ucastMode ucastlevel   bcastMode bcastLevel   mcastMode mcastLevel\n");
    sal_printf("----------+---------+------------+---------+------------+---------+----------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_storm_control_one(line);
    }
    sal_printf("\n");

    return PM_E_NONE;
}

/*
sFlow Port Information:
                                 Flow-Sample   Flow-Sample
 Port        Counter   Flow      Direction     Rate

*/

static int32
_cdbctl_show_interface_sflow_one(char *line)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    char *key = NULL;

    cfg_get_parse(line, &info, 0);

    if ((0 == sal_strcmp("0", info.value[TBL_INTERFACE_FLD_SFLOW_FLOW_INGRESS]))
        &&(0 == sal_strcmp("0", info.value[TBL_INTERFACE_FLD_SFLOW_FLOW_EGRESS]))
        &&(0 == sal_strcmp("0", info.value[TBL_INTERFACE_FLD_SFLOW_COUNTER_EN])))
    {
        return PM_E_NONE;
    }

    key = info.value[TBL_INTERFACE_FLD_KEY];
    IFNAME_ETH2SHORT(key, ifname_ext);
    sal_printf("%-12s", ifname_ext);

    if (0 == sal_strcmp("1", info.value[TBL_INTERFACE_FLD_SFLOW_COUNTER_EN]))
    {
        sal_printf("%-10s", "Enable");
    }
    else
    {
        sal_printf("%-10s", "Disable");
    }

    if ((0 == sal_strcmp("1", info.value[TBL_INTERFACE_FLD_SFLOW_FLOW_INGRESS])) ||
        (0 == sal_strcmp("1", info.value[TBL_INTERFACE_FLD_SFLOW_FLOW_EGRESS])))
    {
        sal_printf("%-10s", "Enable");
    }
    else
    {
        sal_printf("%-10s", "Disable");
    }

    if ((0 == sal_strcmp("1", info.value[TBL_INTERFACE_FLD_SFLOW_FLOW_INGRESS])) &&
        (0 == sal_strcmp("1", info.value[TBL_INTERFACE_FLD_SFLOW_FLOW_EGRESS])))
    {
        sal_printf("%-14s", "Both");
    }
    else if (0 == sal_strcmp("1", info.value[TBL_INTERFACE_FLD_SFLOW_FLOW_INGRESS]))
    {
        sal_printf("%-14s", "Input");
    }
    else if (0 == sal_strcmp("1", info.value[TBL_INTERFACE_FLD_SFLOW_FLOW_EGRESS]))
    {
        sal_printf("%-14s", "Output");
    }
    else
    {
        sal_printf("%-14s", "N/A");
    }

    if (0 == sal_strcmp("0", info.value[TBL_INTERFACE_FLD_SFLOW_RATE]))
    {
        sal_printf("%-10s", "N/A");
    }
    else
    {
        sal_printf("%-10s", info.value[TBL_INTERFACE_FLD_SFLOW_RATE]);
    }

    sal_printf("\n");
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_sflow(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    char line[FILE_READ_LINE_32K_LEN];

    sal_printf("sFlow Port Information:\n");
    sal_printf("                                Flow-Sample   Flow-Sample\n");
    sal_printf("Port        Counter   Flow      Direction     Rate\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_sflow_one(line);
    }

    sal_printf("\n");
    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_aclqos_if_one(char *line)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    char *key = NULL;
    int32 rc = PM_E_NONE;
    cfg_parse_get_info_t aclqos_if;
    char *trust_str[GLB_QOS_TRUST_MAX] =
    {
        "port",
        "outer",
        "cos",
        "dscp-exp",
        "ip-prec",
        "ctag-cos",
        "stag-cos"
    };
    char *replace_str[GLB_QOS_REPLACE_MAX] =
    {
        "none",
        "cos",
        "dscp-exp",
        "stag-cos",
        "ctag-cos"
    };
    
    int32 value = 0;
    
    cfg_get_parse(line, &info, 0);
    
    key = info.value[TBL_INTERFACE_FLD_KEY];
    IFNAME_ETH2SHORT(key, ifname_ext);

    if (0 == sal_strcmp("{}", info.value[TBL_INTERFACE_FLD_ACLQOS_IF]))
    {
        return PM_E_NONE;
    }

    sal_printf("%-10s", ifname_ext);    
    sal_memset(&aclqos_if, 0, sizeof(aclqos_if));
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_ACLQOS_IF], &aclqos_if, 1);

    
    sal_printf("%-7s",  aclqos_if.value[DS_ACLQOS_IF_FLD_DOMAIN]);

    value = sal_atoi(aclqos_if.value[DS_ACLQOS_IF_FLD_TRUST]);
    sal_printf("%-9s",  trust_str[value]);
    
    value = sal_atoi(aclqos_if.value[DS_ACLQOS_IF_FLD_REPLACE]);
    sal_printf("%-9s",  replace_str[value]);

    sal_printf("%-8s",  aclqos_if.value[DS_ACLQOS_IF_FLD_DEFAULT_COS]);

    value = sal_atoi(aclqos_if.value[DS_ACLQOS_IF_FLD_SHAPE_PIR]);
    if (value == GLB_QOS_SHAPE_DEFAULT_RATE)
    {
        sal_printf("%-10s",  "none");
    }
    else 
    {
        sal_printf("%-10s",  aclqos_if.value[DS_ACLQOS_IF_FLD_SHAPE_PIR]);
    }

    if (0 == sal_strcmp(aclqos_if.value[DS_ACLQOS_IF_FLD_INPUT_POLICER], ""))
    {
        sal_printf("%-21s",  "none");
    }
    else
    {
        sal_printf("%-21s",  aclqos_if.value[DS_ACLQOS_IF_FLD_INPUT_POLICER]);
    }
    
    if (0 == sal_strcmp(aclqos_if.value[DS_ACLQOS_IF_FLD_OUTPUT_POLICER], ""))
    {
        sal_printf("%-s",  "none");
    }
    else
    {
        sal_printf("%-s",  aclqos_if.value[DS_ACLQOS_IF_FLD_OUTPUT_POLICER]);
    }
    
    sal_printf("\n");
    return rc;
}


static int32
_cdbctl_show_interface_aclqos_if(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];
 /*         %-10s       %-7d   %-9s    %-9s     %-8d    %-10d     %-21s                   %-s*/
 sal_printf("Port      domain trust    replace  def-cos shape     input-policer        output-policer\n");
 sal_printf("---------+------+--------+--------+-------+---------+--------------------+--------------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_aclqos_if_one(line);
    }
    sal_printf("\n");

    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_aclqos_if_queue_one(char *line)
{
    cfg_parse_get_info_t info;
    int32 rc = PM_E_NONE;
    cfg_parse_get_info_t aclqos_if;
    cfg_parse_get_info_t queue_class;
    cfg_parse_get_info_t queue_weight;
    cfg_parse_get_info_t queue_drop;
    cfg_parse_get_info_t queue_shape;
    cfg_parse_get_info_t queue_drop_mode;
    cfg_parse_get_info_t queue_ecn_enable;

    char* drop_mode[] = 
    {
        "tail-drop",
        "random-detect"
    };
    int32 i = 0;
    
    cfg_get_parse(line, &info, 0);
    
    if (0 == sal_strcmp("{}", info.value[TBL_INTERFACE_FLD_ACLQOS_IF]))
    {
        return PM_E_NONE;
    }

    sal_memset(&aclqos_if, 0, sizeof(aclqos_if));
    sal_memset(&queue_class, 0, sizeof(queue_class));
    sal_memset(&queue_weight, 0, sizeof(queue_weight));
    sal_memset(&queue_drop, 0, sizeof(queue_drop));
    sal_memset(&queue_shape, 0, sizeof(queue_shape));
    
    cfg_get_parse(info.value[TBL_INTERFACE_FLD_ACLQOS_IF], &aclqos_if, 1);
    cfg_get_parse_comma(aclqos_if.value[DS_ACLQOS_IF_FLD_QUEUE_CLASS], &queue_class);
    cfg_get_parse_comma(aclqos_if.value[DS_ACLQOS_IF_FLD_QUEUE_WEIGHT], &queue_weight);
    cfg_get_parse_comma(aclqos_if.value[DS_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE], &queue_drop);
    cfg_get_parse_comma(aclqos_if.value[DS_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE], &queue_shape);
    cfg_get_parse_comma(aclqos_if.value[DS_ACLQOS_IF_FLD_QUEUE_DROP_MODE], &queue_drop_mode);
    cfg_get_parse_comma(aclqos_if.value[DS_ACLQOS_IF_FLD_QUEUE_ECN_ENABLE], &queue_ecn_enable);
    
    for (i = 0; i < GLB_QOS_PORT_QUEUE_NUM; i++)
    {
        sal_printf("%-6d",  i);
        if (queue_class.argv[i])
        {
            sal_printf("%-6s",  queue_class.argv[i]);
        }
        if (queue_weight.argv[i])
        {
            sal_printf("%-7s",  queue_weight.argv[i]);
        }
        if (queue_drop_mode.argv[i])
        {
            sal_printf("%-14s",  drop_mode[sal_atoi(queue_drop_mode.argv[i])]);
        }
        if (queue_ecn_enable.argv[i])
        {
            sal_printf("%-8s",  sal_atoi(queue_ecn_enable.argv[i]) ? "enable" : "disable");
        }         
        if (queue_drop.argv[i] && sal_strlen(queue_drop.argv[i]))
        {
            sal_printf("%-21s",  queue_drop.argv[i]);
        }
        else
        {
            sal_printf("%-21s",  "none");
        }
        if (queue_shape.argv[i] && sal_strlen(queue_shape.argv[i]))
        {
            sal_printf("%-s",  queue_shape.argv[i]);
        }
        else
        {
            sal_printf("%-s",  "none");
        }
        sal_printf("\n");
    }

    return rc;
}


static int32
_cdbctl_show_interface_aclqos_if_queue(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_32K_LEN];
 /*         %-6s   %-6d  %-7d   %-14s         %-8s    %-21s               %-s */
 sal_printf("Queue class weight drop-mode     ecn     drop-profile        shape-profile\n");
 sal_printf("-----+-----+------+-------------+-------+-------------------+-------------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        _cdbctl_show_interface_aclqos_if_queue_one(line);
    }
    sal_printf("\n");

    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_aclqos_if_queue_shape(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    int32 i = 0;
    char  line[FILE_READ_LINE_32K_LEN];
    cfg_parse_get_info_t info;
    int32 value = 0;

    /*         %-6s   %-10d     %-d  */
    sal_printf("Queue CIR(Kbps) PIR(Kbps)\n");
    sal_printf("-----+---------+---------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        sal_printf("%-6d", i++);
        if (('\0' == line[0]) || ('\n' == line[0]))
        {
            sal_printf("\n");
            continue;
        }
        sal_memset(&info, 0, sizeof(info));
        cfg_get_parse(line, &info, 0);
        value = sal_atoi(info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_FLAGS_CIR]);
        if (0 != value)
        {
            sal_printf("%-10s", info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_CIR]);
        }
        value = sal_atoi(info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_FLAGS_PIR]);
        if (0 != value)
        {
            sal_printf("%-s", info.value[TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_PIR]);
        }
        sal_printf("\n");
    }

    return PM_E_NONE;
}

    static int32
_cdbctl_show_interface_aclqos_if_queue_drop(FILE *fp, char *para, cfg_parse_info_t *p_info)
{
    int32 i = 0;
    char  line[FILE_READ_LINE_32K_LEN];
    cfg_parse_get_info_t info;
    bool random_detect = FALSE;

    /*         %-6s   %-15d          %-7s   %-10s     %-10s     %-s           */
    sal_printf("Queue Drop-mode      Color  Max_Tresh Min_Tresh Probability\n");
    sal_printf("-----+--------------+------+---------+---------+---------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_32K_LEN, fp))
    {
        sal_memset(&info, 0, sizeof(info));
        cfg_get_parse(line, &info, 0);        
        sal_printf("%-6d", i++);
        if (0 != sal_atoi(info.value[TBL_QOS_DROP_PROFILE_FLD_FLAGS_RANDOM_DETECT]))
        {
            random_detect = TRUE;
            sal_printf("%-15s", "random-detect");
        }
        else
        {
            random_detect = FALSE;
            sal_printf("%-15s", "tail-drop");
        }
        sal_printf("%-7s", "green");
        sal_printf("%-10s", info.value[TBL_QOS_DROP_PROFILE_FLD_GREEN_MAX_THREASHOLD]);
        if (random_detect)
        {
            sal_printf("%-10s", info.value[TBL_QOS_DROP_PROFILE_FLD_GREEN_MIN_THREASHOLD]);
            sal_printf("%-s/65536", info.value[TBL_QOS_DROP_PROFILE_FLD_GREEN_PROBABILITY]);
        }
        sal_printf("\n");
        
        sal_printf("%21s", " ");
        sal_printf("%-7s", "yellow");
        sal_printf("%-10s", info.value[TBL_QOS_DROP_PROFILE_FLD_YELLOW_MAX_THREASHOLD]);
        if (random_detect)
        {
            sal_printf("%-10s", info.value[TBL_QOS_DROP_PROFILE_FLD_YELLOW_MIN_THREASHOLD]);
            sal_printf("%-s/65536", info.value[TBL_QOS_DROP_PROFILE_FLD_YELLOW_PROBABILITY]);
        }
        sal_printf("\n");

        sal_printf("%21s", " ");
        sal_printf("%-7s", "red");
        sal_printf("%-10s", info.value[TBL_QOS_DROP_PROFILE_FLD_RED_MAX_THREASHOLD]);
        if (random_detect)
        {
            sal_printf("%-10s", info.value[TBL_QOS_DROP_PROFILE_FLD_RED_MIN_THREASHOLD]);
            sal_printf("%-s/65536", info.value[TBL_QOS_DROP_PROFILE_FLD_RED_PROBABILITY]);
        }
        sal_printf("\n");

    }
    sal_printf("\n");

    return PM_E_NONE;
}

static int32
_cdbctl_show_interface_dump_all(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        sal_printf("%s", line);;
    }

    return PM_E_NONE;
}

int32
cdbctl_show_interface(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  *para = p_data;
    uint32 show_one = FALSE;
    
    if (data_len)
    {
        if (!sal_strcmp(para, "status"))
        {
            _cdbctl_show_interface_status(fp, p_info);
        }
        else if (!sal_strcmp(para, "description"))
        {
            _cdbctl_show_interface_description(fp, p_info);
        }
        else if (!sal_strcmp(para, "loopback"))
        {
            _cdbctl_show_interface_loopback(fp, p_info);
        }
        else if (!sal_strcmp(para, "key"))
        {
            _cdbctl_show_interface_key(fp, p_info);
        }
        else if (sal_strstr(para, "summary") && !sal_strstr(para, "lag"))
        {
            /* should be not "lag//type/summary" */
            _cdbctl_show_interface_summary(fp, p_info);
        }
        else if (sal_strstr(para, "switchport"))
        {
            if (sal_strcmp(para, "switchport"))
            {
                show_one = TRUE;
            }
            _cdbctl_show_interface_switchport(fp, p_info, show_one);
        }
        else if (sal_strstr(para, "port_security_max_mac"))
        {
            _cdbctl_show_interface_port_security_max_mac(fp, p_info);
        }
        else if (sal_strstr(para, "trunk"))
        {
            _cdbctl_show_interface_trunk(fp, p_info);
        }
        else if (sal_strstr(para, "lagmember"))
        {
            _cdbctl_show_interface_lag_member(fp, para, p_info);
        }
        else if (sal_strstr(para, "lag"))
        {
            _cdbctl_show_interface_lag(fp, para, p_info);
        }
        else if (sal_strstr(para, "lacp"))
        {
            _cdbctl_show_interface_lacp(fp, para, p_info);
        }
        else if (sal_strstr(para, "port_isolate"))
        {
            _cdbctl_show_interface_port_isolate(fp, para, p_info);
        }
        else if (sal_strstr(para, "private_vlan"))
        {
            _cdbctl_show_interface_private_vlan(fp, para, p_info);
        }
        else if (sal_strstr(para, "flowcontrol"))
        {
            _cdbctl_show_interface_flowcontrol(fp, para, p_info);
        }
        else if (sal_strstr(para, "storm_control"))
        {
            _cdbctl_show_interface_storm_control(fp, para, p_info);
        }
        else if (sal_strstr(para, "sflow"))
        {
            _cdbctl_show_interface_sflow(fp, para, p_info);
        }
        else if (sal_strstr(para, "aclqos_if_queue_drop"))
        {
            _cdbctl_show_interface_aclqos_if_queue_drop(fp, para, p_info);
        }  
        else if (sal_strstr(para, "aclqos_if_queue_shape"))
        {
            _cdbctl_show_interface_aclqos_if_queue_shape(fp, para, p_info);
        }  
        else if (sal_strstr(para, "aclqos_if_queue_stats"))
        {
            _cdbctl_show_interface_dump_all(fp, p_info);
        }                 
        else if (sal_strstr(para, "aclqos_if_queue"))
        {
            _cdbctl_show_interface_aclqos_if_queue(fp, para, p_info);
        }  
        else if (sal_strstr(para, "aclqos_if_pmap_stats"))
        {
            _cdbctl_show_interface_dump_all(fp, p_info);
        }                 
        else if (sal_strstr(para, "aclqos_if_policer_stats"))
        {
            _cdbctl_show_interface_dump_all(fp, p_info);
        }                 
        else if (sal_strstr(para, "aclqos_if"))
        {
            _cdbctl_show_interface_aclqos_if(fp, para, p_info);
        }                 
        else
        {
            _cdbctl_show_interface_detail(fp, p_info);
        }
    }
    else
    {
        _cdbctl_show_interface_detail(fp, p_info);
    }

    return PM_E_NONE;
}

int32
cdbctl_show_lag_global_sysid(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char *field_str = (char*)p_data;
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    char *priority = NULL;
    char *system_id = NULL;
    
    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        if (sal_strstr(field_str, "mlag"))
        {
            priority = info.value[TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_PRIORITY];
            system_id = info.value[TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_ID];
        }
        else
        {
            priority = info.value[TBL_LAG_GLOBAL_FLD_SYSTEM_PRIORITY];
            system_id = info.value[TBL_LAG_GLOBAL_FLD_SYSTEM_ID];
        }
        sal_printf("System ID: %s,%s\n", priority, system_id);
    }

    return PM_E_NONE;
}

int32
cdbctl_show_lag_global_load_balance(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    char *i = NULL;
    int load_balance = 0;

    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        i = info.value[TBL_LAG_GLOBAL_FLD_LOAD_BALANCE];
        load_balance = sal_atoi(i);

        sal_printf("Port-channel load-balance hash fields:\n");
        sal_printf("--------------------------------------\n");

        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_SRC_MAC))
        {
            sal_printf("%-15s\n", "src-mac");
        }

        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_DST_MAC))
        {
            sal_printf("%-15s\n", "dst-mac");
        }

        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_SRC_IP))
        {
            sal_printf("%-15s\n", "src-ip");
        }

        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_DST_IP))
        {
            sal_printf("%-15s\n", "dst-ip");
        }

        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_IP_PROTOCOL))
        {
            sal_printf("%-15s\n", "ip-protocol");
        }

        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_SRC_PORT))
        {
            sal_printf("%-15s\n", "src-port-l4");
        }

        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_DST_PORT))
        {
            sal_printf("%-15s\n", "dst-port-l4");
        }
        
        if(GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_VXLAN_VNI))
        {
            sal_printf("%-15s\n", "vxlan-vni");
        }

        if(GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_GRE_KEY))
        {
            sal_printf("%-15s\n", "gre-key");
        }

        if(GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_NVGRE_VNI))
        {
            sal_printf("%-15s\n", "nvgre-vsid");
        }

        if(GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_NVGRE_FLOW_ID))
        {
            sal_printf("%-15s\n", "nvgre-flow-id");
        }
        
        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_INNER_SRC_MAC))
        {
            sal_printf("%-15s\n", "inner-src-mac");
        }
        
        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_INNER_DST_MAC))
        {
            sal_printf("%-15s\n", "inner-dst-mac");
        }
        
        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_INNER_SRC_IP))
        {
            sal_printf("%-15s\n", "inner-src-ip");
        }
        
        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_INNER_DST_IP))
        {
            sal_printf("%-15s\n", "inner-dst-ip");
        }

        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_INNER_IP_PROTOCOL))
        {
            sal_printf("%-15s\n", "inner-ip-protocol");
        }

        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_INNER_SRC_PORT_L4))
        {
            sal_printf("%-15s\n", "inner-src-port-l4");
        }

        if (GLB_FLAG_ISSET(load_balance, GLB_LAG_LOAD_BALANCE_INNER_DST_PORT_L4))
        {
            sal_printf("%-15s\n", "inner-dst-port-l4");
        }

        sal_printf("\n");
    }
    return PM_E_NONE;
}

