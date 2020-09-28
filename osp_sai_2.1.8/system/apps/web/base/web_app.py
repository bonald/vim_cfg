#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import Flask, jsonify, redirect, url_for, request
from datetime import timedelta
import json
import auth, config
import re

import vcl

app = Flask(import_name=__name__, static_folder='../static')
app.secret_key = config.secret_key
app.permanent_session_lifetime = timedelta(minutes=config.token_live_time)
auth = auth.ApiAuth()

@app.before_request
def before_request():
    if vcl.config.is_dev == False:
        url = str(request.url).replace(request.host_url, '')
        is_api = True if re.match(r'^api/', url) else False
        if is_api:
            auth.update_session_tbl();

@app.route('/')
@auth.login_required
def index():
    return redirect(url_for('static', filename='switch/index.html'))


@app.route('/api/token/', methods=['POST'])
def make_token():
    resp = auth.make_token(json.loads(request.data));
    return jsonify(resp)

@app.route('/api/token/logout', methods=['POST'])
def del_token():
    resp = auth.del_token(json.loads(request.data));
    return jsonify(resp)

@app.route('/api/product/', methods=['GET'])
def get_product_R():
    resp = auth.get_product();
    print resp
    return jsonify(resp)


if __name__ == '__main__':
    pass
