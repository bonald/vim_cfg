#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/interface/<key>/', methods=['GET', 'PUT'])
@base.web_app.auth.login_required
def interface_mgr_CURD(key):
    if request.method == 'GET':
        resp = api.interface.get(key)
    elif request.method == 'PUT':
        resp = api.interface.modify(base.parse.parse_json(request.data))
    return jsonify(resp)

@base.web_app.app.route('/api/interface/<key>/enable/', methods=['PUT'])
@base.web_app.auth.login_required
def interface_enable_U(key):
    resp = api.interface.set_port_status(key, True)
    return jsonify(resp)

@base.web_app.app.route('/api/interface/<key>/disable/', methods=['PUT'])
@base.web_app.auth.login_required
def interface_disable_U(key):
    resp = api.interface.set_port_status(key, False)
    return jsonify(resp)

@base.web_app.app.route('/api/interface/flow/', methods=['PUT'])
@base.web_app.auth.login_required
def interface_flow_U():
	if request.method == 'GET':
		resp = api.interface.get_port_flow(base.parse.parse_json(request.data))
	elif request.method == 'PUT':
		resp = api.interface.set_port_flow(base.parse.parse_json(request.data))
	return jsonify(resp)

@base.web_app.app.route('/api/interface/split/', methods=['GET'])
@base.web_app.auth.login_required
def interface_split_CURD():
    if request.method == 'GET':
        resp = api.interface.get_port_split_status()
    return jsonify(resp)