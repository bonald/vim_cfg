#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 9, 2016

@author: wangqj
'''
from flask import jsonify, request
import base
import api

@base.web_app.app.route('/api/ipv4_routing/<key>/', methods=['GET'])
@base.web_app.auth.login_required
def ipv4_routing_R(key):
    resp = api.ipv4_routing.ipv4_routing_get(key)
    return jsonify(resp)

 