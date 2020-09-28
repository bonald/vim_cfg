;Public.app.controller('ctlHomeTacasMgr', ['$scope', 'SecretApi', 'Alert', '$interval', '$location', '$document','$rootScope', function($scope, SecretApi, Alert, $interval, $location, $document,$rootScope) {

    $scope.server_list = []; 
    $scope.serverHost = ''; 
    $scope.serverKey = ''; 
    $scope.srvVrfsel = 'mgmt';
	$scope.serverKeyEn = false;

    if ($rootScope.System_product == 'Tap-350' || $rootScope.System_product == 'Tap-580'
        || $rootScope.System_product == 'Tap-550' || $rootScope.System_product == 'Tap-530') {
        $scope.vrfIds= ['mgmt'];
    } else {
        $scope.vrfIds= ['mgmt', 'default'];
    }

    $scope.getTacasServer = function() {
        SecretApi({
            url: '/api/tacas_mgr/tacas_server/',
            method: 'GET',
        }, function(data) {
            $scope.server_list = data.server;
        }, function() {

        });
    };

    $scope.addTacasServer = function(server) {

        SecretApi({
            url: '/api/tacas_mgr/tacas_server/',
            method: 'PUT',
            data: {
                host: $scope.serverHost,
                KeyEn: $scope.serverKeyEn,
                key: $scope.serverKey,
                inband : $scope.srvVrfsel,
            },
        }, function(data) {
            $scope.load();
        }, function() {

        });
    };

    $scope.delTacasServer = function(server) {
        SecretApi({
            url: '/api/tacas_mgr/tacas_server/',
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
        $scope.getTacasServer();
    };

    $scope.load();

}]);
