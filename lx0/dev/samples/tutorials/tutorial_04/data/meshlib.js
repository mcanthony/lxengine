//===========================================================================//
// QuadMesh
//===========================================================================//

function QuadMesh() {
    this._meshType = "QuadMesh";
    this._vertices = [];
    this._faces = [];    
}

QuadMesh.prototype.addVertex = function (x, y, z) {
    this._vertices.push([x, y, z]);
}
QuadMesh.prototype.addNormal = function (x, y, z) {
    if (this._normals === undefined) this._normals = [];
    this._normals.push([x, y, z]);
}
QuadMesh.prototype.addColor = function (x, y, z) {
    if (this._colors === undefined) this._colors = [];
    this._colors.push([x, y, z]);
}
QuadMesh.prototype.addFace = function(i0,i1,i2,i3)
{
    this._faces.push([i0,i1,i2,i3]);
}

//===========================================================================//
// HalfEdgeMesh
//===========================================================================//

var HalfEdgeMesh = (function () {

    function Face() {
        this.edge = null;
    }
    function Edge() {
        this.vertex = null;
        this.face = null;
        this.next = null;
        this.opposite = null;
    }
    function Vertex(x, y, z) {
        this.position = [x, y, z];
        this.edge = null;
    }

    function convertFromQuadMesh(mesh) {

        for (var vi = 0; vi < mesh._vertices.length; ++vi) {
            var v = mesh._vertices[vi];
            this._vertices.push(new Vertex(v[0], v[1], v[2]));
        }

        for (var fi = 0; fi < mesh._faces.length; ++fi) {
            this._faces.push(new Face());

            var face = mesh._faces[fi];
            var edges = [];
            for (var vi = 0; vi < face.length; ++vi)
                edges.push(new Edge());

            for (var vi = 0; vi < face.length; ++vi) {
                var vj = (vi + 1) % face.length;
                edges[vi].vertex = this._vertices[face[vi]];
                edges[vi].face = this._faces[fi];
                edges[vi].next = edges[vj];
                edges[vi].opposite = null;

                edges[vi].vertex.edge = edges[vi];
            }
            for (var i = 0; i < edges.length; ++i)
                this._edges.push(edges[i]);

            this._faces[fi].edge = edges[0];
        }

        //
        // O(N^2) algorithm here: we could do better.
        //
        var edges = this._edges.slice(0);
        for (var i = 0; i < edges.length; i += 2) {
            for (var j = i + 1; j < edges.length; ++j) {
                if (edges[i].vertex === edges[j].next.vertex) {
                    edges[i].opposite = edges[j];
                    edges[j].opposite = edges[i];
                    edges[j] = edges[i + 1];
                    break;
                }
            }
        }
    }

    function HalfEdgeMesh(mesh) {
        this._meshType = "HalfEdgeMesh";
        this._faces = [];
        this._edges = [];
        this._vertices = [];

        if (/function QuadMesh/.test(mesh.constructor))
            convertFromQuadMesh.call(this, mesh);
        else
            throw "Unknown mesh type";
    }

    HalfEdgeMesh.prototype.integrityCheck = function () {
        for (var i = 0; i < this._faces.length; ++i) {
            if (this._faces[i].edge.face !== this._faces[i]) {
                throw "Edge face incorrect";
            }
        }
        for (var i = 0; i < this._edges.length; ++i) {
            if (this._edges[i].opposite.opposite !== this._edges[i]) {
                throw "Edge opposite incorrect";
            }
        }
        for (var i = 0; i < this._vertices.length; ++i) {
            if (this._vertices[i].edge.vertex !== this._vertices[i] && this._vertices[i].edge.next.vertex !== this._vertices[i]) {
                throw "Edge vertex incorrect";
            }
        }
    };
    
    //
    // Interpolates outward from the half-edge's base vertex.
    //
    // A value of 0 returns the position of the base vertex; a 
    // value of 1 will be a duplicate of the position of the 
    // opposing vertex.
    //
    Edge.prototype.interpolatePosition = function (t) {
      var v = this.position;
      var u = this.opposite.position;
      var s = (1 - t);
      return [
        v[0] * s + u[0] * t,
        v[1] * s + u[1] * t,
        v[2] * s + u[2] * t,
      ]; 
    };
    
    //
    // Iterate all half-edges that have this vertex as a base.
    //
    Vertex.prototype.iterateEdges = function (f) {
        var edge = this.edge;
        do {
            f(edge);
            edge = edge.opposite.next;
        } while (edge != this.edge);
    };
    
    //
    // Iterate the vertices of a face in CCW order.
    //    
    Face.prototype.iterateVertices = function(f)
    {
        var edge = this.edge;
        do {
            f(edge.vertex);
            edge = edge.next;
        } while (edge != this.edge);
    };

    HalfEdgeMesh.prototype.iterateFaces = function (f) {
        for (var i = 0; i < this._faces.length; ++i) {
            f(this._faces[i]);
        }
    };
    
    
    //
    // WIP
    // 
    // Take a vertex and "smooth" the corner by creating a face at 
    // that vertex.  
    //
    HalfEdgeMesh.prototype.smoothVertex = function (vertex, amount) {
            
        var vertices = [];
        var vertexEdges = [];
        
        // Create the new vertices
        vertex.iterateEdges(function(edge) {
            vertexEdges.push(edge);
            
            var p = edge.interpolatePosition(amount);
            var vertex = new Vertex(p[0], p[1], p[2]);           
            vertices.push(vertex);
        });
        
        //
        // Create the edges
        //        
        var faceEdges = [];
        var stitchEdges = [];        
        for (var i = 0; i < vertices.length; ++i) {
            faceEdges.push(new Edge());
            stitchEdges.push(new Edge());
        }

        //
        // Create the face
        //
        var face = new Face();       
        
        //
        // Link everything up
        //
        face.edge = faceEdges[0];
        
        for (var i = 0; i < vertices.length; ++i) {
            var j = (i + 1) % vertices.length;

            faceEdges[i].face = face;
            faceEdges[i].vertex = vertices[i];
            faceEdges[i].next = faceEdges[j];
            faceEdges[i].opposite = stitchEdges[i];
            
            stitchEdges[i].face = vertexEdges[j].face;
            stitchEdges[i].vertex = vertices[j];
            stitchEdges[i].next = vertexEdges[i];
            stitchEdges[i].opposite = faceEdges[i];

            vertexEdges[i].face.vertex = vertices[i];                                 
            vertexEdges[i].vertex = vertices[i];
            vertexEdges[i].opposite.next = stitchEdges[i];
            
            vertices[i].edge = vertexEdges[i];
        }
        
    };

    return HalfEdgeMesh;
})();

