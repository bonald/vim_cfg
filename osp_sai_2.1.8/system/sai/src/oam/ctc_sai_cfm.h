#if !defined (__SAI_CFM_H_)
#define __SAI_CFM_H_

#include <saicfm.h>
#include <saiswitch.h>

typedef enum
{
    SAI_CFM_ENABEL,
    SAI_CFM_MODE,
    SAI_CFM_MAID,
}ctc_sai_cfm_update_type_t;

sai_status_t
ctc_sai_cfm_init(void);

typedef int32 (*SAI_CFM_ISR_CB)(sai_attribute_t *attr);

#ifdef GREATBELT
sai_status_t
ctc_sai_cfm_set_lmep_cc_enable_to_sdk_for_port(uint32 port_id, bool enable);
#endif

#endif

