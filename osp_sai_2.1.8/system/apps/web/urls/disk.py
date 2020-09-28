#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on Apr 25, 2016

@author: wangqj
'''

from flask import request, jsonify
import base
import api

@base.web_app.app.route('/api/disk/', methods=['GET'])
@base.web_app.auth.login_required
def update_R():
    resp = api.disk.get()
    return jsonify(resp)
