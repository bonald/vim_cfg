#include "proto.h"
#include "cdbclient.h"
#include "glb_l2_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_pvlan_define.h"
#include "gen/tbl_pvlan.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_sys_spec_define.h"

static int32
_cdbctl_show_fdb_one(char *line)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_result_t rslt;
    cfg_parse_get_info_t info;
    char *fid_str = NULL;
    char *mac_str = NULL;
    char ifname[IFNAME_SIZE];
    char type_str[16];
    int32 ifindex = 0;
    mac_addr_t mac;
    port_bmp_t port_bmp;
    uint32 with_key = TRUE;
    uint32 is_static = FALSE;
    uint32 is_security = FALSE;
    int32 rc = PM_E_NONE;
        
    cfg_get_parse(line, &info, 0);

    mac_str = info.value[TBL_FDB_FLD_KEY];
    fid_str = sal_strstr(mac_str, CMD_KEY_DIV_STR);
    fid_str[0] = CMD_ZERO_CHAR;
    fid_str += 1;

    sal_memset(mac, 0, sizeof(mac));
    rc = cdb_mac_str2val(mac, mac_str);
    if (rc < 0)
    {
        return PM_E_FAIL;
    }

    is_static = ('0' != info.value[TBL_FDB_FLD_STATIC][0]) ? TRUE : FALSE;
    is_security = ('0' != info.value[TBL_FDB_FLD_SECURITY][0]) ? TRUE : FALSE;
    type_str[0] = CMD_ZERO_CHAR;
    if (is_static)
    {
        sal_strcat(type_str, "static");
    }
    else
    {
        sal_strcat(type_str, "dynamic");
    }

    if (is_security)
    {
        sal_strcat(type_str, "(*)");
    }
    
    if (GLB_IS_MCAST_MAC(mac))
    {
        GLB_BMP_INIT(port_bmp);
        rc = cdb_bitmap_str2val(info.value[TBL_FDB_FLD_IFINDEX], 0, GLB_AGG_IFINDEX_MIN + GLB_LINKAGG_MAX_GROUP, port_bmp, GLB_PORT_BMP_WORD_MAX, rslt.desc);
        GLB_BMP_ITER_BEGIN(port_bmp, ifindex)
        {
            ifname[0] = CMD_ZERO_CHAR;
            tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_SIZE);
            IFNAME_ETH2SHORT(ifname, ifname_ext);
            if (with_key)
            {
                sal_printf("%-7s %-16s %-12s %-8s\n", 
                    fid_str,
                    mac_str,
                    type_str,
                    ifname_ext);
                with_key = FALSE;
            }
            else
            {
                sal_printf("%-37s %-8s\n", 
                    "",
                    ifname_ext);
            }
        }
        GLB_BMP_ITER_END(port_bmp, ifindex);
    }
    else
    {
        ifindex = cdb_int_str2val(info.value[TBL_FDB_FLD_IFINDEX], &rc);
        if (rc < 0)
        {
            return PM_E_FAIL;
        }
        ifname[0] = CMD_ZERO_CHAR;
        tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_SIZE);
        IFNAME_ETH2SHORT(ifname, ifname_ext);
        sal_printf("%-7s %-16s %-12s %-8s\n", 
            fid_str,
            mac_str,
            type_str,
            ifname_ext);
    }

    return rc;
}


