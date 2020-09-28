#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on Apr 29, 2016

@author: wangqj
'''

import types
import parse
import re

def _dict_check_opt(value, arr):
    
    val = value
    
    if len(arr) == 2:
        opt, opt_val = arr[0], arr[1]
        
        if type(val) == types.StringType and opt in ['<', '<=', '>', '>=']:
            val = len(value)
        
        if opt == '==':
            return val == opt_val
        elif opt == '!=':
            return val != opt_val
        elif opt == '<':
            return val < opt_val
        elif opt == '<=':
            return val <= opt_val
        elif opt == '>':
            return val > opt_val
        elif opt == '>=':
            return val >= opt_val
        elif opt == 'in':
            return val in opt_val
        elif opt == 're':
            return True if re.match(opt_val, val) else False
            
    elif len(arr) >= 3:
        relation = arr[2]
        res1 = _dict_check_opt(val, arr[0:2])
        res2 = _dict_check_opt(val, arr[3:])
        if relation == 'and':
            return res1 and res2
        elif relation == 'or':
            return res1 or res2
        else:
            return False
        
    else:
        return False
        


def dictionary(dict_data, arr):
    """
    @param arr [
        [$key, $type, $opt1, $opt_val1, $relation1, $opt2, $opt_val2, $relation2 ..., $err_reason, $err_code]
    ]
        $key: key in arr
        $type: 'string' or 'int' or 'array' or 'dict'
        $opt, $opt_val:
            None
            '==', string | number
            '!=', string | number
            '<' | '<=' | '>' | '>=', number
            'in', array of string or number
            're', regexp
        $relation:
            relation to next $opt: None | 'and' | 'or'
        $err_reason: error reason
        $err_code: error code
        
        example:
            dictionary({'ip': '192.168.1.1', 'vlan': 4094, 'type': 'dynamic', 'pwd': 'abcd1234'}, [
                ['ip', 'string', 're', r'^(\d{1,3}\.){3}\d{1,3}$', 'bad request: ip', 1],
                ['vlan', 'int', '>=', 2, 'and', '<', 4095, 'bad request: vlan', 1],
                ['type', 'string', 'in', ['static', 'dynamic'], 'bad request: type', 1],
                ['pwd', 'string', '>=', 6, 'and', '<=', 10, 'bad password', 1],
            ])
    
    @return (None, {err_code: int, err_reason: string}) or (dict after check, None)
    """
    res = {}
    
    if not dict_data or type(dict_data) != types.DictType:
        return (None, {'err_code': -1, 'err_reason': 'dict_data error'})
    
    for i in arr:
        err = {'err_code': i[-1], 'err_reason': i[-2]}
        i = i[:-2]
        
        key, data_type = i[0], i[1]
        value = dict_data.get(key)
        if value == None:
            return (None, err)
        if data_type == 'int':
            value = parse.parse_int(value)
            if value == None:
                return (None, err)
        elif data_type == 'string':
            value = str(value)
        elif data_type == 'array':
            if type(value) != types.ListType or not value:
                return (None, err)
        elif data_type == 'dict':
            if type(value) != types.DictionaryType:
                return (None, err)
        else:
            return (None, {'err_code': -1, 'err_reason': 'unsupport type'})
        
        # $opt
        if len(i) > 2 and not _dict_check_opt(value, i[2:]):
            return (None, err)
            
        res[key] = value
    
    return (res, None)


def array(list_data, arr, filter=False):
    """
    @param arr [
        $type, $opt1, $opt_val1, $relation1, $opt2, $opt_val2, $relation2 ..., $err_reason, $err_code
    ]
        $type: 'string' or 'int' or 'array' or 'dict'
        $opt, $opt_val:
            None
            '==', string | number
            '!=', string | number
            '<' | '<=' | '>' | '>=', number
            'in', array of string or number
            're', regexp
        $relation:
            relation to next $opt: None | 'and' | 'or'
        $err_reason: error reason
        $err_code: error code
    
    @param filter:
        if filter is True, ignore the mismatch item
        if false, all items must matched
        
        example:
            array(['eth-0-1', 'eth-0-2'], [
                ['string', 're', r'^[a-zA-Z\-]{3,}$', 'bad request: ip', 1],
            ], True)
    
    @return (None, {err_code: int, err_reason: string}) or (array after check, None)
    """
    ret_arr = []
    if type(list_data) != types.ListType or not list_data:
        return (None, None)
    
    err = {'err_code': arr[-1], 'err_reason': arr[-2]}
    for item in list_data:
        data_type = arr[0]
        if data_type == 'int':
            item = parse.parse_int(item)
            if item == None:
                return (None, err)
        elif data_type == 'string':
            item = str(item)
        else:
            return (None, err)
            
        # $opt
        if len(arr) > 3 and not _dict_check_opt(item, arr[1:-2]):
            if filter == True:
                continue
            else:
                return (None, err)
        else:
            ret_arr.append(item)
        
    return (ret_arr, None)

if __name__ == '__main__':
    
#    print dictionary({'ip': '192.168.1.1', 'vlan': 123, 'type': 'dynamic', 'pwd': 'abcd1234'}, [
#        ['vlan', 'int', '>=', 2, 'and', '<', 4095, 'bad vlan', 1],
#        ['ip', 'string', 're', r'^(\d{1,3}\.){3}\d{1,3}$', 'bad ip address', 1],
#        ['type', 'string', '==', 'static', 'or', '==', 'dynamic', 'bad port type', 1],
#        ['pwd', 'string', '>=', 6, 'and', '<=', 10, 'bad password', 1]
#    ])
#    
#    print dictionary({'ip': '192.168.1.1', 'vlan': 'aaaaaaaaaa', 'type': 'dynamic'}, [
#        ['vlan', 'int', 'bad vlan', 1],
#    ])

#    print array(['eth-0-1', 'eth-0-2', '__fdsfa__'], ['string', 're', r'^[a-zA-Z0-9\-]{3,15}$', 'bad request', 1], True)
    
    pass

