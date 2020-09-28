/****************************************************************************
 * ctp_decoder.c  provides all CTP Packet Decoder Fucntion and Macros define.
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
 * This Array used for store control word of encorder
 */
static uint16    dec_cw[CONTROL_WORD_NUM];

/*
 * This Variablies used for store handled and to be handled command string
 */
static ctp_command_hdr_t dec_left_cmdl;
static ctp_command_hdr_t dec_right_cmdl;
static int g_dec_initialized = 0;

/*
 * This Variables used for store packet infomation.
 */
ctp_pkt_info_t   dec_pkt;
static uint8     *payload        = NULL;
static uint8     rep_exist       = 0;

static void decode_free_header();

#define M_DEC_INITIALIZED   \
do { \
    if (!g_dec_initialized) \
    { \
        g_dec_initialized = 1; \
        TAILQ_INIT(&dec_left_cmdl); \
        TAILQ_INIT(&dec_right_cmdl); \
        TAILQ_INIT(&dec_pkt.header); \
    } \
} while (0)

/****************************************************************************
 *
 * Functions
 *
 ****************************************************************************/
/*******************************************************************************
 * Name   : decoder_exist_handle
 * Purpose: decoder field exist handler.
 * Input  : ...;
 * Output : None.
 * Return : if exist, return true, else return false.
 * Note   : .
*******************************************************************************/

static uint8
decoder_exist_handle(ctp_field_info_t  *field_info,
                     ctp_field_seg_hdr_t *field_seg, uint16 *jump_id)
{
    int               idx = 0;
    ctp_flags_exist_t *exist;

    for(;;)
    {
        int               logic = 0; 
		int               logic_store = 0;
        char              *exp = NULL;
        uint16            flag_store = 0;

        exist = &field_info->exist[idx];
        exp   = exist->exp;
        logic = ctc_pktxr_logical_expression_calculator(exp, field_seg, dec_cw);
#if 0
printf("++++++++++++++++++++++++++++++++exist_exp:%s\t  logic:%d\t   idx:%d\n",exp,logic,idx);
#endif
        if(0 == idx)
        {
            logic_store = logic;
        }
        else
        {
            if(flag_store & OP_FLAG_AND_NEXT)
            {
                logic_store = logic_store && logic;
            }
            else
            {
                logic_store = logic_store || logic;
            }
        }

        if(exist->op_flags & OP_FLAG_STAND_ONLY)
        {
            if(logic_store)
            {
                return 1;
            }
        }

        /* to next expression */
        if(!exist->next)
        {
            break;
        }

        idx ++;
        flag_store = exist->op_flags;
    }

    *jump_id = exist->field_id;
    return 0;
}
/*******************************************************************************
 * Name   : decoder_next_protocol_handle
 * Purpose: hand the set next protocol flags.
 * Input  : None.
 * Output : None.
 * Return : .
 * Note   : N/A
*******************************************************************************/
static void
decoder_next_protocol_handle(ctp_field_info_t  *field_info,
                            ctp_field_seg_hdr_t *field_seg)
{
    int               idx = 0;

    for(;;)
    {
        int               logic = 0, logic_store = 0;
        ctp_flags_nextp_t *nextp = NULL;
        char              *exp = NULL;
        uint16            flag_store = 0;

        nextp = &field_info->nextp[idx];
        exp   = nextp->exp;
        logic = ctc_pktxr_logical_expression_calculator(exp, field_seg, dec_cw);
#if 0
printf("++++++++++++++++++++++++++++++next_exp:%s   logic:%d\n",exp,logic);
#endif

        if(0 == idx)
        {
            logic_store = logic;
        }
        else
        {
            if(flag_store & OP_FLAG_AND_NEXT)
            {
                logic_store = logic_store && logic;
            }
            else
            {
                logic_store = logic_store || logic;
            }
        }

        if(nextp->op_flags & OP_FLAG_STAND_ONLY)
        {
            if(logic_store)
            {
                dec_cw[NEXT_TYPE] = nextp->protocol;
                break;
            }
            else
            {
                if(!nextp->next)
                {
                    break;
                }

                idx ++;
                flag_store = nextp->op_flags;
            }
        }

    }

}
/*******************************************************************************
 * Name   : decoder_jump_handle
 * Purpose: hand the set jump flags.
 * Input  : None.
 * Output : None.
 * Return : .
 * Note   : N/A
*******************************************************************************/
static uint8
decoder_jump_handle(ctp_field_info_t  *field_info,
                    ctp_field_seg_hdr_t *field_seg, uint16 *jump_id)
{
    int               idx = 0;

    for(;;)
    {
        int               logic = 0; 
		int               logic_store = 0;
        ctp_flags_jump_t  *jump = NULL;
        char              *exp = NULL;
        uint16            flag_store = 0;

        jump = &field_info->jump[idx];
        exp   = jump->exp;
        logic = ctc_pktxr_logical_expression_calculator(exp, field_seg, dec_cw);

        if(0 == idx)
        {
            logic_store = logic;
        }
        else
        {
            if(flag_store & OP_FLAG_AND_NEXT)
            {
                logic_store = logic_store && logic;
            }
            else
            {
                logic_store = logic_store || logic;
            }
        }

        if(jump->op_flags & OP_FLAG_STAND_ONLY)
        {
            if(logic_store)
            {
                *jump_id = jump->field_id;

                if(1 == rep_exist)
                    {
                        if(jump->op_flags & OP_FLAG_DEC_CNT)
                        {
                                dec_cw[REPEAT] --;
                        }
                    }


                return 1;
            }
            else
            {
                if(!jump->next)
                {
                    break;
                }

            idx ++;
            flag_store = jump->op_flags;
            }
        }
    }

    return 0;
}


