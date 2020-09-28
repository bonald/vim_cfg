
#include "proto.h"
#include "version.h"
#include "cdbclient.h"
#include "glb_const.h"
#include "glb_tempfile_define.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_version_define.h"
#include "gen/tbl_manage_if_define.h"
#include "gen/tbl_bootimage_define.h"
#include "gen/tbl_card_define.h"
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_mirror_define.h"
#include "gen/tbl_mirror.h"
#include "gen/tbl_tap_group_define.h"
#include "gen/tbl_tap_group.h"
#include "gen/tbl_mem_summary_define.h"
#include "gen/tbl_mem_summary.h"
#include "lib_image.h"
#include "gen/tbl_fiber_define.h"
#include "gen/tbl_fiber.h"
#include "gen/tbl_log_global_define.h"
#include "gen/tbl_log_define.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_fan_define.h"
#include "gen/tbl_fan.h"
#include "gen/tbl_psu_define.h"
#include "gen/tbl_psu.h"
#include "gen/tbl_sensor_define.h"
#include "gen/tbl_sensor.h"
#include "gen/tbl_reboot_info_define.h"
#include "gen/tbl_reboot_info.h"
#include "gen/tbl_ssh_cfg_define.h"
#include "gen/tbl_snmp_cfg_define.h"

#define _IS_BIT_SET(flag, bit)   (((flag) & (1 << (bit))) ? 1 : 0)
#define IPV4_MASK_TO_LEN(mask, len) \
    { \
        int32 i; \
        len = 0; \
        for (i = 31; i >= 0; i--) \
        { \
            if (_IS_BIT_SET((mask), (uint32)i)) \
            {(len)++; } \
            else \
            {; } \
        } \
    }

static int32
_cdbctl_show_bootimage_bootimage(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    cdbclt_get_tbl_line(TBL_VERSION, NULL, 0, line, FILE_READ_LINE_LEN);
    cfg_get_parse(line, &info, 0);
    
    sal_printf("Current boot image version: %s-%s\n",
        info.value[TBL_VERSION_FLD_PRODUCT], info.value[TBL_VERSION_FLD_VERSION]);
    return lib_image_show_boot_image();
}

static int32
_cdbctl_show_bootimage_boot(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;

    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse_maxargc(line, &info, 0, TBL_BOOTIMAGE_FLD_MAX);
        /* for bug 47544, check boot image when boot from flash */
        char szFullName[M_FULLPATH_MAX_LEN + 1];
        char* pFileName = NULL;
        int32 rc = PM_E_NONE;
        if ((0 != sal_strcmp(info.value[TBL_BOOTIMAGE_FLD_MODE], "tftp:"))
            &&(0 != sal_strcmp(info.value[TBL_BOOTIMAGE_FLD_MODE], "tftp6:")))
        {
            szFullName[0] = CMD_ZERO_CHAR;
            szFullName[M_FULLPATH_MAX_LEN] = CMD_ZERO_CHAR;
            if ((rc = readlink(M_CUR_BOOT_IMAGE_LINK_FULLNAME,szFullName, M_FULLPATH_MAX_LEN)) < 0)
            {
                sal_printf("%% Can't read boot configuration file: %s\n", sal_strerror(errno));
                return PM_E_FAIL;
            }
            szFullName[rc] = CMD_ZERO_CHAR;
            if ((pFileName = sal_strrchr(szFullName, '/')) == NULL)
            {
                pFileName = szFullName;
            }
            else
            {
                pFileName++;
                if (CMD_ZERO_CHAR == pFileName)
                {
                    sal_printf("%% Invalid boot configuration file.\n");
                    return PM_E_FAIL;
                }
            }            
        }
        
        sal_printf("The current boot image version is: %s-%s\n", "CNOS", OSP_VERSION);
        sal_printf("The current running image is: %s\n", info.value[TBL_BOOTIMAGE_FLD_CURRENT_IMAGE]);
        if (0 == sal_strcmp(info.value[TBL_BOOTIMAGE_FLD_MODE], "tftp:"))
        {
            sal_printf("The next running image is: tftp://%s/%s\n", 
                info.value[TBL_BOOTIMAGE_FLD_SERVERIP], info.value[TBL_BOOTIMAGE_FLD_IMAGE]);
        }
        else if (0 == sal_strcmp(info.value[TBL_BOOTIMAGE_FLD_MODE], "tftp6:"))
        {
            sal_printf("The next running image is: tftp6://%s/%s\n", 
                info.value[TBL_BOOTIMAGE_FLD_SERVERIP], info.value[TBL_BOOTIMAGE_FLD_IMAGE]);
        }        
        else
        {
            if (lib_image_check_image_file(M_CUR_BOOT_IMAGE_LINK_FULLNAME, 0, 0, 0) != 0)
            {
                sal_printf("%% Can't read the next boot image file.\n");
                return PM_E_FAIL;
            }
            sal_printf("The next running image is: flash:/%s/%s\n", 
                "boot", info.value[TBL_BOOTIMAGE_FLD_IMAGE]);
            if (0 == sal_strcmp(info.value[TBL_BOOTIMAGE_FLD_FAST], "1"))
            {
                sal_printf("The next running mode is fast boot\n"); 
            }
        }
    }

    return PM_E_NONE;
}


static int32
_cdbctl_show_sys_global_routemac(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    char *route_mac = NULL;

    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        route_mac = info.value[TBL_SYS_GLOBAL_FLD_ROUTE_MAC];
        sal_printf("  Route MAC is: %-s\n", route_mac);        
    }

    return PM_E_NONE;
}

static void 
_mac_addr_add(uint8* mac, uint32 cnt)
{
    uint32_t value;
    uint32_t val_high, val_low;

    val_high = (mac[0]<<16)+(mac[1]<<8)+mac[2];
    val_low = (mac[3]<<16)+(mac[4]<<8)+mac[5];
    value = val_low + cnt;
    val_low = value & 0xffffff;
    if(value > 0xffffff)
    {
        val_high = val_high+1;
    }
    mac[0] = (val_high >> 16) & 0xff;
    mac[1] = (val_high >> 8) & 0xff;
    mac[2] = val_high & 0xff;
    mac[3] = (val_low >> 16) & 0xff;
    mac[4] = (val_low >> 8) & 0xff;
    mac[5] = val_low & 0xff;
}

static int32
_cdbctl_show_sys_global_systemmac(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    char *mac_str = NULL;
    char *mac_num_str = NULL;
    char buf[MAC_ADDR_STR_LEN];
    mac_addr_t mac;
    int32 mac_num = 0;
    int32 i = 0;
    int32 rc = PM_E_NONE;

    sal_memset(mac, 0, sizeof(mac));
    
    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        mac_str = info.value[TBL_SYS_GLOBAL_FLD_ROUTE_MAC];
        mac_num_str = info.value[TBL_SYS_GLOBAL_FLD_MAC_NUM];
        cdb_mac_str2val(mac, mac_str);
        mac_num = cdb_int_str2val(mac_num_str, &rc);

        sal_printf("%-7s %-14s\n", "Index", "System MAC");
        for (i = 0; i < mac_num; i++)
        {
            sal_sprintf(buf, "%.04hx.%.04hx.%.04hx", sal_ntoh16(((uint16*)mac)[0]),
                                                     sal_ntoh16(((uint16*)mac)[1]), 
                                                     sal_ntoh16(((uint16*)mac)[2]));
            sal_printf("mac%-4d %-14s\n", i, buf);
            //cdb_mac_val2str(buf, MAC_ADDR_STR_LEN, mac)
            _mac_addr_add(mac, 1);
        }
    }

    return PM_E_NONE;
}

static int32
_cdbctl_show_sys_global_services(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    char  ssh_cfg_line[FILE_READ_LINE_LEN];
    char  snmp_cfg_line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info, ssh_info, snmp_info;
    int nValue;
    int32 rc = PM_E_NONE;

    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);      

        sal_printf("Networking services configuration:\n");
        sal_printf("%-16s %-13s %-10s %-10s %s\n", "Service Name", "Status","Port","Protocol", "Service ACL");
        sal_printf("----------------+-------------+----------+----------+------------\n");
#ifndef TAPPRODUCT		
        nValue = atoi(info.value[TBL_SYS_GLOBAL_FLD_DHCP_SERVICE_ENABLE]);
        sal_printf("%-16s %-13s %-10s %-10s %s\n", "dhcp", enable_str_lower(nValue), "67/68", "UDP", "-");
