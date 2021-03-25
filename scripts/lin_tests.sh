#!/bin/bash

if [ -z "${enable_lcov}" ]; then
    ENABLE_LCOV=1
fi

script_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

. ${script_path}/lin_env.sh $1
TEST=$2

CTEST_ARGS="-C Release --verbose"
if [[ ! "${TEST}" == "" ]]; then
    echo "Run test: ${TEST}"
    CTEST_ARGS="${CTEST_ARGS} -R ${TEST}"
    ENABLE_LCOV=0
fi

pushd ${build_path}

if [ ${ENABLE_LCOV} -eq 1 ]; then
    lcov_doc_path=${build_path}/doc/lcov
    lcov_tmp_path=${lcov_doc_path}/tmp
    lcov_init_tmp_filepath=${lcov_tmp_path}/ed247_library.init.tmp.lcov
    lcov_init_filepath=${lcov_tmp_path}/ed247_library.init.lcov
    lcov_info_tmp_filepath=${lcov_tmp_path}/ed247_library.info.tmp.lcov
    lcov_info_filepath=${lcov_tmp_path}/ed247_library.info.lcov
    lcov_final_filepath=${lcov_tmp_path}/ed247_library.final.lcov

    if [[ -e ${lcov_tmp_path} ]]; then
        rm -rf ${lcov_tmp_path}
    fi
    mkdir -p ${lcov_tmp_path}

    lcov --zerocounters --directory ./tests --directory ./src --directory ./shared
    lcov --capture --directory ./tests --directory ./src --directory ./shared --output-file ${lcov_init_tmp_filepath} -initial
    # lcov --remove ${lcov_init_tmp_filepath} '/usr/*' '*tests*' '*gtest*' '*utils*' '*memhooks*' '*shared*' -o ${lcov_init_filepath}
    lcov --remove ${lcov_init_tmp_filepath} '/usr/*' '*gtest*' -o ${lcov_init_filepath}
fi

ctest ${CTEST_ARGS}

if [ ${ENABLE_LCOV} -eq 1 ]; then
    lcov --capture --directory ./tests --directory ./src --directory ./shared --output-file ${lcov_info_tmp_filepath}
    # lcov --remove ${lcov_info_tmp_filepath} '/usr/*' '*tests*' '*/gtest*' '*utils*' '*memhooks*' '*shared*' -o ${lcov_info_filepath}
    lcov --remove ${lcov_info_tmp_filepath} '/usr/*' '*gtest*' -o ${lcov_info_filepath}
    lcov --add-tracefile ${lcov_init_filepath} --add-tracefile ${lcov_info_filepath} -o ${lcov_final_filepath}

    genhtml --demangle-cpp --num-space 4 --sort --function-coverage --no-branch-coverage --legend --title "ED-247 LIBRARY" -o ${lcov_doc_path} ${lcov_final_filepath}

    rm -rf ${lcov_tmp_path}
fi

if [[ ! $? -eq 0 ]]; then
    echo "CMake failed to test the project"
    popd
    exit 1
fi

popd