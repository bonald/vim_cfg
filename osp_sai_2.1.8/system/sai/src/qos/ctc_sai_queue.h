#if !defined (__SAI_QUEUE_H_)
#define __SAI_QUEUE_H_

#include <saiqueue.h>
#include <ctc_sai_common.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_vector.h>
#include <ctc_opf.h>


typedef struct ctc_sai_queue_entry_s
{
    uint32_t wred_id;
    uint32_t wred_type;
    uint32_t scheduler_id;
    uint32_t buffer_id;
}ctc_sai_queue_entry_t;

typedef struct ctc_sai_queue_info_s
{
    ctc_hash_t *ctc_sai_queue_info_hash;
}ctc_sai_queue_info_t;

typedef struct ctc_sai_queue_hash_s
{
    ctc_sai_queue_entry_t queue_entries[CTC_SAI_PORT_QUEUE_NUM];
    uint32_t          gport; 
}ctc_sai_queue_hash_t;

sai_status_t ctc_sai_queue_init();
#endif
