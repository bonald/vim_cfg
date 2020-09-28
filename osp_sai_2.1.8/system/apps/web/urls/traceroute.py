#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base
import api

@base.web_app.app.route('/api/traceroute/<vrf_id>/<dest_ip>/<int:timeout>/', methods=['GET'])
@base.web_app.auth.login_required
def tools_traceroute_R(vrf_id, dest_ip, timeout):
    if request.method == 'GET':
        resp = api.traceroute.get(vrf_id, dest_ip, timeout)
    return jsonify(resp)
