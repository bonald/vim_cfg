/****************************************************************************
* $Id$
*  authd command process
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander
* Date          : 2015-12-03 11:07
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <netinet/ip6.h>
#include <assert.h>

#include "proto.h"
#include "gen/tbl_auth_server_define.h"
#include "gen/tbl_auth_server.h"
#include "gen/tbl_auth_cfg_define.h"
#include "gen/tbl_auth_cfg.h"
#include "gen/tbl_auth_session_define.h"
#include "gen/tbl_auth_session.h"
#include "gen/tbl_authd_debug_define.h"
#include "gen/tbl_authd_debug.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "param_check.h"
#include "lib_fs.h"
#include "taclib.h"
#include "authd.h"
#include "authd_debug.h"
#include "gen/tbl_dot1x_global_define.h"
#include "gen/tbl_dot1x_global.h"
#include "gen/tbl_dot1x_port_define.h"
#include "gen/tbl_dot1x_port.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_dot1x_radius_define.h"
#include "gen/tbl_dot1x_radius.h"
#include "gen/tbl_dot1x_mac_define.h"
#include "gen/tbl_dot1x_mac.h"
#include "dot1x_api.h"
#include "rsa_derhex.h"
#include "sal_base64.h"
#include "gen/tbl_vty_define.h"
#include "gen/tbl_vty.h"
#include "gen/tbl_author_define.h"
#include "gen/tbl_author.h"
#include "gen/tbl_account_define.h"
#include "gen/tbl_account.h"
#include "gen/tbl_login_define.h"
#include "gen/tbl_login.h"
#include "gen/tbl_accountcmd_define.h"
#include "gen/tbl_accountcmd.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/

DEBUG_DECLAR(authd_debug);

#ifndef IN_CLASSD
#define IN_CLASSD(i)	((((long)(i))&0xf0000000)==0xe0000000)
#define IN_MULTICAST IN_CLASSD
#define ITIMER_REAL 0
#define MAXHOSTNAMELEN 64
#endif

#ifndef IPV4_NET0
#define IPV4_NET0(a)    ((((u_int32_t) (a)) & 0xff000000) == 0x00000000)
#define IPV4_NET127(a)  ((((u_int32_t) (a)) & 0xff000000) == 0x7f000000)
#endif

extern int app_is_password_encryption();

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/
extern auth_be_op_t tacplus_be_ops;
extern auth_be_op_t radius_be_ops;
ctc_task_t *g_p_dead_timer = NULL;


/****************************************************************************
 *
* Function
* 
****************************************************************************/

void start_connect_cb(void *pArg);
int auth_do_request(tbl_auth_session_t *pSess, tbl_auth_server_t *pSrv);
void auth_read_timeout_cb(void *pArg);
void
authd_delete_account_stop_session(tbl_auth_session_t  *p_auth_session);
void
authd_send_next_account_stop(tbl_auth_session_t   *p_auth_old_session);

/* modified by liwh for bug 46709, 2018-05-24 */
void auth_reactivate_debug_info(tbl_auth_server_t   *pItem)
{
    char szServer[128];
    
    cdb_auth_server_key_val2str(szServer, 128, &pItem->key);

    AUTHD_EVENT_DEBUG( 
                "auth server %s reawake (manually)", szServer);
}
/* liwh end */

int
auth_server_check_ip(addr_t *pAddr)
{
    u_int32_t network;
   
    if (AF_INET == pAddr->family) {
        u_char *p =  (u_char *)&pAddr->u.prefix4;
        
        /* filtrate ip with 255.255.255.255 */
        if (255 == p[0] && 255 == p[1] && 255 == p[2] && 255 == p[3]) {
            return -1;
        }
        
        network = ntohl (*(int*)&pAddr->u.prefix4);
        if (network == 0 || IN_BADCLASS(network) || IN_MULTICAST (network)
            || IN_EXPERIMENTAL(network) || IPV4_NET127(network)
            || IPV4_NET0(network)) {
            return -1;
        }
    } else if (IN6_IS_ADDR_MULTICAST (&pAddr->u.prefix6)
               || IN6_IS_ADDR_LOOPBACK (&pAddr->u.prefix6)
               || IN6_IS_ADDR_UNSPECIFIED (&pAddr->u.prefix6)) {
        return -1;
    }
    
    return 0;
}

int32
auth_check_str(tbl_auth_server_t *pitem, char *name, int32 max_len, char *err_desc)
{
    char              *p = NULL;
    char              passwd[PASSWD_SIZE];
    char              *q = passwd;

    /* modified by yejl for bug 39511, 2016-12-15 */
    /* If the type is tacplus, check the key string; if the type is radius, don't check the key string*/
    if (NULL != pitem)
    {
        if (pitem->key.type == AUTH_SERVER_TYPE_TACPLUS)
        {
            /* check the chars */
            p = name;
            while ('\0' != *p)
            {
                if ((*p == '%')||(*p == '`'))
                {
                    sal_sprintf(err_desc, "The key can include 0-9A-Za-z~!@#$^&*()_+-={}[]|:;<>,.");
                    return CDB_NAME_ERR_INVALID_CHAR;
                }
                
                p++;
            }

            p = name;
            while ('\0' != *p)
            {
                if ('\\' != *p)
                {
                    *q = *p;
                    p++;
                    q++;
                }
                else
                {
                    p++;
                }
            }
            *q = '\0';

            if (sal_strlen(passwd) > max_len)
            {
                sal_sprintf(err_desc, "Invalid shared key! its length range should be [1, 64]");
                return CDB_NAME_ERR_EXCEED_LEN;
            }

            sal_memset(name, 0, sal_strlen(name));
            sal_memcpy(name, passwd, sal_strlen(passwd));            
        }
        else if (pitem->key.type == AUTH_SERVER_TYPE_RADIUS)
        {
            /* check the string length */
            if (sal_strlen(name) > max_len)
            {
                sal_sprintf(err_desc, "String length should not exceed %u", max_len);
                return CDB_NAME_ERR_EXCEED_LEN;
            }
        }
    }
    /*end with yejl*/
    
   
/* modified by liwh for bug 39373, 2016-07-28 */
#if 0    
    /* check the chars */
    p = (char *)name;
    while ('\0' != *p)
    {
        if ((sal_isdigit(*p)) 
            || (sal_isalpha(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p)
            || ('!' == *p)
            || ('$' == *p))
        {
            p++;
            continue;
        }
        
        sal_sprintf(err_desc, "Character should be \"0-9A-Za-z.-_\"");
        return CDB_NAME_ERR_INVALID_CHAR;
    }
#endif

    return CDB_NAME_ERR_NONE;
}

/* --- authentication server --- */
static int
_check_auth_server_field(char *pszkey, char *pszValue, tbl_auth_server_t *pitem, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_AUTH_SERVER);
    cdb_field_t *p_field = NULL;
    int32 field_id;
    int nValue;
    char              *q = NULL;
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }
    if (pitem == NULL)
    {
        return PM_E_NOT_EXIST;
    }
    switch ((tbl_auth_server_field_id_t)field_id) {
    case TBL_AUTH_SERVER_FLD_SECRET:
    case TBL_AUTH_SERVER_FLD_ENC_SECRET:
        if (auth_check_str(pitem, pszValue, 
                (field_id == TBL_AUTH_SERVER_FLD_SECRET) ? GLB_AUTH_SECRET_SIZE : (PASSWD_SIZE - 1),
                 para->p_rs->desc) < 0) {
            goto err_out;
        }

        if (TBL_AUTH_SERVER_FLD_ENC_SECRET == field_id) {
            char szTmp[PASSWD_SIZE];
            if (0 == sal_strlen(pszValue))
            {
                snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid key string");
                goto err_out;
            }
            if (do_decrypt(M_DEFAULT_PW_STR, pszValue, strlen(pszValue),
						   szTmp, PASSWD_SIZE) != 0) {
				snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid encrypt password");
				goto err_out;
			}
			q = szTmp;
            while ('\0' != *q)
            {
                if (!isprint(*q)) 
                {
                    CFG_FUNC_ERR_RET("Invalid password string");
                }
                q++;
            }
            snprintf(pitem->enc_secret, PASSWD_SIZE, "%s", pszValue);
            memset(pitem->secret, 0x00, GLB_AUTH_SECRET_SIZE + 1);
            
        } else {
            if (p_db_glb->service_password_encryption) {
                char szTmp[PASSWD_SIZE];
                gen_passwd_crypt(pszValue, szTmp, PASSWD_SIZE);
                snprintf(pitem->enc_secret, PASSWD_SIZE, "%s", szTmp);
                memset(pitem->secret, 0x00, GLB_AUTH_SECRET_SIZE + 1);
            } else {
                memset(pitem->enc_secret, 0x00, PASSWD_SIZE);
                snprintf(pitem->secret, GLB_AUTH_SECRET_SIZE + 1, "%s", pszValue);
            }
        }
        break;
    case TBL_AUTH_SERVER_FLD_PORT:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue) 
                || nValue < 1 || nValue > 65535) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid authentication server port"); 
            goto err_out;
        }
        pitem->port = nValue;
        break;
    case TBL_AUTH_SERVER_FLD_TIMEOUT:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue) 
                || nValue < GLB_AUTH_TIMEOUT_MIN
                || nValue > GLB_AUTH_TIMEOUT_MAX) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid authentication timeout seconds"); 
            goto err_out;
        }
        pitem->timeout = nValue;
        break;

    case TBL_AUTH_SERVER_FLD_RETRIES:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue) 
                || nValue < GLB_AUTH_RETRIES_MIN
                || nValue > GLB_AUTH_RETRIES_MAX) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid authentication retry times"); 
            goto err_out;
        }
        pitem->retries = nValue;
        break;
    case TBL_AUTH_SERVER_FLD_DEAD:
        nValue = atoi(pszValue);
        if (0 != nValue) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid authentication dead value"); 
            goto err_out;
        }
        pitem->dead = nValue;
        break;
    case TBL_AUTH_SERVER_FLD_IS_INBAND:
        pitem->is_inband = (0 == sal_strcmp("inband", pszValue)) ? TRUE : FALSE;
        break;
    case TBL_AUTH_SERVER_FLD_REAWAKE_TIME:
    case TBL_AUTH_SERVER_FLD_MAX:
    case TBL_AUTH_SERVER_FLD_KEY:
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid parameters %s", pszValue); 
        goto err_out;
        break;
    }
    
    return field_id;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
tbl_auth_server_dump_filter(tbl_auth_server_t *p_auth, tbl_iter_args_t *pargs)
{
    tbl_auth_server_t *p_filter = (tbl_auth_server_t *)pargs->argv[2];

    if ((NULL == p_filter || p_auth == p_filter)
        //&& (AUTH_SERVER_TYPE_TACPLUS == p_auth->key.type)
        ) {
        return tbl_auth_server_dump_one(p_auth, pargs);
    }

    return PM_E_NONE;
}

