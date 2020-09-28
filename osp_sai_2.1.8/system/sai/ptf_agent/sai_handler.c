/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "sal.h"
#include "sai.h"
#include "genlog.h"

#include <thrift/c_glib/thrift.h>
#include <thrift/c_glib/thrift_application_exception.h>
#include "ptf_agent.h"
#include "sai_handler.h"

#define u64_fmt "0x%llx"

extern PTF_HDLR_LOG_OUT_FUNC PTF_HDLR_LOG;

G_DEFINE_TYPE (ctc_sai_rpcHandler,
               ctc_sai_rpc_handler,
               TYPE_SWITCH_SAI_RPC_HANDLER);
char*
ptf_hdlr_neighbor_val2str(sai_neighbor_entry_t *neighbor, char *str)
{
    char addr_str[64];
    
    addr_str[0] = 0;
    str[0] = 0;

    if (SAI_IP_ADDR_FAMILY_IPV4 == neighbor->ip_address.addr_family)
    {
        sal_inet_ntop(AF_INET, (void*)&neighbor->ip_address.addr.ip4, addr_str, 64);
    }
    else if (SAI_IP_ADDR_FAMILY_IPV6 == neighbor->ip_address.addr_family)
    {
        sal_inet_ntop(AF_INET6, (void*)neighbor->ip_address.addr.ip6, addr_str, 64);
    }
    else
    {
        sal_sprintf(str, "invalid addr family %u", neighbor->ip_address.addr_family);
    }

    sal_sprintf(str, "rif_id = 0x%llx, addr = %s", (unsigned long long)neighbor->rif_id, addr_str);

    return str;
}

char*
ptf_hdlr_route_entry_val2str(sai_unicast_route_entry_t *route, char *str)
{
    char addr_str[64];
    char mask_str[64];
    
    addr_str[0] = 0;
    mask_str[0] = 0;
    str[0] = 0;

    if (SAI_IP_ADDR_FAMILY_IPV4 == route->destination.addr_family)
    {
        sal_inet_ntop(AF_INET, (void*)&route->destination.addr.ip4, addr_str, 64);
        sal_inet_ntop(AF_INET, (void*)&route->destination.mask.ip4, mask_str, 64);
    }
    else if (SAI_IP_ADDR_FAMILY_IPV6 == route->destination.addr_family)
    {
        sal_inet_ntop(AF_INET6, (void*)route->destination.addr.ip6, addr_str, 64);
        sal_inet_ntop(AF_INET6, (void*)route->destination.mask.ip6, mask_str, 64);
    }
    else
    {
        sal_sprintf(str, "invalid addr family %u", route->destination.addr_family);
    }

    sal_sprintf(str, "vr_id = 0x%lx, addr = %s, mask = %s", (unsigned long)route->vr_id, addr_str, mask_str);

    return str;
}

char*
ptf_hdlr_ip_address_val2str(sai_ip_address_t *ip_address, char *str)
{
    char addr_str[64];
    
    addr_str[0] = 0;
    str[0] = 0;

    if (SAI_IP_ADDR_FAMILY_IPV4 == ip_address->addr_family)
    {
        sal_inet_ntop(AF_INET, (void*)&ip_address->addr.ip4, addr_str, 64);
    }
    else if (SAI_IP_ADDR_FAMILY_IPV6 == ip_address->addr_family)
    {
        sal_inet_ntop(AF_INET6, (void*)ip_address->addr.ip6, addr_str, 64);
    }
    else
    {
        sal_sprintf(str, "invalid addr family %u", ip_address->addr_family);
    }

    sal_sprintf(str, "addr = %s", addr_str);

    return str;
}

char*
ptf_hdlr_ip_address_val2str_by_acldata(sai_ip4_t *ip4, char *str)
{
    char addr_str[64];
    
    addr_str[0] = 0;
    str[0] = 0;
    sal_inet_ntop(AF_INET, (void*)ip4, addr_str, 64);

    sal_sprintf(str, "addr = %s", addr_str);

    return str;
}

char*
ptf_hdlr_mac_val2str(uint8 *mac, char *str)
{
    str[0] = 0;
    sal_sprintf(str, "%02x%02x.%02x%02x.%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return str;
}

#define __PTF_HANDLER_FORMAT_FUNCTION__

int32
ptf_hdlr_format_mac_thrift_string_to_sai(const gchar *thrift, uint8 *sai)
{
    uint32 i = 0;
    uint32 j = 0;
    char let = 0;
    
    sal_memset(sai, 0, PTF_ADDR_LEN);

    for (i = 0; i < sal_strlen(thrift); i++)
    {
        let = thrift[i];
        if (let >= '0' && let <= '9')
        {
            sai[j/2] = (sai[j/2] << 4) + (let - '0');
            j++;
        }
        else if (let >= 'a' && let <= 'f') 
        {
            sai[j/2] = (sai[j/2] << 4) + (let - 'a' + 10);
            j++;
        } 
        else if (let >= 'A' && let <= 'F')
        {
            sai[j/2] = (sai[j/2] << 4) + (let - 'A' + 10);
            j++;
        }
    }
    return (j != 12);
}

int32
ptf_hdlr_format_switch_attribute_thrift_to_sai(const sai_thrift_attribute_t *thrift, sai_attribute_t *sai)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    sai->id = thrift->id;
    
    switch (sai->id)
    {
    case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
        rc = ptf_hdlr_format_mac_thrift_string_to_sai(thrift->value->mac, sai->value.mac);
        break;

    default:
        return SAI_STATUS_NOT_SUPPORTED;
    }
    
    return rc;
}

int32
ptf_hdlr_format_fdb_entry_thrift_to_sai(const sai_thrift_fdb_entry_t *thrift, sai_fdb_entry_t *sai)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    sai->vlan_id = thrift->vlan_id;
    rc = ptf_hdlr_format_mac_thrift_string_to_sai(thrift->mac_address, sai->mac_address);
    
    return rc;
}

int32
ptf_hdlr_format_fdb_entry_attr_list_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai, uint32 *attr_count)
{
    sai_thrift_attribute_t *thrift_attr[3];
    int32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    *attr_count = 0;

    if (thrift->len > 3)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        thrift_attr[i] = g_ptr_array_index(thrift, i);
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        switch (thrift_attr[i]->id)
        {
        case SAI_FDB_ENTRY_ATTR_TYPE:
            sai[i].id = SAI_FDB_ENTRY_ATTR_TYPE;
            sai[i].value.s32 = thrift_attr[i]->value->s32;
            break;
        case SAI_FDB_ENTRY_ATTR_PORT_ID:
            sai[i].id = SAI_FDB_ENTRY_ATTR_PORT_ID;
            sai[i].value.oid = thrift_attr[i]->value->oid;
            break;
        case SAI_FDB_ENTRY_ATTR_PACKET_ACTION:
            sai[i].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION;
            sai[i].value.s32 = thrift_attr[i]->value->s32;
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    *attr_count = thrift->len;
    return rc;
}

int32
ptf_hdlr_format_port_attr_thrift_to_sai(const sai_thrift_attribute_t * thrift_attr, sai_attribute_t *sai)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    sai->id = thrift_attr->id;
    switch (thrift_attr->id)
    {
    case SAI_PORT_ATTR_ADMIN_STATE:
    case SAI_PORT_ATTR_DROP_UNTAGGED:
    case SAI_PORT_ATTR_DROP_TAGGED:
        sai->value.booldata = thrift_attr->value->booldata;
        break;

    case SAI_PORT_ATTR_PORT_VLAN_ID:
        sai->value.u16 = thrift_attr->value->u16;
        break;
    
    default:
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    return rc;
}

int32
ptf_hdlr_format_fdb_flush_attr_list_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai, uint32 *attr_count)
{
    sai_thrift_attribute_t *thrift_attr[3];
    int32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    *attr_count = 0;

    if (thrift->len > 3)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        thrift_attr[i] = g_ptr_array_index(thrift, i);
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        switch (thrift_attr[i]->id)
        {
        case SAI_FDB_FLUSH_ATTR_PORT_ID:
            sai[i].id = SAI_FDB_FLUSH_ATTR_PORT_ID;
            sai[i].value.oid = thrift_attr[i]->value->oid;
            break;
        case SAI_FDB_FLUSH_ATTR_VLAN_ID:
            sai[i].id = SAI_FDB_FLUSH_ATTR_VLAN_ID;
            sai[i].value.u16 = thrift_attr[i]->value->u16;
            break;
        case SAI_FDB_FLUSH_ATTR_ENTRY_TYPE:
            sai[i].id = SAI_FDB_FLUSH_ATTR_ENTRY_TYPE;
            sai[i].value.s32 = thrift_attr[i]->value->s32;
            break;
        case SAI_FDB_FLUSH_ATTR_MAC:
            sai[i].id = SAI_FDB_FLUSH_ATTR_MAC;
            sal_memcpy(sai[i].value.mac, thrift_attr[i]->value->mac, sizeof(mac_addr_t));
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    *attr_count = thrift->len;
    return rc;
}

#define __PTF_HANDLER_PROCESS_FUNCTION__

#define __PTF_HANDLER_PROCESS_FUNCTION_PORT__

gboolean
ptf_hdlr_sai_thrift_set_port_attribute (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t port_id, const sai_thrift_attribute_t * thrift_attr, GError **error)
{
    sai_port_api_t *port_api = NULL;
    sai_attribute_t attr[1];
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_PORT, (void**)&port_api));

    /* 2. convert thrift format to SAI format */
    PTF_ERROR_RETURN(ptf_hdlr_format_port_attr_thrift_to_sai(thrift_attr, attr));

    /* 3. debug */
    PTF_HDLR_LOG("##### port API - set_port_attribute(), portid: "u64_fmt" #####\n", 
        port_id);

    switch (attr->id)
    {
    case SAI_PORT_ATTR_PORT_VLAN_ID:
        PTF_HDLR_LOG("  attr_id: %s, vid: %u\n", "SAI_PORT_ATTR_PORT_VLAN_ID", attr->value.u16);
        break;
    case SAI_PORT_ATTR_ADMIN_STATE:
        PTF_HDLR_LOG("  attr_id: %s, enable: %u\n", "SAI_PORT_ATTR_ADMIN_STATE", attr->value.booldata);
        break;        
    case SAI_PORT_ATTR_DROP_UNTAGGED:
        PTF_HDLR_LOG("  attr_id: %s, enable: %u\n", "SAI_PORT_ATTR_DROP_UNTAGGED", attr->value.booldata);
        break;
    case SAI_PORT_ATTR_DROP_TAGGED:
        PTF_HDLR_LOG("  attr_id: %s, enable: %u\n", "SAI_PORT_ATTR_DROP_TAGGED", attr->value.booldata);
        break;
        
    default:
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* 4. call SAI API */
    if (port_api)
    {
        PTF_ERROR_RETURN(port_api->set_port_attribute(port_id, attr));
    }
    else
    {
        PTF_HDLR_LOG("call port_api->set_port_attribute()\n");
    }
    
    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

#define __PTF_HANDLER_PROCESS_FUNCTION_FDB__

gboolean
ptf_hdlr_sai_thrift_create_fdb_entry (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, 
        const sai_thrift_fdb_entry_t * thrift_fdb_entry, const GPtrArray * thrift_attr_list, GError **error)
{
    char mac_str[32];
    sai_fdb_api_t *fdb_api = NULL;
    sai_fdb_entry_t fdb_entry;
    sai_attribute_t attr[3];
    uint32 attr_count = 0;
    uint32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_FDB, (void**)&fdb_api));

    /* 2. convert thrift format to SAI format */
    PTF_ERROR_RETURN(ptf_hdlr_format_fdb_entry_thrift_to_sai(thrift_fdb_entry, &fdb_entry));
    PTF_ERROR_RETURN(ptf_hdlr_format_fdb_entry_attr_list_thrift_to_sai(thrift_attr_list, attr, &attr_count));
    ptf_hdlr_mac_val2str(fdb_entry.mac_address, mac_str);

    /* 3. debug */
    PTF_HDLR_LOG("##### fdb API - create_fdb_entry(mac=%s, vid=%u, attr_num=%u) #####\n", 
        mac_str, fdb_entry.vlan_id, attr_count);

    for (i = 0; i < attr_count; i++)
    {
        switch (attr[i].id)
        {
        case SAI_FDB_ENTRY_ATTR_TYPE:
            PTF_HDLR_LOG("  type: %u\n", attr[i].value.s32);
            break;
        case SAI_FDB_ENTRY_ATTR_PORT_ID:
            PTF_HDLR_LOG("  portid: "u64_fmt"\n", attr[i].value.oid);
            break;
        case SAI_FDB_ENTRY_ATTR_PACKET_ACTION:
            PTF_HDLR_LOG("  action: %u\n", attr[i].value.s32);
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    /* 4. call SAI API */
    if (fdb_api)
    {
        PTF_ERROR_RETURN(fdb_api->create_fdb_entry(&fdb_entry, attr_count, attr));
    }
    else
    {
        PTF_HDLR_LOG("call fdb_api->create_fdb_entry()\n");
    }
    
    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_delete_fdb_entry (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, 
        const sai_thrift_fdb_entry_t * thrift_fdb_entry, GError **error)
{
    char mac_str[32];
    sai_fdb_api_t *fdb_api = NULL;
    sai_fdb_entry_t fdb_entry;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_FDB, (void**)&fdb_api));

    /* 2. convert thrift format to SAI format */
    PTF_ERROR_RETURN(ptf_hdlr_format_fdb_entry_thrift_to_sai(thrift_fdb_entry, &fdb_entry));
    ptf_hdlr_mac_val2str(fdb_entry.mac_address, mac_str);

    /* 3. debug */
    PTF_HDLR_LOG("##### fdb API - delete_fdb_entry(mac=%s, vid=%u) #####\n", 
        mac_str, fdb_entry.vlan_id);

    /* 4. call SAI API */
    if (fdb_api)
    {
        PTF_ERROR_RETURN(fdb_api->remove_fdb_entry(&fdb_entry));
    }
    else
    {
        PTF_HDLR_LOG("call fdb_api->remove_fdb_entry()\n");
    }
    
    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_flush_fdb_entries (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, 
        const GPtrArray * thrift_attr_list, GError **error)
{
    sai_fdb_api_t *fdb_api = NULL;
    sai_attribute_t attr[3];
    uint32 attr_count = 0;
    uint32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_FDB, (void**)&fdb_api));

    /* 2. convert thrift format to SAI format */
    PTF_ERROR_RETURN(ptf_hdlr_format_fdb_flush_attr_list_thrift_to_sai(thrift_attr_list, attr, &attr_count));

    /* 3. debug */
    PTF_HDLR_LOG("##### fdb API - flush_fdb_entries(attr_num=%u) #####\n", attr_count);

    for (i = 0; i < attr_count; i++)
    {
        switch (attr[i].id)
        {
        case SAI_FDB_FLUSH_ATTR_PORT_ID:
            PTF_HDLR_LOG("  portid: "u64_fmt"\n", attr[i].value.oid);
            break;
        case SAI_FDB_FLUSH_ATTR_VLAN_ID:
            PTF_HDLR_LOG("  vid: %u\n", attr[i].value.u16);
            break;
        case SAI_FDB_FLUSH_ATTR_ENTRY_TYPE:
            PTF_HDLR_LOG("  flush type: %u\n", attr[i].value.s32);
            break;
        case SAI_FDB_FLUSH_ATTR_MAC:
            PTF_HDLR_LOG("  flush type: %02X:%02X:%02X:%02X:%02X:%02X\n", 
                attr[i].value.mac[0], attr[i].value.mac[1], attr[i].value.mac[2],
                attr[i].value.mac[3], attr[i].value.mac[4], attr[i].value.mac[5]);
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    /* 4. call SAI API */
    if (fdb_api)
    {
        PTF_ERROR_RETURN(fdb_api->flush_fdb_entries(attr_count, attr));
    }
    else
    {
        PTF_HDLR_LOG("call fdb_api->flush_fdb_entries()\n");
    }
    
    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

#define __PTF_HANDLER_PROCESS_FUNCTION_VLAN__

gboolean
ptf_hdlr_sai_thrift_create_vlan(switch_sai_rpcIf *iface, sai_thrift_status_t* _return, 
        const sai_thrift_vlan_id_t vlan_id, GError **error)
{
    sai_vlan_api_t *vlan_api = NULL;
    sai_vlan_id_t vid = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_VLAN, (void**)&vlan_api));

    /* 2. convert thrift format to SAI format */
    vid = vlan_id;
    
    /* 3. debug */
    PTF_HDLR_LOG("##### vlan API - create_vlan(vid=%u) #####\n", vid);

    /* 4. call SAI API */
    if (vlan_api)
    {
        PTF_ERROR_RETURN(vlan_api->create_vlan(vid));
    }
    else
    {
        PTF_HDLR_LOG("call vlan_api->create_vlan()\n");
    }
    
    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_delete_vlan(switch_sai_rpcIf *iface, sai_thrift_status_t* _return, 
        const sai_thrift_vlan_id_t vlan_id, GError **error)
{
    sai_vlan_api_t *vlan_api = NULL;
    sai_vlan_id_t vid = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_VLAN, (void**)&vlan_api));

    /* 2. convert thrift format to SAI format */
    vid = vlan_id;
    
    /* 3. debug */
    PTF_HDLR_LOG("##### vlan API - delete_vlan(vid=%u) #####\n", vid);

    /* 4. call SAI API */
    if (vlan_api)
    {
        PTF_ERROR_RETURN(vlan_api->remove_vlan(vid));
    }
    else
    {
        PTF_HDLR_LOG("call vlan_api->remove_vlan()\n");
    }
    
    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

