#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 9, 2016

@author: wangqj
'''
from flask import jsonify, request
import base
import api

## ----- dhcp snooping enable  -----
@base.web_app.app.route('/api/dhcp_snooping/load_dhcp_enabl/', methods=['GET'])
@base.web_app.auth.login_required
def dhcp_snooping_enable_R():
    resp = api.dhcp_snooping.load_dhcp_enabl_get()
    return jsonify(resp)

@base.web_app.app.route('/api/dhcp_snooping/load_dhcp_enabl/<int:enable>/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_snooping_enable_U(enable):
    resp = api.dhcp_snooping.load_dhcp_enabl_set(enable)
    return jsonify(resp)

## ----- dhcp service enable  -----
@base.web_app.app.route('/api/dhcp_snooping/load_dhcp_service_enabl/', methods=['GET'])
@base.web_app.auth.login_required
def dhcp_snooping_service_enable_R():
    resp = api.dhcp_snooping.load_dhcp_service_enabl_get()
    return jsonify(resp)

@base.web_app.app.route('/api/dhcp_snooping/load_dhcp_service_enabl/<int:enable>/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_snooping_service_enable_U(enable):
    resp = api.dhcp_snooping.load_dhcp_service_enabl_set(enable)
    return jsonify(resp)    
    
## ----- dhcp verify mac  -----
@base.web_app.app.route('/api/dhcp_snooping/load_dhcp_verify_mac/', methods=['GET'])
@base.web_app.auth.login_required
def dhcp_snooping_verify_mac_R():
    resp = api.dhcp_snooping.load_dhcp_verify_mac_get()
    return jsonify(resp)

@base.web_app.app.route('/api/dhcp_snooping/load_dhcp_verify_mac/<int:enable>/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_snooping_verify_mac_U(enable):
    resp = api.dhcp_snooping.load_dhcp_verify_mac_set(enable)
    return jsonify(resp)

## ----- dhcp save database  -----
@base.web_app.app.route('/api/dhcp_snooping/dhcp_save_database/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_save_database_U():
    resp = api.dhcp_snooping.dhcp_save_database_set()
    return jsonify(resp)

## ----- dhcp allow untrusted  -----
@base.web_app.app.route('/api/dhcp_snooping/dhcp_allow_untrusted/', methods=['GET'])
@base.web_app.auth.login_required
def dhcp_allow_untrusted_R():
    resp = api.dhcp_snooping.dhcp_allow_untrusted_get()
    return jsonify(resp)

@base.web_app.app.route('/api/dhcp_snooping/dhcp_allow_untrusted/<int:enable>/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_allow_untrusted_U(enable):
    resp = api.dhcp_snooping.dhcp_allow_untrusted_set(enable)
    return jsonify(resp)

## ----- dhcp snooping information option  -----
@base.web_app.app.route('/api/dhcp_snooping/dhcp_snooping_information_option/', methods=['GET'])
@base.web_app.auth.login_required
def dhcp_snooping_information_option_R():
    resp = api.dhcp_snooping.dhcp_snooping_information_option_get()
    return jsonify(resp)

@base.web_app.app.route('/api/dhcp_snooping/dhcp_snooping_information_option/<int:enable>/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_snooping_information_option_U(enable):
    resp = api.dhcp_snooping.dhcp_snooping_information_option_set(enable)
    return jsonify(resp)
    
## ----- dhcp bind entry  -----
@base.web_app.app.route('/api/dhcp_snooping/dhcp_load_bind_entry/', methods=['GET'])
@base.web_app.auth.login_required
def dhcp_load_bind_entry_R():
    resp = api.dhcp_snooping.dhcp_load_bind_entry_get()
    return jsonify(resp)

@base.web_app.app.route('/api/dhcp_snooping/dhcp_del_one_bind_entry/<bind_mac>/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_del_one_bind_entry_R(bind_mac):
    resp = api.dhcp_snooping.dhcp_del_one_bind_entry(bind_mac)
    return jsonify(resp)

'''       
@base.web_app.app.route('/api/dhcp_snooping/dhcp_del_all_bind_entry/', methods=['DELETE'])
@base.web_app.auth.login_required
def dhcp_del_all_bind_entry_R():
    resp = api.dhcp_snooping.dhcp_del_all_bind_entry(base.parse.parse_json(request.data))
    return jsonify(resp)
    
 
