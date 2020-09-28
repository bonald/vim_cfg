#include "proto.h"
#include "lib_tblinc.h"
#include "lib_sysctl.h"
#include "route.h"
#include "cdb_const.h"
#include "glb_l2_define.h"
#include "glb_arpacl_define.h"
#include "arp_define.h"
#include "arpinsp_api.h"
#include "arpinsp.h"
#include "arpratelimit.h"
#include "arpratelimit_api.h"
#include "arpacl.h"
#include "arp_debug.h"
#include "arp_ether.h"

#define GLB_ARP_RATE_LIMIT_TIMER_VALUE 30

extern char*
format_time_str(char *pszBuf, size_t nLen, char *pszFormat, time_t nTime);

int32
arpratelimit_get_cur_time_str(char *p_curtime_str)
{
    time_t nCur = sal_time(NULL);
    char cur_time[M_TIME_BUFSIZE] = {'\0'};

    if (NULL == p_curtime_str)
    {
        ARPINSP_PROTO_DEBUG("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_FAIL;
    }
    
    format_time_str(cur_time, M_TIME_BUFSIZE, "%Y-%m-%d %H:%M:%S ", nCur);
    sal_strncpy(p_curtime_str, (char*)cur_time, sal_strlen((char*)cur_time));

    return PM_E_NONE;
}

sal_time_t
_time_boottime_msec(sal_time_t *tp)
{
    struct sal_timeval tv;

    ctc_task_boottime(&tv);
    if (tp)
    {
        *tp = (sal_time_t)(tv.tv_usec / 1000);
    }

    return (sal_time_t)(tv.tv_usec / 1000);
}

/*arp rate limit timer callback function ,default value is 30s*/
void
arpratelimit_timer_cb(void* p_argv)
{
    tbl_route_if_t *p_rt_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
    char cur_time[M_TIME_BUFSIZE] = {'\0'};
    tbl_route_if_master_t *p_master_rt_if = tbl_route_if_get_master();
    tbl_route_global_t * p_rt_glb = tbl_route_global_get_route_global(); 
    ctclib_slistnode_t *listnode = NULL;

    p_rt_glb->arpratelimit_timer = NULL;

    /* loop all l3 port*/
    if (NULL != p_master_rt_if)
    {
        CTCLIB_SLIST_LOOP(p_master_rt_if->if_list, p_rt_if, listnode)
        {
            IFNAME_ETH2FULL(p_rt_if->key.name, ifname_ext);
            
            /* if this port is abnormal ,print log*/
            if (TRUE == p_rt_if->arp_rate_limit_port_abnormal_flag)
            {
                /*print log if abnormal flag is true on flag*/
                arpratelimit_get_cur_time_str(cur_time);

                logid_sys(LOG_ARPRATELIMIT_4_ABNORMAL, ifname_ext, p_rt_if->arp_rate_limit_pkt_curr, cur_time, p_rt_if->arp_rate_limit_pkt_max);
            }
            
            /*clear statistic for all interface*/
            p_rt_if->arp_rate_limit_pkt_curr = 0;
            
            /* set the abnormal flag to false*/
            p_rt_if->arp_rate_limit_port_abnormal_flag = FALSE;
        }
     }
    
    p_rt_glb->arpratelimit_timer = CTC_TASK_ADD_TIME(arpratelimit_timer_cb, NULL, GLB_ARP_RATE_LIMIT_TIMER_VALUE);
    p_rt_glb->arpratelimit_timer_start_time_sec = ctc_time_boottime_sec(NULL);
    p_rt_glb->arpratelimit_timer_start_time_msec = _time_boottime_msec(NULL);

    return;
}

void
arpratelimit_init()
{
    tbl_route_global_t * p_rt_glb = tbl_route_global_get_route_global(); 
    
    p_rt_glb->arpratelimit_timer = CTC_TASK_ADD_TIME(arpratelimit_timer_cb, NULL, GLB_ARP_RATE_LIMIT_TIMER_VALUE);
    
    return;
}

tbl_route_if_t * 
arpratelimit_get_l3_if_from_msg(uint32 vlanid_pkt, tbl_interface_t *p_db_if_input)
{
    tbl_interface_t *pst_if     = NULL;
    tbl_route_if_t *pst_l3_if_tmp = NULL;
    char ifname[IFNAME_SIZE];
    tbl_interface_t *pst_if_cp = p_db_if_input;
    char                ifname_ext[IFNAME_EXT_SIZE];

    ifname[0] = CMD_ZERO_CHAR;
    IFNAME_ETH2FULL(p_db_if_input->key.name, ifname_ext);
    ARPINSP_PKT_DEBUG("name(%s) vlanid_pkt(%d)\n", ifname_ext, vlanid_pkt);

    /*vlanif*/
    if (GLB_IF_MODE_L2 == p_db_if_input->mode && p_db_if_input->brgif)
    {
        sal_sprintf(ifname, "vlan%u", vlanid_pkt);
        pst_if = tbl_interface_get_interface_by_name(ifname);
        if (NULL != pst_if && GLB_IF_TYPE_VLAN_IF == pst_if->hw_type)
        {
            pst_if_cp = pst_if;
        }
    }

    pst_l3_if_tmp = tbl_route_if_get_route_if_by_name(pst_if_cp->key.name);            
    
    return pst_l3_if_tmp;
}

/*called when rcv pkts more than cfg value in 30s*/
int32 
arprateimit_currpktnum_exceed_action(tbl_interface_t *p_db_if, tbl_route_if_t *p_rt_if)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
    char cur_time[M_TIME_BUFSIZE] = {'\0'};
    int32 rc = PM_E_NONE;
        
    if (NULL == p_db_if || NULL == p_rt_if)
    {
        ARPINSP_PKT_DEBUG("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_ARP_RATE_LIMIT_ERR;
    }
    
    /* print log and set abnormal flag*/
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    arpratelimit_get_cur_time_str(cur_time);
    
    logid_sys(LOG_ARPRATELIMIT_4_EXCEED, ifname_ext, p_rt_if->arp_rate_limit_pkt_curr, cur_time, p_rt_if->arp_rate_limit_pkt_max, 
                                                            arpratelimit_exceed_action_str[p_rt_if->arp_rate_limit_violation]);
    
    /*abnormal flag is YES*/
    p_rt_if->arp_rate_limit_port_abnormal_flag = TRUE;

    if (ARP_RATE_LIMIT_ACTION_ERRDISABLE == p_rt_if->arp_rate_limit_violation)
    {
        ARPINSP_PKT_DEBUG("(%s, %d) Ifname %s set errdisable\n", __FUNCTION__, __LINE__, ifname_ext);
        PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_ARP_RATE_LIMIT_ERRDISABLE_OCCUR));
    }
    
    return PM_E_NONE;
}

