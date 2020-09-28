
#include "proto.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_mem_summary_define.h"
#include "gen/tbl_mem_summary.h"
#include "gen/tbl_cpu_mem_history_define.h"
#include "gen/tbl_cpu_mem_history.h"
#include "appcfg.h"
#include "app_cpu_mem.h"
#include "lib_snmp.h"
#include "cJSON.h"

/*Added by zhush for bug 52768, 2019-07-09*/
static struct cpu_stat cpu_stats[301];
static uint32_t cpu_stat_cur;
/*end zhush*/

extern char*
format_time_str(char *pszBuf, size_t nLen, char *pszFormat, time_t nTime);

int32
appcfg_get_cpu_usage_ctctop(uint32 *usage)
{
    FILE *pFile = NULL;
    char strLine[CMD_BUF_1024];
    uint32 i = 0;
    uint32 parse_ok = FALSE;
    uint32 cpu_total_line = FALSE;
    uint32 user_time = 0;
    uint32 nice_time = 0;
    uint32 sys_time = 0;
    uint32 idle_time = 0;
    uint32 iow_time = 0;
    uint32 irq_time = 0;
    uint32 sirq_time = 0;
    uint32 total_time = 0;
    uint32 usage_per = 0;

    *usage = 0;
    
    pFile = sal_fopen(CPU_INFO_PATH, "r");
    
    if((NULL == pFile) || (sal_feof(pFile)))
    {
        return PM_E_FAIL;
    }
   
    sal_memset(strLine, 0, sizeof(strLine));

    while(sal_fgets(strLine, CMD_BUF_1024, pFile) != NULL)
    {
        if (0 == sal_strncmp(strLine, "User  Nice", 10))
        {
            cpu_total_line = TRUE;
            continue;
        }
        
        if (cpu_total_line)
        {
            sal_sscanf(strLine, "%d%d%d%d%d%d%d%d", &user_time, &nice_time, &sys_time, &idle_time, &iow_time, &irq_time, &sirq_time, &total_time);
            cpu_total_line = FALSE;
            if (total_time)
            {
                i++;
                parse_ok = TRUE;
            }
        }
    }

    sal_fclose(pFile);
  
    if (!parse_ok)
    {
        return PM_E_FAIL;
    }
    usage_per = ((total_time - idle_time)*100)/total_time;
    *usage = usage_per;

    return PM_E_NONE;
}

