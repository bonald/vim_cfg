#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ssm_global_define.h"
#include "gen/tbl_ssm_global.h"
#include "gen/tbl_ssm_port_define.h"
#include "gen/tbl_ssm_port.h"
#include "gen/tbl_ssm_debug_define.h"
#include "gen/tbl_ssm_debug.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "ssm.h"
#include "ssm_api.h"
#include "ssm_opts.h"
#include "ctc_task.h"
#include "ssm_pdu.h"
#include "ssm_fsm.h"
#include "ssm_desc.h"

static int32 
ssm_tx_rx_quality_check(uint32 option_mode, char *quality_str)
{
    int32 quality_value = 0;
    if (SSM_OPTION_MODE_1 == option_mode)
    {
        if (0 == sal_strcmp(quality_str, "prc" ))
        {
            quality_value = SSM_QL_PRC;
        }
        else if (0 == sal_strcmp(quality_str, "ssu-a" ))
        {
            quality_value = SSM_QL_TNC;
        }
        else if (0 == sal_strcmp(quality_str, "ssu-b" ))
        {
            quality_value = SSM_QL_SSUB;
        }
        else if (0 == sal_strcmp(quality_str, "eec1" ))
        {
            quality_value = SSM_QL_EEC1;
        }
        else if (0 == sal_strcmp(quality_str, "dnu" ))
        {
            quality_value = SSM_QL_DNU;
        }
        else if (0 == sal_strcmp(quality_str, "na" ))
        {
            quality_value = SSM_QL_INV3;
        }
        else
        {
            quality_value = -1;
        }
    }
    else
    {
        if (0 == sal_strcmp(quality_str, "stu" ))
        {
            quality_value = SSM_QL_STU;
        }
        else if (0 == sal_strcmp(quality_str, "prs" ))
        {
            quality_value = SSM_QL_PRS;
        }
        else if (0 == sal_strcmp(quality_str, "tnc" ))
        {
            quality_value = SSM_QL_TNC;
        }
        else if (0 == sal_strcmp(quality_str, "st2" ))
        {
            quality_value = SSM_QL_ST2;
        }
        else if (0 == sal_strcmp(quality_str, "eec2" ))
        {
            quality_value = SSM_QL_EEC2;
        }
        else if (0 == sal_strcmp(quality_str, "smc" ))
        {
            quality_value = SSM_QL_SMC;
        }
        else if (0 == sal_strcmp(quality_str, "st3e" ))
        {
            quality_value = SSM_QL_ST3E;
        }
        else if (0 == sal_strcmp(quality_str, "prov" ))
        {
            quality_value = SSM_QL_PROV;
        }
        else if (0 == sal_strcmp(quality_str, "dus" ))
        {
            quality_value = SSM_QL_DNU;
        }
        else if (0 == sal_strcmp(quality_str, "na" ))
        {
            quality_value = SSM_QL_INV3;
        }
        else
        {
            quality_value = -1;
        }
    }
    return quality_value;
}

