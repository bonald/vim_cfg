;Public.app.controller('ctlHomeStatus', ['$scope', 'SecretHttp', 'Alert', '$interval', '$location', '$cookieStore', function($scope, SecretHttp, Alert, $interval, $location, $cookieStore) {

    $scope.cpu = [];
    $scope.memory = [];
    $scope.fan = [];
    $scope.psu = [];
    $scope.sensor = [];
    $scope.card = [];
    $scope.if = [];
    $scope.ifLine1 = [];
    $scope.ifLine2 = [];
    $scope.version = [];
    $scope.bootimage = [];
    $scope.sysglobal = [];
    $scope.uptime = [];
    $scope.refreshIntervalArr = [30, 60, 120, 300];
    var cookiesrefresh = $cookieStore.get("city");
    if (cookiesrefresh == null){
        $scope.refreshInterval = $scope.refreshIntervalArr[0];
    }
    else{
        $scope.refreshInterval = cookiesrefresh;
    }




    $scope._interval = null;
/*
    var updateData = setInterval(function(){
        $scope.getUptime();
        $scope.getCpu();
        $scope.getMemory();
        $scope.getIf();
    }, 5000);
*/
    $scope.getIf = function() {
        SecretHttp({
            url: '/api/interface/all/',
            method: 'GET',
        })
            .then(function (resp) {
                if (resp.status == 200) {
                    $scope.if = resp.data.arr;
                    var length = $scope.if.length;
                    var i =0;
                    var j =0;
                    var count1 = 0;
                    var count2 = 0;
                    $scope.ifLine1 = [];
                    $scope.ifLine2 = [];
                    $scope.ifLine1.subif = [];
                    $scope.ifLine2.subif = [];
                    for(i = 0; i < length; i++) {
                        if($scope.if[i].hw_type.search('Port')>=0)
                        {
                            if (j%2) {
                                if($scope.if[i].key.search('_')>=0){
                                    $scope.if[i].count = count1;
                                    //$scope.if[i].portnum = parseInt($scope.if[i].key.indexOf("_"));
                                    count1 += 4;
                                    $scope.ifLine2.push($scope.if[i]);
                                    $scope.if[i].portnum = 1;
                                    $scope.ifLine2.subif.push($scope.if[i]);
                                    $scope.if[i+1].portnum = 2;
                                    $scope.ifLine2.subif.push($scope.if[i + 1]);
                                    $scope.if[i+2].portnum = 3;
                                    $scope.ifLine2.subif.push($scope.if[i + 2]);
                                    $scope.if[i+3].portnum = 4;
                                    $scope.ifLine2.subif.push($scope.if[i + 3]);
                                    i = i+3;
                                }
                                else {
                                    var portnum = $scope.if[i].key.substring(6);
                                    $scope.if[i].portnum = parseInt(portnum);
                                    $scope.ifLine2.push($scope.if[i]);
                                }
                            }
                            else {
                                if($scope.if[i].key.search('_')>=0){
                                    $scope.if[i].count = count2;
                                    count2 += 4;
                                    $scope.ifLine1.push($scope.if[i]);
                                    $scope.if[i].portnum = 1;
                                    $scope.ifLine1.subif.push($scope.if[i]);
                                    $scope.if[i+1].portnum = 2;
                                    $scope.ifLine1.subif.push($scope.if[i + 1]);
                                    $scope.if[i+2].portnum = 3;
                                    $scope.ifLine1.subif.push($scope.if[i + 2]);
                                    $scope.if[i+3].portnum = 4;
                                    $scope.ifLine1.subif.push($scope.if[i + 3]);
                                    i = i+3;
                                }
                                else {
                                    var portnum = $scope.if[i].key.substring(6);
                                    $scope.if[i].portnum = parseInt(portnum);
                                    $scope.ifLine1.push($scope.if[i]);
                                }
                            }
                            j++;
                        }
                    }

                }
                else {
                    alert('error');
                }
            });
    };

    $scope.getCard = function() {
        SecretHttp({
            url: '/api/card/',
            method: 'GET',
        })
            .then(function(resp) {
                if(resp.status == 200) {
                    $scope.card = resp.data.arr;
                }
                else {
                    alert('error');
                }
            });
    };

        $scope.getVersion = function() {
            SecretHttp({
                url: '/api/version/',
                method: 'GET',
            })
                .then(function(resp) {
                    if(resp.status == 200) {
                        $scope.version = resp.data.arr;
                    }
                    else {
                        alert('error');
                    }
                });

        };

    $scope.getBootimage = function() {
        SecretHttp({
            url: '/api/boot/',
            method: 'GET',
        })
            .then(function(resp) {
                if(resp.status == 200) {
                    $scope.bootimage = resp.data.boot.current_image;
                }
                else {
                    alert('error');
                }
            });
    };

    $scope.getSysglobal = function() {
        SecretHttp({
            url: '/api/sysglobal/',
            method: 'GET',
        })
            .then(function(resp) {
                if(resp.status == 200) {
                    $scope.sysglobal = resp.data.arr;
                    var x=+resp.data.arr[0].http_timeout;
                    var y=x/60;
                    $scope.http_timeout = y;
                }
                else {
                    alert('error');
                }
            });
    };

    $scope.getUptime = function() {
        SecretHttp({
            url: '/api/uptime/',
            method: 'GET',
        })
            .then(function(resp) {
                if(resp.status == 200) {
                    $scope.uptime = resp.data.arr;
                }
                else {
                    alert('error');
                }
            });
    };

    $scope.getMemory = function() {
        SecretHttp({
            url: '/api/memory/',
            method: 'GET',
        })
            .then(function(resp) {
                if(resp.status == 200) {
                    $scope.memory = resp.data.arr;
                }
                else {
                    alert('error');
                }
            });
    };

    $scope.getCpu = function() {
        SecretHttp({
            url: '/api/cpu/',
            method: 'GET',
        })
            .then(function(resp) {
                if(resp.status == 200) {
                    $scope.cpu = resp.data.arr;
                }
                else {
                    alert('error');
                }
            });
    };

    $scope.getFan = function() {
        SecretHttp({
            url: '/api/fan/',
            method: 'GET',
        })
            .then(function(resp) {
                if(resp.status == 200) {
                    $scope.fan = resp.data.arr;
                }
                else {
                    alert('error');
                }
            });
    };

    $scope.getPsu = function() {
        SecretHttp({
            url: '/api/psu/',
            method: 'GET',
        })
            .then(function(resp) {
                if(resp.status == 200) {
                    $scope.psu = resp.data.arr;
                }
                else {
                    alert('error');
                }
            });

    };

    $scope.getSensor = function() {
        SecretHttp({
            url: '/api/sensor/',
            method: 'GET',
        })
            .then(function(resp) {
                if(resp.status == 200) {
                    $scope.sensor = resp.data.arr;
                }
                else {
                    alert('error');
                }
            });

    };

    $scope.timeoutchange = function() {
        SecretHttp({
            url: '/api/sysglobal/',
            method: 'PUT',
        })
            .then(function(resp) {
                if(resp.status == 200) {
                    $scope.sysglobal = resp.data.arr;
                    var x=+resp.data.arr[0].http_timeout;
                    var y=x/60;
                    $scope.http_timeout = y;
                }
                else {
                    alert('error');
                }
            });

    };

    $scope.load = function() {
        $scope.getCpu();
        $scope.getCard();
        $scope.getIf();
        $scope.getVersion();
        $scope.getSysglobal();
        $scope.getBootimage();
        $scope.getUptime();
        $scope.getMemory();
        $scope.getFan();
        $scope.getPsu();
        $scope.getSensor();
    };


    $scope.refresh = function() {
        $scope._interval = $interval(function() {
            if($location.path() != '/home/index') {
                $interval.cancel($scope._interval);
                return;
            };
            $scope.load();
        }, $scope.refreshInterval * 1000);
    };

    $scope.selectchange = function(){

        $cookieStore.remove('city');
        $cookieStore.put('city', $scope.refreshInterval);

    };

    $scope.$watch('refreshInterval', function(value) {
        $interval.cancel($scope._interval);
        $scope.refresh();
    });

    $scope.load();
    $scope.refresh();
}]);
