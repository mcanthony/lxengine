@echo off
REM ===========================================================================
REM Front Matter...
REM ===========================================================================
REM
REM The purpose of this batch file is simply to illustrate various DOS batch
REM file programming techniques.  The batch file itself does nothing useful.
REM
REM 
REM
REM LICENSE 
REM (http://www.opensource.org/licenses/mit-license.php)
REM
REM Copyright (c) 2010 athile@athile.net (http://www.athile.net)
REM
REM Permission is hereby granted, free of charge, to any person obtaining a 
REM copy of this software and associated documentation files (the "Software"), 
REM to deal in the Software without restriction, including without limitation 
REM the rights to use, copy, modify, merge, publish, distribute, sublicense, 
REM and/or sell copies of the Software, and to permit persons to whom the 
REM Software is furnished to do so, subject to the following conditions:
REM
REM The above copyright notice and this permission notice shall be included in
REM all copies or substantial portions of the Software.
REM
REM THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
REM IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
REM FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
REM AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
REM LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
REM FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
REM IN THE SOFTWARE.
REM
REM ===========================================================================


REM ===========================================================================
REM Main entry point
REM ===========================================================================

echo.
echo Running sample DOS batch file...
echo.

REM
REM Demonstrate some function calls
REM
call:checkFileExists test.bat
call:checkFileExists subdir
call:checkFileExists subdir2

echo.
echo DOS batch file done.
echo.
goto:EOF

REM ===========================================================================
REM Exit point
REM ===========================================================================

REM ===========================================================================
REM Helper Functions
REM ===========================================================================

REM
REM Define a function
REM See: http://www.dostips.com/DtTutoFunctions.php
REM
:checkFileExists
IF EXIST "%~1" (
    echo Found file %~1
) ELSE (
    echo Could not find file %~1!
)
goto:EOF
