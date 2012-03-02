(function() {
    lx0.message("Creating mesh...");
    var mesh = new HalfEdgeMesh(createBox());
    
	
	var lineList = new LineList();
	mesh.iterateEdges(function (edge) {
		if (!edge._mark)
		{		
			lineList.addSegment(
				edge.vertex.position,
				edge.opposite.vertex.position
			);
			edge._mark = true;
			edge.opposite._mark = true;
		}
	});
	mesh.iterateEdges(function (edge) {
		delete edge._mark;
	});    
    return lineList;
})();