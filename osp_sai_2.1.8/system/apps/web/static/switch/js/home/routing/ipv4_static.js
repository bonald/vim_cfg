
;Public.app.controller('ctlHomeIPv4Static', ['$scope', 'SecretHttp', 'Alert', function($scope, SecretHttp, Alert) {

    /* AJAX */
    $scope.addRoute = {};
    $scope.multiSel = []; /* for delete channel group */
    $scope.routing = [];
    $scope.routing_arr = [];

    /* pagination */
    $scope.pgCurrent = 1;
    $scope.pgItemCnt = 0;
    $scope.pgItemNumEachPage = 10;
    
    /* user defines */
    $scope.onPageChange = function() {
    	
    };
    
    $scope.mask_default = 
    {
       select: "255.255.255.0(24)"
    }
    $scope.mask = 
    [
      "0.0.0.0(0)",
      "128.0.0.0(1)",
      "192.0.0.0(2)",
      "224.0.0.0(3)",
      "240.0.0.0(4)",
      "248.0.0.0(5)",
      "252.0.0.0(6)",
      "254.0.0.0(7)",
      "255.0.0.0(8)",
      "255.128.0.0(9)",
      "255.192.0.0(10)",
      "255.224.0.0(11)",
      "255.240.0.0(12)",
      "255.248.0.0(13)",
      "255.252.0.0(14)",
      "255.254.0.0(15)",      
      "255.255.0.0(16)",    
      "255.255.128.0(17)",          
      "255.255.192.0(18)",     
      "255.255.224.0(19)",     
      "255.255.240.0(20)",     
      "255.255.248.0(21)",     
      "255.255.252.0(22)",     
      "255.255.254.0(23)",     
      "255.255.255.0(24)",     
   
      "255.255.255.128(25)",    
      "255.255.255.192(26)",    
      "255.255.255.224(27)",    
      "255.255.255.240(28)",    
      "255.255.255.248(29)",          
      
      "255.255.255.252(30)", 
      "255.255.255.254(31)", 
      "255.255.255.255(32)"       
    ];


    /* functions */

    $scope.setCheckboxAll = function(state) {
        for(var i = 0; i < $scope.routing.length; i++) {
            $scope.multiSel[i] = state; 
        }
    }

    $scope.deleteRouteItem = function(entry_arr) {

        SecretHttp({
            url: '/api/ipv4_static/ipv4_static_route_delete/',
            method: 'DELETE',
            data:{
                entry_arr : entry_arr,
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
    };

    $scope.deleteRouteItemMulti = function() {
        var entry_arr = [];
        var temp = {};
        
        for(var i = 0; i < $scope.routing.length; i++) {
            if($scope.multiSel[i]) {

                entry_arr.push({ip: $scope.routing[i].ip, nexthop: $scope.routing[i].nexthop});

            }
        }

        $scope.deleteRouteItem(entry_arr);  
        $scope.loadRoute(); 
        $scope.SELCTLALL = '';
    };
    
    $scope.addStaticRouteItem = function() {

        SecretHttp({
            url: '/api/ipv4_static/ipv4_static_route_set/',
            method: 'PUT',
            data:{
                dest : $scope.addRoute.dest,
                mast : $scope.mask.indexOf($scope.mask_default.select),
                nexthop : $scope.addRoute.nexthop,
                distance : $scope.addRoute.distance,
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
                
        $scope.addRoute.dest = '';    
        $scope.addRoute.mast = '';  
        $scope.addRoute.nexthop = '';  
        $scope.addRoute.distance = '';  
        $scope.loadRoute();
        //$scope.addStaticRouteItemShow = false;  
    };


    /* functions */

    $scope.loadRoute = function() {
        $scope.routing = [];
        var len = 0;
        SecretHttp({
            url: '/api/ipv4_routing/all/',
            method: 'GET',
        })
        .then(function(resp) {
            if(resp.status == 200) {

                $scope.routing = resp.data.static;
                $scope.multiSel = [];
                $scope.pgItemCnt = $scope.routing.length;           
            }
            else {
                Alert(resp.data['err_reason']);
            }
        });
        
    };
    
    $scope.load = function() {
        $scope.loadRoute();
    };    
    
    $scope.load();
    
}]);


