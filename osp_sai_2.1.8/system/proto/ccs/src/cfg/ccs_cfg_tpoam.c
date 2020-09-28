#include "proto.h"

#include "gen/tbl_tpoam_global_define.h"
#include "gen/tbl_tpoam_global.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"
#include "gen/tbl_lsp_pe_define.h"
#include "gen/tbl_lsp_pe.h"
#include "gen/tbl_lsp_p_define.h"
#include "gen/tbl_lsp_p.h"
#include "gen/tbl_pw_tpe_define.h"
#include "gen/tbl_pw_tpe.h"

#include "glb_tp_oam.h"
#include "cdb_const.h"
#include "cfg_cmd.h"
#include "ccs_cfg.h"

int32
_ccs_cfg_build_tpoam_oam_session(FILE* fp, tbl_oam_session_t *p_oam_session)
{
    char                 description[TPOAM_DESCRIPTION_SIZE] = {0};
    
    if (NULL == p_oam_session)
    {
        return PM_E_NOT_EXIST;
    }
    sal_fprintf (fp, "tpoam session %d\n", p_oam_session->key.session_id);

    if (0 != sal_strcmp(p_oam_session->description, description))
    {
        sal_fprintf (fp, " description %s\n", p_oam_session->description);
    }
    
    if (0 != sal_strcmp(p_oam_session->megid_str, GLB_TPOAM_MEGID_STR_DEFAULT))
    {
        sal_fprintf (fp, " oam-y1731 megid %s\n", p_oam_session->megid_str);    
    }

    if (TPOAM_MP_TYPE_MIP == p_oam_session->mp_type)
    {
        return PM_E_NONE;
    }

    if (GLB_TP_OAM_DEFAULT_MEPID != p_oam_session->mepid || 
        (p_oam_session->oam_type != GLB_OAM_TYPE_MPLS_PW && GLB_TP_OAM_DEFAULT_CC_INTERVAL != p_oam_session->cc_interval) ||
        (p_oam_session->oam_type == GLB_OAM_TYPE_MPLS_PW && GLB_TP_OAM_DEFAULT_CC_INTERVAL_PW != p_oam_session->cc_interval))
    {
        sal_fprintf (fp, " oam-y1731 mepid %u interval %u\n", p_oam_session->mepid, p_oam_session->cc_interval);
    }

    if (GLB_TP_OAM_DEFAULT_RMEPID != p_oam_session->rmepid)
    {
        sal_fprintf (fp, " oam-y1731 rmepid %u\n", p_oam_session->rmepid);
    }
    
    if (GLB_TP_OAM_DEFAULT_CC_ENABLE_STATE != p_oam_session->cc_enable)
    {
        sal_fprintf (fp, " no oam-y1731 cc\n");
    }

    /*dm*/
    if (TPOAM_SESSION_DM_DELAY_THRESHOLD_DEFAULT != p_oam_session->oam_dm.delay_threshold)
    {
        sal_fprintf (fp, " oam-y1731 dm threshold %"PRId64"\n", p_oam_session->oam_dm.delay_threshold);
    }

    if (GLB_TP_OAM_DEFAULT_DM_ENABLE_STATE != p_oam_session->dm_enable || TPOAM_SESSION_DM_INTERVAL_DEFAULT != p_oam_session->oam_dm.para.interval)
    {
        if (FALSE == p_oam_session->dm_enable)
        {
            sal_fprintf (fp, " no oam-y1731 dm enable \n");
        }
        else
        {
            sal_fprintf (fp, " oam-y1731 dm enable interval %s \n", tpoam_desc_dm_interval_val2str_without_s(p_oam_session->oam_dm.para.interval));
        }
    }

    /*lm*/
    if (TPOAM_SESSION_LM_LOCAL_LOSS_THRESHOLD_DEFAULT != p_oam_session->oam_lm.local_loss_threshold)
    {
        sal_fprintf (fp, " oam-y1731 lm threshold local %.3f \n", (float)p_oam_session->oam_lm.local_loss_threshold / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_1000);
    }
    
    if (TPOAM_SESSION_LM_REMOTE_LOSS_THRESHOLD_DEFAULT != p_oam_session->oam_lm.remote_loss_threshold)
    {
        sal_fprintf (fp, " oam-y1731 lm threshold remote %.3f \n", (float)p_oam_session->oam_lm.remote_loss_threshold / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_1000);
    }

    if ((GLB_TP_OAM_DEFAULT_LM_ENABLE_STATE != p_oam_session->lm_enable) || TPOAM_LM_COS_TYPE_DEFAULT != p_oam_session->oam_lm.lm_dual.cos_type
        || TPOAM_LM_DUAL_STATS_INTERVAL_DEFAULT != p_oam_session->oam_lm.lm_dual.stats_interval)
    {
        if (GLB_TP_OAM_DEFAULT_LM_ENABLE_STATE != p_oam_session->lm_enable)
        {
            sal_fprintf (fp, " no oam-y1731 lm enable");
        }
        else
        {
            sal_fprintf (fp, " oam-y1731 lm enable dual-ended ");
            if (TPOAM_LM_COS_TYPE_DEFAULT == p_oam_session->oam_lm.lm_dual.cos_type)
            {
                sal_fprintf (fp, "all-exp ");
            }
            else if (TPOAM_LM_COS_TYPE_PER_COS == p_oam_session->oam_lm.lm_dual.cos_type)
            {
                sal_fprintf (fp, "per-exp ");
            }
            else
            {
                sal_fprintf (fp, "exp %d ", p_oam_session->oam_lm.lm_dual.cos_value);
            }

            sal_fprintf (fp, "stats-interval %d ", p_oam_session->oam_lm.lm_dual.stats_interval);
        }

        sal_fprintf (fp, "\n");
    }
    
    return PM_E_NONE;
}

