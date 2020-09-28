#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 3, 2016

@author: wangqj
'''

from flask import request, jsonify
import base
import api

@base.web_app.app.route('/api/sys_global/', methods=['GET'])
@base.web_app.auth.login_required
def sys_global_R():
    resp = api.sys_global.get()
    return jsonify(resp)

@base.web_app.app.route('/api/sys_global/', methods=['PUT'])
@base.web_app.auth.login_required
def sys_global_U():
    resp = api.sys_global.update(base.parse.parse_json(request.data))
    return jsonify(resp)
