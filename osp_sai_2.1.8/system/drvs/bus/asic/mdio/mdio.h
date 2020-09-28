#ifndef __CTC_MDIO_H__
#define __CTC_MDIO_H__

#include "sal_common.h"
#include "glb_hw_define.h"

/****************************************************
*
*       MACRO Define
*
*****************************************************/
#define MDIO_TYPE_1G        0x0
#define MDIO_TYPE_10G       0x1

#define MDIO_ST_1G          0x1
#define MDIO_ST_10G         0x0

#define MDIO_OP_1G_READ     0x2
#define MDIO_OP_1G_WRITE    0x1

#define MDIO_OP_10G_ADDR    0x0
#define MDIO_OP_10G_WRITE   0x1
#define MDIO_OP_10G_POST    0x2
#define MDIO_OP_10G_READ    0x3

#define MDIO_DEVTYPE_10G_RESV   0x0
#define MDIO_DEVTYPE_10G_PMD_A  0x1
#define MDIO_DEVTYPE_10G_WIS    0x2
#define MDIO_DEVTYPE_10G_PCS    0x3
#define MDIO_DEVTYPE_10G_PHY_XS 0x4
#define MDIO_DEVTYPE_10G_DTE_XS 0x5

#define MDIO_OFF       0x04
#define MDC_OFF        0x00
#define MD_DIR         0x08
#define MD_CMD         0x20
#define MD_CMDDONE     0x0c
#define MD_READ        0x1c

/****************************************************
*
*   TYPES Define
*
****************************************************/
typedef struct mdiocmd_s
{
    uint32   reserved       :2;
    uint32   mdiocmd_st     :2;
    uint32   mdiocmd_op     :2;
    uint32   mdiocmd_prtad  :5;
    uint32   mdiocmd_devad  :5;
    uint32   mdiocmd_data   :16;
} mdiocmd_t;

typedef union mdio_cmd_u
{
   mdiocmd_t field;
   uint32 val;
} mdio_cmd_union_t;

typedef enum
{
    FPGA_PHY_1G_TYPE = 0,
    FPGA_PHY_10G_TYPE,
    ASIC_PHY_1G_TYPE,
    ASIC_PHY_10G_TYPE,
    CPU_PHY_1G_TYPE,
    ASIC_GB_PHY_1G_TYPE,
    ASIC_GB_PHY_10G_TYPE,
    ASIC_GB_PHY_10G_BY_1G_TYPE,
    ASIC_GG_PHY_1G_TYPE,
    ASIC_GG_PHY_10G_TYPE,
    ASIC_GG_PHY_1G_10G_TYPE, /*1G mdio access 10G phy*/
    ASIC_D2_PHY_1G_TYPE,
    ASIC_D2_PHY_10G_TYPE,
    ASIC_TM_PHY_1G_TYPE,
    ASIC_TM_PHY_10G_TYPE,
} mdio_bus_type_t;

typedef struct
{
    uint8 phy_addr;
    uint8 reg;      /* the offset of the phy internal address */ 
    uint16 val;     /* the value of the register */
}gephy_para_t;

typedef struct 
{
    uint8 phy_addr;
    uint16 reg;         /* the offset of the phy internal address */ 
    uint16 devno;       /* the device NO. of the xg phy*/
    uint16 val;         /* the value of the register */
} xgphy_para_t;

typedef union mdio_bus_para_s
{
    gephy_para_t gephy_para;    /*1000M phy para*/
    xgphy_para_t xgphy_para;    /*XG phy para*/

}mdio_bus_para_t;

/* for duet2 mdio support */
#define GLB_MIDO_BUS0    0x00
#define GLB_MIDO_BUS1    0x01
#define GLB_MIDO_BUS2    0x02
#define GLB_MIDO_BUS3    0x03
#define GLB_MIDO_BUS4    0x04
#define GLB_MIDO_BUS5    0x05
#define GLB_MIDO_BUS6    0x06
#define GLB_MIDO_BUS7    0x07

typedef struct mdio_info_s
{
    uint32 base_addr;
    mdio_bus_type_t bus_type;
    glb_slot_num_t slot_card_idx;        /* for GB Demo board, mark slot card location for each PHY chip */
}mdio_info_t;

typedef struct mdio_handle_s mdio_handle_t;

struct mdio_handle_s
{
    int32 (*read)(mdio_handle_t* , mdio_bus_para_t* );
    int32 (*write)(mdio_handle_t* , mdio_bus_para_t* );
    mdio_info_t data;
};

/***********************************************************
*
*   FUNCTION Define
*
***********************************************************/
mdio_handle_t* mdio_create_handle(mdio_info_t* );

#endif /*__CTC_MDIO_H__*/
