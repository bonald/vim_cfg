/****************************************************************************
* $Id$
*  Centec PHY information related global MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : ychen
* Date          : 2010-07-16 
* Reason        : First Create.
****************************************************************************/

#ifndef __GLB_PHY_DEFINE_H__
#define __GLB_PHY_DEFINE_H__

#define PHY_CONTROL_SW_RESET                   (1U << 15)

#define PORT_SUPPORT_SPEED_100M    1<<0
#define PORT_SUPPORT_SPEED_1G      1<<1
#define PORT_SUPPORT_SPEED_10G     1<<2
#define PORT_SUPPORT_SPEED_40G     1<<3
#define PORT_SUPPORT_SPEED_100G    1<<4
#define PORT_SUPPORT_SPEED_MASK 0x1f

enum port_media_type_s
{
    PORT_MEDIA_TYPE_DEFAULT,
    PORT_MEDIA_TYPE_SFP,
    PORT_MEDIA_TYPE_QSFP,
    PORT_MEDIA_TYPE_MAX
};
typedef enum port_media_type_s port_media_type_t;

enum port_numchg_type_e
{
    PORT_NUMCHG_TYPE_NONE = 0,
    PORT_NUMCHG_TYPE_10G = 1,
    PORT_NUMCHG_TYPE_40G = 2,
    PORT_NUMCHG_TYPE_1G = 3,
    PORT_NUMCHG_TYPE_25G = 4,
    PORT_NUMCHG_TYPE_50G = 5,
    PORT_NUMCHG_TYPE_MAX /* for bug 51475 */
};
typedef enum port_numchg_type_e port_numchg_type_t;

#define PHY_CTL_REG             0x0          /*bit 12:auto; 6,13: 10M-0,100M-1,1000M-2; 8:Full-1,Half-0*/
#define PHY_STATUS_REG          0x1
#define PHY_IDENTIFIER1_REG     0x2
#define PHY_IDENTIFIER2_REG     0x3
#define AUTO_NEGO_ADVERTI_REG   0x4         /*bit 8,7,6,5: 100F,100H,10F,10H*/
#define AUTO_NEGO_LP_ADVERTI_REG   0x5
#define DIAG_1G_CTL_REG         0x9         /*bit 9,8 : 1000F, 1000H*/
#define PHY_INT_STATUS_REG      0x1a

#define PHY_CTL_SW_RESET (1U<<15)

/* Modified by liuht to support eee for bug 28298, 2014-05-16 */
/* bit0 ~ bit7 : eee local capabilities
 * bit8 ~ bit15: eee status
 * bit16~ bit23: eee link partner capabilities
 * bit24~ bit31: reserve
*/

#define GLB_PHY_EEE_LOCAL_CAPABILITY_10GBASE_KR  (1<<6)
#define GLB_PHY_EEE_LOCAL_CAPABILITY_10GBASE_KX4 (1<<5)
#define GLB_PHY_EEE_LOCAL_CAPABILITY_1000BASE_KX (1<<4)
#define GLB_PHY_EEE_LOCAL_CAPABILITY_10GBASE_T   (1<<3)
#define GLB_PHY_EEE_LOCAL_CAPABILITY_1000BASE_T  (1<<2)
#define GLB_PHY_EEE_LOCAL_CAPABILITY_100BASE_TX  (1<<1)

#if 0
#define GLB_PHY_EEE_LOCAL_CAPABILITY_100BASE_TX (1<<0)
#define GLB_PHY_EEE_LOCAL_CAPABILITY_1000BASE_T (1<<1)
#endif
#define GLB_PHY_EEE_LOCAL_TX_LPI_STATUS         (1<<8)
#define GLB_PHY_EEE_LOCAL_RX_LPI_STATUS         (1<<9)
#define GLB_PHY_EEE_LOCAL_LINK_STATUS           (1<<10)

#define GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_100BASE_TX (1<<16)

#define GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_10GBASE_KR  (1<<22)
#define GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_10GBASE_KX4 (1<<21)
#define GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_1000BASE_KX (1<<20)
#define GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_10GBASE_T   (1<<19)
#define GLB_PHY_EEE_LINK_PARTNER_CAPABILITY_1000BASE_T  (1<<18)
/* End of liuht modified */

