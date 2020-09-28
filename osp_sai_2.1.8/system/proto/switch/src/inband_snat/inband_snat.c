#include "proto.h"
#include "switch.h"

#include "inband_snat.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_inband_snat_define.h"
#include "gen/tbl_inband_snat.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/ds_connected_define.h"
#include "gen/ds_connected.h"
#include "gen/tbl_nat_global_define.h"
#include "gen/tbl_nat_global.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>


#ifdef _GLB_UML_SYSTEM_
#define INBAND_SNAT_IPTABLES_PREFIX "iptables"
#else
#define INBAND_SNAT_IPTABLES_PREFIX "xtables-multi iptables"
#endif

/* modified by liwh for bug 53499, 2018-09-12 */
#define INBAND_SNAT_SOURCE_IP_TIME_INTERVAL   60
/* liwh end */

typedef enum {
    INBAND_SNAT_RULE_ADD,
    INBAND_SNAT_RULE_DEL,
} inband_snat_rule_op_t;


tbl_inband_snat_t *
inband_sant_get_info_by_ifname(const char *ifname)
{
    tbl_inband_snat_master_t *p_master = tbl_inband_snat_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_inband_snat_t *p_db_inband_snat = NULL;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->inband_snat_list, p_db_inband_snat, listnode, next)
    {
        if (0 == sal_strcmp(ifname, p_db_inband_snat->ifname))
        {
            return p_db_inband_snat; 
        }
    }

    return NULL;
}


static int32
_inband_snat_rule_op(tbl_inband_snat_t *p_nat, inband_snat_rule_op_t op)
{
    char dst_ip[16];
    char src_ip[16];
    char *cmd_buf = NULL;
    int32 rc = PM_E_NONE; 
    
    cmd_buf = (char *)sal_malloc(1024);
    if (NULL == cmd_buf)
    {
        return PM_E_NO_MEMORY;
    }

    sal_inet_ntop(AF_INET, (void *)&p_nat->ip, src_ip, sizeof(src_ip));
    sal_inet_ntop(AF_INET, (void *)&p_nat->key.ip, dst_ip, sizeof(dst_ip));
    
#if defined (TSINGMA) && !defined (TAPPRODUCT) && !defined (OFPRODUCT)
    tbl_nat_global_t *p_nat_glb = NULL;
    p_nat_glb = tbl_nat_global_get_nat_global();

    if (INBAND_SNAT_RULE_ADD == op)
    {
        p_nat_glb->nft_hdl++;
        p_nat->nft_handle = p_nat_glb->nft_hdl;
        sal_sprintf(cmd_buf, "nft insert rule nat postrouting handle %u ip daddr %s %s dport %u snat %s",
                    p_nat_glb->nft_hdl_upstream_filter_nat_rule,
                    dst_ip,
                    p_nat->key.proto == INBAND_IP_PROTO_TCP ? "tcp" : "udp",
                    p_nat->key.port,
                    src_ip);
        tbl_nat_global_set_nat_global_field(p_nat_glb, TBL_NAT_GLOBAL_FLD_NFT_HDL);
    }
    else if (INBAND_SNAT_RULE_DEL == op)
    {
        if (p_nat->nft_handle)
        {
            sal_sprintf(cmd_buf, "nft delete rule nat postrouting handle %u", p_nat->nft_handle);
            p_nat->nft_handle = 0;
        }
    }
    
    system(cmd_buf);
#else
    sal_sprintf(cmd_buf, INBAND_SNAT_IPTABLES_PREFIX" -t nat -%s POSTROUTING -d %s -p %s --dport %hu -j SNAT --to-source %s",
                op == INBAND_SNAT_RULE_ADD ? "A" : "D",
                dst_ip,
                p_nat->key.proto == INBAND_IP_PROTO_TCP ? "tcp" : "udp",
                p_nat->key.port,
                src_ip);
    
    if (0 != system(cmd_buf) >> 8)
    {
        rc = PM_E_FAIL;
    }

#endif

#ifndef _GLB_UML_SYSTEM_
#ifdef TAPPRODUCT
    /* NTP */
    if(INBAND_SNAT_RULE_DEL == op && p_nat->key.port == 123)
    {
        sal_sprintf(cmd_buf, "conntrack -D -p udp -d %s",
                dst_ip);

        system(cmd_buf);
    }
#endif
#endif
    sal_free(cmd_buf);

    return rc;
}


