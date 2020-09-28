;Public.app.controller('ctlHomePortMgrStatistic', ['$scope', 'SecretApi', 'Alert', '$interval', '$location', '$document', function($scope, SecretApi, Alert, $interval, $location, $document) {
    /* AJAX */
    
    /* user defined */
    $scope.loading = false;
    $scope.stats = {if_statistic_arr: []};

    $scope.multiSel = [];
    $scope.multiSelAllCtl = false;

    $scope.selected = {arrIndex: []};

    $scope.dispIfIdx = 0;
    $scope.dispDetail = {}; // detail

    $scope.load = function() {
        $scope.loading = true;

        SecretApi({
            url: '/api/if_statistic/all',
            method: 'GET',
        }, function(data) {
            $scope.stats = data;
            $scope.statsInfo = data.if_statistic_arr;
            $scope.ifidx = _.map($scope.stats.if_statistic_arr, function(obj) {return obj.ifidx});
        }, function() {
            $scope.loading = false;
        });
    };
    
    /**
     * select
     */
    $scope.setCheckboxAll = function(opt) {
    
        var len = $scope.statsInfo.length;
        switch(opt) {
            case true:
                for(i = 0; i < len; i++) {
                    $scope.multiSel[i] = true;
                }
                $scope.multiSelAllCtl = true;
                break;
            case false:
                for(i = 0; i < len; i++) {
                    $scope.multiSel[i] = false;
                }
                $scope.multiSelAllCtl = false;
                break;
            case 'inverse':
                for(i = 0; i < len; i++) {
                    $scope.multiSel[i] = ($scope.multiSel[i] == true) ? false : true;
                }
                break;
        }
    };
    
    $scope.getMultiSelIndex = function() {
        var arr = [];
        for(var i = 0; i < $scope.multiSel.length; i++) {
            if($scope.multiSel[i]) {
                arr.push(i);
            }
        } 
        return arr;
    };

    $scope.hasSel = function() {
        var arr = $scope.getMultiSelIndex();
        var res = true;
        if(arr.length == 0) return false;

        $scope.selected.arrIndex = arr;
        return res;
    };

    $scope.update = function() {
        $scope.load();
    }

    $scope.clearByPortArr = function(arrPort) {
        SecretApi({
            url: '/api/if_statistic/all',
            method: 'POST',
            data: {
                ifidx_arr: arrPort,
            },
        }, function(data) {
            $scope.load();
        }, function() {
            $scope.loading = false; 
        });

        $scope.loading = true;
    }

    $scope.clear = function() {
        var arrPort = _.map($scope.selected.arrIndex, function(index) {return $scope.stats.if_statistic_arr[index].ifname;});
        $scope.clearByPortArr(arrPort);
    }

    /* BEGIN: display statistic detail */
    $scope.$watch('dispIfIdx', function(value) {
        $scope._refreshDetail();
    });
    $scope.$watch('stats', function(value) {
        $scope._refreshDetail();
    });

    $scope.viewDetail = function(x) {
        $scope.dispIfIdx = x.ifidx;
        $scope.dispIfName = x.ifname;
        $('#statisticDetailModal').modal(); 
    };

    $scope._refreshDetail = function() {
        var value = $scope.dispIfIdx;
        for(var i = 0; i < $scope.stats.if_statistic_arr.length; i++) {
            var tmp = $scope.stats.if_statistic_arr[i];
            if(tmp.ifidx == value) {
                $scope.dispDetail = tmp;
                return;
            }
        }
    };

    $scope.clearDetail = function() {
        var arr = [$scope.dispIfName];
        $scope.clearByPortArr(arr);
    };
    /* END: display statistic detail */

    $scope.load();

    // fixed
    var $obj = $document.find('#if-statistic-modify');
    $obj.find('.panel').width($obj.width());
}]);
