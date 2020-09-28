;Public.app
.directive('pagination', function() {
    return {
        restrict: 'E',

        transclude: true,

        template: [
            '<div class="ng-pagination">',
            '    <nav ng-show="pageNums.length > 1">',
            '        <ul class="pagination">',
            '            <li ng-click="goToPage(1)">',
            '                <a href="javascript:;">{{ pgHomeText }}</a>',
            '            </li>',
            '            <li ng-click="goToPage(pgCurrent - 1)">',
            '                <a href="javascript:;" aria-label="Previous">',
            '                    <span aria-hidden="true">&laquo;</span>',
            '                </a>',
            '            </li>',
            '            <li ng-repeat="num in pageNums" ng-class="{\'active\': pgCurrent == num}" ng-click="goToPage(num)">',
            '                <a href="javascript:;">{{ num }}</a>',
            '            </li>',
            '            <li ng-click="goToPage(pgCurrent + 1)">',
            '                <a href="javascript:;" aria-label="Next">',
            '                    <span aria-hidden="true">&raquo;</span>',
            '                </a>',
            '            </li>',
            '            <li ng-click="goToPage(pgCnt)">',
            '                <a href="javascript:;">{{ pgLastText }}</a>',
            '            </li>',
            '            <li>',
            '                <form class="form form-inline">',
            '                    <input type="text" class="form-control pag-input" ng-model="jumpToPage"><label>&nbsp;&nbsp;/{{ pgCnt }}</label>',
            '                    <button class="btn btn-primary" ng-click="goToPage(jumpToPage)" ng-disabled="!jumpToPage || jumpToPage <= 0 || jumpToPage > pgCnt">Go</button>',
            '                </form>',
            '            </li>',
            '        </ul>',
            '    </nav>',
            '</div>',
        ].join(''),

        scope: {
            pgItemCnt: '=',      // item count
            pgItemNumEachPage: '<?', // item number each page
            pgCurrent: '=',     // current page
            pgMaxBtnCnt: '<?', // max display button count
            pgOnChange: '&?',
            pgHomeText: '@?',
            pgLastText: '@?',
        },
        
        link: function(scope, element, attrs) {
            /* init */
            (function() {
                scope.pgItemCnt = angular.isDefined(scope.pgItemCnt) ? parseInt(scope.pgItemCnt) : 0;
                scope.pgItemNumEachPage = angular.isDefined(scope.pgItemNumEachPage) ? parseInt(scope.pgItemNumEachPage) : 10;
                scope.pgCurrent = angular.isDefined(scope.pgCurrent) ? parseInt(scope.pgCurrent) : 1;
                scope.pgMaxBtnCnt = angular.isDefined(scope.pgMaxBtnCnt) ? parseInt(scope.pgMaxBtnCnt) : 9;
                scope.pgOnChange = angular.isDefined(scope.pgOnChange) ? scope.pgOnChange : function() {};
                scope.pgHomeText = angular.isDefined(scope.pgHomeText) ? scope.pgHomeText : 'Home';
                scope.pgLastText = angular.isDefined(scope.pgLastText) ? scope.pgLastText : 'End';
                
                /* other */
                scope.btnCenterIndex = Math.ceil(scope.pgMaxBtnCnt / 2);
                scope.pageNums = [];
            })();

            scope.$watch('pgItemCnt', function(newVal) {
//                scope.pgCurrent = 1;
                scope.pgCnt = Math.ceil(scope.pgItemCnt / scope.pgItemNumEachPage);
                scope._refresh();
            });

            scope.$watch('pgCurrent', function(newVal) {
                scope.jumpToPage = scope.pgCurrent;
                scope.pgOnChange();
            });

            scope.goToPage = function(num) {
                scope.pgCurrent = parseInt(num);
                scope._refresh();
            };

            scope._refresh = function() {
                /* 越界检测 */
                scope.pgCurrent = (scope.pgCurrent < 1) ? 1 : scope.pgCurrent;
                scope.pgCurrent = (scope.pgCurrent > scope.pgCnt) ? scope.pgCnt : scope.pgCurrent;

                scope.pageNums = [];

                if(scope.pgCnt <= scope.pgMaxBtnCnt) { /* 显示所有按钮 */
                    for(var i = 0; i < scope.pgCnt; i++) {
                        scope.pageNums.push(i + 1); 
                    }
                }
                else { /* 显示部分按钮 */
                    /* start */
                    var indexStart = scope.pgCurrent - scope.btnCenterIndex;
                    indexStart = (indexStart < 0) ? 0 : indexStart;

                    /* end */
                    var indexEnd = scope.pgCurrent + scope.pgMaxBtnCnt - scope.btnCenterIndex;
                    indexStart = (indexEnd >= scope.pgCnt) ? (scope.pgCnt - scope.pgMaxBtnCnt) : indexStart;

                    for(var i = 0; i < scope.pgMaxBtnCnt; i++) {
                        scope.pageNums.push(indexStart + i + 1); 
                    }
                }
            };
        },
    };
})



;
