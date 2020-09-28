;Public.app.controller('ctlHomeSrvMgrMacMacAddressTable', ['$scope', '$filter', 'SecretApi', 'Alert', '$timeout', function($scope, $filter, SecretApi, Alert, $timeout) {

    $scope.fdbFiltered = [];
    $scope.fdbDisp = [];
    
    /* AJAX */
    $scope.initLoading = true;
    $scope.macAddressTable = [];
    $scope.portIndexObj = {}; // e.g. {'1': 'eth-0-1', '2': 'eth-0-2'}
    $scope.loading = false;

    /* user defined */
    $scope.multiSel = {};
    $scope.addFdbFlag = false; /* display add form when flag is true */
    $scope.newFdb = {
        macAddr: '',
        port: '',
        vlan: '',
    };

    /* pagination */
    $scope.pgCurrent = 1;
    $scope.pgItemCnt = 0;
    $scope.pgItemNumEachPage = 10;

    /* filter */
    $scope.fdbFilterStr = '';
    $scope.vlanFilterStr = '';
    $scope.macAddressFilterStr = '';

    $scope.portOptions = ['']; // e.g. ['eth-0-1', 'eth-0-2', ...]
    $scope.portFilterStr = $scope.portOptions[0];

    $scope.typeOptions = ['']; // e.g. ['static', 'dynamic',...]
    $scope.typeFilterStr = $scope.typeOptions[0];


    /* watch */
    $scope.$watch('fdbFilterStr', function(value) {
        $scope.pgCurrent = 1;
        $scope._refresh();
    });
    $scope.$watch('vlanFilterStr', function(value) {
        $scope.pgCurrent = 1;
        $scope._refresh();
    });
    $scope.$watch('macAddressFilterStr', function(value) {
        $scope.pgCurrent = 1;
        $scope._refresh();
    });
    $scope.$watch('portFilterStr', function(value) {
        $scope.pgCurrent = 1;
        $scope._refresh();
    });
    $scope.$watch('typeFilterStr', function(value) {
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
        $scope.setCheckboxAll(false);

        if(state != 'pageOnly') {
            $scope.fdbFiltered = $filter('orderBy')($scope.macAddressTable, 'mac_address')
            $scope.fdbFiltered = $filter('filter')($scope.fdbFiltered, {
                vlan_id: $scope.vlanFilterStr,
                mac_address: $scope.macAddressFilterStr,
                type: $scope.typeFilterStr,
                port: $scope.portFilterStr,
            });
            $scope.fdbFiltered = $filter('filter')($scope.fdbFiltered, $scope.fdbFilterStr);

            $scope.pgItemCnt = $scope.fdbFiltered.length;
            if(_.isNumber(state)) {
                $scope.pgCurrent = state;
            }
        }
        $scope.fdbDisp = $filter('limitTo')($scope.fdbFiltered, $scope.pgItemNumEachPage, ($scope.pgCurrent - 1) * $scope.pgItemNumEachPage);
    };


    /* functions */
    $scope.refreshTooltip = function() {
        $timeout(function() {
            $('[data-toggle="tooltip"]').tooltip({container: 'body'});
            $('.tooltip.fade.bottom.in').remove();
        }, 0);
    };

    $scope.getAllPort = function() {
        $scope.portIndexObj = {};

        SecretApi({
            url: '/api/interface/all/',
        }, function(data) {
            data.arr = _.filter(data.arr, function(item) {
                return item.hw_type == 'Port' || item.hw_type == 'LAG'; 
            });
            _.each(data.arr, function(item) {
                $scope.portIndexObj[item.ifindex] = item.key;
            });
            $scope.newFdb.port = $scope.portIndexObj[data.arr[0].ifindex];
        });
    };

    $scope.getKey = function(mac_address, port, vlan_id) {
        var str = mac_address + port + vlan_id.toString();
        return BASE64.encoder(str);
    }

    /**
     * mac address table没有id，需要自己手动创建索引
     */
    $scope.addKey = function(fdb) {
        for(var i = 0; i < fdb.length; i++) {
            var item = fdb[i];
            fdb[i].key = $scope.getKey(item.mac_address, item.port, item.vlan_id);
        }
        return fdb;
    };

    $scope.load = function() {
        $scope.initLoading = true;

        SecretApi({
            url: '/api/mac_address_table/',
        }, function(data) {
            $scope.multiSel = {};

            $scope.macAddressTable = $scope.addKey(data.mac_address_table);
            $scope.refreshAllOptions();
            $scope.refreshTooltip();

            $scope._refresh(1);
        }, function() {
            $scope.initLoading = false;
        });
    };

    $scope.refreshAllOptions = function() {
        var portOptions = {'': true}; // e.g. {'eth-0-1': true, 'eth-0-2': true, ...}
        var typeOptions = {'': true}; // e.g. {'static': true, 'dynamic': true,...}
        
        _.each($scope.macAddressTable, function(item) {
            /* port options */
            portOptions[item.port] = true;

            /* type options */
            typeOptions[item.type] = true;
        });

        $scope.portOptions = _.keys(portOptions);
        $scope.typeOptions = _.keys(typeOptions);
    };

    /**
     * select
     */
    $scope.setCheckboxAll = function(opt) {
        switch(opt) {
            case true:
                _.each($scope.fdbDisp, function(x) {
                    $scope.multiSel[x.key] = true;
                });
                break;
            case false:
                $scope.SELCTLALL = false;
                _.each($scope.fdbDisp, function(x) {
                    $scope.multiSel[x.key] = false;
                });
                break;
        }
    };

    /**
     * delete mac address table
     * @param fdb:
     *     object: {type: xxx, port: xxx, ...}
     *     array: [{fdb object}, {fdb object}, ...]
     */
    $scope.deleteFdb = function(fdb) {
        /* param check */
        var fdbArr = [];
        if(_.isArray(fdb))        fdbArr = fdb; 
        else if(_.isObject(fdb))  fdbArr = [fdb]; 
        else return;

        /* delete */
        SecretApi({
            url: '/api/mac_address_table/',
            method: 'DELETE',
            data: {
                mac_address_table: fdbArr,
            },
        }, function() {
            var fdbKeyArr = _.map(fdbArr, function(item) {return item.key;});
            $scope.macAddressTable = _.filter($scope.macAddressTable, function(item) {
                if(fdbKeyArr.indexOf(item.key) != -1) { // delete
                    return false;
                }
                return true;
            });
            $scope._refresh();
            $scope.refreshTooltip();
        });
    };

    $scope.multiDelete = function() {
        var arr = [];

        _.each($scope.fdbDisp, function(item) {
            if($scope.multiSel[item.key]) {
                arr.push(item);
            }
        });

        $scope.deleteFdb(arr);
        $scope.load();
    };
	
    $scope.deleteAllFdb = function() {

        /* delete */
        SecretApi({
            url: '/api/mac_address_table/delete_all',
            method: 'DELETE',

        }, function() {

            $scope._refresh();
            $scope.refreshTooltip();
        });
    };
    
	$scope.deleteAll = function() {

        $scope.deleteAllFdb();
        $scope.load();
    };

    /**
     * add new mac address table
     */
    $scope.addNewFdb = function() {
        $scope.loading = true;

        var mac_address = $scope.newFdb.macAddr;
        var port = $scope.newFdb.port;
        var vlan_id = parseInt($scope.newFdb.vlan);

        SecretApi({
            url: '/api/mac_address_table/', 
            method: 'POST',
            data: {
                vlan_id: vlan_id,
                mac_address: mac_address,
                port: port,
            },
        }, function(data) {
            /* get index of equal vlan_id and mac_address */
            var index = -1;
            for(var i = 0; i < $scope.macAddressTable.length; i++) {
                var obj = $scope.macAddressTable[i]; 
                if(obj.mac_address == mac_address && parseInt(obj.vlan_id) == vlan_id) {
                    index = i;
                    break;
                }
            }

            var fdbTmp = {
                mac_address: mac_address,
                port: port,
                vlan_id: vlan_id,
                type: 'static',
                key: $scope.getKey(mac_address, port, vlan_id),
            };
            if(index == -1) $scope.macAddressTable.push(fdbTmp);
            else            $scope.macAddressTable[index] = fdbTmp;

            $scope._refresh();
            $scope.load();
        }, function() {
            $scope.loading = false;  
        });
    };

    $scope.isDeleteEnable = function() {
        return _.values($scope.multiSel).indexOf(true) != -1; 
    };



    $scope.getAllPort();
    $scope.load();
}]);
