@echo off
pushd .
cd ..\dev\libs\glgeom\doc
doxygen
popd
pushd .
cd ..\dev\libs\lxengine\doc
doxygen
popd
echo.
echo Index: 
echo   ..\dev\libs\glgeom\doc\api\html\index.html
echo   ..\dev\libs\lxengine\doc\api\html\index.html
echo.
popd
