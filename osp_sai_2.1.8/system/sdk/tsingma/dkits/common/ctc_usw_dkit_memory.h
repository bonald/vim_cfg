#ifndef _CTC_DKIT_GOLDENGATE_MEMORY_H
#define _CTC_DKIT_GOLDENGATE_MEMORY_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_dkit.h"

extern int32
ctc_usw_dkit_memory_process(void* p_para);

extern int32
ctc_usw_dkit_memory_is_invalid_table(uint8 lchip, uint32 tbl_id);

extern int32
ctc_usw_dkit_memory_read_table(uint8 lchip, void* p_param);

extern int32
ctc_usw_dkit_memory_write_table(uint8 lchip, void* p_param);

#ifdef __cplusplus
}
#endif

#endif


