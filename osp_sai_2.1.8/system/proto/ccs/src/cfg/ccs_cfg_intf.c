
#include "proto.h"
#include "glb_const.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_fdb.h"
#include "gen/tbl_psfdb_define.h"
#include "gen/tbl_psfdb.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_global.h"
#include "gen/tbl_msti_port_define.h"
#include "gen/tbl_msti_port.h"
#include "gen/tbl_aclqos_if_define.h"
#include "gen/tbl_aclqos_if.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_qos_global_define.h"
#include "gen/tbl_qos_global.h"
#include "gen/ds_circuit_id_define.h"
#include "mstp_config.h"
#include "mstp_define.h"
#include "lacp_config.h"
#include "lldp_define.h"
#include "ipmc_define.h"
#include "glb_chip_specific.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_lldp_if_define.h"
#include "gen/tbl_lldp_if.h"
#include "gen/tbl_dot1x_port_define.h"
#include "gen/tbl_dot1x_port.h"
#include "gen/tbl_openflow_interface_define.h"
#include "gen/tbl_openflow_interface.h"
#include "gen/tbl_openflow_tunnel_interface_define.h"
#include "gen/tbl_openflow_tunnel_interface.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_port_stats_rate_define.h"
#include "gen/tbl_port_stats_rate.h"
#include "gen/tbl_ssm_global_define.h"
#include "gen/tbl_ssm_global.h"
#include "gen/tbl_ssm_port_define.h"
#include "gen/tbl_ssm_port.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"
#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_ipmc_intf_define.h"
#include "gen/tbl_ipmc_intf.h"
#include "gen/tbl_rmon_ether_stats_define.h"
#include "gen/tbl_rmon_ether_stats.h"
#include "gen/tbl_rmon_history_define.h"
#include "gen/tbl_rmon_history.h"
#include "gen/tbl_hash_value_profile_define.h"
#include "gen/tbl_hash_value_profile.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "gen/tbl_cfm_lmep_define.h"
#include "gen/tbl_cfm_lmep.h"
#include "gen/tbl_cfm_rmep_define.h"
#include "gen/tbl_cfm_rmep.h"

#include "rsa_derhex.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"


int app_is_password_encryption(void)
{
    tbl_sys_global_t* pGlobal = tbl_sys_global_get_sys_global();
    if (pGlobal && pGlobal->service_password_encryption) {
        return 1;
    }
    return 0;
}

#define CMD_BUF_256             256

enum
{
    CCS_CFG_BRGIF_ALLOW_VLAN_ACCESS = 0,
    CCS_CFG_BRGIF_ALLOW_VLAN_TRUNK,
    CCS_CFG_BRGIF_ALLOW_VLAN_QINQ,
    CCS_CFG_BRGIF_ALLOW_VLAN_MAX
};

static char *ccs_cfg_brgif_allow_vlan_str[CCS_CFG_BRGIF_ALLOW_VLAN_MAX] =
{
    INTEND_1_STR"switchport access allowed vlan ",
    INTEND_1_STR"switchport trunk allowed vlan ",
    INTEND_1_STR"switchport dot1q-tunnel allowed vlan ",
};

#define CCS_CFG_ALLOW_VLAN_WRITE_FILE(_line, _type, _action,_has_left_vlan) \
    _line[sal_strlen(_line) - 1] = CMD_ZERO_CHAR; \
    sal_fprintf(fp,"%s\n", _line); \
    sal_sprintf(_line, "%s%s", ccs_cfg_brgif_allow_vlan_str[_type], _action); \
    _has_left_vlan = FALSE;

#define CCS_CFG_UNTAGGED_VLAN_WRITE_FILE(_line, _action, _has_left_vlan) \
    _line[sal_strlen(_line) - 1] = CMD_ZERO_CHAR; \
    sal_fprintf(fp,"%s\n", _line); \
    sal_sprintf(_line, INTEND_1_STR"switchport trunk untagged vlan %s", _action); \
    _has_left_vlan = FALSE;

#ifndef TAPPRODUCT

static int32
_ccs_cfg_get_port_msti_list(tbl_mstp_port_t *p_db_port, tbl_msti_port_t **pp_db_msti_port, uint32 *msti_count)
{
    tbl_msti_port_master_t *p_master = tbl_msti_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_msti_port_t *p_db_msti_port = NULL;
    uint32 count = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_master->port_msti_list, p_db_msti_port, listnode)
    {
        if (0 == sal_strcmp(p_db_msti_port->key.name, p_db_port->key.name))
        {
            pp_db_msti_port[count] = p_db_msti_port;
            count++;
        }
    }

    *msti_count = count;

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

static int32
_ccs_cfg_build_mstp_port_one(FILE* fp, tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_msti_port_t *pp_db_msti_port[GLB_STP_INSTANCE_MAX];
    uint32 msti_count = 0;
    uint32 i = 0;

    sal_memset(pp_db_msti_port, 0, sizeof(tbl_msti_port_t*)*GLB_STP_INSTANCE_MAX);
    _ccs_cfg_get_port_msti_list(p_db_port, pp_db_msti_port, &msti_count);

    if (TRUE != p_db_port->port_enable)
    {
        sal_fprintf(fp, INTEND_1_STR"spanning-tree port disable\n");
    }

    if (MSTP_DEF_PORT_PRIORITY != p_db_port->priority)
    {
        sal_fprintf(fp, INTEND_1_STR"spanning-tree port-priority %u\n", p_db_port->priority);
    }

    if (0 != p_db_port->adminPortPathCost)
    {
        sal_fprintf(fp, INTEND_1_STR"spanning-tree path-cost %u\n", p_db_port->adminPortPathCost);
    }

    if (MSTP_TYPE_RSTP == p_db_glb->type)
    {
        if (BPDU_VERSION_DEFAULT != p_db_port->adminForceVersion)
        {
            sal_fprintf(fp, INTEND_1_STR"spanning-tree force-version %u\n", p_db_port->adminForceVersion);
        }
    }
    else if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        if (BPDU_VERSION_DEFAULT != p_db_port->adminForceVersion)
        {
            sal_fprintf(fp, INTEND_1_STR"spanning-tree force-version %u\n", p_db_port->adminForceVersion);
        }
    }
    
    if (MSTP_EDGEPORT_DEFAULT != p_db_port->admin_bpduguard)
    {
        sal_fprintf(fp, INTEND_1_STR"spanning-tree edgeport bpdu-guard %s\n",
            cdb_enum_val2str(mstp_edgeport_strs, MSTP_EDGEPORT_MAX, p_db_port->admin_bpduguard));
    }

    /* modified by liwh for bug 37514, 2016-05-17 */
    if (p_db_port->restricted_tcn)
    {
        sal_fprintf(fp, INTEND_1_STR"spanning-tree restricted-tcn\n");        
    }

    if (p_db_port->restricted_role)
    {
        sal_fprintf(fp, INTEND_1_STR"spanning-tree restricted-role\n");        
    }
    /* liwh end */
    
    if (MSTP_EDGEPORT_DEFAULT != p_db_port->admin_bpdufilter)
    {
        sal_fprintf(fp, INTEND_1_STR"spanning-tree edgeport bpdu-filter %s\n",
            cdb_enum_val2str(mstp_edgeport_strs, MSTP_EDGEPORT_MAX, p_db_port->admin_bpdufilter));
    }
    
    if (MSTP_LINK_SHARED == p_db_port->adminP2PMac)
    {
        sal_fprintf(fp, INTEND_1_STR"spanning-tree link-type shared\n");
    }
    else if (MSTP_LINK_P2P == p_db_port->adminP2PMac)
    {
        sal_fprintf(fp, INTEND_1_STR"spanning-tree link-type point-to-point\n");
    }
    
    if (p_db_port->adminEdge)
    {
        sal_fprintf(fp, INTEND_1_STR"spanning-tree edgeport\n");
    }
    
    if (p_db_port->admin_rootguard)
    {
        sal_fprintf(fp, INTEND_1_STR"spanning-tree guard root\n");
    }
    
    if (p_db_port->admin_loopguard)
    {
        sal_fprintf(fp, INTEND_1_STR"spanning-tree guard loop\n");
    }

    for (i = 0; i < msti_count; i++)
    {
        if (0 != pp_db_msti_port[i]->adminPortPathCost)
        {
            sal_fprintf(fp, INTEND_1_STR"spanning-tree instance %u path-cost %u\n", 
                pp_db_msti_port[i]->key.instance, pp_db_msti_port[i]->adminPortPathCost);
        }
        if (MSTP_DEF_PORT_PRIORITY != pp_db_msti_port[i]->priority)
        {
            sal_fprintf(fp, INTEND_1_STR"spanning-tree instance %u port-priority %u\n", 
                pp_db_msti_port[i]->key.instance, pp_db_msti_port[i]->priority);
        }
        if (pp_db_msti_port[i]->restricted_tcn)
        {
            sal_fprintf(fp, INTEND_1_STR"spanning-tree instance %u restricted-tcn\n", pp_db_msti_port[i]->key.instance);        
        }
        if (pp_db_msti_port[i]->restricted_role)
        {
            sal_fprintf(fp, INTEND_1_STR"spanning-tree instance %u restricted-role\n", pp_db_msti_port[i]->key.instance);        
        }
    }
    
    return PM_E_NONE;
}
#endif /* TAPPRODUCT */

static int32
_ccs_cfg_openflow_packet_type_to_str(uint32 type, char *str)
{
    int32 ret = PM_E_FAIL;

    if (type >= GLB_OPF_PKT_TYPE_MAX || type < GLB_OPF_PKT_TYPE_STP)
    {
        return PM_E_FAIL;
    }

    if (GLB_OPF_PKT_TYPE_STP == type)
    {
        sal_strcpy(str, "stp");
        ret = PM_E_NONE;
    }
    if (GLB_OPF_PKT_TYPE_LACP == type)
    {
        sal_strcpy(str, "lacp");
        ret = PM_E_NONE;
    }
    if (GLB_OPF_PKT_TYPE_LLDP == type)
    {
        sal_strcpy(str, "lldp");
        ret = PM_E_NONE;
    }
    if (GLB_OPF_PKT_TYPE_IGMP == type)
    {
        sal_strcpy(str, "igmp");
        ret = PM_E_FAIL;
    }
    if (GLB_OPF_PKT_TYPE_ERPS == type)
    {
        sal_strcpy(str, "erps");
        ret = PM_E_NONE;
    }
    if (GLB_OPF_PKT_TYPE_DOT1X == type)
    {
        sal_strcpy(str, "dot1x");
        ret = PM_E_NONE;
    }
    if (GLB_OPF_PKT_TYPE_ARP == type)
    {
        sal_strcpy(str, "arp");
        ret = PM_E_NONE;
    }
    return ret;
}

