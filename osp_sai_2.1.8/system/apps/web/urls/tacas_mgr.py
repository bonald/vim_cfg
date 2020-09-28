#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/tacas_mgr/tacas_server/', methods=['GET', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def tacas_mgr_server_CURD():
    if request.method == 'GET':
        resp = api.tacas_mgr.tacas_server_get()
    elif request.method == 'PUT':
        resp = api.tacas_mgr.tacas_server_add(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.tacas_mgr.tacas_server_del(base.parse.parse_json(request.data))
    return jsonify(resp)