static int32
_inband_snat_dump_all(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
 
    tbl_inband_snat_iterate((TBL_ITER_CB_FUNC)tbl_inband_snat_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}


static int32
_inband_snat_string_to_key(cfg_cmd_para_t *para, const char *str, inband_snat_key_t *p_key)
{
    int32 rc = PM_E_NONE;
    char ip_buf[GLB_IPV6_STR_LEN] = "";
    int  proto;  /* 0-tcp, 1-udp */
    addr_t  ip;

    sal_memset(p_key, 0, sizeof(inband_snat_key_t));

    if (3 != sal_sscanf(str, "%d#%hu#%s", &proto, &p_key->port, ip_buf))
    {
        CFG_INVALID_PARAM_RET("Invalid parameter %s", str);
    }

    /* verify params */
    p_key->proto = proto;
    if (p_key->proto >= INBAND_IP_PROTO_MAX)
    {
        CFG_INVALID_PARAM_RET("Invalid protocol: %d", (int)p_key->proto);
    }

    sal_memset(&ip, 0, sizeof(ip));
    if (1 == sal_inet_pton(AF_INET, ip_buf, &ip.u.prefix4))
    {
        ip.family = AF_INET;
    }
    else if (1 == sal_inet_pton(AF_INET6, ip_buf, &ip.u.prefix6))
    {
        ip.family = AF_INET6;
    }
    else
    {
        CFG_INVALID_PARAM_RET("Invalid ip address: %s", ip_buf);
    }

    if (ip.family == AF_INET6)
    {
        /* current not support ipv6 inband snat */
        return PM_E_NOT_SUPPORT;
    }

    sal_memcpy(&p_key->ip, &ip.u.prefix4, sizeof(addr_ipv4_t));
	
    return rc;
}


static int32
_inband_snat_parse_args(char **argv, int32 argc, cfg_cmd_para_t *para, tbl_inband_snat_t *p_nat)
{
    int32 rc = PM_E_NONE;
    int i;
    uint32 has_type = FALSE;
    uint32 has_ifname = FALSE;
    uint32 has_ip = FALSE;

    for (i = 0; i < argc; i++)
    {
        if (0 == sal_strcmp("type", argv[i]))
        {
            i++;
            p_nat->type = atoi(argv[i]);
            if (p_nat->type >= INBAND_SNAT_TYPE_MAX || p_nat->type == INBAND_SNAT_TYPE_INVALID)
            {
                CFG_INVALID_PARAM_RET("Invalid type: '%s'", argv[i]);
            }
            has_type = TRUE;
        }
        else if (0 == sal_strcmp("ifname", argv[i]))
        {
            i++;
            if ('\0' != argv[i][0])
            {
                sal_strcpy(p_nat->ifname, argv[i]);
                has_ifname = TRUE;
            }
            else
            {
                continue;
            }
        }
        else if (0 == sal_strcmp("ip", argv[i]))
        {
            i++;
            if ('\0' != argv[i][0])
            {
                if (1 != sal_inet_pton(AF_INET, argv[i], &p_nat->ip))
                {
                    CFG_INVALID_PARAM_RET("Invalid IP address: %s", argv[i]);
                }

                uint32 ntoh_s_addr = sal_ntoh32(p_nat->ip.s_addr);
                /* Check IP address */
                if (IN_EXPERIMENTAL(ntoh_s_addr) 
                || IN_CLASSD(ntoh_s_addr)
                || IN_LOOPBACK(ntoh_s_addr)
                || IPV4_NET0(ntoh_s_addr))
                {
                    CFG_INVALID_PARAM_RET("Invalid IP address: %s", argv[i]);
                }
                has_ip = TRUE;
            }
            else
            {
                continue; 
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid argument: '%s'", argv[i]);
        }
    }

    if (!has_type)
    {
        CFG_INVALID_PARAM_RET("'type' is required");
    }
    if (!has_ifname && !has_ip)
    {
        CFG_INVALID_PARAM_RET("'ifname' or 'ip' is required");
    }

    return rc;
}


static int32
_inband_snat_get_ipaddr_by_ifname(const char *ifname, addr_ipv4_t *p_ip)
{
    int fd;
    struct ifreq ifr;
    int res;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);

    res = ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);

    memcpy(p_ip, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, sizeof(addr_ipv4_t));

    return (0 == res) ? PM_E_NONE : PM_E_FAIL;
}

