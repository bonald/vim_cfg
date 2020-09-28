/**
 @file ctc_usw_interrupt_cli.h

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-10-23

 @version v2.0

 This file define interrupt CLI functions

*/

#ifndef _CTC_USW_INTERRUPT_CLI_H
#define _CTC_USW_INTERRUPT_CLI_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_cli.h"

extern int32
ctc_usw_interrupt_cli_init(void);

extern int32
ctc_usw_interrupt_cli_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* _CTC_USW_INTERRUPT_CLI_H */

