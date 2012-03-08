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

#include <lx0/core/log/log.hpp>
#include <lx0/subsystem/rasterizer/gl/glinterface.hpp>

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

namespace lx0 
{
    namespace subsystem
    {
        namespace rasterizer_ns
        {

            OpenGlApi3_2::OpenGlApi3_2()
            {
                ::memset(this, sizeof(*this), 0);
            }

            struct FunctionLoader
            {
                FunctionLoader(const char* name)
                {
                    mhModule = ::LoadLibraryA(name);   
                    mfpWglGetProcAddress = (PFNWGLGETPROCADDRESSPROC)::GetProcAddress(mhModule, "wglGetProcAddress");
                }
                ~FunctionLoader()
                {
                    //::FreeLibrary(mhModule);
                }

                template <typename T>
                void load(const char* name, T& t)
                {
                    void* pEntryPoint;

	                pEntryPoint = (*mfpWglGetProcAddress)(name);
                    if (!pEntryPoint)
                        pEntryPoint = ::GetProcAddress(mhModule, name);
	                lx_check_error(pEntryPoint);

                    t = reinterpret_cast<T>(pEntryPoint);
                }

                typedef PROC (APIENTRYP PFNWGLGETPROCADDRESSPROC)(LPCSTR lpszProc);
                HMODULE                  mhModule;
                PFNWGLGETPROCADDRESSPROC mfpWglGetProcAddress;
                
            };

