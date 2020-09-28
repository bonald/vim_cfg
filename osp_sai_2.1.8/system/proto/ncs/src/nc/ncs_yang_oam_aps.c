
#ifdef HAVE_NETCONF

#include "proto.h"
#include "ncutil.h"
#include "ncs.h"
#include "ncs_api.h"
#include "ncs_yang.h"

void
reset_field_aps_group(ncs_aps_group_t *p_aps_group)
{
    int i = 0;
    for (i = 0; i < APS_GROUP_FIELD_MAX; i++)
    {
        p_aps_group->field_op[i] = FIELD_INITIAL_VALUE;
    }
}

void
reset_field_oam_session(ncs_oam_session_t *p_oam_session)
{
    int i = 0;
    for (i = 0; i < OAM_SESSION_FIELD_MAX; i++)
    {
        p_oam_session->field_op[i] = FIELD_INITIAL_VALUE;
    }
}

int32
ncs_get_lsp_aps_and_pw_aps_count(void)
{
    tbl_lsp_aps_group_master_t *p_master1 = tbl_lsp_aps_group_get_master();
    tbl_pw_aps_group_master_t *p_master2 = tbl_pw_aps_group_get_master();
    uint32 lsp_aps_count = 0;
    uint32 pw_aps_count = 0;

    lsp_aps_count = p_master1->lsp_aps_group_list->count;
    pw_aps_count = p_master2->pw_aps_group_list->count;
    
    return lsp_aps_count + pw_aps_count;
}

int32
ncs_judge_whether_next(sr_notif_event_t ev, sr_change_oper_t op, sr_val_t *old_val, sr_val_t *new_val, int32* pre_key, char* node_name, char* key_name)
{
    int32 is_next = 0;
    int32 cur_key = 0;
    sr_xpath_ctx_t xp_ctx = {0};


    switch (op)
    {
        case SR_OP_CREATED:
            cur_key = atoi(sr_xpath_key_value(new_val->xpath, node_name, key_name, &xp_ctx));
            break;
            
        case SR_OP_DELETED:
            cur_key = atoi(sr_xpath_key_value(old_val->xpath, node_name, key_name, &xp_ctx));
            break;
            
        case SR_OP_MODIFIED:
            if (SR_EV_ABORT == ev)
            {
                cur_key = atoi(sr_xpath_key_value(old_val->xpath, node_name, key_name, &xp_ctx));
            }
            else
            {
                cur_key = atoi(sr_xpath_key_value(new_val->xpath, node_name, key_name, &xp_ctx));
            }
            break;
            
        default:
            return SR_ERR_INVAL_ARG;
                    
    }
    if ((cur_key == *pre_key) || (-1 == *pre_key))
    {
        is_next = 0;
    }
    else
    {
        is_next = 1;
    }

    *pre_key = cur_key;

    sr_xpath_recover(&xp_ctx);

    return is_next;
}

