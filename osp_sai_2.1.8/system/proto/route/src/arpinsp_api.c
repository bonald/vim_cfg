#include "proto.h"
#include "lib_tblinc.h"
#include "lib_sysctl.h"
#include "route.h"
#include "arp_define.h"
#include "cdb_const.h"
#include "glb_l2_define.h"
#include "glb_arpacl_define.h"
#include "arpinsp.h"
#include "arpacl.h"
#include "arpinsp_api.h"
#include "sal.h"

/* log buffer queue for arp inspection */
S_ArpInspLogBuf g_arpinsp_logBuffer;
extern ctc_task_t *arpinsp_logtimer;

extern char*
format_time_str(char *pszBuf, size_t nLen, char *pszFormat, time_t nTime);

S_ArpInspLogItem *arpinsp_malloc_logitem(uint32 nLen)
{
    S_ArpInspLogItem *p = NULL;

    if (nLen < 0 || nLen >= LOGGING_BUF_SIZE)
    {
        return NULL;
    }
    
    p = XMALLOC(MEM_ARPINSP_LOGBUF, sizeof(S_ArpInspLogItem));
    if (NULL == p)
    {
        return NULL;
    }

    p->pszLog = XMALLOC(MEM_ARPINSP_LOGBUF, (nLen + 1));
    if (NULL == p->pszLog)
    {
        XFREE(MEM_ARPINSP_LOGBUF, p);
        return NULL;
    }
    p->nLen = (nLen + 1);
    
    return p;
}

void 
arpinsp_free_logitem(S_ArpInspLogItem *pItem)
{
    if (NULL != pItem)
    {
        if (NULL != pItem->pszLog)
        {
            XFREE(MEM_ARPINSP_LOGBUF, pItem->pszLog);
        }
        XFREE(MEM_ARPINSP_LOGBUF, pItem);
    }
    return;
}

void
arpinsp_write_syslog(S_ArpInspLogItem *pItem_buf)
{
    if (NULL == pItem_buf)
    {
        return;
    }
    switch (pItem_buf->severy) 
    {
        case E_EMERGENCY:
            break;
        case E_ALERT:
            break;
        case E_CRITICAL:
            break;
        case E_ERROR:
            logid_sys(LOG_ARPINSP_3_CHECK, pItem_buf->pszLog);
            break;
        case E_WARNING:
            logid_sys(LOG_ARPINSP_4_RESULT, pItem_buf->pszLog);
            break;
        case E_NOTICE:
            logid_sys(LOG_ARPINSP_5_ACL, pItem_buf->pszLog);
            break;
        case E_INFORMATIONAL:
            break;
        case E_DEBUG:
            break;
        case E_MAX:
            break;
    }
    return;
}

void
arpinsp_write_log_buffer_timer_cb(void *p_argv)
{
    uint32 log_number = 0;
    tbl_route_global_t * p_rt_glb = tbl_route_global_get_route_global();
    S_ArpInspLogItem *pItem_buf = NULL; 
    arpinsp_logtimer = NULL;

    /*added by chenc to fix bug 50306, 2018-12-12*/
    if (0 == p_rt_glb->arpinsp_logbuf_logsec)
    {
        if (CTC_TASK_IS_RUNNING(arpinsp_logtimer))
        {
            CTC_TASK_STOP_TIMER(arpinsp_logtimer);
        }
        return;
    }
        
    /*added by yejl to fix bug44158, 2017-05-25*/
    if (0 == p_rt_glb->arpinsp_logbuf_lognum)
    {
        arpinsp_logtimer = CTC_TASK_ADD_TIME(arpinsp_write_log_buffer_timer_cb, NULL, p_rt_glb->arpinsp_logbuf_logsec);
        return;
    }
    /*ended by yejl*/
    while (!TAILQ_EMPTY(&g_arpinsp_logBuffer))
    {
        pItem_buf = TAILQ_FIRST(&g_arpinsp_logBuffer);
        /* remove record from log list */
        TAILQ_REMOVE(&g_arpinsp_logBuffer, pItem_buf, entry);
        /* save the log info to syslog */
        arpinsp_write_syslog(pItem_buf);
        arpinsp_free_logitem(pItem_buf);
        p_rt_glb->arpinsp_logbuf_curnum --;
        log_number++;
        if (log_number == p_rt_glb->arpinsp_logbuf_lognum)
        {
            break;
        }
    }
    arpinsp_logtimer = CTC_TASK_ADD_TIME(arpinsp_write_log_buffer_timer_cb, NULL, p_rt_glb->arpinsp_logbuf_logsec);
    return;
}

