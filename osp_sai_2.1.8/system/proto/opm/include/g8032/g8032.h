#ifndef __G8032_H__
#define __G8032_H__
#include "gen/tbl_g8032_ring_define.h"
#include "gen/tbl_g8032_ring.h"
#define G8032_ERROR_BASE	   	-100
#define G8032_BRIDGE_NOT_FOUND  	(G8032_ERROR_BASE  + 1)
#define G8032_API_ERR_MEM       	(G8032_ERROR_BASE  + 2)
#define G8032_RING_ALREADY_EXISTS       (G8032_ERROR_BASE  + 3)
#define G8032_NODE_DELETE_ERROR         (G8032_ERROR_BASE  + 4)
#define G8032_API_ERR_RING_NOT_FOUND    (G8032_ERROR_BASE  + 5)
#define G8032_XCALLOC_FAILED 		(G8032_ERROR_BASE  + 6)
#define G8032_RING_TREE_CREATION_ERROR  (G8032_ERROR_BASE  + 7)
#define G8032_FAILURE			(G8032_ERROR_BASE  + 8)
#define G8032_RING_NOT_FOUND	        (G8032_ERROR_BASE  + 9)
#define G8032_PDU_PARSING_FAILED 	(G8032_ERROR_BASE  + 10)
#define G8032_INVALID_STATUS_IN_PDU     (G8032_ERROR_BASE  + 11)
#define G8032_RING_NULL		        (G8032_ERROR_BASE  + 12)
#define G8032_INVALID_INTF_TYPE  	(G8032_ERROR_BASE  + 13)
#define G8032_NOT_IN_GUARD_STATE	(G8032_ERROR_BASE  + 15)
#define G8032_SIGNAL_FAIL_FOR_WORKING   (G8032_ERROR_BASE  + 16)
#define G8032_VALIDATE_ERR_IN_PDU       (G8032_ERROR_BASE  + 17)
#define G8032_INVALID_REQUEST_STATE	(G8032_ERROR_BASE  + 18)
#define G8032_CFM_MD_NOTFOUND		(G8032_ERROR_BASE  + 19)
#define G8032_CFM_MA_NOTFOUND		(G8032_ERROR_BASE  + 20)
#define G8032_ONMD_BR_PORT_NOT_FOUND    (G8032_ERROR_BASE  + 21)
#define G8032_CFM_MEP_NOTFOUND	        (G8032_ERROR_BASE  + 22)
#define G8032_RPL_UNBLOCK_FAILURE       (G8032_ERROR_BASE  + 23)
#define G8032_PVID_OF_MA_PG_NOT_MATCHING (G8032_ERROR_BASE + 24)
#define G8032_EAST_INTERFACE_NOT_FOUND  (G8032_ERROR_BASE  + 25)
#define G8032_WEST_INTERFACE_NOT_FOUND  (G8032_ERROR_BASE  + 26)
#define G8032_INIT_FAIL                 (G8032_ERROR_BASE  + 27)
#define G8032_MA_VLAN_NOT_FOUND         (G8032_ERROR_BASE  + 28)
#define G8032_DISCARD_PDU               (G8032_ERROR_BASE  + 29)
#define G8032_WTR_NOT_ALLOWED_FOR_NON_OWNER (G8032_ERROR_BASE + 30)
#define G8032_INVALID_WTR_TIMEOUT         (G8032_ERROR_BASE + 31)
#define G8032_INVALID_HOLD_OFF_TIMEOUT  (G8032_ERROR_BASE + 32)
#define G8032_INVALID_GUARD_TIMEOUT     (G8032_ERROR_BASE + 33)
#define G8032_SHARED_LINK_NOT_CONFG     (G8032_ERROR_BASE + 34)
#define G8032_MEP_MISTMATCH             (G8032_ERROR_BASE + 35)
#define G032_CONFIG_COMPLETE            (G8032_ERROR_BASE + 36)
#define G8032_INTERFACE_MISMATCH        (G8032_ERROR_BASE + 37)
#define G8032_RPL_BLOCK_FAILURE         (G8032_ERROR_BASE + 38)
#define G8032_SHARED_LINK_ON_RPL        (G8032_ERROR_BASE + 39)
#define G8032_SHARED_LINK_CNTRL_VLAN_CONFLICT (G8032_ERROR_BASE + 40)

#define G8032_RAPS_SUB_CODE_FLUSH_FDB 0xE0

#define G8032_FLAG_ISSET(V,F)        (((V) & (F)) == (F))

#define G8032_RING_DEFAULT_GUARD_TIMER_INTERVAL_MS 500
#define G8032_RING_DEFAULT_WTR_TIMER_INTERVAL_IN_MIN 5 
#define G8032_DEF_RAPS_MEL_VALUE        7
#define G8032_RING_DEF_RAPS_TX_INTERVAL_SECONDS            5
#define G8032_RING_HOLD_MIN_TIMEOUT_MS 0
#define G8032_RING_HOLD_MAX_TIMEOUT_MS 10000

typedef enum
{
    G8032_STATE_SF = 0xB0,
    G8032_STATE_NR = 0x00,
    G8032_STATE_FS = 0xD0,
    G8032_STATE_MS = 0x70,
    G8032_STATE_EVENT = 0xE0,
}raps_pkt_state_t;

typedef enum
{
    G8032_STATUS_RB_SET = 0x4,
    G8032_STATUS_DNF_SET = 0x2,
    G8032_STATUS_BPR_SET = 0x1,
}raps_pkt_status_t;

typedef struct
{
    uint32 req_state;
    uint8 is_east;   
    tbl_g8032_ring_t *p_g8032_ring;
}hold_off_info_t;
  
int32
g8032_start(void);

int32
g8032_ring_cmd_get_ring(uint32 ring_id, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field);

int32
g8032_ring_switch_trigged_by_cfm(uint32  ring_id, uint32  ifindex, bool isUp);

int32 
g8032_delete_association_by_cfm(uint32  ring_id);
#endif
