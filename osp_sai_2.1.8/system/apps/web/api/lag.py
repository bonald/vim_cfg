#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 9, 2016

@author: wangqj
'''

import base
import types
from vcl import vcmd
import portMapping

portMap = portMapping.portNameMapping()

def load_balance_get():
    """
    API:
    
    GET:
    
    Retrun: {
        state: {
            src-mac      bool,
            dst-mac      bool,
            src-ip       bool,
            dst-ip       bool,
            ip-protocol  bool,
            src-port-l4  bool,
            dst-port-l4  bool,
        },
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'cmd excute error', # err_code: 1
    ]
    

    cmd = 'cdbctl read/cdb/sys/chip'
    status, arr = vcmd.get_status_arr(cmd)
    type = arr[0]['type']
    l2_hash_field = []
    ip_hash_field = []
    ipv6_hash_field = []
    gre_hash_field = []
    vxlan_hash_field = []
    nvgre_hash_field = []
    mpls_hash_field = []

    if type =='Greatbelt':
        obj = {
            'state': {
                'src-mac'    : False,
                'dst-mac'    : False,
                'src-ip'     : False,
                'dst-ip'     : False,
                'ip-protocol': False,
                'src-port-l4': False,
                'dst-port-l4': False,
                'vxlan-vni'  : False,
                'inner-src-mac': False,
                'inner-dst-mac': False,
                'inner-src-ip' : False,
                'inner-dst-ip' : False,
                'gre-key'      : False,
                'nvgre-vsid'   : False,
                'nvgre-flow-id': False,

                'src-dst-ip'   : False,
                'src-dst-ip-src-dst-port': False,
                'src-dst-mac'  : False,
                'src-dst-port' : False,
            },
            'error': False,
            'err_code': 0, 
            'err_reason': ''
        }
    elif type == 'Tsingma':
        obj = {
            'state_tap530': {
                'l2-eth-type': False,
                'l2-macda': False,
                'l2-macsa': False,
                'l2-src-interface': False,
                'l2-vlan': False,

                'mpls-2nd-label': False,
                'mpls-3rd-label': False,
                'mpls-source-interface': False,
                'mpls-top-label': False,

                'ip-l4-destport': False,
                'ip-l4-sourceport': False,
                'ip-ipda': False,
                'ip-ipsa': False,
                'ip-ip-protocol': False,
                'ip-src-interface': False,

                'ipv6-l4-destport': False,
                'ipv6-l4-sourceport': False,
                'ipv6-ipda': False,
                'ipv6-ipsa': False,
                'ipv6-ip-protocol': False,
                'ipv6-src-interface': False,

                'gre-gre-key': False,
                'gre-gre-protocol': False,
                'gre-ipda': False,
                'gre-ipsa': False,
                'gre-src-interface': False,

                'nvgre-inner2-eth-type': False,
                'nvgre-inner2-macda': False,
                'nvgre-inner2-macsa': False,
                'nvgre-inner3-ip-protocol': False,
                'nvgre-inner3-ipda': False,
                'nvgre-inner3-ipsa': False,
                'nvgre-outer-gre-protocol': False,
                'nvgre-outer-ipda': False,
                'nvgre-outer-ipsa': False,
                'nvgre-vsid': False,
                'nvgre-src-interface': False,

                'vxlan-inner2-eth-type': False,
                'vxlan-inner2-macda': False,
                'vxlan-inner2-macsa': False,
                'vxlan-inner3-l4-destport': False,
                'vxlan-inner3-ip-protocol': False,
                'vxlan-inner3-ipda': False,
                'vxlan-inner3-ipsa': False,
                'vxlan-inner3-l4-sourceport': False,
                'vxlan-outer-l4-destport': False,
                'vxlan-outer-ipda': False,
                'vxlan-outer-ipsa': False,
                'vxlan-outer-l4-sourceport': False,
                'vxlan-outer-vlan': False,
                'vxlan-src-interface': False,
                'vxlan-vni': False,
            },
            'setip_enable': True,
            'setipv6_enable': True,
            'setmpls_enable': True,
            'error': False,
            'err_code': 0,
            'err_reason': '',
        }

    else:
        obj = {
            'state': {
                'src-mac'    : False,
                'dst-mac'    : False,
                'src-ip'     : False,
                'dst-ip'     : False,
                'ip-protocol': False,
                'src-port-l4': False,
                'dst-port-l4': False,
                'vxlan-vni'  : False,
                'inner-src-mac': False,
                'inner-dst-mac': False,
                'inner-src-ip' : False,
                'inner-dst-ip' : False,
                'inner-ip-protocol':False,
                'inner-src-port-l4':False,
                'inner-dst-port-l4': False,
                'gre-key'      : False,
                'nvgre-vsid'   : False,
                'nvgre-flow-id': False,
            },
            'error': False,
            'err_code': 0, 
            'err_reason': ''
        }  

    (status, lines) = vcmd.get_status_lines('cdbctl show/cdb/sys/lag_global/load-balance')
    if status != 0 or len(lines) <= 2:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ''.join(lines)
        return obj
    
    if type == 'Greatbelt':
        lines = lines[2:]

        if 'src-ip' in lines and 'dst-ip' in lines and 'src-port-l4' in lines and 'dst-port-l4' in lines:
            obj['state']['src-dst-ip-src-dst-port'] = True

        if 'src-ip' in lines and 'dst-ip' in lines:
            obj['state']['src-dst-ip'] = True

        if 'src-mac' in lines and 'dst-mac' in lines:
            obj['state']['src-dst-mac'] = True

        if 'src-port-l4' in lines and 'dst-port-l4' in lines:
            obj['state']['src-dst-port'] = True

        for key in lines:
            if obj['state'].has_key(key):
                obj['state'][key] = True
    elif type == 'Tsingma':
        (status, lines) = vcmd.get_status_lines('cdbctl show/cdb/l2/show_hash_field/port-channel')
        lines1 = lines[13:]
        str1 = ','.join(lines1)
        str1 = str1.replace(',', ' ')
        str1 = str1.replace(':', ' ')
        arr1 = str1.split()
        sub_arr = arr1[arr1.index('l2'):arr1.index('ip')]
        for filed_ in sub_arr:
            filed_ = 'l2-' + filed_
            l2_hash_field.append(filed_)
        l2_hash = l2_hash_field[1:]

        sub_arr = arr1[arr1.index('ip'):arr1.index('ipv6')]
        for filed_ in sub_arr:
            filed_ = 'ip-' + filed_
            ip_hash_field.append(filed_)
        ip_hash = ip_hash_field[1:]

        sub_arr = arr1[arr1.index('ipv6'):arr1.index('gre')]
        for filed_ in sub_arr:
            filed_ = 'ipv6-' + filed_
            ipv6_hash_field.append(filed_)
        ipv6_hash = ipv6_hash_field[1:]

        sub_arr = arr1[arr1.index('gre'):arr1.index('vxlan')]
        for filed_ in sub_arr:
            filed_ = 'gre-' + filed_
            gre_hash_field.append(filed_)
        gre_hash = gre_hash_field[1:]

        sub_arr = arr1[arr1.index('vxlan'):arr1.index('nvgre')]
        for filed_ in sub_arr:
            filed_ = 'vxlan-' + filed_
            vxlan_hash_field.append(filed_)
        vxlan_hash = vxlan_hash_field[1:]

        sub_arr = arr1[arr1.index('nvgre'):arr1.index('mpls')]
        for filed_ in sub_arr:
            filed_ = 'nvgre-' + filed_
            nvgre_hash_field.append(filed_)
        nvgre_hash = nvgre_hash_field[1:]

        sub_arr = arr1[arr1.index('mpls'):]
        for filed_ in sub_arr:
            if filed_ != 'inner-ipda' and filed_!= 'inner-ipsa':
                filed_ = 'mpls-' + filed_
                mpls_hash_field.append(filed_)
        mpls_hash = mpls_hash_field[1:]




        for key in gre_hash:
            obj['state_tap530'][key] = True
        for key in l2_hash:
            obj['state_tap530'][key] = True
        for key in ip_hash:
            obj['state_tap530'][key] = True
        for key in ipv6_hash:
            obj['state_tap530'][key] = True
        for key in nvgre_hash:
            obj['state_tap530'][key] = True
        for key in mpls_hash:
            obj['state_tap530'][key] = True
        for key in vxlan_hash:
            obj['state_tap530'][key] = True


        cmd = 'cdbctl read/cdb/l2/hash_field_profile/port-channel'

        (status, lines) = vcmd.get_status_lines(cmd)

        bitmapstr = lines[0][lines[0].find('bitmap_disable=')+15:]
        if len(bitmapstr) <= 2:
            bitmapval = int(bitmapstr)
        else:
            bitmapval = int(bitmapstr[0:bitmapstr.find('/')])
        if(bitmapval&1):
            obj['setip_enable'] = False
        else:
            obj['setip_enable'] = True
        if(bitmapval&2):
            obj['setipv6_enable'] = False
        else:
            obj['setipv6_enable'] = True
        if (bitmapval&8):
            obj['setmpls_enable'] = False
        else:
            obj['setmpls_enable'] = True
        return obj
    else:
        lines = lines[2:]
        for key in lines:
            if obj['state'].has_key(key):
                obj['state'][key] = True
    return obj

def product_get():
    _err_reason = [
        '',  # err_code: 0
        'bad request',  # err_code: 1
        'cmd excute error',  # err_code: 2
    ]
    obj = {'product': '','error': False, 'err_code': 0, 'err_reason': ''}
    (status, lines) = vcmd.get_status_lines("cat /tmp/ctcos_board_info | grep 'product_serie'")
    if status != 0 :
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ''.join(lines)
        return obj
    obj['product']=lines[0][-2:]
    return obj

def load_balance_set(lb_type, lb_state):
    """
    API:
    
    PUT: /<lb_type>/<lb_state>/
        
    @param lb_type:
        src-mac | dst-mac | src-ip | dst-ip | ip-protocol | src-port-l4 | dst-port-l4
    @param lb_state:
        set | unset
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check 
    lb_type = str(lb_type)
    lb_state = str(lb_state)

    #if not lb_type or not lb_type in ['src-mac', 'dst-mac', 'src-ip', 'dst-ip', 'ip-protocol', 'src-port-l4', 'dst-port-l4'] or not lb_state in ['set', 'unset']:
    #    obj['error'] = True
    #    obj['err_code'] = 1
    #    obj['err_reason'] = _err_reason[1]
    #    return obj

    cmd = 'cdbctl read/cdb/sys/chip'
    status, arr = vcmd.get_status_arr(cmd)
    type = arr[0]['type']

    if type =='Greatbelt':
        if lb_state == 'set':
            cmd = 'cdbctl update/cdb/sys/lag_global/load_balance/overwrite/%s' % (lb_type)

            status, lines = vcmd.get_status_lines(cmd)
            if status != 0:
                obj['error'] = True
                obj['err_code'] = 2
                obj['err_reason'] = '; '.join(lines)
                return obj
        else:
            cmd = 'cdbctl update/cdb/sys/lag_global/load_balance/set/default'

            status, lines = vcmd.get_status_lines(cmd)
            if status != 0:
                obj['error'] = True
                obj['err_code'] = 2
                obj['err_reason'] = '; '.join(lines)
                return obj            
    else:    
        # excute
        cmd = 'cdbctl update/cdb/sys/lag_global/load_balance/%s/%s' % (lb_state, lb_type)
        status, lines = vcmd.get_status_lines(cmd)
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(lines)
            return obj
    
    return obj
    
    
    
def channel_group_get(gid):
    """
    API:
    
    GET: / or /<int:gid>/
    
    @param gid: group id
    
    Retrun: {
        if /:
            group_arr: [group_dict, group_dict, ...],
        elif /<int:git>/:
            group: group_dict
        
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    group_dict {
        lag_name: string,
        protocol: 'static' | 'LACP',
        group_state: 'L2' | 'L3',
        ports_in_bundle: int,
        member_ports: ['eth-0-1', ...],
        max_ports_count: int,
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
        'group is not exist', # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check 
    if gid != 'all' and base.parse.parse_int(gid) == None:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
    
    # excute
    if gid == 'all':
        status, arr = vcmd.get_status_arr('cdbctl read/cdb/interface/lag//type/summary')
    else:
        status, arr = vcmd.get_status_arr('cdbctl read/cdb/interface/lag/%d/type/summary' % int(gid))
        
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj

    max_ports_count = 16
    status, version = vcmd.get_status_arr('cdbctl read/cdb/sys/version')

    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj

    for info in version:
        product = info.get('product')
    
    if (product.find('580') != -1 or product.find('550') != -1 or product.find('530') != -1):
        max_ports_count = 64

    # arr shaping
    arr_shaping = []
    for i in arr:
        lag_name = i.get('key')
        protocol = 'None' if i.get('lag') == None else 'static' if i.get('lag').get('mode') == 'static' else 'LACP'
        group_state = 'L2' if i.get('mode') == 'switch' else 'L3'
        ports_in_bundle = 'None' if i.get('lag') == None else base.parse.parse_int(i.get('lag').get('bundle_ports_count'))
        member_ports = 'None' if i.get('lag') == None else i.get('lag').get('member_ports').split(',')
        for mem_item in range(len(member_ports)):
            member_ports[mem_item] = portMap.Mapping(member_ports[mem_item])
        
        member_ports, err = base.check.array(member_ports, ['string', '!=', '', '', 1], True)
        
        arr_shaping.append({
            'lag_name': lag_name,
            'protocol': protocol,
            'group_state': group_state,
            'ports_in_bundle': ports_in_bundle,
            'member_ports': member_ports,
            'max_ports_count': max_ports_count
        })

    # Insert "load_balance_mode" fild to arr
    status, lines = vcmd.get_status_arr("cdbctl read/cdb/sys/lag_global/load_balance_mode")
    for lag in arr_shaping:

        lag_name = lag.get('lag_name')
        replace = lag_name.replace('agg', '')

        if (replace.isdigit()):
            lagId = int(replace)
        else:
            continue

        mode = lines[0].get('load_balance_mode').split(',')
        m = mode[lagId]
        lag['mode'] = m

    # return
    if gid == 'all':
        obj['group_arr'] = arr_shaping
    elif len(arr_shaping) >= 1:
        obj['group'] = arr_shaping[0]
    else:
        obj['error'] = True
        obj['err_code'] = 3 # 不要随便�?
        obj['err_reason'] = _err_reason[3]
        return obj
    
    return obj
    
    
    
def channel_group_set(gid,agg_mode_seclected,channel_mode, req_data):
    """
    API:
    
    PUT: /<int:gid>/
    {
        if_arr: [
            'eth-0-1',
            ...
        ],
    }
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    # param check 
    if base.parse.parse_int(gid) == None or req_data.get('if_arr') == None or type(req_data['if_arr']) != types.ListType:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj    
    
    res, err = base.check.array(req_data['if_arr'], ['string', 'bad request: if_arr', 1], False)
    if res == None:
        obj['error'] = True
        obj['err_code'] = err['err_code']
        obj['err_reason'] = err['err_reason']
        return obj    
    
    # clear old member port
    res1 = channel_group_get(gid)
    if res1['error']:
        if res1['err_code'] != 3:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = res1['err_reason']
            return obj    
    else:
        res2 = channel_group_delete({'if_arr': res1['group']['member_ports']})
        if res2['error']:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = res2['err_reason']
            return obj    
        
        
    # excute
    if agg_mode_seclected==1:
        status, lines = vcmd.get_status_lines(['cdbctl create/cdb/interface/%s/lag/%d/mode/static' % (port, int(gid)) for port in res])
    else:
        if channel_mode==1:
            status, lines = vcmd.get_status_lines(['cdbctl create/cdb/interface/%s/lag/%d/mode/active' % (port, int(gid)) for port in res])
        else:
            status, lines = vcmd.get_status_lines(['cdbctl create/cdb/interface/%s/lag/%d/mode/passive' % (port, int(gid)) for port in res])
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj
        
        
def channel_group_delete(req_data):
    """
    API:
    
    DELETE: /<int:gid>/
    {
        if_arr: [
            'eth-0-1',
            ...
        ],
    }
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check 
    if req_data.get('if_arr') == None or type(req_data['if_arr']) != types.ListType:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj    
    
    res, err = base.check.array(req_data['if_arr'], ['string', 'bad request: if_arr', 1], False)
    if res == None:
        obj['error'] = True
        obj['err_code'] = err['err_code']
        obj['err_reason'] = err['err_reason']
        return obj
    status, lines = vcmd.get_status_lines(['cdbctl read/cdb/interface/%s/lag/' % port for port in res])
    a=lines[0].split('/')
    b=a[2].split('=')
    # excute
    if b[1]=='static':
        status, lines = vcmd.get_status_lines(['cdbctl delete/cdb/interface/%s/lag/static' % port for port in res])
    else:
        status, lines = vcmd.get_status_lines(['cdbctl delete/cdb/interface/%s/lag/dynamic' % port for port in res])
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj
            
def channel_agg_mode_set(gid, req_data):
    """
    API:
    
    DELETE: /<int:gid>/
    {
        if_arr: [
            'eth-0-1',
            ...
        ],
    }
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    gid = str(gid)
    mode = str(req_data['mode'])
    
    cmd = "cdbctl update/cdb/sys/lag_global/load_balance_mode/%s/%s" % (gid, mode)

    # excute
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj  
    
    return obj    

def load_balance_tunnel_hash_set(hashmode):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request param',    # err_code: 1
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    vcmd.get_status_lines('cdbctl update/cdb/sys/lag_global/load_balance_inner_field_en/inner_field_en/%s' % str(hashmode))
    return obj

def load_balance_tunnel_hash_get():
    """
     API:

     GET:

     Retrun: {

         arr: [group_dict, group_dict, ...],
         error: bool,
         err_code: int,
         err_reason: string
     }

     group_dict {
         load_balance: string,
         load_balance_inner_filed_en: '0' | '1' | '2',
         ...
     }
     """
    _err_reason = [
            '',                    # err_code: 0
            'get tunnel hash mode cfg error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    arr = vcmd.get_arr('cdbctl read/cdb/sys/lag_global/')
    if arr:
        obj['arr'] = arr[0]
        #print obj
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]

    return obj

def load_balance_set_gre_hash(key, protocol, ipda, ipsa, srcinterface):
    _err_reason = [
        '',  # err_code: 0
        'bad request',  # err_code: 1
        'cmd excute error',  # err_code: 2
        'group is not exist',  # err_code: 3
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    # obj['error'] = True
    # obj['err_code'] = 1
    # obj['err_reason'] = _err_reason[1]
    if key=="true":

        gre_key = "gre-key"
    else:
        gre_key = ""

    if protocol == "true":
        gre_protocol = "gre-protocol"
    else:
        gre_protocol = ""

    if ipda=="true":
        gre_ipda = "ipda"
    else:
        gre_ipda = ""

    if ipsa=="true":
        gre_ipsa = "ipsa"
    else:
        gre_ipsa = ""
    if srcinterface=="true":
        gre_srcinterface = "src-interface"
    else:
        gre_srcinterface = ""

    if gre_key == "" and gre_protocol == "" and  gre_ipda == "" and gre_ipsa == "" and gre_srcinterface ==  "":
        gre_cmd = "cdbctl update/cdb/l2/hash_field_profile/port-channel/gre_set/0"
    else:
        gre_cmd = 'cdbctl update/cdb/l2/hash_field_profile/port-channel/gre_set/gre-key/%s/gre-protocol/%s/ipda/%s/ipsa/%s/src-interface/%s/end' %(gre_key, gre_protocol, gre_ipda, gre_ipsa, gre_srcinterface)
    status, lines = vcmd.get_status_lines(gre_cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj


    return obj


def load_balance_set_l2_hash(type, macda, macsa, srcinterface, vlan):
    _err_reason = [
        '',  # err_code: 0
        'bad request',  # err_code: 1
        'cmd excute error',  # err_code: 2
        'group is not exist',  # err_code: 3
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    # obj['error'] = True
    # obj['err_code'] = 1
    # obj['err_reason'] = _err_reason[1]
    if type=="true":

        l2_type = "eth-type"
    else:
        l2_type = ""

    if macda == "true":
        l2_macda = "macda"
    else:
        l2_macda = ""

    if macsa=="true":
        l2_macsa = "macsa"
    else:
        l2_macsa = ""

    if srcinterface == "true":
        l2_srcinterface = "src-interface"
    else:
        l2_srcinterface = ""

    if vlan=="true":
        l2_vlan = "vlan"
    else:
        l2_vlan = ""

    if l2_type == "" and l2_macda == "" and  l2_macsa == "" and l2_srcinterface == "" and l2_vlan ==  "":
        l2_cmd = "cdbctl update/cdb/l2/hash_field_profile/port-channel/l2_set/0"
    else:
        l2_cmd = 'cdbctl update/cdb/l2/hash_field_profile/port-channel/l2_set/eth-type/%s/macda/%s/macsa/%s/src-interface/%s/vlan/%s/end'%(l2_type, l2_macda, l2_macsa, l2_srcinterface, l2_vlan)
    status, lines = vcmd.get_status_lines(l2_cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj


    return obj

def load_balance_set_ip_hash(destport, sourceport, ipda, ipsa, ip_protocol, srcinterface):
    _err_reason = [
        '',  # err_code: 0
        'bad request',  # err_code: 1
        'cmd excute error',  # err_code: 2
        'group is not exist',  # err_code: 3
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    # obj['error'] = True
    # obj['err_code'] = 1
    # obj['err_reason'] = _err_reason[1]
    if destport=="true":

        ip_destport = "destport"
    else:
        ip_destport = ""

    if sourceport == "true":
        ip_sourceport = "sourceport"
    else:
        ip_sourceport = ""

    if ipda=="true":
        ip_ipda = "ipda"
    else:
        ip_ipda = ""

    if ipsa == "true":
        ip_ipsa = "ipsa"
    else:
        ip_ipsa = ""

    if ip_protocol=="true":
        ip_ip_protocol = "ip-protocol"
    else:
        ip_ip_protocol = ""
    if srcinterface=="true":
        ip_srcinterface = "src-interface"
    else:
        ip_srcinterface = ""

    if ip_destport == "" and ip_sourceport == "" and  ip_ipda == "" and ip_ipsa == "" and ip_ip_protocol ==  "" and ip_srcinterface ==  "":
        ip_cmd = "cdbctl update/cdb/l2/hash_field_profile/port-channel/ip_set/0"
    else:
        ip_cmd = 'cdbctl update/cdb/l2/hash_field_profile/port-channel/ip_set/destport/%s/ip-protocol/%s/ipda/%s/ipsa/%s/sourceport/%s/src-interface/%s/end'%(ip_destport, ip_ip_protocol, ip_ipda, ip_ipsa, ip_sourceport, ip_srcinterface)
    status, lines = vcmd.get_status_lines(ip_cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj

    return obj

def load_balance_set_ipv6_hash(destport, sourceport, ipda, ipsa, ip_protocol, srcinterface):
        _err_reason = [
            '',  # err_code: 0
            'bad request',  # err_code: 1
            'cmd excute error',  # err_code: 2
            'group is not exist',  # err_code: 3
        ]

        obj = {'error': False, 'err_code': 0, 'err_reason': ''}

        # obj['error'] = True
        # obj['err_code'] = 1
        # obj['err_reason'] = _err_reason[1]
        if destport == "true":

            ipv6_destport = "destport"
        else:
            ipv6_destport = ""

        if sourceport == "true":
            ipv6_sourceport = "sourceport"
        else:
            ipv6_sourceport = ""

        if ipda == "true":
            ipv6_ipda = "ipda"
        else:
            ipv6_ipda = ""

        if ipsa == "true":
            ipv6_ipsa = "ipsa"
        else:
            ipv6_ipsa = ""

        if ip_protocol == "true":
            ipv6_ip_protocol = "ip-protocol"
        else:
            ipv6_ip_protocol = ""
        if srcinterface == "true":
            ipv6_srcinterface = "src-interface"
        else:
            ipv6_srcinterface = ""

        if ipv6_destport == "" and ipv6_sourceport == "" and ipv6_ipda == "" and ipv6_ipsa == "" and ipv6_ip_protocol == "" and ipv6_srcinterface == "":
            ipv6_cmd = "cdbctl update/cdb/l2/hash_field_profile/port-channel/ipv6_set/0"
        else:
            ipv6_cmd = 'cdbctl update/cdb/l2/hash_field_profile/port-channel/ipv6_set/destport/%s/ip-protocol/%s/ipda/%s/ipsa/%s/sourceport/%s/src-interface/%s/end' % (ipv6_destport, ipv6_ip_protocol, ipv6_ipda, ipv6_ipsa, ipv6_sourceport, ipv6_srcinterface)
        status, lines = vcmd.get_status_lines(ipv6_cmd)
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(lines)
            return obj

        return obj

def load_balance_set_mpls_hash(label2nd, label3rd, srcinterface,toplabel):
    _err_reason = [
        '',  # err_code: 0
        'bad request',  # err_code: 1
        'cmd excute error',  # err_code: 2
        'group is not exist',  # err_code: 3
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    # obj['error'] = True
    # obj['err_code'] = 1
    # obj['err_reason'] = _err_reason[1]
    if label2nd == "true":

        mpls_label2nd = "2nd-label"
    else:
        mpls_label2nd = ""

    if label3rd == "true":
        mpls_label3rd = "3rd-label"
    else:
        mpls_label3rd = ""

    mpls_inneripda = ""
    mpls_inneripsa = ""

    if srcinterface == "true":
        mpls_srcinterface = "src-interface"
    else:
        mpls_srcinterface = ""
    if toplabel == "true":
        mpls_toplabel = "top-label"
    else:
        mpls_toplabel = ""

    if mpls_label2nd == "" and mpls_label3rd == "" and mpls_inneripda == "" and mpls_inneripsa == "" and mpls_srcinterface == "" and mpls_toplabel == "":
        mpls_cmd = "cdbctl update/cdb/l2/hash_field_profile/port-channel/mpls_set/0"
    else:
        mpls_cmd = 'cdbctl update/cdb/l2/hash_field_profile/port-channel/mpls_set/2nd-label/%s/3rd-label/%s/inner////src-interface/%s/top-label/%s/end' % (
        mpls_label2nd, mpls_label3rd, mpls_srcinterface, mpls_toplabel)
    status, lines = vcmd.get_status_lines(mpls_cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj

    return obj

def load_balance_set_nvgre_hash(innerl2type, innerl2macda, innerl2macsa, innerl3ip_protocol, innerl3ipda, innerl3ipsa, outergre_protocol, outeripda, outeripsa, vsid, srcinterface,nvgreval):
    _err_reason = [
        '',  # err_code: 0
        'bad request',  # err_code: 1
        'cmd excute error',  # err_code: 2
        'group is not exist',  # err_code: 3
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    # obj['error'] = True
    # obj['err_code'] = 1
    # obj['err_reason'] = _err_reason[1]
    if innerl2type == "true":

        nvgre_innerl2type = "eth-type"
    else:
        nvgre_innerl2type = ""

    if innerl2macda == "true":
        nvgre_innerl2macda = "macda"
    else:
        nvgre_innerl2macda = ""

    if innerl2macsa == "true":
        nvgre_innerl2macsa = "macsa"
    else:
        nvgre_innerl2macsa = ""

    if innerl3ip_protocol == "true":
        nvgre_innerl3ip_protocol = "ip-protocol"
    else:
        nvgre_innerl3ip_protocol = ""

    if innerl3ipda == "true" or outeripda == "true":
        nvgre_ipda = "ipda"
    else:
        nvgre_ipda = ""

    if innerl3ipsa == "true" or outeripsa == "true":
        nvgre_ipsa = "ipsa"
    else:
        nvgre_ipsa = ""

    if outergre_protocol == "true":
        nvgre_outergre_protocol = "gre-protocol"
    else:
        nvgre_outergre_protocol = ""

    if vsid == "true":
        nvgre_vsid = "vsid"
    else:
        nvgre_vsid = ""

    if srcinterface == "true":
        nvgre_srcinterface = "src-interface"
    else:
        nvgre_srcinterface = ""




    if nvgre_innerl2type == "" and nvgre_innerl2macda == "" and nvgre_innerl2macsa == "" and nvgre_innerl3ip_protocol == "" and nvgre_ipda == "" and nvgre_ipsa == "" and nvgre_outergre_protocol == "" and nvgre_vsid == "" and nvgre_srcinterface == "":
        nvgre_cmd = "cdbctl update/cdb/l2/hash_field_profile/port-channel/nvgre_set/0"
    elif nvgreval == "nvgre-inner2":
        nvgre_cmd = 'cdbctl update/cdb/l2/hash_field_profile/port-channel/nvgre_set/inner-layer2/inner-layer2/%s/%s/%s/inner-layer3/////outer/////vsid/%s/src-interface/%s/end'% (
            nvgre_innerl2type, nvgre_innerl2macda, nvgre_innerl2macsa, nvgre_vsid, nvgre_srcinterface)
    elif nvgreval == "nvgre-inner3":
        nvgre_cmd = 'cdbctl update/cdb/l2/hash_field_profile/port-channel/nvgre_set/inner-layer2/////inner-layer3/inner-layer3/%s/%s/%s/outer/////vsid/%s/src-interface/%s/end'% (
            nvgre_innerl3ip_protocol, nvgre_ipda, nvgre_ipsa, nvgre_vsid, nvgre_srcinterface)
    else:
        nvgre_cmd = 'cdbctl update/cdb/l2/hash_field_profile/port-channel/nvgre_set/inner-layer2/////inner-layer3/////outer/outer/%s/%s/%s/vsid/%s/src-interface/%s/end' % (
            nvgre_outergre_protocol, nvgre_ipda, nvgre_ipsa, nvgre_vsid, nvgre_srcinterface)
    status, lines = vcmd.get_status_lines(nvgre_cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj

    return obj


def load_balance_set_vxlan_hash(innerl2type, innerl2macda, innerl2macsa, innerl3destport, innerl3ip_protocol, innerl3ipda, innerl3ipsa, innerl3sourceport, outer_destport, outeripda, outeripsa, outer_sourceport, vlan, srcinterface, vni, vxlanval):
    _err_reason = [
        '',  # err_code: 0
        'bad request',  # err_code: 1
        'cmd excute error',  # err_code: 2
        'group is not exist',  # err_code: 3
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    # obj['error'] = True
    # obj['err_code'] = 1
    # obj['err_reason'] = _err_reason[1]
    if innerl2type == "true":

        vxlan_innerl2type = "eth-type"
    else:
        vxlan_innerl2type = ""

    if innerl2macda == "true":
        vxlan_innerl2macda = "macda"
    else:
        vxlan_innerl2macda = ""

    if innerl2macsa == "true":
        vxlan_innerl2macsa = "macsa"
    else:
        vxlan_innerl2macsa = ""

    if innerl3ip_protocol == "true":
        vxlan_innerl3ip_protocol = "ip-protocol"
    else:
        vxlan_innerl3ip_protocol = ""

    if vlan == "true":
        vxlan_vlan = "vlan"
    else:
        vxlan_vlan = ""

    if innerl3ipda == "true" or outeripda == "true":
        vxlan_ipda = "ipda"
    else:
        vxlan_ipda = ""

    if innerl3ipsa == "true" or outeripsa == "true":
        vxlan_ipsa = "ipsa"
    else:
        vxlan_ipsa = ""

    if outer_destport == "true" or innerl3destport == "true":
        vxlan_destport = "destport"
    else:
        vxlan_destport = ""

    if outer_sourceport == "true" or innerl3sourceport == "true":
        vxlan_sourceport = "sourceport"
    else:
        vxlan_sourceport = ""

    if srcinterface == "true":
        vxlan_srcinterface = "src-interface"
    else:
        vxlan_srcinterface = ""

    if vni == "true":
        vxlan_vni = "vni"
    else:
        vxlan_vni = ""



    if vxlan_innerl2type == "" and vxlan_innerl2macda == "" and vxlan_innerl2macsa == "" and vxlan_innerl3ip_protocol == "" and vxlan_vlan == "" and vxlan_ipda == "" and vxlan_ipsa == "" and vxlan_destport == "" and vxlan_sourceport == "" and vxlan_srcinterface == "" and vxlan_vni == "":
        vxlan_cmd = " cdbctl update/cdb/l2/hash_field_profile/port-channel/vxlan_set/0"
    elif vxlanval == "vxlan-inner2":
        vxlan_cmd = 'cdbctl update/cdb/l2/hash_field_profile/port-channel/vxlan_set/inner-layer2/inner-layer2/%s/%s/%s/inner-layer3///////outer///////src-interface/%s/vni/%s/end'% (
            vxlan_innerl2type, vxlan_innerl2macda, vxlan_innerl2macsa, vxlan_srcinterface, vxlan_vni)
    elif vxlanval == "vxlan-inner3":
        vxlan_cmd = 'cdbctl update/cdb/l2/hash_field_profile/port-channel/vxlan_set/inner-layer2/////inner-layer3/inner-layer3/%s/%s/%s/%s/%s/outer///////src-interface/%s/vni/%s/end'% (
            vxlan_destport, vxlan_innerl3ip_protocol, vxlan_ipda, vxlan_ipsa, vxlan_sourceport, vxlan_srcinterface, vxlan_vni)
    else:
        vxlan_cmd = 'cdbctl update/cdb/l2/hash_field_profile/port-channel/vxlan_set/inner-layer2/////inner-layer3///////outer/outer/%s/%s/%s/%s/%s/src-interface/%s/vni/%s/end' % (
            vxlan_destport, vxlan_ipda, vxlan_ipsa, vxlan_sourceport, vxlan_vlan, vxlan_srcinterface, vxlan_vni)
    status, lines = vcmd.get_status_lines(vxlan_cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj

    return obj

def load_balance_set_hash_ip_enable(ip_enable):
    _err_reason = [
        '',  # err_code: 0
        'bad request',  # err_code: 1
        'cmd excute error',  # err_code: 2
        'group is not exist',  # err_code: 3
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    # obj['error'] = True
    # obj['err_code'] = 1
    # obj['err_reason'] = _err_reason[1]

    if ip_enable == "true":
        cmd = "cdbctl update/cdb/l2/hash_field_profile/port-channel/bitmap_disable/ip/1"
    else:
        cmd = "cdbctl update/cdb/l2/hash_field_profile/port-channel/bitmap_disable/ip/0"
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj

    return obj


def load_balance_set_hash_ipv6_enable(ipv6_enable):
    _err_reason = [
        '',  # err_code: 0
        'bad request',  # err_code: 1
        'cmd excute error',  # err_code: 2
        'group is not exist',  # err_code: 3
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    # obj['error'] = True
    # obj['err_code'] = 1
    # obj['err_reason'] = _err_reason[1]

    if ipv6_enable == "true":
        cmd = "cdbctl update/cdb/l2/hash_field_profile/port-channel/bitmap_disable/ipv6/1"
    else:
        cmd = "cdbctl update/cdb/l2/hash_field_profile/port-channel/bitmap_disable/ipv6/0"
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj

    return obj

def load_balance_set_hash_mpls_enable(mpls_enable):
    _err_reason = [
        '',  # err_code: 0
        'bad request',  # err_code: 1
        'cmd excute error',  # err_code: 2
        'group is not exist',  # err_code: 3
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    # obj['error'] = True
    # obj['err_code'] = 1
    # obj['err_reason'] = _err_reason[1]

    if mpls_enable == "true":
        cmd = "cdbctl update/cdb/l2/hash_field_profile/port-channel/bitmap_disable/mpls/1"
    else:
        cmd = "cdbctl update/cdb/l2/hash_field_profile/port-channel/bitmap_disable/mpls/0"
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj

    return obj
