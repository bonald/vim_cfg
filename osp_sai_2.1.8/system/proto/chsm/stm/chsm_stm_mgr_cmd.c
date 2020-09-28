/****************************************************************************
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Centec
 * Date         :       2015-08-06
 * Reason       :       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "chsm_inc.h"

/****************************************************************************
 *  
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
 *  
* Global and Declaration
*
*****************************************************************************/


/****************************************************************************
 *  
* Function
*
*****************************************************************************/

int32
_chsm_stm_mgr_cmd_map_sys_spec_type(char **argv, int32 argc, cfg_cmd_para_t *para, uint32 *type)
{
    tbl_chassis_t* p_chassis = NULL;
    uint32 i = 1;
    
    if (argc >= 2)
    {
        if (0 == sal_strcmp(argv[i], "default"))
        {
            *type = GLB_STM_MODE_DEFAULT;            
        }
        else if (0 == sal_strcmp(argv[i], "ipv4"))
        {
            *type = GLB_STM_MODE_IPUCV4;            
        }
        else if (0 == sal_strcmp(argv[i], "vlan"))
        {
            *type = GLB_STM_MODE_VLAN;            
        }
        else if (0 == sal_strcmp(argv[i], "ipv6"))
        {
            *type = GLB_STM_MODE_IPUCV6;            
        }
        else if (0 == sal_strcmp(argv[i], "nvgre"))
        {
            *type = GLB_STM_MODE_NVGRE;            
        }
        else if (0 == sal_strcmp(argv[i], "l2vpn"))
        {
            *type = GLB_STM_MODE_L2VPN;            
        }
        else if (0 == sal_strcmp(argv[i], "openflow-default"))
        {
            *type = GLB_STM_MODE_OF_DEFAULT;            
        }
        else if (0 == sal_strcmp(argv[i], "openflow-hybrid"))
        {
            *type = GLB_STM_MODE_OF_HYBRID;            
        }
        else if (0 == sal_strcmp(argv[i], "openflow-ipv6"))
        {
            *type = GLB_STM_MODE_OF_IPV6;            
        }
        else if (0 == sal_strcmp(argv[i], "flow6"))
        {
            *type = GLB_STM_MODE_TAP_IPV4_IPV6;
        }
        else if (0 == sal_strcmp(argv[i], "current"))
        {
            p_chassis = tbl_chassis_get_chassis();  
            
            *type = p_chassis? p_chassis->cur_stm_mode : GLB_STM_MODE_DEFAULT;
        }
        else if (0 == sal_strcmp(argv[i], "next"))
        {
            p_chassis = tbl_chassis_get_chassis();  
            
            *type = p_chassis? p_chassis->next_stm_mode : GLB_STM_MODE_DEFAULT;           
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid stm profile type: %s", argv[0]);
        }        
        return PM_E_NONE;
    }
    CFG_INVALID_PARAM_RET("Stm profile type is not specified");
}

int32 
chsm_stm_mgr_show_sys_spec(uint8 type, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    field_parse_t field;
    tbl_iter_args_t args;
    tbl_sys_spec_t *spec = NULL;
    #if 0
    tbl_sys_spec_t *cur_spec = NULL;
    tbl_sys_spec_t *next_boot_spec = NULL;    
    #endif
    int32 rc = PM_E_NONE;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);

    fp = sal_fopen(para->p_show->path, "w+");
    if (!fp)
    {
        return PM_E_FAIL;
    }

    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = &field;
    args.argv[1] = fp;

    spec = tbl_sys_spec_get_sys_spec_by_type(type);
    PM_E_RETURN(tbl_sys_spec_dump_one(spec, &args));

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
chsm_stm_mgr_cmd_process_sys_spec(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 type = 0;

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;
    
    case CDB_OPER_GET:
        if (argc)
        {
            PM_E_RETURN(_chsm_stm_mgr_cmd_map_sys_spec_type(argv, argc, para, &type));
        }
        else
        {
            type = chsm_mgt_get_stm_mode();
        }
        
        rc = chsm_stm_mgr_show_sys_spec(type, para);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

