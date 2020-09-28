
#include "proto.h"
#include "pb_cdb_inc.h"
#include "tbl.pb-c.h"
#include "pb_tbl.h"
#include "pb_common.h"

uint32 g_pb_alloc_count = 0;
uint32 g_pb_free_count = 0;

static void *
pb_alloc(void *allocator_data, size_t size)
{
    g_pb_alloc_count++;
    return XMALLOC(MEM_LIB_PROTOBUF, size);
}

static void
pb_free(void *allocator_data, void *data)
{
    g_pb_free_count++;
    XFREE(MEM_LIB_PROTOBUF, data);
}

ProtobufCAllocator osp_pbc_allocator = 
{
    .alloc = &pb_alloc,
    .free = &pb_free,
    .allocator_data = NULL,
};

char*
pb_subscribe_response_oper_val2str(uint32 oper)
{
    switch (oper)
    {
    case CDB__SUBSCRIBE_RESPONSE__CDB_OPERTION__OPER_ADD:
        return "create";
    case CDB__SUBSCRIBE_RESPONSE__CDB_OPERTION__OPER_SET:
        return "update";
    case CDB__SUBSCRIBE_RESPONSE__CDB_OPERTION__OPER_GET:
        return "read";
    case CDB__SUBSCRIBE_RESPONSE__CDB_OPERTION__OPER_DEL:
        return "delete";
    default:
        return "invalid";
    }
}

int32
pb_subscribe_response_to_pb(pb_subscribe_response_t *p_resp, Cdb__SubscribeResponse *p_pb)
{
    switch (p_resp->oper)
    {
    case CDB_OPER_ADD:
        p_pb->action = CDB__SUBSCRIBE_RESPONSE__CDB_OPERTION__OPER_ADD;
        break;
    case CDB_OPER_SET:
        p_pb->action = CDB__SUBSCRIBE_RESPONSE__CDB_OPERTION__OPER_SET;
        p_pb->field_id = p_resp->field_id;
        break;
    case CDB_OPER_GET:
        p_pb->action = CDB__SUBSCRIBE_RESPONSE__CDB_OPERTION__OPER_GET;
        break;
    case CDB_OPER_DEL:
        p_pb->action = CDB__SUBSCRIBE_RESPONSE__CDB_OPERTION__OPER_DEL;
        break;
    default:
        return PM_E_FAIL;
    }

    switch (p_resp->type)
    {
    case CDB_NODE_TBL:
        p_pb->tbl_id = p_resp->tbl_id;
        p_pb->ds_id = INVALID_UINT32_ID;
        p_pb->ds2_id = INVALID_UINT32_ID;
        break;
    case CDB_NODE_DS:
        p_pb->tbl_id = p_resp->tbl_id;
        p_pb->ds_id = p_resp->ds_id;
        if (0 != p_resp->ds2_id)
        {
            p_pb->ds2_id = p_resp->ds2_id;
        }
        else
        {
            p_pb->ds2_id = INVALID_UINT32_ID;
        }
        break;
    default:
        return PM_E_FAIL;
    }
    
    return PM_E_NONE;
}

int32
pb_subscribe_response_dump(Cdb__SubscribeResponse *p_pb, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/action=(%s, %u)", pb_subscribe_response_oper_val2str(p_pb->action), p_pb->action);
    offset += sal_sprintf(out + offset, "/tbl_id=%u", p_pb->tbl_id);
    offset += sal_sprintf(out + offset, "/field_id=%u", p_pb->field_id);
    offset += sal_sprintf(out + offset, "/ds_id=%u", p_pb->ds_id);
    offset += sal_sprintf(out + offset, "/ds2_id=%u", p_pb->ds2_id);
    offset += sal_sprintf(out + offset, "\n");
    
    return PM_E_NONE;
}

int32
pb_subscribe_response_to_pb_free_packed(Cdb__SubscribeResponse *p_pb)
{
    return PM_E_NONE;
}

int32
pb_subscribe_response_pack(pb_subscribe_response_t *p_resp, uint8 *buf, uint32 buf_len)
{
    Cdb__SubscribeResponse pb_ctrl = CDB__SUBSCRIBE_RESPONSE__INIT;
    int32 len = 0;

    pb_subscribe_response_to_pb(p_resp, &pb_ctrl);
    len = cdb__subscribe_response__pack(&pb_ctrl, buf);
    pb_subscribe_response_to_pb_free_packed(&pb_ctrl);

    return len;
}

int32
pb_subscribe_request_to_pb(pb_subscribe_request_t *p_req, Cdb__SubscribeRequest *p_pb)
{
    if (CDB_SUB_CDB == p_req->type)
    {
        p_pb->cdb->node_type = p_req->cdb.node_type;
        p_pb->cdb->tbl_id = p_req->cdb.tbl_id;
        p_pb->cdb->field = p_req->cdb.field_id;
        if (p_req->cdb.key)
        {
            p_pb->cdb->key = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_req->cdb.key)+1);
            sal_memcpy(p_pb->cdb->key, p_req->cdb.key, sal_strlen(p_req->cdb.key));
        }
    }
    else if (CDB_SUB_TBL == p_req->type)
    {
        p_pb->tbl->sync = p_req->tbl.sync;
        p_pb->tbl->n_table = p_req->tbl.num;
        p_pb->tbl->table = XCALLOC(MEM_LIB_PROTOBUF, sizeof(uint32)*p_req->tbl.num);
        sal_memcpy(p_pb->tbl->table, p_req->tbl.tbl_id_list, sizeof(uint32)*p_req->tbl.num);
    }
    else
    {
        return PM_E_FAIL;
    }

    p_pb->pid = p_req->pid;
    p_pb->src = p_req->src;
    p_pb->format = p_req->format;
    p_pb->type = p_req->type;
    
    return PM_E_NONE;
}

int32
pb_subscribe_request_dump(Cdb__SubscribeRequest *p_pb, char *out)
{
    Cdb__SubscribeRequestTbl *p_tbl = NULL;
    Cdb__SubscribeRequestCdb *p_cdb = NULL;
    int32 i = 0;
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/type=%u", p_pb->type);
    offset += sal_sprintf(out + offset, "/pid=%u", p_pb->pid);
    offset += sal_sprintf(out + offset, "/src=%u", p_pb->src);
    offset += sal_sprintf(out + offset, "/format=%s", cdb_enum_val2str(cdb_sub_format_str, CDB_SUB_FORMAT_MAX, p_pb->format));

    if (CDB_SUB_CDB == p_pb->type)
    {
        p_cdb = p_pb->cdb;
        offset += sal_sprintf(out + offset, "/node_type=%u", p_cdb->node_type);
        offset += sal_sprintf(out + offset, "/tbl_id=%u", p_cdb->tbl_id);
        offset += sal_sprintf(out + offset, "/field=%u", p_cdb->field);
    }
    else if (CDB_SUB_TBL == p_pb->type)
    {
        p_tbl = p_pb->tbl;
        
        offset += sal_sprintf(out + offset, "/n_table=%u", (uint32_t)p_tbl->n_table);
        offset += sal_sprintf(out + offset, "/table=(");
        for (i = 0; i < p_tbl->n_table; i++)
        {
            offset += sal_sprintf(out + offset, "%u,", p_tbl->table[i]);
        }
        offset += sal_sprintf(out + offset, ")");
    }
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

int32
pb_subscribe_request_to_pb_free_packed(Cdb__SubscribeRequest *p_pb)
{
    if (CDB_SUB_CDB == p_pb->type)
    {
        if (p_pb->cdb)
        {
            if (p_pb->cdb->key)
            {
                XFREE(MEM_LIB_PROTOBUF, p_pb->cdb->key);
                p_pb->cdb->key = NULL;
            }
        }
    }
    else if (CDB_SUB_TBL == p_pb->type)
    {
        if (p_pb->tbl)
        {
            if (p_pb->tbl->table)
            {
                XFREE(MEM_LIB_PROTOBUF, p_pb->tbl->table);
                p_pb->tbl->table = NULL;
            }
        }
    }
    
    return PM_E_NONE;
}

int32
pb_subscribe_request_pack(pb_subscribe_request_t *p_req, uint8 *buf, uint32 buf_len)
{
    Cdb__SubscribeRequest pb_req = CDB__SUBSCRIBE_REQUEST__INIT;
    Cdb__SubscribeRequestCdb pb_cdb = CDB__SUBSCRIBE_REQUEST_CDB__INIT;
    Cdb__SubscribeRequestTbl pb_tbl = CDB__SUBSCRIBE_REQUEST_TBL__INIT;
    char out[MSG_SIZE];
    int32 len = 0;

    if (CDB_SUB_CDB == p_req->type)
    {
        pb_req.cdb = &pb_cdb;
        pb_subscribe_request_to_pb(p_req, &pb_req);
    }
    else if (CDB_SUB_TBL == p_req->type)
    {
        pb_req.tbl = &pb_tbl;
        pb_subscribe_request_to_pb(p_req, &pb_req);
    }
    else
    {
        return PM_E_FAIL;
    }

    pb_subscribe_request_dump(&pb_req, out);
    sal_printf("%s", out);
    len = cdb__subscribe_request__pack(&pb_req, buf);
    pb_subscribe_request_to_pb_free_packed(&pb_req);

    return len;
}

int32
pb_bitmap_array_dump(uint8 *array, uint32 len, char *out)
{
    char buf[MSG_SIZE];
    const uint32 *bmp = (const uint32 *)array;
    uint32 bmp_len = len/4;
    int32 offset = 0;

    cdb_bitmap_val2str(buf, MSG_SIZE, bmp, bmp_len);
    offset += sal_sprintf(out, "%s", buf);
    
    return offset;
}

int32
pb_int8_array_dump(int8 *array, uint32 len, char *out)
{
    int32 i = 0;
    int32 offset = 0;

    for (i = 0; i < len; i++)
    {
        offset += sal_sprintf(out + offset, "%d,", array[i]);
    }
    
    return offset;
}

int32
pb_uint8_array_dump(uint8 *array, uint32 len, char *out)
{
    int32 i = 0;
    int32 offset = 0;

    for (i = 0; i < len; i++)
    {
        offset += sal_sprintf(out + offset, "%u,", array[i]);
    }
    
    return offset;
}

int32
pb_int32_array_dump(int32 *array, uint32 len, char *out)
{
    int32 i = 0;
    int32 offset = 0;

    for (i = 0; i < len; i++)
    {
        offset += sal_sprintf(out + offset, "%d,", array[i]);
    }
    
    return offset;
}

int32
pb_uint32_array_dump(uint32 *array, uint32 len, char *out)
{
    int32 i = 0;
    int32 offset = 0;

    for (i = 0; i < len; i++)
    {
        offset += sal_sprintf(out + offset, "%u,", array[i]);
    }
    
    return offset;
}

int32
pb_double_array_dump(double *array, uint32 len, char *out)
{
    int32 i = 0;
    int32 offset = 0;

    for (i = 0; i < len; i++)
    {
        offset += sal_sprintf(out + offset, "%f,", array[i]);
    }
    
    return offset;
}

int32
pb_string_array_dump(char **array, uint32 len, char *out)
{
    int32 i = 0;
    int32 offset = 0;

    if (NULL == array)
    {
        return offset;
    }

    for (i = 0; i < len; i++)
    {
        if (array[i])
        {
            offset += sal_sprintf(out + offset, "%s,", array[i]);
        }
    }
    
    return offset;
}

int32
pb_compose_fdb_key_t_to_pb(fdb_key_t *p_cdb, Cdb__ComposeFdbKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->fid = p_cdb->fid;
        p_pb->mac.len = sizeof(p_cdb->mac);
        p_pb->mac.data = XCALLOC(MEM_LIB_PROTOBUF, sizeof(p_cdb->mac));
        sal_memcpy(p_pb->mac.data, p_cdb->mac, sizeof(p_cdb->mac));
    }
    
    return PM_E_NONE;
}

int32
pb_compose_fdb_key_t_dump(Cdb__ComposeFdbKeyT *p_pb, char *out)
{
    int32 len = 0;

    if (p_pb->mac.data)
    {    
        len = sal_sprintf(out, "/fdb_key=(mac: %.04hX.%.04hX.%.04hX, fid: %u)", 
            sal_ntoh16(((uint16*)p_pb->mac.data)[0]),
            sal_ntoh16(((uint16*)p_pb->mac.data)[1]),
            sal_ntoh16(((uint16*)p_pb->mac.data)[2]),
            p_pb->fid);
    }
    
    return len;
}

int32
pb_compose_fdb_key_t_to_pb_free_packed(Cdb__ComposeFdbKeyT *p_pb)
{
    if (p_pb)
    {
        if (p_pb->mac.data)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->mac.data);
            p_pb->mac.data = NULL;
        }
        p_pb->mac.len = 0;
    }

    return PM_E_NONE;
}

int32
pb_compose_erps_ring_key_t_to_pb(erps_ring_key_t *p_cdb, Cdb__ComposeErpsRingKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->domain_id = p_cdb->domain_id;
        p_pb->ring_id = p_cdb->ring_id;
    }
    
    return PM_E_NONE;
}

int32
pb_compose_erps_ring_key_t_dump(Cdb__ComposeErpsRingKeyT *p_pb, char *out)
{
    int32 len = 0;

    len = sal_sprintf(out, "/erps_ring_key:domain_id=%u, ring_id=%u", p_pb->domain_id, p_pb->ring_id);

    return len;
}

int32
pb_compose_erps_ring_key_t_to_pb_free_packed(Cdb__ComposeErpsRingKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_mac_addr_t_to_pb(uint8 *p_cdb, Cdb__ComposeMacAddrT *p_pb)
{
    if (p_pb)
    {
        p_pb->mac.len = sizeof(mac_addr_t);
        p_pb->mac.data = XCALLOC(MEM_LIB_PROTOBUF, sizeof(mac_addr_t));
        sal_memcpy(p_pb->mac.data, p_cdb, sizeof(mac_addr_t));
    }
    
    return PM_E_NONE;
}

int32
pb_compose_mac_addr_t_dump(Cdb__ComposeMacAddrT *p_pb, char *out)
{
    int32 len = 0;
    
    if (p_pb->mac.data)
    {
        len = sal_sprintf(out, "/mac=%.04hX.%.04hX.%.04hX", 
            sal_ntoh16(((uint16*)p_pb->mac.data)[0]),
            sal_ntoh16(((uint16*)p_pb->mac.data)[1]),
            sal_ntoh16(((uint16*)p_pb->mac.data)[2]));
    }

    return len;
}

int32
pb_compose_mac_addr_t_to_pb_free_packed(Cdb__ComposeMacAddrT *p_pb)
{
    if (p_pb)
    {
        if (p_pb->mac.data)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->mac.data);
            p_pb->mac.data = NULL;
        }
        p_pb->mac.len = 0;
    }
    
    return PM_E_NONE;
}