int32
ssm_cmd_set_synce_global_enable(uint32 synce_enable, tbl_ssm_global_t* p_ssm_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (synce_enable == p_ssm_glb->synce_enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(ssm_set_synce_global_enable(synce_enable));

    return rc;
}

int32
ssm_cmd_set_synce_local_priority(uint32 local_priority, tbl_ssm_global_t* p_ssm_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (local_priority < SSM_PRIORITY_MIN || local_priority > SSM_PRIORITY_MAX)
    {
        CFG_INVALID_PARAM_RET("Invalid priority value, the priority range is 1 to 255");
    }
    
    /* 2. check duplicate config */
    if (local_priority == p_ssm_glb->local_priority)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(ssm_set_synce_local_priority(local_priority));

    return rc;    
}

int32
ssm_cmd_set_synce_system_quality(uint32 option_mode, uint32 system_quality, tbl_ssm_global_t* p_ssm_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if ((SSM_OPTION_MODE_1 == option_mode) && (0 != p_ssm_glb->force_quality_option2_cnt))
    {
        CFG_CONFLICT_RET("Can't configure quality value, option collision between force quality and local quality");
    }
    if ((SSM_OPTION_MODE_2 == option_mode) && (0 !=p_ssm_glb->force_quality_option1_cnt))
    {
        CFG_CONFLICT_RET("Can't configure quality value, option collision between force quality and local quality");
    }
    
    /* 2. check duplicate config */
    if ((option_mode == p_ssm_glb->option_mode) && (system_quality == p_ssm_glb->system_quality))
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(ssm_set_synce_system_quality(option_mode, system_quality));

    return rc;
}

int32
ssm_cmd_set_synce_device_role(uint32 device_role, tbl_ssm_global_t* p_ssm_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (device_role == p_ssm_glb->device_role)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(ssm_set_synce_device_role(device_role));

    return rc;
}

int32
ssm_cmd_set_synce_ssm_enable(uint32 ssm_enable, tbl_ssm_global_t* p_ssm_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (ssm_enable == p_ssm_glb->ssm_enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(ssm_set_synce_ssm_enable(ssm_enable));

    return rc;
}

int32
ssm_cmd_set_synce_ssm_extend_enable(uint32 ssm_extend_enable, tbl_ssm_global_t* p_ssm_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
   
    /* 2. check duplicate config */
    if (ssm_extend_enable == p_ssm_glb->ssm_extend_enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(ssm_set_synce_ssm_extend_enable(ssm_extend_enable));

    return rc;
}

int32
ssm_cmd_set_synce_holdoff_time(uint32 holdoff_time, tbl_ssm_global_t* p_ssm_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    holdoff_time = holdoff_time * 100;
    if (holdoff_time < SYNCE_HOLDOFF_TIME_MIN || holdoff_time > SYNCE_HOLDOFF_TIME_MAX)
    {
        CFG_CONFLICT_RET("Invalid hold-off time, the holdoff-time range is 300ms to 1800ms");
    }
    
    /* 2. check duplicate config */
    if (holdoff_time == p_ssm_glb->holdoff_time)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(ssm_set_synce_holdoff_time(holdoff_time));

    return rc;
}

int32
ssm_cmd_set_synce_wait_to_restore_time(uint32 wtr_time, tbl_ssm_global_t* p_ssm_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    wtr_time = wtr_time * 60;
    if (wtr_time < SYNCE_WTR_TIME_MIN || wtr_time > SYNCE_WTR_TIME_MAX)
    {
        CFG_CONFLICT_RET("Invalid wait-to-restore time, the wtr-time range is 0 minute to 12 minutes");
    }
    
    /* 2. check duplicate config */
    if (wtr_time == p_ssm_glb->wtr_time)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(ssm_set_synce_wait_to_restore_time(wtr_time));

    return rc;
}

int32
ssm_cmd_set_synce_clock_id(ssm_clock_id_t clock_id, tbl_ssm_global_t* p_ssm_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (0 == sal_memcmp(p_ssm_glb->device_clock_id, clock_id, SSM_CLOCK_ID_LEN))
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(ssm_set_synce_clock_id(clock_id));

    return rc;
}
    
int32
ssm_global_cmd_get_ssm_global(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_ssm_global_dump_one, &args);
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
ssm_cmd_set_if_synce_enable(tbl_ssm_port_t* p_ssm_if, tbl_ssm_port_t* p_db_ssm_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_SIZE];
    tbl_interface_t * p_db_if = NULL;
    
    /* 1. config check */
    p_db_if = tbl_interface_get_interface_by_name(p_db_ssm_if->key.name);
    if (NULL == p_db_if)
    {
        CFG_INVALID_PARAM_RET("Interface %s does not exist", IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext));
    }
    if (!p_db_if->support_ssm_en)
    {
        CFG_INVALID_PARAM_RET("Interface %s does not suppor synce ssm", IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext));
    }
    
    /* 2. check duplicate config */
    if (p_ssm_if->synce_enable == p_db_ssm_if->synce_enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(ssm_set_if_synce_enable(p_ssm_if));
    
    return rc;
}

int32
ssm_cmd_set_if_ssm_pdu_direction(tbl_ssm_port_t* p_ssm_if, tbl_ssm_port_t* p_db_ssm_if, uint32 set_enable, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (set_enable)
    {
        /* 2. check duplicate config */
        if (p_ssm_if->ssm_pdu_direct == p_db_ssm_if->ssm_pdu_direct)
        {
            return PM_E_NONE;
        }
        if ((SSM_PDU_RX == p_ssm_if->ssm_pdu_direct && SSM_PDU_TX == p_db_ssm_if->ssm_pdu_direct) ||
            (SSM_PDU_TX == p_ssm_if->ssm_pdu_direct && SSM_PDU_RX == p_db_ssm_if->ssm_pdu_direct) ||
            (SSM_PDU_TX == p_ssm_if->ssm_pdu_direct && SSM_PDU_ALL == p_db_ssm_if->ssm_pdu_direct) ||
            (SSM_PDU_RX == p_ssm_if->ssm_pdu_direct && SSM_PDU_ALL == p_db_ssm_if->ssm_pdu_direct) )
        {
            p_ssm_if->ssm_pdu_direct = SSM_PDU_ALL;
        }
    }
    else
    {
        /* 2. check duplicate config */
        if ((p_ssm_if->ssm_pdu_direct == p_db_ssm_if->ssm_pdu_direct) || 
            (p_ssm_if->ssm_pdu_direct == SSM_PDU_ALL) || 
            (p_db_ssm_if->ssm_pdu_direct == SSM_PDU_NONE))
        {
            p_ssm_if->ssm_pdu_direct = SSM_PDU_NONE;
        }
        else
        {
            if (SSM_PDU_RX == p_ssm_if->ssm_pdu_direct)
            {
                p_ssm_if->ssm_pdu_direct = SSM_PDU_TX;
            }
            else
            {
                p_ssm_if->ssm_pdu_direct = SSM_PDU_RX;
            }   
        }
    }

    /* 3. call API */
    PM_E_RETURN(ssm_set_if_ssm_pdu_direction(p_ssm_if));
    
    return rc;
}

int32
ssm_cmd_set_if_priority(tbl_ssm_port_t* p_ssm_if, tbl_ssm_port_t* p_db_ssm_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (p_ssm_if->priority < SSM_PRIORITY_MIN || p_ssm_if->priority > SSM_PRIORITY_MAX)
    {
        CFG_INVALID_PARAM_RET("Invalid priority value, the priority range is 1 to 255");
    }
    
    /* 2. check duplicate config */
    if (p_ssm_if->priority == p_db_ssm_if->priority)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(ssm_set_if_priority(p_ssm_if));
    
    return rc;
}

int32
ssm_cmd_set_if_replace_clockid_enable(tbl_ssm_port_t* p_ssm_if, tbl_ssm_port_t* p_db_ssm_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_ssm_if->replace_clockid_en == p_db_ssm_if->replace_clockid_en)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(ssm_set_if_replace_clockid_enable(p_ssm_if));
    
    return rc;
}

