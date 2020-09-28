;Public.app.controller('ctlHomeSysMgrFileMgr', ['$scope', 'SecretApi', 'Alert', '$document', '$timeout', 'Confirm', function($scope, SecretApi, Alert, $document, $timeout, Confirm) {

    /* ajax */
    $scope.files = {flash: [], boot: [], startConf: [], log: []};
    $scope.image = {current: '', next: ''};
    $scope.startConf = {current: ''};
    $scope.diskInfo = {flash: {}, disk: {}};

    /* user defined */
    $scope.tabsUsed = 'flash'; // 'flash' or 'image' or 'start_conf' or 'syslog'
    $scope.multiSelBoot = {};
    $scope.multiSelStartConf = {};
    $scope.multiSelLog = {};
    $scope.multiSelFlash = {};
    $scope.backup = {filename: '', backupFilename: '', type: ''};

    $scope.bootChart = new Chart($document.find('#file-mgr-graph-boot')[0].getContext('2d'));
    $scope.flashChart = new Chart($document.find('#file-mgr-graph-flash')[0].getContext('2d'));

    $scope.orderItems = [
        {model: 'filename', display: 'Filename'},
        {model: 'datetime', display: 'Last Modify'},
        {model: 'size', display: 'Size'},
    ],
    $scope.order = {
        reverse: false,
        by: $scope.orderItems[0],
    };

    /* functions */
    $scope.load = function() {
        $scope.multiSelBoot = {};
        $scope.multiSelStartConf = {};
        $scope.multiSelLog = {};
        $scope.multiSelFlash = {};

        $scope.loadDisk();
        switch($scope.tabsUsed) {
            case 'image':
                $scope.loadBoot();
                break;
            case 'start_conf':
                $scope.loadStartConf();
                break;
            case 'syslog':
                $scope.loadSyslog();
                break;
            case 'flash':
                $scope.loadFlash();
                break;
        }
    };

    $scope.refreshTooltip = function() {
        $timeout(function() {
            $('[data-toggle="tooltip"]').tooltip({container: 'body'});
            $('.tooltip.fade.bottom.in').remove();
        }, 0);
    };

    $scope.loadDisk = function() {
        SecretApi({
            url: '/api/disk/',
            method: 'GET',
        }, function(data) {
            $scope.diskInfo.boot = data.boot;
            $scope.diskInfo.flash = data.flash;

            // draw
            $scope.bootChart.Pie([
                {value: $scope.diskInfo.boot.used_p, color: '#FC9286', highlight: '#FF6161', label: 'used'},
                {value: 100 - $scope.diskInfo.boot.used_p, color: '#33FF99', highlight: '#33FF66', label: 'avail'},
            ]);
            $scope.flashChart.Pie([
                {value: $scope.diskInfo.flash.used_p, color: '#FC9286', highlight: '#FF6161', label: 'used'},
                {value: 100 - $scope.diskInfo.flash.used_p, color: '#33FF99', highlight: '#33FF66', label: 'avail'},
            ]);
        });
    };

    $scope.loadBoot = function() {
        SecretApi({
            url: '/api/file_mgr/image/',
            method: 'GET',
        }, function(data) {
            $scope.files.boot = data.boot_files;
            $scope.image = data.image;

            $scope.refreshTooltip();
        });
    };

    $scope.loadStartConf = function() {
        SecretApi({
            url: '/api/file_mgr/start_conf/',
            method: 'GET',
        }, function(data) {
            $scope.files.startConf = data.start_conf_files;
            $scope.startConf = data.start_conf;

            $scope.refreshTooltip();
        });
    };

    $scope.loadSyslog = function() {
        SecretApi({
            url: '/api/file_mgr/syslog/',
            method: 'GET',
        }, function(data) {
            $scope.files.log = data.log_files;

            $scope.refreshTooltip();
        });
    };


    $scope.loadFlash = function() {
        SecretApi({
            url: '/api/file_mgr/flash/',
            method: 'GET',
        }, function(data) {
            $scope.files.flash = data.flash_files;

            $scope.refreshTooltip();
        });
    };

    $scope.changeTabs = function(tab) {
        $scope.tabsUsed = tab;
        $scope.load();
    };

    $scope.bootFileDeleteEn = function(filename) {
        return (filename == $scope.image.current || filename == $scope.image.next) ? false : true;
    };

    /**
     * @param file_arr: [{type: 'image' or 'start_conf' or 'syslog', filename: filename}, {}, ...]
     */
     $scope.deleteFile = function(file_arr) {
        Confirm('Are you sure delete?', function() {
            _exec();
        });

        function _exec() {
            SecretApi({
                url: '/api/file_mgr/',
                method: 'DELETE',
                data: {
                    file_arr: file_arr,
                }
            }, function(data) {
                $scope.load();
            });
         }
    };


    $scope.setNextStartupImage = function(filename) {
        var res = Alert({
            type: 'info',
            title: 'Attention',
            content: 'This operation will take several times, Please wait with patience...',
            time: 0,
        });

        SecretApi({
            url: '/api/boot/'+filename+'/',
            method: 'PUT',
        }, function(data) {
            $scope.load();
            res.hide();
        });
    };

    $scope.setNextStartupConfig = function(filename) {
        Confirm('The old startup-config.conf will be covered, continue?', function() {
            _exec(); 
        });

        function _exec() {
            SecretApi({
                url: '/api/boot/startup_conf/'+filename+'/',
                method: 'PUT',
            }, function(data) {
                $scope.load();
            });
        }
    };

    function getSelObj() {
        var obj = {checkboxModel: {}, data: []};
        switch($scope.tabsUsed) {
            case 'image':
                obj.checkboxModel = $scope.multiSelBoot;
                obj.data = $scope.files.boot;
                break;
            case 'start_conf':
                obj.checkboxModel = $scope.multiSelStartConf;
                obj.data = $scope.files.startConf;
                break;
            case 'syslog':
                obj.checkboxModel = $scope.multiSelLog;
                obj.data = $scope.files.log;
                break;
            case 'flash':
                obj.checkboxModel = $scope.multiSelFlash;
                obj.data = $scope.files.flash;
                break;
        }
        return obj;
    }

    /**
     * @param status
     *      bool true: select all
     *      bool false: clear all
     *      string 'inverse': inverse select
     */
    $scope.setCheckboxAll = function(status) {
        var res = getSelObj();
        _.each(res.data, function(file) {
            filename = file.filename;
            if(_.isBoolean(status)) {
                res.checkboxModel[filename] = status;
            }
            else if(status == 'inverse') {
                tmp = res.checkboxModel[filename];
                if(_.isUndefined(tmp))  res.checkboxModel[filename] = true;
                else                    res.checkboxModel[filename] = !res.checkboxModel[filename];
            }
        })
    };

    $scope.multiDelEn = function() {
        var res = getSelObj().checkboxModel;
        var values = _.values(res);
        return _.contains(values, true);
    };


    $scope.deleteMultiFile = function() {
        var res = getSelObj().checkboxModel;
        var arr = []
        for(i in res) {
            if(res[i]) arr.push({type: $scope.tabsUsed, filename: i});
        }
        $scope.deleteFile(arr);
    }

    /**
     * @param opt: 'show' or 'submit'
     *
     */
    $scope.backupFile = function(opt, filename) {

        if(opt == 'show') {
            $scope.backup.filename = filename;
            $scope.backup.type = $scope.tabsUsed;
            $scope.backup.backupFilename = filename  +'_BAK';
            $('#backupFileModal').modal();
        }
        else { // submit
            var objAlert = Alert({title: 'Attention', type: 'info', content: 'Please wait with patience...', time: 0});

            SecretApi({
                url: '/api/file_mgr/'+$scope.backup.type+'/'+$scope.backup.filename+'/'+$scope.backup.backupFilename+'/',
                method: 'PUT',
            }, function(data) {
                $scope.load();
            }, function() {
                objAlert.hide(); 
            });
        }
    };


    $scope.load();
}]);
