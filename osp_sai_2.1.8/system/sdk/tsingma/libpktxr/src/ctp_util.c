/****************************************************************************
 * ctp_util.h  provides all CTP common Utility and Macros define.
 *
 * Copyright: (c)2007 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       Anchor Xie.
 * Date:         2007-4-13.
 * Reason:       First Create.
*****************************************************************************/

#include "malloc.h"
/****************************************************************************
 *
 * Header Files
 *
****************************************************************************/
#include "ctp_common.h"

uint32 cts_dbg_flag;

static int g_search_seq = E_SEARCH_NORMAL;
void
ctc_pktxr_set_search_sequence(E_SearchSeq eseq)
{
    g_search_seq = eseq;
    return;
}

E_SearchSeq
ctc_pktxr_get_search_sequence(void)
{
    return g_search_seq;
}

/****************************************************************************
 *
 * Data Structure and Defines
 *
****************************************************************************/
#define is_a_letter(ch)         ((toupper(ch)>='A')&&(toupper(ch)<='Z'))
#define CRCPOLY_LITTLE_ENDIAN  0xedb88320
#define CRCPOLY_BIG_ENDIAN     0x04c11db7

/****************************************************************************
 *
 * Global and Declarations
 *
 ****************************************************************************/
 char debug = 1;
/*
 * These Arraies used for store control word of the encording packet
 */
static char *control_word[] =
{
    "@MTU",
    "@EN_8022",
    "@PKT_LEN",
    "@PKT_TYPE",
    "@REPEAT",
    "@NEXT_TYPE",
    "@FLEN_FID",
    "@FIEL_LEN",
    "@PLD_OFFSET",
    "@PLD_LEN",
    "@PKT_CRC"
};

/*
 * These Arraies used for store error code string
 */
static char *err_string[] =
{
    "CTP_E_NONE",
    "CTP_E_MIS_FIELD",
    "CTP_E_LESS_LEN",
    "CTP_E_MEMORY_ERROR",
    "CTP_E_EMPTY",
    "CTP_E_EXISTS",
    "CTP_E_MODIFY_FAILED",
    "CTP_E_NUM",
};

/****************************************************************************
 *
 * Functions
 *
 ****************************************************************************/
/*******************************************************************************
 * Name   : get_current_maths_sign
 * Purpose: handle the exp and get the math sign, the post is the exp after the math sign
 * Input  : exp
 * Output : the sign is the math sign
 * Return : .
 * Note   : N/A
*******************************************************************************/
static void
get_current_maths_sign(char *exp, char **post, char *sign)
{
    char *curr_offset;
    char *sign_offset = exp + strlen(exp);
    char *symbl[] = {"+", "-", "*", "/", "&", "|", "]"};
    int  idx;

    for(idx = 0; idx < 7; idx ++)
    {
        curr_offset = strstr(exp, symbl[idx]);
        if((NULL != curr_offset) && (curr_offset < sign_offset))
        {
            sign_offset = curr_offset;
            sscanf(curr_offset,"%c", sign);
            *post = sign_offset;
        }
    }
}

/*******************************************************************************
 * Name   : calculate_result
 * Purpose: get the first maths sign.
 * Input  : opt: the exp before the sign(include the sign) sign: the beforehand sign
 * Input  : fiedl_seg: the list for looking up the field value  cw: for looking up the control word
 * Output : result: the calculated result.
 * Return : .
 * Note   : N/A
*******************************************************************************/
static void
calculate_result(uint32 *result, char *opt, char sign,
                    ctp_field_seg_hdr_t *field_seg, uint16 *cw)
{
    int     temp = 0;
    char    *pch0 = NULL, *pch1 = NULL;
    char    str[30];
    char    buf[128];
    E_SearchSeq eseq = ctc_pktxr_get_search_sequence();

    pch0  = strstr(opt,"$"); // handle the field value
    if(NULL != pch0)
    {
        ctp_field_seg_t  *seg = NULL;
        ctp_field_info_t *finfo = NULL;

        pch1 = strstr((pch0 + 1),"$");
        memset(str, 0, 30);
        memcpy(str, pch0, pch1 - pch0);
        if (E_SEARCH_NORMAL == eseq)
        {
            TAILQ_FOREACH(seg, field_seg, entry)
            {
                finfo = ctc_pktxr_get_field_info_pointer(seg->field_id);
                if (NULL != strstr(str, finfo->descriptor))
                {
                    int len;

                    len = ((seg->old_len + 7)/8) * 2 + 1; // string is two times the char
                    if (1 == len)    // add this to handle the repeat num for the len is 1
                    {
                        len = len + 2;
                    }
                    else if (len >= 127)
                    {
                        len = 127;
                    }
                    ctc_pktxr_uint2str(buf, seg->value, len - 1); // get the string value
                    sscanf(buf, "%x", &temp);
                    break;
                }
            }
        }
        else
        {
            TAILQ_FOREACH_REVERSE(seg, field_seg, tagctp_field_seg_hdr_t, entry)
            {
                finfo = ctc_pktxr_get_field_info_pointer(seg->field_id);
                if (NULL != strstr(str, finfo->descriptor))
                {
                    int len;

                    len = ((seg->old_len+ 7)/8) * 2 + 1; // string is two times the char
                    if (1 == len)    // add this to handle the repeat num for the len is 1
                    {
                        len = len + 2;
                    }
                    else if (len >= 127)
                    {
                        len = 127;
                    }
                    ctc_pktxr_uint2str(buf, seg->value, len - 1); // get the string value
                    sscanf(buf, "%x", &temp);
                    break;
                }
            }
        }
        goto END;
    }

    pch0  = strstr(opt,"@"); // handle the control word
    if(NULL != pch0)
    {
        int   idx;

        pch1  = strstr((pch0 + 1),"@");
        memset(str, 0, 30);
        memcpy(str, pch0, pch1 - pch0);
        for(idx = 0;idx < CONTROL_WORD_NUM; idx ++)
        {
            if(NULL != strstr(str, control_word[idx]))
            {
                temp = (uint32)cw[idx]; // if find, get the control word value
                break;
            }
        }
        goto END;
    }

    pch0  = strstr(opt,"#"); //handle the absolute value
    if(NULL != pch0)
    {
        sscanf(pch0 + 1, "%x", &temp);
    }

END:    // record the result last exp calculated and used for this current calculation
    switch(sign)
    {
        case '\0':
            *result = temp;
            break;

        case '+':
            *result = *result + temp;
            break;

         case '-':
            *result = *result - temp;
            break;

        case '*':
            *result = *result * temp;
            break;

        case '/':
            *result = *result / temp;
            break;

        case '&':
            *result = *result & temp;
            break;

        default:
            *result = *result | temp;
            break;
   }
}

