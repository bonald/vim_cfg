#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_lldp_global_define.h"
#include "gen/tbl_lldp_global.h"
#include "gen/tbl_lldp_if_define.h"
#include "gen/tbl_lldp_if.h"
#include "switch.h"
#include "lldp_define.h"
#include "lldp.h"
#include "lldp_api.h"


static
u_int8_t stp_proto_id [LLDP_802_3_PROTO_ID_LEN] = {0x00, 0x00, 0x42, 0x42,
                                                   0x03, 0x00, 0x00, 0x00};
static
u_int8_t rstp_proto_id [LLDP_802_3_PROTO_ID_LEN] = {0x00, 0x00, 0x42, 0x42,
                                                    0x03, 0x00, 0x00, 0x02};
static
u_int8_t mstp_proto_id [LLDP_802_3_PROTO_ID_LEN] = {0x00, 0x00, 0x42, 0x42,
                                                    0x03, 0x00, 0x00, 0x03};
static
u_int8_t gmrp_proto_id [LLDP_802_3_PROTO_ID_LEN] = {0x00, 0x00, 0x42, 0x42,
                                                    0x03, 0x00, 0x01, 0x02};
static
u_int8_t mmrp_proto_id [LLDP_MRP_PROTO_ID_LEN] = {0x81, 0x10};

static
u_int8_t gvrp_proto_id [LLDP_802_3_PROTO_ID_LEN] = {0x00, 0x00, 0x42, 0x42,
                                                    0x03, 0x00, 0x01, 0x01};
static
u_int8_t mvrp_proto_id [LLDP_MRP_PROTO_ID_LEN] = {0x81, 0x20};

static
u_int8_t dot1x_proto_id [LLDP_DOT1X_PROTO_ID_LEN] = {0x88, 0x8e};

static
u_int8_t lacp_proto_id [LLDP_SLOW_PROTO_ID_LEN] = {0x88, 0x09, 0x01};

static
u_int8_t efm_oam_proto_id [LLDP_SLOW_PROTO_ID_LEN] = {0x88, 0x09, 0x03};


static lldp_protocol_identity_t proto [LLDP_PROTO_MAX] =
{
  {stp_proto_id, LLDP_802_3_PROTO_ID_LEN, LLDP_PROTO_STP,
   "Spanning Tree Protocol"},
  {rstp_proto_id, LLDP_802_3_PROTO_ID_LEN, LLDP_PROTO_RSTP,
   "Rapid Spanning Tree Protocol"},
  {mstp_proto_id, LLDP_802_3_PROTO_ID_LEN, LLDP_PROTO_MSTP,
   "Multiple Spanning Tree Protocol"},
  {gmrp_proto_id, LLDP_802_3_PROTO_ID_LEN, LLDP_PROTO_GMRP,
   "GARP Multicast Registration Protocol"},
  {mmrp_proto_id, LLDP_MRP_PROTO_ID_LEN, LLDP_PROTO_MMRP,
   "MRP Multicast Registration Protocol"},
  {gvrp_proto_id, LLDP_802_3_PROTO_ID_LEN, LLDP_PROTO_GVRP,
   "GARP VLAN Registration Protocol"},
  {mvrp_proto_id, LLDP_MRP_PROTO_ID_LEN, LLDP_PROTO_MVRP,
   "MRP VLAN Registration Protocol"},
  {lacp_proto_id, LLDP_SLOW_PROTO_ID_LEN, LLDP_PROTO_LACP,
   "Link Aggregation"},
  {dot1x_proto_id, LLDP_DOT1X_PROTO_ID_LEN, LLDP_PROTO_DOT1X,
   "Port Authentication"},
  {efm_oam_proto_id, LLDP_SLOW_PROTO_ID_LEN, LLDP_PROTO_EFM_OAM,
   "Ethernet OAM"},
};

struct lldp_dot3MauType
{
    uint8 value;
    char *descr;
    char *name;
};

struct lldp_autoNegCap
{
    char *name;
    char *descr;
};

