#include "proto.h"

#include "gen/tbl_cfm_global_define.h"
#include "gen/tbl_cfm_global.h"
#include "gen/tbl_cfm_md_define.h"
#include "gen/tbl_cfm_md.h"
#include "gen/tbl_cfm_ma_define.h"
#include "gen/tbl_cfm_ma.h"

#include "cdb_const.h"
#include "cfg_cmd.h"
#include "ccs_cfg.h"

int32
ccs_cfg_build_cfm_global(FILE * fp, cfg_cmd_para_t * para)
{
    tbl_cfm_global_t *g_p_master = tbl_cfm_global_get_cfm_global();
    uint32               has_config = FALSE;

    if (g_p_master->cfm_mode)
    {
        sal_fprintf(fp, "ethernet cfm mode y1731\n");
        has_config = TRUE;
    }

    if (has_config) 
    {
        sal_fprintf(fp, "!\n");
    }

    if (g_p_master->cfm_enable)
    {
        sal_fprintf(fp, "ethernet cfm enable\n");
        has_config = TRUE;
    }
    
    if (has_config) 
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_cfm_md(FILE * fp, cfg_cmd_para_t * para)
{
    tbl_cfm_md_master_t  *g_p_master = tbl_cfm_md_get_master();
    tbl_cfm_md_t              *p_md = NULL;
    ctclib_slistnode_t         *listnode = NULL;
    tbl_cfm_ma_master_t  *g_p_master_ma = tbl_cfm_ma_get_master();
    tbl_cfm_ma_t              *p_ma = NULL;
    ctclib_slistnode_t         *listnode_ma = NULL;

    CTCLIB_SLIST_LOOP(g_p_master->cfm_md_list, p_md, listnode)
    {
        if (0 == p_md->pkt_name_length)
        { 
            if (GLB_CFM_NO_MD_NAME == p_md->name_type)
            {
                sal_fprintf(fp, "ethernet cfm domain %s level %d format no-md-name\n", p_md->key.name, p_md->level);    
            }
            else
            {
                sal_fprintf(fp, "ethernet cfm domain %s level %d\n", p_md->key.name, p_md->level);
            }
        }
        else
        {
            if (GLB_CFM_MD_CHAR_STRING == p_md->name_type)
            {
                sal_fprintf(fp, "ethernet cfm domain %s level %d format string %s\n", p_md->key.name, p_md->level, p_md->pkt_name_input);        
            }
            else if (GLB_CFM_DNS_NAME == p_md->name_type)
            {
                sal_fprintf(fp, "ethernet cfm domain %s level %d format dns %s\n", p_md->key.name, p_md->level, p_md->pkt_name_input);        
            }
            else if (GLB_CFM_MAC_ADDR_TWO_OCTET_INTEGER == p_md->name_type)
            {
                sal_fprintf(fp, "ethernet cfm domain %s level %d format mac-address %s\n", p_md->key.name, p_md->level, p_md->pkt_name_input);
            }
        }

        CTCLIB_SLIST_LOOP(g_p_master_ma->cfm_ma_list, p_ma, listnode_ma)
        {
            if (0 == sal_strcmp((char*)p_ma->key.name, p_md->key.name))
            {
                if (0 != p_ma->key.vlan_id)
                {
                    if (0 == sal_strlen((char*)p_ma->pkt_name))
                    { 
                        sal_fprintf(fp, " service %s vlan %d\n", p_ma->name, p_ma->key.vlan_id);
                    }
                    else
                    {
                        if (GLB_CFM_MA_CHAR_STRING == p_ma->name_type)
                        {
                            sal_fprintf(fp, " service %s vlan %d format string %s\n", p_ma->name, p_ma->key.vlan_id, p_ma->pkt_name);
                        }
                        else
                        {
                            sal_fprintf(fp, " service %s vlan %d format icc-based %s\n", p_ma->name, p_ma->key.vlan_id, p_ma->pkt_name);    
                        }
                    }
                }
                else
                {
                     if (0 == sal_strlen((char*)p_ma->pkt_name))
                    { 
                        sal_fprintf(fp, " service %s\n", p_ma->name);
                    }
                    else
                    {
                        if (GLB_CFM_MA_CHAR_STRING == p_ma->name_type)
                        {
                            sal_fprintf(fp, " service %s format string %s\n", p_ma->name, p_ma->pkt_name);
                        }
                        else
                        {
                            sal_fprintf(fp, " service %s format icc-based %s\n", p_ma->name, p_ma->pkt_name);    
                        }
                    }   
                }
            }
        }

        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_cfm_ma(FILE * fp, cfg_cmd_para_t * para)
{
    tbl_cfm_md_master_t  *g_p_master = tbl_cfm_md_get_master();
    tbl_cfm_md_t              *p_md = NULL;
    ctclib_slistnode_t         *listnode = NULL;
    tbl_cfm_ma_master_t  *g_p_master_ma = tbl_cfm_ma_get_master();
    tbl_cfm_ma_t              *p_ma = NULL;
    ctclib_slistnode_t         *listnode_ma = NULL;
    bool                             add_line = FALSE;

    CTCLIB_SLIST_LOOP(g_p_master->cfm_md_list, p_md, listnode)
    {
        CTCLIB_SLIST_LOOP(g_p_master_ma->cfm_ma_list, p_ma, listnode_ma)
        {
            /* the ma is not belong to md */
            if (sal_strcmp(p_md->key.name, (char*)p_ma->key.name))
            {
                continue;
            }

            add_line = FALSE;

            if (0 != p_ma->key.vlan_id)
            {
                if (p_ma->cci_enabled)
                {
                    sal_fprintf(fp, "ethernet cfm cc enable domain %s vlan %d\n", p_ma->key.name, p_ma->key.vlan_id);
                    add_line = TRUE;
                }
    
                if (GLB_CFM_PRIO_MAX != p_ma->ccm_vlan_cos)
                {
                    sal_fprintf(fp, "ethernet cfm cc domain %s vlan %d priority %d\n", p_ma->key.name, p_ma->key.vlan_id, p_ma->ccm_vlan_cos);
                    add_line = TRUE;
                }
    
                if (p_ma->error_write_disable)
                {
                    sal_fprintf(fp, "no ethernet cfm errors enable domain %s vlan %d\n", p_ma->key.name, p_ma->key.vlan_id);
                    add_line = TRUE;
                }
            }
            else
            {
                if (p_ma->cci_enabled)
                {
                    sal_fprintf(fp, "ethernet cfm cc enable domain %s\n", p_ma->key.name);
                    add_line = TRUE;
                }
    
                if (p_ma->error_write_disable)
                {
                    sal_fprintf(fp, "no ethernet cfm errors enable domain %s\n", p_ma->key.name);
                    add_line = TRUE;
                }
            }

            if (add_line)
            {
                sal_fprintf(fp, "!\n");
            }
        }
    }

    return PM_E_NONE;
}