int32
app_process_cpu_mem_history(uint32 cpu_usage, uint32 mem_usage)
{
    uint32 cpu_usage_minute   = 0;
    uint32 cpu_usage_10minute = 0;
    uint32 cpu_usage_count    = 0;
    uint32 mem_usage_minute   = 0;
    uint32 mem_usage_10minute = 0;
    uint32 mem_usage_count    = 0;
    uint32 save_index   = 0;
    uint32 it_index     = 0;
    uint32 it_index2    = 0;
    struct sal_timeval  tv;
    int32 rc = PM_E_NONE;

    tbl_cpu_mem_history_t *p_db_history = tbl_cpu_mem_history_get_cpu_mem_history();
    if (!p_db_history)
    {
        return PM_E_NONE;
    }

    save_index = p_db_history->polling_index % GLB_CPU_MEM_MINUTE_SIZE;
    p_db_history->cpu_polling_save[save_index] = cpu_usage;
    p_db_history->mem_polling_save[save_index] = mem_usage;

    if (save_index == GLB_CPU_MEM_MINUTE_SIZE - 1)
    {
        sal_gettimeofday(&tv);

        for (it_index = 0; it_index < GLB_CPU_MEM_MINUTE_SIZE; ++it_index)
        {
            cpu_usage_count += p_db_history->cpu_polling_save[it_index];
            mem_usage_count += p_db_history->mem_polling_save[it_index];
        }
        cpu_usage_minute = cpu_usage_count / GLB_CPU_MEM_MINUTE_SIZE;
        mem_usage_minute = mem_usage_count / GLB_CPU_MEM_MINUTE_SIZE;

        /* save for 1 hour usage history */
        p_db_history->cpu_history_hour[p_db_history->saving_index % GLB_CPU_MEM_HOUR_SIZE] = cpu_usage_minute;
        p_db_history->mem_history_hour[p_db_history->saving_index % GLB_CPU_MEM_HOUR_SIZE] = mem_usage_minute;
        p_db_history->hour_last_save = (uint64)(tv.tv_sec) * 1000 + (uint64)(tv.tv_usec) / 1000;

        if (p_db_history->saving_index >= GLB_CPU_MEM_HOUR_SIZE)
        {
            p_db_history->hour_start = (p_db_history->saving_index % GLB_CPU_MEM_HOUR_SIZE + 1) % GLB_CPU_MEM_HOUR_SIZE;
        }

        p_db_history->saving_index++;

        if (0 == p_db_history->saving_index % GLB_CPU_MEM_DAY_PERSIZE)
        {
            cpu_usage_count = 0;
            mem_usage_count = 0;

            /* data saved in 1 hour after system startup */
            if (p_db_history->saving_index <= GLB_CPU_MEM_HOUR_SIZE)
            {
                it_index2 = p_db_history->saving_index - 1;
                for (it_index = 0; it_index < GLB_CPU_MEM_DAY_PERSIZE; ++it_index, --it_index2)
                {
                    cpu_usage_count += p_db_history->cpu_history_hour[it_index2];
                    mem_usage_count += p_db_history->mem_history_hour[it_index2];
                }

                cpu_usage_10minute = cpu_usage_count / GLB_CPU_MEM_DAY_PERSIZE;
                mem_usage_10minute = mem_usage_count / GLB_CPU_MEM_DAY_PERSIZE;

                /* save for 12 hours usage history */
                p_db_history->cpu_history_day[(p_db_history->saving_index / GLB_CPU_MEM_DAY_PERSIZE - 1) % GLB_CPU_MEM_DAY_SIZE] = cpu_usage_10minute;
                p_db_history->mem_history_day[(p_db_history->saving_index / GLB_CPU_MEM_DAY_PERSIZE - 1) % GLB_CPU_MEM_DAY_SIZE] = mem_usage_10minute;
                p_db_history->day_last_save = (uint64)(tv.tv_sec) * 1000 + (uint64)(tv.tv_usec) / 1000;
            }
            else
            {
                it_index2 = p_db_history->hour_start;
                for (it_index = 0; it_index < GLB_CPU_MEM_DAY_PERSIZE; ++it_index, --it_index2)
                {
                    if (it_index2 == 0)
                    {
                        it_index2 = GLB_CPU_MEM_HOUR_SIZE;
                        cpu_usage_count += p_db_history->cpu_history_hour[it_index2 - 1];
                        mem_usage_count += p_db_history->mem_history_hour[it_index2 - 1];
                    }
                    else
                    {
                        cpu_usage_count += p_db_history->cpu_history_hour[it_index2 - 1];
                        mem_usage_count += p_db_history->mem_history_hour[it_index2 - 1];
                    }
                }

                cpu_usage_10minute = cpu_usage_count / GLB_CPU_MEM_DAY_PERSIZE;
                mem_usage_10minute = mem_usage_count / GLB_CPU_MEM_DAY_PERSIZE;

                /* save for 12 hours usage history */
                p_db_history->cpu_history_day[(p_db_history->saving_index / GLB_CPU_MEM_DAY_PERSIZE - 1) % GLB_CPU_MEM_DAY_SIZE] = cpu_usage_10minute;
                p_db_history->mem_history_day[(p_db_history->saving_index / GLB_CPU_MEM_DAY_PERSIZE - 1) % GLB_CPU_MEM_DAY_SIZE] = mem_usage_10minute;
                p_db_history->day_last_save = (uint64)(tv.tv_sec) * 1000 + (uint64)(tv.tv_usec) / 1000;
            }

            if (p_db_history->saving_index / GLB_CPU_MEM_DAY_PERSIZE > GLB_CPU_MEM_DAY_SIZE)
            {
                p_db_history->day_start = (p_db_history->saving_index / GLB_CPU_MEM_DAY_PERSIZE) % GLB_CPU_MEM_DAY_SIZE;
            }
        }

    }

    p_db_history->polling_index++;
    
    return rc;
}

