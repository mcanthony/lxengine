(function() {
    lx0.message("Creating mesh...");
    var mesh = new HalfEdgeMesh(createBox());
    mesh.integrityCheck();
    
	for (var i = 0; i < 3; ++i)
	{
		lx0.message("Smoothing iteration " + i + "...");
		mesh.iterateVertices(function (vertex) {
			if (vertex.position[2] > 0)
				vertex.group = 1;
		});
		mesh.iterateVertices(function(vertex) {
			if (vertex.group === 1)
			{
				lx0.message("Smoothing vertex...");
				delete vertex.group;
				mesh.smoothVertex(vertex, .35);
				mesh.integrityCheck();
			}
		});
	}

    lx0.message("Converting to triangle mesh...");            
    var polyMesh = mesh.createPolyMesh();
    polyMesh.integrityCheck();    
    var triMesh = polyMesh.createTriMesh();
    return triMesh;
})();