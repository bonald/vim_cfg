;Public.app.controller('ctlHomeSysMgrUpdateMgr', ['$scope', 'SecretApi', 'Alert', 'FileUploader', 'GetToken', '$location', '$timeout', 'Confirm', function($scope, SecretApi, Alert, FileUploader, GetToken, $location, $timeout, Confirm) {

    /* check html5 file api */
    if(window.File && window.FileReader && window.FileList && window.Blob) {
    }
    else {
        $location.path('/home/sys_mgr/update_mgr_without_fileapi/');
    }

    /* ajax */
    $scope.bootImage = '';
    $scope.version = {};
    $scope.freeSize = 0;
    $scope.uploader = new FileUploader();
    $scope.uploader_flash = new FileUploader();
    $scope.bootFiles = [];
    $scope.flashFiles = [];

    /* settings */
    $scope.uploader.queueLimit = 1;
    $scope.uploader_flash.queueLimit = 1;

    /* user defines */
    $scope.uploadPercent = 0;
    $scope.uploadSuccess = false;
    $scope.uploadError = false;
    $scope.uploading = false;

    $scope.uploadPercent_flash = 0;
    $scope.uploadSuccess_flash = false;
    $scope.uploadError_flash = false;
    $scope.uploading_flash = false;

    $scope.imageName = false;
    $scope.file_name_flash=false;
    $scope.waiting = true;

    /* functions */
    $scope.refreshTooltip = function() {
        $timeout(function() {
            $('[data-toggle="tooltip"]').tooltip({container: 'body'});
            $('.tooltip.fade.bottom.in').remove();
        }, 0);
    };

    $scope.load = function() {
        SecretApi({
            url: '/api/boot/',
            method: 'GET',
        }, function(data) {
            $scope.bootImage = data.boot.current_image;
        });

        SecretApi({
            url: '/api/version/',
            method: 'GET',
        }, function(data) {
            $scope.version = data.arr[0];
        });

        SecretApi({
            url: '/api/file_mgr/image/',
            method: 'GET',
        }, function(data) {
            $scope.bootFiles = _.map(data.boot_files, function(item) {return item.filename;});
        }, function() {
            $scope.waiting = false;
        });

        SecretApi({
            url: '/api/disk/',
            method: 'GET',
        }, function(data) {
            switch(data.boot.avail.slice(-1)) {
            case 'G':
                $scope.freeSize = data.boot.avail.slice(0,-1) * 1024 * 1024 * 1024
                break;
            case 'M':
                $scope.freeSize = data.boot.avail.slice(0,-1) * 1024 * 1024
                break;
            case 'K':
                $scope.freeSize = data.boot.avail.slice(0,-1) * 1024
                break;
            default:
                $scope.freeSize = data.boot.avail.slice
                break;
        }
            /*console.log($scope.freeSize);*/
        });

        $scope.waiting = true;
    };

    $scope.clearAllFiles = function() {
        $scope.uploader.clearQueue();
        $scope.uploader_flash.clearQueue();
    };

    $scope.uploadImage = function(applyEn) {
        var item = $scope.uploader.queue[0];

        if(!item) {
//            Alert('please select a boot image file!'); 
            return;
        }

        /* check file exists */
        var filename = item.file.name;
        //console.log($scope.freeSize);
        //console.log(item.file.size);
        if (item.file.size >= $scope.freeSize)
        {
            var res = Alert({
                type: 'info',
                title: 'Attention',
                content: 'There is not enough space!',
                time: 0,
            });
            return;
        }
        if($scope.bootFiles.indexOf(filename) != -1) {
            Confirm('The file with same name already exists, are you sure cover the old file?', function() {
                _upload();
            }, function() {
                return;
            }); 
        }
        else {
            _upload(); 
        }

        function _upload() {
            if(item) {
                $scope.imageName = item.file.name;
                obj = GetToken();
                item.url = Public.apiServer + '/api/boot/upload_image/?username='+obj.username+'&token='+obj.token;

                item.onBeforeUpload = function() {
                    $scope.uploadPercent = 0;
                    $scope.uploadSuccess = false;
                    $scope.uploadError = false;
                    $scope.uploading = true;
                };
                item.onSuccess = function(resp) {
                    $scope.uploadSuccess = true;
                    $scope.uploading = false;
                    $scope.uploadSuccessFn(resp, applyEn);
                };
                item.onError = function() {
                    $scope.uploadError = true;
                    Alert('upload failed');
                    $scope.uploading = false;
                };
                item.onProgress = function(percent) {
                    $scope.uploadPercent = percent;
                };

                item.upload();
            }
            else {
                Alert('please select a boot image file!'); 
            }
            $scope.refreshTooltip();
        }
    };

    $scope.uploadSuccessFn = function(resp, applyEn) {
        if(!resp.error) {
            if(applyEn == false) {
                $scope.load();
                return;
            }

            var res = Alert({
                type: 'info',
                title: 'Attention',
                content: 'updating the next boot image...',
                time: 0,
            });
            SecretApi({
                url: '/api/boot/delete_if_failed/'+$scope.imageName+'/',
                method: 'PUT',
            }, function(data) {
                $location.path('/home/reboot_and_save');
            }, function() {
                res.hide(); 
            });
        }
        else {
            $scope.uploadSuccess = false;
            $scope.uploadError = true;
        }
    };

    $scope.uploadfile2flash = function() {
        var item = $scope.uploader_flash.queue[0];

        if(!item) {
//            Alert('please select a boot image file!');
            return;
        }

        /* check file exists */
        var filename = item.file.name;
        //console.log($scope.freeSize);
        //console.log(item.file.size);
        if (item.file.size >= $scope.freeSize)
        {
            var res = Alert({
                type: 'info',
                title: 'Attention',
                content: 'There is not enough space!',
                time: 0,
            });
            return;
        }
        if($scope.flashFiles.indexOf(filename) != -1) {
            Confirm('The file with same name already exists, are you sure cover the old file?', function() {
                _upload();
            }, function() {
                return;
            });
        }
        else {
            _upload();
        }

        function _upload() {
            if(item) {
                $scope.file_name_flash = item.file.name;
                obj = GetToken();
                item.url = Public.apiServer + '/api/flash/upload_files/?username='+obj.username+'&token='+obj.token;

                item.onBeforeUpload = function() {
                    $scope.uploadPercent_flash = 0;
                    $scope.uploadSuccess_flash = false;
                    $scope.uploadError_flash = false;
                    $scope.uploading_flash = true;
                };
                item.onSuccess = function(resp) {
                    $scope.uploadSuccess_flash = true;
                    $scope.uploading_flash = false;
                    if(!resp.error) {
                        $scope.load();
                    }else{
                        $scope.uploadSuccess_flash = false;
                        $scope.uploadError_flash = true;
                        Alert(resp.err_reason);
                    }
                };
                item.onError = function() {
                    $scope.uploadError_flash = true;
                    Alert('upload failed');
                    $scope.uploading_flash = false;
                };
                item.onProgress = function(percent) {
                    $scope.uploadPercent_flash = percent;
                };

                item.upload();
            }
            else {
                Alert('please select a file!');
            }
            $scope.refreshTooltip();
        }
    };

    $scope.load();
    $scope.refreshTooltip();
}]);
