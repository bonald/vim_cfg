#if !defined (__SAI_VLANCLASS_H_)
#define __SAI_VLANCLASS_H_

#include "saivlanclass.h"
#include "ctc_opf.h"
#include <ctc_mix.h>

#define CTC_SAI_VALNCLASS_RULE_MAX      4096
#define CTC_SAI_VALNCLASS_GROUP_MAX     32

#define CTC_SAI_MAX_IPV4_MASK_LEN       32
#define CTC_SAI_MAX_IPV6_MASK_LEN       128

#define     CTC_VLANCLASS_P_IP              1
#define     CTC_VLANCLASS_P_IPV4            0x0800
#define     CTC_VLANCLASS_P_IPV6            0x86DD
#define     CTC_VLANCLASS_P_MPLS            0x8847
#define     CTC_VLANCLASS_P_MPLSMCAST       0x8848
#define     CTC_VLANCLASS_P_IPV4ARP         0x0806
#define     CTC_VLANCLASS_P_IPV4RARP        0x8035
#define     CTC_VLANCLASS_P_EAPOL           0x888E
#define     CTC_VLANCLASS_P_8021AG          0x8902
#define     CTC_VLANCLASS_P_SLOWPROTO       0x8809
#define     CTC_VLANCLASS_P_PPPOE           13  
#define     CTC_VLANCLASS_P_FLEXL3          15

sai_status_t ctc_sai_vlanclass_init();

#endif /* !__SAI_VLANCLASS_H_ */
