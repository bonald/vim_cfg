#ifndef __OFP_PORT_H__
#define __OFP_PORT_H__

#include "glb_if_define.h"

#define OFP_INVALID_GPORT       GLB_INVALID_GPORTID
#define OFP_INVALID_OFP_PORT    0xffff
#define OFP_INVALID_IFINEX      GLB_INVALID_IFINDEX

/* Modifed by weizj for tunnel bug 33434 */
/* ofport:
 *    l2gre    201 - 1100
 *    bond     1201 - 1216
 *    nvgre    1301 - 2200
 *    vxlan    2201 - 3100
 */
#define OFP_MAX_PORT_NUM 3101

/* 24Q after port split will have 24*4 = 96, so we must modify this number to a more larger number. */
#define OFP_MAX_PHY_PORT_NUM 128

#define OFP_PHYSICAL_PORT_NO_BASE 0
#define OFP_PHYSICAL_PORT_NO_MAX  (OFP_PHYSICAL_PORT_NO_BASE + OFP_MAX_PHY_PORT_NUM + 1)

#define OFP_PORT_BRIDGE_PORT_NUMBER 0
#define OFP_PORT_CPU_PORT_NAME      "eth1"
#define OFP_PORT_BRIDGE_NAME        "br0"

#define OFP_MIN_STUB_PORT_NUM   1
#define OFP_MAX_STUB_PORT_NUM   200

#define OFP_STUB_PORT_NO_BASE   4000
#define OFP_STUB_PORT_NO_MAX    (OFP_STUB_PORT_NO_BASE + OFP_MAX_STUB_PORT_NUM + 1)

#define OFP_MIN_AGG_PORT_NUM   1
/*modified by hansf for bug 43750*/
//#define OFP_MAX_AGG_PORT_NUM   16
#define OFP_MAX_AGG_PORT_NUM   GLB_LAG_GROUP_NUM

#define OFP_AGG_PORT_NO_BASE   1220
#define OFP_AGG_PORT_NO_MAX    (OFP_AGG_PORT_NO_BASE + OFP_MAX_AGG_PORT_NUM + 1)


#define OFP_STUB_IFINDEX_BASE   10000

/* queue */
#define OFP_MAX_QUEUE_VALUE   8
#define OFP_QUEUE_DISABLE_RATE  0xFFFF    /* In 1/10 of a percent; >1000 -> disabled. [spec1.3]*/
#define OFP_QUEUE_DISABLE_ABSO_RATE  0xFFFFFFFF    

/**
 * @brief interface type
 */
enum ofp_interface_type_e
{
    OFP_INTERFACE_TYPE_INTERNAL,    /**< internal interface */
    OFP_INTERFACE_TYPE_TAP,         /**< tap interface */
    OFP_INTERFACE_TYPE_PHYSICAL,    /**< physical interface */
    /* Added by weizj for tunnel */
    OFP_INTERFACE_TYPE_L2GRE,       /**< l2gre interface */
    OFP_INTERFACE_TYPE_NVGRE,       /**< nvgre interface */
    OFP_INTERFACE_TYPE_VXLAN,       /**< vxlan interface */
    /* End by weizj for tunnel */
    OFP_INTERFACE_TYPE_SYSTEM,      /**< system. e.g br0 */
    OFP_INTERFACE_TYPE_BOND,        /**< bond interface */
    OFP_INTERFACE_TYPE_STUB,        /**< stub interface */
    OFP_INTERFACE_TYPE_AGG,        /**< stub interface */

    OFP_INTERFACE_TYPE_UNKNOWN,     /**< unknown */
    OFP_INTERFACE_TYPE_MAX          /**< interface max */
};
typedef enum ofp_interface_type_e ofp_interface_type_t;

/* Modifed by weizj for tunnel */
#define OFP_MAP_INTF_TYPE_STR(type)                     \
    (type == OFP_INTERFACE_TYPE_INTERNAL ? "Internal" : \
     type == OFP_INTERFACE_TYPE_TAP      ? "Tap"      : \
     type == OFP_INTERFACE_TYPE_PHYSICAL ? "Physical" : \
     type == OFP_INTERFACE_TYPE_L2GRE    ? "L2GRE"    : \
     type == OFP_INTERFACE_TYPE_NVGRE    ? "NVGRE"    : \
     type == OFP_INTERFACE_TYPE_VXLAN    ? "VXLAN"    : \
     type == OFP_INTERFACE_TYPE_SYSTEM   ? "SYSTEM"   : \
     type == OFP_INTERFACE_TYPE_BOND     ? "Bond"     : \
     type == OFP_INTERFACE_TYPE_STUB     ? "STUB"     : \
     "Unknown")
/* End by weizj for tunnel */

/**
 * @brief port information
 */
struct ofp_port_info_s
{
    ofp_interface_type_t type;          /**< interface type */

    char       name[OFP_IFNAME_SIZE];   /**< port name */
    uint16_ofp ofport;                  /**< OVS port number */
    uint32_ofp  ifindex;                /**< Interface index */

    void* info;                         /**< port attr info */
};
typedef struct ofp_port_info_s ofp_port_info_t;

#endif /* !__OFP_PORT_H__ */
