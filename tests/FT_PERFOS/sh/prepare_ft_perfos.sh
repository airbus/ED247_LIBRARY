#!/bin/bash

###############################################################################
# The MIT Licence                                                             #
#                                                                             #
# Copyright (c) 2019 Airbus Operations S.A.S                                  #
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

while getopts ":h" opt
do
    case "${opt}" in
        *)
            echo "Prepare the ECIC running configuration files and running environment
    TEST_LOOP_COUNT                 How many cycles should be performed (setting ED247_TEST_LOOP_COUNT environment variable)
    TEST_SAMPLE_MAX_NUMBER          Change attribute SampleMaxSize of ECIC xml files
    TEST_NUM_CHANNELS               Number of channels
    TEST_NUM_STREAMS_PER_CHANNELS   Number of streams per channels
    TEST_NUM_SIGNALS_PER_STREAMS    Number of signals per streams (for discretes only)
    TEST_MASTER_IP_ADDRESS          Ip address of the master
    TEST_SLAVE_IP_ADDRESS           Ip address of the slave
    -h                              Display this help
    
    If you are running on several computers, please setup the ED247_TEST_IP_ADDRESS to the IP address of the remote master/slave."
            ;;
    esac
done

export TEST_LOOP_COUNT=10000
export TEST_SAMPLE_MAX_NUMBER=1
export TEST_NUM_CHANNELS=1
export TEST_NUM_STREAMS_PER_CHANNELS=1
export TEST_NUM_SIGNALS_PER_STREAMS=1
export TEST_MASTER_IP_ADDRESS="127.0.0.1"
export TEST_SLAVE_IP_ADDRESS="127.0.0.1"

if [[ ! -z $1 ]]; then
    export TEST_LOOP_COUNT=$1
fi
if [[ ! -z $2 ]]; then
    export TEST_SAMPLE_MAX_NUMBER=$2
fi
if [[ ! -z $3 ]]; then
    export TEST_NUM_CHANNELS=$3
fi
if [[ ! -z $3 ]]; then
    export TEST_NUM_STREAMS_PER_CHANNELS=$4
fi
if [[ ! -z $3 ]]; then
    export TEST_NUM_SIGNALS_PER_STREAMS=$5
fi
if [[ ! -z $3 ]]; then
    export TEST_MASTER_IP_ADDRESS=$6
fi
if [[ ! -z $4 ]]; then
    export TEST_SLAVE_IP_ADDRESS=$7
fi