static struct lldp_dot3MauType g_lldp_dot3_mau_type[LLDP_DOT3_MAU_TYPE_MAX + 1] = 
{
    {0,  "Unknown", "Unknown"},    
    {LLDP_DOT3_MAU_TYPE_AUI, "no internal MAU, view from AUI", "AUI"},
    {LLDP_DOT3_MAU_TYPE_10Base5, "thick coax MAU", "10Base5"},
    {LLDP_DOT3_MAU_TYPE_Foirl, "FOIRL MAU", "Foirl"},
    {LLDP_DOT3_MAU_TYPE_10Base2, "thin coax MAU", "10Base2"},
    {LLDP_DOT3_MAU_TYPE_10BaseT, "UTP MAU", "10BaseT"},
    {LLDP_DOT3_MAU_TYPE_10BaseFP, "passive fiber MAU", "10BaseFP"},
    {LLDP_DOT3_MAU_TYPE_10BaseFB, "sync fiber MAU", "10BaseFB"},
    {LLDP_DOT3_MAU_TYPE_10BaseFL, "async fiber MAU", "10BaseFL"},
    {LLDP_DOT3_MAU_TYPE_10Broad36, "broadband DTE MAU", "10Broad36"},
    {LLDP_DOT3_MAU_TYPE_10BaseTHD, "UTP MAU, half duplex mode", "10BaseTHD"},
    {LLDP_DOT3_MAU_TYPE_10BaseTFD, "UTP MAU, full duplex mode", "10BaseTFD"},
    {LLDP_DOT3_MAU_TYPE_10BaseFLHD, "async fiber MAU, half duplex mode", "10BaseFLHD"},
    {LLDP_DOT3_MAU_TYPE_10BaseFLFD, "async fiber MAU, full duplex mode", "10BaseFLFD"},
    {LLDP_DOT3_MAU_TYPE_100BaseT4, "4 pair category 3 UTP", "100BaseT4"},
    {LLDP_DOT3_MAU_TYPE_100BaseTXHD, "2 pair category 5 UTP, half duplex mode", "100BaseTXHD"},
    {LLDP_DOT3_MAU_TYPE_100BaseTXFD, "2 pair category 5 UTP, full duplex mode", "100BaseTXFD"},
    {LLDP_DOT3_MAU_TYPE_100BaseFXHD, "X fiber over PMT, half duplex mode", "100BaseFXHD"},
    {LLDP_DOT3_MAU_TYPE_100BaseFXFD, "X fiber over PMT, full duplex mode", "100BaseFXFD"},
    {LLDP_DOT3_MAU_TYPE_100BaseT2HD, "2 pair category 3 UTP, half duplex mode", "100BaseT2HD"},
    {LLDP_DOT3_MAU_TYPE_100BaseT2FD, "2 pair category 3 UTP, full duplex mode", "100BaseT2FD"},
    {LLDP_DOT3_MAU_TYPE_1000BaseXHD, "PCS/PMA, unknown PMD, half duplex mode", "1000BaseXHD"},
    {LLDP_DOT3_MAU_TYPE_1000BaseXFD, "PCS/PMA, unknown PMD, full duplex mode", "1000BaseXFD"},
    {LLDP_DOT3_MAU_TYPE_1000BaseLXHD, "Fiber over long-wavelength laser, half duplex mode", "1000BaseLXHD"},
    {LLDP_DOT3_MAU_TYPE_1000BaseLXFD, "Fiber over long-wavelength laser, full duplex mode", "1000BaseLXFD"},
    {LLDP_DOT3_MAU_TYPE_1000BaseSXHD, "Fiber over short-wavelength laser, half duplex mode", "1000BaseSXHD"},
    {LLDP_DOT3_MAU_TYPE_1000BaseSXFD, "Fiber over short-wavelength laser, full duplex mode", "1000BaseSXFD"},
    {LLDP_DOT3_MAU_TYPE_1000BaseCXHD, "Copper over 150-Ohm balanced cable, half duplex mode", "1000BaseCXHD"},
    {LLDP_DOT3_MAU_TYPE_1000BaseCXFD, "Copper over 150-Ohm balanced cable, full duplex mode", "1000BaseCXFD"},
    {LLDP_DOT3_MAU_TYPE_1000BaseTHD, "Four-pair Category 5 UTP, half duplex mode", "1000BaseTHD"},
    {LLDP_DOT3_MAU_TYPE_1000BaseTFD, "Four-pair Category 5 UTP, full duplex mode", "1000BaseTFD"},
    {LLDP_DOT3_MAU_TYPE_10GigBaseX, "X PCS/PMA, unknown PMD.", "10GigBaseX"},
    {LLDP_DOT3_MAU_TYPE_10GigBaseLX4, "X fiber over WWDM optics", "10GigBaseLX4"},
    {LLDP_DOT3_MAU_TYPE_10GigBaseR, "R PCS/PMA, unknown PMD.", "10GigBaseR"},
    {LLDP_DOT3_MAU_TYPE_10GigBaseER, "R fiber over 1550 nm optics", "10GigBaseER"},
    {LLDP_DOT3_MAU_TYPE_10GigBaseLR, "R fiber over 1310 nm optics", "10GigBaseLR"},
    {LLDP_DOT3_MAU_TYPE_10GigBaseSR, "R fiber over 850 nm optics", "10GigBaseSR"},
    {LLDP_DOT3_MAU_TYPE_10GigBaseW, "W PCS/PMA, unknown PMD.", "10GigBaseW"},
    {LLDP_DOT3_MAU_TYPE_10GigBaseEW, "W fiber over 1550 nm optics", "10GigBaseEW"},
    {LLDP_DOT3_MAU_TYPE_10GigBaseLW, "W fiber over 1310 nm optics", "10GigBaseLW"},
    {LLDP_DOT3_MAU_TYPE_10GigBaseSW, "W fiber over 850 nm optics", "10GigBaseSW"},
    {LLDP_DOT3_MAU_TYPE_10GigBaseCX4, "X copper over 8 pair 100-Ohm balanced cable", "10GigBaseCX4"},
    {LLDP_DOT3_MAU_TYPE_2BaseTL, "Voice grade UTP copper, up to 2700m, optional PAF", "2BaseTL"},
    {LLDP_DOT3_MAU_TYPE_10PassTS, "Voice grade UTP copper, up to 750m, optional PAF", "10PassTS"},
    {LLDP_DOT3_MAU_TYPE_100BaseBX10D, "One single-mode fiber OLT, long wavelength, 10km", "100BaseBX10D"},
    {LLDP_DOT3_MAU_TYPE_100BaseBX10U, "One single-mode fiber ONU, long wavelength, 10km", "100BaseBX10U"},
    {LLDP_DOT3_MAU_TYPE_100BaseLX10, "Two single-mode fibers, long wavelength, 10km", "100BaseLX10"},
    {LLDP_DOT3_MAU_TYPE_1000BaseBX10D, "One single-mode fiber OLT, long wavelength, 10km", "1000BaseBX10D"},
    {LLDP_DOT3_MAU_TYPE_1000BaseBX10U, "One single-mode fiber ONU, long wavelength, 10km", "1000BaseBX10U"},
    {LLDP_DOT3_MAU_TYPE_1000BaseLX10, "Two sigle-mode fiber, long wavelength, 10km", "1000BaseLX10"},
    {LLDP_DOT3_MAU_TYPE_1000BasePX10D, "One single-mode fiber EPON OLT, 10km", "1000BasePX10D"},
    {LLDP_DOT3_MAU_TYPE_1000BasePX10U, "One single-mode fiber EPON ONU, 10km", "1000BasePX10U"},
    {LLDP_DOT3_MAU_TYPE_1000BasePX20D, "One single-mode fiber EPON OLT, 20km", "1000BasePX20D"},
    {LLDP_DOT3_MAU_TYPE_1000BasePX20U, "One single-mode fiber EPON ONU, 20km", "1000BasePX20U"},

