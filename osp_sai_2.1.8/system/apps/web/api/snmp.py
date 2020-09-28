#!/usr/bin/python
#-*- coding: utf-8 -*-

from vcl import vcmd
import string
import base64
import os

def snmp_cfg_get():
    """
    TBD
    """
    _err_reason = [
            '',                    # err_code: 0
            'get snmp cfg error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    arr = vcmd.get_arr('cdbctl read/cdb/app/snmp_cfg/')
    if arr:
        obj['cfg'] = arr[0]
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]

    return obj

def snmp_mgr_update_status(type, enable):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    if type =="server_enable":
        vcmd.get_status_lines('cdbctl update/cdb/app/snmp_cfg/server_enable/%d' % int(enable))

    if type =="trap_enable":
        vcmd.get_status_lines('cdbctl update/cdb/app/snmp_cfg/trap_enable_coldstart/%d' % int(enable))
        vcmd.get_status_lines('cdbctl update/cdb/app/snmp_cfg/trap_enable_warmstart/%d' % int(enable))
        vcmd.get_status_lines('cdbctl update/cdb/app/snmp_cfg/trap_enable_linkdown/%d' % int(enable))
        vcmd.get_status_lines('cdbctl update/cdb/app/snmp_cfg/trap_enable_linkup/%d' % int(enable))
        vcmd.get_status_lines('cdbctl update/cdb/app/snmp_cfg/trap_enable_system/%d' % int(enable))
        vcmd.get_status_lines('cdbctl update/cdb/app/snmp_cfg/trap_enable_vrrp/%d' %int(enable))

    return obj

def snmp_mgr_update_version(version):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    vcmd.get_status_lines('cdbctl update/cdb/app/snmp_cfg/version/%s' % str(version))

    return obj    

