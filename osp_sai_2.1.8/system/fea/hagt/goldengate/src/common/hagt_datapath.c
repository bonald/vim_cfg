/****************************************************************************
 * hsrv_vlan.c     :  Hal server datapath related function
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       weij
 * Date         :       2010-07-12
 * Reason       :       First Create.
 ****************************************************************************/

/****************************************************************************
*
* Header Files
*
****************************************************************************/
#include "hagt_inc.h"
#include "ctc_api.h"
#include "ctc_debug.h"
#include "ctc_interrupt.h"
#include "hagt_datapath_priv.h"

int32
_hagt_app_parse_file_fseek(ctc_app_parse_file_t* p_ctc_parsr_file, int32 pos, int32 dir)
{
    int32 cur_pos = 0;
    cur_pos = p_ctc_parsr_file->pos;
    
    switch(dir)
    {
        case 0:
            p_ctc_parsr_file->pos = pos;
            break;
        case 1:
            p_ctc_parsr_file->pos += pos;
            break;
        case 2:
            p_ctc_parsr_file->pos = p_ctc_parsr_file->len - pos;
            break;
    }
    
    return cur_pos;
}

char*
_hagt_app_parse_file_fgets(char* s, int n, ctc_app_parse_file_t *p_file)
{
    int32 copy_len = 0;
    if(NULL == p_file)
    {
        return NULL;
    }
        
    if(_hagt_app_parse_file_feof(p_file))
    {
        return NULL;    
    }
         
    while(copy_len < n)
    {
        if(p_file->pos == p_file->len)
        {
            break;
        }
        
        *(s + copy_len) = *(((char*)p_file->mem_addr) + p_file->pos);
        p_file->pos++;
        copy_len++;
        
        if('\n' == *(s + copy_len - 1))
        {
            break;
        }
    }
    
    return s;
}

static int32
_hagt_app_string_atrim(char* output, const char* input)
{
    char* p = NULL;

    /*trim left space*/
    while (*input != '\0')
    {
        if (WHITE_SPACE(*input))
        {
            ++input;
        }
        else
        {
            break;
        }
    }

    strcpy(output, input);
    /*trim right space*/
    p = output + strlen(output) - 1;

    while (p >= output)
    {
        /*skip empty line*/
        if (WHITE_SPACE(*p) || ('\r' == (*p)) || ('\n' == (*p)))
        {
            --p;
        }
        else
        {
            break;
        }
    }

    *(++p) = '\0';

    return 0;
}

static int32
_hagt_app_get_interage(const char* string, uint32* integer)
{
    char* ch = NULL;
    uint32 val = 0;

    ch = strstr((char*)string, "=");

    if (NULL == ch)
    {
        return -1;
    }
    else
    {
        ch++;
    }

    while (sal_isspace(*ch))
    {
        ch++;
    }

    sscanf((char*)ch, "%d", &val);
    *integer = val;

    return 0;
}

static int32
_hagt_app_do_parse(ctc_app_parse_t* p_app_parse, ctc_app_parse_file_t* p_file)
{
    char string[64] = "";
    char line[64]   = "";
    
    int  err_map[CTC_APP_PARSE_RET_MAX] = {CTC_E_ENTRY_NOT_EXIST,
                                           CTC_E_NONE,
                                           CTC_E_ENTRY_NOT_EXIST};
    
    int32 ret = CTC_APP_PARSE_RET_NOT_FIND;
    
    _hagt_app_parse_file_fseek(p_file, 0, SEEK_SET);
    
    while (!_hagt_app_parse_file_feof(p_file))
    {
        sal_memset(string, 0, sizeof(string));
        sal_memset(line, 0, sizeof(line));
        _hagt_app_parse_file_fgets(string, sizeof(string), p_file);

        /*comment line*/
        if ('#' == string[0] && '{' != string[1] && '}' != string[1])
        {
            continue;
        }

        if (EMPTY_LINE(string[0]))
        {
            continue;
        }

        _hagt_app_string_atrim(line, string);
        
        if (EMPTY_LINE(line[0]))
        {
            continue;
        }
        
        if(ret = p_app_parse->func(line, p_app_parse),ret != CTC_APP_PARSE_RET_CONTINUE)
        {
            break;
        }
    }
    
    *p_app_parse->ret_p_entry_num = p_app_parse->cur_entry_num;
    
    return err_map[ret];
}


static int32 
_hagt_app_do_cfg_file(const char* line, struct ctc_app_parse_s* p_app_parse)
{
    if(CTC_FLAG_ISSET(p_app_parse->parse_flag,CTC_APP_PARSE_FLAG_GLOBAL))
    {
        if(sal_strncmp(line,p_app_parse->field,strlen(p_app_parse->field)) == 0)
        {
            _hagt_app_get_interage(line, p_app_parse->value);
            
            p_app_parse->cur_entry_num++;
            
            return CTC_APP_PARSE_RET_FIND;
        }
    }
    
    return CTC_APP_PARSE_RET_CONTINUE;
}