int
ncs_oam_aps_group_change_process(sr_session_ctx_t *srs,sr_notif_event_t ev, sr_change_oper_t op,
    sr_val_t *old_val, sr_val_t *new_val,  ncs_aps_group_t *p_aps_group, int32* p_aps_groups_count)
{

    int rc = SR_ERR_OK;
    sr_val_t *tmp = NULL;
    sr_xpath_ctx_t xp_ctx = {0};

    char cmd[CMD_BUF_256];
    sr_val_t* value = NULL;

    sal_memset(cmd, 0, sizeof(cmd));
    
    if (NULL != new_val)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "operation:%s, event:%s, xpath:%s\n", op==SR_OP_CREATED?"create":op==SR_OP_MODIFIED?"modify":op==SR_OP_DELETED?"delete":op==SR_OP_MOVED?"move":"unknown"
            , ev==SR_EV_VERIFY?"verify":ev==SR_EV_APPLY?"apply":ev==SR_EV_ABORT?"abort":"unknown" ,new_val->xpath);
    }

    switch(op) {
    case SR_OP_CREATED:
        if (NULL != new_val) {
           tmp = new_val;
        }
        break;
    case SR_OP_DELETED:
        if (NULL != old_val) {
            tmp = old_val;
        }
        break;
    case SR_OP_MODIFIED:
        if (NULL != old_val && NULL != new_val) 
        {
            if (SR_EV_ABORT == ev)
            {
                tmp = old_val;
            }
            else
            {
                tmp = new_val;
            }
        }
        break;
    case SR_OP_MOVED:
        return SR_ERR_UNSUPPORTED;
        
        break;
    default:
        rc = SR_ERR_INVAL_ARG;
        break;
    }

    if (NULL != tmp && (SR_OP_CREATED == op || SR_OP_DELETED == op || SR_OP_MODIFIED == op))
    {
        /*when the field is modified, the group_id and group_mode is necessary*/
        if (SR_OP_MODIFIED == op)
        {
            p_aps_group->group_id = atoi(sr_xpath_key_value(tmp->xpath, "aps-group", "group-id", &xp_ctx));
            sr_xpath_recover(&xp_ctx);

            sal_snprintf(cmd, CMD_BUF_256, "/centec-oam-aps:aps-groups/aps-group[group-id='%d']/group-mode", p_aps_group->group_id);
            if (sr_get_item(srs, cmd, &value))
            {
                sr_free_val(value);
                return SR_ERR_INVAL_ARG;
            }
            if (!sal_strcmp(value->data.enum_val, "lsp-mode"))
            {
                p_aps_group->group_mode = 1;
            }
            else if (!sal_strcmp(value->data.enum_val, "pw-mode"))
            {
                p_aps_group->group_mode = 0;
            }
            
            sal_memset(cmd, 0, sizeof(cmd));
            sr_free_val(value);
        }
        
        if (sr_xpath_node_name_eq(tmp->xpath, "group-id"))
        {         
            p_aps_group->field_op[FIELD_OP_GROUP_ID] = op;
            p_aps_group->group_id = tmp->data.uint32_val;
            if (SR_OP_CREATED == op)
            {
                (*p_aps_groups_count)++;
            }
            else if (SR_OP_DELETED == op)
            {
                (*p_aps_groups_count)--;
            }
            
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "group-mode"))
        {
            p_aps_group->field_op[FIELD_OP_GROUP_MODE] = op;
            if (0 == sal_strcmp(tmp->data.enum_val, "pw-mode"))
            {
                p_aps_group->group_mode = 0;
            }
            else if (0 == sal_strcmp(tmp->data.enum_val, "lsp-mode"))
            {
                p_aps_group->group_mode = 1;
            }
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "g8131-mode"))
        {
            p_aps_group->field_op[FIELD_OP_G8131_MODE] = op;
            if (0 == sal_strcmp(tmp->data.enum_val, "revertive"))
            {
                p_aps_group->g8131_mode= 0;
            }
            else if (0 == sal_strcmp(tmp->data.enum_val, "non-revertive"))
            {
                p_aps_group->g8131_mode = 1;
            }
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "g8131-timer-hold-off"))
        {
            p_aps_group->field_op[FIELD_OP_G8131_TIMER_HOLD_OFF] = op;
            p_aps_group->g8131_timer_hold_off = tmp->data.uint32_val;
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "g8131-timer-wait-to-restore"))
        {
            if (SR_OP_MODIFIED == op)
            {
                sal_snprintf(cmd, CMD_BUF_256, "/centec-oam-aps:aps-groups/aps-group[group-id='%d']/g8131-mode", p_aps_group->group_id);
                if (sr_get_item(srs, cmd, &value))
                {
                    sr_free_val(value);
                    return SR_ERR_INVAL_ARG;
                }
                if (!sal_strcmp(value->data.enum_val, "revertive"))
                {
                    p_aps_group->g8131_mode= 0;
                }
                else if (!sal_strcmp(value->data.enum_val, "non-revertive"))
                {
                    p_aps_group->g8131_mode= 1;
                }
            }
            
            p_aps_group->field_op[FIELD_OP_G8131_TIMER_WAIT_TO_RESTORE] = op;
            p_aps_group->g8131_timer_wait_to_restore = tmp->data.uint32_val;
        }
        
    }
    
    return rc;
}