int32
pb_compose_msti_port_key_t_to_pb(msti_port_key_t *p_cdb, Cdb__ComposeMstiPortKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_cdb->name)+1);
        sal_strcpy(p_pb->name, p_cdb->name);
        p_pb->instance = p_cdb->instance;
    }
    
    return PM_E_NONE;
}

int32
pb_compose_msti_port_key_t_dump(Cdb__ComposeMstiPortKeyT *p_pb, char *out)
{
    int32 len = 0;
    
    if (p_pb)
    {
        len = sal_sprintf(out, "/msti_key=(name=%s, instance=%u)", p_pb->name, p_pb->instance);
    }

    return len;
}

int32
pb_compose_msti_port_key_t_to_pb_free_packed(Cdb__ComposeMstiPortKeyT *p_pb)
{
    if (p_pb)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->name);
        p_pb->name = NULL;
    }

    return PM_E_NONE;
}

int32
pb_compose_route_node_key_t_to_pb(route_node_key_t *p_cdb, Cdb__ComposeRouteNodeKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->vrf_id = p_cdb->vrf_id;
        p_pb->family = p_cdb->p.family;
        p_pb->prefixlen = p_cdb->p.prefixlen;
        p_pb->prefixlen = p_cdb->p.prefixlen;

        if (AF_INET == p_pb->family)
        {
            p_pb->ipv4_addr = p_cdb->p.u.prefix4.s_addr;
        }
        else if (AF_INET6 == p_pb->family)
        {
            p_pb->n_ipv6_addr = sizeof(addr_ipv6_t);
            p_pb->ipv6_addr = XCALLOC(MEM_LIB_PROTOBUF, sizeof(addr_ipv6_t));
            sal_memcpy(p_pb->ipv6_addr, &p_cdb->p.u.prefix6, sizeof(addr_ipv6_t));
        }
    }

    return PM_E_NONE;
}

int32
pb_compose_route_node_key_t_dump(Cdb__ComposeRouteNodeKeyT *p_pb, char *out)
{
    int32 len = 0;
    char str[CMD_BUF_32];
    
    if (AF_INET == p_pb->family)
    {
        /* IPv4 */
        len = sal_sprintf(out, "/route_node_key=(vrf_id=%u, family=ipv4, prefixlen=%u, addr=%s)", 
            p_pb->vrf_id, p_pb->prefixlen, sal_inet_ntop(AF_INET, (void*)&(p_pb->ipv4_addr), str, CMD_BUF_32));
    }
    else if (AF_INET6 == p_pb->family)
    {
        /* IPv6 */
        len = sal_sprintf(out, "/route_node_key=(vrf_id=%u, family=ipv6, prefixlen=%u, addr=%s)", 
            p_pb->vrf_id, p_pb->prefixlen, "ipv6_todo");
    }
    
    return len;
}

int32
pb_compose_route_node_key_t_to_pb_free_packed(Cdb__ComposeRouteNodeKeyT *p_pb)
{
    if (p_pb)
    {
        if (AF_INET6 == p_pb->family)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->ipv6_addr);
            p_pb->ipv6_addr = NULL;
            p_pb->n_ipv6_addr = 0;
        }
    }
    
    return PM_E_NONE;
}

int32
pb_compose_route_node_key_with_ad_t_to_pb(route_node_key_with_ad_t *p_cdb, Cdb__ComposeRouteNodeKeyWithAdT *p_pb)
{
    if (p_pb)
    {
        p_pb->vrf_id = p_cdb->vrf_id;
        p_pb->family = p_cdb->p.family;
        p_pb->prefixlen = p_cdb->p.prefixlen;
        p_pb->prefixlen = p_cdb->p.prefixlen;

        if (AF_INET == p_pb->family)
        {
            p_pb->ipv4_addr = p_cdb->p.u.prefix4.s_addr;
        }
        else if (AF_INET6 == p_pb->family)
        {
            p_pb->n_ipv6_addr = sizeof(addr_ipv6_t);
            p_pb->ipv6_addr = XCALLOC(MEM_LIB_PROTOBUF, sizeof(addr_ipv6_t));
            sal_memcpy(p_pb->ipv6_addr, &p_cdb->p.u.prefix6, sizeof(addr_ipv6_t));
        }
    }

    return PM_E_NONE;
}

int32
pb_compose_route_node_key_with_ad_t_dump(Cdb__ComposeRouteNodeKeyWithAdT *p_pb, char *out)
{
    int32 len = 0;
    char str[CMD_BUF_32];
    
    if (AF_INET == p_pb->family)
    {
        /* IPv4 */
        len = sal_sprintf(out, "/route_node_key_with_ad=(vrf_id=%u, family=ipv4, prefixlen=%u, addr=%s)", 
            p_pb->vrf_id, p_pb->prefixlen, sal_inet_ntop(AF_INET, (void*)&(p_pb->ipv4_addr), str, CMD_BUF_32));
    }
    else if (AF_INET6 == p_pb->family)
    {
        /* IPv6 */
        len = sal_sprintf(out, "/route_node_key_with_ad=(vrf_id=%u, family=ipv6, prefixlen=%u, addr=%s)", 
            p_pb->vrf_id, p_pb->prefixlen, "ipv6_todo");
    }
    
    return len;
}

int32
pb_compose_route_node_key_with_ad_t_to_pb_free_packed(Cdb__ComposeRouteNodeKeyWithAdT *p_pb)
{
    if (p_pb)
    {
        if (AF_INET6 == p_pb->family)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->ipv6_addr);
            p_pb->ipv6_addr = NULL;
            p_pb->n_ipv6_addr = 0;
        }
    }
    
    return PM_E_NONE;
}

int32
pb_compose_ospf_area_range_key_t_to_pb(ospf_area_range_key_t *p_cdb, Cdb__ComposeOspfAreaRangeKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->area_id = p_cdb->area_id;
        p_pb->family = p_cdb->p.family;
        p_pb->prefixlen = p_cdb->p.prefixlen;
        p_pb->prefixlen = p_cdb->p.prefixlen;

        if (AF_INET == p_pb->family)
        {
            p_pb->ipv4_addr = p_cdb->p.u.prefix4.s_addr;
        }
        else if (AF_INET6 == p_pb->family)
        {
            p_pb->n_ipv6_addr = sizeof(addr_ipv6_t);
            p_pb->ipv6_addr = XCALLOC(MEM_LIB_PROTOBUF, sizeof(addr_ipv6_t));
            sal_memcpy(p_pb->ipv6_addr, &p_cdb->p.u.prefix6, sizeof(addr_ipv6_t));
        }
    }

    return PM_E_NONE;
}

int32
pb_compose_ospf_area_range_key_t_dump(Cdb__ComposeOspfAreaRangeKeyT *p_pb, char *out)
{
    int32 len = 0;
    char str[CMD_BUF_32];
    
    if (AF_INET == p_pb->family)
    {
        /* IPv4 */
        len = sal_sprintf(out, "/ospf_area_range_key=(area_id=%u, family=ipv4, prefixlen=%u, addr=%s)", 
            p_pb->area_id, p_pb->prefixlen, sal_inet_ntop(AF_INET, (void*)&(p_pb->ipv4_addr), str, CMD_BUF_32));
    }
    else if (AF_INET6 == p_pb->family)
    {
        /* IPv6 */
        len = sal_sprintf(out, "/ospf_area_range_key=(area_id=%u, family=ipv6, prefixlen=%u, addr=%s)", 
            p_pb->area_id, p_pb->prefixlen, "ipv6_todo");
    }
    
    return len;
}

int32
pb_compose_ospf_area_range_key_t_to_pb_free_packed(Cdb__ComposeOspfAreaRangeKeyT *p_pb)
{
    if (p_pb)
    {
        if (AF_INET6 == p_pb->family)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->ipv6_addr);
            p_pb->ipv6_addr = NULL;
            p_pb->n_ipv6_addr = 0;
        }
    }
    
    return PM_E_NONE;
}

int32
pb_compose_ospf6_area_range_key_t_to_pb(ospf6_area_range_key_t *p_cdb, Cdb__ComposeOspfAreaRangeKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->area_id = p_cdb->area_id;
        p_pb->family = p_cdb->p.family;
        p_pb->prefixlen = p_cdb->p.prefixlen;
        p_pb->prefixlen = p_cdb->p.prefixlen;

        if (AF_INET == p_pb->family)
        {
            p_pb->ipv4_addr = p_cdb->p.u.prefix4.s_addr;
        }
        else if (AF_INET6 == p_pb->family)
        {
            p_pb->n_ipv6_addr = sizeof(addr_ipv6_t);
            p_pb->ipv6_addr = XCALLOC(MEM_LIB_PROTOBUF, sizeof(addr_ipv6_t));
            sal_memcpy(p_pb->ipv6_addr, &p_cdb->p.u.prefix6, sizeof(addr_ipv6_t));
        }
    }

    return PM_E_NONE;
}

int32
pb_compose_ospf6_area_range_key_t_dump(Cdb__ComposeOspfAreaRangeKeyT *p_pb, char *out)
{
    int32 len = 0;
    char str[CMD_BUF_32];
    
    if (AF_INET == p_pb->family)
    {
        /* IPv4 */
        len = sal_sprintf(out, "/ospf6_area_range_key=(area_id=%u, family=ipv4, prefixlen=%u, addr=%s)", 
            p_pb->area_id, p_pb->prefixlen, sal_inet_ntop(AF_INET, (void*)&(p_pb->ipv4_addr), str, CMD_BUF_32));
    }
    else if (AF_INET6 == p_pb->family)
    {
        /* IPv6 */
        len = sal_sprintf(out, "/ospf6_area_range_key=(area_id=%u, family=ipv6, prefixlen=%u, addr=%s)", 
            p_pb->area_id, p_pb->prefixlen, "ipv6_todo");
    }
    
    return len;
}

int32
pb_compose_ospf6_area_range_key_t_to_pb_free_packed(Cdb__ComposeOspfAreaRangeKeyT *p_pb)
{
    if (p_pb)
    {
        if (AF_INET6 == p_pb->family)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->ipv6_addr);
            p_pb->ipv6_addr = NULL;
            p_pb->n_ipv6_addr = 0;
        }
    }
    
    return PM_E_NONE;
}

int32
pb_compose_ospf_area_vlink_key_t_to_pb(ospf_area_vlink_key_t *p_cdb, Cdb__ComposeOspfAreaVlinkKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->area_id = p_cdb->area_id;
        p_pb->vl_peer = p_cdb->vl_peer.s_addr;
    }

    return PM_E_NONE;
}

int32
pb_compose_ospf_area_vlink_key_t_dump(Cdb__ComposeOspfAreaVlinkKeyT *p_pb, char *out)
{
    int32 len = 0;
    char str[CMD_BUF_32];
    
    len = sal_sprintf(out, "/ospf_area_vlink_key=(area_id=%u, vl_peer=%s)", 
        p_pb->area_id, sal_inet_ntop(AF_INET, (void*)&(p_pb->vl_peer), str, CMD_BUF_32));
    
    return len;
}

int32
pb_compose_ospf_area_vlink_key_t_to_pb_free_packed(Cdb__ComposeOspfAreaVlinkKeyT *p_pb)
{
		    return PM_E_NONE;
}

int32
pb_compose_rip_timers_t_to_pb(rip_timers_t *p_cdb, Cdb__ComposeRipTimersT *p_pb)
{
	return PM_E_NONE;
}

int32
pb_compose_rip_timers_t_dump(Cdb__ComposeRipTimersT *p_pb, char *out)
{
	return PM_E_NONE;
}

int32
pb_compose_rip_timers_t_to_pb_free_packed(Cdb__ComposeRipTimersT *p_pb)
{
	return PM_E_NONE;
}

int32
pb_compose_binding_key_t_to_pb(binding_key_t *p_cdb, Cdb__ComposeBindingKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->ipv6 = p_cdb->ipv6;
        p_pb->mac.len = sizeof(p_cdb->mac);
        p_pb->mac.data = XCALLOC(MEM_LIB_PROTOBUF, sizeof(p_cdb->mac));
        sal_memcpy(p_pb->mac.data, p_cdb->mac, sizeof(p_cdb->mac));
        p_pb->vid = p_cdb->vid;
    }
    
    return PM_E_NONE;
}

int32
pb_compose_binding_key_t_dump(Cdb__ComposeBindingKeyT *p_pb, char *out)
{
    int32 len = 0;

    if (p_pb->mac.data)
    {    
        len = sal_sprintf(out, "/binding_key=(mac: %.04hX.%.04hX.%.04hX, ipv6: %u, vid: %u)", 
            sal_ntoh16(((uint16*)p_pb->mac.data)[0]),
            sal_ntoh16(((uint16*)p_pb->mac.data)[1]),
            sal_ntoh16(((uint16*)p_pb->mac.data)[2]),
            p_pb->ipv6,
            p_pb->vid);
    }
    
    return len;
}

int32
pb_compose_binding_key_t_to_pb_free_packed(Cdb__ComposeBindingKeyT *p_pb)
{
    if (p_pb)
    {
        if (p_pb->mac.data)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->mac.data);
            p_pb->mac.data = NULL;
        }
        p_pb->mac.len = 0;
    }

    return PM_E_NONE;
}

int32
pb_compose_ns_port_forwarding_key_t_to_pb(ns_port_forwarding_key_t *p_cdb, Cdb__ComposeNsPortForwardingKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->type = p_cdb->type;
        p_pb->proto = p_cdb->proto;
        p_pb->port = p_cdb->port;
        p_pb->mapped_addr = p_cdb->mapped_addr.s_addr;
    }

    return PM_E_NONE;
}

int32
pb_compose_ns_port_forwarding_key_t_dump(Cdb__ComposeNsPortForwardingKeyT *p_pb, char *out)
{
    char str[CMD_BUF_64];
    addr_ipv4_t addr;

    int32 len = 0;
    
    if (p_pb)
    {
        addr.s_addr = p_pb->mapped_addr;
        len = sal_sprintf(out, "/ns_port_forwarding_key=(type=%u, proto=%u, port=%u, addr=%s)", 
            p_pb->type, 
            p_pb->proto, 
            p_pb->port, 
            cdb_addr_ipv4_val2str(str, CMD_BUF_64, &addr));
    }

    return len;
}