/*******************************************************************************
 * Name   : decoder_length_handle
 * Purpose: hand the set length flags.
 * Input  : None.
 * Output : None.
 * Return :.
 * Note   : N/A
*******************************************************************************/
static void
decoder_length_handle(ctp_field_info_t  *field_info, ctp_field_seg_hdr_t *fiedl_seg)
{
    ctp_flags_len_t    *length;
    char               *exp;
    uint32             len;

    length = field_info->length;

    if(NULL == length)
    {
        CTP_OUT("LENGTH_HANDLE_POINTER NOT EXIST, FIELD = %s\n",
                field_info->descriptor);
        return;
    }

    exp = length->exp;
    if(NULL == exp)
    {
        CTP_OUT("LENGTH_EXPRESSION_POINTER NOT EXIST, FIELD = %s\n",
                field_info->descriptor);
        return;
    }

    ctc_pktxr_math_expression_calculator(exp, fiedl_seg, dec_cw, &len);
    dec_cw[FLEN_FID] = length->field_id;
    dec_cw[FIEL_LEN] = (uint16)len;
}

 /*******************************************************************************
 * Name   : decoder_repeat_handle
 * Purpose: hand the set repeat counter flags.
 * Input  : None.
 * Output : None.
 * Return : .
 * Note   :
*******************************************************************************/
static void
decoder_repeat_handle(ctp_field_info_t  *field_info, ctp_field_seg_hdr_t *fiedl_seg)
{
    ctp_flags_repeat_t *repeat;
    char               *exp;
    uint32             rep_cnt;

    repeat = field_info->repeat;

    if(NULL == repeat)
    {
        CTP_OUT("REPEAT_HANDLE_POINTER NOT EXIST, FIELD = %s\n",
                field_info->descriptor);
        return;
    }

    exp = repeat->exp;
    if(NULL == exp)
    {
        CTP_OUT("REPEAT_EXPRESSION_POINTER NOT EXIST, FIELD = %s\n",
                field_info->descriptor);
        return;
    }

    ctc_pktxr_math_expression_calculator(exp, fiedl_seg, dec_cw, &rep_cnt);
    if(0 != field_info->field_len)
        {
            dec_cw[REPEAT] = (uint16)rep_cnt ;
            rep_exist = 1;
        }
    else
        {
            dec_cw[REPEAT] = 0;
            rep_exist = 0;
        }
#if 0
    /* Alexander: for decode, we need repeat anyway */
    dec_cw[REPEAT] = 1;


    dec_cw[REPEAT] = (uint16)rep_cnt ;  // shoule minus 1 because the fields have been set once
#endif

}

