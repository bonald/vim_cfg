;Public.app.controller('ctlHomeToolsTraceroute', ['$scope', 'SecretApi', 'Alert', '$rootScope', function($scope, SecretApi, Alert, $rootScope) {

    $scope.content = ['N/A'];
    $scope.loading = false;
    $scope.timeout = 5;
    $scope.waringDisp = false;

    if ($rootScope.System_product == 'Tap-350' || $rootScope.System_product == 'Tap-580'
        || $rootScope.System_product == 'Tap-550' || $rootScope.System_product == 'Tap-530') {
        $scope.vrfIds= ['mgmt'];
    } else {
        $scope.vrfIds= ['mgmt', 'default'];
    }

    /* model */
    $scope.selVrf = $scope.vrfIds[0];
    $scope.destIP = '127.0.0.1';

    /* functions */
    $scope.exec = function() {
        $scope.loading = true;
        $scope.content = '';

        SecretApi({
            url: '/api/traceroute/'+ $scope.selVrf +'/'+ $scope.destIP +'/'+$scope.timeout+'/',
            method: 'GET',
        }, function(data) {
            $scope.content = data.trace_resp;
        }, function() {
            $scope.loading = false;
        });
    };
}]);
