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

###############################################################################
# Common material                                                             #
###############################################################################

script_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

product="LIBED247"
version=$(cat ${script_path}/VERSION)

# Error handler
function error
{
    if [[ "$1" != "" ]]; then
        echo "*** $1 ***"
    fi
    echo "*** ERROR ***"
    exit 1
}

wordsizes=("x86" "x64")
if [[ `uname -s` == "Linux" ]]; then
    target="linux"
    function convert2realpath {
        echo "$1"
    }
    function convert2cygpath {
        echo "$1"
    }
    compilers=("gcc4.8.5")
else
    target="windows"
    function convert2realpath {
        cygpath -d "$1" 2>1 >/dev/null
        if [[ $? == 0 ]]; then
            tmp=$(cygpath -d "$1")
            echo "${tmp[*]}"
        else
            echo "$(cygpath -w $1)"
        fi
    }

    function convert2cygpath {
        echo "$(cygpath $1)"
    }
    case "${script_path}" in
        //* | \\\\*)
            error "${PRODUCT} cannot be compiled from an UNC path (e.g. \\\\filer\\${PRODUCT} or //filer/${PRODUCT}). You must mount it on a Windows drive, and use its local path to compile it."
            ;;
    esac
    compilers=("mingw4.9.2" "msvc2019" "msvc2017" "msvc2015")
fi

build_path="${script_path}/build"
install_path="${script_path}/install"
if [[ -f "${script_path}/my_dependencies.sh" ]]; then
    default_depencies_file="${script_path}/my_dependencies.sh"
else
    default_depencies_file="${script_path}/dependencies.sh"
fi


# Display executed command and result
function issue_command
{
    echo "+ ""$@"
    "$@"
}

# Display list of available command in mgr.sh
function print_cmd_list
{
    echo "List of available commands:"
    echo "Command lines must have the following format: 'mgr.sh command [options] [compiler arch [other_parameters]]'"
    echo "The available commands with associated option is listed below:"
    for cmd in ${cmdlist}; do
        var_name="cmdhelp_${cmd}"
        help="${!var_name}"
        printf "   %-8s\n    %s\n" "${cmd}" "${help}"
    done
}

# Setup environment
function source_generated_env
{
    if [[ ! -e "${generated_env}" ]]; then
        error "Cannot find generated environment file. Please run 'build' target first."
    fi
    . ${generated_env}
}

function retrieve_compiler_and_wordsize
{
    # Retrieve compiler & wordsize
    _COMPILER=${1}
    if [[ -z "${_COMPILER}" ]]; then
        _COMPILER=${compilers[0]}
    else
        local ok=0
        for c in "${compilers[@]}"; do
            if [[ ${c} == ${_COMPILER} ]]; then
                ok=1
            fi
        done
        if [[ ${ok} -eq 0 ]]; then
            error "Unknown compiler [${_COMPILER}]. Available compilers: ${compilers[@]}"
        fi
    fi
    echo "# Selected compler: ${_COMPILER}"
    _WORDSIZE=${2}
    if [[ -z "${_WORDSIZE}" ]]; then
        _WORDSIZE=${wordsizes[0]}
    else
        local ok=0
        for w in "${wordsizes[@]}"; do
            if [[ ${w} == ${_WORDSIZE} ]]; then
                ok=1
            fi
        done
        if [[ ${ok} -eq 0 ]]; then
            error "Unknown wordsize [${_WORDSIZE}]. Available wordsizes: ${wordsizes[@]}"
        fi
    fi
    echo "# Selected word size: ${_WORDSIZE}"

    # Setup compilation & install directories
    suffix_path=${target}/${_COMPILER}/${_WORDSIZE}
    build_target=${build_path}/${suffix_path}
    echo "# Build directory : ${build_target}"
    install_target=${install_path}/${suffix_path}
    echo "# Install directory : ${install_target}"
    tests_path="${install_target}/tests"
    test_env_filepath="${tests_path}/setup_test_env.sh"
    if [[ ${_COMPILER} =~ ^msvc.* ]] || [[ ${_COMPILER} =~ ^mingw.* ]]; then
        test_run_filename="run_test.bat"
        test_run_filepath="${tests_path}/${test_run_filename}"
        test_filename="test.sh"
        test_filepath="${tests_path}/${test_filename}"
    else
        test_filepath="${tests_path}/test.sh"
    fi
    generated_cmake="${build_target}/_dependencies.cmake"
}

