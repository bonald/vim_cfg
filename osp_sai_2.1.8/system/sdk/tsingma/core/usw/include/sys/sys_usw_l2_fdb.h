/**
   @file sys_usw_l2_fdb.h

   @date 2013-07-19

   @version v2.0

   The file defines Macro, stored data structure for Normal FDB module
 */
#ifndef _SYS_USW_L2_FDB_H_
#define _SYS_USW_L2_FDB_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_const.h"
#include "ctc_hash.h"
#include "ctc_vector.h"
#include "ctc_spool.h"
#include "ctc_l2.h"

#define REGULAR_INVALID_FID(fid)    ((fid == 0)||(!pl2_usw_master[lchip] || ((fid) > pl2_usw_master[lchip]->cfg_max_fid)))

#define SYS_L2_FID_CHECK(fid)         \
    if (REGULAR_INVALID_FID(fid))     \
    {                                 \
        return CTC_E_BADID; \
    }

#define SYS_L2_MAX_BLACK_HOLE_ENTRY  128

typedef struct
{
    uint32        flag;      /**< ctc_l2_flag_t  CTC_L2_xxx flags*/
    uint32        key_index; /**< mac key index */
    uint32        ad_index;  /**< mac ad index */
    uint8         pending;
    uint8         rsv[3];
}sys_l2_detail_t;

enum sys_l2_station_move_op_type_e
{
    SYS_L2_STATION_MOVE_OP_PRIORITY,
    SYS_L2_STATION_MOVE_OP_ACTION,
    MAX_SYS_L2_STATION_MOVE_OP
};
typedef enum sys_l2_station_move_op_type_e sys_l2_station_move_op_type_t;

struct sys_l2_master_s
{
    uint32      l2mc_count;          /**< multicast entry count */
    uint32      def_count;           /**< default entry count */
    uint32      ad_index_drop;       /**< all Ad share it if drop */
    uint32      dft_entry_base;      /**< index base of default entry */

    /*** soft tabls to store fdb node ***/
    ctc_hash_t*   fdb_hash;            /**< node is sys_l2_node_t, key is mac+fid*/
    ctc_spool_t * ad_spool;            /**< ds mac spool */
    ctc_vector_t* fid_vec;             /**< node is sys_l2_fid_node_t, key is fid or fid+port */
    ctc_vector_t* vport2nh_vec;        /**< node is sys_l2_vport_2_nhid_t, index is logic port */

    sal_mutex_t * l2_mutex;            /**< mutex for fdb */
    sal_mutex_t * l2_dump_mutex;       /**< mutex for dump fdb */
    uint32      cfg_vport_num;      /**< num of logic port */
    uint8       cfg_hw_learn;       /**< support hw learning. but not force all do hw learning. */
    uint8       static_fdb_limit;
    uint16      cfg_max_fid;        /**< CTC_MAX_FID_ID, 16383=0x3fff*/
    uint32      mac_fdb_bmp[SYS_L2_MAX_BLACK_HOLE_ENTRY/32];          /*bitmap for mac based fdb, dont't care fid*/
    uint8       trie_sort_en;
    uint8       vp_alloc_ad_en;   /**< if set, indicate logic port alloc ad_index by opf when bind nexthop,only supported in SW learning mode*/
    uint16      rchip_ad_rsv[SYS_USW_MAX_GCHIP_CHIP_ID];

    int32       (*sync_add_db)(uint8 lchip, ctc_l2_addr_t* l2_addr, uint32 nhid, uint32 index);
    int32       (*sync_remove_db)(uint8 lchip, ctc_l2_addr_t* l2_addr);
};
typedef struct sys_l2_master_s   sys_l2_master_t;

extern sys_l2_master_t* pl2_usw_master[CTC_MAX_LOCAL_CHIP_NUM];

/**********************************************************************************
                      Define API function interfaces
 ***********************************************************************************/

extern int32
sys_usw_l2_fdb_init(uint8 lchip, void* l2_fdb_global_cfg);

int32
sys_usw_l2_set_default_entry_features(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr);

int32
sys_usw_l2_get_default_entry_features(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr);

