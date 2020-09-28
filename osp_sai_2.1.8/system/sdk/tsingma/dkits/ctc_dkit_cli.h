#ifndef _CTC_DKIT_CLI_H_
#define _CTC_DKIT_CLI_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "sal.h"

extern int32 ctc_dkit_cli_chip_special_deinit(uint8 lchip);
extern int32 ctc_dkit_cli_chip_special_init(uint8 lchip);
extern int32 ctc_dkit_cli_init(uint8 cli_tree_mode);

#ifdef __cplusplus
}
#endif

#endif

