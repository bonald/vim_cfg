#include <assert.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#include "proto.h"
#include "gen/tbl_snmp_cfg_define.h"
#include "gen/tbl_snmp_cfg.h"
#include "gen/tbl_snmp_view_define.h"
#include "gen/tbl_snmp_view.h"
#include "gen/tbl_snmp_community_define.h"
#include "gen/tbl_snmp_community.h"
#include "gen/tbl_snmp_trap_define.h"
#include "gen/tbl_snmp_trap.h"
#include "gen/tbl_snmp_inform_define.h"
#include "gen/tbl_snmp_inform.h"
#include "gen/tbl_snmp_context_define.h"
#include "gen/tbl_snmp_context.h"
#include "gen/tbl_snmp_usm_user_define.h"
#include "gen/tbl_snmp_usm_user.h"
#include "gen/tbl_snmp_group_usm_define.h"
#include "gen/tbl_snmp_group_usm.h"
#include "gen/tbl_snmp_access_usm_define.h"
#include "gen/tbl_snmp_access_usm.h"
#include "gen/tbl_snmp_notify_define.h"
#include "gen/tbl_snmp_notify.h"
#include "gen/tbl_snmp_target_addr_define.h"
#include "gen/tbl_snmp_target_addr.h"
#include "gen/tbl_snmp_target_params_define.h"
#include "gen/tbl_snmp_target_params.h"
#include "gen/tbl_card_define.h"
#include "gen/tbl_card.h"
#include "gen/tbl_version_define.h"
#include "gen/tbl_version.h"

#include "rsa_derhex.h"
#include "appcfg.h"
#include "app_snmp.h"
#include "version.h"
#include "glb_compile_time.h"
#include "glb_tempfile_define.h"

#ifdef _GLB_UML_SYSTEM_
#define M_SNMPD_PROG        "/centec_switch/sbin/snmpd"
#define M_SUBAGENT_START_SH "/centec_switch/sbin/subagent.sh"
#define M_SUBAGENT_DIR      "/centec_switch/sbin"
#define M_SNMPD_CFGFILE     "/centec_switch/etc/snmpd.conf"
#else
#define M_SNMPD_PROG        "snmpd"
#define M_SUBAGENT_START_SH "subagent.sh"
#define M_SUBAGENT_DIR      "/usr/sbin"
#define M_SNMPD_CFGFILE     CTC_ETC_BASE_DIR"/etc/snmpd.conf"
#endif
#define M_SNMPD_PIDFILE     "/var/run/snmpd.pid"

#define M_SNMPD_USM_CFGFILE     "/var/net-snmp/snmpd.conf"

#define GLB_APP_SNMPD_EN_TIMER     7

extern int _decode_snmp_line_str(char *pszLine, char *pszBuf, size_t bufsz);
extern int app_is_startup_done();

static u_int32_t _g_secidx = 0; /* 0 based */
ctc_task_t *p_snmp_en_timer = NULL;

int app_snmp_free_secidx(int idx)
{
    if (idx < 0 || idx >= MAX_SNMP_COMMUNITY_NUM) {
        return -1;
    }

    _g_secidx &= ~(1 >> idx);
    return 0;
}

int app_snmp_alloc_secidx()
{
    int i;

    for (i = 0; i < MAX_SNMP_COMMUNITY_NUM; i++) {
        if (!(_g_secidx & (1 << i))) {
            _g_secidx |= (1 << i);
            return i;
        }
    }

    return -1;
}

static int
_app_snmpd_stop(void)
{
    char szCmd[512];
    char syCmd[512];
    FILE *fp = NULL;

    if (!app_is_startup_done()) {
        return 0;
    }

    snprintf(szCmd, 512, 
        "cdbctl delete/cdb/sys/ns_port_forwarding/0#1#161#0.0.0.0/ip/0.0.0.0; "
        " ps -ax 2>/dev/null | grep snmpd | grep -v grep | awk '{ print $1}' | xargs kill;");
    system(szCmd);

#ifdef CONFIG_SNMPAGT
    snprintf(syCmd, 512,
    "ps -fe 2>/dev/null| grep snmpagent | grep -v grep | awk '{print $2}' | xargs kill -9 $2;");
#else
    snprintf(syCmd, 512,
    "ps -fe 2>/dev/null| grep snmp_agent_main.py | grep -v grep | awk '{print $2}' | xargs kill -9 $2;");
#endif
    system(syCmd);

    if ((fp=fopen(M_SNMPD_USM_CFGFILE, "r")) != NULL)
    {
        fclose(fp);
        system("rm /var/net-snmp/snmpd.conf");
    }

    return 0;
}

static int
_app_snmpd_start()
{
    char szCmd[512];
    char syCmd[512];

    if (!app_is_startup_done()) {
        return 0;
    }

    snprintf(szCmd, 512, "%s -c %s; "
        "cdbctl create/cdb/sys/ns_port_forwarding/0#1#161#0.0.0.0/ip/0.0.0.0;", 
        M_SNMPD_PROG, M_SNMPD_CFGFILE);   
    system(szCmd);

#ifdef CONFIG_SNMPAGT
    snprintf(syCmd, 512, "snmpagent &");
#else
    snprintf(syCmd, 512, "%s %s &",
      M_SUBAGENT_START_SH, M_SUBAGENT_DIR);
#endif
    system(syCmd);
    return 0;
}

