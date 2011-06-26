//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011 athile@athile.net (http://www.athile.net)

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

#pragma once

#include <gl/glew.h>
#include <windows.h>        // Unfortunately must be included on Windows for GL.h to work
#include <gl/GL.h>

namespace lx0 
{
    namespace subsystem
    {
        /*!
            \defgroup lx0_subsystem_rasterizer lx0_subsystem_rasterizer
            \ingroup Subsystem
         */
        namespace rasterizer_ns
        {
            class GLInterface 
            {
            public:
                virtual ~GLInterface() {}

                virtual void        bindVertexArray     (GLuint array)                                                      { glBindVertexArray(array); }
                virtual void        deleteVertexArrays  (GLsizei n, const GLuint *arrays)                                   { glDeleteVertexArrays(n, arrays); }
                virtual void        genVertexArrays     (GLsizei n, GLuint *arrays)                                         { glGenVertexArrays(n, arrays); }
                virtual GLboolean   isVertexArray       (GLuint array)                                                      { return glIsVertexArray(array); }

                virtual void        genQueries          (GLsizei n, GLuint *ids)                                            { glGenQueries(n, ids); }
                virtual void        deleteQueries       (GLsizei n, const GLuint *ids)                                      { glDeleteQueries(n, ids); }
                virtual GLboolean   isQuery             (GLuint id)                                                         { return glIsQuery(id); }
                virtual void        beginQuery          (GLenum target, GLuint id)                                          { glBeginQuery(target, id); }
                virtual void        endQuery            (GLenum target)                                                     { glEndQuery(target); }
                virtual void        getQueryiv          (GLenum target, GLenum pname, GLint *params)                        { glGetQueryiv(target, pname, params); }
                virtual void        getQueryObjectiv    (GLuint id, GLenum pname, GLint *params)                            { glGetQueryObjectiv(id, pname, params); }
                virtual void        getQueryObjectuiv   (GLuint id, GLenum pname, GLuint *params)                           { glGetQueryObjectuiv(id, pname, params); }
                virtual void        bindBuffer          (GLenum target, GLuint buffer)                                      { glBindBuffer(target, buffer); }
                virtual void        deleteBuffers       (GLsizei n, const GLuint *buffers)                                  { glDeleteBuffers(n, buffers); }
                virtual void        genBuffers          (GLsizei n, GLuint *buffers)                                        { glGenBuffers(n, buffers); }
                virtual GLboolean   isBuffer            (GLuint buffer)                                                     { return glIsBuffer(buffer); }
                virtual void        bufferData          (GLenum target, GLsizeiptr size, const GLvoid *dat, GLenum usage)   { glBufferData(target, size, dat, usage); }
                virtual void        bufferSubData       (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data) { glBufferSubData(target, offset, size, data); }
                virtual void        getBufferSubData    (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data)     { glGetBufferSubData(target, offset, size, data); }
                virtual GLvoid*     mapBuffer           (GLenum target, GLenum access)                                      { return glMapBuffer(target, access); }
                virtual GLboolean   unmapBuffer         (GLenum target)                                                     { return glUnmapBuffer(target); }
                virtual void        getBufferParameteriv(GLenum target, GLenum pname, GLint *params)                        { glGetBufferParameteriv(target, pname, params); }
                virtual void        getBufferPointerv   (GLenum target, GLenum pname, GLvoid* *params)                      { glGetBufferPointerv(target, pname, params); }
            };
        }
    }

    using namespace lx0::subsystem::rasterizer_ns;
}
