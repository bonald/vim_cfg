/**
 *  @file ctc_packet_test_cli.h
 *
 *   @date 2018-12-20
 *
 *    @version v1.0
 *
 *    ---file comments----
 *    */

#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"
#include "ctc_cli.h"

/**
 *  @brief  Initialize sdk packet test module CLIs, for usw only
 *
 *   @param[in/out]
 *
 *    @return CTC_E_XXX
 *
 *    */
extern int32
ctc_usw_packet_test_cli_init(void);

extern int32
ctc_usw_packet_test_cli_deinit(void);


#ifdef __cplusplus
}
#endif


