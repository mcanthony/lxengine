call env_vs10.bat
msbuild lx0.sln /p:Configuration=Release
msbuild INSTALL.vcxproj /p:Configuration=Release
