//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

    Copyright (c) 2010-2011 athile@athile.net (http://www.athile.net)

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

#include "lx0/canvas/canvas.hpp"
#include "lx0/core/core.hpp"

#include <windows.h>
#include <windowsx.h>

#include <gl/glew.h>
#include <gl/wglew.h>
#include <gl/GL.h>


using namespace lx0::core;

#define m_hWnd reinterpret_cast<HWND&>(mOpaqueHwnd)
#define m_hDC  reinterpret_cast<HDC&>(m_opaque_hDC)
#define m_hRC  reinterpret_cast<HGLRC&>(m_opaque_hRC)

namespace lx0 { namespace canvas { namespace platform {

    //===========================================================================//

    namespace {
    
        int
        s_winToLxKey (int winKey)
        {
            if (winKey >= 'A' && winKey <= 'Z')
                return KC_A + (winKey - 'A');

            switch (winKey)
            {
            case VK_ESCAPE: return KC_ESCAPE;
            default:        return 0;
            }
        }
        
        void 
        s_getKeyState (KeyboardState& lxState, int eventVKey)
        {
            BYTE winState[256];
            if(::GetKeyboardState(winState) != FALSE)
            {
                // The most significant bit indicates if the key is down
                for (int i = 0; i < 256; ++i)
                    lxState.bDown[s_winToLxKey(i)] = !!(winState[i] & 0x80);

                // GetKeyboardState() returns the keyboard state after the events
                // have been processed - i.e. not the current key if this function
                // is called in the key event handler itself.  Therefore, manually
                // check the key associated with the event.
                // 
                // http://msdn.microsoft.com/en-us/library/ms646299(v=vs.85).aspx.  See Remarks
                // section.
                //
                lxState.bDown[s_winToLxKey(eventVKey)] = !(::GetAsyncKeyState(eventVKey) && 0x8000);
            }
            else
            {
                lx_error("Call to Win32 GetKeyboardState() failed!");
                
                for (int i = 0; i < KC_COUNT; ++i)
                    lxState.bDown[i] = false;
            }
        }
    }

    //===========================================================================//

    KeyboardState::KeyboardState()
    {
        for (int i = 0; i < KC_COUNT; ++i)
            bDown[i] = false;
    }

    //===========================================================================//
    //!
    /*!
     */
    class Win32WindowClass
    {
    public:
                        Win32WindowClass    (WNDPROC wndProc);
                        ~Win32WindowClass   (void);

        const char*     className           (void) const { return "LxCanvas"; }
        void            registerClass       (void);
        void            unregisterClass     (void);
    
    
    protected:
        int         m_refCount;
        WNDPROC     m_wndProc;
        WNDCLASSEX  m_wndClass;
    };

    Win32WindowClass::Win32WindowClass (WNDPROC wndProc)
        : m_refCount (0)
        , m_wndProc  (wndProc)
    {

    }

    Win32WindowClass::~Win32WindowClass (void)
    {
        lx_check_error(m_refCount == 0, "Not all references to the window class released");
    }

    void Win32WindowClass::registerClass (void)
    {
        if (m_refCount++ == 0)
        {
            ::memset(&m_wndClass, 0, sizeof(WNDCLASSEX));            
            m_wndClass.cbSize        = sizeof(WNDCLASSEX);
            m_wndClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            m_wndClass.lpfnWndProc   = m_wndProc;
            m_wndClass.cbClsExtra    = 0;
            m_wndClass.cbWndExtra    = 0;
            m_wndClass.hInstance     = ::GetModuleHandle(NULL);
            m_wndClass.hCursor       = LoadCursor(0, IDC_ARROW);
            m_wndClass.lpszClassName = className();

            if (!::RegisterClassEx(&m_wndClass))
                lx_error("Cannot register window class");
        }
    }

    void Win32WindowClass::unregisterClass (void)
    {
        if (--m_refCount == 0)
        {
            if (!::UnregisterClass(className(), ::GetModuleHandle(NULL)))
                lx_error("Unregistering class failed.");
        }
    }

    //===========================================================================//

    Win32WindowClass CanvasBase::s_windowClass((WNDPROC)CanvasBase::windowProc);

    CanvasBase::CanvasBase()
        : mOpaqueHwnd (NULL)
    {
        s_windowClass.registerClass();
    }

    CanvasBase::~CanvasBase()
    {
        lx_check_error(m_hWnd == NULL, "Call destroy() on the window before deleting it! "
            "This allows for proper clean-up of other objects which may depend on the window.");
    }

    size_t
    CanvasBase::handle (void)
    {
        return reinterpret_cast<size_t>(m_hWnd);
    }

    void
    CanvasBase::show (void) 
    { 
        // Due to strange behavior when calling this during the WM_CREATE message, move the keyboard
        // initialization to the show call.  This is *not* ideal, but the prior code certainly
        // was not working.
        s_getKeyState(mKeyboard, 0);

        ::ShowWindow((HWND)m_hWnd, SW_SHOWDEFAULT);  
        ::UpdateWindow((HWND)m_hWnd); 
    }

    void
    CanvasBase::invalidate (void)
    {
        // Notify to Windows that the full window must be redrawn.  
        // This signals a WM_PAINT message.
        ::InvalidateRect((HWND)m_hWnd, NULL, FALSE);
    }

    void        
    CanvasBase::destroy (void) 
    { 
        ::DestroyWindow((HWND)m_hWnd);
        m_hWnd = 0; 

        s_windowClass.unregisterClass();
    }

    const KeyboardState&    
    CanvasBase::keyboard (void) const
    {
        return mKeyboard;
    }

    void 
    CanvasBase::overrideParentWndProc (void* wndProc)
    {
        ::SetWindowLongPtr((HWND)m_hWnd, GWLP_WNDPROC, (LONG)(LONG_PTR)wndProc);
    }


    static int
    _handleMouseDown (ButtonState& button, LPARAM lParam)
    {
        const int clientX = GET_X_LPARAM(lParam);
        const int clientY = GET_Y_LPARAM(lParam);
                
        button.bDown = true;
        button.bDragging = false;
        button.startX = clientX;
        button.startY = clientY;

        return 0;
    }

    static bool
    _handleMouseUp (ButtonState& button, LPARAM lParam)
    {
        const int clientX = GET_X_LPARAM(lParam);
        const int clientY = GET_Y_LPARAM(lParam);

        button.bDown = false;

        if (!button.bDragging)
        {
            return true;
        }
        else
        {
            button.bDragging = false;
            return false;
        }
    }

    int CALLBACK 
    CanvasBase::windowProc (void* hWnd_, unsigned int uMsg_, unsigned int* wParam_, long* lParam_)
    {
        HWND hWnd = (HWND)hWnd_;
        UINT uMsg = (UINT)uMsg_;
        WPARAM wParam = (WPARAM)wParam_;
        LPARAM lParam = (LPARAM)lParam_;

        // The "this" pointer has been stored in the GWL_USERDATA slot of the window.  See WM_CREATE.
        CanvasBase* pWin = reinterpret_cast<CanvasBase*>((LONG_PTR)GetWindowLongPtr(hWnd, GWL_USERDATA));

        switch (uMsg)
        {
            case WM_CREATE:
                {
                    LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
                    CanvasBase* pWin = reinterpret_cast<CanvasBase*>(pCreateStruct->lpCreateParams);
                    pWin->mOpaqueHwnd = (void*)hWnd;
                    SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG)(LONG_PTR)pWin);

                    pWin->impCreate();
                    pWin->slotCreate();               
            
                    return 0;   
                }

            case WM_KEYDOWN:
                {
                    ///@todo Add the ctrl-shift extended key state
                    unsigned int keyCode = (UINT)(wParam);

                    pWin->mKeyboard.bDown[s_winToLxKey(keyCode)] = true;

                    if (pWin->impKeyDown(keyCode))
                        pWin->slotKeyDown(keyCode);
                }
                break;
        
            case WM_KEYUP:
                {
                    ///@todo Add the ctrl-shift extended key state
                    unsigned int keyCode = (UINT)(wParam);
                    
                    pWin->mKeyboard.bDown[s_winToLxKey(keyCode)] = false;

                    if (pWin->impKeyUp(keyCode))
                        pWin->slotKeyUp(keyCode);
                }
                break;


            case WM_CLOSE:
                {
                    if (pWin->impClose())
                        pWin->slotClose();
                }
                break;
        
            case WM_DESTROY:
                {
                    pWin->impDestroy();
                    pWin->slotDestroy();
                }
                break;

            case WM_SIZE:
                {
                    int iWidth  = LOWORD(lParam);
                    int iHeight = HIWORD(lParam);                    

                    if (pWin->impResize(iWidth, iHeight))
                        pWin->slotResize(iWidth, iHeight);

                    return 0;
                }			    
                break;
        
            case WM_PAINT: 
                {
                    if (pWin->impRedraw())
                        pWin->slotRedraw();                
                    return DefWindowProc(hWnd, uMsg, wParam, lParam);
                }
                break;

            case WM_LBUTTONDOWN: return _handleMouseDown(pWin->mLButton, lParam);
            case WM_MBUTTONDOWN: return _handleMouseDown(pWin->mMButton, lParam);
            case WM_RBUTTONDOWN: return _handleMouseDown(pWin->mRButton, lParam);

            case WM_LBUTTONUP:      
                {   
                    if (_handleMouseUp(pWin->mLButton, lParam))
                    {
                        if (pWin->impLMouseClick(pWin->mMouse, pWin->mLButton, KeyModifiers()))
                            pWin->slotLMouseClick(pWin->mMouse, pWin->mLButton, KeyModifiers());
                    }
                } 
                return 0;

            case WM_MBUTTONUP:      
                {   
                    if (_handleMouseUp(pWin->mMButton, lParam))
                    {
                        if (pWin->impMMouseClick(pWin->mMouse, pWin->mMButton, KeyModifiers()))
                            pWin->slotMMouseClick(pWin->mMouse, pWin->mMButton, KeyModifiers());
                    }
                } 
                return 0;

            case WM_RBUTTONUP:      
                {   
                    if (_handleMouseUp(pWin->mRButton, lParam))
                    {
                        if (pWin->impRMouseClick(pWin->mMouse, pWin->mRButton, KeyModifiers()))
                            pWin->slotRMouseClick(pWin->mMouse, pWin->mRButton, KeyModifiers());
                    }
                } 
                return 0;

            case WM_MOUSEMOVE:
                {
                    int clientX = GET_X_LPARAM(lParam); 
                    int clientY = GET_Y_LPARAM(lParam); 

                    pWin->mMouse.previousX = pWin->mMouse.x;
                    pWin->mMouse.previousY = pWin->mMouse.y;
                    pWin->mMouse.x = clientX;
                    pWin->mMouse.y = clientY;

                    if (pWin->mLButton.bDown)
                    {
                        pWin->mLButton.bDragging = true;

                        if (pWin->impLMouseDrag(pWin->mMouse, pWin->mLButton, KeyModifiers()))
                            pWin->slotLMouseDrag(pWin->mMouse, pWin->mLButton, KeyModifiers());
                    }
                    if (pWin->mMButton.bDown)
                    {
                        pWin->mMButton.bDragging = true;

                        if (pWin->impMMouseDrag(pWin->mMouse, pWin->mMButton, KeyModifiers()))
                            pWin->slotMMouseDrag(pWin->mMouse, pWin->mMButton, KeyModifiers());
                    }
                    if (pWin->mRButton.bDown)
                    {
                        pWin->mRButton.bDragging = true;

                        if (pWin->impRMouseDrag(pWin->mMouse, pWin->mRButton, KeyModifiers()))
                            pWin->slotRMouseDrag(pWin->mMouse, pWin->mRButton, KeyModifiers());
                    }
                }
                break;

            case WM_MOUSEWHEEL:
                {
                    const int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
                    if (pWin->impMouseWheel(pWin->mMouse, wheelDelta))
                        pWin->slotMouseWheel(pWin->mMouse, wheelDelta);
                
                }
                return 0;

            default:
                return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
        }

        return 0;
    }


    //===========================================================================//

    CanvasGL::CanvasGL (const char* pszTitle, int w, int h, bool bResizeable)
        : m_opaque_hDC     (0)
        , m_opaque_hRC   (0)
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
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         rect.right - rect.left,
                         rect.bottom - rect.top,
                         NULL, NULL, 
                         hInstance, 
                         this);	
        if (!hWnd)    
            lx_error("Could not create window");
    }

    CanvasGL::~CanvasGL (void)
    {
        lx_check_error(m_hRC == 0, "Rendering context should be destroyed before the object is deleted.");
        lx_check_error(m_hDC == 0, "Device context should be released before the object is deleted.");
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

        // Create the rendering context
        createGlContext();
    }

    void CanvasGL::impDestroy()
    {
        destroyGlContext();
    }

    bool CanvasGL::impRedraw()
    {
        // The default behavior of the imp/signal is to call the imp method, then the signals.
        // However, in this case the imp methods wants to swap the buffer *after* the slots
        // have been called.  
        // 
        slotRedraw();

        ::SwapBuffers(::GetDC(m_hWnd));

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
    
        2) Initialize GLEW.  GLEW is being used as the interface to OpenGL to avoid having
           to manually pull in all the extensions.  No need to unnecessarily duplicate 
           boilerplate code.

        3) Create the OpenGL 3.2 context using the pixel format found in (1) and the methods
           made available via GLEW in (2).
     */
    void 
    CanvasGL::createGlContext()
    {
        lx_check_error(m_hDC == 0, "Context is non-zero.  Already initialized?");
        lx_check_error(m_hRC == 0, "Context is non-zero.  Already initialized?");

        m_hDC = GetDC(m_hWnd);
        m_hRC = 0;

        PIXELFORMATDESCRIPTOR pfd;
        ::memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

        pfd.dwFlags |= PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.iLayerType = PFD_MAIN_PLANE;
        pfd.cColorBits = 32;


        GLuint uPixelFormat = ::ChoosePixelFormat(m_hDC, &pfd);
        if (!uPixelFormat)
            lx_error("Cannot find suitable pixel format");

        if (::SetPixelFormat(m_hDC, uPixelFormat, &pfd) == FALSE)
            lx_error("Cannot set pixel format");

        // Create a temporary context so that GLEW will initialize properly
        //
        HGLRC tempRC = wglCreateContext(m_hDC);

        if (!tempRC)
            lx_error("Cannot create OpenGL context");

        if (!wglMakeCurrent(m_hDC, tempRC))
            lx_error("Cannot set rendering context");

        // Initialize GLEW (pulls in all the OpenGL extension functions)
        //
        glewInit();

        // Create the OpenGL 3.2 context using wglCreateContextAttribsARB.
        //
        int attribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 2,
            WGL_CONTEXT_FLAGS_ARB, 
            WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            WGL_CONTEXT_PROFILE_MASK_ARB, 
            WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
            0
        };
    
        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;
        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");

        if (wglCreateContextAttribsARB)
        {
            // Create the new context and delete the previous created temporary context
            m_hRC = wglCreateContextAttribsARB(m_hDC,0, attribs);
            wglMakeCurrent(0,0);
            wglDeleteContext(tempRC);		   
            wglMakeCurrent(m_hDC, m_hRC);
        }
        else
        {
            lx_warn("Using a pre-OpenGL 3.2 context");
            m_hRC = tempRC;
        }
    
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
                lx_error("Release Of DC And RC Failed.");

            if (!wglDeleteContext(m_hRC))
                lx_error("Release Rendering Context Failed.");

            m_hRC = 0;
        }

        if (m_hDC) 
        {
            if (!ReleaseDC(m_hWnd, m_hDC))
                lx_error("Cannot Release Window Context");
            m_hDC = 0;
        }
    }

    //--------------------------------------------------------------------------//

    void 
    CanvasHost::create (CanvasBase* pWin, const char* id, bool bVisible)
    {
        m_windows.insert(std::make_pair(id, pWin));
        if (bVisible)
            pWin->show();
    }

    bool 
    CanvasHost::processEvents()
    {
        bool bQuit = false;

        MSG uMsg;
        while (::PeekMessage(&uMsg, 0, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&uMsg);
            ::DispatchMessage(&uMsg);

            if (uMsg.message == WM_QUIT)
                bQuit = true;
        }

        return bQuit;
    }

    void 
    CanvasHost::destroyWindows()
    {
        for (WindowMap::iterator it = m_windows.begin(); it != m_windows.end(); ++it)
            it->second->destroy();
    }

    void                
    CanvasHost::shutdown (void)
    {
        destroyWindows();
        ::PostQuitMessage(0);
    }

}}}

