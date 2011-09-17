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

#include <lx0/lxengine.hpp>
#include <lx0/subsystem/canvas.hpp>

#include <windows.h>
#include <windowsx.h>

#include "windowclass.hpp"

using namespace lx0::core;

//
// Good or Evil?
//
// Makes the coding in this file sane and keeps the window-ishs out of the header.
//
#define m_hWnd reinterpret_cast<HWND&>(mOpaqueHwnd)
#define m_hDC  reinterpret_cast<HDC&>(m_opaque_hDC)
#define m_hRC  reinterpret_cast<HGLRC&>(m_opaque_hRC)

namespace lx0 { namespace subsystem { namespace canvas_ns { namespace detail {

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
            case VK_SPACE:  return KC_SPACE;
            case VK_SHIFT:  return KC_SHIFT;
            case VK_LSHIFT: return KC_LSHIFT;
            case VK_RSHIFT: return KC_RSHIFT;
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

                lxState.modifiers.alt = (winState[VK_MENU] & 0x80) ? 1 : 0;
                lxState.modifiers.lalt = (winState[VK_LMENU] & 0x80) ? 1 : 0;
                lxState.modifiers.ralt = (winState[VK_RMENU] & 0x80) ? 1 : 0;
                lxState.modifiers.shift = (winState[VK_SHIFT] & 0x80) ? 1 : 0;
                lxState.modifiers.lshift = (winState[VK_LSHIFT] & 0x80) ? 1 : 0;
                lxState.modifiers.rshift = (winState[VK_RSHIFT] & 0x80) ? 1 : 0;
                lxState.modifiers.ctrl = (winState[VK_CONTROL] & 0x80) ? 1 : 0;
                lxState.modifiers.lctrl = (winState[VK_LCONTROL] & 0x80) ? 1 : 0;
                lxState.modifiers.rctrl = (winState[VK_RCONTROL] & 0x80) ? 1 : 0;

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
                throw lx_error_exception("Call to Win32 GetKeyboardState() failed!");
                
                for (int i = 0; i < KC_COUNT; ++i)
                    lxState.bDown[i] = false;
            }
        }
    }





    //===========================================================================//

    Win32WindowClass CanvasBase::s_windowClass("LxCanvas", (WNDPROC)CanvasBase::windowProc);

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
                        pWin->slotKeyDown(s_winToLxKey(keyCode));
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

}}}}

