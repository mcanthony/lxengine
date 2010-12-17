"""
LxEngine CMake project_group() workaround script

DESCRIPTION

This script is a hard-coded script that injects solution folders into
the lx0.sln solution file, thus organizing the individual projects 
into folders.

This can be *easily modified to work with other projects* by simply
changing the parameters in the call to write_sln().


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
"""



import re
import uuid

def write_sln(solution_file, folders):
    
    text = ""
    solution = open(solution_file)
    for line in solution:
        text += line

    global_section = ""
    project_section = ""    
    for i in range(len(folders)/2):
        folder = folders[i*2];
        projects = folders[i*2 + 1]
        
        print "Processing %(folder)s..." % { "folder" : folder }
        
        # The first GUID is a 'magic number' that lets Visual Studio know this
        # is a "Nested Solution Folder".  The second is a unique identifier for
        # the particular folder, so generate one randomly.
        #
        uuid1 = "{2150E333-8FDC-42A3-9474-1A3956D46DE8}"
        uuid2 = str(uuid.uuid4()).upper()
        
        project_section += """Project("%(uuid1)s") = "%(name)s", "%(name)s", "{%(uuid2)s}"\n"""  % { "name" : folder, "uuid1" : uuid1, "uuid2" : uuid2 }
        project_section += "EndProject\n"

        for project in projects:
            retext = project + ".vcxproj\", \"{([^}]+)}\""
            regex = re.compile(retext);
            res = regex.search(text)
            
            puuid = res.group(1)
        
            # The tab characters here matter!  Visual Studio will fail to parse this correctly if
            # spaces are used instead of tabs.
            #
            global_section += "\t\t{%(puuid)s} = {%(fuuid)s}\n" % { "puuid" : puuid, "fuuid" : uuid2 }
        
    global_section = "\tGlobalSection(NestedProjects) = preSolution\n" + global_section + "\tEndGlobalSection\n";
      
    
    #
    # Insert the generated sections to the appropriate locations
    # and write the file back out to disk
    #
    text = ""
    solution = open(solution_file)
    for line in solution:
        if line == "Global\n":
            text += project_section
        if line == "EndGlobal\n":
            text += global_section
        text += line
    
    solution = open(solution_file, "w")
    solution.write(text)
    
    
#
# Invoke the function to move the projects into folders:
# this is hard-coded to the LxEngine project structure, but
# obviously can be changed easily to match a different project.
# (It's be great to have CMake generate this data structure 
# automatically; ).
#
write_sln("lx0.sln", [
    "1. Libs", [
        "lxcore",
        "lxengine",
        "lxcanvas"
    ],
    "2. Samples", [
        "sm_lx_cube_rain",
        "sm_lx_cube_asteriods",
        "sm_lxcanvas",
        "sm_ogre_minimal",
        "sm_v8_basic",
        "cpp_smartptr"
    ],
    "3. Sandbox", [
        "blendload",
        "sb_fixedpoint",
        "elm_function",
        "elm_reference",
        "sandbox_shadergraph",
    ],
    "4. Benchmarks", [
        "bm_lxvar",
    ],
    "5. Unit Tests", [
        "ut_jsonparser",
        "ut_lx_vector",
    ],
    "6. CMake", [
        "ALL_BUILD",
        "ZERO_CHECK",
        "INSTALL"
    ],
]
)
