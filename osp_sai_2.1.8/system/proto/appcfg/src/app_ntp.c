#include <assert.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#include "proto.h"
#include "appcfg.h"
#include "version.h"
#include "bsd_queue.h"
#include "app_ntp.h"
#include "prefix.h"
#include "param_check.h"


#include "switch.h"
#include "glb_l2_define.h"
#include "gen/tbl_ntp_server_define.h"
#include "gen/tbl_ntp_key_define.h"
#include "gen/tbl_ntp_ace_define.h"
#include "gen/tbl_ntp_cfg_define.h"
#include "gen/tbl_ntp_server.h"
#include "gen/tbl_ntp_key.h"
#include "gen/tbl_ntp_ace.h"
#include "gen/tbl_ntp_cfg.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"

/* Execute command in child process. */
static int32
execute_ntp_command()
{
    sal_system("ntpd -U 30 -c /etc/ntp.conf -p /var/run/ntpd.pid -g -l /dev/null");
    sal_task_sleep(100);
    return 0;
}

/* ntp write config */
void __ntp_write_config_with_cdb()
{
    ctclib_slistnode_t      *listnode   = NULL; 
    tbl_ntp_server_t        *server     = NULL;
    tbl_ntp_server_master_t *pntp_master = NULL;
    tbl_ntp_key_t           *ntp_key     = NULL;
    tbl_ntp_key_master_t    *pntp_key_master = NULL;
    tbl_ntp_cfg_t           *pntp_config     = NULL;
    
    tbl_interface_master_t  *p_master   = tbl_interface_get_master();
    tbl_interface_t         *p_db_if    = NULL;

    char ntp_cmd_buf[128]                = {0};
    char _tmp[64]                        = {0};

    FILE * savefile = sal_fopen(NTP_CONFIG_FILE, SAL_OPEN_RW);

    pntp_config = tbl_ntp_cfg_get_ntp_cfg();

    sal_sprintf(ntp_cmd_buf, "%s\r\n","# default acl");
    sal_fprintf(savefile,ntp_cmd_buf);
    //sal_sprintf(ntp_cmd_buf, "%s\r\n", "restrict default nomodify notrap noquery");
    sal_sprintf(ntp_cmd_buf, "%s\r\n", "restrict 127.0.0.1");
    sal_fprintf(savefile,ntp_cmd_buf);
    sal_sprintf(ntp_cmd_buf, "%s\r\n", "restrict -6 ::1");
    sal_fprintf(savefile,ntp_cmd_buf);
    sal_sprintf(ntp_cmd_buf, "%s %f\r\n", "broadcastdelay ",
                                pntp_config->bcastDelay == 0 ? 
                                    3.0f :(float)pntp_config->bcastDelay/1000.0f);
    sal_fprintf(savefile,ntp_cmd_buf);
    sal_sprintf(ntp_cmd_buf, "%s\r\n", "keys /etc/ntp/keys");
    sal_fprintf(savefile,ntp_cmd_buf);
    sal_sprintf(ntp_cmd_buf, "%s\r\n", "trustedkey 64567");
    sal_fprintf(savefile,ntp_cmd_buf);
    sal_sprintf(ntp_cmd_buf, "%s\r\n", "requestkey 64567");
    sal_fprintf(savefile,ntp_cmd_buf);
    sal_sprintf(ntp_cmd_buf, "%s\r\n", "driftfile /etc/ntp/drift");
    sal_fprintf(savefile,ntp_cmd_buf);
    sal_sprintf(ntp_cmd_buf, "%s\r\n", "enable mode7");
    sal_fprintf(savefile,ntp_cmd_buf);
//    sal_sprintf(ntp_cmd_buf, "%s\r\n", "disable ntp");
//    sal_fprintf(savefile,ntp_cmd_buf);
    sal_sprintf(ntp_cmd_buf, "%s\r\n", "enable bclient");
    sal_fprintf(savefile,ntp_cmd_buf);

    if(0 == pntp_config->authentication)
    {
        sal_sprintf(ntp_cmd_buf, "%s\r\n", "disable auth");
        sal_fprintf(savefile,ntp_cmd_buf);
    }else{
		sal_sprintf(ntp_cmd_buf, "%s\r\n", "enable auth");
        sal_fprintf(savefile,ntp_cmd_buf);
	}
    
    sal_sprintf(ntp_cmd_buf, "%s\r\n", "tinker panic 0");
    sal_fprintf(savefile,ntp_cmd_buf);
//    sal_sprintf(ntp_cmd_buf, "%s\r\n", "tinker step 0");
//    sal_fprintf(savefile,ntp_cmd_buf);
//    sal_sprintf(ntp_cmd_buf, "%s\r\n", "tinker stepout 0");
//    sal_fprintf(savefile,ntp_cmd_buf);
      
    pntp_master = tbl_ntp_server_get_master();
    CTCLIB_SLIST_LOOP(pntp_master->ntp_server_list, server, listnode)
    {
        if (server->type == M_NTP_TYPE_UCAST_SERVER)
        {
            
            //sal_sprintf(_tmp,"server %s iburst ", server->key.host);
            if(server->keyid != 0)
            {
                if ((server->version != 0)&&(server->prefer == 0))
                {   
                    sal_sprintf(_tmp,"server %s iburst key %d version %d", server->key.host,server->keyid,server->version);
                }
                else if ((server->version == 0)&&(server->prefer != 0))
                {
                    sal_sprintf(_tmp,"server %s iburst key %d prefer %d", server->key.host,server->keyid,server->prefer);
                }
                else if ((server->version != 0)&&(server->prefer != 0))
                {
                    sal_sprintf(_tmp,"server %s iburst key %d version %d prefer %d", server->key.host,server->keyid,server->version,server->prefer);
                }
                else
                {
                    sal_sprintf(_tmp,"server %s iburst key %d", server->key.host,server->keyid);
                }
                


                //sal_sprintf(_tmp,"server %s iburst key %d", server->key.host,server->keyid);
                //sal_sprintf(_tmp + sal_strlen(ntp_cmd_buf)," key %d", server->keyid);
                //sal_strcat(ntp_cmd_buf, _tmp);
            }
            else
            {
                if ((server->version != 0)&&(server->prefer == 0))
                {   
                    sal_sprintf(_tmp,"server %s iburst version %d", server->key.host,server->version);
                }
                else if ((server->version == 0)&&(server->prefer != 0))
                {
                    sal_sprintf(_tmp,"server %s iburst prefer %d", server->key.host,server->prefer);
                }
                else if ((server->version != 0)&&(server->prefer != 0))
                {
                    sal_sprintf(_tmp,"server %s iburst version %d prefer %d", server->key.host,server->version,server->prefer);
                }
                else
                {
                    sal_sprintf(_tmp,"server %s iburst ", server->key.host);
                }

            }

            /*if(server->version != 0)
            {
                sal_sprintf(_tmp + sal_strlen(ntp_cmd_buf)," version %d", server->version);
                sal_strcat(ntp_cmd_buf, _tmp);
            }
            if(server->prefer != 0)
            {
                sal_sprintf(_tmp + sal_strlen(ntp_cmd_buf)," prefer");
                sal_strcat(ntp_cmd_buf, _tmp);
            }*/
            sal_strcpy(ntp_cmd_buf, _tmp);
            sal_fprintf(savefile,"%s\r\n", ntp_cmd_buf);
        }
    }

    /* write trustedkey */
    pntp_key_master = tbl_ntp_key_get_master();
    CTCLIB_SLIST_LOOP(pntp_key_master->ntp_key_list, ntp_key, listnode)
    {
        if(ntp_key->trusted)
        {
            sal_fprintf(savefile, "trustedkey %u\r\n", ntp_key->key.keyid);
        }
    }

    /* write interface disable */
    sal_sprintf(ntp_cmd_buf, "%s\r\n", "interface listen *");
    sal_fprintf(savefile,ntp_cmd_buf);
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if(p_db_if->ntp_disable)
        {
            sal_sprintf(ntp_cmd_buf, "interface drop %s\r\n", p_db_if->key.name);
            sal_fprintf(savefile,ntp_cmd_buf);
        }
    }

    sal_fclose(savefile);
}

/* ntp write config */
void __ntp_update_config_and_restart()
{
    ntp_save_keypair();
    __ntp_write_config_with_cdb();

    sal_system("killall ntpd");
    sal_system("rm -rf /var/run/ntpd.pid");
    //sal_system("ntpd -U 30 -c /etc/ntp.conf -p /var/run/ntpd.pid -g -l /dev/null &");
    execute_ntp_command();
}

/*interface status*/
uint32 g_app_ntp_ifstatus[M_NTP_MAX_INTERFACE_CNT];

int ntp_config_set_If_status(int ifnum, int flag)
{
    if (ifnum >= M_NTP_MAX_INTERFACE_CNT)
    {
        return -1;
    }

    g_app_ntp_ifstatus[ifnum] |= flag;

    return 0;
}