static int32
_auth_server_cmd_get(tbl_auth_server_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = pitem;
    tbl_auth_server_iterate((TBL_ITER_CB_FUNC)tbl_auth_server_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
auth_server_set(tbl_auth_server_field_id_t field_id, tbl_auth_server_t *pitem)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    switch (field_id) {
    case TBL_AUTH_SERVER_FLD_SECRET:
    case TBL_AUTH_SERVER_FLD_ENC_SECRET:
    case TBL_AUTH_SERVER_FLD_PORT:
    case TBL_AUTH_SERVER_FLD_TIMEOUT:
    case TBL_AUTH_SERVER_FLD_RETRIES:
    case TBL_AUTH_SERVER_FLD_DEAD:
    case TBL_AUTH_SERVER_FLD_REAWAKE_TIME:
    case TBL_AUTH_SERVER_FLD_IS_INBAND:
        break;
    case TBL_AUTH_SERVER_FLD_KEY:
    case TBL_AUTH_SERVER_FLD_MAX:
        return PM_E_NONE;
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_auth_server_set_auth_server_field(pitem, field_id));
    return rc;
}

static int32
_auth_server_set_secret(tbl_auth_server_t *pitem, 
    tbl_auth_server_t *p_db_item, cfg_cmd_para_t *para)
{
    char szTmp[PASSWD_SIZE];
    int32 rc = PM_E_NONE;

    if ('\0' != pitem->secret[0] && app_is_password_encryption()) {
        gen_passwd_crypt(pitem->secret, szTmp, PASSWD_SIZE);
        snprintf(pitem->enc_secret, PASSWD_SIZE, "%s", szTmp);
        memset(pitem->secret, 0x00, GLB_AUTH_SECRET_SIZE + 1);
    }

    if (strcasecmp(pitem->secret, p_db_item->secret)) {
        rc = tbl_auth_server_set_auth_server_field(pitem, TBL_AUTH_SERVER_FLD_SECRET);
    }

    if (strcasecmp(pitem->enc_secret, p_db_item->enc_secret)) {
        rc |= tbl_auth_server_set_auth_server_field(pitem, TBL_AUTH_SERVER_FLD_ENC_SECRET);
    }
    return rc;
}


static int32
_auth_server_cmd_set(tbl_auth_server_field_id_t field_id, tbl_auth_server_t *pitem, 
    tbl_auth_server_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (field_id) {
    case TBL_AUTH_SERVER_FLD_SECRET:
        if (!sal_strcmp(pitem->secret, p_db_item->secret)) {
            return PM_E_NONE;
        }
        break;
    case TBL_AUTH_SERVER_FLD_ENC_SECRET:
        if (!sal_strcmp(pitem->enc_secret, p_db_item->enc_secret)) {
            return PM_E_NONE;
        }
        break;
    case TBL_AUTH_SERVER_FLD_PORT:
        if (pitem->port == p_db_item->port) {
            return PM_E_NONE;
        }
        break;
    case TBL_AUTH_SERVER_FLD_TIMEOUT:
        if (pitem->timeout == p_db_item->timeout) {
            return PM_E_NONE;
        }
        break;
    case TBL_AUTH_SERVER_FLD_RETRIES:
        if (pitem->retries == p_db_item->retries) {
            return PM_E_NONE;
        }
        break;
    case TBL_AUTH_SERVER_FLD_DEAD:
        if (pitem->dead == p_db_item->dead) {
            return PM_E_NONE;
        }

        /* modified by liwh for bug 46709, 2018-05-24 */
        if (p_db_item->dead && (AUTH_SERVER_TYPE_TACPLUS == p_db_item->key.type))
        {
            auth_reactivate_debug_info(p_db_item);
        }
         /* liwh end */
        break;    
    case TBL_AUTH_SERVER_FLD_REAWAKE_TIME:
        if (pitem->reawake_time == p_db_item->reawake_time) {
            return PM_E_NONE;
        }
        break;        

    case TBL_AUTH_SERVER_FLD_MAX:
    case TBL_AUTH_SERVER_FLD_KEY:
        break;

    default:
        break;
    }

    if (TBL_AUTH_SERVER_FLD_SECRET == field_id 
            || TBL_AUTH_SERVER_FLD_ENC_SECRET == field_id) {
        return _auth_server_set_secret(pitem, p_db_item, para);
    } 

    PM_E_RETURN(auth_server_set(field_id, pitem));
    return rc;
}

static int32
_auth_server_add(auth_server_key_t *pkey, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_auth_server_master_t *pmaster;
    tbl_auth_server_t auth_server;
    tbl_auth_server_t *p_auth_server = &auth_server;
    int i;
    
    if (tbl_auth_server_get_auth_server(pkey) != NULL) {
        return PM_E_NONE;
    }
    
    sal_memset(p_auth_server, 0x00, sizeof(tbl_auth_server_t));
    sal_memcpy(&p_auth_server->key, pkey, sizeof(auth_server_key_t));

    for (i = 1; i < argc; i += 2) {
        if (_check_auth_server_field(argv[i], argv[i + 1], p_auth_server, para) == -1) {
            return PM_E_CFG_INVALID_PARAM; 
        }
    }

    pmaster = tbl_auth_server_get_master();

    AUTHD_EVENT_DEBUG("_auth_server_add pkey->type is %d pmaster->radius_cnt is %d pmaster->tacplus_cnt is %d", 
        pkey->type, pmaster->radius_cnt, pmaster->tacplus_cnt);
    
    if ((AUTH_SERVER_TYPE_RADIUS == pkey->type && pmaster->radius_cnt >= GLB_AUTH_RADIUS_SERVER_NUM)
        || (AUTH_SERVER_TYPE_TACPLUS == pkey->type && pmaster->tacplus_cnt >= GLB_AUTH_TACPLUS_SERVER_NUM)) {
        /* modified by liwh for bug 43590, 2018-02-07 */
        //CFG_CONFLICT_RET("Exceeding maximum authentication server limit");
        
        if (AUTH_SERVER_TYPE_RADIUS == pkey->type)
        {
            CFG_CONFLICT_RET("No more than 3 RADIUS servers are supported");
        }
        else
        {
            CFG_CONFLICT_RET("No more than 3 TACPLUS servers are supported");
        }
        /* liwh end */
    }

    /* setting ports */
    if (AUTH_SERVER_TYPE_RADIUS == pkey->type) {
        p_auth_server->port = pkey->sin_port;
    } else {
        p_auth_server->port = pkey->sin_port;
    }
    for (i = 1; i < argc; i += 2) {
        if (_check_auth_server_field(argv[i], argv[i + 1], p_auth_server, para) == -1) {
            goto err_out;
        }
    }    

    if (AUTH_SERVER_TYPE_RADIUS == pkey->type) {
        pmaster->radius_cnt++;
    } else {
        pmaster->tacplus_cnt++;
    }
    pmaster->cur_id++;
    p_auth_server->id = pmaster->cur_id;
    AUTHD_EVENT_DEBUG("add authentication server %s", argv[0]);
    return tbl_auth_server_add_auth_server(p_auth_server);

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return PM_E_CFG_INVALID_PARAM;
}

static int32
_auth_server_del(auth_server_key_t *pkey, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_auth_server_t *p_auth_server = tbl_auth_server_get_auth_server(pkey);
    tbl_auth_server_master_t *pmaster = tbl_auth_server_get_master();

    if (NULL == p_auth_server) {
        CFG_INVALID_PARAM_RET("No such authentication server");
    }

    if (NULL != p_auth_server->pRead) {
        ctc_task_delete_read(p_auth_server->pRead);
    }

    AUTHD_EVENT_DEBUG("_auth_server_del p_auth_server->key.type is %d pmaster->radius_cnt is %d pmaster->tacplus_cnt is %d", 
        p_auth_server->key.type, pmaster->radius_cnt, pmaster->tacplus_cnt);

    if (AUTH_SERVER_TYPE_RADIUS == p_auth_server->key.type) {
        pmaster->radius_cnt--;
    } else {
        pmaster->tacplus_cnt--;
    }

    AUTHD_EVENT_DEBUG("del authentication server %s", argv[0]);
    return tbl_auth_server_del_auth_server(pkey);
}

int32
reactivate_tacacs_server(void)
{
    int32                     rc = PM_E_NONE;    
    ctclib_slistnode_t       *listnode = NULL;
    tbl_auth_server_master_t *p_master = NULL;
    tbl_auth_server_t        *pItem = NULL;   

    p_master = tbl_auth_server_get_master();
    if (!p_master)
    {
        AUTHD_ERR_PRINT("reactivate_tacacs_server can't get tbl_auth_server master"); 
        return rc;
    }
    
    CTCLIB_SLIST_LOOP(p_master->auth_servers, pItem, listnode) 
    {
        if (pItem->dead && (AUTH_SERVER_TYPE_TACPLUS == pItem->key.type))
        {
            pItem->dead = FALSE;

            /* modified by liwh for bug 46709, 2018-05-24 */
            auth_reactivate_debug_info(pItem);
            /* liwh end */
        }
    }

    return rc;
}

int32
auth_server_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 field_id = TBL_AUTH_SERVER_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    uint32 is_inband = FALSE;
    int32 rc = PM_E_NONE;
    tbl_auth_server_t item;
    tbl_auth_server_t *p_db_item = NULL;
    char ip_addr[IPADDR_BUF_SIZE] = {0};

    CFG_DBG_FUNC();
    
    p_node = cdb_get_tbl(TBL_AUTH_SERVER);
    
    /* modified by liwh for bug 38812, 2016-06-06 */
    if ((argc == 2) && (0 == sal_memcmp(argv[0], "all", 3))
        && (0 == sal_memcmp(argv[1], "dead", 4)))
    {
        reactivate_tacacs_server();
        return rc;
    }
    /* liwh end */

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        sal_memset(&item, 0, sizeof(tbl_auth_server_t));
        rc = cdb_auth_server_key_str2val(&item.key, argv[0], para->p_rs->desc);
        if (rc < 0) {
            CFG_INVALID_PARAM_RET("Invalid authentication server key %s", argv[0]);
        }
        if (auth_server_check_ip(&item.key.addr) != 0) {
            CFG_INVALID_PARAM_RET("Invalid authentication server ip address");
        }

        p_db_item = tbl_auth_server_get_auth_server(&item.key);
    }
    switch (para->oper) {
    case CDB_OPER_ADD:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        if (NULL != p_db_item) {
            cdb_addr_val2str(ip_addr, 47, &item.key.addr);
            CFG_INVALID_PARAM_RET("Create tacacs server %s auth_port %d failure when it is exist", ip_addr, item.key.sin_port);
        }        
        rc = _auth_server_add(&item.key, argv, argc, para);
        break;

    case CDB_OPER_DEL:
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("TACACS host does not exist");
        }
        if (argc != 3)
        {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
        is_inband = (0 == sal_strcmp("inband", argv[2])) ? TRUE : FALSE;
        if (is_inband != p_db_item->is_inband)
        {
            CFG_INVALID_PARAM_RET("The mgmt-if is mismatch");
        }
        rc = _auth_server_del(&p_db_item->key, argv, argc, para);
        break;

    case CDB_OPER_SET:
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("TACACS host does not exist");
        }
        
        if (2 == argc) {
            if (!strcasecmp(argv[1], "secret") || !strcasecmp(argv[1], "enc_secret")) { 
                field_id = TBL_AUTH_SERVER_FLD_SECRET;
                memset(item.secret, 0x00, GLB_AUTH_SECRET_SIZE+1);
                memset(item.enc_secret, 0x00, PASSWD_SIZE);
                return _auth_server_set_secret(&item, p_db_item, para);             
            }
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_auth_server_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM; 
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            rc = _auth_server_cmd_set(field_id, &item, p_db_item, para);
        }
        break;

    case CDB_OPER_GET:
        if (argc > 0 && NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("TACACS host does not exist");
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _auth_server_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}