int32
pb_compose_ns_port_forwarding_key_t_to_pb_free_packed(Cdb__ComposeNsPortForwardingKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_ns_route_key_t_to_pb(ns_route_key_t *p_cdb, Cdb__ComposeNsRouteKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->port = p_cdb->port;
        p_pb->proto = p_cdb->proto;
        p_pb->family = p_cdb->ip.family;

        if (AF_INET == p_pb->family)
        {
            p_pb->ipv4_addr = p_cdb->ip.u.prefix4.s_addr;
        }
        else if (AF_INET6 == p_pb->family)
        {
            p_pb->n_ipv6_addr = sizeof(addr_ipv6_t);
            p_pb->ipv6_addr = XCALLOC(MEM_LIB_PROTOBUF, sizeof(addr_ipv6_t));
            sal_memcpy(p_pb->ipv6_addr, &p_cdb->ip.u.prefix6, sizeof(addr_ipv6_t));
        }
    }

    return PM_E_NONE;
}

int32
pb_compose_ns_route_key_t_dump(Cdb__ComposeNsRouteKeyT *p_pb, char *out)
{
    char str[CMD_BUF_64];
    int32 len = 0;
    
    if (p_pb)
    {
        if (AF_INET == p_pb->family)
        {
            len = sal_sprintf(out, "/ns_route_key=(proto=%u, port=%u, addr=%s)", 
                p_pb->proto, 
                p_pb->port, 
                cdb_addr_ipv4_val2str(str, CMD_BUF_64, (void*)&p_pb->ipv4_addr));
        }
        else
        {
             len = sal_sprintf(out, "/ns_route_key=(proto=%u, port=%u, addr=%s)", 
                p_pb->proto, 
                p_pb->port, 
                cdb_addr_ipv6_val2str(str, CMD_BUF_64, (void*)p_pb->ipv6_addr));
        }
    }

    return len;
}

int32
pb_compose_ns_route_key_t_to_pb_free_packed(Cdb__ComposeNsRouteKeyT *p_pb)
{
    if (p_pb)
    {
        if (AF_INET6 == p_pb->family)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->ipv6_addr);
            p_pb->ipv6_addr = NULL;
            p_pb->n_ipv6_addr = 0;
        }
    }
    
    return PM_E_NONE;
}

int32
pb_compose_openflow_manager_key_t_to_pb(openflow_manager_key_t *p_cdb, Cdb__ComposeOpenflowManagerKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->ip = p_cdb->ip.s_addr;
        p_pb->port = p_cdb->port;
        p_pb->proto = p_cdb->proto;
    }

    return PM_E_NONE;
}

int32
pb_compose_openflow_manager_key_t_dump(Cdb__ComposeOpenflowManagerKeyT *p_pb, char *out)
{
    char str[CMD_BUF_64];
    addr_ipv4_t addr;

    int32 len = 0;
    
    if (p_pb)
    {
        addr.s_addr = p_pb->ip;
        len = sal_sprintf(out, "/openflow_manager_key=(proto=%u, port=%u, addr=%s)", 
            p_pb->proto, 
            p_pb->port, 
            cdb_addr_ipv4_val2str(str, CMD_BUF_64, &addr));
    }

    return len;
}

int32
pb_compose_openflow_manager_key_t_to_pb_free_packed(Cdb__ComposeOpenflowManagerKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_mstp_digest_t_to_pb(mstp_digest_t *p_cdb, Cdb__ComposeMstpDigestT *p_pb)
{
    if (p_pb)
    {
        p_pb->data.len = MSTP_DIGEST_LEN;
        p_pb->data.data = XCALLOC(MEM_LIB_PROTOBUF, MSTP_DIGEST_LEN);
        sal_memcpy(p_pb->data.data, p_cdb->data, MSTP_DIGEST_LEN);
    }
    
    return PM_E_NONE;
}

int32
pb_compose_mstp_digest_t_dump(Cdb__ComposeMstpDigestT *p_pb, char *out)
{
    uint32 i = 0;
    int32 len = 0;
    
    if (p_pb)
    {
        len += sal_sprintf(out + len, "/mstp_digest=0x");
        if (p_pb->data.len)
        {
            for (i = 0; i < p_pb->data.len; i++)
            {
                len += sal_sprintf(out + len, "%02X", p_pb->data.data[i]);
            }
        }
    }

    return len;
}

int32
pb_compose_mstp_digest_t_to_pb_free_packed(Cdb__ComposeMstpDigestT *p_pb)
{
    if (p_pb)
    {
        if (p_pb->data.data)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->data.data);
            p_pb->data.data = NULL;
        }
        p_pb->data.len = 0;
    }

    return PM_E_NONE;
}