int ntp_config_chk_if_status(int ifnum, int flag)
{
    if (ifnum >= M_NTP_MAX_INTERFACE_CNT)
    {
        return -1;
    }

    if (g_app_ntp_ifstatus[ifnum] & (~flag))
    {
        return -1;
    }

    return 0;
}

int ntp_config_clr_If_status( int ifnum, int flag)
{
    if (ifnum >= M_NTP_MAX_INTERFACE_CNT)
    {
        return -1;
    }

    g_app_ntp_ifstatus[ifnum] &= ~flag;

    return 0;
}

int32
_app_static_dns_get_ip_by_hostname(char *hostname, char *ip_addr);

static char * restrict_option_name[] =
{
    "version",
    "kod",
    "ignore",
    "noquery",
    "nomodify",
    "notrap",
    "lowpriotrap",
    "noserve",
    "nopeer",
    "notrust",
    "limited",
    "ntpport"
};


char *
ntp_get_resopt_name(int32 i)
{
    if (i >= 0 && i < BITOFFSET_END)
    {
        return restrict_option_name[i];
    }

    sal_assert(0);
    return "unknown";
}

int check_netserverip(const char *hostip)
{
 unsigned int part_first;
 unsigned int part_second;
 unsigned int part_third;
 unsigned int part_fourth;
 
 if (NULL == hostip)
 {
     return -1;
 }
 if (sal_strlen(hostip) < 7  || sal_strlen(hostip) > 15)
 {
     return -1;
 }
 
if (sal_strspn(hostip, "0123456789.") != sal_strlen(hostip))
{
    return -1;
}
 if (4 != sal_sscanf(hostip, "%d.%d.%d.%d", &part_first, &part_second, &part_third, &part_fourth))
     return -1;
 if((part_first==0) && (part_second==0)&&(part_third==0) &&(part_fourth==0))
 {
     return -1;
 }
 if (part_first < 0 || part_first > 255 || part_second < 0 || part_second > 255
     || part_third < 0 || part_third > 255 || part_fourth < 0 || part_fourth > 255)
 {
     return -1;
 }
 return 0;
 

}

int check_ntpserverip(const char *pszHostname)
{
    if (!check_hostname(pszHostname))
    {
        return 0;
    }
    if (!check_netserverip(pszHostname))
    {
        return 0;
    }
    
    if (!check_ipv6(pszHostname))
    {
        return 0;
    }

    return -1;
}

int32 
check_keyid_in_used(uint32 keyid)
{
    ctclib_slistnode_t * listnode = NULL; 
    tbl_ntp_server_t        *server = NULL;
    tbl_ntp_server_master_t *server_master = tbl_ntp_server_get_master();
    
    if (server_master==NULL)
    {
        return PM_E_NOT_PERMIT;
    }
    
    CTCLIB_SLIST_LOOP(server_master->ntp_server_list, server, listnode)
    {
        if(keyid == server->keyid)
        {
            return 0;
        }
    }

    return -1;

}

/*
 * check ntp key is valid?
 * return: 0 -> valid;
 *        -1 -> invalid; 
 */
int32
check_ntp_key_valid(char* value)
{
    int32       key_len = 0;
    char        *strkey  = NULL;
    size_t	    j;
    static const char hex[] = "0123456789abcdef";
	char	    *ptr;

    strkey = value;

    /* 1: check char is valid ? */
    while ('\0' != *strkey)
    {
        if (!sal_char_isprint(*strkey))
        {
            return -1;
        }
        strkey++;
    }

    /* 1: check string is hex ? */
    key_len = sal_strlen(value);
    strkey  = value;
    if (key_len > 20) {
		for (j = 0; j < key_len; j++) {
			ptr = strchr(hex, sal_tolower((unsigned char)strkey[j]));
			if (ptr == NULL)
				break;	        /* abort decoding */
		}
        
		if (j < key_len) {
			return -1;
		}
    }

    return 0;
}

/* increment count for this type */
void
ntp_inc_count(tbl_ntp_cfg_t *this, int type)
{
    if (NULL == this)
    {
        sal_assert(0);
        return;
    }

    switch (type)
    {
    case M_NTP_TYPE_UCAST_SERVER:
        this->ucastServerCount++;
        break;
    case M_NTP_TYPE_UCAST_CLIENT:
        this->ucastClientCount++;
        break;
#if HAVE_NTP_MCAST_CLI        
    case M_NTP_TYPE_MCAST_SERVER:
        this->mcastServerCount++;
        break;
    case M_NTP_TYPE_MCAST_CLIENT:
        this->mcastClientCount++;
        break;
#endif
    case M_NTP_TYPE_BCAST_SERVER:
        this->bcastServerCount++;
        break;
    default:
        sal_assert(0);
        return;
    }

    return;
}



/* decrement count for this type */
void
ntp_dec_count(tbl_ntp_cfg_t *this, int type)
{
    if (NULL == this)
    {
        sal_assert(0);
        return;
    }

    switch (type)
    {
    case M_NTP_TYPE_UCAST_SERVER:
        this->ucastServerCount--;
        break;
    case M_NTP_TYPE_UCAST_CLIENT:
        this->ucastClientCount--;
        break;
#if HAVE_NTP_MCAST_CLI        
    case M_NTP_TYPE_MCAST_SERVER:
        this->mcastServerCount--;
        break;
    case M_NTP_TYPE_MCAST_CLIENT:
        this->mcastClientCount--;
        break;
#endif
    case M_NTP_TYPE_BCAST_SERVER:
        this->bcastServerCount--;
        break;
    /* modified by liwh for bug 49775, 2018-06-05 */
    case M_NTP_TYPE_UCAST_SERVER_INBAND:
        this->ucastServerCount_inband--;
        break;
    case M_NTP_TYPE_UCAST_SERVER_OUTBAND:
        this->ucastServerCount_outband--;
        break;
    /* liwh end */
    default:
        sal_assert(0);
        return;
    }

    return;
}


/* check count overflow for this type */
int
ntp_count_overflow(tbl_ntp_cfg_t* this, int type)
{
    if (NULL == this)
    {
        sal_assert(0);
        return 1;
    }

    switch (type)
    {
    case M_NTP_TYPE_UCAST_SERVER:
        return (this->ucastServerCount >= M_NTP_MAX_UCASTSERVER_CNT);
    case M_NTP_TYPE_UCAST_CLIENT:
        return (this->ucastClientCount >= M_NTP_MAX_UCASTCLIENT_CNT);
#if HAVE_NTP_MCAST_CLI        
    case M_NTP_TYPE_MCAST_SERVER:
        return (this->mcastServerCount >= M_NTP_MAX_MCASTSERVER_CNT);
    case M_NTP_TYPE_MCAST_CLIENT:
        return (this->mcastClientCount >= M_NTP_MAX_MCASTCLIENT_CNT);
#endif
    case M_NTP_TYPE_BCAST_SERVER:
        return (this->bcastServerCount >= M_NTP_MAX_BCASTSERVER_CNT);
    default:
        sal_assert(0);
        return 1;
    }

    return 0;
}


int32
ntp_get_ifindex(char *ifname, int32 *pifnum)
{
    int module = -1;
    int port = -1;

    if (0 == sal_strncmp (ifname, "eth-", 4))
    {
        sal_sscanf(ifname, "eth-%u-%u", &module, &port);
        port = module * M_NTP_MAX_ETH_IF_CNT_ONE_SLOT + port;
    }
    else if (0 == sal_strncmp (ifname, "lo", 2))
    {
        sal_sscanf(ifname, "loopback%u", &port);
        port = port + M_NTP_MAX_ETH_IF_CNT;
    }
    else if (0 == sal_strncmp (ifname, "vlan", 4))
    {
        sal_sscanf(ifname, "vlan%u", &port);
        port = port + M_NTP_MAX_ETH_IF_CNT + M_NTP_MAX_LOOPBACK_IF_CNT;
    }
    else
    {
        return PM_E_INVALID_IFNAME;
    }

    *pifnum = port;
    return 0;
}


/*******************************************************************************
 * Purpose: add a ntp unicast server address
 ******************************************************************************/