int32
auth_server_dot1x_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 field_id = TBL_AUTH_SERVER_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_auth_server_t item;
    tbl_auth_server_t *p_db_item = NULL;

    CFG_DBG_FUNC();
    
    p_node = cdb_get_tbl(TBL_AUTH_SERVER);

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        sal_memset(&item, 0, sizeof(tbl_auth_server_t));
        rc = cdb_auth_server_key_str2val(&item.key, argv[0], para->p_rs->desc);
        if (rc < 0) {
            CFG_INVALID_PARAM_RET("Invalid authentication server key %s", argv[0]);
        }
        if (auth_server_check_ip(&item.key.addr) != 0) {
            CFG_INVALID_PARAM_RET("Invalid authentication server ip address");
        }

        p_db_item = tbl_auth_server_get_auth_server(&item.key);
    }
    switch (para->oper) {
    case CDB_OPER_ADD:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        if (NULL != p_db_item) {
            CFG_INVALID_PARAM_RET("Authentication server %s exist", argv[0]);
        }        
        rc = _auth_server_add(&item.key, argv, argc, para);
        break;

    case CDB_OPER_DEL:
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Authentication server %s not found", argv[0]);
        }
        rc = _auth_server_del(&p_db_item->key, argv, argc, para);
        break;

    case CDB_OPER_SET:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Authentication server %s not found", argv[0]);
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_auth_server_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM; 
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            rc = _auth_server_cmd_set(field_id, &item, p_db_item, para);
        }
        break;

    case CDB_OPER_GET:
        if (argc > 0 && NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Authentication server %s not found", argv[0]);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _auth_server_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

tbl_auth_server_t *auth_server_get_first(auth_server_type_t eType)
{
    tbl_auth_server_master_t *p_master = tbl_auth_server_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_auth_server_t *pItem = NULL;
    char szServer[128];

    CTCLIB_SLIST_LOOP(p_master->auth_servers, pItem, listnode) {
        if (0 == pItem->dead && eType == pItem->key.type) {
            cdb_auth_server_key_val2str(szServer, 128, &pItem->key);
            AUTHD_EVENT_DEBUG(
                "get the first auth server: id %u %s", pItem->id, szServer);
            return pItem;
        }
    }

    AUTHD_EVENT_DEBUG( "can't get the first auth server");
    return NULL;
}

tbl_auth_server_t *auth_server_get_next(u_int32_t nCurId, auth_server_type_t eType)
{
    tbl_auth_server_master_t *p_master = tbl_auth_server_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_auth_server_t *pItem = NULL;
    char szServer[128];

    CTCLIB_SLIST_LOOP(p_master->auth_servers, pItem, listnode) {
        if (eType == pItem->key.type && pItem->id >= nCurId) {
            /* >= for server item deleted */
            break;
        }
    }

    if (NULL == listnode) {
        AUTHD_EVENT_DEBUG( 
                "can't get the next auth server");
        return NULL;
    }

    listnode = listnode->next;
    while (listnode) {
        pItem = (tbl_auth_server_t *)listnode->data;
        /* modified by liwh for bug 38806, 2016-05-31 */
        if (pItem && (eType == pItem->key.type) && (nCurId != pItem->id && 0 == pItem->dead)) {
        //if (pItem && (nCurId != pItem->id && 0 == pItem->dead)) {
        /* liwh end */
            cdb_auth_server_key_val2str(szServer, 128, &pItem->key);
            AUTHD_EVENT_DEBUG( 
                "get the next auth server: id %u %s", pItem->id, szServer);
            return pItem;
        }
        listnode = listnode->next;
    }

    AUTHD_EVENT_DEBUG( 
            "can't get the next auth server");    
    return NULL;
}

tbl_auth_server_t *auth_server_get_by_id(u_int32_t nId)
{
    tbl_auth_server_master_t *p_master = tbl_auth_server_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_auth_server_t *pItem = NULL;

    CTCLIB_SLIST_LOOP(p_master->auth_servers, pItem, listnode) {
        if (nId == pItem->id) {
            char szServer[128];
            cdb_auth_server_key_val2str(szServer, 128, &pItem->key);
            AUTHD_EVENT_DEBUG(
                    "get auth server by id %u %s", nId, szServer);
            return pItem;
        }
    }

    AUTHD_EVENT_DEBUG(
            "can't get auth server by id %u", nId);
    return NULL;
}

void auth_server_sock_close(tbl_auth_server_t *pSrv)
{
    if (NULL != pSrv) {
        char szServer[128];
        cdb_auth_server_key_val2str(szServer, 128, &pSrv->key);    
        AUTHD_EVENT_DEBUG( 
                "close socket %d to server id %u %s", pSrv->fd, pSrv->id, szServer);
                
        if (NULL != pSrv->pRead) {
            ctc_task_delete_read(pSrv->pRead);
            pSrv->pRead = NULL;
        }

        pSrv->blen = 0;
        if (pSrv->fd >= 0) {
            close(pSrv->fd);
        }
        pSrv->fd = -1;
        pSrv->connected = 0;
    }
    return;
}

int set_auth_server_status(tbl_auth_server_t *pItem, int nDead)
{
    tbl_auth_server_t stItem;
    char szServer[128];

    if (!pItem)
    {
        return 0;    
    }

    cdb_auth_server_key_val2str(szServer, 128, &pItem->key);

    memset(&stItem, 0x00, sizeof(tbl_auth_server_t));
    memcpy(&stItem.key, &pItem->key, sizeof(auth_server_key_t));
    stItem.dead = nDead ? 1 : 0;
    auth_server_set(TBL_AUTH_SERVER_FLD_DEAD, &stItem);
    if (nDead) {
        tbl_auth_cfg_t *pGlobalCfg = tbl_auth_cfg_get_auth_cfg();
        int nDeadTime = pGlobalCfg->deadtime > 0 ? 
            pGlobalCfg->deadtime : GLB_AUTH_DEFAULT_DEADTIME;

        stItem.reawake_time = time(NULL) + nDeadTime;
        auth_server_set(TBL_AUTH_SERVER_FLD_REAWAKE_TIME, &stItem);
        AUTHD_EVENT_DEBUG(
                "auth server %s dead %d seconds", szServer, nDeadTime);
    } else {
        AUTHD_EVENT_DEBUG( 
                "auth server %s reawake", szServer);
    }
    return 0;
}

void auth_server_dead_timer(void *pArg)
{
    tbl_auth_server_master_t *p_master = tbl_auth_server_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_auth_server_t *pItem = NULL;
    time_t tCurrent = time(NULL);

    g_p_dead_timer = NULL;

    CTCLIB_SLIST_LOOP(p_master->auth_servers, pItem, listnode) {
        if (pItem->dead && pItem->reawake_time <= tCurrent) {
            set_auth_server_status(pItem, 0);
        }
    }

    g_p_dead_timer = ctc_task_add_timer(TASK_PRI_NORMAL,
            auth_server_dead_timer, NULL, 1);
    return;
}

/* --- authentication server --- */
static int
_check_auth_session_field(char *pszkey, char *pszValue, tbl_auth_session_t *pitem, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_AUTH_SESSION);
    cdb_field_t *p_field = NULL;
    int32 field_id;
    char szPasswd[PASSWD_SIZE];

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_auth_session_field_id_t)field_id) {
    case TBL_AUTH_SESSION_FLD_USER:
        if (cdb_user_str2val(&pitem->user, pszValue, para->p_rs->desc) == -1) {
            goto err_out;
        }
        break;
    case TBL_AUTH_SESSION_FLD_PASSWD:
        if (sal_b64_pton((const char *)pszValue, 
                (unsigned char *)szPasswd, PASSWD_SIZE) == -1) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid password string"); 
            goto err_out;
        
        }
        snprintf(pitem->passwd, PASSWD_SIZE * 2, "%s", pszValue);
        break;
    case TBL_AUTH_SESSION_FLD_AUTHOR_START:
        pitem->author_start = atoi(pszValue);
        break;
    case TBL_AUTH_SESSION_FLD_ACCT_START:
        pitem->acct_start = atoi(pszValue);
        break;    
    case TBL_AUTH_SESSION_FLD_KEY:
    case TBL_AUTH_SESSION_FLD_STATUS:
    case TBL_AUTH_SESSION_FLD_MAX:
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid parameters %s/%s",
                pszkey, pszValue); 
        goto err_out;
        break;
    default:
        break;
    }
    
    return field_id;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
tbl_auth_session_dump_filter(tbl_auth_session_t *p_auth, tbl_iter_args_t *pargs)
{
    tbl_auth_session_t *p_filter = (tbl_auth_session_t *)pargs->argv[2];

    if (NULL == p_filter || p_auth == p_filter) {
        return tbl_auth_session_dump_one(p_auth, pargs);
    }

    return PM_E_NONE;
}

