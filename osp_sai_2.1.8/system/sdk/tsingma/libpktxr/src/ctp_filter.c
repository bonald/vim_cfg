/****************************************************************************
 * ctp_filter.c  provides all CTP Packet filter Fucntion and Macros define.
 *
 * Copyright: (c)2007 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       Anchor Xie.
 * Date:         2007-4-16.
 * Reason:       First Create.
*****************************************************************************/

/****************************************************************************
 *
 * Header Files
 *
****************************************************************************/
#include "ctp_common.h"
#include "ctp_util.h"

/****************************************************************************
 *
 * Data Structure and Defines
 *
****************************************************************************/

/****************************************************************************
 *
 * Global and Declarations
 *
 ****************************************************************************/

/*
 * This Variablies used for store handled and to be handled command string
 */
static ctp_command_hdr_t flt_left_cmdl;
static uint8     *flt_pld        = NULL;
uint16           pld_len          = 0;
static int g_flt_initialized = 0;
#define M_FLT_INITIALIZED   \
do { \
    if (!g_flt_initialized) \
    { \
        g_flt_initialized = 1; \
        TAILQ_INIT(&flt_left_cmdl); \
    } \
} while (0)


/*===================function declear==================*/
ctp_error_e ctc_pktxr_filter_reset(void);

/****************************************************************************
 *
 * Functions
 *
 ****************************************************************************/
/*******************************************************************************
 * Name   : field_compare
 * Purpose: compare the value between the input cmd and the value from dec_pkt.
 * Input  : packet - input packet ponter.
 * Output : None.
 * Return : .
 * Note   : user need release its own memory.
*******************************************************************************/
static uint8
field_compare(ctp_cmd_t *cmd, uint8 **chk_value)
{
    uint16           field_id = 0, idx = 0;
    ctp_field_info_t *finfo = NULL;
    uint8            *mask = NULL, *value = NULL, *pch = NULL;
    ctp_field_seg_t  *field_seg = NULL;
    char *keyw_str = NULL;

    keyw_str = (char *)ctc_pktxr_ctckal_malloc(strlen(cmd->keyword) + 1);
    if (NULL == keyw_str)
    {
        return CTP_E_MEMORY_ERROR;
    }
    memset(keyw_str, 0, strlen(cmd->keyword) + 1);
    strcpy(keyw_str, cmd->keyword);
    keyw_str++;
    *(keyw_str + strlen(cmd->keyword) - 2) = '\0' ;

    M_FLT_INITIALIZED;

    *chk_value = NULL;

    /* get field ID */
    for(field_id = 0; field_id < FIELD_NUM; field_id++)
    {
        finfo = ctc_pktxr_get_field_info_pointer(field_id);
        if(0 == strcmp(keyw_str, finfo->descriptor))
        {
            break;
        }
    }
    keyw_str--;
    ctc_pktxr_ctckal_free(keyw_str);
    /* faild lookup */
    if(FIELD_NUM == field_id)
    {
        return 0;
    }

    /* get field segment */
    if(CTP_E_NONE != ctc_pktxr_decoder_get_field(field_id, &field_seg)) // got the value from the dec_pkt
    {
        return 0;
    }

    if (field_seg->length <= 0)
    {
        /* ignore control field */
        return 0;
    }

    /* get mask and value */
    mask  = NULL;
    value = NULL;
    pch  = (uint8*)strstr(cmd->value, "@");
    if(NULL == pch)
    {
        value = ctc_pktxr_ctckal_malloc((field_seg->length + 7)/ 8);
        if (NULL == value)
        {
            return CTP_E_MEMORY_ERROR;
        }

        mask  = ctc_pktxr_ctckal_malloc((field_seg->length + 7)/ 8);
        if (NULL == mask)
        {
            ctc_pktxr_ctckal_free(value);
            return CTP_E_MEMORY_ERROR;
        }
        memset(value, 0, ((field_seg->length + 7)/ 8));
        memset (mask, 0, (field_seg->length + 7)/ 8);
        //ctc_pktxr_string2char(cmd->value, value);
        /* here to handle the condition when the length is larger
        than 8 and you set the value string only with strlen 2 and
        move the pointer of value to the correct place   */
        ctc_pktxr_string2char(cmd->value, value + ((field_seg->length + 7)/ 8)
                                      - (strlen(cmd->value) + 1)/2);
    }
    else    // handle the control word
    {
        value = ctc_pktxr_ctckal_malloc((field_seg->length + 7)/ 8);
        if (NULL == value)
        {
            return CTP_E_MEMORY_ERROR;
        }
        mask = ctc_pktxr_ctckal_malloc((field_seg->length + 7)/ 8);
        if (NULL == mask)
        {
            ctc_pktxr_ctckal_free(value);
            return CTP_E_MEMORY_ERROR;
        }
        memset(value, 0, ((field_seg->length + 7)/ 8));
        memset (mask, 0, (field_seg->length + 7)/ 8);
        //ctc_pktxr_string2char(cmd->value, value);
        ctc_pktxr_string2char(cmd->value, value+ ((field_seg->length + 7)/ 8)
                                     - (strlen(cmd->value) + 1)/2);
        ctc_pktxr_string2char((char*)(pch + 1), mask);
    }

    /* here if checking value fail, we will first convert the field_seg->value to str
    and then will set it to the output chk_value */
    for(idx = 0; idx < ((field_seg->length + 7)/ 8); idx ++)
    {
        //if((value[idx] & mask[idx]) != (field_seg->value[idx] & mask[idx]))
        if((value[idx] ) != (field_seg->value[idx] ))
        {
            int str_len;
            uint8 *get_value;
            get_value = NULL;
            str_len = ((field_seg->length + 7)/ 8) * 2 + 1;
            get_value = ctc_pktxr_ctckal_malloc(str_len);
            if (NULL == get_value)
            {
                ctc_pktxr_ctckal_free(mask);
                ctc_pktxr_ctckal_free(value);
                return CTP_E_MEMORY_ERROR;
            }
            memset(get_value, 0, str_len);
            ctc_pktxr_uint2str((char*)get_value, field_seg->value, str_len - 1);
            *chk_value = get_value;
            ctc_pktxr_ctckal_free(mask);
            ctc_pktxr_ctckal_free(value);
            return 0;
        }
    }

    ctc_pktxr_ctckal_free(mask);
    ctc_pktxr_ctckal_free(value);
    return 1;
}