int32
pb_compose_ptp_default_t_to_pb(ptp_default_t *p_cdb, Cdb__ComposePtpDefaultT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_ptp_default_t_dump(Cdb__ComposePtpDefaultT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_ptp_default_t_to_pb_free_packed(Cdb__ComposePtpDefaultT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_ptp_current_t_to_pb(ptp_current_t *p_cdb, Cdb__ComposePtpCurrentT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_ptp_current_t_dump(Cdb__ComposePtpCurrentT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_ptp_current_t_to_pb_free_packed(Cdb__ComposePtpCurrentT *p_pb)
{

    return PM_E_NONE;
}
int32
pb_compose_ptp_parent_t_to_pb(ptp_parent_t *p_cdb, Cdb__ComposePtpParentT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_ptp_parent_t_dump(Cdb__ComposePtpParentT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_ptp_parent_t_to_pb_free_packed(Cdb__ComposePtpParentT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_ptp_time_properties_t_to_pb(ptp_time_properties_t *p_cdb, Cdb__ComposePtpTimePropertiesT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_ptp_time_properties_t_dump(Cdb__ComposePtpTimePropertiesT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_ptp_time_properties_t_to_pb_free_packed(Cdb__ComposePtpTimePropertiesT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_ptp_global_c_t_to_pb(ptp_global_c_t *p_cdb, Cdb__ComposePtpGlobalCT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_ptp_global_c_t_dump(Cdb__ComposePtpGlobalCT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_ptp_global_c_t_to_pb_free_packed(Cdb__ComposePtpGlobalCT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_ptp_port_t_to_pb(ptp_port_t *p_cdb, Cdb__ComposePtpPortT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_ptp_port_t_dump(Cdb__ComposePtpPortT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_ptp_port_t_to_pb_free_packed(Cdb__ComposePtpPortT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_glb_ptp_port_address_t_to_pb(glb_ptp_port_address_t *p_cdb, Cdb__ComposeGlbPtpPortAddressT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_glb_ptp_port_address_t_dump(Cdb__ComposeGlbPtpPortAddressT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_glb_ptp_port_address_t_to_pb_free_packed(Cdb__ComposeGlbPtpPortAddressT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_glb_ptp_timeinterval_t_to_pb(glb_ptp_timeinterval_t *p_cdb, Cdb__ComposeGlbPtpTimeintervalT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_glb_ptp_timeinterval_t_dump(Cdb__ComposeGlbPtpTimeintervalT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_glb_ptp_timeinterval_t_to_pb_free_packed(Cdb__ComposeGlbPtpTimeintervalT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_ptp_vlan_info_t_to_pb(ptp_vlan_info_t *p_cdb, Cdb__ComposePtpVlanInfoT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_ptp_vlan_info_t_dump(Cdb__ComposePtpVlanInfoT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_ptp_vlan_info_t_to_pb_free_packed(Cdb__ComposePtpVlanInfoT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_ptp_dump_flag_t_to_pb(ptp_dump_flag_t *p_cdb, Cdb__ComposePtpDumpFlagT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_ptp_dump_flag_t_dump(Cdb__ComposePtpDumpFlagT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_ptp_dump_flag_t_to_pb_free_packed(Cdb__ComposePtpDumpFlagT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_ptp_port_p_t_to_pb(ptp_port_p_t *p_cdb, Cdb__ComposePtpPortPT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_ptp_port_p_t_dump(Cdb__ComposePtpPortPT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_ptp_port_p_t_to_pb_free_packed(Cdb__ComposePtpPortPT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_ptp_port_c_t_to_pb(ptp_port_c_t *p_cdb, Cdb__ComposePtpPortCT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_ptp_port_c_t_dump(Cdb__ComposePtpPortCT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_ptp_port_c_t_to_pb_free_packed(Cdb__ComposePtpPortCT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_ptp_port_stats_t_to_pb(ptp_port_stats_t *p_cdb, Cdb__ComposePtpPortStatsT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_ptp_port_stats_t_dump(Cdb__ComposePtpPortStatsT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_ptp_port_stats_t_to_pb_free_packed(Cdb__ComposePtpPortStatsT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_ptp_msg_proc_ops_t_to_pb(ptp_msg_proc_ops_t *p_cdb, Cdb__ComposePtpMsgProcOpsT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_ptp_msg_proc_ops_t_dump(Cdb__ComposePtpMsgProcOpsT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_ptp_msg_proc_ops_t_to_pb_free_packed(Cdb__ComposePtpMsgProcOpsT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_kernel_if_ipv6_addr_t_to_pb(kernel_if_ipv6_addr_t *p_cdb, Cdb__ComposeKernelIfIpv6AddrT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_kernel_if_ipv6_addr_t_dump(Cdb__ComposeKernelIfIpv6AddrT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_kernel_if_ipv6_addr_t_to_pb_free_packed(Cdb__ComposeKernelIfIpv6AddrT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_brg_allowed_vlan_t_to_pb(brg_allowed_vlan_t *p_cdb, Cdb__ComposeBrgAllowedVlanT *p_pb)
{
    if (p_pb)
    {
        p_pb->vlan_bmp.len = sizeof(vlan_bmp_t);
        p_pb->vlan_bmp.data = XCALLOC(MEM_LIB_PROTOBUF, sizeof(vlan_bmp_t));
        sal_memcpy(p_pb->vlan_bmp.data, p_cdb->vlan_bmp, sizeof(vlan_bmp_t));
    }
    
    return PM_E_NONE;
}

int32
pb_compose_brg_allowed_vlan_t_dump(Cdb__ComposeBrgAllowedVlanT *p_pb, char *out)
{
    uint32 *bmp = NULL;
    uint32 word_len = 0;
    int32 len = 0;

    if (p_pb->vlan_bmp.data)
    {
        bmp = (uint32*)p_pb->vlan_bmp.data;
        word_len = p_pb->vlan_bmp.len / 4;
        cdb_bitmap_val2str(out, MSG_SIZE, bmp, word_len);
    }

    return len;
}

int32
pb_compose_brg_allowed_vlan_t_to_pb_free_packed(Cdb__ComposeBrgAllowedVlanT *p_pb)
{
    if (p_pb->vlan_bmp.data)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->vlan_bmp.data);
        p_pb->vlan_bmp.data = NULL;
    }
    p_pb->vlan_bmp.len = 0;
        
    return PM_E_NONE;
}

int32
pb_compose_tpoam_dm_t_to_pb(tpoam_dm_t *p_cdb, Cdb__ComposeTpoamDmT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_dm_t_dump(Cdb__ComposeTpoamDmT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_tpoam_dm_t_to_pb_free_packed(Cdb__ComposeTpoamDmT *p_pb)
{

        
    return PM_E_NONE;
}

int32
pb_compose_tpoam_lm_t_to_pb(tpoam_lm_t *p_cdb, Cdb__ComposeTpoamLmT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_lm_t_dump(Cdb__ComposeTpoamLmT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_tpoam_lm_t_to_pb_free_packed(Cdb__ComposeTpoamLmT *p_pb)
{

        
    return PM_E_NONE;
}

int32
pb_compose_tpoam_lb_t_to_pb(tpoam_lb_t *p_cdb, Cdb__ComposeTpoamLbT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_lb_t_dump(Cdb__ComposeTpoamLbT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_tpoam_lb_t_to_pb_free_packed(Cdb__ComposeTpoamLbT *p_pb)
{

        
    return PM_E_NONE;
}

int32
pb_compose_tpoam_event_t_to_pb(tpoam_event_t *p_cdb, Cdb__ComposeTpoamEventT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_event_t_dump(Cdb__ComposeTpoamEventT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_tpoam_event_t_to_pb_free_packed(Cdb__ComposeTpoamEventT *p_pb)
{

        
    return PM_E_NONE;
}

int32
pb_compose_tpoam_stats_t_to_pb(tpoam_stats_t *p_cdb, Cdb__ComposeTpoamStatsT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_stats_t_dump(Cdb__ComposeTpoamStatsT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_tpoam_stats_t_to_pb_free_packed(Cdb__ComposeTpoamStatsT *p_pb)
{

        
    return PM_E_NONE;
}

int32
pb_compose_tpoam_list_t_to_pb(tpoam_list_t *p_cdb, Cdb__ComposeTpoamListT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_list_t_dump(Cdb__ComposeTpoamListT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_tpoam_list_t_to_pb_free_packed(Cdb__ComposeTpoamListT *p_pb)
{

        
    return PM_E_NONE;
}

int32
pb_compose_tpoam_rc_count_t_to_pb(tpoam_rc_count_t *p_cdb, Cdb__ComposeTpoamRcCountT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_rc_count_t_dump(Cdb__ComposeTpoamRcCountT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_tpoam_rc_count_t_to_pb_free_packed(Cdb__ComposeTpoamRcCountT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_exec_t_to_pb(tpoam_exec_t *p_cdb, Cdb__ComposeTpoamExecT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_exec_t_dump(Cdb__ComposeTpoamExecT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_tpoam_exec_t_to_pb_free_packed(Cdb__ComposeTpoamExecT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_trap_cache_t_to_pb(tpoam_trap_cache_t *p_cdb, Cdb__ComposeTpoamTrapCacheT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_trap_cache_t_dump(Cdb__ComposeTpoamTrapCacheT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_tpoam_trap_cache_t_to_pb_free_packed(Cdb__ComposeTpoamTrapCacheT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_ratelimit_t_to_pb(tpoam_ratelimit_t *p_cdb, Cdb__ComposeTpoamRatelimitT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_ratelimit_t_dump(Cdb__ComposeTpoamRatelimitT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_tpoam_ratelimit_t_to_pb_free_packed(Cdb__ComposeTpoamRatelimitT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_lmm_exec_t_to_pb(tpoam_lmm_exec_t*p_cdb, Cdb__ComposeTpoamLmmExecT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_lmm_exec_t_dump(Cdb__ComposeTpoamLmmExecT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_tpoam_lmm_exec_t_to_pb_free_packed(Cdb__ComposeTpoamLmmExecT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_lb_exec_t_to_pb(tpoam_lb_exec_t*p_cdb, Cdb__ComposeTpoamLbExecT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_lb_exec_t_dump(Cdb__ComposeTpoamLbExecT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_tpoam_lb_exec_t_to_pb_free_packed(Cdb__ComposeTpoamLbExecT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_1dm_dmm_exec_t_to_pb(tpoam_1dm_dmm_exec_t*p_cdb, Cdb__ComposeTpoam1dmDmmExecT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_tpoam_1dm_dmm_exec_t_dump(Cdb__ComposeTpoam1dmDmmExecT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_tpoam_1dm_dmm_exec_t_to_pb_free_packed(Cdb__ComposeTpoam1dmDmmExecT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_upload_tpoam_dm_t_to_pb(upload_tpoam_dm_t*p_cdb, Cdb__ComposeUploadTpoamDmT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_upload_tpoam_dm_t_dump(Cdb__ComposeUploadTpoamDmT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_upload_tpoam_dm_t_to_pb_free_packed(Cdb__ComposeUploadTpoamDmT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_upload_tpoam_lm_t_to_pb(upload_tpoam_lm_t*p_cdb, Cdb__ComposeUploadTpoamLmT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_upload_tpoam_lm_t_dump(Cdb__ComposeUploadTpoamLmT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_upload_tpoam_lm_t_to_pb_free_packed(Cdb__ComposeUploadTpoamLmT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_upload_tpoam_ccm_t_to_pb(upload_tpoam_ccm_t*p_cdb, Cdb__ComposeUploadTpoamCcmT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_upload_tpoam_ccm_t_dump(Cdb__ComposeUploadTpoamCcmT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_upload_tpoam_ccm_t_to_pb_free_packed(Cdb__ComposeUploadTpoamCcmT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_ptp_common_message_header_t_to_pb(ptp_common_message_header_t *p_cdb, Cdb__ComposePtpCommonMessageHeaderT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_ptp_common_message_header_t_dump(Cdb__ComposePtpCommonMessageHeaderT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_ptp_common_message_header_t_to_pb_free_packed(Cdb__ComposePtpCommonMessageHeaderT *p_pb)
{

        
    return PM_E_NONE;
}

int32
pb_compose_ptp_msg_announce_t_to_pb(ptp_msg_announce_t *p_cdb, Cdb__ComposePtpMsgAnnounceT *p_pb)
{

    
    return PM_E_NONE;
}

int32
pb_compose_ptp_msg_announce_t_dump(Cdb__ComposePtpMsgAnnounceT *p_pb, char *out)
{


    return 10;
}

int32
pb_compose_ptp_msg_announce_t_to_pb_free_packed(Cdb__ComposePtpMsgAnnounceT *p_pb)
{

        
    return PM_E_NONE;
}

int32
pb_compose_glb_board_type_t_to_pb(glb_board_type_t *p_cdb, Cdb__ComposeGlbBoardTypeT *p_pb)
{
    p_pb->series = p_cdb->series;
    p_pb->type = p_cdb->type;
    
    return PM_E_NONE;
}

int32
pb_compose_glb_board_type_t_dump(Cdb__ComposeGlbBoardTypeT *p_pb, char *out)
{
    int32 len = 0;
    
    len = sal_sprintf(out, "/series=%u, type=%u", p_pb->series, p_pb->type);

    return len;
}

int32
pb_compose_glb_board_type_t_to_pb_free_packed(Cdb__ComposeGlbBoardTypeT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_time_range_value_t_to_pb(time_range_value_t *p_cdb, Cdb__ComposeTimeRangeValueT *p_pb)
{
    p_pb->type = p_cdb->type;
    
    if (GLB_TIME_RANGE_TYPE_ABSOLUTE == p_cdb->type)
    {
        p_pb->has_absolute_start = TRUE;
        p_pb->absolute_start = p_cdb->ab_time.start;
        p_pb->absolute_end = TRUE;
        p_pb->absolute_end = p_cdb->ab_time.end;
    }
    else if (GLB_TIME_RANGE_TYPE_ABSOLUTE == p_cdb->type)
    {
        p_pb->has_periodic_start_day = TRUE;
        p_pb->periodic_start_day = p_cdb->per_time.start.day;
        p_pb->has_periodic_start_hour = TRUE;
        p_pb->periodic_start_hour = p_cdb->per_time.start.hour;
        p_pb->has_periodic_start_min = TRUE;
        p_pb->periodic_start_min = p_cdb->per_time.start.min;
        p_pb->has_periodic_end_day = TRUE;
        p_pb->periodic_end_day = p_cdb->per_time.end.day;
        p_pb->has_periodic_end_hour = TRUE;
        p_pb->periodic_end_hour = p_cdb->per_time.end.hour;
        p_pb->has_periodic_end_min = TRUE;
        p_pb->periodic_end_min = p_cdb->per_time.end.min;
    }
    else
    {
        return PM_E_FAIL;
    }
    
    return PM_E_NONE;
}

int32
pb_compose_time_range_value_t_dump(Cdb__ComposeTimeRangeValueT *p_pb, char *out)
{
    int32 len = 0;

    if (GLB_TIME_RANGE_TYPE_ABSOLUTE == p_pb->type)
    {
        len = sal_sprintf(out, "/time_range_value=(type=absolute, start=%d, end=%d)", 
            p_pb->absolute_start, p_pb->absolute_end);
    }
    else if (GLB_TIME_RANGE_TYPE_PERIODIC == p_pb->type)
    {
        len = sal_sprintf(out, "/time_range_value=(type=periodic, start(day,hour,min)=(%d,%d,%d), end=(%d,%d,%d))",
            p_pb->periodic_start_day, p_pb->periodic_start_hour, p_pb->periodic_start_min,
            p_pb->periodic_end_day, p_pb->periodic_end_hour, p_pb->periodic_end_min);
    }

    return len;
}

int32
pb_compose_time_range_value_t_to_pb_free_packed(Cdb__ComposeTimeRangeValueT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_time_range_timer_t_to_pb(time_range_timer_t *p_cdb, Cdb__ComposeTimeRangeTimerT *p_pb)
{
    p_pb->status = p_cdb->status;
    
    return PM_E_NONE;
}

int32
pb_compose_time_range_timer_t_dump(Cdb__ComposeTimeRangeTimerT *p_pb, char *out)
{
    int32 len = 0;
    
    len = sal_sprintf(out, "/time_range_timer=(status=%u)", p_pb->status);
    return len;
}

int32
pb_compose_time_range_timer_t_to_pb_free_packed(Cdb__ComposeTimeRangeTimerT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_snmp_trap_key_t_to_pb(snmp_trap_key_t *p_cdb, Cdb__ComposeSnmpTrapKeyT *p_pb)
{
    Cdb__ComposeAddrT tmp = CDB__COMPOSE_ADDR_T__INIT;

    if (p_pb)
    {
        p_pb->ip_addr = XCALLOC(MEM_LIB_PROTOBUF, sizeof(Cdb__ComposeAddrT));
        *(p_pb->ip_addr) = tmp;
        pb_compose_addr_t_to_pb(&p_cdb->ip_addr, p_pb->ip_addr);
        p_pb->community = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_cdb->community)+1);
        sal_strcpy(p_pb->community, p_cdb->community);
        
        p_pb->udpport = p_cdb->udpport;
    }

    return PM_E_NONE;
}

int32
pb_compose_snmp_trap_key_t_dump(Cdb__ComposeSnmpTrapKeyT *p_pb, char *out)
{
    int32 len = 0;

    if (p_pb)
    {   
        len += sal_sprintf(out + len, "/snmp_trap_key_t=(");
        len += pb_compose_addr_t_dump(p_pb->ip_addr, out + len);
        len += sal_sprintf(out + len, "community=%s, udpport=%u)", p_pb->community, p_pb->udpport);
    }
    
    return len;
}

int32
pb_compose_snmp_trap_key_t_to_pb_free_packed(Cdb__ComposeSnmpTrapKeyT *p_pb)
{
    if (p_pb)
    {
        if (p_pb->ip_addr)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->ip_addr);
            p_pb->ip_addr = NULL;
        }
        if (p_pb->community)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->community);
            p_pb->community = NULL;
        }
    }

    return PM_E_NONE;
}

int32
pb_compose_snmp_view_key_t_to_pb(snmp_view_key_t *p_cdb, Cdb__ComposeSnmpViewKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_cdb->name)+1);
        sal_strcpy(p_pb->name, p_cdb->name);

        p_pb->subtree = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_cdb->subtree)+1);
        sal_strcpy(p_pb->subtree, p_cdb->subtree);
    }

    return PM_E_NONE;
}

int32
pb_compose_snmp_view_key_t_dump(Cdb__ComposeSnmpViewKeyT *p_pb, char *out)
{
    int32 len = 0;

    if (p_pb)
    {   
        len += sal_sprintf(out + len, "/snmp_view_key_t=(");
        len += sal_sprintf(out + len, "name=%s, subtree=%s)", 
            p_pb->name, p_pb->subtree);
    }
    
    return len;
}

int32
pb_compose_snmp_view_key_t_to_pb_free_packed(Cdb__ComposeSnmpViewKeyT *p_pb)
{
    if (p_pb)
    {
        if (p_pb->name)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->name);
            p_pb->name= NULL;
        }

        if (p_pb->subtree)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->subtree);
            p_pb->subtree= NULL;
        }
    }

    return PM_E_NONE;
}

int32
pb_compose_snmp_group_usm_key_t_to_pb(snmp_group_usm_key_t *p_cdb, Cdb__ComposeSnmpGroupUsmKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->grp_name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_cdb->grp_name)+1);
        sal_strcpy(p_pb->grp_name, p_cdb->grp_name);

        p_pb->security_name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_cdb->security_name)+1);
        sal_strcpy(p_pb->security_name, p_cdb->security_name);
    }

    return PM_E_NONE;
}

int32
pb_compose_snmp_group_usm_key_t_dump(Cdb__ComposeSnmpGroupUsmKeyT *p_pb, char *out)
{
    int32 len = 0;

    if (p_pb)
    {   
        len += sal_sprintf(out + len, "/snmp_group_usm_key_t=(");
        len += sal_sprintf(out + len, "grp_name=%s, security_name=%s)", 
            p_pb->grp_name, p_pb->security_name);
    }
    
    return len;
}

int32
pb_compose_snmp_group_usm_key_t_to_pb_free_packed(Cdb__ComposeSnmpGroupUsmKeyT *p_pb)
{
    if (p_pb)
    {
        if (p_pb->grp_name)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->grp_name);
            p_pb->grp_name = NULL;
        }

        if (p_pb->security_name)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->security_name);
            p_pb->security_name = NULL;
        }
    }

    return PM_E_NONE;
}


int32
pb_compose_snmp_inform_key_t_to_pb(snmp_inform_key_t *p_cdb, Cdb__ComposeSnmpInformKeyT *p_pb)
{
    Cdb__ComposeAddrT tmp = CDB__COMPOSE_ADDR_T__INIT;

    if (p_pb)
    {
        p_pb->ip_addr = XCALLOC(MEM_LIB_PROTOBUF, sizeof(Cdb__ComposeAddrT));
        *(p_pb->ip_addr) = tmp;
        pb_compose_addr_t_to_pb(&p_cdb->ip_addr, p_pb->ip_addr);
        p_pb->community = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_cdb->community)+1);
        sal_strcpy(p_pb->community, p_cdb->community);
        
        p_pb->udpport = p_cdb->udpport;
    }

    return PM_E_NONE;
}

int32
pb_compose_snmp_inform_key_t_dump(Cdb__ComposeSnmpInformKeyT *p_pb, char *out)
{
    int32 len = 0;

    if (p_pb)
    {   
        len += sal_sprintf(out + len, "/snmp_inform_key_t=(");
        len += pb_compose_addr_t_dump(p_pb->ip_addr, out + len);
        len += sal_sprintf(out + len, "community=%s, udpport=%u)", p_pb->community, p_pb->udpport);
    }
    
    return len;
}

int32
pb_compose_snmp_inform_key_t_to_pb_free_packed(Cdb__ComposeSnmpInformKeyT *p_pb)
{
    if (p_pb)
    {
        if (p_pb->ip_addr)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->ip_addr);
            p_pb->ip_addr = NULL;
        }
        if (p_pb->community)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->community);
            p_pb->community = NULL;
        }
    }
    
    return PM_E_NONE;
}

int32
pb_compose_rsa_keystr_t_to_pb(rsa_keystr_t *p_cdb, Cdb__ComposeRsaKeystrT *p_pb)
{
    if (p_pb)
    {
        p_pb->rsa.len = sizeof(rsa_keystr_t);
        p_pb->rsa.data = XCALLOC(MEM_LIB_PROTOBUF, sizeof(rsa_keystr_t));
        sal_memcpy(p_pb->rsa.data, p_cdb->buf, sizeof(rsa_keystr_t));
    }
    
    return PM_E_NONE;
}

int32
pb_compose_rsa_keystr_t_dump(Cdb__ComposeRsaKeystrT *p_pb, char *out)
{
    int32 len = 0;
    
    len = sal_sprintf(out, "RSA has %u bytes, omit to dump", M_RSA_PEM_BUF_SIZE);
    return len;
}

int32
pb_compose_rsa_keystr_t_to_pb_free_packed(Cdb__ComposeRsaKeystrT *p_pb)
{
    if (p_pb->rsa.data)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->rsa.data);
        p_pb->rsa.data = NULL;
    }
    p_pb->rsa.len = 0;

    return PM_E_NONE;
}

int32
pb_compose_timestamp_t_to_pb(timestamp_t *p_cdb, Cdb__ComposeTimestampT *p_pb)
{
    p_pb->tv_sec = p_cdb->tv_sec;
    p_pb->tv_usec = p_cdb->tv_usec;
    
    return PM_E_NONE;
}

int32
pb_compose_timestamp_t_dump(Cdb__ComposeTimestampT *p_pb, char *out)
{
    char str[CMD_BUF_64];
    timestamp_t ts;
    int32 len = 0;
    
    ts.tv_sec = p_pb->tv_sec;
    ts.tv_usec = p_pb->tv_usec;
    cdb_timestamp_val2str(str, CMD_BUF_64, &ts);
    len = sal_sprintf(out, "%s", str);

    return len;
}

int32
pb_compose_timestamp_t_to_pb_free_packed(Cdb__ComposeTimestampT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_mail_t_to_pb(mail_t *p_cdb, Cdb__ComposeMailT *p_pb)
{
    if (p_pb)
    {
        p_pb->server = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_cdb->server)+1);
        sal_strcpy(p_pb->server, p_cdb->server);
        p_pb->from = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_cdb->from)+1);
        sal_strcpy(p_pb->from, p_cdb->from);
        p_pb->to = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_cdb->to)+1);
        sal_strcpy(p_pb->to, p_cdb->to);
        p_pb->cc = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_cdb->cc)+1);
        sal_strcpy(p_pb->cc, p_cdb->cc);
        p_pb->subject = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_cdb->subject)+1);
        sal_strcpy(p_pb->subject, p_cdb->subject);
        p_pb->body = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_cdb->body)+1);
        sal_strcpy(p_pb->body, p_cdb->body);
        p_pb->attach = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_cdb->attach)+1);
        sal_strcpy(p_pb->attach, p_cdb->attach);
    }
    
    return PM_E_NONE;
}

int32
pb_compose_mail_t_dump(Cdb__ComposeMailT *p_pb, char *out)
{
    int32 len = 0;

    if (p_pb)
    {    
        len += sal_sprintf(out + len, "/mail_t=(server=%s, from=%s, to=%s, cc=%s, subject=%s, body=%s, attach=%s)", 
            p_pb->server, p_pb->from, p_pb->to, p_pb->cc, p_pb->subject, p_pb->body, p_pb->attach);
    }
    
    return len;
}

int32
pb_compose_mail_t_to_pb_free_packed(Cdb__ComposeMailT *p_pb)
{
    if (p_pb)
    {
        if (p_pb->server)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->server);
            p_pb->server = NULL;
        }
        if (p_pb->from)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->from);
            p_pb->from = NULL;
        }
        if (p_pb->to)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->to);
            p_pb->to = NULL;
        }
        if (p_pb->cc)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->cc);
            p_pb->cc = NULL;
        }
        if (p_pb->subject)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->subject);
            p_pb->subject = NULL;
        }
        if (p_pb->body)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->body);
            p_pb->body = NULL;
        }
        if (p_pb->attach)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->attach);
            p_pb->attach = NULL;
        }
    }

    return PM_E_NONE;
}

int32
pb_compose_mstp_brg_id_t_to_pb(mstp_brg_id_t *p_cdb, Cdb__ComposeMstpBrgIdT *p_pb)
{
    if (p_pb)
    {
        p_pb->priority = p_cdb->priority;
        p_pb->address.len = sizeof(p_cdb->address);
        p_pb->address.data = XCALLOC(MEM_LIB_PROTOBUF, sizeof(p_cdb->address));
        sal_memcpy(p_pb->address.data, p_cdb->address, sizeof(p_cdb->address));
    }
    
    return PM_E_NONE;
}

int32
pb_compose_mstp_brg_id_t_dump(Cdb__ComposeMstpBrgIdT *p_pb, char *out)
{
    int32 len = 0;

    if (p_pb->address.data)
    {    
        len = sal_sprintf(out, "/mstp_brg_id_t=(address: %.04hX.%.04hX.%.04hX, priority: %u)", 
            sal_ntoh16(((uint16*)p_pb->address.data)[0]),
            sal_ntoh16(((uint16*)p_pb->address.data)[1]),
            sal_ntoh16(((uint16*)p_pb->address.data)[2]),
            p_pb->priority);
    }
    
    return len;
}

int32
pb_compose_mstp_brg_id_t_to_pb_free_packed(Cdb__ComposeMstpBrgIdT *p_pb)
{
    if (p_pb)
    {
        if (p_pb->address.data)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->address.data);
            p_pb->address.data = NULL;
        }
        p_pb->address.len = 0;
    }

    return PM_E_NONE;
}

int32
pb_compose_glb_stats_t_to_pb(glb_stats_t *p_cdb, Cdb__ComposeGlbStatsT *p_pb)
{
    p_pb->packet_count = p_cdb->packet_count;
    p_pb->byte_count = p_cdb->byte_count;

    return PM_E_NONE;
}

int32
pb_compose_glb_stats_t_dump(Cdb__ComposeGlbStatsT *p_pb, char *out)
{
    int32 len = 0;

    len = sal_sprintf(out, "/packet_count=%"PRIu64", byte_count=%"PRIu64, p_pb->packet_count, p_pb->byte_count);

    return len;
}

int32
pb_compose_glb_stats_t_to_pb_free_packed(Cdb__ComposeGlbStatsT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_glb_flow_l4_port_t_to_pb(glb_flow_l4_port_t *p_cdb, Cdb__ComposeGlbFlowL4PortT *p_pb)
{
    p_pb->operator_ = p_cdb->operator;
    p_pb->port = p_cdb->port;
    p_pb->port2 = p_cdb->port2;

    return PM_E_NONE;
}

int32
pb_compose_glb_flow_l4_port_t_dump(Cdb__ComposeGlbFlowL4PortT *p_pb, char *out)
{
    int32 len = 0;

    len = sal_sprintf(out, "/operator_=%u, port=%u, port2=%u", p_pb->operator_, p_pb->port, p_pb->port2);

    return len;
}

int32
pb_compose_glb_flow_l4_port_t_to_pb_free_packed(Cdb__ComposeGlbFlowL4PortT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_glb_flow_tcp_flag_t_to_pb(glb_flow_tcp_flag_t *p_cdb, Cdb__ComposeGlbFlowTcpFlagT *p_pb)
{
    p_pb->operator_ = p_cdb->operator;
    p_pb->tcp_flags = p_cdb->tcp_flags;
    
    return PM_E_NONE;
}

int32
pb_compose_glb_flow_tcp_flag_t_dump(Cdb__ComposeGlbFlowTcpFlagT *p_pb, char *out)
{
    int32 len = 0;
    
    len = sal_sprintf(out, "/operator_=%u, tcp_flags=%u", p_pb->operator_, p_pb->tcp_flags);

    return len;
}

int32
pb_compose_glb_flow_tcp_flag_t_to_pb_free_packed(Cdb__ComposeGlbFlowTcpFlagT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_sal_time_t_to_pb(sal_time_t *p_cdb, Cdb__ComposeSalTimeT *p_pb)
{
    p_pb->time = (uint32)*p_cdb;
    return PM_E_NONE;
}

int32
pb_compose_sal_time_t_dump(Cdb__ComposeSalTimeT *p_pb, char *out)
{
    int32 len = 0;
    
    len = sal_sprintf(out, "/time=%u", p_pb->time);
        
    return len;
}

int32
pb_compose_sal_time_t_to_pb_free_packed(Cdb__ComposeSalTimeT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_prefix_t_to_pb(prefix_t *p_cdb, Cdb__ComposePrefixT *p_pb)
{
    if (p_pb)
    {
        p_pb->family = p_cdb->family;
        p_pb->prefixlen = p_cdb->prefixlen;
        
        if (AF_INET == p_pb->family)
        {
            p_pb->ipv4_addr = p_cdb->u.prefix4.s_addr;
        }
        else if (AF_INET6 == p_pb->family)
        {
            p_pb->n_ipv6_addr = sizeof(addr_ipv6_t);
            p_pb->ipv6_addr = XCALLOC(MEM_LIB_PROTOBUF, sizeof(addr_ipv6_t));
            sal_memcpy(p_pb->ipv6_addr, &p_cdb->u.prefix6, sizeof(addr_ipv6_t));
        }
    }

    return PM_E_NONE;
}
int32
pb_compose_summer_time_t_to_pb(summer_time_t * p_tm,Cdb__ComposeSummerTimeT *tm)
{
    return PM_E_NONE;
}
int32
pb_compose_which_summer_time_t_to_pb(which_summer_time_t * p_wtm,Cdb__ComposeWhichSummerTimeT *wtm)
{
    return PM_E_NONE;
}
int32
pb_compose_summer_time_t_to_pb_free_packed(Cdb__ComposeSummerTimeT *p_tm)
{
    return PM_E_NONE;
}
int32
pb_compose_which_summer_time_t_to_pb_free_packed(Cdb__ComposeWhichSummerTimeT *p_wtm)
{
    return PM_E_NONE;
}
int32
pb_compose_summer_time_t_dump(Cdb__ComposeSummerTimeT *p_tm,char* out)
{
    return PM_E_NONE;
}
int32
pb_compose_which_summer_time_t_dump(Cdb__ComposeWhichSummerTimeT  *p_wtm,char* out)
{
    return PM_E_NONE;
}



int32
pb_compose_prefix_t_dump(Cdb__ComposePrefixT *p_pb, char *out)
{
    int32 len = 0;
    char str[CMD_BUF_32];
    
    if (AF_INET == p_pb->family)
    {
        /* IPv4 */
        len = sal_sprintf(out, "/addr_t=(family=ipv4, prefixlen=%u, addr=%s)", 
            p_pb->prefixlen, sal_inet_ntop(AF_INET, (void*)&(p_pb->ipv4_addr), str, CMD_BUF_32));
    }
    else if (AF_INET6 == p_pb->family)
    {
        /* IPv6 */
        len = sal_sprintf(out, "/addr_t=(family=ipv6, prefixlen=%u, addr=%s)", 
            p_pb->prefixlen, "ipv6_todo");
    }

    return len;
}

int32
pb_compose_prefix_t_to_pb_free_packed(Cdb__ComposePrefixT *p_pb)
{
    if (p_pb)
    {
        if (AF_INET6 == p_pb->family)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->ipv6_addr);
            p_pb->ipv6_addr = NULL;
            p_pb->n_ipv6_addr = 0;
        }
    }

    return PM_E_NONE;
}

int32
pb_compose_prefix_ipv4_t_to_pb(prefix_ipv4_t *p_cdb, Cdb__ComposePrefixIpv4T *p_pb)
{
    if (p_pb)
    {
        p_pb->family = p_cdb->family;
        p_pb->prefixlen = p_cdb->prefixlen;
        p_pb->ipv4_addr = p_cdb->prefix.s_addr;
    }

    return PM_E_NONE;
}

int32
pb_compose_prefix_ipv4_t_dump(Cdb__ComposePrefixIpv4T *p_pb, char *out)
{
    int32 len = 0;
    char str[CMD_BUF_32];
    
    if (AF_INET == p_pb->family)
    {
        /* IPv4 */
        len = sal_sprintf(out, "/addr_t=(family=ipv4, prefixlen=%u, addr=%s)", 
            p_pb->prefixlen, sal_inet_ntop(AF_INET, (void*)&(p_pb->ipv4_addr), str, CMD_BUF_32));
    }

    return len;
}

int32
pb_compose_prefix_ipv4_t_to_pb_free_packed(Cdb__ComposePrefixIpv4T *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_addr_t_to_pb(addr_t *p_cdb, Cdb__ComposeAddrT *p_pb)
{
    if (p_pb)
    {
        p_pb->family = p_cdb->family;

        if (AF_INET == p_pb->family)
        {
            p_pb->ipv4_addr = p_cdb->u.prefix4.s_addr;
        }
        else if (AF_INET6 == p_pb->family)
        {
            p_pb->n_ipv6_addr = sizeof(addr_ipv6_t);
            p_pb->ipv6_addr = XCALLOC(MEM_LIB_PROTOBUF, sizeof(addr_ipv6_t));
            sal_memcpy(p_pb->ipv6_addr, &p_cdb->u.prefix6, sizeof(addr_ipv6_t));
        }
    }

    return PM_E_NONE;
}

int32
pb_compose_addr_t_dump(Cdb__ComposeAddrT *p_pb, char *out)
{
    int32 len = 0;
    char str[CMD_BUF_32];
    
    if (AF_INET == p_pb->family)
    {
        /* IPv4 */
        len = sal_sprintf(out, "/addr_t=(family=ipv4, addr=%s)", 
            sal_inet_ntop(AF_INET, (void*)&(p_pb->ipv4_addr), str, CMD_BUF_32));
    }
    else if (AF_INET6 == p_pb->family)
    {
        /* IPv6 */
        len = sal_sprintf(out, "/addr_t=(family=ipv6, addr=%s)", 
            "ipv6_todo");
    }

    return len;
}

int32
pb_compose_addr_t_to_pb_free_packed(Cdb__ComposeAddrT *p_pb)
{
    if (p_pb)
    {
        if (AF_INET6 == p_pb->family)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->ipv6_addr);
            p_pb->ipv6_addr = NULL;
            p_pb->n_ipv6_addr = 0;
        }
    }

    return PM_E_NONE;
}

int32
pb_compose_snmp_info_t_to_pb(snmp_info_t *p_cdb, Cdb__ComposeSnmpInfoT *p_pb)
{
    if (p_cdb && p_pb) {
        if (strlen(p_cdb->info) > 0) {
            p_pb->info = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->info);
        } else {
            p_pb->info = XSTRDUP(MEM_LIB_PROTOBUF, "");
        }
    }

    return PM_E_NONE;
}

int32
pb_compose_snmp_info_t_dump(Cdb__ComposeSnmpInfoT *p_pb, char *out)
{
    char str[SNMP_INFO_SIZE * 2];

    if (NULL == p_pb->info || strlen(p_pb->info) <= 0) {
        snprintf(str, SNMP_INFO_SIZE * 2, "%s", "");
    } else {
        sal_b64_ntop((unsigned char *)p_pb->info, strlen(p_pb->info),
            str, SNMP_INFO_SIZE * 2);
    }

    return sal_sprintf(out, "/snmp_info_t=(%s)", str);
}

int32
pb_compose_snmp_info_t_to_pb_free_packed(Cdb__ComposeSnmpInfoT *p_pb)
{
    if (p_pb && p_pb->info)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->info);
    }

    return PM_E_NONE;
}

int32
pb_compose_user_t_to_pb(user_t *p_cdb, Cdb__ComposeUserT *p_pb)
{
    if (p_cdb && p_pb) {
        if (strlen(p_cdb->name) > 0) {
            p_pb->name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->name);
        } else {
            p_pb->name = XSTRDUP(MEM_LIB_PROTOBUF, "");
        }
    }

    return PM_E_NONE;
}

int32
pb_compose_user_t_dump(Cdb__ComposeUserT *p_pb, char *out)
{
    char str[USERNAME_SIZE * 2];

    if (NULL == p_pb->name || strlen(p_pb->name) <= 0) {
        snprintf(str, USERNAME_SIZE * 2, "%s", "");
    } else {
        sal_b64_ntop((unsigned char *)p_pb->name, strlen(p_pb->name),
            str, USERNAME_SIZE * 2);
    }

    return sal_sprintf(out, "/user_t=(%s)", str);
}

int32
pb_compose_user_t_to_pb_free_packed(Cdb__ComposeUserT *p_pb)
{
    if (p_pb && p_pb->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->name);
    }

    return PM_E_NONE;
}



int32
pb_compose_dot1x_radius_key_t_to_pb(dot1x_radius_key_t *p_cdb, Cdb__ComposeDot1xRadiusKeyT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_dot1x_radius_key_t_dump(Cdb__ComposeDot1xRadiusKeyT *p_pb, char *out)
{
    int32 len = 0;

    return len;
}

int32
pb_compose_dot1x_radius_key_t_to_pb_free_packed(Cdb__ComposeDot1xRadiusKeyT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_sflow_collector_key_t_to_pb(sflow_collector_key_t *p_cdb, Cdb__ComposeSflowCollectorKeyT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_sflow_collector_key_t_dump(Cdb__ComposeSflowCollectorKeyT *p_pb, char *out)
{
    int32 len = 0;

    return len;
}

int32
pb_compose_sflow_collector_key_t_to_pb_free_packed(Cdb__ComposeSflowCollectorKeyT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_dot1x_mac_key_t_to_pb(dot1x_mac_key_t *p_cdb, Cdb__ComposeDot1xMacKeyT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_dot1x_mac_key_t_dump(Cdb__ComposeDot1xMacKeyT *p_pb, char *out)
{
    int32 len = 0;

    return len;
}

int32
pb_compose_dot1x_mac_key_t_to_pb_free_packed(Cdb__ComposeDot1xMacKeyT *p_pb)
{

    return PM_E_NONE;
}


int32
pb_compose_addr_ipv4_t_to_pb(addr_ipv4_t *p_cdb, Cdb__ComposeAddrIpv4T *p_pb)
{
    p_pb->ipv4_addr = p_cdb->s_addr;
    return PM_E_NONE;
}

int32
pb_compose_addr_ipv4_t_dump(Cdb__ComposeAddrIpv4T *p_pb, char *out)
{
    char str[CMD_BUF_32];
    addr_ipv4_t addr;
    int32 len = 0;

    addr.s_addr = p_pb->ipv4_addr;
    str[0] = CMD_ZERO_CHAR;
    sal_inet_ntop(AF_INET, (void*)&addr, str, CMD_BUF_32);
    len = sal_sprintf(out, "%s", str);
    return len;
}

int32
pb_compose_addr_ipv4_t_to_pb_free_packed(Cdb__ComposeAddrIpv4T *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_addr_ipv6_t_to_pb(addr_ipv6_t *p_cdb, Cdb__ComposeAddrIpv6T *p_pb)
{
    //p_pb->ipv6_addr = p_cdb->s6_addr;
    return PM_E_NONE;
}

int32
pb_compose_addr_ipv6_t_dump(Cdb__ComposeAddrIpv6T *p_pb, char *out)
{
    //char str[CMD_BUF_64];
    //addr_ipv6_t addr;
    int32 len = 0;

    //addr.s_addr = p_pb->ipv4_addr;
    //str[0] = CMD_ZERO_CHAR;
    //sal_inet_ntop(AF_INET, (void*)&addr, str, CMD_BUF_64);
    //len = sal_sprintf(out, "%s", str);
    return len;
}

int32
pb_compose_addr_ipv6_t_to_pb_free_packed(Cdb__ComposeAddrIpv6T *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_nexthop_key_t_to_pb(nexthop_key_t *p_cdb, Cdb__ComposeNexthopKeyT *p_pb)
{
    p_pb->type = p_cdb->type;

    p_pb->ifname = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->ifname);
        
    if (NH_TYPE_IPV4 == p_pb->type)
    {
        p_pb->has_ipv4_addr = TRUE;
        p_pb->ipv4_addr = p_cdb->u.ipv4.s_addr;
    }
    else if (NH_TYPE_IPV6 == p_pb->type)
    {
        p_pb->n_ipv6_addr = sizeof(addr_ipv6_t);
        p_pb->ipv6_addr = XCALLOC(MEM_LIB_PROTOBUF, sizeof(addr_ipv6_t));
        sal_memcpy(p_pb->ipv6_addr, &p_cdb->u.ipv6, sizeof(addr_ipv6_t));
    }
    else if (NH_TYPE_BLACKHOLE == p_pb->type)
    {
    }
    else
    {
        return PM_E_FAIL;
    }
    
    return PM_E_NONE;
}

int32
pb_compose_nexthop_key_t_dump(Cdb__ComposeNexthopKeyT *p_pb, char *out)
{
    int32 len = 0;
    char str[CMD_BUF_32];
    
    if (NH_TYPE_IFINDEX == p_pb->type)
    {
        /* Ifindex */
        len = sal_sprintf(out, "/nexthop_key_t=(type=ifindex, ifname=%s)", 
            p_pb->ifname);
    }
    else if (NH_TYPE_IPV4 == p_pb->type)
    {
        /* IPv4 */
        len = sal_sprintf(out, "/nexthop_key_t=(type=ipv4, addr=%s)", 
            sal_inet_ntop(AF_INET, (void*)&(p_pb->ipv4_addr), str, CMD_BUF_32));
    }
    else if (NH_TYPE_IPV6 == p_pb->type)
    {
        /* IPv6 */
        len = sal_sprintf(out, "/nexthop_key_t=(type=ipv6, addr=%s)", 
            "ipv6_todo");
    }
    else if (NH_TYPE_BLACKHOLE == p_pb->type)
    {
        /* blackhole */
        len = sal_sprintf(out, "/nexthop_key_t=(type=blackhole)");
    }

    return len;
}

int32
pb_compose_nexthop_key_t_to_pb_free_packed(Cdb__ComposeNexthopKeyT *p_pb)
{
    if (p_pb)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->ifname);
        p_pb->ifname = NULL;

        if (NH_TYPE_IPV6 == p_pb->type)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->ipv6_addr);
            p_pb->ipv6_addr = NULL;
            p_pb->n_ipv6_addr = 0;
        }
    }
    
    return PM_E_NONE;
}

int32
pb_compose_nexthop_ecmp_t_to_pb(nexthop_ecmp_t *p_cdb, Cdb__ComposeNexthopEcmpT *p_pb)
{
    uint32 i = 0;
    
    p_pb->nh_num = p_cdb->nh_num;
    p_pb->n_nh = p_cdb->nh_num;

    p_pb->nh = XCALLOC(MEM_LIB_PROTOBUF, sizeof(void*)*p_cdb->nh_num);
        
    for (i = 0; i < p_cdb->nh_num; i++)
    {
        p_pb->nh[i] = XCALLOC(MEM_LIB_PROTOBUF, sizeof(Cdb__ComposeNexthopKeyT));
        pb_compose_nexthop_key_t_to_pb(&p_cdb->nh[i], p_pb->nh[i]);
    }
    
    return PM_E_NONE;
}

int32
pb_compose_nexthop_ecmp_t_dump(Cdb__ComposeNexthopEcmpT *p_pb, char *out)
{
    uint32 len = 0;
    uint32 i = 0;
    
    for (i = 0; i < p_pb->nh_num; i++)
    {
        len += pb_compose_nexthop_key_t_dump(p_pb->nh[i], (out + len));
    }

    return len;
}

int32
pb_compose_nexthop_ecmp_t_to_pb_free_packed(Cdb__ComposeNexthopEcmpT *p_pb)
{
    uint32 i = 0;
    
    if (p_pb)
    {
        if (p_pb->nh)
        {
            for (i = 0; i < p_pb->nh_num; i++)
            {
                pb_compose_nexthop_key_t_to_pb_free_packed(p_pb->nh[i]);
                XFREE(MEM_LIB_PROTOBUF, p_pb->nh[i]);
                p_pb->nh[i] = NULL;
            }

            XFREE(MEM_LIB_PROTOBUF, p_pb->nh);
            p_pb->nh = NULL;
        }
    }
    
    return PM_E_NONE;
}

int32
pb_compose_nexthop_ecmp_routed_t_to_pb(nexthop_ecmp_routed_t *p_cdb, Cdb__ComposeNexthopEcmpRoutedT *p_pb)
{
    uint32 i = 0;
    
    p_pb->nh_num = p_cdb->nh_num;
    p_pb->n_nh = p_cdb->nh_num;

    p_pb->nh = XCALLOC(MEM_LIB_PROTOBUF, sizeof(void*)*p_cdb->nh_num);
        
    for (i = 0; i < p_cdb->nh_num; i++)
    {
        p_pb->nh[i] = XCALLOC(MEM_LIB_PROTOBUF, sizeof(Cdb__ComposeNexthopKeyT));
        pb_compose_nexthop_key_t_to_pb(&p_cdb->nh[i], p_pb->nh[i]);
    }
    
    return PM_E_NONE;
}

int32
pb_compose_nexthop_ecmp_routed_t_dump(Cdb__ComposeNexthopEcmpRoutedT *p_pb, char *out)
{
    uint32 len = 0;
    uint32 i = 0;
    
    for (i = 0; i < p_pb->nh_num; i++)
    {
        len += pb_compose_nexthop_key_t_dump(p_pb->nh[i], (out + len));
    }

    return len;
}

int32
pb_compose_nexthop_ecmp_routed_t_to_pb_free_packed(Cdb__ComposeNexthopEcmpRoutedT *p_pb)
{
    uint32 i = 0;
    
    if (p_pb)
    {
        if (p_pb->nh)
        {
            for (i = 0; i < p_pb->nh_num; i++)
            {
                pb_compose_nexthop_key_t_to_pb_free_packed(p_pb->nh[i]);
                XFREE(MEM_LIB_PROTOBUF, p_pb->nh[i]);
                p_pb->nh[i] = NULL;
            }

            XFREE(MEM_LIB_PROTOBUF, p_pb->nh);
            p_pb->nh = NULL;
        }
    }
    
    return PM_E_NONE;
}


int32
pb_compose_ace_config_key_t_to_pb(ace_config_key_t *p_cdb, Cdb__ComposeAceConfigKeyT *p_pb)
{
    p_pb->seq_no = p_cdb->seq_no;
    p_pb->acl_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->acl_name);
    
    return PM_E_NONE;
}

int32
pb_compose_ace_config_key_t_dump(Cdb__ComposeAceConfigKeyT *p_pb, char *out)
{
    uint32 len = 0;
    len = sal_sprintf(out, "/nexthop_key_t=(acl_name=%s, seq_no=%u)", 
        p_pb->acl_name, p_pb->seq_no);

    return len;
}

int32
pb_compose_ace_config_key_t_to_pb_free_packed(Cdb__ComposeAceConfigKeyT *p_pb)
{
    if (p_pb && p_pb->acl_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->acl_name);
        p_pb->acl_name = NULL;
    }
    
    return PM_E_NONE;
}

int32
pb_compose_lldp_msap_id_t_to_pb(lldp_msap_id_t *p_cdb, Cdb__ComposeLldpMsapIdT *p_pb)
{
    p_pb->chassis_id_type = p_cdb->chassis_id.type;
    p_pb->port_id_type = p_cdb->port_id.type;

    p_pb->chassis_id.data = XCALLOC(MEM_LIB_PROTOBUF, p_cdb->chassis_id.len);
    sal_memcpy(p_pb->chassis_id.data, &p_cdb->chassis_id.id, p_cdb->chassis_id.len);

    p_pb->port_id.data = XCALLOC(MEM_LIB_PROTOBUF, p_cdb->port_id.len);
    sal_memcpy(p_pb->port_id.data, &p_cdb->port_id.id, p_cdb->port_id.len);

    return PM_E_NONE;
}

int32
pb_compose_lldp_msap_id_t_dump(Cdb__ComposeLldpMsapIdT *p_pb, char *out)
{
    int32 len = 0;
    
    if (LLDP_CHASSIS_ID_SUBTYPE_MAC_ADDRESS == p_pb->chassis_id_type)
    {
        /* Ifindex */
        len = sal_sprintf(out, "/chassis_id=(type=mac,)");
    }
    if (LLDP_PORT_ID_SUBTYPE_IF_NAME == p_pb->port_id_type)
    {
        /* blackhole */
        len = sal_sprintf(out, "/port_id=(type=ifname,)");
    }

    return len;
}

int32
pb_compose_lldp_msap_id_t_to_pb_free_packed(Cdb__ComposeLldpMsapIdT *p_pb)
{
    if (p_pb)
    {
        if (p_pb->chassis_id.data)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->chassis_id.data);
            p_pb->chassis_id.data = NULL;
        }
        if (p_pb->port_id.data)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->port_id.data);
            p_pb->port_id.data = NULL;
        }
    }
    
    return PM_E_NONE;
}


int32
pb_compose_ptp_foreign_master_t_to_pb(ptp_foreign_master_t *p_cdb, Cdb__ComposePtpForeignMasterT *p_pb)
{
/*refer pb_compose_lldp_msap_id_t_to_pb*/

    return PM_E_NONE;
}

int32
pb_compose_ptp_foreign_master_t_dump(Cdb__ComposePtpForeignMasterT *p_pb, char *out)
{
    //int32 len = 0;
    

    return 10;
   // return len;
}

int32
pb_compose_ptp_foreign_master_t_to_pb_free_packed(Cdb__ComposePtpForeignMasterT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_ptp_residence_time_cache_t_to_pb(ptp_residence_time_cache_t *p_cdb, Cdb__ComposePtpResidenceTimeCacheT *p_pb)
{
/*refer pb_compose_lldp_msap_id_t_to_pb*/

    return PM_E_NONE;
}

int32
pb_compose_ptp_residence_time_cache_t_dump(Cdb__ComposePtpResidenceTimeCacheT *p_pb, char *out)
{
    //int32 len = 0;
    

    return 10;
   // return len;
}

int32
pb_compose_ptp_residence_time_cache_t_to_pb_free_packed(Cdb__ComposePtpResidenceTimeCacheT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_class_in_policy_config_key_t_to_pb(class_in_policy_config_key_t *p_cdb, Cdb__ComposeClassInPolicyConfigKeyT *p_pb)
{
    p_pb->policy_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->policy_name);
    p_pb->class_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->class_name);
    
    return PM_E_NONE;
}

int32
pb_compose_class_in_policy_config_key_t_dump(Cdb__ComposeClassInPolicyConfigKeyT *p_pb, char *out)
{
    uint32 len = 0;
    len = sal_sprintf(out, "/class_in_policy_config_key_t=(policy_name=%s, class_name=%s)", 
        p_pb->policy_name, p_pb->class_name);

    return len;
}

int32
pb_compose_class_in_policy_config_key_t_to_pb_free_packed(Cdb__ComposeClassInPolicyConfigKeyT *p_pb)
{
    if (p_pb && p_pb->policy_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->policy_name);
        p_pb->policy_name = NULL;
    }

    if (p_pb && p_pb->class_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->class_name);
        p_pb->class_name = NULL;
    }
    
    return PM_E_NONE;
}

int32
pb_compose_acl_in_class_config_key_t_to_pb(acl_in_class_config_key_t *p_cdb, Cdb__ComposeAclInClassConfigKeyT *p_pb)
{
    p_pb->class_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->class_name);
    p_pb->acl_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->acl_name);
    
    return PM_E_NONE;
}

int32
pb_compose_acl_in_class_config_key_t_dump(Cdb__ComposeAclInClassConfigKeyT *p_pb, char *out)
{
    uint32 len = 0;
    len = sal_sprintf(out, "/acl_in_class_config_key_t=(class_name=%s, acl_name=%s)", 
        p_pb->class_name, p_pb->acl_name);

    return len;
}

int32
pb_compose_acl_in_class_config_key_t_to_pb_free_packed(Cdb__ComposeAclInClassConfigKeyT *p_pb)
{
    if (p_pb && p_pb->class_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->class_name);
        p_pb->class_name = NULL;
    }

    if (p_pb && p_pb->acl_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->acl_name);
        p_pb->acl_name = NULL;
    }
    
    return PM_E_NONE;
}

int32
pb_compose_class_map_action_config_key_t_to_pb(class_map_action_config_key_t *p_cdb, Cdb__ComposeClassMapActionConfigKeyT *p_pb)
{
    p_pb->policy_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->policy_name);
    p_pb->class_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->class_name);
    
    return PM_E_NONE;
}

int32
pb_compose_class_map_action_config_key_t_dump(Cdb__ComposeClassMapActionConfigKeyT *p_pb, char *out)
{
    uint32 len = 0;
    len = sal_sprintf(out, "/class_map_action_config_key_t=(policy_name=%s, class_name=%s)", 
        p_pb->policy_name, p_pb->class_name);

    return len;
}

int32
pb_compose_class_map_action_config_key_t_to_pb_free_packed(Cdb__ComposeClassMapActionConfigKeyT *p_pb)
{
    if (p_pb && p_pb->policy_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->policy_name);
        p_pb->policy_name = NULL;
    }

    if (p_pb && p_pb->class_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->class_name);
        p_pb->class_name = NULL;
    }
    
    return PM_E_NONE;
}

int32
pb_compose_auth_server_key_t_to_pb(auth_server_key_t *p_cdb, Cdb__ComposeAuthServerKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_auth_server_key_t_dump(Cdb__ComposeAuthServerKeyT *p_pb, char *out)
{
    return 0;
}

int32
pb_compose_auth_server_key_t_to_pb_free_packed(Cdb__ComposeAuthServerKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_auth_session_key_t_to_pb(auth_session_key_t *p_cdb, Cdb__ComposeAuthSessionKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_auth_session_key_t_dump(Cdb__ComposeAuthSessionKeyT *p_pb, char *out)
{
    return 0;
}

int32
pb_compose_auth_session_key_t_to_pb_free_packed(Cdb__ComposeAuthSessionKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_lldp_civic_address_t_to_pb(lldp_civic_address_t *p_cdb, Cdb__ComposeLldpCivicAddressT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_lldp_civic_address_t_dump(Cdb__ComposeLldpCivicAddressT *p_pb, char *out)
{
    return 0;
}

int32
pb_compose_lldp_civic_address_t_to_pb_free_packed(Cdb__ComposeLldpCivicAddressT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_igsp_group_key_t_to_pb(igsp_group_key_t *p_cdb, Cdb__ComposeIgspGroupKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_igsp_group_key_t_dump(Cdb__ComposeIgspGroupKeyT *p_pb, char *out)
{
    return 0;
}

int32
pb_compose_igsp_group_key_t_to_pb_free_packed(Cdb__ComposeIgspGroupKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_fea_g8032_ring_key_t_to_pb(fea_g8032_ring_key_t *p_cdb, Cdb__ComposeFeaG8032RingKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_fea_g8032_ring_key_t_dump(Cdb__ComposeFeaG8032RingKeyT *p_pb, char *out)
{
    return 0;
}

int32
pb_compose_fea_g8032_ring_key_t_to_pb_free_packed(Cdb__ComposeFeaG8032RingKeyT *p_pb)
{
    return PM_E_NONE;
}
int32
pb_compose_mirror_mac_escape_key_t_to_pb(mirror_mac_escape_key_t *p_cdb, Cdb__ComposeMirrorMacEscapeKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_mirror_mac_escape_key_t_dump(Cdb__ComposeMirrorMacEscapeKeyT *p_pb, char *out)
{
    return PM_E_NONE;
}

int32
pb_compose_mirror_mac_escape_key_t_to_pb_free_packed(Cdb__ComposeMirrorMacEscapeKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_glb_ptp_port_id_t_to_pb(tbl_ptp_foreign_key_t *p_cdb, Cdb__ComposeGlbPtpPortIdT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_glb_ptp_port_id_t_dump(Cdb__ComposeGlbPtpPortIdT *p_pb, char *out)
{
    return PM_E_NONE;
}

int32
pb_compose_glb_ptp_port_id_t_to_pb_free_packed(Cdb__ComposeGlbPtpPortIdT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_tap_five_tuple_key_t_to_pb(tbl_tap_five_tuple_key_t *p_cdb, Cdb__ComposeTapFiveTupleKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->ip_sa = p_cdb->ip_sa.s_addr;
        p_pb->ip_da = p_cdb->ip_da.s_addr;
        p_pb->l4_src_port = p_cdb->l4_src_port;
        p_pb->l4_dst_port = p_cdb->l4_dst_port;
        p_pb->l4_protocol = p_cdb->l4_protocol;
    }

    return PM_E_NONE;
}

int32
pb_compose_tap_five_tuple_key_t_dump(Cdb__ComposeTapFiveTupleKeyT *p_pb, char *out)
{
    char str_ipsa[CMD_BUF_64];
    char str_ipda[CMD_BUF_64];
    addr_ipv4_t addr_ipsa;
    addr_ipv4_t addr_ipda;

    int32 len = 0;
    
    if (p_pb)
    {
        addr_ipsa.s_addr = p_pb->ip_sa;
        addr_ipda.s_addr = p_pb->ip_da;
        len = sal_sprintf(out, "/ns_port_forwarding_key=(ip_sa=%s, ip_da=%s, l4_src_port=%u, l4_dst_port=%u, l4_protocol=%u)", 
            cdb_addr_ipv4_val2str(str_ipsa, CMD_BUF_64, &addr_ipsa),
            cdb_addr_ipv4_val2str(str_ipda, CMD_BUF_64, &addr_ipda), p_pb->l4_src_port, p_pb->l4_dst_port, p_pb->l4_protocol);
    }

    return len;
}

int32
pb_compose_tap_five_tuple_key_t_to_pb_free_packed(Cdb__ComposeTapFiveTupleKeyT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_tap_tuple_acl_entry_id_t_to_pb(tap_tuple_acl_entry_id_t *p_cdb, Cdb__ComposeTapTupleAclEntryIdT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_tap_tuple_acl_entry_id_t_dump(Cdb__ComposeTapTupleAclEntryIdT *p_pb, char *out)
{
    //uint32 i = 0;
    int32 len = 0;
    
    return len;
}

int32
pb_compose_tap_tuple_acl_entry_id_t_to_pb_free_packed(Cdb__ComposeTapTupleAclEntryIdT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_tap_group_ingress_key_t_to_pb(tbl_tap_group_ingress_key_t *p_cdb, Cdb__ComposeTapGroupIngressKeyT *p_pb)
{
    p_pb->tap_group = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->tap_group);
    p_pb->if_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->if_name);
    
    return PM_E_NONE;
}

int32
pb_compose_tap_group_ingress_key_t_dump(Cdb__ComposeTapGroupIngressKeyT *p_pb, char *out)
{
    uint32 len = 0;
    len = sal_sprintf(out, "/tap_group_ingress_key_t=(tap_group=%s, if_name=%s)", 
        p_pb->tap_group, p_pb->if_name);

    return len;
}

int32
pb_compose_tap_group_ingress_key_t_to_pb_free_packed(Cdb__ComposeTapGroupIngressKeyT *p_pb)
{
    if (p_pb && p_pb->tap_group)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->tap_group);
        p_pb->tap_group = NULL;
    }

    if (p_pb && p_pb->if_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->if_name);
        p_pb->if_name = NULL;
    }
    
    return PM_E_NONE;
}

int32
pb_compose_tap_group_ingress_flow_key_t_to_pb(tbl_tap_group_ingress_flow_key_t *p_cdb, Cdb__ComposeTapGroupIngressFlowKeyT *p_pb)
{
    p_pb->tap_group = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->tap_group);
    p_pb->if_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->if_name);
    p_pb->flow_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->flow_name);
    
    return PM_E_NONE;
}

int32
pb_compose_tap_group_ingress_flow_key_t_dump(Cdb__ComposeTapGroupIngressFlowKeyT *p_pb, char *out)
{
    uint32 len = 0;
    len = sal_sprintf(out, "/tap_group_ingress_flow_key_t=(tap_group=%s, if_name=%s, flow_name=%s)", 
        p_pb->tap_group, p_pb->if_name, p_pb->flow_name);

    return len;
}

int32
pb_compose_tap_group_ingress_flow_key_t_to_pb_free_packed(Cdb__ComposeTapGroupIngressFlowKeyT *p_pb)
{
    if (p_pb && p_pb->tap_group)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->tap_group);
        p_pb->tap_group = NULL;
    }

    if (p_pb && p_pb->if_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->if_name);
        p_pb->if_name = NULL;
    }

    if (p_pb && p_pb->flow_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->flow_name);
        p_pb->flow_name = NULL;
    }
    return PM_E_NONE;
}


int32
pb_compose_tap_group_egress_key_t_to_pb(tbl_tap_group_egress_key_t *p_cdb, Cdb__ComposeTapGroupEgressKeyT *p_pb)
{
    p_pb->tap_group = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->tap_group);
    p_pb->if_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->if_name);
    
    return PM_E_NONE;
}

int32
pb_compose_tap_group_egress_key_t_dump(Cdb__ComposeTapGroupEgressKeyT *p_pb, char *out)
{
    uint32 len = 0;
    len = sal_sprintf(out, "/tap_group_egress_key_t=(tap_group=%s, if_name=%s)", 
        p_pb->tap_group, p_pb->if_name);

    return len;
}

int32
pb_compose_tap_group_egress_key_t_to_pb_free_packed(Cdb__ComposeTapGroupEgressKeyT *p_pb)
{
    if (p_pb && p_pb->tap_group)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->tap_group);
        p_pb->tap_group = NULL;
    }

    if (p_pb && p_pb->if_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->if_name);
        p_pb->if_name = NULL;
    }
    
    return PM_E_NONE;
}

int32
pb_compose_inband_snat_key_t_to_pb(inband_snat_key_t *p_cdb, Cdb__ComposeInbandSnatKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->ip = p_cdb->ip.s_addr;
        p_pb->port = p_cdb->port;
        p_pb->proto = p_cdb->proto;
    }

    return PM_E_NONE;
}

int32
pb_compose_inband_snat_key_t_dump(Cdb__ComposeInbandSnatKeyT *p_pb, char *out)
{
    char str[CMD_BUF_64];
    addr_ipv4_t addr;

    int32 len = 0;
    
    if (p_pb)
    {
        addr.s_addr = p_pb->ip;
        len = sal_sprintf(out, "/inband_snat_key=(proto=%u, port=%u, addr=%s)", 
            p_pb->proto, 
            p_pb->port, 
            cdb_addr_ipv4_val2str(str, CMD_BUF_64, &addr));
    }

    return len;
}

int32
pb_compose_inband_snat_key_t_to_pb_free_packed(Cdb__ComposeInbandSnatKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_arpace_config_key_t_to_pb(arpace_config_key_t *p_cdb, Cdb__ComposeArpaceConfigKeyT *p_pb)
{
    p_pb->seq_no = p_cdb->seq_no;
    p_pb->arpacl_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->arpacl_name);
    
    return PM_E_NONE;
}

int32
pb_compose_arpace_config_key_t_dump(Cdb__ComposeArpaceConfigKeyT *p_pb, char *out)
{
    uint32 len = 0;
    len = sal_sprintf(out, "/nexthop_key_t=(arpacl_name=%s, seq_no=%u)", 
        p_pb->arpacl_name, p_pb->seq_no);

    return len;
}

int32
pb_compose_arpace_config_key_t_to_pb_free_packed(Cdb__ComposeArpaceConfigKeyT *p_pb)
{
    if (p_pb && p_pb->arpacl_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->arpacl_name);
        p_pb->arpacl_name = NULL;
    }
    
    return PM_E_NONE;
}

int32
pb_compose_ssm_clock_id_t_to_pb(ssm_clock_id_t *p_cdb, Cdb__ComposeSsmClockIdT *p_pb)
{
    if (p_pb)
    {
        p_pb->mac.len = SSM_CLOCK_ID_LEN;
        p_pb->mac.data = XCALLOC(MEM_LIB_PROTOBUF, SSM_CLOCK_ID_LEN);
        sal_memcpy(p_pb->mac.data, p_cdb, SSM_CLOCK_ID_LEN);
    }
    
    return PM_E_NONE;
}

int32
pb_compose_ssm_clock_id_t_dump(Cdb__ComposeSsmClockIdT *p_pb, char *out)
{
    int32 len = 0;
    
    if (p_pb->mac.data)
    {
        len = sal_sprintf(out, "/clock_id=%.04hX.%.04hX.%.04hX.%04hX", 
            sal_ntoh16(((uint16*)p_pb->mac.data)[0]),
            sal_ntoh16(((uint16*)p_pb->mac.data)[1]),
            sal_ntoh16(((uint16*)p_pb->mac.data)[2]),
            sal_ntoh16(((uint16*)p_pb->mac.data)[3]));
    }

    return len;
}

int32
pb_compose_ssm_clock_id_t_to_pb_free_packed(Cdb__ComposeSsmClockIdT *p_pb)
{
    if (p_pb)
    {
        if (p_pb->mac.data)
        {
            XFREE(MEM_LIB_PROTOBUF, p_pb->mac.data);
            p_pb->mac.data = NULL;
        }
        p_pb->mac.len = 0;
    }
    return PM_E_NONE;
}

int32
pb_compose_rmon_stats_history_key_t_to_pb(rmon_stats_history_key_t *p_cdb, Cdb__ComposeRmonStatsHistoryKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->history_index = p_cdb->history_index;
        p_pb->stats_index = p_cdb->stats_index;
    }

    return PM_E_NONE;
}

int32
pb_compose_rmon_stats_history_key_t_dump(Cdb__ComposeRmonStatsHistoryKeyT *p_pb, char *out)
{
    int32 len = 0;

    len = sal_sprintf(out, "/rmon_stats_history_key:history_index=%u, stats_index=%u", p_pb->history_index, p_pb->stats_index);

    return len;
}

int32
pb_compose_rmon_stats_history_key_t_to_pb_free_packed(Cdb__ComposeRmonStatsHistoryKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_ipsla_packet_stats_key_t_to_pb(ipsla_packet_stats_key_t *p_cdb, Cdb__ComposeIpslaPacketStatsKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->entry_id = p_cdb->entry_id;
        p_pb->packet_index = p_cdb->packet_index;
    }

    return PM_E_NONE;
}

int32
pb_compose_ipsla_packet_stats_key_t_dump(Cdb__ComposeIpslaPacketStatsKeyT *p_pb, char *out)
{
    int32 len = 0;

    len = sal_sprintf(out, "/ipsla_packet_stats_key:entry_id=%u, packet_index=%u", p_pb->entry_id, p_pb->packet_index);

    return len;
}

int32
pb_compose_ipsla_packet_stats_key_t_to_pb_free_packed(Cdb__ComposeIpslaPacketStatsKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_ipsla_test_stats_key_t_to_pb(ipsla_test_stats_key_t *p_cdb, Cdb__ComposeIpslaTestStatsKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->entry_id = p_cdb->entry_id;
        p_pb->test_index = p_cdb->test_index;
    }

    return PM_E_NONE;
}

int32
pb_compose_ipsla_test_stats_key_t_dump(Cdb__ComposeIpslaTestStatsKeyT *p_pb, char *out)
{
    int32 len = 0;

    len = sal_sprintf(out, "/ipsla_test_stats_key:entry_id=%u, test_index=%u", p_pb->entry_id, p_pb->test_index);

    return len;
}

int32
pb_compose_ipsla_test_stats_key_t_to_pb_free_packed(Cdb__ComposeIpslaTestStatsKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_mrt_key_t_to_pb(mrt_key_t *p_cdb, Cdb__ComposeMrtKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->vrf_id = p_cdb->vrf_id;
        p_pb->source = p_cdb->source.s_addr;
        p_pb->group = p_cdb->group.s_addr;
    }

    return PM_E_NONE;
}

int32
pb_compose_mrt_key_t_dump(Cdb__ComposeMrtKeyT *p_pb, char *out)
{
    int32 len = 0;
    addr_ipv4_t source;
    addr_ipv4_t group;
    char sourcestr[CMD_BUF_64];
    char groupstr[CMD_BUF_64];
    
    if (p_pb)
    {
        source.s_addr = p_pb->source;
        group.s_addr = p_pb->group;
        len = sal_sprintf(out, "/mrt_key=(vrf_id=%u, source=%s, group=%s)", 
            p_pb->vrf_id, cdb_addr_ipv4_val2str(sourcestr, CMD_BUF_64, &source), 
            cdb_addr_ipv4_val2str(groupstr, CMD_BUF_64, &group));
    }

    return len;
}

int32
pb_compose_mrt_key_t_to_pb_free_packed(Cdb__ComposeMrtKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_vlan_mapping_entry_key_t_to_pb(vlan_mapping_entry_key_t *p_cdb, Cdb__ComposeVlanMappingEntryKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->raw_vid = p_cdb->raw_vid;
        p_pb->table_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->table_name);
    }

    return PM_E_NONE;
}

int32
pb_compose_vlan_mapping_entry_key_t_dump(Cdb__ComposeVlanMappingEntryKeyT *p_pb, char *out)
{
    int32 len = 0;

    len = sal_sprintf(out, "/vlan_mapping_entry_key:raw_vid=%u, table_name=%s", p_pb->raw_vid, p_pb->table_name);

    return len;
}

int32
pb_compose_vlan_mapping_entry_key_t_to_pb_free_packed(Cdb__ComposeVlanMappingEntryKeyT *p_pb)
{
    if (p_pb && p_pb->table_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->table_name);
        p_pb->table_name = NULL;
    }
    return PM_E_NONE;
}

int32
pb_compose_egress_mapping_entry_key_t_to_pb(egress_mapping_entry_key_t *p_cdb, Cdb__ComposeEgressMappingEntryKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->raw_cvid = p_cdb->raw_cvid;
        p_pb->raw_svid = p_cdb->raw_svid;
        p_pb->table_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->table_name);
    }

    return PM_E_NONE;
}

int32
pb_compose_egress_mapping_entry_key_t_dump(Cdb__ComposeEgressMappingEntryKeyT *p_pb, char *out)
{
    int32 len = 0;

    len = sal_sprintf(out, "/vlan_mapping_entry_key:raw_cvid=%u, raw_svid=%u, table_name=%s", p_pb->raw_cvid, p_pb->raw_svid, p_pb->table_name);

    return len;
}

int32
pb_compose_egress_mapping_entry_key_t_to_pb_free_packed(Cdb__ComposeEgressMappingEntryKeyT *p_pb)
{
    if (p_pb && p_pb->table_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->table_name);
        p_pb->table_name = NULL;
    }
    return PM_E_NONE;
}


