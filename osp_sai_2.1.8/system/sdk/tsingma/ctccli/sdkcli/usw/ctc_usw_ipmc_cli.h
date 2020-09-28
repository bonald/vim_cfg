#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
#ifndef _CTC_USW_IPMC_CLI_H
#define _CTC_USW_IPMC_CLI_H
#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"
#include "ctc_cli.h"

extern int32
ctc_usw_ipmc_cli_init(void);

extern int32
ctc_usw_ipmc_cli_deinit(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_IPMC_CLI_H */

#endif