static int32 
_hagt_app_do_table_cfg_file(const char* line, struct ctc_app_parse_s* p_app_parse)
{
    if(CTC_FLAG_ISSET(p_app_parse->parse_flag,CTC_APP_PARSE_FLAG_TABLE_LEAVE))
    {
        if(sal_strncmp(line, "#}", strlen("#}")) == 0)
        {
            CTC_UNSET_FLAG(p_app_parse->parse_flag,CTC_APP_PARSE_FLAG_TABLE_LEAVE);
            CTC_SET_FLAG(p_app_parse->parse_flag,CTC_APP_PARSE_FLAG_GLOBAL);
        }
        return CTC_APP_PARSE_RET_CONTINUE;
    }
    
    if(CTC_FLAG_ISSET(p_app_parse->parse_flag,CTC_APP_PARSE_FLAG_TABLE_ENTER))
    {
        if(sal_strncmp(line,p_app_parse->field,strlen(p_app_parse->field)) == 0)
        {
            CTC_SET_FLAG(p_app_parse->parse_flag,CTC_APP_PARSE_FLAG_TABLE);
            CTC_UNSET_FLAG(p_app_parse->parse_flag,CTC_APP_PARSE_FLAG_TABLE_ENTER);
        }else{
            CTC_SET_FLAG(p_app_parse->parse_flag,CTC_APP_PARSE_FLAG_TABLE_LEAVE);
            CTC_UNSET_FLAG(p_app_parse->parse_flag,CTC_APP_PARSE_FLAG_TABLE_ENTER);
        }
        return CTC_APP_PARSE_RET_CONTINUE;
    }

    if(CTC_FLAG_ISSET(p_app_parse->parse_flag,CTC_APP_PARSE_FLAG_GLOBAL))
    {
        if(sal_strncmp(line, "#{", strlen("#{")) == 0)
        {
            CTC_SET_FLAG(p_app_parse->parse_flag,CTC_APP_PARSE_FLAG_TABLE_ENTER);
            CTC_UNSET_FLAG(p_app_parse->parse_flag,CTC_APP_PARSE_FLAG_GLOBAL);
            
            return CTC_APP_PARSE_RET_CONTINUE;
        }
    }
    
    if(CTC_FLAG_ISSET(p_app_parse->parse_flag,CTC_APP_PARSE_FLAG_TABLE))
    {
        if(sal_strncmp(line, "#}", strlen("#}")) == 0)
        {
            CTC_UNSET_FLAG(p_app_parse->parse_flag,CTC_APP_PARSE_FLAG_TABLE);
            CTC_SET_FLAG(p_app_parse->parse_flag,CTC_APP_PARSE_FLAG_GLOBAL);
            
            return CTC_APP_PARSE_RET_FIND;
        }
        
        if(sal_strncmp(line,p_app_parse->field_sub,strlen(p_app_parse->field_sub)) == 0)
        {
            if(p_app_parse->cur_entry_num >= p_app_parse->max_entry_num)
                return CTC_APP_PARSE_RET_NOT_FIND;
                
            _hagt_app_get_interage(line, ((uint32*)p_app_parse->value) + p_app_parse->cur_entry_num);
            p_app_parse->cur_entry_num++;
        }
    }
    
    return CTC_APP_PARSE_RET_CONTINUE;
}

int32
hagt_app_parse_file(ctc_app_parse_file_t* p_file, 
                       const char* field, 
                       const char* field_sub, 
                       void* value, 
                       uint8 *p_entry_num)
{
    char field_tmp[64];
    char field_sub_tmp[64];
    ctc_app_parse_t app_parse;
    
    HAGT_CHECK_PTR(p_file);
    HAGT_CHECK_PTR(field);
    HAGT_CHECK_PTR(value);
    HAGT_CHECK_PTR(p_entry_num);
    
    sal_memset(&app_parse, 0, sizeof(ctc_app_parse_t));
    sal_memset(&field_tmp, 0, sizeof(field_tmp));
    sal_memset(&field_sub_tmp, 0, sizeof(field_sub_tmp));
    
    sal_strcat(field_tmp, "[");
    sal_strcat(field_tmp, field);
    sal_strcat(field_tmp, "]");
    
    app_parse.parse_flag= CTC_APP_PARSE_FLAG_GLOBAL;
    app_parse.field     = field_tmp;
    app_parse.field_sub = NULL;
    app_parse.value     = value;
    app_parse.cur_entry_num = 0;
    app_parse.max_entry_num = *p_entry_num;
    app_parse.ret_p_entry_num = p_entry_num;
    
    if(NULL == field_sub)
    {
        app_parse.func = _hagt_app_do_cfg_file;
    }else{
        sal_strcat(field_sub_tmp, "[");
        sal_strcat(field_sub_tmp, field_sub);
        sal_strcat(field_sub_tmp, "]");
        app_parse.field_sub = field_sub_tmp;
        app_parse.func = _hagt_app_do_table_cfg_file;
    }
    
    return _hagt_app_do_parse(&app_parse, p_file);
}