//===========================================================================//
// Add-Ins
//===========================================================================//

HalfEdgeMesh.prototype.createQuadMesh = function () {
    var h = this;
    var m = new QuadMesh();

    for (var i = 0; i < h._vertices.length; ++i) {
        var v = h._vertices[i];
        var p = v.position;

        m.addVertex(p[0], p[1], p[2]);
        m.addNormal(1, 0, 0);
        m.addColor(1, 1, 1);

        // Create a temporary property on the vertex to allow a lookup table
        // to be built.  
        //
        // The runtime cost of doing this unclear; V8 optimizes for
        // objects with a fixed set of properties.
        v._index = i;
    }

    h.iterateFaces(function (face) {
        var indices = [];
        face.iterateVertices(function (v) {
            indices.push(v._index);
        });
        m.addFace.apply(m, indices);
    });

    // Remove the temporary property
    for (var i = 0; i < h._vertices.length; ++i)
        delete h._vertices[i]._index;

    return m;
}


//===========================================================================//
// Helpers
//===========================================================================//

function createBox()
{
    var m = new QuadMesh();

    m.addVertex(-0.5, -0.5, -0.5);  // 0
    m.addVertex( 0.5, -0.5, -0.5);  // 1
    m.addVertex( 0.5,  0.5, -0.5);  // 2
    m.addVertex(-0.5,  0.5, -0.5);  // 3
    m.addVertex(-0.5, -0.5,  0.5);  // 4
    m.addVertex( 0.5, -0.5,  0.5);  // 5
    m.addVertex( 0.5,  0.5,  0.5);  // 6
    m.addVertex(-0.5,  0.5,  0.5);  // 7

    m.addFace(1, 2, 6, 5);  // +X = 0
    m.addFace(0, 4, 7, 3);  // -X = 1
    m.addFace(2, 3, 7, 6);  // +Y = 2
    m.addFace(0, 1, 5, 4);  // -Y = 3
    m.addFace(4, 5, 6, 7);  // +Z = 4
    m.addFace(3, 2, 1, 0);  // -Z = 5

    return m;
}

