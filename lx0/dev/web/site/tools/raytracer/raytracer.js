var raytracer = {};
(function(NS) {

    NS.calculateFrustum = function(
        worldPosition,
        viewForward,
        worldUp,
        nearDistance,
        fieldOfView,
        aspectRatio) 
    {
        var forward = lx.vec.normalize(viewForward);
        var right = lx.vec.normalize(lx.vec.cross(viewForward, worldUp));
        var up = lx.vec.normalize(lx.vec.cross(right, forward));

        var width = 2 * nearDistance * Math.tan(fieldOfView / 2);
        var height = width / aspectRatio;

        var f = 
        {
            eye : worldPosition,
            xAxis : lx.vec.mulScalar(right, width),
            yAxis : lx.vec.mulScalar(up, height),
        };
        f.origin = lx.vec.addVec(
            worldPosition, 
            lx.vec.mulScalar(forward, nearDistance), 
            lx.vec.mulScalar(right, -width / 2),
            lx.vec.mulScalar(up, -height / 2)
        );

        return f;
    }

    var shaders =
    {
        diffuse : function() {
                
            var _v = lx.vec;
            Object.freeze(_v);

            return function (fragPosition, fragNormal, lightPosition)
            {
                var L = _v.sub(lightPosition, fragPosition);
                var d = _v.length(L);
                L = _v.mulScalar(L, 1/d);

                var nDotL = _v.dot(fragNormal, L);
                return Math.max(0, nDotL);
            };
        }(),
    };


    NS.RenderLoop = lx.core.buildClass(State, function() { }, {
                
        _canvas : null,
        _ctx : null,
        _width : null,
        _height : null,

        _renderRow : function(frustum, y) {

            var ctx = this._ctx;
            var objects = this._objects;
            var lights = this._lights;

            var rowData = ctx.createImageData(this._width, 1);
            var pixels = rowData.data;

            for (var x = 0; x < this._width; ++x) {

                var ray = {};
                ray.origin = frustum.eye;                            
                ray.destination = lx.vec.addVec(
                    frustum.origin,
                    lx.vec.mulScalar(frustum.xAxis, x / (this._width - 1)),
                    lx.vec.mulScalar(frustum.yAxis, y / (this._height- 1))
                );
                ray.direction = lx.vec.normalize( lx.vec.sub(ray.destination, ray.origin) );

                var isect = undefined;
                for (var i = 0; i < objects.length; ++i)
                {
                    var isect2 = objects[i].intersect(ray);
                    if (isect2 && (!isect || isect2.distance < isect.distance)) {
                    isect = isect2;
                    isect.object = objects[i];
                    }
                }

                var color = [ 0, 0, 0 ];
                if (isect)
                {
                    var mat = {};
                    mat.diffuse = isect.object.diffuse || [1, 1, 1];

                    var diffuse = shaders.diffuse(isect.position, isect.normal, [10, 10, 5]);
                    var c = lx.vec.mulScalar(mat.diffuse, diffuse);
                    color = lx.vec.addVec(color, c);
                }
                color = lx.vec.clamp(color, 0.0, 1.0);
                color = lx.vec.floor( lx.vec.mulScalar(color, 255) );

                pixels[x * 4 + 0] = color[0];
                pixels[x * 4 + 1] = color[1];
                pixels[x * 4 + 2] = color[2];
                pixels[x * 4 + 3] = 255;
            }
            ctx.putImageData(rowData, 0, this._height - y - 1);

        },

        init : function (wtime) {

            this._canvas = $("#canvas")[0];                    
            this._ctx    = this._canvas.getContext('2d');
            this._height = this._canvas.height;
            this._width  = this._canvas.width;

            this._stats = {};
            this._stats.renderStart = new Date().valueOf();

            this._objects = [];
            this._objects.push(
                new lx.vec.Sphere({ radius : 1, center : [1, 1, 1], diffuse : [ .95, .75, .6] }),
                new lx.vec.Plane({ normal : [ 1, 0, 0 ], diffuse : [ 1, .95, .95] }),
                new lx.vec.Plane({ normal : [ 0, 1, 0 ], diffuse : [ .95, 1, .95] }),
                new lx.vec.Plane({ normal : [ 0, 0, 1 ], diffuse : [ .75, .75, 1] })
            );

            this._lights = [];
            this._lights.push(
                [10, 10, 5]
            );

            var frustum = NS.calculateFrustum([5, 5, 5], [-1, -1, -1], [0, 0, 1], .01, Math.PI / 4, 1);

            this._tasks = [];

            var _this = this;
            for (var y = 0; y < this._height; ++y) {
                this._tasks.push( (function(y) {  return function() { 
                    _this._renderRow(frustum, _this._height - y - 1); 
                };})(y) );
            }
        },

        update : function (wtime) {
                
            if (this._tasks.length)
            {
                for (var i = 0 ; i < 8 && this._tasks.length; ++i)
                    (this._tasks.shift())();
            }
            else
                engine.changeState(null);
        },

        draw : function (wtime) {
        },

        shutdown : function(wtime) {

            this._stats.renderEnd = new Date().valueOf();
            $("#renderTime").text("Render time: " + (this._stats.renderEnd - this._stats.renderStart) + "ms");

            var dataHRef = this._canvas.toDataURL("image/png");            $("#download").attr("href", dataHRef);
        },
    });

})(raytracer);
