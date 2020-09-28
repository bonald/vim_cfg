#include "proto.h"
#include "lib_tblinc.h"
#include "glb_if_define.h"
#include "glb_l2_define.h"
#include "glb_hash_field_value_define.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_hash_field_profile_define.h" 
#include "gen/tbl_hash_field_profile.h" 
#include "gen/tbl_hash_value_profile_define.h" 
#include "gen/tbl_hash_value_profile.h" 
#include "gen/tbl_hash_value_global_define.h" 
#include "gen/tbl_hash_value_global.h" 
#include "gen/tbl_class_map_config_define.h" 
#include "gen/tbl_class_map_config.h" 
#include "gen/tbl_brg_global_define.h" 
#include "gen/tbl_brg_global.h" 
#include "gen/tbl_interface_define.h" 
#include "gen/tbl_interface.h" 
#include "gen/tbl_vlan_define.h" 
#include "switch.h"
#include "acl.h"
#include "hash_field_profile.h"
#include "hash_value_profile_and_global.h"
#include "if_agg.h"
#include "if_mgr.h"
#include "hsrv_msg.h"
#include "hsrv_msg_if.h"
#include "ctclib_opb.h"

extern ctclib_opb_t    hash_field_pro_index_opb;

int32
hash_field_profile_cmd_set_description(tbl_hash_field_profile_t* p_hash_fld_pro, tbl_hash_field_profile_t* p_db_hash_fld_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (0 == sal_memcmp(p_hash_fld_pro->description, p_db_hash_fld_pro->description, sizeof(p_hash_fld_pro->description)))
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_field_profile_set_description(p_hash_fld_pro));
    return rc;
}

int32
hash_field_profile_cmd_set_l2_field_value(tbl_hash_field_profile_t* p_hash_fld_pro, tbl_hash_field_profile_t* p_db_hash_fld_pro, cfg_cmd_para_t *para)
{   
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_fld_pro->l2_set == p_db_hash_fld_pro->l2_set)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_field_profile_set_l2_field_value(p_hash_fld_pro));
    return rc;
}

int32
hash_field_profile_cmd_set_ip_field_value(tbl_hash_field_profile_t* p_hash_fld_pro, tbl_hash_field_profile_t* p_db_hash_fld_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_fld_pro->ip_set == p_db_hash_fld_pro->ip_set)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_field_profile_set_ip_field_value(p_hash_fld_pro));
    return rc;
}

int32
hash_field_profile_cmd_set_ipv6_field_value(tbl_hash_field_profile_t* p_hash_fld_pro, tbl_hash_field_profile_t* p_db_hash_fld_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_fld_pro->ipv6_set == p_db_hash_fld_pro->ipv6_set)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_field_profile_set_ipv6_field_value(p_hash_fld_pro));
    return rc;
}

int32
hash_field_profile_cmd_set_gre_field_value(tbl_hash_field_profile_t* p_hash_fld_pro, tbl_hash_field_profile_t* p_db_hash_fld_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_fld_pro->gre_set == p_db_hash_fld_pro->gre_set)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_field_profile_set_gre_field_value(p_hash_fld_pro));
    return rc;
}

int32
hash_field_profile_cmd_set_l3vpn_field_value(tbl_hash_field_profile_t* p_hash_fld_pro, tbl_hash_field_profile_t* p_db_hash_fld_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_fld_pro->l3vpn_set == p_db_hash_fld_pro->l3vpn_set)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_field_profile_set_l3vpn_field_value(p_hash_fld_pro));
    return rc;
}

int32
hash_field_profile_cmd_set_nvgre_field_value(tbl_hash_field_profile_t* p_hash_fld_pro, tbl_hash_field_profile_t* p_db_hash_fld_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_fld_pro->nvgre_set == p_db_hash_fld_pro->nvgre_set)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_field_profile_set_nvgre_field_value(p_hash_fld_pro));
    return rc;
}


int32
hash_field_profile_cmd_set_mpls_field_value(tbl_hash_field_profile_t* p_hash_fld_pro, tbl_hash_field_profile_t* p_db_hash_fld_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_fld_pro->mpls_set == p_db_hash_fld_pro->mpls_set)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_field_profile_set_mpls_field_value(p_hash_fld_pro));
    return rc;
}

int32
hash_field_profile_cmd_set_vpws_field_value(tbl_hash_field_profile_t* p_hash_fld_pro, tbl_hash_field_profile_t* p_db_hash_fld_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_fld_pro->vpws_set == p_db_hash_fld_pro->vpws_set)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_field_profile_set_vpws_field_value(p_hash_fld_pro));
    return rc;
}

int32
hash_field_profile_cmd_set_vpls_field_value(tbl_hash_field_profile_t* p_hash_fld_pro, tbl_hash_field_profile_t* p_db_hash_fld_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_fld_pro->vpls_set == p_db_hash_fld_pro->vpls_set)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_field_profile_set_vpls_field_value(p_hash_fld_pro));
    return rc;
}

int32
hash_field_profile_cmd_set_vxlan_field_value(tbl_hash_field_profile_t* p_hash_fld_pro, tbl_hash_field_profile_t* p_db_hash_fld_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_fld_pro->vxlan_set == p_db_hash_fld_pro->vxlan_set)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_field_profile_set_vxlan_field_value(p_hash_fld_pro));
    return rc;
}

int32
hash_field_profile_cmd_set_hash_arithmetic(tbl_hash_field_profile_t* p_hash_fld_pro, tbl_hash_field_profile_t* p_db_hash_fld_pro, cfg_cmd_para_t *para)
{   
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_fld_pro->hash_arithmetic == p_db_hash_fld_pro->hash_arithmetic)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_field_profile_set_hash_arithmetic(p_hash_fld_pro));
    return rc;
}

int32
hash_field_profile_cmd_set_mode_symmetry(tbl_hash_field_profile_t* p_hash_fld_pro, tbl_hash_field_profile_t* p_db_hash_fld_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_fld_pro->mode_symmetry == p_db_hash_fld_pro->mode_symmetry)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_field_profile_set_mode_symmetry(p_hash_fld_pro));
    return rc;
}

