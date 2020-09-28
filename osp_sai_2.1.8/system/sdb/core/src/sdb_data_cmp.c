
#include "sdb.h"
#include "gen/stbl_rif_define.h"
#include "gen/stbl_nexthop_define.h"
#include "gen/stbl_hostif_define.h"
#include "gen/stbl_neighbor_define.h"
#include "gen/stbl_acl_port_define.h"
#include "gen/stbl_cfm_lmep_define.h"
#include "gen/stbl_cfm_rmep_define.h"
#include "gen/stbl_qos_mpls_domain_define.h"
#include "gen/stbl_qos_mpls_flow_define.h"

int32
stbl_rif_cmp(void *data1, void *data2)
{
    stbl_rif_t *p_rif1 = (stbl_rif_t*)data1;
    stbl_rif_t *p_rif2 = (stbl_rif_t*)data2;

    return (p_rif1->key.rif_id - p_rif2->key.rif_id);
}

int32
stbl_nexthop_cmp(void *data1, void *data2)
{
    stbl_nexthop_t *p_nh1 = (stbl_nexthop_t*)data1;
    stbl_nexthop_t *p_nh2 = (stbl_nexthop_t*)data2;

    return (p_nh1->nh_oid - p_nh2->nh_oid);
}

int32
stbl_hostif_cmp(void *data1, void *data2)
{
    stbl_hostif_t *p_hif1 = (stbl_hostif_t*)data1;
    stbl_hostif_t *p_hif2 = (stbl_hostif_t*)data2;
    uint32 hif_id1 = 0;
    uint32 hif_id2 = 0;
    
    hif_id1 = (p_hif1->key.hif_oid & 0xFFFFFFFF);
    hif_id2 = (p_hif2->key.hif_oid & 0xFFFFFFFF);
    
    return (hif_id1 - hif_id2);
}

int32
stbl_neighbor_cmp(void *data1, void *data2)
{
    stbl_neighbor_t *p_hif1 = (stbl_neighbor_t*)data1;
    stbl_neighbor_t *p_hif2 = (stbl_neighbor_t*)data2;
    
    return sal_memcmp(&p_hif1->key, &p_hif2->key, sizeof(p_hif1->key));
}

int32
stbl_acl_port_cmp(void *data1, void *data2)
{
    stbl_acl_port_t *p_port1 = (stbl_acl_port_t*)data1;
    stbl_acl_port_t *p_port2 = (stbl_acl_port_t*)data2;

    return (p_port1->key.port_oid - p_port2->key.port_oid);
}
int32
stbl_cfm_lmep_cmp(void *data1, void *data2)
{
    stbl_cfm_lmep_t *p_lmep1 = (stbl_cfm_lmep_t*)data1;
    stbl_cfm_lmep_t *p_lmep2 = (stbl_cfm_lmep_t*)data2;

    return (p_lmep1->key.index - p_lmep2->key.index);
}

int32
stbl_cfm_rmep_cmp(void *data1, void *data2)
{
    stbl_cfm_rmep_t *p_rmep1 = (stbl_cfm_rmep_t*)data1;
    stbl_cfm_rmep_t *p_rmep2 = (stbl_cfm_rmep_t*)data2;

    return (p_rmep1->key.index - p_rmep2->key.index);
}

int32
stbl_qos_mpls_domain_cmp(void *data1, void *data2)
{
    stbl_qos_mpls_domain_t *p_mpls_domain1 = (stbl_qos_mpls_domain_t*)data1;
    stbl_qos_mpls_domain_t *p_mpls_domain2 = (stbl_qos_mpls_domain_t*)data2;

    return (p_mpls_domain1->key.mpls_label - p_mpls_domain2->key.mpls_label);
}

int32
stbl_qos_mpls_flow_cmp(void *data1, void *data2)
{
    stbl_qos_mpls_flow_t *p_mpls_flow1 = (stbl_qos_mpls_flow_t*)data1;
    stbl_qos_mpls_flow_t *p_mpls_flow2 = (stbl_qos_mpls_flow_t*)data2;

    return (p_mpls_flow1->key.mpls_label - p_mpls_flow2->key.mpls_label);
}
