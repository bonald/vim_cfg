
#include "ctc_cli.h"
#include "ctc_dkit.h"
#include "ctc_dkit_common.h"
#include "ctc_dkit_cli.h"

#define DKITS_SWAP32(val) \
    ((uint32)( \
         (((uint32)(val) & (uint32)0x000000ffUL) << 24) | \
         (((uint32)(val) & (uint32)0x0000ff00UL) << 8) | \
         (((uint32)(val) & (uint32)0x00ff0000UL) >> 8) | \
         (((uint32)(val) & (uint32)0xff000000UL) >> 24)))

enum swap_direction_e
{
    HOST_TO_NETWORK,
    NETWORK_TO_HOST
};
typedef enum swap_direction_e swap_direction_t;

enum ctc_dkits_format_type_e
{
    CTC_DKITS_FORMAT_TYPE_NONE,
    CTC_DKITS_FORMAT_TYPE_1BYTE,
    CTC_DKITS_FORMAT_TYPE_2BYTE,
    CTC_DKITS_FORMAT_TYPE_4BYTE,
    CTC_DKITS_FORMAT_TYPE_NUM
};
typedef enum ctc_dkits_format_type_e ctc_dkits_format_type_t;

enum ctc_dkits_dump_tbl_diff_flag_e
{
    CTC_DKITS_DUMP_TBL_DIFF_FLAG_TBL = 1U << 0,
    CTC_DKITS_DUMP_TBL_DIFF_FLAG_KEY = 1U << 1,
    CTC_DKITS_DUMP_TBL_DIFF_FLAG_AD  = 1U << 2,
    CTC_DKITS_DUMP_TBL_DIFF_FLAG_ALL = 1U << 3
};
typedef enum ctc_dkits_dump_tbl_diff_flag_e ctc_dkits_dump_tbl_diff_flag_t;

struct ctc_dkits_dump_tbl_block_data_s
{
    uint32 entry;
    uint8  repeat;
}__attribute__((packed));
typedef struct ctc_dkits_dump_tbl_block_data_s ctc_dkits_dump_tbl_block_data_t;

extern ctc_dkit_chip_api_t* g_dkit_chip_api[CTC_DKITS_MAX_LOCAL_CHIP_NUM];

#define __INTERNAL_FUNCTION__
void
ctc_dkits_dump_swap32(uint32* data, int32 len)
{
    int32 cnt;

    for (cnt = 0; cnt < len; cnt++)
    {
        data[cnt] = DKITS_SWAP32(data[cnt]);
    }

    return;
}

void
ctc_dkits_dump_cfg_brief_str(char* p_str, uint8 pos, int8 extr_len)
{
    if (extr_len < 0)
    {
        return;
    }

    sal_memcpy(p_str + pos - extr_len, p_str + pos, (sal_strlen(p_str + pos) + 1));

    return;
}

int32
ctc_dkits_dump_evaluate_entry(uint16 word, uint32* p_entry, uint8* p_repeat)
{
    uint32 i = 0, j = 0;
    uint32 entry = 0;

    /* 4 byte repeat */
    for (i = 0; i < word; i++)
    {
        entry = *(p_entry + i);

        for (j = i + 1; j < word; j++)
        {
            if (*(p_entry + j) != entry)
            {
                break;
            }

            if (*(p_entry + j) == entry)
            {
                (*p_repeat)++;
                i++;
            }
        }
    }

    return CLI_SUCCESS;
}

int32
ctc_dkits_dump_plain_entry(uint8 word, uint32* p_entry, uint8 *p_buf, uint16* p_buf_len)
{
    uint8 i = 0;

    for (i = 0; i < word; i++)
    {
        sal_memcpy(p_buf + *p_buf_len, p_entry + i, sizeof(uint32));
        *p_buf_len += sizeof(uint32);
    }

    return CLI_SUCCESS;
}