    {LLDP_DOT3_MAU_TYPE_MAX, "Unknown", "Unknown"}
};

struct lldp_autoNegCap g_lldp_auto_neg_cap [LLDP_AUTONEGO_CAP_MAX] = 
{
    {"Other", "unknown"},
    {"10baseT", "10BASE-T half duplex mode"},
    {"10baseTFD", "10BASE-T full duplex mode"},
    {"100baseT4", "100BASE-T4"},
    {"100baseTX", "100BASE-TX half duplex mode"},
    {"100baseTXFD", "100BASE-TX full duplex mode"},
    {"100baseT2", "100BASE-T2 half duplex mode"},
    {"100baseT2FD", "100BASE-T2 full duplex mode"},
    {"FdxPause", "PAUSE for full-duplex links"},
    {"FdxAPause", "Asymmetric PAUSE for full-duplex links"},
    {"FdxSPause", "Symmetric PAUSE for full-duplex links"},
    {"FdxBPause", "Asymmetric and Symmetric PAUSE for full-duplex links"},
    {"1000baseX", "1000BASE-X -LX -SX -CX half duplex mode"},
    {"1000baseXFD", "1000BASE-X -LX -SX -CX full duplex mode"},
    {"1000baseT", "1000BASE-T half duplex mode"},
    {"1000baseTFD", "1000BASE-T full duplex mode"}                                                                                                                                    
};

char * lldp_auto_neg_cap_to_name(uint32 i)
{
    if (i < LLDP_AUTONEGO_CAP_MAX && i > 0)
    {
        return g_lldp_auto_neg_cap[i].name;
    }

    return g_lldp_auto_neg_cap[0].name;
}

char * lldp_auto_neg_cap_to_descr(uint32 i)
{
    if (i < LLDP_AUTONEGO_CAP_MAX && i > 0)
    {
        return g_lldp_auto_neg_cap[i].descr;
    }

    return g_lldp_auto_neg_cap[0].descr;
}