int32
app_cpu_usage_check(uint32 *cpu_usage)
{
    tbl_mem_summary_t *p_db_sum = tbl_mem_summary_get_mem_summary();
    uint32 usage = 0;
    int32 rc = PM_E_NONE;

    rc = appcfg_get_cpu_usage_ctctop(&usage);
    if (rc < 0)
    {
        return rc;
    }
    *cpu_usage = usage;

    if (usage > p_db_sum->cpu_high_threshold)
    {
        if (APPCFG_CPU_LOG_COUNT == p_db_sum->cpu_high)
        {
            snmp_trap_cpu_usage_trap(TRUE, usage);
            logid_sys(LOG_SYSTEM_4_CPU_THRESHOLD, p_db_sum->cpu_high_threshold, CPU_MEM_POLLING_INTERVAL*APPCFG_CPU_LOG_COUNT);
            p_db_sum->cpu_high = APPCFG_CPU_LOG_THRESHOLD;
            PM_E_RETURN(tbl_mem_summary_set_mem_summary_field(p_db_sum, TBL_MEM_SUMMARY_FLD_CPU_HIGH));
        }
        else if (APPCFG_CPU_LOG_COUNT > p_db_sum->cpu_high)
        {
            p_db_sum->cpu_high++;
            PM_E_RETURN(tbl_mem_summary_set_mem_summary_field(p_db_sum, TBL_MEM_SUMMARY_FLD_CPU_HIGH));
        }
    }
    else if (usage < p_db_sum->cpu_low_threshold)
    {
        if (APPCFG_CPU_LOG_THRESHOLD == p_db_sum->cpu_high)
        {
            snmp_trap_cpu_usage_trap(FALSE, usage);
            logid_sys(LOG_SYSTEM_4_CPU_RESUME, p_db_sum->cpu_low_threshold);
        }
        p_db_sum->cpu_high = 0;
        PM_E_RETURN(tbl_mem_summary_set_mem_summary_field(p_db_sum, TBL_MEM_SUMMARY_FLD_CPU_HIGH));
    }
    else
    {
        if (APPCFG_CPU_LOG_THRESHOLD != p_db_sum->cpu_high)
        {
            p_db_sum->cpu_high = 0;
            PM_E_RETURN(tbl_mem_summary_set_mem_summary_field(p_db_sum, TBL_MEM_SUMMARY_FLD_CPU_HIGH));
        }
    }

    return PM_E_NONE;
}

int32
app_show_mem_summary_info(int32 *total, int32 *free, int32 *buf, int32 *cached)
{
    FILE *pFile = NULL;
    char strLine[CMD_BUF_64];
    char strName[CMD_BUF_64];  
    int32 nTotal, nFree, nBuffers, nCached;
    int32 newFree, newTotal;
    int32 curr_thresh1_mem = 0;

    /* modified by kcao for c and python stats is different; 
    * int ctc_memcheck_thresh2 = SYSTEM_MEM_DEFAULT_THRESH2;
    */
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    tbl_mem_summary_t *p_db_sum = tbl_mem_summary_get_mem_summary();

    /* remove sync, modified by shil for bug50699 and bug50700, 2019-01-09 */
    //system("sync && echo 3 > /proc/sys/vm/drop_caches");

    pFile = sal_fopen(MEM_INFO_PATH, "r");
    if((NULL == pFile) || (sal_feof(pFile)))
    {
        return -1;
    }

    sal_memset(strLine, 0, sizeof(strLine));
    sal_memset(strName, 0, sizeof(strName));

    /*first line is total memory size*/
    sal_fgets(strLine, CMD_BUF_64, pFile);  
    sal_sscanf(strLine, "%s%d", strName, &nTotal);

    /*second line is free memory size*/
    sal_fgets(strLine, CMD_BUF_64, pFile);
    sal_sscanf(strLine, "%s%d", strName, &nFree);

    /*third line is buffer memory size*/
    sal_fgets(strLine, CMD_BUF_64, pFile);
    sal_sscanf(strLine, "%s%d", strName, &nBuffers);

    /*fourth line is cached memory size*/
    sal_fgets(strLine, CMD_BUF_64, pFile);
    sal_sscanf(strLine, "%s%d", strName, &nCached);

    sal_fclose(pFile);

    curr_thresh1_mem = (p_db_glb->memory_threshold1) * 1024;
    newTotal = (nTotal > curr_thresh1_mem) ? (nTotal - curr_thresh1_mem) : 0;
    newFree = (nFree > curr_thresh1_mem) ? (nFree - curr_thresh1_mem) : 0;

    if (newFree < newTotal/10)
    {
        if (!p_db_sum->mem_high)
        {
            p_db_sum->mem_high = TRUE;
            tbl_mem_summary_set_mem_summary_field(p_db_sum, TBL_MEM_SUMMARY_FLD_MEM_HIGH);
            snmp_trap_memory_usage_trap(TRUE, newFree);
        }
    }
    else if (newFree > newTotal/2)
    {
        if (p_db_sum->mem_high)
        {
            p_db_sum->mem_high = FALSE;
            tbl_mem_summary_set_mem_summary_field(p_db_sum, TBL_MEM_SUMMARY_FLD_MEM_HIGH);
            snmp_trap_memory_usage_trap(FALSE, newFree);
        }
    }

    *total = newTotal;
    *free = newFree;
    *buf = nBuffers;
    *cached = nCached;

    return PM_E_NONE;
}

