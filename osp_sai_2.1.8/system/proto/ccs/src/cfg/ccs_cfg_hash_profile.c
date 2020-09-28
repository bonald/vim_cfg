#include "proto.h"
#include "glb_hash_field_value_define.h"
#include "gen/tbl_hash_field_profile_define.h"
#include "gen/tbl_hash_field_profile.h"
#include "gen/tbl_hash_value_profile_define.h"
#include "gen/tbl_hash_value_profile.h"
#include "gen/tbl_hash_value_global_define.h"
#include "gen/tbl_hash_value_global.h"
#include "switch.h"


void
_ccs_cfg_build_hash_get_profile_name_by_profile_id(int32 field_id, char profile_name[64])
{
    tbl_hash_field_profile_master_t *p_master_field_pro = tbl_hash_field_profile_get_master();
    tbl_hash_field_profile_t *p_db_hash_field_pro = NULL; 
    ctclib_slistnode_t *listnode = NULL;
    
    CTCLIB_SLIST_LOOP(p_master_field_pro->hash_field_profile_list, p_db_hash_field_pro, listnode)
    {
        if (p_db_hash_field_pro->index == field_id)
        {   
            sal_strcpy(profile_name, p_db_hash_field_pro->key.name);
        }
    }
}

int32
_ccs_cfg_build_hash_field_get_string_by_l2set_value(char* l2set_str, uint32 l2set)
{
    char str_tmp[20] = {'\0'};
    uint32 has_l2set = FALSE;
    
    if (GLB_FLAG_ISSET(l2set, GLB_HASH_PROFILE_L2_ETH_TYPE))
    {
        sal_sprintf (str_tmp, "%s ", "eth-type");
        sal_strcat (l2set_str, str_tmp);
        has_l2set = TRUE;
    }
    if (GLB_FLAG_ISSET(l2set, GLB_HASH_PROFILE_L2_DST_MAC))
    {
        sal_sprintf (str_tmp, "%s ", "macda");
        sal_strcat (l2set_str, str_tmp);
        has_l2set = TRUE;
    }
    if (GLB_FLAG_ISSET(l2set, GLB_HASH_PROFILE_L2_SRC_MAC))
    {
        sal_sprintf (str_tmp, "%s ", "macsa");
        sal_strcat (l2set_str, str_tmp);
        has_l2set = TRUE;
    }
    if (GLB_FLAG_ISSET(l2set, GLB_HASH_PROFILE_L2_SRC_INTERFACE))
    {
        sal_sprintf (str_tmp, "%s ", "src-interface");
        sal_strcat (l2set_str, str_tmp);
        has_l2set = TRUE;
    }
    if (GLB_FLAG_ISSET(l2set, GLB_HASH_PROFILE_L2_VLAN))
    {
        sal_sprintf (str_tmp, "%s ", "vlan");
        sal_strcat (l2set_str, str_tmp);
        has_l2set = TRUE;
    }
    if (has_l2set)
    {
        l2set_str[sal_strlen(l2set_str)-1] = CMD_ZERO_CHAR;    
    }
    return PM_E_NONE;
}


int32
_ccs_cfg_build_hash_field_get_string_by_ipset_value(char* ipset_str, uint32 ipset)
{
    char str_tmp[20] = {'\0'};
    uint32 has_ipset = FALSE;
    
    if (GLB_FLAG_ISSET(ipset, GLB_HASH_PROFILE_IP_DST_PORT))
    {
        sal_sprintf (str_tmp, "%s ", "destport");
        sal_strcat (ipset_str, str_tmp);
        has_ipset = TRUE;
    }
    if (GLB_FLAG_ISSET(ipset, GLB_HASH_PROFILE_IP_IP_PROTOCOL))
    {
        sal_sprintf (str_tmp, "%s ", "ip-protocol");
        sal_strcat (ipset_str, str_tmp);
        has_ipset = TRUE;
    }
    if (GLB_FLAG_ISSET(ipset, GLB_HASH_PROFILE_IP_DST_IP))
    {
        sal_sprintf (str_tmp, "%s ", "ipda");
        sal_strcat (ipset_str, str_tmp);
        has_ipset = TRUE;
    }
    if (GLB_FLAG_ISSET(ipset, GLB_HASH_PROFILE_IP_SRC_IP))
    {
        sal_sprintf (str_tmp, "%s ", "ipsa");
        sal_strcat (ipset_str, str_tmp);
        has_ipset = TRUE;
    }
    if (GLB_FLAG_ISSET(ipset, GLB_HASH_PROFILE_IP_SRC_PORT))
    {
        sal_sprintf (str_tmp, "%s ", "sourceport");
        sal_strcat (ipset_str, str_tmp);
        has_ipset = TRUE;
    }
    if (GLB_FLAG_ISSET(ipset, GLB_HASH_PROFILE_IP_SRC_INTERFACE))
    {
        sal_sprintf (str_tmp, "%s ", "src-interface");
        sal_strcat (ipset_str, str_tmp);
        has_ipset = TRUE;
    }
    if (has_ipset)
    {
        ipset_str[sal_strlen(ipset_str)-1] = CMD_ZERO_CHAR;    
    }
    return PM_E_NONE;
}

