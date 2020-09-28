#ifndef __HSRV_SYNC_FDB_H__
#define __HSRV_SYNC_FDB_H__
#include "ctc_l2.h"
#include "glb_l2_define.h"
#include "hsrv_brgif.h"
#include "cfg_cmd.h"

#define HSRV_FDB_HW_DUMP_BUFFER_LEN  100
#define HSRV_FDB_LOOP_CACHE_SIZE    16
#define HSRV_FDB_LOOP_BUCKET_TIME    1
#define HSRV_FDB_LEARNING_EN_TIME    2

typedef enum
{
    HSRV_MAC_ADDR_TYPE_STATIC,
    HSRV_MAC_ADDR_TYPE_MULTICAST,
    HSRV_MAC_ADDR_TYPE_DYNAMIC,
    HSRV_MAC_ADDR_TYPE_UNICAST_ALL,
    HSRV_MAC_ADDR_TYPE_STATIC_PORT_SECURITY,
    HSRV_MAC_ADDR_TYPE_DYNAMIC_PORT_SECURITY,
    HSRV_MAC_ADDR_TYPE_ALL_PORT_SECURITY,
    HSRV_MAC_ADDR_TYPE_ALL
} hsrv_mac_addr_type_t;

typedef struct
{
    vid_t           vid;    /* key */
#ifdef FDB_SLIST
    ctclib_slist_t  *fdb_list;
#else
    fea_fdb_tailq_head_t fdb_queue;
#endif
} hsrv_fdb_vlan_t;

typedef struct hsrv_fdb_s
{    
    /*Added by xgu for hardware learning, dump hw fdb database need ifname, 2013-9-16*/
    /*buffer userd to dump hw fdb database*//*HAGT_BRG_HW_DUMP_BUFFER_LEN*/
    ctc_l2_addr_t* hw_dump_buffer;
    uint32 hw_dump_buffer_len;

    uint32 hsrv_learn_enable;
    uint32 hsrv_memchk_learn_disable;/*when the memory check is abnormal, set it 1*/

//    uint32 hsrv_fdb_size;  /* hash number + tcam number, stored in p_spec_glb->ucast_fdb */
    /* counter */
    int32 hsrv_valid_fdb_number;/*Because nexhop use the fdb hash,it will add invalid fdb*/
    int32 dynamic_security_entry_cnt;
    int32 static_security_entry_cnt;
    int32 static_fdb_cnt;
    int32 dynamic_fdb_cnt;
    
    sal_mutex_t *fdb_mutex;
    sal_mutex_t *mlag_sync_mutex;

    hsrv_fdb_vlan_t *vlan_array[GLB_MAX_VLAN_ID];
    glb_mlag_fdb_sync_t mlag_sync_cache;
} hsrv_fdb_t;

struct hsrv_fdb_loop_cache_s
{
    uint32  count;
    uint32  index;
    uint16  port_array[HSRV_FDB_LOOP_CACHE_SIZE];
};
typedef struct hsrv_fdb_loop_cache_s hsrv_fdb_loop_cache_t;

struct hsrv_fdb_loop_s
{
    uint32  cache_err_threshold;
    hsrv_fdb_loop_cache_t cache;
};
typedef struct hsrv_fdb_loop_s hsrv_fdb_loop_t;

int32
hsrv_fdb_sai_add_fdb(tbl_fea_fdb_t *p_fdb, tbl_fea_brg_if_t* p_if);

int32
hsrv_fdb_sai_del_fdb(tbl_fea_fdb_t *p_db_fdb);

int32
hsrv_security_op_security(uint32 ifindex, ds_brgif_t  *p_brgif);

int32
hsrv_vlan_security_op_security(vid_t vid);

/* FDB DB APIs */
int32
hsrv_vlan_security_check_security_mac_num(vid_t vid, uint32 add_entry, uint32 *p_allow_add);

int32
hsrv_security_check_security_mac_num(tbl_fea_brg_if_t *p_db_if, uint32 add_entry, uint32 is_static, uint32 *p_allow_add);

int32
hsrv_fdb_db_add_fdb(tbl_fea_fdb_t *p_fdb, tbl_fea_brg_if_t *p_db_if);

int32
hsrv_fdb_db_update_fdb(tbl_fea_fdb_t *p_fdb, tbl_fea_fdb_t *p_db_fdb);

int32
hsrv_fdb_db_del_fdb(tbl_fea_fdb_t *p_db_fdb);

int32
hsrv_fdb_del_fdb_one_by_flush(tbl_fea_fdb_t *p_fdb);

int32
hsrv_fdb_vlan_add(vid_t vid);

int32
hsrv_fdb_vlan_del(vid_t vid);

tbl_fea_fdb_t*
hsrv_fdb_lookup_fdb(tbl_fea_fdb_t *p_fdb);

int32
hsrv_fdb_hw_get_count_number(uint32* static_count, uint32* dynamic_count, uint32* total_count);

int32
hsrv_fdb_add_check_full(tbl_fea_fdb_t *p_fdb);

