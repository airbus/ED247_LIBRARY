@echo off

set CMAKE_ARGS=

:GETOPTS
if /I "%1" == "-h" (
    echo "Arguments:"
    echo "  --input                : Generate as input (default: output)"
    echo "  --filepath <filepath>  : Generated ECIC filepath (default: ./ecic.xml)"
    echo "  --num_channels <num>   : Number of channels (default: 1)"
    echo "  --num_streams <num>    : Number of streams per channel (default: 1)"
    echo "  --num_signals <num>    : Number of signals per stream (for DIS/ANA/NAD/VNAD) (default: 1)"
    echo "  --stream_type <type>   : Type of stream (default: A429_Stream)"
    goto :EOF
)
if /I "%1" == "--input" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DOUTPUT=OFF"
    shift
)
if /I "%1" == "--filepath" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DFILEPATH=%2"
    shift
    shift
)
if /I "%1" == "--num_channels" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DNUM_CHANNELS=%2"
    shift
    shift
)
if /I "%1" == "--num_streams" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DNUM_STREAMS=%2"
    shift
    shift
)
if /I "%1" == "--num_signals" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DNUM_SIGNALS=%2"
    shift
    shift
)
if /I "%1" == "--stream_type" (
    set "CMAKE_ARGS=%CMAKE_ARGS% --DSTREAM_TYPE=%2"
    shift
    shift
)
if not "%1" == "" goto GETOPTS

call %~dp0win_env.bat

set "CMAKE_ARGS=%CMAKE_ARGS% -P %~dp0/../cmake/gen.cmake"

cmake %CMAKE_ARGS%

if %errorlevel% NEQ 0 (
    echo "CMake failed to test the project"
    cd /d %~dp0
    exit /b 1
)

:EOF