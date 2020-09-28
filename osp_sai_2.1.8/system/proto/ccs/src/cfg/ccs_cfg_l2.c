
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_fdb.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_mcfdb.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_macfilter.h"
#include "gen/tbl_ipsg_fib_define.h"
#include "gen/tbl_ipsg_fib.h"
#include "gen/tbl_ipsg_s_mac_define.h"
#include "gen/tbl_ipsg_s_mac.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
#include "gen/tbl_lag_global_define.h"
#include "gen/tbl_lag_global.h"
#include "gen/tbl_mlag_define.h"
#include "gen/tbl_mlag.h"
#include "gen/tbl_mlag_peer_define.h"
#include "gen/tbl_mlag_peer.h"
#include "lacp_config.h"
#include "switch.h"
#include "gen/tbl_opm_global_define.h"
#include "gen/tbl_opm_global.h"
#include "gen/tbl_igsp_global_define.h"
#include "gen/tbl_igsp_global.h"
#include "gen/tbl_igsp_intf_define.h"
#include "gen/tbl_igsp_intf.h"
#include "gen/tbl_igsp_group_define.h"
#include "gen/tbl_igsp_group.h"
#include "gen/tbl_lldp_global_define.h"
#include "gen/tbl_lldp_global.h"
#include "igmp_define.h"
#include "lldp_define.h"
#include "gen/tbl_dot1x_global_define.h"
#include "gen/tbl_dot1x_global.h"
#include "gen/tbl_dot1x_radius_define.h"
#include "gen/tbl_dot1x_radius.h"
#include "gen/tbl_vlanclass_rule_define.h"
#include "gen/tbl_vlanclass_rule.h"
#include "gen/tbl_vlanclass_group_define.h"
#include "gen/tbl_vlanclass_group.h"
#include "vlanclass.h"
#include "gen/tbl_l2_protocol_define.h"
#include "gen/tbl_l2_protocol.h"
#ifdef CONFIG_MLAG
#include "mlag.h"
#endif /* !CONFIG_MLAG */
#ifdef CONFIG_ERPS
#include "gen/tbl_erps_domain_define.h"
#include "gen/tbl_erps_domain.h"
#include "gen/tbl_erps_ring_define.h"
#include "gen/tbl_erps_ring.h"
#include "erps_define.h"
#endif /* !CONFIG_ERPS */
#ifdef CONFIG_SFLOW
#include "gen/tbl_sflow_global_define.h"
#include "gen/tbl_sflow_global.h"
#include "gen/tbl_sflow_collector_define.h"
#include "gen/tbl_sflow_collector.h"
#endif
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "gen/tbl_vlan_mapping_table_define.h"
#include "gen/tbl_vlan_mapping_table.h"
#include "gen/tbl_vlan_mapping_entry_define.h"
#include "gen/tbl_vlan_mapping_entry.h"
#include "gen/tbl_egress_mapping_entry_define.h"
#include "gen/tbl_egress_mapping_entry.h"
#include "gen/tbl_vlan_group_define.h"
#include "gen/tbl_vlan_group.h"
#include "gen/tbl_evc_entry_define.h"
#include "gen/tbl_evc_entry.h"
#include "gen/tbl_mapped_vlan_info_define.h"
#include "gen/tbl_mapped_vlan_info.h"
#include "gen/tbl_g8032_ring_define.h"
#include "gen/tbl_g8032_ring.h"


/* For bug 38891, optimize show running performance */
typedef struct
{
    uint32            count;
    tbl_igsp_group_t *ptr_array[IGMP_LIMIT_DEF];
} ccs_cfg_igsp_group_vlan_sort_t;

typedef struct
{
    ccs_cfg_igsp_group_vlan_sort_t *vlan[GLB_MAX_VLAN_ID];
} ccs_cfg_igsp_group_sort_t;

extern int32
ns_route_is_inband(addr_ipv4_t ip, uint16 port, ns_route_proto_t protocol);

extern int32
ns_route_ipv6_is_inband(addr_ipv6_t ipv6_addr, uint16 port, ns_route_proto_t protocol);

extern int32
inband_snat_get_if_or_ip(const addr_ipv4_t *ip, uint16_t port, inband_snat_proto_t proto, char *buf);

static int32
_ccs_cfg_build_vlan_one(FILE* fp, tbl_vlan_t *p_db_vlan, uint32 *is_default)
{
    uint32 enter_vlan_mode = FALSE;
    char vlan_default_name[VLANNAME_SIZE];
    
    /* build default */
    if (GLB_DEF_VLAN_ID == p_db_vlan->key.vid)
    {
        sal_snprintf(vlan_default_name, VLANNAME_SIZE, "default");
    }
    else
    {
        sal_snprintf(vlan_default_name, VLANNAME_SIZE, "VLAN%04u", p_db_vlan->key.vid);
    }

    enter_vlan_mode = FALSE;
    *is_default = TRUE;
    
    if (sal_strcmp(vlan_default_name, p_db_vlan->name))
    {
        if (!enter_vlan_mode)
        {
            sal_fprintf(fp, "vlan %u\n", p_db_vlan->key.vid);
            enter_vlan_mode = TRUE;
        }
        sal_fprintf(fp, INTEND_1_STR"name %s\n", p_db_vlan->name);
        *is_default = FALSE;
    }

    if (TRUE != p_db_vlan->enable)
    {
        if (!enter_vlan_mode)
        {
            sal_fprintf(fp, "vlan %u\n", p_db_vlan->key.vid);
            enter_vlan_mode = TRUE;
        }
        sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" enable\n");
        *is_default = FALSE;
    }

    if (TRUE != p_db_vlan->mac_learning_en)
    {
        if (!enter_vlan_mode)
        {
            sal_fprintf(fp, "vlan %u\n", p_db_vlan->key.vid);
            enter_vlan_mode = TRUE;
        }
        sal_fprintf(fp, INTEND_1_STR"mac learning disable\n");
        *is_default = FALSE;
    }

    if (GLB_VLAN_SEC_SPEC_MAX_MAC_COUNT != p_db_vlan->max_mac_count)
    {
        if (!enter_vlan_mode)
        {
            sal_fprintf(fp, "vlan %u\n", p_db_vlan->key.vid);
            enter_vlan_mode = TRUE;
        }
        sal_fprintf(fp, INTEND_1_STR"mac-limit maximum %u\n", p_db_vlan->max_mac_count);
        *is_default = FALSE;
    }

    if (GLB_VLAN_SECURITY_ACTION_FORWARD != p_db_vlan->security_action)
    {
        if (!enter_vlan_mode)
        {
            sal_fprintf(fp, "vlan %u\n", p_db_vlan->key.vid);
            enter_vlan_mode = TRUE;
        }
        sal_fprintf(fp, INTEND_1_STR"mac-limit action %s\n", 
            cdb_enum_val2str(glb_vlan_security_action_strs, GLB_VLAN_SECURITY_ACTION_MAX, p_db_vlan->security_action));                     
        *is_default = FALSE;
    }

    if (TRUE == p_db_vlan->stats_en)
    {
        if (!enter_vlan_mode)
        {
            sal_fprintf(fp, "vlan %u\n", p_db_vlan->key.vid);
            enter_vlan_mode = TRUE;
        }
        sal_fprintf(fp, INTEND_1_STR"statistics enable\n");
        *is_default = FALSE;
    }

    if (enter_vlan_mode)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_vlan(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_vlan_master_t *p_master = tbl_vlan_get_master();
    tbl_vlan_t *p_db_vlan = NULL;
    uint32 vid = 0;
    uint32 bmp_count = 0;
    uint32 is_default = FALSE;
    vlan_bmp_t bmp;
    char buf[MAX_CMD_STR_LEN];
    
    GLB_BMP_INIT(bmp);
    
    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_vlan = p_master->vlan_array[vid];
        if (NULL == p_db_vlan)
        {
            continue;
        }
        _ccs_cfg_build_vlan_one(fp, p_db_vlan, &is_default);
        if (is_default)
        {
            /* default vlan1 should in running-config */
            if (GLB_DEF_VLAN_ID != vid)
            {
                GLB_BMP_SET(bmp, vid);
                bmp_count++;
            }
        }
    }

    if (bmp_count > 1)
    {
        sal_fprintf(fp, "vlan range %s\n", cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, bmp, GLB_VLAN_BMP_WORD_MAX));
        sal_fprintf(fp, "!\n");
    }
    else if (bmp_count == 1)
    {
        sal_fprintf(fp, "vlan %s\n", cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, bmp, GLB_VLAN_BMP_WORD_MAX));
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

#ifdef CONFIG_ERPS
#define M_STR_BUFSIZE  1600

void 
erps_get_instancestr(tbl_erps_domain_t *p_db_domain, char *str)
{
    uint32           index = 0;
    uint32           start = 0xffff;
    uint32           end = 0;
    char             *p = NULL;

    for (index = 0; index < p_db_domain->instance_id_num; index++)
    {
        if (0xffff == start)
        { 
            start = p_db_domain->instance_id[index];
            end = p_db_domain->instance_id[index];
            continue;
        }
        else
        {
            if ((end + 1) == p_db_domain->instance_id[index])
            {
                end = p_db_domain->instance_id[index];
                continue;
            }
        }

        p = str;
        p = p + sal_strlen(str);

        if (start < end)
        {
            sal_snprintf(p, M_STR_BUFSIZE, "%d-%d,", start, end);
        }
        else
        {
            sal_snprintf(p, M_STR_BUFSIZE, "%d,", start);
        }
        start = p_db_domain->instance_id[index];
        end = p_db_domain->instance_id[index];
    }

    p = str;
    p = p + sal_strlen(str);
    
    if (start < end)
    {
        sal_snprintf(p, M_STR_BUFSIZE, "%d-%d", start, end);
    }
    else if (start == end)
    {
        sal_snprintf(p, M_STR_BUFSIZE, "%d", start);
    }

    return;
}

