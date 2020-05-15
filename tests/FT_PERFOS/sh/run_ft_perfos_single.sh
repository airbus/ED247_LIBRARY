#!/bin/bash

###############################################################################
# The MIT Licence                                                             #
#                                                                             #
# Copyright (c) 2020 Airbus Operations S.A.S                                  #
#                                                                             #
# Permission is hereby granted, free of charge, to any person obtaining a     #
# copy of this software and associated documentation files (the "Software"),  #
# to deal in the Software without restriction, including without limitation   #
# the rights to use, copy, modify, merge, publish, distribute, sublicense,    #
# and/or sell copies of the Software, and to permit persons to whom the       #
# Software is furnished to do so, subject to the following conditions:        #
#                                                                             #
# The above copyright notice and this permission notice shall be included     #
# in all copies or substantial portions of the Software.                      #
#                                                                             #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  #
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    #
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE #
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      #
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     #
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         #
# DEALINGS IN THE SOFTWARE.                                                   #
###############################################################################

test_name=ft_perfos

script_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

master=${test_name}_master
slave=${test_name}_slave

if  [[ $# -ne 2 ]]; then
    echo "Invalid number of argument: launch.sh [master/slave] [REMOTE IP ADDRESS]"
    exit 1
fi

export MS=${1}
export ED247_TEST_IP_ADDRESS=${2}

pushd ${script_path}

if [[ `uname -s` == "Linux" ]]; then
    if [[ "${MS}" == "master" ]]; then
        ./$master
    else
        ./$slave
    fi    
else
    if [[ "${MS}" == "master" ]]; then
        cmd "/C call ../run_test.bat ${master}"
    else
        cmd "/C call ../run_test.bat ${slave}"
    fi
fi

popd

exit $?