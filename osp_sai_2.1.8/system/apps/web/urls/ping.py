#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base
import api

@base.web_app.app.route('/api/ping/<int:count>/<vrf_id>/<dest_ip>/<src_ip>/<src_if>/', methods=['GET'])
@base.web_app.auth.login_required
def tools_ping_R(count, vrf_id, dest_ip,src_ip,src_if):
    if request.method == 'GET':
        resp = api.ping.get(count, vrf_id, dest_ip,src_ip,src_if)
    return jsonify(resp)