int
_app_snmpd_restart(void)
{
    tbl_snmp_cfg_t *p_db_cfg = tbl_snmp_cfg_get_snmp_cfg();
    
    if (!app_is_startup_done() || !p_db_cfg->enable) {
        return 0;
    }

    if (!p_db_cfg->server_enable)
    {
        return 0;
    }
    
    /* send SIGHUP to snmp */
    return system("ps -ax 2>/dev/null | grep snmpd | grep -v grep | awk '{ print $1}' | xargs kill -1");
}


static int32
tbl_snmp_view_cfg_write(tbl_snmp_view_t *p_snmp_view, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs->argv[0];
    char  mask[2*SNMP_MASK_SIZE] = "0x";

    if ('\0' == p_snmp_view->key.view.name) {
        /* incomplete */
        return PM_E_NONE;
    }

    int i = 0, j = 2;
    while('\0' != p_snmp_view->mask[i])
    {
        mask[j] = p_snmp_view->mask[i];
        if (0 != i % 2 && '\0' != p_snmp_view->mask[i+1])
        {
            mask[++j] = ':';
        }
        ++i;
        ++j;
    }

    if (i % 2)
    {
        mask[j] = 'f';
    }

    fprintf(fp, "view %s %s %s %s\r\n",
            p_snmp_view->key.view.name,
            (SNMP_VIEW_CLASS_INCLUDE == p_snmp_view->type) ? "included" : "excluded",
            p_snmp_view->key.view.subtree,
            ('\0' != p_snmp_view->mask[0]) ? mask : "");

    
    return PM_E_NONE;
}


int snmp_gen_view_cfg(FILE *fp)
{
    tbl_iter_args_t args;
    args.argv[0] = fp;
    tbl_snmp_view_iterate((TBL_ITER_CB_FUNC)tbl_snmp_view_cfg_write, &args);
    return PM_E_NONE;
}

static int32
tbl_snmp_community_cfg_write(tbl_snmp_community_t *p_snmp_community, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs->argv[0];

    if ('\0' != p_snmp_community->view[0]) {
        fprintf(fp, "%s %s default -V %s\r\n",
                (SNMP_COMMUNITY_RO == p_snmp_community->type) ? "rocommunity" : "rwcommunity",
                p_snmp_community->key.name,
                p_snmp_community->view);
                
    } else {
        fprintf(fp, "%s %s\r\n",
                (SNMP_COMMUNITY_RO == p_snmp_community->type) ? "rocommunity" : "rwcommunity",
                p_snmp_community->key.name);    
    }
    
    if ('\0' != p_snmp_community->view[0]) {
        fprintf(fp, "%s %s default -V %s\r\n",
                (SNMP_COMMUNITY_RO == p_snmp_community->type) ? "rocommunity6" : "rwcommunity6",
                p_snmp_community->key.name,
                p_snmp_community->view);
                
    } else {
        fprintf(fp, "%s %s\r\n",
                (SNMP_COMMUNITY_RO == p_snmp_community->type) ? "rocommunity6" : "rwcommunity6",
                p_snmp_community->key.name);    
    }
    
    fprintf(fp, "com2sec comm%d default %s\r\n",
        (2*p_snmp_community->secidx + 1), p_snmp_community->key.name);
    fprintf(fp, "group grpcomm%d v1 comm%d\r\n",
        (2*p_snmp_community->secidx + 1), (2*p_snmp_community->secidx + 1));
    fprintf(fp, "group grpcomm%d v2c comm%d\r\n",
        (2*p_snmp_community->secidx + 1), (2*p_snmp_community->secidx + 1));
    
    fprintf(fp, "com2sec6 comm%d :: %s\r\n",
        (2*p_snmp_community->secidx + 2), p_snmp_community->key.name);
    fprintf(fp, "group grpcomm%d v1 comm%d\r\n",
        (2*p_snmp_community->secidx + 2), (2*p_snmp_community->secidx + 2));
    fprintf(fp, "group grpcomm%d v2c comm%d\r\n",
        (2*p_snmp_community->secidx + 2), (2*p_snmp_community->secidx + 2));

    return PM_E_NONE;
}

int snmp_gen_community_cfg(FILE *fp)
{
    tbl_iter_args_t args;
    args.argv[0] = fp;
    tbl_snmp_community_iterate((TBL_ITER_CB_FUNC)tbl_snmp_community_cfg_write, &args);
    return PM_E_NONE;
}


static int32
tbl_snmp_context_cfg_write(tbl_snmp_context_t *p_snmp_context_entry, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs->argv[0];

    if (p_snmp_context_entry->flags != 0xFF)
    {
        fprintf(fp, "addContext %s\r\n", p_snmp_context_entry->key.context_name);
    }
    else
    {
        fprintf(fp, "delContext %s\r\n", p_snmp_context_entry->key.context_name);
    }
    
    return PM_E_NONE;
}


int snmp_gen_context_cfg(FILE *fp)
{
    tbl_iter_args_t args;
    args.argv[0] = fp;
    tbl_snmp_context_iterate((TBL_ITER_CB_FUNC)tbl_snmp_context_cfg_write, &args);
    return PM_E_NONE;
}  

