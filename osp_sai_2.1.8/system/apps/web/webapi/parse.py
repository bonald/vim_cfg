#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on Aug 20, 2016

@author: wangqj
'''

import json
import types
import base64

class Parser(object):
    
    @staticmethod
    def int(string, error=None):
        '''
        parse int
        e.g.: 
            '1' -> 1
            'a' -> error
            '1a' -> error
            'a1' -> error
        '''
        res = error
        try:
            res = int(string)
        except Exception:
            pass
        return res
    
    @staticmethod
    def float(string, error=None):
        '''
        parse float
        '''
        res = error
        try:
            res = float(string)
        except Exception:
            pass
        return res
                
                
    @staticmethod
    def dict(string, error={}):
        '''
        parse dictionary
        '''
        res = error
        try:
            res = json.loads(string)
        except Exception:
            pass
        
        if type(res) != types.DictionaryType:
            res = error
        
        return res
    
    @staticmethod
    def base64(string, error=''):
        '''
        decode base64
        '''
        res = error
        try:
            res = base64.decodestring(str(string))
        except Exception:
            pass
        
        return res    
    
    @staticmethod
    def string(string, error=''):
        res = error
        try:
            res = str(string)
        except Exception:
            pass
        
        return res    
    
    
    
    