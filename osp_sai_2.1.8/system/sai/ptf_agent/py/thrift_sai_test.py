#!/usr/bin/env python

#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements. See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership. The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License. You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the License for the
# specific language governing permissions and limitations
# under the License.
#

import sys, glob
sys.path.append('switch_sai_thrift')

#from tutorial import Calculator
#from tutorial.ttypes import *

import switch_sai_thrift
import switch_sai_thrift.switch_sai_rpc as switch_sai_rpc

from switch import  *
from switch_sai_thrift.ttypes import  *
from switch_sai_thrift.sai_headers import  *

switch_inited=0
port_list = []

# DUT configuration
PTF_DUT_ADDRESS = 'localhost'
PTF_DUT_PORT = 9090
#PTF_DUT_ADDRESS = '10.10.39.103'
#PTF_DUT_PORT = 60013
if (len(sys.argv) == 2):
	PTF_DUT_PORT = sys.argv[1]

if (len(sys.argv) == 3):
	PTF_DUT_ADDRESS = sys.argv[1]
	PTF_DUT_PORT = sys.argv[2]

print "Connect to DUT: %s port: %s" % (PTF_DUT_ADDRESS, PTF_DUT_PORT)

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

# Start Test
transport = TSocket.TSocket(PTF_DUT_ADDRESS, PTF_DUT_PORT)
transport = TTransport.TBufferedTransport(transport)
protocol = TBinaryProtocol.TBinaryProtocol(transport)

client = switch_sai_rpc.Client(protocol)
transport.open()

### port API
enable_port_API_set_port_attribute		= 0
### fdb API
enable_fdb_API_create_fdb_entry			= 0
enable_fdb_API_delete_fdb_entry			= 0
enable_fdb_API_flush_fdb_entries		= 0
### vlan API
enable_vlan_API_create_vlan				= 0
enable_vlan_API_delete_vlan				= 0
enable_vlan_API_add_ports_to_vlan		= 0
enable_vlan_API_remove_ports_from_vlan	= 0
enable_vlan_API_get_vlan_stats			= 0
### virtual router API
enable_virtualrouterAPI_create_virtual_router		= 0
enable_virtualrouterAPI_remove_virtual_router		= 0
### route API
enable_route_API_create_route						= 0
enable_route_API_remove_route						= 0
### router interface API
enable_routerinterfaceAPI_create_router_interface	= 0
enable_routerinterfaceAPI_remove_router_interface	= 0
### next hop API
enable_nexthop_API_create_next_hop					= 0
enable_nexthop_API_remove_next_hop					= 0
### next hop group API
enable_nexthopgroupAPI_create_next_hop_group		= 0
enable_nexthopgroupAPI_remove_next_hop_group		= 0
enable_nexthopgroupAPI_add_next_hop_to_group		= 0
enable_nexthopgroupAPI_remove_next_hop_from_group	= 0
### lag API
enable_lag_API_create_lag					= 0
enable_lag_API_remove_lag					= 0
enable_lag_API_create_lag_member			= 0
enable_lag_API_remove_lag_member			= 0
### stp API
enable_stp_API_create_stp_entry				= 0
enable_stp_API_remove_stp_entry				= 0
enable_stp_API_set_stp_port_state			= 0
enable_stp_API_get_stp_port_state			= 0
### neighbor API
enable_neighbor_API_create_neighbor_entry	= 0
enable_neighbor_API_remove_neighbor_entry	= 0
### switch API
enable_switch_API_get_switch_attribute		= 0
enable_switch_API_set_switch_attribute		= 0
### Trap API
enable_trap_API_create_hostif				= 0
enable_trap_API_remove_hostif				= 0
enable_trap_API_create_hostif_trap_group	= 0
enable_trap_API_remove_hostif_trap_group	= 0
enable_trap_API_create_hostif_trap			= 0
enable_trap_API_remove_hostif_trap			= 0
enable_trap_API_set_hostif_trap				= 0
### ACL API
enable_ACL_API_create_acl_table				= 0
enable_ACL_API_delete_acl_table				= 0
enable_ACL_API_create_acl_entry				= 0
enable_ACL_API_delete_acl_entry				= 0
enable_ACL_API_create_acl_counter			= 0
enable_ACL_API_delete_acl_counter			= 0
enable_ACL_API_get_acl_counter_attribute	= 0
### Mirror API
enable_Mirror_API_create_mirror_session		= 1
enable_Mirror_API_remove_mirror_session		= 1
### Policer API
enable_Policer_API_create_policer			= 0
enable_Policer_API_remove_policer			= 0
enable_Policer_API_get_policer_stats		= 0

