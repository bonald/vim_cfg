/**
 @file ctc_dma_cli.h

 @date 2012-3-20

 @version v2.0

---file comments----
*/

#ifndef _CTC_USW_DMA_CLI_H
#define _CTC_USW_DMA_CLI_H
#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk DMA module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/
extern int32
ctc_usw_dma_cli_init(void);

extern int32
ctc_usw_dma_cli_deinit(void);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_USW_DMA_CLI_H */