int32
hash_field_profile_cmd_set_bitmap_disable(tbl_hash_field_profile_t* p_hash_fld_pro, tbl_hash_field_profile_t* p_db_hash_fld_pro, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_hash_fld_pro->bitmap_disable == p_db_hash_fld_pro->bitmap_disable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hash_field_profile_set_bitmap_disable(p_hash_fld_pro));
    return rc;
}

int32
hash_field_profile_cmd_get_hash_field_profile(char *hash_profile_name, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_hash_field_profile_key_t hash_fld_pro_key;
    tbl_hash_field_profile_t* p_db_hash_fld_pro = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (NULL == hash_profile_name)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_hash_field_profile_dump_one, &args);
    }
    else
    {
        sal_memset(&hash_fld_pro_key, 0, sizeof(hash_fld_pro_key));
        sal_strncpy(hash_fld_pro_key.name, hash_profile_name, HASH_FIELD_PROFILE_NAME_SIZE);
        p_db_hash_fld_pro = tbl_hash_field_profile_get_hash_field_profile(&hash_fld_pro_key);
        if (NULL == p_db_hash_fld_pro)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("Hash field %s does not exist", hash_profile_name);
        }        
        tbl_hash_field_profile_dump_one(p_db_hash_fld_pro, &args);
    }
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}


void
hash_field_profile_show_hash_field_segment(FILE* fp, uint32 *p_counter)
{
    if (HASH_SHOW_SEG_NUM == *p_counter)
    {
        sal_fprintf(fp, "\n");
        *p_counter = 0;
        sal_fprintf(fp, "                     ");
    }
}

void
hash_field_profile_show_hash_field_l2(FILE* fp, uint32 l2_set)
{
    uint32 counter = 0;
    
    sal_fprintf(fp, " %-20s", "l2:");
    if (l2_set & GLB_HASH_PROFILE_L2_SRC_MAC)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "macsa");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (l2_set & GLB_HASH_PROFILE_L2_DST_MAC)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "macda");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (l2_set & GLB_HASH_PROFILE_L2_VLAN)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "vlan");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (l2_set & GLB_HASH_PROFILE_L2_ETH_TYPE)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "eth-type");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (l2_set & GLB_HASH_PROFILE_L2_SRC_INTERFACE)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "src-interface");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (counter != 0)
    {
        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "\n");
    }
    else
    {
        sal_fprintf(fp, "\n");
    }
}

void
hash_field_profile_show_hash_field_ip(FILE* fp, uint32 ip_set)
{
    uint32 counter = 0;
    
    sal_fprintf(fp, " %-20s", "ip:");
    if (ip_set & GLB_HASH_PROFILE_IP_SRC_IP)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "ipsa");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (ip_set & GLB_HASH_PROFILE_IP_DST_IP)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "ipda");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (ip_set & GLB_HASH_PROFILE_IP_SRC_PORT)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "l4-sourceport");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (ip_set & GLB_HASH_PROFILE_IP_DST_PORT)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "l4-destport");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    } 
    if (ip_set & GLB_HASH_PROFILE_IP_IP_PROTOCOL)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "ip-protocol");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (ip_set & GLB_HASH_PROFILE_IP_SRC_INTERFACE)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "src-interface");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (counter != 0)
    {
        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "\n");
    }
    else
    {
        sal_fprintf(fp, "\n");
    }
}

void
hash_field_profile_show_hash_field_ipv6(FILE* fp, uint32 ipv6_set)
{
    uint32 counter = 0;
    
    sal_fprintf(fp, " %-20s","ipv6:");
    if (ipv6_set & GLB_HASH_PROFILE_IP_SRC_IP)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "ipsa");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (ipv6_set & GLB_HASH_PROFILE_IP_DST_IP)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "ipda");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (ipv6_set & GLB_HASH_PROFILE_IP_SRC_PORT)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "l4-sourceport");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (ipv6_set & GLB_HASH_PROFILE_IP_DST_PORT)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "l4-destport");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    } 
    if (ipv6_set & GLB_HASH_PROFILE_IP_IP_PROTOCOL)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "ip-protocol");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (ipv6_set & GLB_HASH_PROFILE_IP_SRC_INTERFACE)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "src-interface");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (counter != 0)
    {
        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "\n");
    }
    else
    {
        sal_fprintf(fp, "\n");
    }
}

void
hash_field_profile_show_hash_field_gre(FILE* fp, uint32 gre_set)
{
    uint32 counter = 0;
    
    sal_fprintf(fp, " %-20s","gre:");
    if (gre_set & GLB_HASH_PROFILE_GRE_SRC_IP)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "ipsa");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (gre_set & GLB_HASH_PROFILE_GRE_DST_IP)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "ipda");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (gre_set & GLB_HASH_PROFILE_GRE_KEY)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "gre-key");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (gre_set & GLB_HASH_PROFILE_GRE_PROTOCOL)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "gre-protocol");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (gre_set & GLB_HASH_PROFILE_GRE_SRC_INTERFACE)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "src-interface");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (counter != 0)
    {
        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "\n");
    }
    else
    {
        sal_fprintf(fp, "\n");
    }
}

