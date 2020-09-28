#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/snmp/get_cfg', methods=['GET'])
@base.web_app.auth.login_required
def snmp_mgr_cfg_R():
    resp = api.snmp.snmp_cfg_get()
    return jsonify(resp)

@base.web_app.app.route('/api/snmp/update_status/<type>/<int:enable>/', methods=['SET'])
@base.web_app.auth.login_required
def snmp_mgr_update_status_U(type, enable):
    resp = api.snmp.snmp_mgr_update_status(type, enable)
    return jsonify(resp) 

@base.web_app.app.route('/api/snmp/update_version/<version>/', methods=['SET'])
@base.web_app.auth.login_required
def snmp_mgr_update_version_U(version):
    resp = api.snmp.snmp_mgr_update_version(version)
    return jsonify(resp)     

@base.web_app.app.route('/api/snmp/target_addr_table/', methods=['GET', 'POST', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def snmp_mgr_target_addr_CURD():
    if request.method == 'GET':
        resp = api.snmp.snmp_target_addr_get()
    elif request.method == 'PUT':
        resp = api.snmp.snmp_target_addr_set(base.parse.parse_json(request.data)) 
    elif request.method == 'DELETE':
        resp = api.snmp.snmp_target_addr_del(base.parse.parse_json(request.data).get('targetAddr'))   
    return jsonify(resp)   
    
@base.web_app.app.route('/api/snmp/snmp_community_table/', methods=['GET', 'POST', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def snmp_mgr_community_CURD():
    if request.method == 'GET':
        resp = api.snmp.snmp_community_get()
    elif request.method == 'PUT':
        resp = api.snmp.snmp_community_set(base.parse.parse_json(request.data)) 
    elif request.method == 'DELETE':
        resp = api.snmp.snmp_community_del(base.parse.parse_json(request.data).get('community'))   
    return jsonify(resp)

@base.web_app.app.route('/api/snmp/snmp_context/', methods=['GET', 'POST', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def snmp_mgr_context_CURD():
    if request.method == 'GET':
        resp = api.snmp.snmp_context_get()
    elif request.method == 'PUT':
        resp = api.snmp.snmp_context_set(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.snmp.snmp_context_del(base.parse.parse_json(request.data).get('conText'))
    return jsonify(resp)

@base.web_app.app.route('/api/snmp/snmp_usm_user/', methods=['GET', 'POST', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def snmp_mgr_usm_user_CURD():
    if request.method == 'GET':
        resp = api.snmp.snmp_usm_user_get()
    elif request.method == 'PUT':
        resp = api.snmp.snmp_usm_user_set(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.snmp.snmp_usm_user_del(base.parse.parse_json(request.data).get('Usm_user'))
    return jsonify(resp)

@base.web_app.app.route('/api/snmp/snmp_notify/', methods=['GET', 'POST', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def snmp_mgr_notify_CURD():
    if request.method == 'GET':
        resp = api.snmp.snmp_notify_get()
    elif request.method == 'PUT':
        resp = api.snmp.snmp_notify_set(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.snmp.snmp_notify_del(base.parse.parse_json(request.data).get('noTify'))
    return jsonify(resp)

@base.web_app.app.route('/api/snmp/snmp_target_params/', methods=['GET', 'POST', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def snmp_mgr_params_CURD():
    if request.method == 'GET':
        resp = api.snmp.snmp_target_params_get()
    elif request.method == 'PUT':
        resp = api.snmp.snmp_target_params_set(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.snmp.snmp_target_params_del(base.parse.parse_json(request.data).get('target_Params'))
    return jsonify(resp)

@base.web_app.app.route('/api/snmp/snmp_target_address/', methods=['GET', 'POST', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def snmp_mgr_target_address_CURD():
    if request.method == 'GET':
        resp = api.snmp.snmp_target_address_get()
    elif request.method == 'PUT':
        resp = api.snmp.snmp_target_address_set(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.snmp.snmp_target_address_del(base.parse.parse_json(request.data).get('trapName'))
    return jsonify(resp)
 