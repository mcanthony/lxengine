//===========================================================================//
/*
                                   LxEngine

    LICENSE

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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

// Standard headers
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

// Lx0 headers
#include <lx0/lxengine.hpp>

using namespace lx0;

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

static void runImageTest (lxvar& results, std::string execName, std::string groupName, std::string testname, std::string filename)
{
    std::string imageName = boost::str( boost::format("unittest_results\\%1%\\%2%.png") % groupName % testname );
    std::string diffName = boost::str( boost::format("unittest_results\\%1%\\%2%.diff.png") % groupName % testname );
    std::string baseline =  boost::str( boost::format("media2\\unittest_baselines\\%1%\\%2%.png") % groupName % testname);

    std::string cmd = boost::str( boost::format(
        "%3% --file=\"%1%\" --width=256 --height=256 --output=%2% 2>stderr.txt 1>stdout.txt")
        % filename
        % imageName
        % execName
        );

    std::cout << "\n";
    std::cout << "=============================================================================\n";
    std::cout << cmd << "\n";
    std::cout << "=============================================================================\n";

    system(cmd.c_str());
    
    
    std::string compareCmd = (boost::format("compare \"%1%\" \"%2%\" -compose Src \"%3%") % imageName % baseline % diffName).str();
    system(compareCmd.c_str());

    std::vector<std::string> stdoutLines;
    std::vector<std::string> stderrLines;
    boost::split(stdoutLines, lx0::string_from_file("stdout.txt"), boost::is_any_of("\n"));
    boost::split(stderrLines, lx0::string_from_file("stderr.txt"), boost::is_any_of("\n"));

    lxvar test;
    test["name"]     = testname;
    test["image"]    = imageName;
    test["baseline"] = baseline;
    test["diff"]     = diffName;
    test["stdout"]   = stdoutLines;
    test["stderr"]   = stderrLines;
    results.push(test);
}

static void runTest (lxvar& results, std::string testname, std::string filename)
{
    runImageTest(results, "Release\\sm_raytracer.exe", "raytracer", testname, filename);
    runImageTest(results, "Release\\sm_rasterizer.exe", "rasterizer", testname, filename);
}

static void runTestSet()
{
    system("mkdir unittest_results");
    system("mkdir unittest_results\\raytracer");
    system("mkdir unittest_results\\rasterizer");

    lxvar results;
    results["date"] = lx0::lx_ctime();

    std::vector<std::string> files;
    lx0::find_files_in_directory(files, "media2/appdata/sm_raytracer", "xml");

    for (auto it = files.begin(); it != files.end(); ++it)
    {
        boost::filesystem::path path(*it);
        std::string filename = path.string();
        std::string testName = path.filename().string().substr(0, path.filename().string().length() - path.extension().string().length());

        runTest(results["tests"], testName, filename);
    }

    std::ofstream file;
    file.open("results.json");
    file << lx0::format_json(results) << std::endl;
    file.close();

    system("copy ..\\dev\\tools\\view_results.html .");
    system("view_results.html");
}

int 
main (int argc, char** argv)
{
    int exitCode = -1;
    try
    {
        EnginePtr spEngine = Engine::acquire();
        {
            auto spDocument = spEngine->loadDocument("media2/appdata/sb_unittest_app/unittests.xml");
            runTestSet();
        }
        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        throw lx_fatal_exception("Fatal: unhandled exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
