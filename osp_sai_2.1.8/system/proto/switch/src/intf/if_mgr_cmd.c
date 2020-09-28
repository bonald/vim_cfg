
#include "proto.h"
#include "gen/tbl_policy_map_config_define.h" 
#include "gen/tbl_policy_map_config.h" 
#include "gen/tbl_class_map_config_define.h" 
#include "gen/tbl_class_map_config.h" 
#include "gen/tbl_time_range_define.h"
#include "gen/tbl_time_range.h"
#include "glb_if_define.h"
#include "glb_phy_define.h"
#include "glb_l2_define.h"
#include "glb_ratelimit_define.h"
#include "glb_hash_field_value_define.h"
#include "lib_tblinc.h"
#include "switch.h"
#include "switch_api.h"
#include "swth_sys.h"
#include "if_mgr.h"
#include "if_agg.h"
#include "brg_if.h"
#include "lacp_pdu.h"
#include "lacp.h"
#include "qos/qos_mgr.h"
#include "mirror_define.h"
#include "acl.h"
#include "errdisable.h"
#include "acl_policy.h"
#include "ipsour/ip_source.h"
#include "brg_fdb.h"
#include "ctclib_opb.h"
#include "gen/tbl_openflow_interface_define.h"
#include "gen/tbl_openflow_interface.h"
#include "gen/tbl_openflow_tunnel_interface_define.h"
#include "gen/tbl_openflow_tunnel_interface.h"
#include "gen/tbl_openflow_tunnel_local_ip_cnt_define.h"
#include "gen/tbl_openflow_tunnel_local_ip_cnt.h"
#include "gen/tbl_ssm_port_define.h"
#include "gen/tbl_ssm_port.h"
#include "gen/tbl_lldp_if_define.h"
#include "gen/tbl_lldp_if.h"
#include "gen/tbl_hash_value_profile_define.h"
#include "gen/tbl_hash_value_profile.h"
#include "gen/tbl_qos_queue_smart_buffer_define.h"
#include "gen/tbl_qos_queue_smart_buffer.h"
#include "fei_openflow_port.h"
#include "sflow_api_header.h"
#include "hash_field_profile.h"
#include "hash_value_profile_and_global.h"
/* modified by liwh for bug 46651, 2019-03-06 */
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_sys_spec.h"
#include "gen/tbl_l2edit_define.h"
#include "gen/tbl_l2edit.h"
/* liwh end */
#include "gen/tbl_ecmp_global_define.h"
#include "gen/tbl_ecmp_global.h"
/*Added by zhush for bug 52768, 2019-07-04*/
#include "cJSON.h"

#define IF_MGR_SHOW_ALL     0
#define IF_MGR_SHOW_LAG     1
#define IF_MGR_SHOW_LACP    2
#define IF_MGR_SHOW_PHY_LAG 3
#define IF_MGR_SHOW_PHY     4

extern int32
tap_cmd_process_interface_pmap(char *pmap, bool isadd, tbl_interface_t *p_db_if);

#ifdef CONFIG_L2

static int32
_if_mgr_cmd_set_aclqos_if_replace_cos(tbl_interface_t *p_db_if, bool enable)
{
    ds_aclqos_if_t aclqos_if;
    int32 rc = PM_E_NONE;

    if (!tbl_qos_global_get_qos_enable() || (NULL == p_db_if->aclqos_if))
    {
        return PM_E_NONE;
    }

    sal_memset(&aclqos_if, 0, sizeof(aclqos_if));
    aclqos_if.replace = enable ? GLB_QOS_REPLACE_COS : GLB_QOS_REPLACE_NONE;
    
    PM_E_RETURN(ds_aclqos_if_set_aclqos_if_field(p_db_if, &aclqos_if, DS_ACLQOS_IF_FLD_REPLACE));
    PM_E_RETURN(qos_mgr_port_queue_update_handle(GLB_QOS_UPDATE_OP_SET, p_db_if->ifindex, 0, 0, GLB_QOS_PORT_QUEUE_UPDATE_REPLACE));

    return PM_E_NONE;
}
#endif /* CONFIG_L2 */

static int32
_if_mgr_cmd_add_if(char *ifname, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_t intf;
    tbl_interface_t *p_if = &intf;
    tbl_interface_t *p_db_if = NULL;
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    uint32 value = 0;
    uint32 vlan_if_count = 0;
    int32 rc = PM_E_NONE;
    
    /* 1. get ifindex from ifname */
    sal_memset(p_if, 0, sizeof(tbl_interface_t));
    sal_strncpy(p_if->key.name, ifname, IFNAME_SIZE);
    IFNAME_ETH2FULL(ifname, ifname_ext);
    
    /* 2. check exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (p_db_if)
    {
        return PM_E_NONE;
    }

    /* 3. add different interface (port/agg/vlan/tunnel/...) */
    rc = tbl_interface_get_ifindex_by_name(ifname);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Invalid interface name %s", ifname_ext);
        return PM_E_INVALID_IFNAME;
    }
    p_if->ifindex = rc;
    
    if (0 == sal_memcmp(ifname, GLB_IFNAME_ETH_PREFIX, GLB_IFNAME_ETH_PREFIX_LEN))
    {
        if (argc != 11)
        {
            CFG_CONFLICT_RET("Interface %s does not exist", ifname_ext);
        }
        rc = if_mgr_add_port_if(ifname, argv, argc, p_if, para);
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_VLAN_PREFIX, GLB_IFNAME_VLAN_PREFIX_LEN))
    {
        /* check vlan interface number limitation */
        vlan_if_count = if_mgr_get_vlan_if_count();
        if (vlan_if_count >= GLB_MAX_VLAN_IF_NUMBER)
        {
            CFG_CONFLICT_RET("Exceeding maximum VLAN interfaces limit");
        }

        /* check per-condition */
        rc = sal_sscanf(ifname, GLB_IFNAME_VLAN_PREFIX"%u", &value);
        vlan_key.vid = value;
        p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
        if (NULL == p_db_vlan)
        {
            CFG_CONFLICT_RET("Vlan %u does not exist", value);
        }

        rc = if_mgr_add_vlan_if(ifname, argv, argc, p_if);
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_AGG_PREFIX, GLB_IFNAME_AGG_PREFIX_LEN))
    {
        CFG_CONFLICT_RET("Interface %s does not exist", ifname_ext);
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_LOOPBACK_PREFIX, GLB_IFNAME_LOOPBACK_PREFIX_LEN))
    {
        rc = if_mgr_add_loopback_if(ifname, argv, argc, p_if);
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_L2GRE_PREFIX, GLB_IFNAME_L2GRE_PREFIX_LEN))
    {
        if(!check_feature_support(CONFIG_FEATURE_HYBRID_TUNNEL))
        {
            CFG_INVALID_PARAM_RET("Not supported on this product");
        }
        rc = if_mgr_add_l2gre_if(ifname, argv, argc, p_if);
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_L3GRE_PREFIX, GLB_IFNAME_L3GRE_PREFIX_LEN))
    {
        if(!check_feature_support(CONFIG_FEATURE_HYBRID_TUNNEL))
        {
            CFG_INVALID_PARAM_RET("Not supported on this product");
        }
        rc = if_mgr_add_l3gre_if(ifname, argv, argc, p_if);
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_NVGRE_PREFIX, GLB_IFNAME_NVGRE_PREFIX_LEN))
    {
        if(!check_feature_support(CONFIG_FEATURE_HYBRID_TUNNEL))
        {
            CFG_INVALID_PARAM_RET("Not supported on this product");
        }
        rc = if_mgr_add_nvgre_if(ifname, argv, argc, p_if);
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_VXLAN_PREFIX, GLB_IFNAME_VXLAN_PREFIX_LEN))
    {
        if(!check_feature_support(CONFIG_FEATURE_HYBRID_TUNNEL))
        {
            CFG_INVALID_PARAM_RET("Not supported on this product");
        }
        rc = if_mgr_add_vxlan_if(ifname, argv, argc, p_if);
    }
    else
    {
        CFG_INVALID_PARAM_RET("Invalid interface name: %s,", ifname_ext);
    }
    
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add interface fail, %s", pm_err_desc(rc));
    }

    return rc;
}

static int32
_if_mgr_cmd_del_if(char *ifname, cfg_cmd_para_t *para)
{
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    sal_memset(&if_key, 0 ,sizeof(if_key));
    sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
    
    /* 2. check exist */
    p_db_if = tbl_interface_get_interface(&if_key);
    if (!p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s does not exist", ifname);
    }

    if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Cannot delete physical interface %s", IFNAME_ETH2FULL(ifname, ifname_ext));
    }

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Cannot delete LAG interface %s directly", ifname);
    }

    if (0 != p_db_if->tap_group_igs_ref_cnt || 0 != p_db_if->tap_group_egs_ref_cnt)
    {
        CFG_CONFLICT_RET("Interface %s is already applied with tap group", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

#if 1
    if (p_db_if->inband_snat_type)
    {
        CFG_INVALID_PARAM_RET("'%s' already assigned to %s",
                              p_db_if->key.name,
                              INBAND_SNAT_TYPE_TO_STRING(p_db_if->inband_snat_type));
    }
#endif

    rc = if_mgr_del_if(ifname, para);
    if (rc < 0)
    {
        if(PM_E_CFG_CONFLICT == rc)
        {
            CFG_FUNC_ERR_RET("This interface is enable openflow, please disable it first");
        }
        else
        {
            CFG_FUNC_ERR_RET("Delete interface fail, %s", pm_err_desc(rc));
        }
    }
    return rc;
}

static int32
_if_mgr_cmd_get_if_all(char *ifname, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field, uint32 show_type)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    FILE *fp = NULL;
    tbl_interface_key_t key;
    tbl_interface_t *p_db_if = NULL;
    //tbl_port_stats_t *p_port_stats = NULL;
    //tbl_port_stats_key_t port_stats_key;
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;
    char ifname_ext[IFNAME_EXT_SIZE];
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    if (NULL == ifname)
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
        {
            if (IF_MGR_SHOW_PHY_LAG == show_type)
            {
                if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
                {
                    continue;
                }
            }
            else if (IF_MGR_SHOW_PHY == show_type)
            {
                if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
                {
                    continue;
                }
            }
            tbl_interface_dump_one(p_db_if, &args);
            //port_stats_key.ifindex = p_db_if->ifindex;
            #if 0
            p_port_stats = tbl_port_stats_get_port_stats(&port_stats_key);
            if(p_port_stats == NULL)
            {
                continue;
            }
            tbl_port_stats_dump_one(p_port_stats, &args);
            #endif
        }
    }
    else
    {
        sal_memset(&key, 0, sizeof(key));
        sal_strncpy(key.name, ifname, IFNAME_SIZE);
        p_db_if = tbl_interface_get_interface(&key);
        if (!p_db_if)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("Interface %s is not exist", IFNAME_ETH2FULL(ifname, ifname_ext));
        }
        tbl_interface_dump_one(p_db_if, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_if_mgr_cmd_get_if_lag_lacp(char *ifname, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field, uint32 show_type)
{
    FILE *fp = NULL;
    tbl_interface_t *p_if_lag = NULL;
    tbl_interface_t *p_if_mem = NULL;
    tbl_iter_args_t args;
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *listnode_mem = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    if (NULL == ifname)
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_if_lag, listnode)
        {
            if (!INTERFACE_IS_LAG_GROUP(p_if_lag))
            {
                /* bypass Non-LAG */
                continue;
            }

            if (GLB_AGG_MODE_STATIC == p_if_lag->lag->mode)
            {
                if (IF_MGR_SHOW_LACP == show_type)
                {
                    /* LACP should bypass static LAG */
                    continue;
                }
            }

            /* LACP need member port info */
            if (IF_MGR_SHOW_LACP == show_type)
            {
                CTCLIB_SLIST_LOOP(p_if_lag->lag->member_ports.obj_list, p_if_mem, listnode_mem)
                {
                    tbl_interface_dump_one(p_if_mem, &args);
                }
            }
            else
            {
                tbl_interface_dump_one(p_if_lag, &args);
            }
        }
    }
    else
    {
        p_if_lag = tbl_interface_get_interface_by_name(ifname);
        if (!p_if_lag)
        {
            para->p_show->rc = PM_E_NONE;
            sal_fclose(fp);
            return PM_E_NONE;
        }

        if (GLB_AGG_MODE_STATIC == p_if_lag->lag->mode)
        {
            if (IF_MGR_SHOW_LACP == show_type)
            {
                CFG_DEL_SHOW_PATH(para->p_show->path, fp);
                CFG_CONFLICT_RET("Interface %s is static LAG", ifname);
            }
        }
        
        /* LACP need member port info */
        if (IF_MGR_SHOW_LACP == show_type)
        {
            CTCLIB_SLIST_LOOP(p_if_lag->lag->member_ports.obj_list, p_if_mem, listnode_mem)
            {
                tbl_interface_dump_one(p_if_mem, &args);
            }
        }
        else
        {
            tbl_interface_dump_one(p_if_lag, &args);
        }
    }
    
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_if_mgr_cmd_get_if(char *ifname, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field, uint32 show_type)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;
    
    if (IF_MGR_SHOW_ALL == show_type ||IF_MGR_SHOW_PHY == show_type || IF_MGR_SHOW_PHY_LAG == show_type)
    {
        rc = _if_mgr_cmd_get_if_all(ifname, para, p_node, p_field, show_type);
        if (rc < 0)
        {
            CFG_CONFLICT_RET("Interface %s does not exist", IFNAME_ETH2FULL(ifname, ifname_ext));
        }
    }
    else if (IF_MGR_SHOW_LAG == show_type || IF_MGR_SHOW_LACP == show_type)
    {
        return _if_mgr_cmd_get_if_lag_lacp(ifname, para, p_node, p_field, show_type);
    }

    return PM_E_NONE;
}

/* Fix bug 44508, allowed vlan is time consumed to sync */
#define IF_MGR_ALLOWED_VLAN_WAIT_TIME       8   /* ms */
#define IF_MGR_ALLOWED_VLAN_WAIT_COUNT      50  /* wait 8ms/50vlans */

#ifdef CONFIG_L2
static int32
_if_mgr_cmd_set_brgif_allowed_vlan(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, glb_vlan_port_type_t cfg_type, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    vlan_bmp_t tmp_bmp;
    tbl_vlan_key_t vlan_key;
    ds_brgif_t *p_db_brgif = p_db_if->brgif;
    tbl_vlan_t *p_db_vlan = NULL;
    uint32 allowed_vlan_count = 0;
    uint32 add_count = 0;
    vid_t allowed_vlan_fail_vid = 0;
    vid_t vid = 0;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (GLB_VLAN_PORT_TYPE_ACCESS == p_db_if->vlan_type)
    {
        if (GLB_IF_PVLAN_TYPE_NONE != p_db_if->pvlan_type)
        {
            CFG_CONFLICT_RET("Interface %s is private-vlan port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
        if (GLB_VLAN_PORT_TYPE_ACCESS != cfg_type)
        {
            CFG_CONFLICT_RET("Interface %s is access port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
    }
    
    if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
    {
        if (GLB_VLAN_PORT_TYPE_TRUNK != cfg_type)
        {
            CFG_CONFLICT_RET("Interface %s is trunk port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
    }

    if (GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type)
    {
        if (GLB_VLAN_PORT_TYPE_QINQ != cfg_type)
        {
            CFG_CONFLICT_RET("Interface %s is dot1q-tunnel port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
    }

    if ((DS_BRGIF_ALLOWED_VLAN_ACT_ADD == p_brgif->allowed_vlan.action) || (DS_BRGIF_ALLOWED_VLAN_ACT_DEL == p_brgif->allowed_vlan.action))
    {
        if ((GLB_VLAN_PORT_TYPE_TRUNK != p_db_if->vlan_type) && (GLB_VLAN_PORT_TYPE_ACCESS != p_db_if->vlan_type)
            && (GLB_VLAN_PORT_TYPE_QINQ != p_db_if->vlan_type))
        {
            CFG_CONFLICT_RET("Interface %s is not access, trunk or dot1q-tunnel port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
    }
    else
    {
        if (GLB_VLAN_PORT_TYPE_TRUNK != p_db_if->vlan_type && GLB_VLAN_PORT_TYPE_QINQ != p_db_if->vlan_type)
        {
            CFG_CONFLICT_RET("Interface %s is not trunk or dot1q-tunnel port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
    }

    switch (p_brgif->allowed_vlan.action)
    {
    case DS_BRGIF_ALLOWED_VLAN_ACT_ADD:
        if (DS_BRGIF_ALLOWED_VLAN_ACT_ALL == p_db_brgif->allowed_vlan.action)
        {
            p_brgif->allowed_vlan.action = DS_BRGIF_ALLOWED_VLAN_ACT_ALL;
        }
        sal_memcpy(tmp_bmp, p_db_brgif->allowed_vlan.vlan_bmp, sizeof(vlan_bmp_t));
        GLB_BMP_ITER_BEGIN(p_brgif->allowed_vlan.vlan_bmp, vid)
        {
            allowed_vlan_count++;
            vlan_key.vid = vid;
            p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
            if (NULL == p_db_vlan)
            {
                allowed_vlan_fail_vid = vid;
                continue;
            }
            if (GLB_BMP_ISSET(p_db_brgif->allowed_vlan.vlan_bmp, vid))
            {
            }
            else
            {
                PM_E_RETURN(brg_if_set_allowed_vlan_process(p_db_if, vid, TRUE, TRUE, TRUE));
                GLB_BMP_SET(tmp_bmp, vid);
                add_count++;
                if (0 == add_count%IF_MGR_ALLOWED_VLAN_WAIT_COUNT)
                {
                    sal_task_sleep(IF_MGR_ALLOWED_VLAN_WAIT_TIME);
                }
            }
        }
        GLB_BMP_ITER_END(p_brgif->allowed_vlan.vlan_bmp, vid);
        sal_memcpy(p_brgif->allowed_vlan.vlan_bmp, tmp_bmp, sizeof(vlan_bmp_t));
        if (0 != allowed_vlan_fail_vid)
        {
            if (1 == allowed_vlan_count)
            {
                para->p_rs->rc = PM_E_CFG_CONFLICT;
                sal_snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Vlan %u is not exist", allowed_vlan_fail_vid);
            }
            else
            {
                para->p_rs->rc = PM_E_CFG_CONFLICT;
                sal_snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Part of the VLANs add to the port fail");
            }
        }
        break;

    case DS_BRGIF_ALLOWED_VLAN_ACT_DEL:
        p_brgif->allowed_vlan.action = DS_BRGIF_ALLOWED_VLAN_ACT_ADD;
        sal_memcpy(tmp_bmp, p_db_brgif->allowed_vlan.vlan_bmp, sizeof(vlan_bmp_t));
        if (GLB_VLAN_PORT_TYPE_ACCESS == p_db_if->vlan_type)
        {
            if (GLB_BMP_ISSET(p_brgif->allowed_vlan.vlan_bmp, p_db_brgif->pvid))
            {
                CFG_CONFLICT_RET("Can't remove default VID %u from the port", p_db_brgif->pvid);
            }
        }
        GLB_BMP_ITER_BEGIN(p_brgif->allowed_vlan.vlan_bmp, vid)
        {
            if (GLB_BMP_ISSET(p_db_brgif->allowed_vlan.vlan_bmp, vid))
            {
                /* modified by liwh for bug 42728, 2017-03-14 */
                if (p_db_if->erps_enable)
                {
                    vlan_key.vid = vid;
                    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
                    if (!p_db_vlan)
                    {
                        return PM_E_NOT_EXIST;
                    }
        
                    if (p_db_vlan->erps_domain_id)
                    {
                        CFG_CONFLICT_RET("Can not remove an erps domain primary/secondary port from a control vlan");
                    }
                }
                /* liwh end */

                PM_E_RETURN(brg_if_set_allowed_vlan_process(p_db_if, vid, FALSE, TRUE, TRUE));
                GLB_BMP_UNSET(tmp_bmp, vid);
            }
            else
            {
            }
        }
        GLB_BMP_ITER_END(p_brgif->allowed_vlan.vlan_bmp, vid);
        sal_memcpy(p_brgif->allowed_vlan.vlan_bmp, tmp_bmp, sizeof(vlan_bmp_t));
        break;

    case DS_BRGIF_ALLOWED_VLAN_ACT_ALL:
        sal_memcpy(tmp_bmp, p_db_brgif->allowed_vlan.vlan_bmp, sizeof(vlan_bmp_t));
        for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
        {
            vlan_key.vid = vid;
            p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
            if (NULL == p_db_vlan)
            {
                continue;
            }
            if (GLB_BMP_ISSET(p_db_brgif->allowed_vlan.vlan_bmp, vid))
            {
            }
            else
            {
                PM_E_RETURN(brg_if_set_allowed_vlan_process(p_db_if, vid, TRUE, TRUE, TRUE));
                GLB_BMP_SET(tmp_bmp, vid);
                add_count++;
                if (0 == add_count%IF_MGR_ALLOWED_VLAN_WAIT_COUNT)
                {
                    sal_task_sleep(IF_MGR_ALLOWED_VLAN_WAIT_TIME);
                }
            }
        }
        sal_memcpy(p_brgif->allowed_vlan.vlan_bmp, tmp_bmp, sizeof(vlan_bmp_t));
        break;
        
    case DS_BRGIF_ALLOWED_VLAN_ACT_NONE:
        GLB_BMP_ITER_BEGIN(p_db_brgif->allowed_vlan.vlan_bmp, vid)
        {
            PM_E_RETURN(brg_if_set_allowed_vlan_process(p_db_if, vid, FALSE, TRUE, TRUE));
        }
        GLB_BMP_ITER_END(p_db_brgif->allowed_vlan.vlan_bmp, vid);
        GLB_BMP_INIT(p_db_brgif->allowed_vlan.vlan_bmp);
        break;

    default:
        SAL_ASSERT(0);
    }

    /* 2. call API */
    PM_E_RETURN(brg_if_set_allowed_vlan_db(p_db_if, p_brgif, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_set_brgif_untagged_vlan(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, glb_vlan_port_type_t cfg_type, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    vlan_bmp_t tmp_bmp;
    tbl_vlan_key_t vlan_key;
    ds_brgif_t *p_db_brgif = p_db_if->brgif;
    tbl_vlan_t *p_db_vlan = NULL;
    uint32 untagged_vlan_count = 0;
    uint32 add_count = 0;
    vid_t untagged_vlan_fail_vid = 0;
    vid_t vid = 0;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (GLB_VLAN_PORT_TYPE_TRUNK != p_db_if->vlan_type)
    {       
        CFG_CONFLICT_RET("Interface %s is not trunk port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    switch (p_brgif->untagged_vlan.action)
    {
    case DS_BRGIF_UNTAGGED_VLAN_ACT_ADD:
        sal_memcpy(tmp_bmp, p_db_brgif->untagged_vlan.vlan_bmp, sizeof(vlan_bmp_t));
        GLB_BMP_ITER_BEGIN(p_brgif->untagged_vlan.vlan_bmp, vid)
        {
            untagged_vlan_count++;
            vlan_key.vid = vid;
            p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
            if (NULL == p_db_vlan)
            {
                untagged_vlan_fail_vid = vid;
                continue;
            }
            if (p_db_brgif->pvid == vid)
            {
                untagged_vlan_fail_vid = vid;
                continue;
            }
            if (GLB_BMP_ISSET(p_db_brgif->untagged_vlan.vlan_bmp, vid))
            {
            }
            else
            {
                GLB_BMP_SET(tmp_bmp, vid);
                add_count++;
            }
        }
        GLB_BMP_ITER_END(p_brgif->untagged_vlan.vlan_bmp, vid);
        sal_memcpy(p_brgif->untagged_vlan.vlan_bmp, tmp_bmp, sizeof(vlan_bmp_t));
        if (0 != untagged_vlan_fail_vid)
        {
            if (1 == untagged_vlan_count)
            {
                para->p_rs->rc = PM_E_CFG_CONFLICT;
                sal_snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Vlan %u is not exist or is native vlan", untagged_vlan_fail_vid);
            }
            else
            {
                para->p_rs->rc = PM_E_CFG_CONFLICT;
                sal_snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Part of the VLANs add to the port fail");
            }
        }
        break;

    case DS_BRGIF_UNTAGGED_VLAN_ACT_DEL:
        sal_memcpy(tmp_bmp, p_db_brgif->untagged_vlan.vlan_bmp, sizeof(vlan_bmp_t));
        GLB_BMP_ITER_BEGIN(p_brgif->untagged_vlan.vlan_bmp, vid)
        {
            if (GLB_BMP_ISSET(p_db_brgif->untagged_vlan.vlan_bmp, vid))
            {
                GLB_BMP_UNSET(tmp_bmp, vid);
            }
            else
            {
            }
        }
        GLB_BMP_ITER_END(p_brgif->untagged_vlan.vlan_bmp, vid);
        sal_memcpy(p_brgif->untagged_vlan.vlan_bmp, tmp_bmp, sizeof(vlan_bmp_t));
        break;

    default:
        SAL_ASSERT(0);
    }

    /* 2. call API */
    PM_E_RETURN(brg_if_set_untagged_vlan_db(p_db_if, p_brgif, TRUE));
    return rc;
}


static int32
_if_mgr_cmd_set_brgif_pvid(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, uint8 vlan_type, cfg_cmd_para_t *para)
{
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t* p_vlan = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    /* 1. config check */
    vlan_key.vid = p_brgif->pvid;
    p_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_vlan)
    {
        CFG_CONFLICT_RET("Vlan %u is not exist", p_brgif->pvid);
    }
    if (vlan_type != p_db_if->vlan_type)
    {
        CFG_CONFLICT_RET("Interface %s is not %s port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), 
            cdb_enum_val2str(glb_vlan_port_type_strs, GLB_VLAN_PORT_TYPE_MAX, vlan_type));
    }
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Cannot configure aggregator member");
    }
    if (GLB_IF_PVLAN_TYPE_NONE != p_db_if->pvlan_type)
    {
        CFG_CONFLICT_RET("Interface has configured as a private VLAN");
    }
    
    /* 2. check duplicate config */
    if (p_db_if->brgif->pvid == p_brgif->pvid)
    {
        return PM_E_NONE;
    }

    /* modified by liwh for bug 40620, 2017-01-22 */
    if (p_db_if->dot1x_cfg_guest_vlan == p_brgif->pvid)
    {
        CFG_CONFLICT_RET("Can't configure native VLAN same as guest VLAN");
    }
    /* liwh end */
    
    /* 3. call API */
    PM_E_RETURN(brg_if_set_pvid(p_db_if, p_brgif->pvid, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_set_brgif_inner_native_vlan(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, uint8 vlan_type, cfg_cmd_para_t *para)
{
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t* p_vlan = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    /* 1. config check */
    vlan_key.vid = p_brgif->inner_native_vid;
    p_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_vlan)
    {
        CFG_CONFLICT_RET("Vlan %u is not exist", p_brgif->inner_native_vid);
    }
    if (vlan_type != p_db_if->vlan_type)
    {
        CFG_CONFLICT_RET("Interface %s is not %s port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), 
            cdb_enum_val2str(glb_vlan_port_type_strs, GLB_VLAN_PORT_TYPE_MAX, vlan_type));
    }
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Cannot configure aggregator member");
    }
    if (GLB_VLAN_QINQ_TYPE_SELECTIVE != p_db_if->brgif->qinq_type)
    {
        CFG_CONFLICT_RET("Interface %s is not selective qinq port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    if (GLB_IF_PVLAN_TYPE_NONE != p_db_if->pvlan_type)
    {
        CFG_CONFLICT_RET("Interface has configured as a private VLAN");
    }
    
    /* 2. check duplicate config */
    if (p_db_if->brgif->inner_native_vid == p_brgif->inner_native_vid)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(brg_if_set_inner_native_vlan(p_db_if, p_brgif->inner_native_vid, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_set_brgif_qinq_type(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char  ifname_ext[IFNAME_EXT_SIZE];

    /* 1. config check */
    if (GLB_VLAN_PORT_TYPE_QINQ != p_db_if->vlan_type)
    {
        CFG_CONFLICT_RET("Interface %s isn't configured as QinQ port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    if (GLB_IF_PVLAN_TYPE_NONE != p_db_if->pvlan_type)
    {
        CFG_CONFLICT_RET("Interface has configured as a private VLAN");
    }
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Cannot configure aggregator member");
    }

    if (p_db_if->brgif->inner_native_vid && GLB_VLAN_QINQ_TYPE_BASIC == p_brgif->qinq_type)
    {
        CFG_CONFLICT_RET("Can't configure qinq basic when untagged inner-vlan pvid is enabled");
    }

    if (p_db_if->brgif->qinq_default_fwd && GLB_VLAN_QINQ_TYPE_BASIC == p_brgif->qinq_type)
    {
        CFG_CONFLICT_RET("Can't configure qinq basic when miss-forward is enabled");
    }
    
    /* 2. check duplicate config */
    if (p_db_if->brgif->qinq_type == p_brgif->qinq_type)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_if_set_qinq_type(p_db_if, p_brgif->qinq_type, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_set_brgif_vlan_translation_en(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char  ifname_ext[IFNAME_EXT_SIZE];

    /* 1. config check */
    if (GLB_VLAN_PORT_TYPE_TRUNK != p_db_if->vlan_type)
    {
        CFG_CONFLICT_RET("Interface %s isn't configured as trunk port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    if (GLB_IF_PVLAN_TYPE_NONE != p_db_if->pvlan_type)
    {
        CFG_CONFLICT_RET("Interface has configured as a private VLAN");
    }
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Cannot configure aggregator member");
    }
    if (p_db_if->ip_source_type)
    {
        CFG_CONFLICT_RET("Interface is enabled IP source guard");
    }
    if (p_db_if->vlanclass_active_type)
    {
        CFG_CONFLICT_RET("Interface is enabled vlan classification");
    }
    if (p_db_if->l2pro_uplink)
    {
        CFG_CONFLICT_RET("Interface is configured as l2 protocol uplink port");
    }

    if (p_db_if->current_g8032_ring_count)
    {
        CFG_FUNC_ERR_RET("G8032 has been configured, can't configure vlan translation");
    }

    /* modified by liwh for bug 53010, 2019-07-22 */
    if_mgr_clear_interface_cfm_meps(p_db_if);
    /* liwh end */

    /* 2. check duplicate config */
    if (p_db_if->brgif->vlan_translation_en == p_brgif->vlan_translation_en)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_if_set_vlan_translation_en(p_db_if, p_brgif->vlan_translation_en, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_set_brgif_vlan_mapping_table(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, 
        uint8 vlan_type, uint32 is_add, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    /* config check */
    if (vlan_type != p_db_if->vlan_type)
    {
        CFG_CONFLICT_RET("Interface %s is not %s port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), 
            cdb_enum_val2str(glb_vlan_port_type_strs, GLB_VLAN_PORT_TYPE_MAX, vlan_type));
    }

    if (GLB_IF_PVLAN_TYPE_NONE != p_db_if->pvlan_type)
    {
        CFG_CONFLICT_RET("Interface has configured as a private VLAN");
    }

    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Cannot configure aggregator member");
    }

    if (GLB_VLAN_PORT_TYPE_TRUNK == vlan_type)
    {
        if (!p_db_if->brgif->vlan_translation_en)
        {
            CFG_CONFLICT_RET("Interface not enable vlan translation");
        }
        /* call API */
        PM_E_RETURN(brg_if_apply_vlanmap_table(p_db_if, p_brgif, is_add, para));
    }
    else if (GLB_VLAN_PORT_TYPE_QINQ == vlan_type)
    {
        if (GLB_VLAN_QINQ_TYPE_SELECTIVE != p_db_if->brgif->qinq_type)
        {
            CFG_CONFLICT_RET("Interface %s is not selective qinq port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
        /* call API */
        PM_E_RETURN(brg_if_apply_vlanmap_table(p_db_if, p_brgif, is_add, para));
    }
    else
    {
        CFG_CONFLICT_RET("Can't apply vlan mapping table execpt on trunk or selective qinq port");
    }
    
    return rc;
}

static int32
_if_mgr_cmd_set_brgif_qinq_default_fwd(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char  ifname_ext[IFNAME_EXT_SIZE];

    /* 1. config check */
    if (GLB_VLAN_PORT_TYPE_QINQ != p_db_if->vlan_type)
    {
        CFG_CONFLICT_RET("Interface %s isn't configured as dot1q-tunnel port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    if (GLB_VLAN_QINQ_TYPE_SELECTIVE != p_db_if->brgif->qinq_type)
    {
        CFG_CONFLICT_RET("Interface %s isn't configured as QinQ selective mode", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    if (GLB_IF_PVLAN_TYPE_NONE != p_db_if->pvlan_type)
    {
        CFG_CONFLICT_RET("Interface has configured as a private VLAN");
    }
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Cannot configure aggregator member");
    }
    if (p_db_if->ip_source_type)
    {
        CFG_CONFLICT_RET("Interface is enabled IP source guard");
    }
    if (p_db_if->vlanclass_active_type)
    {
        CFG_CONFLICT_RET("Interface is enabled vlan classification");
    }

    /* 2. check duplicate config */
    if (p_db_if->brgif->qinq_default_fwd == p_brgif->qinq_default_fwd)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_if_set_qinq_default_fwd(p_db_if, p_brgif->qinq_default_fwd, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_set_brgif_vlan_trans_default_fwd(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char  ifname_ext[IFNAME_EXT_SIZE];

    /* 1. config check */
    if (GLB_VLAN_PORT_TYPE_TRUNK != p_db_if->vlan_type)
    {
        CFG_CONFLICT_RET("Interface %s isn't configured as trunk port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    if (!p_db_if->brgif->vlan_translation_en)
    {
        CFG_CONFLICT_RET("Interface %s isn't enable vlan translation", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    if (GLB_IF_PVLAN_TYPE_NONE != p_db_if->pvlan_type)
    {
        CFG_CONFLICT_RET("Interface has configured as a private VLAN");
    }
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Cannot configure aggregator member");
    }
    if (p_db_if->ip_source_type)
    {
        CFG_CONFLICT_RET("Interface is enabled IP source guard");
    }
    if (p_db_if->vlanclass_active_type)
    {
        CFG_CONFLICT_RET("Interface is enabled vlan classification");
    }

    /* 2. check duplicate config */
    if (p_db_if->brgif->vlan_trans_default_fwd == p_brgif->vlan_trans_default_fwd)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_if_set_vlan_trans_default_fwd(p_db_if, p_brgif->vlan_trans_default_fwd, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_set_brgif_dot1q_tag_native_en(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (GLB_IF_PVLAN_TYPE_NONE != p_db_if->pvlan_type)
    {
        CFG_CONFLICT_RET("Interface has configured as a private VLAN");
    }
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Cannot configure aggregator member");
    }

    if (GLB_VLAN_PORT_TYPE_TRUNK != p_db_if->vlan_type)
    {
        CFG_CONFLICT_RET("Dot1q tag native can't be configured except for trunk port");
    }

    /* 2. check duplicate config */
    if (p_db_if->brgif->dot1q_tag_native_en == p_brgif->dot1q_tag_native_en)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_if_set_dot1q_tag_native_en(p_db_if, p_brgif->dot1q_tag_native_en, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_set_brgif_tpid_type(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    /* 1. config check */

    if (GLB_VLAN_PORT_TYPE_TRUNK != p_db_if->vlan_type)
    {
        CFG_CONFLICT_RET("Interface %s is not trunk port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    /* 2. check duplicate config */
    if (p_db_if->brgif->tpid_type == p_brgif->tpid_type)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_if_set_tpid_type(p_db_if, p_brgif));
    return rc;
}

static int32
_if_mgr_cmd_set_brgif_ingress_filter(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (GLB_VLAN_PORT_TYPE_TRUNK != p_db_if->vlan_type)
    {
        CFG_CONFLICT_RET("Interface %s is not trunk port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    /* 2. check duplicate config */
    if (p_db_if->brgif->ingress_filter == p_brgif->ingress_filter)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_if_set_ingress_filter(p_db_if, p_brgif, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_set_brgif_mac_learning_en(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    rc = swth_api_is_mlag_peer_link(p_db_if->ifindex);
    if (TRUE == rc)
    {
        CFG_CONFLICT_RET("Can't configure mac learning on MLAG peer-link interface");
    }
    
    if (p_brgif->mac_learning_en)
    {
        if (GLB_LB_PHY_LOOPBACK == p_db_if->phy_loopback_mode)
        {
            CFG_CONFLICT_RET("Can't enable mac learning on loopback internal interface");
        }
    }
    
    /* 2. check duplicate config */
    if (p_db_if->brgif->mac_learning_en == p_brgif->mac_learning_en)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_if_set_mac_learning_en(p_db_if, p_brgif->mac_learning_en, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_set_brgif_port_security_en(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];

    /* 1. config check */
    if (p_db_if->mlag_id)
    {
        CFG_CONFLICT_RET("Cannot set port security on MLAG interface");
    }
    /* Added by kcao 2017-01-11 for bug 41372, check port-security and MLAG peer-link */
    if (p_db_if->brgif && p_db_if->brgif->mlag_is_peer_link)
    {
        CFG_CONFLICT_RET("Cannot set port security on MLAG peer-link");
    }
    
    /* 2. check duplicate config */
    if (p_db_if->brgif->port_security_en == p_brgif->port_security_en)
    {
        if (!p_db_if->brgif->port_security_en && !p_brgif->port_security_en)
        {
            CFG_CONFLICT_RET("This port have not enabled port security");
        }
        return PM_E_NONE;
    } 
    
    /* 3. check whether there is static fdb on port */
    if(brg_fdb_is_static_fdb_on_port(p_db_if->ifindex))
    {
        CFG_FUNC_ERR_RET("Static FDB exist on interface %s", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    
    /* 4. call API */
    PM_E_RETURN(brg_if_set_port_security_en(p_db_if, p_brgif->port_security_en, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_set_brgif_port_security_mac_maxinum(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];

    /* 1. config check */
    if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
    {
        if (p_db_if->static_security_num > p_brgif->max_mac)
        {
            CFG_CONFLICT_RET("Configured security address count of interface %s is more than the security max number", 
                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
    }
    
    /* 2. check duplicate config */
    if (p_db_if->brgif->max_mac == p_brgif->max_mac)
    {
        return PM_E_NONE;
    }   
    /* 3. call API */
    PM_E_RETURN(brg_if_set_port_security_mac_maximum(p_db_if, p_brgif->max_mac, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_set_brgif_port_security_violation(ds_brgif_t *p_brgif, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_if->brgif->violate_mode == p_brgif->violate_mode)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(brg_if_set_port_security_voilation(p_db_if, p_brgif->violate_mode, TRUE));
    return rc;
}
#endif /* CONFIG_L2 */

static int32
_if_mgr_cmd_set_lacp_port_priority(uint16 port_priority, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    ds_lacp_t *p_db_lacp = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (!INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Interface %s is not LAG member port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    p_db_lacp = p_db_if->lacp;
    if (NULL == p_db_lacp)
    {
        CFG_CONFLICT_RET("Interface %s is not bound to dynamic LAG", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    /* 2. check duplicate config */
    if (p_db_lacp->actor_port_priority == port_priority)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(lacp_set_port_priority(p_db_if, port_priority));
    return rc;
}

static int32
_if_mgr_cmd_set_lacp_port_short_timeout(uint32 enable, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    ds_lacp_t *p_db_lacp = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (!INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Interface %s is not LAG member port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    p_db_lacp = p_db_if->lacp;
    if (NULL == p_db_lacp)
    {
        CFG_CONFLICT_RET("Interface %s is not bound to dynamic LAG", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    /* 2. check duplicate config */
    if (p_db_lacp->short_timeout == enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(lacp_set_port_short_timeout(p_db_if, enable));
    return rc;
}

static int32
_if_mgr_cmd_set_desc(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. call API */
    PM_E_RETURN(if_mgr_set_desc(p_if));
    return rc;
}

static int32
_if_mgr_cmd_set_svlan_tpid(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];

#ifdef TAPPRODUCT
    /* 1. config check */
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }

    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_if->svlan_tpid == p_if->svlan_tpid)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(if_mgr_set_svlan_tpid(p_if));
#else
    /* 1. config check */
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Can not set dot1q ethertype on interface %s\n", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_if->svlan_tpid == p_if->svlan_tpid)
    {
        return PM_E_NONE;
    }

    PM_E_RETURN(if_mgr_set_dot1q_ethertype(p_db_if, p_if->svlan_tpid, para, TRUE));

#endif

    return rc;
}

const char*
_if_mgr_get_fec_str(tbl_interface_t* p_db_if)
{
    switch(p_db_if->fec)
    {
        case GLB_FEC_RS:
            return "RS";
        case GLB_FEC_BASE_R:
            return "BASE-R";
        case GLB_FEC_OFF:
            return "OFF";
        default :
            if(p_db_if->fec&GLB_FEC_DEFAULT_MASK)
                return "DEFAULT";
            else
                return "UNKNOWN";
    }
}

const char*
_if_mgr_get_fec_status_str(tbl_interface_t* p_db_if)
{
    switch(p_db_if->fec_status)
    {
        case GLB_FEC_RS:
            return "RS";
        case GLB_FEC_BASE_R:
            return "BASE-R";
        case GLB_FEC_OFF:
            return "OFF";
        default :
            return "UNKNOWN";
    }
}

#if 0
void 
_if_mgr_get_fec_ability_str(tbl_interface_t* p_db_if,char ability[256])
{
    int has_ability=0;
    uint32 fec_ability = p_db_if->cl73_ability;
    memset(ability, 0, 256);
    if(fec_ability&GLB_CL73_FEC_ABILITY)
    {
        sal_strcat(ability,"\n              IEEE-10G-BASER-ABILITY");
        has_ability=1;
    }
    if(fec_ability&GLB_CL73_FEC_REQUESTED)
    {
        sal_strcat(ability,"\n              IEEE-10G-BASER-REQ");
        has_ability=1;
    }    
    if(fec_ability&GLB_CL73_25G_RS_FEC_REQUESTED)
    {
        sal_strcat(ability,"\n              IEEE-25G-RS-REQ");
        has_ability=1;
    }
    if(fec_ability&GLB_CL73_25G_BASER_FEC_REQUESTED)
    {
        sal_strcat(ability,"\n              IEEE-25G-BASER-REQ");
        has_ability=1;
    }
    if(fec_ability&GLB_CSTM_RS_FEC_ABILITY)
    {
        sal_strcat(ability,"\n              Consortium-RS-ABILITY");
        has_ability=1;
    }
    if(fec_ability&GLB_CSTM_BASER_FEC_ABILITY)
    {
        sal_strcat(ability,"\n              Consortium-BASER-ABILITY");
        has_ability=1;
    }
    if(fec_ability&GLB_CSTM_RS_FEC_REQUESTED)
    {
        sal_strcat(ability,"\n              Consortium-RS-REQ");
        has_ability=1;
    }
    if(fec_ability&GLB_CSTM_BASER_FEC_REQUESTED)
    {
        sal_strcat(ability,"\n              Consortium-BASER-REQ");
        has_ability=1;
    }
    if(has_ability == 0)
    {
        sal_strcat(ability,"NONE");
    }
    ability[sal_strlen(ability)]='\0';
    return ;

}
#endif

const char*
_if_mgr_get_duplex_str(uint32 duplex, uint32 oper_duplex)
{
    switch(duplex)
    {
    case GLB_DUPLEX_AUTO:
        switch(oper_duplex)
        {
            case GLB_DUPLEX_HALF:
                return "half";

            case GLB_DUPLEX_FULL:
                return "full";

            default :
                return "auto";
        }

    case GLB_DUPLEX_HALF:
        return "half";

    case GLB_DUPLEX_FULL:
        return "full";

    default :
        return "unknown";
    }
}

char *
_if_mgr_get_speed_str(uint32 speed, uint32 oper_speed)
{
    switch(speed)
    {
    case GLB_SPEED_AUTO:
        switch(oper_speed)
        {
        case GLB_SPEED_10M:
            return "10Mb/s";

        case GLB_SPEED_100M:
            return "100Mb/s";

        case GLB_SPEED_1G:
            return "1000Mb/s";

        case GLB_SPEED_2_5G:
            return "2.5Gb/s";

        case GLB_SPEED_5G:
            return "5Gb/s";
            
        case GLB_SPEED_10G:
            return "10Gb/s";

        case GLB_SPEED_40G:
            return "40Gb/s";

        case GLB_SPEED_25G:
            return "25Gb/s";

        case GLB_SPEED_50G:
            return "50Gb/s";            

        case GLB_SPEED_100G:
            return "100Gb/s";
    
        default :
            return "auto";
        }

    case GLB_SPEED_10M:
        return "10Mb/s";

    case GLB_SPEED_100M:
        return "100Mb/s";

    case GLB_SPEED_1G:
        return "1000Mb/s";

    case GLB_SPEED_2_5G:
        return "2.5Gb/s";

    case GLB_SPEED_5G:
        return "5Gb/s";
        
    case GLB_SPEED_10G:
        return "10Gb/s";

    case GLB_SPEED_40G:
        return "40Gb/s";

    case GLB_SPEED_25G:
        return "25Gb/s";

    case GLB_SPEED_50G:
        return "50Gb/s";

    case GLB_SPEED_100G:
        return "100Gb/s";

    default:
        return "unknown";
    }
}

static int32
_if_mgr_cmd_check_speed(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];

    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    
    switch (p_db_if->phy_type)
    {
    case GLB_PHY_TYPE_1000BASE_T:
        if ((GLB_SPEED_1G   == p_if->speed)
         || (GLB_SPEED_100M == p_if->speed)
         || (GLB_SPEED_10M  == p_if->speed)
         || (GLB_SPEED_AUTO == p_if->speed))
        {
            /* for bug 48904 */
            if((GLB_SPEED_1G == p_if->speed) && (GLB_DUPLEX_AUTO != p_db_if->duplex))
            {
                CFG_CONFLICT_RET("Interface %s forced duplex unsupport to set speed %s", 
                    IFNAME_ETH2FULL(p_if->key.name, ifname_ext), 
                    cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_if->speed));
            }
        }
        else
        {
            CFG_CONFLICT_RET("Interface %s PHY type is %s, unsupport to set speed %s", 
                IFNAME_ETH2FULL(p_if->key.name, ifname_ext), cdb_enum_val2str(glb_phy_type_strs, GLB_PHY_TYPE_MAX, p_db_if->phy_type),
                cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_if->speed));
        }
        break;
    case GLB_PHY_TYPE_2G5BASE_T:
    case GLB_PHY_TYPE_5GBASE_T:
        if (((GLB_SPEED_10G  == p_if->speed) && GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_10G))
         || ((GLB_SPEED_2_5G == p_if->speed) && GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_2_5G ))
         || ((GLB_SPEED_5G  == p_if->speed) && GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_5G))
         || ((GLB_SPEED_1G   == p_if->speed) && GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_1G  ))
         || ((GLB_SPEED_100M == p_if->speed) && GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100M))
         || (GLB_SPEED_AUTO == p_if->speed))
        {
            break;
        }
        else
        {
            CFG_CONFLICT_RET("Interface %s unsupport to set speed %s", 
                IFNAME_ETH2FULL(p_if->key.name, ifname_ext), 
                cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_if->speed));
        }
        break;
    default:
        if (((GLB_SPEED_100G == p_if->speed) && GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100G))
         || ((GLB_SPEED_40G  == p_if->speed) && GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_40G ))
         || ((GLB_SPEED_50G  == p_if->speed) && GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_50G ))
         || ((GLB_SPEED_25G  == p_if->speed) && GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_25G ))
         || ((GLB_SPEED_10G  == p_if->speed) && GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_10G ))
         || ((GLB_SPEED_1G   == p_if->speed) && GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_1G  ))
         || ((GLB_SPEED_100M == p_if->speed) && GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100M)))
        {
            if(GLB_SPEED_100M == p_if->speed)
            {
                /* for bug 54452, modified by tongzb, 2019-11-21 */
                CFG_INVALID_PARAM_RET("Warning: Speed %s is no support when insert 1000BASE_T_SFP", 
                                        cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_if->speed));
            }
            break;
        }
        else if ((GLB_SPEED_AUTO == p_if->speed) 
              && (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_1G)
                ||GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_10G)
                ||GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100G)))
        {
            break;
        }
        else
        {
            CFG_CONFLICT_RET("Interface %s unsupport to set speed %s", 
                IFNAME_ETH2FULL(p_if->key.name, ifname_ext), 
                cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_if->speed));
        }
    }

    return PM_E_NONE;
}

static int32
_if_mgr_cmd_check_duplex(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    //uint32 raw_phy_type = GLB_PHY_TYPE_UNKNOWN;
    
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    
    /*fix bug32860, cr10838, liangf, 2015-04-23*/    
    /* phy_type has mask befor so do not mask here, for bug 45768 */
    //raw_phy_type = (~GLB_PORT_PHY_TYPE_MASK) & p_db_if->phy_type;
    switch (p_db_if->phy_type)
    {
    case GLB_PHY_TYPE_1000BASE_T: /*Port Type: RJ-45*/
        if((GLB_SPEED_1G == p_db_if->speed) && (GLB_DUPLEX_AUTO != p_if->duplex))
        {
            CFG_CONFLICT_RET("Interface %s forced 1000M speed unsupport to set duplex %s", 
                IFNAME_ETH2FULL(p_if->key.name, ifname_ext), 
                cdb_enum_val2str(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, p_if->duplex));
        }
        break;
    case GLB_PHY_TYPE_2G5BASE_T:
    case GLB_PHY_TYPE_5GBASE_T:
        if((GLB_SPEED_100M != p_db_if->speed) && (GLB_DUPLEX_AUTO != p_if->duplex))
        {
            CFG_CONFLICT_RET("Interface %s forced %s speed unsupport to set duplex %s", 
                IFNAME_ETH2FULL(p_if->key.name, ifname_ext), 
                cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_if->speed),
                cdb_enum_val2str(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, p_if->duplex));
        }
        break;
    default: /*Port Type: Fiber*/
        if(GLB_DUPLEX_HALF == p_if->duplex)
        {
            CFG_CONFLICT_RET("Interface %s unsupport to set duplex %s", 
                IFNAME_ETH2FULL(p_if->key.name, ifname_ext),
                cdb_enum_val2str(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, p_if->duplex));
        }
        else if(GLB_DUPLEX_AUTO == p_if->duplex)
        {
            if((GLB_SPEED_1G == p_db_if->speed) 
                || (GLB_SPEED_10G == p_db_if->speed)
                || (GLB_SPEED_40G == p_db_if->speed)
                || (GLB_SPEED_100G == p_db_if->speed)
                || (GLB_SPEED_50G == p_db_if->speed)
                || (GLB_SPEED_25G == p_db_if->speed)
                || (GLB_SPEED_AUTO == p_db_if->speed))
            {
                break;
            }
            else
            {
                CFG_CONFLICT_RET("Interface %s speed %s unsupport to set duplex %s", 
                    IFNAME_ETH2FULL(p_if->key.name, ifname_ext), 
                    cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_db_if->speed),
                    cdb_enum_val2str(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, p_if->duplex));
            }
        }
    }

    return PM_E_NONE;
}

static int32
_if_mgr_cmd_check_unidir(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }

    if (!GLB_FLAG_ISSET(p_db_if->phyport_flag, GLB_PHYPORT_FLAG_UNIDIR))
    {
        CFG_CONFLICT_RET("Interface %s unsupport undirection function", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }

    return PM_E_NONE;
}

static int32
_if_mgr_cmd_check_poe(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }

    if (!GLB_FLAG_ISSET(p_db_if->phyport_flag, GLB_PHYPORT_FLAG_POE))
    {
        CFG_CONFLICT_RET("Interface %s unsupport poe function", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }

    return PM_E_NONE;
}

static int32
_if_mgr_cmd_check_fec(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    if ((GLB_CHIP_GREATEBELT == tbl_chip_get_type())
        ||(GLB_CHIP_GOLDENGATE == tbl_chip_get_type()))
    {
        if (!GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100G))
        {
            /* for bug 45705, modified by tongzb */
            CFG_CONFLICT_RET("FEC is only support on 100G physical interface");
        }
    }
    else if(GLB_CHIP_DUET2 == tbl_chip_get_type())
    {
        if (!GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100G)
            &&!GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_25G)
            &&!GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_50G))
        {
            CFG_CONFLICT_RET("FEC is only support on 25G/50G/100G physical interface");
        }
    }
    else if(GLB_CHIP_TSINGMA == tbl_chip_get_type())
    {
        if (!GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100G)
            &&!GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_25G)
            &&!GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_50G)
            &&!GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_40G)
            &&!GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_10G))
        {
            CFG_CONFLICT_RET("FEC is only support on 10G/25G/40G/50G/100G physical interface");
        }
    }
    else
    {
        CFG_CONFLICT_RET("FEC is not support on this ASIC chip");
    }

    return PM_E_NONE;
}

static int32
_if_mgr_cmd_check_fec_warning(tbl_interface_t *p_db_if, uint8 fec_config)
{
    int32 warning = 0;
    if((fec_config&GLB_FEC_DEFAULT_MASK)!=GLB_FEC_DEFAULT)
    {
        if(p_db_if->speed == GLB_SPEED_AUTO)
        {
            if(GLB_CHIP_DUET2 == tbl_chip_get_type())
            {
                if((p_db_if->oper_speed != GLB_SPEED_100G)&&(p_db_if->oper_speed != GLB_SPEED_50G)
                    &&(p_db_if->oper_speed != GLB_SPEED_25G)&&(p_db_if->oper_speed != GLB_SPEED_AUTO))
                {
                    warning = 1;
                }
            }
            else if(GLB_CHIP_TSINGMA == tbl_chip_get_type())
            {
                if((p_db_if->oper_speed != GLB_SPEED_100G)&&(p_db_if->oper_speed != GLB_SPEED_50G)
                    &&(p_db_if->oper_speed != GLB_SPEED_25G)&&(p_db_if->oper_speed != GLB_SPEED_AUTO)
                    &&(p_db_if->oper_speed != GLB_SPEED_10G)&&(p_db_if->oper_speed != GLB_SPEED_40G))
                {
                    warning = 1;
                }
            }
        }
        else
        {
            if(GLB_CHIP_DUET2 == tbl_chip_get_type())
            {
                if((p_db_if->speed != GLB_SPEED_100G)&&(p_db_if->speed != GLB_SPEED_50G)
                    &&(p_db_if->speed != GLB_SPEED_25G)&&(p_db_if->speed != GLB_SPEED_AUTO))
                {
                    warning = 1;
                }
            }
            else if(GLB_CHIP_TSINGMA == tbl_chip_get_type())
            {
                if((p_db_if->speed != GLB_SPEED_100G)&&(p_db_if->speed != GLB_SPEED_50G)
                    &&(p_db_if->speed != GLB_SPEED_25G)&&(p_db_if->speed != GLB_SPEED_AUTO)
                    &&(p_db_if->speed != GLB_SPEED_10G)&&(p_db_if->speed != GLB_SPEED_40G))
                {
                    warning = 1;
                }
            }
        }
        /* for bug 51457, do not check warning on gg board */
        if((GLB_CHIP_DUET2 == tbl_chip_get_type())||(GLB_CHIP_TSINGMA == tbl_chip_get_type()))
        {
            if((fec_config&GLB_FEC_MASK)!=GLB_FEC_BASE_R)
            {
                if(((p_db_if->speed == GLB_SPEED_AUTO)&&(p_db_if->oper_speed == GLB_SPEED_100G))
                    ||(p_db_if->speed == GLB_SPEED_100G))
                {
                    warning = 1;
                }
            }
            if((fec_config&GLB_FEC_MASK)!=GLB_FEC_RS)
            {
                if(GLB_CHIP_TSINGMA == tbl_chip_get_type())
                {
                    if(((p_db_if->speed == GLB_SPEED_AUTO)&&(p_db_if->oper_speed == GLB_SPEED_40G))
                        ||(p_db_if->speed == GLB_SPEED_40G))
                    {
                        warning = 1;
                    }
                    if(((p_db_if->speed == GLB_SPEED_AUTO)&&(p_db_if->oper_speed == GLB_SPEED_10G))
                        ||(p_db_if->speed == GLB_SPEED_10G))
                    {
                        warning = 1;
                    }
                }
            }
        }
    }    
    return warning;
}

static int32
_if_mgr_cmd_set_speed(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);

    /* Fix bug33452, 'no speed' parser issue, qicx, 2015-05-27 */
    if (GLB_SPEED_AUTO == p_if->speed)
    {
        if ((GLB_PHY_TYPE_1000BASE_T == p_db_if->phy_type)
            || (GLB_PHY_TYPE_2G5BASE_T == p_db_if->phy_type)
            || (GLB_PHY_TYPE_5GBASE_T == p_db_if->phy_type))
        {
            p_if->speed = GLB_SPEED_AUTO;
        }
        else
        {
            if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100G))
            {
                p_if->speed = GLB_SPEED_AUTO;
            }
            else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_40G))
            {
                CFG_INVALID_PARAM_RET("Unsupported port speed ability");
            }
            /* for bug 45768, by tongzb, support speed auto on 1G port */
            else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_1G))
            {
                p_if->speed = GLB_SPEED_AUTO;
            }            
            else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100M))
            {
                CFG_INVALID_PARAM_RET("Unsupported port speed ability");
            }
            else
            {
                p_if->speed = GLB_SPEED_AUTO;
            }
        }
    }
    else if (GLB_SPEED_MAX == p_if->speed)  /* for "no speed" */
    {
        if ((GLB_PHY_TYPE_1000BASE_T == p_db_if->phy_type)
            || (GLB_PHY_TYPE_2G5BASE_T == p_db_if->phy_type)
            || (GLB_PHY_TYPE_5GBASE_T == p_db_if->phy_type))
        {
            p_if->speed = GLB_SPEED_AUTO;
        }
        else
        {
            if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100G))
            {
                p_if->speed = GLB_SPEED_AUTO;
            }
            else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_40G))
            {
                p_if->speed = GLB_SPEED_40G;
            }            
            else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_50G))
            {
                p_if->speed = GLB_SPEED_50G;
            }               
            else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_10G))
            {
                p_if->speed = GLB_SPEED_AUTO;
            }
            else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_1G))
            {
                p_if->speed = GLB_SPEED_AUTO;
            }
            else if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100M))
            {
                p_if->speed = GLB_SPEED_100M;
            }
            else
            {
                p_if->speed = GLB_SPEED_AUTO;
            }
        }
    }
      
    /* 1. config check */
    /* for bug 54452, modified by tongzb, 2019-11-21 */
    rc = _if_mgr_cmd_check_speed(p_if, p_db_if, para);
    if(rc != PM_E_CFG_INVALID_PARAM)
    {
        return rc;
    }
    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_if->speed == p_if->speed)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(if_mgr_set_speed(p_if));

    /* 4. update storm-ctl level */
    if(p_db_if->storm_control)
    {
        PM_E_RETURN(if_mgr_update_storm_control_level(p_db_if,p_db_if->storm_control));
    }
    return rc;
}

static int32
_if_mgr_cmd_set_duplex(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    
    if (GLB_DUPLEX_MAX == p_if->duplex)
    {
        if (GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_100G)
            ||GLB_FLAG_ISSET(p_db_if->support_speed_flag, GLB_SUPPORT_SPEED_40G))
        {
            p_if->duplex = GLB_DUPLEX_FULL;
        }
        else
        {
            p_if->duplex = GLB_DUPLEX_AUTO;
        }
    }

    /* 1. config check */
    PM_E_RETURN(_if_mgr_cmd_check_duplex(p_if, p_db_if, para));

    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    /* 2. check duplicate config */
    if (p_db_if->duplex == p_if->duplex)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(if_mgr_set_duplex(p_if));    
    return rc;
}

/* added by tongzb for bug 44368, support loopback on CNOS */
static int32
_if_mgr_cmd_set_loopback(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];

    /* 1. config check */
    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    /* for bug 45577, support config loopback on LAG member port */
    //IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    if (GLB_LB_PHY_LOOPBACK == p_if->phy_loopback_mode)
    {
        if (p_db_if->brgif)
        {
            if (p_db_if->brgif->mac_learning_en)
            {
                CFG_CONFLICT_RET("Please disable mac learning on the interface at first");
            }
        }
    }
    
    /* 2. check duplicate config */
    if (p_db_if->phy_loopback_mode == p_if->phy_loopback_mode)
    {
        if(p_if->phy_loopback_mode == GLB_LB_PHY_NEAR)
        {
            if(p_db_if->internal_lp_ifindex == p_if->internal_lp_ifindex)
                return PM_E_NONE;
        }
        else
            return PM_E_NONE;
    }
    /* add by tongzb for bug 45552, modified the error info */
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port, can't enable loopback", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    if (GLB_LOOPBACK_MAX <= if_mgr_get_loopback_count() && GLB_LB_PHY_FAR != p_if->phy_loopback_mode && 
        GLB_LB_NONE != p_if->phy_loopback_mode && GLB_LB_PHY_LOOPBACK != p_if->phy_loopback_mode)
    {
        CFG_CONFLICT_RET("Mostly %d interface can be configurated phy internal or port loopback", GLB_LOOPBACK_MAX);
    }
    /* 3. call API */
    PM_E_RETURN(if_mgr_set_loopback(p_if));
    return rc;
}

static int32
_if_mgr_cmd_set_metadata(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    
    /* 1. config check */
    if(p_db_if->tap_group_igs_ref_cnt)
    {
        CFG_CONFLICT_RET("Can not set metadata with flow is used");
        return PM_E_CFG_CONFLICT;
    }

//    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    /* 2. check duplicate config */
    if (p_db_if->metadata_enable == p_if->metadata_enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(if_mgr_set_metadata(p_if));    
    return rc;
}

/* added by tongzb for bug 44092, support config media type on CNOS */
static int32
_if_mgr_cmd_set_media(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    /* 2. check duplicate config */

    /* 3. call API */
    PM_E_RETURN(if_mgr_set_media(p_if));    
    return rc;
}

static int32
_if_mgr_cmd_set_unidir(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    PM_E_RETURN(_if_mgr_cmd_check_unidir(p_if, p_db_if, para));

    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    
    /* 2. check duplicate config */
    if (p_db_if->unidir == p_if->unidir)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(if_mgr_set_unidir(p_if));    
    return rc;
}

static int32
_if_mgr_cmd_set_fec(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 warning = 0;
    /* 1. config check */
    PM_E_RETURN(_if_mgr_cmd_check_fec(p_if, p_db_if, para));

    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    /* 2. check duplicate config */
    if (p_db_if->fec == p_if->fec)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(if_mgr_set_fec(p_if));

    /* 4. post handle */
    warning = _if_mgr_cmd_check_fec_warning(p_db_if,p_if->fec);
    if(warning)
    {
        /* for bug 51457, do not check warning on gg board, and change warning note */
        PM_CFG_E_RET_256(para->p_rs, PM_E_CFG_CONFLICT, 
            "Warning: FEC %s can be configured but not in effect on %s port\n"
            "%% Please use show interface to confirm FEC status details\n",
            _if_mgr_get_fec_str(p_db_if),_if_mgr_get_speed_str(p_db_if->speed,p_db_if->oper_speed));
    }
    return rc;
}

static int32
_if_mgr_cmd_set_poe(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    /* 1. config check */
    PM_E_RETURN(_if_mgr_cmd_check_poe(p_if, p_db_if, para));

    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    
    /* 2. check duplicate config */
    if ((p_db_if->poe.enable == p_if->poe.enable)
        &&(p_db_if->poe.force_power == p_if->poe.force_power)
        &&(p_db_if->poe.mode == p_if->poe.mode)
        &&(p_db_if->poe.priority == p_if->poe.priority)
        &&(p_db_if->poe.power_limit == p_if->poe.power_limit))
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(if_mgr_set_poe(p_if));

    return rc;
}

static int32
_if_mgr_cmd_set_reflective(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    /* 2. check duplicate config */
    if (p_db_if->reflective_bridge_en == p_if->reflective_bridge_en)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(if_mgr_set_reflective(p_if));    
    return rc;
}

static int32
_if_mgr_cmd_set_flowcontrol_send(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (p_db_if->pfc_admin)
    {
        CFG_CONFLICT_RET("Interface %s is already enabled priority flow control", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    
    IF_MGR_CHECK_LAG_GROUP(p_db_if);
//    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    /* 3. call API */
    PM_E_RETURN(if_mgr_set_flowcontrol_send(p_if));
    return rc;
}

static int32
_if_mgr_cmd_set_flowcontrol_receive(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (p_db_if->pfc_admin)
    {
        CFG_CONFLICT_RET("Interface %s is already enabled priority flow control", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    
    IF_MGR_CHECK_LAG_GROUP(p_db_if);
//    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    /* 2. call API */
    PM_E_RETURN(if_mgr_set_flowcontrol_receive(p_if));
    return rc;
}

static int32
_if_mgr_cmd_set_pfc_en(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;
    uint32 i = 0;
    
    /* 1. config check */
    if (GLB_FLAG_ISSET(p_db_if->flowcontrol, GLB_IF_FLOW_CONTROL_RECEIVE) || GLB_FLAG_ISSET(p_db_if->flowcontrol, GLB_IF_FLOW_CONTROL_SEND))
    {
        CFG_CONFLICT_RET("Interface %s is already enabled flow control", p_if->key.name);
    }
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    
    IF_MGR_CHECK_LAG_GROUP(p_db_if);
//    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    /* 2. check duplicate config */
    if (p_db_if->pfc_admin == p_if->pfc_admin)
    {
        return PM_E_NONE;
    }
    
    /* 2. call API */
    PM_E_RETURN(if_mgr_set_pfc_en(p_if));

    if (!p_if->pfc_admin)
    {   
        for (i = 0; i< 8; i++)
        {
            if (GLB_IS_BIT_SET(p_db_if->pfc_priority, i))
            {
                GLB_CLEAR_BIT(p_db_if->pfc_priority, i);
            }
        }
        PM_E_RETURN(if_mgr_set_pfc_priority(p_db_if));
    }
    
    return rc;
}

static int32
_if_mgr_cmd_set_pfc_priority(tbl_interface_t *p_if, tbl_interface_t *p_db_if, uint8 priority, uint32 is_add, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    uint32 old_is_add = FALSE;
    int32 rc = PM_E_NONE;
    
    /* 1. config check */
    if (!p_db_if->pfc_admin)
    {
        CFG_CONFLICT_RET("PFC is not enabled on interface %s", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    if (GLB_FLAG_ISSET(p_db_if->flowcontrol, GLB_IF_FLOW_CONTROL_RECEIVE) || GLB_FLAG_ISSET(p_db_if->flowcontrol, GLB_IF_FLOW_CONTROL_SEND))
    {
        CFG_CONFLICT_RET("Interface %s is already enabled flow control", p_if->key.name);
    }
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }

    /* 2. check duplicate config */
    old_is_add = GLB_IS_BIT_SET(p_db_if->pfc_priority, priority);
    if (is_add)
    {
        if (old_is_add == is_add)
        {
            CFG_CONFLICT_RET("Priority %u is exist on interface %s", priority, IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
        }
        else
        {
            GLB_SET_BIT(p_db_if->pfc_priority, priority);
        }
    }
    else
    {
        if (old_is_add == is_add)
        {
            CFG_CONFLICT_RET("Priority %u is not exist on interface %s", priority, IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
        }
        else
        {
            GLB_CLEAR_BIT(p_db_if->pfc_priority, priority);
        }
    }
   
    /* 2. call API */
    PM_E_RETURN(if_mgr_set_pfc_priority(p_db_if));
    
    return rc;
}

static int32
_if_mgr_cmd_set_holdtime(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    if (GLB_PHY_TYPE_1000BASE_T == p_db_if->phy_type)
    {
        CFG_CONFLICT_RET("Interface %s is not fiber port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    if (GLB_FLAG_ISSET(p_db_if->phyport_flag, GLB_PHYPORT_FLAG_COMBO))
    {
        CFG_CONFLICT_RET("Interface %s is combo port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    
    /* 2. call API */
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_CARRIER_DOWN_HOLDTIME_INTERVAL));
    if (0 != p_if->carrier_down_holdtime_interval)
    {
        CFG_PROMPT_RET("Link down-hold-time should be used cautiously");
    }

    return rc;
}

static int32
_if_mgr_cmd_set_jumboframe_en(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    /* 2. call API */
    PM_E_RETURN(if_mgr_set_jumboframe_en(p_if, p_if->jumboframe_en, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_set_isolate_group_id(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port or LAG", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    if (p_db_if->pvlan_type)
    {
        CFG_CONFLICT_RET("Conflict detected! Private vlan is configured on this interface");
    }

    if (p_db_if->mlag_id)
    {
        CFG_CONFLICT_RET("Can't configure port-isolate group on mlag interface");
    }

    if (switch_is_mlag_peer_link(p_db_if->ifindex))
    {
        CFG_CONFLICT_RET("Can't configure port-isolate group on mlag peer-link");
    }

    if (GLB_PORT_ISOLATE_GROUP_MAX <= p_if->isolate_group_id)
    {
        /* GLB_PORT_ISOLATE_GROUP_MAX is reserved for MLAG */
        CFG_CONFLICT_RET("Invalid isolate group id %u", p_if->isolate_group_id);
    }

    /* 2. check duplicate config */
    if (p_db_if->isolate_group_id == p_if->isolate_group_id)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(if_mgr_set_isolate_group_id(p_if, p_if->isolate_group_id, TRUE));

    return rc;
}

static int32
_if_mgr_cmd_set_dhcp_snooping_trust(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);

    /* 1. config check */
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    if (GLB_IF_MODE_L2 != p_db_if->mode)
    {
        CFG_CONFLICT_RET("Interface %s is not layer two interface", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }

    /* 2. check duplicate config */
    if (p_db_if->dhcp_snooping_trust == p_if->dhcp_snooping_trust)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    rc = tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_DHCP_SNOOPING_TRUST);
    return rc;
}

static int32
_if_mgr_cmd_set_dhclient_requested_flag(char **argv, int32 argc, tbl_interface_t *p_db_if, cfg_cmd_para_t *para, bool set)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    ds_dhclient_if_t *p_db_dhclient = NULL;
    int32 rc = PM_E_NONE;
    int32 optpos = 0;
    uint32 flags = 0;
    dhc_req_option_t *ro = NULL;

    /* 1. config check */
    p_db_dhclient = p_db_if->dhclient_if;
    if (NULL == p_db_dhclient)
    {
        CFG_CONFLICT_RET("Interface %s is not exist", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    M_DHC_REQ_OPTION_ARR(req_arr);
    flags = p_db_dhclient->requested_flags;
    
    for (optpos = 0; optpos<argc; optpos++)
    {
        ro = req_arr;
        while (NULL != ro->name)
        {
            if (sal_strcmp(argv[optpos], ro->name) == 0)
            {
                if (set)
                {
                    flags |= ro->flag;
                }
                else
                {
                    flags &= ~ ro->flag;
                }
                break;
            }

            ro++;
        }

    }

    if (p_db_dhclient->requested_flags == flags)
    {
        return PM_E_NONE;
    }

    p_db_dhclient->requested_flags = flags;
    
    /* 3. call API */
    rc = ds_dhclient_if_set_dhclient_if_field(p_db_if, p_db_dhclient, DS_DHCLIENT_IF_FLD_REQUESTED_FLAGS);
    return rc;
}

static int32
_if_mgr_cmd_set_dhclient_client_id(char **argv, int32 argc, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    ds_dhclient_if_t *p_db_dhclient = NULL;
    int32 rc = PM_E_NONE;
    char new_client_id[DHCP_NAME_SIZE] = {'0'};
    uint32 client_flags =0;
    tbl_interface_t  *p_if = NULL;
    tbl_interface_t intf;

    char *port_client_id = argv[3];
    char *port_client_type = argv[4];

    /* 1. config check */
    p_db_dhclient = p_db_if->dhclient_if;
    if (NULL == p_db_dhclient)
    {
        CFG_CONFLICT_RET("Interface %s is not exist", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    if (NULL != port_client_id)
    {   
        if (0 == sal_strcmp(port_client_type, "ascii") )
        {
            if (sal_strlen(port_client_id) > DHCP_NAME_MAX_LEN)
            {
                /* for bug 52212 modified by wus 2019-8-1 */
                CFG_CONFLICT_RET("The length of ASCII string should not exceed 63");
            }
            client_flags = GLB_DHCLIENT_FLAG_ASCII;
        }
        else if (0 == sal_strcmp(port_client_type, "hex") )
        {
            if (sal_strlen(port_client_id) > DHCP_NAME_MAX_LEN*2)
            {
                /* for bug 52212 modified by wus 2019-8-1 */
                CFG_CONFLICT_RET("The length of hex string should not exceed 126");
            }
            client_flags = GLB_DHCLIENT_FLAG_HEX;
        }
        else
        {
            /* check exist */
            sal_memset(&intf, 0, sizeof(intf));
            sal_strncpy(intf.key.name, port_client_id, IFNAME_SIZE);
	
            client_flags = GLB_DHCLIENT_FLAG_PORT;

            p_if = tbl_interface_get_interface(&intf.key);
            if (!p_if)
            {
                tbl_sys_global_t *p_sys_global = tbl_sys_global_get_sys_global();
                if (p_sys_global && p_sys_global->startup_done)
                {
                    CFG_CONFLICT_RET("Interface %s not found", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
                }
            }
        }
        sal_strncpy(new_client_id, port_client_id, DHCP_NAME_SIZE);

        /* 2. check duplicate config */
        if (0 == sal_strcmp(p_db_dhclient->client_id, new_client_id)
        && p_db_dhclient->client_flags == client_flags)
        {
            return PM_E_NONE;
        }
        
        p_db_dhclient->client_flags = client_flags;
        sal_strncpy(p_db_dhclient->client_id, new_client_id, DHCP_NAME_SIZE);
    }
    else
    {   /* clear */
        p_db_dhclient->client_flags = 0;
        sal_memset(p_db_dhclient->client_id, 0x0, DHCP_NAME_SIZE);
    }
    
    /* 3. update local CDB */
    rc = ds_dhclient_if_set_dhclient_if_field(p_db_if, p_db_dhclient, DS_DHCLIENT_IF_FLD_CLIENT_FLAGS);
    rc += ds_dhclient_if_set_dhclient_if_field(p_db_if, p_db_dhclient, DS_DHCLIENT_IF_FLD_CLIENT_ID);
    
    return rc;
}

static int32
_if_mgr_cmd_set_dhclient_class_id(char **argv, int32 argc, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    ds_dhclient_if_t *p_db_dhclient = NULL;
    int32 rc = PM_E_NONE;
    uint32 class_flags =0;

    char *port_class_id = argv[3];
    char *port_class_type = argv[4];

    /* 1. config check */
    p_db_dhclient = p_db_if->dhclient_if;
    if (NULL == p_db_dhclient)
    {
        CFG_CONFLICT_RET("Interface %s is not exist", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    if (NULL != port_class_id)
    {
        if (0 == sal_strcmp(port_class_type, "ascii") )
        {
            if (sal_strlen(port_class_id) > DHCP_NAME_MAX_LEN)
            {
                /* for bug 52212 modified by wus 2019-8-1 */
                CFG_CONFLICT_RET("The length of ASCII string should not exceed 63");
            }
            class_flags = GLB_DHCLIENT_FLAG_ASCII;
        }
        else
        {
            if (sal_strlen(port_class_id) > DHCP_NAME_MAX_LEN*2)
            {
                /* for bug 52212 modified by wus 2019-8-1 */
                CFG_CONFLICT_RET("The length of hex string should not exceed 126");
            }
            class_flags = GLB_DHCLIENT_FLAG_HEX;
        }

        /* 2. check duplicate config */
        if (0 == sal_strcmp(p_db_dhclient->class_id, port_class_id)
        && p_db_dhclient->class_flags == class_flags)
        {
            return PM_E_NONE;
        }
        p_db_dhclient->class_flags = class_flags;
        sal_strncpy(p_db_dhclient->class_id, port_class_id, DHCP_NAME_SIZE);
    }
    else
    {
        p_db_dhclient->class_flags = 0;
        sal_memset(p_db_dhclient->class_id, 0x0, DHCP_NAME_SIZE);
    }
    
    /* 3. update local CDB */
    rc = ds_dhclient_if_set_dhclient_if_field(p_db_if, p_db_dhclient, DS_DHCLIENT_IF_FLD_CLASS_FLAGS);
    rc += ds_dhclient_if_set_dhclient_if_field(p_db_if, p_db_dhclient, DS_DHCLIENT_IF_FLD_CLASS_ID);
    
    return rc;
}

static int32
_if_mgr_cmd_set_dhclient_lease(uint32 lease, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    ds_dhclient_if_t *p_db_dhclient = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_dhclient = p_db_if->dhclient_if;
    if (NULL == p_db_dhclient)
    {
        CFG_CONFLICT_RET("Interface %s is not exist", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    /* 2. check duplicate config */
    if (p_db_dhclient->lease == lease)
    {
        return PM_E_NONE;
    }

    p_db_dhclient->lease = lease;
        
    /* 3. update local CDB */
    rc = ds_dhclient_if_set_dhclient_if_field(p_db_if, p_db_dhclient, DS_DHCLIENT_IF_FLD_LEASE);
    
    return rc;
}

static int32
_if_mgr_cmd_set_dhclient_hostname(char *sys_port_hostname, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    ds_dhclient_if_t *p_db_dhclient = NULL;
    int32 rc = PM_E_NONE;
    int32 desc_len = 0;

    /* 1. config check */
    p_db_dhclient = p_db_if->dhclient_if;
    if (NULL == p_db_dhclient)
    {
        CFG_CONFLICT_RET("Interface %s is not exist", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    if (NULL == sys_port_hostname)
    {
        sal_memset(p_db_dhclient->hostname, 0, sizeof(p_db_dhclient->hostname));
    }
    else
    {
        desc_len = sal_strlen(sys_port_hostname);
        if (desc_len > COMM_NAME_SIZE)
        {
            CFG_INVALID_PARAM_RET("Hostname length should be [1, 64]");
        }
        rc = cdb_check_name(sys_port_hostname, COMM_NAME_SIZE, para->p_rs->desc);
        if (rc < 0)
        {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }

        /* 2. check duplicate config */
        if (0 == sal_strcmp(p_db_dhclient->hostname, sys_port_hostname))
        {
            return PM_E_NONE;
        }

        sal_memset(p_db_dhclient->hostname, 0, sizeof(p_db_dhclient->hostname));
        sal_strncpy(p_db_dhclient->hostname, sys_port_hostname, COMM_NAME_SIZE);
    }
    
    /* 3. update local CDB */
    rc = ds_dhclient_if_set_dhclient_if_field(p_db_if, p_db_dhclient, DS_DHCLIENT_IF_FLD_HOSTNAME);
    
    return rc;
}

#ifdef CONFIG_PVLAN

static int32
if_mgr_set_pvlan_secodary_check(uint32 pvlan_type, vid_t primary_vlan, vid_t community_vlan, cfg_cmd_para_t *para)
{
    tbl_pvlan_t  *p_db_pvlan = NULL;
    ds_pvlan_community_t *p_ds_pvlan = NULL;
    ds_pvlan_community_t p_community;
    tbl_pvlan_key_t  pvlan_db_key;
    int32 rc = PM_E_NONE;
    
    bool community_bmp_is_full = FALSE;
    
    if (GLB_IF_PVLAN_TYPE_COMMUNITY == pvlan_type && 0 == community_vlan)
    {
        CFG_CONFLICT_RET("Secondary vlan %u is invalid", community_vlan);
    }

    if (GLB_IF_PVLAN_TYPE_COMMUNITY == pvlan_type)
    {
        pvlan_db_key.primary_vid = primary_vlan;
        p_db_pvlan = tbl_pvlan_get_pvlan(&pvlan_db_key);
        if (p_db_pvlan)
        {
            GLB_BMP_CHECK_COMMUNITY_GROUP_IS_FULL(p_db_pvlan->community_group_id, community_bmp_is_full);
            if(community_bmp_is_full)
            {
                sal_memset(&p_community, 0, sizeof(ds_pvlan_community_t));
                p_community.key.community_vid = community_vlan; 
                p_ds_pvlan = ds_pvlan_community_get_pvlan_community(p_db_pvlan, &p_community);
                if(p_ds_pvlan == NULL)
                {
                    CFG_CONFLICT_RET("Only 16 secondary vlans can be set for private vlan %d", primary_vlan);
                }
            }
        }
    }
    return rc;
}

static int32
_if_mgr_cmd_process_interface_set_pvlan_mode(tbl_interface_t *p_if,  tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;
    
    /* 1. config check */
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port or LAG", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    if (GLB_IF_MODE_L2 != p_db_if->mode)
    {
        CFG_CONFLICT_RET("Interface %s is not in switch mode", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    if (p_db_if->isolate_group_id)
    {
        CFG_CONFLICT_RET("Conflict detected! Port-isolate is configured on this interface");
    }
    if (p_db_if->mlag_id)
    {
        CFG_CONFLICT_RET("Conflict detected! mlag is enabled on this interface");
    }
    if (switch_is_mlag_peer_link(p_db_if->ifindex))
    {
        CFG_CONFLICT_RET("Conflict detected! mlag peer-link is enabled on this interface");
    }

    /* modified by wangjj for fix erps bug 41389 41027, 2016-11-25 */
    if (p_db_if && p_db_if->erps_enable)
    {
        CFG_CONFLICT_RET("ERPS domain primary/secondary/edge/common/vpls interface mode can not be changed");
    }
    
    /* modified by liwh for bug 43751, 2018-02-08 */
    if (p_db_if->dot1x_enable)
    {
        CFG_CONFLICT_RET("Conflict detected! dot1x is enabled on this interface!");
    }
    /* liwh end */

    if (p_db_if->ip_source_type)
    {
        CFG_CONFLICT_RET("Interface is enabled IP source guard");
    }
    if (p_db_if->vlanclass_active_type)
    {
        CFG_CONFLICT_RET("Interface is enabled vlan classification");
    }
    if (p_db_if->l2pro_uplink)
    {
        CFG_CONFLICT_RET("Interface is configured as l2 protocol uplink port");
    }
    if (GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type)
    {
        CFG_CONFLICT_RET("Interface is configured as QinQ port");
    }
    if (p_db_if->brgif->vlan_translation_en)
    {
        CFG_CONFLICT_RET("Interface is configured as vlan translation enabled port");
    }

    /* 2. check the primary vlan and secondary vlan*/
    PM_E_RETURN(if_mgr_set_pvlan_secodary_check(p_if->pvlan_type, GLB_VLAN_DEFAULT_VID, 0, para));

    /* 3. get the pvlan type*/
    if (p_if->pvlan_type == GLB_IF_PVLAN_TYPE_HOST)
    {
        if ((p_db_if->pvlan_type == GLB_IF_PVLAN_TYPE_ISOLATED) || (p_db_if->pvlan_type == GLB_IF_PVLAN_TYPE_COMMUNITY))
        {
            p_if->pvlan_type = p_db_if->pvlan_type; 
        }
        else
        {
            p_if->pvlan_type = GLB_IF_PVLAN_TYPE_ISOLATED;
        }
    }
    else
    {
        p_if->pvlan_type = GLB_IF_PVLAN_TYPE_PROMISCUOUS;
    }
    
    /* 4. check duplicate config */
    if (p_if->pvlan_type == p_db_if->pvlan_type)
    {
        return PM_E_NONE;
    }
 
    /* 5. call API */
    rc = if_mgr_set_pvlan_mode(p_if, p_if->pvlan_type, TRUE);
    
    return rc;
        
}

static int32
_if_mgr_cmd_process_interface_set_pvlan(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t* p_vlan = NULL;
    int32 rc = PM_E_NONE;

    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);

    /* 1. config check */
    vlan_key.vid = p_if->private_vlan;
    p_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_vlan)
    {
        CFG_CONFLICT_RET("Vlan %u is not exist", p_if->private_vlan);
    }
    if (p_db_if->pvlan_type == GLB_IF_PVLAN_TYPE_NONE)
    {
        CFG_CONFLICT_RET("Port mode is not set to private-vlan");
    }
    if (p_if->pvlan_type)
    {
        if (p_if->pvlan_type != p_db_if->pvlan_type)
        {
            if (p_db_if->pvlan_type == GLB_IF_PVLAN_TYPE_PROMISCUOUS)
            {
                CFG_CONFLICT_RET("The port mode is promiscuous, can not set as isolate or community");
            }
            else if (p_if->pvlan_type == GLB_IF_PVLAN_TYPE_PROMISCUOUS)
            {
                CFG_CONFLICT_RET("Please select a type of isolate or community for the host port");
            }
        }
    }
    
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    /* 2. check the secondary vlan*/
    PM_E_RETURN(if_mgr_set_pvlan_secodary_check(p_if->pvlan_type, p_if->private_vlan, p_if->community_vlan, para));

    /* 3. check duplicate config */
    if ((p_db_if->private_vlan== p_if->private_vlan) 
        && (p_db_if->pvlan_type == p_if->pvlan_type)
        && (p_db_if->community_vlan == p_if->community_vlan))
    {
        return PM_E_NONE;
    }

    /* modified by liwh for bug 40620, 2017-01-22 */
    if (p_db_if->dot1x_cfg_guest_vlan == p_if->private_vlan)
    {
        CFG_CONFLICT_RET("Can't configure native VLAN same as guest VLAN");
    }
    /* liwh end */

    /* 4. call API */
    rc = if_mgr_set_pvlan(p_if, p_if->pvlan_type, p_if->private_vlan, p_if->community_vlan, TRUE);

    return rc;
}

#endif /* CONFIG_PVLAN */

#ifdef CONFIG_VLANCLASS
static int32 
_if_mgr_cmd_set_vlanclass_add_group_to_if(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;    
    char ifname_ext[IFNAME_EXT_SIZE];    
    tbl_vlanclass_group_key_t vclass_gkey;    
    tbl_vlanclass_group_t* p_db_vclass_group = NULL;

    /* 1. config check */
    if (GLB_IF_MODE_L2 != p_db_if->mode)
    {
        CFG_CONFLICT_RET("Interface %s is not switch port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    if ((p_if->vlanclass_active_type == GLB_VLAN_CLASS_PROTOCOL_BASED)          
        && (p_if->vlanclass_group_id != GLB_VLANCLASS_GROUP_ID_MAX))
    {
        CFG_CONFLICT_RET("Only Group 31 can be activated for protocol");
    }

    vclass_gkey.group_id = p_if->vlanclass_group_id;    
    p_db_vclass_group = tbl_vlanclass_group_get_vlanclass_group(&vclass_gkey);    
    if (NULL == p_db_vclass_group)    
    {        
        CFG_CONFLICT_RET("Group %d is not configured", p_if->vlanclass_group_id);    
    }

    if (p_db_if->ip_source_type)
    {       
        CFG_CONFLICT_RET("IP source guard is already enabled on this interface");
    }
    if (p_db_if->dot1x_enable)
    {
        CFG_CONFLICT_RET("Conflict detected! dot1x have already enabled on this interface");        
    }
    if (p_db_if->brgif->vlan_translation_en)
    {
        CFG_CONFLICT_RET("Interface %s is enabled vlan translation", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    if (GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type)
    {
        CFG_CONFLICT_RET("Interface %s is configured as QinQ port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    if (p_db_if->l2pro_uplink)
    {
        CFG_CONFLICT_RET("Interface %s is configured as l2 protocol uplink port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    /* modified by liwh for bug 53010, 2019-07-22 */
    if_mgr_clear_interface_cfm_meps(p_db_if);
    /* liwh end */

    /* 2. check duplicate config */    
    if ((p_if->vlanclass_group_id == p_db_if->vlanclass_group_id)        
        && (p_if->vlanclass_active_type == p_db_if->vlanclass_active_type))
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    rc = if_mgr_set_vlanclass_add_group_to_if(p_if, p_if->vlanclass_group_id, p_if->vlanclass_active_type); 
    
    return rc;
}

static int32 
_if_mgr_cmd_set_vlanclass_del_group_from_if(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;    
    char ifname_ext[IFNAME_EXT_SIZE];

    /* 1. config check */
    if (GLB_IF_MODE_L2 != p_db_if->mode)
    {
        CFG_CONFLICT_RET("Interface %s is not switch port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_if->vlanclass_active_type == p_db_if->vlanclass_active_type)
    {
        return PM_E_NONE;
    }

    /* 3. call API */    
    rc = if_mgr_set_vlanclass_del_group_from_if(p_if);  
    
    return rc;
}

static int32
_if_mgr_cmd_set_vlanclass_default_action(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;    
    char ifname_ext[IFNAME_EXT_SIZE];

    /* 1. config check */
    if (GLB_IF_MODE_L2 != p_db_if->mode)
    {
        CFG_CONFLICT_RET("Interface %s is not switch port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    if (p_db_if->ip_source_type)
    {       
        CFG_CONFLICT_RET("IP source guard is already enabled on this interface");
    }
    if (p_db_if->dot1x_enable)
    {
        CFG_CONFLICT_RET("Conflict detected! dot1x have already enabled on this interface");        
    }
    if (!(p_db_if->vlanclass_active_type == GLB_VLAN_CLASS_MAC_BASED) &&         
        !(p_db_if->vlanclass_active_type == GLB_VLAN_CLASS_IP_BASED))
    {
        if (p_if->vlanclass_default_action)
        {
            CFG_CONFLICT_RET("Please enable ip based vlan classifier or mac based vlan classsifier");
        }
    }

    /* 2. check duplicate config */
    if (p_if->vlanclass_default_action == p_db_if->vlanclass_default_action)
    {
        return PM_E_NONE;
    }

    /* 3. call API */    
    rc = if_mgr_set_vlanclass_default_action(p_if, p_if->vlanclass_default_action);
    
    return rc; 
}

#endif /* CONFIG_VLANCLASS */

#ifdef CONFIG_L2

static int32
_if_mgr_cmd_set_mode(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_pvlan_t  *p_db_pvlan = NULL;
    tbl_pvlan_key_t  pvlan_db_key;
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_openflow_interface_t openflow_intf;
    tbl_openflow_interface_t* p_openflow_intf = NULL;
    uint32 proto_index = 0;
    /* modified by liwh for bug 50959, 2019-01-29 */
    ctclib_slistnode_t *lnode = NULL;
    ctclib_slistnode_t *nnext = NULL;
    ds_circuit_id_t *pItem;
    /* liwh end */

    /* 1. config check */
#if 1
    if (GLB_IF_MODE_L2 == p_if->mode && GLB_IF_MODE_L3 == p_db_if->mode && p_db_if->inband_snat_type)
    {
        CFG_INVALID_PARAM_RET("'%s' already assigned to %s",
                              p_db_if->key.name,
                              INBAND_SNAT_TYPE_TO_STRING(p_db_if->inband_snat_type));
    }
#endif

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("This command is forbidden on LAG interface");
    }

    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    if (p_db_if && p_db_if->brgif && p_db_if->brgif->mlag_is_peer_link)
    {
        CFG_CONFLICT_RET("Cannot configure for MLAG peer-link has been configured");
    }
    
    if ((GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG)))
    {
        CFG_CONFLICT_RET("Cannot configure mirror destination port");
    }

    if ((p_db_if->erps_enable) && (GLB_IF_MODE_L2 != p_if->mode))
    {
        CFG_CONFLICT_RET("This interface is already used by ERPS");
    }

    /* modified by liwh for bug 40053, 2016-08-10 */
    if (p_db_if && p_db_if->dot1x_enable)
    {
        CFG_CONFLICT_RET("Conflict detected! dot1x is enabled on this interface!");
    }
    /* liwh end */

    /* 2. check duplicate config */
    if (p_db_if->mode == p_if->mode)
    {
        return PM_E_NONE;
    }
    if(GLB_IF_MODE_L3 == p_if->mode)
    {
        sal_memset(&openflow_intf, 0, sizeof(openflow_intf));
        sal_strncpy(openflow_intf.key.name, p_db_if->key.name, IFNAME_SIZE);
            /* check exist */
        p_openflow_intf = tbl_openflow_interface_get_openflow_interface(&openflow_intf.key);
        if(p_openflow_intf)
        {
            if(p_openflow_intf->ingress_add_native_vlan_enable)
            {
                CFG_CONFLICT_RET("Conflict! if port set ingress add native vlan enable, should not set no switchport!");
            }
        }
    }
    if (GLB_IF_MODE_L3 == p_if->mode && GLB_IF_MODE_L3 != p_db_if->mode)
    {
        if (p_db_if->current_g8032_ring_count)
        {
            CFG_FUNC_ERR_RET("G8032 has been configured, should not set no switchport ");
        }
    }
    if (GLB_IF_MODE_L3 == p_if->mode)
    {
        _if_mgr_cmd_set_aclqos_if_replace_cos(p_db_if, FALSE);
        if (p_db_if->ip_source_type)
        {
            rc = ipsour_port_mode_update(IP_NONE, p_db_if);
        }      
    }
    if (p_db_if->pvlan_type != GLB_IF_PVLAN_TYPE_NONE)
    {
        pvlan_db_key.primary_vid = p_db_if->private_vlan;
        p_db_pvlan = tbl_pvlan_get_pvlan(&pvlan_db_key);
        if(p_db_pvlan != NULL)
        {
            if(GLB_IF_PVLAN_TYPE_COMMUNITY == p_db_if->pvlan_type)
            {
                rc = if_mgr_unmapping_community_vlan_to_group(p_db_if, p_db_if->community_vlan);
            }
            else if(GLB_IF_PVLAN_TYPE_ISOLATED == p_db_if->pvlan_type)
            {
                PM_E_RETURN(tbl_pvlan_set_isloate_ports(p_db_pvlan, FALSE, p_db_if));
            }
            else if(GLB_IF_PVLAN_TYPE_PROMISCUOUS == p_db_if->pvlan_type)
            {
                PM_E_RETURN(tbl_pvlan_set_promiscuous_ports(p_db_pvlan, FALSE, p_db_if));
            }

            if((p_db_pvlan->community_groups.obj_list->count == 0) 
            && (p_db_pvlan->isloate_ports.obj_list->count == 0)
            && (p_db_pvlan->promiscuous_ports.obj_list->count == 0))
            {
                PM_E_RETURN(tbl_pvlan_del_pvlan(&pvlan_db_key));
            }
         }
         PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_PVLAN_TYPE));
         PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_PRIVATE_VLAN));
         PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_COMMUNITY_VLAN));
         PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_PVLAN_GROUP_ID));
    }

    if (GLB_IF_MODE_L3 == p_if->mode)
    {
        /*del the vlanclass on interface*/
        if (p_db_if->vlanclass_active_type)
        {
            PM_E_RETURN(if_mgr_set_vlanclass_del_group_from_if(p_db_if));
        }

        /*bug:43867, if l2 port change to routed port, set dhcp snooping trust disable*/
        if (TRUE == p_db_if->dhcp_snooping_trust)
        {
            p_if->dhcp_snooping_trust = FALSE;
            PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_DHCP_SNOOPING_TRUST));
        }

        /*Added by yejl for l2protocol*/
        /*set the l2pro_uplink to false*/
        if (p_db_if->l2pro_uplink)
        {   
            p_if->l2pro_uplink = FALSE;
            PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_L2PRO_UPLINK));
        }
        /*del the ds_l2protocol*/
        if (p_db_if->l2protocol.obj_list->count != 0)
        {
            PM_E_RETURN(if_mgr_del_ds_l2protocol(p_db_if));
            for (proto_index = 0; proto_index < GLB_L2_PROTO_INDEX_MAX; proto_index++)
            {
                p_db_if->l2pro_if_action_vlan.l2action[proto_index] = GLB_L2_PROTO_PEER; 
                p_db_if->l2pro_if_action_vlan.l2vlan[proto_index] = GLB_VLAN_NONE; 
            }
            PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_L2PRO_IF_ACTION_VLAN));
        }
        
        /*ended by yejl*/

        /* modified by liwh for bug 50959, 2019-01-29 */
        CTCLIB_SLIST_LOOP_DEL(p_db_if->circuit_id.obj_list, pItem, lnode, nnext) {
            ds_circuit_id_del_circuit_id(p_db_if, pItem);
        }
        /* liwh end */

        /* modified by liwh for bug 53010, 2019-07-22 */
        if_mgr_clear_interface_cfm_meps(p_db_if);
        /* liwh end */
    }
 
    /* 3. call API */
    PM_E_RETURN(if_mgr_set_mode(p_if, p_if->mode, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_set_vlan_type(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port or LAG", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    if (GLB_IF_MODE_L2 != p_db_if->mode)
    {
        CFG_CONFLICT_RET("Interface %s is not in switch mode", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }

    if (p_db_if && p_db_if->brgif && p_db_if->brgif->mlag_is_peer_link)
    {
        CFG_CONFLICT_RET("Cannot configure for MLAG peer-link has been configured");
    }

    /* modified by wangjj for fix erps bug 41389 41027, 2016-11-25 */
    if (p_db_if && p_db_if->erps_enable && GLB_VLAN_PORT_TYPE_TRUNK != p_if->vlan_type)
    {
        CFG_CONFLICT_RET("ERPS domain primary/secondary/edge/common/vpls interface mode can not be changed");
    }

    if (p_db_if && p_db_if->dot1x_enable)
    {
        CFG_CONFLICT_RET("Conflict detected! dot1x is enabled on this interface!");
    }

    if ((p_db_if->erps_enable) 
        && (GLB_VLAN_PORT_TYPE_ACCESS == p_if->vlan_type))
    {
        CFG_CONFLICT_RET("This interface is already used by ERPS");
    }
    /*added by yejl for l2protocol uplink*/
    if (p_db_if->l2pro_uplink && (GLB_VLAN_PORT_TYPE_TRUNK != p_if->vlan_type))
    {
        CFG_CONFLICT_RET("This interface is already l2protocol uplink port, only trunk mode can be configured");    
    }

    if (GLB_VLAN_PORT_TYPE_QINQ == p_if->vlan_type)
    {
        if (p_db_if->ip_source_type)
        {
            CFG_CONFLICT_RET("IP source guard is enabled on this port");            
        }
        if (p_db_if->vlanclass_active_type)
        {
            CFG_CONFLICT_RET("Interface is enabled vlan classification");
        }
        if (p_db_if->l2pro_uplink)
        {
            CFG_CONFLICT_RET("Interface is configured as l2 protocol uplink port");
        }
        if (p_db_if->brgif->vlan_translation_en)
        {
            CFG_CONFLICT_RET("Interface is now configured for vlan translation");
        }
        if (GLB_IF_PVLAN_TYPE_NONE != p_db_if->pvlan_type)
        {
            CFG_CONFLICT_RET("Interface has configured as a private VLAN");
        }
    }

    /* modified by liwh for bug 53621, 2019-09-19 */
    if (p_if->vlan_type != p_db_if->vlan_type)
    {
        if_mgr_clear_interface_cfm_meps(p_db_if);
    }
    /* liwh end */
    
#ifdef OFPRODUCT
    if (GLB_VLAN_PORT_TYPE_QINQ == p_if->vlan_type)
    {
        CFG_INVALID_PARAM_RET("Hybrid system unsupport to set QinQ port!");
    }
#endif

    /* 2. check duplicate config */
    if ((p_db_if->vlan_type == p_if->vlan_type) && 
        (p_db_if->pvlan_type == GLB_IF_PVLAN_TYPE_NONE))
    {
        return PM_E_NONE;
    }
    
    if (GLB_VLAN_PORT_TYPE_ACCESS == p_if->vlan_type)
    {
        _if_mgr_cmd_set_aclqos_if_replace_cos(p_db_if, FALSE);
    }
    else if (GLB_VLAN_PORT_TYPE_TRUNK == p_if->vlan_type)
    {
        _if_mgr_cmd_set_aclqos_if_replace_cos(p_db_if, TRUE);
    }
    /* 3. call API */
    PM_E_RETURN(if_mgr_set_vlan_type(p_if, p_if->vlan_type, TRUE));
    return rc;
}
#endif /* CONFIG_L2 */

static int32
_if_mgr_cmd_set_up(tbl_interface_t* p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    tbl_interface_t     *pst_if_lag     = NULL;
    tbl_interface_key_t if_key;
    uint32              agg_if_up = 0;      
    uint32              enable = 0;
    uint32              shutdown = FALSE;
    uint32              old_if_admin_up = FALSE;
    uint32              old_if_up = FALSE;
    uint32              new_if_up = FALSE;
    /* support vrrp priority 0 advt modified by liwh for bug 45503, 2017-10-30 */
    char                sz_fullcmd[GLB_MAX_CMD_BUFSZ] = {0};
    /* liwh end */

    int32 rc = PM_E_NONE;

    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);

    /* 1. if system startup done, need check duplicate config */
    if (swth_sys_is_startup_done())
    {
        old_if_admin_up = p_db_if->admin_up;
        old_if_up = tbl_interface_is_up(p_db_if);
        new_if_up = tbl_interface_is_up(p_if);

        if (old_if_admin_up == new_if_up)
        {
            if (old_if_admin_up == old_if_up)
            {
                if (!INTERFACE_IS_LAG_GROUP(p_db_if))
                {
                    return PM_E_NONE;
                }
            }
        }
    }

    /* 2. config check */
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG))
    {
        CFG_CONFLICT_RET("Cannot configure mirror destination port");
    }

    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        sal_memset(&if_key, 0, sizeof(if_key));
        sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_db_if->lag->lag_id);
        pst_if_lag = tbl_interface_get_interface(&if_key);
        if (NULL == pst_if_lag)
        {
            SAL_ASSERT(0);
        }
        
        agg_if_up = tbl_interface_is_up(pst_if_lag);        

        if (new_if_up && !agg_if_up)
        {
            CFG_CONFLICT_RET("Interface %s is admin down, can't no shutdown", pst_if_lag->key.name);
        }
    }    
    
    /* load startup config, not no shutdown */
    if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        if (!swth_sys_is_startup_done())
        {
            shutdown = (tbl_interface_is_up(p_if)) ? FALSE : TRUE;
            if (shutdown)
            {
                GLB_SET_FLAG(p_db_if->flags, GLB_IFF_CFG_SHUTDOWN);
            }
            return PM_E_NONE;
        }
    }

    enable = (GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP)) ? TRUE : FALSE;

    /* support vrrp priority 0 advt modified by liwh for bug 45503, 2017-10-30 */
    if (!enable && swth_sys_is_startup_done() && (GLB_IF_MODE_L3 == p_db_if->mode))
    {
        snprintf(sz_fullcmd, GLB_MAX_CMD_BUFSZ, "cdbctl update/cdb/l3/l3if_pre_down/%s", 
            p_db_if->key.name);
    
        sal_system(sz_fullcmd);
    }
    /* liwh end */

    /* 3. call API */
    PM_E_RETURN(if_mgr_set_up(p_if, enable, TRUE, TRUE));

    /*fix bug 49041, shutdown then no shutdown lead to frr not send netlink msg*/
    usleep(100000);
    
    return rc;
}

static int32
_if_mgr_cmd_set_mtu(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;
    tbl_route_if_t *p_db_rt_if = NULL;

    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    
    /* 1. config check */
    if (GLB_IF_MODE_L3 != p_db_if->mode)
    {
        CFG_CONFLICT_RET("Interface %s is not in routed mode", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    p_db_rt_if = tbl_route_if_get_route_if_by_name(p_if->key.name);
    if (p_if->mtu < GLB_IPV6_MIN_MTU && p_db_rt_if && p_db_rt_if->ifc_ipv6.obj_list->count > 0)
    {
        CFG_CONFLICT_RET ("MTU must be at least 1280 on an interface which configured ipv6 address");
    }
    
    /* 2. check duplicate config */
    if (p_db_if->mtu == p_if->mtu)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(if_mgr_set_mtu(p_db_if, p_if->mtu, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_set_ipsg_enable(char **argv, int32 argc, tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char    ifname_ext[IFNAME_EXT_SIZE];
    uint32  ipsg_type   = IP_NONE;
    int32   rc          = PM_E_NONE;

    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    
    /* 1. config check */
    if (GLB_IF_MODE_L2 != p_db_if->mode)
    {
        CFG_CONFLICT_RET("IP source guard only supports switch port");
    }
    
    if ((GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type) && (NULL != p_db_if->lag || NULL != p_db_if->lacp))
    {
        CFG_CONFLICT_RET("This port has been added to a aggregation port");
    }

    if (GLB_FLAG_ISSET(p_db_if->dot1x_flag, GLB_IF_DOT1X_MODE_MAC))
    {
        CFG_INVALID_PARAM_RET("Interface is now configured for 802.1x mac based");
    }   

    /* data trans */
    sal_memset(ifname_ext, 0, sizeof(ifname_ext));
    sal_strncpy(ifname_ext, argv[0], sizeof(ifname_ext));
    if (4 == argc)
    {
        if (0 == sal_strcmp(argv[2], "ipsg_enable"))
        {
            if (0 == sal_strcmp(argv[3], "ip"))
            {
                ipsg_type = IP_FILTERING;
            }

            if (0 == sal_strcmp(argv[3], "ip-mac"))
            {
                ipsg_type = IP_MAC_FILTERING;
            }

            if (0 == sal_strcmp(argv[3], "ip-vlan"))
            {
                ipsg_type = IP_VLAN_FILTERING;
            }

            if (0 == sal_strcmp(argv[3], "ip-mac-vlan"))
            {
                ipsg_type = IP_MAC_VLAN_FILTERING;
            }
        }
    
        /* check duplicate config */
        if (ipsg_type && p_db_if->ip_source_type == ipsg_type)
        {
            CFG_CONFLICT_RET("IP source guard is already enabled on this port");
        }

        if (p_db_if->brgif->vlan_translation_en)
        {
            CFG_CONFLICT_RET("Interface %s is enabled vlan translation", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
        if (GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type)
        {
            CFG_CONFLICT_RET("Interface %s is configured for QinQ port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
        if (p_db_if->vlanclass_active_type)
        {
            CFG_CONFLICT_RET("Interface is enabled vlan classification");
        }
        if (p_db_if->l2pro_uplink)
        {
            CFG_CONFLICT_RET("Interface is configured as l2 protocol uplink port");
        }
        if (GLB_IF_PVLAN_TYPE_NONE != p_db_if->pvlan_type)
        {
            CFG_CONFLICT_RET("Interface has configured as a private VLAN");
        }

        if (IP_NONE == p_db_if->ip_source_type)
        {
            /* first enable */
            
        }
        else
        {
            /* type change and check and update */
        }

        rc = ipsour_port_enable(ipsg_type, p_db_if);

    }
    else if (3 == argc)
    {
        if (0 == sal_strcmp(argv[2], "ipsg_disable"))
        {
            ipsg_type = IP_NONE;
        }

        if (IP_NONE == p_db_if->ip_source_type)
        {
            /* already disable, log it */
        }
        else
        {
            /* to disable */
        }

        rc = ipsour_port_disable(ipsg_type, p_db_if);
    }
    else
    {
        return PM_E_NONE;
    }

    return rc;
}

static int32
_if_mgr_cmd_set_bandwidth(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 bandwidth = 0;

    /* 1. config check */
    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    /* 2. check duplicate config */
    if (p_db_if->bandwidth == p_if->bandwidth)
    {
        /*added by huwx for bug 42467, 2017-01-10*/
        if (0 == p_db_if->bandwidth)
        {
            PM_E_RETURN(if_mgr_get_default_bandwidth(p_db_if, &bandwidth));
            p_db_if->oper_bandwidth = bandwidth;
            PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_OPER_BANDWIDTH));
        }
        /*end added*/

        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(if_mgr_set_bandwidth(p_if));
    return rc;
}

static int32
_if_mgr_cmd_set_lag_param_max_active_num(char **argv, int32 argc, cfg_cmd_para_t *para, tbl_interface_t *p_db_if)
{
    uint32 value = 0;
    int32 rc = 0;
    if(GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
         CFG_CONFLICT_RET("This cli should only set in agg port");
    }

     PM_CFG_GET_UINT(value, argv[3], para->p_rs);
     if(value == p_db_if->lag->max_active_number)
     {
        return rc;
     }

     _if_active_backup_agg_oper(p_db_if, NULL, IF_AGG_ACTIVE_BACKUP_OP_CHANGE_MAX_NUMBER, value, NULL);

     return rc;
}



static int32
_if_mgr_cmd_add_if_to_lag(char **argv, int32 argc, cfg_cmd_para_t *para, tbl_interface_t *p_db_if)
{
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if_lag = NULL;
    uint32 value = 0;
    ds_lag_t lag;
    ds_lag_t *p_db_lag = NULL;
#ifdef CONFIG_L2
 //   ds_brgif_t* p_brgif = NULL;
#endif /* CONFIG_L2 */
    char ifname_ext[IFNAME_EXT_SIZE];
    uint32 member_count = 0;
    int32 rc = PM_E_NONE;
    tbl_openflow_interface_key_t openflow_if_key;
    tbl_openflow_interface_t* p_openflow_if = NULL;
    uint32 agg_member_priority = 0;

   /*apply acl can't add to agg*/
   /*added by yejl for IP/IPv6/MAC acl check*/
    if (0 != sal_strlen(p_db_if->igs_acl) || 0 !=sal_strlen(p_db_if->egs_acl))
    {
        CFG_CONFLICT_RET("Interface %s is already applied with ip access-list", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    if (0 != sal_strlen(p_db_if->igs_ipv6_acl) || 0 !=sal_strlen(p_db_if->egs_ipv6_acl))
    {
        CFG_CONFLICT_RET("Interface %s is already applied with ipv6 access-list", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    if (0 != sal_strlen(p_db_if->igs_mac_acl) || 0 !=sal_strlen(p_db_if->egs_mac_acl))
    {
        CFG_CONFLICT_RET("Interface %s is already applied with mac access-list", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    /*ended by yejl*/
    
    if (0 != sal_strlen(p_db_if->igs_policy_map) || 0 !=sal_strlen(p_db_if->egs_policy_map))
    {
        CFG_CONFLICT_RET("Interface %s is already applied with service policy map", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    /* tap group interface can't add to agg */
    if (0 != p_db_if->tap_group_igs_ref_cnt || 0 != p_db_if->tap_group_egs_ref_cnt)
    {
        CFG_CONFLICT_RET("Interface %s is already applied with tap group", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    sal_memset(&lag, 0, sizeof(lag));
    //PM_CFG_CHECK_EXPECT_PARAMS(argc, 6, para->p_rs);
    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
    lag.lag_id = value;
    PM_CFG_GET_ENUM(value, glb_agg_mode_strs, GLB_IF_AGG_MAX, argv[4], para->p_rs);
    lag.mode = value;
    if(6 == argc)
    {
        PM_CFG_GET_UINT(agg_member_priority, argv[5], para->p_rs);
    }

    p_db_lag = p_db_if->lag;
        /* 1. config check */
    if (NULL != p_db_lag)
    {
        if(p_db_lag->lag_id != lag.lag_id )
        {
            CFG_CONFLICT_RET("Interface %s has been added into LAG %u", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_lag->lag_id);
        }
        if(p_db_lag->mode != lag.mode)
        {
            CFG_CONFLICT_RET("Interface %s has been added into LAG %u in different mode", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_lag->lag_id);
        }
        rc = _if_active_backup_agg_oper(NULL, p_db_if, IF_AGG_ACTIVE_BACKUP_OP_CHANGE_MEM_PRIORITY, agg_member_priority, NULL);
        return rc;
    }

    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", lag.lag_id);

    /* modified by liwh for bug 39388, 2016-08-29 */
    if (lag.lag_id > GLB_LAG_GROUP_NUM)
    {
        CFG_CONFLICT_RET("Max LAG group id should be less than %d", GLB_LAG_GROUP_NUM+1);
    }
    /* liwh end */

    /* lookup whether the first add */
    p_db_if_lag = tbl_interface_get_interface(&if_key);
    if (p_db_if_lag && p_db_if_lag->lag)
    {
        /* added by yejl to fix bug37750,2016-06-27 */
        if (GLB_FLAG_ISSET(p_db_if_lag->mirror_enable, MIRROR_DEST_FLAG)) 
        {
            CFG_CONFLICT_RET("The aggregator is the mirror destination interface");
        }
        
        /* added by yejl to fix bug41787,2016-12-01 */
        if (p_db_if_lag->vlanclass_active_type)
        {
            CFG_CONFLICT_RET("Vlanclass have already enabled on this aggregator, can't add member port");
        }
        
        member_count = CTCLIB_SLISTCOUNT(p_db_if_lag->lag->member_ports.obj_list);
        if (member_count >= GLB_LINKAGG_MAX_MEMBER)
        {
            CFG_CONFLICT_RET("%s has already configured max %u member ports", if_key.name, GLB_LINKAGG_MAX_MEMBER);
        }
    }
#ifndef TAPPRODUCT
    if (p_db_if->aclqos_if)
    {
        if ((p_db_if->aclqos_if->input_policy_map[0] != '\0') || (p_db_if->aclqos_if->output_policy_map[0] != '\0'))
        {
            CFG_CONFLICT_RET("This interface is already applied with policy-map");
        }
    }
#endif

    if (p_db_if->sflow_counter_en)
    {
        
        CFG_CONFLICT_RET("The port has been configured sflow related configuration,not allowed to join agg ");
    }
    
    if (p_db_if->dot1x_enable)
    {
        CFG_CONFLICT_RET("Conflict detected! dot1x have already enabled on this interface");        
    }
    if (p_db_if->vlanclass_active_type)
    {
        CFG_CONFLICT_RET("Conflict detected! vlanclass have already enabled on this interface");        
    }

    sal_strncpy(openflow_if_key.name, p_db_if->key.name, IFNAME_SIZE);
    p_openflow_if = tbl_openflow_interface_get_openflow_interface(&openflow_if_key);
    if(p_openflow_if)
    {
        if(p_openflow_if->openflow_enable)
        {
            CFG_CONFLICT_RET("This interface is already enable openflow");
        }
    }

    if (switch_is_mlag_peer_link(p_db_if->ifindex))
    {
        if (GLB_AGG_MODE_STATIC == lag.mode)
        {
            CFG_CONFLICT_RET("Can't configure static-channel-group on mlag peer-link interface");
        }
        else
        {
            CFG_CONFLICT_RET("Can't configure channel-group on mlag peer-link interface");
        }
    }

    if (p_db_if->erps_enable)
    {
        CFG_CONFLICT_RET("This interface is already used by ERPS");
    }

    if (p_db_if->ip_source_type)
    {
        CFG_CONFLICT_RET("IP source guard is already enabled on this port");
    }

    if (p_db_if->dhcp_snooping_trust)
    {
        CFG_CONFLICT_RET("Conflict with dhcp snooping trust set on this interface");
    }
    /* added by yangl for g8032,2019-06-28 */
    if (p_db_if->current_g8032_ring_count)
    {
        CFG_CONFLICT_RET("Conflict with g8032 set on this interface");
    }
    PM_E_RETURN(if_agg_add_agg(&lag, para, p_db_if, agg_member_priority));

    return PM_E_NONE;
}

static int32
_if_mgr_cmd_del_if_from_lag(char **argv, int32 argc, cfg_cmd_para_t *para, tbl_interface_t *p_db_if)
{
    ds_lag_t *p_db_lag = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;
    tbl_interface_t     *p_db_if_lag = NULL;
    tbl_interface_key_t if_key;
    tbl_openflow_interface_key_t openflow_if_key;
    tbl_openflow_interface_t* p_openflow_if = NULL;
    char                sz_fullcmd[GLB_MAX_CMD_BUFSZ] = {0};
    /* modified by liwh for bug 50219, 2018-12-19 */
    tbl_route_if_t               *p_db_rtif = NULL;
    tbl_route_if_t                rtif;
    /* liwh end */

    p_db_lag = p_db_if->lag;

    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
    
    /* 1. config check */
    if (NULL == p_db_lag)
    {
        /* port is not bound to LAG */
        return PM_E_NONE;        
    }

    if (GLB_AGG_MODE_STATIC == p_db_lag->mode)
    {
        if (0 != sal_strcmp(glb_agg_mode_strs[GLB_AGG_MODE_STATIC], argv[2]))
        {
            CFG_CONFLICT_RET("Interface %s is static LAG's member port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
    }
    else
    {
        if (0 == sal_strcmp(glb_agg_mode_strs[GLB_AGG_MODE_STATIC], argv[2]))
        {
            CFG_CONFLICT_RET("Interface %s is dynamic LAG's member port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
    }

    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_db_if->lag->lag_id);
    p_db_if_lag = tbl_interface_get_interface(&if_key);
    if (p_db_if_lag)
    {
        /* added by yejl to fix bug37750,2016-06-27 */
        if (GLB_FLAG_ISSET(p_db_if_lag->mirror_enable, MIRROR_DEST_FLAG)) 
        {
            CFG_CONFLICT_RET("The aggregator is the mirror destination interface");
        }
        if ((p_db_if_lag->erps_enable)
            && p_db_if_lag->lag && p_db_if_lag->lag->member_ports.obj_list
            && (1 == p_db_if_lag->lag->member_ports.obj_list->count))
        {
            CFG_CONFLICT_RET("The aggregator is already configured as specific ERPS domain interface, can't delete the last member");
        }

        if ((0 != p_db_if_lag->tap_group_igs_ref_cnt || 0 != p_db_if_lag->tap_group_egs_ref_cnt)
            && p_db_if_lag->lag && p_db_if_lag->lag->member_ports.obj_list
            && (1 == p_db_if_lag->lag->member_ports.obj_list->count))
        {
            CFG_CONFLICT_RET("Interface %s is already applied with tap group,can't delete the last member", 
                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }

        /* modified by liwh for bug 49282, 2018-09-27 */
        if (p_db_if_lag->lag && p_db_if_lag->lag->member_ports.obj_list
            && (1 == p_db_if_lag->lag->member_ports.obj_list->count)
            && swth_sys_is_startup_done() && (GLB_IF_MODE_L3 == p_db_if_lag->mode))
        {
            snprintf(sz_fullcmd, GLB_MAX_CMD_BUFSZ, "cdbctl update/cdb/l3/l3if_pre_down/%s", 
            p_db_if_lag->key.name);
    
            sal_system(sz_fullcmd);
        }
        /* liwh end */

        /* modified by liwh for bug 50219, 2018-12-19 */
        if (p_db_if_lag->lag && p_db_if_lag->lag->member_ports.obj_list
            && (1 == p_db_if_lag->lag->member_ports.obj_list->count)
            && swth_sys_is_startup_done() && (GLB_IF_MODE_L3 == p_db_if_lag->mode))
        {
            sal_memset(&rtif, 0, sizeof(tbl_route_if_t));
            sal_strncpy(rtif.key.name, p_db_if_lag->key.name, IFNAME_SIZE);
            p_db_rtif = tbl_route_if_get_route_if(&rtif.key);
            if (p_db_rtif)
            {
                if  (p_db_rtif->dhcp_client_enable)
                {
                    snprintf(sz_fullcmd, GLB_MAX_CMD_BUFSZ, "cdbctl update/cdb/dhcp/dhcp_l3if_pre_delete/%s", 
                        p_db_if_lag->key.name);
                    sal_system(sz_fullcmd);
                }

                if  (p_db_rtif->dhcpv6_client_enable)
                {
                    snprintf(sz_fullcmd, GLB_MAX_CMD_BUFSZ, "cdbctl update/cdb/dhcp/dhcpv6_l3if_pre_delete/%s", 
                        p_db_if_lag->key.name);
                    sal_system(sz_fullcmd);
                }
            }
        }
        /* liwh end */    
    }

    sal_memset(&openflow_if_key, 0, sizeof(openflow_if_key));
    sal_snprintf(openflow_if_key.name, IFNAME_SIZE, "agg%u", p_db_lag->lag_id);
    p_openflow_if = tbl_openflow_interface_get_openflow_interface(&openflow_if_key);
    if(p_openflow_if)
    {
        if(p_openflow_if->openflow_enable)
        {
                /*hw_mac_addr not care about L2/L3*/
                if (0 == sal_memcmp(p_db_if_lag->hw_mac_addr, p_db_if->hw_mac_addr, MAC_ADDR_LEN))
                {
                    CFG_CONFLICT_RET("The agg's mac is come from this member port, you should disable openflow first");
                }
        }
    }
    if(1 == CDB_REF_LIST_COUNT(&p_db_if_lag->lag->member_ports))
    {
        if(p_openflow_if)
        {
            if(p_openflow_if->openflow_enable)
            {
                CFG_CONFLICT_RET("The aggregator is already enable openflow, remove the last member should disable openflow first");
            }
        }

        rc = acl_is_redirected_interface(p_db_if_lag);
        if (rc < 0)
        {
            CFG_CONFLICT_RET("The aggregator is the redirect interface for ACL");
        }
    }
    
    PM_E_RETURN(if_agg_del_agg(p_db_if));
    
    return PM_E_NONE;
}

int32
_if_mgr_check_ecmp_load_balance_type_with_agg(cfg_cmd_para_t *para)
{
    tbl_ecmp_global_t     *p_db_ecmp_glb = NULL;

    p_db_ecmp_glb = tbl_ecmp_global_get_ecmp_global();
    if (!p_db_ecmp_glb)
    {
        CFG_CONFLICT_RET("Tbl_ecmp_global not found");
    }
    
    if (p_db_ecmp_glb->load_balance_mode == GLB_ECMP_DYNAMIC_LOAD_BALANCE
        || p_db_ecmp_glb->load_balance_mode == GLB_ECMP_SELF_HEALING_LOAD_BALANCE)
    {
        CFG_CONFLICT_RET("It doesn't support configuring linkagg when ecmp mode is %s",
                        p_db_ecmp_glb->load_balance_mode == GLB_ECMP_DYNAMIC_LOAD_BALANCE ? "dlb":"self-healing");
    }

    return PM_E_NONE;
}

static int32
_if_mgr_cmd_process_interface_lag(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_interface_t intf;
    tbl_interface_t *p_db_if = NULL;
    tbl_route_if_key_t stKey;
    tbl_route_if_t *pItem = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;
    uint32 field_id = 0;
    cdb_field_t *p_field = NULL;
    cdb_node_t* p_node = NULL;

    sal_memset(&intf, 0, sizeof(intf));
    sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);

    /* check exist */
    p_db_if = tbl_interface_get_interface(&intf.key);
    if (!p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s not found", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
    }
    if(!((GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type) && (CDB_OPER_SET == para->oper)))
    {
        if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
        {
            CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
        }
    }

    /* modified by liul for bug 41675, check Conflict 2016-11-16 */
    memset(&stKey, 0, sizeof(stKey));
    sal_strncpy(stKey.name, argv[0], IFNAME_SIZE);
    if ((pItem = tbl_route_if_get_route_if(&stKey)) != NULL) 
    {
        if (0 != pItem->dhcp_server_group)
        {
            CFG_CONFLICT_RET("Conflict with dhcp relay function enabled on this interface");
        }

        if (0 != pItem->dhcp_client_enable)
        {
            CFG_CONFLICT_RET("Conflict with dhcp client function enabled on this interface");
        }

        /* modified by liwh for bug 50533, 2018-12-25 */
        if (0 != pItem->dhcpv6_client_enable)
        {
            CFG_CONFLICT_RET("Conflict with dhcpv6 client function enabled on this interface");
        }
        /*liwh end */
    }
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        /*added by chenc for bug 52404 in 2019-06-03*/
        rc = _if_mgr_check_ecmp_load_balance_type_with_agg(para);
        if(0 != rc)
        {
            return rc;
        }
        /*chenc end*/
        
        rc = _if_mgr_cmd_add_if_to_lag(argv, argc, para, p_db_if);
        break;

    case CDB_OPER_DEL:
        rc = _if_mgr_cmd_del_if_from_lag(argv, argc, para, p_db_if);
        break;

    case CDB_OPER_SET:
        p_node = cdb_get_ds(DS_LAG);
        
        field_id = cdb_get_field_id(argv[2], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[2]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }
        if(field_id == DS_LAG_FLD_MAX_ACTIVE_NUMBER)
        {
            if(p_db_if->lag)
            {
                if(p_db_if->lag->mode != GLB_AGG_MODE_STATIC)
                {
                    CFG_INVALID_PARAM_RET("This cli should only be set in static agg mode");
                }
            }
            rc = _if_mgr_cmd_set_lag_param_max_active_num(argv, argc, para, p_db_if);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Not support set action");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
    }

    return rc;        
}

static int32
_if_mgr_cmd_process_interface_set_lacp(char **argv, int32 argc, cfg_cmd_para_t *para, tbl_interface_t *p_db_if)
{
    cdb_node_t* p_node = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    uint16 port_priority = 0;
    uint32 enable = FALSE;
    int32 value = 0;
    uint32 i = 2;
    int32 rc = PM_E_NONE;

    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    p_node = cdb_get_ds(DS_LACP);
    field_id = cdb_get_field_id(argv[i], p_node, &p_field);
    if (field_id < 0)
    {
        CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
    }
    if (CDB_ACCESS_RD == p_field->access)
    {    
        CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
    }

    switch (field_id)
    {
    case DS_LACP_FLD_ACTOR_PORT_PRIORITY:
        PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
        port_priority = value;
        rc = _if_mgr_cmd_set_lacp_port_priority(port_priority, p_db_if, para);
        break;
    case DS_LACP_FLD_SHORT_TIMEOUT:
        PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
        enable = value;
        rc = _if_mgr_cmd_set_lacp_port_short_timeout(enable, p_db_if, para);
        break;
    default:
        CFG_FUNC_ERR_RET("Unknown support changed field %s", argv[i]);
        break;
    }

    return rc;
}

static int32
_if_mgr_cmd_set_dhclient_iana_t1(int32 argc, char **argv,  tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    ds_dhclient_if_t *p_db_dhclient = NULL;
    int32 rc = PM_E_NONE;
    uint32 t1 = 0;
    uint32 t2 = 0;

    if (FALSE == acl_stm_is_ipv6_profile())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }  
    
    /* 1. config check */
    p_db_dhclient = p_db_if->dhclient_if;
    if (NULL == p_db_dhclient)
    {
        CFG_CONFLICT_RET("Interface %s is not exist", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    PM_CFG_GET_UINT(t1, argv[3], para->p_rs);
    PM_CFG_GET_UINT(t2, argv[5], para->p_rs);

    if (t2 < t1)
    {
        CFG_CONFLICT_RET("IANA T1 shouldn't be greater than IANA T2");
    }
    
    /* 2. check duplicate config */
    if ((p_db_dhclient->iana_t1 == t1) && (p_db_dhclient->iana_t2 == t2))
    {
        return PM_E_NONE;
    }

    p_db_dhclient->iana_t1 = t1;
    p_db_dhclient->iana_t2 = t2;
        
    /* 3. update local CDB */
    rc = ds_dhclient_if_set_dhclient_if_field(p_db_if, p_db_dhclient, DS_DHCLIENT_IF_FLD_IANA_T1);
    if (rc)
    {
        return rc;    
    }

    rc = ds_dhclient_if_set_dhclient_if_field(p_db_if, p_db_dhclient, DS_DHCLIENT_IF_FLD_IANA_T2);
    if (rc)
    {
        return rc;    
    }
    
    return rc;
}

static int32
_if_mgr_cmd_process_interface_set_dhcp_client(char **argv, int32 argc, cfg_cmd_para_t *para, tbl_interface_t *p_db_if)
{
    tbl_route_if_t *p_db_rf = NULL;
    tbl_route_if_key_t  rif_key;
    cdb_node_t* p_node = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    uint32 lease = 0;
    int32 value = 0;
    uint32 i = 2;
    int32 rc = PM_E_NONE;

    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    p_node = cdb_get_ds(DS_DHCLIENT_IF);
    field_id = cdb_get_field_id(argv[i], p_node, &p_field);
    if (field_id < 0)
    {
        CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
    }
    if (CDB_ACCESS_RD == p_field->access)
    {    
        CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
    }

    sal_memset(&rif_key, 0, sizeof(rif_key));
    sal_strncpy(rif_key.name, p_db_if->key.name, IFNAME_SIZE);
    p_db_rf = tbl_route_if_get_route_if(&rif_key);
    
    /* physical, agg, vlan support only*/
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type
    && GLB_IF_TYPE_VLAN_IF != p_db_if->hw_type
    && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type
    )
    {
        CFG_CONFLICT_RET("Dhcp client is not supported on this type of interface\n");
    }

    switch (field_id)
    {
    case DS_DHCLIENT_IF_FLD_LEASE:
        if (NULL != p_db_rf && p_db_rf->dhcp_client_enable)
        {    
            CFG_INVALID_PARAM_RET("Can't be set after dhcp client function enabled");
        }

        PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
        lease = value;
        rc = _if_mgr_cmd_set_dhclient_lease(lease, p_db_if, para);
        break;
    case DS_DHCLIENT_IF_FLD_REQUESTED_FLAGS:
        if (NULL != p_db_rf && p_db_rf->dhcp_client_enable)
        {    
            CFG_INVALID_PARAM_RET("Can't be set after dhcp client function enabled");
        }
        
        rc = _if_mgr_cmd_set_dhclient_requested_flag(argv, argc, p_db_if, para, 1);
        break;
    case DS_DHCLIENT_IF_FLD_CLIENT_ID:
        if (NULL != p_db_rf && p_db_rf->dhcp_client_enable)
        {    
            CFG_INVALID_PARAM_RET("Can't be set after dhcp client function enabled");
        }
        
        rc = _if_mgr_cmd_set_dhclient_client_id(argv, argc, p_db_if, para);
        break;
        
    case DS_DHCLIENT_IF_FLD_CLASS_ID:
        if (NULL != p_db_rf && p_db_rf->dhcp_client_enable)
        {    
            CFG_INVALID_PARAM_RET("Can't be set after dhcp client function enabled");
        }
        
        rc = _if_mgr_cmd_set_dhclient_class_id(argv, argc, p_db_if, para);
        break;

    case DS_DHCLIENT_IF_FLD_HOSTNAME:
        if (NULL != p_db_rf && p_db_rf->dhcp_client_enable)
        {    
            CFG_INVALID_PARAM_RET("Can't be set after dhcp client function enabled");
        }
        
        rc = _if_mgr_cmd_set_dhclient_hostname(argv[i+1], p_db_if, para);
        break;

    case DS_DHCLIENT_IF_FLD_IANA_T1:
        if (NULL != p_db_rf && p_db_rf->dhcpv6_client_enable)
        {    
            CFG_INVALID_PARAM_RET("Can't be set after dhcpv6 client function enabled");
        }
        
        rc = _if_mgr_cmd_set_dhclient_iana_t1(argc, argv, p_db_if, para);
        break;
        
    default:
        CFG_FUNC_ERR_RET("Unknown support changed field %s", argv[i]);
        break;
    }

    return rc;
}

static int32
_if_mgr_cmd_process_interface_unset_dhcp_client(char **argv, int32 argc, cfg_cmd_para_t *para, tbl_interface_t *p_db_if)
{
    cdb_node_t* p_node = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    uint32 i = 2;
    int32 rc = PM_E_NONE;
    tbl_route_if_t *p_db_rf = NULL;
    tbl_route_if_key_t  rif_key;
    
    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    p_node = cdb_get_ds(DS_DHCLIENT_IF);
    field_id = cdb_get_field_id(argv[i], p_node, &p_field);
    if (field_id < 0)
    {
        CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
    }
    if (CDB_ACCESS_RD == p_field->access)
    {    
        CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
    }
    sal_memset(&rif_key, 0, sizeof(rif_key));
    sal_strncpy(rif_key.name, p_db_if->key.name, IFNAME_SIZE);
    p_db_rf = tbl_route_if_get_route_if(&rif_key);
    if (NULL != p_db_rf && p_db_rf->dhcp_client_enable)
    {    
        CFG_INVALID_PARAM_RET("Can't be set after dhcp client function enabled");
    }
    switch (field_id)
    {
    case DS_DHCLIENT_IF_FLD_REQUESTED_FLAGS:
        rc = _if_mgr_cmd_set_dhclient_requested_flag(argv, argc, p_db_if, para, 0);
        break;

    default:
        CFG_FUNC_ERR_RET("Unknown support changed field %s", argv[i]);
        break;
    }

    return rc;
}

static int32
_if_mgr_cmd_process_interface_parser_storm_control(char **argv, int32 argc, cfg_cmd_para_t *para, 
                                        uint32 *rate, uint32* type, uint32* mode)
{
    int32 rc = PM_E_NONE;  
    int32 i = 2;
    uint32 len = 0;
    int32 value = 0;

    if (0 == sal_strcmp(argv[i], "set"))
    {
        i++;
        if (0 == sal_strcmp(argv[i], "broadcast"))
        {
            *type = GLB_RATELIMIT_BROADCAST;
        }
        else if (0 == sal_strcmp(argv[i], "multicast"))
        {
            *type = GLB_RATELIMIT_MULTICAST;
        }
        else if (0 == sal_strcmp(argv[i], "unicast"))
        {
            *type = GLB_RATELIMIT_UNICAST;
        }
        else
        {
            CFG_INVALID_PARAM_RET("Unkown storm control type: %s", argv[i]);
        }
        
        i++;
        len = 0;
        if (0 == sal_strcmp(argv[i], "level"))
        {
            *mode = GLB_RATELIMIT_MODE_LEVEL;
            i++;
            while (argv[i][len] != '\0')        
            {
                if (argv[i][len] != '.')
                {
                    len++;                        
                    continue;
                }
                argv[i][len] = '\0';
                value = sal_atoi(argv[i]);
                *rate = value;
                argv[i][len] = '.';
                len++;
                break;
            } 
            if  (len < sal_strlen(argv[i]))
            {
                value = sal_atoi(argv[i] + len);
                /* A.B */
                if (1 == sal_strlen(argv[i] + len))
                {
                    *rate = *rate * 100 + value * 10;
                }
                else if (2 == sal_strlen(argv[i] + len))
                {
                    *rate= *rate * 100 + value;
                }
                else 
                {
                     CFG_INVALID_PARAM_RET("The level value must be the form of 0.00-100.00");
                }
            } 
            else 
            {
                value = sal_atoi(argv[i]);
                *rate = value * 100;     
            }
        }
        else if (0 == sal_strcmp(argv[i], "pps"))
        {
            *mode = GLB_RATELIMIT_MODE_PPS;
            i++;
            while (argv[i][len] != '\0')        
            {
                len++;  
                continue;
            } 
            value = sal_atoi(argv[i]);
            *rate = value ;  
        }
        else
        {
            CFG_INVALID_PARAM_RET("Unkown storm control mode: %s", argv[i]);
        } 
    }
    else if (0 == sal_strcmp(argv[i], "unset"))
    {
        *mode = GLB_RATELIMIT_MODE_NONE;
        i++;
        if (0 == sal_strcmp(argv[i], "broadcast"))
        {
            *type = GLB_RATELIMIT_BROADCAST;
        }
        else if (0 == sal_strcmp(argv[i], "multicast"))
        {
            *type = GLB_RATELIMIT_MULTICAST;
        }
        else if (0 == sal_strcmp(argv[i], "unicast"))
        {
            *type = GLB_RATELIMIT_UNICAST;
        }
        else
        {
            CFG_INVALID_PARAM_RET("Unkown storm control type: %s", argv[i]);
        }
        
    }
   else 
   {
         CFG_INVALID_PARAM_RET("Invalid operation!");
   }

   return rc;
}

static int32
_if_mgr_cmd_process_interface_set_storm_control(char **argv, int32 argc, cfg_cmd_para_t *para, 
                                tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    uint32 type = 0;
    uint32 mode = 0;   
    uint32 rate = 0;      

    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);

    if (p_db_if->mode != GLB_IF_MODE_L2)
    {
        CFG_INVALID_PARAM_RET("Interface not configured for switching"); 
    }

    if (p_db_if->hw_type == GLB_IF_TYPE_LINKAGG_IF)
    {
        CFG_INVALID_PARAM_RET("Cannot configure aggregator"); 
    }    
    
    PM_E_RETURN(_if_mgr_cmd_process_interface_parser_storm_control
                    (argv, argc, para, &rate, &type, &mode));

    if (GLB_RATELIMIT_MAX_TYPE <= type)
    {
        return PM_E_INVALID_PARAM;
    }
    
    if (GLB_RATELIMIT_MODE_PPS == mode)
    {
        if (GLB_CHIP_GREATEBELT == tbl_chip_get_type() || GLB_CHIP_GOLDENGATE == tbl_chip_get_type())
        {
            if (rate % 1000 != 0)
            {
                CFG_INVALID_PARAM_RET("The pps value must be in multiples of 1000");
            }
        }
        else
        {
            /* STMCTL_MAX_PPS should be 148809523 in d2 and tm system */
            if (rate > GLB_SECURITY_STMCTL_MAX_PPS)
            {
                CFG_INVALID_PARAM_RET("The pps value must less than or equal to 148809523");
            }
        }
    }

    if ((GLB_RATELIMIT_MODE_LEVEL == mode) && (rate == 10000))
    {
        /*when the level value is 100(the rate is (level * 100))*/
        CFG_INVALID_PARAM_RET("The level value cannot be set 100");
    }
     
    PM_E_RETURN(if_mgr_set_storm_control(p_db_if, rate, type, mode));

    return rc;
        
}
  
static glb_qos_trust_state_t
_if_mgr_aclqos_if_map_trust(char* str)
{
    int32 i = 0;
    char *mode_str[GLB_QOS_TRUST_MAX] =
    {
        "port",
        "outer",
        "cos",
        "tos",
        "dscp-exp",
        "ip-prec",
        "ctag-cos",
        "stag-cos"
    };
        
    for (i = GLB_QOS_TRUST_PORT; i < GLB_QOS_TRUST_MAX; i++)
    {
        if (0 == sal_strcmp(str, mode_str[i]))
        {
            return i;
        }
    }
    return GLB_QOS_TRUST_MAX;
}
  
static glb_qos_replace_state_t
_if_mgr_aclqos_if_map_replace(char* str)
{
    int32 i = 0;
    char *mode_str[GLB_QOS_REPLACE_MAX] =
    {
        "none",
        "cos",
        "dscp-exp",
        "stag-cos",
        "ctag-cos"
    };
        
    for (i = GLB_QOS_REPLACE_NONE; i < GLB_QOS_REPLACE_MAX; i++)
    {
        if (0 == sal_strcmp(str, mode_str[i]))
        {
            return i;
        }
    }
    return GLB_QOS_REPLACE_MAX;
}
      
static int32
_if_mgr_cmd_process_interface_set_aclqos_if(char **argv, int32 argc, cfg_cmd_para_t *para, 
                                tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    uint32 i = 0;   
    cdb_node_t* p_node = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    int32 qid = 0;
    int32 dir = 0;
    int32 type = 0;
    ds_aclqos_if_t aclqos_if;
    tbl_qos_policer_profile_t *p_db_qos_policer_profile = NULL;
    tbl_qos_queue_shape_profile_t *p_db_qos_queue_shaper_profile = NULL;
    tbl_qos_drop_profile_t *p_db_qos_drop_profile = NULL;
    char *name = NULL;

    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    
    /* check  physical interface,for enter queue configure mode */
    if (argc == 2)
    {
        if (p_db_if->hw_type != GLB_IF_TYPE_PORT_IF)
        {
            CFG_INVALID_PARAM_RET("Interface %s is not a physical port!", p_db_if->key.name); 
        }    

        return PM_E_NONE;
    }

    
    p_node = cdb_get_ds(DS_ACLQOS_IF);
    i = 2;
    field_id = cdb_get_field_id(argv[2], p_node, &p_field);
    i++;
    if (!tbl_qos_global_get_qos_enable() && (DS_ACLQOS_IF_FLD_INPUT_POLICY_MAP != field_id)
            && (DS_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP != field_id))
    
    {
        CFG_INVALID_PARAM_RET("QoS is not enabled!");
    }
    if (field_id < 0)
    {
        CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
    }
    if (CDB_ACCESS_RD == p_field->access)
    {    
        CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
    }

    if (NULL == p_db_if->aclqos_if)
    {
        if ((DS_ACLQOS_IF_FLD_INPUT_POLICY_MAP != field_id)
                && (DS_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP != field_id))
        {
            CFG_INVALID_PARAM_RET("Interface %s is not a port!", p_db_if->key.name); 
        }
    }

    sal_memset(&aclqos_if, 0, sizeof(aclqos_if));
    switch (field_id)
    {
    case DS_ACLQOS_IF_FLD_DEFAULT_COS:
        value = sal_atoi(argv[i]);
        aclqos_if.default_cos = value;
        if (p_db_if->aclqos_if->default_cos == value)
        {
            return rc;
        }
        type = GLB_QOS_PORT_QUEUE_UPDATE_DEFCOS;
        break;
    case DS_ACLQOS_IF_FLD_DOMAIN:
        value = sal_atoi(argv[i]);
        aclqos_if.domain = value;
        if (p_db_if->aclqos_if->domain == value)
        {
            return rc;
        }
        type = GLB_QOS_PORT_QUEUE_UPDATE_DOMAIN;
        
        break;        
    case DS_ACLQOS_IF_FLD_TRUST:
        value = _if_mgr_aclqos_if_map_trust(argv[i]);
        aclqos_if.trust = value;
        if (p_db_if->aclqos_if->trust == value)
        {
            return rc;
        }
        type = GLB_QOS_PORT_QUEUE_UPDATE_TRUST;
        break;
    case DS_ACLQOS_IF_FLD_REPLACE:    
        if (GLB_IF_MODE_L2 != p_db_if->mode)
        {
            CFG_INVALID_PARAM_RET("Interface %s not configured for switching", p_db_if->key.name); 
        }
        value = _if_mgr_aclqos_if_map_replace(argv[i]);
        if ((GLB_VLAN_PORT_TYPE_ACCESS == p_db_if->vlan_type || GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type) && 
                ((GLB_QOS_REPLACE_COS == value) ||
                 (GLB_QOS_REPLACE_STAG_COS == value) || 
                 (GLB_QOS_REPLACE_CTAG_COS == value)))
        {
            CFG_INVALID_PARAM_RET("Can't configure replace cos on access port %s", p_db_if->key.name); 
        }
        if (p_db_if->aclqos_if->replace == value)
        {
            return rc;
        }
        aclqos_if.replace = value;
        type = GLB_QOS_PORT_QUEUE_UPDATE_REPLACE;
        break;
    case DS_ACLQOS_IF_FLD_SHAPE_PIR:
        if (sal_strlen(argv[i]))
        {
            value = sal_atoi(argv[i]);
            if (p_db_if->aclqos_if->shape_pir == value)
            {
                return rc;
            }
            aclqos_if.shape_pir = value;
        }
        else
        {
            if (p_db_if->aclqos_if->shape_pir == GLB_QOS_SHAPE_DEFAULT_RATE)
            {
                return rc;
            }
            aclqos_if.shape_pir = GLB_QOS_SHAPE_DEFAULT_RATE;
        }
        type = GLB_QOS_PORT_QUEUE_UPDATE_SHAPE;
        break;
    case DS_ACLQOS_IF_FLD_QUEUE_STATS_ENABLE:
        value = sal_atoi(argv[i]);
        // TODO:maybe enabled by default
        if (p_db_if->aclqos_if->queue_stats_enable == value)
        {
            return rc;
        }
        aclqos_if.queue_stats_enable = value;    
        type = GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_STATS_EN;
        break;
    case DS_ACLQOS_IF_FLD_INPUT_POLICY_MAP:
    case DS_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP:
        if ((GLB_IF_TYPE_PORT_IF != p_db_if->hw_type) && (GLB_IF_TYPE_VLAN_IF != p_db_if->hw_type)
                && (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type))
        {
            CFG_FUNC_ERR_RET(" Interface type error!"); 
        }
        if (INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            CFG_FUNC_ERR_RET(" Cannot configure aggregator member."); 
        }
        if (DS_ACLQOS_IF_FLD_INPUT_POLICY_MAP == field_id)
        {
            dir = GLB_INGRESS;
            name = p_db_if->aclqos_if->input_policy_map;
            sal_strcpy(aclqos_if.input_policy_map, argv[i]);
        }
        else
        {
            dir = GLB_EGRESS;
            name = p_db_if->aclqos_if->output_policy_map;
            sal_strcpy(aclqos_if.output_policy_map, argv[i]);
//            if (('\0' != argv[i][0]) && !acl_mgr_check_pmap_egress_unsupport_flags(argv[i]))
//            {
//               CFG_FUNC_ERR_RET("Failed because this policy-map is not supported on output"); 
//            }
        }
        /* attach policy-map */
        if ((0 != sal_strlen(argv[i])) && (NULL != p_db_if->aclqos_if) && ('\0' != name[0]))
        {
            CFG_FUNC_ERR_RET(" Policy-map(%s) is already attached", name);
        }
        /* deattach policy-map */
        if ((0 == sal_strlen(argv[i])) && ((NULL == p_db_if->aclqos_if) || ('\0' == name[0])))
        {
            CFG_FUNC_ERR_RET(" Can't find %s policy-map on interface", (GLB_INGRESS == dir) ? "input" : "output");
        }

        if (sal_strlen(argv[i]))
        {
            type = GLB_QOS_PORT_QUEUE_UPDATE_ATTACH_PMAP;
        }
        else
        {
            type = GLB_QOS_PORT_QUEUE_UPDATE_DEATTACH_PMAP;
        }

        /* select input/output policy-map by field id when update tbl */
        /* deattach policy-map should be processed here, since we need policy-map name */ 
        PM_E_RETURN(qos_mgr_port_queue_update_handle(GLB_QOS_UPDATE_OP_SET, p_db_if->ifindex, qid, dir, type));
        break;
    case DS_ACLQOS_IF_FLD_INPUT_POLICER:
        dir = GLB_INGRESS; 
        sal_strcpy(aclqos_if.input_policer, argv[i]);
    case DS_ACLQOS_IF_FLD_OUTPUT_POLICER:
        if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
        {
            CFG_FUNC_ERR_RET(" This interface cannot be used for QoS"); 
        }
        if (DS_ACLQOS_IF_FLD_OUTPUT_POLICER == field_id)
        {
            name = p_db_if->aclqos_if->output_policer;
            dir = GLB_EGRESS; 
            sal_strcpy(aclqos_if.output_policer, argv[i]);
        }
        else
        {
            name = p_db_if->aclqos_if->input_policer;
        }

        if (0 == strcmp(name, argv[i]))
        {
            return rc;
        }
        /* check policer profile exist */
        if (0 != sal_strlen(argv[i]))
        {
            p_db_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(argv[i]);
            if (NULL == p_db_qos_policer_profile)
            {
                CFG_INVALID_PARAM_RET("Can't find policer profile (%s)", aclqos_if.input_policer);
            }
            else if (GLB_FLAG_ISSET(p_db_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_AGP))
            {
                CFG_INVALID_PARAM_RET("Policer profile(%s) is aggregate policer", aclqos_if.input_policer);
            }
            else if (0 == p_db_qos_policer_profile->flags)
            {
                CFG_INVALID_PARAM_RET("Policer profile(%s) parameters are not configured", p_db_qos_policer_profile->name);
            }
        }
        type = GLB_QOS_PORT_QUEUE_UPDATE_POLICER;
        break;
#if 0
    case DS_ACLQOS_IF_FLD_OUTPUT_POLICER:
        if (0 == strcmp(p_db_if->aclqos_if->output_policer, argv[i]))
        {
            return rc;
        }
        sal_strcpy(aclqos_if.output_policer, argv[i]); 
        if (aclqos_if.output_policer[0] != '\0')
        {
            p_db_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(aclqos_if.output_policer);
            if (!p_db_qos_policer_profile)
            {
                CFG_INVALID_PARAM_RET("Can't find policer profile (%s)", aclqos_if.output_policer);
            }
            else if (!p_db_qos_policer_profile->flags)
            {
                CFG_INVALID_PARAM_RET("Policer profile(%s) parameters are not configured", aclqos_if.output_policer);
            }
        }
        type = GLB_QOS_PORT_QUEUE_UPDATE_POLICER;
        dir = GLB_EGRESS;
        break;
#endif 
    case DS_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE:
        qid = sal_atoi(argv[i]);
        i++;
        if (0 == strcmp(p_db_if->aclqos_if->queue_shape_profile[qid], argv[i]))
        {
            return rc;
        }
        if ('\0' != argv[i][0])
        {
            p_db_qos_queue_shaper_profile = tbl_qos_queue_shape_profile_get_profile_by_name(argv[i]);
            if (NULL == p_db_qos_queue_shaper_profile)
            {
                CFG_INVALID_PARAM_RET("Can't find shape profile (%s)", argv[i]);
            }
            else if (0 == p_db_qos_queue_shaper_profile->flags)
            {
                CFG_INVALID_PARAM_RET("Shape profile(%s) parameters are not configured", argv[i]);
            }
        }
        PM_E_RETURN(ds_aclqos_if_get_aclqos_if_field(p_db_if, field_id, aclqos_if.queue_shape_profile));
        sal_memcpy(aclqos_if.queue_shape_profile[qid], argv[i], QOS_NAME_SIZE);
        type = GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_SHAPE;
        dir = GLB_EGRESS;
        break;
    case DS_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE:
        PM_E_RETURN(ds_aclqos_if_get_aclqos_if_field(p_db_if, field_id, aclqos_if.queue_drop_profile));
        qid = sal_atoi(argv[i]);
        i++;
        if ('\0' != argv[i][0])
        {
            if (0 == strcmp(p_db_if->aclqos_if->queue_drop_profile[qid], argv[i]))
            {
                return rc;
            }
            p_db_qos_drop_profile = tbl_qos_drop_profile_get_profile_by_name(argv[i]);
            if (NULL == p_db_qos_drop_profile)
            {
                CFG_INVALID_PARAM_RET("Can't find drop profile (%s)", argv[i]);
            }
            else if (0 == p_db_qos_drop_profile->flags)
            {
                CFG_INVALID_PARAM_RET("Drop profile(%s) parameters are not configured", argv[i]);
            }
            sal_memcpy(aclqos_if.queue_drop_profile[qid], argv[i], QOS_NAME_SIZE);
        }
        else
        {
            if (GLB_QOS_QUEUE_DROP_MODE_WTD == p_db_if->aclqos_if->queue_drop_mode[qid])
            {
                if (0 == strcmp(p_db_if->aclqos_if->queue_drop_profile[qid], GLB_QOS_DEF_WTD_PROFILE))
                {
                    return rc;
                }
                sal_memcpy(aclqos_if.queue_drop_profile[qid], GLB_QOS_DEF_WTD_PROFILE, QOS_NAME_SIZE);
            } 
            else
            {
                if (0 == strcmp(p_db_if->aclqos_if->queue_drop_profile[qid], GLB_QOS_DEF_WRED_PROFILE))
                {
                    return rc;
                }
                sal_memcpy(aclqos_if.queue_drop_profile[qid], GLB_QOS_DEF_WRED_PROFILE, QOS_NAME_SIZE);
            }
        }

        type = GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_DROP;
        dir = GLB_EGRESS;
        break;
    case DS_ACLQOS_IF_FLD_QUEUE_CLASS:
        qid = sal_atoi(argv[i]);
        i++;
        PM_E_RETURN(ds_aclqos_if_get_aclqos_if_field(p_db_if, field_id, aclqos_if.queue_class));
        if (sal_strlen(argv[i]))
        {
            value = sal_atoi(argv[i]);
        }
        else
        {
            /* set class to default */
            value = qos_mgr_get_default_queue_class(qid);
        }

        if (p_db_if->aclqos_if->queue_class[qid] == value)
        {
            return rc;
        }

        aclqos_if.queue_class[qid] = value;
        type = GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_CLASS;
        break;
    case DS_ACLQOS_IF_FLD_QUEUE_WEIGHT:
        qid = sal_atoi(argv[i]);
        i++;
        PM_E_RETURN(ds_aclqos_if_get_aclqos_if_field(p_db_if, field_id, aclqos_if.queue_weight));
        if (sal_strlen(argv[i]))
        {
            value = sal_atoi(argv[i]);
        }
        else
        {
            /* set class to default */
            value = qos_mgr_get_default_queue_weight(qid);
        }

        if (p_db_if->aclqos_if->queue_weight[qid] == value)
        {
            return rc;
        }

        aclqos_if.queue_weight[qid] = value;
        type = GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_WEIGHT;
        break;
    case DS_ACLQOS_IF_FLD_QUEUE_DROP_MODE:
        qid = sal_atoi(argv[i]);
        i++;
        PM_E_RETURN(ds_aclqos_if_get_aclqos_if_field(p_db_if, field_id, aclqos_if.queue_drop_mode));
        if (sal_strlen(argv[i]))
        {
            value = sal_atoi(argv[i]);
        }
        else
        {
            /* set class to default */
            value = 0;
        }

        if (p_db_if->aclqos_if->queue_drop_mode[qid] == value)
        {
            return rc;
        }

        aclqos_if.queue_drop_mode[qid] = value;
        type = GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_DROP_MODE;
        break;
    case DS_ACLQOS_IF_FLD_QUEUE_ECN_ENABLE:
        qid = sal_atoi(argv[i]);
        i++;
        PM_E_RETURN(ds_aclqos_if_get_aclqos_if_field(p_db_if, field_id, aclqos_if.queue_ecn_enable));
        if (sal_strlen(argv[i]))
        {
            value = sal_atoi(argv[i]);
        }
        else
        {
            /* set class to default */
            value = 0;
        }

        if (p_db_if->aclqos_if->queue_ecn_enable[qid] == value)
        {
            return rc;
        }

        aclqos_if.queue_ecn_enable[qid] = value;
        type = GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_ECN;
        break;
        
    default:
        CFG_FUNC_ERR_RET("Unknown support changed field %s", argv[i]);
        break;
    }
//    /* check conflict */
//    if (!tbl_qos_global_get_qos_enable() && (field_id != DS_ACLQOS_IF_FLD_INPUT_POLICY_MAP) && 
//            (field_id != DS_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP))
//    {
//        CFG_INVALID_PARAM_RET("QoS is not enabled!");
//    }
  
    /* if aclqos_if exist */
    if (p_db_if->aclqos_if)
    {
        PM_E_RETURN(ds_aclqos_if_set_aclqos_if_field(p_db_if, &aclqos_if, field_id));
    }
    else
    {
        /* set pmap, we reuse phb_enable to indicate qos disable,
         * we qos enable, phb_enable would be set to correct value */
        aclqos_if.phb_enable = 2;
        PM_E_RETURN(ds_aclqos_if_add_aclqos_if(p_db_if, &aclqos_if));
    }

    PM_E_RETURN(qos_mgr_port_queue_update_handle(GLB_QOS_UPDATE_OP_SET, p_db_if->ifindex, qid, dir, type));

    return rc;
        
}

int32
tap_cmd_process_interface_pmap(char *pmap, bool isadd, tbl_interface_t *p_db_if)
{
    ds_aclqos_if_t aclqos_if;
    int32 qid = 0;
    int32 dir = 0;
    int32 type = 0;
    int32 rc = PM_E_NONE;

    sal_memset(&aclqos_if, 0, sizeof(aclqos_if));

    dir = GLB_INGRESS;
    sal_strcpy(aclqos_if.input_policy_map, pmap);

    if (isadd)
    {
        type = GLB_QOS_PORT_QUEUE_UPDATE_ATTACH_PMAP;
    }
    else
    {
        type = GLB_QOS_PORT_QUEUE_UPDATE_DEATTACH_PMAP;
    }

    /* select input/output policy-map by field id when update tbl */
    /* deattach policy-map should be processed here, since we need policy-map name */ 
    PM_E_RETURN(qos_mgr_port_queue_update_handle(GLB_QOS_UPDATE_OP_SET, p_db_if->ifindex, qid, dir, type));


    /* if aclqos_if exist */
    if (p_db_if->aclqos_if)
    {
        PM_E_RETURN(ds_aclqos_if_set_aclqos_if_field(p_db_if, &aclqos_if, DS_ACLQOS_IF_FLD_INPUT_POLICY_MAP));
    }
    else
    {
        /* set pmap, we reuse phb_enable to indicate qos disable,
         * we qos enable, phb_enable would be set to correct value */
        aclqos_if.phb_enable = 2;
        PM_E_RETURN(ds_aclqos_if_add_aclqos_if(p_db_if, &aclqos_if));
    }

    PM_E_RETURN(qos_mgr_port_queue_update_handle(GLB_QOS_UPDATE_OP_SET, p_db_if->ifindex, qid, dir, type));
    return rc;
}

static int32
_if_mgr_cmd_process_interface_set_circuit_id(char **argv, int32 argc, cfg_cmd_para_t *para, tbl_interface_t *p_db_if)
{
    cdb_node_t* p_node = NULL;
    ds_circuit_id_t dsItem;
    ds_circuit_id_t *pItem;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value;
    uint32 i;
    char *p;
    int32 rc = PM_E_NONE;
    int isDel = 0;

    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    if (GLB_IF_MODE_L2 != p_db_if->mode) {
        CFG_CONFLICT_RET("Can only configure on layer two interface");
    }
    if (argc < 3 || (argc % 2) != 1) {
        CFG_CONFLICT_RET("Invalid parameters");
    }

    /* check sub action */
    if (!strcasecmp(argv[2], "add")) {
    } else if (!strcasecmp(argv[2], "del")) {
        isDel = 1;
    } else if (!strcasecmp(argv[2], "flush")) {
        ctclib_slistnode_t *lnode = NULL;
        ctclib_slistnode_t *nnext = NULL;
        CTCLIB_SLIST_LOOP_DEL(p_db_if->circuit_id.obj_list, pItem, lnode, nnext) {
            ds_circuit_id_del_circuit_id(p_db_if, pItem);
        }
        return PM_E_NONE;
    } else {
        CFG_INVALID_PARAM_RET("Invalid sub action %s", argv[2]);
    }

    p_node = cdb_get_ds(DS_CIRCUIT_ID);
    memset(&dsItem, 0x00, sizeof(ds_circuit_id_t));
    /* check parameters */
    for (i = 3; i < argc; i += 2) {
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0) {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (DS_CIRCUIT_ID_FLD_KEY != field_id
                && CDB_ACCESS_RD == p_field->access) {    
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch((ds_circuit_id_field_id_t)field_id) {
        case DS_CIRCUIT_ID_FLD_KEY:
            p = argv[i + 1];
            value = atoi(p);
            if (strspn(p, "01234567890") != strlen(p) 
                    || value < 0 || value > GLB_VLAN_MAX) {
                CFG_INVALID_PARAM_RET("Invalid field %s", argv[i]);
            }
            dsItem.key.vid = value;
            break;
        case DS_CIRCUIT_ID_FLD_CIRCUIT:
            p = argv[i + 1];
            if (cdb_check_str(p, DHCSNOOPING_MAX_CIRCUIT_ID_STRING - 1,
                               para->p_rs->desc) < 0) {
                para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
                return PM_E_CFG_INVALID_PARAM;
            }
            snprintf(dsItem.circuit, DHCSNOOPING_MAX_CIRCUIT_ID_STRING, "%s", p);
            break;

        case DS_CIRCUIT_ID_FLD_MAX:
            continue;
        }        
    }

    if (0 == dsItem.key.vid) {
        CFG_CONFLICT_RET("Invalid parameters, please specify circuit id key");
    }

    pItem = ds_circuit_id_get_circuit_id(p_db_if, &dsItem);
    if (isDel) {
        if (pItem) {
            rc = ds_circuit_id_del_circuit_id(p_db_if, &dsItem);
        } else {
            rc = PM_E_NONE;
        }
        return rc;
    }
    
    /* add item */
    for (i = 3; i < argc; i += 2) {
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        switch((ds_circuit_id_field_id_t)field_id) {
        case DS_CIRCUIT_ID_FLD_CIRCUIT:
            if (NULL != pItem) {
                /* update */
                rc = ds_circuit_id_set_circuit_id_field(p_db_if, &dsItem, 
                        DS_CIRCUIT_ID_FLD_CIRCUIT);
            } else {
                rc = ds_circuit_id_add_circuit_id(p_db_if, &dsItem);
            }
            break;

        case DS_CIRCUIT_ID_FLD_KEY:
        case DS_CIRCUIT_ID_FLD_MAX:
            continue;
        }        
    }

    return rc;
}

static int32
_if_mgr_cmd_process_interface_set_crc_check(tbl_interface_t* p_if, uint32 iter_member)
{
    int32 rc = PM_E_NONE;
    tbl_interface_t *p_db_if = NULL;

    /* 1. check */    
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_CRC_ERROR_CHECK_EN));
    return rc;
}

static int32
_if_mgr_cmd_set_crc_check_enable(tbl_interface_t* p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */
    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    
    /* 2. check duplicate config */
    if (p_db_if->crc_error_check_en == p_if->crc_error_check_en)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(_if_mgr_cmd_process_interface_set_crc_check(p_if, TRUE));
    return rc;
}

static int32
_if_mgr_cmd_process_interface_set_crc_recalculation(tbl_interface_t* p_if, uint32 iter_member)
{
    int32 rc = PM_E_NONE;
    tbl_interface_t *p_db_if = NULL;

    /* 1. check */    
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. process */
    //PM_E_RETURN(fei_if_set_crc_recalculation_enable(p_if->ifindex, p_if->crc_recalculation_en));

    /* 2. update local CDB */
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_CRC_RECALCULATION_EN));
    return rc;
}

static int32
_if_mgr_cmd_set_crc_recalculation_enable(tbl_interface_t* p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    
    /* 2. check duplicate config */
    if (p_db_if->crc_recalculation_en == p_if->crc_recalculation_en)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(_if_mgr_cmd_process_interface_set_crc_recalculation(p_if, TRUE));
    return rc;
}

#ifdef CONFIG_L2
static int32
_if_mgr_cmd_process_interface_set_brgif(char **argv, int32 argc, cfg_cmd_para_t *para, tbl_interface_t *p_db_if)
{
    cdb_node_t* p_node = NULL;
    ds_brgif_t brgif;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    uint32 i = 2;
    int32 vlan_type = 0;
    glb_vlan_port_type_t cfg_type = 0;
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    p_node = cdb_get_ds(DS_BRGIF);
    field_id = cdb_get_field_id(argv[i], p_node, &p_field);
    if (field_id < 0)
    {
        CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
    }
    if (CDB_ACCESS_RD == p_field->access)
    {    
        CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
    }

    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port or LAG", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    if (GLB_IF_MODE_L2 != p_db_if->mode)
    {
        CFG_CONFLICT_RET("Interface %s is not in switch mode", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    
    sal_memset(&brgif, 0, sizeof(brgif));
    switch (field_id)
    {
    case DS_BRGIF_FLD_ALLOWED_VLAN:
        PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
        if (0 == sal_strcmp("access", argv[i+1]))
        {
            cfg_type = GLB_VLAN_PORT_TYPE_ACCESS;
        }
        else if (0 == sal_strcmp("trunk", argv[i+1]))
        {
            cfg_type = GLB_VLAN_PORT_TYPE_TRUNK;
        }
        else if (0 == sal_strcmp("dot1q-tunnel", argv[i+1]))
        {
            cfg_type = GLB_VLAN_PORT_TYPE_QINQ;
        }
        else
        {
            CFG_FUNC_ERR_RET("Unknown port vlan type %s", argv[i+1]);
        }
        PM_CFG_GET_ENUM(value, ds_brgif_allowed_vlan_act_strs, DS_BRGIF_ALLOWED_VLAN_ACT_MAX, argv[i+2], para->p_rs);
        brgif.allowed_vlan.action = value;
        switch (value)
        {
        case DS_BRGIF_ALLOWED_VLAN_ACT_ADD:
        case DS_BRGIF_ALLOWED_VLAN_ACT_DEL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 6, para->p_rs);
            rc = cdb_bitmap_str2val(argv[i+3], GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, brgif.allowed_vlan.vlan_bmp, GLB_VLAN_BMP_WORD_MAX, para->p_rs->desc);
            if (rc < 0)
            {
                para->p_rs->rc = rc;
                return rc;
            }
            break;
        case DS_BRGIF_ALLOWED_VLAN_ACT_ALL:
        case DS_BRGIF_ALLOWED_VLAN_ACT_NONE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 5, para->p_rs);
            break;
        default:
            CFG_FUNC_ERR_RET("Unknown allowed vlan action %s", argv[i+2]);
        }
        rc = _if_mgr_cmd_set_brgif_allowed_vlan(&brgif, p_db_if, cfg_type, para);        
        break;
        
    case DS_BRGIF_FLD_UNTAGGED_VLAN:
        PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
        if (0 == sal_strcmp("trunk", argv[i+1]))
        {
            cfg_type = GLB_VLAN_PORT_TYPE_TRUNK;
        }
        else
        {
            CFG_FUNC_ERR_RET("Unknown port vlan type %s", argv[i+1]);
        }
        PM_CFG_GET_ENUM(value, ds_brgif_allowed_vlan_act_strs, DS_BRGIF_ALLOWED_VLAN_ACT_MAX, argv[i+2], para->p_rs);
        brgif.untagged_vlan.action = value;
        switch (value)
        {
        case DS_BRGIF_ALLOWED_VLAN_ACT_ADD:
        case DS_BRGIF_ALLOWED_VLAN_ACT_DEL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 6, para->p_rs);
            rc = cdb_bitmap_str2val(argv[i+3], GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, brgif.untagged_vlan.vlan_bmp, GLB_VLAN_BMP_WORD_MAX, para->p_rs->desc);
            if (rc < 0)
            {
                para->p_rs->rc = rc;
                return rc;
            }
            break;
        default:
            CFG_FUNC_ERR_RET("Unknown allowed vlan action %s", argv[i+2]);
        }
        rc = _if_mgr_cmd_set_brgif_untagged_vlan(&brgif, p_db_if, cfg_type, para);        
        break;

    case DS_BRGIF_FLD_PVID:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 5, para->p_rs);
        PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
        PM_CFG_GET_ENUM(vlan_type, glb_vlan_port_type_strs, GLB_VLAN_PORT_TYPE_MAX, argv[i+2], para->p_rs);
        brgif.pvid = value;
        rc = _if_mgr_cmd_set_brgif_pvid(&brgif, p_db_if, vlan_type, para);
        break;
    case DS_BRGIF_FLD_INNER_NATIVE_VID:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 5, para->p_rs);
        PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
        PM_CFG_GET_ENUM(vlan_type, glb_vlan_port_type_strs, GLB_VLAN_PORT_TYPE_MAX, argv[i+2], para->p_rs);
        brgif.inner_native_vid = value;
        rc = _if_mgr_cmd_set_brgif_inner_native_vlan(&brgif, p_db_if, vlan_type, para);
        break;
    case DS_BRGIF_FLD_QINQ_TYPE:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        if (!sal_strcmp("basic", argv[i+1]))
        {
            brgif.qinq_type = GLB_VLAN_QINQ_TYPE_BASIC;
        }
        else if (!sal_strcmp("selective", argv[i+1]))
        {
            brgif.qinq_type = GLB_VLAN_QINQ_TYPE_SELECTIVE;
        }
        else
        {
            CFG_INVALID_PARAM_RET("Unknown QinQ type %s", argv[i+1]);
        }
        rc = _if_mgr_cmd_set_brgif_qinq_type(&brgif, p_db_if, para);
        break;
    case DS_BRGIF_FLD_VLAN_TRANSLATION_EN:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
        brgif.vlan_translation_en = value;
        rc = _if_mgr_cmd_set_brgif_vlan_translation_en(&brgif, p_db_if, para);
        break;
    case DS_BRGIF_FLD_VLAN_MAPPING_TABLE:
        PM_CFG_CHECK_MIN_PARAMS(argc, 4, para->p_rs);
        PM_CFG_GET_ENUM(vlan_type, glb_vlan_port_type_strs, GLB_VLAN_PORT_TYPE_MAX, argv[i+1], para->p_rs);
        if (4 == argc)
        {
            rc = _if_mgr_cmd_set_brgif_vlan_mapping_table(&brgif, p_db_if, vlan_type, FALSE, para);
        }
        else if (sal_strlen(argv[i+2]) <= GLB_VLAN_MAPPING_TAB_NAME_MAX)
        {
            sal_strncpy(brgif.vlan_mapping_table, argv[i+2], GLB_VLAN_MAPPING_TAB_NAME_MAX);
            rc = _if_mgr_cmd_set_brgif_vlan_mapping_table(&brgif, p_db_if, vlan_type, TRUE, para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("wrong table");
        }
        break;
    case DS_BRGIF_FLD_QINQ_DEFAULT_FWD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
        brgif.qinq_default_fwd = value;
        rc = _if_mgr_cmd_set_brgif_qinq_default_fwd(&brgif, p_db_if, para);
        break;
    case DS_BRGIF_FLD_VLAN_TRANS_DEFAULT_FWD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
        brgif.vlan_trans_default_fwd = value;
        rc = _if_mgr_cmd_set_brgif_vlan_trans_default_fwd(&brgif, p_db_if, para);
        break;
    case DS_BRGIF_FLD_DOT1Q_TAG_NATIVE_EN:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
        brgif.dot1q_tag_native_en = value;
        rc = _if_mgr_cmd_set_brgif_dot1q_tag_native_en(&brgif, p_db_if, para);
        break;
    case DS_BRGIF_FLD_TPID_TYPE:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
        brgif.tpid_type = value;
        rc = _if_mgr_cmd_set_brgif_tpid_type(&brgif, p_db_if, para);
        break;
    case DS_BRGIF_FLD_INGRESS_FILTER:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
        brgif.ingress_filter = value;
        rc = _if_mgr_cmd_set_brgif_ingress_filter(&brgif, p_db_if, para);
        break;
    case DS_BRGIF_FLD_MAC_LEARNING_EN:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
        brgif.mac_learning_en = value;
        rc = _if_mgr_cmd_set_brgif_mac_learning_en(&brgif, p_db_if, para);
        break;
    case DS_BRGIF_FLD_PORT_SECURITY_EN:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
        brgif.port_security_en = value;
        rc = _if_mgr_cmd_set_brgif_port_security_en(&brgif, p_db_if, para);
        break;
    case DS_BRGIF_FLD_MAX_MAC:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
        brgif.max_mac = value;
        rc = _if_mgr_cmd_set_brgif_port_security_mac_maxinum(&brgif, p_db_if, para);
        break;
    case DS_BRGIF_FLD_VIOLATE_MODE:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
        brgif.violate_mode = value;
        rc = _if_mgr_cmd_set_brgif_port_security_violation(&brgif, p_db_if, para);
        break;
    default:
        CFG_FUNC_ERR_RET("Unknown support changed field %s", argv[i]);
        break;
    }

    return rc;
}
#endif /* CONFIG_L2 */

static int32
_if_mgr_cmd_get_opf_intf(char *ifname, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE            *fp = NULL;
    tbl_iter_args_t args;

    sal_memset(&args, 0 ,sizeof(args));
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = ifname;
    
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_openflow_interface_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_if_mgr_cmd_get_opf_tunnel_intf(char *ifname, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE            *fp = NULL;
    tbl_iter_args_t args;

    sal_memset(&args, 0 ,sizeof(args));
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = ifname;
    
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_openflow_tunnel_interface_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_if_mgr_cmd_set_openflow_enable(tbl_interface_t *p_db_if, int32 enable, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
#ifdef CONFIG_OPENFLOW
    ds_openflow_if_t *p_db_of_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);

    /* 1. config check */
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", ifname_ext);
    }
        
    /* 2. check duplicate config */
    p_db_of_if = p_db_if->openflow_if;
    if (enable)
    {
        if (p_db_of_if)
        {
            return PM_E_NONE;
        }
        rc = if_mgr_set_openflow_enable(p_db_if);
    }
    else
    {
        if (NULL == p_db_of_if)
        {
            return PM_E_NONE;
        }
        rc = if_mgr_set_openflow_disable(p_db_if);
    }
#else
    CFG_FUNC_UNSUPPORT_OPENFLOW();
#endif /* CONFIG_OPENFLOW */

    return rc;
}

int32 if_mgr_openflow_add_tunnel_interface_check_param(uint32 tunnel_type, tbl_openflow_tunnel_interface_t *p_openflow_tunnel_intf , cfg_cmd_para_t *para)
{
    tbl_openflow_tunnel_interface_master_t *p_master =NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_openflow_tunnel_interface_t *p_db_tunnel_if = NULL;
    tbl_interface_t *p_db_if = NULL;

    p_master = tbl_openflow_tunnel_interface_get_master();
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->tunnel_if_list, p_db_tunnel_if, listnode, next)
    {
        p_db_if = tbl_interface_get_interface_by_name(p_db_tunnel_if->key.name);
        if(p_db_if)
        {
            if(tunnel_type == p_db_if->hw_type)
            {
                if(p_db_tunnel_if->openflow_enable)
                {
                    if (0 == strcmp(p_db_tunnel_if->remote_ip, p_openflow_tunnel_intf->remote_ip))
                    {
                         CFG_INVALID_PARAM_RET("This remote ip is exist in this type tunnel");
                    }
                }
            }
        }
    }

    return 0;
}

/* modified by liwh for bug 46651, 2019-03-06 */
void
if_mgr_set_dynamic_tunnel_cnt(int32 add)
{
    tbl_sys_spec_key_t     sys_spec_key;
    tbl_sys_spec_t            *p_sys_spec = NULL;

    sys_spec_key.type = GLB_STM_MODE_OF_HYBRID;
    p_sys_spec = tbl_sys_spec_get_sys_spec(&sys_spec_key);
    if (p_sys_spec)
    {
        if (add)
        {
            p_sys_spec->openflow_dynamic_tunnel_num = p_sys_spec->openflow_dynamic_tunnel_num + 1;    
        }
        else
        {
            p_sys_spec->openflow_dynamic_tunnel_num = p_sys_spec->openflow_dynamic_tunnel_num - 1;    
        } 

        tbl_sys_spec_set_sys_spec_field(p_sys_spec, TBL_SYS_SPEC_FLD_OPENFLOW_DYNAMIC_TUNNEL_NUM);
    }
    
    return;
}

uint32
if_mgr_get_l2edit_cnt(void)
{
    uint32 cnt =0;
    tbl_sys_spec_key_t     sys_spec_key;
    tbl_sys_spec_t            *p_sys_spec = NULL;
    tbl_l2edit_master_t     *p_l2edit_master = NULL;

    sys_spec_key.type = GLB_STM_MODE_OF_HYBRID;
    p_sys_spec = tbl_sys_spec_get_sys_spec(&sys_spec_key);
    if (p_sys_spec)
    {
        cnt = p_sys_spec->openflow_dynamic_tunnel_num;    
    }

    p_l2edit_master = tbl_l2edit_get_master();
    if (p_l2edit_master)
    {
        cnt = cnt + p_l2edit_master->l2edit_hash->count;
    }

    cnt = cnt *2;

    return cnt;
}

void
if_mgr_openflow_get_l2edit_key(tbl_openflow_tunnel_interface_t *p_openflow_tunnel_intf, tbl_l2edit_key_t *p_l2edit_key)
{
    tbl_interface_t    *p_db_bind_if = NULL;
    mac_addr_t          dest_mac;

    if ((NULL == p_openflow_tunnel_intf) || (NULL == p_l2edit_key))
    {
        return;
    }
    
    p_db_bind_if = tbl_interface_get_interface_by_name(p_openflow_tunnel_intf->bind_port_name);
    if (!p_db_bind_if)
    {
        return;
    }

    sal_sscanf(p_openflow_tunnel_intf->bind_mac, "%hx.%hx.%hx",
             (unsigned short *)&dest_mac[0],
             (unsigned short *)&dest_mac[2],
             (unsigned short *)&dest_mac[4]);
    *((unsigned short *)&dest_mac[0]) = sal_ntoh16(*((unsigned short *)&dest_mac[0]));
    *((unsigned short *)&dest_mac[2]) = sal_ntoh16(*((unsigned short *)&dest_mac[2]));
    *((unsigned short *)&dest_mac[4]) = sal_ntoh16(*((unsigned short *)&dest_mac[4]));

    sal_memcpy(p_l2edit_key->dest_mac, dest_mac, GLB_ETH_ADDR_LEN);          
    sal_memcpy(p_l2edit_key->src_mac, p_db_bind_if->mac_addr, GLB_ETH_ADDR_LEN);
    p_l2edit_key->fid = p_openflow_tunnel_intf->bind_vlan;

    return;
}
/* liwh end */

int32 if_mgr_openflow_tunnel_interface_openflow_en(char* interface_name, uint32 openflow_enable, cfg_cmd_para_t *para)
{
    tbl_openflow_tunnel_interface_t *p_openflow_tunnel_intf = NULL;
    tbl_openflow_tunnel_interface_t openflow_tunnel_intf;
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_t *p_db_if = NULL;
    char cmd[MAX_CMD_STR_LEN] = {0};
    int ret = 0;
    char port_type[GLB_OPENFLOW_TUNNEL_TYPE_LEN];
    int32 rc = PM_E_NONE;
    /* modified by liwh for bug 46651, 2019-03-06 */
    uint32                l2edit_num = 0; 
    tbl_l2edit_t         *p_l2edit = NULL;
    tbl_l2edit_key_t   l2edit_key;
    tbl_l2edit_t          l2edit_entry; 
    /* liwh end */

    sal_memset(&openflow_tunnel_intf, 0, sizeof(openflow_tunnel_intf));
    sal_strncpy(openflow_tunnel_intf.key.name, interface_name, IFNAME_SIZE);
    p_openflow_tunnel_intf = tbl_openflow_tunnel_interface_get_openflow_tunnel_interface(&openflow_tunnel_intf.key);
    if(NULL == p_openflow_tunnel_intf)
    {
        CFG_CONFLICT_RET("This interface is a tunnel interface");
    }

    p_db_if = tbl_interface_get_interface_by_name(interface_name);
    if (p_db_if->hw_type == GLB_IF_TYPE_L3GRE_IF)
    {
        CFG_CONFLICT_RET("Now L3gre is not supported");
    }
    switch(p_db_if->hw_type)
    {
        case GLB_IF_TYPE_L2GRE_IF:
            sal_strncpy(port_type, "l2gre", GLB_OPENFLOW_TUNNEL_TYPE_LEN);
            break;
        case GLB_IF_TYPE_NVGRE_IF:
            sal_strncpy(port_type, "nvgre", GLB_OPENFLOW_TUNNEL_TYPE_LEN);
            break;
        case GLB_IF_TYPE_VXLAN_IF:
            sal_strncpy(port_type, "vxlan", GLB_OPENFLOW_TUNNEL_TYPE_LEN);
            break;
        default:
            CFG_CONFLICT_RET("This interface type is not supported");
            break;
    }
    IFNAME_ETH2FULL(p_openflow_tunnel_intf->bind_port_name, ifname_ext);
    if(openflow_enable == p_openflow_tunnel_intf->openflow_enable)
    {
        return PM_E_NONE;
    }

    /* modified by liwh for bug 46651, 2019-03-06 */
    if (openflow_enable && ((p_db_if->hw_type == GLB_IF_TYPE_VXLAN_IF) || (p_db_if->hw_type == GLB_IF_TYPE_L2GRE_IF) || (p_db_if->hw_type == GLB_IF_TYPE_NVGRE_IF)))
    {
        /* get l2edit number */
        l2edit_num = if_mgr_get_l2edit_cnt();
        
        /* for dynamic vxlan, reserve l2edit resource */
        if (!GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_BIND_PORT))
        {
            if ( (l2edit_num + 2 > GLB_OPENFLOW_L2EDIT_MAX))
            {
                CFG_CONFLICT_RET("Can't add dynamic vxlan because there is no l2edit resource");    
            }
        }
        /* for static vxlan, check l2 edit resource */
        else
        {
            /* find whether l2edit is exist */ 
            if_mgr_openflow_get_l2edit_key(p_openflow_tunnel_intf, &l2edit_key);
            p_l2edit = tbl_l2edit_get_l2edit(&l2edit_key);

            /* if l2edit is not exist, check l2edit resource */
            if  (!p_l2edit)
            {
                if (l2edit_num + 2 > GLB_OPENFLOW_L2EDIT_MAX)
                {
                    CFG_CONFLICT_RET("Can't add static vxlan because there is no l2edit resource");    
                }   
            }
        }
    }
    /* liwh end */
    
    if(!openflow_enable)
    {
        /* added by hansf for netconf */
        if (CDB_CFG_NC_CHECK == para->cfg)
        {
            return PM_E_NONE;
        }
        /* ended by hansf */
        
        sal_sprintf(cmd, "ovs-vsctl del-port-centec br0 %s", p_openflow_tunnel_intf->key.name);
        ret = system(cmd);
        if(ret != 0)
        {
            CFG_INVALID_PARAM_RET("Can not delete the interface successfully");
        }
    }
    else
    {

    #ifdef TTPPRODUCT
        if((!GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_DSTIP))
            ||(!GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_SRCIP)))
        {
            CFG_INVALID_PARAM_RET("Should configure src ip, remote ip before enable openflow");
        }

        //ret = if_mgr_openflow_add_tunnel_interface_check_param(p_db_if->hw_type, p_openflow_tunnel_intf, para);
        if(ret)
        {
            return ret;
        }
        
        /* added by hansf for netconf */
        if (CDB_CFG_NC_CHECK == para->cfg)
        {
            return PM_E_NONE;
        }
        /* ended by hansf */
        
        sal_sprintf(cmd, "ovs-vsctl add-port-centec br0 %s -- set interface %s type=%s options:remote_ip=%s options:local_ip=%s", \
            p_openflow_tunnel_intf->key.name, p_openflow_tunnel_intf->key.name, port_type, p_openflow_tunnel_intf->remote_ip, p_openflow_tunnel_intf->source_ip);
    #else
        if((!GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_DSTIP))
            ||(!GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_SRCIP)))
        {
            CFG_INVALID_PARAM_RET("Should configure src ip, remote ip, before enable openflow");
        }
        ret = if_mgr_openflow_add_tunnel_interface_check_param(p_db_if->hw_type, p_openflow_tunnel_intf, para);
        if(ret)
        {
            return ret;
        }
        if(GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_BIND_PORT))
        {
            if(GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_BIND_VLAN))
            {
                sal_sprintf(cmd, "ovs-vsctl add-port-centec br0 %s -- set interface %s type=%s options:remote_ip=%s options:bind_port=%s options:nexthop_mac=%s options:local_ip=%s options:vlan_id=%d",
                p_openflow_tunnel_intf->key.name, p_openflow_tunnel_intf->key.name, port_type, p_openflow_tunnel_intf->remote_ip, ifname_ext, 
                p_openflow_tunnel_intf->bind_mac, p_openflow_tunnel_intf->source_ip, p_openflow_tunnel_intf->bind_vlan);

            }
            else
            {
                sal_sprintf(cmd, "ovs-vsctl add-port-centec br0 %s -- set interface %s type=%s options:remote_ip=%s options:bind_port=%s options:nexthop_mac=%s options:local_ip=%s",
                p_openflow_tunnel_intf->key.name, p_openflow_tunnel_intf->key.name, port_type, p_openflow_tunnel_intf->remote_ip, ifname_ext, p_openflow_tunnel_intf->bind_mac, p_openflow_tunnel_intf->source_ip);
            }
        }
        else
        {
            sal_sprintf(cmd, "ovs-vsctl add-port-centec br0 %s -- set interface %s type=%s options:remote_ip=%s options:local_ip=%s",
            p_openflow_tunnel_intf->key.name, p_openflow_tunnel_intf->key.name, port_type, p_openflow_tunnel_intf->remote_ip, p_openflow_tunnel_intf->source_ip);
        }

    #endif

        ret = system(cmd);
        if(ret)
        {
            CFG_INVALID_PARAM_RET("Can not add or delete the interface successfully, please check the ip and port");
        }
    }

    /* modified by liwh for bug 46651, 2019-03-06 */
    if ((p_db_if->hw_type == GLB_IF_TYPE_VXLAN_IF) || (p_db_if->hw_type == GLB_IF_TYPE_L2GRE_IF) || (p_db_if->hw_type == GLB_IF_TYPE_NVGRE_IF))
    {
        /* count dynamic vxlan interface */
        if (!GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_BIND_PORT))
        {         
            if_mgr_set_dynamic_tunnel_cnt(openflow_enable);
        }
        /* reserve l2edit entry for static vxlan interface */
        else
        {
            if (openflow_enable)
            {
                 if  (!p_l2edit)
                 {
                     sal_memset(&l2edit_entry, 0, sizeof(tbl_l2edit_t));
                     sal_memcpy(&l2edit_entry.key, &l2edit_key, sizeof(tbl_l2edit_key_t));
                     l2edit_entry.count = 1;
                     tbl_l2edit_add_l2edit(&l2edit_entry);
                 }
                 else
                 {
                     p_l2edit->count++;  
                 }
            }
            else 
            {
                /* find whether l2edit is exist */ 
                if_mgr_openflow_get_l2edit_key(p_openflow_tunnel_intf, &l2edit_key);
                p_l2edit = tbl_l2edit_get_l2edit(&l2edit_key);
                if (p_l2edit)
                {
                    p_l2edit->count--;                    
                    if (0 == p_l2edit->count)
                    {
                        tbl_l2edit_del_l2edit(&l2edit_key);
                    }
                }
            }
        }
    }
    /* liwh end */

    p_openflow_tunnel_intf->openflow_enable = openflow_enable;
    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_OPENFLOW_ENABLE);

    return rc;
}

static int32
_if_mgr_cmd_set_arpinsp_trust_enable(tbl_interface_t* p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];
    
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    
    /* 1. config check */
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port or LAG", ifname_ext);
    }
    if (GLB_IF_MODE_L2 != p_db_if->mode)
    {
        CFG_CONFLICT_RET("Interface %s is not switchport", ifname_ext);
    }
    
    /*2. duplicate check*/
    if (p_if->arpinsp_trust_enable == p_db_if->arpinsp_trust_enable)
    {
        return PM_E_NONE;
    }

    /*3. call API*/
    rc = if_mgr_set_arpinsp_trust_enable(p_if, p_if->arpinsp_trust_enable, TRUE);
    return rc;
}

static int32
_if_mgr_cmd_set_arp_numberlimit_enable(tbl_interface_t* p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];
    
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    
    /* 1. config check */
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type && GLB_IF_TYPE_VLAN_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port or LAG or vlanif", ifname_ext);
    }
    if (GLB_IF_MODE_L3 != p_db_if->mode)
    {
        CFG_CONFLICT_RET("Interface %s is not L3 port", ifname_ext);
    }
    
    /*2. duplicate check*/
    if (p_if->arp_numberlimit_enable == p_db_if->arp_numberlimit_enable)
    {
        return PM_E_NONE;
    }
    
    /*3. call API*/
    rc = if_mgr_set_arp_numberlimit_enable(p_if, p_if->arp_numberlimit_enable);
    return rc;
}

static int32
_if_mgr_cmd_set_arp_numberlimit_number(tbl_interface_t* p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];
    
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    
    /* 1. config check */
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type && GLB_IF_TYPE_VLAN_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port or LAG or vlanif", ifname_ext);
    }
    if (GLB_IF_MODE_L3 != p_db_if->mode)
    {
        CFG_CONFLICT_RET("Interface %s is not L3 port", ifname_ext);
    }
    
    /*2. duplicate check*/
    if (p_if->arp_numberlimit_number == p_db_if->arp_numberlimit_number)
    {
        return PM_E_NONE;
    }
    
    /*3. call API*/
    rc = if_mgr_set_arp_numberlimit_number(p_if, p_if->arp_numberlimit_number);
    return rc;
}

static int32
_if_mgr_cmd_set_arp_numberlimit_violate(tbl_interface_t* p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];
    
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    
    /* 1. config check */
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type && GLB_IF_TYPE_VLAN_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port or LAG or vlanif", ifname_ext);
    }
    if (GLB_IF_MODE_L3 != p_db_if->mode)
    {
        CFG_CONFLICT_RET("Interface %s is not L3 port", ifname_ext);
    }
    
    /*2. duplicate check*/
    if (p_if->arp_numberlimit_violate == p_db_if->arp_numberlimit_violate)
    {
        return PM_E_NONE;
    }
    
    /*3. call API*/
    rc = if_mgr_set_arp_numberlimit_violate(p_if, p_if->arp_numberlimit_violate);
    return rc;
}

int32
if_mgr_cmd_process_openflow_tunnel_interface(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_openflow_tunnel_interface_t openflow_tunnel_intf;
    tbl_openflow_tunnel_interface_t *p_openflow_tunnel_intf = NULL;
    tbl_openflow_interface_t openflow_intf;
    tbl_openflow_interface_t *p_openflow_intf = NULL;

    int32 field_id = 0;
    cdb_field_t *p_field = NULL;


    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;
    uint32 value = 0;
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_t intf;
    field_parse_t   field;
    char cmd[MAX_CMD_STR_LEN] = {0};
    int ret = 0;
    char port_type[GLB_OPENFLOW_TUNNEL_TYPE_LEN];
    uint32 tunnel_current_ipsa_cnt = 0;
    tbl_openflow_tunnel_local_ip_cnt_t tunnel_local_ip;
    tbl_openflow_tunnel_local_ip_cnt_t* p_tunnel_local_ip = NULL;

    tbl_openflow_tunnel_local_ip_cnt_t tunnel_local_ip_org;
    tbl_openflow_tunnel_local_ip_cnt_t* p_tunnel_local_ip_org = NULL;
    uint32 flag_set = 0;
    /* modified by liwh for bug 43521, 2019-03-11 */
    addr_ipv4_t ipv4_addr;
    uint32      ntoh_s_addr = 0;
    /* liwh end */
    /* modified by liwh for bug 46644, 2019-03-11 */
    mac_addr_t          dest_mac;
    /* liwh end */

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_OPENFLOW_TUNNEL_INTERFACE);
    switch (para->oper)
    {
        case CDB_OPER_ADD:
            break;
        case CDB_OPER_DEL:
            sal_memset(&openflow_tunnel_intf, 0, sizeof(openflow_tunnel_intf));
            sal_strncpy(openflow_tunnel_intf.key.name, argv[0], IFNAME_SIZE);
            p_openflow_tunnel_intf = tbl_openflow_tunnel_interface_get_openflow_tunnel_interface(&openflow_tunnel_intf.key);
            if(NULL == p_openflow_tunnel_intf)
            {
                CFG_CONFLICT_RET("This interface is a tunnel interface");
            }
            field_id = cdb_get_field_id(argv[1], p_node, &p_field);
            switch(field_id)
            {
                case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_PORT_NAME:
                    if(p_openflow_tunnel_intf->openflow_enable)
                    {
                        CFG_INVALID_PARAM_RET("Openflow is enable at the port, if you want to modify options, please set openflow disable first");
                    }                

                    /* modified by liwh for bug 48828, 2018-02-25 */
                    sal_memset(&openflow_intf, 0, sizeof(openflow_intf));
                    sal_strncpy(openflow_intf.key.name, p_openflow_tunnel_intf->bind_port_name, IFNAME_SIZE);
                    p_openflow_intf = tbl_openflow_interface_get_openflow_interface(&openflow_intf.key);
                    if (p_openflow_intf)
                    {
                        p_openflow_intf->is_bind_tunnel = FALSE;
                        tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_IS_BIND_TUNNEL);
                    }
                    /* liwh end */
                        
                    sal_strncpy(p_openflow_tunnel_intf->bind_port_name, "", IFNAME_SIZE);
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_PORT_NAME);
                    sal_strncpy(p_openflow_tunnel_intf->bind_mac, "", MACADDR_BUF_SIZE);
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_MAC);
                    p_openflow_tunnel_intf->bind_vlan = 0;
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_VLAN);
                    GLB_UNSET_FLAG(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_BIND_PORT);
                    GLB_UNSET_FLAG(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_BIND_MAC);
                    GLB_UNSET_FLAG(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_BIND_VLAN);
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_FLAG);
                    break;

                case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_REMOTE_IP:
                    if(p_openflow_tunnel_intf->openflow_enable)
                    {
                        CFG_INVALID_PARAM_RET("Openflow is enable at the port, if you want to modify options, please set openflow disable first");
                    }
                    if(!GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_DSTIP))
                    {
                        CFG_INVALID_PARAM_RET("No remote ip set at this port");
                    }
                    sal_strncpy(p_openflow_tunnel_intf->remote_ip, "", IPADDR_BUF_SIZE);
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_REMOTE_IP);
                    GLB_UNSET_FLAG(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_DSTIP);
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_FLAG);
                    break;

                case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_SOURCE_IP:
                    if(p_openflow_tunnel_intf->openflow_enable)
                    {
                        CFG_INVALID_PARAM_RET("Openflow is enable at the port, if you want to modify options, please set openflow disable first");
                    }
                    if(!GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_SRCIP))
                    {
                        CFG_INVALID_PARAM_RET("No local ip set at this port");
                    }
                    sal_memset(&tunnel_local_ip, 0x0, sizeof(tbl_openflow_tunnel_local_ip_cnt_t));
                    sal_memset(&tunnel_local_ip.key, 0x0, sizeof(tbl_openflow_tunnel_local_ip_cnt_key_t));
                    sal_strncpy(tunnel_local_ip.key.local_ip, p_openflow_tunnel_intf->source_ip, IPADDR_BUF_SIZE);
                    p_tunnel_local_ip = tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt(&tunnel_local_ip.key);
                    if(p_tunnel_local_ip)
                    {
                        p_tunnel_local_ip->local_ip_ref --;
                        if(0 == p_tunnel_local_ip->local_ip_ref)
                        {
                            tbl_openflow_tunnel_local_ip_cnt_del_openflow_tunnel_local_ip_cnt(&tunnel_local_ip.key);
                        }
                    }
                    sal_strncpy(p_openflow_tunnel_intf->source_ip, "", IPADDR_BUF_SIZE);
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_SOURCE_IP);
                    GLB_UNSET_FLAG(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_SRCIP);
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_FLAG);
                    break;
                 default:
                    break;
            }
            break;
        case CDB_OPER_SET:
            sal_memset(&openflow_tunnel_intf, 0, sizeof(openflow_tunnel_intf));
            sal_strncpy(openflow_tunnel_intf.key.name, argv[0], IFNAME_SIZE);
            sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);
            field_id = cdb_get_field_id(argv[1], p_node, &p_field);
            /* check exist */
            p_openflow_tunnel_intf = tbl_openflow_tunnel_interface_get_openflow_tunnel_interface(&openflow_tunnel_intf.key);
            
            p_db_if = tbl_interface_get_interface(&intf.key);
            if ((!p_openflow_tunnel_intf) || (!p_db_if))
            {
                CFG_CONFLICT_RET("This interface is not exit");
            }
            switch(p_db_if->hw_type)
            {
                case GLB_IF_TYPE_L2GRE_IF:
                    sal_strncpy(port_type, "l2gre", GLB_OPENFLOW_TUNNEL_TYPE_LEN);
                    break;
                case GLB_IF_TYPE_NVGRE_IF:
                    sal_strncpy(port_type, "nvgre", GLB_OPENFLOW_TUNNEL_TYPE_LEN);
                    break;
                case GLB_IF_TYPE_VXLAN_IF:
                    sal_strncpy(port_type, "vxlan", GLB_OPENFLOW_TUNNEL_TYPE_LEN);
                    break;
                default:
                    CFG_CONFLICT_RET("This interface type is not supported");
                    break;
            }
            switch(field_id)
            {
                case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_OPENFLOW_ENABLE:
                    if (p_db_if->hw_type == GLB_IF_TYPE_L3GRE_IF)
                    {
                        CFG_CONFLICT_RET("Now L3gre is not supported");
                    }
                    IFNAME_ETH2FULL(p_openflow_tunnel_intf->bind_port_name, ifname_ext);
                    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                    if(value == p_openflow_tunnel_intf->openflow_enable)
                    {
                        return rc;
                    }
                    if(!value)
                    {
                        /* added by hansf for netconf */
                        if (CDB_CFG_NC_CHECK == para->cfg)
                        {
                            return PM_E_NONE;
                        }
                        /* ended by hansf */
                        
                        sal_sprintf(cmd, "ovs-vsctl del-port-centec br0 %s", p_openflow_tunnel_intf->key.name);
                        ret = system(cmd);
                        if(ret != 0)
                        {
                            CFG_INVALID_PARAM_RET("Can not delete the interface successfully");
                        }
                    }
                    else
                    {
                    /*modify by hansf , ttp product get nexthop info from arp and route */
                    #ifdef TTPPRODUCT
                        if((!GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_DSTIP))
                            ||(!GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_SRCIP)))
                        {
                            CFG_INVALID_PARAM_RET("Should configure src ip, remote ip before enable openflow");
                        }

                        /* added by hansf for netconf */
                        if (CDB_CFG_NC_CHECK == para->cfg)
                        {
                            return PM_E_NONE;
                        }
                        /* ended by hansf */
                        
                        sal_sprintf(cmd, "ovs-vsctl add-port-centec br0 %s -- set interface %s type=%s options:remote_ip=%s options:local_ip=%s", \
                            p_openflow_tunnel_intf->key.name, p_openflow_tunnel_intf->key.name, port_type, p_openflow_tunnel_intf->remote_ip, p_openflow_tunnel_intf->source_ip);
                    #else
                        if((!GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_DSTIP))
                            ||(!GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_SRCIP)))
                        {
                            CFG_INVALID_PARAM_RET("Should configure src ip, remote ip, before enable openflow");
                        }
                        if(GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_BIND_PORT))
                        {
                            if(GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_BIND_VLAN))
                            {
                                sal_sprintf(cmd, "ovs-vsctl add-port-centec br0 %s -- set interface %s type=%s options:remote_ip=%s options:bind_port=%s options:nexthop_mac=%s options:local_ip=%s options:vlan_id=%d",
                                p_openflow_tunnel_intf->key.name, p_openflow_tunnel_intf->key.name, port_type, p_openflow_tunnel_intf->remote_ip, ifname_ext, 
                                p_openflow_tunnel_intf->bind_mac, p_openflow_tunnel_intf->source_ip, p_openflow_tunnel_intf->bind_vlan);

                            }
                            else
                            {
                                sal_sprintf(cmd, "ovs-vsctl add-port-centec br0 %s -- set interface %s type=%s options:remote_ip=%s options:bind_port=%s options:nexthop_mac=%s options:local_ip=%s",
                                p_openflow_tunnel_intf->key.name, p_openflow_tunnel_intf->key.name, port_type, p_openflow_tunnel_intf->remote_ip, ifname_ext, p_openflow_tunnel_intf->bind_mac, p_openflow_tunnel_intf->source_ip);
                            }
                        }
                        else
                        {
                                sal_sprintf(cmd, "ovs-vsctl add-port-centec br0 %s -- set interface %s type=%s options:remote_ip=%s options:local_ip=%s",
                                p_openflow_tunnel_intf->key.name, p_openflow_tunnel_intf->key.name, port_type, p_openflow_tunnel_intf->remote_ip, p_openflow_tunnel_intf->source_ip);
                        }

                    #endif
                        
                        ret = system(cmd);
                    }
                    p_openflow_tunnel_intf->openflow_enable = value;
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_OPENFLOW_ENABLE);
                    break;
                case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_REMOTE_IP:
                    if(p_openflow_tunnel_intf->openflow_enable)
                    {
                        CFG_INVALID_PARAM_RET("Openflow is enable at the port, if you want to modify options, please set openflow disable first");
                    }

                    /* modified by liwh for bug 43521, 2019-03-11 */
                    sal_inet_pton(AF_INET, argv[2], &ipv4_addr);
                    ntoh_s_addr = sal_ntoh32(ipv4_addr.s_addr);
                
                    /* Check IP address */
                    if (IN_EXPERIMENTAL(ntoh_s_addr) 
                        || IN_CLASSD(ntoh_s_addr)
                        || IN_LOOPBACK(ntoh_s_addr)
                        || IPV4_NET0(ntoh_s_addr)
                        || IPV4_LINKLOCAL(ntoh_s_addr))
                    {
                        CFG_INVALID_PARAM_RET("Invalid IPv4 address");
                    }
                    /* liwh end */

                    /* modified by liwh for bug 49180, 2019-02-27 */
                    if  (GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_SRCIP)
                        && (sal_strlen(p_openflow_tunnel_intf->source_ip) == sal_strlen(argv[2]))
                        && (0 == sal_strcmp(p_openflow_tunnel_intf->source_ip, argv[2])))
                    {
                        CFG_INVALID_PARAM_RET("Tunnel-remote-ip can't be same as tunnel-source-ip");
                    }
                    /* liwh end */

                    /* added by hansf for netconf */
                    if (CDB_CFG_NC_CHECK == para->cfg)
                    {
                        return PM_E_NONE;
                    }
                    /* ended by hansf */
                    sal_strncpy(p_openflow_tunnel_intf->remote_ip, argv[2], IPADDR_BUF_SIZE);
                    GLB_SET_FLAG(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_DSTIP);
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_REMOTE_IP);
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_FLAG);
                    break;
               case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_SOURCE_IP:
                    if(p_openflow_tunnel_intf->openflow_enable)
                    {
                        CFG_INVALID_PARAM_RET("Openflow is enable at the port, if you want to modify options, please set openflow disable first");
                    }

                    /* modified by liwh for bug 43521, 2019-03-11 */
                    sal_inet_pton(AF_INET, argv[2], &ipv4_addr);
                    ntoh_s_addr = sal_ntoh32(ipv4_addr.s_addr);
                
                    /* Check IP address */
                    if (IN_EXPERIMENTAL(ntoh_s_addr) 
                        || IN_CLASSD(ntoh_s_addr)
                        || IN_LOOPBACK(ntoh_s_addr)
                        || IPV4_NET0(ntoh_s_addr)
                        || IPV4_LINKLOCAL(ntoh_s_addr))
                    {
                        CFG_INVALID_PARAM_RET("Invalid IPv4 address");
                    }
                    /* liwh end */

                    /* modified by liwh for bug 49180, 2019-02-27 */
                    if  (GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_DSTIP)
                        && (sal_strlen(p_openflow_tunnel_intf->remote_ip) == sal_strlen(argv[2]))
                        && (0 == sal_strcmp(p_openflow_tunnel_intf->remote_ip, argv[2])))
                    {
                        CFG_INVALID_PARAM_RET("Tunnel-source-ip can't be same as tunnel-remote-ip");
                    }
                    /* liwh end */
   
                    

                    /* added by hansf for netconf */
                    if (CDB_CFG_NC_CHECK == para->cfg)
                    {
                        if(GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_SRCIP))
                        {
                            sal_memset(&tunnel_local_ip_org, 0x0, sizeof(tbl_openflow_tunnel_local_ip_cnt_t));
                            sal_memset(&tunnel_local_ip_org.key, 0x0, sizeof(tbl_openflow_tunnel_local_ip_cnt_key_t));
                            sal_strncpy(tunnel_local_ip_org.key.local_ip, p_openflow_tunnel_intf->source_ip, IPADDR_BUF_SIZE);
                            p_tunnel_local_ip_org = tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt(&tunnel_local_ip_org.key);
                            if(p_tunnel_local_ip_org)
                            {
                                if(1 == p_tunnel_local_ip_org->local_ip_ref)
                                {
                                    flag_set = 1;
                                }
                            }
                        }
                        /*check ipsa res*/
                        sal_memset(&tunnel_local_ip, 0x0, sizeof(tbl_openflow_tunnel_local_ip_cnt_t));
                        sal_memset(&tunnel_local_ip.key, 0x0, sizeof(tbl_openflow_tunnel_local_ip_cnt_key_t));
                        sal_strncpy(tunnel_local_ip.key.local_ip, argv[2], IPADDR_BUF_SIZE);
                        p_tunnel_local_ip = tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt(&tunnel_local_ip.key);
                        if(!p_tunnel_local_ip)
                        {
                            tunnel_current_ipsa_cnt = tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt_count();
                            if(GLB_OPENFLOW_TUNNEL_LOCAL_IP_CNT_MAX == tunnel_current_ipsa_cnt && flag_set == 0)
                            {
                                CFG_INVALID_PARAM_RET("The kind of local ip can not be more than 4");
                            }
                        }
                        
                        return PM_E_NONE;
                    }
                    else
                    {
                        if(GLB_FLAG_ISSET(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_SRCIP))
                        {
                            sal_memset(&tunnel_local_ip_org, 0x0, sizeof(tbl_openflow_tunnel_local_ip_cnt_t));
                            sal_memset(&tunnel_local_ip_org.key, 0x0, sizeof(tbl_openflow_tunnel_local_ip_cnt_key_t));
                            sal_strncpy(tunnel_local_ip_org.key.local_ip, p_openflow_tunnel_intf->source_ip, IPADDR_BUF_SIZE);
                            p_tunnel_local_ip_org = tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt(&tunnel_local_ip_org.key);
                            if(p_tunnel_local_ip_org)
                            {
                                p_tunnel_local_ip_org->local_ip_ref --;
                                if(0 == p_tunnel_local_ip_org->local_ip_ref)
                                {
                                    tbl_openflow_tunnel_local_ip_cnt_del_openflow_tunnel_local_ip_cnt(&tunnel_local_ip_org.key);
                                }
                            }
                        }
                        /*check ipsa res*/
                        sal_memset(&tunnel_local_ip, 0x0, sizeof(tbl_openflow_tunnel_local_ip_cnt_t));
                        sal_memset(&tunnel_local_ip.key, 0x0, sizeof(tbl_openflow_tunnel_local_ip_cnt_key_t));
                        sal_strncpy(tunnel_local_ip.key.local_ip, argv[2], IPADDR_BUF_SIZE);
                        p_tunnel_local_ip = tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt(&tunnel_local_ip.key);
                        if(!p_tunnel_local_ip)
                        {
                            tunnel_current_ipsa_cnt = tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt_count();
                            if(GLB_OPENFLOW_TUNNEL_LOCAL_IP_CNT_MAX == tunnel_current_ipsa_cnt)
                            {
                                CFG_INVALID_PARAM_RET("The kind of local ip can not be more than 4");
                            }
                            tunnel_local_ip.local_ip_ref = 0;
                            tbl_openflow_tunnel_local_ip_cnt_add_openflow_tunnel_local_ip_cnt(&tunnel_local_ip);
                            p_tunnel_local_ip = tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt(&tunnel_local_ip.key);
                        }
                        p_tunnel_local_ip->local_ip_ref ++;
                        tbl_openflow_tunnel_local_ip_cnt_set_openflow_tunnel_local_ip_cnt_field(p_tunnel_local_ip, TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_LOCAL_IP_REF);
                        sal_strncpy(p_openflow_tunnel_intf->source_ip, argv[2], IPADDR_BUF_SIZE);
                        GLB_SET_FLAG(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_SRCIP);
                        tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_SOURCE_IP);
                        tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_FLAG);
                    }
                    break;
               case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_PORT_NAME:
                    if(p_openflow_tunnel_intf->openflow_enable)
                    {
                        CFG_INVALID_PARAM_RET("Openflow is enable at the port, if you want to modify options, please set openflow disable first");
                    }
                    sal_memset(&openflow_intf, 0, sizeof(openflow_intf));
                    sal_strncpy(openflow_intf.key.name, argv[2], IFNAME_SIZE);
                    p_openflow_intf = tbl_openflow_interface_get_openflow_interface(&openflow_intf.key);
                    if(NULL == p_openflow_intf)
                    {
                        CFG_INVALID_PARAM_RET("this bind port is not enable openflow");
                    }
                    if(!p_openflow_intf->openflow_enable)
                    {
                        CFG_INVALID_PARAM_RET("this bind port is not enable openflow");
                    }
                    if(p_openflow_intf->bind_tunnel_type != p_db_if->hw_type)
                    {
                        CFG_INVALID_PARAM_RET("This bind port had been set other tunnel type (default is vxlan tunnel), one port only can be set one kind of tunnel");
                    }            

                    /* modified by liwh for bug 46644, 2019-03-11 */
                    sal_sscanf(argv[4], "%hx.%hx.%hx",
                        (unsigned short *)&dest_mac[0],
                        (unsigned short *)&dest_mac[2],
                        (unsigned short *)&dest_mac[4]);
                    
                    *((unsigned short *)&dest_mac[0]) = sal_ntoh16(*((unsigned short *)&dest_mac[0]));
                    *((unsigned short *)&dest_mac[2]) = sal_ntoh16(*((unsigned short *)&dest_mac[2]));
                    *((unsigned short *)&dest_mac[4]) = sal_ntoh16(*((unsigned short *)&dest_mac[4]));

                    if (GLB_IS_MCAST_MAC(dest_mac) || GLB_IS_BCAST_MAC(dest_mac) || GLB_IS_ZERO_MAC(dest_mac))
                    {
                        CFG_INVALID_PARAM_RET("Invalid nexthop mac address");
                    }
                    /* liwh end */
                    
                    sal_strncpy(p_openflow_tunnel_intf->bind_port_name, argv[2], IFNAME_SIZE);
                    GLB_SET_FLAG(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_BIND_PORT);
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_PORT_NAME);
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_FLAG);

                    /*bind mac*/
                    sal_strncpy(p_openflow_tunnel_intf->bind_mac, argv[4], MACADDR_BUF_SIZE);
                    GLB_SET_FLAG(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_BIND_MAC);
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_MAC);
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_FLAG);

                    /*bind vlan*/
                    GLB_UNSET_FLAG(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_BIND_VLAN);
                    p_openflow_tunnel_intf->bind_vlan  = 0;
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_VLAN);
                    tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_FLAG);
                    if(argc ==6)
                    {
                        PM_CFG_GET_UINT(value, argv[5], para->p_rs);
                        if(value)
                        {
                            p_openflow_tunnel_intf->bind_vlan = value;
                            GLB_SET_FLAG(p_openflow_tunnel_intf->flag, GLB_OPENFLOW_TUNNEL_IFF_BIND_VLAN);
                            tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_VLAN);
                            tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_openflow_tunnel_intf, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_FLAG);
                        }
                    }

                    /* modified by liwh for bug 48828, 2018-02-25 */
                    if (p_openflow_intf)
                    {
                        p_openflow_intf->is_bind_tunnel = TRUE;
                        tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_IS_BIND_TUNNEL);
                    }
                    /* liwh end */
                    break;
                default:
                    CFG_INVALID_PARAM_RET("Invalid action");
                    break;
            }
            break;
        case CDB_OPER_GET:

            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            rc = _if_mgr_cmd_get_opf_tunnel_intf(argv[0], para, p_node, &field);
            break;
        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }
    return rc;
}

int32
if_mgr_cmd_process_openflow_interface(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_openflow_interface_t openflow_intf;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    tbl_openflow_interface_t *p_openflow_intf = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;
    uint32 value = 0;
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_t intf;
    uint32 protected_vlan = 0;
    tbl_sys_global_t* p_sys_global = NULL;
    uint32 index = 0;
    int32 packet_type = 0;
    int32 packet_act = 0;
    field_parse_t   field;
    char cmd[MAX_CMD_STR_LEN] = {0};
    int ret = 0;
    glb_if_type_t tunnel_type = GLB_IF_TYPE_NULL;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_OPENFLOW_INTERFACE);
    switch (para->oper)
    {
        case CDB_OPER_ADD:
            break;
        case CDB_OPER_DEL:
            break;
        case CDB_OPER_SET:
            sal_memset(&openflow_intf, 0, sizeof(openflow_intf));
            sal_memset(&intf, 0, sizeof(intf));
            sal_strncpy(openflow_intf.key.name, argv[0], IFNAME_SIZE);
            sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);
            p_db_if = tbl_interface_get_interface(&intf.key);
            if(NULL == p_db_if)
            {
                return PM_E_NOT_EXIST;
            }
            field_id = cdb_get_field_id(argv[1], p_node, &p_field);
            if((p_db_if->hw_type == GLB_IF_TYPE_L2GRE_IF)||
                (p_db_if->hw_type == GLB_IF_TYPE_L3GRE_IF) ||
                (p_db_if->hw_type == GLB_IF_TYPE_NVGRE_IF)||
                (p_db_if->hw_type == GLB_IF_TYPE_VXLAN_IF))
            {
                if(TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_ENABLE == field_id)
                {
                    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                    ret = if_mgr_openflow_tunnel_interface_openflow_en(argv[0], value, para);
                }
                else
                {
                    CFG_CONFLICT_RET("This operation is not supported at an tunnel port");
                }
                return ret;
            }
            /* check exist */
            p_openflow_intf = tbl_openflow_interface_get_openflow_interface(&openflow_intf.key);
            if (!p_openflow_intf)
            {
                CFG_CONFLICT_RET("Vlan interface or loopback interface can not be configured openflow enable");
            }
            switch(field_id)
            {
                case TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_ENABLE:
                    if ((p_db_if->hw_type != GLB_IF_TYPE_PORT_IF) && (p_db_if->hw_type !=GLB_IF_TYPE_LINKAGG_IF))
                    {
                        CFG_CONFLICT_RET("Now only phyport and agg port support enable openflow");
                    }
                        
                    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                    if(!value)
                    {
                    #if 0
                        if(p_openflow_intf->is_add_to_br0)
                        {
                            CFG_INVALID_PARAM_RET("Ovs-db should delete this port or remove it from bond first");
                        }
                    #endif
                        for(index = 0; index < OPENFLOW_PROTECTED_VLAN; index ++)
                        {
                            if(p_openflow_intf->protected_vlan[index])
                            {
                                CFG_INVALID_PARAM_RET("Please delete protected vlan first");
                            }
                        }
                        if(p_openflow_intf->ingress_add_native_vlan_enable || p_openflow_intf->egress_remove_native_vlan_enable)
                        {
                            CFG_INVALID_PARAM_RET("Please delete native vlan config first");
                        }
                        if(0 == p_openflow_intf->obey_vlan_filter)
                        {
                            CFG_INVALID_PARAM_RET("Please delete vlan-filter disable config first");
                        }
                        if(p_openflow_intf->bind_tunnel_type != GLB_IF_TYPE_VXLAN_IF)
                        {
                            CFG_INVALID_PARAM_RET("Please set default tunnel type config first");
                        }

                        /* modified by liwh for bug 51315, 2019-03-19 */
                        if (p_openflow_intf->tunnel_mpls_disable)
                        {
                            CFG_INVALID_PARAM_RET("Please delete no openflow tunnel-mpls enable config first");
                        }
                        /* liwh end */
                    }
                    else
                    {
                        if(INTERFACE_IS_LAG_MEMBER(p_db_if))
                        {
                            CFG_INVALID_PARAM_RET("This port is a lag member, can not enable openflow");
                        }
                        /*modified by hansf for bug 43750 remove only agg1-16 openflow enabel restriction*/
                        if(INTERFACE_IS_LAG_GROUP(p_db_if))
                        {
                            if((p_db_if->ifindex - GLB_AGG_IFINDEX_MIN) > GLB_LAG_GROUP_NUM) /*OFP_AGG_PORT_NO_MAX*/ 
                            {
                                CFG_INVALID_PARAM_RET("Only agg1~%d support enable openflow", GLB_LAG_GROUP_NUM);
                            }
                        }
                    }

                    /* added by hansf for netconf */
                    if (CDB_CFG_NC_CHECK == para->cfg)
                    {
                        return PM_E_NONE;
                    }
                    /*ended by hansf*/
                    
                    if(value == p_openflow_intf->openflow_enable)
                    {
                        return rc;
                    }
                    IFNAME_ETH2FULL(p_openflow_intf->key.name, ifname_ext);
                    if(INTERFACE_IS_LAG_GROUP(p_db_if))
                    {
                        if(value)
                        {
                            sal_sprintf(cmd, "ovs-vsctl add-agg-centec br0  %s -- set interface  %s type=agg", ifname_ext, ifname_ext);
                            ret = system(cmd);
                        }
                        else
                        {
                            sal_sprintf(cmd, "ovs-vsctl del-port-centec br0  %s", ifname_ext);
                            ret = system(cmd);
                            if(ret != 0)
                            {
                                CFG_INVALID_PARAM_RET("Ovs-db may bind tunnel on this port , please remove them first");
                            }
                        }
                    }
                    else
                    {
                        if(value)
                        {
                            sal_sprintf(cmd, "ovs-vsctl add-port-centec br0  %s -- set interface  %s type=switch", ifname_ext, ifname_ext);
                            ret = system(cmd);
                        }
                        else
                        {
                            sal_sprintf(cmd, "ovs-vsctl del-port-centec br0  %s", ifname_ext);
                            ret = system(cmd);
                            if(ret != 0)
                            {
                                CFG_INVALID_PARAM_RET("Ovs-db may bind tunnel on this port , please remove them first");
                            }
                        }
                    }
                    
                    p_db_if->openflow_en = value;
                    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_OPENFLOW_EN);
                    p_openflow_intf->openflow_enable = value;
                    tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_ENABLE);
                    break;
                case TBL_OPENFLOW_INTERFACE_FLD_OBEY_VLAN_FILTER:
                    if(!p_openflow_intf->openflow_enable)
                    {
                        CFG_INVALID_PARAM_RET("Openflow is not enable at the port");
                    }
                    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                    if(value == p_openflow_intf->obey_vlan_filter)
                    {
                        return rc;
                    }
                    p_openflow_intf->obey_vlan_filter = value;
                    tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_OBEY_VLAN_FILTER);
                    break;
                case TBL_OPENFLOW_INTERFACE_FLD_BIND_TUNNEL_TYPE:
                    /* modified by liwh for bug 51306, 2019-03-19 */
                    if(!check_feature_support(CONFIG_FEATURE_HYBRID_TUNNEL))
                    {
                        CFG_INVALID_PARAM_RET("Not supported on this product");
                    }
                    /* liwh end */
                     
                    if(!p_openflow_intf->openflow_enable)
                    {
                        CFG_INVALID_PARAM_RET("Openflow is not enable at the port");
                    }

                    /* modified by liwh for bug 48828, 2018-02-25 */
                    if (p_openflow_intf->is_bind_tunnel)
                    {
                        CFG_INVALID_PARAM_RET("Can't change tunnel type because the port is bound by tunnel");
                    }
                    /* liwh end */
                    
                    if(0 == sal_strcmp(argv[2], "vxlan"))
                    {
                        tunnel_type =  GLB_IF_TYPE_VXLAN_IF;
                    }
                    else if(0 == sal_strcmp(argv[2], "nvgre"))
                    {
                        tunnel_type = GLB_IF_TYPE_NVGRE_IF;
                    }
                    else if(0 == sal_strcmp(argv[2], "l2gre"))
                    {
                        tunnel_type = GLB_IF_TYPE_L2GRE_IF;
                    }
                    else if(0 == sal_strcmp(argv[2], "default"))
                    {
                        tunnel_type =  GLB_IF_TYPE_VXLAN_IF;
                    }

                    if(tunnel_type == p_openflow_intf->bind_tunnel_type)
                    {
                        return rc;
                    }
                    p_openflow_intf->bind_tunnel_type = tunnel_type;
                    tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_BIND_TUNNEL_TYPE);
                    break;
                case TBL_OPENFLOW_INTERFACE_FLD_TUNNEL_MPLS_DISABLE:
                    if(!p_openflow_intf->openflow_enable)
                    {
                        CFG_INVALID_PARAM_RET("Openflow is not enable at the port");
                    }
                    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                    if(value == p_openflow_intf->tunnel_mpls_disable)
                    {
                        return rc;
                    }
                    p_openflow_intf->tunnel_mpls_disable = value;
                    tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_TUNNEL_MPLS_DISABLE);
                    break;
                case TBL_OPENFLOW_INTERFACE_FLD_INGRESS_ADD_NATIVE_VLAN_ENABLE:
                    if(!p_openflow_intf->openflow_enable)
                    {
                        CFG_INVALID_PARAM_RET("Openflow is not enable at the port");
                    }
                    if(GLB_IF_MODE_L3 == p_db_if->mode)
                    {
                        CFG_INVALID_PARAM_RET("This function can not enable on routed port");
                    }
                    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                    if(value == p_openflow_intf->ingress_add_native_vlan_enable)
                    {
                        return rc;
                    }
                    p_openflow_intf->ingress_add_native_vlan_enable = value;
                    rc = tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_INGRESS_ADD_NATIVE_VLAN_ENABLE);
                    break;
                case TBL_OPENFLOW_INTERFACE_FLD_EGRESS_REMOVE_NATIVE_VLAN_ENABLE:
                    if(!p_openflow_intf->openflow_enable)
                    {
                        CFG_INVALID_PARAM_RET("Openflow is not enable at the port");
                    }
                    PM_CFG_GET_UINT(value, argv[2], para->p_rs);
                    if(value == p_openflow_intf->egress_remove_native_vlan_enable)
                    {
                        return rc;
                    }
                    p_openflow_intf->egress_remove_native_vlan_enable = value;
                    rc = tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_EGRESS_REMOVE_NATIVE_VLAN_ENABLE);
                    break;
                case TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN:
                    PM_CFG_GET_UINT(protected_vlan, argv[2], para->p_rs);
                    p_sys_global = tbl_sys_global_get_sys_global();
                    if(!p_openflow_intf->openflow_enable)
                    {
                        CFG_INVALID_PARAM_RET("Openflow is not enable at the port");
                    }
                    if(0 == sal_strcmp(argv[3], "set"))
                    {
                        if(p_sys_global->protected_vlan_cnt >= OPENFLOW_PROTECTED_VLAN)
                        {
                            CFG_INVALID_PARAM_RET("The number of protected vlan should not be more than 50!");
                        }
                        for(index = 0; index < OPENFLOW_PROTECTED_VLAN; index ++)
                        {
                            if(protected_vlan == p_openflow_intf->protected_vlan[index])
                            {
                                CFG_INVALID_PARAM_RET("This vlan had been set for protected vlan");
                            }
                        }
                        for(index = 0; index < OPENFLOW_PROTECTED_VLAN; index ++)
                        {
                            if(0 == p_openflow_intf->protected_vlan[index])
                            {
                                p_openflow_intf->protected_vlan[index] = protected_vlan;

                                p_sys_global->protected_vlan_cnt ++;
                                p_openflow_intf->protected_vlan_change = protected_vlan;
                                tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE);
                                p_openflow_intf->protected_vlan_change_is_set = 1;
                                tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE_IS_SET);
                                tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN);
                                tbl_sys_global_set_sys_global_field(p_sys_global, TBL_SYS_GLOBAL_FLD_PROTECTED_VLAN_CNT);
                                break;
                            }
                        }
                    }
                    else
                    {
                        if(protected_vlan)
                        {
                            for(index = 0; index < OPENFLOW_PROTECTED_VLAN; index ++)
                            {
                                if(protected_vlan == p_openflow_intf->protected_vlan[index])
                                {
                                    p_openflow_intf->protected_vlan[index] = 0;
                                    p_sys_global->protected_vlan_cnt --;
                                    p_openflow_intf->protected_vlan_change = protected_vlan;
                                    tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE);
                                    p_openflow_intf->protected_vlan_change_is_set = 0;
                                    tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE_IS_SET);
                                    tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN);
                                    tbl_sys_global_set_sys_global_field(p_sys_global, TBL_SYS_GLOBAL_FLD_PROTECTED_VLAN_CNT);
                                    break;
                                }
                            }
                            if(index == OPENFLOW_PROTECTED_VLAN)
                            {
                                CFG_INVALID_PARAM_RET("This protected vlan is not exist!");
                            }
                        }
                        else
                        {
                            for(index = 0; index < OPENFLOW_PROTECTED_VLAN; index ++)
                            {
                                if(0 != p_openflow_intf->protected_vlan[index])
                                {
                                    p_openflow_intf->protected_vlan_change = p_openflow_intf->protected_vlan[index];
                                    tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE);
                                    p_openflow_intf->protected_vlan_change_is_set = 0;
                                    tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE_IS_SET);
                                    p_openflow_intf->protected_vlan[index] = 0;
                                    tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN);
                                    p_sys_global->protected_vlan_cnt --;
                                }
                            }
                            tbl_sys_global_set_sys_global_field(p_sys_global, TBL_SYS_GLOBAL_FLD_PROTECTED_VLAN_CNT);
                        }
                    }
                    break;

                case TBL_OPENFLOW_INTERFACE_FLD_ACTION:
                    packet_type = if_mgr_packet_type(argv[2]);
                    packet_act = if_mgr_packet_action(argv[3]);
                    p_openflow_intf->action[packet_type] = packet_act;
                    tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_ACTION);
                    break;
                    
                default:
                    CFG_INVALID_PARAM_RET("Invalid action");
                    break;
            }
            break;
        case CDB_OPER_GET:

            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            rc = _if_mgr_cmd_get_opf_intf(argv[0], para, p_node, &field);
            break;
        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }
    return rc;
}
static int32
_if_mgr_cmd_set_latency_event_threshold(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{    
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;
    uint32 chip_type = 0;

    /* 1. config check */
    chip_type = tbl_chip_get_type();
    if (GLB_CHIP_GOLDENGATE != chip_type)
    {
        CFG_UNSUPPORT_PRODUCT_RET();
    }
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }

    if (p_if->latency_event_low >=  p_if->latency_event_high)
    {
        CFG_INVALID_PARAM_RET("The high threshold must be greater than low threshold");
    }

    /* 2. check duplicate config */  
    if ((p_if->latency_event_low == p_db_if->latency_event_low) 
        && (p_if->latency_event_high == p_db_if->latency_event_high)) 
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */    
    PM_E_RETURN(if_mgr_set_latency_event_threshold(p_if, p_if->latency_event_low, p_if->latency_event_high));

    return PM_E_NONE;    
}

static int32
_if_mgr_cmd_set_latency_event_enable(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;
    uint32 chip_type = 0;
    
    /* 1. config check */
    chip_type = tbl_chip_get_type();
    if (GLB_CHIP_GOLDENGATE != chip_type)
    {
        CFG_UNSUPPORT_PRODUCT_RET();
    }
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_if->key.name, ifname_ext));
    }

    /* 2. check duplicate config */     
    if (p_db_if->latency_event_enable == p_if->latency_event_enable) 
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */    
    PM_E_RETURN(if_mgr_set_latency_event_enable(p_if, p_if->latency_event_enable));

    return PM_E_NONE;
}


int32
if_mgr_cmd_set_l2protcol_vlan(ds_l2protocol_t *p_dsl2proto, ds_l2protocol_t *p_db_dsl2proto, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */     
    if ((p_dsl2proto->l2pro_vlan == p_db_dsl2proto->l2pro_vlan) &&
        (0 == sal_strlen(p_db_dsl2proto->l2pro_evc_name))) 
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */    
    PM_E_RETURN(if_mgr_set_l2protocol_vlan(p_dsl2proto, p_db_if));

    return rc;
}

int32
if_mgr_cmd_set_l2protcol_action(ds_l2protocol_t *p_dsl2proto, ds_l2protocol_t *p_db_dsl2proto, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */
    if (p_db_if->l2pro_uplink && p_dsl2proto->l2pro_action == GLB_L2_PROTO_TUNNEL)
    {
        CFG_CONFLICT_RET("Can't configure l2protocol tunnel when l2protocol uplink has been enabled");    
    }
    /* 2. check duplicate config */     
    if (p_dsl2proto->l2pro_action == p_db_dsl2proto->l2pro_action) 
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */    
    PM_E_RETURN(if_mgr_set_l2protocol_action(p_dsl2proto, p_db_if));

    return rc;
}

int32
if_mgr_cmd_set_l2protcol_evc(ds_l2protocol_t *p_dsl2proto, ds_l2protocol_t *p_db_dsl2proto, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */ 
    if (0 == sal_strcmp(p_dsl2proto->l2pro_evc_name, p_db_dsl2proto->l2pro_evc_name))
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */    
    PM_E_RETURN(if_mgr_set_l2protocol_evc(p_dsl2proto, p_db_if));

    return rc;
}

static int32
_if_mgr_cmd_process_interface_set_l2protocol(char **argv, int32 argc, cfg_cmd_para_t *para, tbl_interface_t *p_db_if)
{
    ds_l2protocol_t dsl2proto;
    ds_l2protocol_t *p_db_dsl2proto = NULL;
    int32 rc = PM_E_NONE;
    uint32 action_type = 0;
    vid_t vlan_id = 0;
    uint32 proto_index = 0;

    tbl_evc_entry_t* p_db_evc_entry = NULL;
    tbl_evc_entry_key_t evc_entry_key;
    tbl_evc_entry_t* p_db_evc_entry1 = NULL;
    tbl_evc_entry_key_t evc_entry_key1;
    tbl_l2_protocol_key_t l2protocol_key;
    tbl_l2_protocol_t* p_db_l2protocol = NULL;

    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    if (GLB_IF_MODE_L2 != p_db_if->mode) 
    {
        CFG_CONFLICT_RET("Can only configure on layer two interface");
    }
    if ((GLB_IF_TYPE_PORT_IF != p_db_if->hw_type) &&
        (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type))
    {
        CFG_CONFLICT_RET("Can only configure on physical port or agg port");
    }

    sal_memset(&dsl2proto, 0, sizeof(dsl2proto));
    sal_memset(&l2protocol_key, 0, sizeof(l2protocol_key));
    sal_memset(&evc_entry_key, 0, sizeof(evc_entry_key));
    sal_memset(&evc_entry_key1, 0, sizeof(evc_entry_key1));

    /*If is the user defined protocol, check the protocol name is exist*/
    PM_CFG_CHECK_MIN_PARAMS(argc, 5, para->p_rs);
    if (0 == sal_strcmp(argv[2], "user_defined"))
    {
        sal_strncpy(l2protocol_key.name, argv[4], GLB_L2_PROTOCOL_NAME_SIZE+1);
        p_db_l2protocol = tbl_l2_protocol_get_l2_protocol(&l2protocol_key);
        if (NULL == p_db_l2protocol)
        {
            CFG_CONFLICT_RET("L2 protocol %s does not exist", argv[4]);
        }
    }

    /*Get the l2 protocol name table*/
    sal_strncpy(dsl2proto.key.l2pro_name, argv[4], GLB_L2_PROTOCOL_NAME_SIZE+1);
    p_db_dsl2proto = ds_l2protocol_get_l2protocol(p_db_if, &dsl2proto);
    if (0 == sal_strcmp(argv[4], "dot1x"))
    {
        proto_index = 0;    
    }
    else if (0 == sal_strcmp(argv[4], "lldp"))
    {
        proto_index = 1;
    }
    else if (0 == sal_strcmp(argv[4], "slow-proto"))
    {
        proto_index = 2;
    }  
    else if (0 == sal_strcmp(argv[4], "stp"))
    {
        proto_index = 3;
    }
    
    if (0 == sal_strcmp(argv[3], "add"))/*Creat ds_l2protocol table*/
    {
        if (NULL == p_db_dsl2proto)
        {
            if (0 == sal_strcmp(argv[2], "user_defined"))
            {
                dsl2proto.is_standard_pro = FALSE;
                proto_index = p_db_l2protocol->protocol_index;
            }
            else
            {
                dsl2proto.is_standard_pro = TRUE;
            }
            dsl2proto.l2pro_action = GLB_L2_PROTO_PEER;
            PM_E_RETURN(ds_l2protocol_add_l2protocol(p_db_if, &dsl2proto));
            
            p_db_if->l2pro_if_action_vlan.l2action[proto_index] = GLB_L2_PROTO_PEER; 
            p_db_if->l2pro_if_action_vlan.l2vlan[proto_index] = GLB_VLAN_NONE;
            PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_L2PRO_IF_ACTION_VLAN));
        }
        else
        {
            return PM_E_NONE;
        }
    }
    else if (0 == sal_strcmp(argv[3], "update")) /*update ds_l2protocol table*/
    {
        if (NULL == p_db_dsl2proto)
        {
            CFG_CONFLICT_RET("L2 protocol %s is not enable", argv[4]);
        }
        if (0 == sal_strcmp(argv[5], "protocol-vlan"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 8, para->p_rs);
            if (0 == sal_strcmp(argv[6], "vlan")) /*Set protocol vlan ID*/
            {
                PM_CFG_GET_UINT(vlan_id, argv[7], para->p_rs);
                dsl2proto.l2pro_vlan = vlan_id;
                if (sal_strlen(p_db_dsl2proto->l2pro_evc_name))
                {
                    sal_strncpy(evc_entry_key.name, p_db_dsl2proto->l2pro_evc_name, GLB_VLAN_EVC_NAME_LEN+1);
                    p_db_evc_entry = tbl_evc_entry_get_evc_entry(&evc_entry_key);
                    
                }
                PM_E_RETURN(if_mgr_cmd_set_l2protcol_vlan(&dsl2proto, p_db_dsl2proto, p_db_if, para));

                if (p_db_evc_entry && p_db_evc_entry->ref_count)
                {
                    p_db_evc_entry->ref_count--;
                    PM_E_RETURN(tbl_evc_entry_set_evc_entry_field(p_db_evc_entry, TBL_EVC_ENTRY_FLD_REF_COUNT));
                }
                
                if (!p_db_dsl2proto->is_standard_pro)
                {
                    proto_index = p_db_l2protocol->protocol_index;    
                }
                p_db_if->l2pro_if_action_vlan.l2vlan[proto_index] = vlan_id; 
                PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_L2PRO_IF_ACTION_VLAN));
            }
            else if (0 == sal_strcmp(argv[6], "evc"))/*Set protocol EVC name*/
            {
                sal_strncpy(evc_entry_key.name, argv[7], GLB_VLAN_EVC_NAME_LEN+1);
                p_db_evc_entry = tbl_evc_entry_get_evc_entry(&evc_entry_key);
                if (NULL == p_db_evc_entry)
                {
                    CFG_CONFLICT_RET("Evc %s does not exist", argv[7]);
                }
                if (ETHERNET_EVC_TYPE_DOUBLE_VLAN == p_db_evc_entry->evc_type)
                {
                    CFG_CONFLICT_RET("Can't configure the evc with double vlan tag");
                }
                else if (ETHERNET_EVC_TYPE_NONE == p_db_evc_entry->evc_type)
                {
                    CFG_CONFLICT_RET("Can't configure the evc with no vlan tag");
                }
                /*Modified by yejl for bug 48266, 2018-07-30*/
                if (sal_strlen(p_db_dsl2proto->l2pro_evc_name))
                {
                    sal_strncpy(evc_entry_key1.name, p_db_dsl2proto->l2pro_evc_name, GLB_VLAN_EVC_NAME_LEN+1);
                    p_db_evc_entry1 = tbl_evc_entry_get_evc_entry(&evc_entry_key1);
                    if (p_db_evc_entry1 && p_db_evc_entry1->ref_count)
                    {
                        p_db_evc_entry1->ref_count--;
                        PM_E_RETURN(tbl_evc_entry_set_evc_entry_field(p_db_evc_entry1, TBL_EVC_ENTRY_FLD_REF_COUNT));
                    }
                }
                /*ended*/
            
                sal_strncpy(dsl2proto.l2pro_evc_name, argv[7], GLB_VLAN_EVC_NAME_LEN + 1);
                vlan_id = p_db_evc_entry->mapped_svid;
                PM_E_RETURN(if_mgr_cmd_set_l2protcol_evc(&dsl2proto, p_db_dsl2proto, p_db_if, para));
                
                p_db_evc_entry->ref_count++;
                PM_E_RETURN(tbl_evc_entry_set_evc_entry_field(p_db_evc_entry, TBL_EVC_ENTRY_FLD_REF_COUNT));
                
                if (!p_db_dsl2proto->is_standard_pro)
                {
                    proto_index = p_db_l2protocol->protocol_index;    
                }
                p_db_if->l2pro_if_action_vlan.l2vlan[proto_index] = vlan_id; 
                PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_L2PRO_IF_ACTION_VLAN));
            }
        }
        else if (0 == sal_strcmp(argv[5], "protocol-action"))/*Set protocol action*/
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 7, para->p_rs);
            if (0 == sal_strcmp(argv[6], "discard"))
            {
                action_type = GLB_L2_PROTO_DISCARD;    
            }
            else if (0 == sal_strcmp(argv[6], "tunnel"))
            {
                action_type = GLB_L2_PROTO_TUNNEL;
            }
            else if (0 == sal_strcmp(argv[6], "peer"))
            {
                action_type = GLB_L2_PROTO_PEER;
            }   
            dsl2proto.l2pro_action = action_type;
            PM_E_RETURN(if_mgr_cmd_set_l2protcol_action(&dsl2proto, p_db_dsl2proto, p_db_if, para));
            if (!p_db_dsl2proto->is_standard_pro)
            {
                proto_index = p_db_l2protocol->protocol_index;
            }
            p_db_if->l2pro_if_action_vlan.l2action[proto_index] = action_type; 
            PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_L2PRO_IF_ACTION_VLAN));
        }
    }
    else if (0 == sal_strcmp(argv[3], "del"))/*Delete ds_l2protocol table*/
    {
        if (NULL == p_db_dsl2proto)
        {
            return PM_E_NONE;
        }
        else
        {
            if (!p_db_dsl2proto->is_standard_pro)
            {
                proto_index = p_db_l2protocol->protocol_index;
            }
            /*Modified by yejl for bug 48218, 2018-07-26*/
            if (sal_strlen(p_db_dsl2proto->l2pro_evc_name))
            {
                sal_strncpy(evc_entry_key.name, p_db_dsl2proto->l2pro_evc_name, GLB_VLAN_EVC_NAME_LEN+1);
                p_db_evc_entry = tbl_evc_entry_get_evc_entry(&evc_entry_key);
                if (p_db_evc_entry && p_db_evc_entry->ref_count)
                {
                    p_db_evc_entry->ref_count--;
                    PM_E_RETURN(tbl_evc_entry_set_evc_entry_field(p_db_evc_entry, TBL_EVC_ENTRY_FLD_REF_COUNT));
                }
            }
            /*ended*/
            PM_E_RETURN(ds_l2protocol_del_l2protocol(p_db_if, &dsl2proto));

            p_db_if->l2pro_if_action_vlan.l2action[proto_index] = 0; 
            p_db_if->l2pro_if_action_vlan.l2vlan[proto_index] = 0; 
            PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_L2PRO_IF_ACTION_VLAN));
        }
    }
    return rc;
}

int32
_if_mgr_cmd_set_l2protocol_uplink(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    tbl_lldp_if_t* p_db_lldp_if = NULL;
    tbl_lldp_if_key_t lldp_if_key;
    int32 rc = PM_E_NONE;
    uint32 i = 0;

    /* 1. config check */
    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    if (GLB_IF_MODE_L2 != p_db_if->mode) 
    {
        CFG_CONFLICT_RET("Can only configure on layer two interface");
    }
    if ((GLB_IF_TYPE_PORT_IF != p_db_if->hw_type) &&
        (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type))
    {
        CFG_CONFLICT_RET("Can only configure on physical port or agg port");
    }
    if (p_if->l2pro_uplink)
    {
        if (p_db_if->vlanclass_active_type)
        {
            CFG_CONFLICT_RET("Vlan classification is enabled on this interface");
        }
        if (p_db_if->brgif->vlan_translation_en)
        {
            CFG_CONFLICT_RET("Vlan translation is enabled on this interface");
        }
        if (p_db_if->vlan_type != GLB_VLAN_PORT_TYPE_TRUNK)
        {
            CFG_CONFLICT_RET("L2protocol uplink enable can be only configured on trunk port");
        }
        if (p_db_if->ip_source_type)
        {       
            CFG_CONFLICT_RET("IP source guard is already enabled on this interface");
        }
        if (GLB_IF_PVLAN_TYPE_NONE != p_db_if->pvlan_type)
        {
            CFG_CONFLICT_RET("Interface is configured as a private VLAN");
        }
        if (p_db_if->dot1x_enable)
        {
            CFG_CONFLICT_RET("Dot1x is enabled on this interface!");
        }

        for (i = GLB_L2_PROTO_INDEX_0; i<= GLB_L2_PROTO_INDEX_7; i++)
        {
            if (p_db_if->l2pro_if_action_vlan.l2action[i] == GLB_L2_PROTO_TUNNEL)
            {
                CFG_CONFLICT_RET("L2protocol uplink can't be enabled when l2protocol tunnel has been enabled");
            }
        }

        /*added by yejl for bug 48292, 2018-08-01*/
        if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
        {
            sal_memset(&lldp_if_key, 0, sizeof(lldp_if_key));
            lldp_if_key.ifindex = p_db_if->ifindex;
            p_db_lldp_if = tbl_lldp_if_get_lldp_if(&lldp_if_key);
            if (NULL == p_db_lldp_if)
            {
                CFG_CONFLICT_RET("L2protocol uplink can't be enabled because lldp is disable on this interface");
            }
        }
    }
    
    /* 2. check duplicate config */     
    if (p_if->l2pro_uplink == p_db_if->l2pro_uplink) 
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */    
    PM_E_RETURN(if_mgr_set_l2protocol_uplink(p_if));

    return rc;
}

static int32
_if_mgr_cmd_set_flow_statistics(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];

    /* 1. config check */
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Can not set flow-statistics on interface %s", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_if->ifg_stats_en== p_if->ifg_stats_en)
    {
        return PM_E_NONE;
    }

    PM_E_RETURN(if_mgr_set_flow_statistics(p_db_if, p_if->ifg_stats_en));

    return rc;
}

/*Added by zhush for bug 52768, 2019-07-06*/
static int32
_if_mgr_cmd_process_interface_show_summary_one_json(tbl_interface_t* p_db_if, tbl_port_stats_rate_t *p_stats_rate, FILE* fp, cfg_cmd_para_t *para)
{
    uint32 is_running = FALSE;
    const char *hw_type = NULL;
    char rxbs[32] = {0};
    char rxps[32] = {0};
    char txbs[32] = {0};
    char txps[32] = {0};
    uint64 u_rxbs = 0;
    uint64 u_rxps = 0;
    uint64 u_txbs = 0;
    uint64 u_txps = 0;
    uint8 ifg_stats_en = 0;
    char ifname_ext[IFNAME_EXT_SIZE];
    cJSON *root = NULL;
    cJSON *if_cj = NULL;
    char *out = NULL;
    
    hw_type = cdb_enum_val2str(glb_if_type_strs, GLB_IF_TYPE_MAX, p_db_if->hw_type);
    if ((0 != sal_strcmp(hw_type, "Port")) && (0 != sal_strcmp(hw_type, "LAG")))
    {
        /* only show Port and LAG*/
        return PM_E_NONE;
    }

    if (tbl_interface_is_running(p_db_if))
    {
        is_running = TRUE;
    }

    if (p_db_if->ifg_stats_en)
    {
        ifg_stats_en = TRUE;
    }
    
    u_rxbs = p_stats_rate->octets_rcv_rate;
    u_rxps = p_stats_rate->pkts_rcv_rate;
    u_txbs = p_stats_rate->octets_send_rate;
    u_txps =p_stats_rate->pkts_send_rate;

    if (ifg_stats_en)
    {
        u_rxbs = u_rxbs + u_rxps*GLB_DEF_IPG_LEN*8;
        u_txbs = u_txbs + u_txps*GLB_DEF_IPG_LEN*8;
    }

    sal_sprintf(rxbs, "%"PRIu64, u_rxbs); 
    sal_sprintf(rxps, "%"PRIu64, u_rxps); 
    sal_sprintf(txbs, "%"PRIu64, u_txbs); 
    sal_sprintf(txps, "%"PRIu64, u_txps); 
    

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    root = cJSON_CreateObject();
    if_cj = cJSON_CreateObject();
    cJSON_AddItemToObject(root, ifname_ext, if_cj);	
    cJSON_AddStringToObject(if_cj, "Link", is_running ? "UP" : "DOWN");
    cJSON_AddStringToObject(if_cj, "RXBS", rxbs);
    cJSON_AddStringToObject(if_cj, "RXPS", rxps);
    cJSON_AddStringToObject(if_cj, "TXBS", txbs);
    cJSON_AddStringToObject(if_cj, "TXPS", txps);

     out = cJSON_Print(root);
    sal_fprintf(fp, out);
    sal_fprintf(fp,"\n");

    cJSON_Delete(root);
    cJSON_free(out);

    return PM_E_NONE;
}

/*Added by zhush for bug 52768, 2019-07-06*/
static int32
 _if_mgr_cmd_process_interface_show_summary_json(char **argv, tbl_interface_t* p_db_if, cfg_cmd_para_t *para, uint32 if_all)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    FILE *fp = NULL;
    tbl_port_stats_rate_t *p_db_stats_rate = NULL;
    tbl_port_stats_rate_t port_stats_rate;
    uint32 ifindex = 0;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if(!fp)
    {
    	return PM_E_NONE;
    }
    if (!if_all)
    {
        ifindex = tbl_interface_get_ifindex_by_name(argv[0]);
        port_stats_rate.key.ifindex = ifindex;
        p_db_stats_rate = tbl_port_stats_rate_get_port_stats_rate(&port_stats_rate.key);
        if (NULL == p_db_stats_rate)
        {
             CFG_FUNC_ERR_RET("Port stats rate ifindex %d not exist\n", ifindex);
        }            
        _if_mgr_cmd_process_interface_show_summary_one_json(p_db_if, p_db_stats_rate, fp, para);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
        {
            port_stats_rate.key.ifindex = p_db_if->ifindex;
            p_db_stats_rate = tbl_port_stats_rate_get_port_stats_rate(&port_stats_rate.key);
            _if_mgr_cmd_process_interface_show_summary_one_json(p_db_if, p_db_stats_rate, fp, para);
        }
    }
    
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}
/*end zhus*/

int32
_if_mgr_cmd_set_hash_value_profile_select(tbl_interface_t *p_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 agg_hash = 0;
    tbl_lag_global_t* p_lag_global = NULL;
    p_lag_global = tbl_lag_global_get_lag_global();
    tbl_hash_value_global_t * p_hash_value_glb = NULL;
    p_hash_value_glb = tbl_hash_value_global_get_hash_value_global();
    tbl_hash_value_profile_t * p_hash_value_pro = NULL;
    ds_lag_t* p_lag = NULL;
    uint32 old_profileid = p_db_if->hash_value_profile_id;
    uint32 new_profileid = p_if->hash_value_profile_id;
    
    /* 1. config check */
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Load balance can be only configured on physical port and link aggregation");
    }
    if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type && p_if->hash_value_profile_dir == GLB_EGRESS)
    {
        CFG_CONFLICT_RET("Output can only be configured on link aggregation");
    }
    
    if (INTERFACE_IS_LAG_GROUP(p_db_if))
    {
        p_lag = p_db_if->lag;
        if (p_lag == NULL)
        {
            CFG_CONFLICT_RET("Ds Lag is NULL");
        }
        if (p_if->hash_value_profile_dir == GLB_EGRESS)
        {
            old_profileid = p_db_if->hash_value_profile_id_agg;
            if (p_lag_global->load_balance_mode[p_lag->lag_id] == GLB_ROUND_ROBIN_LOAD_BALANCE)
            {
                CFG_CONFLICT_RET("Can't configure output on round robin load balance mode link aggregation");
            }
            if (new_profileid == 0)
            {
                agg_hash = p_hash_value_glb->agg_hash;
            }
            else
            {
                p_hash_value_pro = hash_value_profile_lookup_by_id(new_profileid);
                if (NULL == p_hash_value_pro)
                {
                    CFG_CONFLICT_RET("Can not get the hash value profile by profile id %u", new_profileid);
                }
                if (p_hash_value_pro->bitmap_command & GLB_HASH_VALUE_PROFILE_INPUT_CFG)
                {
                    CFG_CONFLICT_RET("Only port-channel select configure support on output");
                }
                if (!(p_hash_value_pro->bitmap_command & GLB_COMMAND_PORT_CHANNEL_SET_ALL))
                {
                    CFG_CONFLICT_RET("Port-channel hash not configure");
                }
                agg_hash = p_hash_value_pro->agg_input;
            }
            /*check duplicate config */
            if (p_if->hash_value_profile_dir == p_db_if->hash_value_profile_dir &&
                p_if->hash_value_profile_id == p_db_if->hash_value_profile_id_agg)
            {
                return PM_E_NONE;
            }
            p_if->hash_value_agg_hash = agg_hash;
            p_if->hash_value_profile_id_agg = p_if->hash_value_profile_id;
            PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_HASH_VALUE_AGG_HASH));
            PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID_AGG));
        }
        else
        {
            /*check duplicate config */
            if (p_if->hash_value_profile_dir == p_db_if->hash_value_profile_dir &&
                p_if->hash_value_profile_id == p_db_if->hash_value_profile_id)
            {
                return PM_E_NONE;
            }
            PM_E_RETURN(if_mgr_set_hash_value_profile_select(p_if));
        }
    }
    else
    {
        /*check duplicate config */
        if (p_if->hash_value_profile_dir == p_db_if->hash_value_profile_dir &&
            p_if->hash_value_profile_id == p_db_if->hash_value_profile_id)
        {
            return PM_E_NONE;
        }
        PM_E_RETURN(if_mgr_set_hash_value_profile_select(p_if));
    }
    PM_E_RETURN(hash_value_profile_unlock(old_profileid));
    PM_E_RETURN(hash_value_profile_lock(new_profileid));

    return rc;
}

int32
if_mgr_cmd_process_interface(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_interface_t intf;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
#ifdef CONFIG_PVLAN
    vid_t primary_vlan = 0;
    vid_t community_vlan = 0;
    int32 pvlan_type = GLB_IF_PVLAN_TYPE_PROMISCUOUS;
#endif /* CONFIG_PVLAN */
    int32 i = 0;
    char agg_ifname[IFNAME_SIZE];
    char *ifname = NULL;
    tbl_interface_t *p_db_if = NULL;
    field_parse_t field;
    uint32 show_type = IF_MGR_SHOW_ALL;
    uint32 parse_field = TRUE;
    uint32 only_key = FALSE;
#ifdef CONFIG_SFLOW
    uint8 sflow_rate = 0;
    uint32 sflow_glb_enable = FALSE;
#endif /* CONFIG_SFLOW */
#ifdef CONFIG_VLANCLASS
    uint32 vclass_group_id = 0;
    uint32 vclass_activate_type = 0;
    uint32 vclass_default_action = 0;
#endif /* CONFIG_VLANCLASS */
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 desc_len = 0;
    int32 rc = PM_E_NONE;
    uint32 stm_max_ingress = 0;
    uint32 stm_max_egress = 0;
    uint32 high_thrd = 0; 
    uint32 low_thrd = 0; 
    tbl_hash_value_profile_t *p_db_hash_value_pro = NULL;
    tbl_hash_value_profile_key_t hash_value_pro_key;
    uint32 hash_value_pro_dir = 0;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_INTERFACE);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (1 < argc && 0 == sal_strcmp(argv[1], "lag"))
        {
            rc = _if_mgr_cmd_process_interface_lag(argv, argc, para);
        }
        else
        {
            rc = _if_mgr_cmd_add_if(argv[0], argv, argc, para);
        }
        break;

    case CDB_OPER_DEL:
        if (1 == argc)
        {
            rc = _if_mgr_cmd_del_if(argv[0], para);
        }
        else if ((2 == argc) && (0 == sal_strcmp(argv[1], "stats")))
        {
            sal_memset(&intf, 0, sizeof(intf));
            sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);

            p_db_if = tbl_interface_get_interface(&intf.key);
            if (p_db_if)
            {
                rc = if_mgr_clear_if_stats(p_db_if, NULL);
            }
            else
            {
                p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)if_mgr_clear_if_stats, NULL);
            }
        }
        else if (0 == sal_strcmp(argv[1], "dhclient_if"))
        {
            sal_memset(&intf, 0, sizeof(intf));
            sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);

            p_db_if = tbl_interface_get_interface(&intf.key);
            if (!p_db_if)
            {
                CFG_CONFLICT_RET("Interface %s not found", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
            }
            rc = _if_mgr_cmd_process_interface_unset_dhcp_client(argv, argc, para, p_db_if);
        }

        else
        {
            rc = _if_mgr_cmd_process_interface_lag(argv, argc, para);
        }
        break;

    case CDB_OPER_SET:

        if (1 < argc && 0 == sal_strcmp(argv[1], "lag"))
        {
            rc = _if_mgr_cmd_process_interface_lag(argv, argc, para);
            return rc;
        }
        sal_memset(&intf, 0, sizeof(intf));
        sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);

        /* check exist */
        p_db_if = tbl_interface_get_interface(&intf.key);
        if (!p_db_if)
        {
            CFG_CONFLICT_RET("Interface %s not found", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
        }
        intf.ifindex = p_db_if->ifindex;
        
        i++;
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }
        switch (field_id)
        {
        case TBL_INTERFACE_FLD_DESC:
            if (argc > i+1)
            {
                desc_len = sal_strlen(argv[i+1]);
                if (desc_len > DESC_SIZE)
                {
                    CFG_INVALID_PARAM_RET("Description length should be [1, 64]");
                }
                rc = cdb_check_name(argv[i+1], DESC_SIZE, para->p_rs->desc);
                if (rc < 0)
                {
                    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
                    return rc;
                }
                sal_strncpy(intf.desc, argv[i+1], DESC_SIZE);
            }
            rc = _if_mgr_cmd_set_desc(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_SVLAN_TPID:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);

#ifdef TAPPRODUCT
            if (!sal_strcmp("0x8100", argv[i+1]))
            {
                intf.svlan_tpid = 0x8100;
            }
            else if (!sal_strcmp("0x9100", argv[i+1]))
            {
                intf.svlan_tpid = 0x9100;
            }
            else
            {
                intf.svlan_tpid = 0x88A8;
            }
#else
            if (sal_strlen(argv[i+1]) > 8 || sal_strspn(argv[i+1], "0123456789AaBbCcDdEeFf") != sal_strlen(argv[i+1]))
            {
                CFG_INVALID_PARAM_RET("Invalid TPID %s", argv[i+1]);
            }
            value = strtoul(argv[i+1], NULL, 16);
            intf.svlan_tpid = value;
#endif
            rc = _if_mgr_cmd_set_svlan_tpid(&intf, p_db_if, para);

            break;
        case TBL_INTERFACE_FLD_SPEED:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            /* added by tongzb for bug 44092, support config media type on CNOS */
            if(GLB_FLAG_ISSET(p_db_if->phyport_flag, GLB_PHYPORT_FLAG_COMBO)&&(p_db_if->media == GLB_PORT_TYPE_AUTO))
            {
                CFG_CONFLICT_RET("Can not set speed with interface media type auto select");
                return PM_E_NOT_SUPPORT;
            }
            if (!sal_strcmp("default", argv[i+1]))
            {
                intf.speed = GLB_SPEED_MAX;
            }
            else if (!sal_strcmp("auto", argv[i+1]))
            {
                intf.speed = GLB_SPEED_AUTO;
            }
            else
            {
                PM_CFG_GET_ENUM(value, glb_port_speed_strs, GLB_PORT_SPEED_MAX, argv[i+1], para->p_rs);
                intf.speed = value;
            }
            
            rc = _if_mgr_cmd_set_speed(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_DUPLEX:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            /* added by tongzb for bug 44092, support config media type on CNOS */
            if(GLB_FLAG_ISSET(p_db_if->phyport_flag, GLB_PHYPORT_FLAG_COMBO)&&(p_db_if->media == GLB_PORT_TYPE_AUTO))
            {
                CFG_CONFLICT_RET("Can not set duplex with interface media type auto select");
                return PM_E_NOT_SUPPORT;
            }
            if (!sal_strcmp("default", argv[i+1]))
            {
                intf.duplex = GLB_DUPLEX_MAX;
            }
            else
            {
                PM_CFG_GET_ENUM(value, glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, argv[i+1], para->p_rs);
                intf.duplex = value;
            }

            rc = _if_mgr_cmd_set_duplex(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_METADATA_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            
            if (!sal_strcmp("1", argv[i+1]))
            {
                intf.metadata_enable= TRUE;
            }
            else
            {
                intf.metadata_enable= FALSE;
            }

            rc = _if_mgr_cmd_set_metadata(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_UNIDIR:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_ENUM(value, glb_port_unidir_strs, GLB_PORT_UNIDIR_MAX, argv[i+1], para->p_rs);
            intf.unidir = value;
            rc = _if_mgr_cmd_set_unidir(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_FEC:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            if ((GLB_CHIP_GREATEBELT == tbl_chip_get_type())
                ||(GLB_CHIP_GOLDENGATE == tbl_chip_get_type()))
            {
                if (!sal_strcmp("enable", argv[i+1]))
                {
                    intf.fec = 1;
                }
                else if (!sal_strcmp("disable", argv[i+1]))
                {
                    intf.fec = 0;
                }
                else if (!sal_strcmp("default", argv[i+1]))
                {
                    intf.fec = 0;
                }                
                else
                {
                    CFG_INVALID_PARAM_RET("Invalid fec configure on GB/GG board!");
                }
            }
            else
            {
                if (!sal_strcmp("none", argv[i+1]))
                {
                    intf.fec = GLB_FEC_OFF;
                }
                else if (!sal_strcmp("rs", argv[i+1]))
                {
                    intf.fec = GLB_FEC_RS;
                }
                else if (!sal_strcmp("baser", argv[i+1]))
                {
                    intf.fec = GLB_FEC_BASE_R;
                }
                else if (!sal_strcmp("default", argv[i+1]))
                {
                    intf.fec = GLB_FEC_DEFAULT|GLB_FEC_ALL;
                } 
                else
                {
                    CFG_INVALID_PARAM_RET("Invalid fec configure on D2/TM board!");
                }
            }
            rc = _if_mgr_cmd_set_fec(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_FLOWCONTROL_SEND:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            GLB_SET_FLAG_COND(value, intf.flowcontrol, GLB_IF_FLOW_CONTROL_SEND);
            rc = _if_mgr_cmd_set_flowcontrol_send(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_FLOWCONTROL_RECEIVE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            GLB_SET_FLAG_COND(value, intf.flowcontrol, GLB_IF_FLOW_CONTROL_RECEIVE);
            rc = _if_mgr_cmd_set_flowcontrol_receive(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_PFC_ADMIN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_ENUM(value, glb_port_pfc_mode, GLB_PFC_FLOWCTRL_MAX, argv[i+1], para->p_rs);
            intf.pfc_admin = value;
            rc = _if_mgr_cmd_set_pfc_en(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_PFC_PRIORITY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            uint8 priority = 0;
            uint8 is_add = FALSE;
            if (0 == sal_strcmp(argv[i+1], "add"))
            {
                is_add = TRUE;
            }
            else if (0 == sal_strcmp(argv[i+1], "del"))
            {
                is_add = FALSE;
            }
            PM_CFG_GET_UINT(priority, argv[i+2], para->p_rs);
            rc = _if_mgr_cmd_set_pfc_priority(&intf, p_db_if, priority, is_add, para);
            break;
        case TBL_INTERFACE_FLD_CARRIER_DOWN_HOLDTIME_INTERVAL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_INTEGER(value, argv[i+1], para->p_rs);
            intf.carrier_down_holdtime_interval = value;
            rc = _if_mgr_cmd_set_holdtime(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_JUMBOFRAME_EN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            intf.jumboframe_en = value;
            rc = _if_mgr_cmd_set_jumboframe_en(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_ISOLATE_GROUP_ID:
            IF_MGR_CHECK_OPENFLOW_PORT(p_db_if);
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            intf.isolate_group_id = value;
            rc = _if_mgr_cmd_set_isolate_group_id(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_DHCP_SNOOPING_TRUST:
            IF_MGR_CHECK_OPENFLOW_PORT(p_db_if);
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            intf.dhcp_snooping_trust = value;
            rc = _if_mgr_cmd_set_dhcp_snooping_trust(&intf, p_db_if, para);
            break;
#ifdef CONFIG_PVLAN
        case TBL_INTERFACE_FLD_PVLAN_TYPE:
            IF_MGR_CHECK_OPENFLOW_PORT(p_db_if);
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_ENUM(pvlan_type, glb_if_pvlan_type_strs, GLB_IF_PVLAN_TYPE_MAX, argv[i+1], para->p_rs);
            intf.pvlan_type = pvlan_type;
            rc = _if_mgr_cmd_process_interface_set_pvlan_mode(&intf, p_db_if, para);
            break;    
        case TBL_INTERFACE_FLD_PRIVATE_VLAN:
            IF_MGR_CHECK_OPENFLOW_PORT(p_db_if);
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(primary_vlan, argv[i+1], para->p_rs);
            if (argc >3)
            {
                PM_CFG_GET_ENUM(pvlan_type, glb_if_pvlan_type_strs, GLB_IF_PVLAN_TYPE_MAX, argv[i+2], para->p_rs);
                if(argc > 4)
                {
                    PM_CFG_GET_UINT(community_vlan, argv[i+3], para->p_rs);
                }
            }
            intf.private_vlan = primary_vlan;
            intf.pvlan_type = pvlan_type;
            intf.community_vlan = community_vlan;
            rc = _if_mgr_cmd_process_interface_set_pvlan(&intf, p_db_if, para);
            break;       
#endif /* CONFIG_PVLAN */
#ifdef CONFIG_L2
        case TBL_INTERFACE_FLD_MODE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_ENUM(value, glb_if_mode_strs, GLB_IF_MODE_MAX, argv[i+1], para->p_rs);
            intf.mode = value;
            rc = _if_mgr_cmd_set_mode(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_VLAN_TYPE:
            IF_MGR_CHECK_OPENFLOW_PORT(p_db_if);
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_ENUM(value, glb_vlan_port_type_strs, GLB_VLAN_PORT_TYPE_MAX, argv[i+1], para->p_rs);
            intf.vlan_type = value;
            rc = _if_mgr_cmd_set_vlan_type(&intf, p_db_if, para);
            break;
#endif /* CONFIG_L2 */
        case TBL_INTERFACE_FLD_UP:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            GLB_SET_FLAG_COND(value, intf.flags, GLB_IFF_UP);
            rc = _if_mgr_cmd_set_up(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_MTU:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            intf.mtu = value;
            rc = _if_mgr_cmd_set_mtu(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_IP_SOURCE_TYPE:
            //PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            rc = _if_mgr_cmd_set_ipsg_enable(argv, argc, &intf, p_db_if, para);
            break;   

        case TBL_INTERFACE_FLD_IP_SOURCE_ITEM_NUMBER:
            break; 
            
        case TBL_INTERFACE_FLD_BANDWIDTH:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            intf.bandwidth = value;
            rc = _if_mgr_cmd_set_bandwidth(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_CIRCUIT_ID:
            /* ds process */
            return _if_mgr_cmd_process_interface_set_circuit_id(argv, argc, para, p_db_if);
            break;
#ifdef CONFIG_L2
        case TBL_INTERFACE_FLD_BRGIF:
            IF_MGR_CHECK_OPENFLOW_PORT(p_db_if);
            /* ds process */
            return _if_mgr_cmd_process_interface_set_brgif(argv, argc, para, p_db_if);
            break;
#endif /* CONFIG_L2 */
        case TBL_INTERFACE_FLD_LACP:
            /* ds process */
            return _if_mgr_cmd_process_interface_set_lacp(argv, argc, para, p_db_if);
            break;  

        case TBL_INTERFACE_FLD_DHCLIENT_IF:
            /* ds process */
            return _if_mgr_cmd_process_interface_set_dhcp_client(argv, argc, para, p_db_if);
            break;  

        case TBL_INTERFACE_FLD_STORM_CONTROL:
            IF_MGR_CHECK_OPENFLOW_PORT(p_db_if);
            /* ds process */
            return _if_mgr_cmd_process_interface_set_storm_control(argv, argc, para, p_db_if);
            break;      
        case TBL_INTERFACE_FLD_ACLQOS_IF:
            IF_MGR_CHECK_OPENFLOW_PORT(p_db_if);
            /* ds process */
            return _if_mgr_cmd_process_interface_set_aclqos_if(argv, argc, para, p_db_if);
            break;
#ifdef CONFIG_SFLOW
        case TBL_INTERFACE_FLD_SFLOW_COUNTER_EN:
            sflow_glb_enable = sflow_api_get_sflow_enable();
            if (!sflow_glb_enable)
            {
                CFG_INVALID_PARAM_RET("sFlow is not enabled globally!");
            }

            if ((GLB_IF_TYPE_PORT_IF != p_db_if->hw_type) && (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type))
            {
                CFG_INVALID_PARAM_RET("sFlow is only supported on physical interface or LAG");
            }
            
            if (INTERFACE_IS_LAG_MEMBER(p_db_if))
            {
                CFG_INVALID_PARAM_RET("sFlow is not supported on LAG member port");
            }

            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);

            rc = if_mgr_set_sflow_counter_enable(p_db_if, value);
            if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
            {
                CFG_PROMPT_RET("Warning: Both member-port and agg-logical-port will be collected");
            }
            break;
        case TBL_INTERFACE_FLD_SFLOW_FLOW_EN:
            sflow_glb_enable = sflow_api_get_sflow_enable();
            if (!sflow_glb_enable)
            {
                CFG_INVALID_PARAM_RET("sFlow is not enabled globally!");
            }

            if ((GLB_IF_TYPE_PORT_IF != p_db_if->hw_type) && (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type))
            {
                CFG_INVALID_PARAM_RET("sFlow is only supported on physical interface or LAG");
            }
            
            if (INTERFACE_IS_LAG_MEMBER(p_db_if))
            {
                CFG_INVALID_PARAM_RET("sFlow is not supported on LAG member port");
            }

            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);

            if (0 == sal_strcmp(argv[3], "both"))
            {
                rc = if_mgr_set_sflow_flow_enable(p_db_if, value, TRUE, TRUE);
            }
            else if (0 == sal_strcmp(argv[3], "input"))
            {
                rc = if_mgr_set_sflow_flow_enable(p_db_if, value, TRUE, FALSE);
            }
            else
            {
                rc = if_mgr_set_sflow_flow_enable(p_db_if, value, FALSE, TRUE);
            }
            break;

        case TBL_INTERFACE_FLD_SFLOW_RATE:
            sflow_glb_enable = sflow_api_get_sflow_enable();
            if (!sflow_glb_enable)
            {
                CFG_INVALID_PARAM_RET("sFlow is not enabled globally!");
            }

            if ((GLB_IF_TYPE_PORT_IF != p_db_if->hw_type) && (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type))
            {
                CFG_INVALID_PARAM_RET("sFlow is only supported on physical interface or LAG");
            }
            
            if (INTERFACE_IS_LAG_MEMBER(p_db_if))
            {
                CFG_INVALID_PARAM_RET("sFlow is not supported on LAG member port");
            }

            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);

            if (0 == value)
            {
                value = GLB_SFLOW_DEFAULT_RATE;
            }

            rc = if_mgr_sflow_if_map_rate(value, &sflow_rate);
            if (PM_E_INVALID_PARAM == rc)
            {
                CFG_CONFLICT_RET("Failed, sample rate value must be a power of 2, allowed values are:\n    32    64  128  ...");
            }

#ifdef GREATBELT
            if (value > GLB_SFLOW_DEFAULT_RATE)
            {
                CFG_INVALID_PARAM_RET("This sflow sampling rate can't be supported on this platform");
            }
#endif

            rc = if_mgr_set_sflow_rate(p_db_if, value);
            if (rc)
            {
                return rc;    
            }


            if (value < GLB_SFLOW_DEFAULT_RATE)
            /* liwh end */
            {
#ifdef GREATBELT
                CFG_PROMPT_RET("Warning: sFlow sampling requires high CPU usage, especially with a low rate.\n"
                               "Suggested rate not less than 8192.");
#else
                CFG_PROMPT_RET("Warning: sFlow sampling requires high CPU usage, especially with a low rate.\n"
                               "Suggested rate not less than 8192.");
#endif
            }  
            break;
#endif /* CONFIG_SFLOW */

#ifdef CONFIG_VLANCLASS
        case TBL_INTERFACE_FLD_VLANCLASS_GROUP_ID:
            IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 2, 5, para->p_rs);
            if (argc == 2)
            {                
                rc = _if_mgr_cmd_set_vlanclass_del_group_from_if(&intf, p_db_if, para);           
            }
            else if (argc == 5)
            {
                PM_CFG_GET_UINT(vclass_group_id, argv[i+1], para->p_rs);
                if (0 == sal_strcmp(argv[i+3], "mac"))
                {
                    vclass_activate_type = GLB_VLAN_CLASS_MAC_BASED;
                }
                else if (0 == sal_strcmp(argv[i+3], "ip"))
                {
                    vclass_activate_type = GLB_VLAN_CLASS_IP_BASED;
                }
                else if (0 == sal_strcmp(argv[i+3], "protocol"))
                {
                     vclass_activate_type = GLB_VLAN_CLASS_PROTOCOL_BASED;
                }
                else
                {   
                    CFG_INVALID_PARAM_RET("Invalid activate type!");
                }
                intf.vlanclass_group_id = vclass_group_id;                
                intf.vlanclass_active_type = vclass_activate_type;                
                rc = _if_mgr_cmd_set_vlanclass_add_group_to_if(&intf, p_db_if, para);      
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid parameter number!");
            }
            break;
        case TBL_INTERFACE_FLD_VLANCLASS_DEFAULT_ACTION:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);            
            PM_CFG_GET_UINT(vclass_default_action, argv[i+1], para->p_rs);            
            intf.vlanclass_default_action = vclass_default_action;            
            rc = _if_mgr_cmd_set_vlanclass_default_action(&intf, p_db_if, para);            
            break;
#endif /* CONFIG_VLANCLASS */

        case TBL_INTERFACE_FLD_CRC_ERROR_CHECK_EN:
            /* ds process */
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            intf.crc_error_check_en = value;
            return _if_mgr_cmd_set_crc_check_enable(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_CRC_RECALCULATION_EN:
            /* ds process */
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            intf.crc_recalculation_en = value;
            return _if_mgr_cmd_set_crc_recalculation_enable(&intf, p_db_if, para);
            break; 

        case TBL_INTERFACE_FLD_IGS_ACL:
            #ifdef OFPRODUCT
            //CFG_INVALID_PARAM_RET("Hybrid system unsupport to set acl!");
            CFG_UNSUPPORT_PRODUCT_RET();
            #endif
            if (0 != sal_strncmp(argv[0], "eth", 3) && 0 != sal_strncmp(argv[0], "agg", 3))
            {
                CFG_INVALID_PARAM_RET("IP access-list no support interface %s", argv[0]);
            }
            if (argc == 3)
            {
                if (INTERFACE_IS_LAG_MEMBER(p_db_if))
                {
                    CFG_INVALID_PARAM_RET("IP access-list is not supported on link-agg member!");
                }

                /*modified by yejl for bug 49698, only one of ip acl,mac acl,ipv6 acl can be apply on one interface, 2018-10-26*/
                if (0 != sal_strlen(p_db_if->igs_acl))
                {
                    CFG_INVALID_PARAM_RET("IP access-list %s should be removed first", p_db_if->igs_acl);
                }
                if (0 != sal_strlen(p_db_if->igs_ipv6_acl))
                {
                    CFG_INVALID_PARAM_RET("IPv6 access-list %s should be removed first", p_db_if->igs_ipv6_acl);
                }
                if (0 != sal_strlen(p_db_if->igs_mac_acl))
                {
                    CFG_INVALID_PARAM_RET("MAC access-list %s should be removed first", p_db_if->igs_mac_acl);
                }
                /* modify by huwx for bug 47270, port acl&service policy confilct check */
                if (0 != sal_strlen(p_db_if->igs_policy_map))
                {
                    CFG_INVALID_PARAM_RET("Servic-policy map %s should be removed first", p_db_if->igs_policy_map);
                }
                rc = acl_attach_interface(argv[2], argv[0], GLB_ACL_CONFIG_TYPE_NORMAL, TRUE);
                if (PM_E_INVALID_IFNAME == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[0]);
                }
                else if (PM_E_NOT_EXIST == rc)
                {
                    CFG_INVALID_PARAM_RET("IP access-list %s should be configured first", argv[2]);
                }
                else if (PM_E_NOT_SUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("The %s is not the ip access list name", argv[2]);
                }
                else if (PM_E_CFG_NO_RESOURCE == rc)
                {
					if(check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE))
					{
		            	/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress entry number is %d/%d",
	                                            stm_max_ingress, stm_max_egress);
					}
                    else
                    {
						/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress:ingress6/egress6 entry number is %d/%d:%d/%d",
	                                            stm_max_ingress, stm_max_egress,
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS),
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS));
					}
                }
                else if (PM_E_RESOURCE_FULL == rc)
                {
                    CFG_INVALID_PARAM_RET("Exceeded maximum port range or lt or gt number");
                }
                else if (PM_E_CFG_ACL_TCP_NO_RESPURCE == rc)
                {
                    CFG_INVALID_PARAM_RET("Exceeded maximum tcp flags entry number 4");
                }
                else if (PM_E_INVALID_PARAM == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid ip-access list name");
                }
            }
            else
            {
                rc = acl_detach_interface(p_db_if->igs_acl, argv[0], GLB_ACL_CONFIG_TYPE_NORMAL, TRUE);
                if (PM_E_NOT_EXIST == rc)
                {
                    CFG_INVALID_PARAM_RET("IP access-list is not applied, please configure first");
                }
                else if (PM_E_INVALID_IFNAME == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[0]);
                }
                else if (PM_E_NOT_SUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("The %s is not the ip access list name", p_db_if->igs_acl);
                }
            }
            break;
        case TBL_INTERFACE_FLD_IGS_IPV6_ACL:
            #ifdef OFPRODUCT
            CFG_UNSUPPORT_PRODUCT_RET();
            #endif
            if (FALSE == acl_stm_is_ipv6_profile())
            {
                CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
            }   
            
            if (0 != sal_strncmp(argv[0], "eth", 3) && 0 != sal_strncmp(argv[0], "agg", 3))
            {
                CFG_INVALID_PARAM_RET("IPv6 access-list no support interface %s", argv[0]);
            }
            if (3 == argc)
            {
                if (INTERFACE_IS_LAG_MEMBER(p_db_if))
                {
                    CFG_INVALID_PARAM_RET("IPv6 access-list is not supported on link-agg member!");
                }
                
                /*modified by yejl for bug 49698, only one of ip acl,mac acl,ipv6 acl can be apply on one interface, 2018-10-26*/
                if (0 != sal_strlen(p_db_if->igs_ipv6_acl))
                {
                    CFG_INVALID_PARAM_RET("IPv6 access-list %s should be removed first", p_db_if->igs_ipv6_acl);
                }
                if (0 != sal_strlen(p_db_if->igs_acl))
                {
                    CFG_INVALID_PARAM_RET("IP access-list %s should be removed first", p_db_if->igs_acl);
                }
                if (0 != sal_strlen(p_db_if->igs_mac_acl))
                {
                    CFG_INVALID_PARAM_RET("MAC access-list %s should be removed first", p_db_if->igs_mac_acl);
                }
                /* modify by huwx for bug 47270, port acl&service policy confilct check */
                if (0 != sal_strlen(p_db_if->igs_policy_map))
                {
                    CFG_INVALID_PARAM_RET("Servic-policy map %s should be removed first", p_db_if->igs_policy_map);
                }

                rc = acl_attach_interface(argv[2], argv[0], GLB_ACL_CONFIG_TYPE_IPV6, TRUE);
                if (PM_E_INVALID_IFNAME == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[0]);
                }
                else if (PM_E_NOT_EXIST == rc)
                {
                    CFG_INVALID_PARAM_RET("IPv6 access-list %s should be configured first", argv[2]);
                }
                else if (PM_E_NOT_SUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("The %s is not the ipv6 access-list name", argv[2]);
                }
                else if (PM_E_CFG_NO_RESOURCE == rc)
                {
					if (check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE))
					{
		            	/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress entry number is %d/%d",
	                                            stm_max_ingress, stm_max_egress);
					}
                    else
					{
						/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress:ingress6/egress6 entry number is %d/%d:%d/%d",
	                                            stm_max_ingress, stm_max_egress,
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS),
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS));
					}
                }
                else if (PM_E_RESOURCE_FULL == rc)
                {
                    CFG_INVALID_PARAM_RET("Exceeded maximum port range or lt or gt number");
                }
                else if (PM_E_CFG_ACL_TCP_NO_RESPURCE == rc)
                {
                    CFG_INVALID_PARAM_RET("Exceeded maximum tcp flags entry number 4");
                }
                else if (PM_E_INVALID_PARAM == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid ipv6-access list name");
                }
            }
            else
            {
                rc = acl_detach_interface(p_db_if->igs_ipv6_acl, argv[0], GLB_ACL_CONFIG_TYPE_IPV6, TRUE);
                if (PM_E_NOT_EXIST == rc)
                {
                    CFG_INVALID_PARAM_RET("IPv6 access-list is not applied, please configure first");
                }
                else if (PM_E_INVALID_IFNAME == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[0]);
                }
                else if (PM_E_NOT_SUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("The %s is not the ipv6 access-list name", p_db_if->igs_ipv6_acl);
                }
            }
            break;

        case TBL_INTERFACE_FLD_IGS_MAC_ACL:
            #ifdef OFPRODUCT
            CFG_UNSUPPORT_PRODUCT_RET();
            #endif
            if (0 != sal_strncmp(argv[0], "eth", 3) && 0 != sal_strncmp(argv[0], "agg", 3))
            {
                CFG_INVALID_PARAM_RET("Mac access-list no support interface %s", argv[0]);
            }
            if (3 == argc)
            {
                if (INTERFACE_IS_LAG_MEMBER(p_db_if))
                {
                    CFG_INVALID_PARAM_RET("Mac access-list is not supported on link-agg member!");
                }
                
                /*modified by yejl for bug 49698, only one of ip acl,mac acl,ipv6 acl can be apply on one interface, 2018-10-26*/
                if (0 != sal_strlen(p_db_if->igs_mac_acl))
                {
                    CFG_INVALID_PARAM_RET("Mac access-list %s should be removed first", p_db_if->igs_mac_acl);
                }
                if (0 != sal_strlen(p_db_if->igs_acl))
                {
                    CFG_INVALID_PARAM_RET("IP access-list %s should be removed first", p_db_if->igs_acl);
                }
                if (0 != sal_strlen(p_db_if->igs_ipv6_acl))
                {
                    CFG_INVALID_PARAM_RET("IPv6 access-list %s should be removed first", p_db_if->igs_ipv6_acl);
                }
                /* modify by huwx for bug 47270, port acl&service policy confilct check */
                if (0 != sal_strlen(p_db_if->igs_policy_map))
                {
                    CFG_INVALID_PARAM_RET("Servic-policy map %s should be removed first", p_db_if->igs_policy_map);
                }

                rc = acl_attach_interface(argv[2], argv[0], GLB_ACL_CONFIG_TYPE_MAC, TRUE);
                if (PM_E_INVALID_IFNAME == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[0]);
                }
                else if (PM_E_NOT_EXIST == rc)
                {
                    CFG_INVALID_PARAM_RET("Mac access-list %s should be configured first", argv[2]);
                }
                else if (PM_E_NOT_SUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("The %s is not the mac access-list name", argv[2]);
                }
                else if (PM_E_CFG_NO_RESOURCE == rc)
                {
					if (check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE))
					{
		            	/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress entry number is %d/%d",
	                                            stm_max_ingress, stm_max_egress);
					}
                    else
					{
						/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress:ingress6/egress6 entry number is %d/%d:%d/%d",
	                                            stm_max_ingress, stm_max_egress,
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS),
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS));
					}
                }
                else if (PM_E_RESOURCE_FULL == rc)
                {
                    CFG_INVALID_PARAM_RET("Exceeded maximum port range or lt or gt number");
                }
                else if (PM_E_CFG_ACL_TCP_NO_RESPURCE == rc)
                {
                    CFG_INVALID_PARAM_RET("Exceeded maximum tcp flags entry number 4");
                }
                else if (PM_E_INVALID_PARAM == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid mac-access list name");
                }
            }
            else
            {
                rc = acl_detach_interface(p_db_if->igs_mac_acl, argv[0], GLB_ACL_CONFIG_TYPE_MAC, TRUE);
                if (PM_E_NOT_EXIST == rc)
                {
                    CFG_INVALID_PARAM_RET("Mac access-list is not applied, please configure first");
                }
                else if (PM_E_INVALID_IFNAME == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[0]);
                }
                else if (PM_E_NOT_SUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("The %s is not the mac access-list name", p_db_if->igs_mac_acl);
                }
            }
            break;
            
        case TBL_INTERFACE_FLD_EGS_ACL:
            #ifdef OFPRODUCT
            CFG_UNSUPPORT_PRODUCT_RET();
            #endif

            if (0 != sal_strncmp(argv[0], "eth", 3) && 0 != sal_strncmp(argv[0], "agg", 3))
            {
                CFG_INVALID_PARAM_RET("IP access-list not support interface %s", argv[0]);
            }
            if (3 == argc)
            {
                if (INTERFACE_IS_LAG_MEMBER(p_db_if))
                {
                    CFG_INVALID_PARAM_RET("IP access-list is not supported on link-agg member!");
                }

                /*modified by yejl for bug 49698, only one of ip acl,mac acl,ipv6 acl can be apply on one interface, 2018-10-26*/
                if (0 != sal_strlen(p_db_if->egs_acl))
                {
                    CFG_INVALID_PARAM_RET("IP access-list %s should be removed first", p_db_if->egs_acl);
                }
                if (0 != sal_strlen(p_db_if->egs_ipv6_acl))
                {
                    CFG_INVALID_PARAM_RET("IPv6 access-list %s should be removed first", p_db_if->egs_ipv6_acl);
                }
                if (0 != sal_strlen(p_db_if->egs_mac_acl))
                {
                    CFG_INVALID_PARAM_RET("MAC access-list %s should be removed first", p_db_if->egs_mac_acl);
                }

                /* modify by huwx for bug 47270, port acl&service policy confilct check */
                if (0 != sal_strlen(p_db_if->egs_policy_map))
                {
                    CFG_INVALID_PARAM_RET("Servic-policy map %s should be removed first", p_db_if->egs_policy_map);
                }

                rc = acl_attach_interface(argv[2], argv[0], GLB_ACL_CONFIG_TYPE_NORMAL, FALSE);
                if (PM_E_NOT_EXIST == rc)
                {
                    CFG_INVALID_PARAM_RET("IP access-list %s should be configured first", argv[2]);
                }
                else if (PM_E_CFG_PM_ERROR == rc)
                {
                    CFG_INVALID_PARAM_RET("Not support ether_type configuration in egress ace on this product");
                }
                else if (PM_E_NOT_SUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("The %s is not the ip access-list name", argv[2]);
                }
                else if (PM_E_CFG_NO_RESOURCE == rc)
                {
                    if (check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE))
					{
		            	/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress entry number is %d/%d",
	                                            stm_max_ingress, stm_max_egress);
					}
                    else
					{
						/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress:ingress6/egress6 entry number is %d/%d:%d/%d",
	                                            stm_max_ingress, stm_max_egress,
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS),
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS));
					}
                }
                else if (PM_E_RESOURCE_FULL == rc)
                {
                    CFG_INVALID_PARAM_RET("Exceeded maximum port range or lt or gt number");
                }
                else if (PM_E_CFG_ACL_TCP_NO_RESPURCE == rc)
                {
                    CFG_INVALID_PARAM_RET("Exceeded maximum tcp flags entry number 4");
                }
                else if (PM_E_INVALID_PARAM == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid ip-access list name");
                }
            }
            else
            {
                rc = acl_detach_interface(p_db_if->egs_acl, argv[0], GLB_ACL_CONFIG_TYPE_NORMAL, FALSE);
                if (PM_E_NOT_EXIST == rc)
                {
                    CFG_INVALID_PARAM_RET("IP access-list is not applied, please configure first");
                }
                else if (PM_E_INVALID_IFNAME == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[0]);
                }
                else if (PM_E_NOT_SUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("The %s is not the ip access-list name", p_db_if->egs_acl);
                }
            }
            break;
        case TBL_INTERFACE_FLD_EGS_IPV6_ACL:
            #ifdef OFPRODUCT
            CFG_UNSUPPORT_PRODUCT_RET();
            #endif
            if (FALSE == acl_stm_is_ipv6_profile())
            {
                CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
            } 
            if (0 != sal_strncmp(argv[0], "eth", 3) && 0 != sal_strncmp(argv[0], "agg", 3))
            {
                CFG_INVALID_PARAM_RET("IPv6 access-list no support interface %s", argv[0]);
            }
            if (3 == argc)
            {
                if (INTERFACE_IS_LAG_MEMBER(p_db_if))
                {
                    CFG_INVALID_PARAM_RET("IPv6 access-list is not supported on link-agg member!");
                }

                /*modified by yejl for bug 49698, only one of ip acl,mac acl,ipv6 acl can be apply on one interface, 2018-10-26*/
                if (0 != sal_strlen(p_db_if->egs_ipv6_acl))
                {
                    CFG_INVALID_PARAM_RET("IPv6 access-list %s should be removed first", p_db_if->egs_ipv6_acl);
                }
                if (0 != sal_strlen(p_db_if->egs_acl))
                {
                    CFG_INVALID_PARAM_RET("IP access-list %s should be removed first", p_db_if->egs_acl);
                }
                if (0 != sal_strlen(p_db_if->egs_mac_acl))
                {
                    CFG_INVALID_PARAM_RET("MAC access-list %s should be removed first", p_db_if->egs_mac_acl);
                }

                /* modify by huwx for bug 47270, port acl&service policy confilct check */
                if (0 != sal_strlen(p_db_if->egs_policy_map))
                {
                    CFG_INVALID_PARAM_RET("Servic-policy map %s should be removed first", p_db_if->egs_policy_map);
                }

                rc = acl_attach_interface(argv[2], argv[0], GLB_ACL_CONFIG_TYPE_IPV6, FALSE);
                if (PM_E_INVALID_IFNAME == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[0]);
                }
                else if (PM_E_NOT_EXIST == rc)
                {
                    CFG_INVALID_PARAM_RET("IPv6 access-list %s should be configured first", argv[2]);
                }
                else if (PM_E_CFG_PM_ERROR == rc)
                {
                    CFG_INVALID_PARAM_RET("Not support ether_type configuration in egress ace on this product");
                }
                else if (PM_E_NOT_SUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("The %s is not the ipv6 access-list name", argv[2]);
                }
                else if (PM_E_CFG_NO_RESOURCE == rc)
                {
                    if (check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE))
					{
		            	/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress entry number is %d/%d",
	                                            stm_max_ingress, stm_max_egress);
					}
                    else
					{
						/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress:ingress6/egress6 entry number is %d/%d:%d/%d",
	                                            stm_max_ingress, stm_max_egress,
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS),
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS));
					}
                }
                else if (PM_E_RESOURCE_FULL == rc)
                {
                    CFG_INVALID_PARAM_RET("Exceeded maximum port range or lt or gt number");
                }
                else if (PM_E_CFG_ACL_TCP_NO_RESPURCE == rc)
                {
                    CFG_INVALID_PARAM_RET("Exceeded maximum tcp flags entry number 4");
                }
                else if (PM_E_INVALID_PARAM == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid ipv6-access list name");
                }
            }
            else
            {
                rc = acl_detach_interface(p_db_if->egs_ipv6_acl, argv[0], GLB_ACL_CONFIG_TYPE_IPV6, FALSE);
                if (PM_E_INVALID_IFNAME == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[0]);
                }
                else if (PM_E_NOT_EXIST == rc)
                {
                    CFG_INVALID_PARAM_RET("IPv6 access-list is not applied, please configure first");
                }
                else if (PM_E_NOT_SUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("The %s is not the ipv6 access-list name", p_db_if->egs_ipv6_acl);
                }
            }
            break;

        case TBL_INTERFACE_FLD_EGS_MAC_ACL:
            #ifdef OFPRODUCT
            CFG_UNSUPPORT_PRODUCT_RET();
            #endif
            if (0 != sal_strncmp(argv[0], "eth", 3) && 0 != sal_strncmp(argv[0], "agg", 3))
            {
                CFG_INVALID_PARAM_RET("Mac access-list no support interface %s", argv[0]);
            }
            if (3 == argc)
            {
                if (INTERFACE_IS_LAG_MEMBER(p_db_if))
                {
                    CFG_INVALID_PARAM_RET("Mac access-list is not supported on link-agg member!");
                }

                /*modified by yejl for bug 49698, only one of ip acl,mac acl,ipv6 acl can be apply on one interface, 2018-10-26*/
                if (0 != sal_strlen(p_db_if->egs_mac_acl))
                {
                    CFG_INVALID_PARAM_RET("Mac access-list %s should be removed first", p_db_if->egs_mac_acl);
                }
                if (0 != sal_strlen(p_db_if->egs_acl))
                {
                    CFG_INVALID_PARAM_RET("IP access-list %s should be removed first", p_db_if->egs_acl);
                }
                if (0 != sal_strlen(p_db_if->egs_ipv6_acl))
                {
                    CFG_INVALID_PARAM_RET("IPv6 access-list %s should be removed first", p_db_if->egs_ipv6_acl);
                }

                /* modify by huwx for bug 47270, port acl&service policy confilct check */
                if (0 != sal_strlen(p_db_if->egs_policy_map))
                {
                    CFG_INVALID_PARAM_RET("Servic-policy map %s should be removed first", p_db_if->egs_policy_map);
                }
                
                rc = acl_attach_interface(argv[2], argv[0], GLB_ACL_CONFIG_TYPE_MAC, FALSE);
                if (PM_E_INVALID_IFNAME == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[0]);
                }
                else if (PM_E_NOT_EXIST == rc)
                {
                    CFG_INVALID_PARAM_RET("Mac access-list %s should be configured first", argv[2]);
                }
                else if (PM_E_CFG_PM_ERROR == rc)
                {
                    CFG_INVALID_PARAM_RET("Not support ether_type configuration in egress ace on this product");
                }
                else if (PM_E_NOT_SUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("The %s is not the mac access-list name", argv[2]);
                }
                else if (PM_E_CFG_NO_RESOURCE == rc)
                {
                    if (check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE))
					{
		            	/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress entry number is %d/%d",
	                                            stm_max_ingress, stm_max_egress);
					}
                    else
					{
						/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress:ingress6/egress6 entry number is %d/%d:%d/%d",
	                                            stm_max_ingress, stm_max_egress,
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS),
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS));
					}
                }
                else if (PM_E_RESOURCE_FULL == rc)
                {
                    CFG_INVALID_PARAM_RET("Exceeded maximum port range or lt or gt number");
                }
                else if (PM_E_CFG_ACL_TCP_NO_RESPURCE == rc)
                {
                    CFG_INVALID_PARAM_RET("Exceeded maximum tcp flags entry number 4");
                }
                else if (PM_E_INVALID_PARAM == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid mac-access list name");
                }
            }
            else
            {
                rc = acl_detach_interface(p_db_if->egs_mac_acl, argv[0], GLB_ACL_CONFIG_TYPE_MAC, FALSE);
                if (PM_E_INVALID_IFNAME == rc)
                {
                    CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[0]);
                }
                else if (PM_E_NOT_EXIST == rc)
                {
                    CFG_INVALID_PARAM_RET("Mac access-list is not applied, please configure first");
                }
                else if (PM_E_NOT_SUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("The %s is not the mac access-list name", p_db_if->egs_mac_acl);
                }
            }
            break;

        case TBL_INTERFACE_FLD_IGS_POLICY_MAP:
#ifdef OFPRODUCT
            CFG_UNSUPPORT_PRODUCT_RET();
#endif
            if (0 != sal_strncmp(argv[0], "eth", 3) && 0 != sal_strncmp(argv[0], "agg", 3))
            {
                CFG_INVALID_PARAM_RET("Policy map not support interface %s", argv[0]);
            }
            if (3 == argc)
            {
                if (INTERFACE_IS_LAG_MEMBER(p_db_if))
                {
                    CFG_INVALID_PARAM_RET("Policy map is not supported on link-agg member!");
                }

                if (0 != sal_strlen(p_db_if->igs_policy_map))
                {
                    CFG_INVALID_PARAM_RET("Policy map %s should be removed first", p_db_if->igs_policy_map);
                }

                /* modify by huwx for bug 47270, port acl&service policy confilct check */
                if (0 != sal_strlen(p_db_if->igs_acl))
                {
                    CFG_INVALID_PARAM_RET("IP access-list %s should be removed first", p_db_if->igs_acl);
                }
                if (0 != sal_strlen(p_db_if->igs_ipv6_acl))
                {
                    CFG_INVALID_PARAM_RET("IPv6 access-list %s should be removed first", p_db_if->igs_ipv6_acl);
                }
                if (0 != sal_strlen(p_db_if->igs_mac_acl))
                {
                    CFG_INVALID_PARAM_RET("Mac access-list %s should be removed first", p_db_if->igs_mac_acl);
                }

                rc = policy_map_attach_interface(argv[2], argv[0], TRUE);
                if (PM_E_PMAP_NOT_EXIST == rc)
                {
                    CFG_INVALID_PARAM_RET("Policy map %s should be configured first", argv[2]);
                }
                else if (PM_E_NOT_SUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("Policy map can not support this action");
                }
                else if (PM_E_INVALID_PARAM == rc)
                {
                    CFG_INVALID_PARAM_RET("The initial character of acl name should be a-z, A-Z, or 0-9. The max policy-map name len is 20.");
                }
                else if (PM_E_CFG_NO_RESOURCE == rc)
                {
                    if (check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE))
					{
		            	/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress entry number is %d/%d",
	                                            stm_max_ingress, stm_max_egress);
					}
                    else
					{
						/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress:ingress6/egress6 entry number is %d/%d:%d/%d",
	                                            stm_max_ingress, stm_max_egress,
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS),
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS));
					}
                }
                else if (PM_E_CFG_POLICER_NO_RESOURCE == rc)
                {
                    CFG_INVALID_PARAM_RET("QoS ingress flow policer profile resource full!");
                }
                else if (PM_E_CFG_ACL_TCP_NO_RESPURCE == rc)
                {
                    CFG_INVALID_PARAM_RET("Exceeded maximum tcp flags entry number 4");
                }
                if (PM_E_NONE != rc)
                {
                    CFG_INVALID_PARAM_RET("Policy map configure failed");
                }
            }
            else
            {
                if (0 == sal_strlen(p_db_if->igs_policy_map))
                {
                    CFG_INVALID_PARAM_RET("Can't find input policy-map on interface");
                }
                policy_map_detach_interface(p_db_if->igs_policy_map, argv[0], TRUE);
            }
            break;

        case TBL_INTERFACE_FLD_EGS_POLICY_MAP:
            #ifdef OFPRODUCT
            CFG_UNSUPPORT_PRODUCT_RET();
            #endif
            if (0 != sal_strncmp(argv[0], "eth", 3) && 0 != sal_strncmp(argv[0], "agg", 3))
            {
                CFG_INVALID_PARAM_RET("Policy map not support interface %s", argv[0]);
            }
            if (3 == argc)
            {
                if (INTERFACE_IS_LAG_MEMBER(p_db_if))
                {
                    CFG_INVALID_PARAM_RET("Policy map is not supported on link-agg member!");
                }

                if (0 != sal_strlen(p_db_if->egs_policy_map))
                {
                    CFG_INVALID_PARAM_RET("Policy map %s should be removed first", p_db_if->egs_policy_map);
                }

                /* modify by huwx for bug 47270, port acl&service policy confilct check */
                if (0 != sal_strlen(p_db_if->egs_acl))
                {
                    CFG_INVALID_PARAM_RET("IP access-list %s should be removed first", p_db_if->egs_acl);
                }
                if (0 != sal_strlen(p_db_if->egs_ipv6_acl))
                {
                    CFG_INVALID_PARAM_RET("IPv6 access-list %s should be removed first", p_db_if->egs_ipv6_acl);
                }
                if (0 != sal_strlen(p_db_if->egs_mac_acl))
                {
                    CFG_INVALID_PARAM_RET("Mac access-list %s should be removed first", p_db_if->egs_mac_acl);
                }

                rc = policy_map_attach_interface(argv[2], argv[0], FALSE);
                if (PM_E_PMAP_NOT_EXIST == rc)
                {
                    CFG_INVALID_PARAM_RET("Policy map %s should be configured first", argv[2]);
                }
                else if (PM_E_NOT_SUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("Policy map can not support this action");
                }
                else if (PM_E_INVALID_PARAM == rc)
                {
                    CFG_INVALID_PARAM_RET("The initial character of acl name should be a-z, A-Z, or 0-9. The max policy-map name len is 20.");
                }
                else if (PM_E_CFG_PM_ERROR == rc)
                {
                    CFG_INVALID_PARAM_RET("Not support ether_type configuration in egress ace on this product");
                }
                else if (PM_E_CFG_NO_RESOURCE == rc)
                {
                    if (check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE))
					{
		            	/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress entry number is %d/%d",
	                                            stm_max_ingress, stm_max_egress);
					}
                    else
					{
						/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
	                    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
	                    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
	                    CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress:ingress6/egress6 entry number is %d/%d:%d/%d",
	                                            stm_max_ingress, stm_max_egress,
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS),
	                                            lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS));
					}
                }
                else if (PM_E_CFG_POLICER_NO_RESOURCE == rc)
                {
                    CFG_INVALID_PARAM_RET("QoS egress flow policer profile resource full!");
                }
                else if (PM_E_CFG_ACL_TCP_NO_RESPURCE == rc)
                {
                    CFG_INVALID_PARAM_RET("Exceeded maximum tcp flags entry number 4");
                }
                /*modified by yejl for bug 49938 and bug 49973, 2018-11-13*/
                else if (PM_E_CHECK_CMAP_CTAG_COS_UNSUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("Failed because set c-vlan cos does not support on output");    
                }
                else if (PM_E_CHECK_CMAP_CTAG_UNSUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("Failed because set c-vlan does not support on output");
                }
                else if (PM_E_CHECK_CMAP_DSCP_UNSUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("Failed because set dscp does not support on output");
                }
                else if (PM_E_CHECK_CMAP_STAG_COS_UNSUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("Failed because set s-vlan cos does not support on output");
                }
                else if (PM_E_CHECK_CMAP_STAG_UNSUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("Failed because set s-vlan does not support on output");
                }
                else if (PM_E_CHECK_CMAP_TC_TO_COLOR_UNSUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("Failed because set traffic-class to color does not support on output");
                }
                else if (PM_E_CHECK_CMAP_REDIRECT_UNSUPPORT == rc)
                {
                    CFG_INVALID_PARAM_RET("Failed because redirect to interface does not support on output");
                }
                /*end*/
                else if (PM_E_NONE != rc)
                {
                    CFG_INVALID_PARAM_RET("Policy map configure failed");
                }
            }
            else
            {
                if (0 == sal_strlen(p_db_if->egs_policy_map))
                {
                    CFG_INVALID_PARAM_RET("Can't find output policy-map on interface");
                }
                policy_map_detach_interface(p_db_if->egs_policy_map, argv[0], FALSE);
            }
            break;

        case TBL_INTERFACE_FLD_OPENFLOW_IF:
            IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            if (0 == sal_strcmp(argv[i+1], "enable"))
            {
                PM_CFG_GET_BOOL(value, argv[i+2], para->p_rs);
                rc = _if_mgr_cmd_set_openflow_enable(p_db_if, value, para);
            }
            else
            {
                CFG_INVALID_PARAM_RET("Unsupport to config %s", argv[i+1]);
            }
            break;
            
        case TBL_INTERFACE_FLD_ARPINSP_TRUST_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(intf.arpinsp_trust_enable, argv[i+1], para->p_rs);
            rc = _if_mgr_cmd_set_arpinsp_trust_enable(&intf, p_db_if, para);
            break;
           
        case TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs); 
            PM_CFG_GET_UINT(intf.arp_numberlimit_enable, argv[i+1], para->p_rs);
            rc = _if_mgr_cmd_set_arp_numberlimit_enable(&intf, p_db_if, para);
            break;
            
        case TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_NUMBER:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs); 
            PM_CFG_GET_UINT(intf.arp_numberlimit_number, argv[i+1], para->p_rs);
            rc = _if_mgr_cmd_set_arp_numberlimit_number(&intf, p_db_if, para);
            break;

        case TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_VIOLATE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs); 
            PM_CFG_GET_UINT(intf.arp_numberlimit_violate, argv[i+1], para->p_rs);
            rc = _if_mgr_cmd_set_arp_numberlimit_violate(&intf, p_db_if, para);
            break;
        /* added by tongzb for bug 44368, support loopback on CNOS */
        //case TBL_INTERFACE_FLD_PHYPORT_LOOPBACK:
        case TBL_INTERFACE_FLD_PHY_LOOPBACK_MODE:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            if(GLB_FLAG_ISSET(p_db_if->phyport_flag, GLB_PHYPORT_FLAG_LOOPBACK))
            {
                if (!sal_strcmp("default", argv[i+1]))
                {
                    intf.phy_loopback_mode = GLB_LB_NONE;
                    intf.internal_lp_ifindex = 0;/* this is no used */
                }
                else if (!sal_strcmp("external", argv[i+1]))
                {
                    intf.phy_loopback_mode = GLB_LB_PHY_FAR;
                    intf.internal_lp_ifindex = 0;/* this is no used */
                }
                else if (!sal_strcmp("internal", argv[i+1]))
                {
                    if (argc >= 4)
                    {
                        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
                        tbl_interface_t* p_in_if = NULL;
                        tbl_interface_key_t if_key;
                        intf.phy_loopback_mode = GLB_LB_PHY_NEAR;
                        sal_memset(&if_key, 0, sizeof(if_key));
                        sal_strncpy(if_key.name, argv[i+2], IFNAME_SIZE);
                        p_in_if = tbl_interface_get_interface(&if_key);
                        if (NULL == p_in_if)
                        {
                            /* modified by tongzb for bug 45687, add the error info */
                            CFG_INVALID_PARAM_RET("Invalid destination interface name");
                            return PM_E_NOT_EXIST;
                        }
                        intf.internal_lp_ifindex = p_in_if->ifindex;
                    }
                    else
                    {
                        intf.phy_loopback_mode = GLB_LB_PHY_LOOPBACK;
                        intf.internal_lp_ifindex = 0;/* this is no used */
                    }
                    
                }
                else if (!sal_strcmp("port", argv[i+1]))
                {
                    intf.phy_loopback_mode = GLB_LB_PORT;
                    intf.internal_lp_ifindex = 0;/* this is no used */
                }
                else if (!sal_strcmp("port_swap", argv[i+1]))
                {
                    intf.phy_loopback_mode = GLB_LB_PORT_SWAP;
                    intf.internal_lp_ifindex = 0;/* this is no used */
                }
                rc = _if_mgr_cmd_set_loopback(&intf, p_db_if, para);
            }
            else
            {
                /* modified by tongzb for bug 53417, modified the error info about 530 board */
                if(GLB_CHIP_TSINGMA == tbl_chip_get_type())
                {
                    CFG_CONFLICT_RET("Not supported on this product");
                }
                /* modified by tongzb for bug 45552, modified the error info */
                CFG_INVALID_PARAM_RET("Interface %s  can't configure loopback", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
                return PM_E_NOT_SUPPORT;
            }
            break;
        /* added by tongzb for bug 44092, support config media type on CNOS */
        case TBL_INTERFACE_FLD_MEDIA:
            PM_CFG_CHECK_MIN_PARAMS(argc, 2, para->p_rs);
            if(GLB_FLAG_ISSET(p_db_if->phyport_flag, GLB_PHYPORT_FLAG_COMBO))
            {
                if (!sal_strcmp("auto", argv[i+1]))
                {
                    intf.media = GLB_PORT_TYPE_AUTO;
                }
                else if (!sal_strcmp("rj45", argv[i+1]))
                {
                    intf.media = GLB_PORT_TYPE_COPPER;
                }
                else if (!sal_strcmp("sfp", argv[i+1]))
                {
                    intf.media = GLB_PORT_TYPE_FIBER;
                }
                rc = _if_mgr_cmd_set_media(&intf, p_db_if, para);
                /*when change port media type, all speed and duplex should be reset*/
                if(p_db_if->speed != GLB_SPEED_AUTO)
                {
                    intf.speed = GLB_SPEED_AUTO;
                    rc = _if_mgr_cmd_set_speed(&intf, p_db_if, para);
                }                    
                if(p_db_if->duplex != GLB_DUPLEX_AUTO)
                {
                    intf.duplex = GLB_DUPLEX_AUTO;
                    rc = _if_mgr_cmd_set_duplex(&intf, p_db_if, para);
                }
            }
            else
            {
                /* modified by tongzb for bug 50607, modified the error info */
                CFG_INVALID_PARAM_RET("Port not support for changing media-type");
                return PM_E_NOT_SUPPORT;
            }
            break;
        case TBL_INTERFACE_FLD_LATENCY_EVENT_ENABLE:
            IF_MGR_CHECK_OPENFLOW_PORT(p_db_if);
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            intf.latency_event_enable = value;
            rc = _if_mgr_cmd_set_latency_event_enable(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_LATENCY_EVENT_LOW:
            IF_MGR_CHECK_OPENFLOW_PORT(p_db_if);
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 5, para->p_rs);
            PM_CFG_GET_UINT(low_thrd, argv[2], para->p_rs);
            PM_CFG_GET_UINT(high_thrd, argv[4], para->p_rs);
            intf.latency_event_high = high_thrd;
            intf.latency_event_low = low_thrd;
            rc = _if_mgr_cmd_set_latency_event_threshold(&intf, p_db_if, para);
            break;

        case TBL_INTERFACE_FLD_REFLECTIVE_BRIDGE_EN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            if((p_db_if->hw_type != GLB_IF_TYPE_PORT_IF) && (p_db_if->hw_type != GLB_IF_TYPE_LINKAGG_IF))
            {
                CFG_INVALID_PARAM_RET("This cli only can be used in phy port or agg port");
            }
            if (!sal_strcmp("enable", argv[i+1]))
            {
                intf.reflective_bridge_en = 1;
            }
            else
            {
                intf.reflective_bridge_en = 0;
            }
            rc = _if_mgr_cmd_set_reflective(&intf, p_db_if, para);
            break;
            
        case TBL_INTERFACE_FLD_L2PROTOCOL:
            /* ds process */
            return _if_mgr_cmd_process_interface_set_l2protocol(argv, argc, para, p_db_if);
            break;
        case TBL_INTERFACE_FLD_L2PRO_UPLINK:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            intf.l2pro_uplink = value;
            rc = _if_mgr_cmd_set_l2protocol_uplink(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_IFG_STATS_EN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            intf.ifg_stats_en = (0 == sal_strcmp(argv[2], "enable")) ? 1 : 0;
            rc = _if_mgr_cmd_set_flow_statistics(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 3, 4, para->p_rs);
            if (argc == 4)
            {
                sal_memset(&hash_value_pro_key, 0, sizeof(hash_value_pro_key));
                sal_strncpy(hash_value_pro_key.name, argv[2], HASH_VALUE_PROFILE_NAME_SIZE);
                p_db_hash_value_pro = tbl_hash_value_profile_get_hash_value_profile(&hash_value_pro_key);
                if (NULL == p_db_hash_value_pro)
                {
                    CFG_INVALID_PARAM_RET("Hash value %s does not exist", argv[2]);
                }
                hash_value_pro_dir = ((0 == sal_strcmp(argv[3], "input")) ? GLB_INGRESS : GLB_EGRESS);
                intf.hash_value_profile_dir = hash_value_pro_dir;
                intf.hash_value_profile_id = p_db_hash_value_pro->index;
            }
            else
            {
                hash_value_pro_dir = ((0 == sal_strcmp(argv[2], "input")) ? GLB_INGRESS : GLB_EGRESS);
                intf.hash_value_profile_dir = hash_value_pro_dir;
                intf.hash_value_profile_id = 0;
            }
            rc = _if_mgr_cmd_set_hash_value_profile_select(&intf, p_db_if, para);
            break;
        case TBL_INTERFACE_FLD_POE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            /* added by tongzb for bug 44092, support config media type on CNOS */
            if(!GLB_FLAG_ISSET(p_db_if->phyport_flag, GLB_PHYPORT_FLAG_POE))
            {
                CFG_CONFLICT_RET("Interface unsupport poe");
                return PM_E_NOT_SUPPORT;
            }
            /* init intf */
            intf.poe.enable = p_db_if->poe.enable;
            intf.poe.force_power = p_db_if->poe.force_power;
            intf.poe.mode = p_db_if->poe.mode;
            intf.poe.priority = p_db_if->poe.priority;
            intf.poe.power_limit = p_db_if->poe.power_limit;          
            if (!sal_strcmp("enable", argv[i+1]))
            {
                PM_CFG_GET_BOOL(value, argv[i+2], para->p_rs);
                intf.poe.enable = value;
            }
            else if (!sal_strcmp("force_power", argv[i+1]))
            {
                PM_CFG_GET_BOOL(value, argv[i+2], para->p_rs);
                intf.poe.force_power = value;
            }
            else if (!sal_strcmp("mode", argv[i+1]))
            {
                if (!sal_strcmp("af", argv[i+2]))
                {
                    intf.poe.mode = POE_AF;
                }
                else if (!sal_strcmp("at", argv[i+2]))
                {
                    intf.poe.mode = POE_AT;
                }
                else if (!sal_strcmp("poh", argv[i+2]))
                {
                    intf.poe.mode = POE_POH;
                }
            }
            else if (!sal_strcmp("priority", argv[i+1]))
            {
                PM_CFG_GET_UINT(value, argv[i+2], para->p_rs);
                intf.poe.priority = value;
            }
            else if (!sal_strcmp("max", argv[i+1]))
            {
                PM_CFG_GET_UINT(value, argv[i+2], para->p_rs);
                intf.poe.power_limit = value;
            }            
            else
            {
                CFG_INVALID_PARAM_RET("Unsupport changed  poe configure about %s", argv[i+1]);
                break;
            }
            rc = _if_mgr_cmd_set_poe(&intf, p_db_if, para);
            break;            
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        if (0 == argc)
        {
            ifname = NULL;
        }
        else if (1 == argc)
        {
            if (!sal_strcmp(argv[0], "status"))
            {
                ifname = NULL;
            }
            else if (!sal_strcmp(argv[0], "sflow"))
            {
                show_type = IF_MGR_SHOW_PHY_LAG;
                ifname = NULL;
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "switchport"))
            {
                show_type = IF_MGR_SHOW_PHY_LAG;
                ifname = NULL;
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "summary"))
            {
                show_type = IF_MGR_SHOW_PHY_LAG;
                ifname = NULL;
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "trunk"))
            {
                show_type = IF_MGR_SHOW_PHY_LAG;
                ifname = NULL;
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "port_isolate"))
            {
                show_type = IF_MGR_SHOW_PHY_LAG;
                ifname = NULL;
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "private_vlan"))
            {
                show_type = IF_MGR_SHOW_PHY;
                ifname = NULL;
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "flowcontrol"))
            {
                show_type = IF_MGR_SHOW_PHY;
                ifname = NULL;
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "storm_control"))
            {
                show_type = IF_MGR_SHOW_PHY;
                ifname = NULL;
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "aclqos_if"))
            {
                if (!tbl_qos_global_get_qos_enable())
                {
                    break;
                }
                ifname = NULL;            
                parse_field = FALSE;
                show_type = IF_MGR_SHOW_ALL;
            }
            else if (!sal_strcmp(argv[0], "key"))
            {
                show_type = IF_MGR_SHOW_ALL;
                ifname = NULL;
                parse_field = FALSE;
                only_key = TRUE;
            }
            else if (!sal_strcmp(argv[0], "description"))
            {
                show_type = IF_MGR_SHOW_ALL;
                ifname = NULL;
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "loopback"))
            {
                show_type = IF_MGR_SHOW_ALL;
                ifname = NULL;
                parse_field = FALSE;
            }
            else
            {
                ifname = argv[0];
            }
        }
        else if (2 <= argc)
        {
            ifname = argv[0];
            if (!sal_strcmp(argv[1], "switchport"))
            {
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[1], "summary"))
            {
                if (0 == sal_strlen(ifname))
                {
                    ifname = NULL;
                }
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "lagmember"))
            {
                ifname = argv[1];            
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "lag"))
            {
                show_type = IF_MGR_SHOW_LAG;
            }
            else if (!sal_strcmp(argv[0], "lacp"))
            {
                show_type = IF_MGR_SHOW_LACP;
            }
            else if (!sal_strcmp(argv[0], "port_isolate"))
            {
                show_type = IF_MGR_SHOW_PHY_LAG;
                ifname = NULL;
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[1], "private_vlan"))
            {
               parse_field = FALSE;
            }
            
            else if (!sal_strcmp(argv[0], "flowcontrol"))
            {
                show_type = IF_MGR_SHOW_PHY;
                ifname = argv[1];
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "storm_control"))
            {
                show_type = IF_MGR_SHOW_PHY;
                ifname = argv[1];
                parse_field = FALSE;
            }  
            else if (!sal_strcmp(argv[0], "aclqos_if"))
            {
                if (!tbl_qos_global_get_qos_enable())
                {
                    break;
                }
                ifname = argv[1];            
                parse_field = FALSE;
                show_type = IF_MGR_SHOW_ALL;
            }
            else if (!sal_strcmp(argv[0], "aclqos_if_queue"))
            {
                if (!tbl_qos_global_get_qos_enable())
                {
                    break;
                }
                ifname = argv[1];            
                parse_field = FALSE;
                show_type = IF_MGR_SHOW_ALL;
            }            
            else if (!sal_strcmp(argv[0], "aclqos_if_queue_drop"))
            {
                //if (!tbl_qos_global_get_qos_enable())
               // {
                //    break;
               // }
                //ifname = argv[1];
                //qos_mgr_cmd_get_port_queue_drop(ifname, para);
                break;
            }            
            else if (!sal_strcmp(argv[0], "aclqos_if_queue_shape"))
            {
                if (!tbl_qos_global_get_qos_enable())
                {
                    break;
                }
                ifname = argv[1];
                qos_mgr_cmd_get_port_queue_shape(ifname, para);
                break;
            }            
            else if (!sal_strcmp(argv[0], "aclqos_if_pmap_stats"))
            {
                ifname = argv[1];
                parse_field = FALSE;
//                acl_mgr_cmd_pmap_if_get_pmap_stats(ifname, para, argv[2], argv[3], argv[4],
//                    (argc == 6) ? argv[5] : NULL);
                break;
            }
            else if (!sal_strcmp(argv[0], "aclqos_if_policer_stats"))
            {
                break;
            }
            else if (!sal_strcmp(argv[0], "port_security_max_mac"))
            {
                show_type = IF_MGR_SHOW_PHY_LAG;
                ifname = argv[1];
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "aclqos_if_queue_stats"))
            {
                break;
            }
            else if (!sal_strcmp(argv[0], "description"))
            {
                show_type = IF_MGR_SHOW_ALL;
                ifname = argv[1];
                parse_field = FALSE;
            }
            
            if (IF_MGR_SHOW_LAG == show_type || IF_MGR_SHOW_LACP == show_type)
            {
                if (sal_strlen(argv[1]))
                {
                    sal_memset(agg_ifname, 0, sizeof(agg_ifname));
                    PM_CFG_GET_UINT(value, argv[1], para->p_rs);
                    sal_snprintf(agg_ifname, IFNAME_SIZE, GLB_IFNAME_AGG_PREFIX"%d", value);
                    ifname = agg_ifname;
                }
                else
                {
                    ifname = NULL;
                }
                parse_field = FALSE;
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }

        if (parse_field)
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }
        else
        {
            if (only_key)
            {
                field.field_id[0] = TBL_INTERFACE_FLD_KEY;
            }
            else
            {
                sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            }
        }
        
        rc = _if_mgr_cmd_get_if(ifname, para, p_node, &field, show_type);
        break;
/*Added by zhush for bug 52768, 2019-07-06*/
    case CDB_OPER_GETJSON:  
        if (1 == argc && 0 ==sal_strcmp(argv[0], "summary"))
        { 
             _if_mgr_cmd_process_interface_show_summary_json(argv, p_db_if, para, TRUE);	 
        }
	else if (2 == argc && 0 ==sal_strcmp(argv[1], "summary"))
	{
		  sal_memset(&intf, 0, sizeof(intf));
                sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);

                p_db_if = tbl_interface_get_interface(&intf.key);
                if (NULL == p_db_if)
                {
                        IFNAME_ETH2FULL(argv[0], ifname_ext);
                        CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
                 }
		  _if_mgr_cmd_process_interface_show_summary_json(argv, p_db_if, para, FALSE);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");    
        }
        break;
/*end zhush*/
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
if_mgr_cmd_get_aclqos_if(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_aclqos_if_t* p_aclqos_if = NULL;
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    tbl_iter_args_t args;
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    ctclib_slistnode_t *listnode = NULL;

    tbl_aclqos_if_master_t *p_master = tbl_aclqos_if_get_master();

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_node = cdb_get_tbl(TBL_ACLQOS_IF);
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[1] = fp;
    /*dump all aclqos_if*/
    if(argc == 0)
    {
        rc = cfg_cmd_parse_field(argv, 0, p_node, &field, FALSE);
        if (rc < 0)
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        args.argv[0] = &field;
        CTCLIB_SLIST_LOOP(p_master->if_list, p_aclqos_if, listnode)
        {
            tbl_aclqos_if_dump_one(p_aclqos_if, &args);
        }
        sal_fclose(fp);
        return PM_E_NONE;
    }
    p_aclqos_if = tbl_aclqos_if_get_aclqos_if_by_name(argv[1]);
    if(!p_aclqos_if)
    {
        sal_fclose(fp);
        CFG_INVALID_PARAM_RET("Interface %s doesn't exist.", argv[1]);
    }

    if((1 == argc) || (2 == argc))
    {
        rc = cfg_cmd_parse_field(argv, 0, p_node, &field, FALSE);
        if (rc < 0)
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        args.argv[0] = &field;
        tbl_aclqos_if_dump_one(p_aclqos_if, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
if_mgr_cmd_add_aclqos_if(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_aclqos_if_t aclqos_if;
    tbl_aclqos_if_t* p_aclqos_if;
    tbl_interface_t * p_db_if;

    if (0 == argc)
    {
        return PM_E_NONE;
    }
    
    sal_memset(&aclqos_if, 0, sizeof(tbl_aclqos_if_t));
   // sal_memset(&fea_port_policer_apply,0x0, sizeof(tbl_fea_port_policer_apply_t));
    p_aclqos_if = tbl_aclqos_if_get_aclqos_if_by_name(argv[0]);
    p_db_if = tbl_interface_get_interface_by_name(argv[0]);
    if(!p_aclqos_if && p_db_if)
    {
        //sal_memcpy(aclqos_if.key.name,argv[0], (sizeof(char))*IFNAME_SIZE);
        sal_strcpy(aclqos_if.key.name,argv[0]);
        aclqos_if.ifindex = p_db_if->ifindex;
        aclqos_if.port_type = GLB_IF_TYPE_PORT_IF;
        tbl_aclqos_if_add_aclqos_if(&aclqos_if);
    }
    return PM_E_NONE;
}

int32
if_mgr_cmd_set_aclqos_if(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_aclqos_if_t aclqos_if;
    tbl_aclqos_if_t* p_aclqos_if = NULL;
    int32 field_id = 0;
    int32 queue_id = 0;
    uint32 domain_id = 0;
    cdb_field_t *p_field = NULL;
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;
    uint32 flags_field_id = 0;
    tbl_qos_policer_profile_t* p_qos_policer_profile = NULL;
    tbl_qos_drop_profile_t *  p_qos_drop_profile = NULL;
    tbl_qos_queue_shape_profile_t *  p_qos_queue_shape_profile = NULL;
    tbl_qos_port_shape_profile_t *  p_qos_port_shape_profile = NULL;
    tbl_qos_queue_smart_buffer_t* p_db_queue_smart_buffer = NULL;
    tbl_qos_queue_smart_buffer_key_t queue_smart_buffer_key;
    tbl_interface_t intf;
    tbl_interface_t *p_db_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    sal_memset(&intf, 0, sizeof(intf));
    sal_strncpy(intf.key.name, argv[1], IFNAME_SIZE);
    uint32 dscp_replace = 0;
    uint32 cos_replace = 0;
    uint32 argv_number = 0;
    uint32 trust_type = 0;
    uint32 port_tc = 0;
    bool flag = FALSE;
    bool is_port_if = FALSE;
    bool is_vlan_if = FALSE;
    uint32 policer_res_cnt = 0;
    

#ifdef USW
    flag = TRUE;
#else
    flag = FALSE;
#endif

    /* check exist */
    p_db_if = tbl_interface_get_interface(&intf.key);
    if (!p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s not found", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
    }

    if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        is_port_if = TRUE;
    }
    if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
    {
        is_vlan_if = TRUE;
    }
    
    p_node = cdb_get_tbl(TBL_ACLQOS_IF);
    field_id = cdb_get_field_id(argv[0], p_node, &p_field);   

    /* BEGIN: check profile name exist, add by wangqj for bug 39533 */
    switch(field_id) 
    {
        case TBL_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE:
            argv_number = 5;
            break;
        case TBL_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE:
            argv_number = 5;
            break;
        case TBL_ACLQOS_IF_FLD_PORT_SHAPE_PROFILE:
            argv_number = 4;
            break;
        case TBL_ACLQOS_IF_FLD_INPUT_POLICER:
            argv_number = 4;
            break;
        case TBL_ACLQOS_IF_FLD_OUTPUT_POLICER:
            argv_number = 4;
            break;
    }

    if (flag)
    {
        if (TBL_ACLQOS_IF_FLD_DSCP_DOMAIN == field_id || TBL_ACLQOS_IF_FLD_TRUST == field_id)
        {
            if (TBL_ACLQOS_IF_FLD_TRUST == field_id)
            {
                if (!is_port_if && !is_vlan_if)
                {
                    trust_type = _if_mgr_aclqos_if_map_trust(argv[2]);
                    if (GLB_QOS_TRUST_DSCP == trust_type || GLB_QOS_TRUST_MAX == trust_type)
                    {
                        CFG_CONFLICT_RET("Interface %s is not physical port or vlan interface", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
                    }
                    else
                    {
                        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
                    }
                }
            }
            else if (TBL_ACLQOS_IF_FLD_DSCP_DOMAIN == field_id)
            {
                if (!is_port_if && !is_vlan_if)
                {
                    CFG_CONFLICT_RET("Interface %s is not physical port or vlan interface", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
                }
            }
            else
            {
                if (!is_port_if && !is_vlan_if)
                {
                    CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(intf.key.name, ifname_ext)); 
                }
            }
        }
        else
        {
            if (!is_port_if)
            {
                CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
            }
        }
    }
    else
    {
        if (!is_port_if)
        {
            CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
        }
    }

    sal_memset(&aclqos_if, 0, sizeof(tbl_aclqos_if_t));
    p_aclqos_if = tbl_aclqos_if_get_aclqos_if_by_name(argv[1]);
    if(!p_aclqos_if)
    {
        return PM_E_NOT_EXIST;
    }
    
    if(argc < argv_number) 
    {
        CFG_INVALID_PARAM_RET("please specific a valid profile name");
    }
    /* END: check profile name exist */

    switch(field_id)
    {
        case TBL_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE:
            queue_id = sal_atoi(argv[3]);
            sal_memset(&queue_smart_buffer_key, 0, sizeof(queue_smart_buffer_key));
            queue_smart_buffer_key.queue_id = queue_id;
            p_db_queue_smart_buffer = tbl_qos_queue_smart_buffer_get_qos_queue_smart_buffer(&queue_smart_buffer_key);
            if (p_db_queue_smart_buffer)
            {
                if (p_db_queue_smart_buffer->smart_buffer_enable)
                {
                    CFG_INVALID_PARAM_RET("The queue drop mode is dynamic, can't set tail-drop or random drop!");
                }
            }
            if('\0' != argv[4][0])
            {
                if(sal_strcmp(p_aclqos_if->queue_drop_profile[queue_id], GLB_QOS_DEF_WTD_PROFILE) && sal_strcmp(p_aclqos_if->queue_drop_profile[queue_id], GLB_QOS_DEF_WRED_PROFILE))
                {
                    CFG_INVALID_PARAM_RET("The queue had been applied drop profile! Please delete it first!");
                }
                p_qos_drop_profile = tbl_qos_drop_profile_get_profile_by_name(argv[4]);
                if(!p_qos_drop_profile)
                {
                    CFG_INVALID_PARAM_RET("The drop profile is not exist!");
                }
                p_qos_drop_profile->ref_cnt ++;
                PM_E_RETURN(tbl_qos_drop_profile_set_qos_drop_profile_field(p_qos_drop_profile, TBL_QOS_DROP_PROFILE_FLD_REF_CNT));
                sal_strcpy(p_aclqos_if->queue_drop_profile[queue_id], argv[4]);
            }
            else
            {
                if(!sal_strcmp(p_aclqos_if->queue_drop_profile[queue_id], GLB_QOS_DEF_WTD_PROFILE))
                {
                    return PM_E_NONE;
                }
                p_qos_drop_profile = tbl_qos_drop_profile_get_profile_by_name(p_aclqos_if->queue_drop_profile[queue_id]);
                if(!p_qos_drop_profile)
                {
                    return PM_E_NOT_EXIST;
                }
                p_qos_drop_profile->ref_cnt --;
                PM_E_RETURN(tbl_qos_drop_profile_set_qos_drop_profile_field(p_qos_drop_profile, TBL_QOS_DROP_PROFILE_FLD_REF_CNT));
                sal_strcpy(p_aclqos_if->queue_drop_profile[queue_id], GLB_QOS_DEF_WTD_PROFILE);
            }
            break;
            
        case TBL_ACLQOS_IF_FLD_QUEUE_DROP_MODE:
            queue_id = sal_atoi(argv[3]);
            sal_memset(&queue_smart_buffer_key, 0, sizeof(queue_smart_buffer_key));
            queue_smart_buffer_key.queue_id = queue_id;
            p_db_queue_smart_buffer = tbl_qos_queue_smart_buffer_get_qos_queue_smart_buffer(&queue_smart_buffer_key);
            if (p_db_queue_smart_buffer)
            {
                if (p_db_queue_smart_buffer->smart_buffer_enable)
                {
                    CFG_INVALID_PARAM_RET("The queue drop mode is dynamic, can't set tail-drop or random drop!");
                }
            }
            if(1 == sal_atoi(argv[4]))
            {
                p_aclqos_if->queue_drop_mode[queue_id] = GLB_QOS_QUEUE_DROP_MODE_WRED;
                #ifdef GREATBELT
                if(!sal_strcmp(p_aclqos_if->queue_drop_profile[queue_id], GLB_QOS_DEF_WTD_PROFILE))
                {
                    sal_strcpy(p_aclqos_if->queue_drop_profile[queue_id], GLB_QOS_DEF_WRED_PROFILE);
                    tbl_aclqos_if_set_aclqos_if_field(p_aclqos_if, TBL_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE);
                }
                #endif
                
            }
            else
            {
                p_aclqos_if->queue_drop_mode[queue_id] = GLB_QOS_QUEUE_DROP_MODE_WTD;
                #ifdef GREATBELT
                if(!sal_strcmp(p_aclqos_if->queue_drop_profile[queue_id], GLB_QOS_DEF_WRED_PROFILE))
                {
                    sal_strcpy(p_aclqos_if->queue_drop_profile[queue_id], GLB_QOS_DEF_WTD_PROFILE);
                    tbl_aclqos_if_set_aclqos_if_field(p_aclqos_if, TBL_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE);
                }
                #endif
            }
            break;

        case TBL_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE:
            queue_id = sal_atoi(argv[3]);
            if('\0' != argv[4][0])
            {
                if(sal_strcmp(p_aclqos_if->queue_shape_profile[queue_id], GLB_QOS_DEF_SCHEDULER_PROFILE))
                {
                    CFG_INVALID_PARAM_RET("The queue had been applied scheduler profile! Please delete it first!");
                }
                p_qos_queue_shape_profile = tbl_qos_queue_shape_profile_get_profile_by_name(argv[4]);
                if(!p_qos_queue_shape_profile)
                {
                    CFG_INVALID_PARAM_RET("The scheduler profile is not exist!");
                }
                p_qos_queue_shape_profile->ref_cnt ++;
                PM_E_RETURN(tbl_qos_queue_shape_profile_set_qos_queue_shape_profile_field(p_qos_queue_shape_profile, TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_REF_CNT));
                sal_strcpy(p_aclqos_if->queue_shape_profile[queue_id], argv[4]);
            }
            else
            {
                if(!sal_strcmp(p_aclqos_if->queue_shape_profile[queue_id], GLB_QOS_DEF_SCHEDULER_PROFILE))
                {
                    return PM_E_NONE;
                }
                p_qos_queue_shape_profile = tbl_qos_queue_shape_profile_get_profile_by_name(p_aclqos_if->queue_shape_profile[queue_id]);
                if(!p_qos_queue_shape_profile)
                {
                    return PM_E_NOT_EXIST;
                }
                p_qos_queue_shape_profile->ref_cnt --;
                PM_E_RETURN(tbl_qos_queue_shape_profile_set_qos_queue_shape_profile_field(p_qos_queue_shape_profile, TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_REF_CNT));
                sal_strcpy(p_aclqos_if->queue_shape_profile[queue_id], GLB_QOS_DEF_SCHEDULER_PROFILE);
            }
            break;
        case TBL_ACLQOS_IF_FLD_DOMAIN:
            domain_id = sal_atoi(argv[3]);
            p_aclqos_if->domain = domain_id;
            flags_field_id = TBL_ACLQOS_IF_FLD_FLAGS_DOMAIN;
            if(domain_id)
                GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_DOMAIN);
            else
                GLB_UNSET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_DOMAIN);
            break;
        case TBL_ACLQOS_IF_FLD_COS_DOMAIN:
            domain_id = sal_atoi(argv[3]);
            p_aclqos_if->cos_domain = domain_id;
            break;
         case TBL_ACLQOS_IF_FLD_DSCP_DOMAIN:
            domain_id = sal_atoi(argv[3]);
            p_aclqos_if->dscp_domain = domain_id;
            break;
        case TBL_ACLQOS_IF_FLD_PORT_SHAPE_PROFILE:
            if('\0' != argv[3][0])
            {
                if(sal_strcmp(p_aclqos_if->port_shape_profile, GLB_QOS_DEF_PORT_SHAPE_PROFILE))
                {
                    CFG_INVALID_PARAM_RET("The port had been applied shape profile! Please delete it first!");
                }
                p_qos_port_shape_profile = tbl_qos_port_shape_profile_get_profile_by_name(argv[3]);
                if(!p_qos_port_shape_profile)
                {
                    CFG_INVALID_PARAM_RET("The scheduler profile is not exist!");
                }
                p_qos_port_shape_profile->ref_cnt ++;
                PM_E_RETURN(tbl_qos_port_shape_profile_set_qos_port_shape_profile_field(p_qos_port_shape_profile, TBL_QOS_PORT_SHAPE_PROFILE_FLD_REF_CNT));
                sal_strcpy(p_aclqos_if->port_shape_profile, argv[3]);
                GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_PORT_SHAPE_PROFILE);
            }
            else
            {
                if(!sal_strcmp(p_aclqos_if->port_shape_profile, GLB_QOS_DEF_PORT_SHAPE_PROFILE))
                {
                    return PM_E_NONE;
                }
                p_qos_port_shape_profile = tbl_qos_port_shape_profile_get_profile_by_name(p_aclqos_if->port_shape_profile);
                if(!p_qos_port_shape_profile)
                {
                    return PM_E_NOT_EXIST;
                }
                p_qos_port_shape_profile->ref_cnt --;
                PM_E_RETURN(tbl_qos_port_shape_profile_set_qos_port_shape_profile_field(p_qos_port_shape_profile, TBL_QOS_PORT_SHAPE_PROFILE_FLD_REF_CNT));
                sal_strcpy(p_aclqos_if->port_shape_profile, GLB_QOS_DEF_PORT_SHAPE_PROFILE);
                GLB_UNSET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_PORT_SHAPE_PROFILE);
            }
            flags_field_id = TBL_ACLQOS_IF_FLD_FLAGS_PORT_SHAPE_PROFILE;
            break;
        case TBL_ACLQOS_IF_FLD_PORT_TC:
            port_tc = sal_atoi(argv[3]);
            p_aclqos_if->port_tc= port_tc;
            flags_field_id = TBL_ACLQOS_IF_FLD_FLAGS_PORT_TC;
            GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_PORT_TC);
            break;
        case TBL_ACLQOS_IF_FLD_TRUST:
            trust_type = _if_mgr_aclqos_if_map_trust(argv[2]);
            if (flag)
            {
                //vlan if just support configurate trust dscp and no trust
                if (is_vlan_if) 
                {
                    if (GLB_QOS_TRUST_MAX !=trust_type && GLB_QOS_TRUST_DSCP != trust_type)
                    {
                        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
                    }
                }
                if (GLB_QOS_TRUST_MAX ==trust_type)
                {
                    //default trust cos
                    trust_type = GLB_QOS_TRUST_COS;
                }
            }
            p_aclqos_if->trust = trust_type;
            flags_field_id = TBL_ACLQOS_IF_FLD_FLAGS_TRUST;
            GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_TRUST);
            break;
        case TBL_ACLQOS_IF_FLD_REPLACE_DSCP:
            dscp_replace = sal_atoi(argv[3]);
            if(p_aclqos_if->replace_dscp == dscp_replace)
            {
                if(dscp_replace)
                {
                    CFG_INVALID_PARAM_RET("Dscp replace had been enabled on this interface!");
                }
                else
                {
                    CFG_INVALID_PARAM_RET("Not configured replace dscp on this interface!");
                }
            }
            p_aclqos_if->replace_dscp = dscp_replace;
            flags_field_id = TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_DSCP;
            GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_REPLACE_DSCP);
            break;
        case TBL_ACLQOS_IF_FLD_REPLACE_COS:
            cos_replace = sal_atoi(argv[3]);
            if(p_aclqos_if->replace_cos == cos_replace)
            {
                if(cos_replace)
                {
                    CFG_INVALID_PARAM_RET("Cos replace had been enabled on this interface!");
                }
                else 
                {
                    CFG_INVALID_PARAM_RET("Not configured replace cos on this interface!");
                }
            }
            p_aclqos_if->replace_cos = cos_replace;
            flags_field_id = TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_COS;
            GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_REPLACE_COS);
            break;
        case TBL_ACLQOS_IF_FLD_INPUT_POLICER:
            if('\0' != argv[3][0])
            {
                if(GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_INPUT_POLICER))   
                {
                        CFG_INVALID_PARAM_RET("The port had been applied ingress policer! Please delete it first!");
                }
                p_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(argv[3]);
                if(!p_qos_policer_profile)
                {
                    CFG_INVALID_PARAM_RET("Policer profile is not exist!");
                }

                /* check policer profile resource */
                if (qos_mgr_policer_res_check(argv[3], TRUE, TRUE, &policer_res_cnt))
                {
                    CFG_CONFLICT_RET("QoS ingress port policer profile resource full!");
                }
                /* add policer profile resource */
                PM_E_RETURN(qos_mgr_policer_res_add(argv[3], TRUE, TRUE));
                
                p_qos_policer_profile->ref_cnt ++;
                PM_E_RETURN(tbl_qos_policer_profile_set_qos_policer_profile_field(p_qos_policer_profile, TBL_QOS_POLICER_PROFILE_FLD_REF_CNT));
                sal_strcpy(p_aclqos_if->input_policer, argv[3]);
                GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_INPUT_POLICER);
            }
            else
            {
                if(!GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_INPUT_POLICER))   
                {
                    CFG_INVALID_PARAM_RET("None ingress policer applied on this interface!");
                }

                p_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(p_aclqos_if->input_policer);
                if(!p_qos_policer_profile)
                {
                    CFG_INVALID_PARAM_RET("Policer profile is not exist!");
                }

                /* del policer profile resource */
                PM_E_RETURN(qos_mgr_policer_res_del(p_aclqos_if->input_policer, TRUE, TRUE));
                
                p_qos_policer_profile->ref_cnt --;
                PM_E_RETURN(tbl_qos_policer_profile_set_qos_policer_profile_field(p_qos_policer_profile, TBL_QOS_POLICER_PROFILE_FLD_REF_CNT));
                sal_memset(p_aclqos_if->input_policer, 0x0, POLICER_NAME_SIZE+1);
                GLB_UNSET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_INPUT_POLICER);
            }
            flags_field_id = TBL_ACLQOS_IF_FLD_FLAGS_INPUT_POLICER;
            break;
        case TBL_ACLQOS_IF_FLD_OUTPUT_POLICER:
            if('\0' != argv[3][0])
            {
                if(GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_OUTPUT_POLICER))   
                {
                        CFG_INVALID_PARAM_RET("The port had been applied egress policer! Please delete it first!");
                }
                p_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(argv[3]);
                if(!p_qos_policer_profile)
                {
                    CFG_INVALID_PARAM_RET("Policer profile is not exist!");
                }

                /* check policer profile resource */
                if (qos_mgr_policer_res_check(argv[3], FALSE, TRUE, &policer_res_cnt))
                {
                    CFG_CONFLICT_RET("QoS egress port policer profile resource full!");
                }
                /* add policer profile resource */
                PM_E_RETURN(qos_mgr_policer_res_add(argv[3], FALSE, TRUE));
                
                p_qos_policer_profile->ref_cnt ++;
                PM_E_RETURN(tbl_qos_policer_profile_set_qos_policer_profile_field(p_qos_policer_profile, TBL_QOS_POLICER_PROFILE_FLD_REF_CNT));
                sal_strcpy(p_aclqos_if->output_policer, argv[3]);
                GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_OUTPUT_POLICER);
            }
            else
            {
                if(!GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_OUTPUT_POLICER))   
                {
                    CFG_INVALID_PARAM_RET("None egress policer applied on thhis interface!");
                }
                p_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(p_aclqos_if->output_policer);
                if(!p_qos_policer_profile)
                {
                    CFG_INVALID_PARAM_RET("Policer profile is not exist!");
                }

                /* del policer profile resource */
                PM_E_RETURN(qos_mgr_policer_res_del(p_aclqos_if->output_policer, FALSE, TRUE));
                
                p_qos_policer_profile->ref_cnt --;
                PM_E_RETURN(tbl_qos_policer_profile_set_qos_policer_profile_field(p_qos_policer_profile, TBL_QOS_POLICER_PROFILE_FLD_REF_CNT));
                sal_memset(p_aclqos_if->output_policer, 0x0, POLICER_NAME_SIZE+1);
                GLB_UNSET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_OUTPUT_POLICER);
            }
            flags_field_id = TBL_ACLQOS_IF_FLD_FLAGS_OUTPUT_POLICER;
            break;
    }
    
    PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_aclqos_if, field_id));
    if (flags_field_id)
    {
        PM_E_RETURN(tbl_aclqos_if_set_aclqos_if_field(p_aclqos_if, flags_field_id));
    }

    return rc;
}

int32
if_mgr_cmd_process_aclqos_if(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (para->oper)
    {

        case CDB_OPER_ADD:
            PM_E_RETURN(if_mgr_cmd_add_aclqos_if(argv, argc, para));
            break;
      #if 0
        case CDB_OPER_DEL:
            break;
     #endif
        case CDB_OPER_SET:
            if (!tbl_qos_global_get_qos_enable())             
            {
                CFG_CONFLICT_RET("QoS state is disabled"); 
                return PM_E_NONE; 
            }  
            PM_E_RETURN(if_mgr_cmd_set_aclqos_if(argv, argc, para));
            break;
        case CDB_OPER_GET:
            if (!tbl_qos_global_get_qos_enable())             
            {
                if (para->oper != CDB_OPER_GET)
                {
                    CFG_CONFLICT_RET("QoS state is disabled"); 
                } 
                return PM_E_NONE; 
            }
            if (argc > 0)
            {
                if (!sal_strcmp(argv[0], "queue_drop_profile"))
                {
                    PM_E_RETURN(qos_mgr_cmd_get_port_queue_drop(argv[1], para));
                }
                else if(!sal_strcmp(argv[0], "queue_shape_profile"))
                {
                    PM_E_RETURN(qos_mgr_cmd_get_port_queue_shape(argv[1], para));
                }
                else if(!sal_strcmp(argv[0], "port_shape_profile"))
                {
                    PM_E_RETURN(qos_mgr_cmd_get_port_shape(argv[1], para));
                }
                else if(!sal_strcmp(argv[0], "classification_remark"))
                {
                    PM_E_RETURN(if_mgr_cmd_get_aclqos_if(argv, argc, para));
                }
            }
            else
            {
               PM_E_RETURN(if_mgr_cmd_get_aclqos_if(argv, argc, para));
            }
            break;
        default:
            
            break;
    }
    return PM_E_NONE;
}

int32
_if_mgr_cmd_set_input_rate_threshold(tbl_port_stats_rate_t *p_db_stats_rate,
    uint8 threshold, uint8 resumerate, cfg_cmd_para_t *para)
{
    int32  rc = PM_E_NONE;

    if (threshold < resumerate)
    {
        CFG_CONFLICT_RET("Resume-rate can't be larger than input-rate");
    }

    if (p_db_stats_rate->input_rate_threshold == threshold
       && p_db_stats_rate->input_rate_resume == resumerate)
    {
        return rc;
    }

    p_db_stats_rate->input_rate_threshold = threshold;
    p_db_stats_rate->input_rate_resume = resumerate;
    PM_E_RETURN(tbl_port_stats_rate_set_port_stats_rate_field(p_db_stats_rate,
                TBL_PORT_STATS_RATE_FLD_INPUT_RATE_THRESHOLD));
    PM_E_RETURN(tbl_port_stats_rate_set_port_stats_rate_field(p_db_stats_rate,
                TBL_PORT_STATS_RATE_FLD_INPUT_RATE_RESUME));

    if (0 == threshold && 0 == resumerate)
    {
        p_db_stats_rate->input_rate_state = LOG_THRESHOLD_DISABLE;
    }
    else
    {
        p_db_stats_rate->input_rate_state = LOG_THRESHOLD_ENABLE;
        if (threshold - resumerate < 10)
        {
            CFG_PROMPT_RET("Notice: The gap of input-rate and resume-rate could be larger than 10%% to avoid vibration of the log information");
        }
    }

    return rc;
}

int32
_if_mgr_cmd_set_output_rate_threshold(tbl_port_stats_rate_t *p_db_stats_rate,
    uint8 threshold, uint8 resumerate, cfg_cmd_para_t *para)
{
    int32  rc = PM_E_NONE;

    if (threshold < resumerate)
    {
        CFG_CONFLICT_RET("Resume-rate can't be larger than output-rate");
    }

    if (p_db_stats_rate->output_rate_threshold == threshold
       && p_db_stats_rate->output_rate_resume == resumerate)
    {
        return rc;
    }

    p_db_stats_rate->output_rate_threshold = threshold;
    p_db_stats_rate->output_rate_resume = resumerate;
    PM_E_RETURN(tbl_port_stats_rate_set_port_stats_rate_field(p_db_stats_rate,
                TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_THRESHOLD));
    PM_E_RETURN(tbl_port_stats_rate_set_port_stats_rate_field(p_db_stats_rate,
                TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_RESUME));

    if (0 == threshold && 0 == resumerate)
    {
        p_db_stats_rate->output_rate_state = LOG_THRESHOLD_DISABLE;
    }
    else
    {
        p_db_stats_rate->output_rate_state = LOG_THRESHOLD_ENABLE;
        if (threshold - resumerate < 10)
        {
            CFG_PROMPT_RET("Notice: The gap of output-rate and resume-rate could be larger than 10%% to avoid vibration of the log information");
        }
    }

    return rc;
}

int32
_if_mgr_cmd_set_output_discard_threshold(tbl_port_stats_rate_t *p_db_stats_rate,
    uint32 threshold, uint32 interval, cfg_cmd_para_t *para)
{
    int32  rc = PM_E_NONE;

    if (p_db_stats_rate->output_discard_threshold == threshold &&
       p_db_stats_rate->output_discard_interval  == interval)
    {
        return rc;
    }

    p_db_stats_rate->output_discard_threshold = threshold;
    p_db_stats_rate->output_discard_interval  = interval;
    PM_E_RETURN(tbl_port_stats_rate_set_port_stats_rate_field(p_db_stats_rate,
                TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_THRESHOLD));
    PM_E_RETURN(tbl_port_stats_rate_set_port_stats_rate_field(p_db_stats_rate,
                TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_INTERVAL));

    return rc;
}

int32
_if_mgr_cmd_set_load_interval(tbl_port_stats_rate_t *p_db_stats_rate, uint16 interval, cfg_cmd_para_t *para)
{
    int32  rc = PM_E_NONE;
    if (p_db_stats_rate->load_interval == interval)
    {
        return rc;
    }

    if (interval < GLB_IF_STATS_LOAD_MIN_INTVAL || interval > GLB_IF_STATS_LOAD_DEF_INTVAL)
    {
        CFG_CONFLICT_RET("Load-interval must be smaller than 30 seconds and less than 5 minutes");
    }

    if (interval % GLB_IF_STATS_LOAD_MIN_INTVAL != 0)
    {
        CFG_CONFLICT_RET("Load-interval must be in increments of 30 seconds");
    }

    p_db_stats_rate->load_interval = interval;
    PM_E_RETURN(tbl_port_stats_rate_set_port_stats_rate_field(p_db_stats_rate,
                        TBL_PORT_STATS_RATE_FLD_LOAD_INTERVAL));

    return rc;
}

const char*
_if_mgr_cmd_get_errdis_reason(tbl_interface_t* p_db_if, char* errdis_reason)
{
    uint32 i = 0;
    errdis_reason[0] = CMD_ZERO_CHAR;

    if (ERRDIS_NO_ERRDIS != p_db_if->errdisable_reason)
    {
        for(i = 0; i < ERRDIS_RSN_MAX; i++)
        {
            if (GLB_FLAG_ISSET(p_db_if->errdisable_reason, 1<<i))
            {
                sal_snprintf(errdis_reason, 64, " (%s)", errdis_rsn_strs[i]);
                return errdis_reason;
            }
        }
    }

    return errdis_reason;
}

const char*
_if_mgr_cmd_get_status(tbl_interface_t* p_db_if)
{
    if (p_db_if->errdisable_reason != ERRDIS_NO_ERRDIS)
    {
        return "Error Disabled";
    }
    else if ((GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG)))
    {
        return "Monitoring";
    }
    else if ((p_db_if->flags & (GLB_IFF_UP|GLB_IFF_RUNNING)) == (GLB_IFF_UP|GLB_IFF_RUNNING))
    {
        return "UP";
    }
    else
    {
        return (p_db_if->flags & GLB_IFF_UP) ? "DOWN" : "Administratively DOWN";
    }
}

const char*
_if_mgr_cmd_show_interface_media_hwtype_str(tbl_interface_t* p_db_if)
{
    switch (p_db_if->hw_type)
    {
    case GLB_IF_TYPE_PORT_IF:
        return cdb_enum_val2str(glb_phy_type_strs, GLB_PHY_TYPE_MAX, p_db_if->phy_type);
        
    case GLB_IF_TYPE_LINKAGG_IF:
        return "Aggregation";

    case GLB_IF_TYPE_VLAN_IF:
        return "Vlan Interface";
    case GLB_IF_TYPE_SUB_IF:
        return "Sub Interface";
    case GLB_IF_TYPE_LOOPBACK_IF:
        return "Lookback Interface";
    case GLB_IF_TYPE_TUNNEL_IF:
        return "Tunnel Interface";
    case GLB_IF_TYPE_L2GRE_IF:
        return "L2gre Interface";
    case GLB_IF_TYPE_L3GRE_IF:
        return "L3gre Interface";
    case GLB_IF_TYPE_NVGRE_IF:
        return "Nvgre Interface";
    case GLB_IF_TYPE_VXLAN_IF:
        return "Vxlan Interface";
    case GLB_IF_TYPE_CPU_IF:
        return "CPU Interface";
        
    default:
        return "Unknown";
    }
}

const char*
_if_mgr_cmd_get_broadcast_str(addr_ipv4_t ip_addr, uint32 mask_len)
{
    uint32 mask_un = 0;
    static char buf[256];
    addr_ipv4_t ip_addr_broadcast;
    sal_memset(buf, 0, sizeof(buf));
    ip_addr.s_addr = sal_htonl(ip_addr.s_addr);
    IPV4_LEN_TO_MASK(mask_un, mask_len);
    mask_un = ~mask_un;
    ip_addr_broadcast.s_addr = ip_addr.s_addr | mask_un;
    ip_addr_broadcast.s_addr = sal_htonl(ip_addr_broadcast.s_addr);
    
    return cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &ip_addr_broadcast);
                
}

static int32
_if_mgr_cmd_show_interface_stats(uint32 ifindex, FILE* fp)
{
    char load_intval_str[30] = {0};
    uint16 minutes;
    uint16 seconds;
    tbl_port_stats_t* p_db_ps = NULL;
    tbl_port_stats_key_t port_stats_key;
    tbl_port_stats_rate_t* p_db_rate = NULL;
    tbl_port_stats_rate_key_t port_stats_rate_key;
    tbl_interface_t *p_db_if = NULL;
    uint32 ts = (uint32)ctc_time_boottime_sec(NULL);
    uint32 delta_ts = 0;
    char   time_str[CMD_BUF_64];

    sal_memset(&port_stats_key, 0, sizeof(port_stats_key));
    sal_memset(&port_stats_rate_key, 0, sizeof(port_stats_rate_key));
    port_stats_key.ifindex = ifindex;
    port_stats_rate_key.ifindex = ifindex;
    p_db_ps = tbl_port_stats_get_port_stats(&port_stats_key);
    p_db_rate = tbl_port_stats_rate_get_port_stats_rate(&port_stats_rate_key);
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    uint64 octets_rcv_rate = 0;
    uint64 octets_send_rate = 0;
    uint64 octets_rcv = 0;
    uint64 octets_send = 0;
    if (NULL == p_db_ps || NULL == p_db_rate || NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    delta_ts = ts - p_db_ps->last_clear_ts;
    if (0 == p_db_ps->last_clear_ts)
    {
        sal_fprintf(fp, "    Last clearing of \"show interface\" counters never\n");
    }
    else
    {
        ctc_time_seconds_to_ymd_hms(delta_ts, time_str, CMD_BUF_64);
        sal_fprintf(fp, "    Last clearing of \"show interface\" counters %s\n", time_str);
    }

    minutes = p_db_rate->load_interval / 60;
    seconds = p_db_rate->load_interval % 60;
    if(0 == seconds)
    {
        if(1 == minutes)
        {
            sal_sprintf(load_intval_str, "1 minute");
        }
        else
        {
            sal_sprintf(load_intval_str, "%d minutes", minutes);
        }
    }
    else if(0 == minutes)
    {
        sal_sprintf(load_intval_str, "30 seconds");
    }
    else
    {
        if(1 == minutes)
        {
            sal_sprintf(load_intval_str, "1 minute, %d seconds", seconds);
        }
        else
        {
            sal_sprintf(load_intval_str, "%d minutes, %d seconds", minutes, seconds);
        }
    } 
    if (p_db_if->ifg_stats_en)
    {
        octets_rcv_rate = p_db_rate->octets_rcv_rate + p_db_rate->pkts_rcv_rate*GLB_DEF_IPG_LEN*8;
        octets_send_rate = p_db_rate->octets_send_rate + p_db_rate->pkts_send_rate*GLB_DEF_IPG_LEN*8;
        octets_rcv = p_db_ps->octets_rcv + p_db_ps->pkts_rcv*GLB_DEF_IPG_LEN;
        octets_send = p_db_ps->octets_send + p_db_ps->pkts_send*GLB_DEF_IPG_LEN;
    }
    else
    {
        octets_rcv_rate = p_db_rate->octets_rcv_rate;
        octets_send_rate = p_db_rate->octets_send_rate;
        octets_rcv = p_db_ps->octets_rcv;
        octets_send = p_db_ps->octets_send;
    }
    
    sal_fprintf(fp,"    %s input rate %"PRIu64" bits/sec, %"PRIu64" packets/sec\n",load_intval_str, 
                     octets_rcv_rate,p_db_rate->pkts_rcv_rate);   
    sal_fprintf(fp,"    %s output rate %"PRIu64" bits/sec, %"PRIu64" packets/sec\n",load_intval_str, 
                     octets_send_rate,p_db_rate->pkts_send_rate);
    sal_fprintf(fp,"    %"PRIu64" packets input, %"PRIu64" bytes\n", p_db_ps->pkts_rcv,octets_rcv);
    sal_fprintf(fp,"    Received %"PRIu64" unicast,", p_db_ps->uc_pkts_rcv);
    sal_fprintf(fp," %"PRIu64" broadcast,", p_db_ps->brdc_pkts_rcv);
    sal_fprintf(fp," %"PRIu64" multicast\n", p_db_ps->mc_pkts_rcv);
    sal_fprintf(fp,"    %"PRIu64" runts,", p_db_ps->undersize_pkts);
    sal_fprintf(fp," %"PRIu64" giants,", p_db_ps->oversize_pkts); 
    /* The receive input error packets add the num of crc error for fix bug 39425 */
    sal_fprintf(fp," %"PRIu64" input errors,", (p_db_ps->mac_rcv_error + p_db_ps->bad_crc)); 
    sal_fprintf(fp," %"PRIu64" CRC\n", p_db_ps->bad_crc); 
    sal_fprintf(fp,"    %"PRIu64" frame,", p_db_ps->frame_error); 
    sal_fprintf(fp," %"PRIu64" overrun,", p_db_ps->drop_events); 
    sal_fprintf(fp," %"PRIu64" pause input\n", p_db_ps->pause_rcv);
    sal_fprintf(fp,"    %"PRIu64" packets output, %"PRIu64" bytes\n", p_db_ps->pkts_send, octets_send);
    sal_fprintf(fp,"    Transmitted %"PRIu64" unicast,", p_db_ps->uc_pkts_send);
    sal_fprintf(fp," %"PRIu64" broadcast,", p_db_ps->brdc_pkts_send);
    sal_fprintf(fp," %"PRIu64" multicast\n", p_db_ps->mc_pkts_send);
    sal_fprintf(fp,"    %"PRIu64" underruns,", p_db_ps->underruns);
    sal_fprintf(fp," %"PRIu64" output errors,", p_db_ps->mac_transmit_err);
    sal_fprintf(fp," %"PRIu64" pause output\n", p_db_ps->pause_send);

    return PM_E_NONE;
}

/*Added by zhush for bug 52768, 2019-07-04*/
static int32
_if_mgr_cmd_show_interface_stats_json(uint32 ifindex, cJSON* if_cj)
{
    char load_intval_str[30] = {0};
    uint16 minutes;
    uint16 seconds;
    tbl_port_stats_t* p_db_ps = NULL;
    tbl_port_stats_key_t port_stats_key;
    tbl_port_stats_rate_t* p_db_rate = NULL;
    tbl_port_stats_rate_key_t port_stats_rate_key;
    tbl_interface_t *p_db_if = NULL;
    char   byte_str[CMD_BUF_64];
    char   pkt_str[CMD_BUF_64];
    cJSON *input_info = NULL;
    cJSON *output_info = NULL;
	
    sal_memset(&port_stats_key, 0, sizeof(port_stats_key));
    sal_memset(&port_stats_rate_key, 0, sizeof(port_stats_rate_key));
    port_stats_key.ifindex = ifindex;
    port_stats_rate_key.ifindex = ifindex;
    p_db_ps = tbl_port_stats_get_port_stats(&port_stats_key);
    p_db_rate = tbl_port_stats_rate_get_port_stats_rate(&port_stats_rate_key);
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    uint64 octets_rcv_rate = 0;
    uint64 octets_send_rate = 0;
    uint64 octets_rcv = 0;
    uint64 octets_send = 0;
    if (NULL == p_db_ps || NULL == p_db_rate || NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    minutes = p_db_rate->load_interval / 60;
    seconds = p_db_rate->load_interval % 60;
    if(0 == seconds)
    {
        if(1 == minutes)
        {
            sal_sprintf(load_intval_str, "1 minute");
        }
        else
        {
            sal_sprintf(load_intval_str, "%d minutes", minutes);
        }
    }
    else if(0 == minutes)
    {
        sal_sprintf(load_intval_str, "30 seconds");
    }
    else
    {
        if(1 == minutes)
        {
            sal_sprintf(load_intval_str, "1 minute, %d seconds", seconds);
        }
        else
        {
            sal_sprintf(load_intval_str, "%d minutes, %d seconds", minutes, seconds);
        }
    } 
    if (p_db_if->ifg_stats_en)
    {
        octets_rcv_rate = p_db_rate->octets_rcv_rate + p_db_rate->pkts_rcv_rate*GLB_DEF_IPG_LEN*8;
        octets_send_rate = p_db_rate->octets_send_rate + p_db_rate->pkts_send_rate*GLB_DEF_IPG_LEN*8;
        octets_rcv = p_db_ps->octets_rcv + p_db_ps->pkts_rcv*GLB_DEF_IPG_LEN;
        octets_send = p_db_ps->octets_send + p_db_ps->pkts_send*GLB_DEF_IPG_LEN;
    }
    else
    {
        octets_rcv_rate = p_db_rate->octets_rcv_rate;
        octets_send_rate = p_db_rate->octets_send_rate;
        octets_rcv = p_db_ps->octets_rcv;
        octets_send = p_db_ps->octets_send;
    }

    input_info = cJSON_CreateObject();
    cJSON_AddItemToObject(if_cj, "Input information", input_info);
    sal_snprintf(byte_str, sizeof(byte_str), "%"PRIu64" ", octets_rcv_rate);
    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_rate->pkts_rcv_rate);
    cJSON_AddStringToObject(input_info, "input time", load_intval_str);	
    cJSON_AddStringToObject(input_info, "input rate", byte_str);
    cJSON_AddStringToObject(input_info, "packets", pkt_str);
	
    sal_snprintf(byte_str, sizeof(byte_str), "%"PRIu64" ", octets_send_rate);
    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_rate->pkts_send_rate);
    output_info = cJSON_CreateObject();
    cJSON_AddItemToObject(if_cj, "Output information", output_info);
    cJSON_AddStringToObject(output_info, "output time", load_intval_str);	
    cJSON_AddStringToObject(output_info, "output rate", byte_str);
    cJSON_AddStringToObject(output_info, "packets", pkt_str);

    sal_snprintf(byte_str, sizeof(byte_str), "%"PRIu64" ", p_db_ps->pkts_rcv);
    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", octets_rcv);
    cJSON_AddStringToObject(input_info, "packets input", pkt_str);
    cJSON_AddStringToObject(input_info, "bytes input", byte_str);

    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_ps->pkts_rcv);
    cJSON_AddStringToObject(input_info, "received unicate", pkt_str);

    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_ps->brdc_pkts_rcv);
    cJSON_AddStringToObject(input_info, "received broadcast", pkt_str);

    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_ps->mc_pkts_rcv);
    cJSON_AddStringToObject(input_info, "received multicast", pkt_str);

    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_ps->undersize_pkts);
    cJSON_AddStringToObject(input_info, "received runts", pkt_str);

    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_ps->oversize_pkts);
    cJSON_AddStringToObject(input_info, "received giants", pkt_str);
    /* The receive input error packets add the num of crc error for fix bug 39425 */
    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", (p_db_ps->mac_rcv_error + p_db_ps->bad_crc));
    cJSON_AddStringToObject(input_info, "input errors", pkt_str);

    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_ps->bad_crc);
    cJSON_AddStringToObject(input_info, "CRC", pkt_str);

    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_ps->frame_error);
    cJSON_AddStringToObject(input_info, "frame", pkt_str);

    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_ps->drop_events);
    cJSON_AddStringToObject(input_info, "overrun", pkt_str);

    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_ps->pause_rcv);
    cJSON_AddStringToObject(input_info, "received pause input", pkt_str);

    
    sal_snprintf(byte_str, sizeof(byte_str), "%"PRIu64" ", p_db_ps->pkts_send);
    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", octets_send);
    cJSON_AddStringToObject(output_info, "packets output", pkt_str);
    cJSON_AddStringToObject(output_info, "bytes output", byte_str);

    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_ps->uc_pkts_send);
    cJSON_AddStringToObject(output_info, "Transmitted unicast", pkt_str);

    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_ps->uc_pkts_send);
    cJSON_AddStringToObject(output_info, "Transmitted broadcast", pkt_str);

    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_ps->uc_pkts_send);
    cJSON_AddStringToObject(output_info, "Transmitted multicast", pkt_str);

    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_ps->uc_pkts_send);
    cJSON_AddStringToObject(output_info, "underruns", pkt_str);

    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_ps->uc_pkts_send);
    cJSON_AddStringToObject(output_info, "output errors", pkt_str);

    sal_snprintf(pkt_str, sizeof(pkt_str), "%"PRIu64" ", p_db_ps->uc_pkts_send);
    cJSON_AddStringToObject(output_info, "pause output", pkt_str);

    return PM_E_NONE;
}
/*end zhush*/

static int32
_if_mgr_cmd_process_interface_show_one_overlay_tn(tbl_interface_t* p_db_if, FILE* fp, cfg_cmd_para_t *para)
{
    if (!INTERFACE_IS_OVERLAY_TN(p_db_if))
    {
        return PM_E_NONE;
    }
    
    #ifdef OFPRODUCT
    sal_fprintf(fp,"  warning: This command is reserved, please use \"show openflow interface tunnel brief\" \n");

    sal_fprintf(fp, "\n");
    #endif
    
    return PM_E_NONE;
#if 0
    sal_fprintf(fp,"Interface %s\n", p_db_if->key.name);
    sal_fprintf(fp,"  Hardware is %s\n", cdb_enum_val2str(glb_if_type_strs, GLB_IF_TYPE_MAX, p_db_if->hw_type));
    sal_fprintf(fp,"  Index %u\n", p_db_if->ifindex);
    if (sal_strlen(p_db_if->desc))
    {
        sal_fprintf(fp,"  Description is: %s\n", p_db_if->desc);
    }
    sal_fprintf(fp,"  warning: This command is reserved, please use \"show openflow interface tunnel brief\" \n");

    sal_fprintf(fp, "\n");
    
    return PM_E_NONE;
#endif
}

static int32
_if_mgr_cmd_process_interface_show_one_normal(tbl_interface_t* p_db_if, FILE* fp, cfg_cmd_para_t *para)
{
    tbl_route_if_t      *p_db_rf = NULL;
    ds_connected_t      *p_db_conn = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    tbl_route_if_key_t  rif_key;
    char                ifname_ext[IFNAME_EXT_SIZE];
    char                mac_addr_buf[CMD_BUF_32];
    char                buf[256];
    char                errdis_reason[64];
    //char                fec_ability_str[256];

    sal_memset(&rif_key, 0, sizeof(rif_key));
    sal_strncpy(rif_key.name, p_db_if->key.name, IFNAME_SIZE);
    p_db_rf = tbl_route_if_get_route_if(&rif_key);

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    
    if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type || GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        if (GLB_IF_MODE_L3 == p_db_if->mode)
        {
            if (NULL == p_db_rf)
            {
                sal_fprintf(fp,"Interface %s has no route interface, please wait a second and show again\n", ifname_ext);
                return PM_E_NONE;
            }
            cdb_mac_addr_val2str(mac_addr_buf, CMD_BUF_32, p_db_rf->mac);
        }
        else if (GLB_IF_MODE_L2 == p_db_if->mode)
        {
            cdb_mac_addr_val2str(mac_addr_buf, CMD_BUF_32, p_db_if->mac_addr);
        }

        sal_memset(errdis_reason, 0, sizeof(errdis_reason));
        _if_mgr_cmd_get_errdis_reason(p_db_if, errdis_reason);
        sal_fprintf(fp,"Interface %s\n", ifname_ext);
        sal_fprintf(fp,"  Interface current state: %s%s\n", _if_mgr_cmd_get_status(p_db_if), errdis_reason); 
        sal_fprintf(fp,"  Hardware is %s, ", cdb_enum_val2str(glb_if_type_strs, GLB_IF_TYPE_MAX, p_db_if->hw_type));
        sal_fprintf(fp,"address is %s\n", mac_addr_buf);
        if (sal_strlen(p_db_if->desc))
        {
            sal_fprintf(fp,"  Description is: %s\n", p_db_if->desc);
        }
        sal_fprintf(fp,"  Bandwidth %d kbits\n", p_db_if->oper_bandwidth);
        sal_fprintf(fp,"  Index %u , Metric %s\n", p_db_if->ifindex, "1");
        
        sal_fprintf(fp,"  Speed - %s ," , _if_mgr_get_speed_str(p_db_if->speed, p_db_if->oper_speed));
        sal_fprintf(fp,"  Duplex - %s ," , _if_mgr_get_duplex_str(p_db_if->duplex, p_db_if->oper_duplex));
        #ifdef TAPPRODUCT
        sal_fprintf(fp,"  Metadata - %s ," , p_db_if->metadata_enable ? "Enable" : "Disable");
        #endif 
        sal_fprintf(fp,"  Media type is %s\n" , _if_mgr_cmd_show_interface_media_hwtype_str(p_db_if));
        sal_fprintf(fp,"  svlan-tpid - 0x%x\n" , p_db_if->svlan_tpid);
        if (p_db_if->carrier_down_holdtime_interval)
        {
            sal_fprintf(fp,"  Carrier down holdtime - %u ms\n", p_db_if->carrier_down_holdtime_interval);
        }
        /* Modified by liuht for bug42854, 2017-03-30 */
        if ((GLB_SPEED_AUTO != p_db_if->speed) && (GLB_DUPLEX_AUTO != p_db_if->duplex))
        {
            sal_fprintf(fp,"  Link type is force link\n");
        }
        else
        {
            sal_fprintf(fp,"  Link type is autonegotiation\n");
        }
        if ((GLB_CHIP_GREATEBELT != tbl_chip_get_type())&&(GLB_CHIP_GOLDENGATE != tbl_chip_get_type()))
        {
            sal_fprintf(fp,"  FEC config: %s",_if_mgr_get_fec_str(p_db_if));
            if(_if_mgr_cmd_check_fec_warning(p_db_if,p_db_if->fec))
            {
                sal_fprintf(fp,"(Invalid)\n");
            }
            else
            {
                sal_fprintf(fp,"\n");
            }
#if 0 /* you can show this in shell */
            _if_mgr_get_fec_ability_str(p_db_if,fec_ability_str);
            sal_fprintf(fp,"  FEC ability: %s\n",fec_ability_str);
#endif            
            sal_fprintf(fp,"  FEC status: %s\n",tbl_interface_is_running(p_db_if)? _if_mgr_get_fec_status_str(p_db_if):"Unknown");
        }
        sal_fprintf(fp,"  Admin input flow-control is %s, output flow-control is %s\n",
            on_off_str_lower((GLB_FLAG_ISSET(p_db_if->flowcontrol, GLB_IF_FLOW_CONTROL_RECEIVE))),  
            on_off_str_lower((GLB_FLAG_ISSET(p_db_if->flowcontrol, GLB_IF_FLOW_CONTROL_SEND))));
          
        sal_fprintf(fp,"  Oper input flow-control is %s, output flow-control is %s\n",
            on_off_str_lower((GLB_FLAG_ISSET(p_db_if->flowcontrol, GLB_IF_FLOW_OPER_CONTROL_RECEIVE))),
            on_off_str_lower((GLB_FLAG_ISSET(p_db_if->flowcontrol, GLB_IF_FLOW_OPER_CONTROL_SEND))));

        sal_fprintf(fp,"  The Maximum Frame Size is %u bytes\n",
                (p_db_if->jumboframe_en) ? GLB_DFT_MAX_FRAME_SIZE_1 : GLB_DFT_MAX_FRAME_SIZE_0);
        sal_fprintf(fp,"  Flow-statistics include-interframe %s\n", p_db_if->ifg_stats_en?"enable":"disable");
        
        if (GLB_IF_MODE_L3 == p_db_if->mode)
        {
#if 0 /* removed by kcao for bug 44387 */
            sal_fprintf(fp,"  VRF binding: not bound\n");
            sal_fprintf(fp,"  No Virtual private Wire service configured\n");
#endif
            sal_fprintf(fp,"  ARP timeout is %us ,", p_db_rf->arp_timeout);
            sal_fprintf(fp,"  ARP retry interval %us\n", p_db_rf->arp_retrans_time);
            sal_fprintf(fp,"  ARP Proxy is %s, Local ARP Proxy is %s\n", (p_db_rf->arp_proxy_en ? "enabled" : "disabled"), (p_db_rf->local_arp_proxy_en ? "enabled" : "disabled"));
            sal_fprintf(fp,"  The maximum transmit unit (MTU) is %u bytes\n", p_db_rf->mtu);

            CTCLIB_SLIST_LOOP(p_db_rf->ifc_ipv4.obj_list, p_db_conn, listnode)
            {
                if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
                {
                    sal_fprintf(fp,"  Internet primary address:\n");
                    sal_fprintf(fp,"    %s%s", cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_conn->key.address.u.prefix4), "/");
                    sal_fprintf(fp,"%u ", p_db_conn->key.address.prefixlen);
                    sal_fprintf(fp,"broadcast %s\n", _if_mgr_cmd_get_broadcast_str(p_db_conn->destination.u.prefix4, p_db_conn->destination.prefixlen));
                }
            }
        }

        _if_mgr_cmd_show_interface_stats(p_db_if->ifindex, fp);
    }
    else if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type || GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type )
    {
        if (NULL == p_db_rf)
        {
            sal_fprintf(fp,"Interface %s has no route interface, please wait a second and show again\n", ifname_ext);
            return PM_E_NONE;
        }
        sal_fprintf(fp,"Interface %s\n", ifname_ext);  
        /*fix bug 44468: add errdisable reason show on vlanif*/
        if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
        {
            sal_memset(errdis_reason, 0, sizeof(errdis_reason));
            _if_mgr_cmd_get_errdis_reason(p_db_if, errdis_reason);
            sal_fprintf(fp,"  Interface current state: %s%s\n", _if_mgr_cmd_get_status(p_db_if), errdis_reason); 
        }
        else
        {
            sal_fprintf(fp,"  Interface current state: %s\n", _if_mgr_cmd_get_status(p_db_if));
        }
        if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
        {
            sal_fprintf(fp,"  Hardware is %s, ", cdb_enum_val2str(glb_if_type_strs, GLB_IF_TYPE_MAX, p_db_if->hw_type));
            sal_fprintf(fp,"address is %s\n", cdb_mac_addr_val2str(mac_addr_buf, CMD_BUF_32, p_db_rf->mac));
        }
        else 
        {
             sal_fprintf(fp,"  Hardware is %s\n", cdb_enum_val2str(glb_if_type_strs, GLB_IF_TYPE_MAX, p_db_if->hw_type));
        }
        if (sal_strlen(p_db_if->desc))
        {
            sal_fprintf(fp,"  Description is: %s\n", p_db_if->desc);
        }
        else
        {
            sal_fprintf(fp,"  Description is: NULL\n");
        }
        sal_fprintf(fp,"  Bandwidth %d kbits\n", p_db_if->oper_bandwidth);
        sal_fprintf(fp,"  Index %u , Metric %s\n", p_db_if->ifindex, "1");
        sal_fprintf(fp,"  The maximum transmit unit (MTU) is %u bytes\n", p_db_rf->mtu);

#if 0 /* removed by kcao for bug 44387 */
        sal_fprintf(fp,"  VRF binding: not bound\n");
        sal_fprintf(fp,"  No Virtual private Wire service configured\n");
#endif
        sal_fprintf(fp,"  ARP timeout is %us ,", p_db_rf->arp_timeout);
        sal_fprintf(fp,"  ARP retry interval %us\n", p_db_rf->arp_retrans_time);
        sal_fprintf(fp,"  ARP Proxy is %s, Local ARP Proxy is %s\n", (p_db_rf->arp_proxy_en ? "enabled" : "disabled"), (p_db_rf->local_arp_proxy_en ? "enabled" : "disabled"));
        
        CTCLIB_SLIST_LOOP(p_db_rf->ifc_ipv4.obj_list, p_db_conn, listnode)
        {
            if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
            {
                sal_fprintf(fp,"  Internet primary address:\n");
                sal_fprintf(fp,"    %s%s", cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_conn->key.address.u.prefix4), "/");
                sal_fprintf(fp,"%u ", p_db_conn->key.address.prefixlen);
                sal_fprintf(fp,"broadcast %s\n", _if_mgr_cmd_get_broadcast_str(p_db_conn->destination.u.prefix4, p_db_conn->destination.prefixlen));
            }
        }
    }
    /* added by chenc for show interface show two enter, because tbl_interface has cpu interface, but not show*/
    else
    {
        return PM_E_NONE;
    }
    /*chenc end*/

    sal_fprintf(fp,"\n");

    return PM_E_NONE;
}

/*Added by zhush for bug 52768, 2019-07-04*/
static int32
_if_mgr_cmd_process_interface_show_one_normal_json(tbl_interface_t* p_db_if, FILE* fp, cfg_cmd_para_t *para)
{
    tbl_route_if_t      *p_db_rf = NULL;
    ds_connected_t      *p_db_conn = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    tbl_route_if_key_t  rif_key;
    char                ifname_ext[IFNAME_EXT_SIZE];
    char                mac_addr_buf[CMD_BUF_32];
    char                buf[256];
    char                errdis_reason[64];
    cJSON             *root = NULL;
    cJSON             *if_cj = NULL;
    cJSON             *if_pri_add = NULL;
    cJSON             *if_con = NULL;
    char                *out = NULL;
	
    sal_memset(&rif_key, 0, sizeof(rif_key));
    sal_strncpy(rif_key.name, p_db_if->key.name, IFNAME_SIZE);
    p_db_rf = tbl_route_if_get_route_if(&rif_key);

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    
    if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type || GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        if (GLB_IF_MODE_L3 == p_db_if->mode)
        {
            if (NULL == p_db_rf)
            {
                sal_fprintf(fp,"Interface %s has no route interface, please wait a second and show again\n", ifname_ext);
                return PM_E_NONE;
            }
            cdb_mac_addr_val2str(mac_addr_buf, CMD_BUF_32, p_db_rf->mac);
        }
        else if (GLB_IF_MODE_L2 == p_db_if->mode)
        {
            cdb_mac_addr_val2str(mac_addr_buf, CMD_BUF_32, p_db_if->mac_addr);
        }

        sal_memset(errdis_reason, 0, sizeof(errdis_reason));
        _if_mgr_cmd_get_errdis_reason(p_db_if, errdis_reason);
	 root = cJSON_CreateObject();
	 if_cj = cJSON_CreateObject();
	 cJSON_AddItemToObject(root, ifname_ext, if_cj);
	 cJSON_AddStringToObject(if_cj, "Interface current state", _if_mgr_cmd_get_status(p_db_if));
	 if (strlen(errdis_reason) != 0)
        {
	     cJSON_AddStringToObject(if_cj, "Errdisable reason", errdis_reason);
        }
	 cJSON_AddStringToObject(if_cj, "Hardware", cdb_enum_val2str(glb_if_type_strs, GLB_IF_TYPE_MAX, p_db_if->hw_type));
	 cJSON_AddStringToObject(if_cj, "address",  mac_addr_buf);
        if (sal_strlen(p_db_if->desc))
        {
	     cJSON_AddStringToObject(if_cj, "Description",  p_db_if->desc);
        }
	 cJSON_AddNumberToObject(if_cj, "Bandwidth(kbits)",  p_db_if->oper_bandwidth);
        cJSON_AddNumberToObject(if_cj, "Index",  p_db_if->ifindex);
        cJSON_AddStringToObject(if_cj, "Metric",  "1");
	 cJSON_AddStringToObject(if_cj, "Speed", _if_mgr_get_speed_str(p_db_if->speed, p_db_if->oper_speed));
        cJSON_AddStringToObject(if_cj, "Duplex", _if_mgr_get_duplex_str(p_db_if->duplex, p_db_if->oper_duplex));
        #ifdef TAPPRODUCT
	 cJSON_AddStringToObject(if_cj, "Media", p_db_if->metadata_enable ? "Enable" : "Disable");
        #endif 
	 cJSON_AddStringToObject(if_cj, "Media type", _if_mgr_cmd_show_interface_media_hwtype_str(p_db_if));

        /* Modified by liuht for bug42854, 2017-03-30 */
        if ((GLB_SPEED_AUTO != p_db_if->speed) && (GLB_DUPLEX_AUTO != p_db_if->duplex))
        {
            cJSON_AddStringToObject(if_cj, "Link type", "force link");
        }
        else
        {
            cJSON_AddStringToObject(if_cj, "Link type", "autonegotiation");
        }
       
        cJSON_AddStringToObject(if_cj, "Admin input flow-control", on_off_str_lower((GLB_FLAG_ISSET(p_db_if->flowcontrol, GLB_IF_FLOW_CONTROL_RECEIVE))));
	 cJSON_AddStringToObject(if_cj, "Output flow-control", on_off_str_lower((GLB_FLAG_ISSET(p_db_if->flowcontrol, GLB_IF_FLOW_CONTROL_SEND))));
        cJSON_AddStringToObject(if_cj, "Oper input flow-control", on_off_str_lower((GLB_FLAG_ISSET(p_db_if->flowcontrol, GLB_IF_FLOW_OPER_CONTROL_RECEIVE))));
	 cJSON_AddStringToObject(if_cj, "Output flow-control", on_off_str_lower((GLB_FLAG_ISSET(p_db_if->flowcontrol, GLB_IF_FLOW_OPER_CONTROL_SEND))));
        cJSON_AddNumberToObject(if_cj, "The Maximum Frame Size(bytes)",  (p_db_if->jumboframe_en) ? GLB_DFT_MAX_FRAME_SIZE_1 : GLB_DFT_MAX_FRAME_SIZE_0);
        if (GLB_IF_MODE_L3 == p_db_if->mode)
        {
#if 0 /* removed by kcao for bug 44387 */
            sal_fprintf(fp,"  VRF binding: not bound\n");
            sal_fprintf(fp,"  No Virtual private Wire service configured\n");
#endif
	     cJSON_AddNumberToObject(if_cj, "ARP timeout",  p_db_rf->arp_timeout);
            cJSON_AddNumberToObject(if_cj, "ARP retry interval(s)",  p_db_rf->arp_retrans_time);
	     cJSON_AddStringToObject(if_cj, "ARP Proxy",  (p_db_rf->arp_proxy_en ? "enabled" : "disabled"));
            cJSON_AddStringToObject(if_cj, "Local ARP Proxy",  (p_db_rf->local_arp_proxy_en ? "enabled" : "disabled"));  

            CTCLIB_SLIST_LOOP(p_db_rf->ifc_ipv4.obj_list, p_db_conn, listnode)
            {
                if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
                {
                    if_pri_add = cJSON_CreateObject();
                    cJSON_AddItemToObject(if_cj, "Internet primary address", if_pri_add);
                    if_con = cJSON_CreateObject();
                    cJSON_AddItemToObject(if_pri_add, "connected", if_con);
		      cJSON_AddStringToObject(if_con, "interface address", cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_conn->key.address.u.prefix4));
		      cJSON_AddNumberToObject(if_con, "subnet mask", p_db_conn->key.address.prefixlen);
                    cJSON_AddStringToObject(if_con, "broadcast", _if_mgr_cmd_get_broadcast_str(p_db_conn->destination.u.prefix4, p_db_conn->destination.prefixlen));
                }
            }
        }

        _if_mgr_cmd_show_interface_stats_json(p_db_if->ifindex, if_cj);
    }
    else if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type || GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type )
    {
        if (NULL == p_db_rf)
        {
            sal_fprintf(fp,"Interface %s has no route interface, please wait a second and show again\n", ifname_ext);
            return PM_E_NONE;
        }
         root = cJSON_CreateObject();
	 if_cj = cJSON_CreateObject();
	 cJSON_AddItemToObject(root, ifname_ext, if_cj);
	 
        /*fix bug 44468: add errdisable reason show on vlanif*/
        if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
        {
            sal_memset(errdis_reason, 0, sizeof(errdis_reason));
            _if_mgr_cmd_get_errdis_reason(p_db_if, errdis_reason);
	     cJSON_AddStringToObject(if_cj, "Interface current state", _if_mgr_cmd_get_status(p_db_if)); 
	     cJSON_AddStringToObject(if_cj, "Errdisable reason", errdis_reason);
        }
        else
        {
            cJSON_AddStringToObject(if_cj, "Interface current state", _if_mgr_cmd_get_status(p_db_if)); 
        }
        if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
        {
	     cJSON_AddStringToObject(if_cj, "Hardware", cdb_enum_val2str(glb_if_type_strs, GLB_IF_TYPE_MAX, p_db_if->hw_type));
	     cJSON_AddStringToObject(if_cj, "address",  cdb_mac_addr_val2str(mac_addr_buf, CMD_BUF_32, p_db_rf->mac));
        }
        else 
        {
            cJSON_AddStringToObject(if_cj, "Hardware", cdb_enum_val2str(glb_if_type_strs, GLB_IF_TYPE_MAX, p_db_if->hw_type));
        }
        if (sal_strlen(p_db_if->desc))
        {
	     cJSON_AddStringToObject(if_cj, "Description",  p_db_if->desc);
        }
        else
        {
            cJSON_AddStringToObject(if_cj, "Description",  "NULL");
        }
        
	 cJSON_AddNumberToObject(if_cj, "Bandwidth(kbits)",  p_db_if->oper_bandwidth);
        cJSON_AddNumberToObject(if_cj, "Index",  p_db_if->ifindex);
        cJSON_AddStringToObject(if_cj, "Metric",  "1");
	 cJSON_AddNumberToObject(if_cj, "The Maximum Frame Size(bytes)",  p_db_rf->mtu);

#if 0 /* removed by kcao for bug 44387 */
        sal_fprintf(fp,"  VRF binding: not bound\n");
        sal_fprintf(fp,"  No Virtual private Wire service configured\n");
#endif
        cJSON_AddNumberToObject(if_cj, "ARP timeout",  p_db_rf->arp_timeout);
        cJSON_AddNumberToObject(if_cj, "ARP retry interval(s)",  p_db_rf->arp_retrans_time);
	 cJSON_AddStringToObject(if_cj, "ARP Proxy",  (p_db_rf->arp_proxy_en ? "enabled" : "disabled"));
        cJSON_AddStringToObject(if_cj, "Local ARP Proxy",  (p_db_rf->local_arp_proxy_en ? "enabled" : "disabled"));
		
        CTCLIB_SLIST_LOOP(p_db_rf->ifc_ipv4.obj_list, p_db_conn, listnode)
        {
            if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
            {
		  if_pri_add = cJSON_CreateObject();
                cJSON_AddItemToObject(if_cj, "Internet primary address", if_pri_add);
                if_con = cJSON_CreateObject();
                cJSON_AddItemToObject(if_cj, "connected", if_con);
		  cJSON_AddStringToObject(if_con, "interface address", cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_conn->key.address.u.prefix4));
		  cJSON_AddNumberToObject(if_con, "subnet mask", p_db_conn->key.address.prefixlen);
                cJSON_AddStringToObject(if_con, "broadcast", _if_mgr_cmd_get_broadcast_str(p_db_conn->destination.u.prefix4, p_db_conn->destination.prefixlen));
            }
        }
    }
    /* added by chenc for show interface show two enter, because tbl_interface has cpu interface, but not show*/
    else
    {
        return PM_E_NONE;
    }
    /*chenc end*/
    out = cJSON_Print(root);
    sal_fprintf(fp, out);
    sal_fprintf(fp,"\n");

    cJSON_Delete(root);
    cJSON_free(out);

    return PM_E_NONE;
}
/*end zhush*/

static int32
_if_mgr_cmd_process_interface_show_one(tbl_interface_t* p_db_if, FILE* fp, cfg_cmd_para_t *para)
{
    if (INTERFACE_IS_OVERLAY_TN(p_db_if))
    {
        _if_mgr_cmd_process_interface_show_one_overlay_tn(p_db_if, fp, para);
    }
    else
    {
        _if_mgr_cmd_process_interface_show_one_normal(p_db_if, fp, para);
    }

    return PM_E_NONE;
}

/*Added by zhush for bug 52768, 2019-07-04*/
static int32
_if_mgr_cmd_process_interface_show_one_json(tbl_interface_t* p_db_if, FILE* fp, cfg_cmd_para_t *para)
{
    if (INTERFACE_IS_OVERLAY_TN(p_db_if))
    {
        _if_mgr_cmd_process_interface_show_one_overlay_tn(p_db_if, fp, para);
    }
    else
    {
        _if_mgr_cmd_process_interface_show_one_normal_json(p_db_if, fp, para);
    }

    return PM_E_NONE;
}
/*end zhush*/

static void
 _if_mgr_cmd_process_interface_null0_show_content(FILE *fp)
{
    sal_fprintf(fp,"Interface null0\n");
    sal_fprintf(fp,"  Interface current state: UP\n");
    sal_fprintf(fp,"  Line protocol current state: UP (spoofing)\n");
    sal_fprintf(fp,"  Physical is null device\n");
    sal_fprintf(fp,"\n");
}

static int32
 _if_mgr_cmd_process_interface_show(tbl_interface_t* p_db_if, cfg_cmd_para_t *para)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    if (p_db_if)
    {
        _if_mgr_cmd_process_interface_show_one(p_db_if, fp, para);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
        {
            _if_mgr_cmd_process_interface_show_one(p_db_if, fp, para);
        }
        _if_mgr_cmd_process_interface_null0_show_content(fp);
    }
    
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

/*Added by zhush for bug 52768, 2019-07-04*/
static int32
 _if_mgr_cmd_process_interface_show_json(tbl_interface_t* p_db_if, cfg_cmd_para_t *para)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    if (p_db_if)
    {
        _if_mgr_cmd_process_interface_show_one_json(p_db_if, fp, para);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
        {
            _if_mgr_cmd_process_interface_show_one_json(p_db_if, fp, para);
        }
    }
    
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

static int32
 _if_mgr_cmd_process_interface_null0_show(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
   _if_mgr_cmd_process_interface_null0_show_content(fp);
    
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
if_mgr_cmd_process_interface_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_t* p_db_if = NULL;
    tbl_interface_key_t if_key;
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            _if_mgr_cmd_process_interface_show(NULL, para);
        }
        else if (1 == argc)
        {
            sal_memset(&if_key, 0, sizeof(if_key));
            if (0 != sal_strcmp(argv[0], "null0"))
            {
                sal_strncpy(if_key.name, argv[0], IFNAME_SIZE);
                p_db_if = tbl_interface_get_interface(&if_key);
                if (NULL == p_db_if)
                {
                    IFNAME_ETH2FULL(argv[0], ifname_ext);
                    CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
                } 
                _if_mgr_cmd_process_interface_show(p_db_if, para);
            }
            else
            {
                _if_mgr_cmd_process_interface_null0_show(para);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");    
        }
        break;
 /*Added by zhush for bug 52768, 2019-07-04*/
    case CDB_OPER_GETJSON:  
	 if (0 == argc)
        {
            _if_mgr_cmd_process_interface_show_json(NULL, para);
        }
        else if (1 == argc)
        {
            sal_memset(&if_key, 0, sizeof(if_key));
            if (0 != sal_strcmp(argv[0], "null0"))
            {
                sal_strncpy(if_key.name, argv[0], IFNAME_SIZE);
                p_db_if = tbl_interface_get_interface(&if_key);
                if (NULL == p_db_if)
                {
                    IFNAME_ETH2FULL(argv[0], ifname_ext);
                    CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
                } 
                _if_mgr_cmd_process_interface_show_json(p_db_if, para);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");    
        }
        break;
/*end zhush*/
    default:
        CFG_INVALID_PARAM_RET("Invalid action");  
        break;
    }
    
    return PM_E_NONE;
}

static int32
_if_show_bandwidth_usage(FILE *fp, tbl_port_stats_rate_t *p_db_stats_rate, char *dir)
{
    int32  rc = PM_E_NONE;
    char   ifname[IFNAME_SIZE];
    char   ifname_ext[IFNAME_EXT_SIZE];
    char   speed_str[15] = {0};
    char   threshold[5] = {0};
    char   resume[5]    = {0};
    char   usage[5]     = {0};
    char   load_interval[15] = {0};
    tbl_interface_key_t intf;
    tbl_interface_t *p_db_if = NULL;

    tbl_interface_get_name_by_ifindex(p_db_stats_rate->key.ifindex, ifname, IFNAME_SIZE);
    IFNAME_ETH2FULL(ifname, ifname_ext);
    
    sal_memset(&intf, 0, sizeof(intf));
    sal_strncpy(intf.name, ifname, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&intf);
    if (!p_db_if)
    {
        return rc;
    }

    sal_memset(speed_str, 0, sizeof(speed_str));
    sal_sprintf(speed_str,"%s" , _if_mgr_get_speed_str(p_db_if->speed, p_db_if->oper_speed));

    if (NULL == dir)
    {
        return rc;
    }
    if (!sal_strcmp(dir, "all"))
    {
        if(p_db_stats_rate->input_rate_threshold)
        {
            sal_sprintf(threshold, "%d%%", p_db_stats_rate->input_rate_threshold);
            sal_sprintf(resume, "%d%%", p_db_stats_rate->input_rate_resume);
            sal_sprintf(usage, "%d%%", (p_db_stats_rate->input_rate < 100) ? p_db_stats_rate->input_rate : 100);
            sal_sprintf(load_interval, "%ds", p_db_stats_rate->load_interval);
            sal_fprintf(fp, "%-10s %-10s %-8s %-13s %-10s %-10s %s\n", ifname_ext, "input", speed_str,
                        load_interval, threshold, resume, usage);
        }

        if(p_db_stats_rate->output_rate_threshold)
        {
            sal_sprintf(threshold, "%d%%", p_db_stats_rate->output_rate_threshold);
            sal_sprintf(resume, "%d%%", p_db_stats_rate->output_rate_resume);
            sal_sprintf(usage, "%d%%", (p_db_stats_rate->output_rate < 100) ? p_db_stats_rate->output_rate : 100);
            sal_sprintf(load_interval, "%ds", p_db_stats_rate->load_interval);
            sal_fprintf(fp, "%-10s %-10s %-8s %-13s %-10s %-10s %s\n", ifname_ext, "output", speed_str,
                        load_interval, threshold, resume, usage);
        }
    }
    else if (!sal_strcmp(dir, "input"))
    {
        sal_sprintf(threshold, "%d%%", p_db_stats_rate->input_rate_threshold);
        sal_sprintf(resume, "%d%%", p_db_stats_rate->input_rate_resume);
        sal_sprintf(usage, "%d%%", (p_db_stats_rate->input_rate < 100) ? p_db_stats_rate->input_rate : 100);
        sal_sprintf(load_interval, "%ds", p_db_stats_rate->load_interval);
        sal_fprintf(fp, "%-10s %-10s %-8s %-13s %-10s %-10s %s\n", ifname_ext, "input", speed_str,
                        load_interval, threshold, resume, usage);
    }
    else if (!sal_strcmp(dir, "output"))
    {
        sal_sprintf(threshold, "%d%%", p_db_stats_rate->output_rate_threshold);
        sal_sprintf(resume, "%d%%", p_db_stats_rate->output_rate_resume);
        sal_sprintf(usage, "%d%%", (p_db_stats_rate->output_rate < 100) ? p_db_stats_rate->output_rate : 100);
        sal_sprintf(load_interval, "%ds", p_db_stats_rate->load_interval);
        sal_fprintf(fp, "%-10s %-10s %-8s %-13s %-10s %-10s %s\n", ifname_ext, "output", speed_str,
                    load_interval, threshold, resume, usage);
    }

    return rc;
}

int32
if_mgr_cmd_show_bandwidth_usage(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32      rc        = PM_E_NONE;
    FILE      *fp        = NULL;
    uint32     ifindex = 0;
    ctclib_slistnode_t     *listnode = NULL;
    tbl_port_stats_rate_key_t   port_stats_rate_key;
    tbl_port_stats_rate_t      *p_db_stats_rate;
    tbl_port_stats_rate_master_t *p_master  = NULL;

    p_master = tbl_port_stats_rate_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Tbl_port_stats_rate is not exist");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "%-10s %-10s %-8s %-13s %-10s %-10s %s", "Name", "Direction", "Speed",
                    "Load-interval", "Threshold", "ResumeRate", "Usage\n");
    sal_fprintf(fp, "----------+----------+--------+-------------+----------+----------+------\n");
    if (0 == argc)
    {
        CTCLIB_SLIST_LOOP(p_master->port_stats_rate_list, p_db_stats_rate, listnode)
        {
            _if_show_bandwidth_usage(fp, p_db_stats_rate, "all");
        }
    }
    else
    {
        ifindex = tbl_interface_get_ifindex_by_name(argv[0]);
        port_stats_rate_key.ifindex = ifindex;
        p_db_stats_rate = tbl_port_stats_rate_get_port_stats_rate(&port_stats_rate_key);
        if (NULL == p_db_stats_rate)
        {
            CFG_FUNC_ERR_RET("Port stats rate ifindex %d not exist", ifindex);
        }

        if (1 == argc)
        {
            if(0 == p_db_stats_rate->input_rate_threshold && 0 == p_db_stats_rate->output_rate_threshold)
            {
                CFG_CONFLICT_RET("Interface %s not set bandwidth log threshold", argv[0]);
            }
            _if_show_bandwidth_usage(fp, p_db_stats_rate, "all");
        }
        else
        {
            if(!sal_strcmp(argv[1], "input") && 0 == p_db_stats_rate->input_rate_threshold)
            {
                CFG_CONFLICT_RET("Interface %s not set input bandwidth log threshold", argv[0]);
            }
            else if(!sal_strcmp(argv[1], "output") && 0 == p_db_stats_rate->output_rate_threshold)
            {
                CFG_CONFLICT_RET("Interface %s not set output bandwidth log threshold", argv[0]);
            }
            _if_show_bandwidth_usage(fp, p_db_stats_rate, argv[1]);
        }
    }

    sal_fclose(fp);
    fp = NULL;
    return rc;
}


static int32
_if_mgr_cmd_process_isolation_get(tbl_isolation_t* p_db_iso, field_parse_t *p_field, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    if (p_db_iso)
    {
        tbl_isolation_dump_one(p_db_iso, &args);
    }
    else
    {
        tbl_isolation_iterate((TBL_ITER_CB_FUNC)tbl_isolation_dump_one, &args);
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
if_mgr_cmd_process_isolation(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_isolation_key_t key;
    tbl_isolation_t* p_db_iso = NULL;
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_INTERFACE);

    switch (para->oper)
    {
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, 0, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        if (0 == argc)
        {
        }
        else if (1 == argc)
        {
            sal_memset(&key, 0, sizeof(key));
            PM_CFG_GET_UINT(key.isolation_id, argv[0], para->p_rs);
            p_db_iso = tbl_isolation_get_isolation(&key);
            if (NULL == p_db_iso)
            {
                CFG_CONFLICT_RET("Isolation group %u not exist", key.isolation_id);
            }
            _if_mgr_cmd_process_isolation_get(p_db_iso, &field, para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");    
        }
        _if_mgr_cmd_process_isolation_get(p_db_iso, &field, para);
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");  
        break;
    }

    return PM_E_NONE;
}

static int32
_if_mgr_cmd_process_isolation_show_one(tbl_isolation_t* p_db_iso, FILE *fp, uint32 *banner)
{
    tbl_interface_t *p_db_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    uint32 i = 0;

    if (TRUE == *banner)
    {
        *banner = FALSE;
        sal_fprintf(fp, "--------------------------------------------------------------------------------\n");
        sal_fprintf(fp, "%-9s %s\n", "Group ID", "Member ports");
        sal_fprintf(fp, "---------+----------------------------------------------------------------------\n");
    }

    sal_fprintf(fp, "%-9u ", p_db_iso->key.isolation_id);

    CTCLIB_SLIST_LOOP(p_db_iso->ports.obj_list, p_db_if, listnode)
    {
        if (INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            continue;
        }
        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
        if (i && (i % 7) == 0)
        {
            sal_fprintf(fp, "\n");
            sal_fprintf(fp, "%-9s ", "");
        }
        i++;
        sal_fprintf(fp, "%-10s", ifname_ext);
    }
    sal_fprintf(fp, "\n");
    sal_fprintf(fp, "--------------------------------------------------------------------------------\n");

    return PM_E_NONE;
}

static int32
_if_mgr_cmd_process_isolation_show(tbl_isolation_t* p_db_iso, cfg_cmd_para_t *para)
{
    tbl_brg_global_t *p_brg_glb = tbl_brg_global_get_brg_global();
    tbl_isolation_master_t *p_master = tbl_isolation_get_master();
    uint32 isolation_id = 0;
    uint32 banner = TRUE;
    FILE *fp = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    sal_fprintf(fp, "%-18s : %s\n", "Port Isolate Mode", 
        cdb_enum_val2str(glb_port_isolate_strs, GLB_PORT_ISOLATE_MAX, p_brg_glb->port_isolate_mode));

    if (p_db_iso)
    {
        _if_mgr_cmd_process_isolation_show_one(p_db_iso, fp, &banner);
    }
    else
    {

        for (isolation_id = 0; isolation_id < GLB_PORT_ISOLATE_GROUP_MAX; isolation_id++)
        {
            p_db_iso = p_master->iso_array[isolation_id];
            if (NULL == p_db_iso)
            {
                continue;
            }
            _if_mgr_cmd_process_isolation_show_one(p_db_iso, fp, &banner);
        }
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
if_mgr_cmd_process_isolation_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_isolation_key_t key;
    tbl_isolation_t* p_db_iso = NULL;
    int32 rc = PM_E_NONE;
    
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            _if_mgr_cmd_process_isolation_show(NULL, para);
        }
        else if (1 == argc)
        {
            sal_memset(&key, 0, sizeof(key));
            PM_CFG_GET_UINT(key.isolation_id, argv[0], para->p_rs);
            p_db_iso = tbl_isolation_get_isolation(&key);
            if (NULL == p_db_iso)
            {
                CFG_DEL_SHOW_PATH(para->p_show->path, fp);
                CFG_CONFLICT_RET("Isolation group %u not exist", key.isolation_id);
            }
            _if_mgr_cmd_process_isolation_show(p_db_iso, para);
        }
        else
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_INVALID_PARAM_RET("Invalid parameter");    
        }
        break;
        
    default:
        CFG_DEL_SHOW_PATH(para->p_show->path, fp);
        CFG_INVALID_PARAM_RET("Invalid action");  
        break;
    }
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

static int32
_if_mgr_cmd_process_pvlan_show_one_vlan(tbl_pvlan_t* p_db_pvlan, FILE *fp, uint32 *banner)
{
    tbl_interface_t *p_db_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ds_pvlan_community_t *p_ds_pvlan = NULL;
    ds_pvlan_community_t p_community;
    
    char ifname_ext[IFNAME_EXT_SIZE];
    vid_t community_vlan = 0;
    int i = 0;

    if (TRUE == *banner)
    {
        *banner = FALSE;
        sal_fprintf(fp, "%-10s%-10s%-15s%-10s\n", "Primary","Secondary","Type","Ports");
        sal_fprintf(fp, "---------+---------+--------------+----------\n");
    }

    if (p_db_pvlan->promiscuous_ports.obj_list->count > 0)
    {
        i = 0;
        sal_fprintf(fp, "%-10d%-10s%-15s", p_db_pvlan->key.primary_vid, "N/A", "promiscuous");
        CTCLIB_SLIST_LOOP(p_db_pvlan->promiscuous_ports.obj_list, p_db_if, listnode)
        {
            if (INTERFACE_IS_LAG_MEMBER(p_db_if))
            {
                continue;
            }
            IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
            if(i && (i % 4) == 0)
            {
                sal_fprintf(fp, "\n");
                sal_fprintf(fp, "%-35s", "");
            }
            i++;
            sal_fprintf(fp, "%-10s", ifname_ext);
        }
        sal_fprintf(fp, "\n");
    }

    if (p_db_pvlan->isloate_ports.obj_list->count > 0)
    {
        i = 0;
        sal_fprintf(fp, "%-10d%-10s%-15s", p_db_pvlan->key.primary_vid, "N/A", "isolate");
        CTCLIB_SLIST_LOOP(p_db_pvlan->isloate_ports.obj_list, p_db_if, listnode)
        {
            if (INTERFACE_IS_LAG_MEMBER(p_db_if))
            {
                continue;
            }
            IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
            if(i && (i % 4) == 0)
            {
                sal_fprintf(fp, "\n");
                sal_fprintf(fp, "%-35s", "");
            }
            i++;
            sal_fprintf(fp, "%-10s", ifname_ext);
        }
        sal_fprintf(fp, "\n");
    }

    if (p_db_pvlan->community_groups.obj_list->count > 0)
    {
        for (community_vlan = GLB_VLAN_DEFAULT_VID; community_vlan <= GLB_VLAN_MAX; community_vlan++)
        {
            i = 0;
            sal_memset(&p_community, 0, sizeof(ds_pvlan_community_t));
            p_community.key.community_vid= community_vlan;
            p_ds_pvlan = ds_pvlan_community_get_pvlan_community(p_db_pvlan, &p_community);
            if (NULL == p_ds_pvlan)
            {
                continue;
            }
            
            sal_fprintf(fp, "%-10d%-10d%-15s", p_db_pvlan->key.primary_vid, p_ds_pvlan->key.community_vid, "community");
            CTCLIB_SLIST_LOOP(p_ds_pvlan->ports.obj_list, p_db_if, listnode)
            {
                if (INTERFACE_IS_LAG_MEMBER(p_db_if))
                {
                    continue;
                }
                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
                if(i && (i % 4) == 0)
                {
                    sal_fprintf(fp, "\n");
                    sal_fprintf(fp, "%-35s", "");
                }
                i++;
                sal_fprintf(fp, "%-10s", ifname_ext);
            }
            sal_fprintf(fp, "\n");
        }
    }

    return PM_E_NONE;
}

static int32
_if_mgr_cmd_process_pvlan_vlan_show(tbl_pvlan_t* p_db_pvlan, cfg_cmd_para_t *para)
{   
    tbl_pvlan_key_t  pvlan_key;
    tbl_pvlan_t* p_pvlan = NULL;
    vid_t vlan_id = 0;
    uint32 banner = TRUE;
    FILE *fp = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (p_db_pvlan)
    {
        _if_mgr_cmd_process_pvlan_show_one_vlan(p_db_pvlan, fp, &banner);
    }
    else
    {
        for (vlan_id = GLB_VLAN_DEFAULT_VID; vlan_id <= GLB_VLAN_MAX; vlan_id++)
        {
            sal_memset(&pvlan_key, 0, sizeof(tbl_pvlan_key_t));
            pvlan_key.primary_vid = vlan_id;
            p_pvlan = tbl_pvlan_get_pvlan(&pvlan_key);
            if (NULL == p_pvlan)
            {
                continue;
            }
            _if_mgr_cmd_process_pvlan_show_one_vlan(p_pvlan, fp, &banner);
        }
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

static int32
_if_mgr_cmd_process_pvlan_interface_show(tbl_interface_t* p_db_if, cfg_cmd_para_t *para)
{   
    FILE *fp = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    
    if (GLB_IF_PVLAN_TYPE_PROMISCUOUS == p_db_if->pvlan_type)
    {
        sal_fprintf(fp, "Private vlan mode: %s\n", "promiscuous");
    }
    else if (GLB_IF_PVLAN_TYPE_ISOLATED == p_db_if->pvlan_type)
    {
        sal_fprintf(fp, "Private vlan mode: %s\n", "isolate");
    }
    else if (GLB_IF_PVLAN_TYPE_COMMUNITY == p_db_if->pvlan_type)
    {
        sal_fprintf(fp, "Private vlan mode: %s\n", "community");
    }

    /*added by yejl to fix bug42062, 2016-12-10*/
    sal_fprintf(fp, "Primary vlan id  : %d\n", p_db_if->private_vlan);

    if (GLB_IF_PVLAN_TYPE_COMMUNITY == p_db_if->pvlan_type)
    {
        sal_fprintf(fp, "Community vlan id: %d\n", p_db_if->community_vlan);
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
if_mgr_cmd_process_pvlan_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_pvlan_key_t pvlan_key;
    tbl_interface_key_t if_key;
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t* p_db_vlan = NULL;
    tbl_pvlan_t* p_db_pvlan = NULL;
    tbl_interface_t* p_db_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            _if_mgr_cmd_process_pvlan_vlan_show(NULL, para);
        }
        else if (argc == 2)
        {
            if (0 == sal_strcmp(argv[0], "vlan"))
            {
                sal_memset(&vlan_key, 0, sizeof(tbl_vlan_key_t));
                PM_CFG_GET_UINT(vlan_key.vid, argv[1], para->p_rs);
                p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
                if (NULL == p_db_vlan)
                {
                    CFG_CONFLICT_RET("Vlan %u does not exist", vlan_key.vid);
                }
                
                sal_memset(&pvlan_key, 0, sizeof(pvlan_key));
                PM_CFG_GET_UINT(pvlan_key.primary_vid, argv[1], para->p_rs);
                p_db_pvlan = tbl_pvlan_get_pvlan(&pvlan_key);
                if (NULL == p_db_pvlan)
                {
                    CFG_CONFLICT_RET("Vlan %u is not configured as private vlan", pvlan_key.primary_vid);
                }
                
                _if_mgr_cmd_process_pvlan_vlan_show(p_db_pvlan, para);
            }
            else if (0 == sal_strcmp(argv[0], "interface"))
            {
                sal_memset(&if_key, 0, sizeof(if_key));
                sal_strncpy(if_key.name, argv[1], IFNAME_SIZE);
                p_db_if = tbl_interface_get_interface(&if_key);
                IFNAME_ETH2FULL(argv[1], ifname_ext);
                if (NULL == p_db_if)
                {
                    CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
                }
                if (GLB_IF_PVLAN_TYPE_NONE == p_db_if->pvlan_type)
                {
                    CFG_INVALID_PARAM_RET("Interface %s is not configured as private vlan", ifname_ext);
                }
                _if_mgr_cmd_process_pvlan_interface_show(p_db_if, para);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");    
        }
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");  
        break;
    }

    return PM_E_NONE;
}

static int32
_if_mgr_cmd_process_pfc_show_banner(FILE *fp)
{
    sal_fprintf(fp,"Port        PFC-enable        PFC-enable on priority    RxPause   TxPause\n");
    sal_fprintf(fp,"            admin   oper      admin       oper                           \n");
    sal_fprintf(fp,"--------------------------------------------------------------------------\n");
    
    return PM_E_NONE;
}

static int32
 _if_mgr_cmd_process_pfc_show_one(tbl_interface_t* p_db_if, FILE *fp)
{    
    char ifname_ext[IFNAME_EXT_SIZE];
    char pri_str[9];
    char pri_str_admin[9];
    char pri_str_operate[9]; 
    char rxpause_str[UINT64_STR_LEN];
    char txpause_str[UINT64_STR_LEN];
    int i = 0;
    int count = 0;
   
    sal_memset(pri_str, 0, sizeof(pri_str));
    sal_memset(pri_str_admin, 0, sizeof(pri_str_admin));
    sal_memset(pri_str_operate, 0, sizeof(pri_str_operate));

    tbl_port_stats_t* p_db_ps = NULL;
    tbl_port_stats_key_t port_stats_key;
    sal_memset(&port_stats_key, 0, sizeof(port_stats_key));
    port_stats_key.ifindex = p_db_if->ifindex;
    p_db_ps = tbl_port_stats_get_port_stats(&port_stats_key);
    if (NULL == p_db_ps)
    {
        return PM_E_NONE;
    }
    cds_uint64_to_str(p_db_ps->pause_rcv, rxpause_str);
    cds_uint64_to_str(p_db_ps->pause_send, txpause_str);
    
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    if (!p_db_if->pfc_admin)
    {
         sal_fprintf(fp, "%-12s%-8s%-10s%-12s%-14s%-10s%-10s\n", ifname_ext, "off","off","off","off", rxpause_str, txpause_str);
    }
    else
    {
        for (i = 0; i < 8; i++)
        {
           if (GLB_IS_BIT_SET(p_db_if->pfc_priority, i))
           {
               sal_sprintf (pri_str, "%d", i);
               sal_strcat (pri_str_admin, pri_str);
               sal_sprintf (pri_str, "%d", i);
               sal_strcat (pri_str_operate, pri_str);
               count++;
           }   
        }
        if (count == 0)
        {
           sal_strcat (pri_str_admin, "on");
           sal_strcat (pri_str_operate, "on");
        }
        sal_fprintf(fp, "%-12s%-8s%-10s%-12s%-14s%-10s%-10s\n", ifname_ext, "on","on", pri_str_admin, pri_str_operate, rxpause_str, txpause_str);
    }
    
    return PM_E_NONE;
}

static int32
 _if_mgr_cmd_process_pfc_show(tbl_interface_t* p_db_if, cfg_cmd_para_t *para)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    bool banner = TRUE;
    
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
     
    if (p_db_if)
    {
        _if_mgr_cmd_process_pfc_show_banner(fp);
        _if_mgr_cmd_process_pfc_show_one(p_db_if, fp);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
        {
            if (banner == TRUE)
            {
                _if_mgr_cmd_process_pfc_show_banner(fp);
            }
            banner = FALSE;
            _if_mgr_cmd_process_pfc_show_one(p_db_if, fp);
        }
    }
    
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
if_mgr_cmd_process_pfc_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_interface_t* p_db_if = NULL;
    tbl_interface_key_t  if_key;
    char ifname_ext[IFNAME_EXT_SIZE];
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (1 == argc)
        {
            _if_mgr_cmd_process_pfc_show(NULL, para);
        }
        else if (2 == argc)
        {
            sal_memset(&if_key, 0, sizeof(if_key));
            sal_strncpy(if_key.name, argv[1], IFNAME_SIZE);
            p_db_if = tbl_interface_get_interface(&if_key);
            if (NULL == p_db_if)
            {
                IFNAME_ETH2FULL(argv[1], ifname_ext);
                CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
            } 
            _if_mgr_cmd_process_pfc_show(p_db_if, para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");    
        }
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");  
        break;
    }

    return PM_E_NONE;
}

