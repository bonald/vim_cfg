#ifndef _COLD_API_H_
#define _COLD_API_H_

#include "cold.h"

typedef enum
{
   COLD_LOG_TYPE_CPU_INFO = 0,     
   COLD_LOG_TYPE_MEM_INFO,
   COLD_LOG_TYPE_ENV_INFO,
   COLD_LOG_TYPE_GEN,
   COLD_LOG_TYPE_MAX = 10 /* Max module number */
} cold_log_type_t;

typedef enum
{
    COLD_LOG_LEVEL_EMERGENCY = 0,
    COLD_LOG_LEVEL_ALERT = 1,
    COLD_LOG_LEVEL_CRITICAL = 2,
    COLD_LOG_LEVEL_ERROR = 3,
    COLD_LOG_LEVEL_WARNING = 4,
    COLD_LOG_LEVEL_NOTICE = 5,
    COLD_LOG_LEVEL_INFORMATIONAL = 6,
    COLD_LOG_LEVEL_DEBUG = 7,
    COLD_LOG_LEVEL_MAX
} cold_log_level_t;


#define COLD_DIR_NAME                  "/mnt/flash/cold"
#define COLD_DIR_LOG_NAME              "/mnt/flash/cold/log"
#define COLD_DIR_RUNLOG_NAME           "/mnt/flash/cold/running"
#define COLD_DIR_CMD_NAME              "/mnt/flash/cold/cmd"
#define COLD_DIR_BAK_NAME              "/mnt/flash/cold/bak"
#define COLD_CONF                      "coldcfg.cfg"

char* cold_format_uptime_str(char* uptime_desc);
int cold_logging_init(cold_log_type_t log_type);
void cold_logging(cold_log_type_t log_tye, cold_log_level_t log_level, const char* fmt, ...);
int cold_parser_cold_cfg(char* str_desc, cold_cfg_t* cold_cfg);
char* code_format_cold_cfg(cold_cfg_t* p_cold_cfg, char* str_desc);
#endif/* _COLD_API_H_ */	
