
#include "proto.h"
#include "gen/tbl_cpu_traffic_define.h"
#include "gen/tbl_cpu_traffic.h"
#include "gen/tbl_cpu_traffic_group_define.h"
#include "gen/tbl_cpu_traffic_group.h"
#include "gen/tbl_qos_global_define.h"
#include "gen/tbl_qos_global.h"
#include "gen/tbl_qos_domain_define.h"
#include "gen/tbl_qos_domain.h"
#include "gen/tbl_qos_policer_action_profile_define.h"
#include "gen/tbl_qos_policer_action_profile.h"
#include "gen/tbl_qos_policer_profile_define.h"
#include "gen/tbl_qos_policer_profile.h"
#include "gen/tbl_qos_drop_profile_define.h"
#include "gen/tbl_qos_drop_profile.h"
#include "gen/tbl_qos_queue_shape_profile_define.h"
#include "gen/tbl_qos_queue_shape_profile.h"
#include "gen/tbl_qos_port_shape_profile_define.h"
#include "gen/tbl_qos_port_shape_profile.h"
#include "gen/tbl_copp_cfg_define.h"
#include "gen/tbl_copp_cfg.h"
#include "gen/tbl_qos_mpls_domain_define.h"
#include "gen/tbl_qos_mpls_domain.h"
#include "gen/tbl_qos_queue_smart_buffer_define.h"
#include "gen/tbl_qos_queue_smart_buffer.h"


static copp_reason_info_t cpu_traffic_info[] =
{
    {GLB_CPU_TRAFFIC_ARP,            "arp"          , COPP_REASON_DEFAULT_VALUE_ARP,GLB_CPU_TRAFFIC_GROUP_1, TRUE},
    {GLB_CPU_TRAFFIC_STP,            "bpdu"         , COPP_REASON_DEFAULT_VALUE_BPDU,GLB_CPU_TRAFFIC_GROUP_4, TRUE},
    {GLB_CPU_TRAFFIC_LACP,           "slow-protocol", COPP_REASON_DEFAULT_VALUE_SLOW_PROTO,GLB_CPU_TRAFFIC_GROUP_4, TRUE},
    {GLB_CPU_TRAFFIC_EAPOL,          "eapol"        , COPP_REASON_DEFAULT_VALUE_EAPOL,GLB_CPU_TRAFFIC_GROUP_3, TRUE},
    {GLB_CPU_TRAFFIC_LLDP,           "lldp"         , COPP_REASON_DEFAULT_VALUE_LLDP,GLB_CPU_TRAFFIC_GROUP_3, TRUE},
    {GLB_CPU_TRAFFIC_IGMP,           "igmp"         , COPP_REASON_DEFAULT_VALUE_IGMP,GLB_CPU_TRAFFIC_GROUP_3, TRUE},
    {GLB_CPU_TRAFFIC_ERPS,           "erps"         , COPP_REASON_DEFAULT_VALUE_ERPS,GLB_CPU_TRAFFIC_GROUP_4, TRUE},
    {GLB_CPU_TRAFFIC_DHCP,           "dhcp"         , COPP_REASON_DEFAULT_VALUE_DHCP,GLB_CPU_TRAFFIC_GROUP_3, TRUE},
    {GLB_CPU_TRAFFIC_OSPF,           "ospf"         , COPP_REASON_DEFAULT_VALUE_OSPF,GLB_CPU_TRAFFIC_GROUP_4, TRUE},
    {GLB_CPU_TRAFFIC_PIM,            "pim"          , COPP_REASON_DEFAULT_VALUE_PIM,GLB_CPU_TRAFFIC_GROUP_4, FALSE},
    /*modified by yejl for bug 49548, 2018-12-12*/
    /*modified by yejl for bug 50657, 2019-01-14*/
    {GLB_CPU_TRAFFIC_VRRP,           "vrrp"         , COPP_REASON_DEFAULT_VALUE_VRRP,GLB_CPU_TRAFFIC_GROUP_4, TRUE},
    {GLB_CPU_TRAFFIC_BGP,            "bgp"          , COPP_REASON_DEFAULT_VALUE_BGP,GLB_CPU_TRAFFIC_GROUP_4, FALSE},
    {GLB_CPU_TRAFFIC_COPY_TO_CPU,    "management-traffic"   , COPP_REASON_DEFAULT_VALUE_MAGMT_TRA,GLB_CPU_TRAFFIC_GROUP_5, TRUE},
    {GLB_CPU_TRAFFIC_PACKET_IN,      "packet-in"          , COPP_REASON_DEFAULT_VALUE_PACKET_IN,GLB_CPU_TRAFFIC_GROUP_2, TRUE},
    {GLB_CPU_TRAFFIC_PTP,            "ptp"         , COPP_REASON_DEFAULT_VALUE_PTP,GLB_CPU_TRAFFIC_GROUP_3, TRUE},
    {GLB_CPU_TRAFFIC_TTL_ERROR,      "ttl-expired"  , COPP_REASON_DEFAULT_VALUE_TTL_FAIL,GLB_CPU_TRAFFIC_GROUP_0, TRUE},
    {GLB_CPU_TRAFFIC_L3_MTU_ERROR,   "mtu-fail"  , COPP_REASON_DEFAULT_VALUE_MTU_FAIL,GLB_CPU_TRAFFIC_GROUP_0, TRUE}, 
    {GLB_CPU_TRAFFIC_TELNET,         "telnet"  , COPP_REASON_DEFAULT_VALUE_TELNET,GLB_CPU_TRAFFIC_GROUP_5, TRUE},
    {GLB_CPU_TRAFFIC_SSH,            "ssh"  , COPP_REASON_DEFAULT_VALUE_SSH,GLB_CPU_TRAFFIC_GROUP_5, TRUE},
    {GLB_CPU_TRAFFIC_MLAG,           "mlag"  ,COPP_REASON_DEFAULT_VALUE_MLAG,GLB_CPU_TRAFFIC_GROUP_4, TRUE},
    {GLB_CPU_TRAFFIC_PACKET_L2PRO_PROTOCOL_MAC,         "l2pro-protocol-mac"  , COPP_REASON_DEFAULT_VALUE_L2PRO_PROTOCOL_MAC,GLB_CPU_TRAFFIC_GROUP_4, TRUE},
    {GLB_CPU_TRAFFIC_PACKET_L2PRO_GROUP_MAC,            "l2pro-group-mac"  , COPP_REASON_DEFAULT_VALUE_L2PRO_GROUP_MAC,GLB_CPU_TRAFFIC_GROUP_4, TRUE},    
    {GLB_CPU_TRAFFIC_IPV6_NEIGHBOR_DISCOVERY,           "icmpv6"  , COPP_REASON_DEFAULT_VALUE_ND,GLB_CPU_TRAFFIC_GROUP_1, TRUE},

};