void
app_cpu_mem_polling_timer_fn(void *p_data)
{
    tbl_mem_summary_t mem_info;
    int32 nTotal = 0, nFree = 0, nBuffers = 0, nCached = 0; 
    uint32 cpu_usage = 0;
    uint32 mem_usage = 0;

    /* check memory */
    app_show_mem_summary_info(&nTotal, &nFree, &nBuffers, &nCached);
    mem_usage = 100 - nFree * 100 / nTotal;

    sal_memset(&mem_info, 0, sizeof(mem_info)); 
    mem_info.total = nTotal;
    mem_info.used = (nTotal - nFree);
    mem_info.free = nFree;
    tbl_mem_summary_set_mem_summary_field(&mem_info, TBL_MEM_SUMMARY_FLD_TOTAL);
    tbl_mem_summary_set_mem_summary_field(&mem_info, TBL_MEM_SUMMARY_FLD_USED);
    tbl_mem_summary_set_mem_summary_field(&mem_info, TBL_MEM_SUMMARY_FLD_FREE);
    
    /* check cpu */
    app_cpu_usage_check(&cpu_usage);

    /* save cpu & memory history usage */
    app_process_cpu_mem_history(cpu_usage, mem_usage);

    //CTC_TASK_ADD_TIME_MSEC(app_cpu_mem_polling_timer_fn, NULL, 100);
    ctc_task_add_timer(TASK_PRI_NORMAL, app_cpu_mem_polling_timer_fn, NULL, CPU_MEM_POLLING_INTERVAL);
    
    return;
}

/*Added by zhush for bug 52768, 2019-07-09*/
void
app_cpu_usage_polling_timer_fn(void *p_data)
{
    static FILE *filp =NULL;
    static char buf[0x100];
    uint64_t user, nice, system, idle, iowait, irq, softirq, steal;

    if (!filp)
        filp = sal_fopen("/proc/stat", "r");

    if (!filp)
        goto out;

    sal_rewind(filp);
    sal_fflush(filp);
    sal_fgets(buf, sizeof(buf), filp);
    sal_sscanf(buf, "cpu %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64"",
            &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);

    cpu_stats[cpu_stat_cur].used = user+nice+system+iowait+irq+softirq+steal;
    cpu_stats[cpu_stat_cur].total = cpu_stats[cpu_stat_cur].used + idle;

    cpu_stat_cur++;
    if (cpu_stat_cur >= 301)
        cpu_stat_cur = 0;
	
out:
    ctc_task_add_timer(TASK_PRI_NORMAL, app_cpu_usage_polling_timer_fn, NULL, CPU_USAGE_POLLING_INTERVAL);
    return;
}


