/****************************************************************************
 * stm_snmp.c
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       AUTHOR
 * Date         :       CREATE TIME
 * Reason       :       First Create.
 ****************************************************************************/
#if 0
#include "proto.h" 
#include "snmp.h" 
#include "snmp_api.h" 
#include "stm_snmp.h" 
#include "stm_alloc.h"
#include "glb_stm_define.h"
#include "chsm_error.h"

extern struct stm_profile_spec_info_s g_snmp_spec_info;     
static oid stm_oid[] = {STMMIB};

int
_stm_snmp_oid_compare(const oid * in_name1,
                 size_t len1, const oid * in_name2, size_t len2)
{
    register int    len;
    register const oid *name1 = in_name1;
    register const oid *name2 = in_name2;

    /*
     * len = minimum of len1 and len2 
     */
    if (len1 < len2)
        len = len1;
    else
        len = len2;
    /*
     * find first non-matching OID 
     */
    while (len-- > 0) {
        /*
         * these must be done in seperate comparisons, since
         * subtracting them and using that result has problems with
         * subids > 2^31. 
         */
        if (*(name1) != *(name2)) {
            if (*(name1) < *(name2))
                return -1;
            return 1;
        }
        name1++;
        name2++;
    }
    /*
     * both OIDs equal up to length of shorter OID 
     */
    if (len1 < len2)
        return -1;
    if (len2 < len1)
        return 1;
    return 0;
}