/* modified by liwh for bug 53499, 2018-09-12 */
int32
inband_snat_delete_tftp_ftp_scp_entry_by_nat(tbl_inband_snat_t   *p_nat)
{
    int32                      rc = PM_E_NONE;
    char                       cmd_buf[200] = {0};
    char                       dst_ip[16] = {0};
    char                       src_ip[16] = {0};

    sal_inet_ntop(AF_INET, (void *)&p_nat->ip, src_ip, sizeof(src_ip));
    sal_inet_ntop(AF_INET, (void *)&p_nat->key.ip, dst_ip, sizeof(dst_ip));

#if defined (GOLDENGATE) || defined (GREATBELT)
    /* del rule to kernel*/
    sal_sprintf(cmd_buf, INBAND_SNAT_IPTABLES_PREFIX" -t nat -D POSTROUTING -d %s -j SNAT --to-source %s",
                dst_ip, src_ip);
    system(cmd_buf);
#endif

#if defined(DUET2)
    sal_sprintf(cmd_buf, "nft delete table nat");
    system(cmd_buf);
#endif

#if defined(TSINGMA)
#if defined (TAPPRODUCT) || defined (OFPRODUCT)
     sal_sprintf(cmd_buf, "nft delete table nat");
    system(cmd_buf);
#else
    if (p_nat->nft_handle)
    {
        sal_sprintf(cmd_buf, "nft delete rule nat postrouting handle %u", p_nat->nft_handle);
        system(cmd_buf);
        p_nat->nft_handle = 0;
    }
#endif
#endif

    if (p_nat->snat_timer)
    {
        CTC_TASK_STOP_TIMER(p_nat->snat_timer);
    }
     
    /* delete nat entry from db */
    tbl_inband_snat_del_inband_snat(&p_nat->key);

    return rc;
}