int32
ptf_hdlr_format_vlan_port_list_thrift_to_sai(const GPtrArray *thrift, sai_vlan_port_t *sai, uint32 *port_count)
{
    sai_thrift_vlan_port_t *thrift_ports[PTF_MAX_PORT_NUM];
    int32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    *port_count = 0;

    if (thrift->len > PTF_MAX_PORT_NUM)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        thrift_ports[i] = g_ptr_array_index(thrift, i);
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        sai[i].port_id = thrift_ports[i]->port_id;
        sai[i].tagging_mode = thrift_ports[i]->tagging_mode;
    }
    *port_count = thrift->len;
    
    return rc;
}

gboolean
ptf_hdlr_sai_thrift_add_ports_to_vlan(switch_sai_rpcIf *iface, sai_thrift_status_t* _return, 
        const sai_thrift_vlan_id_t vlan_id, const GPtrArray * thrift_port_list, GError **error)
{
    sai_vlan_api_t *vlan_api = NULL;
    sai_vlan_port_t vlan_ports[PTF_MAX_PORT_NUM];
    uint32 port_count = 0;
    sai_vlan_id_t vid = 0;
    uint32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_VLAN, (void**)&vlan_api));

    /* 2. convert thrift format to SAI format */
    vid = vlan_id;
    PTF_ERROR_RETURN(ptf_hdlr_format_vlan_port_list_thrift_to_sai(thrift_port_list, vlan_ports, &port_count));
    
    /* 3. debug */
    PTF_HDLR_LOG("##### vlan API - add_ports_to_vlan(vid=%u) #####\n", vid);
    for (i = 0; i < port_count; i++)
    {
        PTF_HDLR_LOG("  port[%u], port_id "u64_fmt", tagging_mode %u\n", 
            i, vlan_ports[i].port_id, vlan_ports[i].tagging_mode);
    }
    
    /* 4. call SAI API */
    if (vlan_api)
    {
        PTF_ERROR_RETURN(vlan_api->add_ports_to_vlan(vid, port_count, vlan_ports));
    }
    else
    {
        PTF_HDLR_LOG("call vlan_api->add_ports_to_vlan()\n");
    }

    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_ports_from_vlan(switch_sai_rpcIf *iface, sai_thrift_status_t* _return, 
        const sai_thrift_vlan_id_t vlan_id, const GPtrArray * thrift_port_list, GError **error)
{
    sai_vlan_api_t *vlan_api = NULL;
    sai_vlan_port_t vlan_ports[PTF_MAX_PORT_NUM];
    uint32 port_count = 0;
    sai_vlan_id_t vid = 0;
    uint32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_VLAN, (void**)&vlan_api));

    /* 2. convert thrift format to SAI format */
    vid = vlan_id;
    PTF_ERROR_RETURN(ptf_hdlr_format_vlan_port_list_thrift_to_sai(thrift_port_list, vlan_ports, &port_count));

    /* 3. debug */
    PTF_HDLR_LOG("##### vlan API - remove_ports_from_vlan(vid=%u) #####\n", vid);
    for (i = 0; i < port_count; i++)
    {
        PTF_HDLR_LOG("  port[%u], port_id "u64_fmt", tagging_mode %u\n", 
            i, vlan_ports[i].port_id, vlan_ports[i].tagging_mode);
    }
    
    /* 4. call SAI API */
    if (vlan_api)
    {
        PTF_ERROR_RETURN(vlan_api->remove_ports_from_vlan(vid, port_count, vlan_ports));
    }
    else
    {
        PTF_HDLR_LOG("call vlan_api->remove_ports_from_vlan()\n");
    }

    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_get_vlan_stats (switch_sai_rpcIf *iface, GArray ** _return, const gint16 vlan_id, const GArray * counter_ids, const gint32 number_of_counters, GError **error)
{
    sai_vlan_api_t *vlan_api = NULL;
    sai_vlan_stat_counter_t sai_counter_ids[32];
    uint64_t counters[32];
    sai_vlan_id_t vid = 0;
    uint32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    rc = (sai_api_query(SAI_API_VLAN, (void**)&vlan_api));
    if (rc < 0)
    {
        return TRUE;
    }

    /* 2. convert thrift format to SAI format */
    sal_memset(counters, 0, sizeof(counters));
    vid = vlan_id;

    /* 3. debug */
    PTF_HDLR_LOG("##### vlan API - get_vlan_stats(vid=%u, number_of_counters=%u) #####\n", vid, number_of_counters);
    for (i = 0; i < number_of_counters; i++)
    {
        sai_counter_ids[i] = g_array_index (counter_ids, gint32, i);
        PTF_HDLR_LOG("  counter_ids[%u] = %d\n", i, sai_counter_ids[i]);
    }
    
    /* 4. call SAI API */
    if (vlan_api)
    {
        rc = (vlan_api->get_vlan_stats(vid, sai_counter_ids, number_of_counters, counters));
        if (rc < 0)
        {
            return TRUE;
        }
    }
    else
    {
        PTF_HDLR_LOG("call vlan_api->get_vlan_stats()\n");
        for (i = 0; i < number_of_counters; i++)
        {
            counters[i] = 100 * i;
        }
    }

    for (i = 0; i < number_of_counters; i++)
    {
        g_array_append_val (*_return, counters[i]);
    }
    
    /* 5. return result */

    return TRUE;
}

#define __PTF_HANDLER_PROCESS_FUNCTION_VIRTUAL_ROUTER__


int32
ptf_hdlr_format_virtual_router_attr_list_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai, uint32 *attr_count)
{
    sai_thrift_attribute_t *thrift_attr[5];
    int32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    *attr_count = 0;

    if (thrift->len > 5)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        thrift_attr[i] = g_ptr_array_index(thrift, i);
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        switch (thrift_attr[i]->id)
        {
        case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE:
        case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE:
            sai[i].id = thrift_attr[i]->id;
            sai[i].value.booldata = thrift_attr[i]->value->booldata;
            break;
        case SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS:
            sai[i].id = thrift_attr[i]->id;
            rc = ptf_hdlr_format_mac_thrift_string_to_sai(thrift_attr[i]->value->mac, sai->value.mac);
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    *attr_count = thrift->len;
    
    return rc;
}


gboolean
ptf_hdlr_sai_thrift_create_virtual_router (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error)
{
    char mac_str[32];
    sai_attribute_t attr[5];
    uint32 attr_count = 0;
    uint32 i = 0;
    sai_object_id_t vr_id = 0;
    sai_virtual_router_api_t *vr_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_VIRTUAL_ROUTER, (void**)&vr_api));

    /* 2. convert thrift format to SAI format */
    PTF_ERROR_RETURN(ptf_hdlr_format_virtual_router_attr_list_thrift_to_sai(thrift_attr_list, attr, &attr_count));

    /* 3. debug */
    mac_str[0] = '\0';
    PTF_HDLR_LOG("##### virtual_router API - create_virtual_router() #####\n");
    for (i = 0; i < attr_count; i++)
    {
        switch (attr[i].id)
        {
        case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE:
            PTF_HDLR_LOG("  SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE enable = %u\n", attr[i].value.booldata);
            break;
        case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE:
            PTF_HDLR_LOG("  SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE enable = %u\n", attr[i].value.booldata);
            break;
        case SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS:
            ptf_hdlr_mac_val2str(attr[i].value.mac, mac_str);
            PTF_HDLR_LOG("  SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS MAC = %s\n", mac_str);
            break;
        }
    }

    /* 4. call SAI API */
    if (vr_api)
    {
        PTF_ERROR_RETURN(vr_api->create_virtual_router(&vr_id, attr_count, attr));
    }
    else
    {
        PTF_HDLR_LOG("call vr_api->create_virtual_router(), vr_id = %lld\n", vr_id);
        vr_id = 0x30000000A;
    }
    
    /* 5. return result */
    *_return = vr_id;

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_virtual_router (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t vr_id, GError **error)
{
    sai_virtual_router_api_t *vr_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_VIRTUAL_ROUTER, (void**)&vr_api));

    /* 2. convert thrift format to SAI format */

    /* 3. debug */
    PTF_HDLR_LOG("##### virtual router API - remove_virtual_router(vr_id="u64_fmt") #####\n", vr_id);

    /* 4. call SAI API */
    if (vr_api)
    {
        PTF_ERROR_RETURN(vr_api->remove_virtual_router(vr_id));
    }
    else
    {
        PTF_HDLR_LOG("call vr_api->remove_virtual_router()\n");
    }
    
    /* 5. return result */    
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

#define __PTF_HANDLER_PROCESS_FUNCTION_ROUTE__

int32 
sai_thrift_string_to_v4_ip(gchar *v4_str, uint32 *v4_ip)
{
    int32 rc = 0;
    rc = sal_inet_pton(AF_INET, v4_str, v4_ip);
    if (rc <= 0)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    return SAI_STATUS_SUCCESS;
}

int32 
sai_thrift_string_to_v6_ip(gchar *v6_str, uint8 *v6_ip)
{
    int32 rc = 0;
    rc = sal_inet_pton(AF_INET6, v6_str, v6_ip);
    if (rc <= 0)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    return SAI_STATUS_SUCCESS;
}

int32
ptf_hdlr_format_ip_address_thrift_to_sai(const sai_thrift_ip_address_t *thrift, sai_ip_address_t *sai)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    if (SAI_IP_ADDR_FAMILY_IPV4 == thrift->addr_family)
    {
        sal_memset(sai, 0, sizeof(sai_ip_address_t));
        sai->addr_family = thrift->addr_family;
        rc |= sai_thrift_string_to_v4_ip(thrift->addr->ip4, &sai->addr.ip4);
    }
    else if (SAI_IP_ADDR_FAMILY_IPV6 == thrift->addr_family)
    {
        sai->addr_family = thrift->addr_family;
        rc |= sai_thrift_string_to_v6_ip(thrift->addr->ip6, sai->addr.ip6);
    }
    else
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    return SAI_STATUS_SUCCESS;
}


int32
ptf_hdlr_format_ip_prefix_thrift_to_sai(const sai_thrift_ip_prefix_t *thrift, sai_ip_prefix_t *sai)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    if (SAI_IP_ADDR_FAMILY_IPV4 == thrift->addr_family)
    {
        sal_memset(sai, 0, sizeof(sai_ip_prefix_t));
        sai->addr_family = thrift->addr_family;
        rc |= sai_thrift_string_to_v4_ip(thrift->addr->ip4, &sai->addr.ip4);
        rc |= sai_thrift_string_to_v4_ip(thrift->mask->ip4, &sai->mask.ip4);
    }
    else if (SAI_IP_ADDR_FAMILY_IPV6 == thrift->addr_family)
    {
        sai->addr_family = thrift->addr_family;
        rc |= sai_thrift_string_to_v6_ip(thrift->addr->ip6, sai->addr.ip6);
        rc |= sai_thrift_string_to_v6_ip(thrift->mask->ip6, sai->mask.ip6);
    }
    else
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    return SAI_STATUS_SUCCESS;
}

int32
ptf_hdlr_format_unicast_route_entry_thrift_to_sai(const sai_thrift_unicast_route_entry_t * thrift, sai_unicast_route_entry_t *sai)
{
    sai->vr_id = thrift->vr_id;
    return ptf_hdlr_format_ip_prefix_thrift_to_sai(thrift->destination, &sai->destination);
}

int32
ptf_hdlr_format_neighbor_entry_thrift_to_sai(const sai_thrift_neighbor_entry_t * thrift, sai_neighbor_entry_t *sai)
{
    sai->rif_id = thrift->rif_id;
    return ptf_hdlr_format_ip_address_thrift_to_sai(thrift->ip_address, &sai->ip_address);
}