/*******************************************************************************
 * Name   : ctc_pktxr_math_expression_calculator
 * Purpose: get the math sign and calculate the result.
 * Input  : exp
 * Output : result
 * Return : .
 * Note   : N/A
*******************************************************************************/
void
ctc_pktxr_math_expression_calculator(char *exp, ctp_field_seg_hdr_t *field_seg,
                           uint16 *cw, uint32 *result)
{
    char *pch0 = NULL,*pch1 = NULL;
    char str[50], curr_sign;
    char last_sign ='\0';

    *result = 0;
    /* get start pointer */
    pch0 = strstr(exp, "["); // the start place sign

    do
    {
        get_current_maths_sign(pch0, &pch1, &curr_sign); // get the first exp
        memset(str, 0, 50);
        memcpy(str, pch0, (int)(pch1 - pch0 + 1));

        calculate_result(result, str, last_sign,field_seg, cw);

        pch0 = pch1 + 1;
        last_sign = curr_sign;

    }while(curr_sign != ']');

}

/*******************************************************************************
 * Name   : logical_expression_u32
 * Purpose: logical_expression_u32.
 * Input  : .
 * Output : None.
 * Return : .
 * Note   : N/A
*******************************************************************************/
static int
logical_expression_u32(char *exp, ctp_field_seg_hdr_t *fiedl_seg,uint16 *cw)
{
    uint32 lresult, rresult;
    char *pch0,*pch1;
    char lstr[50], rstr[50];

    memset(lstr, 0, 50);
    memset(rstr, 0, 50);

    pch0 = strstr(exp, "[");
    if(NULL == pch0)
    {
        return 0;
    }

    pch1 = strstr(exp, "]");
    if((NULL == pch0) || (pch1 <= pch0))
    {
        return 0;
    }
    memcpy(lstr, pch0, (int)(pch1 - pch0 + 1));

    pch0 = strstr(pch1, "[");
    if(NULL == pch0)
    {
        return 0;
    }

    pch1 = strstr(pch0, "]");
    if((NULL == pch0) || (pch1 <= pch0))
    {
        return 0;
    }
    memcpy(rstr, pch0, (int)(pch1 - pch0 + 1));

    ctc_pktxr_math_expression_calculator(lstr, fiedl_seg, cw, &lresult); // the left math result
    ctc_pktxr_math_expression_calculator(rstr, fiedl_seg, cw, &rresult);// the right math result

    /* if the logical calculation result is right, the return value none zero, otherwise is zero */
    if(NULL != strstr(exp, "!="))
    {
        return (lresult != rresult);
    }
    else if(NULL != strstr(exp, "=="))
    {
        return (lresult == rresult);
    }
    else if(NULL != strstr(exp, ">="))
    {
        return (lresult >= rresult);
    }
    else if(NULL != strstr(exp, "<="))
    {
        return (lresult <= rresult);
    }
    else if(NULL != strstr(exp, ">"))
    {
        return (lresult > rresult);
    }
    else
    {
        return (lresult < rresult);
    }
}

/*******************************************************************************
 * Name   : logical_expression_str
 * Purpose: logical_expression_str.
 * Input  : .
 * Output : None.
 * Return : .
 * Note   : N/A
*******************************************************************************/
static int
logical_expression_str(char *exp, ctp_field_seg_hdr_t *field_seg,uint16 *cw)
{
    int              result, len;
    char             *pch0, *ch = NULL;
    ctp_field_seg_t  *seg = NULL;
    ctp_field_info_t *finfo = NULL;

    result = 0;
    TAILQ_FOREACH(seg, field_seg, entry)
    {
        finfo = ctc_pktxr_get_field_info_pointer(seg->field_id);
        if(NULL != strstr(exp, finfo->descriptor))
        {

            len = ((seg->length + 7)/8) * 2 + 1;
            ch = (char *)ctc_pktxr_ctckal_malloc(len);
            if (NULL == ch)
            {
                return CTP_E_MEMORY_ERROR;
            }
            ctc_pktxr_uint2str(ch, seg->value,  len - 1);
            result = 1;
            break;
        }
    }

    if(!result)
    {
        if (NULL != ch)
        {
            ctc_pktxr_ctckal_free(ch);
        }
        return 0;
    }

    pch0   = strstr(exp, "#");
    if (NULL == pch0)
    {
        ctc_pktxr_ctckal_free(ch);
        return 0;
    }

    result = strcmp(ch, pch0 + 1);
    ctc_pktxr_ctckal_free(ch);
    if(NULL != strstr(exp, "!="))
    {
        return (result != 0);
    }
    else if(NULL != strstr(exp, "=="))
    {
        return (result == 0);
    }
    else if(NULL != strstr(exp, ">="))
    {
        return (result >= 0);
    }
    else if(NULL != strstr(exp, "<="))
    {
        return (result <= 0);
    }
    else if(NULL != strstr(exp, ">"))
    {
        return (result > 0);
    }

    return (result < 0);
}