int32
_ccs_cfg_build_hash_field_get_string_by_greset_value(char* greset_str, uint32 greset)
{
    char str_tmp[20] = {'\0'};
    uint32 has_greset = FALSE;
    
    if (GLB_FLAG_ISSET(greset, GLB_HASH_PROFILE_GRE_DST_IP))
    {
        sal_sprintf (str_tmp, "%s ", "ipda");
        sal_strcat (greset_str, str_tmp);
        has_greset = TRUE;
    }
    if (GLB_FLAG_ISSET(greset, GLB_HASH_PROFILE_GRE_SRC_IP))
    {
        sal_sprintf (str_tmp, "%s ", "ipsa");
        sal_strcat (greset_str, str_tmp);
        has_greset = TRUE;
    }
    if (GLB_FLAG_ISSET(greset, GLB_HASH_PROFILE_GRE_KEY))
    {
        sal_sprintf (str_tmp, "%s ", "gre-key");
        sal_strcat (greset_str, str_tmp);
        has_greset = TRUE;
    }
    if (GLB_FLAG_ISSET(greset, GLB_HASH_PROFILE_GRE_PROTOCOL))
    {
        sal_sprintf (str_tmp, "%s ", "gre-protocol");
        sal_strcat (greset_str, str_tmp);
        has_greset = TRUE;
    }
    if (GLB_FLAG_ISSET(greset, GLB_HASH_PROFILE_GRE_SRC_INTERFACE))
    {
        sal_sprintf (str_tmp, "%s ", "src-interface");
        sal_strcat (greset_str, str_tmp);
        has_greset = TRUE;
    }
    if (has_greset)
    {
        greset_str[sal_strlen(greset_str)-1] = CMD_ZERO_CHAR;    
    }
    return PM_E_NONE;
}

int32
_ccs_cfg_build_hash_field_get_string_by_vpwsset_value(char* vpwsset_str, uint32 vpwsset)
{
    char str_tmp[20] = {'\0'};
    uint32 has_vpwsset = FALSE;
    
    if (GLB_FLAG_ISSET(vpwsset, GLB_HASH_PROFILE_VPWS_2ND_LABEL))
    {
        sal_sprintf (str_tmp, "%s ", "2nd-label");
        sal_strcat (vpwsset_str, str_tmp);
        has_vpwsset = TRUE;
    }
    if (GLB_FLAG_ISSET(vpwsset, GLB_HASH_PROFILE_VPWS_3RD_LABEL))
    {
        sal_sprintf (str_tmp, "%s ", "3rd-label");
        sal_strcat (vpwsset_str, str_tmp);
        has_vpwsset = TRUE;
    }
    if (GLB_FLAG_ISSET(vpwsset, GLB_HASH_PROFILE_VPWS_TOP_LABEL))
    {
        sal_sprintf (str_tmp, "%s ", "top-label");
        sal_strcat (vpwsset_str, str_tmp);
        has_vpwsset = TRUE;
    }
    if (GLB_FLAG_ISSET(vpwsset, GLB_HASH_PROFILE_VPWS_SRC_INTERFACE))
    {
        sal_sprintf (str_tmp, "%s ", "src-interface");
        sal_strcat (vpwsset_str, str_tmp);
        has_vpwsset = TRUE;
    }
    if (has_vpwsset)
    {
        vpwsset_str[sal_strlen(vpwsset_str)-1] = CMD_ZERO_CHAR;    
    }
    return PM_E_NONE;
}

int32
_ccs_cfg_build_hash_field_get_string_by_vplsset_value(char* vplsset_str, uint32 vplsset)
{
    char str_tmp[20] = {'\0'};
    uint32 has_vplsset = FALSE;
    
    if (GLB_FLAG_ISSET(vplsset, GLB_HASH_PROFILE_VPLS_SRC_INTERFACE))
    {
        sal_sprintf (str_tmp, "%s ", "src-interface");
        sal_strcat (vplsset_str, str_tmp);
        has_vplsset = TRUE;
    }
    if (GLB_FLAG_ISSET(vplsset, GLB_HASH_PROFILE_VPLS_L2IN_BIT))
    {
        sal_sprintf (str_tmp, "%s ", "inner-layer2"); 
        sal_strcat (vplsset_str, str_tmp);
        if (GLB_FLAG_ISSET(vplsset, GLB_HASH_PROFILE_VPLS_L2_ETH_TYPE))
        {
            sal_sprintf (str_tmp, "%s ", "eth-type");
            sal_strcat (vplsset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vplsset, GLB_HASH_PROFILE_VPLS_L2_DST_MAC))
        {
            sal_sprintf (str_tmp, "%s ", "macda");
            sal_strcat (vplsset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vplsset, GLB_HASH_PROFILE_VPLS_L2_SRC_MAC))
        {
            sal_sprintf (str_tmp, "%s ", "macsa");
            sal_strcat (vplsset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vplsset, GLB_HASH_PROFILE_VPLS_L2_VLAN))
        {
            sal_sprintf (str_tmp, "%s ", "vlan");
            sal_strcat (vplsset_str, str_tmp);
        }
        has_vplsset = TRUE;
    } 
    if (GLB_FLAG_ISSET(vplsset, GLB_HASH_PROFILE_VPLS_L3IN_BIT))
    {
        sal_sprintf (str_tmp, "%s ", "inner-layer3"); 
        sal_strcat (vplsset_str, str_tmp);
        if (GLB_FLAG_ISSET(vplsset, GLB_HASH_PROFILE_VPLS_L3_DST_PORT))
        {
            sal_sprintf (str_tmp, "%s ", "destport");
            sal_strcat (vplsset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vplsset, GLB_HASH_PROFILE_VPLS_L3_IP_PROTOCOL))
        {
            sal_sprintf (str_tmp, "%s ", "ip-protocol");
            sal_strcat (vplsset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vplsset, GLB_HASH_PROFILE_VPLS_L3_DST_IP))
        {
            sal_sprintf (str_tmp, "%s ", "ipda");
            sal_strcat (vplsset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vplsset, GLB_HASH_PROFILE_VPLS_L3_SRC_IP))
        {
            sal_sprintf (str_tmp, "%s ", "ipsa");
            sal_strcat (vplsset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vplsset, GLB_HASH_PROFILE_VPLS_L3_SRC_PORT))
        {
            sal_sprintf (str_tmp, "%s ", "sourceport");
            sal_strcat (vplsset_str, str_tmp);
        }
        has_vplsset = TRUE;
    } 
   
    if (has_vplsset)
    {
        vplsset_str[sal_strlen(vplsset_str)-1] = CMD_ZERO_CHAR;
    }
    return PM_E_NONE;
}

