#ifndef __PHY_AQR40X_H__
#define __PHY_AQR40X_H__

#include "phy_drv.h"
#include "glb_hw_define.h"
#if 0
#define AQ_RET_OK 0
/*! The maximum number of ports that can be MDIO bootloaded at once. */
#define AQ_MAX_NUM_PHY_IDS 48
/*! The byte offset from the top of the PHY image to the header content (HHD devices). */
#define AQ_PHY_IMAGE_HEADER_CONTENT_OFFSET_HHD 0x300
/*! The byte address, in processor memory, of the start of the IRAM segment. */
#define AQ_IRAM_BASE_ADDRESS 0x40000000
/*! The byte address, in processor memory, of the start of the DRAM segment. */
#define AQ_DRAM_BASE_ADDRESS 0x3FFE0000

typedef enum {False = 0, True = 1} AQ_boolean;
/*! This enumeration is used to select a loopback mode for an Aquantia
    PHY.*/
typedef enum    
{    
  /*! This indicates that no loopback mode is selected.*/
 No_Loopback,   
  /*! This selects the network loopback test mode within the system
      interface.*/
 SystemInterface_NetworkLoopback,   
  /*! This selects the system loopback test mode within the network
      interface.*/
 NetworkInterface_SystemLoopback   
} AQ_API_LoopbackControl;   


/*! This enumeration is used to report what the current connection
    state an Aquantia PHY is in.*/
typedef enum    
{    
  /*! This indicates that the target Aquantia PHY is in the process of
      performing a reset. This may be triggered by either a power-
      supply fluctuation event, a soft-reset event, a hard-reset
      event, or by pulling the Reset* pin on the Aquantia PHY low.*/
 AQ_API_ConnS_Reset,   
  /*! This indicates that the target Aquantia PHY's processor is
      stalled, so it is incapable of operation. This state occurs when
      the target Aquantia PHY is operating in boot-load mode, and is
      waiting to have an image loaded into its IRAM and DRAM.*/
 AQ_API_ConnS_Stall,   
  /*! This indicates that the target Aquantia PHY is in an inactive
      state and is not connected to a far-end device, nor is it
      attempting to. This occurs when the Aquantia PHY is in low-power
      mode, high-impedance mode, is advertising no connection
      capability, or being reset.*/
 AQ_API_ConnS_Inactive,   
  /*! This indicates that the target Aquantia PHY is in the process of
      performing cable diagnostics during the MDI / MDI-X phase of the
      link connection.*/
 AQ_API_ConnS_CableDiagnostics,   
  /*! This indicates that the target Aquantia PHY is in the process of
      attempting to connect with 10G as the highest advertised rate.*/
 AQ_API_ConnS_10G_Autoneg,   
  /*! This indicates that the target Aquantia PHY is in the process of
      attempting to connect with 1G as the highest advertised rate.*/
 AQ_API_ConnS_1G_Autoneg,   
  /*! This indicates that the target Aquantia PHY is in the process of
      attempting to connect with 100M as the highest advertised
      rate.*/
 AQ_API_ConnS_100M_Autoneg,   
  /*! This indicates that the target Aquantia PHY is attempting to
      connect with the far-end device at 10G, and is in the training
      phase of the process.*/
 AQ_API_ConnS_10G_Training,   
  /*! This indicates that the target Aquantia PHY is attempting to
      connect with the far-end device at 1G, and is in the training
      phase of the process.*/
 AQ_API_ConnS_1G_Training,   
  /*! This indicates that the target Aquantia PHY is attempting to
      connect with the far-end device at 100M, and is initializing the
      1G/100M core.*/
 AQ_API_ConnS_100M_Training,   
  /*! This indicates that the target Aquantia PHY is connected to a
      far-end device at 10G.*/
 AQ_API_ConnS_10G_Connected,   
  /*! This indicates that the target Aquantia PHY is connected to a
      far-end device at 1G.*/
 AQ_API_ConnS_1G_Connected,   
  /*! This indicates that the target Aquantia PHY is connected to a
      far-end device at 1G and Wake On LAN is enabled.*/
 AQ_API_ConnS_1G_WoL_Connected,   
  /*! This indicates that the target Aquantia PHY is connected to a
      far-end device at 100M.*/
 AQ_API_ConnS_100M_Connected,   
  /*! This indicates that the target Aquantia PHY is connected to a
      far-end device at 100M and Wake On LAN is enabled.*/
 AQ_API_ConnS_100M_WoL_Connected,   
  /*! This indicates that the target Aquantia PHY is in the 2 second
      time-out in order to complete link break.*/
 AQ_API_ConnS_LinkBreak,   
  /*! This indicates that the target Aquantia PHY is in a test mode.*/
 AQ_API_ConnS_TestMode,   
  /*! This indicates that the target Aquantia PHY is in a loopback
      mode.*/
 AQ_API_ConnS_LoopbackMode,   
  /*! This indicates that the target Aquantia PHY is in a low power
      mode.*/
 AQ_API_ConnS_LowPowerMode,   
  /*! This state typically indicates that the target Aquantia PHY is
      not functioning properly (i.e. not powered on, etc.).*/
 AQ_API_ConnS_Invalid   
} AQ_API_ConnectionState;  

/*! This enumeration is used to report the current MDI / MDI-X state
    an Aquantia PHY.*/
typedef enum    
{    
  /*! This indicates that the target Aquantia PHY has not resolved the
      MDI / MDI-X state.*/
 AQ_API_MS_None,   
  /*! This indicates that the target Aquantia PHY has resolved the MDI
      / MDI-X state as MDI.*/
 AQ_API_MS_MDI,   
  /*! This indicates that the target Aquantia PHY has resolved the MDI
      / MDI-X state as MDI-X.*/
 AQ_API_MS_MDI_X   
} AQ_API_MDI_State;  

