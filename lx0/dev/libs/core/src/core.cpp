#include <iostream>
#include <cstdio> 
#include <cstdarg>

#include <lx0/core.hpp>

namespace lx0 { namespace core {

    void 
    log (const char* format, ...)
    {
        char buffer[512] = "";

        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

        std::cout << buffer << std::endl;
    }

}}
