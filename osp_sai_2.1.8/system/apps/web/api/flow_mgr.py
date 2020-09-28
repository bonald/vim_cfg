#!/usr/bin/python
#-*- coding: utf-8 -*-

from vcl import vcmd
import string
import base64

def flow_table_add(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    flow_name = str(req_data.get('name'))
    flow_type = str(req_data.get('type'))
    decap     = str(req_data.get('decap'))

    if flow_type == 'flow':
        cmd = 'cdbctl create/cdb/l2/acl_config/%s/type/flow' % (flow_name)
    elif flow_type == 'flow_match':
        cmd = 'cdbctl create/cdb/l2/acl_config/%s/type/flow_match' % (flow_name)
    else:
        cmd = 'cdbctl create/cdb/l2/acl_config/%s' % (flow_name)

    if decap == 'True':
        cmd += "/flow_ace_type/decap"

    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ' '.join(lines)
        return obj

    return obj 

def flow_table_get():
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'acl': [], 'flow': [], 'flow_match': [],'error': False, 'err_code': 0, 'err_reason': ''}

    cmd = 'cdbctl read/cdb/l2/acl_config/'
    arr = vcmd.get_arr(cmd)

    # change '/key=#FLOW#flow1/'  to '/key=flow1/'
    for entry in arr:
        key = entry['key']

        if '#' in key:
            name_list = key.split('#')
            entry['key'] = name_list[-1]
            if 'FLOW' in key:
                obj['flow'].append(entry)
            else:
                obj['flow_match'].append(entry)
        else:
            obj['acl'].append(entry)

    return obj      

def flow_table_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    flow_name = str(req_data.get('name'))
    flow_type = str(req_data.get('type'))

    if flow_type == 'flow':
        cmd = 'cdbctl delete/cdb/l2/acl_config/%s/type/flow' % (flow_name)
    elif flow_type == 'flow_match':
        cmd = 'cdbctl delete/cdb/l2/acl_config/%s/type/flow_match' % (flow_name)
    else:
        cmd = 'cdbctl delete/cdb/l2/acl_config/%s' % (flow_name)

    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ' '.join(lines)
        return obj

    return obj    

def flow_acl_config_remark_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    flow_name = str(req_data.get('name'))
    flow_type = str(req_data.get('type'))   

    if flow_type == 'flow':
        cmd = 'cdbctl update/cdb/l2/acl_config/%s/remark/del/type/flow' % (flow_name)
    elif flow_type == 'acl':
        cmd = 'cdbctl update/cdb/l2/acl_config/%s/remark/del' % (flow_name)
    else:
        cmd = 'cdbctl update/cdb/l2/acl_config/%s/remark/del/type/flow_match' % (flow_name)

    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ' '.join(lines)
        return obj 

    return obj

