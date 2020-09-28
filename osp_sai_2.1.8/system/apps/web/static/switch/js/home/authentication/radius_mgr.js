;Public.app.controller('ctlHomeRadiusMgr', ['$scope', 'SecretApi', 'Alert', '$interval', '$location', '$document','$rootScope', function($scope, SecretApi, Alert, $interval, $location, $document,$rootScope) {

    $scope.cfg_list = []; 
    $scope.dot1x_radius = [];
    $scope.srvVrfsel = 'mgmt';
	$scope.serverKeyEn = false;
    $scope.serverKey = ''; 
    $scope.serverHost = ''; 

    if ($rootScope.System_product == 'Tap-350' || $rootScope.System_product == 'Tap-580'
        || $rootScope.System_product == 'Tap-550' || $rootScope.System_product == 'Tap-530') {
        $scope.vrfIds= ['mgmt'];
    } else {
        $scope.vrfIds= ['mgmt', 'default'];
    }

    $scope.getRadiusServer = function() {
        SecretApi({
            url: '/api/radius_mgr/radius_server/',
            method: 'GET',
        }, function(data) {
            $scope.cfg_list = data.auth_cfg;
            $scope.dot1x_radius = data.dot1x_radius;
        }, function() {

        });
    };

    $scope.addRadiusServer = function(server) {
        SecretApi({
            url: '/api/radius_mgr/radius_server/',
            method: 'PUT',
            data: {
                host: $scope.serverHost,
                inband : $scope.srvVrfsel,
                KeyEn: $scope.serverKeyEn,
                key: $scope.serverKey,
            },
        }, function(data) {
            $scope.load();
        }, function() {

        });
    };

    $scope.delRadiusServer = function(server) {
        SecretApi({
            url: '/api/radius_mgr/radius_server/',
            method: 'DELETE',
            data: {
                host: server.host,
                inband : server.is_inband,
            },
        }, function(data) {
            $scope.load();
        }, function() {

        });
    };

    $scope.serverKeyEnFun = function() {
        $scope.serverKeyEn = !($scope.serverKeyEn);
        $scope.serverKey = ''; 
    }; 
    /**************************************************************************************************/        
    $scope.load = function() {
        $scope.getRadiusServer();
    };

    $scope.load();

}]);