int
ncs_api_aps_group_config(cdb_cfg_t cfg_nc, ncs_aps_group_t *p_aps_group)
{
    int rc = SR_ERR_OK;

    /*group-mode is not be modified*/
    if (SR_OP_MODIFIED == p_aps_group->field_op[FIELD_OP_GROUP_MODE])
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "Fail to add group, the aps group has already existed in other group mode!");
        return SR_ERR_INVAL_ARG;   
    }
    
    if (SR_OP_CREATED == p_aps_group->field_op[FIELD_OP_GROUP_ID])
    {        
        rc += ncs_api_oam_add_aps_group(cfg_nc, p_aps_group);
    }
    
    if (SR_OP_DELETED == p_aps_group->field_op[FIELD_OP_GROUP_ID])
    {
        rc += ncs_api_oam_delete_aps_group(cfg_nc, p_aps_group);
        if (rc < 0)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "Fail to delete group, the aps group has already removed by other way!");
            return SR_ERR_INVAL_ARG;
        }
    }
    
    if ((SR_OP_CREATED == p_aps_group->field_op[FIELD_OP_G8131_MODE]) || (SR_OP_MODIFIED == p_aps_group->field_op[FIELD_OP_G8131_MODE]))
    {
        rc += ncs_api_oam_set_g8131_mode(cfg_nc, p_aps_group);
    }
    
    if ((SR_OP_CREATED == p_aps_group->field_op[FIELD_OP_G8131_TIMER_HOLD_OFF]) || (SR_OP_MODIFIED == p_aps_group->field_op[FIELD_OP_G8131_TIMER_HOLD_OFF]))
    {  
        rc += ncs_api_set_g8131_timer_hold_off(cfg_nc, p_aps_group);
    }
    
    if ((SR_OP_CREATED == p_aps_group->field_op[FIELD_OP_G8131_TIMER_WAIT_TO_RESTORE]) || (SR_OP_MODIFIED == p_aps_group->field_op[FIELD_OP_G8131_TIMER_WAIT_TO_RESTORE]))
    {
        
        /*If the g8131 mode is non-revertive, the wait-to-restore time is not allowed to configure*/
        if (0 == p_aps_group->g8131_mode)
        {
            rc += ncs_api_set_g8131_timer_wtr(cfg_nc, p_aps_group);
        }
        else
        {
            /*if the g8131 mode is non-revertive, the wtr-time is not care*/
            return SR_ERR_OK;       
        }
    }

    return rc;
                
}

