#include "proto.h"
#include "dirent.h"
#include "cold.h"
#include "cold_api.h"
#include "cold_item.h"
#include "cold_log.h"

static bool 
_cold_is_empty_dir(char* dir)
{
    DIR *pdir = NULL;
    uint32 num = 0;
    
    if(!dir)
        return TRUE;
    
    pdir = opendir(dir);
    while (pdir)
    { 
        if (readdir(pdir) != NULL) 
            ++num; 
        else 
            break;
    }
    closedir(pdir); 
    
    if(num > 2)
    {
        return FALSE;
    } 
    return TRUE;
}

static void 
_cold_backup_last_log_file(cold_cfg_t* p_cold_cfg)
{
    uint8 new_bak_index = 0; 
    char ach_line_str[COLD_MAX_LINE_CHAR + 1] = {0};
    char ach_cmd_str[COLD_CMD_EXE_CHAR + 1] = {0};
    char ach_file_name[COLD_PATH_NAME_LEN+ 1] = {0};
    FILE* file = NULL;
    sal_time_t tcurrtime;
    struct sal_tm* p_time = NULL; 
  
    sal_memset(&tcurrtime, 0, sizeof(sal_time_t));
        
    if (_cold_is_empty_dir(COLD_DIR_RUNLOG_NAME)
        && _cold_is_empty_dir(COLD_DIR_LOG_NAME))
    {
        return;
    } 
   
    if (p_cold_cfg->last_bak_index >= p_cold_cfg->log_bak_num)
    {
        new_bak_index = 1;
    }
    else
    {
        new_bak_index = p_cold_cfg->last_bak_index + 1;
    }
    
    /*Modify by hezhichuan  for fixing bug 6471 08/08/21*/
    sal_time(&tcurrtime);
    p_time = localtime(&tcurrtime);  

    if(!p_time)
    {
        return ;
    }
    
    if (p_cold_cfg->last_bak_index)
    {
        /*delete  old log  file & */   
        sal_snprintf(ach_cmd_str, COLD_CMD_EXE_CHAR, "rm -rf %s/log.%d.*.tar.gz  > /tmp/cold_err.log 2>&1",
        COLD_DIR_BAK_NAME, new_bak_index);
        system(ach_cmd_str);
    }

    /*create log.new_bak_index.gz*/
    sal_snprintf(ach_cmd_str, COLD_CMD_EXE_CHAR, 
                "tar -czvf %s/log.%d.%.2d%.2d%.2d%.2d.tar.gz %s/%s %s/*.* %s/*.* > /tmp/cold_err.log 2>&1",
    COLD_DIR_BAK_NAME,new_bak_index,(p_time->tm_mon + 1), p_time->tm_mday , p_time->tm_hour, p_time->tm_min,
    COLD_DIR_NAME,COLD_CONF,COLD_DIR_RUNLOG_NAME,COLD_DIR_LOG_NAME);
    system(ach_cmd_str);
    
    /*delete  old log  file */   
    sal_snprintf(ach_cmd_str, COLD_CMD_EXE_CHAR, "rm -rf %s/*.* %s/*.* > /tmp/cold_err.log 2>&1", 
                COLD_DIR_RUNLOG_NAME, COLD_DIR_LOG_NAME);
    system(ach_cmd_str);

     /*update cold_cfg.ini */
    p_cold_cfg->last_bak_index = new_bak_index;   
    sal_snprintf(ach_file_name, COLD_PATH_NAME_LEN, "%s/%s", COLD_DIR_NAME, COLD_CONF);  
    file = sal_fopen( ach_file_name, "w+");
    if (file)
    {
        code_format_cold_cfg(p_cold_cfg, ach_line_str); 
        sal_fputs(ach_line_str, file);
        sal_fclose( file );
    } 
}

void 
cold_get_cold_cfg(cold_cfg_t* pcold_cfg)
{
    char ach_file_name[COLD_PATH_NAME_LEN + 1] =  {0};
    char ach_line_str[COLD_MAX_LINE_CHAR + 1] = {0};
    FILE* file = NULL;

    sal_snprintf(ach_file_name, COLD_PATH_NAME_LEN, "%s/%s", COLD_DIR_NAME, COLD_CONF);
    if (!pcold_cfg)
    {
        return ;
    }
      
    file = sal_fopen(ach_file_name, "r+");
    if (!file)
    {
        file = sal_fopen(ach_file_name, "w+");
        if (file)
        {
            pcold_cfg->module_enable  = 1;          
            pcold_cfg->log_bak_num    = 8;
            pcold_cfg->last_bak_index = 0;        
            pcold_cfg->reserved  = 0;  
            
            if (NULL != code_format_cold_cfg(pcold_cfg, ach_line_str))
            {
                sal_fputs(ach_line_str, file);
            }
            sal_fclose(file);
        }
    }
    else
    { 
        if (sal_fgets(ach_line_str, COLD_MAX_LINE_CHAR, file))
        {       
            cold_parser_cold_cfg(ach_line_str, pcold_cfg);
        }       
        sal_fclose(file);
    }

    return ; 
}

bool
cold_init(void)
{
    cold_cfg_t cold_cfg;

    cold_get_cold_cfg(&cold_cfg);
    if (!cold_cfg.module_enable)
    {
        return FALSE;      
    }
    _cold_backup_last_log_file(&cold_cfg);
    cold_logging_init(COLD_LOG_TYPE_CPU_INFO);
    cold_logging_init(COLD_LOG_TYPE_MEM_INFO);
    cold_logging_init(COLD_LOG_TYPE_ENV_INFO);
    cold_logging_init(COLD_LOG_TYPE_GEN);
    cold_item_cpu_usage_init();
    
    return TRUE;
}