static int32
_ccs_cfg_psfdb_get_static(tbl_psfdb_t *p_psfdb, tbl_iter_args_t *pargs)
{
    char mac_addr_buf[CMD_BUF_32];
    uint32 ifindex = *(uint32*)pargs->argv[0];
    FILE *fp = pargs->argv[1];

    if (GLB_FLAG_ISSET(p_psfdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
    {
        if (p_psfdb->ifindex == ifindex)
        {
            sal_fprintf(fp, INTEND_1_STR"switchport port-security mac-address %s vlan %u\n", 
                    cdb_mac_addr_val2str(mac_addr_buf, CMD_BUF_32, p_psfdb->key.mac), p_psfdb->key.fid);
        }
    }
    return PM_E_NONE;
}

int32
_ccs_cfg_build_psfdb_of_one_port(FILE* fp, tbl_interface_t *p_db_if)
{
    tbl_iter_args_t args;
    
    if(NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    
    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &p_db_if->ifindex;
    args.argv[1] = fp;

    tbl_psfdb_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_psfdb_get_static, &args);

    return PM_E_NONE;
}

int32
_ccs_cfg_build_brgif_allowed_vlan(FILE* fp, tbl_interface_t *p_db_if, ds_brgif_t *p_db_brgif, uint32 type)
{
    vid_t vid = 0;
    vid_t start_offset = 0;
    vid_t end_vid = 0;
    uint32 offset = 0;
    char tmp[16];
    char line[256];
    uint32 has_left_vlan = FALSE;
    vlan_bmp_t vlan_bmp;

    sal_sprintf(line, "%sadd ", ccs_cfg_brgif_allow_vlan_str[type]);

    if (CCS_CFG_BRGIF_ALLOW_VLAN_ACCESS == type)
    {
        start_offset = GLB_DEF_VLAN_ID;
    }
    else
    {
        start_offset = GLB_DEF_VLAN_ID+1;
    }

    sal_memcpy(vlan_bmp, p_db_brgif->allowed_vlan.vlan_bmp, sizeof(vlan_bmp));
    if (CCS_CFG_BRGIF_ALLOW_VLAN_ACCESS == type)
    {
        GLB_BMP_UNSET(vlan_bmp, p_db_brgif->pvid);
    }
    
    for (offset = start_offset; offset < GLB_BMP_BITS((vlan_bmp)); offset++)
    {
        if (GLB_BMP_ISSET(vlan_bmp, offset))
        {
            vid = offset;
            end_vid = vid;
            do 
            {
                end_vid++;
            } while (GLB_BMP_ISSET(vlan_bmp, end_vid));
            end_vid--;
            offset = end_vid;
            if (vid == end_vid)
            {
                sal_sprintf(tmp, "%u,", vid);
                if (sal_strlen(line)+sal_strlen(tmp) >= MAX_CMD_LINE_LEN)
                {
                    CCS_CFG_ALLOW_VLAN_WRITE_FILE(line, type, "add ", has_left_vlan);
                }
                sal_strncat(line, tmp, 5);
                has_left_vlan = TRUE;
            }                
            else
            {          
                if (end_vid == (vid+1))
                {
                    sal_sprintf(tmp, "%u,%u,", vid, end_vid);
                }
                else
                {
                    sal_sprintf(tmp, "%u-%u,", vid, end_vid);
                }
                if (sal_strlen(line)+sal_strlen(tmp) >= MAX_CMD_LINE_LEN)
                {
                    CCS_CFG_ALLOW_VLAN_WRITE_FILE(line, type, "add ", has_left_vlan);
                }
                sal_strncat(line,tmp,10);
                has_left_vlan = TRUE;
            }                             
        }
    }
    
    if (has_left_vlan)
    {
        CCS_CFG_ALLOW_VLAN_WRITE_FILE(line, CCS_CFG_BRGIF_ALLOW_VLAN_TRUNK, "add ", has_left_vlan);
    }

    if (CCS_CFG_BRGIF_ALLOW_VLAN_ACCESS != type)
    {
        if (!GLB_BMP_ISSET(vlan_bmp, GLB_DEF_VLAN_ID))
        {
            sal_sprintf(line, "%s%s%u", ccs_cfg_brgif_allow_vlan_str[type], "remove ", GLB_DEF_VLAN_ID);
            sal_fprintf(fp,"%s\n", line);
        }
    }
    
    return PM_E_NONE;
}

int32
_ccs_cfg_build_brgif_untagged_vlan(FILE* fp, tbl_interface_t *p_db_if, ds_brgif_t *p_db_brgif, uint32 type)
{
    vid_t vid = 0;
    vid_t start_offset = 0;
    vid_t end_vid = 0;
    uint32 offset = 0;
    char tmp[16];
    char line[256];
    uint32 has_left_vlan = FALSE;
    vlan_bmp_t vlan_bmp;
    
    sal_sprintf(line, INTEND_1_STR"switchport trunk untagged vlan add ");

    start_offset = GLB_DEF_VLAN_ID;

    sal_memcpy(vlan_bmp, p_db_brgif->untagged_vlan.vlan_bmp, sizeof(vlan_bmp));
    if (CCS_CFG_BRGIF_ALLOW_VLAN_ACCESS == type)
    {
        GLB_BMP_UNSET(vlan_bmp, p_db_brgif->pvid);
    }
    
    for (offset = start_offset; offset < GLB_BMP_BITS((vlan_bmp)); offset++)
    {
        if (GLB_BMP_ISSET(vlan_bmp, offset))
        {
            vid = offset;
            end_vid = vid;
            do 
            {
                end_vid++;
            } while (GLB_BMP_ISSET(vlan_bmp, end_vid));
            end_vid--;
            offset = end_vid;
            if (vid == end_vid)
            {
                sal_sprintf(tmp, "%u,", vid);
                if (sal_strlen(line)+sal_strlen(tmp) >= MAX_CMD_LINE_LEN)
                {
                    CCS_CFG_UNTAGGED_VLAN_WRITE_FILE(line, "add ", has_left_vlan);
                }
                sal_strncat(line, tmp, 5);
                has_left_vlan = TRUE;
            }                
            else
            {          
                if (end_vid == (vid+1))
                {
                    sal_sprintf(tmp, "%u,%u,", vid, end_vid);
                }
                else
                {
                    sal_sprintf(tmp, "%u-%u,", vid, end_vid);
                }
                if (sal_strlen(line)+sal_strlen(tmp) >= MAX_CMD_LINE_LEN)
                {
                    CCS_CFG_UNTAGGED_VLAN_WRITE_FILE(line, "add ", has_left_vlan);
                }
                sal_strncat(line,tmp,10);
                has_left_vlan = TRUE;
            }                             
        }
    }
    
    if (has_left_vlan)
    {
        CCS_CFG_UNTAGGED_VLAN_WRITE_FILE(line, "add ", has_left_vlan);
    }

    return PM_E_NONE;
}

static char*
ccs_cfg_ssm_quality_to_desc(uint32 quality_value)
{
    tbl_ssm_global_t* p_ssm_glb = tbl_ssm_global_get_ssm_global();
    switch (quality_value)
    {
     case SSM_QL_DNU:
         if (SSM_OPTION_MODE_1 == p_ssm_glb->option_mode)
         {
             return "dnu";
         }
         else
         {
             return "dus";
         }
     case SSM_QL_EEC1:
         return "eec1";
     case SSM_QL_EEC2:
         return "eec2";
     case SSM_QL_PRC:
         return "prc";
     case SSM_QL_PRS:
         return "prs";   
     case SSM_QL_STU:
         return "stu";
     case SSM_QL_TNC:
         if (SSM_OPTION_MODE_1 == p_ssm_glb->option_mode)
         {
             return "ssu-a";
         }
         else
         {
             return "tnc";
         }
     case SSM_QL_ST2:
         return "st2";
     case SSM_QL_SSUB:
         return "ssu-b";
     case SSM_QL_SMC:
         return "smc";
     case SSM_QL_ST3E:
         return "st3e";
     case SSM_QL_PROV:
         return "prov";

     default:
         return "dnu";
    }
}

int32
ccs_cfg_build_interface_one_synce_port(FILE* fp, tbl_interface_t *p_db_if)
{
    tbl_ssm_port_key_t ssm_port_key;
    tbl_ssm_port_t* p_db_ssm_port = NULL;
    
    /*SyncE*/
    sal_memset(&ssm_port_key, 0x0, sizeof(ssm_port_key));
    sal_strncpy(ssm_port_key.name, p_db_if->key.name, IFNAME_SIZE);
    p_db_ssm_port = tbl_ssm_port_get_ssm_port(&ssm_port_key);
    if (p_db_ssm_port)
    {
        if (p_db_ssm_port->synce_enable)
        {
            sal_fprintf(fp, INTEND_1_STR"clock synce enable\n");
        }

        if (SSM_PDU_ALL != p_db_ssm_port->ssm_pdu_direct)
        {
            if (SSM_PDU_NONE != p_db_ssm_port->ssm_pdu_direct)
            {
                if (SSM_PDU_RX == p_db_ssm_port->ssm_pdu_direct)
                {
                    sal_fprintf(fp, INTEND_1_STR"no clock synce ssm enable tx\n");
                }
                if (SSM_PDU_TX == p_db_ssm_port->ssm_pdu_direct)
                {
                    sal_fprintf(fp, INTEND_1_STR"no clock synce ssm enable rx\n");
                }
            }
            else
            {   
                sal_fprintf(fp, INTEND_1_STR"no clock synce ssm enable all\n"); 
            }
        }

        if (SSM_IF_DEFAULT_PRIORITY != p_db_ssm_port->priority)
        {
            sal_fprintf(fp, INTEND_1_STR"clock synce priority %d\n", p_db_ssm_port->priority);    
        }

        if (p_db_ssm_port->replace_clockid_en)
        {
            sal_fprintf(fp, INTEND_1_STR"clock synce extend replace-clockid\n"); 
        }

        if (p_db_ssm_port->dnu_group_id)
        {
            sal_fprintf(fp, INTEND_1_STR"clock synce dnu-group %d\n", p_db_ssm_port->dnu_group_id);
        }

        if (SSM_QL_INV3 != p_db_ssm_port->force_rx_quality)
        {
            sal_fprintf(fp, INTEND_1_STR"clock synce quality-eth rx %s\n", ccs_cfg_ssm_quality_to_desc(p_db_ssm_port->force_rx_quality));
        }

        if (SSM_QL_INV3 != p_db_ssm_port->force_tx_quality)
        {
            sal_fprintf(fp, INTEND_1_STR"clock synce quality-eth tx %s\n", ccs_cfg_ssm_quality_to_desc(p_db_ssm_port->force_tx_quality));
        }
    }
    return PM_E_NONE;
}

void
ccs_cfg_build_interface_one_ptp_port(FILE* fp, tbl_interface_t *p_db_if)
{
    #define PTP_IP_ADDR_STR_LEN 16
    
    tbl_ptp_port_t *p_db_ptp_port = NULL;
    tbl_ptp_port_key_t ptp_port_key;
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();

    sal_memset(&ptp_port_key,0x0, sizeof(ptp_port_key));
    sal_strcpy(ptp_port_key.name, p_db_if->key.name);
    p_db_ptp_port = tbl_ptp_port_get_ptp_port(&ptp_port_key);
    if(p_db_ptp_port && 0 == p_db_ptp_port->lag_id)
    {
        if (PTP_ASYMMETRY_CORRECTION_DEF != p_db_ptp_port->asymmetry_correction.v)
        {
            sal_fprintf(fp, INTEND_1_STR"ptp asymmetry-correction %"PRId64"\n", p_db_ptp_port->asymmetry_correction.v);
        }
/*
        if (PTP_NUMBER_OF_ALTERNATE_MASTERS_DEF != p_port->number_of_alternate_masters)
        {
            sal_fprintf(fp, INTEND_1_STR"ptp number-of-alternate-masters %d\n", p_port->number_of_alternate_masters);
        }
        */
        if(PTP_LATENCY_CORRECTION_DEF != p_db_ptp_port->ingress_latency)
        {
            sal_fprintf(fp, INTEND_1_STR"ptp ingress-latency %d\n", p_db_ptp_port->ingress_latency);
        }

        if(PTP_LATENCY_CORRECTION_DEF != p_db_ptp_port->egress_latency)
        {
            sal_fprintf(fp, INTEND_1_STR"ptp egress-latency %d\n", p_db_ptp_port->egress_latency);
        }

        if (PTP_DEVICE_E2ETC == g_p_master->device_type || PTP_DEVICE_P2PTC == g_p_master->device_type)
        {
            /*_ptp_cli_interface_config_write_tc*/
            if (PTP_DELAY_MECHANISM_P2P == p_db_ptp_port->port_ds.delay_mechanism)
            {
                if (PTP_MIN_PDELAYREQ_INTERVAL_DEF != p_db_ptp_port->port_ds.log_min_pdelay_req_interval)
                {
                    sal_fprintf(fp, INTEND_1_STR"ptp min-pdelayreq-interval %d\n", p_db_ptp_port->port_ds.log_min_pdelay_req_interval);
                }
            }
        }
        else
        {
            /*_ptp_cli_interface_config_write_bc*/
            if (PTP_DELAY_MECHANISM_DEF != p_db_ptp_port->port_ds.delay_mechanism)
            {
                if (PTP_DELAY_MECHANISM_E2E == p_db_ptp_port->port_ds.delay_mechanism)
                {
                    sal_fprintf(fp, INTEND_1_STR"ptp delay-mechanism %s\n", "normal");
                }
                else if (PTP_DELAY_MECHANISM_P2P == p_db_ptp_port->port_ds.delay_mechanism)
                {
                    sal_fprintf(fp, INTEND_1_STR"ptp delay-mechanism %s\n", "peer");
                }
                else
                {
                    sal_fprintf(fp, INTEND_1_STR"ptp delay-mechanism %s\n", "disable");
                }
            }

            if (PTP_ANNOUNCE_INTERVAL_DEF != p_db_ptp_port->port_ds.log_announce_interval)
            {
                sal_fprintf(fp, INTEND_1_STR"ptp announce-interval %d\n", p_db_ptp_port->port_ds.log_announce_interval);
            }

            if (PTP_SYNC_INTERVAL_DEF != p_db_ptp_port->port_ds.log_sync_interval)
            {
                sal_fprintf(fp, INTEND_1_STR"ptp sync-interval %d\n", p_db_ptp_port->port_ds.log_sync_interval);
            }

            if (PTP_MIN_DELAYREQ_INTERVAL_DEF != p_db_ptp_port->port_ds.log_min_delay_req_interval)
            {
                sal_fprintf(fp, INTEND_1_STR"ptp min-delayreq-interval %d\n", p_db_ptp_port->port_ds.log_min_delay_req_interval);
            }

            if (PTP_MIN_PDELAYREQ_INTERVAL_DEF != p_db_ptp_port->port_ds.log_min_pdelay_req_interval)
            {
                sal_fprintf(fp, INTEND_1_STR"ptp min-pdelayreq-interval %d\n", p_db_ptp_port->port_ds.log_min_pdelay_req_interval);
            }

            if (PTP_ANNOUNCE_RECEIPT_TIMEOUT_DEF != p_db_ptp_port->port_ds.announce_receipt_timeout)
            {
                sal_fprintf(fp, INTEND_1_STR"ptp announce-receipt-timeout %u\n", p_db_ptp_port->port_ds.announce_receipt_timeout);
            }
        }

        /*_ptp_cli_interface_port_addr_config_write*/
        if (PTP_NET_PROTO_DEF != p_db_ptp_port->port_addr.protocol)
        {
            if (PTP_NET_PROTO_IEEE_802_3 == p_db_ptp_port->port_addr.protocol)
            {
                sal_fprintf(fp, INTEND_1_STR"ptp protocol mac\n");
            }
            else if (PTP_NET_PROTO_UDP_IPV4 == p_db_ptp_port->port_addr.protocol)
            {
                char tmp[PTP_IP_ADDR_STR_LEN];
                
                sal_memset(tmp, 0x00, PTP_IP_ADDR_STR_LEN);
                cdb_addr_ipv4_val2str(tmp, sizeof(tmp), &(p_db_ptp_port->port_addr.field.ip_addr));
                
                //uint32 ip = sal_hton32(p_db_ptp_port->port_addr.field.ip_addr.s_addr);
                //sal_inet_ntop(AF_INET, &ip, tmp, PTP_IP_ADDR_STR_LEN);
                
                sal_fprintf(fp, INTEND_1_STR"ptp protocol udp source-ip %s\n", tmp);
            }
            else if (PTP_NET_PROTO_UDP_IPV4_UNICAST == p_db_ptp_port->port_addr.protocol)
            {
                char tmp_sa[PTP_IP_ADDR_STR_LEN];
                char tmp_da[PTP_IP_ADDR_STR_LEN];
                
                sal_memset(tmp_sa, 0x00, PTP_IP_ADDR_STR_LEN);
                cdb_addr_ipv4_val2str(tmp_sa, sizeof(tmp_sa), &(p_db_ptp_port->port_addr.field.ip_addr));
                
                //uint32 ip_sa = sal_hton32(p_db_ptp_port->port_addr.field.ip_addr.s_addr);
                //sal_inet_ntop(AF_INET, &ip_sa, tmp_sa, PTP_IP_ADDR_STR_LEN);
                
                sal_memset(tmp_da, 0x00, PTP_IP_ADDR_STR_LEN);
                //cdb_addr_ipv4_val2str(tmp_da, sizeof(tmp_da), &(p_db_ptp_port->port_addr.dest_ip_addr));
                
                uint32 ip_da = (p_db_ptp_port->port_addr.dest_ip_addr.s_addr);
                sal_inet_ntop(AF_INET, &ip_da, tmp_da, PTP_IP_ADDR_STR_LEN);

                sal_fprintf(fp, INTEND_1_STR"ptp protocol udp source-ip %s unicast dest-ip %s\n", tmp_sa, tmp_da);
            }
        }
        
        /*_ptp_cli_interface_vlan_tagging_config_write*/
        if (0 != p_db_ptp_port->vlan_info.vid)
        {
            sal_fprintf(fp, INTEND_1_STR"ptp tagging vlan %u", p_db_ptp_port->vlan_info.vid);

            if (0 != p_db_ptp_port->vlan_info.cos)
            {
                sal_fprintf(fp, " cos %u", p_db_ptp_port->vlan_info.cos);
            }

            sal_fprintf(fp, INTEND_1_STR"\n");
        }
        
        /*ptp enable on interface*/
        if (p_db_ptp_port->enable)
        {
            sal_fprintf(fp, INTEND_1_STR"ptp enable\n");
        }
    }
    
    return;
}

int32
ccs_cfg_build_interface_one_ipmc_port(FILE* fp, tbl_interface_t *p_db_if)
{
    tbl_ipmc_intf_key_t    ipmc_intf_key;
    tbl_ipmc_intf_t       *p_db_ipmc_intf = NULL;

    sal_memset(&ipmc_intf_key, 0x0, sizeof(ipmc_intf_key));
    sal_strcpy(ipmc_intf_key.name, p_db_if->key.name);
    p_db_ipmc_intf = tbl_ipmc_intf_get_ipmc_intf(&ipmc_intf_key);

    if (!p_db_ipmc_intf)
    {
        return PM_E_NONE;
    }

    if (p_db_ipmc_intf->igmp_enable)
    {
        sal_fprintf(fp, INTEND_1_STR"ip igmp\n");
    }

    if (p_db_ipmc_intf->pim_enable)
    {
        sal_fprintf(fp, INTEND_1_STR"ip pim sm\n");
    }

    if (!p_db_ipmc_intf->igmp_enable && !p_db_ipmc_intf->pim_enable)
    {
        return PM_E_NONE;
    }

    if (0 != p_db_ipmc_intf->igmp_version && IGMP_DEFAULT_VERSION != p_db_ipmc_intf->igmp_version)
    {
        sal_fprintf(fp, INTEND_1_STR"ip igmp version %d\n", p_db_ipmc_intf->igmp_version);
    }

    if (0 != p_db_ipmc_intf->igmp_query_interval && IGMP_GENERAL_QUERY_INTERVAL != p_db_ipmc_intf->igmp_query_interval)
    {
        sal_fprintf(fp, INTEND_1_STR"ip igmp query-interval %d\n", p_db_ipmc_intf->igmp_query_interval);
    }

    if (0 != p_db_ipmc_intf->igmp_query_max_response_time && IGMP_QUERY_MAX_RESPONSE_TIME != p_db_ipmc_intf->igmp_query_max_response_time)
    {
        sal_fprintf(fp, INTEND_1_STR"ip igmp query-max-response-time %d\n", p_db_ipmc_intf->igmp_query_max_response_time);
    }

    if (0 != p_db_ipmc_intf->pim_hello_period && PIM_DEFAULT_HELLO_PERIOD != p_db_ipmc_intf->pim_hello_period)
    {
        sal_fprintf(fp, INTEND_1_STR"ip pim hello-interval %d\n", p_db_ipmc_intf->pim_hello_period);
    }

    if (0 != p_db_ipmc_intf->pim_hello_holdtime &&  (int32) (3.5 * p_db_ipmc_intf->pim_hello_period) != p_db_ipmc_intf->pim_hello_holdtime 
              && PIM_DEFAULT_HELLO_HOLDTIME != p_db_ipmc_intf->pim_hello_holdtime)
    {
        sal_fprintf(fp, INTEND_1_STR"ip pim hello-holdtime %d\n", p_db_ipmc_intf->pim_hello_holdtime);
    }

    if (0 != p_db_ipmc_intf->pim_dr_priority && PIM_DEFAULT_DR_PRIORITY != p_db_ipmc_intf->pim_dr_priority)
    {
        sal_fprintf(fp, INTEND_1_STR"ip pim drpriority %d\n", p_db_ipmc_intf->pim_dr_priority);
    }

    return PM_E_NONE;
}

char*
ccs_get_hash_value_profile_name_by_profile_id(uint32 profile_id)
{
    tbl_hash_value_profile_master_t *p_master_value_pro = tbl_hash_value_profile_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_hash_value_profile_t* p_hash_value_pro = NULL;

    CTCLIB_SLIST_LOOP(p_master_value_pro->hash_value_profile_list, p_hash_value_pro, listnode)
    {
        if (p_hash_value_pro && p_hash_value_pro->index == profile_id)
        {
            return p_hash_value_pro->key.name;
        }
    }
    return NULL;
}

static char*
ccs_l2protcol_get_protocol_action(uint32 proto_action)
{
    switch (proto_action)
    {
    case GLB_L2_PROTO_TUNNEL:
        return "tunnel";
    case GLB_L2_PROTO_PEER:
        return "peer";
    case GLB_L2_PROTO_DISCARD:
        return "discard";
    default:
        return "invalid";
    }
}

int32
ccs_cfg_build_interface_one_l2protocol_port(FILE* fp, tbl_interface_t *p_db_if)
{
    ds_l2protocol_t* p_ds_l2protocol = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    if (GLB_IF_MODE_L2 != p_db_if->mode)
    {
        return PM_E_NONE;
    }
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && 
        GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        return PM_E_NONE;
    }
    CTCLIB_SLIST_LOOP(p_db_if->l2protocol.obj_list, p_ds_l2protocol, listnode)
    {
        sal_fprintf(fp, INTEND_1_STR"l2protocol %s enable\n", p_ds_l2protocol->key.l2pro_name);
        if (p_ds_l2protocol->l2pro_vlan)
        {
            sal_fprintf(fp, INTEND_1_STR"l2protocol %s protocol-vlan %u\n", p_ds_l2protocol->key.l2pro_name, p_ds_l2protocol->l2pro_vlan);
        }
        else if (sal_strlen(p_ds_l2protocol->l2pro_evc_name))
        {
            sal_fprintf(fp, INTEND_1_STR"l2protocol %s protocol-vlan evc %s\n", p_ds_l2protocol->key.l2pro_name, p_ds_l2protocol->l2pro_evc_name);
        }
        if (p_ds_l2protocol->l2pro_action != GLB_L2_PROTO_PEER)
        {
            sal_fprintf(fp, INTEND_1_STR"l2protocol %s protocol-action %s\n", 
                p_ds_l2protocol->key.l2pro_name,
                ccs_l2protcol_get_protocol_action(p_ds_l2protocol->l2pro_action));
        }
    }
    if (p_db_if->l2pro_uplink)
    {
        sal_fprintf(fp, INTEND_1_STR"l2protocol uplink enable\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_interface_one_rmon_port(FILE* fp, tbl_interface_t *p_db_if)
{
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_rmon_ether_stats_t *p_db_stats = NULL;
    tbl_rmon_ether_stats_master_t *p_stats_master = tbl_rmon_ether_stats_get_master();
    tbl_rmon_history_t *p_db_his = NULL;
    tbl_rmon_history_master_t *p_his_master = tbl_rmon_history_get_master();

    if (NULL == p_stats_master || NULL == p_his_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_stats_master->rmon_ether_stats_list, p_db_stats, listnode, next)
    {
        if (p_db_stats->data_source_ifindex == p_db_if->ifindex)
        {
            if (sal_strcmp(p_db_stats->owner, RMON_DEFAULT_NAME))
            {
                sal_fprintf(fp, INTEND_1_STR"rmon collection stats %d owner %s\n", p_db_stats->key.index, p_db_stats->owner);
            }
            else
            {
                sal_fprintf(fp, INTEND_1_STR"rmon collection stats %d\n", p_db_stats->key.index);
            }
            break;
        }
    }

    CTCLIB_SLIST_LOOP_DEL(p_his_master->rmon_history_list, p_db_his, listnode, next)
    {
        if (p_db_his->data_source == p_db_if->ifindex)
        {
            sal_fprintf(fp, INTEND_1_STR"rmon collection history %d", p_db_his->key.index);

            if (p_db_his->buckets_requested != RMON_HISTORY_CTRL_BR_DEFAULT)
            {
                sal_fprintf(fp, " buckets %d", p_db_his->buckets_requested);
            }

            if (p_db_his->interval != RMON_HISTORY_CTRL_SAMPLE_INTERVAL_DEFAULT)
            {
                sal_fprintf(fp, " interval %d", p_db_his->interval);
            }
            
            if (sal_strcmp(p_db_his->owner, RMON_DEFAULT_NAME))
            {
                sal_fprintf(fp, " owner %s", p_db_his->owner);
            }
            sal_fprintf(fp, "\n");
            break;
        }
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_interface_cfm_mep(FILE* fp, tbl_interface_t *p_db_if)
{
    tbl_cfm_lmep_master_t   *p_db_glb = NULL;
    tbl_cfm_lmep_t          *p_lmep = NULL;
    ctclib_slistnode_t        *listnode = NULL;

    p_db_glb = tbl_cfm_lmep_get_master();
    if (p_db_glb)
    {
        CTCLIB_SLIST_LOOP(p_db_glb->cfm_lmep_list, p_lmep, listnode)
        {
            if (p_lmep->key.ifindex == p_db_if->ifindex)
            {
                if (p_lmep->key.vlan_id)
                {
                    sal_fprintf(fp, INTEND_1_STR"ethernet cfm mep %s mpid %d domain %s vlan %d interval %d\n",
                        p_lmep->direction ? "up" : "down", p_lmep->mep_id, p_lmep->md_name, p_lmep->key.vlan_id, p_lmep->cci_interval);
                }
                else
                {
                    sal_fprintf(fp, INTEND_1_STR"ethernet cfm mep %s mpid %d domain %s interval %d\n",
                        p_lmep->direction ? "up" : "down", p_lmep->mep_id, p_lmep->md_name, p_lmep->cci_interval);                    
                }
            }
        }
    }    

    return PM_E_NONE;
}

int32
ccs_cfg_build_interface_cfm_rmep(FILE* fp, tbl_interface_t *p_db_if)
{
    tbl_cfm_rmep_master_t   *p_db_glb = NULL;
    tbl_cfm_rmep_t               *p_rmep = NULL;
    ctclib_slistnode_t             *listnode = NULL;

    p_db_glb = tbl_cfm_rmep_get_master();
    if (p_db_glb)
    {
        CTCLIB_SLIST_LOOP(p_db_glb->cfm_rmep_list, p_rmep, listnode)
        {
            if (p_rmep->ifindex == p_db_if->ifindex)
            {
                if (p_rmep->key.vlan_id)
                {
                    sal_fprintf(fp, INTEND_1_STR"ethernet cfm mep crosscheck mpid %d domain %s vlan %d mac %02x%02x.%02x%02x.%02x%02x\n",
                        p_rmep->key.rmep_id, p_rmep->md_name, p_rmep->key.vlan_id, p_rmep->mac_addr[0], p_rmep->mac_addr[1], 
                        p_rmep->mac_addr[2], p_rmep->mac_addr[3], p_rmep->mac_addr[4], p_rmep->mac_addr[5]);
                }
                else
                {
                     sal_fprintf(fp, INTEND_1_STR"ethernet cfm mep crosscheck mpid %d domain %s mac %02x%02x.%02x%02x.%02x%02x\n",
                        p_rmep->key.rmep_id, p_rmep->md_name, p_rmep->mac_addr[0], p_rmep->mac_addr[1], 
                        p_rmep->mac_addr[2], p_rmep->mac_addr[3], p_rmep->mac_addr[4], p_rmep->mac_addr[5]);                   
                }
            }
        }
    }    

    return PM_E_NONE;
}

int32
ccs_cfg_build_interface_one(FILE* fp, tbl_interface_t *p_db_if)
{
#ifndef TAPPRODUCT
    ds_circuit_id_t *p_db_circuit = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_mstp_port_t *p_db_mstp_port = NULL;
    tbl_route_if_t *p_db_rtif = NULL;
    tbl_route_if_key_t rt_if_key;
    tbl_lldp_if_t *p_db_lldp_if = NULL;
#endif
    ds_brgif_t *p_db_brgif = NULL;
    ds_dhclient_if_t *p_db_dhclient = NULL;
    tbl_lldp_if_t lldp_if;
    tbl_aclqos_if_t * p_aclqos_if =NULL;
    uint32 queue_index = 0;
    uint32 has_config = FALSE;
    char ifname_ext[IFNAME_EXT_SIZE];
    char dhclient_ext[IFNAME_EXT_SIZE];
    char buf[CMD_BUF_256];
    char buf2[CMD_BUF_256];
    tbl_dot1x_port_t*    p_dot1x_port = NULL;
    tbl_openflow_interface_t *p_openflow_db_if = NULL;
    tbl_openflow_interface_key_t openflow_if_key;
    tbl_openflow_tunnel_interface_t *p_openflow_db_tunnel_if = NULL;
    tbl_openflow_tunnel_interface_key_t openflow_tunnel_if_key;
    tbl_port_stats_rate_key_t   port_stats_rate_key;
    tbl_port_stats_rate_t      *p_db_stats_rate;
    uint32     ifindex = 0;
    char bind_ifname_ext[IFNAME_EXT_SIZE];
    uint32 index =0;
    uint32 day =0, hour = 0, min = 0;
    uint32 is_usw = 0;
    uint32 fec_support = 0;
    char *hash_value_name = NULL;
	char auth_key[256];
	char rip_auth_string[STRING_SIZE];
#ifdef USW
    is_usw = 1;
#else
    is_usw = 0;
#endif
    
#ifndef TAPPRODUCT
    char nd_line[256];
    char nd_tmp[256];
#endif

    if (GLB_IF_TYPE_CPU_IF == p_db_if->hw_type)
    {
        return PM_E_NONE;
    }
    
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    sal_fprintf(fp, "interface %s\n", ifname_ext);

    sal_memset(&lldp_if, 0, sizeof(lldp_if));    

    if (sal_strlen(p_db_if->desc))
    {
        sal_fprintf(fp, INTEND_1_STR"description %s\n", p_db_if->desc);
    }

    if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        /* for bug 53128 by tongzb 2019-07-31*/
        if(GLB_FLAG_ISSET(p_db_if->phyport_flag, GLB_PHYPORT_FLAG_COMBO))
        {
            if(p_db_if->media== GLB_PORT_TYPE_COPPER)
            {
                sal_fprintf(fp, INTEND_1_STR"media-type rj45\n");
            }
            else if(p_db_if->media == GLB_PORT_TYPE_FIBER)
            {
                sal_fprintf(fp, INTEND_1_STR"media-type sfp\n");
            }
        }        
        if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100G))
        {
            if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100G) 
                && (GLB_SPEED_100G == p_db_if->speed))
            {
                sal_fprintf(fp, INTEND_1_STR"speed 100G\n");
            }
            else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_40G) 
                && (GLB_SPEED_40G == p_db_if->speed))
            {
                sal_fprintf(fp, INTEND_1_STR"speed 40G\n");
            }
            if(GLB_DUPLEX_AUTO == p_db_if->duplex)
            {
                sal_fprintf(fp, INTEND_1_STR"duplex auto\n");
            }
        }
        else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_40G))
        {
            if(GLB_DUPLEX_AUTO == p_db_if->duplex)
            {
                sal_fprintf(fp, INTEND_1_STR"duplex auto\n");
            }
        }
        else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_50G))
        {
            if(GLB_DUPLEX_FULL == p_db_if->duplex)
            {
                sal_fprintf(fp, INTEND_1_STR"duplex full\n");
            }
        }
        else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_25G))
        {
            if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_25G) 
                && (GLB_SPEED_25G == p_db_if->speed))
            {
                sal_fprintf(fp, INTEND_1_STR"speed 25G\n");
            }
            else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_10G) 
                && (GLB_SPEED_10G == p_db_if->speed))
            {
                sal_fprintf(fp, INTEND_1_STR"speed 10G\n");
            }
            if(GLB_DUPLEX_FULL == p_db_if->duplex)
            {
                sal_fprintf(fp, INTEND_1_STR"duplex full\n");
            }
        }
        else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_10G))
        {
            if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_10G) 
                && (GLB_SPEED_10G == p_db_if->speed))
            {
                sal_fprintf(fp, INTEND_1_STR"speed 10G\n");
            }
            else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_1G) 
                && (GLB_SPEED_1G == p_db_if->speed))
            {
                sal_fprintf(fp, INTEND_1_STR"speed 1000\n");
            }
            if(GLB_DUPLEX_FULL == p_db_if->duplex)
            {
                sal_fprintf(fp, INTEND_1_STR"duplex full\n");
            }
        }
        else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_1G)
            || GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_2_5G)
            || GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_5G))
        {
            if (GLB_SPEED_AUTO != p_db_if->speed)
            {
                sal_fprintf(fp, INTEND_1_STR"speed %s\n", 
                    cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_db_if->speed));
            }

            if (GLB_DUPLEX_AUTO != p_db_if->duplex)
            {
                sal_fprintf(fp, INTEND_1_STR"duplex %s\n", 
                    cdb_enum_val2str(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, p_db_if->duplex));
            }
        }
        if (p_db_if->carrier_down_holdtime_interval)
        {
            sal_fprintf(fp, INTEND_1_STR"carrier down-hold-time interval %u\n", p_db_if->carrier_down_holdtime_interval);
        }
        if (p_db_if->latency_event_enable)
        {
            sal_fprintf(fp, INTEND_1_STR"hardware monitor latency event enable\n");
        }
        if ((p_db_if->latency_event_high !=  LATENCY_EVENT_HIGH_THRESHOLD_DEFAULT)
            || (p_db_if->latency_event_low !=  LATENCY_EVENT_LOW_THRESHOLD_DEFAULT))
        {
            sal_fprintf(fp, INTEND_1_STR"hardware monitor latency event threshold low %u high %u\n", p_db_if->latency_event_low, p_db_if->latency_event_high);   
        }
        /* poe configure */
        if (DEFAULT_POE_ENALBE != p_db_if->poe.enable)
        {
            sal_fprintf(fp, INTEND_1_STR"poe disable\n");
        }
        if (p_db_if->poe.force_power)
        {
            sal_fprintf(fp, INTEND_1_STR"poe force-power\n");
        }
        if (p_db_if->poe.mode != 1)
        {
            sal_fprintf(fp, INTEND_1_STR"poe mode %s\n",p_db_if->poe.mode==0?"af":(p_db_if->poe.mode==2?"poh":""));
        }
        if (p_db_if->poe.priority != 3)
        {
            sal_fprintf(fp, INTEND_1_STR"poe priority %s\n",p_db_if->poe.priority==1?"critical":(p_db_if->poe.priority==2?"high":""));
        }
        if (p_db_if->poe.power_limit != 0)
        {
            sal_fprintf(fp, INTEND_1_STR"poe max-power %d\n",p_db_if->poe.power_limit);
        }

    }
    else
    {
    }

    p_aclqos_if = tbl_aclqos_if_get_aclqos_if_by_name(p_db_if->key.name);
    if(p_aclqos_if)
    {
        if (GLB_IF_TYPE_VLAN_IF == p_aclqos_if->port_type)
        {
             if (p_aclqos_if->dscp_domain)
             {
                sal_fprintf(fp, INTEND_1_STR"qos dscp-domain %d\n", p_aclqos_if->dscp_domain);
             }

            if(GLB_QOS_TRUST_DSCP == p_aclqos_if->trust)
            {
                if (is_usw)
                {
                    sal_fprintf(fp, INTEND_1_STR"trust dscp\n");
                }
                else
                {
                    sal_fprintf(fp, INTEND_1_STR"trust dscp-exp\n");
                }
            }
        }
        else
        {
             if(GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_DOMAIN))
             {
                sal_fprintf(fp, INTEND_1_STR"qos domain %d\n", p_aclqos_if->domain);
             }

             if (p_aclqos_if->cos_domain)
             {
                sal_fprintf(fp, INTEND_1_STR"qos cos-domain %d\n", p_aclqos_if->cos_domain);
             }

             if (p_aclqos_if->dscp_domain)
             {
                sal_fprintf(fp, INTEND_1_STR"qos dscp-domain %d\n", p_aclqos_if->dscp_domain);
             }
             
             if(p_aclqos_if->port_tc != 0)
             {
                sal_fprintf(fp, INTEND_1_STR"set cos %d\n",p_aclqos_if->port_tc);
             }

            if(tbl_qos_global_get_qos_enable())
            {
                if(GLB_QOS_TRUST_PORT == p_aclqos_if->trust)
                {
                    sal_fprintf(fp, INTEND_1_STR"trust port\n");
                }
                else if(GLB_QOS_TRUST_DSCP == p_aclqos_if->trust)
                {
                    if (is_usw)
                    {
                        sal_fprintf(fp, INTEND_1_STR"trust dscp\n");
                    }
                    else
                    {
                        sal_fprintf(fp, INTEND_1_STR"trust dscp-exp\n");
                    }
                }
               
                if(p_aclqos_if->replace_dscp)
                {
                    sal_fprintf(fp, INTEND_1_STR"replace dscp\n");
                }
                if(!p_aclqos_if->replace_cos)
                {
                    sal_fprintf(fp, INTEND_1_STR"no replace cos\n");
                }
            }

            if(GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_PORT_SHAPE_PROFILE))
            {
                sal_fprintf(fp, INTEND_1_STR"qos port-shape-profile %s\n", p_aclqos_if->port_shape_profile);
            }

            if(GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_INPUT_POLICER))
            {
                sal_fprintf(fp, INTEND_1_STR"qos port-policer input %s\n", p_aclqos_if->input_policer);
            }

            if(GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_OUTPUT_POLICER))
            {
                sal_fprintf(fp, INTEND_1_STR"qos port-policer output %s\n", p_aclqos_if->output_policer);
            }

            for(queue_index = 0; queue_index < GLB_QOS_PORT_QUEUE_NUM; queue_index++)
            {
                if((0 != sal_strcmp(p_aclqos_if->queue_drop_profile[queue_index], GLB_QOS_DEF_WTD_PROFILE)) && (0 != sal_strcmp(p_aclqos_if->queue_drop_profile[queue_index], GLB_QOS_DEF_WRED_PROFILE)))
                {
                    if(!has_config)
                    {
                        sal_fprintf(fp, INTEND_1_STR"qos queue %d\n", queue_index);
                        has_config = TRUE;
                    }
                    sal_fprintf(fp, INTEND_1_STR INTEND_1_STR"drop-profile %s\n", p_aclqos_if->queue_drop_profile[queue_index]);
                }
                if((0 != sal_strcmp(p_aclqos_if->queue_shape_profile[queue_index], GLB_QOS_DEF_SCHEDULER_PROFILE)))
                {
                    if(!has_config)
                    {
                        sal_fprintf(fp, INTEND_1_STR"qos queue %d\n", queue_index);
                        has_config = TRUE;
                    }
                    sal_fprintf(fp, INTEND_1_STR INTEND_1_STR"scheduler-profile %s\n", p_aclqos_if->queue_shape_profile[queue_index]);
                }
                if(GLB_QOS_QUEUE_DROP_MODE_WTD != p_aclqos_if->queue_drop_mode[queue_index])
                {
                    if(!has_config)
                    {
                        sal_fprintf(fp, INTEND_1_STR"qos queue %d\n", queue_index);
                        has_config = TRUE;
                    }
                    sal_fprintf(fp, INTEND_1_STR INTEND_1_STR"random-detect enable\n");
                }
                has_config = FALSE;
            }
        }
    }

    if (GLB_PORT_UNIDIR_ENABLE == p_db_if->unidir)
    {
        sal_fprintf(fp, INTEND_1_STR"unidirectional enable\n");
    }

    if ((p_db_if->hw_type == GLB_IF_TYPE_PORT_IF)&&(p_db_if->fec&GLB_FEC_DEFAULT_MASK) != GLB_FEC_DEFAULT)
    {
        if ((GLB_CHIP_GREATEBELT == tbl_chip_get_type())
          ||(GLB_CHIP_GOLDENGATE == tbl_chip_get_type()))
        {
          if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100G))
          {
            fec_support = GLB_GG_FEC_SUPPORT;
          }
        }
        else if(GLB_CHIP_DUET2 == tbl_chip_get_type())
        {
          if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100G)
              ||GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_25G)
              ||GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_50G))
          {
            fec_support = GLB_D2_FEC_SUPPORT;
          }
        }
        else if(GLB_CHIP_TSINGMA == tbl_chip_get_type())
        {
          if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100G)
              ||GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_25G)
              ||GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_50G)
              ||GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_40G)
              ||GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_10G))
          {
            fec_support = GLB_D2_FEC_SUPPORT;
          }
        }
        if(fec_support == GLB_D2_FEC_SUPPORT)/* D2/TM */
        {
          if((p_db_if->fec&GLB_FEC_MASK) == GLB_FEC_RS)
          {
              sal_fprintf(fp, INTEND_1_STR"fec rs\n");
          }
          else if((p_db_if->fec&GLB_FEC_MASK) == GLB_FEC_BASE_R)
          {
              sal_fprintf(fp, INTEND_1_STR"fec baser\n");
          }
          else if((p_db_if->fec&GLB_FEC_MASK) == GLB_FEC_OFF)
          {
              sal_fprintf(fp, INTEND_1_STR"fec none\n");
          } 
        }
        else if(fec_support == GLB_GG_FEC_SUPPORT)/* GG */
        {
            /* for bug 51432, gg just support fec 0/1 */
            if(p_db_if->fec)
            {
                sal_fprintf(fp, INTEND_1_STR"fec enable\n");
            }
        }
    }

    if (GLB_DEFAULT_TPID != p_db_if->svlan_tpid && 
       (p_db_if->hw_type == GLB_IF_TYPE_PORT_IF || p_db_if->hw_type == GLB_IF_TYPE_LINKAGG_IF))
    {
#ifdef TAPPRODUCT
        if (0 != p_db_if->svlan_tpid)
        {
            sal_fprintf(fp, INTEND_1_STR"svlan-tpid 0x%x\n", p_db_if->svlan_tpid);
        }
#else
        sal_fprintf(fp, INTEND_1_STR"dot1q ethertype %x\n", p_db_if->svlan_tpid);
#endif
    }

    if (1 == p_db_if->reflective_bridge_en)
    {
        sal_fprintf(fp, INTEND_1_STR"reflective-bridge enable\n");
    }

    if (GLB_FLAG_ISSET(p_db_if->flowcontrol, GLB_IF_FLOW_CONTROL_SEND))
    {
        sal_fprintf(fp, INTEND_1_STR"flowcontrol send on\n");
    }
    else if (GLB_PORT_UNIDIR_RX_ONLY == p_db_if->unidir)
    {
        sal_fprintf(fp, INTEND_1_STR"unidirectional rx-only\n");
    }
    else if (GLB_PORT_UNIDIR_TX_ONLY == p_db_if->unidir)
    {
        sal_fprintf(fp, INTEND_1_STR"unidirectional tx-only\n");
    }

    if (GLB_FLAG_ISSET(p_db_if->flowcontrol, GLB_IF_FLOW_CONTROL_RECEIVE))
    {
        sal_fprintf(fp, INTEND_1_STR"flowcontrol receive on\n");
    }

    if (!p_db_if->ifg_stats_en && (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type || GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type))
    {
        sal_fprintf(fp, INTEND_1_STR"flow-statistics include-interframe disable\n");
    }

    if (p_db_if->pfc_admin)
    {
        int i = 0;
        char pri_str[9]; 
        sal_memset(pri_str, 0, sizeof(pri_str)); 
        
        sal_fprintf(fp, INTEND_1_STR"priority-flow-control mode on\n");
        for (i = 0; i < 8; i++)
        {
            if(GLB_IS_BIT_SET(p_db_if->pfc_priority, i))
            {
                sal_sprintf (pri_str, "%d ", i);
                sal_fprintf(fp, INTEND_1_STR"priority-flow-control enable priority %s\n", pri_str);
            }   
        }
    }
    
    if (p_db_if->ntp_disable)
    {
        sal_fprintf(fp, INTEND_1_STR"ntp disable\n");
    }

    if (p_db_if->ntp_broadcast_client)
    {
        sal_fprintf(fp, INTEND_1_STR"ntp broadcast client\n");
    }
    
