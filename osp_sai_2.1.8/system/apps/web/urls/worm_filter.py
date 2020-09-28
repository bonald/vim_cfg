#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on July 4, 2016

@author: huwx
'''

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/security/worm_filter/', methods=['GET', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def worm_filter__CURD():
    if request.method == 'GET':
        resp = api.worm_filter.get()
    elif request.method == 'PUT':
        resp = api.worm_filter.add(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.worm_filter.delete(base.parse.parse_json(request.data))
    return jsonify(resp)




