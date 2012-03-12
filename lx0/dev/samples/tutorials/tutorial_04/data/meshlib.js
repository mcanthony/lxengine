//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2012 athile@athile.net (http://www.athile.net)

    Permission is hereby granted, free of charge, to any person obtaining a 
    copy of this software and associated documentation files (the "Software"), 
    to deal in the Software without restriction, including without limitation 
    the rights to use, copy, modify, merge, publish, distribute, sublicense, 
    and/or sell copies of the Software, and to permit persons to whom the 
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
    IN THE SOFTWARE.
*/
//===========================================================================//

//===========================================================================//
// Vector helpers
//===========================================================================//

function vec3_sub(u, v) {
    return [
        u[0] - v[0],
        u[1] - v[1],
        u[2] - v[2],
    ];
};

function vec3_dot(u, v) {
    return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
};

function vec3_cross(u, v) {
    return [
        u[1] * v[2] - v[1] * u[2],
        u[2] * v[0] - v[2] * u[0],
        u[0] * v[1] - v[0] * u[1],
    ];
};

function vec3_normal(u,v,w) {
    var n = vec3_cross(vec3_sub(v,u), vec3_sub(w,v));	
    var m = Math.sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    return [ n[0] / m, n[1] / m, n[2] / m ];
};


//===========================================================================//
// PointList
//===========================================================================//

function PointList() {
    this._meshType = "PointList";
    this._vertices = [];
}

PointList.prototype.addVertex = function (x, y, z) {
    var vertex = { position : [x, y, z] };
    this._vertices.push(vertex);
    return vertex;
}

//===========================================================================//
// LineList
//===========================================================================//

function LineList() {
    this._meshType = "LineList";
    this._vertices = [];
}

LineList.prototype.addSegment = function (v0, v1) {
    this._vertices.push({ position : v0.slice(0) });
	this._vertices.push({ position : v1.slice(0) });
}

//===========================================================================//
// TriMesh
//===========================================================================//

function TriMesh() {
    this._meshType = "TriMesh";
    this._vertices = [];
    this._faces = [];    
}

TriMesh.prototype.addVertex = function (x, y, z) {
    var vertex = { position : [x, y, z] };
    this._vertices.push(vertex);
    return vertex;
}
TriMesh.prototype.addFace = function(i0,i1,i2)
{
    this._faces.push({ indices : [i0,i1,i2] });
}

//===========================================================================//
// QuadMesh
//===========================================================================//

function QuadMesh() {
    this._meshType = "QuadMesh";
    this._vertices = [];
    this._faces = [];    
}

QuadMesh.prototype.addVertex = function (x, y, z) {
    var vertex = { position : [x, y, z] };
    this._vertices.push(vertex);
    return vertex;
}
QuadMesh.prototype.addFace = function(i0,i1,i2,i3)
{
    this._faces.push({ indices : [i0,i1,i2,i3] });
}

//===========================================================================//
// PolyMesh
//===========================================================================//

function PolyMesh() {
    this._meshType = "PolyMesh";
    this._vertices = [];
    this._faces = [];    
}

PolyMesh.prototype.addVertex = function (x, y, z) {
    var vertex = { position : [x, y, z] };
    this._vertices.push(vertex);
    return vertex;
}
PolyMesh.prototype.addFace = function()
{
    var face = { indices : [] };    
    for (var i = 0; i < arguments.length; ++i)
    {
        var index = arguments[i];        
        if (index === undefined)
            throw "Invalid index";
        
        face.indices.push(index);
    }
        
    this._faces.push(face);
}

PolyMesh.prototype.iterateVertices = function(f)
{
    var vertices = this._vertices.slice(0);
    for (var i = 0; i < vertices.length; ++i) {
        f(vertices[i]);
    }
}


PolyMesh.prototype.iterateFaces = function(f)
{
    for (var i = 0; i < this._faces.length; ++i) {
        f(this._faces[i]);
    }
}

