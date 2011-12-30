
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

        var r = .4;
        return (_v.lengthSqrd(t1) < r * r) ? 1 : 0;
    };

    NS.tile = function(uv)
    {
        var t = _v.fract(uv);
        t = _v.abs(_v.sub([.5, .5], t));

        return (t[0] < .1 || t[1] < .1) ? 0 : 1;
    };

    NS.diamond = function(uv)
    {
        var t = _v.fract(uv);
        t = _v.abs(_v.sub([.5, .5], t));

        var r = .4;
        return (t[0] + t[1] > r) ? 0 : 1;
    };

    NS.wave = function(uv)
    {
        uv = _v.fract(uv);

        uv[1] += .15 * Math.sin(2 * uv[0] * Math.PI);
        uv[1] = Math.abs(.5 - uv[1]);

        return (uv[1] < .15) ? 0 : 1;
    };

    NS.star = function(uv)
    {
        var uv = _v.abs( _v.sub([.5,.5], _v.fract(uv) ) );
        uv[0] = Math.log(Math.E * (Math.abs(uv[0]) + .5));
        uv[1] = Math.log(Math.E * (Math.abs(uv[1]) + .5));
        
        var r = .52;
        return uv[0] * uv[1] > r * r ? 0 : 1;
    };
    
    NS.test = function(uv)
    {
        var uv = _v.abs( _v.sub([.5,.5], _v.fract(uv) ) );
        uv[0] = Math.pow((Math.abs(uv[0]) + .5), .7);
        uv[1] = Math.pow((Math.abs(uv[1]) + .5), 1.5);
        return uv[0] * uv[1] > .65 * Math.cos(uv[0]) ? 0 : 1;
    };

    return NS;
})(lx.patterns);