void
hash_field_profile_show_hash_field_vxlan(FILE* fp, uint32 vxlan_set)
{
    uint32 counter = 0;
    
    sal_fprintf(fp, " %-20s","vxlan:");
    if (vxlan_set & GLB_HASH_PROFILE_VXLAN_SRC_INTERFACE)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "src-interface");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (vxlan_set & GLB_HASH_PROFILE_VXLAN_VNI)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "vni");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    
    if (vxlan_set & GLB_HASH_PROFILE_VXLAN_OUTER_BIT)
    {
        if (vxlan_set & GLB_HASH_PROFILE_VXLAN_OUTER_VLAN)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "outer-vlan");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vxlan_set & GLB_HASH_PROFILE_VXLAN_OUTER_DST_PORT)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "outer-l4-destport");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vxlan_set & GLB_HASH_PROFILE_VXLAN_OUTER_SRC_PORT)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "outer-l4-sourceport");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vxlan_set & GLB_HASH_PROFILE_VXLAN_OUTER_DST_IP)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "outer-ipda");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vxlan_set & GLB_HASH_PROFILE_VXLAN_OUTER_SRC_IP)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "outer-ipsa");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
    }
    
    if (vxlan_set & GLB_HASH_PROFILE_VXLAN_L2IN_BIT)
    {
        if (vxlan_set & GLB_HASH_PROFILE_VXLAN_INNER_ETH_TYPE)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner2-eth-type");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vxlan_set & GLB_HASH_PROFILE_VXLAN_INNER_DST_MAC)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner2-macda");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vxlan_set & GLB_HASH_PROFILE_VXLAN_INNER_SRC_MAC)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner2-macsa");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
    }

    if (vxlan_set & GLB_HASH_PROFILE_VXLAN_L3IN_BIT)
    {
        if (vxlan_set & GLB_HASH_PROFILE_VXLAN_INNER_DST_IP)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner3-ipda");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vxlan_set & GLB_HASH_PROFILE_VXLAN_INNER_SRC_IP)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner3-ipsa");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vxlan_set & GLB_HASH_PROFILE_VXLAN_INNER_IP_PROTOCOL)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner3-ip-protocol");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vxlan_set & GLB_HASH_PROFILE_VXLAN_INNER_DST_PORT)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner3-l4-destport");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vxlan_set & GLB_HASH_PROFILE_VXLAN_INNER_SRC_PORT)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner3-l4-sourceport");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
    }

    if (counter != 0)
    {
        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "\n");
    }
    else
    {
        sal_fprintf(fp, "\n");
    }
}

void
hash_field_profile_show_hash_field_nvgre(FILE* fp, uint32 nvgre_set)
{
    uint32 counter = 0;
    
    sal_fprintf(fp, " %-20s","nvgre:");
    if (nvgre_set & GLB_HASH_PROFILE_NVGRE_SRC_INTERFACE)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "src-interface");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (nvgre_set & GLB_HASH_PROFILE_NVGRE_VSID)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "vsid");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    
    if (nvgre_set & GLB_HASH_PROFILE_NVGRE_OUT_BIT)
    {
        if (nvgre_set & GLB_HASH_PROFILE_NVGRE_OUTER_GRE_PROTOCOL)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "outer-gre-protocol");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (nvgre_set & GLB_HASH_PROFILE_NVGRE_OUTER_DST_IP)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "outer-ipda");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (nvgre_set & GLB_HASH_PROFILE_NVGRE_OUTER_SRC_IP)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "outer-ipsa");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
    }
    
    if (nvgre_set & GLB_HASH_PROFILE_NVGRE_L2IN_BIT)
    {
        if (nvgre_set & GLB_HASH_PROFILE_NVGRE_INNER_ETH_TYPE)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner2-eth-type");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (nvgre_set & GLB_HASH_PROFILE_NVGRE_INNER_DST_MAC)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner2-macda");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (nvgre_set & GLB_HASH_PROFILE_NVGRE_INNER_SRC_MAC)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner2-macsa");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
    }

    if (nvgre_set & GLB_HASH_PROFILE_NVGRE_L3IN_BIT)
    {
        if (nvgre_set & GLB_HASH_PROFILE_NVGRE_INNER_DST_IP)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner3-ipda");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (nvgre_set & GLB_HASH_PROFILE_NVGRE_INNER_SRC_IP)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner3-ipsa");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (nvgre_set & GLB_HASH_PROFILE_NVGRE_INNER_IP_PROTOCOL)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner3-ip-protocol");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
    }

    if (counter != 0)
    {
        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "\n");
    }
    else
    {
        sal_fprintf(fp, "\n");
    }
}

void
hash_field_profile_show_hash_field_mpls(FILE* fp, uint32 mpls_set)
{
    uint32 counter = 0;
    
    sal_fprintf(fp, " %-20s","mpls:");
    if (mpls_set & GLB_HASH_PROFILE_MPLS_TOP_LABEL)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "top-label");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (mpls_set & GLB_HASH_PROFILE_MPLS_2ND_LABEL)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "2nd-label");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (mpls_set & GLB_HASH_PROFILE_MPLS_3RD_LABEL)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "3rd-label");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    
/*spec is not currently supported*/
#if 0/*added by wanh for bug 54157, 2019-11-09*/
    if (mpls_set & GLB_HASH_PROFILE_MPLS_DST_IP)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "inner-ipda");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (mpls_set & GLB_HASH_PROFILE_MPLS_SRC_IP)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "inner-ipsa");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
#endif/*wanh end*/ 

    if (mpls_set & GLB_HASH_PROFILE_MPLS_SRC_INTERFACE)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "source-interface");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }

    if (counter != 0)
    {
        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "\n");
    }
    else
    {
        sal_fprintf(fp, "\n");
    }
}

