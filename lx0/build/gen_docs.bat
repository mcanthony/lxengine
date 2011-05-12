@echo off
pushd .
cd ..\dev\libs\glgeom\doc
doxygen
echo.
echo Index: ..\dev\libs\glgeom\doc\api\html\index.html
echo.
popd
