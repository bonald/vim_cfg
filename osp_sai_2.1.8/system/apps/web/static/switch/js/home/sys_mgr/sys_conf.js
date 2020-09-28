;Public.app.controller('ctlHomeSysMgrSysConf', ['$scope', 'SecretApi', 'Alert', 'Base64', function($scope, SecretApi, Alert, Base64) {

    /* AJAX */

    /* user defined */
    $scope.net = {mgmtIP: '', gateway: ''};
    $scope.sys = {hostname: ''};
    
    $scope.netRef = {mgmtIP: '', gateway: ''};
    $scope.sysRef = {hostname: ''};

    /* functions */
    $scope.loadNet = function() {
        SecretApi({
            url: '/api/manage_if/', 
        }, function(data) {
            $scope.net.mgmtIP = data.ip;
            $scope.net.gateway = data.gateway;

            $scope.netRef.mgmtIP = data.ip;
            $scope.netRef.gateway = data.gateway;
        });
    };

    $scope.loadSys = function() {
        SecretApi({
            url: '/api/sys_global/', 
        }, function(data) {
            $scope.sys.hostname = data.hostname;
            $scope.sysRef.hostname = data.hostname;
        });
    };

    $scope.load = function() {
        $scope.loadNet();
        $scope.loadSys();
    };

    $scope.modifyIpAddr = function() {
        SecretApi({
            url: '/api/manage_if/ip/'+Base64.encodeString($scope.net.mgmtIP)+'/', 
            method: 'PUT',
        }, function() {
            Alert({type: 'success', title: 'Success', content: 'OK!', time: 1});
            $scope.loadNet();
        });
    };

    $scope.modifyGateway = function() {
        SecretApi({
            url: '/api/manage_if/gateway/'+Base64.encodeString($scope.net.gateway)+'/', 
            method: 'PUT',
        }, function() {
            Alert({type: 'success', title: 'Success', content: 'OK!', time: 1});
            $scope.loadNet();
        });
    };

    $scope.modifyHostname = function() {
        SecretApi({
            url: '/api/sys_global/', 
            method: 'PUT',
            data: {
                hostname: $scope.sys.hostname, 
            },
        }, function() {
            Alert({type: 'success', title: 'Success', content: 'OK!', time: 1});
            $scope.loadSys();
        });
    };

    $scope.load();
}]);
