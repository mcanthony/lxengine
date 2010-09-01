
#pragma once

namespace lx0 { namespace core {

    void assert (bool condition);
    void assert (bool condition, const char* format, ...);

    void fatal  (const char* format, ...);
    void error  (const char* format, ...);
    void warn   (const char* format, ...);
    void log    (const char* format, ...);

    void debug  (const char* format, ...);
}}