int32
ctc_dkits_dump_diff_flex(ctc_dkits_dump_key_tbl_info_t* p_key_tbl_info,
                          ctc_dkits_dump_tbl_entry_t* p_src_tbl,
                          ctc_dkits_dump_tbl_entry_t* p_dst_tbl,
                          sal_file_t p_wf)
{
    uint32 serdes_id = 0;
    uint32 register_mode = 0;
    uint32 offset = 0;
    uint32 value1 = 0;
    uint32 value2 = 0;
    char format2[100] = {0};
    char* p_mode[CTC_DKIT_SERDES_PLLB+1] = {"Tx", "Rx", "Common", "PLLA", "PLLB"};
    char str1[100] = {0};
    char str2[100] = {0};
    char format[50] = {0};

    if (p_key_tbl_info->tbl_type == CTC_DKITS_DUMP_TBL_TYPE_SERDES)
    {
        ctc_dkits_dump_serdes_tbl_t* p_src_serdes_tbl = (ctc_dkits_dump_serdes_tbl_t*)p_src_tbl->data_entry;
        ctc_dkits_dump_serdes_tbl_t* p_dst_serdes_tbl = (ctc_dkits_dump_serdes_tbl_t*)p_dst_tbl->data_entry;

        if (p_src_serdes_tbl->data != p_dst_serdes_tbl->data)
        {
                sal_sprintf(format2, "%%-%du%%-%ds%%-%du%%-%ds%%-%ds", 10, 10, 10, 20, 20);
                serdes_id = p_src_serdes_tbl->serdes_id;
                register_mode = p_src_serdes_tbl->serdes_mode;
                offset = p_src_serdes_tbl->offset;
                value1 = p_src_serdes_tbl->data;
                value2 = p_dst_serdes_tbl->data;

                sal_fprintf(p_wf, format2, serdes_id, ((register_mode<=CTC_DKIT_SERDES_PLLB)?p_mode[register_mode]:""), offset,
                    CTC_DKITS_DUMP_HEX(str1, format, value1, 4), CTC_DKITS_DUMP_HEX(str2, format, value2, 4));
                sal_fprintf(p_wf, "\n");
        }
    }

    return 0;
}

int32
ctc_dkits_dump_get_tbl_entry(uint8 lchip, sal_file_t p_rf, uint8 cfg_endian, ctc_dkits_dump_tbl_entry_t* p_tbl_entry,
                    ctc_dkits_dump_key_tbl_info_t* p_info)
{
    uint8  buf[500] = {0};
    ctc_dkits_dump_tbl_block_hdr_t tbl_block_hdr;
    int32 ret = 0;

    /*get entry from bin*/
    sal_memset(&tbl_block_hdr, 0, sizeof(ctc_dkits_dump_tbl_block_hdr_t));
    ctc_dkits_dump_read_bin(lchip, buf, cfg_endian, &tbl_block_hdr, p_rf);

    /*get entry refer info:tbl_type, order */
     /*ret = _ctc_greatbelt_dkits_dump_decode_entry(&tbl_block_hdr, p_info);*/
    ret = g_dkit_chip_api[lchip]->dkits_dump_decode_entry(lchip, &tbl_block_hdr, p_info);

    /*store entry info */
    ret = g_dkit_chip_api[lchip]->dkits_dump_data2tbl(lchip, p_info->tbl_type, cfg_endian, &tbl_block_hdr, buf, p_tbl_entry);

    return ret;
}

