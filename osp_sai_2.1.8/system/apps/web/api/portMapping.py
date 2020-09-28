#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os

class Singleton(object):
    def __new__(cls, *args, **kwargs):
        if not hasattr(cls, '_inst'):
            cls._inst = super(Singleton, cls).__new__(cls, *args, **kwargs)
        return cls._inst


class portNameMapping(Singleton):
    def __init__(self):

        if hasattr(self, 'default_port_type'):
            return

        self.default_port_type = {
            "prefix": "eth-",
            "full_prefix": "eth-"
        }

        self.port_type_connection = {
            "connection": "-",
            "sub_connection": "/"
        }

        self.port_type_arrys = []
        self.port_info_maps = {}

        self.__load_ctcos_port_type()
        self.__load_ctcos_port_info()

    def __load_ctcos_port_type(self, file_path='/tmp/ctcos_port_cli_info'):
        try:
            with open(file_path) as file:
                for line in file:
                    if line.startswith('port_connection'):
                        line_arrs = line.split()
                        self.port_type_connection["connection"] = line_arrs[1]
                        self.port_type_connection["sub_connection"] = line_arrs[3]
                        continue
                    if line.startswith('prefix'):
                        line_arrs = line.split()
                        self.port_type_arrys.append({'prefix': line_arrs[1], 'full_prefix': line_arrs[3]})
                        continue
        except IOError:
            return

    def __get_types_with_port_type_id(self, type_id=0):
        if 0 == type_id:
            return self.default_port_type
        if 0 == len(self.port_type_arrys):
            return self.default_port_type

        return self.port_type_arrys[type_id - 1]

    def __load_ctcos_port_info(self, file_path='/tmp/ctcos_port_info'):
        try:
            with open(file_path) as file:
                for line in file:
                    line_arrs = line.split()
                    port_node = {}
                    port_node['port_id'] = line_arrs[3]
                    port_node['port_type'] = self.__get_types_with_port_type_id(int(line_arrs[1]))
                    port_node['connection'] = self.port_type_connection
                    if not self.port_info_maps.has_key(line_arrs[2]):
                        self.port_info_maps[line_arrs[2]] = {}
                    self.port_info_maps[line_arrs[2]][line_arrs[3]] = port_node
        except IOError:
            return

    def Mapping(self, portName, is_sort=True):
        # eth-0-1 or eth-0-1/1
        split_id = 0
        if not portName.startswith('eth-') :
            return portName
        if not self.port_info_maps :
            return portName

        if -1 != portName.find('_'):
            sub_line = portName.split('_')
            split_id = sub_line[1]
            orig_portname = sub_line[0]
        else:
            orig_portname = portName

        sub_line = orig_portname.split('-')
        slot_id = sub_line[1]
        port_id = sub_line[2]

        if is_sort:
            retPortName = self.port_info_maps[slot_id][port_id]['port_type']['prefix']
        else:
            retPortName = self.port_info_maps[slot_id][port_id]['port_type']['full_prefix']
        retPortName += slot_id
        retPortName += self.port_info_maps[slot_id][port_id]['connection']['connection']
        retPortName += port_id

        if split_id != 0:
            retPortName += self.port_info_maps[slot_id][port_id]['connection']['sub_connection']
            retPortName += split_id

        return retPortName

    def getPortConnection(self):
        return self.port_type_connection


def main():
    portMapping = portNameMapping()
    portMapping1 = portNameMapping()
    name = portMapping.Mapping('eth-0-1')
    print name
    name = portMapping.Mapping('eth-0-1_1')
    print name

    name = portMapping1.Mapping('eth-0-1', False)
    print name
    name = portMapping1.Mapping('eth-0-1_1', False)
    print name

    name = portMapping.Mapping('eth-0-2')
    print name
    name = portMapping.Mapping('eth-0-2_2')
    print name

    name = portMapping.Mapping('eth-0-3')
    print name
    name = portMapping.Mapping('eth-0-3_3')
    print name

    name = portMapping.Mapping('eth-0-4')
    print name
    name = portMapping.Mapping('eth-0-4_4')
    print name


if __name__ == '__main__':
    main()