#if XXX
#include "dhcpd.h"
#include "dhcp_version.h"
#include "dhcp_relayagent.h"

#include "pal.h"
#include "lib.h"
#include "avl_tree.h"

#include "thread.h"
#include "log.h"
#include "ctclib_hash.h"
#include "dhcrelay_snmp.h"
#include "dhcrelay_func.h"
#include "param_check.h"
#include "snmp_api.h"
#ifdef HAVE_IPV6
#include "dhcrelay6_func.h"

extern dhcp_relay_pkt_stats_t g_dhcp_relay_pkt_stats;
extern u_int16_t g_dhcprelay6_remoteid_format;
extern u_int8_t g_dhcprelay6_pdroute_distance;

static char g_szServerGroupIp6[1024];

extern int dhcprelay6_serveritem_add(S_Dhcpv6ServerGroup *pGroup, char *pszAddr, char *name);
extern int dhcprelay6_serveritem_free(S_Dhcpv6ServerItem *pItem);
extern int dhcprelay6_cli_pdroute_clear(struct cli *cli, char *prefix, char *ip, char *ifname);
#endif

/* DHCP-MIB instances. */
oid dhcp_oid [] = { DHCPMIB};
oid dhcpr_oid [] = { DHCPRMIB};
oid dhcpr_global_oid [] = { DHCPRGLOBALMIB};
oid dhcpr_server_oid [] = { DHCPRSERVERMIB};

char g_szServerGroup[1024];

extern int drop_agent_mismatches;
extern int add_agent_options;
extern E_RelayPolicy agent_relay_mode;

/* statistics */
extern int bad_circuit_id;
extern int bogus_giaddr_drops;
extern int client_packet_errors;
extern int client_packets_relayed;
extern int corrupt_agent_options;
extern int missing_agent_option;
extern int missing_circuit_id;
extern int server_packet_errors;
extern int server_packets_relayed;

/* Hook functions. */
//static unsigned char *dhcp_snmp_enable ();
//static unsigned char *dhcp_relay_snmp();
static unsigned char *dhcpRelayOption82();
static unsigned char *dhcpRelayIpToGroup();

static unsigned char *dhcpRStatisticsEntry();

extern void host_cli_out(void *pArg, char *fmt, ...);

static int dhcp_int_val;
static unsigned char dhcp_char_str[1024];

int
dhcrelay_snmp_index_get (struct variable *v, oid *name, size_t *length, 
		    int *index , int indexdimension, int exact)
{ 
    if (exact)
    {
        if (*length - v->namelen != indexdimension)
            return -1;

        oid_copy(index, name + v->namelen, indexdimension);
    }
    else
    {
        int len = *length - v->namelen;
        if (len > indexdimension)
            len = indexdimension;

        pal_mem_set(index, 0, sizeof(int)*indexdimension);

        if(len > 0)
            oid_copy(index, name + v->namelen, len);
    }

    return 0;
}

void
dhcrelay_snmp_index_set (struct variable *v, oid *name, size_t *length, int *index, int indexdimension)
{
    oid_copy (name + v->namelen, index, indexdimension);
    *length = v->namelen + indexdimension;
}


static int
write_dhcp_snmp_enable(int action, u_char * var_val, u_char var_val_type, 
        size_t var_val_len, u_char * statP, oid * name, size_t length,
        struct variable *v, u_int32_t vr_id)
{
    int intVal = 0;

    pal_mem_cpy(&intVal, var_val, var_val_len);
    switch (v->magic)
    {
        case DHCPENABLE:
            if (DHCP_TRUTH_VALUE_TRUE == intVal)
            {
                if (DHCPRELAY_CHK_FLAG(DHCP_SERVICE_FLAG))
                    break;
                DHCPRELAY_SET_FLAG(DHCP_SERVICE_FLAG);
            }
            else if (DHCP_TRUTH_VALUE_FALSE == intVal)
            {
                if (!DHCPRELAY_CHK_FLAG(DHCP_SERVICE_FLAG))
                    break;
                DHCPRELAY_CLR_FLAG(DHCP_SERVICE_FLAG);
            }
                
            dhcprelay_service_onoff(
                        DHCPRELAY_CHK_FLAG((DHCP_SERVICE_FLAG | DHCP_RELAY_SERVICE_FLAG)));                
            break;
        default:
            return SNMP_ERR_GENERR;
            break;
    }
    return SNMP_ERR_NOERROR;
}



static unsigned char *
dhcp_snmp_enable(struct variable *v, oid *name,
        size_t *length, int exact, size_t *var_len, 
        WriteMethod **write_method, u_int32_t vr_id)
{
    if (!v || !name || !length || !var_len || !write_method)
    {
        return NULL;
    }

    if (MATCH_FAILED == snmp_header_generic(v, name, length, exact, var_len, write_method))
    {
        return NULL;
    }

    switch(v->magic)
    {
        case DHCPENABLE:
            DHCP_SNMP_RETURN_INTEGER(DHCPRELAY_CHK_FLAG(DHCP_SERVICE_FLAG) ?  
                            DHCP_TRUTH_VALUE_TRUE: DHCP_TRUTH_VALUE_FALSE); 
            break;
        default:
            pal_assert(0);
            break;
    }

    return NULL;
}

static int
write_dhcp_relay_snmp(int action, u_char * var_val, u_char var_val_type, 
        size_t var_val_len, u_char * statP, oid * name, size_t length,
        struct variable *v, u_int32_t vr_id)
{
    int intVal = 0;
    pal_mem_cpy(&intVal, var_val, var_val_len);

    switch (v->magic)
    {
        case DHCPRENABLE:
            if (intVal == DHCP_TRUTH_VALUE_TRUE)
            {
                if (DHCPRELAY_CHK_FLAG(DHCP_RELAY_SERVICE_FLAG))
                    return SNMP_ERR_NOERROR;

                DHCPRELAY_SET_FLAG(DHCP_RELAY_SERVICE_FLAG);
                dhcprelay_service_onoff(
                    DHCPRELAY_CHK_FLAG((DHCP_SERVICE_FLAG | DHCP_RELAY_SERVICE_FLAG)));
            }
            else if (intVal == DHCP_TRUTH_VALUE_FALSE)
            {
                if (!DHCPRELAY_CHK_FLAG(DHCP_RELAY_SERVICE_FLAG))
                    return SNMP_ERR_NOERROR;

                DHCPRELAY_CLR_FLAG(DHCP_RELAY_SERVICE_FLAG);
                dhcprelay_service_onoff(
                        DHCPRELAY_CHK_FLAG((DHCP_SERVICE_FLAG | DHCP_RELAY_SERVICE_FLAG)));
            }
            break;
            
        case DHCPRINFOTRUSTALL:
            if (intVal == DHCP_TRUTH_VALUE_TRUE)
            {
                if (g_dhcprelayCfg.bTrustAllFlag)
                    break;

                g_dhcprelayCfg.bTrustAllFlag = 1;
            }
            else if (intVal == DHCP_TRUTH_VALUE_FALSE)
            {
                if (!g_dhcprelayCfg.bTrustAllFlag)
                    break;

                g_dhcprelayCfg.bTrustAllFlag = 0;
            }
            break;
            
        default:
            return SNMP_ERR_GENERR;
            break;
    }
    return SNMP_ERR_NOERROR;
}

static unsigned char *
dhcp_relay_snmp(struct variable *v, oid *name,
        size_t *length, int exact, size_t *var_len, 
        WriteMethod **write_method, u_int32_t vr_id)
{
    if (!v || !name || !length || !var_len || !write_method)
    {
        return NULL;
    }

    if (MATCH_FAILED == snmp_header_generic(v, name, length, exact, var_len, write_method))
    {
        return NULL;
    }

    switch(v->magic)
    {
        case DHCPRENABLE:
            DHCP_SNMP_RETURN_INTEGER(DHCPRELAY_CHK_FLAG(DHCP_RELAY_SERVICE_FLAG)?
                    DHCP_TRUTH_VALUE_TRUE : DHCP_TRUTH_VALUE_FALSE);            
            break;
            
        case DHCPRINFOTRUSTALL:
            DHCP_SNMP_RETURN_INTEGER(g_dhcprelayCfg.bTrustAllFlag? 
                    DHCP_TRUTH_VALUE_TRUE : DHCP_TRUTH_VALUE_FALSE);            
            break;
        default:
            break;
    }

    return NULL;
}

static int
write_dhcpRelayOption82(int action, u_char * var_val, u_char var_val_type, 
        size_t var_val_len, u_char * statP, oid * name, size_t length,
        struct variable *v, u_int32_t vr_id)
{
    int intVal = 0;

    pal_mem_cpy(&intVal, var_val, var_val_len);
    
