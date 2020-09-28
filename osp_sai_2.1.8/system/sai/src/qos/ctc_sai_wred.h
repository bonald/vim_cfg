#if !defined (__SAI_WRED_H_)
#define __SAI_WRED_H_

#include <saiwred.h>
#include <ctc_sai_common.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_vector.h>
#include <ctc_opf.h>

#define GOLDENGATE_DEFAULT_WRED_PROBABILITY 19
#define SAI_QOS_DROP_DEF_PRO    19
#define SAI_QOS_DROP_DEF_THRESH     600
#define USW_DEFAULT_WRED_DROP_MAX_TH_3_CRITIC_VALUE 64

typedef struct ctc_sai_wred_info_s
{
    ctc_vector_t*   pvector;
    uint32_t        counter_max;
    ctc_vector_t    *pcounter_vector;
}ctc_sai_wred_info_t;

typedef struct ctc_sai_wred_s
{
    uint32_t    wred_id;
    ctc_vector_t*   wred_vector;
    uint32 used_cnt;
}ctc_sai_wred_t;

sai_status_t ctc_sai_wred_init();

#endif
