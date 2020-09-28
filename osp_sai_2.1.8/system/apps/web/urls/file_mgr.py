#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/file_mgr/', methods=['DELETE'])
@base.web_app.auth.login_required
def sys_mgr_file_mgr_D():
    if request.method == 'DELETE':
        resp = api.file_mgr.delete(base.parse.parse_json(request.data))
    return jsonify(resp)

@base.web_app.app.route('/api/file_mgr/<file_type>/', methods=['GET'])
@base.web_app.auth.login_required
def sys_mgr_file_mgr_R(file_type):
    resp = api.file_mgr.get(file_type)
    return jsonify(resp)

@base.web_app.app.route('/api/file_mgr/<file_type>/<filename>/<backup_filename>/', methods=['PUT'])
@base.web_app.auth.login_required
def sys_mgr_file_mgr_U(file_type, filename, backup_filename):
    resp = api.file_mgr.backup(file_type, filename, backup_filename)
    return jsonify(resp)
