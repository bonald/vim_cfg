;Public.app.controller('ctlHeader', ['$scope', '$http', '$location', '$rootScope', '$translate', function($scope, $http, $location, $rootScope, $translate) {

    $scope.getproduct = function() {
     $http({
                url: '/api/product/',
                method: 'GET',
            })
            .then(function(resp){
            localStorage.setItem('systemproduct', resp.data.system_product) ;

            }), function() {

            }

    }

     $rootScope.isAcitve = function(regex_url) {
        var res = RegExp(regex_url).test($location.path());
        return res;
    };

    $scope.load = function() {
        $scope.checkToken(); 
        $scope.setLanguage();
        $scope.changeLanguage('en_US');
    };

    $scope.checkToken = function() {
        token = localStorage.getItem('token'),
        username = localStorage.getItem('username'),
        time = localStorage.getItem('time'),

        now =  parseInt((new Date().getTime()) / 1000);
        if(!token || !username || !time || now - time > Public.tokenLiveTime) {
            $location.path('/login');
        }

        $rootScope.UserInfo = {
            username: username,
            token: token,
            time: time,
        };
    };
    
    /**
     * use default language if localStorage.getItem('language') does not exist,
     * otherwise use the language when last logout.
     */
    $scope.setLanguage = function() {
        // check exist
        var language = localStorage.getItem('language');
        if(language) {
            $rootScope.changeLanguage(language);
        }
        else {
            $rootScope.changeLanguage(Public.defaultLanguage);
        }
    };

    $rootScope.logout = function() {
        $http({
            url: '/api/token/logout',
            method: 'POST',
            data: {
                token: localStorage.getItem('token'),
            },
        }).then(function(resp) { // success
            respFlag = true;
        }, function() { // error
            respFlag = true;
        });

        localStorage.removeItem('token');
        localStorage.removeItem('username');
        localStorage.removeItem('time');
        localStorage.removeItem('language');

        $location.path('/login');
    };

    $rootScope.changeLanguage = function(language) {
        $translate.use(language);
        $rootScope.currentUseLanguage = language;
        localStorage.setItem('language', language);
    };

    $rootScope.Sysinfo = {
        product: window.Public.Product,
    }

    $scope.load();
    $rootScope.System_product = localStorage.getItem('systemproduct');
}]);
