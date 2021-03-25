#!/bin/bash

echo "##"
echo "## CLEAN"
echo "##"
echo ""

script_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

. ${script_path}/lin_env.sh $1

if [ -d ${build_path} ]; then
    echo "Remove ${build_path}"
    rm -rf ${build_path}
fi

if [ -d ${install_path} ]; then
    echo "Remove ${install_path}"
    rm -rf ${install_path}
fi

echo ""