/*******************************************************************************
 * Name   : field_list_compare
 * Purpose: compare the value between the input cmd and the value from dec_pkt.
 * Input  : packet - input packet ponter.
 * Output : None.
 * Return : .
 * Note   : user need release its own memory.
*******************************************************************************/
static uint8
field_list_compare(ctp_cmd_t *cmd, uint8 **chk_value, int repeat_cnt)
{
    uint16           field_id = 0, idx = 0;
    ctp_field_info_t *finfo = NULL;
    uint8            *mask = NULL, *value = NULL, *pch = NULL;
    ctp_field_seg_t  *field_seg = NULL;
    char *keyw_str=NULL;

    keyw_str = (char *)ctc_pktxr_ctckal_malloc(strlen(cmd->keyword) + 1);
        if (NULL == keyw_str)
        {
            return CTP_E_MEMORY_ERROR;
        }
    memset(keyw_str, 0, strlen(cmd->keyword) + 1);
    strcpy(keyw_str, cmd->keyword);
    keyw_str++;
    *(keyw_str + strlen(cmd->keyword) - 2) = '\0' ;

    M_FLT_INITIALIZED;

    *chk_value = NULL;

    /* get field ID */
    for(field_id = 0; field_id < FIELD_NUM; field_id++)
    {
        finfo = ctc_pktxr_get_field_info_pointer(field_id);
        if(0 == strcmp(keyw_str, finfo->descriptor))
        {
            break;
        }
    }
    keyw_str--;
    ctc_pktxr_ctckal_free(keyw_str);

    /* faild lookup */
    if(FIELD_NUM == field_id)
    {
        return 0;
    }

    /* get field segment */
    if(CTP_E_NONE != ctc_pktxr_decoder_get_list_field(field_id, &field_seg, repeat_cnt))  // got the value from the dec_pkt
    {
        return 0;
    }

    if (field_seg->length <= 0)
    {
        /* ignore control field */
        return 0;
    }

    /* get mask and value */
    mask  = NULL;
    value = NULL;
    pch  = (uint8*)strstr(cmd->value, "@");
    if(NULL == pch)
    {
        value = ctc_pktxr_ctckal_malloc((field_seg->length + 7)/ 8);
        if (NULL == value)
        {
            return CTP_E_MEMORY_ERROR;
        }

        mask  = ctc_pktxr_ctckal_malloc((field_seg->length + 7)/ 8);
        if (NULL == mask)
        {
            ctc_pktxr_ctckal_free(value);
            return CTP_E_MEMORY_ERROR;
        }
        memset(value, 0, ((field_seg->length + 7)/ 8));
        memset (mask, 0, (field_seg->length + 7)/ 8);
        //ctc_pktxr_string2char(cmd->value, value);
        /* here to handle the condition when the length is larger
        than 8 and you set the value string only with strlen 2 and
        move the pointer of value to the correct place   */
        ctc_pktxr_string2char(cmd->value, value + ((field_seg->length + 7)/ 8)
                                      - (strlen(cmd->value) + 1)/2);
    }
    else    // handle the control word
    {
        value = ctc_pktxr_ctckal_malloc((field_seg->length + 7)/ 8);
        if (NULL == value)
        {
            return CTP_E_MEMORY_ERROR;
        }
        mask = ctc_pktxr_ctckal_malloc((field_seg->length + 7)/ 8);
        if (NULL == mask)
        {
            ctc_pktxr_ctckal_free(value);
            return CTP_E_MEMORY_ERROR;
        }
        memset(value, 0, ((field_seg->length + 7)/ 8));
        memset (mask, 0, (field_seg->length + 7)/ 8);
        //ctc_pktxr_string2char(cmd->value, value);
        ctc_pktxr_string2char(cmd->value, value+ ((field_seg->length + 7)/ 8)
                                     - (strlen(cmd->value) + 1)/2);
        ctc_pktxr_string2char((char*)(pch + 1), mask);
    }

    /* here if checking value fail, we will first convert the field_seg->value to str
    and then will set it to the output chk_value */
    for(idx = 0; idx < ((field_seg->length + 7)/ 8); idx ++)
    {
        //if((value[idx] & mask[idx]) != (field_seg->value[idx] & mask[idx]))
        if((value[idx] ) != (field_seg->value[idx] ))
        {
            int str_len;
            uint8 *get_value;
            get_value = NULL;
            str_len = ((field_seg->length + 7)/ 8) * 2 + 1;
            get_value = ctc_pktxr_ctckal_malloc(str_len);
            if (NULL == get_value)
            {
                ctc_pktxr_ctckal_free(mask);
                ctc_pktxr_ctckal_free(value);
                return CTP_E_MEMORY_ERROR;
            }
            memset(get_value, 0, str_len);
            ctc_pktxr_uint2str((char*)get_value, field_seg->value, str_len - 1);
            *chk_value = get_value;
            ctc_pktxr_ctckal_free(mask);
            ctc_pktxr_ctckal_free(value);
            return 0;
        }
    }

    ctc_pktxr_ctckal_free(mask);
    ctc_pktxr_ctckal_free(value);
    return 1;
}


