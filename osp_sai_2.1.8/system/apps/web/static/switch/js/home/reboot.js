;Public.app.controller('ctlHomeReboot', ['$scope', '$rootScope', '$http', 'SecretApi', 'Alert', '$routeParams', '$interval', '$location', function($scope, $rootScope, $http, SecretApi, Alert, $routeParams, $interval, $location) {
    var url = '/api/boot/reboot/false/';

    if($routeParams.save) {
        url = '/api/boot/reboot/true/';
    }

    SecretApi({
        url: url,
        method: 'GET',
    }, function() {
        $scope.waitReboot();
    });


    $scope.waitReboot = function() {
        var vm = $scope.vm = {};
        vm.value = 0;
        vm.style = 'progress-bar-striped';
        vm.showLabel = true;
        vm.striped = true;
        var index = 0;
        var timeId = 1000;
        var start = $interval(function(){
        vm.value = parseInt((++index)/1.8);
        if (index > 180) {
            vm.value = 100;
            $interval.cancel(start);
            $rootScope.logout();
            window.location.reload();
        }
        if (index > 80){
            var respFlag = true;
            $http({
               url: '/api/server_state/',
               method: 'GET',
          }).then(function(resp) { // success
               respFlag = true;
              if(!_.isUndefined(resp.data.state) && resp.data.state == true) {
                   $interval.cancel(start);
                   vm.value = 100;
                   $rootScope.logout();
                   window.location.reload();
               }
        });

      }




    }, timeId);
  }


}]);