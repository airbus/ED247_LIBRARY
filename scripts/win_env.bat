@echo off

if "%1"=="" (
    set COMPILER=mingw4.9.2_x64
) else (
    set "COMPILER=%1"
)

set "WORKSPACE=%~dp0.."

set "BUILD_PATH=%WORKSPACE%\_build"
set "INSTALL_PATH=%WORKSPACE%\_install"

if not exist "%BUILD_PATH%" mkdir "%BUILD_PATH%"
if not exist "%INSTALL_PATH%" mkdir "%INSTALL_PATH%"

set "IOLAND_ROOT=Y:\"

:: CMAKE
set "PATH=%IOLAND_ROOT%DEV\COTS\CMAKE\V3.19.4\BINARIES\Windows\bin;%PATH%"

:: NINJA
set "PATH=%IOLAND_ROOT%REF\COTS\NINJA\V1.8.2\BINARIES\Windows;%PATH%"

:: DOXYGEN
set "PATH=%IOLAND_ROOT%REF\COTS\DOXYGEN\V1.8.14\BINARIES\Windows\bin;%PATH%"

:: COMPILER
if "%COMPILER%"=="mingw4.9.2_x64" (
    set "PATH=%IOLAND_ROOT%REF\COTS\MinGW\V4.9.2\BINARIES\Win64\bin;%IOLAND_ROOT%REF\COTS\MinGW\V4.9.2\BINARIES\Win64\lib;%PATH%"
    set "CMAKE_TOOLCHAIN=%WORKSPACE%\cmake\toolchains\windows_mingw_x64.cmake"
)
if "%COMPILER%"=="mingw4.9.2_x86" (
    set "PATH=%IOLAND_ROOT%REF\COTS\MinGW\V4.9.2\BINARIES\Win32\bin;%IOLAND_ROOT%REF\COTS\MinGW\V4.9.2\BINARIES\Win32\lib;%PATH%"
    set "CMAKE_TOOLCHAIN=%WORKSPACE%\cmake\toolchains\windows_mingw_x86.cmake"
)
if "%COMPILER%"=="msvc2019_x64" (
    if "%ALREADY_DONE%"=="1" (
        echo Environment already setup
    ) else (
        call C:/PROGRA~2/MICROS~4/2019/COMMUN~1/VC/AUXILI~1/Build/vcvars64.bat
        set ALREADY_DONE=1
    )
    set "CMAKE_TOOLCHAIN=%WORKSPACE%\cmake\toolchains\windows_msvc_x64.cmake"
)
if "%COMPILER%"=="msvc2019_x86" (
    if "%ALREADY_DONE%"=="1" (
        echo Environment already setup
    ) else (
        call C:/PROGRA~2/MICROS~4/2019/COMMUN~1/VC/AUXILI~1/Build/vcvarsamd64_x86.bat
        set ALREADY_DONE=1
    )
    set "CMAKE_TOOLCHAIN=%WORKSPACE%\cmake\toolchains\windows_msvc_x86.cmake"
)
if "%COMPILER%"=="msvc2017_x64" (
    if "%ALREADY_DONE%"=="1" (
        echo Environment already setup
    ) else (
        call C:/PROGRA~2/MICROS~4/2017/COMMUN~1/VC/AUXILI~1/Build/vcvars64.bat
        set ALREADY_DONE=1
    )
    set "CMAKE_TOOLCHAIN=%WORKSPACE%\cmake\toolchains\windows_msvc_x64.cmake"
)
if "%COMPILER%"=="msvc2017_x86" (
    if "%ALREADY_DONE%"=="1" (
        echo Environment already setup
    ) else (
        call C:/PROGRA~2/MICROS~4/2017/COMMUN~1/VC/AUXILI~1/Build/vcvarsamd64_x86.bat
        set ALREADY_DONE=1
    )
    set "CMAKE_TOOLCHAIN=%WORKSPACE%\cmake\toolchains\windows_msvc_x86.cmake"
)
if "%COMPILER%"=="msvc2015_x64" (
    if "%ALREADY_DONE%"=="1" (
        echo Environment already setup
    ) else (
        call C:/PROGRA~2/MICROS~2.0/VC/vcvarsall.bat amd64 8.1
        set ALREADY_DONE=1
    )
    set "CMAKE_TOOLCHAIN=%WORKSPACE%\cmake\toolchains\windows_msvc_x64.cmake"
)
if "%COMPILER%"=="msvc2015_x86" (
    if "%ALREADY_DONE%"=="1" (
        echo Environment already setup
    ) else (
        call C:/PROGRA~2/MICROS~2.0/VC/vcvarsall.bat amd64_x86 8.1
        set ALREADY_DONE=1
    )
    set "CMAKE_TOOLCHAIN=%WORKSPACE%\cmake\toolchains\windows_msvc_x86.cmake"
)


:: CMAKE TOOLCHAIN
if not exist "%CMAKE_TOOLCHAIN%" (
    echo CMake toolchain file %CMAKE_TOOLCHAIN% does not exist !
    exit /b 1
)