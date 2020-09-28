
#include "proto.h"
#include "ptp_datatype.h"
#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"
#include "gen/tbl_ptp_foreign_define.h"
#include "gen/tbl_ptp_foreign.h"
#include "gen/tbl_ptp_debug_define.h"
#include "gen/tbl_ptp_debug.h"
#include "gen/tbl_ptp_residence_time_cache_define.h"
#include "gen/tbl_ptp_residence_time_cache.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"

#include "ptp.h"
#include "ptp_api.h"
#include "ptp_desc.h"
#include "ptp_error.h"
#include "ptp_ops.h"
#include "ptp_constant.h"
#include "ptp_util.h"
#include "ptp_adpt.h"

#ifndef ptp_global_cfg
static int32
_ptp_cmd_set_time_prop_intern_time_source(uint8 time_source, tbl_ptp_global_t* p_ptp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    switch (time_source)
    {
    case PTP_TIME_SOURCE_ATOMIC_CLOCK:
    case PTP_TIME_SOURCE_GPS:
    case PTP_TIME_SOURCE_TERRESTRIAL_RADIO:
    case PTP_TIME_SOURCE_PTP:
    case PTP_TIME_SOURCE_NTP:
    case PTP_TIME_SOURCE_HAND_SET:
    case PTP_TIME_SOURCE_OTHER:
    case PTP_TIME_SOURCE_INTERNAL_OSCILLATOR:
        break;
    default:
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (time_source == p_ptp_glb->time_prop_intern.time_source)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_time_prop_intern_time_source(time_source));

    return rc;
}

static int32
_ptp_cmd_set_time_prop_intern_leap61(uint8 leap61, tbl_ptp_global_t* p_ptp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }
    
    /* 2. check duplicate config */
    if (leap61 == p_ptp_glb->time_prop_intern.leap61)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_time_prop_intern_leap61(leap61));

    return rc;
}

static int32
_ptp_cmd_set_time_prop_intern_leap59(uint8 leap59, tbl_ptp_global_t* p_ptp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }
    
    /* 2. check duplicate config */
    if (leap59 == p_ptp_glb->time_prop_intern.leap59)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_time_prop_intern_leap59(leap59));

    return rc;
}

static int32
_ptp_cmd_set_time_prop_intern_current_utc_offset(int16 current_utc_offset, tbl_ptp_global_t* p_ptp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    if (current_utc_offset < 0 || current_utc_offset > 100)
    {
        //CFG_INVALID_PARAM_RET("Invalid utc offset %hu", current_utc_offset);
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (current_utc_offset == p_ptp_glb->time_prop_intern.current_utc_offset)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_time_prop_intern_current_utc_offset(current_utc_offset));

    return rc;
}

static int32
_ptp_cmd_set_default_ds_clk_class(uint8 clk_class, tbl_ptp_global_t* p_ptp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();

    /* 1. config check */
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    switch (clk_class)
    {
    case 6:
    case 7:
    case 13:
    case 14:
    case 52:
    case 58:
    case 187:
    case 193:
    case 248:
        break;

    default:
        //CFG_INVALID_PARAM_RET("Invalid clock class %hhu", clk_class);
        return PTP_E_API_INVALID_CMD_INPUT;
    }

    if (g_p_master->default_ds.slave_only)
    {
        return PTP_E_API_CANNOT_OP_ON_SLAVE_ONLY;
    }
    
    /* 2. check duplicate config */
    if (clk_class == p_ptp_glb->default_ds.clock_quality.clk_class)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_default_ds_clk_class(clk_class));

    return rc;
}

static int32
_ptp_cmd_set_default_ds_clk_accuracy(uint8 clk_accuracy, tbl_ptp_global_t* p_ptp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    if (!(clk_accuracy >= 0x20 && clk_accuracy <= 0x31) && !(0xfe == clk_accuracy))
    {
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (clk_accuracy == p_ptp_glb->default_ds.clock_quality.clk_accuracy)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_default_ds_clk_accuracy(clk_accuracy));

    return rc;
}
static int32
_ptp_cmd_set_default_ds_priority2(uint8 priority2, tbl_ptp_global_t* p_ptp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    if (priority2 < 0 || priority2 > 255)
    {
        //CFG_INVALID_PARAM_RET("Invalid priority2 %hhu", priority2);
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (priority2 == p_ptp_glb->default_ds.priority2)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_default_ds_priority2(priority2));

    return rc;
}

static int32
_ptp_cmd_set_default_ds_priority1(uint8 priority1, tbl_ptp_global_t* p_ptp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    if (priority1 < 0 || priority1 > 255)
    {
        //CFG_INVALID_PARAM_RET("Invalid priority1 %hhu", priority1);
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (priority1 == p_ptp_glb->default_ds.priority1)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_default_ds_priority1(priority1));

    return rc;
}

static int32
_ptp_cmd_set_default_ds_primary_domain(uint8 primary_domain, tbl_ptp_global_t* p_ptp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (!ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    if (primary_domain < 0 || primary_domain > 255)
    {
        //CFG_INVALID_PARAM_RET("Invalid domain id %hhu", primary_domain);
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (primary_domain == p_ptp_glb->default_ds.primary_domain)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_default_ds_primary_domain(primary_domain));

    return rc;
}

static int32
_ptp_cmd_set_default_ds_slave_only(uint8 slave_only, tbl_ptp_global_t* p_ptp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (!ptp_ops_is_oc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }
    
    /* 2. check duplicate config */
    if (slave_only == p_ptp_glb->default_ds.slave_only)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_default_ds_slave_only(slave_only));

    return rc;
}

static int32
_ptp_cmd_set_default_ds_domain_number(uint8 domain_number, tbl_ptp_global_t* p_ptp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    if (domain_number < 0 || domain_number > 255)
    {
        //CFG_INVALID_PARAM_RET("Invalid domain id %hhu", domain_number);
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (domain_number == p_ptp_glb->default_ds.domain_number)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_default_ds_domain_number(domain_number));

    return rc;
}

static int32
_ptp_cmd_set_default_ds_two_step_flag(uint32 two_step_flag, tbl_ptp_global_t* p_ptp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (two_step_flag == p_ptp_glb->default_ds.two_step_flag)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
   PM_E_RETURN(ptp_api_set_default_ds_two_step_flag(two_step_flag));

    return PTP_E_API_CLEAR_CONFIGURATION;
}

static int32
_ptp_cmd_set_system_time(uint32 set_system_time, tbl_ptp_global_t* p_ptp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }
    
    /* 2. check duplicate config */
    if (set_system_time == p_ptp_glb->set_system_time)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_system_time(set_system_time));

    return rc;
}

static int32
_ptp_cmd_set_device_type(uint8 device_type, tbl_ptp_global_t* p_ptp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (device_type < PTP_DEVICE_OC || device_type > PTP_DEVICE_E2ETC)
    {
        //CFG_INVALID_PARAM_RET("Invalid device type");
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (device_type == p_ptp_glb->device_type)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    rc = ptp_api_set_device_type(device_type);

    return rc;
}

static int32
_ptp_cmd_set_global_enable(uint32 global_enable, tbl_ptp_global_t* p_ptp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (global_enable == p_ptp_glb->global_enable)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_global_enable(global_enable));

    return rc;
}

int32 _ptp_cmd_set_clock_id(int32 argc, char **argv, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;
    glb_ptp_clock_id_t clock_id;
    glb_ptp_clock_id_t clk_id = {0x00, 0x00, 0x00, 0xFF, 0xFE};
    tbl_sys_global_t* p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_ptp_global_t* p_ptp_glb = tbl_ptp_global_get_ptp_global();
    
    sal_memset(clock_id, 0, sizeof(clock_id));

    /* 1. config check */
    if (2 == argc)
    {
        sal_memcpy(&clk_id[0], &p_sys_glb->route_mac[0], 3);
        sal_memcpy(&clk_id[5], &p_sys_glb->route_mac[3], 3);
        sal_memcpy(clock_id, clk_id, SSM_CLOCK_ID_LEN);
    }
    else
    {
        rc = cdb_ssm_clock_id_str2val(argv[2], clock_id);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Clock id address, format is XXXX.XXXX.XXXX.XXXX, exclude ffff.ffff.ffff.ffff and 0.0.0.0");
            return PTP_E_API_INVALID_CMD_INPUT;
        }
    }
    
    /* 2. check duplicate config */
    if (0 == sal_memcmp(p_ptp_glb->default_ds.clock_identity, clock_id, GLB_PTP_CLOCK_ID_LEN))
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_clock_id(clock_id));

    return PTP_E_SUCCESS;
}

static int32
_ptp_global_cmd_get_ptp_global(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_ptp_global_t* p_ptp_glb = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_ptp_glb = tbl_ptp_global_get_ptp_global();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_ptp_global_dump_one(p_ptp_glb, &args);
    sal_fclose(fp);

    return PTP_E_SUCCESS;
}

