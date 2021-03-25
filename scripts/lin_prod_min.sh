#!/bin/bash

script_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

sh ${script_path}/lin_clean.sh $1
if [[ ! $? -eq 0 ]]; then
    exit 1
fi

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