int32
ccs_cfg_build_copp_cfg(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_copp_cfg_t *p_copp_glb = tbl_copp_cfg_get_copp_cfg();
    uint32 has_config = FALSE;
    uint32 has_config_copp = FALSE;
    uint32 reason_idx;
    tbl_cpu_traffic_key_t cpu_traffic_key;
    tbl_cpu_traffic_t * p_cpu_traffic = NULL;
    uint32 group_id;
    tbl_cpu_traffic_group_key_t cpu_traffic_group_key;
    tbl_cpu_traffic_group_t * p_cpu_traffic_group = NULL;

    if(0 != sal_strlen(p_copp_glb->copp_pname) || 
        GLB_CPU_TRAFFIC_DEFAULT_RATE != p_copp_glb->copp_total_rate )
    {
        sal_fprintf(fp, "control-plane\n");
        has_config_copp = TRUE;
        has_config = TRUE;
    }
    if (0 != sal_strlen(p_copp_glb->copp_pname))
    {
        sal_fprintf(fp, " policy input %s\n", p_copp_glb->copp_pname);
        has_config = TRUE;
    }
    
    for (reason_idx = 0; reason_idx < (sizeof(cpu_traffic_info)/sizeof(copp_reason_info_t)); reason_idx ++)
    {
        sal_memset(&cpu_traffic_key, 0, sizeof(cpu_traffic_key));
        cpu_traffic_key.reason_id = cpu_traffic_info[reason_idx].reason_id;
        p_cpu_traffic = tbl_cpu_traffic_get_cpu_traffic(&cpu_traffic_key);
        if (NULL == p_cpu_traffic)
        {
            continue;    
        }
        if ((cpu_traffic_info[reason_idx].default_rate != p_cpu_traffic->rate ||
            cpu_traffic_info[reason_idx].default_class_id != p_cpu_traffic->class_id)
            && TRUE ==  cpu_traffic_info[reason_idx].visable)
        {
            if (FALSE == has_config_copp)
            {
                sal_fprintf(fp, "control-plane\n");
                has_config_copp = TRUE;
            }
            sal_fprintf(fp, " reason %s rate %"PRIu64"\n", 
                cpu_traffic_info[reason_idx].reason_map_str, 
                p_cpu_traffic->rate);
            has_config = TRUE;
        }
    }

    for (group_id = GLB_CPU_TRAFFIC_GROUP_0; group_id < GLB_CPU_TRAFFIC_GROUP_MAX; group_id ++)
    {
        sal_memset(&cpu_traffic_group_key, 0, sizeof(cpu_traffic_group_key));
        cpu_traffic_group_key.class_id = group_id;
        p_cpu_traffic_group = tbl_cpu_traffic_group_get_cpu_traffic_group(&cpu_traffic_group_key);
        if (NULL == p_cpu_traffic_group)
        {
            continue;
        }
        if ((GLB_CPU_TRAFFIC_DEFAULT_RATE != p_cpu_traffic_group->rate && group_id != GLB_CPU_TRAFFIC_GROUP_0)||
            (GLB_CPU_TRAFFIC_CLASS0_DEFAULT_RATE != p_cpu_traffic_group->rate && group_id == GLB_CPU_TRAFFIC_GROUP_0))
        {
            if (FALSE == has_config_copp)
            {
                sal_fprintf(fp, "control-plane\n");
                has_config_copp = TRUE;
            }
            sal_fprintf(fp, " class %d rate %"PRIu64"\n", group_id, p_cpu_traffic_group->rate);
            has_config = TRUE;
        }
    }

    if (GLB_CPU_TRAFFIC_DEFAULT_RATE != p_copp_glb->copp_total_rate)
    {
        sal_fprintf(fp, " total rate %d\n", p_copp_glb->copp_total_rate);
        has_config = TRUE;
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}


int32
ccs_cfg_build_cpu_traffic(FILE* fp, cfg_cmd_para_t *para)
{    
    return PM_E_NONE;
}

int32
ccs_cfg_build_cpu_traffic_group(FILE* fp, cfg_cmd_para_t *para)
{
#if 0
    uint32 group_id;
    tbl_cpu_traffic_group_key_t cpu_traffic_group_key;
    tbl_cpu_traffic_group_t * p_cpu_traffic_group;
    uint32 has_config = FALSE;
    
    for(group_id = GLB_CPU_TRAFFIC_GROUP_LOW; group_id < GLB_CPU_TRAFFIC_GROUP_MAX; group_id ++)
    {
        cpu_traffic_group_key.class_id = group_id;
        p_cpu_traffic_group = tbl_cpu_traffic_group_get_cpu_traffic_group(&cpu_traffic_group_key);
        if(GLB_CPU_TRAFFIC_DEFAULT_RATE != p_cpu_traffic_group->rate)
        {
            sal_fprintf(fp, "cpu-traffic-limit class %d rate %"PRIu64"\n", group_id, p_cpu_traffic_group->rate);
            has_config = TRUE;
        }
        if (has_config)
        {
            sal_fprintf(fp, "!\n");
        }
    }
#endif
    return PM_E_NONE;
}

int32
ccs_cfg_build_qos_global(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_qos_global_t *p_qos_glb = tbl_qos_global_get_qos_global();
    uint32 has_config = FALSE;

    if(!p_qos_glb->qos_enable)
    {
        sal_fprintf(fp, "qos global\n");  
        sal_fprintf(fp, " no qos enable\n");
        has_config = TRUE;
    }
    if(p_qos_glb->port_policer_first_enable)
    {
        if(!has_config)
        {
            sal_fprintf(fp, "qos global\n");  
            has_config = TRUE;
        }
        sal_fprintf(fp, " port-policer first enable\n");
    }
    if (!p_qos_glb->qos_ipg_enable)
    {
        if (!has_config)
        {
            sal_fprintf(fp, "qos global\n");
            has_config = TRUE;
        }
        sal_fprintf(fp, " qos ipg disable\n");
    }
    if (p_qos_glb->buffer_event_enable)
    {
        sal_fprintf(fp, "hardware monitor buffer event enable\n"); 
    } 
    if ((p_qos_glb->buffer_event_high != GLB_BUFFER_EVENT_HIGH_THRESHOLD_DEFAULT)
        ||(p_qos_glb->buffer_event_low!=GLB_BUFFER_EVENT_LOW_THRESHOLD_DEFAULT))
    {
        sal_fprintf(fp, "hardware monitor buffer event threshold low %u high %u\n", p_qos_glb->buffer_event_low, p_qos_glb->buffer_event_high); 
    }
    
    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_qos_mpls_domain_dump_one(tbl_qos_mpls_domain_t *p_qos_mpls_domain, tbl_iter_args_t *pargs)
{
    FILE *fp = pargs->argv[0];

    sal_fprintf(fp, "qos mpls-label %d exp-domain %d\n", p_qos_mpls_domain->key.mpls_label, p_qos_mpls_domain->exp_domain);
    
    return PM_E_NONE;
}

/*modified by wanh for bug 52727, 2019-09-16*/
int32
ccs_cfg_build_qos_mpls_domain(FILE* fp, cfg_cmd_para_t *para)
{

	tbl_qos_mpls_domain_t * p_qos_mpls_domain = NULL;
	int i = 0;
	char buf[MAX_CMD_STR_LEN];
	tbl_qos_mpls_domain_master_t *p_master = tbl_qos_mpls_domain_get_master();
	ctclib_slistnode_t *listnode = NULL;
	
	int32 exp_domain[7] = {0, 0, 0, 0, 0, 0, 0};
	int32 exp_domain_cnt = 0;
	
	mpls_label_bmp_t mpls_label_bmp[7];	
	for (i = 0; i < 7; i++)
	{
		sal_memset(mpls_label_bmp[i], 0, sizeof(mpls_label_bmp[i]));		
	}
	
	CTCLIB_SLIST_LOOP(p_master->qos_mpls_domain_list, p_qos_mpls_domain, listnode)
	{
		if (NULL == p_qos_mpls_domain)
		{
			continue;
		}
		for (i = 0; i < 7; i++)
		{
			if (p_qos_mpls_domain->exp_domain == i)
			{
				GLB_BMP_SET(mpls_label_bmp[i], p_qos_mpls_domain->key.mpls_label);
				exp_domain[i] ++;

				exp_domain_cnt++;
			}		
		}
	}

	if(0 != exp_domain_cnt)
	{
		for (i = 0; i < 7; i++)
		{
			if (exp_domain[i] > 0 )
			{
				if (exp_domain[i] == 1)
				{
					/*no range */
					sal_fprintf(fp, "qos mpls-label %s exp-domain %d\n", cdb_mpls_label_bitmap_val2str(buf, MAX_CMD_STR_LEN, mpls_label_bmp[i], GLB_MPLS_LABEL_BMP_WORD_MAX), i);
					sal_fprintf(fp, "!\n");
				}
				else
				{
					/*range*/
					sal_fprintf(fp, "qos mpls-label range %s exp-domain %d\n", cdb_mpls_label_bitmap_val2str(buf, MAX_CMD_STR_LEN, mpls_label_bmp[i], GLB_MPLS_LABEL_BMP_WORD_MAX), i);
					sal_fprintf(fp, "!\n");
				}
			}
		}
	}
	
    return PM_E_NONE;
}
/*wanh end*/

/*added by yejl for bug 54031, 2019-11-14*/
int32
ccs_cfg_build_qos_queue_smart_buffer(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_qos_queue_smart_buffer_t * p_qos_queue_sm_buffer = NULL;
    tbl_qos_queue_smart_buffer_key_t queue_smart_buffer_key;
    char buf[MAX_CMD_STR_LEN];
    uint32_t queue_bmp[GLB_QOS_QUEUE_SMART_BUFFER_NUM];
    int32 queue_cnt = 0;
    int i = 0;

    sal_memset(&queue_bmp, 0, sizeof(queue_bmp));
    for (i = 0; i < 7; i ++)
    {
        sal_memset(&queue_smart_buffer_key, 0, sizeof(queue_smart_buffer_key));
        queue_smart_buffer_key.queue_id = i;
        p_qos_queue_sm_buffer = tbl_qos_queue_smart_buffer_get_qos_queue_smart_buffer(&queue_smart_buffer_key);
        if (p_qos_queue_sm_buffer)
        {
            if (!p_qos_queue_sm_buffer->smart_buffer_enable)
            {
                GLB_BMP_SET(queue_bmp, i);
                queue_cnt ++;
            }
        }
    }
    if (0 != queue_cnt)
    {
        if (1 == queue_cnt)
        {
            sal_fprintf(fp, "no qos queue %s smart-buffer enable\n", cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, queue_bmp, GLB_QOS_QUEUE_SMART_BUFFER_NUM));
            sal_fprintf(fp, "!\n");
        }
        else
        {
            sal_fprintf(fp, "no qos queue range %s smart-buffer enable\n", cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, queue_bmp, GLB_QOS_QUEUE_SMART_BUFFER_NUM));
            sal_fprintf(fp, "!\n");
        }
    }
    return PM_E_NONE;
}
/*ended*/

