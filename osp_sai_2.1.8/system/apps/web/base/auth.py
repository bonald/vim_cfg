#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, session, jsonify, redirect, url_for
from functools import wraps
from hashlib import md5
import json
import random
import time
import commands
from vcl import vcmd
import network

class UserAuthBase(object):
    
    def verify_password(self, username, password, csess):
        pass
    
    def update_password(self, username, password):
        pass
    
    def delete_user(self, username):
        pass
    
class UserAuth(UserAuthBase):
    
    def verify_password(self, username, password, csess):
        rip = request.remote_addr
        rport = request.environ.get('REMOTE_PORT')
        if rip == "127.0.0.1":
            vrip = vcmd.getRemoteIP()
            if vrip != None:
                rip = vrip
        # user name may conation special chars (eg: %)
        enccmd = 'fnconvert -c encoding -m \'%s\'' %(str(username))
        status, encuser = vcmd.get_status_lines_sp(enccmd)

        exec_str = "cdbctl \'update/cdb/sys/web_login/%s/user/%s/passwd/%s/ipaddr/%s/sport/%s\'" %(str(csess), str(encuser[0]), str(password), str(rip),str(rport))
        print exec_str
        status, output = vcmd.get_status_lines(exec_str)
        
        if status == 0 or output[0] == '% Web session exist': 
            return True, ''
        return False, output[0]
    
    def update_password(self, username, password):
        pass
    
    def delete_user(self, username):
        pass