###############################################################################
# Build                                                                       #
###############################################################################
cmdhelp_build="# Build and put it in 'install' directory
     compiler           Compiler (first default): ${compilers[@]}
     wordsize           Word size (first default): ${wordsizes[@]}
     -c                 Clean first
     -o                 Generate Doxygen code documentation
     -d dependency_file Use a custom dependency file"
function cmd_build
{

    _CLEAN=
    _DOX=0
    _DEP="${default_depencies_file}"
    
    while getopts ":cod:" opt
    do
        case "${opt}" in
            c)
                _CLEAN=1
                ;;
            o)
                _DOX=1
                ;;
            d)
                _DEP=$(realpath ${OPTARG})
                echo "${OPTARG} => ${_DEP}"
                ;;
            *)
                error "Unknown option [${opt}]"
                ;;
        esac
    done

    shift $((OPTIND-1))

    retrieve_compiler_and_wordsize ${1} ${2}

    # Call clean if needed
    if [[ ! -z ${_CLEAN} ]]; then
        echo "# Cleaning ..."
        issue_command rm -rf ${build_target}
        issue_command rm -rf ${install_target}
        echo "# Cleaning done"
    fi

    # Setup directories
    mkdir -p ${build_target}
    mkdir -p ${install_target}

    # Setup dependencies if needed
    generate_dependencies_files "${_DEP}"

    if [[ ${target} == "linux" ]]; then
        source_generated_env

        # Build
        issue_command pushd ${build_target}
        cmake $(convert2realpath "${script_path}") -G"Ninja" \
            -DENABLE_DOC=${_DOX} \
            -DCMAKE_BUILD_TYPE=debug \
            -DCMAKE_INSTALL_PREFIX=$(convert2realpath ${install_target}) || error "Cmake cannot configure the project"
        cmake --build $(convert2realpath ${build_target}) --target all || error "Cmake cannot build the project"
        if [[ ${_DOX} == 1 ]]; then
            cmake --build $(convert2realpath ${build_target}) --target doxygen || error "Cmake cannot run doxygen"
        fi
        cmake --build $(convert2realpath ${build_target}) --target install || error "Cmake cannot install the project"
        issue_command popd
        
    elif [[ ${_COMPILER} =~ ^mingw.* ]]; then
        generated_cmd=${build_target}/_cmake.bat
        cat <<END >${generated_cmd}
:: Generated by mgr.sh - $(date)

@echo off

call $(convert2realpath ${generated_env})

pushd %NINJA_PATH%

echo [NINJA] Change to %cd%
set NINJA_NEWPATH=%cd%
set PATH=%NINJA_NEWPATH%;%PATH%

pushd %DOXYGEN_PATH%

echo [DOXYGEN] Change to %cd%
set DOXYGEN_NEWPATH=%cd%
set PATH=%DOXYGEN_NEWPATH%\bin;%PATH%

pushd %MINGW_PATH%

echo [MINGW] Change to %cd%
set MINGW_NEWPATH=%cd%
set PATH=%MINGW_NEWPATH%\bin;%MINGW_NEWPATH%\lib;%PATH%

pushd %~dp0

cmake $(convert2realpath "${script_path}") -G"Ninja" -DENABLE_DOC=${_DOX} -DCMAKE_BUILD_TYPE=debug -DCMAKE_INSTALL_PREFIX=$(convert2realpath ${install_target})
if %errorlevel% NEQ 0 (
    echo "Cmake cannot configure the project"
    exit /b 1
)

cmake --build $(convert2realpath ${build_target}) --target all
if %errorlevel% NEQ 0 (
    echo "Cmake cannot build the project"
    exit /b 1
)