char * lldp_mau_type_to_name(uint32 i)
{   
    if (i < LLDP_DOT3_MAU_TYPE_MAX && i > 0)
    {
        return g_lldp_dot3_mau_type[i].name;
    }

    return g_lldp_dot3_mau_type[LLDP_DOT3_MAU_TYPE_MAX].name;
}

char * lldp_mau_type_to_descr(uint32 i)
{   
    if (i < LLDP_DOT3_MAU_TYPE_MAX && i > 0)
    {
        return g_lldp_dot3_mau_type[i].descr;
    }

    return g_lldp_dot3_mau_type[LLDP_DOT3_MAU_TYPE_MAX].descr;
}


int32 
lldp_autoneg_support_get (tbl_interface_t* p_db_if)
{
     uint32 autonego_support = 0;
     GLB_SET_BIT(autonego_support, LLDP_MAU_AUTONEG_SUPPORTED);
     if (p_db_if->speed == GLB_SPEED_AUTO || p_db_if->duplex == GLB_DUPLEX_AUTO)
     {
          GLB_SET_BIT(autonego_support, LLDP_MAU_AUTONEG_ENABLED);
     }
     return autonego_support;
}

int32 
lldp_autoneg_capability_get (tbl_interface_t* p_db_if)
{
    uint32 bandwidth = 0 ;
    uint32 autonego_cap = LLDP_AUTONEGO_CAP_BOTHER;
    bandwidth = (p_db_if->oper_bandwidth)/1000/1000;
    if (p_db_if->duplex == GLB_DUPLEX_HALF)
    {
        if (bandwidth == 10)
        {
            autonego_cap = LLDP_AUTONEGO_CAP_B10BASET;        
        }
        else if (bandwidth == 100)
        {
            autonego_cap = LLDP_AUTONEGO_CAP_B100BASET2;            
        }
        else if (bandwidth == 1000)
        {
            autonego_cap = LLDP_AUTONEGO_CAP_B1000BASET;            
        }
        else
        {
            autonego_cap = LLDP_AUTONEGO_CAP_BOTHER;
        }
    }
    else if (p_db_if->duplex == GLB_DUPLEX_FULL)
    {
        if (bandwidth == 10)
        {
            autonego_cap = LLDP_AUTONEGO_CAP_B10BASETFD;        
        }
        else if (bandwidth == 100)
        {
            autonego_cap = LLDP_AUTONEGO_CAP_B100BASET2FD;            
        }
        else if (bandwidth == 1000)
        {
            autonego_cap = LLDP_AUTONEGO_CAP_B1000BASETFD;            
        }
        else
        {
            autonego_cap = LLDP_AUTONEGO_CAP_BOTHER;
        }    
    }
    return autonego_cap;
}

int32
lldp_oper_mau_type_get (tbl_interface_t* p_db_if)
{
    uint32 oper_mau_type = 0;
    uint32 bandwidth = 0;
    bandwidth = (p_db_if->oper_bandwidth)/1000 /1000;
    if (p_db_if->duplex == GLB_DUPLEX_HALF)
    {
        if (bandwidth == 10)
        {
            oper_mau_type = LLDP_DOT3_MAU_TYPE_10BaseTHD;        
        }
        else if (bandwidth == 100)
        {
            oper_mau_type = LLDP_DOT3_MAU_TYPE_100BaseT2HD;            
        }
        else if (bandwidth == 1000)
        {
            oper_mau_type = LLDP_DOT3_MAU_TYPE_1000BaseTHD;            
        }
    }
    else if (p_db_if->duplex == GLB_DUPLEX_FULL)
    {
        if (bandwidth == 10)
        {
           oper_mau_type = LLDP_DOT3_MAU_TYPE_10BaseTFD;        
        }
        else if (bandwidth == 100)
        {
            oper_mau_type = LLDP_DOT3_MAU_TYPE_100BaseT2FD;            
        }
        else if (bandwidth == 1000)
        {
            oper_mau_type = LLDP_DOT3_MAU_TYPE_1000BaseTFD;            
        }
    }
  return oper_mau_type;
}

lldp_protocol_identity_t*
lldp_get_protocol_identity (uint32 proto_id)
{
  if (proto_id >= LLDP_PROTO_MAX)
    return NULL;

  return &proto[proto_id];
}

uint8
lldp_get_protocol_id (uint8 *protocol_payload,
                            uint8 protocol_id_len)
{
  uint8 proto_id;

  for (proto_id = LLDP_PROTO_STP; proto_id < LLDP_PROTO_MAX; proto_id++)
  {
      if (proto[proto_id].protocol_id_len == protocol_id_len &&
        (sal_memcmp(proto[proto_id].protocol_payload, protocol_payload, protocol_id_len) == 0))
     
        return proto[proto_id].protocol_id;
  }

  return LLDP_PROTO_MAX;
}

