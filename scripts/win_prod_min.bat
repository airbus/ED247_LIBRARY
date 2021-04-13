@echo off

call %~dp0win_clean.bat %1
if %errorlevel% NEQ 0 (
    exit /b 1
)

@REM set build_samples=ON
@REM set build_utils=ON
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