enum glb_phy_media_e
{
    GLB_MEDIA_100M = 0x0,
    GLB_MEDIA_1G,
    GLB_MEDIA_10G,
    GLB_IFNAME_MEDIA_MAX,
};
typedef enum glb_phy_media_e glb_phy_media_t;

/*need to update cdb_ds.c at same time*/
enum glb_phy_type_e
{
    GLB_PHY_TYPE_UNKNOWN = 0x0,
    GLB_PHY_TYPE_1000BASE_T,    
    GLB_PHY_TYPE_10BASE_T,
    GLB_PHY_TYPE_100BASE_TX,
    GLB_PHY_TYPE_BASE_PX,
    GLB_PHY_TYPE_BASE_BX10,    
    GLB_PHY_TYPE_100BASE_FX,
    GLB_PHY_TYPE_100BASE_LX,
    GLB_PHY_TYPE_1000BASE_T_SFP,
    GLB_PHY_TYPE_1000BASE_CX,
    GLB_PHY_TYPE_1000BASE_LX,
    GLB_PHY_TYPE_1000BASE_SX,
    GLB_PHY_TYPE_1000BASE_ZX, /*add by qianj for bug 44540,should be in front of 1000BASE_SX.*/
    GLB_PHY_TYPE_10GBASE_ZR,
    GLB_PHY_TYPE_10GBASE_ER,
    GLB_PHY_TYPE_10GBASE_LRM,
    GLB_PHY_TYPE_10GBASE_LR,
    GLB_PHY_TYPE_10GBASE_SR,
    GLB_PHY_TYPE_10GBASE_PASSIVE_COPPER, /*Modify by jqiu for bug 15855. 2011-08-25*/
    GLB_PHY_TYPE_10GBASE_ACTIVE_COPPER,
    GLB_PHY_TYPE_40GBASE_CR4,
    GLB_PHY_TYPE_40GBASE_SR4,
    GLB_PHY_TYPE_40GBASE_LR4,
    GLB_PHY_TYPE_40GBASE_XLPPI,
    GLB_PHY_TYPE_100G_AOC,
    GLB_PHY_TYPE_100GBASE_SR4,
    GLB_PHY_TYPE_100GBASE_LR4,
    GLB_PHY_TYPE_100GBASE_ER4,
    GLB_PHY_TYPE_100GBASE_SR10,
    GLB_PHY_TYPE_100G_CWDM4_MSA_FEC,
    GLB_PHY_TYPE_100G_PSM4_SMF,
    GLB_PHY_TYPE_100G_ACC,
    GLB_PHY_TYPE_100G_CWDM4_MSA_NOFEC,
    GLB_PHY_TYPE_100GBASE_CR4,
    GLB_PHY_TYPE_40GBASE_ER4,
    GLB_PHY_TYPE_4X10GBASE_SR,
    GLB_PHY_TYPE_40G_PSM4_SMF,
    GLB_PHY_TYPE_10GBASE_T,
    GLB_PHY_TYPE_2G5BASE_T,
    GLB_PHY_TYPE_5GBASE_T,
    GLB_PHY_TYPE_10GBASE_KR,
    GLB_PHY_TYPE_40GBASE_KR4,
    GLB_PHY_TYPE_100GBASE_KR4,
    GLB_PHY_TYPE_25GAUI_C2M_AOC_BER_L,
    GLB_PHY_TYPE_25GAUI_C2M_AOC_BER_H,
    GLB_PHY_TYPE_25GBASE_SR,
    GLB_PHY_TYPE_25GBASE_LR,
    GLB_PHY_TYPE_25GBASE_ER,
    GLB_PHY_TYPE_25GAUI_C2M_ACC_BER_L,
    GLB_PHY_TYPE_25GAUI_C2M_ACC_BER_H,
    GLB_PHY_TYPE_25GBASE_CR_L,
    GLB_PHY_TYPE_25GBASE_CR_S,
    GLB_PHY_TYPE_25GBASE_CR_N,
    GLB_PHY_TYPE_25GBASE_PASSIVE_CABLE,
    GLB_PHY_TYPE_MAX,
};
typedef enum glb_phy_type_e glb_phy_type_t;

