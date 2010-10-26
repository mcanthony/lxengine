
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

// Add a temporary cube to drop from the sky
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

