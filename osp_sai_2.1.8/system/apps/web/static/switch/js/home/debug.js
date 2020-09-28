;Public.app.controller('ctlHomeDebug', ['$scope', 'SecretApi', function($scope, SecretApi) {

    var historyId = 0;

    $scope.resp = {
        status: 0,
        lines: [''],
    };
    $scope.history = [];
    $scope.cmd = '';

    function appendHistory(cmd) {
        var obj = {id: historyId, cmd: cmd};
        $scope.history.push(obj);
        historyId++;
    }

    $scope.sendCmd = function() {
        appendHistory($scope.cmd);

        var base64 = BASE64.encoder($scope.cmd);
        SecretApi({
            url: '/api/shell/'+base64+'/', 
        }, function(data) {
            $scope.cmd = '';
            $scope.resp = data.resp;
        });
    };

    $scope.updateCmd = function(cmd) {
        $scope.cmd = cmd; 
    };

    $scope.clearHistory = function() {
        $scope.history = []; 
        $scope.historyId = 0;
    };
}]);
