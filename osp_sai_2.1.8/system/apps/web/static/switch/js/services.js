;Public.app

/**
 * return {
 *     uername: '',
 *     token: '',
 * }
 */
.value('GetToken', function() {
    obj = {
        username: '',
        token: '',
    };

    obj.token =  localStorage.getItem('token');
    obj.username = localStorage.getItem('username');

    time = localStorage.getItem('time');
    localStorage.setItem('time', time); // refresh time

    return obj;
})



/**
 * alert message
 * @param options
 *     string: display error message, 5 seconds
 *     object: {
 *         title: string
 *         type: danger | waring | success | info
 *         content: string
 *         time: number of seconds or 0(auto dismiss never, you must use hide() to dismiss it)
 *     }
 */
.config(['$provide', function($provide) {
    $provide.provider('Alert', function() {
        var self = this;

        self.$get = ['$rootScope', '$timeout', function($rootScope, $timeout) {
            return function(options) {
                $timeout.cancel($rootScope.alertTimePromise);

                if(_.isString(options)) {
                    options = {
                        type: 'danger',
                        title: 'Error!',
                        content: options,
                        time: 3,
                    }
                }

                options = _.extend({time: 3}, options);
                
                switch(options.type) {
                    case 'danger':
                          options.class = 'alert-danger';
                          break;
                    case 'waring':
                          options.class = 'alert-waring';
                          break;
                    case 'success':
                          options.class = 'alert-success';
                          break;
                    case 'info':
                          options.class = 'alert-info';
                          break;
                }

                $rootScope.alertOptions = options;
                $rootScope.alertShow = true;
                
                time = parseInt(options.time)
                if(time != 0) {
                    $rootScope.alertTimePromise = $timeout(function() {
                        $rootScope.alertShow = false;
                    }, time * 1000)
                }

                return {
                    hide: function() {
                        $rootScope.alertShow = false;
                    }
                };
            };
        }];
    });
}])


/**
 * confirm
 */
.config(['$provide', function($provide) {
    $provide.provider('Confirm', function() {
        var self = this;

        self.$get = ['$rootScope', '$timeout', function($rootScope, $timeout) {
            return function(content, fnYes, fnNo) {
                if(!_.isFunction(fnYes)) fnYes = function() {}; 
                if(!_.isFunction(fnNo)) fnNo = function() {}; 

                $rootScope.confirmShow = true;

                $rootScope.confirm = {
                    fnClickYes: function() {
                        $rootScope.confirmShow = false;
                        fnYes();
                    },
                    fnClickNo: function() {
                        $rootScope.confirmShow = false;
                        fnNo();
                    },
                    fnClickClose: function() {
                        $rootScope.confirmShow = false;
                    },
                    content: content,
                };
            };
        }];
    });
}])


.config(['$provide', function($provide) {
    $provide.provider('SecretHttp', function() {
        var self = this;

        self.$get = ['$http', function($http) {
            return function(options) {
                if(_.isUndefined(options.method)) options.method = 'GET';

                time = localStorage.getItem('time');

                if(options.method == 'GET') {
                    options.params = _.extend({
                        token: localStorage.getItem('token'),
                    }, options.params);
                }
                else {
                    options.data = _.extend({
                        token: localStorage.getItem('token'),
                    }, options.data);
                }
                
                localStorage.setItem('time', parseInt((new Date().getTime()) / 1000)); // refresh time

                return $http(options);
            };
        }];
    });
}])

.config(['$provide', function($provide) {
    $provide.provider('SecretApi', function() {
        var self = this;

        self.$get = ['SecretHttp', 'Alert', function(SecretHttp, Alert) {
            return function(options, fnSuccess, fnBefore, fnAfter) {
                SecretHttp(options)
                .then(function(resp) {
                    if(fnBefore) fnBefore();
                    if(!_.isObject(resp.data)) {
                        Alert('bad response');
                        return; 
                    }
                    if(_.isUndefined(resp.data.error)) {
                        Alert('response.error is undefined');
                        return;
                    }
                    if(!resp.data.error) {
                        fnSuccess(resp.data);
                    }
                    else {
                        Alert(resp.data['err_reason']);
                    }
                    if(fnAfter) fnAfter();
                }, function() {
                    if(fnBefore) fnBefore();
                    Alert('AJAX error');
                    if(fnAfter) fnAfter();
                });
            };
        }];
    });
}])

