;window.Public = {
    app: angular.module('appMain', ['ngRoute', 'ngAnimate', 'pascalprecht.translate', 'angularFileUpload', 'ngCookies']),

    routers: [ // 格式：[路由，控制器，html路径]
        /* login */
        ['/login', 'ctlLogin', './templates/login.html'],

        /* home */
        ['/home/index', 'ctlHomeStatus', './templates/home/status.html'],
        /* sys_mgr */
        ['/home/sys_mgr/file_mgr', 'ctlHomeSysMgrFileMgr', './templates/home/sys_mgr/file_mgr.html'],
        ['/home/sys_mgr/update_mgr', 'ctlHomeSysMgrUpdateMgr', './templates/home/sys_mgr/update_mgr.html'],
        ['/home/sys_mgr/update_mgr_without_fileapi', 'ctlHomeSysMgrUpdateMgrWithoutFileApi', './templates/home/sys_mgr/update_mgr_without_fileapi.html'],
        ['/home/sys_mgr/sys_conf', 'ctlHomeSysMgrSysConf', './templates/home/sys_mgr/sys_conf.html'],
        ['/home/sys_mgr/log_mgr', 'ctlHomeSysMgrLogMgr', './templates/home/sys_mgr/log_mgr.html'],
        ['/home/sys_mgr/snmp_mgr', 'ctlHomeSysMgrSnmpMgr', './templates/home/sys_mgr/snmp_mgr.html'],
        ['/home/sys_mgr/time_mgr', 'ctlHomeSysMgrTimeMgr', './templates/home/sys_mgr/time_mgr.html'],
        ['/home/filemgr', 'ctlHomeFilemgr', './templates/home/filemgr.html'],
        /* port_mgr */
        ['/home/port_mgr/status', 'ctlHomePortMgrStatus', './templates/home/port_mgr/status.html'],
        ['/home/port_mgr/if_statistic', 'ctlHomePortMgrStatistic', './templates/home/port_mgr/if_statistic.html'],
        ['/home/port_mgr/link_agg', 'ctlHomePortMgrLinkAgg', './templates/home/port_mgr/link_agg.html'],
        ['/home/port_mgr/storm_ctl', 'ctlHomePortMgrStormCtl', './templates/home/port_mgr/storm_ctl.html'],
        ['/home/port_mgr/transceiver', 'ctlHomeTransceiverStatus', './templates/home/port_mgr/transceiver.html'],


        /* srv_mgr */
        ['/home/srv_mgr/vlan/status', 'ctlHomeSrvMgrVlanStatus', './templates/home/srv_mgr/vlan/status.html'],
        ['/home/srv_mgr/vlan/access_trunk', 'ctlHomeSrvMgrVlanAccessTrunk', './templates/home/srv_mgr/vlan/access_trunk.html'],

        ['/home/srv_mgr/mac/mac_address_table', 'ctlHomeSrvMgrMacMacAddressTable', './templates/home/srv_mgr/mac/mac_address_table.html'],

        /* security */
        ['/home/routing/ipv4_routing', 'ctlHomeIPv4Routing', './templates/home/routing/ipv4_routing.html'],
        ['/home/routing/ipv4_static', 'ctlHomeIPv4Static', './templates/home/routing/ipv4_static.html'],
        ['/home/routing/ipv4_port_cfg', 'ctlHomeIPv4Port', './templates/home/routing/ipv4_port_cfg.html'],
        
        /* security */
        ['/home/security/ddos_prevent', 'ctlHomeSecuDDosPrevent', './templates/home/security/ddos_prevent.html'],
        ['/home/security/worm_filter', 'ctlHomeSecuWormFilter', './templates/home/security/worm_filter.html'],
        ['/home/security/arp_table', 'ctlHomeSecuArpTable', './templates/home/security/arp.html'],
        ['/home/security/dhcp_snooping', 'ctlHomeSecurityDhcpSnoop', './templates/home/security/dhcp_snooping.html'],
        ['/home/security/dot1x', 'ctlHomeDot1x', './templates/home/security/dot1x.html'],
		['/home/security/web_sessions', 'ctlHomeSecurityWebSessions', './templates/home/security/web_sessions.html'],
		['/home/security/user_management', 'ctlHomeSecurityUserManagement', './templates/home/security/user_management.html'],

        /* authentication */
        ['/home/authentication/aaa_mgr', 'ctlHomeAaaMgr', './templates/home/authentication/aaa_mgr.html'],
        ['/home/authentication/tacas_mgr', 'ctlHomeTacasMgr', './templates/home/authentication/tacas_mgr.html'],
        ['/home/authentication/radius_mgr', 'ctlHomeRadiusMgr', './templates/home/authentication/radius_mgr.html'],

        /* tap */
        ['/home/tap/tap_mgr', 'ctlHomeTapMgr', './templates/home/tap/tap_mgr.html'],
        ['/home/tap/flow_mgr', 'ctlHomeTapFLowMgr', './templates/home/tap/flow_mgr.html'],
        ['/home/tap/flow_inner_match_mgr', 'ctlHomeTapFLowInnerMgr', './templates/home/tap/flow_inner_match_mgr.html'],
        ['/home/tap/acl_mgr', 'ctlHomeTapAclMgr', './templates/home/tap/acl_mgr.html'],
        ['/home/tap/tap_statistics', 'ctlHomeTapFLowStatistics', './templates/home/tap/tap_statistics.html'],
        
        /* tools */
        ['/home/tools/ping', 'ctlHomeToolsPing', './templates/home/tools/ping.html'],
        ['/home/tools/traceroute', 'ctlHomeToolsTraceroute', './templates/home/tools/traceroute.html'],

        /* reboot and save */
        ['/home/reboot_and_save', 'ctlHomeRebootAndSave', './templates/home/reboot_and_save.html'],
        ['/home/reboot/:save?', 'ctlHomeReboot', './templates/home/reboot.html'],
        ['/home/debug', 'ctlHomeDebug', './templates/home/debug.html'],
    ],

    apiServer: '',

    tokenLiveTime: 600, // seconds

    baseUrl: '',

    defaultLanguage: 'zh_CN',
    
    /*!! build_powerpc.sh will re-modify it so DONT modify 'CNOS' word follow line except you know what you do !!*/
    Product: 'CNOS', // 'CNOS' or 'TAP'. use $rootScope.System_product
};


/* 修改$httpProvider给每个请求添加apiServer */
Public.app.config(['$httpProvider', function($httpProvider) {
    $httpProvider.interceptors.push(['$q', function($q) {
        return {
            'request': function(conf) {
                var isHtml = /^.+\.html$/.test(conf.url);
                if(!isHtml) {
                    conf.url = Public.apiServer + conf.url;
                }
                return conf || $q.when(conf); 
            }
        }; 
    }]);
}]);


/* 为每个auth error重定向到login */
Public.app.config(['$httpProvider', function($httpProvider) {
    $httpProvider.interceptors.push(['$q', '$location', '$rootScope', function($q, $location, $rootScope) {
        return {
            'response': function(conf) {
                if(conf.data['err_auth'] == true) {
                    $rootScope.authErrorFlag = true;
                    $rootScope.authErrorReason = conf.data['err_reason'];
                    $location.path('/login')
                }
                return conf || $q.when(conf); 
            }
        }; 
    }]);
}]);

