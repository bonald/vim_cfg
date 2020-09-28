#if !defined (__SAI_MACESCAPE_H_)
#define __SAI_MACESCAPE_H_

#include <saimacescape.h>

#define CTC_SAI_MAC_ESCAPE_NUM_MAX  2

typedef struct ctc_sai_macescape_entry_s
{
    sai_mac_escape_entry_t     macescape_key;
    uint32                     is_valid;
}ctc_sai_macescape_entry_t;

typedef struct ctc_sai_macescape_info_s
{
    uint32_t                   entry_count;
    ctc_sai_macescape_entry_t  macescape[CTC_SAI_MAC_ESCAPE_NUM_MAX];
}ctc_sai_macescape_info_t;

sai_status_t
ctc_sai_macescape_init();


#endif /* !__SAI_MACESCAPE_H_ */

