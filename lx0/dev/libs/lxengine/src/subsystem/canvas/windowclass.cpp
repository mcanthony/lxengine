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

#include <windows.h>
#include <windowsx.h>
#include "windowclass.hpp"

using namespace lx0::subsystem::canvas_ns::detail;

Win32WindowClass::Win32WindowClass (const char* name, WNDPROC wndProc)
    : m_refCount (0)
    , m_name     (name)
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

