#ifndef __PHY_DRV_H__
#define __PHY_DRV_H__

#include "sal_common.h"
#include "sal_mutex.h"
#include "glb_hw_define.h"
#include "glb_phy_define.h"
#include "mdio.h"
#if 0
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#endif
/****************************************************
*
*       MACRO Define
*
*****************************************************/


/****************************************************
*
*   TYPES Define
*
****************************************************/
#define XGPHY_PMA_DEV                   0x1
#define XGPHY_WIS_DEV                   0x2
#define XGPHY_PCS_DEV                   0x3
#define XGPHY_XS_DEV                    0x4

#define XGPHY_REG_CONTROL_REG            0x0
#define XGPHY_REG_STATUS_REG            0x1

/* PHY register pages */
#define PHY_PAGE_STANDARD   0x0000 /**< Standard registers */
#define PHY_PAGE_EXTENDED   0x0001 /**< Extended registers */
#define PHY_PAGE_EXTENDED_2 0x0002 /**< Extended registers - page 2 */
#define PHY_PAGE_EXTENDED_3 0x0003 /**< Extended registers - page 3 */
#define PHY_PAGE_EXTENDED_4 0x0004 /**< Extended registers - page 4 */
#define PHY_PAGE_GPIO       0x0010 /**< GPIO registers */

typedef enum
{
    PORT_PHY_VSC8211,
    PORT_PHY_VSC8x58,
    PORT_PHY_VSC8488,
    PORT_PHY_VSC8512,
    PORT_PHY_NULL,
    PORT_PHY_LSI1082,
    PORT_PHY_VSC8504,
    PORT_PHY_SFP, /*bug23865*/
    PORT_PHY_AQR405,
    PORT_PHY_AQR407,
    PORT_PHY_AQR408,
    PORT_PHY_AQR409,
    PORT_PHY_AQR411,
    PORT_PHY_MV88E154X,
    PORT_PHY_MV88E1680,
    PORT_PHY_MV88E1548,
    MAX_PHY_TYPE,
}phy_device_type_t;

/* Work mode */
typedef enum
{
    PHY_WORK_MODE_NORMAL=0,
    PHY_WORK_MODE_PASSTHROUGH,
}phy_work_mode_t;

/* enable auto negotiation */
typedef enum
{
    PHY_DISABLE_AUTO_NEGO = 0,
    PHY_ENABLE_AUTO_NEGO,
}phy_enable_auto_nego_t;

/* phy firmware load mode, added by tongzb for aqr phy */
typedef enum
{
    PHY_FW_NONE = 0, /* phy need't load the firmware */
    PHY_FW_MDIO,  /* load firmware through mdio, each mdio need load once */
    PHY_FW_FLASH, /* load firmware through flash, just need load the firmware into the flash once */
}phy_fw_load_mode_t;

/* For the purpose of read and write the PHY register */
typedef struct phyreg_param_s 
{
    uint8 dev_no;           /* dev type of 10G phy, for 1G phy it's not used*/
    union {
        uint8 regaddr8;     /* 1G phy reg 8bit width */
        uint16 regaddr16;   /* 10G phy 16bit width*/
    } dat;
} phyreg_param_t;

struct flowctrl_adv_ability_s
{
     uint8 asymmetric_pause; /*if 1 enable pause,0 disable pause*/
     uint8 symmetric_pause;  /*if 1 enable pause,0 disable pause*/
};
typedef struct flowctrl_adv_ability_s flowctrl_adv_ability_t;


/* VeriPHY result */
typedef struct {
    int8                      link;      /* Link status */
    glb_pair_state_t          status[4]; /* Status, pair A-D (0-3) */
    uint8                     length[4]; /* Length (meters), pair A-D (0-3) */
} phy_veriphy_result_t;


