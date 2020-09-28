
;Public.app.controller('ctlHomeIPv4Routing', ['$scope', 'SecretApi', 'Alert', function($scope, SecretApi, Alert) {

    /* AJAX */
    $scope.tabsUsed = 'connected'; // 'connected' or 'kernel' or 'static', 'RIP', 'OSPF'....
    $scope.routing_arr = [];
    $scope.routing_static = [];
    $scope.routing_connet = [];  
    $scope.routing_ospf = [];
    $scope.multiSel = []; /* for delete channel group */
    
    /* pagination */
    $scope.pgCurrent_static = 1;
    $scope.pgItemCnt_static = 0;

    $scope.pgCurrent_connet = 1;
    $scope.pgItemCnt_connet = 0;

    $scope.pgCurrent_ospf = 1;
    $scope.pgItemCnt_ospf = 0;

    $scope.pgItemNumEachPage = 10;

     /* functions */

    $scope.setCheckboxConnectecdAll = function(state) {
        for(var i = 0; i < $scope.routing_connet.length; i++) {
            $scope.multiSel[i] = state;
        }
    }

    $scope.setCheckboxStaticAll = function(state) {
        for(var i = 0; i < $scope.routing_static.length; i++) {
            $scope.multiSel[i] = state;
        }
    }

    $scope.setCheckboxOspfAll = function(state) {
        for(var i = 0; i < $scope.routing_ospf.length; i++) {
            $scope.multiSel[i] = state;
        }
    }

    $scope.orderItems = [
    {model: 'ifname', display: 'ifname'},
    {model: 'IP/Mask', display: 'iP/Mask'},
    {model: 'Nextop', display: 'nextop'},
    ],

    $scope.order = {
        reverse: false,
        by: $scope.orderItems[0],
    };
    
    /* user defines */
    $scope.onPageChange = function() {
    	
    };
    
    $scope.changeTabs = function(tab) {
        $scope.tabsUsed = tab;
        $scope.load();
    };

    $scope.loadRoute = function() {
        SecretApi({
            url: '/api/ipv4_routing/all/',
            method: 'GET',
        }, function(data) {

                $scope.routing_arr = data.static;

                $scope.routing_static = data.static;     
                $scope.pgItemCnt_static = $scope.routing_static.length;   

                $scope.routing_connet = data.connect;     
                $scope.pgItemCnt_connet = $scope.routing_connet.length;   

                $scope.routing_ospf = data.ospf;  
                $scope.pgItemCnt_ospf = $scope.routing_ospf.length;             
            }
        )
        };

    /* functions */
    $scope.load = function() {
        $scope.loadRoute();      
    };  
    
    $scope.load(); 
    
}]);