#endif		
        nValue = atoi(info.value[TBL_SYS_GLOBAL_FLD_HTTP_DISABLE]);
        sal_printf("%-16s %-13s %-10s %-10s %s\n", "http", enable_str_lower(!nValue), info.value[TBL_SYS_GLOBAL_FLD_HTTP_PORT], "TCP", "-");
        nValue = atoi(info.value[TBL_SYS_GLOBAL_FLD_HTTPS_DISABLE]);
        sal_printf("%-16s %-13s %-10s %-10s %s\n", "https", enable_str_lower(!nValue), info.value[TBL_SYS_GLOBAL_FLD_HTTPS_PORT], "TCP", "-");
        nValue = atoi(info.value[TBL_SYS_GLOBAL_FLD_RPCAPI_DISABLE]);
        if (nValue)
        {
            sal_printf("%-16s %-13s %-10s %-10s %s\n", "rpc-api", enable_str_lower(!nValue), "-", "TCP", "-");        
        }
        else
        {
            sal_printf("%-16s %-13s %-10s %-10s %s\n", "rpc-api", enable_str_lower(!nValue), info.value[TBL_SYS_GLOBAL_FLD_RPCAPI_PORT], "TCP", "-");        
        }
        
        nValue = atoi(info.value[TBL_SYS_GLOBAL_FLD_TELNET_DISABLE]);
        if (0 == sal_strlen(info.value[TBL_SYS_GLOBAL_FLD_TELNET_ACL]))
        {
            sal_printf("%-16s %-13s %-10s %-10s %s\n", "telnet", enable_str_lower(!nValue), info.value[TBL_SYS_GLOBAL_FLD_TELNET_PORT], "TCP", "-");
        }
        else
        {
            sal_printf("%-16s %-13s %-10s %-10s %s\n", "telnet", enable_str_lower(!nValue), 
                       info.value[TBL_SYS_GLOBAL_FLD_TELNET_PORT], "TCP", info.value[TBL_SYS_GLOBAL_FLD_TELNET_ACL]);
        }
    }
    PM_E_RETURN(cdbclt_get_tbl_line(TBL_SSH_CFG, NULL, 0, ssh_cfg_line, FILE_READ_LINE_LEN));
    cfg_get_parse(ssh_cfg_line, &ssh_info, 0);
    nValue = atoi(ssh_info.value[TBL_SSH_CFG_FLD_ENABLE]);
    if (0 == sal_strlen(info.value[TBL_SYS_GLOBAL_FLD_SSH_ACL]))
    {
        sal_printf("%-16s %-13s %-10s %-10s %s\n", "ssh", enable_str_lower(nValue), ssh_info.value[TBL_SSH_CFG_FLD_LISTEN_PORT], "TCP", "-");
    }
    else
    {
        sal_printf("%-16s %-13s %-10s %-10s %s\n", "ssh", enable_str_lower(nValue), 
            ssh_info.value[TBL_SSH_CFG_FLD_LISTEN_PORT], "TCP", info.value[TBL_SYS_GLOBAL_FLD_SSH_ACL]);
    }
#ifdef HAVE_NETCONF
    nValue = atoi(ssh_info.value[TBL_SSH_CFG_FLD_NETCONF_SERVICE]);
    sal_printf("%-16s %-13s %-10s %-10s %s\n", "netconf", enable_str_lower(nValue), ssh_info.value[TBL_SSH_CFG_FLD_NETCONF_PORT], "TCP", "-");
#endif /* !HAVE_NETCONF */

    PM_E_RETURN(cdbclt_get_tbl_line(TBL_SNMP_CFG, NULL, 0, snmp_cfg_line, FILE_READ_LINE_LEN));
    cfg_get_parse(snmp_cfg_line, &snmp_info, 0);
    nValue = atoi(snmp_info.value[TBL_SNMP_CFG_FLD_SERVER_ENABLE]);
    sal_printf("%-16s %-13s %-10s %-10s %s\n", "snmp", enable_str_lower(nValue), "161", "UDP", "-");

    return PM_E_NONE;
}

static int32
_cdbctl_show_sys_global_services_rpcapi(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    int32 nValue = 0;
    int32 nValue1 = 0;
    int32 enable = 0;    
    uint32 counter_execute = 0;
    uint32 counter_deny = 0;
    FILE *counter_fp = NULL;

    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);      

        sal_printf("RPC-API service configuration:\n");
        nValue = atoi(info.value[TBL_SYS_GLOBAL_FLD_RPCAPI_DISABLE]);
        enable = !nValue;
        sal_printf(" Server State        : %s\n", enable_str_lower(!nValue));
        sal_printf(" Port                : %s\n", info.value[TBL_SYS_GLOBAL_FLD_RPCAPI_PORT]);
        nValue = atoi(info.value[TBL_SYS_GLOBAL_FLD_RPCAPI_AUTH_MODE]);
        nValue1 = atoi(info.value[TBL_SYS_GLOBAL_FLD_OPER_RPCAPI_AUTH_MODE]);
        if (nValue == nValue1)
        {
            sal_printf(" Authentication Mode : %s\n", (nValue) ? "basic" : "none");
        }
        else
        {
            sal_printf(" Authentication Mode : %s (Current effective is %s)\n", (nValue) ? "basic" : "none", (nValue1) ? "basic" : "none");
        }
        nValue = atoi(info.value[TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_EN]);
        if (nValue)
        {
            sal_printf(" SSL State           : %s\n", enable_str_lower(nValue));
            sal_printf(" SSL Port            : %s\n", info.value[TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_PORT]);
        }
        else
        {
            sal_printf(" SSL State           : %s\n", enable_str_lower(nValue));
        }

        if (enable)
        {
            counter_fp = sal_fopen(RPCAPI_COUNTER_FILE, "r");
            if (counter_fp)
            {
                while(sal_fgets(line, CMD_BUF_1024, counter_fp) != NULL)
                {
                    sal_sscanf(line, "%u %u", &counter_execute, &counter_deny);
                }
                sal_fclose(counter_fp);
            }
        }
        sal_printf(" Message Execute     : %u\n", counter_execute);
        sal_printf(" Message Deny        : %u\n", counter_deny);
    }
    
    return PM_E_NONE;
}

int32 _cdbctl_show_sys_global_aaa_status(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    char *aaa_enable           = NULL;

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);

        aaa_enable           = info.value[TBL_SYS_GLOBAL_FLD_AAA_NEW_MODEL];

        sal_printf("AAA status:\n");
        if (0 == sal_strcmp(aaa_enable, "1"))
        {
            sal_printf("    Authentication enable\n");
            /* modified by liwh for bug 47410, 2018-06-05 */
            sal_printf("    Authorization  enable\n");
            sal_printf("    Accounting     enable\n");
            /* liwh end */
        }
        else
        {
            sal_printf("    Authentication disable\n");
            /* modified by liwh for bug 47410, 2018-06-05 */
            sal_printf("    Authorization  disable\n");
            sal_printf("    Accounting     disable\n");
            /* liwh end */
        }

        sal_printf("\n");
    }

    return PM_E_NONE;
}

int32 _cdbctl_show_sys_global_aaa_privilege_mapping(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    char *aaa_enable           = NULL;
    int privilege1 = 0;
    int privilege2 = 0;
    int privilege3 = 0;
    char str1[32] = {0};

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);

        aaa_enable = info.value[TBL_SYS_GLOBAL_FLD_AAA_NEW_MODEL];

        /*added by yejl to fix bug 41141, 2016-12-07*/
        if (0 == sal_strcmp(aaa_enable, "0"))
        {
            sal_printf("%% Please enable aaa first!\n");
            return PM_E_NONE;
        }

        privilege1 = atoi(info.value[TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE1]);
        privilege2 = atoi(info.value[TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE2]);
        privilege3 = atoi(info.value[TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE3]);

        sal_printf("%10s %10s %10s\n", "Server", "Switch", "Server");
        sal_printf("=====================================\n");
        
        //privilege 1
        if (0 == privilege1)
        {
            sal_printf("%10d %10d %10d\n", 0, 1, 0);
        }
        else
        {
            sal_snprintf(str1, 32, "%d~%d", 0, privilege1);
            sal_printf("%10s %10d %10d\n", str1, 1, privilege1);
        }
    
        //privilege 2
        if (privilege2 - privilege1 > 1)
        {
            sal_snprintf(str1, 32, "%d~%d", privilege1 + 1, privilege2);
        }
        else
        {
            sal_snprintf(str1, 32, "%d", privilege2);
        }
        sal_printf("%10s %10d %10d\n", str1, 2, privilege2);
        
        //privilege 3
        if (privilege3 - privilege2 > 1)
        {
            sal_snprintf(str1, 32, "%d~%d", privilege2 + 1, privilege3);
        }
        else
        {
            sal_snprintf(str1, 32, "%d", privilege3);
        }
        sal_printf("%10s %10d %10d\n", str1, 3, privilege3);
    
        //privilege 4
        if (CISCO_PRIVILEGE_MAX - privilege3 > 1)
        {
            sal_snprintf(str1, 32, "%d~%d", privilege3 + 1, CISCO_PRIVILEGE_MAX);
        }
        else
        {
            sal_snprintf(str1, 32, "%d", CISCO_PRIVILEGE_MAX);
        }
        sal_printf("%10s %10d %10d\n", str1, 4, CISCO_PRIVILEGE_MAX);
    }

    return PM_E_NONE;
}

bool first_egress_line = TRUE;
bool first_ingress_line = TRUE;
bool flow_interface = TRUE;

static char tmpstr1_tap[64];
static char tmpstr2_flow[64];
static char tmpstr3_interface[64];

