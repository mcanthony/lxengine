@echo off
call "%VS100COMNTOOLS%vsvars32.bat"
echo.
cmake ..\dev -G "Visual Studio 10"
echo.
dir /b *.sln
echo.
echo To build from the command line, run "msbuild lx0.sln".
echo Otherwise, run "devenv lx0.sln" to bring up the IDE.
echo.