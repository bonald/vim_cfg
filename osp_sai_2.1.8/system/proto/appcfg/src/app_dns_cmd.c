
#include <openssl/bn.h>
#include <assert.h>

#include "proto.h"
#include "gen/tbl_static_dns_define.h"
#include "gen/tbl_static_dns.h"
#include "gen/tbl_dynamic_dns_domain_define.h"
#include "gen/tbl_dynamic_dns_domain.h"
#include "gen/tbl_dynamic_dns_server_define.h"
#include "gen/tbl_dynamic_dns_server.h"
#include "appcfg.h"
#include "app_dns.h"
#include "lib_fs.h"


static int32
_app_static_dns_cmd_get_static_dns(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_static_dns_iterate((TBL_ITER_CB_FUNC)tbl_static_dns_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_app_dynamic_dns_domain_cmd_get_dynamic_dns_domain(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_dynamic_dns_domain_iterate((TBL_ITER_CB_FUNC)tbl_dynamic_dns_domain_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_app_dynamic_dns_server_cmd_get_dynamic_dns_server(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_dynamic_dns_server_iterate((TBL_ITER_CB_FUNC)tbl_dynamic_dns_server_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
app_static_dns_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    tbl_static_dns_t static_dns;
    tbl_static_dns_t *p_db_static_dns = NULL;
    tbl_static_dns_master_t *p_db_static_master = NULL;
    int32 rc = PM_E_NONE;
    addr_t ip_addr;
    char addr_buf[CMD_BUF_32];
    
    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_STATIC_DNS);
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        sal_memset(&static_dns, 0, sizeof(static_dns));
        rc = cdb_check_name(argv[0], DNS_HOSTNAME_SIZE, para->p_rs->desc);
        if (rc < 0)
        {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
        sal_strncpy(static_dns.key.name, argv[0], DNS_HOSTNAME_SIZE);
        
        p_db_static_master = tbl_static_dns_get_master();

        /* check exist */
        p_db_static_dns = tbl_static_dns_get_static_dns(&static_dns.key);

        if(p_db_static_master->static_list->count == MAX_STATIC_HOST_ENTRY)
        {
            if(p_db_static_dns == NULL)
            {
              CFG_INVALID_PARAM_RET("Too many ip host entries");/*modified by yejl for bug 54025, 2019-10-17*/
            }
        }
        /*modified by guoxd for bug 52442, 2019-09-19*/
        /*ip address 0.0.0.0-0.255.255.255, 127.0.0.0-127.255.255.255, 224.0.0.0-255.255.255.255 not properly for host ip*/
        CFG_PARSER_IP_ADDR(argv[1], &ip_addr.u.prefix4, para->p_rs); 
        ip_addr .family = AF_INET;
        if(IPV4_CHECK3(ip_addr.u.prefix4.s_addr)
            || IPV4_CHECK5(ip_addr.u.prefix4.s_addr)
            || IPV4_CHECK6(ip_addr.u.prefix4.s_addr))
        {
            sal_memset(addr_buf,0,sizeof(addr_buf));
            CFG_CONFLICT_RET("Address %s is not vaild", cdb_addr_val2str(addr_buf, CMD_BUF_32, &ip_addr));
        }
        rc = _app_static_dns_add_cfg(argv[0], argv[1]);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        sal_memset(&static_dns, 0, sizeof(static_dns));
        rc = cdb_check_name(argv[0], DNS_HOSTNAME_SIZE, para->p_rs->desc);
        if (rc < 0)
        {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
        sal_strncpy(static_dns.key.name, argv[0], DNS_HOSTNAME_SIZE);
        

        /* check exist */
        p_db_static_dns = tbl_static_dns_get_static_dns(&static_dns.key);
     
        if(p_db_static_dns == NULL)
        {
          CFG_INVALID_PARAM_RET(" Error: host name doesn't exist");
        }

        rc = _app_static_dns_del_cfg(argv[0]);
        break;

    case CDB_OPER_SET:       
        break;
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _app_static_dns_cmd_get_static_dns(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
app_dynamic_dns_domain_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    tbl_dynamic_dns_domain_t dynamic_domain;
    tbl_dynamic_dns_domain_t *p_db_dynamic_domain = NULL;
    tbl_dynamic_dns_domain_master_t *p_db_dynamic_domain_master = NULL;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_DYNAMIC_DNS_DOMAIN);
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        sal_memset(&dynamic_domain, 0, sizeof(dynamic_domain));
        rc = cdb_check_name(argv[0], DNS_HOSTNAME_SIZE, para->p_rs->desc);
        if (rc < 0)
        {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
        sal_strncpy(dynamic_domain.key.name, argv[0], DNS_HOSTNAME_SIZE);
        
        p_db_dynamic_domain_master = tbl_dynamic_dns_domain_get_master();

        /* check exist */
        p_db_dynamic_domain = tbl_dynamic_dns_domain_get_dynamic_dns_domain(&dynamic_domain.key);

        if(p_db_dynamic_domain)
        {
            CFG_INVALID_PARAM_RET("Error: duplicate domain");
        }

        if(p_db_dynamic_domain_master->dynamic_domain_list->count == MAX_DYNAMIC_DNS_DOMAIN_NUM)
        {
            if(p_db_dynamic_domain == NULL)
            {
                CFG_INVALID_PARAM_RET("Error: too many domains");
            }
        }

        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);

        rc = _app_dynamic_dns_domain_add_cfg(argv[0]);
        break;
   case CDB_OPER_DEL:
        sal_memset(&dynamic_domain, 0, sizeof(dynamic_domain));
        rc = cdb_check_name(argv[0], DNS_HOSTNAME_SIZE, para->p_rs->desc);
        if (rc < 0)
        {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
        sal_strncpy(dynamic_domain.key.name, argv[0], DNS_HOSTNAME_SIZE);
        

        /* check exist */
        p_db_dynamic_domain = tbl_dynamic_dns_domain_get_dynamic_dns_domain(&dynamic_domain.key);

        if(p_db_dynamic_domain == NULL)
        {
            CFG_INVALID_PARAM_RET("Error: domain not exist");
        }
        
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);

        rc = _app_dynamic_dns_domain_del_cfg(argv[0]);
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _app_dynamic_dns_domain_cmd_get_dynamic_dns_domain(para, p_node, &field);
        break;

    case CDB_OPER_SET:       

        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}



int32
app_dynamic_dns_server_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    tbl_dynamic_dns_server_t dynamic_server;
    tbl_dynamic_dns_server_t *p_db_dynamic_server = NULL;
    tbl_dynamic_dns_server_master_t *p_db_dynamic_server_master = NULL;
    int32 rc = PM_E_NONE;
    addr_t ip_addr;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_DYNAMIC_DNS_DOMAIN);
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        sal_memset(&dynamic_server, 0, sizeof(dynamic_server));
        
        CFG_PARSER_IP_ADDR(argv[0], &ip_addr, para->p_rs);
        
        cdb_addr_str2val(&dynamic_server.key.ip_addr, argv[0], IPADDR_BUF_SIZE);
        
        p_db_dynamic_server_master = tbl_dynamic_dns_server_get_master();

        /* check exist */
        p_db_dynamic_server = tbl_dynamic_dns_server_get_dynamic_dns_server(&dynamic_server.key);

        if(p_db_dynamic_server)
        {
            CFG_INVALID_PARAM_RET("Error: duplicate name server");
        }

        if(p_db_dynamic_server_master->dynamic_server_list->count == MAX_DYNAMIC_DNS_SERVER_NUM)
        {
            if(p_db_dynamic_server == NULL)
            {
                CFG_INVALID_PARAM_RET("Error: too many servers");
            }
        }

        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);

        rc = _app_dynamic_dns_server_add_cfg(&dynamic_server);
        break;

    case CDB_OPER_DEL:
        sal_memset(&dynamic_server, 0, sizeof(dynamic_server));
        
        cdb_addr_str2val(&dynamic_server.key.ip_addr, argv[0], IPADDR_BUF_SIZE);
        
        /* check exist */
        p_db_dynamic_server = tbl_dynamic_dns_server_get_dynamic_dns_server(&dynamic_server.key);

        if(p_db_dynamic_server == NULL)
        {
            CFG_INVALID_PARAM_RET("Error: name server not exist");
        }
        
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);

        rc = _app_dynamic_dns_server_del_cfg(&dynamic_server.key);
        break;

    case CDB_OPER_SET:       

        break;
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _app_dynamic_dns_server_cmd_get_dynamic_dns_server(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}