/*******************************************************************************
 * Name   : packet_decoder
 * Purpose: decode the packet and save the result to dec_pkt.
 * Input  : packet - input packet ponter.
 * Output : None.
 * Return : .
 * Note   : user need release its own memory.
*******************************************************************************/
ctp_error_e
ctc_pktxr_packet_decoder(uint8 *packet)
{
    uint16 field_id;
    ctp_header_info_t *curr_header = NULL;
    ctp_field_info_t *field_info = NULL;
    int ret;
    uint32 start = 0, end = 0;
    /*ctp_field_seg_t *curr_seg = NULL;*/

    M_DEC_INITIALIZED;
    ctc_pktxr_set_search_sequence(E_SEARCH_REVERSE);

    /* reset control word ram */
    memset(dec_cw, 0, CONTROL_WORD_NUM * 2);
    dec_cw[NEXT_TYPE] = HEADER_TYPE_NUM;
    dec_cw[FLEN_FID]  = FIELD_NUM;

    /* control word */
    CTP_ERR_RETURN(ctc_pktxr_control_word_handle(&dec_left_cmdl, &dec_right_cmdl,
                                      dec_cw), "decoder control_word_handle");

    dec_pkt.type   = dec_cw[PKT_TYPE];
    if (dec_cw[PKT_LEN] == 0)
    {
        dec_pkt.length = CTS_XR_PKT_MTU;
    }
    else
    {
        dec_pkt.length = dec_cw[PKT_LEN];
    }

    if (!TAILQ_EMPTY(&dec_pkt.header))
    {
        decode_free_header();
    }
    TAILQ_INIT(&dec_pkt.header);
    curr_header = XCALLOC(MTYPE_NEW_CTP_DECODE, sizeof(ctp_header_info_t));
    if (NULL == curr_header)
    {
        return CTP_E_MEMORY_ERROR;
    }
    TAILQ_INIT(&curr_header->seg_list);
    TAILQ_INSERT_TAIL(&dec_pkt.header, curr_header, entry);
    curr_header->next_type = HEADER_TYPE_NUM;

    /* get rthe first field  */
    field_id = ctc_pktxr_get_header_start_field(dec_cw[PKT_TYPE]);
#if 0
    printf("++++++++++++++++++++++++packet:\n");
    int i;
    for (i=0; i < 18;i++){
        printf("%d: %02x \t",i,packet[i]);
        if (i%16 == 0)
            printf("\n");
    }
#endif
    /* per field handle */
    while (NULL_FIELD != field_id)
    {
        uint8           found;
        uint16          flen,     flen_old,        jump_id;
        ctp_cmd_t       *cmd = NULL;
        uint8            *value = NULL;
        ctp_field_seg_t  *temp_seg = NULL;

        field_info =  ctc_pktxr_get_field_info_pointer(field_id);
        /* detemind field length */
        found      = ctc_pktxr_length_lookup(dec_cw, field_id, &flen);
        flen       = found? flen : field_info->field_len;
        flen_old   = field_info->field_len;

        /* Exist Command Look up */
        found      = ctc_pktxr_cmd_lookup(&dec_left_cmdl, &dec_right_cmdl,
                                     field_info->descriptor, &cmd);

#if 0
        printf("Field->descriptor:%s ",field_info->descriptor);
#endif

        /* found only used here to determine the UDN_TYPE*/
        if(0 == flen)
        {
            uint16 t;

            if((field_info->flags & CTP_FLAG_UDN_TYPE) && (found))
            {
                for (t =0; t < HEADER_TYPE_NUM; t ++)
                {
                    if(strstr((char*)cmd->value, ctc_pktxr_get_header_type_string(t)))
                    {
                        dec_cw[NEXT_TYPE] = t;
                        break;
                    }
                }
                jump_id = FIELD_NUM;
                goto NEXT_FIELD;
            }

            if (field_info->flags & CTP_FLAG_UDN_TYPE)
            {
                jump_id = FIELD_NUM;
                goto NEXT_FIELD;
            }
            if (0 == field_info->flags )
            {
                jump_id = field_id + 1;
                goto NEXT_FIELD;
            }
        }
        else
        {
            /* get the field vaule from packet */
            end  = start + flen - 1;
            if((end/8) >= dec_pkt.length)
            {
                return CTP_E_NONE;
            }
            value = (uint8 *) XMALLOC(MTYPE_NEW_CTP_DECODE,(flen + 7) / 8);
            if (NULL == value)
            {
                return CTP_E_MEMORY_ERROR;
            }

            ctc_pktxr_get_field_bit(value, start, end, packet);
            start = end + 1; /* zhao feng 2007-5-24 */

            /* add to field segment list*/
            temp_seg = (ctp_field_seg_t *) XCALLOC (MTYPE_NEW_CTP_DECODE, sizeof(ctp_field_seg_t));
            if (NULL == temp_seg)
            {
                XFREE(MTYPE_NEW_CTP_DECODE,value);
                return CTP_E_MEMORY_ERROR;
            }
            temp_seg->field_id = field_id;
            temp_seg->length   = flen;
            temp_seg->old_len  = flen_old;
            temp_seg->value    = value;
            TAILQ_INIT(&temp_seg->mod_list);
            value = NULL;
            TAILQ_INSERT_TAIL(&curr_header->seg_list, temp_seg, entry);

#if 0
        int  lengg = ((flen + 7)/ 8) * 2 + 1;
        memset(tmp, 0, lengg);
        uint2str(tmp, value, lengg - 1);
        printf("field_value: %s\n",tmp);
#endif
        }

        /* exist handle */
        if (field_info->flags & CTP_FLAG_EXIST)
        {
            //curr_header->seg_list = temp_seg;
            //temp_seg->next = NULL;

            if (!decoder_exist_handle(field_info,
                   &curr_header->seg_list, &jump_id))
            {
                /*  free memory whitch is not exist */
                TAILQ_REMOVE(&curr_header->seg_list, temp_seg, entry);
                start = start - flen; // must minus the flen if not exist
                XFREE(MTYPE_NEW_CTP_DECODE,temp_seg->value);
                XFREE(MTYPE_NEW_CTP_DECODE,temp_seg);
                temp_seg = NULL;
                goto NEXT_FIELD;
            }
            /*  zhao feng 2007-5-24
            else
            {
                start = end + 1;
            }
            */
        }


        /* set next protocol handle . move before exist to prevent ignored by non-exist */
        if (field_info->flags & CTP_FLAG_NXT_PROT)
        {
            decoder_next_protocol_handle(field_info, &curr_header->seg_list);
            curr_header->next_type = dec_cw[NEXT_TYPE];
        }


        /* set repeat counter handle */
        if(field_info->flags & CTP_FLAG_REPEAT)
        {
            decoder_repeat_handle(field_info, &curr_header->seg_list);
        }

        /* set other field length handle handle */

        if(field_info->flags & CTP_FLAG_LENGTH)
        {
            decoder_length_handle(field_info, &curr_header->seg_list);
        }

        /* set jump handle */
        if(field_info->flags & CTP_FLAG_JUMP)
        {
            /* if "exist" flag, ignore jump flag  */
            if (!(field_info->flags & CTP_FLAG_EXIST)) {
                if(decoder_jump_handle(field_info, &curr_header->seg_list, &jump_id))
                {
                    goto NEXT_FIELD;
                }
            }
        }
        jump_id = field_id + 1;

NEXT_FIELD:
        if(FIELD_NUM == jump_id)
        {
            if(HEADER_TYPE_NUM == dec_cw[NEXT_TYPE])
            {
                field_id = NULL_FIELD;
            }
            else /* add a new header */
            {
                ctp_header_info_t *temp = NULL;

                temp = (ctp_header_info_t *)XCALLOC(MTYPE_NEW_CTP_DECODE, sizeof(ctp_header_info_t));
                if (NULL == temp)
                {
                    return CTP_E_MEMORY_ERROR;
                }

                TAILQ_INIT(&temp->seg_list);
                TAILQ_INSERT_TAIL(&dec_pkt.header, temp, entry);
                curr_header = temp;
                curr_header->next_type = HEADER_TYPE_NUM;
                field_id = ctc_pktxr_get_header_start_field(dec_cw[NEXT_TYPE]);
                dec_cw[NEXT_TYPE] = HEADER_TYPE_NUM;
                /*curr_seg = NULL;*/
            }
        }
        else
        {
            field_id = jump_id;
        }
    }

    /* payload get */
    if(dec_cw[PLD_LEN] != 0)
    {
        if ((dec_cw[PLD_LEN] + dec_cw[PLD_OFFSET]) > (dec_cw[PKT_LEN] - 4))
        {
            CTP_ERR_RETURN(CTP_E_LESS_LEN, "decoder payload length check");
        }
        else
        {
            if (NULL != payload)
            {
                XFREE(MTYPE_NEW_CTP_DECODE,payload);
            }
            payload = ctc_pktxr_ctckal_malloc(dec_cw[PLD_LEN]);
            if (NULL == payload)
            {
                return CTP_E_MEMORY_ERROR;
            }
            memcpy(payload, (dec_pkt.packet + dec_cw[PLD_OFFSET]),
                dec_cw[PLD_LEN]);
        }
    }

    return CTP_E_NONE;
}

