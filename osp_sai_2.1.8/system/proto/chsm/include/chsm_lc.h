#ifndef __CHSM_LC_H__
#define __CHSM_LC_H__

//#include "glb_device.h"
#define CLI_CFG_ALL_LINECARD_NO    0xFF

int chsm_lc_reload_lc (uint32_t phy_slot_no);
int chsm_lc_remove_lc (uint32_t phy_slot_no);
int chsm_lc_set_type(int slot, int brd_type, int series);
int chsm_lc_set_pkt_seq_chk_en(uint32_t enable);
int chsm_lc_get_pkt_seq_chk_en(uint32_t *enable);
int chsm_lc_show_diag_item(uint32_t is_detail);
int chsm_lc_upgrade_lcdimage(uint32_t phy_slot, int type, char *image_path);
int chsm_lc_show_lcdimage(uint32_t phy_slot);
int chsm_lc_check_slot(int slot);
int chsm_lc_config_write ();
#endif /* __CHSM_LC_H__ */