static int32
tbl_snmp_usm_user_cfg_write(tbl_snmp_usm_user_t *p_snmp_usm_user, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs->argv[0];
    char auth_pwd[SNMP_PASSWORD_SIZE] = {0};
    char pri_pwd[SNMP_PASSWORD_SIZE] = {0};
    char pri_type[SNMP_NAME_SIZE] = {0};
    tbl_snmp_cfg_t *p_db_cfg = NULL;

    if ('\0' != p_snmp_usm_user->enc_auth_pwd[0])
    {
        do_decrypt(M_DEFAULT_PW_STR, p_snmp_usm_user->enc_auth_pwd, 
            sal_strlen(p_snmp_usm_user->enc_auth_pwd), auth_pwd, SNMP_PASSWORD_SIZE);
    }
    else
    {
        snprintf(auth_pwd, SNMP_PASSWORD_SIZE, "%s", p_snmp_usm_user->auth_pwd);
    }

    if ('\0' != p_snmp_usm_user->enc_priv_pwd[0])
    {
        do_decrypt(M_DEFAULT_PW_STR, p_snmp_usm_user->enc_priv_pwd, 
            sal_strlen(p_snmp_usm_user->enc_priv_pwd), pri_pwd, SNMP_PASSWORD_SIZE);
    }
    else
    {
        snprintf(pri_pwd, SNMP_PASSWORD_SIZE, "%s", p_snmp_usm_user->priv_pwd);
    }

    if (p_snmp_usm_user->priv_type == SNMP_PRIVACY_AES)
    {
        snprintf(pri_type, SNMP_NAME_SIZE, "AES");
    }
    else if (p_snmp_usm_user->priv_type == SNMP_PRIVACY_DES)
    {
        snprintf(pri_type, SNMP_NAME_SIZE, "DES");
    }

    if (p_snmp_usm_user->flags != 0xFF)
    {
        p_db_cfg = tbl_snmp_cfg_get_snmp_cfg();

        if (0 == p_snmp_usm_user->auth_type)
        {
            if ('\0' != p_snmp_usm_user->remo_engid[0]) 
            {
                fprintf(fp, "createUser -e \"%s\" %s\r\n", 
                    p_snmp_usm_user->remo_engid, p_snmp_usm_user->key.usm_user_name); 
            }
            else
            {
                fprintf(fp, "createUser %s\r\n", p_snmp_usm_user->key.usm_user_name); 
            }
            return PM_E_NONE;
        }

        if ('\0' != p_snmp_usm_user->remo_engid[0]) 
        {
            if ('\0' != pri_pwd[0])
            {
                fprintf(fp, "createUser -e \"%s\" %s %s \"%s\" %s \"%s\"\r\n",
                    p_snmp_usm_user->remo_engid, 
                    p_snmp_usm_user->key.usm_user_name,
                    (SNMP_AUTHENTICATION_MD5 == p_snmp_usm_user->auth_type) ? "MD5" : "SHA",
                    auth_pwd, pri_type, pri_pwd);  
            }
            else
            {
                fprintf(fp, "createUser -e \"%s\" %s %s \"%s\" %s\r\n",
                    p_snmp_usm_user->remo_engid, 
                    p_snmp_usm_user->key.usm_user_name,
                    (SNMP_AUTHENTICATION_MD5 == p_snmp_usm_user->auth_type) ? "MD5" : "SHA",
                    auth_pwd, pri_type);  
            }
        } 
        else if ('\0' != p_db_cfg->engineid[0])
        {
            if ('\0' != pri_pwd[0])
            {
                fprintf(fp, "createUser -e %s %s %s \"%s\" %s \"%s\"\r\n",
                    p_db_cfg->engineid, 
                    p_snmp_usm_user->key.usm_user_name,
                    (SNMP_AUTHENTICATION_MD5 == p_snmp_usm_user->auth_type) ? "MD5" : "SHA",
                    auth_pwd, pri_type, pri_pwd);  
            }
            else
            {
                fprintf(fp, "createUser -e %s %s %s \"%s\" %s\r\n",
                    p_db_cfg->engineid, 
                    p_snmp_usm_user->key.usm_user_name,
                    (SNMP_AUTHENTICATION_MD5 == p_snmp_usm_user->auth_type) ? "MD5" : "SHA",
                    auth_pwd, pri_type);  
            }
        }
        else 
        {
            if ('\0' != pri_pwd[0])
            {
                fprintf(fp, "createUser %s %s \"%s\" %s \"%s\"\r\n",
                    p_snmp_usm_user->key.usm_user_name,
                    (SNMP_AUTHENTICATION_MD5 == p_snmp_usm_user->auth_type) ? "MD5" : "SHA",
                    auth_pwd, pri_type, pri_pwd);  
            }
            else
            {
                fprintf(fp, "createUser %s %s \"%s\" %s\r\n",
                    p_snmp_usm_user->key.usm_user_name,
                    (SNMP_AUTHENTICATION_MD5 == p_snmp_usm_user->auth_type) ? "MD5" : "SHA",
                    auth_pwd, pri_type); 
            }
        }
    }
    else
    {
        fprintf(fp, "delusmUser %s\r\n", p_snmp_usm_user->key.usm_user_name);
    }
    
    return PM_E_NONE;
}