class ApiAuth(object):
    
    def __init__(self):
        self.__md5_ins = md5()
        self.__user_ins = UserAuth()
        pass

    def login_required(self, f):
        """
        API:
        
        GET/POST/PUT/DELETE {
           token: md5,
           username: string,
        }
        
        Retrun {
            err_auth: bool,
            err_code: int,
            err_reason: string,
        }
        """
        @wraps(f)
        def decorated(*args, **kwargs):
            _err_reason = [
                '',                # err_code: 0
                'no token',        # err_code: 1
                'token to exceed the time limit', # err_code: 2
            ]
            
            ret_obj = {'err_auth': False, 'error': False, 'err_code': 0, 'err_reason': ''}
            
            # get token and username
            if request.args.has_key('token'):
                token = request.args.get('token')
            else:
                try:
                    obj = json.loads(request.data)
                    token = obj.get('token')
                except ValueError:
                    token = None
                
            # if no token
            if token == None:
                ssl_en = commands.getoutput(r"cat /var/web/http.conf | grep ssl_en | sed 's/ssl_en=\([^\/]\+\).*/\1/'")
                if (ssl_en != ''):
                    int_ssl_en = int(ssl_en)
                else:
                    int_ssl_en = 0
                if str(request.url_rule) == '/':
                    if (int_ssl_en == 1):
                        if request.url.startswith('http://'):
                           url = request.url.replace('http://', 'https://', 1) + 'static/switch/index.html'
                        return redirect(url)
                    else:
                        return redirect(url_for('static', filename='switch/index.html'))

                ret_obj['err_auth'] = True
                ret_obj['err_code'] = 1
                ret_obj['err_reason'] = _err_reason[1]
                return jsonify(ret_obj)
            
            # check token
            obj = session.get(token)
            if obj == None:
                ret_obj['err_auth'] = True
                ret_obj['err_code'] = 2
                ret_obj['err_reason'] = _err_reason[2]
                return jsonify(ret_obj)
             
            # success
            return f(*args, **kwargs)
        return decorated
    
    def __token_match(self, token, uid):
        """
        match token by userid
        """
        return False
    
    def __auth_error(self):
        pass

    def make_token(self, obj):
        """
        POST {
           username: string,
           password: string
        }
        
        Retrun {
            token: md5 or None,
            error: bool,
            err_code: int,
            err_reason: string,
        }
        
        """
        _err_reason = [
            '',                             # err_code: 0
            'bad request',                  # err_code: 1
            'No such user',                 # err_code: 2
            'Too many web users',           # err_code: 3
            'Password error',               # err_code: 4
            'Privilege not enough',         # err_code: 5
            'Service type mismatch',         # err_code: 6
        ]
            
        ret_obj = {'token': None, 'error': False, 'err_code': 0, 'err_reason': ''}
        
        #param check
        username = obj.get('username')
        password = obj.get('password')
        if username == None or password == None:
            ret_obj['error'] = True
            ret_obj['err_code'] = 1
            ret_obj['err_reason'] = _err_reason[1]
            return ret_obj

        #login
        # make session
        md5_src = username + str(time.time()) + '_' + str(random.randint(0, 10000))
        self.__md5_ins.update(md5_src)
        md5_str = self.__md5_ins.hexdigest()
        
        res, err_str = self.__user_ins.verify_password(username, password, md5_str)
        if res == False:
            ret_obj['error'] = True
            ret_obj['err_code'] = 2
            if err_str == '% Too many web users':
                ret_obj['err_reason'] = _err_reason[3]
            elif err_str == '% No such user':
                ret_obj['err_reason'] = _err_reason[2]
            elif err_str == '% Privilege denied':
                ret_obj['err_reason'] = _err_reason[5]
            elif err_str == '% Permission denied':
                ret_obj['err_reason'] = _err_reason[4]
            elif err_str == '% User is not supported for web':
                ret_obj['err_reason'] = _err_reason[6]
            else: 
                ret_obj['err_reason'] = _err_reason[1]
            return ret_obj
            
        #login successful
        ret_obj['token'] = md5_str
        
        #update session
        session[md5_str] = True;
        session['token'] = md5_str;
        
        return ret_obj
 
    def del_token(self, obj):
        """
        DELETE {
           token: string,
        }
        
        Retrun {
            error: bool,
            err_code: int,
            err_reason: string,
        }
        
        """
        _err_reason = [
            '',                             # err_code: 0
            'bad request',                  # err_code: 1
            'invalid token',                # err_code: 2
        ]
            
        ret_obj = {'error': False, 'err_code': 0, 'err_reason': ''}
        
        #param check

        token = obj.get('token')
        if token == None:
            ret_obj['error'] = True
            ret_obj['err_code'] = 1
            ret_obj['err_reason'] = _err_reason[1]
            return ret_obj

        # delete web session
        exec_str = 'cdbctl delete/cdb/app/login/%s ' %(str(token))
        status, output = vcmd.get_status_lines(exec_str)
        return ret_obj

    def get_product(self):
        obj = {'system_product': [], 'error': False, 'err_code': 0, 'err_reason': ''}
        print 1
        exec_str = 'cdbctl read/cdb/sys/sys_global/system_product/'
        status, lines_1 = vcmd.get_status_lines(exec_str)
        print status
        print lines_1
        lines = lines_1[0]
        print lines
        if status != 0:
            return None
        else:
            if lines == '/system_product=0':
                product = 'CNOS'
                print product
            elif lines == '/system_product=1':
                product = 'Hybrid'
                print product
            elif lines == '/system_product=2':
                product = 'Tap'
                print product

        exec_str = 'cdbctl read/cdb/sys/chip/'
        status, lines_2 = vcmd.get_status_lines(exec_str)

        lines = lines_2[0]

        print lines
        if status != 0:
            return None
        else:
            if lines == '/type=Goldengate':
                chip = '580'
                print chip
            elif lines == '/type=Greatbelt':
                chip = '350'
                print chip
            elif lines == '/type=Duet2':
                chip = '550'
                print chip

            elif lines == '/type=Tsingma':
                chip = '530'
                print chip

        system_product = product+'-'+chip
        print system_product
        obj['system_product'] = system_product
        print obj
        return obj

    def update_session_tbl(self):
            if session != None:
                token = session.get('token')
                if token != None:
                    exec_str = 'cdbctl read/cdb/app/login/%s >/dev/null 2>&1' % (str(token));
                    status, output = vcmd.get_status_lines(exec_str);
                    if status != 0:
                        # session being clear or timeout
                        session.pop('token')
                        session.pop(token)
                        return redirect(url_for('static', filename='switch/index.html'))

    if __name__ == '__main__':
        pass