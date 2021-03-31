#!/bin/bash

echo "##"
echo "## BUILD"
echo "##"
echo ""

script_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

. ${script_path}/lin_env.sh $1

pushd ${build_path}
cmake --build . --target all
if [[ ! $? -eq 0 ]]; then
    echo "CMake cannot build the project"
    popd
    exit 1
fi

popd

echo ""