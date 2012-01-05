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
            xAxis : lx.vec.scale(right, width),
            yAxis : lx.vec.scale(up, height),
        };
        f.origin = lx.vec.addVec(
            worldPosition, 
            lx.vec.mulScalar(forward, nearDistance), 
            lx.vec.mulScalar(right, -width / 2),
            lx.vec.mulScalar(up, -height / 2)
        );

        return f;
    }

    var shaders = (function() {
        
        var _v = lx.vec;
        Object.freeze(_v);

        return {

            spherical : function (fragPositionOc, scale)
            {
                var r = _v.length(fragPositionOc);
                var phi = Math.atan2(fragPositionOc[1], fragPositionOc[0]);
                var theta = Math.acos(fragPositionOc[2] / r);

                return [
                    scale[0] * (phi + Math.PI) / (2 * Math.PI),
                    scale[1] * theta / Math.PI,
                ];
            },

            attenuation : function(fragPosition, lightPosition, scale)
            {
                var L = _v.sub(lightPosition, fragPosition);
                var d = _v.length(L);
                
                return 1.0 / _v.dot(scale, [ 1, d, d*d ]);
            },
            
            specular : function(eyePosition, fragPosition, fragNormal, lightPosition, specularEx) {

                var L = _v.sub(lightPosition, fragPosition);
                var d = _v.length(L);
                L = _v.scale(L, 1/d);

                var V = _v.normalize( _v.sub(eyePosition, fragPosition) );
                var H = _v.normalize( _v.add(L, V) );
                var term = Math.pow(Math.max(_v.dot(fragNormal,H), 0.0), specularEx);
                
                return term;
            },

            diffuse : function (fragPosition, fragNormal, lightPosition)
            {
                var L = _v.sub(lightPosition, fragPosition);
                var d = _v.length(L);
                L = _v.scale(L, 1/d);

                var nDotL = _v.dot(fragNormal, L);
                return Math.max(0, nDotL);
            }
        };
    })();

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


    NS.RenderLoop = lx.core.buildClass(State, function(options) 
    {
        this._sampleCount = options.sampleCount; 
        this._sceneUrl = options.sceneUrl || "scene_00.xml";

        if (this._sampleCount > 1)
            this._renderPixel = NS.RenderLoop.prototype._renderPixel5;
        else
            this._renderPixel = NS.RenderLoop.prototype._renderSample;
    }, 
    {
        _canvas : null,
        _ctx : null,
        _width : null,
        _height : null,
        _cache : {},

        _acquireShader : (function()
        {
            var cache = {};
            return function(text) {
                var compiled = cache[text]; 
                if (!(text in cache))
                {
                    var source = text;
                    source = source.replace(/fragment\./g, "state.fragment.");
                    eval("compiled = function(state) { " + source + " };");
                    cache[text] = compiled;
                }
                return compiled; 
            };
        })(),

        _traceRay : function (ray) {

            var scene = {};
            var objects = this._objects;
            var lights = this._lights;

            var color = [ 0, 0, 0 ];

            var state = {
                scene : null,
                style : null,
                object : {
                    ref : null,
                    center : null,
                },
                material : null,
                fragment : {
                    position : null,
                    normal : null,
                },
                context : {
                },
                program : {
                },
            };
            


            var isect = intersect(ray, objects);
            if (isect)
            {
                var obj = isect.object;

                state.scene = scene;
                
                state.object.ref = isect.object;
                state.object.center = isect.object.center();

                state.fragment.position = isect.position;
                state.fragment.positionWc = isect.position;
                state.fragment.positionOc = _lxbb_sub_vec3(state.fragment.positionWc, state.object.center);
                state.fragment.normal = isect.normal;

                state.context.diffuse = obj.diffuse || state.scene.diffuse || [1, 1, 1];
                if (typeof state.context.diffuse == "string") state.context.diffuse = this._acquireShader(state.context.diffuse)(state);

                var mat = {};
                mat.diffuse = state.context.diffuse;

                mat.specular = isect.object.specular || mat.diffuse || [1, 1, 1];
                mat.specEx = isect.object.specularExponent || 32.0;

                for (var i = 0; i < lights.length; ++i)
                {
                    var light = lights[i];
                    var lightRay = {
                        origin : isect.position,
                        direction : lx.vec.normalize( lx.vec.sub(lights[i].position, isect.position) ),
                    };
                    lightRay.origin = lx.vec.addVec(lightRay.origin, lx.vec.mulScalar(lightRay.direction, 0.01));

                    var lightSect = intersect(lightRay, objects);
                    var shadowTerm = (!lightSect || (lightSect.distance > isect.distance)) ? 1 : 0.25;

                    var intensity = light.intensity || 1.0;

                    if (shadowTerm > 0)
                    {
                        var attenuationScale = [1, 0, 0];
                        var attenuation = shaders.attenuation(isect.position, lights[i].position, attenuationScale);

                        var diffuse = shaders.diffuse(isect.position, isect.normal, lights[i].position);
                        var specular = shaders.specular(ray.origin, isect.position, isect.normal, lights[i].position, mat.specEx); 
                        var c = lx.vec.add( 
                            lx.vec.scale(mat.diffuse, diffuse), 
                            lx.vec.scale(mat.specular, specular) 
                        );
                        c = lx.vec.scale(c, shadowTerm * intensity * attenuation);
                        color = lx.vec.addVec(color, c);
                    }
                }
            }

            return color;
        },

        _renderSample : function(frustum, x, y) {
            var ray = {};
            ray.origin = frustum.eye;                            
            ray.destination = lx.vec.addVec(
                frustum.origin,
                lx.vec.scale(frustum.xAxis, x / (this._width - 1)),
                lx.vec.scale(frustum.yAxis, y / (this._height- 1))
            );
            ray.direction = lx.vec.normalize( lx.vec.sub(ray.destination, ray.origin) );
                
            return this._traceRay(ray);
        },

        _renderPixel : function(frustum, x, y) {
            return this._renderSample(frustum, x, y);
        },

        _renderPixel5 : function(frustum, x, y) {

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
            return lx.vec.scale(color, 1 / sampleSet.length);
        },

        _renderRow : function(frustum, y) {

            var ctx = this._ctx;


            var rowData = ctx.createImageData(this._width, 1);
            var pixels = rowData.data;

            for (var x = 0; x < this._width; ++x) {

                var color = this._renderPixel(frustum, x, y);
                color = lx.vec.clamp(color, 0.0, 1.0);
                color = lx.vec.floor( lx.vec.scale(color, 255) );

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
            this._camera = {};
            this._objects = [];
            this._lights = [];

            var _this = this;
            $.ajax(this._sceneUrl + "?rand=" + Math.random(), {
                dataType : "xml",
                async : false,
                success : function(xml) {
                    $(xml).find("Text").each(function() {
                        var text = $(this).text();
                        text = text.replace(/\n/g, " ");
                        text = text.replace(/\"/g, "\\\"");
                        text = "\"" + text + "\""; 
                        $(this).replaceWith(text);
                    });
                    $(xml).find("Objects").children().each(function() {
                        switch (this.tagName)
                        {
                        case "Camera":
                            {
                                eval("var options = " + $(this).text() + ";");
                                _this._camera = options;
                            }
                            break;
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

            var cam = this._camera;
            var direction = lx.vec.sub(cam.target, cam.position);
            var frustum = NS.calculateFrustum(cam.position, direction, [0, 0, 1], .01, Math.PI / 6, this._width / this._height);

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
