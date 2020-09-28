
#include "proto.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_manage_if_define.h"
#include "gen/tbl_manage_if.h"
#include "gen/tbl_mirror_define.h"
#include "gen/tbl_cpu_mirror_global_define.h"
#include "gen/tbl_cpu_mirror_global.h"
#include "gen/tbl_mirror.h"
#include "gen/tbl_mirror_mac_escape_define.h"
#include "gen/tbl_mirror_mac_escape.h"
#include "gen/tbl_tap_group_define.h"
#include "gen/tbl_tap_group.h"
#include "gen/tbl_card_define.h"
#include "gen/tbl_card.h"
#include "gen/tbl_port_define.h"
#include "gen/tbl_port.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_log_global_define.h"
#include "gen/tbl_log_global.h"
#include "gen/tbl_ns_port_forwarding_define.h"
#include "gen/tbl_ns_port_forwarding.h"
#include "gen/tbl_ns_route_define.h"
#include "gen/tbl_ns_route.h"
#include "gen/tbl_errdisable_define.h"
#include "gen/tbl_errdisable.h"
#include "gen/tbl_errdisable_flap_define.h"
#include "gen/tbl_errdisable_flap.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
#include "gen/tbl_fea_time_define.h"
#include "gen/tbl_fea_time.h"
#include "gen/tbl_bhm_global_define.h"
#include "gen/tbl_bhm_global.h"
#include "gen/tbl_inband_snat_define.h"
#include "gen/tbl_inband_snat.h"
#include "gen/tbl_ssh_cfg_define.h"
#include "gen/tbl_ssh_cfg.h"
#include "gen/tbl_ssm_global_define.h"
#include "gen/tbl_ssm_global.h"
#include "gen/tbl_controller_attr_define.h"
#include "gen/tbl_controller_attr.h"
#include "gen/tbl_mem_summary_define.h"
#include "gen/tbl_mem_summary.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "lib_fs.h"
#ifdef OFDPAPRODUCT
#include "gen/tbl_controller_define.h"
#endif

int32
inband_snat_get_if_or_ip(const addr_ipv4_t *ip, uint16_t port, inband_snat_proto_t proto, char *buf)
{
    tbl_inband_snat_key_t key;
    tbl_inband_snat_t *p_nat;

    *buf = '\0';
    sal_memset(&key, 0, sizeof(tbl_inband_snat_key_t));
    sal_memcpy(&key.ip, ip, sizeof(addr_ipv4_t));
    key.port = port;
    key.proto = proto;
    p_nat = tbl_inband_snat_get_inband_snat(&key);
    if (p_nat)
    {
        if (p_nat->ifname[0])
        {
            IFNAME_ETH2FULL(p_nat->ifname, buf);
        }
        else
        {
            sal_inet_ntop(AF_INET, &p_nat->ip, buf, 16);
        }
    }

    return p_nat ? PM_E_NONE : PM_E_NOT_EXIST;
}

