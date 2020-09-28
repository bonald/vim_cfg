#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/aaa_mgr/aaa_status/', methods=['GET', 'PUT'])
@base.web_app.auth.login_required
def aaa_mgr_status_CURD():
    if request.method == 'GET':
        resp = api.aaa_mgr.aaa_status_get()
    elif request.method == 'PUT':
        resp = api.aaa_mgr.aaa_status_set(base.parse.parse_json(request.data).get('enable'))
    return jsonify(resp)

@base.web_app.app.route('/api/aaa_mgr/aaa_method_list/', methods=['GET', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def aaa_mgr_method_list_CURD():
    if request.method == 'GET':
        resp = api.aaa_mgr.aaa_method_get()
    elif request.method == 'PUT':
        resp = api.aaa_mgr.aaa_method_set(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.aaa_mgr.aaa_method_del(base.parse.parse_json(request.data))
    return jsonify(resp)    