/* MAC interface */
enum glb_phy_mac_interface_e
{
    GLB_MAC_INTERFACE_NONE = -1,
    GLB_MAC_INTERFACE_SGMII = 0, /*default*/
    GLB_MAC_INTERFACE_SERDES,
    GLB_MAC_INTERFACE_QSGMII,
    GLB_MAC_INTERFACE_RGMII,
    GLB_MAC_INTERFACE_MAX,
};
typedef enum glb_phy_mac_interface_e glb_mac_interface_t;

/* Port media */
enum glb_port_media_e
{
    GLB_PORT_TYPE_AUTO,
    GLB_PORT_TYPE_COPPER,  /*twisted pair */
    GLB_PORT_TYPE_FIBER,   /* optical*/
    GLB_PORT_TYPE_SGMII,   /* sgmii type, use for sgmii passthrough mode. bug31199*/
    GLB_PORT_MEDIA_MAX
};
typedef enum glb_port_media_e glb_port_media_t;
typedef enum glb_port_media_e glb_port_interface_t;
enum glb_port_unidir_e
{
    GLB_PORT_UNIDIR_DISABLE, /* normal mode */
    GLB_PORT_UNIDIR_ENABLE,  /* both rx/tx enabled */
    GLB_PORT_UNIDIR_RX_ONLY, /* only rx enabled */
    GLB_PORT_UNIDIR_TX_ONLY, /* only tx enabled, but now is same with GLB_PORT_UNIDIR_ENABLE */
    GLB_PORT_UNIDIR_MAX
};
typedef enum glb_port_unidir_e glb_port_unidir_t;


enum glb_vtss_phy_recov_clk_e
{
    GLB_VTSS_PHY_RECOV_CLK1,   /**< RCVRD_CLK1 */
    GLB_VTSS_PHY_RECOV_CLK2,   /**< RCVRD_CLK2 */
    GLB_VTSS_PHY_RECOV_MAX
};
typedef enum glb_vtss_phy_recov_clk_e glb_vtss_phy_recov_clk_t;

/** \brief PHY clock sources */
enum glb_vtss_phy_clk_source_e
{
    GLB_VTSS_PHY_CLK_DISABLED, /**< Recovered Clock Disable */
    GLB_VTSS_PHY_SERDES_MEDIA, /**< SerDes PHY */
    GLB_VTSS_PHY_COPPER_MEDIA, /**< Copper PHY */
    GLB_VTSS_PHY_TCLK_OUT,     /**< Transmitter TCLK */
    GLB_VTSS_PHY_LOCAL_XTAL,    /**< Local XTAL */
    GLB_VTSS_PHY_CLK_MAX
};
typedef enum glb_vtss_phy_clk_source_e glb_vtss_phy_clk_source_t;

/** \brief PHY clock frequencies */
enum glb_vtss_phy_freq_e
{
    GLB_VTSS_PHY_FREQ_25M,  /**< 25 MHz */
    GLB_VTSS_PHY_FREQ_125M, /**< 125 MHz */
    GLB_VTSS_PHY_FREQ_3125M, /**< 31.25 MHz This is only valid on ATOM family - NOT Enzo*/
    GLB_VTSS_PHY_FREQ_MAX,
};
typedef enum glb_vtss_phy_freq_e glb_vtss_phy_freq_t;

/** \brief PHY clock squelch levels */
enum glb_vtss_phy_clk_squelch_e
{
    GLB_VTSS_PHY_CLK_SQUELCH_MAX,
    GLB_VTSS_PHY_CLK_SQUELCH_MED,
    GLB_VTSS_PHY_CLK_SQUELCH_MIN,
    GLB_VTSS_PHY_CLK_SQUELCH_NONE
};
typedef enum glb_vtss_phy_clk_squelch_e glb_vtss_phy_clk_squelch_t;

/** \brief PHY clock configuration */
struct glb_vtss_phy_clock_conf_s
{
    glb_vtss_phy_recov_clk_t    phy_clk_port;
    glb_vtss_phy_clk_source_t   src;     /**< Clock source */
    glb_vtss_phy_freq_t         freq;    /**< Clock requency */
    glb_vtss_phy_clk_squelch_t  squelch; /**< Clock squelsh level */
};
typedef struct glb_vtss_phy_clock_conf_s glb_vtss_phy_clock_conf_t;

