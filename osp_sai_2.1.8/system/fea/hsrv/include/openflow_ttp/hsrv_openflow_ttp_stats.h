#ifndef __HSRV_OPENFLOW_TTP_STATS_H__
#define __HSRV_OPENFLOW_TTP_STATS_H__

#define HSRV_OPENFLOW_STATS_MALLOC(size) HSRV_MALLOC(MEM_FEA_HSRV_OPENFLOW_STATS, size)
#define HSRV_OPENFLOW_STATS_FREE(ptr) \
do { \
    HSRV_FREE(MEM_FEA_HSRV_OPENFLOW_STATS, ptr);  \
    ptr = NULL; \
}while(0)

#define HSRV_OPENFLOW_TTP_STATS_POLLING_INTERVAL 2

struct hsrv_openflow_stats_info_s
{
    glb_stats_type_t type;
    uint32 stats_id;
    uint32 ctc_stats_id;    /* policer doesn't use this field, policer get stats by policer_id */
    glb_stats_result_t stats_rslt;
};
typedef struct hsrv_openflow_stats_info_s hsrv_openflow_ttp_stats_info_t;

int32 
hsrv_openflow_add_stats(glb_stats_type_t resource_type, uint32 resource_id, uint32 group_or_table_id, uint32 *stats_id);

int32
hsrv_openflow_del_stats(uint32 stats_id);

int32
hsrv_openflow_clear_stats(uint32 stats_id);

hsrv_openflow_ttp_stats_info_t*
hsrv_openflow_get_stats_info(uint32 stats_id);

int32
hsrv_openflow_stats_start(void);

#endif
