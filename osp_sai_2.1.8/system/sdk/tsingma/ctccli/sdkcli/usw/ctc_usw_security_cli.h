/**
 *  @file ctc_usw_security_cli.h
 *
 *   @date 2011-11-25
 *
 *    @version v2.0
 *
 *    ---file comments----
 *    */

#ifndef _CTC_USW_SECURITY_CLI_H
#define _CTC_USW_SECURITY_CLI_H
#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"
#include "ctc_cli.h"

/**
 *  @brief  Initialize sdk security module CLIs, for greatbelt only
 *
 *   @param[in/out]
 *
 *    @return CTC_E_XXX
 *
 *    */
extern int32
ctc_usw_security_cli_init(void);

extern int32
ctc_usw_security_cli_deinit(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_USW_SECURITY_CLI_H */
