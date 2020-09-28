
#include "proto.h"
#include "gen/tbl_tap_group_define.h"
#include "gen/tbl_tap_group.h"
#include "gen/tbl_tap_group_ingress_define.h"
#include "gen/tbl_tap_group_ingress.h"
#include "gen/tbl_tap_group_ingress_flow_define.h"
#include "gen/tbl_tap_group_ingress_flow.h"
#include "gen/tbl_tap_group_egress_define.h"
#include "gen/tbl_tap_group_egress.h"
#include "gen/tbl_tap_five_tuple_define.h"
#include "gen/tbl_tap_five_tuple.h"
#include "gen/tbl_tap_dpi_global_define.h"
#include "gen/tbl_tap_dpi_global.h"
#include "glb_tap_define.h"
#include "lib_acl.h"

int32
__build_tap_group_ingress_in_tap_group(tbl_tap_group_ingress_t *ptap_group_ingress, 
                                        tbl_iter_args_t *pargs)
{
    FILE*   cfg_fp          = pargs->argv[0];
    char*   tap_group_name  = pargs->argv[1];
    char ifname_ext[IFNAME_EXT_SIZE];
    char    buf[CMD_BUF_256];

    if (0 != sal_strcmp(tap_group_name, ptap_group_ingress->key.tap_group))
    {
        return 0;
    }

    IFNAME_ETH2FULL(ptap_group_ingress->key.if_name, ifname_ext);
    sal_fprintf (cfg_fp, " ingress %s", ifname_ext);

    if (ptap_group_ingress->ing_mark_vlan_en)
    {
        sal_fprintf (cfg_fp, " mark-source %d", ptap_group_ingress->ing_mark_vlan_vid);
    }

    if (ptap_group_ingress->ing_untag_en)
    {
        sal_fprintf (cfg_fp, " un-tag");
    }

    if (ptap_group_ingress->ing_trunction_en)
    {
        sal_fprintf (cfg_fp, " truncation");
    }

    /* show edit-mac/ip */
    if(ptap_group_ingress->ing_edit_dest_mac_en){
        sal_fprintf (cfg_fp, " edit-macda %.04hX.%.04hX.%.04hX",
            sal_ntoh16(((uint16*)ptap_group_ingress->ing_edit_dest_mac)[0]),
            sal_ntoh16(((uint16*)ptap_group_ingress->ing_edit_dest_mac)[1]), 
            sal_ntoh16(((uint16*)ptap_group_ingress->ing_edit_dest_mac)[2]));
    }

    if(ptap_group_ingress->ing_edit_src_mac_en){
        sal_fprintf (cfg_fp, " edit-macsa %.04hX.%.04hX.%.04hX",
            sal_ntoh16(((uint16*)ptap_group_ingress->ing_edit_src_mac)[0]),
            sal_ntoh16(((uint16*)ptap_group_ingress->ing_edit_src_mac)[1]), 
            sal_ntoh16(((uint16*)ptap_group_ingress->ing_edit_src_mac)[2]));
    }

    if(ptap_group_ingress->ing_edit_vlan_en){
        sal_fprintf (cfg_fp, " edit-vlan %d", ptap_group_ingress->ing_edit_vlan);
    }

    if(ptap_group_ingress->ing_edit_ipda_en){
        if (AF_INET == ptap_group_ingress->ing_edit_ipda.family)
        {
            sal_fprintf (cfg_fp, " edit-ipda %s",
                cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &ptap_group_ingress->ing_edit_ipda.u.prefix4));
        }
        else if (AF_INET6 == ptap_group_ingress->ing_edit_ipda.family)
        {
            sal_fprintf (cfg_fp, " edit-ipv6da %s",
                cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &ptap_group_ingress->ing_edit_ipda.u.prefix6));
        }
    }

    if(ptap_group_ingress->ing_edit_ipsa_en){
        if (AF_INET == ptap_group_ingress->ing_edit_ipsa.family)
        {
            sal_fprintf (cfg_fp, " edit-ipsa %s",
                cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &ptap_group_ingress->ing_edit_ipsa.u.prefix4));
        }
        else if (AF_INET6 == ptap_group_ingress->ing_edit_ipsa.family)
        {
            sal_fprintf (cfg_fp, " edit-ipv6sa %s",
                cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &ptap_group_ingress->ing_edit_ipsa.u.prefix6));
        }
    }

    sal_fprintf (cfg_fp, "\n");
    return PM_E_NONE;
}