int 
ntp_add_ucast_server(cfg_cmd_para_t *para, char *hostname, uint32 keyid, int version, int prefer)
{
    tbl_ntp_server_t *p_ntp_server = NULL;
    tbl_ntp_server_t *p1_ntp_server = NULL;
    tbl_ntp_server_t ntp_server;
    tbl_ntp_cfg_t* this = NULL;
    tbl_ntp_key_key_t ntp_key;
    //char ipBuf[NTP_MAX_HOST_IP_SIZE];
    char *ip = (char *)hostname;
    int32 rc = PM_E_NONE;

    /*save configuration to local db*/

    if ( check_ntpserverip(hostname) )  
    {
        return PM_E_INVALID_PARAM;
    }

    
    this = tbl_ntp_cfg_get_ntp_cfg();
    if(NULL == this)
    {
        return PM_E_NOT_INITED;
    }
    
    
    if (keyid != 0)
    {
        ntp_key.keyid = keyid;
        if(NULL == tbl_ntp_key_get_ntp_key(&ntp_key))
        {
            CFG_INVALID_PARAM_RET("No such key");
        }
    }

    if(ctclib_slist_count(tbl_ntp_server_get_master()->ntp_server_list) >= NTP_SERVER_MAX)
    {
	return PM_E_EXCEED_MAX_NUM;
    }
    
    /*save config to cdb*/
    sal_memset(&ntp_server, 0, sizeof(ntp_server));
    p_ntp_server = &ntp_server;
    p_ntp_server->keyid = keyid;
    p_ntp_server->prefer = prefer;
    p_ntp_server->version = version;
    p_ntp_server->type = M_NTP_TYPE_UCAST_SERVER;

    /* modified by liwh for bug 49264, 2018-09-26 
         app_ntp_server_cmd_process check ip and ipv6 address in app_get_ip_str2val */
    /*if (check_netserverip(hostname))
    {
        if (_app_static_dns_get_ip_by_hostname((char *)hostname, ipBuf))
        {
            CFG_CONFLICT_RET("Error: host name doesn't exist");
        }
        ip = ipBuf;
    }*/
    /* liwh end */
    sal_memcpy(p_ntp_server->key.host, ip, NTP_MAX_HOST_IP_SIZE);
    p1_ntp_server = tbl_ntp_server_get_ntp_server(&p_ntp_server->key);

    if (NULL != p1_ntp_server)
    {
        CFG_CONFLICT_RET("Error: server exist, please delete first");
        return PM_E_INVALID_PARAM;
    }
    else
    {
        if (ntp_count_overflow(this, M_NTP_TYPE_UCAST_SERVER))
        {
            return PM_E_EXCEED_MAX_NUM;
        }

        rc = tbl_ntp_server_add_ntp_server(p_ntp_server);
        if(rc)
        {
            return rc;
        }
        
        ntp_inc_count(this, M_NTP_TYPE_UCAST_SERVER);
    }
#if 0    
    /*running config to ntpd*/
    sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"addserver %s iburst", hostname);
    if (keyid != 0)
    {
        sal_snprintf(szTmp, M_NTP_CMD_BUFSZ, " keyid %d", keyid);
        sal_strcat(szCmd, szTmp);
    }
    if (version != 0)
    {
        sal_snprintf(szTmp, M_NTP_CMD_BUFSZ, " version %d", version);
        sal_strcat(szCmd, szTmp);
    }
    if (prefer != 0)
    {
        sal_strcat(szCmd, " prefer");
    }
    sal_strcat(szCmd, "\"");
#endif        
    __ntp_update_config_and_restart();
#ifndef _GLB_UML_SYSTEM_
#if 0
    if (0 != ntp_config_exec( M_NTP_LOCALHOST_IP, szCmd, 0))
    {
        tbl_ntp_server_del_ntp_server(&p_ntp_server->key);
        ntp_dec_count(this,M_NTP_TYPE_UCAST_SERVER);
        
        return PM_E_FAIL;
    }
#endif
#endif
    return rc;
}

int32
ntp_del_ucast_server(cfg_cmd_para_t *para, char *hostname, char *inband)
{
#ifndef _GLB_UML_SYSTEM_
    /* modified by liwh for bug 51533, 2019-04-10 */
    //char szCmd[M_NTP_CMD_BUFSZ];
    /* liwh end */
#endif
    tbl_ntp_server_t *p_ntp_server = NULL;
    tbl_ntp_server_t ntp_server;
    //char ipBuf[NTP_MAX_HOST_IP_SIZE];
    tbl_ntp_cfg_t* this = NULL;
    tbl_ntp_server_t *p1_ntp_server = NULL;
    uint32 is_inband = FALSE;
    int32 rc = PM_E_NONE;
    addr_t             ip_addr;

    sal_memset(&ip_addr, 0, sizeof(addr_t));

    if (check_ntpserverip(hostname))
    {
        return PM_E_INVALID_PARAM;
    }

    if (NULL == hostname)
    {
        return PM_E_INVALID_PARAM;
    }

    this = tbl_ntp_cfg_get_ntp_cfg();
    if (NULL == this)
    {
        return PM_E_NOT_INITED;
    }
    
    sal_memset(&ntp_server, 0, sizeof(ntp_server));
    p_ntp_server = &ntp_server;

    /* modified by liwh for bug 49264, 2018-09-26 */
    rc = app_get_ip_str2val((char *)hostname, &ip_addr);
    if (PM_E_NONE != rc)
    {
         CFG_INVALID_PARAM_RET("Please check the ip address of ntp server!");    
    }
    
    /*if (check_netserverip(hostname))
    {
        if (_app_static_dns_get_ip_by_hostname((char *)hostname, ipBuf))
            return PM_E_INVALID_PARAM;
        sal_memcpy(&p_ntp_server->key.host, ipBuf, NTP_MAX_HOST_IP_SIZE);
    }
    else
    {
        
    }*/
    sal_strncpy(p_ntp_server->key.host, hostname, NTP_MAX_HOST_IP_SIZE);
    /* liwh end */
    
    p1_ntp_server = tbl_ntp_server_get_ntp_server(&p_ntp_server->key);

    if (NULL == p1_ntp_server || M_NTP_TYPE_UCAST_SERVER != p1_ntp_server->type)
    {
        CFG_CONFLICT_RET("No such item exist");
    }

    is_inband = (0 == sal_strcmp("inband", inband)) ? TRUE : FALSE;
    if (is_inband != p1_ntp_server->is_inband)
    {
        CFG_CONFLICT_RET("No such item exist");
    }

    /* modified by liwh for bug 49775, 2018-06-05 */
    if (p1_ntp_server->is_inband)
    {
        ntp_dec_count(this, M_NTP_TYPE_UCAST_SERVER_INBAND);    
    }
    else
    {
        ntp_dec_count(this, M_NTP_TYPE_UCAST_SERVER_OUTBAND);
    }
    /* liwh end */
    
    ntp_dec_count(this, M_NTP_TYPE_UCAST_SERVER);
    tbl_ntp_server_del_ntp_server(&p_ntp_server->key);

    __ntp_update_config_and_restart();
#if 0    
    sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"unconfig %s\"", hostname);
#endif
#ifdef _GLB_UML_SYSTEM_
    return 0;
#else
    /* modified by liwh for bug 51533, 2019-04-10 
         szCmd is uninitizlized, shouldn't execute it by system */
    //return ntp_config_exec(M_NTP_LOCALHOST_IP, szCmd, 0);
    return 0;
    /* liwh end */
#endif
}



int32
ntp_get_ucast_server(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_ntp_server_iterate((TBL_ITER_CB_FUNC)tbl_ntp_server_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}



/*******************************************************************************
 * Purpose: add a ntp unicast peer address
 ******************************************************************************/
int
ntp_add_ucast_peer(cfg_cmd_para_t *para, char *hostname, uint32 keyid, int version, int prefer)
{

    char szCmd[M_NTP_CMD_BUFSZ];
    char szTmp[M_NTP_CMD_BUFSZ];
    tbl_ntp_server_t *p_ntp_server = NULL;
    tbl_ntp_server_t *p1_ntp_server = NULL;
    tbl_ntp_server_t ntp_server;
    tbl_ntp_key_key_t ntp_key;
    tbl_ntp_cfg_t* this = NULL;
    char ipBuf[NTP_MAX_HOST_IP_SIZE];
    char *ip = (char *)hostname;
    int32 rc = PM_E_NONE;

    /*save configuration to local db*/

    if ( check_ntpserverip(hostname) )  
    {
        return PM_E_INVALID_PARAM;
    }

    
   this = tbl_ntp_cfg_get_ntp_cfg();
   if(NULL == this)
   {
        return PM_E_NOT_INITED;
   }
    if (keyid != 0)
    {
        ntp_key.keyid = keyid;
        if(NULL == tbl_ntp_key_get_ntp_key(&ntp_key))
        {
            return PM_E_NOT_PERMIT;
        }
    }

    if(ctclib_slist_count(tbl_ntp_server_get_master()->ntp_server_list) >= NTP_SERVER_MAX)
    {
	return PM_E_EXCEED_MAX_NUM;
    }

    /*save config to cdb*/
    sal_memset(&ntp_server, 0, sizeof(ntp_server));
    p_ntp_server = &ntp_server;
    p_ntp_server->keyid = keyid;
    p_ntp_server->prefer = prefer;
    p_ntp_server->version = version;
    p_ntp_server->type = M_NTP_TYPE_UCAST_CLIENT;
    if (check_netserverip(hostname))
    {
        if (_app_static_dns_get_ip_by_hostname((char *)hostname, ipBuf))
        {
            CFG_CONFLICT_RET("Error: host name doesn't exist");
        }
        ip = ipBuf;
    }
    sal_memcpy(&p_ntp_server->key.host, ip, NTP_MAX_HOST_IP_SIZE);
    sal_memcpy(p_ntp_server->key.host, ip, NTP_MAX_HOST_IP_SIZE);

    
    p1_ntp_server = tbl_ntp_server_get_ntp_server(&p_ntp_server->key);

    if (NULL != p1_ntp_server)
    {
        if (M_NTP_TYPE_UCAST_CLIENT != p1_ntp_server->type)
        {
            return PM_E_INVALID_PARAM;
        }
    }
    else
    {
        if (ntp_count_overflow(this, M_NTP_TYPE_UCAST_CLIENT))
        {
            return PM_E_EXCEED_MAX_NUM;
        }

        rc = tbl_ntp_server_add_ntp_server(p_ntp_server);
        if(rc)
        {
            return rc;
        }
        
        ntp_inc_count(this, M_NTP_TYPE_UCAST_CLIENT);
    }
   

    sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"addpeer %s iburst", hostname);
    if (keyid != 0)
    {
        sal_snprintf(szTmp, M_NTP_CMD_BUFSZ, " keyid %d", keyid);
        sal_strcat(szCmd, szTmp);
    }
    if (version != 0)
    {
        sal_snprintf(szTmp, M_NTP_CMD_BUFSZ, " version %d", version);
        sal_strcat(szCmd, szTmp);
    }
    if (prefer != 0)
    {
        sal_strcat(szCmd, " prefer");
    }
    sal_strcat(szCmd, "\"");
        
#ifndef _GLB_UML_SYSTEM_
    if (PM_E_NONE != ntp_config_exec(M_NTP_LOCALHOST_IP, szCmd, 0))
    {
        tbl_ntp_server_del_ntp_server(&p_ntp_server->key);
        ntp_dec_count(this,M_NTP_TYPE_UCAST_SERVER);
        return PM_E_FAIL;
    }
#else
    (void)szCmd;
#endif

    return rc;
}