/**
 * canvas: canvas document
 * type: ['Bar', 'Line']
 * labels: array
 * data: array
 * options: Chart options, eg: {animation: false}
 * interval: eg: 5
 */
.value('CreateGraph', function(canvas, type, labels, data, options, time) {
    var ChartData = {
        labels : labels,
        datasets : [{
            fillColor : "rgba(151,187,205,0.5)",
            strokeColor : "rgba(151,187,205,0.8)",
            highlightFill : "rgba(151,187,205,0.75)",
            highlightStroke : "rgba(151,187,205,1)",
            data: data
        }],
    };

    if(!options['responsive']) options.responsive = true;

    var res = new Chart(canvas.getContext('2d'));

    res[type](ChartData, options);

    time = parseInt(time)
    if(0 != time) {
        setInterval(function() {
            res[type](ChartData, options);
        }, time);
    }

    return res;
})

/**
 * convert range string to array
 * @param sRange:
 *    string of range:
 *        e.g. '1,3-6,10' -> [1,3,4,5,6,10]
 * @param isStrict:
 *    true or undefined or false
 *    strict mode, return [] if sRange has invalid character.
 */
.value('Range2Array', function(sRange, isStrict) {
    if(sRange == '') return [];

    var arrRet = [];
    var arrRange = sRange.split(',');
    isStrict = (_.isBoolean(isStrict) && isStrict == true) ? true : false;

    for(var i = 0; i < arrRange.length; i++) {
        var tmpRange = arrRange[i];
        if(/^\d+$/.test(tmpRange)) {
            arrRet.push(parseInt(tmpRange));
        }
        else if(/^\d+\-\d+$/.test(tmpRange)) {
            var tmp = tmpRange.split('-');
            var start = parseInt(tmp[0]);
            var end = parseInt(tmp[1]);

            if(isStrict && start > end) return [];

            var arrTmp = _.range(start, end + 1);
            arrRet = arrRet.concat(arrTmp);
        }
        else if(isStrict) {
            return []; 
        }
    }

    return arrRet;
})

/**
 * convert array to range string
 * @param arr:
 *    int array:
 *        e.g. [1,3,4,5,6,10] -> '1,3-6,10'
 */
.value('Array2Range', function(arr){
    if(arr.length == 0) return ''
    if(arr.length == 1) return arr[0].toString();

    var strRet = '';
    var arrBreak = [];

    var breaking = false;
    var startNum = parseInt(arr[0])
    for(var i = 1; i < arr.length; i++) {
        if(parseInt(arr[i]) - parseInt(arr[i - 1])> 1) { // breaking
            arrBreak.push({start: startNum, end: parseInt(arr[i - 1])});
            startNum = parseInt(arr[i]);
        }
    }
    arrBreak.push({start: startNum, end: parseInt(arr[arr.length - 1])});

    var arrStr = [];
    for(var i = 0; i < arrBreak.length; i++) {
        var tmp = arrBreak[i]; 
        if(tmp.start == tmp.end) {
            arrStr.push(tmp.start.toString());
        }
        else {
            arrStr.push(tmp.start.toString() + '-' + tmp.end.toString());
        }
    }

    strRet = arrStr.join(',');
    return strRet;
})


.value('Base64', {
    encodeString: function(string) {
        return BASE64.encoder(string);
    },

    decodeString: function(string) {
        var res = '';
        var arr = BASE64.decoder(string);

        for(var i = 0; i < arr.length; i++) {
            res += String.fromCharCode(arr[i]); 
        };

        return res;
    },
})

;