int
cdbctl_get_tap_flow_interface_by_class(char *class_name)
{
    bool isfirst_plus = TRUE;
    int32 tapstrlen = 0;
    int32 tapflowstrlen = 0;
    int32 idx = 0;

    for (idx = 0; idx < sal_strlen(class_name); idx++)
    {
        if (isfirst_plus && ('+' == class_name[idx]))
        {
            tapstrlen = idx;
            isfirst_plus = FALSE;
            continue;
        }
        else if ('+' == class_name[idx])
        { 
            tapflowstrlen = idx;
            break;
        }
    }

    if (0 == tapstrlen || 0 == tapflowstrlen)
    {
        return PM_E_NOT_EXIST;
    }

    sal_memset(tmpstr1_tap, 0, 64);
    sal_memset(tmpstr2_flow, 0, 64);
    sal_memset(tmpstr3_interface, 0, 64);

    sal_strncpy(tmpstr1_tap, class_name, tapstrlen);
    sal_strncpy(tmpstr2_flow, class_name+tapstrlen+1, tapflowstrlen-tapstrlen-1);
    sal_strcpy(tmpstr3_interface, class_name+tapflowstrlen+1);
    return PM_E_NONE;
}

int32 cdbctl_show_mirror(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    uint32 sessionid = 0;
    char *mirror_session_id;
    char *mirror_ingress_port_bmp;
    char *mirror_ingress_vlan_bmp;
    char *mirror_egress_port_bmp;
    char *mirror_egress_vlan_bmp;
    char *mirror_dest_type;
    char *mirror_dest_port;
    char *mirror_dest_vlan;
    port_bmp_t ingress_port_bmp;
    port_bmp_t egress_port_bmp;
    vlan_bmp_t ingress_vlan_bmp;
    vlan_bmp_t egress_vlan_bmp;
    uint32 ifindex = 0;
    uint32 vlanid = 0;
    char if_str[IFNAME_SIZE+1];
    char ifname_ext[IFNAME_SIZE+1];
    int rc = 0;

    CFG_DBG_FUNC();
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);

        mirror_session_id = info.value[TBL_MIRROR_FLD_KEY];
        mirror_ingress_port_bmp = info.value[TBL_MIRROR_FLD_SOURCE_IGS_PORT_BMP]; 
        mirror_egress_port_bmp = info.value[TBL_MIRROR_FLD_SOURCE_EGS_PORT_BMP]; 
        mirror_ingress_vlan_bmp = info.value[TBL_MIRROR_FLD_SOURCE_IGS_VLAN_BMP]; 
        mirror_egress_vlan_bmp = info.value[TBL_MIRROR_FLD_SOURCE_EGS_VLAN_BMP]; 
        mirror_dest_type = info.value[TBL_MIRROR_FLD_DEST_TYPE];
        mirror_dest_port = info.value[TBL_MIRROR_FLD_DEST_PORT];
        mirror_dest_vlan = info.value[TBL_MIRROR_FLD_DEST_VLAN];

        sessionid = cdb_int_str2val(mirror_session_id, &rc);
        if (rc)
        {
            sal_printf("%s\n", "No monitor configuration in the system for session");
            return PM_E_NONE;
        }

        sal_printf("Session   %d\n", sessionid);
        if (0 == sal_strcmp(mirror_dest_type,"MIRROR_DEST_NONE"))
        {
            sal_printf("%-17s: %s\n", "Status", "Invalid");
        }
        else
        {
            sal_printf("%-17s: %s\n", "Status", "Valid");
        }
        sal_printf("%-17s: %s\n", "Type", "Local Session");
        sal_printf("%-17s: %s\n", "Source Ports", "");
    
        GLB_BMP_INIT(ingress_port_bmp);
        GLB_BMP_INIT(egress_port_bmp);
        rc = cdb_bitmap_str2val(mirror_ingress_port_bmp, 0, GLB_AGG_IFINDEX_MIN + GLB_LINKAGG_MAX_GROUP, ingress_port_bmp, GLB_PORT_BMP_WORD_MAX, p_info->buf);
        rc = cdb_bitmap_str2val(mirror_egress_port_bmp, 0, GLB_AGG_IFINDEX_MIN + GLB_LINKAGG_MAX_GROUP, egress_port_bmp, GLB_PORT_BMP_WORD_MAX, p_info->buf);
        GLB_BMP_INIT(ingress_vlan_bmp);
        GLB_BMP_INIT(egress_vlan_bmp);
        rc = cdb_bitmap_str2val(mirror_ingress_vlan_bmp, 0, GLB_VLAN_MAX, ingress_vlan_bmp, GLB_VLAN_BMP_WORD_MAX, p_info->buf);
        rc = cdb_bitmap_str2val(mirror_egress_vlan_bmp, 0, GLB_VLAN_MAX, egress_vlan_bmp, GLB_VLAN_BMP_WORD_MAX, p_info->buf);
        sal_printf("%-17s: %s", "Receive Only", "");
        GLB_BMP_ITER_BEGIN(ingress_port_bmp, ifindex)
        {
            if (!GLB_BMP_ISSET(egress_port_bmp, ifindex))
            {
                rc = tbl_interface_get_name_by_ifindex(ifindex, if_str, IFNAME_SIZE);
                if (rc)
                {
                    continue;
                }
                IFNAME_ETH2SHORT(if_str, ifname_ext);
                sal_printf("%s ", ifname_ext);
            }
        }
        sal_printf("\n");
        GLB_BMP_ITER_END(ingress_port_bmp, ifindex);

        sal_printf("%-17s: %s", "Transmit Only", "");
        GLB_BMP_ITER_BEGIN(egress_port_bmp, ifindex)
        {
            if (!GLB_BMP_ISSET(ingress_port_bmp, ifindex))
            {
                rc = tbl_interface_get_name_by_ifindex(ifindex, if_str, IFNAME_SIZE);
                if (rc)
                {
                    continue;
                }
                IFNAME_ETH2SHORT(if_str, ifname_ext);
                sal_printf("%s ", ifname_ext);
            }
        }
        sal_printf("\n");
        GLB_BMP_ITER_END(egress_port_bmp, ifindex);
        sal_printf("%-17s: %s", "Both", "");
        GLB_BMP_ITER_BEGIN(egress_port_bmp, ifindex)
        {
            if (GLB_BMP_ISSET(ingress_port_bmp, ifindex))
            {
                rc = tbl_interface_get_name_by_ifindex(ifindex, if_str, IFNAME_SIZE);
                if (rc)
                {
                    continue;
                }
                IFNAME_ETH2SHORT(if_str, ifname_ext);
                sal_printf("%s ", ifname_ext);
            }
        }
        sal_printf("\n");
        GLB_BMP_ITER_END(egress_port_bmp, ifindex);

        sal_printf("%-17s: %s", "Source Vlans", "");
        sal_printf("\n");
        sal_printf("%-17s: %s", "Receive Only", "");
        GLB_BMP_ITER_BEGIN(ingress_vlan_bmp, vlanid)
        {
            if (!GLB_BMP_ISSET(egress_vlan_bmp, vlanid))
            {
                sal_printf("%d    ", vlanid);
            }
        }
        sal_printf("\n");
        GLB_BMP_ITER_END(ingress_vlan_bmp, vlanid);

        sal_printf("%-17s: %s", "Transmit Only", "");
        GLB_BMP_ITER_BEGIN(egress_vlan_bmp, vlanid)
        {
            if (!GLB_BMP_ISSET(ingress_vlan_bmp, vlanid))
            {
                sal_printf("%d    ", vlanid);
            }
        }
        sal_printf("\n");
        GLB_BMP_ITER_END(egress_vlan_bmp, vlanid);

        sal_printf("%-17s: %s", "Both", "");
        GLB_BMP_ITER_BEGIN(egress_vlan_bmp, vlanid)
        {
            if (GLB_BMP_ISSET(ingress_vlan_bmp, vlanid))
            {
                sal_printf("%d    ", vlanid);
            }
        }
        sal_printf("\n");
        GLB_BMP_ITER_END(egress_vlan_bmp, vlanid);
    
        ifindex = cdb_int_str2val(mirror_dest_port, &rc);
        if (rc)
        {
            return rc;
        }

        vlanid = cdb_int_str2val(mirror_dest_vlan, &rc);
        if (rc)
        {
            return rc;
        }

        if (0 == sal_strcmp(mirror_dest_type,"MIRROR_DEST_PORT"))
        {
            rc = tbl_interface_get_name_by_ifindex(ifindex, if_str, IFNAME_SIZE);
            if (rc)
            {
                 return rc;
            }
            IFNAME_ETH2SHORT(if_str, ifname_ext);
            sal_printf("%-17s: %s\n", "Destination Port", ifname_ext);
        }
        else if (0 == sal_strcmp(mirror_dest_type,"MIRROR_DEST_VLAN"))
        {
            rc = tbl_interface_get_name_by_ifindex(ifindex, if_str, IFNAME_SIZE);
            if (rc)
            {
                 return rc;
            }
            IFNAME_ETH2SHORT(if_str, ifname_ext);
            sal_printf("%-17s: %s\n", "Destination Port", ifname_ext);
            sal_printf("%-17s: %d\n", "Destination remote VLAN ", vlanid);
        }
        else
        {
            sal_printf("%-17s: %s\n", "Destination Port", "N/A");
        }
    }

    return PM_E_NONE;
}