int32
ptf_hdlr_format_route_attr_list_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai, uint32 *attr_count)
{
    sai_thrift_attribute_t *thrift_attr[32];
    int32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    *attr_count = 0;

    if (thrift->len > 32)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        thrift_attr[i] = g_ptr_array_index(thrift, i);
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        switch (thrift_attr[i]->id)
        {
        case SAI_ROUTE_ATTR_NEXT_HOP_ID:
            sai[i].id = thrift_attr[i]->id;
            sai[i].value.oid = thrift_attr[i]->value->oid;
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    *attr_count = thrift->len;
    
    return rc;
}

gboolean
ptf_hdlr_sai_thrift_create_route (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_unicast_route_entry_t * thrift_unicast_route_entry, const GPtrArray * thrift_attr_list, GError **error)
{
    sai_unicast_route_entry_t ucast_route_entry;
    sai_attribute_t attr[32];
    char route_entry_str[128];
    uint32 attr_count = 0;
    uint32 i = 0;
    sai_route_api_t *route_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_ROUTE, (void**)&route_api));

    /* 2. convert thrift format to SAI format */
    sal_memset(&ucast_route_entry, 0, sizeof(ucast_route_entry));
    PTF_ERROR_RETURN(ptf_hdlr_format_unicast_route_entry_thrift_to_sai(thrift_unicast_route_entry, &ucast_route_entry));
    PTF_ERROR_RETURN(ptf_hdlr_format_route_attr_list_thrift_to_sai(thrift_attr_list, attr, &attr_count));

    /* 3. debug */
    PTF_HDLR_LOG("##### route_api API - create_route() #####\n");
    ptf_hdlr_route_entry_val2str(&ucast_route_entry, route_entry_str);
    PTF_HDLR_LOG("  route_entry: %s\n", route_entry_str);
    for (i = 0; i < attr_count; i++)
    {
        switch (attr[i].id)
        {
        case SAI_ROUTE_ATTR_NEXT_HOP_ID:
            PTF_HDLR_LOG("  SAI_ROUTE_ATTR_NEXT_HOP_ID nexthop_id[%u] = "u64_fmt"\n", i, attr[i].value.oid);
            break;
        }
    }

    /* 4. call SAI API */
    if (route_api)
    {
        PTF_ERROR_RETURN(route_api->create_route(&ucast_route_entry, attr_count, attr));
    }
    else
    {
        PTF_HDLR_LOG("call route_api->create_route()\n");
    }
    
    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_route (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_unicast_route_entry_t * thrift_unicast_route_entry, GError **error)
{
    sai_unicast_route_entry_t ucast_route_entry;
    char route_entry_str[128];
    sai_route_api_t *route_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_ROUTE, (void**)&route_api));

    /* 2. convert thrift format to SAI format */
    sal_memset(&ucast_route_entry, 0, sizeof(ucast_route_entry));
    PTF_ERROR_RETURN(ptf_hdlr_format_unicast_route_entry_thrift_to_sai(thrift_unicast_route_entry, &ucast_route_entry));

    /* 3. debug */
    PTF_HDLR_LOG("##### route_api API - remove_route() #####\n");
    ptf_hdlr_route_entry_val2str(&ucast_route_entry, route_entry_str);
    PTF_HDLR_LOG("  route_entry: %s\n", route_entry_str);

    /* 4. call SAI API */
    if (route_api)
    {
        PTF_ERROR_RETURN(route_api->remove_route(&ucast_route_entry));
    }
    else
    {
        PTF_HDLR_LOG("call route_api->remove_route()\n");
    }
    
    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

#define __PTF_HANDLER_PROCESS_FUNCTION_ROUTER_INTERFACE__


int32
ptf_hdlr_format_rif_attr_list_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai, uint32 *attr_count)
{
    sai_thrift_attribute_t *thrift_attr[8];
    int32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    *attr_count = 0;

    if (thrift->len > 8)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        thrift_attr[i] = g_ptr_array_index(thrift, i);
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        sai[i].id = thrift_attr[i]->id;
        switch (thrift_attr[i]->id)
        {
        case SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID:
            sai[i].value.oid= thrift_attr[i]->value->oid;
            break;
        case SAI_ROUTER_INTERFACE_ATTR_TYPE:
            sai[i].value.s32 = thrift_attr[i]->value->s32;
            break;
        case SAI_ROUTER_INTERFACE_ATTR_PORT_ID:
            sai[i].value.oid= thrift_attr[i]->value->oid;
            break;
        case SAI_ROUTER_INTERFACE_ATTR_VLAN_ID:
            sai[i].value.u16= thrift_attr[i]->value->u16;
            break;
        case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
            rc = ptf_hdlr_format_mac_thrift_string_to_sai(thrift_attr[i]->value->mac, sai[i].value.mac);
            break;
        case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE:
            sai[i].value.booldata = thrift_attr[i]->value->booldata;
            break;
        case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE:
            sai[i].value.booldata = thrift_attr[i]->value->booldata;
            break;
        case SAI_ROUTER_INTERFACE_ATTR_MTU:
            sai[i].value.u32 = thrift_attr[i]->value->u32;
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    *attr_count = thrift->len;
    
    return rc;
}



gboolean
ptf_hdlr_sai_thrift_create_router_interface (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error)
{
    PTF_HDLR_LOG("##### router interface API - create_router_interface() #####\n");
    sai_object_id_t rif_id = 0;
    char mac_str[32];
    sai_attribute_t attr[8];
    uint32 attr_count = 0;
    uint32 i = 0;
    sai_router_interface_api_t*rif_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE, (void**)&rif_api));

    /* 2. convert thrift format to SAI format */
    PTF_ERROR_RETURN(ptf_hdlr_format_rif_attr_list_thrift_to_sai(thrift_attr_list, attr, &attr_count));

    /* 3. debug */
    PTF_HDLR_LOG("##### router interface API - create_router_interface() #####\n");

    for (i = 0; i < attr_count; i++)
    {
        switch (attr[i].id)
        {
        case SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID:
            PTF_HDLR_LOG("  SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID %llx\n", attr[i].value.oid);
            break;
        case SAI_ROUTER_INTERFACE_ATTR_TYPE:
            PTF_HDLR_LOG("  SAI_ROUTER_INTERFACE_ATTR_TYPE %d\n", attr[i].value.s32);
            break;
        case SAI_ROUTER_INTERFACE_ATTR_PORT_ID:
            PTF_HDLR_LOG("  SAI_ROUTER_INTERFACE_ATTR_PORT_ID %llx\n", attr[i].value.oid);
            break;
        case SAI_ROUTER_INTERFACE_ATTR_VLAN_ID:
            PTF_HDLR_LOG("  SAI_ROUTER_INTERFACE_ATTR_VLAN_ID %d\n", attr[i].value.u16);
            break;
        case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
            ptf_hdlr_mac_val2str(attr[i].value.mac, mac_str);
            PTF_HDLR_LOG("  SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS MAC = %s\n", mac_str);
            break;
        case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE:
            PTF_HDLR_LOG("  SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE %d\n", attr[i].value.s32);
            break;
        case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE:
            PTF_HDLR_LOG("  SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE %d\n", attr[i].value.s32);
            break;
        case SAI_ROUTER_INTERFACE_ATTR_MTU:
            PTF_HDLR_LOG("  SAI_ROUTER_INTERFACE_ATTR_MTU %d\n", attr[i].value.s32);
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    /* 4. call SAI API */
    if (rif_api)
    {
        PTF_ERROR_RETURN(rif_api->create_router_interface(&rif_id, attr_count, attr));
    }
    else
    {
        PTF_HDLR_LOG("call rif_api->create_router_interface\n");
        rif_id = 333;
    }
    
    /* 5. return result */
    *_return = rif_id;

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_router_interface (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t rif_id, GError **error)
{
    PTF_HDLR_LOG("##### router interface API - remove_router_interface() #####\n");
    sai_router_interface_api_t *rif_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE, (void**)&rif_api));

    /* 2. convert thrift format to SAI format */

    /* 3. debug */
    PTF_HDLR_LOG("##### router interface API - remove_router_interface(rif_id="u64_fmt") #####\n", rif_id);

    /* 4. call SAI API */
    if (rif_api)
    {
        PTF_ERROR_RETURN(rif_api->remove_router_interface(rif_id));
    }
    else
    {
        PTF_HDLR_LOG("call rif_api->remove_virtual_router()\n");
    }
    
    /* 5. return result */    
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

#define __PTF_HANDLER_PROCESS_FUNCTION_NEXTHOP__

int32
ptf_hdlr_format_next_hop_attr_list_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai, uint32 *attr_count)
{
    sai_thrift_attribute_t *thrift_attr[8];
    int32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    *attr_count = 0;

    if (thrift->len > 8)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        thrift_attr[i] = g_ptr_array_index(thrift, i);
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        sai[i].id = thrift_attr[i]->id;
        switch (thrift_attr[i]->id)
        {
        case SAI_NEXT_HOP_ATTR_TYPE:
            sai[i].value.s32 = thrift_attr[i]->value->s32;
            break;
        case SAI_NEXT_HOP_ATTR_IP:
            ptf_hdlr_format_ip_address_thrift_to_sai(thrift_attr[i]->value->ipaddr, &sai[i].value.ipaddr);
            break;
        case SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID:
            sai[i].value.oid = thrift_attr[i]->value->oid;
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    *attr_count = thrift->len;
    
    return rc;
}

gboolean
ptf_hdlr_sai_thrift_create_next_hop (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error)
{
    sai_object_id_t next_hop_id = 0;
    sai_attribute_t attr[8];
    char ip_addr_str[128];
    uint32 attr_count = 0;
    uint32 i = 0;
    sai_next_hop_api_t *nh_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_NEXT_HOP, (void**)&nh_api));

    /* 2. convert thrift format to SAI format */
    PTF_ERROR_RETURN(ptf_hdlr_format_next_hop_attr_list_thrift_to_sai(thrift_attr_list, attr, &attr_count));

    /* 3. debug */
    PTF_HDLR_LOG("##### next hop API - create_next_hop() #####\n");
    for (i = 0; i < attr_count; i++)
    {
        switch (attr[i].id)
        {
        case SAI_NEXT_HOP_ATTR_TYPE:
            PTF_HDLR_LOG("  SAI_NEXT_HOP_ATTR_TYPE %d\n", attr[i].value.s32);
            break;
        case SAI_NEXT_HOP_ATTR_IP:
            ptf_hdlr_ip_address_val2str(&attr[i].value.ipaddr, ip_addr_str);
            PTF_HDLR_LOG("  SAI_NEXT_HOP_ATTR_IP %s\n", ip_addr_str);
            break;
        case SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID:
            PTF_HDLR_LOG("  SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID "u64_fmt"\n", attr[i].value.oid);
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    /* 4. call SAI API */
    if (nh_api)
    {
        PTF_ERROR_RETURN(nh_api->create_next_hop(&next_hop_id, attr_count, attr));
    }
    else
    {
        PTF_HDLR_LOG("call nh_api->create_route()\n");
        next_hop_id = 333;
    }
    
    /* 5. return result */
    *_return = next_hop_id;

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_next_hop (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t next_hop_id, GError **error)
{
    sai_next_hop_api_t *nh_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_NEXT_HOP, (void**)&nh_api));

    /* 2. convert thrift format to SAI format */
    
    /* 3. debug */
    PTF_HDLR_LOG("##### next hop API - remove_next_hop(next_hop_id="u64_fmt") #####\n", next_hop_id);
    
    /* 4. call SAI API */
    if (nh_api)
    {
        PTF_ERROR_RETURN(nh_api->remove_next_hop(next_hop_id));
    }
    else
    {
        PTF_HDLR_LOG("call nh_api->remove_next_hop()\n");
    }

    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

#define __PTF_HANDLER_PROCESS_FUNCTION_NEXTHOP_GROUP__

int32
ptf_hdlr_format_next_hop_group_attr_list_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai, uint32 *attr_count,
    sai_object_id_t *next_hops)
{
    sai_thrift_attribute_t *thrift_attr[8];
    uint32 i = 0;
    uint32 j = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    *attr_count = 0;

    if (thrift->len > 8)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        thrift_attr[i] = g_ptr_array_index(thrift, i);
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        sai[i].id = thrift_attr[i]->id;
        switch (thrift_attr[i]->id)
        {
        case SAI_NEXT_HOP_GROUP_ATTR_TYPE:
            sai[i].value.s32 = thrift_attr[i]->value->s32;
            break;
        case SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT:
            sai[i].value.u32 = thrift_attr[i]->value->u32;
            break;
        case SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST:
            sai[i].value.objlist.list = next_hops;
            for (j = 0; j < thrift_attr[i]->value->objlist->count; j++)
            {
                sai[i].value.objlist.list[j] = g_array_index (thrift_attr[i]->value->objlist->object_id_list, gint64, j);
            }
            sai[i].value.objlist.count = thrift_attr[i]->value->objlist->count;
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    *attr_count = thrift->len;
    
    return rc;
}

gboolean
ptf_hdlr_sai_thrift_create_next_hop_group (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error)
{
    sai_object_id_t next_hops[64];
    sai_object_id_t next_hop_group_id = 0;
    sai_attribute_t attr[8];
    uint32 attr_count = 0;
    uint32 i = 0;
    uint32 j = 0;
    sai_next_hop_group_api_t *nh_group_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_NEXT_HOP_GROUP, (void**)&nh_group_api));

    /* 2. convert thrift format to SAI format */
    PTF_ERROR_RETURN(ptf_hdlr_format_next_hop_group_attr_list_thrift_to_sai(thrift_attr_list, attr, &attr_count, next_hops));

    /* 3. debug */
    PTF_HDLR_LOG("##### next hop group API - create_next_hop_group() #####\n");
    for (i = 0; i < attr_count; i++)
    {
        switch (attr[i].id)
        {
        case SAI_NEXT_HOP_GROUP_ATTR_TYPE:
            PTF_HDLR_LOG("  SAI_NEXT_HOP_GROUP_ATTR_TYPE %d\n", attr[i].value.s32);
            break;
        case SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT:
            PTF_HDLR_LOG("  SAI_NEXT_HOP_ATTR_IP %u\n", attr[i].value.u32);
            break;
        case SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST:
            PTF_HDLR_LOG("  SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST count %u\n", attr[i].value.objlist.count);
            for (j = 0; j < attr[i].value.objlist.count; j++)
            {
                PTF_HDLR_LOG("    NEXT_HOP_LIST[%u] = "u64_fmt"\n", j, attr[i].value.objlist.list[j]);
            }
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    /* 4. call SAI API */
    if (nh_group_api)
    {
        PTF_ERROR_RETURN(nh_group_api->create_next_hop_group(&next_hop_group_id, attr_count, attr));
    }
    else
    {
        PTF_HDLR_LOG("call nh_group_api->create_next_hop_group()\n");
        next_hop_group_id = 444;
    }
    
    /* 5. return result */
    *_return = next_hop_group_id;

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_next_hop_group (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t next_hop_group_id, GError **error)
{
    sai_next_hop_group_api_t *nh_group_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_NEXT_HOP_GROUP, (void**)&nh_group_api));

    /* 2. convert thrift format to SAI format */
    
    /* 3. debug */
    PTF_HDLR_LOG("##### next hop group API - remove_next_hop_group(next_hop_group_id="u64_fmt") #####\n", next_hop_group_id);
    
    /* 4. call SAI API */
    if (nh_group_api)
    {
        PTF_ERROR_RETURN(nh_group_api->remove_next_hop_group(next_hop_group_id));
    }
    else
    {
        PTF_HDLR_LOG("call nh_group_api->remove_next_hop_group()\n");
    }

    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

int32
ptf_hdlr_format_next_hop_list_thrift_to_sai(const GArray *thrift, sai_object_id_t *sai, uint32 *count)
{
    uint32 i = 0;
    
    for (i = 0; i < thrift->len; i++)
    {
        sai[i] = g_array_index (thrift, gint64, i);
    }
    *count = thrift->len;
    
    return SAI_STATUS_SUCCESS;
}
  
gboolean
ptf_hdlr_sai_thrift_add_next_hop_to_group (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t next_hop_group_id, const GArray * thrift_nexthops, GError **error)
{
    sai_next_hop_group_api_t *nh_group_api = NULL;
    sai_object_id_t next_hops[64];
    uint32 next_hop_count = 0;
    uint32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_NEXT_HOP_GROUP, (void**)&nh_group_api));

    /* 2. convert thrift format to SAI format */
    
    /* 3. debug */
    PTF_ERROR_RETURN(ptf_hdlr_format_next_hop_list_thrift_to_sai(thrift_nexthops, next_hops, &next_hop_count));
    PTF_HDLR_LOG("##### next hop group API - add_next_hop_to_group(next_hop_group_id="u64_fmt") #####\n", next_hop_group_id);
    PTF_HDLR_LOG("  next_hop_count %u\n", next_hop_count);
    for (i = 0; i < next_hop_count; i++)
    {
        PTF_HDLR_LOG("    next_hops[%u] = "u64_fmt"\n", i, next_hops[i]);
    }
    
    /* 4. call SAI API */
    if (nh_group_api)
    {
        PTF_ERROR_RETURN(nh_group_api->add_next_hop_to_group(next_hop_group_id, next_hop_count, next_hops));
    }
    else
    {
        PTF_HDLR_LOG("call nh_group_api->add_next_hop_to_group()\n");
    }

    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_next_hop_from_group (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t next_hop_group_id, const GArray * thrift_nexthops, GError **error)
{
    sai_next_hop_group_api_t *nh_group_api = NULL;
    sai_object_id_t next_hops[64];
    uint32 next_hop_count = 0;
    uint32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_NEXT_HOP_GROUP, (void**)&nh_group_api));

    /* 2. convert thrift format to SAI format */
    
    /* 3. debug */
    PTF_ERROR_RETURN(ptf_hdlr_format_next_hop_list_thrift_to_sai(thrift_nexthops, next_hops, &next_hop_count));
    PTF_HDLR_LOG("##### next hop group API - remove_next_hop_from_group(next_hop_group_id="u64_fmt") #####\n", next_hop_group_id);
    PTF_HDLR_LOG("  next_hop_count %u\n", next_hop_count);
    for (i = 0; i < next_hop_count; i++)
    {
        PTF_HDLR_LOG("    next_hops[%u] = "u64_fmt"\n", i, next_hops[i]);
    }

    /* 4. call SAI API */
    if (nh_group_api)
    {
        PTF_ERROR_RETURN(nh_group_api->remove_next_hop_from_group(next_hop_group_id, next_hop_count, next_hops));
    }
    else
    {
        PTF_HDLR_LOG("call nh_group_api->remove_next_hop_from_group()\n");
    }

    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

