#include "proto.h"
#include "lib_tblinc.h"
#include "lib_sysctl.h"
#include "route.h"
#include "arp_define.h"
#include "glb_l2_define.h"
#include "cdb_const.h"
#include "glb_acl_define.h"
#include "glb_arpacl_define.h"
#include "arpacl.h"
#include "arpinsp.h"


int32
arpacl_check_ace_config(tbl_arpace_config_t *p_arpace_config, tbl_iter_args_t *pargs)
{
    tbl_arpace_config_t *pst_new_arpace = pargs->argv[0];
    
    if (0 != sal_strcmp(p_arpace_config->key.arpacl_name, pst_new_arpace->key.arpacl_name))
    {
        return PM_E_NONE;
    }
    if ((p_arpace_config->arp_type == pst_new_arpace->arp_type) &&
        (((p_arpace_config->src_mac[0] & (0xff - p_arpace_config->src_mac_mask[0])) ==
        (pst_new_arpace->src_mac[0] & (0xff - pst_new_arpace->src_mac_mask[0]))) &&
        ((p_arpace_config->src_mac[1] & (0xff - p_arpace_config->src_mac_mask[1]))== 
        (pst_new_arpace->src_mac[1] & (0xff - pst_new_arpace->src_mac_mask[1]))) &&
        ((p_arpace_config->src_mac[2] & (0xff - p_arpace_config->src_mac_mask[2]))== 
        (pst_new_arpace->src_mac[2] & (0xff - pst_new_arpace->src_mac_mask[2]))) &&
        ((p_arpace_config->src_mac[3] & (0xff - p_arpace_config->src_mac_mask[3]))== 
        (pst_new_arpace->src_mac[3] & (0xff - pst_new_arpace->src_mac_mask[3]))) &&
        ((p_arpace_config->src_mac[4] & (0xff - p_arpace_config->src_mac_mask[4]))== 
        (pst_new_arpace->src_mac[4] & (0xff - pst_new_arpace->src_mac_mask[4]))) &&
        ((p_arpace_config->src_mac[5] & (0xff - p_arpace_config->src_mac_mask[5]))== 
        (pst_new_arpace->src_mac[5] & (0xff - pst_new_arpace->src_mac_mask[5]))) &&
        0 == sal_memcmp(p_arpace_config->src_mac_mask, pst_new_arpace->src_mac_mask, sizeof(pst_new_arpace->src_mac_mask)))&&
        ((p_arpace_config->src_ip.s_addr & ~(p_arpace_config->src_ip_mask.s_addr)) == 
        (pst_new_arpace->src_ip.s_addr & ~(pst_new_arpace->src_ip_mask.s_addr)) &&
        p_arpace_config->src_ip_mask.s_addr == pst_new_arpace->src_ip_mask.s_addr))
    {
        pargs->argv[1] = p_arpace_config;
    }
    
    return PM_E_NONE;
}

bool
arpacl_config_is_used(char *arpacl_name)
{
    tbl_arpinsp_t * p_db_arpinsp = NULL;
    tbl_arpinsp_key_t arpinsp_key;
    uint32 i = 0;
    bool is_used = FALSE;

    for (i = GLB_DEF_VLAN_ID; i< GLB_MAX_VLAN_ID; i++)
    {
        sal_memset(&arpinsp_key, 0, sizeof(tbl_arpinsp_key_t));
        arpinsp_key.vlan_id = i;
        p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
        if (NULL == p_db_arpinsp)
        {
            continue;
        }
        if (0 == sal_strcmp(arpacl_name, p_db_arpinsp->filter))
        {
            is_used = TRUE;
            return is_used;
        }
    }
    return is_used;
}

int32
arpacl_config_clear_arpace(char *arpacl_name)
{
    tbl_arpace_config_master_t* p_arpace_master = tbl_arpace_config_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_arpace_config_t *p_db_arpace_config = NULL;
    int rc = PM_E_NONE;

    CTCLIB_SLIST_LOOP_DEL(p_arpace_master->arpace_config_list, p_db_arpace_config, listnode, next)
    {
        if ((0 == sal_strcmp(arpacl_name, p_db_arpace_config->key.arpacl_name)))
        {
            rc = tbl_arpace_config_del_arpace_config(&(p_db_arpace_config->key));  
            if (rc < 0)
            {
                return rc;
            }
        }
    }
    return rc;
}

