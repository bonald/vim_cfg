#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/tap/group_table/', methods=['GET', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def tap_group_table_CURD():
    if request.method == 'GET':
        resp = api.tap_mgr.tap_group_table_get()
    elif request.method == 'PUT':
        resp = api.tap_mgr.tap_group_table_add(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.tap_mgr.tap_group_table_del(base.parse.parse_json(request.data).get('group_name'))
    return jsonify(resp)

@base.web_app.app.route('/api/tap/truncation/', methods=['GET', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def tap_truncation_CURD():
    if request.method == 'GET':
        resp = api.tap_mgr.tap_truncation_get()
    elif request.method == 'PUT':
        resp = api.tap_mgr.tap_truncation_set(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.tap_mgr.tap_truncation_del()        
    return jsonify(resp)    

@base.web_app.app.route('/api/tap/description/', methods=['GET', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def tap_description_CURD():
    if request.method == 'GET':
        resp = api.tap_mgr.tap_description_get()
    elif request.method == 'PUT':
        resp = api.tap_mgr.tap_description_set(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.tap_mgr.tap_description_del()        
    return jsonify(resp)        

@base.web_app.app.route('/api/tap/tap_group_egress/', methods=['GET', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def tap_entry_CURD():
    if request.method == 'GET':
        resp = api.tap_mgr.tap_entry_get()
    elif request.method == 'PUT':
        resp = api.tap_mgr.tap_entry_set(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.tap_mgr.tap_entry_del(base.parse.parse_json(request.data))      
    return jsonify(resp)

@base.web_app.app.route('/api/tap/tap_timestamp/', methods=['GET', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def tap_timestamp_CURD():
    if request.method == 'GET':
        resp = api.tap_mgr.tap_timestamp_get()
    elif request.method == 'PUT':
        resp = api.tap_mgr.tap_timestamp_set(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.tap_mgr.tap_timestamp_del(base.parse.parse_json(request.data))      
    return jsonify(resp)     

    