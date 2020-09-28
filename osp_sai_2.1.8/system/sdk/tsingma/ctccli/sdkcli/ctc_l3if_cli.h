/**
 @file ctc_l3if_cli.h

 @date 2009-10-30

 @version v2.0

---file comments----
*/

#ifndef _CTC_L3IF_CLI_H
#define _CTC_L3IF_CLI_H
#include "sal.h"
#include "ctc_cli.h"

/****************************************************************************
 *
* Define
*
*****************************************************************************/
#define L3IF_CLI_SHOW_PROPERTY_FORMATE "%-31s%s\n"

extern char *prop_str[];

/**
 @brief  Initialize sdk Mirror module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/

#ifdef __cplusplus
extern "C" {
#endif

int32
ctc_l3if_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_L3IF_CLI_H */