static int32
_cdbctl_show_psfdb_one(char *line)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_result_t rslt;
    cfg_parse_get_info_t info;
    char *fid_str = NULL;
    char *mac_str = NULL;
    char ifname[IFNAME_SIZE];
    char type_str[32];
    int32 ifindex = 0;
    mac_addr_t mac;
    port_bmp_t port_bmp;
    uint32 with_key = TRUE;
    uint32 is_static = FALSE;
    int32 rc = PM_E_NONE;
        
    cfg_get_parse(line, &info, 0);

    mac_str = info.value[TBL_FDB_FLD_KEY];
    fid_str = sal_strstr(mac_str, CMD_KEY_DIV_STR);
    fid_str[0] = CMD_ZERO_CHAR;
    fid_str += 1;

    sal_memset(mac, 0, sizeof(mac));
    rc = cdb_mac_str2val(mac, mac_str);
    if (rc < 0)
    {
        return PM_E_FAIL;
    }

    is_static = ('0' != info.value[TBL_FDB_FLD_STATIC][0]) ? TRUE : FALSE;
    type_str[0] = CMD_ZERO_CHAR;
    if (is_static)
    {
        sal_strcat(type_str, "SecureConfigured");
    }
    else
    {
        sal_strcat(type_str, "SecureLearned");
    }

    if (GLB_IS_MCAST_MAC(mac))
    {
        GLB_BMP_INIT(port_bmp);
        rc = cdb_bitmap_str2val(info.value[TBL_FDB_FLD_IFINDEX], 0, GLB_AGG_IFINDEX_MIN + GLB_LINKAGG_MAX_GROUP, port_bmp, GLB_PORT_BMP_WORD_MAX, rslt.desc);
        GLB_BMP_ITER_BEGIN(port_bmp, ifindex)
        {
            ifname[0] = CMD_ZERO_CHAR;
            tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_SIZE);
            IFNAME_ETH2SHORT(ifname, ifname_ext);
            if (with_key)
            {
                sal_printf("%-7s %-16s %-12s %-8s\n", 
                    fid_str,
                    mac_str,
                    type_str,
                    ifname_ext);
                with_key = FALSE;
            }
            else
            {
                sal_printf("%-37s %-8s\n", 
                    "",
                    ifname_ext);
            }
        }
        GLB_BMP_ITER_END(port_bmp, ifindex);
    }
    else
    {
        ifindex = cdb_int_str2val(info.value[TBL_FDB_FLD_IFINDEX], &rc);
        if (rc < 0)
        {
            return PM_E_FAIL;
        }
        ifname[0] = CMD_ZERO_CHAR;
        tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_SIZE);
        IFNAME_ETH2SHORT(ifname, ifname_ext);
        sal_printf("%-7s %-16s %-16s %-8s\n", 
            fid_str,
            mac_str,
            type_str,
            ifname_ext);
    }

    return rc;
}


static int32
_cdbctl_show_macfilter_one(char *line)
{
    cfg_parse_get_info_t info;
    char *mac_str = NULL;
    int32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);

    mac_str = info.value[TBL_MACFILTER_FLD_KEY];
    sal_printf("%-14s\n", mac_str);

    return rc;
}


static int32
_cdbctl_show_vlan_one(char *line, uint32 *p_banner)
{
    cfg_parse_get_info_t info;
    cfg_result_t rslt;
    port_bmp_t port_bmp;
    char *bmp_str = NULL;
    uint32 first_line = TRUE;
    uint32 ifindex = 0;
    char member_str[MAX_CMD_LINE_LEN];
    char ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_EXT_SIZE];
    uint32 str_len = 0;
    int32 rc = PM_E_NONE;
    
    if (TRUE == *p_banner)
    {
        *p_banner = FALSE;
        sal_printf("%-8s %-16s %-8s %-8s %-36s\n", "VLAN ID", "Name", "State", "Instance", "Member ports");
        sal_printf("--------+----------------+--------+--------+----------------------------------\n");
    }

    cfg_get_parse(line, &info, 0);

    sal_printf("%-8s %-16s %-8s %-8s ", 
        info.value[TBL_VLAN_FLD_KEY], 
        info.value[TBL_VLAN_FLD_NAME], 
        active_str(enable_value(info.value[TBL_VLAN_FLD_ENABLE])), 
        info.value[TBL_VLAN_FLD_INSTANCE]);

    bmp_str = info.value[TBL_VLAN_FLD_MEMBER_PORT];
    GLB_BMP_INIT(port_bmp);
    rc = cdb_bitmap_str2val(bmp_str, 0, GLB_AGG_IFINDEX_MIN + GLB_LINKAGG_MAX_GROUP, port_bmp, GLB_PORT_BMP_WORD_MAX, rslt.desc);

    member_str[0] = CMD_ZERO_CHAR;
    GLB_BMP_ITER_BEGIN(port_bmp, ifindex)
    {
        rc = tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_SIZE);
        if (rc < 0)
        {
            continue;
        }
        IFNAME_ETH2SHORT(ifname, ifname_ext);
        sal_strncat(ifname_ext, ", ", 2);
        
        if (sal_strlen(member_str) + sal_strlen(ifname_ext) >= 36)
        {
            if (first_line)
            {
                first_line = FALSE;
                sal_printf("%s\n", member_str);
            }
            else
            {
                sal_printf("%-43s %s\n", "", member_str);
            }
            member_str[0] = CMD_ZERO_CHAR;
            
        }
        sal_strcat(member_str, ifname_ext);
    }
    GLB_BMP_ITER_END(port_bmp, ifindex);

    str_len = sal_strlen(member_str);
    if (str_len)
    {
        if (CMD_COMMA_CHAR == member_str[str_len-2])
        {
            member_str[str_len-2] = CMD_ZERO_CHAR;
        }
        
        if (first_line)
        {
            first_line = FALSE;
            sal_printf("%s", member_str);
        }
        else
        {
            sal_printf("%-43s %s", "", member_str);
        }
    }
    
    sal_printf("\n");

    return PM_E_NONE;
}

