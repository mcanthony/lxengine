
try { eval("lx"); } catch (e) { lx = {}; }
lx.vec = (function () {

    var NS = {};

    //===========================================================================//
    // Settings
    //===========================================================================//

    NS._debug = true;

    //===========================================================================//
    // Vec functions
    //===========================================================================//

    NS.cross = function (u, v) {
        return [
            u[1] * v[2] - v[1] * u[2],
            u[2] * v[0] - v[2] * u[0],
            u[0] * v[1] - v[0] * u[1],
        ];
    };

    NS.length = function (u) {
        if (u[2] > -Infinity && u[2] < Infinity)
            return Math.sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
        else
            return Math.sqrt(u[0] * u[0] + u[1] * u[1]);
    };

    NS.lengthSqrd = function (u) {
        if (u[2] > -Infinity && u[2] < Infinity)
            return (u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
        else
            return (u[0] * u[0] + u[1] * u[1]);
    };

    NS.normalize = function (u) {
        var len = NS.length(u);
        return [
            u[0] / len,
            u[1] / len,
            u[2] / len
        ];
    };

    NS.sub = function (u, v) {
        return [
            u[0] - v[0],
            u[1] - v[1],
            u[2] - v[2]
        ];
    };

    NS.addVec = function () {
        var sum = arguments[0].slice(0);
        for (var i = 1; i < arguments.length; ++i) {
            sum[0] += arguments[i][0];
            sum[1] += arguments[i][1];
            sum[2] += arguments[i][2];
        }
        return sum;
    };

    NS.add = NS.addVec;

    NS.mulScalar = function (u, s) {
        return [
            u[0] * s,
            u[1] * s,
            u[2] * s,
        ];
    };

    NS.scale = NS.mulScalar;
    NS.mul = function (u, v) {
        return [
            u[0] * v[0],
            u[1] * v[1],
            u[2] * v[2],
        ];
    };

    NS.abs = function (u) {
        return [
            Math.abs(u[0]),
            Math.abs(u[1]),
            Math.abs(u[2]),
        ];
    };

    NS.floor = function (u) {
        return [
            Math.floor(u[0]),
            Math.floor(u[1]),
            Math.floor(u[2]),
        ];
    };

    NS.fract = function (u) {
        return [
            u[0] - Math.floor(u[0]),
            u[1] - Math.floor(u[1]),
            u[2] - Math.floor(u[2])
        ];
    };

    NS.valid = function (u) {
        for (var i = 0; i < 3; ++i) {
            if (isNaN(u[i]))
                return false;
        }
        return true;
    };

    NS.assertValid = function (u) {
        var ok = lib.valid(u);
        if (!ok) {
            alert("Assertion failed: " + u);
        }
    };

    NS.dot = function (u, v) {
        return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
    };

    NS.floor = function (u) {
        return [Math.floor(u[0]), Math.floor(u[1]), Math.floor(u[2])];
    };

    NS.clamp = function (u, min, max) {
        return [
            Math.min(max, Math.max(u[0], min)),
            Math.min(max, Math.max(u[1], min)),
            Math.min(max, Math.max(u[2], min))
        ];
    };

    //===========================================================================//
    // Intersection Functions
    //===========================================================================//

    NS.intersectPlane = function (ray, plane) {

        var n = plane.normal;
        var d = plane.d;
        var o = ray.origin;
        var v = ray.direction;

        var cs = NS.dot(n, v);
        if (cs != 0) {
            var dot_no = NS.dot(n, o);
            var t = -(dot_no + d) / cs;
            if (t >= 0) {
                var intersection = {};
                intersection.distance = t;
                intersection.normal = n;
                intersection.position = NS.addVec(o, NS.mulScalar(v, t));
                return intersection;
            }
        }
        return false;
    };

    NS.intersectSphere = function (ray, sphere) {

        var u = NS.sub(sphere._center, ray.origin);
        var v = NS.normalize(ray.direction);

        var dotUV = NS.dot(u, v);
        if (dotUV < 0)
            return false;

        var d_sqrd = NS.lengthSqrd(u) - dotUV * dotUV;
        var r_sqrd = sphere._radius * sphere._radius;

        if (d_sqrd > r_sqrd)
            return false;

        var dp = Math.sqrt(r_sqrd - d_sqrd);
        var dr = [
            dotUV - dp,
            dotUV + dp
        ];

        function hit(d) {
            var intersect = {};
            intersect.distance = d;
            intersect.position = NS.addVec(ray.origin, NS.mulScalar(ray.direction, d));
            intersect.normal = NS.normalize(intersect.position);
            return intersect;
        }

        if (dr[0] < 0)
            if (dr[1] < 0)
                return false;
            else
                return hit(dr[1]);
        else
            if (dr[1] < 0)
                return hit(dr[0]);
            else
                return hit(Math.min(dr[0], dr[1]));
    };


    //===========================================================================//
    // Types
    //===========================================================================//

    NS.Light = function (opts) {
        if (opts) for (var key in opts)
            this[key] = opts[key];
    };

    NS.Sphere = function (opts) {
        this._radius = opts.radius || 0;
        this._center = opts.center || [0, 0, 0];

        if (opts) for (var key in opts) {
            if (key != "center" && key != "radius")
                this[key] = opts[key];
        }
    };
    NS.Sphere.prototype.intersect = function (ray) { return NS.intersectSphere(ray, this); };
    NS.Sphere.prototype.center = function () { return this._center; }

    NS.Plane = function (opts) {
        this.normal = [0, 0, 1];
        this.d = 0;

        if (opts) for (var key in opts)
            this[key] = opts[key];
    };

    NS.Plane.prototype.intersect = function (ray) { return NS.intersectPlane(ray, this); };
    NS.Plane.prototype.center = function () {
        return _lxbb_mul_vec2_float(this.normal, -this.d);
    };

    return NS;
})();


(function () {

    function addBasicFunction(name)
    {
        var code = "";
        code += "_lxbb_%name%_vec2 = function (v) { return [ Math.%name%(v[0]), Math.%name%(v[1]) ]; };".replace(/%name%/g, name);
        code += "_lxbb_%name%_vec3 = function (v) { return [ Math.%name%(v[0]), Math.%name%(v[1]), Math.%name%(v[2]) ]; };".replace(/%name%/g, name);
        code += "_lxbb_%name%_vec4 = function (v) { return [ Math.%name%(v[0]), Math.%name%(v[1]), Math.%name%(v[2]), Math.%name%(v[3]) ]; };".replace(/%name%/g, name);
        eval(code);
    }
    function addBasicFunctions(nameList)
    {
        var names = nameList.split(',');
        for (var i = 0; i < names.length; ++i)
            addBasicFunction(names[i]);
    }
    addBasicFunctions("abs,floor,sqrt,log,sin,cos,tan,asin,acos");

    _lxbb_copy_vec2 = function (v) { return [v[0], v[1]]; }
    _lxbb_copy_vec3 = function (v) { return [v[0], v[1], v[2]]; }

    _lxbb_fract_float = function(v) { return v - Math.floor(v); }
    _lxbb_fract_vec2 = function(v) { return [ v[0] - Math.floor(v[0]), v[1] - Math.floor(v[1]) ]; }

    _lxbb_add_vec2       = function (u, v) { return [ u[0] + v[0], u[1] + v[1] ]; }
    _lxbb_sub_vec2       = function (u, v) { return [ u[0] - v[0], u[1] - v[1] ]; }
    _lxbb_sub_vec3       = function (u, v) { return [ u[0] - v[0], u[1] - v[1], u[2] - v[2] ]; }
    _lxbb_mul_vec2       = function (v, u) { return [ v[0] * u[0], v[1] * u[1] ]; }    
    _lxbb_mul_vec2_float = function (v, s) { return [ v[0] * s, v[1] * s ]; }
    _lxbb_mul_float_vec2 = function (s, v) { return [ v[0] * s, v[1] * s ]; }
    _lxbb_div_vec2_float = function (v, s) { return [ v[0] / s, v[1] / s ]; }

    _lxbb_pow_vec2_float = function(u, e) { return [Math.pow(u[0], e),  Math.pow(u[1], e) ]; }

    _lxbb_normalize_vec2 = function(v) { var d = _lxbb_length_vec2(v); return [v[0]/d,v[1]/d]; }
    _lxbb_normalize_vec3 = function(v) { var d = _lxbb_length_vec3(v); return [v[0]/d,v[1]/d,v[2]/d]; }

    _lxbb_dot_vec2_vec2 = function(u, v) { return u[0] * v[0] + u[1] * v[1]; }
    _lxbb_length_vec2 = function(v) { return Math.sqrt(v[0] * v[0] + v[1] * v[1]); }
    _lxbb_length_vec3 = function(v) { return Math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]); }
    _lxbb_lengthSqrd_vec2 = function(v) { return v[0] * v[0] + v[1] * v[1]; }

})();

