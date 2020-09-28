
/****************************************************************************
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Centec
 * Date         :       2015-08-06
 * Reason       :       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "chsm_inc.h"
#include "chsm.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "gen/tbl_clock_define.h"
#include "gen/tbl_clock.h"
#include "gen/tbl_tag_summer_clock_define.h"
#include "gen/tbl_tag_summer_clock.h"
#include "gen/tbl_poe_define.h"
#include "gen/tbl_poe.h"
#include <sys/mount.h>
#include <linux/fs.h>
#include <linux/ethtool.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include "cJSON.h"

/****************************************************************************
 *  
* Defines and Macros
*
*****************************************************************************/
typedef struct tagProcCtrl
{
    char strPIDFile[MAX_PROC_LEN + 1];
    int signal;
}CHSM_ProcCtrl;

CHSM_ProcCtrl  chsm_stProcCtrl[] =
{
    /*{"/var/run/ntpd.pid", SIGHUP},*/
    {"/var/run/syslog-ng.pid", SIGUSR2}
};
#define _CHSM_TBL_VERSION_
int32
chsm_cmd_show_reboot_info(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field);

/****************************************************************************
 *  
* Global and Declaration
*
*****************************************************************************/
static int32
_chsm_cmd_get_version(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_version_t* p_ver = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_ver = tbl_version_get_version();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_version_dump_one(p_ver, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

/*Added by zhush for bug 52768, 2019-07-04*/
static int32
_chsm_cmd_get_version_json(cfg_cmd_para_t *para, cdb_node_t *p_node)
{
    FILE *fp = NULL;
    tbl_version_t* p_ver = NULL;
    tbl_bootimage_t* p_bot = NULL;
    tbl_card_t* p_card = NULL;
    tbl_card_key_t key;
    tbl_sys_global_t* p_sys_gol = NULL;
    struct sal_timeval tv;
    uint32 days = 0;
    uint32 hours = 0;
    uint32 minutes = 0;

    cJSON *slots = NULL;
    cJSON *host_uptime = NULL;
    char str_buf[128] = "";
    char *out = NULL;
	
    p_ver = tbl_version_get_version();
    key.id = 0;
    p_card = tbl_card_get_card(&key);

    p_bot = tbl_bootimage_get_bootimage();
    p_sys_gol = tbl_sys_global_get_sys_global();
    
    ctc_task_update_boottime(&tv);
    days = tv.tv_sec / 86400;
    hours = (tv.tv_sec / 3600) % 24;
    minutes = (tv.tv_sec / 60) % 60;
	
    slots = cJSON_CreateObject();

    if(p_ver->is_cst)
   {
       cJSON_AddStringToObject(slots, "Copyright", p_ver->cst_company);
       cJSON_AddStringToObject(slots, "Device Type", p_ver->cst_hw_type);
       cJSON_AddStringToObject(slots, "Device Info", p_ver->cst_hw_info);
       cJSON_AddStringToObject(slots, "Device serial number", (0!=sal_strlen(p_card->serial_no))?p_card->serial_no:"Unknown");
       cJSON_AddStringToObject(slots, "Software Version", p_ver->cst_sw_ver);
       cJSON_AddStringToObject(slots, "Hardware Version", p_ver->cst_hw_ver);
    }
    else
    {
    	
       cJSON_AddStringToObject(slots, "Software", p_ver->package);
	cJSON_AddStringToObject(slots, "Product", p_ver->product);
	cJSON_AddStringToObject(slots, "Version", p_ver->version);
       cJSON_AddStringToObject(slots, "Copyright", p_ver->company);
	cJSON_AddStringToObject(slots, "The current running image", p_bot->current_image);
	
	host_uptime = cJSON_CreateObject();
	cJSON_AddItemToObject(slots, "host uptime" , host_uptime);
	cJSON_AddStringToObject(host_uptime, "hostname", p_sys_gol->hostname);
	sal_snprintf(str_buf, sizeof(str_buf), "%d", days);
	cJSON_AddStringToObject(host_uptime, "uptime(days)", str_buf);
       sal_snprintf(str_buf, sizeof(str_buf), "%d", hours);
       cJSON_AddStringToObject(host_uptime, "uptime(hours)", str_buf);
       sal_snprintf(str_buf, sizeof(str_buf), "%d", minutes);
       cJSON_AddStringToObject(host_uptime, "uptime(minutes)", str_buf);
	   
	cJSON_AddStringToObject(slots, "Hardware Type", p_card->hardware_type);
       cJSON_AddStringToObject(slots, "Hardware Version", p_card->hardware_ver);
	cJSON_AddNumberToObject(slots, "SDRAM size(M)",  p_card->dram_size);
	cJSON_AddNumberToObject(slots, "Flash size(M)", p_card->flash_size);
	cJSON_AddStringToObject(slots, "EPLD Version", p_card->epld_ver);
	cJSON_AddStringToObject(slots, "BootRom Version", p_card->bootrom_ver);
	cJSON_AddStringToObject(slots, "System serial number", p_card->serial_no);
    }
	
    out = cJSON_Print(slots);

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_fprintf(fp, out);
    sal_fprintf(fp,"\n");
    sal_fclose(fp);

    cJSON_Delete(slots);
    cJSON_free(out);
  
    return PM_E_NONE;
}
/*end zhush*/

int32
chsm_cmd_process_version(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    tbl_version_t ver;
    int32 rc = PM_E_NONE;

    p_node = cdb_get_tbl(TBL_VERSION);

    switch (para->oper)
    {
    case CDB_OPER_GET:
        if ((1 == argc) && !sal_strcmp(argv[0], "force"))
        {
            sal_memset(&field, FLD_ID_ALL, sizeof(field));
        }
        else
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }
        rc = _chsm_cmd_get_version(para, p_node, &field);
        break;
/*Added by zhush for bug 52768, 2019-07-04*/
    case CDB_OPER_GETJSON:
        rc = _chsm_cmd_get_version_json(para, p_node);
        break;
/*end zhush*/
    case CDB_OPER_SET:
        sal_memset(&ver, 0, sizeof(ver));
        sal_strncpy(ver.company, argv[5], OEM_INFO_BUFSIZ);
        sal_strncpy(ver.package, argv[0], OEM_INFO_BUFSIZ);
        sal_strncpy(ver.product, argv[1], OEM_INFO_BUFSIZ);
        sal_strncpy(ver.hw_type, argv[2], OEM_INFO_BUFSIZ);
        
        tbl_version_set_version_field(&ver, TBL_VERSION_FLD_COMPANY);
        tbl_version_set_version_field(&ver, TBL_VERSION_FLD_PACKAGE);
        tbl_version_set_version_field(&ver, TBL_VERSION_FLD_PRODUCT);
        tbl_version_set_version_field(&ver, TBL_VERSION_FLD_HW_TYPE);
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

#define _CHSM_TBL_MANAGE_IF_

#ifdef _KERNEL_VER_2_6_
static int32 manage_if_fd = -1;
#else
#endif

char*
chsm_manage_if_get_ifname()
{
#ifdef _GLB_UML_SYSTEM_
    return "eth99";
#else
    return "eth0";
#endif
}


static int
_chsm_get_mgmt_if_linklocal(char *link_local)
{
	char szCmd[CMD_BUF_256] = {0};
    FILE *fp = NULL;
    char buf[CMD_BUF_1024];  
	char * if_name = NULL;
	
	if_name = chsm_manage_if_get_ifname();
    #ifdef _GLB_UML_SYSTEM_
	sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ifconfig %s | grep inet6 | grep link | awk '{print $2\"/\"$4}'>%s", if_name,GLB_CONFIG_MGMT_LINKLOCAL);
	#else
	sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ifconfig %s | grep inet6 | grep Link | awk '{print $3}'>%s", if_name,GLB_CONFIG_MGMT_LINKLOCAL);
	#endif
    system(szCmd);
	if (syscmd_file_exist(GLB_CONFIG_MGMT_LINKLOCAL))
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

/*
static int
_chsm_del_mgmt_if_linklocal()
{
	char link_local[CMD_BUF_64];
	char * if_name = NULL;
	char szCmd[CMD_BUF_256] = {0};
	if_name = chsm_manage_if_get_ifname();
	_chsm_get_mgmt_if_linklocal(link_local);
	sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX"ip -6 addr del %s dev %s", link_local, if_name);
    system(szCmd);
    return PM_E_NONE;
}
*/

int32
chsm_manage_if_set_addr(tbl_manage_if_t *p_mng_if)
{
    char *ifname = NULL;
    prefix_ipv4_t *p_prefix = &p_mng_if->addr;
    addr_ipv4_t mask;
    addr_ipv4_t broadcast;
    int32 rc = PM_E_NONE;

#ifdef _KERNEL_VER_2_6_
#else
    char cmd_str[MAX_CMD_LINE_LEN];
    char addr_str[INET_ADDRSTRLEN];
    char mask_str[INET_ADDRSTRLEN];
#endif
    
    sal_memset(&mask, 0, sizeof(mask));
    sal_memset(&broadcast, 0, sizeof(broadcast));

    if (p_prefix->prefixlen <= 30)
    {
        addr_mask_len2ip_v4(p_prefix->prefixlen, &mask);
        broadcast.s_addr = p_prefix->prefix.s_addr | ~mask.s_addr;
    }

#ifdef _KERNEL_VER_2_6_
    ifname = chsm_manage_if_get_ifname();
    if (rc = lib_ioctl_manage_if_set_addr(manage_if_fd, ifname, SIOCSIFADDR, p_prefix->prefix) < 0)
    {
        return rc;
    }

    if (0 != p_prefix->prefix.s_addr)
    {
        if (rc = lib_ioctl_manage_if_set_addr(manage_if_fd, ifname, SIOCSIFNETMASK, mask) < 0)
        {
            return rc;
        }
        if (rc = lib_ioctl_manage_if_set_addr(manage_if_fd, ifname, SIOCSIFBRDADDR, broadcast) < 0)
        {
            return rc;
        }
    }
#else
    ifname = chsm_manage_if_get_ifname();
    sal_inet_ntop(AF_INET, &p_prefix->prefix, addr_str, INET_ADDRSTRLEN);
    
    sal_snprintf(cmd_str, MAX_CMD_LINE_LEN, NETNS_CMD_PREFIX" ifconfig %s %s>/dev/null 2>&1", 
            ifname,addr_str);
    rc = system(cmd_str);
    if (rc) 
    {
        return rc;
    }
    
    if (0 != p_prefix->prefix.s_addr)
    {
        sal_inet_ntop(AF_INET, &mask, mask_str, INET_ADDRSTRLEN);
        sal_snprintf(cmd_str, MAX_CMD_LINE_LEN, NETNS_CMD_PREFIX" ifconfig %s netmask %s>/dev/null 2>&1", 
            ifname,mask_str);
        rc = system(cmd_str);
        if (rc) 
        {
            return rc;
        }
    }
#endif

    lib_sysctl_set_intf_arp_miss_by_name(ifname, FALSE);

    rc = tbl_manage_if_set_manage_if_field(p_mng_if, TBL_MANAGE_IF_FLD_ADDR);
    return rc;
}

static int
chsm_get_mgmt_if_linklocal(char *link_local)
{
    char szCmd[CMD_BUF_256] = {0};
    FILE *fp = NULL;
    char buf[CMD_BUF_1024];  
    char * if_name = NULL;
    char *div = NULL;

    if_name = chsm_manage_if_get_ifname();
#ifdef _GLB_UML_SYSTEM_
    sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ifconfig %s | grep inet6 | grep link | awk '{print $2\"/\"$4}'>%s", if_name,GLB_CONFIG_MGMT_LINKLOCAL);
#else
    sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ifconfig %s | grep inet6 | grep Link | awk '{print $3}'>%s", if_name,GLB_CONFIG_MGMT_LINKLOCAL);
#endif
    system(szCmd);
    if (syscmd_file_exist(GLB_CONFIG_MGMT_LINKLOCAL))
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
        else
        {
            log_sys(M_MOD_CHSM, E_ERROR, "Open %s failed\n", GLB_CONFIG_MGMT_LINKLOCAL);
            return PM_E_FAIL;
        }
    }
    else
    {
        log_sys(M_MOD_CHSM, E_ERROR, "File %s not exist\n", GLB_CONFIG_MGMT_LINKLOCAL);
        return PM_E_FAIL;
    }

    div = sal_strstr(link_local, "\n");
    if (div)
    {
        div[0] = CMD_ZERO_CHAR;
    }
    div = NULL;
    //div = sal_strstr(link_local, "/");
    //if (div)
    //{
    //    div[0] = CMD_ZERO_CHAR;
    //}
    
    return PM_E_NONE;
}

/*fix bug 49746 by chenc*/
void
_chsm_set_mgmt_if_disable_ipv6(char *ifname, int32 disable)
{
    char szCmd[CMD_BUF_256] = {0};
    
    sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" sysctl -w net.ipv6.conf.%s.disable_ipv6=%d >/dev/null 2>&1", ifname, disable);
    system(szCmd);
}

int32
chsm_manage_if_set_addr_v6(tbl_manage_if_t *p_mng_if, uint32 is_add)
{
    char *ifname = NULL;
    prefix_t *p_prefix = &p_mng_if->addr_v6;
    int32 rc = PM_E_NONE;
    prefix_t addr_v6;
    char cmd_str[CMD_BUF_256] = {0};
    char addr_str[INET6_ADDRSTRLEN] = {0};
    char szCmd[CMD_BUF_256] = {0};

    tbl_manage_if_t *p_db_mng_if = NULL;
    p_db_mng_if = tbl_manage_if_get_manage_if();
    if (!p_db_mng_if)
    {
        return PM_E_NONE;
    }

    ifname = chsm_manage_if_get_ifname();

    if (is_add)
    {
        sal_memset(&addr_v6, 0x00, sizeof(addr_v6));
        if (0 == sal_memcmp(&p_db_mng_if->addr_v6, &addr_v6, sizeof(addr_v6)))
        {
            //sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ip -6 addr add %s dev %s >/dev/null 2>&1", p_db_mng_if->link_local_addr_str, ifname);
            //system(szCmd);
            _chsm_set_mgmt_if_disable_ipv6(ifname, 0);
        }
    }
    
    sal_inet_ntop(AF_INET6, &p_prefix->u.prefix6, addr_str, INET6_ADDRSTRLEN);
    sal_snprintf(cmd_str, sizeof(cmd_str), NETNS_CMD_PREFIX" ip -6 addr %s %s/%u dev %s >/dev/null 2>&1", 
            is_add ? "add" : "del", addr_str, p_prefix->prefixlen, ifname);
    rc = system(cmd_str);

    if (!is_add)
    {
        sal_memset(szCmd, 0x00, sizeof(szCmd));
        sal_memset(&p_db_mng_if->addr_v6, 0x00, sizeof(p_db_mng_if->addr_v6));
        //sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ip -6 addr del %s dev %s >/dev/null 2>&1", p_db_mng_if->link_local_addr_str, ifname);
        //system(szCmd);
        //_chsm_set_mgmt_if_disable_ipv6(ifname, 1);
    }
    
    if (rc) 
    {
        return rc;
    }

#if 0
    lib_sysctl_set_intf_arp_miss_by_name_v6(ifname, FALSE);
#endif

    return rc;
}

int32
chsm_manage_if_set_gateway(tbl_manage_if_t *p_mng_if, uint32 is_add)
{
    char cmd_str[MAX_CMD_LINE_LEN];
    char addr_str[INET_ADDRSTRLEN];
    int32 rc = PM_E_NONE;

    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_memset(addr_str, 0, sizeof(addr_str));

    sal_inet_ntop(AF_INET, &p_mng_if->gateway, addr_str, INET_ADDRSTRLEN);
    
    if (is_add)
    {
        sal_snprintf(cmd_str, MAX_CMD_LINE_LEN, NETNS_CMD_PREFIX" route add default gw %s >/dev/null 2>&1", 
            addr_str);
    }
    else
    {
        sal_snprintf(cmd_str, MAX_CMD_LINE_LEN, NETNS_CMD_PREFIX" route del default gw %s >/dev/null 2>&1",
            addr_str);
        sal_memset(&p_mng_if->gateway, 0, sizeof(addr_ipv4_t));
    }
    
    rc = system(cmd_str);
    if (rc) 
    {
        return -1;
    }
    
    tbl_manage_if_set_manage_if_field(p_mng_if, TBL_MANAGE_IF_FLD_GATEWAY);

    return rc;
}

int32
chsm_manage_if_set_gateway_v6(tbl_manage_if_t *p_mng_if, uint32 is_add)
{
    char cmd_str[CMD_BUF_256] = {0};
    char addr_str[INET6_ADDRSTRLEN];
    int32 rc = PM_E_NONE;

    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_memset(addr_str, 0, sizeof(addr_str));

    cdb_addr_ipv6_val2str(addr_str, INET6_ADDRSTRLEN, &p_mng_if->gateway_v6.u.prefix6);
    
    if (is_add)
    {
        sal_snprintf(cmd_str, sizeof(cmd_str), NETNS_CMD_PREFIX" route -A inet6 add default gw %s >/dev/null 2>&1", 
            addr_str);
    }
    else
    {
        sal_snprintf(cmd_str, sizeof(cmd_str), NETNS_CMD_PREFIX" route -A inet6 del default gw %s >/dev/null 2>&1",
            addr_str);
        sal_memset(&p_mng_if->gateway_v6, 0, sizeof(p_mng_if->gateway_v6));
    }
    
    rc = system(cmd_str);
    if (rc) 
    {
        return -1;
    }
    
    tbl_manage_if_set_manage_if_field(p_mng_if, TBL_MANAGE_IF_FLD_GATEWAY_V6);

    return rc;
}

static int32
_chsm_cmd_get_manage_if(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_manage_if_t* p_mng_if = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_mng_if = tbl_manage_if_get_manage_if();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_manage_if_dump_one(p_mng_if, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_chsm_cmd_manage_if_set_addr(tbl_manage_if_t *p_db_mng_if, tbl_manage_if_t *p_mng_if, cfg_cmd_para_t *para)
{
    addr_ipv4_t old_gw;
    int32 rc = PM_E_NONE;

    /* 1. check duplicate config */
    if (0 == sal_memcmp(&p_db_mng_if->addr, &p_mng_if->addr, sizeof(prefix_ipv4_t)))
    {
        return PM_E_NONE;
    }

    if (p_mng_if->addr.prefixlen > 32)
    {
        CFG_INVALID_PARAM_RET("Invalid prefix length %u", p_mng_if->addr.prefixlen);
    }
        
    rc = chsm_manage_if_set_addr(p_mng_if);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Add managment if address failed");
    }
    
    /* update default gateway */
    if (0 != p_db_mng_if->gateway.s_addr)
    {
        old_gw = p_db_mng_if->gateway;
        p_mng_if->gateway = old_gw;
        rc = chsm_manage_if_set_gateway(p_mng_if, FALSE);
        if (rc < 0)
        {
            /* delete maybe fail for kernel has removed default gateway */
            sal_memset(&p_mng_if->gateway, 0, sizeof(addr_ipv4_t));
            rc = tbl_manage_if_set_manage_if_field(p_mng_if, TBL_MANAGE_IF_FLD_GATEWAY);
        }
        
        p_mng_if->gateway = old_gw;
        rc = chsm_manage_if_set_gateway(p_mng_if, TRUE);
        if (rc < 0)
        {
            return PM_E_NONE;
        }
    }

    return rc;
}

static int32
_chsm_cmd_manage_if_set_gateway(tbl_manage_if_t *p_db_mng_if, tbl_manage_if_t *p_mng_if, cfg_cmd_para_t *para)
{
    addr_ipv4_t old_gw;
    uint32 has_old_gw = FALSE;
    uint32 has_new_gw = FALSE;
    int32 rc = PM_E_NONE;

    /* 1. check duplicate config */
    if (0 == sal_memcmp(&p_db_mng_if->gateway, &p_mng_if->gateway, sizeof(addr_ipv4_t)))
    {
        return PM_E_NONE;
    }

    has_old_gw = (0 != p_db_mng_if->gateway.s_addr) ? TRUE : FALSE;
    has_new_gw = (0 != p_mng_if->gateway.s_addr) ? TRUE : FALSE;

    if (has_old_gw)
    {
        old_gw = p_db_mng_if->gateway;
        rc = chsm_manage_if_set_gateway(p_db_mng_if, FALSE);
    }

    if (has_new_gw)
    {
        rc = chsm_manage_if_set_gateway(p_mng_if, TRUE);
        if (rc < 0)
        {
            /* resume old gateway if has */
            if (has_old_gw)
            {
                p_mng_if->gateway = old_gw;
                rc = chsm_manage_if_set_gateway(p_mng_if, TRUE);
            }
            CFG_CONFLICT_RET("Set managment interface default gateway failed");
        }
    }
    
    return rc;
}

static int32
_chsm_cmd_manage_if_set_gateway_v6(tbl_manage_if_t *p_db_mng_if, tbl_manage_if_t *p_mng_if, cfg_cmd_para_t *para)
{
    addr_ipv6_t old_gw;
    addr_ipv6_t addr_ipv6_tmp;
    uint32 has_old_gw = FALSE;
    uint32 has_new_gw = FALSE;
    int32 rc = PM_E_NONE;
    
    sal_memset(&addr_ipv6_tmp, 0, sizeof(addr_ipv6_tmp));
    sal_memset(&old_gw, 0, sizeof(old_gw));

    /* 1. check duplicate config */
    if (0 == sal_memcmp(&p_db_mng_if->gateway_v6, &p_mng_if->gateway_v6, sizeof(p_db_mng_if->gateway_v6)))
    {
        return PM_E_NONE;
    }

    has_old_gw = (0 != sal_memcmp(&p_db_mng_if->gateway_v6.u.prefix6, &addr_ipv6_tmp, sizeof(addr_ipv6_tmp))) ? TRUE : FALSE;
    has_new_gw = (0 != sal_memcmp(&p_mng_if->gateway_v6.u.prefix6, &addr_ipv6_tmp, sizeof(addr_ipv6_tmp))) ? TRUE : FALSE;

    if (has_old_gw)
    {
        sal_memcpy(&old_gw, &p_db_mng_if->gateway_v6.u.prefix6, sizeof(old_gw));
        rc = chsm_manage_if_set_gateway_v6(p_db_mng_if, FALSE);
    }

    if (has_new_gw)
    {
        rc = chsm_manage_if_set_gateway_v6(p_mng_if, TRUE);
        if (rc < 0)
        {
            /* resume old gateway if has */
            if (has_old_gw)
            {
                sal_memcpy(&p_mng_if->gateway_v6.u.prefix6, &old_gw, sizeof(old_gw));
                rc = chsm_manage_if_set_gateway_v6(p_mng_if, TRUE);
            }
            CFG_CONFLICT_RET("Set managment interface default gateway failed");
        }
    }
    
    return rc;
}

static int32
_chsm_cmd_manage_if_unset_gateway_v6(tbl_manage_if_t *p_db_mng_if, tbl_manage_if_t *p_mng_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. check duplicate config */
    if (0 != sal_memcmp(&p_db_mng_if->gateway_v6, &p_mng_if->gateway_v6, sizeof(p_db_mng_if->gateway_v6)))
    {
        return PM_E_MANAGEMENT_IF_GATEWAY_NOT_EXIST;
    }

    rc = chsm_manage_if_set_gateway_v6(p_db_mng_if, FALSE);
    
    return rc;
}

static int32
_chsm_cmd_manage_if_set_addr_v6(tbl_manage_if_t *p_db_mng_if, tbl_manage_if_t *p_mng_if, cfg_cmd_para_t *para, uint32 is_no_management_ipv6)
{
    char buf[CMD_BUF_256];
    int32 rc = PM_E_NONE;
    prefix_t old_gw;

    /* 1. check duplicate config */
    if (0 == sal_memcmp(&p_db_mng_if->addr_v6, &p_mng_if->addr_v6, sizeof(prefix_t)))
    {
        return PM_E_NONE;
    }

    if (!is_no_management_ipv6)
    {
        if (p_mng_if->addr_v6.prefixlen > 128)
        {
            CFG_INVALID_PARAM_RET("Invalid prefix length %u", p_mng_if->addr_v6.prefixlen);
        }

        if (lib_check_ipv6_prefix(&p_mng_if->addr_v6) < 0)
        {
            prefix_val2str(buf, CMD_BUF_256, &p_mng_if->addr_v6, TRUE);
            CFG_INVALID_PARAM_RET("Bad address %s", buf);
        }
     }
    if (p_db_mng_if->addr_v6.prefixlen)
    {
        rc = chsm_manage_if_set_addr_v6(p_db_mng_if, FALSE);
    } 
    if (!is_no_management_ipv6)
    {
        if (p_mng_if->addr_v6.prefixlen)
        {
            rc = chsm_manage_if_set_addr_v6(p_mng_if, TRUE);
            if (rc < 0)
            {
                CFG_CONFLICT_RET("Add managment if address failed");
            }
        }
    }
    if (is_no_management_ipv6)
    {
        rc = chsm_manage_if_set_gateway_v6(p_db_mng_if, FALSE);
    }
    rc = tbl_manage_if_set_manage_if_field(p_mng_if, TBL_MANAGE_IF_FLD_ADDR_V6);

    sal_memset(&old_gw, 0x00, sizeof(old_gw));
    /* update default gateway */
    if (0 != sal_memcmp(&p_db_mng_if->gateway_v6, &old_gw, sizeof(old_gw)))
    {
        sal_memcpy(&old_gw, &p_db_mng_if->gateway_v6, sizeof(old_gw));
        sal_memcpy(&p_mng_if->gateway_v6, &old_gw, sizeof(old_gw));
        rc = chsm_manage_if_set_gateway_v6(p_mng_if, FALSE);
        if (rc < 0)
        {
            /* delete maybe fail for kernel has removed default gateway */
            sal_memset(&p_mng_if->gateway_v6, 0, sizeof(p_mng_if->gateway_v6));
            rc = tbl_manage_if_set_manage_if_field(p_mng_if, TBL_MANAGE_IF_FLD_GATEWAY_V6);
        }
        
        sal_memcpy(&p_mng_if->gateway_v6, &old_gw, sizeof(old_gw));
        rc = chsm_manage_if_set_gateway_v6(p_mng_if, TRUE);
        if (rc < 0)
        {
            return PM_E_NONE;
        }
    }
    else
    {
        if (is_no_management_ipv6)
        {
            char *ifname = NULL;
            ifname = chsm_manage_if_get_ifname();
            _chsm_set_mgmt_if_disable_ipv6(ifname, 1);
        }
    }

    return rc;
}

uint32
chsm_api_is_ipv6_profile()
{
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis(); 
    if (NULL == p_chassis) 
    { 
        return FALSE;
    }

    type = p_chassis->cur_stm_mode;    
    if (GLB_STM_MODE_IPUCV6 == type) 
    {
        return TRUE;
    }
    
    return FALSE;
}

uint32
chsm_api_is_ipv6_profile_for_gb_gg()
{
#if defined (GOLDENGATE) || defined (GREATBELT)
    /*tbl_chassis_t   *p_chassis  = NULL;
    int32               type = 0;

    p_chassis = tbl_chassis_get_chassis(); 
    if (NULL == p_chassis) 
    { 
        return FALSE;
    }

    type = p_chassis->cur_stm_mode;

    if (GLB_STM_MODE_OF_IPV6 == type) 
    {
        return TRUE;
    }*/
    return TRUE;
#endif
    
    return FALSE;
}


static int32
_chsm_cmd_manage_if_set_dhclient_en(tbl_manage_if_t *p_db_mng_if, uint32 enable, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. check duplicate config */
    if (p_db_mng_if->dhclient_enable == enable)
    {
        return PM_E_NONE;
    }
    p_db_mng_if->dhclient_enable = enable;

    /* 2. call API */
    PM_E_RETURN(tbl_manage_if_set_manage_if_field(p_db_mng_if, TBL_MANAGE_IF_FLD_DHCLIENT_ENABLE));
    
    return PM_E_NONE;
}

static int32
_chsm_cmd_manage_if_set_dhclientv6_en(tbl_manage_if_t *p_db_mng_if, uint32 enable, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char *ifname = NULL;

    if (FALSE == chsm_api_is_ipv6_profile())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    /* 1. check duplicate config */
    if (p_db_mng_if->dhclientv6_enable == enable)
    {
        return PM_E_NONE;
    }

    ifname = chsm_manage_if_get_ifname();
    if (enable)
    {
        _chsm_set_mgmt_if_disable_ipv6(ifname, 0);
        //sleep(1);
    }
    
    p_db_mng_if->dhclientv6_enable = enable;

    /* 2. call API */
    PM_E_RETURN(tbl_manage_if_set_manage_if_field(p_db_mng_if, TBL_MANAGE_IF_FLD_DHCLIENTV6_ENABLE));

    if (!enable)
    {
        _chsm_set_mgmt_if_disable_ipv6(ifname, 1);   
    }
    
    return PM_E_NONE;
}

int32
chsm_cmd_process_manage_if(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_manage_if_t *p_db_mng_if = NULL;
    field_parse_t field;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    cdb_node_t* p_node = NULL;
    tbl_manage_if_t mng_if;
    int32 rc = PM_E_NONE;
    uint32 value = 0;
    uint32 is_no_management_ipv6 = FALSE;
    addr_ipv6_t addr_ipv6_tmp;
    p_node = cdb_get_tbl(TBL_MANAGE_IF);
    addr_ipv4_t         ipv4_zero;

    sal_memset(&ipv4_zero, 0, sizeof(addr_ipv4_t));

    switch (para->oper)
    {
    case CDB_OPER_SET:
        p_db_mng_if = tbl_manage_if_get_manage_if();
        sal_memset(&mng_if, 0, sizeof(mng_if));
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }
        
        switch (field_id)
        {
        case TBL_MANAGE_IF_FLD_ADDR:
            if (p_db_mng_if->dhclient_enable)
            {
                CFG_INVALID_PARAM_RET("Conflict with dhcp client function enabled on management interface");
            }
	
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            CFG_PARSER_PREFIXV4(argv[1], argv[2], &mng_if.addr, para->p_rs);
            rc = _chsm_cmd_manage_if_set_addr(p_db_mng_if, &mng_if, para);
            break;
            
        case TBL_MANAGE_IF_FLD_GATEWAY:
            if (p_db_mng_if->dhclient_enable)
            {
                CFG_INVALID_PARAM_RET("Conflict with dhcp client function enabled on management interface");
            }
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 2, para->p_rs);
            if (2 == argc)
            {
                CFG_PARSER_IP_ADDR(argv[1], &mng_if.gateway, para->p_rs);
                if (0 == mng_if.gateway.s_addr)
                {
                    CFG_CONFLICT_RET("Invalid gateway %s", argv[1]);
                }
            }
            else
            {
                mng_if.gateway.s_addr = 0;
            }
            rc = _chsm_cmd_manage_if_set_gateway(p_db_mng_if, &mng_if, para);
            break;

        case TBL_MANAGE_IF_FLD_ADDR_V6:
            if (p_db_mng_if->dhclientv6_enable)
            {
                CFG_INVALID_PARAM_RET("Conflict with dhcpv6 client function enabled on management interface");
            }
            /*modified by chenc for bug 53945, 2019-10-17*/
            #if 0
            if (FALSE == chsm_api_is_ipv6_profile())
            {
                if (FALSE == chsm_api_is_ipv6_profile_for_gb_gg())
                {
                    CFG_CONFLICT_RET("No ipv6 resource, please change profile");
                }
            }
            #endif
            /*chenc end*/
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 3, para->p_rs);
            if (argc == 3)
            {
                CFG_PARSER_PREFIX(argv[1], argv[2], &mng_if.addr_v6, para->p_rs);
            }
            else
            {
                is_no_management_ipv6 = TRUE;
            }
            
            if (IN6_IS_ADDR_LINKLOCAL(&mng_if.addr_v6.u.prefix6))
            {
                CFG_CONFLICT_RET("Can't be link-local address"); 
            }
			/*add by zhw for check mng_if ipv6 address*/
			if (IN6_IS_ADDR_6TO4(&mng_if.addr_v6.u.prefix6))
    		{
        		CFG_INVALID_PARAM_RET ("Can't be 6to4 address");
    		}
			/*end add*/
            rc = _chsm_cmd_manage_if_set_addr_v6(p_db_mng_if, &mng_if, para, is_no_management_ipv6);
            break;

        case TBL_MANAGE_IF_FLD_GATEWAY_V6:
            if (p_db_mng_if->dhclientv6_enable)
            {
                CFG_INVALID_PARAM_RET("Conflict with dhcpv6 client function enabled on management interface");
            }
            /*modified by chenc for bug 53945, 2019-10-17*/
            #if 0
            if (FALSE == chsm_api_is_ipv6_profile())
            {
                if (FALSE == chsm_api_is_ipv6_profile_for_gb_gg())
                {
                    CFG_CONFLICT_RET("No ipv6 resource, please change profile");
                }
            }
            #endif
            /*chenc end*/
            sal_memset(&addr_ipv6_tmp, 0, sizeof(addr_ipv6_tmp));
            if (0 == sal_memcmp(&addr_ipv6_tmp, &(p_db_mng_if->addr_v6.u.prefix6), sizeof(addr_ipv6_tmp)))
            {
                CFG_CONFLICT_RET("No ipv6 address, can't configure gateway");
            }
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 3, 2, para->p_rs);
            sal_memset(&addr_ipv6_tmp, 0, sizeof(addr_ipv6_tmp));
            CFG_PARSER_IPV6_ADDR(argv[1], &mng_if.gateway_v6.u.prefix6, para->p_rs);
            mng_if.gateway_v6.family = AF_INET6;
            if (0 == sal_memcmp(&addr_ipv6_tmp, &mng_if.gateway_v6.u.prefix6, sizeof(addr_ipv6_tmp)))
            {
                CFG_CONFLICT_RET("Invalid gateway %s", argv[1]);
            }

            if (IN6_IS_ADDR_LINKLOCAL(&mng_if.gateway_v6.u.prefix6))
            {
                CFG_INVALID_PARAM_RET("Can't be link-local address");
            }
                
            if (2 == argc)
            {
                rc = _chsm_cmd_manage_if_set_gateway_v6(p_db_mng_if, &mng_if, para);
            }
            else
            {
                rc = _chsm_cmd_manage_if_unset_gateway_v6(p_db_mng_if, &mng_if, para);
                if (PM_E_MANAGEMENT_IF_GATEWAY_NOT_EXIST == rc)
                {
                    CFG_CONFLICT_RET("Gateway %s does not exist", argv[1]);
                }
            }
            break;

        case TBL_MANAGE_IF_FLD_DHCLIENT_ENABLE:
            if (sal_memcmp(&p_db_mng_if->addr.prefix, &ipv4_zero, sizeof(addr_ipv4_t))
                || sal_memcmp(&p_db_mng_if->gateway, &ipv4_zero, sizeof(addr_ipv4_t)))
            {
                CFG_INVALID_PARAM_RET("Dhcp client function can't be enabled when ip address or route configured");
            }
            
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            if (0 == value && 0 == p_db_mng_if->dhclient_enable)
            {
                CFG_CONFLICT_RET("No DHCP address configured");
            }

            /* 1. reset mgmt route and ip first*/
            sal_memset(&mng_if, 0, sizeof(mng_if));
            rc = _chsm_cmd_manage_if_set_gateway(p_db_mng_if, &mng_if, para);

            CFG_PARSER_PREFIXV4("0.0.0.0", "0", &mng_if.addr, para->p_rs);
            rc = _chsm_cmd_manage_if_set_addr(p_db_mng_if, &mng_if, para);

            /* 2. enable dhclient on mgmt if */
            rc = _chsm_cmd_manage_if_set_dhclient_en(p_db_mng_if,value, para);
            break;
            
        case TBL_MANAGE_IF_FLD_DHCLIENT_ADDR:
            break;

        case TBL_MANAGE_IF_FLD_DHCLIENT_GATEWAY:
            break;

        case TBL_MANAGE_IF_FLD_DHCLIENTV6_ENABLE:
            if ((AF_INET6 == p_db_mng_if->addr_v6.family)
                || (AF_INET6 == p_db_mng_if->gateway_v6.family))
            {
                CFG_INVALID_PARAM_RET("Dhcpv6 client function can't be enabled when ipv6 address or route configured");
            }
            
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            
            rc = _chsm_cmd_manage_if_set_dhclientv6_en(p_db_mng_if,value, para);
            break;
            
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[0]);
            break;
        }
        break;

    case CDB_OPER_GET:
        if (argc)
        {
            if ((0 == sal_strcmp(argv[0], "ipv4")) || (0 == sal_strcmp(argv[0], "ipv6")))
            {
                sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            }
            else
            {
                rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
                if (rc < 0)
                {
                    CFG_INVALID_PARAM_RET("Invalid field");
                }
                
            }
        }
        else
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }
        rc = _chsm_cmd_get_manage_if(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

#define _CHSM_TBL_BOOTIMAGE_

static int32
_chsm_cmd_bootimage_get(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_bootimage_t* p_boot = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_boot = tbl_bootimage_get_bootimage();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_bootimage_dump_one(p_boot, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_chsm_cmd_bootimage_set_tftp(tbl_bootimage_t *p_db_boot, tbl_bootimage_t *p_boot, cfg_cmd_para_t *para)
{
    FILE *fp;
    char bootcmd[M_FULLPATH_MAX_LEN];
    tbl_card_t* p_card = tbl_card_get_default_card();
    tbl_manage_if_t *p_db_mng_if = tbl_manage_if_get_manage_if();
    
    if(!p_card)
    {
        return PM_E_NOT_INITED;
    }

#ifdef _CTC_X86_64_
    fp = sal_fopen(CHSM_DEBIAN_BOOT_CONFIG_FILE, "w+");
    if (fp)
    {
        sal_fprintf(fp, "tftp://%s/%s\n", p_boot->serverip, p_boot->image);
        sal_fclose(fp);
    }
    sal_strcpy(bootcmd, "debian_tftp");
#else
    /* get ipaddr */
    if (BOOT_MODE_TFTP == p_boot->mode)
    {
        sal_inet_ntop(AF_INET, &p_db_mng_if->addr.prefix, p_boot->ipaddr, SYS_INFO_SIZE);
    }
    else if (BOOT_MODE_TFTP6 == p_boot->mode)
    {
        sal_inet_ntop(AF_INET6, &p_db_mng_if->addr_v6.u.prefix6, p_boot->ipaddr, SYS_INFO_SIZE);
    }
    if(p_card->platform_type == 0) /* for not onie platform */
    {

#ifdef CTC_ENV_IN_SPI_FLASH
        if (BOOT_MODE_TFTP == p_boot->mode)
        {
            sal_snprintf(bootcmd, M_FULLPATH_MAX_LEN,
                        "fw_setenv bootcmd boot_tftp %s 1>/dev/null 2>&1; "
                        "fw_setenv serverip %s 1>/dev/null 2>&1; "
                        "fw_setenv ipaddr %s 1>/dev/null 2>&1",
                        p_boot->image, p_boot->serverip, p_boot->ipaddr);
        }
        else if (BOOT_MODE_TFTP6 == p_boot->mode)
        {
            /* todo */
            return PM_E_NOT_SUPPORT;
        }
#else
        if (BOOT_MODE_TFTP == p_boot->mode)
        {
            sal_strcpy(bootcmd,"bootcmd=boot_tftp ");
            sal_strcat(bootcmd,p_boot->image);
            sal_strcat(bootcmd,"&serverip=");
            sal_strcat(bootcmd,p_boot->serverip);
            sal_strcat(bootcmd,"&ipaddr=");
            sal_strcat(bootcmd,p_boot->ipaddr);
        }
        else if (BOOT_MODE_TFTP6 == p_boot->mode)
        {
            sal_strcpy(bootcmd,"bootcmd=boot_tftp6 ");
            sal_strcat(bootcmd,p_boot->image);
            sal_strcat(bootcmd,"&serverip6=");
            sal_strcat(bootcmd,p_boot->serverip);
            sal_strcat(bootcmd,"&ip6addr=");
            sal_strcat(bootcmd,p_boot->ipaddr);
        }
#endif             
    }
    else
    {
        char szTmpName[] = "/tmp/fw_env.txt";
        fp = sal_fopen (szTmpName, "w+");
        if (fp == NULL)
        {
            CFG_INVALID_PARAM_RET( "Open env file fail!");
        }
        if (BOOT_MODE_TFTP == p_boot->mode)
        {
            sal_fprintf(fp, "bootcmd run check_boot_reason; run nos_bootcmd; run onie_bootcmd\n"); 
            sal_fprintf(fp, "hw_load tftpboot %s\n",p_boot->image); 
            sal_fprintf(fp, "serverip %s\n",p_boot->serverip); 
            sal_fprintf(fp, "ipaddr %s\n",p_boot->ipaddr);         
            sal_fprintf(fp, "copy_img echo \"Loading image...\" && run hw_load\n");        
            sal_fprintf(fp, "nos_bootcmd run copy_img && setenv bootargs bootimage=TftpFile://$serverip:%s \
        console=$consoledev,$baudrate cache-sram=0xfff00000,0x10000 && bootm $loadaddr\n",\
            p_boot->image);
        }
        else if (BOOT_MODE_TFTP6 == p_boot->mode)
        {
            /* todo */
            return PM_E_NOT_SUPPORT;
        }
        sal_fclose(fp);
      
        sal_strcpy(bootcmd,"fw_setenv -f -s /tmp/fw_env.txt ");
    }
#endif

    sal_memcpy(p_boot->bootcmd, bootcmd, M_FULLPATH_MAX_LEN);
    sal_memcpy(p_card->bootcmd, bootcmd, M_FULLPATH_MAX_LEN);
    tbl_card_set_card_field(p_card, TBL_CARD_FLD_BOOTCMD);

    tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_MODE);
    tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_FAST);
    tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_IMAGE);
    tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_SERVERIP);
    tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_IPADDR);
    tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_BOOTCMD);

    /* wait 2s to ensure bootcmd be updated to eeprom */
    sleep(2);
    
    return PM_E_NONE;
}

