#ifndef __LCM_CARD_H__
#define __LCM_CARD_H__

#include "glb_hw_define.h"

enum lcm_board_material_s
{
    BOARD_MATERIAL_FR4 = 0,
    BOARD_MATERIAL_M4 = 1,
    BOARD_MATERIAL_M6 =2,
};

struct lcm_card_port_s
{
    uint8  chip_idx;
    uint16 mac_idx;/* Modifed, support slice */
    uint16 logic_port_idx;
    uint16 phy_addr;

    int8 serdes_id;/* the serdes id of port, init by serdes_ffe_max table */
};
typedef struct lcm_card_port_s lcm_card_port_t;

struct lcm_card_port_panel_mapping_s
{
    uint8 panel_slot_no;   //panel slot id for this port
    uint8 panel_port_no;   //panel port id for this port
    int8 panel_subport_no;//pannel sub port id for this port
    glb_ppt_numchg_type_t    port_numchg_type;
};
typedef struct lcm_card_port_panel_mapping_s lcm_card_port_panel_mapping_t;

struct lcm_card_serdes_info_s
{
    int8 serdes_id;    //serdes id
    uint8 serdes_mode; //serdes mode, example xfi,sgmii,qsgmii,xaui,xlg,
};
typedef struct lcm_card_serdes_info_s lcm_card_serdes_info_t;

struct lcm_ffe_coefficient_s
{
    uint8 c0;
    uint8 c1;
    uint8 c2;
    uint8 c3;    
};
typedef struct lcm_ffe_coefficient_s lcm_ffe_coefficient_t;

struct lcm_card_serdes_ffe_s
{
    int8 serdes_id;
    /* Added by qicx, 2015-05-13 
    *  If one board only has one material, then 'trace_len2' will no use.
    *  If one board owns two materials, then 'trace_len' for one kind of material and 'trace_len2' for other.
    *  such as E580-48X6Q, 'trace_len' for M4, 'trace_len2' for FR4.
    */
    uint8 trace_len;   /* default trace_len */
    uint8 trace_len2;
    /* If one port need different ffe for different speed, then 'coeff1' for one speed, 'coeff2' for other.
    *  such as if one port support 40G and 100G, and the ffe for 40G is different from 100G,
    *  then 'coeff1' for 40G and 'coeff2' for 100G.
    *  such as HW-48X2Q4Z, the 100G port, 'coeff1' for speed 40G/10G, 'coeff2' for speed 100G.
    */
    /* for tsingma board, the 100G port need configrure different ffe for different speed and different media type */
    lcm_ffe_coefficient_t coeff1;
    lcm_ffe_coefficient_t coeff2;
    lcm_ffe_coefficient_t coeff3;
    lcm_ffe_coefficient_t coeff4;    
    uint8 lchip;
};
typedef struct lcm_card_serdes_ffe_s lcm_card_serdes_ffe_t;

#define COEFF1(c10,c11,c12,c13) .coeff1 = {.c0 = c10,.c1 = c11,.c2 = c12,.c3 = c13}
#define COEFF2(c20,c21,c22,c23) .coeff2 = {.c0 = c20,.c1 = c21,.c2 = c22,.c3 = c23}
#define COEFF3(c30,c31,c32,c33) .coeff3 = {.c0 = c30,.c1 = c31,.c2 = c32,.c3 = c33}
#define COEFF4(c40,c41,c42,c43) .coeff4 = {.c0 = c40,.c1 = c41,.c2 = c42,.c3 = c43}


struct lcm_chip_serdes_ffe_s
{
    uint8  serdes_id;                                       /**<[GB.GG] serdes id */
    uint8  mode;                                            /**<[GB.GG] relate to ctc_chip_serdes_ffe_mode_t */
    uint8  board_material;                                  /**<[GB.GG] motherboard material: 0--FR4, 1--M4, 2--M6 */
    uint8  trace_len;                                       /**<[GB.GG] trace length: 0-- (0~4)inch, 1--(4~7)inch, 2--(7~10)inch */
    uint8  c0;                                              /**<[GB.GG] coefficient value */
    uint8  c1;
    uint8  c2;
    uint8  c3;
    uint8  speed;                                           /* for different speed maybe use different ffe */ 
    uint8  lchip;                                           /* for multi-chip board, such as 530-48x */
    uint8  is_DAC;
};
typedef struct lcm_chip_serdes_ffe_s lcm_chip_serdes_ffe_t;

