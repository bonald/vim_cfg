/**
 @file ctc_stress_test_cli.h

 @date 2017-03-21

 @version v1.0

*/

#ifndef _CTC_STRESS_TEST_CLI_H
#define _CTC_STRESS_TEST_CLI_H

#include "sal.h"
#include "ctc_cli.h"

#ifdef __cplusplus
extern "C" {
#endif




enum ctc_cli_stress_op_type_e
{
    CTC_CLI_STRESS_OP_TYPE_ADD,
    CTC_CLI_STRESS_OP_TYPE_REMOVE,
    CTC_CLI_STRESS_OP_TYPE_UPDATE,
    CTC_CLI_STRESS_OP_TYPE_MAX
};
typedef enum ctc_cli_stress_op_type_e ctc_cli_stress_op_type_t;

enum ctc_cli_stress_test_type_e
{
    CTC_CLI_STRESS_TEST_TYPE_SPEC,            /* Spec test */
    CTC_CLI_STRESS_TEST_TYPE_STRESS,        /* Stress test */
    CTC_CLI_STRESS_TEST_TYPE_PERF,            /* Performance test */
    CTC_CLI_STRESS_TEST_TYPE_MAX
};
typedef enum ctc_cli_stress_test_type_e ctc_cli_stress_test_type_t;


enum ctc_cli_stress_data_mode_e
{
    CTC_CLI_STRESS_DATA_MODE_INCREASE,
    CTC_CLI_STRESS_DATA_MODE_RANDOM,
    CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL,
    CTC_CLI_STRESS_DATA_MODE_MAX
};
typedef enum ctc_cli_stress_data_mode_e ctc_cli_stress_data_mode_t;

enum ctc_cli_stress_priority_mode_e
{
    CTC_CLI_STRESS_PRIORITY_MODE_FIXED,
    CTC_CLI_STRESS_PRIORITY_MODE_INCREASE,
    CTC_CLI_STRESS_PRIORITY_MODE_DECREASE,
    CTC_CLI_STRESS_PRIORITY_MODE_RANDOM,
    CTC_CLI_STRESS_PRIORITY_MODE_MAX
};
typedef enum ctc_cli_stress_priority_mode_e ctc_cli_stress_priority_mode_t;

struct ctc_cli_stress_output_s
{
    char* buf;
    ctc_cli_stress_op_type_t op;
    sal_systime_t  tv_start;
    sal_systime_t  tv_end;
};
typedef struct ctc_cli_stress_output_s ctc_cli_stress_output_t;

struct ctc_cli_stress_s
{
    uint8 op_type;
    uint8 test_type;
    uint8 data_mode;
    uint8 priority_mode;   /* used for scl/acl tcam entry priority mode refer to ctc_cli_stress_priority_mode_t */
};
typedef struct ctc_cli_stress_s ctc_cli_stress_t;

#define CTC_CLI_STRESS_CHECK_RETURN(ret, type, count)                                    \
    {                                                                      \
        if ((ret < 0) && (CTC_CLI_STRESS_TEST_TYPE_STRESS != type)) {                                                     \
            if((CTC_CLI_STRESS_TEST_TYPE_SPEC == type) && ((ret == CTC_E_NO_RESOURCE) || (ret == CTC_E_PROFILE_NO_RESOURCE)     \
                || (ret == CTC_E_NO_MEMORY))) {                   \
            ctc_cli_out("Operate valid count: %d,  ret: %d, %s\n", count, ret, ctc_get_error_desc(ret));  \
            return CLI_ERROR; }}                                            \
    }


/*internal api*/
/*fdb/vlan/stp,etc*/
extern int32
ctc_stress_test_l2_cli_init(void);
/*ipuc/ipmc/ip tunnel/l3if*/
extern int32
ctc_stress_test_l3_cli_init(void);
/*oam & aps*/
extern int32
ctc_stress_test_oam_cli_init(void);
/*acl/scl*/
extern int32
ctc_stress_test_flow_cli_init(void);

/*port/lag/l3if/bpe/internal port*/
extern int32
ctc_stress_test_interface_cli_init(void);
/*overlay /fcoe/tunnel/wlan/mpls*/
extern int32
ctc_stress_test_tunnel_cli_init(void);
/*nexthop*/
extern int32
ctc_stress_test_nexthop_cli_init(void);
/*app index/xgpon*/
extern int32
ctc_stress_test_app_cli_init(void);
/*other module*/
extern int32
ctc_stress_test_misc_cli_init(void);
extern int32
ctc_cli_stress_test_output(ctc_cli_stress_output_t* output);

/*api*/
extern int32
ctc_stress_test_cli_init(void);


#ifdef __cplusplus
}
#endif

#endif

