var engine = {};

engine._state = null;
engine._actionQueue = [];

engine.setTimeout = function (code, delay) {
    return window.setTimeout(code, delay);
};

engine.changeState = function (state) {
    this._actionQueue.push(function () {
        try {
            this._state = state;
            this.gametime = 0;
            this._state.init(this._gametime);
        } catch (e) {
            console.log(e);
            console.log(state);
            throw e;
        }
    });
};

engine.run = function () {
    this.gametime = 0;
    var gametick = 20;

    this._state.init(this.gametime);

    var _this = this;
    (function gameLoop() {

        lib.each(_this._actionQueue, function () {
            this.call(_this);
        });
        _this._actionQueue = [];

        _this._state.update(_this.gametime);
        _this._state.draw(_this.gametime);
        _this.gametime += gametick;
        _this.setTimeout(gameLoop, gametick);
    })();
};