int32
_cdbctl_show_version(cfg_parse_get_info_t *info, cfg_parse_get_info_t *card_info)
{
    char  line[FILE_READ_LINE_LEN];
    char  debian_version[CMD_BUF_256];
    char  bootimage_line[FILE_READ_LINE_LEN];
    char  hostname[HOSTNAME_SIZE+1];
    struct sal_timeval tv;
    struct sal_timeval warmboot_tv;
    uint32 days = 0;
    uint32 hours = 0;
    uint32 seconds = 0;
    uint32 warmboot_time = 0;
    int32 warmboot_type = 0;
    int32 rc = PM_E_NONE;
    FILE *fptmp;
    char path[FILE_READ_LINE_LEN];
    char *version = NULL;
    char *company = NULL;
    char *package = NULL;
    char *product = NULL;
    cfg_parse_get_info_t sysglb_info;
    cfg_parse_get_info_t bootimage_info;
    uint32 epld_status = 0;
    
    version = info->value[TBL_VERSION_FLD_VERSION];
    company = info->value[TBL_VERSION_FLD_COMPANY];
    package = info->value[TBL_VERSION_FLD_PACKAGE];
    product = info->value[TBL_VERSION_FLD_PRODUCT];


    rc = cdbclt_get_tbl(TBL_BOOTIMAGE, NULL, 0, path);
    fptmp = sal_fopen(path, "r");
    if (!fptmp)
    {
        return PM_E_FAIL;
    }

    while (NULL != sal_fgets(bootimage_line, FILE_READ_LINE_LEN, fptmp))
    {
        cfg_get_parse_maxargc(bootimage_line, &bootimage_info, 0, TBL_BOOTIMAGE_FLD_MAX);
    }
    sal_fclose(fptmp);
    sal_unlink(path);

    rc = cdbclt_get_tbl_line(TBL_SYS_GLOBAL, NULL, 0, line, FILE_READ_LINE_LEN);
    rc = cfg_get_parse(line, &sysglb_info, 0);
    sal_memset(hostname, 0, sizeof(hostname));
    sal_strncpy(hostname, sysglb_info.value[TBL_SYS_GLOBAL_FLD_HOSTNAME], HOSTNAME_SIZE);
    warmboot_type = cdb_enum_str2val(glb_warmboot_type_strs, GLB_WARMBOOT_TYPE_MAX, sysglb_info.value[TBL_SYS_GLOBAL_FLD_WARMBOOT_TYPE]);
    if (GLB_WARMBOOT_NORMAL != warmboot_type)
    {
        sal_sscanf(sysglb_info.value[TBL_SYS_GLOBAL_FLD_WARMBOOT_TIME], "%u", &warmboot_time);
    }
    
    ctc_task_update_boottime(&tv);
    days = tv.tv_sec / 86400;
    hours = (tv.tv_sec / 3600) % 24;
    seconds = (tv.tv_sec / 60) % 60;
    (void)rc;
    (void)debian_version;
    
#ifdef _CTC_X86_64_
#define DEBIAN_VERSION_PATH     "/etc/version"
    sal_memset(debian_version, 0, sizeof(debian_version));
    fptmp = sal_fopen(DEBIAN_VERSION_PATH, "r");
    if (fptmp)
    {
        sal_fgets(debian_version, FILE_READ_LINE_LEN, fptmp);
        lib_strip_newline(debian_version);
        sal_fclose(fptmp);
    }
    sal_printf("Debian Version %s\n", debian_version);
#endif /* !_CTC_X86_64_ */
    sal_printf("%s Software, %s, Version %s\n", package, product, version);
    sal_printf("Copyright (C) %s. All rights reserved.\n", company);

    sal_printf("The current running image is: %s\n", bootimage_info.value[TBL_BOOTIMAGE_FLD_CURRENT_IMAGE]);
    sal_printf("\n");
    
    sal_printf("%s uptime is %u days, %u hours, %u minutes\n", 
        hostname, days, hours, seconds);
    if (GLB_WARMBOOT_NORMAL != warmboot_type)
    {
        warmboot_tv = tv;
        warmboot_tv.tv_sec -= warmboot_time;            
        days = warmboot_tv.tv_sec / 86400;
        hours = (warmboot_tv.tv_sec / 3600) % 24;
        seconds = (warmboot_tv.tv_sec / 60) % 60;
        sal_printf("%s %s uptime is %u days, %u hours, %u minutes\n", 
            hostname, (GLB_WARMBOOT_FAST == warmboot_type) ? "fastboot" : "warmboot", days, hours, seconds);
    }
    sal_printf("Hardware Type        : %s\n", card_info->value[TBL_CARD_FLD_HARDWARE_NAME]);
    sal_printf("SDRAM size           : %sM\n", card_info->value[TBL_CARD_FLD_DRAM_SIZE]);
    sal_printf("Flash size           : %sM\n", card_info->value[TBL_CARD_FLD_FLASH_SIZE]);
    sal_printf("Hardware Version     : %s\n", card_info->value[TBL_CARD_FLD_HARDWARE_VER]);
    epld_status = cdb_int_str2val(card_info->value[TBL_CARD_FLD_EPLD_STATUS], &rc);
    if(epld_status == GLB_EPLD_STATUS_PRESENT)
    {
        sal_printf("EPLD Version         : %s\n", card_info->value[TBL_CARD_FLD_EPLD_VER]);
    }
    sal_printf("BootRom Version      : %s\n", card_info->value[TBL_CARD_FLD_BOOTROM_VER]);
    sal_printf("System serial number : %s\n", card_info->value[TBL_CARD_FLD_SERIAL_NO]);

    return 0;
}
    
int32
_cdbctl_show_version_cst(cfg_parse_get_info_t *info, cfg_parse_get_info_t *card_info)
{
    sal_printf("Copyright (C) %s. All rights reserved.\n", info->value[TBL_VERSION_FLD_CST_COMPANY]);
    sal_printf("Device Type is %s\n", info->value[TBL_VERSION_FLD_CST_HW_TYPE]);
    sal_printf("Device Info is %s\n", info->value[TBL_VERSION_FLD_CST_HW_INFO]);
    sal_printf("Device serial number is %s\n", (0!=sal_strlen(card_info->value[TBL_CARD_FLD_SERIAL_NO]))
        ? card_info->value[TBL_CARD_FLD_SERIAL_NO]:"Unknown");
    sal_printf("Software Version %s\n", info->value[TBL_VERSION_FLD_CST_SW_VER]);
    sal_printf("Hardware Version is %s\n",info->value[TBL_VERSION_FLD_CST_HW_VER]);
    return 0;
}

int32
cdbctl_show_version(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    char  card_line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t card_info;
    int32 rc = PM_E_NONE;
    uint32 force = FALSE;
    uint32 no_cst = FALSE;
    
    cdbclt_get_tbl_line(TBL_CARD, NULL, 0, card_line, FILE_READ_LINE_LEN);
    cfg_get_parse(card_line, &card_info, 0);

    if (data_len)
    {
        force = TRUE;
    }
    
    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        if (0 == sal_strcmp(info.value[TBL_VERSION_FLD_IS_CST], "0"))
        {
            no_cst = TRUE;
        }         
        if (force || no_cst)
        {
            _cdbctl_show_version(&info, &card_info);
        }
        else
        {
            _cdbctl_show_version_cst(&info, &card_info);
        }
    }

    return rc;
}

int _cdbctl_file_exist(char *filename)
{
    struct stat stat_buf;
    
    if (!stat(filename, &stat_buf))
    {
        return 1;
    }
    return 0;
}
char*
_cdbctl_manage_if_get_ifname()
{
#ifdef _GLB_UML_SYSTEM_
    return "eth99";
#else
    return "eth0";
#endif
}
static int
_cdbctl_get_mgmt_if_linklocal(char *link_local)
{
	char szCmd[CMD_BUF_256] = {0};
    FILE *fp = NULL;
    char buf[CMD_BUF_1024];  
	char * if_name = NULL;
	
	if_name = _cdbctl_manage_if_get_ifname();
	#ifdef _GLB_UML_SYSTEM_
	sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ifconfig %s | grep inet6 | grep link | awk '{print $2\"/\"$4}'>%s", if_name,GLB_CONFIG_MGMT_LINKLOCAL);
	#else
	sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ifconfig %s | grep inet6 | grep Link | awk '{print $3}'>%s", if_name,GLB_CONFIG_MGMT_LINKLOCAL);
	#endif
    system(szCmd);
	if (_cdbctl_file_exist(GLB_CONFIG_MGMT_LINKLOCAL))
    {
        fp = sal_fopen(GLB_CONFIG_MGMT_LINKLOCAL, SAL_OPEN_RO);
        if (NULL != fp)
        {
            while (NULL != sal_fgets(buf, CMD_BUF_1024, fp))
            {
               sal_strncpy(link_local,buf,sal_strlen(buf));
            }
            sal_fclose(fp);
        }
    } 
	return PM_E_NONE;

}

