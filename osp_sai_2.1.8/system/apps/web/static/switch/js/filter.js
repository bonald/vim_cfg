;Public.app

/**
 * disp file size such as 50M, 100K, 1.1K
 */
.filter('fsize', function() {
    var KB = 1024;
    var MB = KB * 1024;
    var GB = MB * 1024;
    var TB = GB * 1024;

    function int2Str(num) {
        if(!_.isNumber(num)) return 'NaN';

        var str = '';
        if(num / TB > 1) str = '' + (num / TB).toFixed(3) + 'T';
        else if(num / GB > 1) str = '' + (num / GB).toFixed(3) + 'G';
        else if(num / MB > 1) str = '' + (num / MB).toFixed(3) + 'M';
        else if(num / KB > 1) str = '' + (num / KB).toFixed(3) + 'K';
        else str = '' + num + 'B';

        return str;
    }

    return function(input) {
        var output = null;

        if(_.isNumber(input)) {
            output = int2Str(input);
        }
        else if(_.isArray(input)) {
            output = [];

            _.each(input, function(item) {
                output.push(int2Str(item))
            });
        }

        return output;
    };
})

.filter('switchPort', function() {
    return function(input) {
        if(/^.*_\d$/.test(input)) {
            return input.replace('_', '/');
        }
        else {
            return input; 
        }
    };
})

.filter('ipv4', function() {
    return function(input) {
        var tmp = input.match(/(\d{1,3}\.){3}(\d{1,3})/)[0];
        return tmp;
    };
})

.filter('mask', function() {
    return function(input) {
        var tmp = input.match(/#(\d{1,2})/)[1];
        
        if(tmp) {
            
        }
        else {
            return "none";
        }
        
        return tmp;
    };
})

;
