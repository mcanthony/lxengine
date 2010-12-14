/*
    This is a rough, workaround imitation of a JQuery-like interface
    for the LxEngine DOM.   Development is currently primarily 
    focused on the core architecture; therefore, this code is largely
    "good enough" and no better in terms of quality in order to allow
    core development to continue.

    A full, production quality re-write of this file will likely be
    necessary once the core architecture and APIs are more well-defined.
 */

var $ = function (selector) {
    var q = new LxQuery();
    return q.init(selector);
};

var LxQuery = function () {};
LxQuery.prototype = {
    init: function (selector) {
        var re_singleTag = /^<(\w+)\s*\/?>(?:<\/\1>)?$/;
        var re_id = /^#(\w+)$/;
        var re_tag = /^(\w+)$/;

        if (typeof(selector) == "object") {
            return this._select([selector]);
        }

        var result = re_singleTag.exec(selector);
        if (result) {
            var elem = document.createElement(result[1]);
            return this._select([elem]);
        }

        var result = re_id.exec(selector);
        if (result) {
            var elem = document.getElementById(result[1]);
            return this._select([elem]);
        }

        var result = re_tag.exec(selector);
        if (result) {
            var elems = document.getElementsByTagName(result[1]);
            return this._select(elems);
        }

        alert("Could not find elem for selector '" + selector + "'");

        return this._select([]);
    },
    _selection: [],
    _select: function (e) {
        this._selection = e;
        return this;
    },
    attr: function (name, value) {
        if (value != undefined)
        {
            if (this._psuedoAttributes[name] === undefined) {
                for (var i = 0; i < this._selection.length; ++i) {
                    var elem = this._selection[i];           
                    elem.setAttribute(name, value);
                }
            }
            else
                this._psuedoAttributes[name].call(this, value);
        }
        else
            return this._selection[0].getAttribute(name);
        return this;
    },
    value : function () {
        if (arguments.length == 0)
        {
            var elem = this._selection[0];
            return elem.value;
        }
        else if (arguments.length == 1)
        {
            var elem = this._selection[0];
            elem.value = arguments[0];
        }
    },
    remove: function () {
        for (var i = 0; i < this._selection.length; ++i) {
            var elem = this._selection[i];
            elem.parentNode.removeChild(elem);
        }
        return this;
    },
    append: function (elem) {
        for (var i = 0; i < this._selection.length; ++i) {
            var parent = this._selection[i];
            var child = elem._selection[0];
            parent.appendChild(child);
        }
        return this;
    },
    _hide : function(e) {
        e.setAttribute("display", "none");
    },
    _show : function(e) {
        e.setAttribute("display", "block");
    },
    hide: function () {
        for (var i = 0; i < this._selection.length; ++i) {
            var e = this._selection[i];
            this._hide(e);
        }
    },
    show: function () {
        for (var i = 0; i < this._selection.length; ++i) {
            var e = this._selection[i];
            this._show(e);
        }
    },
    toggle: function () {
        for (var i = 0; i < this._selection.length; ++i) {
            var e = this._selection[i];
            var t = e.getAttribute("display");
            if (t == "none")
                this._show(e);
            else
                this._hide(e);
        }
    },

    _psuedoAttributes : {},
};