#define CTCSDK_PRINT(fmt, args...)                             \
    {                                                              \
    FILE * fp_console = NULL;                                  \
    fp_console = fopen("/tmp/temp0916", "a+");                   \
    fprintf(fp_console, fmt, ##args);                      \
    fclose(fp_console);                                        \
    }

bool 
inband_snat_check_tftp_ftp_scp_process_file(void)
{
    static char    line[128] = {};
    char            *tmpFile  = "/tmp/tmp_source_ip_tables";
    FILE            *fp      = NULL;

    CTCSDK_PRINT("%s %d\n\n", __func__, __LINE__);

    /* 1. open tmp_source_ip_tables */
    fp = fopen(tmpFile, "r");
    if (NULL == fp)
    {
        printf("open /tmp/tmp_source_ip_tables failed");
        return FALSE;
    }

    sal_memset(line, 0, 128);
    fgets(line, sizeof(line), fp);

    CTCSDK_PRINT("%s %d line %s\n\n", __func__, __LINE__, line);

    if(strlen(line) > 1)
    {
        return TRUE;
    }

    return FALSE;
}

bool
inband_snat_check_tftp_ftp_scp_process_exist(void)
{
    static char    line[128] = {};
    char            *tmpFile  = "/tmp/tmp_source_ip_tables";
    
    /* check tftp */
    sprintf(line, "ps -u root | grep ctctftp |  awk '{print $1}' > %s", tmpFile);
    if(system(line) != 0)
    {
        return FALSE;
    }

    if (inband_snat_check_tftp_ftp_scp_process_file())
    {
        return TRUE;
    }

    /* check scp */
    sprintf(line, "ps -u root | grep scp |  awk '{print $1}' > %s", tmpFile);
    if(system(line) != 0)
    {
        return FALSE;
    }

    if (inband_snat_check_tftp_ftp_scp_process_file())
    {
        return TRUE;
    }

    /* check ftp */
    sprintf(line, "ps -u root | grep fnconvert |  awk '{print $1}' > %s", tmpFile);
    if(system(line) != 0)
    {
        return FALSE;
    }

    if (inband_snat_check_tftp_ftp_scp_process_file())
    {
        return TRUE;
    }

    return FALSE;
}

void inband_snat_timer_fn(void *p_data)
{
    tbl_inband_snat_t     *p_nat = (tbl_inband_snat_t*)p_data;

    p_nat->snat_timer = NULL;
    if (p_nat)
    {
        if (inband_snat_check_tftp_ftp_scp_process_exist())
        {
            p_nat->snat_timer = ctc_task_add_timer(TASK_PRI_NORMAL, inband_snat_timer_fn, 
                p_nat, INBAND_SNAT_SOURCE_IP_TIME_INTERVAL);
        }
        else
        {
            inband_snat_delete_tftp_ftp_scp_entry_by_nat(p_nat);     
        }
    }

    return;
}
/* liwh end */

/* modified by liwh for bug 53343, 2019-08-28 */
static int32
_inband_snat_add_tftp_ftp_scp_entry(char **argv, int32 argc, cfg_cmd_para_t *para, tbl_sys_global_t *p_sys_glb)
{
    int32                      rc = PM_E_NONE;
    tbl_inband_snat_t   nat;
    tbl_inband_snat_t   *p_nat = NULL;
    addr_t                    addr;
    char                       dst_ip[200] = {0};
    char                       src_ip[16] = {0};
    uint32                    index_i = 0;
    uint32                    index_j = 0;
    uint32                    length = 0;
    bool                        is_addr = FALSE;
    uint32                    ntoh_s_addr = 0;
    char                       cmd_buf[200] = {0};
#if defined(TSINGMA) && !defined (TAPPRODUCT) && !defined (OFPRODUCT)
    tbl_nat_global_t          *p_nat_glb = NULL;
#endif

    sal_memset(&nat, 0, sizeof(tbl_inband_snat_t));

    /* get nat src ip */
    sal_strcpy(src_ip, argv[argc -1]);
    
    /* get nat dst ip */
    /* 10.10.38.160 */
    if (sal_strlen(argv[2]) <= 15)
    {
        sal_strcpy(dst_ip, argv[2]);
    }

    /* 10.10.38.160: */
    if ((1 != sal_inet_pton(AF_INET, dst_ip, &nat.key.ip))
        && (argv[2][0] >= '1') && (argv[2][0] <= '9'))
    {
        sal_memset(dst_ip, 0, 200);
        length = sal_strlen(argv[2]);
        for (index_i = 0; index_i < length; index_i++)
        {
            if (argv[2][index_i] == ':')
            {
                break;
            }
            else
            {
                dst_ip[index_j] = argv[2][index_i];
                index_j++;
            }   
        }
    }

    /* @10.10.38.160: */
    if (1 != sal_inet_pton(AF_INET, dst_ip, &nat.key.ip))
    {
        sal_memset(dst_ip, 0, 200);
        length = sal_strlen(argv[2]);
        for (index_i = 0; index_i < length; index_i++)
        {
            if (argv[2][index_i] == '@')
            {
                is_addr = TRUE;   
            }
            else
            {
                if (is_addr)
                {
                    if (argv[2][index_i] == ':')
                    {
                        break;
                    }
                    else
                    {
                        dst_ip[index_j] = argv[2][index_i];
                        index_j++;
                    }
                }
            }
        }
    }

    /* get dst ip and check dst ip */
    if (1 != sal_inet_pton(AF_INET, dst_ip, &nat.key.ip))
    {
        CFG_INVALID_PARAM_RET("Source-ip can't be configured when server IP address is invalid");
    }

    ntoh_s_addr = sal_ntoh32(nat.key.ip.s_addr);
    /* Check IP address */
    if (IN_EXPERIMENTAL(ntoh_s_addr) 
        || IN_CLASSD(ntoh_s_addr)
        || IN_LOOPBACK(ntoh_s_addr)
        || IPV4_NET0(ntoh_s_addr))
    {
        CFG_INVALID_PARAM_RET("Source-ip can't be configured when server IP address is invalid");
    }

    /* get src ip and check src ip */
    if (1 != sal_inet_pton(AF_INET, src_ip, &nat.ip))
    {
        CFG_INVALID_PARAM_RET("Invalid IP address: %s", src_ip);
    }

    ntoh_s_addr = sal_ntoh32(nat.ip.s_addr);
    /* Check IP address */
    if (IN_EXPERIMENTAL(ntoh_s_addr) 
        || IN_CLASSD(ntoh_s_addr)
        || IN_LOOPBACK(ntoh_s_addr)
        || IPV4_NET0(ntoh_s_addr))
    {
        CFG_INVALID_PARAM_RET("Invalid IP address: %s", src_ip);
    }

    /* check ip exist */
    if (p_sys_glb->startup_done)
    {
        sal_memset(&addr, 0, sizeof(addr));
        addr.family = AF_INET;
        addr.u.prefix4.s_addr = nat.ip.s_addr;
        if (FALSE == route_if_addr_exists(&addr))
        {
            CFG_CONFLICT_RET("IP address %s not configured", src_ip);
        }
    }

    nat.type = INBAND_SNAT_TYPE_TFTP_FTP_SCP_SERVER;

    /* check entry exist */
    p_nat = tbl_inband_snat_get_inband_snat(&nat.key);
    if (p_nat)
    {
        /* modified by liwh for bug 53499, 2018-09-12 */
        if (inband_snat_check_tftp_ftp_scp_process_exist())
        {
            CFG_INVALID_PARAM_RET("Only one source ip can be configureed for destination %s",
                inet_ntoa((struct in_addr)p_nat->key.ip));
        }
        else
        {
            inband_snat_delete_tftp_ftp_scp_entry_by_nat(p_nat);     
        }
        /* liwh end */
    }

#if defined (GOLDENGATE) || defined (GREATBELT)
    /* add rule to kernel*/
    sal_sprintf(cmd_buf, INBAND_SNAT_IPTABLES_PREFIX" -t nat -A POSTROUTING -d %s -j SNAT --to-source %s",
                dst_ip, src_ip);

    if (0 != system(cmd_buf) >> 8)
    {
        CFG_INVALID_PARAM_RET("Can't add rule to the iptables");
    }
#endif

#if defined(DUET2)
    sal_sprintf(cmd_buf, "nft add table nat");
    if (0 != system(cmd_buf) >> 8)
    {
        CFG_INVALID_PARAM_RET("Can't add nf_tables nat");
    }

    sal_sprintf(cmd_buf, "nft add chain nat postrouting '{type nat hook postrouting priority 0; policy accept;}'");
    if (0 != system(cmd_buf) >> 8)
    {
        CFG_INVALID_PARAM_RET("Can't add nat postrouting");
    }

    sal_sprintf(cmd_buf, "nft add rule nat postrouting ip daddr %s counter snat %s",
                dst_ip, src_ip);

    if (0 != system(cmd_buf) >> 8)
    {
        CFG_INVALID_PARAM_RET("Can't add nat postrouting entry");
    }
#endif

#if defined (TSINGMA) 
#if defined (TAPPRODUCT) || defined (OFPRODUCT)
    sal_sprintf(cmd_buf, "nft add table nat");
    if (0 != system(cmd_buf) >> 8)
    {
        CFG_INVALID_PARAM_RET("Can't add nf_tables nat");
    }

    sal_sprintf(cmd_buf, "nft add chain nat postrouting '{type nat hook postrouting priority 0; policy accept;}'");
    if (0 != system(cmd_buf) >> 8)
    {
        CFG_INVALID_PARAM_RET("Can't add nat postrouting");
    }

    sal_sprintf(cmd_buf, "nft add rule nat postrouting ip daddr %s counter snat %s",
                dst_ip, src_ip);

    if (0 != system(cmd_buf) >> 8)
    {
        CFG_INVALID_PARAM_RET("Can't add nat postrouting entry");
    }
#else
    p_nat_glb = tbl_nat_global_get_nat_global();
    if (!p_nat_glb)
    {
        CFG_INVALID_PARAM_RET("Can't add nat postrouting entry");    
    }

    p_nat_glb->nft_hdl++;
    nat.nft_handle = p_nat_glb->nft_hdl;
    sal_sprintf(cmd_buf, "nft insert rule nat postrouting handle %u ip daddr %s counter snat to %s",
        p_nat_glb->nft_hdl_upstream_filter_nat_rule,
        dst_ip, src_ip);

    if (0 != system(cmd_buf) >> 8)
    {
        CFG_INVALID_PARAM_RET("Can't add nat postrouting entry");
    }
        
    tbl_nat_global_set_nat_global_field(p_nat_glb, TBL_NAT_GLOBAL_FLD_NFT_HDL);
#endif
#endif

    /* append to db */
    PM_E_RETURN(tbl_inband_snat_add_inband_snat(&nat));

    /* modified by liwh for bug 53499, 2018-09-12 */
    p_nat = tbl_inband_snat_get_inband_snat(&nat.key);
    if (p_nat)
    {
        p_nat->snat_timer = ctc_task_add_timer(TASK_PRI_NORMAL, inband_snat_timer_fn, 
            p_nat, INBAND_SNAT_SOURCE_IP_TIME_INTERVAL);
    }
    /* liwh end */

    return rc;
}

static int32
_inband_snat_del_tftp_ftp_scp_entry(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                      rc = PM_E_NONE;
    tbl_inband_snat_t   nat;
    char                       dst_ip[200] = {0};
    char                       src_ip[16] = {0};
    uint32                    index_i = 0;
    uint32                    index_j = 0;
    uint32                    length = 0;
    bool                        is_addr = FALSE;
    //char                       cmd_buf[200] = {0};
//#if defined(TSINGMA) && !defined (TAPPRODUCT) && !defined (OFPRODUCT)
    tbl_inband_snat_t   *p_nat = NULL;
//#endif

    sal_memset(&nat, 0, sizeof(tbl_inband_snat_t));

    /* get nat src ip */
    sal_strcpy(src_ip, argv[argc -1]);
    
    /* get nat dst ip */
    /* 10.10.38.160 */
    if (sal_strlen(argv[2]) <= 15)
    {
        sal_strcpy(dst_ip, argv[2]);
    }

    /* 10.10.38.160: */
    if ((1 != sal_inet_pton(AF_INET, dst_ip, &nat.key.ip))
        && (argv[2][0] >= '1') && (argv[2][0] <= '9'))
    {
        sal_memset(dst_ip, 0, 200);
        length = sal_strlen(argv[2]);
        for (index_i = 0; index_i < length; index_i++)
        {
            if (argv[2][index_i] == ':')
            {
                break;
            }
            else
            {
                dst_ip[index_j] = argv[2][index_i];
                index_j++;
            }   
        }
    }

    /* @10.10.38.160: */
    if (1 != sal_inet_pton(AF_INET, dst_ip, &nat.key.ip))
    {
        sal_memset(dst_ip, 0, 200);
        length = sal_strlen(argv[2]);
        for (index_i = 0; index_i < length; index_i++)
        {
            if (argv[2][index_i] == '@')
            {
                is_addr = TRUE;   
            }
            else
            {
                if (is_addr)
                {
                    if (argv[2][index_i] == ':')
                    {
                        break;
                    }
                    else
                    {
                        dst_ip[index_j] = argv[2][index_i];
                        index_j++;
                    }
                }
            }
        }
    }

    /* get dst ip and check dst ip */
    if (1 != sal_inet_pton(AF_INET, dst_ip, &nat.key.ip))
    {
        CFG_INVALID_PARAM_RET("Invalid IP address: %s", dst_ip);
    }

    p_nat = tbl_inband_snat_get_inband_snat(&nat.key);
    if (p_nat)
    {
        inband_snat_delete_tftp_ftp_scp_entry_by_nat(p_nat);
    }

    return rc;
}
/* liwh end */

static int32
_inband_snat_add_entry(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    int32 rc = PM_E_NONE;
    tbl_inband_snat_t nat;
    tbl_inband_snat_t *p_nat;
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if = NULL;
    addr_t addr;
    char if_buf[IFNAME_SIZE];
    char addr_buf[CMD_BUF_32];

    /* modified by liwh for bug 53343, 2019-08-28 */
    if ((0 == sal_strcmp(argv[0], "tftp:")) || (0 == sal_strcmp(argv[0], "ftp:"))
        || (0 == sal_strcmp(argv[0], "scp:")))
    {
        rc = _inband_snat_add_tftp_ftp_scp_entry(argv, argc, para, p_sys_glb);
        return rc;    
    }
    /* liwh end */

    /* args must be /$proto#$port#$ip/type/$type/[ifname/$ifname]|[ip/$ip] */

    /* parse args */
    sal_memset(&nat, 0, sizeof(tbl_inband_snat_t));
    PM_E_RETURN(_inband_snat_string_to_key(para, argv[0], &nat.key));
    PM_E_RETURN(_inband_snat_parse_args(argv + 1, argc - 1, para, &nat));

    if (nat.ifname[0])
    {
        /* check interface exist */
        sal_memset(&if_key, 0, sizeof(tbl_interface_key_t));
        sal_strcpy(if_key.name, nat.ifname);
        p_db_if = tbl_interface_get_interface(&if_key);
        if (NULL == p_db_if)
        {
            CFG_INVALID_PARAM_RET("Interface '%s' does not exist", nat.ifname);
        }

        /* get ipaddress */
        rc = _inband_snat_get_ipaddr_by_ifname(nat.ifname, &nat.ip);
        if (rc)
        {
            IFNAME_ETH2FULL(nat.ifname, if_buf);
            CFG_INVALID_PARAM_RET("Can't get '%s' ip address", if_buf);
        }
    }

    if (p_sys_glb->startup_done)
    {
        sal_memset(&addr, 0, sizeof(addr));
        addr.family = AF_INET;
        addr.u.prefix4.s_addr = nat.ip.s_addr;
        if (FALSE == route_if_addr_exists(&addr))
        {
            CFG_CONFLICT_RET("IP address %s not configured", cdb_addr_val2str(addr_buf, CMD_BUF_32, &addr));
        }
    }

    /* check entry exist */
    p_nat = tbl_inband_snat_get_inband_snat(&nat.key);
    if (p_nat)
    {
        CFG_INVALID_PARAM_RET("%s:%s:%hu already assigned to %s",
                               p_nat->key.proto == INBAND_IP_PROTO_TCP ? "tcp" : "udp",
                               inet_ntoa((struct in_addr)p_nat->key.ip),
                               p_nat->key.port,
                               INBAND_SNAT_TYPE_TO_STRING(p_nat->type));
    }


    /* add rule */
    rc = _inband_snat_rule_op(&nat, INBAND_SNAT_RULE_ADD);
    if (rc)
    {
        CFG_INVALID_PARAM_RET("Can't add rule to the iptables");
    }

    /* append to db */
    PM_E_RETURN(tbl_inband_snat_add_inband_snat(&nat));

    /* mark on interface */
    if (nat.ifname[0])
    {
        if (p_db_if)
        {
            p_db_if->inband_snat_type = nat.type;
            PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_INBAND_SNAT_TYPE));
        }
    }

    return rc;
}