if "${_DOX}" == "1" (
    cmake --build $(convert2realpath ${build_target}) --target doxygen
    if !errorlevel! NEQ 0 (
        echo "Cmake cannot run doxygen"
        exit /b 1
    )
)

cmake --build $(convert2realpath ${build_target}) --target install
if %errorlevel% NEQ 0 (
    echo "Cmake cannot install the project"
    exit /b 1
)

popd

popd

popd

popd

popd

echo "# DONE"

END
        cmd "/C $(convert2realpath ${generated_cmd})"
    else
        generated_cmd=${build_target}/_cmake.bat
        cat <<END >${generated_cmd}
:: Generated by mgr.sh - $(date)

@echo off

call $(convert2realpath ${generated_env})

pushd %NINJA_PATH%

echo Change to %cd%
set NINJA_NEWPATH=%cd%
set PATH=%NINJA_NEWPATH%;%PATH%

pushd %DOXYGEN_PATH%

echo Change to %cd%
set DOXYGEN_NEWPATH=%cd%
set PATH=%DOXYGEN_NEWPATH%;%PATH%

pushd %MSVC_PATH%

echo Change to %cd%
echo call %MSVC_EXE% %MSVC_ARG% %MSVC_ARG2%
call %MSVC_EXE% %MSVC_ARG% %MSVC_ARG2%

pushd %~dp0

cmake $(convert2realpath "${script_path}") -G"Ninja" -DENABLE_DOC=${_DOX} -DCMAKE_BUILD_TYPE=debug -DCMAKE_INSTALL_PREFIX=$(convert2realpath ${install_target})
if %errorlevel% NEQ 0 (
    echo "Cmake cannot configure the project"
    exit /b 1
)

cmake --build $(convert2realpath ${build_target}) --target all
if %errorlevel% NEQ 0 (
    echo "Cmake cannot build the project"
    exit /b 1
)

if "${_DOX}" == "1" (
    cmake --build $(convert2realpath ${build_target}) --target doxygen
    if !errorlevel! NEQ 0 (
        echo "Cmake cannot run doxygen"
        exit /b 1
    )
)

cmake --build $(convert2realpath ${build_target}) --target install
if %errorlevel% NEQ 0 (
    echo "Cmake cannot install the project"
    exit /b 1
)

popd

popd

popd

echo "# DONE"
END
        cmd "/C $(convert2realpath ${generated_cmd})"
    fi
}

function define
{
    case $# in
        0|1|2)
            error "Not enough number of arguments for 'define' function";;
        3) # SUFFIX NAME VALUE
            eval ${2}_${1}="${3}"
            ;;
        4) # SUFFIX NAME OSTYPE VALUE
            if [[ ${target} == ${3} ]]; then
                eval ${2}_${1}="${4}"
            fi
            ;;
        5) # SUFFIX NAME COMPILER WORDSIZE VALUE
            if [[ (${_COMPILER} == ${3}) && (${_WORDSIZE} == ${4}) ]]; then
                eval ${2}_${1}="${5}"
            fi
            ;;
        *)
            error "Too much number of arguments for 'define' function";;
    esac
}

function define_path
{
    define PATH "$@"
}

function define_exe
{
    define EXE $@
}

function define_arg
{
    define ARG $@
}

function define_arg2
{
    define ARG2 $@
}

