(function() {
    var mesh = new HalfEdgeMesh(createBox());
    mesh.integrityCheck();
    return mesh.createQuadMesh();
})();