static int32
_inband_snat_del_entry(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_inband_snat_key_t key;
    tbl_inband_snat_t *p_nat;
    tbl_interface_key_t if_key;
    tbl_interface_t *p_if;

    /* modified by liwh for bug 53343, 2019-08-28 */
    if ((0 == sal_strcmp(argv[0], "tftp:")) || (0 == sal_strcmp(argv[0], "ftp:"))
        || (0 == sal_strcmp(argv[0], "scp:")))
    {
        rc = _inband_snat_del_tftp_ftp_scp_entry(argv, argc, para);
        return rc;    
    }
    /* liwh end */

    /* args must be /$proto#$port#$ip */
    if (argc != 1)
    {
        CFG_INVALID_PARAM_RET("Format error");
    }

    /* parse args */
    sal_memset(&key, 0, sizeof(tbl_inband_snat_key_t));
    PM_E_RETURN(_inband_snat_string_to_key(para, argv[0], &key));

    /* check entry exist */
    p_nat = tbl_inband_snat_get_inband_snat(&key);
    if (NULL == p_nat)
    {
        return PM_E_NOT_EXIST;
    }

    /* delete rule */
    rc = _inband_snat_rule_op(p_nat, INBAND_SNAT_RULE_DEL);
    if (rc)
    {
        CFG_INVALID_PARAM_RET("Can't delete rule from the iptables");
    }

    /* release interface */
    if (p_nat->ifname[0])
    {
        sal_memset(&if_key, 0, sizeof(tbl_interface_key_t));
        sal_strcpy(if_key.name, p_nat->ifname);
        p_if = tbl_interface_get_interface(&if_key);
        if (NULL == p_if)
        {
            CFG_INVALID_PARAM_RET("Interface '%s' does not exist", p_nat->ifname);
        }
        p_if->inband_snat_type = INBAND_SNAT_TYPE_INVALID;
        PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_INBAND_SNAT_TYPE));
    }

    /* delete from db */
    PM_E_RETURN(tbl_inband_snat_del_inband_snat(&key));

    return rc;
}


