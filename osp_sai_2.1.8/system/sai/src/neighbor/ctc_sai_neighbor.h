#if !defined (__SAI_NEIGHBOR_H_)
#define __SAI_NEIGHBOR_H_

#include <saineighbor.h>
#include <saiswitch.h>

#define CTC_SAI_NEIGHBOR_MAX  16384

typedef int32 (*SAI_NEI_CB)(sai_neighbor_entry_t* p_neighbor_entry);

/* modified by liwh for bug 48486, 2018-08-14 */
typedef int32 (*SAI_NEI_ND_CB)(uint8 *p_daddr, char *ifname);
/* liwh end */

typedef enum
{
    SAI_NEIGHBOR_CREATE,
    SAI_NEIGHBOR_REMOVE,
    SAI_NEIGHBOR_UPDATE_PORT,
    SAI_NEIGHBOR_UPDATE_ACTION,
    SAI_NEIGHBOR_UPDATE_L3IFTYPE,
    SAI_NEIGHBOR_UPDATE_CATEGORY_ID,
}ctc_sai_neighbor_update_type_t;

struct ipv6hdr {
#if _GLB_HOST_IS_LE
    uint8     priority:4;
    uint8     version:4;
#else
    uint8     version:4;
    uint8     priority:4;
#endif
    uint8     flow_lbl[3];
    uint16   payload_len;
    uint8     nexthdr;
    uint8     hop_limit;
    uint8     saddr[16];
    uint8     daddr[16];
};

extern sai_status_t
neighbor_update_nexthop(
    _In_  stbl_neighbor_t           *p_sdb_nb,
    _In_  sai_object_id_t           port_id,
    _In_  uint32_t                  update_type);

sai_status_t
ctc_sai_neighbor_init(void);

sai_status_t
ctc_neighbor_update_entry(
    _In_  stbl_neighbor_t *p_sdb_nb,
    _In_  uint32_t         update_type);

/* modified by liwh for bug 48486, 2018-08-14 */
int32
ctc_nd_update_neighbor(uint8 *p_daddr, char *ifname);
/* liwh end */

#endif

