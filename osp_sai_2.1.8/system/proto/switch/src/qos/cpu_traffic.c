#include "proto.h"
#include "lib_tblinc.h"
#include "switch.h"
#include "glb_cpu_traffic_define.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "gen/tbl_cpu_traffic_define.h"
#include "gen/tbl_cpu_traffic.h"
#include "gen/tbl_cpu_traffic_group_define.h"
#include "gen/tbl_cpu_traffic_group.h"

static copp_reason_info_t cpu_traffic_info[] =
{
    {GLB_CPU_TRAFFIC_ARP,            "arp"          , COPP_REASON_DEFAULT_VALUE_ARP,GLB_CPU_TRAFFIC_GROUP_1, TRUE},
    {GLB_CPU_TRAFFIC_STP,            "bpdu"         , COPP_REASON_DEFAULT_VALUE_BPDU,GLB_CPU_TRAFFIC_GROUP_4, TRUE},
    {GLB_CPU_TRAFFIC_LACP,           "slow-protocol", COPP_REASON_DEFAULT_VALUE_SLOW_PROTO,GLB_CPU_TRAFFIC_GROUP_4, TRUE},
    {GLB_CPU_TRAFFIC_EAPOL,          "eapol"        , COPP_REASON_DEFAULT_VALUE_EAPOL,GLB_CPU_TRAFFIC_GROUP_3, TRUE},
    {GLB_CPU_TRAFFIC_LLDP,           "lldp"         , COPP_REASON_DEFAULT_VALUE_LLDP,GLB_CPU_TRAFFIC_GROUP_3, TRUE},
    {GLB_CPU_TRAFFIC_IGMP,           "igmp"         , COPP_REASON_DEFAULT_VALUE_IGMP,GLB_CPU_TRAFFIC_GROUP_3, TRUE},
    {GLB_CPU_TRAFFIC_ERPS,           "erps"         , COPP_REASON_DEFAULT_VALUE_ERPS,GLB_CPU_TRAFFIC_GROUP_4, TRUE},
    {GLB_CPU_TRAFFIC_DHCP,           "dhcp"         , COPP_REASON_DEFAULT_VALUE_DHCP,GLB_CPU_TRAFFIC_GROUP_3, TRUE},
    {GLB_CPU_TRAFFIC_OSPF,           "ospf"         , COPP_REASON_DEFAULT_VALUE_OSPF,GLB_CPU_TRAFFIC_GROUP_4, TRUE},
    {GLB_CPU_TRAFFIC_PIM,            "pim"          , COPP_REASON_DEFAULT_VALUE_PIM,GLB_CPU_TRAFFIC_GROUP_4, FALSE},
    /*modified by yejl for bug 49548, 2018-12-12*/
    /*modified by yejl for bug 50657, 2019-01-14*/
    {GLB_CPU_TRAFFIC_VRRP,           "vrrp"         , COPP_REASON_DEFAULT_VALUE_VRRP,GLB_CPU_TRAFFIC_GROUP_4, TRUE},
    {GLB_CPU_TRAFFIC_BGP,            "bgp"          , COPP_REASON_DEFAULT_VALUE_BGP,GLB_CPU_TRAFFIC_GROUP_4, FALSE},
    {GLB_CPU_TRAFFIC_COPY_TO_CPU,    "management-traffic"   , COPP_REASON_DEFAULT_VALUE_MAGMT_TRA,GLB_CPU_TRAFFIC_GROUP_5, TRUE},
    {GLB_CPU_TRAFFIC_PACKET_IN,      "packet-in"          , COPP_REASON_DEFAULT_VALUE_PACKET_IN,GLB_CPU_TRAFFIC_GROUP_2, TRUE},
    {GLB_CPU_TRAFFIC_PTP,            "ptp"         , COPP_REASON_DEFAULT_VALUE_PTP,GLB_CPU_TRAFFIC_GROUP_3, TRUE},
    {GLB_CPU_TRAFFIC_TTL_ERROR,      "ttl-expired"  , COPP_REASON_DEFAULT_VALUE_TTL_FAIL,GLB_CPU_TRAFFIC_GROUP_0, TRUE},
    {GLB_CPU_TRAFFIC_L3_MTU_ERROR,   "mtu-fail"  , COPP_REASON_DEFAULT_VALUE_MTU_FAIL,GLB_CPU_TRAFFIC_GROUP_0, TRUE},
    {GLB_CPU_TRAFFIC_TELNET,         "telnet"  , COPP_REASON_DEFAULT_VALUE_TELNET,GLB_CPU_TRAFFIC_GROUP_5, TRUE},
    {GLB_CPU_TRAFFIC_SSH,            "ssh"  , COPP_REASON_DEFAULT_VALUE_SSH,GLB_CPU_TRAFFIC_GROUP_5, TRUE},   
    {GLB_CPU_TRAFFIC_MLAG,           "mlag"  ,COPP_REASON_DEFAULT_VALUE_MLAG,GLB_CPU_TRAFFIC_GROUP_4, TRUE},
    {GLB_CPU_TRAFFIC_PACKET_L2PRO_PROTOCOL_MAC,            "l2pro-protocol-mac"  , COPP_REASON_DEFAULT_VALUE_L2PRO_PROTOCOL_MAC,GLB_CPU_TRAFFIC_GROUP_4, TRUE},
    {GLB_CPU_TRAFFIC_PACKET_L2PRO_GROUP_MAC,            "l2pro-group-mac"  , COPP_REASON_DEFAULT_VALUE_L2PRO_GROUP_MAC,GLB_CPU_TRAFFIC_GROUP_4, TRUE},    
    {GLB_CPU_TRAFFIC_IPV6_NEIGHBOR_DISCOVERY,            "icmpv6"  , COPP_REASON_DEFAULT_VALUE_ND,GLB_CPU_TRAFFIC_GROUP_1, TRUE},
    
};