int32
ntp_del_ucast_peer(cfg_cmd_para_t *para, char *hostname, char *inband)
{
    char szCmd[M_NTP_CMD_BUFSZ];
    tbl_ntp_server_t *p_ntp_server = NULL;
    tbl_ntp_server_t ntp_server;
    char ipBuf[NTP_MAX_HOST_IP_SIZE];
    char *ip = (char *)hostname;
    tbl_ntp_cfg_t* this = NULL;
    tbl_ntp_server_t *p1_ntp_server = NULL;
    uint32 is_inband = FALSE;

    if (check_ntpserverip(hostname))
    {
        return PM_E_INVALID_PARAM;
    }

    
    if (NULL == hostname)
    {
        return PM_E_INVALID_PARAM;
    }

    this = tbl_ntp_cfg_get_ntp_cfg();
    if (NULL == this)
    {
        return PM_E_NOT_INITED;
    }
    
    if (check_netserverip(hostname))
    {
        if (_app_static_dns_get_ip_by_hostname((char *)hostname, ipBuf))
        {
            return PM_E_NOT_EXIST;
        }
        ip = ipBuf;
    }
    
    sal_memset(&ntp_server, 0, sizeof(ntp_server));
    p_ntp_server = &ntp_server;
    sal_memcpy(&p_ntp_server->key.host, ip, NTP_MAX_HOST_IP_SIZE);

    p1_ntp_server = tbl_ntp_server_get_ntp_server(&p_ntp_server->key);

    if (NULL == p1_ntp_server || M_NTP_TYPE_UCAST_CLIENT != p1_ntp_server->type)
    {
        CFG_CONFLICT_RET("No such item exist");
    }
    
    is_inband = (0 == sal_strcmp("inband", inband)) ? TRUE : FALSE;
    if (is_inband != p1_ntp_server->is_inband)
    {
        CFG_CONFLICT_RET("No such item exist");
    }

    ntp_dec_count(this, M_NTP_TYPE_UCAST_CLIENT);
    tbl_ntp_server_del_ntp_server(&p_ntp_server->key);

    sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"unconfig %s\"", hostname);
#ifdef _GLB_UML_SYSTEM_
    (void)szCmd;
    return 0;
#else
    return ntp_config_exec(M_NTP_LOCALHOST_IP, szCmd, 0);
#endif
}



int ntp_add_trustedkey(cfg_cmd_para_t *para, uint32 keyid)
{
    int32 rc = PM_E_NONE;
    tbl_ntp_key_t* key = NULL;
    tbl_ntp_key_key_t ntp_key;
  
    if (keyid < 1 || keyid > NTP_MAX_KEY_VALUE)
    {
        return PM_E_INVALID_PARAM;
    }
    ntp_key.keyid = keyid;
    key = tbl_ntp_key_get_ntp_key(&ntp_key);
    if (NULL == key)
    {
        return PM_E_NOT_EXIST;
    }

    key->trusted = TRUE;

    rc = tbl_ntp_key_set_ntp_key_field(key, TBL_NTP_KEY_FLD_TRUSTED);

    if(!rc)
    {
        __ntp_update_config_and_restart();
#if 0
        sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"trustedkey %u\"", keyid);
        ntp_config_exec(M_NTP_LOCALHOST_IP, szCmd, 0);
#endif
    }
    
    return PM_E_NONE;
}


int 
ntp_del_trustedkey(cfg_cmd_para_t *para, uint32 keyid)
{
    tbl_ntp_key_t* key = NULL;
    tbl_ntp_key_key_t ntp_key;

    if (keyid < 1 || keyid > NTP_MAX_KEY_VALUE)
    {
        return PM_E_INVALID_PARAM;
    }
    ntp_key.keyid = keyid;
    key = tbl_ntp_key_get_ntp_key(&ntp_key);
    if (NULL == key)
    {
        return PM_E_NOT_EXIST;
    }

    key->trusted = FALSE;
    tbl_ntp_key_set_ntp_key_field(key, TBL_NTP_KEY_FLD_TRUSTED);

    __ntp_update_config_and_restart();

    return PM_E_NONE;
}


