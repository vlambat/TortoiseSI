@echo off
if "%1"=="" goto missingparam
set OUTDIR=%1
set CLEAN=""
if "%2"=="clean" set CLEAN="clean"

call :nmake 1031 de
call :nmake 1041 ja
call :nmake 1028 zh_TW
call :nmake 2052 zh_CN
goto :eof

:nmake
nmake /f Makefile %CLEAN% outdir=%OUTDIR% LANGID=%1 ISO=%2
goto :eof

:missingparam
echo Missing parameter:
echo %~nx0 [output path]
pause
