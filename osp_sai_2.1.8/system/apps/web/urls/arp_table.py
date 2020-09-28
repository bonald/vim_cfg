#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on July 4, 2016

@author: huwx
'''

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/security/arp_table/', methods=['GET', 'POST', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def arp_table_CURD():
    if request.method == 'GET':
        resp = api.arp_table.get()
    elif request.method == 'PUT':
        resp = api.arp_table.add(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.arp_table.delete(base.parse.parse_json(request.data))
    elif request.method == 'POST':
        resp = api.arp_table.clear()
    return jsonify(resp)

@base.web_app.app.route('/api/security/arp_table/gratuitous_arp/', methods=['GET','PUT'])
@base.web_app.auth.login_required
def arp_table_gratuitous_CURD():
    if request.method == 'GET':
        resp = api.arp_table.gratuitous_arp_get()
    elif request.method == 'PUT':
        resp = api.arp_table.gratuitous_arp_set(base.parse.parse_json(request.data))
    return jsonify(resp)