static int32
_chsm_cmd_bootimage_set_flash_mode(tbl_bootimage_t *p_db_boot, tbl_bootimage_t *p_boot, cfg_cmd_para_t *para)
{
    FILE *fp;
    char bootcmd[M_FULLPATH_MAX_LEN];
    tbl_card_t* p_card = tbl_card_get_default_card();
    uint32 mode_change = FALSE;

    if(!p_card)
    {
        return PM_E_NOT_INITED;
    }

#ifdef _CTC_X86_64_
    sal_memset(bootcmd, 0, sizeof(bootcmd));
    fp = sal_fopen(CHSM_DEBIAN_BOOT_CONFIG_FILE, "r");
    if (fp)
    {
        sal_fgets(bootcmd, M_FULLPATH_MAX_LEN, fp);
        if (NULL == sal_strstr(bootcmd, CHSM_DEBIAN_UIMAGE_PATH))
        {
            mode_change = TRUE;
        }
        sal_fclose(fp);
    }
    if (mode_change)
    {
        fp = sal_fopen(CHSM_DEBIAN_BOOT_CONFIG_FILE, "w+");
        {
            sal_fprintf(fp, "%s\n", CHSM_DEBIAN_UIMAGE_PATH);
            sal_fclose(fp);
        }
    }
    sal_strcpy(bootcmd, "debian_flash");
#else
    (void)mode_change;
    if(p_card->platform_type == 0)   /* for not onie platform */
    {
#ifdef CTC_ENV_IN_SPI_FLASH
        sal_snprintf(bootcmd, M_FULLPATH_MAX_LEN, "fw_setenv bootcmd boot_flash 1>/dev/null 2>&1");
#else        
        sal_strcpy(bootcmd,"bootcmd=boot_flash");
#endif
    }
    else
    {
        char szTmpName[] = "/tmp/fw_env.txt";
        
        fp = sal_fopen (szTmpName, "w+");
        if (fp == NULL)
        {
            CFG_INVALID_PARAM_RET("Open env file fail!");
        }
        
        sal_fprintf(fp, "bootcmd run check_boot_reason; run nos_bootcmd; run onie_bootcmd\n");
        sal_fprintf(fp, "hw_load mtdpart default && ubi part boot 4096 \
    && ubifsmount boot && ubifsload 1000000 /uImage\n");      
        sal_fprintf(fp, "copy_img echo \"Loading image...\" && run hw_load\n");        
        sal_fprintf(fp, "nos_bootcmd run copy_img && setenv bootargs bootimage=uImage \
    console=$consoledev,$baudrate cache-sram=0xfff00000,0x10000 \
    && bootm $loadaddr\n"); 
        sal_fclose(fp);
        sal_strcpy(bootcmd,"fw_setenv -f -s /tmp/fw_env.txt ");
    }
#endif
    
    sal_memcpy(p_boot->bootcmd, bootcmd, M_FULLPATH_MAX_LEN);
    sal_memcpy(p_card->bootcmd, bootcmd, M_FULLPATH_MAX_LEN);
    tbl_card_set_card_field(p_card, TBL_CARD_FLD_BOOTCMD);
    
    tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_MODE);
    tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_FAST);
    tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_IMAGE);
    tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_SERVERIP);
    tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_IPADDR);
    tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_BOOTCMD);

#ifdef _CTC_ARM_HI3535_
    system("/etc/installOS.sh /mnt/flash/boot/uImage 1>/dev/null 2>&1"); 
#endif
    /* wait 2s to ensure bootcmd be updated to eeprom */
    sleep(2);
    
    return PM_E_NONE;
}

/*
date -d "2017-10-13 00:00:00" +%s
1507852800
*/
#define DATETIME_20171013_SEC 1507852800

static int32
_chsm_cmd_bootimage_set_flash(tbl_bootimage_t *p_db_boot, tbl_bootimage_t *p_boot, cfg_cmd_para_t *para)
{
    char szShowName[M_FULLPATH_MAX_LEN];
    char szFullName[M_FULLPATH_MAX_LEN];
    char szBuf[M_FULLPATH_MAX_LEN];
    uint32 create_time = 0;
    int32 rc = PM_E_NONE;

    sal_memset(szShowName, 0, sizeof(szShowName));
    sal_memset(szFullName, 0, sizeof(szFullName));
    sal_memset(szBuf, 0, sizeof(szBuf));
    
    sal_strncpy(szShowName, SHOW_NAME_FLASH_BOOT"/", SYS_INFO_SIZE);
    sal_strncat(szShowName, p_boot->image, SYS_INFO_SIZE);
            
    if (gen_check_and_get_filename_bytype(szShowName, szFullName, 
                M_FULLPATH_MAX_LEN, E_DISK_FILE_FLASH) != 0)
    {
        CFG_INVALID_PARAM_RET("Invalid image file path: %s", szShowName);
    } 

    if (!lib_fs_is_file(szFullName))
    {
        CFG_INVALID_PARAM_RET("Not a file error: %s", szShowName);
    }

    if (!sal_strcmp(szFullName, M_CUR_BOOT_IMAGE_LINK_FULLNAME))
    {
        CFG_INVALID_PARAM_RET("Can't specify boot configuration file as system image file");
    }
    
    if (sal_strncmp(szFullName, M_BOOT_IMAGE_PATH, sal_strlen(M_BOOT_IMAGE_PATH)))
    {
        char szShowName[M_FULLPATH_MAX_LEN];
        
        gen_check_and_gen_showname(M_BOOT_IMAGE_PATH, szShowName);
        CFG_INVALID_PARAM_RET("Please remove system image files into directory %s/", szShowName);
    }
    else 
    {
        /* check if is flash:/bootXXX/YYY or flash:/boot/XXX/YYY */
        char *p = szFullName + sal_strlen(M_BOOT_IMAGE_PATH);

        if ('/' != *p || NULL != sal_strchr(p + 1, '/'))
        {
            char szShowName[M_FULLPATH_MAX_LEN];
            
            gen_check_and_gen_showname(M_BOOT_IMAGE_PATH, szShowName);
            CFG_INVALID_PARAM_RET("Please remove system image files into directory %s/", szShowName);
        }
    }
    
    /* Added by kcao for bug 45431, check old version */
    if (p_boot->fast)
    {
        rc = get_createtime_from_image(szFullName, &create_time);
        if (rc < 0)
        {
            CFG_CONFLICT_RET("Read create time from %s failed", szFullName);
        }
        if (DATETIME_20171013_SEC > create_time)
        {
            CFG_CONFLICT_RET("Image does not support fast boot");
        }
    }
    
    /* do crc check */
    if (lib_image_check_image_file(szFullName, szBuf, M_FULLPATH_MAX_LEN, 1) != 0)
    {
        CFG_FUNC_ERR_RET("%s", szBuf);
    }

    if (p_boot->fast)
    {
        rc = lib_image_get_ramfs(szFullName);
        if (rc < 0)
        {
            log_sys(M_MOD_CHSM, E_ERROR, "boot fast %s, extract rootfs failed", szShowName);
            CFG_FUNC_ERR_RET("Extract rootfs from %s failed", szShowName);
        }
        else
        {
            /*Modified the szFullName to szShowName by yejl for bug 45661, 2018-02-05*/    
            log_sys(M_MOD_CHSM, E_WARNING, "boot fast %s, extract rootfs succeed", szShowName);
        }
    }

#ifndef _GLB_UML_SYSTEM_
    match_boot_and_remount(szFullName, MS_REMOUNT | MS_SILENT);
#endif
    sal_unlink(M_CUR_BOOT_IMAGE_LINK_FULLNAME);
    if (sal_symlink(szFullName + sal_strlen(M_BOOT_IMAGE_PATH) + 1, M_CUR_BOOT_IMAGE_LINK_FULLNAME) != 0)
    {
#ifndef _GLB_UML_SYSTEM_
        match_boot_and_remount(szFullName, MS_RDONLY | MS_REMOUNT | MS_SILENT);
#endif
        CFG_FUNC_ERR_RET("Setting the next boot image file faild: %s", sal_strerror(errno));
    }
#ifndef _GLB_UML_SYSTEM_
    match_boot_and_remount(szFullName, MS_RDONLY | MS_REMOUNT | MS_SILENT);
#endif
    
    return _chsm_cmd_bootimage_set_flash_mode(p_db_boot, p_boot, para);
}

static int32
_chsm_cmd_bootimage_set(tbl_bootimage_t *p_db_boot, tbl_bootimage_t *p_boot, cfg_cmd_para_t *para)
{
    uint32 has_addr = TRUE;
    int32 rc = PM_E_NONE;

    /* 1. check duplicate config */
    if (0 == sal_memcmp(p_db_boot, &p_boot, sizeof(tbl_bootimage_t)))
    {
        return PM_E_NONE;
    }

    if ((BOOT_MODE_TFTP == p_boot->mode)||(BOOT_MODE_TFTP6 == p_boot->mode))
    {
        if (!has_addr)
        {
            CFG_CONFLICT_RET("Managment ip address should be configured in startup-config at first");
        }
        PM_E_RETURN(_chsm_cmd_bootimage_set_tftp(p_db_boot, p_boot, para));
    }
    else
    {
        PM_E_RETURN(_chsm_cmd_bootimage_set_flash(p_db_boot, p_boot, para));
    }
    lib_image_remove_ramfs();
    
    return rc;
}

#define MEMORY_FREE_FILENAME    "/tmp/.memory_free"

int32
chsm_get_free_memory(uint32 *free_mem_k)
{
    char  line[CMD_BUF_256];
    uint32 value = 0;
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    *free_mem_k = 0;
    sal_system("cdbctl read/cdb/sys/mem_summary/free > "MEMORY_FREE_FILENAME);
    
    fp = sal_fopen(MEMORY_FREE_FILENAME, SAL_OPEN_RO);
    if (NULL == fp)
    {
        return PM_E_FAIL;
    }
    
    if (NULL != sal_fgets(line, CMD_BUF_256, fp))
    {
        if (1 == sal_sscanf(line, "/free=%u", &value))
        {
            *free_mem_k = value;
        }
        else
        {
            rc = PM_E_FAIL;
        }
    }
    
    sal_fclose(fp);
    sal_unlink(MEMORY_FREE_FILENAME);

    return rc;
}

static int32
_chsm_cmd_check_boot_fast_condition()
{
    tbl_bootimage_t *p_db_boot = tbl_bootimage_get_bootimage();
    char *curr_image = NULL;
    
    curr_image = sal_strstr(p_db_boot->current_image, "TftpFile://");
    if (curr_image)
    {
        return PM_E_FAIL;
    }
    curr_image = sal_strstr(p_db_boot->current_image, "Tftp6File://");
    if (curr_image)
    {
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}

static int32
_chsm_cmd_bootimage_fast_set(tbl_bootimage_t *p_db_boot, tbl_bootimage_t *p_boot, cfg_cmd_para_t *para)
{
    uint32 free_mem_k = 0;
    int32 rc = PM_E_NONE;

    /* 1. check duplicate config */
    if (0 == sal_memcmp(p_db_boot, &p_boot, sizeof(tbl_bootimage_t)))
    {
        return PM_E_NONE;
    }

#ifdef _CTC_X86_64_
    CFG_CONFLICT_RET("Not supported on X64 CPU");
#endif

    if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
    {
        CFG_UNSUPPORT_PRODUCT_RET();
    }
    
    rc = _chsm_cmd_check_boot_fast_condition();
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Cannot set boot fast with tftp boot mode");
    }
        
    rc = chsm_get_free_memory(&free_mem_k);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Get system free memory failed");
    }
    
    if (free_mem_k < 300000)
    {
        CFG_CONFLICT_RET("System free memory is insufficient to decompression ramfs");
    }

    PM_E_RETURN(_chsm_cmd_bootimage_set_flash(p_db_boot, p_boot, para));
    
    return rc;
}

