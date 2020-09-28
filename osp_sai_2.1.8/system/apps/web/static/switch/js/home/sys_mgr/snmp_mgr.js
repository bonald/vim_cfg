;Public.app.controller('ctlHomeSysMgrSnmpMgr', ['$scope', '$filter','SecretApi', 'Alert', '$rootScope', function($scope, $filter, SecretApi, Alert, $rootScope) {

    $scope.hostConf = 'active';
    $scope.navTabs = 'hostConf';
    $scope.enablesnmpServer = true;
    $scope.enableSnmpTrap = true;
    $scope.ver = 'all';
    $scope.targetAddre = '';
    $scope.trapAuthen = '';
    $scope.trapEncryp = '';
    $scope.trapParams = '';
    $scope.trapTargetAddr = '';
    $scope.trapTargetCommunity = '';
    $scope.trapTargetUser = '';
    $scope.trapContextName = '';
    $scope.trapTargetAuthen = '';
    $scope.trapTargetEncry = '';
    $scope.trapTargetNotify = '';
    $scope.trapTargetTagname = '';
    $scope.trapNotitype = '';
    $scope.trapTargetParams = '';
    $scope.trapTargetUsername = '';
    $scope.trapTargetAddress = '';
    $scope.trapTargetCommunity = '';
    $scope.trapTargetServer = '';
    $scope.trapTargetList = '';
    $scope.configure = {};
    $scope.snmpTargetList = [];
    $scope.communityAll = [];
    $scope.snmpTargetV3Context = [];
    $scope.snmpTargetV3Usm = [];
    $scope.snmpTargetNotify = [];
    $scope.snmpTargetParams = [];
    $scope.snmpTargetAddress = [];
    $scope.initLoading = true; 
    $scope.communityName = '';
    $scope.communityAccess = '';
    
    $scope.versions =  [
          'all',
          'v1',
          'v2c',
          'v3'
      ];

    $scope.authenpassword = ['md5', 'sha'];

    $scope.encryppassword = ['des', 'aes'];

    $scope.notifytype = ['trap', 'inform'];

    $scope.targetParams = ['noauth','auth', 'priv'];

    $scope.targetaddress =  [
          'mgmt',
          'default',
      ];

    $scope.access =  [
          'read-only',
          'read-write',
      ];
   
    $scope.changeSnmpServerState = function() {
    
        var enable = ($scope.enablesnmpServer == true) ? 0 : 1;
        var type ='server_enable';

        SecretApi({
            url: '/api/snmp/update_status/'+type+'/'+enable+'/',
            method: 'SET',
        }, function(data) {
            $scope.load();
        })
    };

    $scope.changeSnmpTrapState = function() {
        var enable = ($scope.enableSnmpTrap == true) ? 0 : 1;
        var type ='trap_enable';
        SecretApi({
            url: '/api/snmp/update_status/'+type+'/'+enable+'/',
            method: 'SET',
        }, function(data) {
            $scope.load();
        })
    };
    
    $scope.setSnmpVersion = function() {
        SecretApi({
            url: '/api/snmp/update_version/'+$scope.ver+'/',
            method: 'SET',
        }, function(data) {
            $scope.load();
        })
    };

    $scope.addTrapTargetAddr = function() {

        SecretApi({
            url: '/api/snmp/target_addr_table/',
            method: 'PUT',
            data: {
                ipaddr: $scope.trapTargetAddr,
                commu: $scope.trapTargetCommu,
                addr: $scope.targetAddre,
            },            
        }, function(data) {
            $scope.load();
        })
    };

    $scope.delTrapTargetAddr = function(srv) {

        SecretApi({
            url: '/api/snmp/target_addr_table/',
            method: 'DELETE',
            data: {
                targetAddr: srv,
            },
        }, function(data) {
            $scope.load();
        })
    };

    $scope.getTrapTargetAddr = function() {

        SecretApi({
            url: '/api/snmp/target_addr_table/',
            method: 'GET',          
        }, function(data) {
            $scope.snmpTargetList = data.cfg
        })
    };

    /*context*/
    $scope.addContextname = function() {

        SecretApi({
            url: '/api/snmp/snmp_context/',
            method: 'PUT',
            data: {
                contnama: $scope.trapContextName,
            },
        }, function(data) {
            $scope.load();
        })
    };

    $scope.getContextname = function() {

        SecretApi({
            url: '/api/snmp/snmp_context/',
            method: 'GET',
        }, function(data) {
            $scope.snmpTargetV3Context = data.cfg
        })
    };

    $scope.delContextName = function(ct) {

        SecretApi({
            url: '/api/snmp/snmp_context/',
            method: 'DELETE',
            data: {
                conText: ct,
            },
        }, function(data) {
            $scope.load();
        })
    };

    /*usm-user*/
   $scope.addTrapTargetUser = function() {

        SecretApi({
            url: '/api/snmp/snmp_usm_user/',
            method: 'PUT',
            data: {
                usmuser : $scope.trapTargetUser,
                authensel : $scope.trapAuthen,
                authen : $scope.trapTargetAuthen,
                encrypsel : $scope.trapEncryp,
                encry : $scope.trapTargetEncry,
            },
        }, function(data) {
            $scope.load();
        })
    };

   $scope.getTrapTargetUser = function() {

        SecretApi({
            url: '/api/snmp/snmp_usm_user/',
            method: 'GET',
        }, function(data) {
            $scope.snmpTargetV3Usm = data.cfg
        })
    };

   $scope.delTrapTargetUser = function(uu) {

        SecretApi({
            url: '/api/snmp/snmp_usm_user/',
            method: 'DELETE',
            data: {
                Usm_user: uu,
            },
        }, function(data) {
            $scope.load();
        })
    };

    /*notify*/
   $scope.addTrapTargetNotify = function() {

        SecretApi({
            url: '/api/snmp/snmp_notify/',
            method: 'PUT',
            data: {
                notiname: $scope.trapTargetNotify,
                tagname: $scope.trapTargetTagname,
                notitype: $scope.trapNotitype,
            },
        }, function(data) {
            $scope.load();
        })
    };

   $scope.getTrapTargetNotify = function() {

        SecretApi({
            url: '/api/snmp/snmp_notify/',
            method: 'GET',
        }, function(data) {
            $scope.snmpTargetNotify = data.cfg
        })
    };

   $scope.delTrapTargetNotify = function(nt) {

        SecretApi({
            url: '/api/snmp/snmp_notify/',
            method: 'DELETE',
            data: {
                noTify: nt,
            },
        }, function(data) {
            $scope.load();
        })
    };

    /*target_params*/
   $scope.addTrapTargetParams = function() {

        SecretApi({
            url: '/api/snmp/snmp_target_params/',
            method: 'PUT',
            data: {
                params: $scope.trapTargetParams,
                username: $scope.trapTargetUsername,
                paramsel: $scope.trapParams,
            },
        }, function(data) {
            $scope.load();
        })
   };

   $scope.getTrapTargetParams = function() {

        SecretApi({
            url: '/api/snmp/snmp_target_params/',
            method: 'GET',
        }, function(data) {
            $scope.snmpTargetParams = data.cfg
        })
    };

   $scope.delTrapTargetParams = function(pa) {

        SecretApi({
            url: '/api/snmp/snmp_target_params/',
            method: 'DELETE',
            data: {
                target_Params: pa,
            },
        }, function(data) {
            $scope.load();
        })
    };

    /*target_address_v3*/
   $scope.addTrapTargetAddrV3 = function() {

        SecretApi({
            url: '/api/snmp/snmp_target_address/',
            method: 'PUT',
            data: {
                name: $scope.trapTargetAddress,
                params: $scope.trapTargetCommunity,
                ipaddr: $scope.trapTargetServer,
                tagname: $scope.trapTargetList,
            },
        }, function(data) {
            $scope.load();
        })
    };

   $scope.getTrapTargetAddrV3 = function() {

        SecretApi({
            url: '/api/snmp/snmp_target_address/',
            method: 'GET',
        }, function(data) {
            $scope.snmpTargetAddress = data.cfg
        })
    };

   $scope.delTrapTargetAddress = function(ad) {

        SecretApi({
            url: '/api/snmp/snmp_target_address/',
            method: 'DELETE',
            data: {
                trapName: ad,
            },
        }, function(data) {
            $scope.load();
        })
   };

   $scope.addSnmpCommunity = function() {

        SecretApi({
            url: '/api/snmp/snmp_community_table/',
            method: 'PUT',
            data: {
                name: $scope.communityName,
                access: $scope.communityAccess,
            },            
        }, function(data) {
            $scope.load();
        })
    };

    $scope.delSnmpCommunity = function(entry) {

        SecretApi({
            url: '/api/snmp/snmp_community_table/',
            method: 'DELETE',
            data: {
                community: entry,
            },
        }, function(data) {
            $scope.load();
        })
    };
  
    $scope.getSnmpCommunity = function() {

        SecretApi({
            url: '/api/snmp/snmp_community_table/',
            method: 'GET',          
        }, function(data) {
            $scope.communityAll = data.cfg
        })
    };
  
    $scope.load = function() {
        $scope.initLoading = true;
        SecretApi({
            url: '/api/snmp/get_cfg',
            method: 'GET',
        }, function(data) {
            $scope.configure = data.cfg;
            $scope.enablesnmpServer = ($scope.configure.server_enable==1)? true : false;
            $scope.ver = $scope.configure.version;   
            $scope.enableSnmpTrap = ($scope.configure.trap_enable_coldstart     == 1 
                                     && $scope.configure.trap_enable_linkdown   == 1
                                     && $scope.configure.trap_enable_linkup     == 1
                                     && $scope.configure.trap_enable_system     == 1
                                     && $scope.configure.trap_enable_warmstart  == 1
                                     && $scope.configure.trap_enable_vrrp       == 1
                                     )? true : false;   
           $scope.getTrapTargetAddr();
           $scope.getSnmpCommunity();
           $scope.getContextname();
           $scope.getTrapTargetUser();
           $scope.getTrapTargetNotify();
           $scope.getTrapTargetParams();
           $scope.getTrapTargetAddrV3();
        }, function() {
            $scope.initLoading = false;
        })
    };

    $scope.load();
}]);