port_id1 = 0x100000008	
vid1 = 10
mac1 = '00:11:22:5A:5A:FF'
mac_action = 1

###################################################### 
# port API
###################################################### 
if enable_port_API_set_port_attribute:
	print "##### Test port API - set_port_attribute() #####"
	print "Test SAI_PORT_ATTR_PORT_VLAN_ID"
	attr_value = sai_thrift_attribute_value_t(u16=vid1)
	attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
	client.sai_thrift_set_port_attribute(port_id1, attr)

	print "Test SAI_PORT_ATTR_ADMIN_STATE"
	attr_value = sai_thrift_attribute_value_t(booldata=1)
	attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_ADMIN_STATE, value=attr_value)
	client.sai_thrift_set_port_attribute(port_id1, attr)

###################################################### 
# fdb API
###################################################### 
if enable_fdb_API_create_fdb_entry:
	print "##### Test fdb API - create_fdb_entry() #####"
	sai_thrift_create_fdb(client, vid1, mac1, port_id1, mac_action)

if enable_fdb_API_delete_fdb_entry:
	print "##### Test fdb API - delete_fdb_entry() #####"
	sai_thrift_delete_fdb(client, vid1, mac1, port_id1)

if enable_fdb_API_flush_fdb_entries:
	print "##### Test fdb API - flush_fdb_entries() #####"
	sai_thrift_flush_fdb_by_vlan(client, vid1)

###################################################### 
# vlan API
###################################################### 
if enable_vlan_API_create_vlan:
	print "##### Test vlan API - create_vlan() #####"
	client.sai_thrift_create_vlan(vid1)

if enable_vlan_API_delete_vlan:
	print "##### Test vlan API - delete_vlan() #####"
	client.sai_thrift_delete_vlan(vid1)

if enable_vlan_API_add_ports_to_vlan:
	print "##### Test vlan API - add_ports_to_vlan() #####"
	vlan_port1 = sai_thrift_vlan_port_t(port_id=11, tagging_mode=SAI_VLAN_PORT_UNTAGGED)
	vlan_port2 = sai_thrift_vlan_port_t(port_id=12, tagging_mode=SAI_VLAN_PORT_UNTAGGED)
	client.sai_thrift_add_ports_to_vlan(vid1, [vlan_port1,vlan_port2])

if enable_vlan_API_remove_ports_from_vlan:
	print "##### Test vlan API - remove_ports_from_vlan() #####"
	vlan_port1 = sai_thrift_vlan_port_t(port_id=11, tagging_mode=SAI_VLAN_PORT_UNTAGGED)
	vlan_port2 = sai_thrift_vlan_port_t(port_id=12, tagging_mode=SAI_VLAN_PORT_UNTAGGED)
	client.sai_thrift_remove_ports_from_vlan(vid1, [vlan_port1,vlan_port2])

if enable_vlan_API_get_vlan_stats:
	print "##### Test vlan API - get_vlan_stats() #####"
	vlan_id1 = 11
	counter_ids = [0, 2, 4, 6]
	number_of_counters = 4
	counters = client.sai_thrift_get_vlan_stats(vlan_id1, counter_ids, number_of_counters)
	print counters

###################################################### 
# virtual router API
###################################################### 
if enable_virtualrouterAPI_create_virtual_router:
	print "##### Test virtual router API - create_virtual_router() #####"
	vr_id1 = sai_thrift_create_virtual_router(client, 1, 1)
	print "vr_id = 0x%lx" % vr_id1

if enable_virtualrouterAPI_remove_virtual_router:
	print "##### Test virtual router API - remove_virtual_router() #####"
	client.sai_thrift_remove_virtual_router(vr_id1)

###################################################### 
# route API
###################################################### 
if enable_route_API_create_route:
	print "##### Test route API - create_route() #####"
	addr_family = SAI_IP_ADDR_FAMILY_IPV4
	ip_addr1 = '10.10.10.1'
	ip_mask1 = '255.255.255.255'
	vr_id1 = 10
	nhop1 = 11
	sai_thrift_create_route(client, vr_id1, addr_family, ip_addr1, ip_mask1, nhop1)

