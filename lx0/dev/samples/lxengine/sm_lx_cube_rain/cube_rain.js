
var cube_id = 0;
for (var grid_y = 0; grid_y < 3; grid_y++)
{
    for (var grid_x = 0; grid_x < 3; grid_x++)
    {
        var color = [];
        color[0] = 0.8;
        color[1] = 0.5 + 0.5 * (grid_y / 3.0);
        color[2] = 0.6 + 0.2 * (grid_x / 3.0);

        var tr = [];
        tr[0] = (grid_x - 1) * 1.5;
        tr[1] = (grid_y - 1) * 1.5;
        tr[2] = 0.5;
       
        var ref = $("<Ref/>");
        ref.attr("id", "cube" + (++cube_id));
        ref.attr("ref", "unit_cube");
        ref.attr("translation", tr);
        ref.attr("color", color);
        $("#grid").append(ref);
    }
}

var ref = $("<Ref/>");
ref.attr("ref", "unit_cube");
ref.attr("translation", [.25, -.5, 5.5]);
ref.attr("mass", 1.0);
ref.attr("color", [255 / 255.0, 40 / 255.0, 10 / 255.0]);
ref.attr("material", "checker");
$("#grid").append(ref);

ref = $("<Ref/>");
ref.attr("ref", "unit_cube");
ref.attr("translation", [-.65, .75, 8.5]);
ref.attr("mass", 1.0);
$("#grid").append(ref);

ref = $("<Ref/>");
ref.attr("ref", "unit_sphere2");
ref.attr("translation", [.29, -.47, 16.5]);
ref.attr("mass", 2.1);
ref.attr("bounds_type", "sphere");
ref.attr("material", "phong { ambient : [0, 0, 0], diffuse : [.9, .4, .3], specular : [.4, .4, .9] }");
$("#grid").append(ref);

for (var gy = 0; gy < 5; ++gy) {
    for (var gx = 0; gx < 5; ++gx) {

        var px = -1.7 + .1 * gx;
        var py = 1.2;
        var pz = 6 + 0.8 * (gy * 5 + gx);

        ref = $("<Ref/>");
        ref.attr("ref", "small_sphere");
        ref.attr("translation", [px, py, pz]);
        ref.attr("mass", .1);
        ref.attr("bounds_type", "sphere");
        $("#grid").append(ref);
    }
}

var rain = {
    time : 0,
    freq : 100,
    live : [],
    drop : function () {
        if (rain.freq > 0) {

            for (var i = 0; i < rain.live.length;) {
                if (rain.live[i][0] < rain.time) {
                    rain.live[i][1].remove();
                    rain.live.shift();
                }
                else
                    ++i;
            }
            var shin = 1;
            var spec = [ 0, 0, 0 ];
            var color = Math.random();
            if (color < .15)
                color = [ 22.0 / 255.0, 120.0 / 255.0, 181.0 / 255.0 ];
            else if (color < .3) {
                color = [ 96.0 / 255.0, 203.0 / 255.0, 175.0 / 255.0 ];
                spec = [0, 0, .5 ];
                shin = 32;
            }
            else if (color < .45) {
                color = [ 181.0 / 255.0, 170.0 / 255.0, 246.0 / 255.0 ];
                spec = [ .5, .5, 0];
                shin = 128;
            }
            else {
                color = [ 51.0 / 255.0, 88.0 / 255.0, 249.0 / 255.0 ]; 
                spec = [ .5, .5, .5];
                shin = 256;
            }   

            var pos = [
                Math.random() * 6 - 3,
                Math.random() * 6 - 3,
                11
            ];
            ref = $("<Ref/>");
            ref.attr("ref", "small_sphere");
            ref.attr("translation", pos);
            ref.attr("bounds_type", "sphere");
            ref.attr("color", color);
            ref.attr("specular", spec);
            ref.attr("shininess", shin)
            ref.attr("mass", .1 + Math.random() * .5);
            ref.attr("restitution", 0.4);
            $("#grid").append(ref);

            rain.live.push([ rain.time + 9000, ref]);
        }
        rain.time += rain.freq;
        window.setTimeout(rain.freq, rain.drop);
    },
};

function fadeIn()
{
    var scene = $("Scene");
    var fade = scene.attr("fade");
    if (fade > 0.01)
    {
        fade -= 0.025;
        scene.attr("fade", fade);
        window.setTimeout(50, fadeIn);
    }
    else
        scene.attr("fade", 0);
}
window.setTimeout(2000, fadeIn);

window.setTimeout(10000, function() {
    rain.drop();
});

window.setTimeout(6000, function () {
    var ref = $("<Ref/>");
    ref.attr("ref", "unit_sphere");
    ref.attr("translation", [-.25, .65, 12.5]);
    ref.attr("mass", 5.5);
    ref.attr("bounds_type", "sphere");
    ref.attr("color", [255 / 255.0, 187 / 255.0, 56 / 255.0]);
    ref.attr("material", "solid");
    $("#grid").append(ref);
});

window.setTimeout(4500, function() {
    $("#background_music").attr("sound_state", "playing");
});

var wind_cycle = 0;

window.onKeyDown = function (e) {

    if (e.keyCode >= 2 && e.keyCode <= 10) {
        $("#bounce").attr("sound_state", "playing");
        $("#cube" + e.keyChar).toggle();
    }
    else if (e.keyChar == "r") {
        // Cycle through cube rain intensity
        var tr = {};
        tr[1000] = 600;
        tr[600] = 200;
        tr[200] = 100;
        tr[100] = 0;
        tr[0] = 1000;
        rain.freq = tr[rain.freq];
        __lx_print("Rain frequency: " + rain.freq);
    }
    else if (e.keyChar == "w") {
        var tr = [
            "none",
            "calm",
            "light air",
            "light breeze",
            "gentle breeze",
            "moderate breeze",
            "fresh breeze",
            "strong breeze",
            "near gale",
            "gale",
            "strong gale",
            "whole gale",
            "storm",
            "hurricane"
        ];
        wind_cycle = (++wind_cycle) % tr.length;
        var current = tr[wind_cycle];
        $("Scene").attr("wind", current + " west");
        __lx_print("Wind: " + current + " " + $("Scene").attr("wind_velocity") + " m/s");
    }
};
