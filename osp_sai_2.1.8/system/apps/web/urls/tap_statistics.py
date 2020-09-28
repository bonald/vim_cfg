#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, jsonify
import base, api

@base.web_app.app.route('/api/tap_statistics/all/', methods=['GET'])
def tap_port_statistics():
    resp = api.tap_statistics.get()
    return jsonify(resp)


@base.web_app.app.route('/api/tap_statistics/ifname/<key>', methods=['GET'])
def flow_statistics_one(key):
    resp = api.tap_statistics.get_flow_statistics(key)
    return jsonify(resp)

@base.web_app.app.route('/api/tap_statistics/ifname/<key>/<subkey>', methods=['GET'])
def flow_statistics_split_port(key, subkey):
    ifname = key + '/' + subkey
    resp = api.tap_statistics.get_flow_statistics(ifname)
    return jsonify(resp)