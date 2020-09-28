#ifndef __OFP_BOND_H__
#define __OFP_BOND_H__

#define OFP_MIN_BOND_PORT_NUM   1
#define OFP_MAX_BOND_PORT_NUM   16

#define OFP_BOND_PORT_NO_BASE   1200
#define OFP_BOND_PORT_NO_MAX    (OFP_BOND_PORT_NO_BASE + OFP_MAX_BOND_PORT_NUM )

#define OFP_BOND_IFINDEX_BASE   0x800

/**
 * @brief bond mode
 */
enum ofp_bond_mode {
    OFP_BM_TCP,     /**<  Transport Layer Load Balance. */
    OFP_BM_SLB,     /**<  Source Load Balance. */
    OFP_BM_AB       /**<  Active Backup. */
};

/* LACP Protocol Implementation. */
enum ofp_lacp_status {
    OFP_LACP_NEGOTIATED,                  /**< Successful LACP negotations. */
    OFP_LACP_CONFIGURED,                  /**< LACP is enabled but not negotiated. */
    OFP_LACP_DISABLED,                    /**< LACP is not enabled. */
    OFP_LACP_MAX                          /**< Max. */
};

/**
 * @brief bond slave information
 */
struct ofp_bond_slave_s {
    char name[OFP_IFNAME_SIZE];         /**< port name */
    uint16_ofp ofport;                  /**< OVS port number */

    /* Link status. */
    bool enabled;                       /**< May be chosen for flows? */
    struct list list_node;    
};
typedef struct ofp_bond_slave_s ofp_bond_slave_t;

/**
 * @brief bond information
 */
struct ofp_bond_info_s
{
    char name[OFP_IFNAME_SIZE];         /**< port name */

    /* Slaves */
    struct list slaves;         

    /* Bonding configure info */
    enum ofp_bond_mode balance;         /**< Balancing mode, one of OFP_BM_*. */
    enum ofp_lacp_status lacp_status;   /**< Status of LACP negotiations. */
    uint16_ofp active_slave_ofport;     /**< OVS port number */
    uint8_ofp lacp_enable;
};
typedef struct ofp_bond_info_s ofp_bond_t;

#endif /* !__OFP_BOND_H__ */