int32
ssm_cmd_set_if_dnu_group_id(tbl_ssm_port_t* p_ssm_if, tbl_ssm_port_t* p_db_ssm_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (p_ssm_if->dnu_group_id < SSM_DNU_GROUP_MIN || p_ssm_if->dnu_group_id > SSM_DNU_GROUP_MAX)
    {
        CFG_INVALID_PARAM_RET("Invalid DNU group value, the DNU group range is 0 to 255");
    }
    
    /* 2. check duplicate config */
    if (p_ssm_if->dnu_group_id == p_db_ssm_if->dnu_group_id)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(ssm_set_if_dnu_group_id(p_ssm_if));
    
    return rc;
}

int32
ssm_cmd_set_if_tx_quality_value(tbl_ssm_port_t* p_ssm_if, tbl_ssm_port_t* p_db_ssm_if, char * quality_str, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 quality_value = 0;
    tbl_ssm_global_t * p_ssm_glb = tbl_ssm_global_get_ssm_global();

    /* 1. config check */
    if (SSM_OPTION_MODE_1 == p_ssm_glb->option_mode)
    {
        quality_value = ssm_tx_rx_quality_check(SSM_OPTION_MODE_1, quality_str);
        if (quality_value < 0)
        {
            CFG_CONFLICT_RET("Can't configure quality value %s under option1 mode", quality_str);
        }
    }
    else
    {
        quality_value = ssm_tx_rx_quality_check(SSM_OPTION_MODE_2, quality_str);
        if (quality_value < 0)
        {
            CFG_CONFLICT_RET("Can't configure quality value %s under option2 mode", quality_str);
        }
    }

    /* 2. check duplicate config */
    if (quality_value == p_db_ssm_if->force_tx_quality)
    {
        return PM_E_NONE;
    }
    else
    {
        if ((SSM_QL_INV3 == p_db_ssm_if->force_tx_quality) && (SSM_QL_INV3 != quality_value))
        {
            if (SSM_OPTION_MODE_1 == p_ssm_glb->option_mode)
            {
                p_ssm_glb->force_quality_option1_cnt++;
            }
            else
            {
                p_ssm_glb->force_quality_option2_cnt++;
            }
        }
        if((SSM_QL_INV3 != p_db_ssm_if->force_tx_quality) && (SSM_QL_INV3 == quality_value))
        {
            if (SSM_OPTION_MODE_1 == p_ssm_glb->option_mode)
            {
                p_ssm_glb->force_quality_option1_cnt--;
            }
            else
            {
                p_ssm_glb->force_quality_option2_cnt--;
            }
        }
        p_ssm_if->force_tx_quality = quality_value;
    }
    
    /* 3. call API */
    PM_E_RETURN(ssm_set_if_tx_quality_value(p_ssm_if));

    return rc;
}


