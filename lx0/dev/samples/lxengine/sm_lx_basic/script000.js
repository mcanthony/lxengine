
for (var grid_y = 0; grid_y < 4; grid_y++)
{
    for (var grid_x = 0; grid_x < 4; grid_x++)
    {
        var tr = [];
        tr[0] = (grid_x - 1) * 1.5;
        tr[1] = (grid_y - 1) * 1.5;
        tr[2] = 0.5;
       
        var ref = $("<Ref/>");
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

var rainDrop = function () {
    var pos = [
        Math.random() * 6 - 3,
        Math.random() * 6 - 3,
        12
    ];
    ref = $("<Ref/>");
    ref.attr("ref", "small_sphere");
    ref.attr("translation", pos);
    ref.attr("mass", .01);
    ref.attr("bounds_type", "sphere");
    $("#grid").append(ref);

    window.setTimeout(100, rainDrop);
}

window.setTimeout(10000, rainDrop);

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
        // TBD: toggle hide/show of grid cube of the given number
        alert("Key " + e.keyChar + " was pressed.");
    }
    else if (e.keyChar == "r") {
        // TBD: Cycle through cube rain intensity
    }
    else if (e.keyChar == "w") {
        // TBD: Cycle through wind intensity
    }
};