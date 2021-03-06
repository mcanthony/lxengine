@echo off
setlocal

REM ===========================================================================
REM LxEngine Windows Dependency Builder
REM ===========================================================================
REM
REM This script automates the following:
REM - Download the correct version of dependency source code
REM - Minimal CMake-like compiler & tool detection [1]
REM - Build the dependencies 
REM - Copy the built files to a standard layout
REM - Provide a CMake include file to use the built files in LxEngine projects
REM
REM Currently only Visual Studio 2010 is supported.  This is due to development 
REM time constraints, not inherent technical limitations.
REM
REM Notes:
REM [1] An attempt was made to implement this script directly in the CMake
REM     language, but the result was too cumbersome and largely unnecessary
REM     since this script is intentionally tailored to support only Visual
REM     Studio compilers.
REM 
REM TODO
REM - Copy resulting binaries to uniform sdk layout
REM - Move source into a root "packages" directory
REM - Test Release build support
REM - Test x64 support
REM
REM LICENSE 
REM (http://www.opensource.org/licenses/mit-license.php)
REM
REM Copyright (c) 2010-2011 athile@athile.net (http://www.athile.net)
REM
REM Permission is hereby granted, free of charge, to any person obtaining a 
REM copy of this software and associated documentation files (the "Software"), 
REM to deal in the Software without restriction, including without limitation 
REM the rights to use, copy, modify, merge, publish, distribute, sublicense, 
REM and/or sell copies of the Software, and to permit persons to whom the 
REM Software is furnished to do so, subject to the following conditions:
REM
REM The above copyright notice and this permission notice shall be included in
REM all copies or substantial portions of the Software.
REM
REM THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
REM IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
REM FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
REM AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
REM LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
REM FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
REM IN THE SOFTWARE.
REM
REM ===========================================================================


REM ===========================================================================
REM Global settings
REM ===========================================================================

set DOWNLOAD_SITE=http://www.athile.net/files
set DEPENDENCY_FILE=dependencies.7z


REM ===========================================================================
REM Beginning of script
REM ===========================================================================

title LxEngine Automated Dependency Builder
echo.
echo.
echo --------------------------------------------------------------------------
echo LxEngine Automated Windows Dependency Builder
echo (alpha)
echo.
echo As this dependency builder is in active development, feedback is 
echo appreciated on what did or did not work for you.
echo --------------------------------------------------------------------------
echo.
echo.

set FAILURE=0

REM ===========================================================================
REM Ensure Visual Studio command-line is available
REM ===========================================================================

echo.
echo Checking for Visual Studio installation...
echo.

REM TODO: It would be nice to invoke cmake here to get this information for us.
REM 

IF NOT "%VS100COMNTOOLS%"=="" (
    echo Found Visual Studio 2010
    set VCVER=10
    set VSVARS_BAT="%VS100COMNTOOLS%\vsvars32.bat"
    set VCPROJEXT=vcxproj
    set PSDK=sdk\msvc10
) ELSE IF NOT "%VS90COMNTOOLS%"=="" (
    echo Found Visual Studio 2009
    set VCVER=9
    set VSVARS_BAT="%VS90COMNTOOLS%\varvars32.bat"
    set VCPROJEXT=vcproj
    set PSDK=sdk\msvc9
) ELSE (
    echo.
    echo ERROR: Could not find installation of Visual Studio 2010 or 2009!
    echo.
    goto:EOF
)

REM Check if the Visual Studio command-line tools are there.
REM Don't always run it or else it will duplicately add paths to PATH
REM
IF "%VSINSTALLDIR%"=="" (
    echo Setting up Visual Studio command line tools
    call %VSVARS_BAT%
) ELSE (
    echo Visual Studio command line tools appear to be setup already.
)
IF "%VSINSTALLDIR%"=="" (
    echo ERROR: Failed to set up Visual Studio command-line tools!
    goto:EOF
)

REM ===========================================================================
REM Grab the dependencies
REM
REM - Download 7zip
REM - Download the dependencies zip
REM - Extract the zip
REM ===========================================================================

title Downloading and extracting dependency sources
echo.
echo Ensuring dependency source is available...
echo.

IF NOT EXIST 7za.exe (
    echo.
    echo ======================= IMPORTANT NOTE ===============================
    echo.
    echo Detected that the dependencies are not yet installed or build.  The 
    echo batch file will now attempt to download and extract them.  
    echo.
    echo.
    echo    !!!  THIS MAY TAKE AN EXTREMELY LONG TIME TO FINISH  !!!
    echo.
    echo.
    echo The build process involves downloading over 100 MBs of source code and
    echo building multiple configurations, including the optimized versions.
    echo This process should only have to be run once, but it may take quite
    echo some time to finish.
    echo.
    echo ======================================================================
    echo.
    echo.

    call:auto_download 7za.exe
    IF %FAILURE%==1 ( goto:EOF )
    
    call:auto_download %DEPENDENCY_FILE%
    IF %FAILURE%==1 ( goto:EOF )
      
    7za x dependencies.7z   
) ELSE (
    echo Found 7za.exe.
)

call:auto_extract_source boost_1_46_1 boost_1_46_1.7z
call:auto_extract_source glm-0.9.2.3  glm-0.9.2.3.zip
call:auto_extract_source bullet-2.78 bullet-2.78.zip
call:auto_extract_source niflib niflib-6cce687f3aebfdc292e812c7dbbfa98d4bffb25b.7z
call:auto_extract_source zlib-1.2.5 zlib-1.2.5.7z
call:auto_extract_source lpng154 lpng154.7z
call:auto_extract_source GL3 GL3-20120406.7z

if %FAILURE%==1 ( 
    echo.
    echo ERROR: It appears at least one dependency did not download and extract correctly.
    echo You may wish to extract them manually and rerun the script.
    echo.
    goto:EOF 
)


REM ===========================================================================
REM Check the dependency source is installed properly
REM ===========================================================================

call:ensure_directory audiere_1_9_4
call:ensure_directory freetype_2_4_2
call:ensure_directory libogg-1.2.1
call:ensure_directory libvorbis-1.3.2
call:ensure_directory ogre_1_7_1
call:ensure_directory ois_1_2_0
call:ensure_directory openal_1_1
call:ensure_directory v8
call:ensure_directory source\GL3

if %FAILURE%==1 ( 
    echo.
    echo ERROR: It appears the dependencies did not extract correctly.
    echo You may wish to extract them manually and rerun the script.
    echo.
    goto:EOF 
)

REM ===========================================================================
REM Find Python & Scons for the Google V8 build
REM ===========================================================================

title Checking for required tools...

echo.
echo Searching for required tools...

set PYTHONEXE_PATH=%SystemDrive%\Python27
IF EXIST "%PYTHONEXE_PATH%\python.exe" ( goto PYTHONFOUND )
set PYTHONEXE_PATH=%SystemDrive%\Python26
IF EXIST "%PYTHONEXE_PATH%\python.exe" ( goto PYTHONFOUND )
set PYTHONEXE_PATH=%SystemDrive%\Python25
IF EXIST "%PYTHONEXE_PATH%\python.exe" ( goto PYTHONFOUND )

echo ERROR: Could not locate Python installation!
echo.
goto:EOF

:PYTHONFOUND
IF NOT EXIST "%PYTHONEXE_PATH%\Scripts\scons.bat" (
    echo ERROR: Could not find scons installed within %PYTHONPATH%
    echo.
    goto:EOF
) ELSE (
    echo Found Python at "%PYTHONEXE_PATH%"
)

IF "%DXSDK_DIR%"=="" (
    echo ERROR: Could not find DirectX SDK!
    echo.
    goto END
) ELSE (
    echo Found DirectX at "%DXSDK_DIR%"
)


REM ===========================================================================
REM
REM Build Packages....
REM
REM ===========================================================================

echo.
echo Building packages...
echo.

REM ===========================================================================
REM "Build" GL3 header files (just a simple copy)
REM ===========================================================================

echo * Copying GL3 headers 
call:copy_directory source\gl3 %PSDK%\gl3\include\gl3

REM ===========================================================================
REM Build Boost
REM ===========================================================================

set PROJECT=Boost
set TESTFILE=stage\lib\libboost_iostreams-vc100-mt-gd-1_46_1.lib 
set ROOTDIR=source\boost_1_46_1
echo call bootstrap.bat >_t.bat
echo bjam.exe >>_t.bat

call:build_project %PROJECT% %ROOTDIR% %TESTFILE%
call:test_file_or_fail %ROOTDIR%\boost\any.hpp
IF %FAILURE%==1 (goto:EOF)


set TESTFILE2=%PSDK%\boost\include\boost\any.hpp
IF NOT EXIST %TESTFILE2% (
    call:copy_directory source\boost_1_46_1\boost %PSDK%\boost\include\boost
    call:copy_directory source\boost_1_46_1\stage\lib %PSDK%\boost\lib
)
call:test_file_or_fail %TESTFILE2%
call:test_file_or_fail %PSDK%\boost\lib\libboost_filesystem-vc100-mt-gd.lib
IF %FAILURE%==1 (goto:EOF)

REM ===========================================================================
REM Build OGRE
REM ===========================================================================

REM TODO: This still has a VS2010 dependency

set PROJECT=OGRE
set TESTFILE=sdk\bin\Release\OgreMain.dll
set ROOTDIR=ogre_1_7_1\ogre_build

echo pushd.>_t.bat
echo cd ..\Dependencies\src>>_t.bat
echo title Building OGRE Debug Dependencies>>_t.bat
echo msbuild OgreDependencies.VS2010.sln /p:Configuration=Debug >>_t.bat
echo title Building OGRE Release Dependencies>>_t.bat
echo msbuild OgreDependencies.VS2010.sln /p:Configuration=Release >>_t.bat
echo popd.>>_t.bat
echo title Configuring OGRE>>_t.bat
echo cmake ..\ogre_src -DOGRE_DEPENDENCIES_DIR=..\Dependencies -DBOOST_ROOT=..\..\boost_1_44_0\boost_1_44_0>>_t.bat
echo title Building OGRE Debug>>_t.bat
echo msbuild ALL_BUILD.vcxproj /p:Configuration=Debug >>_t.bat
echo title Building OGRE Release>>_t.bat
echo msbuild ALL_BUILD.vcxproj /p:Configuration=Release >>_t.bat
echo msbuild INSTALL.vcxproj /p:Configuration=Debug >>_t.bat
echo msbuild INSTALL.vcxproj /p:Configuration=Release >>_t.bat
echo copy bin\Debug\*.pdb sdk\bin\Debug >>_t.bat
echo copy bin\Debug\*.cfg sdk\bin\Debug >>_t.bat
echo copy bin\Release\*.pdb sdk\bin\Release >>_t.bat
echo copy bin\Release\*.cfg sdk\bin\Release >>_t.bat
echo title build_vs.bat>>_t.bat

call:build_project %PROJECT% %ROOTDIR% %TESTFILE%
IF %FAILURE%==1 (goto:EOF)

call:copy_directory %ROOTDIR%\sdk %PSDK%\ogre

REM ===========================================================================
REM Build OIS
REM ===========================================================================

set PROJECT=OIS
set ROOTDIR=ois_1_2_0\ois
set TESTFILE=lib\OIS_static.lib

echo cd Win32 >_t.bat
echo msbuild OIS.vcxproj /p:Configuration=Debug >>_t.bat
echo msbuild OIS.vcxproj /p:Configuration=Release >>_t.bat
echo cd .. >>_t.bat

call:build_project %PROJECT% %ROOTDIR% %TESTFILE%
IF %FAILURE%==1 (goto:EOF)

call:copy_files %ROOTDIR%\lib\*.lib %PSDK%\ois\lib
call:copy_directory %ROOTDIR%\includes %PSDK%\ois\include\ois

REM ===========================================================================
REM Build Bullet
REM ===========================================================================

set PROJECT=Bullet
set ROOTDIR=source\bullet-2.78
set TESTFILE=..\bullet_build\lib\Debug\BulletCollision.lib

echo mkdir ..\bullet_build>_t.bat
echo cd ..\bullet_build>>_t.bat
echo cmake ..\bullet-2.78 -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON>>_t.bat
echo msbuild ALL_BUILD.vcxproj /p:Configuration=Debug >>_t.bat
echo msbuild ALL_BUILD.vcxproj /p:Configuration=Release >>_t.bat

call:build_project %PROJECT% %ROOTDIR% %TESTFILE%
IF %FAILURE%==1 (goto:EOF)   

call:copy_files %ROOTDIR%\src\*.h %PSDK%\bullet\include\bullet
call:copy_directory %ROOTDIR%\..\bullet_build\lib %PSDK%\bullet\lib

REM ===========================================================================
REM Build Google V8
REM ===========================================================================
REM
REM Not sure why copying msobj100.dll is necessary, but it seems to be.  Since
REM this is a one time process, go ahead and copy the file without worrying
REM about why...
REM

set PROJECT=V8
set ROOTDIR=v8\v8
set TESTFILE=sdk\lib\v8.lib

echo copy "%VS100COMNTOOLS%..\IDE\msobj100.dll" . > _t.bat
echo call "%PYTHONEXE_PATH%\Scripts\scons.bat" env="PATH:%PATH%,INCLUDE:%INCLUDE%,LIB:%LIB%" mode=debug >>_t.bat
echo call "%PYTHONEXE_PATH%\Scripts\scons.bat" env="PATH:%PATH%,INCLUDE:%INCLUDE%,LIB:%LIB%">>_t.bat
echo mkdir sdk>>_t.bat
echo mkdir sdk\lib>>_t.bat
echo mkdir sdk\include>>_t.bat
echo mkdir sdk\include\v8>>_t.bat
echo call copy v8.lib sdk\lib>>_t.bat
echo call copy v8_g.lib sdk\lib>>_t.bat
echo call copy include\* sdk\include\v8>>_t.bat

call:build_project %PROJECT% %ROOTDIR% %TESTFILE%
IF %FAILURE%==1 (goto:EOF)

call:copy_directory %ROOTDIR%\sdk %PSDK%\v8

REM ===========================================================================
REM Build OpenAL Software Implementation
REM ===========================================================================

set PROJECT=OpenAL
set ROOTDIR=openal_1_1\openal
set TESTFILE=OpenAL-Soft\build\Debug\openal32.lib

echo pushd .>_t.bat
echo cd OpenAL-Soft\build>>_t.bat
echo cmake .. >>_t.bat
echo msbuild OpenAL.sln /p:Configuration=Debug >>_t.bat
echo msbuild OpenAL.sln /p:Configuration=Release >>_t.bat
echo popd >>_t.bat
echo pushd .>>_t.bat
echo REM Hack since alut does not include the AL prefix on the headers>>_t.bat
echo copy include\AL\*.h include >>_t.bat
echo mkdir alut_build_debug >>_t.bat
echo cd alut_build_debug >>_t.bat
echo cmake -DOPENAL_LIB_DIR=..\OpenAL-Soft\build\Debug -DOPENAL_INCLUDE_DIR=..\include ..\alut>>_t.bat
echo msbuild Alut.sln /p:Configuration=Debug >>_t.bat
echo cd ..>>_t.bat
echo mkdir alut_build_release >>_t.bat
echo cd alut_build_release >>_t.bat
echo cmake -DOPENAL_LIB_DIR=..\OpenAL-Soft\build\Release -DOPENAL_INCLUDE_DIR=..\include ..\alut>>_t.bat
echo msbuild Alut.sln /p:Configuration=Release >>_t.bat
echo popd >>_t.bat

call:build_project %PROJECT% %ROOTDIR% %TESTFILE%
IF %FAILURE%==1 (goto:EOF)

call:copy_files %ROOTDIR%\include\AL\*.h %PSDK%\openal\include\AL
call:copy_files %ROOTDIR%\alut\include\AL\*.h %PSDK%\openal\include\AL
call:copy_files %ROOTDIR%\OpenAL-Soft\build\Debug\*.lib %PSDK%\openal\lib\Debug
call:copy_files %ROOTDIR%\OpenAL-Soft\build\Release\*.lib %PSDK%\openal\lib\Release
call:copy_files %ROOTDIR%\alut_build_debug\Debug\*.lib %PSDK%\openal\lib\Debug
call:copy_files %ROOTDIR%\alut_build_release\Release\*.lib %PSDK%\openal\lib\Release
call:copy_files %ROOTDIR%\alut_build_debug\Debug\*.dll %PSDK%\openal\bin\Debug
call:copy_files %ROOTDIR%\alut_build_release\Release\*.dll %PSDK%\openal\bin\Release

call:test_file_or_fail %PSDK%\openal\lib\Debug\OpenAL32.lib
call:test_file_or_fail %PSDK%\openal\lib\Debug\alut.lib
call:test_file_or_fail %PSDK%\openal\lib\Release\OpenAL32.lib
call:test_file_or_fail %PSDK%\openal\lib\Release\alut.lib
IF %FAILURE%==1 (
    erase %ROOTDIR%\%TESTFILE%
    goto:EOF
)

REM ===========================================================================
REM Build Ogg
REM ===========================================================================

set PROJECT=Ogg
set ROOTDIR=libogg-1.2.1
set TESTFILE=win32\VS2010\Win32\Debug\libogg.lib 

echo pushd .>_t.bat
echo msbuild win32\VS2010\libogg_dynamic.sln /p:Configuration=Debug >>_t.bat
echo msbuild win32\VS2010\libogg_dynamic.sln /p:Configuration=Release >>_t.bat
echo popd >>_t.bat

call:build_project %PROJECT% %ROOTDIR% %TESTFILE%
IF %FAILURE%==1 (goto:EOF)

call:copy_files %ROOTDIR%\include\ogg\*.h %PSDK%\libogg\include\ogg
call:copy_files %ROOTDIR%\win32\VS2010\Win32\Debug\*.lib %PSDK%\libogg\lib\Debug
call:copy_files %ROOTDIR%\win32\VS2010\Win32\Debug\*.dll %PSDK%\libogg\bin\Debug
call:copy_files %ROOTDIR%\win32\VS2010\Win32\Release\*.lib %PSDK%\libogg\lib\Release
call:copy_files %ROOTDIR%\win32\VS2010\Win32\Release\*.dll %PSDK%\libogg\bin\Release

REM ===========================================================================
REM Build Vorbis
REM ===========================================================================

set PROJECT=Vorbis
set ROOTDIR=libvorbis-1.3.2
set TESTFILE=win32\VS2010\Win32\Debug\libvorbis.lib 

echo pushd .>_t.bat
echo cd win32\VS2010>>_t.bat
echo pushd .>>_t.bat
echo msbuild vorbis_dynamic.sln /p:"Configuration=Debug">>_t.bat
echo msbuild vorbis_dynamic.sln /p:"Configuration=Release">>_t.bat
echo popd >>_t.bat
echo popd >>_t.bat

call:build_project %PROJECT% %ROOTDIR% %TESTFILE%
IF %FAILURE%==1 (goto:EOF)

call:copy_files %ROOTDIR%\include\vorbis\*.h %PSDK%\libvorbis\include\vorbis
call:copy_files %ROOTDIR%\win32\VS2010\Win32\Debug\*.lib %PSDK%\libvorbis\lib\Debug
call:copy_files %ROOTDIR%\win32\VS2010\Win32\Debug\*.dll %PSDK%\libvorbis\bin\Debug
call:copy_files %ROOTDIR%\win32\VS2010\Win32\Release\*.lib %PSDK%\libvorbis\lib\Release
call:copy_files %ROOTDIR%\win32\VS2010\Win32\Release\*.dll %PSDK%\libvorbis\bin\Release

REM ===========================================================================
REM Build Audiere
REM ===========================================================================

set PROJECT=Audiere
set ROOTDIR=audiere_1_9_4\audiere
set TESTFILE=vc10\bin\Debug\audiere.dll

echo pushd .>_t.bat
echo cd vc10>>_t.bat
echo msbuild audiere\audiere.vcxproj /p:Configuration=Debug>>_t.bat
echo msbuild audiere\audiere.vcxproj /p:Configuration=Release>>_t.bat
echo popd>>_t.bat

call:build_project %PROJECT% %ROOTDIR% %TESTFILE%
IF %FAILURE%==1 (goto:EOF)


REM ===========================================================================
REM Build FreeType
REM ===========================================================================

set PROJECT=FreeType
set ROOTDIR=freetype_2_4_2\freetype
set TESTFILE=objs\win32\vc2008\freetype242_D.lib

echo pushd .>_t.bat
echo cd builds\win32\vc2010>>_t.bat
echo msbuild freetype.vcxproj /p:Configuration=Debug>>_t.bat
echo msbuild freetype.vcxproj /p:Configuration=Release>>_t.bat
echo popd>>_t.bat

call:build_project %PROJECT% %ROOTDIR% %TESTFILE%
IF %FAILURE%==1 (goto:EOF)

REM ===========================================================================
REM Copy GLM
REM ===========================================================================

set PROJECT=GLM
set ROOTDIR=source\glm-0.9.2.3
set TESTFILE=glm\glm.hpp

echo pushd .>_t.bat
echo popd >>_t.bat

call:build_project %PROJECT% %ROOTDIR% %TESTFILE%
IF %FAILURE%==1 (goto:EOF)

call:copy_directory %ROOTDIR%\glm %PSDK%\glm\include\glm

REM ===========================================================================
REM Build NifLib
REM ===========================================================================

REM
REM Disable building the unit tests.  Simply can't get them to find Boost and
REM build properly.
REM
move source\niflib\test\CMakeLists.txt source\niflib\test\CMakeLists.txt.orig
echo.
echo cmake_minimum_required(VERSION 2.8)>source\niflib\test\CMakeLists.txt

set PROJECT=NifLib
set ROOTDIR=source\niflib
set TESTFILE=..\niflib_build\Debug\niflib.lib

echo pushd .>_t.bat
echo mkdir ..\niflib_build>>_t.bat
echo cd ..\niflib_build>>_t.bat
echo cmake ..\niflib>>_t.bat
echo msbuild niflib.vcxproj /p:Configuration=Debug>>_t.bat
echo msbuild niflib.vcxproj /p:Configuration=Release>>_t.bat
echo popd >>_t.bat

call:build_project %PROJECT% %ROOTDIR% %TESTFILE%
IF %FAILURE%==1 (goto:EOF)

set BUILDDIR=%ROOTDIR%\..\niflib_build
call:copy_directory %ROOTDIR%\include %PSDK%\niflib\include\niflib
call:copy_files %BUILDDIR%\Debug\*.lib %PSDK%\niflib\lib\Debug
call:copy_files %BUILDDIR%\Release\*.lib %PSDK%\niflib\lib\Release
call:copy_files %BUILDDIR%\Debug\*.dll %PSDK%\niflib\bin\Debug
call:copy_files %BUILDDIR%\Release\*.dll %PSDK%\niflib\bin\Release

REM ===========================================================================
REM Build zlib
REM ===========================================================================

set PROJECT=zlib
set ROOTDIR=source\zlib-1.2.5
set TESTFILE=..\zlib_build\Debug\zlibd.lib

echo mkdir ..\zlib_build>_t.bat
echo cd ..\zlib_build>>_t.bat
echo erase ..\zlib-1.2.5\zconf.h>>_t.bat
echo cmake ..\zlib-1.2.5 -DBUILD_SHARED_LIBS=OFF>>_t.bat
echo msbuild ALL_BUILD.vcxproj /p:Configuration=Debug >>_t.bat
echo msbuild ALL_BUILD.vcxproj /p:Configuration=Release >>_t.bat

call:build_project %PROJECT% %ROOTDIR% %TESTFILE%
IF %FAILURE%==1 (goto:EOF)   

call:copy_files %ROOTDIR%\*.h %PSDK%\zlib\include\zlib
call:copy_files %ROOTDIR%\..\zlib_build\zconf.h %PSDK%\zlib\include\zlib
call:copy_files %ROOTDIR%\..\zlib_build\Debug\*.lib %PSDK%\zlib\lib\Debug
call:copy_files %ROOTDIR%\..\zlib_build\Release\*.lib %PSDK%\zlib\lib\Release
call:copy_files %ROOTDIR%\..\zlib_build\Debug\*.dll %PSDK%\zlib\bin\Debug
call:copy_files %ROOTDIR%\..\zlib_build\Release\*.dll %PSDK%\zlib\bin\Release


REM ===========================================================================
REM Build lpng
REM ===========================================================================

set PROJECT=libpng
set ROOTDIR=source\lpng154
set TESTFILE=..\lpng_build\Release\libpng15_static.lib

echo mkdir ..\lpng_build>_t.bat
echo cd ..\lpng_build>>_t.bat
echo erase ..\lpng154\zconf.h>>_t.bat
echo cmake ..\lpng154 -DZLIB_INCLUDE_DIR=..\..\%PSDK%\zlib\include\zlib -DZLIB_LIBRARY=..\..\%PSDK%\zlib\lib -DPNG_SHARED=OFF>>_t.bat
echo msbuild png15_static.vcxproj /p:Configuration=Debug >>_t.bat
echo msbuild png15_static.vcxproj /p:Configuration=Release >>_t.bat

call:build_project %PROJECT% %ROOTDIR% %TESTFILE%
IF %FAILURE%==1 (goto:EOF)   

call:copy_files %ROOTDIR%\*.h %PSDK%\libpng\include\libpng
call:copy_files %ROOTDIR%\..\lpng_build\Debug\*.lib %PSDK%\libpng\lib\Debug
call:copy_files %ROOTDIR%\..\lpng_build\Release\*.lib %PSDK%\libpng\lib\Release
call:copy_files %ROOTDIR%\..\lpng_build\Debug\*.dll %PSDK%\libpng\bin\Debug
call:copy_files %ROOTDIR%\..\lpng_build\Release\*.dll %PSDK%\libpng\bin\Release

REM ===========================================================================
REM Build process complete
REM ===========================================================================

title Build complete.
echo.
echo.
echo Build process appears to have succeeded.
echo.

goto:EOF


REM ===========================================================================
REM Helper Functions
REM ===========================================================================

REM auto_download
REM
REM Checks if a file exists, if it does not, attempts to download it using
REM wget.  If the file is still not present, it sets FAILURE=1.
REM
REM If the file already exists, does nothing.
REM
REM
:auto_download

    IF NOT EXIST %1 ( 
        wget %DOWNLOAD_SITE%/%1 
    ) ELSE (
        echo Found %1.  Skipping download.
    )
    
    IF NOT EXIST %1 (
        echo.
        echo ERROR: Failed to download dependent file:
        echo    %1
        echo.
        echo This may be due to an inaccessible internet connection or the
        echo download site may be down.  Please attempt to extract the
        echo dependencies manually then rerun the script to continue the build
        echo and install process.
        echo.
        set FAILURE=1
    )
goto:EOF

:auto_extract_source

    IF %FAILURE%==1 (
        echo WARNING: Prior failure detected.  Skipping extraction of '%1'.
    ) ELSE (
        IF NOT EXIST source\%1 (
            pushd .
            cd source
            call:auto_download_source %2
            ..\7za.exe x %2
            popd
        ) ELSE (
            echo Found source directory '%1'
        )
    )

goto:EOF


:auto_download_source

    IF NOT EXIST %1 ( 
        ..\wget %DOWNLOAD_SITE%/extern/%1 
    ) ELSE (
        echo Found %1.  Skipping download.
    )
    
    IF NOT EXIST %1 (
        echo.
        echo ERROR: Failed to download dependent file:
        echo    %1
        echo.
        echo This may be due to an inaccessible internet connection or the
        echo download site may be down.  Please attempt to extract the
        echo dependencies manually then rerun the script to continue the build
        echo and install process.
        echo.
        set FAILURE=1
    )
goto:EOF

REM ensure_directory
REM
REM Sets FAILURE=1 if the given file/directory does not exist.
REM
REM Useful for basic sanity checking that the files are where
REM they are expected.
REM
:ensure_directory
    IF NOT EXIST %1 (
        echo ERROR: Could not find directory %1!
        echo Current directory: %CD%
        set FAILURE=1
    ) ELSE (
        echo Found directory %1
    )
goto:EOF


REM copy_directory
REM
REM
:copy_directory
IF NOT EXIST %2 ( mkdir %2 )
xcopy %1\* %2 /D /E /Y
goto:EOF

REM copy_files
REM
REM
:copy_files
IF NOT EXIST %2 ( mkdir %2 )
xcopy %1 %2 /D /S /Y
goto:EOF


REM test_file_or_fail
REM
REM
:test_file_or_fail
IF NOT EXIST %1% (
    echo.
    echo ERROR Could not find %1%!
    echo Assuming build failed.
    echo.
    echo Please post a request for help at http://athile.net/library/forums/!
    echo.
    set FAILURE=1
) ELSE (
    echo OK: Found file %1% 
)
goto:EOF

REM build_project
REM
REM Changes the working directory, checks for the existence of a test file
REM to determine if the project has already built, and then - if that file
REM does not exist - calls "_t.bat" which is the script file the caller
REM should have generated that contains the build steps.
REM
:build_project
    set PROJECT=%1
    set ROOTDIR=%2
    set TESTFILE=%3
    
    title Building %PROJECT%...
    echo.
    echo * Building %PROJECT% from %ROOTDIR%

    REM The caller is supposed to generate a file called _t.bat containing
    REM the build steps before calling build_project.
    REM
    IF NOT EXIST _t.bat (
        echo INTERNAL ERROR: Could not find build commands in _t.bat for
        echo project %PROJECT%!
        set FAILURE=1
        goto:EOF
    )

    copy /Y _t.bat _buildlast.bat
    move _t.bat %ROOTDIR%
    pushd .
    cd %ROOTDIR%
    
    REM The TESTFILE is the sentinel used to determine if a build succeeded
    REM or not.  This is not foolproof - for example, a partially 
    REM successful build could generate false positives.
    REM
    IF NOT EXIST %TESTFILE% (
        echo.
        echo * %PROJECT%: Building...
        echo.

        call _t.bat
        erase _t.bat
        
        IF NOT EXIST %TESTFILE% (
            echo.
            echo %PROJECT%: ERROR Could not find %TESTFILE%!
            echo Assuming build failed.
            echo.
            set FAILURE=1
        )
    ) ELSE (
        echo * %PROJECT%: Found %TESTFILE%.  Assuming already built.
    )
    popd
    
    IF %FAILURE%==0 ( erase _buildlast.bat )
    
goto:EOF