if enable_route_API_remove_route:
	print "##### Test route API - remove_route() #####"
	addr_family = SAI_IP_ADDR_FAMILY_IPV4
	ip_addr1 = '10.10.10.1'
	ip_mask1 = '255.255.255.0'
	vr_id1 = 20
	nhop1 = 21
	sai_thrift_remove_route(client, vr_id1, addr_family, ip_addr1, ip_mask1, nhop1)

###################################################### 
# router interface API
###################################################### 
if enable_routerinterfaceAPI_create_router_interface:
	print "##### Test router interface API - create_router_interface() #####"
	vr_id1 = 1
	is_port = 1
	rif_id1 = sai_thrift_create_router_interface(client, vr_id1, is_port, 10, 10, 1, 1, mac1)
	print rif_id1

if enable_routerinterfaceAPI_remove_router_interface:
	print "##### Test router interface API - remove_router_interface() #####"
	client.sai_thrift_remove_router_interface(rif_id1)

###################################################### 
# next hop API
###################################################### 
if enable_nexthop_API_create_next_hop:
	print "##### Test next hop API - create_next_hop() #####"
	addr_family = SAI_IP_ADDR_FAMILY_IPV4
	ip_addr1 = '10.10.10.1'
	ip_mask1 = '255.255.255.255'
	rif_id1 = 3
	nhop1 = sai_thrift_create_nhop(client, addr_family, ip_addr1, rif_id1)
	print "nhid = %u" % nhop1

if enable_nexthop_API_remove_next_hop:
	print "##### Test next hop API - remove_next_hop() #####"
	nhop1 = 4
	client.sai_thrift_remove_next_hop(nhop1)

###################################################### 
# next hop group API
###################################################### 
if enable_nexthopgroupAPI_create_next_hop_group:
	print "##### Test next hop group API - create_next_hop_group() #####"
	nhop1 = 11
	nhop2 = 12
	nhop_group1 = sai_thrift_create_next_hop_group(client, [nhop1, nhop2])
	print "nhop_group1 = 0x%lx" % nhop_group1

if enable_nexthopgroupAPI_remove_next_hop_group:
	print "##### Test next hop group API - remove_next_hop_group() #####"
	nhop_group1 = 22
	client.sai_thrift_remove_next_hop_group(nhop_group1)

if enable_nexthopgroupAPI_add_next_hop_to_group:
	print "##### Test next hop group API - add_next_hop_to_group() #####"
	nhop_group1 = 22
	nhop1 = 11
	nhop2 = 12
	client.sai_thrift_add_next_hop_to_group(nhop_group1, [nhop1, nhop2])

if enable_nexthopgroupAPI_remove_next_hop_from_group:
	print "##### Test next hop group API - remove_next_hop_from_group() #####"
	nhop_group1 = 22
	nhop1 = 11
	nhop2 = 12
	client.sai_thrift_remove_next_hop_from_group(nhop_group1, [nhop1, nhop2])

###################################################### 
# lag API
###################################################### 
if enable_lag_API_create_lag:
	print "##### Test lag API - create_lag() #####"
	port1 = 10
	port2 = 11
	port3 = 12
	port4 = 13
	lag_id1 = sai_thrift_create_lag(client, [port1, port2, port3, port4])
	print "lag_id %u" % (lag_id1)

if enable_lag_API_remove_lag:
	print "##### Test lag API - remove_lag() #####"
	client.sai_thrift_remove_lag(lag_id1)

if enable_lag_API_create_lag_member:
	print "##### Test lag API - create_lag_member() #####"
	lag_id1 = 12
	port_id1 = 13
	lag_member_id1 = sai_thrift_create_lag_member(client, lag_id1, port_id1)

if enable_lag_API_remove_lag_member:
	print "##### Test lag API - remove_lag_member() #####"
	lag_member_id1 = 123
	client.sai_thrift_remove_lag_member(lag_member_id1)

###################################################### 
# stp API
###################################################### 
if enable_stp_API_create_stp_entry:
	print "##### Test stp API - create_stp_entry() #####"
	vlan_list1 = [10, 11]
	stp_id1 = sai_thrift_create_stp_entry(client, vlan_list1)
	print "stp_id = %u" % stp_id1

if enable_stp_API_remove_stp_entry:
	print "##### Test stp API - remove_stp_entry() #####"
	client.sai_thrift_remove_stp_entry(stp_id1)