int32
ssm_cmd_set_if_rx_quality_value(tbl_ssm_port_t* p_ssm_if, tbl_ssm_port_t* p_db_ssm_if, char * quality_str, cfg_cmd_para_t *para)
{
    
    int32 rc = PM_E_NONE;
    int32 quality_value = 0;
    tbl_ssm_global_t * p_ssm_glb = tbl_ssm_global_get_ssm_global();

    /* 1. config check */
    if (SSM_OPTION_MODE_1 == p_ssm_glb->option_mode)
    {
        quality_value = ssm_tx_rx_quality_check(SSM_OPTION_MODE_1, quality_str);
        if (quality_value < 0)
        {
            CFG_CONFLICT_RET ("Can't configure quality value %s under option1 mode", quality_str);
        }
    }
    else
    {
        quality_value = ssm_tx_rx_quality_check(SSM_OPTION_MODE_2, quality_str);
        if (quality_value < 0)
        {
            CFG_CONFLICT_RET ("Can't configure quality value %s under option2 mode", quality_str);
        }
    }

    /* 2. check duplicate config */
    if (quality_value == p_db_ssm_if->force_rx_quality)
    {
        return PM_E_NONE;
    }
    else
    {
        if ((SSM_QL_INV3 == p_db_ssm_if->force_rx_quality) && (SSM_QL_INV3 != quality_value))
        {
            if (SSM_OPTION_MODE_1 == p_ssm_glb->option_mode)
            {
                p_ssm_glb->force_quality_option1_cnt++;
            }
            else
            {
                p_ssm_glb->force_quality_option2_cnt++;
            }
        }
        if((SSM_QL_INV3 != p_db_ssm_if->force_rx_quality) && (SSM_QL_INV3 == quality_value))
        {
            if (SSM_OPTION_MODE_1 == p_ssm_glb->option_mode)
            {
                p_ssm_glb->force_quality_option1_cnt--;
            }
            else
            {
                p_ssm_glb->force_quality_option2_cnt--;
            }
        }
        p_ssm_if->force_rx_quality = quality_value;
    }
    
    /* 3. call API */
    PM_E_RETURN(ssm_set_if_rx_quality_value(p_ssm_if));
    
    return rc;
}

int32
ssm_port_cmd_get_ssm_port(char* ifname, field_parse_t *p_field, cfg_cmd_para_t *para)
{
    tbl_ssm_port_master_t   *p_master = tbl_ssm_port_get_master();
    ctclib_slistnode_t      *listnode = NULL;
    tbl_ssm_port_t          *p_db_ssm_port = NULL;
    tbl_ssm_port_key_t      ssm_port_key;
    tbl_iter_args_t         args;
    FILE                    *fp = NULL;
    char                    ifname_ext[IFNAME_EXT_SIZE];
    int32                   rc = PM_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    if (ifname)
    {   
        sal_memset(&ssm_port_key, 0, sizeof(ssm_port_key));
        sal_strncpy(ssm_port_key.name, ifname, IFNAME_SIZE);
        p_db_ssm_port = tbl_ssm_port_get_ssm_port(&ssm_port_key);
        if (NULL == p_db_ssm_port)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("Interface %s is not exist", IFNAME_ETH2FULL(ifname, ifname_ext));
        }
        tbl_ssm_port_dump_one(p_db_ssm_port, &args);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_port, listnode)
        {
            tbl_ssm_port_dump_one(p_db_ssm_port, &args);
        }
    }
    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

