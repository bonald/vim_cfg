
;Public.app.controller('ctlHomeIPv4Port', ['$scope', 'SecretApi', 'Alert', function($scope, SecretApi, Alert) {

    /* AJAX */
    $scope.interface = [];

    $scope.pgCurrent = 1;
    $scope.pgItemCnt = 0;
    
    $scope.pgItemNumEachPage = 10;
    
    $scope.interface_ip_address = 
    {
       iface: "",
       ipaddr: "",
       ipmask: "",
       enable: "",
       secondary: "false"
    }
    
    $scope.interface_ip_redirects = 
    {
       iface: "",
       enable: ""
    }

    $scope.interface_ip_verify_unicast_reverse = 
    {
       iface: "",
       enable: ""
    }
    
   
    /* user defines */
    $scope.mask = 
    [
      "0.0.0.0(0)",
      "128.0.0.0(1)",
      "192.0.0.0(2)",
      "224.0.0.0(3)",
      "240.0.0.0(4)",
      "248.0.0.0(5)",
      "252.0.0.0(6)",
      "254.0.0.0(7)",
      "255.0.0.0(8)",
      "255.128.0.0(9)",
      "255.192.0.0(10)",
      "255.224.0.0(11)",
      "255.240.0.0(12)",
      "255.248.0.0(13)",
      "255.252.0.0(14)",
      "255.254.0.0(15)",      
      "255.255.0.0(16)",    
      "255.255.128.0(17)",          
      "255.255.192.0(18)",     
      "255.255.224.0(19)",     
      "255.255.240.0(20)",     
      "255.255.248.0(21)",     
      "255.255.252.0(22)",     
      "255.255.254.0(23)",     
      "255.255.255.0(24)",     
   
      "255.255.255.128(25)",    
      "255.255.255.192(26)",    
      "255.255.255.224(27)",    
      "255.255.255.240(28)",    
      "255.255.255.248(29)",          
      
      "255.255.255.252(30)", 
      "255.255.255.254(31)", 
      "255.255.255.255(32)"       
    ];

    $scope.option_default = 
    {
       select: ""
    }
    
    $scope.option = 
    [
      "Add",
      "Delete"      
    ];

    $scope.secondary = 
    [
      "Default",
      "Secondary"      
    ];
    
    $scope.enable = 
    [
      "Enable",
      "Disable"      
    ];

    /* functions */

    $scope.onPageChange = function() {
    	
    };
    
    $scope.IPAddress = function() {
        iface = $scope.interface_ip_address.iface.name,
        ipaddr = $scope.interface_ip_address.ipaddr,
        ipmask = $scope.mask.indexOf($scope.interface_ip_address.ipmask)     ,    
        secondary = $scope.interface_ip_address.secondary,      
        
        SecretApi({
            url: '/api/ipv4_port_cfg/ipAddress/'+iface+'/'+ipaddr+'/'+ipmask+'/'+"Add"+'/'+secondary+'/',
            method: 'PUT',
        }, function(data) {

        });
               
        $scope.interface_ip_address.iface = " ";    
        $scope.interface_ip_address.ipaddr = " ";  
        $scope.interface_ip_address.mask = " ";    
        $scope.interface_ip_address.enable = " ";  
        $scope.interface_ip_address.secondary = " ";  
      
        $scope.load();

    };    
    
    $scope.ipVerifyUnicastReversePath = function() {

        iface = $scope.interface_ip_verify_unicast_reverse.iface.name,
        enable = $scope.interface_ip_verify_unicast_reverse.enable,     
        
        SecretApi({
            url: '/api/ipv4_port_cfg/ipVerifyUnicastReversePath/'+iface+'/'+enable+'/',
            method: 'PUT',
        }, function(data) {

        });
                
        $scope.interface_ip_verify_unicast_reverse.iface = '';    
        $scope.interface_ip_verify_unicast_reverse.enable = '';  

        $scope.load();
    };

    $scope.ipRedirect = function() {

        iface = $scope.interface_ip_redirects.iface.name,
        enable = $scope.interface_ip_redirects.enable,     

        SecretApi({
            url: '/api/ipv4_port_cfg/ip_redirects/'+iface+'/'+enable+'/',
            method: 'PUT',
        }, function(data) {

        });
                
        $scope.interface_ip_redirects.iface = '';    
        $scope.interface_ip_redirects.enable = '';  

        $scope.load();

    };    

    /* delete ip */
    $scope.deleteIp = function(port) {

        SecretApi({
            url: '/api/ipv4_port_cfg/ipAddress/'+port.name+'/'+port.ip+'/'+port.mask+'/'+'Delete'+'/'+port.flags_secondary+'/',
            method: 'PUT',
        }, function(data) {
            $scope.loadL3Port(); ;
        });
    };
    
    /*  init   */
    $scope.loadIf = function() {
        SecretApi({
            url: '/api/interface/all/',
        }, function(data) {
            $scope.interface = _.filter(data.arr, function(item) {return (item.hw_type == 'Port' || item.hw_type == 'VLAN' || item.hw_type == 'Loopback');});
        });
    };

    $scope.loadL3Port = function() {
        $scope.l3_port_ip = [];
        $scope.l3_port_name = [];        
        
        SecretApi({
            url: '/api/ipv4_port_cfg/load_l3_port/all/',
            method: 'GET',
        }, function(data) {

            $scope.ifp = _.filter(data.arr, function(item) {return (item.ip != 'N/A');});
            var len = $scope.ifp.length;
            for (var i=0; i<len; i++){
                $scope.l3_port_ip.push($scope.ifp[i]);
                $scope.l3_port_ip[i].box = false;
            }
            $scope.pgItemCnt = $scope.ifp.length;  

            $scope.port_list = data.arr;
            len = $scope.port_list.length;
            for (var i=0; i<len; i++){
                var len_port = $scope.l3_port_name.length;
                var add = true;
                for (var j=0; j<len_port; j++){
                    if ($scope.l3_port_name[j].name == $scope.port_list[i].name){
                        add = false;
                    }
                }
                if (add == true){
                    $scope.l3_port_name.push($scope.port_list[i]);
                }

            }
            
        });
    };
         
    $scope.load = function() {
        $scope.loadIf();
        $scope.loadL3Port();        
    };    
    
    $scope.load(); 
    
}]);