static int32
_cdbctl_show_brg_fdb_aging_time(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    char *aging_time = NULL;
    
    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        aging_time = info.value[0];
        /*Modified by yejl to fix bug40695, 2016-09-13*/
        if (0 == sal_strcmp(aging_time, "0"))
        {
            sal_printf("MAC address table is not ageing\n");
        }
        else
        {
            sal_printf("MAC address table ageing time is %s seconds\n", aging_time);
        }
    }

    return PM_E_NONE;
}

static int32
_cdbctl_show_vlan_security_one(char *line, uint32 *p_banner)
{
    cfg_parse_get_info_t info;

    if (TRUE == *p_banner)
    {
        *p_banner = FALSE;
        sal_printf(" Vlan  learning-en  max-mac-count  cur-mac-count  action\n");
        sal_printf(" -----+------------+--------------+--------------+-------\n"); 
    }

    cfg_get_parse(line, &info, 0);

    sal_printf(" %-5s %-12s ", 
        info.value[0], 
        enable_str(enable_value(info.value[1])));

    if (!sal_memcmp(info.value[2],"4294967295", 10))
    {
         sal_printf("%-14s ", "N/A");        
    }
    else
    {
        sal_printf("%-14s ", info.value[2]);
    }

    sal_printf("%-14s ", info.value[3]);
    
    if (!sal_memcmp(info.value[4],"forward", 7))
    {
        sal_printf("%s\n", "Forward");
    }
    else if (!sal_memcmp(info.value[4],"discard", 7))
    {
        sal_printf("%s\n", "Discard");
    }
    else if (!sal_memcmp(info.value[4],"warn", 4))
    {
        sal_printf("%s\n", "Warn");
    }      

    return PM_E_NONE;
}

int32
cdbctl_show_vlan(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    uint32 banner = TRUE;

    if (0 == sal_memcmp(p_info->path, "cdb/l2/vlan/security", 20))
    {
        while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
        {
            _cdbctl_show_vlan_security_one(line, &banner);
        }
        
        return PM_E_NONE;
    }

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbctl_show_vlan_one(line, &banner);
    }

    return PM_E_NONE;
}

