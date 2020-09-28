#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/user_management/', methods=['GET','ADD','PUT'])
#@base.web_app.auth.login_required
def web_user_management_proc():
    if request.method == 'ADD':
        resp = api.user_management.add(base.parse.parse_json(request.data))
    if request.method == 'GET':
        resp = api.user_management.get(base.parse.parse_json(request.data))
    if request.method == 'PUT':
        resp = api.user_management.set(base.parse.parse_json(request.data))
    return jsonify(resp)
    
@base.web_app.app.route('/api/user_management/<username>/', methods=['Delete'])
#@base.web_app.auth.login_required
def web_user_management_proc_D(username):
    resp = api.user_management.delete(base.parse.parse_string(username))
    return jsonify(resp)