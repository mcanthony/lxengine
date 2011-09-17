//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

#include <lx0/lxengine.hpp>
#include <lx0/subsystem/canvas.hpp>

#include <windows.h>
#include <windowsx.h>
#include "windowclass.hpp"

#include <GL3/gl3w_modified.hpp>

// Inline the WGL defines since this is the only place they're used
#define WGL_CONTEXT_MAJOR_VERSION_ARB		        0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB		        0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB		            0x2093
#define WGL_CONTEXT_FLAGS_ARB			            0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB		        0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB		            0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB	    0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB	        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB   0x00000002

typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int* attribList);

#define WGL_NUMBER_PIXEL_FORMATS_ARB                0x2000
#define WGL_DRAW_TO_WINDOW_ARB                      0x2001
#define WGL_DRAW_TO_BITMAP_ARB                      0x2002
#define WGL_ACCELERATION_ARB                        0x2003
#define WGL_NEED_PALETTE_ARB                        0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB                 0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB                  0x2006
#define WGL_SWAP_METHOD_ARB                         0x2007
#define WGL_NUMBER_OVERLAYS_ARB                     0x2008
#define WGL_NUMBER_UNDERLAYS_ARB                    0x2009
#define WGL_TRANSPARENT_ARB                         0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB               0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB             0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB              0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB             0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB             0x203B
#define WGL_SHARE_DEPTH_ARB                         0x200C
#define WGL_SHARE_STENCIL_ARB                       0x200D
#define WGL_SHARE_ACCUM_ARB                         0x200E
#define WGL_SUPPORT_GDI_ARB                         0x200F
#define WGL_SUPPORT_OPENGL_ARB                      0x2010
#define WGL_DOUBLE_BUFFER_ARB                       0x2011
#define WGL_STEREO_ARB                              0x2012
#define WGL_PIXEL_TYPE_ARB                          0x2013
#define WGL_COLOR_BITS_ARB                          0x2014
#define WGL_RED_BITS_ARB                            0x2015
#define WGL_RED_SHIFT_ARB                           0x2016
#define WGL_GREEN_BITS_ARB                          0x2017
#define WGL_GREEN_SHIFT_ARB                         0x2018
#define WGL_BLUE_BITS_ARB                           0x2019
#define WGL_BLUE_SHIFT_ARB                          0x201A
#define WGL_ALPHA_BITS_ARB                          0x201B
#define WGL_ALPHA_SHIFT_ARB                         0x201C
#define WGL_ACCUM_BITS_ARB                          0x201D
#define WGL_ACCUM_RED_BITS_ARB                      0x201E
#define WGL_ACCUM_GREEN_BITS_ARB                    0x201F
#define WGL_ACCUM_BLUE_BITS_ARB                     0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB                    0x2021
#define WGL_DEPTH_BITS_ARB                          0x2022
#define WGL_STENCIL_BITS_ARB                        0x2023
#define WGL_AUX_BUFFERS_ARB                         0x2024

#define WGL_NO_ACCELERATION_ARB                     0x2025
#define WGL_GENERIC_ACCELERATION_ARB                0x2026
#define WGL_FULL_ACCELERATION_ARB                   0x2027
#define WGL_SWAP_EXCHANGE_ARB                       0x2028
#define WGL_SWAP_COPY_ARB                           0x2029
#define WGL_SWAP_UNDEFINED_ARB                      0x202A
#define WGL_TYPE_RGBA_ARB                           0x202B
#define WGL_TYPE_COLORINDEX_ARB                     0x202C