/*******************************************************************************
 * Name   : ctc_pktxr_logical_expression_calculator
 * Purpose: logical expression calculate.
 * Input  : .
 * Output : None.
 * Return : .
 * Note   : N/A
*******************************************************************************/
int
ctc_pktxr_logical_expression_calculator(char *exp, ctp_field_seg_hdr_t *field_seg,uint16 *cw)
{

    if(NULL != strstr(exp, "null"))
    {
        return 1;
    }
    else if(NULL != strstr(exp, "%ss"))
    {
        return logical_expression_str(exp,field_seg, cw);
    }
    else
    {
        return logical_expression_u32(exp,field_seg, cw);
    }
}

/*******************************************************************************
 * Name   : ctc_pktxr_length_lookup
 * Purpose: look up the field length.length handle  handle length field
 * Input  : .
 * Output : None.
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
uint8
ctc_pktxr_length_lookup(uint16 *cw, uint16 f, uint16 *l)
{
    if(cw[FLEN_FID] == f)
    {
        cw[FLEN_FID] = FIELD_NUM;
        *l           = cw[FIEL_LEN];
        return 1;
    }

    return 0;
}

 /*******************************************************************************
 * Name   : cmmd_lookup
 * Purpose: look up the command string.
 * Input  : .
 * Output : None.
 * Return : if found return 1 else return 0.
 * Note   : N/A
*******************************************************************************/
uint8
ctc_pktxr_cmd_lookup(ctp_command_hdr_t *l, ctp_command_hdr_t *r, char *key, ctp_cmd_t **f)
{
    ctp_cmd_t *lcurr = NULL;



    TAILQ_FOREACH(lcurr, l, entry)
    {
        //printf("======================key: %s (%d)  lcurr: %s (%d) %s\n",key,strlen(key),lcurr->keyword,strlen(lcurr->keyword),lcurr->value);

        if(NULL != strstr(lcurr->keyword, key) && ((strlen(lcurr->keyword)-strlen(key))==2))
        {
#if 0
        printf("======================key: %s (%d)  lcurr: %s (%d) %s\n",key,strlen(key),lcurr->keyword,strlen(lcurr->keyword),lcurr->value);
#endif
            *f = lcurr;
            TAILQ_REMOVE(l, lcurr, entry);

            /* add it to right command list */
            TAILQ_INSERT_TAIL(r, lcurr, entry);
            return 1;
        }
    }

    return 0;
}

#if 0
/*******************************************************************************
 * Name   :  printf_cmd_queue
 * Purpose:  printf cmd queue
 * Input  : .
 * Output : None.
 * Return : None
 * Note   : N/A
*******************************************************************************/
static void
printf_cmd_queue(ctp_command_hdr_t *l)
{
    ctp_cmd_t *lcurr = NULL;
    int iii=0;
    TAILQ_FOREACH(lcurr, l, entry)
    {

      printf("%d  keyword=%s",++iii,lcurr->keyword);
      printf("keyword=%s\n", lcurr->value);

    }

}
#endif


/*******************************************************************************
 * Name   : ctc_pktxr_control_word_handle
 * Purpose: Handle the control word command string.
 * Input  : .
 * Output : cw, the array for storing the control word value
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
ctp_error_e
ctc_pktxr_control_word_handle(ctp_command_hdr_t *l, ctp_command_hdr_t *r, uint16 *cw)
{
    ctp_cmd_t *lcurr = NULL;
    int temp = 0;

    TAILQ_FOREACH(lcurr, l, entry)
    {
        int cw_idx;
        for (cw_idx = 0; cw_idx < CONTROL_WORD_NUM; cw_idx ++)
        {
            if(NULL != strstr(lcurr->keyword, control_word[cw_idx]))
            {
                sscanf (lcurr->value, "%x", &temp) ; // whether need to handle
                cw[cw_idx] = (uint16)temp;
                break;
            }
        }
    }

    return CTP_E_NONE;
}

/*******************************************************************************
 * Name   : ctc_pktxr_get_err_code_string
 * Purpose: Get the pointer of stored error string.
 * Input  : error code.
 * Output : None.
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
char *
ctc_pktxr_get_err_code_string(uint16 error)
{
    return err_string[error];
}


/*******************************************************************************
 * Name   : ctc_pktxr_convert_to_upper
 * Purpose: conver to the src string to upper and store it to dest string.
 * Input  : dest - the destination string pointer.
 *        : src  - the source string pointer.
 * Output : None.
 * Return : N/A.
 * Note   : N/A
*******************************************************************************/
void
ctc_pktxr_convert_to_upper(char *dest, const char *src)
{
    while (*src != '\0')
    {
        if(is_a_letter(*src))
        {
            *dest = toupper(*src);
        }
        else
        {
            *dest = *src;
        }
        dest ++;
        src ++;
    }
}

/****************************************************************************
 * Name:       get_bit_value
 * Purpose:    get bit range of src.
 * Parameters:
 * Input:      src  -- source buffer.
 *             start_bit -- start bit number.
 *             bit_len -- get bit len.
 *             field_len -- the whole field len.
 * Output:     N/A.
 * Return:     retvalue -- the value of bit in src.
 *
 * Note:       N/A.
 ****************************************************************************/