#if 0  // zhao feng 2007-5-25
/*******************************************************************************
 * Name   : packet_filter
 * Purpose: decode the packet and save the result to dec_pkt.
 * Input  : packet - input packet ponter.
 * Output : None.
 * Return : .
 * Note   : user need release its own memory.
*******************************************************************************/

ctp_error_e
packet_filter(uint8 *packet, uint8 *match)
{
    ctp_cmd_t *temp;
    int       ret;
    uint16    pay_len;
    uint8     *dpayload;
    uint8     **chk_value;

    *match    = 0;
    /* copy command to decoder */
    temp       = flt_left_cmdl;
    while (NULL != temp)
    {
        CTP_ERR_RETURN(decode_command_set(temp->keyword,
            temp->value), "packet_filter*decode_command_set");
        temp   = temp->next;
    }

    /* packet decoder */
    CTP_ERR_RETURN(packet_decoder(packet), "packet_filter*packet_decoder");

    /* per command handle */
    temp = flt_left_cmdl;  // the cmd list user set to check the field value
    while(NULL != temp)
    {
        if(NULL == strstr(temp->keyword, "@"))
        {
            /* normal field handle */
            if (!field_compare(temp, chk_value))  // when check value fail directly return and math is 0
            {
                return CTP_E_NONE;
            }
        }

        temp = temp->next;
    }

    /* payload handle */
    if(0 != pld_len)
    {
        decoder_get_payload(&pay_len, &dpayload);
        if(pld_len != pay_len)
        {
            CTP_ERR_RETURN(CTP_E_LESS_LEN, "filter payload");
        }

        for(ret = 0; ret < pld_len; ret ++)
        {
            if (flt_pld[ret] != dpayload[ret])
            {
                return CTP_E_NONE;
            }
        }
    }

    *match    = 1;
    return CTP_E_NONE;
}
#endif
/*******************************************************************************
 * Name   : packet_filter_value
 * Purpose: use flt_left_cmdl to give the value user set and use decode_command_set
 * to handle the control word and UDT and packet_decoder(packet) is to get the dec_pkt
 * then field_compare is called and it this function it first get the field_id according to the
 * cmd and then it will get the field_seg then the cmd->value(user set) and field_seg->value
 * will be compared and the field_seg->value will be returned.
 * Input  : packet - input packet ponter.
 * Output : match 1 represent check succeed and got_value will be set.
 * Return :
 * Note   : user need release its own memory.
*******************************************************************************/
int32
ctc_pktxr_packet_filter_value(uint8 *packet, uint16 *match, uint8 **got_value)
{
    ctp_cmd_t *temp = NULL;
    ctp_cmd_t *sub_temp = NULL;
    int       ret = 0;
    uint16    pay_len = 0;
    uint8     *dpayload = NULL;
    uint8     *chk_value = NULL;

    M_FLT_INITIALIZED;

    /* copy command to decoder */
    TAILQ_FOREACH(temp, &flt_left_cmdl, entry)
    {
        CTP_ERR_RETURN(ctc_pktxr_decode_command_set(temp->keyword,
            temp->value), "packet_filter*decode_command_set");
    }

    /* packet decoder */
    CTP_ERR_RETURN(ctc_pktxr_packet_decoder(packet), "packet_filter*packet_decoder");

    /* default is match */
    *match = 1;

    /* per command handle */
    TAILQ_FOREACH(temp, &flt_left_cmdl, entry)
    {
        int rep_cnt = 0;

        if (NULL == strstr(temp->keyword, "@"))
        {

            /* compare the primary value */
            if (!field_compare(temp, &chk_value))  // when check value fail directly return and math is 0
            {
                *match = 0;

                if (NULL == chk_value)
                {
                    printf("Check %s", temp->keyword);
                    printf(" FAIL: no such field\n");
                    continue;
                }

                /* just record the fist value */
                printf("Check %s", temp->keyword);
                printf(" FAIL: the value you set is wrong and should be: %s\n", chk_value);
                if (NULL == *got_value)
                {
                    *got_value = chk_value;
                }
                else
                {
                    ctc_pktxr_ctckal_free(chk_value);
                }
            }
            else
            {
                /* just record the first value */
                if (NULL == *got_value)
                {
                    *got_value = (uint8*)ctc_pktxr_ctckal_strdup(temp->value);
                    if (NULL == *got_value)
                    {
                        printf("Check %s", temp->keyword);
                        printf(" FAIL: out of memory\n");
                        ctc_pktxr_filter_reset();
                        return CTP_E_MEMORY_ERROR;
                    }
                }
                printf("Check %s", temp->keyword);
                printf(" PASS: the check right value is : %s\n", temp->value);
            }

            if (TAILQ_EMPTY(&temp->list))
            {
                continue;
            }

            TAILQ_FOREACH(sub_temp, &temp->list, entry)
            {
                rep_cnt++;
                if (!field_list_compare(sub_temp, &chk_value, rep_cnt)) // when check value fail directly return and math is 0
                {
                    *match = 0;

                    if (NULL == chk_value)
                    {
                        printf("Check %s", sub_temp->keyword);
                        printf(" FAIL: no such field\n");
                        continue;
                    }

                    /* just record the fist value */
                    printf("Check %s", sub_temp->keyword);
                    printf(" FAIL: the value you set is wrong and should be: %s\n", chk_value);
                    if (NULL == *got_value)
                    {
                        *got_value = chk_value;
                    }
                    else
                    {
                        ctc_pktxr_ctckal_free(chk_value);
                    }
                }
                else
                {
                    /* just record the first value */
                    if (NULL == *got_value)
                    {
                        *got_value = (uint8*)ctc_pktxr_ctckal_strdup(sub_temp->value);
                        if (NULL == *got_value)
                        {
                            printf("Check %s", sub_temp->keyword);
                            printf(" FAIL: out of memory\n");
                            ctc_pktxr_filter_reset();
                            return CTP_E_MEMORY_ERROR;
                        }
                    }
                    printf("Check %s", sub_temp->keyword);
                    printf(" PASS: the check right value is : %s\n", sub_temp->value);
                }
            }
        }
    }

    /* payload handle */
    if(0 != pld_len)
    {
        ctc_pktxr_decoder_get_payload(&pay_len, &dpayload);
        if(pld_len != pay_len)
        {
            CTP_ERR_RETURN(CTP_E_LESS_LEN, "filter payload");
        }

        for(ret = 0; ret < pld_len; ret ++)
        {
            if (flt_pld[ret] != dpayload[ret])
            {
                *match = 0;
                ctc_pktxr_filter_reset();
                return CTP_E_NONE;
            }
        }
    }

    ctc_pktxr_filter_reset();
    return CTP_E_NONE;
}