function generate_dependencies_files
{
    echo "Generating dependencies paths configuration files..."
    dep_file="$1"
    if [[ ! -e ${dep_file} ]]; then
        error "Cannot find dependencies file: ${dep_file}"
    fi

    if [[ ${target} == "linux" ]]; then
        generated_env=${build_target}/_dependencies.sh
    else
        generated_env=${build_target}/_dependencies.bat
    fi
    
    (
        # Setup dependencies paths and executables
        echo "# Setup dependencies"
        . ${dep_file} &&
        
        # Setup CMake variables
        LIBED247_VERSION=${version}
        COMPILER=${_COMPILER}
        WORDSIZE=${_WORDSIZE}
        cmake_variables=(COMPILER WORDSIZE LIBED247_VERSION DOXYGEN_PATH LCOV_PATH LCOV_EXE GENHTML_EXE GTEST_PATH LIBXML2_PATH XMLLINT_EXE)
        if [[ ${_COMPILER} == "mingw4.9.2" ]]; then
            cmake_variables+=(MINGW_PATH)
        fi
        if [[ ${_COMPILER} =~ ^msvc.* ]]; then
            cmake_variables+=(MSVC_PATH MSVC_EXE MSVC_ARG MSVC_ARG2)
        fi

        # Write to file
        rm -f ${generated_cmake}
        for var in "${cmake_variables[@]}"; do
            echo "# Set CMake variable [${var}]=[${!var}]"
            if [[ ! ${_COMPILER} =~ ^msvc.* ]]; then
                value=${!var}
                if [[ (${_COMPILER} =~ ^mingw.*) && (! ${value} =~ ^//.*) ]]; then
                    if [[ ${var} =~ .*_PATH$ ]]; then
                        echo "SET(${var}_CYGPATH $(convert2cygpath ${value}))"
                    fi
                    value=$(convert2realpath ${value})
                else
                    if [[ ${var} =~ .*_PATH$ ]]; then
                        echo "SET(${var}_CYGPATH ${value})"
                    fi
                fi
                echo "SET(${var} ${value//\\/\/})"
            elif [[ ! -z ${!var} ]]; then
                vvar=${!var}
                value=$(convert2realpath "${vvar}")
                echo "SET(${var} ${value//\\/\/})"
                echo "SET(${var}_CYGPATH ${value//\\/\/})"
            fi
        done >> ${generated_cmake}

        # Setup environment variables
        generated_path+=("$(convert2cygpath ${CMAKE_PATH})/bin")
        generated_path+=("$(convert2cygpath ${NINJA_PATH})")
        generated_path+=("$(convert2cygpath ${LCOV_PATH})/bin")
        generated_path+=("$(convert2cygpath ${DOXYGEN_PATH})/bin")

        if [[ ${_COMPILER} =~ ^mingw.* ]]; then
            generated_path+=("$(convert2cygpath ${MINGW_PATH})/bin")
        fi
        
        generated_path+=("$(convert2cygpath ${LIBXML2_PATH})/bin" "$(convert2cygpath ${LIBXML2_PATH})/lib" "$(convert2cygpath ${LIBXML2_PATH})/include")

        if [[ (${_WORDSIZE} == "x64") && (! ${_COMPILER} =~ ^msvc.*)]]; then
            generated_path+=("$(convert2cygpath ${GTEST_PATH})/lib64")
        else
            generated_path+=("$(convert2cygpath ${GTEST_PATH})/lib")
        fi

        if [[ (${target} == "linux") || (${_COMPILER} == "mingw4.9.2") ]]; then
            generated_ldpath=("$(convert2cygpath ${GTEST_PATH})/lib64")
            generated_ldpath+=("$(convert2cygpath ${LIBXML2_PATH})/bin" "$(convert2cygpath ${LIBXML2_PATH})/lib")
        fi

        if [[ ${target} == "linux" ]]; then
            write_path=$(printf "%s:" "${generated_path[@]}")
            write_ldpath=$(printf "%s:" "${generated_ldpath[@]}")
            cat <<END >${generated_env}
# Generated by mgr.sh - $(date)
export LD_LIBRARY_PATH=${write_ldpath}\${LD_LIBRARY_PATH}
export PATH=${write_path}\${PATH}
export NINJA_PATH=${NINJA_PATH}
END
        elif [[ ${_COMPILER} =~ ^mingw.* ]]; then
            declare -a generated_path_
            for path in "${generated_path[@]}"; do
                generated_path_+=("$(convert2realpath "${path}")")
            done
            write_path=$(printf "%s;" "${generated_path_[@]}")
            cmake_path_="$(convert2realpath "${CMAKE_PATH}")"
            mingw_path_="$(convert2realpath "${MINGW_PATH}")"
            ninja_path_="$(convert2realpath "${NINJA_PATH}")"
            doxygen_path_="$(convert2realpath "${DOXYGEN_PATH}")"
            cat <<END >${generated_env}
:: Generated by mgr.sh - $(date)
set PATH=${write_path}%PATH%
set CMAKE_PATH=$(echo ${cmake_path_})
set MINGW_PATH=$(echo ${mingw_path_})
set NINJA_PATH=$(echo ${ninja_path_})
set DOXYGEN_PATH=$(echo ${doxygen_path_})
END
        else
            declare -a generated_path_
            for path in "${generated_path[@]}"; do
                generated_path_+=("$(convert2realpath "${path}")")
            done
            write_path=$(printf "%s;" "${generated_path_[@]}")
            msvc_path_="$(convert2realpath "${MSVC_PATH}")"
            ninja_path_="$(convert2realpath "${NINJA_PATH}")"
            cat <<END >${generated_env}
:: Generated by mgr.sh - $(date)
set PATH=${write_path}%PATH%
set MSVC_PATH="$(echo ${msvc_path_})"
set MSVC_EXE=${MSVC_EXE}
set MSVC_ARG=${MSVC_ARG}
set MSVC_ARG2=${MSVC_ARG2}
set NINJA_PATH=$(echo ${ninja_path_})
END
        fi
    )
}

###############################################################################
# Clean                                                                       #
###############################################################################
cmdhelp_clean="# Delete ${PRODUCT} 'install' and 'build' directories"
function cmd_clean 
{
    issue_command rm -rf ${install_path} ${build_path}
}

###############################################################################
# Help                                                                        #
###############################################################################
cmdhelp_help="# Print help"
function cmd_help 
{
    print_cmd_list
}

###############################################################################
# Publish                                                                     #
###############################################################################
cmdhelp_publish="# Publish"
function cmd_publish
{
    print_cmd_list
}

###############################################################################
# Tests                                                                       #
###############################################################################
cmdhelp_tests="# Tests
     compiler           Compiler (first default): ${compilers[@]}
     wordsize           Word size (first default): ${wordsizes[@]}"

function cmd_tests {

    _LCOV=1

    # shift $((OPTIND-1))

    retrieve_compiler_and_wordsize ${1} ${2}

    shift 2

    lcov_doc_path=${install_target}/doc/lcov
    lcov_tmp_path=${lcov_doc_path}/tmp
    lcov_init_tmp_filepath=${lcov_tmp_path}/${product}.init.tmp.lcov
    lcov_init_filepath=${lcov_tmp_path}/${product}.init.lcov
    lcov_info_tmp_filepath=${lcov_tmp_path}/${product}.info.tmp.lcov
    lcov_info_filepath=${lcov_tmp_path}/${product}.info.lcov
    lcov_final_filepath=${lcov_tmp_path}/${product}.final.lcov

    if [[ ${_COMPILER} =~ ^msvc.* ]] || [[ ${_COMPILER} =~ ^mingw.* ]]; then
	    #Setup test environment
	    if [[ ! -e ${test_run_filepath} ]]; then
            error "Cannot find setup script (${test_run_filepath})"
        fi
		# Test script
		if [[ ! -e ${test_filepath} ]]; then
            error "Cannot find test script (tests/test.sh)"
        fi

        if [[ ${_COMPILER} =~ ^mingw.* ]] && [[ ! -z ${_LCOV} ]]; then
            . ${test_env_filepath}
            lcov_prerun
        fi
        
        # Launch IOtest
        pushd ${install_target}/tests/bin
        sh ${test_filepath} "$@"
        popd

        if [[ ${_COMPILER} =~ ^mingw.* ]] && [[ ! -z ${_LCOV} ]]; then
            lcov_postrun
        fi
        
        #pushd ${tests_path}
        #cmd "/C ${test_filename} ${@}"
        #popd
    else
        # Setup test environment
        if [[ ! -e ${test_env_filepath} ]]; then
            error "Cannot find setup script (${test_env_filepath})"
        fi
        . ${test_env_filepath}

        # Test script
        if [[ ! -e ${test_filepath} ]]; then
            error "Cannot find test script (tests/test.sh)"
        fi

        if [[ ! -z ${_LCOV} ]]; then
            lcov_prerun
        fi

        # Launch IOtest
        pushd ${install_target}/tests/bin
        sh ${test_filepath} "$@"
        popd

        if [[ ! -z ${_LCOV} ]]; then
            lcov_postrun
        fi
    fi
}

function lcov_prerun()
{
    if [[ -z ${LCOV_EXE} ]]; then
        error "Cannot find code coverage executable"
    fi
    command -v ${LCOV_EXE} >/dev/null 2>&1
    res=$?
    if [[ $res -ne 0 ]]; then
        error "Cannot find code coverage tool ($res)"
    fi
    # Clean
    if [[ -e ${lcov_tmp_path} ]]; then
        issue_command rm -rf ${lcov_tmp_path}
    fi
    issue_command mkdir -p ${lcov_tmp_path}
    # Initialize LCOV
    issue_command ${LCOV_EXE} --zerocounters --directory ${build_target}/tests > /dev/null 2>&1
    issue_command ${LCOV_EXE} --capture --directory ${build_target}/tests --output-file ${lcov_init_tmp_filepath} -initial > /dev/null 2>&1
    if [[ ! -z ${MINGW_PATH} ]]; then
        issue_command ${LCOV_EXE} --remove ${lcov_init_tmp_filepath} "*mingw32/*" "/usr/*" "*tests/api*" "*/gtest*" -o ${lcov_init_filepath} > /dev/null 2>&1
    else
        issue_command ${LCOV_EXE} --remove ${lcov_init_tmp_filepath} '/usr/*' "*tests/api*" "*/gtest*" -o ${lcov_init_filepath} > /dev/null 2>&1
    fi
}

function lcov_postrun()
{
    command -v ${LCOV_EXE} >/dev/null 2>&1
    res=$?
    if [[ $res -ne 0 ]]; then
        error "Cannot find code coverage tool ($res)"
    fi

    # Capture LCOV
    issue_command ${LCOV_EXE} --capture --directory ${build_target}/tests --output-file ${lcov_info_tmp_filepath} > /dev/null 2>&1
    if [[ ! -z ${MINGW_PATH} ]]; then
        issue_command ${LCOV_EXE} --remove ${lcov_info_tmp_filepath} "*mingw32/*" "/usr/*" "*tests/api*" "*/gtest*" -o ${lcov_info_filepath} > /dev/null 2>&1
    else
        issue_command ${LCOV_EXE} --remove ${lcov_info_tmp_filepath} '/usr/*' "*tests/api*" "*/gtest*" -o ${lcov_info_filepath} > /dev/null 2>&1
    fi
    issue_command ${LCOV_EXE} --add-tracefile ${lcov_init_filepath} --add-tracefile ${lcov_info_filepath} -o ${lcov_final_filepath} > /dev/null 2>&1
    # Generate HTML report
    issue_command ${GENHTML_EXE} --prefix ${script_path}/src --prefix ${script_path} --demangle-cpp --num-space 4 --sort --function-coverage --no-branch-coverage --legend --title ${product} -o ${lcov_doc_path} ${lcov_final_filepath}
    # Clean
    issue_command rm -rf ${lcov_tmp_path}
}

###############################################################################
# Test                                                                        #
###############################################################################
cmdhelp_test="# Run a single test in tests directory
     compiler           Compiler (first default): ${compilers[@]}
     wordsize           Word size (first default): ${wordsizes[@]}
     test               The name of the test
     "
function cmd_test {

    while getopts ":db" opt
    do
        case "${opt}" in
            *)
                error "Unknown option [${opt}]"
                ;;
        esac
    done

    shift $((OPTIND-1))

    retrieve_compiler_and_wordsize ${1} ${2}

    shift 2
    
    pushd ${tests_path}
    if [[ ${_COMPILER} =~ ^msvc.* ]] || [[ ${_COMPILER} =~ ^mingw.* ]]; then
        test_exe=$(echo "${1}.exe" | awk '{print tolower($0)}')
        test_sh=$(echo "launch_${1}.sh")
        iotest_script="iotest.sh"
		iotest_path="${tests_path}/${iotest_script}"
        test_launch_filepath="${tests_path}/bin/${test_sh}"
        if [[ ! -f ${test_launch_filepath} ]]; then
            error "Cannot find test launcher (${test_launch_filepath})"
        fi
        pushd bin
        sh ${iotest_path} run -i "${test_launch_filepath}"
        popd

    else
        test_sh=$(echo "launch_${1}.sh")
        test_exe=$(echo "${1}" | awk '{print tolower($0)}')
        test_exe_filepath="${tests_path}/bin/${test_exe}"
        test_launch_filepath="${tests_path}/bin/${test_sh}"
        iotest_script="${tests_path}/iotest.sh"
        if [[ ! -f ${test_launch_filepath} ]]; then
            error "Cannot find test launcher (${test_launch_filepath})"
        fi

        . ${test_env_filepath}
        pushd bin
        #echo "xml:${tests_path}/bin/${test_exe}_output.xml"
        #${test_exe_filepath} --gtest_output="xml:${test_exe}_report.xml"
        sh ${iotest_script} run -i "${test_launch_filepath}"
        popd
    fi
    popd

}

###############################################################################
# Exec                                                                        #
###############################################################################
cmdhelp_exec="# Run a single executable in tests directory
     -d                 Launch with debugger
     -p                 Launch with perf
     compiler           Compiler (first default): ${compilers[@]}
     wordsize           Word size (first default): ${wordsizes[@]}
     test_executable    The name of the test executable
     "
function cmd_exec {

    _DEBUG=0
    _PERF=0

    while getopts ":dp" opt
    do
        case "${opt}" in
            d)
                _DEBUG=1
                ;;
            p)
                _PERF=1
                ;;
            *)
                error "Unknown option [${opt}]"
                ;;
        esac
    done

    shift $((OPTIND-1))

    retrieve_compiler_and_wordsize ${1} ${2}

    shift 2
    
    pushd ${tests_path}
    if [[ ${_COMPILER} =~ ^msvc.* ]]; then
        if [[ ${_DEBUG} == 1 ]]; then
            echo "# Cannot run in debug mode for MSVC"
        fi
        if [[ ${_PERF} == 1 ]]; then
            echo "# Cannot run in perf mode for MSVC"
        fi
        test_exe=$(echo "${1}" | awk '{print tolower($0)}')
        test_exe_filepath="${tests_path}/bin/${test_exe}"
        test_sh=$(echo "launch_${1}.sh")
        iotest_script="iotest.sh"
		iotest_path="${tests_path}/${iotest_script}"
        if [[ ! -f ${test_exe_filepath} ]]; then
            error "Cannot find test launcher (${test_exe_filepath})"
        fi

        pushd bin
        shift 1
        cmd "/C call ..\run_test.bat $test_exe ${@}"
        popd
    else
        test_sh=$(echo "launch_${1}.sh")
        test_exe=$(echo "${1}" | awk '{print tolower($0)}')
        test_exe_filepath="${tests_path}/bin/${test_exe}"
        iotest_script="${tests_path}/iotest.sh"
        if [[ ! -f ${test_exe_filepath} ]]; then
            error "Cannot find test executable (${test_exe_filepath})"
        fi

        . ${test_env_filepath}
        pushd bin
        shift 1
        if [[ ${_DEBUG} -eq 1 ]]; then
            gdb ${test_exe_filepath} ${@}
        elif [[ ${_PERF} -eq 1 ]]; then
            perf record -g -v ${test_exe_filepath} ${@}
        else
            ${test_exe_filepath} ${@}
        fi
        popd
    fi
    popd

}

