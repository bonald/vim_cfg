#ifndef __LACP_CONFIG_H__
#define __LACP_CONFIG_H__

#define LACP_DEFAULT_PORT_PRIORITY      0x8000
#define LACP_DEFAULT_SYSTEM_PRIORITY    0x8000

/* Select the number of significant hash bits used to index links */
#define LACP_PORT_HASH_BITS             3
#define LACP_PORT_HASH_SIZE             (1 << LACP_PORT_HASH_BITS)
#define LACP_PORT_HASH_MASK             (LACP_PORT_HASH_SIZE-1)

/* Maximum PDUs transmitted in fast periodic time - 43.4.16 */
#define LACP_TX_LIMIT                   3
#define LACP_PDU_PKT_LEN                128
#define LACP_MAX_LINKS                  64

#define LACP_MAX_ATTACH_LINKS           GLB_LINKAGG_MAX_MEMBER
#define LACP_MAX_AGGREGATOR_LINKS       LACP_MAX_LINKS

#define LACP_LINK_PRIORITY_MIN          1
#define LACP_LINK_PRIORITY_MAX          65535

#endif /* !__LACP_CONFIG_H__ */
