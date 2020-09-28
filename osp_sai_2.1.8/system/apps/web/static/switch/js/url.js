// 默认所有不存在的路由重定向到/home/index

;Public.app.config(['$routeProvider', function($routeProvider) {

    $routeProvider.otherwise({
        redirectTo: '/home/index' 
    });

    for(i = 0; i < Public.routers.length; i++) {
        $routeProvider.when(Public.routers[i][0], {
            controller: Public.routers[i][1],
            templateUrl: Public.routers[i][2]
        }); 
    }
    
}]);