int32
__build_tap_group_ingress_flow_in_tap_group(tbl_tap_group_ingress_flow_t *ptap_group_ingress_flow, 
                                        tbl_iter_args_t *pargs)
{
    FILE*   cfg_fp          = pargs->argv[0];
    char*   tap_group_name  = pargs->argv[1];
    char ifname_ext[IFNAME_EXT_SIZE];

    if(0 != sal_strcmp(tap_group_name, ptap_group_ingress_flow->key.tap_group))
    {
        return 0;
    }

    IFNAME_ETH2FULL(ptap_group_ingress_flow->key.if_name, ifname_ext);
    sal_fprintf (cfg_fp, " ingress %s flow %s", ifname_ext, 
            acl_get_acl_config_key_name(ptap_group_ingress_flow->key.flow_name, GLB_ACL_CONFIG_TYPE_FLOW));

    if(ptap_group_ingress_flow->ing_mark_vlan_en)
    {
        sal_fprintf (cfg_fp, " mark-source %d", ptap_group_ingress_flow->ing_mark_vlan_vid);
    }

    if (ptap_group_ingress_flow->ing_untag_en)
    {
        sal_fprintf (cfg_fp, " un-tag");
    }

    sal_fprintf (cfg_fp, "\n");
    return PM_E_NONE;
}



int32
__build_tap_group_egress_in_tap_group(
                                        tbl_tap_group_egress_t *ptap_group_egress, 
                                        tbl_iter_args_t *pargs)
{
    FILE*   cfg_fp          = pargs->argv[0];
    char*   tap_group_name  = pargs->argv[1];
    char ifname_ext[IFNAME_EXT_SIZE];

    if(0 != sal_strcmp(tap_group_name, ptap_group_egress->key.tap_group)){
        return 0;
    }

    IFNAME_ETH2FULL(ptap_group_egress->key.if_name, ifname_ext);
    sal_fprintf (cfg_fp, " egress %s", ifname_ext);

    if(ptap_group_egress->timestamp_en){
        sal_fprintf (cfg_fp, " timestamp");
    }

    sal_fprintf (cfg_fp, "\n");
    return PM_E_NONE;
}

int32
_ccs_cfg_build_tap_group(tbl_iter_args_t *pargs)
{
    FILE* cfg_fp = pargs->argv[0];
    tbl_tap_group_t **ptapgroup_arry = pargs->argv[1];
    uint32 type = *((uint32*)pargs->argv[2]);
    tbl_iter_args_t args;
    uint32          idx = 0;

    for(idx = 0; idx < GLB_TAP_GROUP_MAX; idx++)     
    {
        if(ptapgroup_arry[idx] == NULL)
        {
            continue;
        }

        if((GLB_TAP_GROUP_TYPE_NORMAL == type && 
            GLB_IS_BIT_SET(ptapgroup_arry[idx]->flags, GLB_TAP_FLAGS_ASSIGN_ID)) || 
           (GLB_TAP_GROUP_TYPE_ASSIGN_ID == type && 
            !GLB_IS_BIT_SET(ptapgroup_arry[idx]->flags, GLB_TAP_FLAGS_ASSIGN_ID))){
            continue;
        }
#if 0
        if(GLB_IS_BIT_SET(ptapgroup_arry[idx]->flags, GLB_TAP_FLAGS_ASSIGN_ID))
        {
            sal_fprintf (cfg_fp, "tap-group %s %d\n", ptapgroup_arry[idx]->key.name,
                    ptapgroup_arry[idx]->id);
        }else{
            sal_fprintf (cfg_fp, "tap-group %s\n", ptapgroup_arry[idx]->key.name);
        }
#else
        sal_fprintf (cfg_fp, "tap-group %s %d\n", ptapgroup_arry[idx]->key.name,
                    ptapgroup_arry[idx]->id);
#endif
        if (sal_strlen(ptapgroup_arry[idx]->desc))
        {
            sal_fprintf (cfg_fp, " description %s\n", ptapgroup_arry[idx]->desc);
        }
        
        args.argv[0] = cfg_fp;
        args.argv[1] = (void *)(ptapgroup_arry[idx]->key.name);
        tbl_tap_group_ingress_iterate((TBL_ITER_CB_FUNC)__build_tap_group_ingress_in_tap_group, &args);
        tbl_tap_group_ingress_flow_iterate((TBL_ITER_CB_FUNC)__build_tap_group_ingress_flow_in_tap_group, &args);
        tbl_tap_group_egress_iterate((TBL_ITER_CB_FUNC)__build_tap_group_egress_in_tap_group, &args);

        sal_fprintf (cfg_fp, "!\n");
    }

    return PM_E_NONE;
}