int32
ccs_cfg_build_qos_domain(FILE* fp, cfg_cmd_para_t *para)
{
    uint32 cos_index = 0;
    uint32 exp_index = 0;
    uint32 dscp_index = 0;
    uint32 tc_index = 0;
    uint32 color_index = 0;
    uint32 domain_index = 0;
    tbl_qos_domain_key_t qos_domain_key;
    tbl_qos_domain_t* p_qos_domain_db = NULL;
    uint32 domain_has_config = FALSE;
    uint32 has_config = FALSE;
    char *color_str[] = {"none", "red", "yellow", "green"};

    sal_memset(&qos_domain_key, 0x0, sizeof(tbl_qos_domain_key_t));

    for(domain_index = 0; domain_index < 8; domain_index ++)
    {
        qos_domain_key.id = domain_index;
        p_qos_domain_db = tbl_qos_domain_get_qos_domain(&qos_domain_key);
        if(NULL == p_qos_domain_db)
        {
            continue;
        }
    #ifdef USW
        /*print cos domain */
        for(cos_index =0; cos_index < GLB_QOS_COS_NUM; cos_index ++)
        {
            if((p_qos_domain_db->cos2tc[cos_index] != cos_index) || (p_qos_domain_db->cos2color[cos_index] != GLB_QOS_COLOR_GREEN))
            {
                if(!domain_has_config)
                {
                    sal_fprintf(fp, "qos cos-domain %d\n", domain_index);
                    domain_has_config = TRUE;
                    has_config ++;
                }
                sal_fprintf(fp, " cos %d to tc %d color %s\n", cos_index, p_qos_domain_db->cos2tc[cos_index], color_str[p_qos_domain_db->cos2color[cos_index]]);
            }
        }
        for(tc_index = 0; tc_index < GLB_QOS_TC_NUM; tc_index ++)
        {
            for(color_index = 0; color_index < GLB_QOS_COLOR_MAX; color_index ++)
            {
                if(p_qos_domain_db->tc_color2cos[(tc_index << 2) + color_index] != tc_index)
                {
                    if(!domain_has_config)
                    {
                        sal_fprintf(fp, "qos cos-domain %d\n", domain_index);
                        domain_has_config = TRUE;
                        has_config ++;
                    }
                    sal_fprintf(fp, " tc %d color %s to cos %d\n", tc_index, color_str[color_index], p_qos_domain_db->tc_color2cos[(tc_index << 2) + color_index]);
                }
            }
        }
        if (has_config)
        {
            sal_fprintf (fp, "  exit\n");
            sal_fprintf(fp, "!\n");
        }

        /* print exp domain */
        domain_has_config = FALSE;
        has_config = FALSE;
        for(exp_index =0; exp_index < GLB_QOS_EXP_NUM; exp_index ++)
        {
            if((p_qos_domain_db->exp2tc[exp_index] != exp_index) || (p_qos_domain_db->exp2color[exp_index] != GLB_QOS_COLOR_GREEN))
            {
                if(!domain_has_config)
                {
                    sal_fprintf(fp, "qos exp-domain %d\n", domain_index);
                    domain_has_config = TRUE;
                    has_config ++;
                }
                sal_fprintf(fp, " exp %d to tc %d color %s\n", exp_index, p_qos_domain_db->exp2tc[exp_index], color_str[p_qos_domain_db->exp2color[exp_index]]);
            }
        }
        for(tc_index = 0; tc_index < GLB_QOS_TC_NUM; tc_index ++)
        {
            for(color_index = 0; color_index < GLB_QOS_COLOR_MAX; color_index ++)
            {
                if(p_qos_domain_db->tc_color2exp[(tc_index << 2) + color_index] != tc_index)
                {
                    if(!domain_has_config)
                    {
                        sal_fprintf(fp, "qos exp-domain %d\n", domain_index);
                        domain_has_config = TRUE;
                        has_config ++;
                    }
                    sal_fprintf(fp, " tc %d color %s to exp %d\n", tc_index, color_str[color_index], p_qos_domain_db->tc_color2exp[(tc_index << 2) + color_index]);
                }
            }
        }
        if (has_config)
        {
            sal_fprintf (fp, "  exit\n");
            sal_fprintf(fp, "!\n");
        }

        /* printf dscp domain */
        domain_has_config = FALSE;
        has_config = FALSE;
        for(dscp_index = 0; dscp_index < GLB_QOS_DSCP_NUM; dscp_index ++)
        {
            if((p_qos_domain_db->dscp2tc[dscp_index] != (dscp_index/8)) || (p_qos_domain_db->dscp2color[dscp_index] != GLB_QOS_COLOR_GREEN))
            {
                if(!domain_has_config)
                {
                    sal_fprintf(fp, "qos dscp-domain %d\n", domain_index);
                    domain_has_config = TRUE;
                    has_config ++;
                }
                sal_fprintf(fp, " dscp %d to tc %d color %s\n", dscp_index, p_qos_domain_db->dscp2tc[dscp_index], color_str[p_qos_domain_db->dscp2color[dscp_index]]);
            }
        }
        
        for(tc_index = 0; tc_index < GLB_QOS_TC_NUM; tc_index ++)
        {
            for(color_index = 0; color_index < GLB_QOS_COLOR_MAX; color_index ++)
            {
                if(p_qos_domain_db->tc_color2dscp[(tc_index << 2) + color_index] !=( tc_index * 8))
                {
                    if(!domain_has_config)
                    {
                        sal_fprintf(fp, "qos dscp-domain %d\n", domain_index);
                        domain_has_config = TRUE;
                        has_config ++;
                    }
                    sal_fprintf(fp, " tc %d color %s to dscp %d\n", tc_index, color_str[color_index], p_qos_domain_db->tc_color2dscp[(tc_index << 2) + color_index]);
                }
            }
        }
        if (has_config)
        {
            sal_fprintf (fp, "  exit\n");
            sal_fprintf(fp, "!\n");
        }
    #else
        /*print cos to tc and color*/
        for(cos_index =0; cos_index < GLB_QOS_COS_NUM; cos_index ++)
        {
            if((p_qos_domain_db->cos2tc[cos_index] != cos_index) || (p_qos_domain_db->cos2color[cos_index] != GLB_QOS_COLOR_GREEN))
            {
                if(!domain_has_config)
                {
                    sal_fprintf(fp, "qos domain %d\n", domain_index);
                    domain_has_config = TRUE;
                    has_config ++;
                }
                sal_fprintf(fp, " cos %d to tc %d color %s\n", cos_index, p_qos_domain_db->cos2tc[cos_index], color_str[p_qos_domain_db->cos2color[cos_index]]);
            }
        }
        for(exp_index =0; exp_index < GLB_QOS_EXP_NUM; exp_index ++)
        {
            if((p_qos_domain_db->exp2tc[exp_index] != exp_index) || (p_qos_domain_db->exp2color[exp_index] != GLB_QOS_COLOR_GREEN))
            {
                if(!domain_has_config)
                {
                    sal_fprintf(fp, "qos domain %d\n", domain_index);
                    domain_has_config = TRUE;
                    has_config ++;
                }
                sal_fprintf(fp, " exp %d to tc %d color %s\n", exp_index, p_qos_domain_db->exp2tc[exp_index], color_str[p_qos_domain_db->exp2color[exp_index]]);
            }
        }
        for(dscp_index = 0; dscp_index < GLB_QOS_DSCP_NUM; dscp_index ++)
        {
            if((p_qos_domain_db->dscp2tc[dscp_index] != (dscp_index/8)) || (p_qos_domain_db->dscp2color[dscp_index] != GLB_QOS_COLOR_GREEN))
            {
                if(!domain_has_config)
                {
                    sal_fprintf(fp, "qos domain %d\n", domain_index);
                    domain_has_config = TRUE;
                    has_config ++;
                }
                sal_fprintf(fp, " dscp %d to tc %d color %s\n", dscp_index, p_qos_domain_db->dscp2tc[dscp_index], color_str[p_qos_domain_db->dscp2color[dscp_index]]);
            }
        }
        for(tc_index = 0; tc_index < GLB_QOS_TC_NUM; tc_index ++)
        {
            for(color_index = 0; color_index < GLB_QOS_COLOR_MAX; color_index ++)
            {
                if(p_qos_domain_db->tc_color2cos[(tc_index << 2) + color_index] != tc_index)
                {
                    if(!domain_has_config)
                    {
                        sal_fprintf(fp, "qos domain %d\n", domain_index);
                        domain_has_config = TRUE;
                        has_config ++;
                    }
                    sal_fprintf(fp, " tc %d color %s to cos %d\n", tc_index, color_str[color_index], p_qos_domain_db->tc_color2cos[(tc_index << 2) + color_index]);
                }
            }
        }
        for(tc_index = 0; tc_index < GLB_QOS_TC_NUM; tc_index ++)
        {
            for(color_index = 0; color_index < GLB_QOS_COLOR_MAX; color_index ++)
            {
                if(p_qos_domain_db->tc_color2exp[(tc_index << 2) + color_index] != tc_index)
                {
                    if(!domain_has_config)
                    {
                        sal_fprintf(fp, "qos domain %d\n", domain_index);
                        domain_has_config = TRUE;
                        has_config ++;
                    }
                    sal_fprintf(fp, " tc %d color %s to exp %d\n", tc_index, color_str[color_index], p_qos_domain_db->tc_color2exp[(tc_index << 2) + color_index]);
                }
            }
        }
        for(tc_index = 0; tc_index < GLB_QOS_TC_NUM; tc_index ++)
        {
            for(color_index = 0; color_index < GLB_QOS_COLOR_MAX; color_index ++)
            {
                if(p_qos_domain_db->tc_color2dscp[(tc_index << 2) + color_index] !=( tc_index * 8))
                {
                    if(!domain_has_config)
                    {
                        sal_fprintf(fp, "qos domain %d\n", domain_index);
                        domain_has_config = TRUE;
                        has_config ++;
                    }
                    sal_fprintf(fp, " tc %d color %s to dscp %d\n", tc_index, color_str[color_index], p_qos_domain_db->tc_color2dscp[(tc_index << 2) + color_index]);
                }
            }
        }
        if (has_config)
        {
            sal_fprintf (fp, "  exit\n");
            sal_fprintf(fp, "!\n");
        }
    #endif
        has_config = FALSE;
        domain_has_config = FALSE;
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_qos_policer_action_profile(FILE* fp, cfg_cmd_para_t *para)
{
    uint32 id_index = 0;
    tbl_qos_policer_action_profile_key_t qos_policer_action_profile_key;
    tbl_qos_policer_action_profile_t* p_qos_policer_action_profile_db;

    sal_memset(&qos_policer_action_profile_key, 0x0, sizeof(tbl_qos_policer_action_profile_key_t));

    for(id_index = 0; id_index < GLB_QOS_POLICER_ACTION_PROFILE_NUM; id_index++)
    {
        qos_policer_action_profile_key.id = id_index;
        p_qos_policer_action_profile_db = tbl_qos_policer_action_profile_get_qos_policer_action_profile(&qos_policer_action_profile_key);
        if(NULL == p_qos_policer_action_profile_db)
        {
            continue;
        }
        sal_fprintf(fp,"qos policer-action-profile %s\n", p_qos_policer_action_profile_db->name);
        if (p_qos_policer_action_profile_db->confirm_action.drop_packet)
        {
            sal_fprintf(fp," set confirm-action drop\n");
        }
        if (p_qos_policer_action_profile_db->confirm_action.tc_valid)
        {
            sal_fprintf(fp," set confirm-action tc %d\n", p_qos_policer_action_profile_db->confirm_action.tc);
        }
        
        if (p_qos_policer_action_profile_db->exceed_action.drop_packet)
        {
            sal_fprintf(fp," set exceed-action drop\n");
        }
        if (p_qos_policer_action_profile_db->exceed_action.tc_valid)
        {
            sal_fprintf(fp," set exceed-action tc %d\n", p_qos_policer_action_profile_db->exceed_action.tc);
        }
        
        if (!p_qos_policer_action_profile_db->violate_action.drop_packet)
        {
            sal_fprintf(fp," set violate-action transmit\n");
        }
        if (p_qos_policer_action_profile_db->violate_action.tc_valid)
        {
            sal_fprintf(fp," set violate-action tc %d\n", p_qos_policer_action_profile_db->violate_action.tc);
        }

        sal_fprintf (fp, "  exit\n");
        sal_fprintf(fp,"!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_qos_policer_profile(FILE* fp, cfg_cmd_para_t *para)
{
    uint32 policer_id_index = 0;
    tbl_qos_policer_profile_key_t qos_policer_profile_key;
    tbl_qos_policer_profile_t* p_qos_policer_profile_db;
    char* mode_str[] = 
    {
        "rfc2697",
        "rfc2698",
        "rfc4115",
        "bwp"
    };
    char* color_mode_str[] = 
    {
        "color-blind",
        "color-aware"
    };   
    sal_memset(&qos_policer_profile_key, 0x0, sizeof(tbl_qos_policer_profile_key_t));

    for(policer_id_index = 0; policer_id_index < GLB_QOS_POLICER_PROFILE_NUM; policer_id_index++)
    {
        qos_policer_profile_key.id = policer_id_index;
        p_qos_policer_profile_db = tbl_qos_policer_profile_get_qos_policer_profile(&qos_policer_profile_key);
        if(NULL == p_qos_policer_profile_db)
        {
            continue;
        }
        sal_fprintf(fp,"qos policer-profile %s\n", p_qos_policer_profile_db->name);
        sal_fprintf(fp," mode %s", mode_str[p_qos_policer_profile_db->algorithm_mode]);
        if (p_qos_policer_profile_db->is_pps)
        {
            sal_fprintf(fp, " pps");
        }
        if(GLB_QOS_POLICER_MODE_RFC4115 == p_qos_policer_profile_db->algorithm_mode)
        {
            sal_fprintf(fp," %s cir %"PRIu64" cbs %d eir %"PRIu64" ebs %d ", color_mode_str[p_qos_policer_profile_db->color_mode], p_qos_policer_profile_db->cir,
                p_qos_policer_profile_db->cbs, p_qos_policer_profile_db->eir_or_pir, p_qos_policer_profile_db->ebs_or_pbs);

            if (GLB_FLAG_ISSET(p_qos_policer_profile_db->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
            {
                sal_fprintf(fp,"stats");
            }
        }
        if(GLB_QOS_POLICER_MODE_RFC2697 == p_qos_policer_profile_db->algorithm_mode)
        {
            sal_fprintf(fp," %s cir %"PRIu64" cbs %d ebs %d ", color_mode_str[p_qos_policer_profile_db->color_mode], p_qos_policer_profile_db->cir,
                p_qos_policer_profile_db->cbs, p_qos_policer_profile_db->ebs_or_pbs);
            if (GLB_FLAG_ISSET(p_qos_policer_profile_db->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
            {
                sal_fprintf(fp,"stats");
            }
        }
        if ('\0' != p_qos_policer_profile_db->action_profile_name[0])
        {
            sal_fprintf(fp,"\n");
            sal_fprintf(fp," set actoin-profile %s", p_qos_policer_profile_db->action_profile_name);
        }
        sal_fprintf(fp,"\n");
        sal_fprintf (fp, "  exit\n");
        sal_fprintf(fp,"!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_qos_drop_profile(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_qos_drop_profile_key_t qos_drop_profile_key;
    tbl_qos_drop_profile_t* p_qos_drop_profile_db;
    uint32 drop_id_index = 0;
    uint32 flag = 0;
    uint32 duet2_flag = 0;

#ifdef GREATBELT
        flag = 1;
#endif
#ifdef USW
        flag = 1;
        duet2_flag = 1;
#endif
    sal_memset(&qos_drop_profile_key, 0x0, sizeof(tbl_qos_drop_profile_key_t));
    for(drop_id_index = 2; drop_id_index < GLB_QOS_DROP_PROFILE_NUM; drop_id_index++)
    {
        qos_drop_profile_key.id = drop_id_index;
        p_qos_drop_profile_db = tbl_qos_drop_profile_get_qos_drop_profile(&qos_drop_profile_key);
        if(NULL == p_qos_drop_profile_db)
        {
            continue;
        }
        sal_fprintf(fp,"qos drop-profile %s\n", p_qos_drop_profile_db->name);
        if (GLB_FLAG_ISSET(p_qos_drop_profile_db->flags, GLB_DROP_PROFILE_FLAGS_GREEN_MAX_THRD))
        {
            sal_fprintf(fp," green maximum %d", p_qos_drop_profile_db->green_max_threashold);
            if (duet2_flag)
            {
                sal_fprintf(fp," minimun %d", p_qos_drop_profile_db->green_min_threashold);
            }
            if (flag)
            {
                if(GLB_FLAG_ISSET(p_qos_drop_profile_db->flags, GLB_DROP_PROFILE_FLAGS_GREEN_PROB))
                {
                    sal_fprintf(fp," probability %d", p_qos_drop_profile_db->green_probability);
                }
            }
            sal_fprintf(fp, "\n");
        }
        if (GLB_FLAG_ISSET(p_qos_drop_profile_db->flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MAX_THRD))
        {
            sal_fprintf(fp," yellow maximum %d", p_qos_drop_profile_db->yellow_max_threashold);
            if (duet2_flag)
            {
                sal_fprintf(fp," minimun %d", p_qos_drop_profile_db->yellow_min_threashold);
            }
            if (flag)
            {
                if(GLB_FLAG_ISSET(p_qos_drop_profile_db->flags, GLB_DROP_PROFILE_FLAGS_YELLOW_PROB))
                {
                    sal_fprintf(fp," probability %d", p_qos_drop_profile_db->yellow_probability);
                }
            }
            sal_fprintf(fp, "\n");
        }
        if (GLB_FLAG_ISSET(p_qos_drop_profile_db->flags, GLB_DROP_PROFILE_FLAGS_RED_MAX_THRD))
        {
            sal_fprintf(fp," red maximum %d", p_qos_drop_profile_db->red_max_threashold);
            if (duet2_flag)
            {
                sal_fprintf(fp," minimun %d", p_qos_drop_profile_db->red_min_threashold);
            }
            if (flag)
            {
                if(GLB_FLAG_ISSET(p_qos_drop_profile_db->flags, GLB_DROP_PROFILE_FLAGS_RED_PROB))
                {
                    sal_fprintf(fp," probability %d", p_qos_drop_profile_db->red_probability);
                }
            }
            sal_fprintf(fp, "\n");
        }
        sal_fprintf (fp, "  exit\n");
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_qos_scheduler(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_qos_queue_shape_profile_key_t qos_queue_shape_profile_key;
    tbl_qos_queue_shape_profile_t* p_qos_queue_shape_profile_db;
    uint32 scheduler_id_index = 0;
    char* scheduler_mode_str[] = 
    {
        "sp",
        "dwrr"
    };   
    sal_memset(&qos_queue_shape_profile_key, 0x0, sizeof(tbl_qos_queue_shape_profile_key_t));
    for(scheduler_id_index = 1; scheduler_id_index < GLB_QOS_SHAPE_PROFILE_NUM; scheduler_id_index++)
    {
        qos_queue_shape_profile_key.id = scheduler_id_index;
        p_qos_queue_shape_profile_db = tbl_qos_queue_shape_profile_get_qos_queue_shape_profile(&qos_queue_shape_profile_key);
        if(NULL == p_qos_queue_shape_profile_db)
        {
            continue;
        }
        sal_fprintf(fp,"qos scheduler-profile %s\n", p_qos_queue_shape_profile_db->name);
        if(GLB_FLAG_ISSET(p_qos_queue_shape_profile_db->flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_MODE))
            sal_fprintf(fp," mode %s\n", scheduler_mode_str[p_qos_queue_shape_profile_db->mode]);
    #if 0
        if(SCHEDULER_MODE_DWRR == p_qos_queue_shape_profile_db->mode)
        {
            if(GLB_FLAG_ISSET(p_qos_queue_shape_profile_db->flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_WEIGHT))
                sal_fprintf(fp," weight %d\n", p_qos_queue_shape_profile_db->queue_weight);
        }
    #endif
    if(GLB_FLAG_ISSET(p_qos_queue_shape_profile_db->flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_WEIGHT))
                sal_fprintf(fp," weight %d\n", p_qos_queue_shape_profile_db->queue_weight);

        if(GLB_FLAG_ISSET(p_qos_queue_shape_profile_db->flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_PIR))
             sal_fprintf(fp," pir %d\n", p_qos_queue_shape_profile_db->pir);
        sal_fprintf (fp, "  exit\n");
        sal_fprintf(fp,"!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_qos_port_shape_profile(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_qos_port_shape_profile_key_t qos_port_shape_profile_key;
    tbl_qos_port_shape_profile_t* p_qos_port_shape_profile_db;
    uint32 profile_id_index = 0;
    sal_memset(&qos_port_shape_profile_key, 0x0, sizeof(tbl_qos_port_shape_profile_key_t));
    for(profile_id_index = 1; profile_id_index < GLB_QOS_PORT_SHAPE_PROFILE_NUM; profile_id_index++)
    {
        qos_port_shape_profile_key.id = profile_id_index;
        p_qos_port_shape_profile_db = tbl_qos_port_shape_profile_get_qos_port_shape_profile(&qos_port_shape_profile_key);
        if(NULL == p_qos_port_shape_profile_db)
        {
            continue;
        }
        sal_fprintf(fp,"qos port-shape-profile %s\n", p_qos_port_shape_profile_db->name);
        if(GLB_FLAG_ISSET(p_qos_port_shape_profile_db->flags, GLB_PORT_SHAPE_PROFILE_FLAGS_PIR))
             sal_fprintf(fp," pir %d\n", p_qos_port_shape_profile_db->pir);
        sal_fprintf (fp, "  exit\n");
        sal_fprintf(fp,"!\n");
    }
    return PM_E_NONE;
}

