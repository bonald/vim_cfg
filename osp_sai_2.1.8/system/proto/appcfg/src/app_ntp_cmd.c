
#include "proto.h"
#include "appcfg.h"
#include "sal.h"
#include "lib_fs.h"

#include "switch.h"
#include "gen/tbl_ntp_server_define.h"
#include "gen/tbl_ntp_key_define.h"
#include "gen/tbl_ntp_ace_define.h"
#include "gen/tbl_ntp_cfg_define.h"
#include "gen/tbl_ntp_server.h"
#include "gen/tbl_ntp_key.h"
#include "gen/tbl_ntp_ace.h"
#include "gen/tbl_ntp_cfg.h"

#include <errno.h>
#include "app_ntp.h"


/* NTP cli parameter check */
#define M_NTP_PARAMCHECK(cnt, msg) \
do  \
{ \
    if ((i + (cnt)) == argc) \
    { \
        CFG_INVALID_PARAM_RET(msg); \
        return -1; \
    } \
} \
while (0)

int32
app_get_ip_str2val(char *str, addr_t *addr)
{
    if (1 == sal_inet_pton(AF_INET, str, &addr->u.prefix4))
    {  
        addr->family = AF_INET;
        return PM_E_NONE;
    }
    else if (1 == sal_inet_pton(AF_INET6, str, &addr->u.prefix6))
    {
        addr->family = AF_INET6;
        return PM_E_NONE;
    }
    
    return PM_E_FAIL;
}

int32
app_ntp_server_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    int32 i = 0;
    int32 keyid = 0;
    int32 version = 0;
    int32 prefer = 0;
    addr_t             ip_addr;
    
    CFG_DBG_FUNC();

    sal_memset(&ip_addr, 0, sizeof(addr_t));

    p_node = cdb_get_tbl(TBL_NTP_SERVER);
    if(argc ==0)
    {
        switch (para->oper)
        {  
        case CDB_OPER_GET:
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
    
            rc = ntp_get_ucast_server(para, p_node, &field);
            lib_cmd_process_cli_return(para, rc);
            break;
    
        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
        }
        return PM_E_NONE;
    }

    if(!sal_strcasecmp(argv[0], "peer" ))
    {
        switch (para->oper)
        {
        case CDB_OPER_ADD:
    
              /*skip HOSTNAME*/
              i = 2;
              while (i < argc)
              {
                  /* process key number */
                  if (!sal_strcasecmp(argv[i], "key"))
                  {
                      M_NTP_PARAMCHECK(1, "Please specify key number\n");
                      keyid = sal_strtou32(argv[i + 1], NULL, 10);
                      i += 2;
                      continue;
                  }
          
                  /* process protocol version */
                  if (!sal_strcasecmp(argv[i], "version"))
                  {
                      M_NTP_PARAMCHECK(1, "Please specify protocol version number\n");
                      version = sal_strtou32(argv[i + 1], NULL, 10);
                      i += 2;
                      continue;
                  }
          
                  /* process "prefer" parameter*/
                  if (!sal_strcasecmp(argv[i], "prefer"))
                  {
                      prefer = 1;
                      i += 1;
                      continue;
                  }
        
                break;
            }

            rc = ntp_add_ucast_peer(para, argv[1], keyid, version, prefer);
            lib_cmd_process_cli_return(para, rc);
            break;
    
        case CDB_OPER_DEL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            rc = ntp_del_ucast_peer(para, argv[1], argv[2]);
            lib_cmd_process_cli_return(para, rc);
            break;
    
        case CDB_OPER_SET:       
            break;
            
        case CDB_OPER_GET:
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
    
            rc = ntp_get_ucast_server(para, p_node, &field);
            lib_cmd_process_cli_return(para, rc);
            break;
    
        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
        }
    }
#if 1 /* address optimization by wangqj */
    else if (0 == sal_strcmp(argv[0], "set_band"))
    {
        if (3 != argc)
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }
        ntp_server_set_namespace(para, argv[1], argv[2]);
    }
