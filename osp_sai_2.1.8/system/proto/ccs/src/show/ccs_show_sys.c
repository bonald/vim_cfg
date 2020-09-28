#include "proto.h"
#include "lib_tblinc.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"

#define SHOW_SYS_SPEC(string, num1, num2) \
do{\
        sal_fprintf(fp,"    %-38s: %-d/%-d\n", (string), (num1), (num2));\
}while(0)


int32
ccs_cmd_map_sys_spec_type(char **argv, int32 argc, cfg_cmd_para_t *para, int32 *type)
{
    tbl_chassis_t* p_chassis = NULL;
    if (argc)
    {
        if (0 == sal_strcmp(argv[0], "default"))
        {
            *type = GLB_STM_MODE_DEFAULT;
        }
        else if (0 == sal_strcmp(argv[0], "ipv4"))
        {
            *type = GLB_STM_MODE_IPUCV4;
        }
        else if (0 == sal_strcmp(argv[0], "vlan"))
        {
            *type = GLB_STM_MODE_VLAN;
        }
        else if (0 == sal_strcmp(argv[0], "ipv6"))
        {
            *type = GLB_STM_MODE_IPUCV6;
        }
        else if (0 == sal_strcmp(argv[0], "nvgre"))
        {
            *type = GLB_STM_MODE_NVGRE;
        }
        else if (0 == sal_strcmp(argv[0], "l2vpn"))
        {
            *type = GLB_STM_MODE_L2VPN;
        }
        else if (0 == sal_strcmp(argv[0], "openflow-default"))
        {
            *type = GLB_STM_MODE_OF_DEFAULT;
        }
        else if (0 == sal_strcmp(argv[0], "openflow-hybrid"))
        {
            *type = GLB_STM_MODE_OF_HYBRID;
        }
        else if (0 == sal_strcmp(argv[0], "current"))
        {
            p_chassis = tbl_chassis_get_chassis();
            *type = p_chassis->cur_stm_mode;
        }
        else if (0 == sal_strcmp(argv[0], "next"))
        {
            p_chassis = tbl_chassis_get_chassis();
            *type = p_chassis->next_stm_mode;
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid stm profile type: %s", argv[0]);
        }
        return PM_E_NONE;
    }
    CFG_INVALID_PARAM_RET("Stm profile type is not specified");
}



int32
ccs_show_stm(tbl_sys_spec_t *p_sys_spec, FILE *fp, cfg_cmd_para_t *para)
{
    tbl_vlan_master_t* p_vlan_master = tbl_vlan_get_master();
    tbl_arp_master_t * p_arp_master = tbl_arp_get_master();
    tbl_iproute_node_master_t * p_ipfib_master = tbl_iproute_node_get_master();
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    tbl_acl_entry_master_t * p_acl_master = tbl_acl_entry_get_master();

    tbl_acl_config_master_t* p_acl_cfg_master = tbl_acl_config_get_master();
    tbl_ace_config_master_t* p_ace_cfg_master = tbl_ace_config_get_master();
    tbl_vlan_t *p_db_vlan = NULL;
    uint32 vid = 0;
    uint32 num = 0;

    if(!p_vlan_master || !p_arp_master || !p_acl_master)
    {
        return PM_E_NOT_INITED;
    }

    if(GLB_STM_MODE_VLAN == p_sys_spec->key.type)
    {
        sal_fprintf(fp,"%s:\n", "vlan profile:");
    }
    else if(GLB_STM_MODE_IPUCV6 == p_sys_spec->key.type)
    {
        sal_fprintf(fp,"%s:\n", "IPv6 profile:");
    }
    else
    {
        sal_fprintf(fp,"%s:\n", "default profile:");
    }


    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_vlan = p_vlan_master->vlan_array[vid];
        if (NULL == p_db_vlan)
        {
            continue;
        }
        else
        {
            num ++;
        }

    }
    sal_fprintf(fp,"number of Ethernet features:\n");
    SHOW_SYS_SPEC("VLAN forwarding instances", num, p_sys_spec->vlan_instance);


    sal_fprintf(fp,"number of IP unicast features:\n");
    num = p_arp_master->arp_hash->count;
    SHOW_SYS_SPEC("IPv4 host routes", num, p_sys_spec->host_routes);

    num = p_ipfib_master->rn_hash->count;
    SHOW_SYS_SPEC("Indirect IPv4 routes", num, p_sys_spec->indirect_routes);

    num = p_rt_glb->current_ecmp_routes;
    SHOW_SYS_SPEC("IPv4 ecmp groups", num, p_sys_spec->ecmp_routes);

    sal_fprintf(fp,"number of Security features:\n");
    num = p_acl_master->acl_entry_hash->count;
    SHOW_SYS_SPEC("ACL entries", num, p_sys_spec->flow_entries_ingress + p_sys_spec->flow_entries_egress);

    num = p_acl_cfg_master->acl_config_hash->count;
    SHOW_SYS_SPEC("System ACL configure", num, GLB_ACL_CONFIG_MAX);
    num = p_ace_cfg_master->ace_config_hash->count;
    SHOW_SYS_SPEC("System ACE configure", num, GLB_ACE_CONFIG_MAX);

    sal_fprintf (fp, "\n");
    return PM_E_NONE;
}

int32
ccs_cmd_show_stm(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    FILE *fp = NULL;
    tbl_sys_spec_t * p_sys_spec = NULL;
    int32 type = 1000;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        if (NULL == fp)
        {
            return PM_E_NO_MEMORY;
        }
        ccs_cmd_map_sys_spec_type(argv, argc, para, &type);
        if (type < GLB_STM_MODE_MAX)
        {
            p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
            if(p_sys_spec)
            {
                rc = ccs_show_stm(p_sys_spec, fp, para);
                if (rc < 0)
                {
                    sal_fclose(fp);
                    return rc;
                }
            }
        }

        sal_fclose(fp);
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }

    return rc;
}