/* free decode header */
static void
decode_free_header(void)
{
    ctp_field_seg_t  *curr_seg = NULL;
    ctp_header_info_t *curr_header = NULL;

    M_DEC_INITIALIZED;

    /* free memory of header */
    while(!TAILQ_EMPTY(&dec_pkt.header))
    {
        curr_header = TAILQ_FIRST(&dec_pkt.header);
        TAILQ_REMOVE(&dec_pkt.header, curr_header, entry);

        while(!TAILQ_EMPTY(&curr_header->seg_list))
        {
            curr_seg = TAILQ_FIRST(&curr_header->seg_list);
            TAILQ_REMOVE(&curr_header->seg_list, curr_seg, entry);
            XFREE(MTYPE_NEW_CTP_DECODE,curr_seg->value);
            XFREE(MTYPE_NEW_CTP_DECODE,curr_seg);
        }

        XFREE(MTYPE_NEW_CTP_DECODE,curr_header);
    }

    TAILQ_INIT(&dec_pkt.header);
    return;
}

/*******************************************************************************
 * Name   : decoder_reset
 * Purpose: reset decorder command.
 * Input  : keyw - keyword of field set or command set.
 *        : value- value of field set or command set
 * Output : None.
 * Return : .
 * Note   : N/A
*******************************************************************************/
static ctp_error_e
decoder_reset(void)
{
    ctp_cmd_t *temp = NULL;
    ctp_cmd_t *psub = NULL;

    M_DEC_INITIALIZED;

    /* free left command list */
    while(!TAILQ_EMPTY(&dec_left_cmdl))
    {
        temp = TAILQ_FIRST(&dec_left_cmdl);
        TAILQ_REMOVE(&dec_left_cmdl, temp, entry);
        while (!TAILQ_EMPTY(&temp->list))
        {
            psub = TAILQ_FIRST(&temp->list);
            TAILQ_REMOVE(&temp->list, psub, entry);
            XFREE(MTYPE_NEW_CTP_DECODE,psub->keyword);
            XFREE(MTYPE_NEW_CTP_DECODE,psub->value);
            XFREE(MTYPE_NEW_CTP_DECODE,psub);
        }
        XFREE(MTYPE_NEW_CTP_DECODE,temp->keyword);
        XFREE(MTYPE_NEW_CTP_DECODE,temp->value);
        XFREE(MTYPE_NEW_CTP_DECODE,temp);
    }
    TAILQ_INIT(&dec_left_cmdl);

    /* free right command list */
    while(!TAILQ_EMPTY(&dec_right_cmdl))
    {
        temp = TAILQ_FIRST(&dec_right_cmdl);
        TAILQ_REMOVE(&dec_right_cmdl, temp, entry);
        while (!TAILQ_EMPTY(&temp->list))
        {
            psub = TAILQ_FIRST(&temp->list);
            TAILQ_REMOVE(&temp->list, psub, entry);
            XFREE(MTYPE_NEW_CTP_DECODE,psub->keyword);
            XFREE(MTYPE_NEW_CTP_DECODE,psub->value);
            XFREE(MTYPE_NEW_CTP_DECODE,psub);
        }
        XFREE(MTYPE_NEW_CTP_DECODE,temp->keyword);
        XFREE(MTYPE_NEW_CTP_DECODE,temp->value);
        XFREE(MTYPE_NEW_CTP_DECODE,temp);
    }
    TAILQ_INIT(&dec_left_cmdl);


    decode_free_header();
    /* payload release */
    if(NULL != payload)
    {
        XFREE(MTYPE_NEW_CTP_DECODE,payload);
        payload = NULL;
    }
    return CTP_E_NONE;
}