typedef enum    
{    
  /*! Identifies that the Aquantia PHY was configured as the master
      during autonegotiation.*/
 AQ_API_MSS_Master,   
  /*! Identifies that the Aquantia PHY was configured as the slave
      during autonegotiation.*/
 AQ_API_MSS_Slave   
} AQ_API_MasterSlaveStatus;  

/*! Enumerates the system interface's various operating modes.*/
typedef enum    
{    
  /*! XFI mode.*/
 AQ_API_SIOM_XFI,   
  /*! Backplane KR mode.*/
 AQ_API_SIOM_BackplaneKR,   
  /*! Backplane KX mode.*/
 AQ_API_SIOM_BackplaneKX,   
  /*! XAUI mode.*/
 AQ_API_SIOM_XAUI,   
  /*! XAUI Pause Based mode.*/
 AQ_API_SIOM_XAUI_PauseBased,   
  /*! RXAUI mode.*/
 AQ_API_SIOM_RXAUI,   
  /*! SGMII mode.*/
 AQ_API_SIOM_SGMII,   
  /*! MAC mode.*/
 AQ_API_SIOM_MAC,   
  /*! System interface is off.*/
 AQ_API_SIOM_OFF,   
  /*! Unrecognized system interface mode.*/
 AQ_API_SIOM_Other   
} AQ_API_SysIntfOpMode; 

 
/*! Enumerates the system interface's various data rates.*/
typedef enum    
{    
  /*! 10M data rate.*/
 AQ_API_SIR_10M,   
  /*! 100M data rate.*/
 AQ_API_SIR_100M,   
  /*! 1G data rate.*/
 AQ_API_SIR_1G,   
  /*! 10G data rate.*/
 AQ_API_SIR_10G,   
  /*! System interface is powered down.*/
 AQ_API_SIR_PowerDown,   
  /*! Unrecognized system interface rate.*/
 AQ_API_SIR_Other   
} AQ_API_SysIntfRate;   

/*! Describes the system interface's autonegotiation completion
    status.*/
typedef enum    
{    
  /*! Autonegotiation not applicable based on the current system
      interface operating mode.*/
 AQ_API_SIAS_NA,   
  /*! Autonegotiation has not been completed.*/
 AQ_API_SIAS_NotComplete,   
  /*! Autonegotiation has been completed.*/
 AQ_API_SIAS_Complete,   
  /*! Unrecognized system interface autonegotiation completion
      status.*/
 AQ_API_SIAS_Other   
} AQ_API_SysIntfAutonegStatus;  

/*! This structure is used to convey the connection status an Aquantia
    PHY.*/
typedef struct    
{    
 /*MDI connection state*/   
  /*! If true, a far-end Ethernet device exists, as valid link pulses
      have been detected in the most recent autonegotiation session,
      or a valid Ethernet connection has been established. If false,
      no connection is established, and the most recent attempt at
      autonegotiation failed to detect any valid link pulses. This
      field is duplicated for convenience in
      AQ_API_LinkPartnerStatus.*/
 AQ_boolean  farEndDeviceExists; 
  /*! If true, energy greater than -15 dBm is present on the line.
      This may or may not be a valid Ethernet signal, depending on the
      connection status.*/
 AQ_boolean  energyOnLine; 
  /*! The number of autonegotiation attempts that have been made since
      power-up or a valid connection was lost. If a connection is
      established, this is the number of autonegotiation attempts that
      were made before the link was established. If a connection is
      not established, this is the number of autonegotiation attempts
      that have been tried so far. Upon reaching saturation, the value
      wraps back to zero.*/
 uint16_t  numberOfConnectionAttempts; 
 /*state*/   
  /*! The current state of the Aquantia PHY.*/
 AQ_API_ConnectionState   state;
  /*! If true, the connection is a duplex connection.Otherwise the
      connection is simplex (or non-existent).*/
 AQ_boolean   duplex;
  /*! The current MDI / MDI-X state of the connection.*/
 AQ_API_MDI_State   MDI_MDI_X;
  /*! The result of the master / slave resolution from autonegotiation
      process.*/
 AQ_API_MasterSlaveStatus   masterSlaveStatus;
 /*system interface state*/   
  /*! Identifies the current operating mode of the system interface.
      Compatibility: 28nm PHYs only.*/
 AQ_API_SysIntfOpMode   sysIntfCurrentMode;
  /*! The current data rate of system interface.
      Compatibility: 28nm PHYs only.*/
 AQ_API_SysIntfRate   sysIntfCurrentRate;
  /*! If true, system interface is ready to transmit.
      Compatibility: 28nm PHYs only.*/
 AQ_boolean   txReady;
  /*! If true, system interface is linked and ready to receive.
      Compatibility: 28nm PHYs only.*/
 AQ_boolean   rxLinkUp;
  /*! Reports the completion progress of the system interface's
      autonegotiation.
      Compatibility: 28nm PHYs only.*/
 AQ_API_SysIntfAutonegStatus   sysIntfAutonegStatus;
} AQ_API_ConnectionStatus;    


#define AQ_RET_LOOPBACK_BAD_ENTRY_STATE 214

#endif

#define PHY_AQR40X_MAX_NUM_PHY_IDS 12
phy_handle_t* phy_aqr40x_dev_register(phy_info_t* pphy_info);

#endif /* !__PHY_aqr40x_H__ */

