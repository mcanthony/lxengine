//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

    Copyright (c) 2010 athile@athile.net (http://www.athile.net)

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

#include "lxcanvas/lxcanvas.hpp"
#include "lx0/core.hpp"

#include <windows.h>
#include <windowsx.h>

using namespace lx0::core;

namespace lx0 { namespace canvas { namespace platform {

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

    Win32WindowClass Win32WindowBase::s_windowClass((WNDPROC)Win32WindowBase::windowProc);

    Win32WindowBase::Win32WindowBase()
        : m_hWnd (NULL)
    {
        s_windowClass.registerClass();
    }

    Win32WindowBase::~Win32WindowBase()
    {
        lx_check_error(m_hWnd == NULL, "Call destroy() on the window before deleting it! "
            "This allows for proper clean-up of other objects which may depend on the window.");
    }

    void
    Win32WindowBase::show (void) 
    { 
        ::ShowWindow((HWND)m_hWnd, SW_SHOWDEFAULT);  
        ::UpdateWindow((HWND)m_hWnd); 
    }

    void
    Win32WindowBase::invalidate (void)
    {
        // Notify to Windows that the full window must be redrawn.  
        // This signals a WM_PAINT message.
        ::InvalidateRect((HWND)m_hWnd, NULL, FALSE);
    }

    void        
    Win32WindowBase::destroy (void) 
    { 
        ::DestroyWindow((HWND)m_hWnd);
        m_hWnd = 0; 

        s_windowClass.unregisterClass();
    }

    void 
    Win32WindowBase::overrideParentWndProc (void* wndProc)
    {
        ::SetWindowLongPtr((HWND)m_hWnd, GWLP_WNDPROC, (LONG)(LONG_PTR)wndProc);
    }


    int CALLBACK 
    Win32WindowBase::windowProc (void* hWnd_, unsigned int uMsg_, unsigned int* wParam_, long* lParam_)
    {
        HWND hWnd = (HWND)hWnd_;
        UINT uMsg = (UINT)uMsg_;
        WPARAM wParam = (WPARAM)wParam_;
        LPARAM lParam = (LPARAM)lParam_;

        // The "this" pointer has been stored in the GWL_USERDATA slot of the window.  See WM_CREATE.
        Win32WindowBase* pWin = reinterpret_cast<Win32WindowBase*>((LONG_PTR)GetWindowLongPtr(hWnd, GWL_USERDATA));

        switch (uMsg)
        {
            case WM_CREATE:
                {
                    LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
                    Win32WindowBase* pWin = reinterpret_cast<Win32WindowBase*>(pCreateStruct->lpCreateParams);
                    pWin->m_hWnd = (void*)hWnd;
                    SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG)(LONG_PTR)pWin);

                    pWin->impCreate();
                    pWin->slotCreate();               
            
                    return 0;   
                }

            case WM_KEYDOWN:
                {
                    ///@todo Add the ctrl-shift extended key state
                    unsigned int keyCode = (UINT)(wParam);

                    if (pWin->impKeyDown(keyCode))
                        pWin->slotKeyDown(keyCode);
                }
                break;
        
            case WM_KEYUP:
                {
                    ///@todo Add the ctrl-shift extended key state
                    unsigned int keyCode = (UINT)(wParam);

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

            case WM_LBUTTONDOWN:    
                {   
                    const int clientX = GET_X_LPARAM(lParam);
                    const int clientY = GET_Y_LPARAM(lParam);
                
                    pWin->m_lButton.bDown = true;  
                    pWin->m_lButton.startX = clientX;
                    pWin->m_lButton.startY = clientY;
                } 
                return 0;

            case WM_LBUTTONUP:      
                {   
                    const int clientX = GET_X_LPARAM(lParam);
                    const int clientY = GET_Y_LPARAM(lParam);

                    pWin->m_lButton.bDown = false;
                } 
                return 0;

            case WM_MOUSEMOVE:
                {
                    int clientX = GET_X_LPARAM(lParam); 
                    int clientY = GET_Y_LPARAM(lParam); 

                    pWin->m_mouse.previousX = pWin->m_mouse.x;
                    pWin->m_mouse.previousY = pWin->m_mouse.y;
                    pWin->m_mouse.x = clientX;
                    pWin->m_mouse.y = clientY;

                    if (pWin->m_lButton.bDown)
                    {
                        if (pWin->impLMouseDrag(pWin->m_mouse, pWin->m_lButton, KeyModifiers()))
                            pWin->slotLMouseDrag(pWin->m_mouse, pWin->m_lButton, KeyModifiers());
                    }
                }
                break;

            case WM_MOUSEWHEEL:
                {
                    const int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
                    if (pWin->impMouseWheel(pWin->m_mouse, wheelDelta))
                        pWin->slotMouseWheel(pWin->m_mouse, wheelDelta);
                
                }
                return 0;

            default:
                return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
        }

        return 0;
    }


}}}

