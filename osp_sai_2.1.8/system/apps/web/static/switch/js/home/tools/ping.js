;Public.app.controller('ctlHomeToolsPing', ['$scope', 'SecretApi', 'Alert', '$rootScope', function($scope, SecretApi, Alert, $rootScope) {

    $scope.content = ['N/A'];
    $scope.loading = false;

    if ($rootScope.System_product == 'Tap-350' || $rootScope.System_product == 'Tap-580'
        || $rootScope.System_product == 'Tap-550' || $rootScope.System_product == 'Tap-530') {
        $scope.vrfIds= ['mgmt'];
    } else {
        $scope.vrfIds= ['mgmt', 'default'];
    }

    /* model */
    $scope.selVrf = $scope.vrfIds[0];
    $scope.count = 4;
    $scope.destIP = '127.0.0.1';

    /* functions */
    $scope.iftable = function() {

        SecretApi({
        url: '/api/if_statistic/all',
        method: 'GET',
    }, function(data) {
        $scope.ifname_all = data.if_statistic_arr;

    });

    }
    $scope.ping = function() {
        $scope.loading = true;
        $scope.content = '';
        var jsonData = JSON.stringify($scope.srcif);
        SecretApi({
            url: '/api/ping/'+ $scope.count +'/'+ $scope.selVrf +'/'+ $scope.destIP +'/'+ $scope.srcIP +'/'+ jsonData +'/',
            method: 'GET',
        }, function(data) {
            $scope.content = data.ping_resp;
        }, function() {
            $scope.loading = false;
        });
    };
}]);
