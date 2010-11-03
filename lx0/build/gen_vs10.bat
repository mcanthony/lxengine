@echo off

call env_vs10.bat

echo.
erase CMakeCache.txt
cmake ..\dev -G "Visual Studio 10"
echo.
dir /b *.sln
echo.
echo Run:
echo --------------------------------------------------------------------------
echo msbuild lx0.sln         - to build from the command line
echo devenv lx0.sln          - to bring up the Visual Studio IDE
echo msbuild install.vcxproj - to install dependent binaries 
echo.
echo (Need Help? Visit http://www.athile.net for support or to leave suggestions
echo on how to improve the process.)
echo.