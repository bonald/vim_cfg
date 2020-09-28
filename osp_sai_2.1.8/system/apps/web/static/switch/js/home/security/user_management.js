;Public.app.controller('ctlHomeSecurityUserManagement', ['$scope', 'SecretApi', 'Alert', '$document', '$timeout', 'Confirm', function($scope, SecretApi, Alert, $document, $timeout, Confirm) {
    /* ajax */
    $scope.user = [];

    /* user defined */
    $scope.multiSelUser = [];
    $scope.curToken = localStorage.getItem('token');

     $scope.privilege =  [
          '1',
          '2',
          '3',
          '4'
      ];

    /* functions */
    $scope.load = function() {
        $scope.multiSelUser = [];
        $scope.loadUser();
    };

    $scope.loadUser = function() {
        SecretApi({
            url: '/api/user_management/',
            method: 'GET'
        }, function(data) {
            $scope.user = data.cfg;
        });
    };

    $scope.updateUser = function(username) {
        SecretApi({
            url: '/api/user_management/',
            method: 'PUT',
            data: {
                username : $scope.user.username,
                password : $scope.user.password,
                conPasswd : $scope.user.conPasswd,
                privilege : $scope.user.privilege,
            }
        }, function(data) {
            $scope.load();
        });
        $scope.user.username = '';
        $scope.user.password = '';
        $scope.user.conPasswd = '';
        $scope.user.privilege = '';
        $scope.loadUser();
        $scope.showSetUserForm = false;
    };

    /**
     * @param username
     */
    $scope.deleteUser = function(username) {
        SecretApi({
            url: '/api/user_management/' + username + '/',
            method: 'DELETE',
            data: {
                'username': username
            }
        }, function(data) {
            $scope.load();
        });
    };

	/**
     * @param status
     *      bool true: select all
     *      bool false: clear all
     */
    $scope.setCheckboxAll = function(status) {
		for(var i = 0; i < $scope.user.length; i++) {
			$scope.multiSelUser[i] = status;
        }
    };
	
    $scope.deleteMulti = function() {
		for(var i = 0; i < $scope.user.length; i++) {
			var x = $scope.user[i];
            if($scope.multiSelUser[i]) {
				$scope.deleteUser(x.username);
            }
        }
    };


    $scope.change = function() {
        var mx = 0;
		for(var i = 0; i < $scope.user.length; i++) {
			var x = $scope.user[i];
            if($scope.multiSelUser[i]) {
				mx++
            }
            $scope.selected = mx;
        }
    };



	$scope.addUser = function() {

        SecretApi({
            url: '/api/user_management/',
            method: 'ADD',
            data:{
                username : $scope.user.username,
                password : $scope.user.password,
                conPasswd : $scope.user.conPasswd,
                privilege : $scope.user.privilege,
            },
        }, function(data) {
            $scope.load();
        });

        $scope.user.username = '';    
        $scope.user.password = '';  
        $scope.user.conPasswd = '';  
        $scope.user.privilege = '';  
        $scope.loadUser();
        $scope.showAddUserForm = false; 
    };

    $scope.load();

} ]);