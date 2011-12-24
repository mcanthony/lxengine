try { eval("lx"); } catch (e) { lx = {}; }
lx.core = lx.core || {};

(function (NS) {

    //=======================================================================//
    // Math functions
    //=======================================================================//

    NS.random = function (min, max) {
        return Math.random() * (max - min) + min;
    };

    NS.clamp = function (i, n, m) {
        return Math.max(Math.min(i, m), n);
    };

    NS.blend = function (a, b, t) {
        return a * (1 - t) + b * t;
    };

    NS.easeIn = function (x) { return Math.sin(x * Math.PI / 2); };

    NS.easeIn2 = function (x) { NS.easeIn(NS.easeIn(x)); };

    //=======================================================================//
    // String functions
    //=======================================================================//

    NS.trim = function (s) {
        return s.replace(/^\s+|\s+$/g, "")
    };

    //=======================================================================//
    // Array functions
    //=======================================================================//

    NS.each = function (x, f) {
        for (var i in x) {
            f.call(x[i], i, x[i]);
        }
    };

    //=======================================================================//
    // Code Generation
    //=======================================================================//

    NS.buildClass = function (parent, ctor, members) {

        NS.each(parent.prototype, function (key, value) {
            ctor.prototype[key] = value;
        });
        NS.each(members, function (key, value) {
            ctor.prototype[key] = value;
        });

        return ctor;
    };

    import_libCore = function (NS) {
        return NS;
    };

})(lx.core);