int32
hsrv_fdb_db_flush_mlag_mi();

int32
hsrv_fdb_db_flush_all();

/* modified by liwh for bug 53553, 2019-10-23 */
int32
//hsrv_fdb_db_flush_vlan(vid_t vid);
hsrv_fdb_db_flush_vlan(vid_t vid, uint32 *p_has_entry);
/* liwh end */

/* modified by liwh for bug 53553, 2019-10-23 */
int32
//hsrv_fdb_db_flush_port(uint32 ifindex, uint32 mlag_sync);
hsrv_fdb_db_flush_port(uint32 ifindex, uint32 mlag_sync, uint32 *p_has_entry);
/* liwh end */

int32
hsrv_fdb_db_flush_by_port(uint32 ifindex);

/* modified by liwh for bug 53553, 2019-10-23 */
int32
//hsrv_fdb_db_flush_port_vlan(uint32 ifindex, vid_t vid);
hsrv_fdb_db_flush_port_vlan(uint32 ifindex, vid_t vid, uint32 *p_has_entry);
/* liwh end */

/* modified by liwh for bug 53553, 2019-10-23 */
int32
//hsrv_fdb_db_flush_mac(uint8 *mac);
hsrv_fdb_db_flush_mac(uint8 *mac, uint32 *p_has_entry);
/* liwh end */

int32
hsrv_fdb_sw_get_count(cfg_cmd_para_t *para);

int32
hsrv_fdb_sw_get_add_fdb_fail(hsrv_mac_addr_type_t type, cfg_cmd_para_t *para);

/* MLAG FDB APIs */
int32
hsrv_fdb_mlag_tx_fdb(tbl_fea_fdb_t *p_db_fdb, glb_mlag_fdb_sync_act_t act);

int32
hsrv_fdb_show_one(tbl_fea_fdb_t *p_db_fdb, hsrv_mac_addr_type_t type, FILE *fp);

int32
hsrv_fdb_sw_show_banner(FILE *fp);

int32
hsrv_fdb_sw_get_all(hsrv_mac_addr_type_t type, cfg_cmd_para_t *para);

int32
hsrv_fdb_sw_get_interface(int32 ifindex, hsrv_mac_addr_type_t type, cfg_cmd_para_t *para);

int32
hsrv_fdb_sw_get_errdisable_fdb_loop(cfg_cmd_para_t *para);

int32
hsrv_fdb_sw_get_mac(uint8 *mac, hsrv_mac_addr_type_t type, cfg_cmd_para_t *para);

int32
hsrv_fdb_sw_get_vlan(vid_t vid, hsrv_mac_addr_type_t type, cfg_cmd_para_t *para);


/* security FDB APIs */

int32
hsrv_fdb_mlag_tx_flush_all();

int32
hsrv_fdb_mlag_tx_flush_vlan(vid_t vid);

int32
hsrv_fdb_mlag_tx_flush_port(uint32 ifindex);

int32
hsrv_fdb_mlag_tx_flush_port_vlan(uint32 ifindex, vid_t vid);

int32
hsrv_fdb_mlag_tx_flush_mac(uint8 *mac);

int32
hsrv_fdb_mlag_peer_conf_cb(uint32 ifindex, uint32 enable);

int32
hsrv_fdb_mlag_set_protect_en(uint32 mlag_id, uint32 protect_en);

int32
hsrv_fdb_mlag_flush_peer_link_fdb(uint32 ifindex);

int32
hsrv_fdb_mlag_sync_flush_peer_link(uint32 ifindex);

int32
hsrv_fdb_mlag_peer_link_destory_cb(uint32 ifindex);

/* FDB APIs */
int32
hsrv_fdb_learn_arp_fdb(uint16 vid, uint32 ifindex, uint8 *mac);

int32
hsrv_fdb_sync_add_fdb(tbl_fdb_t *p_pm_fdb);

int32
hsrv_fdb_sync_del_fdb(tbl_fdb_t *p_pm_fdb);

int32
hsrv_fdb_get_count_number(uint32 *static_count, uint32 *static_security_count, uint32 *dynamic_count, uint32 *dynamic_security_count, uint32 *total_count);

int32
hsrv_fdb_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

/* FDB init APIs */
int32
hsrv_fdb_db_init();

int32
hsrv_fdb_hw_init();

/*fdb_loop*/
int32
hsrv_fdb_set_fdb_loop(tbl_brg_global_t* p_brg_global);

int32
hsrv_fdb_set_fdb_loop_detect_en(tbl_brg_global_t* p_brg_global);

int32
_hsrv_fdb_del_fdb(tbl_fea_fdb_t *p_fdb, uint32 is_static, uint32 need_sync);

int32
hsrv_fdb_start();

/* modified by liwh for bug 53553, 2019-10-23 */
void hsrv_fdb_lock();

void hsrv_fdb_unlock();
/* liwh end */

#endif /* !__HSRV_SYNC_FDB_H__ */
