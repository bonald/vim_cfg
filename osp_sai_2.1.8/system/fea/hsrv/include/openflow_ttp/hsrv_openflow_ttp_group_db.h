#ifndef __HSRV_OPENFLOW_TTP_GROUP_DB_H__
#define __HSRV_OPENFLOW_TTP_GROUP_DB_H__

#define HSRV_MIN_BUCKET_INDEX 1
#define HSRV_MAX_BCUKET_INDEX 0xffffffff

/**
 * @brief Fea layer groupdb master
 */
struct hsrv_openflow_ttp_group_master_s
{
    ctclib_hash_t* p_group_hash;         /**< hsrv_openflow_ttp_group_t */
    ctclib_hash_t* p_group_bucket;         /**< hsrv_openflow_ttp_group_bucket_entry_t */
    ctclib_opf_t* pst_group_bucket_index_opf;
};
typedef struct hsrv_openflow_ttp_group_master_s hsrv_openflow_ttp_group_master_t;

extern hsrv_openflow_ttp_group_master_t *g_pst_openflow_ttp_group_master;

#define HSRV_OPENFLOW_TTP_GROUP_HASH           g_pst_openflow_ttp_group_master->p_group_hash
#define HSRV_OPENFLOW_TTP_GROUP_BUCKET_HASH           g_pst_openflow_ttp_group_master->p_group_bucket



int32
hsrv_openflow_groupdb_add_group(hsrv_openflow_ttp_group_t* p_group);

int32
hsrv_openflow_groupdb_del_group(uint32 group_id);

int32
hsrv_openflow_groupdb_lookup_group(uint32 group_id, hsrv_openflow_ttp_group_t** pp_group);

int32
hsrv_openflow_groupdb_add_group_bucket(hsrv_openflow_ttp_group_bucket_entry_t* p_group_bucket);

int32
hsrv_openflow_groupdb_del_group_bucket(uint32 bucket_index);

int32
hsrv_openflow_groupdb_lookup_group_bucket(uint32 bucket_index, hsrv_openflow_ttp_group_bucket_entry_t** pp_group_bucket);

int32
hsrv_openflow_groupdb_alloc_bucket_index(uint32 *bucket_index);

int32
hsrv_openflow_groupdb_release_bucket_index(uint32 bucket_index);

int32
hsrv_openflow_groupdb_start(void);

#endif /* !__HSRV_OPENFLOW_GROUP_PRIV_H__ */