int snmp_gen_usm_user_cfg(FILE *fp)
{
    tbl_iter_args_t args;
    args.argv[0] = fp;
    tbl_snmp_usm_user_iterate((TBL_ITER_CB_FUNC)tbl_snmp_usm_user_cfg_write, &args);
    return PM_E_NONE;
}   

static int32
tbl_snmp_group_usm_cfg_write(tbl_snmp_group_usm_t *p_snmp_group_usm, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs->argv[0];

    if (p_snmp_group_usm->flags != 0xFF)
    {
        fprintf(fp, "group %s usm %s\r\n",
            p_snmp_group_usm->key.group_usm.grp_name, 
            p_snmp_group_usm->key.group_usm.security_name);  
    }
    else
    {
        fprintf(fp, "groupdel %s usm %s\r\n",
            p_snmp_group_usm->key.group_usm.grp_name, 
            p_snmp_group_usm->key.group_usm.security_name); 
    }
    
    return PM_E_NONE;
}


int snmp_gen_group_usm_cfg(FILE *fp)
{
    tbl_iter_args_t args;
    args.argv[0] = fp;
    tbl_snmp_group_usm_iterate((TBL_ITER_CB_FUNC)tbl_snmp_group_usm_cfg_write, &args);
    return PM_E_NONE;
}   

static int32
tbl_snmp_access_usm_cfg_write(tbl_snmp_access_usm_t *p_snmp_access_usm, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs->argv[0];
    char usm_type[SNMP_NAME_SIZE] = {0};

    if (p_snmp_access_usm->usm_level == SNMP_LEVEL_AUTHPRIV)
    {
        snprintf(usm_type, SNMP_NAME_SIZE, "priv");
    }
    else if (p_snmp_access_usm->usm_level == SNMP_LEVEL_AUTHNOPRIV)
    {
        snprintf(usm_type, SNMP_NAME_SIZE, "auth");
    }
    else
    {
        snprintf(usm_type, SNMP_NAME_SIZE, "noauth");
    }

    if (p_snmp_access_usm->flags != 0xFF)
    {
        if ('\0' != p_snmp_access_usm->context_name[0])
        {
            fprintf(fp, "access %s %s usm %s %s %s %s %s\r\n",
            p_snmp_access_usm->key.usm_access_name, 
            p_snmp_access_usm->context_name,
            usm_type,
            (TRUE == p_snmp_access_usm->isprefix)? "prefix":"exact",
            p_snmp_access_usm->read_view,
            p_snmp_access_usm->write_view,
            p_snmp_access_usm->notify_view);
        }
        else
        {
            fprintf(fp, "access %s \"\" usm %s %s %s %s %s\r\n",
            p_snmp_access_usm->key.usm_access_name, 
            usm_type,
            (TRUE == p_snmp_access_usm->isprefix)? "prefix":"exact",
            p_snmp_access_usm->read_view,
            p_snmp_access_usm->write_view,
            p_snmp_access_usm->notify_view);
        }
    }
    else
    {
        fprintf(fp, "accessdel %s %s %s\r\n",
            p_snmp_access_usm->key.usm_access_name, 
            (TRUE == p_snmp_access_usm->isprefix)? "prefix":"exact",
            usm_type); 
    }
    
    return PM_E_NONE;
}


int snmp_gen_access_usm_cfg(FILE *fp)
{
    tbl_iter_args_t args;
    args.argv[0] = fp;
    tbl_snmp_access_usm_iterate((TBL_ITER_CB_FUNC)tbl_snmp_access_usm_cfg_write, &args);
    return PM_E_NONE;
}  


static int32
tbl_snmp_notify_entry_cfg_write(tbl_snmp_notify_t *p_snmp_notify_entry, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs->argv[0];

    if (p_snmp_notify_entry->flags != 0xFF)
    {
        fprintf(fp, "snmpNotifyTable %s %s %s 3 1\r\n",
            p_snmp_notify_entry->key.notify_name, 
            p_snmp_notify_entry->tag_name,
            (p_snmp_notify_entry->notify_type == TRUE) ? "2":"1");  
    }
    else
    {
        fprintf(fp, "snmpNotifyTableDel %s\r\n",
            p_snmp_notify_entry->key.notify_name);  
    }
    
    return PM_E_NONE;
}


int snmp_gen_notify_entry_cfg(FILE *fp)
{
    tbl_iter_args_t args;
    args.argv[0] = fp;
    tbl_snmp_notify_iterate((TBL_ITER_CB_FUNC)tbl_snmp_notify_entry_cfg_write, &args);
    return PM_E_NONE;
}  

#if 0 //TODO:ipv6 process
char *
read_config_save_octet_string_without_separator(char *saveto, u_char * str, size_t len)
{
    int i;
    u_char *cp;

    /*
     * is everything easily printable 
     */
    for (i = 0, cp = str; i < (int) len && cp &&
         (isalpha(*cp) || isdigit(*cp) || *cp == ' '); cp++, i++);

    if (str != NULL) {
        sprintf(saveto, "0x");
        saveto += 2;
        for (i = 0; i < (int) len; i++) {
            sprintf(saveto, "%02x", str[i]);
            saveto = saveto + 2;
        }
    } else {
        sprintf(saveto, "\"\"");
        saveto += 2;
    }

    return saveto;
}
#endif

