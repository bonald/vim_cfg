#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on July 12, 2016

@author: huwx
'''

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/security/ddos_prevent/', methods=['GET', 'POST', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def ddos_prevent_table_CURD():
    if request.method == 'GET':
        resp = api.ddos_prevent.get()
    elif request.method == 'PUT':
        resp = api.ddos_prevent.modify(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.ddos_prevent.delete(base.parse.parse_json(request.data))
    elif request.method == 'POST':
        resp = api.ddos_prevent.clear()
    return jsonify(resp)






