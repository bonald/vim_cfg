/* Added by weizj for tunnel: show running */
#include "proto.h"
#include "gen/tbl_openflow_define.h"
#include "gen/tbl_openflow.h"
#include "glb_openflow_define.h"


/*************************
 * Global and Declaration
 *************************/


/*************************************************************************************************
 * Name         : ccs_cfg_build_openflow
 * Purpose      : Ccs layer build openflow param
 * Input        : fp: FILE*, para: cfg_cmd_para_t*
 * Output       : N/A 
 * Return       : PM_E_XXX
**************************************************************************************************/
int32
ccs_cfg_build_openflow(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_openflow_t *p_master = tbl_openflow_get_openflow();
    char ip_str[GLB_MAX_DESC_STR_LEN];

    if (!p_master)
    {
        return PM_E_NONE;
    }

    if(p_master->dpid_is_set)
    {
        sal_fprintf(fp, "openflow set dpid %s\n", p_master->dpid);
        sal_fprintf(fp, "!\n");
    }
    if (strlen(p_master->private_key) && strlen(p_master->certificate) && strlen(p_master->ca_cert))
    {
        sal_fprintf(fp, "openflow set ssl-key flash:/%s flash:/%s flash:/%s\n", p_master->private_key, p_master->certificate, p_master->ca_cert);
        sal_fprintf(fp, "!\n");
    }
    /* Modifed by weizj for tunnel bug 33358 */
    if (!sal_strcmp(p_master->decap_mode, "type-1"))
    {
        sal_fprintf(fp, "openflow tunnel decap_mode %s\n", p_master->decap_mode);
        sal_fprintf(fp, "!\n");
    }

    if (p_master->local_vtep_ip.s_addr)
    {
        inet_ntop (AF_INET, &p_master->local_vtep_ip.s_addr, ip_str, sizeof(ip_str));
        sal_fprintf(fp, "openflow tunnel local_vtep_ip %s\n", ip_str);
        sal_fprintf(fp, "!\n");
    }
/*removed by wangl for hybrid bug48975*/
#if 0
    if (p_master->group_hash_key != GLB_ECMP_HASH_KEY_ALL)
    {
        sal_fprintf(fp, "group select hash key");

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_MACSA))
        {
            sal_fprintf(fp, " macsa");
        }

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_MACDA))
        {
            sal_fprintf(fp, " macda");
        }

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_ETHERTYPE))
        {
            sal_fprintf(fp, " ether-type");
        }

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_IPSA))
        {
            sal_fprintf(fp, " ipsa");
        }

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_IPDA))
        {
            sal_fprintf(fp, " ipda");
        }

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_IPPRO))
        {
            sal_fprintf(fp, " ip-protocol");
        }

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_DSCP))
        {
            sal_fprintf(fp, " dscp");
        }

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_SRC_PORT))
        {
            sal_fprintf(fp, " src-port");
        }

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_DST_PORT))
        {
            sal_fprintf(fp, " dst-port");
        }

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_INNER_MACSA))
        {
            sal_fprintf(fp, " inner-macsa");
        }

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_INNER_MACDA))
        {
            sal_fprintf(fp, " inner-macda");
        }

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_INNER_IPSA))
        {
            sal_fprintf(fp, " inner-ipsa");
        }

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_INNER_IPDA))
        {
            sal_fprintf(fp, " inner-ipda");
        }

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_INNER_IP_PROTOCOL))
        {
            sal_fprintf(fp, " inner-ip-protocol");
        }

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_INNER_SRC_PORT))
        {
            sal_fprintf(fp, " inner-src-port");
        }

        if (GLB_FLAG_ISSET(p_master->group_hash_key, GLB_ECMP_HASH_KEY_INNER_DST_PORT))
        {
            sal_fprintf(fp, " inner-dst-port");
        }

        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "!\n");
    }
