
#ifndef __PROTO_MODULE_H__
#define __PROTO_MODULE_H__

typedef enum pm_id_e
{
    PM_ID_INVALID       = 0,
    PM_ID_CTL           = 1,
    PM_ID_CCS           = 2,
    PM_ID_CDS           = 3,
    PM_ID_CHSM          = 4,
    PM_ID_SWITCH        = 5,
    PM_ID_ROUTED        = 6,
    PM_ID_OPM           = 7,
    PM_ID_AUTHD         = 8,
    PM_ID_APPCFG        = 9,
    PM_ID_FEA           = 10,
    PM_ID_OPENFLOW      = 11,
    PM_ID_DHCRELAY      = 12,
    PM_ID_DHCSNOOPING   = 13,
    PM_ID_DHCLIENT      = 14,
    PM_ID_PTP           = 15,
    PM_ID_BHM           = 16,
    PM_ID_EVENTMGR      = 17,
    PM_ID_SSM           = 18,
    PM_ID_SUBSCRIBER    = 19,
    PM_ID_NCS           = 20,
    PM_ID_DHCPD6        = 21,
    PM_ID_DHCLIENT6        = 22,
    PM_ID_MAX           = 23
} pm_id_t;

typedef enum pm_id_ext_e
{
    PM_ID_EXT_BASE      = PM_ID_MAX,
    PM_ID_EXT_ZEBRA     = PM_ID_EXT_BASE + 4,
    PM_ID_EXT_OSPFD     = PM_ID_EXT_BASE + 5,
    PM_ID_EXT_BGPD      = PM_ID_EXT_BASE + 6,
    PM_ID_EXT_PIMD      = PM_ID_EXT_BASE + 7,
    PM_ID_EXT_OVSDB     = PM_ID_EXT_BASE + 8,
    PM_ID_EXT_MAX       = PM_ID_EXT_BASE + 9
} pm_id_ext_t;

const char*
proto_module_desc(pm_id_t pm_id);

const char*
proto_module_name_desc(pm_id_t pm_id);

const char*
proto_module_ext_name_desc(pm_id_ext_t pm_id);

#endif /* !__PROTO_MODULE_H__ */