uint32
stm_is_ipv6_profile()
{
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis(); 
    if (NULL == p_chassis) 
    { 
        return FALSE;
    }

    type = p_chassis->cur_stm_mode;    
    if (GLB_STM_MODE_IPUCV6 == type) 
    {
        return TRUE;
    }
    
    return FALSE;
}

uint32
_cpu_traffic_reason_get_default_rate(uint32 reason_id)
{
    uint32 reason_idx = 0;
    for (reason_idx = 0; reason_idx < (sizeof(cpu_traffic_info)/sizeof(copp_reason_info_t)); reason_idx ++)
    {
        if (cpu_traffic_info[reason_idx].reason_id == reason_id)
        {
            return cpu_traffic_info[reason_idx].default_rate;
        }
    }

    return 0;
}

int32
_cpu_traffic_reason_create(uint32 reason_id, glb_cpu_traffic_group_t class_id)
{
    int32 rc = PM_E_NONE;
    tbl_cpu_traffic_t cpu_traffic;
    sal_memset(&cpu_traffic, 0x0, sizeof(cpu_traffic));

    cpu_traffic.key.reason_id = reason_id;
    PM_E_RETURN(tbl_cpu_traffic_add_cpu_traffic(&cpu_traffic));
    cpu_traffic.class_id = class_id;
    PM_E_RETURN(tbl_cpu_traffic_set_cpu_traffic_field(&cpu_traffic, TBL_CPU_TRAFFIC_FLD_CLASS_ID));
    cpu_traffic.rate = _cpu_traffic_reason_get_default_rate(reason_id);
    PM_E_RETURN(tbl_cpu_traffic_set_cpu_traffic_field(&cpu_traffic, TBL_CPU_TRAFFIC_FLD_RATE));

    return PM_E_NONE;
}

