#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 9, 2016

@author: wangqj
'''
from flask import jsonify, request
import base
import api

## ----- dot1x load dot1x port -----    
@base.web_app.app.route('/api/ipv4_port_cfg/load_l3_port/<key>/', methods=['GET'])
@base.web_app.auth.login_required
def ipv4_port_load_l3_port_R(key):
    resp = api.ipv4_port_cfg.ipv4_port_load_l3_port_get(key)
    return jsonify(resp)


@base.web_app.app.route('/api/ipv4_port_cfg/ipAddress/<iface>/<ipaddr>/<mask>/<enable>/<secondary>/', methods=['PUT'])
@base.web_app.auth.login_required
def ipv4_port_ip_address_U(iface, ipaddr, mask, enable, secondary):
    resp = api.ipv4_port_cfg.ipv4_port_ip_address_set(iface, ipaddr, mask, enable, secondary)
    return jsonify(resp)    
    
@base.web_app.app.route('/api/ipv4_port_cfg/ip_redirects/<iface>/<enable>/', methods=['PUT'])
@base.web_app.auth.login_required
def ipv4_port_ip_redirect_U(iface, enable):
    resp = api.ipv4_port_cfg.ipv4_port_ip_redirect_set(iface, enable)
    return jsonify(resp)        

@base.web_app.app.route('/api/ipv4_port_cfg/ipVerifyUnicastReversePath/<iface>/<enable>/', methods=['PUT'])
@base.web_app.auth.login_required
def ipv4_port_verify_unicast_reverse_path_U(iface, enable):
    resp = api.ipv4_port_cfg.ipv4_port_verify_unicast_reverse_path_set(iface, enable)
    return jsonify(resp)    