#if 0
ctp_error_e
ctc_pktxr_packet_get_valuelist(uint8 *packet, char *got_value ,uint8 *key_word ,uint8 *key_value ,uint8 number)
{
    int       ret = 0;
    ctp_cmd_t *temp = NULL;
    temp = (ctp_cmd_t *)ctc_pktxr_ctckal_malloc(sizeof(ctp_cmd_t));
    temp->keyword = (char*)key_word;
    temp->value = (char*)key_value;
    uint8     *sou_value = NULL;
    char     *dest_value = got_value;

    CTP_ERR_RETURN(ctc_pktxr_decode_command_set(temp->keyword,
            temp->value), "packet_filter*decode_command_set");
      /* packet decoder */
    CTP_ERR_RETURN(ctc_pktxr_packet_decoder(packet), "packet_filter*packet_decoder");
    if (!field_compare(temp, &sou_value))
    {
        if (NULL == sou_value)
          {
             printf("Check %s", temp->keyword);
             printf(" FAIL: no such field\n");
             ctc_pktxr_ctckal_free(temp);
             return CTP_E_MIS_FIELD;
          }
          memcpy (dest_value, (char*)sou_value ,strlen((char*)sou_value));
          dest_value += strlen((char*)sou_value);
          ctc_pktxr_ctckal_free(sou_value);
    }
    else
    {
           printf ("\n C code have something wrong ,need debug!\n");
           ctc_pktxr_ctckal_free(sou_value);
    }

    for(ret=1;ret< number;ret++)
    {
        if (!field_list_compare(temp, &sou_value, ret)) // when check value fail directly return and math is 0
        {

          if (NULL == sou_value)
          {
              ctc_pktxr_ctckal_free(temp);
             return CTP_E_MIS_FIELD;
          }
          memcpy (dest_value, sou_value ,strlen((char*)sou_value));
          dest_value += strlen((char*)sou_value);
          ctc_pktxr_ctckal_free(sou_value);

        }
        else
        {
           printf ("\n C code have something wrong ,need debug!\n");
           ctc_pktxr_ctckal_free(sou_value);
        }
    }

    ctc_pktxr_ctckal_free(temp);
    return CTP_E_NONE;
}
#endif