#endif
    else 
    {
        switch (para->oper)
        {
        case CDB_OPER_ADD:
    
            /*skip HOSTNAME*/
            i = 1;
            while (i < argc)
            {
                /* process key number */
                if (!sal_strcasecmp(argv[i], "key"))
                {
                    M_NTP_PARAMCHECK(1, "Please specify key number\n");
                    keyid = sal_strtou32(argv[i + 1], NULL, 10);
                    i += 2;
                    continue;
                }
      
                /* process protocol version */
                if (!sal_strcasecmp(argv[i], "version"))
                {
                      M_NTP_PARAMCHECK(1, "Please specify protocol version number\n"); 
                      version = sal_strtou32(argv[i + 1], NULL, 10);
                      i += 2;
                      continue;
                }
      
                /* process "prefer" parameter*/
                if (!sal_strcasecmp(argv[i], "prefer"))
                {
                    prefer = 1;
                    i += 1;
                    continue;
                }
    
                break;
            }

            rc = app_get_ip_str2val(argv[0], &ip_addr);
            if (PM_E_NONE != rc)
            {
                 CFG_INVALID_PARAM_RET("Please check the ip address of ntp server!");    
            }

            if (AF_INET == ip_addr.family)
            {
                ip_addr.u.prefix4.s_addr = sal_ntohl(ip_addr.u.prefix4.s_addr);
                if(IPV4_NET127(ip_addr.u.prefix4.s_addr)
                    || IPV4_CHECK2(ip_addr.u.prefix4.s_addr)
                    || IPV4_CHECK3(ip_addr.u.prefix4.s_addr)
                    || IPV4_NET0(ip_addr.u.prefix4.s_addr))
                {
                    CFG_INVALID_PARAM_RET("Please check the ip address of ntp server!");
                }
            }
            rc = ntp_add_ucast_server(para, argv[0], keyid, version, prefer);
            lib_cmd_process_cli_return(para, rc);
            break;
    
        case CDB_OPER_DEL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            rc = ntp_del_ucast_server(para, argv[0], argv[1]);
            lib_cmd_process_cli_return(para, rc);
            break;
    
        case CDB_OPER_SET:       
            break;
            
        case CDB_OPER_GET:
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
    
            rc = ntp_get_ucast_server(para, p_node, &field);
            lib_cmd_process_cli_return(para, rc);
            break;
    
        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
        }
    }
        
    return rc;
}


