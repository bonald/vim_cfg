;Public.app.controller('ctlHomeAaaMgr', ['$scope', 'SecretApi', 'SecretHttp', 'Alert', '$interval', '$location', '$document', function($scope, SecretApi, SecretHttp, Alert, $interval, $location, $document) {
    
    $scope.enable_aaa = false;
    $scope.method_list = [];
    $scope.author_method_list = [];
    $scope.account_method_list = [];
    $scope.accountcmd_method_list = [];
    $scope.authen_name_list = [];

    $scope.dispMethod = {};
    $scope.method1 = 'none';
    $scope.method2 = 'none';
    $scope.method3 = 'none';
    $scope.method4 = 'none';
    $scope.method5before = 'none';
    $scope.method5 = 'none';
    $scope.method6 = 'none';
    $scope.method7 = 'none';
    $scope.method8 = 'none';

    $scope.method_list_enum = [
        'enable',
        'line',
        'local',
        'tacplus',
        'radius',
        'none',
        ''
        ];

    $scope.method_list_author = [
        'local',
        'radius',
        'tacplus',
        'none',
        ''
        ];
    $scope.method_list_account_before = [
        'start-stop',
        'stop-only',
        'none',
        ]
    $scope.method_list_account = [
        'radius',
        'tacplus',
        'none',
        ''
        ];

    $scope.method_list_accountcmd = [
        'tacplus',
        'none',
        ''
        ];

    $scope.aaa_method_default = function(x){
        return x.name == 'default'
    }
    $scope.aaa_method_none_default = function(x){
        return x.name != 'default'
    }

    $scope.updateAAAState = function() {

        $scope.enable_aaa = !($scope.enable_aaa);

        SecretHttp({
            url: '/api/aaa_mgr/aaa_status/',
            method: 'PUT',
            data:{
                enable : ($scope.enable_aaa == true) ? 1 : 0,
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
        $scope.load();
    };

    $scope.getAAAState = function() {

        SecretApi({
            url: '/api/aaa_mgr/aaa_status/',
            method: 'GET',
        }, function(data) {
            $scope.enable_aaa = (data.aaa_new_model == 1)? true:false;
        }, function() {

        });
    };

     $scope.setMethodList = function(key) {
        SecretApi({
            url: '/api/aaa_mgr/aaa_method_list/',
            method: 'PUT',
            data: {
                type: 'AUTHEN_LOGIN',
                name: key,
                method1: $scope.method1,
                method2: $scope.method2,
            },
        }, function(data) {
            $scope.load();
            $scope.method1='none';
            $scope.method2='none';
        }, function() {

        });
    };

     $scope.getMethodList = function() {

        SecretApi({
            url: '/api/aaa_mgr/aaa_method_list/',
            method: 'GET',
        }, function(data) {
            $scope.method_list = data.method_list;
            $scope.author_method_list = data.author_method_list;
            $scope.account_method_list = data.account_method_list;
            $scope.accountcmd_method_list = data.accountcmd_method_list;
        }, function() {

        });
    };

     $scope.delMethodList = function(name) {

        SecretApi({
            url: '/api/aaa_mgr/aaa_method_list/',
            method: 'DELETE',
            data: {
                type: 'AUTHEN_LOGIN',
                name: name,
            },
        }, function(data) {
            $scope.load();
            $scope.method1='none';
            $scope.method2='none';            
        }, function() {

        });
    };

     $scope.setAuthorMethodList = function(key) {
        SecretApi({
            url: '/api/aaa_mgr/aaa_method_list/',
            method: 'PUT',
            data: {
                type: 'AAA_ML_AUTHOR_SHELL',
                name: key,
                method3: $scope.method3,
                method4: $scope.method4,
            },
        }, function(data) {
            $scope.load();
            $scope.method3='none';
            $scope.method4='none';
        }, function() {

        });
    };

     $scope.delAuthorMethodList = function(name) {
        SecretApi({
            url: '/api/aaa_mgr/aaa_method_list/',
            method: 'DELETE',
            data: {
                type: 'AAA_ML_AUTHOR_SHELL',
                name: name,
            },
        }, function(data) {
            $scope.load();
            $scope.method3='none';
            $scope.method4='none';
        }, function() {

        });
    };

    $scope.setAccountMethodList = function(key) {
        SecretApi({
            url: '/api/aaa_mgr/aaa_method_list/',
            method: 'PUT',
            data: {
                type: 'AAA_ML_ACCT_SHELL',
                name: key,
                method5before: $scope.method5before,
                method5: $scope.method5,
                method6: $scope.method6,
            },
        }, function(data) {
            $scope.load();
            $scope.method5before='none';
            $scope.method5='none';
            $scope.method6='none';
        }, function() {

        });
    };

     $scope.delAccountMethodList = function(name) {
        SecretApi({
            url: '/api/aaa_mgr/aaa_method_list/',
            method: 'DELETE',
            data: {
                type: 'AAA_ML_ACCT_SHELL',
                name: name,
            },
        }, function(data) {
            $scope.load();
            $scope.method5before='none';
            $scope.method5='none';
            $scope.method6='none';
        }, function() {

        });
    };

    $scope.setAccountcmdMethodList = function(key) {
        SecretApi({
            url: '/api/aaa_mgr/aaa_method_list/',
            method: 'PUT',
            data: {
                type: 'AAA_ML_ACCT_COMMAND',
                name: key,
                method7: $scope.method7,
                method8: $scope.method8,
            },
        }, function(data) {
            $scope.load();
            $scope.method7='none';
            $scope.method8='none';
        }, function() {

        });
    };

     $scope.delAccountcmdMethodList = function(name) {
        SecretApi({
            url: '/api/aaa_mgr/aaa_method_list/',
            method: 'DELETE',
            data: {
                type: 'AAA_ML_ACCT_COMMAND',
                name: name,
            },
        }, function(data) {
            $scope.load();
            $scope.method7='none';
            $scope.method8='none';
        }, function() {

        });
    };
    /**************************************************************************************************/    
    /******************* modal Tap add entry **********************************************************/
    /**************************************************************************************************/  
    $scope.$watch('dispMethodName', function(value) {
        //$scope._refreshDetail();
    });
  
    $scope.viewListDetail = function(method) {
        $scope.dispMethod = method;
        $scope.dispMethodName = method.name;
        $('#load_Authentication_Cfg').modal(); 
    };

    $scope.viewAuthorDetail = function(method) {
        $scope.dispMethod = method;
        $scope.dispMethodName = method.name;
        $('#load_authorization_Cfg').modal(); 
    };

    $scope.viewAccountListDetail = function(method) {
        $scope.dispMethod = method;
        $scope.dispMethodName = method.name;
        $('#load_AAA_ML_ACCT_SHELL_Cfg').modal();
    };

    $scope.viewAccountcmdListDetail = function(method) {
        $scope.dispMethod = method;
        $scope.dispMethodName = method.name;
        $('#load_AAA_ML_ACCT_COMMAND_Cfg').modal();
    };
    
    /**************************************************************************************************/        
    $scope.load = function() {
        $scope.getAAAState();
        $scope.getMethodList();
    };

    $scope.load();

}]);