int32 
arpinsp_logBuf_add(unsigned char *str, E_SEVERITY servery)
{
    tbl_route_global_t * p_rt_glb = tbl_route_global_get_route_global();
    S_ArpInspLogItem *pItem = NULL;
    time_t nCur = sal_time(NULL);
    char szBuf[M_LOG_BUFSIZE] = {'\0'};
    char log_time[M_TIME_BUFSIZE] = {'\0'};
    int nLen = 0;

    format_time_str(log_time, M_TIME_BUFSIZE, "%Y-%m-%d %H:%M:%S ", nCur);
    sal_strncpy((char*)szBuf, (char*)str, sal_strlen((char*)str));
    nLen = sal_strlen(szBuf);

    if (p_rt_glb->arpinsp_logbuf_logsec == 0)
    {
        pItem = arpinsp_malloc_logitem(nLen);
        if (!pItem)
        {
            return PM_E_FAIL;
        }
        sal_memcpy(pItem->pszLog, szBuf, nLen);
        pItem->pszLog[nLen] = '\0';
        sal_memcpy(pItem->time_str, log_time, M_TIME_BUFSIZE);
        pItem->severy = servery;
        arpinsp_write_syslog(pItem);
        arpinsp_free_logitem(pItem);
        return PM_E_NONE;
    }

    if (p_rt_glb->arpinsp_logbuf_curnum >= p_rt_glb->arpinsp_logbuf_entrynum)
    {
        pItem = TAILQ_FIRST(&g_arpinsp_logBuffer);
        TAILQ_REMOVE(&g_arpinsp_logBuffer, pItem, entry);
        if (pItem->nLen <= nLen)
        {
            /* can't reuse this log item */
            arpinsp_free_logitem(pItem);
            pItem = NULL;
        }
        p_rt_glb->arpinsp_logbuf_curnum--;
    }

    /* malloc log item if needed */
    if (NULL == pItem)
    {
        pItem = arpinsp_malloc_logitem(nLen);
        if (NULL == pItem)
        {
            return PM_E_FAIL;
        }
    }

    /* we have malloced 1 more byte to hold the '\0' */
    sal_memcpy(pItem->pszLog, szBuf, nLen);
    pItem->pszLog[nLen] = '\0';
    pItem->severy = servery;
    sal_memcpy(pItem->time_str, log_time, M_TIME_BUFSIZE);

    /* insert record to log buffer queue */
    TAILQ_INSERT_TAIL(&g_arpinsp_logBuffer, pItem, entry);
    p_rt_glb->arpinsp_logbuf_curnum++;

    return PM_E_NONE;
}