/* Indicate phy port initialization sequence */
enum glb_port_init_seq_e
{
    GLB_NO_INIT_FLAG = 0,
    GLB_PRE_INIT_FLAG,
    GLB_POST_INIT_FLAG,
    GLB_DUAL_INIT_FLAG,
    GLB_MAX_INIT_FLAG,
};
typedef enum glb_port_init_seq_e glb_port_init_seq_t;

/*PHY real media interface type*/
enum glb_media_interface_e
{
    GLB_MEDIA_RJ45, /*Cat5*/
    GLB_MEDIA_1000BX, /* 1000BASE-X*/
    GLB_MEDIA_XFP,
    GLB_MEDIA_SFP_PLUS,
    GLB_MEDIA_PASS_THROUGH, /*SerDes(Passthrough) */
    GLB_MEDIA_FI_100FX, /* 100FX*/
    GLB_MEDIA_AMS_START,        /* kcao, should be continuous */
    GLB_MEDIA_AMS_CU_PASSTHRU,     /*Cat5 or SerDes(Passthrough), cat5 prefer.*/
    GLB_MEDIA_AMS_FI_PASSTHRU, /*Cat5 or SerDes(Passthrough), SerDes prefer.*/
    GLB_MEDIA_AMS_CU_1000BX, /*Cat5 or 1000BASE-X, cat5 prefer.*/
    GLB_MEDIA_AMS_FI_1000BX, /*Cat5 or 1000BASE-X, 1000BASE-X prefer.*/
    GLB_MEDIA_AMS_CU_100FX, /*Cat5 or 100FX, cat5 prefer.*/
    GLB_MEDIA_AMS_FI_100FX, /*Cat5 or 100FX, 100FX prefer.*/
    GLB_MEDIA_QSFP_PLUS,
    GLB_MEDIA_MAX
};
typedef enum glb_media_interface_e glb_media_interface_t;
typedef enum glb_media_interface_e glb_media_t;

/*duplex*/
enum glb_port_duplex_e
{
    GLB_DUPLEX_AUTO,
    GLB_DUPLEX_HALF,
    GLB_DUPLEX_FULL,
    GLB_PORT_DUPLEX_MAX
};
typedef enum glb_port_duplex_e glb_port_duplex_t;

/* Modified by liuht for bug 28155, 2014-04-17 */
/* Support force master or slave mode */
enum glb_port_master_slave_e
{
    GLB_MODE_AUTO = 0,
    GLB_MODE_MASTER,
    GLB_MODE_SLAVE,
    GLB_MODE_MAX
};
typedef enum glb_port_master_slave_e glb_port_master_slave_t;
/* End of liuht modified */

/* speed*/
enum glb_port_speed_e
{
    GLB_SPEED_AUTO,
    GLB_SPEED_10M,
    GLB_SPEED_100M,
    GLB_SPEED_1G,
    GLB_SPEED_10G,
    GLB_SPEED_40G,
    GLB_SPEED_100G,
    GLB_SPEED_5G,
    GLB_SPEED_2_5G,
    GLB_SPEED_25G,
    GLB_SPEED_50G,
    GLB_PORT_SPEED_MAX
};
typedef enum glb_port_speed_e glb_port_speed_t;

#define GLB_SPEED_MAX   GLB_PORT_SPEED_MAX
#define GLB_DUPLEX_MAX   GLB_PORT_DUPLEX_MAX

#define GLB_PHY_PORT_IS_100G(SUPPORT_SPEED)   ((SUPPORT_SPEED) & GLB_SUPPORT_SPEED_100G) ? TRUE : FALSE
#define GLB_PHY_PORT_IS_40G(SUPPORT_SPEED)    ((SUPPORT_SPEED) & GLB_SUPPORT_SPEED_40G) ? TRUE : FALSE
#define GLB_PHY_PORT_IS_10G(SUPPORT_SPEED)    ((SUPPORT_SPEED) & GLB_SUPPORT_SPEED_10G) ? TRUE : FALSE
#define GLB_PHY_PORT_IS_50G(SUPPORT_SPEED)    ((SUPPORT_SPEED) & GLB_SUPPORT_SPEED_50G) ? TRUE : FALSE
#define GLB_PHY_PORT_IS_25G(SUPPORT_SPEED)    ((SUPPORT_SPEED) & GLB_SUPPORT_SPEED_25G) ? TRUE : FALSE
#define GLB_PHY_PORT_IS_5G(SUPPORT_SPEED)     ((SUPPORT_SPEED) & GLB_SUPPORT_SPEED_5G) ? TRUE : FALSE
#define GLB_PHY_PORT_IS_2_5G(SUPPORT_SPEED)   ((SUPPORT_SPEED) & GLB_SUPPORT_SPEED_2_5G) ? TRUE : FALSE
#define GLB_PHY_PORT_IS_1G(SUPPORT_SPEED)     ((SUPPORT_SPEED) & GLB_SUPPORT_SPEED_1G) ? TRUE : FALSE
#define GLB_PHY_PORT_IS_100M(SUPPORT_SPEED)   ((SUPPORT_SPEED) & GLB_SUPPORT_SPEED_100M) ? TRUE : FALSE

