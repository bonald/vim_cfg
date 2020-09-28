
#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/cmd_api/', methods=['POST'])
#@base.web_app.auth.login_required
def cmd_api_CURD():

    if request.method == 'POST':
        resp = api.cmd_api.command(base.parse.parse_json(request.data), request.authorization, request)

    return jsonify(resp)