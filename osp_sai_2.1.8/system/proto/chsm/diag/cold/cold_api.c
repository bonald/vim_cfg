#include "proto.h"
#include "chsm.h"
#include "chsm_debug.h"
#include "cold.h"
#include "cold_api.h"
#include "cold_item.h"
#include "cold_log.h"

static sal_time_t 
_cold_get_sys_up_time(void)
{
    struct sal_timeval uptime;
    sal_time_t sys_up_time = 0;
    ctc_task_boottime(&uptime);
    sys_up_time = uptime.tv_sec;
    
    return sys_up_time;
}

char* 
cold_format_uptime_str(char* uptime_desc)
{
    uint32 sys_up_time = 0;
    
    if (!uptime_desc )
    {
        return "" ;
    }

    sys_up_time = _cold_get_sys_up_time();
    sal_snprintf(uptime_desc, COLD_TIME_DESC, "%d days,%d hours,%d minutes", 
                    (sys_up_time / 86400) % 365, (sys_up_time / 3600) % 24, 
                    (sys_up_time / 60) % 60);

    return uptime_desc;
}

static void 
_cold_format_logging_item_header(char* item_desc, uint16 desc_len, cold_log_level_t log_level)
{
 
    sal_time_t tcurrtime;
    char up_time_desc[COLD_TIME_DESC+1] = {0};
    
    if (!item_desc || desc_len < COLD_MAX_KEY_SIZE)
    {
        return ;
    }

    sal_time(&tcurrtime);
    sal_snprintf(item_desc, desc_len, "%sLevel:%d %s(uptime:%s)",STR_RETURN, log_level,
                sal_ctime( &tcurrtime ), cold_format_uptime_str(up_time_desc)); 
    desc_len = COLD_MIN(desc_len,COLD_MAX_KEY_SIZE);

    if(desc_len < 2)
    {
        return ;
    }
    sal_memcpy(&item_desc[desc_len-2],STR_RETURN,2);
}

static char*
_cold_get_log_name_desc(cold_log_type_t log_type)
{
   static char table_name[COLD_PATH_NAME_LEN+1];
   switch(log_type)
   {
       case COLD_LOG_TYPE_CPU_INFO:
           return "cpu_info.log";
       case COLD_LOG_TYPE_MEM_INFO:
           return "mem_info.log";
       case COLD_LOG_TYPE_ENV_INFO:
           return "env_info.log";
       case COLD_LOG_TYPE_GEN:
           return "gen.log";
       default:
           sal_snprintf(table_name, COLD_PATH_NAME_LEN, "cold_log%d.log", log_type);
           return table_name;   
   }
}

static cold_db_table_id_t 
_cold_get_logging_tbl_id(cold_log_type_t log_type)
{
    switch(log_type)
    {
        case  COLD_LOG_TYPE_CPU_INFO:  
            return COLD_DB_CPU_SYS_INFO_TID;
        case COLD_LOG_TYPE_MEM_INFO:
            return COLD_DB_MEM_SYS_INFO_TID;
        case COLD_LOG_TYPE_ENV_INFO:
            return COLD_DB_ENV_INFO_TID;     
        case COLD_LOG_TYPE_GEN:
            return COLD_DB_GEN_INFO_TID;            
        default:
            return COLD_DB_MAX_TABLE_ID; 
    }
}

static void 
_cold_get_logging_tbl_hdr(cold_log_type_t log_type, cold_db_tbl_header_t *tbl_hdr)
{
    sal_memset(tbl_hdr, 0, sizeof(cold_db_tbl_header_t));
    
    tbl_hdr->handle_flag = COLD_OF_COVER_REC;
    tbl_hdr->key_size    = COLD_MAX_KEY_SIZE;
    tbl_hdr->data_size   = COLD_DEFAULT_REC_DATA_SIZE;
    tbl_hdr->max_item_num = COLD_DEFAULT_MAX_ITEM_NUM;

    switch(log_type)
    {
        case COLD_LOG_TYPE_CPU_INFO:
        case COLD_LOG_TYPE_MEM_INFO:
        case COLD_LOG_TYPE_ENV_INFO:
            tbl_hdr->data_size   = COLD_DEFAULT_REC_DATA_SIZE;
            break;    
        default:
            break;        
    }
}

