#if !defined (__SAI_ISOLATION_H_)
#define __SAI_ISOLATION_H_

#include "saiisolation.h"
#include "ctc_opf.h"

#define CTC_SAI_ISOLATION_MAX           30
#define CTC_SAI_ISOLATION_MLAG          31
#define CTC_SAI_ISOLATION_MLAG_UCAST    CTC_SAI_ISOLATION_MAX

typedef struct
{
    sai_object_id_t                     isolated_oid;
    uint32                              isolated_id;
    sai_isolaton_group_drop_pkt_type_t  type;
    sai_isolaton_group_mode_t           mode;
} ctc_sai_isolation_group_t;

typedef struct
{
    uint32                      first_alloc;    /* first_alloc must be for MLAG */
    uint32                      max_count;
    ctc_sai_isolation_group_t   group[CTC_SAI_ISOLATION_MAX+1];    
} ctc_sai_isolation_info_t;

sai_status_t ctc_sai_isolation_init();

ctc_sai_isolation_group_t*
ctc_sai_get_isolation_group(sai_object_id_t isolation_oid);

#endif /* !__SAI_ISOLATION_H_ */