int32
ccs_cfg_build_banner(FILE* fp, cfg_cmd_para_t *para)
{
    FILE *fp_banner = NULL;
    char buf[512];
    uint32 has_config = FALSE;
    
    fp_banner = sal_fopen(BANNER_MOTD_FILE, SAL_OPEN_RO);
    if (fp_banner)
    {
        sal_fprintf(fp, "banner motd %s\n", BANNER_KEY_WORD);
        while (sal_fgets(buf, 512, fp_banner))
        {
            sal_fprintf(fp, "%s", buf);
        }
        sal_fprintf(fp, "%s\n", BANNER_KEY_WORD);
        sal_fclose(fp_banner);
    }

    fp_banner = sal_fopen(BANNER_EXEC_FILE, SAL_OPEN_RO);
    if (fp_banner)
    {
        sal_fprintf(fp, "banner exec %s\n", BANNER_KEY_WORD);
        while (sal_fgets(buf, 512, fp_banner))
        {
            sal_fprintf(fp, "%s", buf);
        }
        sal_fprintf(fp, "%s\n", BANNER_KEY_WORD);
        has_config = TRUE;
        sal_fclose(fp_banner);
    }
    
    fp_banner = sal_fopen(BANNER_LOGIN_FILE, SAL_OPEN_RO);
    if (fp_banner)
    {
        sal_fprintf(fp, "banner login %s\n", BANNER_KEY_WORD);
        while (sal_fgets(buf, 512, fp_banner))
        {
            sal_fprintf(fp, "%s", buf);
        }
        sal_fprintf(fp, "%s\n", BANNER_KEY_WORD);
        has_config = TRUE;
        sal_fclose(fp_banner);
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_sys_global(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    //char rpcapi_str[CMD_BUF_64];
    //char tmp_str[CMD_BUF_64];
    uint32 has_config = FALSE;
#ifndef TAPPRODUCT 
    
#else
    mac_addr_t zero_mac;
    char macda_str[64];
    char macsa_str[64];
#endif

#ifndef TAPPRODUCT    
    if (p_db_glb->service_password_encryption)
    {
        sal_fprintf(fp, "service password-encryption\n");
         /* add by wus for bug52967 modify password "admin" in 2019-07-15*/
        //sal_fprintf(fp, "service password-encryption aes\n");
        has_config = TRUE;
    }
#endif
    if (sal_strlen(p_db_glb->hostname))
    {
        if (sal_strcmp("Switch", p_db_glb->hostname))
        {
            sal_fprintf(fp, "hostname %s\n", p_db_glb->hostname);
            sal_fprintf(fp, "!\n");
            has_config = TRUE;
        }
    }

    ccs_cfg_build_banner(fp, para);
    
#ifndef TAPPRODUCT 
    if (p_db_glb->ipg_shaping_enable)
    {
        sal_fprintf(fp, "ipg shaping enable\n");
        has_config = TRUE;
    }

    if (p_db_glb->ipg_policing_enable)
    {
        sal_fprintf(fp, "ipg policer enable\n");
        has_config = TRUE;
    }

    if (p_db_glb->ipg_storm_control_enable)
    {
        sal_fprintf(fp, "ipg storm-control enable\n");
        has_config = TRUE;
    }
#endif
    /*added by yejl to fix bug41142, 2016-12-12*/
    if (p_db_glb->aaa_new_model)
    {
        sal_fprintf(fp, "aaa new-model\n");
        if ((CISCO_PRIVILEGE_MIN != p_db_glb->aaa_privilege1)
        || (CISCO_PRIVILEGE_TWO != p_db_glb->aaa_privilege2)
        || (CISCO_PRIVILEGE_THREE != p_db_glb->aaa_privilege3))
        {
            sal_fprintf(fp, "aaa privilege mapping %d %d %d\n", p_db_glb->aaa_privilege1, p_db_glb->aaa_privilege2, 
                p_db_glb->aaa_privilege3);
        }
        sal_fprintf(fp, "!\n");
        has_config = TRUE;
    }

#ifndef TAPPRODUCT
    if (p_db_glb->dhcp_service_enable)
    {
        sal_fprintf(fp, "service dhcp enable\n");
        has_config = TRUE;
    }

    if (p_db_glb->ptf_port)
    {
        sal_fprintf(fp, "ptf enable %u\n", p_db_glb->ptf_port);
        has_config = TRUE;
    }
#endif
    if (p_db_glb->telnet_disable)
    {
        sal_fprintf(fp, "service telnet disable\n");
        has_config = TRUE;
    }
    if (p_db_glb->telnet_port != GLB_TELNET_PORT_DEF)
    {
        sal_fprintf(fp, "service telnet port %d\n", p_db_glb->telnet_port);
        has_config = TRUE;
    }
#ifdef OFPRODUCT
    if (!p_db_glb->http_disable)
    {
        sal_fprintf(fp, "service http enable\n");
        has_config = TRUE;
    }
#else
    if (p_db_glb->http_disable)
    {
        sal_fprintf(fp, "service http disable\n");
        has_config = TRUE;
    }
#endif
    if (p_db_glb->http_port != GLB_HTTP_PORT_DEF)
    {
        sal_fprintf(fp, "service http port %d\n", p_db_glb->http_port);
        has_config = TRUE;
    }
    if (p_db_glb->http_timeout != GLB_HTTP_TIMEOUT_DEF)
    {
        sal_fprintf(fp, "service http timeout %d\n", p_db_glb->http_timeout/60);
        has_config = TRUE;
    }
    /* the order should be careful */
    if (p_db_glb->https_port != GLB_HTTPS_PORT_DEF)
    {
        sal_fprintf(fp, "service https port %d\n", p_db_glb->https_port);
        has_config = TRUE;
    }
    if (!p_db_glb->https_disable)
    {
        sal_fprintf(fp, "service https enable\n");
        has_config = TRUE;
    }

    /* bug53609 move service rpc cfg after user cfg */
#if 0    
    if (p_db_glb->rpcapi_auth_mode != GLB_RPCAPI_AUTH_NONE)
    {
        sal_fprintf(fp, "service rpc-api auth-mode basic\n");
        has_config = TRUE;
    }
    if (!p_db_glb->rpcapi_disable)
    {
        sal_memset(rpcapi_str, 0, sizeof(rpcapi_str));
        sal_memset(tmp_str, 0, sizeof(tmp_str));
        if (p_db_glb->rpcapi_port != GLB_HTTP_PORT_DEF)
        {
            sal_snprintf(tmp_str, CMD_BUF_64, " port %u", p_db_glb->rpcapi_port);
            sal_strcat(rpcapi_str, tmp_str);
        }
        if (p_db_glb->rpcapi_ssl_en)
        {
            if (p_db_glb->rpcapi_ssl_port != GLB_HTTPS_PORT_DEF)
            {
                sal_snprintf(tmp_str, CMD_BUF_64, " ssl ssl-port %u", p_db_glb->rpcapi_ssl_port);
            }
            else
            {
                sal_snprintf(tmp_str, CMD_BUF_64, " ssl");
            }
            sal_strcat(rpcapi_str, tmp_str);
        }
        sal_fprintf(fp, "service rpc-api enable%s\n", rpcapi_str);
        has_config = TRUE;
    }
#endif

#ifndef TAPPRODUCT
    if (p_db_glb->cut_through_enable)
    {
        if (GLB_CHIP_GOLDENGATE == tbl_chip_get_type())
        {
            sal_fprintf(fp, "cut-through-forwarding enable");
            if (CUT_THROUGH_FORWARDING_SPEED_1G_10G_100G == p_db_glb->cut_through_enable)
            {
                sal_fprintf(fp, " 1G-10G-100G");
            }
            else if (CUT_THROUGH_FORWARDING_SPEED_1G_10G_40G == p_db_glb->cut_through_enable)
            {
                sal_fprintf(fp, " 1G-10G-40G");
            }
            sal_fprintf(fp, "\n");
        }
        else
        {
            /* USW system using bitmap */
            sal_fprintf(fp, "cut-through-forwarding enable\n");
        }

        has_config = TRUE;
    }
#else
    if (p_db_glb->cut_through_enable)
    {
        sal_fprintf(fp, "cut-through-forwarding enable\n");
        has_config = TRUE;
    }    
    sal_memset(zero_mac, 0, sizeof(mac_addr_t));
    sal_memset(macda_str, 0, 64);
    sal_memset(macsa_str, 0, 64);
    
    if ((0 != sal_memcmp(p_db_glb->tap_ts_macda, zero_mac, sizeof(mac_addr_t)))
        || (0 != sal_memcmp(p_db_glb->tap_ts_macsa, zero_mac, sizeof(mac_addr_t)))
        || (0 != p_db_glb->ether_type))
    {
        cdb_mac_val2str(macda_str, sizeof(macda_str), p_db_glb->tap_ts_macda);
        cdb_mac_val2str(macsa_str, sizeof(macsa_str), p_db_glb->tap_ts_macsa);
        sal_fprintf(fp, "timestamp-over-ether %s %s 0x%x\n",
            macda_str, macsa_str, p_db_glb->ether_type);
        sal_fprintf(fp, "!\n");
    }

    if (p_db_glb->trunction_length != 0 
		&& p_db_glb->trunction_length != config_get_int32(CONFIG_FEATURE_TAP_TRUNCTION_LENGTH_MAX, 144))
    {
        sal_fprintf(fp, "truncation %d\n", p_db_glb->trunction_length);
        sal_fprintf(fp, "!\n");
    }
#endif

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_mem_summary(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_mem_summary_t* p_db_mem_info = NULL;
    p_db_mem_info = tbl_mem_summary_get_mem_summary();
    if (NULL == p_db_mem_info)
    {
        return PM_E_INVALID_PARAM;
    }
    if ((p_db_mem_info->cpu_low_threshold!=CPU_LOW_DEFULT_THRESHOLD)||(p_db_mem_info->cpu_high_threshold!=CPU_HIGH_DEFULT_THRESHOLD))
    {
        sal_fprintf(fp, "cpu-threshold %d %d\n", 
                         p_db_mem_info->cpu_low_threshold,
                         p_db_mem_info->cpu_high_threshold);
    }
    return PM_E_NONE;
}

#ifdef OFDPAPRODUCT
int32
ccs_cfg_build_controller(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_controller_master_t *p_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_controller_t *p_db_controller = NULL;
    int32 rc = PM_E_NONE;
    uint32 has_config = FALSE;
    char buf[MAX_CMD_STR_LEN];
    
    p_master = tbl_controller_get_master();

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->controller_list, p_db_controller, listnode, next)
    {
        sal_fprintf(fp, "controller add id %s addr %s port %d\n",
                p_db_controller->key.name,
                cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_db_controller->ip),
                p_db_controller->port);
        has_config = TRUE;
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}
#endif

int32
ccs_cfg_build_sys_manage_if(FILE* fp, cfg_cmd_para_t *para)
{
    char buf[CMD_BUF_256];
    tbl_manage_if_t *p_db_mgtif = tbl_manage_if_get_manage_if();
    uint32 has_config = FALSE;
    addr_ipv6_t addr_ipv6_tmp;
    
    sal_memset(&addr_ipv6_tmp, 0, sizeof(addr_ipv6_tmp));

    if (TRUE == p_db_mgtif->dhclient_enable)
    {
        sal_fprintf(fp, "management ip address dhcp\n");
        has_config = TRUE;
    }
    else
    {
        if (p_db_mgtif->addr.prefix.s_addr)
        {
            sal_fprintf(fp, "management ip address %s/%u\n", 
                cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_db_mgtif->addr.prefix),
                p_db_mgtif->addr.prefixlen);
            has_config = TRUE;
        }

        if (p_db_mgtif->gateway.s_addr)
        {
            sal_fprintf(fp, "management route add gateway %s\n", 
                cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_db_mgtif->gateway));
            has_config = TRUE;
        }
    }

    if (p_db_mgtif->dhclientv6_enable)
    {
        sal_fprintf(fp, "management ipv6 address dhcp\n");
        has_config = TRUE;
    }
    else
    {
        if (p_db_mgtif->addr_v6.prefixlen)
        {
            sal_fprintf(fp, "management ipv6 address %s\n", 
                prefix_val2str(buf, MAX_CMD_STR_LEN, &p_db_mgtif->addr_v6, TRUE));
            has_config = TRUE;
        }

        if (0 != sal_memcmp(&p_db_mgtif->gateway_v6.u.prefix6, &addr_ipv6_tmp, sizeof(addr_ipv6_tmp)))
        {
            sal_fprintf(fp, "management ipv6 route add gateway %s\n", 
                cdb_addr_ipv6_val2str(buf, MAX_CMD_STR_LEN, &p_db_mgtif->gateway_v6.u.prefix6));
            has_config = TRUE;
        }
    }
    
    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_sys_mirror_one(FILE* fp, tbl_mirror_t *p_db_mirror)
{
    char if_str[IFNAME_SIZE+1];
    char ifname_ext[IFNAME_SIZE+1];
    uint32 ifindex = 0;
    uint32 vid = 0;
    int32 rc = PM_E_NONE;
    tbl_interface_t* p_db_if = NULL;
    
    if (MIRROR_DEST_PORT == p_db_mirror->dest_type)
    {
        rc = tbl_interface_get_name_by_ifindex(p_db_mirror->dest_port, if_str, IFNAME_SIZE);
        if (rc)
        {
             return PM_E_NONE;
        }
        IFNAME_ETH2SHORT(if_str, ifname_ext);
        sal_fprintf(fp, "monitor session %u destination interface %s\n", p_db_mirror->key.id, ifname_ext);
    }
    else if (MIRROR_DEST_CPU == p_db_mirror->dest_type)
    {
        sal_fprintf(fp, "monitor session %u destination cpu\n", p_db_mirror->key.id);
    }
    else if (MIRROR_DEST_VLAN == p_db_mirror->dest_type)
    {
        rc = tbl_interface_get_name_by_ifindex(p_db_mirror->dest_port, if_str, IFNAME_SIZE);
        if (rc)
        {
             return PM_E_NONE;
        }
        IFNAME_ETH2SHORT(if_str, ifname_ext);
        sal_fprintf(fp, "monitor session %u destination remote vlan %u interface %s\n",
            p_db_mirror->key.id, p_db_mirror->dest_vlan, ifname_ext);
    }
    else if (MIRROR_DEST_GROUP == p_db_mirror->dest_type)
    {
        GLB_BMP_ITER_BEGIN(p_db_mirror->dest_port_bmp, ifindex)
        {
            rc = tbl_interface_get_name_by_ifindex(ifindex, if_str, IFNAME_SIZE);
            if (rc)
            {
                continue;
            }
            IFNAME_ETH2SHORT(if_str, ifname_ext);
            sal_fprintf(fp, "monitor session %u destination group %u member %s\n", 
                p_db_mirror->key.id, p_db_mirror->dest_group, ifname_ext);
        }
        GLB_BMP_ITER_END(p_db_mirror->dest_port_bmp, ifindex);
    }
    
    GLB_BMP_ITER_BEGIN(p_db_mirror->source_igs_port_bmp, ifindex)
    {
        if (!GLB_BMP_ISSET(p_db_mirror->source_egs_port_bmp, ifindex))
        {
            rc = tbl_interface_get_name_by_ifindex(ifindex, if_str, IFNAME_SIZE);
            if (rc)
            {
                continue;
            }
            p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
            if (NULL == p_db_if)
            {
                return PM_E_INVALID_PARAM;
            }
            if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type && (NULL == p_db_if->lag && NULL == p_db_if->lacp))
            {
                IFNAME_ETH2SHORT(if_str, ifname_ext);
                sal_fprintf(fp, "monitor session %u source interface %s rx\n", p_db_mirror->key.id, ifname_ext);
            }
            else if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type && (NULL != p_db_if->lag || NULL != p_db_if->lacp))
            {
                IFNAME_ETH2SHORT(if_str, ifname_ext);
                sal_fprintf(fp, "monitor session %u source interface %s rx\n", p_db_mirror->key.id, ifname_ext);
            }
        }
    }
    GLB_BMP_ITER_END(p_db_mirror->source_igs_port_bmp, ifindex);

    GLB_BMP_ITER_BEGIN(p_db_mirror->source_egs_port_bmp, ifindex)
    {
        if (!GLB_BMP_ISSET(p_db_mirror->source_igs_port_bmp, ifindex))
        {
            rc = tbl_interface_get_name_by_ifindex(ifindex, if_str, IFNAME_SIZE);
            if (rc)
            {
                continue;
            }
            p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
            if (NULL == p_db_if)
            {
                return PM_E_INVALID_PARAM;
            }
            if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type && (NULL == p_db_if->lag && NULL == p_db_if->lacp))
            {
                IFNAME_ETH2SHORT(if_str, ifname_ext);
                sal_fprintf(fp, "monitor session %u source interface %s tx\n", p_db_mirror->key.id, ifname_ext);
            }
            else if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type && (NULL != p_db_if->lag || NULL != p_db_if->lacp))
            {
                IFNAME_ETH2SHORT(if_str, ifname_ext);
                sal_fprintf(fp, "monitor session %u source interface %s tx\n", p_db_mirror->key.id, ifname_ext);
            }
        }
    }
    GLB_BMP_ITER_END(p_db_mirror->source_egs_port_bmp, ifindex);

    GLB_BMP_ITER_BEGIN(p_db_mirror->source_egs_port_bmp, ifindex)
    {
        if (GLB_BMP_ISSET(p_db_mirror->source_igs_port_bmp, ifindex))
        {
            rc = tbl_interface_get_name_by_ifindex(ifindex, if_str, IFNAME_SIZE);
            if (rc)
            {
                continue;
            }
            p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
            if (NULL == p_db_if)
            {
                return PM_E_INVALID_PARAM;
            }
            if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type && (NULL == p_db_if->lag && NULL == p_db_if->lacp))
            {
                IFNAME_ETH2SHORT(if_str, ifname_ext);
                sal_fprintf(fp, "monitor session %u source interface %s both\n", p_db_mirror->key.id, ifname_ext);
            }
            else if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type && (NULL != p_db_if->lag || NULL != p_db_if->lacp))
            {
                IFNAME_ETH2SHORT(if_str, ifname_ext);
                sal_fprintf(fp, "monitor session %u source interface %s both\n", p_db_mirror->key.id, ifname_ext);
            }
        }
    }
    GLB_BMP_ITER_END(p_db_mirror->source_egs_port_bmp, ifindex);


    GLB_BMP_ITER_BEGIN(p_db_mirror->source_igs_vlan_bmp, vid)
    {
        if (!GLB_BMP_ISSET(p_db_mirror->source_egs_vlan_bmp, vid))
        {
            sal_fprintf(fp, "monitor session %u source vlan %u rx\n", p_db_mirror->key.id, vid);
        }
    }
    GLB_BMP_ITER_END(p_db_mirror->source_igs_vlan_bmp, vid);

    GLB_BMP_ITER_BEGIN(p_db_mirror->source_egs_vlan_bmp, vid)
    {
        if (!GLB_BMP_ISSET(p_db_mirror->source_igs_vlan_bmp, vid))
        {
            sal_fprintf(fp, "monitor session %u source vlan %u tx\n", p_db_mirror->key.id, vid);
        }
    }
    GLB_BMP_ITER_END(p_db_mirror->source_egs_vlan_bmp, vid);

    GLB_BMP_ITER_BEGIN(p_db_mirror->source_egs_vlan_bmp, vid)
    {
        if (GLB_BMP_ISSET(p_db_mirror->source_igs_vlan_bmp, vid))
        {
            sal_fprintf(fp, "monitor session %u source vlan %u both\n", p_db_mirror->key.id, vid);
        }
    }
    GLB_BMP_ITER_END(p_db_mirror->source_egs_vlan_bmp, vid);

    if (p_db_mirror->cpu_source_dir == GLB_MIRROR_DIR_BOTH)
    {
        sal_fprintf(fp, "monitor session %u source cpu both\n", p_db_mirror->key.id);
    }
    else
    {
        if (GLB_FLAG_ISSET(p_db_mirror->cpu_source_dir, GLB_MIRROR_DIR_IGS))
        {
            sal_fprintf(fp, "monitor session %u source cpu rx\n", p_db_mirror->key.id);
        }
        
        if (GLB_FLAG_ISSET(p_db_mirror->cpu_source_dir, GLB_MIRROR_DIR_EGS))
        {
            sal_fprintf(fp, "monitor session %u source cpu tx\n", p_db_mirror->key.id);
        }
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_cpu_mirror_global(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_cpu_mirror_global_t *p_cpu_mirror = tbl_cpu_mirror_global_get_cpu_mirror_global();
    uint32 has_config = FALSE;
    
    if (GLB_CPU_MIRROR_STRATEGY_REPLACE != p_cpu_mirror->strategy)
    {
        sal_fprintf(fp, "monitor cpu capture strategy drop\n");
        has_config = TRUE;
    }
    if (GLB_CPU_MIRROR_BUF_SIZE_DEF != p_cpu_mirror->buf_size)
    {
        sal_fprintf(fp, "monitor cpu set packet buffer %u\n", p_cpu_mirror->buf_size);
        has_config = TRUE;
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_sys_mirror(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    uint32 has_config = FALSE;
    uint32 sessionid = 0;

    for (sessionid = 1; sessionid <= GLB_MIRROR_MAX_SESSION_ID; sessionid++)
    {
        sal_memset(&mirror_key, 0, sizeof(tbl_mirror_key_t));
        mirror_key.id = sessionid;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            continue;
        }
        _ccs_cfg_build_sys_mirror_one(fp, p_db_mirror);
        has_config = TRUE;
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_sys_mirror_mac_escape(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_mirror_mac_escape_master_t* p_master = tbl_mirror_mac_escape_get_master();
    tbl_mirror_mac_escape_t *p_db_mac_escape = NULL;
    ctclib_slistnode_t *listnode = NULL;
    const uint8 *mac = NULL;
    const uint8 *mask = NULL;
    uint32 has_config = FALSE;

    CTCLIB_SLIST_LOOP(p_master->mirror_mac_escape_list, p_db_mac_escape, listnode)
    {
        mac = p_db_mac_escape->key.mac;
        mask = p_db_mac_escape->key.mask;
        sal_fprintf(fp, "monitor mac escape %.04hX.%.04hX.%.04hX %.04hX.%.04hX.%.04hX\n",
            sal_ntoh16(((uint16*)mac)[0]),
            sal_ntoh16(((uint16*)mac)[1]), 
            sal_ntoh16(((uint16*)mac)[2]),
            sal_ntoh16(((uint16*)mask)[0]),
            sal_ntoh16(((uint16*)mask)[1]), 
            sal_ntoh16(((uint16*)mask)[2]));
        has_config = TRUE;
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

static char tmpstr1_tap[64];
static char tmpstr2_flow[64];
static char tmpstr3_interface[64];

int
_ccs_tap_get_tap_flow_interface_by_class(char *class_name)
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


int32
_ccs_cfg_build_sys_tap(tbl_tap_group_t *tap_group, tbl_iter_args_t *pargs)
{
    FILE* fp = NULL;
#if 0
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_key_t if_key;
    uint32 ifindex = 0;
    char if_str[IFNAME_SIZE];
    char *flow_name = NULL;
    uint32 vlan_id = 0;
    int rc = 0;
#endif

    fp = pargs->argv[0];
    if (NULL == fp)
    {
        return PM_E_NONE;
    }
#if 0 /* by xuwj */
    sal_fprintf(fp, "tap-group %s", tap_group->key.name);
    if (tap_group->truncation)
    {
        sal_fprintf(fp, " truncation");
    }

    sal_fprintf(fp, "\n");
    if (0 != sal_strlen(tap_group->desc))
    {
        sal_fprintf(fp, " description %s\n", tap_group->desc);
    }

    GLB_BMP_ITER_BEGIN(tap_group->igs_port_bmp, ifindex)
    {
        vlan_id = 0;
        rc = tbl_interface_get_name_by_ifindex(ifindex, if_str, IFNAME_SIZE);
        if (rc)
        {
            continue;
        }

        sal_strncpy(if_key.name, if_str, IFNAME_SIZE);
        p_db_if = tbl_interface_get_interface(&if_key);
        if (NULL == p_db_if)
        {
            continue;
        }

        flow_name = _ccs_tap_get_flow_by_tap_interface(tap_group->key.name, if_str, &vlan_id);
        if (NULL != flow_name)
        {
            sal_fprintf(fp, " ingress %s flow %s", if_get_port_shortname_name(if_str), flow_name);
        }
        else
        {
            if (GLB_BMP_ISSET(tap_group->igs_port_mark_vlan_bmp, ifindex))
            {
                vlan_id = p_db_if->tap_mark_vlan;
            }

            sal_fprintf(fp, " ingress %s", if_get_port_shortname_name(if_str));
        }

        if (0 != vlan_id)
        {
            sal_fprintf(fp, " mark-source %d\n", vlan_id);
        }
        else
        {
            sal_fprintf(fp, "\n");
        }
    }
    GLB_BMP_ITER_END(tap_group->igs_port_bmp, ifindex);

    GLB_BMP_ITER_BEGIN(tap_group->egs_port_bmp, ifindex)
    {
        rc = tbl_interface_get_name_by_ifindex(ifindex, if_str, IFNAME_SIZE);
        if (rc)
        {
            continue;
        }

        sal_fprintf(fp, " egress %s", if_get_port_shortname_name(if_str));

        if (GLB_BMP_ISSET(tap_group->egs_port_timestamp_bmp, ifindex))
        {
            sal_fprintf(fp, " timestamp");
        }

        sal_fprintf(fp, "\n");
    }
    
    GLB_BMP_ITER_END(tap_group->egs_port_bmp, ifindex);
#endif
    sal_fprintf(fp, "!\n");
    return PM_E_NONE;
}

int32
ccs_cfg_build_sys_tap(FILE* fp, char *key, cfg_cmd_para_t *para)
{
#if 0
    tbl_sys_global_t* tbl_sys_global = NULL;
    tbl_tap_group_t* pt_tap_group = NULL;
    tbl_tap_group_key_t tap_group_key;
    tbl_iter_args_t args;
    mac_addr_t zero_mac;
    char macda_str[64];
    char macsa_str[64];

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = fp;

    tbl_sys_global = tbl_sys_global_get_sys_global();
    if (NULL == tbl_sys_global)
    {
        return PM_E_NONE;
    }

    sal_memset(zero_mac, 0, sizeof(mac_addr_t));
    sal_memset(macda_str, 0, 64);
    sal_memset(macsa_str, 0, 64);
    if ((0 != sal_memcmp(tbl_sys_global->tap_ts_macda, zero_mac, sizeof(mac_addr_t)))
        || (0 != sal_memcmp(tbl_sys_global->tap_ts_macsa, zero_mac, sizeof(mac_addr_t)))
        || (0 != tbl_sys_global->ether_type))
    {
        cdb_mac_val2str(macda_str, 64, tbl_sys_global->tap_ts_macda);
        cdb_mac_val2str(macsa_str, 64, tbl_sys_global->tap_ts_macsa);
        sal_fprintf(fp, "timestamp-over-ether %s %s 0x%x\n",
            macda_str, macsa_str, tbl_sys_global->ether_type);
        sal_fprintf(fp, "!\n");
    }

    if (tbl_sys_global->trunction_length != 0)
    {
        sal_fprintf(fp, "truncation %d\n", tbl_sys_global->trunction_length);
        sal_fprintf(fp, "!\n");
    }

    if (key)
    {
        sal_memset(&tap_group_key, 0, sizeof(tap_group_key));
        sal_strncpy(tap_group_key.name, key, TAP_NAME_SIZE);
        pt_tap_group= tbl_tap_group_get_tap_group(&tap_group_key);
        if (NULL != pt_tap_group)
        {
            _ccs_cfg_build_sys_tap(pt_tap_group, &args);
        }
    }
    else
    {
        tbl_tap_group_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_sys_tap, &args);
    }
#endif
    return PM_E_NONE;
}

int32
ccs_cfg_build_sys_card(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_card_t *p_db_card = NULL;
    uint32 has_config = FALSE;
    
    p_db_card = tbl_card_get_default_card();
    if (NULL == p_db_card)
    {
        return PM_E_NONE;
    }

    if ((DEFAULT_LOW_TMPR != p_db_card->tmpr_low) 
        || (DEFAULT_HIGH_TMPR != p_db_card->tmpr_high)
        || (DEFAULT_CRIT_TMPR != p_db_card->tmpr_critical))
    {
        sal_fprintf(fp, "temperature %d %d %d\n", 
            p_db_card->tmpr_low, 
            p_db_card->tmpr_high,
            p_db_card->tmpr_critical);
        has_config = TRUE;
    }

    if ((DEFAULT_FAN_RATE_BOTTOM != p_db_card->fan_speed_bottom) 
        || (DEFAULT_FAN_RATE_LOW != p_db_card->fan_speed_low) 
        || (DEFAULT_FAN_RATE_HIGH != p_db_card->fan_speed_high)
        || (DEFAULT_FAN_RATE_FULL != p_db_card->fan_speed_full) )
    {
        sal_fprintf(fp, "fan %d %d %d %d\n", 
            p_db_card->fan_speed_bottom, 
            p_db_card->fan_speed_low,
            p_db_card->fan_speed_high,
            p_db_card->fan_speed_full);
        has_config = TRUE;
    }    

    if (E_DIAG_LEVEL_MINIMAL == p_db_card->next_bootup_diag_level)
    {
        sal_fprintf(fp, "diagnostic bootup level minimal\n");
        has_config = TRUE;
    }
    else if (E_DIAG_LEVEL_COMPLETE == p_db_card->next_bootup_diag_level)
    {
        sal_fprintf(fp, "diagnostic bootup level complete\n");
        has_config = TRUE;
    }

    if (DEFAULT_POE_ENALBE != p_db_card->glb_poe_enable)
    {
        sal_fprintf(fp, "poe global disable\n");
        has_config = TRUE;
    }

    if (DEFAULT_POE_POWER_MAX != p_db_card->glb_poe_power_max)
    {
        sal_fprintf(fp, "poe global max-power %d\n",p_db_card->glb_poe_power_max);
        has_config = TRUE;
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_sys_port(FILE* fp, cfg_cmd_para_t *para)
{
    char ifname[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    tbl_port_master_t *p_master = tbl_port_get_master();
    tbl_port_t *p_db_port = NULL;
    uint32 port_idx = 0;
    uint32 idx = 0;
    uint32 ifname_len = 0;
    uint32 has_config = FALSE;
    uint8 bypass_port_cnt[MAX_BYPASS_NUM] = {0};
    char bypass_ifname1[MAX_BYPASS_NUM][GLB_PHY_PORT_FULL_NAME_SZ];
    char bypass_ifname2[MAX_BYPASS_NUM][GLB_PHY_PORT_FULL_NAME_SZ];
    
    for (port_idx = 0; port_idx < GLB_CARD_PORT_NUM_MAX; port_idx++)
    {
        p_db_port = p_master->port_array[port_idx];
        if (NULL == p_db_port)
        {
            continue;
        }

        if (GLB_PPT_NUMCHG_TYPE_NONE != p_db_port->split_type)
        {
            /* strip subport_no */
            if_build_port_fullname_by_slot_port(ifname, p_db_port->slot_no, p_db_port->panel_port_no, p_db_port->panel_sub_port_no);
            if (p_db_port->panel_sub_port_no)
            {
                ifname_len = sal_strlen(ifname);
                if ((ifname_len > 2) && (('/' == ifname[ifname_len-2]) || (':' == ifname[ifname_len-2])))
                {
                    ifname[ifname_len-2] = '\0';
                }
            }
            sal_fprintf(fp, "split interface %s %s\n", 
                ifname, cdb_enum_val2str(glb_ppt_numchg_type_strs, GLB_PPT_NUMCHG_TYPE_MAX, p_db_port->split_type));
            has_config = TRUE;
        }
        /* Fix bug 45042, only sfp need to show running */
        if (GLB_PORT_MEDIA_TYPE_SFP == p_db_port->port_media_type)
        {
            /* strip subport_no */
            if_build_port_fullname_by_slot_port(ifname, p_db_port->slot_no, p_db_port->panel_port_no, p_db_port->panel_sub_port_no);
            if (p_db_port->panel_sub_port_no)
            {
                ifname_len = sal_strlen(ifname);
                if ((ifname_len > 2) && (('/' == ifname[ifname_len-2]) || (':' == ifname[ifname_len-2])))
                {
                    ifname[ifname_len-2] = '\0';
                }
            }
            sal_fprintf(fp, "switch interface %s %s\n", 
                ifname, cdb_enum_val2str(glb_port_media_type_strs, GLB_PORT_MEDIA_TYPE_MAX, p_db_port->port_media_type));
            has_config = TRUE;
        }
    }

    for (port_idx = 0; port_idx < GLB_CARD_PORT_NUM_MAX; port_idx++)
    {
        p_db_port = p_master->port_array[port_idx];
        if (NULL == p_db_port)
        {
            continue;
        }

        if (p_db_port->bypass)
        {
            for (idx=0;idx<MAX_BYPASS_NUM;idx++)
            {
                if(p_db_port->bypass_slot == idx)
                    break;
            }
            
            bypass_port_cnt[idx]++;
            /* strip subport_no */
            if(bypass_port_cnt[idx] == 1)
                if_build_port_fullname_by_slot_port(bypass_ifname1[idx], p_db_port->slot_no, p_db_port->panel_port_no, p_db_port->panel_sub_port_no);
            else if(bypass_port_cnt[idx] == 2)
                if_build_port_fullname_by_slot_port(bypass_ifname2[idx], p_db_port->slot_no, p_db_port->panel_port_no, p_db_port->panel_sub_port_no);
        }
    }
    /* for bug 53765, by tongzb 2019-09-27 */
    for (idx=0;idx<MAX_BYPASS_NUM;idx++)
    {
        if(bypass_port_cnt[idx] == 2)
        {
            sal_fprintf(fp, "bypass interface %s %s slot %d\n", bypass_ifname1[idx],bypass_ifname2[idx],idx);
            has_config = TRUE;
        }
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_log_global(FILE* fp, cfg_cmd_para_t *para)
{
    char buf[CMD_BUF_256];
    tbl_log_global_t *p_log_glb = tbl_log_global_get_log_global();
    uint32 has_config = FALSE;
    
    cdb_bitmap_val2str(buf, CMD_BUF_256, p_log_glb->log_to_cdb, sizeof(logid_bmp_t)/4);
    if (sal_strlen(buf))
    {
        sal_fprintf(fp, "logging copy-to-cdb %s\n", buf);            
        has_config = TRUE;
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_sys_errdisable(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    tbl_brg_global_t *p_brg_glb = tbl_brg_global_get_brg_global();
    tbl_errdisable_flap_key_t flap_key;
    tbl_errdisable_flap_t *p_db_errdis_flap = NULL;
    tbl_errdisable_master_t *p_master = tbl_errdisable_get_master();
    tbl_errdisable_t *p_db_err = NULL;
    uint32 reason = 0;
    uint32 has_config = FALSE;

    if (GLB_FDB_LOOP_BUCKET_SIZE_DEF != p_brg_glb->fdb_loop_max_size
     || GLB_FDB_LOOP_BUCKET_RATE_DEF != p_brg_glb->fdb_loop_add_rate)
    {
        sal_fprintf(fp, "errdisable fdb-loop %u %u\n", p_brg_glb->fdb_loop_max_size, p_brg_glb->fdb_loop_add_rate);
        has_config = TRUE;
    }

    flap_key.flap_rsn = ERRDIS_FLAP_REASON_LINK;
    p_db_errdis_flap = tbl_errdisable_flap_get_errdisable_flap(&flap_key);
    if (p_db_errdis_flap)
    {
        if (GLB_LINK_FLAP_MAX_FLAP_COUNT != p_db_errdis_flap->flap_cnt_threshold
         || GLB_LINK_FLAP_MAX_FLAP_SECOND != p_db_errdis_flap->flap_time_threshold)
        sal_fprintf(fp, "errdisable flap reason link-flap %u %u\n", 
            p_db_errdis_flap->flap_cnt_threshold, p_db_errdis_flap->flap_time_threshold);
        has_config = TRUE;
    }

    if (ERRDISABLE_DEFAULT_INTERVAL != p_db_glb->errdisable_interval)
    {
        sal_fprintf(fp, "errdisable recovery interval %u\n", p_db_glb->errdisable_interval);
        has_config = TRUE;
    }

    for (reason = 0; reason < ERRDIS_RSN_MAX; reason++)
    {
        p_db_err = p_master->errdisable_array[reason];
        if (NULL == p_db_err)
        {
            continue;
        }

        if (p_db_err->recovery_en)
        {
            sal_fprintf(fp, "errdisable recovery reason %s\n", 
                cdb_enum_val2str(errdis_rsn_strs, ERRDIS_RSN_MAX, p_db_err->key.reason));
            has_config = TRUE;
        }
    }

    for (reason = 0; reason < ERRDIS_RSN_MAX; reason++)
    {
        p_db_err = p_master->errdisable_array[reason];
        if (NULL == p_db_err)
        {
            continue;
        }

        if (ERRDIS_FDB_LOOP == reason)
        {
            if (TRUE == p_db_err->errdisable_en)
            {
                sal_fprintf(fp, "errdisable detect reason %s\n", 
                    cdb_enum_val2str(errdis_rsn_strs, ERRDIS_RSN_MAX, p_db_err->key.reason));
                has_config = TRUE;
            }
        }
        else
        {
            if (TRUE != p_db_err->errdisable_en)
            {
                sal_fprintf(fp, CMD_NO_STR" errdisable detect reason %s\n", 
                    cdb_enum_val2str(errdis_rsn_strs, ERRDIS_RSN_MAX, p_db_err->key.reason));
                has_config = TRUE;
            }
        }
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ns_pf_is_default_entry(tbl_ns_port_forwarding_key_t *pkey)
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_ssh_cfg_t *p_ssh_cfg = tbl_ssh_cfg_get_ssh_cfg();

    if (pkey) {
        /* snmp server/telnetd/sshd default local ns mapping should be hidden from running cfg. */
        if ((NS_PORT_FORWARDING_UDP == pkey->proto && 161 == pkey->port) ||
            (NS_PORT_FORWARDING_TCP == pkey->proto && GLB_SSH_PORT_DEF == pkey->port) ||
            (NS_PORT_FORWARDING_TCP == pkey->proto && GLB_TELNET_PORT_DEF == pkey->port) ||
            (NS_PORT_FORWARDING_TCP == pkey->proto && GLB_HTTP_PORT_DEF == pkey->port) ||
            (NS_PORT_FORWARDING_TCP == pkey->proto && GLB_HTTPS_PORT_DEF == pkey->port)) {
            return 1;
        }
        
        /* modified telnetd/sshd port */
        if ((NS_PORT_FORWARDING_TCP == pkey->proto && p_sys_glb->telnet_port == pkey->port) ||
            (NS_PORT_FORWARDING_TCP == pkey->proto && p_sys_glb->http_port == pkey->port) ||
            (NS_PORT_FORWARDING_TCP == pkey->proto && p_sys_glb->https_port == pkey->port) ||
            (NS_PORT_FORWARDING_TCP == pkey->proto && p_sys_glb->rpcapi_port == pkey->port) ||
            (NS_PORT_FORWARDING_TCP == pkey->proto && p_sys_glb->rpcapi_ssl_port == pkey->port) ||
            (NS_PORT_FORWARDING_TCP == pkey->proto && p_ssh_cfg->listen_port == pkey->port) ||
            (NS_PORT_FORWARDING_TCP == pkey->proto && p_ssh_cfg->netconf_port == pkey->port)) {
            return 1;
        }
    }
    
    return 0;
}

int32
_ccs_cfg_build_sys_ns_port_forwarding(tbl_ns_port_forwarding_t *ns_port_forwarding, tbl_iter_args_t *args)
{
    tbl_ns_port_forwarding_key_t *ns_port_forwarding_key = &ns_port_forwarding->key;
    char buf[CMD_BUF_256];
    FILE* fp = args->argv[0];
    uint32 *has_config = (uint32 *)(args->argv[1]);
    char mapped_addr[16];
    uint8 build_mapped_addr = 0;

    if (NS_PORT_FORWARDING_REMOTE == ns_port_forwarding_key->type)
    {
        cdb_addr_ipv4_val2str(mapped_addr, 16, (void *)&ns_port_forwarding->key.mapped_addr);

        /* Skip build default mgmt ip addr 169.254.1.2. */
        if (strcmp(mapped_addr, GLB_NS_PORT_FORWARDING_VETH_MGMT_IP_ADDR))
        {
            build_mapped_addr = 1;
        }
    
        if (!build_mapped_addr) 
        {
            sal_fprintf(fp, "ns-port-forwarding remote-service %s %u %s\n", 
                        ns_port_forwarding_key->proto == NS_PORT_FORWARDING_TCP ? "tcp" : "udp",
                        ns_port_forwarding_key->port,
                        cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &ns_port_forwarding->ip));
        }
        else
        {
            sal_fprintf(fp, "ns-port-forwarding remote-service %s %u %s %s\n", 
                        ns_port_forwarding_key->proto == NS_PORT_FORWARDING_TCP ? "tcp" : "udp",
                        ns_port_forwarding_key->port,
                        cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &ns_port_forwarding->ip),
                        mapped_addr);
        }
    }
    else
    {
#ifndef TAPPRODUCT
        /* skip default entry */
        if (!ns_pf_is_default_entry(ns_port_forwarding_key)) {
            sal_fprintf(fp, "ns-port-forwarding local-service %s %u\n", 
                        ns_port_forwarding_key->proto == NS_PORT_FORWARDING_TCP ? "tcp" : "udp",
                        ns_port_forwarding_key->port);
        }
#endif
    }

    if (FALSE == *has_config)
    {
        *has_config = TRUE;
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_sys_ns_port_forwarding(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;
    uint32 has_config = FALSE;

    if (NULL == fp)
    {
        return PM_E_INVALID_PARAM;
    }

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = fp;
    args.argv[1] = &has_config;

    tbl_ns_port_forwarding_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_sys_ns_port_forwarding, &args);

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
_ccs_cfg_build_sys_ns_route(tbl_ns_route_t *p_ns_route, tbl_iter_args_t *args)
{
    FILE* fp = args->argv[0];
    uint32 *has_config = (uint32 *)(args->argv[1]);
    char ip_buf[GLB_IPV6_STR_LEN] = "";
    char if_or_ip[IFNAME_SIZE] = "";
    tbl_inband_snat_key_t key;
    tbl_inband_snat_t *p_nat;
    tbl_controller_attr_key_t controller_key;
    tbl_controller_attr_t *p_controller;

    /* check TBL_INBAND_SNAT */
    sal_memset(&controller_key, 0, sizeof(tbl_controller_attr_key_t));
    sal_memset(&key, 0, sizeof(tbl_inband_snat_key_t));
    key.ip = p_ns_route->key.ip.u.prefix4;
    key.port = p_ns_route->key.port;
    key.proto = INBAND_IP_PROTO_TCP;
    p_nat = tbl_inband_snat_get_inband_snat(&key);
    if (p_nat)
    {
        if (p_nat->ifname[0])
        {
            IFNAME_ETH2FULL(p_nat->ifname, if_or_ip);
        }
        else
        {
            sal_inet_ntop(AF_INET, &p_nat->ip, if_or_ip, sizeof(if_or_ip));
        }
    }
    sal_memcpy(&controller_key, &p_ns_route->key, sizeof(tbl_controller_attr_key_t));
    p_controller = tbl_controller_attr_get_controller_attr(&controller_key);

    if (NS_ROUTE_FLAG_OPENFLOW_CONTROLLER == p_ns_route->flag)
    {
        if (AF_INET == p_ns_route->key.ip.family)
        {
            sal_inet_ntop(AF_INET, &p_ns_route->key.ip.u.prefix4, ip_buf, sizeof(ip_buf));
        }
        else
        {
            sal_inet_ntop(AF_INET6, &p_ns_route->key.ip.u.prefix6, ip_buf, sizeof(ip_buf));
        }
        sal_fprintf(fp, "openflow set controller %s%s %s %hu", 
                        p_ns_route->is_inband ? "" : "mgmt-if ",
                        NS_ROUTE_PROTO_TO_STRING(p_ns_route->key.proto), 
                        ip_buf,
                        p_ns_route->key.port);
        
        if (p_nat)
        {
            sal_fprintf(fp, " %s %s", "source-ip", if_or_ip);
        }
        if (p_controller)
        {
            if (p_controller->max_backoff != GLB_CONTROLLER_DEFAULT_MAX_BACKOFF)
            {
                sal_fprintf(fp, " max_backoff %d", p_controller->max_backoff);
            }
            if (p_controller->inactivity_probe != GLB_CONTROLLER_DEFAULT_INACTIVITY_PROBE)
            {
                sal_fprintf(fp, " inactivity_probe %d", p_controller->inactivity_probe);
            }
        }
        sal_fprintf(fp, "\n");
    }

    if (FALSE == *has_config)
    {
        *has_config = TRUE;
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_sys_ns_route(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;
    uint32 has_config = FALSE;

    if (NULL == fp)
    {
        return PM_E_INVALID_PARAM;
    }

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = fp;
    args.argv[1] = &has_config;

    tbl_ns_route_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_sys_ns_route, &args);

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_fea_time(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_fea_time_t *p_db_time = tbl_fea_time_get_fea_time();

    if (GLB_TIME_SYNC_SYSTIME == p_db_time->sync_type)
    {
        sal_fprintf(fp, "timestamp sync systime\n");        
    }
    else if (GLB_TIME_SYNC_PTP == p_db_time->sync_type)
    {
        sal_fprintf(fp, "timestamp sync ptp\n");        
    }

    return PM_E_NONE;
}

int32
ccs_cfg_bhm_global(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_bhm_global_t *p_db_bhm = tbl_bhm_global_get_bhm_global();
    uint32 has_config = FALSE;

    if (GLB_BHM_SHUTDOWN == p_db_bhm->reactive)
    {
        sal_fprintf(fp, "heart-beat-monitor reactivate shutdown\n");
        has_config = TRUE;
    }
    else if (GLB_BHM_WARNING == p_db_bhm->reactive)
    {
        sal_fprintf(fp, "heart-beat-monitor reactivate warning\n");
        has_config = TRUE;
    }

    if (FALSE == p_db_bhm->sw_watchdog_enable)
    {
        sal_fprintf(fp, CMD_NO_STR" heart-beat-monitor enable\n");
        has_config = TRUE;
    }

    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}


static char *
ccs_ssm_desc_quality(uint32 value, uint8 desc_mode)
{   
    if (SSM_OPTION_MODE_1 == desc_mode)
    {
        if (SSM_QL_PRC == value)
        {
            return "PRC";
        }
        else if (SSM_QL_EEC1 == value)
        {
            return "EEC1";
        }
        else
        {
            return "NULL";
        }
    }
    else
    {
        if (SSM_QL_PRS == value)
        {
            return "PRS";
        }
        else if (SSM_QL_DNU == value)
        {
            return "DUS"; 
        }
        else if (SSM_QL_EEC2 == value)
        {
            return "EEC2";
        }
        else
        {
            return "NULL";
        }
            
    }
}

int32
ccs_cfg_build_ssm_global(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_ssm_global_t *p_db_ssm = tbl_ssm_global_get_ssm_global();
    tbl_sys_global_t* p_sys_glb = tbl_sys_global_get_sys_global();
    uint32            has_config = FALSE;
    const uint8       *clockid_addr = p_db_ssm->device_clock_id;

    /*get the default clock id*/
    ssm_clock_id_t clock_id;
    ssm_clock_id_t clk_id = {0x00, 0x00, 0x00, 0xFF, 0xFE};
    sal_memset(clock_id, 0, sizeof(clock_id));
    sal_memcpy(&clk_id[0], &p_sys_glb->route_mac[0], 3);
    sal_memcpy(&clk_id[5], &p_sys_glb->route_mac[3], 3);
    sal_memcpy(clock_id, clk_id, SSM_CLOCK_ID_LEN);
    /**/

    if (p_db_ssm->synce_enable)
    {
        sal_fprintf(fp, "clock synce global-enable\n");
        has_config = TRUE;
    }
    if (p_db_ssm->ssm_enable)
    {
        sal_fprintf(fp, "clock synce ssm enable\n");
        has_config = TRUE;
    }

    if (p_db_ssm->ssm_extend_enable)
    {
        sal_fprintf(fp, "clock synce ssm extend\n");
        has_config = TRUE;
    }

    if (SSM_CLOCK_PRIORITY_DEFAUTL != p_db_ssm->local_priority)
    {
        sal_fprintf(fp, "clock synce local-priority %d\n", p_db_ssm->local_priority);
        has_config = TRUE;
    }

    if (SSM_DEVICE_MAST == p_db_ssm->device_role)
    {
        sal_fprintf(fp, "clock synce master-only\n");
        has_config = TRUE;
    }
    else if (SSM_DEVICE_SLAV == p_db_ssm->device_role)
    {
        sal_fprintf(fp, "clock synce slave-only\n");
        has_config = TRUE;
    }

    if (SYNCE_HOLDOFF_DEFAULT_TIME != p_db_ssm->holdoff_time)
    {
        sal_fprintf(fp, "clock synce hold-off %d\n", p_db_ssm->holdoff_time/100);
        has_config = TRUE;
    }

    if (SYNCE_WTR_DEFAULT_TIME != p_db_ssm->wtr_time)
    {
        sal_fprintf(fp, "clock synce wait-to-restore %d\n", p_db_ssm->wtr_time/60);
        has_config = TRUE;
    }
    
    if (SSM_OPTION_MODE_1 == p_db_ssm->option_mode)
    {
        if (SSM_QL_DNU != p_db_ssm->system_quality)
        {
            sal_fprintf(fp, "clock synce local-quality option1 %s\n", ccs_ssm_desc_quality(p_db_ssm->system_quality, SSM_OPTION_MODE_1));
            has_config = TRUE;
        }
    }
    else
    {
        sal_fprintf(fp, "clock synce local-quality option2 %s\n", ccs_ssm_desc_quality(p_db_ssm->system_quality, SSM_OPTION_MODE_2));
        has_config = TRUE;
    }

    if (0 != sal_memcmp(clock_id, p_db_ssm->device_clock_id, SSM_CLOCK_ID_LEN))
    {
        sal_fprintf(fp, "clock synce clock-id %.04hX.%.04hX.%.04hX.%.04hX\n", 
                    sal_ntoh16(((uint16*)clockid_addr)[0]),
                    sal_ntoh16(((uint16*)clockid_addr)[1]),
                    sal_ntoh16(((uint16*)clockid_addr)[2]),
                    sal_ntoh16(((uint16*)clockid_addr)[3]));
        has_config = TRUE;
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

