/****************************************************************************
*  sflow command process
*
* Modify History:
* Revision      : R0.01
* Author        : Liwh
* Date          : 2016-05-15 11:07
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

#include <limits.h>
#include <errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "switch.h"
#include "proto.h"
#include "param_check.h"
#include "gen/tbl_sflow_collector_define.h"
#include "gen/tbl_sflow_collector.h"
#include "gen/tbl_sflow_global_define.h"
#include "gen/tbl_sflow_global.h"
#include "swth_sys.h"
#include "sflow.h"
#include "sflow_api_header.h"
#include "cdb_ds.h"
#include "ns_route.h"
#include "if_mgr.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
extern struct in_addr router_id_zebra;
extern int32
inband_snat_del_if_or_ip(const addr_ipv4_t *ip, uint16_t port, inband_snat_proto_t proto);

/* modified by liwh for bug 53920, 2019-10-14 */
uint32
acl_stm_is_ipv6_profile();
/* liwh end */
/****************************************************************************
 *
* Functions
* 
****************************************************************************/
int
sflow_check_ip(addr_t *pAddr)
{
    uint32 network;

    if (!pAddr)
    {
        return PM_E_INVALID_PARAM;
    }
    
    if (AF_INET == pAddr->family) 
    {
        u_char *p =  (u_char *)&pAddr->u.prefix4;
        
        /* filtrate ip with 255.255.255.255 */
        if (255 == p[0] && 255 == p[1] && 255 == p[2] && 255 == p[3]) 
        {
            return PM_E_INVALID_PARAM;
        }
        
        network = ntohl (*(int*)&pAddr->u.prefix4);
        if (network == 0 || IN_BADCLASS(network) || IN_MULTICAST (network)
            || IN_EXPERIMENTAL(network) || IPV4_NET127(network)
            || IPV4_NET0(network)) {
            return PM_E_INVALID_PARAM;
        }
    } else if (IN6_IS_ADDR_MULTICAST (&pAddr->u.prefix6)
               || IN6_IS_ADDR_LOOPBACK (&pAddr->u.prefix6)
               || IN6_IS_ADDR_UNSPECIFIED (&pAddr->u.prefix6)) 
    {
        return PM_E_INVALID_PARAM;
    }
    
    return PM_E_NONE;
}

int32
sflow_api_set_sflow_agent(addr_t *ip_address)
{
    tbl_sflow_global_t *p_sflow_glb = NULL;
    tbl_sflow_global_t sflow_glb;

    p_sflow_glb = tbl_sflow_global_get_sflow_global();
    if (NULL == p_sflow_glb)
    {
        return PM_E_NOT_INITED;
    }

    sal_memset(&sflow_glb, 0, sizeof(tbl_sflow_global_t));    
    sal_memcpy(&sflow_glb.agent, ip_address, sizeof(addr_t));
    tbl_sflow_global_set_sflow_global_field(&sflow_glb, TBL_SFLOW_GLOBAL_FLD_AGENT);

    return PM_E_NONE;
}

int32
sflow_api_set_sflow_agent_ipv6(addr_t *ip_address)
{
    tbl_sflow_global_t *p_sflow_glb = NULL;
    tbl_sflow_global_t sflow_glb;

    p_sflow_glb = tbl_sflow_global_get_sflow_global();
    if (NULL == p_sflow_glb)
    {
        return PM_E_NOT_INITED;
    }

    sal_memset(&sflow_glb, 0, sizeof(tbl_sflow_global_t));    
    sal_memcpy(&sflow_glb.agent_ipv6, ip_address, sizeof(addr_t));
    tbl_sflow_global_set_sflow_global_field(&sflow_glb, TBL_SFLOW_GLOBAL_FLD_AGENT_IPV6);

    return PM_E_NONE;
}

int32
sflow_api_set_sflow_version(uint32 version)
{
    tbl_sflow_global_t             sflow_glb;
    tbl_sflow_global_t             *p_sflow_glb = NULL;   

    /* check value isn't change */
    p_sflow_glb = tbl_sflow_global_get_sflow_global();
    if (!p_sflow_glb)
    {
        return PM_E_NOT_EXIST;
    }

    /* check value isn't change */
    if (p_sflow_glb->version == version)
    {
        return PM_E_NONE;
    }

    sal_memset(&sflow_glb, 0, sizeof(tbl_sflow_global_t));
    sflow_glb.version = version;
    tbl_sflow_global_set_sflow_global_field(&sflow_glb, TBL_SFLOW_GLOBAL_FLD_VERSION);
    return PM_E_NONE;
}

