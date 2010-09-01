#include <iostream>
#include <cstdio> 
#include <cstdarg>

#include <lx0/core.hpp>

namespace lx0 { namespace core {

    slot<void (const char*)> slotFatal;
    slot<void (const char*)> slotError;
    slot<void (const char*)> slotWarn;
    slot<void (const char*)> slotLog;
    slot<void (const char*)> slotAssert;
    slot<void (const char*)> slotDebug;

    void 
    log (const char* format, ...)
    {
        char buffer[512] = "";

        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

        slotLog(buffer);
    }

}}