if enable_stp_API_set_stp_port_state:
	print "##### Test stp API - set_stp_port_state() #####"
	port_id1 = 0x100000009
	stp_id1 = 0x200000009
	client.sai_thrift_set_stp_port_state(stp_id1, port_id1, 3)

if enable_stp_API_get_stp_port_state:
	print "##### Test stp API - get_stp_port_state() #####"
	port_id1 = 0x10000000a
	stp_id1 = 0x20000000a
	stp_state = client.sai_thrift_get_stp_port_state(stp_id1, port_id1)
	print "stp_state = %u" % stp_state

###################################################### 
# neighbor API
###################################################### 
if enable_neighbor_API_create_neighbor_entry:
	print "##### Test neighbor API - create_neighbor_entry() #####"
	addr_family = SAI_IP_ADDR_FAMILY_IPV4
	rif_id1 = 111
	nhop_ip1 = '11.11.11.11'
	dmac1 = '00:11:22:33:44:55'
	sai_thrift_create_neighbor(client, addr_family, rif_id1, nhop_ip1, dmac1)

if enable_neighbor_API_remove_neighbor_entry:
	print "##### Test neighbor API - remove_neighbor_entry() #####"
	addr_family = SAI_IP_ADDR_FAMILY_IPV4
	rif_id1 = 2222
	nhop_ip1 = '22.22.33.44'
	dmac1 = '00:11:22:33:44:55'
	sai_thrift_remove_neighbor(client, addr_family, rif_id1, nhop_ip1, dmac1)

###################################################### 
# switch API
###################################################### 
if enable_switch_API_get_switch_attribute:
	print "##### Test switch API - get_switch_attribute() #####"
	switch_attr_list = client.sai_thrift_get_switch_attribute()
	attr_list = switch_attr_list.attr_list

	for attribute in attr_list:
		if attribute.id == 0:
			print "max ports: %u" % attribute.value.u32
		elif attribute.id == 1:
			for x in attribute.value.objlist.object_id_list:
				port_list.append(x)
				print "%lx" % x
	print port_list

if enable_switch_API_set_switch_attribute:
	print "##### Test switch API - set_switch_attribute() #####"
	attr_value = sai_thrift_attribute_value_t(mac='00:77:66:55:44:33')
	attr = sai_thrift_attribute_t(id=SAI_SWITCH_ATTR_SRC_MAC_ADDRESS, value=attr_value)
	client.sai_thrift_set_switch_attribute(attr)

###################################################### 
# Mirror API
######################################################
if enable_Mirror_API_create_mirror_session:
	print "##### Test Mirror API - create_mirror_session() #####"
	mirror_type = SAI_MIRROR_TYPE_LOCAL
	port3 = 111
	ingress_mirror_id = sai_thrift_create_mirror_session(client, mirror_type, port3,
												 0, 0, 0,
												 None, None,
												 0, None, None,
												 0, 0, 0, 0)
	print "ingress_mirror_id = %u" % (ingress_mirror_id)

if enable_Mirror_API_remove_mirror_session:
	print "##### Test Mirror API - remove_mirror_session() #####"
	ingress_mirror_id = 123
	client.sai_thrift_remove_mirror_session(ingress_mirror_id)

###################################################### 
# Policer API
###################################################### 
if enable_Policer_API_create_policer:
	print "##### Test Policer API - create_policer() #####"
	create_policer_attr1_value = sai_thrift_attribute_value_t(s32=SAI_METER_TYPE_BYTES)
	create_policer_attr1 = sai_thrift_attribute_t(id=SAI_POLICER_ATTR_METER_TYPE, value=create_policer_attr1_value)
	create_policer_attr2_value = sai_thrift_attribute_value_t(s32=SAI_POLICER_MODE_Tr_TCM)
	create_policer_attr2 = sai_thrift_attribute_t(id=SAI_POLICER_ATTR_MODE, value=create_policer_attr2_value)
	create_policer_attr3_value = sai_thrift_attribute_value_t(s32=SAI_POLICER_COLOR_SOURCE_AWARE)
	create_policer_attr3 = sai_thrift_attribute_t(id=SAI_POLICER_ATTR_COLOR_SOURCE, value=create_policer_attr3_value)
	create_policer_attr4_value = sai_thrift_attribute_value_t(u64=5678)
	create_policer_attr4 = sai_thrift_attribute_t(id=SAI_POLICER_ATTR_CBS, value=create_policer_attr4_value)
	create_policer_attr5_value = sai_thrift_attribute_value_t(s32=0)
	create_policer_attr5 = sai_thrift_attribute_t(id=SAI_POLICER_ATTR_ENABLE_COUNTER_LIST, value=create_policer_attr5_value)
	create_policer_attr_list = [create_policer_attr1, create_policer_attr2, create_policer_attr3, create_policer_attr4, create_policer_attr5]
	client.sai_thrift_create_policer(create_policer_attr_list)

