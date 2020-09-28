#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on Aug 20, 2016

@author: wangqj
'''

import re
import types
from parse import Parser

class VerifyParam(object):
    '''
    e.g.:
        int:
            ins = VerifyParam('10')
            ins.type('int') -> ins.__param = int('10')
            ins == 10
            ins != 9
            ins > 9
            ins >= 10
            ins < 11
            ins <= 10
            ins.any([9, 10, 11])

        string:
            ins = VerifyParam('abc')
            ins.type('string')
            ins == 'abc'
            ins != 'abcd'
            ins > 'aaa'
            ins < 'bbb'
            ins >= 'abc'
            ins <= 'abc'
            len(ins) == 3
            ins.any(['a', 'b', 'abc'])
            ins.match(r'^\w+$')

        float:
            ins = VerifyParam('1.2')
            ins.type('float')
            ins == 1.1
            ins != 1
            ins > 1
            ins < 2
            ins >= 1.1
            ins <= 1.1
            ins.any([1, 1.1])

        bool:
            ins = VerifyParam('true')
            ins.type('bool')
            ins == True
            ins != False

        dict:
            ins = VerifyParam({'key1': 'val1', 'key2': 'val2'})
            ins.type('dict')
            ins == {'key1': 'val1', 'key2': 'val2'}
            ins == {'key2': 'val2', 'key1': 'val1'}
            len(ins) == 2

        array:
            ins = VerifyParam([1, 2, 3, 4])
            ins.type('array')
            ins == [1, 2, 3, 4]
            ins != [1, 2, 4, 3]
            len(ins) == 4
    '''
    
    def __init__(self, param):
        self.__param = param

    def __lt__(self, other):
        return self.__param < other
    
    def __le__(self, other):
        return self.__param <= other
    
    def __gt__(self, other):
        return self.__param > other
    
    def __ge__(self, other):
        return self.__param >= other
    
    def __eq__(self, other):
        return self.__param == other
    
    def __ne__(self, other):
        return not self.__eq__(other)
    
    def __len__(self):
        length = 0
        if type(self.__param) == types.IntType:
            length = len(str(self.__param))
        else:
            length = len(self.__param)
        return length
    
    def match(self, regexp):
        '''
        match regexp
        '''
        match = re.match(regexp, self.__param)
        return True if match else False
    
    def any(self, array):
        '''
        return True if item in array
        '''
        return self.__param in array
    
    def type(self, string):
        '''
        @param string:
            'string' | 'int' | 'float' | 'bool' | 'dict' | 'array'
        '''
        tp = type(self.__param)
        
        if string == 'string':
            return tp == types.StringType
        elif string == 'int':
            tmp = Parser.int(self.__param, None)
            if tmp != None:
                self.__param = tmp
            tp = type(self.__param)
            return tp == types.IntType
        elif string == 'float':
            tmp = Parser.float(self.__param, None)
            if tmp != None:
                self.__param = tmp
            tp = type(self.__param)
            return tp == types.FloatType
        elif string == 'bool':
            return tp == types.BooleanType
        elif string == 'dict':
            return tp == types.DictionaryType
        elif string == 'array':
            return tp == types.ListType
        return False
    
    def get(self):
        return self.__param

class Verify(object):
    
    @staticmethod
    def dict(data, verify):
        '''
        dict verification
        @param data:
            [dict] The dictionary that you want to verify
        @param verify:
            [dict] verify dictionary:
                {
                    'key': (verify function, Exception class)
                } 
                or 
                {
                    'key?': (verify function, Exception class)
                }
                'key?' means ignore raise Exception if date.get('key') is None
        @return verified dictionary
        
        e.g.:
            Verify.dict({'id': '1', 'name': 'a1'}, {
                'id': (lambda x: x.type('string') and x > 0, ApiError(400, 1, 'invalid id')),
                'name': (lambda x: x.type('string') and x.match(r'\w\d') and len(x) < 10, ApiError(400, 2, 'invalid name')),
            })
            -> {'id': 1, 'name': 'a1'}

            Verify.dict({'id': '1'}, {
                'id': (lambda x: x.type('string') and x > 0, ApiError(400, 1, 'invalid id')),
                'name?': (lambda x: x.type('string') and x.match(r'\w\d') and len(x) < 10, ApiError(400, 2, 'invalid name')),
            })
            -> {'id': 1}
            
            Verify.dict({'id': '1'}, {
                'id': (lambda x: x.type('string') and x > 0, ApiError(400, 1, 'invalid id')),
                'name': (lambda x: x.type('string') and x.match(r'\w\d') and len(x) < 10, ApiError(400, 2, 'invalid name')),
            })
            -> raise ApiError(400, 2, 'invalid name')
        '''
        ignore = False # ignore mismatch
        ret = {}
        
        for key in verify:
            (fn, excep) = verify.get(key)
            
            ignore = True if key[-1] == '?' else False
            key = key[:-1] if key[-1] == '?' else key
            
            value = data.get(key)
            
            # check value exists
            if value == None:
                if ignore:
                    continue
                else:
                    raise excep
                
            # check value
            ins = VerifyParam(value)
            if not fn(ins):
                raise excep
                
            # append to new dict
            ret[key] = ins.get()
            
        return ret

        
    @staticmethod
    def array(arr, fn, excep_class, ignore_mismatch=False):
        '''
        array verification
        @param arr:
            [array] The array that you want to verify
        @param fn:
            [function] verify function
        @param excep_class:
            [Exception] exception class
        @param ignore_mismatch:
            [bool] ignore raise exception if the item mismatch
        @return verified array
            
        e.g.:
            Verify.array([1, 2, 3, 4, 5, 'a', '7', '8'], lambda x: x.type('int') and x > 0, ApiError(400, 1, 'invalid key'), True)
            -> [1, 2, 3, 4, 5, 7, 8]
            
            Verify.array([1, 2, 3, 4, 5, 'a', '7', '8'], lambda x: x.type('int') and x > 0, ApiError(400, 1, 'invalid key'), False)
            -> raise ApiError(400, 1, 'invalid key')
        '''
        ret = []
        for item in arr:
            ins = VerifyParam(item)
            
            if not fn(ins):
                if ignore_mismatch:
                    continue
                else:
                    raise excep_class
                
            ret.append(ins.get())
        return ret


if __name__ == '__main__':
    
    from api_error import ApiError
    
    print Verify.dict({'vlan': '123'}, {
        'vlan':  (lambda x: x.type('int') and x >= 1 and x <= 4095,
                  ApiError(400, 1, 'invalid vlan')),
        'name?': (lambda x: x.type('string') and len(x) < 15 and x.match(r'^\w+\d+$'), 
                  ApiError(400, 2, 'invalid name')),
    })
    
    print Verify.array([1, 2, 3, 4, 5, 'a', '7', '8'], lambda x: x.type('int') and x > 0, ApiError(400, 1, 'invalid key'), True)
    
    pass
    
    