PolyMesh.prototype.faceVertex = function(face, i)
{
    if (!(i < face.indices.length))
        throw "Invalid face vertex index";

    var index = face.indices[i];
    
    if (!(index < this._vertices.length))
        throw "Face index out of bounds";
    
    var vertex = this._vertices[index];

    if (vertex === undefined)
        throw "Vertex at index #" + index + " is undefined";    
    if (vertex.position === undefined)
        throw "Vertex does not have a position";
    
    return vertex;
}

PolyMesh.prototype.integrityCheck = function()
{
    var mesh = this;
    mesh.iterateFaces(function (face) {
        
        if (face.indices.length < 3)
            throw "Degenerate face detected.  Less than 3 vertices";
        
        var v0 = mesh.faceVertex(face, 0).position;
        var v1 = mesh.faceVertex(face, 1).position;
        var v2 = mesh.faceVertex(face, 2).position;
        var baseNormal = vec3_normal(v0,v1,v2);
        
        for (var i = 3; i < face.indices.length; ++i)
        {
            var v0 = mesh.faceVertex(face, i - 2).position;
            var v1 = mesh.faceVertex(face, i - 1).position;
            var v2 = mesh.faceVertex(face, i).position;
            var n = vec3_normal(v0,v1,v2);
            
            var dot = vec3_dot(baseNormal, n);
            
            if (dot < -.95)
                throw "Non-planar face in PolyMesh";
            else if (dot < .9)
                throw "Non-planar face in PolyMesh (dot = " + dot + ")";
        }
    });
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
            var v = mesh._vertices[vi].position;
            this._vertices.push(new Vertex(v[0], v[1], v[2]));
        }

        for (var fi = 0; fi < mesh._faces.length; ++fi) {
            this._faces.push(new Face());

            var face = mesh._faces[fi].indices;
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
                if (edges[i].vertex === edges[j].next.vertex                
                    && edges[i].next.vertex === edges[j].vertex) 
                {
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
    
    Edge.prototype.previous = function () {
        var last = this;
        this.iterateEdges(function(edge) {
            last = edge;
        });
        return last;
    };

    Edge.prototype.checkFaceIntegrity = function() {		
        var fail = false;
        var edge = this;
        var face = edge.face;
                
        var faceIndices = [];
        do 
        {
            faceIndices.push(edge.face._index);
            if (edge.face !== face) {
                fail = true;
            }		
            edge = edge.next;					
        } while (edge !== this);
        
        if (fail)
        {
            lx0.message("ERROR Edge face loop corrupt:");
            lx0.message("Face indices = [" + faceIndices + "]");
            throw "Edge face corrupt";
        }
    };

    HalfEdgeMesh.prototype.integrityCheck = function () {
    
        var mesh = this;
        this.indexElements();
        
        mesh.iterateVertices(function(vertex) {
            if (vertex._index === undefined) throw "indexElements did not set a index for all vertices";
            vertex.checkIntegrity();
            vertex.iterateEdges(function(edge) {
            });
        });
    
        for (var i = 0; i < this._faces.length; ++i) {
            if (this._faces[i].edge.face !== this._faces[i]) {
                throw "Edge face incorrect.  Edges in loop do not all refer to same face.";
            }
        }
        for (var i = 0; i < this._edges.length; ++i) {
            if (this._edges[i].opposite == null)
                throw "Null opposite edge";                
            if (this._edges[i].opposite.opposite !== this._edges[i]) {
                throw "Edge opposite incorrect #" + i;
            }
            
            if (this._edges[i].face === this._edges[i].opposite.face)
                throw "Opposing half edges share same face";            
                
            this._edges[i].checkFaceIntegrity();
        }
        for (var i = 0; i < this._vertices.length; ++i) {
            if (this._vertices[i].edge.vertex !== this._vertices[i]
                && this._vertices[i].edge.next.vertex !== this._vertices[i])
            {
                throw "Edge vertex incorrect";
            }
        }
        
        //
        // Check that all vertices referenced by the faces are in the
        // _vertices array.  Do so by marking each with a custom property.
        // Then check that all vertices in the array are referenced; do
        // this by inverting the process.
        //
        for (var i = 0; i < this._vertices.length; ++i)
            this._vertices[i]._inarray = true;
            
        this.iterateFaces(function (face) {
            face.iterateVertices(function (v) {
                if (v._inarray != true)
                    throw "Face references vertex not in the array";                
            });
        });
        for (var i = 0; i < this._vertices.length; ++i)
            delete this._vertices[i]._inarray;
            
        this.iterateFaces(function (face) {
            face.iterateVertices(function (v) {
                v._isreferenced = true;
            });
        });        
        for (var i = 0; i < this._vertices.length; ++i)
            if (this._vertices[i]._isreferenced != true)
                throw "Orphaned vertex in array";
        
        this.iterateFaces(function (face) {
            face.iterateVertices(function (v) {
                delete v._isreferenced;
            });
        });
        
        //
        // Check that all the faces are planar
        //
        var mesh = this;
        mesh.iterateFaces(function (face) {
                    
            var vertices = [];
            face.iterateVertices(function(vertex) {
                vertices.push(vertex.position);
            });
            
            if (vertices.length < 3)
                throw "Degenerate face detected.  Less than 3 vertices";			
            
            var bv0 = vertices[0];
            var bv1 = vertices[1];
            var bv2 = vertices[2];
            var baseNormal = vec3_normal(bv0,bv1,bv2);
            
            for (var i = 3; i < vertices.length; ++i)
            {
                var v0 = vertices[i - 2];
                var v1 = vertices[i - 1];
                var v2 = vertices[i];
                var n = vec3_normal(v0,v1,v2);
                
                var dot = vec3_dot(baseNormal, n);				
                if (dot < .9)
                {
                    lx0.message("Face vertices");
                    for (var k = 0; k < vertices.length; ++k)
                        lx0.message("V" + k + " " + vertices[k]);
                    
                    lx0.message("Offending face");					
                    lx0.message("V" + (i - 2) + " " + v0);
                    lx0.message("V" + (i - 1) + " " + v1);
                    lx0.message("V" + i + " " + v2);
                    throw "Non-planar face in HalfEdgeMesh (dot = " + dot + ")";
                }
            }
        });
    };
           
    
    //
    // Interpolates outward from the half-edge's base vertex.
    //
    // A value of 0 returns the position of the base vertex; a 
    // value of 1 will be a duplicate of the position of the 
    // opposing vertex.
    //
    Edge.prototype.interpolatePosition = function (t) {
      var v = this.vertex.position;
      var u = this.opposite.vertex.position;
      var s = (1 - t);
      return [
        v[0] * s + u[0] * t,
        v[1] * s + u[1] * t,
        v[2] * s + u[2] * t,
      ]; 
    };
    
    Edge.prototype.faceEdgeCount = function() {
        var count = 0;
        var edge = this;
        do {
            count++;
            edge = edge.next;
        } while (edge != this);		
        return count;
    }
    
    Edge.prototype.iterateEdges = function(f) {
        var edge = this;
        do {
            f(edge);
            edge = edge.next;
        } while (edge != this);
    }
    
    Vertex.prototype.checkIntegrity = function() {
                
        var vertex = this;
        var edgeCount = 0;
        var edgeTotal = this.degree();
        
        this.iterateEdges(function(edge) {   
         
            if (edge.vertex !== vertex)
            {
                lx0.message("*** Corrupt vertex detected on edge " + edgeCount + " of " + edgeTotal + " of vertex:");
                lx0.message("Vertex index = " + vertex._index + "    <" + vertex.position + ">");
                lx0.message("Edge index = " + edge._index);
                lx0.message("Edge vertex index = " + edge.vertex._index + "    <" + edge.vertex.position + ">");
                lx0.message("Edge opposite vertex = " + edge.opposite.vertex._index + "    <" + edge.opposite.vertex.position + ">");
                
                var indices = [];
                vertex.iterateEdges(function(edge) {
                    indices.push(edge.vertex._index);
                });
                lx0.message("Vertex indices = " + indices);
                
                throw "Corrupt edge-vertex relationship";
            }
            
            edgeCount++;
        });
    };
    
    //
    // Iterate all half-edges that have this vertex as a base.
    //
    // The iteration is in *clockwise* order: this is the opposite
    // of the face orientation, which is assumed to be counter-clockwise.
    //
    Vertex.prototype.iterateEdges = function (f) {	
        var edge = this.edge;
        do {		
            f(edge);
            edge = edge.opposite.next;
        } while (edge !== this.edge);
    };
    
    //
    // I.e. number of edges on the vertex
    //
    Vertex.prototype.degree = function (f) {	
        var edge = this.edge;
        var count = 0;
        do {		
            count++;
            edge = edge.opposite.next;
        } while (edge !== this.edge);
        return count;
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

    HalfEdgeMesh.prototype.iterateVertices = function (f) {
        var vertices = this._vertices.slice(0);
        for (var i = 0; i < vertices.length; ++i) {
            f(vertices[i]);
        }
    };

	HalfEdgeMesh.prototype.iterateEdges = function (f) {
        for (var i = 0; i < this._edges.length; ++i) {
            f(this._edges[i]);
        }
    };

    HalfEdgeMesh.prototype.iterateFaces = function (f) {
        for (var i = 0; i < this._faces.length; ++i) {
            f(this._faces[i]);
        }
    };
    
    HalfEdgeMesh.prototype.indexElements = function() {
    
        //
        // Ensure any orphaned indices are cleared out.  In a defect free
        // program, this is unnecessary as there should be no orphans, but
        // for robustness we'll do it as means to detect orphans.
        // 
        for (var i = 0; i < this._faces.length; ++i)
        {
            delete this._faces[i].edge._index;
        }
        for (var i = 0; i < this._edges.length; ++i)
        {
            delete this._edges[i].vertex._index;
            delete this._edges[i].face._index;
            delete this._edges[i].next._index;
            delete this._edges[i].opposite._index;
        }
        for (var i = 0; i < this._vertices.length; ++i)
        {
            delete this._vertices[i].edge._index;
        }
    
        //
        // Set the indices
        //
        for (var i = 0; i < this._faces.length; ++i)
            this._faces[i]._index = i;
        for (var i = 0; i < this._edges.length; ++i)
            this._edges[i]._index = i;
        for (var i = 0; i < this._vertices.length; ++i)
            this._vertices[i]._index = i;
    };
    
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
            lx0.assert(edge.vertex === vertex);
            
            var p = edge.interpolatePosition(amount);
            var vert = new Vertex(p[0], p[1], p[2]);           
            vertices.push(vert);   
        });
        this._vertices = this._vertices.concat(vertices);
        
        for (var i = 0; i < this._vertices.length; ++i)
        {
            if (this._vertices[i] === vertex)
            {
                this._vertices[i] = this._vertices.pop();
                break;
            }
        }
        
        //
        // Create the edges
        //        
        var faceEdges = [];
        var stitchEdges = [];        
        for (var i = 0; i < vertices.length; ++i) {
            faceEdges.push(new Edge());
            stitchEdges.push(new Edge());
        }        
        this._edges = this._edges.concat(faceEdges, stitchEdges);

        //
        // Create the face
        //
        var face = new Face();       
        this._faces.push(face);		
        
        //
        // Integrity check
        //
        if (false)
        {
            lx0.message("Integrity check 1...");
            for (var i = 0; i < vertices.length; ++i) {
                var j = (i + 1) % vertices.length;
                var h = (i + vertices.length - 1) % vertices.length;			
                
                vertexEdges[i].checkFaceIntegrity();	
                
                if (vertexEdges[i].opposite.next !== vertexEdges[j]) throw "E1000";
                if (vertexEdges[i].opposite.opposite !== vertexEdges[i]) throw "E1001";	
            }
        }
        
        //
        // Link everything up
        //
        face.edge = faceEdges[0];
        
        for (var i = 0; i < vertices.length; ++i) {
            var j = (i + 1) % vertices.length;
            var h = (i + vertices.length - 1) % vertices.length;

            faceEdges[i].face = face;
            faceEdges[i].vertex = vertices[i];
            faceEdges[i].next = faceEdges[h];
            faceEdges[i].opposite = stitchEdges[i];
            
            stitchEdges[i].face = vertexEdges[i].face;
            stitchEdges[i].vertex = vertices[h];
            stitchEdges[i].next = vertexEdges[i];
            stitchEdges[i].opposite = faceEdges[i];		

            vertexEdges[i].vertex = vertices[i];			
            vertexEdges[h].opposite.next= stitchEdges[i];	// Note: vertexEdges[h] === vertexEdges[i].previous()
            
            vertices[i].edge = vertexEdges[i];
        }
                
        //
        // Integrity checking
        //
        if (false)
        {
            lx0.message("Integrity check 2...");
            this.indexElements();
            
            for (var i = 0; i < vertices.length; ++i) {
                lx0.message("New Vertex #" + i + " degree=" + vertices[i].degree());
            }
            
            for (var i = 0; i < vertices.length; ++i) {
                var j = (i + 1) % vertices.length;
                var h = (i + vertices.length - 1) % vertices.length;
                var g = (i + vertices.length - 2) % vertices.length;
                
                vertices[i].checkIntegrity();
                
                lx0.assert(faceEdges[i].face === face);
                
                lx0.message("Check integrity faceEdges " + i);
                faceEdges[i].checkFaceIntegrity();
                lx0.message("Check integrity vertexEdges " + i);
                vertexEdges[i].checkFaceIntegrity();				
                lx0.message("Check integrity stitchEdges " + i);
                stitchEdges[i].checkFaceIntegrity();					
            }
        
            if (faceEdges[0].faceEdgeCount() != vertexEdges.length)
            {
                lx0.message("Vertex edges = " + vertexEdges.length);
                lx0.message("Face edges = " + faceEdges[0].faceEdgeCount());				
                throw "Incorrect face edge count";
            }
            
            lx0.message("Check new face integrity");
            faceEdges[0].checkFaceIntegrity();
            for (var i = 0; i < stitchEdges.length; ++i)
            {
                lx0.message("Check stitched face " + i + " integrity");
                stitchEdges[i].checkFaceIntegrity();
            }
        }
        
        //
        // Return the set of newly created elements
        //
        return {
            face : face,
            edges : faceEdges.concat(stitchEdges),
            vertices : vertices
        };
    };

    return HalfEdgeMesh;
})();

