;Public.app.controller('ctlLogin', ['$scope', '$http','$rootScope', '$location', '$translate', function($scope, $http, $rootScope, $location, $translate) {

    $scope.isSubmit = false;
    
    $scope.form = {
        username: '',
        password: '',
        verficode_id: '',
    };

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

    $scope.submit = function() {
    
        /* Forbidden repeated submit */
        if (isSubmit == true) {
          return;
        }
        isSubmit = true;
        $http({
            url: '/api/token/',
            method: 'POST',
            data: {
                username: $scope.form.username,
                password: $scope.form.password,
            },
        })
        .then(function(resp) {
            if(resp.status == 200 && !resp.data.error) {
                time = parseInt((new Date().getTime()) / 1000);

                // refresh user infomation
                localStorage.setItem('username', $scope.form.username);
                localStorage.setItem('token', resp.data.token);
                localStorage.setItem('time', time);

                // clear error flag
                $rootScope.authErrorFlag = false;

                // login successful
                $rootScope.UserInfo = {
                    username: $scope.form.username,
                    token: resp.data.token,
                    time: time,
                };
                $scope.getproduct();
                $rootScope.System_product = localStorage.getItem('systemproduct')
                $location.path('/');

            }
            else {
                $rootScope.authErrorFlag = true;
                $rootScope.authErrorReason = resp.data['err_reason'];
                
                isSubmit = false;
            }
        }, function() { // ajax error
            $rootScope.authErrorFlag = true;
            $rootScope.authErrorReason = 'ajax request error';
        });
    }; 

    isSubmit = false;
}]);
