###############################################################################
# Setup the paths of dependencies and executable names
###############################################################################

# The paths and executables must be defined using 'define_path' or 'define_exe' predefined functions.
# define_path or define_exe signatures are:
# define_path/define_exe NAME VALUE
#       Setup the path/executable without regard to OS type or compiler.
# define_path/define_exe NAME OSTYPE VALUE
#       Setup the path/executable according to the OS type.
# define_path/define_exe NAME COMPILER WORDSIZE VALUE
#       Setup the path/executable according to the compiler and wordsize. For available compilers and wordsizes, please refer to 'sh mgr.sh help build'.

###############################################################################
# Common
###############################################################################

# CMAKE
define_path CMAKE       linux   "To be filled with CMake (V3.13.4) root path"
define_path CMAKE       windows "To be filled with CMake (V3.13.4) root path"
define_exe  CMAKE   cmake

# NINJA
define_path NINJA       linux   "To be filled with Ninja (V1.8.2) root path"
define_path NINJA       windows "To be filled with Ninja (V1.8.2) root path"
define_exe  NINJA       linux   ninja
define_exe  NINJA       windows ninja.exe

# LCOV
define_path LCOV        linux   "To be filled with Lcov (V1.13.0) root path"
define_path LCOV        windows "To be filled with Lcov (V1.13.0) root path"
define_exe  LCOV    lcov
define_exe  GENHTML genhtml

# DOXYGEN
define_path DOXYGEN     linux   "To be filled with Doxygen (V1.8.11) root path"
define_path DOXYGEN     windows "To be filled with Doxygen (V1.8.11) root path"

###############################################################################
# Compiler dependent
###############################################################################

# MINGW
define_path MINGW   mingw4.9.2  x64 "To be filled with MinGW (V4.9.2) root path"
define_path MINGW   mingw4.9.2  x86 "To be filled with MinGW (V4.9.2) root path"

# MSVC
define_path MSVC    msvc2017    x64 "To be filled with MSVC (V15.9.9) root path"
define_path MSVC    msvc2017    x86 "To be filled with MSVC (V15.9.9) root path"
define_exe  MSVC    msvc2017    x64 "vcvars64.bat"
define_exe  MSVC    msvc2017    x86 "vcvarsamd64_x86.bat"

# GTEST
define_path GTEST   gcc4.8.5    x64 "To be filled with GTest (V1.8.1) root path"
define_path GTEST   gcc4.8.5    x86 "To be filled with GTest (V1.8.1) root path"
define_path GTEST   mingw4.9.2  x64 "To be filled with GTest (V1.8.1) root path"
define_path GTEST   mingw4.9.2  x86 "To be filled with GTest (V1.8.1) root path"
define_path GTEST   msvc2017    x64 "To be filled with GTest (V1.8.1) root path"
define_path GTEST   msvc2017    x86 "To be filled with GTest (V1.8.1) root path"

# LIBXML2
define_path LIBXML2 gcc4.8.5    x64 "To be filled with LIBXML2 (V2.9.9) root path"
define_path LIBXML2 gcc4.8.5    x86 "To be filled with LIBXML2 (V2.9.9) root path"
define_path LIBXML2 mingw4.9.2  x64 "To be filled with LIBXML2 (V2.9.9) root path"
define_path LIBXML2 mingw4.9.2  x86 "To be filled with LIBXML2 (V2.9.9) root path"
define_path LIBXML2 msvc2017    x64 "To be filled with LIBXML2 (V2.9.9) root path"
define_path LIBXML2 msvc2017    x86 "To be filled with LIBXML2 (V2.9.9) root path"
define_exe  XMLLINT linux   xmllint
define_exe  XMLLINT windows xmllint.exe
