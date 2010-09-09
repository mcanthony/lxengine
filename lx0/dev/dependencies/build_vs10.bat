@echo off

REM ===========================================================================
REM Todos
REM ===========================================================================
REM
REM TODO: Convert this to a CMake file.  It can handle all this logic and will
REM       make building for the "current" configuration easier.
REM 
REM TODO: Move dependencies into a dependencies/src directory, then COPY all 
REM       built files (bin, lib, include, pdb) into a dependencies/sdk 
REM       directory
REM
REM TODO: Add 64-bit / 32-bit support
REM TODO: Add VS2008 support (convert to CMake first)
REM
REM ===========================================================================


REM ===========================================================================
REM Beginning of script
REM ===========================================================================

echo.
echo --------------------------------------------------------------------------
echo LxEngine automated dependency builder
echo (alpha)
echo.
echo As this dependency builder is in active development, feedback is 
echo appreciated on what did or did not work for you.
echo --------------------------------------------------------------------------
echo.
echo.

REM ===========================================================================
REM Grab the dependencies
REM ===========================================================================

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

    wget http://www.athile.net/files/7za.exe
    
    IF NOT EXIST 7za.exe (
        echo.
        echo ERROR: Failed to download dependencies.  Is your computer connected
        echo to the internet?
        echo.
        goto END
    )
    
    IF NOT EXIST dependencies.7z (
        wget http://www.athile.net/files/dependencies.7z
    )
    IF NOT EXIST dependencies.7z (
        echo.
        echo ERROR: Failed to download dependencies.  Is your computer connected
        echo to the internet?
        echo.
        goto END
    ) 
    
    7za x dependencies.7z   
)

REM ===========================================================================
REM Ensure Visual Studio 10 command-line is available
REM ===========================================================================

IF "%VSINSTALLDIR%" == "" (
    IF "%VS100COMNTOOLS%" == "" (
        echo Error: Could not find Visual Studio 2010 installation!
        echo Normally the VS100COMNTOOLS environment variable will be set
        echo if Visual Studio 2010 is installed.
        echo.
        echo Exiting!
        echo.
        goto END
    ) ELSE (
        call "%VS100COMNTOOLS%\vsvars32.bat"
    )
)

REM ===========================================================================
REM Check the dependency source is installed properly
REM ===========================================================================

IF NOT EXIST boost_1_44_0 (
    echo ERROR: Could not find Boost in dependencies\boost_1_44_0!
    goto END
)
if NOT EXIST ogre_1_7_1 (
    echo ERROR: Could not find Ogre in dependencies\ogre_1_7_1!
    goto END
)
if NOT EXIST bullet_2_76 (
    echo ERROR: Could not find Bullet in dependencies\bullet_2_76!
    goto END
)

if NOT EXIST openal_1_1 (
    echo ERROR: Could not find OpenAL in dependencies\openal_1_1!
    goto END
)

set PYTHONEXE_PATH=%SystemDrive%\Python27
IF EXIST "%PYTHONEXE_PATH%\python.exe" ( goto PYTHONFOUND )
set PYTHONEXE_PATH=%SystemDrive%\Python26
IF EXIST "%PYTHONEXE_PATH%\python.exe" ( goto PYTHONFOUND )
set PYTHONEXE_PATH=%SystemDrive%\Python25
IF EXIST "%PYTHONEXE_PATH%\python.exe" ( goto PYTHONFOUND )
echo ERROR: Could not locate Python installation!
echo.
goto END

:PYTHONFOUND
IF NOT EXIST "%PYTHONEXE_PATH%\Scripts\scons.bat" (
    echo ERROR: Could not find scons installed within %PYTHONPATH%
    echo.
    goto END
)


IF "%DXSDK_DIR%" == "" (
    echo ERROR: Could not find DirectX SDK!
    echo.
    goto END
)

REM ===========================================================================
REM Build Boost
REM ===========================================================================

pushd .
cd boost_1_44_0\boost_1_44_0
    
IF NOT EXIST stage\lib\libboost_iostreams-vc100-mt-gd-1_44.lib (
    echo.
    echo * BOOST: Building...
    echo.

    call bootstrap.bat
    bjam.exe
    bjam.exe install --prefix=sdk

) ELSE (
    echo * BOOST: Found at least one Boost library.  Assuming Boost has already been built.
)

REM
REM Check that it really did succeed
REM

IF NOT EXIST stage\lib\libboost_iostreams-vc100-mt-gd-1_44.lib (
    echo.
    echo ERROR: Cannot find compiled Boost libraries!  Did Boost fail to 
    echo compile?
    echo.
    popd
    goto END
)
popd

REM ===========================================================================
REM Build OGRE
REM ===========================================================================

pushd .
cd ogre_1_7_1\ogre_build

