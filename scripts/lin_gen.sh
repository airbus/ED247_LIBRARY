#!/bin/bash

script_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

CMAKE_ARGS=""

while getopts "hif:c:s:n:t:m:" OPTION; do
    case $OPTION in
    h)
        echo "Arguments:
    -i              : Generate as input (default: output)
    -f <filepath>   : Generated ECIC filepath (default: ./ecic.xml)
    -c <num>        : Number of channels (default: 1)
    -s <num>        : Number of streams per channel (default: 1)
    -n <num>        : Number of signals per stream (for DIS/ANA/NAD/VNAD) (default: 1)
    -t <type>       : Type of stream (default: A429_Stream)"
        exit 0
        ;;
    i)
        CMAKE_ARGS="${CMAKE_ARGS} -DOUTPUT=OFF"
        ;;
    f)
        CMAKE_ARGS="${CMAKE_ARGS} -DFILEPATH=$OPTARG"
        ;;
    c)
        CMAKE_ARGS="${CMAKE_ARGS} -DNUM_CHANNELS=$OPTARG"
        ;;
    s)
        CMAKE_ARGS="${CMAKE_ARGS} -DNUM_STREAMS=$OPTARG"
        ;;
    n)
        CMAKE_ARGS="${CMAKE_ARGS} -DNUM_SIGNALS=$OPTARG"
        ;;
    t)
        CMAKE_ARGS="${CMAKE_ARGS} -STREAM_TYPE=$OPTARG"
        ;;
    m)
        CMAKE_ARGS="${CMAKE_ARGS} -TEST_MULTICAST_INTERFACE_IP=$OPTARG"
        ;;
    *)
        echo "Incorrect options provided"
        exit 1
        ;;
    esac
done

. ${script_path}/lin_env.sh

cmake ${CMAKE_ARGS} -P ${script_path}/../cmake/gen.cmake

if [[ ! $? -eq 0 ]]; then
    echo "CMake cannot build the project"
    exit 1
fi