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
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/uniform_on_sphere.hpp>
#include <boost/random/variate_generator.hpp>

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

        //
        // Silently ignore non-existent directories
        //
        if (exists(directory))
        {
            std::string ext = boost::str( boost::format(".%1%") % extension );

            for (directory_iterator dit(directory); dit != directory_iterator(); ++dit)
            {
                if (is_regular_file(dit->status()))
                {
                    path filepath = dit->path();
                    if (boost::ends_with(filepath.filename().string(), ext))
                    {
                        files.push_back( filepath.normalize().string() );
                    }
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
        Simple and convenient.
     */
    std::string 
    string_from_file (std::string filename)
    {
        FILE* fp;
        fopen_s(&fp, filename.c_str(), "r");
        lx_check_error(fp != nullptr, "string_from_file: file '%s' not found!", filename.c_str());

        fseek(fp, 0L, SEEK_END);        
        unsigned int fileSize = ftell(fp);
        
        std::string s(fileSize, 0);        
        rewind(fp);      

        // File size does NOT necessarily equal readCount since we open in text
        // rather than binary mode.
        unsigned int readCount = fread(&s[0], 1, fileSize, fp);
        s.resize(readCount);

        fclose(fp);
        return s;
    }

    lx0::lxvar 
    lxvar_from_file (std::string filename)
    {
        std::string s = string_from_file(filename);

        const char* p = s.c_str();
        return lxvar::parse(filename, 0, p);
    }

    std::string         
    lx_itoa (size_t i)
    {
        std::ostringstream stream;
        stream << i;
        return stream.str();
    }

    /*!
        Returns the current time as in the form:

        YYYYMMDD_HHMMSS

        e.g. 20110903_154632
     */
    std::string
    lx_timestring (void)
    {
        time_t rawtime;
        time(&rawtime);
        struct tm* timeinfo = localtime(&rawtime);
            
        return boost::str( boost::format("%04d%02d%02d_%02d%02d%02d") 
            % (timeinfo->tm_year + 1900) 
            % (timeinfo->tm_mon + 1)
            % (timeinfo->tm_mday)
            % (timeinfo->tm_hour)
            % (timeinfo->tm_min)
            % (timeinfo->tm_sec)
            );
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

    bool 
    lx_little_endian (void)
    {
        // Credit to the Quake 2 source code (Swap_Init) for this test
        lx0::uint8 bytes[2] = { 1, 0 };
        return (*(short*)bytes == 1) ? true  : false; 
    }

    namespace 
    {
        class RandomUnit
        {
        public:
            RandomUnit (int seed)
                : mDie   (mGenerator, mRange)
            {
                mGenerator.seed(seed);
            }

            float operator() () 
            {
                return mDie();
            }

        private:
            boost::mt19937 mGenerator;
            boost::uniform_01<float> mRange;
            boost::variate_generator<boost::mt19937, boost::uniform_01<float> > mDie;
        };

        class RandomFloat
        {
        public:
            RandomFloat (int seed, float min, float max)
                : mRange (min, max)
                , mDie   (mGenerator, mRange)
            {
                mGenerator.seed(seed);
            }

            float operator() () const
            {
                return mDie();
            }

        private:
            boost::mt19937 mGenerator;
            boost::uniform_real<float> mRange;
            mutable boost::variate_generator<boost::mt19937, boost::uniform_real<float> > mDie;
        };

        class RandomInt
        {
        public:
            RandomInt (int seed, int min, int max)
                : mRange (min, max)
                , mDie   (mGenerator, mRange)
            {
                mGenerator.seed(seed);
            }

            int operator() () const
            {
                return mDie();
            }

        private:
            boost::mt19937 mGenerator;
            boost::uniform_int<> mRange;
            mutable boost::variate_generator<boost::mt19937, boost::uniform_int<> > mDie;
        };
    }


    float                   
    random_unit (void)
    {
        static RandomUnit die(0);
        return die();
    }

    glgeom::vector3f        
    random_vector3f (void)
    {
        static auto die = random_die_f(-1, 1, 0);
        return glgeom::normalize( glgeom::vector3f( die(), die(), die() ) );
    }

    std::function<float()>  
    random_die_f (float min, float max, int seed)
    {
        RandomFloat die(seed, min, max);
        return [die]() { return die(); };
    }

    std::function<int()>    
    random_die_i (int min, int max, int seed)
    {
        RandomInt die(seed, min, max);
        return [die]() { return die(); };
    }

    /*
        A hack for convenience: Visual Studio wants to run projects from the 
        directory cmake builds them to - but LxEngine is setup to use 
        a common run location so shared media can be readily accessed.
        
        Reset the current path automatically since this is such a common case.

        The exception (not enforced) is that this would only be called after a 
        lx_in_debugger() check.
     */
    void
    _lx_change_current_path_to_lx_root (void)
    {
        using namespace boost::filesystem;

        // Practically wins out here...check for fixed path names to determine
        // the redirect
        if (!is_directory("common"))
        {
            lx_log("Cannot find 'common' directory.  Attempting to find correct path.");
            lx_log("Current path is: %1%", current_path().string() );

            // Try up to 6 levels of nesting
            std::string prefix = "..";
            for (int i = 0; i < 6; ++i)
            {
                if (!is_directory(prefix + "/common"))
                {
                    prefix += "/..";
                }
                else
                { 
                    current_path(prefix);
                    lx_log( "Current path changed to: %1%", current_path().string() );
                    break;
                }
            }
        }
    }

}}}