###############################################################################
# Setup                                                                       #
###############################################################################
cmdhelp_setup="# Copy locally some dependencies and configure the environment to use them
     compiler           Compiler (first default): ${compilers[@]}
     wordsize           Word size (first default): ${wordsizes[@]}
     -n name            Configuration name ('new_dependencies' by default)
     -c                 Clean befor copying dependencies
     -d dependency_file Use a custom dependency file
     "
function cmd_setup {
    
    _DEP="${default_depencies_file}"
    _CLEAN=
    _NAME="new_dependencies"
    
    while getopts ":cd:n:" opt
    do
        case "${opt}" in
            c)
                _CLEAN=1
                echo "# Clean"
                ;;
            d)
                _DEP=$(realpath ${OPTARG})
                echo "# Dependency file [${_DEP}]"
                ;;
            n)
                _NAME=${OPTARG}
                echo "# Name [${_NAME}]"
                ;;
            *)
                error "Unknown option [${opt}]"
                ;;
        esac
    done

    shift $((OPTIND-1))

    retrieve_compiler_and_wordsize ${1} ${2}

    _DEST=$(pwd)/deps/${_NAME}

    mkdir -p ${_DEST}

    echo "Generating dependencies paths configuration files..."
    dep_file="${_DEP}"
    if [[ ! -e ${dep_file} ]]; then
        error "Cannot find dependencies file: ${dep_file}"
    fi

    NEWDEP=${script_path}/${_NAME}.sh

    if [[ (! -e ${NEWDEP}) ]]; then
        cp -f ${_DEP} ${NEWDEP}
    fi

    cat <<END >>${NEWDEP}

