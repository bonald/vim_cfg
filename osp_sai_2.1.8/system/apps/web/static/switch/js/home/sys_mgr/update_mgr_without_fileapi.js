;Public.app.controller('ctlHomeSysMgrUpdateMgrWithoutFileApi', ['$scope', 'SecretApi', 'Alert', 'GetToken', function($scope, SecretApi, Alert, GetToken) {
    /* ajax */
    $scope.bootImage = {};
    $scope.version = {};

    /* user defines */
    $scope.formAction = Public.apiServer + '/boot/upload_image_legacy/';
    $scope.username = GetToken().username;
    $scope.token = GetToken().token;

    /* functions */
    $scope.load = function() {
        SecretApi({
            url: '/api/bootimage/',
            method: 'GET',
        }, function(data) {
            $scope.bootImage = data.arr[0];
        });

        SecretApi({
            url: '/api/version/',
            method: 'GET',
        }, function(data) {
            $scope.version = data.arr[0];
        });
    };

    $scope.load();
}]);
