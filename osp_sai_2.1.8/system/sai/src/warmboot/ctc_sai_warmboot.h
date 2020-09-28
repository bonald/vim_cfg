
#ifndef __SAI_WARMBOOT_H_
#define __SAI_WARMBOOT_H_

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

int32
ctc_sai_set_wb_en(uint32 en);

int32
ctc_sai_set_wb_restoring(uint32 restoring);

uint32
ctc_sai_get_wb_restoring();

int32
ctc_sai_warmboot_init();

#endif  /* __SAI_WARMBOOT_H_ */

