
;Public.app.controller('ctlHomePortMgrStormCtl', ['$scope', 'SecretHttp', 'Alert', function($scope, SecretHttp, Alert) {

    $scope.if = [];
    $scope.setif = false;
    $scope.chk = false;
    $scope.chooseOne = false;
    $scope.selectedIf = {};
    $scope.newIf = {};
    $scope.type = '';
    
    $scope.chooseAll = function() {
        var len = $scope.if.length;

        chooseOne = false;
        $scope.setif = false;
        if($scope.chk == true){
            $scope.setif = true;
            for(i=0; i<len; i++){
                $scope.if[i].box = true;
            }
        }
        else{
            for(i=0; i<len; i++){
                $scope.if[i].box = false;
            }
        }}

    $scope.choose = function() {
        var len = $scope.if.length;

        $scope.setif = false;
        var j = 0;
        for(i=0; i<len; i++){
            if($scope.if[i].box == true){
                $scope.setif = true;
                $scope.selectedIf = $scope.if[i];
                j ++;
            }
        }
        if(j == 1){
            $scope.chooseOne = true;
        }else{
            $scope.chooseOne = false;
        }
    };

    $scope.un_mode_disabled = function(){
        $scope.newIf.ucast_mode = '0';
    }

    $scope.un_mode_pps = function(){
        $scope.newIf.ucast_mode = '1';
    }    
   
    $scope.un_mode_level = function(){
        $scope.newIf.ucast_mode = '2';
    }

    $scope.bc_mode_disabled = function(){
        $scope.newIf.bcast_mode = '0';
    }

    $scope.bc_mode_pps = function(){
        $scope.newIf.bcast_mode = '1';
    }    
   
    $scope.bc_mode_level = function(){
        $scope.newIf.bcast_mode = '2';
    }

    $scope.mc_mode_disabled = function(){
        $scope.newIf.mcast_mode = '0';
    }

    $scope.mc_mode_pps = function(){
        $scope.newIf.mcast_mode = '1';
    }    
   
    $scope.mc_mode_level = function(){
        $scope.newIf.mcast_mode = '2';
    }
    
 
    $scope.modifyIf = function(key){

        SecretHttp({
            url: '/api/storm_ctl/all/',
            method: 'PUT',
            data:{
                key : key,
                type: $scope.type,
                ucast_mode : $scope.newIf.ucast_mode,
                ucast_rate : $scope.newIf.ucast_rate,
                bcast_mode : $scope.newIf.bcast_mode,
                bcast_rate : $scope.newIf.bcast_rate,
                mcast_mode : $scope.newIf.mcast_mode,
                mcast_rate : $scope.newIf.mcast_rate,
            },
        })

            .then(function(resp) {
                if (resp.status == 200) {
                    if(resp.data['error']) {
                        Alert(resp.data['err_reason']);
                    }
                }
                else {
                    Alert('http server error!');
                }
            });
    }
    $scope.modifyUcastAll = function(){
        $scope.type = 'Ucast';
        var len = $scope.if.length;
        for(i=0; i<len; i++){
            if($scope.if[i].box == true){
                $scope.modifyIf($scope.if[i].key);
            }
        }
        $scope.newIf.ucast_mode = 0;
        $scope.newIf.ucast_rate = 0;
        
        $scope.load();
    };

    $scope.modifyBcastAll = function(){
        $scope.type = 'Bcast';
        var len = $scope.if.length;
        for(i=0; i<len; i++){
            if($scope.if[i].box == true){
                $scope.modifyIf($scope.if[i].key);
            }
        }
        $scope.newIf.bcast_mode = 0;
        $scope.newIf.bcast_rate = 0;
        $scope.load();
    };
    
    $scope.modifyMcastAll = function(){
        $scope.type = 'Mcast';
        var len = $scope.if.length;
        for(i=0; i<len; i++){
            if($scope.if[i].box == true){
                $scope.modifyIf($scope.if[i].key);
            }
        }
        $scope.newIf.mcast_mode = 0;
        $scope.newIf.mcast_rate = 0;
        $scope.load();
    };    
    $scope.getIf = function() {
        $scope.if = [];

        SecretHttp({
            url: '/api/storm_ctl/all/',
            method: 'GET',
        })
            .then(function(resp) {
                if(resp.status == 200) {
                    $scope.ifp = resp.data.arr;
                    var len = $scope.ifp.length;
                    for (i=0; i<len; i++){
                        if($scope.ifp[i].key.search('eth')>=0){
                            $scope.if.push($scope.ifp[i]);
                            $scope.if[i].box = false;
                            
                            if ($scope.if[i].storm_control.ucast_mode == '2') {
                              rate = Number($scope.if[i].storm_control.ucast_rate)
                              rate = rate / 100;
                              $scope.if[i].storm_control.ucast_rate = rate;
                              $scope.if[i].storm_control.ucast_rate += "%";                          
                            }
                            
                            if ($scope.if[i].storm_control.bcast_mode == '2') {

                              rate = Number($scope.if[i].storm_control.bcast_rate)
                              
                              rate = rate / 100;

                              $scope.if[i].storm_control.bcast_rate = rate;
                              $scope.if[i].storm_control.bcast_rate += "%";                          
                            }
                            
                            if ($scope.if[i].storm_control.mcast_mode == '2') {
                              rate = Number($scope.if[i].storm_control.mcast_rate)
                              rate = rate / 100;
                              $scope.if[i].storm_control.mcast_rate = rate;
                              $scope.if[i].storm_control.mcast_rate += "%";                          
                            } 
                            
                            /* storm control disable, we choose display '-' */

                            if ($scope.if[i].storm_control.ucast_mode == '0') {

                              $scope.if[i].storm_control.ucast_rate = "-";
                        
                            }
                            
                            if ($scope.if[i].storm_control.bcast_mode == '0') {
                              $scope.if[i].storm_control.bcast_rate = "-";                        
                            }
                            
                            if ($scope.if[i].storm_control.mcast_mode == '0') {
   
                              $scope.if[i].storm_control.mcast_rate = "-";
                                               
                            }                              
                        }
                    }

                }
                else {
                    Alert(resp.data['err_reason']);
                }
            });
    };
    
    $scope.ucastRateCheck = function() {

        if ($scope.newIf.ucast_mode == '1') {

            if (parseInt($scope.newIf.ucast_rate) >=0 && parseInt($scope.newIf.ucast_rate) <= 1000000000) {

              return true;
            }
        }
        else if ($scope.newIf.ucast_mode == '2'){
            rate = Number($scope.newIf.ucast_rate)

            if (Number($scope.newIf.ucast_rate) >=0.00 && Number($scope.newIf.ucast_rate) <= 99.99 && $scope.newIf.ucast_rate.length <= 5) {

              return true;        
            }
        }
        else if ($scope.newIf.ucast_mode == '0'){
            return true;       
        }        
        return false;      
    };

    $scope.bcastRateCheck = function() {

        if ($scope.newIf.ucast_mode == '1') {

            if (parseInt($scope.newIf.ucast_rate) >=0 && parseInt($scope.newIf.ucast_rate) <= 1000000000) {

              return true;
            }
        }
        else if ($scope.newIf.ucast_mode == '2'){
            rate = Number($scope.newIf.ucast_rate)

            if (Number($scope.newIf.ucast_rate) >=0.00 && Number($scope.newIf.ucast_rate) <= 99.99 && $scope.newIf.ucast_rate.length <= 5) {

              return true;        
            }
        }
        else if ($scope.newIf.ucast_mode == '0'){
            return true;       
        }           
        return false;      
    };

    $scope.mcastRateCheck = function() {

        if ($scope.newIf.ucast_mode == '1') {

            if (parseInt($scope.newIf.ucast_rate) >=0 && parseInt($scope.newIf.ucast_rate) <= 1000000000) {

              return true;
            }
        }
        else if ($scope.newIf.ucast_mode == '2'){
            rate = Number($scope.newIf.ucast_rate)

            if (Number($scope.newIf.ucast_rate) >=0.00 && Number($scope.newIf.ucast_rate) <= 99.99 && $scope.newIf.ucast_rate.length <= 5) {

              return true;        
            }
        }
        else if ($scope.newIf.ucast_mode == '0'){
            return true;       
        }           
        return false;      
    };
    
    $scope.load = function() {
        $scope.setif = false;
        
        $scope.getIf();
    };

    $scope.load();
}]);