void 
erps_instance_str_print(FILE* fp, char *str, int max)
{
    #define M_STR_LINE_MAX  80
    char instance_str[M_STR_LINE_MAX] = {'\0'};
    char *start = 0;
    char *end = 0;
    int count = 0;

    if (NULL == str || max < 1 || max > M_STR_LINE_MAX)
    {
        return;
    }
    /* parse the sting and write vlan item one by one */
    if (sal_strlen(str))
    {
        start = str;
        end = str;
        while ((NULL != end) && (count < ERPS_MAX_INSTANCE_NO))
        {
            /*void dead circle*/
            count++;
            end = sal_strstr(end, ",");
            if (NULL == end)
            {
                /* at the end of the string */
                break;
            }
            end++;
            if (end - start > max)
            {
                sal_snprintf(instance_str, end - start, start);
                
                sal_fprintf(fp, INTEND_1_STR"mstp instance %s\n", instance_str);
                
                sal_memset(instance_str, 0, sizeof(instance_str));
                start = end;
            }
        }
        if ((start != end) && (*start != '\0'))
        {
            sal_snprintf(instance_str, (sal_strlen(start) + 1), start);
            sal_fprintf(fp, INTEND_1_STR"mstp instance %s\n", instance_str);
        }    
    }

    return;
}

static int32
_ccs_cfg_build_erps_domain_one(FILE* fp, tbl_erps_domain_t *p_db_domain)
{
    tbl_erps_ring_t *          p_erps_ring = NULL;
    ctclib_slistnode_t *       listnode2 = NULL;  
    tbl_interface_t*           p_if = NULL;
    char                       ifname_ext[IFNAME_EXT_SIZE];
    char                       str[M_STR_BUFSIZE] = {'\0'};
 
    sal_fprintf(fp, "erps %u %s\n", p_db_domain->key.domain_id, p_db_domain->name);

    if (0 != p_db_domain->instance_id_num)
    {
        erps_get_instancestr(p_db_domain,  str);
        //sal_fprintf(fp, INTEND_1_STR"mstp instance %s\n", str);
        erps_instance_str_print(fp, str, 53);
    }

    if (0 != p_db_domain->primary_control_vlan_id)
    {
        sal_fprintf(fp, INTEND_1_STR"primary control vlan %u\n", p_db_domain->primary_control_vlan_id);
    }

    if (0 != p_db_domain->sub_control_vlan_id)
    {
        sal_fprintf(fp, INTEND_1_STR"sub control vlan %u\n", p_db_domain->sub_control_vlan_id);
    }

    /* modified by wangjj for erps bug 41013, 2016-11-24 */
    if (GLB_ERPS_DOMAIN_FAILURE_TIME_DEFAULT != p_db_domain->failure_timer_interval)
    {
        sal_fprintf(fp, INTEND_1_STR"failtime %u\n", p_db_domain->failure_timer_interval);
    }

    if (GLB_ERPS_DOMAIN_HELLO_TIME_DEFAULT != p_db_domain->hello_timer_interval)
    {
        sal_fprintf(fp, INTEND_1_STR"hellotime %u\n", p_db_domain->hello_timer_interval);
    }

    sal_fprintf(fp, "!\n");

    CTCLIB_SLIST_LOOP(p_db_domain->erps_ring_list.obj_list, p_erps_ring, listnode2)
    {
        sal_fprintf(fp, "erps ring %u %u", p_erps_ring->key.domain_id, p_erps_ring->key.ring_id);

        if (ERPS_RING_PRIMARY == p_erps_ring->erps_ring_level)
        {
            sal_fprintf(fp, " level primary\n");
        }
        else
        {
            sal_fprintf(fp, " level sub\n");    
        }
        
        if (ERPS_NODE_EDGE_UNKNOWN == p_erps_ring->edge_node_role)     
        {
            if (ERPS_NODE_MASTER == p_erps_ring->node_role)
            {
                sal_fprintf(fp, INTEND_1_STR"mode master\n");
            }
            else if (ERPS_NODE_TRANSIT == p_erps_ring->node_role)
            {
                sal_fprintf(fp, INTEND_1_STR"mode transit\n");
            }

            if (p_erps_ring->primary_port_ifindex)
            {
                p_if = tbl_interface_get_interface_by_ifindex(p_erps_ring->primary_port_ifindex);

                IFNAME_ETH2FULL(p_if->key.name, ifname_ext);
                sal_fprintf(fp, INTEND_1_STR"primary interface %s\n", ifname_ext);
            }
    
            if (p_erps_ring->secondary_port_ifindex)
            {
                p_if = tbl_interface_get_interface_by_ifindex(p_erps_ring->secondary_port_ifindex);
                IFNAME_ETH2FULL(p_if->key.name, ifname_ext);
                sal_fprintf(fp, INTEND_1_STR"secondary interface %s\n", ifname_ext);
            }
        }
        else
        {
            if (ERPS_NODE_EDGE == p_erps_ring->edge_node_role)
            {
                sal_fprintf(fp, INTEND_1_STR"edge-mode edge\n");
                
                if (!p_erps_ring->erps_srpt_enable)
                {
                    sal_fprintf(fp, INTEND_1_STR"srpt disable\n");
                }
            }
            else if (ERPS_NODE_ASSI_EDGE == p_erps_ring->edge_node_role)
            {
                sal_fprintf(fp, INTEND_1_STR"edge-mode assistant-edge\n");
            }

            if (p_erps_ring->primary_port_ifindex)
            {
                p_if = tbl_interface_get_interface_by_ifindex(p_erps_ring->primary_port_ifindex);
                IFNAME_ETH2FULL(p_if->key.name, ifname_ext);
                sal_fprintf(fp, INTEND_1_STR"edge interface %s\n", ifname_ext);
            }
    
            if (p_erps_ring->secondary_port_ifindex)
            {
                p_if = tbl_interface_get_interface_by_ifindex(p_erps_ring->secondary_port_ifindex);
                IFNAME_ETH2FULL(p_if->key.name, ifname_ext);
                sal_fprintf(fp, INTEND_1_STR"common interface %s\n", ifname_ext);
            }
        }

        if (ERPS_IDLE != p_erps_ring->erps_ring_state)
        {
            sal_fprintf(fp, INTEND_1_STR"enable\n");
        }

        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}
#endif /* !CONFIG_ERPS */

int32
ccs_cfg_build_erps_domain(FILE* fp, cfg_cmd_para_t *para)
{
#ifdef CONFIG_ERPS
    tbl_erps_domain_master_t *p_master = NULL;
    tbl_erps_domain_t     *p_db_domain = NULL;
    uint16 domain_id = 0;

    p_master = tbl_erps_domain_get_master();

    for (domain_id = 1; domain_id < GLB_MAX_ERPS_DOMAIN; domain_id++)
    {
        p_db_domain = p_master->domain_array[domain_id];
        if (NULL == p_db_domain)
        {
            continue;
        }
        _ccs_cfg_build_erps_domain_one(fp, p_db_domain);
    }
    
#endif /* !CONFIG_ERPS */
    return PM_E_NONE;
}

int32
ccs_cfg_build_opm_global(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_opm_global_t *p_db_glb = NULL;
    
    /* check exist */
    p_db_glb = tbl_opm_global_get_opm_global();
    if ((p_db_glb) && GLB_FLAG_ISSET(p_db_glb->erps, OPMGLB_FLAG_ERPS_MODE_HUAWEI))
    {
        sal_fprintf(fp, "erps mode rrpp\n");
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_lldp_global(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_lldp_global_t *p_db_glb = NULL;
    int32_t           has_config = FALSE;
    char buf[256];
    
    /* check exist */
    p_db_glb = tbl_lldp_global_get_lldp_global();
    if (p_db_glb == NULL)
    {
        return PM_E_NONE;
    }

    if (p_db_glb->enable)
    {
        sal_fprintf(fp, "lldp enable\n");
         has_config = TRUE;
    }
    if (GLB_IS_BIT_SET(p_db_glb->config_flag, LLDP_CONF_MGMT_IP_ADDRESS))
    {
         sal_fprintf(fp, "lldp management ip %s\n", cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_glb->management_ip.u.prefix4));
         has_config = TRUE;
    }
    if (sal_strlen(p_db_glb->system_desc))
    {
        sal_fprintf(fp, "lldp system-description %s\n", p_db_glb->system_desc);
         has_config = TRUE;
    }
    if (sal_strlen(p_db_glb->system_name))
    {
        sal_fprintf(fp, "lldp system-name %s\n", p_db_glb->system_name);
         has_config = TRUE;
    }
    if (p_db_glb->tx_hold != LLDP_TX_HOLD_DEFAULT)
    {
        sal_fprintf(fp, "lldp timer tx-hold %u\n", p_db_glb->tx_hold);
         has_config = TRUE;
    }
    if (p_db_glb->reinit_delay != LLDP_REINIT_DELAY_DEFAULT)
    {
         sal_fprintf(fp, "lldp timer reinit-delay %u\n", p_db_glb->reinit_delay);
         has_config = TRUE;
    }
    if (p_db_glb->tx_interval != LLDP_TX_INTERVAL_DEFAULT)
    {
         sal_fprintf(fp, "lldp timer tx-interval %u\n", p_db_glb->tx_interval);
         has_config = TRUE;
    }
    if (p_db_glb->tx_delay != LLDP_TX_DELAY_DEFAULT)
    {
         sal_fprintf(fp, "lldp timer tx-delay %u\n", p_db_glb->tx_delay);
         has_config = TRUE;
    }

    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}
static int32
_ccs_cfg_build_fdb_one(tbl_fdb_t *p_db_fdb, tbl_iter_args_t *pargs)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    char ifname[IFNAME_SIZE];
    FILE *fp = NULL;
    uint32 *has_config = NULL;
    const uint8 *mac = p_db_fdb->key.mac;
    int32 rc = PM_E_NONE;

    fp = (FILE*)(pargs->argv[0]);
    has_config = (uint32 *)(pargs->argv[1]);
    *has_config = TRUE;

    ifname[0] = CMD_ZERO_CHAR;
    ifname_ext[0] = CMD_ZERO_CHAR;
    
    rc = tbl_interface_get_name_by_ifindex(p_db_fdb->ifindex, ifname, IFNAME_SIZE);
    if (rc < 0)
    {
        return PM_E_NONE;
    }
    IFNAME_ETH2FULL(ifname, ifname_ext);
    if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC) && !GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
    {
         sal_fprintf(fp, "mac-address-table %.04hX.%.04hX.%.04hX forward %s vlan %u\n", 
            sal_ntoh16(((uint16*)mac)[0]),
            sal_ntoh16(((uint16*)mac)[1]), 
            sal_ntoh16(((uint16*)mac)[2]),
            ifname_ext, 
            p_db_fdb->key.fid);
    }   

    return PM_E_NONE;
}