static uint8
get_bit_value(uint8 *src, uint32 start_bit, uint32 bit_len, uint32 field_len)
{
    uint8 retvalue = 0;
    uint32 total_len = (field_len+7)/8*8;
    uint32 real_start_bit = start_bit + (total_len - field_len);
    uint32 start_byte = real_start_bit/8;
    uint32 byte_start_bit = real_start_bit%8;
    uint32 mask = 0,mask1 =0;
    uint32 rest_bit_len = 0;

    if((byte_start_bit + bit_len) <= 8)
    {
        mask = ((1<<bit_len) -1) << (8-byte_start_bit-bit_len);
        retvalue = (src[start_byte]&mask)>>(8-byte_start_bit-bit_len);
    }
    else
    {
        //rest_bit_len = (byte_start_bit + bit_len) - 8;
        rest_bit_len = (8 - byte_start_bit);
        mask = ((1<<rest_bit_len) -1);
        mask1 = ((1<<(8-rest_bit_len)) -1) << rest_bit_len;

        retvalue = (((src[start_byte]&mask)<<(8-rest_bit_len))
                    |((src[start_byte+1]&mask1)>>rest_bit_len)) >> (8-bit_len);


    }

    return retvalue;



}
/****************************************************************************
 * Name:       ctc_pktxr_set_field_bit
 * Purpose:    copy src to dest with a bits range.
 * Parameters:
 * Input:      src  -- source buffer.
 *             start bit -- start bit number.
 *             end bit -- end bit number.
 * Output:     dest -- dest buffer.
 * Return:     DRV_E_NONE = success.
 *           Other = Error, please reference DRV_E_XXX.
 * Note:       because we only use it for little endian,
 *             no need care big endian.
 ****************************************************************************/
void
ctc_pktxr_set_field_bit(uint8 *dest, uint32 start_bit, uint32 end_bit, uint8 *src)
{
    uint32 cur_byte, cur_dest_bit, cur_src_sbit = 0;
    uint32 field_bit_len = end_bit - start_bit + 1;
    //uint8 src_value = 0;

    cur_dest_bit = start_bit;

    for(cur_byte = start_bit / 8; cur_byte <= end_bit / 8; cur_byte ++)
    {
        uint32 cur_dbit, /*cur_sbit,*/ bit_num/*, cur_src_byte*/;
        uint8  mask, src_byte;

        /* calculate bit numbers and dest start bit in current byte */
        // get the bit number 8 or the field bit len(the field_len<8)
        bit_num = (end_bit >= ((cur_byte + 1) * 8)) ?\
                            8 : (end_bit - cur_byte * 8 + 1);
        cur_dbit = cur_dest_bit % 8; // current place of the field
        // must decrease the bit_num for the (end_bit - cur_byte * 8 + 1) included the cur_dbit when the field_len<8
        bit_num = bit_num - cur_dbit; // the releative num_bit to the start
        //mask   = ((1 << bit_num) - 1) << cur_dbit;/* zhao feng 2007-5-24
        mask   = ((1 << bit_num) - 1) << (8 - bit_num - cur_dbit);

        /* clear current dest buff byte will be fill , set to zero*/
        dest[cur_byte] = dest[cur_byte] & (~mask);

        /* retrieve bit number bits from source */
        /*cur_sbit = cur_src_sbit % 8;
        cur_src_byte = cur_src_sbit / 8;*/


        /* modify get value method, start, by shanz 2013-02-28 */
        src_byte = get_bit_value(src, cur_src_sbit,bit_num,field_bit_len);
        cur_src_sbit = cur_src_sbit + bit_num;
        /*end, by shanz 2013-02-28*/

#if 0

        /* add this method to handle the condition when the field is cross the 8-bit boundary */
        if (((end_bit - start_bit + 1) != bit_num)&& (cur_dbit != 0))
        {
            src_byte = src[cur_src_byte] & mask;
            if ( (end_bit - start_bit) <= 8)
            {
                uint8 bit_offset = end_bit - start_bit - bit_num + 1;
                src_byte = (src[cur_src_byte] & (mask << bit_offset)) >> bit_offset;
            }
            cur_src_sbit = cur_src_sbit + bit_num;
        }
        else if((8 - cur_sbit) >= bit_num)
        {
            //src_byte = ((src[cur_src_byte] >> cur_sbit) << cur_dbit) & mask;/* zhao feng 2007-5-24
            src_byte = ((src[cur_src_byte]) << (8 - bit_num - cur_dbit)) & mask;
            cur_src_sbit = cur_src_sbit + bit_num;
        }
        else
        {
            uint8 filled_bitnum = (8 - bit_num - cur_dbit);

            cur_src_sbit = cur_src_sbit + bit_num;
            src_byte = src[cur_src_byte + 1] << filled_bitnum;
            src_byte = (src_byte ) & mask;
        }
#endif
        /* modify get value method, start, by shanz 2013-02-28 */
        //dest[cur_byte] = dest[cur_byte] | src_byte;
        dest[cur_byte] = dest[cur_byte] | (src_byte << (8 - bit_num - cur_dbit));
        /*end, by shanz 2013-02-28*/

        cur_dest_bit = cur_dest_bit + bit_num;
    }

}

/****************************************************************************
 * Name:       string2char
 * Purpose:    conver a char buffer to string.
 * Parameters:
 * Input:      str   -- pointer to string.
 * Output:     buff  -- pointer to char buffer
 * Note:       none.
 *****************************************************************************/