int32
pb_compose_mapped_vlan_info_key_t_to_pb(mapped_vlan_info_key_t *p_cdb, Cdb__ComposeMappedVlanInfoKeyT *p_pb)
{
    if (p_pb)
    {
        p_pb->evc_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->evc_name);
        p_pb->table_name = XSTRDUP(MEM_LIB_PROTOBUF, p_cdb->table_name);
    }

    return PM_E_NONE;
}

int32
pb_compose_mapped_vlan_info_key_t_dump(Cdb__ComposeMappedVlanInfoKeyT *p_pb, char *out)
{
    int32 len = 0;

    len = sal_sprintf(out, "/mapped_vlan_info_key:evc_name=%s, table_name=%s", p_pb->evc_name, p_pb->table_name);

    return len;
}

int32
pb_compose_mapped_vlan_info_key_t_to_pb_free_packed(Cdb__ComposeMappedVlanInfoKeyT *p_pb)
{
    if (p_pb && p_pb->evc_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->evc_name);
        p_pb->evc_name = NULL;
    }

    if (p_pb && p_pb->table_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb->table_name);
        p_pb->table_name = NULL;
    }
    return PM_E_NONE;
}

int32
pb_compose_nd_prefix_key_t_to_pb(nd_prefix_key_t *p_cdb, Cdb__ComposeNdPrefixKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_nd_prefix_key_t_dump(Cdb__ComposeNdPrefixKeyT *p_pb, char *out)
{
    return 0;
}

int32
pb_compose_nd_prefix_key_t_to_pb_free_packed(Cdb__ComposeNdPrefixKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_l2pro_if_action_vlan_t_to_pb(l2pro_if_action_vlan_t *p_cdb, Cdb__ComposeL2proIfActionVlanT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_l2pro_if_action_vlan_t_dump(Cdb__ComposeL2proIfActionVlanT *p_pb, char *out)
{
    return PM_E_NONE;
}

int32
pb_compose_l2pro_if_action_vlan_t_to_pb_free_packed(Cdb__ComposeL2proIfActionVlanT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_glb_qos_color_action_t_to_pb(glb_qos_color_action_t *p_cdb, Cdb__ComposeGlbQosColorActionT *p_pb)
{
    if (p_pb)
    {
        p_pb->drop_packet = p_cdb->drop_packet;
        p_pb->tc_valid = p_cdb->tc_valid;
        p_pb->tc = p_cdb->tc;
    }
    return PM_E_NONE;
}

int32
pb_compose_glb_qos_color_action_t_dump(Cdb__ComposeGlbQosColorActionT *p_pb, char *out)
{
    int32 len = 0;

    len = sal_sprintf(out, "/color_action:drop=%u, tc_valid=%u, tc=%u", p_pb->drop_packet, p_pb->tc_valid, p_pb->tc);
    return len;
}

int32
pb_compose_glb_qos_color_action_t_to_pb_free_packed(Cdb__ComposeGlbQosColorActionT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_qos_policer_res_key_t_to_pb(qos_policer_res_key_t *p_cdb, Cdb__ComposeQosPolicerResKeyT *p_pb)
{

    return PM_E_NONE;
}

int32
pb_compose_qos_policer_res_key_t_dump(Cdb__ComposeQosPolicerResKeyT *p_pb, char *out)
{
    return PM_E_NONE;
}

int32
pb_compose_qos_policer_res_key_t_to_pb_free_packed(Cdb__ComposeQosPolicerResKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_l2edit_key_t_to_pb(l2edit_key_t *p_cdb, Cdb__ComposeL2editKeyT *p_pb)
{
      return PM_E_NONE;
}

int32
pb_compose_l2edit_key_t_dump(Cdb__ComposeL2editKeyT *p_pb, char *out)
{
    return PM_E_NONE;
}

int32
pb_compose_l2edit_key_t_to_pb_free_packed(Cdb__ComposeL2editKeyT *p_pb)
{
    return PM_E_NONE;
}


int32
pb_compose_nat_session_key_t_to_pb(nat_session_key_t *p_cdb, Cdb__ComposeNatSessionKeyT *p_pb)
{
      return PM_E_NONE;
}

int32
pb_compose_nat_session_key_t_dump(Cdb__ComposeNatSessionKeyT *p_pb, char *out)
{
    return PM_E_NONE;
}

int32
pb_compose_nat_session_key_t_to_pb_free_packed(Cdb__ComposeNatSessionKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_glb_poe_cfg_t_to_pb(glb_poe_cfg_t *p_cdb, Cdb__ComposeGlbPoeCfgT *p_pb)
{
    
    return PM_E_NONE;
}

int32
pb_compose_glb_poe_cfg_t_dump(Cdb__ComposeGlbPoeCfgT *p_pb, char *out)
{
    return PM_E_NONE;
}

int32
pb_compose_glb_poe_cfg_t_to_pb_free_packed(Cdb__ComposeGlbPoeCfgT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_nat_rule_key_t_to_pb(nat_rule_key_t *p_cdb, Cdb__ComposeNatRuleKeyT *p_pb)
{
      return PM_E_NONE;
}

int32
pb_compose_nat_rule_key_t_dump(Cdb__ComposeNatRuleKeyT *p_pb, char *out)
{
    return PM_E_NONE;
}

int32
pb_compose_nat_rule_key_t_to_pb_free_packed(Cdb__ComposeNatRuleKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_ctclib_opb_t_to_pb(ctclib_opb_t *p_cdb, Cdb__ComposeCtclibOpbT *p_pb)
{
      return PM_E_NONE;
}

int32
pb_compose_ctclib_opb_t_dump(Cdb__ComposeCtclibOpbT *p_pb, char *out)
{
    return PM_E_NONE;
}

int32
pb_compose_ctclib_opb_t_to_pb_free_packed(Cdb__ComposeCtclibOpbT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_nat_proxy_arp_key_t_to_pb(nat_proxy_arp_key_t *p_cdb, Cdb__ComposeNatProxyArpKeyT *p_pb)
{
      return PM_E_NONE;
}

int32
pb_compose_nat_proxy_arp_key_t_dump(Cdb__ComposeNatProxyArpKeyT *p_pb, char *out)
{
    return PM_E_NONE;
}

int32
pb_compose_nat_proxy_arp_key_t_to_pb_free_packed(Cdb__ComposeNatProxyArpKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_cfm_ma_key_t_to_pb(cfm_ma_key_t *p_cdb, Cdb__ComposeCfmMaKeyT *p_pb)
{
      return PM_E_NONE;
}

int32
pb_compose_cfm_ma_key_t_dump(Cdb__ComposeCfmMaKeyT *p_pb, char *out)
{
    return PM_E_NONE;
}

int32
pb_compose_cfm_ma_key_t_to_pb_free_packed(Cdb__ComposeCfmMaKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_cfm_lmep_key_t_to_pb(cfm_lmep_key_t *p_cdb, Cdb__ComposeCfmLmepKeyT *p_pb)
{
      return PM_E_NONE;
}

int32
pb_compose_cfm_lmep_key_t_dump(Cdb__ComposeCfmLmepKeyT *p_pb, char *out)
{
    return PM_E_NONE;
}

int32
pb_compose_cfm_lmep_key_t_to_pb_free_packed(Cdb__ComposeCfmLmepKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_cfm_rmep_key_t_to_pb(cfm_rmep_key_t *p_cdb, Cdb__ComposeCfmRmepKeyT *p_pb)
{
      return PM_E_NONE;
}

int32
pb_compose_cfm_rmep_key_t_dump(Cdb__ComposeCfmRmepKeyT *p_pb, char *out)
{
    return PM_E_NONE;
}

int32
pb_compose_cfm_rmep_key_t_to_pb_free_packed(Cdb__ComposeCfmRmepKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_cfm_time_key_t_to_pb(cfm_time_key_t *p_cdb, Cdb__ComposeCfmTimeKeyT *p_pb)
{
      return PM_E_NONE;
}

int32
pb_compose_cfm_time_key_t_dump(Cdb__ComposeCfmTimeKeyT *p_pb, char *out)
{
    return PM_E_NONE;
}

int32
pb_compose_cfm_time_key_t_to_pb_free_packed(Cdb__ComposeCfmTimeKeyT *p_pb)
{
    return PM_E_NONE;
}

int32
pb_compose_pim_sm_gprefix_t_to_pb(pim_sm_gprefix_t *p_cdb, Cdb__ComposePimSmGprefixT *p_pb)
{
	return PM_E_NONE;
}

int32
pb_compose_pim_sm_gprefix_t_dump(Cdb__ComposePimSmGprefixT *p_pb, char *out)
{
	return PM_E_NONE;
}

int32
pb_compose_pim_sm_gprefix_t_to_pb_free_packed(Cdb__ComposePimSmGprefixT *p_pb)
{
	return PM_E_NONE;
}
int32
pb_compose_pim_static_rp_addr_t_to_pb(pim_static_rp_addr_t *p_cdb, Cdb__ComposePimStaticRpAddrT *p_pb)
{
	return PM_E_NONE;	
}

int32
pb_compose_pim_static_rp_addr_t_dump(Cdb__ComposePimStaticRpAddrT *p_pb, char *out)
{
		return PM_E_NONE;
}

int32
pb_compose_pim_static_rp_addr_t_to_pb_free_packed(Cdb__ComposePimStaticRpAddrT *p_pb)
{
	return PM_E_NONE;
}