int32
cpu_traffic_id_init()
{
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_ARP, GLB_CPU_TRAFFIC_GROUP_1));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_STP, GLB_CPU_TRAFFIC_GROUP_4));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_LACP, GLB_CPU_TRAFFIC_GROUP_4));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_EAPOL, GLB_CPU_TRAFFIC_GROUP_3));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_LLDP, GLB_CPU_TRAFFIC_GROUP_3));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_IGMP, GLB_CPU_TRAFFIC_GROUP_3));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_ERPS, GLB_CPU_TRAFFIC_GROUP_4));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_DHCP, GLB_CPU_TRAFFIC_GROUP_3));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_OSPF, GLB_CPU_TRAFFIC_GROUP_4));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_PIM, GLB_CPU_TRAFFIC_GROUP_4));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_VRRP, GLB_CPU_TRAFFIC_GROUP_4));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_BGP, GLB_CPU_TRAFFIC_GROUP_4));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_MLAG, GLB_CPU_TRAFFIC_GROUP_4));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_COPY_TO_CPU, GLB_CPU_TRAFFIC_GROUP_5));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_TELNET, GLB_CPU_TRAFFIC_GROUP_5));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_SSH, GLB_CPU_TRAFFIC_GROUP_5));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_PACKET_IN, GLB_CPU_TRAFFIC_GROUP_2));
    /*for ptp add by chenc*/
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_PTP, GLB_CPU_TRAFFIC_GROUP_3));

    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_TTL_ERROR, GLB_CPU_TRAFFIC_GROUP_0));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_L3_MTU_ERROR, GLB_CPU_TRAFFIC_GROUP_0));
    /*for CPU_mirror by shengx*/
    //PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_PACKET_CPU_MIRROR, GLB_CPU_TRAFFIC_GROUP_0));

    /*for L2protocol by yejl*/
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_PACKET_L2PRO_PROTOCOL_MAC, GLB_CPU_TRAFFIC_GROUP_4));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_PACKET_L2PRO_GROUP_MAC, GLB_CPU_TRAFFIC_GROUP_4));
    PM_E_RETURN(_cpu_traffic_reason_create(GLB_CPU_TRAFFIC_IPV6_NEIGHBOR_DISCOVERY, GLB_CPU_TRAFFIC_GROUP_1));
    
    #if 0
    /*high group for protocol packet*/
    for(cpu_reason_id = GLB_CPU_TRAFFIC_STP; cpu_reason_id < GLB_CPU_TRAFFIC_ROUTER_CUSTOM_RANGE_BASE; cpu_reason_id ++)
    {
        sal_memset(&cpu_traffic, 0x0, sizeof(tbl_cpu_traffic_t));
        cpu_traffic.key.reason_id = cpu_reason_id;
        PM_E_RETURN(tbl_cpu_traffic_add_cpu_traffic(&cpu_traffic));
        cpu_traffic.class_id = GLB_CPU_TRAFFIC_GROUP_HIGH;
        PM_E_RETURN(tbl_cpu_traffic_set_cpu_traffic_field(&cpu_traffic, TBL_CPU_TRAFFIC_FLD_CLASS_ID));
    }

    /*default group for other cpu reason*/
    for(cpu_reason_id = GLB_CPU_TRAFFIC_L3_MTU_ERROR; cpu_reason_id <GLB_CPU_TRAFFIC_CUSTOM_EXCEPTION_RANGE_BASE; cpu_reason_id ++)
    {
        sal_memset(&cpu_traffic, 0x0, sizeof(tbl_cpu_traffic_t));
        cpu_traffic.key.reason_id = cpu_reason_id;
        PM_E_RETURN(tbl_cpu_traffic_add_cpu_traffic(&cpu_traffic));
        cpu_traffic.class_id = GLB_CPU_TRAFFIC_GROUP_DEFAULT;
        PM_E_RETURN(tbl_cpu_traffic_set_cpu_traffic_field(&cpu_traffic, TBL_CPU_TRAFFIC_FLD_CLASS_ID));
    }
    #endif

    return PM_E_NONE;
}