extern int32
sys_usw_l2_get_fdb_count(uint8 lchip, ctc_l2_fdb_query_t* pQuery);

extern int32
sys_usw_l2_get_fdb_entry(uint8 lchip, ctc_l2_fdb_query_t* pQuery,
                                ctc_l2_fdb_query_rst_t* query_rst);

extern int32
sys_usw_l2_add_fdb(uint8 lchip, ctc_l2_addr_t* l2_addr, uint8 with_nh, uint32 nhid);

extern int32
sys_usw_l2_remove_fdb(uint8 lchip, ctc_l2_addr_t* l2_addr);

extern int32
sys_usw_l2_remove_fdb_by_index(uint8 lchip, uint32 index);

extern int32
sys_usw_l2_get_fdb_by_index(uint8 lchip, uint32 index, ctc_l2_addr_t* l2_addr, sys_l2_detail_t* pi);

extern int32
sys_usw_l2_flush_fdb(uint8 lchip, ctc_l2_flush_fdb_t* pFlush);

extern int32
sys_usw_l2_add_default_entry(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr);

extern int32
sys_usw_l2_remove_default_entry(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr);

extern int32
sys_usw_l2_add_port_to_default_entry(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr);

extern int32
sys_usw_l2_remove_port_from_default_entry(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr);

extern int32
sys_usw_l2mcast_add_addr(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr);

extern int32
sys_usw_l2mcast_remove_addr(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr);

extern int32
sys_usw_l2mcast_add_member(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr);

extern int32
sys_usw_l2mcast_remove_member(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr);

extern int32
sys_usw_l2_show_status(uint8 lchip);

extern int32
sys_usw_l2_set_nhid_by_logic_port(uint8 lchip, uint16 logic_port, uint32 nhp_id);

extern int32
sys_usw_l2_get_nhid_by_logic_port(uint8 lchip, uint16 logic_port, uint32 *nhp_id);

 /*extern int32*/
 /*sys_usw_l2_sync_hw_info(uint8 lchip, void* pf);*/

extern int32
sys_usw_l2_set_dsmac_for_linkagg_hw_learning(uint8 lchip, uint32 gport, bool b_add);

extern int32
sys_usw_l2_set_dsmac_for_bpe(uint8 lchip, uint32 gport, bool b_add);

extern int32
sys_usw_l2_set_fid_property(uint8 lchip, uint16 fid_id, uint32 property, uint32 value);

extern int32
sys_usw_l2_get_fid_property(uint8 lchip, uint16 fid_id, uint32 property, uint32* value);

extern int32
sys_usw_l2_set_default_entry_features(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr);

extern int32
sys_usw_l2_reserve_rchip_ad_index(uint8 lchip, uint8 gchip, uint16 max_port_num);

extern int32
sys_usw_l2_remove_pending_fdb(uint8 lchip, ctc_l2_addr_t* l2_addr, uint32 ad_idx);

extern int32
sys_usw_l2_set_station_move(uint8 lchip, uint32 gport, uint8 type, uint32 value);

extern int32
sys_usw_l2_get_station_move(uint8 lchip, uint32 gport, uint8 type, uint32* value);

extern bool
sys_usw_l2_fdb_trie_sort_en(uint8 lchip);

extern int32
sys_usw_l2_default_entry_set_mac_auth(uint8 lchip, uint16 fid, bool enable);

extern int32
sys_usw_l2_default_entry_get_mac_auth(uint8 lchip, uint16 fid, bool* enable);

extern int32
sys_usw_l2_fdb_set_entry_hit(uint8 lchip, ctc_l2_addr_t* l2_addr, uint8 hit);

extern int32
sys_usw_l2_fdb_get_entry_hit(uint8 lchip, ctc_l2_addr_t* l2_addr, uint8* hit);

extern int32
sys_usw_l2_fdb_deinit(uint8 lchip);

extern int32
sys_usw_l2_replace_fdb(uint8 lchip, ctc_l2_replace_t* p_replace);

#ifdef __cplusplus
}
#endif
#endif /*end of _SYS_USW_L2_FDB_H_*/