int32
ntp_get_trustedkey(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_ntp_key_iterate((TBL_ITER_CB_FUNC)tbl_ntp_key_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32 
ntp_save_keypair()
{
    FILE * savefile = sal_fopen(NTP_KEY_FILE, SAL_OPEN_RW);
    ctclib_slistnode_t * listnode = NULL; 
    tbl_ntp_key_t *key = NULL;
    tbl_ntp_key_master_t* key_master = tbl_ntp_key_get_master();
    if (savefile==NULL || key_master==NULL)
    {
        if (NULL != savefile)
        {
            sal_fclose(savefile);
        }
        return PM_E_NOT_PERMIT;
    }
    
    sal_fprintf(savefile, "%d M %s\n", NTP_DEFAULT_REQUEST_KEY_NO,
                NTP_DEFAULT_REQUEST_KEY);
    
    CTCLIB_SLIST_LOOP(key_master->ntp_key_list, key, listnode)
    {
        sal_fprintf(savefile, "%u M %s\n", key->key.keyid, key->value);
    }

    sal_fclose(savefile);
    return PM_E_NONE;

}

int32
ntp_add_keypair(cfg_cmd_para_t *para, uint32 keyid, char* value)
{
    tbl_ntp_key_t newkey;
    tbl_ntp_key_t *key = NULL;
    
    int32 rc = PM_E_NONE;
    
    if (value == NULL)
    {
        return PM_E_INVALID_PARAM;
    }

    /* check ntp key valid */
    /*added by yejl to fix bug43885, 2017-06-12*/
    if(check_name_character_and_len(value, NTP_MAX_KEY_LEN-1))
    {
        CFG_INVALID_PARAM_RET("The first character should be a-z or A-Z or 0-9, character only can be 0-9A-Za-z.-_ and the max len is %d.", NTP_MAX_KEY_LEN-1);
        return PM_E_INVALID_PARAM;
    }
    /*ended by yejl*/

    sal_memset(&newkey, 0, sizeof(newkey));
    newkey.key.keyid = keyid;

    key = tbl_ntp_key_get_ntp_key(&newkey.key);
    if(key)
    {
        sal_snprintf(key->value, NTP_MAX_KEY_LEN,value);
        rc = tbl_ntp_key_set_ntp_key_field(key,TBL_NTP_KEY_FLD_VALUE);
    }else{
        newkey.trusted = FALSE;
        sal_snprintf(newkey.value, NTP_MAX_KEY_LEN,value);
        rc = tbl_ntp_key_add_ntp_key(&newkey);
    }
    
    rc |= ntp_save_keypair();

    if(!rc)
    {
        __ntp_update_config_and_restart();
    }

    return rc;
}

int32
ntp_del_keypair(cfg_cmd_para_t *para, uint32 keyid, int32 need_restart)
{
    tbl_ntp_key_t newkey;
    int32 rc = PM_E_NONE;

    sal_memset(&newkey, 0, sizeof(newkey));
    newkey.key.keyid = keyid;

    if(!check_keyid_in_used(keyid))
    {
        CFG_INVALID_PARAM_RET("Key is in use");
    }

    rc = tbl_ntp_key_del_ntp_key(&newkey.key);
    if(rc)
    {
        return PM_E_NOT_EXIST;
    }
//    ntp_save_keypair();

    if (need_restart)
    {
        __ntp_update_config_and_restart();
    }

    return rc;
}

int32
ntp_if_reload(cfg_cmd_para_t *para)
{
    tbl_ntp_cfg_t* ntp_cfg = tbl_ntp_cfg_get_ntp_cfg();

    if(!ntp_cfg)
    {
        return PM_E_NOT_INITED;
    }
    
    ntp_config_exec(M_NTP_LOCALHOST_IP, "ntpdc -nc \"ifreload\"", 0);

    ntp_cfg->ifreloadCount ++;

    return PM_E_NONE;
}


int32 
ntp_enable_refclock(cfg_cmd_para_t *para, int32 stratum)
{
    char szCmd[M_NTP_CMD_BUFSZ];
    tbl_ntp_cfg_t* ntp_cfg = tbl_ntp_cfg_get_ntp_cfg();

    if(!ntp_cfg)
    {
        return PM_E_NOT_INITED;
    }

    if(stratum == 0)
    {
        ntp_cfg->brefclock = 0;
        ntp_cfg->refclock_stratum = M_NTP_DEFAULT_REFCLOCK_STRATUM;

        tbl_ntp_cfg_set_ntp_cfg_field(ntp_cfg, TBL_NTP_CFG_FLD_BREFCLOCK);
        tbl_ntp_cfg_set_ntp_cfg_field(ntp_cfg, TBL_NTP_CFG_FLD_REFCLOCK_STRATUM);   

        sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, 
                     "ntpdc -nc \"unconfig %s\"",
                     M_NTP_REFCLOCK_ADDR);
        return ntp_config_exec(M_NTP_LOCALHOST_IP, szCmd, 0);
    }

    if (ntp_cfg->brefclock && stratum == ntp_cfg->refclock_stratum)
    {
        return PM_E_NONE;
    }
    
    ntp_cfg->brefclock = 1;
    ntp_cfg->refclock_stratum = stratum;

    tbl_ntp_cfg_set_ntp_cfg_field(ntp_cfg, TBL_NTP_CFG_FLD_BREFCLOCK);
    tbl_ntp_cfg_set_ntp_cfg_field(ntp_cfg, TBL_NTP_CFG_FLD_REFCLOCK_STRATUM);   

    sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, 
                 "ntpdc -nc \"addrefclock %s\" -c \"fudge %s val1 %d\"",
                 M_NTP_REFCLOCK_ADDR, M_NTP_REFCLOCK_ADDR, stratum);
    ntp_config_exec(M_NTP_LOCALHOST_IP, szCmd, 0);

    return PM_E_NONE;
}

int32 
ntp_set_broadcast_delay(cfg_cmd_para_t *para, int32 delay)
{
    tbl_ntp_cfg_t* ntp_cfg = tbl_ntp_cfg_get_ntp_cfg();
    
    if(!ntp_cfg)
    {
        return PM_E_NOT_INITED;
    }
    if(delay == 0 || delay == M_NTP_DEFAULT_BCASTDELAY)
    {
        /* In cdb record bcastdelay as 0, and use 3000 to instruct ntpdc. */
        delay = M_NTP_DEFAULT_BCASTDELAY;
        ntp_cfg->bcastDelay = 0;
    }
    else
    {
        ntp_cfg->bcastDelay = delay;
    }

    tbl_ntp_cfg_set_ntp_cfg_field(ntp_cfg, TBL_NTP_CFG_FLD_BCASTDELAY);

    __ntp_update_config_and_restart();

    return PM_E_NONE;
}

int32 
ntp_set_minimum_distance(cfg_cmd_para_t *para, int32 minimum_distance)
{
    char szCmd[M_NTP_CMD_BUFSZ];
    tbl_ntp_cfg_t* ntp_cfg = tbl_ntp_cfg_get_ntp_cfg();
    
    if(!ntp_cfg)
    {
        return PM_E_NOT_INITED;
    }
    if(minimum_distance == 0 || minimum_distance == M_NTP_MINIMUM_DISTANCE)
    {
        /* In cdb record minimum_distance as 0, and use 1 to instruct ntpdc. */
        minimum_distance = M_NTP_MINIMUM_DISTANCE;
        ntp_cfg->minimumDistance = 0;
    }
    else
    {
        ntp_cfg->minimumDistance = minimum_distance;
    }

    tbl_ntp_cfg_set_ntp_cfg_field(ntp_cfg, TBL_NTP_CFG_FLD_MINIMUMDISTANCE);

    sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"setmindist %u\"", minimum_distance);
    return ntp_config_exec(M_NTP_LOCALHOST_IP, szCmd, 0);

    return PM_E_NONE;
}

int32 
ntp_set_auth_enable(cfg_cmd_para_t *para, char* enable)
{
    tbl_ntp_cfg_t* ntp_cfg = tbl_ntp_cfg_get_ntp_cfg();
    
    if(!ntp_cfg)
    {
        return PM_E_NOT_INITED;
    }
    if(!sal_strcasecmp(enable, "enable"))
    {
        ntp_cfg->authentication = 1;
    }
    else if(!sal_strcasecmp(enable, "disable"))
    {
        ntp_cfg->authentication = 0;
    }
    else
    {
        return PM_E_INVALID_PARAM;
    }

    tbl_ntp_cfg_set_ntp_cfg_field(ntp_cfg, TBL_NTP_CFG_FLD_AUTHENTICATION);

    __ntp_update_config_and_restart();

    return PM_E_NONE;
}


int32 
ntp_clear_stats(cfg_cmd_para_t *para)
{
    tbl_ntp_cfg_t* ntp_cfg = tbl_ntp_cfg_get_ntp_cfg();
    
    if(!ntp_cfg)
    {
        return PM_E_NOT_INITED;
    }

    ntp_cfg->clearStats ++;
    
    ntp_config_exec(M_NTP_LOCALHOST_IP, "ntpdc -nc \"reset io\"", 0);
    return PM_E_NONE;
}

int32 
ntp_set_debug_level(cfg_cmd_para_t *para, char* level)
{
    tbl_ntp_cfg_t *p_db_cfg = tbl_ntp_cfg_get_ntp_cfg();
    char cmd_str[CMD_BUF_64];
    int32 debug_level = 2;
    
    if (0 == sal_strcmp("emerg", level))
    {
        debug_level = 0;
    }
    else if (0 == sal_strcmp("alert", level))
    {
        debug_level = 1;
    }
    else if (0 == sal_strcmp("crit", level))
    {
        debug_level = 2;
    }
    else if (0 == sal_strcmp("error", level))
    {
        debug_level = 3;
    }
    else if (0 == sal_strcmp("warning", level))
    {
        debug_level = 4;
    }
    else if (0 == sal_strcmp("notice", level))
    {
        debug_level = 5;
    }
    else if (0 == sal_strcmp("info", level))
    {
        debug_level = 6;
    }
    else if (0 == sal_strcmp("debug", level))
    {
        debug_level = 7;
    }
    else
    {
        return PM_E_INVALID_PARAM;
    }

    /* bypass same config */
    if (p_db_cfg->debug_level == debug_level)
    {
        return PM_E_NONE;
    }

    p_db_cfg->debug_level = debug_level;
    tbl_ntp_cfg_set_ntp_cfg_field(p_db_cfg, TBL_NTP_CFG_FLD_DEBUG_LEVEL);
    
    sal_system("killall ntpd");
    sal_system("rm -rf /var/run/ntpd.pid");
    sal_snprintf(cmd_str, CMD_BUF_64, "echo '%d' > /etc/ntp/ntpd.debug", debug_level);
    sal_system(cmd_str);
    execute_ntp_command();
    
    return PM_E_NONE;
}

