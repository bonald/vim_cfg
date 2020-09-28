#ifndef _CTC_DKIT_GOLDENGATE_MISC_H
#define _CTC_DKIT_GOLDENGATE_MISC_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_dkit.h"

#if 1
extern int32
ctc_usw_dkit_misc_serdes_ctl(void* p_para);
#endif

extern int32
ctc_usw_dkit_misc_integrity_check(uint8 lchip, void*, void*, void*);

extern int32
ctc_usw_dkit_misc_chip_info(uint8 lchip, sal_file_t p_wf);

#if 1
extern int32
ctc_usw_dkit_misc_read_serdes(void*);

extern int32
ctc_usw_dkit_misc_write_serdes(void*);

extern int32
ctc_usw_dkit_misc_read_serdes_aec_aet(void*);

extern int32
ctc_usw_dkit_misc_write_serdes_aec_aet(void*);

extern int32
ctc_usw_dkit_misc_serdes_resert(uint8 lchip, uint16 serdes_id);

extern int32
ctc_usw_dkit_misc_serdes_status(uint8 lchip, uint16 serdes_id, uint32 type, char* file_name);

extern int32
ctc_usw_dkit_misc_serdes_register(uint8 lchip);

#endif

#ifdef __cplusplus
}
#endif

#endif