/*arp rate limit pkt process entry */
int32 
arpratelimit_packet_process(uint32 vlanid_pkt, tbl_interface_t *p_db_if_input)
{
    tbl_route_if_t *p_rt_if = NULL;
    tbl_interface_t *p_db_if = NULL;
    int32 is_l3port = TRUE;
    char ifname_ext[IFNAME_EXT_SIZE];

    /*get l3 port*/
    p_rt_if = arpratelimit_get_l3_if_from_msg(vlanid_pkt, p_db_if_input);
    if (NULL == p_rt_if)
    {
        ARPINSP_PKT_DEBUG("(%s, %d) Cannot get L3 if\n", __FUNCTION__, __LINE__);
        return PM_E_NONE;
    }

    IFNAME_ETH2FULL(p_rt_if->key.name, ifname_ext);
    /*arp rate-limit is enable ?*/
    if (FALSE == p_rt_if->arp_rate_limit_en)
    {
        /*modified by chenc for bug 51010, 2019-02-14*/
        //ARPINSP_PKT_DEBUG("(%s, %d) arp rate limit is disable on %s\n", __FUNCTION__, __LINE__, ifname_ext);
        /*ended chenc*/
        return PM_E_NONE;
    }

    p_db_if = tbl_interface_get_interface_by_name(p_rt_if->key.name);
    if (NULL == p_db_if)
    {
        ARPINSP_PKT_DEBUG("(%s, %d) Interface %s not exist\n", __FUNCTION__, __LINE__, ifname_ext);
        return PM_E_NONE;
    }
        
    is_l3port = arpratelimit_is_arpratelimit_l3_port(p_db_if);
    if (FALSE == is_l3port)
    {
        ARPINSP_PKT_DEBUG("(%s, %d) Interface %s is not routed port or LAG or vlanif\n", __FUNCTION__, __LINE__, ifname_ext);
        return PM_E_NONE;
    }

    /*fix bug 44455: vlanif errdisable*/
    if (ERRDIS_NO_ERRDIS != p_db_if->errdisable_reason && GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
    {
        ARPINSP_PKT_DEBUG("(%s, %d) ifname(%s) already ERRDISABLE , return\n", __FUNCTION__, __LINE__, ifname_ext);
        return PM_E_ARP_RATE_LIMIT_ERR;
    }

    /*pkt num ++*/
    p_rt_if->arp_rate_limit_pkt_curr++;
    
    ARPINSP_PKT_DEBUG("(%s, %d) Ifname(%s) curr++(%d)\n", __FUNCTION__, __LINE__, ifname_ext, p_rt_if->arp_rate_limit_pkt_curr);
    if (TRUE == p_rt_if->arp_rate_limit_port_abnormal_flag)
    {
        ARPINSP_PKT_DEBUG("(%s, %d) ifname(%s) already abnormal ,return\n", __FUNCTION__, __LINE__, ifname_ext);
        return PM_E_ARP_RATE_LIMIT_ERR;
    }
    
    /*is pkt num == max + 1*/
    if ((p_rt_if->arp_rate_limit_pkt_max + 1) == p_rt_if->arp_rate_limit_pkt_curr)
    {
        /*abnormal process*/
        ARPINSP_PKT_DEBUG("(%s, %d) ifname(%s) current pkt num(%d), more than cfg (%d), set abnormal flag\n", 
            __FUNCTION__, __LINE__, ifname_ext, p_rt_if->arp_rate_limit_pkt_curr, p_rt_if->arp_rate_limit_pkt_max);
        return arprateimit_currpktnum_exceed_action(p_db_if, p_rt_if);
    }
    
    return PM_E_NONE;
}


/*output title of cmd show ip arp rate-limit  */
int32
_arpratelimit_cmd_show_interface_limit_banner(FILE *fp)
{
    if (NULL == fp)
    {
        ARPINSP_PROTO_DEBUG("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_ARP_RATE_LIMIT_ERR;
    }
    
    sal_fprintf(fp,"Port        States   MaxLimitNum  CurrentNum  Violation     Abnormal\n");
    sal_fprintf(fp,"-----------+--------+------------+-----------+-------------+-------\n");

    return PM_E_NONE;
}

int32
_arpratelimit_cmd_show_interface_limit_timer(FILE *fp)
{
    tbl_route_global_t * p_rt_glb = tbl_route_global_get_route_global(); 
    sal_time_t cur_time_sec = 0;
    sal_time_t cur_time_of_ratelimit_sec = 0;
    int32 cur_time_msec = 0;
    int32 cur_time_of_ratelimit_msec = 0;
    
    if (NULL == fp)
    {
        return PM_E_ARP_RATE_LIMIT_ERR;
    }

    cur_time_sec = ctc_time_boottime_sec(NULL);
    cur_time_msec = _time_boottime_msec(NULL);
    
    cur_time_of_ratelimit_sec = (p_rt_glb->arpratelimit_timer_start_time_sec < cur_time_sec) ? (cur_time_sec - p_rt_glb->arpratelimit_timer_start_time_sec) : 0;

    if (p_rt_glb->arpratelimit_timer_start_time_msec <= cur_time_msec)
    {
        cur_time_of_ratelimit_msec = cur_time_msec - p_rt_glb->arpratelimit_timer_start_time_msec;
    }
    else
    {
        /*msec is not enough to decrese, get 1000msec from sec, so sec -1*/
        cur_time_of_ratelimit_sec -= 1;
        cur_time_of_ratelimit_msec = cur_time_msec + 1000 - p_rt_glb->arpratelimit_timer_start_time_msec;
    }
    
    if (cur_time_of_ratelimit_sec > GLB_ARP_RATE_LIMIT_TIMER_VALUE)
    {
        cur_time_of_ratelimit_sec = 0;
    }
    
    sal_fprintf(fp,"The statistics is counting at %d.%ds in 30.00s's time period\n", (int32)cur_time_of_ratelimit_sec, (int32)cur_time_of_ratelimit_msec / 10);

    return PM_E_NONE;
}

/*output one port of arp rate limit*/
int32 
_arpratelimit_cmd_show_interface_limit_one(tbl_route_if_t * p_rt_if, tbl_interface_t *p_if, FILE *fp, cfg_cmd_para_t *para, int32 showDisable)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    if (NULL == fp || NULL == p_rt_if || NULL == p_if)
    {
        ARPINSP_PROTO_DEBUG("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_ARP_RATE_LIMIT_ERR;
    }

    IFNAME_ETH2FULL(p_if->key.name, ifname_ext);
    
    if ((GLB_IF_TYPE_VLAN_IF == p_if->hw_type)  //vlanif
            || (GLB_IF_TYPE_LINKAGG_IF == p_if->hw_type)    //agg if
            || (GLB_IF_TYPE_PORT_IF == p_if->hw_type && GLB_IF_MODE_L3 == p_if->mode))   //route port
          //  || (GLB_IF_TYPE_PORT_IF == p_if->hw_type && (!INTERFACE_IS_LAG_MEMBER(p_if)))) 
    {
        if (p_rt_if->arp_rate_limit_en || TRUE == showDisable)
        {
            sal_fprintf(fp, "%-10s  %-7s  %-11d  %-10d  %-12s  %-4s\n", 
                                ifname_ext, 
                                arpratelimit_enable_str[p_rt_if->arp_rate_limit_en], 
                                p_rt_if->arp_rate_limit_pkt_max, 
                                p_rt_if->arp_rate_limit_pkt_curr, 
                                arpratelimit_exceed_action_str[p_rt_if->arp_rate_limit_violation], 
                                arpratelimit_abnormal_str[p_rt_if->arp_rate_limit_port_abnormal_flag]);
            return PM_E_NONE;
        }
    } 
            
    return PM_E_FAIL;
}

/*output all port of arp rate limit*/
int32
_arpratelimit_cmd_show_interface(tbl_interface_t* p_db_if_input, cfg_cmd_para_t *para)
{
    tbl_route_if_master_t *p_master_rt_if = tbl_route_if_get_master();
    ctclib_slistnode_t *listnode = NULL;
    FILE *fp = NULL;
    uint32 interface_config = FALSE;
    tbl_interface_t     *p_db_if    = NULL;
    tbl_route_if_t      *p_db_rtif  = NULL;
    char ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    /*if p_db_if_input is NULL , show all*/
    if (NULL == para)
    {
        ARPINSP_PROTO_DEBUG("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_ARP_RATE_LIMIT_ERR;
    }
   
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    /*display one*/
    if (p_db_if_input)
    {
        p_db_rtif = tbl_route_if_get_route_if_by_name(p_db_if_input->key.name);
        if (NULL == p_db_rtif)
        {
            sal_fclose(fp);
            fp = NULL;
            IFNAME_ETH2FULL(p_db_if_input->key.name, ifname_ext);
            CFG_CONFLICT_RET("Interface %s is not L3 port ", ifname_ext);
        }
        _arpratelimit_cmd_show_interface_limit_timer(fp);
        _arpratelimit_cmd_show_interface_limit_banner(fp);
        _arpratelimit_cmd_show_interface_limit_one(p_db_rtif, p_db_if_input, fp, para, TRUE);
    }
    else
    {
        if (NULL != p_master_rt_if)
        {
            CTCLIB_SLIST_LOOP(p_master_rt_if->if_list, p_db_rtif, listnode)
            {
                p_db_if = tbl_interface_get_interface_by_name(p_db_rtif->key.name);
                if (NULL == p_db_if)
                {
                    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
                    ARPINSP_PROTO_DEBUG("(%s, %d) Display all, interface %s not exist \n", __FUNCTION__, __LINE__, ifname_ext);
                    continue;
                }

                if (INTERFACE_IS_LAG_MEMBER(p_db_if))
                {
                    continue;
                }

                /*if disable arp rate limit on port, not show*/
                if (ARP_RATE_LIMIT_DISABLE == p_db_rtif->arp_rate_limit_en)
                {
                    continue;
                }

                if (FALSE == interface_config)
                {
                    _arpratelimit_cmd_show_interface_limit_timer(fp);
                    _arpratelimit_cmd_show_interface_limit_banner(fp);
                }
                _arpratelimit_cmd_show_interface_limit_one(p_db_rtif, p_db_if, fp, para, FALSE);
                interface_config = TRUE;
            }
            
            if (interface_config == FALSE)
            {
                sal_fprintf(fp, "No ARP rate limit in system");
            }
        }
    }
    
    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