/*******************************************************************************
 * Name   : decode_command_set
 * Purpose: handle decorder command only for the control word.
 * Input  : keyw - keyword of field set or command set.
 *        : value- value of field set or command set
 * Output : None.
 * Return : .
 * Note   : N/A
*******************************************************************************/
ctp_error_e
ctc_pktxr_decode_command_set(const char *keyw, const char *value)
{
    char      *cmd_str = NULL;
    ctp_cmd_t *cmd_list = NULL;
    int       ret = 0;

    M_DEC_INITIALIZED;

    cmd_str = (char *)XMALLOC(MTYPE_NEW_CTP_DECODE,strlen(keyw) + 1);
    if (NULL == cmd_str)
    {
        return CTP_E_MEMORY_ERROR;
    }
    memset(cmd_str, 0, strlen(keyw) + 1);
    ctc_pktxr_convert_to_upper(cmd_str, keyw);

    if(strstr(cmd_str, "@RESET_DECORDER"))
    {
        XFREE(MTYPE_NEW_CTP_DECODE,cmd_str);
        /* kick off CTP send packet proecessing */
        CTP_ERR_RETURN(decoder_reset(), "decoder_reset");
        return CTP_E_NONE;
    }
    else
    {
        cmd_list =  (ctp_cmd_t *) XCALLOC(MTYPE_NEW_CTP_DECODE, sizeof(ctp_cmd_t));
        if (NULL == cmd_list)
        {
            XFREE(MTYPE_NEW_CTP_DECODE,cmd_str);
            return CTP_E_MEMORY_ERROR;
        }
        TAILQ_INIT(&cmd_list->list);
        cmd_list->keyword = cmd_str;
        cmd_str = (char *)XMALLOC(MTYPE_NEW_CTP_DECODE,strlen(value) + 1);
        if (NULL == cmd_str)
        {
            XFREE(MTYPE_NEW_CTP_DECODE,cmd_list->keyword);
            XFREE(MTYPE_NEW_CTP_DECODE,cmd_list);
            return CTP_E_MEMORY_ERROR;
        }
        strcpy(cmd_str, value);
        cmd_list->value   = cmd_str;
        TAILQ_INSERT_TAIL(&dec_left_cmdl, cmd_list, entry);
    }

    return CTP_E_NONE;
}

