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
        
        uuid1 = "{2150E333-8FDC-42A3-9474-1A3956D46DE8}"
        uuid2 = str(uuid.uuid4()).upper()
        
        project_section += """Project("%(uuid1)s") = "%(name)s", "%(name)s", "{%(uuid2)s}"\n"""  % { "name" : folder, "uuid1" : uuid1, "uuid2" : uuid2 }
        project_section += "EndProject\n"

        for project in projects:
            retext = project + ".vcxproj\", \"{([^}]+)}\""
            regex = re.compile(retext);
            res = regex.search(text)
            
            puuid = res.group(1)
        
            global_section += "\t\t{%(puuid)s} = {%(fuuid)s}\n" % { "puuid" : puuid, "fuuid" : uuid2 }
        
    global_section = "\tGlobalSection(NestedProjects) = preSolution\n" + global_section + "\tEndGlobalSection\n";
      
    
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
    
write_sln("lx0.sln", [
    "1. Libs", [
        "lxcore",
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