int32
ptp_cmd_process_ptp_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    tbl_ptp_global_t* p_ptp_glb = NULL;
    cdb_node_t* p_node = NULL;
    uint32 value = 0;
    int32 signed_value = 0;
    int32 rc = PTP_E_SUCCESS;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_PTP_GLOBAL);
    p_ptp_glb = tbl_ptp_global_get_ptp_global();
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {    
          //  CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }
        else
        {
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }
        
        switch (field_id)
        {
        case TBL_PTP_GLOBAL_FLD_GLOBAL_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[1], para->p_rs);
            rc = _ptp_cmd_set_global_enable(value, p_ptp_glb, para);
            break;

        case TBL_PTP_GLOBAL_FLD_DEVICE_TYPE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            value = ptp_desc_dev_type_str2val(argv[1]);
            rc = _ptp_cmd_set_device_type(value, p_ptp_glb, para);
            if (PTP_E_API_CLEAR_CONFIGURATION == rc)
            {
                CFG_PROMPT_RET("Notice: PTP configurations have been cleared, except clock-id, global-enable, device-type and clock-step");
            }
            break;

        case TBL_PTP_GLOBAL_FLD_SET_SYSTEM_TIME:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[1], para->p_rs);
            rc = _ptp_cmd_set_system_time(value, p_ptp_glb, para);
            break;

        default:
            if (!sal_strcmp(argv[0], "ptp_struct_set_default_ds"))
            {
                if (!sal_strcmp(argv[1], "two_step_flag"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                    value = ptp_desc_two_step_str2val(argv[2]);
                    rc = _ptp_cmd_set_default_ds_two_step_flag(value, p_ptp_glb, para);
                    if (PTP_E_API_CLEAR_CONFIGURATION == rc)
                    {
                        CFG_PROMPT_RET("Notice: PTP configurations have been cleared, except clock-id, global-enable, device-type and clock-step");
                    }
                }
                else if (!sal_strcmp(argv[1], "domain_number"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                    rc = _ptp_cmd_set_default_ds_domain_number(value, p_ptp_glb, para);
                }
                else if (!sal_strcmp(argv[1], "slave_only"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                    rc = _ptp_cmd_set_default_ds_slave_only(value, p_ptp_glb, para);
                }
                else if (!sal_strcmp(argv[1], "primary_domain"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                    rc = _ptp_cmd_set_default_ds_primary_domain(value, p_ptp_glb, para);
                }
                else if (!sal_strcmp(argv[1], "priority1"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                    rc = _ptp_cmd_set_default_ds_priority1(value, p_ptp_glb, para);
                }
                else if (!sal_strcmp(argv[1], "priority2"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                    rc = _ptp_cmd_set_default_ds_priority2(value, p_ptp_glb, para);
                }
                else if (!sal_strcmp(argv[1], "clk_accuracy"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                    rc = _ptp_cmd_set_default_ds_clk_accuracy(value, p_ptp_glb, para);
                }
                else if (!sal_strcmp(argv[1], "clk_class"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                   rc =  _ptp_cmd_set_default_ds_clk_class(value, p_ptp_glb, para);
                }
                else if (!sal_strcmp(argv[1], "clock_identity"))
                {
                    PM_CFG_CHECK_EXPECT_2PARAMS(argc, 3, 2, para->p_rs);
                    rc =  _ptp_cmd_set_clock_id(argc, argv, para);
					if(PM_E_CFG_INVALID_PARAM == rc)
					{
						return PM_E_CFG_INVALID_PARAM;
					}
                }
            }
            else if (!sal_strcmp(argv[0], "ptp_struct_set_time_prop_intern"))
            {
                if (!sal_strcmp(argv[1], "current_utc_offset"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                    PM_CFG_GET_UINT(signed_value, argv[2], para->p_rs);
                    rc = _ptp_cmd_set_time_prop_intern_current_utc_offset(signed_value, p_ptp_glb, para);
                }
                else if (!sal_strcmp(argv[1], "leap59"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                    PM_CFG_GET_BOOL(value, argv[2], para->p_rs);
                    rc = _ptp_cmd_set_time_prop_intern_leap59(value, p_ptp_glb, para);
                }
                else if (!sal_strcmp(argv[1], "leap61"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                    PM_CFG_GET_BOOL(value, argv[2], para->p_rs);
                    rc = _ptp_cmd_set_time_prop_intern_leap61(value, p_ptp_glb, para);
                }
                else if (!sal_strcmp(argv[1], "time_source"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
                    value = ptp_desc_time_source_str2val(argv[2]);
                    rc = _ptp_cmd_set_time_prop_intern_time_source(value, p_ptp_glb, para);
                }
            }
            else
            {
                CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[0]);
            }
            
            break;
        }
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _ptp_global_cmd_get_ptp_global(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    PTP_CLI_ERROR_RETURN(rc);
    return PTP_E_SUCCESS;
}
#endif

#ifndef ptp_interface_cfg
static int32
_ptp_cmd_set_port_port_ds_delay_mechanism(uint8 delay_mechanism, tbl_ptp_port_t* p_db_ptp_port, cfg_cmd_para_t *para)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    if (g_p_master->global_enable && p_db_ptp_port->enable && p_db_ptp_port->link_up)
    {
        return PTP_E_API_CANNOT_OP_ON_RUNNING;
    }

    switch (delay_mechanism)
    {
    case PTP_DELAY_MECHANISM_E2E:
    case PTP_DELAY_MECHANISM_P2P:
    case PTP_DELAY_MECHANISM_DISABLE:
        break;
    default:
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (delay_mechanism == p_db_ptp_port->port_ds.delay_mechanism)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_port_port_ds_delay_mechanism(p_db_ptp_port, delay_mechanism));

    return rc;
}

static int32
_ptp_cmd_set_port_port_ds_announce_receipt_timeout(uint8 announce_receipt_timeout, tbl_ptp_port_t* p_db_ptp_port, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    if (announce_receipt_timeout < 2 || announce_receipt_timeout > 255)
    {
        //CFG_INVALID_PARAM_RET("Invalid announce receipt timeout %hhu", announce_receipt_timeout);
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (announce_receipt_timeout == p_db_ptp_port->port_ds.announce_receipt_timeout)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_port_port_ds_announce_receipt_timeout(p_db_ptp_port, announce_receipt_timeout));

    return rc;
}

static int32
_ptp_cmd_set_port_port_ds_log_min_pdelay_req_interval(int8 log_min_pdelay_req_interval, tbl_ptp_port_t* p_db_ptp_port, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;
    
    /* 1. config check */
    //add tc check
    if (ptp_ops_is_tc())
    {
        //for ptp_bc_tc_abnor_15_pdelay_interval_tc.tcl
        //return PTP_E_API_INVALID_DEVICE_TYPE;
    }
    
    if (PTP_DELAY_MECHANISM_P2P != p_db_ptp_port->port_ds.delay_mechanism)
    {
        return PTP_E_API_INVALID_DELAY_MECHANISM;
    }

    if (log_min_pdelay_req_interval < -1 || log_min_pdelay_req_interval > 10)
    {
        //CFG_INVALID_PARAM_RET("Invalid min pdelay req interval %hhd", log_min_pdelay_req_interval);
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (log_min_pdelay_req_interval == p_db_ptp_port->port_ds.log_min_pdelay_req_interval)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_port_port_ds_log_min_pdelay_req_interval(p_db_ptp_port, log_min_pdelay_req_interval));

    return rc;
}

static int32
_ptp_cmd_set_port_port_ds_log_min_delay_req_interval(int8 log_min_delay_req_interval, tbl_ptp_port_t* p_db_ptp_port, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    if (PTP_DELAY_MECHANISM_E2E != p_db_ptp_port->port_ds.delay_mechanism)
    {
        return PTP_E_API_INVALID_DELAY_MECHANISM;
    }

    if (log_min_delay_req_interval < -1 || log_min_delay_req_interval > 10)
    {
        //CFG_INVALID_PARAM_RET("Invalid min delay req interval %hhd", log_min_delay_req_interval);
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (log_min_delay_req_interval == p_db_ptp_port->port_ds.log_min_delay_req_interval)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_port_port_ds_log_min_delay_req_interval(p_db_ptp_port, log_min_delay_req_interval));

    return rc;
}

static int32
_ptp_cmd_set_port_port_ds_log_sync_interval(int8 log_sync_interval, tbl_ptp_port_t* p_db_ptp_port, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }

    if (log_sync_interval < -1 || log_sync_interval > 10)
    {
        //CFG_INVALID_PARAM_RET("Invalid sync interval %hhd", log_sync_interval);
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (log_sync_interval == p_db_ptp_port->port_ds.log_sync_interval)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_port_port_ds_log_sync_interval(p_db_ptp_port, log_sync_interval));

    return rc;
}

static int32
_ptp_cmd_set_port_port_ds_log_announce_interval(int8 log_announce_interval, tbl_ptp_port_t* p_db_ptp_port, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (ptp_ops_is_tc())
    {
        return PTP_E_API_INVALID_DEVICE_TYPE;
    }
    
    if (log_announce_interval < 0 || log_announce_interval > 10)
    {
        //CFG_INVALID_PARAM_RET("Invalid announce interval %hhd", log_announce_interval);
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (log_announce_interval == p_db_ptp_port->port_ds.log_announce_interval)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_port_port_ds_log_announce_interval(p_db_ptp_port, log_announce_interval));

    return rc;
}

int32 _ptp_cmd_ip_valid_check(addr_ipv4_t *p_ip_addr)
{
    uint32 ip_addr = sal_ntoh32(p_ip_addr->s_addr);
    if (IN_MULTICAST(ip_addr) || IN_EXPERIMENTAL(ip_addr) || IN_BADCLASS(ip_addr))
    {
        return FALSE;
    }
    if (INADDR_ANY == ip_addr || INADDR_NONE == ip_addr || IN_LOOPBACKNET == (ip_addr >> IN_CLASSA_NSHIFT))
    {
        return FALSE;
    }

    return TRUE;
}

static int32
_ptp_cmd_set_port_port_addr_duplicate_check(addr_ipv4_t *sip, addr_ipv4_t *dip)
{
    tbl_ptp_port_master_t * p_port_master = tbl_ptp_port_get_master();
    tbl_ptp_port_t * p_db_ptp_port = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *next = NULL;
    
    CTCLIB_SLIST_LOOP_DEL(p_port_master->ptp_port_list, p_db_ptp_port, listnode, next)
    {
        if (0 == sal_memcmp(sip, &p_db_ptp_port->port_addr.field.ip_addr, sizeof(addr_ipv4_t)) && 0 == sal_memcmp(dip, &p_db_ptp_port->port_addr.dest_ip_addr, sizeof(addr_ipv4_t)))
        {
            return PTP_E_API_INVALID_IP_ADDRESS_SOURCE_DEST_CONFLICT_WITH_OTHER;
        }
    }

    return PTP_E_SUCCESS;
}

static int32
_ptp_cmd_set_port_port_addr(char **argv, int32 argc, tbl_ptp_port_t* p_db_ptp_port, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;
    uint32 value = 0;
    uint16 protocol = 0;
    addr_ipv4_t sip;
    addr_ipv4_t dip;

    sal_memset(&sip, 0, sizeof(addr_ipv4_t));
    sal_memset(&dip, 0, sizeof(addr_ipv4_t));

    /* 1. config check */
    if (!sal_strcmp(argv[3], "mac"))/*ptp protocol mac      no ptp protocol*/
    {
        protocol = PTP_NET_PROTO_IEEE_802_3;
        /* 2. check duplicate config */
        if (protocol == p_db_ptp_port->port_addr.protocol)
        {
            return PTP_E_SUCCESS;
        }
        
        p_db_ptp_port->port_addr.protocol = PTP_NET_PROTO_IEEE_802_3;
        sal_memset(p_db_ptp_port->port_addr.field.pad, 0x00, GLB_PTP_MAX_PORT_ADDR_LEN);
        sal_memset(&(p_db_ptp_port->port_addr.dest_ip_addr), 0x00, sizeof(addr_ipv4_t));
        p_db_ptp_port->port_addr.length = 0;
    }
    else if (!sal_strcmp(argv[3], "udp"))
    {
        if (argc == 6)
        {
            if (!sal_strcmp(argv[4], "udp_source_ip"))/*ptp protocol udp source-ip A.B.C.D */
            {
                CFG_PARSER_IP_ADDR(argv[5], &sip, para->p_rs);
                if (FALSE == _ptp_cmd_ip_valid_check(&sip))
                {
                    return PTP_E_API_INVALID_IP_ADDRESS;
                }

                protocol = PTP_NET_PROTO_UDP_IPV4;
                
                /* 2. check duplicate config */
                if (protocol == p_db_ptp_port->port_addr.protocol && sip.s_addr == p_db_ptp_port->port_addr.field.ip_addr.s_addr)
                {
                    return PTP_E_SUCCESS;
                }
        
                p_db_ptp_port->port_addr.protocol = PTP_NET_PROTO_UDP_IPV4;
                p_db_ptp_port->port_addr.field.ip_addr = sip;
                sal_memset(&(p_db_ptp_port->port_addr.dest_ip_addr), 0, sizeof(addr_ipv4_t));
                p_db_ptp_port->port_addr.length = 4; /*P220 D7 the length of IPv4 address*/
            }
            else if (!sal_strcmp(argv[4], "unicast_dest_ip"))/*no ptp protocol unicast*/
            {
                if(PTP_NET_PROTO_UDP_IPV4_UNICAST != p_db_ptp_port->port_addr.protocol)
                {
                    //cli_out(cli, "%% unicast ptp is not used in this port\n");
                    return PTP_E_API_NO_UNICAST_ON_INVALID_PORT;
                }
    
                protocol = PTP_NET_PROTO_UDP_IPV4;
                PM_CFG_GET_UINT(value, argv[5], para->p_rs);
                if (value == 0)
                {
                    p_db_ptp_port->port_addr.protocol = PTP_NET_PROTO_UDP_IPV4;
                    sal_memset(&(p_db_ptp_port->port_addr.dest_ip_addr), 0, sizeof(addr_ipv4_t));
                    p_db_ptp_port->port_addr.length = 4; /*P220 D7 the length of IPv4 address*/
                }
                else
                {
                    PTP_LOG_ERR("Error parameters");
                }
            }
        }
        else if (argc == 8)/*ptp protocol udp source-ip A.B.C.D unicast dest-ip A.B.C.D*/
        {
            protocol = PTP_NET_PROTO_UDP_IPV4_UNICAST;
            CFG_PARSER_IP_ADDR(argv[5], &sip, para->p_rs);
            CFG_PARSER_IP_ADDR(argv[7], &dip, para->p_rs);
            if (0 == sal_memcmp(&sip, &dip, sizeof(sip)))
            {
                return PTP_E_API_INVALID_IP_ADDRESS_SAME_SOURCE_DEST;
            }

            if (FALSE == _ptp_cmd_ip_valid_check(&sip) || FALSE == _ptp_cmd_ip_valid_check(&dip))
            {
                return PTP_E_API_INVALID_IP_ADDRESS;
            }

            /* 2. check duplicate config */
            if (sal_memcmp(&sip, &p_db_ptp_port->port_addr.field.ip_addr, sizeof(addr_ipv4_t)) || sal_memcmp(&dip, &p_db_ptp_port->port_addr.dest_ip_addr, sizeof(addr_ipv4_t)))
            {
                /*check duplicate with other interface*/
                rc = _ptp_cmd_set_port_port_addr_duplicate_check(&sip, &dip);
                if (PTP_E_SUCCESS != rc)
                {
                    return rc;
                }
                
                p_db_ptp_port->port_addr.field.ip_addr = sip;
                sal_memset(&(p_db_ptp_port->port_addr.dest_ip_addr), 0, sizeof(addr_ipv4_t));
                p_db_ptp_port->port_addr.length = 4; /*P220 D7 the length of IPv4 address*/
                
                p_db_ptp_port->port_addr.protocol = PTP_NET_PROTO_UDP_IPV4_UNICAST;
                p_db_ptp_port->port_addr.dest_ip_addr = dip;
            }
            else
            {
                return PTP_E_SUCCESS;
            }
        }
    }

    rc = tbl_ptp_port_set_ptp_port_field(p_db_ptp_port, TBL_PTP_PORT_FLD_PORT_ADDR);
    
    return rc;
}

static int32
_ptp_cmd_set_port_vlan_info(uint32 vlan_id, uint8 cos, tbl_ptp_port_t* p_db_ptp_port, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;
    PTP_API_INVALID_INTERFACE_RET(p_db_ptp_port);
    
    /* 1. config check */
    
    /* 2. check duplicate config */
    if (vlan_id == p_db_ptp_port->vlan_info.vid && cos == p_db_ptp_port->vlan_info.cos)
    {
        return PTP_E_SUCCESS;
    }

    if (vlan_id == 4096)/*no ptp tagging vlan*/
    {
        vlan_id = PTP_VLAN_ID_NONE;
        cos = PTP_VLAN_COS_MIN;
    }
    else if (vlan_id == 0 && cos == 8)/*no ptp tagging cos*/
    {
        vlan_id = p_db_ptp_port->vlan_info.vid;
        cos = PTP_VLAN_COS_MIN;
    }
    else if (cos == 0xff)
    {
        cos = PTP_VLAN_COS_MIN;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_port_vlan_info(p_db_ptp_port, vlan_id, cos));

    return rc;
}

static int32
_ptp_cmd_set_port_egress_latency(int32 egress_latency, tbl_ptp_port_t* p_db_ptp_port, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (egress_latency < 0 || egress_latency > 65535)
    {
        //CFG_INVALID_PARAM_RET("Invalid egress latency %d", egress_latency);
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (egress_latency == p_db_ptp_port->egress_latency)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_port_egress_latency(p_db_ptp_port, egress_latency));

    return rc;
}

static int32
_ptp_cmd_set_port_ingress_latency(int32 ingress_latency, tbl_ptp_port_t* p_db_ptp_port, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (ingress_latency < 0 || ingress_latency > 65535)
    {
        //CFG_INVALID_PARAM_RET("Invalid ingress latency %d", ingress_latency);
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (ingress_latency == p_db_ptp_port->ingress_latency)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_port_ingress_latency(p_db_ptp_port, ingress_latency));

    return rc;
}

static int32
_ptp_cmd_set_port_asymmetry_correction(int64 asymmetry_correction, tbl_ptp_port_t* p_db_ptp_port, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;

    /* 1. config check */
    if (asymmetry_correction < -2000000 || asymmetry_correction > 2000000)
    {
        //CFG_INVALID_PARAM_RET("Invalid asymmetry correction %"PRId64"", asymmetry_correction);
        return PTP_E_API_INVALID_CMD_INPUT;
    }
    
    /* 2. check duplicate config */
    if (asymmetry_correction == p_db_ptp_port->asymmetry_correction.v)
    {
        return PTP_E_SUCCESS;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_port_asymmetry_correction(p_db_ptp_port, asymmetry_correction));

    return rc;
}

static int32
_ptp_cmd_set_port_enable(uint32 enable, tbl_ptp_port_t* p_db_ptp_port, cfg_cmd_para_t *para)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    int32 rc = PTP_E_SUCCESS;

    PTP_API_INVALID_INTERFACE_RET(p_db_ptp_port);
    
    /* 2. check duplicate config */
    if (enable == p_db_ptp_port->enable)
    {
        return PTP_E_SUCCESS;
    }

    /*mv here for duplicate config*/
    /* 1. config check */
    if (ptp_ops_is_oc() && enable && g_p_master->default_ds.number_ports)
    {
        return PTP_E_API_OC_TOO_MANY_PORT;
    }
    
    /* 3. call API */
    PM_E_RETURN(ptp_api_set_port_enable(p_db_ptp_port, enable));

    return rc;
}

static int32
_ptp_cmd_get_ptp_port(tbl_ptp_port_t* p_db_ptp_port, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_ptp_port_master_t *p_master = tbl_ptp_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    if (NULL == p_db_ptp_port)
    {
        CTCLIB_SLIST_LOOP(p_master->ptp_port_list, p_db_ptp_port, listnode)
        {
            tbl_ptp_port_dump_one(p_db_ptp_port, &args);
        }
    }
    else
    {
        tbl_ptp_port_dump_one(p_db_ptp_port, &args);
    }
    
    sal_fclose(fp);
    
    return PTP_E_SUCCESS;
}

int32
ptp_cmd_process_ptp_port(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    tbl_ptp_port_t ptp_port;
    tbl_ptp_port_t* p_db_ptp_port = NULL;
    uint32 value = 0;
    uint32 value_2 = 0;
    int32 signed_value = 0;
    cdb_node_t* p_node = NULL;
    uint32 i = 0;
    int32 rc = PTP_E_SUCCESS;
    tbl_interface_t *p_db_if = NULL;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_PTP_PORT);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 2, para->p_rs);
        sal_memset(&ptp_port, 0, sizeof(ptp_port));
        sal_strncpy(ptp_port.key.name, argv[0], IFNAME_SIZE);

        p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(ptp_port.key));
        if (NULL == p_db_if)
        {
            CFG_CONFLICT_RET("Interface %s does not exist", IFNAME_ETH2FULL(ptp_port.key.name, ifname_ext));
        }

        if (p_db_if->hw_type != GLB_IF_TYPE_PORT_IF && p_db_if->hw_type != GLB_IF_TYPE_LINKAGG_IF)
        {
            CFG_CONFLICT_RET("PTP only support configure on physical port and LAG port");
        }

        /* check exist */
        p_db_ptp_port = tbl_ptp_port_get_ptp_port(&ptp_port.key);
        if (!p_db_ptp_port)
        {
            CFG_CONFLICT_RET("PTP port %s not found", IFNAME_ETH2FULL(ptp_port.key.name, ifname_ext));
        }

        /*member port cannot cfg*/
        IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
        ptp_port.ifindex = p_db_ptp_port->ifindex;
        
        i++;
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            //CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }        
        else
        {
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        switch (field_id)
        {
        case TBL_PTP_PORT_FLD_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[2], para->p_rs);
            rc = _ptp_cmd_set_port_enable(value, p_db_ptp_port, para);
            break;

        case TBL_PTP_PORT_FLD_ASYMMETRY_CORRECTION:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_INTEGER(signed_value, argv[2], para->p_rs);
            /*int32 is enough*/
            rc = _ptp_cmd_set_port_asymmetry_correction(signed_value, p_db_ptp_port, para);
            break;

        case TBL_PTP_PORT_FLD_INGRESS_LATENCY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _ptp_cmd_set_port_ingress_latency(value, p_db_ptp_port, para);
            break;

        case TBL_PTP_PORT_FLD_EGRESS_LATENCY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _ptp_cmd_set_port_egress_latency(value, p_db_ptp_port, para);
            break;

        case TBL_PTP_PORT_FLD_VLAN_INFO:
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            value_2 = 0xff;
            if (argc > 4)
            {
                PM_CFG_GET_UINT(value_2, argv[4], para->p_rs);
            }
            rc = _ptp_cmd_set_port_vlan_info(value, value_2, p_db_ptp_port, para);
            break;

        case TBL_PTP_PORT_FLD_PORT_ADDR:
            rc = _ptp_cmd_set_port_port_addr(argv, argc, p_db_ptp_port, para);
            break;
            
        default:
            /*argv[0] is ifname*/
            if (!sal_strcmp(argv[1], "ptp_struct_set_port_ds"))
            {
                if (!sal_strcmp(argv[2], "log_announce_interval"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
                    PM_CFG_GET_UINT(value, argv[3], para->p_rs);
                    rc = _ptp_cmd_set_port_port_ds_log_announce_interval(value, p_db_ptp_port, para);
                }
                else if (!sal_strcmp(argv[2], "log_sync_interval"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
                    PM_CFG_GET_INTEGER(signed_value, argv[3], para->p_rs);
                    rc = _ptp_cmd_set_port_port_ds_log_sync_interval(signed_value, p_db_ptp_port, para);
                }
                else if (!sal_strcmp(argv[2], "log_min_delay_req_interval"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
                    PM_CFG_GET_INTEGER(signed_value, argv[3], para->p_rs);
                    rc = _ptp_cmd_set_port_port_ds_log_min_delay_req_interval(signed_value, p_db_ptp_port, para);
                }
                else if (!sal_strcmp(argv[2], "log_min_pdelay_req_interval"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
                    PM_CFG_GET_INTEGER(signed_value, argv[3], para->p_rs);
                    rc = _ptp_cmd_set_port_port_ds_log_min_pdelay_req_interval(signed_value, p_db_ptp_port, para);
                }
                else if (!sal_strcmp(argv[2], "announce_receipt_timeout"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
                    PM_CFG_GET_UINT(value, argv[3], para->p_rs);
                    rc = _ptp_cmd_set_port_port_ds_announce_receipt_timeout(value, p_db_ptp_port, para);
                }
                else if (!sal_strcmp(argv[2], "delay_mechanism"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
                    value = ptp_desc_delay_mechanism_str2val(argv[3]);
                    rc = _ptp_cmd_set_port_port_ds_delay_mechanism(value, p_db_ptp_port, para);
                }
            }
            else
            {
                CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            }
            break;
        }

        break;

    case CDB_OPER_GET:
        if (argc >= 1)
        {
            /* check exist */
            sal_memset(&ptp_port, 0, sizeof(ptp_port));
            sal_strncpy(ptp_port.key.name, argv[0], IFNAME_SIZE);
            p_db_ptp_port = tbl_ptp_port_get_ptp_port(&ptp_port.key);
            if (!p_db_ptp_port)
            {
                CFG_CONFLICT_RET("PTP port %s not found", IFNAME_ETH2FULL(ptp_port.key.name, ifname_ext));
            }
        }
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        
        rc = _ptp_cmd_get_ptp_port(p_db_ptp_port, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    PTP_CLI_ERROR_RETURN(rc);
    return rc;
}
#endif

#ifndef ptp_show
static void
_ptp_cmd_show_global_bc(uint32 showbrief, cfg_cmd_para_t *para)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    char ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
    glb_ptp_clock_id_t clk_id_zero;
    FILE *fp = NULL;
    uint32 clock_id_show_na = FALSE;

    sal_memset(clk_id_zero, 0x00, sizeof(clk_id_zero));
    
    if (NULL == para)
    {
        PTP_LOG_ERR("Input parameter is null, (%s, %d)\n", __FUNCTION__, __LINE__);
        return ;
    }
   
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        PTP_LOG_ERR("Input parameter is null, (%s, %d)\n", __FUNCTION__, __LINE__);
        return ;
    }
    
    sal_fprintf(fp,
        "--------------------------  Global Configure  --------------------------\n"
        "PTP State              : %s\n"
        "Port Number            : %u\n"
        "Domain                 : %hhu\n"
        "Slave Only             : %s\n"
        "Clock Type             : %s\n"
        "Clock Step             : %s\n"
        "Priority1              : %hhu\n"
        "Priority2              : %hhu\n"
        "Clock Accuracy         : 0x%hhx\n"
        "Clock Class            : %hhu\n"
        "Time Source            : %s(%hhu)\n"
        "UTC Offset             : %hd\n"
        "Leap59                 : %s\n"
        "Leap61                 : %s\n"
        "Local Clock Identity   : %s\n"
	"Set systime via PTP    : %s\n",
        ptp_desc_enable(g_p_master->global_enable),
        g_p_master->default_ds.number_ports,
        g_p_master->default_ds.domain_number,
        ptp_desc_bool(g_p_master->default_ds.slave_only),
        ptp_desc_dev_type(g_p_master->device_type),
        ptp_desc_two_step(g_p_master->default_ds.two_step_flag),
        g_p_master->default_ds.priority1,
        g_p_master->default_ds.priority2,
        g_p_master->default_ds.clock_quality.clk_accuracy,
        g_p_master->default_ds.clock_quality.clk_class,
        ptp_desc_time_source(g_p_master->time_prop_intern.time_source), g_p_master->time_prop_intern.time_source,
        g_p_master->time_prop_intern.current_utc_offset,
        ptp_desc_bool(g_p_master->time_prop_intern.leap59),
        ptp_desc_bool(g_p_master->time_prop_intern.leap61),
        ptp_desc_clockid(g_p_master->default_ds.clock_identity),
	 ptp_desc_bool(g_p_master->set_system_time));

    if (showbrief)
    {
        sal_fclose(fp);
        fp = NULL;
        
        return;
    }

    if (ptp_api_current_slave_port_not_null(g_p_master->current_slave_port))
    {
        IFNAME_ETH2FULL(g_p_master->current_slave_port, ifname_ext);
    }
    else
    {
        sal_memcpy(ifname_ext, "N/A", sizeof(ifname_ext));
    }

    if (0 == sal_memcmp(clk_id_zero, g_p_master->parent_ds.parent_port_identity.clock_id, GLB_PTP_CLOCK_ID_LEN))
    {
        clock_id_show_na = TRUE;
    }
    
    sal_fprintf(fp,
        "\n---------------------------  BMC Properties  ---------------------------\n"
        "Receive Member         : %s\n"
        "Parent Clock Identity  : %s\n"
        "Parent Port Number     : %hu\n",
        ifname_ext,
        clock_id_show_na ? "N/A" : ptp_desc_clockid(g_p_master->parent_ds.parent_port_identity.clock_id),
        g_p_master->parent_ds.parent_port_identity.port_num);

    /*fix bug 48894 by chenc*/
    if (g_p_master->current_ds.mean_path_delay < 0)
    {
        sal_fprintf(fp,
            "Mean Path Delay        : 0\n");
    }
    else
    {
        sal_fprintf(fp,
            "Mean Path Delay        : %"PRId64"\n",
            g_p_master->current_ds.mean_path_delay);
    }

    sal_fprintf(fp,
        "Offset From Master     : %"PRId64"\n"
        "Step Removed           : %hu\n",
        g_p_master->current_ds.offset_from_master,
        g_p_master->current_ds.steps_removed);

    clock_id_show_na = FALSE;
    if (0 == sal_memcmp(clk_id_zero, g_p_master->parent_ds.grandmaster_identity, GLB_PTP_CLOCK_ID_LEN))
    {
        clock_id_show_na = TRUE;
    }
    
    sal_fprintf(fp,
        "GM Clock Identity      : %s\n"
        "GM Priority1           : %hhu\n"
        "GM Priority2           : %hhu\n"
        "GM Clock Accuracy      : 0x%hhx\n"
        "GM Clock Class         : %hhu\n"
        "GM Time Source         : %s(%hhu)\n"
        "GM UTC Offset          : %hd\n"
        "GM UTC Offset Valid    : %s\n"
        "GM Time Scale          : %s\n"
        "GM Time Traceable      : %s\n"
        "GM Leap59              : %s\n"
        "GM Leap61              : %s\n"
        "GM Frequency Traceable : %s\n"
        "\n",
        clock_id_show_na ? "N/A" : ptp_desc_clockid(g_p_master->parent_ds.grandmaster_identity),
        g_p_master->parent_ds.grandmaster_priority1,
        g_p_master->parent_ds.grandmaster_priority2,
        g_p_master->parent_ds.grandmaster_clock_quality.clk_accuracy,
        g_p_master->parent_ds.grandmaster_clock_quality.clk_class,
        ptp_desc_time_source(g_p_master->time_prop_ds.time_source), g_p_master->time_prop_ds.time_source,
        g_p_master->time_prop_ds.current_utc_offset,
        ptp_desc_bool(g_p_master->time_prop_ds.current_utc_offset_valid),
        (g_p_master->time_prop_ds.ptp_timescale ? "PTP" : "ARB"),
        ptp_desc_bool(g_p_master->time_prop_ds.time_traceable),
        ptp_desc_bool(g_p_master->time_prop_ds.leap59),
        ptp_desc_bool(g_p_master->time_prop_ds.leap61),
        ptp_desc_bool(g_p_master->time_prop_ds.frequency_traceable));

    sal_fclose(fp);
    fp = NULL;
    
    return ;
}

static void
_ptp_cmd_show_global_tc(cfg_cmd_para_t *para)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();

    FILE *fp = NULL;
    
    if (NULL == para)
    {
        PTP_LOG_ERR("Input parameter is null, (%s, %d)\n", __FUNCTION__, __LINE__);
        return ;
    }
   
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        PTP_LOG_ERR("Input parameter is null, (%s, %d)\n", __FUNCTION__, __LINE__);
        return ;
    }

    sal_fprintf(fp,
        "--------------------------  Global Configure  --------------------------\n"
        "PTP State              : %s\n"
        "Port Number            : %u\n"
        "Primary Domain         : %hhu\n"
        "Clock Type             : %s\n"
        "Clock Step             : %s\n"
        "Local Clock ID         : %s\n",
        ptp_desc_enable(g_p_master->global_enable),
        g_p_master->default_ds.number_ports,
        g_p_master->default_ds.primary_domain,
        ptp_desc_dev_type(g_p_master->device_type),
        ptp_desc_two_step(g_p_master->default_ds.two_step_flag),
        ptp_desc_clockid(g_p_master->default_ds.clock_identity));

    sal_fclose(fp);
    fp = NULL;

    return ;
}

static void
_ptp_cmd_show_ptp_global(uint32 is_show_ptp_brief, cfg_cmd_para_t *para)
{
    if (ptp_ops_is_tc())
    {
        _ptp_cmd_show_global_tc(para);
    }
    else
    {
        _ptp_cmd_show_global_bc(is_show_ptp_brief, para);
    }

    return;
}

static int32
_ptp_cmd_show_interface_foreign_master(void *obj, tbl_iter_args_t *pargs)
{
    FILE *fp = pargs->argv[0];
    tbl_ptp_foreign_t *fmds = obj;
    uint32 best = FALSE;
    char port_identity[32];
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    char ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    IFNAME_ETH2FULL(fmds->p_port_belong_to->key.name, ifname_ext);
    sal_snprintf(port_identity, 32, "%s@%u", ptp_desc_clockid(fmds->key.foreign_master_port_identity.clock_id),
        fmds->key.foreign_master_port_identity.port_num);
        
    best = (0 == ptp_util_portid_cmp(&g_p_master->parent_ds.parent_port_identity, &fmds->key.foreign_master_port_identity));
    
    sal_fprintf(fp, "%2s%-32s%-14s%s\n", (best ? "*" : ""), port_identity, ptp_desc_bool(fmds->qualified), ifname_ext);
            
    return PTP_E_SUCCESS;
}

static void
_ptp_cmd_show_foreign_master(cfg_cmd_para_t *para)
{
    FILE            *fp = NULL;
    tbl_iter_args_t args;    
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        PTP_LOG_ERR("Input parameter is null, (%s, %d)\n", __FUNCTION__, __LINE__);
        return ;
    }

    sal_fprintf(fp, "Codes: * - selected grandmaster\n");
    sal_fprintf(fp, "  Foreign_master_port_identity    Qualification Interface\n");
    sal_fprintf(fp, "---------------------------------+--------------+-----------------------\n");

    args.argv[0] = fp;
    tbl_ptp_foreign_iterate(_ptp_cmd_show_interface_foreign_master, &args);

    sal_fclose(fp);
    fp = NULL;
    
    return;
}


static void
_ptp_cmd_show_interface_one_stats(FILE *fp, tbl_ptp_port_t *p_port, uint32 print_ifname, uint32 statistics_packet_type)
{
    if (TRUE == print_ifname)
    {
        sal_fprintf(fp, 
            "------------------------------------------------------------------------\n"
            "Interface                  : %s\n",
            ptp_desc_ifname(p_port));
    }

    sal_fprintf(fp, 
            "---------------------------  Port Statistic  ---------------------------\n");

    if (PTP_STATS_MAX == statistics_packet_type || PTP_STATS_RX == statistics_packet_type)
    {
        sal_fprintf(fp, 
            "Recv Packet Statistics\n"
            "------------------------------------------------------------------------\n"
            "Announce          : %-12u   Sync                  : %-12u\n"
            "Delay_Req         : %-12u   Pdelay_Req            : %-12u\n"
            "Delay_Resp        : %-12u   Pdelay_Resp           : %-12u\n"
            "Follow_Up         : %-12u   Pdelay_Resp_Follow_Up : %-12u\n"
            "Unknown           : %-12u\n",
            p_port->packet_stats[PTP_STATS_RX].announce,       p_port->packet_stats[PTP_STATS_RX].sync,
            p_port->packet_stats[PTP_STATS_RX].delay_req,      p_port->packet_stats[PTP_STATS_RX].pdelay_req,
            p_port->packet_stats[PTP_STATS_RX].delay_resp,     p_port->packet_stats[PTP_STATS_RX].pdelay_resp,
            p_port->packet_stats[PTP_STATS_RX].follow_up,      p_port->packet_stats[PTP_STATS_RX].pdelay_resp_follow_up,
            p_port->packet_stats[PTP_STATS_RX].unknown);
    }
    
    if (PTP_STATS_MAX == statistics_packet_type || PTP_STATS_TX== statistics_packet_type)
    {
            sal_fprintf(fp, 
            "Send Packet Statistics\n"
            "------------------------------------------------------------------------\n"
            "Announce          : %-12u   Sync                  : %-12u\n"
            "Delay_Req         : %-12u   Pdelay_Req            : %-12u\n"
            "Delay_Resp        : %-12u   Pdelay_Resp           : %-12u\n"
            "Follow_Up         : %-12u   Pdelay_Resp_Follow_Up : %-12u\n"
            "Unknown           : %-12u\n",
            p_port->packet_stats[PTP_STATS_TX].announce,       p_port->packet_stats[PTP_STATS_TX].sync,
            p_port->packet_stats[PTP_STATS_TX].delay_req,      p_port->packet_stats[PTP_STATS_TX].pdelay_req,
            p_port->packet_stats[PTP_STATS_TX].delay_resp,     p_port->packet_stats[PTP_STATS_TX].pdelay_resp,
            p_port->packet_stats[PTP_STATS_TX].follow_up,      p_port->packet_stats[PTP_STATS_TX].pdelay_resp_follow_up,
            p_port->packet_stats[PTP_STATS_TX].unknown);
    }
    
    if (PTP_STATS_MAX == statistics_packet_type || PTP_STATS_DROP == statistics_packet_type)
    {
            sal_fprintf(fp, 
            "Discard Packet Statistics\n"
            "------------------------------------------------------------------------\n"
            "Announce          : %-12u   Sync                  : %-12u\n"
            "Delay_Req         : %-12u   Pdelay_Req            : %-12u\n"
            "Delay_Resp        : %-12u   Pdelay_Resp           : %-12u\n"
            "Follow_Up         : %-12u   Pdelay_Resp_Follow_Up : %-12u\n"
            "Unknown           : %-12u\n",
            p_port->packet_stats[PTP_STATS_DROP].announce,     p_port->packet_stats[PTP_STATS_DROP].sync,
            p_port->packet_stats[PTP_STATS_DROP].delay_req,    p_port->packet_stats[PTP_STATS_DROP].pdelay_req,
            p_port->packet_stats[PTP_STATS_DROP].delay_resp,   p_port->packet_stats[PTP_STATS_DROP].pdelay_resp,
            p_port->packet_stats[PTP_STATS_DROP].follow_up,    p_port->packet_stats[PTP_STATS_DROP].pdelay_resp_follow_up,
            p_port->packet_stats[PTP_STATS_DROP].unknown);
    }

    return;
}

#define PTP_IP_ADDR_STR_LEN 16

static int32
_ptp_cmd_show_interface_one(void *obj, tbl_iter_args_t *pargs)
{
    FILE *fp = pargs->argv[0];
    uint32 *showbrief = pargs->argv[1];
    tbl_ptp_port_t *p_port_display = pargs->argv[2];
    uint32 *had_brief_title_been_show = pargs->argv[3];
    tbl_interface_t *p_interface = obj;
    tbl_ptp_port_t *p_port = NULL;
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    uint32 statistics_packet_type = PTP_STATS_MAX;

    /*lag member not display*/
    if (INTERFACE_IS_LAG_MEMBER((p_interface)))
    {
        return PTP_E_SUCCESS;
    }
    
    p_port = tbl_ptp_port_get_ptp_port((tbl_ptp_port_key_t *)(&(p_interface->key)));
    if (NULL == p_port)
    {
        return PTP_E_SUCCESS;
    }
    
    if (p_port_display != NULL && p_port->ifindex != p_port_display->ifindex)
    {
        return PTP_E_SUCCESS;
    }
    
    uint32 two_step = PTP_TWO_STEP_DEF;
    two_step = g_p_master->default_ds.two_step_flag;

    char *port_state_desc;
    
    if (ptp_ops_is_tc())
    {
        port_state_desc = p_port->port_ds.faulty_flag ? "faulty" : "normal";
    }
    else
    {
        port_state_desc = ptp_desc_state(p_port->port_ds.port_state);
    }

    if (*showbrief)
    {
        if (FALSE == *had_brief_title_been_show)
        {
            sal_fprintf(fp,
                        "Name            State           Delay-mechanism Enable          Step\n"
                        "---------------+---------------+---------------+---------------+--------\n");
            *had_brief_title_been_show = TRUE;
        }
    
        sal_fprintf(fp, 
            "%-16s%-16s%-16s%-16s%s\n",
            ptp_desc_ifname(p_port),
            port_state_desc,
            ptp_desc_delay_mechanism(p_port->port_ds.delay_mechanism),
            ptp_desc_enable(p_port->enable),
            (two_step ? "two" : "one"));

        return PTP_E_SUCCESS;
    }

    sal_fprintf(fp,
            "------------------------------------------------------------------------\n"
            "Interface                  : %s\n"
            "---------------------------  Port Configure  ---------------------------\n"
            "PTP state                  : %s\n"
            "Port ID                    : %s@%hu\n"
            "Delay Mechanism            : %s\n"
            "Step Mode                  : %s\n"
            "Port State                 : %s\n",
            ptp_desc_ifname(p_port),
            ptp_desc_enable(p_port->enable),
            ptp_desc_clockid(g_p_master->default_ds.clock_identity),
            p_port->port_ds.port_identity_port_num,
            ptp_desc_delay_mechanism(p_port->port_ds.delay_mechanism),
            (two_step ? "two" : "one"),
            port_state_desc);

    if (PTP_NET_PROTO_IEEE_802_3 == p_port->port_addr.protocol)
    {
        sal_fprintf(fp,
            "Addressing                 : IEEE 802.3 /Ethernet\n");
    }
    else if (PTP_NET_PROTO_UDP_IPV4 == p_port->port_addr.protocol)
    {
        char tmp[PTP_IP_ADDR_STR_LEN];
        
        sal_memset(tmp, 0x00, PTP_IP_ADDR_STR_LEN);
        uint32 ip = sal_hton32(p_port->port_addr.field.ip_addr.s_addr);
        sal_inet_ntop(AF_INET, &ip, tmp, PTP_IP_ADDR_STR_LEN);
        sal_fprintf(fp,
            "Addressing                 : UDP /IP version 4\n"
            "Port Address               : %s\n",
            tmp);
    }
    else if(PTP_NET_PROTO_UDP_IPV4_UNICAST == p_port->port_addr.protocol)
    {
        char tmp[PTP_IP_ADDR_STR_LEN];
        
        sal_memset(tmp, 0x00, PTP_IP_ADDR_STR_LEN);
        uint32 ip = sal_hton32(p_port->port_addr.dest_ip_addr.s_addr);
        sal_inet_ntop(AF_INET, &ip, tmp, PTP_IP_ADDR_STR_LEN);
        sal_fprintf(fp,
            "Addressing                 : UDP /IP version 4 unicast\n"
            "Port Address               : %s\n",
            tmp);
    }

    if (0 == p_port->vlan_info.vid)
    {
        sal_fprintf(fp,
            "Tagging Mode               : untagged\n");
    }
    else if ((PTP_NET_PROTO_UDP_IPV4 == p_port->port_addr.protocol) || (PTP_NET_PROTO_UDP_IPV4_UNICAST == p_port->port_addr.protocol))
    {
        sal_fprintf(fp,
            "Tagging Mode               : tagged\n"
            "VLAN/COS                   : %hd/%hd\n",
            p_port->vlan_info.vid,
            p_port->vlan_info.cos);
    }

    if (!ptp_ops_is_tc())
    {
        sal_fprintf(fp,
            "Announce Interval          : %hhd\n"
            "Sync Interval              : %hhd\n"
            "Announce Receipt Timeout   : %hhu\n",
            p_port->port_ds.log_announce_interval,
            p_port->port_ds.log_sync_interval,
            p_port->port_ds.announce_receipt_timeout);
    }

    if (!ptp_ops_is_tc() && PTP_DELAY_MECHANISM_E2E == p_port->port_ds.delay_mechanism)
    {
        sal_fprintf(fp,
            "Delay_Req Interval         : %hhd\n",
            p_port->port_ds.log_min_delay_req_interval);
    }
    
    if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
    {
        sal_fprintf(fp,
            "PDelay_Req Interval        : %hhd\n"
            "Peer Mean Path Delay       : %"PRId64"\n",
            p_port->port_ds.log_min_pdelay_req_interval,
            p_port->port_ds.peer_mean_path_delay);
    }

    _ptp_cmd_show_interface_one_stats(fp, p_port, FALSE, statistics_packet_type);

    return PTP_E_SUCCESS;
}

static void
_ptp_cmd_show_interface(uint32 is_show_brief, tbl_ptp_port_t *p_port, cfg_cmd_para_t *para)
{
    FILE            *fp = NULL;
    tbl_iter_args_t args;
    uint32 had_brief_title_been_show = FALSE;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        PTP_LOG_ERR("Input parameter is null, (%s, %d)\n", __FUNCTION__, __LINE__);
        return ;
    }
    
#if 0
    if (is_show_brief)
    {
        sal_fprintf(fp,
                    "Name            State           Delay-mechanism Enable          Step\n"
                    "---------------+---------------+---------------+---------------+--------\n");
    }
#endif

    args.argv[0] = fp;
    args.argv[1] = &is_show_brief;
    args.argv[2] = p_port;
    args.argv[3] = &had_brief_title_been_show;
    
    /*use tbl_interface_iterate for 1/2/3/4, not 1/10/11/12*/
    tbl_interface_iterate(_ptp_cmd_show_interface_one, &args);

    sal_fclose(fp);
    fp = NULL;
    
    return;
}


static int32
_ptp_cmd_show_second(cfg_cmd_para_t *para)
{
    FILE            *fp = NULL;
    int32 second = 0;
    int32 nanoseconds = 0;
    time_t time;
    
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        PTP_LOG_ERR("Input parameter is null, (%s, %d)\n", __FUNCTION__, __LINE__);
        return PTP_E_ERROR;
    }

    sal_fprintf(fp, "Note:\n");
    sal_fprintf(fp, "     PTP SECOND: start from 1970-01-01 00:00:00\n");
    sal_fprintf(fp, "     WN: week number, unit is weeks, start from 1980-01-06 00:00:00\n");
    sal_fprintf(fp, "     TOW: time of week, unit is seconds, start from 1980-01-06 00:00:00\n");
    sal_fprintf(fp, "\n");
    
    PTP_CLI_ERROR_RETURN(ptp_adpt_get_hardware_second(&second, &nanoseconds));
    sal_fprintf(fp, "PTP SECOND: %d\n", second);

    if(second < g_p_master->time_prop_ds.current_utc_offset)
    {
        sal_fprintf(fp, "Current time is less than the start of UTC, 1970-01-01!\n");
    }
    else
    {
        time = second - g_p_master->time_prop_ds.current_utc_offset;
        sal_fprintf(fp, "UTC DATE: %s", ctime(&time));/*ctime include \n*/
    }

    if(second < PTP_SECOND_BT_UNIX_WITH_GPS)
    {
         sal_fprintf(fp, "Current time is less than %u second, can not show TOW!\n", PTP_SECOND_BT_UNIX_WITH_GPS);
    }
    else
    {
        sal_fprintf(fp, "WN: %d, TOW: %d\n", ((second - PTP_SECOND_BT_UNIX_WITH_GPS)/604800), ((second - PTP_SECOND_BT_UNIX_WITH_GPS)%604800));
    }

    sal_fclose(fp);
    fp = NULL;
    
    return PTP_E_SUCCESS;
}

int
_uint64_to_str(uint64_t src, char dest[UINT64_STR_LEN])
{
    uint64_t i = 20, j = 0;;
    uint64_t value, sum;

    memset(dest, 0, UINT64_STR_LEN);
    if(0 == src)
    {
        dest[0] = 48;
        return 0;
    }

    sum = src;
    while(sum)
    {
        value =  sum % 10;
        if(i > 0)
            dest[i--] = value + 48;

        sum = sum / 10;
    }

    /*move the string to the front*/
    for(j = 0; j < (UINT64_STR_LEN - 1 - i); j++)
        dest[j] = dest[i + j + 1];

    for(; j <= UINT64_STR_LEN - 1; j++)
        dest[j] = 0;

    return 0;
}

static int32
_ptp_cmd_show_enable_info(cfg_cmd_para_t *para)
{
    FILE            *fp = NULL;
    uint64_t hw_sec = 0;
    uint64_t hw_msec = 0;
    uint64_t platform_sec = 0;
    uint64_t platform_msec = 0;
    char sec_str[UINT64_STR_LEN] = {0};
    char msec_str[UINT64_STR_LEN] = {0};
    char total_offset[UINT64_STR_LEN] = {0};
    
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        PTP_LOG_ERR("Input parameter is null, (%s, %d)\n", __FUNCTION__, __LINE__);
        return PTP_E_ERROR;
    }
    
    hw_sec = g_p_master->glb_enable_hw_msec_time/1000;
    hw_msec = g_p_master->glb_enable_hw_msec_time%1000;
    platform_sec = g_p_master->glb_enable_pf_msec_time/1000;
    platform_msec = g_p_master->glb_enable_pf_msec_time%1000;

    _uint64_to_str(hw_sec, sec_str);
    _uint64_to_str(hw_msec, msec_str);
    sal_fprintf(fp, "ASIC enable timer:     sec:%s, msec:%s\n", sec_str, msec_str);
    
    _uint64_to_str(platform_sec, sec_str);
    _uint64_to_str(platform_msec, msec_str);
    sal_fprintf(fp, "Platform enable timer: sec:%s, msec:%s\n", sec_str, msec_str);

    _uint64_to_str(g_p_master->total_offset, total_offset);
    sal_fprintf(fp, "Total offset: nsec:%s\n", total_offset);

    sal_fclose(fp);
    fp = NULL;
    
    return PTP_E_SUCCESS;
}

static int32
_ptp_cmd_show_statistics_interface_one(void *obj, tbl_iter_args_t *pargs)
{
    FILE *fp = pargs->argv[0];
    tbl_ptp_port_t *p_port_display = pargs->argv[1];
    uint32 *statistics_packet_type = pargs->argv[2];
    
    tbl_interface_t *p_interface = obj;
    tbl_ptp_port_t *p_db_ptp_port = NULL;

    /*lag member not display*/
    if (INTERFACE_IS_LAG_MEMBER((p_interface)))
    {
        return PTP_E_SUCCESS;
    }
    
    p_db_ptp_port = tbl_ptp_port_get_ptp_port((tbl_ptp_port_key_t *)(&(p_interface->key)));
    if (NULL == p_db_ptp_port)
    {
        return PTP_E_SUCCESS;
    }
    
    if (p_port_display != NULL && p_db_ptp_port->ifindex != p_port_display->ifindex)
    {
        return PTP_E_SUCCESS;
    }

    _ptp_cmd_show_interface_one_stats(fp, p_db_ptp_port, TRUE, *statistics_packet_type);

    return PTP_E_SUCCESS;
}


static int32
_ptp_cmd_show_statistics_interface(cfg_cmd_para_t *para, tbl_ptp_port_t* p_ptp_port, uint32 statistics_packet_type)
{
    tbl_iter_args_t args;
    FILE            *fp = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        PTP_LOG_ERR("Input parameter is null, (%s, %d)\n", __FUNCTION__, __LINE__);
        return PTP_E_ERROR;
    }
    
    args.argv[0] = fp;
    args.argv[1] = p_ptp_port;
    args.argv[2] = &statistics_packet_type;
    
    /*use tbl_interface_iterate for 1/2/3/4, not 1/10/11/12*/
    tbl_interface_iterate(_ptp_cmd_show_statistics_interface_one, &args);

    sal_fclose(fp);
    fp = NULL;
    
    return PTP_E_SUCCESS;
}

static int32
_ptp_cmd_show_statistics(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    uint32 statistics_packet_type = PTP_STATS_MAX;
    tbl_ptp_port_t ptp_port;
    tbl_ptp_port_t* p_db_ptp_port = NULL;

    if (1 == argc)
    {
        if (0 != sal_strcmp(argv[0], "statistics"))
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }
    }
    else if (2 == argc)
    {
        if (0 != sal_strcmp(argv[0], "statistics"))
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }
        
        sal_memset(&ptp_port, 0, sizeof(ptp_port));
        sal_strncpy(ptp_port.key.name, argv[1], IFNAME_SIZE);
        
        /* check exist */
        p_db_ptp_port = tbl_ptp_port_get_ptp_port(&ptp_port.key);
        if (!p_db_ptp_port)
        {
            CFG_CONFLICT_RET("PTP port %s not found", IFNAME_ETH2FULL(ptp_port.key.name, ifname_ext));
        }
    }
    else if (3 == argc)
    {
        if (0 != sal_strcmp(argv[0], "statistics"))
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }
        
        sal_memset(&ptp_port, 0, sizeof(ptp_port));
        sal_strncpy(ptp_port.key.name, argv[1], IFNAME_SIZE);
        
        /* check exist */
        p_db_ptp_port = tbl_ptp_port_get_ptp_port(&ptp_port.key);
        if (!p_db_ptp_port)
        {
            CFG_CONFLICT_RET("PTP port %s not found", IFNAME_ETH2FULL(ptp_port.key.name, ifname_ext));
        }

        statistics_packet_type = ptp_desc_statistics_packet_type_str2val(argv[2]);
        if (PTP_STR_TO_VAL_ERROR == statistics_packet_type)
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }
    }
    else
    {
        CFG_INVALID_PARAM_RET("Invalid parameter");
    }

    _ptp_cmd_show_statistics_interface(para, p_db_ptp_port, statistics_packet_type);
    
    return PTP_E_SUCCESS;
}

