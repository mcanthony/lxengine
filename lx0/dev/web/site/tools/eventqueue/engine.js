//
// Utility meta-events
//
var co = {

    //
    // Repeat an event "count" times
    //
    repeat: function (count, event) {
        var ctor = function () {
            this.i = 0;
            this.j = count;
            this.event = event;
        }
        ctor.prototype.run = function (time) {
            if (this.i < this.j) {
                var ret = this.event.run(time);
                if (ret > 0)
                    return ret;

                if (ret < 0)
                    this.i++;
                return this.run(time);
            }
            else {
                this.i = 0;
                return -1;
            }
        }
        return new ctor();
    },

    //
    // Chain a sequence of events together into a single event
    //
    sequence: function () {
        var ctor = function () {
            this.sequence = [];
            this.index = 0;
        }
        ctor.prototype.run = function (time) {
            if (this.index < this.sequence.length) {
                var event = this.sequence[this.index];
                var ret = event.run(time);

                if (ret > 0)
                    return ret;
                if (ret < 0)
                    this.index++;
                return this.run(time);
            }
            else {
                this.index = 0;
                return -1;
            }
        };

        var obj = new ctor();
        for (var i = 0; i < arguments.length; ++i)
            obj.sequence.push(arguments[i]);
        return obj;
    },

    //
    // Pause execution of the event sequence for "delay" milliseconds
    //
    wait: function (delay) {
        var obj = new Object();
        var initial = function (time) {
            this.run = function (time) {
                this.run = initial;
                return -1;
            };
            return delay;
        };
        obj.run = initial;
        return obj;
    },

    //
    // Wrap a function into an event
    //
    func: function (f) {
        var obj = new Object();
        obj.run = function (time) {
            f(time);
            return -1;
        };
        return obj;
    }
};

//
// Engine
// 
// (Runs the event loop)
//
var Engine = function () {
    this._currentTime = 0;
    this._tickLength = 20;
    this._timerId = 0;
    this._queue = [];
};
Engine.prototype.addEvent = function (delay, event) {
    this._queue.push([this._currentTime + delay, event]);
};
Engine.prototype.update = function () {

    // Sort every time.  This is a prototype, so a naive, inefficient strategy like
    // this is acceptable.
    this._queue.sort(function (a, b) {
        return a[0] - b[0];
    });

    if (this._queue.length > 0) {
        var reAdd = [];

        while (this._queue.length > 0 && this._queue[0][0] <= this._currentTime) {
            var event = this._queue.shift()[1];
            var ret = event.run(this._currentTime);
            if (ret >= 0) {
                reAdd.push([ret, event]);
            }
        }

        // Re-add any unfinished events to the queue
        for (var i = 0; i < reAdd.length; ++i)
            this.addEvent(reAdd[i][0], reAdd[i][1]);
    }
    else
        window.clearInterval(this._timerId);

    this._currentTime += this._tickLength;
};

Engine.prototype.run = function () {
    var _this = this;
    this._timerId = window.setInterval(function () {
        _this.update();
    }, this._tickLength);
};