static int
_cdbctl_get_mgmt_if_ipv4_address(char *ipv4_addr)
{
    char szCmd[CMD_BUF_256] = {0};
    FILE *fp = NULL;
    char buf[CMD_BUF_1024];  
    char * if_name = NULL;
	
    if_name = _cdbctl_manage_if_get_ifname();
#ifdef _GLB_UML_SYSTEM_
    sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ip netns exec mgmt ifconfig %s | grep 'inet' | grep 'netmask' | awk '{print $2}'>%s", if_name,GLB_CONFIG_MGMT_IPV4_ADDR);
#else        
    sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ip netns exec mgmt ifconfig %s | grep 'inet addr:' | awk '{print $2}'>%s", if_name,GLB_CONFIG_MGMT_IPV4_ADDR);
#endif
    system(szCmd);
    if (_cdbctl_file_exist(GLB_CONFIG_MGMT_IPV4_ADDR))
    {
        fp = sal_fopen(GLB_CONFIG_MGMT_IPV4_ADDR, SAL_OPEN_RO);
        if (NULL != fp)
        {
            while (NULL != sal_fgets(buf, CMD_BUF_1024, fp))
            {
#ifdef _GLB_UML_SYSTEM_
               sal_strncpy(ipv4_addr, buf, (sal_strlen(buf) -1));
#else
               sal_strncpy(ipv4_addr, &buf[5], (sal_strlen(buf) - 6));
#endif
            }

            sal_fclose(fp);
        }
    } 

    return PM_E_NONE;
}

static int
_cdbctl_get_and_print_mgmt_if_ipv6_address(void)
{
    char szCmd[CMD_BUF_256] = {0};
    FILE *fp = NULL;
    char buf[CMD_BUF_1024];  
    char * if_name = NULL;
    uint8   addr_count = 0;
#ifndef _GLB_UML_SYSTEM_
    char    *div = NULL;
#endif
    char      ipv6_addr[CMD_BUF_64] ={0}; 
	
    if_name = _cdbctl_manage_if_get_ifname();
#ifdef _GLB_UML_SYSTEM_
    sal_snprintf(szCmd, sizeof(szCmd), "ip netns exec mgmt ifconfig %s | grep 'inet6' | grep global |awk '{print $2}'>%s", if_name,GLB_CONFIG_MGMT_IPV6_ADDR);
#else        
    sal_snprintf(szCmd, sizeof(szCmd), "ip netns exec mgmt ifconfig %s | grep 'inet6 addr:' | grep Global | awk '{print $3}'>%s", if_name,GLB_CONFIG_MGMT_IPV6_ADDR);
#endif
    system(szCmd);
    if (_cdbctl_file_exist(GLB_CONFIG_MGMT_IPV6_ADDR))
    {
        fp = sal_fopen(GLB_CONFIG_MGMT_IPV6_ADDR, SAL_OPEN_RO);
        if (NULL != fp)
        {
            while (NULL != sal_fgets(buf, CMD_BUF_1024, fp))
            {
#ifdef _GLB_UML_SYSTEM_
                sal_strncpy(ipv6_addr, buf, (sal_strlen(buf) -1));

                if ('\0' != ipv6_addr[0])
                {
                    sal_printf("  Management IPv6 address is: %s\n", ipv6_addr);
                    addr_count++;
                }
#else
                sal_strncpy(ipv6_addr, buf, (sal_strlen(buf) - 1));

                if ('\0' != ipv6_addr[0])
                {
                    div = sal_strstr(ipv6_addr, "\n");
                    if (div)
                    {
                        div[0] = CMD_ZERO_CHAR;
                    }
                    div = NULL;
                    div = sal_strstr(ipv6_addr, "/");
                    if (div)
                    {
                        div[0] = CMD_ZERO_CHAR;
                    }
                
                    sal_printf("  Management IPv6 address is: %s\n", ipv6_addr);

                    addr_count++;
                }
#endif
             }
 
            if (0 == addr_count)
            {
                sal_printf("  Unbounded at present\n");    
            }

            sal_fclose(fp);
        }
    } 

    return PM_E_NONE;
}

static int
_cdbctl_get_mgmt_if_ipv4_address_mask(char *ipv4_addr)
{
    char szCmd[CMD_BUF_256] = {0};
    FILE *fp = NULL;
    char buf[CMD_BUF_1024];  
    char * if_name = NULL;
	
    if_name = _cdbctl_manage_if_get_ifname();
#ifdef _GLB_UML_SYSTEM_
    sal_snprintf(szCmd, sizeof(szCmd), "ip netns exec mgmt ifconfig %s | grep 'inet' | grep 'netmask' | awk '{print $4}'>%s", if_name, GLB_CONFIG_MGMT_IPV4_ADDR);
#else        
    sal_snprintf(szCmd, sizeof(szCmd), "ip netns exec mgmt ifconfig %s | grep 'inet addr:' | awk '{print $4}'>%s", if_name, GLB_CONFIG_MGMT_IPV4_ADDR);
#endif
    system(szCmd);
    if (_cdbctl_file_exist(GLB_CONFIG_MGMT_IPV4_ADDR))
    {
        fp = sal_fopen(GLB_CONFIG_MGMT_IPV4_ADDR, SAL_OPEN_RO);
        if (NULL != fp)
        {
            while (NULL != sal_fgets(buf, CMD_BUF_1024, fp))
            {
#ifdef _GLB_UML_SYSTEM_
               sal_strncpy(ipv4_addr, buf, (sal_strlen(buf)-1));
#else
               sal_strncpy(ipv4_addr, &buf[5], (sal_strlen(buf) - 6));
#endif
            }

            sal_fclose(fp);
        }
    } 

    return PM_E_NONE;
}

static int
_cdbctl_get_mgmt_if_ipv4_gateway(char *ipv4_addr)
{
    char szCmd[CMD_BUF_256] = {0};
    FILE *fp = NULL;
    char buf[CMD_BUF_1024];  
    char * if_name = NULL;
	
    if_name = _cdbctl_manage_if_get_ifname();
    sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ip netns exec mgmt route -n | grep UG | grep %s | awk '{print $2}'>%s", if_name, GLB_CONFIG_MGMT_IPV4_ADDR);
    system(szCmd);
    
    if (_cdbctl_file_exist(GLB_CONFIG_MGMT_IPV4_ADDR))
    {
        fp = sal_fopen(GLB_CONFIG_MGMT_IPV4_ADDR, SAL_OPEN_RO);
        if (NULL != fp)
        {
            while (NULL != sal_fgets(buf, CMD_BUF_1024, fp))
            {
               sal_strncpy(ipv4_addr, buf, (sal_strlen(buf)-1));
            }

            sal_fclose(fp);
        }
    } 

    return PM_E_NONE;
}

