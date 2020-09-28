;Public.app.controller('ctlHomeSecuWormFilter', ['$scope', '$filter', 'SecretApi', 'Alert', '$timeout', function($scope, $filter, SecretApi, Alert, $timeout) {

    
    /* AJAX */
    $scope.initLoading = true;
    $scope.filterTable = {};
    $scope.loading = false;
    $scope.delload = false;

    /* user defined */
    $scope.addFilterFlag = false; /* display add form when flag is true */
    $scope.newFilter = {
        filtername: '',
        protocol: 'tcp',
        dstport: '',
        isrange: '',
    };

    /* pagination */
    $scope.pgCurrent = 1;
    $scope.pgItemCnt = 0;
    $scope.pgItemNumEachPage = 10;

    /* filter */
    $scope.wormFilterStr = '';



    $scope.onPageChange = function() {
    	
    };



    /* functions */
    $scope.refreshTooltip = function() {
        $timeout(function() {
            $('[data-toggle="tooltip"]').tooltip({container: 'body'});
            $('.tooltip.fade.bottom.in').remove();
        }, 0);
    };

    $scope.addNewFilter = function() { 
        SecretApi({
            url: '/api/security/worm_filter/',
            method: 'PUT',
            data: {
              filtername: $scope.newFilter.filtername,
              protocol: $scope.newFilter.protocol,
              dstport: $scope.newFilter.dstport,
              isrange: $scope.newFilter.isrange,
                  }
            }, function(data) {                                               
                $scope.load();                                                
            }, null, 
		    function() {                                             
                $scope.newFilter.filtername = '';                         
                $scope.newFilter.protocol = 'tcp';                        
                $scope.newFilter.dstport = '';                            
                $scope.newFilter.isrange = '';                            
        });  
    };
    
    $scope.btClick = function() {
        $scope.newFilter.filtername = 'bt';
        $scope.newFilter.protocol = 'tcp';
        $scope.newFilter.dstport = '6881';
        $scope.newFilter.isrange = '6889';
        $scope.addNewFilter();
    }; 
    
    $scope.thunderClick = function() {
        $scope.newFilter.filtername = 'thunder1';
        $scope.newFilter.protocol = 'tcp';
        $scope.newFilter.dstport = '3076';
        $scope.newFilter.isrange = '';
        $scope.addNewFilter();
        $scope.newFilter.filtername = 'thunder2';
        $scope.newFilter.protocol = 'tcp';
        $scope.newFilter.dstport = '3077';
        $scope.newFilter.isrange = '';
        $scope.addNewFilter();
        $scope.newFilter.filtername = 'thunder3';
        $scope.newFilter.protocol = 'tcp';
        $scope.newFilter.dstport = '5200';
        $scope.newFilter.isrange = '';
        $scope.addNewFilter();
        $scope.newFilter.filtername = 'thunder4';
        $scope.newFilter.protocol = 'tcp';
        $scope.newFilter.dstport = '6200';
        $scope.newFilter.isrange = '';
        $scope.addNewFilter();
    }; 
    
    $scope.eMuleClick = function() {
        $scope.newFilter.filtername = 'eMule1';
        $scope.newFilter.protocol = 'tcp';
        $scope.newFilter.dstport = '4662';
        $scope.newFilter.isrange = '';
        $scope.addNewFilter();
        $scope.newFilter.filtername = 'eMule2';
        $scope.newFilter.protocol = 'udp';
        $scope.newFilter.dstport = '4772';
        $scope.newFilter.isrange = '';
        $scope.addNewFilter();
    }; 

    $scope.NachiClick = function() {
        $scope.newFilter.filtername = 'NachiBlasterD';
        $scope.newFilter.protocol = 'tcp';
        $scope.newFilter.dstport = '707';
        $scope.newFilter.isrange = '';
        $scope.addNewFilter();
    }; 

    $scope.SasserClick = function() {
        $scope.newFilter.filtername = 'Sasser1';
        $scope.newFilter.protocol = 'tcp';
        $scope.newFilter.dstport = '5554';
        $scope.newFilter.isrange = '';
        $scope.addNewFilter();
        $scope.newFilter.filtername = 'Sasser2';
        $scope.newFilter.protocol = 'udp';
        $scope.newFilter.dstport = '9996';
        $scope.newFilter.isrange = '';
        $scope.addNewFilter();
    }; 
    
    

    $scope.SQLSlammerClick = function() {
        $scope.newFilter.filtername = 'SQLSla1';
        $scope.newFilter.protocol = 'tcp';
        $scope.newFilter.dstport = '1433';
        $scope.newFilter.isrange = '';
        $scope.addNewFilter();
        $scope.newFilter.filtername = 'SQLSla2';
        $scope.newFilter.protocol = 'udp';
        $scope.newFilter.dstport = '1433';
        $scope.newFilter.isrange = '';
        $scope.addNewFilter();
        $scope.newFilter.filtername = 'SQLSla3';
        $scope.newFilter.protocol = 'tcp';
        $scope.newFilter.dstport = '1434';
        $scope.newFilter.isrange = '';
        $scope.addNewFilter();
        $scope.newFilter.filtername = 'SQLSla4';
        $scope.newFilter.protocol = 'udp';
        $scope.newFilter.dstport = '1434';
        $scope.newFilter.isrange = '';
        $scope.addNewFilter();
    };     
    

    $scope.deleteFilter = function(name) {  
        $scope.delload = true;          
        SecretApi({
            url: '/api/security/worm_filter/',
            method: 'DELETE',
            data: {
              filtername: name,
            }
            
        }, function(data) {
            $scope.load();
        }, function()
        {
            $scope.delload = false;
        });
    };
    

    $scope.load = function() {
        SecretApi({
            url: '/api/security/worm_filter/', 
            method: 'GET',
        }, function(data) {

            $scope.filterTable = data.worm_filter;
            $scope.pgItemCnt = $scope.filterTable.length;
            //console.log($scope.filterTable);

        });
        $scope.initLoading = false;
    };
    $scope.load();


}]);
