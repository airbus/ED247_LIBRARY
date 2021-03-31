#!/bin/bash

script_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

workspace=${script_path}/..

build_path=${workspace}/_build
install_path=${workspace}/_install

if [ ! -d ${build_path} ]; then
    mkdir -p ${build_path}
fi

if [ ! -d ${install_path} ]; then
    mkdir -p ${install_path}
fi

# CMAKE
export PATH=/home/ioland/DEV/COTS/CMAKE/V3.19.4/BINARIES/Linux/bin:${PATH}
if [ -z "$1" ]; then
    compiler=gcc_x86
else
    compiler=$1
fi
cmake_toolchain=${workspace}/cmake/toolchains/linux_${compiler}.cmake
if [ ! -e ${cmake_toolchain} ]; then
    echo "CMake toolchain file ${cmake_toolchain} does not exist !"
    exit 1
fi

# NINJA
export PATH=/home/ioland/REF/COTS/NINJA/V1.8.2/BINARIES/Linux:${PATH}

# DOXYGEN
export PATH=/home/ioland/REF/COTS/DOXYGEN/V1.8.14/BINRIES/Linux/bin:${PATH}

# LCOV
export PATH=/home/ioland/REF/COTS/LCOV/V1.13.0/BINARIES/Linux64/bin:${PATH}

export PATH=/usr/include:${PATH}
export PATH=/usr/lib64:${PATH}
export LD_LIBRARY_PATH=/usr/lib64:${LD_LIBRARY_PATH}