int32
cdbctl_show_manage_if(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    char *div = NULL;
    char gateway[CMD_BUF_64];
    char link_local[CMD_BUF_64];
    char ipv4_addr[CMD_BUF_64] ={0}; 
    char ipv4_addr_mask[CMD_BUF_64] ={0};
    char ipv4_gateway[CMD_BUF_64] ={0};
    uint32   ipv4_mask = 0;
    uint32   ipv4_addr_ip = 0;
    uint32   ipv4_len = 0;
    //char      ipv6_addr[CMD_BUF_64] ={0}; 
#ifdef _GLB_UML_SYSTEM_
    //uint32   ipv6_len = 64;
#endif

    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        if (0 == sal_strcmp(p_info->path,"cdb/sys/manage_if/ipv4"))
        {
            if (TRUE == atoi(info.value[TBL_MANAGE_IF_FLD_DHCLIENT_ENABLE]))
            {
                sal_printf("Management IP address negotiated via DHCP\n");

                /* get mask */
                 _cdbctl_get_mgmt_if_ipv4_address_mask(ipv4_addr_mask);
                sal_inet_pton(AF_INET, ipv4_addr_mask, &ipv4_mask);
                IPV4_MASK_TO_LEN(ipv4_mask, ipv4_len) ;

                /* get ip address */
                _cdbctl_get_mgmt_if_ipv4_address(ipv4_addr);
                sal_inet_pton(AF_INET, ipv4_addr, &ipv4_addr_ip);

                /* display ipv4 addr and mask */
                if ('\0' != ipv4_addr[0])
                {
                    sal_printf("  Management IP address is: %s/%d\n", ipv4_addr, ipv4_len);
                }
                else
                {
                    sal_printf("  Unbounded at present\n");    
                }

                /* get IPV4 gateway */
                _cdbctl_get_mgmt_if_ipv4_gateway(ipv4_gateway);
                if ('\0' != ipv4_gateway[0])
                {
                    sal_printf("  IPv4 Gateway:  %s\n", ipv4_gateway);
                }
                else
                {
                    sal_printf("  IPv4 Gateway:  0.0.0.0\n");     
                }
            }
            else
            {
                if (NULL == sal_strstr(info.value[TBL_MANAGE_IF_FLD_ADDR], "0.0.0.0"))
                {
                    div = sal_strstr(info.value[TBL_MANAGE_IF_FLD_ADDR], CMD_KEY_DIV_STR);
                    if (div)
                    {
                        div[0] = CMD_DIVISION_CHAR;
                    }
                    sal_printf("Management IPv4 address:  %s\n", info.value[TBL_MANAGE_IF_FLD_ADDR]);
                    sal_printf("IPv4 Gateway:  %s\n", info.value[TBL_MANAGE_IF_FLD_GATEWAY]);
                }
                else
                {
                    sal_printf("% No management IP address configured\n");
                }
            }
        }
        if (0 == sal_strncmp(p_info->path,"cdb/sys/manage_if/ipv6",sal_strlen("cdb/sys/manage_if/ipv6")))
        {
            if (TRUE == atoi(info.value[TBL_MANAGE_IF_FLD_DHCLIENTV6_ENABLE]))
            {
                sal_printf("Management IPv6 address negotiated via DHCPv6\n");

                /* get ipv6 address */
                _cdbctl_get_and_print_mgmt_if_ipv6_address();

#if 0
                /* display ipv6 addr and mask */
                if ('\0' != ipv6_addr[0])
                {
#ifdef _GLB_UML_SYSTEM_
                    sal_printf("  Management IPv6 address is: %s\n", ipv6_addr);
#else
                    div = sal_strstr(ipv6_addr, "\n");
                    if (div)
                    {
                        div[0] = CMD_ZERO_CHAR;
                    }
                    div = NULL;
                    div = sal_strstr(ipv6_addr, "/");
                    if (div)
                    {
                        div[0] = CMD_ZERO_CHAR;
                    }
                
                    sal_printf("  Management IPv6 address is: %s\n", ipv6_addr);
#endif
                }
                else
                {
                    sal_printf("  Unbounded at present\n");    
                }
#endif

                _cdbctl_get_mgmt_if_linklocal(link_local);
                div = sal_strstr(link_local, "\n");
                if (div)
                {
                    div[0] = CMD_ZERO_CHAR;
                }
                div = NULL;
                div = sal_strstr(link_local, "/");
                if (div)
                {
                    div[0] = CMD_ZERO_CHAR;
                }
                sal_printf("  Link Local: %s\n", link_local);

                /* IPV6 gateway */
                sal_printf("  IPv6 Gateway:  ::\n");     
            }
            else if(0!= sal_strcmp(info.value[TBL_MANAGE_IF_FLD_ADDR_V6],"#0"))
            {
                div = sal_strstr(info.value[TBL_MANAGE_IF_FLD_ADDR_V6], CMD_KEY_DIV_STR);
                if (div)
                {
                    div[0] = CMD_DIVISION_CHAR;
                }
                _cdbctl_get_mgmt_if_linklocal(link_local);
                sal_printf("Management IPv6 address: %s\n", info.value[TBL_MANAGE_IF_FLD_ADDR_V6]);
                sal_strcpy(gateway,info.value[TBL_MANAGE_IF_FLD_GATEWAY_V6]);
                div = sal_strstr(link_local, "\n");
                if (div)
                {
                    div[0] = CMD_ZERO_CHAR;
                }
                div = NULL;
                div = sal_strstr(link_local, "/");
                if (div)
                {
                    div[0] = CMD_ZERO_CHAR;
                }
                sal_printf("Link Local: %s\n", link_local);
                if (0 == sal_strcmp(info.value[TBL_MANAGE_IF_FLD_GATEWAY_V6],"#0"))
                {
                    sal_printf("IPv6 Gateway: ::\n"); 
                }
                else
                {   
                    div = sal_strstr(gateway, CMD_KEY_DIV_STR);
                    if (div)
                    {
                        div[0] = CMD_ZERO_CHAR;
                    }
                    sal_printf("IPv6 Gateway: %s\n", gateway); 
                }
            }
            else
            {
               sal_printf("% No management IPv6 address configured\n");
            }
        }        
    }
    else
    {
        sal_printf("% No management IP address configured\n");
    }

    return PM_E_NONE;
}

int32
cdbctl_show_bootimage(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    if (data_len)
    {
        return _cdbctl_show_bootimage_bootimage(p_data, data_len, fp, p_info);
    }
    else
    {
        return _cdbctl_show_bootimage_boot(p_data, data_len, fp, p_info);
    }
}

int32
cdbctl_show_sys_global(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char *field_str = (char*)p_data;
    
    if (sal_strstr(field_str, "routemac"))
    {
        return _cdbctl_show_sys_global_routemac(p_data, data_len, fp, p_info);
    }
    else if (sal_strstr(field_str, "systemmac"))
    {
        return _cdbctl_show_sys_global_systemmac(p_data, data_len, fp, p_info);
    }
    else if (sal_strstr(field_str, "services_rpcapi"))
    {
        return _cdbctl_show_sys_global_services_rpcapi(p_data, data_len, fp, p_info);
    }
    else if (sal_strstr(field_str, "services"))
    {
        return _cdbctl_show_sys_global_services(p_data, data_len, fp, p_info);
    }
    else if (sal_strstr(field_str, "aaa_new_model"))
    {
        return _cdbctl_show_sys_global_aaa_status(p_data, data_len, fp, p_info);
    }
    else if (sal_strstr(field_str, "aaa_privilege1"))
    {
        return _cdbctl_show_sys_global_aaa_privilege_mapping(p_data, data_len, fp, p_info);
    }
    return PM_E_FAIL;
}

int32
_cdbctl_show_reboot_info(FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t reboot_info;
    uint32 support_reboot_info = 0;
    uint32 i = 0;
    int32 rc = PM_E_NONE;
    char  line[FILE_READ_LINE_LEN];


    rc = cdbclt_get_tbl_line(TBL_CHASSIS, NULL, 0, line, FILE_READ_LINE_LEN);
    rc = cfg_get_parse(line, &info, 0);
    support_reboot_info = cdb_int_str2val(info.value[TBL_CHASSIS_FLD_SUPPORT_REBOOT_INFO], &rc);
    sal_memset(line, 0, sizeof(line));

    if(support_reboot_info)
    {
        sal_printf("%-10s %-15s %-20s\n","Times","Reboot Type","Reboot Time (UTC)");
        sal_printf("----------+---------------+--------------------\n");
        
        while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
        {  
            cfg_get_parse_maxargc(line, &reboot_info, 0,TBL_REBOOT_INFO_FLD_MAX);
            sal_printf("%-10d %-15s %-20s\n",
            ++i, 
            reboot_info.value[TBL_REBOOT_INFO_FLD_REBOOT_TYPE], 
            reboot_info.value[TBL_REBOOT_INFO_FLD_REBOOT_TIME]);
            sal_memset(line, 0, sizeof(line));                  
        }
    }
    else
    {
        sal_printf("Don't support this CLI\n");
    }
    
    return PM_E_NONE;
}

int32
cdbctl_show_rebootinfo(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    int32 rc = PM_E_NONE; 

    rc = _cdbctl_show_reboot_info(fp, p_info);

    return rc; 
}

typedef struct
{
    double curr[FIBER_VAL_TYPE_MAX2];
    double high_alarm;
    double low_alarm;
    double high_warn;
    double low_warn;
} fiber_values_t;

/*curr_num from 0*/
static char* 
_cdbctl_get_fiber_alarm_flag(fiber_values_t *p_values, int curr_num)
{
    if (p_values->curr[curr_num] >= p_values->high_alarm)
        return "++";
    else if (p_values->curr[curr_num] >= p_values->high_alarm)
        return "+";
    else if (p_values->curr[curr_num] <= p_values->low_alarm)
        return "--";
    else if (p_values->curr[curr_num] <= p_values->low_warn)
        return "-";
    else
        return "";
}

static int32
_cdbctl_get_fiber_values(fiber_values_t *p_values, char **curr, char *high_alarm, char *low_alarm, char *high_warn, char *low_warn)
{
    p_values->high_alarm = 0;
    p_values->low_alarm = 0;
    p_values->high_warn = 0;
    p_values->low_warn = 0;
    int i = 0;
    
    for(i=FIBER_CURRENT_VALUE1; i<FIBER_VAL_TYPE_MAX2; i++)
    {
        p_values->curr[i] = 0;
        sal_sscanf(curr[i], "%lf", &p_values->curr[i]);
    }
    sal_sscanf(high_alarm,  "%lf", &p_values->high_alarm);
    sal_sscanf(low_alarm,   "%lf", &p_values->low_alarm);
    sal_sscanf(high_warn,   "%lf", &p_values->high_warn);
    sal_sscanf(low_warn,    "%lf", &p_values->low_warn);    
        
    return PM_E_NONE;
}