int32
ccs_cfg_build_fdb(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;
    uint32 has_config = FALSE;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = fp;
    args.argv[1] = &has_config;
 
    tbl_fdb_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_fdb_one, &args);

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_dot1x_global(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_dot1x_global_t *p_db_glb = NULL;
    
    /* check exist */
    p_db_glb = tbl_dot1x_global_get_dot1x_global();

    if (!p_db_glb)
    {
        return PM_E_NONE;
    }
    
    if (p_db_glb->global_auth_enable)
    {
        sal_fprintf(fp, "dot1x system-auth-ctrl\n");
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_dot1x_radius(FILE* fp, cfg_cmd_para_t *para)
{
#if 0
    tbl_dot1x_radius_master_t *p_glb_radius = NULL;
    tbl_dot1x_radius_t *p_db_dot1x_radius = NULL;
    ctclib_slistnode_t *listnode = NULL;  
    
    /* check exist */
    p_glb_radius = tbl_dot1x_radius_get_master();

    if (!p_glb_radius)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_glb_radius->radius_list, p_db_dot1x_radius, listnode)
    {
        sal_fprintf(fp, "radius-server host %s", p_db_dot1x_radius->server_string);

        if (RADIUS_SERVER_PORT != p_db_dot1x_radius->key.sin_port)
        {
            sal_fprintf(fp, " auth-port %d", p_db_dot1x_radius->key.sin_port);
        }
        
        if (RADIUS_TIMEOUT_DEFAULT != p_db_dot1x_radius->timeout)
        {
            sal_fprintf(fp, " timeout %d", p_db_dot1x_radius->timeout);
        }
        
        if (RADIUS_RETRANSMIT_DEFAULT != p_db_dot1x_radius->max_retry)
        {
            sal_fprintf(fp, " retransmit %d", p_db_dot1x_radius->max_retry);
        }
        
        if (p_db_dot1x_radius->shared_secret_valid)
        {
            sal_fprintf(fp, " key %s", p_db_dot1x_radius->shared_secret);
        }

        sal_fprintf(fp, "\n"); 
        sal_fprintf(fp, "!\n");
    }
 #endif   
    return PM_E_NONE;
}


static int32
_ccs_cfg_build_ipsg_one(tbl_ipsg_fib_t *p_db_ipsg_fib, tbl_iter_args_t *pargs)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    FILE *fp = NULL;
    uint32 *has_config = NULL;
    const uint8 *mac = p_db_ipsg_fib->mac_addr;
    char    buf[CMD_BUF_256];

    fp = (FILE*)(pargs->argv[0]);
    has_config = (uint32 *)(pargs->argv[1]);
    *has_config = TRUE;

    sal_memset(ifname_ext, 0, sizeof(ifname_ext));
    sal_memset(buf, 0, sizeof(buf));
    IFNAME_ETH2FULL(p_db_ipsg_fib->ifname, ifname_ext);

    sal_fprintf(fp, "ip source binding mac %.04hx.%.04hx.%.04hx vlan %u ip %s interface %s\n",
        sal_ntoh16(((uint16*)mac)[0]),
        sal_ntoh16(((uint16*)mac)[1]), 
        sal_ntoh16(((uint16*)mac)[2]),
        p_db_ipsg_fib->key.fid,
        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_ipsg_fib->ip_addr.u.prefix4),
        ifname_ext);
    
    return PM_E_NONE;
}

static int32
_ccs_cfg_build_ipsg_mac_one(tbl_ipsg_s_mac_t *p_db_ipsg_mac, tbl_iter_args_t *pargs)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    FILE *fp = NULL;
    uint32 *has_config = NULL;
    const uint8 *mac = p_db_ipsg_mac->mac_addr;
    char    buf[CMD_BUF_256];

    fp = (FILE*)(pargs->argv[0]);
    has_config = (uint32 *)(pargs->argv[1]);
    *has_config = TRUE;

    sal_memset(ifname_ext, 0, sizeof(ifname_ext));
    sal_memset(buf, 0, sizeof(buf));
    IFNAME_ETH2FULL(p_db_ipsg_mac->ifname, ifname_ext);

    sal_fprintf(fp, "ip source binding mac %.04hx.%.04hx.%.04hx vlan %u ip %s interface %s\n",
        sal_ntoh16(((uint16*)mac)[0]),
        sal_ntoh16(((uint16*)mac)[1]), 
        sal_ntoh16(((uint16*)mac)[2]),
        p_db_ipsg_mac->vid,
        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_ipsg_mac->ip_addr.u.prefix4),
        ifname_ext);
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_ipsg(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;
    uint32 has_config = FALSE;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = fp;
    args.argv[1] = &has_config;
 
    tbl_ipsg_fib_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ipsg_one, &args);

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_ipsg_mac(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;
    uint32 has_config = FALSE;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = fp;
    args.argv[1] = &has_config;
 
    tbl_ipsg_s_mac_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_ipsg_mac_one, &args);

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