int32
_ccs_cfg_build_hash_field_get_string_by_l3vpnset_value(char* l3vpnset_str, uint32 l3vpnset)
{
    char str_tmp[20] = {'\0'};
    uint32 has_l3vpnset = FALSE;
    
    if (GLB_FLAG_ISSET(l3vpnset, GLB_HASH_PROFILE_L3VPN_SRC_INTERFACE))
    {
        sal_sprintf (str_tmp, "%s ", "src-interface");
        sal_strcat (l3vpnset_str, str_tmp);
        has_l3vpnset = TRUE;
    }
    if (GLB_FLAG_ISSET(l3vpnset, GLB_HASH_PROFILE_L3VPN_L3IN_BIT))
    {
        sal_sprintf (str_tmp, "%s ", "inner"); 
        sal_strcat (l3vpnset_str, str_tmp);
        if (GLB_FLAG_ISSET(l3vpnset, GLB_HASH_PROFILE_L3VPN_DST_PORT))
        {
            sal_sprintf (str_tmp, "%s ", "destport");
            sal_strcat (l3vpnset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(l3vpnset, GLB_HASH_PROFILE_L3VPN_IP_PROTOCOL))
        {
            sal_sprintf (str_tmp, "%s ", "ip-protocol");
            sal_strcat (l3vpnset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(l3vpnset, GLB_HASH_PROFILE_L3VPN_DST_IP))
        {
            sal_sprintf (str_tmp, "%s ", "ipda");
            sal_strcat (l3vpnset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(l3vpnset, GLB_HASH_PROFILE_L3VPN_SRC_IP))
        {
            sal_sprintf (str_tmp, "%s ", "ipsa");
            sal_strcat (l3vpnset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(l3vpnset, GLB_HASH_PROFILE_L3VPN_SRC_PORT))
        {
            sal_sprintf (str_tmp, "%s ", "sourceport");
            sal_strcat (l3vpnset_str, str_tmp);
        }
        has_l3vpnset = TRUE;
    } 
   
    if (has_l3vpnset)
    {
        l3vpnset_str[sal_strlen(l3vpnset_str)-1] = CMD_ZERO_CHAR;    
    }
    return PM_E_NONE;
}


int32
_ccs_cfg_build_hash_field_get_string_by_nvgreset_value(char* nvgreset_str, uint32 nvgreset)
{
    char str_tmp[20] = {'\0'};
    uint32 has_nvgreset = FALSE;
    
    if (GLB_FLAG_ISSET(nvgreset, GLB_HASH_PROFILE_NVGRE_SRC_INTERFACE))
    {
        sal_sprintf (str_tmp, "%s ", "src-interface");
        sal_strcat (nvgreset_str, str_tmp);
        has_nvgreset = TRUE;
    }
    if (GLB_FLAG_ISSET(nvgreset, GLB_HASH_PROFILE_NVGRE_VSID))
    {
        sal_sprintf (str_tmp, "%s ", "vsid");
        sal_strcat (nvgreset_str, str_tmp);
        has_nvgreset = TRUE;
    }
    
    if (GLB_FLAG_ISSET(nvgreset, GLB_HASH_PROFILE_NVGRE_L2IN_BIT))
    {
        sal_sprintf (str_tmp, "%s ", "inner-layer2"); 
        sal_strcat (nvgreset_str, str_tmp);
        if (GLB_FLAG_ISSET(nvgreset, GLB_HASH_PROFILE_NVGRE_INNER_ETH_TYPE))
        {
            sal_sprintf (str_tmp, "%s ", "eth-type");
            sal_strcat (nvgreset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(nvgreset, GLB_HASH_PROFILE_NVGRE_INNER_DST_MAC))
        {
            sal_sprintf (str_tmp, "%s ", "macda");
            sal_strcat (nvgreset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(nvgreset, GLB_HASH_PROFILE_NVGRE_INNER_SRC_MAC))
        {
            sal_sprintf (str_tmp, "%s ", "macsa");
            sal_strcat (nvgreset_str, str_tmp);
        }
        has_nvgreset = TRUE;
    } 
    if (GLB_FLAG_ISSET(nvgreset, GLB_HASH_PROFILE_NVGRE_L3IN_BIT))
    {
        sal_sprintf (str_tmp, "%s ", "inner-layer3"); 
        sal_strcat (nvgreset_str, str_tmp);
        if (GLB_FLAG_ISSET(nvgreset, GLB_HASH_PROFILE_NVGRE_INNER_IP_PROTOCOL))
        {
            sal_sprintf (str_tmp, "%s ", "ip-protocol");
            sal_strcat (nvgreset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(nvgreset, GLB_HASH_PROFILE_NVGRE_INNER_DST_IP))
        {
            sal_sprintf (str_tmp, "%s ", "ipda");
            sal_strcat (nvgreset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(nvgreset, GLB_HASH_PROFILE_NVGRE_INNER_SRC_IP))
        {
            sal_sprintf (str_tmp, "%s ", "ipsa");
            sal_strcat (nvgreset_str, str_tmp);
        }
        has_nvgreset = TRUE;
    } 
    if (GLB_FLAG_ISSET(nvgreset, GLB_HASH_PROFILE_NVGRE_OUT_BIT))
    {
        sal_sprintf (str_tmp, "%s ", "outer"); 
        sal_strcat (nvgreset_str, str_tmp);
        if (GLB_FLAG_ISSET(nvgreset, GLB_HASH_PROFILE_NVGRE_OUTER_GRE_PROTOCOL))
        {
            sal_sprintf (str_tmp, "%s ", "gre-protocol");
            sal_strcat (nvgreset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(nvgreset, GLB_HASH_PROFILE_NVGRE_OUTER_DST_IP))
        {
            sal_sprintf (str_tmp, "%s ", "ipda");
            sal_strcat (nvgreset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(nvgreset, GLB_HASH_PROFILE_NVGRE_OUTER_SRC_IP))
        {
            sal_sprintf (str_tmp, "%s ", "ipsa");
            sal_strcat (nvgreset_str, str_tmp);
        }
        has_nvgreset = TRUE;
    } 
   
    if (has_nvgreset)
    {
        nvgreset_str[sal_strlen(nvgreset_str)-1] = CMD_ZERO_CHAR;    
    }
    return PM_E_NONE;
}

int32
_ccs_cfg_build_hash_field_get_string_by_mplsset_value(char* mplsset_str, uint32 mplsset)
{
    char str_tmp[20] = {'\0'};
    uint32 has_mplsset = FALSE;
    
    if (GLB_FLAG_ISSET(mplsset, GLB_HASH_PROFILE_MPLS_2ND_LABEL))
    {
        sal_sprintf (str_tmp, "%s ", "2nd-label");
        sal_strcat (mplsset_str, str_tmp);
        has_mplsset = TRUE;
    }
    if (GLB_FLAG_ISSET(mplsset, GLB_HASH_PROFILE_MPLS_3RD_LABEL))
    {
        sal_sprintf (str_tmp, "%s ", "3rd-label");
        sal_strcat (mplsset_str, str_tmp);
        has_mplsset = TRUE;
    }
    
    if (GLB_FLAG_ISSET(mplsset, GLB_HASH_PROFILE_MPLS_TOP_LABEL))
    {
        sal_sprintf (str_tmp, "%s ", "top-label"); 
        sal_strcat (mplsset_str, str_tmp);
        has_mplsset = TRUE;
    } 

/*spec is not currently supported*/
#if 0/*added by wanh for bug 54157, 2019-11-09*/    
    if (GLB_FLAG_ISSET(mplsset, GLB_HASH_PROFILE_MPLS_INNER_BIT))
    {
        sal_sprintf (str_tmp, "%s ", "inner"); 
        sal_strcat (mplsset_str, str_tmp);
        if (GLB_FLAG_ISSET(mplsset, GLB_HASH_PROFILE_MPLS_DST_IP))
        {
            sal_sprintf (str_tmp, "%s ", "ipda");
            sal_strcat (mplsset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(mplsset, GLB_HASH_PROFILE_MPLS_SRC_IP))
        {
            sal_sprintf (str_tmp, "%s ", "ipsa");
            sal_strcat (mplsset_str, str_tmp);
        }
        has_mplsset = TRUE;
    } 
#endif/*wanh end*/

    if (GLB_FLAG_ISSET(mplsset, GLB_HASH_PROFILE_MPLS_SRC_INTERFACE))
    {
        sal_sprintf (str_tmp, "%s ", "src-interface"); 
        sal_strcat (mplsset_str, str_tmp);
        has_mplsset = TRUE;
    } 
   
    if (has_mplsset)
    {
        mplsset_str[sal_strlen(mplsset_str)-1] = CMD_ZERO_CHAR;    
    }
    return PM_E_NONE;
}


int32
_ccs_cfg_build_hash_field_get_string_by_vxlanset_value(char* vxlanset_str, uint32 vxlanset)
{
    char str_tmp[20] = {'\0'};
    uint32 has_vxlanset = FALSE;

    
    if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_SRC_INTERFACE))
    {
        sal_sprintf (str_tmp, "%s ", "src-interface");
        sal_strcat (vxlanset_str, str_tmp);
        has_vxlanset = TRUE;
    }
    if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_VNI))
    {
        sal_sprintf (str_tmp, "%s ", "vni");
        sal_strcat (vxlanset_str, str_tmp);
        has_vxlanset = TRUE;
    }
    
    if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_L2IN_BIT))
    {
        sal_sprintf (str_tmp, "%s ", "inner-layer2"); 
        sal_strcat (vxlanset_str, str_tmp);
        if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_INNER_ETH_TYPE))
        {
            sal_sprintf (str_tmp, "%s ", "eth-type");
            sal_strcat (vxlanset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_INNER_DST_MAC))
        {
            sal_sprintf (str_tmp, "%s ", "macda");
            sal_strcat (vxlanset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_INNER_SRC_MAC))
        {
            sal_sprintf (str_tmp, "%s ", "macsa");
            sal_strcat (vxlanset_str, str_tmp);
        }
        has_vxlanset = TRUE;
    } 
    if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_L3IN_BIT))
    {
        sal_sprintf (str_tmp, "%s ", "inner-layer3"); 
        sal_strcat (vxlanset_str, str_tmp);
        if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_INNER_DST_PORT))
        {
            sal_sprintf (str_tmp, "%s ", "destport");
            sal_strcat (vxlanset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_INNER_IP_PROTOCOL))
        {
            sal_sprintf (str_tmp, "%s ", "ip-protocol");
            sal_strcat (vxlanset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_INNER_DST_IP))
        {
            sal_sprintf (str_tmp, "%s ", "ipda");
            sal_strcat (vxlanset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_INNER_SRC_IP))
        {
            sal_sprintf (str_tmp, "%s ", "ipsa");
            sal_strcat (vxlanset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_INNER_SRC_PORT))
        {
            sal_sprintf (str_tmp, "%s ", "sourceport");
            sal_strcat (vxlanset_str, str_tmp);
        }
        has_vxlanset = TRUE;
    } 
    if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_OUTER_BIT))
    {
        sal_sprintf (str_tmp, "%s ", "outer"); 
        sal_strcat (vxlanset_str, str_tmp);
        if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_OUTER_DST_PORT))
        {
            sal_sprintf (str_tmp, "%s ", "destport");
            sal_strcat (vxlanset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_OUTER_DST_IP))
        {
            sal_sprintf (str_tmp, "%s ", "ipda");
            sal_strcat (vxlanset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_OUTER_SRC_IP))
        {
            sal_sprintf (str_tmp, "%s ", "ipsa");
            sal_strcat (vxlanset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_OUTER_SRC_PORT))
        {
            sal_sprintf (str_tmp, "%s ", "sourceport");
            sal_strcat (vxlanset_str, str_tmp);
        }
        if (GLB_FLAG_ISSET(vxlanset, GLB_HASH_PROFILE_VXLAN_OUTER_VLAN))
        {
            sal_sprintf (str_tmp, "%s ", "vlan");
            sal_strcat (vxlanset_str, str_tmp);
        }
        has_vxlanset = TRUE;
    } 
   
    if (has_vxlanset)
    {
        vxlanset_str[sal_strlen(vxlanset_str)-1] = CMD_ZERO_CHAR;    
    }
    return PM_E_NONE;
}



