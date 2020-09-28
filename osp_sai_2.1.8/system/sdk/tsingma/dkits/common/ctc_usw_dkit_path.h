#ifndef _CTC_GOLDENGATE_DKIT_PATH_H_
#define _CTC_GOLDENGATE_DKIT_PATH_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "sal.h"

struct ctc_dkit_path_stats_s
{
    uint8 ipe_stats[2];
    uint8 epe_stats[2];
    uint8 bsr_stats;
    uint8 rsv;
};
typedef struct ctc_dkit_path_stats_s ctc_dkit_path_stats_t;

extern int32
ctc_usw_dkit_path_process(void* p_para);


#ifdef __cplusplus
}
#endif

#endif

