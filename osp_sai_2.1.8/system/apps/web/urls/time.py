#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/time/ntp_server/', methods=['GET', 'POST', 'PUT', 'DELETE'])
@base.web_app.auth.login_required
def clock_ntp_cfg_CURD():
    if request.method == 'GET':
        resp = api.time.ntp_srv_get()
    elif request.method == 'DELETE':
        resp = api.time.ntp_srv_del(base.parse.parse_json(request.data).get('ntp_srv_table'))
    elif request.method == 'PUT':
        resp = api.time.ntp_srv_add(base.parse.parse_json(request.data))
    return jsonify(resp)

@base.web_app.app.route('/api/time/current_time/', methods=['GET', 'PUT'])
@base.web_app.auth.login_required
def clock_current_time_CURD():
    if request.method == 'GET':
        resp = api.time.clock_current_time_get()
    elif request.method == 'PUT':
        resp = api.time.clock_current_time_set(base.parse.parse_json(request.data))        
    return jsonify(resp)       

@base.web_app.app.route('/api/time/timestap_sync/', methods=['GET', 'PUT'])
@base.web_app.auth.login_required
def clock_timestamp_sync_CURD():
    if request.method == 'GET':
        resp = api.time.clock_timestamp_sync_get()
    elif request.method == 'PUT':
        resp = api.time.clock_timestamp_sync_set(base.parse.parse_json(request.data).get('enable'))        
    return jsonify(resp)        