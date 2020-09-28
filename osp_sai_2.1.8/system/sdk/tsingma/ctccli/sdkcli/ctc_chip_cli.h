/**
 @file ctc_chip_cli.h

 @date 2012-07-09

 @version v2.0

---file comments----
*/

#ifndef _CTC_CHIP_CLI_H
#define _CTC_CHIP_CLI_H

#include "sal.h"
#include "ctc_cli.h"

#define CHECK_FLAG(V, F)      ((V)&(F))
#define SET_FLAG(V, F)        (V) = (V) | (F)
#define UNSET_FLAG(V, F)      (V) = (V)&~(F)

#ifdef __cplusplus
extern "C" {
#endif

extern int32
ctc_chip_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_CHIP_CLI_H */