static int32
_ccs_cfg_build_mcfdb_one(tbl_mcfdb_t *p_db_mcfdb, tbl_iter_args_t *pargs)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    char ifname[IFNAME_SIZE];
    FILE *fp = NULL;
    uint32 *has_config = NULL;
    const uint8 *mac = p_db_mcfdb->key.mac;
    uint32 ifindex = 0;
    int32 rc = PM_E_NONE;

    fp = (FILE*)(pargs->argv[0]);
    has_config = (uint32 *)(pargs->argv[1]);

    if (!p_db_mcfdb->manual)
    {
        return PM_E_NONE;
    }
    
    *has_config = TRUE;

    GLB_BMP_ITER_BEGIN(p_db_mcfdb->l2mc_port, ifindex)
    {
        ifname[0] = CMD_ZERO_CHAR;
        ifname_ext[0] = CMD_ZERO_CHAR;
        rc = tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_SIZE);
        if (rc < 0)
        {
            continue;
        }
        IFNAME_ETH2FULL(ifname, ifname_ext);
        sal_fprintf(fp, "mac-address-table %.04hX.%.04hX.%.04hX forward %s vlan %u\n", 
            sal_ntoh16(((uint16*)mac)[0]),
            sal_ntoh16(((uint16*)mac)[1]), 
            sal_ntoh16(((uint16*)mac)[2]),
            ifname_ext, 
            p_db_mcfdb->key.fid);
    }
    GLB_BMP_ITER_END(p_db_mcfdb->l2mc_port, ifindex);
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_l2mc_fdb(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;
    uint32 has_config = FALSE;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = fp;
    args.argv[1] = &has_config;
 
    tbl_mcfdb_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_mcfdb_one, &args);

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_macfilter(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_macfilter_master_t *p_master = tbl_macfilter_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_macfilter_t *p_db_macflt = NULL;
    const uint8 *mac = NULL;
    uint32 has_config = FALSE;

    CTCLIB_SLIST_LOOP(p_master->macflt_list, p_db_macflt, listnode)
    {
        mac = p_db_macflt->key.mac;
        sal_fprintf(fp, "mac-address-table %.04hX.%.04hX.%.04hX discard\n",
            sal_ntoh16(((uint16*)mac)[0]),
            sal_ntoh16(((uint16*)mac)[1]), 
            sal_ntoh16(((uint16*)mac)[2]));
        has_config = TRUE;
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_brg_global(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_brg_global_t *p_db_glb = tbl_brg_global_get_brg_global();
    uint32 has_config = FALSE;

    if (GLB_FDB_DEFAULT_AGING_TIME != p_db_glb->fdb_aging_time)
    {
        sal_fprintf(fp, "mac-address-table ageing-time %u\n", p_db_glb->fdb_aging_time);
        has_config = TRUE;
    }
    if (p_db_glb->hw_learning_enable)
    {
        sal_fprintf(fp, "mac-address-table hardware-learning enable\n");
        has_config = TRUE;
    }
    if (GLB_PORT_ISOLATE_L2 != p_db_glb->port_isolate_mode)
    {
        sal_fprintf(fp, "port-isolate mode all\n");
        has_config = TRUE;
    }

    if (IPSOUR_DEFAULT_NUMBER != p_db_glb->ipsg_max_port_binding)
    {
        sal_fprintf(fp, "ip source maximal binding number per-port %u \n",
            p_db_glb->ipsg_max_port_binding);
        has_config = TRUE;
    }
    if (p_db_glb->trap_delay_up)
    {
        sal_fprintf(fp, "snmp-server trap delay linkup %u\n", p_db_glb->trap_delay_up);
        has_config = TRUE;
    }
    if (p_db_glb->trap_delay_down)
    {
        sal_fprintf(fp, "snmp-server trap delay linkdown %u\n", p_db_glb->trap_delay_down);
        has_config = TRUE;
    }
    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_lag_global(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_lag_global_t *p_db_glb = tbl_lag_global_get_lag_global();
    uint32 has_config = FALSE;
#ifdef TAPPRODUCT
    uint8 idx;
#else
    uint8 msa;
    uint8 mda;
    uint8 ipsa;
    uint8 ipda;
    uint8 pro;    
    uint8 psrc;  
    uint8 pdst;
#endif
    uint32 i = 1;
    uint32 chip_type = tbl_chip_get_type();
    
    if (LACP_DEFAULT_SYSTEM_PRIORITY != p_db_glb->system_priority)
    {
        sal_fprintf(fp, "lacp system-priority %u\n", p_db_glb->system_priority);
        has_config = TRUE;
    }
    if (GLB_LAG_HASH_ALGO_CRC == p_db_glb->hash_arithmetic)
    {
        sal_fprintf(fp, "port-channel load-balance hash-arithmetic %s\n", "crc");
        has_config = TRUE;
    }

    for (i = 1; i < GLB_LAG_GROUP_NUM+1; i++)
    {
 
            if (GLB_LAG_LOAD_BALANCE_RR == p_db_glb->load_balance_mode[i])
            {
                sal_fprintf(fp, "port-channel %d load-balance-mode round-robin\n", i);
                has_config = TRUE;
            }
            else if (GLB_LAG_LOAD_BALANCE_DLB == p_db_glb->load_balance_mode[i])
            {
                sal_fprintf(fp, "port-channel %d load-balance-mode dynamic\n", i);
                has_config = TRUE;
            }

     }

#ifdef TAPPRODUCT
    if(check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
    {
        if (p_db_glb->load_balance_inner_field_en != GLB_AGG_LOAD_BALANCE_INNER_FIELD_EN)
        {
            switch(p_db_glb->load_balance_inner_field_en)
            {
			case GLB_LAG_TUNNEL_HASH_MODE_OUTER:
                sal_fprintf(fp, "port-channel load-balance tunnel-hash-mode outer\n");
				break;
            case GLB_LAG_TUNNEL_HASH_MODE_BOTH:
                sal_fprintf(fp, "port-channel load-balance tunnel-hash-mode both\n");
				break;
			case GLB_LAG_TUNNEL_HASH_MODE_INNER:
                sal_fprintf(fp, "port-channel load-balance tunnel-hash-mode inner\n");
				break;
        	}
            has_config = TRUE;
        }
    }
    
    if (GLB_AGG_LOAD_BALANCE_CTC_DEFAULT != p_db_glb->load_balance)
    {
        if (GLB_CHIP_GREATEBELT == chip_type)
        {
            if (GLB_LAG_LOAD_BALANCE_DST_IP == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance dst-ip\n");
            }
            else if (GLB_LAG_LOAD_BALANCE_DST_MAC == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance dst-mac\n");
            }
            else if (GLB_LAG_LOAD_BALANCE_DST_PORT == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance dst-port\n");
            }
            else if (GLB_LAG_LOAD_BALANCE_SRC_IP == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance src-ip\n");
            }
            else if (GLB_LAG_LOAD_BALANCE_SRC_MAC == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance src-mac\n");
            }
            else if (GLB_LAG_LOAD_BALANCE_SRC_PORT == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance src-port\n");
            }
            else if ((GLB_LAG_LOAD_BALANCE_SRC_IP | GLB_LAG_LOAD_BALANCE_DST_IP) == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance src-dst-ip\n");
            }
            else if ((GLB_LAG_LOAD_BALANCE_SRC_MAC | GLB_LAG_LOAD_BALANCE_DST_MAC) == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance src-dst-mac\n");
            }
            else if ((GLB_LAG_LOAD_BALANCE_SRC_PORT | GLB_LAG_LOAD_BALANCE_DST_PORT) == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance src-dst-port\n");
            }
            else if ((GLB_LAG_LOAD_BALANCE_SRC_PORT | GLB_LAG_LOAD_BALANCE_DST_PORT | 
                      GLB_LAG_LOAD_BALANCE_SRC_IP | GLB_LAG_LOAD_BALANCE_DST_IP) == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance src-dst-ip-src-dst-port\n");
            }
        }
        else if (GLB_CHIP_GOLDENGATE == chip_type || GLB_CHIP_DUET2 == chip_type)
        {
            for(idx = 0; idx < GLB_AGG_LOAD_BALANCE_MAX; idx++)
            {
                if(BIT_IS_SET(p_db_glb->load_balance,idx)) 
                {
                    if(!BIT_IS_SET(GLB_AGG_LOAD_BALANCE_CTC_DEFAULT,idx))
                    {
                        sal_fprintf(fp, "port-channel load-balance set %s\n", 
                            cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, idx));
                    }
                }else if(BIT_IS_SET(GLB_AGG_LOAD_BALANCE_CTC_DEFAULT,idx)){
                    sal_fprintf(fp, CMD_NO_STR" port-channel load-balance set %s\n", 
                            cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, idx));
                }
            }
        }
        
        has_config = TRUE;
    }
#else
    if (GLB_AGG_LOAD_BALANCE_CTC_DEFAULT != p_db_glb->load_balance)
    {
        if (GLB_CHIP_GREATEBELT == chip_type)
        {
            if (GLB_LAG_LOAD_BALANCE_DST_IP == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance dst-ip\n");
            }
            else if (GLB_LAG_LOAD_BALANCE_DST_MAC == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance dst-mac\n");
            }
            else if (GLB_LAG_LOAD_BALANCE_DST_PORT == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance dst-port\n");
            }
            else if (GLB_LAG_LOAD_BALANCE_SRC_IP == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance src-ip\n");
            }
            else if (GLB_LAG_LOAD_BALANCE_SRC_MAC == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance src-mac\n");
            }
            else if (GLB_LAG_LOAD_BALANCE_SRC_PORT == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance src-port\n");
            }
            else if ((GLB_LAG_LOAD_BALANCE_SRC_IP | GLB_LAG_LOAD_BALANCE_DST_IP) == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance src-dst-ip\n");
            }
            else if ((GLB_LAG_LOAD_BALANCE_SRC_MAC | GLB_LAG_LOAD_BALANCE_DST_MAC) == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance src-dst-mac\n");
            }
            else if ((GLB_LAG_LOAD_BALANCE_SRC_PORT | GLB_LAG_LOAD_BALANCE_DST_PORT) == p_db_glb->load_balance)
            {
                sal_fprintf(fp, "port-channel load-balance src-dst-port\n");
            }
        }
        else if (GLB_CHIP_GOLDENGATE == chip_type || GLB_CHIP_DUET2 == chip_type)
        {
            if(!(p_db_glb->load_balance & GLB_LAG_LOAD_BALANCE_SRC_MAC))
            {
                msa = 0;
                sal_fprintf(fp, CMD_NO_STR" port-channel load-balance set %s\n", 
                    cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, msa));
            }

            if(!(p_db_glb->load_balance & GLB_LAG_LOAD_BALANCE_DST_MAC))
            {
                mda = 1;
                sal_fprintf(fp, CMD_NO_STR" port-channel load-balance set %s\n", 
                    cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, mda));
            }

            if(!(p_db_glb->load_balance & GLB_LAG_LOAD_BALANCE_SRC_IP))
            {
                ipsa = 2;
                sal_fprintf(fp, CMD_NO_STR" port-channel load-balance set %s\n", 
                    cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, ipsa));
            }

            if(!(p_db_glb->load_balance & GLB_LAG_LOAD_BALANCE_DST_IP))
            {
                ipda = 3;
                sal_fprintf(fp, CMD_NO_STR" port-channel load-balance set %s\n", 
                    cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, ipda));
            }

            if(p_db_glb->load_balance & GLB_LAG_LOAD_BALANCE_IP_PROTOCOL)
            {
                pro = 4;
                sal_fprintf(fp, "port-channel load-balance set %s\n", 
                    cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, pro));
            }

            if(p_db_glb->load_balance & GLB_LAG_LOAD_BALANCE_SRC_PORT)
            {
                psrc = 5;
                sal_fprintf(fp, "port-channel load-balance set %s\n", 
                    cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, psrc));
            }

            if(p_db_glb->load_balance & GLB_LAG_LOAD_BALANCE_DST_PORT)
            {
                pdst = 6;
                sal_fprintf(fp, "port-channel load-balance set %s\n", 
                    cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, pdst));
            }

            if(p_db_glb->load_balance & GLB_LAG_LOAD_BALANCE_INNER_SRC_MAC)
            {
                pdst = 8;
                sal_fprintf(fp, "port-channel load-balance set %s\n", 
                    cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, pdst));
            }

            if(p_db_glb->load_balance & GLB_LAG_LOAD_BALANCE_INNER_DST_MAC)
            {
                pdst = 9;
                sal_fprintf(fp, "port-channel load-balance set %s\n", 
                    cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, pdst));
            }

            if(p_db_glb->load_balance & GLB_LAG_LOAD_BALANCE_INNER_SRC_IP)
            {
                pdst = 10;
                sal_fprintf(fp, "port-channel load-balance set %s\n", 
                    cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, pdst));
            }

            if(p_db_glb->load_balance & GLB_LAG_LOAD_BALANCE_INNER_DST_IP)
            {
                pdst = 11;
                sal_fprintf(fp, "port-channel load-balance set %s\n", 
                    cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, pdst));
            }

            if(p_db_glb->load_balance & GLB_LAG_LOAD_BALANCE_INNER_IP_PROTOCOL)
            {
                pdst = 12;
                sal_fprintf(fp, "port-channel load-balance set %s\n", 
                    cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, pdst));
            }

            if(p_db_glb->load_balance & GLB_LAG_LOAD_BALANCE_INNER_SRC_PORT_L4)
            {
                pdst = 13;
                sal_fprintf(fp, "port-channel load-balance set %s\n", 
                    cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, pdst));
            }

            if(p_db_glb->load_balance & GLB_LAG_LOAD_BALANCE_INNER_DST_PORT_L4)
            {
                pdst = 14;
                sal_fprintf(fp, "port-channel load-balance set %s\n", 
                    cdb_enum_val2str(glb_agg_load_balance_strs, GLB_AGG_LOAD_BALANCE_MAX, pdst));
            }
        }
        
        has_config = TRUE;
    }
#endif

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_mlag(FILE* fp, cfg_cmd_para_t *para)
{
#ifdef CONFIG_MLAG
    char buf[CMD_BUF_64];
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    tbl_mlag_peer_t *p_db_peer = tbl_mlag_peer_get_mlag_peer();
    tbl_interface_t *p_db_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    uint32 has_config = FALSE;
    
    if (p_db_mlag->enable)
    {
        sal_fprintf(fp, "mlag configuration\n");
        has_config = TRUE;

        if (!p_db_mlag->reload_delay_auto)
        {
            sal_fprintf(fp, "  reload-delay %u\n", p_db_mlag->reload_delay_interval);
        }
        if ((MLAG_DEFAULT_KEEPALIVE != p_db_peer->keepalive) || (MLAG_DEFAULT_HOLDTIME != p_db_peer->holdtime))
        {
            sal_fprintf(fp, "  timer mlag %u %u\n", p_db_peer->keepalive, p_db_peer->holdtime);
        }

        if (p_db_mlag->peer_link_ifindex)
        {
            p_db_if = tbl_interface_get_interface_by_ifindex(p_db_mlag->peer_link_ifindex);
            if (p_db_if)
            {
                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
                sal_fprintf(fp, "  peer-link %s\n", ifname_ext);
            }
        }
        if (AF_INET == p_db_peer->peer_addr.family)
        {
            cdb_addr_val2str(buf, CMD_BUF_64, &p_db_peer->peer_addr);
            sal_fprintf(fp, "  peer-address %s\n", buf);
        }
    }
    
    if (has_config)
    {
        sal_fprintf(fp, "  exit\n");
        sal_fprintf(fp, "!\n");
    }
    
#endif /* !CONFIG_MLAG */
    return PM_E_NONE;
}

