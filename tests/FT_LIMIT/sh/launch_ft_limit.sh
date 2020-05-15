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
test_name_send=${test_name}_send
test_name_recv=${test_name}_recv

cmd=${test_name##launch_}
cmd_send=${test_name_send##launch_}
cmd_recv=${test_name_recv##launch_}

nb_channels=10
nb_streams_per_channels=100

function write_ecic () {
    if [[ $# != 2 ]]
    then
        echo "Function takes 2 arguments: Filename InOrOut"
        return 1
    fi

    echo '<?xml version="1.0" encoding="UTF-8"?>' > $1    
    echo ' ' >> $1
    echo '<!--' >> $1
    echo 'The MIT Licence' >> $1
    echo ' ' >> $1
    echo 'Copyright (c) 2020 Airbus Operations S.A.S' >> $1
    echo ' ' >> $1
    echo 'Permission is hereby granted, free of charge, to any person obtaining a' >> $1
    echo 'copy of this software and associated documentation files (the "Software"),' >> $1
    echo 'to deal in the Software without restriction, including without limitation' >> $1
    echo 'the rights to use, copy, modify, merge, publish, distribute, sublicense,' >> $1
    echo 'and/or sell copies of the Software, and to permit persons to whom the' >> $1
    echo 'Software is furnished to do so, subject to the following conditions:' >> $1
    echo ' ' >> $1
    echo 'The above copyright notice and this permission notice shall be included' >> $1
    echo 'in all copies or substantial portions of the Software.' >> $1
    echo ' ' >> $1
    echo 'THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR' >> $1
    echo 'IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,' >> $1
    echo 'FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE' >> $1
    echo 'AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER' >> $1
    echo 'LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING' >> $1
    echo 'FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER' >> $1
    echo 'DEALINGS IN THE SOFTWARE.' >> $1
    echo '-->' >> $1
    echo ' ' >> $1
    echo '<ED247ComponentInstanceConfiguration ComponentType="Virtual" Name="Component_'$2'" Comment="" StandardRevision="A" Identifier="0"' >> $1
    echo '        xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="ED247A_ECIC.xsd">' >> $1
    echo '    <Channels>' >> $1
    
    # Fill with multi-channels
    for channel in $(seq -w $nb_channels)
    do
        echo '        <MultiChannel Name="Channel_'$channel'">' >> $1
        echo '            <FrameFormat StandardRevision="A"/>' >> $1
        echo '            <ComInterface>' >> $1
        echo '                <UDP_Sockets>' >> $1
        echo '                    <UDP_Socket DstIP="224.1.1.1" DstPort="'$((4000+$((10#$channel))))'"/>' >> $1
        echo '                </UDP_Sockets>' >> $1
        echo '            </ComInterface>' >> $1
        echo '            <Streams>' >> $1
        for stream in $(seq -w $nb_streams_per_channels)
        do
            echo '                <A429_Stream UID="'$((10#$stream))'" Name="C'$channel'S'$stream'" Direction="'$2'"/>' >> $1
        done
        echo '            </Streams>' >> $1
        echo '        </MultiChannel>' >> $1
    done

    echo '    </Channels>' >> $1
    echo '</ED247ComponentInstanceConfiguration>' >> $1
}

write_ecic ${cmd_send}.xml Out 
write_ecic ${cmd_recv}.xml In

if [[ `uname -s` == "Linux" ]]; then
    ./$cmd_recv > $cmd_recv.log 2>&1&
    ./$cmd_send > $cmd_send.log 2>&1
else
    cmd "/C start /b call ..\run_test.bat ${cmd_recv} > ${cmd_recv}.log 2>&1"
    cmd "/C call ..\run_test.bat $cmd_send > ${cmd_send}.log 2>&1"
    exit $?
fi

exit $?
