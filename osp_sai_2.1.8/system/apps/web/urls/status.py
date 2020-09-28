#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/fan/', methods=['GET'])
@base.web_app.auth.login_required
def status_fan_CURD():
    if request.method == 'GET':
        resp = api.status.fanGet()
    return jsonify(resp)

@base.web_app.app.route('/api/psu/', methods=['GET'])
@base.web_app.auth.login_required
def status_psu_CURD():
    if request.method == 'GET':
        resp = api.status.psuGet()
    return jsonify(resp)

@base.web_app.app.route('/api/sensor/', methods=['GET'])
@base.web_app.auth.login_required
def status_sensor_CURD():
    if request.method == 'GET':
        resp = api.status.sensorGet()
    return jsonify(resp)

@base.web_app.app.route('/api/card/', methods=['GET'])
@base.web_app.auth.login_required
def status_card_CURD():
    if request.method == 'GET':
        resp = api.status.cardGet()
    return jsonify(resp)

@base.web_app.app.route('/api/version/', methods=['GET'])
@base.web_app.auth.login_required
def status_version_CURD():
    if request.method == 'GET':
        resp = api.status.versionGet()
    return jsonify(resp)

@base.web_app.app.route('/api/bootimage/', methods=['GET'])
@base.web_app.auth.login_required
def status_bootimage_CURD():
    if request.method == 'GET':
        resp = api.status.bootimageGet()
    return jsonify(resp)

@base.web_app.app.route('/api/sysglobal/', methods=['GET'])
@base.web_app.auth.login_required
def status_sysglobal_CURD():
    if request.method == 'GET':
        resp = api.status.sysglobalGet()
    return jsonify(resp)

@base.web_app.app.route('/api/memory/', methods=['GET'])
@base.web_app.auth.login_required
def status_memoryUsage_CURD():
    if request.method == 'GET':
        resp = api.status.memoryGet()
    return jsonify(resp)

@base.web_app.app.route('/api/cpu/', methods=['GET'])
@base.web_app.auth.login_required
def status_cpuUsage_CURD():
    if request.method == 'GET':
        resp = api.status.cpuGet()
    return jsonify(resp)

@base.web_app.app.route('/api/uptime/', methods=['GET'])
@base.web_app.auth.login_required
def status_uptime_CURD():
    if request.method == 'GET':
        resp = api.status.uptimeGet()
    return jsonify(resp)


@base.web_app.app.route('/api/dev_status/', methods=['GET'])
@base.web_app.auth.login_required
def status_all_R():
    resp = {'error': False, 'err_code': 0, 'err_reason': ''}
    fun_list = {
        'fan': api.status.fanGet,
        'psu': api.status.psuGet,
        'sensor': api.status.sensorGet,
        'card': api.status.cardGet,
        'version': api.status.versionGet,
        'boot_image': api.status.bootimageGet,
        'sys_global': api.status.sysglobalGet,
        'memory_usage': api.status.memoryGet,
        'cpu_usage': api.status.cpuGet,
        'uptime': api.status.uptimeGet,
    }
    
    for key in fun_list.keys():
        resp_tmp = fun_list[key]()
        if resp_tmp['error']:
            resp['error'] = resp_tmp['error']
            resp['err_code'] = resp_tmp['err_code']
            resp['err_reason'] = resp_tmp['err_reason']
            return jsonify(resp)
        resp[key] = resp_tmp
    
    return jsonify(resp)



