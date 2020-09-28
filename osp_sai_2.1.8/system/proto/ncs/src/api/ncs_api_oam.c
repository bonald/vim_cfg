
#include "proto.h"
#include "ncs_api.h"
#include "ncs.h"


int32
ncs_send_cfg_to_diff_tbl(cdb_cfg_t cfg, ncs_aps_group_t* p_aps_group, cdb_oper_t oper, char* cmd)
{
    int32 rc = 0;
    
    if (1 == p_aps_group->group_mode)
    {
        rc = ncs_sock_cfg_sync(cfg, oper, CDB_NODE_TBL, TBL_LSP_APS_GROUP, cmd, sal_strlen(cmd)+1);
    }
    else if (0 == p_aps_group->group_mode)
    {
        rc = ncs_sock_cfg_sync(cfg, oper, CDB_NODE_TBL, TBL_PW_APS_GROUP, cmd, sal_strlen(cmd)+1);
    }

    return rc;
}

int32
ncs_api_oam_add_aps_group(cdb_cfg_t cfg, ncs_aps_group_t* p_aps_group)
{
    char cmd[CMD_BUF_256];

    sal_memset(cmd, 0, sizeof(cmd));

    /*cdbctl create/cdb/openflow/lsp_aps_group/${lsp_aps_group_id}*/
    sal_snprintf(cmd, CMD_BUF_256, "%d", p_aps_group->group_id);

    return ncs_send_cfg_to_diff_tbl(cfg, p_aps_group, CDB_OPER_ADD, cmd);
}

int32
ncs_api_oam_delete_aps_group(cdb_cfg_t cfg, ncs_aps_group_t* p_aps_group)
{
    char cmd[CMD_BUF_256];

    sal_memset(cmd, 0, sizeof(cmd));

    /*cdbctl delete/cdb/openflow/lsp_aps_group/${lsp_aps_group_id}*/
    sal_snprintf(cmd, CMD_BUF_256, "%d", p_aps_group->group_id);

    return ncs_send_cfg_to_diff_tbl(cfg, p_aps_group, CDB_OPER_DEL, cmd);   
}

int32
ncs_api_oam_set_g8131_mode(cdb_cfg_t cfg, ncs_aps_group_t * p_aps_group)
{
    char cmd[CMD_BUF_256];

    sal_memset(cmd, 0, sizeof(cmd));

    if (1 == p_aps_group->g8131_mode)
    {
        /*cdbctl update/cdb/openflow/lsp_aps_group/${lsp_aps_group_id}/is_revertive/0*/
        sal_snprintf(cmd, CMD_BUF_256, "%d/is_revertive/0", p_aps_group->group_id);
    }
    else if (0 == p_aps_group->g8131_mode)
    {
        /*cdbctl update/cdb/openflow/lsp_aps_group/${lsp_aps_group_id}/is_revertive/1*/
        sal_snprintf(cmd, CMD_BUF_256, "%d/is_revertive/1", p_aps_group->group_id);
    }

    return ncs_send_cfg_to_diff_tbl(cfg, p_aps_group, CDB_OPER_SET, cmd);   
}

int32
ncs_api_set_g8131_timer_hold_off(cdb_cfg_t cfg, ncs_aps_group_t* p_aps_group)
{
    char cmd[CMD_BUF_256];

    sal_memset(cmd, 0, sizeof(cmd));
    
    /*cdbctl update/cdb/openflow/lsp_aps_group/${lsp_aps_group_id}/hold_off_time/${hold_off_value}*/
    sal_snprintf(cmd, CMD_BUF_256, "%d/hold_off_time/%d", p_aps_group->group_id, p_aps_group->g8131_timer_hold_off);

    return ncs_send_cfg_to_diff_tbl(cfg, p_aps_group, CDB_OPER_SET, cmd);   
    
}

int32
ncs_api_set_g8131_timer_wtr(cdb_cfg_t cfg, ncs_aps_group_t* p_aps_group)
{
    char cmd[CMD_BUF_256];

    sal_memset(cmd, 0, sizeof(cmd));

    /*cdbctl update/cdb/openflow/lsp_aps_group/${lsp_aps_group_id}/wait_to_restore_time/${wtr_value}*/
    sal_snprintf(cmd, CMD_BUF_256, "%d/wait_to_restore_time/%d", p_aps_group->group_id, p_aps_group->g8131_timer_wait_to_restore);

    return ncs_send_cfg_to_diff_tbl(cfg, p_aps_group, CDB_OPER_SET, cmd);   
    
}