int32
ptp_cmd_process_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PTP_E_SUCCESS;
    vlan_bmp_t bmp;
    uint32 is_brief = FALSE;
    uint32 is_show_ptp_brief = FALSE;

    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    tbl_ptp_port_t ptp_port;
    tbl_ptp_port_t* p_db_ptp_port = NULL;
    
    sal_memset(bmp, 0, sizeof(bmp));
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (argc == 1)           
        {
            if (0 == sal_strcmp(argv[0], "all"))/*show ptp*/
            {
                is_show_ptp_brief = FALSE;
                _ptp_cmd_show_ptp_global(is_show_ptp_brief, para);
            }
            else if (0 == sal_strcmp(argv[0], "brief"))/*show ptp brief*/
            {
                is_show_ptp_brief = TRUE;
                _ptp_cmd_show_ptp_global(is_show_ptp_brief, para);
            }
            else if (0 == sal_strcmp(argv[0], "foreign-master"))/*show ptp foreign-master*/
            {
                _ptp_cmd_show_foreign_master(para);
            }
            else if (0 == sal_strcmp(argv[0], "second"))/*show ptp second*/
            {
                _ptp_cmd_show_second(para);
            }
            else if (0 == sal_strcmp(argv[0], "enable-info"))/*show ptp enable-info*/
            {
                _ptp_cmd_show_enable_info(para);
            }
            else if (0 == sal_strcmp(argv[0], "statistics"))/*show ptp statistics*/
            {
                rc = _ptp_cmd_show_statistics(argv, argc, para);
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid parameter");
            }
        }
        else if (argc == 2 || argc == 3)/*show ptp statistics ifname [recieve | send | discard]*/
        {
            rc = _ptp_cmd_show_statistics(argv, argc, para);
        }
        else if (argc == 4)/*show ptp interface*/
        {
            if (0 != sal_strcmp(argv[1], ""))
            {
                sal_memset(&ptp_port, 0, sizeof(ptp_port));
                sal_strncpy(ptp_port.key.name, argv[1], IFNAME_SIZE);
                
                /* check exist */
                p_db_ptp_port = tbl_ptp_port_get_ptp_port(&ptp_port.key);
                if (!p_db_ptp_port)
                {
                    CFG_CONFLICT_RET("PTP port %s not found", IFNAME_ETH2FULL(ptp_port.key.name, ifname_ext));
                }
            }
            
            if (0 != sal_strcmp(argv[3], "all"))
            {
                is_brief = TRUE;
            }
            
            _ptp_cmd_show_interface(is_brief, p_db_ptp_port, para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }

        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid parameter");
        break;
    }
    
    return rc;
}
#endif

