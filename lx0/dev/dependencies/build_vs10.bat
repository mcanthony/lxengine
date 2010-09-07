@echo off

echo.
echo --------------------------------------------------------------------------
echo LxEngine automated dependency builder
echo (alpha)
echo.
echo Please let us know whether or not this batch file works correctly for you.
echo We are trying to improve it as much as possible.
echo --------------------------------------------------------------------------
echo.
echo.

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
    msbuild ALL_BUILD.vcxproj
    
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
    msbuild ALL_BUILD.vcxproj
        
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
REM Clean-up and verification
REM ===========================================================================

:SUCCESS
echo.
echo Build script appears to have succeeded.
goto END

:END
