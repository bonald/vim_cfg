
;Public.app.controller('ctlHomeSecurityDhcpSnoop', ['$scope', 'SecretApi', 'Alert', function($scope, SecretApi, Alert) {

    /* AJAX */
    $scope.enable_dhcp_snooping = false;
    $scope.enable_dhcp_service = false;
    $scope.enable_verify_mac = false;
    $scope.SaveState = false;
    $scope.allow_untrusted = false;
    $scope.information_option = false;
    $scope.display_detail = false;
    $scope.interface = [];
    $scope.bind_entry = [];
    $scope.statistics = {};
    $scope.trust_port = {};
    $scope.remote_id = {};
    $scope.snoop_vlan = {};
    $scope.snoop_db = {};
    $scope.multiSelIf = []; /* for add snooping trust port */
    $scope.trust_port = []; /* for add snooping trust port */  
    $scope.SELCTLALL = false;   
    $scope.interval = {value:0};    

    /* pagination */
    $scope.pgCurrent = 1;
    $scope.pgItemCnt = 0;
    $scope.pgItemNumEachPage = 10;
    
    $scope.newFdb = {
        macAddr: '',
        vlan: '',
        ip: '',
        port: '',
        expiry: '',        
        };
    
    /* user defines */
    $scope.switchState = false;

    /* functions */
    $scope.onPageChange = function() {
    	
    };
    $scope.enableDhcpSnooping = function() {
        SecretApi({
            url: '/api/dhcp_snooping/load_dhcp_enabl/', 
        }, function(data) {
            $scope.enable_dhcp_snooping = data.dhcp_snooping_enable == 0 ? false : true;
        });
    };

    $scope.enableDhcpService = function() {
        SecretApi({
            url: '/api/dhcp_snooping/load_dhcp_service_enabl/', 
        }, function(data) {
            $scope.enable_dhcp_service = data.dhcp_service_enable == 0 ? false : true;
        });
    };
    
    $scope.verifyMac = function() {
        SecretApi({
            url: '/api/dhcp_snooping/load_dhcp_verify_mac/', 
        }, function(data) {
            $scope.enable_verify_mac = data.verify_mac_address == 0 ? false : true;
        });
    };    

    $scope.allowUntrusted = function() {
        SecretApi({
            url: '/api/dhcp_snooping/dhcp_allow_untrusted/', 
        }, function(data) {
            $scope.allow_untrusted = data.allow_untrusted == 0 ? false : true;     
        });
    };   

    $scope.informationOption = function() {

        SecretApi({
            url: '/api/dhcp_snooping/dhcp_snooping_information_option/', 
        }, function(data) {
            $scope.information_option = data.add_agent_options == 0 ? false : true;
          
        });
    };

    $scope.loadIf = function() {
        $scope.multiSelIf = [];
        $scope.portIndexObj = {};        
        SecretApi({
            url: '/api/interface/all/',
        }, function(data) {
            $scope.interface = _.filter(data.arr, function(item) {return item.hw_type == 'Port';});

            /* set selected */
            for(var i = 0; i < $scope.interface.length; i ++) {
                if($scope.interface[i].dhcp_snooping_trust == '1') {
                    $scope.multiSelIf[i] = true;
                }
            }    

            _.each(data.arr, function(item) {
                
                if (item.key.indexOf("eth") == '0' || item.key.indexOf("agg") == '0') {
                   $scope.portIndexObj[item.ifindex] = item.key;
                }
                
            });
            $scope.newFdb.port = $scope.portIndexObj[data.arr[0].ifindex];
            
        });
    };

    $scope.clear = function(){
        SecretApi({
            url: '/api/dhcp_snooping/dhcp_packet_statistics/',
            method: 'PUT',
        }, function(data) {
            $scope.load();
        });
    };

    $scope.load_bind_entry = function() {
        $scope.bind_entry = [];
        SecretApi({
            url: '/api/dhcp_snooping/dhcp_load_bind_entry/',
        }, function(data) {
            var arr = data.arr;
            var len = arr.length;
            for (i=0; i<len; i++){
                $scope.bind_entry.push(arr[i]);
                $scope.bind_entry[i].box = false;
            }
            $scope.pgItemCnt = $scope.bind_entry.length;
            /*$scope.bind_entry = data.arr;*/
        });
    };

    $scope.get_statistics = function() {
        SecretApi({
            url: '/api/dhcp_snooping/dhcp_packet_statistics/',
        }, function(data) {
            $scope.statistics = data.statistics;
        });
    };

    $scope.loadPortTrust = function() {
        SecretApi({
            url: '/api/dhcp_snooping/dhcp_port_trust/',
        }, function(data) {
            $scope.trust_port = data.trust_port;
        });
    };

    $scope.addTrustPort = function() {
        var ifs = [];
        
        for(var i = 0; i < $scope.multiSelIf.length; i++) {
            if($scope.multiSelIf[i]) ifs.push($scope.interface[i].key);
        }        
        
        SecretApi({
            url: '/api/dhcp_snooping/dhcp_port_trust/',
            method: 'PUT',
            data: {
                if_arr: ifs,   
            },
        }, function(data) {

        });
        $scope.DisplaySnoopTrust = false;
        $scope.load();        
    };

    $scope.delTrustPort = function() {
        var ifs = [];
        
        for(var i = 0; i < $scope.multiSelIf.length; i++) {
            if($scope.multiSelIf[i]) ifs.push($scope.interface[i].key);
        }        
        
        SecretApi({
            url: '/api/dhcp_snooping/dhcp_del_port_trust/',
            method: 'PUT',
            data: {
                if_arr: ifs,   
            },
        }, function(data) {

        });
        $scope.DisplaySnoopTrust = false;
        $scope.load();
    };
    
    $scope.chooseAll = function() {
        var len = $scope.bind_entry.length;

        chooseOne = false;
        $scope.setif = false;
        if($scope.SELCTLALL == true){
            $scope.setif = true;
            for(i=0; i<len; i++){
                $scope.bind_entry[i].box = true;
            }
        }
        else{
            for(i=0; i<len; i++){
                $scope.bind_entry[i].box = false;
            }
        }}

    $scope.choose = function() {
        var len = $scope.bind_entry.length;

        $scope.setif = false;
        var j = 0;
        for(i=0; i<len; i++){
            if($scope.bind_entry[i].box == true){
                $scope.setif = true;
                $scope.selectedIf = $scope.bind_entry[i];
                j ++;
            }
        }
        if(j == 1){
            $scope.chooseOne = true;
        }else{
            $scope.chooseOne = false;
        }
    };
    
    $scope.changeEnableState = function() {
        $scope.display_detail = false;
        
        var enable = ($scope.enable_dhcp_snooping == true) ? 0 : 1;
        
        if($scope.enable_dhcp_snooping) $scope.display_detail = true;
                
        SecretApi({
            url: '/api/dhcp_snooping/load_dhcp_enabl/'+enable+'/',
            method: 'PUT',
        }, function() {
            $scope.enableDhcpSnooping(); 
        });
    }    

    $scope.changeserviceState = function() {

        var enable = ($scope.enable_dhcp_service == true) ? 0 : 1;      
        
        SecretApi({
            url: '/api/dhcp_snooping/load_dhcp_service_enabl/'+enable+'/',
            method: 'PUT',
        }, function() {
            $scope.enableDhcpService();
        });
    }  
    
    $scope.changeVerifyState = function() {

        var enable = ($scope.enable_verify_mac == true) ? 0 : 1;     
        
        SecretApi({
            url: '/api/dhcp_snooping/load_dhcp_verify_mac/'+enable+'/',
            method: 'PUT',
        }, function() {
            $scope.verifyMac();            
        });
    }    

    $scope.changeSaveState = function() {
        SecretApi({
            url: '/api/dhcp_snooping/dhcp_save_database/',
            method: 'PUT',
        }, function() {
       
        });
    }    

    $scope.changeUntrustedState = function() {

        var enable = ($scope.allow_untrusted == true) ? 0 : 1;      
        
        SecretApi({
            url: '/api/dhcp_snooping/dhcp_allow_untrusted/'+enable+'/',
            method: 'PUT',
        }, function() {
            $scope.allowUntrusted();            
        });
    }    

    $scope.changeInformationOptionState = function() {

        var enable = ($scope.information_option == true) ? 0 : 1;     
        
        SecretApi({
            url: '/api/dhcp_snooping/dhcp_snooping_information_option/'+enable+'/',
            method: 'PUT',
        }, function() {
            $scope.informationOption();            
        });
    }       
      
    $scope.chooseAutoSave = function() {
     /*   $scope.DisplayAutoSave = true; */
    };

    $scope.chooseRemoteId = function() {
     /*   $scope.DisplayRemoteId = true; */
    };

    $scope.chooseSnoopTrust = function() {
     /*   $scope.DisplayRemoteId = true; */
     $scope.multiSelIf = [];
     $scope.loadIf();
    };
    
    $scope.modifySaveInterval = function(){

        SecretApi({
            url: '/api/dhcp_snooping/dhcp_modify_interval/'+$scope.interval.value+'/',
            method: 'PUT',
        }, function() {
        /*    $scope.enableDhcp(); */
        });
        $scope.DisplayAutoSave = false;
        $scope.load();           
    }

    $scope.GetSaveInterval = function(){

        SecretApi({
            url: '/api/dhcp_snooping/dhcp_get_interval/',
            method: 'GET',
        }, function(data) {
            $scope.interval.value = parseInt(data.database_save_interval);
        });
    }
    
    $scope.modifyRemoteId = function(){

        SecretApi({
            url: '/api/dhcp_snooping/dhcp_modify_remote_id/'+$scope.remote_id.value+'/',
            method: 'PUT',
        }, function() {
        /*    $scope.enableDhcp(); */
        });
        $scope.DisplayRemoteId = false;
        $scope.load();
    }     

    $scope.addSnoopVlan = function(){

        SecretApi({
            url: '/api/dhcp_snooping/dhcp_add_snoop_vlan/'+$scope.snoop_vlan.value+'/',
            method: 'PUT',
        }, function() {
        /*    $scope.enableDhcp(); */
        });
        $scope.DisplaySnoopVlan = false;
        $scope.snoop_vlan.value = '';
        $scope.load();            
    }     

    $scope.delSnoopVlan = function(){

        SecretApi({
            url: '/api/dhcp_snooping/dhcp_del_snoop_vlan/'+$scope.snoop_vlan.value+'/',
            method: 'PUT',
        }, function() {
        /*    $scope.enableDhcp(); */
        });
        $scope.DisplaySnoopVlan = false;
        $scope.snoop_vlan.value = '';
        $scope.load();            
    }       

    $scope.loadDB = function(){

        SecretApi({
            url: '/api/dhcp_snooping/dhcp_load_snoop_db/'+$scope.snoop_db.value+'/',
            method: 'PUT',
        }, function() {
        /*    $scope.enableDhcp(); */
        });
        $scope.DisplaySnoopDB = false;
        $scope.snoop_db.value = '';
        $scope.load();
    }   

    /**
     * add new mac address table
     */
    $scope.addNewFdb = function() {

        var mac_address = $scope.newFdb.macAddr;
        var port = $scope.newFdb.port;
        var vlan_id = parseInt($scope.newFdb.vlan);
        var ip = $scope.newFdb.ip;
        var expiry = $scope.newFdb.expiry;
        
        SecretApi({
            url: '/api/dhcp_snooping/dhcp_bind_entry_set/', 
            method: 'POST',
            data: {
                vlan_id: vlan_id,
                mac_address: mac_address,
                port: port,
                ip: ip,
                expiry: expiry,
                },
        }, function(data) {
/*        

            var index = -1;
            for(var i = 0; i < $scope.macAddressTable.length; i++) {
                var obj = $scope.macAddressTable[i]; 
                if(obj.mac_address == mac_address && parseInt(obj.vlan_id) == vlan_id) {
                    index = i;
                    break;
                }
            }

            var fdbTmp = {
                mac_address: mac_address,
                port: port,
                vlan_id: vlan_id,
                type: 'static',
                key: $scope.getKey(mac_address, port, vlan_id),
            };
            if(index == -1) $scope.macAddressTable.push(fdbTmp);
            else            $scope.macAddressTable[index] = fdbTmp;

            $scope._refresh();
            $scope.load();
*/            
        }, function() {
/*        
            $scope.loading = false;  
*/            
        });
        
        addFdbFlag = true;
        $scope.load();
    };

    $scope.isDeleteEnable = function() {
        for(var i = 0; i < $scope.bind_entry.length; i++) {
          var entry = $scope.bind_entry[i];
          if (true == entry.box) {
            return true;
          }
        }
    };

    $scope.deleteBindEntry = function(bind_mac) {

        SecretApi({
            url: '/api/dhcp_snooping/dhcp_del_one_bind_entry/'+bind_mac+'/',
            method: 'PUT',
        }, function() {

        });
    };
    
	$scope.deleteAll = function() {
  
        for (var i = 0; i < $scope.bind_entry.length; i ++) {
          $scope.deleteBindEntry($scope.bind_entry[i].mac);
        }

        $scope.load();
    };    

	$scope.deleteMulti = function() {
  
        for (var i = 0; i < $scope.bind_entry.length; i ++) {
          if (true == $scope.bind_entry[i].box) {
            $scope.deleteBindEntry($scope.bind_entry[i].mac);
          }        
        }

        $scope.load();
    };  
    
    $scope.load = function() {
        $scope.enableDhcpSnooping();
        $scope.enableDhcpService();
        $scope.verifyMac();       
        $scope.allowUntrusted();    
        $scope.informationOption();
        $scope.get_statistics();
        $scope.loadIf();
        $scope.load_bind_entry();
        $scope.loadPortTrust();
        $scope.GetSaveInterval();
    };
    
    $scope.load();
    
}]);