static int32
_inband_snat_get_entry(char **argv, int32 argc, cfg_cmd_para_t *para, field_parse_t *p_field)
{
    int32 rc = PM_E_NONE;
    tbl_inband_snat_key_t key;
    tbl_inband_snat_t *p_nat;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    /* args must be /$proto#$port#$ip */
    if (argc != 1)
    {
        CFG_INVALID_PARAM_RET("Format error");
    }

    /* parse args */
    sal_memset(&key, 0, sizeof(tbl_inband_snat_key_t));
    PM_E_RETURN(_inband_snat_string_to_key(para, argv[0], &key));

    /* check entry exist */
    p_nat = tbl_inband_snat_get_inband_snat(&key);
    if (NULL == p_nat)
    {
        return PM_E_NOT_EXIST;
    }

    /* dump entry */
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_inband_snat_dump_one(p_nat, &args);

    sal_fclose(fp);

    return rc;
}


int32
inband_snat_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    field_parse_t field;
    
    CFG_DBG_FUNC();

    switch (para->oper)
    {
        case CDB_OPER_GET:
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            if (0 == argc)
            {
                PM_E_RETURN(_inband_snat_dump_all(para, &field));
            }
            else
            {
                _inband_snat_get_entry(argv, argc, para, &field);
            }
            break;

        case CDB_OPER_ADD:
            PM_E_RETURN(_inband_snat_add_entry(argv, argc, para));
            break;

        case CDB_OPER_DEL:
            PM_E_RETURN(_inband_snat_del_entry(argv, argc, para));
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}


