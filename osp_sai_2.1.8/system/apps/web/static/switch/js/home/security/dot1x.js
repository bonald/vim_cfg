
;Public.app.controller('ctlHomeDot1x', ['$scope', 'SecretApi', 'Alert', 'Base64', function($scope, SecretApi, Alert, Base64) {

    /* AJAX */
    $scope.auth_ctl_state = false;

    $scope.re_active_radius_all = false;
    $scope.re_active_tacacs_all = false;
    $scope.chk = false;
    
    $scope.interface = [];    
    $scope.MultiSelIf = []; /* for add snooping trust port */
    $scope.multiSelIf_auth = []; /* for add snooping trust port */    
    $scope.multiSelIf_clear = []; /* for add snooping trust port */        
    
    $scope.radius = {};
    $scope.dot1x_port = [];
    $scope.new_dot1x_port = {};  

    $scope.port_control = 
    {
       iface: "",
       enable: ""
    }

    $scope.enable = 
    [
      "Auto",
      "Force-unauthorized",
      "Force-authorized",
      "Disable"      
    ];
    
    /* user defines */


    /* functions */

    $scope.enableAuthCtl = function() {
        SecretApi({
            url: '/api/dot1x/load_Auth_Ctl/', 
        }, function(data) {
            $scope.auth_ctl_state = data.dhcp_snooping_enable;
            $scope.auth_ctl_state = (data.dhcp_snooping_enable == true)?  1 : 0;
        });
    };

    $scope.changeAuthCtlState = function() {    
    
        var auth_enable = parseInt($scope.auth_ctl_state) ? 0 : 1;

        $scope.display_detail = false;
        if($scope.auth_ctl_state) $scope.display_detail = true;
        
        SecretApi({
            url: '/api/dot1x/load_Auth_Ctl_set/'+auth_enable+'/',
            method: 'PUT',
        }, function() {
            $scope.enableAuthCtl();  
        });
    }    

    $scope.reActiveRadiusServer = function() {
        
        SecretApi({
            url: '/api/dot1x/re_active_radius_server/',
            method: 'PUT',
        }, function() {
        /*    $scope.load();  */
        });
    }    
    $scope.reActiveTacacsServer = function() {
        var enable = parseInt($scope.auth_ctl_state) ? 0 : 1;

        $scope.display_detail = false;
        if($scope.auth_ctl_state) $scope.display_detail = true;
        
        SecretApi({
            url: '/api/dot1x/re_active_tacacs_server/',
            method: 'PUT',
        }, function() {
        /*    $scope.enableAuthCtl(); */
        });
    }    
    
    $scope.initPort = function() {
        var ifs = [];
        

        for(var i = 0; i < $scope.MultiSelIf.length; i++) {

            if($scope.MultiSelIf[i]) ifs.push($scope.interface[i].key);
        }        
         
        SecretApi({
            url: '/api/dot1x/init_port/',
            method: 'PUT',
            data: {
                if_arr: ifs,   
            },
        }, function(data) {
            $scope.DisplayInitInterface = false;
            $scope.load(); 
        });
    };   

    $scope.reAuthPort = function() {
        var ifs = [];
        
        for(var i = 0; i < $scope.multiSelIf_auth.length; i++) {
            if($scope.multiSelIf_auth[i]) ifs.push($scope.interface[i].key);
        }        
         
        SecretApi({
            url: '/api/dot1x/re_auth_port/',
            method: 'PUT',
            data: {
                if_arr: ifs,   
            },
        }, function(data) {
            $scope.DisplayInitInterface = false;
            $scope.load(); 
        });
    };     

    $scope.clearPort = function() {
        var ifs = [];
        
        for(var i = 0; i < $scope.multiSelIf_clear.length; i++) {
            if($scope.multiSelIf_clear[i]) ifs.push($scope.interface[i].key);
        }        
         
        SecretApi({
            url: '/api/dot1x/clear_port/',
            method: 'PUT',
            data: {
                if_arr: ifs,   
            },
        }, function(data) {
            $scope.DisplayInitInterface = false;
            $scope.load(); 
        });
    };     


    $scope.dot1xRadiusDeadtime = function() {

        SecretApi({
            url: '/api/dot1x/dot1xRadiusDeadtime/'+$scope.radius.deadtime+'/',
            method: 'PUT',
        }, function(data) {
            $scope.loadChannelGroup();
            $scope.addChannelGroupShow = false;       
            $scope.load();
        });
        
        $scope.radius.deadtime = '';
        $scope.load();
    };

    $scope.dot1xRadiusRetransmit = function() {
        retransmit = $scope.radius.retransmit;


        SecretApi({
            url: '/api/dot1x/dot1xRadiusRetransmit/'+$scope.radius.retransmit+'/',
            method: 'PUT',
        }, function(data) {
            $scope.loadChannelGroup();
            $scope.addChannelGroupShow = false;
        });
        $scope.radius.retransmit = '';
        $scope.load();
    };

    $scope.dot1xRadiusTimeout = function() {

        SecretApi({
            url: '/api/dot1x/dot1xRadiusTimeout/'+$scope.radius.timeout+'/',
            method: 'PUT',
        }, function(data) {
            $scope.loadChannelGroup();
            $scope.addChannelGroupShow = false;
        });
        $scope.radius.timeout = '';
        $scope.load();        
    };

    $scope.dot1xRadiusKey = function() {

        SecretApi({
            url: '/api/dot1x/dot1xRadiusKey/'+Base64.encodeString($scope.radius.key)+'/',
            method: 'PUT',
        }, function(data) {
            $scope.loadChannelGroup();
            $scope.addChannelGroupShow = false;
        });
    };

    $scope.chooseAll = function() {
        var len = $scope.dot1x_port.length;

        chooseOne = false;
        $scope.setif = false;
        if($scope.chk == true){
            $scope.setif = true;
            for(i=0; i<len; i++){
                $scope.dot1x_port[i].box = true;
            }
        }
        else{
            for(i=0; i<len; i++){
                $scope.dot1x_port[i].box = false;
            }
        }}

    $scope.choose = function() {
        var len = $scope.dot1x_port.length;

        $scope.setif = false;
        var j = 0;
        for(i=0; i<len; i++){
            if($scope.dot1x_port[i].box == true){
                $scope.setif = true;
                $scope.selectedIf = $scope.dot1x_port[i];
                j ++;
            }
        }
        if(j == 1){
            $scope.chooseOne = true;
        }else{
            $scope.chooseOne = false;
        }
    };

    $scope.loadChannelGroup = function() {
      /* extend */
    };   
/****************** dot1x port configure function START ************************/
/* port-control */   
    $scope.auto = function(){

            $scope.new_dot1x_port.state = '1';
      
    }

    $scope.unauth = function(){

            $scope.new_dot1x_port.state = '2';
     
    }

    $scope.auth = function(){

            $scope.new_dot1x_port.state = '3';
     
    }

    $scope.portControlOne = function(name){
        
         SecretApi({
            url: '/api/dot1x/port_control/'+name+'/'+$scope.new_dot1x_port.state+'/',
            method: 'PUT',
        }, function() {
            /* $scope.loadLoadBalance(); */
        });
        
        return false;
    }    
    
    $scope.portControlAll = function(){

        var len = $scope.dot1x_port.length;
        for(i=0; i<len; i++){
            if($scope.dot1x_port[i].box == true){
                $scope.portControlOne($scope.dot1x_port[i].name);
            }
        }   
        $scope.load();          
    }

    $scope.globalPortControlSet = function() {

        iface = $scope.port_control.iface.key,
        enable = $scope.port_control.enable,    
     
        SecretApi({
            url: '/api/dot1x/port_control/'+iface+'/'+enable+'/',
            method: 'PUT',
        }, function(data) {


        });
               
        $scope.port_control.iface = '';    
        $scope.port_control.enable = '';  

        $scope.load();

    };    
    $scope.in = function(){

            $scope.new_dot1x_port.direction = '1';
     
    }

    $scope.both = function(){

            $scope.new_dot1x_port.direction = '0';
     
    }

    $scope.portDirectionOne = function(name){
        
         SecretApi({
            url: '/api/dot1x/port_direction/'+name+'/'+$scope.new_dot1x_port.direction+'/',
            method: 'PUT',
        }, function() {
            /* $scope.loadLoadBalance(); */
        });
        
        return false;
    }    
    
    $scope.portDirectionAll = function(){

        var len = $scope.dot1x_port.length;
        for(i=0; i<len; i++){
            if($scope.dot1x_port[i].box == true){
                $scope.portDirectionOne($scope.dot1x_port[i].name);
            }
        }   
        $scope.load();          
    }    
/* version */    
    $scope.version1 = function(){

            $scope.new_dot1x_port.version = '1';
      
    }

    $scope.version2 = function(){

            $scope.new_dot1x_port.version = '2';
     
    }

    $scope.versionOne = function(name){
        
         SecretApi({
            url: '/api/dot1x/version/'+name+'/'+$scope.new_dot1x_port.version+'/',
            method: 'PUT',
        }, function() {
            /* $scope.loadLoadBalance(); */
        });
        
        return false;
    }    
    
    $scope.versionAll = function(){

        var len = $scope.dot1x_port.length;
        for(i=0; i<len; i++){
            if($scope.dot1x_port[i].box == true){
                $scope.versionOne($scope.dot1x_port[i].name);
            }
        }    
        $scope.load();          
    }


/* re-authentication */    
    $scope.reAuth_on = function(){

            $scope.new_dot1x_port.reAuth_on = '1';      
    }

    $scope.reAuth_off = function(){

            $scope.new_dot1x_port.reAuth_on = '0';
     
    }

    $scope.reAuthenticationOne = function(name){
        
         SecretApi({
            url: '/api/dot1x/reAuthentication/'+name+'/'+$scope.new_dot1x_port.reAuth_on+'/',
            method: 'PUT',
        }, function() {
            /* $scope.loadLoadBalance(); */
        });
        
        return false;
    }    
    
    $scope.reAuthenticationAll = function(){

        var len = $scope.dot1x_port.length;
        for(i=0; i<len; i++){
            if($scope.dot1x_port[i].box == true){
                $scope.reAuthenticationOne($scope.dot1x_port[i].name);
            }
        }  
        $scope.load();          
    }   

    $scope.bypass_en = function(){

            $scope.new_dot1x_port.bypass_en = '1';
    }

    $scope.bypass_off = function(){

            $scope.new_dot1x_port.bypass_en = '0';
     
    }
 
    $scope.port_mode_mac = function(){

            $scope.new_dot1x_port.port_mode = '1';
    }

    $scope.port_mode_port = function(){

            $scope.new_dot1x_port.port_mode = '2';
     
    }
    
    $scope.macAuthBypassOne = function(name){
        
         SecretApi({
            url: '/api/dot1x/macAuthBypassOne/'+name+'/'+$scope.new_dot1x_port.bypass_en+'/',
            method: 'PUT',
        }, function() {
            /* $scope.loadLoadBalance(); */
        });
        
        return false;
    }    
    
    $scope.macAuthBypassAll = function(){

        var len = $scope.dot1x_port.length;
        for(i=0; i<len; i++){
            if($scope.dot1x_port[i].box == true){
                $scope.macAuthBypassOne($scope.dot1x_port[i].name);
            }
        }    
        $scope.load();         
    }    

    $scope.maxReqOne = function(name){
        
         SecretApi({
            url: '/api/dot1x/maxreq/'+name+'/'+$scope.new_dot1x_port.req+'/',
            method: 'PUT',
        }, function() {
            /* $scope.loadLoadBalance(); */
        });
        
        return false;
    }    
    
    $scope.maxReqAll = function(){

        var len = $scope.dot1x_port.length;
        for(i=0; i<len; i++){
            if($scope.dot1x_port[i].box == true){
                $scope.maxReqOne($scope.dot1x_port[i].name);
            }
        }     
        $scope.load(); 
    }  

    $scope.txPeriodOne = function(name){
        
         SecretApi({
            url: '/api/dot1x/txperiod/'+name+'/'+$scope.new_dot1x_port.period+'/',
            method: 'PUT',
        }, function() {
            /* $scope.loadLoadBalance(); */
        });
        
        return false;
    }    
    
    $scope.txPeriodAll = function(){

        var len = $scope.dot1x_port.length;
        for(i=0; i<len; i++){
            if($scope.dot1x_port[i].box == true){
                $scope.txPeriodOne($scope.dot1x_port[i].name);
            }
        }    
        $scope.load();         
    }  

    $scope.quietPeriodOne = function(name){
        
         SecretApi({
            url: '/api/dot1x/quietPeriod/'+name+'/'+$scope.new_dot1x_port.quiet_period+'/',
            method: 'PUT',
        }, function() {
            /* $scope.loadLoadBalance(); */
        });
        
        return false;
    }    
    
    $scope.quietPeriodAll = function(){

        var len = $scope.dot1x_port.length;
        for(i=0; i<len; i++){
            if($scope.dot1x_port[i].box == true){
                $scope.quietPeriodOne($scope.dot1x_port[i].name);
            }
        }    
        $scope.load();         
    } 
    
    $scope.reAuthperiodOne = function(name){
        
         SecretApi({
            url: '/api/dot1x/reAuthperiod/'+name+'/'+$scope.new_dot1x_port.re_authperiod+'/',
            method: 'PUT',
        }, function() {
            /* $scope.loadLoadBalance(); */
        });
        
        return false;
    }    
    
    $scope.reAuthperiodAll = function(){

        var len = $scope.dot1x_port.length;
        for(i=0; i<len; i++){
            if($scope.dot1x_port[i].box == true){
                $scope.reAuthperiodOne($scope.dot1x_port[i].name);
            }
        }   
        $scope.load();         
    }      

    $scope.suppTimeoutOne = function(name){
        
         SecretApi({
            url: '/api/dot1x/supptimeoutOne/'+name+'/'+$scope.new_dot1x_port.supp_timeout+'/',
            method: 'PUT',
        }, function() {
            /* $scope.loadLoadBalance(); */
        });
        
        return false;
    }    
    
    $scope.suppTimeoutAll = function(){

        var len = $scope.dot1x_port.length;
        for(i=0; i<len; i++){
            if($scope.dot1x_port[i].box == true){
                $scope.suppTimeoutOne($scope.dot1x_port[i].name);
            }
        }   
        $scope.load();         
    }  

    $scope.serverTimeoutOne = function(name){
        
         SecretApi({
            url: '/api/dot1x/serverTimeoutOne/'+name+'/'+$scope.new_dot1x_port.server_timeout+'/',
            method: 'PUT',
        }, function() {
            /* $scope.loadLoadBalance(); */
        });
        
        return false;
    }    
    
    $scope.serverTimeoutAll = function(){

        var len = $scope.dot1x_port.length;
        for(i=0; i<len; i++){
            if($scope.dot1x_port[i].box == true){
                $scope.serverTimeoutOne($scope.dot1x_port[i].name);
            }
        }     
        $scope.load();         
    }      

    $scope.guestVlanOne = function(name){
        
         SecretApi({
            url: '/api/dot1x/guestVlanOne/'+name+'/'+$scope.new_dot1x_port.guest_vlan+'/',
            method: 'PUT',
        }, function() {
            /* $scope.loadLoadBalance(); */
        });
        
        return false;
    }    
    
    $scope.guestVlanAll = function(){

        var len = $scope.dot1x_port.length;
        for(i=0; i<len; i++){
            if($scope.dot1x_port[i].box == true){
                $scope.guestVlanOne($scope.dot1x_port[i].name);
            }
        }  
        $scope.load();         
    }   

    $scope.portModeOne = function(name){
        
         SecretApi({
            url: '/api/dot1x/portModeOne/'+name+'/'+$scope.new_dot1x_port.port_mode+'/',
            method: 'PUT',
        }, function() {
            /* $scope.loadLoadBalance(); */
        });
        
        return false;
    }    
    
    $scope.portModeAll = function(){

        var len = $scope.dot1x_port.length;
        for(i=0; i<len; i++){
            if($scope.dot1x_port[i].box == true){
                $scope.portModeOne($scope.dot1x_port[i].name);
            }
        }   
        $scope.load();         
    }  

    $scope.maxUserOne = function(name){
        
         SecretApi({
            url: '/api/dot1x/maxUserOne/'+name+'/'+$scope.new_dot1x_port.max_user+'/',
            method: 'PUT',
        }, function() {
            /* $scope.loadLoadBalance(); */
        });
        
        return false;
    }    
    
    $scope.maxUserAll = function(){

        var len = $scope.dot1x_port.length;
        for(i=0; i<len; i++){
            if($scope.dot1x_port[i].box == true){
                $scope.maxUserOne($scope.dot1x_port[i].name);
            }
        } 
        $scope.load(); 
    }     
/****************** dot1x port configure function  END ************************/    

    $scope.loadIf = function() {
        $scope.MultiSelIf = [];
        $scope.multiSelIf_auth = [];
        $scope.multiSelIf_clear = [];
        $scope.new_dot1x_port = {};  
        $scope.setif = false;
        $scope.chk = false;
        
        SecretApi({
            url: '/api/interface/all/',
        }, function(data) {
            $scope.interface = _.filter(data.arr, function(item) {return item.hw_type == 'Port';});  
        });
    };
        
    $scope.loadDot1xPort = function() {
        $scope.dot1x_port = [];

        SecretApi({
            url: '/api/dot1x/load_dot1x_port/all/',
            method: 'GET',
        }, function(data) {

            $scope.ifp = data.arr;
            var len = $scope.ifp.length;
            for (i=0; i<len; i++){
                $scope.dot1x_port.push($scope.ifp[i]);
                $scope.dot1x_port[i].box = false;
            }


        });
    };
     
    $scope.load = function() {
        $scope.enableAuthCtl();
        $scope.loadIf();      
        $scope.loadDot1xPort();  
    };    
    
    $scope.load(); 
    
}]);


