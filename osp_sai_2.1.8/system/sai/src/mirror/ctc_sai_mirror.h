#if !defined (__SAI_MIRROR_H_)
#define __SAI_MIRROR_H_

#include <saimirror.h>
#include<ctc_opf.h>
#include <ctc_mix.h>
#include <ctc_hash.h>
#include <ctc_sai_nexthop.h>

#define CTC_SAI_MIRROR_SESSION_MAX  4
#define CTC_SAI_MIRROR_MCAST_GROUP_ID 4095 /*added by yejl to fix bug44848, 2017-10-23*/
#define CTC_SAI_MIRROR_MULTI_DEST_PORT_NUM  128

typedef struct ctc_sai_mirror_entry_s
{
    sai_object_id_t                    sid;
    sai_object_id_t                    nhid;
    sai_object_id_t                    monitor_port[CTC_SAI_MIRROR_MULTI_DEST_PORT_NUM];  /* the array of mirror dest port*/
    int32                              monitor_port_count; /* the number of dest port */
    int32                              monitor_type;   /* sai_mirror_type_t */
    int32                              encap_type; /*sai_erspan_encapsulation_type_t*/ 

    uint32                             dest_gport;
    uint16                             tpid;
    uint16                             gre_protocol_type;
    sai_vlan_id_t                      vlanid;

    uint8                              add_multidest_member;  /*If add the dest port in multi-dest mirror, is TRUE*/
    uint8                              is_multi_dest_mirror;  /*When creat multi-dest mirror, is TRUE*/
    uint8                              tc;   /*traffic class*/
    uint8                              vlan_pri;
    uint8                              iphdr_version;
    uint8                              tos;
    uint8                              ttl;
    
    sai_ip_address_t                   src_ip_address;
    sai_ip_address_t                   dst_ip_address;  

    sai_mac_t                          src_mac_address;
    sai_mac_t                          dst_mac_address;
}ctc_sai_mirror_entry_t;

typedef struct ctc_sai_mirror_info_s
{
    uint32_t        max_count;
    ctc_sai_mirror_entry_t  session[CTC_SAI_MIRROR_SESSION_MAX];
}ctc_sai_mirror_info_t;

ctc_sai_mirror_entry_t*
ctc_sai_mirror_entry_get(sai_object_id_t session_oid);

ctc_sai_mirror_entry_t*
ctc_sai_mirror_session_id_entry_get(uint32 sessionid);

sai_status_t
ctc_sai_mirror_alloc_offset(uint32_t *popf_index);

sai_status_t
ctc_sai_mirror_free_offset(uint32_t opf_index);

sai_status_t ctc_sai_mirror_init();


#endif