typedef struct
{
    /* Variables common to all tasks */
    uint8 task_state;   /* 0x00 ==> idle (but still serviced) */
                                /* 0x01 - 0x7f ==> task-defined, */
                                /* bit [7]: 1 ==> abort requested */

    /* VeriPHY public variables */
    uint8 flags;        /* bit [7:6] = VeriPHY operating mode */
                                /*        00 ==> full VeriPHY algorithm */
                                /*        01 ==> anomaly-search only */
                                /*        10 ==> anomaly-search w/o x-pair only */
                                /*        11 ==> reserved */
                                /* bit [5:4] = unreliablePtr (if bit [3] == 1) */
                                /* bit [3]   = unreliablePtr presence flag */
                                /* bit [2]   = getCableLength done flag */
                                /* bit [1]   = valid */
                                /* bit [0]   = linkup-mode */
    uint8  stat[4];      /* status for pairs A-D (0-3), 4-bit unsigned number */
                                /*        most signiciant 4-bits represents prev. status */
    uint8 loc[4];       /* length/fault location for pairs A-D (0-3), 8-bit unsgn */

    /* VeriPHY private variables */
    int8 subchan;
    int8 nc;
    uint8 numCoeffs;
    uint8 firstCoeff;
    int16 strength[4];          /* fault strength for pairs A-D (0-3), 14-bit signed int. */
    int16 thresh[4];            /* threshold (used in different places), 13-bit unsgn */
    int16 log2VGAx256;          /* log2(VGA gain scalefactor)*256 (0 for link-down case) */
    int8  signFlip;      /* count used to improve location accuracy */
    int32 tr_raw0188;            /* wastes one byte */
    int16 maxAbsCoeff;
    int16 coeff[12]; 
    uint16 saveMediaSelect;   /* store previous media */ 
    phy_veriphy_result_t result; /* VCT test result */
    uint8 count;             /* Timeout count*/
    int32 ret;                /* store the status func return result */
} phy_veriphy_task_t;


typedef struct
{
    int action;             /* if 1 link down yo link up; 0 link up to link down, -1, mean link do not change */
    int duplex_change;      /* if 1, duplex change */
    int speed_change;       /* if 1, speed change */
    int flowctrl_change;   /* if 1, flow control change */
    int media_change;       /* if 1, media change */
    int master_slave_change;/* if 0 mode-auto; 1 mode-master; 2 mode-slave for bug 28155 */
    /* Modified by liuht to support eee for bug 28298, 2014-05-16 */
    int eee_status_change;  /* if 1, eee status change */
    /* Modified by liuht for bug 29005, 2014-06-17 */
    int link_change;
    /* End of liuht modified */
    int cl73_ability_change;
    int fec_status_change;
} phy_state_change_t;

typedef struct
{
    int8 enable;   /*phy enable flag 1: enable; 0:disable*/
    int8 syncE_enable; /* phy syncE enable flag 1: enable; 0:disable*/
    glb_media_interface_t  media_type;    /*Auto default*/
    glb_mac_interface_t mac_if;     /* SGMII default */
    glb_port_speed_t speed;
    glb_port_duplex_t duplex;
    glb_port_master_slave_t master_slave; /* Modified by liuht for bug 28155, 2014-04-17 */
    glb_port_flowctrl_t flowctrl;     /* flow cntrol, default is disable */
    flowctrl_adv_ability_t flowctrl_ability; /* phy flowctrl ability, default is disable */
    glb_lb_phy_t    lb_mode;        /*loopback mode*/
    int32 mode;             /* phy work mode: normal or pass-through*/
    glb_port_init_seq_t phy_init_seq_flag; /* for vsc8512 phy, indicate pre-reset/reset/post-reset sequence */
    glb_slot_num_t slot_card_idx;        /* for GB Demo board, mark slot card location for each PHY chip */
    uint8 trace_length; /*for vsc8512 phy, indicate the trace length for different position.*/
    glb_vtss_phy_clock_conf_t phy_clk_conf;
    /* Modified by liuht to support eee for bug 28298, 2014-05-16 */
    glb_eee_enable_t eee_enable; /* enable or disable eee function */    
    /* End of liuht modified */
    uint8 cl73_enable;       /* enable when speed 40G, duplex auto*/
    uint32 cl73_speed_ability;     /*GG do CL73 802.3ap depend this ability. get ability from FIBER type. */
    uint32 fec_ability;  /* FEC ability */
    uint8 is_DAC;            /* fiber is DAC?*/
    glb_port_speed_t fiber_ability;/*1/10G:depend on fiber speed;40/100G:depend on fiber type*/    
    glb_port_unidir_t unidir;
    /* added by liuyang, 2016-5-3 */
    uint8 fec_en; /* 1 enable, otherwise 0 */
    uint32 link_tolerable_time;/* default is 0s, for no tolerable */
    uint8 bypass;
} phy_manage_info_t;

