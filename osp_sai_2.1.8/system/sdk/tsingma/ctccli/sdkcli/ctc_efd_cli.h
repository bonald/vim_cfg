/**
 @file ctc_efd_cli.h

 @date 2011-11-25

 @version v2.0

---file comments----
*/

#ifndef _CTC_EFD_CLI_H
#define _CTC_EFD_CLI_H
#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk EFD module CLIs, only for show function

 @param[in/out]

 @return CTC_E_XXX

*/
int32
ctc_efd_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_EFD_CLI_H */