int32 
ntp_interface_disable(cfg_cmd_para_t *para, char* ifname, int32 disable)
{
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if = NULL;
    int32 ifnum = 0;
    char ifname_ext[IFNAME_EXT_SIZE];

    if (ntp_get_ifindex(ifname, &ifnum))
    {
        CFG_INVALID_PARAM_RET("This command is only used on interface \"eth-*\" or \"loopback*\" or \"vlan*\"");
        return PM_E_INVALID_IFNAME;
    }
#if 0
    if (disable)
    {
        if (ntp_config_chk_if_status(ifnum, M_NTP_LOCK_INTERFACE))
        {
            CFG_CONFLICT_RET("NTP: Unconfigure other NTP commands on this interface before executing 'ntp disable'");
            return PM_E_CFG_CONFLICT;
        }

        if (ntp_config_set_If_status(ifnum, M_NTP_LOCK_INTERFACE))
        {
            CFG_CONFLICT_RET("NTP: Interface %s does not exist", IFNAME_ETH2FULL(ifname, ifname_ext));
            return PM_E_INVALID_PARAM;
        }
    }
    else
    {
        if (ntp_config_chk_if_status(ifnum, M_NTP_LOCK_INTERFACE))
        {
            CFG_CONFLICT_RET("NTP: Interface %s does not exist", IFNAME_ETH2FULL(ifname, ifname_ext));
            return PM_E_INVALID_PARAM;
        }
        if (ntp_config_clr_If_status(ifnum, M_NTP_LOCK_INTERFACE))
        {
            CFG_CONFLICT_RET("NTP: Interface %s does not exist", IFNAME_ETH2FULL(ifname, ifname_ext));
            return PM_E_INVALID_PARAM;
        }
    }
#endif
    sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s does not exist", IFNAME_ETH2FULL(ifname, ifname_ext));
        return PM_E_INVALID_IFNAME;
    }
#if 0    
    if(disable)
    {
        if(p_db_if->ntp_broadcast_client)
        {
            sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"setifstatus %d %d\"", ifnum, 
                M_NTP_LOCK_INTERFACE+M_NTP_BROADCAST_CLIENT);
        }
        else
        {
            sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"setifstatus %d %d\"", ifnum, M_NTP_LOCK_INTERFACE);
        }
    }
    else
    {
        if(p_db_if->ntp_broadcast_client)
        {
            sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"setifstatus %d %d\"", ifnum, M_NTP_BROADCAST_CLIENT);
        }
        else
        {
            sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"setifstatus %d %d\"", ifnum, 0);
        }
    }
#endif    
    if(p_db_if->ntp_disable == disable)
    {
        return 0;
    }

    p_db_if->ntp_disable = disable;
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_NTP_DISABLE);

    __ntp_update_config_and_restart();

    return PM_E_NONE;
}


int32 
ntp_interface_broadcast_client(cfg_cmd_para_t *para, char* ifname, int32 bclient)
{
    char szCmd[M_NTP_CMD_BUFSZ];
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if = NULL;
    int32 ifnum = 0;
    char ifname_ext[IFNAME_EXT_SIZE];

    if (ntp_get_ifindex(ifname, &ifnum))
    {
        CFG_INVALID_PARAM_RET("This command is only used on interface \"eth-*\" or \"loopback*\" or \"vlan*\"");
        return PM_E_INVALID_IFNAME;
    }

    if (bclient)
    {
        if (ntp_config_chk_if_status(ifnum, M_NTP_BROADCAST_CLIENT))
        {
            CFG_CONFLICT_RET("NTP: NTP is disabled on this interface");
            return PM_E_CFG_CONFLICT;
        }

        if (ntp_config_set_If_status(ifnum, M_NTP_BROADCAST_CLIENT))
        {
            CFG_CONFLICT_RET("NTP: Interface %s does not exist", IFNAME_ETH2FULL(ifname, ifname_ext));
            return PM_E_INVALID_PARAM;
        }
    }
    else
    {
        if (ntp_config_chk_if_status(ifnum, M_NTP_BROADCAST_CLIENT))
        {
            CFG_CONFLICT_RET("NTP: Interface %s does not exist", IFNAME_ETH2FULL(ifname, ifname_ext));
            return PM_E_INVALID_PARAM;
        }
        if (ntp_config_clr_If_status(ifnum, M_NTP_BROADCAST_CLIENT))
        {
            CFG_CONFLICT_RET("NTP: Interface %s does not exist", IFNAME_ETH2FULL(ifname, ifname_ext));
            return PM_E_INVALID_PARAM;
        }
    }

    sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s does not exist", IFNAME_ETH2FULL(ifname, ifname_ext));
        return PM_E_INVALID_IFNAME;
    }
    if(bclient)
    {
        if(p_db_if->ntp_disable)
        {
            sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"setifstatus %d %d\"", ifnum, 
                M_NTP_LOCK_INTERFACE+M_NTP_BROADCAST_CLIENT);
        }
        else
        {
            sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"setifstatus %d %d\"", ifnum, M_NTP_BROADCAST_CLIENT);
        }
    }
    else
    {
        if(p_db_if->ntp_disable)
        {
            sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"setifstatus %d %d\"", ifnum, M_NTP_LOCK_INTERFACE);
        }
        else
        {
            sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"setifstatus %d %d\"", ifnum, 0);
        }
    }

    p_db_if->ntp_broadcast_client = bclient;
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_NTP_BROADCAST_CLIENT);
    return ntp_config_exec(M_NTP_LOCALHOST_IP, szCmd, 0);
}


int32
ntp_get_global_cfg(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
    tbl_ntp_cfg_t* cfg = tbl_ntp_cfg_get_ntp_cfg();

    if(!cfg)
    {
        return -1;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_ntp_cfg_dump_one(cfg,&args);
    sal_fclose(fp);

    return PM_E_NONE;
}


/* access control list */
int32 
ntp_del_ace(cfg_cmd_para_t *para, int32 argc, char* argv[])
{
    int32 family = AF_INET;
    int32 masklen;
    int32 rc = PM_E_NONE;
    char maskstr[NTP_MAX_HOST_IP_SIZE];
    struct sal_in4_addr addr4;
    struct sal_in6_addr addr6;

    char szCmd[M_NTP_CMD_BUFSZ];
    char* hoststr = NULL;
    
    tbl_ntp_ace_t *pace = NULL;
    tbl_ntp_ace_t ace;
    tbl_ntp_cfg_t* this = tbl_ntp_cfg_get_ntp_cfg();


    if(NULL == this )
    {
        return PM_E_NOT_INITED;
    }

    if (this->aclCount >= M_NTP_MAX_ACL_CNT)
    {
        return PM_E_EXCEED_MAX_NUM;
    }

    if (argc < 1)
    {
        return PM_E_INVALID_PARAM;
    }

    if ( check_ntpserverip(argv[0]) )
    {
        return PM_E_INVALID_PARAM;
    }

    
    if (!sal_strcmp(argv[0], "127.0.0.1") || !sal_strcmp(argv[0], "0.0.0.0")
        || !sal_strcmp(argv[0], "::1") || !sal_strcmp(argv[0], "::"))   /* Added by Percy Wang for bug 18562, 2012-03-14 */
    {
        return PM_E_INVALID_PARAM;
    }

    /*get family*/
    if (1 == sal_inet_pton(AF_INET, argv[0], &addr4))
    {
        family = AF_INET;
    }
    else if (1 == sal_inet_pton(AF_INET6, argv[0], &addr6))
    {
        family = AF_INET6;
    }
    else
    {
        family = AF_INET;/*consider hostname always as ipv4, need think carefully later*/
    }

    /*get mask*/  
    if( argc>=3 && sal_strcmp(argv[1],"mask")==0 )
    {
        masklen = sal_atoi(argv[2]);
        if ((AF_INET == family && (masklen <= 0 || masklen >= 33))
            || (AF_INET6 == family && (masklen <= 0 || masklen >= 129)))
        {
            return PM_E_INVALID_PARAM;
        }                
    } 
    else
    {
        masklen = (AF_INET == family) ? 32 : 128;
    }
    
    if( argc>=3 && sal_strcmp(argv[1],"mask")==0 )
    {
        masklen = sal_atoi(argv[2]);
        if ((AF_INET == family && (masklen <= 0 || masklen >= 33))
            || (AF_INET6 == family && (masklen <= 0 || masklen >= 129)))
        {
            return PM_E_INVALID_PARAM;
        }    
        masklen2ipstr(family, masklen, maskstr);
    }
    else
    {
        if (AF_INET == family)
        {
            sal_strncpy(maskstr, M_NTP_FULLMASK_IP, NTP_MAX_HOST_IP_SIZE);
            masklen = 32;
        }
        else
        {
            sal_strncpy(maskstr, M_NTP_FULLMASK_IP6, NTP_MAX_HOST_IP_SIZE);
            masklen = 128;
        }
    }
    sal_memset(&ace, 0, sizeof(ace));
    sal_snprintf(ace.key.host, NTP_MAX_HOST_IP_SIZE, argv[0]);       
    sal_snprintf(ace.key.mask, NTP_MAX_HOST_IP_SIZE, maskstr);

    pace = tbl_ntp_ace_get_ntp_ace(&ace.key);
    if(pace == NULL)
    {
        return PM_E_NOT_EXIST;
    }
    
    if (AF_INET == pace->family)
    {
        sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"delrestrict %s %s\"",
                     pace->key.host, pace->key.mask);
    }
    else
    {
        sal_snprintf(szCmd, M_NTP_CMD_BUFSZ, "ntpdc -nc \"delrestrict -6 [%s] [%s]\"",
                     pace->key.host, pace->key.mask);
    }

    if (AF_INET == pace->family)
    {
        hoststr = M_NTP_LOCALHOST_IP;
    }
    else
    {
        hoststr = M_NTP_LOCALHOST_IP6;
    }
    
    ntp_config_exec(hoststr, szCmd, 0);
    rc = tbl_ntp_ace_del_ntp_ace(&ace.key);
    
    this->aclCount--;
    return rc;
}

