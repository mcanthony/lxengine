@echo off

@rem vsvars32.bat does not check if it has already been run and can overflow
@rem the PATH variable if it is run continually
@rem ----
@if "%GEN_VS10_RUN%" == "" (
   call "%VS100COMNTOOLS%vsvars32.bat"
   set "GEN_VS10_RUN=1"
)

echo.
erase CMakeCache.txt
cmake ..\dev -G "Visual Studio 10"
echo.
dir /b *.sln
echo.
echo To build from the command line, run "msbuild lx0.sln".
echo Otherwise, run "devenv lx0.sln" to bring up the IDE.
echo Run "msbuild install.vcxproj" once to install dependent 
echo binaries.
echo.