int32
sflow_api_clean_interface(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs)
{
    if (p_db_if->sflow_flow_ingress || p_db_if->sflow_flow_egress)
    {
        if_mgr_set_sflow_flow_enable(p_db_if, FALSE, 
            p_db_if->sflow_flow_ingress,
            p_db_if->sflow_flow_egress);
    }

    if (GLB_SFLOW_DEFAULT_RATE != p_db_if->sflow_rate)
    {
        if_mgr_set_sflow_rate(p_db_if, GLB_SFLOW_DEFAULT_RATE);
    }

    if (p_db_if->sflow_counter_en)
    {
        if_mgr_set_sflow_counter_enable(p_db_if, FALSE);
    }
    
    return PM_E_NONE;
}

int32
sflow_api_set_sflow_enable(uint32 enable, cfg_cmd_para_t *para)
{
    tbl_sflow_global_t             sflow_glb;
    tbl_sflow_global_t             *p_sflow_glb = NULL;   
    addr_t                         address;
    tbl_sflow_collector_master_t   *p_glb_collector = NULL;
    tbl_sflow_collector_t          *p_db_collector = NULL;
    ctclib_slistnode_t             *listnode = NULL;
    ctclib_slistnode_t             *next = NULL;    
    int32                          sock = 0;    
    char                           szCmd[30];
    char                           buf[128];

    /* check value isn't change */
    p_sflow_glb = tbl_sflow_global_get_sflow_global();
    if (!p_sflow_glb)
    {
        return PM_E_NOT_INITED;
    }

    /* check value isn't change */
    if (p_sflow_glb->enable == enable)
    {
        if (enable)
        {
            return PM_E_EXIST;
        }
        else
        {
            return PM_E_NOT_EXIST;
        }
    }

    sal_memset(&sflow_glb, 0, sizeof(tbl_sflow_global_t));
    sflow_glb.enable = enable;
    tbl_sflow_global_set_sflow_global_field(&sflow_glb, TBL_SFLOW_GLOBAL_FLD_ENABLE);

    if (!enable)
    {
        /* clear interface sflow rate, sample, counter */
        tbl_interface_iterate((TBL_ITER_CB_FUNC)sflow_api_clean_interface, NULL);

        /* set sflow agent to 0.0.0.0 */
        sal_memset(&address, 0, sizeof(addr_t));
        sflow_api_set_sflow_agent(&address);

        /* modified by liwh for bug 53920, 2019-10-12 */
        /* set sflow agent_ipv6 to :: */
        sflow_api_set_sflow_agent_ipv6(&address);
        /* liwh end */

        /* set sflow counter interval to default value */
        sflow_api_set_sflow_counter_interval(SFLOW_COUNTER_DEFAULT_INTERVAL);

        /* set sflow version to default version 5 */
        sflow_api_set_sflow_version(SFLOW_VERSION5);

        /* remove all sflow collector */
        p_glb_collector = tbl_sflow_collector_get_master();
        if (!p_glb_collector)
        {
            SFLOW_LOG_ERR("TBL_SFLOW_COLLECTOR is not initialized");
            return PM_E_NOT_EXIST;
        }
        
        CTCLIB_SLIST_LOOP_DEL(p_glb_collector->sflow_collector_list, p_db_collector, listnode, next)
        {
            if (p_db_collector->transdelay_timer)
            {
                ctc_task_delete_timer(p_db_collector->transdelay_timer);
                p_db_collector->transdelay_timer = NULL;    
            }
    
            /* if sock is exist, close socket */
            if (p_db_collector->sendfd > 0)
            {
                sock = p_db_collector->sendfd;
                close(sock);
            }

            sal_memset(szCmd, 0, 30);
            sal_memset(buf, 0, 12);
    
            snprintf(szCmd, 30, 
                "1#%d#%s", p_db_collector->key.udp_port,
                cdb_addr_val2str(buf, 128, &p_db_collector->key.addr));
            ns_route_delete_route(szCmd, para);
            inband_snat_del_if_or_ip(&p_db_collector->key.addr.u.prefix4, p_db_collector->key.udp_port, INBAND_IP_PROTO_UDP);
            
            /* delete sflow collector */
            tbl_sflow_collector_del_sflow_collector(&p_db_collector->key);
        }
    }

    return PM_E_NONE;
}

