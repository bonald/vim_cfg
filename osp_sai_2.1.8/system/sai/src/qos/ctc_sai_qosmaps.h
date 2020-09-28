#if !defined (__SAI_QOSMAPS_H_)
#define __SAI_QOSMAPS_H_

#include <saiqosmaps.h>
#include <ctc_sai_common.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_vector.h>
#include <ctc_opf.h>

#define CTC_SAI_QOS_MAP_TC_NUMBER  8

typedef struct ctc_sai_qos_map_ctc_dir_domain_id_s
{
    uint32_t in_domain;
    uint32_t out_domain;
}ctc_sai_qos_map_ctc_dir_domain_id_t;

typedef struct ctc_sai_qos_tc_and_color_s
{
    uint8_t tc;
    uint8_t color;
}ctc_sai_qos_tc_and_color_t;

typedef struct ctc_sai_qos_domain_in_s
{
    ctc_sai_qos_tc_and_color_t dot1p_to_tc_color[8];
    ctc_sai_qos_tc_and_color_t exp_to_tc_color[8];
    ctc_sai_qos_tc_and_color_t dscp_to_tc_color[64];
}ctc_sai_qos_domain_in_t;

typedef struct ctc_sai_qos_domain_out_s
{
    uint32_t tc_and_color_to_dot1p[CTC_SAI_QOS_MAP_TC_NUMBER][3];
    uint32_t tc_and_color_to_exp[CTC_SAI_QOS_MAP_TC_NUMBER][3];
    uint32_t tc_and_color_to_dscp[CTC_SAI_QOS_MAP_TC_NUMBER][3];
}ctc_sai_qos_domain_out_t;

typedef struct ctc_sai_qos_domain_port_bind_map_id_s
{
    uint32_t dot1p_to_tc_color_map_id;
    uint32_t exp_to_tc_color_map_id;
    uint32_t dscp_to_tc_color_map_id;
    uint32_t tc_and_color_to_dot1p_map_id;
    uint32_t tc_and_color_to_exp_map_id;
    uint32_t tc_and_color_to_dscp_map_id;
}ctc_sai_qos_domain_port_bind_map_id_t;

typedef struct ctc_sai_qos_domain_id_s
{
    ctc_sai_qos_domain_port_bind_map_id_t domain_instance;
    uint32_t in_used_cnt;
    uint32_t out_used_cnt;
}ctc_sai_qos_domain_id_t;

typedef struct ctc_sai_qos_map_info_s
{
    ctc_vector_t*   pvector;
    uint32_t        counter_max;
    ctc_vector_t    *pcounter_vector;
    ctc_sai_qos_domain_in_t qos_domain_in_default;
    ctc_sai_qos_domain_out_t qos_domain_out_default;
    ctc_hash_t *ctc_sai_qos_map_hash;
}ctc_sai_qos_map_info_t;

typedef struct ctc_sai_qos_map_s
{
    uint32_t    qos_map_id;
    uint32_t    qos_map_type;  /*sai_qos_map_type_t*/
    ctc_vector_t*   qos_map_vector;
    uint32_t used_cnt;
}ctc_sai_qos_map_t;

typedef struct ctc_sai_qos_map_hash_s
{
    ctc_sai_qos_domain_port_bind_map_id_t ctc_sai_qos_map_port_index;
    ctc_sai_qos_map_ctc_dir_domain_id_t ctc_sai_qos_map_port_doamin;
    uint32 gport;
}ctc_sai_qos_map_hash_t;

sai_status_t ctc_sai_qosmaps_init();

#endif
