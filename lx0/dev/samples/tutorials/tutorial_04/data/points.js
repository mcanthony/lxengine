(function() {
    lx0.message("Creating point list...");
    var mesh = new PointList();    
    var step = 0.05;
    for (var z = -.5; z < .5 + step/2; z += step)
	{
		for (var y = -.5; y < .5 + step/2; y += step)
		{
			for (var x = -.5; x < .5 + step/2; x += step)
			{
			    if (   (z < -.5 + step/2 || z > .5 - step/2)
			        || (y < -.5 + step/2 || y > .5 - step/2)
			        || (x < -.5 + step/2 || x > .5 - step/2) )
			    {
				    mesh.addVertex(x, y, z);
				}
			}		
		}
	}
    return mesh.createPrimitiveBuffer();
})();