static int32
_cdbctl_show_pvlan_one(char *line, uint32 *p_banner)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t ds_info;
    int32 rc = PM_E_NONE;
    cfg_parse_get_info_t refer_list;   
    cfg_parse_get_info_t refer_ds_list;  
    int32 i = 0;
    int32 j = 0;
    int32 k = 0;
    int32 value = 0;
    int32 ds_value = 0;
    char intf_line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t intf_info;
    uint32 mem_count = 0;


    cfg_get_parse(line, &info, 0);
    
    /*promiscuous*/
    if (info.value[TBL_PVLAN_FLD_PROMISCUOUS_PORTS] && 
          sal_strlen(info.value[TBL_PVLAN_FLD_PROMISCUOUS_PORTS]))
    {
        value = cdb_int_str2val(info.value[TBL_PVLAN_FLD_KEY], &rc);
        sal_printf("%-10d%-10s%-15s", value, "N/A", "promiscuous"); 
        mem_count = 0;
        cfg_get_parse_referlist(info.value[TBL_PVLAN_FLD_PROMISCUOUS_PORTS], FALSE, &refer_list);
        for (i = 0; i < refer_list.argc; i++)
        {    
             PM_E_RETURN(cdbclt_get_tbl_line(TBL_INTERFACE, refer_list.argv[i], sal_strlen(refer_list.argv[i]), intf_line, FILE_READ_LINE_LEN));
             cfg_get_parse(intf_line, &intf_info, 0);
 
             if (0 != sal_strcmp("{}", intf_info.value[TBL_INTERFACE_FLD_LAG]) 
                 && 0 != sal_strcmp("LAG", intf_info.value[TBL_INTERFACE_FLD_HW_TYPE]))
             {
                 /* bypass LAG member port */
                 continue;
             }
             if(0 != mem_count && 0 == mem_count % 4)
             {
                 sal_printf( "\n");
                 for(j = 0; j < 35; j++)sal_printf( " ");
             }
             sal_printf( "%-10s", refer_list.argv[i]);
             mem_count++;
        }
        sal_printf("\n");
    }

    /*isolate*/
    if (info.value[TBL_PVLAN_FLD_ISLOATE_PORTS] && 
          sal_strlen(info.value[TBL_PVLAN_FLD_ISLOATE_PORTS]))
    {
        value = cdb_int_str2val(info.value[TBL_PVLAN_FLD_KEY], &rc);
        sal_printf("%-10d%-10s%-15s", value, "N/A", "isolate"); 
        mem_count = 0;
        cfg_get_parse_referlist(info.value[TBL_PVLAN_FLD_ISLOATE_PORTS], FALSE, &refer_list);
        for (i = 0; i < refer_list.argc; i++)
        {
             PM_E_RETURN(cdbclt_get_tbl_line(TBL_INTERFACE, refer_list.argv[i], sal_strlen(refer_list.argv[i]), intf_line, FILE_READ_LINE_LEN));
             cfg_get_parse(intf_line, &intf_info, 0);
 
             if (0 != sal_strcmp("{}", intf_info.value[TBL_INTERFACE_FLD_LAG]) 
                 && 0 != sal_strcmp("LAG", intf_info.value[TBL_INTERFACE_FLD_HW_TYPE]))
             {
                 /* bypass LAG member port */
                 continue;
             }
             if(0 != mem_count && 0 == mem_count % 4)
             {
                 sal_printf( "\n");
                 for(j = 0; j < 35; j++)sal_printf( " ");
             }
             sal_printf( "%-10s", refer_list.argv[i]);
             mem_count++;
        }
        sal_printf("\n");
    }

    /*coumunity*/
    if (info.value[TBL_PVLAN_FLD_COMMUNITY_GROUPS] && 
          sal_strlen(info.value[TBL_PVLAN_FLD_COMMUNITY_GROUPS]))
    {
       value = cdb_int_str2val(info.value[TBL_PVLAN_FLD_KEY], &rc);
       cfg_get_parse_referlist(info.value[TBL_PVLAN_FLD_COMMUNITY_GROUPS], TRUE, &refer_list);
       
       for (i = 0; i < refer_list.argc; i++)
       {
            cfg_get_parse(refer_list.argv[i], &ds_info, 1);      
            ds_value = cdb_int_str2val(ds_info.value[DS_PVLAN_COMMUNITY_FLD_KEY], &rc);
            sal_printf("%-10d%-10d%-15s", value, ds_value , "community"); 
            cfg_get_parse_referlist(ds_info.value[DS_PVLAN_COMMUNITY_FLD_PORTS], FALSE, &refer_ds_list);
            mem_count = 0;
            for(k = 0; k <refer_ds_list.argc; k++)
            {
                PM_E_RETURN(cdbclt_get_tbl_line(TBL_INTERFACE, refer_ds_list.argv[k], sal_strlen(refer_ds_list.argv[k]), intf_line, FILE_READ_LINE_LEN));
                cfg_get_parse(intf_line, &intf_info, 0);

                if (0 != sal_strcmp("{}", intf_info.value[TBL_INTERFACE_FLD_LAG]) 
                    && 0 != sal_strcmp("LAG", intf_info.value[TBL_INTERFACE_FLD_HW_TYPE]))
                {
                    /* bypass LAG member port */
                    continue;
                }
    
                if(0 != mem_count && 0 == mem_count % 4)
                {
                    sal_printf( "\n");
                    for(j = 0; j < 35; j++)sal_printf( " ");
                }
                sal_printf( "%-10s", refer_ds_list.argv[k]);
                mem_count++;
            }
            sal_printf("\n");
       }
    }

    return PM_E_NONE;
}

int32
cdbctl_show_pvlan(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    uint32 banner = TRUE;

     sal_printf("%-10s%-10s%-15s%-10s\n", "Primary", "Secondary","Type","Ports");
     sal_printf("---------+---------+--------------+-------------------------------------------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbctl_show_pvlan_one(line, &banner);
    }

    return PM_E_NONE;
}