IF EXIST bin\debug\OgreMain_d.dll (
    echo * OGRE: Found OgreMain_d.dll.  Presuming OGRE has been built already.
) ELSE (
    echo.
    echo * OGRE: Building...
    echo.
    
    cmake ..\ogre_src -DOGRE_DEPENDENCIES_DIR=..\OgreDependencies_MSVC_20100501\Dependencies -DBOOST_ROOT=..\..\boost_1_44_0\boost_1_44_0
    
    IF NOT EXIST ALL_BUILD.vcxproj (
        echo.
        echo ERROR: CMake failed to generate project files for building OGRE!
        echo See CMake error log.
        echo.
        popd
        goto END
    )
    msbuild ALL_BUILD.vcxproj /p:Configuration=Debug
    msbuild ALL_BUILD.vcxproj /p:Configuration=Release
    msbuild INSTALL.vcxproj
    copy bin\Debug\*.pdb sdk\bin\Debug
    copy bin\Debug\*.cfg sdk\bin\Debug
    copy bin\Release\*.pdb sdk\bin\Release
    copy bin\Release\*.cfg sdk\bin\Release
    
    IF NOT EXIST bin\debug\OgreMain_d.dll (
        echo.
        echo ERROR: OGRE build failed.  Could not find bin\debug\OgreMain_d.dll.
        echo.
        popd
        goto END
    )
)

popd

REM ===========================================================================
REM Build OIS
REM ===========================================================================

pushd .
cd ois_1_2_0\ois

IF EXIST lib\OIS_static_d.lib (
    echo * OIS: Found OIS_static_d.lib
) ELSE (
    echo.
    echo * OIS: Building...
    echo.
 
    cd Win32
    msbuild OIS.vcxproj /p:Configuration=Debug
    msbuild OIS.vcxproj /p:Configuration=Release
    cd ..
    
    IF NOT EXIST lib\OIS_static_d.lib (
        echo.
        echo ERROR: OIS build failed.  Could not find lib\OIS_static_d.lib
        echo.
        popd
        goto END
    )
)
popd

REM ===========================================================================
REM Build Bullet
REM ===========================================================================


pushd .
cd bullet_2_76\bullet_build

IF EXIST lib\Debug\BulletCollision.lib (
    echo * BULLET: Found BulletCollision.lib.  Presuming Bullet has been built already.
) ELSE (
    echo.
    echo * BULLET: Building...
    echo.
    
    cmake ..\bullet_src 
    
    IF NOT EXIST ALL_BUILD.vcxproj (
        echo.
        echo ERROR: CMake failed to generate project files for building Bullet!
        echo See CMake error log.
        echo.
        popd
        goto END
    )
    msbuild ALL_BUILD.vcxproj /p:Configuration=Debug
    msbuild ALL_BUILD.vcxproj /p:Configuration=Release
        
    IF NOT EXIST lib\Debug\BulletCollision.lib (
        echo.
        echo ERROR: Bullet build failed.  Could not find lib\Debug\BulletCollision.lib.
        echo.
        popd
        goto END
    )
)

popd

REM ===========================================================================
REM Build Google V8
REM ===========================================================================

pushd .
cd v8\v8

IF EXIST sdk\lib\v8.lib (
    echo * V8: Found v8.lib.  Presuming V8 has been built already.
) ELSE (
    echo.
    echo * V8: Building...
    echo.
 
    call "%PYTHONEXE_PATH%\Scripts\scons.bat" env="PATH:%PATH%,INCLUDE:%INCLUDE%,LIB:%LIB%"
    
    IF EXIST v8.lib (
        mkdir sdk
        mkdir sdk\lib
        mkdir sdk\include
        mkdir sdk\include\v8
        call copy v8.lib sdk\lib
        call copy include\* sdk\include\v8
    )
        
    IF NOT EXIST sdk\lib\v8.lib (
        echo.
        echo ERROR: V8 build failed.  Could not find sdk\lib\v8.lib
        echo.
        popd
        goto END
    )
)
popd

REM ===========================================================================
REM Build OpenAL Software Implementation
REM ===========================================================================

pushd .
cd openal_1_1\openal

IF EXIST OpenAL-Soft\build\Debug\openal32.lib (
    echo * OpenAL: Found openal32.lib.  Presuming OpenAL has been built already.
) ELSE (
    echo.
    echo * OpenAL: Building...
    echo.
 
    cd OpenAL-Soft\build
    pushd .
    cmake ..
    msbuild OpenAL.sln /p:Configuration=Debug
    msbuild OpenAL.sln /p:Configuration=Release
    popd
           
    IF NOT EXIST OpenAL-Soft\build\Debug\openal32.lib (
        echo.
        echo ERROR: OpenAL build failed.  Could not find OpenAL-Soft\build\Debug\openal32.lib
        echo.
        popd
        goto END
    )
)
popd


REM ===========================================================================
REM Clean-up and verification
REM ===========================================================================

:SUCCESS
echo.
echo Build script appears to have succeeded.
goto END

:END
