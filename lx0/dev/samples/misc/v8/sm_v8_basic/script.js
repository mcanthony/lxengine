
for (var grid_y = 0; grid_y < 3; grid_y++)
{
    for (var grid_x = 0; grid_x < 3; grid_x++)
    {
        var tr = [];
        tr[0] = (grid_x - 1) * 1.5;
        tr[1] = (grid_y - 1) * 1.5;
        tr[2] = 0.5;
        $("#grid").append("<Ref ref='unit_cube'/>");
    }
}