function write_channels () {
    echo '      <MultiChannel Name="Channel_'${NamePrefix}'_'${channel}'">' >> ${filepath}
    echo '          <FrameFormat StandardRevision="A"/>' >> ${filepath}
    echo '          <ComInterface>' >> ${filepath}
    echo '              <UDP_Sockets>' >> ${filepath}
    echo '                  <UDP_Socket DstIP="'${DstIP}'" DstPort="'$((${PortBase}+$((10#${channel}))))'" SrcPort="'$((${PortBase}+10000+10#${channel}))'" Direction="'${Direction}'"/>' >> ${filepath}
    echo '              </UDP_Sockets>' >> ${filepath}
    echo '          </ComInterface>' >> ${filepath}
    echo '          <Streams>' >> ${filepath}
    for stream in $(seq -w ${TEST_NUM_STREAMS_PER_CHANNELS}); do
        if [[ "${stream_type}" == "A429_Stream" ]]; then
            echo '              <'${stream_type}' UID="'$((10#${stream}))'" Name="Stream_'${NamePrefix}'_'$((10#${channel}))'_'$((10#${stream}))'" Direction="'${Direction}'" SampleMaxNumber="'${TEST_SAMPLE_MAX_NUMBER}'">' >> ${filepath}
        else
            echo '              <'${stream_type}' UID="'$((10#${stream}))'" Name="Stream_'${NamePrefix}'_'$((10#${channel}))'_'$((10#${stream}))'" Direction="'${Direction}'" SampleMaxNumber="'${TEST_SAMPLE_MAX_NUMBER}'">' >> ${filepath}
        fi
        if [[ "${stream_type}" == "DIS_Stream" ]]; then
            echo '                  <Signals>' >> ${filepath}
            for signal in $(seq -w ${TEST_NUM_SIGNALS_PER_STREAMS}); do
                echo '                      <Signal Name="Signal_'${NamePrefix}'_'$((10#${channel}))'_'$((10#${stream}))'_'$((10#${signal}))'" ByteOffset="'$((10#${signal}-1))'"/>' >> ${filepath}
            done
            echo '                  </Signals>' >> ${filepath}
        fi
        echo '              </'${stream_type}'>' >> ${filepath}
    done
    echo '          </Streams>' >> ${filepath}
    echo '      </MultiChannel>' >> ${filepath}
}

function write_ecic () {
    stream_type=$1
    master=$2
    filepath=$3

    cat <<END >${filepath}
<?xml version="1.0" encoding="UTF-8"?>

<!--
The MIT Licence

Copyright (c) 2019 Airbus Operations S.A.S

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
-->

END

    if [[ ${master} -eq 1 ]]; then
        echo '<ED247ComponentInstanceConfiguration Name="Master" StandardRevision="A" Identifier="0">' >> ${filepath}
    else
        echo '<ED247ComponentInstanceConfiguration Name="Slave" StandardRevision="A" Identifier="1">' >> ${filepath}
    fi
    echo '    <Channels>' >> ${filepath}
    for channel in $(seq -w ${TEST_NUM_CHANNELS}); do
        # Emitter
        if [[ ${master} -eq 1 ]]; then
            Direction="Out"
        else
            Direction="In"
        fi
        DstIP=${TEST_SLAVE_IP_ADDRESS}
        NamePrefix="M2S"
        PortBase=5000
        write_channels
        # Receiver
        if [[ ${master} -eq 1 ]]; then
            Direction="In"
        else
            Direction="Out"
        fi
        DstIP=${TEST_MASTER_IP_ADDRESS}
        NamePrefix="S2M"
        PortBase=10000
        write_channels
    done

    echo '    </Channels>' >> ${filepath}
    echo '</ED247ComponentInstanceConfiguration>' >> ${filepath}
}

script_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
config_path=${script_path}/../config/${test_name}

if [[ ! -d "${config_path}" ]]; then
    mkdir ${config_path}
fi

write_ecic "A429_Stream" 1 ${config_path}/run_a429_master.xml
write_ecic "A429_Stream" 0 ${config_path}/run_a429_slave.xml
write_ecic "DIS_Stream" 1 ${config_path}/run_dis_master.xml
write_ecic "DIS_Stream" 0 ${config_path}/run_dis_slave.xml

# files='a429 dis dis_simple disx10'

# for prefix in $files
# do

#     echo "# Configure ECIC files '${prefix}'"
#     sed 's/DstIP=\"127\.0\.0\.1\" DstPort=\"5001\"/DstIP=\"'${MASTER_IP_ADDRESS}'\" DstPort=\"5001\"/g' ${config_path}/${prefix}_master_uc.xml > ${config_path}/run_${prefix}_master_uc.xml
#     sed -i 's/DstIP=\"127\.0\.0\.1\" DstPort=\"5000\"/DstIP=\"'${SLAVE_IP_ADDRESS}'\" DstPort=\"5000\"/g' ${config_path}/run_${prefix}_master_uc.xml
#     sed -i 's/SampleMaxNumber=\"25\"/SampleMaxNumber=\"'${SAMPLE_MAX_NUMBER}'\"/g' ${config_path}/run_${prefix}_master_uc.xml
#     sed 's/DstIP=\"127\.0\.0\.1\" DstPort=\"5000\"/DstIP=\"'${SLAVE_IP_ADDRESS}'\" DstPort=\"5000\"/g' ${config_path}/${prefix}_slave_uc.xml > ${config_path}/run_${prefix}_slave_uc.xml
#     sed -i 's/DstIP=\"127\.0\.0\.1\" DstPort=\"5001\"/DstIP=\"'${MASTER_IP_ADDRESS}'\" DstPort=\"5001\"/g' ${config_path}/run_${prefix}_slave_uc.xml
#     sed -i 's/SampleMaxNumber=\"25\"/SampleMaxNumber=\"'${SAMPLE_MAX_NUMBER}'\"/g' ${config_path}/run_${prefix}_slave_uc.xml

# done

