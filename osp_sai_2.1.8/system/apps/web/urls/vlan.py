#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/vlan/', methods=['GET', 'POST', 'PUT', 'DELETE'])
#@base.web_app.auth.login_required
def srv_mgr_vlan_CURD():
    if request.method == 'GET':
        resp = api.vlan.status()
    elif request.method == 'POST':
        resp = api.vlan.add(base.parse.parse_json(request.data))
    elif request.method == 'PUT':
        resp = api.vlan.modify(base.parse.parse_json(request.data))
    elif request.method == 'DELETE':
        resp = api.vlan.delete(base.parse.parse_json(request.data))
    return jsonify(resp)

@base.web_app.app.route('/api/vlan/range/', methods=['POST', 'DELETE'])
@base.web_app.auth.login_required
def srv_mgr_vlan_range_CD():
    if request.method == 'POST':
        resp = api.vlan.range_add(base.parse.parse_json(request.data))
        pass
    elif request.method == 'DELETE':
        resp = api.vlan.range_delete(base.parse.parse_json(request.data))
        pass
    
    return jsonify(resp)

@base.web_app.app.route('/api/vlan/access/', methods=['PUT'])
@base.web_app.auth.login_required
def vlan_access_U():
    resp = api.vlan.access_update(base.parse.parse_json(request.data))
    return jsonify(resp)

@base.web_app.app.route('/api/vlan/trunk/', methods=['PUT'])
@base.web_app.auth.login_required
def vlan_trunk_U():
    resp = api.vlan.trunk_update(base.parse.parse_json(request.data))
    return jsonify(resp)