extern int
split_tokens(char *pszBuf, size_t nLen, unsigned long nMaxToks, 
             char *pszDelimiters, unsigned long *pnToks, char ***pppTokArr);

static int32
tbl_snmp_target_addr_entry_cfg_write(tbl_snmp_target_addr_t *p_snmp_tar_addr_entry, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs->argv[0];
    int a = 0;
    int b = 0;
    char address[256] = {0};
    char *domain = NULL;
    tbl_snmp_target_params_key_t target_param_key;
    tbl_snmp_target_params_t *p_db_tar_params = NULL;
    tbl_snmp_usm_user_key_t user_key;
    tbl_snmp_usm_user_t *p_db_user = NULL;
    tbl_snmp_notify_t *p_db_notify = NULL;
    char ip_address[47] = {0};
    char usm_type[SNMP_NAME_SIZE] = {0};
    char tag_Tmp[SNMP_NAME_SIZE] = {0};
    char auth_pwd[SNMP_PASSWORD_SIZE] = {0};
    char priv_pwd[SNMP_PASSWORD_SIZE] = {0};
    char enginid[SNMP_ENGINEID_SIZE] = {0};
    char privcy_str[SNMP_SPRINT_MAX_LEN] = {0};
    char **ppTokArr = NULL;
    unsigned long nToks = 0;
    int i;
    tbl_snmp_cfg_t *p_db_cfg = NULL;
 
    memset(&target_param_key, 0x0, sizeof(target_param_key));
    memset(&user_key, 0x0, sizeof(user_key));

    a = (p_snmp_tar_addr_entry->upd_port & 0xff00)  >> 8;
    b = (p_snmp_tar_addr_entry->upd_port & 0x00ff)  >> 0;
    if (p_snmp_tar_addr_entry->ip_addr.family == AF_INET6)
    {
    #if 0 //TODO:ipv6 process
        //read_config_save_octet_string_without_separator(address, (unsigned char *)&p_snmp_tar_addr_entry->ip_addr, IPV6_MAX_BITLEN/8);
        snprintf(address, sizeof(address), "0x%16s%02x%02x", p_snmp_tar_addr_entry->ip_addr.u.val, a, b);
    #endif
        domain = ".1.3.6.1.2.1.100.1.2";

    }
    else if(p_snmp_tar_addr_entry->ip_addr.family == AF_INET)
    {
        snprintf(address, sizeof(address), "0x%08x%02x%02x", ntohl(p_snmp_tar_addr_entry->ip_addr.u.prefix4.s_addr), a, b);
        domain = ".1.3.6.1.6.1.1";
    }
    
    if (p_snmp_tar_addr_entry->flags != 0xFF)
    {
        fprintf(fp, "targetAddr %s %s %s %d %d \"%s\" 3 1\r\n",
            p_snmp_tar_addr_entry->key.taraddr_name, domain,
            address, p_snmp_tar_addr_entry->timeout, p_snmp_tar_addr_entry->retries,
            p_snmp_tar_addr_entry->tag_list); 
            snprintf(target_param_key.params_name, SNMP_NAME_SIZE, "%s", p_snmp_tar_addr_entry->para_name);
            
        cdb_addr_val2str(ip_address, 47, &p_snmp_tar_addr_entry->ip_addr);
        p_db_tar_params = tbl_snmp_target_params_get_snmp_target_params(&target_param_key);
        if (NULL == p_db_tar_params || 0xFF == p_db_tar_params->flags)
        {
            return PM_E_NONE;
        }
        snprintf(user_key.usm_user_name, SNMP_NAME_SIZE, "%s", p_db_tar_params->user_name);
        p_db_user = tbl_snmp_usm_user_get_snmp_usm_user(&user_key);
        if (NULL == p_db_user || 0xFF == p_db_user->flags)
        {
            return PM_E_NONE;
        }
        if ('\0' != p_db_user->enc_auth_pwd[0])
        {
            do_decrypt(M_DEFAULT_PW_STR, p_db_user->enc_auth_pwd, 
                sal_strlen(p_db_user->enc_auth_pwd), auth_pwd, SNMP_PASSWORD_SIZE);
        }
        else
        {
            snprintf(auth_pwd, SNMP_PASSWORD_SIZE, "%s", p_db_user->auth_pwd);
        }

        if (p_db_user->priv_type)
        {
            if ('\0' != p_db_user->enc_priv_pwd[0])
            {
                do_decrypt(M_DEFAULT_PW_STR, p_db_user->enc_priv_pwd, 
                    sal_strlen(p_db_user->enc_priv_pwd), priv_pwd, SNMP_PASSWORD_SIZE);
            }
            else
            {
                snprintf(priv_pwd, SNMP_PASSWORD_SIZE, "%s", p_db_user->priv_pwd);
            }
            snprintf(privcy_str, SNMP_SPRINT_MAX_LEN, "-x %s -X %s",
            (SNMP_PRIVACY_AES == p_db_user->priv_type) ? "AES":"DES", priv_pwd);
        }
        
        if (p_db_tar_params->usm_level == SNMP_LEVEL_AUTHPRIV)
        {
            snprintf(usm_type, SNMP_NAME_SIZE, "authPriv");
        }
        else if (p_db_tar_params->usm_level == SNMP_LEVEL_AUTHNOPRIV)
        {
            snprintf(usm_type, SNMP_NAME_SIZE, "authNoPriv");
        }
        else
        {
            snprintf(usm_type, SNMP_NAME_SIZE, "noAuthNoPriv");
        }

        if (0 == split_tokens(p_snmp_tar_addr_entry->tag_list, sal_strlen(p_snmp_tar_addr_entry->tag_list), SNMP_MAX_TAGS, 
			   " ", &nToks, &ppTokArr))
        {
            for (i = 0; i < nToks; i++)
            {
                memset(tag_Tmp, 0x0, sizeof(tag_Tmp));
                snprintf(tag_Tmp, SNMP_NAME_SIZE, ppTokArr[i]);
                p_db_notify = tbl_snmp_notify_get_by_tagname(tag_Tmp);
                p_db_cfg = tbl_snmp_cfg_get_snmp_cfg();
                if ('\0' != p_db_user->remo_engid[0])
                {
                    snprintf(enginid, SNMP_ENGINEID_SIZE, p_db_user->remo_engid);
                }
                else if ('\0' != p_db_cfg->engineid[0])
                {
                    snprintf(enginid, SNMP_ENGINEID_SIZE, p_db_cfg->engineid);
                }
                else
                {
                    snprintf(enginid, SNMP_ENGINEID_SIZE, "30383038303830383038");
                }
                    
                if (NULL != p_db_notify && p_db_notify->flags != 0xFF)
                {
                    
                    fprintf(fp, "trapsess %s-v 3 -e %s -u %s -a %s -A %s %s -l %s -r %d -t %d %s:%d\r\n", 
                        0 == p_db_notify->notify_type ? "": "-Ci ",
                        enginid,
                        user_key.usm_user_name,
                        (SNMP_AUTHENTICATION_MD5 == p_db_user->auth_type) ? "MD5" : "SHA",
                        auth_pwd, (0 == p_db_user->priv_type)? "":privcy_str, usm_type,
                        p_snmp_tar_addr_entry->retries, p_snmp_tar_addr_entry->timeout,
                        ip_address, p_snmp_tar_addr_entry->upd_port); 
                }
            }
        } 
    }
    else
    {
        fprintf(fp, "targetAddrdel %s\r\n", p_snmp_tar_addr_entry->key.taraddr_name); 
    }    

    return PM_E_NONE;
}