    switch (v->magic)
    {
        case DHCPROPTION82:
            if (DHCP_TRUTH_VALUE_TRUE == intVal)
                add_agent_options = 1;
            else if (DHCP_TRUTH_VALUE_FALSE == intVal)
                add_agent_options = 0;
            break;
            
        case DHCPROPTION82CHECK:
            if (DHCP_TRUTH_VALUE_TRUE == intVal)
                drop_agent_mismatches = 1;
            else if (DHCP_TRUTH_VALUE_FALSE == intVal)
                drop_agent_mismatches = 0;
            break;
            
        case DHCPROPTION82POLICY:
            agent_relay_mode = intVal;
            break;
            
        default:
            return SNMP_ERR_GENERR;
            break;
    }
    return SNMP_ERR_NOERROR;
}

static unsigned char *
dhcpRelayOption82(struct variable *v, oid *name,
        size_t *length, int exact, size_t *var_len, 
        WriteMethod **write_method, u_int32_t vr_id)
{
    if (!v || !name || !length || !var_len || !write_method)
    {
        return NULL;
    }

    if (MATCH_FAILED == snmp_header_generic(v, name, length, exact, var_len, write_method))
    {
        return NULL;
    }

    /* Return the current value of the variable */
    switch (v->magic)
    {  
        case DHCPROPTION82:	
            DHCP_SNMP_RETURN_INTEGER(add_agent_options ? DHCP_TRUTH_VALUE_TRUE : DHCP_TRUTH_VALUE_FALSE); 
            break;
        case DHCPROPTION82CHECK:	
            DHCP_SNMP_RETURN_INTEGER(drop_agent_mismatches ? DHCP_TRUTH_VALUE_TRUE : DHCP_TRUTH_VALUE_FALSE); 
            break;
        case DHCPROPTION82POLICY:	
            DHCP_SNMP_RETURN_INTEGER(agent_relay_mode); 
            break;
            
        default:
            break;
    }
    
    return NULL;
}

/* Lookup server group */
S_DhcpServerGroup *
dhcrelay_snmp_servergroup_lookup (int *index, int exact, u_int32_t id)
{
    S_DhcpServerGroup *pGroup = NULL;

    TAILQ_FOREACH(pGroup, &g_dhcprelayCfg.groupHdr, entry)
    {
        if (exact)
        {
            if (*index == pGroup->nGroupIdx)
            {
                if (TAILQ_EMPTY(&pGroup->itemHdr))
                {
                    continue;
                }   

                /* match item */
                return pGroup;
            }
        }
        else
        {
            if (*index == 0 || pGroup->nGroupIdx > *index)
            {
                if (TAILQ_EMPTY(&pGroup->itemHdr))
                {
                    continue;
                }   
                *index = pGroup->nGroupIdx;
                return pGroup;
            }
        }
    }
    
    return NULL;
}

int
dhcpr_snmp_server_group_rowstatus (int *index, int status)
{
    S_DhcpServerGroup *pGroup = NULL;
    S_DhcpServerItem *pItem = NULL;
    char **ppTokArr = NULL;
    unsigned long nToks = 0;
    struct cli cli;
    
    cli.out_func = (CLI_OUT_FUNC) host_cli_out;

    switch (status)
    {
        case SNMP_ROW_ACTIVE:
            /* for bug 23731 modified by liuyang 2013-7-9*/
            pGroup = dhcrelay_snmp_servergroup_lookup (index, 1, 0);
            if (pGroup)
            {
                return SNMP_ERR_NOERROR;
            }
            else
            {
                return SNMP_ERR_INCONSISTENTVALUE;
            }

        case SNMP_ROW_CREATEANDGO:
            if (!g_dhcprelay_initialized) 
                return SNMP_ERR_INCONSISTENTVALUE;
                
            if (g_szServerGroup[0] == '\0')
                return SNMP_ERR_INCONSISTENTVALUE;
                
            if (split_tokens(g_szServerGroup, pal_strlen(g_szServerGroup), DHCP_RELAY_MAX_SERVER,
                             ",", &nToks, &ppTokArr) != 0)
            {
                if (NULL != ppTokArr)
                {
                    free_tokens(&ppTokArr);
                }
                pal_mem_set(&g_szServerGroup, 0, sizeof(g_szServerGroup));   
                return SNMP_ERR_INCONSISTENTVALUE;
            }

            if (dhcprelay_server_address_check(nToks, ppTokArr) != CLI_SUCCESS)
            {
                if (NULL != ppTokArr)
                {
                    free_tokens(&ppTokArr);
                }
                pal_mem_set(&g_szServerGroup, 0, sizeof(g_szServerGroup));   
                return SNMP_ERR_INCONSISTENTVALUE;
            }

            /* locate group item */
            pGroup = dhcprelay_locate_servergroup(*index);
            if (NULL != pGroup)
            {
                /* del all ip address */
                while (!TAILQ_EMPTY(&pGroup->itemHdr))
                {
                    pItem = TAILQ_FIRST(&pGroup->itemHdr);
                    TAILQ_REMOVE(&pGroup->itemHdr, pItem, entry);
                    dhcprelay_serveritem_free(pItem);
                }
            }

            dhcprelay_serverlist_add(&cli, *index, nToks, ppTokArr);
            pal_mem_set(&g_szServerGroup, 0, sizeof(g_szServerGroup));   
            break;

        case SNMP_ROW_DESTROY:
            /* locate group item */
            pGroup = dhcprelay_locate_servergroup(*index);
            if (NULL == pGroup)
            {
                return SNMP_ERR_NOERROR;
            }

            /* del all ip address */
            while (!TAILQ_EMPTY(&pGroup->itemHdr))
            {
                pItem = TAILQ_FIRST(&pGroup->itemHdr);
                TAILQ_REMOVE(&pGroup->itemHdr, pItem, entry);
                dhcprelay_serveritem_free(pItem);
            }
            pal_mem_set(g_szServerGroup, 0, sizeof(g_szServerGroup));   
            break;    
        default:
            /* for bug 23941 modified by liuyang 2013-7-16*/
            return SNMP_ERR_WRONGVALUE;
    }  

    return SNMP_ERR_NOERROR;
}

static int
write_dhcpRelayIpToGroup(int action, u_char * var_val, u_char var_val_type, 
        size_t var_val_len, u_char * statP, oid * name, size_t length,
        struct variable *v, u_int32_t vr_id)
{
    int ret = 0;
    int index;
    int status = 0;

    ret = dhcrelay_snmp_index_get (v, name, &length, &index, 1, 1);

    switch(v->magic)
    {
        case DHCPRGROUPSERVERIP:
            pal_mem_set(&g_szServerGroup, 0, sizeof(g_szServerGroup));   
            pal_mem_cpy(g_szServerGroup, var_val, pal_strlen((char *)var_val));
            break;
            
        case DHCPRGROUPSTATUS:
            pal_mem_cpy(&status, var_val, sizeof(int));
            return dhcpr_snmp_server_group_rowstatus(&index, status);

        default:
            return SNMP_ERR_GENERR;
    }   
    return SNMP_ERR_NOERROR;
}

static unsigned char *
dhcpRelayIpToGroup(struct variable *v, oid *name,
        size_t *length, int exact, size_t *var_len, 
        WriteMethod **write_method, u_int32_t vr_id)
{
    int index = 0;
    int ret;
    S_DhcpServerGroup *pGroup = NULL;
    char szAddrBuf[SU_ADDRSTRLEN + 1];
    S_DhcpServerItem *pItem = NULL;
    char strbuf[1024];
    int len;
    
    ret = dhcrelay_snmp_index_get (v, name, length, &index, 1, exact);
    if (ret < 0) 
        return NULL;
    if (exact && (index < 1 || index > DHCP_RELAY_MAX_GROUP))
    {
        return NULL;
    }
   
    /* Return the current value of the variable */
    switch (v->magic)
    {
        case DHCPRGROUPGROUPID:
            pGroup = dhcrelay_snmp_servergroup_lookup (&index, exact, vr_id);
            if (!pGroup)        
                return NULL;

            if(!exact)
            {
                index = pGroup->nGroupIdx;
                dhcrelay_snmp_index_set (v, name, length, &index, 1);
            }
            break;
        case DHCPRGROUPSERVERIP:	
            pGroup = dhcrelay_snmp_servergroup_lookup (&index, exact, vr_id);
            if (!pGroup)        
                return NULL;

            if(!exact)
            {
                index = pGroup->nGroupIdx;
                dhcrelay_snmp_index_set (v, name, length, &index, 1);
            }
            
            strbuf[0] = '\0';
            TAILQ_FOREACH(pItem, &pGroup->itemHdr, entry)
            {
                pal_strcat(strbuf, sockunion2str(&pItem->su, szAddrBuf, SU_ADDRSTRLEN + 1));
                pal_strcat(strbuf, ",");
            }
            len = pal_strlen(strbuf);
            strbuf[len-1] = '\0';
            DHCP_SNMP_RETURN_STRING(strbuf);
            break;
        case DHCPRGROUPSTATUS:
            pGroup = dhcrelay_snmp_servergroup_lookup (&index, exact, vr_id);
            if (!pGroup)        
                return NULL;

            if(!exact)
            {
                index = pGroup->nGroupIdx;
                dhcrelay_snmp_index_set (v, name, length, &index, 1);
            }
            DHCP_SNMP_RETURN_INTEGER(DHCP_SNMP_ROW_STATUS_ACTIVE);
        default:
            break;
    }
    
    return NULL;


}