/*added by yejl for bug 54080, 2019-10-24*/
/*
void
hash_field_profile_show_hash_field_vpws(FILE* fp, uint32 vpws_set)
{
    uint32 counter = 0;
    
    sal_fprintf(fp, " %-20s","vpws:");
    if (vpws_set & GLB_HASH_PROFILE_VPWS_TOP_LABEL)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "top-label");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (vpws_set & GLB_HASH_PROFILE_VPWS_2ND_LABEL)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "2nd-label");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (vpws_set & GLB_HASH_PROFILE_VPWS_3RD_LABEL)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "3rd-label");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (vpws_set & GLB_HASH_PROFILE_VPWS_SRC_INTERFACE)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "source-interface");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }

    if (counter != 0)
    {
        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "\n");
    }
    else
    {
        sal_fprintf(fp, "\n");
    }
}

void
hash_field_profile_show_hash_field_vpls(FILE* fp, uint32 vpls_set)
{
    uint32 counter = 0;
    
    sal_fprintf(fp, " %-20s","vpls:");
    if (vpls_set & GLB_HASH_PROFILE_VPLS_SRC_INTERFACE)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "src-interface");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (vpls_set & GLB_HASH_PROFILE_VPLS_L2IN_BIT)
    {
        if (vpls_set & GLB_HASH_PROFILE_VPLS_L2_ETH_TYPE)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner2-eth-type");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vpls_set & GLB_HASH_PROFILE_VPLS_L2_VLAN)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner2-vlan");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vpls_set & GLB_HASH_PROFILE_VPLS_L2_DST_MAC)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner2-macda");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vpls_set & GLB_HASH_PROFILE_VPLS_L2_SRC_MAC)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner2-macsa");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
    }

    if (vpls_set & GLB_HASH_PROFILE_VPLS_L3IN_BIT)
    {
        if (vpls_set & GLB_HASH_PROFILE_VPLS_L3_DST_IP)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner3-ipda");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vpls_set & GLB_HASH_PROFILE_VPLS_L3_SRC_IP)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner3-ipsa");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vpls_set & GLB_HASH_PROFILE_VPLS_L3_IP_PROTOCOL)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner3-ip-protocol");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vpls_set & GLB_HASH_PROFILE_VPLS_L3_DST_PORT)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner3-l4-destport");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
        if (vpls_set & GLB_HASH_PROFILE_VPLS_L3_SRC_PORT)
        {
            counter++;
            sal_fprintf(fp, "  %-20s", "inner3-l4-sourceport");
            hash_field_profile_show_hash_field_segment(fp, &counter);
        }
    }

    if (counter != 0)
    {
        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "\n");
    }
    else
    {
        sal_fprintf(fp, "\n");
    }
}

void
hash_field_profile_show_hash_field_l3vpn(FILE* fp, uint32 l3vpn_set)
{
    uint32 counter = 0;
    
    sal_fprintf(fp, " %-20s","l3vpn:");
    if (l3vpn_set & GLB_HASH_PROFILE_L3VPN_SRC_INTERFACE)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "src-interface");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }

    if (l3vpn_set & GLB_HASH_PROFILE_L3VPN_DST_IP)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "inner-ipda");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (l3vpn_set & GLB_HASH_PROFILE_L3VPN_SRC_IP)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "inner-ipsa");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (l3vpn_set & GLB_HASH_PROFILE_L3VPN_IP_PROTOCOL)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "inner-ip-protocol");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (l3vpn_set & GLB_HASH_PROFILE_L3VPN_DST_PORT)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "inner-l4-destport");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    if (l3vpn_set & GLB_HASH_PROFILE_L3VPN_SRC_PORT)
    {
        counter++;
        sal_fprintf(fp, "  %-20s", "inner-l4-sourceport");
        hash_field_profile_show_hash_field_segment(fp, &counter);
    }
    
    if (counter != 0)
    {
        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "\n");
    }
    else
    {
        sal_fprintf(fp, "\n");
    }
}
*/
/*end*/

int32
hash_field_profile_show_hash_field_profile_one(FILE* fp, tbl_hash_field_profile_t* p_db_hash_fld_pro)
{
    sal_fprintf(fp, "hash-field name: %s\n", p_db_hash_fld_pro->key.name);
    if (sal_strlen(p_db_hash_fld_pro->description))
    {
        sal_fprintf (fp, "description: %s\n", p_db_hash_fld_pro->description);
    }
    sal_fprintf (fp, " %-34s %-35s\n", "Option", "Control type");
    sal_fprintf (fp, "----------------------------------------------------------------------\n");

    sal_fprintf (fp, " %-35s", "hash-arithmetic first");
    if (GLB_FLAG_ISSET(p_db_hash_fld_pro->hash_arithmetic, GLB_HASH_PROFILE_HASH_ARITHMETIC_A_CRC16_1))
    {
        sal_fprintf (fp, "crc\n");
    }
    else if (GLB_FLAG_ISSET(p_db_hash_fld_pro->hash_arithmetic, GLB_HASH_PROFILE_HASH_ARITHMETIC_A_XOR16))
    {
        sal_fprintf (fp, "xor\n");
    }
    sal_fprintf (fp, " %-35s", "hash-arithmetic second");
    if (GLB_FLAG_ISSET(p_db_hash_fld_pro->hash_arithmetic, GLB_HASH_PROFILE_HASH_ARITHMETIC_B_CRC16_1))
    {
        sal_fprintf (fp, "crc\n");
    }
    else if (GLB_FLAG_ISSET(p_db_hash_fld_pro->hash_arithmetic, GLB_HASH_PROFILE_HASH_ARITHMETIC_B_XOR16))
    {
        sal_fprintf (fp, "xor\n");
    }

    sal_fprintf (fp, " %-35s", "hash symmetry");
    if (p_db_hash_fld_pro->mode_symmetry)
    {
        sal_fprintf (fp, "enable\n");
    }
    else
    {
        sal_fprintf (fp, "disable\n");
    }

    sal_fprintf (fp, " %-35s", "ip");
    if (p_db_hash_fld_pro->bitmap_disable & GLB_HASH_PROFILE_IP_DISABLE)
    {
        sal_fprintf (fp, "disable\n");
    }
    else
    {
        sal_fprintf (fp, "enable\n");
    }
    
//    if (acl_stm_is_ipv6_profile())
    {
        sal_fprintf (fp, " %-35s", "ipv6");
        if (p_db_hash_fld_pro->bitmap_disable & GLB_HASH_PROFILE_IPV6_DISABLE)
        {
            sal_fprintf (fp, "disable\n");
        }
        else
        {
            sal_fprintf (fp, "enable\n");
        }
    }

    sal_fprintf (fp, " %-35s", "mpls");
    if (p_db_hash_fld_pro->bitmap_disable & GLB_HASH_PROFILE_MPLS_DISABLE)
    {
        sal_fprintf (fp, "disable\n");
    }
    else
    {
        sal_fprintf (fp, "enable\n");
    }

    sal_fprintf (fp, "----------------------------------------------------------------------\n");
    sal_fprintf (fp, " hash field select\n");
    sal_fprintf (fp, " %-20s  %-20s\n","Packet","HashField");
    sal_fprintf (fp, "----------------------------------------------------------------------\n");

    hash_field_profile_show_hash_field_l2(fp, p_db_hash_fld_pro->l2_set);
    hash_field_profile_show_hash_field_ip(fp, p_db_hash_fld_pro->ip_set);
//    if (acl_stm_is_ipv6_profile())
    {
        hash_field_profile_show_hash_field_ipv6(fp, p_db_hash_fld_pro->ipv6_set);
    }
    hash_field_profile_show_hash_field_gre(fp, p_db_hash_fld_pro->gre_set);
    hash_field_profile_show_hash_field_vxlan(fp, p_db_hash_fld_pro->vxlan_set);
    hash_field_profile_show_hash_field_nvgre(fp, p_db_hash_fld_pro->nvgre_set);
    hash_field_profile_show_hash_field_mpls(fp, p_db_hash_fld_pro->mpls_set);
    
    /*added by yejl for bug 54080, 2019-10-24*/
    //hash_field_profile_show_hash_field_vpws(fp, p_db_hash_fld_pro->vpws_set);
    //hash_field_profile_show_hash_field_vpls(fp, p_db_hash_fld_pro->vpls_set);
    //hash_field_profile_show_hash_field_l3vpn(fp, p_db_hash_fld_pro->l3vpn_set);
    /*ended*/
    
    return PM_E_NONE;
}