int
ncs_oam_aps_group_list_change_cb(sr_session_ctx_t *srs, const char *xpath, 
    sr_notif_event_t event, void *private_ctx)
{
    int rc = SR_ERR_OK;
    sr_change_iter_t *it = NULL;
    sr_val_t *new_val = NULL;
    sr_val_t *old_val = NULL;
    uint8 is_end = 0;
    uint8 is_next = 0;
    sr_change_oper_t op;
    ncs_aps_group_t  aps_group;
    ncs_aps_group_t  copy_aps_group;
    ncs_aps_group_t * p_aps_group = NULL;
    int32 pre_key = -1;
    int32 aps_groups_count = 0;
    uint32 cur_aps_groups_count = 0;
    
    sal_memset(&aps_group, 0, sizeof(ncs_aps_group_t));
    sal_memset(&copy_aps_group, 0, sizeof(ncs_aps_group_t));
    reset_field_aps_group(&aps_group);
    
    rc = ctc_nc_get_changes_iter(srs, "/centec-oam-aps:aps-groups/aps-group", &it);

    if (SR_ERR_OK != rc) {
        goto err_out;        
    }

    while (!is_end)
    {
        rc = ctc_nc_get_change_next(srs, it, &op, &old_val, &new_val);
                        
        if (SR_ERR_OK == rc)
        {
            is_next = ncs_judge_whether_next(event, op, old_val, new_val, &pre_key, "aps-group", "group-id");
            if (is_next)
            {
                sal_memcpy(&copy_aps_group, &aps_group, sizeof(ncs_aps_group_t));
                sal_memset(&aps_group, 0, sizeof(ncs_aps_group_t));
                reset_field_aps_group(&aps_group);
            }          
            rc = ncs_oam_aps_group_change_process(srs, event, op, old_val, new_val, &aps_group, &aps_groups_count);
        }
        else
        {
            is_end = 1;
            rc = SR_ERR_OK;
        }

        if (SR_ERR_OK != rc) 
        {
            goto err_out;
        }
        if (NULL != old_val) 
        {
            sr_free_val(old_val);
            old_val = NULL;
        }
        if (NULL != new_val) {
            sr_free_val(new_val);
            new_val = NULL;
        }

        if (is_end || is_next)
        {
            if (is_next)
            {
                p_aps_group = &copy_aps_group;

                /*set 0 after using is_next flag*/
                is_next = 0;
            }
            else 
            {
                p_aps_group = &aps_group;
            }
            
            if (SR_EV_VERIFY == event)
            {
                cur_aps_groups_count = ncs_get_lsp_aps_and_pw_aps_count();
                if (cur_aps_groups_count + aps_groups_count <= G8131_APS_MAXIMUM_COUNT)
                {
                    rc = ncs_api_aps_group_config(CDB_CFG_NC_CHECK, p_aps_group);
                    if (rc)
                    {
                        rc = SR_ERR_INVAL_ARG;
                        goto err_out;
                    }                
                }
                else
                {
                    log_sys(M_MOD_INTERNAL, E_ERROR, "Fail to add group, only at most 500 aps groups are supported, current groups count is %d!", cur_aps_groups_count);
                    return SR_ERR_INVAL_ARG;
                }
            }
            else if (SR_EV_APPLY == event)
            {
                rc = ncs_api_aps_group_config(CDB_CFG_NC_SET, p_aps_group);
                if (rc)
                {
                    rc = SR_ERR_INVAL_ARG;
                    goto err_out;
                }
            }  
            
        }
              
    }

err_out:
    if (NULL != old_val) {
        sr_free_val(old_val);
        old_val = NULL;
    }
    if (NULL != new_val) {
        sr_free_val(new_val);
        new_val = NULL;
    }
    if (NULL != it) {
        sr_free_change_iter(it);
    }

    return rc;
}