int32
cdbctl_show_fdb(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];

    sal_printf("            Mac Address Table\n");
    sal_printf("----------------------------------------------\n");
    sal_printf("(*) - Security Entry\n");
    sal_printf("%-7s %-16s %-12s %-8s\n", "VLAN ID", "MAC Address", "Type", "Port");
    sal_printf("-------+----------------+------------+--------\n");
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbctl_show_fdb_one(line);
    }

    return PM_E_NONE;
}


int32
cdbctl_show_psfdb_current_port_count(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
#define STATIC_CNT_ARGV         0


    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];
    int32 static_cnt = 0;
    int32 rc = PM_E_NONE;

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        static_cnt          = cdb_int_str2val(info.value[STATIC_CNT_ARGV], &rc);
        if (rc < 0)
        {
            return PM_E_NONE;
        }

        sal_printf("Current MAC addresses  : %d\n", static_cnt);
    }
    return PM_E_NONE;
}

int32
cdbctl_show_psfdb_interface(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
#define STATIC_PS_ENABLE         0
#define STATIC_PS_VOILATION      1
#define STATIC_PS_MAC_MAXNIUM    2
#define STATIC_PS_TOTAL_MAC_NUM    3
#define STATIC_PS_STATIC_CONFIGURED_MAC_NUM 4

    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];
    int32 static_ps_enable = 0;
    int32 static_ps_violation = 0;
    int32 static_ps_mac_maxnimum = 0;
    int32 static_ps_total_mac_num = 0;
    int32 static_ps_static_configured_mac_num = 0;
    int32 rc = PM_E_NONE;

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        static_ps_enable          = cdb_int_str2val(info.value[STATIC_PS_ENABLE], &rc);
        if (rc < 0)
        {
            return PM_E_NONE;
        }
        static_ps_violation = cdb_int_str2val(info.value[STATIC_PS_VOILATION], &rc);
        if (rc < 0)
        {
            return PM_E_NONE;
        }
        static_ps_mac_maxnimum         = cdb_int_str2val(info.value[STATIC_PS_MAC_MAXNIUM], &rc);
        if (rc < 0)
        {
            return PM_E_NONE;
        }
        static_ps_total_mac_num         = cdb_int_str2val(info.value[STATIC_PS_TOTAL_MAC_NUM], &rc);
        if (rc < 0)
        {
            return PM_E_NONE;
        }
        static_ps_static_configured_mac_num= cdb_int_str2val(info.value[STATIC_PS_STATIC_CONFIGURED_MAC_NUM], &rc);
        if (rc < 0)
        {
            return PM_E_NONE;
        }
        
        sal_printf("Port security                    : %s\n", (static_ps_enable)?"enabled":"disabled");
        if(static_ps_violation == GLB_PORT_SECURITY_VIOLATION_MODE_PROTECT)
             sal_printf("Violation mode                   : %5s\n", "discard packet silence" );
        else if(static_ps_violation == GLB_PORT_SECURITY_VIOLATION_MODE_RESTRICT)
             sal_printf("Violation mode                   : %5s\n", "discard packet and log" );
        else 
             sal_printf("Violation mode                   : %5s\n", "discard packet, log and set interface error-disabled" );
        
        sal_printf("Maximum MAC addresses            : %d\n",  static_ps_mac_maxnimum);
        sal_printf("Total MAC addresses              : %d\n",  static_ps_total_mac_num);
        sal_printf("Static configured MAC addresses  : %d\n",  static_ps_static_configured_mac_num);
    }
    return PM_E_NONE;
}