/* Loopback*/
enum glb_lb_phy_e
{
    GLB_LB_NONE,
    GLB_LB_PHY_FAR,
    GLB_LB_PHY_NEAR,   
    GLB_LB_PHY_NEAR_DIAG, /* added by liuht for bootup diag for bug 27508, 2014-06-30 */
    GLB_LB_PHY_LOOPBACK,
    GLB_LB_PORT,
    GLB_LB_PORT_SWAP,
    GLB_LB_MAX
};
typedef enum glb_lb_phy_e glb_lb_phy_t;

/* flow control, need to keep same as nsm_if_direction. */
enum glb_flowctrl_direction
{
    GLB_FLOWCTRL_RECV = 0,
    GLB_FLOWCTRL_SEND
};
/* priority flow control. */
enum glb_pfc_flowctrl
{
    GLB_PFC_FLOWCTRL_OFF = 0,
    GLB_PFC_FLOWCTRL_ON,
    GLB_PFC_FLOWCTRL_MAX,
};

#define GLB_FLOWCTRL_DISABLE 0
#define GLB_FLOWCTRL_ENABLE 1

struct glb_port_flowctrl_s
{
    uint8 send;
    uint8 recv;
};
typedef struct glb_port_flowctrl_s glb_port_flowctrl_t;

/* Modified by liuht to support eee for bug 28298, 2014-05-16 */
enum glb_eee_enable_e
{
    GLB_EEE_ENABLE = 1,
    GLB_EEE_DISABLE,
    GLB_EEE_MAX
};
typedef enum glb_eee_enable_e glb_eee_enable_t;
/* End of liuht modified */

/*bug30363 jqiu add 2014-10-23 for customer request*/
typedef enum phy_signal_test_mode_e
{
    PHY_SIG_TEST_MODE_NORMAL,  /*normal operation mode*/
    PHY_SIG_TEST_MODE_1, /*Transmit waveform test*/
    PHY_SIG_TEST_MODE_2, /*Transmit jitter test as master*/
    PHY_SIG_TEST_MODE_3, /*Transmit jitter test as slave*/
    PHY_SIG_TEST_MODE_PRBS9, /*PRBS9 test*/
    PHY_SIG_TEST_MODE_PRBS31, /*PRBS31 test*/
}phy_signal_test_mode_t;


/* Modified by liuht to support eee for bug 28298, 2014-05-16 */
struct glb_port_eee_status_s
{
    uint16 wake_error_count;
    /* bit0 ~ bit7 : eee local capabilities
     * bit8 ~ bit15: eee status
     * bit16~ bit23: eee link partner capabilities
     * bit24~ bit31: reserve */
    uint32 status; 
};
typedef struct glb_port_eee_status_s glb_port_eee_status_t;
/* End of liuht modified */

