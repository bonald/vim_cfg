;Public.app.controller('ctlHomeSecurityWebSessions', ['$scope', 'SecretApi', 'Alert', '$document', '$timeout', 'Confirm', function($scope, SecretApi, Alert, $document, $timeout, Confirm) {

    /* ajax */
    $scope.sess = [];

    /* user defined */
    $scope.multiSelSess = {};
    $scope.curToken = localStorage.getItem('token');	

    /* functions */
    $scope.load = function() {
        $scope.multiSelSess = {};
        $scope.loadSess();
    };

    $scope.loadSess = function() {
        SecretApi({
            url: '/api/websess/',
            method: 'GET',
        }, function(data) {
            $scope.sess = data.sess;
        });
    };

    /**
     * @param sess_arr: [sess1, sess2, ...]
     */
    $scope.deleteSess = function(sess_arr) {
        SecretApi({
            url: '/api/websess/',
            method: 'DELETE',
            data: {
                'sess_arr': sess_arr,
            }
        }, function(data) {
            $scope.load();
        });
    };


    /**
     * @param status
     *      bool true: select all
     *      bool false: clear all
     *      string 'inverse': inverse select
     */
    $scope.setCheckboxAll = function(status) {
        _.each($scope.sess, function(x) {
            if(_.isBoolean(status)) {
                $scope.multiSelSess[x.id] = status
            } else if(status == 'inverse') {
                tmp = $scope.multiSelSess[x.id];
                if(_.isUndefined(tmp))  $scope.multiSelSess[x.id] = true;
                else                    $scope.multiSelSess[x.id] = !$scope.multiSelSess[x.id];
            }
        })
    };

    $scope.multiDelEn = function() {
        var values = _.values($scope.multiSelSess);
        return _.contains(values, true);
    };

    $scope.deleteMultiSess = function() {
        var arr = []
		_.each($scope.sess, function(x) {
            if($scope.multiSelSess[x.id]) {
                arr.push(x.id)
            }
        })
        $scope.deleteSess(arr)
    }

    $scope.load();
}]);