int32
ssm_cmd_get_ssm_debug(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    tbl_ssm_debug_t* p_dbg = NULL;
    
    FILE *fp = NULL;    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    p_dbg = tbl_ssm_debug_get_ssm_debug();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_ssm_debug_dump_one(p_dbg, &args);
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

int32
ssm_cmd_process_ssm_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    cdb_node_t* p_node = NULL;
    
    tbl_ssm_global_t* p_ssm_glb = NULL;
    tbl_sys_global_t* p_sys_glb = tbl_sys_global_get_sys_global();
    uint32 synce_enable = 0;
    uint32 local_priority = 0;
    uint32 option_mode = 0;
    uint32 system_quality = SSM_QL_DNU;
    uint32 device_role = 0;
    uint32 ssm_enable = 0;
    uint32 ssm_extend_enable = 0;
    uint32 holdoff_time = 0;
    uint32 wtr_time = 0;
    ssm_clock_id_t clock_id;
    ssm_clock_id_t clk_id = {0x00, 0x00, 0x00, 0xFF, 0xFE};
    int32  rc = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_SSM_GLOBAL);
    p_ssm_glb = tbl_ssm_global_get_ssm_global();

    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
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
        case TBL_SSM_GLOBAL_FLD_SYNCE_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(synce_enable, argv[1], para->p_rs);
            rc = ssm_cmd_set_synce_global_enable(synce_enable, p_ssm_glb, para);
            break;

       case TBL_SSM_GLOBAL_FLD_LOCAL_PRIORITY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(local_priority, argv[1], para->p_rs);
            rc = ssm_cmd_set_synce_local_priority(local_priority, p_ssm_glb, para);
            break;
            
       case TBL_SSM_GLOBAL_FLD_OPTION_MODE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            PM_CFG_GET_UINT(option_mode, argv[1], para->p_rs);
            if (SSM_OPTION_MODE_1 == option_mode)
            {
                if (0 == sal_strcmp(argv[3], "eec1")) 
                {
                    system_quality = SSM_QL_EEC1;
                } 
                else if (0 == sal_strcmp(argv[3], "prc"))
                {
                    system_quality = SSM_QL_PRC;
                }
                else
                {
                    system_quality = SSM_QL_DNU;
                }
                rc = ssm_cmd_set_synce_system_quality(option_mode, system_quality, p_ssm_glb, para);
            }
            else if (SSM_OPTION_MODE_2 == option_mode)
            {
                if (0 == sal_strcmp(argv[3], "eec2")) 
                {
                    system_quality = SSM_QL_EEC2;
                } 
                else if (0 == sal_strcmp(argv[3], "prs"))
                {
                    system_quality = SSM_QL_PRS;
                }
                else
                {
                    system_quality = SSM_QL_DNU;
                }
                rc = ssm_cmd_set_synce_system_quality(option_mode, system_quality, p_ssm_glb, para);
            }
            else
            {
                return PM_E_INVALID_PARAM;
            }
            break;
            
       case TBL_SSM_GLOBAL_FLD_DEVICE_ROLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            if (0 == sal_strcmp(argv[1], "master-only")) 
            {
                device_role = SSM_DEVICE_MAST;
            }
            else if (0 == sal_strcmp(argv[1], "slave-only")) 
            {
                device_role = SSM_DEVICE_SLAV;
            }
            else if (0 == sal_strcmp(argv[1], "default-role"))
            {
                device_role = SSM_DEVICE_STAND;
            }
            rc = ssm_cmd_set_synce_device_role(device_role, p_ssm_glb, para);
            break;
            
       case TBL_SSM_GLOBAL_FLD_SSM_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(ssm_enable, argv[1], para->p_rs);
            rc = ssm_cmd_set_synce_ssm_enable(ssm_enable, p_ssm_glb, para);
            break;
            
       case TBL_SSM_GLOBAL_FLD_SSM_EXTEND_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(ssm_extend_enable, argv[1], para->p_rs);
            rc = ssm_cmd_set_synce_ssm_extend_enable(ssm_extend_enable, p_ssm_glb, para);
            break;

       case TBL_SSM_GLOBAL_FLD_HOLDOFF_TIME:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(holdoff_time, argv[1], para->p_rs);
            rc = ssm_cmd_set_synce_holdoff_time(holdoff_time, p_ssm_glb, para);
            break;

       case TBL_SSM_GLOBAL_FLD_WTR_TIME:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(wtr_time, argv[1], para->p_rs);
            rc = ssm_cmd_set_synce_wait_to_restore_time(wtr_time, p_ssm_glb, para);
            break;
       case TBL_SSM_GLOBAL_FLD_DEVICE_CLOCK_ID:
            sal_memset(clock_id, 0, sizeof(clock_id));
            PM_CFG_CHECK_MIN_PARAMS(argc, 2, para->p_rs);
            if (argc == 2)
            {
                sal_memcpy(&clk_id[0], &p_sys_glb->route_mac[0], 3);
                sal_memcpy(&clk_id[5], &p_sys_glb->route_mac[3], 3);
                sal_memcpy(clock_id, clk_id, SSM_CLOCK_ID_LEN);
            }
            else if (argc == 3)
            {
                rc = cdb_ssm_clock_id_str2val(argv[2], clock_id);
                if (rc < 0)
                {
                    CFG_INVALID_PARAM_RET("Invalid clock id address %s", argv[2]);
                }
            }
            rc = ssm_cmd_set_synce_clock_id(clock_id, p_ssm_glb, para);
            break;
            
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[0]);
            break;
        }
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = ssm_global_cmd_get_ssm_global(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    return rc;
}

