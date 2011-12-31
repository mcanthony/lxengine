
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

    NS.spotwave = function(uv)
    {
        return NS.spot(uv) * NS.wave(uv);
    };

    NS.spotdiamondxor= function(uv)
    {
        var t = NS.spot(_v.scale(uv, 2)) + NS.diamond(uv);
        return (t != 1) ? 1 : 0;
    };

    NS.star = function(uv)
    {
        var uv = _v.abs( _v.sub([.5,.5], _v.fract(uv) ) );
        uv[0] = Math.log(Math.E * (Math.abs(uv[0]) + .5));
        uv[1] = Math.log(Math.E * (Math.abs(uv[1]) + .5));
        
        var r = .52;
        return uv[0] * uv[1] > r * r ? 0 : 1;
    };
    
    NS.ribbon = function(uv)
    {
        uv = _v.fract(uv);

        uv[1] += .15 * Math.sin(2 * uv[0] * Math.PI);
        uv = _v.abs(_v.sub([.5, .5], uv));

        var r = (uv[0] + .1) * .15;
        return (uv[1] < r) ? 0 : 1;
    }

    NS.misc1 = function(uv)
    {
        var uv = _v.abs( _v.sub([.5,.5], _v.fract(uv) ) );
        uv[0] = Math.pow((Math.abs(uv[0]) + .5), .7);
        uv[1] = Math.pow((Math.abs(uv[1]) + .5), 1.5);
        return uv[0] * uv[1] > .65 * Math.cos(uv[0]) ? 0 : 1;
    };

    NS.weave = function(uv)
    {
        var s = uv[0] * 2;
        var t = uv[1] * 2;

        // Extract the fractional and integer parts
        var fs = s - Math.floor(s);
        var ft = t - Math.floor(t);
        var is = s - fs;
        var it = t - ft;

        // Switch the fractional part to a distance
        // from the tile center
        fs = Math.abs(.5 - fs);
        ft = Math.abs(.5 - ft);

        // Create a smooth curve from .5 to 1.0
        ds = Math.cos(fs * Math.PI);
        ds = Math.pow(ds, .75);
        ds = .5 + ds / 2.0;

        dt = Math.cos(ft * Math.PI);
        dt = Math.pow(dt, .75);
        dt = .5 + dt / 2.0;

        // Invert the over/under on every other tile
        if ((is + it) % 2 == 0) {
            var tmp = fs;
            fs = ft;
            ft = tmp;

            tmp = ds;
            ds = dt;
            dt = tmp;

        }

        // Invert the color of the lower strip
        ds = 1.0 - ds;

        if (fs < .4)
            return 1 - dt;
        else if (ft < .4)
            return 1 - ds;
        else
            return 1;
    }

    NS.test = function(uv)
    {
        var t = NS.tile(_v.scale(uv, 1)) || NS.spot(_v.scale(uv, 8));
        return t;
    }

    return NS;
})(lx.patterns);