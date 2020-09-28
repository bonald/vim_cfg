;Public.app.controller('ctlHomeSrvMgrVlanAccessTrunk', ['$scope', 'SecretApi', '$document', 'Range2Array', function($scope, SecretApi, $document, Range2Array) {

    /* AJAX */
    $scope.ifInfo = [];
    
    /* user defined */
    $scope.loading = false;

    $scope.multiSel = [];
    $scope.multiSelAllCtl = false;

    $scope.selected = {arrIndex: [], type: ''};
    $scope.modify = {pvid: '', permitVlan: ''};


    /* function */

    $scope.loadIfInfo = function() {
        SecretApi({
            url: '/api/interface/all/',
        }, function(data) {
            $scope.multiSel = [];
//            $scope.ifInfo = data.arr;
            $scope.ifInfo = _.filter(data.arr, function(item) {return item.hw_type == 'Port';});
        });
    };

    $scope.load = function() {
        $scope.loadIfInfo();
    };

    /**
     * select
     */
    $scope.setCheckboxAll = function(opt) {
    
        var len = $scope.ifInfo.length;
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
        var type = false;
        var arr = $scope.getMultiSelIndex();
        var res = true;
        if(arr.length == 0) return false;

        _.each(arr, function(index) {
            if(type == false) {
                type = $scope.ifInfo[index].vlan_type;
                if(type != 'access' && type != 'trunk') {
                    res = false; 
                    return;
                }
            }
            else {
                if($scope.ifInfo[index].vlan_type != type) {
                    res = false;
                    return;
                } 
            }
        });

        $scope.selected.arrIndex = arr;
        $scope.selected.type = type;
        return res;
    };

    $scope.modify = function() {
        var type = $scope.selected.type;
        var pvid = $scope.modify.pvid;
        var permitVlan = $scope.modify.permitVlan;
        var arrPort = _.map($scope.selected.arrIndex, function(index) {return $scope.ifInfo[index].key;});

        if(type == 'access') {
            SecretApi({
                url: '/api/vlan/access/',
                method: 'PUT',
                data: {
                    vlan_id: pvid,
                    interface_arr: arrPort,
                },
            }, function(data) {
                $scope.load();
            }, function() {
                $scope.loading = false; 
            });
        }
        else if(type == 'trunk') {
            SecretApi({
                url: '/api/vlan/trunk/',
                method: 'PUT',
                data: {
                    vlan_id: pvid,
                    permit_vlan: permitVlan,
                    interface_arr: arrPort,
                },
            }, function(data) {
                $scope.load();
            }, function() {
                $scope.loading = false; 
            });
        }

        $scope.loading = true;
    }

    $scope.vlanRangeAvail = function() {
        var str = $scope.modify.permitVlan;
        if(!str) return false;

        var arr = Range2Array(str, true);
        if(arr.length == 0) return false;

        return !_.some(arr, function(val) {return (val < 1 || val > 4094)});
    };

    $scope.load();

    // fixed
    var $obj = $document.find('#access-trunk-multi-modify');
    $obj.find('.panel').width($obj.width());
}]);
