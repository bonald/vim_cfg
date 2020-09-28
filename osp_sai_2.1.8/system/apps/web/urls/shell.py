#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 9, 2016

@author: wangqj
'''

from flask import request, jsonify
import base
import api

@base.web_app.app.route('/api/shell/<cmd_base64>/', methods=['GET'])
@base.web_app.auth.login_required
def shell_R(cmd_base64):
    resp = api.shell.get(cmd_base64)
    return jsonify(resp)