def snmp_target_addr_set(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    ipaddr = str(req_data.get('ipaddr'))
    commu = str(req_data.get('commu'))
    addr = str(req_data.get('addr'))
    if addr == "mgmt":
       NS_BAND = "outband"
    else:
      NS_BAND = "inband"

    if False == os.path.isfile('/switch/etc/cmd/tap_group.xml'):
        cmd = 'cdbctl update/cdb/l3/check_mgmt_if_ip/%s' % ipaddr
        status, lines = vcmd.get_status_lines(cmd)
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 1;
            obj['err_reason'] = _err_reason[1];
            return obj;

    cmd = 'cdbctl create/cdb/sys/ns_route/1#162#%s/5/%s/' % (ipaddr , NS_BAND)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    cmd = 'cdbctl create/cdb/app/snmp_trap/%s/%s/162'  % (ipaddr, commu)
    status, lines = vcmd.get_status_lines(cmd)   
    if status != 0:

        # rollback process  
        cmd = 'cdbctl create/cdb/app/snmp_trap/%s/public/162' % ipaddr  
        vcmd.get_status_lines(cmd)

        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj; 
    else:
        cmd = 'cdbctl update/cdb/app/snmp_trap/%s/%s/162/%s/' % (ipaddr, commu, NS_BAND)
        vcmd.get_status_lines(cmd)       

    return obj  

def snmp_target_addr_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    ipaddr = str(req_data.get('trapaddr'))
    commu = str(req_data.get('community'))
    addr = str(req_data.get('vrf'))

    if addr == "mgmt":
       NS_BAND = "outband"
    else:
        NS_BAND = "inband"

    cmd = 'cdbctl delete/cdb/app/snmp_trap/%s/%s/162/%s' % (ipaddr, commu, NS_BAND)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    cmd = 'cdbctl delete/cdb/sys/ns_route/1#162#%s/%s' %(ipaddr, NS_BAND)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    cmd = 'cdbctl delete/cdb/sys/inband_snat/1#162#%s' % (ipaddr)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj

def snmp_target_addr_get():
    _err_reason = [
            '',                    # err_code: 0
            'get snmp cfg error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    cmd = 'cdbctl read/cdb/app/snmp_trap'
    arr = vcmd.get_arr(cmd)
    if arr:
        for entry in arr:

            key = entry['key']
            key = key[:(len(key) - 1)]
            sublist  = key.split(',')

            entry.pop('key')
            entry['trapaddr'] = sublist[0]
            entry['community'] = sublist[1]
            entry['port'] = sublist[2]

            if entry['is_inband'] == '1':
                entry['vrf'] = 'default'
            elif entry['is_inband'] == '0':
                entry['vrf'] = 'mgmt'

        obj['cfg'] = arr
    else:
        obj['cfg'] = []

    return obj

def snmp_context_get():
    _err_reason = [
            '',                    # err_code: 0
            'get snmp cfg error',  # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    cmd = 'cdbctl read/cdb/app/snmp_context'

    arr = vcmd.get_arr(cmd)
    if arr:
        for entry in arr:
            key = entry['key']
            key = key[:(len(key))]
            sublist = key.split(',')

            entry.pop('key')
            entry['context'] = sublist[0]

        obj['cfg'] = arr
    else:
        obj['cfg'] = []

    return obj

def snmp_context_set(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    contnama = str(req_data.get('contnama'))

    cmd = 'cdbctl create/cdb/app/snmp_context/%s' % (contnama)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj

def  snmp_context_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    contnama = str(req_data.get('context'))

    cmd = 'cdbctl delete/cdb/app/snmp_context/%s' % (contnama)
    status, lines = vcmd.get_status_lines(cmd)

    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj

def snmp_usm_user_get():
    _err_reason = [
            '',                    # err_code: 0
            'get snmp cfg error',  # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    cmd = 'cdbctl read/cdb/app/snmp_usm_user'

    arr = vcmd.get_arr(cmd)
    if arr:
        for entry in arr:
            # key = entry['key']
            # key = key[:(len(key) - 1)]

            # entry.pop('key')
            entry['username'] = entry['key']
            if entry['auth_type'] == '1':
                entry['authensel'] = 'md5'
            elif entry['auth_type'] == '2':
                entry['authensel'] = 'sha'

            entry['authenpwd'] = entry['auth_pwd']

            if entry['priv_type'] == '1':
                entry['encrysel'] = 'des'
            elif entry['priv_type'] == '2':
                entry['encrysel'] = 'aes'
            entry['encrypwd'] = entry['priv_pwd']

        obj['cfg'] = arr
    else:
        obj['cfg'] = []

    return obj

def snmp_usm_user_set(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    usmuser = str(req_data.get('usmuser'))
    authensel = str(req_data.get('authensel'))
    authen = str(req_data.get('authen'))
    encrypsel = str(req_data.get('encrypsel'))
    encry = str(req_data.get('encry'))

    cmd = 'cdbctl create/cdb/app/snmp_usm_user/%s/remote//authentication/%s/auth_pwd//"%s"/privacy/%s/pri_pwd//"%s"/' % (usmuser, authensel, authen, encrypsel, encry)
    print cmd
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj

def snmp_usm_user_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    usmuser = str(req_data.get('username'))

    cmd = 'cdbctl delete/cdb/app/snmp_usm_user/%s' % (usmuser)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj

def snmp_notify_get():
    _err_reason = [
            '',                    # err_code: 0
            'get snmp cfg error',  # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    cmd = 'cdbctl read/cdb/app/snmp_notify'

    arr = vcmd.get_arr(cmd)
    if arr:
        for entry in arr:
            #key = entry['key']
            #key = key[:(len(key))]
            #sublist = key.split(',')

            #entry.pop('key')
            entry['notify'] = entry['key']

            entry['tag'] = entry['tag_name']

            if entry['notify_type'] == '0':
                entry['type'] = 'trap'
            elif entry['notify_type'] == '1':
                entry['type'] = 'inform'

        obj['cfg'] = arr
    else:
        obj['cfg'] = []
    return obj

def snmp_notify_set(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    notiname = str(req_data.get('notiname'))
    tagname = str(req_data.get('tagname'))
    notitype = str(req_data.get('notitype'))

    cmd = 'cdbctl create/cdb/app/snmp_notify/%s/tag/%s/%s/' % (notiname, tagname, notitype)

    print cmd
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj

def snmp_notify_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    notiname = str(req_data.get('notify'))

    cmd = 'cdbctl delete/cdb/app/snmp_notify/%s' % (notiname)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj

def snmp_target_params_get():
    _err_reason = [
            '',                    # err_code: 0
            'get snmp cfg error',  # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    cmd = 'cdbctl read/cdb/app/snmp_target_params'

    arr = vcmd.get_arr(cmd)
    if arr:
        for entry in arr:
            # key = entry['key']
            # key = key[:(len(key))]
            # sublist = key.split(',')

            # entry.pop('key')
            entry['params'] = entry['key']

            entry['username'] = entry['user_name']

            if entry['usm_level'] == '1':
                entry['selpar'] = 'noauth'
            elif entry['usm_level'] == '2':
                entry['selpar'] = 'auth'
            elif entry['usm_level'] == '3':
                entry['selpar'] = 'priv'

        obj['cfg'] = arr
    else:
        obj['cfg'] = []

    return obj

def snmp_target_params_set(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    params = str(req_data.get('params'))
    username = str(req_data.get('username'))
    paramsel = str(req_data.get('paramsel'))

    cmd = 'cdbctl create/cdb/app/snmp_target_params/%s/user/%s/%s/' % (params, username, paramsel)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj

def snmp_target_params_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    params = str(req_data.get('params'))

    cmd = 'cdbctl delete/cdb/app/snmp_target_params/%s' % (params)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj

def snmp_target_address_get():
    _err_reason = [
            '',                    # err_code: 0
            'get snmp cfg error',  # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    cmd = 'cdbctl read/cdb/app/snmp_target_addr'

    arr = vcmd.get_arr(cmd)

    if arr:
        for entry in arr:

            entry['trapaddress'] = entry['key']
            entry['communitys'] = entry['para_name']
            entry['server'] = entry['ip_addr']
            entry['traname'] = entry['tag_list']

        obj['cfg'] = arr
    else:
        obj['cfg'] = []

    return obj

def snmp_target_address_set(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    name = str(req_data.get('name'))
    params = str(req_data.get('params'))
    ipaddr = str(req_data.get('ipaddr'))
    tagname = str(req_data.get('tagname'))
    print(req_data)
    cmd = 'cdbctl update/cdb/l3/check_mgmt_if_ip/%s' % (ipaddr)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    cmd = 'cdbctl create/cdb/sys/ns_route/1#162#%s/5/outband/' % (ipaddr)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    cmd = 'cdbctl create/cdb/app/snmp_target_addr/%s/param/%s/mgmt_if/mgmt-if/ip_addr/%s/udp_port/162/timeout/2/retries/3/taglist/%s' % (name, params, ipaddr, tagname)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        cmd = 'cdbctl delete/cdb/sys/ns_route/1#162#%s/5/outband/' % (ipaddr)
        status, lines = vcmd.get_status_lines(cmd)
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj

def snmp_target_address_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    name = str(req_data.get('trapaddress'))
    ipaddr = str(req_data.get('server'))
    print(name)
    print(ipaddr)
    cmd = 'cdbctl delete/cdb/app/snmp_target_addr/%s/mgmt_if/mgmt_if/ip_addr/%s/udp_port/162/' % (name , ipaddr)
    print cmd
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    cmd = 'cdbctl delete/cdb/sys/ns_route/1#162#%s/outband/' % (ipaddr)
    print cmd
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj

def snmp_community_get():
    _err_reason = [
            '',                    # err_code: 0
            'get snmp cfg error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    arr = vcmd.get_arr('cdbctl read/cdb/app/snmp_community')
    if arr:
        obj['cfg'] = arr
    else:
        obj['cfg'] = []

    return obj

def snmp_community_set(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'Community Configuration repeat',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    name = str(req_data.get('name'))
    access = str(req_data.get('access'))

    cmd = 'cdbctl create/cdb/app/snmp_community/%s/type/%s' % (name, access)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj  

def snmp_community_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            'cannot set interface', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    name = str(req_data.get('key'))
    access = str(req_data.get('type'))

    cmd = 'cdbctl delete/cdb/app/snmp_community/%s' % (name)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj  

if __name__ == '__main__':
    pass