/*bug32103 jqiu 20150305. add lchip and serdes info*/
struct phy_port_info_s
{
    uint16 port_id; /* when multichip, port_id include chip info, so should be uint16 */
    uint8 lchip;
    uint8 serdes_id;
    uint8 serdes_num;   /* bug44010, support serdes loopback, qicx, 2017-05-15 */
};
typedef struct phy_port_info_s phy_port_info_t;

/* add phy_gpio_info struct for phy gpio init when board use phy combo mode, such as hf */
struct phy_gpio_info_s
{
    uint32 gpio_dir;
    uint32 sigdet_polarity; /* 1 is active low, 0 is active high */
};
typedef struct phy_gpio_info_s phy_gpio_info_t;

typedef struct
{
    sal_mutex_t* pmutex;
    phy_device_type_t phy_device_type;
    mdio_bus_type_t mdio_bus;
    uint32 base_addr;      /*mdio controller base addr*/     
    uint8 phy_addr;    /*the phy address of each port on one phy chip*/
    phy_port_info_t port_info;
    uint16 fiber_abs_addr;  /*fiber absent status register address. ctrl by phy GPIO*/
    uint16 fiber_txdis_addr; /* fiber tx disable register address, ctrl by phy GPIO*/
    uint8 port_num; /* port id, added by liuht for bug 25808 */
    uint8 gpio_init;        /*is GPIO need init*/
    phy_gpio_info_t gpio_info;  /* when GPIO need init, this info is need also */
    uint16 switch_chnl_id;  /*xgmac and 1g phy switch channel*/
    phy_manage_info_t phy_manage_info;
    glb_phy_state_t phy_stat_flag;
    uint8 vct_support;      /* 1 support, otherwise 0 */
    phy_veriphy_task_t *phy_veriphy_tsk;
    void *data;            /*currently, only used for l2switch internal phy, ex:vtss7390*/
    /*modified by jcao for bug 14399, 2011-03-31*/
    glb_phy_led_type_t led_type;
    glb_phy_led_freq_t led_freq;/* added by tongzb, for config led freq, 2017-01-06 */
    /* modified by qicx to get PHY revision, 2012-10-18 */
    uint8 phy_revision;
    uint16 part_num; /*phy part number*/
    void * next_phy_hdl; /* Pointer to next level phy handle. use for MAC+PHY+PHY mode. bug23865*/
    uint8 pair_reverse;/* MDI Reversed (ABCD -> DCBA) */
    uint8 zg_port_id;  /* 100G port id, the first 100G port is 1, and 0 means invalid */

    uint32 fiber_ability; /* speed ability from fiber type*/
    uint8 serdes_lb_en; /* For bug44010, qicx, 2017-05-23. Serdes loopback enable/disable flag, 
                                  1-->internal or external loopback enable;
                                  0-->no loopback */
    uint8 speed_ability; /* tongzb for aqr40x*/
    uint8 serdes_mode;/* tongzb for aqr40x*/
    phy_fw_load_mode_t fw_load_mode;/* tongzb for aqr40x*/
    firmware_ver_t fw_ver;/* tongzb for aqr40x*/
    uint8 fw_fix_port;/* tongzb for aqr40x, load firmware from flash*/
    uint8 fw_start_addr;/* tongzb for aqr40x, load firmware from mdio*/
    uint32 port_sum_num; /* tongzb for aqr40x, the total numbers of  all phy*/
    uint8 ports_per_mdio; /* port numbers per mdio bus */
    uint32 mdio_BC_addr; /* mdio broadcast address for gangload mode */
    char * firmware_name;
    uint8 hop_count;    /* para for load aqr phy load firmware from mdio */
    uint8 port_led_ctrl; /* for phy manual led, bit[7:4] is set which phy's LED pin to control the first led , and bit[3:0] for the second led */
} phy_info_t;

