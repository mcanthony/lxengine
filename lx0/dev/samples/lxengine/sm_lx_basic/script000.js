
var cube_id = 0;
for (var grid_y = 0; grid_y < 3; grid_y++)
{
    for (var grid_x = 0; grid_x < 3; grid_x++)
    {
        var tr = [];
        tr[0] = (grid_x - 1) * 1.5;
        tr[1] = (grid_y - 1) * 1.5;
        tr[2] = 0.5;
       
        var ref = $("<Ref/>");
        ref.attr("id", "cube" + (cube_id++));
        ref.attr("ref", "unit_cube");
        ref.attr("translation", tr);
        $("#grid").append(ref);
    }
}

var ref = $("<Ref/>");
ref.attr("ref", "unit_cube");
ref.attr("translation", [.25, -.5, 5.5]);
ref.attr("mass", 1.0);
$("#grid").append(ref);

ref = $("<Ref/>");
ref.attr("ref", "unit_cube");
ref.attr("translation", [-.65, .75, 8.5]);
ref.attr("mass", 1.0);
$("#grid").append(ref);

ref = $("<Ref/>");
ref.attr("ref", "unit_sphere");
ref.attr("translation", [.29, -.47, 16.5]);
ref.attr("mass", 2.5);
ref.attr("bounds_type", "sphere");
$("#grid").append(ref);

for (var gy = 0; gy < 5; ++gy) {
    for (var gx = 0; gx < 5; ++gx) {

        var px = -1.7 + .1 * gx;
        var py = 1.2;
        var pz = 6 + 0.8 * (gy * 5 + gx);

        ref = $("<Ref/>");
        ref.attr("ref", "small_sphere");
        ref.attr("translation", [px, py, pz]);
        ref.attr("mass", .01);
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

            var pos = [
                Math.random() * 6 - 3,
                Math.random() * 6 - 3,
                12
            ];
            ref = $("<Ref/>");
            ref.attr("ref", "small_sphere");
            ref.attr("translation", pos);
            ref.attr("bounds_type", "sphere");
            ref.attr("mass", .01);
            ref.attr("restitution", 0.8);
            $("#grid").append(ref);

            rain.live.push([ rain.time + 8000, ref]);
        }
        rain.time += rain.freq;
        window.setTimeout(rain.freq, rain.drop);
    },
};



window.setTimeout(10000, rain.drop);

window.setTimeout(6000, function () {
    var ref = $("<Ref/>");
    ref.attr("ref", "unit_sphere");
    ref.attr("translation", [-.25, .65, 12.5]);
    ref.attr("mass", 5.5);
    ref.attr("bounds_type", "sphere");
    $("#grid").append(ref);
});

window.onKeyDown = function (e) {

    if (e.keyCode >= 1 && e.keyCode <= 9) {
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
        // TBD: Cycle through wind intensity
    }
};