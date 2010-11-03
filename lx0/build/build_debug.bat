call env_vs10.bat
msbuild lx0.sln /p:Configuration=Debug
msbuild INSTALL.vcxproj /p:Configuration=Debug