struct glb_phy_state_s
{
    uint32 link_up;    /* if non-zero, link is up; otherwise link is down */
    glb_port_duplex_t duplex;     /* if PAL_PHY_DUPLEX_AUTO, auto-neg not completed */
    glb_port_speed_t speed;      /* if PAL_PHY_SPEED_AUTO, auto-neg not completed */
    glb_port_flowctrl_t  flowctrl; /* flow control */
    glb_port_interface_t linkup_media; /* media type:auto, copper, fiber */
    glb_port_master_slave_t master_slave; /* Modified by liuht for bug 28155, 2014-04-17 */
#if 0 
    /*Bug36946. when GB/GG force signal detect set to 0, mac enable/disable needn't depend on los.*/
    /*Signal detect, 0 means not detect signal, 1 means detect signal, 2 means force signal detect*/
    uint8 sig_det;
#endif
    /* Modified by liuht to support eee for bug 28298, 2014-05-16 */
    glb_eee_enable_t eee_enable;
    glb_port_eee_status_t eee_status;
    /* End of liuht modified */
    /* Modified by liuht for bug 29005, 2014-06-17 */
    uint32 port_status_change;
    /* End of liuht modified */
    uint8 cl73_status;   /* indicate clause73 status. 0:disable; other:enable*/
    uint32 fec_status;  /* no auto status*/
    uint32 cl73_ability;    
};
typedef struct glb_phy_state_s glb_phy_state_t;

struct glb_port_ffe_s
{
    uint8 config[4];
    uint8 c0[4];
    uint8 c1[4];
    uint8 c2[4];
    uint8 c3[4];
    uint8 channel;
};
typedef struct glb_port_ffe_s glb_port_ffe_t;

struct glb_port_cfg_s
{
    uint32             enable;   
    glb_lb_phy_t       loopback;
    glb_port_duplex_t  duplex;
    glb_port_speed_t   speed;
    glb_port_master_slave_t   master_slave; /* Modified by liuht for bug 28155, 2014-04-17 */
    glb_port_flowctrl_t  flowctrl;
    glb_port_ffe_t              ffe;/* modified by tongzb 2018-11-19 */
    glb_port_interface_t  media; /* port type:auto, copper, fiber*/
    glb_port_unidir_t     unidir;
    uint32               syncE_enable;
    uint32               stack_trunk;
};
typedef struct glb_port_cfg_s glb_port_cfg_t;
struct glb_summer_time_s {
int32 tm_sec;
int32 tm_min;
int32 tm_hour;
int32 tm_mday;
int32 tm_mon;
int32 tm_year;
int32 tm_wday;
int32 tm_yday;
int32 tm_isdst;
int64 tm_gmtoff;
};
typedef struct glb_summer_time_s summer_time_t;
struct glb_which_summer_time_s
{
    int32 wday_tm_sec;  /* seconds */
    int32 wday_tm_min;  /* minutes */
    int32 wday_tm_hour; /* hours */
    int32 wday_tm_which;   /* enum whichWday, 1: the first wday_tm_wday ... 4: the fourth wday_tm_wday 5:the last wday_tm_wday */
    int32 wday_tm_wday; /* enum weekday, day of the week */
    int32 wday_tm_mon;  /* month <1-12> */
    int32 wday_tm_year; /* year */
};
typedef struct glb_which_summer_time_s which_summer_time_t;

/*link*/
enum glb_port_link_e
{    
    GLB_LINK_DOWN = 0,
    GLB_LINK_UP,
    GLB_LINK_MAX
};
typedef enum glb_port_link_e glb_port_link_t;

/*VCT state. Bug25277*/
enum glb_port_vct_state_e
{
    GLB_VCT_WAIT = -2,
    GLB_VCT_ERROR = -1,
    GLB_VCT_OK = 0,        
};
typedef enum glb_port_vct_state_e glb_port_vct_state_t;

/* added by tongzb for bug 43279 */
#define FIBER_TRANS_TYPE_LEN 32
#define FIBER_COMPLIANCE_CODE_LEN 8
#define FIBER_VENDOR_NAME_LEN 16
#define FIBER_VENDOR_OUI_LEN 3
#define FIBER_VENDOR_PN_LEN 16
#define FIBER_VENDOR_REV_LEN 4
#define FIBER_VENDOR_SN_LEN 16

typedef struct date_code
{
    uint8 year[3];
    uint8 month[3];
    uint8 day[3];
    uint8 lot[3];
} date_code_t;

typedef enum
{
    //FIBER_CURRENT_VALUE,
    FIBER_HIGH_ALARM_VALUE,
    FIBER_LOW_ALARM_VALUE,
    FIBER_HIGH_WARN_VALUE,
    FIBER_LOW_WARN_VALUE,
    FIBER_VAL_TYPE_MAX,
} fiber_value_type_t;