int
_stm_snmpd_index_get (struct variable *v, oid *name, uint32 *length, int *index, int exact)
{
    int result, len;
    *index = 0;

    result = _stm_snmp_oid_compare (name, v->namelen, v->name, v->namelen);
    if (exact)
    {     
        /* Check the length. */
        if (result != 0 || *length - v->namelen != 1)
        {
            return -1; 
        }
        *index = name[v->namelen];
        return 0;
    }
    else 
    {
        /* return -1 if greater than our our table entry */
        if (result == 0)
        {
            /* use index value if present, otherwise 0. */
            len = *length - v->namelen;
            if (len == 0)
            {
                *index  = 0;
            }
            else if (len == 1) 
            {
                *index = name[v->namelen];
                (*index)++;
            }
            else
            {
                return -1;
            }
        }
        else if (result < 0)
        {
           memcpy((char *) name, (char *)v->name, ((int) v->namelen + 1) * sizeof(oid));
            *length = v->namelen + 1;
            *index  = 0;
        }
        else
        {
            return -1;
        }
    }
  return 0;
}
static int
_stmmibobject_table_value_check(struct variable *v, uint8 *var_val, uint32 var_val_len)
{ 
    int32 int_val = 0;
    switch (v->magic)
    {
        case STMMIBOBJECT_STMNEXTTYPE:
            sal_memcpy(&int_val, var_val, sizeof(int_val));
            if (int_val < 0 || int_val >= GLB_STM_MODE_MAX)
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        default:
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
} 

static uint8 *
_stmmibobject_table_read(struct variable *v, oid *name, uint32 *length, int32 exact,
                                                  uint32 *var_len, WriteMethod **write_method, uint32 vr_id)
{ 
    int32 next_boot_mode = 0;
    
     if (MATCH_FAILED == snmp_header_generic(v, name, length, exact, var_len, write_method))
     {
         return NULL;
     }

    switch (v->magic)
    {
        case STMMIBOBJECT_STMCURRENTTYPE:
            next_boot_mode = _stmmibobject_table_get_config();
            CHSM_STM_SNMP_RETURN_INTEGER(next_boot_mode);
            break;
        
        case STMMIBOBJECT_STMNEXTTYPE:
            next_boot_mode =  _stmmibobject_table_get_next_boot_mode();
            CHSM_STM_SNMP_RETURN_INTEGER(next_boot_mode);
            break;
            
        default:
            break;
    }

    return NULL;
} 

static uint8 *
_stm_show_table_read(struct variable *v, oid *name, uint32 *length, int32 exact,
                                                  uint32 *var_len, WriteMethod **write_method, uint32 vr_id)
{ 
    int ret = 0;
    bool first_in = FALSE;
    int index = 0;
    uint8 tcam_mode = 0;
    glb_system_fea_spec_t* sys_spec = NULL;
    
    ret = _stm_snmpd_index_get(v, name, length, &index,  exact);

    if (0 != ret)
    {
        return SNMP_FAIL;
    }

    if (index < 0 || index >= GLB_STM_MODE_MAX)
    {
        return SNMP_FAIL;
    }
    
    tcam_mode = index;
    ret = stm_show_table_lookup(&tcam_mode, exact, &sys_spec, first_in);
    if (SNMP_SUCCESS != ret)
    {
        return NULL;
    }
    if (!exact)
    {
        index = tcam_mode;
        snmp_api_table_index_set(v, name, length, (uint32 *)&index, 1);
    }
    switch (v->magic)
    {
    /*1. ethernet*/
        case STM_SHOW_PROFILETYPE:
            break;
        case STM_SHOW_VLANNUMBER:
            SNMP_API_RETURN_INTEGER(GLB_MAX_VLAN_ID-1);

        case STM_SHOW_VLANFORWARDINGINSTANCES:
            SNMP_API_RETURN_INTEGER(sys_spec->vlan_instance);

        case STM_SHOW_UCASTMACADDRESSES:
            SNMP_API_RETURN_INTEGER(sys_spec->ucast_fdb);

        case STM_SHOW_MCASTMACADDRESSES:
            SNMP_API_RETURN_INTEGER(sys_spec->l2mc_entries);

        case STM_SHOW_BLACKHOLEMACADDRESSES:
            SNMP_API_RETURN_INTEGER(sys_spec->mac_filter);

        case STM_SHOW_MAXAPPLIEDVLANMAPPING:
            if (GLB_CFM_MAX_MEP_RMEP_SIZE <= sys_spec->cfm_local_and_remote_meps)
            {
                SNMP_API_RETURN_INTEGER(GLB_CFM_MAX_MEP_RMEP_SIZE);
            }
            else
            {
                SNMP_API_RETURN_INTEGER(sys_spec->cfm_local_and_remote_meps);
            }

        case STM_SHOW_CFMLOCALANDREMOTEMEPS:
            SNMP_API_RETURN_INTEGER(sys_spec->cfm_lck);

        case STM_SHOW_G8031GROUPS:
            SNMP_API_RETURN_INTEGER(sys_spec->g8031_groups);

        case STM_SHOW_G8032RINGS:
            SNMP_API_RETURN_INTEGER(sys_spec->g8032_rings);

    /*2. vlan class*/
        case STM_SHOW_MACBASEDVLANCLASSIFICATION:       /*GLB_MAC_BASED_VLAN_CLASS*/
            SNMP_API_RETURN_INTEGER(sys_spec->mac_based_vlan_class);

        case STM_SHOW_IPV4BASEDVLANCLASSIFICATION:      /*GLB_IPV4_BASED_VLAN_CLASS*/
            SNMP_API_RETURN_INTEGER(sys_spec->ipv4_based_vlan_class);

        case STM_SHOW_IPV6BASEDVLANCLASSIFICATION:      /*GLB_IPV6_BASED_VLAN_CLASS*/
            SNMP_API_RETURN_INTEGER(sys_spec->ipv6_based_vlan_class);

    /*3. ip ucast routing*/
        case STM_SHOW_IPV4HOSTROUTES:       /*GLB_HOST_ROUTEV4*/
            SNMP_API_RETURN_INTEGER(sys_spec->host_routes);

        case STM_SHOW_INDIRECTIPV4ROUTES:
            SNMP_API_RETURN_INTEGER(sys_spec->remote_routes);   /*GLB_REMOTE_ROUTEV4*/

        case STM_SHOW_IPV4ECMPGROUPS:       /*GLB_ECMP_ROUTEV4*/
            SNMP_API_RETURN_INTEGER(sys_spec->ecmp_routes);

        case STM_SHOW_IPV6HOSTROUTES:       /*GLB_HOST_ROUTEV6*/
            SNMP_API_RETURN_INTEGER(sys_spec->host_v6_routes);

        case STM_SHOW_INDIRECTIPV6ROUTES:       /*GLB_REMOTE_ROUTEV6*/
            SNMP_API_RETURN_INTEGER(sys_spec->remote_v6_routes);

        case STM_SHOW_IPV6ECMPGROUPS:           /*GLB_ECMP_ROUTEV6*/
            SNMP_API_RETURN_INTEGER(sys_spec->ecmp_v6_routes);

        case STM_SHOW_IPTUNNELPEERS:        /*GLB_IP_TUNNEL*/
            SNMP_API_RETURN_INTEGER(sys_spec->ip_tunnel);

        case STM_SHOW_NATFORIVIPEERS:       /*GLB_IVI_PEERS*/
            SNMP_API_RETURN_INTEGER(sys_spec->ivi_peers);

    /*4. ip mcast routing*/
        case STM_SHOW_IPV4MCASTROUTES:      /*GLB_L3MC*/
            SNMP_API_RETURN_INTEGER(sys_spec->l3mc_entries);

        case STM_SHOW_IPV4MCASTTOTALMEMBERS:    /*GLB_MEM_PORT_L3MC*/
            SNMP_API_RETURN_INTEGER(sys_spec->l3mc_member_ports);

        case STM_SHOW_IPV6MCASTROUTES:      /*GLB_L3MC_V6*/
            SNMP_API_RETURN_INTEGER(sys_spec->l3mc_v6_entries);

        case STM_SHOW_IPV6MCASTTOTALMEMBERS:    /*GLB_MEM_PORT_L3MC_v6*/
            SNMP_API_RETURN_INTEGER(sys_spec->l3mc_v6_member_ports);

    /*5. security and acl, qos*/
        case STM_SHOW_IPV4SOURCEGUARDENTRIES:       /*GLB_IPV4_SOURCE_GUARD*/
            SNMP_API_RETURN_INTEGER(sys_spec->ipv4_source_guard);

        case STM_SHOW_IPV6SOURCEGUARDENTRIES:       /*GLB_IPV6_SOURCE_GUARD*/
            SNMP_API_RETURN_INTEGER(sys_spec->ipv6_source_guard);

        case STM_SHOW_IPV4ACLANDQOSFLOWENTRIES:     /*GLB_FLOW_ENTRY*/
            SNMP_API_RETURN_INTEGER(sys_spec->flow_entries);

        case STM_SHOW_IPV6ACLANDQOSFLOWENTRIES:     /*GLB_FLOW_ENTRY_V6*/
            SNMP_API_RETURN_INTEGER(sys_spec->flow_v6_entries);

        case STM_SHOW_FTN:      /*GLB_FTN*/
            SNMP_API_RETURN_INTEGER(sys_spec->ftn_entries);

        case STM_SHOW_INCOMINGLABELMAP:     /*GLB_ILM*/
            SNMP_API_RETURN_INTEGER(sys_spec->ilm_entries);

        case STM_SHOW_VPWS:         /*GLB_VPWS*/
            SNMP_API_RETURN_INTEGER(sys_spec->vpws);

        case STM_SHOW_LSPPE:        /*GLB_LSP_PE*/
            SNMP_API_RETURN_INTEGER(sys_spec->lsp_pe);

        case STM_SHOW_LSPP:         /*GLB_LSP_P*/
            SNMP_API_RETURN_INTEGER(sys_spec->lsp_p);

        case STM_SHOW_VPLSPEER:     /*GLB_VPLS_PEER*/
            SNMP_API_RETURN_INTEGER(sys_spec->vpls_peer);

        case STM_SHOW_VPLSAC:       /*GLB_VPLS_AC*/
            SNMP_API_RETURN_INTEGER(sys_spec->vpls_ac);

        case STM_SHOW_VPLSINSTANCE:     /*GLB_VSI*/
            SNMP_API_RETURN_INTEGER(sys_spec->vsi);

        case STM_SHOW_LSPOAM:       /*GLB_LSP_OAM*/
            SNMP_API_RETURN_INTEGER(sys_spec->lsp_oam);

        case STM_SHOW_OAMFORPSEUDOWRIE:     /*GLB_PW_OAM*/
            SNMP_API_RETURN_INTEGER(sys_spec->pw_oam);

        case STM_SHOW_MPLSAPSTUNNEL:        /*GLB_MPLS_APS_TUNNEL*/
            SNMP_API_RETURN_INTEGER(sys_spec->mpls_aps_tunnel);

        default:
            break;
    }
    return NULL;
} 

static int
_stmmibobject_table_write(int32 action, uint8 *var_val, uint8 var_val_type, uint32 var_val_len,
                    uint8 *statP, oid *name, uint32 length, struct variable *v, uint32 vr_id)
{ 
    int int_val = 0;
    int tcam_mod = 0;
    int32 ret = 0;
    
    switch (v->magic)
    {
        case STMMIBOBJECT_STMNEXTTYPE:
            sal_memcpy(&int_val, var_val, var_val_len);
            tcam_mod = int_val;
            ret = stm_alloc_write_profile_mode(tcam_mod);
            
            if (0 != ret && CHSM_E_STM_MODE_IN_USE != ret )
            {
                return SNMP_ERR_INCONSISTENTVALUE;
            }
            break;
            
        default:
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
} 
 
struct variable stm_variables[] = 
{
    {STMMIBOBJECT_STMCURRENTTYPE, ASN_INTEGER, RONLY, _stmmibobject_table_read, 2, { 1, 1}}, 

    {STMMIBOBJECT_STMNEXTTYPE, ASN_INTEGER, RWRITE, _stmmibobject_table_read, 2, {1, 2}, 
    _stmmibobject_table_value_check, snmp_api_non_table_index_check, 
    _stmmibobject_table_write}, 

    {STM_SHOW_PROFILETYPE, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 1}}, 

    {STM_SHOW_VLANNUMBER, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 2}}, 

    {STM_SHOW_VLANFORWARDINGINSTANCES, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 3}}, 

    {STM_SHOW_UCASTMACADDRESSES, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 4}}, 
    {STM_SHOW_MCASTMACADDRESSES, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 5}}, 

    {STM_SHOW_BLACKHOLEMACADDRESSES, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 6}}, 

    {STM_SHOW_MAXAPPLIEDVLANMAPPING, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 7}}, 

    {STM_SHOW_CFMLOCALANDREMOTEMEPS, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 8}}, 

    {STM_SHOW_G8031GROUPS, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 9}}, 

    {STM_SHOW_G8032RINGS, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 10}}, 

    {STM_SHOW_MACBASEDVLANCLASSIFICATION, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 11}}, 

    {STM_SHOW_IPV4BASEDVLANCLASSIFICATION, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 12}}, 

    {STM_SHOW_IPV6BASEDVLANCLASSIFICATION, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 13}}, 

    {STM_SHOW_IPV4HOSTROUTES, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 14}}, 

    {STM_SHOW_INDIRECTIPV4ROUTES, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 15}}, 

    {STM_SHOW_IPV4ECMPGROUPS, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 16}}, 

    {STM_SHOW_IPV6HOSTROUTES, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 17}}, 

    {STM_SHOW_INDIRECTIPV6ROUTES, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 18}}, 

    {STM_SHOW_IPV6ECMPGROUPS, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 19}}, 

    {STM_SHOW_IPTUNNELPEERS, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 20}}, 

    {STM_SHOW_NATFORIVIPEERS, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 21}}, 

    {STM_SHOW_IPV4MCASTROUTES, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 22}}, 

    {STM_SHOW_IPV4MCASTTOTALMEMBERS, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 23}}, 

    {STM_SHOW_IPV6MCASTROUTES, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 24}}, 

    {STM_SHOW_IPV6MCASTTOTALMEMBERS, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 25}}, 

    {STM_SHOW_IPV4SOURCEGUARDENTRIES, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 26}}, 

    {STM_SHOW_IPV6SOURCEGUARDENTRIES, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 27}}, 

    {STM_SHOW_IPV4ACLANDQOSFLOWENTRIES, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 28}}, 

    {STM_SHOW_IPV6ACLANDQOSFLOWENTRIES, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 29}}, 

    {STM_SHOW_FTN, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 30}}, 

    {STM_SHOW_INCOMINGLABELMAP, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 31}}, 

    {STM_SHOW_VPWS, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 32}}, 

    {STM_SHOW_LSPPE, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 33}}, 

    {STM_SHOW_LSPP, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 34}}, 

    {STM_SHOW_VPLSPEER, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 35}}, 

    {STM_SHOW_VPLSAC, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 36}}, 

    {STM_SHOW_VPLSINSTANCE, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 37}}, 

    {STM_SHOW_LSPOAM, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 38}}, 

    {STM_SHOW_OAMFORPSEUDOWRIE, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 39}}, 

    {STM_SHOW_MPLSAPSTUNNEL, ASN_INTEGER, RONLY, _stm_show_table_read, 4, {1, 3, 1, 40}}, 
};

void
stm_snmp_init(struct lib_globals *zg)
{ 
    REGISTER_MIB(zg, "stm", stm_variables, variable, stm_oid);
} 

#endif
