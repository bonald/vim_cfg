#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/log_mgr/', methods=['GET', 'POST', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def log_mgr_file_mgr_CURD():
    if request.method == 'GET':
        resp = api.log_mgr.get()
    elif request.method == 'DELETE':
        resp = api.log_mgr.flush()
    return jsonify(resp)
@base.web_app.app.route('/api/log_mgr/get_cfg', methods=['GET'])
@base.web_app.auth.login_required
def log_mgr_cfg_R():
    resp = api.log_mgr.log_cfg_get()
    return jsonify(resp)

@base.web_app.app.route('/api/log_mgr/update_log_status/', methods=['PUT'])
@base.web_app.auth.login_required
def log_mgr_update_status_U():
    resp = api.log_mgr.log_mgr_update_status(base.parse.parse_json(request.data))
    return jsonify(resp)

@base.web_app.app.route('/api/log_mgr/update_log_level/', methods=['PUT'])
@base.web_app.auth.login_required
def log_mgr_update_level_U():
    resp = api.log_mgr.log_mgr_update_level(base.parse.parse_json(request.data))
    return jsonify(resp)   

@base.web_app.app.route('/api/log_mgr/update_log_buffer_size/', methods=['PUT'])
@base.web_app.auth.login_required
def log_mgr_update_buffer_size_U():
    resp = api.log_mgr.log_mgr_update_buffer_size(base.parse.parse_json(request.data))
    return jsonify(resp) 

@base.web_app.app.route('/api/log_mgr/server_addr_table/', methods=['GET', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def log_mgr_server_addr_CURD():
    if request.method == 'GET':
        resp = api.log_mgr.log_mgr_srv_addr_get()
    elif request.method == 'PUT':
        resp = api.log_mgr.log_mgr_srv_addr_set(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.log_mgr.log_mgr_srv_addr_del(base.parse.parse_json(request.data))
    return jsonify(resp)
