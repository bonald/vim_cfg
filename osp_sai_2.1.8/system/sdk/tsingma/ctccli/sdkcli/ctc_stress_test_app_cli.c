/**
 @file ctc_stress_test_cli.c

 @date 2017-03-21

 @version v1.0

 The file applies cli to test sdk module of SDK
*/

#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_stress_test_cli.h"

struct ctc_cli_stress_test_app_s
{
 
};
typedef struct ctc_cli_stress_test_app_s ctc_cli_stress_test_app_t;

ctc_cli_stress_test_app_t  *g_ctc_stress_app;

int32
ctc_stress_test_app_cli_init(void)
{
    g_ctc_stress_app = (ctc_cli_stress_test_app_t*)sal_malloc(sizeof(ctc_cli_stress_test_app_t));   
    if(!g_ctc_stress_app)
    {
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}