if enable_Policer_API_remove_policer:
	print "##### Test Policer API - remove_policer() #####"
	policer_id = 4
	client.sai_thrift_remove_policer(policer_id)
	
if enable_Policer_API_get_policer_stats:
	print "##### Test Policer API - get_policer_stats() #####"
	policer_id = 11
	counter_ids = [0,1,2,3,4,5,6,7]
	counters = client.sai_thrift_get_policer_stats(policer_id, counter_ids)
	print counters

###################################################### 
# Policer API
###################################################### 
if enable_ACL_API_create_acl_table:
   print "##### Test ACL API - create_acl_table() #####"
   addr_family = SAI_IP_ADDR_FAMILY_IPV4
   ip_src = 1
   ip_dst = 1
   ip_proto = 1
   in_ports = 1
   out_ports = 1
   in_port = 1
   out_port = 1
   table_id = sai_thrift_create_acl_table(client, addr_family, ip_src, ip_dst, ip_proto, in_ports, out_ports, in_port, out_port)
   print "tablr_id = %lu" % table_id

if enable_ACL_API_delete_acl_table:
   print "##### Test ACL API - delete_acl_table() #####"
   addr_family = SAI_IP_ADDR_FAMILY_IPV4
   table_id = 0x1000000000000002
   client.sai_thrift_delete_acl_table(table_id)   

if enable_ACL_API_create_acl_entry:
   print "##### Test ACL API - create_acl_entry() #####"
   addr_family = SAI_IP_ADDR_FAMILY_IPV4
   acl_table_id = 2
   action = 2
   ip_src = '11.11.11.11'
   ip_src_mask = '255.255.0.0'
   ip_dst = '11.22.22.22'
   ip_dst_mask = '255.255.255.0'
   ip_proto = 10
   in_port = 3
   out_port = 4
   ingress_mirror = 5
   egress_mirror = 6
   in_port_list = 0
   out_port_list = 0
   acl_entry_id = sai_thrift_create_acl_entry(client, acl_table_id, action, addr_family, ip_src, ip_src_mask, ip_dst, ip_dst_mask, ip_proto, in_port_list, out_port_list, in_port, out_port, ingress_mirror, egress_mirror) 
   print "acl_entry_id = %lu" % acl_entry_id 
 
if enable_ACL_API_delete_acl_entry:
   print "##### Test ACL API - delete_acl_entry() #####"
   table_id = 0x1000000000000002
   client.sai_thrift_delete_acl_entry(table_id)  
 
if enable_ACL_API_create_acl_counter:
   print "##### Test ACL API - create_acl_counter() #####"   
   create_acl_counter_attr1_value = sai_thrift_attribute_value_t(oid=12345)
   create_acl_counter_attr1 = sai_thrift_attribute_t(id=SAI_ACL_ENTRY_ATTR_TABLE_ID, value=create_acl_counter_attr1_value)
   create_policer_attr_list = [create_acl_counter_attr1]
   counter_id = client.sai_thrift_create_acl_counter(create_policer_attr_list)  
   print "counter_id = %lu" % counter_id 
   
if enable_ACL_API_delete_acl_counter:
   print "##### Test ACL API - delete_acl_counter() #####"  
   acl_counter_id = 5678
   client.sai_thrift_delete_acl_counter(acl_counter_id)   
 
if enable_ACL_API_get_acl_counter_attribute:
   print "##### Test ACL API - get_acl_counter_attribute() #####"  
   acl_counter_id = 5678
   get_acl_counter_attr_list = [3,4]
   counter_list = client.sai_thrift_get_acl_counter_attribute(acl_counter_id, get_acl_counter_attr_list)   
   print "counter_list length = %u" % len(counter_list)
   for counter in counter_list:
	   print "counter: 0x%lx" % counter.u64

transport.close()
