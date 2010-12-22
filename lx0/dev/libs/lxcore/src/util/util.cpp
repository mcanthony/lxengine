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

#include <iostream>
#include <string>
#include <sstream>
#include <ctime>

#include <lx0/core.hpp>
#include <lx0/util.hpp>

using namespace lx0::core;

namespace lx0 { namespace util {


    /*!
        \todo The current implementation actually returns true _if the file can
        be opened_.  This is actually a subset of the files that exist.  This
        needs be clarified via a renaming of this method.
     */
    bool
    lx_file_exists (std::string filename)
    {
        std::string s;
        FILE* fp;
        fopen_s(&fp, filename.c_str(), "r");

        if (fp == nullptr)
            return false;
        else 
        {
            fclose(fp);
            return true;
        }
    }

    /*!
        Inefficient, but simple and convenient.
     */
    std::string 
    lx_file_to_string (std::string filename)
    {
        std::string s;
        FILE* fp;
        fopen_s(&fp, filename.c_str(), "r");

        lx_check_error(fp != nullptr, "lx_file_to_string: file '%s' not found!", filename.c_str());

        char szString[4096];
        while (fgets(szString, 4096, fp) != NULL) {
            s += std::string(szString);
        }
        fclose(fp);

        return s;
    }

    lx0::core::lxvar 
    lx_file_to_json (const char* pszFilename)
    {
        std::string s = lx_file_to_string(pszFilename);

        const char* p = s.c_str();
        return lxvar::parse(p);
    }

    std::string         
    lx_itoa (size_t i)
    {
        std::ostringstream stream;
        stream << i;
        return stream.str();
    }

    std::string
    lx_ctime (void)
    {
        time_t rawtime;
        time(&rawtime);
        std::string s( ctime(&rawtime) );

        // Trim trailing whitespace
        size_t whitespaceCount = 0;
        for (auto it = s.rbegin(); it != s.rend(); it++)
        {
            if (isspace(*it))
                whitespaceCount++;
            else
                break;
        }
        s.resize( s.size() - whitespaceCount );

        return s;
    }

}}