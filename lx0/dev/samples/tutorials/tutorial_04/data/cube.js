(function() {
    var mesh = new HalfEdgeMesh(createBox());
    mesh.integrityCheck();
    mesh.smoothVertex(mesh._vertices[6], .25);
    mesh.integrityCheck();    
    return mesh.createPolyMesh().createTriMesh();
})();