int32
ncs_api_add_sessions(cdb_cfg_t cfg, ncs_oam_session_t* p_session)
{
    char cmd[CMD_BUF_256];

    sal_memset(cmd, 0, sizeof(cmd));

    /*cdbctl add/cdb/openflow/oam_session/${session_id}*/
    sal_snprintf(cmd, CMD_BUF_256, "%d", p_session->session_id);

    return ncs_sock_cfg_sync(cfg, CDB_OPER_ADD, CDB_NODE_TBL, TBL_OAM_SESSION, cmd, sal_strlen(cmd)+1);
}

int32
ncs_api_delete_sessions(cdb_cfg_t cfg, ncs_oam_session_t * p_session)
{
    char cmd[CMD_BUF_256];

    sal_memset(cmd, 0, sizeof(cmd));

    /*cdbctl delete/cdb/openflow/oam_session/${session_id}*/
    sal_snprintf(cmd, CMD_BUF_256, "%d", p_session->session_id);

    return ncs_sock_cfg_sync(cfg, CDB_OPER_DEL, CDB_NODE_TBL, TBL_OAM_SESSION, cmd, sal_strlen(cmd)+1);
}

int32
ncs_api_set_session_description(cdb_cfg_t cfg, ncs_oam_session_t* p_session)
{
    char cmd[CMD_BUF_256];
    sal_memset(cmd, 0, sizeof(cmd));
    
    /*cdbctl update/cdb/openflow/oam_session/${session_id}/description/${description_line}*/
    sal_snprintf(cmd, CMD_BUF_256, "%d/description/%s", p_session->session_id, p_session->session_description);
    
    return ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_OAM_SESSION, cmd, sal_strlen(cmd)+1);
}

int32
ncs_api_set_megid(cdb_cfg_t cfg, ncs_oam_session_t* p_session)
{
    char cmd[CMD_BUF_256];
    sal_memset(cmd, 0, sizeof(cmd));
    
    /*cdbctl update/cdb/openflow/oam_session/${session_id}/megid_str/${meg_id}/icclen/${icclen_value}/level/${level_value}*/
    sal_snprintf(cmd, CMD_BUF_256, "%d/megid_str/%s/icclen/level", p_session->session_id, p_session->megid);
    
    return ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_OAM_SESSION, cmd, sal_strlen(cmd)+1);
}

int32
ncs_api_set_mepid(cdb_cfg_t cfg, ncs_oam_session_t* p_session)
{
    char cmd[CMD_BUF_256];
    sal_memset(cmd, 0, sizeof(cmd));

    /*cdbctl update/cdb/openflow/oam_session/${session_id}/mepid/${mep_id}/interval/${interval_value}/exp/${exp_value}*/
    sal_snprintf(cmd, CMD_BUF_256, "%d/mepid/%d/interval/%d/exp", p_session->session_id, p_session->mepid, p_session->cc_interval);

    return ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_OAM_SESSION, cmd, sal_strlen(cmd)+1);
}

int32
ncs_api_set_rmepid(cdb_cfg_t cfg, ncs_oam_session_t* p_session)
{
    char cmd[CMD_BUF_256];
    sal_memset(cmd, 0, sizeof(cmd));

    /*cdbctl update/cdb/openflow/oam_session/${session_id}/rmepid/${rmep_id}*/
    sal_snprintf(cmd, CMD_BUF_256, "%d/rmepid/%d", p_session->session_id, p_session->rmepid);

    return ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_OAM_SESSION, cmd, sal_strlen(cmd)+1);
}

