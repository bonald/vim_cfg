;Public.app.controller('ctlHomeSecuArpTable', ['$scope', '$filter', 'SecretApi', 'Alert', '$timeout', function($scope, $filter, SecretApi, Alert, $timeout) {

    
    /* AJAX */
    $scope.initLoading = true;
    $scope.arpTable = {};
    $scope.loading = false;
    $scope.gratuitousArpLearnState = false;
    $scope.delarp = false;

    /* user defined */
    $scope.addArpFlag = false; /* display add form when flag is true */
    $scope.newArp = {
        ipAddr: '',
        macAddr: '',
    };

    /* pagination */
    $scope.pgCurrent = 1;
    $scope.pgItemCnt = 0;
    $scope.pgItemNumEachPage = 10;

    /* filter */
    $scope.arpFilterStr = '';



    $scope.onPageChange = function() {
    	
    };



    /* functions */
    $scope.refreshTooltip = function() {
        $timeout(function() {
            $('[data-toggle="tooltip"]').tooltip({container: 'body'});
            $('.tooltip.fade.bottom.in').remove();
        }, 0);
    };

    $scope.addNewArp = function() {
        SecretApi({
            url: '/api/security/arp_table/',
            method: 'PUT',
            data: {
              ip_address: $scope.newArp.ipAddr,
              mac_address: $scope.newArp.macAddr,
            }
        }, function(data) {
            $scope.load();
        });
        $scope.newArp.ipAddr = '';
        $scope.newArp.macAddr = '';
    };
    
    $scope.enableGratuitousArp = function() {
        SecretApi({
            url: '/api/security/arp_table/gratuitous_arp/', 
        }, function(data) {
            //console.log(data);
            $scope.gratuitousArpLearnState = data.gratuitous_arp_learn_en == 0 ? false : true;
        });
    };
    
    
    $scope.changeEnableState = function() {
        var enable = ($scope.gratuitousArpLearnState == true) ? 0 : 1;
        //console.log(enable);
        SecretApi({
            url: '/api/security/arp_table/gratuitous_arp/',
            method: 'PUT',
            data : { 
              gratuitous_arp: enable,
            }
        }, function(data) {
            $scope.enableGratuitousArp();
            
        });
    };

    $scope.clear = function(){
        SecretApi({
            url: '/api/security/arp_table/',
            method: 'POST',
        }, function(data) {
            $scope.load();
        });
    };
    

    
    $scope.deleteArp = function(ipaddress) { 
        $scope.delarp = true;           
        SecretApi({
            url: '/api/security/arp_table/',
            method: 'DELETE',
            data: {
              ip_address: ipaddress,
            }
        }, function(data) {
            $scope.load();
        }
        , function()
        {
            $scope.delarp = false;
        });
    };
    

    $scope.load = function() {
        SecretApi({
            url: '/api/security/arp_table/', 
            method: 'GET',
        }, function(data) {

            $scope.arpTable = data.arp_table;
            $scope.pgItemCnt = $scope.arpTable.length;
            //console.log($scope.arpTable);
            $scope.enableGratuitousArp();

        });
        $scope.initLoading = false;
    };
    $scope.load();


}]);