int32
ctc_dkits_dump_read_bin(uint8 lchip, uint8* p_buf, uint8 cfg_endian, ctc_dkits_dump_tbl_block_hdr_t* p_tbl_block_hdr, sal_file_t p_rf)
{
    uint32 entry_byte = 0;
    uint8 host_endian;
    uint32 ret = 0;

    host_endian = g_dkit_chip_api[lchip]->drv_get_host_type(lchip);
    ret = sal_fread(p_tbl_block_hdr, sizeof(ctc_dkits_dump_tbl_block_hdr_t), 1, p_rf);
    if(1 != ret)
    {
        return CLI_ERROR;
    }

    if (cfg_endian != host_endian)
    {
        ctc_dkits_dump_swap32((uint32*)p_tbl_block_hdr, 1);
    }

    if (CTC_DKITS_FORMAT_TYPE_4BYTE == p_tbl_block_hdr->format)
    {
        entry_byte = sizeof(ctc_dkits_dump_tbl_block_data_t);
    }
    else
    {
        entry_byte = sizeof(uint32);
    }
    ret = sal_fread(p_buf, (p_tbl_block_hdr->entry_num + 1) * entry_byte, 1, p_rf);
    if(1 != ret)
    {
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

void
ctc_dkits_dump_txt_header(ctc_dkits_dump_text_header_type_t type, sal_file_t p_wf)
{
    uint32 n = 0, i = 0;
    char format[100] = {0};

    if (CTC_DKITS_DUMP_TEXT_HEADER_DUMP_TBL == type)
    {
        n = TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN \
            + (DUMP_TBL_WORD_NUM_PER_LINE * (DUMP_ENTRY_WORD_STR_LEN + DUMP_ENTRY_WORD_SPACE_LEN));

        for (i = 0; i < n; i++)
        {
            sal_fprintf(p_wf, "-");
        }
        sal_fprintf(p_wf, "\n");

        sal_sprintf(format, "%%-%ds%%s\n", (TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN));

        sal_fprintf(p_wf, format, "TblName[Index]", "Value");

        for (i = 0; i < n; i++)
        {
            sal_fprintf(p_wf, "-");
        }
        sal_fprintf(p_wf, "\n");
    }
    else if (CTC_DKITS_DUMP_TEXT_HEADER_DUMP_TCAM == type)
    {
        n = TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN \
            + (DUMP_TCAM_WORD_NUM_PER_LINE * (DUMP_ENTRY_WORD_STR_LEN + DUMP_ENTRY_WORD_SPACE_LEN));

        for (i = 0; i < n; i++)
        {
            sal_fprintf(p_wf, "-");
        }
        sal_fprintf(p_wf, "\n");

        sal_sprintf(format, "%%-%ds%%-%ds%%s\n",
                    (TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN),
                    (((DUMP_TCAM_WORD_NUM_PER_LINE/2) * DUMP_ENTRY_WORD_STR_LEN)
                    + (DUMP_ENTRY_WORD_SPACE_LEN * 3)));

        sal_fprintf(p_wf, format, "TblName[Index]", "Data", "Mask");

        for (i = 0; i < n; i++)
        {
            sal_fprintf(p_wf, "-");
        }
        sal_fprintf(p_wf, "\n");
    }
    else if (CTC_DKITS_DUMP_TEXT_HEADER_DIFF_TBL == type)
    {
        n = TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN \
            + FIELD_NAME_MAX_PRINT_LEN + COLUNM_SPACE_LEN \
            + (DUMP_ENTRY_WORD_STR_LEN * 2) + DUMP_ENTRY_WORD_SPACE_LEN;

        for (i = 0; i < n; i++)
        {
            sal_fprintf(p_wf, "-");
        }
        sal_fprintf(p_wf, "\n");

        sal_sprintf(format, "%%-%ds%%-%ds%%-9s%%-9s\n", (TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN),
                                                        (FIELD_NAME_MAX_PRINT_LEN + COLUNM_SPACE_LEN));

        sal_fprintf(p_wf, format, "TblName[Index]", "FieldName", "File1", "File2");

        for (i = 0; i < n; i++)
        {
            sal_fprintf(p_wf, "-");
        }
        sal_fprintf(p_wf, "\n");
    }
    else if (CTC_DKITS_DUMP_TEXT_HEADER_DIFF_HASH == type)
    {
        n = TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN \
            + (DIFF_HASH_WORD_NUM_PER_LINE * DUMP_ENTRY_WORD_STR_LEN)
            + ((DIFF_HASH_WORD_NUM_PER_LINE - 1) * DUMP_ENTRY_WORD_SPACE_LEN);

        for (i = 0; i < n; i++)
        {
            sal_fprintf(p_wf, "-");
        }
        sal_fprintf(p_wf, "\n");

        sal_sprintf(format, "%%-%ds%%-%ds%%s\n",
                    (TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN),
                    ((DIFF_HASH_WORD_NUM_PER_LINE / 2) * (DUMP_ENTRY_WORD_STR_LEN + DUMP_ENTRY_WORD_SPACE_LEN)));

        sal_fprintf(p_wf, format, "TblName[Index]", "File1", "File2");

        for (i = 0; i < n; i++)
        {
            sal_fprintf(p_wf, "-");
        }
        sal_fprintf(p_wf, "\n");
    }
    else if (CTC_DKITS_DUMP_TEXT_HEADER_DIFF_TCAM == type)
    {
        n = TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN \
            + (DIFF_TCAM_WORD_NUM_PER_LINE * DUMP_ENTRY_WORD_STR_LEN)
            + ((DIFF_TCAM_WORD_NUM_PER_LINE - 1) * DUMP_ENTRY_WORD_SPACE_LEN);

        for (i = 0; i < n; i++)
        {
            sal_fprintf(p_wf, "-");
        }
        sal_fprintf(p_wf, "\n");

        sal_sprintf(format, "%%-%ds%%-%ds%%-%ds%%-%ds%%s\n",
                    (TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN),
                    ((DIFF_TCAM_WORD_NUM_PER_LINE / 4) * (DUMP_ENTRY_WORD_STR_LEN + DUMP_ENTRY_WORD_SPACE_LEN)),
                    ((DIFF_TCAM_WORD_NUM_PER_LINE / 4) * (DUMP_ENTRY_WORD_STR_LEN + DUMP_ENTRY_WORD_SPACE_LEN)),
                    ((DIFF_TCAM_WORD_NUM_PER_LINE / 4) * (DUMP_ENTRY_WORD_STR_LEN + DUMP_ENTRY_WORD_SPACE_LEN)));

        sal_fprintf(p_wf, format, "TblName[Index]", "File1 data", "File1 mask", "File2 data", "File2 mask");

        for (i = 0; i < n; i++)
        {
            sal_fprintf(p_wf, "-");
        }
        sal_fprintf(p_wf, "\n");
    }
    else if (CTC_DKITS_DUMP_TEXT_HEADER_DUMP_SERDES == type)
    {
        n = TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN \
            + (DUMP_TBL_WORD_NUM_PER_LINE * (DUMP_ENTRY_WORD_STR_LEN + DUMP_ENTRY_WORD_SPACE_LEN));

        for (i = 0; i < n; i++)
        {
            sal_fprintf(p_wf, "-");
        }
        sal_fprintf(p_wf, "\n");

        sal_sprintf(format, "%%-%ds%%-%ds%%-%ds%%s\n", 10, 10, 10);

        sal_fprintf(p_wf, format, "SerdesID", "Register", "Offset", "Value");

        for (i = 0; i < n; i++)
        {
            sal_fprintf(p_wf, "-");
        }
        sal_fprintf(p_wf, "\n");
    }
    else if (CTC_DKITS_DUMP_TEXT_HEADER_DIFF_SERDES == type)
    {
        n = TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN \
            + (DUMP_TBL_WORD_NUM_PER_LINE * (DUMP_ENTRY_WORD_STR_LEN + DUMP_ENTRY_WORD_SPACE_LEN));

        for (i = 0; i < n; i++)
        {
            sal_fprintf(p_wf, "-");
        }
        sal_fprintf(p_wf, "\n");

        sal_sprintf(format, "%%-%ds%%-%ds%%-%ds%%-%ds%%-%ds\n", 10, 10, 10, 20, 20);

        sal_fprintf(p_wf, format, "SerdesID", "Register", "Offset", "File1 Data", "File2 Data");

        for (i = 0; i < n; i++)
        {
            sal_fprintf(p_wf, "-");
        }
        sal_fprintf(p_wf, "\n");
    }
    return;
}