static int32
_sflow_cmd_get_sflow_global(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_sflow_global_t* p_sflow_glb = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_sflow_glb = tbl_sflow_global_get_sflow_global();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_sflow_global_dump_one(p_sflow_glb, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
sflow_cmd_process_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                   rc = PM_E_NONE;
    addr_t                  ip_address;
    uint32                  sflow_enable = 0;
    uint32                  sflow_version = 0;
    uint32                  sflow_counter_interval = 0;
    field_parse_t           field;
    cdb_node_t              *p_node = NULL;
    tbl_sflow_global_t      *p_sflow_glb = NULL;
    int32                   field_id = 0;
    cdb_field_t             *p_field = NULL;
    int32                   value = 0;
    tbl_sflow_global_t      sflow_glb;

    sal_memset(&sflow_glb, 0, sizeof(tbl_sflow_global_t));
    sal_memset(&ip_address, 0, sizeof(addr_t));

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SFLOW_GLOBAL);
    if (!p_node)
    {
       CFG_FUNC_ERR_RET("TBL_SFLOW_GLOBAL is not initialized");     
    }
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_SET:
        /* check exist */
        p_sflow_glb = tbl_sflow_global_get_sflow_global();
        if (!p_sflow_glb)
        {
            CFG_CONFLICT_RET("SFLOW init failed");
        }

        if (0 == argc)
        {
            return PM_E_INVALID_PARAM;    
        }

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
        case TBL_SFLOW_GLOBAL_FLD_ENABLE:
            PM_CFG_GET_UINT(sflow_enable, argv[1], para->p_rs);
            rc = sflow_api_set_sflow_enable(sflow_enable, para);
            if (PM_E_EXIST == rc)
            {
                CFG_INVALID_PARAM_RET("sFlow is already enabled");
            }
            else if (PM_E_NOT_EXIST == rc)
            {
                CFG_INVALID_PARAM_RET("sFlow is not enabled globally!");
            }
            break;
        case TBL_SFLOW_GLOBAL_FLD_VERSION:
            PM_CFG_GET_UINT(sflow_version, argv[1], para->p_rs);
            if (FALSE == sflow_api_get_sflow_enable())
            {
                CFG_INVALID_PARAM_RET("sFlow is not enabled globally!");
            }
            rc = sflow_api_set_sflow_version(sflow_version);
            break;
            
        case TBL_SFLOW_GLOBAL_FLD_AGENT:
            if (FALSE == sflow_api_get_sflow_enable())
            {
                CFG_INVALID_PARAM_RET("sFlow is not enabled globally!");
            }

            if (argc > 1)
            {
                cdb_addr_str2val(&ip_address, argv[1], IPADDR_BUF_SIZE);
    
                rc = sflow_check_ip(&ip_address);
                if (rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid Agent IP address");
                }
            }
                
            rc = sflow_api_set_sflow_agent(&ip_address);
            if (rc)
            {
                CFG_INVALID_PARAM_RET("Invalid Agent IP address");
            }
            break;

       /* modified by liwh for bug 53920, 2019-10-11 */
       case TBL_SFLOW_GLOBAL_FLD_AGENT_IPV6:
            if (FALSE == sflow_api_get_sflow_enable())
            {
                CFG_INVALID_PARAM_RET("sFlow is not enabled globally!");
            }

            if (argc > 1)
            {
                cdb_addr_str2val(&ip_address, argv[1], IPADDR_BUF_SIZE);
    
                rc = sflow_check_ip(&ip_address);
                if (rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid Agent IP address");
                }
            }
                
            rc = sflow_api_set_sflow_agent_ipv6(&ip_address);
            if (rc)
            {
                CFG_INVALID_PARAM_RET("Invalid Agent IP address");
            }
            break;
        /* liwh end */
            
        case TBL_SFLOW_GLOBAL_FLD_COUNTER_INTERVAL:      
            if (FALSE == sflow_api_get_sflow_enable())
            {
                CFG_INVALID_PARAM_RET("sFlow is not enabled globally!");
            }

            PM_CFG_GET_UINT(sflow_counter_interval, argv[1], para->p_rs);
            rc = sflow_api_set_sflow_counter_interval(sflow_counter_interval);
            break;

        case TBL_SFLOW_GLOBAL_FLD_SFLOW_ALL:
            PM_CFG_GET_BOOL(value, argv[1], para->p_rs);
            GLB_SET_FLAG_COND(value, sflow_glb.sflow, SFLOW_FLAG_ALL);
            rc = tbl_sflow_global_set_sflow_global_field(&sflow_glb, field_id);
            break;
        
        case TBL_SFLOW_GLOBAL_FLD_SFLOW_COUNTER:
            PM_CFG_GET_BOOL(value, argv[1], para->p_rs);
            GLB_SET_FLAG_COND(value, sflow_glb.sflow, SFLOW_FLAG_COUNTER);
            rc = tbl_sflow_global_set_sflow_global_field(&sflow_glb, field_id);
            break;

        case TBL_SFLOW_GLOBAL_FLD_SFLOW_SAMPLE:
            PM_CFG_GET_BOOL(value, argv[1], para->p_rs);
            GLB_SET_FLAG_COND(value, sflow_glb.sflow, SFLOW_FLAG_SAMPLE);
            rc = tbl_sflow_global_set_sflow_global_field(&sflow_glb, field_id);
            break;

        case TBL_SFLOW_GLOBAL_FLD_SFLOW_PACKET:
            PM_CFG_GET_BOOL(value, argv[1], para->p_rs);
            GLB_SET_FLAG_COND(value, sflow_glb.sflow, SFLOW_FLAG_PACKET);
            rc = tbl_sflow_global_set_sflow_global_field(&sflow_glb, field_id);
            break;
            
        default:
            CFG_INVALID_PARAM_RET("Invalid parameters");
            break;
        }
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
            
        rc = _sflow_cmd_get_sflow_global(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_sflow_cmd_add_collector(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    bool                           sflow_glb_enable = FALSE;
    cdb_node_t                     *p_node = NULL;
    tbl_sflow_collector_t          collector;
    tbl_sflow_collector_master_t   *p_glb_collector = NULL;
    tbl_sflow_collector_t          *p_db_collector = NULL;
    int32                          field_id = 0;
    cdb_field_t                    *p_field = NULL;
    int32                          rc = PM_E_NONE;
    uint8                          i = 0;
    int32                          sock = 0;

    sal_memset(&collector, 0, sizeof(tbl_sflow_collector_t));
    
    /* check sflow is enable */
    sflow_glb_enable = sflow_api_get_sflow_enable();
    if (!sflow_glb_enable)
    {
        CFG_INVALID_PARAM_RET("sFlow is not enabled globally!");
    }

    /* check tbl exist */
    p_glb_collector = tbl_sflow_collector_get_master();
    if (!p_glb_collector)
    {
        CFG_FUNC_ERR_RET("TBL_SFLOW_COLLECTOR is not initialized");
    }
    
    p_node = cdb_get_tbl(TBL_SFLOW_COLLECTOR);
    if (!p_node)
    {
       CFG_FUNC_ERR_RET("TBL_SFLOW_COLLECTOR is not initialized");     
    }

    field_id = cdb_get_field_id(argv[i], p_node, &p_field);
    if (field_id < 0)
    {
        CFG_FUNC_ERR_RET("Unknown field %s", argv[i]);
    }

    if (TBL_SFLOW_COLLECTOR_FLD_KEY == field_id)
    {
        /* get collector ip address */
        i++;

        /* modified by liwh for bug 53920, 2019-10-11 */
        /*CFG_PARSER_IP_ADDR(argv[i], &collector.key.addr.u.prefix4, para->p_rs);
        collector.key.addr.family = AF_INET;*/

        rc = cdb_addr_str2val(&collector.key.addr, argv[i], sal_strlen(argv[i]));
        if (rc < 0) {
            CFG_INVALID_PARAM_RET("Invalid IP address %s", argv[i]);
        }
        /* liwh end */

        /* check ip address is valid */
        if (sflow_check_ip(&collector.key.addr) != 0) 
        {
            CFG_INVALID_PARAM_RET("Invalid address!");
        }

        /* get collector udp port */
        i++;
        PM_CFG_GET_UINT(collector.key.udp_port, argv[i], para->p_rs);

        /* if collector is exist, return and don't print error */
        p_db_collector = tbl_sflow_collector_get_sflow_collector(&collector.key);
        if (p_db_collector)
        {
            CFG_FUNC_ERR_RET("Sflow collector is exist");
        }

        /* check collector number */
        if (p_glb_collector->sflow_collector_list->count >= SFLOW_MAXCOLLECTOR)
        {
            CFG_FUNC_ERR_RET("Sflow collector number exceeds!");
        }

        /* start sflow collector socket */
        sock = socket (collector.key.addr.family, SOCK_DGRAM, 0);
        if (sock < 0)
        {
            CFG_FUNC_ERR_RET("Create sflow collector %s:%s failure", argv[1], argv[2]);
        }

        collector.sendfd = (uint32)sock;

        /* add sflow collector */
        rc = tbl_sflow_collector_add_sflow_collector(&collector);
        if (rc)
        {
            close(sock);
            CFG_FUNC_ERR_RET("Create sflow collector %s:%s failure", argv[1], argv[2]);
        }
    }
    
    return rc;
}

static int32
_sflow_cmd_delete_collector(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    bool                           sflow_glb_enable = FALSE;
    cdb_node_t                     *p_node = NULL;
    tbl_sflow_collector_t          collector;
    tbl_sflow_collector_master_t   *p_glb_collector = NULL;
    tbl_sflow_collector_t          *p_db_collector = NULL;
    int32                          field_id = 0;
    cdb_field_t                    *p_field = NULL;
    int32                          rc = PM_E_NONE;
    uint8                          i = 0;
    uint32                         is_inband = 0;
    int32                          sock = 0;
    tbl_ns_route_t                 *p_ns_route  = NULL;
    ns_route_key_t key;
    
    sal_memset(&collector, 0, sizeof(tbl_sflow_collector_t));
    sal_memset(&key, 0, sizeof(tbl_ns_route_key_t));

    /* modified by liwh for bug 53920, 2019-10-11 */
    //key.ip.family = AF_INET;
    /* liwh end */
    
    /* check sflow is enable */
    sflow_glb_enable = sflow_api_get_sflow_enable();
    if (!sflow_glb_enable)
    {
        CFG_INVALID_PARAM_RET("sFlow is not enabled globally!");
    }

    /* check tbl exist */
    p_glb_collector = tbl_sflow_collector_get_master();
    if (!p_glb_collector)
    {
        CFG_FUNC_ERR_RET("TBL_SFLOW_COLLECTOR is not initialized");
    }
    
    p_node = cdb_get_tbl(TBL_SFLOW_COLLECTOR);
    if (!p_node)
    {
       CFG_FUNC_ERR_RET("TBL_SFLOW_COLLECTOR is not initialized");     
    }

    field_id = cdb_get_field_id(argv[i], p_node, &p_field);
    if (field_id < 0)
    {
        CFG_FUNC_ERR_RET("Unknown field %s", argv[i]);
    }

    if (TBL_SFLOW_COLLECTOR_FLD_KEY == field_id)
    {
        /* get collector ip address */
        i++;

        /* modified by liwh for bug 53920, 2019-10-11 */
        /*CFG_PARSER_IP_ADDR(argv[i], &collector.key.addr.u.prefix4, para->p_rs);
        collector.key.addr.family = AF_INET;
        key.ip.u.prefix4.s_addr = collector.key.addr.u.prefix4.s_addr;*/

        rc = cdb_addr_str2val(&collector.key.addr, argv[i], sal_strlen(argv[i]));
        if (rc < 0) {
            CFG_INVALID_PARAM_RET("Invalid IP address %s", argv[i]);
        }

        sal_memcpy(&key.ip, &collector.key.addr, sizeof(addr_t));
        /* liwh end */
        
        /* get collector udp port */
        i++;
        PM_CFG_GET_UINT(collector.key.udp_port, argv[i], para->p_rs);
        key.port = collector.key.udp_port;
        key.proto = 1;
        i++;
        if (NULL == argv[3])
        {
            CFG_FUNC_ERR_RET("Invalid paraments!");  
        }
        if (0 == sal_strcmp(argv[3],"outband"))
        {
            is_inband = 0;
        }
        if (0 == sal_strcmp(argv[3],"inband"))
        {
            is_inband = 1;
        }
        /* if collector is not exist, return error */
        p_db_collector = tbl_sflow_collector_get_sflow_collector(&collector.key);
        if (!p_db_collector)
        {
            CFG_FUNC_ERR_RET("Sflow collector does not exist!");    
        }
        p_ns_route = tbl_ns_route_get_ns_route(&key);
        if (!p_ns_route)
        {
            CFG_FUNC_ERR_RET("Sflow collector does not exist!");    
        }
        if (is_inband != p_ns_route->is_inband)
        {
            CFG_FUNC_ERR_RET("Sflow collector does not exist!"); 
        }
        if (p_db_collector->transdelay_timer)
        {
            ctc_task_delete_timer(p_db_collector->transdelay_timer);
            p_db_collector->transdelay_timer = NULL;    
        }

        /* if sock is exist, close socket */
        if (p_db_collector->sendfd > 0)
        {
            sock = p_db_collector->sendfd;
            close(sock);
        }
        CTC_TASK_STOP_TIMER(p_db_collector->t_log_timer);
        /* delete sflow collector */
        rc = tbl_sflow_collector_del_sflow_collector(&collector.key);
        if (rc)
        {
            CFG_FUNC_ERR_RET("Delete sflow collector %s:%s failure", argv[1], argv[2]);
        }
    }
    
    return rc;
}

int32
sflow_cmd_parse_collector_field(char **argv, int32 argc, cdb_node_t* p_node, field_parse_t *field, uint32 with_key)
{
    cdb_tbl_info_t    *p_tbl_info = p_node->tbl_info;
    uint32            field_id = FLD_ID_ALL;
    uint32            key_id = 0;

    if (with_key)
    {
        key_id = 2;
    }
    
    sal_memset(field, FLD_ID_ALL, sizeof(field_parse_t));
    if (key_id >= argc)
    {
        return PM_E_NONE;
    }
    
    for (field_id = 0; field_id < p_tbl_info->field_num; field_id++)
    {
        if (!sal_strcmp(argv[key_id], p_tbl_info->field[field_id].name))
        {
            field->field_id[0] = field_id;
            return PM_E_NONE;
        }
    }
    
    return PM_E_FAIL;
}

static int32
_sflow_cmd_get_collector(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    bool                           sflow_glb_enable = FALSE;
    cdb_node_t                     *p_node = NULL;
    tbl_sflow_collector_t          collector;
    tbl_sflow_collector_master_t   *p_glb_collector = NULL;
    tbl_sflow_collector_t          *p_db_collector = NULL;
    int32                          field_id = 0;
    int32                          rc = PM_E_NONE;
    uint8                          i = 0; 
    FILE                           *fp = NULL;
    tbl_iter_args_t                args;
    ctclib_slistnode_t             *listnode = NULL;
    field_parse_t                  field;

    sal_memset(&collector, 0, sizeof(tbl_sflow_collector_t));
    
    /* check sflow is enable */
    sflow_glb_enable = sflow_api_get_sflow_enable();
    if (!sflow_glb_enable)
    {
        return PM_E_NONE;
    }

    /* check tbl exist */
    p_glb_collector = tbl_sflow_collector_get_master();
    if (!p_glb_collector)
    {
        CFG_FUNC_ERR_RET("TBL_SFLOW_COLLECTOR is not initialized");
    }
    
    p_node = cdb_get_tbl(TBL_SFLOW_COLLECTOR);
    if (!p_node)
    {
       CFG_FUNC_ERR_RET("TBL_SFLOW_COLLECTOR is not initialized");     
    }

    field_id = sflow_cmd_parse_collector_field(argv, argc, p_node, &field, TRUE);
    if (field_id < 0)
    {
        CFG_FUNC_ERR_RET("Unknown field %s", argv[i]);
    }
    
    if (0 == argc)
    { 
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        args.argv[0] = &field;
        args.argv[1] = fp;

        CTCLIB_SLIST_LOOP(p_glb_collector->sflow_collector_list, p_db_collector, listnode)
        {
            tbl_sflow_collector_dump_one(p_db_collector, &args);
        }

        sal_fclose(fp);
    }
    else if (argc >= 2)
    {
        /* modified by liwh for bug 53920, 2019-10-11 */
        /*CFG_PARSER_IP_ADDR(argv[0], &collector.key.addr.u.prefix4, para->p_rs);
        collector.key.addr.family = AF_INET;
        PM_CFG_GET_UINT(collector.key.udp_port, argv[1], para->p_rs); */

        rc = cdb_addr_str2val(&collector.key.addr, argv[0], sal_strlen(argv[0]));
        if (rc < 0) {
            CFG_INVALID_PARAM_RET("Invalid IP address %s", argv[0]);
        }
        /* liwh end */
        
        p_db_collector = tbl_sflow_collector_get_sflow_collector(&collector.key);
        if (!p_db_collector)
        {
            CFG_FUNC_ERR_RET("Sflow collector %s:%s is not exist!", argv[0], argv[1]);
        }
        
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        args.argv[0] = &field;
        args.argv[1] = fp;

        tbl_sflow_collector_dump_one(p_db_collector, &args);

        sal_fclose(fp);
    }
    
    return rc;
}


int32
sflow_cmd_process_collector(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                   rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        rc = _sflow_cmd_add_collector(argv, argc, para);
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_DEL:
        rc = _sflow_cmd_delete_collector(argv, argc, para);
        break;

    case CDB_OPER_GET:
        rc = _sflow_cmd_get_collector(argv, argc, para);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

void
sflow_show_system (cfg_cmd_para_t *para, FILE *fp)
{ 
    tbl_sflow_global_t             *p_sflow_glb = NULL;
    char                           buf[CMD_BUF_256];
    char                           ifname_ext[IFNAME_EXT_SIZE];
    tbl_sflow_collector_t          *p_db_collector = NULL;
    tbl_sflow_collector_master_t   *p_glb_collector = NULL;
    ctclib_slistnode_t             *listnode = NULL;
    uint32                         index = 0;
    tbl_interface_master_t         *p_glb_interface = NULL;
    tbl_interface_t                *p_db_if = NULL;
    char                           cli_str[SFLOW_DISPLAY_LEN] = {0};
    uint32                         strlen = 0;
    char                           *ifsampler_str = NULL;
    /* modified by liwh for bug 45810, 2018-02-06 */
    char                           szCmd[30];
    int32                          ret = PM_E_NONE;
    /* liwh end */

    /* get sflow global */
    p_sflow_glb = tbl_sflow_global_get_sflow_global();
    if (!p_sflow_glb)
    {
        return;
    }

    /* get collector global */
    p_glb_collector = tbl_sflow_collector_get_master();
    if (!p_glb_collector)
    {
        SFLOW_LOG_ERR("TBL_SFLOW_COLLECTOR is not initialized");
        return;
    }

    /* get interface global */
    p_glb_interface = tbl_interface_get_master();
    if (!p_glb_interface)
    {
        SFLOW_LOG_ERR("TBL_INTERFACE is not initialized");
        return;        
    }

    if (!p_sflow_glb->enable)
    {
        sal_fprintf(fp, "sFlow is disabled\n");
        return;
    }

    /*added by yejl for sflow version 5*/
    if (SFLOW_VERSION5 == p_sflow_glb->version)
    {
        sal_fprintf(fp, "sFlow Version: %d\n", SFLOW_VERSION5);
    }
    else
    {
        sal_fprintf(fp, "sFlow Version: %d\n", SFLOW_VERSION4);
    }
    /*ended by yejl*/
    
    sal_fprintf(fp, "sFlow Global Information:\n");

    sal_fprintf(fp, " %-30s: %s\n", "Agent IPv4 address", 
        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, (addr_ipv4_t*)&p_sflow_glb->agent.u.prefix4));

    sal_fprintf(fp, " %-30s: %s\n", "Agent IPv6 address", 
        cdb_addr_ipv6_val2str(buf, CMD_BUF_256, (addr_ipv6_t*)&p_sflow_glb->agent_ipv6.u.prefix6));

    sal_fprintf(fp, " %-30s: %u seconds\n", "Counter Sampling Interval", p_sflow_glb->counter_interval);


    if (0 == p_glb_collector->sflow_collector_list->count)
    {
        sal_fprintf(fp, " %-30s: Not configured\n", "Collector IP");
    }
    else
    {
        index = 1;
        CTCLIB_SLIST_LOOP(p_glb_collector->sflow_collector_list, p_db_collector, listnode)
        {
            /* modified by liwh for bug 45810, 2018-02-06 */
            sal_memset(szCmd, 0, 32);
            snprintf(szCmd, 30, 
                "1#%d#%s", p_db_collector->key.udp_port,
                cdb_addr_val2str(buf, 128, &p_db_collector->key.addr));
            ret = ns_route_check_inband(szCmd, para);
            /* liwh end */

            if (ret)
            {
                if (AF_INET == p_db_collector->key.addr.family)
                {
                     sal_fprintf(fp, " Collector %d:\n  IPv4 Address: %s\n  Port: %d\n", index,
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, (addr_ipv4_t*)&p_db_collector->key.addr.u.prefix4),
                        p_db_collector->key.udp_port);                
                }
                else
                {
                    sal_fprintf(fp, " Collector %d:\n  IPv6 Address: %s\n  Port: %d\n", index,
                        cdb_addr_ipv6_val2str(buf, CMD_BUF_256, (addr_ipv6_t*)&p_db_collector->key.addr.u.prefix6),
                        p_db_collector->key.udp_port); 
                }
            }
            else
            {
                if (AF_INET == p_db_collector->key.addr.family)
                {
                    sal_fprintf(fp, " Collector %d:\n  mgmt-if IPv4 Address: %s\n  Port: %d\n", index,
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, (addr_ipv4_t*)&p_db_collector->key.addr.u.prefix4),
                        p_db_collector->key.udp_port);
                }
                else
                {
                     sal_fprintf(fp, " Collector %d:\n  mgmt-if IPv6 Address: %s\n  Port: %d\n", index,
                        cdb_addr_ipv6_val2str(buf, CMD_BUF_256, (addr_ipv6_t*)&p_db_collector->key.addr.u.prefix6),
                        p_db_collector->key.udp_port);                      
                }
            }
            
            index++;
        }
    }

    sal_fprintf(fp, "\nsFlow Port Information:\n"
        "                                 Flow-Sample   Flow-Sample\n"
        " Port        Counter   Flow      Direction     Rate\n"
        " --------------------------------------------------------\n");

    CTCLIB_SLIST_LOOP(p_glb_interface->if_list, p_db_if, listnode)
    {    
        if (INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            continue;
        }
        
        if (!p_db_if->sflow_counter_en && !p_db_if->sflow_flow_ingress && !p_db_if->sflow_flow_egress)
        {
            continue;
        }
        
        /*modified by yejl to optimize the ifname*/
        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
        strlen += sal_snprintf(cli_str + strlen, sizeof(cli_str) - strlen, " %-12s", ifname_ext);
        /*ended by yejl*/
        
        if (p_db_if->sflow_counter_en)
        {
            strlen += sal_snprintf(cli_str + strlen, sizeof(cli_str) - strlen, "%-10s", "Enable");    
        }
        else
        {
            strlen += sal_snprintf(cli_str + strlen, sizeof(cli_str) - strlen, "%-10s", "Disable");
        }

        if (p_db_if->sflow_flow_ingress || p_db_if->sflow_flow_egress)
        {
            if (p_db_if->sflow_flow_ingress && p_db_if->sflow_flow_egress)
            {
                ifsampler_str = "Both";    
            }
            else if (p_db_if->sflow_flow_ingress)
            {
                ifsampler_str = "Input";
            }
            else if (p_db_if->sflow_flow_egress)
            {
                ifsampler_str = "Output";
            }
            else
            {
                ifsampler_str = "Unknow";
            }
            
            strlen += sal_snprintf(cli_str + strlen, sizeof(cli_str) - strlen, "%-10s", "Enable");
            strlen += sal_snprintf(cli_str + strlen, sizeof(cli_str) - strlen, "%-14s", ifsampler_str);
            strlen += sal_snprintf(cli_str + strlen, sizeof(cli_str) - strlen, "%-12u\n", p_db_if->sflow_rate);             
        }
        else
        {
            strlen += sal_snprintf(cli_str + strlen, sizeof(cli_str) - strlen, "%-10s", "Disable");
            strlen += sal_snprintf(cli_str + strlen, sizeof(cli_str) - strlen, "%-14s", "N/A");

            if ((GLB_SFLOW_DEFAULT_RATE != p_db_if->sflow_rate) && (0 != p_db_if->sflow_rate))
            {
                strlen += sal_snprintf(cli_str + strlen, sizeof(cli_str) - strlen, "%-12u\n", p_db_if->sflow_rate);
            }
            else
            {
                strlen += sal_snprintf(cli_str + strlen, sizeof(cli_str) - strlen, "%-s\n", "N/A");
            }
        }

        sal_fprintf(fp, "%s", cli_str);
        strlen = 0;
        sal_memset(cli_str, 0, SFLOW_DISPLAY_LEN);
    }
     
    return; 
}