int32
ncs_api_set_cc_config(cdb_cfg_t cfg, ncs_oam_session_t* p_session)
{
    char cmd[CMD_BUF_256];
    sal_memset(cmd, 0, sizeof(cmd));

    if (1 == p_session->cc_config)
    {
        /*cdbctl update/cdb/openflow/oam_session/${session_id}/cc_enable/1*/
        sal_snprintf(cmd, CMD_BUF_256, "%d/cc_enable/1", p_session->session_id);
    }
    else if (0 == p_session->cc_config)
    {
        /*cdbctl update/cdb/openflow/oam_session/${session_id}/cc_enable/0*/
        sal_snprintf(cmd, CMD_BUF_256, "%d/cc_enable/0", p_session->session_id);
    }

    return ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_OAM_SESSION, cmd, sal_strlen(cmd)+1);
    
}

int32
ncs_api_set_dm_config(cdb_cfg_t cfg, ncs_oam_session_t* p_session)
{
    char cmd[CMD_BUF_256];
    sal_memset(cmd, 0, sizeof(cmd));

    if (1 == p_session->dm_config)
    {
        /*cdbctl update/cdb/openflow/oam_session/${session_id}/dm_enable/1/interval/${part2}*/
        sal_snprintf(cmd, CMD_BUF_256, "%d/dm_enable/1/interval/%s", p_session->session_id, p_session->dm_interval);
    }
    else if (0 == p_session->dm_config)
    {
        /*cdbctl update/cdb/openflow/oam_session/${session_id}/dm_enable/0*/
        sal_snprintf(cmd, CMD_BUF_256, "%d/dm_enable/0", p_session->session_id);
    }

    return ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_OAM_SESSION, cmd, sal_strlen(cmd)+1);
}

int32
ncs_api_set_dm_threshold(cdb_cfg_t cfg, ncs_oam_session_t* p_session)
{
    char cmd[CMD_BUF_256];
    sal_memset(cmd, 0, sizeof(cmd));

    /*cdbctl update/cdb/openflow/oam_session/${session_id}/oam_dm/${threshold_value}*/
    sal_snprintf(cmd, CMD_BUF_256, "%d/oam_dm/%d", p_session->session_id, p_session->dm_threshold);

    return ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_OAM_SESSION, cmd, sal_strlen(cmd)+1);
    
}

int32
ncs_api_set_lm_config(cdb_cfg_t cfg, ncs_oam_session_t* p_session)
{
    char cmd[CMD_BUF_256];
    sal_memset(cmd, 0, sizeof(cmd));

    if (1 == p_session->lm_config)
    {
        /*cdbctl update/cdb/openflow/oam_session/${session_id}/lm_enable/2/all-exp/all-exp/stats-interval/1/cache-size/${cache_size_value}*/
        sal_snprintf(cmd, CMD_BUF_256, "%d/lm_enable/2/all-exp/all-exp/stats-interval/1/cache-size/", p_session->session_id);
    }
    else if (0 == p_session->lm_config)
    {
        /*cdbctl update/cdb/openflow/oam_session/${session_id}/lm_enable/0*/
        sal_snprintf(cmd, CMD_BUF_256, "%d/lm_enable/0", p_session->session_id);
    }

    return ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_OAM_SESSION, cmd, sal_strlen(cmd)+1);
}

int32
ncs_api_set_lm_local_threshold(cdb_cfg_t cfg, ncs_oam_session_t* p_session)
{
    char cmd[CMD_BUF_256];
    sal_memset(cmd, 0, sizeof(cmd));

    /*cdbctl update/cdb/openflow/oam_session/${session_id}/oam_lm/${part1}/${threshold_value}*/
    sal_snprintf(cmd, CMD_BUF_256, "%d/oam_lm/local/%d", p_session->session_id, p_session->lm_local_threshold);

    return ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_OAM_SESSION, cmd, sal_strlen(cmd)+1);
}

int32
ncs_api_set_lm_remote_threshold(cdb_cfg_t cfg, ncs_oam_session_t* p_session)
{
    char cmd[CMD_BUF_256];
    sal_memset(cmd, 0, sizeof(cmd));

    /*cdbctl update/cdb/openflow/oam_session/${session_id}/oam_lm/${part1}/${threshold_value}*/
    sal_snprintf(cmd, CMD_BUF_256, "%d/oam_lm/remote/%d", p_session->session_id, p_session->lm_remote_threshold);

    return ncs_sock_cfg_sync(cfg, CDB_OPER_SET, CDB_NODE_TBL, TBL_OAM_SESSION, cmd, sal_strlen(cmd)+1);
}