void
ctc_pktxr_uint2str(char *str, uint8 *buff, int len)
{
    int post;

    for(post = 0; post < (len/2); post ++)
    {
        char nibble, ch;

        nibble = (buff[post] >> 4) & 0x0F;
        ch    =  (nibble > 9) ? 'A' : '0';
        nibble = (nibble > 9) ? (nibble - 10):nibble ;
        str[ 2 * post ] = nibble + ch;

        nibble = buff[post] & 0x0F;
        ch    =  (nibble > 9) ? 'A' : '0';
        nibble = (nibble > 9) ? (nibble - 10):nibble;
        str[ 2 * post + 1] = nibble + ch;
    }

    str[2 * post] = '\0';
}

/****************************************************************************
 * Name:       ctc_pktxr_string2char
 * Purpose:    conver string to char buffer.
 * Parameters:
 * Input:      str   -- pointer to string.
 * Output:     buff  -- pointer to char buffer
 * Note:       none.
 *****************************************************************************/
void
ctc_pktxr_string2char(char *str, uint8 *buff)
{
    char     *sp = str;
    char     temp[3];
    int      /*flag = 1,*/ dest_cnt = 0;
    int      dest= 'g';/*fix need debug bug, taoqp 2012-06-08*/

    while(('\0' != *sp) && ('@' != *sp))
    {
        memset(temp, 0, 3);
LOOP1:
        if (((*sp >= '0') && (*sp <= '9'))
            || (((toupper(*sp)>='A')) && ((toupper(*sp)<='F'))))
        {
            temp[0] = *sp;
            sp ++;

            if ((strlen(str) % 2) != 0)
            {
                /*flag = 1;*/
                goto INPUT;
            }
        }
        else if (('\0' == *sp) || ('@' == *sp))
        {
            goto INPUT;
        }
        else
        {
            sp++;
            goto LOOP1;
        }

LOOP2:
        if (((*sp >= '0') && (*sp <= '9'))
            || (((toupper(*sp)>='A')) && ((toupper(*sp)<='F'))))
        {
            temp[1] = *sp;
            sp ++;
        }
        else if (('\0' == *sp) || ('@' == *sp))
        {
            goto INPUT;
        }
        else
        {
            sp++;
            goto LOOP2;
        }

INPUT:
        sscanf(temp, "%02x", &dest);
        buff[dest_cnt] = (uint8)dest; //store the value to an array
        dest_cnt ++;
    }
    return;
}

/****************************************************************************
 * Name:       ctc_pktxr_get_field_bit
 * Purpose:    copy src to dest with a bits range.
 * Parameters:
 * Input:      src  -- source buffer.
 *             start bit -- start bit number.
 *             end bit -- end bit number.
 * Output:     dest -- dest buffer.
 * Return:     DRV_E_NONE = success.
 *           Other = Error, please reference DRV_E_XXX.
 * Note:       because we only use it for little endian,
 *             no need care big endian.
 ****************************************************************************/
void
ctc_pktxr_get_field_bit(uint8 *dest, uint32 start_bit, uint32 end_bit, uint8 *src)
{
    uint32 curr_src_byte, cur_sbit;
    uint32 cur_dest_byte = 0;
    uint32 cur_src_sbit = start_bit;
    uint8  src_byte;

    for(curr_src_byte = start_bit / 8; curr_src_byte <= end_bit / 8; curr_src_byte ++)
    {
        uint8  mask0 = 0, /*mask1 = 0, */num_bit = 0, bit_num0 = 0/*, bit_num1 = 0, last_bit_num0 = 0*/ ;

        /* calculate bit numbers and dest start bit in current byte */
        num_bit = (end_bit >= ((curr_src_byte + 1) * 8)) ?\
                            8 : (end_bit - curr_src_byte * 8 + 1);
        cur_sbit = cur_src_sbit % 8;
        bit_num0 = num_bit - cur_sbit; // the releative num_bit to the start
        //bit_num1 = num_bit - bit_num0; // only used when the field is not start from 8

        /* calculate bit numbers and dest start bit in current byte */
        mask0 = ((1<<bit_num0) - 1);
        //mask0   = ((1 << bit_num0) - 1) << bit_num1;
        /*mask1   = (1 << bit_num1) - 1;*/

        /* first bit segment handle */
        src_byte = src[curr_src_byte];
#if 0
        if ( (bit_num0 + last_bit_num0) < 8){
            dest[cur_dest_byte] = ((src_byte >>(8 - cur_sbit - bit_num0)) & mask0) + (dest[cur_dest_byte]<<last_bit_num0);
        } else {
             /* clear current dest buff byte will be fill */
            dest[cur_dest_byte] = 0;
             //dest[cur_dest_byte] |= (src_byte << bit_num1) & mask0;
            dest[cur_dest_byte] |= (src_byte >>(8 - cur_sbit - bit_num0)) & mask0;
        }
#endif
        dest[cur_dest_byte] = 0;
        dest[cur_dest_byte] |= (src_byte >>(8 - cur_sbit - bit_num0)) & mask0;

        /* first bit segment handle */
        /*
        if(0 != bit_num1)
        {
            src_byte             = src[curr_src_byte + 1] >> (8 - bit_num1);
            dest[cur_dest_byte] |= src_byte  & mask1;
        }
        */
        //cur_src_sbit = cur_src_sbit + num_bit - bit_num0;taoqp 20110922
#if 0
        printf("curr_src_byte:%d  cur_dest_byte:%d    dest[cur_dest_byte]:%02x  i:%d  (end_bit - start_bit):%d\n",\
            curr_src_byte,cur_dest_byte,dest[cur_dest_byte],i,(end_bit - start_bit));
#endif
        cur_src_sbit = cur_src_sbit + bit_num0;
        /*last_bit_num0 = bit_num0;*/
        cur_dest_byte ++;
    }
}

