/**
 @file ctc_stress_test_cli.c

 @date oam17-03-21

 @version v1.0

 The file applies cli to test sdk module of SDK
*/

#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_stress_test_cli.h"

int32
ctc_cli_stress_test_output(ctc_cli_stress_output_t* output)
{
    sal_systime_t* tv1;
    sal_systime_t* tv2;
    char* op[CTC_CLI_STRESS_OP_TYPE_MAX] = {"Add", "Remove", "Update"};


    tv1 = &(output->tv_start);
    tv2 = &(output->tv_end);

    ctc_cli_out("%s, OP:%s\n", output->buf, op[output->op]);
    ctc_cli_out("START TIME:[%10u s:%3u ms]\n", tv1->tv_sec, tv1->tv_usec / 1000);
    ctc_cli_out("END   TIME:[%10u s:%3u ms]\n", tv2->tv_sec, tv2->tv_usec / 1000);
    if (tv2->tv_usec >= tv1->tv_usec)
    {
        ctc_cli_out("USED  TIME:[%10u s:%3u ms]\n", tv2->tv_sec - tv1->tv_sec, (tv2->tv_usec - tv1->tv_usec) / 1000);
    }
    else
    {
        ctc_cli_out("USED  TIME:[%10u s:%3u ms]\n", tv2->tv_sec - tv1->tv_sec - 1, (tv2->tv_usec + 1000000 - tv1->tv_usec) / 1000);
    }

    return 0;
}



int32
ctc_stress_test_cli_init(void)
{
    ctc_stress_test_l2_cli_init();
    ctc_stress_test_l3_cli_init();
    ctc_stress_test_oam_cli_init();
    ctc_stress_test_flow_cli_init();
    ctc_stress_test_interface_cli_init();
    ctc_stress_test_tunnel_cli_init();
    ctc_stress_test_nexthop_cli_init();
    ctc_stress_test_app_cli_init();
    ctc_stress_test_misc_cli_init();

   return CLI_SUCCESS;
}


