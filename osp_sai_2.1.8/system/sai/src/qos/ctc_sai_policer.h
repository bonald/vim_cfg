#if !defined (__SAI_POLICER_H_)
#define __SAI_SPOLICER_H_

#include <saipolicer.h>
#include <ctc_sai_common.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_vector.h>
#include <ctc_opf.h>

typedef struct ctc_sai_policer_info_s
{
    ctc_vector_t*   pvector;
    uint32_t        counter_max;
    ctc_vector_t    *pcounter_vector;
    ctc_hash_t *ctc_sai_policer_port_index_hash;
    ctc_hash_t *ctc_sai_egress_policer_port_index_hash;
    ctc_hash_t *ctc_sai_policer_is_in;
}ctc_sai_policer_info_t;

typedef struct ctc_sai_policer_s
{
    uint32_t          policer_id;
    ctc_vector_t*   policer_vector;
    uint32_t          used_port_id;
    uint32_t          used_entry_id;
}ctc_sai_policer_t;

typedef struct ctc_sai_policer_port_index_s
{
    uint32_t          ctc_sai_policer_port_index;
    uint32_t          gport; 
}ctc_sai_policer_port_index_t;

typedef struct ctc_sai_egress_policer_port_index_s
{
    uint32_t          ctc_sai_egress_policer_port_index;
    uint32_t          gport; 
}ctc_sai_egress_policer_port_index_t;


sai_status_t ctc_sai_policer_init();

#endif
