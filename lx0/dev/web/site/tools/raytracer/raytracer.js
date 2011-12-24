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

    function intersect(ray, objects)
    {
        var isect = undefined;
        for (var i = 0; i < objects.length; ++i)
        {
            var isect2 = objects[i].intersect(ray);
            if (isect2 && (!isect || isect2.distance < isect.distance)) {
                isect = isect2;
                isect.object = objects[i];
            }
        }
        return isect;
    } 


    NS.RenderLoop = lx.core.buildClass(State, function() { }, {
                
        _canvas : null,
        _ctx : null,
        _width : null,
        _height : null,

        _traceRay : function (ray) {

            var objects = this._objects;
            var lights = this._lights;

            var color = [ 0, 0, 0 ];

            var isect = intersect(ray, objects);
            if (isect)
            {
                var mat = {};
                mat.diffuse = isect.object.diffuse || [1, 1, 1];

                for (var i = 0; i < lights.length; ++i)
                {
                    var light = lights[i];
                    var lightRay = {
                        origin : isect.position,
                        direction : lx.vec.normalize( lx.vec.sub(lights[i].position, isect.position) ),
                    };
                    lightRay.origin = lx.vec.addVec(lightRay.origin, lx.vec.mulScalar(lightRay.direction, 0.01));

                    var lightSect = intersect(lightRay, objects);
                    var shadowTerm = (!lightSect || (lightSect.distance > isect.distance)) ? 1 : 0.2;

                    var intensity = light.intensity || 1.0;

                    var diffuse = shadowTerm * intensity * shaders.diffuse(isect.position, isect.normal, lights[i].position);
                    var c = lx.vec.mulScalar(mat.diffuse, diffuse);
                    color = lx.vec.addVec(color, c);
                }
            }

            return color;
        },

        _renderSample : function(frustum, x, y) {
            var ray = {};
            ray.origin = frustum.eye;                            
            ray.destination = lx.vec.addVec(
                frustum.origin,
                lx.vec.mulScalar(frustum.xAxis, x / (this._width - 1)),
                lx.vec.mulScalar(frustum.yAxis, y / (this._height- 1))
            );
            ray.direction = lx.vec.normalize( lx.vec.sub(ray.destination, ray.origin) );
                
            return this._traceRay(ray);
        },

        _renderPixel : function(frustum, x, y) {

            var sampleSet = [
                [ -.25, -.25 ],
                [ -.25, .25 ],
                [ .25, -.25 ],
                [ .25, .25 ],
                [ 0, 0, ]
            ];

            var color = [ 0, 0, 0];
            for (var j = 0; j < sampleSet.length; ++j)
            {
                var dx = sampleSet[j][0];
                var dy = sampleSet[j][1];
                               
                var sampleColor = this._renderSample(frustum, x + dx, y + dy);
                color = lx.vec.addVec(color, sampleColor);
            }
            return lx.vec.mulScalar(color, 1 / sampleSet.length);
        },

        _renderRow : function(frustum, y) {

            var ctx = this._ctx;


            var rowData = ctx.createImageData(this._width, 1);
            var pixels = rowData.data;

            for (var x = 0; x < this._width; ++x) {

                var color = this._renderPixel(frustum, x, y);
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

            this._ctx.clearRect(0, 0, this._width, this._height);

            
            this._stats = {};
            this._objects = [];
            this._lights = [];

            var _this = this;
            $.ajax("scene_00.xml", {
                dataType : "xml",
                async : false,
                success : function(xml) {
                    $(xml).find("Objects").children().each(function() {
                        switch (this.tagName)
                        {
                        case "Plane":
                        case "Sphere":
                            {
                                eval("var options = " + $(this).text() + ";");
                                _this._objects.push(new lx.vec[this.tagName](options));
                            }
                            break;
                        case "Light":
                            {
                                eval("var options = " + $(this).text() + ";");
                                _this._lights.push(new lx.vec[this.tagName](options));
                            }
                            break;
                        default:
                            console.log("Unrecognized Object type '" + this.tagName + "'");        
                        }
                    });
                },
            });

            
            this._stats.renderStart = new Date().valueOf();

            var frustum = NS.calculateFrustum([5, 5, 5], [-1, -1, -1], [0, 0, 1], .01, Math.PI / 8, this._width / this._height);

            this._tasks = [];

            for (var y = 0; y < this._height; ++y) {
                this._tasks.push( (function(y) {  return function() { 
                    _this._renderRow(frustum, _this._height - y - 1); 
                };})(y) );
            }
        },

        update : function (wtime) {
                
            if (this._tasks.length)
            {
                for (var i = 0 ; i < 4 && this._tasks.length; ++i)
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