#endif
    if (p_master->bond_hash_use != (GLB_PARSER_LAG_HASH_USE_IP | GLB_PARSER_LAG_HASH_USE_L4)
        || p_master->bond_hash_key != (GLB_PARSER_LAG_HASH_KEY_IP_ALL |GLB_PARSER_LAG_HASH_KEY_L4_ALL ))
    {
        sal_fprintf(fp, "bond select hash use");
        
        if (GLB_FLAG_ISSET(p_master->bond_hash_use, GLB_PARSER_LAG_HASH_USE_MAC))
        {
            sal_fprintf(fp, " mac");
        }
        
        if (GLB_FLAG_ISSET(p_master->bond_hash_use, GLB_PARSER_LAG_HASH_USE_IP))
        {
            sal_fprintf(fp, " ip");
        }
                
        if (GLB_FLAG_ISSET(p_master->bond_hash_use, GLB_PARSER_LAG_HASH_USE_L4))
        {
            sal_fprintf(fp, " layer4");
        }

        sal_fprintf(fp,"\n");

        /* Mac */
        {
            sal_fprintf(fp, "bond select hash mac");

            if (GLB_FLAG_ISSET(p_master->bond_hash_key, GLB_PARSER_LAG_HASH_KEY_PORT))
            {
                sal_fprintf(fp, " port");
            }

            if (GLB_FLAG_ISSET(p_master->bond_hash_key, GLB_PARSER_LAG_HASH_KEY_MACSA))
            {
                sal_fprintf(fp, " macsa");
            }
            if (GLB_FLAG_ISSET(p_master->bond_hash_key, GLB_PARSER_LAG_HASH_KEY_MACDA))
            {
                sal_fprintf(fp, " macda");
            }

            if (GLB_FLAG_ISSET(p_master->bond_hash_key, GLB_PARSER_LAG_HASH_KEY_VID))
            {
                sal_fprintf(fp, " vid");
            }

            if (GLB_FLAG_ISSET(p_master->bond_hash_key, GLB_PARSER_LAG_HASH_KEY_COS))
            {
                sal_fprintf(fp, " cos");
            }

            if (GLB_FLAG_ISSET(p_master->bond_hash_key, GLB_PARSER_LAG_HASH_KEY_L2PRO))
            {
                sal_fprintf(fp, " ether-type");
            }

            sal_fprintf(fp, "\n");
        }

        /* IP */
        {
            sal_fprintf(fp, "bond select hash ip");

            if (GLB_FLAG_ISSET(p_master->bond_hash_key, GLB_PARSER_LAG_HASH_KEY_IPSA))
            {
                sal_fprintf(fp, " ipsa");
            }

            if (GLB_FLAG_ISSET(p_master->bond_hash_key, GLB_PARSER_LAG_HASH_KEY_IPDA))
            {
                sal_fprintf(fp, " ipda");
            }
            if (GLB_FLAG_ISSET(p_master->bond_hash_key, GLB_PARSER_LAG_HASH_KEY_IPPRO))
            {
                sal_fprintf(fp, " ip-protocol");
            }

            sal_fprintf(fp, "\n");
        }

        /* Layer4 */
        {
            sal_fprintf(fp, "bond select hash layer4");

            if (GLB_FLAG_ISSET(p_master->bond_hash_key, GLB_PARSER_LAG_HASH_KEY_SRC_PORT))
            {
                sal_fprintf(fp, " src-port");
            }

            if (GLB_FLAG_ISSET(p_master->bond_hash_key, GLB_PARSER_LAG_HASH_KEY_DST_PORT))
            {
                sal_fprintf(fp, " dst-port");
            }
            
            sal_fprintf(fp, "\n");
        }

        sal_fprintf(fp, "!\n");

    }

    if (p_master->lacp_local_proc != TRUE)
    {
        sal_fprintf(fp, CMD_NO_STR" lacp local-process enable");

        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "!\n");
    }
    /* Added by weizj for tunnel bug 33724 */
    if (p_master->flow_drop_pkt_to_ingress_port)
    {
        sal_fprintf(fp, "flow drop-pkt-to-ingress-port\n");
        sal_fprintf(fp, "!\n");
    }
    /* Added by weizj for bug 35135 */
    if (p_master->group_drop_pkt_to_ingress_port)
    {
        sal_fprintf(fp, "group drop-pkt-to-ingress-port\n");
        sal_fprintf(fp, "!\n");
    }
    /* Added by weizj for udf */
    if (p_master->udf_parser)
    {
        sal_fprintf(fp, "openflow udf enable\n");
        sal_fprintf(fp, "!\n");
    }
    if (p_master->ptp_e2e_en)
    {
        sal_fprintf(fp, "openflow e2e enable\n");
        sal_fprintf(fp, "!\n");
    }
    /* Added by weizj for meter */
    if (p_master->meter_ipg)
    {
        sal_fprintf(fp, "openflow meter ipg enable\n");
        sal_fprintf(fp, "!\n");
    }
    /* Added by weizj for inband */
    if (p_master->inband_en)
    {
        sal_fprintf(fp, "inband enable\n");
        sal_fprintf(fp, "!\n");
    }

    if (p_master->inband_stag)
    {
        sal_fprintf(fp, "inband property stag %d\n", p_master->inband_stag);
        sal_fprintf(fp, "!\n");
    }

    if (p_master->inband_addr.prefix.s_addr)
    {
        inet_ntop (AF_INET, &p_master->inband_addr.prefix.s_addr, ip_str, sizeof(ip_str));
        sal_fprintf(fp, "inband ip address %s/%d\n", ip_str, p_master->inband_addr.prefixlen);
        sal_fprintf(fp, "!\n");
    }
    else if (p_master->inband_addr.prefixlen)
    {
        sal_fprintf(fp, "inband ip address dhcp\n");
        sal_fprintf(fp, "!\n");
    }
    if (p_master->inband_gw.s_addr)
    {
        inet_ntop (AF_INET, &p_master->inband_gw.s_addr, ip_str, sizeof(ip_str));
        sal_fprintf(fp, "inband route gateway %s\n", ip_str);
        sal_fprintf(fp, "!\n");
    }
    
    if (!p_master->mpls_parser)
    {
        sal_fprintf(fp, CMD_NO_STR" mpls parser enable\n");
        sal_fprintf(fp, "!\n");
    }
    if(p_master->vxlan_srcport_is_set)
    {
        sal_fprintf(fp, "vxlan-tunnel src-port %d\n", p_master->vxlan_srcport);
        sal_fprintf(fp, "!\n");
    }
    if(p_master->vxlan_destport_is_set)
    {
        sal_fprintf(fp, "vxlan-tunnel dest-port %d\n", p_master->vxlan_destport);
        sal_fprintf(fp, "!\n");
    }

    if(p_master->pw_not_standard_mode)
    {
        sal_fprintf(fp, "no mpls-tp oam-y1731 pw-mode standard\n");
        sal_fprintf(fp, "!\n");
    }
    
    if(p_master->tp_oam_oam_cycle_is_disable)
    {
        sal_fprintf(fp, "no mpls-tp oam-y1731 cycle-packet-in oam-session enable\n");
        sal_fprintf(fp, "!\n");
    }
    if(p_master->tp_oam_oam_interval_is_set)
    {
        if(p_master->tp_oam_oam_interval != GLB_OPF_OAM_PACKET_IN_DEFAULT_INTERVAL)
        {
            sal_fprintf(fp, "mpls-tp oam-y1731 cycle-packet-in oam-session interval %d\n", p_master->tp_oam_oam_interval);
            sal_fprintf(fp, "!\n");
        }
    }
    
    if(p_master->tp_oam_aps_cycle_is_disable)
    {
        sal_fprintf(fp, "no mpls-tp oam-y1731 cycle-packet-in aps-group enable\n");
        sal_fprintf(fp, "!\n");
    }
    if(p_master->tp_oam_aps_interval_is_set)
    {
        if(p_master->tp_oam_aps_interval != GLB_OPF_APS_PACKET_IN_DEFAULT_INTERVAL)
        {
            sal_fprintf(fp, "mpls-tp oam-y1731 cycle-packet-in aps-group interval %d\n", p_master->tp_oam_aps_interval);
            sal_fprintf(fp, "!\n");
        }
    }
    
    if ((p_master->stpid != 0x8100))
    {
        sal_fprintf(fp, "openflow dot1q stpid 0x%x\n", p_master->stpid);
        sal_fprintf(fp, "!\n");
    }
    if ((p_master->ctpid != 0x8100))
    {
        sal_fprintf(fp, "openflow dot1q ctpid 0x%x\n", p_master->ctpid);
        sal_fprintf(fp, "!\n");
    }
    if(p_master->openflow_mac_learning_enable)
    {
        sal_fprintf(fp, "openflow mac-learning enable\n");
        sal_fprintf(fp, "!\n");
    }

    if (p_master->fdb_flow_en)
    {
        sal_fprintf(fp, "openflow extend mac-table enable\n");
        sal_fprintf(fp, "!\n");
    }

    if(p_master->controller_affect_flow_disable)
    {
        sal_fprintf(fp, "openflow set controller-affect-flow disable\n");
        sal_fprintf(fp, "!\n");
    }

    if (p_master->async_msgid_en)
    {
        sal_fprintf(fp, "openflow set async-msg-id enable\n");
        sal_fprintf(fp, "!\n");
    }

    if (p_master->ffgroup_switchback_en)
    {
        sal_fprintf(fp, "openflow set ff-group-cutback enable\n");
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}