int32
arpace_get_new_seq_ref(tbl_arpace_config_t *p_arpace_config, tbl_iter_args_t *pargs)
{
    char *arpacl_name = pargs->argv[0];
    uint32 *seq_no = pargs->argv[1];
    uint32 *new_seq_no = pargs->argv[2];

    if (0 == sal_strcmp(arpacl_name, p_arpace_config->key.arpacl_name))
    {
        if (p_arpace_config->key.seq_no > *new_seq_no && 
            p_arpace_config->key.seq_no < *seq_no)
        {
            *new_seq_no = p_arpace_config->key.seq_no;
        }
    }

    return PM_E_NONE;
}

int32
arpacl_config_add_arpacl(char *arpacl_name)
{
    int32 rc = PM_E_NONE;
    tbl_arpacl_config_t *p_arpacl_config = NULL;
    tbl_arpacl_config_t arpacl_config;
    
    sal_memset(&arpacl_config, 0, sizeof(tbl_arpacl_config_t));
    sal_strncpy(arpacl_config.key.name, arpacl_name, ARPACL_NAME_SIZE);
    p_arpacl_config = tbl_arpacl_config_get_arpacl_config(&(arpacl_config.key));
    if (NULL != p_arpacl_config)
    {
        return PM_E_EXIST;
    }
    PM_E_RETURN(tbl_arpacl_config_add_arpacl_config(&arpacl_config));
    return rc;
}

int32
arpacl_config_del_arpacl(char *arpacl_name)
{
    int32 rc = PM_E_NONE;
    tbl_arpacl_config_t *p_arpacl_config = NULL;
    tbl_arpacl_config_key_t arpacl_config_key;
    
    sal_memset(&arpacl_config_key, 0, sizeof(tbl_arpacl_config_key_t));
    sal_strncpy(arpacl_config_key.name, arpacl_name, ARPACL_NAME_SIZE);
    p_arpacl_config = tbl_arpacl_config_get_arpacl_config(&arpacl_config_key);
    if (NULL == p_arpacl_config)
    {
        return PM_E_NOT_EXIST;
    }
    PM_E_RETURN(tbl_arpacl_config_del_arpacl_config(&arpacl_config_key));
    return rc;
}

int32
arpace_config_add_arpace(tbl_arpace_config_t *p_arpace_config)
{
    int32 rc = PM_E_NONE;
    tbl_arpace_config_key_t arpace_config_key;
    tbl_arpace_config_t *p_db_arpace_config = NULL;

    sal_memset(&arpace_config_key, 0, sizeof(tbl_arpace_config_key_t));
    sal_strcpy(arpace_config_key.arpacl_name, p_arpace_config->key.arpacl_name);
    arpace_config_key.seq_no = p_arpace_config->key.seq_no;
    p_db_arpace_config = tbl_arpace_config_get_arpace_config(&arpace_config_key);
    if (p_db_arpace_config)
    {
        return PM_E_EXIST;
    }
    PM_E_RETURN(tbl_arpace_config_add_arpace_config(p_arpace_config));
    return rc;
}

int32
arpace_config_del_arpace(char *arpacl_name, uint32 p_seq_no)
{
    int32 rc = PM_E_NONE;
    tbl_arpace_config_key_t arpace_config_key;
    tbl_arpace_config_t *p_db_arpace_config = NULL;

    sal_memset(&arpace_config_key, 0, sizeof(tbl_arpace_config_key_t));
    sal_strcpy(arpace_config_key.arpacl_name, arpacl_name);
    arpace_config_key.seq_no = p_seq_no;
    p_db_arpace_config = tbl_arpace_config_get_arpace_config(&arpace_config_key);
    if (NULL == p_db_arpace_config)
    {
        return PM_E_NOT_EXIST;
    }
    PM_E_RETURN(tbl_arpace_config_del_arpace_config(&arpace_config_key));
    return rc;
}
int32
arpacl_start(route_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_ARPACL_CONFIG, arpacl_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_ARPACE_CONFIG, arpace_cmd_process);
    cdb_register_cfg_act_cb(ACT_SHOW_ARPACL, arpacl_cmd_process_show);
    return PM_E_NONE;
}

int32
arpacl_stop(route_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_ARPACL_CONFIG, NULL);
    cdb_register_cfg_tbl_cb(TBL_ARPACE_CONFIG, NULL);
    return PM_E_NONE;
}

