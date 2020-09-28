#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on Apr 27, 2016

@author: wangqj
'''
from flask import jsonify
import base, vcl
import os
import time

@base.web_app.app.route('/api/server_state/', methods=['GET'])
#@base.web_app.auth.login_required
def server_state_R():
    
    obj = {'state': False}
    
    if vcl.config.is_dev: # remote
        res = os.system('ping %s -w 1000 -n 1' % vcl.config.remote)
        if res == 0:
            time.sleep(10)
            obj = {'state': True}
    else: # local
        obj = {'state': True}
    
    return jsonify(obj)