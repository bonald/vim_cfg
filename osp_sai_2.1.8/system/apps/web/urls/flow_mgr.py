#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/flow/flow_table/', methods=['GET', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def flow_table_CURD():
    if request.method == 'GET':
        resp = api.flow_mgr.flow_table_get()
    elif request.method == 'PUT':
        resp = api.flow_mgr.flow_table_add(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.flow_mgr.flow_table_del(base.parse.parse_json(request.data))
    return jsonify(resp)


@base.web_app.app.route('/api/flow/flow_entry/', methods=['GET', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def flow_entry_CURD():
    if request.method == 'GET':
        resp = api.flow_mgr.flow_entry_get()
    elif request.method == 'PUT':
        resp = api.flow_mgr.flow_entry_add(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.flow_mgr.flow_entry_del(base.parse.parse_json(request.data))
    return jsonify(resp)

    
@base.web_app.app.route('/api/flow/acl_config/remark', methods=['GET', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def flow_acl_config_remark_CURD():
    if request.method == 'GET':
        resp = api.flow_mgr.flow_acl_config_remark_get()
    elif request.method == 'PUT':
        resp = api.flow_mgr.flow_acl_config_remark_add(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.flow_mgr.flow_acl_config_remark_del(base.parse.parse_json(request.data))
    return jsonify(resp)   



@base.web_app.app.route('/api/flow/flow_get_entry_from_tbl/<table>/<flow>/', methods=['PUT'])
def flow_get_entry_from_tbl(table,flow):
    if request.method == 'PUT':
        resp = api.flow_mgr.flow_entry_get(table, flow)
    return jsonify(resp)   