int32
ccs_cfg_build_igmp_snooping_vlan_static_group(FILE* fp, vid_t vid, ccs_cfg_igsp_group_sort_t *p_group_sort)
{
    tbl_igsp_group_t *p_db_igsp_grp = NULL;
    tbl_interface_t *p_db_if = NULL;
    char  group_addr_str[CMD_BUF_32];
    char  ifname_ext[IFNAME_EXT_SIZE];
    uint32 i = 0;

    if (NULL == p_group_sort->vlan[vid])
    {
        return PM_E_NONE;
    }
    
    for (i = 0; i < p_group_sort->vlan[vid]->count; i++)
    {
        p_db_igsp_grp = p_group_sort->vlan[vid]->ptr_array[i];
        p_db_if = tbl_interface_get_interface_by_ifindex(p_db_igsp_grp->key.ifindex);
        if (NULL == p_db_if)
        {
            continue;
        }
        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
        cdb_addr_ipv4_val2str(group_addr_str, CMD_BUF_32, &p_db_igsp_grp->key.ip);
        sal_fprintf(fp, "  static-group %s interface %s\n", group_addr_str, ifname_ext);
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_igmp_snooping_vlan_host_join_group(FILE* fp, tbl_igsp_intf_t *p_db_igsp_if)
{
    ds_join_group_t   *p_db_join = NULL;
    ctclib_slistnode_t *listnode = NULL;
    char  group_addr_str[CMD_BUF_32];

    if (p_db_igsp_if->mrouter_list.obj_list)
    {
        CTCLIB_SLIST_LOOP(p_db_igsp_if->join_list.obj_list, p_db_join, listnode)
        {
            cdb_addr_ipv4_val2str(group_addr_str, CMD_BUF_32, &p_db_join->key.group);
            sal_fprintf(fp, "  join-group %s\n", group_addr_str);
        }
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_igmp_snooping_intf(FILE* fp, tbl_igsp_intf_t *p_db_igsp_if, tbl_igsp_global_t *p_db_igsp_glb,
        ccs_cfg_igsp_group_sort_t *p_group_sort)
{
    ds_mrouter_port_t *p_db_mrouter_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    char                ifname_ext[IFNAME_EXT_SIZE];
    char                ip_addr[CMD_BUF_32];
    uint32 has_config = FALSE;

    if (p_db_igsp_if->enable != p_db_igsp_glb->enable)
    {
        has_config = TRUE;
    }
    if (sal_strlen(p_db_igsp_if->acl_name))
    {
        has_config = TRUE;
    }
    if (p_db_igsp_if->fast_leave != p_db_igsp_glb->fast_leave)
    {
        has_config = TRUE;
    }
    if (p_db_igsp_if->version != p_db_igsp_glb->version)
    {
        has_config = TRUE;
    }
    if (p_db_igsp_if->report_suppress != p_db_igsp_glb->report_suppress)
    {
        has_config = TRUE;
    }
    if (p_db_igsp_if->max_member_number != IGMP_LIMIT_DEF)
    {
        has_config = TRUE;
    }
    if (p_db_igsp_if->robustness_var != p_db_igsp_glb->robustness_var)
    {
        has_config = TRUE;
    }
    if (p_db_igsp_if->query_interval != p_db_igsp_glb->query_interval)
    {
        has_config = TRUE;
    }
    if (p_db_igsp_if->admin_other_query_interval)
    {
        has_config = TRUE;
    }
    if (p_db_igsp_if->querier_max_response_time != p_db_igsp_glb->querier_max_response_time)
    {
        has_config = TRUE;
    }
    if (p_db_igsp_if->querier_config_address.s_addr)
    {
        has_config = TRUE;
    }
    if (p_db_igsp_if->querier_enable)
    {
        has_config = TRUE;
    }
    if (p_db_igsp_if->lmqc != p_db_igsp_glb->lmqc)
    {
        has_config = TRUE;
    }
    if (p_db_igsp_if->lmqi != p_db_igsp_glb->lmqi)
    {
        has_config = TRUE;
    }
    if (p_db_igsp_if->discard_unknown != p_db_igsp_glb->discard_unknown)
    {
        has_config = TRUE;
    }
    if (IGMP_MROUTER_PORT_TIMEOUT_DEF != p_db_igsp_if->mrouter_aging_interval)
    {
        has_config = TRUE;
    }
    if (p_db_igsp_if->mrouter_list.obj_list)
    {
        CTCLIB_SLIST_LOOP(p_db_igsp_if->mrouter_list.obj_list, p_db_mrouter_port, listnode)
        {
            if (IGMP_MROUTE_STATIC == p_db_mrouter_port->type)
            {
                has_config = TRUE;
                break;
            }
        }
    }
    if (p_db_igsp_if->join_list.obj_list)
    {
        CTCLIB_SLIST_LOOP(p_db_igsp_if->join_list.obj_list, p_db_mrouter_port, listnode)
        {
            has_config = TRUE;
            break;
        }
    }
    
    if (!has_config)
    {
        if (p_group_sort->vlan[p_db_igsp_if->key.vid])
        {
            has_config = TRUE;
        }
    }
    
    if (has_config)
    {
        sal_fprintf(fp, "ip igmp snooping vlan %u\n", p_db_igsp_if->key.vid);
    }
    
    if (p_db_igsp_if->enable != p_db_igsp_glb->enable)
    {
        if (p_db_igsp_if->enable)
        {
            sal_fprintf(fp, "  enable\n");
        }
        else
        {
            sal_fprintf(fp, "  "CMD_NO_STR" enable\n");
        }
    }
    if (sal_strlen(p_db_igsp_if->acl_name))
    {
        sal_fprintf(fp, "  access-group %s\n", p_db_igsp_if->acl_name);
    }
    if (p_db_igsp_if->fast_leave != p_db_igsp_glb->fast_leave)
    {
        if (p_db_igsp_if->fast_leave)
        {
            sal_fprintf(fp, "  fast-leave\n");
        }
        else
        {
            sal_fprintf(fp, "  "CMD_NO_STR" fast-leave\n");
        }
    }
    if (p_db_igsp_if->version != p_db_igsp_glb->version)
    {
        sal_fprintf(fp, "  version %u\n", p_db_igsp_if->version);
    }
    if (p_db_igsp_if->report_suppress != p_db_igsp_glb->report_suppress)
    {
        if (p_db_igsp_if->report_suppress)
        {
            sal_fprintf(fp, "  report-suppression\n");
        }
        else
        {
            sal_fprintf(fp, "  "CMD_NO_STR" report-suppression\n");
        }
    }    
    if (p_db_igsp_if->max_member_number != IGMP_LIMIT_DEF)
    {
        sal_fprintf(fp, "  max-member-num %u\n", p_db_igsp_if->max_member_number);
    }
    if (p_db_igsp_if->robustness_var != p_db_igsp_glb->robustness_var)
    {
        sal_fprintf(fp, "  robustness-variable %u\n", p_db_igsp_if->robustness_var);
    }
    if (p_db_igsp_if->querier_max_response_time != p_db_igsp_glb->querier_max_response_time)
    {
        sal_fprintf(fp, "  querier max-response-time %u\n", p_db_igsp_if->querier_max_response_time);
    }
    if (p_db_igsp_if->query_interval != p_db_igsp_glb->query_interval)
    {
        sal_fprintf(fp, "  querier query-interval %u\n", p_db_igsp_if->query_interval);
    }
    if (p_db_igsp_if->admin_other_query_interval)
    {
        sal_fprintf(fp, "  querier timeout %u\n", p_db_igsp_if->admin_other_query_interval);
    }
    if (p_db_igsp_if->querier_config_address.s_addr)
    {
        sal_fprintf(fp, "  querier address %s\n", cdb_addr_ipv4_val2str(ip_addr, CMD_BUF_32, &p_db_igsp_if->querier_config_address));
    }
    if (p_db_igsp_if->querier_enable)
    {
        sal_fprintf(fp, "  querier\n");
    }
    if (p_db_igsp_if->lmqc != p_db_igsp_glb->lmqc)
    {
        sal_fprintf(fp, "  last-member-query-count %u\n", p_db_igsp_if->lmqc);
    }
    if (p_db_igsp_if->lmqi != p_db_igsp_glb->lmqi)
    {
        sal_fprintf(fp, "  last-member-query-interval %u\n", p_db_igsp_if->lmqi);
    }
    if (p_db_igsp_if->discard_unknown != p_db_igsp_glb->discard_unknown)
    {
        if (p_db_igsp_if->discard_unknown)
        {
            sal_fprintf(fp, "  discard-unknown\n");
        }
        else
        {
            sal_fprintf(fp, "  "CMD_NO_STR" discard-unknown\n");
        }
    }
    if (IGMP_MROUTER_PORT_TIMEOUT_DEF != p_db_igsp_if->mrouter_aging_interval)
    {
        sal_fprintf(fp, "  mrouter-aging-interval %u\n", p_db_igsp_if->mrouter_aging_interval);
    }
    if (p_db_igsp_if->mrouter_list.obj_list)
    {
        CTCLIB_SLIST_LOOP(p_db_igsp_if->mrouter_list.obj_list, p_db_mrouter_port, listnode)
        {
            if (IGMP_MROUTE_STATIC == p_db_mrouter_port->type)
            {
                IFNAME_ETH2FULL(p_db_mrouter_port->key.name, ifname_ext);
                sal_fprintf(fp, "  mrouter interface %s\n", ifname_ext);
            }
        }
    }

    ccs_cfg_build_igmp_snooping_vlan_static_group(fp, p_db_igsp_if->key.vid, p_group_sort);

    ccs_cfg_build_igmp_snooping_vlan_host_join_group(fp, p_db_igsp_if);

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

static int32
_ccs_cfg_build_igmp_snooping_group_sort_one(tbl_igsp_group_t *p_group, tbl_iter_args_t *pargs)
{
    ccs_cfg_igsp_group_sort_t *p_group_sort = (ccs_cfg_igsp_group_sort_t*)pargs->argv[0];
    uint32 count = 0;

    if (IGMP_TYPE_DYNAMIC == p_group->type)
    {
        return PM_E_NONE;
    }
    if (p_group->key.vid >= GLB_MAX_VLAN_ID)
    {
        return PM_E_NONE;
    }

    if (NULL == p_group_sort->vlan[p_group->key.vid])
    {
        p_group_sort->vlan[p_group->key.vid] = XCALLOC(MEM_PM_TEMP, sizeof(ccs_cfg_igsp_group_vlan_sort_t));
    }

    if (p_group_sort->vlan[p_group->key.vid])
    {
        count = p_group_sort->vlan[p_group->key.vid]->count;
        p_group_sort->vlan[p_group->key.vid]->ptr_array[count] = p_group;
        p_group_sort->vlan[p_group->key.vid]->count = count+1;
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_igmp_snooping_group_sort(ccs_cfg_igsp_group_sort_t *p_group_sort)
{
    uint32 has_group_config = FALSE;
    tbl_iter_args_t args;

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_group_sort;
    
    tbl_igsp_group_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_igmp_snooping_group_sort_one, &args);
    return has_group_config;
}

int32
ccs_cfg_build_igmp_snooping_group_sort_free(ccs_cfg_igsp_group_sort_t *p_group_sort)
{
    uint32 i = 0;

    for (i = 0; i < GLB_MAX_VLAN_ID; i++)
    {
        if (p_group_sort->vlan[i])
        {
            XFREE(MEM_PM_TEMP, p_group_sort->vlan[i]);
            p_group_sort->vlan[i] = NULL;
        }

    }
    return PM_E_NONE;
}
  
int32
ccs_cfg_build_igmp_snooping(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_igsp_global_t *p_db_igsp_glb = tbl_igsp_global_get_igsp_global();
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    uint32 has_config = FALSE;
    uint32 vid = 0;
    ccs_cfg_igsp_group_sort_t group_sort;
    char buf[CMD_BUF_32];

    if (!p_db_igsp_glb->enable)
    {
        sal_fprintf(fp, CMD_NO_STR" ip igmp snooping\n");
        has_config = TRUE;
    }
    if (!p_db_igsp_glb->report_suppress)
    {
        sal_fprintf(fp, CMD_NO_STR" ip igmp snooping report-suppression\n");
        has_config = TRUE;
    }

    if (IGMP_VERSION_DEF != p_db_igsp_glb->version)
    {
        sal_fprintf(fp, "ip igmp snooping version %u\n", p_db_igsp_glb->version);
        has_config = TRUE;
    }
    
    if (p_db_igsp_glb->fast_leave)
    {
        sal_fprintf(fp, "ip igmp snooping fast-leave\n");
        has_config = TRUE;
    }
    if (IGMP_LIMIT_DEF != p_db_igsp_glb->max_member_number)
    {
        sal_fprintf(fp, "ip igmp snooping max-member-num %u\n", p_db_igsp_glb->max_member_number);
        has_config = TRUE;
    }
    if (IGMP_ROBUSTNESS_VAR_DEF != p_db_igsp_glb->robustness_var)
    {
        sal_fprintf(fp, "ip igmp snooping robustness-variable %u\n", p_db_igsp_glb->robustness_var);
        has_config = TRUE;
    }
    if (IGMP_QUERY_RESPONSE_INTERVAL_DEF != p_db_igsp_glb->querier_max_response_time)
    {
        sal_fprintf(fp, "ip igmp snooping querier max-response-time %u\n", p_db_igsp_glb->querier_max_response_time);
        has_config = TRUE;
    }
    if (IGMP_QUERY_INTERVAL_DEF != p_db_igsp_glb->query_interval)
    {
        sal_fprintf(fp, "ip igmp snooping querier query-interval %u\n", p_db_igsp_glb->query_interval);
        has_config = TRUE;
    }
    if (IGMP_LAST_MEMBER_QUERY_COUNT_DEF != p_db_igsp_glb->lmqc)
    {
        sal_fprintf(fp, "ip igmp snooping last-member-query-count %u\n", p_db_igsp_glb->lmqc);
        has_config = TRUE;
    }
    if (IGMP_LAST_MEMBER_QUERY_INTERVAL_DEF != p_db_igsp_glb->lmqi)
    {
        sal_fprintf(fp, "ip igmp snooping last-member-query-interval %u\n", p_db_igsp_glb->lmqi);
        has_config = TRUE;
    }
    if (sal_hton32(INADDR_QUERY_ADDR) != p_db_igsp_glb->global_src.s_addr)
    {
        sal_fprintf(fp, "ip igmp snooping global source-address %s\n", cdb_addr_ipv4_val2str(buf, CMD_BUF_32, &p_db_igsp_glb->global_src));
        has_config = TRUE;
    }
    if (p_db_igsp_glb->discard_unknown)
    {
        sal_fprintf(fp, "ip igmp snooping discard-unknown\n");
        has_config = TRUE;
    }
    
    if (p_db_igsp_glb->tcn_enable)
    {
        sal_fprintf(fp, "ip igmp snooping querier tcn enable\n");
        has_config = TRUE;
    }
    if (IGMP_SNOOPING_TCN_QUERY_CNT != p_db_igsp_glb->tcn_query_count)
    {
        sal_fprintf(fp, "ip igmp snooping querier tcn query-count %u\n", p_db_igsp_glb->tcn_query_count);
        has_config = TRUE;
    }
    if (IGMP_SNOOPING_TCN_QUERY_MAX_RES_TIME != p_db_igsp_glb->tcn_query_max_response_time)
    {
        sal_fprintf(fp, "ip igmp snooping querier tcn query-max-response-time %u\n", p_db_igsp_glb->tcn_query_max_response_time);
        has_config = TRUE;
    }
    if (IGMP_SNOOPING_TCN_QUERY_INTERVAL != p_db_igsp_glb->tcn_query_interval)
    {
        sal_fprintf(fp, "ip igmp snooping querier tcn query-interval %u\n", p_db_igsp_glb->tcn_query_interval);
        has_config = TRUE;
    }
    if (p_db_igsp_glb->host_join_enable)
    {
        sal_fprintf(fp, "ip igmp snooping host-join\n");
        has_config = TRUE;
    }

    sal_memset(&group_sort, 0, sizeof(group_sort));
    ccs_cfg_build_igmp_snooping_group_sort(&group_sort);

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        ccs_cfg_build_igmp_snooping_intf(fp, p_db_igsp_if, p_db_igsp_glb, &group_sort);
    }

    ccs_cfg_build_igmp_snooping_group_sort_free(&group_sort);
    
    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_vlanclass_rule(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_vlanclass_rule_t * p_db_vclass_rule = 0;
    tbl_vlanclass_rule_key_t vclass_rkey;
    char ipv4_addr_buf[CMD_BUF_256];
    char ipv6_addr_buf[CMD_BUF_256];
    char mac_addr_buf[CMD_BUF_32];
    uint32 i = 0;
    uint32 has_config = FALSE;

    for (i = 0; i< GLB_VLANCLASS_RULE_ID_MAX; i++)
    {
        sal_memset(&vclass_rkey, 0, sizeof(tbl_vlanclass_rule_key_t));
        vclass_rkey.rule_id = i;
        p_db_vclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&vclass_rkey);
        if (NULL == p_db_vclass_rule)
        {
            continue;
        }
        switch (p_db_vclass_rule->rule_type)
        {
        case GLB_VLAN_CLASSIFIER_MAC:
             sal_memset(mac_addr_buf, 0, sizeof(mac_addr_buf));
             sal_fprintf(fp, "vlan classifier rule %u mac %s vlan %u\n", 
                        p_db_vclass_rule->key.rule_id,
                        cdb_mac_addr_val2str(mac_addr_buf, CMD_BUF_32, p_db_vclass_rule->mac),
                        p_db_vclass_rule->vlan_id);
             has_config = TRUE;
             break;
        case GLB_VLAN_CLASSIFIER_IPV4:
             sal_memset(ipv4_addr_buf, 0, sizeof(ipv4_addr_buf));
             sal_fprintf(fp, "vlan classifier rule %u ip %s vlan %u\n",
                        p_db_vclass_rule->key.rule_id, 
                        cdb_addr_ipv4_val2str(ipv4_addr_buf, CMD_BUF_256, &p_db_vclass_rule->ip_address.u.prefix4),
                        p_db_vclass_rule->vlan_id);
             has_config = TRUE;
             break;
        case GLB_VLAN_CLASSIFIER_IPV6:
             sal_memset(ipv6_addr_buf, 0, sizeof(ipv6_addr_buf));
             sal_fprintf(fp, "vlan classifier rule %u ipv6 %s vlan %u\n",
                        p_db_vclass_rule->key.rule_id, 
                        cdb_addr_val2str(ipv6_addr_buf, CMD_BUF_256, &p_db_vclass_rule->ip_address), 
                        p_db_vclass_rule->vlan_id);
             break;
        case GLB_VLAN_CLASSIFIER_PROTOCOL:
             sal_fprintf(fp, "vlan classifier rule %u protocol %s vlan %u\n",
                        p_db_vclass_rule->key.rule_id, 
                        cdb_enum_val2str(vlanclass_rule_protype_strs, VLANCLASS_PROTYPE_MAX, p_db_vclass_rule->protocol_type),
                        p_db_vclass_rule->vlan_id);
             has_config = TRUE;
             break;
       }
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_vlanclass_group(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_vlanclass_group_t * p_db_vclass_group = 0;
    tbl_vlanclass_group_key_t vclass_gkey;
    uint32 i = 0;
    uint32 j = 0;
    uint32 has_config = FALSE;

    for (i = 0; i<= GLB_VLANCLASS_GROUP_ID_MAX; i++)
    {
        sal_memset(&vclass_gkey, 0, sizeof(tbl_vlanclass_group_key_t));
        vclass_gkey.group_id = i;
        p_db_vclass_group = tbl_vlanclass_group_get_vlanclass_group(&vclass_gkey);
        if (NULL == p_db_vclass_group)
        {
            continue;
        }
        for (j = 0; j< GLB_VLANCLASS_RULE_ID_MAX; j++)
        {
            if (GLB_BMP_ISSET(p_db_vclass_group->rule_id, j))
            {
                sal_fprintf(fp, "vlan classifier group %u add rule %u\n", i, j);
                has_config = TRUE;
            }
        }
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

#ifdef CONFIG_SFLOW
int32
ccs_cfg_build_sflow_global(FILE * fp, cfg_cmd_para_t * para)
{
    tbl_sflow_global_t   *p_sflow_glb = NULL;
    uint32               has_config = FALSE;
    char                 buf[CMD_BUF_256];

    p_sflow_glb = tbl_sflow_global_get_sflow_global();
 
    if (p_sflow_glb->enable)
    {
        sal_fprintf(fp, "sflow enable\n");
        has_config = TRUE;
    }

    if (0 != p_sflow_glb->agent.u.prefix4.s_addr)
    {
        sal_fprintf(fp, "sflow agent ip %s\n", 
            cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_sflow_glb->agent.u.prefix4));
        has_config = TRUE;
    }

    /* modified by liwh for bug 53920, 2019-10-11 */
    if (AF_INET6 == p_sflow_glb->agent_ipv6.family)
    {
        sal_fprintf(fp, "sflow agent ipv6 %s\n", 
            cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_sflow_glb->agent_ipv6.u.prefix6));
        has_config = TRUE;
    }
    /* liwh end */

    if (SFLOW_COUNTER_DEFAULT_INTERVAL != p_sflow_glb->counter_interval)
    {
        sal_fprintf(fp, "sflow counter interval %u\n", p_sflow_glb->counter_interval);
        has_config = TRUE;
    }

    if (has_config) 
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

static int32
tbl_sflow_collector_cfg_output(tbl_sflow_collector_t *p_collector, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs->argv[0];
    char ipbuf[256];
    int32 is_inband = 0;
    u_int32_t *pnCnt = (u_int32_t *)pargs->argv[1];
    int res;
    char buf[IFNAME_SIZE] = "";
    
    cdb_addr_val2str(ipbuf, 256, &p_collector->key.addr);

    if (AF_INET == p_collector->key.addr.family)
    {
        is_inband = ns_route_is_inband(p_collector->key.addr.u.prefix4, p_collector->key.udp_port, NS_ROUTE_UDP);
        if (-1 != is_inband)
        {
            sal_fprintf(fp, "sflow collector %s%s", (is_inband == 1) ? "" : "mgmt-if ", ipbuf);
        }
        
        if (GLB_SFLOW_COLLECTOR_DEFAULT_PORT != p_collector->key.udp_port) 
        {
            sal_fprintf(fp, " %u", p_collector->key.udp_port);
        } 
    
        res = inband_snat_get_if_or_ip(&p_collector->key.addr.u.prefix4, p_collector->key.udp_port, INBAND_IP_PROTO_UDP, buf);
        if (PM_E_NONE == res)
        {
            sal_fprintf(fp, " source-ip %s",buf);
        }
        sal_fprintf(fp, "\n");
        
        *pnCnt += 1;
    }
    else
    {
        is_inband = ns_route_ipv6_is_inband(p_collector->key.addr.u.prefix6, p_collector->key.udp_port, NS_ROUTE_UDP);

        if (-1 != is_inband)
        {
            sal_fprintf(fp, "sflow collector %s%s", (is_inband == 1) ? "" : "mgmt-if ", ipbuf);
        }
        
        if (GLB_SFLOW_COLLECTOR_DEFAULT_PORT != p_collector->key.udp_port) 
        {
            sal_fprintf(fp, " %u", p_collector->key.udp_port);
        }

        sal_fprintf(fp, "\n");
        
        *pnCnt += 1;
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_sflow_collector(FILE * fp, cfg_cmd_para_t * para)
{
    uint32 has_config = FALSE;
    tbl_iter_args_t args;

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = fp;
    args.argv[1] = &has_config;

    tbl_sflow_collector_iterate((TBL_ITER_CB_FUNC)tbl_sflow_collector_cfg_output, &args);

    if (has_config) 
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}
#endif

int32
ccs_cfg_build_l2_protocol(FILE * fp, cfg_cmd_para_t * para)
{
    uint32 has_config = FALSE;
    tbl_brg_global_t* p_brg_global = tbl_brg_global_get_brg_global();
    
    tbl_l2_protocol_master_t *p_master = tbl_l2_protocol_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_l2_protocol_t* p_db_l2_protocol1 = NULL;
    tbl_l2_protocol_t* p_db_l2_protocol2 = NULL;

    if (p_brg_global && (p_brg_global->l2protocol_cos))
    {
        sal_fprintf(fp, "l2protocol cos %u\n", p_brg_global->l2protocol_cos); 
        has_config = TRUE;
    }
    
    CTCLIB_SLIST_LOOP(p_master->l2_protocol_list, p_db_l2_protocol1, listnode)
    {
        if (!p_db_l2_protocol1->is_standard_pro)
        {
            sal_fprintf(fp, "l2protocol user-defined-protocol %s protocol-mac %.04hX.%.04hX.%.04hX\n", 
                p_db_l2_protocol1->key.name, 
                sal_ntoh16(((uint16*)p_db_l2_protocol1->protocol_mac)[0]),
                sal_ntoh16(((uint16*)p_db_l2_protocol1->protocol_mac)[1]), 
                sal_ntoh16(((uint16*)p_db_l2_protocol1->protocol_mac)[2]));
            has_config = TRUE;
        }
    }
    CTCLIB_SLIST_LOOP(p_master->l2_protocol_list, p_db_l2_protocol2, listnode)
    {
        if (!GLB_IS_ZERO_MAC(p_db_l2_protocol2->group_mac))
        {
            sal_fprintf(fp, "l2protocol %s group-mac %.04hX.%.04hX.%.04hX\n", 
                p_db_l2_protocol2->key.name, 
                sal_ntoh16(((uint16*)p_db_l2_protocol2->group_mac)[0]),
                sal_ntoh16(((uint16*)p_db_l2_protocol2->group_mac)[1]), 
                sal_ntoh16(((uint16*)p_db_l2_protocol2->group_mac)[2]));
        }
        has_config = TRUE;   
    }
    if (has_config) 
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}


int32
ccs_cfg_build_evc_entry(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_evc_entry_t     *p_db_evc = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *next = NULL;
    tbl_evc_entry_master_t *p_master = tbl_evc_entry_get_master();

    if (!p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->evc_entry_list, p_db_evc, listnode, next)
    {
        sal_fprintf(fp, "ethernet evc %s\n", p_db_evc->key.name);
        if (p_db_evc->mapped_cvid)
        {
            sal_fprintf(fp, INTEND_1_STR"dot1q mapped-double-vlan %d %d\n", p_db_evc->mapped_cvid, p_db_evc->mapped_svid);
        }
        else if (p_db_evc->mapped_svid)
        {
            sal_fprintf(fp, INTEND_1_STR"dot1q mapped-vlan %d\n", p_db_evc->mapped_svid);
        }
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

static char *
vlanmap_api_format_vlan_group(uint32 group_id, char *group_str)
{
    tbl_vlan_group_key_t vg_key;
    tbl_vlan_group_t    *p_db_vg = NULL;
    uint32  i = 0;

    sal_memset(&vg_key, 0, sizeof(vg_key));
    vg_key.group_id = group_id;
    p_db_vg = tbl_vlan_group_get_vlan_group(&vg_key);
    if (!p_db_vg)
    {
        return group_str;
    }
    for (i = 0; i < GLB_VLAN_RANGE_ENTRY_NUM; i++)
    {
        if (p_db_vg->valid[i])
        {
            sal_sprintf(group_str + sal_strlen(group_str), "%d-%d,", p_db_vg->min_vid[i], p_db_vg->max_vid[i]);
        }
    }
    if (sal_strlen(group_str))
    {
        group_str[sal_strlen(group_str) - 1] = '\0';
    }

    return group_str;
}

int32
ccs_cfg_build_vlan_group(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_vlan_group_t     *p_db_vg = NULL;
    tbl_vlan_group_master_t 
    *p_master = tbl_vlan_group_get_master();
    uint32 group_id = 0;
    char   group_str[100] = {0};
    uint32 group_exist = FALSE; 

    if (!p_master)
    {
        return PM_E_NONE;
    }

    for (group_id = 0; group_id < GLB_VLAN_RANGE_GROUP_NUM+1; group_id++)
    {
        group_str[0] = '\0';
        p_db_vg = p_master->vlan_group_array[group_id];
        if (NULL == p_db_vg)
        {
            continue;
        }
        sal_fprintf(fp, "raw vlan group %d vlan %s\n", group_id, vlanmap_api_format_vlan_group(group_id, group_str));

        group_exist = TRUE;
    }

    if (group_exist)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_vlan_mapping_table(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_vlan_mapping_table_master_t *p_vmt_master = tbl_vlan_mapping_table_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_vlan_mapping_entry_key_t vm_entry_key;
    tbl_vlan_mapping_entry_t *p_db_vm_entry = NULL;
    tbl_vlan_mapping_table_t *p_db_vm_table = NULL;
    tbl_mapped_vlan_info_master_t *p_mapped_master = tbl_mapped_vlan_info_get_master();
    ctclib_slistnode_t *listnode2 = NULL;
    ctclib_slistnode_t *next2 = NULL;
    tbl_egress_mapping_entry_master_t *p_egress_master = tbl_egress_mapping_entry_get_master();
    ctclib_slistnode_t *listnode3 = NULL;
    ctclib_slistnode_t *next3 = NULL;
    tbl_mapped_vlan_info_t *p_db_mapped_info = NULL;
    tbl_egress_mapping_entry_t *p_db_egress_vme = NULL;
    vlan_bmp_t bmp;
    uint32     all_zero = TRUE;
    char       buf[MAX_CMD_STR_LEN];
    vid_t      raw_vid;

    GLB_BMP_INIT(bmp);

    if (!p_vmt_master || !p_vmt_master || !p_mapped_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_vmt_master->vlan_mapping_table_list, p_db_vm_table, listnode, next)
    {
        sal_fprintf(fp, "vlan mapping table %s\n", p_db_vm_table->key.name);
        if (p_db_vm_table->group_id)
        {
            sal_fprintf(fp, INTEND_1_STR"raw-vlan group %d evc %s\n", 
                p_db_vm_table->group_id, p_db_vm_table->group_evc_entry_name);
        }
        CTCLIB_SLIST_LOOP_DEL(p_mapped_master->mapped_vlan_info_list, p_db_mapped_info, listnode2, next2)
        {
            if (sal_strncmp(p_db_mapped_info->key.table_name, p_db_vm_table->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
            {
                continue;
            }
            GLB_BMP_INIT(bmp);
            GLB_BMP_ITER_BEGIN(p_db_mapped_info->raw_vid_bmp, raw_vid)
            {
                if (GLB_VLAN_NONE == raw_vid)
                {
                    sal_fprintf(fp, INTEND_1_STR"raw-vlan untagged evc %s\n", p_db_mapped_info->key.evc_name);
                }
                else if (GLB_VLAN_ALL == raw_vid)
                {
                    /* move out-of-range entry to end of lines */
                    //sal_fprintf(fp, INTEND_1_STR"raw-vlan out-of-range evc %s\n", p_db_mapped_info->key.evc_name);
                }
                else
                {
                    GLB_BMP_SET(bmp, raw_vid);
                }
            }
            GLB_BMP_ITER_END(p_db_mapped_info->raw_vid_bmp, raw_vid);
            GLB_BMP_CHECK_ALL_ZERO(bmp, all_zero);
            if (!all_zero)
            {
                sal_memset(buf, 0, sizeof(buf));
                sal_fprintf(fp, INTEND_1_STR"raw-vlan %s evc %s\n", cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, bmp, GLB_VLAN_BMP_WORD_MAX), 
                            p_db_mapped_info->key.evc_name);
            }
        }

        sal_memset(&vm_entry_key, 0, sizeof(vm_entry_key));
        vm_entry_key.raw_vid = GLB_VLAN_NONE;
        sal_strncpy(vm_entry_key.table_name, p_db_vm_table->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
        p_db_vm_entry = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&vm_entry_key);
        if (p_db_vm_entry && GLB_VLAN_NONE != p_db_vm_entry->mapped_cvid)
        {
            sal_fprintf(fp, INTEND_1_STR"raw-vlan untagged evc %s\n", p_db_vm_entry->evc_entry);
        }

        sal_memset(&vm_entry_key, 0, sizeof(vm_entry_key));
        vm_entry_key.raw_vid = GLB_VLAN_ALL;
        sal_strncpy(vm_entry_key.table_name, p_db_vm_table->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
        p_db_vm_entry = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&vm_entry_key);
        if (p_db_vm_entry)
        {
            sal_fprintf(fp, INTEND_1_STR"raw-vlan out-of-range evc %s\n", p_db_vm_entry->evc_entry);
        }

        CTCLIB_SLIST_LOOP_DEL(p_egress_master->egress_mapping_entry_list, p_db_egress_vme, listnode3, next3)
        {
            if (sal_strncmp(p_db_egress_vme->key.table_name, p_db_vm_table->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
            {
                continue;
            }

            sal_fprintf(fp, INTEND_1_STR"raw-vlan %d %d egress-vlan ", p_db_egress_vme->key.raw_cvid, p_db_egress_vme->key.raw_svid);
            if (p_db_egress_vme->mapped_vid)
            {
                sal_fprintf(fp, "%d\n", p_db_egress_vme->mapped_vid);
            }
            else
            {
                sal_fprintf(fp, "%s\n", "untagged");
            }
        }
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_g8032_ring(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_g8032_ring_master_t *p_g8032_master = NULL;
    tbl_g8032_ring_t                 *p_g8032_ring = NULL;
    tbl_interface_t                      *p_east_interface = NULL;
    tbl_interface_t                      *p_west_interface = NULL;
    uint32                                                                      i = 0;
    char ifname_ext_east[IFNAME_EXT_SIZE];
    char ifname_ext_west[IFNAME_EXT_SIZE];
    ctclib_slistnode_t *listnode = NULL;

    p_g8032_master = tbl_g8032_ring_get_master();

    if (!p_g8032_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_g8032_master->g8032_ring_list, p_g8032_ring, listnode)
    {
        if (p_g8032_ring->west_port_ifindex)
        {
            p_east_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
            p_west_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
            IFNAME_ETH2FULL(p_east_interface->key.name, ifname_ext_east);
            IFNAME_ETH2FULL(p_west_interface->key.name, ifname_ext_west);
            if (p_east_interface && p_west_interface)
            {
                sal_fprintf(fp, "g8032 ring-id %d east-interface %s west-interface %s %s\n", 
                                        p_g8032_ring->key.ring_id, ifname_ext_east, 
                                        ifname_ext_west, (p_g8032_ring->is_sub_ring) ? "is-sub-ring" : "");

                /* modified by liwh for bug 53010, 2019-07-23 */
                if (0 != sal_strlen(p_g8032_ring->cfm_md_name))
                {
                    sal_fprintf(fp, "  domain %s service %s\n", p_g8032_ring->cfm_md_name, p_g8032_ring->cfm_ma_name);    
                }
                /* liwh end */
                    
                if (p_g8032_ring->instance_count)
                {
                    for (i = 0; i < p_g8032_ring->instance_count; i++)
                    {
                        sal_fprintf(fp, "  instance %d\n", p_g8032_ring->instance[i]);  
                    }
                }

                if (p_g8032_ring->control_vlan)
                {
                    sal_fprintf(fp, "  control-vlan %d\n", p_g8032_ring->control_vlan);  
                }
                

                if (p_g8032_ring->rpl_owner)
                {
                    if (p_g8032_ring->east_port_is_rpl)
                    {
                        sal_fprintf(fp, "  rpl owner east-interface\n");  
                    }
                    else if (p_g8032_ring->west_port_is_rpl)
                    {
                        sal_fprintf(fp, "  rpl owner west-interface\n");  
                    }
                    
                }

                if (p_g8032_ring->rpl_neighbor)
                {
                    if (p_g8032_ring->east_port_is_rpl_neighbor)
                    {
                        sal_fprintf(fp, "  rpl neighbor east-interface\n");  
                    }
                   else if (p_g8032_ring->west_port_is_rpl_neighbor)
                    {
                        sal_fprintf(fp, "  rpl neighbor west-interface\n");  
                    }
                    
                }

                if (GLB_G8032_RING_DEFAULT_WTR_TIMER_INTERVAL_IN_MIN != p_g8032_ring->wtr_timeout_val)
                {
                    sal_fprintf(fp, "  timer wait-to-restore %d\n", p_g8032_ring->wtr_timeout_val);  
                }

                if (GLB_G8032_RING_DEFAULT_GUARD_TIMER_INTERVAL_MS != p_g8032_ring->guard_timeout_val)
                {
                    sal_fprintf(fp, "  timer guard-time %d\n", p_g8032_ring->guard_timeout_val);  
                }

                if ( p_g8032_ring->hold_off_timeout_val)
                {
                    sal_fprintf(fp, "  timer hold-off %d\n", p_g8032_ring->hold_off_timeout_val);  
                }

                if ( GLB_G8032_DEF_RAPS_MEL_VALUE != p_g8032_ring->ring_level_id)
                {
                    sal_fprintf(fp, "  ring-level %d\n", p_g8032_ring->ring_level_id);  
                }

                if (p_g8032_ring->virtual_channel)
                {
                    sal_fprintf(fp, "  virtual-channel enable\n");  
                }

                if (!p_g8032_ring->mode_revertive)
                {
                    sal_fprintf(fp, "  mode non-revertive\n");  
                }

                 if (p_g8032_ring->protection_en)
                {
                    sal_fprintf(fp, "  ring enable\n");  
                }

                  sal_fprintf(fp, "!\n");
                
            }
        }
        else
        {
                p_east_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
                IFNAME_ETH2FULL(p_east_interface->key.name, ifname_ext_east);
                if (p_east_interface)
                {
                    sal_fprintf(fp, "g8032 ring-id %d interface %s major-ring-id %d\n", 
                                        p_g8032_ring->key.ring_id, ifname_ext_east, 
                                        p_g8032_ring->g8032_major_ring_id);

                    /* modified by liwh for bug 53010, 2019-07-23 */
                    if (0 != sal_strlen(p_g8032_ring->cfm_md_name))
                    {
                        sal_fprintf(fp, " domain %s service %s", p_g8032_ring->cfm_md_name, p_g8032_ring->cfm_ma_name);    
                    }
                    /* liwh end */
                        
                    if (p_g8032_ring->instance_count)
                    {
                         for (i = 0; i < p_g8032_ring->instance_count; i++)
                        {
                            sal_fprintf(fp, "  instance %d\n", p_g8032_ring->instance[i]);  
                        }
                    }

                    if (p_g8032_ring->control_vlan)
                    {
                        sal_fprintf(fp, "  control-vlan %d\n", p_g8032_ring->control_vlan);  
                    }
                    

                    if (p_g8032_ring->rpl_owner)
                    {
                        if (p_g8032_ring->east_port_is_rpl)
                        {
                            sal_fprintf(fp, "  rpl owner east-interface\n");  
                        }
                        else if (p_g8032_ring->west_port_is_rpl)
                        {
                            sal_fprintf(fp, "  rpl owner west-interface\n");  
                        }
                        
                    }

                    if (p_g8032_ring->rpl_neighbor)
                    {
                        if (p_g8032_ring->east_port_is_rpl_neighbor)
                        {
                            sal_fprintf(fp, "  rpl neighbor east-interface\n");  
                        }
                        else if (p_g8032_ring->west_port_is_rpl_neighbor)
                        {
                            sal_fprintf(fp, "  rpl neighbor west-interface\n");  
                        }
                        
                    }

                     
                    if (GLB_G8032_RING_DEFAULT_WTR_TIMER_INTERVAL_IN_MIN != p_g8032_ring->wtr_timeout_val)
                    {
                        sal_fprintf(fp, "  timer wait-to-restore %d\n", p_g8032_ring->wtr_timeout_val);  
                    }

                    if (GLB_G8032_RING_DEFAULT_GUARD_TIMER_INTERVAL_MS != p_g8032_ring->guard_timeout_val)
                    {
                        sal_fprintf(fp, "  timer guard-time %d\n", p_g8032_ring->guard_timeout_val);  
                    }

                    if ( p_g8032_ring->hold_off_timeout_val)
                    {
                        sal_fprintf(fp, "  timer hold-off %d\n", p_g8032_ring->hold_off_timeout_val);  
                    }

                    if ( GLB_G8032_DEF_RAPS_MEL_VALUE != p_g8032_ring->ring_level_id)
                    {
                        sal_fprintf(fp, "  ring-level %d\n", p_g8032_ring->ring_level_id);  
                    }

                    if (p_g8032_ring->virtual_channel)
                    {
                        sal_fprintf(fp, "  virtual-channel enable\n");  
                    }

                    if (!p_g8032_ring->mode_revertive)
                    {
                        sal_fprintf(fp, "  mode non-revertive\n");  
                    }

                     if (p_g8032_ring->protection_en)
                    {
                        sal_fprintf(fp, "  ring enable\n");  
                    }

                      sal_fprintf(fp, "!\n");
                        
                    }
             }
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}
