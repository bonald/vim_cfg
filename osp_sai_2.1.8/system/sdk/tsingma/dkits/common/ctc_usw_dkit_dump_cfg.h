#ifndef _CTC_GOLDENGATE_DKIT_DUMP_H_
#define _CTC_GOLDENGATE_DKIT_DUMP_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"
#define CTC_DKITS_FIELD_LEN(p_field)    (p_field->bits)    /* Size of fields */
extern int32 ctc_usw_dkits_dump_tbl2data(uint8 lchip, uint16 tbl_id, uint16 index, uint32* p_data_entry, uint32* p_mask_entry, sal_file_t p_f);
extern int32 ctc_usw_dkits_dump_memory_usage(uint8 lchip, void* p_para);
extern int32 ctc_usw_dkits_dump_cfg_dump(uint8 lchip, void* p_info);
extern int32 ctc_usw_dkits_dump_cfg_decode(uint8 lchip, void* p_para);
extern int32 ctc_usw_dkits_dump_cfg_cmp(uint8 lchip, void* p_para1, void* p_para2);
extern int32 ctc_usw_dkits_dump_decode_entry(uint8 lchip, void* p_para1, void* p_para2);
extern int32 ctc_usw_dkits_dump_data2text(uint8 lchip, sal_file_t p_rf, uint8 cfg_endian, sal_file_t p_wf, uint8 data_translate);
extern int32 ctc_usw_dkits_dump_data2tbl(uint8 lchip, ctc_dkits_dump_tbl_type_t tbl_type, uint8 cfg_endian, ctc_dkits_dump_tbl_block_hdr_t* p_tbl_block_hdr, uint8* p_buf, ctc_dkits_dump_tbl_entry_t* p_tbl_entry);
#ifdef __cplusplus
}
#endif

#endif