#define SFLOW_DBG_PRINT_BANNER() \
do { \
    if (banner) \
    { \
        sal_fprintf(fp, "%-12s %-16s %-20s %-6s\n", "Module", "Feature", "Type", "Status"); \
        sal_fprintf(fp, "------------+----------------+--------------------+------\n"); \
        banner = FALSE; \
    } \
} while (0)

int32
sflow_show_debugging_all(uint32 detail, cfg_cmd_para_t *para)
{
    tbl_sflow_global_t        *p_sflow_glb = NULL;
    uint32                    is_on = FALSE;
    FILE                      *fp = NULL;  
    bool                      first = TRUE;
    bool                      banner = TRUE;
    
    /* get sflow global */
    p_sflow_glb = tbl_sflow_global_get_sflow_global();
    if (!p_sflow_glb)
    {
        return PM_E_FAIL;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    is_on = sflow_debug_is_on(SFLOW_FLAG_COUNTER);
    if (detail || is_on)
    {
        SFLOW_DBG_PRINT_BANNER();
        sal_fprintf(fp, "%-12s %-16s %-20s %-6s\n", "sflow", "sflow", "counter", is_on ? "on" : "off");
        first = FALSE;
    }

    is_on = sflow_debug_is_on(SFLOW_FLAG_SAMPLE);
    if (detail || is_on)
    {
        SFLOW_DBG_PRINT_BANNER();    
        if (first)
        {
            sal_fprintf(fp, "%-12s %-16s %-20s %-6s\n", "sflow", "sflow", "sample", is_on ? "on" : "off");
            first = FALSE;
        }
        else
        {
            sal_fprintf(fp, "%-12s %-16s %-20s %-6s\n", " ", "sflow", "sample", is_on ? "on" : "off");
        }
    }

    is_on = sflow_debug_is_on(SFLOW_FLAG_PACKET);
    if (detail || is_on)
    {
        SFLOW_DBG_PRINT_BANNER();    
        if (first)
        {
            sal_fprintf(fp, "%-12s %-16s %-20s %-6s\n", "sflow", "sflow", "packet", is_on ? "on" : "off");
            first = FALSE;
        }
        else
        {
            sal_fprintf(fp, "%-12s %-16s %-20s %-6s\n", " ", "sflow", "packet", is_on ? "on" : "off");
        }
    }

    if (!first)
    {
        sal_fprintf(fp, "\n");
    }

    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE; 
}

int32
sflow_cmd_show_sflow(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                    rc = PM_E_NONE;
    FILE                     *fp = NULL;
    uint32                   detail = FALSE;
     
    if (0 == argc)
    {  
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
    
        sflow_show_system(para, fp);
        
        sal_fclose(fp);
        fp = NULL;
    }
    else if ((1 == argc) && (0 == sal_memcmp(argv[0], "debugging", sal_strlen("debugging"))))
    {
        sflow_show_debugging_all(detail, para);
    }
    else if ((2 == argc) && (0 == sal_memcmp(argv[0], "debugging", sal_strlen("debugging"))))
    {
        if (0 == sal_memcmp(argv[1], "detail", sal_strlen("detail")))
        {
            detail = TRUE;
        }
        sflow_show_debugging_all(detail, para);
    }

    return rc;
}

int32
sflow_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_SFLOW_GLOBAL,   sflow_cmd_process_global); 

    cdb_register_cfg_tbl_cb(TBL_SFLOW_COLLECTOR,   sflow_cmd_process_collector); 

    cdb_register_cfg_act_cb(ACT_SHOW_SFLOW, sflow_cmd_show_sflow);  

    ipc_register_pkt_rx_fn(GLB_PKT_SFLOW, sflow_pkt_func);

    return 0;
}