int32
_app_act_get_cpu_mem_history(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32    rc        = PM_E_NONE;
    FILE    *fp        = NULL;
    tbl_cpu_mem_history_t *p_db_history = tbl_cpu_mem_history_get_cpu_mem_history();

    uint8  show_type = APPCFG_SHOW_CPU_HISTORY;
    uint8  show_mode = APPCFG_CPU_MEM_HISTORY_1HOUR;
    uint32 show_size = 0;
    uint32 save_size = 0;
    uint8  show_data = 0;
    int32  delta_time = 0;
    struct sal_timeval  tv;
    char   time[256] = {0};

    uint32 it_index = 0;
    uint32 index    = 0;
    uint32 num_fill = 0;
    uint32 it_fill  = 0;

    /* check param */
    PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);

    if (!sal_strcmp("mem", argv[0]))
    {
        show_type = APPCFG_SHOW_MEM_HISTORY;
    }

    if (!sal_strcmp("12hour", argv[1]))
    {
        show_mode = APPCFG_CPU_MEM_HISTORY_12HOUR;
    }

    PM_CFG_GET_UINT(show_size, argv[2], para->p_rs);

    if (show_mode == APPCFG_CPU_MEM_HISTORY_1HOUR)
    {
        if (show_size > GLB_CPU_MEM_HOUR_SIZE)
        {
            CFG_CONFLICT_RET("Size wrong !");
        }
    }
    else
    {
        if (show_size > GLB_CPU_MEM_DAY_SIZE)
        {
            CFG_CONFLICT_RET("Size wrong !");
        }
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    index = (APPCFG_CPU_MEM_HISTORY_1HOUR == show_mode) ? p_db_history->hour_start : p_db_history->day_start;

    save_size = (APPCFG_CPU_MEM_HISTORY_12HOUR == show_mode) ? 
                (p_db_history->saving_index / 10 > GLB_CPU_MEM_DAY_SIZE ? GLB_CPU_MEM_DAY_SIZE : p_db_history->saving_index / 10) : 
                (p_db_history->saving_index > GLB_CPU_MEM_HOUR_SIZE ? GLB_CPU_MEM_HOUR_SIZE : p_db_history->saving_index);
    if (save_size < show_size)
    {
        show_size = save_size;
    }

    /* step forward if show_size < save_size */
    if (show_size < save_size)
    {
        for (it_index = 0; it_index < save_size - show_size; it_index++)
        {
            if (index == ((APPCFG_CPU_MEM_HISTORY_1HOUR == show_mode) ? GLB_CPU_MEM_HOUR_SIZE : GLB_CPU_MEM_DAY_SIZE))
            {
                index = 0;
            }
            index ++;
        }
    }

    /* history header */
    sal_fprintf(fp, "%18s %s usage (last %d minutes)\n"," ",
               (APPCFG_SHOW_CPU_HISTORY == show_type) ? "CPU" : "Memory", 
               (APPCFG_CPU_MEM_HISTORY_1HOUR == show_mode) ? show_size : show_size * GLB_CPU_MEM_DAY_PERSIZE);
    sal_fprintf(fp, "  time |");
    for (it_fill = 1; it_fill <= 50; it_fill++)
    {
        if (it_fill % 5 == 0)
        {
            sal_fprintf(fp, "%d", it_fill / 5);
        }
        else
        {
            sal_fprintf(fp, " ");
        }
    }
    sal_fprintf(fp, "\n");

    sal_fprintf(fp, "(HH:MM)|");
    for (it_fill = 1; it_fill <= 50; it_fill++)
    {
        if (it_fill % 5 == 0)
        {
            sal_fprintf(fp, "0");
        }
        else
        {
            sal_fprintf(fp, ".");
        }
    }
    sal_fprintf(fp, " usage(%%)\n");

    /* plot usage history */
    for (it_index = 0; it_index < show_size; it_index++, index ++)
    {
        /* show index over threshold loop back to index 0 */
        if (index == ((APPCFG_CPU_MEM_HISTORY_1HOUR == show_mode) ? GLB_CPU_MEM_HOUR_SIZE : GLB_CPU_MEM_DAY_SIZE))
        {
            index = 0;
        }

        show_data = (APPCFG_SHOW_CPU_HISTORY == show_type) ? 
                    ((APPCFG_CPU_MEM_HISTORY_1HOUR == show_mode) ? p_db_history->cpu_history_hour[index] : p_db_history->cpu_history_day[index]):
                    ((APPCFG_CPU_MEM_HISTORY_1HOUR == show_mode) ? p_db_history->mem_history_hour[index] : p_db_history->mem_history_day[index]);
        num_fill = show_data / 2;

        delta_time = 1 + it_index - show_size;
        if (APPCFG_CPU_MEM_HISTORY_12HOUR == show_mode)
        {
            delta_time *= GLB_CPU_MEM_DAY_PERSIZE;
        }
 
        tv.tv_sec = ((APPCFG_CPU_MEM_HISTORY_1HOUR == show_mode) ? p_db_history->hour_last_save :
                      p_db_history->day_last_save) / 1000 + delta_time * 60;
        format_time_str(time, 256, "%H:%M", tv.tv_sec);
        sal_fprintf(fp, "%6s |", time); 

        for (it_fill = 0; it_fill < num_fill; it_fill++)
        {
            sal_fprintf(fp, "=");
        }

        for (it_fill = num_fill; it_fill < 50; it_fill++)
        {
            sal_fprintf(fp, " ");
        }

        sal_fprintf(fp, "%4d\n", show_data);
    }
    sal_fclose(fp);
    fp = NULL;

    return rc;    
}