# Configuration [${_COMPILER}] [${_WORDSIZE}] generated by mgr.sh - $(date)
END
    
    # Setup dependencies paths and executables
    echo "# Setup dependencies"
    (
        . ${dep_file} &&

        # Copy
        setup_copy CMAKE
        setup_copy NINJA
        setup_copy LCOV
        setup_copy DOXYGEN
        if [[ (${target} == "windows") && (${_COMPILER} =~ ^mingw.*) ]]; then
            setup_copy MINGW
        fi
        setup_copy GTEST
        setup_copy LIBXML2
    )

}

function setup_copy {
    DEST=${_DEST}/${1}/${_COMPILER}/${_WORDSIZE}
    eval SRC='$'${1}'_PATH'
    echo "# Copy ${1} from [${SRC}] to [${DEST}]"
    cat <<END >>${NEWDEP}
# ${1} from [${SRC}] to [${DEST}]
define_path ${1} ${_COMPILER} ${_WORDSIZE} "$(convert2cygpath ${DEST})"
END
    if [[ ! -e ${SRC} ]]; then
        error "Source directory of ${1} does not exist !"
    fi
    if [[ (-e ${DEST}) && (! -z ${_CLEAN}) ]]; then
        echo "# Clean [${DEST}] ..."
        rm -rf ${DEST}
    fi
    if [[ ! -e ${DEST} ]]; then
        echo "# Copying ..."
        mkdir -p ${DEST}
        cp -rf ${SRC}/* ${DEST} || error "Failed while copying ${SRC} to ${DEST}"
        chmod -R 755 ${DEST} || error "Failed while changing rights"
        echo "# Done"
    else
        echo "# No need to copy ${1}"
    fi
}

###############################################################################
# Deliver                                                                     #
###############################################################################

cmdhelp_deliver="# Generate the ePDN"

function cmd_deliver {
    if [[ `uname -s` == "Linux" ]]; then
        sh /home/CrossTools/ePDN/V2.0.0/scripts/ePdn.sh -d ${script_path}/ED247_LIBRARY.epdn -o ${script_path}/install/PDN.html -t frameworks
    fi
}

###############################################################################
# Main                                                                        #
###############################################################################
cmdlist=$(typeset -F | sed 's/declare -f //' | grep "^cmd_" | sed 's/cmd_//' | sort)

command="$1"
shift

if [[ "${command}" == "" ]]; then
    command="help"
fi

command_func=cmd_${command}
typeset -F ${command_func} > /dev/null || error "This command does not exist: ${command}"

#Call the command
${command_func} "$@" || error "Error during execution of command '${command}'"
