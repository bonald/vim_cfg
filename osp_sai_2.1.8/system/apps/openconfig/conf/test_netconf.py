#!/usr/bin/python
#-*- coding: utf-8 -*-

#
# December 23 2014, Christian Hopps <chopps@gmail.com>
#
# Copyright (c) 2015, Deutsche Telekom AG
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
from __future__ import absolute_import, division, unicode_literals, print_function, nested_scopes
import getpass
import logging
from lxml import etree
from netconf import client
from netconf import server
from netconf.error import RPCError
import sys


from python_module.oc_lr import *
from python_module.oc_if import *
import pyangbind.lib.pybindJSON as pybindJSON
import os
from base.xml2json import *

print("start")

# Instantiate a copy of the pyangbind-kettle module
oclr = openconfig_local_routing()

# Add an entry to the static route list
rt = oclr.local_routes.static_routes.static.add("192.0.2.1/32")
jsondata = pybindJSON.dumps(oclr)
print(jsondata)

# Dump the static routes instance as JSON in IETF format
print(pybindJSON.dumps(oclr.local_routes, mode="ietf"))

global xml
xml = json2xml(jsondata)



logger = logging.getLogger(__name__)
nc_server = None
NC_PORT = 2540
NC_DEBUG = True


class NetconfMethods (server.NetconfMethods):
    def rpc_get (self, unused_session, rpc, *unused_params):
        global xml
        print('aaa')
        #return etree.Element(reply)
        #return etree.Element("ok")
        return xml


def setup_module (unused_module):
    global nc_server

    logging.basicConfig(level=logging.DEBUG)

    if nc_server is not None:
        logger.error("XXX Called setup_module multiple times")
    else:
        sctrl = server.SSHUserPassController(username=getpass.getuser(),
                                             password="admin")
        #sctrl = server.SSHUserPassController(username="admin",
        #                                     password="admin")
        nc_server = server.NetconfSSHServer(server_ctl=sctrl,
                                            server_methods=NetconfMethods(),
                                            port=NC_PORT,
                                            host_key="/data01/users/sw/chenyq2/tmp/netconf-0.5.3/tests/host_key",
                                            debug=NC_DEBUG)


def test_query ():
    query = """
    <get>
    <filter>
    <devices xmlns="http://tail-f.com/ns/ncs">
    <global-settings/>
    </devices>
    </filter>
    </get>
    """
    logger.info("Connecting to 127.0.0.1 port %d", nc_server.port)
    session = client.NetconfSSHSession("127.0.0.1",
                                       username=getpass.getuser(),
                                       password="admin",
                                       port=nc_server.port,
                                       debug=NC_DEBUG)
    return session.send_rpc(query)


def test_bad_query ():
    session = client.NetconfSSHSession("127.0.0.1",
                                       username=getpass.getuser(),
                                       password="admin",
                                       port=nc_server.port,
                                       debug=NC_DEBUG)
    try:
        unused, unused, output = session.send_rpc("<get><unknown/></get>")
        logger.warning("Got unexpected output: %s", str(output))
    except RPCError:
        pass

__author__ = 'Christian Hopps'
__date__ = 'December 23 2014'
__version__ = '1.0'
__docformat__ = "restructuredtext en"

if __name__ == '__main__':
    print('abc')
    print(getpass.getuser())
    setup_module('test')
    res = test_query()
    print(res)