def flow_acl_config_remark_add(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    flow_name = str(req_data.get('name'))
    remark = str(req_data.get('remark'))
    flow_type = str(req_data.get('type'))   

    if flow_type == 'flow':
        cmd = 'cdbctl update/cdb/l2/acl_config/%s/remark/add/%s/type/flow' % (flow_name, remark)
    elif flow_type =='acl':
        cmd = 'cdbctl update/cdb/l2/acl_config/%s/remark/add/%s' % (flow_name, remark)
    else:
        cmd = 'cdbctl update/cdb/l2/acl_config/%s/remark/add/%s/type/flow_match' % (flow_name, remark)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ' '.join(lines)
        return obj

    return obj   

##########
##  flow entry 
##########

def flow_entry_add(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    #print req_data

    flowName = str(req_data.get('flowName'))
    flow_type = str(req_data.get('type'))    
    dir = str(req_data.get('dir'))
    ipProtocolNumber = str(req_data.get('ipProtocolNumber'))
    protocolNum = str(req_data.get('protocolNum'))

    IpType = str(req_data.get('Ipv4v6Type'))
    srcIpEn = str(req_data.get('srcipEn'))
    srcIp = str(req_data.get('srcIp'))
    srcIpMask = str(req_data.get('srcIpMask'))

    dstIpEn = str(req_data.get('dstipEn'))
    dstIp = str(req_data.get('dstIp'))
    dstIpMask = str(req_data.get('dstIpMask'))

    srcIpv6En = str(req_data.get('srcipv6En'))
    srcIpv6 = str(req_data.get('srcIpv6'))
    srcIpv6Mask = str(req_data.get('srcIpv6Mask'))

    dstIpv6En = str(req_data.get('dstipv6En'))
    dstIpv6 = str(req_data.get('dstIpv6'))
    dstIpv6Mask = str(req_data.get('dstIpv6Mask'))

    #print "dstIpv6Mask", dstIpv6Mask

    ethertypeEn = str(req_data.get('ethertypeEn'))
    etherkeyType = str(req_data.get('etherkeyType'))
    etherValue = str(req_data.get('etherValue'))
    etherMask = str(req_data.get('etherMask'))

    #print ipProtocolNumber, protocolNum, IpType, ethertypeEn, etherkeyType, etherValue, etherMask

    icmpcodeEn = str(req_data.get('icmpcodeEn'))
    icmpcodeValue = str(req_data.get('icmpcodeValue'))
    icmptypeEn = str(req_data.get('icmptypeEn'))
    icmptypeValue = str(req_data.get('icmptypeValue'))

    srcPortEn = str(req_data.get('srcPortEn'))
    srcPortType = str(req_data.get('srcPortType'))
    srcPortValue = str(req_data.get('srcPortValue'))

    destPortEn = str(req_data.get('destPortEn'))
    destPortType = str(req_data.get('destPortType'))
    destPortValue = str(req_data.get('destPortValue'))

    stripHeaderEn = str(req_data.get('stripHeaderEn'))
    stripPositionEn = str(req_data.get('stripPositionEn'))
    stripPositionValue = str(req_data.get('stripPositionValue'))
    stripOffsetEn = str(req_data.get('stripOffsetEn'))
    stripOffsetValue = str(req_data.get('stripOffsetValue'))
    stripInnerVxlanEn = str(req_data.get('stripInnerVxlanEn'))

    editMacDaEn = str(req_data.get('editMacDaEn'))
    editMacDaValue = str(req_data.get('editMacDaValue'))
    editMacSaEn = str(req_data.get('editMacSaEn'))
    editMacSaValue = str(req_data.get('editMacSaValue'))
    editIPDaEn  = str(req_data.get('editIPDaEn'))
    editdstIpType = str(req_data.get('editdstIpType'))
    editIPDaValue = str(req_data.get('editIPDaValue'))
    editIPv6DaValue = str(req_data.get('editIPv6DaValue'))
    editIPSaEn  = str(req_data.get('editIPSaEn'))
    editsrcIpType = str(req_data.get('editsrcIpType'))
    editIPSaValue = str(req_data.get('editIPSaValue'))
    editIPv6SaValue = str(req_data.get('editIPv6SaValue'))
    editVlanEn     = str(req_data.get('editVlanEn'))
    editVlanType   = str(req_data.get('editVlanType'))
    editVlanValue  = str(req_data.get('editVlanValue'))

    addl2greEn = str(req_data.get('addl2greEn'))
    addl2grekeyType = str(req_data.get('addl2grekeyType'))
    l2grenum = str(req_data.get('l2grenum'))
    entryL2greMacDa = str(req_data.get('entryL2greMacDa'))
    entryL2greIPDa = str(req_data.get('entryL2greIPDa'))
    entryL2greIPSa = str(req_data.get('entryL2greIPSa'))

    addl3greEn =  str(req_data.get('addl3greEn'))
    entryL3greMacDa =  str(req_data.get('entryL3greMacDa'))
    entryL3greIPDa = str(req_data.get('entryL3greIPDa'))
    entryL3greIPSa = str(req_data.get('entryL3greIPSa'))

    srcMacEn = str(req_data.get('srcMacEn'))
    srcMacType = str(req_data.get('srcMacType'))
    srcMacValue = str(req_data.get('srcMacValue'))
    srcMacMask = str(req_data.get('srcMacMask'))

    destMacEn = str(req_data.get('destMacEn'))
    destMacType = str(req_data.get('destMacType'))
    destMacValue = str(req_data.get('destMacValue'))
    destMacMask = str(req_data.get('destMacMask'))

    VlanEn = str(req_data.get('VlanEn'))
    vlanID = str(req_data.get('vlanID'))
    vlanMask = str(req_data.get('vlanMask'))

    InnerVlanEn = str(req_data.get('InnerVlanEn'))
    innervlanID = str(req_data.get('innervlanID'))
    innervlanMask = str(req_data.get('innervlanMask'))

    trunEn = str(req_data.get('trunEn'))
    removeVlanEn = str(req_data.get('removeVlanEn'))

    vniEn = str(req_data.get('vniEn'))
    VniID = str(req_data.get('VniID'))
    VniMask = str(req_data.get('VniMask'))

    MarkSourceID = str(req_data.get('MarkSourceID'))   

    innerMatchEn = str(req_data.get('innerMatchEn'))
    innerFlow    = str(req_data.get('innerFlow'))

    udfEn     = str(req_data.get('udfEn'))
    udfType   = str(req_data.get('udfType'))
    udfValue  = str(req_data.get('udfValue'))
    udfMask   = str(req_data.get('udfMask'))
    udfOffset = str(req_data.get('udfOffset'))

    greEn     = str(req_data.get('greEn'))
    greValue  = str(req_data.get('greValue'))
    greMask   = str(req_data.get('greMask'))

    erspanEn  = str(req_data.get('erspanEn'))
    erspanValue = str(req_data.get('erspanValue'))
    erspanMask = str(req_data.get('erspanMask'))

    nvgreEn   = str(req_data.get('nvgreEn'))
    nvgreVNI  = str(req_data.get('nvgreVNI'))
    nvgreMask = str(req_data.get('nvgreMask'))

    dscpEn    = str(req_data.get('dscpEn'))
    dscp      = str(req_data.get('dscp'))
    tosEn     = str(req_data.get('tosEn'))
    tos       = str(req_data.get('tos'))

    optionsEn = str(req_data.get('optionsEn'))
    fragEn = str(req_data.get('fragEn'))
    cosEn = str(req_data.get('cosEn'))
    cosValue = str(req_data.get('cosValue'))
    innercosEn = str(req_data.get('innercosEn'))
    innercosValue = str(req_data.get('innercosValue'))

    flowlabelEn     = str(req_data.get('flowlabelEn'))
    flowlabelValue  = str(req_data.get('flowlabelValue'))
    flowlabelMask   = str(req_data.get('flowlabelMask'))


    if dir == "permit":
        cmd ="cdbctl create/cdb/l2/ace_config/%s/0/permit" % flowName
    else:
        cmd ="cdbctl create/cdb/l2/ace_config/%s/0/deny" % flowName

    if flow_type == "flow":
        cmd = cmd + "/type/flow"
    elif flow_type == "flow_match":
        cmd = cmd + "/type/flow_match"
    else:
        pass

    if srcMacEn == 'True':
        cmd = cmd + "/src_mac"

        if srcMacType == 'any':
            cmd = cmd + "/any//"
        elif srcMacType == 'mac':
            cmd = cmd + "/src_mac/%s/%s" % (srcMacValue, srcMacMask)
        elif srcMacType == 'host':
            cmd = cmd + "/host/%s/" % srcMacValue
    else:
        cmd = cmd + "/src_mac///"

    if destMacEn == 'True':
        cmd = cmd + "/dest_mac"

        if destMacType == 'any':
            cmd = cmd + "/any//"
        elif destMacType == 'mac':
            cmd = cmd + "/dest_mac/%s/%s" % (destMacValue, destMacMask)
        elif destMacType == 'host':
            cmd = cmd + "/host/%s/" % destMacValue
    else:
        cmd = cmd + "/dest_mac///"

    if VlanEn == 'True':
        cmd = cmd + "/vlan/vlan_id/%s/%s" % (vlanID, vlanMask)
    else:
        cmd = cmd + "/vlan///"

    if InnerVlanEn == 'True':
        cmd = cmd + "/inner_vlan/vlan_id/%s/%s" % (innervlanID, innervlanMask)
    else:
        cmd = cmd + "/inner_vlan///"

    if cosEn == 'True':
        cmd = cmd + "/cos/cos/%s/0" % cosValue
    else:
        cmd = cmd + "/cos///0"

    if innercosEn == 'True':
        cmd = cmd + "/inner_cos/inner_cos/%s/0" % innercosValue
    else:
        cmd = cmd + "/inner_cos///0"

    if ethertypeEn != 'True':
        if IpType == 'ipv4':
            cmd = cmd + "/ether_type///"

            if srcIpEn != 'True':
                cmd = cmd + "/src-ip/any//"
            else:
                cmd = cmd + "/src-ip/src_ip/%s/%s" % (srcIp, srcIpMask)

            if dstIpEn != 'True':
                cmd = cmd + "/dst-ip/any//"
            else:
                cmd = cmd + "/dst-ip/dest_ip/%s/%s" % (dstIp, dstIpMask)
        else :
            cmd = cmd + "/ether_type///"
            if srcIpv6En != 'True':
                cmd = cmd + "/src-ipv6/any//"
            else:
                cmd = cmd + "/src-ipv6/src_ip/%s/%s" % (srcIpv6, srcIpv6Mask)

            if dstIpv6En != 'True':
                cmd = cmd + "/dst-ipv6/any//"
            else:
                cmd = cmd + "/dst-ipv6/dest_ip/%s/%s" % (dstIpv6, dstIpv6Mask)

        if dscpEn == 'True' and len(dscp):
            cmd += "/dscp/%s/%s/0" % (dscp, dscp)
        else:
            cmd += "/dscp///0"

        if tosEn == 'True' and len(tos):
            cmd += "/precedence/%s/%s/0" % (tos, tos)
        else:
            cmd += "/precedence///0"

        if fragEn == 'True':
            cmd += "/frag/any-fragment"
        else:
            cmd += "/frag/"

        if ipProtocolNumber == 'num':
            cmd = cmd + "/l3-protocol/protocol_num/%s/0" % (protocolNum)
        elif ipProtocolNumber == 'any':
            cmd = cmd + "/l3-protocol/any//0"
        elif ipProtocolNumber == 'tcp':
            cmd = cmd + "/l3-protocol/tcp//0"
        elif ipProtocolNumber == 'udp':
            cmd = cmd + "/l3-protocol/udp//0"
        elif ipProtocolNumber == 'gre':
            cmd = cmd + "/l3-protocol/gre//0"
        elif ipProtocolNumber == 'nvgre':
            cmd = cmd + "/l3-protocol/nvgre//0"
        elif ipProtocolNumber == 'icmp':
            cmd = cmd + "/l3-protocol/icmp//0"
        elif ipProtocolNumber == 'igmp':
            cmd = cmd + "/l3-protocol/igmp//0"

        if srcPortEn == 'True':
            cmd = cmd + "/src-l4-port"

            if srcPortType == 'eq':
                cmd = cmd + "/eq/%s/" % (srcPortValue)
            elif srcPortType == 'gt':
                cmd = cmd + "/gt/%s/" % (srcPortValue)
            elif srcPortType == 'lt':
                cmd = cmd + "/lt/%s/" % (srcPortValue)
            elif srcPortType == 'any':
                cmd = cmd + "/any//"
        else:
            cmd = cmd + "/src-l4-port///"

        if destPortEn == 'True':
            cmd = cmd + "/dest-l4-port"

            if destPortType == 'eq':
                cmd = cmd + "/eq/%s/" % (destPortValue)
            elif destPortType == 'gt':
                cmd = cmd + "/gt/%s/" % (destPortValue)
            elif destPortType == 'lt':
                cmd = cmd + "/lt/%s/" % (destPortValue)
            elif destPortType == 'any':
                cmd = cmd + "/any//"
        else:
            cmd = cmd + "/dest-l4-port///"

        cmd = cmd + '/igmp_type/'

        if icmptypeEn == 'True':
            cmd = cmd + '/icmp_type/%s' % icmptypeValue
            if icmpcodeEn == 'True':
                cmd = cmd + '/%s' % icmpcodeValue
            else:
                cmd = cmd + '/'
        else:
            cmd = cmd + '/icmp_type//'

        cmd = cmd + '/tcpflag///////'

    else:
        if len(etherValue) :
            cmd = cmd + "/ether_type/ether/%s/%s" % (etherValue, etherMask)
        else :
            cmd = cmd + "/ether_type/any//"
        if IpType == 'ipv4':
            cmd = cmd + '/src-ip////dst-ip////dscp////precedence////frag//l3-protocol////src-l4-port////////igmp_type//icmp_type///tcpflag///////'

    if udfEn == 'True' and len(udfValue) and len(udfMask) and len(udfOffset):
        if udfType == 'Layer3':
            cmd += "/udf/ipv4-head"
        else:
            cmd += "/udf/l4-head"

        cmd += "/%s/%s/%s" % (udfValue, udfMask, udfOffset)
    else:
        cmd += "/udf////"

    cmd += '/timerange/'

    if optionsEn == 'True':
        cmd = cmd + '/options/options'
    else :
        cmd = cmd + '/options/'

    if trunEn == 'True':
        cmd = cmd + '/truncation/true'
    else:
        cmd = cmd + '/truncation/'

    cmd += '/snmp_set//'

    if innerMatchEn == 'True' and innerFlow:
        cmd = cmd + "/flow-match/%s" % (innerFlow)

    if editMacDaEn == 'True':
        cmd = cmd + "/edit-macda/%s" % (editMacDaValue)

    if editMacSaEn == 'True':
        cmd = cmd + "/edit-macsa/%s" % (editMacSaValue)

    if editIPDaEn == 'True':
        if editdstIpType == 'ipv4':
            cmd = cmd + "/edit-ipda/%s" % (editIPDaValue)
        elif editdstIpType == 'ipv6':
            cmd = cmd + "/edit-ipv6da/%s" % (editIPv6DaValue)

    if editIPSaEn == 'True':
        if editsrcIpType == 'ipv4':
            cmd = cmd + "/edit-ipsa/%s" % (editIPSaValue)
        elif editsrcIpType == 'ipv6':
            cmd = cmd + "/edit-ipv6sa/%s" % (editIPv6SaValue)

    if editVlanEn == 'True':
        if editVlanType == 'add':
            cmd = cmd + "/add-vlan/%s" % (editVlanValue)
        else :
            cmd = cmd + "/edit-vlan/%s" % (editVlanValue)

    if addl2greEn == 'True':
        cmd = cmd + "/add-l2gre/%s/%s/%s/%s/%s" % (entryL2greIPSa, entryL2greIPDa, entryL2greMacDa, l2grenum, addl2grekeyType)

    if addl3greEn == 'True':
        cmd = cmd + "/add-l3gre/%s/%s/%s" % (entryL3greIPSa, entryL3greIPDa, entryL3greMacDa)



    if removeVlanEn == 'Disable':
        cmd = cmd + "/un-tag/false"
    elif removeVlanEn == 'double-vlan':
        cmd = cmd + "/un-tag/true"
    elif removeVlanEn == 'outer-vlan':
        cmd = cmd + "/un-tag/s-vlan"
    else:
        cmd = cmd + "/un-tag/c-vlan"

    if stripHeaderEn == 'True':
        cmd = cmd + "/strip-header/1"
        if stripPositionEn == 'True':
            cmd = cmd + '/strip-position/%s' % stripPositionValue
        if stripOffsetEn == 'True':
            cmd = cmd + '/strip-offset/%s' % stripOffsetValue
        if stripInnerVxlanEn == 'True' and ethertypeEn != 'True' and IpType == 'ipv4':
            cmd = cmd + '/strip-inner-vxlan-header/1'

    if vniEn == 'True':
        if not VniMask:
            VniMask = '0'
        cmd = cmd + "/vni/%s/%s" % (VniID, VniMask) 

    if len(MarkSourceID):
        cmd = cmd + "/mark-source/%s/" % (MarkSourceID) 

    if greEn == 'True' and len(greValue) and len(greMask):
        cmd += "/gre-key/%s/%s" % (greValue, greMask)

    if erspanEn == 'True' :
        if erspanMask == '0x3FF' or erspanMask == '0x3ff':
            cmd = cmd + '/erspan-id/any'
        else:
            cmd = cmd + '/erspan-id/%s/%s' % (erspanValue, erspanMask)

    if nvgreEn == 'True' and len(nvgreVNI) and len(nvgreMask):
        cmd += "/nvgre-vni/%s/%s" % (nvgreVNI, nvgreMask)

    if flowlabelEn == 'True':
        cmd += '/flow_lable/flow_label_value/%s/%s' % (flowlabelValue, flowlabelMask)


    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ' '.join(lines)
        return obj

    return obj

def flow_entry_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                    # err_code: 0
            'entry is null', # err_code: 1
            ]

    obj = {'entry_arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    entry = str(req_data.get('entry'))
    table = str(req_data.get('table'))
    flow_type = str(req_data.get('type'))    
    temp = entry.split()

    if flow_type == "flow":
        cmd = 'cdbctl delete/cdb/l2/ace_config/%s/%s/snmp_set//type/flow' % (table , temp[1])
    elif flow_type == "flow_match":
        cmd = 'cdbctl delete/cdb/l2/ace_config/%s/%s/snmp_set//type/flow_match' % (table , temp[1])
    else:
        cmd = 'cdbctl delete/cdb/l2/ace_config/%s/%s/snmp_set//' % (table , temp[1])

    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ' '.join(lines)
        return obj

    return obj

def flow_entry_get(table, flowTyep):
    """
    TBD
    """

    _err_reason = [
            '',                    # err_code: 0
            'entry is null', # err_code: 1
            ]

    obj = {'entry_arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}


    if flowTyep == "flow":
        cmd = 'cdbctl show/cdb/l2/show_acl/%s/type/flow' % (table)
    elif flowTyep == "flow_match":
        cmd = 'cdbctl show/cdb/l2/show_acl/%s/type/flow_match' % (table)
    else:
        cmd = 'cdbctl show/cdb/l2/show_acl/%s/type//' % (table)

    arr = vcmd.get_showarr(cmd)
    #print arr

    if len(arr) >= 2:
        arr.pop(0)
        for line in arr:
            #print line
            ## filter "remark xxx" entry
            if (str(line).split()[0]  == 'remark'):
                continue

            obj['entry_arr'].append(str(line))
    return obj