int32
ssm_cmd_process_ssm_port(char **argv, int32 argc, cfg_cmd_para_t *para)
{    
    field_parse_t field;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    cdb_node_t* p_node = NULL;

    tbl_ssm_port_t  p_ssm_if;
    tbl_ssm_port_t *p_db_ssm_if = NULL;
    char  *ifname = NULL;
    uint32 set_enable = FALSE;
    uint32 ssm_pdu_direct = 0;
    int32  rc = PM_E_NONE;
    
    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_SSM_PORT);
    
    sal_memset(&p_ssm_if, 0, sizeof(p_ssm_if));
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
        sal_strncpy(p_ssm_if.key.name, argv[0], IFNAME_SIZE);
        p_db_ssm_if = tbl_ssm_port_get_ssm_port(&(p_ssm_if.key));
        if (NULL == p_db_ssm_if)
        {
            /*Modified by yejl for bug 47920, 2018-07-16*/
            CFG_INVALID_PARAM_RET("Cannot configure on this interface, only physical port can be configured");
        }
        
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
        case TBL_SSM_PORT_FLD_SYNCE_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(p_ssm_if.synce_enable, argv[2], para->p_rs);
            rc = ssm_cmd_set_if_synce_enable(&p_ssm_if, p_db_ssm_if, para);
            break;
            
        case TBL_SSM_PORT_FLD_SSM_PDU_DIRECT:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            PM_CFG_GET_UINT(set_enable, argv[2], para->p_rs);
            if (0 == sal_strcmp(argv[3], "all")) 
            {
                ssm_pdu_direct = SSM_PDU_ALL;
            }
            else if (0 == sal_strcmp(argv[3], "rx"))
            {
                ssm_pdu_direct = SSM_PDU_RX;
            }
            else if (0 == sal_strcmp(argv[3], "tx"))
            {
                ssm_pdu_direct = SSM_PDU_TX;
            }
            p_ssm_if.ssm_pdu_direct = ssm_pdu_direct;
            rc = ssm_cmd_set_if_ssm_pdu_direction(&p_ssm_if, p_db_ssm_if, set_enable, para);
            break;
            
         case TBL_SSM_PORT_FLD_PRIORITY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(p_ssm_if.priority, argv[2], para->p_rs);
            rc = ssm_cmd_set_if_priority(&p_ssm_if, p_db_ssm_if, para);
            break;
            
         case TBL_SSM_PORT_FLD_REPLACE_CLOCKID_EN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(p_ssm_if.replace_clockid_en, argv[2], para->p_rs);
            rc = ssm_cmd_set_if_replace_clockid_enable(&p_ssm_if, p_db_ssm_if, para);
            break;

         case TBL_SSM_PORT_FLD_DNU_GROUP_ID:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(p_ssm_if.dnu_group_id, argv[2], para->p_rs);
            rc = ssm_cmd_set_if_dnu_group_id(&p_ssm_if, p_db_ssm_if, para);
            break;

         case TBL_SSM_PORT_FLD_FORCE_TX_QUALITY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            rc = ssm_cmd_set_if_tx_quality_value(&p_ssm_if, p_db_ssm_if, argv[2], para);
            break;
            
         case TBL_SSM_PORT_FLD_FORCE_RX_QUALITY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            rc = ssm_cmd_set_if_rx_quality_value(&p_ssm_if, p_db_ssm_if, argv[2], para);
            break;
        }
        break;
    case CDB_OPER_GET:
        if (0 == argc)
        {
            ifname = NULL;
        }
        else
        {
            ifname = argv[0];    
        }
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = ssm_port_cmd_get_ssm_port(ifname, &field, para);
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}