typedef enum
{
    LCM_PHY_CUSTOM_ATTO = 1,
    LCM_PHY_CUSTOM_MAX,
} lcm_phy_custom_e;

typedef int32 (*card_init_callback_t)(void);
typedef int32 (*lcm_port_get_sum_ffe_cfg_callback_t)(lcm_chip_serdes_ffe_t*);
typedef int32 (*lcm_port_syce_enable_callback_t)(uint32 , uint32 );
typedef int32 (*lcm_port_cfg_mux_callback_t)(uint32, uint32);
typedef int32 (*lcm_phy_custom_init_callback_t)(lcm_phy_custom_e);

int32 lcm_init_sw_emu(glb_card_t* p_card);
int32 lcm_init_greatbelt_demo(glb_card_t* p_card);
int32 lcm_init_goldengate_demo(glb_card_t* p_card);
int32 lcm_init_e580_48x2q4z(glb_card_t* p_card);
int32 lcm_init_e580_48x6q(glb_card_t* p_card);
int32 lcm_init_e350_48t4x2q(glb_card_t* p_card);
int32 lcm_init_e580_32x2q(glb_card_t* p_card);
int32 lcm_init_e550_24x8y2c(glb_card_t* p_card);
int32 lcm_init_e550_48t4y2q(glb_card_t* p_card);
int32 lcm_init_e550_24t16y(glb_card_t* p_card);
int32 lcm_init_tm2_emu(glb_card_t* p_card);
/* added by liuht for e350 48+4, 2013-11-05 */
int32 lcm_init_e350_48t4xg(glb_card_t* p_card);
int32 lcm_init_e350_8t12x(glb_card_t* p_card);
int32 lcm_init_e350_8t4s12x(glb_card_t* p_card);
int32 lcm_init_e350_24t4xg(glb_card_t* p_card);
int32 lcm_init_e350_8ts12x(glb_card_t* p_card);
int32 lcm_init_e350_65dv1(glb_card_t* p_card);
int32 lcm_init_nid(glb_card_t* p_card);
int32 lcm_init_e350_24x(glb_card_t* p_card);
int32 lcm_init_ptn722(glb_card_t* p_card);
int32 lcm_init_e350_mt_8t12x(glb_card_t* p_card);
int32 lcm_init_e350_pegatron(glb_card_t* p_card);
int32 lcm_init_p580_32x6q(glb_card_t* p_card);
int32 lcm_init_e580_wqt48xt4q(glb_card_t* p_card);
int32 lcm_init_e350_hf_12t4s8x(glb_card_t* p_card);
int32 lcm_init_e530_24x2c_d(glb_card_t* p_card);
int32 lcm_init_e530_48t4x2q_d(glb_card_t* p_card);
int32 lcm_init_e530_48t4x(glb_card_t* p_card);
int32 lcm_init_e530_48x(glb_card_t* p_card);
int32 lcm_init_e530_24x2c(glb_card_t* p_card);
int32 lcm_init_e530_48t4x(glb_card_t* p_card);
int32 lcm_init_e530_48s4x(glb_card_t* p_card);
int32 lcm_init_sg8348c(glb_card_t* p_card);
int32 lcm_init_t65c_8t64x6q(glb_card_t* p_card);
int32
lcm_common_get_board_material(uint8 *bm);
int32
lcm_common_get_slot_no(uint16 * logic_slot_no, uint16 * phy_slot_no);
int32
lcm_common_init_cpu_info();
int32
lcm_common_save_port_info_file(glb_card_t* p_card);
int32
lcm_common_parse_port_numchg_info(glb_card_t* p_card);
int32
lcm_common_ppt_map_port(glb_card_t* p_card, lcm_card_port_panel_mapping_t* port_panel_mapping);
int32
lcm_parse_switch_cli(glb_card_t* p_card, char *cli_str, uint32 *panel_slot, uint32 *panel_port, char type[32]);
int32
lcm_common_get_defaultspeed_from_speedability(uint8 speed_ability, glb_port_speed_t*  speed, glb_port_duplex_t* duplex);
int32
lcm_common_get_defaultfec_from_speedability(uint16 speed_ability, uint32*  fec_ability);
int32 lcm_get_port_info(glb_card_t* p_card);
int32 lcm_common_config_ptp_enable(int enable);//config ptp
int32 lcm_common_config_syncE_enable(int enable);//config syncE
int32 lcm_common_config_reboot_record_type(int type,int32 epld_addr);//config syncE

int32 lcm_common_parse_bootup_diag(glb_card_t* p_card);
#endif


