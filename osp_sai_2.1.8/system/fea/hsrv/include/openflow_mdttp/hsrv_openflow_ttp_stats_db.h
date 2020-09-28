#ifndef __HSRV_OPENFLOW_TTP_stats_DB_H__
#define __HSRV_OPENFLOW_TTP_stats_DB_H__

struct hsrv_openflow_ttp_stats_master_s
{
    ctclib_hash_t* p_stats_hash;         /**< hsrv_openflow_ttp_stats_info_t */
    ctclib_opf_t* pst_stats_id;
    sal_mutex_t* p_stats_mutex;
};
typedef struct hsrv_openflow_ttp_stats_master_s hsrv_openflow_ttp_stats_master_t;

extern hsrv_openflow_ttp_stats_master_t *g_pst_openflow_ttp_stats_master;

#define HSRV_OPENFLOW_TTP_STATS_HASH           g_pst_openflow_ttp_stats_master->p_stats_hash

#define HSRV_OPF_STATS_ID_START 1
#define HSRV_OPF_STATS_ID_NUM   3000


int32
hsrv_openflow_statsdb_add_stats(hsrv_openflow_ttp_stats_info_t* p_stats);

int32
hsrv_openflow_statsdb_del_stats(uint32 stats_id);

int32
hsrv_openflow_statsdb_lookup_stats(uint32 stats_id, hsrv_openflow_ttp_stats_info_t** pp_stats);

int32
hsrv_openflow_statsdb_alloc_stats_id(uint32 *stats_id);

int32
hsrv_openflow_statsdb_release_stats_id(uint32 stats_id);

int32
hsrv_openflow_statsdb_start(void);

#endif