int32 
cpu_traffic_group_init()
{
    tbl_cpu_traffic_group_t cpu_traffic_group;
    int32 rc = PM_E_NONE;
    uint8 i = 0;
    /*creat cpu traffic group*/
    for (i = GLB_CPU_TRAFFIC_GROUP_0; i < GLB_CPU_TRAFFIC_GROUP_MAX; i++)
    {
        sal_memset(&cpu_traffic_group, 0x0, sizeof(cpu_traffic_group));
        cpu_traffic_group.key.class_id = i;
        PM_E_RETURN(tbl_cpu_traffic_group_add_cpu_traffic_group(&cpu_traffic_group));
    }


    return PM_E_NONE;
}

int32
cpu_traffic_group_rate_init()
{
    tbl_cpu_traffic_group_t cpu_traffic_group;
    int32 rc = PM_E_NONE;
    uint8 i = 0;

    for (i = GLB_CPU_TRAFFIC_GROUP_1; i < GLB_CPU_TRAFFIC_GROUP_MAX; i++)
    {
        sal_memset(&cpu_traffic_group, 0x0, sizeof(cpu_traffic_group));
        cpu_traffic_group.key.class_id = i;
        cpu_traffic_group.rate = GLB_CPU_TRAFFIC_DEFAULT_RATE;
        PM_E_RETURN(tbl_cpu_traffic_group_set_cpu_traffic_group_field(&cpu_traffic_group, TBL_CPU_TRAFFIC_GROUP_FLD_RATE));
    }

    sal_memset(&cpu_traffic_group, 0x0, sizeof(cpu_traffic_group));
    cpu_traffic_group.key.class_id = GLB_CPU_TRAFFIC_GROUP_0;
    cpu_traffic_group.rate = GLB_CPU_TRAFFIC_CLASS0_DEFAULT_RATE;
    PM_E_RETURN(tbl_cpu_traffic_group_set_cpu_traffic_group_field(&cpu_traffic_group, TBL_CPU_TRAFFIC_GROUP_FLD_RATE));

    return PM_E_NONE;
}

int32
cpu_traffic_group_cmd_set_field(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_cpu_traffic_group_t cpu_traffic_group;
    tbl_cpu_traffic_group_t * p_cpu_traffic_group = NULL;
    uint32 rate = 0;

    sal_memset(&cpu_traffic_group, 0x0, sizeof(cpu_traffic_group));
    PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
    if(0 == sal_strcmp(argv[0], "class") && 0 == sal_strcmp(argv[2], "rate"))
    {
        PM_CFG_GET_UINT_RANGE(cpu_traffic_group.key.class_id, 0, GLB_CPU_TRAFFIC_REASON_CLASS_MAX-1, argv[1], para->p_rs);
        PM_CFG_GET_UINT_RANGE(rate, 0, GLB_CPU_TRAFFIC_MAX_RATE, argv[3], para->p_rs);
    }
    else
    {
        CFG_INVALID_PARAM_RET("Invalid Params.");
    }
    
    p_cpu_traffic_group = tbl_cpu_traffic_group_get_cpu_traffic_group(&cpu_traffic_group.key);
    if (NULL == p_cpu_traffic_group)
    {
        CFG_INVALID_PARAM_RET("There is no class %s exist.", argv[1]);
    }
    cpu_traffic_group.rate = rate;
    PM_E_RETURN(tbl_cpu_traffic_group_set_cpu_traffic_group_field(&cpu_traffic_group, TBL_CPU_TRAFFIC_GROUP_FLD_RATE));

    return PM_E_NONE;
}

static int32
cpu_traffic_group_get_cpu_traffic_group(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_iter_args_t dump_args;
    field_parse_t field; 
    FILE *fp = NULL;
    tbl_cpu_traffic_group_key_t cpu_traffic_group_key;
    tbl_cpu_traffic_group_t * p_cpu_traffic_group;

    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    dump_args.argv[0] = &field;
    dump_args.argv[1] = fp;

    /* 1. check parameter: */
    if (argc)
    {
        sal_memset(&cpu_traffic_group_key, 0x0, sizeof(tbl_cpu_traffic_group_key_t));
        cpu_traffic_group_key.class_id = sal_atoi(argv[0]);
        p_cpu_traffic_group = tbl_cpu_traffic_group_get_cpu_traffic_group(&cpu_traffic_group_key);
        if (!p_cpu_traffic_group)
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Class %s not exist", argv[0]);
        }
        rc = tbl_cpu_traffic_group_dump_one(p_cpu_traffic_group, &dump_args);
    }
    else
    {
        rc = tbl_cpu_traffic_group_iterate((TBL_ITER_CB_FUNC)tbl_cpu_traffic_group_dump_one, &dump_args);            
    }

    sal_fclose(fp);
    return rc;
}

