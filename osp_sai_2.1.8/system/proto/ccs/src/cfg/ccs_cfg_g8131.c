#include "proto.h"
#include "gen/tbl_aps_group_info_define.h"
#include "gen/tbl_aps_group_info.h"
#include "gen/tbl_lsp_aps_group_define.h"
#include "gen/tbl_lsp_aps_group.h"
#include "gen/tbl_pw_aps_group_define.h"
#include "gen/tbl_pw_aps_group.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"
#include "gen/tbl_lsp_pe_define.h"
#include "gen/tbl_lsp_pe.h"
#include "gen/tbl_lsp_p_define.h"
#include "gen/tbl_lsp_p.h"
#include "gen/tbl_pw_tpe_define.h"
#include "gen/tbl_pw_tpe.h"
#include "glb_g8131_define.h"
#include "glb_tp_oam.h"
#include "cdb_const.h"
#include "cfg_cmd.h"
#include "ccs_cfg.h"

int32
_ccs_cfg_build_g8131_lsp_aps_group(FILE* fp, tbl_lsp_aps_group_t *p_lsp_apsgroup)
{
    if (NULL == p_lsp_apsgroup)
    {
        return PM_E_NOT_EXIST;
    }
    sal_fprintf (fp, "lsp-aps-group %u\n", p_lsp_apsgroup->key.group_id);
    
    /*revertive or non-revertive*/
    if (!p_lsp_apsgroup->is_revertive)
    {
        sal_fprintf (fp, " g8131 mode non-revertive\n");    
    }

    /*permanent or selector*/
    if (p_lsp_apsgroup->bridge_permanent)
    {
        sal_fprintf (fp, " g8131 switch permanent\n");
    }

    /*hold-off time and wait-to-restore time*/
    if ((p_lsp_apsgroup->is_revertive) && (G8131_DEFAULT_WTR_TIMEOUT != p_lsp_apsgroup->wait_to_restore_time))
    {
        sal_fprintf (fp, " g8131 timer wait-to-restore %u\n", p_lsp_apsgroup->wait_to_restore_time/60);
    }
    if (p_lsp_apsgroup->hold_off_time)
    {
        sal_fprintf (fp, " g8131 timer hold-off %u\n", p_lsp_apsgroup->hold_off_time);
    }
    
    return PM_E_NONE;
}

int32
_ccs_cfg_build_g8131_pw_aps_group(FILE* fp, tbl_pw_aps_group_t *p_pw_apsgroup)
{
    if (NULL == p_pw_apsgroup)
    {
        return PM_E_NOT_EXIST;
    }    
    sal_fprintf (fp, "pw-aps-group %u\n", p_pw_apsgroup->key.group_id);

    /*revertive or non-revertive*/
    if (!p_pw_apsgroup->is_revertive)
    {
        sal_fprintf (fp, " g8131 mode non-revertive\n");    
    }

    /*permanent or selector*/
    if (p_pw_apsgroup->bridge_permanent)
    {
        sal_fprintf (fp, " g8131 switch permanent\n");
    }

    /*hold-off time and wait-to-restore time*/
    if ((p_pw_apsgroup->is_revertive) && (G8131_DEFAULT_WTR_TIMEOUT != p_pw_apsgroup->wait_to_restore_time))
    {
        sal_fprintf (fp, " g8131 timer wait-to-restore %u\n", p_pw_apsgroup->wait_to_restore_time/60);
    }
    if (p_pw_apsgroup->hold_off_time)
    {
        sal_fprintf (fp, " g8131 timer hold-off %u\n", p_pw_apsgroup->hold_off_time);
    }
    
    
    return PM_E_NONE;
}


int32 
ccs_cfg_build_g8131_lsp_aps_group(FILE* fp, cfg_cmd_para_t *para)
{
    uint32 has_config = FALSE;
    tbl_lsp_aps_group_master_t *p_master_lspaps = tbl_lsp_aps_group_get_master();
    tbl_lsp_aps_group_t *p_lsp_apsgroup = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_master_lspaps->lsp_aps_group_list, p_lsp_apsgroup, listnode)
    {
        _ccs_cfg_build_g8131_lsp_aps_group(fp, p_lsp_apsgroup);
        has_config = TRUE;
    }
    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_g8131_pw_aps_group(FILE* fp, cfg_cmd_para_t *para)
{
    uint32 has_config = FALSE;
    tbl_pw_aps_group_master_t *p_master_pwaps = tbl_pw_aps_group_get_master();
    tbl_pw_aps_group_t *p_pw_apsgroup = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_master_pwaps->pw_aps_group_list, p_pw_apsgroup, listnode)
    {
        _ccs_cfg_build_g8131_pw_aps_group(fp, p_pw_apsgroup);
        has_config = TRUE;
    }
    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

