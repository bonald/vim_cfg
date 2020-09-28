/****************************************************************************
 * shal_vlan.h :  Macro, data structure for hal server vlan module
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision :   R0.01
 * Author   :   weij
 * Date     :   2010-7-20
 ****************************************************************************/

#ifndef __HAGT_DATAPATH_PRIV_H__
#define __HAGT_DATAPATH_PRIV_H__

#define _hagt_app_parse_file_feof(p_ctc_parsr_file) ((p_ctc_parsr_file)->pos >= (p_ctc_parsr_file)->len ? 1:0)
#define EMPTY_LINE(C)     ((C) == '\0' || (C) == '\r' || (C) == '\n')
#define WHITE_SPACE(C)    ((C) == '\t' || (C) == ' ')
typedef struct ctc_app_parse_file_s
{
    FILE* p_file;
    void* mem_addr;
    int32 pos;
    int32 len;
}ctc_app_parse_file_t;

enum{
    CTC_APP_PARSE_FLAG_GLOBAL = 0x01,
    CTC_APP_PARSE_FLAG_TABLE  = 0x02,
    CTC_APP_PARSE_FLAG_TABLE_ENTER = 0x04,
    CTC_APP_PARSE_FLAG_TABLE_LEAVE = 0x08,
};

enum{
    CTC_APP_PARSE_RET_CONTINUE = 0,
    CTC_APP_PARSE_RET_FIND,
    CTC_APP_PARSE_RET_NOT_FIND,
    CTC_APP_PARSE_RET_MAX,
};

typedef struct ctc_app_parse_s
{
    char* field;
    char* field_sub;
    void* value;
    uint8* ret_p_entry_num;
    uint8  cur_entry_num;
    uint8  max_entry_num;
    int32 (*func)(const char*, struct ctc_app_parse_s*);
    int32 parse_flag;
}ctc_app_parse_t;

#define ctc_app_parse_file_init(p_ctc_parsr_file) \
{                                                 \
    (p_ctc_parsr_file)->p_file   = NULL;          \
    (p_ctc_parsr_file)->mem_addr = NULL;          \
    (p_ctc_parsr_file)->pos      = 0;             \
    (p_ctc_parsr_file)->len      = 0;             \
}

#endif /*__HAGT_DATAPATH_PRIV_H__*/

