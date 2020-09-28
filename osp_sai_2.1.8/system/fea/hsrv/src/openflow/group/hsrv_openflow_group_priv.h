#ifndef __HSRV_OPENFLOW_GROUP_PRIV_H__
#define __HSRV_OPENFLOW_GROUP_PRIV_H__


/**
 * @brief Fea layer groupdb watch port lookup
 */
struct hsrv_openflow_group_watch_port_lookup_s
{
    uint32 ifindex;
    uint32 link_up;

    uint32 group_count;
    uint32 groups[GLB_FF_NUM_GROUP + GLB_SELECT_NUM_GROUP];
};
typedef struct hsrv_openflow_group_watch_port_lookup_s hsrv_openflow_group_watch_port_lookup_t;

/**
 * @brief Fea layer groupdb master
 */
struct hsrv_openflow_group_master_s
{
    ctclib_hash_t* p_group_hash;         /**< hsrv_openflow_group_t */
    uint8 drop_pkt_to_ingress_port;      /**< Added by weizj for bug 35135 */
};
typedef struct hsrv_openflow_group_master_s hsrv_openflow_group_master_t;

extern hsrv_openflow_group_master_t *g_pst_openflow_group_master;

#define HSRV_OPENFLOW_GROUP_HASH           g_pst_openflow_group_master->p_group_hash
/* Added by weizj for bug 35135 */
#define HSRV_OPENFLOW_GROUP_DROP_INGRESS   g_pst_openflow_group_master->drop_pkt_to_ingress_port


/**
 * @brief Fea layer groupdb add group
 */
int32
hsrv_openflow_groupdb_add_group(hsrv_openflow_group_t* p_group);

/**
 * @brief Fea layer groupdb delete group
 */
int32
hsrv_openflow_groupdb_del_group(hsrv_openflow_group_t* p_group);

/**
 * @brief Fea layer groupdb loop group watch port
 */
int32
hsrv_openflow_groupdb_loop_watch_port(hsrv_openflow_group_watch_port_lookup_t* p_data);

int32
hsrv_openflow_groupdb_loop_update_ffgroup();

/**
 * @brief Fea layer groupdb set drop ingress, Added by weizj for bug 35135
 */
int32
hsrv_openflow_groupdb_set_drop_ingress(uint8 is_drop);

/**
 * @brief Fea layer groupdb get drop ingress, Added by weizj for bug 35135
 */
bool
hsrv_openflow_groupdb_get_drop_ingress(void);

/**
 * @brief Fea layer groupdb start
 */
int32
hsrv_openflow_groupdb_start(void);

#endif /* !__HSRV_OPENFLOW_GROUP_PRIV_H__ */
