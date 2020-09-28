#include "proto.h"
#include "glb_l2_define.h"
#include "gen/tbl_errdisable_define.h" 
#include "gen/tbl_errdisable.h"
#include "gen/tbl_errdisable_flap_define.h" 
#include "gen/tbl_errdisable_flap.h"
#include "gen/tbl_interface_define.h" 
#include "gen/tbl_interface.h" 
#include "gen/tbl_vlan_define.h" 
#include "gen/tbl_vlan.h" 
#include "gen/tbl_sys_global_define.h" 
#include "gen/tbl_sys_global.h" 
#include "gen/tbl_brg_global_define.h" 
#include "gen/tbl_brg_global.h" 
#include "switch.h"
#include "switch_api.h"
#include "errdisable.h"
#include "cdbclient.h"

int32
_errdisable_cmd_read_all(cfg_cmd_para_t *para)
{
    tbl_errdisable_key_t errdisable_key;
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_errdisable_t *p_db_errdisable = NULL;
    tbl_iter_args_t args;
    errdis_rsn_t reason;
    FILE *fp = NULL;


    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    for (reason = 0; reason < ERRDIS_RSN_MAX; reason++)
    {
        /* 1. check exist */
        errdisable_key.reason= reason;
        p_db_errdisable = tbl_errdisable_get_errdisable(&errdisable_key);
        if (NULL == p_db_errdisable)
        {
            continue;
        }

        sal_memset(&args, 0 ,sizeof(args));
        p_field = &field;
        sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
        args.argv[0] = p_field;
        args.argv[1] = fp;

        /* 2. dump errdisable */
        tbl_errdisable_dump_one(p_db_errdisable, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
_errdisable_cmd_show_interface(tbl_interface_t *p_if, tbl_iter_args_t *pargs)
{
    FILE *fp = pargs->argv[0];
    uint32 *p_banner = pargs->argv[1];
    int8 i;
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if_lag = NULL;

    char ifname_ext[IFNAME_EXT_SIZE]; 
    IFNAME_ETH2FULL(p_if->key.name, ifname_ext);

    if(INTERFACE_IS_LAG_MEMBER(p_if))
    {
        sal_memset(&if_key, 0, sizeof(if_key));
        sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_if->lag->lag_id);
        p_db_if_lag = tbl_interface_get_interface(&if_key);
        if(p_db_if_lag)
        {
            if(errdisable_if_is_errdisable(p_db_if_lag))
            {
                return PM_E_NONE;
            }
        }
    }
    if(errdisable_if_is_errdisable(p_if))
    {
        if(TRUE == *p_banner)
        {
            sal_fprintf(fp, "Interfaces that will be enabled at the next timeout:\n");
            sal_fprintf(fp, "%-13s %-17s %-14s\n", "Interface", "Errdisable Reason", "Time Left(sec)");
            sal_fprintf(fp, "-------------+-----------------+--------------\n");
            *p_banner = FALSE;
        }  
        for(i = 0; i < ERRDIS_RSN_MAX; i++)
        {
            if (GLB_FLAG_ISSET(p_if->errdisable_reason, 1<<i))
            {
                sal_fprintf(fp, "%-13s %-17s ", ifname_ext, errdis_rsn_strs[i]);
                
                if (CTC_TASK_IS_RUNNING(p_if->errdisable_timer) && (i != ERRDIS_MONITOR_LINK))
                {
                    sal_fprintf(fp, "%d\n", ctc_timer_get_remain_sec(p_if->errdisable_timer));
                }
                else
                {
                    if (ERRDIS_MLAG_RELOAD_DELAY == i)
                    {
                        sal_fprintf(fp, "%d\n", swth_api_get_mlag_remain_sec());
                    }
                    else
                    {
                        sal_fprintf(fp, "unrecovery\n");
                    }
                }
            }
        }
    }

    return PM_E_NONE;
}

int32
_errdisable_cmd_show_recovery_one(tbl_errdisable_t *p_db_errdisable, uint32 *p_banner, FILE *fp)
{
    char str[MAX_CMD_LINE_LEN];
    
    if (TRUE == *p_banner)
    {
        *p_banner = FALSE;
        sal_fprintf(fp, "%-24s %-16s\n", "ErrDisable Reason", "Timer status");
        sal_fprintf(fp, "------------------------+--------------------\n");
    }

    tbl_errdisable_key_val2str(p_db_errdisable, str, MAX_CMD_LINE_LEN);    
    sal_fprintf(fp, "%-24s %-16s\n", str, 
        enabled_str(p_db_errdisable->recovery_en));

    return PM_E_NONE;

}
int32
_errdisable_cmd_show_detect_one(tbl_errdisable_t *p_db_errdisable, uint32 *p_banner, FILE *fp)
{
    char str[MAX_CMD_LINE_LEN];
    
    if (TRUE == *p_banner)
    {
        *p_banner = FALSE;
        sal_fprintf(fp, "%-24s %-16s\n", "ErrDisable Reason", "Detection status");
        sal_fprintf(fp, "------------------------+--------------------\n");
    }

    tbl_errdisable_key_val2str(p_db_errdisable, str, MAX_CMD_LINE_LEN);
    
    sal_fprintf(fp, "%-24s %-16s\n", str, 
        enabled_str(p_db_errdisable->errdisable_en));

    return PM_E_NONE;

}

int32
_errdisable_cmd_show_recovery(cfg_cmd_para_t *para)
{
    tbl_errdisable_key_t errdisable_key;
    tbl_errdisable_t *p_db_errdisable = NULL;
    errdis_rsn_t reason;
    uint32 banner = TRUE;
    tbl_sys_global_t *sys_glb;
    FILE *fp = NULL;
    void *p_master = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    args.argv[0] = fp;

    for (reason = 0; reason < ERRDIS_RSN_MAX; reason++)
    {
        #ifndef OFPRODUCT
        if (ERRDIS_LINK_FLAP != reason && ERRDIS_FDB_LOOP != reason && ERRDIS_PORT_SECURITY_VIOLATION != reason
          && ERRDIS_BPDU_GUARD != reason && ERRDIS_BPDU_LOOP != reason && ERRDIS_ARP_NUMBERLIMIT != reason && ERRDIS_ARP_RATE_LIMIT != reason)
        {
            continue;
        }
        #else
        if (ERRDIS_LINK_FLAP != reason && ERRDIS_FDB_LOOP != reason && ERRDIS_BPDU_GUARD != reason && 
            ERRDIS_BPDU_LOOP != reason && ERRDIS_ARP_NUMBERLIMIT != reason && ERRDIS_ARP_RATE_LIMIT != reason)
        {
            continue;
        }
        #endif
        errdisable_key.reason= reason;
        p_db_errdisable = tbl_errdisable_get_errdisable(&errdisable_key);
        if (NULL == p_db_errdisable)
        {
            continue;
        }

        _errdisable_cmd_show_recovery_one(p_db_errdisable, &banner, fp);
    }

    sys_glb = tbl_sys_global_get_sys_global();
    if(sys_glb)
    {
        sal_fprintf(fp, "\nTimer interval: %d seconds\n\n", sys_glb->errdisable_interval);
    }

    p_master = tbl_interface_get_master();
    if (p_master)
    {
        banner = TRUE;
        args.argv[1] = &banner;
        tbl_interface_iterate((TBL_ITER_CB_FUNC)_errdisable_cmd_show_interface, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
_errdisable_cmd_show_detect(cfg_cmd_para_t *para)
{
     tbl_errdisable_key_t errdisable_key;
     tbl_errdisable_t *p_db_errdisable = NULL;
     errdis_rsn_t reason;
     uint32 banner = TRUE;

     FILE *fp = NULL;
     CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
     fp = sal_fopen(para->p_show->path, "w+");
     
     for (reason = 0; reason < ERRDIS_RSN_MAX; reason++)
     {
        #ifndef OFPRODUCT
        if (ERRDIS_LINK_FLAP != reason && ERRDIS_FDB_LOOP != reason && ERRDIS_PORT_SECURITY_VIOLATION != reason
          && ERRDIS_BPDU_GUARD != reason && ERRDIS_BPDU_LOOP != reason && ERRDIS_ARP_NUMBERLIMIT != reason && ERRDIS_ARP_RATE_LIMIT != reason)
        {
            continue;
        }
        #else
        if (ERRDIS_LINK_FLAP != reason && ERRDIS_FDB_LOOP != reason && ERRDIS_BPDU_GUARD != reason 
            && ERRDIS_BPDU_LOOP != reason && ERRDIS_ARP_NUMBERLIMIT != reason && ERRDIS_ARP_RATE_LIMIT != reason)
        {
            continue;
        }        
        #endif
        
        errdisable_key.reason= reason;
        p_db_errdisable = tbl_errdisable_get_errdisable(&errdisable_key);
        if (NULL == p_db_errdisable)
        {
            continue;
        }
        _errdisable_cmd_show_detect_one(p_db_errdisable, &banner, fp);
     }
     sal_fclose(fp);
     fp = NULL;
     return PM_E_NONE;
}

static int32
_errdisable_cmd_show_flap(cfg_cmd_para_t *para)
{
     errdis_flap_rsn_t flap_rsn = ERRDIS_FLAP_REASON_LINK;
     char str[MAX_CMD_LINE_LEN];
     tbl_errdisable_flap_key_t errdis_flap_key;
     tbl_errdisable_flap_t *p_db_errdis_flap = NULL;
     
     tbl_errdisable_key_t errdis_key;
     tbl_errdisable_t *p_db_errdis = NULL;
     
     FILE *fp = NULL;
     CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
     fp = sal_fopen(para->p_show->path, "w+");

     errdis_flap_key.flap_rsn = flap_rsn;
     p_db_errdis_flap = tbl_errdisable_flap_get_errdisable_flap(&errdis_flap_key);
     if (NULL == p_db_errdis_flap)
     {
         return PM_E_NOT_EXIST;
     }
     errdis_key.reason = p_db_errdis_flap->errdisable_rsn;
     p_db_errdis = tbl_errdisable_get_errdisable(&errdis_key);
     if (NULL == p_db_errdis)
     {
         return PM_E_NOT_EXIST;
     }

     sal_fprintf(fp, "%-17s %-10s %-10s\n", "ErrDisable Reason", "Flaps", "Time (sec)");
     sal_fprintf(fp, "-----------------+----------+----------\n");
     tbl_errdisable_key_val2str(p_db_errdis, str, MAX_CMD_LINE_LEN);
     sal_fprintf(fp, "%-17s %-10u %-10u\n", str, 
        p_db_errdis_flap->flap_cnt_threshold, p_db_errdis_flap->flap_time_threshold);
     
     sal_fclose(fp);
     fp = NULL;
     return PM_E_NONE;
}


int32
_errdisable_cmd_set_interval(uint32 interval)
{
    tbl_sys_global_t sys_glb;
    
    sal_memset(&sys_glb, 0, sizeof(sys_glb));    
    sys_glb.errdisable_interval = interval;

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_ERRDISABLE_INTERVAL);

    return PM_E_NONE;
}

int32
_errdisable_cmd_get_reason_by_str(char* reason_str, errdis_rsn_t *reason )
{
    if(!reason_str || !reason)
    {
        return PM_E_FAIL;
    }

    if ((sal_strcmp (reason_str, "all") == 0))
        *reason = ERRDIS_RSN_MAX;
    else if ((sal_strcmp (reason_str, "bpduguard") == 0))
        *reason = ERRDIS_BPDU_GUARD;
    else if ((sal_strcmp (reason_str, "bpduloop") == 0))
        *reason = ERRDIS_BPDU_LOOP;
    else if ((sal_strcmp (reason_str, "link-monitor-failure") == 0))
        *reason = ERRDIS_LINK_MON_FAILURE;
    else if ((sal_strcmp (reason_str, "oam-remote-failure") == 0))
        *reason = ERRDIS_OAM_REM_FAILURE;
    else if ((sal_strcmp (reason_str, "port-security") == 0))
        *reason = ERRDIS_PORT_SECURITY_VIOLATION;
    else if ((sal_strcmp (reason_str, "link-flap") == 0))
        *reason = ERRDIS_LINK_FLAP;
    else if ((sal_strcmp (reason_str, "udld") == 0))
        *reason = ERRDIS_UDLD_DISABLE;
    else if ((sal_strcmp (reason_str, "fdb-loop") == 0))
        *reason = ERRDIS_FDB_LOOP;
    else if ((sal_strcmp (reason_str, "loopback-detection") == 0))
        *reason = ERRDIS_LOOPBACK_DETECTION;
    else if ((sal_strcmp (reason_str, "arp-numberlimit") == 0))
        *reason = ERRDIS_ARP_NUMBERLIMIT;
    else if ((sal_strcmp (reason_str, "arp-ratelimit") == 0))
        *reason = ERRDIS_ARP_RATE_LIMIT;
    else
        return PM_E_FAIL;

    return PM_E_NONE;
}

int32
_errdisable_cmd_set_detect(char* reason_str, uint32 is_enable)
{
    tbl_errdisable_key_t errdisable_key;
    tbl_errdisable_t *p_db_errdisable = NULL;
    errdis_rsn_t reason;
    int rc = PM_E_NONE;

    if(!reason_str)
    {
        return PM_E_FAIL;
    }

    PM_E_RETURN(_errdisable_cmd_get_reason_by_str(reason_str, &reason));
    
    errdisable_key.reason= reason;
    p_db_errdisable = tbl_errdisable_get_errdisable(&errdisable_key);
    if (NULL == p_db_errdisable)
    {
        return PM_E_NONE;
    }

    if (is_enable == p_db_errdisable->errdisable_en)
    {
        return PM_E_NONE;
    }
    errdisable_set_detect_en(p_db_errdisable, is_enable);

    return PM_E_NONE;
}

int32
_errdisable_cmd_set_recovery(char* reason_str, uint32 is_enable)
{
    tbl_errdisable_key_t errdisable_key;
    tbl_errdisable_t *p_db_errdisable = NULL;
    errdis_rsn_t reason;
    int rc = PM_E_NONE;
    uint32 i;

    if(!reason_str)
    {
        return PM_E_FAIL;
    }
    
    PM_E_RETURN(_errdisable_cmd_get_reason_by_str(reason_str, &reason));

    #ifdef OFPRODUCT
    if (reason == ERRDIS_PORT_SECURITY_VIOLATION)
    {
        return PM_E_INVALID_PARAM;
    }
    #endif

    if(ERRDIS_RSN_MAX == reason)
    {
        for (i = 0; i < ERRDIS_RSN_MAX; i++)
        {
          #ifndef OFPRODUCT
            if (ERRDIS_LINK_FLAP != i && ERRDIS_FDB_LOOP != i && ERRDIS_PORT_SECURITY_VIOLATION != i
             && ERRDIS_BPDU_GUARD != i && ERRDIS_BPDU_LOOP != i && ERRDIS_ARP_NUMBERLIMIT != i && ERRDIS_ARP_RATE_LIMIT != i)
            {
                continue;
            }
          #else
            if (ERRDIS_LINK_FLAP != i && ERRDIS_FDB_LOOP != i && ERRDIS_BPDU_GUARD != i && 
                ERRDIS_BPDU_LOOP != i && ERRDIS_ARP_NUMBERLIMIT != i && ERRDIS_ARP_RATE_LIMIT != i)
            {
                continue;
            }
          #endif

            errdisable_key.reason= i;
            p_db_errdisable = tbl_errdisable_get_errdisable(&errdisable_key);
            if (NULL == p_db_errdisable)
            {
                continue;
            }

            if (is_enable == p_db_errdisable->recovery_en)
            {
                continue;
            }
            errdisable_set_recovery_en(p_db_errdisable, is_enable);
        }
    }
    else
    {
            errdisable_key.reason= reason;
            p_db_errdisable = tbl_errdisable_get_errdisable(&errdisable_key);
            if (NULL == p_db_errdisable)
            {
                return PM_E_NONE;
            }

            if (is_enable == p_db_errdisable->recovery_en)
            {
                return PM_E_NONE;
            }
            errdisable_set_recovery_en(p_db_errdisable, is_enable);
    }

    return PM_E_NONE;
}

int32
_errdisable_cmd_set_fdb_loop(uint32 maxsize, uint32 rate)
{ 
    tbl_errdisable_key_t errdisable_key;
    tbl_errdisable_t *p_db_errdisable = NULL;
    errdis_rsn_t reason = ERRDIS_FDB_LOOP;
    
    errdisable_key.reason= reason;
    p_db_errdisable = tbl_errdisable_get_errdisable(&errdisable_key);
    if (NULL == p_db_errdisable)
    {
        return PM_E_NONE;
    }
    errdisable_set_fdb_loop(p_db_errdisable, maxsize, rate);
    
    return PM_E_NONE;
}

int32
_errdisable_cmd_set_flap(uint32 count, uint32 second)
{
    tbl_errdisable_flap_key_t errdis_flap_key;
    tbl_errdisable_flap_t *p_db_errdis_flap = NULL;
    tbl_errdisable_key_t errdis_key;
    tbl_errdisable_t *p_db_errdis = NULL;

    sal_memset(&errdis_flap_key, 0, sizeof(errdis_flap_key));
    sal_memset(&errdis_key, 0, sizeof(errdis_key));
    
    errdis_flap_rsn_t flap_rsn = ERRDIS_FLAP_REASON_LINK;
    errdis_flap_key.flap_rsn= flap_rsn;
    p_db_errdis_flap = tbl_errdisable_flap_get_errdisable_flap(&errdis_flap_key);
    if (NULL == p_db_errdis_flap)
    {
        return PM_E_NONE;
    }
    p_db_errdis_flap->errdisable_rsn = ERRDIS_LINK_FLAP;
    
    errdis_key.reason = p_db_errdis_flap->errdisable_rsn;
    p_db_errdis = tbl_errdisable_get_errdisable(&errdis_key);
    if (NULL == p_db_errdis)
    {
        return PM_E_NONE;
    }
    errdisable_set_flap(p_db_errdis_flap, count, second);
    
    return PM_E_NONE;
}

int32
errdisable_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int rc = PM_E_NONE;
    uint32 interval = 0;
    uint32 fdb_loop_maxsize = 0;
    uint32 fdb_loop_rate = 0;
    uint32 flap_count = 0;
    uint32 flap_second  = 0;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if(argc == 2)
        {
            if (0 == sal_strcmp(argv[0], "detect"))
            {
                _errdisable_cmd_set_detect(argv[1], TRUE);
            }
        }
        if(argc == 3)
        {
            if (0 == sal_strcmp(argv[0], "recovery"))
            {
                if (0 == sal_strcmp(argv[1], "interval"))
                {
                    PM_CFG_GET_UINT(interval, argv[2], para->p_rs);
                    _errdisable_cmd_set_interval(interval);
                }
                else if (0 == sal_strcmp(argv[1], "reason"))
                {
                    rc = _errdisable_cmd_set_recovery(argv[2], TRUE);
                    #ifdef OFPRODUCT
                    if (rc == PM_E_INVALID_PARAM)
                    {
                        CFG_INVALID_PARAM_RET("Recovery reason port-security can not configure in Hybrid system");
                    }
                    #endif
                }

            }
            if (0 == sal_strcmp(argv[0], "fdb-loop"))
            {
                 PM_CFG_GET_UINT(fdb_loop_maxsize, argv[1], para->p_rs);
                 PM_CFG_GET_UINT(fdb_loop_rate, argv[2], para->p_rs);
                _errdisable_cmd_set_fdb_loop(fdb_loop_maxsize,fdb_loop_rate);
            }
            if (0 == sal_strcmp(argv[0], "flap"))
            {
                 PM_CFG_GET_UINT(flap_count, argv[1], para->p_rs);
                 PM_CFG_GET_UINT(flap_second, argv[2], para->p_rs);
                _errdisable_cmd_set_flap(flap_count,flap_second);
            }
        }
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_DEL:
        if(argc == 1)
        {
            if (0 == sal_strcmp(argv[0], "fdb-loop"))
            {
               _errdisable_cmd_set_fdb_loop(GLB_FDB_LOOP_BUCKET_SIZE_DEF,GLB_FDB_LOOP_BUCKET_RATE_DEF);
            }
            else if (0 == sal_strcmp(argv[0], "flap"))
            {
                _errdisable_cmd_set_flap(GLB_LINK_FLAP_MAX_FLAP_COUNT,GLB_LINK_FLAP_MAX_FLAP_SECOND);
            }
        }
        if(argc == 2)
        {
            if (0 == sal_strcmp(argv[0], "detect"))
            {
                _errdisable_cmd_set_detect(argv[1], FALSE);
            }
            else if (0 == sal_strcmp(argv[0], "recovery"))
            {
                if (0 == sal_strcmp(argv[1], "interval"))
                {
                    _errdisable_cmd_set_interval(ERRDISABLE_DEFAULT_INTERVAL);
                }
            }
        }
        if(argc == 3)
        {
            if (0 == sal_strcmp(argv[0], "recovery"))
            {
                if (0 == sal_strcmp(argv[1], "reason"))
                {
                    rc = _errdisable_cmd_set_recovery(argv[2], FALSE);
                    #ifdef OFPRODUCT
                    if (rc == PM_E_INVALID_PARAM)
                    {
                        CFG_INVALID_PARAM_RET("Recovery reason port-security can not configure in Hybrid system");
                    }
                    #endif
                }
            }
        }
        break;

    case CDB_OPER_GET:
        rc = _errdisable_cmd_read_all(para);
        break;

    case CDB_OPER_SHOW:
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
errdisable_cmd_process_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_GET:
        if (0 == sal_strcmp(argv[0], "detect"))
        {
             rc = _errdisable_cmd_show_detect(para);
        }
        else if (0 == sal_strcmp(argv[0], "recovery"))
        {
             rc = _errdisable_cmd_show_recovery(para);
        }
        else if (0 == sal_strcmp(argv[0], "flap"))
        {
            rc = _errdisable_cmd_show_flap(para);
        }
        break;

    case CDB_OPER_SHOW:
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

