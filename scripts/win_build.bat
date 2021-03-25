@echo off

echo ##
echo ## BUILD
echo ##

call %~dp0win_env.bat %1

cd /d %BUILD_PATH%

cmake.exe --build . --target all

if %errorlevel% NEQ 0 (
    echo "CMake cannot build the project"
    cd /d %~dp0
    exit /b 1
)

cd /d %~dp0
