#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "prefix.h"
#include "rt_if.h"

#if 0
static int32
_rt_if_cmd_get_if(char *ifname, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_route_if_key_t key;
    tbl_route_if_t *p_db_rtif = NULL;
    tbl_iter_args_t args;
    char ifname_ext[IFNAME_EXT_SIZE];
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    if (NULL == ifname)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_route_if_dump_one, &args);
    }
    else
    {
        sal_memset(&key, 0, sizeof(key));
        sal_strncpy(key.name, ifname, IFNAME_SIZE);
        p_db_rtif = tbl_route_if_get_route_if(&key);
        if (!p_db_rtif)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("Interface %s is not L3 interface", IFNAME_ETH2FULL(ifname, ifname_ext));
        }
        tbl_route_if_dump_one(p_db_rtif, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
rt_if_cmd_check_l3_cmd(const char *ifname, cfg_cmd_para_t *para, tbl_interface_t **pp_if, tbl_route_if_t **pp_rt_if)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_route_if_t *p_db_rtif = NULL;
    tbl_interface_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.name, ifname, IFNAME_SIZE);
    *pp_rt_if = NULL;
    
    /* check exist */
    p_db_if = tbl_interface_get_interface(&key);
    if (!p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s is not exist", key.name);
    }
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)&key);
    if (!p_db_rtif)
    {
        CFG_CONFLICT_RET("Interface %s is not L3 interface", key.name);
    }

    *pp_if = p_db_if;
    *pp_rt_if = p_db_rtif;

    return PM_E_NONE;
}

int32
rt_if_cmd_process_route_if(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t *p_node = NULL;
    tbl_route_if_t *p_db_rtif = NULL;
    tbl_route_if_key_t rtif_key;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    char *ifname = NULL;
    field_parse_t field;
    uint32 value = 0;
    int32 rc = PM_E_NONE;
    tbl_interface_t* pst_interface = NULL;
    tbl_interface_key_t if_key;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_ROUTE_IF);
    sal_memset(&if_key, 0, sizeof(if_key));
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 2, para->p_rs);

        sal_memset(&rtif_key, 0, sizeof(rtif_key));
        sal_strcpy(rtif_key.name, argv[0]);
        p_db_rtif = tbl_route_if_get_route_if(&rtif_key);

        field_id = cdb_get_field_id(argv[1], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[1]);
        }

        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch (field_id)
        {
        case TBL_ROUTE_IF_FLD_MTU:
            if (NULL == p_db_rtif)
            {
                CFG_INVALID_PARAM_RET("MTU can not set on layer 2 interface");
            }

            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);

            /*1 check if the route interface is agg */
            sal_strcpy(if_key.name, argv[0]);
            pst_interface = tbl_interface_get_interface(&if_key);
            if (NULL == pst_interface)
            {
                return PM_E_NOT_EXIST;
            }

            if (GLB_IF_TYPE_PORT_IF == pst_interface->hw_type && (pst_interface->lag || pst_interface->lacp))
            {
                CFG_INVALID_PARAM_RET("Cannot configure aggregator member");
            }

            if (GLB_IF_TYPE_LINKAGG_IF == pst_interface->hw_type)
            {
                rc = rt_if_mtu_update_agg_member(pst_interface, value);
            }
            
            rt_if_set_mtu(argv[0], value);
            break;

        case TBL_ROUTE_IF_FLD_ARP_RETRANS_TIME:
            if (NULL == p_db_rtif)
            {
                CFG_INVALID_PARAM_RET("Arp retry-interval can not set on layer 2 interface");
            }

            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rt_if_set_arp_retry_interval(argv[0], value);
            break;

        case TBL_ROUTE_IF_FLD_ARP_TIMEOUT:
            if (NULL == p_db_rtif)
            {
                CFG_INVALID_PARAM_RET("Arp timeout can not set on layer 2 interface");
            }

            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rt_if_set_arp_timeout(argv[0], value);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[1]);
            break;
        }
        break;

    case CDB_OPER_GET:
        if (0 == argc)
        {
            ifname = NULL;
        }
        else if (1 <= argc)
        {
            if (!sal_strcmp(argv[0], "all"))
            {
                ifname = NULL;
            }
            else
            {
                ifname = argv[0];
            }
        }
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        
        rc = _rt_if_cmd_get_if(ifname, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}
#endif

