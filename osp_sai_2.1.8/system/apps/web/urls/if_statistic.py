#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/if_statistic/<key>', methods=['GET', 'POST'])

def if_statistic_mgr_CURD(key):
    if request.method == 'GET':
        resp = api.if_statistic.get(key)
    elif request.method == 'POST':
        resp = api.if_statistic.clear(base.parse.parse_json(request.data))
    return jsonify(resp)



