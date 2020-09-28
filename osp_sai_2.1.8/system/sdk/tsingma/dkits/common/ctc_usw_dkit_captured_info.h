#ifndef _CTC_DKIT_GOLDENGATE_CAPTURED_INFO_H_
#define _CTC_DKIT_GOLDENGATE_CAPTURED_INFO_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "sal.h"
#include "ctc_dkit.h"

enum ctc_dkits_l3da_key_type_e
{
    CTC_DKITS_L3DA_TYPE_IPV4UCAST,
    CTC_DKITS_L3DA_TYPE_IPV6UCAST,
    CTC_DKITS_L3DA_TYPE_IPV4MCAST,
    CTC_DKITS_L3DA_TYPE_IPV6MCAST,
    CTC_DKITS_L3DA_TYPE_FCOE,
    CTC_DKITS_L3DA_TYPE_TRILLUCAST,
    CTC_DKITS_L3DA_TYPE_TRILLMCAST,

    CTC_DKITS_L3DA_TYPE_MAX
};
typedef  enum ctc_dkits_l3da_key_type_e ctc_dkits_l3da_key_type_t;

enum ctc_dkits_l3sa_key_type_e
{
    CTC_DKITS_L3SA_TYPE_IPV4RPF,
    CTC_DKITS_L3SA_TYPE_IPV6RPF,
    CTC_DKITS_L3SA_TYPE_IPV4PBR,
    CTC_DKITS_L3SA_TYPE_IPV6PBR,
    CTC_DKITS_L3SA_TYPE_IPV4NATSA,
    CTC_DKITS_L3SA_TYPE_IPV6NATSA,
    CTC_DKITS_L3SA_TYPE_FCOERPF,

    CTC_DKITS_L3SA_TYPE_MAX
};
typedef  enum ctc_dkits_l3sa_key_type_e ctc_dkits_l3sa_key_type_t;

extern int32
ctc_usw_dkit_captured_install_flow(void* p_para);

extern int32
ctc_usw_dkit_captured_clear(void* p_para);

extern int32
ctc_usw_dkit_captured_path_process(void* p_para);

#ifdef __cplusplus
}
#endif

#endif
