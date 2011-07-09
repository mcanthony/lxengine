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
