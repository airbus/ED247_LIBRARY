@echo off

echo ##
echo ## CMAKE
echo ##

call %~dp0win_env.bat %1

set "CMAKE_OPTIONS=-DCMAKE_TOOLCHAIN_FILE=%CMAKE_TOOLCHAIN%"
set "CMAKE_OPTIONS=%CMAKE_OPTIONS% -DCMAKE_INSTALL_PREFIX=%INSTALL_PATH%"

if not "%build_type%" == "" (
    set "CMAKE_OPTIONS=%CMAKE_OPTIONS% -DCMAKE_BUILD_TYPE=%build_type%"
) else (
    set "CMAKE_OPTIONS=%CMAKE_OPTIONS% -DCMAKE_BUILD_TYPE=Release"
)

if not "%build_samples%" == "" (
    set "CMAKE_OPTIONS=%CMAKE_OPTIONS% -DBUILD_SAMPLES=%build_samples%"
)

if not "%build_tests%" == "" (
    set "CMAKE_OPTIONS=%CMAKE_OPTIONS% -DBUILD_TESTS=%build_tests%"
)

if not "%build_docs%" == "" (
    set "CMAKE_OPTIONS=%CMAKE_OPTIONS% -DBUILD_DOCS=%build_docs%"
)

if not "%build_utils%" == "" (
    set "CMAKE_OPTIONS=%CMAKE_OPTIONS% -DBUILD_UTILS=%build_utils%"
)

if not "%test_multicast_interface_ip%" == "" (
    set "CMAKE_OPTIONS=%CMAKE_OPTIONS% -DTEST_MULTICAST_INTERFACE_IP=%test_multicast_interface_ip%"
)

cd /d %BUILD_PATH%

cmake.exe .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G"Ninja" %CMAKE_OPTIONS%

if %errorlevel% NEQ 0 (
    echo "CMake cannot configure the project"
    cd /d %~dp0
    exit /b 1
)

cd /d %~dp0