/*******************************************************************************
 * Name   : decoder_reset
 * Purpose: reset decorder command.
 * Input  : keyw - keyword of field set or command set.
 *        : value- value of field set or command set
 * Output : None.
 * Return : .
 * Note   : N/A
*******************************************************************************/
ctp_error_e
ctc_pktxr_filter_reset(void)
{
    ctp_cmd_t *temp = NULL;
    ctp_cmd_t *psub = NULL;

    M_FLT_INITIALIZED;

    /* free left command list */
    while(!TAILQ_EMPTY(&flt_left_cmdl))
    {
        temp = TAILQ_FIRST(&flt_left_cmdl);
        TAILQ_REMOVE(&flt_left_cmdl, temp, entry);
        while (!TAILQ_EMPTY(&temp->list))
        {
            psub = TAILQ_FIRST(&temp->list);
            TAILQ_REMOVE(&temp->list, psub, entry);
            ctc_pktxr_ctckal_free(psub->keyword);
            ctc_pktxr_ctckal_free(psub->value);
            ctc_pktxr_ctckal_free(psub);
        }
        ctc_pktxr_ctckal_free(temp->keyword);
        ctc_pktxr_ctckal_free(temp->value);
        ctc_pktxr_ctckal_free(temp);
    }
    TAILQ_INIT(&flt_left_cmdl);

    /* payload release */
    if (NULL != flt_pld)
    {
        ctc_pktxr_ctckal_free(flt_pld);
    }
    flt_pld = NULL;
    pld_len = 0;

    return CTP_E_NONE;
}