int32
_ccs_cfg_build_hash_field_profile(FILE* fp, tbl_hash_field_profile_t * p_hash_fld_pro)
{
    char l2setstr[128] = {'\0'};
    char ipsetstr[128] = {'\0'};
    char ipv6setstr[128] = {'\0'};
    char gresetstr[128] = {'\0'};
    char mplssetstr[128] = {'\0'};
    char nvgresetstr[128] = {'\0'};
    char vxlansetstr[128] = {'\0'};
    /*added by yejl for bug 54080, 2019-10-24*/
    //char vplssetstr[128] = {'\0'};
    //char vpwssetstr[128] = {'\0'};
    //char l3vpnsetstr[128] = {'\0'};
    /*ended*/

    if (NULL == p_hash_fld_pro)
    {
        return PM_E_NOT_EXIST;
    }
    
    /*modified by wanh for bug 54047, 2019-10-19*/
    if (sal_strlen(p_hash_fld_pro->description) ||
        p_hash_fld_pro->hash_arithmetic != GLB_HASH_FIELD_PROFILE_HASH_ARITHMETIC_DEFAULT ||
        p_hash_fld_pro->ipv6_set != GLB_HASH_FIELD_PROFILE_IPV6_SET_DEFAULT ||
        p_hash_fld_pro->ip_set != GLB_HASH_FIELD_PROFILE_IP_SET_DEFAULT ||
        p_hash_fld_pro->l2_set != GLB_HASH_FIELD_PROFILE_L2_SET_DEFAULT ||
        p_hash_fld_pro->gre_set != GLB_HASH_FIELD_PROFILE_GRE_SET_DEFAULT ||
        p_hash_fld_pro->vxlan_set != GLB_HASH_FIELD_PROFILE_VXLAN_SET_DEFAULT ||
        p_hash_fld_pro->nvgre_set != GLB_HASH_FIELD_PROFILE_NVGRE_SET_DEFAULT ||
        p_hash_fld_pro->mpls_set != GLB_HASH_FIELD_PROFILE_MPLS_SET_DEFAULT ||
        /*added by yejl for bug 54080, 2019-10-24*/
        //p_hash_fld_pro->vpws_set != GLB_HASH_FIELD_PROFILE_VPWS_SET_DEFAULT ||
        //p_hash_fld_pro->vpls_set != GLB_HASH_FIELD_PROFILE_VPLS_SET_DEFAULT ||
        //p_hash_fld_pro->l3vpn_set != GLB_HASH_FIELD_PROFILE_L3VPN_SET_DEFAULT ||
        /*ended*/
        p_hash_fld_pro->mode_symmetry ||
        p_hash_fld_pro->bitmap_disable ||
        p_hash_fld_pro->index > GLB_HASH_FIELD_PROFILE_SYS_DEF_MAX_INDEX)  /*wanh end*/
    {
        sal_fprintf (fp, "hash-field %s\n", p_hash_fld_pro->key.name);
    }
    else
    {
        return PM_E_NOT_EXIST;    
    }

    if (sal_strlen(p_hash_fld_pro->description))
    {
        sal_fprintf (fp, " description %s\n", p_hash_fld_pro->description);
    }

    if (p_hash_fld_pro->l2_set != GLB_HASH_FIELD_PROFILE_L2_SET_DEFAULT)
    {
        _ccs_cfg_build_hash_field_get_string_by_l2set_value(l2setstr, p_hash_fld_pro->l2_set);
        if (sal_strlen(l2setstr))
        {
            sal_fprintf (fp, " l2 %s\n", l2setstr);
        }
    }
    if (p_hash_fld_pro->ip_set != GLB_HASH_FIELD_PROFILE_IP_SET_DEFAULT)
    {
        _ccs_cfg_build_hash_field_get_string_by_ipset_value(ipsetstr, p_hash_fld_pro->ip_set);
        if (sal_strlen(ipsetstr))
        {
            sal_fprintf (fp, " ip %s\n", ipsetstr);
        }
    }
    if (p_hash_fld_pro->ipv6_set != GLB_HASH_FIELD_PROFILE_IPV6_SET_DEFAULT)
    {
        _ccs_cfg_build_hash_field_get_string_by_ipset_value(ipv6setstr, p_hash_fld_pro->ipv6_set);
        if (sal_strlen(ipv6setstr))
        {
            sal_fprintf (fp, " ipv6 %s\n", ipv6setstr);
        }
    }

    if (p_hash_fld_pro->gre_set != GLB_HASH_FIELD_PROFILE_GRE_SET_DEFAULT)
    {
        _ccs_cfg_build_hash_field_get_string_by_greset_value(gresetstr, p_hash_fld_pro->gre_set);
        if (sal_strlen(gresetstr))
        {
            sal_fprintf (fp, " gre %s\n", gresetstr);
        }
    }

    if (p_hash_fld_pro->mpls_set != GLB_HASH_FIELD_PROFILE_MPLS_SET_DEFAULT)
    {
        _ccs_cfg_build_hash_field_get_string_by_mplsset_value(mplssetstr, p_hash_fld_pro->mpls_set);
        if (sal_strlen(mplssetstr))
        {
            sal_fprintf (fp, " mpls %s\n", mplssetstr);
        }
    }

    if (p_hash_fld_pro->nvgre_set != GLB_HASH_FIELD_PROFILE_NVGRE_SET_DEFAULT)
    {
        _ccs_cfg_build_hash_field_get_string_by_nvgreset_value(nvgresetstr, p_hash_fld_pro->nvgre_set);
        if (sal_strlen(nvgresetstr))
        {
            sal_fprintf (fp, " nvgre %s\n", nvgresetstr);
        }
    }

    if (p_hash_fld_pro->vxlan_set != GLB_HASH_FIELD_PROFILE_VXLAN_SET_DEFAULT)
    {
        _ccs_cfg_build_hash_field_get_string_by_vxlanset_value(vxlansetstr, p_hash_fld_pro->vxlan_set);
        if (sal_strlen(vxlansetstr))
        {
            sal_fprintf (fp, " vxlan %s\n", vxlansetstr);
        }
    }

/*added by yejl for bug 54080, 2019-10-24*/
/*
    if (p_hash_fld_pro->vpls_set != GLB_HASH_FIELD_PROFILE_VPLS_SET_DEFAULT)
    {
        _ccs_cfg_build_hash_field_get_string_by_vplsset_value(vplssetstr, p_hash_fld_pro->vpls_set);
        if (sal_strlen(vplssetstr))
        {
            sal_fprintf (fp, " vpls %s\n", vplssetstr);
        }
    }

    if (p_hash_fld_pro->vpws_set != GLB_HASH_FIELD_PROFILE_VPWS_SET_DEFAULT)
    {
        _ccs_cfg_build_hash_field_get_string_by_vpwsset_value(vpwssetstr, p_hash_fld_pro->vpws_set);
        if (sal_strlen(vpwssetstr))
        {
            sal_fprintf (fp, " vpws %s\n", vpwssetstr);
        }
    }
    
    if (p_hash_fld_pro->l3vpn_set != GLB_HASH_FIELD_PROFILE_L3VPN_SET_DEFAULT)
    {
        _ccs_cfg_build_hash_field_get_string_by_l3vpnset_value(l3vpnsetstr, p_hash_fld_pro->l3vpn_set);
        if (sal_strlen(l3vpnsetstr))
        {
            sal_fprintf (fp, " l3vpn %s\n", l3vpnsetstr);
        }
    }
*/    
    
    if (p_hash_fld_pro->hash_arithmetic != GLB_HASH_FIELD_PROFILE_HASH_ARITHMETIC_DEFAULT)
    {
        if (GLB_FLAG_ISSET(p_hash_fld_pro->hash_arithmetic, GLB_HASH_PROFILE_HASH_ARITHMETIC_A_CRC16_1) &&
            (GLB_FLAG_ISSET(p_hash_fld_pro->hash_arithmetic, GLB_HASH_PROFILE_HASH_ARITHMETIC_B_CRC16_1)))
        {
            sal_fprintf (fp, " hash-arithmetic first crc second crc\n");
        }
        else if (GLB_FLAG_ISSET(p_hash_fld_pro->hash_arithmetic, GLB_HASH_PROFILE_HASH_ARITHMETIC_A_CRC16_1) &&
                (GLB_FLAG_ISSET(p_hash_fld_pro->hash_arithmetic, GLB_HASH_PROFILE_HASH_ARITHMETIC_B_XOR16)))
        {
            sal_fprintf (fp, " hash-arithmetic first crc second xor\n");
        }
        else if (GLB_FLAG_ISSET(p_hash_fld_pro->hash_arithmetic, GLB_HASH_PROFILE_HASH_ARITHMETIC_A_XOR16) &&
                (GLB_FLAG_ISSET(p_hash_fld_pro->hash_arithmetic, GLB_HASH_PROFILE_HASH_ARITHMETIC_B_XOR16)))
        {
            sal_fprintf (fp, " hash-arithmetic first xor second xor\n");    
        }
            
    }
    if (p_hash_fld_pro->mode_symmetry)
    {
        sal_fprintf (fp, " mode symmetry\n");
    }
    if (GLB_FLAG_ISSET(p_hash_fld_pro->bitmap_disable, GLB_HASH_PROFILE_IP_DISABLE))
    {
        sal_fprintf (fp, " ip disable\n");
    } 
    if (GLB_FLAG_ISSET(p_hash_fld_pro->bitmap_disable, GLB_HASH_PROFILE_IPV6_DISABLE))
    {
        sal_fprintf (fp, " ipv6 disable\n");
    }
    if (GLB_FLAG_ISSET(p_hash_fld_pro->bitmap_disable, GLB_HASH_PROFILE_MPLS_DISABLE))
    {
        sal_fprintf (fp, " mpls disable\n");
    }
    
    return PM_E_NONE;
}

