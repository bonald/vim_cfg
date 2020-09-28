#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify, redirect, session
from werkzeug import secure_filename
import base, api

@base.web_app.app.route('/api/boot/', methods=['GET'])
@base.web_app.auth.login_required
def boot_R():
    resp = api.boot.boot_info()
    return jsonify(resp)

@base.web_app.app.route('/api/boot/reboot/<is_save>/', methods=['GET'])
@base.web_app.auth.login_required
def boot_mgr_reboot_R(is_save):
    resp = api.boot.reboot(is_save)
    return jsonify(resp)

@base.web_app.app.route('/api/boot/<filename>/', methods=['PUT'])
@base.web_app.auth.login_required
def boot_mgr_U(filename):
    resp = api.boot.set_next_boot_img(filename)
    return jsonify(resp)

@base.web_app.app.route('/api/boot/delete_if_failed/<filename>/', methods=['PUT'])
@base.web_app.auth.login_required
def boot_mgr_delete_if_failed_U(filename):
    resp = api.boot.set_next_boot_img(filename)
    if resp.get('error'):
        api.file_mgr.delete({'file_arr': [{'type': 'image', 'filename': filename}]})
    return jsonify(resp)


# set <filename> as next boot start-config.conf
@base.web_app.app.route('/api/boot/startup_conf/<filename>/', methods=['PUT'])
@base.web_app.auth.login_required
def boot_mgr_start_conf_U(filename):
    resp = api.boot.set_next_boot_startup_conf(filename)
    return jsonify(resp)


@base.web_app.app.route('/api/boot/upload_image/', methods=['POST'])
@base.web_app.auth.login_required
def boot_upload_C():
    resp = api.boot.upload_image(request.files.get('file'))
    print resp
    return jsonify(resp)

@base.web_app.app.route('/api/flash/upload_files/', methods=['POST'])
@base.web_app.auth.login_required
def upload_file2flash_C():
    resp = api.boot.upload_flash_files(request.files.get('file'))
    return jsonify(resp)


@base.web_app.app.route('/boot/upload_image_legacy/', methods=['POST'])
def boot_upload_legacy_C():
    # verify login
    token = request.form.get('token')
    username = request.form.get('username')
    session_token = session.get(username)
    fil = request.files.get('file')
    if not token or not username or not session_token.get('token') or session_token.get('token') != token:
        return redirect('/static/switch/index.html#/login') 
    
    # save file
    resp = api.boot.upload_image(fil)
    if resp.get('error'):
        return resp.get('err_reason')
    else: # set next boot image
        resp = api.boot.set_next_boot_img(secure_filename(fil.filename))
        if resp.get('error'):
            # delete image
            api.file_mgr.delete({'file_arr': [{'type': 'image', 'filename': secure_filename(fil.filename)}]})
            return resp.get('err_reason')
        else: # reboot
            return redirect('/static/switch/index.html#/home/reboot_and_save') 
            
    
    
    
    
    