/*******************************************************************************
 * Name   : decoder_get_field
 * Purpose: according to the dec_pkt and field_id, and get the the segment infomations from dec_pkt.
 * Input  : field_id - input field ID.
 * Output : None.
 * Return : .
 * Note   : .
*******************************************************************************/
ctp_error_e
ctc_pktxr_decoder_get_field(uint16 field_id, ctp_field_seg_t **field_seg)
{
    ctp_header_info_t *curr_header = NULL;
    ctp_field_seg_t   *curr_seg = NULL;

    M_DEC_INITIALIZED;

    *field_seg = NULL;
    TAILQ_FOREACH(curr_header, &dec_pkt.header, entry)
    {
        TAILQ_FOREACH(curr_seg, &curr_header->seg_list, entry)
        {
            if (curr_seg->field_id == field_id)
            {
                *field_seg = curr_seg;
                return CTP_E_NONE;
            }
        }
    }

    return CTP_E_MIS_FIELD;
}

/*******************************************************************************
 * Name   : decoder_get_list_field
 * Purpose: according to the dec_pkt and field_id, and get the the segment infomations from dec_pkt.
 * Input  : field_id - input field ID.
 * Output : None.
 * Return : .
 * Note   : .
*******************************************************************************/
ctp_error_e
ctc_pktxr_decoder_get_list_field(uint16 field_id, ctp_field_seg_t **field_seg, int count)
{
    ctp_header_info_t *curr_header = NULL;
    ctp_field_seg_t   *curr_seg = NULL;

    M_DEC_INITIALIZED;

    *field_seg = NULL;
    TAILQ_FOREACH(curr_header, &dec_pkt.header, entry)
    {
        TAILQ_FOREACH(curr_seg, &curr_header->seg_list, entry)
        {
            if (curr_seg->field_id == field_id)
            {
                if (count)
                {
                    count--;
                    continue;
                }

                *field_seg = curr_seg;
                return CTP_E_NONE;
            }
        }
    }

    return CTP_E_MIS_FIELD;
}


