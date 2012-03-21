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

#include <lx0/engine/engine.hpp>
#include <lx0/util/misc/util.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <windows.h>

namespace lx0 { namespace util { namespace misc {

    unsigned int        
    lx_milliseconds (void)
    {
        return ::timeGetTime();
    }

    lx0::int64              
    lx_ticks (void)
    {
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);
        return li.QuadPart;
    }

    lx0::int64              
    lx_ticks_per_second (void)
    {
        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        return li.QuadPart;
    }
    
    void
    lx_message_box (std::string caption, std::string message)
    {
        boost::replace_all(message, "\n", "\n\r");

        HWND hWnd = NULL; //::GetForegroundWindow();
        ::MessageBoxA(hWnd, message.c_str(), caption.c_str(), MB_OK);
    }

    void 
    lx_debugger_message (std::string message)
    {
        ::OutputDebugStringA(message.c_str());
    }


    bool
    lx_in_debugger (void)
    {
        return !!::IsDebuggerPresent();
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
    file_is_open (std::string filename)
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
        lxvar sysinfo = lx0::Engine::acquire()->getSystemInfo()["system"]["display"];

        int winX = 960;

        if (lx_in_debugger() && sysinfo["monitorCount"].as<int>() > 1)
        {
            winX = sysinfo["monitors"][1]["offset"][0];
        }

        // Windows specific hack 
        //
        // Get the console window in a good default position for debugging       
        ::MoveWindow(::GetConsoleWindow(), winX, 0, 880, 1024, TRUE);

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

    lx0::int64  Timer::_ticks()
    {
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);
        return li.QuadPart;
    }

    lx0::int64  Timer::_ticksPerSec()
    {
        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        return li.QuadPart;
    }

    void 
    lx_operating_system_info (lxvar& map)
    {
        // See the OSVERSIONINFOEX MSDN page if you want to make this function more detailed
        OSVERSIONINFOEX	info;
        ZeroMemory(&info, sizeof(OSVERSIONINFOEX));
        info.dwOSVersionInfoSize = sizeof(info);

        ::GetVersionEx((OSVERSIONINFO*)&info);
        map.insert("version", boost::str( boost::format("%u.%u build %u SP %u.%u") 
            % info.dwMajorVersion 
            % info.dwMinorVersion
            % info.dwBuildNumber 
            % info.wServicePackMajor
            % info.wServicePackMinor) );
    }

    lxvar
    _lx_monitor_info ()
    {
        // 
        // Hide the boilerplate of the WIN32 callback in an hidden class
        //
        class _MonitorInfo
        {
        public:
            _MonitorInfo()
            {
                index = 0;
                ::EnumDisplayMonitors(NULL, NULL, enumMonitorsCallback, (LPARAM)this);
            }

            int     index;
            lxvar   data;

        protected:
            static
            BOOL CALLBACK
            enumMonitorsCallback (HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
            {
                _MonitorInfo& monitorInfo = *reinterpret_cast<_MonitorInfo*>(dwData);

                auto& info = monitorInfo.data[monitorInfo.index];
                info["offset"][0] = lprcMonitor->left;
                info["offset"][1] = lprcMonitor->top;
                info["size"][0] = lprcMonitor->right - lprcMonitor->left;
                info["size"][1] = lprcMonitor->bottom - lprcMonitor->top;

                monitorInfo.index++;
                return TRUE;
            }
        };

        _MonitorInfo info;
        return info.data;
    }

    void 
    lx_display_info (lxvar& map)
    {
        map["primaryResolution"][0] = ::GetSystemMetrics(SM_CXSCREEN);
        map["primaryResolution"][1] = ::GetSystemMetrics(SM_CYSCREEN);
        map["virtualResolution"][0] = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
        map["virtualResolution"][1] = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);

        map["monitorCount"] = ::GetSystemMetrics(SM_CMONITORS);
        map["monitors"]     = _lx_monitor_info();
    }

    void
    lx_load_plugin (std::string pluginName)
    {
#ifdef NDEBUG
        std::string filename = std::string() + "plugins/" + pluginName + "/Release/" + pluginName + ".dll"; 
#else 
        std::string filename = std::string() + "plugins/" + pluginName + "/Debug/" + pluginName + ".dll"; 
#endif

        typedef void (*InitializePlugin)(void);

        HMODULE hDLL = ::LoadLibraryA(filename.c_str());
        InitializePlugin pfInitialize = reinterpret_cast<InitializePlugin>( ::GetProcAddress(hDLL, "initializePlugin") );

        if (pfInitialize)
            (*pfInitialize)();
        else 
        {
            bool bExists = file_exists(filename);

            if (!bExists)
                throw lx_error_exception("Could not load plugin '%1%'.  File '%2%' does not exist.", pluginName, filename);
            else
                throw lx_error_exception("Could not load plugin '%1%'", pluginName);
        }
    }  

    _declspec(dllexport) void* _gwpEngine = NULL;

    std::weak_ptr<lx0::Engine>*
    _lx_get_engine_singleton()
    {
        HMODULE hHandle = ::GetModuleHandle(NULL);
        auto pData = (std::weak_ptr<lx0::Engine>**)::GetProcAddress(hHandle, "?_gwpEngine@misc@util@lx0@@3PAXA");
        if (!*pData)
            *pData = new std::weak_ptr<lx0::Engine>;
        return *pData;
    }

    lx0::uint32             
    lx_current_thread_id (void)
    {
        return ::GetCurrentThreadId();
    }

    void                    
    lx_current_thread_priority_below_normal (void)
    {
        HANDLE handle = ::GetCurrentThread();
        ::SetThreadPriority(handle, THREAD_PRIORITY_BELOW_NORMAL);
    }

}}}

