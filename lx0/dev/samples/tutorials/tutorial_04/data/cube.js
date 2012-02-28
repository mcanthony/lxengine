(function() {
    lx0.message("Creating mesh...");
    var mesh = new HalfEdgeMesh(createBox());
    mesh.integrityCheck();
    
    lx0.message("Smoothing vertex...");    
    mesh.smoothVertex(mesh._vertices[6], .36);
    mesh.integrityCheck();

    lx0.message("Converting to triangle mesh...");            
    var polyMesh = mesh.createPolyMesh();
    polyMesh.integrityCheck();    
    var triMesh = polyMesh.createTriMesh();
    return triMesh;
})();