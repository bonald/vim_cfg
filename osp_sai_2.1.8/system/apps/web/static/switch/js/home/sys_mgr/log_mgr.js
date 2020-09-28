;Public.app.controller('ctlHomeSysMgrLogMgr', ['$scope', '$filter', 'SecretApi', 'Alert', '$rootScope', function($scope, $filter, SecretApi, Alert, $rootScope) {

    $scope.syslogs = [];
    $scope.displaylogs = [];
    $scope.display = [];
    $scope.logFiltered = [];
    $scope.searchFlag = true;
    $scope.logConfiguration = 'active';
    $scope.searchClass1 = '';
    $scope.searchClass2 = '';
    $scope.severitySel = 'all';
    $scope.moduleSel = 'all';
    $scope.keyword = '';
    $scope.navTabs = 'log_conf'; // 'log_conf' or 'search_module' or 'search_word'....
    $scope.sysLogLevelSel = 'warning';
    $scope.cacheLogLevelSel = 'debug';
    $scope.severityLogLevelSel = 'warning';
    $scope.timeLogLevelSel = 'bsd';
    $scope.srvVrfsel = 'mgmt';
    $scope.enableLogServer = false;
    $scope.enableLogCache = false;
    $scope.enableLogMerge = false;
    $scope.configure = [];
    $scope.logSrvDict = [];
    $scope.initLoading = true;

    /* pagination */
    $scope.pgCurrent = 1;
    $scope.pgItemCnt = 0;
    $scope.pgItemNumEachPage = 10;	

    $scope.severity = ['0','1','2', '3', '4', '5', '6', '7', 'all'];
    $scope.logLevel = ['emergency','alert','critical', 'error', 'warning', 'notice', 'information', 'debug'];

    if ($rootScope.System_product == 'Tap-350' || $rootScope.System_product == 'Tap-580'
        || $rootScope.System_product == 'Tap-550' || $rootScope.System_product == 'Tap-530') {
        $scope.module = ['AAA', 'LOGGING', 'APP', 'FAN',  'INTERFACE', 'LAG', 'NTP', 'POWER', 'PSU',  'SYSTEM', 'TEMPERATURE', 'TRANSCEIVER','SSHD', 'all'];
    } else {
        $scope.module = ['AAA', 'APP','ARP', 'DHCP', 'DOT1X', 'FAN', 'FDB', 'INTERFACE', 'LACP', 'LAG', 'LLDP', 'MLAG',
        'NTP', 'POWER', 'PSU', 'ROUTE', 'SECURITY', 'STP', 'SYSTEM', 'TEMPERATURE', 'TRANSCEIVER','SSHD', 'all'];
    }

    $scope.timeLevel = ['date','bsd','iso', 'none'];

    if ($rootScope.System_product == 'Tap-350' || $rootScope.System_product == 'Tap-580'
        || $rootScope.System_product == 'Tap-550' || $rootScope.System_product == 'Tap-530') {
        $scope.vrfIds= ['mgmt'];
    } else {
        $scope.vrfIds= ['mgmt', 'default'];
    }

	$scope.onPageChange = function() {
        $scope._refresh('pageOnly');
    };
	
	$scope._refresh = function(state) {

        if(state != 'pageOnly') {
			$scope.logFiltered = $filter('orderBy')($scope.displaylogs.log_arr, '-time');
            $scope.pgItemCnt = $scope.logFiltered.length;

            if(_.isNumber(state)) {
                $scope.pgCurrent = state;
            }
        }

        $scope.display = $filter('limitTo')($scope.logFiltered, $scope.pgItemNumEachPage, ($scope.pgCurrent - 1) * $scope.pgItemNumEachPage);
    };
		
    $scope.severityChange = function(sel) {
        var tmp = [];
        $scope.severitySel = sel;
        angular.forEach($scope.air, function(data, index, array){

            if((data.severity == $scope.severitySel || $scope.severitySel == 'all' )&&
                (data.module == $scope.moduleSel || $scope.moduleSel == "all")){
                $scope.hello++;
                tmp.push(data);
            }
        });
        $scope.displaylogs.log_arr = tmp;
        $scope.pgCurrent = 1;
        $scope._refresh();
    }

    $scope.moduleChange = function(sel) {
        var tmp = [];
        $scope.moduleSel = sel;
        angular.forEach($scope.air, function(data, index, array){
            if((data.severity == $scope.severitySel || $scope.severitySel == 'all' )&&
                (data.module == $scope.moduleSel || $scope.moduleSel == "all")){
                tmp.push(data);
            }
        });
        $scope.displaylogs.log_arr = tmp;
        $scope.pgCurrent = 1;
        $scope._refresh();
    }

    $scope.keywordChange = function(sel) {
        var tmp = [];
        $scope.keyword = sel;
        angular.forEach($scope.air, function(data, index, array) {
            index = data.log.search($scope.keyword);
            if(index != -1) {
                tmp.push(data);
            }
        });
        $scope.displaylogs.log_arr = tmp;
        $scope.pgCurrent = 1;
        $scope._refresh();
    }

    $scope.changeLogServerStatus = function() {

        SecretApi({
            url: '/api/log_mgr/update_log_status/',
            method: 'PUT',
            data:{
                type : "server",
                status : ($scope.enableLogServer == true) ? 0 : 1,

            },
        }, function(data) {
            $scope.loadCfg();
        });
    };

    $scope.changeLogCacheStatus = function() {

        SecretApi({
            url: '/api/log_mgr/update_log_status/',
            method: 'PUT',
            data:{
                type : "cache",
                status : ($scope.enableLogCache == true) ? 0 : 1,
            },
        }, function(data) {
            $scope.loadCfg();
        });
    };

    $scope.changeLogMergeStatus = function() {

        SecretApi({
            url: '/api/log_mgr/update_log_status/',
            method: 'PUT',
            data:{
                type : "merge",
                status : ($scope.enableLogMerge == true) ? 0 : 1,
            },
        }, function(data) {
            $scope.loadCfg();
        });
    };
    
    $scope.sysLogLevelChange = function(sel) {

        SecretApi({
            url: '/api/log_mgr/update_log_level/',
            method: 'PUT',
            data:{
                type : "system",
                level : $scope.sysLogLevelSel,

            },
        }, function(data) {
            $scope.loadCfg();
        });
    };

    $scope.severityLogLevelChange = function(sel) {

        SecretApi({
            url: '/api/log_mgr/update_log_level/',
            method: 'PUT',
            data:{
                type : "severity",
                level : $scope.severityLogLevelSel,

            },
        }, function(data) {
            $scope.loadCfg();
        });
    };

    $scope.cacheLogLevelChange = function(sel) {

        SecretApi({
            url: '/api/log_mgr/update_log_level/',
            method: 'PUT',
            data:{
                type : "cache",
                level : $scope.cacheLogLevelSel,

            },
        }, function(data) {
            $scope.loadCfg();
        });
    };

    $scope.timeLogLevelChange = function(sel) {

        SecretApi({
            url: '/api/log_mgr/update_log_level/',
            method: 'PUT',
            data:{
                type : "time",
                level : $scope.timeLogLevelSel,

            },
        }, function(data) {
            $scope.loadCfg();
        });
    };

    $scope.updateLogBufferSize = function(size) {

        SecretApi({
            url: '/api/log_mgr/update_log_buffer_size/',
            method: 'PUT',
            data:{
                size : $scope.logBufferSize,
            },
        }, function(data) {
           $scope.loadCfg();
        });
    }; 

    $scope.addLogserver = function() {

        SecretApi({
            url: '/api/log_mgr/server_addr_table/',
            method: 'PUT',
            data:{
                serveraddr : $scope.serverAddr,
                inband : $scope.srvVrfsel,
            },
        }, function(data) {
            $scope.loadCfg();
        });
    }; 

    $scope.delLogserver = function(ip, vrf) {

        SecretApi({
            url: '/api/log_mgr/server_addr_table/',
            method: 'DELETE',
            data:{
                serveraddr : ip,
                vrf : (vrf=='0')?"default":"mgmt",
            },
        }, function(data) {
            $scope.loadCfg();
        });
    }; 

    $scope.flush = function() {
        SecretApi({
            url: '/api/log_mgr/',
            method: 'DELETE',
            data: {
                '0': '0',
            },
        })
        $scope.loadCfg();
        $scope.load();
    };

    $scope.load = function() {

        SecretApi({
            url: '/api/log_mgr/',
            method: 'GET',
        }, function(data) {
            $scope.display=data.log_arr;
            $scope.air=data.log_arr;
            if(resp.status == 200) {
                $scope.syslogs = _.clone(resp.data);
                if($scope.searchFlag == true){
                    $scope.severityChange($scope.severitySel);
                }
                else {
                    $scope.keywordChange($scope.keyword);
                }
                $scope._refresh(1);

            }
            else {
                alert('error');
            }
        });
    };

    $scope.loadCfg = function() {
        $scope.initLoading = true;
        SecretApi({
            url: '/api/log_mgr/get_cfg',
            method: 'GET',
        }, function(data) {
            $scope.configure = data.cfg;
            $scope.enableLogServer = ($scope.configure.enable_to_server == 1) ? true : false;
            $scope.enableLogCache = ($scope.configure.enable_to_file == 1) ? true : false;
            $scope.enableLogMerge = ($scope.configure.enable_merge == 1) ? true : false;
            $scope.sysLogLevelSel = $scope.configure.file_severity;
            $scope.cacheLogLevelSel = $scope.configure.module_severity;
            $scope.timeLogLevelSel = $scope.configure.timestamp;
            $scope.severityLogLevelSel = $scope.configure.server_severity;
            $scope.logSrvDict = $scope.configure.server_addr;
        }, function() {
            $scope.initLoading = false;
        })
    };

    $scope.loadCfg();    
}]);
