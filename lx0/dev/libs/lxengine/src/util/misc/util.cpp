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

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <lx0/lxengine.hpp>

using namespace lx0::core;

namespace lx0 { namespace util { namespace misc {


    /*!
        \todo The current implementation actually returns true _if the file can
        be opened_.  This is actually a subset of the files that exist.  This
        needs be clarified via a renaming of this method.
     */
    bool
    file_exists (std::string filename)
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

    void 
    find_files_in_directory (std::vector<std::string>& files, const char* directory, const char* extension)
    {
        using namespace boost::filesystem;

        std::string ext = boost::str( boost::format(".%1%") % extension );

        for (directory_iterator dit(directory); dit != directory_iterator(); ++dit)
        {
            if (is_regular_file(dit->status()))
            {
                path filepath = dit->path();
                if (boost::ends_with(filepath.filename(), ext))
                {
                    files.push_back( filepath.normalize().string() );
                }
            }
        }
    }

    void 
    for_files_in_directory  (const char* path, const char* extension, std::function<void (std::string)> callback)
    {
        std::vector<std::string> files;
        find_files_in_directory(files, path, extension);
        for (auto it = files.begin(); it != files.end(); ++it)
        {
            callback( *it );
        }
    }

    /*!
        Inefficient, but simple and convenient.
     */
    std::string 
    string_from_file (std::string filename)
    {
        std::string s;
        FILE* fp;
        fopen_s(&fp, filename.c_str(), "r");

        lx_check_error(fp != nullptr, "string_from_file: file '%s' not found!", filename.c_str());

        char szString[4096];
        while (fgets(szString, 4096, fp) != NULL) {
            s += std::string(szString);
        }
        fclose(fp);

        return s;
    }

    lx0::lxvar 
    lxvar_from_file (std::string filename)
    {
        std::string s = string_from_file(filename);

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

}}}
