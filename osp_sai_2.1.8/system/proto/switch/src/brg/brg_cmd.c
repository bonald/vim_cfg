
#include "proto.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_fdb.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_mcfdb.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_macfilter.h"
#include "gen/tbl_psfdb_define.h"
#include "gen/tbl_psfdb.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
#include "gen/tbl_tap_group_define.h"
#include "gen/tbl_tap_group.h"
#include "gen/tbl_tap_group_ingress_define.h"
#include "gen/tbl_tap_group_ingress.h"
#include "gen/tbl_tap_group_ingress_flow_define.h"
#include "gen/tbl_tap_group_ingress_flow.h"
#include "gen/tbl_tap_group_egress_define.h"
#include "gen/tbl_tap_group_egress.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "switch.h"
#include "brg.h"
#include "brg_fdb.h"
#ifdef TAPPRODUCT
#include "tap.h"
#endif

static int32
_brg_global_cmd_set_aging_time(uint32 aging_time, tbl_brg_global_t* p_brg_global, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (aging_time == p_brg_global->fdb_aging_time)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_global_set_aging_time(aging_time));
    return rc;
}

static int32
_brg_global_cmd_set_hw_learning_en(uint32 enable, tbl_brg_global_t* p_brg_global, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (enable == p_brg_global->hw_learning_enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_global_set_hw_learning_en(enable));
    return rc;
}

static int32
_brg_global_cmd_set_ipsg_max_port_binding(uint32 num, tbl_brg_global_t* p_brg_global, cfg_cmd_para_t *para)
{
    tbl_interface_master_t  *p_master   = tbl_interface_get_master();
    ctclib_slistnode_t      *listnode   = NULL;    
    tbl_interface_t         *p_db_if    = NULL;
    int32                   rc          = PM_E_NONE;

    /* 1. config check */
    /* 2. check duplicate config */
    if (num == p_brg_global->ipsg_max_port_binding)
    {
        return PM_E_NONE;
    }

    if (0 == num)
    {
        return brg_global_set_ipsg_max_port_binding(num);
    }
    
    if (NULL == p_master)
    {
        return PM_E_FAIL;
    }

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (p_db_if->ip_source_item_number > num)
        {
            CFG_INVALID_PARAM_RET("The ip source binding item on port %s reach number: %d",
                p_db_if->key.name, p_db_if->ip_source_item_number);
        }
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_global_set_ipsg_max_port_binding(num));
    return rc;
}

static int32
_brg_global_cmd_set_port_isolate_mode(uint32 mode, tbl_brg_global_t* p_brg_global, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (mode == p_brg_global->port_isolate_mode)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_global_set_port_isolate_mode(mode));
    return rc;
}

static int32
_brg_global_cmd_set_instance(uint32 is_add, uint32 id, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (is_add)
    {
        rc = brg_alloc_id(CDB_ID_INSTANCE);
        if (rc < 0)
        {
            CFG_CONFLICT_RET("No free instance");
        }
    }
    else
    {
        rc = brg_release_id(CDB_ID_INSTANCE, id);
        if (rc < 0)
        {
            CFG_CONFLICT_RET("Instance %d is not exist", id);
        }
    }

    return rc;
}

static int32
_brg_global_cmd_set_trap_delay_linkup_time(uint32 linkup_time, tbl_brg_global_t* p_brg_global, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (linkup_time == p_brg_global->trap_delay_up)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_global_set_trap_delay_linkup_time(linkup_time));
    
    return rc;
}


static int32
_brg_global_cmd_set_trap_delay_linkdown_time(uint32 linkdown_time, tbl_brg_global_t* p_brg_global, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (linkdown_time == p_brg_global->trap_delay_down)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_global_set_trap_delay_linkdown_time(linkdown_time));
    
    return rc;
}

static int32
_brg_global_cmd_set_l2protocol_cos(uint32 l2protocol_cos, tbl_brg_global_t* p_brg_global, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (l2protocol_cos == p_brg_global->l2protocol_cos)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_global_set_l2protocol_cos(l2protocol_cos));
    
    return rc;
}

static int32
_brg_global_cmd_get_brg_global(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_brg_global_t* p_brg_glb = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_brg_glb = tbl_brg_global_get_brg_global();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_brg_global_dump_one(p_brg_glb, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
brg_cmd_process_brg_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    tbl_brg_global_t* p_brg_global = NULL;
    cdb_node_t* p_node = NULL;
    uint32 value = 0;
    uint32 i = 0;
    uint32 is_add = TRUE;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_BRG_GLOBAL);
    p_brg_global = tbl_brg_global_get_brg_global();

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
        case TBL_BRG_GLOBAL_FLD_FDB_AGING_TIME:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            rc = _brg_global_cmd_set_aging_time(value, p_brg_global, para);
            break;
        case TBL_BRG_GLOBAL_FLD_HW_LEARNING_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[1], para->p_rs);
            rc = _brg_global_cmd_set_hw_learning_en(value, p_brg_global, para);
            break;
        case TBL_BRG_GLOBAL_FLD_IPSG_MAX_PORT_BINDING:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _brg_global_cmd_set_ipsg_max_port_binding(value, p_brg_global, para);
            break;
        case TBL_BRG_GLOBAL_FLD_PORT_ISOLATE_MODE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_ENUM(value, glb_port_isolate_strs, GLB_PORT_ISOLATE_MAX, argv[1], para->p_rs);
            rc = _brg_global_cmd_set_port_isolate_mode(value, p_brg_global, para);
            break;
        case TBL_BRG_GLOBAL_FLD_INSTANCE:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 2, 3, para->p_rs);
            if (0 == sal_strcmp("add", argv[1]))
            {
                is_add = TRUE;
            }
            else if (0 == sal_strcmp("delete", argv[1]))
            {
                is_add = FALSE;
                if (3 != argc)
                {
                    CFG_INVALID_PARAM_RET("args number should be %d", 3);
                }
                else
                {
                    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                }
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid action %s", argv[1]);
            }
            rc = _brg_global_cmd_set_instance(is_add, value, para);
            break;
        case TBL_BRG_GLOBAL_FLD_TRAP_DELAY_UP:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            rc = _brg_global_cmd_set_trap_delay_linkup_time(value, p_brg_global, para);
            break;
        case TBL_BRG_GLOBAL_FLD_TRAP_DELAY_DOWN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            rc = _brg_global_cmd_set_trap_delay_linkdown_time(value, p_brg_global, para);
            break;  
        case TBL_BRG_GLOBAL_FLD_L2PROTOCOL_COS:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            rc = _brg_global_cmd_set_l2protocol_cos(value, p_brg_global, para);
            break;
            
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _brg_global_cmd_get_brg_global(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;

}