static int32
_auth_session_cmd_get(tbl_auth_session_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = pitem;
    tbl_auth_session_iterate((TBL_ITER_CB_FUNC)tbl_auth_session_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
auth_session_set(tbl_auth_session_field_id_t field_id, tbl_auth_session_t *pitem)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    switch (field_id) {
    case TBL_AUTH_SESSION_FLD_USER:
    case TBL_AUTH_SESSION_FLD_PASSWD:
    case TBL_AUTH_SESSION_FLD_STATUS:
        break;
    case TBL_AUTH_SESSION_FLD_KEY:
    case TBL_AUTH_SESSION_FLD_MAX:
        return PM_E_NONE;
    default:
        return PM_E_NONE;
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_auth_session_set_auth_session_field(pitem, field_id));
    return rc;
}

static int32
_auth_session_cmd_set(tbl_auth_session_field_id_t field_id, tbl_auth_session_t *pitem, 
    tbl_auth_session_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (field_id) {
    case TBL_AUTH_SESSION_FLD_USER:
        if (!strcmp(pitem->user.name, p_db_item->user.name)) {
            return PM_E_NONE;
        }
        break;
    case TBL_AUTH_SESSION_FLD_PASSWD:
        if (!strcmp(pitem->passwd, p_db_item->passwd)) {
            return PM_E_NONE;
        }
        break;
    case TBL_AUTH_SESSION_FLD_STATUS:
        if (pitem->status == p_db_item->status) {
            return PM_E_NONE;
        }
        break;
    case TBL_AUTH_SESSION_FLD_MAX:
    case TBL_AUTH_SESSION_FLD_KEY:
        break;
    default:
        break;
    }

    PM_E_RETURN(auth_session_set(field_id, pitem));
    
    return rc;
}

int
auth_get_shared_secret(tbl_auth_server_t *pSrv, char *pszOut, size_t nBufLen)
{
    tbl_auth_cfg_t *pGlobalCfg = tbl_auth_cfg_get_auth_cfg();;
    char *pszSecret = NULL;
    char *pszEncSecret = NULL;

    *pszOut = '\0';

    if (NULL != pSrv) {
        pszSecret = pSrv->secret;
        pszEncSecret = pSrv->enc_secret;
    } else if (AUTH_SERVER_TYPE_TACPLUS != pSrv->key.type) {
        /* liwh: global shared secret is only for radius */
        pszSecret = pGlobalCfg->secret;
        pszEncSecret = pGlobalCfg->enc_secret;
    }

    if (NULL != pszSecret && '\0' != pszSecret[0]) {
        snprintf(pszOut, nBufLen, "%s", pszSecret);
    } else if (NULL != pszEncSecret && '\0' != pszEncSecret[0]) {
        do_decrypt(M_DEFAULT_PW_STR, pszEncSecret, strlen(pszEncSecret),
			       pszOut, nBufLen);
    }
    return 0;
}

int
auth_get_retries(tbl_auth_server_t *pSrv)
{
    tbl_auth_cfg_t *pGlobalCfg;
    
    if (pSrv && (0 != pSrv->retries)) {
        return pSrv->retries;
    }

    pGlobalCfg = tbl_auth_cfg_get_auth_cfg();
    return pGlobalCfg->retries;
}

int
auth_get_timeout(tbl_auth_server_t *pSrv)
{
    tbl_auth_cfg_t *pGlobalCfg;
    
    if (pSrv && (0 != pSrv->timeout)) {
        return pSrv->timeout;
    }

    pGlobalCfg = tbl_auth_cfg_get_auth_cfg();
    return pGlobalCfg->timeout;
}

auth_handle_t *auth_handle_calloc(void)
{
    auth_handle_t *pHandle = XCALLOC(MEM_TBL_AUTH_SESSION, sizeof(auth_handle_t));
    if (NULL == pHandle) {
        log_sys(M_MOD_AUTHD, E_ALERT, "Out of memory"); 
    } 

    AUTHD_EVENT_DEBUG(
            "calloc authentication handle %p", pHandle);    
    return pHandle;
}

void
auth_handle_release(auth_handle_t *pHandle)
{
    auth_be_op_t *beop;

    AUTHD_EVENT_DEBUG(
            "free authentication handle %p", pHandle);
    if (NULL != pHandle) {
        if (NULL != pHandle->pTimeout) {
            ctc_task_delete_timer(pHandle->pTimeout);
        }
        if (NULL != pHandle->pConnect) {

            /* modified by liwh for bug 38572, 2016-05-18 */
            if (TASK_TASK == pHandle->pConnect->type)
            {
                ctc_task_delete_task(pHandle->pConnect);
            }
            else
            {
                ctc_task_delete_write(pHandle->pConnect);
            }
            /* liwh end */
        }

        pHandle->pTimeout = NULL;
        pHandle->pConnect = NULL;        

        beop = pHandle->pCBS;
        if (beop && beop->release) {
            beop->release(pHandle);
        }

        XFREE(MEM_TBL_AUTH_SESSION, pHandle);
    }    
    return;
}

void auth_inprogress(tbl_auth_session_t *pSess, char *pszMsg)
{
    tbl_auth_session_t pItemRet;
    char szSession[128];

    cdb_auth_session_key_val2str(szSession, 128, &pSess->key);
    AUTHD_EVENT_DEBUG(
            "authentication session %s is in progress",
            szSession);
    
    memcpy(&pItemRet.key, &pSess->key, sizeof(tbl_auth_session_key_t));
    pItemRet.status = AUTH_SESSION_STATUS_INPROGRESS;
    auth_session_set(TBL_AUTH_SESSION_FLD_STATUS, &pItemRet);
    return;
}

void auth_accomplished(tbl_auth_session_t *pSess, char *pszMsg)
{
    tbl_auth_session_t pItemRet;
    auth_handle_t *pHandle;
    char szSession[128];

    cdb_auth_session_key_val2str(szSession, 128, &pSess->key);
    AUTHD_EVENT_DEBUG( 
            "authentication session %s is accomplished",
            szSession);

    if (pSess->acct_stop || pSess->acctcmd_start)
    {
        authd_delete_account_stop_session(pSess);
        return;
    }
    
    memcpy(&pItemRet.key, &pSess->key, sizeof(tbl_auth_session_key_t));
    pItemRet.status = AUTH_SESSION_STATUS_SUCCESS;
    auth_session_set(TBL_AUTH_SESSION_FLD_STATUS, &pItemRet);

    pHandle = (auth_handle_t *)pSess->pHandle;
    if (NULL != pHandle) {
        auth_handle_release(pHandle);
        pSess->pHandle = NULL;
    }
    
    return;
}

void auth_failed(tbl_auth_session_t *pSess, char *pErrMsg)
{
    tbl_auth_session_t pItemRet;
    auth_handle_t *pHandle;
    char szSession[128];

    cdb_auth_session_key_val2str(szSession, 128, &pSess->key);
    AUTHD_EVENT_DEBUG( 
            "authentication session %s is failed: %s",
            szSession, pErrMsg);

    if (pSess->is_dot1x_session)
    {
        logid_sys(LOG_DOT1X_4_FAIL, pSess->user.name, "Fail");
    }
    else
    {
        if (pSess->acct_start)
        {
            logid_sys(LOG_AAA_4_FAIL, pSess->user.name, "Accounting", "Fail");
        }
        else if (pSess->author_start)
        {
            logid_sys(LOG_AAA_4_FAIL, pSess->user.name, "Authorization", "Fail");
        }
        else if (!pSess->acct_stop && !pSess->acctcmd_start)
        {
            logid_sys(LOG_AAA_4_FAIL, pSess->user.name, "Authentication", "Fail");
        }
    }

    if (pSess->acct_stop || pSess->acctcmd_start)
    {
        /* modified by liwh for bug 41308, 2017-01-17 
          for acct_stop, should find next account mode */
        if (pSess->acct_stop)
        {
            authd_send_next_account_stop(pSess);
        }
        /* liwh end */
        
        authd_delete_account_stop_session(pSess);
        return;
    }
            
    memcpy(&pItemRet.key, &pSess->key, sizeof(tbl_auth_session_key_t));
    pItemRet.status = AUTH_SESSION_STATUS_FAILED;
    auth_session_set(TBL_AUTH_SESSION_FLD_STATUS, &pItemRet);

    pHandle = (auth_handle_t *)pSess->pHandle;
    if (NULL != pHandle) {
        auth_handle_release(pHandle);
        pSess->pHandle = NULL;
    }
    
    if (pSess->is_dot1x_session)
    {
        dot1x_radius_release_session(pSess);    
    }
        
    return;
}

/* modified by liwh for bug 37986, 2016-06-01 */
void auth_denied(tbl_auth_session_t *pSess, char *pErrMsg)
{
    tbl_auth_session_t pItemRet;
    auth_handle_t *pHandle;
    char szSession[128];

    cdb_auth_session_key_val2str(szSession, 128, &pSess->key);
    AUTHD_EVENT_DEBUG( 
            "authentication session %s is denied: %s",
            szSession, pErrMsg);

    if (pSess->is_dot1x_session)
    {
        logid_sys(LOG_DOT1X_4_FAIL, pSess->user.name, "Denied");
    }
    else
    {
        if (pSess->acct_start)
        {
            logid_sys(LOG_AAA_4_FAIL, pSess->user.name, "Accounting", "Denied");
        }
        else if (pSess->author_start)
        {
            logid_sys(LOG_AAA_4_FAIL, pSess->user.name, "Authorization", "Denied");
        }
        else if (!pSess->acct_stop && !pSess->acctcmd_start)
        {
            logid_sys(LOG_AAA_4_FAIL, pSess->user.name, "Authentication", "Denied");
        }
    }

    if (pSess->acct_stop || pSess->acctcmd_start)
    {
        authd_delete_account_stop_session(pSess);
        return;
    }
            
    memcpy(&pItemRet.key, &pSess->key, sizeof(tbl_auth_session_key_t));
    pItemRet.status = AUTH_SESSION_STATUS_DENIED;
    auth_session_set(TBL_AUTH_SESSION_FLD_STATUS, &pItemRet);

    pHandle = (auth_handle_t *)pSess->pHandle;
    if (NULL != pHandle) {
        auth_handle_release(pHandle);
        pSess->pHandle = NULL;
    }
    
    if (pSess->is_dot1x_session)
    {
        dot1x_radius_release_session(pSess);    
    }
        
    return;
}
/* liwh end */

tbl_auth_session_t *auth_session_get_by_id(u_int32_t uSessId)
{
    tbl_auth_session_master_t *p_master = tbl_auth_session_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_auth_session_t *p_item = NULL;
    
    CTCLIB_SLIST_LOOP(p_master->auth_sessions, p_item, listnode) {
        if (p_item->uCurSessId == uSessId) {
            char szSession[128];

            cdb_auth_session_key_val2str(szSession, 128, &p_item->key);
            AUTHD_EVENT_DEBUG( 
                    "get authentication session %s by id %u",
                    szSession, uSessId);
            return p_item;
        }
    }

    AUTHD_EVENT_DEBUG( 
        "get authentication session by id %u failed", uSessId);
    return NULL;
}

void auth_result_callback(tbl_auth_session_t *pSess, int nRet, char *pszFmt, ...)
{
    auth_handle_t *pHandle = (auth_handle_t *)pSess->pHandle;
    va_list ap;
    char szBuf[256];

    if (NULL == pszFmt) {
        snprintf(szBuf, 256, "%s", "");
    } else {
        va_start(ap, pszFmt);
        vsnprintf(szBuf, 256, pszFmt, ap);
        va_end(ap);
    }
    
    switch(nRet) {
    case M_AUTH_SUCCESS:
        auth_accomplished(pSess, szBuf);
        break;
    case M_AUTH_FAILED:
        /* modified by liwh for bug 37986, 2016-06-01 */
        // auth_failed(pSess, szBuf);
        auth_denied(pSess, szBuf);
        /* liwh end */
        break;   
    case M_AUTH_INPROGRESS:
        auth_inprogress(pSess, szBuf);
        break;
    case M_AUTH_ERROR:
        /* modified by liwh for bug 38228, 2016-05-30 */
        if (NULL != pHandle)
        {
            AUTHD_EVENT_DEBUG( 
                "stop pHandle %p pTimeout", pHandle);
            if (NULL != pHandle->pTimeout) {
                ctc_task_delete_timer(pHandle->pTimeout);
            }
            if (NULL != pHandle->pConnect) {
                if (TASK_TASK == pHandle->pConnect->type)
                {
                    ctc_task_delete_task(pHandle->pConnect);
                }
                else
                {
                    ctc_task_delete_write(pHandle->pConnect);
                }
            }

            pHandle->pTimeout = NULL;
            pHandle->pConnect = NULL;
        }
        /* liwh end */

        /* try next server */
        AUTHD_EVENT_DEBUG( 
            "process auth reply error, will try it later");
        pHandle->pConnect = ctc_task_add_task(TASK_PRI_NORMAL, 
            start_connect_cb, (void *)pSess);
        break;
    }
    
    return;
}

void auth_reply_read_cb(void *pArg)
{
    tbl_auth_server_t *pSrv = (tbl_auth_server_t *)pArg;
    auth_be_op_t *beop;
    char szServer[128];
    int nRet;

    if (AUTH_SERVER_TYPE_TACPLUS == pSrv->key.type) {
        beop = &tacplus_be_ops;
    } else {
        beop = &radius_be_ops;
    }

    cdb_auth_server_key_val2str(szServer, 128, &pSrv->key); 
    AUTHD_EVENT_DEBUG(
        "reply from server %s, try process it", szServer);
    nRet = beop->process_reply(pSrv, &auth_result_callback);
    if (M_AUTH_ERROR == nRet) {
        AUTHD_EVENT_DEBUG(  
            "process reply from server %s error, close socket", szServer);
        auth_server_sock_close(pSrv);
    }

    if (M_AUTH_SUCCESS == nRet) {
        AUTHD_EVENT_DEBUG(  
            "process reply from server %s success, close socket", szServer);
        auth_server_sock_close(pSrv);
    }
    return;
}


void auth_read_timeout_cb(void *pArg)
{
    tbl_auth_session_t *pSess = (tbl_auth_session_t *)pArg;
    auth_handle_t *pHandle = (auth_handle_t *)pSess->pHandle;
    tbl_auth_server_t *pSrv = auth_server_get_by_id(pHandle->nServerId);
    int nRetries = 0;
    char szServer[128];

    pHandle->pTimeout = NULL;
    
    if (NULL != pSrv) {
        nRetries = auth_get_retries(pSrv);
        
        cdb_auth_server_key_val2str(szServer, 128, &pSrv->key);   

        /* modified by liwh for bug 38228, 2016-05-30 
          if pSrv socket is closed, don't send request by this srv again */
        //if (pHandle->nRetries < nRetries) {
        if ((pHandle->nRetries < nRetries) && pSrv->pRead) {
        /* liwh end */
            AUTHD_EVENT_DEBUG(
                "wait reply from server %s timeout, try again (index %u)",
                szServer, pHandle->nRetries);
            pHandle->pTimeout = ctc_task_add_timer(TASK_PRI_NORMAL, 
                auth_read_timeout_cb, pSess, auth_get_timeout(pSrv));
            auth_do_request(pSess, pSrv);
            return;
        }

        AUTHD_EVENT_DEBUG(
            "wait reply from server %s timeout, try next server",
            szServer);
    }

    /* modified by liwh for bug 38228, 2016-05-30 */
    if (NULL != pHandle->pConnect) {
        if (TASK_TASK == pHandle->pConnect->type)
        {
            ctc_task_delete_task(pHandle->pConnect);
        }
        else
        {
            ctc_task_delete_write(pHandle->pConnect);
        }
    }

    pHandle->pConnect = NULL;
    /* liwh end */
    
    /* connect to next server if have */
    pHandle->pConnect = ctc_task_add_task(TASK_PRI_NORMAL, 
        start_connect_cb, (void *)pSess);
    return;
}

int
auth_do_request(tbl_auth_session_t *pSess, tbl_auth_server_t *pSrv)
{
    auth_handle_t *pHandle = (auth_handle_t *)pSess->pHandle;
    auth_be_op_t *beop = pHandle->pCBS;
    int nRet;
    char szServer[128];
    char szSession[128];

    cdb_auth_server_key_val2str(szServer, 128, &pSrv->key); 
    cdb_auth_session_key_val2str(szSession, 128, &pSess->key); 
    pHandle->nRetries++; 
    nRet = beop->send_request(pHandle, pSess, pSrv);
    if (M_AUTH_SEND_SUCCESS != nRet) {
        AUTHD_EVENT_DEBUG( 
                "session %s send request to server %s failed, try next server",
                szSession, szServer);

        /* modified by liwh for bug 38228, 2016-05-30 */
        if (NULL != pHandle->pTimeout) {
            ctc_task_delete_timer(pHandle->pTimeout);
        }
        
        if (NULL != pHandle->pConnect) {
            if (TASK_TASK == pHandle->pConnect->type)
            {
                ctc_task_delete_task(pHandle->pConnect);
            }
            else
            {
                ctc_task_delete_write(pHandle->pConnect);
            }
        }

        pHandle->pTimeout = NULL;
        pHandle->pConnect = NULL;
        /* liwh end */
    
        pHandle->pConnect = ctc_task_add_task(TASK_PRI_NORMAL, 
            start_connect_cb, (void *)pSess);
        return 0;
    }

    /* modified by liwh for bug 41308, 2017-01-17 
      acct_stop should wait for response or timeout */
    //if (pSess->acct_stop || pSess->acctcmd_start)
    if (pSess->acctcmd_start)
    /* liwh end */
    {
        authd_delete_account_stop_session(pSess);
        auth_server_sock_close(pSrv);
    }

    AUTHD_EVENT_DEBUG(
            "session %s send request to server %s success", 
            szSession, szServer);
    return 0;
}

void
auth_connect_check_cb(void *pArg)
{
    tbl_auth_session_t *pSess = (tbl_auth_session_t *)pArg;
    auth_handle_t *pHandle = (auth_handle_t *)pSess->pHandle;
    tbl_auth_server_t *pSrv = NULL;
    char szErrMsg[128];
    int nStatus = 0;
    unsigned int nErrLen;
    char szServer[128];
    char szSession[128];

    cdb_auth_session_key_val2str(szSession, 128, &pSess->key); 
    snprintf(szServer, 128, "%s", "unknown");

    if (NULL != pHandle->pConnect) {

        /* modified by liwh for bug 38572, 2016-05-18 */
        if (TASK_TASK == pHandle->pConnect->type)
        {
            ctc_task_delete_task(pHandle->pConnect);
        }
        else
        {
            ctc_task_delete_write(pHandle->pConnect);
        }
        /* liwh end */
    }
    pHandle->pConnect = NULL;
    if (NULL != pHandle->pTimeout) {
        ctc_task_delete_timer(pHandle->pTimeout);
        pHandle->pTimeout = NULL;
    }

    pSrv = auth_server_get_by_id(pHandle->nServerId);
    if (NULL == pSrv) {
        goto err_out;
    }
    cdb_auth_server_key_val2str(szServer, 128, &pSrv->key); 

    if (getsockopt(pSrv->fd, SOL_SOCKET, SO_ERROR, &nStatus, &nErrLen) != 0) {
        snprintf(szErrMsg, 128, "getsockopt failed: %s", strerror(errno));
        goto err_out;
    }

    if (0 == nStatus) {
        AUTHD_EVENT_DEBUG(
            "session %s connect to server %s success", 
            szSession, szServer);
           
        pSrv->connected = 1;
        if (NULL == pSrv->pRead) {
            pSrv->pRead = ctc_task_add_read(TASK_PRI_NORMAL, 
                auth_reply_read_cb, pSrv, pSrv->fd); 
        }
        pHandle->pTimeout = ctc_task_add_timer(TASK_PRI_NORMAL, 
            auth_read_timeout_cb, pSess, auth_get_timeout(pSrv));
        auth_do_request(pSess, pSrv);
        return;
    }

    /* connect failed */
    errno = nStatus;
    AUTHD_EVENT_DEBUG(
            "session %s connect to server %s failed: %s", 
            szSession, szServer, strerror(errno));
    pHandle->nRetries = auth_get_retries(pSrv);
    set_auth_server_status(pSrv, 1);
    pHandle->pConnect = ctc_task_add_task(TASK_PRI_NORMAL, 
        start_connect_cb, (void *)pSess);
    return;

err_out:
    AUTHD_EVENT_DEBUG( 
            "session %s connect to server %s failed", 
            szSession, szServer);    
    auth_failed(pSess, szErrMsg);
    return;
}

void
auth_connect_timeout_cb(void *pArg)
{
    tbl_auth_session_t *pSess = (tbl_auth_session_t *)pArg;
    auth_handle_t *pHandle = (auth_handle_t *)pSess->pHandle;
    char szSession[128];

    cdb_auth_session_key_val2str(szSession, 128, &pSess->key); 

    /* modified by liwh for bug 38885, 2016-06-06 */
    pHandle->nRetries++;
    /* liwh end */
        
    AUTHD_EVENT_DEBUG( 
            "session %s connect to server %u timeout, retries %u", 
            szSession, pHandle->nServerId, pHandle->nRetries); 
            
    pHandle->pTimeout = NULL;
    if (NULL != pHandle->pConnect) {

        /* modified by liwh for bug 38572, 2016-05-18 */
        if (TASK_TASK == pHandle->pConnect->type)
        {
            ctc_task_delete_task(pHandle->pConnect);
        }
        else
        {
            ctc_task_delete_write(pHandle->pConnect);
        }
        /* liwh end */
        
        pHandle->pConnect = NULL;
    }

    pHandle->pConnect = ctc_task_add_task(TASK_PRI_NORMAL, 
        start_connect_cb, (void *)pSess);
    return;
}

int auth_do_connect(tbl_auth_session_t *pSess)
{
    auth_handle_t *pHandle = (auth_handle_t *)pSess->pHandle;
    auth_be_op_t *beop = pHandle->pCBS;
    tbl_auth_server_t *pSrv = NULL;
    char szErrMsg[128];
    int nCurSrvId = pHandle->nServerId;
    int nStatus;
    int nRetries;
    char szSession[128];
    char szServer[128];

    cdb_auth_session_key_val2str(szSession, 128, &pSess->key);
    AUTHD_EVENT_DEBUG(
            "session %s try connect to auth server id %u, retries %u", 
            szSession, pHandle->nServerId, pHandle->nRetries);

    if (0 == nCurSrvId) {
        /* get the first server */
        pSrv = auth_server_get_first(pSess->key.type);
        if (NULL != pSrv) {
            pHandle->nRetries = 0;
        }
    } else {
        pSrv = auth_server_get_by_id(nCurSrvId);
        if (pSrv) {
            nRetries = auth_get_retries(pSrv);
        }
        if (NULL == pSrv || pHandle->nRetries >= nRetries) {

            if (pSrv)
            {
                set_auth_server_status(pSrv, 1);
            }
            pSrv = auth_server_get_next(nCurSrvId, pSess->key.type);
            if (NULL != pSrv) {
                pHandle->nRetries = 0;
            }
        } 
        /* modified by liwh for bug 38799, 2016-05-31 */
        /*else {
            pHandle->nRetries++;
        }*/
        /* liwh end */
    }

do_next:
    if (NULL == pSrv) {
        if (0 == nCurSrvId) {
            snprintf(szErrMsg, 128, "no authentication server error");
        } else {
            snprintf(szErrMsg, 128, "connect to authentication server failed");
        }
        goto err_out;
    }
    
    cdb_auth_server_key_val2str(szServer, 128, &pSrv->key);
    pHandle->nServerId = pSrv->id;

    if ((pSrv->pRead) && (AUTH_SERVER_TYPE_TACPLUS == pSrv->key.type))
    {
        AUTHD_EVENT_DEBUG( 
                "session %s connect to server %s failed when server is busy, try next server", 
                szSession, szServer);
        pSrv = auth_server_get_next(pHandle->nServerId, pSess->key.type);
        goto do_next;    
    }

    nStatus = beop->do_connect(pHandle, pSrv);
    if (M_AUTH_CONNECT_FAILED == nStatus) {
        /* determine the next server */
        AUTHD_EVENT_DEBUG( 
                "session %s connect to server %s failed, try next server", 
                szSession, szServer);
        set_auth_server_status(pSrv, 1);
        pSrv = auth_server_get_next(pHandle->nServerId, pSess->key.type);
        goto do_next;
    }
      
    if (M_AUTH_CONNECT_INPROGRESS == nStatus) {
        AUTHD_EVENT_DEBUG(
                "session %s connect to server %s in progress, waiting ...", 
                szSession, szServer);
        
        pHandle->pConnect = ctc_task_add_write(TASK_PRI_NORMAL, 
            auth_connect_check_cb, pSess, pSrv->fd);    
        pHandle->pTimeout = ctc_task_add_timer(TASK_PRI_NORMAL, 
            auth_connect_timeout_cb, pSess, auth_get_timeout(pSrv));
        if (NULL == pHandle->pTimeout || NULL == pHandle->pConnect) {
            snprintf(szErrMsg, 128, "out of memory");
            goto err_out;
        }
        return 0;
    }

    /* modified by liwh for bug 47004, 2018-05-23 */
    if (AUTH_SERVER_TYPE_RADIUS != pSrv->key.type)
    {
    /* liwh end */
         AUTHD_EVENT_DEBUG(
        "session %s connect to server %s success, try send request", 
        szSession, szServer);
    /* modified by liwh for bug 47004, 2018-05-23 */
    }
    else
    {
         AUTHD_EVENT_DEBUG(
        "session %s builds udp socket to server %s success, try send request", 
        szSession, szServer); 
    }
    /* liwh end */
     
    if (NULL != pSrv->pRead) {
        ctc_task_delete_read(pSrv->pRead);
    }
    pSrv->pRead = ctc_task_add_read(TASK_PRI_NORMAL, 
        auth_reply_read_cb, pSrv, pSrv->fd); 
    if (NULL != pHandle->pTimeout) {
        ctc_task_delete_timer(pHandle->pTimeout);
    }
    pHandle->pTimeout = ctc_task_add_timer(TASK_PRI_NORMAL, 
        auth_read_timeout_cb, pSess, auth_get_timeout(pSrv));
    return auth_do_request(pSess, pSrv);

err_out:
    auth_failed(pSess, szErrMsg);
    return -1;
}

void start_connect_cb(void *pArg)
{
    tbl_auth_session_t *pSess = (tbl_auth_session_t *)pArg;
    auth_handle_t *pHandle = (auth_handle_t *)pSess->pHandle;
    char szSession[128];
    
    cdb_auth_session_key_val2str(szSession, 128, &pSess->key);
    AUTHD_EVENT_DEBUG(
        "session %s start connect to server id %u, retries %u",
         szSession, pHandle->nServerId, pHandle->nRetries);
    
    pHandle->pConnect = NULL;
    auth_do_connect(pSess);
    return;
}

int
start_auth_process(tbl_auth_session_t *pSess)
{
    auth_handle_t *pHandle = NULL;
    auth_be_op_t *beop = NULL;
    char szErrMsg[128];
    
    if (NULL == pSess || '\0' == pSess->user.name[0] || 
        (('\0' == pSess->passwd[0]) && (!pSess->is_dot1x_session
            && !pSess->acct_start && !pSess->acct_stop && !pSess->acctcmd_start))) {
        return -1;
    }

    if (NULL != pSess->pHandle) {
        auth_handle_release((auth_handle_t *)pSess->pHandle);
        pSess->pHandle = NULL;
    }

    if ((pHandle = auth_handle_calloc()) == NULL) {
        snprintf(szErrMsg, 128, "out of memory");
        goto err_out;
    }

    pSess->pHandle = pHandle;
    pHandle->nServerId = 0;
    pHandle->nRetries = 0;
    
    if (AUTH_SERVER_TYPE_TACPLUS == pSess->key.type) {
        pHandle->pCBS = &tacplus_be_ops;
    } else {
        pHandle->pCBS = &radius_be_ops;
    }

    beop = pHandle->pCBS;
    if (beop->init(pHandle) != 0) {
        snprintf(szErrMsg, 128, "initialize authentication backend handle failed");
        goto err_out;
    }

    pHandle->pConnect = ctc_task_add_task(TASK_PRI_NORMAL, 
        start_connect_cb, (void *)pSess);
    return 0;

err_out:
    auth_failed(pSess, szErrMsg);    
    return -1;
}

int stop_auth_process(tbl_auth_session_t *pSess)
{
    auth_handle_t *pHandle = (auth_handle_t *)pSess->pHandle;
    char szSession[128];
    
    cdb_auth_session_key_val2str(szSession, 128, &pSess->key);
    AUTHD_EVENT_DEBUG(
        "session %s stop authentication", szSession);

    if (NULL == pHandle) {
        return -1;
    }

    auth_handle_release(pHandle);
    pSess->pHandle = NULL;
    return 0;
}

static int32
_auth_session_add(auth_session_key_t *pkey, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_auth_session_master_t *pmaster;
    tbl_auth_session_t auth_session;
    tbl_auth_session_t *p_auth_session = &auth_session;
    char szSession[128];
    int i;
    uint32 rc;

    if (tbl_auth_session_get_auth_session(pkey) != NULL) {
        return PM_E_NONE;
    }
    
    sal_memset(p_auth_session, 0x00, sizeof(tbl_auth_session_t));
    sal_memcpy(&p_auth_session->key, pkey, sizeof(auth_session_key_t));

    pmaster = tbl_auth_session_get_master();
    if (pmaster->session_cnt >= GLB_AUTH_SESSIONS_NUM) {
        CFG_CONFLICT_RET("Exceeding maximum authentication session numbers limit: %d", GLB_AUTH_SESSIONS_NUM);
    }

    for (i = 1; i < argc; i += 2) {
        if (_check_auth_session_field(argv[i], argv[i + 1], p_auth_session, para) == -1) {
            goto err_out;
        }
    }    

    pmaster->session_cnt++;
    rc = tbl_auth_session_add_auth_session(p_auth_session);    
    cdb_auth_session_key_val2str(szSession, 128, &p_auth_session->key);
    AUTHD_EVENT_DEBUG(
        "add session %s, count %u", szSession, pmaster->session_cnt);

    if ('\0' != p_auth_session->user.name[0] && ('\0' != p_auth_session->passwd[0]
            || p_auth_session->acct_start)) {
        p_auth_session = tbl_auth_session_get_auth_session(pkey);
        start_auth_process(p_auth_session);
    }
    return rc;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return PM_E_CFG_INVALID_PARAM;
}

static int32
_auth_session_del(auth_session_key_t *pkey, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_auth_session_t *p_auth_session = tbl_auth_session_get_auth_session(pkey);
    tbl_auth_session_master_t *pmaster = tbl_auth_session_get_master();
    char szSession[128];

    if (NULL == p_auth_session) {
        CFG_INVALID_PARAM_RET("No such authentication session");
    }

    pmaster->session_cnt--;
    cdb_auth_session_key_val2str(szSession, 128, &p_auth_session->key);
    AUTHD_EVENT_DEBUG(
        "delete session %s, count %u", szSession, pmaster->session_cnt);
    if (NULL != p_auth_session->pHandle) {
        stop_auth_process(p_auth_session);
    }
    return tbl_auth_session_del_auth_session(pkey);
}

int32
auth_session_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 field_id = TBL_AUTH_SESSION_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_auth_session_t item;
    tbl_auth_session_t *p_db_item = NULL;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_AUTH_SESSION);

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        sal_memset(&item, 0, sizeof(tbl_auth_session_t));
        rc = cdb_auth_session_key_str2val(&item.key, argv[0], para->p_rs->desc);
        if (rc < 0) {
            return PM_E_CFG_INVALID_PARAM;
        }

        p_db_item = tbl_auth_session_get_auth_session(&item.key);
    }
    switch (para->oper) {
    case CDB_OPER_ADD:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        if (NULL != p_db_item) {
            CFG_INVALID_PARAM_RET("Authentication session %s exist", argv[0]);
        }        
        rc = _auth_session_add(&item.key, argv, argc, para);
        break;

    case CDB_OPER_DEL:
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Authentication session %s not found", argv[0]);
        }
        rc = _auth_session_del(&p_db_item->key, argv, argc, para);
        break;

    case CDB_OPER_SET:
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Authentication session %s not found", argv[0]);
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_auth_session_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM; 
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            rc = _auth_session_cmd_set(field_id, &item, p_db_item, para);
        }
        break;

    case CDB_OPER_GET:
        if (argc > 0 && NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Authentication session %s not found", argv[0]);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _auth_session_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/* --- authd debug --- */
static int
_check_authdbg_field(char *pszkey, char *pszValue, tbl_authd_debug_t *pitem, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_AUTHD_DEBUG);
    cdb_field_t *p_field = NULL;
    int32 field_id;
    int32 nValue;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    if (strcmp(pszValue, "0") && strcmp(pszValue, "1")) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "invalid value for %s", pszkey); 
        goto err_out;
    }
    nValue = atoi(pszValue);
    switch ((tbl_authd_debug_field_id_t)field_id) {
        case TBL_AUTHD_DEBUG_FLD_AUTH_EVENT:
        case TBL_AUTHD_DEBUG_FLD_AUTH_PACKET:
        case TBL_AUTHD_DEBUG_FLD_AUTH_PROTOCOL:
        case TBL_AUTHD_DEBUG_FLD_AUTH_TIMER:
            GLB_SET_FLAG_COND(nValue, pitem->flags, (1 << field_id));
            break;
        case TBL_AUTHD_DEBUG_FLD_MAX:
            break;
    }

    return field_id;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