int32
hash_field_profile_show_hash_field_profile(tbl_hash_field_profile_t* p_db_hash_fld_pro, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    tbl_hash_field_profile_master_t *p_master_fld_pro = tbl_hash_field_profile_get_master();
    ctclib_slistnode_t *listnode = NULL;
    
    if (p_db_hash_fld_pro)
    {
        hash_field_profile_show_hash_field_profile_one(fp, p_db_hash_fld_pro);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master_fld_pro->hash_field_profile_list, p_db_hash_fld_pro, listnode)
        {
            #ifdef TAPPRODUCT
            if (sal_strcmp (p_db_hash_fld_pro->key.name, GLB_HASH_FIELD_ECMP_STR) == 0)
            {
                continue;
            }
            #endif
            hash_field_profile_show_hash_field_profile_one(fp, p_db_hash_fld_pro);
        }
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
hash_field_profile_cmd_show_process_hash(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_hash_field_profile_key_t hash_fld_pro_key;
    tbl_hash_field_profile_t* p_db_hash_fld_pro = NULL;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            rc = hash_field_profile_show_hash_field_profile(NULL, para);
        }
        else
        {           
            sal_strncpy(hash_fld_pro_key.name, argv[0], sizeof(hash_fld_pro_key.name));
            p_db_hash_fld_pro = tbl_hash_field_profile_get_hash_field_profile(&hash_fld_pro_key);
            if (NULL == p_db_hash_fld_pro)
            {
                CFG_INVALID_PARAM_RET("Hash field %s does not exist", argv[0]);
            }

            #ifdef TAPPRODUCT
            if (0 == sal_strcmp(argv[0] , GLB_HASH_FIELD_ECMP_STR))/*ecmp*/
            {
                CFG_CONFLICT_RET("Hash field %s does not exist", GLB_HASH_FIELD_ECMP_STR);
            }
            #endif
            
            rc = hash_field_profile_show_hash_field_profile(p_db_hash_fld_pro, para);        
        }
        break;
    default:
        break;
    }
    return rc ; 
}

