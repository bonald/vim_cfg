
#include "proto.h"
#include "lib_tblinc.h"

#include "chsm.h"
#include "chsm_mgt.h"
#include "chsm_debug.h"
#include "cold.h"
#include "cold_log.h"
#include "cold_api.h"
#include "cold_item.h"


#define MAX_BIG_ITEM_NUM 8

static uint64 g_user = 0;  
static uint64 g_nice = 0;  
static uint64 g_system = 0;
static uint64 g_idle = 0;  
static uint64 g_iowait = 0;  
static uint64 g_irq = 0;   
static uint64 g_softirq = 0;  
static uint64 g_steal = 0;  
static uint64 g_guest = 0;
static uint32 g_max_big_item_num = 0;


static int32
_cold_item_get_cpu_info(cold_sw_item_sys_t* sys_info)
{  
    uint64 user = 0;  
    uint64 nice = 0;  
    uint64 system = 0;
    uint64 idle = 0;  
    uint64 iowait = 0;  
    uint64 irq = 0;   
    uint64 softirq = 0;  
    uint64 steal = 0;  
    uint64 guest = 0;
    uint64 delt = 0;
    uint32 mem_used = 0;
    FILE* stream = NULL;
    char str_line[COLD_MAX_LINE_CHAR + 1] = {0};
    char str_name[COLD_MAX_LINE_CHAR + 1]= {0};;  
    
    /* get CPU info */
    stream = sal_fopen(COLD_CPU_INFO_PATH, SAL_OPEN_RO);
    if((NULL == stream))
    {
        CHSM_LOG_ERR("COLD open file failed.");
        return PM_E_FAIL;
    }
    
    if (sal_feof(stream))
    {
        CHSM_LOG_WARN("COLD reach end of file.");
        sal_fclose(stream);
        return PM_E_FAIL;
    }

    sal_fgets(str_line, COLD_MAX_LINE_CHAR, stream);  
    sal_sscanf(str_line, "%s %"PRId64" %"PRId64" %"PRId64" %"PRId64" %"PRId64" %"PRId64" %"PRId64" %"PRId64" %"PRId64"", 
               str_name, &user, &nice, &system, &idle, &iowait, &irq, &softirq, 
               &steal, &guest);
    sal_fclose(stream);


    delt = (user + nice + system + idle + iowait + irq + softirq + steal + guest) 
            - (g_user + g_nice + g_system + g_idle + g_iowait + g_irq + g_softirq + g_steal + g_guest);

    if(delt)
    {
        sys_info->cpu_usage = 100 - (100 * (idle - g_idle) / delt);
    }

    g_user = user;
    g_nice = nice;
    g_system = system;
    g_idle = idle;
    g_iowait = iowait;
    g_irq = irq;
    g_softirq = softirq;
    g_steal = steal;
    g_guest = guest;

    /* get memory info */
    stream = sal_fopen(COLD_MEM_INFO_PATH, SAL_OPEN_RO);
    if((NULL == stream))
    {
        CHSM_LOG_ERR("COLD open file failed.");
        return PM_E_FAIL;
    }
    if (sal_feof(stream))
    {
        CHSM_LOG_WARN("COLD reach end of file.");
        sal_fclose(stream);
        return PM_E_FAIL;
    }

    /*first line is total memory size*/
    sal_fgets(str_line, COLD_MAX_LINE_CHAR, stream);  
    sal_sscanf(str_line, "%s%d", str_name, &sys_info->mem_info.mem_total);

    /*second line is free memory size*/
    sal_fgets(str_line, COLD_MAX_LINE_CHAR, stream);
    sal_sscanf(str_line, "%s%d", str_name, &sys_info->mem_info.mem_free);

    /*third line is buffer memory size*/
    sal_fgets(str_line, COLD_MAX_LINE_CHAR, stream);
    sal_sscanf(str_line, "%s%d", str_name, &sys_info->mem_info.mem_buffers);
    
     /*fourth line is cached memory size*/
    sal_fgets(str_line, COLD_MAX_LINE_CHAR, stream);
    sal_sscanf(str_line, "%s%d", str_name, &sys_info->mem_info.mem_cached);
    sal_fclose(stream);

    mem_used = sys_info->mem_info.mem_total - sys_info->mem_info.mem_free;
    sys_info->mem_usage = (100 * mem_used)/sys_info->mem_info.mem_total;

    return PM_E_NONE;
}