int32
cpu_traffic_group_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    switch (para->oper)
    {
        case CDB_OPER_ADD:
            break;

        case CDB_OPER_SET:
            cpu_traffic_group_cmd_set_field(argv, argc, para);
            break;
        case CDB_OPER_GET:
            cpu_traffic_group_get_cpu_traffic_group(argv, argc, para);
            break;
        default:
            
            break;
    }
    return PM_E_NONE;
}



int32
_cpu_traffic_reason_str_map_id(char* reason_str, uint32 *reason_id)
{
    uint8 reason_idx = 0;

    for (reason_idx = 0; reason_idx < (sizeof(cpu_traffic_info)/sizeof(copp_reason_info_t)); reason_idx++)
    {
        if (0 == sal_strcmp(cpu_traffic_info[reason_idx].reason_map_str, reason_str))
        {
            *reason_id = cpu_traffic_info[reason_idx].reason_id;
            return PM_E_NONE;
        }
    }

    return PM_E_FAIL;
}

int32
cpu_traffic_cmd_set_field(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_cpu_traffic_t cpu_traffic;
    tbl_cpu_traffic_t * p_cpu_traffic = NULL;
    uint32 rate = 0;

    sal_memset(&cpu_traffic, 0x0, sizeof(cpu_traffic));
    PM_CFG_CHECK_EXPECT_2PARAMS(argc, 3, 4, para->p_rs);
    if(0 == sal_strcmp(argv[0], "reason") && 0 == sal_strcmp(argv[2], "rate"))
    {
        rc = _cpu_traffic_reason_str_map_id(argv[1], &cpu_traffic.key.reason_id);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid copp reason %s.", argv[1]);
        }
        if (4 == argc)
        {
            PM_CFG_GET_UINT_RANGE(rate, 0, GLB_CPU_TRAFFIC_MAX_RATE, argv[3], para->p_rs);
        }
        else
        {
            rate = _cpu_traffic_reason_get_default_rate(cpu_traffic.key.reason_id);
        }
    }
    else
    {
        CFG_INVALID_PARAM_RET("Invalid Params.");
    }
    
    p_cpu_traffic = tbl_cpu_traffic_get_cpu_traffic(&cpu_traffic.key);
    if (NULL == p_cpu_traffic)
    {
        CFG_INVALID_PARAM_RET("There is no copp reason %s exist.", argv[1]);
    }
    cpu_traffic.rate = rate;
    PM_E_RETURN(tbl_cpu_traffic_set_cpu_traffic_field(&cpu_traffic, TBL_CPU_TRAFFIC_FLD_RATE));

    return PM_E_NONE;
}

static int32
cpu_traffic_get_cpu_traffic(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_iter_args_t dump_args;
    field_parse_t field; 
    FILE *fp = NULL;
    tbl_cpu_traffic_key_t cpu_traffic_key;
    tbl_cpu_traffic_t * p_cpu_traffic;

    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    dump_args.argv[0] = &field;
    dump_args.argv[1] = fp;

    /* 1. check parameter: */
    if (argc)
    {
        sal_memset(&cpu_traffic_key, 0x0, sizeof(tbl_cpu_traffic_key_t));
        rc = _cpu_traffic_reason_str_map_id(argv[1], &cpu_traffic_key.reason_id);
        if (rc < 0)
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Invalid copp reason %s.", argv[0]);
        }
        p_cpu_traffic = tbl_cpu_traffic_get_cpu_traffic(&cpu_traffic_key);
        if (!p_cpu_traffic)
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Reason %s not exist", argv[0]);
        }
        rc = tbl_cpu_traffic_dump_one(p_cpu_traffic, &dump_args);
    }
    else
    {
        rc = tbl_cpu_traffic_iterate((TBL_ITER_CB_FUNC)tbl_cpu_traffic_dump_one, &dump_args);            
    }

    sal_fclose(fp);
    return rc;
}

