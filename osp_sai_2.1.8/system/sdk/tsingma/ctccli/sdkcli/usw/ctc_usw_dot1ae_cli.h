#if (FEATURE_MODE == 0)
/**
 *  @file ctc_greatbel_dot1ae_cli.h
 *
 *   @date 2017-08-22
 *
 *    @version v2.0
 *
 *    ---file comments----
 *    */

#ifndef _CTC_USW_DOT1AE_CLI_H
#define _CTC_USW_DOT1AE_CLI_H
#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"
#include "ctc_cli.h"

/**
 *  @brief  Initialize sdk dot1ae module CLIs, for usw only
 *
 *   @param[in/out]
 *
 *    @return CTC_E_XXX
 *
 *    */
extern int32
ctc_usw_dot1ae_cli_init(void);

extern int32
ctc_usw_dot1ae_cli_deinit(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_USW_DOT1AE_CLI_H */

#endif