static void 
_cold_item_format_ps_desc(void)
{
    char cmd_exe_str[COLD_CMD_EXE_CHAR+1] = {0};
    char ach_file_name[COLD_PATH_NAME_LEN+ 1] =  {0 };
    char item_desc[COLD_MAX_LINE_CHAR + 1] = {0};
    FILE* stream = NULL;
    sal_time_t tcurrtime;
    uint32 max_big_item_num = g_max_big_item_num;

    sal_snprintf(ach_file_name, COLD_PATH_NAME_LEN, "%s/%s", COLD_DIR_LOG_NAME, COLD_PS_INFO_FILE_NAME); 
    if(max_big_item_num)
    {
        stream = sal_fopen(ach_file_name, "r+");
    }
    else
    {
        stream = sal_fopen(ach_file_name, "w+");
    }
    if (stream)
    {   
        if(sal_fseek(stream, 0, SEEK_END) < 0)
            CHSM_LOG_ERR("COLD goto end of ps info file fail, %s.", sal_strerror(errno));
        
        sal_time(&tcurrtime);
        sal_fputs(STR_RETURN, stream);   
        sal_snprintf(item_desc, COLD_MAX_LINE_CHAR, "%s(uptime:%s)\n", sal_ctime( &tcurrtime ), cold_format_uptime_str(cmd_exe_str)); 
        sal_fputs(item_desc, stream);
        sal_fputs(STR_RETURN, stream);   
        sal_fclose(stream);
    }

    sal_memset(cmd_exe_str, 0, sizeof(cmd_exe_str));   
    sal_sprintf(cmd_exe_str, "ps aux >> %s", ach_file_name);

    system(cmd_exe_str);
}

static void
_cold_item_format_net_desc(void)
{
    char cmd_exe_str[COLD_CMD_EXE_CHAR+1] = {0};
    char ach_file_name[COLD_PATH_NAME_LEN+ 1] =  {0 };
    char item_desc[COLD_MAX_LINE_CHAR + 1] = {0};
    FILE* stream = NULL;
    sal_time_t tcurrtime;
    uint32 max_big_item_num = g_max_big_item_num;
    
    sal_snprintf(ach_file_name, COLD_PATH_NAME_LEN, "%s/%s", COLD_DIR_LOG_NAME, COLD_NET_INFO_FILE_NAME); 
    if(max_big_item_num)
    {
        stream = sal_fopen(ach_file_name, "r+");
    }
    else
    {
        stream = sal_fopen(ach_file_name, "w+");
    }
    if (stream)
    {   
        if(sal_fseek(stream, 0, SEEK_END) < 0)
            CHSM_LOG_ERR("COLD goto end of net info file fail, %s.", sal_strerror(errno));
        
        sal_time(&tcurrtime);
        sal_fputs(STR_RETURN, stream);   
        sal_snprintf(item_desc, COLD_MAX_LINE_CHAR, "%s(uptime:%s)\n", sal_ctime( &tcurrtime ), cold_format_uptime_str(cmd_exe_str)); 
        sal_fputs(item_desc, stream);
        sal_fputs(STR_RETURN, stream);   
        sal_fclose(stream);
    }

    sal_memset(cmd_exe_str, 0, sizeof(cmd_exe_str));   
    sal_sprintf(cmd_exe_str, "netstat -an >> %s", ach_file_name);

    system(cmd_exe_str);  
}

static int32
_cold_item_report_envinfo(void)
{
#ifndef _GLB_DISTRIBUTE_SYSTEM_
    int32 tmpr = 0;

    /* get highest temperature on the board */
    //TODO

    /*log current temperature to the logging file*/   
    cold_logging(COLD_LOG_TYPE_ENV_INFO, COLD_LOG_LEVEL_EMERGENCY, "tempr %d", tmpr); 
#endif /* _GLB_DISTRIBUTE_SYSTEM_ */
    return PM_E_NONE;
}