int32
hash_field_profile_cmd_process_hash(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    cdb_field_t *p_field = NULL;
    field_parse_t field;
    tbl_hash_field_profile_t hash_fld_pro;
    tbl_hash_field_profile_key_t hash_fld_pro_key;
    tbl_hash_field_profile_t* p_db_hash_fld_pro = NULL;
    tbl_hash_field_profile_master_t* p_hash_fld_pro_master = tbl_hash_field_profile_get_master();
    char *hash_profile_name = NULL;
    int32 field_id = 0;
    uint32 bitmap_disable = 0;
    uint8 argc_idx = 0;
    uint32 hash_fld_pro_index = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_HASH_FIELD_PROFILE);

    sal_memset(&hash_fld_pro, 0, sizeof(hash_fld_pro));
    sal_memset(&hash_fld_pro_key, 0, sizeof(hash_fld_pro_key));
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        /*modified by wanh for bug 54047, 2019-10-16*/
        #ifdef TAPPRODUCT
        if (0 == sal_strcmp(argv[0] , GLB_HASH_FIELD_ECMP_STR))/*ecmp*/
        {
            CFG_CONFLICT_RET("Not support to configure hash-field %s", argv[0]);
        }
        #endif
        sal_strncpy(hash_fld_pro_key.name, argv[0], sizeof(hash_fld_pro.key.name));
        p_db_hash_fld_pro = tbl_hash_field_profile_get_hash_field_profile(&hash_fld_pro_key);
        
        if (NULL != p_db_hash_fld_pro)
        {   
            return PM_E_NONE;
        }
        
        /*check validity*/
        if (check_name_character_and_len(argv[0], HASH_FIELD_PROFILE_NAME_SIZE) != 0)
        {
            CFG_CONFLICT_RET("The initial character should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 64");
        }

        /*check num of user-defined hash field*/
        if (p_hash_fld_pro_master->hash_field_profile_list->count >= GLB_HASH_FIELD_PROFILE_MAX_NUM)
        {
            CFG_CONFLICT_RET("The maximum of user-defined hash field is %u", GLB_HASH_FIELD_PROFILE_USR_DEF_MAX_NUM);
        }

        /*menory allocation*/
        rc = ctclib_opb_alloc_offset(&hash_field_pro_index_opb, &hash_fld_pro_index);
        if (0 != rc)
        {
            return rc;
        }
        PM_E_RETURN(hash_field_profile_create(hash_fld_pro_index, argv[0]));
        
        break;
        /*wanh end*/

    case CDB_OPER_DEL:
        /*added by wanh for bug 54047, 2019-10-16*/

        #ifdef TAPPRODUCT
        if (0 == sal_strcmp(argv[0] , GLB_HASH_FIELD_ECMP_STR))/*ecmp*/
        {
            CFG_CONFLICT_RET("Not support to delete hash-field %s", argv[0]);
        }
        #endif
        
        /*check keyword*/
        if ( (0 == sal_strcmp(argv[0] , GLB_HASH_FIELD_PORT_CHANNEL_STR)) || (0 == sal_strcmp(argv[0] , GLB_HASH_FIELD_ECMP_STR)) )
        {
            CFG_CONFLICT_RET("Can not delete system default hash field: %s", argv[0]);
        }

        /*check existence*/
        sal_strncpy(hash_fld_pro_key.name, argv[0], sizeof(hash_fld_pro_key.name));
        p_db_hash_fld_pro = tbl_hash_field_profile_get_hash_field_profile(&hash_fld_pro_key);
        if (NULL == p_db_hash_fld_pro)
        {
            CFG_CONFLICT_RET("Hash field %s does not exist", argv[0]);
        }

        /*check in use*/
        hash_fld_pro_index = p_db_hash_fld_pro->index;
        rc = hash_field_profile_check_in_use(hash_fld_pro_index);
        if (GLB_HASH_FIELD_IN_USE == rc)
        {
            CFG_CONFLICT_RET("Hash field %s is in use", argv[0]);
        }
      
        /*menory free*/
        PM_E_RETURN(tbl_hash_field_profile_del_hash_field_profile(&hash_fld_pro_key));
        rc = ctclib_opb_free_offset(&hash_field_pro_index_opb, hash_fld_pro_index);
        if (0 != rc)
        {
            return rc;
        }
        
        break;
        /*wanh end*/
        

    case CDB_OPER_SET:
        sal_strncpy(hash_fld_pro.key.name, argv[0], HASH_FIELD_PROFILE_NAME_SIZE);
        p_db_hash_fld_pro = tbl_hash_field_profile_get_hash_field_profile(&(hash_fld_pro.key));
        if (NULL == p_db_hash_fld_pro)
        {
            CFG_INVALID_PARAM_RET("Hash field %s does not exist", argv[0]);
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
        case TBL_HASH_FIELD_PROFILE_FLD_DESCRIPTION:
            if (argc > 2)
            {
                if (check_name_character_and_len(argv[2], DESC_SIZE) != 0)
                {
                    CFG_CONFLICT_RET("The initial character of description name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 64");
                }
                sal_strncpy(hash_fld_pro.description, argv[2], DESC_SIZE);
            }
            rc = hash_field_profile_cmd_set_description(&hash_fld_pro, p_db_hash_fld_pro, para);   
            break;

        case TBL_HASH_FIELD_PROFILE_FLD_GRE_SET:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 3, 13, para->p_rs);
            if (argc > 3)
            {
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("gre-key");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.gre_set, GLB_HASH_PROFILE_GRE_KEY);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("gre-protocol");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.gre_set, GLB_HASH_PROFILE_GRE_PROTOCOL);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ipda");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.gre_set, GLB_HASH_PROFILE_GRE_DST_IP);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ipsa");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.gre_set, GLB_HASH_PROFILE_GRE_SRC_IP);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-interface");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.gre_set, GLB_HASH_PROFILE_GRE_SRC_INTERFACE);
                }
            }
            else
            {
                hash_fld_pro.gre_set = GLB_HASH_FIELD_PROFILE_GRE_SET_DEFAULT;
            }
            rc = hash_field_profile_cmd_set_gre_field_value(&hash_fld_pro, p_db_hash_fld_pro, para); 
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_L2_SET:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 3, 13, para->p_rs);
            if (argc > 3)
            {
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("eth-type");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.l2_set, GLB_HASH_PROFILE_L2_ETH_TYPE);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("macda");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.l2_set, GLB_HASH_PROFILE_L2_DST_MAC);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("macsa");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.l2_set, GLB_HASH_PROFILE_L2_SRC_MAC);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-interface");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.l2_set, GLB_HASH_PROFILE_L2_SRC_INTERFACE);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("vlan");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.l2_set, GLB_HASH_PROFILE_L2_VLAN);
                }
            }
            else
            {
                hash_fld_pro.l2_set = GLB_HASH_FIELD_PROFILE_L2_SET_DEFAULT;
            }
            rc = hash_field_profile_cmd_set_l2_field_value(&hash_fld_pro, p_db_hash_fld_pro, para);   
            break;

        case TBL_HASH_FIELD_PROFILE_FLD_IP_SET:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 3, 15, para->p_rs);
            if (argc > 3)
            {
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("destport");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.ip_set, GLB_HASH_PROFILE_IP_DST_PORT);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ip-protocol");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.ip_set, GLB_HASH_PROFILE_IP_IP_PROTOCOL);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ipda");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.ip_set, GLB_HASH_PROFILE_IP_DST_IP);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ipsa");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.ip_set, GLB_HASH_PROFILE_IP_SRC_IP);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("sourceport");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.ip_set, GLB_HASH_PROFILE_IP_SRC_PORT);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-interface");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.ip_set, GLB_HASH_PROFILE_IP_SRC_INTERFACE);
                }
            }
            else
            {
                hash_fld_pro.ip_set = GLB_HASH_FIELD_PROFILE_IP_SET_DEFAULT;
            }
            rc = hash_field_profile_cmd_set_ip_field_value(&hash_fld_pro, p_db_hash_fld_pro, para);   
            break;
            
        case TBL_HASH_FIELD_PROFILE_FLD_IPV6_SET:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 3, 15, para->p_rs);
            if (argc > 3)
            {
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("destport");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.ipv6_set, GLB_HASH_PROFILE_IP_DST_PORT);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ip-protocol");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.ipv6_set, GLB_HASH_PROFILE_IP_IP_PROTOCOL);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ipda");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.ipv6_set, GLB_HASH_PROFILE_IP_DST_IP);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ipsa");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.ipv6_set, GLB_HASH_PROFILE_IP_SRC_IP);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("sourceport");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.ipv6_set, GLB_HASH_PROFILE_IP_SRC_PORT);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-interface");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.ipv6_set, GLB_HASH_PROFILE_IP_SRC_INTERFACE);
                }
            }
            else
            {
                hash_fld_pro.ipv6_set = GLB_HASH_FIELD_PROFILE_IPV6_SET_DEFAULT;
            }
            rc = hash_field_profile_cmd_set_ipv6_field_value(&hash_fld_pro, p_db_hash_fld_pro, para);   
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_L3VPN_SET:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 3, 12, para->p_rs);
            if (argc > 3)
            {
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("inner");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.l3vpn_set, GLB_HASH_PROFILE_L3VPN_L3IN_BIT);
                    if (0 == sal_strcmp(argv[argc_idx + 2], "destport"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.l3vpn_set, GLB_HASH_PROFILE_L3VPN_DST_PORT);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 3], "ip-protocol"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.l3vpn_set, GLB_HASH_PROFILE_L3VPN_IP_PROTOCOL);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 4], "ipda"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.l3vpn_set, GLB_HASH_PROFILE_L3VPN_DST_IP);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 5], "ipsa"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.l3vpn_set, GLB_HASH_PROFILE_L3VPN_SRC_IP);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 6], "sourceport"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.l3vpn_set, GLB_HASH_PROFILE_L3VPN_SRC_PORT);
                    }
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-interface");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.l3vpn_set, GLB_HASH_PROFILE_L3VPN_SRC_INTERFACE);
                }
            }
            else
            {
                hash_fld_pro.l3vpn_set = GLB_HASH_FIELD_PROFILE_L3VPN_SET_DEFAULT;
            }
            rc = hash_field_profile_cmd_set_l3vpn_field_value(&hash_fld_pro, p_db_hash_fld_pro, para);
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_NVGRE_SET:  
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 3, 22, para->p_rs);
            if (argc > 3)
            {
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("inner-layer2");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.nvgre_set, GLB_HASH_PROFILE_NVGRE_L2IN_BIT);
                    if (0 == sal_strcmp(argv[argc_idx + 2], "eth-type"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.nvgre_set, GLB_HASH_PROFILE_NVGRE_INNER_ETH_TYPE);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 3], "macda"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.nvgre_set, GLB_HASH_PROFILE_NVGRE_INNER_DST_MAC);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 4], "macsa"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.nvgre_set, GLB_HASH_PROFILE_NVGRE_INNER_SRC_MAC);
                    }
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("inner-layer3");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.nvgre_set, GLB_HASH_PROFILE_NVGRE_L3IN_BIT);
                    if (0 == sal_strcmp(argv[argc_idx + 2], "ip-protocol"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.nvgre_set, GLB_HASH_PROFILE_NVGRE_INNER_IP_PROTOCOL);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 3], "ipda"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.nvgre_set, GLB_HASH_PROFILE_NVGRE_INNER_DST_IP);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 4], "ipsa"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.nvgre_set, GLB_HASH_PROFILE_NVGRE_INNER_SRC_IP);
                    }
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("outer");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.nvgre_set, GLB_HASH_PROFILE_NVGRE_OUT_BIT);
                    if (0 == sal_strcmp(argv[argc_idx + 2], "gre-protocol"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.nvgre_set, GLB_HASH_PROFILE_NVGRE_OUTER_GRE_PROTOCOL);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 3], "ipda"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.nvgre_set, GLB_HASH_PROFILE_NVGRE_OUTER_DST_IP);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 4], "ipsa"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.nvgre_set, GLB_HASH_PROFILE_NVGRE_OUTER_SRC_IP);
                    }
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("vsid");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.nvgre_set, GLB_HASH_PROFILE_NVGRE_VSID);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-interface");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.nvgre_set, GLB_HASH_PROFILE_NVGRE_SRC_INTERFACE);
                }
            }
            else
            {
                hash_fld_pro.nvgre_set = GLB_HASH_FIELD_PROFILE_NVGRE_SET_DEFAULT;
            }
            rc = hash_field_profile_cmd_set_nvgre_field_value(&hash_fld_pro, p_db_hash_fld_pro, para);
            break;

        case TBL_HASH_FIELD_PROFILE_FLD_MPLS_SET:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 3, 15, para->p_rs);
            if (argc > 3)
            {
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("2nd-label");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.mpls_set, GLB_HASH_PROFILE_MPLS_2ND_LABEL);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("3rd-label");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.mpls_set, GLB_HASH_PROFILE_MPLS_3RD_LABEL);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("inner");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.mpls_set, GLB_HASH_PROFILE_MPLS_INNER_BIT);
                    if (0 == sal_strcmp(argv[argc_idx + 2], "ipda"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.mpls_set, GLB_HASH_PROFILE_MPLS_DST_IP);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 3], "ipsa"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.mpls_set, GLB_HASH_PROFILE_MPLS_SRC_IP);
                    }
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-interface");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.mpls_set, GLB_HASH_PROFILE_MPLS_SRC_INTERFACE);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("top-label");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.mpls_set, GLB_HASH_PROFILE_MPLS_TOP_LABEL);
                }
            }
            else
            {
                hash_fld_pro.mpls_set = GLB_HASH_FIELD_PROFILE_MPLS_SET_DEFAULT;
            }
            rc = hash_field_profile_cmd_set_mpls_field_value(&hash_fld_pro, p_db_hash_fld_pro, para);
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_VPWS_SET:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 3, 11, para->p_rs);
            if (argc >3)
            {
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("2nd-label");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.vpws_set, GLB_HASH_PROFILE_VPWS_2ND_LABEL);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("3rd-label");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.vpws_set, GLB_HASH_PROFILE_VPWS_3RD_LABEL);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-interface");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.vpws_set, GLB_HASH_PROFILE_VPWS_SRC_INTERFACE);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("top-label");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.vpws_set, GLB_HASH_PROFILE_VPWS_TOP_LABEL);
                }
            }
            else
            {
                hash_fld_pro.vpws_set = GLB_HASH_FIELD_PROFILE_VPWS_SET_DEFAULT;
            }
            rc = hash_field_profile_cmd_set_vpws_field_value(&hash_fld_pro, p_db_hash_fld_pro, para);
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_VPLS_SET:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 3, 18, para->p_rs);
            if (argc > 3)
            {
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("inner-layer2");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.vpls_set, GLB_HASH_PROFILE_VPLS_L2IN_BIT);
                    if (0 == sal_strcmp(argv[argc_idx + 2], "eth-type"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vpls_set, GLB_HASH_PROFILE_VPLS_L2_ETH_TYPE);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 3], "macda"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vpls_set, GLB_HASH_PROFILE_VPLS_L2_DST_MAC);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 4], "macsa"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vpls_set, GLB_HASH_PROFILE_VPLS_L2_SRC_MAC);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 5], "vlan"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vpls_set, GLB_HASH_PROFILE_VPLS_L2_VLAN);
                    }
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("inner-layer3");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.vpls_set, GLB_HASH_PROFILE_VPLS_L3IN_BIT);
                    if (0 == sal_strcmp(argv[argc_idx + 2], "destport"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vpls_set, GLB_HASH_PROFILE_VPLS_L3_DST_PORT);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 3], "ip-protocol"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vpls_set, GLB_HASH_PROFILE_VPLS_L3_IP_PROTOCOL);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 4], "ipda"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vpls_set, GLB_HASH_PROFILE_VPLS_L3_DST_IP);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 5], "ipsa"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vpls_set, GLB_HASH_PROFILE_VPLS_L3_SRC_IP);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 6], "sourceport"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vpls_set, GLB_HASH_PROFILE_VPLS_L3_SRC_PORT);
                    }
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-interface");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.vpls_set, GLB_HASH_PROFILE_VPLS_SRC_INTERFACE);
                }
            }
            else
            {
                hash_fld_pro.vpls_set = GLB_HASH_FIELD_PROFILE_VPLS_SET_DEFAULT;
            }
            rc = hash_field_profile_cmd_set_vpls_field_value(&hash_fld_pro, p_db_hash_fld_pro, para);
            break;

        case TBL_HASH_FIELD_PROFILE_FLD_VXLAN_SET:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 3, 26, para->p_rs);
            if (argc > 3)
            {
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("inner-layer2");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_L2IN_BIT);
                    if (0 == sal_strcmp(argv[argc_idx + 2], "eth-type"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_INNER_ETH_TYPE);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 3], "macda"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_INNER_DST_MAC);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 4], "macsa"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_INNER_SRC_MAC);
                    }
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("inner-layer3");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_L3IN_BIT);
                    if (0 == sal_strcmp(argv[argc_idx + 2], "destport"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_INNER_DST_PORT);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 3], "ip-protocol"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_INNER_IP_PROTOCOL);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 4], "ipda"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_INNER_DST_IP);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 5], "ipsa"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_INNER_SRC_IP);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 6], "sourceport"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_INNER_SRC_PORT);
                    }
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("outer");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_OUTER_BIT);
                    if (0 == sal_strcmp(argv[argc_idx + 2], "destport"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_OUTER_DST_PORT);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 3], "ipda"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_OUTER_DST_IP);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 4], "ipsa"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_OUTER_SRC_IP);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 5], "sourceport"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_OUTER_SRC_PORT);
                    }
                    if (0 == sal_strcmp(argv[argc_idx + 6], "vlan"))
                    {
                        GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_OUTER_VLAN);
                    }
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("src-interface");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_SRC_INTERFACE);
                }
                argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("vni");
                if(argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
                {
                    GLB_SET_FLAG(hash_fld_pro.vxlan_set, GLB_HASH_PROFILE_VXLAN_VNI);
                }
            }
            else
            {
                hash_fld_pro.vxlan_set = GLB_HASH_FIELD_PROFILE_VXLAN_SET_DEFAULT;
            }
            rc = hash_field_profile_cmd_set_vxlan_field_value(&hash_fld_pro, p_db_hash_fld_pro, para);
            break;
            
        case TBL_HASH_FIELD_PROFILE_FLD_HASH_ARITHMETIC:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            if (sal_strcmp (argv[2], "crc") == 0)
            {
                GLB_SET_FLAG(hash_fld_pro.hash_arithmetic, GLB_HASH_PROFILE_HASH_ARITHMETIC_A_CRC16_1);
            }
            else if (sal_strcmp (argv[2], "xor") == 0)
            {
                GLB_SET_FLAG(hash_fld_pro.hash_arithmetic, GLB_HASH_PROFILE_HASH_ARITHMETIC_A_XOR16);
            }
            if (sal_strcmp (argv[3], "crc") == 0)
            {
                GLB_SET_FLAG(hash_fld_pro.hash_arithmetic, GLB_HASH_PROFILE_HASH_ARITHMETIC_B_CRC16_1);
            }
            else if (sal_strcmp (argv[3], "xor") == 0)
            {
                GLB_SET_FLAG(hash_fld_pro.hash_arithmetic, GLB_HASH_PROFILE_HASH_ARITHMETIC_B_XOR16);
            }
            rc = hash_field_profile_cmd_set_hash_arithmetic(&hash_fld_pro, p_db_hash_fld_pro, para);       
            break;

        case TBL_HASH_FIELD_PROFILE_FLD_MODE_SYMMETRY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(hash_fld_pro.mode_symmetry, argv[2], para->p_rs);
            rc = hash_field_profile_cmd_set_mode_symmetry(&hash_fld_pro, p_db_hash_fld_pro, para);       
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_BITMAP_DISABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            PM_CFG_GET_UINT(bitmap_disable, argv[3], para->p_rs);
            if (sal_strcmp (argv[2], "ip") == 0)
            {
                hash_fld_pro.bitmap_disable = p_db_hash_fld_pro->bitmap_disable;
                if (bitmap_disable)
                {
                    GLB_SET_FLAG(hash_fld_pro.bitmap_disable, GLB_HASH_PROFILE_IP_DISABLE);
                }
                else
                {
                    GLB_UNSET_FLAG(hash_fld_pro.bitmap_disable, GLB_HASH_PROFILE_IP_DISABLE);
                }
            }
            else if (sal_strcmp (argv[2], "ipv6") == 0)
            {
                hash_fld_pro.bitmap_disable = p_db_hash_fld_pro->bitmap_disable;
                if (bitmap_disable)
                {
                    GLB_SET_FLAG(hash_fld_pro.bitmap_disable, GLB_HASH_PROFILE_IPV6_DISABLE);
                }
                else
                {
                    GLB_UNSET_FLAG(hash_fld_pro.bitmap_disable, GLB_HASH_PROFILE_IPV6_DISABLE);
                }
            }
            else if (sal_strcmp (argv[2], "mpls") == 0)
            {
                hash_fld_pro.bitmap_disable = p_db_hash_fld_pro->bitmap_disable;
                if (bitmap_disable)
                {
                    GLB_SET_FLAG(hash_fld_pro.bitmap_disable, GLB_HASH_PROFILE_MPLS_DISABLE);
                }
                else
                {
                    GLB_UNSET_FLAG(hash_fld_pro.bitmap_disable, GLB_HASH_PROFILE_MPLS_DISABLE);
                }
            }
            rc = hash_field_profile_cmd_set_bitmap_disable(&hash_fld_pro, p_db_hash_fld_pro, para);       
            break;
        }
        
    case CDB_OPER_GET:
        if (0 == argc)
        {
            hash_profile_name = NULL;
        }
        else if (1 == argc)
        {
            hash_profile_name = argv[0];
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = hash_field_profile_cmd_get_hash_field_profile(hash_profile_name, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