static int
write_dhcpRStatisticsEntry(int action, u_char * var_val, u_char var_val_type, 
        size_t var_val_len, u_char * statP, oid * name, size_t length,
        struct variable *v, u_int32_t vr_id)
{
    int intVal = 0;
    pal_mem_cpy(&intVal, var_val, var_val_len);
    
    switch (v->magic)
    {
        case DHCPRRXSTATISTICSRESET:               
            client_packets_relayed = 0;
            server_packets_relayed = 0;
            client_packet_errors = 0;
            server_packet_errors = 0;
            bogus_giaddr_drops = 0;
            bad_circuit_id = 0;
            corrupt_agent_options = 0;
            missing_agent_option = 0;
            missing_circuit_id = 0;
            break;
            
        default:
            return SNMP_ERR_GENERR;
            break;
    }
    return SNMP_ERR_NOERROR;
}

static unsigned char *
dhcpRStatisticsEntry(struct variable *v, oid *name,
        size_t *length, int exact, size_t *var_len, 
        WriteMethod **write_method, u_int32_t vr_id)
{
    if (!v || !name || !length || !var_len || !write_method)
    {
        return NULL;
    }

    if (MATCH_FAILED == snmp_header_generic(v, name, length, exact, var_len, write_method))
    {
        return NULL;
    }

    switch(v->magic)
    {
        case DHCPRRXCLIENTPKTNUM:
            DHCP_SNMP_RETURN_INTEGER(client_packets_relayed); 
            break;
        case DHCPRRXSERVERPKTNUM:
            DHCP_SNMP_RETURN_INTEGER(server_packets_relayed); 
            break;
        case DHCPRRXCLIENTERRPKTNUM:
            DHCP_SNMP_RETURN_INTEGER(client_packet_errors); 
            break;
        case DHCPRRXSERVERERRPKTNUM:
            DHCP_SNMP_RETURN_INTEGER(server_packet_errors); 
            break;
        case DHCPRRXBOGUSPKTNUM:
            DHCP_SNMP_RETURN_INTEGER(bogus_giaddr_drops); 
            break;
        case DHCPRRXBADIDPKTNUM:
            DHCP_SNMP_RETURN_INTEGER(bad_circuit_id); 
            break;
        case DHCPRRXCORRUPTPKTNUM:
            DHCP_SNMP_RETURN_INTEGER(corrupt_agent_options); 
            break;
        case DHCPRRXMISSAGENTOPPKTNUM:
            DHCP_SNMP_RETURN_INTEGER(missing_agent_option); 
            break;
        case DHCPRRXMISSCIRCUITIDPKTNUM:
            DHCP_SNMP_RETURN_INTEGER(missing_circuit_id); 
            break;
        case DHCPRRXSTATISTICSRESET:
            DHCP_SNMP_RETURN_INTEGER(0); 
            break;
        default:
            pal_assert(0);
            break;
    }

    return NULL;
}

static int
dhcp_snmp_enable_value_check(struct variable *v, uint8 *var_val, uint32 var_val_len)
{
    int int_val = 0;
    pal_mem_cpy(&int_val, var_val, var_val_len);
    switch(v->magic)
    {
        case DHCPENABLE:
            if (DHCP_TRUTH_VALUE_TRUE != int_val && DHCP_TRUTH_VALUE_FALSE != int_val)
            {
               return SNMP_ERR_WRONGVALUE;
            }
            break;

        default:
            return SNMP_ERR_WRONGVALUE;          
    }
    return SNMP_ERR_NOERROR;
}

static int
dhcp_snmp_enable_index_check(struct variable *v, oid *name, uint32 length)
{
    int index;
    int ret;
    ret = dhcrelay_snmp_index_get(v, name, &length, &index, 1, 1);
    if(ret != 0 || index != 0)
    {
        return SNMP_FAIL;
    }
    return SNMP_SUCCESS;
}


static int
dhcp_relay_snmp_value_check(struct variable *v, uint8 *var_val, uint32 var_val_len)
{
    int int_val = 0;
    pal_mem_cpy(&int_val, var_val, var_val_len);
    switch(v->magic)
    {
        case DHCPENABLE:
        case DHCPRINFOTRUSTALL:
            if (DHCP_TRUTH_VALUE_TRUE != int_val && DHCP_TRUTH_VALUE_FALSE != int_val)
            {
               return SNMP_ERR_WRONGVALUE;
            }
            break;

        default:
            return SNMP_ERR_WRONGVALUE;          
    }
    return SNMP_ERR_NOERROR;
}

static int
dhcp_relay_snmp_index_check(struct variable *v, oid *name, uint32 length)
{
    int index;
    int ret;
    ret = dhcrelay_snmp_index_get(v, name, &length, &index, 1, 1);
    if(ret != 0 || index != 0)
    {
        return SNMP_FAIL;
    }
    return SNMP_SUCCESS;
}

static int
dhcpRelayOption82_value_check(struct variable *v, uint8 *var_val, uint32 var_val_len)
{
    int int_val = 0;
    pal_mem_cpy(&int_val, var_val, var_val_len);
    switch(v->magic)
    {
        case DHCPROPTION82:
        case DHCPROPTION82CHECK:
            if (DHCP_TRUTH_VALUE_TRUE != int_val && DHCP_TRUTH_VALUE_FALSE != int_val)
            {
               return SNMP_ERR_WRONGVALUE;
            }
            break;
            
        case DHCPROPTION82POLICY:
            if (int_val > forward_and_append && int_val <= discard)
            {
               return SNMP_ERR_NOERROR;
            }
            return SNMP_ERR_WRONGVALUE;
        default:
            return SNMP_ERR_WRONGVALUE;          
    }
    return SNMP_ERR_NOERROR;
}

static int
dhcpRelayOption82_index_check(struct variable *v, oid *name, uint32 length)
{
    int index;
    int ret;
    ret = dhcrelay_snmp_index_get(v, name, &length, &index, 1, 1);
    if(ret != 0 || index != 0)
    {
        return SNMP_FAIL;
    }
    return SNMP_SUCCESS;
}

static int
dhcpRelayIpToGroup_value_check(struct variable *v, uint8 *var_val, uint32 var_val_len)
{
    char **ppTokArr = NULL;
    unsigned long nToks = 0;
    int intval;
    switch(v->magic)
    {
        case DHCPRGROUPSERVERIP:
            if (split_tokens((char *)var_val, var_val_len, DHCP_RELAY_MAX_SERVER, ",", &nToks, &ppTokArr) != 0)
            {
                if (NULL != ppTokArr)
                {
                    free_tokens(&ppTokArr);
                }
                return SNMP_ERR_WRONGVALUE;
            }
            if (dhcprelay_server_address_check(nToks, ppTokArr) != CLI_SUCCESS)
            {
                if (NULL != ppTokArr)
                {
                    free_tokens(&ppTokArr);
                }
                return SNMP_ERR_WRONGVALUE;
            }
            if (NULL != ppTokArr)
            {
                free_tokens(&ppTokArr);
            }
            break;
            
        case DHCPRGROUPSTATUS:
            pal_mem_cpy(&intval, var_val, 4);
            if ((SNMP_ROW_ACTIVE == intval) || (SNMP_ROW_CREATEANDGO == intval) || (SNMP_ROW_DESTROY == intval))
            {
                return SNMP_ERR_NOERROR;
            }
            return SNMP_ERR_WRONGVALUE;
            
        default:
            return SNMP_ERR_WRONGVALUE;          
    }
    return SNMP_ERR_NOERROR;
}

static int
dhcpRelayIpToGroup_index_check(struct variable *v, oid *name, uint32 length)
{
    int ret = 0;
    int index;
    ret = dhcrelay_snmp_index_get (v, name, &length, &index, 1, 1);
    if (ret < 0) 
    {
        return SNMP_FAIL;
    }

    if (index > 16 || index <= 0)
    {
        return SNMP_FAIL;
    }
    return SNMP_SUCCESS;

}

static int
dhcpRStatisticsEntry_value_check(struct variable *v, uint8 *var_val, uint32 var_val_len)
{
    int int_val = 0;
    pal_mem_cpy(&int_val, var_val, var_val_len);
    switch(v->magic)
    {
        case DHCPRRXSTATISTICSRESET:
            if (DHCP_TRUTH_VALUE_TRUE != int_val)
            {
               return SNMP_ERR_WRONGVALUE;
            }
            break;

        default:
            return SNMP_ERR_WRONGVALUE;          
    }
    return SNMP_ERR_NOERROR;
}

static int
dhcpRStatisticsEntry_index_check(struct variable *v, oid *name, uint32 length)
{
    int ret = 0;
    int index;
    ret = dhcrelay_snmp_index_get (v, name, &length, &index, 1, 1);
    if (ret < 0 || index != 0) 
    {
        return SNMP_FAIL;
    }
    return SNMP_SUCCESS;
}