int32
cdbctl_show_transceiver(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t fiber_info;
    cfg_parse_get_info_t parse_info;
    cfg_parse_get_info_t parse_info1;
    int32 i = 0;
    uint16 wave_length = 0;
    int32 length[5] = {0};
    int32 fiber_flag;
    int32 is_copper;
    uint8 channel_num=0;
    uint8 channel_idx = 0;
    uint8 start = 0;
    uint8 end = 0;
    fiber_values_t values_temp;
    fiber_values_t values_vcc;
    fiber_values_t values_cur;
    fiber_values_t values_txpwr;
    fiber_values_t values_rxpwr;    
    char show_name[COMM_NAME_SIZE];
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        rc = cfg_get_parse(line, &fiber_info, 0);   
        sal_printf("\n");
        IFNAME_ETH2FULL(fiber_info.value[TBL_FIBER_FLD_INTERFACE_NAME], ifname_ext);
        sal_printf("Port %s transceiver info: \n", ifname_ext);
        sal_printf("Transceiver Type: %s\n", fiber_info.value[TBL_FIBER_FLD_FIBER_TYPE_NAME]);
        sal_printf(" Transceiver Vendor Name : %s\n", fiber_info.value[TBL_FIBER_FLD_NAME]);
        sal_printf(" Transceiver PN          : %s\n", fiber_info.value[TBL_FIBER_FLD_PN]);
        sal_printf(" Transceiver S/N         : %s\n", fiber_info.value[TBL_FIBER_FLD_SN]);
        
        rc = cfg_get_parse_comma(fiber_info.value[TBL_FIBER_FLD_WAVELENGTH], &parse_info);
        wave_length = (((uint16)cdb_int_str2val(parse_info.argv[0], &rc))<<8) 
                      | (uint16)cdb_int_str2val(parse_info.argv[1], &rc);
        if(wave_length)
        {
            sal_printf("Transceiver Output Wavelength: %u nm\n", wave_length);
        }
        else
        {
            sal_printf("Transceiver Output Wavelength: N/A\n");
        }
        
        rc = cfg_get_parse_comma(fiber_info.value[TBL_FIBER_FLD_LENGTH], &parse_info);
        for(i=0; i<parse_info.argc; i++)
        {
            length[i] = cdb_int_str2val(parse_info.argv[i], &rc);
        }
        rc = cfg_get_parse_comma(fiber_info.value[TBL_FIBER_FLD_FIBER_FLAG], &parse_info);
        fiber_flag = cdb_int_str2val(parse_info.argv[0], &rc);
        rc = cfg_get_parse_comma(fiber_info.value[TBL_FIBER_FLD_IS_COPPER], &parse_info);
        is_copper = cdb_int_str2val(parse_info.argv[0], &rc);

        if(length[0]!=0 || length[1]!=0 || length[2]!=0
            || length[3]!=0 || length[4]!=0)
        {
            /* SFF-8472 */
            if(fiber_flag == E_FIBER_SFP_P)
            {
                sal_printf("Supported Link Type and Length: \n");
                if (length[0])
                {
                    sal_printf("      Link Length for 9/125um single mode fiber: %u km\n", length[0]);
                }
                if (length[1])
                {
                    sal_printf("      Link Length for 9/125um single mode fiber: %u m\n", length[1]*100);
                }    
                if (length[2])
                {
                    sal_printf("      Link Length for 50/125um multi-mode fiber: %u m\n", length[2]*10);
                }          
                if (length[3])
                {
                    sal_printf("      Link Length for 62.5/125um multi-mode fiber: %u m\n", length[3]*10);
                }          
                /* the length of copper cable or active cable is different in units */
                if (length[4])
                {
                    if(sal_strncmp("25GBASE_SR",fiber_info.value[TBL_FIBER_FLD_FIBER_TYPE_NAME],10))
                    {
                        sal_printf("      Link Length for 50/125um multi-mode OM4 fiber: %u m\n", length[4]*10);        
                    }
                    else
                    {
                        sal_printf("      Link Length for copper: %u m\n", length[4]);
                    }
                }
            }
            /* SFF-8636 */
            /* for bug 45607 */
            else if(fiber_flag == E_FIBER_QSFP_P)
            {
                sal_printf("Supported Link Type and Length: \n");
                if (length[0])
                {
                    sal_printf("      Link Length for 9/125um single mode fiber: %u km\n", length[0]);
                }
                if (length[1])
                {
                    sal_printf("      Link Length for 50/125um multi-mode OM3 fiber: %u m\n", length[1]*2);
                }    
                if (length[2])
                {
                    sal_printf("      Link Length for 50/125um multi-mode OM2 fiber: %u m\n", length[2]);
                }          
                if (length[3])
                {
                    sal_printf("      Link Length for 62.5/125um multi-mode OM1 fiber: %u m\n", length[3]);
                }       
                /* the length of copper cable or active cable is different in units */
                if (length[4])
                {
                    if(is_copper)
                    {
                        sal_printf("      Link Length for copper: %u m\n", length[4]);
                    }
                    else
                    {
                        sal_printf("      Link Length for 50/125um multi-mode OM4 fiber: %u m\n", length[4]*2);
                    }
                }
            }
        }

        if(1 == cdb_int_str2val(fiber_info.value[TBL_FIBER_FLD_IS_DETAIL], &rc))
        {
            if(1 != cdb_int_str2val(fiber_info.value[TBL_FIBER_FLD_DDM_SUPPORT], &rc))
            {
                sal_printf("Digital diagnostic is not implemented.\n");
                continue;    
            }
            
            channel_num = cdb_int_str2val(fiber_info.value[TBL_FIBER_FLD_CHANNEL_NUM], &rc);
            channel_idx = cdb_int_str2val(fiber_info.value[TBL_FIBER_FLD_CHANNEL_IDX], &rc);
            if(channel_idx == FIBER_VAL_TYPE_MAX2)
            {
                start = FIBER_CURRENT_VALUE1;
                end = start + channel_num - 1;
            }
            else
            {
                start = channel_idx;
                end = start;
            }
            
            /*Get value*/
            rc = cfg_get_parse_comma(fiber_info.value[TBL_FIBER_FLD_TMPR2], &parse_info);
            rc = cfg_get_parse_comma(fiber_info.value[TBL_FIBER_FLD_TMPR], &parse_info1);
            _cdbctl_get_fiber_values(&values_temp,
                                    parse_info.argv, 
                                    parse_info1.argv[FIBER_HIGH_ALARM_VALUE], 
                                    parse_info1.argv[FIBER_LOW_ALARM_VALUE],
                                    parse_info1.argv[FIBER_HIGH_WARN_VALUE],
                                    parse_info1.argv[FIBER_LOW_WARN_VALUE]);

            rc = cfg_get_parse_comma(fiber_info.value[TBL_FIBER_FLD_VOLTAGE2], &parse_info);
            rc = cfg_get_parse_comma(fiber_info.value[TBL_FIBER_FLD_VOLTAGE], &parse_info1);
            _cdbctl_get_fiber_values(&values_vcc,
                                    parse_info.argv, 
                                    parse_info1.argv[FIBER_HIGH_ALARM_VALUE], 
                                    parse_info1.argv[FIBER_LOW_ALARM_VALUE],
                                    parse_info1.argv[FIBER_HIGH_WARN_VALUE],
                                    parse_info1.argv[FIBER_LOW_WARN_VALUE]);

            rc = cfg_get_parse_comma(fiber_info.value[TBL_FIBER_FLD_BIAS2], &parse_info);
            rc = cfg_get_parse_comma(fiber_info.value[TBL_FIBER_FLD_BIAS], &parse_info1);
            _cdbctl_get_fiber_values(&values_cur,
                                    parse_info.argv, 
                                    parse_info1.argv[FIBER_HIGH_ALARM_VALUE], 
                                    parse_info1.argv[FIBER_LOW_ALARM_VALUE],
                                    parse_info1.argv[FIBER_HIGH_WARN_VALUE],
                                    parse_info1.argv[FIBER_LOW_WARN_VALUE]);

            rc = cfg_get_parse_comma(fiber_info.value[TBL_FIBER_FLD_TX_PWR2], &parse_info);
            rc = cfg_get_parse_comma(fiber_info.value[TBL_FIBER_FLD_TX_PWR], &parse_info1);
            _cdbctl_get_fiber_values(&values_txpwr,
                                    parse_info.argv, 
                                    parse_info1.argv[FIBER_HIGH_ALARM_VALUE], 
                                    parse_info1.argv[FIBER_LOW_ALARM_VALUE],
                                    parse_info1.argv[FIBER_HIGH_WARN_VALUE],
                                    parse_info1.argv[FIBER_LOW_WARN_VALUE]);  
            
            rc = cfg_get_parse_comma(fiber_info.value[TBL_FIBER_FLD_RX_PWR2], &parse_info);
            rc = cfg_get_parse_comma(fiber_info.value[TBL_FIBER_FLD_RX_PWR], &parse_info1);
            _cdbctl_get_fiber_values(&values_rxpwr,
                                    parse_info.argv, 
                                    parse_info1.argv[FIBER_HIGH_ALARM_VALUE], 
                                    parse_info1.argv[FIBER_LOW_ALARM_VALUE],
                                    parse_info1.argv[FIBER_HIGH_WARN_VALUE],
                                    parse_info1.argv[FIBER_LOW_WARN_VALUE]);           
          
            for(i=start; i<=end; i++)
            {
                if(start != end)
                {
                    sal_sprintf(show_name, "%s chan%d", ifname_ext, i+1);
                }
                else
                {
                    sal_sprintf(show_name, "%s", ifname_ext);
                }
            
                sal_printf("-------------------------------------------------------------------------------\n");
                sal_printf("Transceiver is %s calibrated.\n", 
                    cdb_int_str2val(fiber_info.value[TBL_FIBER_FLD_EXTERNALLY_CALIBRATED], &rc)? "externally":"internally");
                sal_printf("mA: milliamperes, dBm: decibels (milliwatts), NA or N/A: not applicable. \n");
                sal_printf("++ : high alarm, +  : high warning, -  : low warning, -- : low alarm. \n");
                sal_printf("The threshold values are calibrated. \n");

                sal_printf("-------------------------------------------------------------------------------\n");
                sal_printf("                                  High Alarm  High Warn   Low Warn    Low Alarm\n");
                sal_printf("                Temperature       Threshold   Threshold   Threshold   Threshold\n");
                sal_printf("Port            (Celsius)         (Celsius)   (Celsius)   (Celsius)   (Celsius)\n");
                sal_printf("---------------+-----------------+-----------+-----------+-----------+---------\n");
                sal_printf("%-16s", show_name);
                sal_printf("%-11.2f",  values_temp.curr[i]);
                sal_printf("%-7s", _cdbctl_get_fiber_alarm_flag(&values_temp, i)); /*flag*/
                sal_printf("%-12.2f%-12.2f%-12.2f%-9.2f\n", 
                    values_temp.high_alarm, values_temp.high_warn, values_temp.low_warn, values_temp.low_alarm);
             
                sal_printf("-------------------------------------------------------------------------------\n");
                sal_printf("                                  High Alarm  High Warn   Low Warn    Low Alarm\n");
                sal_printf("                Voltage           Threshold   Threshold   Threshold   Threshold\n");
                sal_printf("Port            (Volts)            (Volts)     (Volts)     (Volts)     (Volts) \n");
                sal_printf("---------------+-----------------+-----------+-----------+-----------+---------\n");
                sal_printf("%-16s", show_name);
                sal_printf("%-11.2f",  values_vcc.curr[i]);
                sal_printf("%-7s", _cdbctl_get_fiber_alarm_flag(&values_vcc, i)); /*flag*/
                sal_printf("%-12.2f%-12.2f%-12.2f%-9.2f\n", 
                    values_vcc.high_alarm, values_vcc.high_warn, values_vcc.low_warn, values_vcc.low_alarm);           
                
                sal_printf("-------------------------------------------------------------------------------\n");
                sal_printf("                                  High Alarm  High Warn   Low Warn    Low Alarm\n");
                sal_printf("                Current           Threshold   Threshold   Threshold   Threshold\n");
                sal_printf("Port            (milliamperes)      (mA)        (mA)        (mA)        (mA)   \n");
                sal_printf("---------------+-----------------+-----------+-----------+-----------+---------\n");
                sal_printf("%-16s", show_name);
                sal_printf("%-11.2f",  values_cur.curr[i]);
                sal_printf("%-7s", _cdbctl_get_fiber_alarm_flag(&values_cur, i)); /*flag*/
                sal_printf("%-12.2f%-12.2f%-12.2f%-9.2f\n", 
                    values_cur.high_alarm, values_cur.high_warn, values_cur.low_warn, values_cur.low_alarm);
                
                sal_printf("-------------------------------------------------------------------------------\n");
                sal_printf("                Optical           High Alarm  High Warn   Low Warn    Low Alarm\n");
                sal_printf("                Transmit Power    Threshold   Threshold   Threshold   Threshold\n");
                sal_printf("Port             (dBm)              (dBm)       (dBm)       (dBm)       (dBm)  \n");
                sal_printf("---------------+-----------------+-----------+-----------+-----------+---------\n");
                sal_printf("%-16s", show_name);
                sal_printf("%-11.2f",  values_txpwr.curr[i]);
                sal_printf("%-7s", _cdbctl_get_fiber_alarm_flag(&values_txpwr, i)); /*flag*/
                sal_printf("%-12.2f%-12.2f%-12.2f%-9.2f\n", 
                    values_txpwr.high_alarm, values_txpwr.high_warn, values_txpwr.low_warn, values_txpwr.low_alarm);

                sal_printf("-------------------------------------------------------------------------------\n");
                sal_printf("                Optical           High Alarm  High Warn   Low Warn    Low Alarm\n");
                sal_printf("                Receive Power     Threshold   Threshold   Threshold   Threshold\n");
                sal_printf("Port             (dBm)              (dBm)       (dBm)       (dBm)       (dBm)  \n");
                sal_printf("---------------+-----------------+-----------+-----------+-----------+---------\n");
                sal_printf("%-16s", show_name);
                sal_printf("%-11.2f",  values_rxpwr.curr[i]);
                sal_printf("%-7s", _cdbctl_get_fiber_alarm_flag(&values_rxpwr, i)); /*flag*/
                sal_printf("%-12.2f%-12.2f%-12.2f%-9.2f\n", 
                    values_rxpwr.high_alarm, values_rxpwr.high_warn, values_rxpwr.low_warn, values_rxpwr.low_alarm);           
            }
        }
    }
    
    return PM_E_NONE;    
}