_authdbg_cmd_get(tbl_authd_debug_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_authd_debug_dump_one(pitem, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_authdbg_cmd_set(tbl_authd_debug_field_id_t field_id, tbl_authd_debug_t *pitem, 
        tbl_authd_debug_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (field_id >= TBL_AUTHD_DEBUG_FLD_MAX) {
        return PM_E_FAIL;
    }


    switch ((tbl_authd_debug_field_id_t)field_id) {
        case TBL_AUTHD_DEBUG_FLD_AUTH_EVENT:
        case TBL_AUTHD_DEBUG_FLD_AUTH_PACKET:
        case TBL_AUTHD_DEBUG_FLD_AUTH_PROTOCOL:
        case TBL_AUTHD_DEBUG_FLD_AUTH_TIMER:
            if ((pitem->flags & (1 << field_id)) 
                    == (p_db_item->flags & (1 << field_id))) {
                return PM_E_NONE;
            }

            break;
        case TBL_AUTHD_DEBUG_FLD_MAX:
            break;
    }


    PM_E_RETURN(tbl_authd_debug_set_authd_debug_field(pitem, field_id));
    return rc;
}

#define DBG_AUTH_TYPE_FIELD     0
#define DBG_AUTH_TYPE_ALL       1

int32
authdbg_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_authd_debug_field_id_t field_id = TBL_AUTHD_DEBUG_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_authd_debug_t item;
    tbl_authd_debug_t *p_db_item = NULL;
    int32 value = 0;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_AUTHD_DEBUG);    
    p_db_item = tbl_authd_debug_get_authd_debug();
    switch (para->oper) {
        case CDB_OPER_SET:
            if (0 == sal_strcmp(argv[i], "all"))
            {
                PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
                PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);

                GLB_SET_FLAG_COND(value, item.flags, AUTHD_DBG_FLAG_EVENT);
                GLB_SET_FLAG_COND(value, item.flags, AUTHD_DBG_FLAG_PACKET);
                GLB_SET_FLAG_COND(value, item.flags, AUTHD_DBG_FLAG_PROTO);
                GLB_SET_FLAG_COND(value, item.flags, AUTHD_DBG_FLAG_TIMER);

                rc = tbl_authd_debug_set_authd_debug_field(&item, TBL_AUTHD_DEBUG_FLD_AUTH_EVENT);
                rc = tbl_authd_debug_set_authd_debug_field(&item, TBL_AUTHD_DEBUG_FLD_AUTH_PACKET);
                rc = tbl_authd_debug_set_authd_debug_field(&item, TBL_AUTHD_DEBUG_FLD_AUTH_PROTOCOL);
                rc = tbl_authd_debug_set_authd_debug_field(&item, TBL_AUTHD_DEBUG_FLD_AUTH_TIMER);
            }
            else
            {
                if ((argc % 2) != 0) {
                    CFG_INVALID_PARAM_RET("Invalid parameters");
                }

                for (i = 0; i < argc; i += 2) {
                    if (_check_authdbg_field(argv[i], argv[i + 1], &item, para) == -1) {
                        return PM_E_CFG_INVALID_PARAM;
                    }

                    cdb_get_field_id(argv[i], p_node, &p_field);
                    if (CDB_ACCESS_RD == p_field->access) {
                        CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
                    }
                }

                for (i = 0; i < argc; i += 2) {
                    field_id = cdb_get_field_id(argv[i], p_node, &p_field);
                    rc |= _authdbg_cmd_set(field_id, &item, p_db_item, para);
                }
            }
            break;

        case CDB_OPER_GET:            
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            rc = _authdbg_cmd_get(p_db_item, para, p_node, &field);    
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}

