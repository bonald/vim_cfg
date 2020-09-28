
#ifndef __TBL_PORT_DEFINE_H__
#define __TBL_PORT_DEFINE_H__

/* TBL_PORT field defines */
typedef enum
{
    TBL_PORT_FLD_KEY                  = 0 ,  /* READ */
    TBL_PORT_FLD_SLOT_NO              = 1 ,  /* READ */
    TBL_PORT_FLD_PANEL_PORT_NO        = 2 ,  /* READ */
    TBL_PORT_FLD_PANEL_SUB_PORT_NO    = 3 ,  /* READ */
    TBL_PORT_FLD_PHY_TYPE             = 4 ,  /* READ */
    TBL_PORT_FLD_PHYINFO_FLAG         = 5 ,  /* READ */
    TBL_PORT_FLD_SPEED_CAPBILITY      = 6 ,  /* READ */
    TBL_PORT_FLD_LOGIC_PORT_ID        = 7 ,  /* READ */
    TBL_PORT_FLD_MEDIA                = 8 ,  /* RW */
    TBL_PORT_FLD_PORT_MEDIA_TYPE      = 9 ,  /* RW */
    TBL_PORT_FLD_SUPPORT_MEDIA_SWITCH = 10,  /* RW */
    TBL_PORT_FLD_SPLIT_TYPE           = 11,  /* RW */
    TBL_PORT_FLD_BYPASS               = 12,  /* RW */
    TBL_PORT_FLD_BYPASS_SLOT          = 13,  /* RW */
    TBL_PORT_FLD_TRAINING_ENABLE      = 14,  /* RW */
    TBL_PORT_FLD_TRAINING_STATUS      = 15,  /* READ */
    TBL_PORT_FLD_PRESENT_STATUS       = 16,  /* READ */
    TBL_PORT_FLD_NAME                 = 17,  /* READ */
    TBL_PORT_FLD_MAC                  = 18,  /* READ */
    TBL_PORT_FLD_MAX                  = 19
} tbl_port_field_id_t;

/* TBL_PORT defines */
typedef struct
{
    int32                port_id;
} tbl_port_key_t;

typedef struct
{
    tbl_port_key_t       key;
    int32                slot_no;             /* slot number show on panel */
    int32                panel_port_no;       /* port number show on panel */
    int32                panel_sub_port_no;   /* sub port number show on panel*/
    int32                phy_type;            /* physical type */
    int32                phyinfo_flag;        /* port flags */
    int32                speed_capbility;     /* bits of port speed */
    int32                logic_port_id;       /* logic port of ASIC chip, global port id */
    int32                media;               /* media type, comber fiber or copper*/
    int32                port_media_type;     /* comber port choise, 4 sfp or 1 qsfp*/
    int32                support_media_switch;
    int32                split_type;          /* port split type */
    int32                bypass;              /* bypass function */
    int32                bypass_slot;         /* bypass function */
    int32                training_enable;     /* enable/disable 802.3AP training */
    int32                training_status;     /* if support 802.3AP, store DAC training state */
    int32                present_status;      /* 1 means present, 0 means absent*/
    char                 name[IFNAME_SIZE];
    mac_addr_t           mac;
} tbl_port_t;

typedef struct
{
    tbl_port_t           *port_array[GLB_CHSM_PORT_NUM_MAX];
} tbl_port_master_t;

#endif /* !__TBL_PORT_DEFINE_H__ */