int32
_app_act_get_cpu_history_json(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    struct cpu_stat *s1, *s2;
    uint32_t u, t;
    uint8_t n, f;
    cJSON *root = NULL;
    char *out = NULL;
    char str_buf[10] = "";

    root = cJSON_CreateObject();

    s2 = &cpu_stats[(cpu_stat_cur + 301 - 1) % 301];
    s1 = &cpu_stats[(cpu_stat_cur + 301 - 6) % 301];
    u = (s2->used - s1->used)*100;
    t = s2->total - s1->total;
    if (t == 0)
        t = 1;
    n = u / t;
    f = (u % t)*100 / t;
    snprintf(str_buf, sizeof(str_buf), "%d.%d%%", n, f);
    cJSON_AddStringToObject(root, "CPU usage for five seconds", str_buf);

    s1 = &cpu_stats[(cpu_stat_cur + 301 - 61) % 301];
    u = (s2->used - s1->used)*100;
    t = s2->total - s1->total;
    if (t == 0)
        t = 1;
    n = u / t;
    f = (u % t)*100 / t;
    snprintf(str_buf, sizeof(str_buf), "%d.%d%%", n, f);
    cJSON_AddStringToObject(root, "CPU usage for one minute", str_buf);

    s1 = &cpu_stats[cpu_stat_cur];
    u = (s2->used - s1->used)*100;
    t = s2->total - s1->total;
    if (t == 0)
        t = 1;
    n = u / t;
    f = (u % t)*100 / t;
    snprintf(str_buf, sizeof(str_buf), "%d.%d%%", n, f);
    cJSON_AddStringToObject(root, "CPU usage for five minute", str_buf);
    out = cJSON_Print(root);
	
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
	
    sal_fprintf(fp, out);
    sal_fprintf(fp,"\n");
    sal_fclose(fp);

    cJSON_Delete(root);
    cJSON_free(out);

    return PM_E_NONE;
}

