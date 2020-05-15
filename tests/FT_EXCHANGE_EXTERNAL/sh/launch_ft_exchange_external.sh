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
test_name_tester=${test_name}_tester

cmd=${test_name##launch_}
cmd_tester=${test_name_tester##launch_}

if [[ `uname -s` == "Linux" ]]; then
    ./$cmd ecic_1_complex.xml > $cmd.log 2>&1&
    ./$cmd_tester ecic_2_complex.xml > $cmd_tester.log 2>&1
else
    cmd "/C start /b call ../run_test.bat ${cmd_tester} ecic_2_complex.xml > ${cmd_tester}.log 2>&1"
    cmd "/C call ../run_test.bat ${cmd} ecic_1_complex.xml > ${cmd}.log 2>&1"
fi

exit $?
