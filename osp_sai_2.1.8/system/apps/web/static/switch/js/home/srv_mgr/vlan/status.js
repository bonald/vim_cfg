;Public.app.controller('ctlHomeSrvMgrVlanStatus', ['$scope', '$filter', 'SecretApi', '$timeout', 'Range2Array', 'Array2Range', 'Confirm', function($scope, $filter, SecretApi, $timeout, Range2Array, Array2Range, Confirm) {

    $scope.vlanFiltered = [];
    $scope.vlanDisp = [];

    /* ajax */
    $scope.objIfIndex = {}; // e.g. {'1': 'eth-0-1', '2': 'eth-0-2'}
    $scope.status = {vlan_arr: []};
    $scope.initLoading = true;

    /* user defines */
    $scope.multiSel = [];
    $scope.ids = [];
    $scope.newVlan = {id: '', range: '', name: ''}; // {'id': ?, 'name': ?} 
    $scope.loading = false;
    $scope.showAddForm = false;
    $scope.nameFilter = '';

    $scope.modifyVlanNameId = 0;
    $scope.newVlanName = '';
    $scope.vlanMemberPortIndex = []; // e.g. [[1, 2], ...];
    $scope.newVlanType = 'id'; // 'id' or 'range'

    /* pagination */
    $scope.pgCurrent = 1;
    $scope.pgItemCnt = 0;
    $scope.pgItemNumEachPage = 10;

    /* watch */
    $scope.$watch('nameFilter', function(value) {
        $scope.pgCurrent = 1;
        $scope._refresh();
    });
    
    $scope.onPageChange = function() {
        $scope._refresh('pageOnly');
    };

    /**
     * @param state:
     *      'pageOnly': 不重新过滤数据，仅仅跳转页面
     *      int: 重新过滤数据并跳转到指定页面
     *      undefined: 什么参数都不传则仅仅重新过滤数据
     */
    $scope._refresh = function(state) {
        if($scope.status.vlan_arr.length == 0) return;

        $scope.setCheckboxAll(false);

        if(state != 'pageOnly') {
            $scope.vlanFiltered = $filter('orderBy')($scope.status.vlan_arr, 'id');
            $scope.vlanFiltered = $filter('filter')($scope.vlanFiltered, $scope.nameFilter);
            $scope.pgItemCnt = $scope.vlanFiltered.length;

            if(_.isNumber(state)) {
                $scope.pgCurrent = state;
            }
        }

        var current = $scope.pgCurrent;
        $scope.vlanDisp = $filter('limitTo')($scope.vlanFiltered, $scope.pgItemNumEachPage, ($scope.pgCurrent - 1) * $scope.pgItemNumEachPage);

        /* 当删除最后一页的所有元素后，需要跳转到前一页 */
        if((current - 1) * $scope.pgItemNumEachPage == $scope.pgItemCnt) { 
            $scope._refresh(current - 1);
        }

    };


    /* functions */
    $scope.refreshTooltip = function() {
        $timeout(function() {
            $('[data-toggle="tooltip"]').tooltip({container: 'body'});
            $('.tooltip.fade.bottom.in').remove();
        }, 0);
    };


    $scope.getAllVlanMemberPortKey = function() {
        $scope.vlanMemberPortIndex = [];
        var vlan_arr = $scope.status.vlan_arr;

        for(var i = 0; i < vlan_arr.length; i++) {
            var portArr = Range2Array(vlan_arr[i].member_port);
            var id = vlan_arr[i].id;
            $scope.vlanMemberPortIndex[id] = portArr;
        };
    }

    /* refresh page */
    $scope.load = function() {
        $scope.initLoading = true;

        SecretApi({
            url: '/api/vlan/',
            method: 'GET',
        }, function(data) {
            $scope.status = data;
            $scope.ids = _.map($scope.status.vlan_arr, function(obj) {return parseInt(obj.id)});
            $scope.ids.splice($scope.ids.indexOf(1), 1)
            $scope.multiSel = [];
            $scope.setCheckboxAll(false);
            $scope.refreshTooltip();
            $scope.getAllVlanMemberPortKey();

            $scope._refresh(1);
        }, function() {
            $scope.initLoading = false; 
        });
    }

    $scope.loadIf = function() {
        SecretApi({
            url: '/api/interface/all/',
        }, function(data) {
            $scope.objIfIndex = {};
            _.each(data.arr, function(item) {
                $scope.objIfIndex[item.ifindex.toString()] = item.key;
            });

            $scope.load();
        });
    };

    /* delete vlan */
    $scope.deleteVlan = function(vlan_ids) {
        $scope.loading = true;

        SecretApi({
            url: '/api/vlan/', 
            method: 'DELETE',
            data: {
                vlan_ids: vlan_ids,
            },
        }, function(data) {
//            $scope.load(); 
            if(vlan_ids.length > 1) { // multi del
                $scope.status.vlan_arr = _.filter($scope.status.vlan_arr, function(item) {
                    return !$scope.multiSel[item.id];
                });
            }
            else if(vlan_ids.length == 1) {
                $scope.status.vlan_arr = _.filter($scope.status.vlan_arr, function(item) {
                    return item.id != vlan_ids[0]
                });
            }
            $scope.multiSel = [];
            $scope._refresh();
            $scope.refreshTooltip();
        }, function() {
            $scope.loading = false;
        });
    };


    /* multi delete */
    $scope.multiDel = function() {
        var arrDel = _.filter($scope.ids, function(id) { return $scope.multiSel[id];}); 
        $scope.deleteVlan(arrDel);
    };


    $scope.deleteAllVlan = function() {
        Confirm('Are you sure delete all VLAN?', function() {
            $scope.loading = true;

            var arr = _.filter($scope.status.vlan_arr, function(val) {return parseInt(val.id) != 1;});
            var arr = _.map(arr, function(val) {return val.id;});
            var range_str = Array2Range(arr);
            SecretApi({
                url: '/api/vlan/range/',
                method: 'DELETE',
                data: {
                    range: range_str, 
                },
            }, function() {
                $scope.load();
            }, function() {
                $scope.loading = false;
            });
        })
    };

    /**
     * @param opt:
     *     true: select all
     *     false: cancel all
     *     'invert': invert select
     */
    $scope.setCheckboxAll = function(opt) {
        switch(opt) {
            case true:
                _.each($scope.vlanDisp, function(x) {
                    if(x.id != 1) {
                        $scope.multiSel[x.id] = true;
                    }
                });
                break;
            case false:
                $scope.MULSELCTL = false;
                _.each($scope.vlanDisp, function(x) {
                    $scope.multiSel[x.id] = false;
                });
                break;
        }
    };

    /* multi delete and modify is active ? */
    $scope.isMultiActive = function() {
        return _.contains(_.values($scope.multiSel), true);
    };

    /* add vlan */
    $scope.addVlan = function() {
        $scope.loading = true;

        /* add new vlan by vlan id */
        if($scope.newVlanType == 'id') {
            var id = $scope.newVlan.id;
            var name = $scope.newVlan.name;

            SecretApi({
                url: '/api/vlan/',
                method: 'POST',
                data: {
                    id: id,
                    name: name,
                },
            }, function(data) {
                $scope.load();
                /*
                $scope.status.vlan_arr.push({
                    id: id,
                    name: name,
                    member_port: '',
                });
                $scope._refresh();
                */
            }, function() {
                $scope.loading = false;
            });
        }

        /* add new vlan by range */
        if($scope.newVlanType == 'range') {
            var range_str = $scope.newVlan.range;

            SecretApi({
                url: '/api/vlan/range/',
                method: 'POST',
                data: {
                    range: range_str, 
                }
            }, function(data) {
                $scope.load();
            }, function() {
                $scope.loading = false; 
            });
        }
    };

    /* modify vlan name */
    $scope.modifyVlanName = function(id) {
        if(!isNaN(parseInt(id))) {
            $scope.modifyVlanNameId = parseInt(id);
            $('#modifyVlanNameModal').modal();
        }
        else { // send request
            var vid = $scope.modifyVlanNameId;
            var vname = $scope.newVlanName;

            SecretApi({
                url: '/api/vlan/',
                method: 'PUT',
                data: {
                    id: vid,
                    name: vname,
                },
            }, function() {
//                $scope.load();
                for(var i = 0; i < $scope.status.vlan_arr.length; i++) {
                    if($scope.status.vlan_arr[i].id == vid) {
                        $scope.status.vlan_arr[i].name = vname; 
                    } 
                }
                $scope._refresh();
            });
        }
    };

    /* if addVlanForm.vlanName.$dirty is false, modify vlanName by vlanId, eg: id: 1 -> name: VLAN1 */
    $scope.checkIfModifyName = function() {
        if(!$scope.addVlanForm.vlanName.$dirty && $scope.newVlan.id) {
            $scope.newVlan.name = '';
            //$scope.newVlan.name = 'VLAN' + $scope.newVlan.id;
        }
        else {
            $scope.newVlan.name = '';
        }
    };

    /* check if vlan already exist */
    $scope.checkVlanExist = function() {
        var res = false;
        var vid = parseInt($scope.newVlan.id);
        if(isNaN(vid)) return true;

        _.each($scope.status.vlan_arr, function(item) {
            if(item.id == vid) {
                res = true;
                return;
            }
        });
        return res;
    };

    $scope.vlanRangeAvail = function() {
        var str = $scope.newVlan.range;
        if(!str) return false;

        var arr = Range2Array(str, true);
        if(arr.length == 0) return false;

        return !_.some(arr, function(val) {return (val < 2 || val > 4094)});
    };


    $scope.loadIf();
}]);
