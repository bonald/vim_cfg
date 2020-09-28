#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 9, 2016

@author: wangqj
'''
from flask import jsonify, request
import base
import api


@base.web_app.app.route('/api/ipv4_static/ipv4_static_route_set/', methods=['PUT'])
@base.web_app.auth.login_required
def ipv4_static_route_U():
    resp = api.ipv4_static.ipv4_static_route_set(base.parse.parse_json(request.data))
    return jsonify(resp)

@base.web_app.app.route('/api/ipv4_static/ipv4_static_route_delete/', methods=['DELETE'])
@base.web_app.auth.login_required
def ipv4_static_route_D():
    resp = api.ipv4_static.ipv4_static_route_delete(base.parse.parse_json(request.data))
    return jsonify(resp) 
