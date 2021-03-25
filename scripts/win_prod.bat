@echo off

call %~dp0win_clean.bat %1
if %errorlevel% NEQ 0 (
    exit /b 1
)

set build_type=Debug
set build_samples=ON
set build_tests=ON
set build_docs=OFF
set build_utils=OFF
set test_multicast_interface_ip=192.169.0.3
call %~dp0win_cmake.bat %1
if %errorlevel% NEQ 0 (
    exit /b 1
)

call %~dp0win_build.bat %1
if %errorlevel% NEQ 0 (
    exit /b 1
)

call %~dp0win_tests.bat %1
if %errorlevel% NEQ 0 (
    exit /b 1
)

set build_type=Debug
set build_samples=ON
set build_tests=OFF
set build_docs=ON
set build_utils=ON
set test_multicast_interface_ip=192.169.0.3
call %~dp0win_cmake.bat %1
if %errorlevel% NEQ 0 (
    exit /b 1
)

call %~dp0win_build.bat %1
if %errorlevel% NEQ 0 (
    exit /b 1
)

call %~dp0win_install.bat %1
if %errorlevel% NEQ 0 (
    exit /b 1
)