#define __PTF_HANDLER_PROCESS_FUNCTION_LAG__

int32
ptf_hdlr_format_port_list_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai)
{
    sai_thrift_attribute_t *thrift_attr = NULL;
    int32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    sai->value.objlist.count = 0;
    if (thrift->len)
    {
        thrift_attr = g_ptr_array_index(thrift, 0);
        sai->id = thrift_attr->id;

        sai->value.objlist.count = 0;
        if (SAI_LAG_ATTR_PORT_LIST != sai->id)
        {
            return SAI_STATUS_INVALID_PARAMETER;
        }

        for (i = 0; i < thrift_attr->value->objlist->count; i++)
        {
            sai->value.objlist.list[i] = g_array_index (thrift_attr->value->objlist->object_id_list, gint64, i);
        }
        sai->value.objlist.count = thrift_attr->value->objlist->count;
    }

    return rc;
}
    
gboolean
ptf_hdlr_sai_thrift_create_lag (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error)
{
    sai_object_id_t port_id_list[PTF_MAX_PORT_NUM];
    sai_object_id_t lag_id = 0;
    sai_lag_api_t *lag_api = NULL;
    sai_attribute_t attr[1];
    uint32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_LAG, (void**)&lag_api));

    /* 2. convert thrift format to SAI format */
    attr->value.objlist.list = port_id_list;
    PTF_ERROR_RETURN(ptf_hdlr_format_port_list_thrift_to_sai(thrift_attr_list, attr));
    
    /* 3. debug */
    PTF_HDLR_LOG("##### lag API - create_lag() #####\n");
    for (i = 0; i < attr->value.objlist.count; i++)
    {
        PTF_HDLR_LOG("  port[%u], port_id "u64_fmt"\n", i, attr->value.objlist.list[i]);
    }
    
    /* 4. call SAI API */
    if (lag_api)
    {
        PTF_ERROR_RETURN(lag_api->create_lag(&lag_id, 1, attr));
    }
    else
    {
        PTF_HDLR_LOG("call lag_api->create_lag()\n");
        lag_id = SAI_OBJECT_TYPE_LAG | 3;
    }

    /* 5. return result */
    *_return = lag_id;
    PTF_HDLR_LOG("  returned lag_id = "u64_fmt"\n", lag_id);

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_lag (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t lag_id, GError **error)
{
    sai_lag_api_t *lag_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_LAG, (void**)&lag_api));

    /* 2. convert thrift format to SAI format */
    
    /* 3. debug */
    PTF_HDLR_LOG("##### lag API - remove_lag(lag_id="u64_fmt") #####\n", lag_id);
    
    /* 4. call SAI API */
    if (lag_api)
    {
        PTF_ERROR_RETURN(lag_api->remove_lag(lag_id));
    }
    else
    {
        PTF_HDLR_LOG("call lag_api->remove_lag()\n");
    }

    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

int32
ptf_hdlr_format_lag_member_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai, uint32 *attr_count)
{
    sai_thrift_attribute_t *thrift_attr = NULL;
    int32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    for (i = 0; i < thrift->len; i++)
    {
        thrift_attr = g_ptr_array_index(thrift, i);
        
        switch (thrift_attr->id)
        {
        case SAI_LAG_MEMBER_ATTR_LAG_ID:
            sai[i].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
            sai[i].value.oid = thrift_attr->value->oid;
            break;
        case SAI_LAG_MEMBER_ATTR_PORT_ID:
            sai[i].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
            sai[i].value.oid = thrift_attr->value->oid;
            break;
        default:
            break;
        }
    }
    *attr_count = i;
    
    return rc;
}

gboolean
ptf_hdlr_sai_thrift_create_lag_member (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error)
{
    sai_object_id_t lag_member_id = 0;
    sai_lag_api_t *lag_api = NULL;
    sai_attribute_t attr[2];
    uint32 attr_count = 0;
    uint32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_LAG, (void**)&lag_api));

    /* 2. convert thrift format to SAI format */
    PTF_ERROR_RETURN(ptf_hdlr_format_lag_member_thrift_to_sai(thrift_attr_list, attr, &attr_count));
    
    /* 3. debug */
    PTF_HDLR_LOG("##### lag API - create_lag_member() #####\n");
    for (i = 0; i < attr_count; i++)
    {
        switch (attr[i].id)
        {
        case SAI_LAG_MEMBER_ATTR_LAG_ID:
            PTF_HDLR_LOG("  lag_id "u64_fmt"\n", attr[i].value.oid);
            break;
        case SAI_LAG_MEMBER_ATTR_PORT_ID:
            PTF_HDLR_LOG("  port_id "u64_fmt"\n", attr[i].value.oid);
            break;
        default:
            break;
        }
    }
    
    /* 4. call SAI API */
    if (lag_api)
    {
        PTF_ERROR_RETURN(lag_api->create_lag_member(&lag_member_id, attr_count, attr));
    }
    else
    {
        PTF_HDLR_LOG("call lag_api->create_lag_member()\n");
        lag_member_id = SAI_OBJECT_TYPE_PORT | 3;
    }

    PTF_HDLR_LOG("  returned lag_member_id = "u64_fmt"\n", lag_member_id);
    
    /* 5. return result */
    *_return = lag_member_id;

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_lag_member (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t lag_member_id, GError **error)
{
    sai_object_id_t sai_lag_member_id = 0;
    sai_lag_api_t *lag_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_LAG, (void**)&lag_api));

    /* 2. convert thrift format to SAI format */
    
    /* 3. debug */
    sai_lag_member_id = lag_member_id;
    PTF_HDLR_LOG("##### lag API - remove_lag_member(lag_member_id="u64_fmt") #####\n", sai_lag_member_id);
    
    /* 4. call SAI API */
    if (lag_api)
    {
        PTF_ERROR_RETURN(lag_api->remove_lag_member(sai_lag_member_id));
    }
    else
    {
        PTF_HDLR_LOG("call lag_api->remove_lag_member()\n");
    }

    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

#define __PTF_HANDLER_PROCESS_FUNCTION_STP__


int32
ptf_hdlr_format_stp_attr_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai, uint32 *attr_count)
{
    sai_thrift_attribute_t *thrift_attr = NULL;
    int32 i = 0;
    int32 j = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    for (i = 0; i < thrift->len; i++)
    {
        thrift_attr = g_ptr_array_index(thrift, i);
        
        switch (thrift_attr->id)
        {
        case SAI_STP_ATTR_VLAN_LIST:
            sai->id = SAI_STP_ATTR_VLAN_LIST;
            for (j = 0; j < thrift_attr->value->vlanlist->vlan_count; j++)
            {
                sai->value.vlanlist.vlan_list[j] = 
                    g_array_index (thrift_attr->value->vlanlist->vlan_list, guint16, j);
            }
            sai->value.vlanlist.vlan_count = j;
            *attr_count = sai->value.vlanlist.vlan_count;
            break;

        default:
            break;
        }
    }
    
    return rc;
}

gboolean
ptf_hdlr_sai_thrift_create_stp_entry (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error)
{
    sai_vlan_id_t vlan_list[4096];
    sai_attribute_t attr[1];
    sai_stp_api_t *stp_api = NULL;
    sai_object_id_t stp_id = 0;
    uint32 attr_count = 0;
    uint32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_STP, (void**)&stp_api));

    /* 2. convert thrift format to SAI format */
    attr[0].value.vlanlist.vlan_list = vlan_list;
    PTF_ERROR_RETURN(ptf_hdlr_format_stp_attr_thrift_to_sai(thrift_attr_list, attr, &attr_count));
    
    /* 3. debug */
    PTF_HDLR_LOG("##### stp API - create_stp_entry() #####\n");
    
    for (i = 0; i < attr_count; i++)
    {
        PTF_HDLR_LOG("  vlan_id[%u] = %u\n", i, attr[0].value.vlanlist.vlan_list[i]);
    }
    
    /* 4. call SAI API */
    if (stp_api)
    {
        PTF_ERROR_RETURN(stp_api->create_stp(&stp_id, attr_count, attr));
    }
    else
    {
        PTF_HDLR_LOG("call stp_api->create_stp()\n");
        stp_id = 10;
    }

    /* 5. return result */
    *_return = stp_id;
    PTF_HDLR_LOG(" returned stp_id = %u\n", stp_id);
    
    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_stp_entry (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t stp_id, GError **error)
{
    sai_stp_api_t *stp_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_STP, (void**)&stp_api));

    /* 2. convert thrift format to SAI format */

    /* 3. debug */
    PTF_HDLR_LOG("##### stp API - create_stp_entry(stp_id="u64_fmt") #####\n", stp_id);
    
    /* 4. call SAI API */
    if (stp_api)
    {
        PTF_ERROR_RETURN(stp_api->remove_stp(stp_id));
    }
    else
    {
        PTF_HDLR_LOG("call stp_api->create_stp()\n");
    }

    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_set_stp_port_state (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t stp_id, const sai_thrift_object_id_t port_id, const sai_thrift_port_stp_port_state_t stp_port_state, GError **error)
{
    sai_stp_api_t *stp_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_STP, (void**)&stp_api));

    /* 2. convert thrift format to SAI format */
    
    /* 3. debug */
    PTF_HDLR_LOG("##### stp API - set_stp_port_state(stp_id: "u64_fmt", portid: "u64_fmt", state: %u) #####\n", 
    stp_id, port_id, stp_port_state);
    
    /* 4. call SAI API */
    if (stp_api)
    {
        PTF_ERROR_RETURN(stp_api->set_stp_port_state(stp_id, port_id, stp_port_state));
    }
    else
    {
        PTF_HDLR_LOG("call stp_api->set_stp_port_state()\n");
    }

    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_get_stp_port_state (switch_sai_rpcIf *iface, sai_thrift_port_stp_port_state_t* _return, const sai_thrift_object_id_t stp_id, const sai_thrift_object_id_t port_id, GError **error)
{
    sai_port_stp_port_state_t stp_port_state = 0;
    sai_stp_api_t *stp_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_STP, (void**)&stp_api));

    /* 2. convert thrift format to SAI format */
    
    /* 3. debug */
    PTF_HDLR_LOG("##### stp API - get_stp_port_state(stp_id: "u64_fmt", portid: "u64_fmt") #####\n", stp_id, port_id);
    
    /* 4. call SAI API */
    if (stp_api)
    {
        PTF_ERROR_RETURN(stp_api->get_stp_port_state(stp_id, port_id, &stp_port_state));
    }
    else
    {
        PTF_HDLR_LOG("call stp_api->get_stp_port_state()\n");
        stp_port_state = SAI_PORT_STP_STATE_FORWARDING;
    }

    /* 5. return result */
    *_return = stp_port_state;

    return TRUE;
}