static int32
_build_tap_get_all_tap_group(tbl_tap_group_t *ptap_group, tbl_iter_args_t *pargs)
{
    tbl_tap_group_t **ptapgroup_arry = pargs->argv[1];
    
    sal_assert(ptap_group->id <= GLB_TAP_GROUP_MAX && ptap_group->id >= 0);

    ptapgroup_arry[ptap_group->id - 1] = ptap_group;

    return PM_E_NONE;
}

int32
ccs_cfg_build_tap(FILE* fp, char *key, cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;
    static tbl_tap_group_t **ptapgroup_arry = NULL;
    uint32 type = GLB_TAP_GROUP_TYPE_ALL;

    if(NULL == ptapgroup_arry)
    {
        ptapgroup_arry = XCALLOC(MEM_PM_TEMP, GLB_TAP_GROUP_MAX * sizeof(tbl_tap_group_t*));
        if(NULL == ptapgroup_arry)
        {
            return 0;
        }
    }

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = fp;
    args.argv[1] = ptapgroup_arry;

    sal_memset(ptapgroup_arry, 0, GLB_TAP_GROUP_MAX * sizeof(tbl_tap_group_t*));

    tbl_tap_group_iterate((TBL_ITER_CB_FUNC)_build_tap_get_all_tap_group, &args);
    args.argv[2] = &type;
    _ccs_cfg_build_tap_group(&args);
    return PM_E_NONE;
}


int32
ccs_cfg_build_tapdpi(FILE* fp, char *key, cfg_cmd_para_t *para)
{ 
    uint32                has_config    = FALSE;
    tbl_tap_dpi_global_t *p_tap_dpi_glb = NULL;
    tbl_tap_five_tuple_t *p_db_tap_5tuple = NULL;
    tbl_tap_five_tuple_master_t *tuple_master = tbl_tap_five_tuple_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    char ipsa[16] = {0};
    char ipda[16] = {0};

    p_tap_dpi_glb = tbl_tap_dpi_global_get_tap_dpi_global();
    if (!p_tap_dpi_glb)
    {
        CFG_CONFLICT_RET("Not found tbl_tap_dpi_global");
    }

    if (p_tap_dpi_glb->dpi_action)
    {
        sal_fprintf(fp, "tap-dpi action redirect %s\n", IFNAME_ETH2FULL(p_tap_dpi_glb->redir_if, ifname_ext));
        has_config = TRUE;
    }

    CTCLIB_SLIST_LOOP_DEL(tuple_master->tap_five_tuple_list, p_db_tap_5tuple, listnode, next)
    {
        if (p_db_tap_5tuple)
        {
            sal_memset(ipsa, 0, sizeof(ipsa));
            sal_memset(ipda, 0, sizeof(ipda));
            sal_inet_ntop(AF_INET, (void *)&p_db_tap_5tuple->key.ip_sa, ipsa, sizeof(ipsa));
            sal_inet_ntop(AF_INET, (void *)&p_db_tap_5tuple->key.ip_da, ipda, sizeof(ipda));
            sal_fprintf(fp, "tap-dpi entry ip-da %s ip-sa %s l4-dst-port %d l4-src-port %d l4-protocol %d\n", 
                ipda, ipsa, p_db_tap_5tuple->key.l4_dst_port, p_db_tap_5tuple->key.l4_src_port, p_db_tap_5tuple->key.l4_protocol);
        }
        has_config = TRUE;
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