int32
ssm_cmd_process_ssm_debug(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    int32 i = 0;
    field_parse_t field;

    tbl_ssm_debug_t ssmdbg;
    tbl_ssm_debug_t *p_db_ssmdbg = NULL;
    uint32 multi_type = DBG_SSM_TYPE_FIELD;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_SSM_DEBUG);
    sal_memset(&ssmdbg, 0, sizeof(ssmdbg));
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        /* check exist */
        p_db_ssmdbg = tbl_ssm_debug_get_ssm_debug();
        if (!p_db_ssmdbg)
        {
            CFG_CONFLICT_RET("SSM_DEBUG not found");
        }
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            if (0 == sal_strcmp(argv[i], "all"))
            {
                multi_type = DBG_SSM_TYPE_ALL;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
            }
        }
        PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

        if (DBG_SSM_TYPE_FIELD == multi_type)
        {
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
            switch (field_id)
            {
            case TBL_SSM_DEBUG_FLD_SSM_EVENT:
                GLB_SET_FLAG_COND(value, ssmdbg.ssm, SSM_DEBUG_FLAG_EVENT);
                break;
            case TBL_SSM_DEBUG_FLD_SSM_PACKET_TX:
                GLB_SET_FLAG_COND(value, ssmdbg.ssm, SSM_DEBUG_FLAG_PACKET_TX);
                break;
            case TBL_SSM_DEBUG_FLD_SSM_PACKET_RX:
                GLB_SET_FLAG_COND(value, ssmdbg.ssm, SSM_DEBUG_FLAG_PACKET_RX);
                break;
            case TBL_SSM_DEBUG_FLD_SSM_TIMER:
                GLB_SET_FLAG_COND(value, ssmdbg.ssm, SSM_DEBUG_FLAG_TIMER);
                break;
            }
            PM_E_RETURN(tbl_ssm_debug_set_ssm_debug_field(&ssmdbg, field_id));
        }
        else if (DBG_SSM_TYPE_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, ssmdbg.ssm, SSM_DEBUG_FLAG_EVENT);
            GLB_SET_FLAG_COND(value, ssmdbg.ssm, SSM_DEBUG_FLAG_PACKET_TX);
            GLB_SET_FLAG_COND(value, ssmdbg.ssm, SSM_DEBUG_FLAG_PACKET_RX);
            GLB_SET_FLAG_COND(value, ssmdbg.ssm, SSM_DEBUG_FLAG_TIMER);
            PM_E_RETURN(tbl_ssm_debug_set_ssm_debug_field(&ssmdbg, TBL_SSM_DEBUG_FLD_SSM_EVENT));
            PM_E_RETURN(tbl_ssm_debug_set_ssm_debug_field(&ssmdbg, TBL_SSM_DEBUG_FLD_SSM_PACKET_TX));
            PM_E_RETURN(tbl_ssm_debug_set_ssm_debug_field(&ssmdbg, TBL_SSM_DEBUG_FLD_SSM_PACKET_RX));
            PM_E_RETURN(tbl_ssm_debug_set_ssm_debug_field(&ssmdbg, TBL_SSM_DEBUG_FLD_SSM_TIMER));
        }
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = ssm_cmd_get_ssm_debug(para, p_node, &field);
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
ssm_cmd_process_show_ssm_port(tbl_ssm_port_t* p_db_ssm_if, cfg_cmd_para_t *para)
{    
    char ifname_ext[IFNAME_EXT_SIZE];
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp,"------------------------------------------------------------------------\n");
    sal_fprintf(fp,"Interface                           : %s\n", IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext));
    sal_fprintf(fp,"---------------------------  Port Configure  ---------------------------\n");
    sal_fprintf(fp,"Synce enable                        : %s\n", ssm_desc_enable(p_db_ssm_if->synce_enable));
    
    sal_fprintf(fp,"ESMC RX/TX                          : %s\n", ssm_desc_if_pdu(p_db_ssm_if->ssm_pdu_direct));
    sal_fprintf(fp,"Port up                             : %s\n", ssm_desc_enable(ssm_port_is_up(p_db_ssm_if)));
    sal_fprintf(fp,"Receive quality                     : %s\n", ssm_desc_quality(p_db_ssm_if->receive_quality, 2));
    sal_fprintf(fp,"Send quality                        : %s\n", ssm_desc_quality(p_db_ssm_if->send_quality, 1));

    sal_fprintf(fp,"Quality RX configured               : %s\n", ssm_desc_quality(p_db_ssm_if->force_rx_quality, 1));
    sal_fprintf(fp,"Quality TX configured               : %s\n", ssm_desc_quality(p_db_ssm_if->force_tx_quality, 1));
    sal_fprintf(fp,"Port priority                       : %d\n", p_db_ssm_if->priority);
    if (p_db_ssm_if->dnu_group_id)
    {
        sal_fprintf(fp,"DNU group                           : %d\n", p_db_ssm_if->dnu_group_id);
    }
    else
    {
        sal_fprintf(fp,"DNU group                           : %s\n", "N/A");
    }
    sal_fprintf(fp,"Replace clockid                     : %s\n", ssm_desc_enable(p_db_ssm_if->replace_clockid_en));
    sal_fprintf(fp,"Send timer running                  : %s\n", ssm_desc_enable(p_db_ssm_if->ssm_send_timer ? TRUE : FALSE));
    sal_fprintf(fp,"Timeout timer running               : %s\n", ssm_desc_enable(p_db_ssm_if->ssm_timeout_timer ? TRUE : FALSE));
    
    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

