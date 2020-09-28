#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 3, 2016

@author: wangqj
'''

from flask import request, jsonify
import base
import api

@base.web_app.app.route('/api/manage_if/', methods=['GET'])
@base.web_app.auth.login_required
def manage_if_R():
    resp = api.manage_if.get()
    return jsonify(resp)

@base.web_app.app.route('/api/manage_if/ip/<base64_ip_addr>/', methods=['PUT'])
@base.web_app.auth.login_required
def manage_if_IP_U(base64_ip_addr):
    resp = api.manage_if.update_ip(base.parse.parse_base64(base64_ip_addr))
    return jsonify(resp)

@base.web_app.app.route('/api/manage_if/gateway/<base64_gateway>/', methods=['PUT'])
@base.web_app.auth.login_required
def manage_if_gateway_U(base64_gateway):
    resp = api.manage_if.update_gateway(base.parse.parse_base64(base64_gateway))
    return jsonify(resp)