/*******************************************************************************
 * Name   : ctc_pktxr_decoder_get_payload
 * Purpose: get the the segment infomations.
 * Input  : None.
 * Output : None.
 * Return : .
 * Note   : .
*******************************************************************************/
ctp_error_e
ctc_pktxr_decoder_get_payload(uint16 *len, uint8 **pld)
{
    *len = dec_cw[PKT_LEN];
    *pld = payload;

    return CTP_E_NONE;
}

#if 0
/*******************************************************************************
 * Name   : decoder_command_get_field
 * Purpose: get the value from the dec_pkt according to the keyw user give.
 * Input  : keyw.
 * Output : got_value.
 * Return : .
 * Note   : .
*******************************************************************************/
ctp_error_e
decoder_command_get_field(const char *keyw, char **got_value)
{
    int field_id = 0;
    int rs = 0;
    char *keyw_str = NULL;
    int str_len = 0;
    ctp_field_info_t *finfo = NULL;
    ctp_field_seg_t  *field_seg = NULL;
    keyw_str = (char *)XMALLOC(MTYPE_NEW_CTP_DECODE,strlen(keyw) + 1);
        if (NULL == keyw_str)
        {
            return CTP_E_MEMORY_ERROR;
        }
    memset(keyw_str, 0, strlen(keyw) + 1);
    strcpy(keyw_str, keyw);
    keyw_str++;
    *(keyw_str + strlen(keyw) - 2) = '\0' ;
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
    XFREE(MTYPE_NEW_CTP_DECODE,keyw_str);


    /* get field segment */
    rs = ctc_pktxr_decoder_get_field(field_id, &field_seg);// got the value from the dec_pkt
    if (CTP_E_NONE != rs)
    {
        CTS_DBG("get field failed");
        return rs;
    }

    str_len = ((field_seg->length + 7)/ 8) * 2 + 1;
    memset(*got_value, 0, str_len);
    ctc_pktxr_uint2str(*got_value, field_seg->value, str_len - 1);

    return CTP_E_NONE;

}
#endif



