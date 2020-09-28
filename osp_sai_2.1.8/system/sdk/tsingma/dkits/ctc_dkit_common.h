#ifndef _CTC_DKIT_COMMON_H_
#define _CTC_DKIT_COMMON_H_


#ifdef __cplusplus
extern "C" {
#endif


#define CENTEC_CFG_STAMP                 "This is centec config file"

#define CTC_DKITS_DUMP_HEX(S, F, V, W)                        \
                          ((sal_sprintf(F, "%%0%dX", W))    \
                           ? ((sal_sprintf(S, F, V)) ? S : NULL) : NULL)

#define CTC_DKITS_DUMP_BLOCK_DATA_BYTE(format) \
        (CTC_DKITS_FORMAT_TYPE_4BYTE == format) \
        ? sizeof(ctc_dkits_dump_tbl_block_data_t) : sizeof(uint32);

#define CTC_DKITS_DUMP_BLOCK_HDR_TBLID(p_tbl_block_hdr) \
        (((p_tbl_block_hdr)->tblid_8_12) << 8 | (p_tbl_block_hdr)->tblid_0_7);

#define CTC_DKITS_DUMP_BLOCK_HDR_TBLIDX(p_tbl_block_hdr) \
        ((p_tbl_block_hdr)->idx);

#define CTC_DKITS_DUMP_TCAM_SHARE_INF(offset, blk)   (((offset << 5) | blk) & 0xFFFFFFFF)
#define SHIFT_LEFT_MINUS_ONE(bits)    ( ((bits) < 32) ? ((1 << (bits)) - 1) : -1)

#define TBL_NAME_MAX_PRINT_LEN           35

#define TBL_IDX_MAX_PRINT_LEN            8
#define TBL_NAME_IDX_MAX_PRINT_LEN       (TBL_NAME_MAX_PRINT_LEN + TBL_IDX_MAX_PRINT_LEN)
#define FIELD_NAME_MAX_PRINT_LEN         24
#define COLUNM_SPACE_LEN                 2
#define DUMP_TBL_WORD_NUM_PER_LINE       7
#define DUMP_TCAM_DATA_WORD_NUM_PER_LINE 3
#define DUMP_TCAM_MASK_WORD_NUM_PER_LINE 3
#define DUMP_TCAM_WORD_NUM_PER_LINE      (DUMP_TCAM_DATA_WORD_NUM_PER_LINE + DUMP_TCAM_MASK_WORD_NUM_PER_LINE)
#define DIFF_HASH_WORD_NUM_PER_LINE      6
#define DIFF_TCAM_WORD_NUM_PER_LINE      8
#define DIFF_TCAM_WORD_ALTER_NUM         2
#define DIFF_HASH_WORD_ALTER_NUM         3
#define DUMP_ENTRY_WORD_STR_LEN          8
#define DUMP_ENTRY_WORD_SPACE_LEN        1
#define DUMP_LIVE_HINT_INTERVAL          100000000
#define DUMP_CENTEC_FILE_POSTFIX         ".ctc"
#define DUMP_DECODC_TXT_FILE_POSTFIX     ".txt"
#define DUMP_CFG_HASH_ORDER_OFFSET       0x0
#define DUMP_CFG_TCAM_ORDER_OFFSET       0x100000
#define DUMP_CFG_NH_ORDER_OFFSET         0x1000000

struct ctc_dkits_dump_centec_file_hdr_s
{
    uint32 endian_type;
    uint8  centec_cfg_stamp[sizeof(CENTEC_CFG_STAMP)];
    uint8  version[sizeof(CTC_DKITS_VERSION_STR)];
    uint8  sdk_release_data[sizeof(CTC_DKITS_RELEASE_DATE)];
    uint8  copy_right_time[sizeof(CTC_DKITS_COPYRIGHT_TIME)];
    uint8  chip_name[20];
    uint8  rsv[3];
    ctc_dkits_dump_func_flag_t  func_flag;
}__attribute__((packed));
typedef struct ctc_dkits_dump_centec_file_hdr_s ctc_dkits_dump_centec_file_hdr_t;


struct ctc_dkits_dump_tbl_entry_s
{
    uint32* data_entry;
    uint32* mask_entry;
    uint32* ad_entry;
};
typedef struct ctc_dkits_dump_tbl_entry_s ctc_dkits_dump_tbl_entry_t;

enum ctc_dkits_dump_tbl_type_e
{
    CTC_DKITS_DUMP_TBL_TYPE_STATIC,
    CTC_DKITS_DUMP_TBL_TYPE_SERDES,
    CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY,
    CTC_DKITS_DUMP_TBL_TYPE_HASH_NO_AD,
    CTC_DKITS_DUMP_TBL_TYPE_HASH_AD,
    CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY,
    CTC_DKITS_DUMP_TBL_TYPE_TCAM_AD,
    CTC_DKITS_DUMP_TBL_TYPE_HN,
    CTC_DKITS_DUMP_TBL_TYPE_DBG,
    CTC_DKITS_DUMP_TBL_TYPE_NUM,
    CTC_DKITS_DUMP_TBL_TYPE_INVALID = CTC_DKITS_DUMP_TBL_TYPE_NUM
};
typedef enum ctc_dkits_dump_tbl_type_e ctc_dkits_dump_tbl_type_t;

struct ctc_dkits_dump_key_tbl_info_s
{
    uint32 tblid;
    uint32 idx;
    uint32 ad_tblid;
    uint32 ad_idx;
    uint32 order;
    ctc_dkits_dump_tbl_type_t tbl_type;
};
typedef struct ctc_dkits_dump_key_tbl_info_s ctc_dkits_dump_key_tbl_info_t;

#if (HOST_IS_LE == 0)
struct ctc_dkits_dump_tbl_block_hdr_s
{
    uint32 format       :2;
    uint32 entry_num    :7;
    uint32 idx          :18;
    uint32 tblid_8_12   :5;
    uint8  tblid_0_7;
}__attribute__((packed));
typedef struct ctc_dkits_dump_tbl_block_hdr_s ctc_dkits_dump_tbl_block_hdr_t;

struct ctc_dkits_dump_serdes_tbl_s
{
    uint32 serdes_id   :7;
    uint32 serdes_mode :3;
    uint32 offset      :6;
    uint32 data        :16;
};
typedef struct ctc_dkits_dump_serdes_tbl_s ctc_dkits_dump_serdes_tbl_t;

#else

struct ctc_dkits_dump_tbl_block_hdr_s
{
    uint32 tblid_8_12   :5;
    uint32 idx          :18;
    uint32 entry_num    :7;
    uint32 format       :2;
    uint8  tblid_0_7;
}__attribute__((packed));
typedef struct ctc_dkits_dump_tbl_block_hdr_s ctc_dkits_dump_tbl_block_hdr_t;

struct ctc_dkits_dump_serdes_tbl_s
{
    uint32 data        :16;
    uint32 rsv         :1;
    uint32 offset      :6;
    uint32 serdes_mode :2;
    uint32 serdes_id   :7;
};
typedef struct ctc_dkits_dump_serdes_tbl_s ctc_dkits_dump_serdes_tbl_t;

#endif

enum ctc_dkits_dump_text_header_type_e
{
    CTC_DKITS_DUMP_TEXT_HEADER_DUMP_TBL,
    CTC_DKITS_DUMP_TEXT_HEADER_DUMP_HASH = CTC_DKITS_DUMP_TEXT_HEADER_DUMP_TBL,
    CTC_DKITS_DUMP_TEXT_HEADER_DUMP_SERDES,
    CTC_DKITS_DUMP_TEXT_HEADER_DUMP_TCAM,
    CTC_DKITS_DUMP_TEXT_HEADER_DIFF_TBL,
    CTC_DKITS_DUMP_TEXT_HEADER_DIFF_SERDES,
    CTC_DKITS_DUMP_TEXT_HEADER_DIFF_HASH,
    CTC_DKITS_DUMP_TEXT_HEADER_DIFF_TCAM,
    CTC_DKITS_DUMP_TEXT_HEADER_NUM
};
typedef enum ctc_dkits_dump_text_header_type_e ctc_dkits_dump_text_header_type_t;

enum ctc_dkit_serdes_mode_e
{
    CTC_DKIT_SERDES_TX,
    CTC_DKIT_SERDES_RX,
    CTC_DKIT_SERDES_COMMON,
    CTC_DKIT_SERDES_PLLA,
    CTC_DKIT_SERDES_PLLB,              /*Only GG*/
    CTC_DKIT_SERDES_AEC,               /*Only GG*/
    CTC_DKIT_SERDES_AET,               /*Only GG*/
    CTC_DKIT_SERDES_COMMON_PLL,
    CTC_DKIT_SERDES_AEC_AET,       /*Only GG*/
    CTC_DKIT_SERDES_LINK_TRAINING,       /*Only TM*/
    CTC_DKIT_SERDES_BIST,          /*Only TM*/
    CTC_DKIT_SERDES_FW,          /*Only TM*/
    CTC_DKIT_SERDES_DETAIL,      /*Only TM*/
    CTC_DKIT_SERDES_ALL
};
typedef enum ctc_dkit_serdes_mode_e ctc_dkit_serdes_mode_t;

struct ctc_dkits_ds_s
{
    uint32 entry[32];
};
typedef struct ctc_dkits_ds_s ctc_dkits_ds_t;

struct ctc_dkit_chip_api_s
{
    /*Dump Cfg*/
    int32 (*dkits_dump_decode_entry) (uint8 lchip, void*, void*);
    int32 (*dkits_dump_data2tbl)(uint8 lchip, ctc_dkits_dump_tbl_type_t tbl_type, uint8 cfg_endian, ctc_dkits_dump_tbl_block_hdr_t* p_tbl_block_hdr, uint8* p_buf, ctc_dkits_dump_tbl_entry_t* p_tbl_entry);

    /*Driver */
    int32 (*drv_get_tbl_string_by_id) (uint8 lchip, uint32 tbl_id, char* name);
    int32 (*drv_chip_read) (uint8 lchip, uint32 offset, uint32* p_value);
    int32 (*drv_chip_write) (uint8 lchip, uint32 offset, uint32 value);
    int32 (*drv_get_host_type) (uint8 lchip);

    /*Serdes*/
    int32 (*dkits_dump_serdes_status) (uint8 lchip, uint16 serdes_id, uint32 type, char* file_name);
    int32 (*dkits_read_serdes) (void* para);
    int32 (*dkits_write_serdes) (void* para);
    int32 (*dkits_serdes_ctl) (void* para);
    int32 (*dkits_read_serdes_aec_aet) (void* para);
    int32 (*dkits_write_serdes_aec_aet) (void* para);
    int32 (*dkits_serdes_resert) (uint8 lchip, uint16 serdes_id);
    int32 (*dkits_dump_indirect) (uint8 lchip, sal_file_t p_f, ctc_dkits_dump_func_flag_t flag);

    /*misc*/
    int32 (*dkits_sensor_result)(void*);
    int32 (*dkits_show_pcs_status)(uint8 lchip, uint16 mac_id);
};
typedef struct ctc_dkit_chip_api_s ctc_dkit_chip_api_t;

extern int32
ctc_dkits_dump_read_bin(uint8 lchip, uint8* p_buf, uint8 cfg_endian, ctc_dkits_dump_tbl_block_hdr_t* p_tbl_block_hdr, sal_file_t p_rf);

extern void
ctc_dkits_dump_swap32(uint32* data, int32 len);

extern void
ctc_dkits_dump_txt_header(ctc_dkits_dump_text_header_type_t type, sal_file_t p_wf);

extern int32
ctc_dkits_dump_evaluate_entry(uint16 word, uint32* p_entry, uint8* p_repeat);

extern void
ctc_dkits_dump_cfg_brief_str(char* p_str, uint8 pos, int8 extr_len);

extern int32
ctc_dkits_dump_diff_flex(ctc_dkits_dump_key_tbl_info_t* p_key_tbl_info,
                          ctc_dkits_dump_tbl_entry_t* p_src_tbl,
                          ctc_dkits_dump_tbl_entry_t* p_dst_tbl,
                          sal_file_t p_wf);

extern int32
ctc_dkits_dump_plain_entry(uint8 word, uint32* p_entry, uint8 *p_buf, uint16* p_buf_len);

extern int32
ctc_dkits_dump_get_tbl_entry(uint8 lchip, sal_file_t p_rf, uint8 cfg_endian, ctc_dkits_dump_tbl_entry_t* p_tbl_entry,
                    ctc_dkits_dump_key_tbl_info_t* p_info);

#ifdef __cplusplus
}
#endif

#endif