#ifndef TAPPRODUCT
    #ifdef DUET2
        if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type || GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
        {
            if(!p_db_if->jumboframe_en)
            {
                sal_fprintf(fp, INTEND_1_STR"no jumboframe enable\n");
            }
        }
        else
        {
            if(p_db_if->jumboframe_en)
            {
                sal_fprintf(fp, INTEND_1_STR"jumboframe enable\n");
            }
        }
    #else
        if (p_db_if->jumboframe_en)
        {
            sal_fprintf(fp, INTEND_1_STR"jumboframe enable\n");
        }
    #endif
#endif

    if (p_db_if->isolate_group_id)
    {
        sal_fprintf(fp, INTEND_1_STR"port-isolate group %u\n", p_db_if->isolate_group_id);
    }

    if (p_db_if->bandwidth)
    {
        sal_fprintf(fp, INTEND_1_STR"bandwidth %u\n", p_db_if->bandwidth);
    }

    if (GLB_IF_MODE_L3 == p_db_if->mode)
    {
        if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type) 
        {
            sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" switchport\n");
        }
    }

    /*added by yejl for tsingma hash*/
    if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        if (p_db_if->hash_value_profile_id)
        {
            hash_value_name = ccs_get_hash_value_profile_name_by_profile_id(p_db_if->hash_value_profile_id);
            if (hash_value_name)
            {
                sal_fprintf(fp, " load-balance hash-value %s input\n", hash_value_name);    
            }
        }
    }
    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        if (p_db_if->hash_value_profile_id)
        {
            hash_value_name = ccs_get_hash_value_profile_name_by_profile_id(p_db_if->hash_value_profile_id);
            if (hash_value_name)
            {
                sal_fprintf(fp, " load-balance hash-value %s input\n", hash_value_name);    
            }
        }
        if (p_db_if->hash_value_profile_id_agg)
        {
            hash_value_name = ccs_get_hash_value_profile_name_by_profile_id(p_db_if->hash_value_profile_id_agg);
            if (hash_value_name)
            {
                sal_fprintf(fp, " load-balance hash-value %s output\n", hash_value_name);
            }
        }
    }
    /*end*/
    if (p_db_if->dot1x_enable)
    {
        p_dot1x_port = tbl_dot1x_port_get_dot1x_port((tbl_dot1x_port_key_t*)&p_db_if->ifindex);
        if(p_dot1x_port)
        {    
            if (AUTH_PORT_CTRL_AUTO == p_dot1x_port->authControlledPortControl)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x port-control auto\n");
            }
            else if (AUTH_PORT_CTRL_FORCE_AUTHORIZED == p_dot1x_port->authControlledPortControl)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x port-control force-authorized\n");
            }
            else if (AUTH_PORT_CTRL_FORCE_UNAUTHORIZED == p_dot1x_port->authControlledPortControl)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x port-control force-unauthorized\n");
            }

            if (!p_dot1x_port->auth_ctrl_dir_adminControlledDirections)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x port-control dir both\n");
            } 

            if (AUTH_EAPOL_PROTOCOL_VERSION_2 != p_dot1x_port->protocol_version)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x protocol-version %d\n", p_dot1x_port->protocol_version);                               
            }

            if (DOT1X_DEFAULT_PAE_QUIETPERIOD != p_dot1x_port->auth_pae_quietPeriod)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x timeout quiet-period %d\n", p_dot1x_port->auth_pae_quietPeriod);
            }

            if (DOT1X_DEFAULT_PAE_REAUTHMAX != p_dot1x_port->auth_pae_reAuthMax)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x max-req %d\n", p_dot1x_port->auth_pae_reAuthMax);               
            }

            if (p_dot1x_port->guest_vlan)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x guest-vlan %d\n", p_dot1x_port->guest_vlan); 
            }

            if (DOT1X_DEFAULT_PAE_TXPERIOD != p_dot1x_port->auth_pae_txPeriod)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x timeout tx-period %d\n", p_dot1x_port->auth_pae_txPeriod);
            }

            if (p_dot1x_port->reAuthEnabled)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x reauthentication\n");                               
            }

            if (DOT1X_DEFAULT_REAUTH_PERIOD != p_dot1x_port->reAuthPeriod)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x timeout re-authperiod %d\n", p_dot1x_port->reAuthPeriod);
            }

            if (DOT1X_DEFAULT_BE_SUPP_TIMEOUT != p_dot1x_port->auth_be_suppTimeout)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x timeout supp-timeout %d\n", p_dot1x_port->auth_be_suppTimeout);    
            }

            if (DOT1X_DEFAULT_BE_SERVER_TIMEOUT != p_dot1x_port->auth_be_serverTimeout)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x timeout server-timeout %d\n", p_dot1x_port->auth_be_serverTimeout);    
            }

            if (AUTH_PORT_MODE_PORT != p_dot1x_port->auth_mode)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x port-mode mac\n");    
            }

            if (p_dot1x_port->mac_auth_bypass)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x mac-auth-bypass\n");   
            }

            if (p_dot1x_port->max_user)
            {
                sal_fprintf(fp, INTEND_1_STR"dot1x max-user %d\n", p_dot1x_port->max_user); 
            }
        }
    }

    p_db_dhclient = ds_dhclient_if_get_dhclient_if(p_db_if);
    if (NULL != p_db_dhclient)
    {
        if ((p_db_dhclient->requested_flags != M_DHC_REQ_DEFAULT)) 
        {
            M_DHC_REQ_OPTION_ARR(req_arr);
            int flags;
            dhc_req_option_t *ro;

            flags = (p_db_dhclient->requested_flags & (~(M_DHC_REQ_DEFAULT | M_DHC_REQ_MUST)));
            ro = req_arr;
            while (NULL != ro->name)
            {
                if (ro->flag & flags)
                {
                    sal_fprintf(fp, INTEND_1_STR"dhcp client request %s\n", ro->name);
                }
                ro++;
            }
            
            flags = (p_db_dhclient->requested_flags & M_DHC_REQ_DEFAULT);
            flags |= M_DHC_REQ_MUST;
            ro = req_arr;
            while (NULL != ro->name)
            {
                if ((ro->flag & M_DHC_REQ_DEFAULT) && !(ro->flag & flags))
                {
                    sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" dhcp client request %s\n", ro->name);
                }
                ro++;
            }

        }
        if (1200 != p_db_dhclient->lease)
        {
            day = p_db_dhclient->lease / 86400;
            hour = (p_db_dhclient->lease % 86400) / 3600;
            min = ((p_db_dhclient->lease % 86400) % 3600) / 60;

            if (!min && !hour && !day)
            {
                sal_fprintf(fp, INTEND_1_STR"dhcp client lease infinite\n");
            }
            else if (!min && !hour)
            {
                sal_fprintf(fp, INTEND_1_STR"dhcp client lease %d 0 0\n" , day);
            }
            else if (!min)
            {
                sal_fprintf(fp, INTEND_1_STR"dhcp client lease %d %d 0\n" , day, hour);
            }
            else
            {
                sal_fprintf(fp, INTEND_1_STR"dhcp client lease %d %d %d\n" , day, hour, min);
            }
        }
        if ((0 != sal_strlen(p_db_dhclient->client_id)))
        {
            if (p_db_dhclient->client_flags == GLB_DHCLIENT_FLAG_HEX)
            {
                sal_fprintf(fp, INTEND_1_STR"dhcp client client-id hex %s\n", p_db_dhclient->client_id);
            }
            else if (p_db_dhclient->client_flags == GLB_DHCLIENT_FLAG_ASCII)
            {
                sal_fprintf(fp, INTEND_1_STR"dhcp client client-id ascii %s\n", p_db_dhclient->client_id);
            }
            else if (p_db_dhclient->client_flags == GLB_DHCLIENT_FLAG_PORT)
            {
                IFNAME_ETH2FULL(p_db_dhclient->client_id, dhclient_ext);
                sal_fprintf(fp, INTEND_1_STR"dhcp client client-id %s\n", dhclient_ext);
            }
        }

        if ((0 != sal_strlen(p_db_dhclient->class_id)))
        {
            if (p_db_dhclient->class_flags == GLB_DHCLIENT_FLAG_HEX)
            {
                sal_fprintf(fp, INTEND_1_STR"dhcp client class-id hex %s\n", p_db_dhclient->class_id);
            }
            else if (p_db_dhclient->class_flags == GLB_DHCLIENT_FLAG_ASCII)
            {
                sal_fprintf(fp, INTEND_1_STR"dhcp client class-id %s\n", p_db_dhclient->class_id);
            }
        }
        if ((0 != sal_strlen(p_db_dhclient->hostname)))
        {
            sal_fprintf(fp, INTEND_1_STR"dhcp client hostname %s\n", p_db_dhclient->hostname);
        }

        /* modified by liwh for bug 50098, 2018-11-29 */
        if ((DHCPV6_CLIENT_DEFAULT_IANA_T1_TIME != p_db_dhclient->iana_t1)
            || (DHCPV6_CLIENT_DEFAULT_IANA_T2_TIME != p_db_dhclient->iana_t2))
        {
            day = p_db_dhclient->iana_t1 / 86400;
            hour = (p_db_dhclient->iana_t1 % 86400) / 3600;
            min = ((p_db_dhclient->iana_t1 % 86400) % 3600) / 60;

            if (!min && !hour)
            {
                sal_fprintf(fp, INTEND_1_STR"dhcpv6 client iana t1 %d 0 0" , day);
            }
            else if (!min)
            {
                sal_fprintf(fp, INTEND_1_STR"dhcpv6 client iana t1 %d %d 0" , day, hour);
            }
            else
            {
                sal_fprintf(fp, INTEND_1_STR"dhcpv6 client iana t1 %d %d %d" , day, hour, min);
            }

            day = p_db_dhclient->iana_t2 / 86400;
            hour = (p_db_dhclient->iana_t2 % 86400) / 3600;
            min = ((p_db_dhclient->iana_t2 % 86400) % 3600) / 60;

            if (!min && !hour)
            {
                sal_fprintf(fp, " t2 %d 0 0\n" , day);
            }
            else if (!min)
            {
                sal_fprintf(fp, " t2 %d %d 0\n" , day, hour);
            }
            else
            {
                sal_fprintf(fp, " t2 %d %d %d\n" , day, hour, min);
            }
        }
        /* liwh end */
    }

    if (GLB_IF_MODE_L2 == p_db_if->mode)
    {        
        if (IP_NONE != p_db_if->ip_source_type)
        {
            if (!(GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type && (p_db_if->lag || p_db_if->lacp)))
            {
                if (IP_FILTERING == p_db_if->ip_source_type)
                {
                    sal_fprintf(fp, INTEND_1_STR"ip verify source ip\n");
                }
                else if (IP_MAC_FILTERING == p_db_if->ip_source_type)
                {
                    sal_fprintf(fp, INTEND_1_STR"ip verify source ip-mac\n");
                }
                else if (IP_VLAN_FILTERING == p_db_if->ip_source_type)
                {
                    sal_fprintf(fp, INTEND_1_STR"ip verify source ip-vlan\n");
                }
                else if (IP_MAC_VLAN_FILTERING == p_db_if->ip_source_type)
                {
                    sal_fprintf(fp, INTEND_1_STR"ip verify source ip-mac-vlan\n");
                }
            }
        }        
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf2, 0, sizeof(buf2));

    if (GLB_IF_MODE_L3 == p_db_if->mode)
    {
        tbl_route_if_t *p_route_if = NULL;
        
        p_route_if = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
        if (NULL != p_route_if)
        {
            /*added by yejl to fix bug44333, 2017-06-13*/
            if (p_route_if->dhcp_client_enable)
            {
                sal_fprintf(fp, INTEND_1_STR"ip address dhcp\n");
            }
            /*ended by yejl*/

            /* modified by liwh for bug 49365, 2018-11-27 */
            if (p_route_if->dhcpv6_client_enable)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 address dhcp\n");
            }
            /* liwh end */
                        
            if (p_route_if->unicast_rpf_en)
            {
                sal_fprintf(fp, INTEND_1_STR"ip verify unicast reverse-path\n");
            }

            if (p_route_if->unicast_rpf_en_v6)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 verify unicast reverse-path\n");
            }
            
            if (!p_route_if->ip_redirects_en)
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" ip redirects\n");
            }

            if (!p_route_if->ip_unreachable_en)
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" ip unreachables\n");
            }

            if (p_route_if->dhcp_server_group)
            {
                sal_fprintf(fp, INTEND_1_STR"dhcp-server %u\n", p_route_if->dhcp_server_group);
            }

            if (p_route_if->dhcp_relay_option_trust)
            {
                sal_fprintf(fp, INTEND_1_STR"dhcp relay information trusted\n");
            }
        }

        /*ip arp number-limit*/
        /*modified by yejl to fix bug 44042, 2017-05-16*/
        if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type || 
            GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type || 
            GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type) 
        {
            if (p_db_if->arp_numberlimit_enable)
            {
                sal_fprintf(fp, INTEND_1_STR"ip arp number-limit enable\n");
            }
            if (p_db_if->arp_numberlimit_number != ARP_PORT_NUMBERLIMIT_NUMBER)
            {
                sal_fprintf(fp, INTEND_1_STR"ip arp number-limit maximum %d\n", p_db_if->arp_numberlimit_number);
            }
            if (p_db_if->arp_numberlimit_violate == ARP_PORT_NUMBERLIMIT_RESTRICT)
            {
                sal_fprintf(fp, INTEND_1_STR"ip arp number-limit violation restrict\n");
            }
            else if (p_db_if->arp_numberlimit_violate == ARP_PORT_NUMBERLIMIT_ERRDISABLE)
            {
                sal_fprintf(fp, INTEND_1_STR"ip arp number-limit violation errdisable\n");
            }
        }
        /*ended by yejl*/

        /*ip arp rate-limit*/
        if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type || 
            GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type || 
            GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type) 
        {
            if (NULL != p_route_if)
            {
                /*modified by chenc for bug 51010, 2019-02-14*/
                if (p_route_if->arp_rate_limit_en)
                {
                    sal_fprintf(fp, INTEND_1_STR "ip arp rate-limit enable\n");
                }
                /*ended chenc*/
                
                if (p_route_if->arp_rate_limit_pkt_max != ARP_RATE_LIMIT_DEFAULT_NUMBER)
                {
                    sal_fprintf(fp, INTEND_1_STR"ip arp rate-limit maximum %d\n", p_route_if->arp_rate_limit_pkt_max);
                }
                
                if (p_route_if->arp_rate_limit_violation == ARP_RATE_LIMIT_ACTION_ERRDISABLE)
                {
                    sal_fprintf(fp, INTEND_1_STR"ip arp rate-limit violation errdisable\n");
                }
            }
        }      
    }

    if (!p_db_if->admin_up)
    {
        sal_fprintf(fp, INTEND_1_STR"shutdown\n");
    }

    /* TODO TBL_INTERFACE_FLD_BANDWIDTH */

    if (GLB_IF_MODE_L2 == p_db_if->mode)
    {
        p_db_brgif = p_db_if->brgif;

        /* maybe mode is L2 but brgif is not synced and is NULL */
        if (p_db_brgif)
        {
#ifndef TAPPRODUCT
            if (p_db_if->mlag_id)
            {
                sal_fprintf(fp, INTEND_1_STR"mlag %u\n", p_db_if->mlag_id);
            }
            /* TODO DS_BRGIF_FLD_TPID_TYPE */
            
            if (0 == p_db_brgif->mac_learning_en)
            {
                sal_fprintf(fp, INTEND_1_STR"mac learning disable\n");
            }

            if (GLB_VLAN_PORT_TYPE_ACCESS == p_db_if->vlan_type)
            {
                if (GLB_IF_PVLAN_TYPE_NONE != p_db_if->pvlan_type)
                {
                    if (GLB_IF_PVLAN_TYPE_PROMISCUOUS != p_db_if->pvlan_type)
                    {
                        sal_fprintf(fp, INTEND_1_STR"switchport mode private-vlan host\n");
                    }
                    else
                    {
                        sal_fprintf(fp, INTEND_1_STR"switchport mode private-vlan promiscuous\n");
                    }  
                    
                    if (GLB_IF_PVLAN_TYPE_COMMUNITY == p_db_if->pvlan_type)
                    {
                         sal_fprintf(fp, INTEND_1_STR"switchport private-vlan %u community-vlan %u\n",
                            p_db_if->private_vlan, p_db_if->community_vlan);
                    }
                    else if (GLB_DEF_VLAN_ID != p_db_if->private_vlan)
                    {
                        if (GLB_IF_PVLAN_TYPE_ISOLATED == p_db_if->pvlan_type)
                        {
                             sal_fprintf(fp, INTEND_1_STR"switchport private-vlan %u isolate\n", p_db_if->private_vlan);
                        }
                        else
                        {
                             sal_fprintf(fp, INTEND_1_STR"switchport private-vlan %u\n", p_db_if->private_vlan);     
                        }              
                    }
                }
                else if (GLB_DEF_VLAN_ID != p_db_brgif->pvid)
                {
                    sal_fprintf(fp, INTEND_1_STR"switchport access vlan %u\n", p_db_brgif->pvid);
                }
                _ccs_cfg_build_brgif_allowed_vlan(fp, p_db_if, p_db_brgif, CCS_CFG_BRGIF_ALLOW_VLAN_ACCESS);
            }
            else if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
            {
                sal_fprintf(fp, INTEND_1_STR"switchport mode trunk\n");
                
                if (GLB_DEF_VLAN_ID != p_db_brgif->pvid)
                {
                    sal_fprintf(fp, INTEND_1_STR"switchport trunk native vlan %u\n", p_db_brgif->pvid);
                }

                if (TRUE != p_db_brgif->ingress_filter)
                {
                    sal_fprintf(fp, INTEND_1_STR"switchport trunk ingress-filter disable\n");
                }

                if (p_db_brgif->vlan_translation_en)
                {
                    sal_fprintf(fp, INTEND_1_STR"switchport trunk vlan-translation\n");
                }

                if (sal_strlen(p_db_brgif->vlan_mapping_table))
                {
                    sal_fprintf(fp, INTEND_1_STR"switchport trunk vlan-translation mapping table %s\n", p_db_brgif->vlan_mapping_table);
                }

                if (p_db_brgif->vlan_trans_default_fwd)
                {
                    sal_fprintf(fp, INTEND_1_STR"switchport trunk vlan-translation mapping miss-forward\n");
                }

                switch (p_db_brgif->allowed_vlan.action)
                {
                case DS_BRGIF_ALLOWED_VLAN_ACT_ALL:
                    sal_fprintf(fp, INTEND_1_STR"switchport trunk allowed vlan all\n");
                    break;

                case DS_BRGIF_ALLOWED_VLAN_ACT_NONE:
                    sal_fprintf(fp, INTEND_1_STR"switchport trunk allowed vlan none\n");
                    break;
                default:
                    _ccs_cfg_build_brgif_allowed_vlan(fp, p_db_if, p_db_brgif, CCS_CFG_BRGIF_ALLOW_VLAN_TRUNK);
                    break;
                }

                _ccs_cfg_build_brgif_untagged_vlan(fp, p_db_if, p_db_brgif, CCS_CFG_BRGIF_ALLOW_VLAN_TRUNK);  
            }
            else if (GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type)
            {
                sal_fprintf(fp, INTEND_1_STR"switchport mode dot1q-tunnel\n");

                if (GLB_VLAN_QINQ_TYPE_SELECTIVE == p_db_brgif->qinq_type)
                {
                    sal_fprintf(fp, INTEND_1_STR"switchport dot1q-tunnel type selective\n");
                }

                if (GLB_VLAN_NONE != p_db_brgif->inner_native_vid)
                {
                    sal_fprintf(fp, INTEND_1_STR"switchport dot1q-tunnel native inner-vlan %u\n", p_db_brgif->inner_native_vid);
                }

                if (GLB_VLAN_NONE != p_db_brgif->pvid && GLB_DEF_VLAN_ID != p_db_brgif->pvid)
                {
                    sal_fprintf(fp, INTEND_1_STR"switchport dot1q-tunnel native vlan %u\n", p_db_brgif->pvid);
                }
                if (sal_strlen(p_db_brgif->vlan_mapping_table))
                {
                    sal_fprintf(fp, INTEND_1_STR"switchport dot1q-tunnel vlan mapping table %s\n", p_db_brgif->vlan_mapping_table);
                }

                if (p_db_brgif->qinq_default_fwd)
                {
                    sal_fprintf(fp, INTEND_1_STR"switchport dot1q-tunnel vlan mapping miss-forward\n");
                }

                switch (p_db_brgif->allowed_vlan.action)
                {
                case DS_BRGIF_ALLOWED_VLAN_ACT_ALL:
                    sal_fprintf(fp, INTEND_1_STR"switchport dot1q-tunnel allowed vlan all\n");
                    break;

                case DS_BRGIF_ALLOWED_VLAN_ACT_NONE:
                    sal_fprintf(fp, INTEND_1_STR"switchport dot1q-tunnel allowed vlan none\n");
                    break;
                default:
                    _ccs_cfg_build_brgif_allowed_vlan(fp, p_db_if, p_db_brgif, CCS_CFG_BRGIF_ALLOW_VLAN_QINQ);
                    break;
                }
            }

            if (p_db_brgif->dot1q_tag_native_en)
            {
                sal_fprintf(fp, INTEND_1_STR"vlan dot1q tag native\n");
            }

            if (p_db_brgif->port_security_en)
            {
                sal_fprintf(fp, INTEND_1_STR"switchport port-security\n");
            }
            
            if (GLB_PORT_SECURITY_MAC_DEFAULT_MAXIMUM != p_db_brgif->max_mac)
            {
                sal_fprintf(fp, INTEND_1_STR"switchport port-security maximum %u\n", p_db_brgif->max_mac);
            }
            
            if (GLB_PORT_SECURITY_VIOLATION_MODE_RESTRICT == p_db_brgif->violate_mode)
            {
                sal_fprintf(fp, INTEND_1_STR"switchport port-security violation restrict\n");
            }
            else if (GLB_PORT_SECURITY_VIOLATION_MODE_SHUTDOWN == p_db_brgif->violate_mode)
            {
                sal_fprintf(fp, INTEND_1_STR"switchport port-security violation shutdown\n");
            }
#ifdef SHOWRUN_TODO
            security_list = ds_info.value[DS_BRGIF_FLD_SECURITY_LIST];
            cdbclt_buildcfg_build_if_psfdb(fp, fp, security_list);
            
#endif /* SHOWRUN_TODO */
            /*show the static port-security on port*/
            if (p_db_if->static_security_num)
            {
                _ccs_cfg_build_psfdb_of_one_port(fp, p_db_if);
            }

            if (p_db_if->dhcp_snooping_trust)
            {
                sal_fprintf(fp, INTEND_1_STR"dhcp snooping trust\n");
            }
            CTCLIB_SLIST_LOOP(p_db_if->circuit_id.obj_list, p_db_circuit, listnode)
            {
                sal_fprintf(fp, INTEND_1_STR"dhcp snooping vlan %u information option format-type circuit-id string %s\n",
                    p_db_circuit->key.vid, p_db_circuit->circuit);
            }

            p_db_mstp_port = tbl_mstp_port_get_mstp_port((tbl_mstp_port_key_t*)(&p_db_if->key));
            if (p_db_mstp_port)
            {
                _ccs_cfg_build_mstp_port_one(fp, p_db_mstp_port);
            }
#endif /* TAPPRODUCT */
        }        
    }
    else
    {
#ifndef TAPPRODUCT
        ds_connected_t *p_db_conn = NULL;
        ctclib_slistnode_t *rt_listnode = NULL;
        ds_ospf_auth_t *p_db_auth_md5 = NULL;
        ds_nd_prefix_t      *p_nd_prefix = NULL;
        ds_connected_v6_t *p_db_conn_v6 = NULL;
        
        sal_memcpy(rt_if_key.name, p_db_if->key.name, IFNAME_SIZE);
        p_db_rtif = tbl_route_if_get_route_if(&rt_if_key);
        if (NULL != p_db_rtif)
        {
            if (GLB_ETHER_IF_DEFAULT_MTU != p_db_rtif->mtu)
            {
                sal_fprintf(fp, INTEND_1_STR"mtu %d\n", p_db_rtif->mtu);
            }

           /* for bug 19077, move this configure before agg's configure write */
            if (p_db_rtif->arp_proxy_en)
            {
                sal_fprintf(fp, INTEND_1_STR"proxy-arp enable\n");
            }

            if (p_db_rtif->local_arp_proxy_en)
            {
                sal_fprintf(fp, INTEND_1_STR"local-proxy-arp enable\n");
            }

            if (GLB_ARP_DEFAULT_RETRANS_TIME != p_db_rtif->arp_retrans_time)
            {
                sal_fprintf(fp, INTEND_1_STR"arp retry-interval %d\n", p_db_rtif->arp_retrans_time);
            }

            if (GLB_ARP_DEFAULT_TIMEOUT != p_db_rtif->arp_timeout)
            {
                sal_fprintf(fp, INTEND_1_STR"arp timeout %d\n", p_db_rtif->arp_timeout);
            }

            if ((GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
                    || (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
                    || (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
                    || (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type && (!INTERFACE_IS_LAG_MEMBER(p_db_if))))
            {
                if (!p_db_rtif->dhcp_client_enable)
                {
                    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, rt_listnode)
                    {
                        if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY)
                            /* support vrrp modified by liwh for bug 45215, 2017-09-19 */
                            && !GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_VIRTUAL))
                            /*liwh end */
                        {
                            if ((NULL != p_db_rtif) && (0x0 != p_db_conn->key.address.u.prefix4.s_addr))
                            {
                                sal_fprintf(fp, INTEND_1_STR"ip address %s/%d\n",
                                    cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_db_conn->key.address.u.prefix4), p_db_conn->key.address.prefixlen);
                            }
                        }
                    }

                    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, rt_listnode)
                    {
                        if (GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
                        {
                            if ((NULL != p_db_rtif) && (0x0 != p_db_conn->key.address.u.prefix4.s_addr))
                            {
                                sal_fprintf(fp, INTEND_1_STR"ip address %s/%d secondary\n",
                                    cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_conn->key.address.u.prefix4), p_db_conn->key.address.prefixlen);
                            }
                        }
                    }
                }

                if (!p_db_rtif->dhcpv6_client_enable)
                {
                    /*for bug 48745 ipv6 and ipv4 is independent*/
                    /*linklocal address*/
                    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_db_conn_v6, rt_listnode)
                    {
                        if (GLB_FLAG_ISSET(p_db_conn_v6->flags, RT_IF_ADDRV6_LINKLOCAL)
                            || GLB_FLAG_ISSET(p_db_conn_v6->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL))
                        {
                            if ((NULL != p_db_rtif) )//&& (0x0 != p_db_conn_v6->key.address.u.prefix6.s_addr))
                            {
                                if (IN6_IS_ADDR_LINKLOCAL(&p_db_conn_v6->key.address.u.prefix6))
                                {
                                    if (GLB_FLAG_ISSET(p_db_conn_v6->flags, RT_IF_ADDRV6_LINKLOCAL))
                                    {
                                        sal_fprintf(fp, INTEND_1_STR"ipv6 address %s link-local\n",
                                            cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_db_conn_v6->key.address.u.prefix6));
                                    }
                                    
                                    if (GLB_FLAG_ISSET(p_db_conn_v6->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL))
                                    {
                                        sal_fprintf(fp, INTEND_1_STR"ipv6 address auto link-local\n");
                                    }
                                }
                            }
                        }
                    }
    
                    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_db_conn_v6, rt_listnode)
                    {
                        if (GLB_FLAG_ISSET(p_db_conn_v6->flags, RT_IF_ADDRV6_GLOBAL) || 
                            GLB_FLAG_ISSET(p_db_conn_v6->flags, RT_IF_ADDRV6_GLOBAL_EUI64))
                        {
                            if ((NULL != p_db_rtif) )//&& (0x0 != p_db_conn_v6->key.address.u.prefix4.s_addr))
                            {
                                if (GLB_FLAG_ISSET(p_db_conn_v6->flags, RT_IF_ADDRV6_GLOBAL_EUI64))
                                {
                                    addr_ipv6_t ipv6_addr_tmp;
                                    sal_memset(&ipv6_addr_tmp, 0, sizeof(addr_ipv6_t));
    
                                    sal_memcpy(&ipv6_addr_tmp, &p_db_conn_v6->key.address.u.prefix6, sizeof(addr_ipv6_t));
                                    apply_mask_ipv6_addr(&ipv6_addr_tmp, p_db_conn_v6->key.address.prefixlen);
                                    sal_fprintf(fp, INTEND_1_STR"ipv6 address %s/%d eui-64\n",
                                        cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &ipv6_addr_tmp), p_db_conn_v6->key.address.prefixlen);
                                }
                                else if (GLB_FLAG_ISSET(p_db_conn_v6->flags, RT_IF_ADDRV6_GLOBAL))
                                {
                                    sal_fprintf(fp, INTEND_1_STR"ipv6 address %s/%d\n",
                                        cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_db_conn_v6->key.address.u.prefix6), p_db_conn_v6->key.address.prefixlen);
                                }
                            }
                        }
                    }
                }
            }

            if (OSPF_RIF_AUTH_TYPE_MD5 == p_db_rtif->ospf_auth_type)
            {
                sal_fprintf(fp, INTEND_1_STR"ip ospf authentication message-digest\n");
            }
			else if(OSPF_RIF_AUTH_TYPE_SIMPLE == p_db_rtif->ospf_auth_type)
            {
                sal_fprintf(fp, INTEND_1_STR"ip ospf authentication\n");
            }

            sal_memset(auth_key, 0, sizeof(auth_key));
            if (sal_memcmp(p_db_rtif->ospf_auth_key, auth_key, sizeof(auth_key)))
            {
                sal_fprintf(fp, INTEND_1_STR"ip ospf authentication-key %s\n", p_db_rtif->ospf_auth_key);
            }
            if (p_db_rtif->ospf_cost)
            {
                sal_fprintf(fp, INTEND_1_STR"ip ospf cost %u\n", p_db_rtif->ospf_cost);
            }
			if (p_db_rtif->ospf_priority > 0 && p_db_rtif->ospf_priority != 2)
            {
                sal_fprintf(fp, INTEND_1_STR"ip ospf priority %u\n", p_db_rtif->ospf_priority-1);
            }
            if (OSPF_HELLO_INTERVAL_DEFAULT != p_db_rtif->ospf_timer_hello)
            {
                sal_fprintf(fp, INTEND_1_STR"ip ospf hello-interval %u\n", p_db_rtif->ospf_timer_hello);
            }

            /* modified by liwh for bug 43976, 2017-08-07 */
            if ((p_db_rtif->ospf_timer_hello * GLB_OSPF_DEAD_INTERVAL_MULTI_HELLO)  != p_db_rtif->ospf_timer_dead)
            //if (OSPF_DEAD_INTERVAL_DEFAULT != p_db_rtif->ospf_timer_dead)
            /* liwh end */
            {
                sal_fprintf(fp, INTEND_1_STR"ip ospf dead-interval %u\n", p_db_rtif->ospf_timer_dead);
            }

            CTCLIB_SLIST_LOOP(p_db_rtif->ospf_md5_key.obj_list, p_db_auth_md5, rt_listnode)
            {
                if(app_is_password_encryption())
                {
                    char szPasswd[PASSWD_SIZE] = {0};
                    gen_passwd_crypt(p_db_auth_md5->ospf_auth_md5_key, szPasswd, PASSWD_SIZE);
                    sal_fprintf(fp, INTEND_1_STR"ip ospf message-digest-key %u md5 8 %s\n", p_db_auth_md5->key.id, szPasswd);
                }
                else
                    sal_fprintf(fp, INTEND_1_STR"ip ospf message-digest-key %u md5 %s\n", p_db_auth_md5->key.id, p_db_auth_md5->ospf_auth_md5_key);
            }

            if (p_db_rtif->ospf_mtu_ignore)
            {
                sal_fprintf(fp, INTEND_1_STR"ip ospf mtu-ignore\n");
            }

			if (p_db_rtif->ospf_network_type)
            {
            	switch(p_db_rtif->ospf_network_type)
            	{
					case OSPF_TYPE_BROADCAST:
                		sal_fprintf(fp, INTEND_1_STR"ip ospf network broadcast\n");
						break;
					case OSPF_TYPE_NBMA:
                		sal_fprintf(fp, INTEND_1_STR"ip ospf network non-broadcast\n");
						break;
					case OSPF_TYPE_P2M:
                		sal_fprintf(fp, INTEND_1_STR"ip ospf network point-to-multipoint\n");
						break;
					case OSPF_TYPE_P2P:
                		sal_fprintf(fp, INTEND_1_STR"ip ospf network point-to-point\n");
						break;
            	}
            }
			
			/*add by zhw for ospf3*/
			if (OSPF_HELLO_INTERVAL_DEFAULT != p_db_rtif->ospf6_timer_hello)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 ospf6 hello-interval %u\n", p_db_rtif->ospf6_timer_hello);
            }

            if ((p_db_rtif->ospf6_timer_hello * GLB_OSPF_DEAD_INTERVAL_MULTI_HELLO)  != p_db_rtif->ospf6_timer_dead)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 ospf6 dead-interval %u\n", p_db_rtif->ospf6_timer_dead);
            }

			if (p_db_rtif->ospf6_mtu_ignore)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 ospf6 mtu-ignore\n");
            }

			if (p_db_rtif->ospf6_cost)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 ospf6 cost %u\n", p_db_rtif->ospf6_cost);
            }
			
			if (p_db_rtif->ospf6_priority > 0 && p_db_rtif->ospf6_priority != 2)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 ospf6 priority %u\n", p_db_rtif->ospf6_priority-1);
            }
			if (p_db_rtif->ospf6_network_type)
            {
            	switch(p_db_rtif->ospf6_network_type)
            	{
					case OSPF_TYPE_BROADCAST:
                		sal_fprintf(fp, INTEND_1_STR"ipv6 ospf6 network broadcast\n");
						break;
					case OSPF_TYPE_P2P:
                		sal_fprintf(fp, INTEND_1_STR"ipv6 ospf6 network point-to-point\n");
						break;
            	}
            }
			
			/*end add*/
			
			if ((1 == p_db_rtif->rip_recv_version) || (2 == p_db_rtif->rip_recv_version))
            {
                sal_fprintf(fp, INTEND_1_STR"ip rip receive version %d\n", p_db_rtif->rip_recv_version);
            }
			else if (3 == p_db_rtif->rip_recv_version)
            {
                sal_fprintf(fp, INTEND_1_STR"ip rip receive version 1 2\n");
            }
			else if (4 == p_db_rtif->rip_recv_version)
            {
                sal_fprintf(fp, INTEND_1_STR"ip rip receive version none\n");
            }
			if ((1 == p_db_rtif->rip_recv_version) || (2 == p_db_rtif->rip_recv_version))
            {
                sal_fprintf(fp, INTEND_1_STR"ip rip send version %d\n", p_db_rtif->rip_send_version);
            }
			else if (3 == p_db_rtif->rip_send_version)
            {
                sal_fprintf(fp, INTEND_1_STR"ip rip send version 1 2\n");
            }
			if (p_db_rtif->rip_v2_broadcast)
            {
                sal_fprintf(fp, INTEND_1_STR"ip rip v2-broadcast\n");
            }
			if (p_db_rtif->rip_split_horizon== RIP_SPLIT_ON && p_db_rtif->rip_poisoned_reverse == RIP_POISONED_ON)
            {
                sal_fprintf(fp, INTEND_1_STR"ip rip split-horizon poisoned-reverse\n");
            }
			if (p_db_rtif->rip_split_horizon== RIP_SPLIT_OFF)
            {
                sal_fprintf(fp, INTEND_1_STR"no ip rip split-horizon\n");
            }
			if (p_db_rtif->rip_auth_mode == RIP_AUTH_TEXT)
            {
                sal_fprintf(fp, INTEND_1_STR"ip rip authentication mode text\n");
            }
			else if (p_db_rtif->rip_auth_mode == RIP_AUTH_MD5)
            {
                sal_fprintf(fp, INTEND_1_STR"ip rip authentication mode md5\n");
            }
			sal_memset(rip_auth_string, 0, sizeof(rip_auth_string));
			if (sal_memcmp(p_db_rtif->rip_auth_string, rip_auth_string, sizeof(rip_auth_string)))
			{
                if(app_is_password_encryption() && (p_db_rtif->rip_auth_mode == RIP_AUTH_MD5))
                {
                    char szPasswd[PASSWD_SIZE] = {0};
                    gen_passwd_crypt(p_db_rtif->rip_auth_string, szPasswd, PASSWD_SIZE);
                    sal_fprintf(fp, INTEND_1_STR"ip rip authentication string 8 %s\n", szPasswd);
                }
                else
                    sal_fprintf(fp, INTEND_1_STR"ip rip authentication string %s\n",p_db_rtif->rip_auth_string);
            }

			if (p_db_rtif->ripng_split_horizon== RIPNG_SPLIT_ON && p_db_rtif->ripng_poisoned_reverse == RIPNG_POISONED_ON)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 ripng split-horizon poisoned-reverse\n");
            }
			if (p_db_rtif->ripng_split_horizon== RIPNG_SPLIT_OFF)
            {
                sal_fprintf(fp, INTEND_1_STR"no ipv6 ripng split-horizon\n");
            }

            if (GLB_ND_DUP_ADDR_DETECT_TRANSMITS != p_db_rtif->nd_dad_attempts)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 nd dad attempts %u\n", p_db_rtif->nd_dad_attempts);
            }

            if (GLB_ND_TRT_ADV_INTERVAL != p_db_rtif->nd_ra_interval_max)
            {
                if ((GLB_ND_TRT_ADV_INTERVAL * 0.33) != p_db_rtif->nd_ra_interval_min) 
                {
                    sal_fprintf(fp, INTEND_1_STR"ipv6 nd ra interval %u %u\n", (p_db_rtif->nd_ra_interval_max + 500)/1000,
                       (p_db_rtif->nd_ra_interval_min + 500)/1000);    
                }
                else
                {
                    sal_fprintf(fp, INTEND_1_STR"ipv6 nd ra interval %u\n", (p_db_rtif->nd_ra_interval_max + 500)/1000);
                }
            }
 
            if (GLB_ND_DEFAULT_RA_CUR_HOP_LIMIT != p_db_rtif->nd_ra_currrent_hop_limit)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 nd ra hop-limit %u\n", p_db_rtif->nd_ra_currrent_hop_limit);
            }

            if ((p_db_rtif->nd_ra_interval_max * 3/1000) != p_db_rtif->nd_router_lifetime)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 nd ra lifetime %u\n", p_db_rtif->nd_router_lifetime);
            }

            if (GLB_ND_BASE_REACHABLE_TIME != p_db_rtif->nd_base_reachable_time)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 nd reachable-time %u\n", p_db_rtif->nd_base_reachable_time);
            }

            if (GLB_ND_RETRANS_TIMER != p_db_rtif->nd_retransmit_timer)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 nd ns-interval %u\n", p_db_rtif->nd_retransmit_timer);    
            }

            if (p_db_rtif->nd_managed_config)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 nd managed-config-flag\n");        
            }

            if (p_db_rtif->nd_other_config)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 nd other-config-flag\n");    
            }

            if ((GLB_ND_DEFAULT_PREFIX_VALID_LIFETIME != p_db_rtif->nd_ra_prefix_default_vlifetime)
                || (GLB_ND_DEFAULT_PREFIX_PREFERED_LIFETIME != p_db_rtif->nd_ra_prefix_default_plifetime)
                || (!p_db_rtif->nd_ra_prefix_default_onlink) || (!p_db_rtif->nd_ra_prefix_default_auto))
            {
                sal_sprintf(nd_line, INTEND_1_STR"ipv6 nd prefix default %u %u", p_db_rtif->nd_ra_prefix_default_vlifetime,
                    p_db_rtif->nd_ra_prefix_default_plifetime);

                if (!p_db_rtif->nd_ra_prefix_default_onlink)
                {
                    sal_sprintf(nd_tmp, " off-link");
                    sal_strncat(nd_line, nd_tmp, sal_strlen(nd_tmp));
                }

                if (!p_db_rtif->nd_ra_prefix_default_auto)
                {
                    sal_sprintf(nd_tmp, " no-autoconfig"); 
                    sal_strncat(nd_line, nd_tmp, sal_strlen(nd_tmp));
                }

                sal_sprintf(nd_tmp, "\n"); 
                sal_strncat(nd_line, nd_tmp, sal_strlen(nd_tmp));

                sal_fprintf(fp, nd_line);
            }

            if ((GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
                || (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
                || (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type && (!INTERFACE_IS_LAG_MEMBER(p_db_if))))
            {
                CTCLIB_SLIST_LOOP(p_db_rtif->nd_prefix.obj_list, p_nd_prefix, rt_listnode)
                {
                    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_nd_prefix->key.addr.u.prefix6);

                    sal_sprintf(nd_line, INTEND_1_STR"ipv6 nd prefix %s/%d", buf, p_nd_prefix->key.addr.prefixlen);

                    if (p_nd_prefix->config)
                    {
                        sal_sprintf(nd_tmp, " %u %u", p_nd_prefix->vlifetime, p_nd_prefix->plifetime);
                        sal_strncat(nd_line, nd_tmp, sal_strlen(nd_tmp));

                        if (!p_nd_prefix->onlink)
                        {
                            sal_sprintf(nd_tmp, " off-link");
                            sal_strncat(nd_line, nd_tmp, sal_strlen(nd_tmp));
                        }
        
                        if (!p_nd_prefix->autoconfig)
                        {
                            sal_sprintf(nd_tmp, " no-autoconfig");
                            sal_strncat(nd_line, nd_tmp, sal_strlen(nd_tmp));
                        }
                    }

                    sal_sprintf(nd_tmp, "\n");
                    sal_strncat(nd_line, nd_tmp, sal_strlen(nd_tmp));

                    sal_fprintf(fp, nd_line);
                }    
            }

            if (p_db_rtif->nd_ra_advertise)
            {
                sal_fprintf(fp, INTEND_1_STR"no ipv6 nd ra suppress\n");
            }

            if (!p_db_rtif->nd_ra_advertise_mtu)
            {
                sal_fprintf(fp, INTEND_1_STR"ipv6 nd ra mtu suppress\n");
            }

            if (GLB_IF_NAT_INSIDE == p_db_rtif->nat_enable)
            {
                sal_fprintf(fp, INTEND_1_STR"ip nat enable inside\n");
            }
            else if (GLB_IF_NAT_OUTSIDE == p_db_rtif->nat_enable)
            {
                sal_fprintf(fp, INTEND_1_STR"ip nat enable outside\n");
            }
			/*add by zhw for mc*/
			if (p_db_rtif->pim_mode == PIM_MODE_DM)
            {
                sal_fprintf(fp, INTEND_1_STR"ip pim mode dense-mode\n");
            }
			else if (p_db_rtif->pim_mode == PIM_MODE_SM)
            {
                sal_fprintf(fp, INTEND_1_STR"ip pim mode sparse-mode\n");
            }
			if (p_db_rtif->multicast == 1)
            {
                sal_fprintf(fp, INTEND_1_STR"multicast on\n");
            }
			if (p_db_rtif->igmp == 1)
            {
                sal_fprintf(fp, INTEND_1_STR"igmp on\n");
            }
			if(sal_strlen(p_db_rtif->igmp_version) > 1)
			{
				sal_fprintf(fp, INTEND_1_STR"ip igmp version  %s\n", p_db_rtif->igmp_version);
			}
			if(p_db_rtif->igmp_intf_limit_number != 0)
			{
				sal_fprintf(fp, INTEND_1_STR"ip igmp limit %d\n", p_db_rtif->igmp_intf_limit_number);
			}
			/*end add zhw*/
        }
