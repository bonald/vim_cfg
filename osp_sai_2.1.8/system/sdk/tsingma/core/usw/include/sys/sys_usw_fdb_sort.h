
#ifndef _CTC_L2_FDB_H
#define _CTC_L2_FDB_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_shash.h"
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_l2.h"

typedef int32 (* sys_fdb_sort_get_entry_cb_t)(uint8 lchip, ctc_l2_fdb_query_t* pq, ctc_l2_fdb_query_rst_t* pr);
typedef bool (* sys_fdb_sort_get_trie_sort_cb_t)(uint8 lchip);
#if 0
typedef bool  (* sys_fdb_sort_mac_limit_cb_t)(uint8 lchip, uint32* port_cnt, uint32* vlan_cnt, uint32 system_cnt,
                                              ctc_l2_addr_t* l2_addr, uint8 learn_limit_remove_fdb_en, uint8 is_update);
#endif

extern int32
sys_usw_fdb_sort_init(uint8 lchip);

extern int32
sys_usw_fdb_sort_deinit(uint8 lchip);

extern int32
sys_usw_fdb_sort_get_fdb_entry(uint8 lchip, ctc_l2_fdb_query_t* pQuery, ctc_l2_fdb_query_rst_t* query_rst);

int32
sys_usw_fdb_sort_register_fdb_entry_cb(uint8 lchip, sys_fdb_sort_get_entry_cb_t cb);

int32
sys_usw_fdb_sort_register_trie_sort_cb(uint8 lchip, sys_fdb_sort_get_trie_sort_cb_t cb);
#if 0
int32
sys_usw_fdb_sort_register_mac_limit_cb(uint8 lchip, sys_fdb_sort_mac_limit_cb_t cb);
#endif

#ifdef __cplusplus
}
#endif

#endif /*end of _SYS_L2_FDB_H*/
