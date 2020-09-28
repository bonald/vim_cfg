#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/radius_mgr/radius_server/', methods=['GET', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def radius_mgr_server_CURD():
    if request.method == 'GET':
        resp = api.radius_mgr.radius_server_get()
    elif request.method == 'PUT':
        resp = api.radius_mgr.radius_server_add(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.radius_mgr.radius_server_del(base.parse.parse_json(request.data))
    return jsonify(resp)