#endif /* TAPPRODUCT */
    }

    if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        /* added by tongzb for bug 44010, support loopback on CNOS */
        if(GLB_FLAG_ISSET(p_db_if->phyport_flag, GLB_PHYPORT_FLAG_LOOPBACK))
        {
            if(p_db_if->phy_loopback_mode == GLB_LB_PHY_FAR)
            {
                sal_fprintf(fp, INTEND_1_STR"loopback external\n");
            }
            else if(p_db_if->phy_loopback_mode == GLB_LB_PHY_NEAR)
            {
                char eth_name[IFNAME_SIZE];
                char eth_name2[IFNAME_SIZE];
                if_get_name_by_ifindex(p_db_if->internal_lp_ifindex,eth_name,IFNAME_SIZE);
                IFNAME_ETH2FULL(eth_name, eth_name2);
                sal_fprintf(fp, INTEND_1_STR"loopback phy internal %s\n",eth_name2);
            }
            else if(p_db_if->phy_loopback_mode == GLB_LB_PHY_LOOPBACK)
            {
                sal_fprintf(fp, INTEND_1_STR"loopback internal\n");
            }
            else if(p_db_if->phy_loopback_mode == GLB_LB_PORT)
            {
                sal_fprintf(fp, INTEND_1_STR"loopback port\n");
            }
            else if(p_db_if->phy_loopback_mode == GLB_LB_PORT_SWAP)
            {
                sal_fprintf(fp, INTEND_1_STR"loopback port mac-address swap\n");
            }
        }
        /* added by tongzb for bug 44092, support config media type on CNOS */
    }
    
    if (!INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        if (p_db_if->sflow_counter_en)
        {
            sal_fprintf(fp, INTEND_1_STR"sflow counter-sampling enable\n");
        }
    
        if (p_db_if->sflow_flow_ingress && p_db_if->sflow_flow_egress)
        {
            sal_fprintf(fp, INTEND_1_STR"sflow flow-sampling enable both\n");    
        }
        else if (p_db_if->sflow_flow_ingress)
        {
            sal_fprintf(fp, INTEND_1_STR"sflow flow-sampling enable input\n");
        }
        else if (p_db_if->sflow_flow_egress)
        {
            sal_fprintf(fp, INTEND_1_STR"sflow flow-sampling enable output\n");
        }

         if ((GLB_SFLOW_DEFAULT_RATE != p_db_if->sflow_rate) && (0 != p_db_if->sflow_rate))
        {
            sal_fprintf(fp, INTEND_1_STR"sflow flow-sampling rate %u\n", p_db_if->sflow_rate);
        }
    }

    /* vlanclass configuration */
    if (p_db_if->vlanclass_active_type)
    {
        if (GLB_VLAN_CLASS_MAC_BASED == p_db_if->vlanclass_active_type)
        {
            sal_fprintf(fp, INTEND_1_STR"vlan classifier activate %d based mac\n", p_db_if->vlanclass_group_id);
        }
        else if (GLB_VLAN_CLASS_IP_BASED == p_db_if->vlanclass_active_type)
        {
            sal_fprintf(fp, INTEND_1_STR"vlan classifier activate %d based ip\n", p_db_if->vlanclass_group_id);
        }
        else if (GLB_VLAN_CLASS_PROTOCOL_BASED == p_db_if->vlanclass_active_type)
        {
            sal_fprintf(fp, INTEND_1_STR"vlan classifier activate %d based protocol\n", p_db_if->vlanclass_group_id);
        }
    }
    if (p_db_if->vlanclass_default_action)
    {
        sal_fprintf(fp, INTEND_1_STR"vlan classifier default-action drop\n");
    }
    
    /* storm control configuration */
    if (p_db_if->storm_control)
    {
        if (GLB_RATELIMIT_MODE_LEVEL == p_db_if->storm_control->ucast_mode)
        {
            sal_fprintf(fp, INTEND_1_STR"storm-control unicast level %.2f\n",
                (float)p_db_if->storm_control->ucast_rate/100);
        }
        else if (GLB_RATELIMIT_MODE_PPS == p_db_if->storm_control->ucast_mode)
        {
            sal_fprintf(fp, INTEND_1_STR"storm-control unicast pps %u\n",
                p_db_if->storm_control->ucast_rate);
        }

        if (GLB_RATELIMIT_MODE_LEVEL == p_db_if->storm_control->mcast_mode)
        {
             sal_fprintf(fp, INTEND_1_STR"storm-control multicast level %.2f\n",
                (float)p_db_if->storm_control->mcast_rate/100);
        }
        else if (GLB_RATELIMIT_MODE_PPS == p_db_if->storm_control->mcast_mode)
        {
            sal_fprintf(fp, INTEND_1_STR"storm-control multicast pps %u\n",
                p_db_if->storm_control->mcast_rate);
        }

        if (GLB_RATELIMIT_MODE_LEVEL == p_db_if->storm_control->bcast_mode)
        {
            sal_fprintf(fp, INTEND_1_STR"storm-control broadcast level %.2f\n",
                (float)p_db_if->storm_control->bcast_rate/100);
        }
        else if (GLB_RATELIMIT_MODE_PPS == p_db_if->storm_control->bcast_mode)
        {
            sal_fprintf(fp, INTEND_1_STR"storm-control broadcast pps %u\n",
                p_db_if->storm_control->bcast_rate);
        }
    }

    /*SyncE*/
    ccs_cfg_build_interface_one_synce_port(fp, p_db_if);

    /* arp inspection trust*/
    if (p_db_if->arpinsp_trust_enable)
    {
        sal_fprintf(fp, INTEND_1_STR"ip arp inspection trust\n");
    }

    if (0 != sal_strlen(p_db_if->igs_acl))
    {
        sal_fprintf(fp, INTEND_1_STR"ip access-list %s in\n", p_db_if->igs_acl);
    }

    if (0 != sal_strlen(p_db_if->egs_acl))
    {
#ifdef TAPPRODUCT
        sal_fprintf(fp, INTEND_1_STR"egress %s\n", p_db_if->egs_acl);
#else
        sal_fprintf(fp, INTEND_1_STR"ip access-list %s out\n", p_db_if->egs_acl);
#endif
    }

    /*added by yejl for ipv6 acl*/
    if (0 != sal_strlen(p_db_if->igs_ipv6_acl))
    {
        sal_fprintf(fp, INTEND_1_STR"ipv6 access-list %s in\n", p_db_if->igs_ipv6_acl);
    }
    if (0 != sal_strlen(p_db_if->egs_ipv6_acl))
    {
        sal_fprintf(fp, INTEND_1_STR"ipv6 access-list %s out\n", p_db_if->egs_ipv6_acl);
    }
    /*ended by yejl*/

    /*added by yejl for mac acl*/
    if (0 != sal_strlen(p_db_if->igs_mac_acl))
    {
        sal_fprintf(fp, INTEND_1_STR"mac access-list %s in\n", p_db_if->igs_mac_acl);
    }
    if (0 != sal_strlen(p_db_if->egs_mac_acl))
    {
        sal_fprintf(fp, INTEND_1_STR"mac access-list %s out\n", p_db_if->egs_mac_acl);
    }
    /*ended by yejl*/
    
    if (0 != sal_strlen(p_db_if->igs_policy_map))
    {
        sal_fprintf(fp, INTEND_1_STR"service-policy input %s\n", p_db_if->igs_policy_map);
    }

    if (0 != sal_strlen(p_db_if->egs_policy_map))
    {
        sal_fprintf(fp, INTEND_1_STR"service-policy output %s\n", p_db_if->egs_policy_map);
    }
    /* openflow configuration */
    if (p_db_if->openflow_if)
    {
        if (p_db_if->openflow_if->enable)
        {
            if (p_db_if->openflow_if->obey_vlan_filter)
            {
                sal_fprintf(fp, INTEND_1_STR"openflow enable obey-vlan-filter\n");
            }
            else
            {
                sal_fprintf(fp, INTEND_1_STR"openflow enable\n");
            }
        }
    }

    /* Modified by kcao 2015-01-12 for bug 31501, LAG configuration should be last for most cmds should not be set on LAG member port */
    /* LAG configuration */
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        if (GLB_AGG_MODE_STATIC == p_db_if->lag->mode)
        {
            if((p_db_if->static_agg_priority >> 16))
            {
                sal_fprintf(fp, INTEND_1_STR"static-channel-group %u priority %u\n", p_db_if->lag->lag_id, (p_db_if->static_agg_priority >> 16));
            }
            else
            {
                sal_fprintf(fp, INTEND_1_STR"static-channel-group %u\n", p_db_if->lag->lag_id);
            }
        }
        else if (GLB_AGG_MODE_DYNAMIC_ACTIVE == p_db_if->lag->mode)
        {
            sal_fprintf(fp, INTEND_1_STR"channel-group %u mode active\n", p_db_if->lag->lag_id);
        }
        else if (GLB_AGG_MODE_DYNAMIC_PASSIVE == p_db_if->lag->mode)
        {
            sal_fprintf(fp, INTEND_1_STR"channel-group %u mode passive\n", p_db_if->lag->lag_id);
        }
    }
    if(GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        if(GLB_LINKAGG_MAX_MEMBER != p_db_if->lag->max_active_number)
        {
            sal_fprintf(fp, INTEND_1_STR"max-active-number %u\n", p_db_if->lag->max_active_number);
        }
    }
    