/****************************************************************************
 * Name:       ctc_pktxr_calculate_chksum
 * Purpose:    calculate uint16 checksum of a memory block.
 * Parameters:
 * Input:       p : base pointer of the memory block
 *             length : length of the memory block
 *             init is the initial seed value for calculation
 * Output:     none.
 * Return:     0 = success. Otherwise failure.
 * Note:       none.
 ****************************************************************************/
uint16
ctc_pktxr_calculate_chksum (uint16 *start, uint16 length, uint32 init)
{
    uint32 sum = init;
    uint16 len = length >> 1;

    while (len-- > 0)
        sum += ntohs(*start++);

    if (length & 1)
        sum += (ntohs(*start) & 0xFF00);

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    len = sum & 0xFFFF;
    return(~len);
}

/*****************************************************************************
 * calculate 32 bit crc
 *****************************************************************************/
#ifdef CTP_BIG_ENDIAN
static inline int32
crc32(uint32 seed, unsigned char *data, uint32 data_len, uint32 *p_crc)
{
    uint32 crc;
    int32 i;

    crc = seed;

    while (data_len)
    {
        crc ^= (*data << 24);
        for (i = 0; i < 8; i++)
            crc = (crc << 1) ^ ((crc & 0x80000000) ? CRCPOLY_BIG_ENDIAN: 0);
        data++;
        data_len--;
    }

    *p_crc = ~crc;

     return 0;
}
#else
static inline int32
crc32(uint32 seed, unsigned char *data, uint32 data_len, uint32 *p_crc)
{
    uint32 crc;
    int32 i;

    crc = seed;

    while (data_len)
    {
        crc ^= *data;
        for (i = 0; i < 8; i++)
            crc = (crc >> 1) ^ ((crc & 1) ? CRCPOLY_LITTLE_ENDIAN: 0);
        data++;
        data_len--;
    }

    *p_crc = ~crc;

    return 0;
}
#endif

/*****************************************************************************
 * Name         : ctc_pktxr_ctp_crc32
 * Purpose      : calculate crc32
 * Input        : seed: initial crc seed
 *                data: data buffer to be calculated
 *                data_len: data length
 * Output       : crc: calculated crc
 * Return       :
 * Note         :
 *****************************************************************************/
uint32
ctc_pktxr_ctp_crc32(uint32 seed, unsigned char *data, uint32 data_len)
{
    uint32 crc;

    crc32(seed, data, data_len, &crc);

    return crc;
}

void *ctc_pktxr_ctckal_malloc(uint32 size)
{
    void *p = XMALLOC(MTYPE_NEW_CTP,size) ;
    return p;
}

char *ctc_pktxr_ctckal_strdup(char *ptr)
{
    if (NULL == ptr)
    {
        return NULL;
    }

    char *p =  XSTRDUP(MEM_CLI_MODULE,ptr); //copy the str and return the str point
    return p;
}

void *ctc_pktxr_ctckal_calloc(size_t nelem, size_t elsize)
{
    return XCALLOC(MEM_CLI_MODULE,nelem*elsize);
}

void ctc_pktxr_ctckal_free(void *addr)
{
    if (addr == NULL)
        return;

    XFREE(MTYPE_NEW_CTP,addr);
    return;
}

char *
mem_strdup (const char *str1, int mtype, char *file, int line)
{
    char *str2;
    int  len;

    len = strlen (str1);

    str2 = mem_malloc(mtype,len+1);

    if (str2 == NULL)
      return NULL;

    strcpy (str2, str1);

    return str2;
}

void *
mem_calloc (int size,  int mtype,  char *filename,  int line)
{
    unsigned char *bptr;

    bptr = mem_malloc (mtype, size);

    if (bptr == NULL)
      return NULL;

    memset (bptr, 0, size);

    return (void *) bptr;
}


/******************************************************************************
 * Name   : ctc_pktxr_memcpy_chksum
 * Purpose: copy data to memory location according to edian
 * Input  : p_pkt: address of memory pointer
 *          data:  points to data to be copied
 *          size:  data size
 * Output : p_pkt: memory pointer gets updated based on size
 * Return : SUCCESS
 *          Other = ErrCode
 * Note   : N/A
 *****************************************************************************/
int32
ctc_pktxr_memcpy_chksum(uint8 **p_pkt, void *data, int32 size)
{
    uint8 *pkt = *p_pkt;

    if (size == 2)
    {
        *(uint16*)data = htons(*(uint16*)data);
        memcpy(pkt, data, 2);
        pkt += 2;
    }
    else if (size == 3)
    {
        *(uint32*)data = htonl(*(uint32*)data) >> 8;
        memcpy(pkt, data, 3);
        pkt += 3;
    }
    else if (size == 4)
    {
        *(uint32*)data = htonl(*(uint32*)data);
        memcpy(pkt, data, 4);
        pkt += 4;
    }
    *p_pkt = pkt;

    return 0;
}

#if 0
/******************************************************************************
 * Name   : ctp_tool_set
 * Purpose: determine the tools user used(uml or ixia)
 * Input  : str
 * Output :
 * Return : ret
 * Note   : N/A
 *****************************************************************************/

int ctp_tool_set(char *str)
{
    int ret = 0;
    if (0 == strcmp(str, "Ixia"))
    {
        ret = 1;
    }

    return ret;
}
#endif

