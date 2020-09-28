#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_dkit.h"

extern int32
ctc_dt2_dkit_misc_serdes_status(uint8 lchip, uint16 serdes_id, uint32 type, char* file_name);

extern int32 
ctc_dt2_dkit_misc_serdes_resert(uint8 lchip, uint16 serdes_id);

extern int32
ctc_dt2_dkit_misc_read_serdes(void* para);

extern int32 
ctc_dt2_dkit_misc_write_serdes(void* para);

extern int32
ctc_dt2_dkit_misc_serdes_ctl(void* p_para);

extern int32
ctc_dt2_dkit_misc_read_serdes_aec_aet(void* para);

extern int32
ctc_dt2_dkit_misc_write_serdes_aec_aet(void* para);
extern int32
ctc_dt2_dkits_misc_dump_indirect(uint8 lchip, sal_file_t p_f, ctc_dkits_dump_func_flag_t flag);

#ifdef __cplusplus
}
#endif



