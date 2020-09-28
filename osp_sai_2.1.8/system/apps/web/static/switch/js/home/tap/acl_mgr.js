;Public.app.controller('ctlHomeTapAclMgr', ['$scope', 'SecretApi', 'Alert', '$interval', '$location', '$document', function($scope, SecretApi, Alert, $interval, $location, $document) {
    $scope.newFlowTableName = '';
    $scope.dispFlowName = '';
    $scope.newRemarkNameEnable = true;
    $scope.newRemarkName = '';
    $scope.ipProtocolNumber = 'NUM';
    $scope.ipProtocol = [ 'num', 'any', 'tcp', 'udp', 'icmp', 'igmp'];
    $scope.direction = [ 'permit', 'deny'];
    $scope.Ipv4v6Type = [ 'ipv4', 'ipv6'];
    $scope.acl_arr = [ ];   
    $scope.flow_entry_arr = [ ];  

    $scope.cfgEntry = {
        flowName : '',
        dir : 'permit',
        ipProtocolNumber : 'any',
        protocolNum: '',
        Ipv4v6Type : 'ipv4',

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

        editPacketEn    : false,
        editMacDaEn     : false,
        editMacDaValue  : '',
        editMacSaEn     : false,
        editMacSaValue  : '',
        editIPDaEn      : false,
        editIPDaValue   : '',
        editIPSaEn      : false,
        editIPSaValue   : '',
        editVlanEn      : false,
        editVlanValue   : '',

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
        
        udfEn      : false,
        udfType    : 'Layer3',
        udfValue   : '',
        udfMask    : '',   
        udfOffset  : '',   

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
                type: 'acl',
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
                editMacDaEn: $scope.cfgEntry.editMacDaEn,
                editMacDaValue: $scope.cfgEntry.editMacDaValue,
                editMacSaEn: $scope.cfgEntry.editMacSaEn,
                editMacSaValue: $scope.cfgEntry.editMacSaValue,
                editIPDaEn: $scope.cfgEntry.editIPDaEn,
                editIPDaValue: $scope.cfgEntry.editIPDaValue,
                editIPSaEn: $scope.cfgEntry.editIPSaEn,
                editIPSaValue: $scope.cfgEntry.editIPSaValue,
                editVlanValue: $scope.cfgEntry.editVlanValue,
                
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

                VniID: $scope.cfgEntry.VniID, 
                VniMask: $scope.cfgEntry.VniMask, 

                // FixedMe. useless, only hold a place
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

                udfEn    : $scope.cfgEntry.udfEn,
                udfType  : $scope.cfgEntry.udfType,
                udfValue : $scope.cfgEntry.udfValue,
                udfMask  : $scope.cfgEntry.udfMask,
                udfOffset: $scope.cfgEntry.udfOffset,
	
                flowlabelEn    : $scope.cfgEntry.flowlabelEn,
                flowlabelValue : $scope.cfgEntry.flowlabelValue,
                flowlabelMask  : $scope.cfgEntry.flowlabelMask,

            },            
        }, function(data) {
            //$scope.getFlow();
            //$scope.getFlowEntryFromTable($scope.dispFlowName);
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
		

        return true;      
    };

   $scope.clearFlowEntry= function() {
        //FixME
    } 

   $scope.addNewFlowEntryClose = function() {
        //FixME
    } 

    $scope.getFlowEntryFromTable = function(tblName) {

        flow_type = 'acl'
        SecretApi({
            url: '/api/flow/flow_get_entry_from_tbl/'+tblName+'/'+flow_type+'/',
            method: 'PUT',
             
        }, function(data) {

            $scope.flow_entry_arr = data.entry_arr;

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
                type: 'acl',
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
                type: 'acl',
            },            
        }, function(data) {
            $scope.getFlow();
            $scope.newFlowTableName = '';
        }, function() {
        });

        $scope.loading = true;
    }

    $scope.getFlow = function() {

        SecretApi({
            url: '/api/flow/flow_table/',
            method: 'GET',

        }, function(data) {
            $scope.acl_arr = data.acl;
            
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
                type: 'acl',
            },            
        }, function(data) {
            $scope.getFlow();
        }, function() {

        });

        $scope.loading = true;
    }

    /**************************************************************************************************/    
    /******************* Amazing code, not ask me, I dont know WHY ************************************/   
    /**************************************************************************************************/   
    $scope.$watch('dispFlowName', function(value) {
        $scope._refreshDetail();
    });

    $scope._refreshDetail = function() {
        var value = $scope.dispFlowName;
        for(var i = 0; i < $scope.acl_arr.length; i++) {
            var tmp = $scope.acl_arr[i];
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
                type: 'acl',
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
                type: 'acl',
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
        if ($scope.cfgEntry.ipProtocolNumber.toString()!='tcp'
            && $scope.cfgEntry.ipProtocolNumber.toString()!='udp') {
            $scope.cfgEntry.srcPortEn = false;
            $scope.cfgEntry.destPortEn = false;
        }
        if ($scope.cfgEntry.ipProtocolNumber.toString()!='icmp') {
            $scope.cfgEntry.icmptypeEn = false;
            $scope.cfgEntry.icmpcodeEn = false;
        }
        if ($scope.cfgEntry.ipProtocolNumber.toString()!='any') {
            $scope.cfgEntry.ethertypeEn = false;
        }
    }; 

    $scope.cfgEntryIpFilterType = function() {
        if ($scope.cfgEntry.Ipv4v6Type.toString() =='ipv4') {
            $scope.cfgEntry.flowlabelEn = false;
            $scope.cfgEntry.flowlabelValue  = '';
            $scope.cfgEntry.flowlabelMask   = '';
        }

        if ($scope.cfgEntry.Ipv4v6Type.toString() =='ipv6') {
            $scope.cfgEntry.ethertypeEn = false;
            $scope.cfgEntry.etherkeyType = 'any'
            $scope.cfgEntry.etherValue   = '';
            $scope.cfgEntry.etherMask   = '0x0';
            $scope.cfgEntry.udfEn = false;
            $scope.cfgEntry.udfType   = 'Layer3';
            $scope.cfgEntry.udfMask = ''
            $scope.cfgEntry.udfOffset   = '';
            $scope.cfgEntry.udfValue   = '';
        }
    }; 	
    $scope.viewAddFlowEntry = function(key) {
        $scope.cfgEntry.flowName = key;
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
    };  

    $scope.cfgEntryDestPortTypeFun = function(key) {

        $scope.cfgEntry.destPortType = key;
        $scope.cfgEntry.destPortValue = '';
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
        $scope.cfgEntry.editIPDaValue = '';
        $scope.cfgEntry.editIPSaValue = '';
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

    };  

     $scope.cfgEntryEditIPSaEnFun = function() {
        $scope.cfgEntry.editIPSaEn = !($scope.cfgEntry.editIPSaEn);
        $scope.cfgEntry.editIPSaValue = '';

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
        return $scope.cfgEntry.removeVlanEn!=true && $scope.cfgEntry.stripHeaderEn!=true && $scope.cfgEntry.MarkSource!=true && $scope.cfgEntry.editPacketEn!=true;
    };  
    $scope.cfgEntryTruncationEnFun = function() {
        $scope.cfgEntry.truncationEn = !($scope.cfgEntry.truncationEn);

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
            $scope.cfgEntry.icmptypeEn = false;
            $scope.cfgEntry.icmpcodeEn = false;
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

    $scope.cfgEntryRemoveVlanEnFun = function() {
        $scope.cfgEntry.removeVlanEn = !($scope.cfgEntry.removeVlanEn);
    };  

    $scope.cfgEntryStripHdrEnFun = function() {
        $scope.cfgEntry.stripHeaderEn = !($scope.cfgEntry.stripHeaderEn);
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

    $scope.cfgEntryUdfEnFun = function() {
        $scope.cfgEntry.udfEn     = !($scope.cfgEntry.udfEn);
        $scope.cfgEntry.udfType   = 'Layer3';
        $scope.cfgEntry.udfValue  = '';
        $scope.cfgEntry.udfMask   = '';
        $scope.cfgEntry.udfOffset = '';
    }; 

    $scope.cfgEntryUdfTypeFun = function(key) {
        $scope.cfgEntry.udfType = key;
    }; 
	
    $scope.cfgEntryFlowLabelEnFun = function() {
        $scope.cfgEntry.flowlabelEn     = !($scope.cfgEntry.flowlabelEn);
        $scope.cfgEntry.flowlabelValue  = '';
        $scope.cfgEntry.flowlabelMask   = '0x0';
    }; 
	

    /**************************************************************************************************/        
    $scope.load = function() {

        $scope.getFlow();
        $scope.clearFlowEntry();
        $scope.loading = false;

    };

    $scope.load();

}]);