//===========================================================================//
// Add-Ins
//===========================================================================//

HalfEdgeMesh.prototype.createPolyMesh = function () {
    var h = this;
    var m = new PolyMesh();

    for (var i = 0; i < h._vertices.length; ++i) {
        var v = h._vertices[i];
        var p = v.position;

        var vertex = m.addVertex(p[0], p[1], p[2]);
        vertex.normal = [ 1, 0, 0 ];
        vertex.color = [ 1, 1, 1 ];

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
            if (v._index === undefined)
                throw "Vertex not properly indexed. Is the vertex referenced by the "
                     + "face but not in the _vertices array?";
            
            indices.push(v._index);
        });
        m.addFace.apply(m, indices);
    });

    // Remove the temporary property
    for (var i = 0; i < h._vertices.length; ++i)
        delete h._vertices[i]._index;

    return m;
};


PolyMesh.prototype.createTriMesh = function() {

    var pmesh = this;
    var tmesh = new TriMesh();

    pmesh.iterateVertices(function(v) {
        var p = v.position;

        var vertex = tmesh.addVertex(p[0], p[1], p[2]);
        vertex.normal = [ 1, 0, 0 ];
        vertex.color = [ 1, 1, 1 ];
    });
    
    pmesh.iterateFaces(function(face) {
        var i0 = face.indices[0];
        for (var i = 2; i < face.indices.length; ++i) {
            var i1 = face.indices[i - 1];
            var i2 = face.indices[i];            
            tmesh.addFace(i0, i1, i2);
        }
    });

    return tmesh;
};


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
