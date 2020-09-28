
#ifndef _APP_CPU_MEM_H_
#define _APP_CPU_MEM_H_

#define CPU_MEM_POLLING_INTERVAL    10
#define CPU_USAGE_POLLING_INTERVAL    1
#define APPCFG_CPU_DEFULT_HIGH_THRESHOLD   90
#define APPCFG_CPU_DEFULT_LOW_THRESHOLD    50
#define APPCFG_CPU_MAX_THRESHOLD   100
#define APPCFG_CPU_MIN_THRESHOLD   1
#define APPCFG_CPU_LOG_THRESHOLD    10
#define APPCFG_CPU_LOG_COUNT        3

#define APPCFG_SHOW_CPU_HISTORY     0
#define APPCFG_SHOW_MEM_HISTORY     1
#define APPCFG_CPU_MEM_HISTORY_1HOUR   0
#define APPCFG_CPU_MEM_HISTORY_12HOUR  1

/*Added by zhush for bug 52768, 2019-07-09*/
struct cpu_stat {
    uint64_t used;  /* used time slices */
    uint64_t total; /* total time slices */
};
/*end zhush*/

void
app_cpu_mem_polling_timer_fn(void *p_data);

void
app_cpu_usage_polling_timer_fn(void *p_data);

int32
app_act_show_cpu_mem_history(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_cmd_process_mem_summary(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /* !_APP_CPU_MEM_H_ */