static int32 
_cold_item_report_sysinfo(cold_sw_item_sys_t* sys_info)
{  
    char file_name[COLD_PATH_NAME_LEN+ 1] =  {0 };
    uint32 max_big_item_num = g_max_big_item_num;

    if (!sys_info)
    {
        CHSM_LOG_ERR("COLD invalid sysinfo pointer.");
        return PM_E_INVALID_PARAM;
    }

    /*log cpu usage to the logging file*/   
    cold_logging(COLD_LOG_TYPE_CPU_INFO, COLD_LOG_LEVEL_EMERGENCY, 
                "cpu_usage %d", sys_info->cpu_usage); 

    /*log memory usage and info to the logging file*/    
    cold_logging(COLD_LOG_TYPE_MEM_INFO, COLD_LOG_LEVEL_EMERGENCY,
                "memory_usage %d", sys_info->mem_usage);

    cold_logging(COLD_LOG_TYPE_MEM_INFO, COLD_LOG_LEVEL_EMERGENCY,
                "total: %d, free: %d, buffers: %d, cached: %d", 
                sys_info->mem_info.mem_total, sys_info->mem_info.mem_free, 
                sys_info->mem_info.mem_buffers, sys_info->mem_info.mem_cached);

    if(sys_info->cpu_usage > COLD_HM_CPU_THRESHOLD 
        || sys_info->mem_usage > COLD_HM_MEM_THRESHOLD)
    {
        if(max_big_item_num >= MAX_BIG_ITEM_NUM)
        {
            g_max_big_item_num = 0;
            /* replace the file */
            sal_snprintf(file_name, COLD_PATH_NAME_LEN, "%s/%s", COLD_DIR_LOG_NAME, COLD_NET_INFO_FILE_NAME); 
            sal_unlink(file_name);
            sal_snprintf(file_name, COLD_PATH_NAME_LEN, "%s/%s", COLD_DIR_LOG_NAME, COLD_PS_INFO_FILE_NAME); 
            sal_unlink(file_name);
        }
        else
        {
            g_max_big_item_num++;
        }

        _cold_item_format_ps_desc();
        _cold_item_format_net_desc();
    }

    return PM_E_NONE;
}

void
cold_item_cpu_usage_init(void)
{
    FILE* stream = NULL;
    char str_line[COLD_MAX_LINE_CHAR + 1] = {0};
    char str_name[COLD_MAX_LINE_CHAR + 1]= {0};;  
    
    /* get cpu stat */
    stream = sal_fopen(COLD_CPU_INFO_PATH, SAL_OPEN_RO);
    if((NULL == stream))
    {
        CHSM_LOG_ERR("COLD open cpu info file failed.");
        return;
    }
    
    if (sal_feof(stream))
    {
        CHSM_LOG_WARN("COLD reach end of file.");
        sal_fclose(stream);
        return;
    }

    sal_fgets(str_line, COLD_MAX_LINE_CHAR, stream);  
    sal_sscanf(str_line, "%s %"PRId64" %"PRId64" %"PRId64" %"PRId64" %"PRId64" %"PRId64" %"PRId64" %"PRId64" %"PRId64, 
               str_name, &g_user, &g_nice, &g_system, &g_idle, &g_iowait, &g_irq, 
               &g_softirq, &g_steal, &g_guest);
    sal_fclose(stream);
}

int32
cold_item_sys_info_monitor(void)
{
    int32 ret;
    cold_sw_item_sys_t sys_info;
    
    sal_memset(&sys_info, 0, sizeof(cold_sw_item_sys_t));

    ret = _cold_item_get_cpu_info(&sys_info);

    /* log system info to the logging file */
    ret += _cold_item_report_sysinfo(&sys_info);

    /* log environment info to the logging file */
    ret += _cold_item_report_envinfo();

    return ret;
}

