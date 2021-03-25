# Scripts

The following scripts allow compilation, test and installation of the ED-247 LIBRARY elements. The build is made by default in the `<workspace>/_build` directory and installation in `<workspace>/_install` directory.

**NOTE**: All the scripts are provided as is.

## Windows

This folder contains useful scripts for compilation on Windows, where the environment of compilation shall be wisely set. In order to adapt these scripts to your environment, please have a look at the `win_env.bat <toolchain>` script. It is launched at the very first time of all other scripts.

The first argument `<toolchain>` correspond to one of listed toolchains in `<workspace>/cmake/toolchains` folder with the `windows_` prefix (i.e. `mingw4.9.2_x64` the default one).

The standard order of execution of the scripts is:

-   `win_cmake.bat <toolchain>`: Setup CMake project
-   `win_build.bat <toolchain>`: Build the CMake project.
-   `win_tests.bat <toolchain> <test_name>`: Launch the whole test compaign if `<test_name>`is not set. Otherwise, launched the tests that contains `<test_name>` in their names.
-   `win_install.bat <toolchain>`: Install built elements in the `_install` directory (by default).
-   `win_clean.bat`: Deletes the `_build` & `_install` directories to clean the dev environment.

## Linux

This folder contains useful scripts for compilation on Linux, where the environment of compilation shall be wisely set. In order to adapt these scripts to your environment, please have a look at the `lin_env.sh <toolchain>` script. It is launched at the very first time of all other scripts.

The first argument `<toolchain>` correspond to one of listed toolchains in `<workspace>/cmake/toolchains` folder with the `linux_` prefix (i.e. `gcc4.8.5_x64` the default one).

Here are the Bash scripts:

-   `lin_cmake.sh <toolchain>`: Setup the CMake project.
-   `lin_build.sh <toolchain>`: Build the CMake project.
-   `lin_tests.sh <toolchain> <test_name>`: Launch the whole test compaign if `<test_name>`is not set. Otherwise, launched the tests that contains `<test_name>` in their names.
-   `lin_install.sh <toolchain>`: Install built elements in the `_install` directory (by default).
-   `lin_clean.sh`: Deletes the `_build` & `_install` directories to clean the dev environment.
-   `lin_gen.sh`: A CMake script to generate ECIC configuration files. Please have a look at the several arguments allowing to customize the gneration by running it with the `-h` argument.
-   `lin_prod.sh <toolchain>`: Contains the procedure for a full delivery of the project, including tests and documentation generation.
-   `lin_prod_min.sh <toolchain>`: Contains the procedure for a minimal delivery, meaning only compiling the library and produce binaries in Release mode.
