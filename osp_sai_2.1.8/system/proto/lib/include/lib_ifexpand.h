/****************************************************************************
* $Id$
*  Expand interface for CLI
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2008-08-14 10:48
* Reason        : First Create.
****************************************************************************/

#ifndef _INTF_EXPAND_H_
#define _INTF_EXPAND_H_

/* help strings */
#define M_HELPSTR_MODULE_NUMBER "Module number"
#define M_HELPSTR_SLOT_NUMBER   "Slot number"
#define M_HELPSTR_PORT_NUMBER   "Port number"
#define M_HELPSTR_SUB_PORT_NUMBER   "Sub-port number"
#define M_HELPSTR_VLAN_NUMBER   "Vlan number"

#define M_NUMBER_NOTINUSE       0xFFFFFFFF

/* basic interface tokens */
#define IFNAME_PHYSICAL     "IFPHYSICAL"
#define IFNAME_AGG          "IFAGG"
#define IFNAME_LOOPBACK     "IFLOOPBACK"
#define IFNAME_VLAN         "IFVLAN"
#define IFNAME_TUNNEL       "IFTUNNEL"
#define IFNAME_NULL         "IFNULL"
/* combination interface tokens */
#define IFNAME_COMMON       "IFCOMM"    /* include physical / agg / loopback / vlan interface */
#define IFNAME_ALL          "IFNAME"     /* include all interface */
//#define IFNAME_ALL          "IFALL"     /* include all interface */

/*
 * 1. nMin <= nMax
 * 2. nMin == nMax for single number
 */
typedef struct _tagRangePair {
    unsigned long nMin;
    unsigned long nMax;
} RangePair_t;

#if 0
struct tagRangeItem;
typedef TAILQ_HEAD(tagRangeHdr, tagRangeItem) RangeHdr_t;

typedef struct tagRangeItem
{
    TAILQ_ENTRY(tagRangeItem) entry;

    RangePair_t range;

    RangeHdr_t subhdr;
} RangeItem_t;
#endif

typedef struct _tagPhysicalIntfCfg {
    unsigned long flags;
#define M_PHYSICAL_FLAG_HAVE_GE     0x01
#define M_PHYSICAL_FLAG_HAVE_TENGE  0x02
#define M_PHYSICAL_FLAG_HAVE_FE     0x04
#if 0
    RangeHdr_t geHdr;
    RangeHdr_t tengeHdr;
    RangeHdr_t feHdr;
#endif    
} PhysicalIntfCfg_t;

typedef struct _tagSysIntfCfg {
    IntfType_e  type;
    unsigned long flags;
#define M_EXP_FLAG_NOTINUSE 0x01
#define M_EXP_FLAG_COMMON   0x02
#define M_EXP_FLAG_PORTONLY 0x04
#define M_EXP_FLAG_SLOTPORT 0x08
#define M_EXP_FLAG_CHECK
    char *pszToken;
    char *pszName;
    char *pszDesc;
    PhysicalIntfCfg_t physical;
    RangePair_t slot;
    RangePair_t port;
} SysIntfCfg_t;
/* for none physical interfaces */
#define M_COMINTF_PORT_MIN              port.nMin
#define M_COMINTF_PORT_MAX              port.nMax
#define M_COMINTF_SLOT_MIN              slot.nMin
#define M_COMINTF_SLOT_MAX              slot.nMax
/* for physical interfaces */
#define M_PHYINTF_STRUCT                physical
#define M_PHYINTF_FLAG                  physical.flags
#define M_PHYINTF_FEHDR                 physical.feHdr
#define M_PHYINTF_GEHDR                 physical.geHdr
#define M_PHYINTF_TENGEHDR              physical.tengeHdr

extern SysIntfCfg_t g_stIntfCfg[];

typedef enum tagIntfMatchMode {
    E_MATCH_MODE_COMPLETE = 0,
    E_MATCH_MODE_DESCRIBE
} IntfMatchMode_e;

/* IF_SPLITTING */
#define IFEXPAND_DEF_PHY_PORT_PREFIX GLB_DEF_PHY_PORT_PREFIX
#define IFEXPAND_DEF_PHY_PORT_FULL_PREFIX GLB_DEF_PHY_PORT_FULL_PREFIX

#define IFEXPAND_PHY_PORT_NUM_MAX 256  
#define IFEXPAND_PHT_PORT_TYPE_MAX GLB_PHT_PORT_TYPE_MAX

#define IFEXPAND_ARGC_MAX               128
#define IFEXPAND_ARGV_MAX_LEN           256

struct ifexpand_phy_port_prefix_s
{
    bool valid;
    char prefix[GLB_PHY_PORT_PREFIX_SZ];
    char full_prefix[GLB_PHY_PORT_FULL_PREFIX_SZ];
    char desc[IFEXPAND_ARGC_MAX];
};
typedef struct ifexpand_phy_port_prefix_s ifexpand_phy_port_prefix_t;

struct ifexpand_phy_port_s
{
    uint16 slot_id;
    uint16 port_id;
    uint16 sub_id;
    uint16 port_type;
};
typedef struct ifexpand_phy_port_s ifexpand_phy_port_t;

struct ifexpand_phy_port_master_s
{
    char port_connection;
    char sub_port_connection;
    uint16 valid_num;  /*valid port number in ifexpand_phy_port_t*/
    ifexpand_phy_port_t port[GLB_PHT_PORT_NUM_MAX];
    ifexpand_phy_port_prefix_t port_prefix[GLB_PHT_PORT_TYPE_MAX];

    int split_type[GLB_PORT_SPEED_MAX]; /*the real speed type*/
    
    uint16 match_port_type[GLB_PHT_PORT_TYPE_MAX]; /*save tmp info while parse port*/
    uint16 match_port_id; /*save tmp info while parse port*/
    uint16 match_sub_port_id; /*save tmp info while parse port*/
    char match_name[GLB_PHY_PORT_FULL_NAME_SZ]; /*same as INTERFACE_NAMSIZ*/
};
typedef struct ifexpand_phy_port_master_s ifexpand_phy_port_master_t;

/* IF_SPLITTING, simple stub, need remove later. */ 
struct cli {
};
#define cli_out(cli, ARGS...)            do {} while(0)


unsigned long get_offset_for_value(unsigned long uValue);
int validate_may_in_range(unsigned long uMin, unsigned long uMax, unsigned long uValue);

char *intf_expand_parse_current_ifname(void);

/* for command line help */
int intf_expand_get_helpstr_len(IntfType_e intfType);
void intf_expand_get_phy_if_info(void);
int intf_expand_transform_ifname(char *cp);
char *ifrange_endnum_match(char *cp, IntfMatch_e *pmatch);
int ifrange_get_complete_count();
void ifrange_get_complete_string(char *oldtext, char **ptr, int *nCntRet);
int ifrange_output_helpstr(struct cli *cli, int max, int width);
int ifrange_get_helpstr_len();
int intf_phy_port_in_split(uint16 port_id);
#endif /* !_INTF_EXPAND_H_ */
