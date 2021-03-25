@echo off

set "COMPILER=%1"

call %~dp0win_env.bat %COMPILER%

cd /d %BUILD_PATH%
cmake.exe --build . --target install
if %errorlevel% NEQ 0 (
    echo "CMake cannot install the project"
    cd /d %~dp0
    exit /b 1
)

cd /d %~dp0
