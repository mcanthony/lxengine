cd ..
mkdir build
cd build
call "%VS100COMNTOOLS%vsvars32.bat"
cmake ..\lxengine -G "Visual Studio 10"
dir *.sln