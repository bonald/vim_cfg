;Public.app.controller('ctlHomeSysMgrTimeMgr', ['$scope', '$filter','SecretApi', 'Alert', '$rootScope', function($scope, $filter, SecretApi, Alert, $rootScope) {

    $scope.ntpSrvs = [];
    $scope.currentSysTime = '';
    $scope.clockArr       = ['Hour', 'Minute', 'Second', 'Year', 'Month', 'Day'];

    $scope.timeStampSyncEnable = false;
    $scope.srvVrfsel = 'mgmt';
    $scope.ntpServerAddr = '';
    $scope.lastSyncTime = '';
  
    $scope.logLevel = [
 '(UTC+00:00)Etc/GMT,Europe/London','(UTC+01:00)Europe/Paris,Berlin,Africa/Lagos','(UTC+02:00)Africa/Cairo,Asia/Jerusalem','(UTC+03:00)Asia/Baghdad,Asia/Tehran','(UTC+04:00)Europe/Moscow,Asia/Dubai','(UTC+08:00)Asia/Shanghai,Singapore,Australia/Perth' ];

    if ($rootScope.System_product == 'Tap-350' || $rootScope.System_product == 'Tap-580'
        || $rootScope.System_product == 'Tap-550' || $rootScope.System_product == 'Tap-530') {
        $scope.vrfIds= ['mgmt'];
    } else {
       $scope.vrfIds= ['mgmt', 'default'];
    }
 
    $scope.getSysCurrentTime = function() {
        /* get */
        SecretApi({
            url: '/api/time/current_time/',
            method: 'GET',
        }, function(data) {
            $scope.currentSysTime = data.cfg;
            $scope.clockArr[0]    = data.cfg.substring(0,2);
            $scope.clockArr[1]    = data.cfg.substring(3,5);
            $scope.clockArr[2]    = data.cfg.substring(6,8);
            $scope.clockArr[3]    = data.cfg.substring(20,24);
            $scope.clockArr[5]    = data.cfg.substring(17,19);
            if (data.cfg.substring(13,16) == "Jan") {
                $scope.clockArr[4] = 01;
            }
            if (data.cfg.substring(13,16) == "Feb") {
                $scope.clockArr[4] = 02;
            }
            if (data.cfg.substring(13,16) == "Mar") {
                $scope.clockArr[4] = 03;
            }
            if (data.cfg.substring(13,16) == "Apr") {
                $scope.clockArr[4] = 04;
            }
            if (data.cfg.substring(13,16) == "May") {
                $scope.clockArr[4] = 05;
            }
            if (data.cfg.substring(13,16) == "Jun") {
                $scope.clockArr[4] = 06;
            }
            if (data.cfg.substring(13,16) == "Jul") {
                $scope.clockArr[4] = 07;
            }
            if (data.cfg.substring(13,16) == "Aug") {
                $scope.clockArr[4] = 08;
            }
            if (data.cfg.substring(13,16) == "Sep") {
                $scope.clockArr[4] = 09;
            }
            if (data.cfg.substring(13,16) == "Oct") {
                $scope.clockArr[4] = 10;
            }
            if (data.cfg.substring(13,16) == "Nov") {
                $scope.clockArr[4] = 11;
            }
            if (data.cfg.substring(13,16) == "Dec") {
                $scope.clockArr[4] = 12;
            }
            $scope.load();  
        });
    };

    $scope.setSysCurrentTime = function(clockArr) {
        /* set */
        SecretApi({
            url: '/api/time/current_time/',
            method: 'PUT',
            data: {
                Year: clockArr[3],
                Month: clockArr[4],
                Day: clockArr[5],
                Hour: clockArr[0],
                Minute: clockArr[1],
                Second: clockArr[2],
            },            
        }, function(data) {
            $scope.getSysCurrentTime();
            $scope.load();
        });
    };
    
    $scope.deleteNtpServer = function(srv) {

        /* param check */
        /* delete */
        SecretApi({
            url: '/api/time/ntp_server/',
            method: 'DELETE',
            data: {
                ntp_srv_table: srv,
            },
        }, function() {

            $scope.load();
        });
    };

    $scope.addNtpServer = function() {
        /* add */
        SecretApi({
            url: '/api/time/ntp_server/',
            method: 'PUT',
            data: {
                ntp_srv: $scope.ntpServerAddr,
                inband: $scope.srvVrfsel,
            },
        }, function() {

            $scope.load();  
        });
    };

    /***********************************************************/    
    /******************* TimeStamp Sync ***********************/   
    /***********************************************************/  
    $scope.setTimeStampSyncEnable = function(timeStampSyncEnable) {
        $scope.timeStampSyncEnable = ! $scope.timeStampSyncEnable;

        SecretApi({
            url: '/api/time/timestap_sync/',
            method: 'PUT',
            data: {
                enable: $scope.timeStampSyncEnable ,
            },
        }, function() {

        });   

        $scope.getTimeStampSync();     
    };
    
    $scope.getTimeStampSync = function() {
        SecretApi({
            url: '/api/time/timestap_sync/',
            method: 'GET',
        }, function(data) {
            $scope.lastSyncTime = (data.last.length) ? data.last : '';
            $scope.timeStampSyncEnable = (data.type == "Disabled") ? false : true;
        });  

        $scope.load();  
    };

    /***********************************************************/    
    /******************* Load **********************************/   
    /***********************************************************/  
    $scope.load = function() {
        SecretApi({
            url: '/api/time/ntp_server/',
            method: 'GET',
        }, function(data) {

            $scope.ntpSrvs = data.cfg;     
        });   

    };

    $scope.load();     
    $scope.getSysCurrentTime();
    $scope.getTimeStampSync();     
}]);