#ifndef ptp_clear
int32
_ptp_cmd_clear_statistics_one(void *obj, tbl_iter_args_t *pargs)
{
    tbl_ptp_port_t           *p_db_ptp_port = obj;
    PTP_API_INVALID_INTERFACE_RET(p_db_ptp_port);
    
    sal_memset(&p_db_ptp_port->packet_stats, 0x00, sizeof(ptp_port_stats_t));

    return PTP_E_API_NONE;
}


int32
_ptp_cmd_clear_intf_stats(tbl_ptp_port_t* p_ptp_port)
{
    if (NULL != p_ptp_port)
    {
        PTP_API_INVALID_INTERFACE_RET(p_ptp_port);
        sal_memset(&p_ptp_port->packet_stats, 0x00, sizeof(ptp_port_stats_t));
    }
    else
    {
        tbl_ptp_port_iterate(_ptp_cmd_clear_statistics_one, NULL);
    }
    
    return PTP_E_API_NONE;
}

int32
ptp_cmd_process_clear(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    tbl_ptp_port_t ptp_port;
    tbl_ptp_port_t* p_db_ptp_port = NULL;
    
    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (1 == argc)
        {
            if (0 != sal_strcmp(argv[0], "statistics"))
            {
                CFG_INVALID_PARAM_RET("Invalid parameter");
            }
        }
        else if (2 == argc)
        {
            if (0 != sal_strcmp(argv[0], "statistics"))
            {
                CFG_INVALID_PARAM_RET("Invalid parameter");
            }
            
            sal_memset(&ptp_port, 0, sizeof(ptp_port));
            sal_strncpy(ptp_port.key.name, argv[1], IFNAME_SIZE);
            
            /* check exist */
            p_db_ptp_port = tbl_ptp_port_get_ptp_port(&ptp_port.key);
            if (!p_db_ptp_port)
            {
                CFG_CONFLICT_RET("PTP port %s not found", IFNAME_ETH2FULL(ptp_port.key.name, ifname_ext));
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    _ptp_cmd_clear_intf_stats(p_db_ptp_port);
    
    return PTP_E_SUCCESS;
}
#endif

int32
_ptp_cmd_get_ptp_debug(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_ptp_debug_t* p_dbg = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_dbg = tbl_ptp_debug_get_ptp_debug();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_ptp_debug_dump_one(p_dbg, &args);
    sal_fclose(fp);

    return PTP_E_SUCCESS;
}

int32
ptp_cmd_process_ptp_debug(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_ptp_debug_t dbg;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    int32 i = 0;
    tbl_ptp_debug_t *p_db_dbg = NULL;
    field_parse_t field;
    uint32 multi_type = DBG_PTP_TYPE_FIELD;
    int32 rc = PTP_E_SUCCESS;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_PTP_DEBUG);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&dbg, 0, sizeof(dbg));

        /* check exist */
        p_db_dbg = tbl_ptp_debug_get_ptp_debug();
        if (!p_db_dbg)
        {
            CFG_CONFLICT_RET("Table ptp debug not found");
        }

        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);

        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            if (0 == sal_strcmp(argv[i], "all"))
            {
                multi_type = DBG_PTP_TYPE_ALL;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
            }
        }

        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);

        if (DBG_PTP_TYPE_FIELD == multi_type)
        {
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
            switch (field_id)
            {
            case TBL_PTP_DEBUG_FLD_PTP_COMMUNICATION_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.ptp, PTPDBG_FLAG_COMMUNICATION_DBG);
                break;
            case TBL_PTP_DEBUG_FLD_PTP_BMC_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.ptp, PTPDBG_FLAG_BMC_DBG);
                break;
            case TBL_PTP_DEBUG_FLD_PTP_TIMER_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.ptp, PTPDBG_FLAG_TIMER_DBG);
                break;
            case TBL_PTP_DEBUG_FLD_PTP_SYNCHRONIZATION_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.ptp, PTPDBG_FLAG_SYNCHRONIZATION_DBG);
                break;
            case TBL_PTP_DEBUG_FLD_PTP_PACKET_RX_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.ptp, PTPDBG_FLAG_PACKET_RX_DBG);
                break;
            case TBL_PTP_DEBUG_FLD_PTP_PACKET_TX_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.ptp, PTPDBG_FLAG_PACKET_TX_DBG);
                break;
            case TBL_PTP_DEBUG_FLD_PTP_PACKET_DISCARD_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.ptp, PTPDBG_FLAG_PACKET_DISCARD_DBG);
                break;
            default:
                CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
                break;
            }
            rc = tbl_ptp_debug_set_ptp_debug_field(&dbg, field_id);
        }
        else if (DBG_PTP_TYPE_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, dbg.ptp, PTPDBG_FLAG_COMMUNICATION_DBG);
            GLB_SET_FLAG_COND(value, dbg.ptp, PTPDBG_FLAG_BMC_DBG);
            GLB_SET_FLAG_COND(value, dbg.ptp, PTPDBG_FLAG_TIMER_DBG);
            GLB_SET_FLAG_COND(value, dbg.ptp, PTPDBG_FLAG_SYNCHRONIZATION_DBG);
            GLB_SET_FLAG_COND(value, dbg.ptp, PTPDBG_FLAG_PACKET_RX_DBG);
            GLB_SET_FLAG_COND(value, dbg.ptp, PTPDBG_FLAG_PACKET_TX_DBG);
            GLB_SET_FLAG_COND(value, dbg.ptp, PTPDBG_FLAG_PACKET_DISCARD_DBG);

            rc += tbl_ptp_debug_set_ptp_debug_field(&dbg, TBL_PTP_DEBUG_FLD_PTP_COMMUNICATION_DEBUG);
            rc += tbl_ptp_debug_set_ptp_debug_field(&dbg, TBL_PTP_DEBUG_FLD_PTP_BMC_DEBUG);
            rc += tbl_ptp_debug_set_ptp_debug_field(&dbg, TBL_PTP_DEBUG_FLD_PTP_TIMER_DEBUG);
            rc += tbl_ptp_debug_set_ptp_debug_field(&dbg, TBL_PTP_DEBUG_FLD_PTP_SYNCHRONIZATION_DEBUG);
            rc += tbl_ptp_debug_set_ptp_debug_field(&dbg, TBL_PTP_DEBUG_FLD_PTP_PACKET_RX_DEBUG);
            rc += tbl_ptp_debug_set_ptp_debug_field(&dbg, TBL_PTP_DEBUG_FLD_PTP_PACKET_TX_DEBUG);
            rc += tbl_ptp_debug_set_ptp_debug_field(&dbg, TBL_PTP_DEBUG_FLD_PTP_PACKET_DISCARD_DEBUG);
        }
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _ptp_cmd_get_ptp_debug(para, p_node, &field);
        if (rc < 0)
        {
            CFG_CONFLICT_RET("Invalid path");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}


