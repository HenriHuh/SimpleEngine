@echo off
setlocal

set "REPO_ROOT=%~dp0.."

set "VSDEVCMD="
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" set "VSDEVCMD=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
if not defined VSDEVCMD if exist "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" set "VSDEVCMD=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"

set "CMAKE_BIN="
if exist "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" set "CMAKE_BIN=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
if not defined CMAKE_BIN if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" set "CMAKE_BIN=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"

set "NINJA_BIN="
if exist "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe" set "NINJA_BIN=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
if not defined NINJA_BIN if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe" set "NINJA_BIN=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"

if not defined VSDEVCMD (
    echo Could not find VsDevCmd.bat. Install Visual Studio with the Desktop development with C++ workload.
    exit /b 1
)

if not defined CMAKE_BIN (
    echo Could not find cmake.exe. Install CMake or add it to PATH.
    exit /b 1
)

if not defined NINJA_BIN (
    echo Could not find ninja.exe. Install Ninja or add it to PATH.
    exit /b 1
)

set "PATH=%CMAKE_BIN%;%NINJA_BIN%;%PATH%"

cd /d "%REPO_ROOT%"
call "%VSDEVCMD%" -arch=x64
if errorlevel 1 exit /b %errorlevel%

cmake --preset ninja-debug
if errorlevel 1 exit /b %errorlevel%

cmake --build --preset ninja-debug
if errorlevel 1 exit /b %errorlevel%

echo.
echo Build complete: build\ninja-debug\SimpleEngine.exe
