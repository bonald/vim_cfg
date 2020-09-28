#!/usr/bin/python
#-*- coding: utf-8 -*-

from webapi import WebApi, ApiError, Parser, Verify
from vcl import vcmd

class VlanApi(WebApi):
    
    def get(self):
        '''
        get vlan array
        '''
        raise ApiError(500, 1, 'vlan status is null')
        obj = {'vlan_arr': []}

        args = Verify.dict(self.req_args, {
            'limit?': (lambda x: x.type('int') and x > 0,
                       ApiError(400, 1, 'invalid limit')),
            'index?': (lambda x: x.type('int') and x > 0,
                       ApiError(400, 1, 'invalid index')),
        })

        arr = vcmd.get_arr_simple('cdbctl read/cdb/l2/vlan')
        arr_tmp = []
        if arr:
            for i in arr:
                tmp = {}
                tmp['id']             = Parser.int(i.get('key'), 0)
                tmp['member_port']    = i.get('member_port')
                tmp['name']           = i.get('name')
                
                arr_tmp.append(tmp)
        else:
            raise ApiError(500, 1, 'vlan status is null')
        
        # filter
        start = 0 if None == args.get('index') else args.get('index')
        end   = -1 if None == args.get('limit') else start + args.get('limit')
        arr_tmp = arr_tmp[start:end]

        obj['vlan_arr'] = arr_tmp

        return obj

    def post(self):
        '''
        add vlan
        '''
        res = Verify.dict(self.req_data, {
            'id': (lambda x: x.type('int') and x >= 1 and x <= 4095,
                   ApiError(400, 1, 'invalid vlan id')),
            'name?': (lambda x: x.type('string') and x.match(r'^[a-zA-Z][a-zA-Z\.\-\_0-9]{0,15}$'),
                   ApiError(400, 2, 'invalid vlan name')),
        })

        vlan_id = res.get('id')
        vlan_name = res.get('name')

        # create vlan
        res = vcmd.execute('cdbctl create/cdb/l2/vlan/%d' % vlan_id)
        if res != 0:
            raise ApiError(400, 3, 'cannot create vlan')
        
        # modify name
        if vlan_name:
            res = vcmd.execute('cdbctl update/cdb/l2/vlan/%d/name/%s' % (vlan_id, vlan_name))
            if res != 0:
                raise ApiError(400, 4, 'cannot modify name')

    def put(self):
        '''
        modify vlan name
        '''
        res = Verify.dict(self.req_data, {
            'id': (lambda x: x.type('int') and x >= 1 and x <= 4095,
                   ApiError(400, 1, 'invalid vlan id')),
            'name': (lambda x: x.type('string') and x.match(r'^[a-zA-Z][a-zA-Z\.\-\_0-9]{0,15}$'),
                   ApiError(400, 2, 'invalid vlan name')),
        })

        res = vcmd.execute('cdbctl update/cdb/l2/vlan/%d/name/%s' % (res['id'], res['name']))
        if res != 0:
            raise ApiError(400, 3, 'cannot modify name')

    def delete(self):
        '''
        delete vlans
        '''
        vlan_ids = Verify.array(self.req_data.get('vlan_ids'), lambda x: x.type('int') and x >= 1 and x <= 4095, ApiError(400, 1, 'invalid vlan id'))
        
        status, lines = vcmd.get_status_lines(['cdbctl delete/cdb/l2/vlan/%d' % x for x in vlan_ids])
        if status != 0:
            raise ApiError(400, 2, ' '.join(lines))