static int32
_ptp_cmd_get_ptp_foreign(tbl_ptp_foreign_t* p_db_data, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_ptp_foreign_master_t *p_master = tbl_ptp_foreign_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    if (NULL == p_db_data)
    {
        CTCLIB_SLIST_LOOP(p_master->ptp_foreign_list, p_db_data, listnode)
        {
            tbl_ptp_foreign_dump_one(p_db_data, &args);
        }
    }
    else
    {
        tbl_ptp_foreign_dump_one(p_db_data, &args);
    }
    
    sal_fclose(fp);
    
    return PTP_E_SUCCESS;
}


int32
ptp_cmd_process_ptp_foreign(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    tbl_ptp_foreign_t* p_db_ptp_foreign = NULL;
    cdb_node_t* p_node = NULL;
    int32 rc = PTP_E_SUCCESS;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_PTP_PORT);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        
        rc = _ptp_cmd_get_ptp_foreign(p_db_ptp_foreign, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    PTP_CLI_ERROR_RETURN(rc);
    return rc;
}


static int32
_ptp_cmd_get_ptp_residence_time_cache(tbl_ptp_residence_time_cache_t* p_db_data, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_ptp_residence_time_cache_master_t *p_master = tbl_ptp_residence_time_cache_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    if (NULL == p_db_data)
    {
        CTCLIB_SLIST_LOOP(p_master->ptp_residence_time_cache_list, p_db_data, listnode)
        {
            tbl_ptp_residence_time_cache_dump_one(p_db_data, &args);
        }
    }
    else
    {
        tbl_ptp_residence_time_cache_dump_one(p_db_data, &args);
    }
    
    sal_fclose(fp);
    
    return PTP_E_SUCCESS;
}


int32
ptp_cmd_process_ptp_residence_time_cache(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    tbl_ptp_residence_time_cache_t* p_db_ptp_residence_time_cache = NULL;
    cdb_node_t* p_node = NULL;
    int32 rc = PTP_E_SUCCESS;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_PTP_PORT);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        
        rc = _ptp_cmd_get_ptp_residence_time_cache(p_db_ptp_residence_time_cache, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    PTP_CLI_ERROR_RETURN(rc);
    return rc;
}