int32
_ccs_cfg_build_hash_value_profile(FILE* fp, tbl_hash_value_profile_t * p_hash_value_pro)
{
    uint32 select_value = 0;
    uint32 alg = 0;
    char* arithmetic[2] = {"first", "second"};
    char profile_name[64] = {'\0'};
    
    if (NULL == p_hash_value_pro)
    {
        return PM_E_NOT_EXIST;
    }
    sal_fprintf (fp, "hash-value %s\n", p_hash_value_pro->key.name);
    
    if (sal_strlen(p_hash_value_pro->description))
    {
        sal_fprintf (fp, " description %s\n", p_hash_value_pro->description);
    }
    if (p_hash_value_pro->bitmap_command & GLB_COMMAND_PORT_CHANNEL_SET_ALL)
    {
        select_value = p_hash_value_pro->agg_input/GLB_HASH_OFFSET_VALUE_SIZE;
        alg = p_hash_value_pro->agg_input/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " port-channel select %s hash-arithmetic %s\n", profile_name, arithmetic[alg]);    
        }        
    }
    if (p_hash_value_pro->bitmap_command & GLB_COMMAND_PORT_CHANNEL_SET_STATIC)
    {
        select_value = p_hash_value_pro->agg_static/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_pro->agg_static/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " port-channel static select %s hash-arithmetic %s\n", profile_name, arithmetic[alg]);    
        }
    }
    if (p_hash_value_pro->bitmap_command & GLB_COMMAND_PORT_CHANNEL_SET_DYNAMIC)
    {
        select_value = p_hash_value_pro->agg_dynamic/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_pro->agg_dynamic/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " port-channel dynamic select %s hash-arithmetic %s\n", profile_name, arithmetic[alg]);
        }
    }
    if (p_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_ALL)
    {
        select_value = p_hash_value_pro->ecmp_all/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_pro->ecmp_all/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " ecmp select %s hash-arithmetic %s\n", profile_name, arithmetic[alg]);
        }
    }
    if (p_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_DYNAMIC)
    {
        select_value = p_hash_value_pro->ecmp_dynamic/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_pro->ecmp_dynamic/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " ecmp dynamic select %s hash-arithmetic %s\n", profile_name, arithmetic[alg]);
        }
    }
    if (p_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_FLOW_ID)
    {
        select_value = p_hash_value_pro->ecmp_dynamic_flow_id/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_pro->ecmp_dynamic_flow_id/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " ecmp dynamic flow-id select %s hash-arithmetic %s\n", profile_name, arithmetic[alg]);
        }
    }
    if (p_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_ALL)
    {
        select_value = p_hash_value_pro->ecmp_static/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_pro->ecmp_static/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " ecmp static select %s hash-arithmetic %s\n", profile_name, arithmetic[alg]);
        }
    }
    if (p_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_L3)
    {
        select_value = p_hash_value_pro->ecmp_static_l3/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_pro->ecmp_static_l3/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " ecmp static select %s hash-arithmetic %s l3\n", profile_name, arithmetic[alg]);
        }
    }
    if (p_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_NVGRE)
    {
        select_value = p_hash_value_pro->ecmp_static_nvgre/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_pro->ecmp_static_nvgre/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " ecmp static select %s hash-arithmetic %s nvgre\n", profile_name, arithmetic[alg]);
        }
    }
    if (p_hash_value_pro->bitmap_command & GLB_COMMAND_ECMP_SET_STATIC_VXLAN)
    {
        select_value = p_hash_value_pro->ecmp_static_vxlan/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_pro->ecmp_static_vxlan/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " ecmp static select %s hash-arithmetic %s vxlan\n", profile_name, arithmetic[alg]);
        }
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_hash_field_profile(FILE* fp, cfg_cmd_para_t *para)
{
    uint32 has_config = FALSE;
    int32 rc = 0;
    tbl_hash_field_profile_t * p_hash_fld_pro = NULL;
    tbl_hash_field_profile_master_t *p_master_fld_pro = tbl_hash_field_profile_get_master();
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_master_fld_pro->hash_field_profile_list, p_hash_fld_pro, listnode)
    {
        rc = _ccs_cfg_build_hash_field_profile(fp, p_hash_fld_pro);
        if (rc == PM_E_NONE)
        {
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
ccs_cfg_build_hash_value_profile(FILE* fp, cfg_cmd_para_t *para)
{
    uint32 has_config = FALSE;
    tbl_hash_value_profile_t * p_hash_value_pro = NULL;
    tbl_hash_value_profile_master_t *p_master_value_pro = tbl_hash_value_profile_get_master();
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_master_value_pro->hash_value_profile_list, p_hash_value_pro, listnode)
    {
        _ccs_cfg_build_hash_value_profile(fp, p_hash_value_pro);
        has_config = TRUE;
    }
    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_hash_value_global(FILE* fp, cfg_cmd_para_t *para)
{
    uint32 select_value = 0;
    uint32 alg = 0;
    char* arithmetic[2] = {"first", "second"};
    char profile_name[64] = {'\0'};
    tbl_hash_value_global_t * p_hash_value_glb = NULL;
    p_hash_value_glb = tbl_hash_value_global_get_hash_value_global();
    if (NULL == p_hash_value_glb)
    {
        return PM_E_NONE;         
    }
    if (p_hash_value_glb->agg_hash == GLB_HASH_AGG_HASH_VALUE_DEFAULT_GLOBAL &&
        p_hash_value_glb->ecmp_all == GLB_HASH_ECMP_ALL_DEFAULT_GLOBAL &&
        p_hash_value_glb->ecmp_dynamic == GLB_HASH_ECMP_DYNAMIC_DEFAULT_GLOBAL &&
        p_hash_value_glb->ecmp_dynamic_flow_id == GLB_HASH_ECMP_DYNAMIC_FLOW_ID_DEFAULT_GLOBAL &&
        p_hash_value_glb->ecmp_static == GLB_HASH_ECMP_STATIC_ALL_DEFAULT_GLOBAL &&
        p_hash_value_glb->ecmp_static_l3 == GLB_HASH_ECMP_STATIC_L3_DEFAULT_GLOBAL &&
        p_hash_value_glb->ecmp_static_nvgre == GLB_HASH_ECMP_STATIC_NVGRE_DEFAULT_GLOBAL &&
        p_hash_value_glb->ecmp_static_vxlan == GLB_HASH_ECMP_STATIC_VXLAN_DEFAULT_GLOBAL &&
        p_hash_value_glb->lbid == GLB_HASH_LBID_HASH_VALUE_DEFAULT_GLOBAL)
    {
        return PM_E_NONE;
    }
    sal_fprintf (fp, "hash-value global\n"); 
    if (p_hash_value_glb->agg_hash != GLB_HASH_AGG_HASH_VALUE_DEFAULT_GLOBAL)
    {
        select_value = p_hash_value_glb->agg_hash/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_glb->agg_hash/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " port-channel select %s hash-arithmetic %s\n", profile_name, arithmetic[alg]);    
        }
    }
    if (p_hash_value_glb->lbid != GLB_HASH_LBID_HASH_VALUE_DEFAULT_GLOBAL)
    {
        select_value = p_hash_value_glb->lbid/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_glb->lbid/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " lbid select %s hash-arithmetic %s\n", profile_name, arithmetic[alg]);    
        }
    }
    if (p_hash_value_glb->ecmp_all != GLB_HASH_ECMP_ALL_DEFAULT_GLOBAL && GLB_FLAG_ISSET(p_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_ALL_GLOBAL))
    {
        select_value = p_hash_value_glb->ecmp_all/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_glb->ecmp_all/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " ecmp select %s hash-arithmetic %s\n", profile_name, arithmetic[alg]);    
        }
    }
    if (p_hash_value_glb->ecmp_dynamic != GLB_HASH_ECMP_DYNAMIC_DEFAULT_GLOBAL && GLB_FLAG_ISSET(p_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_DYNAMIC_GLOBAL))
    {
        select_value = p_hash_value_glb->ecmp_dynamic/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_glb->ecmp_dynamic/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " ecmp dynamic select %s hash-arithmetic %s\n", profile_name, arithmetic[alg]);    
        }
    }
    if (p_hash_value_glb->ecmp_dynamic_flow_id != GLB_HASH_ECMP_DYNAMIC_FLOW_ID_DEFAULT_GLOBAL && GLB_FLAG_ISSET(p_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_FLOW_ID_GLOBAL))
    {
        select_value = p_hash_value_glb->ecmp_dynamic_flow_id/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_glb->ecmp_dynamic_flow_id/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " ecmp dynamic flow-id select %s hash-arithmetic %s\n", profile_name, arithmetic[alg]);    
        }
    }
    if (p_hash_value_glb->ecmp_static != GLB_HASH_ECMP_STATIC_ALL_DEFAULT_GLOBAL && GLB_FLAG_ISSET(p_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_ALL_GLOBAL))
    {
        select_value = p_hash_value_glb->ecmp_static/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_glb->ecmp_static/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " ecmp static select %s hash-arithmetic %s\n", profile_name, arithmetic[alg]);    
        }
    }
    if (p_hash_value_glb->ecmp_static_l3 != GLB_HASH_ECMP_STATIC_L3_DEFAULT_GLOBAL && GLB_FLAG_ISSET(p_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_L3_GLOBAL))
    {
        select_value = p_hash_value_glb->ecmp_static_l3/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_glb->ecmp_static_l3/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " ecmp static select %s hash-arithmetic %s l3\n", profile_name, arithmetic[alg]);    
        }
    }
    if (p_hash_value_glb->ecmp_static_nvgre != GLB_HASH_ECMP_STATIC_NVGRE_DEFAULT_GLOBAL && GLB_FLAG_ISSET(p_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_NVGRE_GLOBAL))
    {
        select_value = p_hash_value_glb->ecmp_static_nvgre/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_glb->ecmp_static_nvgre/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " ecmp static select %s hash-arithmetic %s nvgre\n", profile_name, arithmetic[alg]);    
        }
    }
    if (p_hash_value_glb->ecmp_static_vxlan != GLB_HASH_ECMP_STATIC_VXLAN_DEFAULT_GLOBAL && GLB_FLAG_ISSET(p_hash_value_glb->bitmap_command, GLB_COMMAND_ECMP_SET_STATIC_VXLAN_GLOBAL))
    {
        select_value = p_hash_value_glb->ecmp_static_vxlan/GLB_HASH_OFFSET_VALUE_SIZE; 
        alg = p_hash_value_glb->ecmp_static_vxlan/GLB_HASH_ARITHMETIC_SIZE %2;
        sal_memset(&profile_name, 0, sizeof(profile_name));
        _ccs_cfg_build_hash_get_profile_name_by_profile_id(select_value, profile_name);
        if (sal_strlen(profile_name))
        {
            sal_fprintf (fp, " ecmp static select %s hash-arithmetic %s vxlan\n", profile_name, arithmetic[alg]);    
        }
    }
    sal_fprintf(fp, "!\n");
    return PM_E_NONE;
}

