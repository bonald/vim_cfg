#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_lag_global_define.h"
#include "gen/tbl_lag_global.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "switch.h"
#include "lacp_define.h"
#include "lacp_pdu.h"
#include "lacp.h"

static int32
_lacp_cmd_get_lag_global(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_lag_global_t* p_lag_global = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_lag_global = tbl_lag_global_get_lag_global();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_lag_global_dump_one(p_lag_global, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}
static int32
_lacp_cmd_set_hash_arithmetic(uint8 hash_arithmetic_type, tbl_lag_global_t* p_lag_global, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    /*1. check duplicate config*/
    if(hash_arithmetic_type == p_lag_global->hash_arithmetic)
    {
        return PM_E_NONE;
    }
    
    /*2. call API*/
    PM_E_RETURN(lacp_set_hash_arithmetic(hash_arithmetic_type));
    return rc;
}

static int32
_lacp_cmd_set_load_balance_inner_field_en(uint32 load_balance_inner_field_en, tbl_lag_global_t* p_lag_global, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    /*1. check duplicate config*/
    if(load_balance_inner_field_en == p_lag_global->load_balance_inner_field_en)
    {
        return PM_E_NONE;
    }
    
    /*2. call API*/
    PM_E_RETURN(lacp_set_load_balance_inner_field_en(load_balance_inner_field_en));
    return rc;
}

static int32
_lacp_cmd_check_load_balance_mode(uint8 load_balance_mode, tbl_lag_global_t* p_lag_global, cfg_cmd_para_t *para)
{
    tbl_chip_t *p_db_chip = tbl_chip_get_chip();
    uint32 dlb_count = 0;
    uint32 rr_count = 0;
    uint32 i = 0;

    
    if (GLB_LAG_LOAD_BALANCE_MODE_STATIC == load_balance_mode)
    {
        return PM_E_NONE;
    }
    
    for (i = 0; i < GLB_LAG_GROUP_NUM+1; i++)
    {
        if (GLB_LAG_LOAD_BALANCE_DLB == p_lag_global->load_balance_mode[i])
        {
            dlb_count++;
        }
        if (GLB_LAG_LOAD_BALANCE_RR == p_lag_global->load_balance_mode[i])
        {
            rr_count++;
        }        
    }
    
    if (GLB_CHIP_GREATEBELT == p_db_chip->type)
    {
        /* GB uses CTC_LINKAGG_MODE_64: DLB N/A, RR [0-15] */
        if (GLB_LAG_LOAD_BALANCE_DLB == load_balance_mode)
        {
            CFG_CONFLICT_RET("DLB not support on %s", cdb_enum_val2str(glb_chip_type_strs, GLB_CHIP_TYPE_MAX, p_db_chip->type));
        }
        else if (GLB_LAG_LOAD_BALANCE_RR == load_balance_mode)
        {
            if (rr_count >= 16)
            {
                CFG_CONFLICT_RET("LAG round-robin reach to 16 limitations on %s", cdb_enum_val2str(glb_chip_type_strs, GLB_CHIP_TYPE_MAX, p_db_chip->type));
            }
        }
    }
    else if (GLB_CHIP_GOLDENGATE == p_db_chip->type || GLB_CHIP_DUET2 == p_db_chip->type)
    {
        /* GG uses CTC_LINKAGG_MODE_56: DLB [0-7], RR [0-15] */
        if (GLB_LAG_LOAD_BALANCE_DLB == load_balance_mode)
        {
            if (dlb_count >= 8)
            {
                CFG_CONFLICT_RET("LAG DLB reach to 8 limitations on %s", cdb_enum_val2str(glb_chip_type_strs, GLB_CHIP_TYPE_MAX, p_db_chip->type));
            }
        }
        else if (GLB_LAG_LOAD_BALANCE_RR == load_balance_mode)
        {
            if (rr_count >= 16)
            {
                CFG_CONFLICT_RET("LAG round-robin reach to 16 limitations on %s", cdb_enum_val2str(glb_chip_type_strs, GLB_CHIP_TYPE_MAX, p_db_chip->type));
            }
        }

        if (dlb_count + rr_count >= 16)
        {
            CFG_CONFLICT_RET("LAG DLB %u and round-robin %u, reach to 16 limitations on %s", dlb_count, rr_count, cdb_enum_val2str(glb_chip_type_strs, GLB_CHIP_TYPE_MAX, p_db_chip->type));
        }
    }
    else if (GLB_CHIP_TSINGMA == p_db_chip->type)
    {
        /* TM uses CTC_LINKAGG_MODE_56: DLB [0-7], RR [0-15] */
        if (GLB_LAG_LOAD_BALANCE_DLB == load_balance_mode)
        {
            if (dlb_count >= 8)
            {
                CFG_CONFLICT_RET("LAG DLB reach to 8 limitations on %s", cdb_enum_val2str(glb_chip_type_strs, GLB_CHIP_TYPE_MAX, p_db_chip->type));
            }
        }
        else if (GLB_LAG_LOAD_BALANCE_RR == load_balance_mode)
        {
            if (rr_count >= 16)
            {
                CFG_CONFLICT_RET("LAG round-robin reach to 16 limitations on %s", cdb_enum_val2str(glb_chip_type_strs, GLB_CHIP_TYPE_MAX, p_db_chip->type));
            }
        }

        if (dlb_count + rr_count >= 16)
        {
            CFG_CONFLICT_RET("LAG DLB %u and round-robin %u, reach to 16 limitations on %s", dlb_count, rr_count, cdb_enum_val2str(glb_chip_type_strs, GLB_CHIP_TYPE_MAX, p_db_chip->type));
        }
    }

    return PM_E_NONE;
}

static int32
_lacp_cmd_set_load_balance_mode(uint8 channel_group_id, uint8 load_balance_mode, tbl_lag_global_t* p_lag_global, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_key_t if_key;
    tbl_interface_t* p_db_if = NULL;

    if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
    {
        CFG_CONFLICT_RET("Not supported on this product");
    }
        
    /* 1. check conflict config */
    sal_snprintf(if_key.name, IFNAME_SIZE, GLB_IFNAME_AGG_PREFIX"%u", channel_group_id);
    p_db_if = tbl_interface_get_interface_by_name(if_key.name);
    if (NULL != p_db_if)
    {
        CFG_CONFLICT_RET("Can't config, %s have existed", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    
    /* 2. check duplicate config */
    if (load_balance_mode == p_lag_global->load_balance_mode[channel_group_id])
    {
        return PM_E_NONE;
    }

    PM_E_RETURN(_lacp_cmd_check_load_balance_mode(load_balance_mode, p_lag_global, para));
   
    /* 3. call API */
    PM_E_RETURN(lacp_set_load_balance_mode(channel_group_id,load_balance_mode));
    return rc;
}


static int32
_lacp_cmd_set_system_priority(uint16 system_priority, tbl_lag_global_t* p_lag_global, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (system_priority == p_lag_global->system_priority)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(lacp_set_system_priority(system_priority));
    return rc;
}

static int32
_lacp_cmd_overwrite_load_balance(uint32 load_balance_flag, tbl_lag_global_t* p_lag_global, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(lacp_set_load_balance(load_balance_flag));

    return rc;
}

static int32
_lacp_cmd_set_load_balance(uint32 load_balance_flag, tbl_lag_global_t* p_lag_global, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. check */
    if (GLB_AGG_LOAD_BALANCE_CTC_DEFAULT == load_balance_flag)
    {
        p_lag_global->load_balance = load_balance_flag;
        PM_E_RETURN(lacp_set_load_balance(p_lag_global->load_balance));
        return PM_E_NONE;        
    }   

    if(GLB_FLAG_ISSET(p_lag_global->load_balance, load_balance_flag))
    {
        return PM_E_NONE;
    }

    /* 2. process and check of parameter */
    GLB_SET_FLAG(p_lag_global->load_balance, load_balance_flag);

    /* 3. call API */
    PM_E_RETURN(lacp_set_load_balance(p_lag_global->load_balance));

    return rc;
}

static int32
_lacp_cmd_unset_load_balance(uint32 load_balance_flag, tbl_lag_global_t* p_lag_global, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */
    if (!GLB_FLAG_ISSET(p_lag_global->load_balance, load_balance_flag))
    {
        return PM_E_NONE;
    }

    /* 2. process and check of parameter */
    GLB_UNSET_FLAG(p_lag_global->load_balance, load_balance_flag);

    /* 3. call API */
    PM_E_RETURN(lacp_set_load_balance(p_lag_global->load_balance));
    return rc;
}

int32
lacp_cmd_process_lag_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    tbl_lag_global_t* p_lag_global = NULL;
    cdb_node_t* p_node = NULL;
    int32 value = 0;
    uint8 channel_group_id=0;
    int32 value_default = 0;
    uint32 parse_field = FALSE;
    uint32 i = 0;
    int32 rc = PM_E_NONE;
    unsigned char argc_idx = 0;
    uint32 chip_type = 0;
    uint32 is_overwrite = FALSE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_LAG_GLOBAL);
    p_lag_global = tbl_lag_global_get_lag_global();

    switch (para->oper)
    {
    case CDB_OPER_SET:
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }
        
        switch (field_id)
        {
        case TBL_LAG_GLOBAL_FLD_SYSTEM_PRIORITY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            rc = _lacp_cmd_set_system_priority(value, p_lag_global, para);
            break;
            
        case TBL_LAG_GLOBAL_FLD_HASH_ARITHMETIC:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            if (0 == sal_strcmp(argv[1], "xor"))
            {
                value = GLB_LAG_HASH_ALGO_XOR;   
            }
            else if (0 == sal_strcmp(argv[1], "crc"))
            {
                value = GLB_LAG_HASH_ALGO_CRC;   
            }
            rc = _lacp_cmd_set_hash_arithmetic(value, p_lag_global, para);
            break;
        case TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_INNER_FIELD_EN:
            if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
            {
                CFG_INVALID_PARAM_RET("This function not support");
            }
            argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("inner_field_en");
            if(argc_idx != 0xFF){
                if(0 == sal_strcmp(argv[argc_idx + 1], "both")){
                    rc = _lacp_cmd_set_load_balance_inner_field_en(GLB_LAG_TUNNEL_HASH_MODE_BOTH, p_lag_global, para);
                }else if(0 == sal_strcmp(argv[argc_idx + 1], "outer")){
                    rc = _lacp_cmd_set_load_balance_inner_field_en(GLB_LAG_TUNNEL_HASH_MODE_OUTER, p_lag_global, para);
                }else{
                    rc = _lacp_cmd_set_load_balance_inner_field_en(GLB_LAG_TUNNEL_HASH_MODE_INNER, p_lag_global, para);
				}
            }
            break;
            
        case TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_MODE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(channel_group_id, argv[1], para->p_rs);
            if (channel_group_id > GLB_LAG_GROUP_NUM)
            {
                CFG_CONFLICT_RET("Max LAG group id should be less than %d", GLB_LAG_GROUP_NUM+1);
            }
            if (0 == sal_strcmp(argv[2], "dynamic"))
            {
                value = GLB_LAG_LOAD_BALANCE_DLB;   
            }
            else if (0 == sal_strcmp(argv[2], "round-robin"))
            {
                value = GLB_LAG_LOAD_BALANCE_RR;   
            }
            else if (0 == sal_strcmp(argv[2], "static"))
            {
                value = GLB_LAG_LOAD_BALANCE_MODE_STATIC;   
            }
            else if (0 == sal_strcmp(argv[2], "resilient"))
            {
                value = GLB_LAG_LOAD_BALANCE_RESILIENT;   
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid load-banance mode %s", argv[2]);
            }
            rc = _lacp_cmd_set_load_balance_mode(channel_group_id, value, p_lag_global, para);
            break;
            
        case TBL_LAG_GLOBAL_FLD_LOAD_BALANCE:
            chip_type = tbl_chip_get_type();
            if (!sal_strcmp(argv[1], "overwrite"))
            {
                is_overwrite = TRUE;
            }
            if (0 == sal_strcmp(argv[2], "default"))
            {
                if (GLB_CHIP_GREATEBELT == chip_type)
                {
                    is_overwrite = TRUE;
                }
                else if (GLB_CHIP_GOLDENGATE == chip_type)
                {
                    is_overwrite = FALSE;
                }
            }
            if (is_overwrite)
            {
                if (GLB_CHIP_GREATEBELT != chip_type)
                {
                    CFG_UNSUPPORT_PRODUCT_RET();
                }
                
                /* for GB */
                if (0 == sal_strcmp(argv[2], "dst-ip"))
                {
                    value = GLB_LAG_LOAD_BALANCE_DST_IP;
                }
                else if (0 == sal_strcmp(argv[2], "dst-mac"))
                {
                    value = GLB_LAG_LOAD_BALANCE_DST_MAC;
                }
                else if (0 == sal_strcmp(argv[2], "dst-port"))
                {
                    value = GLB_LAG_LOAD_BALANCE_DST_PORT;
                }
                else if (0 == sal_strcmp(argv[2], "src-dst-ip"))
                {
                    value = GLB_LAG_LOAD_BALANCE_SRC_IP | GLB_LAG_LOAD_BALANCE_DST_IP;
                }
                else if (0 == sal_strcmp(argv[2], "src-dst-mac"))
                {
                     value = GLB_LAG_LOAD_BALANCE_SRC_MAC | GLB_LAG_LOAD_BALANCE_DST_MAC;
                }
                else if (0 == sal_strcmp(argv[2], "src-dst-port"))
                {
                    value = GLB_LAG_LOAD_BALANCE_SRC_PORT | GLB_LAG_LOAD_BALANCE_DST_PORT;
                }
                else if (0 == sal_strcmp(argv[2], "src-dst-ip-src-dst-port"))
                {
                    value = GLB_LAG_LOAD_BALANCE_SRC_IP | GLB_LAG_LOAD_BALANCE_DST_IP |
                            GLB_LAG_LOAD_BALANCE_SRC_PORT | GLB_LAG_LOAD_BALANCE_DST_PORT;
                }
                else if (0 == sal_strcmp(argv[2], "src-ip"))
                {
                    value = GLB_LAG_LOAD_BALANCE_SRC_IP;
                }
                else if (0 == sal_strcmp(argv[2], "src-mac"))
                {
                    value = GLB_LAG_LOAD_BALANCE_SRC_MAC;
                }
                else if (0 == sal_strcmp(argv[2], "src-port"))
                {
                    value = GLB_LAG_LOAD_BALANCE_SRC_PORT;
                }
                else if (0 == sal_strcmp(argv[2], "default"))
                {
                    value = GLB_AGG_LOAD_BALANCE_CTC_DEFAULT;
                }
                rc = _lacp_cmd_overwrite_load_balance(value, p_lag_global, para);
            }
            else
            {
                if (GLB_CHIP_GREATEBELT == chip_type)
                {
                    CFG_UNSUPPORT_PRODUCT_RET();
                }
                
                /* for GG/Duet2 */            
                if (0 == sal_strcmp(argv[2], "default"))
                {
                    if(!sal_strcmp(argv[1], "set"))
                    {
                        PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                        value_default = GLB_AGG_LOAD_BALANCE_CTC_DEFAULT;
                        rc = _lacp_cmd_set_load_balance(value_default, p_lag_global, para);
                        return PM_E_NONE;
                    }
                }
                
                if (0 == sal_strcmp(argv[2], "src-mac"))
                {
                    value = GLB_LAG_LOAD_BALANCE_SRC_MAC;
                }
                
                if (0 == sal_strcmp(argv[2], "dst-mac"))
                {
                    value = GLB_LAG_LOAD_BALANCE_DST_MAC;
                }
                
                if (0 == sal_strcmp(argv[2], "src-ip"))
                {
                    value = GLB_LAG_LOAD_BALANCE_SRC_IP;
                }
                
                if (0 == sal_strcmp(argv[2], "dst-ip"))
                {
                    value = GLB_LAG_LOAD_BALANCE_DST_IP;
                }
                
                if (0 == sal_strcmp(argv[2], "ip-protocol"))
                {
                    value = GLB_LAG_LOAD_BALANCE_IP_PROTOCOL;
                }
                
                if (0 == sal_strcmp(argv[2], "src-port-l4"))
                {
                    value = GLB_LAG_LOAD_BALANCE_SRC_PORT;
                }

                if (0 == sal_strcmp(argv[2], "dst-port-l4"))
                {
                    value = GLB_LAG_LOAD_BALANCE_DST_PORT;
                }

                if (0 == sal_strcmp(argv[2], "vxlan-vni"))
                {
                    if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
                    {
                        CFG_INVALID_PARAM_RET("This function not support");
                    }
                    value = GLB_LAG_LOAD_BALANCE_VXLAN_VNI;
                }

                if (0 == sal_strcmp(argv[2], "gre-key"))
                {
                    if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
                    {
                        CFG_INVALID_PARAM_RET("This function not support");
                    }
                    value = GLB_LAG_LOAD_BALANCE_GRE_KEY;
                }

                if (0 == sal_strcmp(argv[2], "nvgre-vsid"))
                {
                    if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
                    {
                        CFG_INVALID_PARAM_RET("This function not support");
                    }
                    value = GLB_LAG_LOAD_BALANCE_NVGRE_VNI;
                }

                if (0 == sal_strcmp(argv[2], "nvgre-flow-id"))
                {
                    if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
                    {
                        CFG_INVALID_PARAM_RET("This function not support");
                    }
                    value = GLB_LAG_LOAD_BALANCE_NVGRE_FLOW_ID;
                }

                if (0 == sal_strcmp(argv[2], "inner-dst-mac"))
                {
                    if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
                    {
                        CFG_INVALID_PARAM_RET("This function not support");
                    }
                    value = GLB_LAG_LOAD_BALANCE_INNER_DST_MAC;
                }

                if (0 == sal_strcmp(argv[2], "inner-src-mac"))
                {
                    if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
                    {
                        CFG_INVALID_PARAM_RET("This function not support");
                    }
                    value = GLB_LAG_LOAD_BALANCE_INNER_SRC_MAC;
                }

                if (0 == sal_strcmp(argv[2], "inner-src-ip"))
                {
                    if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
                    {
                        CFG_INVALID_PARAM_RET("This function not support");
                    }
                    value = GLB_LAG_LOAD_BALANCE_INNER_SRC_IP;
                }

                if (0 == sal_strcmp(argv[2], "inner-dst-ip"))
                {
                    if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
                    {
                        CFG_INVALID_PARAM_RET("This function not support");
                    }
                    value = GLB_LAG_LOAD_BALANCE_INNER_DST_IP;
                }
                
                if (0 == sal_strcmp(argv[2], "inner-ip-protocol"))
                {
                    if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
                    {
                        CFG_INVALID_PARAM_RET("This function not support");
                    }
                    value = GLB_LAG_LOAD_BALANCE_INNER_IP_PROTOCOL;
                }
                
                if (0 == sal_strcmp(argv[2], "inner-src-port-l4"))
                {
                    if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
                    {
                        CFG_INVALID_PARAM_RET("This function not support");
                    }
                    value = GLB_LAG_LOAD_BALANCE_INNER_SRC_PORT_L4;
                }
                
                if (0 == sal_strcmp(argv[2], "inner-dst-port-l4"))
                {
                    if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
                    {
                        CFG_INVALID_PARAM_RET("This function not support");
                    }
                    value = GLB_LAG_LOAD_BALANCE_INNER_DST_PORT_L4;
                }
                
                if(!sal_strcmp(argv[1], "set"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                    rc = _lacp_cmd_set_load_balance(value, p_lag_global, para);
                }
                if(!sal_strcmp(argv[1], "unset"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);                
                    rc = _lacp_cmd_unset_load_balance(value, p_lag_global, para);
                }
            }

            break;
            
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        
        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            if (!sal_strcmp(argv[0], "sysid"))
            {
                parse_field = FALSE;
            }
        }

        if (1 == argc)
        {
            if (!sal_strcmp(argv[0], "load-balance"))
            {
                parse_field = FALSE;
            }
        }
        
        if (parse_field)
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }
        else
        {
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        }
        rc = _lacp_cmd_get_lag_global(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
lacp_cmd_process_clear_counters(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 lag_id = 0;
    int32 rc = PM_E_NONE;
    
    if (argc > 0)
    {
        lag_id = cdb_int_str2val(argv[0], &rc);
        if (rc < 0)
        {
            return PM_E_NONE;
        }
    }
    
    return lacp_clear_counters(lag_id);
}

