;Public.app.controller('ctlHomeNav', ['$scope', '$document', '$location', '$rootScope', function($scope, $document, $location, $rootScope) {


    /* toggle class */
    $document.find('[href="javascript:;"]').click(function() {
        var obj = $document.find(this);
        var parent = obj.parent();

        /* toggle自身active */
        parent.toggleClass('active');
        /* 清除所有子元素active */
        parent.find('ul [href="javascript:;"]').parent().removeClass('active');
    });

}]);
