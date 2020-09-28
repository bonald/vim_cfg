;Public.app.controller('ctlHomeTapFLowStatistics', ['$scope', 'SecretApi', 'Alert', '$interval', '$location', '$document', function($scope, SecretApi, Alert, $interval, $location, $document) {
    /* AJAX */

    /* user defined */
    $scope.loading = false;
    $scope.stats = {if_statistic_arr: []};
    $scope.flow_statistics= {};
    $scope.if_name;


    $scope.load = function() {
        $scope.loading = true;

        SecretApi({
            url: '/api/tap_statistics/all/',
            method: 'GET',
        }, function(data) {
            $scope.stats = data;
        }, function() {
            $scope.loading = false;
        });
    };


    $scope.GetFlowStatistics = function(key) {
        $scope.if_name = key;
        SecretApi({
            url: '/api/tap_statistics/ifname/' + $scope.if_name,
            method: 'GET',
        }, function(data) {
            $scope.flow_statistics = data.flow_statistics;
        });
    };

    $scope.load();
}]);