@base.web_app.app.route('/api/dhcp_snooping/dhcp_load_bind_entry/<int:enable>/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_load_bind_entry_U(enable):
    resp = api.dhcp_snooping.dhcp_load_bind_entry_set(enable)
    return jsonify(resp)
'''

@base.web_app.app.route('/api/dhcp_snooping/dhcp_bind_entry_set/', methods=['POST'])
@base.web_app.auth.login_required
def dhcp_bind_entry_U():
    resp = api.dhcp_snooping.dhcp_bind_entry_set(base.parse.parse_json(request.data))
    return jsonify(resp)
    
## ----- dhcp statistics  -----
@base.web_app.app.route('/api/dhcp_snooping/dhcp_packet_statistics/', methods=['GET'])
@base.web_app.auth.login_required
def dhcp_packet_statistics_R():
    resp = api.dhcp_snooping.dhcp_packet_statistics_get()
    return jsonify(resp)

@base.web_app.app.route('/api/dhcp_snooping/dhcp_packet_statistics/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_packet_statistics_U():
    resp = api.dhcp_snooping.dhcp_packet_statistics_delete()
    return jsonify(resp)

## ----- dhcp port trust  -----
@base.web_app.app.route('/api/dhcp_snooping/dhcp_port_trust/', methods=['GET'])
@base.web_app.auth.login_required
def dhcp_port_trust_R():
    resp = api.dhcp_snooping.dhcp_port_trust_get()
    return jsonify(resp)

@base.web_app.app.route('/api/dhcp_snooping/dhcp_port_trust/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_port_trust_U():
    resp = api.dhcp_snooping.dhcp_port_trust_set(base.parse.parse_json(request.data))
    return jsonify(resp)    

@base.web_app.app.route('/api/dhcp_snooping/dhcp_del_port_trust/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_del_port_trust_U():
    resp = api.dhcp_snooping.dhcp_del_port_trust(base.parse.parse_json(request.data))
    return jsonify(resp)       
    

## ----- dhcp modify interval  -----
@base.web_app.app.route('/api/dhcp_snooping/dhcp_get_interval/', methods=['GET'])
@base.web_app.auth.login_required
def dhcp_get_interval_U():
    resp = api.dhcp_snooping.dhcp_get_interval()
    return jsonify(resp)
    
@base.web_app.app.route('/api/dhcp_snooping/dhcp_modify_interval/<int:interval>/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_modify_interval_U(interval):
    resp = api.dhcp_snooping.dhcp_modify_interval_set(interval)
    return jsonify(resp)
    
@base.web_app.app.route('/api/dhcp_snooping/dhcp_modify_remote_id/<remote_id>/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_modify_remote_id_U(remote_id):
    resp = api.dhcp_snooping.dhcp_modify_remote_id(remote_id)
    return jsonify(resp)
    
@base.web_app.app.route('/api/dhcp_snooping/dhcp_add_snoop_vlan/<vlan_str>/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_add_snoop_vlan_U(vlan_str):
    resp = api.dhcp_snooping.dhcp_add_snoop_vlan(vlan_str)
    return jsonify(resp)

@base.web_app.app.route('/api/dhcp_snooping/dhcp_del_snoop_vlan/<vlan_str>/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_del_snoop_vlan_U(vlan_str):
    resp = api.dhcp_snooping.dhcp_del_snoop_vlan(vlan_str)
    return jsonify(resp)
    
@base.web_app.app.route('/api/dhcp_snooping/dhcp_load_snoop_db/<db_str>/', methods=['PUT'])
@base.web_app.auth.login_required
def dhcp_load_snoop_db_U(db_str):
    resp = api.dhcp_snooping.dhcp_load_snoop_db(db_str)
    return jsonify(resp)    
    