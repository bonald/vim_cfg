;Public.app.controller('ctlHomeRebootAndSave', ['$scope', 'SecretApi', 'Alert', 'Confirm', '$location', function($scope, SecretApi, Alert, Confirm, $location) {

    $scope.reboot = function() {
        Confirm('Are you sure reboot?', function() {
            $location.path('/home/reboot');
        });    
    };

    $scope.save = function() {
        Confirm('Are you sure save current Configuration?', function() {
            SecretApi({
                url: '/api/save_conf/',
                method: 'PUT',
            }, function(data) {
                Alert({
                    type: 'success',
                    title: 'Save successful'
                });
            });
        });    
    };

    $scope.saveAndReboot = function() {
        Confirm('Are you sure reboot?', function() {
            SecretApi({
                url: '/api/save_conf/',
                method: 'PUT',
            }, function(data) {
                $location.path('/home/reboot/save');
            });
        });    
    };
}]);
