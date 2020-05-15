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

test_name=$(basename -s .sh $0)
cmd=${test_name##launch_}

export ED247_LOG_FILEPATH=$cmd.lib.log
if [[ `uname -s` == "Linux" ]]; then
    ./$cmd > $cmd.log 2>&1
else
    cmd "/C call ..\run_test.bat $cmd > ${cmd}.log 2>&1"
fi
result1=$?
if [[ $result1 -ne 0 ]]
then
    echo "The first application has failed, see ${test_name}_report.xml and $cmd.log files"
fi

if [[ `uname -s` == "Linux" ]]; then
    ./${cmd}_invalid_logfile > ${cmd}_invalid_logfile.log 2>&1
else
    cmd "/C call ..\run_test.bat ${cmd}_invalid_logfile > ${cmd}_invalid_logfile.log 2>&1"
fi
result2=$?
if [[ $result2 -ne 0 ]]
then
    echo "The first application has failed, see ${test_name}_invalid_logfile_report.xml and ${cmd}_invalid_logfile.log files"
fi

if [[ $result1 -ne 0 || $result2 -ne 0 ]]
then
    exit 1
fi

exit 0