static int32
ssm_cmd_process_show_ssm_global(cfg_cmd_para_t *para)
{
    char              clock_id_str[SSM_CLOCKID_STR_LEN] = {'\0'};
    char              ifname_ext1[IFNAME_EXT_SIZE] = {'\0'};
    char              ifname_ext2[IFNAME_EXT_SIZE] = {'\0'};
    tbl_ssm_global_t* p_ssm_glb = tbl_ssm_global_get_ssm_global();
    
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp,"-----------------  Global Synce Clock Configure  -----------------\n");
    sal_fprintf(fp,"Device ClockID                     : %s\n", ssm_desc_clockid(clock_id_str, p_ssm_glb->device_clock_id));
    sal_fprintf(fp,"Enable global synce                : %s\n", ssm_desc_enable(p_ssm_glb->synce_enable));
    sal_fprintf(fp,"Enable global ssm                  : %s\n", ssm_desc_enable(p_ssm_glb->ssm_enable));
    sal_fprintf(fp,"SSM option mode                    : %d\n", p_ssm_glb->option_mode);

    sal_fprintf(fp,"Device quality                     : %s\n", ssm_desc_quality(p_ssm_glb->system_quality, 1));
    sal_fprintf(fp,"Current quality                    : %s\n", ssm_desc_quality(p_ssm_glb->current_quality, 2));
    sal_fprintf(fp,"Device priority                    : %d\n", p_ssm_glb->local_priority);
    sal_fprintf(fp,"Current selected interface         : %s\n", ssm_desc_select_interface(ifname_ext1, FALSE));
    sal_fprintf(fp,"Next selected interface            : %s\n", ssm_desc_select_interface(ifname_ext2, TRUE));

    sal_fprintf(fp,"Enable ssm extension               : %s\n", ssm_desc_enable(p_ssm_glb->ssm_extend_enable));
    sal_fprintf(fp,"Clock running state                : %s\n", ssm_desc_device_status(p_ssm_glb));
    sal_fprintf(fp,"Device type                        : %s\n", ssm_desc_role(p_ssm_glb->device_role));
    if (p_ssm_glb->holdoff_send_timer)
    {
        sal_fprintf(fp,"Clock remaining holdoff-time(ms)   : %d\n", ctc_timer_get_remain_msec(p_ssm_glb->holdoff_send_timer));
    }
    else
    {
        sal_fprintf(fp,"Clock remaining holdoff-time(ms)   : %s\n", "N/A");
    }
    if (p_ssm_glb->wtr_send_timer)
    {
        sal_fprintf(fp,"Clock remaining WTR-time(s)        : %d\n", ctc_timer_get_remain_sec(p_ssm_glb->wtr_send_timer));
    }
    else
    {
        sal_fprintf(fp,"Clock remaining WTR-time(s)        : %s\n", "N/A");
    }
    sal_fprintf(fp,"Clock holdoff-time(ms)             : %d\n", p_ssm_glb->holdoff_time ? p_ssm_glb->holdoff_time : 0);
    sal_fprintf(fp,"Clock WTR-time(s)                  : %d\n", p_ssm_glb->wtr_time ? p_ssm_glb->wtr_time : 0);

    sal_fprintf(fp,"------------------------  DPLL status  ----------------------------\n");
    sal_fprintf(fp,"Selected Clock Valid Status        : %s\n", ssm_desc_dpll_selected_if_status(p_ssm_glb));
    sal_fprintf(fp,"DPLL running state                 : %s\n", ssm_desc_dpll_status(p_ssm_glb->dpll_current_status));
    
    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

int32
ssm_cmd_process_show_ssm_dnu_group(uint32 dnu_group_id, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    uint32 i = 0;
    uint32 has_show_title = 0;
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    tbl_ssm_port_master_t *p_master = tbl_ssm_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_ssm_port_t* p_db_ssm_if = NULL;

    if (0 == dnu_group_id)
    {
        for (i = 1; i <= SSM_DNU_GROUP_MAX; i++)
        {
            has_show_title  = 0;
            CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
            {
                if (p_db_ssm_if && p_db_ssm_if->dnu_group_id == i)
                {
                    if (has_show_title == 0)
                    {
                        sal_fprintf(fp, "DNU-Group: %d\n", i);
                        has_show_title = 1;
                    }
                    sal_fprintf(fp, "%s ", IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext)); 
                }  
            }
            if (has_show_title)
            {
                sal_fprintf(fp, "\n");
            }
        }
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
        {
            if (p_db_ssm_if && p_db_ssm_if->dnu_group_id == dnu_group_id)
            {
               if (has_show_title == 0)
               {
                   sal_fprintf(fp, "DNU-Group: %d\n", dnu_group_id);
                   has_show_title = 1;
               }
               sal_fprintf(fp, "%s ", IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext)); 
            }  
        }
    }

    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

int32
ssm_cmd_process_show_ssm(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_ssm_port_t*     p_db_ssm_if = NULL;
    tbl_ssm_port_key_t  ssm_if_key;
    char                ifname_ext[IFNAME_EXT_SIZE];
    uint32              dnu_group_id = 0;
    int32               rc = PM_E_NONE;
    
    CFG_DBG_FUNC();
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            ssm_cmd_process_show_ssm_global(para);
        }
        else if (1 == argc)
        {
            if (0 == sal_strcmp(argv[0], "dnu-group"))
            {
                dnu_group_id = 0;
                ssm_cmd_process_show_ssm_dnu_group(dnu_group_id, para);
            }
            else
            {
                sal_memset(&ssm_if_key, 0, sizeof(ssm_if_key));
                sal_strncpy(ssm_if_key.name, argv[0], IFNAME_SIZE);
                p_db_ssm_if = tbl_ssm_port_get_ssm_port(&ssm_if_key);
                if (NULL == p_db_ssm_if)
                {
                    IFNAME_ETH2FULL(argv[0], ifname_ext);
                    CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
                } 
                ssm_cmd_process_show_ssm_port(p_db_ssm_if, para);
            }
            
        }
        else if (2 == argc)
        {
            PM_CFG_GET_UINT(dnu_group_id, argv[1], para->p_rs);
            ssm_cmd_process_show_ssm_dnu_group(dnu_group_id, para);
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
    
    return PM_E_NONE;
}

