function import_libCore(NS) {

    NS.blend = function (a, b, t) {
        return a * (1 - t) + b * t;
    };

    NS.easeIn = function (x) { return Math.sin(x * Math.PI / 2); };
    NS.easeIn2 = function (x) { NS.easeIn(NS.easeIn(x)); };

    NS.each = function (x, f) {
        for (var i in x) {
            f.call(x[i], i, x[i]);
        }
    };
}
