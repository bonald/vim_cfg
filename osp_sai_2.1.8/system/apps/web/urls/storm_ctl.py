#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/storm_ctl/<key>/', methods=['GET', 'POST', 'PUT', 'DELETE'])
#@base.web_app.auth.login_required
def storm_ctl_mgr_CURD(key):
    if request.method == 'GET':
        resp = api.storm_ctl.get(key)
    elif request.method == 'POST':
        resp = api.storm_ctl.add(base.parse.parse_json(request.data))
    elif request.method == 'PUT':
        resp = api.storm_ctl.modify(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.storm_ctl.delete(base.parse.parse_json(request.data))
    return jsonify(resp)



