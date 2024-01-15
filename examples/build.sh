#!/bin/bash
#
# Simple wrapper to set COTs before running CMake
#
SOURCE_DIR=$(readlink -f `dirname $0`)
BUILD_ROOT=${SOURCE_DIR}/build
#BUILD_VERBOSE_FLAG=--verbose


die() { echo $*; exit 1; }
help() { echo "USAGE: `basename $0` [example]"; }

# Set cots
if [ -e ${SOURCE_DIR}/local_cots.env ]; then
    . ${SOURCE_DIR}/local_cots.env
else
    os() {
        if [ "$OS" = "Windows_NT" ]; then
            echo $OS
        else
            uname -s
        fi
    }

    case $(os) in
        Linux)
            CMAKE_BIN=/home/CrossTools/Cots/CMAKE/V3.22.0/BINARIES/Linux64/bin/cmake
            CMAKE_GENERATOR="Unix Makefiles"
            GENERATOR_BIN=make

            export LIBED247_ROOT=/home/CrossTools/ED247/ED247_LIBRARY/Dev/VA2.0.3_Beta1/Linux64
            ;;

        Windows_NT)
            CMAKE_BIN=//filer011/CrossTools/Cots/CMAKE/V3.22.0/BINARIES/Win64/bin/cmake.exe

            CMAKE_GENERATOR=Ninja
            GENERATOR_BIN=//filer011/CrossTools/Cots/NINJA/1.10.2/BINARIES/Win64/ninja.exe

            MinGW_HOME=//filer011/CrossTools/Cots/MINGW/MinGW64_GCC-10.2.0_CRT-8.0.0
            export CC=${MinGW_HOME}/bin/gcc.exe
            export CXX=${MinGW_HOME}/bin/g++.exe

            export LIBED247_ROOT=//filer011/CrossTools/ED247/ED247_LIBRARY/Ref/V2.0.2/Win64_mingw
            ;;
        *)
            echo "Unsuported system $($os)"
            ;;
    esac
fi


# Parse args
TARGET=all
while [ $# -ne 0 ]; do
      case $1 in
          -h|--help)
              help
              exit
              ;;
          *)
              [ "$TARGET" != "all" ] && die "Only one target is expected!";
              TARGET=$1
              ;;
      esac
      shift
done


# Build
cmake_configure() {
    (
        set -x
        mkdir -p $BUILD_ROOT
        cd $BUILD_ROOT || exit 1
        ${CMAKE_BIN} ${SOURCE_DIR}                            \
              -DCMAKE_GENERATOR="${CMAKE_GENERATOR}"          \
              -DCMAKE_MAKE_PROGRAM="${GENERATOR_BIN}"
    ) || exit 1
}

cmake_build() {
    [ -z $1 ] && die "cmake_build expect an argument!"
    (
        set -x
        ${CMAKE_BIN} --build ${BUILD_ROOT} --target $1 ${BUILD_VERBOSE_FLAG}
    ) ||  exit 1
}

[ ! -d $BUILD_ROOT ] && cmake_configure
cmake_build $TARGET