#define __PTF_HANDLER_PROCESS_FUNCTION_NEIGHBOR__


int32
ptf_hdlr_format_neighbor_attr_list_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai, uint32 *attr_count)
{
    sai_thrift_attribute_t *thrift_attr[8];
    int32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    *attr_count = 0;

    if (thrift->len > 8)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        thrift_attr[i] = g_ptr_array_index(thrift, i);
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        sai[i].id = thrift_attr[i]->id;
        switch (thrift_attr[i]->id)
        {
        case SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS:
            rc = ptf_hdlr_format_mac_thrift_string_to_sai(thrift_attr[i]->value->mac, sai[i].value.mac);
            break;
        case SAI_NEIGHBOR_ATTR_PACKET_ACTION:
            sai[i].value.s32 = thrift_attr[i]->value->s32;
            break;
        case SAI_NEIGHBOR_ATTR_NO_HOST_ROUTE:
            sai[i].value.booldata = thrift_attr[i]->value->booldata;
            break;
        case SAI_NEIGHBOR_ATTR_META_DATA:
            sai[i].value.u32 = thrift_attr[i]->value->u32;
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    *attr_count = thrift->len;
    
    return rc;
}

gboolean
ptf_hdlr_sai_thrift_create_neighbor_entry (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_neighbor_entry_t * thrift_neighbor_entry, const GPtrArray * thrift_attr_list, GError **error)
{
    sai_neighbor_entry_t neighbor_entry;
    sai_attribute_t attr[8];
    char neighbor_str[128];
    char mac_str[128];
    uint32 attr_count = 0;
    uint32 i = 0;
    sai_neighbor_api_t *neighbor_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_NEIGHBOR, (void**)&neighbor_api));

    /* 2. convert thrift format to SAI format */
    sal_memset(&neighbor_entry, 0, sizeof(neighbor_entry));
    PTF_ERROR_RETURN(ptf_hdlr_format_neighbor_entry_thrift_to_sai(thrift_neighbor_entry, &neighbor_entry));
    PTF_ERROR_RETURN(ptf_hdlr_format_neighbor_attr_list_thrift_to_sai(thrift_attr_list, attr, &attr_count));

    /* 3. debug */
    PTF_HDLR_LOG("##### neighbor API - create_neighbor_entry() #####\n");
    ptf_hdlr_neighbor_val2str(&neighbor_entry, neighbor_str);
    PTF_HDLR_LOG("  neighbor: %s\n", neighbor_str);
    for (i = 0; i < attr_count; i++)
    {
        switch (attr[i].id)
        {
        case SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS:
            ptf_hdlr_mac_val2str(attr[i].value.mac, mac_str);
            PTF_HDLR_LOG("  SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS MAC = %s\n", mac_str);
            break;
        case SAI_NEIGHBOR_ATTR_PACKET_ACTION:
            PTF_HDLR_LOG("  SAI_NEIGHBOR_ATTR_PACKET_ACTION = %d\n", attr[i].value.s32);
            break;
        case SAI_NEIGHBOR_ATTR_NO_HOST_ROUTE:
            PTF_HDLR_LOG("  SAI_NEIGHBOR_ATTR_NO_HOST_ROUTE = %d\n", attr[i].value.s32);
            break;
        case SAI_NEIGHBOR_ATTR_META_DATA:
            PTF_HDLR_LOG("  SAI_NEIGHBOR_ATTR_META_DATA = %d\n", attr[i].value.s32);
            break;
        default:
            PTF_HDLR_LOG("  DEFAULT ERROR = %d\n", attr[i].value.s32);
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    /* 4. call SAI API */
    if (neighbor_api)
    {
        PTF_ERROR_RETURN(neighbor_api->create_neighbor_entry(&neighbor_entry, attr_count, attr));
    }
    else
    {
        PTF_HDLR_LOG("call neighbor_api->create_neighbor_entry()\n");
    }
    
    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_neighbor_entry (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_neighbor_entry_t * thrift_neighbor_entry, GError **error)
{
    sai_neighbor_entry_t neighbor_entry;
    char neighbor_str[128];
    sai_neighbor_api_t *neighbor_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_NEIGHBOR, (void**)&neighbor_api));

    /* 2. convert thrift format to SAI format */
    sal_memset(&neighbor_entry, 0, sizeof(neighbor_entry));
    PTF_ERROR_RETURN(ptf_hdlr_format_neighbor_entry_thrift_to_sai(thrift_neighbor_entry, &neighbor_entry));

    /* 3. debug */
    PTF_HDLR_LOG("##### neighbor API - remove_neighbor_entry() #####\n");
    ptf_hdlr_neighbor_val2str(&neighbor_entry, neighbor_str);
    PTF_HDLR_LOG("  neighbor: %s\n", neighbor_str);

    /* 4. call SAI API */
    if (neighbor_api)
    {
        PTF_ERROR_RETURN(neighbor_api->remove_neighbor_entry(&neighbor_entry));
    }
    else
    {
        PTF_HDLR_LOG("call neighbor_api->remove_neighbor_entry()\n");
    }
    
    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;

    return TRUE;
}

#define __PTF_HANDLER_PROCESS_FUNCTION_SWITCH__

gboolean
ptf_hdlr_sai_thrift_get_switch_attribute (switch_sai_rpcIf *iface, sai_thrift_attribute_list_t ** _return, GError **error)
{
    sai_switch_api_t*  switch_api = NULL;
    uint32_t sai_attr_count = 2;
    sai_object_id_t portid_array[64];
    sai_attribute_t sai_attr_list[2];
    sai_object_id_t portid = 0;
    int switch_port_number = 0;
    int i = 0;
    sai_thrift_attribute_t *attr_thrift_port_number = NULL;
    sai_thrift_attribute_value_t *value_thrift_port_number = NULL;
    sai_thrift_attribute_t *attr_thrift_port_list = NULL;
    sai_thrift_attribute_value_t *value_thrift_port_list = NULL;
    sai_thrift_object_list_t *thrift_port_object_list = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    rc = sai_api_query(SAI_API_SWITCH, (void**)&switch_api);
    if (SAI_STATUS_SUCCESS != rc)
    {
        *_return = NULL; \
        return TRUE; \
    }

    /* 2. debug */
    PTF_HDLR_LOG("##### switch API - get_switch_attribute() #####\n");

    /* 3. convert thrift format to SAI format */
    sai_attr_list[0].id = SAI_SWITCH_ATTR_PORT_NUMBER;
    sai_attr_list[1].id = SAI_SWITCH_ATTR_PORT_LIST;
    sai_attr_list[1].value.objlist.count = 64;
    sai_attr_list[1].value.objlist.list = portid_array;

    /* 4. call SAI API */
    if (switch_api)
    {
        rc = switch_api->get_switch_attribute(sai_attr_count, sai_attr_list);
        if (SAI_STATUS_SUCCESS != rc)
        {
            *_return = NULL; \
            return TRUE; \
        }
    }
    else
    {
        sai_attr_list[0].value.u32 = 24;
        sai_attr_list[1].value.objlist.count = sai_attr_list[0].value.u32;
        for (i = 0; i < sai_attr_list[0].value.u32; i++)
        {
            sai_attr_list[1].value.objlist.list[i] = 0x100000000 + i;
        }
    }
    switch_port_number = sai_attr_list[0].value.u32;

    /* 5. return result */
    /* SAI_SWITCH_ATTR_PORT_LIST */
    thrift_port_object_list = g_object_new (TYPE_SAI_THRIFT_OBJECT_LIST_T, "count", switch_port_number, NULL);
    for (i = 0; i < sai_attr_list[1].value.objlist.count; i++)
    {
        portid = sai_attr_list[1].value.objlist.list[i];
        g_array_append_vals (thrift_port_object_list->object_id_list, &portid, 1);
    }
    value_thrift_port_list = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_VALUE_T, 
        "objlist", thrift_port_object_list, 
        NULL);
    attr_thrift_port_list = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_T, 
        "id", SAI_SWITCH_ATTR_PORT_LIST, 
        "value", value_thrift_port_list, 
        NULL);

    value_thrift_port_number = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_VALUE_T, "u32", switch_port_number, NULL);

    attr_thrift_port_number = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_T, NULL);
    
    /* SAI_SWITCH_ATTR_PORT_NUMBER */
    g_object_set (attr_thrift_port_number, "id", SAI_SWITCH_ATTR_PORT_NUMBER, NULL);
    g_object_set (attr_thrift_port_number, "value", value_thrift_port_number, NULL);

    g_ptr_array_add ((*_return)->attr_list, attr_thrift_port_number);
    g_ptr_array_add ((*_return)->attr_list, attr_thrift_port_list);

    g_object_set ((*_return), "attr_count", 2, NULL);

    PTF_HDLR_LOG("  returned switch_port_number = %u\n", switch_port_number);
    for (i = 0; i < switch_port_number; i++)
    {
        sai_attr_list[1].value.objlist.list[i] = 0x100000000 + i;
        PTF_HDLR_LOG("  returned port_id[%u] = "u64_fmt"\n", i, sai_attr_list[1].value.objlist.list[i]);
    }
    
    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_set_switch_attribute (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_attribute_t * attribute, GError **error)
{
    sai_switch_api_t*  switch_api = NULL;
    char str[32];
    sai_attribute_t sai_attr[1];
    sai_status_t rc = SAI_STATUS_SUCCESS;
    
    PTF_HDLR_LOG("##### switch API - set_switch_attribute() #####\n");

    PTF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH, (void**)&switch_api));
    PTF_ERROR_RETURN(ptf_hdlr_format_switch_attribute_thrift_to_sai(attribute, sai_attr));
    
    switch (attribute->id)
    {
    case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
        PTF_HDLR_LOG("ID: %u, MAC: %s\n", sai_attr->id, ptf_hdlr_mac_val2str(sai_attr->value.mac, str));
        if (switch_api)
        {
            PTF_ERROR_RETURN(switch_api->set_switch_attribute(sai_attr));
        }
        else
        {
            PTF_HDLR_LOG("call switch_api->set_switch_attribute()\n");
        }
        break;

    default:
        return SAI_STATUS_NOT_SUPPORTED;
    }
    
    *_return = 0;
    return TRUE;
}

#define __PTF_HANDLER_PROCESS_FUNCTION_TRAP__