int32
hagt_app_parse_open_file(const char* file_name, ctc_app_parse_file_t* p_file)
{
    FILE* local_fp = NULL;
    
    HAGT_CHECK_PTR(p_file);
    
    local_fp = sal_fopen(file_name, "r");
    
    if (NULL == local_fp)
    {
        return CTC_E_UNEXPECT;
    }
    
    p_file->p_file = local_fp;
    
    fseek(p_file->p_file, 0, SEEK_END);
    p_file->len = ftell(p_file->p_file);
    fseek(p_file->p_file, 0, SEEK_SET);
    
    p_file->mem_addr = mmap(NULL, p_file->len, PROT_READ, MAP_PRIVATE, fileno(p_file->p_file), 0);
    
    if(MAP_FAILED == p_file->mem_addr)
    {
        sal_fclose(p_file->p_file);
        ctc_app_parse_file_init(p_file);
        
        return CTC_E_NO_MEMORY;
    }
    
    return CTC_E_NONE;
}

int32
hagt_app_parse_close_file(ctc_app_parse_file_t* p_file)
{
    HAGT_CHECK_PTR(p_file);
    
    if(p_file->mem_addr)
    {
        munmap(p_file->mem_addr, p_file->len);
        p_file->mem_addr = NULL;
    }

    if(p_file->p_file)
    {
        sal_fclose(p_file->p_file);
        p_file->p_file = NULL;
    }
    
    return CTC_E_NONE;
}


int32
hagt_app_read_datapath_profile_pll_info(ctc_app_parse_file_t* pfile, ctc_datapath_global_cfg_t* p_datapath_cfg)
{
    uint8  entry_num = 0;
    int32  value = 0;
    const char* field_sub = NULL;

    /* get PLLA core frequency*/
    entry_num = 1;
    hagt_app_parse_file(pfile,
                   "CORE_PLLA",
                   field_sub,
                   &value,
                   &entry_num);
    p_datapath_cfg->core_frequency_a = value;

    /* get PLLB core frequency*/
    entry_num = 1;
    hagt_app_parse_file(pfile,
                   "CORE_PLLB",
                   field_sub,
                   &value,
                   &entry_num);
    p_datapath_cfg->core_frequency_b = value;

    return CTC_E_NONE;
}

int32
hagt_app_read_datapath_profile_serdes_info(ctc_app_parse_file_t* pfile, ctc_datapath_global_cfg_t* p_datapath_cfg)
{
    uint8    entry_num = CTC_DATAPATH_SERDES_NUM;
    uint8    cnt = 0;
    static uint32   rx_polarity[CTC_DATAPATH_SERDES_NUM] = {0};
    static uint32   tx_polarity[CTC_DATAPATH_SERDES_NUM] = {0};
    static uint32   is_dynamic[CTC_DATAPATH_SERDES_NUM] = {0};
    static uint32   serdes_mode[CTC_DATAPATH_SERDES_NUM] = {0};
    static uint32   serdes_id[CTC_DATAPATH_SERDES_NUM] = {0} ;
    int      ret = 0;

    entry_num = CTC_DATAPATH_SERDES_NUM;
    ret = hagt_app_parse_file(pfile,
                       "SERDES_ITEM",
                       "SERDES_ID",
                       serdes_id,
                       &entry_num);

    ret = hagt_app_parse_file(pfile,
                       "SERDES_ITEM",
                       "SERDES_MODE",
                       serdes_mode,
                       &entry_num);

    ret = hagt_app_parse_file(pfile,
                       "SERDES_ITEM",
                       "SERDES_RX_POLY",
                       rx_polarity,
                       &entry_num);

    ret = hagt_app_parse_file(pfile,
                       "SERDES_ITEM",
                       "SERDES_TX_POLY",
                       tx_polarity,
                       &entry_num);

    ret = hagt_app_parse_file(pfile,
                       "SERDES_ITEM",
                       "SERDES_SWITCH",
                       is_dynamic,
                       &entry_num);

    for(cnt = 0; cnt < CTC_DATAPATH_SERDES_NUM; cnt++)
    {
        p_datapath_cfg->serdes[cnt].mode = serdes_mode[cnt];
        p_datapath_cfg->serdes[cnt].rx_polarity = rx_polarity[cnt];
        p_datapath_cfg->serdes[cnt].tx_polarity = tx_polarity[cnt];
        p_datapath_cfg->serdes[cnt].is_dynamic= is_dynamic[cnt];
    }

    return ret;
}

int32
hagt_app_get_datapath_profile(uint8* file_name, ctc_datapath_global_cfg_t* p_datapath_cfg)
{
    int32   ret;
    ctc_app_parse_file_t file;

    HAGT_CHECK_PTR(file_name);
    HAGT_CHECK_PTR(p_datapath_cfg);

    ret = hagt_app_parse_open_file((char *)file_name, &file);
    if (ret != CTC_E_NONE)
    {
        return ret;
    }

    hagt_app_read_datapath_profile_pll_info(&file, p_datapath_cfg);

    hagt_app_read_datapath_profile_serdes_info(&file, p_datapath_cfg);

    hagt_app_parse_close_file(&file);

    return CTC_E_NONE;
}


