#!/bin/bash

script_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

sh ${script_path}/lin_clean.sh $1
if [[ ! $? -eq 0 ]]; then
    exit 1
fi

export build_type=Debug
export build_samples=ON
export build_tests=ON
export build_docs=OFF
export build_utils=OFF
export test_multicast_interface_ip=
sh ${script_path}/lin_cmake.sh $1
if [[ ! $? -eq 0 ]]; then
    exit 1
fi

sh ${script_path}/lin_build.sh $1
if [[ ! $? -eq 0 ]]; then
    exit 1
fi

sh ${script_path}/lin_tests.sh $1
if [[ ! $? -eq 0 ]]; then
    exit 1
fi

export build_type=Release
export build_samples=ON
export build_tests=OFF
export build_docs=ON
export build_utils=ON
export test_multicast_interface_ip=
sh ${script_path}/lin_cmake.sh $1
if [[ ! $? -eq 0 ]]; then
    exit 1
fi

sh ${script_path}/lin_build.sh $1
if [[ ! $? -eq 0 ]]; then
    exit 1
fi

sh ${script_path}/lin_install.sh $1
if [[ ! $? -eq 0 ]]; then
    exit 1
fi