/* access control list */
int32 
ntp_del_ace_option(cfg_cmd_para_t *para, int32 argc, char* argv[])
{
    int32 optpos;    
    int32 family = AF_INET;
    int32 masklen;
    char maskstr[NTP_MAX_HOST_IP_SIZE];
    struct sal_in4_addr addr4;
    struct sal_in6_addr addr6;
    char szCmd[M_NTP_CMD_BUFSZ];
    char *p = szCmd;
    int leftlen = M_NTP_CMD_BUFSZ;
    int rtv = 0;
    int i = 0;
    int idx = 0;
    
    tbl_ntp_ace_t *pace = NULL;
    tbl_ntp_ace_t ace;
    tbl_ntp_cfg_t* this = tbl_ntp_cfg_get_ntp_cfg();


    if(NULL == this)
    {
        return PM_E_NOT_INITED;
    }

    if (argc < 1)
    {
        return PM_E_INVALID_PARAM;
    }

    if ( check_ntpserverip(argv[0]) )
    {
        return PM_E_INVALID_PARAM;
    }

    if (!sal_strcmp(argv[0], "127.0.0.1") || !sal_strcmp(argv[0], "0.0.0.0")
        || !sal_strcmp(argv[0], "::1") || !sal_strcmp(argv[0], "::"))   /* Added by Percy Wang for bug 18562, 2012-03-14 */
    {
        return PM_E_INVALID_PARAM;
    }

    /*get family*/
    if (1 == sal_inet_pton(AF_INET, argv[0], &addr4))
    {
        family = AF_INET;
    }
    else if (1 == sal_inet_pton(AF_INET6, argv[0], &addr6))
    {
        family = AF_INET6;
    }
    else
    {
        family = AF_INET;/*consider hostname always as ipv4, need think carefully later*/
    }

    /*get mask*/
    sal_memset(maskstr, 0, NTP_MAX_HOST_IP_SIZE);
    if( argc>=3 && sal_strcmp(argv[1],"mask")==0 )
    {
        masklen = sal_atoi(argv[2]);
        if ((AF_INET == family && (masklen <= 0 || masklen >= 33))
            || (AF_INET6 == family && (masklen <= 0 || masklen >= 129)))
        {
            return PM_E_INVALID_PARAM;
        }    
        masklen2ipstr(family, masklen, maskstr);        
        optpos = 3;
    }
    else
    {
        optpos = 1;
        if (AF_INET == family)
        {
            sal_strncpy(maskstr, M_NTP_FULLMASK_IP, NTP_MAX_HOST_IP_SIZE);
            masklen = 32;
        }
        else
        {
            sal_strncpy(maskstr, M_NTP_FULLMASK_IP6, NTP_MAX_HOST_IP_SIZE);
            masklen = 128;
        }
    }

    sal_memset(&ace, 0, sizeof(ace));

    ace.family = family;
    ace.masklen = masklen;    
    sal_snprintf(ace.key.host, NTP_MAX_HOST_IP_SIZE, argv[0]);       
    sal_snprintf(ace.key.mask, NTP_MAX_HOST_IP_SIZE, maskstr);


    pace = tbl_ntp_ace_get_ntp_ace(&ace.key);
    if(NULL == pace)
    {
        return PM_E_NOT_EXIST;

    }


    if (AF_INET == pace->family)
    {
        rtv = sal_snprintf(p, leftlen, "ntpdc -nc \"unrestrict %s %s",
                           pace->key.host, pace->key.mask);
    }
    else
    {
        rtv = sal_snprintf(p, leftlen, "ntpdc -nc \"unrestrict -6 [%s] [%s]",
                           pace->key.host, pace->key.mask);
    }
    
    leftlen -= rtv;
    p += rtv;

    for (idx = optpos; idx < argc; idx++)
    {
        for (i = 0; i < BITOFFSET_END; i++)
        {
            if (!sal_strcmp(argv[idx], ntp_get_resopt_name(i)))
            {
                rtv = sal_snprintf(p, leftlen, " %s", argv[idx]);
                leftlen -= rtv;
                p += rtv;
            }
        }
    }
    
    sal_snprintf(p, leftlen, "\"");
    
    /* clear flag */
    for (idx = optpos; idx < argc; idx++)
    {
        for (i = 0; i < BITOFFSET_END; i++)
        {
            if (!sal_strcmp(argv[idx], ntp_get_resopt_name(i)))
            {
                pace->option &= (~(1 << i));
            }
        }
    }

    tbl_ntp_ace_set_ntp_ace_field(pace, TBL_NTP_ACE_FLD_OPTION);

    if (ntp_config_exec(M_NTP_LOCALHOST_IP, szCmd, 0) != PM_E_NONE)
    {
        return PM_E_NOT_PERMIT;
    }
        
    return PM_E_NONE;
}


/* access control list */
int32 
ntp_add_ace(cfg_cmd_para_t *para, int32 argc, char* argv[])
{
    int32 optpos;    
    int32 family = AF_INET;
    int32 masklen;
    int32 rc = PM_E_NONE;
    char maskstr[NTP_MAX_HOST_IP_SIZE];
    struct sal_in4_addr addr4;
    struct sal_in6_addr addr6;

    char szCmd[M_NTP_CMD_BUFSZ];
    char *p = szCmd;
    int leftlen = M_NTP_CMD_BUFSZ;
    int rtv = 0;
    int i = 0;
    char* hoststr = NULL;
    char ipBuf[NTP_MAX_HOST_IP_SIZE];
    
    tbl_ntp_ace_t *pace = NULL;
    tbl_ntp_ace_t ace;
    tbl_ntp_cfg_t* this = tbl_ntp_cfg_get_ntp_cfg();


    if(NULL == this)
    {
        return PM_E_NOT_INITED;
    }

    if (argc < 1)
    {
        return PM_E_INVALID_PARAM;
    }

    if ( check_ntpserverip(argv[0]) )
    {
        return PM_E_INVALID_PARAM;
    }

    /* check hostname valid*/
    if (check_netserverip(argv[0]))
    {
        if (_app_static_dns_get_ip_by_hostname((char *)argv[0], ipBuf))
        {
            CFG_CONFLICT_RET("Error: host name doesn't exist");
        }        
    }

    if (!sal_strcmp(argv[0], "127.0.0.1") || !sal_strcmp(argv[0], "0.0.0.0")
        || !sal_strcmp(argv[0], "::1") || !sal_strcmp(argv[0], "::"))   /* Added by Percy Wang for bug 18562, 2012-03-14 */
    {
        return PM_E_INVALID_PARAM;
    }

    /*get family*/
    if (1 == sal_inet_pton(AF_INET, argv[0], &addr4))
    {
        family = AF_INET;
    }
    else if (1 == sal_inet_pton(AF_INET6, argv[0], &addr6))
    {
        family = AF_INET6;
    }
    else
    {
        family = AF_INET;/*consider hostname always as ipv4, need think carefully later*/
    }

    /*get mask*/
    sal_memset(maskstr, 0, NTP_MAX_HOST_IP_SIZE);
    if( argc>=3 && sal_strcmp(argv[1],"mask")==0 )
    {
        masklen = sal_atoi(argv[2]);
        if ((AF_INET == family && (masklen <= 0 || masklen >= 33))
            || (AF_INET6 == family && (masklen <= 0 || masklen >= 129)))
        {
            return PM_E_INVALID_PARAM;
        }    
        masklen2ipstr(family, masklen, maskstr);        
        optpos = 3;
    }
    else
    {
        optpos = 1;
        if (AF_INET == family)
        {
            sal_strncpy(maskstr, M_NTP_FULLMASK_IP, NTP_MAX_HOST_IP_SIZE);
            masklen = 32;
        }
        else
        {
            sal_strncpy(maskstr, M_NTP_FULLMASK_IP6, NTP_MAX_HOST_IP_SIZE);
            masklen = 128;
        }
    }

    sal_memset(&ace, 0, sizeof(ace));

    ace.family = family;
    ace.masklen = masklen;    
    sal_snprintf(ace.key.host, NTP_MAX_HOST_IP_SIZE, argv[0]);       
    sal_snprintf(ace.key.mask, NTP_MAX_HOST_IP_SIZE, maskstr);


    for (; optpos<argc; optpos++)
    {
        int bit;
        for (bit=0; bit<BITOFFSET_END; bit++)
        {
            if (!sal_strcmp(argv[optpos], "none"))
            {
                ace.option = 0;
                break;
            }
            if ( sal_strcmp(argv[optpos], restrict_option_name[bit])==0 )
            {
                ace.option |= (1<<bit);
                break;
            }
        }
    }

    pace = tbl_ntp_ace_get_ntp_ace(&ace.key);
    if(pace)
    {
        pace->masklen = masklen;
        pace->family = family;
        pace->option = ace.option;
    }
    else
    {
        if (this->aclCount >= M_NTP_MAX_ACL_CNT)
        {
            return PM_E_EXCEED_MAX_NUM;
        }
        
        rc = tbl_ntp_ace_add_ntp_ace(&ace);
        if(rc)
        {
            return rc;
        }
        this->aclCount++;
        pace = &ace;
    }

    if (0 == pace->option)
    {
        if (AF_INET == pace->family)
        {
            rc = sal_snprintf(p, leftlen, "ntpdc -nc \"delrestrict %s %s\" "
                    "-c \"restrict %s %s ignore\" -c \"unrestrict %s %s ignore\"",
                    pace->key.host, pace->key.mask, pace->key.host, pace->key.mask, 
                    pace->key.host, pace->key.mask);
        }
        else
        {
            rtv = sal_snprintf(p, leftlen, "ntpdc -nc \"delrestrict -6 [%s] [%s]\" "
                    "-c \"restrict -6 [%s] [%s] ignore\" -c \"unrestrict -6 [%s] [%s] ignore\"",
                    pace->key.host, pace->key.mask, pace->key.host, pace->key.mask, 
                    pace->key.host, pace->key.mask);
        }
        
        leftlen -= rtv;
        p += rtv;
    }
    else
    {
        if (AF_INET == pace->family)
        {
            rtv = sal_snprintf(p, leftlen, "ntpdc -nc \"restrict %s %s",
                    pace->key.host, pace->key.mask);
        }
        else
        {
            rtv = sal_snprintf(p, leftlen, "ntpdc -nc \"restrict -6 [%s] [%s]",
                    pace->key.host, pace->key.mask);
        }
       
        leftlen -= rtv;
        p += rtv;
        
        for (i = 0; i < BITOFFSET_END; i++)
        {
            if (pace->option & (1 << i))
            {
                rtv = sal_snprintf(p, leftlen, " %s", ntp_get_resopt_name(i));
                leftlen -= rtv;
                p += rtv;
            }
        }
        sal_snprintf(p, leftlen, "\"");
    }

    if (AF_INET == pace->family)
    {
        hoststr = M_NTP_LOCALHOST_IP;
    }
    else
    {
        hoststr = M_NTP_LOCALHOST_IP6;
    }
    if (ntp_config_exec(hoststr, szCmd, 0) != PM_E_NONE)
    {
        tbl_ntp_ace_del_ntp_ace(&pace->key);
        this->aclCount--;
        return PM_E_IOCTL_FAIL;
    }

    return PM_E_NONE;
}