int
ncs_oam_session_change_process(sr_session_ctx_t *srs, sr_notif_event_t ev, sr_change_oper_t op,
    sr_val_t *old_val, sr_val_t *new_val,  ncs_oam_session_t *p_oam_session)
{
    int rc = SR_ERR_OK;
    sr_val_t *tmp = NULL;
    sr_xpath_ctx_t xp_ctx = {0};

    if (NULL != new_val)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "operation:%s, event:%s, xpath:%s\n", op==SR_OP_CREATED?"create":op==SR_OP_MODIFIED?"modify":op==SR_OP_DELETED?"delete":op==SR_OP_MOVED?"move":"unknown"
            , ev==SR_EV_VERIFY?"verify":ev==SR_EV_APPLY?"apply":ev==SR_EV_ABORT?"abort":"unknown" ,new_val->xpath);
    }

    switch(op) {
    case SR_OP_CREATED:
        if (NULL != new_val) {
           tmp = new_val;
        }
        break;
    case SR_OP_DELETED:
        if (NULL != old_val) {
            tmp = old_val;
        }
        break;
    case SR_OP_MODIFIED:
        if (NULL != old_val && NULL != new_val) 
        {
            if (SR_EV_ABORT == ev)
            {
                tmp = old_val;
            }
            else
            {
                tmp = new_val;
            }
        }
        break;
    case SR_OP_MOVED:
        return SR_ERR_UNSUPPORTED;
        
        break;
    default:
        rc = SR_ERR_INVAL_ARG;
        break;
    }

    if (NULL != tmp && (SR_OP_CREATED == op || SR_OP_DELETED == op || SR_OP_MODIFIED == op))
    {
        if (SR_OP_MODIFIED == op)
        {            
            p_oam_session->session_id = atoi(sr_xpath_key_value(tmp->xpath, "oam-session", "session-id", &xp_ctx));
            sr_xpath_recover(&xp_ctx);
        }
        if (sr_xpath_node_name_eq(tmp->xpath, "session-id"))
        {
            p_oam_session->field_op[FIELD_OP_SESSION_ID] = op;
            p_oam_session->session_id = tmp->data.uint32_val;
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "session-description"))
        {
            p_oam_session->field_op[FIELD_OP_SESSION_DESCRIPTION] = op;
            sal_strcpy(p_oam_session->session_description, tmp->data.string_val);
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "megid"))
        {
            p_oam_session->field_op[FIELD_OP_MEGID] = op;
            sal_strcpy(p_oam_session->megid, tmp->data.string_val);
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "mepid"))
        {
            p_oam_session->field_op[FIELD_OP_MEPID] = op;
            p_oam_session->mepid = tmp->data.uint32_val;
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "cc-interval"))
        {
            p_oam_session->field_op[FIELD_OP_CC_INTERVAL] = op;
            if (0 == sal_strcmp(tmp->data.enum_val, "cc-interval-3.3ms"))
            {
                p_oam_session->cc_interval = 1;
            }
            else if (0 == sal_strcmp(tmp->data.enum_val, "cc-interval-10ms"))
            {
                p_oam_session->cc_interval = 2;
            }
            else if (0 == sal_strcmp(tmp->data.enum_val, "cc-interval-100ms"))
            {
                p_oam_session->cc_interval = 3;
            }
            else if (0 == sal_strcmp(tmp->data.enum_val, "cc-interval-1s"))
            {
                p_oam_session->cc_interval = 4;
            }
            else if (0 == sal_strcmp(tmp->data.enum_val, "cc-interval-10s"))
            {
                p_oam_session->cc_interval = 5;
            }
            else if (0 == sal_strcmp(tmp->data.enum_val, "cc-interval-1min"))
            {
                p_oam_session->cc_interval = 6;
            }
            else if (0 == sal_strcmp(tmp->data.enum_val, "cc-interval-10min"))
            {
                p_oam_session->cc_interval = 7;
            }
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "rmepid"))
        {
            p_oam_session->field_op[FIELD_OP_RMEPID] = op;
            p_oam_session->rmepid = tmp->data.uint32_val;
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "cc-config"))
        {
            p_oam_session->field_op[FIELD_OP_CC_CONFIG] = op;
            if (0 == sal_strcmp(tmp->data.enum_val, "cc-enable"))
            {
                p_oam_session->cc_config = 1;
            }
            else if (0 == sal_strcmp(tmp->data.enum_val, "cc-disable"))
            {
                p_oam_session->cc_config = 0;
            }
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "dm-config"))
        {
            p_oam_session->field_op[FIELD_OP_DM_CONFIG] = op;
            if (0 == sal_strcmp(tmp->data.enum_val, "dm-enable"))
            {
                p_oam_session->dm_config = 1;
            }
            else if (0 == sal_strcmp(tmp->data.enum_val, "dm-disable"))
            {
                p_oam_session->dm_config = 0;
            }
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "dm-interval"))
        {
            p_oam_session->field_op[FIELD_OP_DM_INTERVAL] = op;
            if (0 == sal_strcmp(tmp->data.enum_val, "dm-interval-1s"))
            {
                sal_strcpy(p_oam_session->dm_interval, "1s");
            }
            else if (0 == sal_strcmp(tmp->data.enum_val, "dm-interval-10s"))
            {
                sal_strcpy(p_oam_session->dm_interval, "10s");
            }
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "dm-threshold"))
        {
            p_oam_session->field_op[FIELD_OP_DM_THRESHOLD] = op;
            p_oam_session->dm_threshold = tmp->data.uint32_val;
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "lm-config"))
        {
            p_oam_session->field_op[FIELD_OP_LM_CONFIG] = op;
            if (0 == sal_strcmp(tmp->data.enum_val, "lm-enable"))
            {
                p_oam_session->lm_config = 1;
            }
            else if (0 == sal_strcmp(tmp->data.enum_val, "lm-disable"))
            {
                p_oam_session->lm_config = 0;
            }
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "lm-local-threshold"))
        {
            p_oam_session->field_op[FIELD_OP_LM_LOCAL_THRESHOLD] = op;
            p_oam_session->lm_local_threshold = tmp->data.uint32_val;
        }
        else if (sr_xpath_node_name_eq(tmp->xpath, "lm-remote-threshold"))
        {
            p_oam_session->field_op[FIELD_OP_LM_REMOTE_THRESHOLD] = op;
            p_oam_session->lm_remote_threshold = tmp->data.uint32_val;
        }
        
    }
    return rc;
}

