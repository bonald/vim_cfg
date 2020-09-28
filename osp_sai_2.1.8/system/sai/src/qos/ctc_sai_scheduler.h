#if !defined (__SAI_SCHEDULER_H_)
#define __SAI_SCHEDULER_H_

#include <saischeduler.h>
#include <saipolicer.h>
#include <ctc_sai_common.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_vector.h>
#include <ctc_opf.h>
#ifdef GREATBELT
    #define SAI_QOS_DEF_SHAPING_PIR 10000000
#else
    #define SAI_QOS_DEF_SHAPING_PIR 100000000
#endif
typedef struct ctc_sai_scheduler_info_s
{
    ctc_vector_t*   pvector;
    uint32_t        counter_max;
    ctc_vector_t    *pcounter_vector;
    ctc_hash_t *ctc_sai_scheduler_port_index_hash;
}ctc_sai_scheduler_info_t;

typedef struct ctc_sai_scheduler_s
{
    uint32_t          scheduler_id;
    ctc_vector_t*   scheduler_vector;
    uint32_t          used_cnt;
}ctc_sai_scheduler_t;

typedef struct ctc_scheduler_s
{
    int32_t       scheduler_mode;
    int32_t      scheduler_weight;
    uint64_t     shape_cir;
    uint64_t     shape_cbs;
    uint64_t     shape_pir;
    uint64_t     shape_pbs;
}ctc_scheduler_t;

typedef struct ctc_sai_scheduler_port_index_s
{
    uint32_t          ctc_sai_scheduler_port_index;
    uint32_t          ctc_sai_scheduler_queue_dwrr_mode[CTC_SAI_PORT_QUEUE_NUM];
    uint32_t          gport; 
}ctc_sai_scheduler_port_index_t;



sai_status_t ctc_sai_scheduler_init();

sai_status_t 
ctc_sai_scheduler_copp_set_scheduler(sai_object_id_t queue_id,
                                const sai_attribute_t *attr);
sai_status_t 
ctc_sai_scheduler_copp_get_scheduler(sai_object_id_t queue_id,
                                sai_attribute_t *attr);

#endif