/* --- auth cfg--- */
static int
_check_auth_cfg_field(char *pszkey, char *pszValue, tbl_auth_cfg_t *pitem, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_AUTH_CFG);
    cdb_field_t *p_field = NULL;
    int32 field_id;
    int32 nValue;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_auth_cfg_field_id_t)field_id) {
    case TBL_AUTH_CFG_FLD_SECRET:
    case TBL_AUTH_CFG_FLD_ENC_SECRET:
        if (cdb_check_str(pszValue, 
                (field_id == TBL_AUTH_CFG_FLD_SECRET) ? GLB_AUTH_SECRET_SIZE : (PASSWD_SIZE - 1),
                 para->p_rs->desc) < 0) {
            goto err_out;
        }
        if (TBL_AUTH_CFG_FLD_ENC_SECRET == field_id) {
            char szTmp[PASSWD_SIZE];
            if (do_decrypt(M_DEFAULT_PW_STR, pszValue, strlen(pszValue),
						   szTmp, PASSWD_SIZE) != 0) {
				snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid encrypt password");
				goto err_out;
			}
            snprintf(pitem->enc_secret, PASSWD_SIZE, "%s", pszValue);
            memset(pitem->secret, 0x00, GLB_AUTH_SECRET_SIZE + 1);
        } else {
            memset(pitem->enc_secret, 0x00, PASSWD_SIZE);
            snprintf(pitem->secret, GLB_AUTH_SECRET_SIZE + 1, "%s", pszValue);
        }
        break;
    case TBL_AUTH_CFG_FLD_TIMEOUT:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue) 
                || nValue < GLB_AUTH_TIMEOUT_MIN
                || nValue > GLB_AUTH_TIMEOUT_MAX) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid timeout seconds"); 
            goto err_out;
        }
        pitem->timeout = nValue;
        break;
    case TBL_AUTH_CFG_FLD_RETRIES:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue) 
                || nValue < GLB_AUTH_RETRIES_MIN
                || nValue > GLB_AUTH_RETRIES_MAX) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid retry times"); 
            goto err_out;
        }
        pitem->retries = nValue;
        break;
    case TBL_AUTH_CFG_FLD_DEADTIME:
        nValue = atoi(pszValue);
        if (strspn(pszValue, "0123456789") != strlen(pszValue) 
                || nValue < GLB_AUTH_DEADTIME_MIN
                || nValue > GLB_AUTH_DEADTIME_MAX) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid dead time"); 
            goto err_out;
        }
        pitem->deadtime = nValue;
        break;
    case TBL_AUTH_CFG_FLD_MAX:
        break;
    }

    return field_id;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
