#ifndef _CTC_DKIT_GOLDENGATE_INTERFACE_H_
#define _CTC_DKIT_GOLDENGATE_INTERFACE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "sal.h"

#define CTC_DKIT_INTERFACE_MAC_NUM 64

#define CTC_DKITS_MAC_IS_HSS28G(id) \
    (((id >= 12) && (id <= 15)) || ((id >= 28) && (id <= 31)) || \
    ((id >= 44) && (id <= 47)) || ((id >=60) && (id <= 63)))

#define GET_QSGMII_TBL(mac_id, pcs_id, tbl_id)\
    uint32 tbl_arr[4*12] =         \
        { QsgmiiPcsStatus00_t, QsgmiiPcsStatus10_t, QsgmiiPcsStatus20_t, QsgmiiPcsStatus30_t, \
          QsgmiiPcsStatus01_t, QsgmiiPcsStatus11_t, QsgmiiPcsStatus21_t, QsgmiiPcsStatus31_t, \
          QsgmiiPcsStatus02_t, QsgmiiPcsStatus12_t, QsgmiiPcsStatus22_t, QsgmiiPcsStatus32_t, \
          QsgmiiPcsStatus03_t, QsgmiiPcsStatus13_t, QsgmiiPcsStatus23_t, QsgmiiPcsStatus33_t, \
          QsgmiiPcsStatus04_t, QsgmiiPcsStatus14_t, QsgmiiPcsStatus24_t, QsgmiiPcsStatus34_t, \
          QsgmiiPcsStatus05_t, QsgmiiPcsStatus15_t, QsgmiiPcsStatus25_t, QsgmiiPcsStatus35_t, \
          QsgmiiPcsStatus06_t, QsgmiiPcsStatus16_t, QsgmiiPcsStatus26_t, QsgmiiPcsStatus36_t, \
          QsgmiiPcsStatus07_t, QsgmiiPcsStatus17_t, QsgmiiPcsStatus27_t, QsgmiiPcsStatus37_t, \
          QsgmiiPcsStatus08_t, QsgmiiPcsStatus18_t, QsgmiiPcsStatus28_t, QsgmiiPcsStatus38_t, \
          QsgmiiPcsStatus09_t, QsgmiiPcsStatus19_t, QsgmiiPcsStatus29_t, QsgmiiPcsStatus39_t, \
          QsgmiiPcsStatus010_t, QsgmiiPcsStatus110_t, QsgmiiPcsStatus210_t, QsgmiiPcsStatus310_t, \
          QsgmiiPcsStatus011_t, QsgmiiPcsStatus111_t, QsgmiiPcsStatus211_t, QsgmiiPcsStatus311_t, \
        };                                                                                       \
    uint8 index = 0;                                                                             \
    index = pcs_id*4 + mac_id%4;                                                                 \
    tbl_id = tbl_arr[index];

struct ctc_dkit_interface_status_s
{
    uint16 gport[CTC_DKIT_INTERFACE_MAC_NUM];
    uint8 mac_en[CTC_DKIT_INTERFACE_MAC_NUM];
    uint8 mac_mode[CTC_DKIT_INTERFACE_MAC_NUM];
    uint8 auto_neg[CTC_DKIT_INTERFACE_MAC_NUM];
    uint8 link_up[CTC_DKIT_INTERFACE_MAC_NUM];
    uint8 valid[CTC_DKIT_INTERFACE_MAC_NUM];
};
typedef struct ctc_dkit_interface_status_s ctc_dkit_interface_status_t;

extern int32
ctc_usw_dkit_interface_print_pcs_table(uint8 lchip, uint32 tbl_id, uint32 index);

#ifdef __cplusplus
}
#endif

#endif