#ifndef TAPPRODUCT
    /* LACP configuration */
    if (p_db_if->lacp)
    {
        if (LACP_DEFAULT_PORT_PRIORITY != p_db_if->lacp->actor_port_priority)
        {
            sal_fprintf(fp, INTEND_1_STR"lacp port-priority %u\n", p_db_if->lacp->actor_port_priority);
        }
        
        if (p_db_if->lacp->short_timeout)
        {
            sal_fprintf(fp, INTEND_1_STR"lacp timeout short\n");
        }
    }

    lldp_if.key.ifindex = p_db_if->ifindex;
    p_db_lldp_if = tbl_lldp_if_get_lldp_if(&lldp_if.key);
    if (p_db_lldp_if)
    {
        /*lldp admin status*/
        if (p_db_lldp_if->mode == LLDP_IF_TXONLY)
        {
            sal_fprintf(fp, INTEND_1_STR"lldp enable txonly\n");
        }
        else if (p_db_lldp_if->mode == LLDP_IF_RXONLY)
        {
            sal_fprintf(fp, INTEND_1_STR"lldp enable rxonly\n");
        }

        /*lldp basic tlv*/
        if (!p_db_lldp_if->tlv_enable_basic)
        {
            sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv basic all\n");
        }
        else if (BASIC_TLV_EN_ALL == (p_db_lldp_if->tlv_enable_basic | BASIC_TLV_EN_ALL))
        {
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, BASIC_TLV_EN_PORT_DESCRIPTION))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv basic port-description\n");
            }
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, BASIC_TLV_EN_SYSTEM_NAME))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv basic system-name\n");
            }
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, BASIC_TLV_EN_SYSTEM_DESCRIPTION))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv basic system-description\n");
            }
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, BASIC_TLV_EN_SYSTEM_CAPABILITIES))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv basic system-capabilities\n");
            }
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, BASIC_TLV_EN_MANAGEMENT_ADDRESS))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv basic management-address\n");
            }
        }

        /*lldp 8021-org-specific tvl*/
        if (!(p_db_lldp_if->tlv_enable_8021 & IEEE_8021_TLV_EN_ALL))
        {
            sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv 8021-org-specific all\n");
        }
        else if (IEEE_8021_TLV_EN_ALL != (p_db_lldp_if->tlv_enable_8021 & IEEE_8021_TLV_EN_ALL))
        {
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_PORT_VLAN_ID))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv 8021-org-specific port-vlan\n");
            }
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_PROTO_VLAN_ID))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv 8021-org-specific protocol-vlan\n");
            }
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_PORT_VLAN_NAME))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv 8021-org-specific vlan-name\n");
            }
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_PROTOCOL_ID))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv 8021-org-specific protocol-id\n");
            }
        }

        /*lldp 8023-org-specific tvl*/
        if (!p_db_lldp_if->tlv_enable_8023)
        {
            sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv 8023-org-specific all\n");
        }
        else if (IEEE_8023_TLV_EN_ALL == (p_db_lldp_if->tlv_enable_8023 | IEEE_8023_TLV_EN_ALL))
        {
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8023, IEEE_8023_TLV_EN_MAC_PHY_CONFIG_STATUS))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv 8023-org-specific mac-phy-cfg\n");
            }
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8023, IEEE_8023_TLV_EN_POWER_VIA_MDI))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv 8023-org-specific power\n");
            }
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8023, IEEE_8023_TLV_EN_LINK_AGG_8023))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv 8023-org-specific link-aggregation\n");
            }
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8023, IEEE_8023_TLV_EN_MAX_FRAME_SIZE))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv 8023-org-specific max-frame-size\n");
            }
        }

        /*lldp 8021-org-specific link-aggregation tvl*/
        if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_LINK_AGG_8021) 
            && !GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8023, IEEE_8023_TLV_EN_LINK_AGG_8023))
        {
            sal_fprintf(fp, INTEND_1_STR"lldp tlv 8021-org-specific link-aggregation\n");
        }

        /*lldp med tvl*/
        if (1 == (p_db_lldp_if->tlv_enable_med & MED_TLV_EN_ALL))
        {
            sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv med all\n");
        }
        else if (MED_TLV_EN_ALL != (p_db_lldp_if->tlv_enable_med & MED_TLV_EN_ALL))
        {
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_med, MED_TLV_EN_NETWORK_POLICY))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv med network-policy\n");
            }
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_med, MED_TLV_EN_EXT_POWER_VIA_MDI))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv med ext-power\n");
            }
            if (!GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_med, MED_TLV_EN_INVENTORY))
            {
                sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp tlv med inventory\n");
            }
        }

        /*lldp vlan-name-id*/
        if (p_db_lldp_if->vlan_name_id > 0)
        {
            sal_fprintf(fp, INTEND_1_STR"lldp tlv 8021-org-specific vlan-name-value %u\n", p_db_lldp_if->vlan_name_id);
        }

        /*lldp med tvl location-id*/
        if (MED_LCI_FORMAT_CIVIC == p_db_lldp_if->med_lci_format)
        {
            int i = 0;
            sal_fprintf(fp, INTEND_1_STR"lldp tlv med location-id civic %u %s", 
                p_db_lldp_if->med_lci_civic.what,  p_db_lldp_if->med_lci_civic.countrycode);
            for (i = 0; i < p_db_lldp_if->med_lci_civic.ca_num; i++)
            {
                sal_fprintf(fp, " %s%d", "ca", i+1);
                sal_fprintf(fp, " %d %s", p_db_lldp_if->med_lci_civic.ca_type[i], p_db_lldp_if->med_lci_civic.ca_value[i]);
            }
            sal_fprintf(fp, "\n");
        }
        else if (MED_LCI_FORMAT_ECS_ELIN == p_db_lldp_if->med_lci_format)
        {
            sal_fprintf(fp, INTEND_1_STR"lldp tlv med location-id ecs-elin %s\n", p_db_lldp_if->med_lci_elin);
        }
    }
    else
    {
        if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
        {
            sal_fprintf(fp, INTEND_1_STR CMD_NO_STR" lldp enable\n");
        }
    }
    
    if ((GLB_IF_TYPE_PORT_IF == p_db_if->hw_type) && (!p_db_if->crc_error_check_en))
    {
        sal_fprintf(fp, INTEND_1_STR"crc-check disable\n");
    }
    if ((GLB_IF_TYPE_PORT_IF == p_db_if->hw_type) && (!p_db_if->crc_recalculation_en))
    {
        sal_fprintf(fp, INTEND_1_STR"crc-recalculation disable\n");
    }
