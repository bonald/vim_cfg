;Public.app.controller('ctlHomePortMgrLinkAgg', ['$scope', 'SecretApi', 'Alert', '$timeout', '$rootScope', function($scope, SecretApi, Alert, $timeout, $rootScope) {

        /* AJAX */
    if($rootScope.System_product == 'Tap-530') {
        $scope.state_tap530 = {};
    }else{
        $scope.state = {};
    }
    /* AJAX */
    $scope.state = {};
    $scope.channelGroup = [];
    $scope.interface = [];
    $scope.allAggIf = []; /* all agg interface */
    $scope.ignoreDisableIf = [];
    $scope.tunnleHashMode = '';//['outer', 'both', 'inner'];
    $scope.configure = {};

    /* user defines */
    $scope.switchState = false;
    $scope.stateLoading = {};
    $scope.multiSel = []; /* for delete channel group */
    $scope.multiSelIf = []; /* for add channel group */
    $scope.newAggNum = '';
    $scope.addChannelGroupShow = false;
    $scope.modifyChannelGroupFlag = false;
    $scope.ChannelNameShow = '';
    $scope.selected_mode = 'static-channel-group';
    $scope.aggMode = 'static';
    $scope.AggID = '';
    $scope.existChannelGroupNum ='';
    $scope.ifexistChannelGroup = false;
    $scope.channel_group_select = 'active';
    $scope.setip_enable = true;
    $scope.setipv6_enable = true;
    $scope.setmpls_enable = true;
    $scope.nvgreval='nvgre-outer';
    $scope.vxlanval='vxlan-outer';
    $scope.module_all=['l2','mpls','ip','ipv6','--gre','--nvgre','--vxlan'];
    $scope.module = 'l2';


    if ($rootScope.System_product == 'Tap-350' || $rootScope.System_product == 'Tap-580'|| $rootScope.System_product == 'Tap-550' || $rootScope.System_product == 'Tap-530'){
        $scope.aggmode_select = ['static-channel-group'];
    }else{
        $scope.aggmode_select = ['static-channel-group','channel-group'];
    }

    $scope.channel_group_mode = ['active','passive'];
    if ($rootScope.System_product == 'Tap-350' || $rootScope.System_product == 'Tap-580'|| $rootScope.System_product == 'Tap-550' || $rootScope.System_product == 'Tap-530') {
        $scope.lbMode = [ 'static', 'round-robin'];
    }else{
        $scope.lbMode = [ 'static', 'dynamic', 'round-robin'];
    }
    
    /* functions */

    $scope.loadLoadBalance = function() {
        SecretApi({
            url: '/api/lag/load_balance/', 
        }, function(data) {
            if ($rootScope.System_product == 'Tap-580'|| $rootScope.System_product == 'Tap-550') {
                 $scope.state = data.state;
            }else if($rootScope.System_product == 'Tap-530'){
                $scope.state_tap530 = data.state_tap530;
                $scope.setip_enable = data.setip_enable;
                $scope.setipv6_enable = data.setipv6_enable;
                $scope.setmpls_enable = data.setmpls_enable;
            } else if ($rootScope.System_product == 'Tap-350') {
                $scope.state['src-mac'] = data.state['src-mac'];
                $scope.state['dst-mac'] = data.state['dst-mac'];
                $scope.state['src-ip']  = data.state['src-ip'];
                $scope.state['dst-ip']  = data.state['dst-ip'];
                $scope.state['src-port'] = data.state['src-port-l4'];
                $scope.state['dst-port'] = data.state['dst-port-l4'];
                $scope.state['src-dst-ip'] = data.state['src-dst-ip'];
                $scope.state['src-dst-mac'] = data.state['src-dst-mac'];
                $scope.state['src-dst-port'] = data.state['src-dst-port'];                
            } else {
                /* FixMe. ugly code */
                if($scope.board_product=='10'){
                    $scope.state['src-mac'] = data.state['src-mac'];
                    $scope.state['dst-mac'] = data.state['dst-mac'];
                    $scope.state['src-ip']  = data.state['src-ip'];
                    $scope.state['dst-ip']  = data.state['dst-ip'];
                    $scope.state['ip-protocol'] = data.state['ip-protocol'];
                    $scope.state['src-port-l4'] = data.state['src-port-l4'];
                    $scope.state['dst-port-l4'] = data.state['dst-port-l4'];
                    $scope.state['inner-src-mac'] = data.state['inner-src-mac'];
                    $scope.state['inner-dst-mac'] = data.state['inner-dst-mac'];
                    $scope.state['inner-src-ip'] = data.state['inner-src-ip'];
                    $scope.state['inner-dst-ip'] = data.state['inner-dst-ip'];
                    $scope.state['inner-ip-protocol'] = data.state['inner-ip-protocol'];
                    $scope.state['inner-src-port-l4'] = data.state['inner-src-port-l4'];
                    $scope.state['inner-dst-port-l4'] = data.state['inner-dst-port-l4'];
                }else if($scope.board_product==' 1'){
                    $scope.state['src-mac'] = data.state['src-mac'];
                    $scope.state['dst-mac'] = data.state['dst-mac'];
                    $scope.state['src-ip']  = data.state['src-ip'];
                    $scope.state['dst-ip']  = data.state['dst-ip'];
                    $scope.state['src-port'] = data.state['src-port-l4'];
                    $scope.state['dst-port'] = data.state['dst-port-l4'];
                    $scope.state['src-dst-ip'] = data.state['src-dst-ip'];
                    $scope.state['src-dst-mac'] = data.state['src-dst-mac'];
                    if($scope.state['src-dst-mac']==true){
                        $scope.state['src-mac']=false;
                        $scope.state['dst-mac'] =false;
                    }
                    if($scope.state['src-dst-ip']==true){
                        $scope.state['src-ip']=false;
                        $scope.state['dst-ip'] =false;
                    }

                    $scope.state['src-dst-port'] = data.state['src-dst-port'];
                    if($scope.state['src-dst-port']==true){
                        $scope.state['src-port']=false;
                        $scope.state['dst-port'] =false;
                    }
                }else{
                    $scope.state['src-mac'] = data.state['src-mac'];
                    $scope.state['dst-mac'] = data.state['dst-mac'];
                    $scope.state['src-ip']  = data.state['src-ip'];
                    $scope.state['dst-ip']  = data.state['dst-ip'];
                    $scope.state['ip-protocol'] = data.state['ip-protocol'];
                    $scope.state['src-port-l4'] = data.state['src-port-l4'];
                    $scope.state['dst-port-l4'] = data.state['dst-port-l4'];
                    $scope.state['inner-src-mac'] = data.state['inner-src-mac'];
                    $scope.state['inner-dst-mac'] = data.state['inner-dst-mac'];
                    $scope.state['inner-src-ip'] = data.state['inner-src-ip'];
                    $scope.state['inner-dst-ip'] = data.state['inner-dst-ip'];
                    $scope.state['inner-ip-protocol'] = data.state['inner-ip-protocol'];
                    $scope.state['inner-src-port-l4'] = data.state['inner-src-port-l4'];
                    $scope.state['inner-dst-port-l4'] = data.state['inner-dst-port-l4'];
                }

            }
            if($rootScope.System_product == 'Tap-530'){
                $scope.stateLoading = {};
                _.each(_.keys($scope.state_tap530), function(key) {$scope.stateLoading[key] = false;});
            }else{
                $scope.stateLoading = {};
                _.each(_.keys($scope.state), function(key) {$scope.stateLoading[key] = false;});
            }
        });
    };

    $scope.loadChannelGroup = function() {
        SecretApi({
            url: '/api/lag/channel_group/',
        }, function(data) {
            $scope.channelGroup = data.group_arr;
            $scope.multiSel = [];

            $scope.allAggIf = [];
            for(var i = 0; i < $scope.channelGroup.length; i++) {
                $scope.allAggIf = _.union($scope.allAggIf, $scope.channelGroup[i].member_ports);
            }
        });
    };

    $scope.loadIf = function() {
        SecretApi({
            url: '/api/interface/all/',
        }, function(data) {
            $scope.interface = _.filter(data.arr, function(item) {return item.hw_type == 'Port';});
            $scope.multiSelIf = [];
        });
    };

    $scope.loadTunnelHashMode = function() {

        SecretApi({
            url: '/api/lag/load_balance_tunnel_hash/',
            method: 'GET',
        }, function(data) {
            //$scope.tunnleHashMode = 'outer';
            $scope.configure = data.arr;
            if ($scope.configure.load_balance_inner_field_en == 0) {
                $scope.tunnleHashMode = 'outer';
            }
            if ($scope.configure.load_balance_inner_field_en == 1) {
                $scope.tunnleHashMode = 'both';
            }
            if ($scope.configure.load_balance_inner_field_en == 2) {
                $scope.tunnleHashMode = 'inner';
            }
        })
    };

    $scope.getproduct=function(){

         SecretApi({
            url: '/api/lag/getproduct/',
            method: 'GET',
        }, function(data){
            $scope.board_product = data.product;
        })
    };


    $scope.load = function() {
        $scope.getproduct();
        $scope.loadLoadBalance();
        $scope.loadChannelGroup();
        $scope.loadTunnelHashMode();
        $scope.loadIf();
    };
/*
    $scope.changeTunnelHashState = function() {
        var state = $scope.tunnleHashMode;
        SecretApi({
            url: '/api/lag/update_load_balance_tunnel_hash/'+state+'/',
            method: 'SET',
        }, function(data) {
            $scope.load();
        })
    };
*/
    $scope.cfgTunnelHashFun = function(key) {
        $scope.tunnleHashMode = key;
		var state = $scope.tunnleHashMode;
        SecretApi({
            url: '/api/lag/update_load_balance_tunnel_hash/'+state+'/',
            method: 'SET',
        }, function(data) {
            $scope.loadTunnelHashMode();
        })
    }

    $scope.changeState = function(key) {
        var opt = $scope.state[key] ? 'unset' : 'set';
        $scope.stateLoading[key] = true;

        SecretApi({
            url: '/api/lag/load_balance/'+key+'/'+opt+'/',
            method: 'PUT',
        }, function() {
            $scope.loadLoadBalance();
        });

        return false;
    }

    $scope.setCheckboxAll = function(state) {
        for(var i = 0; i < $scope.channelGroup.length; i++) {
            if($scope.channelGroup[i].protocol == 'static') {
                $scope.multiSel[i] = state; 
            }
        }
    }

    $scope.addChannelGroup = function() {
        var gid = parseInt($scope.newAggNum);
        var ifs = [];
        var agg_mode_seclected = 0;
        var channel_mode=0;;
        if ($scope.selected_mode =='static-channel-group'){
             agg_mode_seclected = 1;
        }else{
                agg_mode_seclected = 2;
                if ($scope.channel_group_select =='active'){
                    channel_mode=1;
                }else{
                    channel_mode=2;
                }
        }
        for(var i = 0; i < $scope.multiSelIf.length; i++) {
            if($scope.multiSelIf[i]) ifs.push($scope.interface[i].key);
        }

        for (var i = 0; i < $scope.channelGroup.length; i++) {
            $scope.existChannelGroup = $scope.channelGroup[i].lag_name;
            $scope.existChannelGroupNum = parseInt($scope.existChannelGroup.match(/\d+/));

            if ($scope.existChannelGroupNum == gid) {
                $scope.ifexistChannelGroup = true;
            }
        }

        if ($scope.ifexistChannelGroup == false){
            SecretApi({
                url: '/api/lag/channel_group/'+gid+'/'+agg_mode_seclected+'/'+channel_mode+'/',
                method: 'PUT',
                data: {
                    if_arr: ifs,
                },
            }, function(data) {
                $scope.loadChannelGroup();
                $scope.addChannelGroupShow = false;
            });
        }
        else {
            $scope.loadChannelGroup();
            $scope.addChannelGroupShow = false;
            $scope.ifexistChannelGroup = false;
            Alert('This Agg Channel Group has been in existence!');
        }

    };

    $scope.deleteChannelGroup = function(if_arr) {
        SecretApi({
            url: '/api/lag/channel_group/',
            method: 'DELETE',
            data: {
                if_arr: if_arr,   
            },
        }, function(data) {
            $scope.loadChannelGroup();
        });
    };

    $scope.deleteChannelGroupMulti = function() {
        var if_arr = [];
        for(var i = 0; i < $scope.channelGroup.length; i++) {
            if($scope.multiSel[i]) {
                if_arr = _.union(if_arr, $scope.channelGroup[i].member_ports);
            }
        }
        $scope.deleteChannelGroup(if_arr);
    };


    $scope.modifyChannelGroup = function(aggName, if_arr) {
        $scope.addChannelGroupShow = true;
        $scope.modifyChannelGroupFlag = true;
        $scope.ChannelNameShow = aggName;

        var if_tmp = _.map($scope.interface, function(item) {return item.sort_name;});

        /* get id */
        $scope.newAggNum = parseInt(aggName.match(/\d+/));
        /* set selected */
        _.each(if_arr, function(item) {
            index = if_tmp.indexOf(item);
            if(index != -1) {
                $scope.multiSelIf[index] = true;
            }
        });
        /* get ignore disable ports */
        $scope.ignoreDisableIf = if_arr;
    };
    
    $scope.setAggMode = function() {
        
        SecretApi({
            url: '/api/lag/channel_agg_mode/'+$scope.AggID+'/',
            method: 'PUT',
            data: {
                mode: $scope.aggMode,   
            },
        }, function(data) {
            $scope.loadChannelGroup();
            $scope.addChannelGroupShow = false;
        });
    };



    $scope.setgrefield = function(){
        SecretApi({
                url: '/api/lag/hash_gre/'+$scope.state_tap530["gre-gre-key"]+'/'+$scope.state_tap530["gre-gre-protocol"]+'/'+$scope.state_tap530["gre-ipda"]+'/'+$scope.state_tap530["gre-ipsa"]+'/'+$scope.state_tap530["gre-src-interface"]+'/',
                method: 'PUT',
        }, function() {
                $scope.loadLoadBalance();
        });
    }

    $scope.setl2field = function(){
        $scope.enable_msg(false);
        SecretApi({
                url: '/api/lag/hash_l2/'+$scope.state_tap530["l2-eth-type"]+'/'+$scope.state_tap530["l2-macda"]+'/'+$scope.state_tap530["l2-macsa"]+'/'+$scope.state_tap530["l2-src-interface"]+'/'+$scope.state_tap530["l2-vlan"]+'/',
                method: 'PUT',
        }, function() {
                $scope.loadLoadBalance();
        });
    }



    $scope.setipfield = function(){
        SecretApi({
                url: '/api/lag/hash_ip/'+$scope.state_tap530["ip-l4-destport"]+'/'+$scope.state_tap530["ip-l4-sourceport"]+'/'+$scope.state_tap530["ip-ipda"]+'/'+$scope.state_tap530["ip-ipsa"]+'/'+$scope.state_tap530["ip-ip-protocol"]+'/'+$scope.state_tap530["ip-src-interface"]+'/',
                method: 'PUT',
        }, function() {
                $scope.loadLoadBalance();
        });
    }

    $scope.setipv6field = function(){
        SecretApi({
                url: '/api/lag/hash_ipv6/'+$scope.state_tap530["ipv6-l4-destport"]+'/'+$scope.state_tap530["ipv6-l4-sourceport"]+'/'+$scope.state_tap530["ipv6-ipda"]+'/'+$scope.state_tap530["ipv6-ipsa"]+'/'+$scope.state_tap530["ipv6-ip-protocol"]+'/'+$scope.state_tap530["ipv6-src-interface"]+'/',
                method: 'PUT',
        }, function() {
                $scope.loadLoadBalance();
        });
    }

    $scope.setmplsfield = function(){
        SecretApi({
                url: '/api/lag/hash_mpls/'+$scope.state_tap530["mpls-2nd-label"]+'/'+$scope.state_tap530["mpls-3rd-label"]+'/'+$scope.state_tap530["mpls-source-interface"]+'/'+$scope.state_tap530["mpls-top-label"]+'/',
                method: 'PUT',
        }, function() {
                $scope.loadLoadBalance();
        });
    }

    $scope.setnvgrefield = function(){
        SecretApi({
                url: '/api/lag/hash_nvgre/'+$scope.state_tap530["nvgre-inner2-eth-type"]+'/'+$scope.state_tap530["nvgre-inner2-macda"]+'/'+$scope.state_tap530["nvgre-inner2-macsa"]+'/'+$scope.state_tap530["nvgre-inner3-ip-protocol"]+'/'+$scope.state_tap530["nvgre-inner3-ipda"]+'/'+$scope.state_tap530["nvgre-inner3-ipsa"]+'/'+$scope.state_tap530["nvgre-outer-gre-protocol"]+'/'+$scope.state_tap530["nvgre-outer-ipda"]+'/'+$scope.state_tap530["nvgre-outer-ipsa"]+'/'+$scope.state_tap530["nvgre-vsid"]+'/'+$scope.state_tap530["nvgre-src-interface"]+'/'+$scope.nvgreval+'/',
                method: 'PUT',
        }, function() {
                $scope.loadLoadBalance();
        });
    }

    $scope.setvxlanfield = function(){
        SecretApi({
                url: '/api/lag/hash_vxlan/'+$scope.state_tap530["vxlan-inner2-eth-type"]+'/'+$scope.state_tap530["vxlan-inner2-macda"]+'/'+$scope.state_tap530["vxlan-inner2-macsa"]+'/'+$scope.state_tap530["vxlan-inner3-l4-destport"]+'/'+$scope.state_tap530["vxlan-inner3-ip-protocol"]+'/'+$scope.state_tap530["vxlan-inner3-ipda"]+'/'+$scope.state_tap530["vxlan-inner3-ipsa"]+'/'+$scope.state_tap530["vxlan-inner3-l4-sourceport"]+'/'+$scope.state_tap530["vxlan-outer-l4-destport"]+'/'+$scope.state_tap530["vxlan-outer-ipda"]+'/'+$scope.state_tap530["vxlan-outer-ipsa"]+'/'+$scope.state_tap530["vxlan-outer-l4-sourceport"]+'/'+$scope.state_tap530["vxlan-outer-vlan"]+'/'+$scope.state_tap530["vxlan-src-interface"]+'/'+$scope.state_tap530["vxlan-vni"]+'/'+$scope.vxlanval+'/',
                method: 'PUT',
        }, function() {
                $scope.loadLoadBalance();
        });
    }


    $scope.setipenable = function(){
        SecretApi({
                url: '/api/lag/hash_ip_enable/' + $scope.setip_enable + '/',
                method: 'PUT',
        }, function() {
                $scope.loadLoadBalance();
                $scope.enable_msg($scope.setip_enable);
        });
    }

    $scope.setipv6enable = function(){
        SecretApi({
                url: '/api/lag/hash_ipv6_enable/' + $scope.setipv6_enable + '/',
                method: 'PUT',
        }, function() {
                $scope.loadLoadBalance();
                $scope.enable_msg($scope.setipv6_enable);
        });
    }

    $scope.setmplsenable = function(){
        SecretApi({
                url: '/api/lag/hash_mpls_enable/' + $scope.setmpls_enable + '/',
                method: 'PUT',
        }, function() {
                $scope.loadLoadBalance();
                $scope.enable_msg($scope.setmpls_enable);
        });
    }

    $scope.nvgrehashmode = function(mode){
        if(mode == 'outer') {
            $scope.nvgreval='nvgre-outer';
        }
        else if(mode == 'layer2'){
            $scope.nvgreval='nvgre-inner2';
        }
        else{
            $scope.nvgreval='nvgre-inner3';
        }
    }

    $scope.vxlanhashmode = function(mode){
        if(mode == 'outer') {
            $scope.vxlanval='vxlan-outer';
        }
        else if(mode == 'layer2'){
            $scope.vxlanval='vxlan-inner2';
        }
        else{
            $scope.vxlanval='vxlan-inner3';
        }
    }

    $scope.enable_msg = function(h_enable){

        if(h_enable == false){
            if($rootScope.currentUseLanguage == 'zh_CN'){
                alert("注意：IP/ipv6/mpls字段hash使能情况�?报文会匹配IP/IPv6/mpls进行hash，匹配失败会根据l2进行hash!");
            }
            else{
                alert("if IP/ipv6/mpls hash-field enable, hash based on IP/IPv6/mpls, forwading will be based on l2 hash when IP/IPv6/mpls matching fails!");
            }
        }
    }

    $scope.load();
}]);


