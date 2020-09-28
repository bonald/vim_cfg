#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/transceiver/<key>/', methods=['GET', 'PUT'])
@base.web_app.auth.login_required
def transceiver_mgr_CURD(key):
    if request.method == 'GET':
        resp = api.transceiver.get(key)
    elif request.method == 'PUT':
        resp = api.transceiver.modify(base.parse.parse_json(request.data))
    return jsonify(resp)