int
ncs_api_oam_session_config(sr_session_ctx_t *srs, cdb_cfg_t cfg_nc, ncs_oam_session_t *p_oam_session)
{
    int rc = SR_ERR_OK;
    char cmd[CMD_BUF_256];
    sr_val_t*  value = NULL;

    sal_memset(cmd, 0, sizeof(cmd));

    if (SR_OP_CREATED == p_oam_session->field_op[FIELD_OP_SESSION_ID])
    {
        rc += ncs_api_add_sessions(cfg_nc, p_oam_session);
    }
    if (SR_OP_DELETED == p_oam_session->field_op[FIELD_OP_SESSION_ID])
    {
        rc += ncs_api_delete_sessions(cfg_nc, p_oam_session);
        if (rc < 0)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "Fail to delete OAM session, the OAM session has already removed by other way!");
            return SR_ERR_INVAL_ARG;
        }
    }
    if ((SR_OP_CREATED == p_oam_session->field_op[FIELD_OP_SESSION_DESCRIPTION]) || (SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_SESSION_DESCRIPTION]))
    {
        rc += ncs_api_set_session_description(cfg_nc, p_oam_session);
    }
    if ((SR_OP_CREATED == p_oam_session->field_op[FIELD_OP_MEGID]) || (SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_MEGID]))
    {
        rc += ncs_api_set_megid(cfg_nc, p_oam_session);
    }
    
    /*two leaf field control one cdbctl command   ---start*/
    if ((SR_OP_CREATED == p_oam_session->field_op[FIELD_OP_MEPID]))
    {        
        rc += ncs_api_set_mepid(cfg_nc, p_oam_session);
    }
    if ((SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_MEPID]) && (SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_CC_INTERVAL]))
    {
        rc += ncs_api_set_mepid(cfg_nc, p_oam_session);
    }
    else if (SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_MEPID])
    {
        sal_snprintf(cmd, CMD_BUF_256, "/centec-oam-aps:oam-sessions/oam-session[session-id='%d']/cc-interval", p_oam_session->session_id);
        if (sr_get_item(srs, cmd, &value))
        {
            sr_free_val(value);
            return SR_ERR_INVAL_ARG;
        }
        if (!sal_strcmp(value->data.enum_val, "cc-interval-3.3ms"))
        {
            p_oam_session->cc_interval = 1;
        }
        else if (!sal_strcmp(value->data.enum_val, "cc-interval-10ms"))
        {
            p_oam_session->cc_interval = 2;
        }
        else if (!sal_strcmp(value->data.enum_val, "cc-interval-100ms"))
        {
            p_oam_session->cc_interval = 3;
        }
        else if (!sal_strcmp(value->data.enum_val, "cc-interval-1s"))
        {
            p_oam_session->cc_interval = 4;
        }
        else if (!sal_strcmp(value->data.enum_val, "cc-interval-10s"))
        {
            p_oam_session->cc_interval = 5;
        }
        else if (!sal_strcmp(value->data.enum_val, "cc-interval-1min"))
        {
            p_oam_session->cc_interval = 6;
        }
        else if (!sal_strcmp(value->data.enum_val, "cc-interval-10min"))
        {
            p_oam_session->cc_interval = 7;
        }
        sr_free_val(value);

        rc += ncs_api_set_mepid(cfg_nc, p_oam_session);
        
    }
    else if (SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_CC_INTERVAL])
    {
        sal_snprintf(cmd, CMD_BUF_256, "/centec-oam-aps:oam-sessions/oam-session[session-id='%d']/mepid", p_oam_session->session_id);
        if (sr_get_item(srs, cmd, &value))
        {
            sr_free_val(value);
            return SR_ERR_INVAL_ARG;
        }
        p_oam_session->mepid = value->data.uint32_val;

        rc += ncs_api_set_mepid(cfg_nc, p_oam_session);
    }
    /*---end*/
    
    if ((SR_OP_CREATED == p_oam_session->field_op[FIELD_OP_RMEPID]) || (SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_RMEPID]))
    {
        rc += ncs_api_set_rmepid(cfg_nc, p_oam_session);
    }
    if ((SR_OP_CREATED == p_oam_session->field_op[FIELD_OP_CC_CONFIG]) || (SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_CC_CONFIG]))
    {
        rc += ncs_api_set_cc_config(cfg_nc, p_oam_session);
    }

    /*two leaf field control one cdbctl command   ---start*/
    if (SR_OP_CREATED == p_oam_session->field_op[FIELD_OP_DM_CONFIG])
    {
        rc += ncs_api_set_dm_config(cfg_nc, p_oam_session);
    }
    if ((SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_DM_CONFIG]) && (SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_DM_INTERVAL]))
    {
        rc += ncs_api_set_dm_config(cfg_nc, p_oam_session);
    }
    else if (SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_DM_CONFIG])
    {
        if (1 == p_oam_session->dm_config)
        {
            sal_memset(cmd, 0, sizeof(cmd));
            sal_snprintf(cmd, CMD_BUF_256, "/centec-oam-aps:oam-sessions/oam-session[session-id='%d']/dm-interval", p_oam_session->session_id);
            if (sr_get_item(srs, cmd, &value))
            {
                sr_free_val(value);
                return SR_ERR_INVAL_ARG;
            }

            if (0 == sal_strcmp(value->data.enum_val, "dm-interval-1s"))
            {
                sal_strcpy(p_oam_session->dm_interval, "1s");
            }
            else if (0 == sal_strcmp(value->data.enum_val, "dm-interval-10s"))
            {
                sal_strcpy(p_oam_session->dm_interval, "10s");
            }
            sr_free_val(value);
        }
        else if (0 == p_oam_session->dm_config)
        {
            
        }
              
        rc += ncs_api_set_dm_config(cfg_nc, p_oam_session);
    }
    else if (SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_DM_INTERVAL])
    {
        sal_memset(cmd, 0, sizeof(cmd));
        sal_snprintf(cmd, CMD_BUF_256, "/centec-oam-aps:oam-sessions/oam-session[session-id='%d']/dm-config", p_oam_session->session_id);
        if (sr_get_item(srs, cmd, &value))
        {
            sr_free_val(value);
            return SR_ERR_INVAL_ARG;
        }
        if (!sal_strcmp(value->data.enum_val, "dm-enable"))
        {
            p_oam_session->dm_config = 1;
        }
        else if (!sal_strcmp(value->data.enum_val, "dm-disable"))
        {
            p_oam_session->dm_config = 0;
        }
        
        sr_free_val(value);
        
        rc += ncs_api_set_dm_config(cfg_nc, p_oam_session);
    }
    /*---end*/
    
    if ((SR_OP_CREATED == p_oam_session->field_op[FIELD_OP_DM_THRESHOLD]) || (SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_DM_THRESHOLD]))
    {
        rc += ncs_api_set_dm_threshold(cfg_nc, p_oam_session);
    }
    if ((SR_OP_CREATED == p_oam_session->field_op[FIELD_OP_LM_CONFIG]) || (SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_LM_CONFIG]))
    {
        rc += ncs_api_set_lm_config(cfg_nc, p_oam_session);
    }
    if ((SR_OP_CREATED == p_oam_session->field_op[FIELD_OP_LM_LOCAL_THRESHOLD]) || (SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_LM_LOCAL_THRESHOLD]))
    {
        rc += ncs_api_set_lm_local_threshold(cfg_nc, p_oam_session);
    }
    if ((SR_OP_CREATED == p_oam_session->field_op[FIELD_OP_LM_REMOTE_THRESHOLD]) || (SR_OP_MODIFIED == p_oam_session->field_op[FIELD_OP_LM_REMOTE_THRESHOLD]))
    {
        rc += ncs_api_set_lm_remote_threshold(cfg_nc, p_oam_session);
    }
    return rc;
}