int snmp_gen_target_addr_entry_cfg(FILE *fp)
{
    tbl_iter_args_t args;
    args.argv[0] = fp;
    tbl_snmp_target_addr_iterate((TBL_ITER_CB_FUNC)tbl_snmp_target_addr_entry_cfg_write, &args);
    return PM_E_NONE;
}

static int32
tbl_snmp_target_params_entry_cfg_write(tbl_snmp_target_params_t *p_snmp_tar_params_entry, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs->argv[0];

    if (p_snmp_tar_params_entry->flags != 0xFF)
    {
        fprintf(fp, "targetParams %s 3 3 %s %d 3 1\r\n",
            p_snmp_tar_params_entry->key.params_name,
            p_snmp_tar_params_entry->user_name,
            p_snmp_tar_params_entry->usm_level);  
    }
    else
    {
        fprintf(fp, "targetParamsdel %s\r\n", p_snmp_tar_params_entry->key.params_name);
    }
    return PM_E_NONE;
}


int snmp_gen_target_params_entry_cfg(FILE *fp)
{
    tbl_iter_args_t args;
    args.argv[0] = fp;
    tbl_snmp_target_params_iterate((TBL_ITER_CB_FUNC)tbl_snmp_target_params_entry_cfg_write, &args);
    return PM_E_NONE;
}


static int32
tbl_snmp_trap_cfg_write(tbl_snmp_trap_t *p_snmp_trap, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs->argv[0];
    char ip_addr[47] = {0}; 
    
    //fprintf(fp, "trapsess -v 2c -c aaa  4.4.4.201:162 .1.3.6.1.6.3.1.1.5 192.168.8.74 2 50 30 .1.3.6.1.6.3.1.1.5.1\r\n");
    #if 1
    cdb_addr_val2str(ip_addr, 47, &p_snmp_trap->key.trap.ip_addr);
    fprintf(fp, "trapsink   %s:%d  %s\r\n",
        ip_addr, p_snmp_trap->key.trap.udpport,p_snmp_trap->key.trap.community);
    fprintf(fp, "trap2sink   %s:%d  %s\r\n",
        ip_addr, p_snmp_trap->key.trap.udpport,p_snmp_trap->key.trap.community);
    #endif
    return PM_E_NONE;
}

int snmp_gen_trap_cfg(FILE *fp)
{
    tbl_iter_args_t args;
    args.argv[0] = fp;
    tbl_snmp_trap_iterate((TBL_ITER_CB_FUNC)tbl_snmp_trap_cfg_write, &args);
    return PM_E_NONE;
}

