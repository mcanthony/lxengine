
var State = function(options) {
    $.extend(this, options);
};
$.extend(State.prototype, {
    init : function(gtime) {},
    update : function(gtime) {},
    draw : function(gtime) {},
    shutdown : function(gtime) {}
});


var engine = {};

engine._state = null;
engine._actionQueue = [];

engine.data =
{
    server: {},
    client: {},
    user: {},
    state: {},
    session: {},
};

engine.setTimeout = function (code, delay) {
    return window.setTimeout(code, delay);
};

engine.changeState = function (state) {
    this._actionQueue.push(function () {
             
        
        if (this._state)
        {
            this._state.shutdown(this._gametime);
        }

        this._state = state;

        if (state) 
        {
            try {            
                this.gametime = 0;
                this._state.init(this._gametime);
            } catch (e) {
                console.log(e);
                console.log(state);
                throw e;
            }
        }
    });
};

engine.gametick = 20;
engine.run = function (initialState) {
    
    this.gametime = 0;

    this._state = initialState;
    this._state.init(this.gametime);

    var _this = this;
    (function mainLoop() {

        lx.core.each(_this._actionQueue, function () {
            this.call(_this);
        });
        _this._actionQueue = [];

        //
        // The main loop is done when the engine has transitioned
        // to the null state.
        //
        if (_this._state)
        {
            _this._state.update(_this.gametime);
            _this._state.draw(_this.gametime);
            _this.gametime += Math.max(_this.gametick, 1);
            _this.setTimeout(mainLoop, _this.gametick);
        }
    })();
};

