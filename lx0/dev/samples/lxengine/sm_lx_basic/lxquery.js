var $ = function (selector) {
    var q = new LxQuery();
    return q.init(selector);
};

var LxQuery = function () {};
LxQuery.prototype = {
    init : function (selector) {
        var re_singleTag = /^<(\w+)\s*\/?>(?:<\/\1>)?$/;
        var re_id = /^#(\w+)$/;

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
        
        return this._select([]);
    },
    _selection : [],
    _select : function (e)
    {
        this._selection = e;
        return this;
    },
    attr : function (name, value)
    {
        for (var i = 0; i < this._selection.length; ++i)
        {
            var elem = this._selection[i];
            elem.setAttribute(name, value);
        }
        return this;
    },
    remove : function ()
    {
        for (var i = 0; i < this._selection.length; ++i)
        {
            var elem = this._selection[i];
            elem.parentNode.removeChild(elem);
        }
        return this;
    },
    append : function (elem)
    {
        for (var i = 0; i < this._selection.length; ++i)
        {
            var parent = this._selection[i];
            var child = elem._selection[0];
            parent.appendChild(child);
        }
        return this;
    },
    
};