/*CRC-4, poly= x^4 + x^1 + x^0 ,init=0*/
/****************************************************************************
 * Name:       ctc_pktxr_calculate_crc4
 * Purpose:    CRC4 bits arithmetic, use for GB bridge header CRC,
               polynomial: (0 1 4);
               data width: 256bits;
               convention: the first serial bit is D[255].
 * Parameters: 256 bits data; len: unit byte; init_crc = 0
 * Input:      256 bits data
 * Output:     4 bits CRC value
 * Return:     4 bits CRC value
 * Note:       None.
 ****************************************************************************/
uint8
ctc_pktxr_calculate_crc4(uint8 *data, int32 len, uint8 init_crc)
{

    uint8 d[256] = {0}, c[4] = {0};
    uint8 NewCRC[4] = {0};
    uint32 i ;
    int16 result = init_crc;

    for (i = 0; i < (len * 8); i++)
    {
        d[255-i] = ((data[i/8])& (1 << (7-i%8))) ? 1: 0;
    }

    NewCRC[0] = d[255] ^ d[251] ^ d[250] ^ d[249] ^ d[248] ^ d[246] ^ d[244] ^
                d[243] ^ d[240] ^ d[236] ^ d[235] ^ d[234] ^ d[233] ^ d[231] ^
                d[229] ^ d[228] ^ d[225] ^ d[221] ^ d[220] ^ d[219] ^ d[218] ^
                d[216] ^ d[214] ^ d[213] ^ d[210] ^ d[206] ^ d[205] ^ d[204] ^
                d[203] ^ d[201] ^ d[199] ^ d[198] ^ d[195] ^ d[191] ^ d[190] ^
                d[189] ^ d[188] ^ d[186] ^ d[184] ^ d[183] ^ d[180] ^ d[176] ^
                d[175] ^ d[174] ^ d[173] ^ d[171] ^ d[169] ^ d[168] ^ d[165] ^
                d[161] ^ d[160] ^ d[159] ^ d[158] ^ d[156] ^ d[154] ^ d[153] ^
                d[150] ^ d[146] ^ d[145] ^ d[144] ^ d[143] ^ d[141] ^ d[139] ^
                d[138] ^ d[135] ^ d[131] ^ d[130] ^ d[129] ^ d[128] ^ d[126] ^
                d[124] ^ d[123] ^ d[120] ^ d[116] ^ d[115] ^ d[114] ^ d[113] ^
                d[111] ^ d[109] ^ d[108] ^ d[105] ^ d[101] ^ d[100] ^ d[99] ^
                d[98] ^ d[96] ^ d[94] ^ d[93] ^ d[90] ^ d[86] ^ d[85] ^ d[84] ^
                d[83] ^ d[81] ^ d[79] ^ d[78] ^ d[75] ^ d[71] ^ d[70] ^ d[69] ^
                d[68] ^ d[66] ^ d[64] ^ d[63] ^ d[60] ^ d[56] ^ d[55] ^ d[54] ^
                d[53] ^ d[51] ^ d[49] ^ d[48] ^ d[45] ^ d[41] ^ d[40] ^ d[39] ^
                d[38] ^ d[36] ^ d[34] ^ d[33] ^ d[30] ^ d[26] ^ d[25] ^ d[24] ^
                d[23] ^ d[21] ^ d[19] ^ d[18] ^ d[15] ^ d[11] ^ d[10] ^ d[9] ^
                d[8] ^ d[6] ^ d[4] ^ d[3] ^ d[0] ^ c[3];

    NewCRC[1] = d[255] ^ d[252] ^ d[248] ^ d[247] ^ d[246] ^ d[245] ^ d[243] ^
                d[241] ^ d[240] ^ d[237] ^ d[233] ^ d[232] ^ d[231] ^ d[230] ^
                d[228] ^ d[226] ^ d[225] ^ d[222] ^ d[218] ^ d[217] ^ d[216] ^
                d[215] ^ d[213] ^ d[211] ^ d[210] ^ d[207] ^ d[203] ^ d[202] ^
                d[201] ^ d[200] ^ d[198] ^ d[196] ^ d[195] ^ d[192] ^ d[188] ^
                d[187] ^ d[186] ^ d[185] ^ d[183] ^ d[181] ^ d[180] ^ d[177] ^
                d[173] ^ d[172] ^ d[171] ^ d[170] ^ d[168] ^ d[166] ^ d[165] ^
                d[162] ^ d[158] ^ d[157] ^ d[156] ^ d[155] ^ d[153] ^ d[151] ^
                d[150] ^ d[147] ^ d[143] ^ d[142] ^ d[141] ^ d[140] ^ d[138] ^
                d[136] ^ d[135] ^ d[132] ^ d[128] ^ d[127] ^ d[126] ^ d[125] ^
                d[123] ^ d[121] ^ d[120] ^ d[117] ^ d[113] ^ d[112] ^ d[111] ^
                d[110] ^ d[108] ^ d[106] ^ d[105] ^ d[102] ^ d[98] ^ d[97] ^
                d[96] ^ d[95] ^ d[93] ^ d[91] ^ d[90] ^ d[87] ^ d[83] ^ d[82] ^
                d[81] ^ d[80] ^ d[78] ^ d[76] ^ d[75] ^ d[72] ^ d[68] ^ d[67] ^
                d[66] ^ d[65] ^ d[63] ^ d[61] ^ d[60] ^ d[57] ^ d[53] ^ d[52] ^
                d[51] ^ d[50] ^ d[48] ^ d[46] ^ d[45] ^ d[42] ^ d[38] ^ d[37] ^
                d[36] ^ d[35] ^ d[33] ^ d[31] ^ d[30] ^ d[27] ^ d[23] ^ d[22] ^
                d[21] ^ d[20] ^ d[18] ^ d[16] ^ d[15] ^ d[12] ^ d[8] ^ d[7] ^
                d[6] ^ d[5] ^ d[3] ^ d[1] ^ d[0] ^ c[0] ^ c[3];

    NewCRC[2] = d[253] ^ d[249] ^ d[248] ^ d[247] ^ d[246] ^ d[244] ^ d[242] ^
                d[241] ^ d[238] ^ d[234] ^ d[233] ^ d[232] ^ d[231] ^ d[229] ^
                d[227] ^ d[226] ^ d[223] ^ d[219] ^ d[218] ^ d[217] ^ d[216] ^
                d[214] ^ d[212] ^ d[211] ^ d[208] ^ d[204] ^ d[203] ^ d[202] ^
                d[201] ^ d[199] ^ d[197] ^ d[196] ^ d[193] ^ d[189] ^ d[188] ^
                d[187] ^ d[186] ^ d[184] ^ d[182] ^ d[181] ^ d[178] ^ d[174] ^
                d[173] ^ d[172] ^ d[171] ^ d[169] ^ d[167] ^ d[166] ^ d[163] ^
                d[159] ^ d[158] ^ d[157] ^ d[156] ^ d[154] ^ d[152] ^ d[151] ^
                d[148] ^ d[144] ^ d[143] ^ d[142] ^ d[141] ^ d[139] ^ d[137] ^
                d[136] ^ d[133] ^ d[129] ^ d[128] ^ d[127] ^ d[126] ^ d[124] ^
                d[122] ^ d[121] ^ d[118] ^ d[114] ^ d[113] ^ d[112] ^ d[111] ^
                d[109] ^ d[107] ^ d[106] ^ d[103] ^ d[99] ^ d[98] ^ d[97] ^ d[96] ^
                d[94] ^ d[92] ^ d[91] ^ d[88] ^ d[84] ^ d[83] ^ d[82] ^ d[81] ^
                d[79] ^ d[77] ^ d[76] ^ d[73] ^ d[69] ^ d[68] ^ d[67] ^ d[66] ^
                d[64] ^ d[62] ^ d[61] ^ d[58] ^ d[54] ^ d[53] ^ d[52] ^ d[51] ^
                d[49] ^ d[47] ^ d[46] ^ d[43] ^ d[39] ^ d[38] ^ d[37] ^ d[36] ^
                d[34] ^ d[32] ^ d[31] ^ d[28] ^ d[24] ^ d[23] ^ d[22] ^ d[21] ^
                d[19] ^ d[17] ^ d[16] ^ d[13] ^ d[9] ^ d[8] ^ d[7] ^ d[6] ^ d[4] ^
                d[2] ^ d[1] ^ c[1];

    NewCRC[3] = d[254] ^ d[250] ^ d[249] ^ d[248] ^ d[247] ^ d[245] ^ d[243] ^
                d[242] ^ d[239] ^ d[235] ^ d[234] ^ d[233] ^ d[232] ^ d[230] ^
                d[228] ^ d[227] ^ d[224] ^ d[220] ^ d[219] ^ d[218] ^ d[217] ^
                d[215] ^ d[213] ^ d[212] ^ d[209] ^ d[205] ^ d[204] ^ d[203] ^
                d[202] ^ d[200] ^ d[198] ^ d[197] ^ d[194] ^ d[190] ^ d[189] ^
                d[188] ^ d[187] ^ d[185] ^ d[183] ^ d[182] ^ d[179] ^ d[175] ^
                d[174] ^ d[173] ^ d[172] ^ d[170] ^ d[168] ^ d[167] ^ d[164] ^
                d[160] ^ d[159] ^ d[158] ^ d[157] ^ d[155] ^ d[153] ^ d[152] ^
                d[149] ^ d[145] ^ d[144] ^ d[143] ^ d[142] ^ d[140] ^ d[138] ^
                d[137] ^ d[134] ^ d[130] ^ d[129] ^ d[128] ^ d[127] ^ d[125] ^
                d[123] ^ d[122] ^ d[119] ^ d[115] ^ d[114] ^ d[113] ^ d[112] ^
                d[110] ^ d[108] ^ d[107] ^ d[104] ^ d[100] ^ d[99] ^ d[98] ^ d[97] ^
                d[95] ^ d[93] ^ d[92] ^ d[89] ^ d[85] ^ d[84] ^ d[83] ^ d[82] ^
                d[80] ^ d[78] ^ d[77] ^ d[74] ^ d[70] ^ d[69] ^ d[68] ^ d[67] ^
                d[65] ^ d[63] ^ d[62] ^ d[59] ^ d[55] ^ d[54] ^ d[53] ^ d[52] ^
                d[50] ^ d[48] ^ d[47] ^ d[44] ^ d[40] ^ d[39] ^ d[38] ^ d[37] ^
                d[35] ^ d[33] ^ d[32] ^ d[29] ^ d[25] ^ d[24] ^ d[23] ^ d[22] ^
                d[20] ^ d[18] ^ d[17] ^ d[14] ^ d[10] ^ d[9] ^ d[8] ^ d[7] ^
                d[5] ^ d[3] ^ d[2] ^ c[2];

    result = (((NewCRC[3]&0x1) << 3)
              | ((NewCRC[2]&0x1) << 2)
              | ((NewCRC[1]&0x1) << 1)
              | (NewCRC[0]&0x1));

    return result;
}





