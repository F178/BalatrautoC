@echo off
setlocal

set "PRESET=%~1"
if "%PRESET%"=="" set "PRESET=x64-debug"

call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
if errorlevel 1 exit /b %errorlevel%

set "CMAKE=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
"%CMAKE%" --preset "%PRESET%"
if errorlevel 1 exit /b %errorlevel%

"%CMAKE%" --build "out\build\%PRESET%"
exit /b %errorlevel%
