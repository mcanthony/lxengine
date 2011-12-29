
try { eval("lx"); } catch (e) { lx = {}; }
if (!lx.patterns) lx.patterns = {};

(function (NS) {

    var _v = lx.vec;
    Object.freeze(_v);

    NS.checker = function (uv)
    {
        var t = _v.abs( _v.fract(uv) );
        var s = _v.floor(_v.scale(t, 2));
        return Math.floor((s[0] + s[1]) % 2);
    };

    NS.spot = function (uv)
    {
        var t0 = _v.abs( _v.fract(uv) );
        var t1 = _v.sub([.5,.5], t0);

        var r = .5;
        return (_v.lengthSqrd(t1) < r * r) ? 1 : 0;
    };

    NS.tile = function(uv)
    {
        var t = _v.fract(uv);
        t = _v.abs(_v.sub([.5, .5], t));

        return (t[0] < .1 || t[1] < .1) ? 0 : 1;
    };
    
    NS.test = function(uv)
    {
        var t0 = _v.abs( _v.fract(uv) );
        var t1 = _v.sub([.5,.5], t0);
        t1[0] = .5 - Math.pow(.5 - t1[0], 2);
        t1[1] = .5 - Math.pow(.5 - t1[1], 2);

        var r = .4;
        return (_v.lengthSqrd(t1) < r * r) ? 1 : 0;
    };

    return NS;
})(lx.patterns);