#ifndef MAD_DEV_DEFINED
typedef struct _MAD_DEV MAD_DEV;
#define MAD_DEV_DEFINED
#endif
#ifndef PHY_HANDLE_T_DEFINED
typedef struct phy_handle_s phy_handle_t;
#define PHY_HANDLE_T_DEFINED
#endif
struct phy_handle_s
{
    int32 (* phy_init)(phy_handle_t* );
    int32 (* phy_post_init)(phy_handle_t* );
    int32 (* config_enable)(phy_handle_t*, int8 );
    int32 (* config_speed)(phy_handle_t*, uint8 );
    int32 (* config_master_slave)(phy_handle_t*, uint8 );
    int32 (* config_duplex)(phy_handle_t*, uint8 );
    int32 (* config_loopback)(phy_handle_t*, uint8 );
    int32 (* config_medium)(phy_handle_t*, uint8, uint8 );
    int32 (* config_flowctrl)(phy_handle_t*, uint8, uint8 );
    int32 (* config_clock)(phy_handle_t*, glb_vtss_phy_clock_conf_t);
    int32 (* config_fec_en)(phy_handle_t*, uint8);
    int32 (* get_link_poll)(phy_handle_t*, glb_phy_state_t*, phy_state_change_t* );
    int32 (* get_link_interupt)(phy_handle_t*, glb_phy_state_t* , phy_state_change_t* );
    int32 (* get_cur_status)(phy_handle_t*, glb_phy_state_t*);
    int32 (* config_phy)(phy_handle_t*,phy_manage_info_t* );
    int32 (* reg_read)(phy_handle_t*, phyreg_param_t* , uint16*);
    int32 (* reg_write)(phy_handle_t*, phyreg_param_t*, uint16);
    int32 (* phy_set_vct_mod)(phy_handle_t* );
    int32 (* phy_get_vct_info)(phy_handle_t*, glb_port_cable_info_t* );
    int32 (* enable_syncE)(phy_handle_t*, int8);
    int32 (* mmd_reg_write_mask)(phy_handle_t*, uint16, uint16, uint16, uint16);
    int32 (* enable_eee)(phy_handle_t*, int8);
    int32 (* config_phy_sig_test_mode)(phy_handle_t*, uint8, uint8);/*bug30363 jqiu add 2014-10-23 for customer request*/
    int32 (* config_unidir)(phy_handle_t*, uint8);
    int32 (* config_prbs)(phy_handle_t* , uint8 ,uint8 ,uint8 *);
    int32(* config_led_mode)(phy_handle_t* , uint8 ,uint8); /*add by chenjr for bug 53838*/
#if (defined DUET2) || (defined TSINGMA)
    int32 (*get_autoneg_ability)(phy_handle_t*, uint32*);
#endif
    phy_info_t phy_info;
    mdio_handle_t *mdio_hdl;
    MAD_DEV   *mad_dev;
};


/***************************************************************************
 *  Name:       : phy_dev_register
 *  Purpose     : Create a phy driver handle;
 *  Input       : phy info, ex: device type,bus type,phy addr,base addr,
                  media type;
 *  Output      : N/A
 *  Return      : phy driver handle or NULL
 *  Note        : N/A
 **************************************************************************/
phy_handle_t* phy_dev_register(phy_info_t* pphy_info);
int32 phy_dev_unregister(phy_handle_t* phy_handle);

void flowctrl_admin_to_ability(glb_port_flowctrl_t* admin_flowctrl, flowctrl_adv_ability_t* ability);
void flowctrl_ability_to_admin(flowctrl_adv_ability_t* ability, glb_port_flowctrl_t* admin_flowctrl);


#endif /*__PHY_DRV_H__*/