gboolean
ptf_hdlr_sai_thrift_create_hostif (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error)
{
    PTF_HDLR_LOG("##### Trap API - create_hostif() #####\n");
    *_return = 0;
    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_hostif (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t hif_id, GError **error)
{
    PTF_HDLR_LOG("##### Trap API - remove_hostif() #####\n");
    *_return = 0;
    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_create_hostif_trap_group (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error)
{
    PTF_HDLR_LOG("##### Trap API - create_hostif_trap_group() #####\n");
    *_return = 0;
    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_hostif_trap_group (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t trap_group_id, GError **error)
{
    PTF_HDLR_LOG("##### Trap API - remove_hostif_trap_group() #####\n");
    *_return = 0;
    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_create_hostif_trap (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const GPtrArray * thrift_attr_list, GError **error)
{
    PTF_HDLR_LOG("##### Trap API - create_hostif_trap() #####\n");
    *_return = 0;
    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_hostif_trap (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_hostif_trap_id_t trap_id, GError **error)
{
    PTF_HDLR_LOG("##### Trap API - remove_hostif_trap() #####\n");
    *_return = 0;
    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_set_hostif_trap (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_hostif_trap_id_t trap_id, const sai_thrift_attribute_t * thrift_attr, GError **error)
{
    PTF_HDLR_LOG("##### Trap API - set_hostif_trap() #####\n");
    *_return = 0;
    return TRUE;
}

#define __PTF_HANDLER_PROCESS_FUNCTION_ACL__

int32
ptf_hdlr_format_acl_table_attr_list_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai, uint32 *attr_count)
{
    sai_thrift_attribute_t *thrift_attr[48];
    int32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    *attr_count = 0;

    if (thrift->len > 11)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        thrift_attr[i] = g_ptr_array_index(thrift, i);
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        switch (thrift_attr[i]->id)
        {
        case SAI_ACL_TABLE_ATTR_STAGE:
            sai[i].id = SAI_ACL_TABLE_ATTR_STAGE;
            sai[i].value.s32 = thrift_attr[i]->value->s32; /*our policer only support type bytes*/
            break;
        case SAI_ACL_TABLE_ATTR_PRIORITY:
            sai[i].id = SAI_ACL_TABLE_ATTR_PRIORITY;
            sai[i].value.u32 = thrift_attr[i]->value->u32;
            break;
        case SAI_ACL_TABLE_ATTR_SIZE:
            sai[i].id = SAI_ACL_TABLE_ATTR_SIZE;
            sai[i].value.s32 = thrift_attr[i]->value->s32;
            break;
        case SAI_ACL_TABLE_ATTR_FIELD_SRC_IPv6:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_IPv6;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break;
        case SAI_ACL_TABLE_ATTR_FIELD_DST_IPv6:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_DST_IPv6;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break;
        case SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break;
        case SAI_ACL_TABLE_ATTR_FIELD_DST_MAC:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break;
        case SAI_ACL_TABLE_ATTR_FIELD_SRC_IP:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_IP;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break;
        case SAI_ACL_TABLE_ATTR_FIELD_DST_IP:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_DST_IP;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break;
        case SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break;
      case SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break;      
      case SAI_ACL_TABLE_ATTR_FIELD_IN_PORT:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_IN_PORT;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break;      
      case SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break;      
      case SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break;     
      case SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_DSCP:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_DSCP;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_ECN:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_ECN;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_TTL:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_TTL;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_TOS:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_TOS;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_IP_TYPE:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_IP_TYPE;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_IP_FRAG:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_IP_FRAG;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_IPv6_FLOW_LABEL:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_IPv6_FLOW_LABEL;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_TC:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_TC;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_USER_META:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_USER_META;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_ROUTE_DST_USER_META:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_ROUTE_DST_USER_META;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_USER_META:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_USER_META;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_PORT_USER_META:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_PORT_USER_META;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_VLAN_USER_META:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_VLAN_USER_META;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_ACL_USER_META:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_ACL_USER_META;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_NPU_META_HIT:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_NPU_META_HIT;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
      case SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_NPU_META_HIT:
            sai[i].id = SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_NPU_META_HIT;
            sai[i].value.booldata= thrift_attr[i]->value->booldata;
            break; 
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    *attr_count = thrift->len;
    return rc;
}

int32
ptf_hdlr_format_acl_entry_attr_list_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai, uint32 *attr_count)
{
    sai_thrift_attribute_t *thrift_attr[48];
    int32 i = 0, j = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;
    static sai_object_id_t port_id_list[PTF_MAX_PORT_NUM];


    *attr_count = 0;

    if (thrift->len > 11)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        thrift_attr[i] = g_ptr_array_index(thrift, i);
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        switch (thrift_attr[i]->id)
        {
        case SAI_ACL_ENTRY_ATTR_TABLE_ID:
            sai[i].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
            sai[i].value.aclfield.data.oid = thrift_attr[i]->value->aclfield->data->oid; 
            break;
        case SAI_ACL_ENTRY_ATTR_PACKET_ACTION:
            sai[i].value.aclaction.enable = 1;
            sai[i].id = SAI_ACL_ENTRY_ATTR_PACKET_ACTION;
            sai[i].value.aclaction.parameter.u8= thrift_attr[i]->value->aclfield->data->u8;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP:
            sai[i].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP;
            sai_thrift_string_to_v4_ip(thrift_attr[i]->value->aclfield->data->ip4, &sai[i].value.aclfield.data.ip4);
            sai_thrift_string_to_v4_ip(thrift_attr[i]->value->aclfield->mask->ip4, &sai[i].value.aclfield.mask.ip4);
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_IP:
            sai[i].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IP;
            sai_thrift_string_to_v4_ip(thrift_attr[i]->value->aclfield->data->ip4, &sai[i].value.aclfield.data.ip4);
            sai_thrift_string_to_v4_ip(thrift_attr[i]->value->aclfield->mask->ip4, &sai[i].value.aclfield.mask.ip4);
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS:
            sai[i].value.aclfield.data.objlist.list = port_id_list;
            sai[i].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
            
            for (j = 0; j < thrift_attr[i]->value->aclfield->data->objlist->count; j++)
            {
                sai[i].value.aclfield.data.objlist.list[j]= g_array_index (thrift_attr[i]->value->aclfield->data->objlist->object_id_list, gint64, j);
            }
            sai[i].value.aclfield.data.objlist.count = thrift_attr[i]->value->aclfield->data->objlist->count;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS:
            sai[i].id = SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS;
            sai[i].value.aclfield.data.objlist.list = port_id_list;

            for (j = 0; j < thrift_attr[i]->value->aclfield->data->objlist->count; j++)
            {
                sai[i].value.aclfield.data.objlist.list[j]= g_array_index (thrift_attr[i]->value->aclfield->data->objlist->object_id_list, gint64, j);
            }
            sai[i].value.aclfield.data.objlist.count = thrift_attr[i]->value->aclfield->data->objlist->count;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT:
            sai[i].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
            sai[i].value.aclfield.data.oid = thrift_attr[i]->value->aclfield->data->oid;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT:
            sai[i].id = SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT;
            sai[i].value.aclfield.data.oid = thrift_attr[i]->value->aclfield->data->oid;
            break;
        case SAI_ACL_ENTRY_ATTR_PRIORITY:
            sai[i].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
            sai[i].value.aclfield.data.u32 = thrift_attr[i]->value->aclfield->data->u32;
            break;
        case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS:
            sai[i].value.aclaction.enable = 1;
            sai[i].id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS;
            sai[i].value.aclaction.parameter.oid = thrift_attr[i]->value->aclfield->data->oid;
            break;
      case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS:
            sai[i].value.aclaction.enable = 1;
            sai[i].id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS;
            sai[i].value.aclaction.parameter.oid = thrift_attr[i]->value->aclfield->data->oid;
            break;      
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    *attr_count = thrift->len;
    return rc;
}

int32
ptf_hdlr_format_acl_entry_counter_attr_list_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai, uint32 *attr_count)
{
    sai_thrift_attribute_t *thrift_attr[1];
    int32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    *attr_count = 0;

    if (thrift->len > 11)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        thrift_attr[i] = g_ptr_array_index(thrift, i);
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        switch (thrift_attr[i]->id)
        {
        case SAI_ACL_COUNTER_ATTR_TABLE_ID:
            sai[i].id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
            sai[i].value.oid = thrift_attr[i]->value->oid; 
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    *attr_count = thrift->len;
    return rc;
}

gboolean
ptf_hdlr_sai_thrift_create_acl_table (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error)
{
    PTF_HDLR_LOG("##### ACL API - create_acl_table() #####\n");

    sai_object_id_t acl_table_id = 0;
    sai_acl_api_t*  acl_api;
    sai_attribute_t attr[16] = {{0}};
    uint32_t attr_count = 0;
    uint32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    PTF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));

    /* 2. convert thrift format to SAI format */
    PTF_ERROR_RETURN(ptf_hdlr_format_acl_table_attr_list_thrift_to_sai(thrift_attr_list, attr, &attr_count)); 

    for (i = 0; i < attr_count; i++)
    {
        switch (attr[i].id)
        {
        case SAI_ACL_TABLE_ATTR_FIELD_SRC_IP:
            PTF_HDLR_LOG("  ip src: %d\n", attr[i].value.booldata);
            break;
        case SAI_ACL_TABLE_ATTR_FIELD_DST_IP:
            PTF_HDLR_LOG("  ip dest: %d\n", attr[i].value.booldata);
            break;
        case SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL:
            PTF_HDLR_LOG("  ip protocol: %d\n", attr[i].value.booldata);
            break;
        case SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS:
            PTF_HDLR_LOG("  in ports: %d\n", attr[i].value.booldata);
            break;
        case SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS:
            PTF_HDLR_LOG("  out ports: %d\n", attr[i].value.booldata);
            break;
        case SAI_ACL_TABLE_ATTR_FIELD_IN_PORT:
            PTF_HDLR_LOG("  in port: %d\n", attr[i].value.booldata);
            break;
        case SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT:
            PTF_HDLR_LOG("  out port: %d\n", attr[i].value.booldata);
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    if(acl_api)
    {
        PTF_ERROR_RETURN(acl_api->create_acl_table(&acl_table_id, attr_count, attr));
        PTF_HDLR_LOG("call acl_api->create_acl_table(), acl_table_id %llx\n", acl_table_id);
    }
    else
    {
        PTF_HDLR_LOG("call acl_api->create_acl_table()\n");
        acl_table_id = 0x1000000000000002;
    }

    *_return = acl_table_id;
    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_delete_acl_table (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t acl_table_id, GError **error)
{
    PTF_HDLR_LOG("##### ACL API - delete_acl_table() #####\n");

    sai_acl_api_t*  acl_api;
    sai_object_id_t table_id;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    PTF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    table_id = acl_table_id;
    PTF_HDLR_LOG("  table id: %llx\n", table_id);

    if(acl_api)
    {
        PTF_ERROR_RETURN(acl_api->delete_acl_table(table_id));
    }
    else
    {
        PTF_HDLR_LOG("call acl_api->delete_acl_table()\n");
    }

    *_return = 0;
    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_create_acl_entry (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error)
{
    PTF_HDLR_LOG("##### ACL API - create_acl_entry() #####\n");
    sai_acl_api_t*  acl_api;
    uint32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;
    char ip_addr_str[128];
    sai_attribute_t attr[32] = {{0}};
    sai_object_id_t acl_entry_id = 0;
    uint32_t attr_count = 0;

    PTF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    /* 2. convert thrift format to SAI format */
    PTF_ERROR_RETURN(ptf_hdlr_format_acl_entry_attr_list_thrift_to_sai(thrift_attr_list, attr, &attr_count)); 

    for (i = 0; i < attr_count; i++)
    {
        switch (attr[i].id)
        {
        case SAI_ACL_ENTRY_ATTR_TABLE_ID:
            PTF_HDLR_LOG("  table id: %llx\n", attr[i].value.aclfield.data.oid);
            break;
        case SAI_ACL_ENTRY_ATTR_PACKET_ACTION:
            PTF_HDLR_LOG("  packet action: %d\n", attr[i].value.aclaction.parameter.u8);
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP:
            ptf_hdlr_ip_address_val2str_by_acldata(&attr[i].value.aclfield.data.ip4, ip_addr_str);
            PTF_HDLR_LOG("  scr ip = %s\n", ip_addr_str);
            ptf_hdlr_ip_address_val2str_by_acldata(&attr[i].value.aclfield.mask.ip4, ip_addr_str);
            PTF_HDLR_LOG("  scr ip mask= %s\n", ip_addr_str);
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_IP:
            ptf_hdlr_ip_address_val2str_by_acldata(&attr[i].value.aclfield.data.ip4, ip_addr_str);
            PTF_HDLR_LOG("  dst ip = %s\n", ip_addr_str);
            ptf_hdlr_ip_address_val2str_by_acldata(&attr[i].value.aclfield.mask.ip4, ip_addr_str);
            PTF_HDLR_LOG("  dst ip mask= %s\n", ip_addr_str);
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS:
            PTF_HDLR_LOG("  in ports \n");
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS:
            PTF_HDLR_LOG("  out ports \n");
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT:
            PTF_HDLR_LOG("  in port: %llx\n", attr[i].value.aclfield.data.oid);
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT:
            PTF_HDLR_LOG("  out port: %llx\n", attr[i].value.aclfield.data.oid);
            break;
        case SAI_ACL_ENTRY_ATTR_PRIORITY:
            PTF_HDLR_LOG("  priority: %d\n", attr[i].value.aclfield.data.u32);
            break;
        case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS:
            PTF_HDLR_LOG("  ingress mirror: %llx\n", attr[i].value.aclaction.parameter.oid);
            break;
        case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS:
            PTF_HDLR_LOG("  egress mirror: %llx\n", attr[i].value.aclaction.parameter.oid);
            break;      
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    if(acl_api)
    {
        PTF_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr));
        PTF_HDLR_LOG("call acl_api->create_acl_entry() acl_entry_id %llx\n", acl_entry_id);
    }
    else
    {
        PTF_HDLR_LOG("call acl_api->create_acl_entry()\n");
        acl_entry_id = 0x1000000000000003;
    }
    *_return = acl_entry_id;
    return TRUE;
}

gboolean ptf_hdlr_sai_thrift_delete_acl_entry (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t acl_entry, GError **error)
{
    PTF_HDLR_LOG("##### ACL API - delete_acl_entry() #####\n");
    sai_status_t rc = SAI_STATUS_SUCCESS;
    sai_acl_api_t *acl_api = NULL;
    sai_object_id_t acl_entry_id;
    
    PTF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    acl_entry_id = acl_entry;
    
    PTF_HDLR_LOG("  acl_entry_id: %llx\n", acl_entry_id);
    if(acl_api)
    {
        PTF_ERROR_RETURN(acl_api->delete_acl_entry(acl_entry_id));
    }
    else
    {
        PTF_HDLR_LOG("call acl_api->delete_acl_entry()\n");
    }

    *_return = 0;
    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_create_acl_counter (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error)
{
    PTF_HDLR_LOG("##### ACL API - create_acl_counter() #####\n");
    sai_acl_api_t*  acl_api;
    uint32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr[1] = {{0}};
    uint32_t attr_count = 0;
    sai_object_id_t acl_counter_id;
    
    PTF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    /* 2. convert thrift format to SAI format */
    PTF_ERROR_RETURN(ptf_hdlr_format_acl_entry_counter_attr_list_thrift_to_sai(thrift_attr_list, attr, &attr_count)); 

    for (i = 0; i < attr_count; i++)
    {
        switch (attr[i].id)
        {
        case SAI_ACL_COUNTER_ATTR_TABLE_ID:
                PTF_HDLR_LOG("  acl_table_id: %lu\n", attr[i].value.oid);
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    
    if(acl_api)
    {
        PTF_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, attr));
    }
    else
    {
        PTF_HDLR_LOG("call acl_api->create_acl_counter()\n");
        acl_counter_id = 0x1000000000000003;
    }
    *_return = acl_counter_id;
    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_delete_acl_counter (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t acl_counter_id, GError **error)
{
    PTF_HDLR_LOG("##### ACL API - delete_acl_counter() #####\n");
    sai_status_t rc = SAI_STATUS_SUCCESS;
    sai_acl_api_t *acl_api = NULL;
    sai_object_id_t sai_acl_counter_id;
    
    PTF_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));
    sai_acl_counter_id = acl_counter_id;
    
    PTF_HDLR_LOG("  acl_counter_id: %lu\n", sai_acl_counter_id);
    if(acl_api)
    {
        PTF_ERROR_RETURN(acl_api->delete_acl_counter(sai_acl_counter_id));
    }
    else
    {
        PTF_HDLR_LOG("call acl_api->delete_acl_counter()\n");
    }

    *_return = 0;
    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_get_acl_counter_attribute (switch_sai_rpcIf *iface, GPtrArray ** _return, const sai_thrift_object_id_t acl_counter_id, const GArray * thrift_attr_ids, GError **error)
{
    PTF_HDLR_LOG("##### ACL API - get_acl_counter_attribute() #####\n");

    sai_status_t rc = SAI_STATUS_SUCCESS;
    sai_acl_api_t *acl_api = NULL;
    sai_object_id_t sai_acl_counter_id;
    sai_attribute_t attr[2];
    sai_thrift_attribute_value_t *return_values[2];
    uint32 i =0;
    
    rc = sai_api_query(SAI_API_ACL,(void**)&acl_api);
    if(rc < 0)
    {
        return TRUE;
    }

    for (i = 0; i < 2; i++)
    {
        attr[i].id = g_array_index (thrift_attr_ids, gint32, i);
    }
    sai_acl_counter_id = acl_counter_id;
    PTF_HDLR_LOG("  acl_counter_id: %lu\n", sai_acl_counter_id);
    for (i = 0; i < 2; i++)
    {
        switch (attr[i].id)
        {
        case SAI_ACL_COUNTER_ATTR_BYTES:
            PTF_HDLR_LOG("get acl counter attr bytes\n");
            break;
        case SAI_ACL_COUNTER_ATTR_PACKETS:
            PTF_HDLR_LOG("get acl counter attr packets\n");
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    if(acl_api)
    {
        acl_api->get_acl_counter_attribute(sai_acl_counter_id, 2, attr);
    }
    else
    {
        PTF_HDLR_LOG("call acl_api->get_acl_counter_attribute()\n");
        for (i = 0; i < 2; i++)
        {
            attr[i].value.u64 = 0x300000000 + 100 * i;
        }
    }
    
    for (i = 0; i < 2; i++)
    {
        switch (attr[i].id)
        {
        case SAI_ACL_COUNTER_ATTR_TABLE_ID:
            return_values[i] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_VALUE_T, "oid", attr[i].value.oid, NULL);
            break;
        case SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT:
        case SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT:
            return_values[i] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_VALUE_T, "booldata", attr[i].value.booldata, NULL);
            break;
        case SAI_ACL_COUNTER_ATTR_BYTES:
        case SAI_ACL_COUNTER_ATTR_PACKETS:
            return_values[i] = g_object_new (TYPE_SAI_THRIFT_ATTRIBUTE_VALUE_T, "u64", attr[i].value.u64, NULL);
            break;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }

        g_ptr_array_add (*_return, return_values[i]);
    }

    return TRUE;
}

#define __PTF_HANDLER_PROCESS_FUNCTION_MIRROR__

int32
ptf_hdlr_format_create_mirror_session_attr_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai, uint32 *attr_count)
{
    sai_thrift_attribute_t *thrift_attr = NULL;
    int32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    for (i = 0; i < thrift->len; i++)
    {
        thrift_attr = g_ptr_array_index(thrift, i);
        
        sai[i].id = thrift_attr->id;
        switch (thrift_attr->id)
        {
        case SAI_MIRROR_SESSION_ATTR_TYPE:
            sai[i].value.s32 = thrift_attr->value->s32;
            break;
        case SAI_MIRROR_SESSION_ATTR_MONITOR_PORT:
            sai[i].value.oid = thrift_attr->value->oid;
            break;
        case SAI_MIRROR_SESSION_ATTR_TC:
            sai[i].value.u8 = thrift_attr->value->u8;
            break;
        case SAI_MIRROR_SESSION_ATTR_VLAN_TPID:
            sai[i].value.u16 = thrift_attr->value->u16;
            break;
        case SAI_MIRROR_SESSION_ATTR_VLAN_ID:
            sai[i].value.u16 = thrift_attr->value->u16;
            break;
        case SAI_MIRROR_SESSION_ATTR_VLAN_PRI:
            sai[i].value.u8 = thrift_attr->value->u8;
            break;
        case SAI_MIRROR_SESSION_ATTR_ENCAP_TYPE:
            sai[i].value.s32 = thrift_attr->value->s32;
            break;
        case SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION:
            sai[i].value.u8 = thrift_attr->value->u8;
            break;
        case SAI_MIRROR_SESSION_ATTR_TOS:
            sai[i].value.u8 = thrift_attr->value->u8;
            break;
        case SAI_MIRROR_SESSION_ATTR_TTL:
            sai[i].value.u8 = thrift_attr->value->u8;
            break;
        case SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS:
            ptf_hdlr_format_ip_address_thrift_to_sai(thrift_attr->value->ipaddr, &sai[i].value.ipaddr);
            break;
        case SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS:
            ptf_hdlr_format_ip_address_thrift_to_sai(thrift_attr->value->ipaddr, &sai[i].value.ipaddr);
            break;
        case SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS:
            ptf_hdlr_format_mac_thrift_string_to_sai(thrift_attr->value->mac, sai[i].value.mac);
            break;
        case SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS:
            ptf_hdlr_format_mac_thrift_string_to_sai(thrift_attr->value->mac, sai[i].value.mac);
            break;
        case SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE:
            sai[i].value.u16 = thrift_attr->value->u16;
            break;

        default:
            break;
        }
    }
    *attr_count = thrift->len;
    
    return rc;
}

gboolean
ptf_hdlr_sai_thrift_create_mirror_session (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error)
{
    sai_attribute_t attr[64];
    sai_object_id_t session_id = 0;
    sai_mirror_api_t *mirror_api = NULL;
    uint32 i = 0;
    uint32 attr_count = 0;
    char mac_str[128];
    char ip_addr_str[128];
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_MIRROR, (void**)&mirror_api));

    /* 2. convert thrift format to SAI format */
    PTF_ERROR_RETURN(ptf_hdlr_format_create_mirror_session_attr_thrift_to_sai(thrift_attr_list, attr, &attr_count));
    
    /* 3. debug */
    PTF_HDLR_LOG("##### Mirror API - create_mirror_session() #####\n");

    PTF_HDLR_LOG("  attr_count = %u\n", attr_count);
    for (i = 0; i < attr_count; i++)
    {
        switch (attr[i].id)
        {
        case SAI_MIRROR_SESSION_ATTR_TYPE:
            PTF_HDLR_LOG("  SAI_MIRROR_SESSION_ATTR_TYPE: %u\n", attr[i].value.u8);
            break;
        case SAI_MIRROR_SESSION_ATTR_MONITOR_PORT:
            PTF_HDLR_LOG("  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT: "u64_fmt"\n", attr[i].value.oid);
            break;
        case SAI_MIRROR_SESSION_ATTR_TC:
            PTF_HDLR_LOG("  SAI_MIRROR_SESSION_ATTR_TC: %u\n", attr[i].value.u8);
            break;
        case SAI_MIRROR_SESSION_ATTR_VLAN_TPID:
            PTF_HDLR_LOG("  SAI_MIRROR_SESSION_ATTR_VLAN_TPID: %u\n", attr[i].value.u16);
            break;
        case SAI_MIRROR_SESSION_ATTR_VLAN_ID:
            PTF_HDLR_LOG("  SAI_MIRROR_SESSION_ATTR_VLAN_ID: %u\n", attr[i].value.u16);
            break;
        case SAI_MIRROR_SESSION_ATTR_VLAN_PRI:
            PTF_HDLR_LOG("  SAI_MIRROR_SESSION_ATTR_VLAN_PRI: %u\n", attr[i].value.u8);
            break;
        case SAI_MIRROR_SESSION_ATTR_ENCAP_TYPE:
            PTF_HDLR_LOG("  SAI_MIRROR_SESSION_ATTR_ENCAP_TYPE: %u\n", attr[i].value.u8);
            break;
        case SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION:
            PTF_HDLR_LOG("  SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION: %u\n", attr[i].value.u8);
            break;
        case SAI_MIRROR_SESSION_ATTR_TOS:
            PTF_HDLR_LOG("  SAI_MIRROR_SESSION_ATTR_TOS: %u\n", attr[i].value.u8);
            break;
        case SAI_MIRROR_SESSION_ATTR_TTL:
            PTF_HDLR_LOG("  SAI_MIRROR_SESSION_ATTR_TTL: %u\n", attr[i].value.u8);
            break;
        case SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS:
            ptf_hdlr_ip_address_val2str(&attr[i].value.ipaddr, ip_addr_str);
            PTF_HDLR_LOG("  SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS %s\n", ip_addr_str);
            break;
        case SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS:
            ptf_hdlr_ip_address_val2str(&attr[i].value.ipaddr, ip_addr_str);
            PTF_HDLR_LOG("  SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS %s\n", ip_addr_str);
            break;
        case SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS:
            ptf_hdlr_mac_val2str(attr[i].value.mac, mac_str);
            PTF_HDLR_LOG("  SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS MAC = %s\n", mac_str);
            break;
        case SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS:
            ptf_hdlr_mac_val2str(attr[i].value.mac, mac_str);
            PTF_HDLR_LOG("  SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS MAC = %s\n", mac_str);
            break;
        case SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE:
            PTF_HDLR_LOG("  SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE: %u\n", attr[i].value.u16);
            break;

        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    
    /* 4. call SAI API */
    if (mirror_api)
    {
        PTF_ERROR_RETURN(mirror_api->create_mirror_session(&session_id, attr_count, attr));
    }
    else
    {
        PTF_HDLR_LOG("call mirror_api->create_mirror_session()\n");
        session_id = 10;
    }

    /* 5. return result */
    *_return = session_id;
    PTF_HDLR_LOG("  returned session_id = "u64_fmt"\n", session_id);

    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_mirror_session (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t session_id, GError **error)
{
    sai_mirror_api_t *mirror_api = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    PTF_ERROR_RETURN(sai_api_query(SAI_API_MIRROR, (void**)&mirror_api));

    /* 2. convert thrift format to SAI format */
    
    /* 3. debug */
    PTF_HDLR_LOG("##### Mirror API - remove_mirror_session(session_id="u64_fmt") #####\n", session_id);
    
    /* 4. call SAI API */
    if (mirror_api)
    {
        PTF_ERROR_RETURN(mirror_api->remove_mirror_session(session_id));
    }
    else
    {
        PTF_HDLR_LOG("call mirror_api->remove_mirror_session()\n");
    }

    /* 5. return result */
    *_return = SAI_STATUS_SUCCESS;
    
    return TRUE;
}

#define __PTF_HANDLER_PROCESS_FUNCTION_POLICER__

int32
ptf_hdlr_format_policer_entry_attr_list_thrift_to_sai(const GPtrArray *thrift, sai_attribute_t *sai, uint32 *attr_count)
{
    sai_thrift_attribute_t *thrift_attr[11];
    int32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    *attr_count = 0;

    if (thrift->len > 11)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        thrift_attr[i] = g_ptr_array_index(thrift, i);
    }
    
    for (i = 0; i < thrift->len; i++)
    {
        switch (thrift_attr[i]->id)
        {
        case SAI_POLICER_ATTR_METER_TYPE:
            sai[i].id = SAI_POLICER_ATTR_METER_TYPE;
            sai[i].value.s32 = SAI_METER_TYPE_BYTES; /*our policer only support type bytes*/
            break;
        case SAI_POLICER_ATTR_MODE:
            sai[i].id = SAI_POLICER_ATTR_MODE;
            sai[i].value.s32 = thrift_attr[i]->value->s32;
            break;
        case SAI_POLICER_ATTR_COLOR_SOURCE:
            sai[i].id = SAI_POLICER_ATTR_COLOR_SOURCE;
            sai[i].value.s32 = thrift_attr[i]->value->s32;
            break;
        case SAI_POLICER_ATTR_CBS:
            sai[i].id = SAI_POLICER_ATTR_CBS;
            sai[i].value.u64= thrift_attr[i]->value->u64;
            break;
        case SAI_POLICER_ATTR_CIR:
            sai[i].id = SAI_POLICER_ATTR_CIR;
            sai[i].value.u64= thrift_attr[i]->value->u64;
            break;
        case SAI_POLICER_ATTR_PBS:
            sai[i].id = SAI_POLICER_ATTR_PBS;
            sai[i].value.u64= thrift_attr[i]->value->u64;
            break;
        case SAI_POLICER_ATTR_PIR:
            sai[i].id = SAI_POLICER_ATTR_PIR;
            sai[i].value.u64= thrift_attr[i]->value->u64;
            break;
        case SAI_POLICER_ATTR_GREEN_PACKET_ACTION:
            sai[i].id = SAI_POLICER_ATTR_GREEN_PACKET_ACTION;
            sai[i].value.s32= thrift_attr[i]->value->s32;
            break;
        case SAI_POLICER_ATTR_YELLOW_PACKET_ACTION:
            sai[i].id = SAI_POLICER_ATTR_YELLOW_PACKET_ACTION;
            sai[i].value.s32= thrift_attr[i]->value->s32;
            break;
        case SAI_POLICER_ATTR_RED_PACKET_ACTION:
            sai[i].id = SAI_POLICER_ATTR_RED_PACKET_ACTION;
            sai[i].value.s32= thrift_attr[i]->value->s32;
            break;
      case SAI_POLICER_ATTR_ENABLE_COUNTER_LIST:
            sai[i].id = SAI_POLICER_ATTR_ENABLE_COUNTER_LIST;
            sai[i].value.s32= thrift_attr[i]->value->s32;
            break;      
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    *attr_count = thrift->len;
    return rc;
}


gboolean
ptf_hdlr_sai_thrift_create_policer (switch_sai_rpcIf *iface, sai_thrift_object_id_t* _return, const GPtrArray * thrift_attr_list, GError **error)
{
    PTF_HDLR_LOG("##### policer API - create_policer() #####\n");
    sai_object_id_t policer_id = 0;
    sai_policer_api_t*  policer_api;
    sai_attribute_t attr[11] = {{0}};
    uint32_t attr_count = 0;
    uint32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    PTF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));

    /* 2. convert thrift format to SAI format */
    PTF_ERROR_RETURN(ptf_hdlr_format_policer_entry_attr_list_thrift_to_sai(thrift_attr_list, attr, &attr_count)); 

    for (i = 0; i < attr_count; i++)
    {
        switch (attr[i].id)
        {
        case SAI_POLICER_ATTR_METER_TYPE:
            PTF_HDLR_LOG("  type: %u\n", attr[i].value.s32);
            break;
        case SAI_POLICER_ATTR_MODE:
            PTF_HDLR_LOG("  mode: %u\n", attr[i].value.s32);
            break;
        case SAI_POLICER_ATTR_COLOR_SOURCE:
            PTF_HDLR_LOG("  color source: %u\n", attr[i].value.s32);
            break;
        case SAI_POLICER_ATTR_CBS:
            PTF_HDLR_LOG("  cbs: %llu\n", attr[i].value.u64);
            break;
        case SAI_POLICER_ATTR_CIR:
            PTF_HDLR_LOG("  cir: %llu\n", attr[i].value.u64);
            break;
        case SAI_POLICER_ATTR_PBS:
            PTF_HDLR_LOG("  pbs: %llu\n", attr[i].value.u64);
            break;
        case SAI_POLICER_ATTR_PIR:
            PTF_HDLR_LOG("  pir: %llu\n", attr[i].value.u64);
            break;
        case SAI_POLICER_ATTR_GREEN_PACKET_ACTION:
            PTF_HDLR_LOG("  green packet action: %u\n", attr[i].value.s32);
            break;
        case SAI_POLICER_ATTR_YELLOW_PACKET_ACTION:
            PTF_HDLR_LOG("  yellow packet action: %u\n", attr[i].value.s32);
            break;
        case SAI_POLICER_ATTR_RED_PACKET_ACTION:
            PTF_HDLR_LOG("  red packet action: %u\n", attr[i].value.s32);
            break;
      case SAI_POLICER_ATTR_ENABLE_COUNTER_LIST:
            PTF_HDLR_LOG("  enable counter list: %u\n", attr[i].value.s32);
            break;      
        default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    if(policer_api)
    {
        PTF_ERROR_RETURN(policer_api->create_policer(&policer_id,attr_count,attr));
    }
    else
    {
        PTF_HDLR_LOG("call policer_api->create_policer()\n");
    }

    *_return = policer_id;

     return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_remove_policer (switch_sai_rpcIf *iface, sai_thrift_status_t* _return, const sai_thrift_object_id_t policer_id, GError **error)
{
    PTF_HDLR_LOG("##### policer API - remove_policer() #####\n");
    sai_object_id_t policer_oid;
    sai_policer_api_t*  policer_api;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    policer_oid = policer_id;

    PTF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));

    PTF_HDLR_LOG("  policer_id: %llu\n", policer_id);
    if(policer_api)
    {
        PTF_ERROR_RETURN(policer_api->remove_policer(policer_oid));
    }
    else
    {
        PTF_HDLR_LOG("call policer_api->remove_policer()\n");
    }
    
    *_return = 0;
    return TRUE;
}

gboolean
ptf_hdlr_sai_thrift_get_policer_stats (switch_sai_rpcIf *iface, GArray ** _return, const sai_thrift_object_id_t policer_id, const GArray * counter_ids, GError **error)
{
    PTF_HDLR_LOG("##### policer API - get_policer_stats() #####\n");

    sai_policer_api_t *policer_api = NULL;
    sai_policer_stat_counter_t sai_counter_ids[8];
    uint64_t counters[8];
    sai_object_id_t sai_policer_id;
    uint32 i = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* 1. query SAI API */
    rc = (sai_api_query(SAI_API_POLICER, (void**)&policer_api));
    if (rc < 0)
    {
        return TRUE;
    }

    /* 2. convert thrift format to SAI format */
    sal_memset(counters, 0, sizeof(counters));
    sai_policer_id = policer_id;

    /* 3. debug */
    PTF_HDLR_LOG("##### vlan API - get_policer_stats(policer_id=%u) #####\n", sai_policer_id);
    for (i = 0; i < 8; i++)
    {
        sai_counter_ids[i] = g_array_index (counter_ids, gint32, i);
        PTF_HDLR_LOG("  counter_ids[%u] = %d\n", i, sai_counter_ids[i]);
    }
    
    /* 4. call SAI API */
    if (policer_api)
    {
        rc = (policer_api->get_policer_statistics(sai_policer_id, sai_counter_ids, 8, counters));
        if (rc < 0)
        {
            return TRUE;
        }
    }
    else
    {
        PTF_HDLR_LOG("call policer_api->get_policer_statistics()\n");
        for (i = 0; i < 8; i++)
        {
            counters[i] = 100 * i;
        }
    }

    for (i = 0; i < 8; i++)
    {
        g_array_append_val (*_return, counters[i]);
    }
    
    return TRUE;
}

static void
ctc_sai_rpc_handler_init (ctc_sai_rpcHandler *self)
{
    THRIFT_UNUSED_VAR (self);
}

static void
ctc_sai_rpc_handler_class_init (ctc_sai_rpcHandlerClass *klass)
{
    switch_sai_rpcHandlerClass *base_class = SWITCH_SAI_RPC_HANDLER_CLASS (klass);

    //port API
    base_class->sai_thrift_set_port_attribute = klass->sai_thrift_set_port_attribute = ptf_hdlr_sai_thrift_set_port_attribute;

    //fdb API
    base_class->sai_thrift_create_fdb_entry = klass->sai_thrift_create_fdb_entry = ptf_hdlr_sai_thrift_create_fdb_entry;
    base_class->sai_thrift_delete_fdb_entry = klass->sai_thrift_delete_fdb_entry = ptf_hdlr_sai_thrift_delete_fdb_entry;
    base_class->sai_thrift_flush_fdb_entries = klass->sai_thrift_flush_fdb_entries = ptf_hdlr_sai_thrift_flush_fdb_entries;

    //vlan API
    base_class->sai_thrift_create_vlan = klass->sai_thrift_create_vlan = ptf_hdlr_sai_thrift_create_vlan;
    base_class->sai_thrift_delete_vlan = klass->sai_thrift_delete_vlan = ptf_hdlr_sai_thrift_delete_vlan;
    base_class->sai_thrift_add_ports_to_vlan = klass->sai_thrift_add_ports_to_vlan = ptf_hdlr_sai_thrift_add_ports_to_vlan;
    base_class->sai_thrift_remove_ports_from_vlan = klass->sai_thrift_remove_ports_from_vlan = ptf_hdlr_sai_thrift_remove_ports_from_vlan;
    base_class->sai_thrift_get_vlan_stats = klass->sai_thrift_get_vlan_stats = ptf_hdlr_sai_thrift_get_vlan_stats;

    //virtual router API
    base_class->sai_thrift_create_virtual_router = klass->sai_thrift_create_virtual_router = ptf_hdlr_sai_thrift_create_virtual_router;
    base_class->sai_thrift_remove_virtual_router = klass->sai_thrift_remove_virtual_router = ptf_hdlr_sai_thrift_remove_virtual_router;

    //route API
    base_class->sai_thrift_create_route = klass->sai_thrift_create_route = ptf_hdlr_sai_thrift_create_route;
    base_class->sai_thrift_remove_route = klass->sai_thrift_remove_route = ptf_hdlr_sai_thrift_remove_route;

    //router interface API
    base_class->sai_thrift_create_router_interface = klass->sai_thrift_create_router_interface = ptf_hdlr_sai_thrift_create_router_interface;
    base_class->sai_thrift_remove_router_interface = klass->sai_thrift_remove_router_interface = ptf_hdlr_sai_thrift_remove_router_interface;

    //next hop API
    base_class->sai_thrift_create_next_hop = klass->sai_thrift_create_next_hop = ptf_hdlr_sai_thrift_create_next_hop;
    base_class->sai_thrift_remove_next_hop = klass->sai_thrift_remove_next_hop = ptf_hdlr_sai_thrift_remove_next_hop;

    //next hop group API
    base_class->sai_thrift_create_next_hop_group = klass->sai_thrift_create_next_hop_group = ptf_hdlr_sai_thrift_create_next_hop_group;
    base_class->sai_thrift_remove_next_hop_group = klass->sai_thrift_remove_next_hop_group = ptf_hdlr_sai_thrift_remove_next_hop_group;
    base_class->sai_thrift_add_next_hop_to_group = klass->sai_thrift_add_next_hop_to_group = ptf_hdlr_sai_thrift_add_next_hop_to_group;
    base_class->sai_thrift_remove_next_hop_from_group = klass->sai_thrift_remove_next_hop_from_group = ptf_hdlr_sai_thrift_remove_next_hop_from_group;

    //lag API
    base_class->sai_thrift_create_lag = klass->sai_thrift_create_lag = ptf_hdlr_sai_thrift_create_lag;
    base_class->sai_thrift_remove_lag = klass->sai_thrift_remove_lag = ptf_hdlr_sai_thrift_remove_lag;
    base_class->sai_thrift_create_lag_member = klass->sai_thrift_create_lag_member = ptf_hdlr_sai_thrift_create_lag_member;
    base_class->sai_thrift_remove_lag_member = klass->sai_thrift_remove_lag_member = ptf_hdlr_sai_thrift_remove_lag_member;

    //stp API
    base_class->sai_thrift_create_stp_entry = klass->sai_thrift_create_stp_entry = ptf_hdlr_sai_thrift_create_stp_entry;
    base_class->sai_thrift_remove_stp_entry = klass->sai_thrift_remove_stp_entry = ptf_hdlr_sai_thrift_remove_stp_entry;
    base_class->sai_thrift_set_stp_port_state = klass->sai_thrift_set_stp_port_state = ptf_hdlr_sai_thrift_set_stp_port_state;
    base_class->sai_thrift_get_stp_port_state = klass->sai_thrift_get_stp_port_state = ptf_hdlr_sai_thrift_get_stp_port_state;

    //neighbor API
    base_class->sai_thrift_create_neighbor_entry = klass->sai_thrift_create_neighbor_entry = ptf_hdlr_sai_thrift_create_neighbor_entry;
    base_class->sai_thrift_remove_neighbor_entry = klass->sai_thrift_remove_neighbor_entry = ptf_hdlr_sai_thrift_remove_neighbor_entry;

    //switch API
    base_class->sai_thrift_get_switch_attribute = klass->sai_thrift_get_switch_attribute = ptf_hdlr_sai_thrift_get_switch_attribute;
    base_class->sai_thrift_set_switch_attribute = klass->sai_thrift_set_switch_attribute = ptf_hdlr_sai_thrift_set_switch_attribute;

    //Trap API
    base_class->sai_thrift_create_hostif = klass->sai_thrift_create_hostif = ptf_hdlr_sai_thrift_create_hostif;
    base_class->sai_thrift_remove_hostif = klass->sai_thrift_remove_hostif = ptf_hdlr_sai_thrift_remove_hostif;
    base_class->sai_thrift_create_hostif_trap_group = klass->sai_thrift_create_hostif_trap_group = ptf_hdlr_sai_thrift_create_hostif_trap_group;
    base_class->sai_thrift_remove_hostif_trap_group = klass->sai_thrift_remove_hostif_trap_group = ptf_hdlr_sai_thrift_remove_hostif_trap_group;
    base_class->sai_thrift_create_hostif_trap = klass->sai_thrift_create_hostif_trap = ptf_hdlr_sai_thrift_create_hostif_trap;
    base_class->sai_thrift_remove_hostif_trap = klass->sai_thrift_remove_hostif_trap = ptf_hdlr_sai_thrift_remove_hostif_trap;
    base_class->sai_thrift_set_hostif_trap = klass->sai_thrift_set_hostif_trap = ptf_hdlr_sai_thrift_set_hostif_trap;

    // ACL API
    base_class->sai_thrift_create_acl_table = klass->sai_thrift_create_acl_table = ptf_hdlr_sai_thrift_create_acl_table;
    base_class->sai_thrift_delete_acl_table = klass->sai_thrift_delete_acl_table = ptf_hdlr_sai_thrift_delete_acl_table;
    base_class->sai_thrift_create_acl_entry = klass->sai_thrift_create_acl_entry = ptf_hdlr_sai_thrift_create_acl_entry;
    base_class->sai_thrift_delete_acl_entry = klass->sai_thrift_delete_acl_entry = ptf_hdlr_sai_thrift_delete_acl_entry;
    base_class->sai_thrift_create_acl_counter = klass->sai_thrift_create_acl_counter = ptf_hdlr_sai_thrift_create_acl_counter;
    base_class->sai_thrift_delete_acl_counter = klass->sai_thrift_delete_acl_counter = ptf_hdlr_sai_thrift_delete_acl_counter;
    base_class->sai_thrift_get_acl_counter_attribute = klass->sai_thrift_get_acl_counter_attribute = ptf_hdlr_sai_thrift_get_acl_counter_attribute;

    // Mirror API
    base_class->sai_thrift_create_mirror_session = klass->sai_thrift_create_mirror_session = ptf_hdlr_sai_thrift_create_mirror_session;
    base_class->sai_thrift_remove_mirror_session = klass->sai_thrift_remove_mirror_session = ptf_hdlr_sai_thrift_remove_mirror_session;

    // Policer API
    base_class->sai_thrift_create_policer = klass->sai_thrift_create_policer = ptf_hdlr_sai_thrift_create_policer;
    base_class->sai_thrift_remove_policer = klass->sai_thrift_remove_policer = ptf_hdlr_sai_thrift_remove_policer;
    base_class->sai_thrift_get_policer_stats = klass->sai_thrift_get_policer_stats = ptf_hdlr_sai_thrift_get_policer_stats;
}

/*
 * set default state for PTF test
 * 1. set port to trunk, for SAI_PORT_ATTR_DROP_UNTAGGED and SAI_PORT_ATTR_DROP_TAGGED is false by default
*/

#define CTC_SAI_OBJECT_TYPE_SET(objtype,index)  					\
    (((sai_object_id_t)objtype << 32) | (sai_object_id_t)index)

int32
ptf_agent_set_default()
{
    sai_vlan_api_t *vlan_api = NULL;
    sai_port_api_t* port_api = NULL;
    sai_switch_api_t*  switch_api = NULL;
    uint32_t sai_attr_count = 1;
    sai_object_id_t portid_array[64];
    sai_attribute_t sai_attr_list[1];
    sai_attribute_t port_attr[1];
    sai_object_id_t portid = 0;
    uint32 port_count = 0;
    uint32 i = 0;
    sai_vlan_port_t vlan_ports[PTF_MAX_PORT_NUM];
    sai_status_t rc = SAI_STATUS_SUCCESS;
    sai_virtual_router_api_t*  router_api;
	

    /* 1. query SAI API */
    rc = sai_api_query(SAI_API_SWITCH, (void**)&switch_api);
    if (SAI_STATUS_SUCCESS != rc)
    {
        return 0;
    }
    rc = sai_api_query(SAI_API_PORT, (void**)&port_api);
    if (SAI_STATUS_SUCCESS != rc)
    {
        return 0;
    }
    rc = sai_api_query(SAI_API_VLAN, (void**)&vlan_api);
    if (SAI_STATUS_SUCCESS != rc)
    {
        return 0;
    }

    sai_api_query(SAI_API_VIRTUAL_ROUTER,(void**)&router_api);
    if (SAI_STATUS_SUCCESS != rc)
    {
        return 0;
    }
    router_api->remove_virtual_router(CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, 0));
	
    
    /* 3. convert thrift format to SAI format */
    sai_attr_list[0].id = SAI_SWITCH_ATTR_PORT_LIST;
    sai_attr_list[0].value.objlist.count = 64;
    sai_attr_list[0].value.objlist.list = portid_array;

    /* 4. call SAI API */
    if (switch_api)
    {
        /* 4.1 get portid list */
        rc = switch_api->get_switch_attribute(sai_attr_count, sai_attr_list);
        if (SAI_STATUS_SUCCESS != rc)
        {
            return 0;
        }

        port_count = sai_attr_list[0].value.objlist.count;
        
        /* 4.2 remove port from vlan 1 */
        for (i = 0; i < port_count; i++)
        {
            vlan_ports[i].port_id = sai_attr_list[0].value.objlist.list[i];
            vlan_ports[i].tagging_mode = SAI_VLAN_PORT_TAGGED;
        }
        vlan_api->remove_ports_from_vlan(1, port_count, vlan_ports);
        
        /* 4.3 set port to trunk */
        port_attr[0].id = SAI_PORT_ATTR_DROP_UNTAGGED;
        port_attr[0].value.booldata = 0;
        for (i = 0; i < port_count; i++)
        {
            portid = sai_attr_list[0].value.objlist.list[i];

            port_api->set_port_attribute(portid, port_attr);
        }

        port_attr[0].id = SAI_PORT_ATTR_DROP_TAGGED;
        port_attr[0].value.booldata = 0;
        for (i = 0; i < port_count; i++)
        {
            portid = sai_attr_list[0].value.objlist.list[i];

            port_api->set_port_attribute(portid, port_attr);
        }
    }
    else
    {
    }

    return 0;
}