int32
cpu_traffic_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    switch (para->oper)
    {
        case CDB_OPER_ADD:
            break;

        case CDB_OPER_SET:
            cpu_traffic_cmd_set_field(argv, argc, para);
            break;
        case CDB_OPER_GET:
            cpu_traffic_get_cpu_traffic(argv, argc, para);
            break;
        default:
            
            break;
    }
    return PM_E_NONE;
}

copp_reason_info_t*
cpu_traffic_get_reason_info(uint32 reason_id)
{
    uint32 reason_idx = 0;
    for (reason_idx = 0; reason_idx < (sizeof(cpu_traffic_info)/sizeof(copp_reason_info_t)); reason_idx ++)
    {
        if (cpu_traffic_info[reason_idx].reason_id == reason_id)
        {
            return &cpu_traffic_info[reason_idx];
        }
    }

    return NULL;
}

int32
_cpu_traffic_cmd_process_show_reason(cfg_cmd_para_t *para, FILE *fp)
{
    tbl_cpu_traffic_master_t *p_master = tbl_cpu_traffic_get_master();
    tbl_cpu_traffic_t *p_db_rsn = NULL;
    uint32 copp_class = 0;
    uint32 copp_reason = 0;
    copp_reason_info_t *p_rsn_info = NULL;

    sal_fprintf(fp, "Rate unit (kbps)\n");
    sal_fprintf(fp, "%-20s %-10s %-10s %-12s\n", "Reason", "Class", "Rate", "Default Rate");
    sal_fprintf(fp, "--------------------+----------+----------+------------\n");
    
    for (copp_class = 0; copp_class <= 5; copp_class++)
    {
        for (copp_reason = 0; copp_reason < GLB_CPU_TRAFFIC_MAX; copp_reason++)
        {
            p_db_rsn = p_master->array[copp_reason];
            if (NULL == p_db_rsn)
            {
                continue;
            }
            if (copp_class != p_db_rsn->class_id)
            {
                continue;
            }
            p_rsn_info = cpu_traffic_get_reason_info(copp_reason);
            if (NULL == p_rsn_info)
            {
                continue;
            }
            if (GLB_CPU_TRAFFIC_PIM == p_rsn_info->reason_id ||
                GLB_CPU_TRAFFIC_BGP == p_rsn_info->reason_id)
            {
                continue;
            }
            sal_fprintf(fp, "%-20s %-10u %-10"PRIu64" %-12u\n", 
                p_rsn_info->reason_map_str, p_db_rsn->class_id, p_db_rsn->rate, p_rsn_info->default_rate);
        }
    }

    return PM_E_NONE;
}

int32
cpu_traffic_cmd_process_show_reason(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE    *fp = NULL;
     
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    _cpu_traffic_cmd_process_show_reason(para, fp);
    
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

int32
cpu_traffic_mgr_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_CPU_TRAFFIC_GROUP, cpu_traffic_group_cmd_process);

    cdb_register_cfg_tbl_cb(TBL_CPU_TRAFFIC, cpu_traffic_cmd_process);

    cdb_register_cfg_act_cb(ACT_SHOW_COPP_REASON, cpu_traffic_cmd_process_show_reason);
    
    return PM_E_NONE;
}

int32
cpu_traffic_init(void)
{
    int32 rc = PM_E_NONE;
    PM_E_RETURN(cpu_traffic_group_init());
    PM_E_RETURN(cpu_traffic_id_init());
    PM_E_RETURN(cpu_traffic_group_rate_init());

    return PM_E_NONE;
}
