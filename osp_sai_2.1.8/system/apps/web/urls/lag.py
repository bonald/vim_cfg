#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 9, 2016

@author: wangqj
'''
from flask import jsonify, request
import base
import api

@base.web_app.app.route('/api/lag/load_balance/', methods=['GET'])
@base.web_app.auth.login_required
def port_channel_R():
    resp = api.lag.load_balance_get()
    return jsonify(resp)

@base.web_app.app.route('/api/lag/getproduct/', methods=['GET'])
@base.web_app.auth.login_required
def product_get_R():
    resp = api.lag.product_get()
    return jsonify(resp)

@base.web_app.app.route('/api/lag/load_balance/<lb_type>/<lb_state>/', methods=['PUT'])
@base.web_app.auth.login_required
def port_channel_U(lb_type, lb_state):
    resp = api.lag.load_balance_set(lb_type, lb_state)
    return jsonify(resp)

@base.web_app.app.route('/api/lag/channel_group/', methods=['GET'])
@base.web_app.auth.login_required
def channel_group_R():
    resp = api.lag.channel_group_get('all')
    return jsonify(resp)

@base.web_app.app.route('/api/lag/channel_group/<int:gid>/', methods=['GET'])
@base.web_app.auth.login_required
def channel_group_R1(gid):
    resp = api.lag.channel_group_get(gid)
    return jsonify(resp)

@base.web_app.app.route('/api/lag/channel_group/<int:gid>/<int:agg_mode_seclected>/<int:channel_mode>/', methods=['PUT'])
@base.web_app.auth.login_required
def channel_group_U(gid,agg_mode_seclected,channel_mode):
    resp = api.lag.channel_group_set(gid,agg_mode_seclected,channel_mode,base.parse.parse_json(request.data))
    return jsonify(resp)

@base.web_app.app.route('/api/lag/channel_group/', methods=['DELETE'])
@base.web_app.auth.login_required
def channel_group_D():
    resp = api.lag.channel_group_delete(base.parse.parse_json(request.data))
    return jsonify(resp)

@base.web_app.app.route('/api/lag/channel_agg_mode/<int:gid>/', methods=['GET', 'PUT'])
@base.web_app.auth.login_required
def channel_agg_mode_U(gid):
    if request.method == 'GET':
        resp = api.lag.channel_agg_mode_get(gid)
    elif request.method == 'PUT':
        resp = api.lag.channel_agg_mode_set(gid, base.parse.parse_json(request.data))

    return jsonify(resp)

@base.web_app.app.route('/api/lag/load_balance_tunnel_hash/', methods=['GET'])
@base.web_app.auth.login_required
def port_channel_tunnel_hash_R():
    resp = api.lag.load_balance_tunnel_hash_get()
    return jsonify(resp)

@base.web_app.app.route('/api/lag/update_load_balance_tunnel_hash/<hashtype>/', methods=['SET'])
@base.web_app.auth.login_required
def port_channel_tunnel_hash_U(hashtype):
    resp = api.lag.load_balance_tunnel_hash_set(hashtype)
    return jsonify(resp)

@base.web_app.app.route('/api/lag/hash_gre/<key>/<protocol>/<ipda>/<ipsa>/<srcinterface>/', methods=['PUT'])
@base.web_app.auth.login_required
def set_hash_gre(key, protocol, ipda, ipsa, srcinterface):
    resp = api.lag.load_balance_set_gre_hash(key, protocol, ipda, ipsa, srcinterface)
    return jsonify(resp)

@base.web_app.app.route('/api/lag/hash_l2/<type>/<macda>/<macsa>/<srcinterface>/<vlan>/', methods=['PUT'])
@base.web_app.auth.login_required
def set_hash_l2(type, macda, macsa, srcinterface, vlan):
    resp = api.lag.load_balance_set_l2_hash(type, macda, macsa, srcinterface, vlan)
    return jsonify(resp)

@base.web_app.app.route('/api/lag/hash_ip/<destport>/<sourceport>/<ipda>/<ipsa>/<ip_protocol>/<srcinterface>/', methods=['PUT'])
@base.web_app.auth.login_required
def set_hash_ip(destport, sourceport, ipda, ipsa, ip_protocol, srcinterface):
    resp = api.lag.load_balance_set_ip_hash(destport, sourceport, ipda, ipsa, ip_protocol, srcinterface)
    return jsonify(resp)

@base.web_app.app.route('/api/lag/hash_ipv6/<destport>/<sourceport>/<ipda>/<ipsa>/<ip_protocol>/<srcinterface>/', methods=['PUT'])
@base.web_app.auth.login_required
def set_hash_ipv6(destport, sourceport, ipda, ipsa, ip_protocol, srcinterface):
    resp = api.lag.load_balance_set_ipv6_hash(destport, sourceport, ipda, ipsa, ip_protocol, srcinterface)
    return jsonify(resp)

@base.web_app.app.route('/api/lag/hash_mpls/<label2nd>/<label3rd>/<srcinterface>/<toplabel>/', methods=['PUT'])
@base.web_app.auth.login_required
def set_hash_mpls(label2nd, label3rd, srcinterface, toplabel):
    resp = api.lag.load_balance_set_mpls_hash(label2nd, label3rd, srcinterface,toplabel)
    return jsonify(resp)

@base.web_app.app.route('/api/lag/hash_nvgre/<innerl2type>/<innerl2macda>/<innerl2macsa>/<innerl3ip_protocol>/<innerl3ipda>/<innerl3ipsa>/<outergre_protocol>/<outeripda>/<outeripsa>/<vsid>/<srcinterface>/<nvgreval>/', methods=['PUT'])
@base.web_app.auth.login_required
def set_hash_nvgre(innerl2type, innerl2macda, innerl2macsa, innerl3ip_protocol, innerl3ipda, innerl3ipsa, outergre_protocol, outeripda, outeripsa, vsid, srcinterface,nvgreval):
    resp = api.lag.load_balance_set_nvgre_hash(innerl2type, innerl2macda, innerl2macsa, innerl3ip_protocol, innerl3ipda, innerl3ipsa, outergre_protocol, outeripda, outeripsa, vsid, srcinterface,nvgreval)
    return jsonify(resp)

@base.web_app.app.route('/api/lag/hash_vxlan/<innerl2type>/<innerl2macda>/<innerl2macsa>/<innerl3destport>/<innerl3ip_protocol>/<innerl3ipda>/<innerl3ipsa>/<innerl3sourceport>/<outer_destport>/<outeripda>/<outeripsa>/<outer_sourceport>/<vlan>/<srcinterface>/<vni>/<vxlanval>/', methods=['PUT'])
@base.web_app.auth.login_required
def set_hash_vxlan(innerl2type, innerl2macda, innerl2macsa, innerl3destport, innerl3ip_protocol, innerl3ipda, innerl3ipsa, innerl3sourceport, outer_destport, outeripda, outeripsa, outer_sourceport, vlan, srcinterface, vni,vxlanval):
    resp = api.lag.load_balance_set_vxlan_hash(innerl2type, innerl2macda, innerl2macsa, innerl3destport, innerl3ip_protocol, innerl3ipda, innerl3ipsa, innerl3sourceport, outer_destport, outeripda, outeripsa, outer_sourceport, vlan, srcinterface, vni,vxlanval)
    return jsonify(resp)

@base.web_app.app.route('/api/lag/hash_ip_enable/<ip_enable>/', methods=['PUT'])
@base.web_app.auth.login_required
def set_hash_ip_enable(ip_enable):
    resp = api.lag.load_balance_set_hash_ip_enable(ip_enable)
    return jsonify(resp)

@base.web_app.app.route('/api/lag/hash_ipv6_enable/<ipv6_enable>/', methods=['PUT'])
@base.web_app.auth.login_required
def set_hash_ipv6_enable(ipv6_enable):
    resp = api.lag.load_balance_set_hash_ipv6_enable(ipv6_enable)
    return jsonify(resp)

@base.web_app.app.route('/api/lag/hash_mpls_enable/<mpls_enable>/', methods=['PUT'])
@base.web_app.auth.login_required
def set_hash_mpls_enable(mpls_enable):
    resp = api.lag.load_balance_set_hash_mpls_enable(mpls_enable)
    return jsonify(resp)