struct variable dhcp_variables[] = 
{
    /* centecDhcpEnable */
    {DHCPENABLE, INTEGER, RWRITE, dhcp_snmp_enable,
     1, {1},dhcp_snmp_enable_value_check, dhcp_snmp_enable_index_check,
     write_dhcp_snmp_enable}
};

/*for bug 23726 remove node and func about intf attribute  by liuyang 2013-7-11*/
struct variable dhcpr_global_variables[] = 
{
      /* centecDhcpREnable. */
    {DHCPRENABLE, INTEGER, RWRITE, dhcp_relay_snmp,
     1, {1}, dhcp_relay_snmp_value_check, dhcp_relay_snmp_index_check,
     write_dhcp_relay_snmp},
      /* centecDhcpRInfoOptionTrustAll. */
    {DHCPRINFOTRUSTALL, TRUTHVALUE, RWRITE, dhcp_relay_snmp,
     1, {2}, dhcp_relay_snmp_value_check, dhcp_relay_snmp_index_check,
     write_dhcp_relay_snmp}
};


struct variable dhcpr_server_variables[] = 
{
      /* centecDhcpRInfoOption. */
    {DHCPROPTION82,         INTEGER, RWRITE, dhcpRelayOption82,
     1, {1}, dhcpRelayOption82_value_check, dhcpRelayOption82_index_check,
     write_dhcpRelayOption82},

      /* centecDhcpRInfoOption. */
    {DHCPROPTION82CHECK,         INTEGER, RWRITE, dhcpRelayOption82,
     1, {2}, dhcpRelayOption82_value_check, dhcpRelayOption82_index_check,
     write_dhcpRelayOption82},
      /* centecDhcpRInfoPolicy. */
    {DHCPROPTION82POLICY,         INTEGER, RWRITE, dhcpRelayOption82,
     1, {3}, dhcpRelayOption82_value_check, dhcpRelayOption82_index_check,
     write_dhcpRelayOption82},
     
      /* centecDhcpRIpToGroupTable. */
    {DHCPRGROUPGROUPID,         INTEGER, NOACCESS, dhcpRelayIpToGroup,
     3, {4, 1, 1}, dhcpRelayIpToGroup_value_check, dhcpRelayIpToGroup_index_check,
     write_dhcpRelayIpToGroup},
    {DHCPRGROUPSERVERIP,          STRING, RWRITE, dhcpRelayIpToGroup,
     3, {4, 1, 2}, dhcpRelayIpToGroup_value_check, dhcpRelayIpToGroup_index_check,
     write_dhcpRelayIpToGroup},
    {DHCPRGROUPSTATUS,     INTEGER, RWRITE, dhcpRelayIpToGroup,
     3, {4, 1, 3}, dhcpRelayIpToGroup_value_check, dhcpRelayIpToGroup_index_check,
     write_dhcpRelayIpToGroup},
     
    /* centecDhcpRStatisticsGroup. */
    {DHCPRRXCLIENTPKTNUM,           INTEGER, RONLY, dhcpRStatisticsEntry,
     2, {5, 1}, dhcpRStatisticsEntry_value_check, dhcpRStatisticsEntry_index_check,
     write_dhcpRStatisticsEntry},
    {DHCPRRXSERVERPKTNUM,            INTEGER, RONLY, dhcpRStatisticsEntry,
     2, {5, 2}, dhcpRStatisticsEntry_value_check, dhcpRStatisticsEntry_index_check,
     write_dhcpRStatisticsEntry},
    {DHCPRRXCLIENTERRPKTNUM,        INTEGER, RONLY, dhcpRStatisticsEntry,
     2, {5, 3}, dhcpRStatisticsEntry_value_check, dhcpRStatisticsEntry_index_check,
     write_dhcpRStatisticsEntry},
    {DHCPRRXSERVERERRPKTNUM,           INTEGER, RONLY, dhcpRStatisticsEntry,
     2, {5, 4}, dhcpRStatisticsEntry_value_check, dhcpRStatisticsEntry_index_check,
     write_dhcpRStatisticsEntry},
    {DHCPRRXBOGUSPKTNUM,     INTEGER, RONLY, dhcpRStatisticsEntry,
     2, {5, 5}, dhcpRStatisticsEntry_value_check, dhcpRStatisticsEntry_index_check,
     write_dhcpRStatisticsEntry},
    {DHCPRRXBADIDPKTNUM,           INTEGER, RONLY, dhcpRStatisticsEntry,
     2, {5, 6}, dhcpRStatisticsEntry_value_check, dhcpRStatisticsEntry_index_check,
     write_dhcpRStatisticsEntry},
    {DHCPRRXCORRUPTPKTNUM,           INTEGER, RONLY, dhcpRStatisticsEntry,
     2, {5, 7}, dhcpRStatisticsEntry_value_check, dhcpRStatisticsEntry_index_check,
     write_dhcpRStatisticsEntry},
    {DHCPRRXMISSAGENTOPPKTNUM,           INTEGER, RONLY, dhcpRStatisticsEntry,
     2, {5, 8}, dhcpRStatisticsEntry_value_check, dhcpRStatisticsEntry_index_check,
     write_dhcpRStatisticsEntry},
    {DHCPRRXMISSCIRCUITIDPKTNUM,           INTEGER, RONLY, dhcpRStatisticsEntry,
     2, {5, 9}, dhcpRStatisticsEntry_value_check, dhcpRStatisticsEntry_index_check,
     write_dhcpRStatisticsEntry},
    {DHCPRRXSTATISTICSRESET,           TRUTHVALUE, RWRITE, dhcpRStatisticsEntry,
     2, {5, 10}, dhcpRStatisticsEntry_value_check, dhcpRStatisticsEntry_index_check,
     write_dhcpRStatisticsEntry}};




/* Register DHCP-MIB. */
void
dhcrelay_snmp_init (struct lib_globals *zg)
{
    REGISTER_MIB_MULTIPM(zg, "dhcp", dhcp_variables, variable, dhcp_oid, zg->protocol);
    REGISTER_MIB(zg, "dhcprelayserver", dhcpr_server_variables, variable, dhcpr_server_oid);
    REGISTER_MIB_MULTIPM(zg, "dhcprglobal", dhcpr_global_variables, variable, dhcpr_global_oid, zg->protocol);
    snmp_start (zg);
}

#ifdef HAVE_IPV6

