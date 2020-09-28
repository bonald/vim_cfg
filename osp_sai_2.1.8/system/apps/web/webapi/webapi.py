#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on Aug 20, 2016

@author: wangqj
'''

import types

import base
from flask import jsonify, request, session

from api_error import ApiError
from parse import Parser

class WebApi(object):
    
    def __init__(self, auth=True):
        '''
        @param auth:
            [bool] enable/disable token authentication
        '''
        # private
        self.__auth_en = auth
        self.__app = self.__get_web_app()
        self.__err = {}
        
        # public
        self.req_data = {}
        self.req_args = {}

        # route - function table
        self.fn_tbl = {} # url + method, e.g.: /api/v1/vlan/GET

        # default method - status code table
        self.default_method_st_tbl = {'GET': 200, 'POST': 201, 'PUT': 200, 'DELETE': 204, 'PATCH': 200}
    
    def __get_web_app(self):
        '''
        get flask app
        '''
        return base.web_app.app
    
    def __authentication(self):
        '''
        verify token from request
        '''
        token = None
        if request.method == 'GET':
            token = self.req_args.get('token')
        else:
            token = self.req_data.get('token')
        
        if token == None:
            raise ApiError(401, 1, 'no token')
    
        if session.get(token) == None:
            raise ApiError(401, 2, 'token to exceed the time limit')
    
    def __execute(self, *args, **kwargs):
        '''
        call execute
        '''
        ret = {}
        http_code = self.default_method_st_tbl.get(request.method)
        res = None

        try:
            # parse request data and args
            self.req_args = self.__get_req_arg()
            self.req_data = self.__get_req_data()

            # authentication
            if self.__auth_en:
                self.__authentication()
            
            # call excute function
            fn = self.fn_tbl.get(str(request.url_rule) + request.method)
            args = self.__unicode_to_str(args)
            kwargs = self.__unicode_to_str(kwargs)
            res = fn(*args, **kwargs)
                
        except ApiError, e:
            self.__err['error'] = {
                'code': e.error_code,
                'detail': e.detail,
            }
            http_code = e.http_code
            ret = self.__err

        # analyze res
        if res == None: # None
            pass
        elif type(res) == types.TupleType and len(res) == 2: # dict, http_code
            (ret, http_code) = res
        elif type(res) == types.IntType: # http_code
            http_code = res
        elif type(res) == types.DictionaryType: # dict
            ret = res
        else:
            raise Exception('method response error')
            
        return jsonify(ret), http_code
    
    def __unicode_to_str(self, data):
        '''
        recursion replace unicode string/int/float to string
        '''
        if type(data) == types.UnicodeType: # boundary
            data = Parser.string(data)
        elif type(data) == types.ListType:
            arr = []
            for i in data:
                arr.append(self.__unicode_to_str(i))
            if len(arr) == 1:
                arr = arr[0]
            data = arr
        elif type(data) == types.DictionaryType:
            obj = {}
            for key in data:
                obj[Parser.string(key)] = self.__unicode_to_str(data[key])
            data = obj
            
        return data
    
    def __get_req_arg(self):
        '''
        get request arguments for GET
        '''
        obj = dict(request.args)
        return self.__unicode_to_str(obj)
    
    def __get_req_data(self):
        '''
        get request data for POST/PUT/DELETE
        '''
        obj = Parser.dict(request.data)
        return self.__unicode_to_str(obj)
    
    def register_url(self, url, fn, method):
        '''
        register url to flask
        '''
        # update route - function table
        self.fn_tbl[url + method] = fn
        # make identifier
        identifier = url + str(id(self)) + method
        # register
        self.__app.add_url_rule(url, identifier, self.__execute, methods=[method])
        return self
    
    
    
    