int32
inband_snat_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_INBAND_SNAT, inband_snat_cmd_process);

    return PM_E_NONE;
}


int32
inband_snat_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_INBAND_SNAT, NULL);

    return PM_E_NONE;
}

/* modified by liwh for bug 44498,44500, 2017-06-27 */
int32
inband_snat_del_if_or_ip(const addr_ipv4_t *ip, uint16_t port, inband_snat_proto_t proto)
{
    tbl_inband_snat_key_t key;
    tbl_inband_snat_t *p_nat;
    int32 rc = PM_E_NONE;
    tbl_interface_key_t if_key;
    tbl_interface_t *p_if;

    sal_memset(&key, 0, sizeof(tbl_inband_snat_key_t));
    sal_memcpy(&key.ip, ip, sizeof(addr_ipv4_t));
    key.port = port;
    key.proto = proto;
    p_nat = tbl_inband_snat_get_inband_snat(&key);
    if (NULL == p_nat)
    {
        return PM_E_NOT_EXIST;
    }

    /* delete rule */
    rc = _inband_snat_rule_op(p_nat, INBAND_SNAT_RULE_DEL);
    if (rc)
    {
        return PM_E_INVALID_PARAM;
    }

    /* release interface */
    if (p_nat->ifname[0])
    {
        sal_memset(&if_key, 0, sizeof(tbl_interface_key_t));
        sal_strcpy(if_key.name, p_nat->ifname);
        p_if = tbl_interface_get_interface(&if_key);
        if (NULL == p_if)
        {
            return PM_E_INVALID_PARAM;
        }
        p_if->inband_snat_type = INBAND_SNAT_TYPE_INVALID;
        PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_INBAND_SNAT_TYPE));
    }

    /* delete from db */
    PM_E_RETURN(tbl_inband_snat_del_inband_snat(&key));

    return PM_E_NONE;
}
/* liwh end */

