#!/bin/bash

echo "##"
echo "## CMAKE"
echo "##"
echo ""

script_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

. ${script_path}/lin_env.sh $1

CMAKE_OPTIONS="-DCMAKE_TOOLCHAIN_FILE=${cmake_toolchain}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DCMAKE_INSTALL_PREFIX=${install_path}"

if [ ! -z "${build_type}" ]; then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=${build_type}"
else
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=Release"
fi

if [ ! -z "${build_samples}" ]; then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DBUILD_SAMPLES=${build_samples}"
fi

if [ ! -z "${build_tests}" ]; then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DBUILD_TESTS=${build_tests}"
fi

if [ ! -z "${build_docs}" ]; then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DBUILD_DOCS=${build_docs}"
fi

if [ ! -z "${build_utils}" ]; then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DBUILD_UTILS=${build_utils}"
fi

if [ ! -z "${test_multicast_interface_ip}" ]; then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DTEST_MULTICAST_INTERFACE_IP=${test_multicast_interface_ip}"
fi

pushd ${build_path}

cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G"Ninja" ${CMAKE_OPTIONS}

if [[ ! $? -eq 0 ]]; then
    echo "CMake cannot configure the project"
    popd
    exit 1
fi

popd

echo ""