
#pragma once

#include <lx0/slot.hpp>

namespace lx0 { namespace core {

    void assert (bool condition);
    void assert (bool condition, const char* format, ...);

    void fatal  (const char* format, ...);
    void error  (const char* format, ...);
    void warn   (const char* format, ...);
    void log    (const char* format, ...);
    void debug  (const char* format, ...);
    
    extern slot<void (const char*)> slotFatal;
    extern slot<void (const char*)> slotError;
    extern slot<void (const char*)> slotWarn;
    extern slot<void (const char*)> slotLog;
    extern slot<void (const char*)> slotAssert;
    extern slot<void (const char*)> slotDebug;
    
}}
