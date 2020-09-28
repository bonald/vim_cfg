;Public.app

.component('btnSwitch', {
    template: [
        '<div class="btn-switch {{ $ctrl.css() }}" ng-click="$ctrl.clickFn()">',
        '   <div class="switch user-select-none">',
        '       {{ $ctrl.text() }}',
        '   </div>',
        '</div>',
    ].join(''),

    bindings: {
        switchState: '<',
        switchTextOn: '@',
        switchTextOff: '@',
        switchOnChange: '&', // function(state) {return bool}
        switchDisabled: '<',
    },

    controller: ['$document', function($document) {
        var ctrl = this;

        if(!angular.isDefined(ctrl.switchTextOn))   ctrl.switchTextOn = 'on';
        if(!angular.isDefined(ctrl.switchTextOff))  ctrl.switchTextOff = 'off';
        if(!angular.isDefined(ctrl.switchState))    ctrl.switchState = false;

        ctrl.css = function() {
            var res = ctrl.switchState == true ? 'active' : '';
            if(ctrl.switchDisabled == true) res += ' disabled';
            return res;
        };

        ctrl.text = function() {
            return ctrl.switchState ? ctrl.switchTextOn : ctrl.switchTextOff;
        };

        ctrl.clickFn = function() {
            if(ctrl.switchDisabled == true) return;

            var res = ctrl.switchOnChange();
            if(res != false) {
                ctrl.switchState = !ctrl.switchState;
            }
        };
    }],
})


;
