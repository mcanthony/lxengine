// Credit: http://stackoverflow.com/questions/4954403/can-jquery-keypress-detect-more-than-one-key-at-the-same-time
(function($) {

    var data =
    {
        _default : 
        {
        },

        keyState : { },
    };

    var methods =
    {
        init : function() {
            var _this = this;
            var state = data.keyState;
            var _default = data._default;
            $(this).keydown(function(event) { state[event.which] = true; return _default[event.which] !== undefined; });
            $(this).keyup(function(event) { delete state[event.which]; return _default[event.which] !== undefined; });
            $(this).data('keyState', data);                
        },

        state : function() {                
            var data = $(this).data('keyState');
            return data.keyState;
        },

        defaultBehavior : function(keyCodes) {
            $.each(keyCodes, function(i, value) {
                data._default[value] = true;
            });
        },
    };

    $.fn.keyState = function(method) {
                
        var args = Array.prototype.slice.call(arguments, 0);
        if (typeof method == "string")
            args.shift();
        else
            method = method || "init";

        var inner;
        var ret = this.each(function() {
            inner = methods[method].apply(this, args);
        });
        return inner || ret;
    };

})(jQuery);