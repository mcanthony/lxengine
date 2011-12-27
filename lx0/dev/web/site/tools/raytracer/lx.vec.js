
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
        return Math.sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
    };

    NS.lengthSqrd = function (u) {
        return (u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
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

    NS.mul = NS.mulScalar;

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

        var u = NS.sub(sphere.center, ray.origin);
        var v = NS.normalize(ray.direction);

        var dotUV = NS.dot(u, v);
        if (dotUV < 0)
            return false;

        var d_sqrd = NS.lengthSqrd(u) - dotUV * dotUV;
        var r_sqrd = sphere.radius * sphere.radius;

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
        this.radius = 0;
        this.center = [0, 0, 0];

        if (opts) for (var key in opts)
            this[key] = opts[key];
    };
    NS.Sphere.prototype.intersect = function (ray) { return NS.intersectSphere(ray, this); };

    NS.Plane = function (opts) {
        this.normal = [0, 0, 1];
        this.d = 0;

        if (opts) for (var key in opts)
            this[key] = opts[key];
    };

    NS.Plane.prototype.intersect = function (ray) { return NS.intersectPlane(ray, this); };

    return NS;
})();