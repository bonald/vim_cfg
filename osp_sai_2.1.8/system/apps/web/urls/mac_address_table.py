#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on Apr 28, 2016

@author: wangqj
'''

from flask import jsonify, request
import base
import api

@base.web_app.app.route('/api/mac_address_table/', methods=['GET'])
@base.web_app.auth.login_required
def mac_address_table_R():
    resp = api.mac_address_table.get()
    return jsonify(resp)

@base.web_app.app.route('/api/mac_address_table/', methods=['DELETE'])
@base.web_app.auth.login_required
def mac_address_table_D():
    resp = api.mac_address_table.delete(base.parse.parse_json(request.data).get('mac_address_table'))
    return jsonify(resp)

@base.web_app.app.route('/api/mac_address_table/', methods=['POST'])
@base.web_app.auth.login_required
def mac_address_table_C():
    resp = api.mac_address_table.add(base.parse.parse_json(request.data))
    return jsonify(resp)

@base.web_app.app.route('/api/mac_address_table/delete_all', methods=['DELETE'])
@base.web_app.auth.login_required
def mac_address_table_delete_all_D():
    resp = api.mac_address_table.delete_all()
    return jsonify(resp)    


