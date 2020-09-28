;Public.app.controller('ctlHomeSecuDDosPrevent', ['$scope', 'SecretApi', 'Alert', function($scope, SecretApi, Alert) {

    $scope.prevent_arr = [];

    $scope.new_pre = {
        smallpkt : '',
        icmpmaxcnt : '',
        udpmaxcnt : '',
        tcpmaxcnt : '',

    };

    $scope.clear = function(){
        SecretApi({
            url: '/api/security/ddos_prevent/',
            method: 'POST',
        }, function(data) {
            $scope.load();
        });
    };
    
    $scope.ddosenable = function(type) {
        var prevent_type = type;
        var prevent_value = 0;
        switch(type) {
            case 'smpkt':
                prevent_value = $scope.new_pre.smallpkt;
                break;
            case 'icmp':
                prevent_value = $scope.new_pre.icmpmaxcnt;
                break;
            case 'udp':
                prevent_value = $scope.new_pre.udpmaxcnt;
                break;
            case 'tcp':
                prevent_value = $scope.new_pre.tcpmaxcnt;
                break;
        }
        
        SecretApi({
            url: '/api/security/ddos_prevent/',
            method: 'PUT',
            data: {
                type: prevent_type,
                value:prevent_value,
            }
        }, function(data) {
            $scope.load();
        });
    };
    
    $scope.ddosdisable = function(type) {
        var prevent_type = type;
        switch(type) {
            case 'smpkt':
                $scope.new_pre.smallpkt = '';
                break;
            case 'icmp':
                $scope.new_pre.icmpmaxcnt = '';
                break;
            case 'udp':
                $scope.new_pre.udpmaxcnt = '';
                break;
            case 'tcp':
                $scope.new_pre.tcpmaxcnt = '';
                break;
        }
                        
        SecretApi({
            url: '/api/security/ddos_prevent/',
            method: 'DELETE',
            data: {
                type: prevent_type,
            }
        }, function(data) {
            $scope.load();
        });
    };
    
    $scope.changeDDosState = function(type) {
        var prevent_type = type;
        var prevent_value = 0;
        switch(type) {
            case 'smurf':
                prevent_value = $scope.prevent_arr.ipt_cfg_smurf != 0 ? 0 : 1;
                break;
            case 'fraggle':
                prevent_value = $scope.prevent_arr.ipt_cfg_fraggle != 0 ? 0 : 1;
                break;
            case 'maceq':
                prevent_value = $scope.prevent_arr.ipt_cfg_maceq != 0 ? 0 : 1;
                break;
            case 'ipeq':
                prevent_value = $scope.prevent_arr.ipt_cfg_ipeq != 0 ? 0 : 1;
                break;
        }
                        
        SecretApi({
            url: '/api/security/ddos_prevent/',
            method: 'PUT',
            data: {
                type: prevent_type,
                value:prevent_value,
            }
        }, function(data) {
            $scope.load();
        });
    };

    
    $scope.load = function() {
        SecretApi({
            url: '/api/security/ddos_prevent/', 
            method: 'GET',
        }, function(data) {
            $scope.status = data;
            $scope.prevent_arr = data.arr;
            //console.log($scope.prevent_arr);
            $scope.prevent_arr.smlpkt_state = $scope.prevent_arr.ipt_cfg_small_pkt != 0? 'Enable' : 'Disable';
            $scope.prevent_arr.smlpkt_value = $scope.prevent_arr.ipt_cfg_small_pkt != 0? $scope.prevent_arr.small_pkt_length : 'None';
            $scope.prevent_arr.tcp_state = $scope.prevent_arr.ipt_cfg_tcp != 0? 'Enable' : 'Disable';
            $scope.prevent_arr.tcp_value = $scope.prevent_arr.ipt_cfg_tcp != 0? $scope.prevent_arr.tcp_rate_limit : 'None';
            $scope.prevent_arr.icmp_state = $scope.prevent_arr.ipt_cfg_icmp != 0? 'Enable' : 'Disable';
            $scope.prevent_arr.icmp_value = $scope.prevent_arr.ipt_cfg_icmp != 0? $scope.prevent_arr.icmp_rate_limit : 'None';
            $scope.prevent_arr.udp_state = $scope.prevent_arr.ipt_cfg_udp != 0? 'Enable' : 'Disable';
            $scope.prevent_arr.udp_value = $scope.prevent_arr.ipt_cfg_udp != 0? $scope.prevent_arr.udp_rate_limit : 'None';
            $scope.prevent_arr.smurf_state = $scope.prevent_arr.ipt_cfg_smurf != 0? 'Enable' : 'Disable';
            $scope.prevent_arr.fraggle_state = $scope.prevent_arr.ipt_cfg_fraggle != 0? 'Enable' : 'Disable';
            $scope.prevent_arr.maceq_state = $scope.prevent_arr.ipt_cfg_maceq != 0? 'Enable' : 'Disable';
            $scope.prevent_arr.ipeq_state = $scope.prevent_arr.ipt_cfg_ipeq != 0? 'Enable' : 'Disable';
        });
    };
    $scope.load();


}]);
