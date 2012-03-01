(function() {
    lx0.message("Creating point list...");
    var mesh = new PointList();    
    for (var z = -.5; z < .59; z += .1)
	{
		for (var y = -.5; y < .59; y += .1)
		{
			for (var x = -.5; x < .59; x += .1)
			{
				mesh.addVertex(x, y, z);
			}		
		}
	}
    return mesh;
})();