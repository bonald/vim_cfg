;Public.app.controller('ctlHomeTapFLowMgr', ['$scope', 'SecretApi', 'Alert', '$interval', '$location', '$document', '$rootScope', function($scope, SecretApi, Alert, $interval, $location, $document, $rootScope) {
    $scope.newFlowTableName = '';
    $scope.dispFlowName = '';
    $scope.newRemarkNameEnable = true;
    $scope.newRemarkName = '';
    $scope.ipProtocolNumber = 'NUM';
    $scope.flowTypeDecap = false;
    $scope.direction = [ 'permit', 'deny'];
    $scope.Ipv4v6Type = [ 'ipv4', 'ipv6'];
    $scope.addl2grekeyType = [ '16', '20', '24', '32'];


    if ($rootScope.System_product == 'Tap-350') {
        $scope.editdstIpType = [ 'ipv4'];
        $scope.editsrcIpType = [ 'ipv4'];
    } else {
        $scope.editdstIpType = [ 'ipv4', 'ipv6'];
        $scope.editsrcIpType = [ 'ipv4', 'ipv6'];
    }
    $scope.flow_arr = [ ];
    $scope.flow_inner_arr = [ ];      
    $scope.flow_entry_arr = [ ];
    $scope.select_flow = {};   
    $scope.inner_flow_list = [ ];

    $scope.untag_mode = [ 'Disable', 'double-vlan','outer-vlan','inner-vlan'];

    if ($rootScope.System_product == 'Tap-350') {
        $scope.ipProtocol = [ 'num', 'any', 'tcp', 'udp', 'icmp', 'igmp'];
    } else {
        $scope.ipProtocol = [ 'num', 'any', 'tcp', 'udp', 'gre', 'nvgre', 'icmp', 'igmp'];
    }

    $scope.cfgEntry = {
        flowName : '',
        dir : 'permit',
        ipProtocolNumber : 'any',
        protocolNum: '',
        Ipv4v6Type : 'ipv4',
        editdstIpType : 'ipv4',
        editsrcIpType : 'ipv4',

        srcipEn: false,
        srcIp : '',
        srcIpMask: '0.0.0.0',
        
        dstipEn: false,      
        dstIp : '',
        dstIpMask : '0.0.0.0',
		
        srcipv6En: false,
        srcIpv6 : '',
        srcIpv6Mask: '0::0',

        dstipv6En: false,      
        dstIpv6 : '',
        dstIpv6Mask : '0::0',
		
        flowlabelEn      : false,
        flowlabelValue   : '',
        flowlabelMask    : '0x0',   
		
        icmpcodeEn       : false,
        icmpcodeValue    : '',
        icmptypeEn       : false,
        icmptypeValue    : '',
		
        etherkeyType: 'any',
        ethertypeEn : false,
        etherValue  : '',
        etherMask   : '0x0',

        srcPortEn : false,
        srcPortType : "any", 
        srcPortValue : '',

        destPortEn : false,
        destPortType : "any", 
        destPortValue : '',     

        stripHeaderEn      : false,
        stripPositionEn    : false,
        stripOffsetEn      : false,
        stripOffsetValue   : '',
        stripPositionValue : '',
        stripInnerVxlanEn  : false,
        
        editPacketEn    : false,
        editMacDaEn     : false,
        editMacDaValue  : '',
        editMacSaEn     : false,
        editMacSaValue  : '',
        editIPDaEn      : false,
        editIPDaType    : '',
        editIPDaValue   : '',
        editIPv6DaValue : '',
        editIPSaEn      : false,
        editIPSaType    : '',
        editIPSaValue   : '',
        editIPv6SaValue : '',
        editVlanEn      : false,
        editVlanType    : 'add',
        editVlanValue   : '',

        addl2greEn      : false,
        addl2grekeyType : '24',
        l2grenum : '',
        entryL2greMacDa : '',
        entryL2greIPDa : '',
        entryL2greIPSa : '',

        addl3greEn : false,
        entryL3greMacDa : '',
        entryL3greIPDa :  '',
        entryL3greIPSa :  '',

        srcMacEn : false,
        SrcMacType : "any", 
        srcMacValue : '',
        srcMacMask : '',

        destMacEn : false,
        destMacType : "any", 
        destMacValue : '',
        destMacMask : '',

        truncationEn: false,
        removeVlanEn: "Disable",

        vniEn   :false,
        VniID   : '',
        VniMask : '0x0',

        VlanEn  : false,
        vlanID  : '',
        vlanMask: '0x0',
		
        InnerVlanEn  : false,
        innervlanID  : '',
        innervlanMask: '0x0',
		
        cosEn     : false,
        cosValue  : '',

        innercosEn     : false,
        innercosValue  : '',

        MarkSource: false,
        MarkSourceID : '',

        dscpEn     : false,
        dscp       : '',
        tosEn      : false,
        tos        : '',  
		
        optionsEn     : false,
        fragEn        : false,

        innerMatchEn : false,
        innerFlow    : '',

        udfEn      : false,
        udfType    : 'Layer3',
        udfValue   : '',
        udfMask    : '',   
        udfOffset  : '',   

        greEn      : false,
        greValue   : '',    
        greMask    : '0x0',  

        erspanEn      : false,
        erspanValue   : '',    
        erspanMask    : '0x0',  	

        nvgreEn    : false,
        nvgreVNI   : '',    
        nvgreMask  : '0x0',
		

    };

    /**************************************************************************************************/    
    /******************* Flow Entry *******************************************************************/   
    /**************************************************************************************************/        
    $scope.addNewFlowEntry = function() {

        SecretApi({
            url: '/api/flow/flow_entry/',
            method: 'PUT',
            data: {
                flowName: $scope.cfgEntry.flowName ,
                decap :$scope.cfgEntry.decap,
                type: 'flow',
                dir : $scope.cfgEntry.dir ,
                ipProtocolNumber: $scope.cfgEntry.ipProtocolNumber,
                protocolNum: $scope.cfgEntry.protocolNum.toString(),

                Ipv4v6Type: $scope.cfgEntry.Ipv4v6Type,
                srcipEn : $scope.cfgEntry.srcipEn,
                srcIp: $scope.cfgEntry.srcIp,
                srcIpMask : $scope.cfgEntry.srcIpMask,

                dstipEn : $scope.cfgEntry.dstipEn,
                dstIp: $scope.cfgEntry.dstIp,
                dstIpMask : $scope.cfgEntry.dstIpMask,
				
                srcipv6En : $scope.cfgEntry.srcipv6En,
                srcIpv6: $scope.cfgEntry.srcIpv6,
                srcIpv6Mask : $scope.cfgEntry.srcIpv6Mask,

                dstipv6En : $scope.cfgEntry.dstipv6En,
                dstIpv6: $scope.cfgEntry.dstIpv6,
                dstIpv6Mask : $scope.cfgEntry.dstIpv6Mask,
				
    	        etherkeyType: $scope.cfgEntry.etherkeyType,
                ethertypeEn : $scope.cfgEntry.ethertypeEn,
                etherValue  : $scope.cfgEntry.etherValue,
                etherMask   : $scope.cfgEntry.etherMask,
				
                icmpcodeEn    : $scope.cfgEntry.icmpcodeEn,
                icmpcodeValue : $scope.cfgEntry.icmpcodeValue,
                icmptypeEn    : $scope.cfgEntry.icmptypeEn,
                icmptypeValue : $scope.cfgEntry.icmptypeValue,

                srcPortEn: $scope.cfgEntry.srcPortEn,
                srcPortType: $scope.cfgEntry.srcPortType,
                srcPortValue: $scope.cfgEntry.srcPortValue,

                destPortEn: $scope.cfgEntry.destPortEn,
                destPortType: $scope.cfgEntry.destPortType,
                destPortValue: $scope.cfgEntry.destPortValue,
 
                stripHeaderEn     : $scope.cfgEntry.stripHeaderEn,
                stripPositionEn   : $scope.cfgEntry.stripPositionEn,
                stripOffsetEn     : $scope.cfgEntry.stripOffsetEn,
                stripOffsetValue  : $scope.cfgEntry.stripOffsetValue,
                stripPositionValue: $scope.cfgEntry.stripPositionValue,
				
                stripInnerVxlanEn : $scope.cfgEntry.stripInnerVxlanEn,

                editMacDaEn: $scope.cfgEntry.editMacDaEn,
                editMacDaValue: $scope.cfgEntry.editMacDaValue,
                editMacSaEn: $scope.cfgEntry.editMacSaEn,
                editMacSaValue: $scope.cfgEntry.editMacSaValue,
                editIPDaEn: $scope.cfgEntry.editIPDaEn,
                editdstIpType: $scope.cfgEntry.editdstIpType,
                editIPDaValue: $scope.cfgEntry.editIPDaValue,
                editIPv6DaValue: $scope.cfgEntry.editIPv6DaValue,
                editIPSaEn: $scope.cfgEntry.editIPSaEn,
                editsrcIpType: $scope.cfgEntry.editsrcIpType,			
                editIPSaValue: $scope.cfgEntry.editIPSaValue,
                editIPv6SaValue: $scope.cfgEntry.editIPv6SaValue,
				
                editVlanEn: $scope.cfgEntry.editVlanEn,
                editVlanType: $scope.cfgEntry.editVlanType,
                editVlanValue: $scope.cfgEntry.editVlanValue,

                addl2greEn      : $scope.cfgEntry.addl2greEn,
                addl2grekeyType : $scope.cfgEntry.addl2grekeyType,
                l2grenum : $scope.cfgEntry.l2grenum,
                entryL2greMacDa : $scope.cfgEntry.entryL2greMacDa,
                entryL2greIPDa : $scope.cfgEntry.entryL2greIPDa,
                entryL2greIPSa : $scope.cfgEntry.entryL2greIPSa,

                addl3greEn : $scope.cfgEntry.addl3greEn,
                entryL3greMacDa : $scope.cfgEntry.entryL3greMacDa,
                entryL3greIPDa :  $scope.cfgEntry.entryL3greIPDa,
                entryL3greIPSa :  $scope.cfgEntry.entryL3greIPSa,
                
                srcMacEn: $scope.cfgEntry.srcMacEn,
                srcMacType: $scope.cfgEntry.srcMacType,
                srcMacValue: $scope.cfgEntry.srcMacValue,
                srcMacMask: $scope.cfgEntry.srcMacMask,

                destMacEn: $scope.cfgEntry.destMacEn,
                destMacType: $scope.cfgEntry.destMacType,
                destMacValue: $scope.cfgEntry.destMacValue,
                destMacMask: $scope.cfgEntry.destMacMask,

                VlanEn: $scope.cfgEntry.VlanEn,
                vlanID: $scope.cfgEntry.VlanID,
                vlanMask : $scope.cfgEntry.VlanMask,
				
                InnerVlanEn: $scope.cfgEntry.InnerVlanEn,
                innervlanID: $scope.cfgEntry.InnerVlanID,
                innervlanMask : $scope.cfgEntry.InnerVlanMask,

                trunEn: $scope.cfgEntry.truncationEn,   
                removeVlanEn: $scope.cfgEntry.removeVlanEn,
				
                vniEn: $scope.cfgEntry.vniEn,
                VniID: $scope.cfgEntry.VniID, 
                VniMask: $scope.cfgEntry.VniMask, 

                MarkSource: $scope.cfgEntry.MarkSource, 
                MarkSourceID: $scope.cfgEntry.MarkSourceID, 

                dscpEn   : $scope.cfgEntry.dscpEn,
                dscp     : $scope.cfgEntry.dscp,
                tosEn    : $scope.cfgEntry.tosEn,
                tos      : $scope.cfgEntry.tos,
				
                optionsEn :$scope.cfgEntry.optionsEn,
                fragEn    :$scope.cfgEntry.fragEn,
				
                cosEn    : $scope.cfgEntry.cosEn,
                cosValue : $scope.cfgEntry.cosValue,
                innercosEn    : $scope.cfgEntry.innercosEn,
                innercosValue : $scope.cfgEntry.innercosValue,

                innerMatchEn  : $scope.cfgEntry.innerMatchEn,
                innerFlow     : $scope.cfgEntry.innerFlow.key,

                udfEn    : $scope.cfgEntry.udfEn,
                udfType  : $scope.cfgEntry.udfType,
                udfValue : $scope.cfgEntry.udfValue,
                udfMask  : $scope.cfgEntry.udfMask,
                udfOffset: $scope.cfgEntry.udfOffset,
	
                flowlabelEn    : $scope.cfgEntry.flowlabelEn,
                flowlabelValue : $scope.cfgEntry.flowlabelValue,
                flowlabelMask  : $scope.cfgEntry.flowlabelMask,

                greEn    : $scope.cfgEntry.greEn,
                greValue : $scope.cfgEntry.greValue,
                greMask  : $scope.cfgEntry.greMask,
				
                erspanEn    : $scope.cfgEntry.erspanEn,
                erspanValue : $scope.cfgEntry.erspanValue,
                erspanMask  : $scope.cfgEntry.erspanMask,
				
				

                nvgreEn  : $scope.cfgEntry.nvgreEn,
                nvgreVNI : $scope.cfgEntry.nvgreVNI,
                nvgreMask: $scope.cfgEntry.nvgreMask,

            },            
        }, function(data) {
            $scope.getFlow();
            $scope.getFlowEntryFromTable($scope.dispFlowName);
        }, function() {

        });

        $scope.loading = true;
    } 

    $scope.addNewFlowEntryCheck = function(addEntry) {

        if ($scope.cfgEntry.Ipv4v6Type.toString() =='ipv6' && $scope.cfgEntry.ipProtocolNumber.toString()=='igmp')  {
            return false;
        }
		
        if ($scope.cfgEntry.Ipv4v6Type.toString() =='ipv6' && $scope.cfgEntry.optionsEn == true && $scope.cfgEntry.fragEn == true)  {
            return false;
        }
		
        if ($scope.cfgEntry.ethertypeEn == true && $scope.cfgEntry.ipProtocolNumber.toString()!='any')  {
            return false;
        }
		
        if ($scope.cfgEntry.ipProtocolNumber.toString()=='num' && addEntry.protocol.$invalid) {
            return false;
        }
		
        if ($scope.cfgEntry.dstipEn == true) {
            if (addEntry.entryDstIp.$invalid || addEntry.entryDstwcard.$invalid) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.dstipv6En == true) {
            if (addEntry.entryDstIpv6.$invalid || addEntry.entryDstv6wcard.$invalid) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.srcipEn == true) {
            if (addEntry.entrySrcIp.$invalid || addEntry.entrySrcwcard.$invalid) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.srcipv6En == true) {
            if (addEntry.entrySrcIpv6.$invalid || addEntry.entrySrcv6wcard.$invalid) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.ethertypeEn == true) {
            if (addEntry.etherValue.$invalid || addEntry.etherMask.$invalid) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.icmptypeEn == true) {
            if (addEntry.icmptypeValue.$invalid) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.icmpcodeEn == true) {
            if (addEntry.icmpcodeValue.$invalid) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.srcPortEn == true) {
            if ($scope.cfgEntry.srcPortType!='any' && addEntry.entrySrcPor.$invalid) {
                return false;
            }
        }

        if ($scope.cfgEntry.destPortEn == true) {
            if ($scope.cfgEntry.destPortType!='any' && addEntry.entryDestPort.$invalid) {
                return false;
            }
        }	

        if ($scope.cfgEntry.srcMacEn == true) {
            if ($scope.cfgEntry.srcMacType =='host' && addEntry.entrySrcMac.$invalid) {
                return false;
            }
            if ($scope.cfgEntry.srcMacType =='mac' && 
			   (addEntry.entrySrcMac.$invalid || addEntry.entrySrcMacMask.$invalid)) {
                return false;
            }
        }	

        if ($scope.cfgEntry.destMacEn == true) {
            if ($scope.cfgEntry.destMacType =='host' && addEntry.entryDestMac.$invalid) {
                return false;
            }
            if ($scope.cfgEntry.destMacType =='mac' && 
        	   (addEntry.entryDestMac.$invalid || addEntry.entryDestMacMask.$invalid)) {
                return false;
            }
        }
			
        if ($scope.cfgEntry.nvgreEn == true) {
            if (addEntry.nvgreVNI.$invalid || addEntry.nvgreMask.$invalid ) {
                return false;
            }
        }

        if ($scope.cfgEntry.dscpEn == true) {
            if (addEntry.dscp.$invalid) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.cosEn == true) {
            if (addEntry.cosValue.$invalid) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.innercosEn == true) {
            if (addEntry.innercosValue.$invalid) {
                return false;
            }
        }

        /* TOS invalid check */
        if ($scope.cfgEntry.tosEn == true) {
            if (addEntry.tos.$invalid) {
                return false;
            }
        }

        /* VLAN invalid check */
        if ($scope.cfgEntry.VlanEn == true) {
            if (addEntry.vlanID.$invalid || addEntry.vlanMask.$invalid) {
                return false;
            }
        }
		
        /* Inner VLAN invalid check */
        if ($scope.cfgEntry.InnerVlanEn == true) {
            if (addEntry.innervlanID.$invalid || addEntry.innervlanMask.$invalid) {
                return false;
            }
        }

        /* UDF invalid check */
        if ($scope.cfgEntry.udfEn == true) {
            if (addEntry.udfOffset.$invalid || addEntry.udfValue.$invalid || addEntry.udfMask.$invalid) {
                return false;
            }
        }
		
        /* Flow label invalid check */
        if ($scope.cfgEntry.flowlabelEn == true) {
            if (addEntry.flowlabelValue.$invalid || addEntry.flowlabelMask.$invalid) {
                return false;
            }
        }

        /* Vxlan-VNI valid check */
        if ($scope.cfgEntry.vniEn == true) {
            if (addEntry.VniID.$invalid || addEntry.VniMask.$invalid) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.greEn == true) {
            if (addEntry.greValue.$invalid || addEntry.greMask.$invalid ) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.decap == 1){
            $scope.cfgEntry.erspanEn = false;
            $scope.cfgEntry.stripInnerVxlanEn = false;
		}
		
        if ($scope.cfgEntry.erspanEn == true){
            if (addEntry.erspanValue.$invalid || addEntry.erspanMask.$invalid ) {
                return false;
            }
        }

        /* Edit ipda invalid check */
        if ($scope.cfgEntry.editIPDaEn == true) {
            if ($scope.cfgEntry.editdstIpType.toString() == 'ipv4' && addEntry.entryIPDa.$invalid) {
                return false;
            }
            if ($scope.cfgEntry.editdstIpType.toString() == 'ipv6' && addEntry.entryIPv6Da.$invalid) {
                return false;
            }
        }
		
        /* Edit ipsa invalid check */
        if ($scope.cfgEntry.editIPSaEn == true) {
            if ($scope.cfgEntry.editsrcIpType.toString() == 'ipv4' && addEntry.entryIPSa.$invalid) {
                return false;
            }
            if ($scope.cfgEntry.editsrcIpType.toString() == 'ipv6' && addEntry.entryIPv6Sa.$invalid) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.editIPSaEn == true && $scope.cfgEntry.editIPDaEn == true) {
            if ($scope.cfgEntry.editsrcIpType.toString() != $scope.cfgEntry.editdstIpType.toString()) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.stripOffsetEn == true) {
            if (addEntry.stripOffset.$invalid) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.MarkSource == true) {
            if (addEntry.MarkSourceID.$invalid) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.editMacDaEn == true) {
            if (addEntry.entryEditMacDa.$invalid) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.editMacSaEn == true) {
            if (addEntry.entryEditMacSa.$invalid) {
                return false;
            }
        }
		
        if ($scope.cfgEntry.editVlanEn == true) {
            if (addEntry.editVlan.$invalid) {
                return false;
            }
        }

        if ($scope.cfgEntry.erspanEn==true && $scope.cfgEntry.ipProtocolNumber.toString()=='gre' 
            && ($scope.cfgEntry.stripHeaderEn == false)) {
            return false;
        }
		
        if ($scope.cfgEntry.erspanEn==true && $scope.cfgEntry.udfEn== true) {
            return false;
        }

		if ($scope.cfgEntry.innerMatchEn == true && $scope.cfgEntry.decap != 1) {
            $scope.cfgEntry.innerMatchEn = false;
			$scope.cfgEntry.innerFlow = 'none';
        }
		
		if ($scope.cfgEntry.innerMatchEn == true && $scope.cfgEntry.innerFlow == '') {
            return false;
        }
		
		if ($scope.cfgEntry.innerMatchEn == true) {
            if ($scope.cfgEntry.ipProtocolNumber.toString()!='gre' &&
                $scope.cfgEntry.ipProtocolNumber.toString()!='nvgre' &&
                ($scope.cfgEntry.ipProtocolNumber.toString()!='udp' &&
                $scope.cfgEntry.destPortType!='eq' && $scope.cfgEntry.destPortEn != true )) {
                return false;
            }
        }
		
		if ($scope.cfgEntry.ipProtocolNumber.toString()!='udp' ||
            $scope.cfgEntry.destPortType != 'eq' ) {
            $scope.cfgEntry.vniEn = false;
        }

        if ($scope.cfgEntry.stripHeaderEn ==true && $scope.cfgEntry.erspanEn == false
            && $scope.cfgEntry.ipProtocolNumber.toString()=='gre' ) {
		    if ($scope.cfgEntry.editMacDaEn == false || $scope.cfgEntry.editMacSaEn == false) {
                return false;
            }
        }
        if ($scope.cfgEntry.removeVlanEn!='Disable' && $scope.cfgEntry.truncationEn == true) {
            return false;
        }
		
        return true;      
    };

    $scope.getFlowEntryFromTable = function(tblName) {

        flow_type = 'flow'
        SecretApi({
            url: '/api/flow/flow_get_entry_from_tbl/'+tblName+'/'+flow_type+'/',
            method: 'PUT',
             
        }, function(data) {
            $scope.flow_entry_arr = data.entry_arr;
            $scope.flow_match_arr = data.flow_match;
        }, function() {

        });

        $scope.loading = true;
    } 

    $scope.delFlowEntryFromTable = function(entry) {

        SecretApi({
            url: '/api/flow/flow_entry/',
            method: 'DELETE',
            data: {
                entry: entry ,
                table: $scope.dispFlowName,
                type: 'flow',
            },            
        }, function(data) {
            $scope.getFlowEntryFromTable($scope.dispFlowName);
        }, function() {

        });

        $scope.loading = true;
    } 

    /**************************************************************************************************/    
    /******************* Flow Table *******************************************************************/   
    /**************************************************************************************************/      
    $scope.addNewFlowTable = function() {

        SecretApi({
            url: '/api/flow/flow_table/',
            method: 'PUT',
            data: {
                name: $scope.newFlowTableName ,
                type: 'flow',
                decap : $scope.flowTypeDecap ,
            },            
        }, function(data) {
            $scope.getFlow();
            $scope.newFlowTableName = '';
            $scope.flowTypeDecap = false;
        }, function() {
        });

        $scope.loading = true;
    }

    $scope.getFlow = function() {

        SecretApi({
            url: '/api/flow/flow_table/',
            method: 'GET',

        }, function(data) {
            $scope.flow_arr = data.flow;
            $scope.flow_inner_arr = data.flow_match;      
        }, function() {
        });

        $scope.loading = true;
    }

    $scope.delFlow = function(entry) {

        SecretApi({
            url: '/api/flow/flow_table/',
            method: 'DELETE',
            data: {
                name: entry.key ,
                type: 'flow',
            },            
        }, function(data) {
            $scope.getFlow();
        }, function() {

        });

        $scope.loading = true;
    }

    $scope.flowTypeDecapEnableFun = function(port) {
        $scope.flowTypeDecap = !($scope.flowTypeDecap); 
    };
    /**************************************************************************************************/    
    /******************* Amazing code, not ask me, I dont know WHY ************************************/   
    /**************************************************************************************************/   
    $scope.$watch('dispFlowName', function(value) {
        $scope._refreshDetail();
    });

    $scope._refreshDetail = function() {
        var value = $scope.dispFlowName;
        for(var i = 0; i < $scope.flow_arr.length; i++) {
            var tmp = $scope.flow_arr[i];
            if(tmp.key == value) {
                $scope.dispDetail = tmp;
                return;
            }
        }
    };  

    $scope.viewDetail = function(flowName) {

        $scope.dispFlowName = flowName;
        $scope.getFlowEntryFromTable(flowName);
        $('#flowEntryDetailModal').modal(); 
    };

    $scope.viewFlowRemarkDetail = function(entry) {
        $scope.dispFlowName = entry.key;
        $scope.newRemarkName = entry.remark;
        if (entry.remark != ''){
            $scope.newRemarkNameEnable = true;
            $scope.newRemarkName = entry.remark;
        } else {
            $scope.newRemarkNameEnable = false;
            $scope.newRemarkName = '';
        }          
        $('#flowRemarkModal').modal(); 
    };

    /**************************************************************************************************/    
    /******************* Flow Remark ******************************************************************/   
    /**************************************************************************************************/  
    $scope.addNewRemark = function() {

        SecretApi({
            url: '/api/flow/acl_config/remark',
            method: 'PUT',
            data: {
                name: $scope.dispFlowName ,
                remark: $scope.newRemarkName ,
                type: 'flow',
            },            
        }, function(data) {
            $scope.load();
        });

        $scope.loading = true;  
    };

    $scope.delNewRemark = function() {
        SecretApi({
            url: '/api/flow/acl_config/remark',
            method: 'DELETE',
            data: {
                name: $scope.dispFlowName,
                type: 'flow',
            },            
        }, function(data) {
            $scope.load();
        });

        $scope.loading = true;  
    };

    $scope.newRemarkNameEnableFun = function(port) {
        $scope.newRemarkNameEnable = !($scope.newRemarkNameEnable); 
        $scope.newRemarkName = '';
    };

    $scope.updateRemark = function() {
        if ($scope.newRemarkNameEnable) {
            $scope.addNewRemark();
        }
        else {
            $scope.delNewRemark();
        }
    };    

    /**************************************************************************************************/    
    /******************* Flow CFG ******************************************************************/   
    /**************************************************************************************************/  
    $scope.cfgEntryIpProtoNum = function() {

        $scope.cfgEntry.protocolNum = '';
        if ($scope.cfgEntry.decap != 1) {
            $scope.cfgEntry.innerMatchEn = false;
        }

        if ($scope.cfgEntry.ipProtocolNumber.toString()!='tcp'
            && $scope.cfgEntry.ipProtocolNumber.toString()!='udp') {
            $scope.cfgEntry.srcPortEn = false;
            $scope.cfgEntry.destPortEn = false;
        }
        if ($scope.cfgEntry.ipProtocolNumber.toString()!='udp' ||
            $scope.cfgEntry.destPortType != 'eq' ) {
            $scope.cfgEntry.nvgreEn = false;
        }
        if ($scope.cfgEntry.ipProtocolNumber.toString()!='nvgre') {
            $scope.cfgEntry.nvgreEn = false;
        }
        if ($scope.cfgEntry.ipProtocolNumber.toString()!='gre') {
            $scope.cfgEntry.greEn = false;
            $scope.cfgEntry.erspanEn = false;
            $scope.cfgEntry.stripInnerVxlanEn  = false;
        }
        if ($scope.cfgEntry.ipProtocolNumber.toString()!='icmp') {
            $scope.cfgEntry.icmptypeEn = false;
            $scope.cfgEntry.icmpcodeEn = false;
        }
        if ($scope.cfgEntry.ipProtocolNumber.toString()!='any') {
            $scope.cfgEntry.ethertypeEn = false;
        }
        if ($scope.cfgEntry.decap == 1){
            $scope.cfgEntry.erspanEn = false;
            $scope.cfgEntry.stripInnerVxlanEn  = false;
		}
        if ($scope.cfgEntry.ipProtocolNumber.toString()!='gre'
            && $scope.cfgEntry.ipProtocolNumber.toString()!='nvgre') {
            $scope.cfgEntry.stripHeaderEn = false;
            $scope.cfgEntry.stripOffsetEn = false;
            $scope.cfgEntry.stripPositionEn = false;
        }
        if (!($scope.cfgEntry.ipProtocolNumber.toString()=='udp'
            && $scope.cfgEntry.destPortEn == true
            && $scope.cfgEntry.destPortType == 'eq')) {
            $scope.cfgEntry.stripHeaderEn = false;
            $scope.cfgEntry.stripOffsetEn = false;
            $scope.cfgEntry.stripPositionEn = false;
        }
    }; 

    $scope.cfgEntryIpFilterType = function() {
        if ($scope.cfgEntry.Ipv4v6Type.toString() =='ipv4') {
            $scope.cfgEntry.flowlabelEn = false;
            $scope.cfgEntry.flowlabelValue  = '';
            $scope.cfgEntry.flowlabelMask   = '0x0';
            $scope.cfgEntry.srcipEn = false;
            $scope.cfgEntry.dstipEn = false;
        }

        if ($scope.cfgEntry.Ipv4v6Type.toString() =='ipv6') {
            $scope.cfgEntry.erspanEn = false;
            $scope.cfgEntry.stripInnerVxlanEn  = false;
            $scope.cfgEntry.erspanMask = '0x0';
            $scope.cfgEntry.ethertypeEn = false;
            $scope.cfgEntry.etherkeyType = 'any'
            $scope.cfgEntry.etherValue   = '';
            $scope.cfgEntry.etherMask   = '0x0';
            $scope.cfgEntry.udfEn = false;
            $scope.cfgEntry.udfType   = 'Layer3';
            $scope.cfgEntry.udfMask = ''
            $scope.cfgEntry.udfOffset   = '';
            $scope.cfgEntry.udfValue   = '';
            $scope.cfgEntry.srcipv6En = false;
            $scope.cfgEntry.dstipv6En = false;
        }
    }; 	

    $scope.viewAddFlowEntry = function(flow) {
        $scope.cfgEntry.flowName = flow.key;
        $scope.dispFlowName = flow.key;
        $scope.select_flow = flow;
        $scope.cfgEntry.decap = flow.decap_en;
    };  

    /* Source Port */
    $scope.cfgEntrySrcPortEnFun = function() {
        $scope.cfgEntry.srcPortEn = !($scope.cfgEntry.srcPortEn);
        $scope.cfgEntry.srcPortType = 'any';
        $scope.cfgEntry.srcPortValue = '';
    };  

    $scope.cfgEntrySrcPortTypeFun = function(key) {
        $scope.cfgEntry.srcPortType = key;
        $scope.cfgEntry.srcPortValue = '';
    };     

    /* Destination Port */
    $scope.cfgEntryDestPortEnFun = function() {
        $scope.cfgEntry.destPortEn = !($scope.cfgEntry.destPortEn);
        $scope.cfgEntry.destPortType = 'any';
        $scope.cfgEntry.destPortValue = '';
        if ($scope.cfgEntry.ipProtocolNumber.toString()!='udp' ||
            $scope.cfgEntry.destPortType != 'eq' ) {
            $scope.cfgEntry.vniEn = false;
        }
    };  

    $scope.cfgEntryDestPortTypeFun = function(key) {
        $scope.cfgEntry.destPortType = key;
        $scope.cfgEntry.destPortValue = '';
        if ($scope.cfgEntry.ipProtocolNumber.toString()!='udp' ||
            $scope.cfgEntry.destPortType != 'eq' ) {
            $scope.cfgEntry.vniEn = false;
        }
    };  

    /* EditPacket */
    $scope.cfgEntryEditPacketEnFun = function() {
        $scope.cfgEntry.editPacketEn = !($scope.cfgEntry.editPacketEn);
        $scope.cfgEntry.editMacDaEn = false;
        $scope.cfgEntry.editMacSaEn = false;
        $scope.cfgEntry.editIPDaEn = false;
        $scope.cfgEntry.editIPSaEn = false;
        $scope.cfgEntry.editVlanEn = false;

        $scope.cfgEntry.editMacDaValue = '';
        $scope.cfgEntry.editMacSaValue = '';
        $scope.cfgEntry.editIPDaType = '';
        $scope.cfgEntry.editIPSaType = '';
        $scope.cfgEntry.editIPDaValue = '';
        $scope.cfgEntry.editIPSaValue = '';
        $scope.cfgEntry.editIPv6DaValue = '';
        $scope.cfgEntry.editIPv6SaValue = '';
        $scope.cfgEntry.editVlanValue = '';

    };  
 
    $scope.cfgEntryEditMacDaEnFun = function() {
        $scope.cfgEntry.editMacDaEn = !($scope.cfgEntry.editMacDaEn);
        $scope.cfgEntry.editMacDaValue = '';

    };  

    $scope.cfgEntryEditMacSaEnFun = function() {
        $scope.cfgEntry.editMacSaEn = !($scope.cfgEntry.editMacSaEn);
        $scope.cfgEntry.editMacSaValue = '';

    };  
 
    $scope.cfgEntryEditIPDaEnFun = function() {
        $scope.cfgEntry.editIPDaEn = !($scope.cfgEntry.editIPDaEn);
        $scope.cfgEntry.editIPDaValue = '';
        $scope.cfgEntry.editIPv6DaValue = '';
        $scope.cfgEntry.editIPDaType = '';

    };  

    $scope.cfgEntryEditIPSaEnFun = function() {
        $scope.cfgEntry.editIPSaEn = !($scope.cfgEntry.editIPSaEn);
        $scope.cfgEntry.editIPSaValue= '';
        $scope.cfgEntry.editIPv6SaValue= '';
        $scope.cfgEntry.editIPSaType= '';
    };  
	
    $scope.cfgEntryEditDipTypeFun = function(key) {
        $scope.cfgEntry.editIPDaType = key;
        $scope.cfgEntry.editIPDaValue = '';
        $scope.cfgEntry.editIPv6DaValue = '';
    }; 
	
    $scope.cfgEntryEditSipTypeFun = function(key) {
        $scope.cfgEntry.editIPSaType = key;
        $scope.cfgEntry.editIPSaValue = '';
        $scope.cfgEntry.editIPv6SaValue = '';
    }; 

    $scope.cfgEntryEditVlanEnFun = function() {
        $scope.cfgEntry.editVlanEn = !($scope.cfgEntry.editVlanEn);
        $scope.cfgEntry.editVlanID = '';

    };

    $scope.cfgEntryaddl2greEnFun = function() {

        $scope.cfgEntry.addl2greEn = !($scope.cfgEntry.addl2greEn);
        $scope.cfgEntry.addl2grekeyType = '24';
        $scope.cfgEntry.l2grenum = '';
        $scope.cfgEntry.entryL2greMacDa = '';
        $scope.cfgEntry.entryL2greIPDa = '';
        $scope.cfgEntry.entryL2greIPSa = '';

    };

        $scope.cfgEntryaddl3greEnFun = function() {

        $scope.cfgEntry.addl3greEn = !($scope.cfgEntry.addl3greEn);
        $scope.cfgEntry.entryL3greMacDa = '';
        $scope.cfgEntry.entryL3greIPDa = '';
        $scope.cfgEntry.entryL3greIPSa = '';

    };

    /* Source Mac */
    $scope.cfgEntrySrcMacEnFun = function() {
        $scope.cfgEntry.srcMacEn = !($scope.cfgEntry.srcMacEn);
        $scope.cfgEntry.srcMacType = 'any';
        $scope.cfgEntry.srcMacValue = '';
    };  

    $scope.cfgEntrySrcMacTypeFun = function(key) {
        $scope.cfgEntry.srcMacType = key;

        $scope.cfgEntry.srcMacValue = '';
        $scope.cfgEntry.srcMacMask = '';
    };    

    $scope.cfgEntryEditVlanTypeFun = function(key) {
        $scope.cfgEntry.editVlanType = key;
    }; 	

    $scope.cfgEntryDestMacEnFun = function() {
        $scope.cfgEntry.destMacEn = !($scope.cfgEntry.destMacEn);
        $scope.cfgEntry.destMacType = 'any';
        $scope.cfgEntry.destMacValue = '';
    };  

    $scope.cfgEntryDestMacTypeFun = function(key) {
        $scope.cfgEntry.destMacType = key;
        $scope.cfgEntry.destMacValue = '';
        $scope.cfgEntry.destMacMask = '';

    };     

    $scope.cfgEntryVlanEnFun = function() {

        $scope.cfgEntry.VlanEn = !($scope.cfgEntry.VlanEn);
        $scope.cfgEntry.VlanID   = '';
        $scope.cfgEntry.VlanMask = '0x0';
    };  

    $scope.cfgEntryInnerVlanEnFun = function() {
        $scope.cfgEntry.InnerVlanEn = !($scope.cfgEntry.InnerVlanEn);
        $scope.cfgEntry.InnerVlanID   = '';
        $scope.cfgEntry.InnerVlanMask = '0x0';
    };
	
    $scope.cfgEntryVlanIdAvail = function() {

        if ($scope.cfgEntry.VlanID >= 2 && $scope.cfgEntry.VlanID <= 4094) {
            return true;
        }
        return false;
    }; 

    $scope.cfgEntryTruncationShowFun = function() {
        return $scope.cfgEntry.removeVlanEn=='Disable' && $scope.cfgEntry.stripHeaderEn!=true && $scope.cfgEntry.MarkSource!=true && $scope.cfgEntry.editPacketEn!=true;
    };  
	
	$scope.cfgEntryStripHeaderShowFun = function() {
        return $scope.cfgEntry.ipProtocolNumber.toString()=='gre' || $scope.cfgEntry.ipProtocolNumber.toString()=='nvgre' || ($scope.cfgEntry.ipProtocolNumber.toString()=='udp' &&
            $scope.cfgEntry.destPortType == 'eq');
    };
	

    $scope.cfgEntryTruncationEnFun = function() {
        $scope.cfgEntry.truncationEn = !($scope.cfgEntry.truncationEn);
        $scope.cfgEntry.removeVlanEn = "Disable";
        $scope.cfgEntry.MarkSource = false;
        $scope.cfgEntry.editPacketEn = false;
        $scope.cfgEntry.editMacDaEn = false;
        $scope.cfgEntry.editMacSaEn = false;
        $scope.cfgEntry.editIPDaEn = false;
        $scope.cfgEntry.editIPSaEn = false;
    };

    $scope.cfgEntryEthertypeEnFun = function() {
        $scope.cfgEntry.ethertypeEn = !($scope.cfgEntry.ethertypeEn);
        if ($scope.cfgEntry.ethertypeEn == true) {
            $scope.cfgEntry.srcipEn = false;
            $scope.cfgEntry.srcipv6En = false;
            $scope.cfgEntry.dstipEn = false;
            $scope.cfgEntry.dstipv6En = false;
            $scope.cfgEntry.dscpEn = false;
            $scope.cfgEntry.optionsEn = false;
            $scope.cfgEntry.tosEn = false;
            $scope.cfgEntry.fragEn = false;
            $scope.cfgEntry.destPortEn = false;
            $scope.cfgEntry.srcPortEn = false;
            $scope.cfgEntry.ipProtocolNumber = 'any';
            $scope.cfgEntry.nvgreEn = false;
            $scope.cfgEntry.greEn = false;
            $scope.cfgEntry.erspanEn = false;
            $scope.cfgEntry.stripInnerVxlanEn  = false;
            $scope.cfgEntry.icmptypeEn = false;
            $scope.cfgEntry.icmpcodeEn = false;
            $scope.cfgEntry.innerMatchEn = false;
            $scope.cfgEntry.vniEn = false;
        }
		
        if ($scope.cfgEntry.ethertypeEn == false) {
            $scope.cfgEntry.etherValue = '';
        }
    };


    $scope.cfgEntrySrcIpEnFun = function() {

        $scope.cfgEntry.srcipEn = !($scope.cfgEntry.srcipEn);
        if ($scope.cfgEntry.srcipEn == false) {
            $scope.cfgEntry.srcIp = ''; 
        }
        else {
            $scope.cfgEntry.srcIpMask = '0.0.0.0'; 
	}
    };
		
    $scope.cfgEntrySrcIpv6EnFun = function() {

        $scope.cfgEntry.srcipv6En = !($scope.cfgEntry.srcipv6En);
        if ($scope.cfgEntry.srcipv6En == false) {
            $scope.cfgEntry.srcIpv6 = ''; 
        }
        else {
            $scope.cfgEntry.srcIpv6Mask = '0::0'; 
        }
    };
	
    $scope.cfgEntryDstIpEnFun = function() {

        $scope.cfgEntry.dstipEn = !($scope.cfgEntry.dstipEn);
        if ($scope.cfgEntry.dstipEn == false) {
            $scope.cfgEntry.dstIp = ''; 
        }
        else{
            $scope.cfgEntry.dstIpMask = '0.0.0.0'; 
        }
    };
	
    $scope.cfgEntryDstIpv6EnFun = function() {

        $scope.cfgEntry.dstipv6En = !($scope.cfgEntry.dstipv6En);
        if ($scope.cfgEntry.dstipv6En == false) {
            $scope.cfgEntry.dstIpv6 = ''; 
        }
        else{
            $scope.cfgEntry.dstIpv6Mask = '0::0'; 
        }
    };


    $scope.cfgEntryVniEnFun = function() {
        $scope.cfgEntry.vniEn = !($scope.cfgEntry.vniEn);
        $scope.cfgEntry.VniID = '';
        $scope.cfgEntry.VniMask = '0x0';
    }; 

    $scope.cfgEntryCosEnFun = function() {
        $scope.cfgEntry.cosEn = !($scope.cfgEntry.cosEn);
        $scope.cfgEntry.cosValue   = '';
    }; 
	
    $scope.cfgEntryInnerCosEnFun = function() {
        $scope.cfgEntry.innercosEn = !($scope.cfgEntry.innercosEn);
        $scope.cfgEntry.innercosValue   = '';
    }; 

	
    $scope.cfgEntryIcmpTypeEnFun = function() {
        $scope.cfgEntry.icmptypeEn = !($scope.cfgEntry.icmptypeEn);
        $scope.cfgEntry.icmptypeValue   = '';
        if (false == $scope.cfgEntry.icmptypeEn) {
            $scope.cfgEntry.icmpcodeEn = false;
        }
    }; 
	
    $scope.cfgEntryIcmpCodeEnFun = function() {
        $scope.cfgEntry.icmpcodeEn = !($scope.cfgEntry.icmpcodeEn);
        $scope.cfgEntry.icmpcodeValue   = '';
    }; 
	
    $scope.cfgEntryDscpEnFun = function() {
        $scope.cfgEntry.dscpEn = !($scope.cfgEntry.dscpEn);
        $scope.cfgEntry.dscp   = '';
    }; 

    $scope.cfgEntryTosEnFun = function() {
        $scope.cfgEntry.tosEn  = !($scope.cfgEntry.tosEn);
        $scope.cfgEntry.tos    = '';
    }; 
	
    $scope.cfgEntryOptionsEnFun = function() {
        $scope.cfgEntry.optionsEn  = !($scope.cfgEntry.optionsEn);
    };
	
    $scope.cfgEntryFragEnFun = function() {
        $scope.cfgEntry.fragEn  = !($scope.cfgEntry.fragEn);
    };

    $scope.cfgEntryInnerMatchEnFun = function() {
        $scope.cfgEntry.innerMatchEn = !($scope.cfgEntry.innerMatchEn);
        $scope.cfgEntry.innerFlow    = '';
    }; 

    $scope.cfgEntryUdfEnFun = function() {
        $scope.cfgEntry.udfEn     = !($scope.cfgEntry.udfEn);
        $scope.cfgEntry.udfType   = 'Layer3';
        $scope.cfgEntry.udfValue  = '';
        $scope.cfgEntry.udfMask   = '';
        $scope.cfgEntry.udfOffset = '';
        if ($scope.cfgEntry.udfEn == true) {
            $scope.cfgEntry.erspanEn = false;
            $scope.cfgEntry.stripInnerVxlanEn = false;
        }
    }; 

    $scope.cfgEntryUdfTypeFun = function(key) {
        $scope.cfgEntry.udfType = key;
    }; 
	
    $scope.cfgEntryFlowLabelEnFun = function() {
        $scope.cfgEntry.flowlabelEn     = !($scope.cfgEntry.flowlabelEn);
        $scope.cfgEntry.flowlabelValue  = '';
        $scope.cfgEntry.flowlabelMask   = '0x0';
    }; 
	

    $scope.cfgEntryGreEnFun = function() {
        $scope.cfgEntry.greEn    = !($scope.cfgEntry.greEn);
        $scope.cfgEntry.greValue = '';
        $scope.cfgEntry.greMask  = '0x0';
    }; 

    $scope.cfgEntryErspanEnFun = function() {
        $scope.cfgEntry.erspanEn    = !($scope.cfgEntry.erspanEn);
        $scope.cfgEntry.erspanValue = '';
        $scope.cfgEntry.stripInnerVxlanEn  = false;
        if ($scope.cfgEntry.erspanEn == true) {
            $scope.cfgEntry.udfEn = false;
        }
    }; 	

    $scope.cfgEntryNvGreEnFun = function() {
        $scope.cfgEntry.nvgreEn   = !($scope.cfgEntry.nvgreEn);
        $scope.cfgEntry.nvgreVNI  = '';
        $scope.cfgEntry.nvgreMask = '0x0';
    }; 

    $scope.cfgEntryMarkSourceEnFun = function() {
        $scope.cfgEntry.MarkSource = !($scope.cfgEntry.MarkSource);
        $scope.cfgEntry.MarkSourceID = '';
        if ($scope.cfgEntry.MarkSource == true) {
            $scope.cfgEntry.removeVlanEn = "Disable";
        }
    }; 

    $scope.cfgEntryStripHeaderEnFun = function() {

        $scope.cfgEntry.stripHeaderEn = !($scope.cfgEntry.stripHeaderEn);
        $scope.cfgEntry.stripPositionEn   = false;
        $scope.cfgEntry.stripOffsetEn     = false;
        $scope.cfgEntry.stripOffsetValue = '';
        $scope.cfgEntry.stripPositionValue = '';
        $scope.cfgEntry.stripInnerVxlanEn = false;
        if ($scope.cfgEntry.stripHeaderEn == true) {
            $scope.cfgEntry.removeVlanEn = "Disable";
        }
    }; 

    $scope.cfgEntrystripPositionEnFun = function() {
        $scope.cfgEntry.stripPositionEn = !($scope.cfgEntry.stripPositionEn);
        $scope.cfgEntry.stripPositionValue = '';
    }; 

    $scope.cfgEntrystripOffsetEnFun = function() {
        $scope.cfgEntry.stripOffsetEn = !($scope.cfgEntry.stripOffsetEn);
        $scope.cfgEntry.stripOffsetValue = '';
    }; 

    $scope.cfgEntryStripPositionFun = function(key) {
        $scope.cfgEntry.stripPositionValue = key;
    }; 

    $scope.cfgEntrystripInnerVxlanEnFun = function() {
        $scope.cfgEntry.stripInnerVxlanEn = !($scope.cfgEntry.stripInnerVxlanEn);
    }    

    /**************************************************************************************************/        
    $scope.load = function() {

        $scope.getFlow();
        $scope.loading = false;

    };

    $scope.load();

}]);
