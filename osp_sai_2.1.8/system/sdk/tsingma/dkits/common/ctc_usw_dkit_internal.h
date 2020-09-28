#ifndef _CTC_DKIT_USW_INTERNAL_H
#define _CTC_DKIT_USW_INTERNAL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_dkit.h"

extern int32
ctc_usw_dkit_internal_cli_init(uint8 lchip, uint8 cli_tree_mode);

extern int32
ctc_usw_dkit_internal_cli_deinit(uint8 lchip, uint8 cli_tree_mode);

#ifdef __cplusplus
}
#endif

#endif