int 
cold_logging_init(cold_log_type_t log_type)
{
    cold_db_tbl_header_t tbl_hdr;
    cold_db_table_id_t table_id;
    char* buffer  = NULL;
    uint16 num_recs = 0 ;
    uint16 rec_size = 0;
    int ret = 0;
    
    _cold_get_logging_tbl_hdr(log_type, &tbl_hdr);
    table_id = _cold_get_logging_tbl_id(log_type);
    rec_size = tbl_hdr.data_size + tbl_hdr.key_size;

    buffer = XCALLOC(MEM_COLD_LOGGING_BUF, 8*rec_size); 
    if (!buffer)
    {
        CHSM_LOG_ERR("COLD no memory for buffer.");
        return PM_E_NO_MEMORY;
    }
    ret = cold_db_init_table(table_id, &tbl_hdr, COLD_DIR_LOG_NAME, _cold_get_log_name_desc(log_type),
                             buffer, 8*rec_size , &num_recs);
    if (ret < 0)
    {
        CHSM_LOG_ERR("COLD init table failed:%s", get_logging_err_desc(ret));
    }
    
    return ret;
}

void 
cold_logging(cold_log_type_t log_type, cold_log_level_t log_level, const char* fmt, ...)
{
    char data_str[COLD_MAX_REC_DATA_SIZE+1] = {0};
    char hdr_str[COLD_MAX_KEY_SIZE+1] = {0};
    uint32 actLen = 0;
    va_list pvList;

    va_start(pvList, fmt);	
    actLen = sal_vsprintf(data_str, fmt, pvList);
    actLen = COLD_MIN(actLen, COLD_MAX_REC_DATA_SIZE); 

    _cold_format_logging_item_header(hdr_str, COLD_MAX_KEY_SIZE, log_level);

    if (log_level <= COLD_LOG_LEVEL_ERROR)
    {
        if(log_level == COLD_LOG_LEVEL_EMERGENCY)
        {
            cold_db_add_record_to_storage(_cold_get_logging_tbl_id(log_type), hdr_str, COLD_MAX_KEY_SIZE,
                                            data_str,actLen);
        }
        else
        {
            cold_db_add_record(_cold_get_logging_tbl_id(log_type), hdr_str, COLD_MAX_KEY_SIZE,
                                data_str,actLen );
        }
    }

    va_end(pvList);
}

char* 
code_format_cold_cfg(cold_cfg_t* p_cold_cfg, char* str_desc)
{   
    if (!str_desc)
    {
        return NULL;
    }

    sal_snprintf(str_desc, COLD_MAX_LINE_CHAR, "%.3d%.3d%.3d%.3d",
                p_cold_cfg->module_enable, p_cold_cfg->log_bak_num, 
                p_cold_cfg->last_bak_index, p_cold_cfg->reserved);  
    
    return str_desc;
}

int 
cold_parser_cold_cfg(char* str_desc, cold_cfg_t* cold_cfg)
{
    int tmp1 = 0;
    int tmp2 = 0;
    int tmp3 = 0;
    int tmp4 = 0;

    if (!str_desc || !cold_cfg)
    {
        return COLD_DB_INVAILD_PARAM ;      
    }    

    sal_sscanf(str_desc, "%3d%3d%3d%3d",&tmp1, &tmp2, &tmp3, &tmp4);

    if (tmp1)
    {
        cold_cfg->module_enable = 1;
    }
    else
    {
        cold_cfg->module_enable  = 0 ;  
    }
    if(tmp2 > COLD_MAX_BAK_LOG_NUM)
    {
        cold_cfg->log_bak_num = COLD_MAX_BAK_LOG_NUM;
    }
    else
    {
        cold_cfg->log_bak_num = (uint8)tmp2 ; 
    }
    
    if(tmp3 > COLD_MAX_BAK_LOG_NUM)
    {
        cold_cfg->last_bak_index = cold_cfg->log_bak_num;
    }
    else
    {
        cold_cfg->last_bak_index = (uint8)tmp3;
    }   

    cold_cfg->reserved = (uint8)tmp4;
    
    return PM_E_NONE;
}