#define WGL_SAMPLE_BUFFERS_ARB	                    0x2041
#define WGL_SAMPLES_ARB		                        0x2042
typedef BOOL  (WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;
static PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARB    = 0;

#define m_hWnd reinterpret_cast<HWND&>(mOpaqueHwnd)
#define m_hDC  reinterpret_cast<HDC&>(m_opaque_hDC)
#define m_hRC  reinterpret_cast<HGLRC&>(m_opaque_hRC)

using namespace lx0::subsystem::canvas_ns::detail;

namespace
{
    static void
    _initPixelFormatDescriptor (PIXELFORMATDESCRIPTOR& pfd)
    {
        ::memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
        pfd.nSize = sizeof(pfd);
        pfd.dwFlags |= PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.iLayerType = PFD_MAIN_PLANE;
        pfd.cColorBits = 32;
    }

    static GLuint 
    _chooseGenericPixelFormat (HDC hDC, PIXELFORMATDESCRIPTOR& pfd)
    {
        _initPixelFormatDescriptor(pfd);
        return ::ChoosePixelFormat(hDC, &pfd);
    }

    static GLuint
    _chooseExtendedPixelFormat (HDC hDC)
    {
        

        //
        // Set up the desired attributes
        //
        std::vector<int> vAttributes;
        vAttributes.reserve(64 * 2);
        auto add_attrib = [&vAttributes](int name, int value) -> int& {
            vAttributes.push_back(name);
            vAttributes.push_back(value);
            return vAttributes.back();
        };
                              add_attrib(WGL_DRAW_TO_WINDOW_ARB, GL_TRUE);
                              add_attrib(WGL_SUPPORT_OPENGL_ARB, GL_TRUE);
                              add_attrib(WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB);
                              add_attrib(WGL_COLOR_BITS_ARB,     24);
                              add_attrib(WGL_ALPHA_BITS_ARB,     8);
                              add_attrib(WGL_DEPTH_BITS_ARB,     16);
                              add_attrib(WGL_STENCIL_BITS_ARB,   0);
                              add_attrib(WGL_DOUBLE_BUFFER_ARB,  GL_TRUE);
        auto& sampleBuffers = add_attrib(WGL_SAMPLE_BUFFERS_ARB, GL_TRUE);
        auto& sampleCount   = add_attrib(WGL_SAMPLES_ARB,        4);
                              add_attrib(0, 0);

        //
        // Local function to try multiple formats
        //
        GLuint format = 0;
        auto try_format = [&] () -> GLuint {
            int pixelFormat[16];
            unsigned int numFormats;
            bool bValid = !!wglChoosePixelFormatARB(hDC, &vAttributes[0], nullptr, 16, &pixelFormat[0], &numFormats);
 
	        // If We Returned True, And Our Format Count Is Greater Than 1
	        if (bValid && numFormats >= 1)
	            return format = pixelFormat[0];
            else
                return format = 0;
        };

        //
        // Return on the first successful format
        //
        for (int i = 16; i >= 1; --i)
        {
            sampleCount = i;
            if (try_format())
                return format;
        }
        lx_warn("Could not find a multi-sampling pixel format");

        //
        // Drop multi-sample support and try again...
        // 
        vAttributes.clear();
        add_attrib(WGL_DRAW_TO_WINDOW_ARB, GL_TRUE);
        add_attrib(WGL_SUPPORT_OPENGL_ARB, GL_TRUE);
        add_attrib(WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB);
        add_attrib(WGL_COLOR_BITS_ARB,     24);
        add_attrib(WGL_ALPHA_BITS_ARB,     8);
        add_attrib(WGL_DEPTH_BITS_ARB,     16);
        add_attrib(WGL_STENCIL_BITS_ARB,   0);
        add_attrib(WGL_DOUBLE_BUFFER_ARB,  GL_TRUE);
        add_attrib(0, 0);

        if (try_format())
            return format;

        //
        // No pixel format whatsoever was found.
        //
        throw lx_error_exception("Could not find a valid pixel format!");
        return 0;
    }

    /*
        Creates a temporary OpenGL-enabled window with no extensions.  This window
        is then used to load the extension functions and find a pixel format for the
        "real" window we'll be using later.

        This double window creation technique is the standard way to create a OpenGL
        window with extended attributes (e.g. multisampling).
     */
    unsigned int
    _createTempWindow (void)
    {
        Win32WindowClass tempClass("tempGL", ::DefWindowProc);
        tempClass.registerClass();

        DWORD dwStyle   = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

        // The input width and height refer to the client area.   Adjust the
        // total window size accordingly based on the window frame style.
        //
        RECT rect = { 0, 0, 256, 256 };
        HINSTANCE hInstance = GetModuleHandle(NULL);
        HWND hWnd = CreateWindowEx(dwExStyle, 
                         tempClass.className(), 
                         "tempGL Window",
                         dwStyle,
                         0,
                         0,
                         rect.right - rect.left,
                         rect.bottom - rect.top,
                         NULL, NULL, 
                         hInstance, 
                         NULL);	
        if (!hWnd)    
            throw lx_error_exception("Could not create window");

        HDC   hDC = GetDC(hWnd);
        HGLRC hRC = 0;

        PIXELFORMATDESCRIPTOR pfd;
        GLuint uPixelFormat = _chooseGenericPixelFormat(hDC, pfd);
        if (!uPixelFormat)
            throw lx_error_exception("Cannot find suitable pixel format");

        if (::SetPixelFormat(hDC, uPixelFormat, &pfd) == FALSE)
            throw lx_error_exception("Cannot set pixel format");

        //
        // Create a temporary context so that the extensions will initialize properly
        //
        HGLRC tempRC = wglCreateContext(hDC);
        if (!tempRC)
            throw lx_error_exception("Cannot create OpenGL context");
        if (!wglMakeCurrent(hDC, tempRC))
            throw lx_error_exception("Cannot set rendering context");

        //
        // Initialize (pulls in all the OpenGL extension functions)
        //
        gl3wInit();
        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");
        wglChoosePixelFormatARB    = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

        //
        // Now that OpenGL is initialized, we can check look for better pixel formats
        // (e.g. multi-sampling).  It's a known annoyance that an OpenGL window has
        // to be initialized before it can be queryed fully to see how you want to
        // initialize it.
        //
        GLuint pixelFormatExtended = _chooseExtendedPixelFormat(hDC);

        //
        // Clean up all these temporary objects
        //
        wglMakeCurrent(0, 0);
        wglDeleteContext(tempRC);
        ::DestroyWindow(hWnd);
        tempClass.unregisterClass();
    
        return pixelFormatExtended;
    }
}


namespace lx0 { namespace subsystem { namespace canvas_ns { namespace detail {

    CanvasGL::CanvasGL (const char* pszTitle, int x, int y, int w, int h, bool bResizable)
        : m_opaque_hDC      (0)
        , m_opaque_hRC      (0)
        , mRedrawActive     (false)
        , mPixelFormat      (0)
    {
        // Determine the pixel format by creating a temporary window first, then 
        // create the real window.
        mPixelFormat = _createTempWindow();
        _createWindow(pszTitle, x, y, w, h, bResizable);
    }

    CanvasGL::~CanvasGL (void)
    {
        lx_check_error(m_hRC == 0, "Rendering context should be destroyed before the object is deleted.");
        lx_check_error(m_hDC == 0, "Device context should be released before the object is deleted.");
    }

    void 
    CanvasGL::swapBuffers (void) 
    {
        ::SwapBuffers(m_hDC);
    }


    void   
    CanvasGL::_createWindow (const char* pszTitle, int x, int y, int w, int h, bool bResizeable)
    {
        DWORD dwStyle   = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

        // Check various options
        //
        if (bResizeable) 
            dwStyle |= WS_SIZEBOX;
        else
            dwStyle &= ~WS_SIZEBOX;

        // The input width and height refer to the client area.   Adjust the
        // total window size accordingly based on the window frame style.
        //
        RECT rect = { 0, 0, w, h };
        ::AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);

        HINSTANCE hInstance = GetModuleHandle(NULL);
        HWND hWnd = CreateWindowEx(dwExStyle, 
                         s_windowClass.className(), 
                         pszTitle,
                         dwStyle,
                         x,
                         y,
                         rect.right - rect.left,
                         rect.bottom - rect.top,
                         NULL, NULL, 
                         hInstance, 
                         this);	
        if (!hWnd)    
            throw lx_error_exception("Could not create window");
    }


    // This is a lower-level hook than the imp/slot mechanism.  Providing a custom WNDPROC allows the 
    // derived class to implement message-specific behavior for any arbitrary message without 
    // imposing that the base class provide wrapper signals for every signal event.
    //
    int __stdcall CanvasGL::windowProc(void* hWnd, unsigned int uMsg, unsigned int* wParam, long* lParam )
    {
        switch (uMsg)
        {
        case WM_ERASEBKGND:
            {
                // See NeHe Productions Lesson 42, should reduce flicker
                return 0;
            }
            break;

        case WM_GETMINMAXINFO:
            {
                // Since this is a window intended for rendering, set a minimum size
                //
                MINMAXINFO* pInfo = reinterpret_cast<LPMINMAXINFO>(lParam);
                pInfo->ptMinTrackSize.x = 200;
                pInfo->ptMinTrackSize.y = 200;
            }
            break;
        }

        return CanvasBase::windowProc(hWnd, uMsg, wParam, lParam);
    }  

    void CanvasGL::impCreate() 
    {
        // The OpenGL does have some custom window handlers that aren't general enough
        // to warrant having wrappers in the parent class.
        overrideParentWndProc(windowProc);

        //
        // Create the rendering context
        //
        createGlContext();
    }

    void CanvasGL::impDestroy()
    {
        destroyGlContext();
    }

    bool CanvasGL::impRedraw()
    {
        if (!mRedrawActive)
        {
            mRedrawActive = true;
            try
            {
                // The default behavior of the imp/signal is to call the imp method, then the signals.
                // However, in this case the imp methods wants to swap the buffer *after* the slots
                // have been called.  
                // 
                slotRedraw();

                ::SwapBuffers(::GetDC(m_hWnd));
            }
            catch (lx0::error_exception& e)
            {
                // Prevent the exception from slipping into the Windows OS message handling code.
                // The message handling code would blindly consume and ignore the exception, which
                // is confusing to the developer.  Since the exception cannot be passed directly
                // up the call stack, postpone the exception until the next Engine update().
                Engine::acquire()->postponeException(e);
            }
            mRedrawActive = false;
        }
        else
            lx_warn("Recursive redraw detected.  Ignoring recursive call.");

        return false;
    }

    bool CanvasGL::impResize(int width, int height) 
    {
        // This might not be helpful if the client code is rendering to multiple sub-windows,
        // but it most cases it will be helpful and not cause problems for the multiple
        // sub-window client.
        //
        glViewport(0, 0, width, height);

        return true;
    }   

    /*!
        Creating the context is composed of several steps:

        1) Create the "pixel format" - i.e. choose a rendering context type from among those
           available on the hardware.  Not all graphics cards support the same set of formats.
    
        2) Initialize the OpenGL extensions.

        3) Create the OpenGL 3.2 context using the pixel format found in (1) and the methods
           made available via extensions in (2).
     */
    void 
    CanvasGL::createGlContext()
    {
        lx_check_error(m_hDC == 0, "Context is non-zero.  Already initialized?");
        lx_check_error(m_hRC == 0, "Context is non-zero.  Already initialized?");

        m_hDC = GetDC(m_hWnd);
        m_hRC = 0;

        PIXELFORMATDESCRIPTOR pfd;
        _initPixelFormatDescriptor(pfd);
        if (::SetPixelFormat(m_hDC, mPixelFormat, &pfd) == FALSE)
            throw lx_error_exception("Cannot set pixel format");
     
        //
        // Create the OpenGL 3.2 context using wglCreateContextAttribsARB.
        //
        // Note that we're using the "compatibility profile", even though LxEngine
        // itself aims to use on the core profile.  Why?  Third-party code or 
        // sample apps outside the LxEngine core might use the Canvas class and
        // wanted compatibility mode.
        //
        int attribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 2,
            WGL_CONTEXT_FLAGS_ARB, 
            WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            WGL_CONTEXT_PROFILE_MASK_ARB, 
            WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
            0,0
        };
    
        if (wglCreateContextAttribsARB)
        {
            // Create the new context and delete the previous created temporary context
            m_hRC = wglCreateContextAttribsARB(m_hDC, 0, attribs);  
            wglMakeCurrent(m_hDC, m_hRC);
        }
        else
            throw lx_error_exception("Using a pre-OpenGL 3.2 context");
    
        // Check post-conditions
        //
        lx_check_error(m_hRC != 0, "Valid rendering context not created!");
        lx_check_error(m_hDC != 0, "Valid device context not created!");
    }

    void
    CanvasGL::destroyGlContext()
    {
        // Release the rendering context
        if (m_hRC) 
        {
            if (!wglMakeCurrent(0, 0))					
                throw lx_error_exception("Release Of DC And RC Failed.");

            if (!wglDeleteContext(m_hRC))
                throw lx_error_exception("Release Rendering Context Failed.");

            m_hRC = 0;
        }

        if (m_hDC) 
        {
            if (!ReleaseDC(m_hWnd, m_hDC))
                throw lx_error_exception("Cannot Release Window Context");
            m_hDC = 0;
        }
    }

}}}}