int32
app_act_show_cpu_mem_history(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
	
    switch (para->oper)
    {
       case CDB_OPER_GET:        
        rc = _app_act_get_cpu_mem_history(argv, argc, para);;
        break;
		
	case CDB_OPER_GETJSON:      
        rc = _app_act_get_cpu_history_json (argv, argc, para);
        break;
		
       default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
             
}
/*end zhush*/

int32
_app_cmd_get_mem_summary(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_mem_summary_t* p_mem_info = NULL;
    tbl_mem_summary_t mem_info;
    tbl_iter_args_t args;
    int32 nTotal = 0, nFree = 0, nBuffers = 0, nCached = 0; 

    sal_memset(&mem_info, 0, sizeof(mem_info)); 
    
    app_show_mem_summary_info(&nTotal, &nFree, &nBuffers, &nCached);
    mem_info.total = nTotal;
    mem_info.used = (nTotal - nFree);
    mem_info.free = nFree;
    mem_info.buffer = nBuffers;
    mem_info.cached = nCached;
    
    tbl_mem_summary_set_mem_summary_field(&mem_info, TBL_MEM_SUMMARY_FLD_TOTAL);
    tbl_mem_summary_set_mem_summary_field(&mem_info, TBL_MEM_SUMMARY_FLD_USED);
    tbl_mem_summary_set_mem_summary_field(&mem_info, TBL_MEM_SUMMARY_FLD_FREE);
    tbl_mem_summary_set_mem_summary_field(&mem_info, TBL_MEM_SUMMARY_FLD_BUFFER);
    tbl_mem_summary_set_mem_summary_field(&mem_info, TBL_MEM_SUMMARY_FLD_CACHED);
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_mem_info = tbl_mem_summary_get_mem_summary();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_mem_summary_dump_one(p_mem_info, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
_app_cmd_get_mem_summary_json(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    int32 nTotal = 0, nFree = 0, nBuffers = 0, nCached = 0; 
    cJSON *root = NULL;
    char *out = NULL; 
    char str_buf[256] = "";

    app_show_mem_summary_info(&nTotal, &nFree, &nBuffers, &nCached);

    root = cJSON_CreateObject(); 
    cJSON_AddNumberToObject(root, "Total memory", nTotal);
    cJSON_AddNumberToObject(root, "Used memory", (nTotal-nFree));
    cJSON_AddNumberToObject(root, "Freed memory", nFree);
    cJSON_AddNumberToObject(root, "Buffer memory", nBuffers);
    cJSON_AddNumberToObject(root, "Cached memory", nCached);
    sal_snprintf(str_buf, sizeof(str_buf), "%.2f%%",  ((float)(nTotal-nFree)/nTotal)*100);
    cJSON_AddStringToObject(root,  "Memory utilization:", str_buf);
    out = cJSON_Print(root);
	
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_fprintf(fp, out);
    sal_fprintf(fp,"\n");
    sal_fclose(fp);

    cJSON_Delete(root);
    cJSON_free(out);

    return PM_E_NONE;
}

int32
_app_cpu_cmd_set_mem_summary(tbl_mem_summary_t* p_mem_info, uint32 value,tbl_mem_summary_field_id_t field_id,cfg_cmd_para_t *para)
{
    if (!p_mem_info)
    {
        return PM_E_INVALID_PARAM;    
    }
    switch (field_id)
    {
    case TBL_MEM_SUMMARY_FLD_CPU_HIGH_THRESHOLD:
        if (value == p_mem_info->cpu_high_threshold)
        {
            return PM_E_NONE;
        }
        p_mem_info->cpu_high_threshold = value;
        tbl_mem_summary_set_mem_summary_field(p_mem_info,TBL_MEM_SUMMARY_FLD_CPU_HIGH_THRESHOLD);
        break;
    case TBL_MEM_SUMMARY_FLD_CPU_LOW_THRESHOLD:
        if (value == p_mem_info->cpu_low_threshold)
        {
            return PM_E_NONE;
        }
        p_mem_info->cpu_low_threshold = value;
        tbl_mem_summary_set_mem_summary_field(p_mem_info,TBL_MEM_SUMMARY_FLD_CPU_LOW_THRESHOLD);
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    return PM_E_NONE;
}

int32
app_cmd_process_mem_summary(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_field_t       *p_field = NULL;
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;
    tbl_mem_summary_t* p_mem_info = NULL;
    int32                field_id = 0;
    int32                low_threshold = 0;
    int32                high_threshold = 0;
    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_MEM_SUMMARY);
    switch (para->oper)
    {
    case CDB_OPER_SET:
        p_mem_info = tbl_mem_summary_get_mem_summary();
        if (!p_mem_info)
        {
            CFG_CONFLICT_RET("Mem_summary init failed");
        }
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }
        switch (field_id)
        {
        case TBL_MEM_SUMMARY_FLD_CPU_HIGH_THRESHOLD:            
        case TBL_MEM_SUMMARY_FLD_CPU_LOW_THRESHOLD:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            PM_CFG_GET_INTEGER(low_threshold, argv[1], para->p_rs);
            PM_CFG_GET_INTEGER(high_threshold, argv[3], para->p_rs);
            if (low_threshold == high_threshold)
            {
                CFG_INVALID_PARAM_RET("This CPU usage low threshold can not be same to the high threshold");
            }
            if (high_threshold<low_threshold)
            {
                CFG_INVALID_PARAM_RET("This CPU usage low threshold can not more than the high threshold");
            }
            if ((high_threshold-low_threshold)<10)
            {
                CFG_INVALID_PARAM_RET("This CPU usage low threshold at least 10 less than the high threshold");
            }
            rc = _app_cpu_cmd_set_mem_summary(p_mem_info, low_threshold,TBL_MEM_SUMMARY_FLD_CPU_LOW_THRESHOLD,para);
            if (rc)
            {
                return rc;    
            }
            rc = _app_cpu_cmd_set_mem_summary(p_mem_info, high_threshold,TBL_MEM_SUMMARY_FLD_CPU_HIGH_THRESHOLD,para);
            if (rc)
            {
                return rc;    
            }
            break;
        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break; 
        }    
    
    case CDB_OPER_GET:

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        rc = _app_cmd_get_mem_summary(para, p_node, &field);
        break;
/*Added by zhush for bug 52768, 2019-07-09*/		
    case CDB_OPER_GETJSON:

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        rc = _app_cmd_get_mem_summary_json(para, p_node, &field);
        break;
/*end zhush*/

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}