int
ncs_oam_aps_session_list_change_cb(sr_session_ctx_t *srs, const char *xpath, 
    sr_notif_event_t event, void *private_ctx)
{
    int rc = SR_ERR_OK;
    sr_change_iter_t *it = NULL;
    sr_val_t *new_val = NULL;
    sr_val_t *old_val = NULL;
    uint8 is_end = 0;
    uint8 is_next = 0;
    sr_change_oper_t op;
    ncs_oam_session_t  oam_session;
    ncs_oam_session_t  copy_oam_session;
    ncs_oam_session_t * p_oam_session = NULL;
    int32 pre_key = -1;

    sal_memset(&oam_session, 0, sizeof(ncs_oam_session_t));
    sal_memset(&copy_oam_session, 0, sizeof(ncs_oam_session_t));
    reset_field_oam_session(&oam_session);

    rc = ctc_nc_get_changes_iter(srs, "/centec-oam-aps:oam-sessions/oam-session", &it);

    if (SR_ERR_OK != rc) {
        goto err_out;        
    }

    while (!is_end)
    {
        rc = ctc_nc_get_change_next(srs, it, &op, &old_val, &new_val);
                        
        if (SR_ERR_OK == rc)
        {
            is_next = ncs_judge_whether_next(event, op, old_val, new_val, &pre_key, "oam-session", "session-id");
            if (is_next)
            {
                sal_memcpy(&copy_oam_session, &oam_session, sizeof(ncs_oam_session_t));
                sal_memset(&oam_session, 0, sizeof(ncs_oam_session_t));
                reset_field_oam_session(&oam_session);
            }          
            rc = ncs_oam_session_change_process(srs, event, op, old_val, new_val, &oam_session);
        }
        else
        {
            is_end = 1;
            rc = SR_ERR_OK;
        }

        if (SR_ERR_OK != rc) 
        {
            goto err_out;
        }
        if (NULL != old_val) 
        {
            sr_free_val(old_val);
            old_val = NULL;
        }
        if (NULL != new_val) {
            sr_free_val(new_val);
            new_val = NULL;
        }

        if (is_end || is_next)
        {
            if (is_next)
            {
                p_oam_session= &copy_oam_session;

                /*set 0 after using is_next flag*/
                is_next = 0;
            }
            else 
            {
                p_oam_session= &oam_session;
            }
            
            if (SR_EV_VERIFY == event)
            {
                rc = ncs_api_oam_session_config(srs, CDB_CFG_NC_CHECK, p_oam_session);
                if (rc)
                {
                    rc = SR_ERR_INVAL_ARG;
                    goto err_out;
                }
            }
            else if (SR_EV_APPLY == event)
            {
                rc = ncs_api_oam_session_config(srs, CDB_CFG_NC_SET, p_oam_session);
                if (rc)
                {
                    rc = SR_ERR_INVAL_ARG;
                    goto err_out;
                }
            }            
        }
    }

err_out:

    if (NULL != old_val) {
        sr_free_val(old_val);
        old_val = NULL;
    }
    if (NULL != new_val) {
        sr_free_val(new_val);
        new_val = NULL;
    }
    if (NULL != it) {
        sr_free_change_iter(it);
    }
    
    return rc;
}

#endif