_auth_cfg_cmd_get(tbl_auth_cfg_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_auth_cfg_dump_one(pitem, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_auth_cfg_set_secret(tbl_auth_cfg_t *pitem, 
    tbl_auth_cfg_t *p_db_item, cfg_cmd_para_t *para)
{
    char szTmp[PASSWD_SIZE];
    int32 rc = PM_E_NONE;

    if ('\0' != pitem->secret[0] && app_is_password_encryption()) {
        gen_passwd_crypt(pitem->secret, szTmp, PASSWD_SIZE);
        snprintf(pitem->enc_secret, PASSWD_SIZE, "%s", szTmp);
        memset(pitem->secret, 0x00, GLB_AUTH_SECRET_SIZE + 1);
    }

    if (strcasecmp(pitem->secret, p_db_item->secret)) {
        rc = tbl_auth_cfg_set_auth_cfg_field(pitem, TBL_AUTH_CFG_FLD_SECRET);
    }

    if (strcasecmp(pitem->enc_secret, p_db_item->enc_secret)) {
        rc |= tbl_auth_cfg_set_auth_cfg_field(pitem, TBL_AUTH_CFG_FLD_ENC_SECRET);
    }
    return rc;
}


static int32
_auth_cfg_cmd_set(tbl_auth_cfg_field_id_t field_id, tbl_auth_cfg_t *pitem, 
        tbl_auth_cfg_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (field_id >= TBL_AUTH_CFG_FLD_MAX) {
        return PM_E_FAIL;
    }

    switch ((tbl_auth_cfg_field_id_t)field_id) {
    case TBL_AUTH_CFG_FLD_SECRET:
        if (!strcmp(pitem->secret, p_db_item->secret)) {
            return PM_E_NONE;
        }
        break;
    case TBL_AUTH_CFG_FLD_ENC_SECRET:
        if (!strcmp(pitem->enc_secret, p_db_item->enc_secret)) {
            return PM_E_NONE;
        }
        break;
    case TBL_AUTH_CFG_FLD_TIMEOUT:
        if (pitem->timeout == p_db_item->timeout) {
            return PM_E_NONE;
        }
        break;
    case TBL_AUTH_CFG_FLD_RETRIES:
        if (pitem->retries == p_db_item->retries) {
            return PM_E_NONE;
        }
        break;
    case TBL_AUTH_CFG_FLD_DEADTIME:
        if (pitem->deadtime == p_db_item->deadtime) {
            return PM_E_NONE;
        }
        break;
    case TBL_AUTH_CFG_FLD_MAX:
        return PM_E_NONE;
    }

    if (TBL_AUTH_CFG_FLD_SECRET == field_id 
            || TBL_AUTH_CFG_FLD_ENC_SECRET == field_id) {
        return _auth_cfg_set_secret(pitem, p_db_item, para);
    }     
    PM_E_RETURN(tbl_auth_cfg_set_auth_cfg_field(pitem, field_id));
    return rc;
}

int32
auth_cfg_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_auth_cfg_field_id_t field_id = TBL_AUTH_CFG_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_auth_cfg_t item;
    tbl_auth_cfg_t *p_db_item = NULL;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_AUTH_CFG);    
    p_db_item = tbl_auth_cfg_get_auth_cfg();
    switch (para->oper) {
    case CDB_OPER_SET:
        if (1 == argc) {
            if (!strcasecmp(argv[1], "secret") || !strcasecmp(argv[1], "enc_secret")) { 
                field_id = TBL_AUTH_CFG_FLD_SECRET;
                memset(item.secret, 0x00, GLB_AUTH_SECRET_SIZE+1);
                memset(item.enc_secret, 0x00, PASSWD_SIZE);
                return _auth_cfg_set_secret(&item, p_db_item, para);             
            }
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        if ((argc % 2) != 0) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        for (i = 0; i < argc; i += 2) {
            if (_check_auth_cfg_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM;
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access) {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 0; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            rc |= _auth_cfg_cmd_set(field_id, &item, p_db_item, para);
        }
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0) {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _auth_cfg_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32 authd_tbl_init()
{
    tbl_auth_cfg_t cfg;
    tbl_auth_server_master_t *pmaster;

    pmaster = tbl_auth_server_get_master();
    pmaster->cur_id = 0;
    pmaster->radius_cnt = 0;
    pmaster->tacplus_cnt = 0;

    /* -- auth server -- */
    memset(&cfg, 0x00, sizeof(tbl_auth_cfg_t));    
    cfg.retries = GLB_AUTH_DEFAULT_RETRIES;
    cfg.deadtime = GLB_AUTH_DEFAULT_DEADTIME;
    cfg.timeout = GLB_AUTH_DEFAULT_TIMEOUT;
    tbl_auth_cfg_set_auth_cfg_field(&cfg, TBL_AUTH_CFG_FLD_SECRET);
    tbl_auth_cfg_set_auth_cfg_field(&cfg, TBL_AUTH_CFG_FLD_RETRIES);
    tbl_auth_cfg_set_auth_cfg_field(&cfg, TBL_AUTH_CFG_FLD_DEADTIME);
    tbl_auth_cfg_set_auth_cfg_field(&cfg, TBL_AUTH_CFG_FLD_TIMEOUT);

    g_p_dead_timer = ctc_task_add_timer(TASK_PRI_NORMAL,
            auth_server_dead_timer, NULL, 1);
    return PM_E_NONE;
}

int
authd_start()
{
    tbl_vty_t vty;
    int i; 
    tbl_author_t author;
    tbl_account_t account;
    
    /* -- vty -- */
    memset(&vty, 0x00, sizeof(tbl_vty_t));
    vty.timeout_min = 10;
    vty.timeout_sec = 0;
#ifdef _GLB_UML_SYSTEM_
    vty.login = LOGIN_METHOD_NOLOGIN;
#else
    vty.login = LOGIN_METHOD_LOCAL;
#endif
    vty.privilege = 1;
    for (i = 0; i < GLB_MAX_VTY_NUM; i++) {
        vty.key.id = i;
        tbl_vty_add_vty(&vty);
    }
    
    /* -- author -- */
    memset(&author, 0x00, sizeof(tbl_author_t));
    snprintf(author.key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", "default");
    snprintf(author.methods, LINE_AUTH_METHOD_LINE_SIZE, "%s", "local");
    tbl_author_add_author(&author);

    /* -- account -- */
    memset(&account, 0x00, sizeof(tbl_account_t));
    snprintf(account.key.name, LINE_AUTH_METHOD_NAME_SIZE, "%s", "default");
    snprintf(account.methods, LINE_AUTH_METHOD_LINE_SIZE, "%s", "none");
    tbl_account_add_account(&account);

    log_sys(M_MOD_AUTHD, E_INFORMATIONAL, "Authd ready to service.");
    return 0;
}

static int32
tbl_auth_server_password_encryption(void *data, tbl_iter_args_t *pargs)
{
    tbl_auth_server_t *pCfg = (tbl_auth_server_t *)data;
    tbl_auth_server_t item;

    if (NULL != pCfg && '\0' != pCfg->secret[0]) {
        char szTmp[PASSWD_SIZE];
        gen_passwd_crypt(pCfg->secret, szTmp, PASSWD_SIZE);
        memset(&item, 0x00, sizeof(tbl_auth_server_t));
        memcpy(&item.key, &pCfg->key, sizeof(tbl_auth_server_key_t));
        snprintf(item.enc_secret, PASSWD_SIZE, "%s", szTmp);
        tbl_auth_server_set_auth_server_field(&item, TBL_AUTH_SERVER_FLD_SECRET);
        tbl_auth_server_set_auth_server_field(&item, TBL_AUTH_SERVER_FLD_ENC_SECRET);
    }

    return PM_E_NONE;
}


int authd_password_encryption(int onoff)
{
    char szTmp[PASSWD_SIZE];
    tbl_auth_cfg_t *pCfg;
    tbl_auth_cfg_t item;
    
    if (!onoff) {
        return 0;
    }

    /* auth cfg */
    pCfg = tbl_auth_cfg_get_auth_cfg();
    if ('\0' != pCfg->secret[0]) {
        gen_passwd_crypt(pCfg->secret, szTmp, PASSWD_SIZE);
        memset(&item, 0x00, sizeof(tbl_auth_cfg_t));
        snprintf(item.enc_secret, PASSWD_SIZE, "%s", szTmp);
        tbl_auth_cfg_set_auth_cfg_field(&item, TBL_AUTH_CFG_FLD_SECRET);
        tbl_auth_cfg_set_auth_cfg_field(&item, TBL_AUTH_CFG_FLD_ENC_SECRET);
    }

    dot1x_radius_key_encryption();

    /* auth server */
    tbl_auth_server_iterate((TBL_ITER_CB_FUNC)tbl_auth_server_password_encryption, NULL);
    return 0;
}

void
authd_send_account_stop(tbl_login_t  *p_db_login, tbl_vty_t  *p_vty)
{
    tbl_account_t                account;    
    tbl_account_t                *p_account = NULL;
    auth_session_key_t           session_key;
    tbl_auth_session_master_t    *pmaster;
    char                         szSession[128];
    tbl_auth_session_t           auth_session;
    tbl_auth_session_t           *p_auth_session = NULL;
    int32                        rc = PM_E_NONE;

    sal_memset(&account, 0x0, sizeof(tbl_account_t));
    sal_memset(&session_key, 0x0, sizeof(auth_session_key_t));
    sal_memset(&auth_session, 0x0, sizeof(tbl_auth_session_t));
    sal_memset(szSession, 0x0, 128);
    
    sal_memcpy(&account.key.name, p_vty->account_method, LINE_AUTH_METHOD_NAME_SIZE);        
    p_account = tbl_account_get_account(&account.key);    

    session_key.index = p_vty->key.id + 1;
    if (p_account && (0 == sal_memcmp(p_account->methods, "radius", 6)))   
    {
        session_key.type = AUTH_SERVER_TYPE_RADIUS;
        sprintf(session_key.name, "klish#%d#%s", session_key.index, "radius");
    }
    else if (p_account && (0 == sal_memcmp(p_account->methods, "tacplus", 7)))   
    {
        session_key.type = AUTH_SERVER_TYPE_TACPLUS;
        sprintf(session_key.name, "klish#%d#%s", session_key.index, "tacplus");
    }
    else
    {
        return;    
    }

    pmaster = tbl_auth_session_get_master();
    if (pmaster->session_cnt >= GLB_AUTH_SESSIONS_NUM) 
    {
        AUTHD_EVENT_DEBUG("Exceeding maximum authentication session numbers limit: %d", 
            GLB_AUTH_SESSIONS_NUM);
        return;
    }
    
    p_auth_session = tbl_auth_session_get_auth_session(&session_key);
    if (p_auth_session) 
    {
        if (p_auth_session->acct_stop || p_auth_session->acctcmd_start)
        {
            authd_delete_account_stop_session(p_auth_session);
        }
        else
        {
            AUTHD_EVENT_DEBUG("Session %s is running for authentication or authorization, can't run accounintg", 
                session_key.name);
            return;
        }
    }
    
    sal_memcpy(&auth_session.key, &session_key, sizeof(auth_session_key_t));
    rc = tbl_auth_session_add_auth_session(&auth_session); 
    if (rc)
    {
        cdb_auth_session_key_val2str(szSession, 128, &session_key);
        AUTHD_ERR_PRINT("add session %s, count %u failure", szSession, pmaster->session_cnt);
        return;
    }

    p_auth_session = tbl_auth_session_get_auth_session(&session_key);
    if (!p_auth_session)
    {
        cdb_auth_session_key_val2str(szSession, 128, &session_key);
        AUTHD_ERR_PRINT("add session %s, count %u failure", szSession, pmaster->session_cnt);
        return;
    }
     
    pmaster->session_cnt++;
    
    AUTHD_EVENT_DEBUG("authd_send_account_stop p_db_login->user.name is %s", p_db_login->user.name);

    /* modified by liwh for bug 50704, 2019-01-08 */
    if ('\0' != p_db_login->user.name[0]) 
    {
        sal_strcpy(p_auth_session->user.name, (char*)p_db_login->user.name);
    }
    else
    {
        sal_memcpy(p_auth_session->user.name, "null", 4);
    }
    /* liwh end */

    p_auth_session->acct_stop = TRUE;

    if ('\0' != p_auth_session->user.name[0]) 
    { 
        start_auth_process(p_auth_session);
    }
 
    return;
}

void
authd_send_next_account_stop(tbl_auth_session_t   *p_auth_old_session)
{
    tbl_account_t                account;    
    tbl_account_t                *p_account = NULL;
    auth_session_key_t           session_key;
    tbl_auth_session_master_t    *pmaster;
    char                         szSession[128];
    tbl_auth_session_t           auth_session;
    tbl_auth_session_t           *p_auth_session = NULL;
    int32                        rc = PM_E_NONE;
    tbl_vty_t                    *p_vty = NULL;
    tbl_vty_t                    vty;

    sal_memset(&account, 0x0, sizeof(tbl_account_t));
    sal_memset(&session_key, 0x0, sizeof(auth_session_key_t));
    sal_memset(&auth_session, 0x0, sizeof(tbl_auth_session_t));
    sal_memset(szSession, 0x0, 128);
    sal_memset(&vty, 0, sizeof(tbl_vty_t));

    if (0 == p_auth_old_session->key.index)
    {
        return;
    }

    vty.key.id = p_auth_old_session->key.index - 1;   
    p_vty = tbl_vty_get_vty(&vty.key);                        

    if (!p_vty)
    {
        return;
    }

    /* modified by liwh for bug 44995, 2018-01-17 
       if both tacplus and radius account modes are tried, stop account */
    if (p_auth_old_session->next_acct_stop)
    {
        return;    
    }
    /* liwh end */

    sal_memcpy(&account.key.name, p_vty->account_method, LINE_AUTH_METHOD_NAME_SIZE);        
    p_account = tbl_account_get_account(&account.key);    

    session_key.index = p_vty->key.id + 1;
    if (p_account && sal_strstr(p_account->methods, "radius")
        && (AUTH_SERVER_TYPE_TACPLUS == p_auth_old_session->key.type))   
    {
        session_key.type = AUTH_SERVER_TYPE_RADIUS;
        sprintf(session_key.name, "klish#%d#%s", session_key.index, "radius");
    }
    else if (p_account && sal_strstr(p_account->methods, "tacplus")
        && (AUTH_SERVER_TYPE_RADIUS == p_auth_old_session->key.type))   
    {
        session_key.type = AUTH_SERVER_TYPE_TACPLUS;
        sprintf(session_key.name, "klish#%d#%s", session_key.index, "tacplus");
    }
    else
    {
        return;    
    }

    pmaster = tbl_auth_session_get_master();
    if (pmaster->session_cnt >= GLB_AUTH_SESSIONS_NUM) 
    {
        AUTHD_EVENT_DEBUG("Exceeding maximum authentication session numbers limit: %d", 
            GLB_AUTH_SESSIONS_NUM);
        return;
    }
    
    p_auth_session = tbl_auth_session_get_auth_session(&session_key);
    if (p_auth_session) 
    {
        if (p_auth_session->acct_stop || p_auth_session->acctcmd_start)
        {
            authd_delete_account_stop_session(p_auth_session);
        }
        else
        {
            AUTHD_EVENT_DEBUG("Session %s is running for authentication or authorization, can't run accounintg", 
                session_key.name);
            return;
        }
    }
    
    sal_memcpy(&auth_session.key, &session_key, sizeof(auth_session_key_t));
    rc = tbl_auth_session_add_auth_session(&auth_session); 
    if (rc)
    {
        cdb_auth_session_key_val2str(szSession, 128, &session_key);
        AUTHD_ERR_PRINT("add session %s, count %u failure", szSession, pmaster->session_cnt);
        return;
    }

    p_auth_session = tbl_auth_session_get_auth_session(&session_key);
    if (!p_auth_session)
    {
        cdb_auth_session_key_val2str(szSession, 128, &session_key);
        AUTHD_ERR_PRINT("add session %s, count %u failure", szSession, pmaster->session_cnt);
        return;
    }
     
    pmaster->session_cnt++;
    
    AUTHD_EVENT_DEBUG("authd_send_next_account_stop p_auth_old_session->user.name is %s", p_auth_old_session->user.name);
    
    sal_strcpy(p_auth_session->user.name, p_auth_old_session->user.name);
    p_auth_session->acct_stop = TRUE;

    /* modified by liwh for bug 44995, 2018-01-17 */
    p_auth_session->next_acct_stop = TRUE;
    /* liwh end */

    if ('\0' != p_auth_session->user.name[0]) 
    { 
        start_auth_process(p_auth_session);
    }
 
    return;
}


void
authd_delete_account_stop_session(tbl_auth_session_t  *p_auth_session)
{
    tbl_auth_session_master_t    *pmaster;
    auth_handle_t *pHandle;
    
    pHandle = (auth_handle_t *)p_auth_session->pHandle;
    if (NULL != pHandle) {
        auth_handle_release(pHandle);
        p_auth_session->pHandle = NULL;
    }
    
    pmaster = tbl_auth_session_get_master();
    if (!pmaster)
    {
        return;
    }
    pmaster->session_cnt--;

    AUTHD_EVENT_DEBUG("authd_delete_account_stop_session p_auth_session->key name %s index %d type %d", p_auth_session->key.name,
        p_auth_session->key.index, p_auth_session->key.type);
    
    tbl_auth_session_del_auth_session(&p_auth_session->key);

    return;
}

uint8 
aaa_map_privilege(uint32 privilege)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    
    if (p_db_glb->aaa_privilege3 < privilege)
    {
        return VTY_PRIVILEGE_VR_MAX;
    }
    if (p_db_glb->aaa_privilege2 < privilege)
    {
        return VTY_PRIVILEGE_LEVEL_THREE;
    }
    if (p_db_glb->aaa_privilege1 < privilege)
    {
        return VTY_PRIVILEGE_LEVEL_TWO;
    }
    return VTY_PRIVILEGE_MIN;
}

uint8 
aaa_remap_privilege(uint8 privilege)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    
    if (VTY_PRIVILEGE_VR_MAX == privilege)
    {
        return CISCO_PRIVILEGE_MAX;
    }
    if (VTY_PRIVILEGE_LEVEL_THREE == privilege)
    {
        return p_db_glb->aaa_privilege3;
    }
    if (VTY_PRIVILEGE_LEVEL_TWO == privilege)
    {
        return p_db_glb->aaa_privilege2;
    }
    return p_db_glb->aaa_privilege1;
}

void
authd_send_account_commands(tbl_login_t  *p_db_login, tbl_vty_t  *p_vty)
{
    tbl_accountcmd_t             accountcmd;    
    tbl_accountcmd_t             *p_accountcmd = NULL;
    auth_session_key_t           session_key;
    tbl_auth_session_master_t    *pmaster;
    char                         szSession[128];
    tbl_auth_session_t           auth_session;
    tbl_auth_session_t           *p_auth_session = NULL;
    int32                        rc = PM_E_NONE;

    sal_memset(&accountcmd, 0x0, sizeof(tbl_accountcmd_t));
    sal_memset(&session_key, 0x0, sizeof(auth_session_key_t));
    sal_memset(&auth_session, 0x0, sizeof(tbl_auth_session_t));
    sal_memset(szSession, 0x0, 128);

    sal_memcpy(&accountcmd.key.name, p_vty->accountcmd_method, LINE_AUTH_METHOD_NAME_SIZE);        
    p_accountcmd = tbl_accountcmd_get_accountcmd(&accountcmd.key);    

    session_key.index = p_vty->key.id + 1;
    if (p_accountcmd && (0 == sal_strncasecmp(p_accountcmd->methods, "tacplus", 7)))   
    {
        session_key.type = AUTH_SERVER_TYPE_TACPLUS;
        sprintf(session_key.name, "klish#%d#%s", session_key.index, "tacplus");
    }
    else
    {
        return;    
    }

    pmaster = tbl_auth_session_get_master();
    if (pmaster->session_cnt >= GLB_AUTH_SESSIONS_NUM) 
    {
        AUTHD_EVENT_DEBUG("Exceeding maximum authentication session numbers limit: %d", 
            GLB_AUTH_SESSIONS_NUM);
        return;
    }
    
    p_auth_session = tbl_auth_session_get_auth_session(&session_key);
    if (!p_auth_session) 
    {
        sal_memcpy(&auth_session.key, &session_key, sizeof(auth_session_key_t));
        rc = tbl_auth_session_add_auth_session(&auth_session); 
        if (rc)
        {
            cdb_auth_session_key_val2str(szSession, 128, &session_key);
            AUTHD_ERR_PRINT("add session %s, count %u failure", szSession, pmaster->session_cnt);
            return;
        }
    
        p_auth_session = tbl_auth_session_get_auth_session(&session_key);
        if (!p_auth_session)
        {
            cdb_auth_session_key_val2str(szSession, 128, &session_key);
            AUTHD_ERR_PRINT("add session %s, count %u failure", szSession, pmaster->session_cnt);
            return;
        }
         
        pmaster->session_cnt++;

        if ('\0' != p_db_login->user.name[0]) 
        {
            sal_strcpy(p_auth_session->user.name, (char*)p_db_login->user.name);
        }
        else
        {
            sal_memcpy(p_auth_session->user.name, "null", 4);
        }
        
        p_auth_session->acctcmd_start = TRUE;

        if ('\0' != p_auth_session->user.name[0]) 
        { 
            start_auth_process(p_auth_session);
        }
    }
 
    return;
}

