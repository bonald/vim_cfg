
;Public.app.controller('ctlHomePortMgrStatus', ['$scope', 'SecretHttp', 'SecretApi', 'Alert', '$rootScope', function($scope, SecretHttp, SecretApi, Alert, $rootScope) {

    $scope.loading = false;

    $scope.if = [];
    $scope.setif = false;
    $scope.chk = false;
    $scope.chooseOne = false;
    $scope.selectedIf = {};
    $scope.newIf = {};
    $scope.dispPortName = '';
    $scope.dispPort = {};
    $scope.portFlowEn = false;
    

    $scope.flow_arr = [ ]; 
    $scope.portFlowName = ''; 

    $scope.portDescriptionEnable = true;
    $scope.portDescription = '';
    $scope.dispPortName = '';

    $scope.portSpeedMode = ['AUTO', '10', '100', '1000', '2.5G', '5G', '10G', '25G', '40G', '100G'];
    $scope.portSpeed = $scope.portSpeedMode[0];

    $scope.portDuplexMod = ['full', 'half', 'auto'];
    $scope.portDuplex = $scope.portDuplexMod[0];

    $scope.portSvlanTpidMod = ['0x8100', '0x88A8', '0x9100'];
    $scope.portSvlanTpid = $scope.portSvlanTpidMod[0];

    $scope.portSplitType = ['10giga', '40giga'];
    $scope.split_type = '10giga';
    $scope.split_status_arr = [];
    $scope.split_cfg_has_changed = false;

    $scope.portSwitchType = ['none', 'sfp', 'qsfp'];
    $scope.portType = 'sfp';

    $scope.portVlanTypeMode  = ['access', 'trunk'];
    $scope.portVlanType      = $scope.portVlanTypeMode[0];

    $scope.portProtlModeType = ['switch', 'routed'];
    $scope.portProtlMode     = $scope.portProtlModeType[0];

    $scope.portEnableType    = ['Enable', 'Disable'];
    $scope.jumboframe_en     = $scope.portEnableType[0];
    $scope.display_combo     = false;

    $scope.chooseAll = function() {
        var len = $scope.if.length;

        chooseOne = false;
        $scope.setif = false;
        if($scope.chk == true){
            $scope.setif = true;
            for(i=0; i<len; i++){
                $scope.if[i].box = true;
            }
        }
        else{
            for(i=0; i<len; i++){
                $scope.if[i].box = false;
            }
        }}

    $scope.choose = function() {
        var len = $scope.if.length;

        $scope.setif = false;
        var j = 0;
        for(i=0; i<len; i++){
            if($scope.if[i].box == true){
                $scope.setif = true;
                $scope.selectedIf = $scope.if[i];
                j ++;
            }
        }
        if(j == 1){
            $scope.chooseOne = true;
        }else{
            $scope.chooseOne = false;
        }
    };

    $scope.up = function(){
        if($scope.newIf.up == true){
            $scope.newIf.state = '1';
        }
        else{
            $scope.newIf.state = '0';
        }
    }

    $scope.down = function(){
        if($scope.newIf.down == true){
            $scope.newIf.state = '0';
        }
        else{
            $scope.newIf.state = '1';
        }
    }

    $scope.mode2switch = function(){
        if($scope.newIf.modeswitch == true){
            $scope.newIf.mode = 'switch';
        }
        else{
            $scope.newIf.mode = 'routed';
        }
    }

    $scope.mode2routed = function(){
        if($scope.newIf.moderouted == true){
            $scope.newIf.mode = 'routed';
        }
        else{
            $scope.newIf.mode = 'switch';
        }
    }

    $scope.vlan2access = function(){
        if($scope.newIf.vlanaccess == true){
            $scope.newIf.vlan_type = 'access';
        }
        else{
            $scope.newIf.vlan_type = 'trunk';
        }
    }

    $scope.vlan2trunk = function(){
        if($scope.newIf.vlantrunk == true){
            $scope.newIf.vlan_type = 'trunk';
        }
        else{
            $scope.newIf.vlan_type = 'access';
        }
    }

    $scope.jumbo2dis = function(){
        if($scope.newIf.jumbo_dis == true){
            $scope.newIf.jumboframe_en = '0';
        }
        else{
            $scope.newIf.jumboframe_en = '1';
        }
    }
    $scope.jumbo2en = function(){
        if($scope.newIf.jumbo_en == true){
            $scope.newIf.jumboframe_en = '1';
        }
        else{
            $scope.newIf.jumboframe_en = '0';
        }
    }

    $scope.modifyIf = function(key){
        SecretHttp({
            url: '/api/interface/all/',
            method: 'PUT',
            data:{
                key : key,
                desc : $scope.newIf.desc,
                up : $scope.newIf.state,
                speed : $scope.newIf.speed,
                mode : $scope.newIf.mode,
                vlan_type : $scope.newIf.vlan_type,
                jumboframe_en : $scope.newIf.jumboframe_en,
            },
        })

            .then(function(resp) {
                if (resp.status == 200) {
                    if(resp.data['error']) {
                        Alert(resp.data['err_reason']);
                    }
                }
                else {
                    Alert('http server error!');
                }
            });
    }
    $scope.modifyAll = function(){
        var len = $scope.if.length;
        for(i=0; i<len; i++){
            if($scope.if[i].box == true){

                $scope.modifyIf($scope.if[i].key);
            }
        }

        $scope.load();
        $scope.setif = false;
        $scope.chk = false;
        $scope.chooseOne = false;


    };
    $scope.getIf = function() {
        $scope.if = [];

        SecretHttp({
            url: '/api/interface/all/',
            method: 'GET',
        })
            .then(function(resp) {
                if(resp.status == 200) {
                    $scope.ifp = resp.data.arr;

                    var len = $scope.ifp.length;
                    for (i=0; i<len; i++){
                        if($scope.ifp[i].key.search('eth')>=0
                            || $scope.ifp[i].key.search('agg')>=0){
                            $scope.if.push($scope.ifp[i]);
                            $scope.if[i].box = false;
                        }
                        if ($scope.ifp[i].key.search('agg')>=0) {
                            $scope.if[i].phy_type = 'LAG';
                        }
                        if($scope.ifp[i].key == 'eth-0-1'){
                            if ($scope.ifp[i].phyport_combo == '1'){
                                $scope.display_combo  = true;
                            }
                        }
                    }
                }
                else {
                    Alert(resp.data['err_reason']);
                }
            });
    };

    $scope.setPortStatus = function(iface) {
        $scope.loading = true;

        var en_str = (iface.up == 0) ? 'enable' : 'disable';
        var url = '/api/interface/'+ iface.key +'/'+ en_str +'/';

        SecretApi({
            url: url,
            method: 'PUT',
        }, function() {
            $scope.load();
        }, function() {
            $scope.loading = false;
        });

        return false;
    };

    /**************************************************************************************************/    
    /******************* Port Flow Modal **********************************************************/   
    /**************************************************************************************************/  
    $scope.$watch('dispPortName', function(value) {
        //$scope._refreshDetail();
    });
    $scope.viewFlowDetail = function(port) {

        $scope.dispPortName = port.full_name;
        if (port.egs_acl.length) {
            $scope.portFlowEn = true;
            $scope.portFlowName = port.egs_acl;
        }
        $('#portFlowModal').modal(); 
    };


    ///// configure Port Flow //////////
    $scope.setPortFlowEn = function(key) {
        $scope.portFlowEn = ! ($scope.portFlowEn);

        if ($scope.portFlowEn == false){
            $scope.setFlowOne();
        }       
    };

    $scope.setFlowOne = function() {

        SecretHttp({
            url: '/api/interface/flow/',
            method: 'PUT',
            data:{
                key : $scope.dispPortName,
                portFlowEn : $scope.portFlowEn,
                portFlowName : ($scope.portFlowName==null)?'NULL':$scope.portFlowName.key,
            },            
        })
            .then(function(resp) {

            });   

        $scope.load();
    };

    $scope.getPortFlowCfgEn = function() {

        SecretHttp({
            url: '/api/interface/flow/',
            method: 'GET',
        })
            .then(function(resp) {
                if(resp.status == 200) {
                    $scope.ifp = resp.data.arr;
                    var len = $scope.ifp.length;
                    for (i=0; i<len; i++){
                        if($scope.ifp[i].key.search('eth')>=0){
                            $scope.if.push($scope.ifp[i]);
                            $scope.if[i].box = false;
                        }
                    }

                }
                else {
                    Alert(resp.data['err_reason']);
                }
            });
    };

    $scope.getFlow = function() {

        SecretHttp({
            url: '/api/flow/flow_table/',
            method: 'GET',
        })
            .then(function(resp) {
                if(resp.status == 200) {
                    $scope.flow_arr = resp.data.acl;
                    if ($scope.flow_arr.length != 0) {
                        $scope.portFlowName = $scope.flow_arr[0].key;
                    }

                }
                else {
                    Alert(resp.data['err_reason']);
                }                

            });
    };

    $scope.getPortStatus = function() {
        $scope.loading = true;

        var en_str = (iface.up == 0) ? 'enable' : 'disable';
        var url = '/api/interface/'+ iface.key +'/'+ en_str +'/';

        SecretApi({
            url: url,
            method: 'PUT',
        }, function() {
            $scope.load();
        }, function() {
            $scope.loading = false;
        });

        return false;
    };

    /**************************************************************************************************/    
    /******************* port description  ***********************************************************************/   
    /**************************************************************************************************/  
    $scope.viewDetailDescription = function(port) {
        $scope.dispPortName = port.full_name;
        $('#portDescriptionModal').modal(); 
    };

    $scope.portDescriptionEnFun = function(port) {
        $scope.portDescriptionEnable = !($scope.portDescriptionEnable); 
        $scope.portDescription = '';
    };

    $scope.updatePortDescription = function() {
        SecretHttp({
            url: '/api/interface/all/',
            method: 'PUT',
            data:{
                operate : 'description',
                key : $scope.dispPortName,
                desc : $scope.portDescription,
                up : '',
                speed : '',
                mode : '',
                vlan_type : '',
                jumboframe_en : '',
            },
        })
        .then(function(resp) {
            if (resp.status == 200) {
                if(resp.data['error']) {
                    Alert(resp.data['err_reason']);
                }
            }
            else {
                Alert('http server error!');
            }
        }); 

        $scope.load();
    }

    /**************************************************************************************************/    
    /******************* Port Type Switch *************************************************************/   
    /**************************************************************************************************/  
    $scope.switchPortType = function() {

        SecretHttp({
            url: '/api/interface/all/',
            method: 'PUT',
            data:{
                operate : 'switchPortType',
                key : 'eth-0-1',
                switchPortType : $scope.portType,
            },
        })
        .then(function(resp) {
            if (resp.status == 200) {
                if(resp.data['error']) {
                    Alert(resp.data['err_reason']);
                }
            }
            else {
                Alert('http server error!');
            }
        }); 

        $scope.load();
    }

    /**************************************************************************************************/    
    /******************* port split *******************************************************************/   
    /**************************************************************************************************/  
    $scope.viewDetailPortSplist = function(port) {
        $scope.dispPortName = port.full_name;
        $scope.dispPort = port;
        var len = $scope.split_status_arr.length;
        var first_split_port = port.sort_name;
        
        if ($scope.split_status_arr[first_split_port].is_split == 'True') {
            // eth-0-1_2 or eth-0-1_3 ... to  eth-0-1_1, split property in first split port
            len = port.sort_name.length;
            first_split_port = port.sort_name.substring(0,len-1) + '1';
        }
        if (port.split_type == 'splited') {
            if ($scope.split_status_arr[first_split_port].split_type == '0'){
                $scope.split_cfg_has_changed = true;
            }
        }else if(port.split_type == 'not_split') {
            if ($scope.split_status_arr[first_split_port].split_type != '0'){
                $scope.split_cfg_has_changed = true;
            }
        }

        $('#portSplitModal').modal(); 

    };

    $scope.getSplitStatus = function() {

        SecretApi({
            url: '/api/interface/split/',
            method: 'GET',
        }, function(data) {
            var index = parseInt(data.port_media_type);

            $scope.portType = $scope.portSwitchType[index];
            $scope.split_status_arr = data.arr;

        }, function() {
            $scope.loading = false;
        });

        return false;
    };

    $scope.aggregatePort = function() {
        SecretHttp({
            url: '/api/interface/all/',
            method: 'PUT',
            data:{
                operate : 'aggregatePort',
                key : $scope.dispPort.key,
            },
        })
        .then(function(resp) {
            if (resp.status == 200) {
                if(resp.data['error']) {
                    Alert(resp.data['err_reason']);
                }
            }
            else {
                Alert('http server error!');
            }
        }); 

        $scope.load();
    }

    $scope.clearAggregatePort = function() {
        SecretHttp({
            url: '/api/interface/all/',
            method: 'PUT',
            data:{
                operate : 'undoAggregatePort',
                key : $scope.dispPort.key,
            },
        })
        .then(function(resp) {
            if (resp.status == 200) {
                if(resp.data['error']) {
                    Alert(resp.data['err_reason']);
                }
            }
            else {
                Alert('http server error!');
            }
        }); 

        $scope.load();
    }

    $scope.splitPort = function() {

        SecretHttp({
            url: '/api/interface/all/',
            method: 'PUT',
            data:{
                operate : 'splitPort',
                key : $scope.dispPortName,
                split_type : $scope.split_type,
            },
        })
        .then(function(resp) {
            if (resp.status == 200) {
                if(resp.data['error']) {
                    Alert(resp.data['err_reason']);
                }
            }
            else {
                Alert('http server error!');
            }
        }); 

        $scope.load();
    }

    $scope.clearSplitPort = function() {
        SecretHttp({
            url: '/api/interface/all/',
            method: 'PUT',
            data:{
                operate : 'undoSplitPort',
                key : $scope.dispPortName,
                split_type : $scope.split_type,
            },
        })
        .then(function(resp) {
            if (resp.status == 200) {
                if(resp.data['error']) {
                    Alert(resp.data['err_reason']);
                }
            }
            else {
                Alert('http server error!');
            }
        }); 

        $scope.load();
    }

    $scope.closeSplitMoal = function() {
        $scope.load();
    }
    /**************************************************************************************************/    
    /******************* Port Duplex Modal **********************************************************/   
    /**************************************************************************************************/  
    $scope.$watch('dispPortName', function(value) {
        //$scope._refreshDetail();
    });
    $scope.viewDuplexDetail = function(port) {

        $scope.dispPortName = port.full_name;

        $('#portDuplexModal').modal();  
    };

    $scope.viewSvlanTpidDetail = function(port) {

        $scope.dispPortName = port.full_name;

        $('#portSvlanTpidModal').modal();
    };

    $scope.updatePortDuplex = function(duplex) {

        SecretApi({
            url: '/api/interface/all/',
            method: 'PUT',
            data:{
                operate : 'duplex',
                key : $scope.dispPortName,
                desc : '',
                up : '',
                duplex : duplex,
                speed : '',
                mode : '',
                vlan_type : '',
                jumboframe_en : '',
            },
         }, function(data) {
           $scope.load();
        });
    };

   $scope.updatePortSvlanTpid = function(svlan_tpid) {

        SecretApi({
            url: '/api/interface/all/',
            method: 'PUT',
            data:{
                operate : 'svlan_tpid',
                key : $scope.dispPortName,
                desc : '',
                up : '',
                duplex : '',
                svlan_tpid : svlan_tpid,
                speed : '',
                mode : '',
                vlan_type : '',
                jumboframe_en : '',
            },
        }, function(data) {
           $scope.load();
        });
   };
    /**************************************************************************************************/    
    /******************* Port Flow Modal **********************************************************/   
    /**************************************************************************************************/  
    $scope.$watch('dispPortName', function(value) {
        //$scope._refreshDetail();
    });
    $scope.viewSpeedDetail = function(port) {

        $scope.dispPortName = port.full_name;
        /*
        if (port.egs_acl.length) {
            $scope.portFlowEn = true;
            $scope.portFlowName = port.egs_acl;
        } */
        $('#portSpeedModal').modal();  
    };

    $scope.updatePortSpeed = function(speed) {

        SecretApi({
            url: '/api/interface/all/',
            method: 'PUT',
            data:{
                operate : 'speed',
                key : $scope.dispPortName,
                desc : '',
                up : '',
                speed : speed,
                mode : '',
                vlan_type : '',
                jumboframe_en : '',
            },
        }, function(data) {
           $scope.load();
        });
    };

    /**************************************************************************************************/    
    /******************* Vlan Type Modal **********************************************************/   
    /**************************************************************************************************/  

    $scope.viewVlanTypeDetail = function(port) {
        $scope.dispPortName = port.key;
        $scope.dispPort     = port;

        $('#portVlanTypeModal').modal();  
    };

    $scope.updatePortVlanType = function() {
        SecretApi({
            url: '/api/interface/all/',
            method: 'PUT',
            data:{
                operate   : 'vlan_type',
                key       : $scope.dispPortName,
                vlan_type : $scope.portVlanType,
            },
        }, function(data) {
           $scope.load();
        });
    };

   /**************************************************************************************************/    
    /******************* protocol mode Modal **********************************************************/   
    /**************************************************************************************************/  

    $scope.viewProtocolModeDetail = function(port) {
        $scope.dispPortName = port.key;
        $scope.dispPort     = port;

        $('#portProtlModeModal').modal();  
    };

    $scope.updatePortProcotolMode = function() {
        SecretApi({
            url    : '/api/interface/all/',
            method : 'PUT',
            data:{
                operate   : 'mode',
                key       : $scope.dispPortName,
                mode      : $scope.portProtlMode,
            },
        }, function(data) {
           $scope.load();
        });
    };


   /**************************************************************************************************/    
    /******************* protocol mode Modal **********************************************************/   
    /**************************************************************************************************/  

    $scope.viewJumboDetail = function(port) {
        $scope.dispPortName = port.key;
        $scope.dispPort     = port;

        $('#portJumboframeModal').modal();  
    };

    $scope.updatePortJumbo = function() {

        SecretApi({
            url    : '/api/interface/all/',
            method : 'PUT',
            data:{
                operate       : 'jumboframe_en',
                key           : $scope.dispPortName,
                jumboframe_en : $scope.jumboframe_en,
            },
        }, function(data) {
           $scope.load();
        });
    };
    /**************************************************************************************************/    
    /******************* Loard  ***********************************************************************/   
    /**************************************************************************************************/  
    $scope.load = function() {

        $scope.split_cfg_has_changed = false;

        $scope.getIf();
        $scope.getFlow();
        $scope.getSplitStatus();
    };

    $scope.load();

}]);