int32
cdbctl_show_memory_summary(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info) 
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    int32 used_mem = 0;
    int32 total_mem = 0;
    int32 rc = PM_E_NONE;
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {        
        rc = cfg_get_parse(line, &info, 0);
        used_mem = cdb_int_str2val(info.value[TBL_MEM_SUMMARY_FLD_USED], &rc);
        total_mem = cdb_int_str2val(info.value[TBL_MEM_SUMMARY_FLD_TOTAL], &rc);
        
        sal_printf("%s %s KB\n",  "Total memory      :", info.value[TBL_MEM_SUMMARY_FLD_TOTAL]);
        sal_printf("%s %s KB\n",  "Used memory       :", info.value[TBL_MEM_SUMMARY_FLD_USED]);
        sal_printf("%s %s KB\n",  "Freed memory      :", info.value[TBL_MEM_SUMMARY_FLD_FREE]);
        sal_printf("%s %s KB\n",  "Buffer memory     :", info.value[TBL_MEM_SUMMARY_FLD_BUFFER]);
        sal_printf("%s %s KB\n",  "Cached memory     :", info.value[TBL_MEM_SUMMARY_FLD_CACHED]);
        sal_printf("%s %.2f%%\n", "Memory utilization:", ((float)used_mem/total_mem)*100);
    }

    return rc;
}

int32
cdbctl_show_log_global(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    cfg_result_t rslt;
    logid_bmp_t bmp;
    char *bmp_str = NULL;
    const char *log_str = NULL;
    uint32 enable = 0;
    uint32 i = 0;

    sal_fgets(line, FILE_READ_LINE_LEN, fp);

    cfg_get_parse(line, &info, 0);

    bmp_str = info.value[TBL_LOG_GLOBAL_FLD_LOG_TO_CDB];
    GLB_BMP_INIT(bmp);
    cdb_bitmap_str2val(bmp_str, 0, LOG_ID_MAX, bmp, GLB_PORT_BMP_WORD_MAX, rslt.desc);

    sal_printf("%-8s %-35s %-10s\n", "Log ID", "Description", "CopyToCDB");
    sal_printf("--------+-----------------------------------+----------\n");
    for (i = 0; i < LOG_ID_MAX; i++)
    {
        enable = GLB_BMP_ISSET(bmp, i);
        log_str = logid_get_desc(i);
        sal_printf("%-8u %-35s %-10s\n", i, log_str, enable_str(enable));
    }

    return PM_E_NONE;
}

static int32
_cdbctl_show_log_one(char *line, uint32 *p_banner)
{
    cfg_parse_get_info_t info;
    

    if (*p_banner)
    {
        *p_banner = FALSE;
        sal_printf("%-10s %-6s %-6s %s\n", "Sequence", "Log ID", "Level", "String");
        sal_printf("----------+------+------+-------------------------------------------------------\n");
    }
    
    cfg_get_parse(line, &info, 0);
    sal_printf("%-10s %-6s %-6s %s\n", 
        info.value[TBL_LOG_FLD_KEY], info.value[TBL_LOG_FLD_LOG_ID], 
        info.value[TBL_LOG_FLD_SEVERITY], info.value[TBL_LOG_FLD_DATA]);

    return PM_E_NONE;
}

int32
cdbctl_show_log(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    uint32 banner = TRUE;

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbctl_show_log_one(line, &banner);
    }
    
    return PM_E_NONE;
}

int32
_cdbctl_show_id_led_status(FILE *fp, cfg_parse_info_t *p_info) 
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    int32 rc = PM_E_NONE;
    uint8 on = 0;
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {        
        rc = cfg_get_parse(line, &info, 0);
        on = cdb_int_str2val(info.value[TBL_CHASSIS_FLD_LOCATION_LED], &rc);
        sal_printf("Indicate led is forced %s\n", on?"on":"off");
    }

    return rc;
}
 
int32
cdbctl_show_chassis(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    int32 rc = PM_E_NONE; 

    rc = _cdbctl_show_id_led_status(fp, p_info); 
    
    return rc; 
}        