static int32
tbl_snmp_inform_cfg_write(tbl_snmp_inform_t *p_snmp_inform, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs->argv[0];
    char ip_addr[47] = {0}; 

    cdb_addr_val2str(ip_addr, 47, &p_snmp_inform->key.inform.ip_addr);
    fprintf(fp, "informsink   %s:%d  %s\r\n",
        ip_addr, p_snmp_inform->key.inform.udpport,p_snmp_inform->key.inform.community);

    return PM_E_NONE;
}

int snmp_gen_inform_cfg(FILE *fp)
{
    tbl_iter_args_t args;
    args.argv[0] = fp;
    tbl_snmp_inform_iterate((TBL_ITER_CB_FUNC)tbl_snmp_inform_cfg_write, &args);
    return PM_E_NONE;
}

int snmp_gen_sysdescr(FILE *fp)
{
    FILE *poem = fopen("/tmp/oem_info", "r");
    char szLine[512];
    char szPackage[SNMP_INFO_SIZE];
    char szProduct[SNMP_INFO_SIZE];
    char szHardware[SNMP_INFO_SIZE];
    char szVersion[SNMP_INFO_SIZE];
    char szCompany[SNMP_INFO_SIZE];
    tbl_version_t* p_version = tbl_version_get_version();
    tbl_card_t * p_card = tbl_card_get_default_card();

    if (NULL == p_version || NULL == p_card)
    {
        return -1;
    }
    snprintf(szPackage, SNMP_INFO_SIZE, "%s", "UnknownOS");
    snprintf(szProduct, SNMP_INFO_SIZE, "%s", "UnknownProduct");
    snprintf(szHardware, SNMP_INFO_SIZE, "%s", "UnknownHW");
    snprintf(szVersion, SNMP_INFO_SIZE, "%s", OSP_VERSION);
    snprintf(szCompany, SNMP_INFO_SIZE, "%s", "UnknownCompany");

    if (NULL != poem) {
        while (fgets(szLine, 512, poem)) {
            M_TRIM_CR(szLine);
            if (strstr(szLine, "package_name") != NULL) {
                snprintf(szPackage, SNMP_INFO_SIZE, "%s",
                    szLine + strlen("package_name") + 1);
            } else if (strstr(szLine, "product_name") != NULL) {
                snprintf(szProduct, SNMP_INFO_SIZE, "%s",
                    szLine + strlen("product_name") + 1);
            } else if (strstr(szLine, "hardware_type") != NULL) {
                snprintf(szHardware, SNMP_INFO_SIZE, "%s",
                    szLine + strlen("hardware_type") + 1);
            } else if (strstr(szLine, "company_full_name") != NULL) {
                snprintf(szCompany, SNMP_INFO_SIZE, "%s",
                    szLine + strlen("company_full_name") + 1);
            }
        }
    }

    if (p_version->is_cst)
    {
        snprintf(szLine, SNMP_INFO_SIZE + strlen("sysdesc \r\n"),
            "sysdescr Copyright (C) %s. All rights reserved. Device Type is %s. Device Info is %s. Device serial number is %s. Software Version %s. Hardware Version is %s.\r\n",
            p_version->cst_company, p_version->cst_hw_type, p_version->cst_hw_info, p_card->serial_no, p_version->cst_sw_ver, p_version->cst_hw_ver);
    }
    else
    {
        snprintf(szLine, SNMP_INFO_SIZE + strlen("sysdesc \r\n"),
            "sysdescr %s software, %s %s, Version %s; Copyright (C) %s. All rights reserved. Compiled at %s.\r\n",
            szPackage, szProduct, szHardware, szVersion, szCompany, COMPILE_TIME_STR);
    }
    fwrite(szLine, strlen(szLine), 1, fp);

    if (NULL != poem) {
        fclose(poem);
    }
    return 0;
}

int snmp_gen_cfgfile(FILE *fp)
{
    tbl_snmp_cfg_t *p_db_cfg = tbl_snmp_cfg_get_snmp_cfg();
    int ret;

    /* output default configuration */
    fprintf(fp, "# Automatically generated file, please don't edit\r\n\r\n");
    fprintf(fp, "master agentx\r\n");
    fprintf(fp, "agentxsocket /var/run/agentx-master\r\n");
    fprintf(fp, "agentXTimeout 3\n");
    fprintf(fp, "agentaddress udp6:161,udp:161\r\n");
    fprintf(fp, "smuxsocket 1.0.0.0\r\n"); /* XXX: disable smux */

    /* out engineID */
    if ('\0' == p_db_cfg->engineid[0]) {
        fprintf(fp, "engineID 0x30383038303830383038\r\n");
    } else {
        fprintf(fp, "engineID 0x%s\r\n", p_db_cfg->engineid);
    }

    /* output version */
    if (SNMP_VERSION_ALL == p_db_cfg->version) {
        fprintf(fp, "agentversion all\r\n");
    } else if (SNMP_VERSION_V1 == p_db_cfg->version) {
        fprintf(fp, "agentversion v1\r\n");
    } else if (SNMP_VERSION_V2C == p_db_cfg->version) {
        fprintf(fp, "agentversion v2c\r\n");
    } else if (SNMP_VERSION_V3 == p_db_cfg->version) {
        fprintf(fp, "agentversion v3\r\n");
    }

    /* generate sysDesc */
    snmp_gen_sysdescr(fp);

    /* output system contact/location */
    if ('\0' != p_db_cfg->contact.info[0]) {
        fprintf(fp, "syscontact %s\r\n", p_db_cfg->contact.info);
    }    
    if ('\0' != p_db_cfg->location.info[0]) {
        fprintf(fp, "syslocation %s\r\n", p_db_cfg->location.info);
    }   

    ret = snmp_gen_view_cfg(fp);
    if (0 != ret) {
        return ret;
    }

    ret = snmp_gen_community_cfg(fp);

    ret = snmp_gen_trap_cfg(fp);
    ret = snmp_gen_inform_cfg(fp);

    ret = snmp_gen_context_cfg(fp);
    ret = snmp_gen_usm_user_cfg(fp);
    ret = snmp_gen_group_usm_cfg(fp);
    ret = snmp_gen_access_usm_cfg(fp);
    ret = snmp_gen_notify_entry_cfg(fp);
    ret = snmp_gen_target_params_entry_cfg(fp);
    ret = snmp_gen_target_addr_entry_cfg(fp);
    return ret;
}

