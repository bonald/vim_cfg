#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 9, 2016

@author: wangqj
'''
from flask import jsonify, request
import base
import api

## ----- dot1x auth ctl  -----
@base.web_app.app.route('/api/dot1x/load_Auth_Ctl/', methods=['GET'])
@base.web_app.auth.login_required
def dot1x_auth_ctl_enable_R():
    resp = api.dot1x.dot1x_auth_ctl_enable_get()
    return jsonify(resp)

@base.web_app.app.route('/api/dot1x/load_Auth_Ctl_set/<int:enable>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_auth_ctl_enable_U(enable):
    resp = api.dot1x.dot1x_auth_ctl_enable_set(enable)
    return jsonify(resp)

## ----- dot1x re-active radius server  -----
@base.web_app.app.route('/api/dot1x/re_active_radius_server/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_re_active_radius_server_U():
    resp = api.dot1x.dot1x_re_active_radius_server()
    return jsonify(resp)

## ----- dot1x re-active tacacs server -----    
@base.web_app.app.route('/api/dot1x/re_active_tacacs_server/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_re_active_tacacs_server_U():
    resp = api.dot1x.dot1x_re_active_tacacs_server()
    return jsonify(resp)
    
    
    
## ----- dot1x init port  -----    
@base.web_app.app.route('/api/dot1x/init_port/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_init_port_U():
    resp = api.dot1x.dot1x_init_port(base.parse.parse_json(request.data))
    return jsonify(resp)

## ----- dot1x re-auth port  -----    
@base.web_app.app.route('/api/dot1x/re_auth_port/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_re_auth_port_U():
    resp = api.dot1x.dot1x_re_auth_port_set(base.parse.parse_json(request.data))
    return jsonify(resp)

## ----- dot1x clear port  -----    
@base.web_app.app.route('/api/dot1x/clear_port/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_clear_port_U():
    resp = api.dot1x.dot1x_clear_port(base.parse.parse_json(request.data))
    return jsonify(resp)


## ----- dot1x load dot1x port -----    
@base.web_app.app.route('/api/dot1x/load_dot1x_port/<key>/', methods=['GET'])
@base.web_app.auth.login_required
def dot1x_load_dot1x_port_R(key):
    resp = api.dot1x.dot1x_load_dot1x_port_get(key)
    return jsonify(resp)



## ----- dot1x radius server deadtime  -----    
@base.web_app.app.route('/api/dot1x/dot1xRadiusDeadtime/<int:deadtime>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_radius_server_deadtime_U(deadtime):
    resp = api.dot1x.dot1x_radius_server_deadtime_set(deadtime)
    return jsonify(resp)

## ----- dot1x radius server retransmit  -----    
@base.web_app.app.route('/api/dot1x/dot1xRadiusRetransmit/<int:retransmit>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_radius_server_retransmit_U(retransmit):
    resp = api.dot1x.dot1x_radius_server_retransmit_set(retransmit)
    return jsonify(resp)

## ----- dot1x radius server timeout  -----    
@base.web_app.app.route('/api/dot1x/dot1xRadiusTimeout/<int:timeout>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_radius_server_timeout_U(timeout):
    resp = api.dot1x.dot1x_radius_server_timeout_set(timeout)
    return jsonify(resp)

## ----- dot1x radius server key  -----    
@base.web_app.app.route('/api/dot1x/dot1xRadiusKey/<base64_radius_srv_key>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_radius_server_key_U(base64_radius_srv_key):
    resp = api.dot1x.dot1x_radius_server_key_set(base.parse.parse_base64(base64_radius_srv_key))
    return jsonify(resp)


## ----- dot1x port-control:  -----    
@base.web_app.app.route('/api/dot1x/port_control/<port_name>/<ctl_state>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_port_control_U(port_name, ctl_state):
    resp = api.dot1x.dot1x_port_control_set(port_name, ctl_state)
    return jsonify(resp)
   
## ----- dot1x radius server key  -----    
@base.web_app.app.route('/api/dot1x/port_direction/<port_name>/<direction>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_port_direction_U(port_name, direction):
    resp = api.dot1x.dot1x_port_direction_set(port_name, direction)
    return jsonify(resp)
    
@base.web_app.app.route('/api/dot1x/version/<port_name>/<version>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_port_protocol_version_U(port_name, version):
    resp = api.dot1x.dot1x_port_protocol_version_set(port_name, version)
    return jsonify(resp)

@base.web_app.app.route('/api/dot1x/reAuthentication/<port_name>/<int:reAuth>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_port_re_authentication_U(port_name, reAuth):
    resp = api.dot1x.dot1x_port_re_authentication_set(port_name, reAuth)
    return jsonify(resp)

@base.web_app.app.route('/api/dot1x/macAuthBypassOne/<port_name>/<int:bypass>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_port_mac_auth_bypass_U(port_name, bypass):
    resp = api.dot1x.dot1x_port_mac_auth_bypass_set(port_name, bypass)
    return jsonify(resp)

@base.web_app.app.route('/api/dot1x/maxreq/<port_name>/<int:req>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_port_max_req_U(port_name, req):
    resp = api.dot1x.dot1x_port_max_req_set(port_name, req)
    return jsonify(resp)

@base.web_app.app.route('/api/dot1x/txperiod/<port_name>/<int:period>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_port_tx_period_U(port_name, period):
    resp = api.dot1x.dot1x_port_tx_period_set(port_name, period)
    return jsonify(resp)    

@base.web_app.app.route('/api/dot1x/quietPeriod/<port_name>/<int:period>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_port_quiet_period_U(port_name, period):
    resp = api.dot1x.dot1x_port_quiet_period_set(port_name, period)
    return jsonify(resp)  
    
@base.web_app.app.route('/api/dot1x/reAuthperiod/<port_name>/<int:re_authperiod>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_port_re_authperiod_U(port_name, re_authperiod):
    resp = api.dot1x.dot1x_port_re_authperiod_set(port_name, re_authperiod)
    return jsonify(resp)    

@base.web_app.app.route('/api/dot1x/supptimeoutOne/<port_name>/<int:supp_timeout>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_port_supp_timeoutOne_U(port_name, supp_timeout):
    resp = api.dot1x.dot1x_port_supp_timeoutOne_set(port_name, supp_timeout)
    return jsonify(resp)      

@base.web_app.app.route('/api/dot1x/serverTimeoutOne/<port_name>/<int:server_timeout>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_port_server_timeout_U(port_name, server_timeout):
    resp = api.dot1x.dot1x_port_server_timeout_set(port_name, server_timeout)
    return jsonify(resp)      

@base.web_app.app.route('/api/dot1x/guestVlanOne/<port_name>/<int:guest_vlan>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_port_guest_vlan_U(port_name, guest_vlan):
    resp = api.dot1x.dot1x_port_guest_vlan_set(port_name, guest_vlan)
    return jsonify(resp)      

@base.web_app.app.route('/api/dot1x/portModeOne/<port_name>/<int:port_mode>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_port_port_mode_U(port_name, port_mode):
    resp = api.dot1x.dot1x_port_port_mode_set(port_name, port_mode)
    return jsonify(resp) 

@base.web_app.app.route('/api/dot1x/maxUserOne/<port_name>/<int:max_user>/', methods=['PUT'])
@base.web_app.auth.login_required
def dot1x_port_max_user_U(port_name, max_user):
    resp = api.dot1x.dot1x_port_max_user_set(port_name, max_user)
    return jsonify(resp) 







    