int32
ntp_get_ace(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_ntp_ace_iterate((TBL_ITER_CB_FUNC)tbl_ntp_ace_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}


int32
ntp_config_exec(char *host, char *pszcmd, uint8 bShowCmd)
{
    char szTmpName[] = "/tmp/ntpdcXXXXXX";
    char szFullCmd[M_NTP_CMD_BUFSZ];
#define M_NTP_LINE_BUFSZ    256
    char szLineBuf[M_NTP_LINE_BUFSZ];
    int fd = -1;
    FILE *fp = NULL;
    int ret = 0;
    
    if (NULL == pszcmd)
    {
        return -1;
    }
    
    /* generate temporary file name */
    if ((fd = mkstemp(szTmpName)) < 0)
    {
        //cli_out (cli, "% Generate temporary file failed: %s\n", sal_strerror(errno));
        sal_unlink(szTmpName);
        return -1;
    }
    sal_close(fd);

    sal_snprintf(szFullCmd, M_NTP_CMD_BUFSZ, "%s %s >%s 2>&1",
                 pszcmd, host, szTmpName);
    if (system(szFullCmd) == -1)
    {
       // cli_out(cli, "% Execute command failed: %s.\n", sal_strerror(errno));
        sal_unlink(szTmpName);
        return -1;
    }

    if ((fp = sal_fopen(szTmpName, SAL_OPEN_RO)) == NULL)
    {
        if (ENOENT == errno)
        {
            return 0;
        }

       // cli_out(cli, "% Open file error: %s\n", sal_strerror(errno));
        sal_unlink(szTmpName);
        return -1;
    }

    while (fgets(szLineBuf, M_NTP_CMD_BUFSZ, fp) != NULL)
    {
        if (!sal_strncmp(szLineBuf, "***", 3))
        {
            /* *** indicate error */
            ret = -1;
         //   cli_out(cli, "%% %s", szLineBuf + 3);
            continue;
        }

        /*do not show ipv6 info when ipv6 is not enabled
        modified by ychen in 2013-01-30 for bug 22144 and 22156*/
/*
        if (sal_strstr(pszcmd, "reslist") && sal_strstr(szLineBuf, ":") && !module_control("IPV6"))
        {
            continue;
        }
*/
        if (bShowCmd)
        {
          //  cli_out(cli, "%s", szLineBuf);
        }
    }
    
    sal_fclose(fp);
    sal_unlink(szTmpName);
    return ret;
}



int32
ntp_show_exec(cfg_cmd_para_t *para, char *host, char *pszcmd)
{
    char szTmpName[] = "/tmp/ntpdcXXXXXX";
    char szFullCmd[M_NTP_CMD_BUFSZ];
#define M_NTP_LINE_BUFSZ    256
    char szLineBuf[M_NTP_LINE_BUFSZ];
    int fd = -1;
    FILE *fp = NULL;
    int ret = 0;
    
    if (NULL == pszcmd)
    {
        return -1;
    }
    
    /* generate temporary file name */
    if ((fd = mkstemp(szTmpName)) < 0)
    {
        //cli_out (cli, "% Generate temporary file failed: %s\n", sal_strerror(errno));
        sal_unlink(szTmpName);
        return -1;
    }
    sal_close(fd);

    sal_snprintf(szFullCmd, M_NTP_CMD_BUFSZ, "%s %s >%s 2>&1",
                 pszcmd, host, szTmpName);
    if (system(szFullCmd) == -1)
    {
       // cli_out(cli, "% Execute command failed: %s.\n", sal_strerror(errno));
        sal_unlink(szTmpName);
        return -1;
    }

    if ((fp = sal_fopen(szTmpName, SAL_OPEN_RO)) == NULL)
    {
        if (ENOENT == errno)
        {
            return 0;
        }

       // cli_out(cli, "% Open file error: %s\n", sal_strerror(errno));
        sal_unlink(szTmpName);
        return -1;
    }

    while (fgets(szLineBuf, M_NTP_CMD_BUFSZ, fp) != NULL)
    {
        if (!sal_strncmp(szLineBuf, "***", 3))
        {
            /* *** indicate error */
            ret = -1;
         //   cli_out(cli, "%% %s", szLineBuf + 3);
            continue;
        }

        /*do not show ipv6 info when ipv6 is not enabled
        modified by ychen in 2013-01-30 for bug 22144 and 22156*/
/*
        if (sal_strstr(pszcmd, "reslist") && sal_strstr(szLineBuf, ":") && !module_control("IPV6"))
        {
            continue;
        }
*/

        sal_snprintf(para->p_rs->desc, CMD_BUF_256, "%s", szLineBuf);
        para->p_rs->rc = PM_E_CFG_CDS_SHOW;

    }
    
    sal_fclose(fp);
    sal_unlink(szTmpName);
    return ret;
}

int32
ntp_server_set_namespace(cfg_cmd_para_t *para, const char *host, const char *is_inband)
{
    tbl_ntp_server_key_t key;
    tbl_ntp_server_t *p_ntp_srv;

    sal_memset(&key, 0, sizeof(tbl_ntp_server_key_t));

    sal_strcpy(key.host, host);

    p_ntp_srv = tbl_ntp_server_get_ntp_server(&key);

    if (NULL == p_ntp_srv)
    {
        CFG_INVALID_PARAM_RET("No such host: %s", host);
    }

    if (0 == sal_strcmp(is_inband, "inband"))
    {
        p_ntp_srv->is_inband = TRUE;
    }
    else
    {
        p_ntp_srv->is_inband = FALSE;
    }

    tbl_ntp_server_set_ntp_server_field(p_ntp_srv, TBL_NTP_SERVER_FLD_IS_INBAND);

    return PM_E_NONE;
}

int32
app_ntp_init()
{
    tbl_ntp_cfg_t* ntp_cfg = tbl_ntp_cfg_get_ntp_cfg();
    if(!ntp_cfg)
    {
        return PM_E_NOT_INITED;
    }
    
    /* create /etc/ntp if it not exists */
    if (!syscmd_file_exist(NTP_KEY_PATH))
    {
        sal_mkdir(NTP_KEY_PATH, 0755);
    }

    /* generate default key file */
    ntp_save_keypair();

    memset((char *)g_app_ntp_ifstatus, 0, sizeof(g_app_ntp_ifstatus));

    return PM_E_NONE;
}

