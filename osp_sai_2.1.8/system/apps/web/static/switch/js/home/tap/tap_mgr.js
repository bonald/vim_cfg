;Public.app.controller('ctlHomeTapMgr', ['$scope', 'SecretApi', 'Alert', '$interval', '$location', '$document','$rootScope', function($scope, SecretApi, Alert, $interval, $location, $document, $rootScope) {

    $scope.tapGroupName = '';
    $scope.tapGroupID = 0;
    $scope.tapGroupDescriptionEnable = true;
    $scope.tapGroupDescription = '';
    $scope.truncationLength = '';
    $scope.group_arr = [];
    $scope.groupSel = {};
    $scope.direction = [ 'ingress', 'egress'];
    $scope.untag_mode = [ 'Disable', 'double-vlan','outer-vlan','inner-vlan'];
    if ($rootScope.System_product == 'Tap-580'
        || $rootScope.System_product == 'Tap-550' || $rootScope.System_product == 'Tap-530') {
        $scope.editdstIpType = [ 'ipv4', 'ipv6'];
        $scope.editsrcIpType = [ 'ipv4', 'ipv6'];
    } else {
        $scope.editdstIpType = [ 'ipv4'];
        $scope.editsrcIpType = [ 'ipv4'];
    }
    $scope.show_group_direction = 'ingress';
    $scope.allIf = [];
    $scope.dispGroupName = '';
    $scope.dispGroup = '';

    $scope.allEgressEntry = [];
    $scope.ifp = [];

    $scope.flow_arr = [ ]; 
    $scope.timestamp = {
        timeStampEn: false,
        macDa: '',
        macSa: '',
        etherType: '',
    }    

    $scope.tapTruncation = {
        truncationEn: false,
        length: '',
    }   
    
    $scope.addEntry =
        {
            port : "",
            group: "",
            entryDirection: 'ingress',
            removeVlanEn: 'Disable',
            trunEn : false,
            tstamp : false,

            markEn: false,
            vlanId: '',
            flowEn: false,
            flow: '',

            stripHeaderEn   : false,
            editPacketEn    : false,
            editMacDaEn     : false,
            editMacDaValue  : '',
            editMacSaEn     : false,
            editMacSaValue  : '',
            editIPDaEn      : false,
            editIPDaValue   : '',
			editIPv6DaValue : '',
            editIPSaEn      : false,
            editIPSaValue   : '',   
			editIPv6SaValue : '',
            editdstIpType   : 'ipv4',
            editsrcIpType   : 'ipv4',			
            editVlanEn      : false,
            editVlanValue   : '', 
        }

    $scope.addTapGroup = function() {

        SecretApi({
            url: '/api/tap/group_table/',
            method: 'PUT',
            data: {
                group_name: $scope.tapGroupName ,
                group_id: $scope.tapGroupID ,
            },
        }, function(data) {
            $scope.load();
        }, function() {
            $scope.loading = false; 
        });

        $scope.loading = true;

    }

    $scope.delTapGroup = function(group) {

        SecretApi({
            url: '/api/tap/group_table/',
            method: 'DELETE',
            data: {
                group_name: group ,
            },
        }, function(data) {
            $scope.load();
        }, function() {

        });

    }

    $scope.getTapGroup = function() {
        SecretApi({
            url: '/api/tap/group_table/',
            method: 'GET',
        }, function(data) {
            $scope.group_arr = data.arr;

        }, function() {

        });
    }

    $scope.delTruncationLength = function() {

        SecretApi({
            url: '/api/tap/truncation/',
            method: 'DELETE',           
        }, function(data) {

        }, function() {

        });
        
        $scope.truncationLength = '';
    }

    $scope.tapGroupDescriptionEnableFun = function(port) {
        $scope.tapGroupDescriptionEnable = !($scope.tapGroupDescriptionEnable); 
        $scope.tapGroupDescription = '';
    };

    $scope.addTapGroupDescription = function() {
        SecretApi({
            url: '/api/tap/description/',
            method: 'PUT',
            data: {
                description: $scope.tapGroupDescription ,
                group_name: $scope.dispGroupName ,
            },            
        }, function(data) {
            $scope.load();
            $scope.tapGroupDescription = '';
        }, function() {

        });
    }

    /**************************************************************************************************/    
    /******************* modal Tap add entry **********************************************************/   
    /**************************************************************************************************/  

    $scope.$watch('dispGroupName', function(value) {
        $scope._refreshDetail();
    });

    $scope._refreshDetail = function() {
        var value = $scope.dispGroupName;
        for(var i = 0; i < $scope.group_arr.length; i++) {
            var tmp = $scope.group_arr[i];
            if(tmp.key == value) {
                $scope.dispDetail = tmp;
                return;
            }
        }
    };    

    $scope.viewDetail = function(groupName) {
        $scope.dispGroupName = groupName;
        $('#tapGroupDetailModal').modal(); 
    };
    /**************************************************************************************************/    
    /******************* modal Tap Group Description **********************************************************/   
    /**************************************************************************************************/  
    $scope.$watch('dispGroupName', function(value) {
        $scope._refreshDetail();
    });

    $scope._refreshDetail = function() {
        var value = $scope.dispGroupName;
        for(var i = 0; i < $scope.group_arr.length; i++) {
            var tmp = $scope.group_arr[i];
            if(tmp.key == value) {
                $scope.dispDetail = tmp;
                return;
            }
        }
    };    

    $scope.viewDetailDescription = function(group) {
        $scope.dispGroup = group;
        $scope.dispGroupName = group.key;
        if (group.desc != ''){
            $scope.tapGroupDescriptionEnable = true;
            $scope.tapGroupDescription = group.desc;
        } else {
            $scope.tapGroupDescriptionEnable = false;
            $scope.tapGroupDescription = '';
        }
        $('#tapGroupDescriptionModal').modal(); 
    };

    $scope.getAllIf = function() {
        $scope.allIf = [];

        SecretApi({
            url: '/api/interface/all/',
            method: 'GET',
        }, function(data) {

            $scope.ifp = data.arr;
            var len = $scope.ifp.length;
            for (i=0; i<len; i++){
                if($scope.ifp[i].hw_type.search('Port')>=0 || $scope.ifp[i].hw_type.search('LAG')>=0){
                    $scope.allIf.push($scope.ifp[i]);
                    $scope.allIf[i].box = false;
                } 
            }

            if ($scope.ifp.length) {

                $scope.addEntry.port = $scope.ifp[0];

            }
        }, function() {

        });
    }

    $scope.addTapGroupEntry = function() {
        SecretApi({
            url: '/api/tap/tap_group_egress/',
            method: 'PUT',
            data: {
                group: $scope.dispGroupName ,
                direction: $scope.addEntry.entryDirection ,
                port: $scope.addEntry.port.key ,
                removeVlanEn: $scope.addEntry.removeVlanEn ,
                trunEn: $scope.addEntry.trunEn ,  
                tstamp: $scope.addEntry.tstamp ,                 
                markEn: $scope.addEntry.markEn,
                vlanId: $scope.addEntry.vlanId,
                flow: ($scope.addEntry.flow==null)?'None':$scope.addEntry.flow.key,

                stripHeaderEn   : $scope.addEntry.stripHeaderEn,
                editMacDaEn     : $scope.addEntry.editMacDaEn,
                editMacDaValue  : $scope.addEntry.editMacDaValue,
                editMacSaEn     : $scope.addEntry.editMacSaEn,
                editMacSaValue  : $scope.addEntry.editMacSaValue,
                editIPDaEn      : $scope.addEntry.editIPDaEn,
                editdstIpType   : $scope.addEntry.editdstIpType,
                editIPDaValue   : $scope.addEntry.editIPDaValue,
                editIPv6DaValue : $scope.addEntry.editIPv6DaValue,
                editIPSaEn      : $scope.addEntry.editIPSaEn,
                editsrcIpType   : $scope.addEntry.editsrcIpType,			
                editIPSaValue   : $scope.addEntry.editIPSaValue,
                editIPv6SaValue : $scope.addEntry.editIPv6SaValue,
				
				
                editVlanEn      : $scope.addEntry.editVlanEn,
                editVlanValue   : $scope.addEntry.editVlanValue,
            },            
        }, function(data) {
            $scope.getTapGroupEntry();
            $scope.addEntry.removeVlanEn = 'Disable';
        }, function() {
            $scope.addEntry.editPacketEn = false;
            $scope.addEntry.stripHeaderEn = false;
            $scope.addEntry.editMacDaEn = false;
            $scope.addEntry.editMacDaValue = '';
            $scope.addEntry.editMacSaEn = false;
            $scope.addEntry.editMacSaValue = '';
            $scope.addEntry.editIPDaEn = false;
            $scope.addEntry.editdstIpType = 'ipv4';
            $scope.addEntry.editIPDaValue = '';
            $scope.addEntry.editIPv6DaValue = '';
            $scope.addEntry.editIPSaEn = false;
            $scope.addEntry.editsrcIpType = 'ipv4';			
            $scope.addEntry.editIPSaValue = '';
            $scope.addEntry.editIPv6SaValue = '';
            $scope.addEntry.editVlanEn = false;
            $scope.addEntry.editVlanValue = '';
        });
    }

   $scope.addNewTapEntryCheck = function() {

        if ($scope.addEntry.port == null || $scope.addEntry.port.key == ''
            || typeof($scope.addEntry.port.key) ==  'undefined' ) {

              return false;

        }
        if ($scope.addEntry.editIPDaEn == true && $scope.addEntry.editIPSaEn == true && $scope.addEntry.editsrcIpType != $scope.addEntry.editdstIpType) {
            return false;
        }
       
        return true;      
    };

    $scope.getTapGroupEntry = function() {

        SecretApi({
            url: '/api/tap/tap_group_egress/',
            method: 'GET',          
        }, function(data) {
            $scope.allEgressEntry = data.egress;
            $scope.allIngressEntry = data.ingress;

        }, function() {

        });
    }

    $scope.delTapEgressEntry = function(entry) {

        SecretApi({
            url: '/api/tap/tap_group_egress/',
            method: 'DELETE',          
            data: {
                direction:'egress',
                key: entry.key ,
            }, 
        }, function(data) {
            $scope.getTapGroupEntry();
        }, function() {

        });
    }

    $scope.TapIngressEntryFilter = function(entry) {

        return entry.name === $scope.dispGroupName;
    }

    $scope.delTapIngressEntry = function(entry) {

        SecretApi({
            url: '/api/tap/tap_group_egress/',
            method: 'DELETE',          
            data: {
                direction:'ingress',
                key: entry.key ,
                flow: entry.flow,
            }, 
        }, function(data) {
            $scope.getTapGroupEntry();
        }, function() {
        });
    }

    $scope.tapGroupShowDirection = function(direction) {
        $scope.show_group_direction = direction;
    }   

    $scope.switchTstampEnFun = function() {
        $scope.addEntry.tstamp = !($scope.addEntry.tstamp);
    }  
    
    $scope.switchTruncationEnFun = function() {
        $scope.addEntry.trunEn = !($scope.addEntry.trunEn);
        $scope.addEntry.removeVlanEn = 'Disable';
        $scope.addEntry.markEn = false;
        $scope.addEntry.vlanId = '';
        $scope.addEntry.flow = '';

        $scope.addEntry.editPacketEn = false;

        $scope.addEntry.editMacDaEn = false;
        $scope.addEntry.editMacSaEn = false;
        $scope.addEntry.editIPDaEn = false;
        $scope.addEntry.editIPSaEn = false;
        $scope.addEntry.editVlanEn = false;
		$scope.addEntry.editsrcIpType = 'ipv4';
		$scope.addEntry.editdesIpType = 'ipv4';

        $scope.addEntry.editMacDaValue = '';
        $scope.addEntry.editMacSaValue = '';
        $scope.addEntry.editIPDaValue = '';
        $scope.addEntry.editIPSaValue = '';
        $scope.addEntry.editIPv6DaValue = '';
        $scope.addEntry.editIPv6SaValue = '';
        $scope.addEntry.editVlanValue = '';
    }  
 

    $scope.switchMarkEnFun = function() {

        $scope.addEntry.markEn = !($scope.addEntry.markEn);
        $scope.addEntry.vlanId = '';
    } 

    $scope.switchFlowEnFun = function() {

        $scope.addEntry.flowEn = !($scope.addEntry.flowEn);
        $scope.addEntry.flow = '';
    } 



    /* EditPacket */
    $scope.cfgEntryEditPacketEnFun = function() {
        $scope.addEntry.editPacketEn = !($scope.addEntry.editPacketEn);
        $scope.addEntry.editMacDaEn = false;
        $scope.addEntry.editMacSaEn = false;
        $scope.addEntry.editIPDaEn = false;
        $scope.addEntry.editIPSaEn = false;
        $scope.addEntry.editVlanEn = false;
		$scope.addEntry.editsrcIpType = 'ipv4';
		$scope.addEntry.editdesIpType = 'ipv4';

        $scope.addEntry.editMacDaValue = '';
        $scope.addEntry.editMacSaValue = '';
        $scope.addEntry.editIPDaValue = '';
        $scope.addEntry.editIPSaValue = '';
        $scope.addEntry.editIPv6DaValue = '';
        $scope.addEntry.editIPv6SaValue = '';
        $scope.addEntry.editVlanValue = '';
    };  
 
     $scope.cfgEntryEditMacDaEnFun = function() {
        $scope.addEntry.editMacDaEn = !($scope.addEntry.editMacDaEn);
        $scope.addEntry.editMacDaValue = '';

    };  

     $scope.cfgEntryEditMacSaEnFun = function() {
        $scope.addEntry.editMacSaEn = !($scope.addEntry.editMacSaEn);
        $scope.addEntry.editMacSaValue = '';

    };  
 
     $scope.cfgEntryEditIPDaEnFun = function() {
        $scope.addEntry.editIPDaEn = !($scope.addEntry.editIPDaEn);
        $scope.addEntry.editIPDaValue = '';
        $scope.addEntry.editIPv6DaValue = '';
		$scope.addEntry.editdesIpType = 'ipv4';
    };  

     $scope.cfgEntryEditIPSaEnFun = function() {
        $scope.addEntry.editIPSaEn = !($scope.addEntry.editIPSaEn);
        $scope.addEntry.editIPSaValue = '';
        $scope.addEntry.editIPv6SaValue = '';
		$scope.addEntry.editsrcIpType = 'ipv4';
    };  
 
     $scope.cfgEntryEditVlanEnFun = function() {
        $scope.addEntry.editVlanEn = !($scope.addEntry.editVlanEn);
        $scope.addEntry.editVlanValue = '';

    };  

    $scope.getFlow = function() {

        SecretApi({
            url: '/api/flow/flow_table/',
            method: 'GET',

        }, function(data) {
            $scope.flow_arr = data.flow;
        }, function() {

        });

        $scope.loading = true;
    }

    /**************************************************************************************************/    
    /******************* modal Tap TimeStamp **********************************************************/   
    /**************************************************************************************************/  
    $scope.timeStampEn = function() {

        $scope.timestamp.timeStampEn = !($scope.timestamp.timeStampEn);
    }
    
    $scope.setTapTimeStamp = function() {

        SecretApi({
            url: '/api/tap/tap_timestamp/',
            method: 'PUT',
            data: {
                timeStampEn: $scope.timestamp.timeStampEn ,
                macDa: $scope.timestamp.macDa ,
                macSa: $scope.timestamp.macSa ,
                etherType: $scope.timestamp.etherType ,
 
            },            
        }, function(data) {
            $scope.getTapTimeStamp();
        }, function() {

        });
      
        $scope.load();
    }

    $scope.getTapTimeStamp = function() {
        SecretApi({
            url: '/api/tap/tap_timestamp/',
            method: 'GET',          
        }, function(data) {
            $scope.timestamp.macDa = data.cfg[0].tap_ts_macda;
            $scope.timestamp.macSa = data.cfg[0].tap_ts_macsa;
            $scope.timestamp.etherType = data.cfg[0].ether_type;
            
            if ($scope.timestamp.macDa == "0000.0000.0000" 
                && $scope.timestamp.macSa == "0000.0000.0000"
                && $scope.timestamp.etherType == "0x0") {
                $scope.timestamp.timeStampEn = false;
            } else {
                $scope.timestamp.timeStampEn = true;
            }
        }, function() {

        });
    }
  
    /**************************************************************************************************/    
    /******************* modal Tap Truncation **********************************************************/   
    /**************************************************************************************************/  
    $scope.tapTruncationEn = function() {

        $scope.tapTruncation.truncationEn = !($scope.tapTruncation.truncationEn);
        
        if ($scope.tapTruncation.truncationEn == false) {
            $scope.tapTruncation.length = '';
        }
    }
    
    $scope.setTruncationLength = function() {

        SecretApi({
            url: '/api/tap/truncation/',
            method: 'PUT',
            data: {
                truncationEn: $scope.tapTruncation.truncationEn ,
                length: $scope.tapTruncation.length ,
            },            
        }, function(data) {

        }, function() {

        });
        $scope.getTruncationLength();
    }

    $scope.getTruncationLength = function() {

        SecretApi({
            url: '/api/tap/truncation/',
            method: 'GET',          
        }, function(data) {
            if (data.trunction == 0){
                $scope.tapTruncation.length = '';
                $scope.tapTruncation.truncationEn = false;
            }else {
                $scope.tapTruncation.length = parseInt(data.trunction);
                $scope.tapTruncation.truncationEn = true;
            }

        }, function() {

        });

    }  
    
    /**************************************************************************************************/    
    /******************* Init **********************************************************/   
    /**************************************************************************************************/      
    $scope.load = function() {
        $scope.getTapGroup();
        $scope.getAllIf();
        $scope.getTapGroupEntry();
        $scope.loading = false;
        $scope.getFlow();
        $scope.getTruncationLength();
        $scope.getTapTimeStamp();
    };

    $scope.load(); 

}]);
