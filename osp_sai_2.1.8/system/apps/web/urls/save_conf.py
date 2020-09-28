#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on Apr 27, 2016

@author: wangqj
'''

from flask import jsonify
import base
import api

@base.web_app.app.route('/api/save_conf/', methods=['PUT'])
@base.web_app.auth.login_required
def save_conf_U():
    resp = api.save_conf.update()
    return jsonify(resp)