            void OpenGlApi3_2::initialize()
            {
                FunctionLoader module("opengl32.dll");
                module.load("glCullFace", cullFace);
                module.load("glFrontFace", frontFace);
                module.load("glHint", hint);
                module.load("glLineWidth", lineWidth);
                module.load("glPointSize", pointSize);
                module.load("glPolygonMode", polygonMode);
                module.load("glScissor", scissor);
                module.load("glTexParameterf", texParameterf);
                module.load("glTexParameterfv", texParameterfv);
                module.load("glTexParameteri", texParameteri);
                module.load("glTexParameteriv", texParameteriv);
                module.load("glTexImage1D", texImage1D);
                module.load("glTexImage2D", texImage2D);
                module.load("glDrawBuffer", drawBuffer);
                module.load("glClear", clear);
                module.load("glClearColor", clearColor);
                module.load("glClearStencil", clearStencil);
                module.load("glClearDepth", clearDepth);
                module.load("glStencilMask", stencilMask);
                module.load("glColorMask", colorMask);
                module.load("glDepthMask", depthMask);
                module.load("glDisable", disable);
                module.load("glEnable", enable);
                module.load("glFinish", finish);
                module.load("glFlush", flush);
                module.load("glBlendFunc", blendFunc);
                module.load("glLogicOp", logicOp);
                module.load("glStencilFunc", stencilFunc);
                module.load("glStencilOp", stencilOp);
                module.load("glDepthFunc", depthFunc);
                module.load("glPixelStoref", pixelStoref);
                module.load("glPixelStorei", pixelStorei);
                module.load("glReadBuffer", readBuffer);
                module.load("glReadPixels", readPixels);
                module.load("glGetBooleanv", getBooleanv);
                module.load("glGetDoublev", getDoublev);
                module.load("glGetError", getError);
                module.load("glGetFloatv", getFloatv);
                module.load("glGetIntegerv", getIntegerv);
                module.load("glGetString", getString);
                module.load("glGetTexImage", getTexImage);
                module.load("glGetTexParameterfv", getTexParameterfv);
                module.load("glGetTexParameteriv", getTexParameteriv);
                module.load("glGetTexLevelParameterfv", getTexLevelParameterfv);
                module.load("glGetTexLevelParameteriv", getTexLevelParameteriv);
                module.load("glIsEnabled", isEnabled);
                module.load("glDepthRange", depthRange);
                module.load("glViewport", viewport);
                module.load("glDrawArrays", drawArrays);
                module.load("glDrawElements", drawElements);
                module.load("glGetPointerv", getPointerv);
                module.load("glPolygonOffset", polygonOffset);
                module.load("glCopyTexImage1D", copyTexImage1D);
                module.load("glCopyTexImage2D", copyTexImage2D);
                module.load("glCopyTexSubImage1D", copyTexSubImage1D);
                module.load("glCopyTexSubImage2D", copyTexSubImage2D);
                module.load("glTexSubImage1D", texSubImage1D);
                module.load("glTexSubImage2D", texSubImage2D);
                module.load("glBindTexture", bindTexture);
                module.load("glDeleteTextures", deleteTextures);
                module.load("glGenTextures", genTextures);
                module.load("glIsTexture", isTexture);
                module.load("glBlendColor", blendColor);
                module.load("glBlendEquation", blendEquation);
                module.load("glDrawRangeElements", drawRangeElements);
                module.load("glTexImage3D", texImage3D);
                module.load("glTexSubImage3D", texSubImage3D);
                module.load("glCopyTexSubImage3D", copyTexSubImage3D);
                module.load("glActiveTexture", activeTexture);
                module.load("glSampleCoverage", sampleCoverage);
                module.load("glCompressedTexImage3D", compressedTexImage3D);
                module.load("glCompressedTexImage2D", compressedTexImage2D);
                module.load("glCompressedTexImage1D", compressedTexImage1D);
                module.load("glCompressedTexSubImage3D", compressedTexSubImage3D);
                module.load("glCompressedTexSubImage2D", compressedTexSubImage2D);
                module.load("glCompressedTexSubImage1D", compressedTexSubImage1D);
                module.load("glGetCompressedTexImage", getCompressedTexImage);
                module.load("glBlendFuncSeparate", blendFuncSeparate);
                module.load("glMultiDrawArrays", multiDrawArrays);
                module.load("glMultiDrawElements", multiDrawElements);
                module.load("glPointParameterf", pointParameterf);
                module.load("glPointParameterfv", pointParameterfv);
                module.load("glPointParameteri", pointParameteri);
                module.load("glPointParameteriv", pointParameteriv);
                module.load("glGenQueries", genQueries);
                module.load("glDeleteQueries", deleteQueries);
                module.load("glIsQuery", isQuery);
                module.load("glBeginQuery", beginQuery);
                module.load("glEndQuery", endQuery);
                module.load("glGetQueryiv", getQueryiv);
                module.load("glGetQueryObjectiv", getQueryObjectiv);
                module.load("glGetQueryObjectuiv", getQueryObjectuiv);
                module.load("glBindBuffer", bindBuffer);
                module.load("glDeleteBuffers", deleteBuffers);
                module.load("glGenBuffers", genBuffers);
                module.load("glIsBuffer", isBuffer);
                module.load("glBufferData", bufferData);
                module.load("glBufferSubData", bufferSubData);
                module.load("glGetBufferSubData", getBufferSubData);
                module.load("glMapBuffer", mapBuffer);
                module.load("glUnmapBuffer", unmapBuffer);
                module.load("glGetBufferParameteriv", getBufferParameteriv);
                module.load("glGetBufferPointerv", getBufferPointerv);
                module.load("glBlendEquationSeparate", blendEquationSeparate);
                module.load("glDrawBuffers", drawBuffers);
                module.load("glStencilOpSeparate", stencilOpSeparate);
                module.load("glStencilFuncSeparate", stencilFuncSeparate);
                module.load("glStencilMaskSeparate", stencilMaskSeparate);
                module.load("glAttachShader", attachShader);
                module.load("glBindAttribLocation", bindAttribLocation);
                module.load("glCompileShader", compileShader);
                module.load("glCreateProgram", createProgram);
                module.load("glCreateShader", createShader);
                module.load("glDeleteProgram", deleteProgram);
                module.load("glDeleteShader", deleteShader);
                module.load("glDetachShader", detachShader);
                module.load("glDisableVertexAttribArray", disableVertexAttribArray);
                module.load("glEnableVertexAttribArray", enableVertexAttribArray);
                module.load("glGetActiveAttrib", getActiveAttrib);
                module.load("glGetActiveUniform", getActiveUniform);
                module.load("glGetAttachedShaders", getAttachedShaders);
                module.load("glGetAttribLocation", getAttribLocation);
                module.load("glGetProgramiv", getProgramiv);
                module.load("glGetProgramInfoLog", getProgramInfoLog);
                module.load("glGetShaderiv", getShaderiv);
                module.load("glGetShaderInfoLog", getShaderInfoLog);
                module.load("glGetShaderSource", getShaderSource);
                module.load("glGetUniformLocation", getUniformLocation);
                module.load("glGetUniformfv", getUniformfv);
                module.load("glGetUniformiv", getUniformiv);
                module.load("glGetVertexAttribdv", getVertexAttribdv);
                module.load("glGetVertexAttribfv", getVertexAttribfv);
                module.load("glGetVertexAttribiv", getVertexAttribiv);
                module.load("glGetVertexAttribPointerv", getVertexAttribPointerv);
                module.load("glIsProgram", isProgram);
                module.load("glIsShader", isShader);
                module.load("glLinkProgram", linkProgram);
                module.load("glShaderSource", shaderSource);
                module.load("glUseProgram", useProgram);
                module.load("glUniform1f", uniform1f);
                module.load("glUniform2f", uniform2f);
                module.load("glUniform3f", uniform3f);
                module.load("glUniform4f", uniform4f);
                module.load("glUniform1i", uniform1i);
                module.load("glUniform2i", uniform2i);
                module.load("glUniform3i", uniform3i);
                module.load("glUniform4i", uniform4i);
                module.load("glUniform1fv", uniform1fv);
                module.load("glUniform2fv", uniform2fv);
                module.load("glUniform3fv", uniform3fv);
                module.load("glUniform4fv", uniform4fv);
                module.load("glUniform1iv", uniform1iv);
                module.load("glUniform2iv", uniform2iv);
                module.load("glUniform3iv", uniform3iv);
                module.load("glUniform4iv", uniform4iv);
                module.load("glUniformMatrix2fv", uniformMatrix2fv);
                module.load("glUniformMatrix3fv", uniformMatrix3fv);
                module.load("glUniformMatrix4fv", uniformMatrix4fv);
                module.load("glValidateProgram", validateProgram);
                module.load("glVertexAttrib1d", vertexAttrib1d);
                module.load("glVertexAttrib1dv", vertexAttrib1dv);
                module.load("glVertexAttrib1f", vertexAttrib1f);
                module.load("glVertexAttrib1fv", vertexAttrib1fv);
                module.load("glVertexAttrib1s", vertexAttrib1s);
                module.load("glVertexAttrib1sv", vertexAttrib1sv);
                module.load("glVertexAttrib2d", vertexAttrib2d);
                module.load("glVertexAttrib2dv", vertexAttrib2dv);
                module.load("glVertexAttrib2f", vertexAttrib2f);
                module.load("glVertexAttrib2fv", vertexAttrib2fv);
                module.load("glVertexAttrib2s", vertexAttrib2s);
                module.load("glVertexAttrib2sv", vertexAttrib2sv);
                module.load("glVertexAttrib3d", vertexAttrib3d);
                module.load("glVertexAttrib3dv", vertexAttrib3dv);
                module.load("glVertexAttrib3f", vertexAttrib3f);
                module.load("glVertexAttrib3fv", vertexAttrib3fv);
                module.load("glVertexAttrib3s", vertexAttrib3s);
                module.load("glVertexAttrib3sv", vertexAttrib3sv);
                module.load("glVertexAttrib4Nbv", vertexAttrib4Nbv);
                module.load("glVertexAttrib4Niv", vertexAttrib4Niv);
                module.load("glVertexAttrib4Nsv", vertexAttrib4Nsv);
                module.load("glVertexAttrib4Nub", vertexAttrib4Nub);
                module.load("glVertexAttrib4Nubv", vertexAttrib4Nubv);
                module.load("glVertexAttrib4Nuiv", vertexAttrib4Nuiv);
                module.load("glVertexAttrib4Nusv", vertexAttrib4Nusv);
                module.load("glVertexAttrib4bv", vertexAttrib4bv);
                module.load("glVertexAttrib4d", vertexAttrib4d);
                module.load("glVertexAttrib4dv", vertexAttrib4dv);
                module.load("glVertexAttrib4f", vertexAttrib4f);
                module.load("glVertexAttrib4fv", vertexAttrib4fv);
                module.load("glVertexAttrib4iv", vertexAttrib4iv);
                module.load("glVertexAttrib4s", vertexAttrib4s);
                module.load("glVertexAttrib4sv", vertexAttrib4sv);
                module.load("glVertexAttrib4ubv", vertexAttrib4ubv);
                module.load("glVertexAttrib4uiv", vertexAttrib4uiv);
                module.load("glVertexAttrib4usv", vertexAttrib4usv);
                module.load("glVertexAttribPointer", vertexAttribPointer);
                module.load("glUniformMatrix2x3fv", uniformMatrix2x3fv);
                module.load("glUniformMatrix3x2fv", uniformMatrix3x2fv);
                module.load("glUniformMatrix2x4fv", uniformMatrix2x4fv);
                module.load("glUniformMatrix4x2fv", uniformMatrix4x2fv);
                module.load("glUniformMatrix3x4fv", uniformMatrix3x4fv);
                module.load("glUniformMatrix4x3fv", uniformMatrix4x3fv);
                module.load("glColorMaski", colorMaski);
                module.load("glGetBooleani_v", getBooleani_v);
                module.load("glGetIntegeri_v", getIntegeri_v);
                module.load("glEnablei", enablei);
                module.load("glDisablei", disablei);
                module.load("glIsEnabledi", isEnabledi);
                module.load("glBeginTransformFeedback", beginTransformFeedback);
                module.load("glEndTransformFeedback", endTransformFeedback);
                module.load("glBindBufferRange", bindBufferRange);
                module.load("glBindBufferBase", bindBufferBase);
                module.load("glTransformFeedbackVaryings", transformFeedbackVaryings);
                module.load("glGetTransformFeedbackVarying", getTransformFeedbackVarying);
                module.load("glClampColor", clampColor);
                module.load("glBeginConditionalRender", beginConditionalRender);
                module.load("glEndConditionalRender", endConditionalRender);
                module.load("glVertexAttribIPointer", vertexAttribIPointer);
                module.load("glGetVertexAttribIiv", getVertexAttribIiv);
                module.load("glGetVertexAttribIuiv", getVertexAttribIuiv);
                module.load("glVertexAttribI1i", vertexAttribI1i);
                module.load("glVertexAttribI2i", vertexAttribI2i);
                module.load("glVertexAttribI3i", vertexAttribI3i);
                module.load("glVertexAttribI4i", vertexAttribI4i);
                module.load("glVertexAttribI1ui", vertexAttribI1ui);
                module.load("glVertexAttribI2ui", vertexAttribI2ui);
                module.load("glVertexAttribI3ui", vertexAttribI3ui);
                module.load("glVertexAttribI4ui", vertexAttribI4ui);
                module.load("glVertexAttribI1iv", vertexAttribI1iv);
                module.load("glVertexAttribI2iv", vertexAttribI2iv);
                module.load("glVertexAttribI3iv", vertexAttribI3iv);
                module.load("glVertexAttribI4iv", vertexAttribI4iv);
                module.load("glVertexAttribI1uiv", vertexAttribI1uiv);
                module.load("glVertexAttribI2uiv", vertexAttribI2uiv);
                module.load("glVertexAttribI3uiv", vertexAttribI3uiv);
                module.load("glVertexAttribI4uiv", vertexAttribI4uiv);
                module.load("glVertexAttribI4bv", vertexAttribI4bv);
                module.load("glVertexAttribI4sv", vertexAttribI4sv);
                module.load("glVertexAttribI4ubv", vertexAttribI4ubv);
                module.load("glVertexAttribI4usv", vertexAttribI4usv);
                module.load("glGetUniformuiv", getUniformuiv);
                module.load("glBindFragDataLocation", bindFragDataLocation);
                module.load("glGetFragDataLocation", getFragDataLocation);
                module.load("glUniform1ui", uniform1ui);
                module.load("glUniform2ui", uniform2ui);
                module.load("glUniform3ui", uniform3ui);
                module.load("glUniform4ui", uniform4ui);
                module.load("glUniform1uiv", uniform1uiv);
                module.load("glUniform2uiv", uniform2uiv);
                module.load("glUniform3uiv", uniform3uiv);
                module.load("glUniform4uiv", uniform4uiv);
                module.load("glTexParameterIiv", texParameterIiv);
                module.load("glTexParameterIuiv", texParameterIuiv);
                module.load("glGetTexParameterIiv", getTexParameterIiv);
                module.load("glGetTexParameterIuiv", getTexParameterIuiv);
                module.load("glClearBufferiv", clearBufferiv);
                module.load("glClearBufferuiv", clearBufferuiv);
                module.load("glClearBufferfv", clearBufferfv);
                module.load("glClearBufferfi", clearBufferfi);
                module.load("glGetStringi", getStringi);
                module.load("glDrawArraysInstanced", drawArraysInstanced);
                module.load("glDrawElementsInstanced", drawElementsInstanced);
                module.load("glTexBuffer", texBuffer);
                module.load("glPrimitiveRestartIndex", primitiveRestartIndex);
                module.load("glGetInteger64i_v", getInteger64i_v);
                module.load("glGetBufferParameteri64v", getBufferParameteri64v);
                module.load("glFramebufferTexture", framebufferTexture);
                module.load("glVertexAttribDivisor", vertexAttribDivisor);
                module.load("glMinSampleShading", minSampleShading);
                module.load("glBlendEquationi", blendEquationi);
                module.load("glBlendEquationSeparatei", blendEquationSeparatei);
                module.load("glBlendFunci", blendFunci);
                module.load("glBlendFuncSeparatei", blendFuncSeparatei);
                module.load("glIsRenderbuffer", isRenderbuffer);
                module.load("glBindRenderbuffer", bindRenderbuffer);
                module.load("glDeleteRenderbuffers", deleteRenderbuffers);
                module.load("glGenRenderbuffers", genRenderbuffers);
                module.load("glRenderbufferStorage", renderbufferStorage);
                module.load("glGetRenderbufferParameteriv", getRenderbufferParameteriv);
                module.load("glIsFramebuffer", isFramebuffer);
                module.load("glBindFramebuffer", bindFramebuffer);
                module.load("glDeleteFramebuffers", deleteFramebuffers);
                module.load("glGenFramebuffers", genFramebuffers);
                module.load("glCheckFramebufferStatus", checkFramebufferStatus);
                module.load("glFramebufferTexture1D", framebufferTexture1D);
                module.load("glFramebufferTexture2D", framebufferTexture2D);
                module.load("glFramebufferTexture3D", framebufferTexture3D);
                module.load("glFramebufferRenderbuffer", framebufferRenderbuffer);
                module.load("glGetFramebufferAttachmentParameteriv", getFramebufferAttachmentParameteriv);
                module.load("glGenerateMipmap", generateMipmap);
                module.load("glBlitFramebuffer", blitFramebuffer);
                module.load("glRenderbufferStorageMultisample", renderbufferStorageMultisample);
                module.load("glFramebufferTextureLayer", framebufferTextureLayer);
                module.load("glMapBufferRange", mapBufferRange);
                module.load("glFlushMappedBufferRange", flushMappedBufferRange);
                module.load("glBindVertexArray", bindVertexArray);
                module.load("glDeleteVertexArrays", deleteVertexArrays);
                module.load("glGenVertexArrays", genVertexArrays);
                module.load("glIsVertexArray", isVertexArray);
                module.load("glGetUniformIndices", getUniformIndices);
                module.load("glGetActiveUniformsiv", getActiveUniformsiv);
                module.load("glGetActiveUniformName", getActiveUniformName);
                module.load("glGetUniformBlockIndex", getUniformBlockIndex);
                module.load("glGetActiveUniformBlockiv", getActiveUniformBlockiv);
                module.load("glGetActiveUniformBlockName", getActiveUniformBlockName);
                module.load("glUniformBlockBinding", uniformBlockBinding);
                module.load("glCopyBufferSubData", copyBufferSubData);
                module.load("glDrawElementsBaseVertex", drawElementsBaseVertex);
                module.load("glDrawRangeElementsBaseVertex", drawRangeElementsBaseVertex);
                module.load("glDrawElementsInstancedBaseVertex", drawElementsInstancedBaseVertex);
                module.load("glMultiDrawElementsBaseVertex", multiDrawElementsBaseVertex);
                module.load("glProvokingVertex", provokingVertex);
                module.load("glFenceSync", fenceSync);
                module.load("glIsSync", isSync);
                module.load("glDeleteSync", deleteSync);
                module.load("glClientWaitSync", clientWaitSync);
                module.load("glWaitSync", waitSync);
                module.load("glGetInteger64v", getInteger64v);
                module.load("glGetSynciv", getSynciv);
                module.load("glTexImage2DMultisample", texImage2DMultisample);
                module.load("glTexImage3DMultisample", texImage3DMultisample);
                module.load("glGetMultisamplefv", getMultisamplefv);
                module.load("glSampleMaski", sampleMaski);
                module.load("glBindFragDataLocationIndexed", bindFragDataLocationIndexed);
                module.load("glGetFragDataIndex", getFragDataIndex);
                module.load("glGenSamplers", genSamplers);
                module.load("glDeleteSamplers", deleteSamplers);
                module.load("glIsSampler", isSampler);
                module.load("glBindSampler", bindSampler);
                module.load("glSamplerParameteri", samplerParameteri);
                module.load("glSamplerParameteriv", samplerParameteriv);
                module.load("glSamplerParameterf", samplerParameterf);
                module.load("glSamplerParameterfv", samplerParameterfv);
                module.load("glSamplerParameterIiv", samplerParameterIiv);
                module.load("glSamplerParameterIuiv", samplerParameterIuiv);
                module.load("glGetSamplerParameteriv", getSamplerParameteriv);
                module.load("glGetSamplerParameterIiv", getSamplerParameterIiv);
                module.load("glGetSamplerParameterfv", getSamplerParameterfv);
                module.load("glGetSamplerParameterIuiv", getSamplerParameterIuiv);
                module.load("glQueryCounter", queryCounter);
                module.load("glGetQueryObjecti64v", getQueryObjecti64v);
                module.load("glGetQueryObjectui64v", getQueryObjectui64v);
                module.load("glVertexP2ui", vertexP2ui);
                module.load("glVertexP2uiv", vertexP2uiv);
                module.load("glVertexP3ui", vertexP3ui);
                module.load("glVertexP3uiv", vertexP3uiv);
                module.load("glVertexP4ui", vertexP4ui);
                module.load("glVertexP4uiv", vertexP4uiv);
                module.load("glTexCoordP1ui", texCoordP1ui);
                module.load("glTexCoordP1uiv", texCoordP1uiv);
                module.load("glTexCoordP2ui", texCoordP2ui);
                module.load("glTexCoordP2uiv", texCoordP2uiv);
                module.load("glTexCoordP3ui", texCoordP3ui);
                module.load("glTexCoordP3uiv", texCoordP3uiv);
                module.load("glTexCoordP4ui", texCoordP4ui);
                module.load("glTexCoordP4uiv", texCoordP4uiv);
                module.load("glMultiTexCoordP1ui", multiTexCoordP1ui);
                module.load("glMultiTexCoordP1uiv", multiTexCoordP1uiv);
                module.load("glMultiTexCoordP2ui", multiTexCoordP2ui);
                module.load("glMultiTexCoordP2uiv", multiTexCoordP2uiv);
                module.load("glMultiTexCoordP3ui", multiTexCoordP3ui);
                module.load("glMultiTexCoordP3uiv", multiTexCoordP3uiv);
                module.load("glMultiTexCoordP4ui", multiTexCoordP4ui);
                module.load("glMultiTexCoordP4uiv", multiTexCoordP4uiv);
                module.load("glNormalP3ui", normalP3ui);
                module.load("glNormalP3uiv", normalP3uiv);
                module.load("glColorP3ui", colorP3ui);
                module.load("glColorP3uiv", colorP3uiv);
                module.load("glColorP4ui", colorP4ui);
                module.load("glColorP4uiv", colorP4uiv);
                module.load("glSecondaryColorP3ui", secondaryColorP3ui);
                module.load("glSecondaryColorP3uiv", secondaryColorP3uiv);
                module.load("glVertexAttribP1ui", vertexAttribP1ui);
                module.load("glVertexAttribP1uiv", vertexAttribP1uiv);
                module.load("glVertexAttribP2ui", vertexAttribP2ui);
                module.load("glVertexAttribP2uiv", vertexAttribP2uiv);
                module.load("glVertexAttribP3ui", vertexAttribP3ui);
                module.load("glVertexAttribP3uiv", vertexAttribP3uiv);
                module.load("glVertexAttribP4ui", vertexAttribP4ui);
                module.load("glVertexAttribP4uiv", vertexAttribP4uiv);
                module.load("glDrawArraysIndirect", drawArraysIndirect);
                module.load("glDrawElementsIndirect", drawElementsIndirect);
                module.load("glUniform1d", uniform1d);
                module.load("glUniform2d", uniform2d);
                module.load("glUniform3d", uniform3d);
                module.load("glUniform4d", uniform4d);
                module.load("glUniform1dv", uniform1dv);
                module.load("glUniform2dv", uniform2dv);
                module.load("glUniform3dv", uniform3dv);
                module.load("glUniform4dv", uniform4dv);
                module.load("glUniformMatrix2dv", uniformMatrix2dv);
                module.load("glUniformMatrix3dv", uniformMatrix3dv);
                module.load("glUniformMatrix4dv", uniformMatrix4dv);
                module.load("glUniformMatrix2x3dv", uniformMatrix2x3dv);
                module.load("glUniformMatrix2x4dv", uniformMatrix2x4dv);
                module.load("glUniformMatrix3x2dv", uniformMatrix3x2dv);
                module.load("glUniformMatrix3x4dv", uniformMatrix3x4dv);
                module.load("glUniformMatrix4x2dv", uniformMatrix4x2dv);
                module.load("glUniformMatrix4x3dv", uniformMatrix4x3dv);
                module.load("glGetUniformdv", getUniformdv);
                module.load("glBindTransformFeedback", bindTransformFeedback);
                module.load("glDeleteTransformFeedbacks", deleteTransformFeedbacks);
                module.load("glGenTransformFeedbacks", genTransformFeedbacks);
                module.load("glIsTransformFeedback", isTransformFeedback);
                module.load("glPauseTransformFeedback", pauseTransformFeedback);
                module.load("glResumeTransformFeedback", resumeTransformFeedback);
                module.load("glDrawTransformFeedback", drawTransformFeedback);
                module.load("glDrawTransformFeedbackStream", drawTransformFeedbackStream);
                module.load("glBeginQueryIndexed", beginQueryIndexed);
                module.load("glEndQueryIndexed", endQueryIndexed);
                module.load("glGetQueryIndexediv", getQueryIndexediv);
                module.load("glReleaseShaderCompiler", releaseShaderCompiler);
                module.load("glShaderBinary", shaderBinary);
                module.load("glGetShaderPrecisionFormat", getShaderPrecisionFormat);
                module.load("glDepthRangef", depthRangef);
                module.load("glClearDepthf", clearDepthf);
                module.load("glGetProgramBinary", getProgramBinary);
                module.load("glProgramBinary", programBinary);
                module.load("glProgramParameteri", programParameteri);
                module.load("glUseProgramStages", useProgramStages);
                module.load("glActiveShaderProgram", activeShaderProgram);
                module.load("glCreateShaderProgramv", createShaderProgramv);
                module.load("glBindProgramPipeline", bindProgramPipeline);
                module.load("glDeleteProgramPipelines", deleteProgramPipelines);
                module.load("glGenProgramPipelines", genProgramPipelines);
                module.load("glIsProgramPipeline", isProgramPipeline);
                module.load("glGetProgramPipelineiv", getProgramPipelineiv);
                module.load("glProgramUniform1i", programUniform1i);
                module.load("glProgramUniform1iv", programUniform1iv);
                module.load("glProgramUniform1f", programUniform1f);
                module.load("glProgramUniform1fv", programUniform1fv);
                module.load("glProgramUniform1d", programUniform1d);
                module.load("glProgramUniform1dv", programUniform1dv);
                module.load("glProgramUniform1ui", programUniform1ui);
                module.load("glProgramUniform1uiv", programUniform1uiv);
                module.load("glProgramUniform2i", programUniform2i);
                module.load("glProgramUniform2iv", programUniform2iv);
                module.load("glProgramUniform2f", programUniform2f);
                module.load("glProgramUniform2fv", programUniform2fv);
                module.load("glProgramUniform2d", programUniform2d);
                module.load("glProgramUniform2dv", programUniform2dv);
                module.load("glProgramUniform2ui", programUniform2ui);
                module.load("glProgramUniform2uiv", programUniform2uiv);
                module.load("glProgramUniform3i", programUniform3i);
                module.load("glProgramUniform3iv", programUniform3iv);
                module.load("glProgramUniform3f", programUniform3f);
                module.load("glProgramUniform3fv", programUniform3fv);
                module.load("glProgramUniform3d", programUniform3d);
                module.load("glProgramUniform3dv", programUniform3dv);
                module.load("glProgramUniform3ui", programUniform3ui);
                module.load("glProgramUniform3uiv", programUniform3uiv);
                module.load("glProgramUniform4i", programUniform4i);
                module.load("glProgramUniform4iv", programUniform4iv);
                module.load("glProgramUniform4f", programUniform4f);
                module.load("glProgramUniform4fv", programUniform4fv);
                module.load("glProgramUniform4d", programUniform4d);
                module.load("glProgramUniform4dv", programUniform4dv);
                module.load("glProgramUniform4ui", programUniform4ui);
                module.load("glProgramUniform4uiv", programUniform4uiv);
                module.load("glProgramUniformMatrix2fv", programUniformMatrix2fv);
                module.load("glProgramUniformMatrix3fv", programUniformMatrix3fv);
                module.load("glProgramUniformMatrix4fv", programUniformMatrix4fv);
                module.load("glProgramUniformMatrix2dv", programUniformMatrix2dv);
                module.load("glProgramUniformMatrix3dv", programUniformMatrix3dv);
                module.load("glProgramUniformMatrix4dv", programUniformMatrix4dv);
                module.load("glProgramUniformMatrix2x3fv", programUniformMatrix2x3fv);
                module.load("glProgramUniformMatrix3x2fv", programUniformMatrix3x2fv);
                module.load("glProgramUniformMatrix2x4fv", programUniformMatrix2x4fv);
                module.load("glProgramUniformMatrix4x2fv", programUniformMatrix4x2fv);
                module.load("glProgramUniformMatrix3x4fv", programUniformMatrix3x4fv);
                module.load("glProgramUniformMatrix4x3fv", programUniformMatrix4x3fv);
                module.load("glProgramUniformMatrix2x3dv", programUniformMatrix2x3dv);
                module.load("glProgramUniformMatrix3x2dv", programUniformMatrix3x2dv);
                module.load("glProgramUniformMatrix2x4dv", programUniformMatrix2x4dv);
                module.load("glProgramUniformMatrix4x2dv", programUniformMatrix4x2dv);
                module.load("glProgramUniformMatrix3x4dv", programUniformMatrix3x4dv);
                module.load("glProgramUniformMatrix4x3dv", programUniformMatrix4x3dv);
            }
        }
    }

    using namespace lx0::subsystem::rasterizer_ns;
}