int32
app_ntp_trustkey_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    uint32 keyid = 0;
    int32 rc = PM_E_NONE;

    
    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_NTP_KEY);
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if(!sal_strcasecmp(argv[0], "trusted_key"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            keyid = sal_strtou32(argv[1], NULL, 10);
            rc = ntp_add_trustedkey(para, keyid);
        }
        else if(!sal_strcasecmp(argv[0], "pair"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            keyid = sal_strtou32(argv[1], NULL, 10);
            /*added by yejl to fix bug43885, 2017-06-12*/
            #if 0
            rc = cdb_check_str(argv[2], NTP_MAX_KEY_LEN, para->p_rs->desc);
            if (rc < 0)
            {
                para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
                return rc;
            }
            #endif
            //rc = ntp_del_keypair(para, keyid, FALSE);
            rc = ntp_add_keypair(para, keyid, argv[2]);
        }
/*Modify by zhush for bug 52043, 2019-07-18*/
	if (PM_E_NOT_EXIST == rc)
	 {
            CFG_INVALID_PARAM_RET("No such key");
	 }
	 else
	 {
            lib_cmd_process_cli_return(para, rc);
	 }        
        break;
/*end zhush*/

    case CDB_OPER_DEL:
        if(!sal_strcasecmp(argv[0], "trusted_key"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            keyid = sal_strtou32(argv[1], NULL, 10);
            rc = ntp_del_trustedkey(para, keyid);
        }
        else if(!sal_strcasecmp(argv[0], "pair"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            keyid = sal_strtou32(argv[1], NULL, 10);
            rc = ntp_del_keypair(para, keyid, TRUE);
        }
/*Modify by zhush for bug 52043, 2019-07-18*/
        if (PM_E_NOT_EXIST == rc)
	 {
            CFG_INVALID_PARAM_RET("No such key");
	 }
	 else
	 {
            lib_cmd_process_cli_return(para, rc);
	 }        
        break;
/*end zhush*/

    case CDB_OPER_SET:       
        break;
        
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = ntp_get_trustedkey(para, p_node, &field);
/*Modify by zhush for bug 52043, 2019-07-18*/
        if (PM_E_NOT_EXIST == rc)
	 {
            CFG_INVALID_PARAM_RET("No such key");
	 }
	 else
	 {
            lib_cmd_process_cli_return(para, rc);
	 }        
        break;
/*end zhush*/

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
app_ntp_normal_cfg_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 stratum = 0;
    int32 broadcast_delay = 0;
    int32 minimum_distance = 0;
    int32 rc = PM_E_NONE;
    tbl_ntp_cfg_t *p_db_cfg = NULL;
    
    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_NTP_CFG);

    /* modified by liwh for bug 49775, 2018-06-05 */
    p_db_cfg = tbl_ntp_cfg_get_ntp_cfg();
    if (NULL == p_db_cfg)
    {
        lib_cmd_process_cli_return(para, PM_E_NOT_INITED);
    }
    /* liwh end */
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:

        break;
    case CDB_OPER_DEL:


        break;

    case CDB_OPER_SET: 
        if(!sal_strcasecmp(argv[0], "if_reload"))
        {
            rc = ntp_if_reload(para);
        }
        else if(!sal_strcasecmp(argv[0], "stratum"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            stratum = sal_strtou32(argv[1], NULL, 10);
            rc = ntp_enable_refclock(para, stratum);
        }
        else if(!sal_strcasecmp(argv[0], "broadcast_delay"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            broadcast_delay = sal_strtou32(argv[1], NULL, 10); 
            rc = ntp_set_broadcast_delay(para, broadcast_delay);
        }
        else if(!sal_strcasecmp(argv[0], "minimum_distance"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            minimum_distance = sal_strtou32(argv[1], NULL, 10); 
            rc = ntp_set_minimum_distance(para, minimum_distance);
        }
        else if(!sal_strcasecmp(argv[0], "auth_enable"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            rc = ntp_set_auth_enable(para, argv[1]);
        }
        else if(!sal_strcasecmp(argv[0], "clear_stats"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
            rc = ntp_clear_stats(para);
        }
        else if(!sal_strcasecmp(argv[0], "debug_level"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            rc = ntp_set_debug_level(para, argv[1]);
        }
        else if (!sal_strcasecmp(argv[0], "ucastServerCount_inband"))
        {
            if (p_db_cfg->ucastServerCount_outband)
            {
                CFG_INVALID_PARAM_RET("Can't configure ntp server and mgmt-if ntp server on same device");    
            }
            
            if (!sal_strcmp(argv[1], "1"))
            {
                p_db_cfg->ucastServerCount_inband++;
            }
            else
            {
                p_db_cfg->ucastServerCount_inband--;    
            }
            
        }
        else if (!sal_strcasecmp(argv[0], "ucastServerCount_outband"))
        {
            if (p_db_cfg->ucastServerCount_inband)
            {
                CFG_INVALID_PARAM_RET("Can't configure ntp server and mgmt-if ntp server on same device");    
            }

            if (!sal_strcmp(argv[1], "1"))
            {
                p_db_cfg->ucastServerCount_outband++;
            }
            else
            {
                p_db_cfg->ucastServerCount_outband--;    
            }
        }
        lib_cmd_process_cli_return(para, rc);
        break;
        
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = ntp_get_global_cfg(para, p_node, &field);
        lib_cmd_process_cli_return(para, rc);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}



int32
app_ntp_interface_cfg_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 disable = 0;
    int32 bclient = 0;
    int32 rc = PM_E_NONE;
    
    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_NTP_IF);
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:

        break;
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET: 
        if(!sal_strcasecmp(argv[0], "disable"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            disable = sal_strtou32(argv[3], NULL, 10);
            rc = ntp_interface_disable(para, argv[1], disable);
        }
        else if(!sal_strcasecmp(argv[0], "broadcast_client"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            bclient = sal_strtou32(argv[3], NULL, 10);
            rc = ntp_interface_broadcast_client(para, argv[1], bclient);
        }
 
        lib_cmd_process_cli_return(para, rc);
        break;
        
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        //rc = ntp_get_trustedkey(para, p_node, &field);
        lib_cmd_process_cli_return(para, rc);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}



int32
app_ntp_ace_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    
    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_NTP_ACE);
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        rc = ntp_add_ace(para, argc, argv);
        lib_cmd_process_cli_return(para, rc);
        
        break;
    case CDB_OPER_DEL:
        if(argc > 3)
        {
            rc = ntp_del_ace_option(para, argc, argv);
        }
        else
        {
            rc = ntp_del_ace(para, argc, argv);
        }
        
        lib_cmd_process_cli_return(para, rc);
        break;

    case CDB_OPER_SET:       
        break;
        
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = ntp_get_ace(para, p_node, &field);
        lib_cmd_process_cli_return(para, rc);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
app_cmd_process_ntp_syncstatus(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    
    case CDB_OPER_SET:
        ntp_show_exec(para,M_NTP_LOCALHOST_IP, "ntpdc -nc syncstatus");
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

