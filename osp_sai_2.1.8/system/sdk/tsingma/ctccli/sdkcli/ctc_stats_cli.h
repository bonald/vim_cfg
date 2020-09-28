/**
 @file ctc_stats_cli.h

 @date 2009-12-29

 @version v2.0

---file comments----
*/

#ifndef _CTC_STATS_CLI_H
#define _CTC_STATS_CLI_H
#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk statistics module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/

#ifdef __cplusplus
extern "C" {
#endif

extern int32
ctc_stats_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_STATS_CLI_H */

