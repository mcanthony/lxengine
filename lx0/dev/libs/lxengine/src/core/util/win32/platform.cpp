//===========================================================================//
/*
                                   LxEngine

    LICENSE

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

#include <lx0/util/misc/util.hpp>

#include <windows.h>

namespace lx0 { namespace util {

    unsigned int        
    lx_milliseconds (void)
    {
        return ::timeGetTime();
    }

    void
    lx_message_box (std::string caption, std::string message)
    {
        HWND hWnd = ::GetForegroundWindow();
        ::MessageBoxA(hWnd, message.c_str(), caption.c_str(), MB_OK);
    }

    void
    lx_break_if_debugging (void)
    {
#ifndef NDEBUG
        if (::IsDebuggerPresent()) 
        {
            __debugbreak();
        }
#endif
    }

    bool
    lx_file_is_open (std::string filename)
    {
        OFSTRUCT fileStruct;
        HFILE hFile = ::OpenFile(filename.c_str(), &fileStruct, OF_SHARE_EXCLUSIVE);
        if (hFile != HFILE_ERROR)
        {
            ::CloseHandle((HANDLE)hFile);
            return false;
        }
        else
            return true;
    }

    void
    _lx_reposition_console()
    {
        // Windows specific hack 
        //
        // Get the console window in a good default position for debugging
        ::MoveWindow(::GetConsoleWindow(), 960, 0, 880, 1024, TRUE);

	    CONSOLE_FONT_INFOEX lpConsoleCurrentFontEx;
	    lpConsoleCurrentFontEx.cbSize = sizeof(CONSOLE_FONT_INFOEX);
	    lpConsoleCurrentFontEx.dwFontSize.X = 7;
	    lpConsoleCurrentFontEx.dwFontSize.Y = 12;
	    lpConsoleCurrentFontEx.FontWeight = 400;
	    lpConsoleCurrentFontEx.nFont = 1;
	    lpConsoleCurrentFontEx.FontFamily = FF_DONTCARE;
	    lstrcpyW(lpConsoleCurrentFontEx.FaceName, L"Ariel");
	    ::SetCurrentConsoleFontEx ( ::GetStdHandle(STD_OUTPUT_HANDLE), false, &lpConsoleCurrentFontEx );
        //
        // End Windows specific hack
    }

}}