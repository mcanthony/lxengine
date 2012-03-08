//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011-2012 athile@athile.net (http://www.athile.net)

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

#include <GL3/gl3w_modified.hpp>

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

            class OpenGlApi3_2
            {
            public:
                OpenGlApi3_2();
                void initialize();

                PFNGLCULLFACEPROC cullFace;
                PFNGLFRONTFACEPROC frontFace;
                PFNGLHINTPROC hint;
                PFNGLLINEWIDTHPROC lineWidth;
                PFNGLPOINTSIZEPROC pointSize;
                PFNGLPOLYGONMODEPROC polygonMode;
                PFNGLSCISSORPROC scissor;
                PFNGLTEXPARAMETERFPROC texParameterf;
                PFNGLTEXPARAMETERFVPROC texParameterfv;
                PFNGLTEXPARAMETERIPROC texParameteri;
                PFNGLTEXPARAMETERIVPROC texParameteriv;
                PFNGLTEXIMAGE1DPROC texImage1D;
                PFNGLTEXIMAGE2DPROC texImage2D;
                PFNGLDRAWBUFFERPROC drawBuffer;
                PFNGLCLEARPROC clear;
                PFNGLCLEARCOLORPROC clearColor;
                PFNGLCLEARSTENCILPROC clearStencil;
                PFNGLCLEARDEPTHPROC clearDepth;
                PFNGLSTENCILMASKPROC stencilMask;
                PFNGLCOLORMASKPROC colorMask;
                PFNGLDEPTHMASKPROC depthMask;
                PFNGLDISABLEPROC disable;
                PFNGLENABLEPROC enable;
                PFNGLFINISHPROC finish;
                PFNGLFLUSHPROC flush;
                PFNGLBLENDFUNCPROC blendFunc;
                PFNGLLOGICOPPROC logicOp;
                PFNGLSTENCILFUNCPROC stencilFunc;
                PFNGLSTENCILOPPROC stencilOp;
                PFNGLDEPTHFUNCPROC depthFunc;
                PFNGLPIXELSTOREFPROC pixelStoref;
                PFNGLPIXELSTOREIPROC pixelStorei;
                PFNGLREADBUFFERPROC readBuffer;
                PFNGLREADPIXELSPROC readPixels;
                PFNGLGETBOOLEANVPROC getBooleanv;
                PFNGLGETDOUBLEVPROC getDoublev;
                PFNGLGETERRORPROC getError;
                PFNGLGETFLOATVPROC getFloatv;
                PFNGLGETINTEGERVPROC getIntegerv;
                PFNGLGETSTRINGPROC getString;
                PFNGLGETTEXIMAGEPROC getTexImage;
                PFNGLGETTEXPARAMETERFVPROC getTexParameterfv;
                PFNGLGETTEXPARAMETERIVPROC getTexParameteriv;
                PFNGLGETTEXLEVELPARAMETERFVPROC getTexLevelParameterfv;
                PFNGLGETTEXLEVELPARAMETERIVPROC getTexLevelParameteriv;
                PFNGLISENABLEDPROC isEnabled;
                PFNGLDEPTHRANGEPROC depthRange;
                PFNGLVIEWPORTPROC viewport;
                PFNGLDRAWARRAYSPROC drawArrays;
                PFNGLDRAWELEMENTSPROC drawElements;
                PFNGLGETPOINTERVPROC getPointerv;
                PFNGLPOLYGONOFFSETPROC polygonOffset;
                PFNGLCOPYTEXIMAGE1DPROC copyTexImage1D;
                PFNGLCOPYTEXIMAGE2DPROC copyTexImage2D;
                PFNGLCOPYTEXSUBIMAGE1DPROC copyTexSubImage1D;
                PFNGLCOPYTEXSUBIMAGE2DPROC copyTexSubImage2D;
                PFNGLTEXSUBIMAGE1DPROC texSubImage1D;
                PFNGLTEXSUBIMAGE2DPROC texSubImage2D;
                PFNGLBINDTEXTUREPROC bindTexture;
                PFNGLDELETETEXTURESPROC deleteTextures;
                PFNGLGENTEXTURESPROC genTextures;
                PFNGLISTEXTUREPROC isTexture;
                PFNGLBLENDCOLORPROC blendColor;
                PFNGLBLENDEQUATIONPROC blendEquation;
                PFNGLDRAWRANGEELEMENTSPROC drawRangeElements;
                PFNGLTEXIMAGE3DPROC texImage3D;
                PFNGLTEXSUBIMAGE3DPROC texSubImage3D;
                PFNGLCOPYTEXSUBIMAGE3DPROC copyTexSubImage3D;
                PFNGLACTIVETEXTUREPROC activeTexture;
                PFNGLSAMPLECOVERAGEPROC sampleCoverage;
                PFNGLCOMPRESSEDTEXIMAGE3DPROC compressedTexImage3D;
                PFNGLCOMPRESSEDTEXIMAGE2DPROC compressedTexImage2D;
                PFNGLCOMPRESSEDTEXIMAGE1DPROC compressedTexImage1D;
                PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC compressedTexSubImage3D;
                PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC compressedTexSubImage2D;
                PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC compressedTexSubImage1D;
                PFNGLGETCOMPRESSEDTEXIMAGEPROC getCompressedTexImage;
                PFNGLBLENDFUNCSEPARATEPROC blendFuncSeparate;
                PFNGLMULTIDRAWARRAYSPROC multiDrawArrays;
                PFNGLMULTIDRAWELEMENTSPROC multiDrawElements;
                PFNGLPOINTPARAMETERFPROC pointParameterf;
                PFNGLPOINTPARAMETERFVPROC pointParameterfv;
                PFNGLPOINTPARAMETERIPROC pointParameteri;
                PFNGLPOINTPARAMETERIVPROC pointParameteriv;
                PFNGLGENQUERIESPROC genQueries;
                PFNGLDELETEQUERIESPROC deleteQueries;
                PFNGLISQUERYPROC isQuery;
                PFNGLBEGINQUERYPROC beginQuery;
                PFNGLENDQUERYPROC endQuery;
                PFNGLGETQUERYIVPROC getQueryiv;
                PFNGLGETQUERYOBJECTIVPROC getQueryObjectiv;
                PFNGLGETQUERYOBJECTUIVPROC getQueryObjectuiv;
                PFNGLBINDBUFFERPROC bindBuffer;
                PFNGLDELETEBUFFERSPROC deleteBuffers;
                PFNGLGENBUFFERSPROC genBuffers;
                PFNGLISBUFFERPROC isBuffer;
                PFNGLBUFFERDATAPROC bufferData;
                PFNGLBUFFERSUBDATAPROC bufferSubData;
                PFNGLGETBUFFERSUBDATAPROC getBufferSubData;
                PFNGLMAPBUFFERPROC mapBuffer;
                PFNGLUNMAPBUFFERPROC unmapBuffer;
                PFNGLGETBUFFERPARAMETERIVPROC getBufferParameteriv;
                PFNGLGETBUFFERPOINTERVPROC getBufferPointerv;
                PFNGLBLENDEQUATIONSEPARATEPROC blendEquationSeparate;
                PFNGLDRAWBUFFERSPROC drawBuffers;
                PFNGLSTENCILOPSEPARATEPROC stencilOpSeparate;
                PFNGLSTENCILFUNCSEPARATEPROC stencilFuncSeparate;
                PFNGLSTENCILMASKSEPARATEPROC stencilMaskSeparate;
                PFNGLATTACHSHADERPROC attachShader;
                PFNGLBINDATTRIBLOCATIONPROC bindAttribLocation;
                PFNGLCOMPILESHADERPROC compileShader;
                PFNGLCREATEPROGRAMPROC createProgram;
                PFNGLCREATESHADERPROC createShader;
                PFNGLDELETEPROGRAMPROC deleteProgram;
                PFNGLDELETESHADERPROC deleteShader;
                PFNGLDETACHSHADERPROC detachShader;
                PFNGLDISABLEVERTEXATTRIBARRAYPROC disableVertexAttribArray;
                PFNGLENABLEVERTEXATTRIBARRAYPROC enableVertexAttribArray;
                PFNGLGETACTIVEATTRIBPROC getActiveAttrib;
                PFNGLGETACTIVEUNIFORMPROC getActiveUniform;
                PFNGLGETATTACHEDSHADERSPROC getAttachedShaders;
                PFNGLGETATTRIBLOCATIONPROC getAttribLocation;
                PFNGLGETPROGRAMIVPROC getProgramiv;
                PFNGLGETPROGRAMINFOLOGPROC getProgramInfoLog;
                PFNGLGETSHADERIVPROC getShaderiv;
                PFNGLGETSHADERINFOLOGPROC getShaderInfoLog;
                PFNGLGETSHADERSOURCEPROC getShaderSource;
                PFNGLGETUNIFORMLOCATIONPROC getUniformLocation;
                PFNGLGETUNIFORMFVPROC getUniformfv;
                PFNGLGETUNIFORMIVPROC getUniformiv;
                PFNGLGETVERTEXATTRIBDVPROC getVertexAttribdv;
                PFNGLGETVERTEXATTRIBFVPROC getVertexAttribfv;
                PFNGLGETVERTEXATTRIBIVPROC getVertexAttribiv;
                PFNGLGETVERTEXATTRIBPOINTERVPROC getVertexAttribPointerv;
                PFNGLISPROGRAMPROC isProgram;
                PFNGLISSHADERPROC isShader;
                PFNGLLINKPROGRAMPROC linkProgram;
                PFNGLSHADERSOURCEPROC shaderSource;
                PFNGLUSEPROGRAMPROC useProgram;
                PFNGLUNIFORM1FPROC uniform1f;
                PFNGLUNIFORM2FPROC uniform2f;
                PFNGLUNIFORM3FPROC uniform3f;
                PFNGLUNIFORM4FPROC uniform4f;
                PFNGLUNIFORM1IPROC uniform1i;
                PFNGLUNIFORM2IPROC uniform2i;
                PFNGLUNIFORM3IPROC uniform3i;
                PFNGLUNIFORM4IPROC uniform4i;
                PFNGLUNIFORM1FVPROC uniform1fv;
                PFNGLUNIFORM2FVPROC uniform2fv;
                PFNGLUNIFORM3FVPROC uniform3fv;
                PFNGLUNIFORM4FVPROC uniform4fv;
                PFNGLUNIFORM1IVPROC uniform1iv;
                PFNGLUNIFORM2IVPROC uniform2iv;
                PFNGLUNIFORM3IVPROC uniform3iv;
                PFNGLUNIFORM4IVPROC uniform4iv;
                PFNGLUNIFORMMATRIX2FVPROC uniformMatrix2fv;
                PFNGLUNIFORMMATRIX3FVPROC uniformMatrix3fv;
                PFNGLUNIFORMMATRIX4FVPROC uniformMatrix4fv;
                PFNGLVALIDATEPROGRAMPROC validateProgram;
                PFNGLVERTEXATTRIB1DPROC vertexAttrib1d;
                PFNGLVERTEXATTRIB1DVPROC vertexAttrib1dv;
                PFNGLVERTEXATTRIB1FPROC vertexAttrib1f;
                PFNGLVERTEXATTRIB1FVPROC vertexAttrib1fv;
                PFNGLVERTEXATTRIB1SPROC vertexAttrib1s;
                PFNGLVERTEXATTRIB1SVPROC vertexAttrib1sv;
                PFNGLVERTEXATTRIB2DPROC vertexAttrib2d;
                PFNGLVERTEXATTRIB2DVPROC vertexAttrib2dv;
                PFNGLVERTEXATTRIB2FPROC vertexAttrib2f;
                PFNGLVERTEXATTRIB2FVPROC vertexAttrib2fv;
                PFNGLVERTEXATTRIB2SPROC vertexAttrib2s;
                PFNGLVERTEXATTRIB2SVPROC vertexAttrib2sv;
                PFNGLVERTEXATTRIB3DPROC vertexAttrib3d;
                PFNGLVERTEXATTRIB3DVPROC vertexAttrib3dv;
                PFNGLVERTEXATTRIB3FPROC vertexAttrib3f;
                PFNGLVERTEXATTRIB3FVPROC vertexAttrib3fv;
                PFNGLVERTEXATTRIB3SPROC vertexAttrib3s;
                PFNGLVERTEXATTRIB3SVPROC vertexAttrib3sv;
                PFNGLVERTEXATTRIB4NBVPROC vertexAttrib4Nbv;
                PFNGLVERTEXATTRIB4NIVPROC vertexAttrib4Niv;
                PFNGLVERTEXATTRIB4NSVPROC vertexAttrib4Nsv;
                PFNGLVERTEXATTRIB4NUBPROC vertexAttrib4Nub;
                PFNGLVERTEXATTRIB4NUBVPROC vertexAttrib4Nubv;
                PFNGLVERTEXATTRIB4NUIVPROC vertexAttrib4Nuiv;
                PFNGLVERTEXATTRIB4NUSVPROC vertexAttrib4Nusv;
                PFNGLVERTEXATTRIB4BVPROC vertexAttrib4bv;
                PFNGLVERTEXATTRIB4DPROC vertexAttrib4d;
                PFNGLVERTEXATTRIB4DVPROC vertexAttrib4dv;
                PFNGLVERTEXATTRIB4FPROC vertexAttrib4f;
                PFNGLVERTEXATTRIB4FVPROC vertexAttrib4fv;
                PFNGLVERTEXATTRIB4IVPROC vertexAttrib4iv;
                PFNGLVERTEXATTRIB4SPROC vertexAttrib4s;
                PFNGLVERTEXATTRIB4SVPROC vertexAttrib4sv;
                PFNGLVERTEXATTRIB4UBVPROC vertexAttrib4ubv;
                PFNGLVERTEXATTRIB4UIVPROC vertexAttrib4uiv;
                PFNGLVERTEXATTRIB4USVPROC vertexAttrib4usv;
                PFNGLVERTEXATTRIBPOINTERPROC vertexAttribPointer;
                PFNGLUNIFORMMATRIX2X3FVPROC uniformMatrix2x3fv;
                PFNGLUNIFORMMATRIX3X2FVPROC uniformMatrix3x2fv;
                PFNGLUNIFORMMATRIX2X4FVPROC uniformMatrix2x4fv;
                PFNGLUNIFORMMATRIX4X2FVPROC uniformMatrix4x2fv;
                PFNGLUNIFORMMATRIX3X4FVPROC uniformMatrix3x4fv;
                PFNGLUNIFORMMATRIX4X3FVPROC uniformMatrix4x3fv;
                PFNGLCOLORMASKIPROC colorMaski;
                PFNGLGETBOOLEANI_VPROC getBooleani_v;
                PFNGLGETINTEGERI_VPROC getIntegeri_v;
                PFNGLENABLEIPROC enablei;
                PFNGLDISABLEIPROC disablei;
                PFNGLISENABLEDIPROC isEnabledi;
                PFNGLBEGINTRANSFORMFEEDBACKPROC beginTransformFeedback;
                PFNGLENDTRANSFORMFEEDBACKPROC endTransformFeedback;
                PFNGLBINDBUFFERRANGEPROC bindBufferRange;
                PFNGLBINDBUFFERBASEPROC bindBufferBase;
                PFNGLTRANSFORMFEEDBACKVARYINGSPROC transformFeedbackVaryings;
                PFNGLGETTRANSFORMFEEDBACKVARYINGPROC getTransformFeedbackVarying;
                PFNGLCLAMPCOLORPROC clampColor;
                PFNGLBEGINCONDITIONALRENDERPROC beginConditionalRender;
                PFNGLENDCONDITIONALRENDERPROC endConditionalRender;
                PFNGLVERTEXATTRIBIPOINTERPROC vertexAttribIPointer;
                PFNGLGETVERTEXATTRIBIIVPROC getVertexAttribIiv;
                PFNGLGETVERTEXATTRIBIUIVPROC getVertexAttribIuiv;
                PFNGLVERTEXATTRIBI1IPROC vertexAttribI1i;
                PFNGLVERTEXATTRIBI2IPROC vertexAttribI2i;
                PFNGLVERTEXATTRIBI3IPROC vertexAttribI3i;
                PFNGLVERTEXATTRIBI4IPROC vertexAttribI4i;
                PFNGLVERTEXATTRIBI1UIPROC vertexAttribI1ui;
                PFNGLVERTEXATTRIBI2UIPROC vertexAttribI2ui;
                PFNGLVERTEXATTRIBI3UIPROC vertexAttribI3ui;
                PFNGLVERTEXATTRIBI4UIPROC vertexAttribI4ui;
                PFNGLVERTEXATTRIBI1IVPROC vertexAttribI1iv;
                PFNGLVERTEXATTRIBI2IVPROC vertexAttribI2iv;
                PFNGLVERTEXATTRIBI3IVPROC vertexAttribI3iv;
                PFNGLVERTEXATTRIBI4IVPROC vertexAttribI4iv;
                PFNGLVERTEXATTRIBI1UIVPROC vertexAttribI1uiv;
                PFNGLVERTEXATTRIBI2UIVPROC vertexAttribI2uiv;
                PFNGLVERTEXATTRIBI3UIVPROC vertexAttribI3uiv;
                PFNGLVERTEXATTRIBI4UIVPROC vertexAttribI4uiv;
                PFNGLVERTEXATTRIBI4BVPROC vertexAttribI4bv;
                PFNGLVERTEXATTRIBI4SVPROC vertexAttribI4sv;
                PFNGLVERTEXATTRIBI4UBVPROC vertexAttribI4ubv;
                PFNGLVERTEXATTRIBI4USVPROC vertexAttribI4usv;
                PFNGLGETUNIFORMUIVPROC getUniformuiv;
                PFNGLBINDFRAGDATALOCATIONPROC bindFragDataLocation;
                PFNGLGETFRAGDATALOCATIONPROC getFragDataLocation;
                PFNGLUNIFORM1UIPROC uniform1ui;
                PFNGLUNIFORM2UIPROC uniform2ui;
                PFNGLUNIFORM3UIPROC uniform3ui;
                PFNGLUNIFORM4UIPROC uniform4ui;
                PFNGLUNIFORM1UIVPROC uniform1uiv;
                PFNGLUNIFORM2UIVPROC uniform2uiv;
                PFNGLUNIFORM3UIVPROC uniform3uiv;
                PFNGLUNIFORM4UIVPROC uniform4uiv;
                PFNGLTEXPARAMETERIIVPROC texParameterIiv;
                PFNGLTEXPARAMETERIUIVPROC texParameterIuiv;
                PFNGLGETTEXPARAMETERIIVPROC getTexParameterIiv;
                PFNGLGETTEXPARAMETERIUIVPROC getTexParameterIuiv;
                PFNGLCLEARBUFFERIVPROC clearBufferiv;
                PFNGLCLEARBUFFERUIVPROC clearBufferuiv;
                PFNGLCLEARBUFFERFVPROC clearBufferfv;
                PFNGLCLEARBUFFERFIPROC clearBufferfi;
                PFNGLGETSTRINGIPROC getStringi;
                PFNGLDRAWARRAYSINSTANCEDPROC drawArraysInstanced;
                PFNGLDRAWELEMENTSINSTANCEDPROC drawElementsInstanced;
                PFNGLTEXBUFFERPROC texBuffer;
                PFNGLPRIMITIVERESTARTINDEXPROC primitiveRestartIndex;
                PFNGLGETINTEGER64I_VPROC getInteger64i_v;
                PFNGLGETBUFFERPARAMETERI64VPROC getBufferParameteri64v;
                PFNGLFRAMEBUFFERTEXTUREPROC framebufferTexture;
                PFNGLVERTEXATTRIBDIVISORPROC vertexAttribDivisor;
                PFNGLMINSAMPLESHADINGPROC minSampleShading;
                PFNGLBLENDEQUATIONIPROC blendEquationi;
                PFNGLBLENDEQUATIONSEPARATEIPROC blendEquationSeparatei;
                PFNGLBLENDFUNCIPROC blendFunci;
                PFNGLBLENDFUNCSEPARATEIPROC blendFuncSeparatei;
                PFNGLISRENDERBUFFERPROC isRenderbuffer;
                PFNGLBINDRENDERBUFFERPROC bindRenderbuffer;
                PFNGLDELETERENDERBUFFERSPROC deleteRenderbuffers;
                PFNGLGENRENDERBUFFERSPROC genRenderbuffers;
                PFNGLRENDERBUFFERSTORAGEPROC renderbufferStorage;
                PFNGLGETRENDERBUFFERPARAMETERIVPROC getRenderbufferParameteriv;
                PFNGLISFRAMEBUFFERPROC isFramebuffer;
                PFNGLBINDFRAMEBUFFERPROC bindFramebuffer;
                PFNGLDELETEFRAMEBUFFERSPROC deleteFramebuffers;
                PFNGLGENFRAMEBUFFERSPROC genFramebuffers;
                PFNGLCHECKFRAMEBUFFERSTATUSPROC checkFramebufferStatus;
                PFNGLFRAMEBUFFERTEXTURE1DPROC framebufferTexture1D;
                PFNGLFRAMEBUFFERTEXTURE2DPROC framebufferTexture2D;
                PFNGLFRAMEBUFFERTEXTURE3DPROC framebufferTexture3D;
                PFNGLFRAMEBUFFERRENDERBUFFERPROC framebufferRenderbuffer;
                PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC getFramebufferAttachmentParameteriv;
                PFNGLGENERATEMIPMAPPROC generateMipmap;
                PFNGLBLITFRAMEBUFFERPROC blitFramebuffer;
                PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC renderbufferStorageMultisample;
                PFNGLFRAMEBUFFERTEXTURELAYERPROC framebufferTextureLayer;
                PFNGLMAPBUFFERRANGEPROC mapBufferRange;
                PFNGLFLUSHMAPPEDBUFFERRANGEPROC flushMappedBufferRange;
                PFNGLBINDVERTEXARRAYPROC bindVertexArray;
                PFNGLDELETEVERTEXARRAYSPROC deleteVertexArrays;
                PFNGLGENVERTEXARRAYSPROC genVertexArrays;
                PFNGLISVERTEXARRAYPROC isVertexArray;
                PFNGLGETUNIFORMINDICESPROC getUniformIndices;
                PFNGLGETACTIVEUNIFORMSIVPROC getActiveUniformsiv;
                PFNGLGETACTIVEUNIFORMNAMEPROC getActiveUniformName;
                PFNGLGETUNIFORMBLOCKINDEXPROC getUniformBlockIndex;
                PFNGLGETACTIVEUNIFORMBLOCKIVPROC getActiveUniformBlockiv;
                PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC getActiveUniformBlockName;
                PFNGLUNIFORMBLOCKBINDINGPROC uniformBlockBinding;
                PFNGLCOPYBUFFERSUBDATAPROC copyBufferSubData;
                PFNGLDRAWELEMENTSBASEVERTEXPROC drawElementsBaseVertex;
                PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC drawRangeElementsBaseVertex;
                PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC drawElementsInstancedBaseVertex;
                PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC multiDrawElementsBaseVertex;
                PFNGLPROVOKINGVERTEXPROC provokingVertex;
                PFNGLFENCESYNCPROC fenceSync;
                PFNGLISSYNCPROC isSync;
                PFNGLDELETESYNCPROC deleteSync;
                PFNGLCLIENTWAITSYNCPROC clientWaitSync;
                PFNGLWAITSYNCPROC waitSync;
                PFNGLGETINTEGER64VPROC getInteger64v;
                PFNGLGETSYNCIVPROC getSynciv;
                PFNGLTEXIMAGE2DMULTISAMPLEPROC texImage2DMultisample;
                PFNGLTEXIMAGE3DMULTISAMPLEPROC texImage3DMultisample;
                PFNGLGETMULTISAMPLEFVPROC getMultisamplefv;
                PFNGLSAMPLEMASKIPROC sampleMaski;
                PFNGLBINDFRAGDATALOCATIONINDEXEDPROC bindFragDataLocationIndexed;
                PFNGLGETFRAGDATAINDEXPROC getFragDataIndex;
                PFNGLGENSAMPLERSPROC genSamplers;
                PFNGLDELETESAMPLERSPROC deleteSamplers;
                PFNGLISSAMPLERPROC isSampler;
                PFNGLBINDSAMPLERPROC bindSampler;
                PFNGLSAMPLERPARAMETERIPROC samplerParameteri;
                PFNGLSAMPLERPARAMETERIVPROC samplerParameteriv;
                PFNGLSAMPLERPARAMETERFPROC samplerParameterf;
                PFNGLSAMPLERPARAMETERFVPROC samplerParameterfv;
                PFNGLSAMPLERPARAMETERIIVPROC samplerParameterIiv;
                PFNGLSAMPLERPARAMETERIUIVPROC samplerParameterIuiv;
                PFNGLGETSAMPLERPARAMETERIVPROC getSamplerParameteriv;
                PFNGLGETSAMPLERPARAMETERIIVPROC getSamplerParameterIiv;
                PFNGLGETSAMPLERPARAMETERFVPROC getSamplerParameterfv;
                PFNGLGETSAMPLERPARAMETERIUIVPROC getSamplerParameterIuiv;
                PFNGLQUERYCOUNTERPROC queryCounter;
                PFNGLGETQUERYOBJECTI64VPROC getQueryObjecti64v;
                PFNGLGETQUERYOBJECTUI64VPROC getQueryObjectui64v;
                PFNGLVERTEXP2UIPROC vertexP2ui;
                PFNGLVERTEXP2UIVPROC vertexP2uiv;
                PFNGLVERTEXP3UIPROC vertexP3ui;
                PFNGLVERTEXP3UIVPROC vertexP3uiv;
                PFNGLVERTEXP4UIPROC vertexP4ui;
                PFNGLVERTEXP4UIVPROC vertexP4uiv;
                PFNGLTEXCOORDP1UIPROC texCoordP1ui;
                PFNGLTEXCOORDP1UIVPROC texCoordP1uiv;
                PFNGLTEXCOORDP2UIPROC texCoordP2ui;
                PFNGLTEXCOORDP2UIVPROC texCoordP2uiv;
                PFNGLTEXCOORDP3UIPROC texCoordP3ui;
                PFNGLTEXCOORDP3UIVPROC texCoordP3uiv;
                PFNGLTEXCOORDP4UIPROC texCoordP4ui;
                PFNGLTEXCOORDP4UIVPROC texCoordP4uiv;
                PFNGLMULTITEXCOORDP1UIPROC multiTexCoordP1ui;
                PFNGLMULTITEXCOORDP1UIVPROC multiTexCoordP1uiv;
                PFNGLMULTITEXCOORDP2UIPROC multiTexCoordP2ui;
                PFNGLMULTITEXCOORDP2UIVPROC multiTexCoordP2uiv;
                PFNGLMULTITEXCOORDP3UIPROC multiTexCoordP3ui;
                PFNGLMULTITEXCOORDP3UIVPROC multiTexCoordP3uiv;
                PFNGLMULTITEXCOORDP4UIPROC multiTexCoordP4ui;
                PFNGLMULTITEXCOORDP4UIVPROC multiTexCoordP4uiv;
                PFNGLNORMALP3UIPROC normalP3ui;
                PFNGLNORMALP3UIVPROC normalP3uiv;
                PFNGLCOLORP3UIPROC colorP3ui;
                PFNGLCOLORP3UIVPROC colorP3uiv;
                PFNGLCOLORP4UIPROC colorP4ui;
                PFNGLCOLORP4UIVPROC colorP4uiv;
                PFNGLSECONDARYCOLORP3UIPROC secondaryColorP3ui;
                PFNGLSECONDARYCOLORP3UIVPROC secondaryColorP3uiv;
                PFNGLVERTEXATTRIBP1UIPROC vertexAttribP1ui;
                PFNGLVERTEXATTRIBP1UIVPROC vertexAttribP1uiv;
                PFNGLVERTEXATTRIBP2UIPROC vertexAttribP2ui;
                PFNGLVERTEXATTRIBP2UIVPROC vertexAttribP2uiv;
                PFNGLVERTEXATTRIBP3UIPROC vertexAttribP3ui;
                PFNGLVERTEXATTRIBP3UIVPROC vertexAttribP3uiv;
                PFNGLVERTEXATTRIBP4UIPROC vertexAttribP4ui;
                PFNGLVERTEXATTRIBP4UIVPROC vertexAttribP4uiv;
                PFNGLDRAWARRAYSINDIRECTPROC drawArraysIndirect;
                PFNGLDRAWELEMENTSINDIRECTPROC drawElementsIndirect;
                PFNGLUNIFORM1DPROC uniform1d;
                PFNGLUNIFORM2DPROC uniform2d;
                PFNGLUNIFORM3DPROC uniform3d;
                PFNGLUNIFORM4DPROC uniform4d;
                PFNGLUNIFORM1DVPROC uniform1dv;
                PFNGLUNIFORM2DVPROC uniform2dv;
                PFNGLUNIFORM3DVPROC uniform3dv;
                PFNGLUNIFORM4DVPROC uniform4dv;
                PFNGLUNIFORMMATRIX2DVPROC uniformMatrix2dv;
                PFNGLUNIFORMMATRIX3DVPROC uniformMatrix3dv;
                PFNGLUNIFORMMATRIX4DVPROC uniformMatrix4dv;
                PFNGLUNIFORMMATRIX2X3DVPROC uniformMatrix2x3dv;
                PFNGLUNIFORMMATRIX2X4DVPROC uniformMatrix2x4dv;
                PFNGLUNIFORMMATRIX3X2DVPROC uniformMatrix3x2dv;
                PFNGLUNIFORMMATRIX3X4DVPROC uniformMatrix3x4dv;
                PFNGLUNIFORMMATRIX4X2DVPROC uniformMatrix4x2dv;
                PFNGLUNIFORMMATRIX4X3DVPROC uniformMatrix4x3dv;
                PFNGLGETUNIFORMDVPROC getUniformdv;
                PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC getSubroutineUniformLocation;
                PFNGLGETSUBROUTINEINDEXPROC getSubroutineIndex;
                PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC getActiveSubroutineUniformiv;
                PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC getActiveSubroutineUniformName;
                PFNGLGETACTIVESUBROUTINENAMEPROC getActiveSubroutineName;
                PFNGLUNIFORMSUBROUTINESUIVPROC uniformSubroutinesuiv;
                PFNGLGETUNIFORMSUBROUTINEUIVPROC getUniformSubroutineuiv;
                PFNGLGETPROGRAMSTAGEIVPROC getProgramStageiv;
                PFNGLPATCHPARAMETERIPROC patchParameteri;
                PFNGLPATCHPARAMETERFVPROC patchParameterfv;
                PFNGLBINDTRANSFORMFEEDBACKPROC bindTransformFeedback;
                PFNGLDELETETRANSFORMFEEDBACKSPROC deleteTransformFeedbacks;
                PFNGLGENTRANSFORMFEEDBACKSPROC genTransformFeedbacks;
                PFNGLISTRANSFORMFEEDBACKPROC isTransformFeedback;
                PFNGLPAUSETRANSFORMFEEDBACKPROC pauseTransformFeedback;
                PFNGLRESUMETRANSFORMFEEDBACKPROC resumeTransformFeedback;
                PFNGLDRAWTRANSFORMFEEDBACKPROC drawTransformFeedback;
                PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC drawTransformFeedbackStream;
                PFNGLBEGINQUERYINDEXEDPROC beginQueryIndexed;
                PFNGLENDQUERYINDEXEDPROC endQueryIndexed;
                PFNGLGETQUERYINDEXEDIVPROC getQueryIndexediv;
                PFNGLRELEASESHADERCOMPILERPROC releaseShaderCompiler;
                PFNGLSHADERBINARYPROC shaderBinary;
                PFNGLGETSHADERPRECISIONFORMATPROC getShaderPrecisionFormat;
                PFNGLDEPTHRANGEFPROC depthRangef;
                PFNGLCLEARDEPTHFPROC clearDepthf;
                PFNGLGETPROGRAMBINARYPROC getProgramBinary;
                PFNGLPROGRAMBINARYPROC programBinary;
                PFNGLPROGRAMPARAMETERIPROC programParameteri;
                PFNGLUSEPROGRAMSTAGESPROC useProgramStages;
                PFNGLACTIVESHADERPROGRAMPROC activeShaderProgram;
                PFNGLCREATESHADERPROGRAMVPROC createShaderProgramv;
                PFNGLBINDPROGRAMPIPELINEPROC bindProgramPipeline;
                PFNGLDELETEPROGRAMPIPELINESPROC deleteProgramPipelines;
                PFNGLGENPROGRAMPIPELINESPROC genProgramPipelines;
                PFNGLISPROGRAMPIPELINEPROC isProgramPipeline;
                PFNGLGETPROGRAMPIPELINEIVPROC getProgramPipelineiv;
                PFNGLPROGRAMUNIFORM1IPROC programUniform1i;
                PFNGLPROGRAMUNIFORM1IVPROC programUniform1iv;
                PFNGLPROGRAMUNIFORM1FPROC programUniform1f;
                PFNGLPROGRAMUNIFORM1FVPROC programUniform1fv;
                PFNGLPROGRAMUNIFORM1DPROC programUniform1d;
                PFNGLPROGRAMUNIFORM1DVPROC programUniform1dv;
                PFNGLPROGRAMUNIFORM1UIPROC programUniform1ui;
                PFNGLPROGRAMUNIFORM1UIVPROC programUniform1uiv;
                PFNGLPROGRAMUNIFORM2IPROC programUniform2i;
                PFNGLPROGRAMUNIFORM2IVPROC programUniform2iv;
                PFNGLPROGRAMUNIFORM2FPROC programUniform2f;
                PFNGLPROGRAMUNIFORM2FVPROC programUniform2fv;
                PFNGLPROGRAMUNIFORM2DPROC programUniform2d;
                PFNGLPROGRAMUNIFORM2DVPROC programUniform2dv;
                PFNGLPROGRAMUNIFORM2UIPROC programUniform2ui;
                PFNGLPROGRAMUNIFORM2UIVPROC programUniform2uiv;
                PFNGLPROGRAMUNIFORM3IPROC programUniform3i;
                PFNGLPROGRAMUNIFORM3IVPROC programUniform3iv;
                PFNGLPROGRAMUNIFORM3FPROC programUniform3f;
                PFNGLPROGRAMUNIFORM3FVPROC programUniform3fv;
                PFNGLPROGRAMUNIFORM3DPROC programUniform3d;
                PFNGLPROGRAMUNIFORM3DVPROC programUniform3dv;
                PFNGLPROGRAMUNIFORM3UIPROC programUniform3ui;
                PFNGLPROGRAMUNIFORM3UIVPROC programUniform3uiv;
                PFNGLPROGRAMUNIFORM4IPROC programUniform4i;
                PFNGLPROGRAMUNIFORM4IVPROC programUniform4iv;
                PFNGLPROGRAMUNIFORM4FPROC programUniform4f;
                PFNGLPROGRAMUNIFORM4FVPROC programUniform4fv;
                PFNGLPROGRAMUNIFORM4DPROC programUniform4d;
                PFNGLPROGRAMUNIFORM4DVPROC programUniform4dv;
                PFNGLPROGRAMUNIFORM4UIPROC programUniform4ui;
                PFNGLPROGRAMUNIFORM4UIVPROC programUniform4uiv;
                PFNGLPROGRAMUNIFORMMATRIX2FVPROC programUniformMatrix2fv;
                PFNGLPROGRAMUNIFORMMATRIX3FVPROC programUniformMatrix3fv;
                PFNGLPROGRAMUNIFORMMATRIX4FVPROC programUniformMatrix4fv;
                PFNGLPROGRAMUNIFORMMATRIX2DVPROC programUniformMatrix2dv;
                PFNGLPROGRAMUNIFORMMATRIX3DVPROC programUniformMatrix3dv;
                PFNGLPROGRAMUNIFORMMATRIX4DVPROC programUniformMatrix4dv;
                PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC programUniformMatrix2x3fv;
                PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC programUniformMatrix3x2fv;
                PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC programUniformMatrix2x4fv;
                PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC programUniformMatrix4x2fv;
                PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC programUniformMatrix3x4fv;
                PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC programUniformMatrix4x3fv;
                PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC programUniformMatrix2x3dv;
                PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC programUniformMatrix3x2dv;
                PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC programUniformMatrix2x4dv;
                PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC programUniformMatrix4x2dv;
                PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC programUniformMatrix3x4dv;
                PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC programUniformMatrix4x3dv;
                PFNGLVALIDATEPROGRAMPIPELINEPROC validateProgramPipeline;
                PFNGLGETPROGRAMPIPELINEINFOLOGPROC getProgramPipelineInfoLog;
                PFNGLVERTEXATTRIBL1DPROC vertexAttribL1d;
                PFNGLVERTEXATTRIBL2DPROC vertexAttribL2d;
                PFNGLVERTEXATTRIBL3DPROC vertexAttribL3d;
                PFNGLVERTEXATTRIBL4DPROC vertexAttribL4d;
                PFNGLVERTEXATTRIBL1DVPROC vertexAttribL1dv;
                PFNGLVERTEXATTRIBL2DVPROC vertexAttribL2dv;
                PFNGLVERTEXATTRIBL3DVPROC vertexAttribL3dv;
                PFNGLVERTEXATTRIBL4DVPROC vertexAttribL4dv;
                PFNGLVERTEXATTRIBLPOINTERPROC vertexAttribLPointer;
                PFNGLGETVERTEXATTRIBLDVPROC getVertexAttribLdv;
                PFNGLVIEWPORTARRAYVPROC viewportArrayv;
                PFNGLVIEWPORTINDEXEDFPROC viewportIndexedf;
                PFNGLVIEWPORTINDEXEDFVPROC viewportIndexedfv;
                PFNGLSCISSORARRAYVPROC scissorArrayv;
                PFNGLSCISSORINDEXEDPROC scissorIndexed;
                PFNGLSCISSORINDEXEDVPROC scissorIndexedv;
                PFNGLDEPTHRANGEARRAYVPROC depthRangeArrayv;
                PFNGLDEPTHRANGEINDEXEDPROC depthRangeIndexed;
                PFNGLGETFLOATI_VPROC getFloati_v;
                PFNGLGETDOUBLEI_VPROC getDoublei_v;
            };
        }
    }

    using namespace lx0::subsystem::rasterizer_ns;
}