static int
dhcpv6_relay_snmp_value_check(struct variable *v, uint8 *var_val, uint32 var_val_len)
{
    int intval = 0;

    if(var_val_len < 4)
    {
        return SNMP_ERR_WRONGVALUE;
    }
    intval = *(int *)var_val;

    switch (v->magic)
    {
        case DHCPV6ENABLE:
        case DHCPV6RENABLE:
        case DHCPV6RREMOTEID:
        case DHCPV6RREMOTEIDDUID:
        case DHCPV6RREMOTEIDIFNAME:
        case DHCPV6RREMOTEIDVLAN:
        case DHCPV6RPDENABLE:
            if (DHCP_TRUTH_VALUE_TRUE != intval && DHCP_TRUTH_VALUE_FALSE != intval)
            {
               return SNMP_ERR_WRONGVALUE;
            }
            break;
            
        case DHCPV6RPDDISTANCE:
            if (intval > 255 || intval < 1)
            {
               return SNMP_ERR_WRONGVALUE;
            }
            break; 
            
        case DHCPV6RPDALLCLEAR:
        case DHCPV6RRXSTATISTICSRESET:
            if (DHCP_TRUTH_VALUE_TRUE != intval)
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;
            
        case DHCPV6RGROUPSTATUS:
            if ((SNMP_ROW_ACTIVE != intval) 
                && (SNMP_ROW_CREATEANDGO != intval) 
                && (SNMP_ROW_DESTROY != intval))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case DHCPV6RPDSTATUS:
            if ((SNMP_ROW_ACTIVE != intval) 
                && (SNMP_ROW_DESTROY != intval))
            {
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        default:
            break;
    }

    return SNMP_ERR_NOERROR;
}

static int
dhcpv6_relay_snmp_index_check(struct variable *v, oid *name, uint32 length)
{
    int ret;
    int index;

    switch (v->magic)
    {
        case DHCPV6RGROUPSERVERIPIF:
        case DHCPV6RGROUPSTATUS:
            ret = dhcrelay_snmp_index_get (v, name, &length, &index, 1, 1);
            if (ret < 0) 
            {
                return SNMP_FAIL;
            }

            if (index > 16 || index <= 0)
            {
                return SNMP_FAIL;
            }
            break;
        default:
            break;
    }

    return SNMP_SUCCESS;
}

int
write_dhcpv6_snmp_enable(int action, u_char * var_val, u_char var_val_type, 
        size_t var_val_len, u_char * statP, oid * name, size_t length,
        struct variable *v, u_int32_t vr_id)
{
    int intVal = 0;

    pal_mem_cpy(&intVal, var_val, var_val_len);
    switch (v->magic)
    {
        case DHCPV6ENABLE:
            if (DHCP_TRUTH_VALUE_TRUE == intVal)
            {
                if (DHCPRELAY6_CHK_FLAG(DHCPV6_SERVICE_FLAG))
                    break;
                DHCPRELAY6_SET_FLAG(DHCPV6_SERVICE_FLAG);
            }
            else if (DHCP_TRUTH_VALUE_FALSE == intVal)
            {
                if (!DHCPRELAY6_CHK_FLAG(DHCPV6_SERVICE_FLAG))
                    break;
                DHCPRELAY6_CLR_FLAG(DHCPV6_SERVICE_FLAG);
            }
                
            dhcprelay6_service_onoff(
                        DHCPRELAY6_CHK_FLAG((DHCPV6_SERVICE_FLAG | DHCPV6_RELAY_SERVICE_FLAG)));                
            break;
        default:
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}


static unsigned char *
dhcpv6_snmp_enable(struct variable *v, oid *name,
        size_t *length, int exact, size_t *var_len, 
        WriteMethod **write_method, u_int32_t vr_id)
{
    if (!v || !name || !length || !var_len || !write_method)
    {
        return NULL;
    }

    if (MATCH_FAILED == snmp_header_generic(v, name, length, exact, var_len, write_method))
    {
        return NULL;
    }

    switch(v->magic)
    {
        case DHCPV6ENABLE:
            DHCP_SNMP_RETURN_INTEGER(DHCPRELAY6_CHK_FLAG(DHCPV6_SERVICE_FLAG) ?  
                            DHCP_TRUTH_VALUE_TRUE: DHCP_TRUTH_VALUE_FALSE); 
            break;
        default:
            pal_assert(0);
            break;
    }

    return NULL;
}

static int
write_dhcpv6_relay_snmp(int action, u_char * var_val, u_char var_val_type, 
        size_t var_val_len, u_char * statP, oid * name, size_t length,
        struct variable *v, u_int32_t vr_id)
{
    int intVal = 0;

    switch (v->magic)
    {
        case DHCPV6RENABLE:
            pal_mem_cpy(&intVal, var_val, var_val_len);
            if (intVal == DHCP_TRUTH_VALUE_TRUE)
            {
                if (DHCPRELAY6_CHK_FLAG(DHCPV6_RELAY_SERVICE_FLAG))
                    break;
                DHCPRELAY6_SET_FLAG(DHCPV6_RELAY_SERVICE_FLAG);
                dhcprelay6_service_onoff(
                    DHCPRELAY6_CHK_FLAG((DHCPV6_SERVICE_FLAG | DHCPV6_RELAY_SERVICE_FLAG)));
            }
            else if (intVal == DHCP_TRUTH_VALUE_FALSE)
            {
                if (!DHCPRELAY6_CHK_FLAG(DHCPV6_RELAY_SERVICE_FLAG))
                    break;

                DHCPRELAY6_CLR_FLAG(DHCPV6_RELAY_SERVICE_FLAG);
                dhcprelay6_service_onoff(
                    DHCPRELAY6_CHK_FLAG((DHCPV6_SERVICE_FLAG | DHCPV6_RELAY_SERVICE_FLAG)));
            }
            break;
        default:
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

static unsigned char *
dhcpv6_relay_snmp(struct variable *v, oid *name,
        size_t *length, int exact, size_t *var_len, 
        WriteMethod **write_method, u_int32_t vr_id)
{
    if (!v || !name || !length || !var_len || !write_method)
    {
        return NULL;
    }

    if (MATCH_FAILED == snmp_header_generic(v, name, length, exact, var_len, write_method))
    {
        return NULL;
    }

    switch(v->magic)
    {
        case DHCPV6RENABLE:
            DHCP_SNMP_RETURN_INTEGER(DHCPRELAY6_CHK_FLAG(DHCPV6_RELAY_SERVICE_FLAG)?
                    DHCP_TRUTH_VALUE_TRUE : DHCP_TRUTH_VALUE_FALSE);            
            break;
        default:
            break;
    }

    return NULL;
}

/* Lookup server group */
S_Dhcpv6ServerGroup *
dhcrelay6_snmp_servergroup_lookup (int *index, int exact)
{
    S_Dhcpv6ServerGroup *pGroup = NULL;
    /* list be already sorted*/
    TAILQ_FOREACH(pGroup, &g_dhcprelay6Cfg.groupHdr, entry)
    {
        if (exact)
        {
            if (*index == pGroup->nGroupIdx)
            {
                if (TAILQ_EMPTY(&pGroup->itemHdr))
                {
                    continue;
                }   

                /* match item */
                return pGroup;
            }
        }
        else
        {
            if (*index == 0 || pGroup->nGroupIdx > *index)
            {
                if (TAILQ_EMPTY(&pGroup->itemHdr))
                {
                    continue;
                }   
                return pGroup;
            }
        }
    }
    
    return NULL;
}

int dhcpv6_relay_server_item_check(unsigned long addr_num, char **addr)
{
    char *ifname = NULL;
    unsigned long nIdx = 0;
    char *p = NULL;
    char *delim = NULL;
    int ret = 0;
    int i = 0;
    char buf[128];
   
    for (nIdx = 0; nIdx < addr_num; nIdx++)
    {
        if (NULL == (p = addr[nIdx]))
        {
            continue;
        }

        ifname = NULL;
        pal_strncpy(buf, addr[nIdx], 128);

        if (NULL != (delim = pal_strchr(addr[nIdx], '-')))
        {
            ifname = buf + (delim - addr[nIdx] + 1);  

            for (i = 0; i < pal_strlen(ifname); i++)
            {
                ifname[i] = pal_char_tolower(ifname[i]);
            }

            if (!if_nametoindex(ifname) 
                || (pal_strncmp(ifname, "eth-", 4)
                && pal_strncmp(ifname, "vlan", 4) 
                && pal_strncmp(ifname, "agg", 3)))
            {
                return CLI_ERROR;
            }

            *(ifname - 1) = '\0';
        } 

        ret = dhcprelay6_server_address_check(buf);
        if (ret == CLI_ERROR
            ||(ret == -2 && !ifname)
            ||(ret == CLI_SUCCESS && ifname))
        {
            return CLI_ERROR;
        }
    }
    return CLI_SUCCESS;
}

int dhcpv6_relay_server_item_add(S_Dhcpv6ServerGroup *pGroup, unsigned long addr_num, char **addr)
{
    char *ifname = NULL;
    unsigned long nIdx = 0;
    char *p = NULL;
    int ret = 0;
    int i = 0;
    char *delim = NULL;
    char buf[128];
    
    for (nIdx = 0; nIdx < addr_num; nIdx++)
    {
        if (NULL == (p = addr[nIdx]))
        {
            continue;
        }

        ifname = NULL;
        pal_strncpy(buf, addr[nIdx], 128);

        if (NULL != (delim = pal_strchr(addr[nIdx], '-')))
        {
            ifname = buf + (delim - addr[nIdx] + 1);
            
            for (i = 0; i < pal_strlen(ifname); i++)
            {
                ifname[i] = pal_char_tolower(ifname[i]);
            }

            *(ifname - 1) = '\0';
        }        
  
        ret = dhcprelay6_serveritem_add(pGroup, buf, ifname);
        if (ret)
        {
            return ret;
        }
    }
    return CLI_SUCCESS;
}

int
dhcpv6_relay_snmp_server_group_rowstatus (int *index, int status)
{
    S_Dhcpv6ServerGroup *pGroup = NULL;
    S_Dhcpv6ServerItem *pItem = NULL;
    char **ppTokArr_addr = NULL;
    unsigned long nToks_addr = 0;

    switch (status)
    {
        case SNMP_ROW_ACTIVE:
            return SNMP_ERR_NOERROR;

        case SNMP_ROW_CREATEANDGO:
            if (!g_dhcprelay6_initialized) 
                return SNMP_ERR_INCONSISTENTVALUE;
                
            if (g_szServerGroupIp6[0] == '\0')
                return SNMP_ERR_INCONSISTENTVALUE;
                
            if (split_tokens(g_szServerGroupIp6, pal_strlen(g_szServerGroupIp6), DHCPV6_RELAY_MAX_SERVER,
                             ",", &nToks_addr, &ppTokArr_addr) )
            {
                if (NULL != ppTokArr_addr)
                {
                    free_tokens(&ppTokArr_addr);
                }
                pal_mem_set(&g_szServerGroupIp6, 0, sizeof(g_szServerGroupIp6));   
                return SNMP_ERR_INCONSISTENTVALUE;
            }

            if (dhcpv6_relay_server_item_check(nToks_addr, ppTokArr_addr) != CLI_SUCCESS)
            {          
                if (NULL != ppTokArr_addr)
                {
                    free_tokens(&ppTokArr_addr);
                }
                pal_mem_set(&g_szServerGroupIp6, 0, sizeof(g_szServerGroupIp6));   
                return SNMP_ERR_INCONSISTENTVALUE;
            }

            /* locate group item */
            pGroup = dhcprelay6_locate_servergroup(*index);
            if (NULL != pGroup)
            {
                /* del all ip address */
                while (!TAILQ_EMPTY(&pGroup->itemHdr))
                {
                    pItem = TAILQ_FIRST(&pGroup->itemHdr);
                    TAILQ_REMOVE(&pGroup->itemHdr, pItem, entry);
                    dhcprelay6_serveritem_free(pItem);
                }
            }
            else
            {
                return SNMP_ERR_INCONSISTENTVALUE;
            }

            /* add ipv6 address */
            if (dhcpv6_relay_server_item_add(pGroup, nToks_addr, ppTokArr_addr) != CLI_SUCCESS)
            {
                return SNMP_ERR_INCONSISTENTVALUE;
            }
            pal_mem_set(&g_szServerGroupIp6, 0, sizeof(g_szServerGroupIp6));   
            break;

        case SNMP_ROW_DESTROY:
            /* locate group item */
            pGroup = dhcprelay6_locate_servergroup(*index);
            if (NULL == pGroup)
            {
                return SNMP_ERR_NOERROR;
            }

            /* del all ip address */
            while (!TAILQ_EMPTY(&pGroup->itemHdr))
            {
                pItem = TAILQ_FIRST(&pGroup->itemHdr);
                TAILQ_REMOVE(&pGroup->itemHdr, pItem, entry);
                dhcprelay6_serveritem_free(pItem);
            }
            pal_mem_set(&g_szServerGroupIp6, 0, sizeof(g_szServerGroupIp6));    
            break;    
        default:
            return SNMP_ERR_GENERR;
    }  

    return SNMP_ERR_NOERROR;
}

static int
write_dhcpv6_relay_srvgrp_snmp(int action, u_char * var_val, u_char var_val_type, 
        size_t var_val_len, u_char * statP, oid * name, size_t length,
        struct variable *v, u_int32_t vr_id)
{
    int ret = 0;
    int index;
    int status = 0;

    ret = dhcrelay_snmp_index_get (v, name, &length, &index, 1, 1);

    switch(v->magic)
    {
        case DHCPV6RGROUPSERVERIPIF:
            pal_mem_set(&g_szServerGroupIp6, 0, sizeof(g_szServerGroupIp6));   
            pal_mem_cpy(g_szServerGroupIp6, var_val, pal_strlen((char *)var_val));
            break;

       
        case DHCPV6RGROUPSTATUS:
            pal_mem_cpy(&status, var_val, sizeof(int));
            return dhcpv6_relay_snmp_server_group_rowstatus(&index, status);

        default:
            return SNMP_ERR_GENERR;
    }   
    return SNMP_ERR_NOERROR;
}

static unsigned char *
dhcpv6_relay_srvgrp_snmp(struct variable *v, oid *name,
        size_t *length, int exact, size_t *var_len, 
        WriteMethod **write_method, u_int32_t vr_id)
{
    int index = 0;
    int ret;
    S_Dhcpv6ServerGroup *pGroup = NULL;
    char szAddrBuf[SU_ADDRSTRLEN + 1];
    S_Dhcpv6ServerItem *pItem = NULL;
    char strbuf[1024];
    int len;
    
    ret = dhcrelay_snmp_index_get (v, name, length, &index, 1, exact);
    if (ret < 0)
    {
        return NULL;
    }
    if (exact && (index < 1 || index > DHCP_RELAY_MAX_GROUP))
    {
        return NULL;
    }

    pGroup = dhcrelay6_snmp_servergroup_lookup (&index, exact);
    if (!pGroup)        
        return NULL;

    if(!exact)
    {
        index = pGroup->nGroupIdx;
        dhcrelay_snmp_index_set (v, name, length, &index, 1);
    }
   
    /* Return the current value of the variable */
    switch (v->magic)
    {
        case DHCPV6RGROUPSERVERIPIF:	
            strbuf[0] = '\0';
            TAILQ_FOREACH(pItem, &pGroup->itemHdr, entry)
            {
                pal_strcat(strbuf, sockunion2str(&pItem->su, szAddrBuf, SU_ADDRSTRLEN + 1));
                if ('\0' != pItem->name[0])
                {
                    pal_strcat(strbuf, "-");
                    pal_strcat(strbuf, pItem->name);                    
                }
                pal_strcat(strbuf, ",");
            }
            len = pal_strlen(strbuf);
            strbuf[len-1] = '\0';
            DHCP_SNMP_RETURN_STRING(strbuf);
            break;
  
        case DHCPV6RGROUPSTATUS:
            DHCP_SNMP_RETURN_INTEGER(DHCP_SNMP_ROW_STATUS_ACTIVE);
        default:
            break;
    }
    
    return NULL;
}

static int
write_dhcpv6_relay_remoteid_snmp(int action, u_char * var_val, u_char var_val_type, 
        size_t var_val_len, u_char * statP, oid * name, size_t length,
        struct variable *v, u_int32_t vr_id)
{
    int intVal = 0;

    pal_mem_cpy(&intVal, var_val, var_val_len);
    
    switch (v->magic)
    {
        case DHCPV6RREMOTEID:
            if (DHCP_TRUTH_VALUE_TRUE == intVal)
            {
                if (DHCPRELAY6_CHK_FLAG(DHCPRELAY6_REMOTEID_OPTION_FLAG))
                {
                    return SNMP_ERR_NOERROR;
                }
                DHCPRELAY6_SET_FLAG(DHCPRELAY6_REMOTEID_OPTION_FLAG);
            }
            else if (DHCP_TRUTH_VALUE_FALSE == intVal)
            {
                if (!DHCPRELAY6_CHK_FLAG(DHCPRELAY6_REMOTEID_OPTION_FLAG))
                {
                    return SNMP_ERR_NOERROR;
                }
                DHCPRELAY6_CLR_FLAG(DHCPRELAY6_REMOTEID_OPTION_FLAG);
            }
            break;
            
        case DHCPV6RREMOTEIDDUID:
            if (DHCP_TRUTH_VALUE_TRUE == intVal)
            {
                g_dhcprelay6_remoteid_format |= DHCPRELAY6_REMOTE_ID_KEYWORD_DUID;
            }
            else if (DHCP_TRUTH_VALUE_FALSE == intVal)
            {
                g_dhcprelay6_remoteid_format &= ~DHCPRELAY6_REMOTE_ID_KEYWORD_DUID;                
            }
            break;
            
        case DHCPV6RREMOTEIDIFNAME:
            if (DHCP_TRUTH_VALUE_TRUE == intVal)
            {
                g_dhcprelay6_remoteid_format |= DHCPRELAY6_REMOTE_ID_KEYWORD_IFNAME;
            }
            else if (DHCP_TRUTH_VALUE_FALSE == intVal)
            {
                g_dhcprelay6_remoteid_format &= ~DHCPRELAY6_REMOTE_ID_KEYWORD_IFNAME;                
            }
            break;

        case DHCPV6RREMOTEIDVLAN:
            if (DHCP_TRUTH_VALUE_TRUE == intVal)
            {
                g_dhcprelay6_remoteid_format |= DHCPRELAY6_REMOTE_ID_KEYWORD_VLAN;
            }
            else if (DHCP_TRUTH_VALUE_FALSE == intVal)
            {
                g_dhcprelay6_remoteid_format &= ~DHCPRELAY6_REMOTE_ID_KEYWORD_VLAN;                
            }
            break;

        default:
            return SNMP_ERR_GENERR;
            break;
    }
    return SNMP_ERR_NOERROR;
}

static unsigned char *
dhcpv6_relay_remoteid_snmp(struct variable *v, oid *name,
        size_t *length, int exact, size_t *var_len, 
        WriteMethod **write_method, u_int32_t vr_id)
{
    if (!v || !name || !length || !var_len || !write_method)
    {
        return NULL;
    }

    if (MATCH_FAILED == snmp_header_generic(v, name, length, exact, var_len, write_method))
    {
        return NULL;
    }

    /* Return the current value of the variable */
    switch (v->magic)
    {  
         case DHCPV6RREMOTEID:
            DHCP_SNMP_RETURN_INTEGER(DHCPRELAY6_CHK_FLAG(DHCPRELAY6_REMOTEID_OPTION_FLAG) ?  
                            DHCP_TRUTH_VALUE_TRUE: DHCP_TRUTH_VALUE_FALSE); 
            break;
            
        case DHCPV6RREMOTEIDDUID:
            DHCP_SNMP_RETURN_INTEGER(
                            (g_dhcprelay6_remoteid_format & DHCPRELAY6_REMOTE_ID_KEYWORD_DUID) ?  
                            DHCP_TRUTH_VALUE_TRUE: DHCP_TRUTH_VALUE_FALSE); 
            break;
            
        case DHCPV6RREMOTEIDIFNAME:
            DHCP_SNMP_RETURN_INTEGER(
                            (g_dhcprelay6_remoteid_format & DHCPRELAY6_REMOTE_ID_KEYWORD_IFNAME) ?  
                            DHCP_TRUTH_VALUE_TRUE: DHCP_TRUTH_VALUE_FALSE); 
            break;

        case DHCPV6RREMOTEIDVLAN:
            DHCP_SNMP_RETURN_INTEGER(
                            (g_dhcprelay6_remoteid_format & DHCPRELAY6_REMOTE_ID_KEYWORD_VLAN) ?  
                            DHCP_TRUTH_VALUE_TRUE: DHCP_TRUTH_VALUE_FALSE); 
            break;

        default:
            break;
    }    
    return NULL;
}

static int
write_dhcpv6_relay_pd_snmp(int action, u_char * var_val, u_char var_val_type, 
        size_t var_val_len, u_char * statP, oid * name, size_t length,
        struct variable *v, u_int32_t vr_id)
{
    int intVal = 0;

    pal_mem_cpy(&intVal, var_val, var_val_len);
    
    switch (v->magic)
    {
        case DHCPV6RPDENABLE:
            if (DHCP_TRUTH_VALUE_TRUE == intVal)
            {
                if (DHCPRELAY6_CHK_FLAG(DHCPRELAY6_IA_PD_ADVERTISE_FLAG))
                {
                    return SNMP_ERR_NOERROR;
                }
                DHCPRELAY6_SET_FLAG(DHCPRELAY6_IA_PD_ADVERTISE_FLAG);
            }
            else if (DHCP_TRUTH_VALUE_FALSE == intVal)
            {
                if (!DHCPRELAY6_CHK_FLAG(DHCPRELAY6_IA_PD_ADVERTISE_FLAG))
                {
                    return SNMP_ERR_NOERROR;
                }
                DHCPRELAY6_CLR_FLAG(DHCPRELAY6_IA_PD_ADVERTISE_FLAG);
            }
            break;
            
        case DHCPV6RPDDISTANCE:
            g_dhcprelay6_pdroute_distance = intVal; 
            break;
            
        case DHCPV6RPDALLCLEAR:
            dhcprelay6_pdroute_clear(NULL, NULL, NULL);
            break;
            
        default:
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

static unsigned char *
dhcpv6_relay_pd_snmp(struct variable *v, oid *name,
        size_t *length, int exact, size_t *var_len, 
        WriteMethod **write_method, u_int32_t vr_id)
{
    if (!v || !name || !length || !var_len || !write_method)
    {
        return NULL;
    }

    if (MATCH_FAILED == snmp_header_generic(v, name, length, exact, var_len, write_method))
    {
        return NULL;
    }

    /* Return the current value of the variable */
    switch (v->magic)
    {  
        case DHCPV6RPDENABLE:
            DHCP_SNMP_RETURN_INTEGER(DHCPRELAY6_CHK_FLAG(DHCPRELAY6_IA_PD_ADVERTISE_FLAG) ?  
                            DHCP_TRUTH_VALUE_TRUE: DHCP_TRUTH_VALUE_FALSE); 
            break;
       
        case DHCPV6RPDDISTANCE:
            DHCP_SNMP_RETURN_INTEGER(g_dhcprelay6_pdroute_distance); 
            break;
            
        case DHCPV6RPDALLCLEAR:
            DHCP_SNMP_RETURN_INTEGER(DHCP_TRUTH_VALUE_FALSE); 
            break;
  
        default:
            break;
    }    
    return NULL;
}

S_Dhcpv6PDRouteItem *
dhcpv6_relay_snmp_pd_item_lookup(u_int32_t *index, int exact)
{
    S_Dhcpv6PDRouteItem *pItem = NULL;
    S_Dhcpv6PDRouteItem *ret = NULL;

    /* list be already sorted*/
    TAILQ_FOREACH(pItem, &g_dhcprelay6_pdroute.pdRouteHdr, entry)
    {
        if(!(pItem->addr->flags  & DHCPRELAY6_IA_PD_ROUTE_INSTALL_RESULT)
            || !(pItem->addr->flags  & DHCPRELAY6_IA_PD_ROUTE_INSTALL_SUCCESS))
        {
            continue;
        }
        
        if (exact)
        {
            if (*index == pItem ->id)
            {
                /* match item */
                return pItem;
            }
        }
        else
        {
            if (pItem ->id > *index)
            {
                if (!ret || ret->id > pItem ->id)
                {
                    ret = pItem;
                }
            }
        }
    }

    if (!exact && ret)
    {
        return ret;
    }    
    return NULL;
}

int
dhcpv6_relay_snmp_pd_table_rowstatus (u_int32_t *index, int status)
{
    int ret;
    S_Dhcpv6PDRouteItem * pItem = NULL;
    struct prefix_ipv6 p;
    
    pItem = dhcpv6_relay_snmp_pd_item_lookup(index, 1);
    if (!pItem)
    {
        return SNMP_ERR_NOERROR;        
    }
    
    switch (status)
    {
        case SNMP_ROW_ACTIVE:
            return SNMP_ERR_NOERROR;

        case SNMP_ROW_DESTROY:
            pal_mem_set(&p, 0, sizeof (p));
            p.prefixlen = pItem->addr->plen;
            pal_mem_cpy(&p.prefix, pItem->addr->address.iabuf, 16);
            ret = dhcprelay6_pdroute_clear(&p, &pItem->pdclient->client_addr, pItem->pdclient->ifname); 
            if (ret)
            {
                return SNMP_ERR_GENERR;
            }
            break;
            
        default:
            return SNMP_ERR_GENERR;
    }  

    return SNMP_ERR_NOERROR;
}

static int
write_dhcpv6_relay_pd_table_snmp(int action, u_char * var_val, u_char var_val_type, 
        size_t var_val_len, u_char * statP, oid * name, size_t length,
        struct variable *v, u_int32_t vr_id)
{
    int status = 0;
    int ret = 0;
    u_int32_t index = 0;
    
    ret = dhcrelay_snmp_index_get (v, name, &length, (int*)&index, 1, 1);
    
    switch (v->magic)
    {
        case DHCPV6RPDSTATUS:
            pal_mem_cpy(&status, var_val, sizeof(int));
            return dhcpv6_relay_snmp_pd_table_rowstatus(&index, status);
            
        default:
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

static unsigned char *
dhcpv6_relay_pd_table_snmp(struct variable *v, oid *name,
        size_t *length, int exact, size_t *var_len, 
        WriteMethod **write_method, u_int32_t vr_id)
{
    int ret = 0;
    S_Dhcpv6PDRouteItem * pItem = NULL;
    u_int32_t index;
    struct pal_timeval now;
    char time[20];
    
    if (!v || !name || !length || !var_len || !write_method)
    {
        return NULL;
    }
    
    ret = dhcrelay_snmp_index_get (v, name, length, (int*)&index, 1, exact);
    if (ret < 0) 
        return NULL;
    
    pItem = dhcpv6_relay_snmp_pd_item_lookup(&index, exact);
    if (!pItem)
    {
        return NULL;        
    }

    if(!exact)
    {
        index = pItem->id;
        dhcrelay_snmp_index_set (v, name, length, (int*)&index, 1);
    }

    /* Return the current value of the variable */
    switch (v->magic)
    {
        case DHCPV6RPDIFNAME:
            DHCP_SNMP_RETURN_STRING(pItem->pdclient->ifname);
            break;
            
        case DHCPV6RPDCLIENTIP:
            DHCP_SNMP_RETURN_STRING(piaddr(pItem->pdclient->client_addr));
            break;
            
        case DHCPV6RPDPREFIX:
            DHCP_SNMP_RETURN_STRING(piaddrcidr_s(&(pItem->addr->address), pItem->addr->plen));
            break;           

        case DHCPV6RPDEXPIRY:
            gettimeofday(&now, NULL);
            format_time_str(time, 20, 
                    "%Y-%m-%d %H:%M:%S", pItem->addr->starts + pItem->addr->max_life - cur_time + now.tv_sec );
            
            DHCP_SNMP_RETURN_STRING(time);
            break; 

        case DHCPV6RPDSTATUS:
            DHCP_SNMP_RETURN_INTEGER(DHCP_SNMP_ROW_STATUS_ACTIVE);
            break;
            
        default:
            break;
    }    
    return NULL;
}

static int
write_dhcpv6_relay_statistics_snmp(int action, u_char * var_val, u_char var_val_type, 
        size_t var_val_len, u_char * statP, oid * name, size_t length,
        struct variable *v, u_int32_t vr_id)
{
    int intVal = 0;
    
    pal_mem_cpy(&intVal, var_val, var_val_len);
    switch (v->magic)
    {
        case DHCPV6RRXSTATISTICSRESET:
            if (1 == intVal)
            {
                pal_mem_set(&g_dhcp_relay_pkt_stats, 0x00, sizeof(g_dhcp_relay_pkt_stats));
            }
            break;
            
        default:
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

static unsigned char *
dhcpv6_relay_statistics_snmp(struct variable *v, oid *name,
        size_t *length, int exact, size_t *var_len, 
        WriteMethod **write_method, u_int32_t vr_id)
{
    if (!v || !name || !length || !var_len || !write_method)
    {
        return NULL;
    }

    if (MATCH_FAILED == snmp_header_generic(v, name, length, exact, var_len, write_method))
    {
        return NULL;
    }

    switch(v->magic)
    {
        case DHCPV6RRXCLIENTPKTNUM:
            DHCP_SNMP_RETURN_INTEGER(g_dhcp_relay_pkt_stats.client6_relayed); 
            break;
        case DHCPV6RRXSERVERPKTNUM:
            DHCP_SNMP_RETURN_INTEGER( g_dhcp_relay_pkt_stats.server6_relayed); 
            break;
        case DHCPV6RRXCLIENTERRPKTNUM:
            DHCP_SNMP_RETURN_INTEGER(g_dhcp_relay_pkt_stats.client6_errors); 
            break;
        case DHCPV6RRXSERVERERRPKTNUM:
            DHCP_SNMP_RETURN_INTEGER(g_dhcp_relay_pkt_stats.client6_errors); 
            break;
        case DHCPV6RRXSTATISTICSRESET:
            DHCP_SNMP_RETURN_INTEGER(0); 
            break;
        default:
            pal_assert(0);
            break;
    }

    return NULL;
}

/* DHCPv6-MIB instances. */
oid dhcpv6_oid [] = { DHCPV6MIB};
oid dhcpv6r_global_oid [] = { DHCPV6RGLOBALMIB};
oid dhcpv6r_server_oid [] = { DHCPV6RSERVERMIB};
oid dhcpv6r_option_oid [] = { DHCPV6ROPTIONMIB};
oid dhcpv6r_stats_oid [] = { DHCPV6RSTATSMIB};

struct variable dhcpv6_variables[] = 
{
    /* centecDhcpv6Enable */
    {DHCPV6ENABLE, INTEGER, RWRITE, dhcpv6_snmp_enable,
     1, {1},dhcpv6_relay_snmp_value_check, snmp_api_non_table_index_check,
     write_dhcpv6_snmp_enable}
};

struct variable dhcpv6r_global_variables[] = 
{
      /* centecDhcpv6REnable. */
    {DHCPV6RENABLE, INTEGER, RWRITE, dhcpv6_relay_snmp,
     1, {1}, dhcpv6_relay_snmp_value_check, snmp_api_non_table_index_check,
     write_dhcpv6_relay_snmp}
};

struct variable dhcpv6r_server_variables[] = 
{
      /* centecDhcpv6RIpToGroupTable. */
    {DHCPV6RGROUPGROUPID,         INTEGER, NOACCESS, dhcpv6_relay_srvgrp_snmp,
     3, {1, 1, 1}, dhcpv6_relay_snmp_value_check, dhcpv6_relay_snmp_index_check},
    {DHCPV6RGROUPSERVERIPIF,          STRING, RWRITE, dhcpv6_relay_srvgrp_snmp,
     3, {1, 1, 2}, dhcpv6_relay_snmp_value_check, dhcpv6_relay_snmp_index_check,
     write_dhcpv6_relay_srvgrp_snmp},
    {DHCPV6RGROUPSTATUS,     INTEGER, RWRITE, dhcpv6_relay_srvgrp_snmp,
     3, {1, 1, 3}, dhcpv6_relay_snmp_value_check, dhcpv6_relay_snmp_index_check,
     write_dhcpv6_relay_srvgrp_snmp}
};

struct variable dhcpv6r_option_variables[] = 
{    
      /* centecDhcpv6RRemoteid. */
    {DHCPV6RREMOTEID, INTEGER, RWRITE, dhcpv6_relay_remoteid_snmp,
     1, {1}, dhcpv6_relay_snmp_value_check, snmp_api_non_table_index_check,
     write_dhcpv6_relay_remoteid_snmp},
    {DHCPV6RREMOTEIDDUID, INTEGER, RWRITE, dhcpv6_relay_remoteid_snmp,
     1, {2}, dhcpv6_relay_snmp_value_check, snmp_api_non_table_index_check,
     write_dhcpv6_relay_remoteid_snmp},
    {DHCPV6RREMOTEIDIFNAME, INTEGER, RWRITE, dhcpv6_relay_remoteid_snmp,
     1, {3}, dhcpv6_relay_snmp_value_check, snmp_api_non_table_index_check,
     write_dhcpv6_relay_remoteid_snmp},
    {DHCPV6RREMOTEIDVLAN, INTEGER, RWRITE, dhcpv6_relay_remoteid_snmp,
     1, {4}, dhcpv6_relay_snmp_value_check, snmp_api_non_table_index_check,
     write_dhcpv6_relay_remoteid_snmp},

         /* centecDhcpv6RPD. */
    {DHCPV6RPDENABLE, INTEGER, RWRITE, dhcpv6_relay_pd_snmp,
     1, {5}, dhcpv6_relay_snmp_value_check, snmp_api_non_table_index_check,
     write_dhcpv6_relay_pd_snmp},
     {DHCPV6RPDDISTANCE, INTEGER, RWRITE, dhcpv6_relay_pd_snmp,
     1, {6}, dhcpv6_relay_snmp_value_check, snmp_api_non_table_index_check,
     write_dhcpv6_relay_pd_snmp},
    {DHCPV6RPDALLCLEAR, INTEGER, RWRITE, dhcpv6_relay_pd_snmp,
     1, {7}, dhcpv6_relay_snmp_value_check, snmp_api_non_table_index_check,
     write_dhcpv6_relay_pd_snmp},

         /* centecDhcpv6RPDtable. */
    {DHCPV6RPDENTRYID, INTEGER, NOACCESS, dhcpv6_relay_pd_table_snmp,
     3, {8, 1, 1}, dhcpv6_relay_snmp_value_check, dhcpv6_relay_snmp_index_check},
    {DHCPV6RPDIFNAME, STRING, RONLY, dhcpv6_relay_pd_table_snmp,
     3, {8, 1, 2}, dhcpv6_relay_snmp_value_check, dhcpv6_relay_snmp_index_check},
    {DHCPV6RPDCLIENTIP, STRING, RONLY, dhcpv6_relay_pd_table_snmp,
     3, {8, 1, 3}, dhcpv6_relay_snmp_value_check, dhcpv6_relay_snmp_index_check},
    {DHCPV6RPDPREFIX, STRING, RONLY, dhcpv6_relay_pd_table_snmp,
     3, {8, 1, 4}, dhcpv6_relay_snmp_value_check, dhcpv6_relay_snmp_index_check},
    {DHCPV6RPDEXPIRY, STRING, RONLY, dhcpv6_relay_pd_table_snmp,
     3, {8, 1, 5}, dhcpv6_relay_snmp_value_check, dhcpv6_relay_snmp_index_check},
    {DHCPV6RPDSTATUS, INTEGER, RWRITE, dhcpv6_relay_pd_table_snmp,
     3, {8, 1, 6}, dhcpv6_relay_snmp_value_check, dhcpv6_relay_snmp_index_check,
     write_dhcpv6_relay_pd_table_snmp}
};

struct variable dhcpv6r_stats_variables[] = 
{
          /* centecDhcpv6RStatisticsGroup. */
    {DHCPV6RRXCLIENTPKTNUM,           INTEGER, RONLY, dhcpv6_relay_statistics_snmp,
     1, {1}, dhcpv6_relay_snmp_value_check, snmp_api_non_table_index_check},
    {DHCPV6RRXSERVERPKTNUM,            INTEGER, RONLY, dhcpv6_relay_statistics_snmp,
     1, {2}, dhcpv6_relay_snmp_value_check, snmp_api_non_table_index_check},
    {DHCPV6RRXCLIENTERRPKTNUM,        INTEGER, RONLY, dhcpv6_relay_statistics_snmp,
     1, {3}, dhcpv6_relay_snmp_value_check, snmp_api_non_table_index_check},
    {DHCPV6RRXSERVERERRPKTNUM,           INTEGER, RONLY, dhcpv6_relay_statistics_snmp,
     1, {4}, dhcpv6_relay_snmp_value_check, snmp_api_non_table_index_check},
    {DHCPV6RRXSTATISTICSRESET,           TRUTHVALUE, RWRITE, dhcpv6_relay_statistics_snmp,
     1, {64}, dhcpv6_relay_snmp_value_check, snmp_api_non_table_index_check,
     write_dhcpv6_relay_statistics_snmp}
};

/* Register DHCP-MIB. */
void
dhcprelay6_snmp_init (struct lib_globals *zg)
{
    REGISTER_MIB_MULTIPM(zg, "dhcpv6", dhcpv6_variables, variable, dhcpv6_oid, zg->protocol);
    REGISTER_MIB_MULTIPM(zg, "dhcpv6rglobal", dhcpv6r_global_variables, variable, dhcpv6r_global_oid, zg->protocol);
    REGISTER_MIB(zg, "dhcpv6relay/servergrp", dhcpv6r_server_variables, variable, dhcpv6r_server_oid);
    REGISTER_MIB(zg, "dhcpv6relay/option", dhcpv6r_option_variables, variable, dhcpv6r_option_oid);
    REGISTER_MIB(zg, "dhcpv6relay/statistics", dhcpv6r_stats_variables, variable, dhcpv6r_stats_oid);
    snmp_start (zg);
}
#endif
#endif