void
arpinsp_mac_string(unsigned char *mac, char *mac_str)
{
    sal_snprintf(mac_str, 18, "%02x:%02x:%02x:%02x:%02x:%02x", 
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return;
}

void
arpinsp_ip_string(unsigned char *ip, char *ip_str) 
{
    sal_snprintf(ip_str, 16, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return;
}

int
arpinsp_check_ip(u_char *ipaddr)
{
    u_int32_t *p =  (u_int32_t *)ipaddr;
    
    /* filtrate ip with 0.0.0.0 */
    if (0 == *p)
    {
        return PM_E_FAIL;
    }
    
    /* filtrate ip with 255.255.255.255 */
    if (255 == ipaddr[0] && 255 == ipaddr[1] && 255 == ipaddr[2] && 255 == ipaddr[3])
    {
        return PM_E_FAIL;
    }
    
    /* filtrate multicast addresses */
    if (IN_MULTICAST(sal_ntoh32(*p)))
    {
        return PM_E_FAIL;
    }
    return PM_E_NONE;
}

int32
arpinsp_validation_check(eth_hdr_t *eth_p, arp_eth_t *arp_p, tbl_arpinsp_t* p_db_arpinsp)
{
    tbl_route_global_t * p_rt_glb = tbl_route_global_get_route_global();
    char szBuf[M_LOG_STRSIZE] = {'\0'};
    char mac_str_one[MAC_STRING_LEN] = {'\0'};
    char mac_str_two[MAC_STRING_LEN] = {'\0'};
    char ip_str[IP_STRING_LEN] = {'\0'};

    if (GLB_FLAG_ISSET(p_rt_glb->arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_SRCMAC))
    {
         /* Check src-mac for additional validation */
        if (sal_memcmp(arp_p->ar_sha, eth_p->src_mac, MAC_ADDR_LEN))
        {
            arpinsp_mac_string((unsigned char*)arp_p->ar_sha, mac_str_one);
            arpinsp_mac_string((unsigned char*)eth_p->src_mac, mac_str_two);
            ARPINSP_PKT_DEBUG("Source MAC %s mismatch with %s in ethernet header\n", mac_str_one, mac_str_two);
            sal_snprintf(szBuf, M_LOG_STRSIZE, "Source MAC %s mismatch with %s in ethernet header\n", 
                                mac_str_one, mac_str_two);
            arpinsp_logBuf_add((unsigned char *)szBuf, E_ERROR);

            p_db_arpinsp->srcmac_failure++;
            p_db_arpinsp->invalid_protocol++;
            p_db_arpinsp->dropped++;
            return ARPINSP_DIFF_SRCMAR_ERROR;
        }
    }

    if (GLB_FLAG_ISSET(p_rt_glb->arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_IP))
    {
        /* Check sender ip for additional validation */
        if (arpinsp_check_ip(arp_p->ar_sip))
        {
            /* packet error */
            arpinsp_ip_string(arp_p->ar_sip, ip_str);  /*get ip string*/
            ARPINSP_PKT_DEBUG("Sender ip %s is invalid", ip_str);
            sal_snprintf(szBuf, M_LOG_STRSIZE, "Sender ip %s is invalid\n", ip_str);
            arpinsp_logBuf_add((unsigned char *)szBuf, E_ERROR);

            p_db_arpinsp->ip_failure++;
            p_db_arpinsp->invalid_protocol++;
            p_db_arpinsp->dropped++;
            return ARPINSP_ERR_IP_ERROR;
        }
        /* for reply arp we should check the target ip */
        if (ARP_OP_REPLY == sal_hton16(arp_p->ar_op))
        {
            /* Check target ip for additional validation */
            if (arpinsp_check_ip(arp_p->ar_tip))
            {
                /* packet error */
                /*added by yejl to fix bug 44114, 2017-05-22*/
                arpinsp_ip_string(arp_p->ar_tip, ip_str);  /*get ip string*/
                ARPINSP_PKT_DEBUG("Target ip %s is invalid", ip_str);
                sal_snprintf(szBuf, M_LOG_STRSIZE, "Target ip %s is invalid\n", ip_str);
                arpinsp_logBuf_add((unsigned char *)szBuf, E_ERROR);

                p_db_arpinsp->ip_failure++;
                p_db_arpinsp->invalid_protocol++;
                p_db_arpinsp->dropped++;
                return ARPINSP_ERR_IP_ERROR;
            }
        }
    }

    /* for reply arp we should check the target mac */
    if (ARP_OP_REPLY == sal_hton16(arp_p->ar_op))
    {
        if (GLB_FLAG_ISSET(p_rt_glb->arpinsp_validate_flag, GLB_ARPINSP_FLAG_VERIFY_DESTMAC))
        {
            /* Check dst-mac for additional validation */
            if (sal_memcmp(arp_p->ar_tha, eth_p->dest_mac, MAC_ADDR_LEN))
            {
                /* packet error */
                arpinsp_mac_string((unsigned char*)arp_p->ar_tha, mac_str_one);     /*get mac string*/
                arpinsp_mac_string((unsigned char*)eth_p->dest_mac, mac_str_two);   /*get mac string*/
                ARPINSP_PKT_DEBUG("Target MAC %s miss match with %s in ethernet header", mac_str_one, mac_str_two);
                sal_snprintf(szBuf, M_LOG_STRSIZE, "Target MAC %s miss match with %s in ethernet header\n", 
                                    mac_str_one, mac_str_two);
                arpinsp_logBuf_add((unsigned char *)szBuf, E_ERROR);
                
                p_db_arpinsp->dstmac_failure++;
                p_db_arpinsp->invalid_protocol++;
                p_db_arpinsp->dropped++;
                return ARPINSP_DIFF_DESTMAR_ERROR;
            }
        }
    }
    return ARPINSP_SUCCESS;
}

static int32
arpacl_access_list_match(tbl_arpace_config_t* p_arpace_config, arp_eth_t *arp_p)
{
    addr_ipv4_t send_addr;
    mac_addr_t mac_addr;
    uint32 i = 0;
    char buf1[CMD_BUF_32] = {'\0'};
    char buf2[CMD_BUF_16] = {'\0'};
    char buf3[CMD_BUF_16] = {'\0'};
    char buf4[CMD_BUF_16] = {'\0'};
    char buf5[CMD_BUF_16] = {'\0'};
    char buf6[CMD_BUF_32] = {'\0'};
    char buf7[CMD_BUF_32] = {'\0'};
    char buf[CMD_BUF_256] = {'\0'};
    uint32 op = sal_hton16(arp_p->ar_op);

    /* 1, check ip*/
    sal_memcpy((char*)&send_addr, arp_p->ar_sip, IPV4_MAX_BYTELEN);
    if ((send_addr.s_addr & ~(p_arpace_config->src_ip_mask.s_addr)) != 
        (p_arpace_config->src_ip.s_addr & ~(p_arpace_config->src_ip_mask.s_addr)))
    {
        return ARP_ACL_IP_NOT_MATCH;
    }
    /* 2, check mac */
    sal_memcpy(mac_addr, arp_p->ar_sha, MAC_ADDR_LEN);   
    for (i = 0; i <MAC_ADDR_LEN; i++)
    {
        if ((mac_addr[i] & (0xff - p_arpace_config->src_mac_mask[i])) !=
             (p_arpace_config->src_mac[i] & (0xff - p_arpace_config->src_mac_mask[i])))
        {
            return ARP_ACL_MAC_NOT_MATCH;
        }
    }

    /* 3, check type*/
    if((p_arpace_config->arp_type != op) && (p_arpace_config->arp_type != GLB_ARP_TYPE_ALL))
    {
        return ARP_ACL_TYPE_NOT_MATCH;
    }

    /* 4, match, log it */
    if (p_arpace_config->arp_log)
    { 
        /*1.sequence number*/
        /*added by yejl to fix bug 44154, 2017-05-25*/
        sal_sprintf(buf1, "ARP_ACL: match arp ace %u", p_arpace_config->key.seq_no);
        sal_strncat(buf, buf1, CMD_BUF_32);

        /*2.deny or permit*/
        sal_sprintf(buf2, " %s", (p_arpace_config->arp_deny? "deny" : "permit"));
        sal_strncat(buf, buf2, CMD_BUF_16);

        /*3. request or response or all*/
        if (p_arpace_config->arp_type == GLB_ARP_REQUEST)
        {
            sal_sprintf(buf3, " %s", "request");
        }
        else if (p_arpace_config->arp_type == GLB_ARP_RESPONSE)
        {
            sal_sprintf(buf3, " %s", "response");
        }
        else if (p_arpace_config->arp_type == GLB_ARP_TYPE_ALL)
        {
            sal_sprintf(buf3, " %s", "all");
        }
        sal_strncat(buf, buf3, CMD_BUF_16);

        /*4. ip*/
        cdb_addr_ipv4_val2str(buf4, CMD_BUF_16, &send_addr);
        sal_sprintf(buf5, " ip %s", buf4);
        sal_strncat(buf, buf5, CMD_BUF_32);
        
        /*5. mac*/
        cdb_mac_addr_val2str(buf6, CMD_BUF_16, mac_addr);
        sal_sprintf(buf7, " mac %s \n", buf6);
        sal_strncat(buf, buf7, CMD_BUF_32);

        arpinsp_logBuf_add((unsigned char *)buf, E_NOTICE);
    }

    return p_arpace_config->arp_deny;
}

int32
arpinsp_acl_check(arp_eth_t *arp_p, tbl_arpinsp_t* p_db_arpinsp)
{
    tbl_arpace_config_master_t* p_arpace_master = tbl_arpace_config_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_arpacl_config_t* p_arpacl_config = NULL;
    tbl_arpacl_config_key_t arpacl_key;
    tbl_arpace_config_t* p_arpace_config = NULL;
    int32 ret = 0;

    sal_memset(&arpacl_key, 0, sizeof(tbl_arpacl_config_key_t));
    sal_strncpy(arpacl_key.name, p_db_arpinsp->filter, ARPINSP_FILTER_MAX_NUM);
    p_arpacl_config = tbl_arpacl_config_get_arpacl_config(&arpacl_key);
    if (NULL == p_arpacl_config)
    {
        return ARP_ACL_NOT_EXIT;
    }
    CTCLIB_SLIST_LOOP(p_arpace_master->arpace_config_list, p_arpace_config, listnode)
    {
        if (0 == sal_strcmp(p_db_arpinsp->filter, p_arpace_config->key.arpacl_name))
        {
            ret = arpacl_access_list_match(p_arpace_config, arp_p);
            if (ret == ARP_ACL_PERMIT || ret == ARP_ACL_DENY)
            {
                return ret;
            }
        }
    }
    return ARP_ACL_NOT_MATCH;
}

int32
arpinsp_acl_log(int32 acl_check_result, tbl_arpinsp_t* p_db_arpinsp)
{
    char szBuf[M_LOG_STRSIZE] = {'\0'};
    switch(acl_check_result)
    {
    case ARP_ACL_DENY:
        if (GLB_ACL_MATCH_MATCHLOG == p_db_arpinsp->acllog_filter || 
            GLB_ACL_DHCP_DENY == p_db_arpinsp->acllog_filter)
        {
            sal_snprintf(szBuf, M_LOG_STRSIZE, "Drop an ARP packet by ARP ACL on vlan %d \n", p_db_arpinsp->key.vlan_id);
            arpinsp_logBuf_add((unsigned char *)szBuf, E_ERROR);
        }
        p_db_arpinsp->acl_dropped++;
        p_db_arpinsp->dropped++;
        return ARPINSP_ACL_DENY;
    case ARP_ACL_PERMIT:
        if (GLB_ACL_MATCH_MATCHLOG == p_db_arpinsp->acllog_filter)
        {
            sal_snprintf(szBuf, M_LOG_STRSIZE, "Permit an ARP packet by ARP ACL on vlan %d \n", p_db_arpinsp->key.vlan_id);
            arpinsp_logBuf_add((unsigned char *)szBuf, E_ERROR);
        }
        p_db_arpinsp->acl_permit++;
        p_db_arpinsp->forwarded++;
        return ARPINSP_SUCCESS;
    case ARP_ACL_NOT_MATCH:
        if (GLB_ARPINSP_ACL_STATIC == p_db_arpinsp->filter_state)
        {
            sal_snprintf(szBuf, M_LOG_STRSIZE, "Drop an ARP packet for ARP ACL mismatch on vlan %d \n", p_db_arpinsp->key.vlan_id);
            arpinsp_logBuf_add((unsigned char *)szBuf, E_WARNING);
            
            p_db_arpinsp->acl_dropped++;
            p_db_arpinsp->dropped++;
            return ARPINSP_ACL_DENY;
        }
        break;
    case ARP_ACL_NOT_EXIT:
        return -1;
    }
    return -1;
}

int32
arpinsp_bindingtable_match(eth_hdr_t *eth_p, arp_eth_t *arp_p, tbl_arpinsp_t* p_db_arpinsp, tbl_interface_t* p_db_if)
{
    tbl_dhcbinding_t    *p_db_dhcbinding   = NULL;  
    tbl_dhcbinding_t    *p_dhcbinding   = NULL; 
    tbl_dhcbinding_master_t *p_master = tbl_dhcbinding_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_dhcbinding_key_t  dhcbinding_key;
    char mac_str_one[MAC_STRING_LEN] = {'\0'};
    char mac_str_two[MAC_STRING_LEN] = {'\0'};
    char szBuf[M_LOG_STRSIZE] = {'\0'};
    char ip_str[IP_STRING_LEN] = {'\0'};
    char szIpBuf[CMD_BUF_16];
    char ifname_ext[IFNAME_EXT_SIZE];
    char ifname_ext1[IFNAME_EXT_SIZE];
    addr_ipv4_t send_addr;

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    
    sal_memset(&dhcbinding_key, 0, sizeof(dhcbinding_key));
    sal_memcpy(dhcbinding_key.mac, arp_p->ar_sha, MAC_ADDR_LEN);
    dhcbinding_key.ipv6 = 0;
    dhcbinding_key.vid = p_db_arpinsp->key.vlan_id;
    p_db_dhcbinding = tbl_dhcbinding_get_dhcbinding(&dhcbinding_key);
    if (NULL == p_db_dhcbinding)
    {
        /*not print log*/
        CTCLIB_SLIST_LOOP(p_master->dhcbinding_list, p_dhcbinding, listnode)
        {
            if (p_dhcbinding->key.vid != p_db_arpinsp->key.vlan_id)
            {
                continue;
            }
            if (0 != sal_memcmp(arp_p->ar_sha, p_dhcbinding->client_mac, MAC_ADDR_LEN))
            {
                sal_memcpy((char*)&send_addr, arp_p->ar_sip, IPV4_MAX_BYTELEN);
                if (send_addr.s_addr == p_dhcbinding->client_ip.u.prefix4.s_addr)
                {
                    arpinsp_mac_string((unsigned char*)arp_p->ar_sha, mac_str_one);     /*get mac string*/
                    arpinsp_mac_string((unsigned char*)p_dhcbinding->client_mac, mac_str_two);   /*get mac string*/
                    ARPINSP_PKT_DEBUG("Source MAC %s miss match with %s in DHCP binding table\n", mac_str_one, mac_str_two);
                    if (GLB_ACL_DHCP_DENY == p_db_arpinsp->dhcplog_filter || 
                        GLB_DHCP_BINDINGS_ALL == p_db_arpinsp->dhcplog_filter)
                    {
                        sal_snprintf(szBuf, M_LOG_STRSIZE, "Source MAC %s miss match with %s in DHCP binding table\n", 
                                     mac_str_one, mac_str_two);
                        arpinsp_logBuf_add((unsigned char *)szBuf, E_ERROR);
                    }
                    return ARPINSP_DIFF_SRCMAR_ERROR;
                }
            }
        }
        return ARPINSP_SUCCESS;
    }

    if (GLB_DHCP_BINDINGS_ALL == p_db_arpinsp->dhcplog_filter)
    {
        arpinsp_logBuf_add((unsigned char *)"Match MAC in DHCP binding table \n", E_ERROR);
    }

    /* check MAC */
    if (sal_memcmp(arp_p->ar_sha, p_db_dhcbinding->client_mac, MAC_ADDR_LEN))
    {
        /* packet error */
        arpinsp_mac_string((unsigned char*)arp_p->ar_sha, mac_str_one);     /*get mac string*/
        arpinsp_mac_string((unsigned char*)p_db_dhcbinding->client_mac, mac_str_two);   /*get mac string*/
        ARPINSP_PKT_DEBUG("Source MAC %s miss match with %s in DHCP binding table\n", mac_str_one, mac_str_two);

        if (GLB_ACL_DHCP_DENY == p_db_arpinsp->dhcplog_filter || 
            GLB_DHCP_BINDINGS_ALL == p_db_arpinsp->dhcplog_filter)
        {
            sal_snprintf(szBuf, M_LOG_STRSIZE, "Source MAC %s miss match with %s in DHCP binding table\n", 
                         mac_str_one, mac_str_two);
            arpinsp_logBuf_add((unsigned char *)szBuf, E_ERROR);
        }
        return ARPINSP_DIFF_SRCMAR_ERROR;
    }

    /* check ip */
    sal_memcpy((char*)&send_addr, arp_p->ar_sip, IPV4_MAX_BYTELEN);
    if (send_addr.s_addr != p_db_dhcbinding->client_ip.u.prefix4.s_addr)
    {
        arpinsp_ip_string(arp_p->ar_sip, ip_str);   /*get ip string*/
        ARPINSP_PKT_DEBUG("Source ip %s miss match with %s in DHCP binding table\n", 
            ip_str, cdb_addr_val2str((char*)szIpBuf, CMD_BUF_128, &p_db_dhcbinding->client_ip));
        if (GLB_ACL_DHCP_DENY == p_db_arpinsp->dhcplog_filter || 
            GLB_DHCP_BINDINGS_ALL == p_db_arpinsp->dhcplog_filter)
        {
            sal_snprintf(szBuf, M_LOG_STRSIZE, "Source ip %s miss match with %s in DHCP binding table \n", 
                            ip_str, cdb_addr_val2str((char*)szIpBuf, CMD_BUF_128, &p_db_dhcbinding->client_ip));
            arpinsp_logBuf_add((unsigned char *)szBuf, E_ERROR);
        }
        return ARPINSP_ERR_IP_ERROR;
    }
    
    /* check vlan */
#if 0
    if (p_db_arpinsp->key.vlan_id != p_db_dhcbinding->vid)
    {
        ARPINSP_PKT_DEBUG("Vlan %d miss match with %d in DHCP binding table\n", p_db_arpinsp->key.vlan_id, p_db_dhcbinding->vid);
        if (GLB_ACL_DHCP_DENY == p_db_arpinsp->dhcplog_filter ||
            GLB_DHCP_BINDINGS_ALL == p_db_arpinsp->dhcplog_filter)
        {
            sal_snprintf(szBuf, M_LOG_STRSIZE, "Vlan %d miss match with %d in DHCP binding table \n", 
                p_db_arpinsp->key.vlan_id, p_db_dhcbinding->vid);
            arpinsp_logBuf_add((unsigned char *)szBuf, E_ERROR);
        }
        return ARPINSP_DIFF_VLAN_ERROR;
    }
#endif   

    /* check interface */
    if (sal_memcmp(p_db_if->key.name, p_db_dhcbinding->ifname, IFNAME_SIZE))
    {
        IFNAME_ETH2FULL(p_db_dhcbinding->ifname, ifname_ext1);
        ARPINSP_PKT_DEBUG("Interface %s miss match with %s in DHCP binding table\n", ifname_ext, ifname_ext1);
        if (GLB_ACL_DHCP_DENY == p_db_arpinsp->dhcplog_filter || 
            GLB_DHCP_BINDINGS_ALL == p_db_arpinsp->dhcplog_filter)
        {
            sal_snprintf(szBuf, M_LOG_STRSIZE, "Interface %s miss match with %s in DHCP binding table \n", ifname_ext, ifname_ext1);
            arpinsp_logBuf_add((unsigned char *)szBuf, E_ERROR);
        }
        return ARPINSP_DIFF_PORT_ERROR;
    }

    /* deal log */
    if (GLB_DHCP_BINDINGS_PERMIT == p_db_arpinsp->dhcplog_filter || 
        GLB_DHCP_BINDINGS_ALL == p_db_arpinsp->dhcplog_filter)
    {
        arpinsp_logBuf_add((unsigned char *)"Match an ARP packet in DHCP binding table. \n", E_ERROR);
    }
    p_db_arpinsp->dhcp_permit++;

    return ARPINSP_SUCCESS;
}

static int32
arpinsp_debug_dump_packet(arp_eth_t *arp_p)
{
    ARPINSP_DEBUG_PRINT("dump ARP packet fields");
    ARPINSP_DEBUG_PRINT("  hardware address = 0x%04x, protocol address = 0x%04x, hardware address length = %d, protocol address length = %d",
    arp_p->ar_hrd, arp_p->ar_pro, arp_p->ar_hln, arp_p->ar_pln);
    if (ARP_OP_REQUEST == sal_hton16(arp_p->ar_op))
    {
        ARPINSP_DEBUG_PRINT("  ARP opcode = Request");
    }
    else if (ARP_OP_REPLY== sal_hton16(arp_p->ar_op))
    {
        ARPINSP_DEBUG_PRINT("  ARP opcode = Reply");
    }
    else
    {
        ARPINSP_DEBUG_PRINT("  ARP opcode = other ARP type");
    }
    ARPINSP_DEBUG_PRINT("  sender MAC = %02x:%02x:%02x:%02x:%02x:%02x, sender IP = %d.%d.%d.%d, target MAC = %02x:%02x:%02x:%02x:%02x:%02x, target IP = %d.%d.%d.%d", 
                        arp_p->ar_sha[0], arp_p->ar_sha[1], 
                        arp_p->ar_sha[2], arp_p->ar_sha[3],
                        arp_p->ar_sha[4], arp_p->ar_sha[5],
                        arp_p->ar_sip[0], arp_p->ar_sip[1],
                        arp_p->ar_sip[2], arp_p->ar_sip[3],
                        arp_p->ar_tha[0], arp_p->ar_tha[1], 
                        arp_p->ar_tha[2], arp_p->ar_tha[3],
                        arp_p->ar_tha[4], arp_p->ar_tha[5],
                        arp_p->ar_tip[0], arp_p->ar_tip[1],
                        arp_p->ar_tip[2], arp_p->ar_tip[3]);
    return ARPINSP_SUCCESS;
}

int32
arpinsp_valid_check(tbl_interface_t* p_db_if, arp_eth_t *arp_p, eth_hdr_t *eth_p, uint32 vid)
{
    tbl_arpinsp_t* p_db_arpinsp = NULL;
    tbl_arpinsp_key_t arpinsp_key;
    char szBuf[M_LOG_STRSIZE] = {'\0'};
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 check_result = 0;

    /*interface is can not be NULL*/
    if (NULL == p_db_if)
    {
        ARPINSP_EVENT_DEBUG("The interface is not existed.\n");
        return ARPINSP_PACERROR;  
    }
    /*interface is must be l2 port*/
    if (p_db_if->mode != GLB_IF_MODE_L2)
    {
        return ARPINSP_NO_CHECK;    
    }
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    
    /* we only deal ARP request and reply */
    if ((ARP_OP_REQUEST != sal_hton16(arp_p->ar_op)) && (ARP_OP_REPLY != sal_hton16(arp_p->ar_op)))
    {
        return ARPINSP_NO_CHECK;
    }

    /*get the correct vlan parameter*/
    if (vid < GLB_VLAN_DEFAULT_VID || vid > GLB_VLAN_MAX)
    {
        /* The VLAN is invalid parameter*/
        ARPINSP_EVENT_DEBUG("The VLAN is invalid parameter\n");
        return ARPINSP_NO_CHECK;
    }

    /*get the arp inspection*/
    sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
    arpinsp_key.vlan_id = vid;
    p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
    if (NULL == p_db_arpinsp)
    {
        /* arpinsp is not exist, we needn't check it*/
        ARPINSP_EVENT_DEBUG("The arp inspection vlan is not existed\n");
        return ARPINSP_NO_CHECK;
    }

    /*1. check whether ARP inspection is enabled on the vlan */
    if (!p_db_arpinsp->enable_state)
    {
        ARPINSP_EVENT_DEBUG("The arp inspection is not enabled on VLAN %d\n", vid);
        p_db_arpinsp->forwarded++;
        return ARPINSP_NO_CHECK;
    }
    
    /*added by yejl to fix bug 44231, 2017-06-06*/
    ARPINSP_EVENT_DEBUG("Get an ARP packet on port %s\n", ifname_ext);
    if (RT_DBG_IS_ON(arpinspection, ARPINSPECTION_PACKET))
    {
        arpinsp_debug_dump_packet(arp_p);
    }
    /*ended by yejl*/
    
    /*2. check whether the interface is trusted */
    if (p_db_if->arpinsp_trust_enable)
    {
        ARPINSP_PKT_DEBUG("Forward an ARP packet on trust port %s\n", ifname_ext);
        p_db_arpinsp->forwarded++;
        return ARPINSP_SUCCESS;
    }

    /*3. additional validation check */
    check_result = arpinsp_validation_check(eth_p, arp_p, p_db_arpinsp);
    if (ARPINSP_SUCCESS != check_result)
    {
        /* add log */
        sal_snprintf(szBuf, M_LOG_STRSIZE, "Drop an ARP packet on port %s for validation check\n", ifname_ext);
        arpinsp_logBuf_add((unsigned char *)szBuf, E_WARNING);
        return check_result;
    }

    /*4. ACL check, if state ARPINSP_ACL_CLEAR means no ACL */
    if (GLB_ARPINSP_ACL_CLEAR != p_db_arpinsp->filter_state)
    {
        check_result = arpinsp_acl_check(arp_p, p_db_arpinsp);
        check_result = arpinsp_acl_log(check_result, p_db_arpinsp);
        if (check_result >= 0)
        {
            return check_result;
        }
    }

    /*5. search binding table */
    check_result = arpinsp_bindingtable_match(eth_p, arp_p, p_db_arpinsp, p_db_if);    
    if (ARPINSP_SUCCESS != check_result)
    {
        /* add log */
        sal_snprintf(szBuf, M_LOG_STRSIZE, "Drop an ARP packet on port %s \n", ifname_ext);
        arpinsp_logBuf_add((unsigned char *)szBuf, E_WARNING);
        p_db_arpinsp->dhcp_dropped++;
        p_db_arpinsp->dropped++;
        return check_result; 
    }
    else
    {
        p_db_arpinsp->forwarded++;
    }

    ARPINSP_EVENT_DEBUG("Check pass an ARP packet on port %s \n", ifname_ext);
    return ARPINSP_SUCCESS;
}

