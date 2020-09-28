#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/websess/', methods=['DELETE','GET'])
@base.web_app.auth.login_required
def web_session_proc():
    if request.method == 'DELETE':
        resp = api.web_sessions.delete(base.parse.parse_json(request.data))
    else:
        resp = api.web_sessions.get(base.parse.parse_json(request.data))
    return jsonify(resp)