/*******************************************************************************
 * Name   : filter_command_set
 * Purpose: handle filter command.
 * Input  : keyw - keyword of field set or command set.
 *        : value- value of field set or command set data & mask
 * Output : None.
 * Return : .
 * Note   : user need release its own memory
*******************************************************************************/
ctp_error_e
ctc_pktxr_filter_command_set(const char *keyw, const char *value)
{
    char      *cmd_str = NULL;
    ctp_cmd_t *cmd_list = NULL;
    ctp_cmd_t *cmd_tmp = NULL;
    int       ret;

    M_FLT_INITIALIZED;

    cmd_str = (char *)ctc_pktxr_ctckal_malloc(strlen(keyw) + 1);
    if (NULL == cmd_str)
    {
        return CTP_E_MEMORY_ERROR;
    }

    memset(cmd_str, 0, strlen(keyw) + 1);
    ctc_pktxr_convert_to_upper(cmd_str, keyw);

    if(strstr(cmd_str, "@RESET_FILTER"))
    {
        ctc_pktxr_ctckal_free(cmd_str);
        /* free memories for old packet */
        CTP_ERR_RETURN(ctc_pktxr_filter_reset(), "filter_reset");
        return CTP_E_NONE;
    }

    if (strstr(cmd_str, "@PLD_STR"))
    {
        ctc_pktxr_ctckal_free(cmd_str);

        pld_len = (strlen(value) + 1)/2;
        flt_pld = (uint8 *)ctc_pktxr_ctckal_malloc(pld_len);
        if (NULL == flt_pld)
        {
            return CTP_E_MEMORY_ERROR;
        }
        ctc_pktxr_string2char((char *)value, flt_pld);
        return CTP_E_NONE;
    }

    cmd_list =  (ctp_cmd_t *) ctc_pktxr_ctckal_calloc(1, sizeof(ctp_cmd_t));
    if (NULL == cmd_list)
    {
        ctc_pktxr_ctckal_free(cmd_str);
        return CTP_E_MEMORY_ERROR;
    }
    TAILQ_INIT(&cmd_list->list);
    cmd_list->keyword = cmd_str;
    cmd_str = (char *)ctc_pktxr_ctckal_malloc(strlen(value) + 1);
    if (NULL == cmd_str)
    {
        ctc_pktxr_ctckal_free(cmd_list->keyword);
        ctc_pktxr_ctckal_free(cmd_list);
        return CTP_E_MEMORY_ERROR;
    }

    strcpy(cmd_str, value);
    cmd_list->value = cmd_str;

    TAILQ_FOREACH_REVERSE(cmd_tmp, &flt_left_cmdl, tagctp_command_hdr_t, entry)
    {
        if (strstr(cmd_tmp->keyword, keyw))
        {
            break;
        }
    }
    if (NULL != cmd_tmp)
    {
        TAILQ_INSERT_TAIL(&cmd_tmp->list, cmd_list, entry);
    }
    else
    {
        TAILQ_INSERT_TAIL(&flt_left_cmdl, cmd_list, entry);
    }

    return CTP_E_NONE;
}

