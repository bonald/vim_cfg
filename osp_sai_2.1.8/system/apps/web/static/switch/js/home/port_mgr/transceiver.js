
;Public.app.controller('ctlHomeTransceiverStatus', ['$scope', 'SecretHttp', 'SecretApi', 'Alert', function($scope, SecretHttp, SecretApi, Alert) {

    $scope.loading = false;

    $scope.if = [];
    $scope.setif = false;
    $scope.chk = false;
    $scope.chooseOne = false;
    $scope.selectedIf = {};
    $scope.newIf = {};

    $scope.getIf = function() {
        $scope.if = [];

        SecretHttp({
            url: '/api/transceiver/all/',
            method: 'GET',
        })
            .then(function(resp) {
                if(resp.status == 200) {
                    $scope.ifp = resp.data.arr;
                    var len = $scope.ifp.length;
                    for (i=0; i<len; i++){
                        
                        if($scope.ifp[i].interface_name.search('eth')>=0){
                            $scope.if.push($scope.ifp[i]);
                            $scope.if[i].box = false;
                        }
                    }
                }
                else {
                    Alert(resp.data['err_reason']);
                }
            });
    };


    $scope.load = function() {
        $scope.getIf();
    };

    $scope.load();
}]);
