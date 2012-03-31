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

function vec3_add(u, v) {
    return [
        u[0] + v[0],
        u[1] + v[1],
        u[2] + v[2],
    ];
};

function vec3_sub(u, v) {
    return [
        u[0] - v[0],
        u[1] - v[1],
        u[2] - v[2],
    ];
};

function vec3_mul(u, s) {
    return [
        u[0] * s,
        u[1] * s,
        u[2] * s,
    ];
};

function vec3_div(u, s) {
    return [
        u[0] / s,
        u[1] / s,
        u[2] / s,
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

function vec3_avg () 
{
    var x = 0;
    var y = 0;
    var z = 0;
    var length = arguments.length;
    for (var i = 0; i < length; ++i)
    {
        var v = arguments[i];
        x += v[0];
        y += v[1];
        z += v[2];
    }
    return [ x / length, y / length, z / length ];
}


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
};

PointList.prototype.createPrimitiveBuffer = function() {
    var prim = {};
    prim.type = "points";
    prim.vertex = {};
    
    prim.vertex.positions = [];    
    for (var i = 0; i < this._vertices.length; ++i)
        prim.vertex.positions[i] = this._vertices[i].position.slice(0);   
        
    return prim;
};

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

LineList.prototype.createPrimitiveBuffer = function() {
    var prim = {};
    prim.type = "lines";
    prim.vertex = {};
    
    prim.vertex.positions = [];    
    for (var i = 0; i < this._vertices.length; ++i)
        prim.vertex.positions[i] = this._vertices[i].position.slice(0);   
        
    return prim;
};

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

TriMesh.prototype.computeFaceNormals = function()
{
    for (var i = 0; i < this._faces.length; ++i) {
        var f = this._faces[i].indices;
        var v0 = this._vertices[f[0]].position;
        var v1 = this._vertices[f[1]].position;
        var v2 = this._vertices[f[2]].position;       
        this._faces[i].normal = vec3_normal(v0,v1,v2);
    }
}

TriMesh.prototype.createPrimitiveBuffer = function() {
    var prim = {};
    prim.type = "triangles";
    prim.vertex = {};
    prim.face = {};
    
    prim.vertex.positions = [];    
    for (var i = 0; i < this._vertices.length; ++i)
        prim.vertex.positions[i] = this._vertices[i].position.slice(0);   
    
    prim.indices = [];    
    for (var i = 0; i < this._faces.length; ++i) {
        prim.indices[i*3+0] = this._faces[i].indices[0];  
        prim.indices[i*3+1] = this._faces[i].indices[1];
        prim.indices[i*3+2] = this._faces[i].indices[2];
    }
    
    if (this._faces[0].normal)
    {
        prim.face.normals = [];
        for (var i = 0; i < this._faces.length; ++i)
            prim.face.normals[i] = this._faces[i].normal.slice(0);
    }
    
    return prim;
};

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
    
        var debug = true;        
        var mesh = this;
                
        //
        // Property completeness checks
        //
        for (var i = 0; i < this._vertices.length; ++i)
        {
            var vertex = this._vertices[i];
            if (!("position" in vertex))  throw "Incomplete vertex definition";
            if (!("edge" in vertex))      throw "Incomplete vertex definition";
            if (!vertex.position)         throw "Incomplete vertex definition";
            if (!vertex.edge)             throw "Incomplete vertex definition";
        }
        
        for (var i = 0; i < this._faces.length; ++i)
        {
            var face = this._faces[i];
            if (!("edge" in face))        throw "Incomplete face definition";
            if (!face.edge)               throw "Incomplete face definition";
        }
        
        for (var i = 0; i < this._edges.length; ++i)
        {
            var edge = this._edges[i];
            if (!("vertex" in edge))      throw "Incomplete edge definition";
            if (!("face" in edge))        throw "Incomplete edge definition";
            if (!("next" in edge))        throw "Incomplete edge definition";
            if (!("opposite" in edge))    throw "Incomplete edge definition";
            if (!edge.vertex)             throw "Incomplete edge definition";
            if (!edge.face)               throw "Incomplete edge definition";
            if (!edge.next)               throw "Incomplete edge definition";
            if (!edge.opposite)           throw "Incomplete edge definition";
        }
        
        //
        // Index checks
        //
        this.indexElements();
        
        for (var i = 0; i < this._vertices.length; ++i)
        {
            var vertex = this._vertices[i];
            if (vertex._index !== i) throw "Corrupt vertex";
            if (vertex.edge !== this._edges[vertex.edge._index]) throw "Corrupt vertex";
        }
        
        for (var i = 0; i < this._faces.length; ++i)
        {
            var face = this._faces[i];
            if (face._index !== i)  throw "Corrupt face";
            if (face.edge !== this._edges[face.edge._index]) throw "Corrupt face";
        }
        
        for (var i = 0; i < this._edges.length; ++i)
        {
            var edge = this._edges[i];
            if (edge._index !== i) throw "Corrupt edge";
            if (edge.vertex !== this._vertices[edge.vertex._index]) throw "Corrupt edge";
            if (edge.face !== this._faces[edge.face ._index]) throw "Corrupt edge";
            if (edge.next !== this._edges[edge.next._index]) throw "Corrupt edge";
            if (edge.opposite !== this._edges[edge.opposite._index]) throw "Corrupt edge";
        }
        
        //
        // Simple invariant checks
        //
        for (var i = 0; i < this._vertices.length; ++i)
        {
            var vertex = this._vertices[i];
        }
        
        for (var i = 0; i < this._faces.length; ++i)
        {
            var face = this._faces[i];
        }
        
        for (var i = 0; i < this._edges.length; ++i)
        {
            var edge = this._edges[i];
        }
        
        
        if (debug) lx0.message("Check 0");
        mesh.iterateVertices(function(vertex) {
            if (debug) lx0.message("Checking vertex: " + vertex._index);
            if (vertex._index === undefined) 
                throw "indexElements did not set a index for all vertices";
            
            if (debug) lx0.message("Checking vertex integrity");
            vertex.checkIntegrity();
            
            if (debug) lx0.message("Checking vertex edges");
            var count = 0;
            vertex.iterateEdges(function(edge) {
                if (debug) lx0.message("Checking vertex-edge " + count);
                count++;
            });
        });
    
        if (debug) lx0.message("Check 1");
        for (var i = 0; i < this._faces.length; ++i) {
            if (this._faces[i].edge.face !== this._faces[i]) {
                throw "Edge face incorrect.  Edges in loop do not all refer to same face.";
            }
        }
        
        if (debug) lx0.message("Check 2");
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
        
        if (debug) lx0.message("Check 3");
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
        if (debug) lx0.message("Check 4");
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
            
        if (debug) lx0.message("Check 5");
        this.iterateFaces(function (face) {
            face.iterateVertices(function (v) {
                v._isreferenced = true;
            });
        });        
        for (var i = 0; i < this._vertices.length; ++i)
            if (this._vertices[i]._isreferenced != true)
                throw "Orphaned vertex in array";
        
        if (debug) lx0.message("Check 6");
        this.iterateFaces(function (face) {
            face.iterateVertices(function (v) {
                delete v._isreferenced;
            });
        });
        
        //
        // Check that all the faces are planar
        //
        if (debug) lx0.message("Check Faces are planar");
        var mesh = this;
        mesh.iterateFaces(function (face) {
                    
            if (debug) lx0.message("Checking planarity for face " + face._index);
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
        
        if (debug) lx0.message("Integrity check done");
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
        
        if (vertex.edge.opposite.opposite !== vertex.edge)
            throw "Edge relationship corrupt";
        if (vertex.edge.vertex !== vertex)
            throw "Vertex edge does not reference vertex!";
        
        var edgeCount = 0;
        lx0.message("Vertex computing degree...");
        var edgeTotal = this.degree();
        
        lx0.message("Degree = " + edgeTotal);
        this.iterateEdges(function(edge) {   
            lx0.message("E" + edgeCount + " checking...");
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
    
    Vertex.prototype.iterateFaces = function (f) {	
        var edge = this.edge;
        do {		
            f(edge.face);
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
            if (edge.vertex !== this)
                throw "Vertex-edge relationship corrupt";
        
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
        if (this.edge === null) throw "Face with undefined edge";
        
        var edge = this.edge;
        do {
            f(edge.vertex);
            edge = edge.next;
        } while (edge != this.edge);
    };
    
    Face.prototype.centroid = function()
    {
        var sum = [0,0,0];
        var count = 0;
        this.iterateVertices(function(vertex) {
            sum = vec3_add(sum, vertex.position);
            ++count;
        });
        return vec3_div(sum, count);
    };
    
    Face.prototype.sides = function() 
    {
        var edge = this.edge;
        var count = 0;
        do {
            count++;
            edge = edge.next;
        } while (edge != this.edge);
        return count;
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
    
        if (this._faces === undefined)    throw "Corrupt mesh";
        if (this._edges === undefined)    throw "Corrupt mesh";
        if (this._vertices === undefined) throw "Corrupt mesh";
    
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
    // Smooth the mesh using Catmull-Clark subdivision
    //
    HalfEdgeMesh.prototype.smooth = function() {
        
        // Alias since we use numerous callbacks
        var mesh = this;
            
        var newVertices = [];
        var newEdges = [];
        var newFaces = []; 
        
        // Temporarily cache the centroid of each face
        lx0.message("Step 1");
        mesh.iterateFaces(function(f) {
            var pos = f.centroid();
            f._centroid = new Vertex(pos[0], pos[1], pos[2]);
            newVertices.push(f._centroid);
        });
        
        //
        // Compute the new positions of each vertex, but don't
        // set them yet (the prior position is needed for all
        // other calculations).
        //
        mesh.iterateVertices(function (vertex) {
            
            var F = [0,0,0];
            var faceCount = 0;
            vertex.iterateFaces(function (face) { 
                F = vec3_add(F, face._centroid.position);
                faceCount++;
            });
            F = vec3_div(F, faceCount);
            
            var R = [0,0,0];
            var edgeCount = 0;
            vertex.iterateEdges(function (edge) {
                var midPoint = vec3_avg(edge.vertex.position, edge.opposite.vertex.position);
                R = vec3_add(R, midPoint);
                edgeCount ++;
            });
            R = vec3_div(R, edgeCount);
            
            
            var t = vec3_add(F, vec3_mul(R, 2));
            var p = vec3_mul(vertex.position, edgeCount - 3);
            vertex._newposition = vec3_div( vec3_add(t, p), edgeCount);
        });
        
        //
        // Split every edge in half using a weighted mid-point based
        // on the Catmull-Clark formulation
        //
        lx0.message("Step 2");
        mesh.iterateEdges(function(edge) {
            //
            // It's easier to split the edge and opposite edge
            // together, but we need to mark edges as processed
            // to ensure we don't process each edge twice.
            //
            if (!edge._mark)
            {
                edge._mark = true;
                edge.opposite._mark = true;
                
                var midPoint = vec3_avg(
                    edge.opposite.face._centroid.position, 
                    edge.face._centroid.position,
                    edge.vertex.position,
                    edge.opposite.vertex.position
                );
                var vertex = new Vertex(midPoint[0], midPoint[1], midPoint[2]);              
                
                var nedge0 = new Edge();
                nedge0.face = edge.face;
                nedge0.vertex = vertex;
                nedge0.opposite = edge.opposite;
                nedge0.next = edge.next;
                edge.next = nedge0;
                               
                var nedge1 = new Edge();
                nedge1.face = edge.opposite.face;
                nedge1.vertex = vertex;
                nedge1.opposite = edge;
                nedge1.next = edge.opposite.next;
                edge.opposite.next = nedge1;
                
                edge.opposite.opposite = nedge0;
                edge.opposite = nedge1;
                vertex.edge = nedge0;
                
                newVertices.push(vertex);
                newEdges.push(nedge0);
                newEdges.push(nedge1);
                
                if (nedge0.opposite.opposite !== nedge0) throw "Error";
                if (nedge1.opposite.opposite !== nedge1) throw "Error";
                if (edge.opposite.opposite !== edge) throw "Error"; 
                if (nedge0.opposite.next.vertex !== nedge0.vertex) throw "Error";
                if (nedge1.opposite.next.vertex !== nedge1.vertex) throw "Error";
                if (edge.vertex !== nedge1.next.vertex) throw "Error";
                if (vertex.degree() != 2) throw "Error";
            }
        });
        
        //
        // Split each face into a set of faces about the centroid
        //
        lx0.message("Step 3");
        mesh.iterateFaces(function(face) {
            var localFaces = [];
            var edge0 = face.edge;
            
            lx0.message("Face sides = " + face.sides());       
            do
            {
                var nextStart = edge0.next;
            
                var edge1 = new Edge();
                var edge2 = new Edge();
                var innerFace = new Face();
                innerFace.edge = edge0;
                innerFace._edge1 = edge1;   // Temp for connecting opposites
                innerFace._edge2 = edge2;   // Temp for connecting opposites
                
                edge0.face = innerFace;
                edge1.face = innerFace;
                edge2.face = innerFace;
                
                edge1.vertex = edge0.next.vertex;
                edge2.vertex = face._centroid;
                edge2.vertex.edge = edge2;
                
                edge0.next = edge1;
                edge1.next = edge2; 
                edge2.next = edge0;

                localFaces.push(innerFace);
                newFaces.push(innerFace);
                newEdges.push(edge1);
                newEdges.push(edge2);
                
                // Iterate to next pair
                edge0 = nextStart;
                
            } while (edge0 !== face.edge);
            
            //
            // Connect opposites
            //
            for (var i = 0; i < localFaces.length; ++i)
            {
                var face0 = localFaces[i];
                var face1 = localFaces[(i+1) % localFaces.length];
                face0._edge1.opposite = face1._edge2;
                face1._edge2.opposite = face0._edge1;
            }
            for (var i = 0; i < localFaces.length; ++i)
            {
                var face = localFaces[i];
                delete face._edge1;
                delete face._edge2;
            }
        });
        
        lx0.message("Face counts = " + mesh._faces.length + " / " + newFaces.length);
        
        //
        // Now move all the original vertices
        //
        mesh.iterateVertices(function (vertex) {
            vertex.position = vertex._newposition;
            delete vertex._newposition;
        });
        
        //
        // Concatenate all the new elements *after* the smoothing
        // to ensure they are not included in any of the iterations
        //
        // Note: all faces were replaced with a set of new faces, thus
        // we *replace* not concatenate the faces array.
        //
        mesh._vertices = mesh._vertices.concat(newVertices);
        mesh._edges    = mesh._edges.concat(newEdges);
        mesh._faces    = newFaces;
        
        lx0.message("Step 4");
        
        // Remove all temporary properties
        mesh.iterateEdges(function(e) {
            delete e._mark;
        });
    }
    
    
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
   
    var faceCount = 0;
    h.iterateFaces(function (face) {
        faceCount++;
        var indices = [];
        face.iterateVertices(function (v) {
            if (v._index === undefined)
                throw "Vertex not properly indexed. Is the vertex referenced by the "
                     + "face but not in the _vertices array?";
            
            indices.push(v._index);
        });
        m.addFace.apply(m, indices);
    });
    lx0.message("Generated polymesh with " + faceCount + " faces");

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