#endif
    sal_memset(&openflow_if_key,0x0, sizeof(tbl_openflow_interface_key_t));
    sal_strcpy(openflow_if_key.name, p_db_if->key.name);
    p_openflow_db_if = tbl_openflow_interface_get_openflow_interface(&openflow_if_key);
    if(p_openflow_db_if)
    {
        if(p_openflow_db_if->openflow_enable)
        {
            sal_fprintf(fp, INTEND_1_STR"openflow enable\n");
        }
        if(!p_openflow_db_if->obey_vlan_filter)
        {
            sal_fprintf(fp, INTEND_1_STR"vlan-filter disable\n");
        }
        if(p_openflow_db_if->bind_tunnel_type != GLB_IF_TYPE_VXLAN_IF)
        {
            if(p_openflow_db_if->bind_tunnel_type == GLB_IF_TYPE_NVGRE_IF)
            {
                sal_fprintf(fp, INTEND_1_STR"openflow tunnel type nvgre\n");
            }
            else if(p_openflow_db_if->bind_tunnel_type == GLB_IF_TYPE_L2GRE_IF)
            {
                sal_fprintf(fp, INTEND_1_STR"openflow tunnel type l2gre\n");
            }
        }
        if(p_openflow_db_if->ingress_add_native_vlan_enable)
        {
            sal_fprintf(fp, INTEND_1_STR"ingress-add-native-vlan enable\n");
        }
        if(p_openflow_db_if->egress_remove_native_vlan_enable)
        {
            sal_fprintf(fp, INTEND_1_STR"egress-remove-native-vlan enable\n");
        }
        if(p_openflow_db_if->tunnel_mpls_disable)
        {
            sal_fprintf(fp, INTEND_1_STR"no openflow tunnel-mpls enable\n");
        }
        for(index = 0; index < OPENFLOW_PROTECTED_VLAN; index ++)
        {
            if(p_openflow_db_if->protected_vlan[index])
            {
                sal_fprintf(fp, INTEND_1_STR"protected-vlan %d\n", p_openflow_db_if->protected_vlan[index]);
            }
        }

        char    pkt_type[16];
        int32   pkt_rc = 0;

        sal_memset(pkt_type, 0 ,sizeof(pkt_type));
        for(index = 0; index < OPENFLOW_PACKET_ACTION; index ++)
        {
            pkt_rc = _ccs_cfg_openflow_packet_type_to_str(index, pkt_type);
            if (PM_E_FAIL == pkt_rc)
            {
                continue;
            }
            if (GLB_OPF_PKT_ACTION_OVS == p_openflow_db_if->action[index])
            {
#ifdef OFPRODUCT
                sal_fprintf(fp, INTEND_1_STR"openflow set packet %s ovs-local\n", pkt_type);
#endif
            }
        }
    }
    sal_memset(&openflow_tunnel_if_key,0x0, sizeof(tbl_openflow_tunnel_interface_key_t));
    sal_strcpy(openflow_tunnel_if_key.name, p_db_if->key.name);
    p_openflow_db_tunnel_if = tbl_openflow_tunnel_interface_get_openflow_tunnel_interface(&openflow_tunnel_if_key);
    if(p_openflow_db_tunnel_if)
    {
        if(GLB_FLAG_ISSET(p_openflow_db_tunnel_if->flag, GLB_OPENFLOW_TUNNEL_IFF_SRCIP))
        {
            sal_fprintf(fp, INTEND_1_STR"tunnel-source-ip %s\n", p_openflow_db_tunnel_if->source_ip);
        }
        if(GLB_FLAG_ISSET(p_openflow_db_tunnel_if->flag, GLB_OPENFLOW_TUNNEL_IFF_DSTIP))
        {
            sal_fprintf(fp, INTEND_1_STR"tunnel-remote-ip %s\n", p_openflow_db_tunnel_if->remote_ip);
        }
        if(GLB_FLAG_ISSET(p_openflow_db_tunnel_if->flag, GLB_OPENFLOW_TUNNEL_IFF_BIND_PORT))
        {
            IFNAME_ETH2FULL(p_openflow_db_tunnel_if->bind_port_name, bind_ifname_ext);
            if(GLB_FLAG_ISSET(p_openflow_db_tunnel_if->flag, GLB_OPENFLOW_TUNNEL_IFF_BIND_VLAN))
            {
                sal_fprintf(fp, INTEND_1_STR"tunnel-bind-static bind-port %s nexthop-mac %s bind-vlan %d\n", bind_ifname_ext, 
                    p_openflow_db_tunnel_if->bind_mac, p_openflow_db_tunnel_if->bind_vlan);
            }
            else
            {
                sal_fprintf(fp, INTEND_1_STR"tunnel-bind-static bind-port %s nexthop-mac %s\n", bind_ifname_ext, p_openflow_db_tunnel_if->bind_mac);
            }
        }
        if(p_openflow_db_tunnel_if->openflow_enable)
        {
            sal_fprintf(fp, INTEND_1_STR"openflow enable\n");
        }
    }

    ifindex = tbl_interface_get_ifindex_by_name(p_db_if->key.name);
    port_stats_rate_key.ifindex = ifindex;
    p_db_stats_rate = tbl_port_stats_rate_get_port_stats_rate(&port_stats_rate_key);
    if (p_db_stats_rate)
    {
        if (GLB_IF_STATS_LOAD_DEF_INTVAL != p_db_stats_rate->load_interval)
        {
            sal_fprintf(fp, INTEND_1_STR"load-interval %d\n", p_db_stats_rate->load_interval);
        }

        if (p_db_stats_rate->input_rate_threshold)
        {
            sal_fprintf(fp, INTEND_1_STR"log-threshold input-rate %d resume-rate %d\n",
                        p_db_stats_rate->input_rate_threshold, p_db_stats_rate->input_rate_resume);
        }

        if (p_db_stats_rate->output_rate_threshold)
        {
            sal_fprintf(fp, INTEND_1_STR"log-threshold output-rate %d resume-rate %d\n",
                        p_db_stats_rate->output_rate_threshold, p_db_stats_rate->output_rate_resume);
        }

        if (p_db_stats_rate->output_discard_threshold)
        {
            sal_fprintf(fp, INTEND_1_STR"log-threshold output-discard %"PRIu64" interval %d\n",
                        p_db_stats_rate->output_discard_threshold, p_db_stats_rate->output_discard_interval);
        }
    }
    
    ccs_cfg_build_interface_one_l2protocol_port(fp, p_db_if);
    ccs_cfg_build_interface_one_ptp_port(fp, p_db_if);

    ccs_cfg_build_interface_one_ipmc_port(fp, p_db_if);

    ccs_cfg_build_interface_one_rmon_port(fp, p_db_if);

    if (p_db_if->cfm_lmep_num)
    {
        ccs_cfg_build_interface_cfm_mep( fp, p_db_if);
    }

    if (p_db_if->cfm_rmep_num)
    {
        ccs_cfg_build_interface_cfm_rmep( fp, p_db_if);
    }
    sal_fprintf(fp, "!\n");

    return PM_E_NONE;
}

int32
ccs_cfg_build_interface(FILE* fp, char *key, cfg_cmd_para_t *para)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
        
    if (key)
    {
        sal_memset(&if_key, 0, sizeof(if_key));
        sal_strncpy(if_key.name, key, IFNAME_SIZE);
        p_db_if = tbl_interface_get_interface(&if_key);
        if (NULL == p_db_if)
        {
            CFG_CONFLICT_RET("Interface %s does not exist", IFNAME_ETH2FULL(key, ifname_ext));            
        }
        ccs_cfg_build_interface_one(fp, p_db_if);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
        {
            ccs_cfg_build_interface_one(fp, p_db_if);
        }
    }
    
    return PM_E_NONE;
}

