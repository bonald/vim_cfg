#if (FEATURE_MODE == 0)
/**
 @file ctc_ptp_cli.c

 @date 2010-06-09

 @version v2.0

 This file defines functions for ptp cli module

*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_ptp_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"

/**
 @brief  Initialize sdk oam module CLIs
*/
int32
ctc_usw_ptp_cli_init(void)
{
    /* ptp cli under sdk mode */

    return CLI_SUCCESS;
}

#endif