int32
cdbctl_show_psfdb_status_one(char *line)
{
#define STATIC_PS1_IFINDEX         0
#define STATIC_PS1_MAC_MAXNIUM    1
#define STATIC_PS1_CURRENT_MAC_NUM    2
#define STATIC_PS1_VIOLATION  3

    int32 static_ps_ifindex = 0;
    int32 static_ps_mac_maxnimum = 0;
    int32 static_ps_current_mac_num = 0;
    int32 static_ps_violation = 0;
    cfg_parse_get_info_t info;
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];
    char ifname[IFNAME_SIZE];

    cfg_get_parse(line, &info, 0);
    static_ps_ifindex          = cdb_int_str2val(info.value[STATIC_PS1_IFINDEX], &rc);
    if (rc < 0)
    {
        return PM_E_NONE;
    }
    static_ps_mac_maxnimum = cdb_int_str2val(info.value[STATIC_PS1_MAC_MAXNIUM], &rc);
    if (rc < 0)
    {
        return PM_E_NONE;
    }
    static_ps_current_mac_num         = cdb_int_str2val(info.value[STATIC_PS1_CURRENT_MAC_NUM], &rc);
    if (rc < 0)
    {
        return PM_E_NONE;
    }
    static_ps_violation         = cdb_int_str2val(info.value[STATIC_PS1_VIOLATION], &rc);
    if (rc < 0)
    {
        return PM_E_NONE;
    }

    ifname[0] = CMD_ZERO_CHAR;
    tbl_interface_get_name_by_ifindex(static_ps_ifindex, ifname, IFNAME_SIZE);
    IFNAME_ETH2SHORT(ifname, ifname_ext);

    sal_printf("  %-10s  %7d        %6d       ", ifname_ext, static_ps_mac_maxnimum, static_ps_current_mac_num);

    if(GLB_PORT_SECURITY_VIOLATION_MODE_PROTECT == static_ps_violation)
        sal_printf("protect\n");
    else if(GLB_PORT_SECURITY_VIOLATION_MODE_RESTRICT == static_ps_violation)
        sal_printf("restrict\n");
    else if(GLB_PORT_SECURITY_VIOLATION_MODE_SHUTDOWN == static_ps_violation)
        sal_printf("shutdown\n");

    return PM_E_NONE;
}


int32
cdbctl_show_psfdb_status(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    uint32 enable_port_security = FALSE;
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        if(FALSE == enable_port_security)
        {
            sal_printf("Secure Port  MaxSecureAddr  CurrentAddr  SecurityViolationMode\n");
            sal_printf("               (Count)        (Count)   \n");
            sal_printf("---------------------------------------------------------------\n");
        }
        cdbctl_show_psfdb_status_one(line);
        enable_port_security = TRUE;
    }
    if(FALSE == enable_port_security)
    {
        sal_printf("%% No port security in system.\n");
    }

    return PM_E_NONE;
}


int32
cdbctl_show_psfdb(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];

    sal_printf("        Secure Mac Address Table\n");
    sal_printf("----------------------------------------------\n");
    sal_printf("%-7s %-16s %-16s %-8s\n", "VLAN ID", "MAC Address", "Type", "Port");
    sal_printf("-------+----------------+------------+--------\n");
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbctl_show_psfdb_one(line);
    }

    return PM_E_NONE;
}

int32
cdbctl_show_macfilter(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];
    int32 curr = 0;
    int32 max = 0;
    int32 rc = PM_E_NONE;

    rc = cdbclt_get_tbl_line(TBL_SYS_SPEC, NULL, 0, line, FILE_READ_LINE_LEN);
    rc = cfg_get_parse(line, &info, 0);

    max = cdb_int_str2val(info.value[TBL_SYS_SPEC_FLD_MAC_FILTER], &rc);
    sal_printf("  Mac Filter Address Table\n");
    sal_printf("----------------------------\n");
    sal_printf("%-14s\n", "MAC Address");
    sal_printf("----------------------------\n");
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbctl_show_macfilter_one(line);
        curr++;
    }

    sal_printf("----------------------------\n");
    sal_printf(" Current count  : %d\n", curr);
    sal_printf(" Max count      : %d\n", max);
    sal_printf(" Left count     : %d\n", (max - curr));
    sal_printf("----------------------------\n");

    return PM_E_NONE;
}

int32
cdbctl_show_brg_global(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char *field_str = (char*)p_data;
    
    if (NULL == field_str)
    {
        return PM_E_FAIL;
    }
    if (sal_strstr(field_str, "oper_fdb_aging_time"))
    {
        return _cdbctl_show_brg_fdb_aging_time(p_data, data_len, fp, p_info);
    }
    
    return PM_E_NONE;
}

int32
cdbctl_show_lag_global(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char *field_str = (char*)p_data;
    
    if (NULL == field_str)
    {
        return PM_E_FAIL;
    }
    if (sal_strstr(field_str, "sysid"))
    {
        cdbctl_show_lag_global_sysid(p_data, data_len, fp, p_info);
    }
    if (sal_strstr(field_str, "load-balance"))
    {
        cdbctl_show_lag_global_load_balance(p_data, data_len, fp, p_info);
    }
    
    return PM_E_NONE;
}