typedef enum
{
    FIBER_CURRENT_VALUE1,
    FIBER_CURRENT_VALUE2,
    FIBER_CURRENT_VALUE3,
    FIBER_CURRENT_VALUE4,
    FIBER_VAL_TYPE_MAX2,
} fiber_value_type2_t;

typedef enum
{   
    TUNE_NONE_MODE,
    FREQ_MODE,
    WAVELENGTH_MODE,
    CHANNEL_MODE,
    WAVELENGTH_CHANNEL_MODE,
} fiber_tune_mode_t;

typedef struct fiber_cal_const
{
    uint8 rx_pwr4[4];
    uint8 rx_pwr3[4];
    uint8 rx_pwr2[4];
    uint8 rx_pwr1[4];
    uint8 rx_pwr0[4];
    uint8 tx_i_slope[2];
    uint8 tx_i_offset[2];
    uint8 tx_pwr_slope[2];
    uint8 tx_pwr_offset[2];
    uint8 t_slope[2];
    uint8 t_offset[2];
    uint8 v_slope[2];
    uint8 v_offset[2];
} fiber_cal_const_t;

typedef enum
{
    E_FIBER_SFP = 1,
    E_FIBER_XFP,           
    E_FIBER_SFP_P,
    E_FIBER_QSFP_P
} fiber_device_t;

typedef struct fiber_info
{
    uint8 fiber_type;   /*glb_phy_type_t*/
    uint8 fiber_flag;/* for bug 45607 */
    uint8 is_copper;/* for bug 45607 */
    uint8 speed;        /*SFP speed = speed * 100Mbit/sec */
    char name[FIBER_VENDOR_NAME_LEN+1];         /*SFP vendor name (ASCII)*/
    date_code_t date_code;        /*Vendor's manufacturing date code*/
    char oui[FIBER_VENDOR_OUI_LEN+1];           /*SFP vendor IEEE company ID*/
    char pn[FIBER_VENDOR_PN_LEN+1];            /*Part number provided by SFP vendor (ASCII)*/
    char rev[FIBER_VENDOR_REV_LEN+1];          /*Revision level for part number provided by vendor (ASCII)*/
    char sn[FIBER_VENDOR_SN_LEN+1];            /*Serial number provided by vendor (ASCII)*/
    uint8 compliance_code[FIBER_COMPLIANCE_CODE_LEN];      /* complianceCode */
    uint8 ext_compliance_code;
    uint8 wavelength[2];          /*output wavelength*/
    uint8 length[5];                 /* length */     
    uint8 ddm_support;            /*is support DDM?*/
    uint8 externally_calibrated;  /*if support DDM, use internally calibrated or externally calibrated?*/
    fiber_cal_const_t cal_const;         /*calculated constant, used for externally calibrated*/
    double        tx_pwr[FIBER_VAL_TYPE_MAX]; //for threshold value
    double        rx_pwr[FIBER_VAL_TYPE_MAX];
    double        tmpr[FIBER_VAL_TYPE_MAX];
    double        voltage[FIBER_VAL_TYPE_MAX];
    double        bias[FIBER_VAL_TYPE_MAX];
    double        tx_pwr2[FIBER_VAL_TYPE_MAX2]; //for current value
    double        rx_pwr2[FIBER_VAL_TYPE_MAX2];
    double        tmpr2[FIBER_VAL_TYPE_MAX2];
    double        voltage2[FIBER_VAL_TYPE_MAX2];
    double        bias2[FIBER_VAL_TYPE_MAX2];
    uint32  alarm_flag[FIBER_VAL_TYPE_MAX2]; /*store alarm info*/
    uint32  warn_flag[FIBER_VAL_TYPE_MAX2]; /*store warn info*/
    uint8 channel_num;
    uint8 channel_idx;
    uint32 update_cnt;
    uint32 updated;
    uint8 tunable_support;/* for bug 43937 support tunble DWDM fiber*/
    fiber_tune_mode_t tune_mode;
    double first_freq; /* units of THz */
    double last_freq;
    double step_freq;
    double tunable_wavelength;
    uint8 dither_support;
    uint32 wave_channels;
    uint32 wave_channel_idx;
    uint8 br_max; /*refer to SFF-8472's A0H,byte 66*/
} fiber_info_t;
/* added end by tongzb for bug 43279 */

#endif
