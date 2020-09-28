#!/usr/bin/env python
# encoding: utf-8
'''
Copyright 2014 Centec Networks Inc. All rights reserved.

@summary:      User management for authentication
@author:       zhaoy
@organization: Centec Networks
@copyright:    2014 Centec Networks Inc. All rights reserved.
'''

class User(object):
    __user_name__ = ''
    __user_password__ = ''
    __user_priv__ = 0

    def __init__(self):
        pass

    def updateName(self, userName):
        self.__user_name__ = userName

    def getName(self):
        return self.__user_name__

    def getPassword(self):
        return self.__user_password__

class users(object):
    __user_management_enable__ = False
    __users__ = {}

    def __init__(self):
        # add user defined users
        self.addUserDefinedUsers()
        self.enableUserManagement(True)

    def addUserDefinedUsers(self):
		szmsg = execute_cmd('cdbctl read/cdb/app/user')
		print szmsg

    def enableUserManagement(self, enable):
        self.__user_management_enable__ = enable

    def getUserManagementEnable(self):
        return self.__user_management_enable__

    def updateUser(self, userName, userPassword):
        if self.__users__.has_key(userName):
            user = self.__users__[userName]
            user.updateName(userName)
            user.updatePassword(userPassword)
        else:
            user = User()
            user.updateName(userName)
            user.updatePassword(userPassword)
            self.__users__[userName] = user

    def delUser(self, userName):
        if self.__users__.has_key(userName):
            del self.__users__[userName]

    def authenUser(self, userName, userPassword):
        if self.__users__.has_key(userName):
            user = self.__users__[userName]
            if user.getPassword() == userPassword:
                return True
        return False

    def listUsers(self):
        for userName in self.__users__:
            user = self.__users__[userName]
            print user
