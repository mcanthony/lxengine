(function() {
    lx0.message("Creating mesh...");
    var mesh = new HalfEdgeMesh(createBox());
    mesh.integrityCheck();
    
    mesh.smooth();
    //mesh.smooth();
    //mesh.integrityCheck();

    lx0.message("Converting to triangle mesh...");            
    var polyMesh = mesh.createPolyMesh();
    polyMesh.integrityCheck();    
    var triMesh = polyMesh.createTriMesh();
    triMesh.computeFaceNormals();
    return triMesh.createPrimitiveBuffer();
})();