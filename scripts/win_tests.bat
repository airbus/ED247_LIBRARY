@echo off

set "COMPILER=%1"
set "TEST=%2"

call %~dp0win_env.bat %COMPILER%

cd /d %BUILD_PATH%

set "CTEST_ARG=-C Release --verbose"

if NOT "%TEST%" == "" (
    set "CTEST_ARG=%CTEST_ARG% -R %TEST%"
)

ctest %CTEST_ARG%

if %errorlevel% NEQ 0 (
    echo "CMake failed to test the project"
    cd /d %~dp0
    exit /b 1
)

cd /d %~dp0