int32
chsm_cmd_process_bootimage(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_bootimage_t *p_db_boot = NULL;
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    tbl_bootimage_t boot;
    int32 boot_mode = 0;
    addr_ipv4_t ip_addr;
    addr_ipv6_t ip6_addr;
    int32 image_length = 0;
    int32 rc = PM_E_NONE;

    p_node = cdb_get_tbl(TBL_BOOTIMAGE);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        p_db_boot = tbl_bootimage_get_bootimage();
        sal_memset(&boot, 0, sizeof(boot));
        if (0 == sal_strcmp("fast", argv[0]))
        {
            boot_mode = cdb_enum_str2val(boot_mode_strs, BOOT_MODE_MAX, argv[1]);
            boot.mode = boot_mode;
            boot.fast = TRUE;
            
            if (BOOT_MODE_FLASH == boot_mode)
            {
                PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                sal_strncpy(boot.image, argv[2], SYS_INFO_SIZE);
            }
            else
            {
                CFG_INVALID_PARAM_RET("Unknown boot mode %s", argv[2]);
            }
            
            rc = _chsm_cmd_bootimage_fast_set(p_db_boot, &boot, para);
        }
        else
        {
            boot_mode = cdb_enum_str2val(boot_mode_strs, BOOT_MODE_MAX, argv[0]);
            boot.mode = boot_mode;
            boot.fast = FALSE;

            if (BOOT_MODE_FLASH == boot_mode)
            {
                PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
                sal_strncpy(boot.image, argv[1], SYS_INFO_SIZE);
            }
            else if ((BOOT_MODE_TFTP == boot_mode)||(BOOT_MODE_TFTP6 == boot_mode))
            {
                PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                if(BOOT_MODE_TFTP == boot_mode)
                {
                    CFG_PARSER_IP_ADDR(argv[1], &ip_addr, para->p_rs);
                }
                else
                {
                    CFG_PARSER_IPV6_ADDR(argv[1], &ip6_addr, para->p_rs);
                }
                sal_strncpy(boot.serverip, argv[1], SYS_INFO_SIZE);
                image_length = sal_strlen(argv[2]);
                if ((0 >= image_length) || SYS_INFO_SIZE <= image_length)
                {
                    CFG_INVALID_PARAM_RET("Image file name length %d is out range of [1, 128)", image_length);
                }
                sal_strncpy(boot.image, argv[2], SYS_INFO_SIZE);
            }
            else
            {
                CFG_INVALID_PARAM_RET("Unknown boot mode %s", argv[0]);
            }
            
            rc = _chsm_cmd_bootimage_set(p_db_boot, &boot, para);
        }
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _chsm_cmd_bootimage_get(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

#define _CHSM_TBL_CHIP_

static int32
_chsm_cmd_get_chip(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_chip_t *p_chip = tbl_chip_get_chip();
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_chip_dump_one(p_chip, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}

int32
chsm_cmd_process_chip(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;
    
    p_node = cdb_get_tbl(TBL_CHIP);

    switch (para->oper)
    {
    case CDB_OPER_GET: 
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _chsm_cmd_get_chip(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
chsm_show_factory_config(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    FILE *fp2 = NULL;
    char buf[CMD_BUF_1024];
    
    if (!syscmd_file_exist(FACTORY_CONFIG_FILE_PATH))
    {
        CFG_INVALID_PARAM_RET("No factory configuration exist");
        return PM_E_INVALID_PARAM;
    }
    
    fp = sal_fopen(FACTORY_CONFIG_FILE_PATH, SAL_OPEN_RO);
    if (NULL != fp)
    {
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp2= sal_fopen(para->p_show->path, "w+");
        if (NULL == fp2)
        {
            sal_fclose(fp);
            return PM_E_NONE;
        }  
        while (NULL != sal_fgets(buf, CMD_BUF_1024, fp))
        {
            sal_fprintf(fp2, "%s", buf);                
        }
        sal_fclose(fp2);
        sal_fclose(fp);
    }
    return PM_E_NONE;
}

static int32
chsm_write_factory_config(char **argv, int32 argc,cfg_cmd_para_t *para)
{
    char src_file[M_FULLPATH_MAX_LEN] = {0};
    char filepath[M_FULLPATH_MAX_LEN] = {0};
    char szCmd[CMD_BUF_256] = {0};
    char *filename = NULL;
  
    if (1 != argc)
    {
        if (3 == argc)
        {
            if (0 != sal_strcmp(argv[1], SHOW_NAME_FLASH))
            {
                CFG_INVALID_PARAM_RET("Invalid directory %s", argv[1]);
            }
            sal_sprintf(filepath, SHOW_NAME_FLASH"/%s", argv[2]);
            filename = argv[2];
        }
        else
        {
            sal_sprintf(filepath, SHOW_NAME_FLASH"/%s", argv[1]);
            filename = argv[1];
        }
        if (gen_check_and_get_filename(filepath, src_file, M_FULLPATH_MAX_LEN) != 0)
        {
            CFG_INVALID_PARAM_RET("The characters must be [0-9a-zA-Z._-()], path length range is [1, 255]. file name length range is [1, 47]");
            return PM_E_INVALID_PARAM;
        }
        if (!syscmd_file_exist(src_file))
        {
            CFG_INVALID_PARAM_RET("No such file error: %s does not exist", filename);
            return PM_E_INVALID_PARAM;
        }
        if (lib_fs_is_directory(src_file)) 
        {
            CFG_INVALID_PARAM_RET("Is directory error: %s", filename);
            return PM_E_INVALID_PARAM;
        }
        if (lib_fs_file_large(src_file))
        {
            CFG_INVALID_PARAM_RET("Don't support the file which is too large: %s", filename);
            return PM_E_INVALID_PARAM;
        }
        
        sal_snprintf(szCmd, sizeof(szCmd), "fnconvert -c filecopy -p '%s' -o '%s' >/dev/null 2>/dev/null ", src_file, FACTORY_CONFIG_FILE_PATH);
        system(szCmd);
        
    }

    return PM_E_NONE;
}

int32
chsm_cmd_process_factory_config(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    switch (para->oper)
    {
    case CDB_OPER_SET: 
        if (0 == sal_strcmp(argv[0], "write"))
        {
            chsm_write_factory_config(argv, argc, para);
        }       
        break;
        
    case CDB_OPER_GET:
            chsm_show_factory_config(para);
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return PM_E_NONE;
}

#define _CHSM_TBL_CARD_

static int32
_chsm_cmd_get_card(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_card_iterate((TBL_ITER_CB_FUNC)tbl_card_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_chsm_cmd_get_chassis(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_chassis_iterate((TBL_ITER_CB_FUNC)tbl_chassis_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}


int32
chsm_set_sensor_tmpr_threshld(tbl_card_t *p_db_card, tbl_iter_args_t *pargs)
{
    int32 low, high, critical;
    int rc = 0;

    if(NULL == p_db_card)
    {
        return PM_E_NOT_EXIST;
    }
    low = *(uint32*)pargs->argv[0];
    high = *(uint32*)pargs->argv[1];
    critical = *(uint32*)pargs->argv[2];
    p_db_card->tmpr_low = low;
    p_db_card->tmpr_high = high;
    p_db_card->tmpr_critical = critical;

    PM_E_RETURN(tbl_card_set_card_field(p_db_card,TBL_CARD_FLD_TMPR_LOW));
    PM_E_RETURN(tbl_card_set_card_field(p_db_card,TBL_CARD_FLD_TMPR_HIGH)); 
    PM_E_RETURN(tbl_card_set_card_field(p_db_card,TBL_CARD_FLD_TMPR_CRITICAL));
      
    return PM_E_NONE;
}


int32
chsm_set_tmpr_threshold(chsm_tmpr_cfg_t* tmpr_cfg)
{
    tbl_iter_args_t dump_args;
    int rc = 0;
    
    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    dump_args.argv[0] = &tmpr_cfg->low;
    dump_args.argv[1] = &tmpr_cfg->high;
    dump_args.argv[2] = &tmpr_cfg->crit;

    PM_E_RETURN(tbl_card_iterate((TBL_ITER_CB_FUNC)chsm_set_sensor_tmpr_threshld, &dump_args));
    
    return PM_E_NONE;
}

int32
chsm_set_fan_speed_one(tbl_card_t *p_db_card, tbl_iter_args_t *pargs)
{
    int32 low, high, full,bottom;
    int rc = 0;

    if(NULL == p_db_card)
    {
        return PM_E_NOT_EXIST;
    }
    bottom = *(uint32*)pargs->argv[0];
    low = *(uint32*)pargs->argv[1];
    high = *(uint32*)pargs->argv[2];
    full = *(uint32*)pargs->argv[3];
    p_db_card->fan_speed_bottom = bottom;
    p_db_card->fan_speed_low = low;
    p_db_card->fan_speed_high = high;
    p_db_card->fan_speed_full = full;

    PM_E_RETURN(tbl_card_set_card_field(p_db_card,TBL_CARD_FLD_FAN_SPEED_BOTTOM));
    PM_E_RETURN(tbl_card_set_card_field(p_db_card,TBL_CARD_FLD_FAN_SPEED_LOW)); 
    PM_E_RETURN(tbl_card_set_card_field(p_db_card,TBL_CARD_FLD_FAN_SPEED_HIGH));
    PM_E_RETURN(tbl_card_set_card_field(p_db_card,TBL_CARD_FLD_FAN_SPEED_FULL));

    return PM_E_NONE;
}

int32
chsm_set_fan_speed(chsm_fan_cfg_t* fan_cfg)
{
    tbl_iter_args_t dump_args;
    int rc = 0;
    
    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    dump_args.argv[0] = &fan_cfg->bottom;
    dump_args.argv[1] = &fan_cfg->low;
    dump_args.argv[2] = &fan_cfg->high;
    dump_args.argv[3] = &fan_cfg->full;

    PM_E_RETURN(tbl_card_iterate((TBL_ITER_CB_FUNC)chsm_set_fan_speed_one, &dump_args));
    
    return PM_E_NONE;
}

int32
chsm_set_sys_poe_one_enable(tbl_card_t *p_db_card, tbl_iter_args_t *pargs)
{
    char enable;
    int rc = 0;

    if(NULL == p_db_card)
    {
        return PM_E_NOT_EXIST;
    }
    enable = *(char*)pargs->argv[0];

    if(p_db_card->glb_poe_enable != enable)
    {
        p_db_card->glb_poe_enable = enable;
        PM_E_RETURN(tbl_card_set_card_field(p_db_card,TBL_CARD_FLD_GLB_POE_ENABLE));
    }

    return PM_E_NONE;
}

int32
chsm_set_sys_poe_one_max(tbl_card_t *p_db_card, tbl_iter_args_t *pargs)
{
    uint32 power_max;
    tbl_poe_master_t *p_poe_master = tbl_poe_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_poe_t *p_db_poe = NULL;
    int32 poe_count = 0;    
    int rc = 0;

    CTCLIB_SLIST_LOOP(p_poe_master->poe_list, p_db_poe, listnode)
    {
        poe_count++;
    }

    if(NULL == p_db_card)
    {
        return PM_E_NOT_EXIST;
    }
    power_max = *(uint32*)pargs->argv[1];

    if(p_db_card->glb_poe_power_max != (power_max/poe_count))
    {
        p_db_card->glb_poe_power_max = (power_max == 0)?DEFAULT_POE_POWER_MAX:(power_max/poe_count);
        PM_E_RETURN(tbl_card_set_card_field(p_db_card,TBL_CARD_FLD_GLB_POE_POWER_MAX));
    }

    return PM_E_NONE;
}

int32
chsm_set_sys_poe_enable(cfg_cmd_para_t *para, chsm_poe_cfg_t* poe_cfg)
{
    tbl_iter_args_t dump_args;
    tbl_poe_master_t *p_poe_master = tbl_poe_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_poe_t *p_db_poe = NULL;
    int32 poe_count = 0;    
    int rc = 0;

    CTCLIB_SLIST_LOOP(p_poe_master->poe_list, p_db_poe, listnode)
    {
        poe_count++;
    }
    if (!poe_count)
    {
        CFG_CONFLICT_RET("This board doesn't support poe!");
    }
    
    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    dump_args.argv[0] = &poe_cfg->enable;
    dump_args.argv[1] = &poe_cfg->power_max;

    PM_E_RETURN(tbl_card_iterate((TBL_ITER_CB_FUNC)chsm_set_sys_poe_one_enable, &dump_args));
    
    return PM_E_NONE;
}

int32
chsm_set_sys_poe_max(cfg_cmd_para_t *para, chsm_poe_cfg_t* poe_cfg)
{
    tbl_iter_args_t dump_args;
    tbl_poe_master_t *p_poe_master = tbl_poe_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_poe_t *p_db_poe = NULL;
    int32 poe_count = 0;    
    int rc = 0;

    CTCLIB_SLIST_LOOP(p_poe_master->poe_list, p_db_poe, listnode)
    {
        poe_count++;
    }
    if (!poe_count)
    {
        CFG_CONFLICT_RET("This board doesn't support poe!");
    }
    
    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    dump_args.argv[0] = &poe_cfg->enable;
    dump_args.argv[1] = &poe_cfg->power_max;

    CTCLIB_SLIST_LOOP(p_poe_master->poe_list, p_db_poe, listnode)
    {
        if((poe_cfg->power_max/poe_count) > p_db_poe->const_power_limit)
        {
            CFG_CONFLICT_RET("Max power limit exceeds total power supply!");
        }
    }
    PM_E_RETURN(tbl_card_iterate((TBL_ITER_CB_FUNC)chsm_set_sys_poe_one_max, &dump_args));
    
    return PM_E_NONE;
}


#define REBOOT_ONE_HOUR_MINUTES     60
#define REBOOT_ONE_MINUTE_SECONDS   60

#define WHITE_SPACE(C) ((C) == '\t' || (C) == ' ')
#define DELIMITER(C)       (WHITE_SPACE(C) || (C) == '\0')
#define ISLEAP(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
unsigned long g_max_day_for_month[] =
{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

char*
ctc_chinese_date_match(char *cp, uint32 *is_match)
{
    char *sp = NULL;
    int32 colons = 0;
    int32 nums = 0;
    char buf[7];
    unsigned long year = 0;
    unsigned long month = 0;
    unsigned long day = 0;
    int idx = 0;

    *is_match = FALSE;

    while (1)
    {
        sal_memset(buf, 0, sizeof (buf));
        sp = cp;
        while (! DELIMITER(*cp))
        {
            if (*cp == '/')
            {
                if (colons >= 2)
                {
                    return cp;
                }

                if (*(cp + 1) == '/')
                {
                    return cp;
                }

                colons++;
                break;
            }
            
            if (!isdigit ((int) *cp))
            {
                return cp;
            }

            cp++;
        }

        if (0 == idx)
        {
            /* for year field */
            if ((cp - sp) > 4)
            {
                return cp;
            }

            if ((cp - sp) < 4)
            {
                if ('\0' == *(cp + 1))
                {
                    *is_match = FALSE;
                }
                return cp;
            }
        }
        else if (cp - sp > 2)
        {
            return cp;
        }

        idx++;

        if (cp - sp)
        {
            sal_strncpy (buf, sp, cp - sp);
            switch (idx)
            {
            case 1:
                /* for year field */
                year = strtoul(buf, NULL, 10);
                if (year > 2037 || year < 2000)
                {
                    return cp;
                }
                break;
            case 2:
                /* for month field */
                month = strtoul(buf, NULL, 10);
                if (month <= 0 || month > 12)
                {
                    return cp;
                }
                break;
            case 3:
                /* for day field */
                day = strtoul(buf, NULL, 10);
                if (day <= 0)
                {
                    return cp;
                }
                if (2 == month && ISLEAP(year))
                {
                    if (day > (g_max_day_for_month[month - 1] + 1))
                    {
                        return cp - 1;
                    }
                }
                else if (day > g_max_day_for_month[month - 1])
                {
                    return cp - 1;
                }
                break;

            default:
                return cp;
            }

            nums++;
        }
            
        if (DELIMITER (*cp))
        {
            break;
        }     
        cp++;
    }     
     
    if (nums < 3)
    {
        *is_match = FALSE;
        return cp;
    }
    
    *is_match = TRUE;
    return cp;
}

char*
ctc_english_date_match(char *cp, uint32 *is_match)
{
    char *sp = NULL;
    int colons = 0;
    int nums = 0;
    char buf[7];
    unsigned long year = 0;
    unsigned long month = 0;
    unsigned long day = 0;
    int idx = 0;

    *is_match = FALSE;

    while (1)
    {
        sal_memset(buf, 0, sizeof (buf));
        sp = cp;
        while (! DELIMITER (*cp))
        {
            if (*cp == '/')
            {
                if (colons >= 2)
                {
                    return cp;
                }

                if (*(cp + 1) == '/')
                {
                    return cp;
                }

                colons++;
                break;
            }
            
            if (!sal_isdigit((int) *cp))
            {
                return cp;
            }

            cp++;
        }

        if (2 == idx)
        {
            /* for year field */
            if ((cp - sp) > 4)
            {
                return cp;
            }

            if ((cp - sp) < 4)
            {
                if ('\0' == *(cp + 1))
                {
                    *is_match = FALSE;
                }
                return cp;
            }
        }
        else if (cp - sp > 2)
        {
            return cp;
        }
        idx++;

        if (cp - sp)
        {
            sal_strncpy (buf, sp, cp - sp);
            switch (idx)
            {
            case 3:
                /* for year field */
                year = strtoul(buf, NULL, 10);
                if (year > 2037 || year < 2000)
                {
                    return cp;
                }
                break;
            case 1:
                /* for month field */
                month = strtoul(buf, NULL, 10);
                if (month <= 0 || month > 12)
                {
                    return cp;
                }
                break;
            case 2:
                /* for day field 
                 * we can't determine Leap year here for
                 * year value is not parsed yet
                 */
                day = strtoul(buf, NULL, 10);
                if (day <= 0)
                {
                    return cp;
                }
                if (2 == month)
                {
                    if (day > (g_max_day_for_month[1] + 1)) 
                    {
                        return cp - 1;
                    }
                }
                else if (day > g_max_day_for_month[month - 1])
                {
                    return cp - 1;
                }
                break;

            default:
                return cp;
            }

            nums++;
        }
            
        if (DELIMITER (*cp))
        {
            break;
        }     
        cp++;
    }     
     
    if (nums < 3)
    {
        *is_match = FALSE;
        return cp;
    }
    
    if (2 == month)
    {
        if (!ISLEAP(year) && day > g_max_day_for_month[1])
        {
            cp -= 6; /* pointer to day field */
            return cp;
        }
    }
    *is_match = TRUE;
    return cp;
}

char*
ctc_month_day_match(char *cp, uint32 *is_match)
{
    char *sp = NULL;
    int colons = 0;
    int nums = 0;
    char buf[7];
    unsigned long year = 0;
    unsigned long month = 0;
    unsigned long day = 0;
    time_t tCur = time(NULL);
    struct tm tmCur;
    int idx = 0;

    *is_match = FALSE;
    localtime_r(&tCur, &tmCur);
    year = tmCur.tm_year + 1900;

    while (1)
    {
        sal_memset(buf, 0, sizeof (buf));
        sp = cp;
        while (! DELIMITER (*cp))
        {
            if (*cp == '/')
            {
                if (colons >= 1)
                {
                    return cp;
                }

                if (*(cp + 1) == '/')
                {
                    return cp;
                }

                colons++;
                break;
            }
            
            if (!sal_isdigit((int) *cp))
            {
                return cp;
            }

            cp++;
        }

        if (cp - sp > 4)
        {
            return cp;
        }

        idx++;
        if (cp - sp)
        {
            sal_strncpy(buf, sp, cp - sp);
            switch (idx)
            {
            case 1:
                /* for month field */
                month = strtoul(buf, NULL, 10);
                if (month <= 0 || month > 12)
                {
                    return cp;
                }
                break;
            case 2:
                /* for day field */
                day = strtoul(buf, NULL, 10);
                if (day <= 0)
                {
                    return cp;
                }
                if (2 == month && ISLEAP(year))
                {
                    if (day > (g_max_day_for_month[month - 1] + 1))
                    {
                        return cp - 1;
                    }
                }
                else if (day > g_max_day_for_month[month - 1])
                {
                    return cp - 1;
                }
                break;

            default:
                return cp;
            }

            nums++;
        }
            
        if (DELIMITER (*cp))
        {
            break;
        }     
        cp++;
    }     
     
    if (nums < 2)
    {
        *is_match = FALSE;
        return cp;
    }
    
    *is_match = TRUE;
    return cp;
}

int32
ctc_str2tm(char *cp, struct sal_tm *tm)
{
    uint32 match = FALSE;
    char *sp = NULL;

    if (NULL == cp || NULL == tm)
    {
        return PM_E_INVALID_PARAM;
    }

    sal_memset(tm, 0x0, sizeof(*tm));
    
    /* YYYY/MM/DD format */
    sp = ctc_chinese_date_match(cp, &match);
    if (match && '\0' == *sp)
    {
        strptime(cp, "%Y/%m/%d", tm);
        return PM_E_NONE;
    }

    /* MM/DD/YYYY format */
    sp = ctc_english_date_match(cp, &match);
    if (match && '\0' == *sp)
    {
        strptime(cp, "%m/%d/%Y", tm);
        return PM_E_NONE;
    }

    /* MM/DD format */
    sp = ctc_month_day_match(cp, &match);
    if (match && '\0' == *sp)
    {
        time_t tCur = time(NULL);
        struct tm tmCur;
        char dbuf[32];

        localtime_r(&tCur, &tmCur);
        snprintf(dbuf, 32, "%d/%s", tmCur.tm_year + 1900, cp);
        strptime(dbuf, "%Y/%m/%d", tm);
        return PM_E_NONE;
    }

    return PM_E_INVALID_PARAM;
}

void
chsm_set_schedule_reboot_alarm_fn(void *p_data)
{
    tbl_chassis_t *p_chassis = tbl_chassis_get_chassis();

    p_chassis->one_minute_reboot_alarm = TRUE;
    tbl_chassis_set_chassis_field(p_chassis, TBL_CHASSIS_FLD_ONE_MINUTE_REBOOT_ALARM);
    
    return;
}

void
chsm_set_schedule_reboot_fn(void *p_data)
{
    tbl_chassis_t *p_chassis = tbl_chassis_get_chassis();

    p_chassis->schedule_reboot_timer = NULL;
        
    /* refer chsm_cmd_process_reboot */
    snmp_trap_reboot_send(1);
    /* added by tongzb for bug 40719 0x05 means SCHEDULE reboot, 2016-09-12 */
    system("reboot_record 0x05 1>/dev/null 2>&1 && sync");
    chsm_reset_system();

    return;
}

int32
chsm_set_schedule_reboot(int32 interval)
{
    tbl_chassis_t *p_chassis = tbl_chassis_get_chassis();
    int32 alarm_interval = 0;

    CTC_TASK_STOP_TIMER(p_chassis->schedule_reboot_timer);
    CTC_TASK_STOP_TIMER(p_chassis->schedule_reboot_alarm_timer);

    if (REBOOT_ONE_MINUTE_SECONDS < interval)
    {
        alarm_interval = interval - REBOOT_ONE_MINUTE_SECONDS;
        p_chassis->schedule_reboot_alarm_timer = ctc_task_add_timer(TASK_PRI_NORMAL, chsm_set_schedule_reboot_alarm_fn, NULL, alarm_interval);
    }
    p_chassis->schedule_reboot_timer = ctc_task_add_timer(TASK_PRI_NORMAL, chsm_set_schedule_reboot_fn, NULL, interval);

    p_chassis->schedule_reboot = TRUE;
    tbl_chassis_set_chassis_field(p_chassis, TBL_CHASSIS_FLD_SCHEDULE_REBOOT);

    return PM_E_NONE;
}

int32
chsm_cmd_set_schedule_reboot_at(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 hour = 0;
    uint32 minute = 0;
    uint32 i = 0;
    uint32 str_len = 0;
    char date_str[CMD_BUF_256];
    struct sal_tm tm;
    struct sal_tm tmCur;
    sal_time_t nCur = sal_time(NULL);
    int32 sec_tm = 0;
    int32 sec_tmCur = 0;
    int32 interval = 0;
    int32 rc = PM_E_NONE;
    
    localtime_r(&nCur, &tmCur);
    
    /* HH:MM */
    if (2 != sal_sscanf(argv[2], "%u:%u", &hour, &minute))
    {
        CFG_INVALID_PARAM_RET("Invalid time parameter %s", argv[2]);
    }

    sal_memset(date_str, 0, sizeof(date_str));
    if (argc > 6)
    {
        CFG_INVALID_PARAM_RET("Invalid date parameter");
    }

    if (argc > 3)
    {
        for (i = 3; i < argc; i++)
        {
            sal_strcat(date_str, argv[i]);
            sal_strcat(date_str, "/");
        }
        str_len = sal_strlen(date_str);
        if (str_len)
        {
            date_str[str_len-1] = CMD_ZERO_CHAR;
            str_len = sal_strlen(date_str);
        }
        if (str_len > 10)
        {
            CFG_INVALID_PARAM_RET("Invalid date parameter");
        }

        rc = ctc_str2tm(date_str, &tm);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid date parameter %s", date_str);
        }
    }
    else
    {
        sal_memcpy(&tm, &tmCur, sizeof(tm));
    }
    tm.tm_sec = 0;
    tm.tm_min = minute;
    tm.tm_hour = hour;

    sec_tm = sal_time_mk(&tm);
    sec_tmCur = sal_time_mk(&tmCur);
    interval = sec_tm - sec_tmCur;

#if 0
    log_sys(M_MOD_CHSM, E_ERROR, "tm(%d/%d/%d %d:%d), tmCur(%d/%d/%d %d:%d)", 
        tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min,
        tmCur.tm_year, tmCur.tm_mon, tmCur.tm_mday, tmCur.tm_hour, tmCur.tm_min);
    log_sys(M_MOD_CHSM, E_ERROR, "sec_tm %d, sec_tmCur %d, interval %d", sec_tm, sec_tmCur, interval);
#endif
    if (interval <= 0)
    {
        CFG_INVALID_PARAM_RET("Past time error");
    }

    chsm_set_schedule_reboot(interval);

    return rc;
}

int32
chsm_cmd_set_schedule_reboot_delay(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 hour = 0;
    uint32 minute = 0;
    int32 interval = 0;
    int32 rc = PM_E_NONE;
    
    /* HH:MM */
    if (sal_strchr(argv[2], ':'))
    {
        if (2 != sal_sscanf(argv[2], "%u:%u", &hour, &minute))
        {
            CFG_INVALID_PARAM_RET("Invalid time parameter %s", argv[2]);
        }
        interval = hour * REBOOT_ONE_HOUR_MINUTES + minute;
    }
    else
    {
        PM_CFG_GET_UINT_RANGE(interval, 1, 720, argv[2], para->p_rs);
    }

    interval = interval * REBOOT_ONE_MINUTE_SECONDS;
    chsm_set_schedule_reboot(interval);

    return PM_E_NONE;
}

int32
chsm_cmd_set_schedule_reboot_disable(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_chassis_t *p_chassis = tbl_chassis_get_chassis();

    CTC_TASK_STOP_TIMER(p_chassis->schedule_reboot_timer);
    CTC_TASK_STOP_TIMER(p_chassis->schedule_reboot_alarm_timer);

    p_chassis->schedule_reboot = FALSE;
    tbl_chassis_set_chassis_field(p_chassis, TBL_CHASSIS_FLD_SCHEDULE_REBOOT);
    
    return PM_E_NONE;
}

int32
chsm_cmd_process_chassis(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_field_t *p_field = NULL;
    cdb_node_t* p_node = NULL;  
    tbl_chassis_t* p_db_chassis = NULL;
    int32 field_id = 0;
    uint32 type = 0;
    int32 rc = PM_E_NONE;
        
    p_db_chassis = tbl_chassis_get_chassis();
    p_node = cdb_get_tbl(TBL_CHASSIS);
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }
        switch(field_id)
        {
            case TBL_CHASSIS_FLD_LOCATION_LED:
                if(!sal_strncmp(argv[1], "on", 2))
                {
                    p_db_chassis->location_led = 1;
                }
                else if(!sal_strncmp(argv[1], "off", 3))
                {
                    p_db_chassis->location_led = 0;
                }
                else
                {
                    CFG_INVALID_PARAM_RET("Unknown command");
                }
                tbl_chassis_set_chassis_field(p_db_chassis, TBL_CHASSIS_FLD_LOCATION_LED);           
                break;

            case TBL_CHASSIS_FLD_NEXT_STM_MODE:
                PM_E_RETURN(_chsm_stm_mgr_cmd_map_sys_spec_type(argv, argc, para, &type));
                p_db_chassis->next_stm_mode = type;
                tbl_chassis_set_chassis_field(p_db_chassis, TBL_CHASSIS_FLD_NEXT_STM_MODE);           
                break;

            case TBL_CHASSIS_FLD_RESET_REBOOT_INFO:
                p_db_chassis->reset_reboot_info = !p_db_chassis->reset_reboot_info;
                tbl_chassis_set_chassis_field(p_db_chassis, TBL_CHASSIS_FLD_RESET_REBOOT_INFO);           
                break;

            case TBL_CHASSIS_FLD_SCHEDULE_REBOOT_TIMER:
                if (0 == sal_strcasecmp("at", argv[1]))
                {
                    PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
                    rc = chsm_cmd_set_schedule_reboot_at(argv, argc, para);
                }
                else if (0 == sal_strcasecmp("delay", argv[1]))
                {
                    PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
                    rc = chsm_cmd_set_schedule_reboot_delay(argv, argc, para);
                }
                else if (0 == sal_strcasecmp("disable", argv[1]))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
                    rc = chsm_cmd_set_schedule_reboot_disable(argv, argc, para);
                }
                else
                {
                    CFG_CONFLICT_RET("Invalid schedule reboot parameter");
                }
                break;
                
            default:
                CFG_CONFLICT_RET("Unsupport action");
                break;
        
        }
        break;
        
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _chsm_cmd_get_chassis(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
chsm_cmd_show_reboot_info(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    p_field->field_id[0] = FLD_ID_ALL;    
    args.argv[0] = p_field;
    args.argv[1] = fp;    
    tbl_card_iterate((TBL_ITER_CB_FUNC)tbl_card_dump_one, &args);
    sal_fclose(fp);
  
    return PM_E_NONE;
}

int32
chsm_cmd_process_card(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    chsm_tmpr_cfg_t tmpr_cfg;
    chsm_fan_cfg_t fan_cfg;
    chsm_poe_cfg_t poe_cfg;
    cdb_field_t *p_field = NULL;
    int32 field_id = 0;
    int32 rc = PM_E_NONE;
    
    p_node = cdb_get_tbl(TBL_CARD);

    switch (para->oper)
    {
    case CDB_OPER_GET:      
        if(argc > 0)
        {
            field_id = cdb_get_field_id(argv[0], p_node, &p_field);
            if (field_id < 0)
            {    
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
            }
        }
        else           
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
            rc = _chsm_cmd_get_card(para, p_node, &field);
        }
        break;
    case CDB_OPER_SET:
        if(argc > 0)
        {
            field_id = cdb_get_field_id(argv[0], p_node, &p_field);
            if (field_id < 0)
            {    
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
            }
            switch(field_id)
            {
                case TBL_CARD_FLD_TMPR_LOW:
                case TBL_CARD_FLD_TMPR_HIGH:
                case TBL_CARD_FLD_TMPR_CRITICAL:
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 6, para->p_rs);
                    rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
                    if (rc < 0)
                    {
                        CFG_INVALID_PARAM_RET("Invalid field");
                    }
                    PM_CFG_GET_INTEGER(tmpr_cfg.low, argv[1], para->p_rs); 
                    PM_CFG_GET_INTEGER(tmpr_cfg.high, argv[3], para->p_rs); 
                    PM_CFG_GET_INTEGER(tmpr_cfg.crit, argv[5], para->p_rs);
                    if (tmpr_cfg.crit < (tmpr_cfg.high+5))
                    {
                        CFG_INVALID_PARAM_RET("Critical temperature threshold must be larger than high temperature at least 5 Celsius degrees");
                    }
                    if (tmpr_cfg.high < (tmpr_cfg.low+30))
                    {
                        CFG_INVALID_PARAM_RET("High temperature threshold must be larger than low temperature at least 30 Celsius degrees");
                    }
                    rc = chsm_set_tmpr_threshold(&tmpr_cfg);
                    break;
                case TBL_CARD_FLD_FAN_SPEED_BOTTOM:
                case TBL_CARD_FLD_FAN_SPEED_LOW:
                case TBL_CARD_FLD_FAN_SPEED_HIGH:
                case TBL_CARD_FLD_FAN_SPEED_FULL:
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 8, para->p_rs);
                    rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
                    if (rc < 0)
                    {
                        CFG_INVALID_PARAM_RET("Invalid field");
                    }
                    PM_CFG_GET_INTEGER(fan_cfg.bottom, argv[1], para->p_rs); 
                    PM_CFG_GET_INTEGER(fan_cfg.low, argv[3], para->p_rs); 
                    PM_CFG_GET_INTEGER(fan_cfg.high, argv[5], para->p_rs);
                    PM_CFG_GET_INTEGER(fan_cfg.full, argv[7], para->p_rs); 
                    if (fan_cfg.low <= fan_cfg.bottom)
                    {
                        CFG_INVALID_PARAM_RET("The low speed must be larger than bottom speed.");
                    }
                    if (fan_cfg.high <= fan_cfg.low)
                    {
                        CFG_INVALID_PARAM_RET("The high speed must be larger than low speed.");
                    }
                    if (fan_cfg.full <= fan_cfg.high)
                    {
                        CFG_INVALID_PARAM_RET("The full speed must be larger than high speed.");
                    }
                    if ((fan_cfg.bottom%10)||(fan_cfg.low%10)||(fan_cfg.high%10)||(fan_cfg.full%10))
                    {
                        CFG_INVALID_PARAM_RET("The speed must step with 10.");
                    } 

                    rc = chsm_set_fan_speed(&fan_cfg);
                    break;    
                case TBL_CARD_FLD_GLB_POE_ENABLE:
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
                    rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
                    if (rc < 0)
                    {
                        CFG_INVALID_PARAM_RET("Invalid field");
                    }
                    PM_CFG_GET_BOOL(poe_cfg.enable, argv[1], para->p_rs); 
                    rc = chsm_set_sys_poe_enable(para,&poe_cfg);
                    break;
                case TBL_CARD_FLD_GLB_POE_POWER_MAX:
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
                    rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
                    if (rc < 0)
                    {
                        CFG_INVALID_PARAM_RET("Invalid field");
                    }
                    PM_CFG_GET_UINT(poe_cfg.power_max, argv[1], para->p_rs); 
                    rc = chsm_set_sys_poe_max(para,&poe_cfg);
                    break;                        
            }
        }
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}


static int32
_chsm_cmd_get_fan(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_fan_iterate((TBL_ITER_CB_FUNC)tbl_fan_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}


int32
chsm_cmd_process_fan(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    cdb_field_t *p_field = NULL;
    int32 field_id = 0;
    int32 rc = PM_E_NONE;
    
    p_node = cdb_get_tbl(TBL_FAN);

    switch (para->oper)
    {
    case CDB_OPER_GET:      
        if(argc > 0)
        {
            field_id = cdb_get_field_id(argv[0], p_node, &p_field);
            if (field_id < 0)
            {    
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
            }
        }
        else           
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
            rc = _chsm_cmd_get_fan(para, p_node, &field);
        }
        break;
    case CDB_OPER_SET:
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_chsm_cmd_get_psu(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_psu_iterate((TBL_ITER_CB_FUNC)tbl_psu_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}


int32
chsm_cmd_process_psu(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    cdb_field_t *p_field = NULL;
    int32 field_id = 0;
    int32 rc = PM_E_NONE;
    
    p_node = cdb_get_tbl(TBL_FAN);

    switch (para->oper)
    {
    case CDB_OPER_GET:      
        if(argc > 0)
        {
            field_id = cdb_get_field_id(argv[0], p_node, &p_field);
            if (field_id < 0)
            {    
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
            }
        }
        else           
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
            rc = _chsm_cmd_get_psu(para, p_node, &field);
        }
        break;
    case CDB_OPER_SET:
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_chsm_cmd_get_sensor(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_sensor_iterate((TBL_ITER_CB_FUNC)tbl_sensor_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}


int32
chsm_cmd_process_sensor(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    cdb_field_t *p_field = NULL;
    int32 field_id = 0;
    int32 rc = PM_E_NONE;
    
    p_node = cdb_get_tbl(TBL_FAN);

    switch (para->oper)
    {
    case CDB_OPER_GET:      
        if(argc > 0)
        {
            field_id = cdb_get_field_id(argv[0], p_node, &p_field);
            if (field_id < 0)
            {    
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
            }
        }
        else           
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
            rc = _chsm_cmd_get_sensor(para, p_node, &field);
        }
        break;
    case CDB_OPER_SET:
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_chsm_cmd_get_vcm(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_vcm_iterate((TBL_ITER_CB_FUNC)tbl_vcm_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}


int32
chsm_cmd_process_vcm(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    cdb_field_t *p_field = NULL;
    int32 field_id = 0;
    int32 rc = PM_E_NONE;
    
    p_node = cdb_get_tbl(TBL_VCM);

    switch (para->oper)
    {
    case CDB_OPER_GET:      
        if(argc > 0)
        {
            field_id = cdb_get_field_id(argv[0], p_node, &p_field);
            if (field_id < 0)
            {    
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
            }
        }
        else           
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
            rc = _chsm_cmd_get_vcm(para, p_node, &field);
        }
        break;
    case CDB_OPER_SET:
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_chsm_cmd_get_poe(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_poe_iterate((TBL_ITER_CB_FUNC)tbl_poe_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}

int32
chsm_cmd_process_poe(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    cdb_field_t *p_field = NULL;
    int32 field_id = 0;
    int32 rc = PM_E_NONE;
    
    p_node = cdb_get_tbl(TBL_VCM);

    switch (para->oper)
    {
    case CDB_OPER_GET:      
        if(argc > 0)
        {
            field_id = cdb_get_field_id(argv[0], p_node, &p_field);
            if (field_id < 0)
            {    
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
            }
        }
        else           
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
            rc = _chsm_cmd_get_poe(para, p_node, &field);
        }
        break;
    case CDB_OPER_SET:
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_chsm_cmd_get_reboot_info(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_reboot_info_iterate((TBL_ITER_CB_FUNC)tbl_reboot_info_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}


int32
chsm_cmd_process_reboot_info(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    cdb_field_t *p_field = NULL;
    int32 field_id = 0;
    int32 rc = PM_E_NONE;
    
    p_node = cdb_get_tbl(TBL_FAN);

    switch (para->oper)
    {
    case CDB_OPER_GET:      
        if(argc > 0)
        {
            field_id = cdb_get_field_id(argv[0], p_node, &p_field);
            if (field_id < 0)
            {    
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
            }
        }
        else           
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
            rc = _chsm_cmd_get_reboot_info(para, p_node, &field);
        }
        break;
    case CDB_OPER_SET:
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}


#define _CHSM_TBL_PORT_

static int32
_chsm_cmd_get_port(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_port_iterate((TBL_ITER_CB_FUNC)tbl_port_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_chsm_port_clear_media_type(tbl_port_t *p_db_port, tbl_port_t *p_port)
{
    p_port->port_media_type = GLB_PORT_MEDIA_TYPE_DEFAULT;
    
    tbl_port_set_port_field(p_port, TBL_PORT_FLD_PORT_MEDIA_TYPE);

    return PM_E_NONE;
}

int32
chsm_port_set_split_type(tbl_port_t *p_db_port, tbl_port_t *p_port)
{
    if (GLB_PPT_NUMCHG_TYPE_NONE == p_port->split_type)
    {
        _chsm_port_clear_media_type(p_db_port, p_port);
    }

    tbl_port_set_port_field(p_port, TBL_PORT_FLD_SPLIT_TYPE);

    return PM_E_NONE;
}

int32
chsm_port_set_media_type(tbl_port_t *p_db_port, tbl_port_t *p_port)
{
    tbl_port_set_port_field(p_port, TBL_PORT_FLD_PORT_MEDIA_TYPE);

    return PM_E_NONE;
}

int32
chsm_port_set_training_enable(tbl_port_t *p_db_port, tbl_port_t *p_port)
{
    tbl_port_set_port_field(p_port, TBL_PORT_FLD_TRAINING_ENABLE);

    return PM_E_NONE;
}

int32
chsm_port_set_bypass(tbl_port_t *p_db_port, tbl_port_t *p_port)
{
    tbl_port_set_port_field(p_port, TBL_PORT_FLD_BYPASS);
    tbl_port_set_port_field(p_port, TBL_PORT_FLD_BYPASS_SLOT);

    return PM_E_NONE;
}

static int32
_chsm_port_cmd_set_split_type(tbl_port_t *p_db_port, tbl_port_t *p_port, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_global = tbl_sys_global_get_sys_global();
    int32 rc = PM_E_NONE;

    if (PORT_NUMCHG_TYPE_40G == p_port->split_type)
    {
        CFG_INVALID_PARAM_RET("Unsupport split to 40giga");
    }
    if ((GLB_CHIP_GREATEBELT == tbl_chip_get_type())
         ||(GLB_CHIP_GOLDENGATE == tbl_chip_get_type()))
    {
        if ((PORT_NUMCHG_TYPE_50G == p_port->split_type)
            ||(PORT_NUMCHG_TYPE_25G == p_port->split_type))
        {
            /* for bug 51475 */
            CFG_INVALID_PARAM_RET("Unsupport split to %s",
                cdb_enum_val2str(glb_ppt_numchg_type_strs, GLB_PPT_NUMCHG_TYPE_MAX, p_port->split_type));
        }
    }
    else if((GLB_CHIP_DUET2 == tbl_chip_get_type())
         ||(GLB_CHIP_TSINGMA == tbl_chip_get_type()))
    {
        if (PORT_NUMCHG_TYPE_50G == p_port->split_type)
        {
            /* for bug 51475 */
            CFG_INVALID_PARAM_RET("Unsupport split to %s",
                cdb_enum_val2str(glb_ppt_numchg_type_strs, GLB_PPT_NUMCHG_TYPE_MAX, p_port->split_type));
        }
    }
    if (p_db_port->split_type == p_port->split_type)
    {
        return PM_E_NONE;
    }
    
    if (!(GLB_SUPPORT_SPEED_40G & p_db_port->speed_capbility) && (!p_db_port->panel_sub_port_no))
    {
        CFG_INVALID_PARAM_RET("Port is unsupport to split");
    }

    rc = chsm_port_set_split_type(p_db_port, p_port);
    if (p_global->startup_done)
    {
        CFG_PROMPT_RET("Notice: Configuration of split interface should be writen in startup-config, and take effect at next reload");
    }

    return rc;
}

static int32
_chsm_port_cmd_set_media_type(tbl_port_t *p_db_port, tbl_port_t *p_port, char *ifname, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_global = tbl_sys_global_get_sys_global();
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;    

    if (!p_db_port->support_media_switch)
    {
        CFG_INVALID_PARAM_RET("Unsupport media type configure on %s", IFNAME_ETH2FULL(ifname, ifname_ext));
    }

    if (GLB_PPT_NUMCHG_TYPE_NONE == p_db_port->split_type)
    {
        CFG_INVALID_PARAM_RET("Please split %s first", IFNAME_ETH2FULL(ifname, ifname_ext));
    }
    
    if (p_db_port->port_media_type == p_port->port_media_type)
    {
        return PM_E_NONE;
    }

    rc = chsm_port_set_media_type(p_db_port, p_port);
    if (p_global->startup_done)
    {
        CFG_PROMPT_RET("Notice: Configuration of switch interface should be writen in startup-config, and take effect at next reload");
    }
    return rc;
}

static int32
_chsm_port_cmd_set_training_enable(tbl_port_t *p_db_port, tbl_port_t *p_port, char *ifname, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];

    if(!GLB_FLAG_ISSET(p_db_port->speed_capbility, PORT_SUPPORT_SPEED_10G)
        && !GLB_FLAG_ISSET(p_db_port->speed_capbility, PORT_SUPPORT_SPEED_40G)
        && !GLB_FLAG_ISSET(p_db_port->speed_capbility, PORT_SUPPORT_SPEED_100G))
    {
        CFG_INVALID_PARAM_RET("Unsupport 802.3 AP training on %s", IFNAME_ETH2FULL(ifname, ifname_ext));
    }
    if(p_db_port->training_enable == p_port->training_enable)
    {
        return PM_E_NONE;
    }

    return chsm_port_set_training_enable(p_db_port, p_port);
}

static int32
_chsm_port_cmd_set_bypass(tbl_port_t *p_db_port, tbl_port_t *p_port, cfg_cmd_para_t *para)
{
    if(p_db_port->bypass == p_port->bypass)
    {
        return PM_E_NONE;
    }

    return chsm_port_set_bypass(p_db_port, p_port);
}

int32
chsm_cmd_process_port(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_port_t *p_db_port = NULL;
    tbl_port_t *p_db_port2 = NULL;
    tbl_port_key_t port_key;
    tbl_port_t port,port2;
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 rc = PM_E_NONE;
    uint8 slot_no = 0;
    uint8 pannel_port_no = 0;
    uint8 sub_pannel_port_no = 0;
    tbl_port_master_t *p_master = tbl_port_get_master();
    tbl_card_t *p_db_card = tbl_card_get_default_card();    
    uint32 port_idx = 0;
    uint8 bypass_port_cnt =0;
    int32 bypass_slot = 0;
    p_node = cdb_get_tbl(TBL_PORT);

    switch (para->oper)
    {
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _chsm_cmd_get_port(para, p_node, &field);
        break;
    case CDB_OPER_SET:
        sal_memset(&port_key, 0, sizeof(port_key));
        sal_memset(&port, 0, sizeof(port));
        sal_memset(&port2, 0, sizeof(port2));

        field_id = cdb_get_field_id(argv[1], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[1]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        if(TBL_PORT_FLD_BYPASS == field_id)
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 5, para->p_rs);
        }
        else
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
        }
        if(TBL_PORT_FLD_BYPASS == field_id)
        {
            if(!((p_db_card->board_series == GLB_SERIES_E350)&&(p_db_card->board_type == GLB_BOARD_E350_HF_12T4S8X)))
            {
                CFG_INVALID_PARAM_RET("Board not support configure bypass");
            }
            PM_CFG_GET_UINT(bypass_slot, argv[4], para->p_rs);
            for (port_idx = 0; port_idx < GLB_CARD_PORT_NUM_MAX; port_idx++)
            {
                if (NULL == p_master->port_array[port_idx])
                {
                    continue;
                }

                if ((p_master->port_array[port_idx]->bypass)&&(p_master->port_array[port_idx]->bypass_slot == bypass_slot))
                {
                    bypass_port_cnt++;
                    if(bypass_port_cnt == 1)
                        p_db_port = p_master->port_array[port_idx];
                    else if (bypass_port_cnt == 2)
                        p_db_port2 = p_master->port_array[port_idx];
                }
            }
            if (0 == sal_strcmp(argv[2], "none"))
            {
                if (2 == bypass_port_cnt)
                {
                    port.panel_sub_port_no = p_db_port->panel_sub_port_no;
                    port.slot_no = p_db_port->slot_no;
                    port.panel_port_no = p_db_port->panel_port_no;
                    port.key.port_id = p_db_port->key.port_id;
                    port.bypass = 0;
                    port2.panel_sub_port_no = p_db_port2->panel_sub_port_no;
                    port2.slot_no = p_db_port2->slot_no;
                    port2.panel_port_no = p_db_port2->panel_port_no;
                    port2.key.port_id = p_db_port2->key.port_id;
                    port2.bypass = 0;                    
                    rc = _chsm_port_cmd_set_bypass(p_db_port, &port, para);
                    rc = _chsm_port_cmd_set_bypass(p_db_port2, &port2, para);
                    return rc;
                }
                else
                    return rc;
            }
            else 
            {
                if (2 == bypass_port_cnt)
                {
                    CFG_INVALID_PARAM_RET("Bypass on slot %d is already be configured", bypass_slot);
                }
            }
        }
        
        if_get_slot_port_by_ethname(argv[0], (uint8*)&slot_no, (uint8*)&pannel_port_no, (uint8*)&sub_pannel_port_no);
        /* Added by kcao for bug 38586 */
        if (TBL_PORT_FLD_SPLIT_TYPE == field_id)
        {
            if (0 != sub_pannel_port_no)
            {
                CFG_INVALID_PARAM_RET("Cannot split splited interface");
            }
        }

        port.panel_sub_port_no = 0; /* strip subportno eth-0-X/Y -> eth-0-X */
        port.slot_no = slot_no;
        port.panel_port_no = pannel_port_no;
        p_db_port = tbl_port_get_port_by_slot_port(port.slot_no, port.panel_port_no, port.panel_sub_port_no);
        if (NULL == p_db_port)
        {
            if (TBL_PORT_FLD_SPLIT_TYPE == field_id || TBL_PORT_FLD_PORT_MEDIA_TYPE == field_id)
            {
                /* after reboot, split interface eth-0-1 config should be stored in eth-0-1/1 */
                port.panel_sub_port_no = 1;
                p_db_port = tbl_port_get_port_by_slot_port(port.slot_no, port.panel_port_no, port.panel_sub_port_no);
                if (NULL == p_db_port)
                {
                    CFG_INVALID_PARAM_RET("Interface %s not exist", argv[0]);
                }
            }
            else
            {
                CFG_INVALID_PARAM_RET("Interface %s not exist", argv[0]);
            }
        }
        port.key.port_id = p_db_port->key.port_id;
        
        switch (field_id)
        {
        case TBL_PORT_FLD_SPLIT_TYPE:
            if (0 == sal_strcmp(argv[2], "none"))
            {
                port.split_type = PORT_NUMCHG_TYPE_NONE;
            }
            else if (0 == sal_strcmp(argv[2], "10giga"))
            {
                port.split_type = PORT_NUMCHG_TYPE_10G;
            }
            else if (0 == sal_strcmp(argv[2], "40giga"))
            {
                port.split_type = PORT_NUMCHG_TYPE_40G;
            }
            else if (0 == sal_strcmp(argv[2], "25giga"))
            {
                port.split_type = PORT_NUMCHG_TYPE_25G;
            }
            else if (0 == sal_strcmp(argv[2], "50giga"))
            {
                port.split_type = PORT_NUMCHG_TYPE_50G;
            }            
            else
            {
                CFG_INVALID_PARAM_RET("Invalid split type %s", argv[2]);
            }
            rc = _chsm_port_cmd_set_split_type(p_db_port, &port, para);
            break;

        case TBL_PORT_FLD_PORT_MEDIA_TYPE:
            if (0 == sal_strcmp(argv[2], "none"))
            {
                port.port_media_type = PORT_MEDIA_TYPE_DEFAULT;
            }
            else if (0 == sal_strcmp(argv[2], "sfp"))
            {
                port.port_media_type = PORT_MEDIA_TYPE_SFP;
            }
            else if (0 == sal_strcmp(argv[2], "qsfp"))
            {
                port.port_media_type = PORT_MEDIA_TYPE_QSFP;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid switch type %s", argv[2]);
            }
            rc = _chsm_port_cmd_set_media_type(p_db_port, &port, argv[0], para);
            break;
        case TBL_PORT_FLD_TRAINING_ENABLE:
            if(0 == sal_strcmp(argv[2], "1"))
            {
                port.training_enable = 1;
            }
            else
            {
                port.training_enable = 0;
            }
            rc = _chsm_port_cmd_set_training_enable(p_db_port, &port, argv[0], para);
            break;
        case TBL_PORT_FLD_BYPASS:
            if_get_slot_port_by_ethname(argv[2], (uint8*)&slot_no, (uint8*)&pannel_port_no, (uint8*)&sub_pannel_port_no);
            port2.panel_sub_port_no = sub_pannel_port_no;
            port2.slot_no = slot_no;
            port2.panel_port_no = pannel_port_no;
            p_db_port2 = tbl_port_get_port_by_slot_port(port2.slot_no, port2.panel_port_no, port2.panel_sub_port_no);
            if (NULL == p_db_port2)
            {
                CFG_INVALID_PARAM_RET("Interface %s not exist", argv[2]);
            }
            port2.key.port_id = p_db_port2->key.port_id;
            port.bypass = 1;
            port2.bypass = 1;
            port.bypass_slot = bypass_slot;
            port2.bypass_slot = bypass_slot;
            if(port2.panel_port_no == port.panel_port_no)
            {
                CFG_INVALID_PARAM_RET("Can not set bypass for two same Interface");
            }
            if((p_db_port2->bypass)&&(p_db_port2->bypass_slot != bypass_slot))
            {
                CFG_INVALID_PARAM_RET("Bypass is already be configured on Interface %s slot %d", argv[2],p_db_port2->bypass_slot);
            }
            if((p_db_port->bypass)&&(p_db_port->bypass_slot != bypass_slot))
            {
                CFG_INVALID_PARAM_RET("Bypass is already be configured on Interface %s slot %d", argv[0],p_db_port->bypass_slot);
            }
            rc = _chsm_port_cmd_set_bypass(p_db_port, &port, para);
            rc = _chsm_port_cmd_set_bypass(p_db_port2, &port2, para);
            break;
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[1]);
            break;
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

#define _CHSM_TBL_FIBER_INFO_

int32
chsm_cmd_process_fiber_info(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint8 slot = 0xff;
    uint8 port = 0xff;
    uint8 sub_port = 0;
    uint8 detail = 0;
    int32 rc = PM_E_NONE;
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (argc == 0)
        {
            port = GLB_CARD_PORT_NUM_MAX;
        }
        else
        {
            if (!sal_strncmp (argv[0], GLB_IFNAME_ETH_PREFIX, 4)) /*TODO liangf*/
            {
                if_get_slot_port_by_ethname(argv[0], &slot, &port, &sub_port);
            }
            else if (!sal_strncmp (argv[0], "all", 3))
            {
                slot = 0xff;
                port = 0xff;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid parameter");
                return PM_E_INVALID_PARAM;
            }

            if (argc == 2)
            {
                detail = 1;
            }
        }
        chsm_port_dump_transceiver(argv[0], slot, port, sub_port, detail, para, TRUE);

        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
chsm_cmd_show_fiber_info(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint8 slot = 0xff;
    uint8 port = 0xff;
    uint8 sub_port = 0;
    uint8 detail = 0;
    int32 rc = PM_E_NONE;
	tbl_interface_key_t if_key;
	tbl_interface_t* p_db_if = NULL;
	char ifname_ext[IFNAME_EXT_SIZE];
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (argc == 0)
        {
            port = GLB_CARD_PORT_NUM_MAX;
        }
        else
        {
            if (!sal_strncmp (argv[0], GLB_IFNAME_ETH_PREFIX, 4)) /*TODO liangf*/
            {
            	sal_memset(&if_key, 0, sizeof(if_key));
	            sal_strncpy(if_key.name, argv[0], IFNAME_SIZE);
	            p_db_if = tbl_interface_get_interface(&if_key);
	            if (NULL == p_db_if)
	            {
	                IFNAME_ETH2FULL(argv[0], ifname_ext);
	                CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
	            } 
                if_get_slot_port_by_ethname(argv[0], &slot, &port, &sub_port);
            }
            else if (!sal_strncmp (argv[0], "all", 3))
            {
                slot = 0xff;
                port = 0xff;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid parameter");
                return PM_E_INVALID_PARAM;
            }

            if (argc == 2)
            {
                detail = 1;
            }
        }
        chsm_port_dump_transceiver(argv[0], slot, port, sub_port, detail, para, FALSE);

        break;
/*Added by zhush for bug 52768, 2019-07-05*/
    case CDB_OPER_GETJSON:
       if (argc == 0)
        {
            port = GLB_CARD_PORT_NUM_MAX;
        }
        else
        {
            if (!sal_strncmp (argv[0], GLB_IFNAME_ETH_PREFIX, 4)) /*TODO liangf*/
            {
            	sal_memset(&if_key, 0, sizeof(if_key));
	            sal_strncpy(if_key.name, argv[0], IFNAME_SIZE);
	            p_db_if = tbl_interface_get_interface(&if_key);
	            if (NULL == p_db_if)
	            {
	                IFNAME_ETH2FULL(argv[0], ifname_ext);
	                CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
	            } 
                if_get_slot_port_by_ethname(argv[0], &slot, &port, &sub_port);
            }
            else if (!sal_strncmp (argv[0], "all", 3))
            {
                slot = 0xff;
                port = 0xff;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid parameter");
                return PM_E_INVALID_PARAM;
            }

            if (argc == 2)
            {
                detail = 1;
            }
        }
        chsm_port_dump_transceiver_json(argv[0], slot, port, sub_port, detail, para, FALSE);

        break;
/*end zhush*/
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

#define _CHSM_ACT_CLOCK_

static int 
_chsm_clock_check_date(int *day, const int nMon, const int nYear)
{
#define ISLEAP(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

    int nDay;
    int ret = 0;

    if(NULL == day)
    {
        return -1;
    }

    /*get argment*/
    nDay = *day;  

    switch(nMon)
    {
        case 2:
        {           
            if(ISLEAP(nYear))
            {
                if(nDay > 29)
                {
                    ret = -1;
                    nDay = 29;
                    break;
                }
            }
            else
            {
                if(nDay > 28)
                {
                    ret = -1;
                    nDay = 28;
                    break;
                }
                else
                    break;
            }
           
        }
        case 4:
        case 6:
        case 9:
        case 11:
        {
            if(nDay > 30)
            {
                ret = -1;
                nDay = 30;
                break;
            }
            else
                break;
        }
        default :
            break;
    }

    *day = nDay;   

    return ret;
}
int32 chsm_set_tzenv()
{
    FILE *pFile = fopen(ZONE_CONFIG_PATH, "r");
    char szBuf[ZONE_LINE_BUF];
    char *newline_position;
            
    if (NULL != pFile)
    {

        while (!feof(pFile))
        {
            fgets(szBuf, ZONE_LINE_BUF, pFile);
            if (memcmp(szBuf, "TZ=",3)==0 && strlen(szBuf)>3)
            {
                /* Should make a TZ=JST-01:00:00 style variable, never include
                   \n or \" character, otherwise tzset will wrongly set daylight saving info because of
                   the extra \n or \" in the end of line. */
                newline_position = strchr(szBuf, '\n');
                if (newline_position)
                {
                    *newline_position = '\0';
                }
                setenv("TZ",szBuf+3,1);
                tzset();
                break;
            }
        }
        fclose(pFile);    
    }
    return PM_E_NONE;
}
int32
_chsm_cmd_get_time_str(char *strTime, uint32 len)
{
    sal_time_t date;

    strTime[0] = CMD_ZERO_CHAR;
    date = sal_time(NULL);
    sal_time_strf(strTime, len, "%T %Z %a %b %d %Y", sal_localtime(&date));
    return PM_E_NONE;
}

int
_chsm_summer_clock_make_time(summer_time_t *ptm, time_t *tmSec)
{
    struct tm tmCur;  /*current date*/
    if((NULL == ptm) || (NULL == tmSec))
    {
        return -1;   
    }    
    
    /*make time*/
    tmCur.tm_year = ptm->tm_year - 1900;
    tmCur.tm_mon = ptm->tm_mon - 1;
    tmCur.tm_mday = ptm->tm_mday;
    tmCur.tm_hour = ptm->tm_hour;
    tmCur.tm_min = ptm->tm_min;
    tmCur.tm_sec = ptm->tm_sec;

    *tmSec = sal_time_mk(&tmCur);

    return 0;
}

int32
chsm_clock_write_profile_timezone(char *strZoneInfo)
{
    FILE *fp = NULL;
    
    if (NULL == strZoneInfo)
    {
        return PM_E_FAIL;
    }

    /*open timezone profile*/
    fp = sal_fopen(ZONE_CONFIG_PATH, SAL_OPEN_RW);
    if (NULL == fp)
    {
        return -1;
    }

    /*write timezone profile*/
    sal_fputs("#!/bin/bash\n", fp);
    sal_fputs("TZ=", fp);
    sal_fputs(strZoneInfo, fp);
    sal_fputs("\n", fp);
    sal_fputs("export TZ", fp);
    sal_fputs("\n", fp);
    /*close timezone profile*/
    sal_fclose(fp);

    chmod(ZONE_CONFIG_PATH, (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH));

    return 0;
}

int 
chsm_clock_notify_other_application()
{
    int nPid;   /*process index*/ 

    for (nPid = 0; nPid < (sizeof(chsm_stProcCtrl)/sizeof(chsm_stProcCtrl[0])); nPid++)
    {
        signal_app((const char*)chsm_stProcCtrl[nPid].strPIDFile, chsm_stProcCtrl[nPid].signal);
    }

    return 0;
}

int32
_chsm_clock_set_timezone(tbl_clock_t *p_clk, uint32 is_add)
{
    char strZone[CMD_BUF_256];
    int32 rc = PM_E_NONE;
    
    sal_memset(strZone, 0, sizeof(strZone));
    
    if (is_add)
    {
        if (p_clk->timezone_positive) 
        {
            sal_sprintf(strZone, "%s-%02u:%02u:%02u", p_clk->timezone_name, 
                p_clk->timezone_hour, p_clk->timezone_minute, p_clk->timezone_second);
        }
        else
        {
            /*negative time zone from UTC*/
            sal_sprintf(strZone, "%s+%02u:%02u:%02u", p_clk->timezone_name, 
                p_clk->timezone_hour, p_clk->timezone_minute, p_clk->timezone_second);
        }
    }
    else
    {
        sal_sprintf(strZone, "%s", "UTC");         
    }  

    rc = setenv("TZ", strZone, 1);    
    if (rc < 0)
    {
        log_sys(M_MOD_LIB, E_ERROR, "Put env failed! ");
        return -1;
    }
    tzset();

    /*syschoronize system time to hardware*/            
    //synchro_time_to_harware ();      
    
    /*must be put before inform other process, write to zone config*/
    chsm_clock_write_profile_timezone(strZone);

    chsm_clock_notify_other_application();

    return rc;
}

int 
_chsm_summer_clock_check_date(int *day, const int nMon, const int nYear, int flag)
{
    int nDay;
    int ret = 0;
    tbl_tag_summer_clock_t *tmCur = tbl_tag_summer_clock_get_tag_summer_clock();;
    if (NULL == tmCur)
    {
       sal_printf("Can not find summer clock table");
       return -1;
    }
    if(NULL == day)
    {
        return -1;
    }

    /*get argment*/
    nDay = *day;  

    switch(nMon)
    {
        case 2:
        {
            if((flag == CHSM_FLAG_DST)&& tmCur->is_recurring)
            {
                if((tmCur->nStartFebDay == 29) ||(tmCur->nEndFebDay == 29))                    
                {
                    if(ISLEAP(nYear))
                        nDay = 29;
                    else
                        nDay = 28;
                    break;
                }
            }
           
            if(ISLEAP(nYear))
            {
                if(nDay > 29)
                {
                    ret = -1;
                    nDay = 29;
                    break;
                }
            }
            else
            {
                if(nDay > 28)
                {
                    ret = -1;
                    nDay = 28;
                    break;
                }
                else
                    break;
            }
           
        }
        case 4:
        case 6:
        case 9:
        case 11:
        {
            if(nDay > 30)
            {
                ret = -1;
                nDay = 30;
                break;
            }
            else
                break;
        }
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
        if (nDay > 31)
        {
            ret = -1;
            nDay = 31;
        }
        break;
        default :
            break;
    }

    *day = nDay;   

    return ret;
}

int32
_chsm_summmer_trans_wday_time_to_localtime(which_summer_time_t *wday_tm, summer_time_t*tm)
{
    struct tm *ptmCur;  /* the first of month date in this year */
    summer_time_t tm_temp;
    time_t tmSec; 
    int32 month_first_day_wady = 0; /* day of the week int the first day of month in this year */
    int32 nYear = wday_tm->wday_tm_year;
    int32 nMon = wday_tm->wday_tm_mon;
    int32 nDay = 0; // get the day later
    int32 nWhich = wday_tm->wday_tm_which;
    int32 nWeekday = wday_tm->wday_tm_wday;
    int32 nHour = wday_tm->wday_tm_hour;
    int32 nMin = wday_tm->wday_tm_min;
    int32 nSec = wday_tm->wday_tm_sec;
    int32 nDay_temp;
    int32 ret;
    
    sal_memset(&tm_temp, 0, sizeof(tm_temp));
    /* set the 1st day of specify month */
    tm_temp.tm_year = nYear;
    tm_temp.tm_mon = nMon;
    tm_temp.tm_mday = 1;    // the first day of this month
    tm_temp.tm_hour = 0;
    tm_temp.tm_min = 0;
    tm_temp.tm_sec = 0; 
    _chsm_summer_clock_make_time(&tm_temp, &tmSec);
    ptmCur = sal_localtime(&tmSec);
    /* day of the week <1-7> 1:Monday 2:Tuesday ... */
    month_first_day_wady = ptmCur->tm_wday;
    /* modify by wulx for bug 47466, 2018-05-30 */
    nDay = 7 * (nWhich - (nWeekday < month_first_day_wady ? 0:1)) + (nWeekday - month_first_day_wady) + 1;
    /* get the current day */
    nDay_temp = nDay;
    ret = _chsm_summer_clock_check_date(&nDay_temp, nMon, nYear, CHSM_FLAG_NO_DST);
    if(ret < 0)
    {
        /* this month not have the fifth wday, so the last is the fourth, not the fifth. one weekday have 7 days */
        nDay -= 7;
    }
    tm->tm_year = nYear;
    tm->tm_mon = nMon;
    tm->tm_mday = nDay;
    tm->tm_hour = nHour;
    tm->tm_min = nMin;
    tm->tm_sec = nSec; 
    return PM_E_NONE;
}
int32
chsm_set_tag_summer_clock(tbl_tag_summer_clock_t *p_summer_clk)
{
    if (NULL == p_summer_clk)
    {
        return PM_E_NONE;
    }
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_STRZONENAME);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_IDSTOFFSET);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_START);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_END);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_TMSTART);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_TMEND);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_IS_RECURRING);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_IS_WDAY_TM);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_NSTARTFEBDAY);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_NENDFEBDAY);
    return PM_E_NONE;
}

int32
_chsm_summer_clock_cal_dst_timer_interval(long int *interval)
{
    time_t date;
    time_t tmSec;  /*time make*/
    char strZoneInfo[CHSM_MAX_TIME_LEN + 1];
    char strZone[CHSM_MAX_ZONE_NAME_LEN + 1];    
    long int nlOffset;
    struct tm *ptmCur = NULL;
    tbl_tag_summer_clock_t *p_db_summer_clk = NULL;
    if(NULL == interval)
    {
        return -1;
    }
    p_db_summer_clk = tbl_tag_summer_clock_get_tag_summer_clock();
    /*get current time and timezone*/
    date = sal_time_sys_current(NULL);
    /*get current timezone from TZ*/
    sal_memset(strZoneInfo, 0, sizeof(strZoneInfo));
    sal_memset(strZone, 0, sizeof(strZone));
    if(NULL == getenv("TZ"))
    {
        sal_strncpy(strZone, "UTC", sizeof(strZone));
        strZone[CHSM_MAX_ZONE_NAME_LEN] = '\0';
    }
    else
    {
        sal_strncpy(strZoneInfo, getenv("TZ"), sizeof(strZoneInfo));
        strZoneInfo[CHSM_MAX_TIME_LEN] = '\0'; 
        sal_sscanf(strZoneInfo, "%s", strZone);  
        strZone[CHSM_MAX_ZONE_NAME_LEN] = '\0';  
    }

   
    date = sal_time_sys_current(NULL);
    if(date == (sal_time_t) - 1)
    {
        log_sys(M_MOD_LIB, E_ERROR, "Unable to get time!errno = %s. ", sal_strerror(errno));
   
    } 

    ptmCur = sal_localtime(&date);
    if(p_db_summer_clk->is_recurring&&p_db_summer_clk->is_wday_tm)
    {
        p_db_summer_clk->wday_tm_start.wday_tm_year = ptmCur->tm_year + 1900;
        p_db_summer_clk->wday_tm_end.wday_tm_year = ptmCur->tm_year + 1900;
        _chsm_summmer_trans_wday_time_to_localtime(&p_db_summer_clk->wday_tm_start, &p_db_summer_clk->tmstart);
        _chsm_summmer_trans_wday_time_to_localtime(&p_db_summer_clk->wday_tm_end, &p_db_summer_clk->tmend);
        if (p_db_summer_clk->wday_tm_start.wday_tm_mon > p_db_summer_clk->wday_tm_end.wday_tm_mon)
        {
            p_db_summer_clk->wday_tm_end.wday_tm_year++;
        }
        p_db_summer_clk->nStartFebDay = p_db_summer_clk->tmstart.tm_mday;
        p_db_summer_clk->nEndFebDay = p_db_summer_clk->tmend.tm_mday;
        chsm_set_tag_summer_clock(p_db_summer_clk);
    }
    else if (p_db_summer_clk->is_recurring && !p_db_summer_clk->is_wday_tm)
    {
        p_db_summer_clk->tmstart.tm_year= ptmCur->tm_year + 1900;
        p_db_summer_clk->tmend.tm_year= ptmCur->tm_year + 1900; 
        if (p_db_summer_clk->wday_tm_start.wday_tm_mon > p_db_summer_clk->wday_tm_end.wday_tm_mon)
        {
            p_db_summer_clk->wday_tm_end.wday_tm_year++;
        }
        chsm_set_tag_summer_clock(p_db_summer_clk);
    }
    
    /*judge whether is summer time*/
    if(!sal_strncmp(strZone, p_db_summer_clk->strZoneName, sal_strlen(p_db_summer_clk->strZoneName)))
    {
        /*current time is summer time, cal the time interval from current time to end summer time*/
        _chsm_summer_clock_make_time(&p_db_summer_clk->tmend, &tmSec);        
    }
    else
    {
        /*current time is NOT summer time, cal the time interval from current time to start summer time*/    
        if(!p_db_summer_clk->is_recurring)
        {
            /*if summer time is not recurring, judge whether end summer time is arrived*/
            _chsm_summer_clock_make_time(&p_db_summer_clk->tmend, &tmSec);
            if((tmSec - (date + p_db_summer_clk->iDstOffset * 60)) < 0)
            {
                *interval = 0;
                return PM_E_NONE;
            }
        }
        _chsm_summer_clock_make_time(&p_db_summer_clk->tmstart, &tmSec);        
    } 

    /*calculate interval*/
    nlOffset = tmSec - date;
    if(nlOffset > 24 * 3600)
    {
        *interval = 24 *3600;
    }
    else if(nlOffset > 3600)
    {
        *interval = 3600;
    }
    else if(nlOffset > 60)
    {
        *interval = 60;
    }
    else
    {
        *interval = 1;
    }
   
    return PM_E_NONE;
}

int32
 _chsm_summer_clock_change_dst_offset(int *minute, int *hour)
{
    long int nlTotalSec = 0;  
    tbl_tag_summer_clock_t *summer_clk = tbl_tag_summer_clock_get_tag_summer_clock();
    tbl_clock_t *p_db_clk = tbl_clock_get_clock();
    if((NULL == minute) || (NULL == hour))
    {
        return -1;
    }
    
    /*in tzset, west is positive, east is negistive, but in our command, 
     west in negtive and east is positive, so here we  need change it*/
    if( OP_POSITIVE == p_db_clk->timezone_positive)
    {
        nlTotalSec = p_db_clk->timezone_hour * 3600 + p_db_clk->timezone_minute * 60 + p_db_clk->timezone_second
                  + summer_clk->iDstOffset * 60;  

        if (nlTotalSec >= 24 * 3600)
        {
             /*should change from east to west*/
             nlTotalSec -= 24 *3600;
             *hour = -(nlTotalSec / 3600);
             *minute = (nlTotalSec / 60)%60;
        }
        else
        {
            *hour = (nlTotalSec / 3600);  
            *minute = (nlTotalSec / 60)%60;
        }
    }
    else
    {
        nlTotalSec = (-p_db_clk->timezone_hour) * 3600 + (-p_db_clk->timezone_minute) * 60 + (-p_db_clk->timezone_second)
                  + summer_clk->iDstOffset * 60; 
        *hour = nlTotalSec / 3600;  
        *minute = (nlTotalSec > 0) ? ((nlTotalSec / 60)%60) : ((-nlTotalSec / 60)%60);
    } 

    return PM_E_NONE;
}
int
_chsm_summer_clock_set_timezone_dst()
{
    char strZone[CHSM_MAX_TIME_LEN + 1]; 
    int nHour, nMin;
    //int ret;
    tbl_tag_summer_clock_t *p_db_summer_clk = NULL;
    tbl_clock_t *p_db_clk = tbl_clock_get_clock();
    sal_memset(strZone, 0, sizeof(strZone));  
    _chsm_summer_clock_change_dst_offset(&nMin, &nHour);            
    p_db_summer_clk = tbl_tag_summer_clock_get_tag_summer_clock();
    p_db_clk->cp_timezone_positive = p_db_clk->timezone_positive;
    if (nHour >= 0)
    {
        sal_sprintf(strZone, "%s-%02u:%02u:%u%u", p_db_summer_clk->strZoneName, nHour, nMin,0,0);
        p_db_clk->timezone_positive = 1;
    }
    else
    {
        sal_sprintf(strZone, "%s+%02u:%02u:%u%u", p_db_summer_clk->strZoneName, (-nHour), nMin,0,0);
        p_db_clk->timezone_positive = 0;
    }
    p_db_clk->cp_timezone_hour = p_db_clk->timezone_hour;
    p_db_clk->cp_timezone_minute = p_db_clk->timezone_minute;
    p_db_clk->cp_timezone_second = p_db_clk->timezone_second;
    sal_strncpy(p_db_clk->cp_timezone_name,p_db_clk->timezone_name,strlen(p_db_clk->timezone_name));
    tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_CP_TIMEZONE_POSITIVE);
    tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_CP_TIMEZONE_HOUR);
    tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_CP_TIMEZONE_MINUTE);
    tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_CP_TIMEZONE_SECOND);
    tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_CP_TIMEZONE_NAME);
    strZone[CHSM_MAX_TIME_LEN] = '\0';
    setenv("TZ", strZone, 1);
    tzset();   
    /*write to zone config*/
    chsm_clock_write_profile_timezone(strZone);
     /*inform other process*/
    chsm_clock_notify_other_application();
    return PM_E_NONE;   
}
void
_chsm_summer_clock_dst_timer_alarm()
{
    long int interval;
    sal_time_t date;
    sal_time_t tmSec;  /*time make*/ 
    tbl_tag_summer_clock_t *p_db_summer_clk = NULL;
    tbl_clock_t *p_clk = NULL;
    p_db_summer_clk = tbl_tag_summer_clock_get_tag_summer_clock();
    /*get current time*/
    date = sal_time_sys_current(NULL);
    if(date == (sal_time_t) - 1)
    {
        log_sys(M_MOD_LIB, E_ERROR, "Unable to get time!errno = %s. ", sal_strerror(errno));
   
    } 
    
    /*when end current time is great than summer time, then change TZ*/
    _chsm_summer_clock_make_time(&p_db_summer_clk->tmend, &tmSec);
    if(((date - tmSec) >= 0) || ((date + CHSM_TIME_ERROR_RANGE - tmSec) >= 0))
    {   
        p_clk = tbl_clock_get_clock();
        if (0 == sal_strlen(p_clk->timezone_name))
        {
            _chsm_clock_set_timezone(p_clk, FALSE);
        }
        else
        {
            _chsm_clock_set_timezone(p_clk, TRUE);
        }
        
        if(p_db_summer_clk->is_recurring)
        {
            if (p_db_summer_clk->is_wday_tm)
            {
                p_db_summer_clk->wday_tm_start.wday_tm_year += 1;
                p_db_summer_clk->wday_tm_end.wday_tm_year += 1;
                _chsm_summmer_trans_wday_time_to_localtime(&p_db_summer_clk->wday_tm_start, &p_db_summer_clk->tmstart);
                _chsm_summmer_trans_wday_time_to_localtime(&p_db_summer_clk->wday_tm_end, &p_db_summer_clk->tmend);
                p_db_summer_clk->nStartFebDay = p_db_summer_clk->tmstart.tm_mday;
                p_db_summer_clk->nEndFebDay = p_db_summer_clk->tmend.tm_mday;
            }
            else
            {
                p_db_summer_clk->tmstart.tm_year += 1;
                p_db_summer_clk->tmend.tm_year += 1;
            }
            _chsm_summer_clock_check_date(&p_db_summer_clk->tmstart.tm_mday, p_db_summer_clk->tmstart.tm_mon, 
                p_db_summer_clk->tmstart.tm_year, CHSM_FLAG_DST);
            _chsm_summer_clock_check_date(&p_db_summer_clk->tmend.tm_mday, p_db_summer_clk->tmend.tm_mon, 
                p_db_summer_clk->tmend.tm_year, CHSM_FLAG_DST); 
             chsm_set_tag_summer_clock(p_db_summer_clk);
            /*add summer time alarm*/
            _chsm_summer_clock_cal_dst_timer_interval(&interval);
            p_db_summer_clk->t_summer_time_timer = CTC_TASK_ADD_TIME(_chsm_summer_clock_dst_timer_alarm, NULL,interval);
            tbl_tag_summer_clock_set_tag_summer_clock_field(p_db_summer_clk,TBL_TAG_SUMMER_CLOCK_FLD_T_SUMMER_TIME_TIMER);
        }
        else
        {
            CTC_TASK_STOP_TIMER(p_db_summer_clk->t_summer_time_timer);   
        }
    }  
    else
    {
        /*when current time is great than start summer time, then change TZ*/
        _chsm_summer_clock_make_time(&p_db_summer_clk->tmstart, &tmSec);
        if(((date - tmSec) >= 0) || ((date + CHSM_TIME_ERROR_RANGE - tmSec) >= 0))
        {
            _chsm_summer_clock_set_timezone_dst();   
        }
        /*add summer time alarm*/
        _chsm_summer_clock_cal_dst_timer_interval(&interval);
        p_db_summer_clk->t_summer_time_timer = CTC_TASK_ADD_TIME(_chsm_summer_clock_dst_timer_alarm, NULL, interval);  
        tbl_tag_summer_clock_set_tag_summer_clock_field(p_db_summer_clk,TBL_TAG_SUMMER_CLOCK_FLD_T_SUMMER_TIME_TIMER);
    }   
}


int32
_chsm_summer_clock_dst_timer_adjust()
{
    long int interval = 0;   /*timer alarm inteval*/   
    tbl_tag_summer_clock_t *p_db_summer_clk = NULL;
    /*adjust summer time alarm*/ 
    _chsm_summer_clock_cal_dst_timer_interval(&interval);
    p_db_summer_clk = tbl_tag_summer_clock_get_tag_summer_clock();
    if(interval != 0)
    {
        p_db_summer_clk->t_summer_time_timer = CTC_TASK_ADD_TIME(_chsm_summer_clock_dst_timer_alarm,NULL,interval); 
        tbl_tag_summer_clock_set_tag_summer_clock_field(p_db_summer_clk,TBL_TAG_SUMMER_CLOCK_FLD_T_SUMMER_TIME_TIMER);
    }
    
    return PM_E_NONE;
}
int32
chsm_cmd_process_clock(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_chassis_t *p_db_chassis = tbl_chassis_get_chassis();
    tbl_clock_t *p_db_clock = tbl_clock_get_clock();
    tbl_tag_summer_clock_t *p_summer_db_clk =  NULL;
    p_summer_db_clk = tbl_tag_summer_clock_get_tag_summer_clock();
    sal_time_t date, date_now;
    int year, month, day, hour, minute, second;   
    int32 rc = PM_E_NONE;
    FILE *fp = NULL;
    char strTime[CHSM_MAX_TIME_LEN + 1];
    char strTime_old[CHSM_MAX_TIME_LEN + 1];
    char offset_str[CMD_BUF_32];
    struct sal_tm tmSys;
    struct sal_timeval tv;
    struct sal_timezone tz;
    char szCmd[CMD_BUF_32];
    sal_time_t tmSec_start;
    sal_time_t tmSec_end;
    struct tm *ptmCur = NULL;
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
    
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        date = sal_time(NULL);
        if(date == (sal_time_t) - 1)
        {
            CFG_FUNC_ERR_RET("Unable to get time of day");
        }  
        chsm_set_tzenv();
        sal_memset(strTime, 0, sizeof(strTime));
        sal_memset(offset_str, 0, sizeof(offset_str));
        sal_time_strf(strTime, sizeof(strTime), "%T %Z %a %b %d %Y", sal_localtime(&date));
        sal_fprintf(fp, "%s\n", strTime);
        if (sal_strlen(p_db_clock->timezone_name))
        {
            sal_snprintf(offset_str, CMD_BUF_32, "%02u:%02u:%02u", p_db_clock->timezone_hour, p_db_clock->timezone_minute, p_db_clock->timezone_second);
            sal_fprintf(fp, "Time Zone(%s) : UTC%s%s\n", p_db_clock->timezone_name, p_db_clock->timezone_positive ? "+" : "-", offset_str);
        }
        sal_fclose(fp);
    
        break;
    case CDB_OPER_SET:
        if (p_db_chassis->schedule_reboot_timer)
        {
            CFG_CONFLICT_RET("Schedule reboot has been configured");
        }
        if(sal_sscanf(argv[0], "%2d:%2d:%2d", &hour, &minute, &second) != 3)
        {
            CFG_INVALID_PARAM_RET("Unable to translate clock time: %s", argv[0]);
        }    
        day = sal_atoi(argv[1]);
        month = sal_atoi(argv[2]);        
        year = sal_atoi(argv[3]);

        /* if ntpd is running, should check ntpd is synced */
        rc = sal_system("pgrep ntpd >/dev/null 2>/dev/null");
        if (0 == rc)
        {
            rc = sal_system("ntpdc -nc sysinfo 127.0.0.1 | grep 'stratum              : 16' >/dev/null 2>/dev/null");
            if (0 != rc)
            {
                CFG_CONFLICT_RET("NTP is synced and cannot set clock");
            }
        }
        _chsm_cmd_get_time_str(strTime_old, CHSM_MAX_TIME_LEN);
        rc = _chsm_clock_check_date(&day, month, year);        
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invaild date");
        }
        
        tmSys.tm_year = year - 1900;
        tmSys.tm_mon = month - 1;
        tmSys.tm_mday = day;
        tmSys.tm_hour = hour;
        tmSys.tm_min = minute;
        tmSys.tm_sec = second;
        
        rc = gettimeofday(&tv, &tz);        
        if(-1 == rc)
        {
            CFG_INVALID_PARAM_RET("Unable to get time of day");
        }

        fp = sal_fopen(CLOCK_CHANGE_DELTA_FILE, "w+");
        if (NULL == fp)
        {
            CFG_INVALID_PARAM_RET("Can't open file");
        }
        sal_fwrite(&tv, 1, sizeof(tv), fp);
        sal_fclose(fp);

        sal_memset(offset_str, 0, sizeof(offset_str));
        sal_snprintf(offset_str, CMD_BUF_32, "%02u:%02u:%02u", hour, minute, second);
        date = sal_time_mk(&tmSys);
        if(sal_strlen(p_summer_db_clk->strZoneName))
        {
            date_now = sal_time_sys_current(NULL);
            ptmCur = sal_localtime(&date_now);
            if(p_summer_db_clk->is_recurring&&p_summer_db_clk->is_wday_tm)
            {
                p_summer_db_clk->wday_tm_start.wday_tm_year = ptmCur->tm_year + 1900;
                p_summer_db_clk->wday_tm_end.wday_tm_year = ptmCur->tm_year + 1900;
                _chsm_summmer_trans_wday_time_to_localtime(&p_summer_db_clk->wday_tm_start, &p_summer_db_clk->tmstart);
                _chsm_summmer_trans_wday_time_to_localtime(&p_summer_db_clk->wday_tm_end, &p_summer_db_clk->tmend);
                p_summer_db_clk->nStartFebDay = p_summer_db_clk->tmstart.tm_mday;
                p_summer_db_clk->nEndFebDay = p_summer_db_clk->tmend.tm_mday;
            }
            else if (p_summer_db_clk->is_recurring && !p_summer_db_clk->is_wday_tm)
            {
                p_summer_db_clk->tmstart.tm_year= ptmCur->tm_year + 1900;
                p_summer_db_clk->tmend.tm_year= ptmCur->tm_year + 1900;
            }
            _chsm_summer_clock_make_time(&p_summer_db_clk->tmend, &tmSec_end);
            _chsm_summer_clock_make_time(&p_summer_db_clk->tmstart, &tmSec_start);
            
            if ((date>=tmSec_start)&&(date<=tmSec_start+p_summer_db_clk->iDstOffset*60))
            {
                CFG_INVALID_PARAM_RET("Invalid configuration.The time set does not comply with summer clock rules.");
            }
            if ((date<=tmSec_end)&&(date>=tmSec_end-p_summer_db_clk->iDstOffset*60))
            {
                CFG_INVALID_PARAM_RET("Invalid configuration.The time set does not comply with summer clock rules.");
            }
            if ((date>=tmSec_start+p_summer_db_clk->iDstOffset*60)&&(date<=tmSec_end-p_summer_db_clk->iDstOffset*60))
            {
                //date = date - p_summer_db_clk->iDstOffset*60;
            }
            if (date>tmSec_end&&!sal_strcmp(tmSys.tm_zone,p_summer_db_clk->strZoneName))
            {
                date = date + p_summer_db_clk->iDstOffset*60;
            }
            
        }
        tv.tv_sec = (uint32)date;
        
        rc = sal_settimeofday(&tv, &tz);
        if(-1 == rc)
        {
            CFG_INVALID_PARAM_RET("Unable to set time of day");
        }

        /*syschoronize system time to hardware
          save hardware clock in UTC mode to simplify timezone processing. */            
        sal_system("hwclock --systohc -u");
        sal_system("cdbctl update/cdb/ptp/sync_systime");
        sal_task_sleep(100);
        snmp_trap_time_change(argv[0], month, day, year);
        
        _chsm_cmd_get_time_str(strTime, CHSM_MAX_TIME_LEN);
        logid_sys(LOG_SYSTEM_4_TIMECHANGE, strTime_old, strTime);
        
        /* add by wangyb for bug 44763, 2017-10-18 */
        sal_snprintf(szCmd, CMD_BUF_64, "touch /dev/pts/* 1>/dev/null 2>&1");
        sal_system(szCmd);
        if (sal_strlen(p_summer_db_clk->strZoneName))
        {
             CTC_TASK_STOP_TIMER(p_summer_db_clk->t_summer_time_timer); 
            _chsm_summer_clock_dst_timer_adjust();
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

#define _CHSM_ACT_UPDATE_EPLD_

int32
chsm_cmd_process_update_epld(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char show_name[M_FULLPATH_MAX_LEN];
    char dst_file[M_FULLPATH_MAX_LEN];
    tbl_card_t* p_db_card = tbl_card_get_default_card();
    int32 i = 0;
    int32 rc = PM_E_NONE;  

    switch (para->oper)
    {
    case CDB_OPER_SET:               
        sal_memset(show_name, 0, sizeof(show_name));
        sal_memset(dst_file, 0, sizeof(dst_file));
        for (i = 0; i < argc; i++)
        {
            sal_strcat(show_name, argv[i]);
            if (i != argc-1)
                sal_strcat(show_name, "/");
        }

        if (gen_check_and_get_filename_bytype(show_name, dst_file, 
                    M_FULLPATH_MAX_LEN, E_DISK_FILE_FLASH) != 0
            && gen_check_and_get_filename_bytype(show_name, dst_file, 
                    M_FULLPATH_MAX_LEN, E_DISK_FILE_UDISK) != 0)
        {
            CFG_INVALID_PARAM_RET("Invalid image file path: %s", show_name);
        }
        sal_memset(p_db_card->update_epld_name, 0, sizeof(p_db_card->update_epld_name));
        sal_memcpy(p_db_card->update_epld_name, dst_file, M_FULLPATH_MAX_LEN);
        tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_UPDATE_EPLD_NAME);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

#define _CHSM_ACT_UPDATE_BOOTROM_

int32
chsm_cmd_process_update_bootrom(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char show_name[M_FULLPATH_MAX_LEN];
    char dst_file[M_FULLPATH_MAX_LEN];
    tbl_card_t* p_db_card = tbl_card_get_default_card();
    int32 i = 0;
    int32 rc = PM_E_NONE;  

    switch (para->oper)
    {
    case CDB_OPER_SET:               
        sal_memset(show_name, 0, sizeof(show_name));
        sal_memset(dst_file, 0, sizeof(dst_file));
        for (i = 0; i < argc; i++)
        {
            sal_strcat(show_name, argv[i]);
            if (i != argc-1)
                sal_strcat(show_name, "/");
        }

        /* for bug 39078, support update bootrom from udisk*/
        if (gen_check_and_get_filename_bytype(show_name, dst_file, 
                    M_FULLPATH_MAX_LEN, E_DISK_FILE_FLASH) != 0
            && gen_check_and_get_filename_bytype(show_name, dst_file, 
                    M_FULLPATH_MAX_LEN, E_DISK_FILE_UDISK) != 0)                    
        {
            CFG_INVALID_PARAM_RET("Invalid image file path: %s", show_name);
        }
        sal_memset(p_db_card->update_bootrom_name, 0, sizeof(p_db_card->update_bootrom_name));
        sal_strncpy(p_db_card->update_bootrom_name, dst_file, GLB_NAME_INFO_STR_MAX);
        tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_UPDATE_BOOTROM_NAME);

        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

#ifndef _GLB_UML_SYSTEM_
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>

#define __SSH_RSA_KEY_FNAME         "/etc/ssh/ssh_host_rsa_key"
#define M_LINE_BUF_SZ 8192

/* local rsa key */
RSA *
_load_rsa_key(char *pszFileName, int bPublicKey)
{
    RSA *prsa = NULL;
    RSA *prsaRet = NULL;
    EVP_PKEY *pkey = NULL;
    BIO *bio = NULL;

    if (NULL == pszFileName)
    {
        return NULL;
    }
    
    /* prepare bio */
    if (NULL == (bio = BIO_new(BIO_s_file())))
    {
        goto error_out;
    }
    if (BIO_read_filename(bio, pszFileName) <= 0)
    {
        goto error_out;
    }

    /* load key */
    if (bPublicKey)
    {
        pkey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    }
    else
    {
        pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
    }
    if (NULL == pkey)
    {
        goto error_out;
    }
    
    if ((prsa = EVP_PKEY_get1_RSA(pkey)) == NULL)
    {
        goto error_out;
    }

    prsaRet = prsa;
    prsa = NULL;
    
error_out:
    if (NULL != prsa)
    {
        RSA_free(prsa);
    }
    if (NULL != bio)
    {
        BIO_free(bio);
    }
    if (NULL != pkey)
    {
        EVP_PKEY_free(pkey);
    }
    return prsaRet;
}

int 
rsa_encdec(bool bEnc, char *pszKeyFileName, int bUsePrivateKey,
    unsigned char *pszBuf, size_t nSize, unsigned char *pszDstBuf, size_t *pnDstSize)
{
    RSA *prsa = NULL;
    unsigned char pad = RSA_PKCS1_PADDING;
    int ret_code = -1;
    int nRet;

    /* try load as private key */
    prsa = _load_rsa_key(pszKeyFileName, 0);
    if (NULL == prsa) {
        if (bUsePrivateKey) {
            goto err_out;
        }
        
        /* try load as public key */
        prsa = _load_rsa_key(pszKeyFileName, 1);
        if (NULL == prsa) {
            goto err_out;
        }
    }

    if (bUsePrivateKey) {
        if (bEnc) {
            nRet = RSA_private_encrypt(nSize, pszBuf, pszDstBuf, prsa, pad);
        } else {
            nRet = RSA_private_decrypt(nSize, pszBuf, pszDstBuf, prsa, pad);
        }    
    } else {
        if (bEnc) {
            nRet = RSA_public_encrypt(nSize, pszBuf, pszDstBuf, prsa, pad);
        } else {
            nRet = RSA_public_decrypt(nSize, pszBuf, pszDstBuf, prsa, pad);
        }        
    }
    if (nRet <= 0) {
        goto err_out;
    }

    pszDstBuf[nRet] = '\0';
    *pnDstSize = nRet;

    ret_code = 0;

err_out:
    if (NULL != prsa) {
        RSA_free(prsa);
    }

    return ret_code;
}

#endif

int32
chsm_cmd_process_generate_oem_file(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;  
#ifndef _GLB_UML_SYSTEM_
    char show_name[M_FULLPATH_MAX_LEN];
    char dst_file[M_FULLPATH_MAX_LEN];
    unsigned char szOemBuf[M_LINE_BUF_SZ];
    unsigned char szDstBuf[M_LINE_BUF_SZ];
    size_t nLen = 0;
    int nSize = 0;
    FILE *fp = NULL;
    int i = 0;

    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(show_name, 0, sizeof(show_name));
        sal_memset(dst_file, 0, sizeof(dst_file));
        sal_memset(szOemBuf , 0, sizeof(szOemBuf));
        sal_memset(szDstBuf, 0, sizeof(szDstBuf));
        for (i = 0; i < argc; i++)
        {
            if (sal_strcmp(argv[i], "parameter") == 0)
            {
                break;
            }
            sal_strcat(show_name, argv[i]);
            sal_strcat(show_name, "/");
        }
        show_name[strlen(show_name)-1] = '\0';

        if (gen_check_and_get_filename_bytype(show_name, dst_file, 
                    M_FULLPATH_MAX_LEN, E_DISK_FILE_FLASH) != 0)
        {
            CFG_INVALID_PARAM_RET("Invalid image file path: %s", show_name);
        }

        sal_strcpy((char *)szOemBuf, "cdbctl update/cdb/sys/update_oem_info/");
	 ++i;
        for (; i < argc; i++)
        {
            sal_strcat((char *)szOemBuf, "\"");
            sal_strcat((char *)szOemBuf, argv[i]);
            sal_strcat((char *)szOemBuf, "\"");
            if (i != argc-1)
                sal_strcat((char *)szOemBuf, "/");
        }
        nSize = strlen((char *)szOemBuf)+1;

       rsa_encdec(1, __SSH_RSA_KEY_FNAME, 0, szOemBuf, nSize, szDstBuf, &nLen);

	fp = sal_fopen(dst_file, "w");
    	if (NULL != fp)
        {
            sal_fwrite(szDstBuf, 1, nLen, fp);
        }
        if (NULL != fp)
        {
            sal_fclose(fp);
        }
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
#endif
    return rc;
}

int32
chsm_cmd_process_update_oem_file(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE; 
#ifndef _GLB_UML_SYSTEM_
    char show_name[M_FULLPATH_MAX_LEN];
    char dst_file[M_FULLPATH_MAX_LEN]; 
    FILE *fp = NULL;
    unsigned char szOemBuf[M_LINE_BUF_SZ];
    unsigned char szDstBuf[M_LINE_BUF_SZ];
    //char cmd[M_LINE_BUF_SZ];
    int nSize = 0;
    int i = 0;
    size_t nLen = 0;

    switch (para->oper)
    {
    case CDB_OPER_SET:               
        sal_memset(show_name, 0, sizeof(show_name));
        sal_memset(dst_file, 0, sizeof(dst_file));
        sal_memset(szOemBuf , 0, sizeof(szOemBuf));
        sal_memset(szDstBuf, 0, sizeof(szDstBuf));
        for (i = 0; i < argc; i++)
        {
            sal_strcat(show_name, argv[i]);
            if (i != argc-1)
                sal_strcat(show_name, "/");
        }

        if (gen_check_and_get_filename_bytype(show_name, dst_file, 
                    M_FULLPATH_MAX_LEN, E_DISK_FILE_FLASH) != 0)
        {
            CFG_INVALID_PARAM_RET("Invalid image file path: %s", show_name);
        }

        fp = sal_fopen(dst_file, "r");
        if (NULL == fp)
        {
            CFG_INVALID_PARAM_RET("Invalid image file path: %s", show_name);
        }
	 nSize = sal_fread(szOemBuf, 1, M_LINE_BUF_SZ, fp);
	 //sal_fread(szOemBuf, 1, M_LINE_BUF_SZ, fp);
        sal_fclose(fp);
        
        rsa_encdec(0, __SSH_RSA_KEY_FNAME, 1, szOemBuf, nSize, szDstBuf, &nLen);

        if (nLen <= 0 || sal_strncmp((const char *)szDstBuf, "cdbctl ", 7)) {
            CFG_INVALID_PARAM_RET("Invalid OEM binary file");
        }

        sal_system((char *)szDstBuf);
        
        CFG_INVALID_PARAM_RET("Update OEM information accomplished");
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
#endif
    return rc;
}



#define _CHSM_ACT_UPDATE_PHY_FIRMWARE_

int32
chsm_cmd_process_update_phy_firmware(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char dst_file[M_FULLPATH_MAX_LEN];
    tbl_card_t* p_db_card = tbl_card_get_default_card();
    int32 i = 0;
    int32 rc = PM_E_NONE;  
    int32 update_phy_firmware_port_idx = 0;
    //FILE * fp_console = NULL;

    switch (para->oper)
    {
    case CDB_OPER_SET:               
        update_phy_firmware_port_idx = atoi(argv[0]);
        //fp_console = fopen("/dev/console", "w+"); 
        //fprintf(fp_console, "%s\n", argv[0]);
        //fclose(fp_console);
        sal_memset(dst_file, 0, sizeof(dst_file));
        if(!((sal_strncmp(argv[1], "check_version", 13) == 0) && (argc == 2)))
        {
            sal_strcat(dst_file, "/");
        }
        for (i = 1; i < argc; i++)
        {
            //fp_console = fopen("/dev/console", "w+"); 
            //fprintf(fp_console, "%s\n", argv[i]);
            //fclose(fp_console);
            sal_strcat(dst_file, argv[i]);
            if (i != argc-1)
                sal_strcat(dst_file, "/");
        }

        p_db_card->update_phy_firmware_port_idx = update_phy_firmware_port_idx;
        sal_memset(p_db_card->update_phy_firmware_name, 0, sizeof(p_db_card->update_phy_firmware_name));
        sal_strncpy(p_db_card->update_phy_firmware_name, dst_file, GLB_NAME_INFO_STR_MAX);
        tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_UPDATE_PHY_FIRMWARE_NAME);

        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
chsm_cmd_process_bootupdiag(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_card_t* p_db_card = tbl_card_get_default_card();
    FILE * fp = NULL;

    switch (para->oper)
    {
    case CDB_OPER_SET: 
        if (argc == 1)
        {
            if (0 == sal_strcmp(argv[0], "no"))
            {
                if (p_db_card->bootup_diag_level == 0)
                {
                    p_db_card->bootup_diag_level = 1;
                    p_db_card->next_bootup_diag_level = 1;
                }
                else
                {
                    p_db_card->next_bootup_diag_level = 1;
                }
            }
            else if (0 == sal_strcmp(argv[0], "minimal"))
            {
                if (p_db_card->bootup_diag_level == 0)
                {
                    p_db_card->bootup_diag_level = 2;
                    p_db_card->next_bootup_diag_level = 2;
                }
                else
                {
                    p_db_card->next_bootup_diag_level = 2;
                }
            }
            else if (0 == sal_strcmp(argv[0], "complete"))
            {
                if (p_db_card->bootup_diag_level == 0)
                {
                    p_db_card->bootup_diag_level = 3;
                    p_db_card->next_bootup_diag_level = 3;
                }
                else
                {
                    p_db_card->next_bootup_diag_level = 3;
                }
            }
        }

        tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_BOOTUP_DIAG_LEVEL);
        tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_NEXT_BOOTUP_DIAG_LEVEL);
        break;

    case CDB_OPER_GET:
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        if (NULL == fp)
        {
            return PM_E_NONE;
        }

        if (p_db_card->bootup_diag_level == 0 || p_db_card->bootup_diag_level == 1)
        {
            sal_fprintf(fp, "The current running is no diagnostic bootup level\n");
        }
        else if (p_db_card->bootup_diag_level == 2)
        {
            sal_fprintf(fp, "The current running bootup diag level is minimal\n");
        }
        else if (p_db_card->bootup_diag_level == 3)
        {
            sal_fprintf(fp, "The current running bootup diag level is complete\n");
        }

        if (p_db_card->next_bootup_diag_level == 0 || p_db_card->next_bootup_diag_level == 1)
        {
            sal_fprintf(fp, "The next running is no diagnostic bootup level\n");
        }
        else if (p_db_card->next_bootup_diag_level == 2)
        {
            sal_fprintf(fp, "The next running bootup diag level is minimal\n");
        }
        else if (p_db_card->next_bootup_diag_level == 3)
        {
            sal_fprintf(fp, "The next running bootup diag level is complete\n");
        }
        
        sal_fclose(fp);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return PM_E_NONE;
}

int32
chsm_cmd_process_reboot(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 save = 1;
    int32 rc = PM_E_NONE;  

    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (argc == 1)
        {
            if (0 == sal_strcmp(argv[0], "save"))
            {
                save = 2;
            }
        }
        snmp_trap_reboot_send(save);
        /* added by tongzb for bug 40719 0x01 means MANUAL reboot, 2016-09-12 */
        system("reboot_record 0x01 1>/dev/null 2>&1 && sync");
        chsm_reset_system();
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

char *
format_time_str(char *pszBuf, size_t nLen, 
                               char *pszFormat, sal_time_t nTime)
{
    struct tm *tm = NULL;
    
    if (NULL == pszBuf || NULL == pszFormat)
    {
        return NULL;
    }

    tm = sal_localtime(&nTime);
    sal_time_strf(pszBuf, nLen, pszFormat, tm);
    return pszBuf;
}
    
static int32
_chsm_cmd_process_show_schedule_reboot(char **argv, int32 argc, cfg_cmd_para_t *para)
{
#define M_SCHDULE_REBOOT_BUFSIZE  32
    char szBuf[M_SCHDULE_REBOOT_BUFSIZE];
    tbl_chassis_t *p_chassis = tbl_chassis_get_chassis();
    int32 interval = 0;
    sal_time_t nCur = sal_time(NULL);
    FILE *fp = NULL;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (NULL == p_chassis->schedule_reboot_timer)
    {
        sal_fprintf(fp, "No reboot schedule\n");
    }
    else
    {
        format_time_str(szBuf, M_SCHDULE_REBOOT_BUFSIZE, "%Y-%m-%d %H:%M:%S", nCur);
        sal_fprintf(fp, "Current time is : %s\n", szBuf);
        
        interval = ctc_timer_get_remain_sec(p_chassis->schedule_reboot_timer);
        nCur += interval;

        format_time_str(szBuf, M_SCHDULE_REBOOT_BUFSIZE, "%Y-%m-%d %H:%M:%S", nCur);
        sal_fprintf(fp, "Will reboot at  : %s\n", szBuf);
    }

    sal_fclose(fp);
    
    return PM_E_NONE;
}

int32
chsm_cmd_process_show_schedule_reboot(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;  

    if (CDB_OPER_GET != para->oper)
    {
        CFG_INVALID_PARAM_RET("Invalid action");
    }
    
    rc = _chsm_cmd_process_show_schedule_reboot(argv, argc, para);
    
    return rc;
}

int32
chsm_cmd_process_update_port_connection(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 i = 0;
    int32 j = 0;
    char *p = NULL;
    cli_phy_port_prefix_t phy_port_prefix[GLB_PHT_PORT_TYPE_MAX];
    port_info_t port_info;
    char tmp_buf[CMD_BUF_16];

    /* modified by liwh for bug 43875, 2019-06-10 */
    CFG_UNSUPPORT_PRODUCT_RET();
    /* liwh end */

    sal_memset(&phy_port_prefix, 0, sizeof(phy_port_prefix));
    sal_memset(&port_info, 0, sizeof(port_info));
    sal_memset(tmp_buf, 0, sizeof(tmp_buf));
   
    sal_b64_pton(argv[0], (unsigned char *)tmp_buf, CMD_BUF_16);
    port_info.connection = tmp_buf[0];
    sal_b64_pton(argv[1], (unsigned char *)tmp_buf, CMD_BUF_16);
    port_info.sub_connection = tmp_buf[0];

    for (i = 2; i < argc; i += 2)
    {
        /*1. check string*/
        if (check_prefix_string(argv[i]))
        {
            CFG_INVALID_PARAM_RET("Invalid prefix %s: can only include [a-zA-Z]. Its length range is [1,4]", argv[i]);
        }
        p = check_prefix_dup(argv[i]);
        if (p)
        {
            CFG_INVALID_PARAM_RET("Invalid prefix %s: duplicate with %s", argv[i], p);
        }
        if (check_prefix_full_string(argv[i+1]))
        {
            CFG_INVALID_PARAM_RET("Invalid prefix %s: can only include [a-zA-Z]. Its length range is [1,16]", argv[i+1]);
        }
        p = check_prefix_dup(argv[i+1]);
        if (p)
        {
            CFG_INVALID_PARAM_RET("Invalid prefix %s: duplicate with %s", argv[i+1], p);
        }

        /*2. check with other prefix*/
        for (j = 0; j < GLB_PHT_PORT_TYPE_MAX; j++)
        {
            if (!sal_strlen(phy_port_prefix[j].prefix))
            {
                continue;
            }
            if (!sal_strncmp(argv[i], phy_port_prefix[j].prefix, sal_strlen(argv[i])))
            {
                CFG_INVALID_PARAM_RET("Invalid prefix %s: conflict with %s", argv[i], phy_port_prefix[j].prefix);
            }
            if (!sal_strncmp(argv[i], phy_port_prefix[j].prefix, sal_strlen(phy_port_prefix[j].prefix)))
            {
                CFG_INVALID_PARAM_RET("Invalid prefix %s: conflict with %s", argv[i], phy_port_prefix[j].prefix);
            }
            if (!sal_strncmp(argv[i], phy_port_prefix[j].full_prefix, sal_strlen(argv[i])))
            {
                CFG_INVALID_PARAM_RET("Invalid prefix %s: conflict with %s", argv[i], phy_port_prefix[j].full_prefix);
            }
            if (!sal_strncmp(argv[i], phy_port_prefix[j].full_prefix, sal_strlen(phy_port_prefix[j].full_prefix)))
            {
                CFG_INVALID_PARAM_RET("Invalid prefix %s: conflict with %s", argv[i], phy_port_prefix[j].full_prefix);
            }
            if (!sal_strncmp(argv[i+1], phy_port_prefix[j].prefix, sal_strlen(argv[i+1])))
            {
                CFG_INVALID_PARAM_RET("Invalid prefix %s: conflict with %s", argv[i+1], phy_port_prefix[j].prefix);
            }
            if (!sal_strncmp(argv[i+1], phy_port_prefix[j].prefix, sal_strlen(phy_port_prefix[j].prefix)))
            {
                CFG_INVALID_PARAM_RET("Invalid prefix %s: conflict with %s", argv[i+1], phy_port_prefix[j].prefix);
            }
            if (!sal_strncmp(argv[i+1], phy_port_prefix[j].full_prefix, sal_strlen(argv[i+1])))
            {
                CFG_INVALID_PARAM_RET("Invalid prefix %s: conflict with %s", argv[i+1], phy_port_prefix[j].full_prefix);
            }
            if (!sal_strncmp(argv[i+1], phy_port_prefix[j].full_prefix, sal_strlen(phy_port_prefix[j].full_prefix)))
            {
                CFG_INVALID_PARAM_RET("Invalid prefix %s: conflict with %s", argv[i+1], phy_port_prefix[j].full_prefix);
            }
        }

        /*3. save db*/
        sal_snprintf(phy_port_prefix[(i-2)/2].prefix, GLB_PHY_PORT_PREFIX_SZ, argv[i]);
        sal_snprintf(phy_port_prefix[(i-2)/2].full_prefix, GLB_PHY_PORT_FULL_PREFIX_SZ, argv[i+1]);
    }   
    
    switch (para->oper)
    {
    case CDB_OPER_SET:               
        port_info.prefix_num= 5;
        for (j = 0; j < port_info.prefix_num; j++)
        {
            if(argc == 2)
            {
                sal_sprintf(port_info.prefix[j], GLB_IFNAME_ETH_PREFIX);
                sal_sprintf(port_info.full_prefix[j], GLB_IFNAME_ETH_PREFIX);
            }
            else
            {
                sal_sprintf(port_info.prefix[j], phy_port_prefix[j].prefix);
                sal_sprintf(port_info.full_prefix[j], phy_port_prefix[j].full_prefix);
            }
        }

        if (chsm_mgt_set_port_info(&port_info))
        {
            CFG_INVALID_PARAM_RET("Set port information failed");
        }
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return PM_E_NONE;
}

int32
_chsm_cmd_get_environment(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_fan_master_t *p_fan_master = tbl_fan_get_master();
    tbl_psu_master_t *p_psu_master = tbl_psu_get_master();
    tbl_sensor_master_t *p_sensor_master = tbl_sensor_get_master();
    tbl_card_t *p_db_card = tbl_card_get_default_card();
    ctclib_slistnode_t *listnode = NULL;
    tbl_fan_t *p_db_fan = NULL;
    tbl_psu_t *p_db_psu = NULL;
    tbl_sensor_t *p_db_sensor = NULL;
    char rate_str[CMD_BUF_32];
    FILE *fp = NULL;
    uint32 is_ok = FALSE;
    uint32 is_fan_reverse = 0;
    char fan_status_str[8][CMD_BUF_256];/* there is no more than 8 fans on one board */
    char index_str[8][CMD_BUF_32];
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NONE;
    }

    /* for bug 45858, For E350-48T4X fan1 and fan2 order is reverse*/
    if (sal_strstr(p_db_card->product_name, "350"))
    {
        if (0 == sal_strcmp(p_db_card->hardware_name, "48T4X"))
        {
            is_fan_reverse = 1;
        }
    }    
    sal_fprintf(fp, "Fan tray status:\n");
    sal_fprintf(fp, "%-10s %-12s %-12s %-12s\n", "Index", "Status", "SpeedRate", "Mode");
    sal_fprintf(fp, "----------+------------+------------+------------\n");
    int index = 0;
    int i = 0;
    CTCLIB_SLIST_LOOP(p_fan_master->fan_list, p_db_fan, listnode)
    {
        is_ok = (!sal_strcmp(p_db_fan->status, "OK"));
        sal_snprintf(rate_str, CMD_BUF_32, "%d", (is_ok ? p_db_fan->percent : 0));
        sal_strcat(rate_str, "%");
        index_str[index][0] = CMD_ZERO_CHAR;
        sal_snprintf(index_str[index], PASSWD_SIZE, "%d-%d", (p_db_fan->tray+1), ((p_db_fan->key.id & 0xff) +1));
        sal_snprintf(fan_status_str[index], CMD_BUF_256,"%-12s %-12s %-12s\n", 
            p_db_fan->status,
            rate_str,
            p_db_fan->mode);
        index++;
    }
    /* for bug 45858, For E350-48T4X fan1 and fan2 order is reverse*/
    for(i=0;i<index;i++)
    {
        if(is_fan_reverse)
        {
            if(i == 0)
            {
                sal_fprintf(fp, "%-10s %s",index_str[i], fan_status_str[1]);
                continue;
            }
            else if(i == 1)
            {
                sal_fprintf(fp, "%-10s %s",index_str[i], fan_status_str[0]);
                continue;
            }
        }
        sal_fprintf(fp, "%-10s %s",index_str[i], fan_status_str[i]);
    }

    sal_fprintf(fp, "\n");
    sal_fprintf(fp, "Power status:\n");
    sal_fprintf(fp, "%-10s %-12s %-12s %-12s %-12s\n", "Index", "Status", "Power", "Type", "Alert");
    sal_fprintf(fp, "----------+------------+------------+------------+------------\n");
    CTCLIB_SLIST_LOOP(p_psu_master->psu_list, p_db_psu, listnode)
    {
        if (p_db_psu->absent)
        {
            sal_fprintf(fp, "%-10d %-12s %-12s %-12s %-12s\n", 
                (p_db_psu->key.id + 1), "ABSENT", "-", "-", "-");
        }
        else
        {
            if(1 == p_db_psu->fixed_status)
            {
                sal_fprintf(fp, "%-10d %-12s %-12s %-12s %-12s\n", 
                            (p_db_psu->key.id + 1), "PRESENT", p_db_psu->run_status,p_db_psu->mode, "-");
            }
            else
            {
                sal_fprintf(fp, "%-10d %-12s %-12s %-12s %-12s\n", 
                            (p_db_psu->key.id + 1), "PRESENT", p_db_psu->run_status, p_db_psu->mode,p_db_psu->alert_status);
            }

        }
    }

    sal_fprintf(fp, "\n");
    sal_fprintf(fp, "Sensor status (Degree Centigrade):\n");
    sal_fprintf(fp, "%-10s %-12s %-12s %-12s %-12s %-12s\n", "Index", "Temperature", "Lower_alarm", "Upper_alarm", "Critical", "Position");
    sal_fprintf(fp, "----------+------------+------------+------------+------------+------------\n");
    CTCLIB_SLIST_LOOP(p_sensor_master->sensor_list, p_db_sensor, listnode)
    {
        sal_fprintf(fp, "%-10d %-12d %-12d %-12d %-12d %-12s\n", 
            (p_db_sensor->key.id+1),
            p_db_sensor->temperature,
            p_db_sensor->tmpr_low,
            p_db_sensor->tmpr_high,
            p_db_sensor->tmpr_critical,
            p_db_sensor->position);
    }

    sal_fclose(fp);
    
    return PM_E_NONE;
}

/*Added by zhush for bug 52768, 2019-07-05*/
int32
_chsm_cmd_get_environment_json(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_fan_master_t *p_fan_master = tbl_fan_get_master();
    tbl_psu_master_t *p_psu_master = tbl_psu_get_master();
    tbl_sensor_master_t *p_sensor_master = tbl_sensor_get_master();
    tbl_card_t *p_db_card = tbl_card_get_default_card();
    ctclib_slistnode_t *listnode = NULL;
    tbl_fan_t *p_db_fan = NULL;
    tbl_psu_t *p_db_psu = NULL;
    tbl_sensor_t *p_db_sensor = NULL;
    char rate_str[CMD_BUF_32];
    FILE *fp = NULL;
    uint32 is_ok = FALSE;
    uint32 is_fan_reverse = 0;
    char fan_status_str[8][CMD_BUF_128];/* there is no more than 8 fans on one board */
    char fan_speedrate_str[8][CMD_BUF_128];
    char fan_mode_str[8][CMD_BUF_128];
    char index_str[8][CMD_BUF_32];
    cJSON *root = NULL;
    cJSON *slots = NULL;
    cJSON *fans = NULL;
    cJSON *powers = NULL;
    cJSON *sensors = NULL;
    cJSON *slot = NULL;
    cJSON *fan = NULL;
    cJSON *power = NULL;
    cJSON *sensor = NULL;
    char str_buf[8] = "";
    int count = 0;
    char *out = NULL;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NONE;
    }
    
    /* for bug 45858, For E350-48T4X fan1 and fan2 order is reverse*/
    if (sal_strstr(p_db_card->product_name, "350"))
    {
        if (0 == sal_strcmp(p_db_card->hardware_name, "48T4X"))
        {
            is_fan_reverse = 1;
        }
    }    
    root = cJSON_CreateObject();
    slots = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "environment infomation", slots);
    slot = cJSON_CreateObject();
    cJSON_AddItemToObject(slots, "0", slot);
    cJSON_AddNullToObject(slot, "Role");
    int index = 0;
    int i = 0;
    CTCLIB_SLIST_LOOP(p_fan_master->fan_list, p_db_fan, listnode)
    {
        is_ok = (!sal_strcmp(p_db_fan->status, "OK"));
        sal_snprintf(rate_str, CMD_BUF_32, "%d", (is_ok ? p_db_fan->percent : 0));
        sal_strcat(rate_str, "%");
        index_str[index][0] = CMD_ZERO_CHAR;
        sal_snprintf(index_str[index], PASSWD_SIZE, "%d-%d", (p_db_fan->tray+1), ((p_db_fan->key.id & 0xff) +1));
        sal_snprintf(fan_status_str[index], CMD_BUF_128,"%s", p_db_fan->status);
        sal_snprintf(fan_speedrate_str[index], CMD_BUF_128, "%s", rate_str);
        sal_snprintf(fan_mode_str[index], CMD_BUF_128, "%s", p_db_fan->mode);
        index++;
    }
    /* for bug 45858, For E350-48T4X fan1 and fan2 order is reverse*/
    fans = cJSON_CreateObject();
    cJSON_AddItemToObject(slot, "Fan tray status", fans);	
    for(i=0;i<index;i++)
    {
        sal_snprintf(str_buf, sizeof(str_buf), "%d", i+1);
	 fan = cJSON_CreateObject();
        cJSON_AddItemToObject(fans, str_buf, fan);
        if(is_fan_reverse)
        {
            if(i == 0)
            {
		  cJSON_AddStringToObject(fan, "Index", index_str[i]);
		  cJSON_AddStringToObject(fan, "Status", fan_status_str[1]);
		  cJSON_AddStringToObject(fan, "SpeedRate", fan_speedrate_str[1]);
		  cJSON_AddStringToObject(fan, "Mode", fan_mode_str[1]);
                continue;
            }
            else if(i == 1)
            {
		  cJSON_AddStringToObject(fan, "Index", index_str[i]);
		  cJSON_AddStringToObject(fan, "Status", fan_status_str[0]);
		  cJSON_AddStringToObject(fan, "SpeedRate", fan_speedrate_str[0]);
		  cJSON_AddStringToObject(fan, "Mode", fan_mode_str[0]);
                continue;
            }
        }
	 cJSON_AddStringToObject(fan, "Index", index_str[i]);
        cJSON_AddStringToObject(fan, "Status", fan_status_str[i]);
        cJSON_AddStringToObject(fan, "SpeedRate", fan_speedrate_str[i]);
        cJSON_AddStringToObject(fan, "Mode", fan_mode_str[i]);
    }

    powers = cJSON_CreateObject();   
    cJSON_AddItemToObject(slot, "Power status", powers);
    CTCLIB_SLIST_LOOP(p_psu_master->psu_list, p_db_psu, listnode)
    {
        sal_snprintf(str_buf, sizeof(str_buf), "%d", count +1);
	 power = cJSON_CreateObject();
	 cJSON_AddItemToObject(powers, str_buf, power);
        if (p_db_psu->absent)
        {
	     cJSON_AddNumberToObject(power, "Index", p_db_psu->key.id + 1);
	     cJSON_AddStringToObject(power, "Status", "ABSENT");
	     cJSON_AddStringToObject(power, "Power", "-");
            cJSON_AddStringToObject(power, "Type", "-");
	     cJSON_AddStringToObject(power, "Alert", "-");
        }
        else
        {
	     cJSON_AddNumberToObject(power, "Index", p_db_psu->key.id + 1);
	     cJSON_AddStringToObject(power, "Status", "PRESENT");
	     cJSON_AddStringToObject(power, "Power", p_db_psu->run_status);
            cJSON_AddStringToObject(power, "Type", p_db_psu->mode);
	     cJSON_AddStringToObject(power, "Alert", p_db_psu->alert_status);
        }
	 count++;
    }

    sensors = cJSON_CreateObject(); 
    cJSON_AddItemToObject(slot, "Sensor status (Degree Centigrade)", sensors);
    count = 0;
    CTCLIB_SLIST_LOOP(p_sensor_master->sensor_list, p_db_sensor, listnode)
    {
        sal_snprintf(str_buf, sizeof(str_buf), "%d", count + 1);
	 sensor = cJSON_CreateObject();
	 cJSON_AddItemToObject(sensors, str_buf, sensor);
	 cJSON_AddNumberToObject(sensor, "Index", (p_db_sensor->key.id+1));
	 cJSON_AddNumberToObject(sensor, "Temperature", p_db_sensor->temperature);
	 cJSON_AddNumberToObject(sensor, "Lower_alarm", p_db_sensor->tmpr_low);
	 cJSON_AddNumberToObject(sensor, "Upper_alarm", p_db_sensor->tmpr_high);
	 cJSON_AddNumberToObject(sensor, "Critical", p_db_sensor->tmpr_critical);
	 cJSON_AddStringToObject(sensor, "Position", p_db_sensor->position);
	 count++;
    }
    out = cJSON_Print(root);
    sal_fprintf(fp, out);
    sal_fprintf(fp,"\n");
    sal_fclose(fp);

    cJSON_Delete(root);
    cJSON_free(out);
    return PM_E_NONE;
}

int32
chsm_cmd_process_show_environment(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        rc = _chsm_cmd_get_environment(argv, argc, para);
        break;
    case CDB_OPER_GETJSON:
        rc = _chsm_cmd_get_environment_json(argv, argc, para);
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
  
}
/*end zhush*/

int32
chsm_cmd_process_show_psu_pmbus_info(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_psu_master_t *p_psu_master = tbl_psu_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_psu_t *p_db_psu = NULL;
    FILE *fp = NULL;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_psu_master->psu_list, p_db_psu, listnode)
    {
        sal_fprintf(fp, "Psu %d:\n",  (p_db_psu->key.id));
        if (p_db_psu->support_pmbus)
        {
            sal_fprintf(fp, "\tstatus_byte: 0x%02x\n", p_db_psu->pmbus_status_byte);
            sal_fprintf(fp, "\tstatus_word: 0x%04x\n", p_db_psu->pmbus_status_word);
            sal_fprintf(fp, "\tstatus_vout: 0x%02x\n", p_db_psu->pmbus_status_vout);
            sal_fprintf(fp, "\tstatus_iout: 0x%02x\n", p_db_psu->pmbus_status_iout);
            sal_fprintf(fp, "\tstatus_input: 0x%02x\n", p_db_psu->pmbus_status_input);
            sal_fprintf(fp, "\tstatus_temperature: 0x%02x\n", p_db_psu->pmbus_status_temperature);
            sal_fprintf(fp, "\tstatus_cml: 0x%02x\n", p_db_psu->pmbus_status_cml);
            sal_fprintf(fp, "\tstatus_other: 0x%02x\n", p_db_psu->pmbus_status_other);
            sal_fprintf(fp, "\tstatus_mfr_specific: 0x%02x\n", p_db_psu->pmbus_status_mfr_specific);
            sal_fprintf(fp, "\tstatus_fans_1_2: 0x%02x\n", p_db_psu->pmbus_status_fans_1_2);
            sal_fprintf(fp, "\tstatus_fans_3_4: 0x%02x\n", p_db_psu->pmbus_status_fans_3_4);
        }
        else
        {
            sal_fprintf(fp, "\tpsu works fine or doesn't support pmbus!\n");
        }
    }

    sal_fclose(fp);
    
    return PM_E_NONE;
}

int32
chsm_cmd_process_show_vcm(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_vcm_master_t *p_vcm_master = tbl_vcm_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_vcm_t *p_db_vcm = NULL;
    double value = 0;
    char *unit = NULL;
    char *channel_name = NULL;
    int32 i;
    FILE *fp = NULL;
    int32 vcm_count = 0;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_vcm_master->vcm_list, p_db_vcm, listnode)
    {
        vcm_count++;
    }
    if (!vcm_count)
    {
        CFG_CONFLICT_RET("this board doesn't support vcm!");
    }
    
    sal_fprintf(fp, "%-10s%-22s%-12s%-12s%-12s%-5s\n",
                        "Module_no", " Pin_name", "value", "Lower_alarm", "Upper_alarm", "Units");
    CTCLIB_SLIST_LOOP(p_vcm_master->vcm_list, p_db_vcm, listnode)
    {
        for (i = 0; i < MAX_VCM_CHANNEL_LEN; i++)
        {
            if (p_db_vcm->flag[i] & GLB_PIN_VOLT
            || p_db_vcm->flag[i] & GLB_PIN_VOLT_POWER
            || p_db_vcm->flag[i] & GLB_PIN_VOLT_IN
            || p_db_vcm->flag[i] & GLB_PIN_VOLT_OUT)
            {
                value = p_db_vcm->volt[i];
                unit = "V";
            }
            else if (p_db_vcm->flag[i] & GLB_PIN_CURR
                    || p_db_vcm->flag[i] & GLB_PIN_CURR_POWER
                    || p_db_vcm->flag[i] & GLB_PIN_CURR_IN
                    || p_db_vcm->flag[i] & GLB_PIN_CURR_OUT)
            {
                value = p_db_vcm->curr[i];
                unit = "A";
            }
            else if (p_db_vcm->flag[i] & GLB_PIN_TMPR)
            {
                value = p_db_vcm->tmpr[i];
                unit = "C";
            }
            
            switch (i) {
                case 0:
                    channel_name = p_db_vcm->channel_name0;
                    break;
                case 1:
                    channel_name = p_db_vcm->channel_name1;
                    break;
                case 2:
                    channel_name = p_db_vcm->channel_name2;
                    break;
                case 3:
                    channel_name = p_db_vcm->channel_name3;
                    break;
                case 4:
                    channel_name = p_db_vcm->channel_name4;
                    break;
                case 5:
                    channel_name = p_db_vcm->channel_name5;
                    break;
                case 6:
                    channel_name = p_db_vcm->channel_name6;
                    break;
                case 7:
                    channel_name = p_db_vcm->channel_name7;
                    break;
                case 8:
                    channel_name = p_db_vcm->channel_name8;
                    break;
                default:
                    break;
            }
            
            sal_fprintf(fp, "%-1d-%-8d%-22s%-12.4f%-12.4f%-12.4f%-2s\n", p_db_vcm->key.id+1, i+1, channel_name,
                                value, p_db_vcm->low[i], p_db_vcm->high[i], unit);

            if (p_db_vcm->flag[i] & GLB_PIN_FLAG_OVER)
            {
                sal_fprintf(fp, "\n");
                break;
            }
        }
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
chsm_cmd_process_show_poe(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char power_limit_str[CMD_BUF_16] = {0};
    char cur_consump_str[CMD_BUF_16] = {0};
    char class_str[CMD_BUF_16] = {0};
    char avail_str[CMD_BUF_16] = {0};
    char voltage_str[CMD_BUF_16] = {0};
    char current_str[CMD_BUF_16] = {0};
    
    tbl_poe_master_t *p_poe_master = tbl_poe_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_poe_t *p_db_poe = NULL;
    int32 value = 0;
    uint8 find_item = 0;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    uint8 panel_slot_no = 0;
    uint8 panel_port_no = 0;
    uint8 panel_sub_port_no = 0;    
    int32 i;
    FILE *fp = NULL;
    int32 poe_count = 0;
    int32 rc = 0;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_poe_master->poe_list, p_db_poe, listnode)
    {
        poe_count++;
    }
    if (!poe_count)
    {
        CFG_CONFLICT_RET("This board doesn't support poe!");
    }

    if (0 == sal_strcmp(argv[0], "sum"))
    {
        if (0 == sal_strcmp(argv[1], "all"))
        {
            CTCLIB_SLIST_LOOP(p_poe_master->poe_list, p_db_poe, listnode)
            {
                sal_fprintf(fp, "Power Source Equipment NO: %d\n",p_db_poe->key.id+1);

                sal_fprintf(fp, "Power Limit: %d W\n",p_db_poe->glb_power_limit);
                sal_fprintf(fp, "Power Available: %d W\n",
                    p_db_poe->glb_power_limit>p_db_poe->glb_cur_consump?p_db_poe->glb_power_limit-p_db_poe->glb_cur_consump:0);
                
                sal_fprintf(fp, "Power Actual Consump: %d W\n",p_db_poe->glb_enable?p_db_poe->glb_cur_consump:0);
                sal_fprintf(fp, "Operation status: %s\n",p_db_poe->glb_enable?"enable":"disable");
                sal_fprintf(fp, "Channel Num for power: %d\n",p_db_poe->pse_channel_num);
            }
        }
        else if (0 == sal_strcmp(argv[1], "id"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value,argv[2],para->p_rs);
            find_item = 0;
            CTCLIB_SLIST_LOOP(p_poe_master->poe_list, p_db_poe, listnode)
            {
                if(find_item)
                    break;
                if(p_db_poe->key.id+1 == value)
                {
                    sal_fprintf(fp, "Power Source Equipment NO: %d\n",p_db_poe->key.id+1);

                    sal_fprintf(fp, "Power Limit: %d W\n",p_db_poe->glb_power_limit);
                    sal_fprintf(fp, "Power Available: %d W\n",
                        p_db_poe->glb_power_limit>p_db_poe->glb_cur_consump?p_db_poe->glb_power_limit-p_db_poe->glb_cur_consump:0);

                    sal_fprintf(fp, "Power Actual Consump: %d W\n",p_db_poe->glb_enable?p_db_poe->glb_cur_consump:0);
                    sal_fprintf(fp, "Operation status: %s\n",p_db_poe->glb_enable?"enable":"disable");
                    sal_fprintf(fp, "Channel Num for power: %d\n",p_db_poe->pse_channel_num);
                    find_item = 1;
                    break;
                }
            }
            if(!find_item)
            {
                CFG_INVALID_PARAM_RET("Invalid pse-id %d", value);
            }
        }
    }
    else if (0 == sal_strcmp(argv[0], "status"))
    {
        if (0 == sal_strcmp(argv[1], "all"))
        {
            CTCLIB_SLIST_LOOP(p_poe_master->poe_list, p_db_poe, listnode)
            {
                sal_fprintf(fp, "Codes : CH - channel\tIF - interface\tST - enable status\n");
                sal_fprintf(fp, "        MD - mode\tCL - class\tPR - priority\n");
                sal_fprintf(fp, "        V - voltage(V)\t\tmA - current(mA)\tTP - temperature('C)\n");
                sal_fprintf(fp, "        LM - power limit(W)\tCS - consumption(W)\tAP - available power(W)\n");
                sal_fprintf(fp, "        DIS - disable\tEN - enable\n");
                sal_fprintf(fp, "        CR - critical\tHI - high\tLO - low\n");
                sal_fprintf(fp, "        OP - operation status of poe\n");
                sal_fprintf(fp, "------+----------+----+----+----+---+---+------+------+---+------+------+------\n");
                sal_fprintf(fp, "%-6s%-11s%-5s%-5s%-5s%-4s%-4s%-7s%-7s%-4s%-8s%-8s%-8s\n",
                                    "CH", "IF", "ST", "OP", "MD", "CL","PR","V","mA","TP","LM","CS","AP");
                for (i = 0; i < MAX_POE_PORT_NUM; i++)
                {
                    if((p_db_poe->pse_channel_id[i]+1!=0)&&(p_db_poe->panel_port_no[i]!=0))
                    {
                        sal_snprintf(power_limit_str, sizeof(power_limit_str), "%.3f",p_db_poe->power_limit[i]/1000.0);     
                        
                        if(p_db_poe->status[i])
                        {
                            sal_snprintf(class_str, sizeof(class_str), "%d",p_db_poe->class_level[i]);
                            sal_snprintf(voltage_str, sizeof(voltage_str), "%.1f",p_db_poe->voltage[i]/10.0);
                            sal_snprintf(current_str, sizeof(current_str), "%d",p_db_poe->current[i]);
                            sal_snprintf(cur_consump_str, sizeof(cur_consump_str), "%.3f",p_db_poe->cur_consump[i]/1000.0);
                            sal_snprintf(avail_str, sizeof(avail_str), "%.3f",
                                p_db_poe->power_limit[i]>p_db_poe->cur_consump[i]?((p_db_poe->power_limit[i] - p_db_poe->cur_consump[i])/1000.0):0);
                        }
                        else
                        {
                            sal_snprintf(class_str, sizeof(class_str), "--");
                            sal_snprintf(voltage_str, sizeof(voltage_str), "%.1f",0.0);
                            sal_snprintf(current_str, sizeof(current_str), "%d",0);
                            sal_snprintf(cur_consump_str, sizeof(cur_consump_str), "%.3f",0.0);
                            sal_snprintf(avail_str, sizeof(avail_str), "%.3f",p_db_poe->power_limit[i]/1000.0);
                        }

                        if_build_port_fullname_by_slot_port(port_name, (uint8)p_db_poe->panel_slot_no[i],
                                    (uint8)p_db_poe->panel_port_no[i], (uint8)p_db_poe->panel_sub_port_no[i]);
                        sal_fprintf(fp, "%d-%-4d%-11s%-5s%-5s%-5s%-4s%-4s%-7s%-7s%-4d%-8s%-8s%-8s\n",
                                    p_db_poe->key.id+1,p_db_poe->pse_channel_id[i]+1,port_name,
                                    p_db_poe->enable[i]?"EN":"DIS",p_db_poe->status[i]?"ON":"OFF", 
                                    p_db_poe->mode[i]==POE_AF?"AF":(p_db_poe->mode[i]==POE_AT?"AT":"POH"), 
                                    class_str,p_db_poe->priority[i]==1?"CR":(p_db_poe->priority[i]==2?"HI":"LO"),
                                    voltage_str,current_str,p_db_poe->status[i]?p_db_poe->temper[i]:0,
                                    power_limit_str,cur_consump_str,avail_str);
                    }
                }

            }
        }
        else if (0 == sal_strcmp(argv[1], "interface"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            if_get_slot_port_by_fullname(argv[2],(uint8*)&panel_slot_no,(uint8*)&panel_port_no,(uint8*)&panel_sub_port_no);
            find_item = 0;
            CTCLIB_SLIST_LOOP(p_poe_master->poe_list, p_db_poe, listnode)
            {
                if(find_item)
                    break;
                for (i = 0; i < MAX_POE_PORT_NUM; i++)
                {
                    if((p_db_poe->panel_slot_no[i] == panel_slot_no)
                        &&(p_db_poe->panel_port_no[i] == panel_port_no)
                        &&(p_db_poe->panel_sub_port_no[i] == panel_sub_port_no))
                    {
                        sal_fprintf(fp, "Power Channel: %d\n",p_db_poe->pse_channel_id[i]+1);
                        sal_fprintf(fp, "Interface Name: %s\n",argv[2]);
                        sal_fprintf(fp, "Status: %s\n",p_db_poe->enable[i]?"enable":"disable");
                        sal_fprintf(fp, "Power Operation: %s\n",p_db_poe->status[i]?"ON":"OFF");
                        sal_fprintf(fp, "Power Mode: %s\n",p_db_poe->mode[i]==POE_AF?"AF":(p_db_poe->mode[i]==POE_AT?"AT":"POH"));

                        sal_snprintf(power_limit_str, sizeof(power_limit_str), "%.3f",p_db_poe->power_limit[i]/1000.0);
                        if(p_db_poe->status[i])
                        {
                            sal_snprintf(class_str, sizeof(class_str), "%d",p_db_poe->class_level[i]);
                            sal_snprintf(voltage_str, sizeof(voltage_str), "%.1f",p_db_poe->voltage[i]/10.0);
                            sal_snprintf(current_str, sizeof(current_str), "%d",p_db_poe->current[i]);
                            sal_snprintf(cur_consump_str, sizeof(cur_consump_str), "%.3f",p_db_poe->cur_consump[i]/1000.0);
                            sal_snprintf(avail_str, sizeof(avail_str), "%.3f",
                                p_db_poe->power_limit[i]>p_db_poe->cur_consump[i]?((p_db_poe->power_limit[i] - p_db_poe->cur_consump[i])/1000.0):0);
                        }
                        else
                        {
                            sal_snprintf(class_str, sizeof(class_str), "--");
                            sal_snprintf(voltage_str, sizeof(voltage_str), "%.1f",0.0);
                            sal_snprintf(current_str, sizeof(current_str), "%d",0);
                            sal_snprintf(cur_consump_str, sizeof(cur_consump_str), "%.3f",0.0);
                            sal_snprintf(avail_str, sizeof(avail_str), "%.3f",p_db_poe->power_limit[i]/1000.0);
                        }                        
  
                        sal_fprintf(fp, "Power Class: %s\n",class_str);
                        sal_fprintf(fp, "Power Priority: %s\n",p_db_poe->priority[i]==1?"critical":(p_db_poe->priority[i]==2?"high":"low"));
                        sal_fprintf(fp, "Voltage: %s V\n",voltage_str);
                        sal_fprintf(fp, "Current: %s mA\n",current_str);
                        sal_fprintf(fp, "Temperature: %d C\n",p_db_poe->status[i]?p_db_poe->temper[i]:0);

                        sal_fprintf(fp, "Power limit: %s W\n",power_limit_str);
                        sal_fprintf(fp, "Power Consump: %s W\n",cur_consump_str);

                        sal_fprintf(fp, "Power available: %s W\n",avail_str);
                        find_item = 1;
                        break;
                    }
                }
            }
            if(!find_item)
            {
                CFG_INVALID_PARAM_RET("Unsupport poe on interface %s", argv[2]);
            }            
        }
    }
    sal_fclose(fp);
    return PM_E_NONE;
}

/* add by wangyb for bug 44518, 2017-08-07*/
struct management_if_info_e
{
    char link_encap[CMD_BUF_64];
    char hwaddr[CMD_BUF_64];               /* MAC */
    char inet_addr[CMD_BUF_64];           /* mgmt if ip address */
    char bcast[CMD_BUF_64];                 /* broadcast address */
    char mask[CMD_BUF_64];
    char if_state[CMD_BUF_64];             /* mgmt if current state */
    char inet6_addr_link[CMD_BUF_64];
    char inet6_addr_global[CMD_BUF_64];
    char inet6_addr1_global[CMD_BUF_64];
    char inet6_addr2_global[CMD_BUF_64];
    int32 mtu;
    uint64 rx_packets;            /* receive packets */
    uint64 tx_packets;            /* transmit packets */
    uint64 rx_bytes;               /* receive bytes */
    uint64 tx_bytes;               /* transmit bytes */
    double rx_bytes_convert;
    double tx_bytes_convert;
    char rx_convert_uint[CMD_BUF_16];
    char tx_convert_uint[CMD_BUF_16];
    char speed[CMD_BUF_16];
    char duplex[CMD_BUF_16];
    /* modified by liwh for bug 50685, 2019-01-21 */
    uint8 inet6_addr_global_count;
    /* liwh end */
};
typedef struct management_if_info_e management_if_info_t;

static management_if_info_t g_mgmt_if_info_clear_counters = {"N/A", "N/A", "N/A", "N/A", "N/A", "N/A",  "N/A", "N/A",  "N/A", "N/A", 0,
                                                                                             0, 0, 0, 0, 0, 0, "N/A", "N/A", "N/A", "N/A", 0};
static const char TRext[] = "\0\0\0Ki\0Mi\0Gi\0Ti";

static int
_chsm_get_mgmt_if_info(management_if_info_t * mgmt_if_info, char * if_name)
{
    char szCmd[CMD_BUF_256] = {0};
    FILE *fp = NULL;
    char buf[CMD_BUF_1024];
    char *temp_buf = NULL;   
 
    sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ifconfig %s > %s", if_name, GLB_CONFIG_MGMT_INFO);
    system(szCmd);

    sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ethtool %s > %s", if_name, GLB_CONFIG_MGMT_ETHTOOL);
    system(szCmd);

    if (syscmd_file_exist(GLB_CONFIG_MGMT_ETHTOOL))
    {
        fp = sal_fopen(GLB_CONFIG_MGMT_ETHTOOL, SAL_OPEN_RO);
        if (NULL != fp)
        {
            while (NULL != sal_fgets(buf, CMD_BUF_1024, fp))
            {
                if (NULL != (temp_buf = sal_strstr(buf, "Speed")))
                {
                    sal_sscanf(temp_buf, "Speed: %s", mgmt_if_info->speed);
                }
                if (NULL != (temp_buf = sal_strstr(buf, "Duplex")))
                {
                    sal_sscanf(temp_buf, "Duplex: %s", mgmt_if_info->duplex);
                }
            }
            sal_fclose(fp);
        }
    }
    
    if (syscmd_file_exist(GLB_CONFIG_MGMT_INFO))
    {
        fp = sal_fopen(GLB_CONFIG_MGMT_INFO, SAL_OPEN_RO);
        if (NULL != fp)
        {   
            while (NULL != sal_fgets(buf, CMD_BUF_1024, fp))
            {
            
                // line 1 "Link encap..."
                if (NULL != (temp_buf = sal_strstr(buf, "Link encap")))
                {
                    sal_sscanf(temp_buf, "Link encap:%s", mgmt_if_info->link_encap);
                }
                if (NULL != (temp_buf = sal_strstr(buf, "HWaddr")))
                {
                    sal_sscanf(temp_buf, "HWaddr %s", mgmt_if_info->hwaddr);
                    continue;
                }
                
                 // line 2 "inet addr..."   
                if (NULL != (temp_buf = sal_strstr(buf, "inet addr")))
                {
                    sal_sscanf(temp_buf, "inet addr:%s", mgmt_if_info->inet_addr);
                }
                if (NULL != (temp_buf = sal_strstr(buf, "Bcast")))
                {
                    sal_sscanf(temp_buf, "Bcast:%s", mgmt_if_info->bcast);
                }
                if (NULL != (temp_buf = sal_strstr(buf, "Mask")))
                {
                    sal_sscanf(temp_buf, "Mask:%s", mgmt_if_info->mask);
                    continue;
                }

                if (NULL != (temp_buf = sal_strstr(buf, "inet6 addr")))
                {
                    if (sal_strstr(buf, "Link"))
                    {
                        sal_sscanf(temp_buf, "inet6 addr:%s", mgmt_if_info->inet6_addr_link);
                        /*add by chenc for bug 53946, 2019-10-17*/
                        if (NULL != (temp_buf = sal_strstr(mgmt_if_info->inet6_addr_link, "/")))
                        {
                            temp_buf[0] = '\0';
                        }
                        /*chenc end*/
                    }
                    else
                    {
                        /* modified by liwh for bug 50685, 2019-01-21 */
                        if (0 == mgmt_if_info->inet6_addr_global_count)
                        {
                            sal_sscanf(temp_buf, "inet6 addr:%s", mgmt_if_info->inet6_addr_global);
                        }
                        else if (1 == mgmt_if_info->inet6_addr_global_count)
                        {
                            sal_sscanf(temp_buf, "inet6 addr:%s", mgmt_if_info->inet6_addr1_global);
                        }
                        else if (2 == mgmt_if_info->inet6_addr_global_count)
                        {
                            sal_sscanf(temp_buf, "inet6 addr:%s", mgmt_if_info->inet6_addr2_global);
                        }
                        mgmt_if_info->inet6_addr_global_count++;
                        /* liwh end */
                    }
                    continue;
                }
               
                // line 5 interface flags, mtu, metric
                if (NULL != (temp_buf = sal_strstr(buf, "MTU")))
                {
                    sal_sscanf(temp_buf, "MTU:%d", &mgmt_if_info->mtu);
                    if (sal_strstr(buf, "UP") && sal_strstr(buf, "RUNNING"))
                    {
                        sal_snprintf(mgmt_if_info->if_state, 16, "%s", "UP");
                    }
                    else
                    {
                        sal_snprintf(mgmt_if_info->if_state, 16, "%s", "DOWN");
                    }
                    continue;
                }

                // line 6 RX packets
                if (NULL != (temp_buf = sal_strstr(buf, "RX packets")))
                {
                    sal_sscanf(temp_buf, "RX packets:%"PRIu64, &mgmt_if_info->rx_packets);
                    continue;
                }
                
                // line 7 TX packets
                if (NULL != (temp_buf = sal_strstr(buf, "TX packets")))
                {
                    sal_sscanf(temp_buf, "TX packets:%"PRIu64, &mgmt_if_info->tx_packets);
                    continue;
                }

                // line 8 collisions, txqueuelen <- don't care
                if (NULL != (temp_buf = sal_strstr(buf, "collisions")))
                {
                    continue;
                }

                // line 9 RX bytes TX bytes
                if (NULL != (temp_buf = sal_strstr(buf, "RX bytes")))
                {
                    sal_sscanf(temp_buf, "RX bytes:%"PRIu64, &mgmt_if_info->rx_bytes);              
                }
                if (NULL != (temp_buf = sal_strstr(buf, "TX bytes")))
                {
                    sal_sscanf(temp_buf, "TX bytes:%"PRIu64, &mgmt_if_info->tx_bytes);
                }
            }
           sal_fclose(fp);
        }
    }
    return PM_E_NONE;
}

/* convert from B to KiB/MiB/GiB/TiB */
static int 
print_bytes_scaled(FILE *fp, uint64 bytes)
{
    uint64 int_part = 0;
    const char *ext = NULL;
    uint32 frac_part = 0;
    uint32 i = 0;
    char str_rx_bytes[CMD_BUF_64] = {0};
    char str_int_part[CMD_BUF_64] = {0};

    frac_part = 0;
    ext = TRext;
    int_part = bytes;
    i = 4;
    do {
        if (int_part >= 1024) {
            frac_part = ((((unsigned int) int_part) & (1024-1)) * 10) / 1024;
            int_part /= 1024;
            ext += 3;	/* KiB, MiB, GiB, TiB */
        }
        --i;
    } while (i);
    UINT64_TO_STR(bytes, str_rx_bytes);
    UINT64_TO_STR(int_part, str_int_part);
    
    sal_fprintf(fp, "%15s Bytes (%s.%d %sB)", str_rx_bytes, str_int_part, frac_part, ext);
    return PM_E_NONE;
}

extern int32
cfg_split_token(char *str, const char *delimiter, uint32 max_token, char **token_array, uint32 *token_count);

int32
_chsm_cmd_process_show_management_neighbor(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char *ifname = NULL;
    char szCmd[CMD_BUF_256] = {0};
    FILE *fp_info = NULL;
    FILE *fp = NULL;
    char buf[CMD_BUF_1024] = {0};
    char *p_ip_v6 = NULL;
    char *link_loacl_addr = NULL;
    char *p_mac = NULL;
    char *token_array[16] = {0};
    uint32 token_count = 0;
    char link_local[CMD_BUF_64]={0};
    ifname = chsm_manage_if_get_ifname();
    sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ip -6 neighbor | grep %s > %s", ifname, GLB_MGMT_NEIGHBOR_INFO);
    sal_system(szCmd);
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NONE;
    }
    _chsm_get_mgmt_if_linklocal(link_local);
    sal_fprintf(fp, "%-40s %-20s %-10s\n", "IPv6 Address", "Hardware Addr", "Interface");
    sal_fprintf(fp, "-----------------------------------------+--------------------+-------------\n");
    if (syscmd_file_exist(GLB_MGMT_NEIGHBOR_INFO))
    {
        fp_info = sal_fopen(GLB_MGMT_NEIGHBOR_INFO, SAL_OPEN_RO);
        if (NULL != fp_info)
        {
            while (NULL != sal_fgets(buf, CMD_BUF_1024, fp_info))
            {
                p_ip_v6 = NULL;
                p_mac = NULL;
                token_array[4] = 0;
                cfg_split_token(buf, " ", 8, token_array, &token_count);
                p_ip_v6 = token_array[0];
                p_mac = token_array[4];
                if (!p_ip_v6 || !p_mac)
                {
                    continue;
                }
                
                token_count= 0;
                cfg_split_token(p_ip_v6, "/", 8, token_array, &token_count);
                link_loacl_addr = token_array[0];
                if (sal_strcmp(link_loacl_addr,link_local))
                {
                    sal_fprintf(fp, "%-40s %-20s %-10s\n", p_ip_v6, p_mac, "mgmt-if");
                }
            }
            sal_fclose(fp_info);
        }
    }
    sal_fclose(fp);
    return PM_E_NONE;
}

int32
_chsm_cmd_process_show_management_arp(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char *ifname = NULL;
    char szCmd[CMD_BUF_256] = {0};
    FILE *fp_info = NULL;
    FILE *fp = NULL;
    char buf[CMD_BUF_1024];
    char *p_ip = NULL;
    char *p_mac = NULL;
    char *p_flag = NULL;
    char *token_array[16];
    uint32 token_count = 0;
 
    ifname = chsm_manage_if_get_ifname();
    sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" cat /proc/net/arp | grep %s > %s", ifname, GLB_MGMT_ARP_INFO);
    sal_system(szCmd);

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NONE;
    }

    sal_fprintf(fp, "%-16s %-20s %-10s\n", "Address", "Hardware Addr", "Interface");
    sal_fprintf(fp, "----------------+--------------------+----------\n");
    
    if (syscmd_file_exist(GLB_MGMT_ARP_INFO))
    {
        fp_info = sal_fopen(GLB_MGMT_ARP_INFO, SAL_OPEN_RO);
        if (NULL != fp_info)
        {
            while (NULL != sal_fgets(buf, CMD_BUF_1024, fp_info))
            {
                cfg_split_token(buf, " ", 8, token_array, &token_count);
                p_ip = token_array[0];
                p_mac = token_array[3];
                p_flag = token_array[2];
                if (!p_ip || !p_mac || !p_flag)
                {
                    continue;
                }
                if (sal_strcmp(p_flag, "0x2"))
                {
                    continue;
                }
                sal_fprintf(fp, "%-16s %-20s %-10s\n", p_ip, p_mac, "mgmt-if");
            }
            sal_fclose(fp_info);
        }
    }
    
    sal_fclose(fp);
    
    return PM_E_NONE;
}

int32
_chsm_cmd_process_show_management_if(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char * if_name = NULL;
    management_if_info_t mgmt_if_info = {"N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "", "", "", "", 0,
                                                              0, 0, 0, 0, 0, 0, "N/A", "N/A", "N/A", "N/A", 0};
    FILE *fp = NULL;
    char str_rx_packets[CMD_BUF_64];
    char str_tx_packets[CMD_BUF_64];
    
    if_name = chsm_manage_if_get_ifname();

    /* clear counters mgmt-if, record mgmt info, when the next time to show mgmt info minus it */
    if (argc > 0)
    {
        if (0 == sal_strcmp(argv[0], "clear_counters"))
        {
            _chsm_get_mgmt_if_info(&g_mgmt_if_info_clear_counters, if_name);
        }
        return PM_E_NONE;
    }
    
    _chsm_get_mgmt_if_info(&mgmt_if_info, if_name);
    
    /* show counters = current counters - record counters */   
    mgmt_if_info.rx_packets -= g_mgmt_if_info_clear_counters.rx_packets;
    mgmt_if_info.rx_bytes -= g_mgmt_if_info_clear_counters.rx_bytes;
    mgmt_if_info.tx_packets -= g_mgmt_if_info_clear_counters.tx_packets;
    mgmt_if_info.tx_bytes -= g_mgmt_if_info_clear_counters.tx_bytes;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NONE;
    }
    
    sal_fprintf(fp, "Management Interface current state: %s\n", mgmt_if_info.if_state);
    if (0 == sal_strcmp(mgmt_if_info.if_state, "DOWN"))
    {
        /* modified by wangyb for bug 45067, 2017-08-28*/
        sal_strcpy(mgmt_if_info.speed, "Auto");
        sal_strcpy(mgmt_if_info.duplex, "Auto");
    }
    sal_fprintf(fp, "Description:\n");
    sal_fprintf(fp, "Link encap: %-14s HWaddr: %s\n", mgmt_if_info.link_encap, mgmt_if_info.hwaddr);
    sal_fprintf(fp, "net addr: %-15s  Mask: %s\n", mgmt_if_info.inet_addr, mgmt_if_info.mask);

    if (sal_strlen(mgmt_if_info.inet6_addr_link))
    {
        sal_fprintf(fp, "net6 addr: %s Scope:Link\n", mgmt_if_info.inet6_addr_link);
    }

    if (sal_strlen(mgmt_if_info.inet6_addr_global))
    {
        sal_fprintf(fp, "net6 addr: %s Scope:Global\n", mgmt_if_info.inet6_addr_global);
    }

    /* modified by liwh for bug 50685, 2019-01-21 */
    if (sal_strlen(mgmt_if_info.inet6_addr1_global))
    {
        sal_fprintf(fp, "net6 addr: %s Scope:Global\n", mgmt_if_info.inet6_addr1_global);
    }

    if (sal_strlen(mgmt_if_info.inet6_addr2_global))
    {
        sal_fprintf(fp, "net6 addr: %s Scope:Global\n", mgmt_if_info.inet6_addr2_global);
    }
    /* liwh end */
	
    sal_fprintf(fp, "Bcast: %-15s     ", mgmt_if_info.bcast);
    if (0 <= mgmt_if_info.mtu)
    {
        sal_fprintf(fp, "MTU: %-14d\n", mgmt_if_info.mtu);
    }
    else
    {
        sal_fprintf(fp, "MTU: %-14s\n", "N/A");
    }
    
    sal_fprintf(fp, "Speed: %-14s", mgmt_if_info.speed);
    sal_fprintf(fp, "      Duplex: %s\n", mgmt_if_info.duplex);
    sal_fprintf(fp, "%s\n", "Auto-negotiation: Enable");
    sal_fprintf(fp, "\n");
    UINT64_TO_STR(mgmt_if_info.rx_packets, str_rx_packets);
    sal_fprintf(fp, "Received:%18s Packets, ", str_rx_packets);
    print_bytes_scaled(fp, mgmt_if_info.rx_bytes);
    sal_fprintf(fp, "\n");
    UINT64_TO_STR(mgmt_if_info.tx_packets, str_tx_packets);
    sal_fprintf(fp, "Transmitted:%15s Packets, ", str_tx_packets);
    print_bytes_scaled(fp, mgmt_if_info.tx_bytes);
    sal_fprintf(fp, "\n");
        
    sal_fclose(fp);
    return PM_E_NONE;
}

int32
chsm_cmd_process_show_management_if(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    if ((1 == argc) && (!sal_strcmp(argv[0], "arp")))
    {
        return _chsm_cmd_process_show_management_arp(argv, argc, para);
    }
    else if ((1 == argc) && (!sal_strcmp(argv[0], "neighbor")))
    {
        return _chsm_cmd_process_show_management_neighbor(argv, argc, para);
    }
    else
    {
        return _chsm_cmd_process_show_management_if(argv, argc, para);
    }
}

int32
chsm_cmd_del_mgmt_link_local_address()
{
    char mgmt_link_local_addr[64] = {0};
    //char szCmd[CMD_BUF_256] = {0};
    tbl_manage_if_t *p_db_mng_if = NULL;
    prefix_t addr_v6;
    int32 rc = 0;
    
    sal_memset(&addr_v6, 0x00, sizeof(addr_v6));
    p_db_mng_if = tbl_manage_if_get_manage_if();
    if (!p_db_mng_if)
    {
        log_sys(M_MOD_CHSM, E_ERROR, "manage_if not exist\n");
        return PM_E_NONE;
    }

    char *ifname = NULL;
    ifname = chsm_manage_if_get_ifname();

    rc = chsm_get_mgmt_if_linklocal(mgmt_link_local_addr);
    if (rc != PM_E_NONE)
    {
        log_sys(M_MOD_CHSM, E_ERROR, "chsm get mgmt_if linklocal fail\n");
        return rc;
    }
    sal_memcpy(p_db_mng_if->link_local_addr_str, mgmt_link_local_addr, sizeof(p_db_mng_if->link_local_addr_str));

    if (0 != sal_memcmp(&p_db_mng_if->addr_v6, &addr_v6, sizeof(addr_v6)))
    {
        /*management ipv6 addr exist, not del link_local address*/
        return PM_E_NONE;
    }

    /* modified by liwh for bug 49365, 2018-12-17 */
    if (p_db_mng_if->dhclientv6_enable)
    {
        /*management ipv6 address dhcp enable, not del link_local address*/
        return PM_E_NONE;
    }
    /*liwh end */
    
    //sal_snprintf(szCmd, sizeof(szCmd), NETNS_CMD_PREFIX" ip -6 addr del %s dev %s >/dev/null 2>&1", mgmt_link_local_addr, ifname);
    //system(szCmd);
    _chsm_set_mgmt_if_disable_ipv6(ifname, 1);

    return PM_E_NONE;
}

int32
chsm_cmd_do_init_done()
{
    return PM_E_NONE;
}

static int32
chsm_cmd_sync_after_sys_global(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (oper)
    {
    case CDB_OPER_SET:
        if (TBL_SYS_GLOBAL_FLD_STARTUP_DONE == field_id)
        {
            chsm_cmd_del_mgmt_link_local_address();
        }
        if (TBL_SYS_GLOBAL_FLD_INIT_DONE == field_id)
        {
            chsm_cmd_do_init_done();
        }
        break;

    case CDB_OPER_GET:
        break;

    default:
        break;
    }
    
    return rc;
}

int32
chsm_cmd_init()
{
    cdb_register_cfg_tbl_cb(TBL_VERSION, chsm_cmd_process_version);
    cdb_register_cfg_tbl_cb(TBL_MANAGE_IF, chsm_cmd_process_manage_if);
    cdb_register_cfg_tbl_cb(TBL_BOOTIMAGE, chsm_cmd_process_bootimage);
    cdb_register_cfg_tbl_cb(TBL_CHASSIS, chsm_cmd_process_chassis);
    cdb_register_cfg_tbl_cb(TBL_CARD, chsm_cmd_process_card);
    cdb_register_cfg_tbl_cb(TBL_FAN, chsm_cmd_process_fan);
    cdb_register_cfg_tbl_cb(TBL_PSU, chsm_cmd_process_psu);
    cdb_register_cfg_tbl_cb(TBL_SENSOR, chsm_cmd_process_sensor);
    cdb_register_cfg_tbl_cb(TBL_VCM, chsm_cmd_process_vcm);
    cdb_register_cfg_tbl_cb(TBL_POE, chsm_cmd_process_poe);
    cdb_register_cfg_tbl_cb(TBL_PORT, chsm_cmd_process_port);
    cdb_register_cfg_tbl_cb(TBL_FIBER, chsm_cmd_process_fiber_info);
    cdb_register_cfg_tbl_cb(TBL_REBOOT_INFO, chsm_cmd_process_reboot_info);
    cdb_register_cfg_tbl_cb(TBL_CHIP, chsm_cmd_process_chip);
    
    cdb_register_cfg_act_cb(ACT_CLOCK, chsm_cmd_process_clock);
    cdb_register_cfg_act_cb(ACT_UPDATE_EPLD, chsm_cmd_process_update_epld);
    cdb_register_cfg_act_cb(ACT_UPDATE_BOOTROM, chsm_cmd_process_update_bootrom);
    cdb_register_cfg_act_cb(ACT_UPDATE_PHY_FIRMWARE, chsm_cmd_process_update_phy_firmware);
    cdb_register_cfg_act_cb(ACT_BOOTUPDIAG, chsm_cmd_process_bootupdiag);
    cdb_register_cfg_act_cb(ACT_REBOOT, chsm_cmd_process_reboot);
    cdb_register_cfg_act_cb(ACT_SHOW_SCHEDULE_REBOOT, chsm_cmd_process_show_schedule_reboot);
    cdb_register_cfg_act_cb(ACT_UPDATE_PORT_CONNECTION, chsm_cmd_process_update_port_connection);
    cdb_register_cfg_act_cb(ACT_SHOW_ENVIRONMENT, chsm_cmd_process_show_environment);
    cdb_register_cfg_act_cb(ACT_SHOW_POE, chsm_cmd_process_show_poe);
    cdb_register_cfg_act_cb(ACT_SHOW_PSU_PMBUS_INFO, chsm_cmd_process_show_psu_pmbus_info);
    cdb_register_cfg_act_cb(ACT_SHOW_VCM, chsm_cmd_process_show_vcm);
    cdb_register_cfg_act_cb(ACT_SHOW_MANAGEMENT_IF, chsm_cmd_process_show_management_if);
    cdb_register_cfg_act_cb(ACT_FACTORY_CONFIG, chsm_cmd_process_factory_config);
    cdb_register_cfg_act_cb(ACT_SHOW_FIBER, chsm_cmd_show_fiber_info);


    cdb_register_cfg_act_cb(ACT_GENERATE_OEM_FILE, chsm_cmd_process_generate_oem_file);
    cdb_register_cfg_act_cb(ACT_UPDATE_OEM_FILE, chsm_cmd_process_update_oem_file);
    
    cdb_pm_subscribe_tbl(TBL_SYS_GLOBAL, NULL, chsm_cmd_sync_after_sys_global);
    cdb_pm_subscribe_tbl(TBL_CARD, NULL, chsm_card_sync);
    cdb_pm_subscribe_tbl(TBL_PORT, chsm_port_sync, NULL);
    /* Modfied by liuht for bug43823, 2017-0601 */
    cdb_pm_subscribe_tbl(TBL_FIBER, chsm_fiber_sync, chsm_fiber_sync_after);
    cdb_pm_subscribe_tbl(TBL_FAN, chsm_fan_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_PSU, chsm_psu_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_SENSOR, chsm_sensor_sync, chsm_sensor_sync_after);
    cdb_pm_subscribe_tbl(TBL_VCM, chsm_vcm_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_POE, chsm_poe_sync, NULL);
    /* added by hansf for process set cur_stm_type */
    cdb_pm_subscribe_tbl(TBL_CHASSIS, NULL, chsm_chassis_sync_after);
    cdb_pm_subscribe_tbl(TBL_TAG_SUMMER_CLOCK, NULL, NULL);
    return PM_E_NONE;
}