int
_app_snmpd_generate_cfg(char *pszErrMsg, size_t nBufLen)
{
    FILE *fp = NULL;
    int ret = -1;

    if ((fp = fopen(M_SNMPD_CFGFILE, "w")) == NULL) {
        if (pszErrMsg) {
            snprintf(pszErrMsg, nBufLen, 
                "Can't open syslogd configuration file: %s",
                strerror(errno));
        }
        goto err_out;
    }

    ret = snmp_gen_cfgfile(fp);
    
err_out:
    if (NULL != fp) {
        fclose(fp);
    }
    return ret;
}


static void
_app_snmp_en_timer_handle()
{
    p_snmp_en_timer = NULL;
    tbl_snmp_cfg_t *p_db_cfg = tbl_snmp_cfg_get_snmp_cfg();
    if (p_db_cfg->server_enable)
    {
        _app_snmpd_restart();
    }

    return;
}


int32
app_snmp_en_file(uint32 en)
{
    FILE *fp = NULL;

    if (en)
    {
        fp = sal_fopen(GLB_SNMP_EN_FILE, "w+");
        sal_fclose(fp);
    }
    else
    {
        sal_unlink(GLB_SNMP_EN_FILE);
    }

    return 0;
}

int32
app_snmp_set(tbl_snmp_cfg_field_id_t field_id, tbl_snmp_cfg_t *pitem)
{
    int32 rc = PM_E_NONE;
    char szErrMsg[256];
    
    rc = tbl_snmp_cfg_set_snmp_cfg_field(pitem, field_id);
    rc = _app_snmpd_generate_cfg(szErrMsg, 256);
    if (TBL_SNMP_CFG_FLD_SERVER_ENABLE == field_id) 
    {
        if (pitem->server_enable)
        {
            rc = _app_snmpd_start();
            p_snmp_en_timer = CTC_TASK_ADD_TIME(_app_snmp_en_timer_handle, NULL, GLB_APP_SNMPD_EN_TIMER);
            app_snmp_en_file(TRUE);
        } 
        else
        {
            rc = _app_snmpd_stop();
            app_snmp_en_file(FALSE);
        }
    } 
    else
    {
        rc = _app_snmpd_restart();
    }
    
    return rc;
}

int32
app_snmp_view_set(tbl_snmp_view_field_id_t field_id, tbl_snmp_view_t *pitem)
{
    int32 rc = PM_E_NONE;
    char szErrMsg[256];
    
    PM_E_RETURN(tbl_snmp_view_set_snmp_view_field(pitem, field_id));
    rc = _app_snmpd_generate_cfg(szErrMsg, 256);
    rc = _app_snmpd_restart();  
    return rc;
}

int32
app_snmp_community_set(tbl_snmp_community_field_id_t field_id, tbl_snmp_community_t *pitem)
{
    int32 rc = PM_E_NONE;
    char szErrMsg[256];
    
    PM_E_RETURN(tbl_snmp_community_set_snmp_community_field(pitem, field_id));
    rc = _app_snmpd_generate_cfg(szErrMsg, 256);
    rc = _app_snmpd_restart();  
    return rc;
}


int32
app_snmp_proc_startup_done()
{
    char szErrMsg[256];
    tbl_snmp_cfg_t *p_db_cfg = tbl_snmp_cfg_get_snmp_cfg();

    _app_snmpd_generate_cfg(szErrMsg, 256);
    if (p_db_cfg->server_enable) 
    {
        _app_snmpd_start();
    }

    return PM_E_NONE;
}

int32
app_snmp_init()
{
    tbl_snmp_cfg_t defcfg;

    memset(&defcfg, 0x00, sizeof(tbl_snmp_cfg_t));
    defcfg.version = SNMP_VERSION_ALL;

    defcfg.enable = 1;

    app_snmp_set(TBL_SNMP_CFG_FLD_VERSION, &defcfg);
    app_snmp_set(TBL_SNMP_CFG_FLD_ENABLE, &defcfg);
   
    return PM_E_NONE;
}