int32 
ccs_cfg_build_tpoam_oam_session(FILE* fp, char *key, cfg_cmd_para_t *para)
{
    uint32 has_config = FALSE;
    tbl_oam_session_master_t *p_oam_session_master = tbl_oam_session_get_master();
    tbl_oam_session_t *p_oam_session = NULL;
    tbl_oam_session_key_t oam_session_key;
    ctclib_slistnode_t *listnode = NULL;
    int32 rc = 0;
    
    if (key)
    {
        sal_memset(&oam_session_key, 0, sizeof(oam_session_key));
        oam_session_key.session_id = cdb_int_str2val(key, &rc);
        if (rc < 0)
        {
            CFG_CONFLICT_RET("Can't convert %s to integer", key);
        }
        
        p_oam_session = tbl_oam_session_get_oam_session(&oam_session_key);
        if (NULL == p_oam_session)
        {
            CFG_CONFLICT_RET("Oam session %s does not exist", key);            
        }
        _ccs_cfg_build_tpoam_oam_session(fp, p_oam_session);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_oam_session_master->oam_session_list, p_oam_session, listnode)
        {
            _ccs_cfg_build_tpoam_oam_session(fp, p_oam_session);
            has_config = TRUE;
        }
    }
    
    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}


int32
ccs_cfg_build_tpoam_global(FILE * fp, cfg_cmd_para_t * para)
{
    tbl_tpoam_global_t *g_p_master = tbl_tpoam_global_get_tpoam_global();
    uint32               has_config = FALSE;
    addr_ipv4_t node_id = {0};
    char        buf[CMD_BUF_16];

    if (!g_p_master->fast_aps_en)
    {
        sal_fprintf(fp, "no mpls-tp oam-y1731 fast-aps enable\n");
        has_config = TRUE;
    }

    if (0 != g_p_master->node_id)
    {
        node_id.s_addr = g_p_master->node_id;
        sal_fprintf(fp, "mpls-tp node-id %-15s \n", cdb_addr_ipv4_val2str(buf, sizeof(buf), &node_id));
        has_config = TRUE;
    }

    if (TRUE != g_p_master->dm_enable_global)
    {
        sal_fprintf(fp, "no mpls-tp oam-y1731 dm enable\n");
        has_config = TRUE;
    }
    
    if (GLB_TPOAM_DM_CALCULATE_INTERVAL_DEFAULT != g_p_master->dm_calc_interval)
    {
        sal_fprintf(fp, "mpls-tp oam-y1731 dm interval %u\n", g_p_master->dm_calc_interval);
        has_config = TRUE;
    }

    if (GLB_TPOAM_DM_DELAY_THRESHOLD_DEFAULT != g_p_master->dm_delay_threshold)
    {
        sal_fprintf(fp, "mpls-tp oam-y1731 dm threshold %u\n", g_p_master->dm_delay_threshold);
        has_config = TRUE;
    }

    if (TRUE != g_p_master->lm_enable_global)
    {
        sal_fprintf(fp, "no mpls-tp oam-y1731 lm enable\n");
        has_config = TRUE;
    }

    if (GLB_TPOAM_LM_CALCULATE_INTERVAL_DEFAULT != g_p_master->lm_calc_interval)
    {
        sal_fprintf(fp, "mpls-tp oam-y1731 lm interval %u\n", g_p_master->lm_calc_interval);
        has_config = TRUE;
    }

    if (GLB_TPOAM_LM_LOCAL_THRESHOLD_DEFAULT != g_p_master->lm_local_threshold)
    {
        sal_fprintf(fp, "mpls-tp oam-y1731 lm threshold local %.3f\n", (float)g_p_master->lm_local_threshold/TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_1000);
        has_config = TRUE;
    }

    if (GLB_TPOAM_LM_REMOTE_THRESHOLD_DEFAULT != g_p_master->lm_remote_threshold)
    {
        sal_fprintf(fp, "mpls-tp oam-y1731 lm threshold remote %.3f\n", (float)g_p_master->lm_remote_threshold/TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_1000);
        has_config = TRUE;
    }
    
    if (has_config) 
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}


