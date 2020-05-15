# Introduction

The ED247 LIBRARY / LIBED247 library is an example of implementation of the ED247A communication standard, standardized by the [EUROCAE WG-97][9].

[9]: https://www.eurocae.net/

## Operating system and compilers

The ED247 LIBRARY / LIBED247 is natively built with the following operating systems and compilers:
* *Linux (RHEL7)*
  * GCC 4.8.5 (`gcc4.8.5`)
    * 32 bits (`x86`)
    * 64 bits (`x64`)
* *Windows (10)*
  * MINGW 4.9.2 (`mingw4.9.2`)
    * 32 bits (`x86`)
    * 64 bits (`x64`)
  * Microsoft Visual Studio C++ 2019 (`msvc2019`)
    * 32 bits (`x86`)
    * 64 bits (`x64`)
  * Microsoft Visual Studio C++ 2017 (`msvc2017`)
    * 32 bits (`x86`)
    * 64 bits (`x64`)
  * Microsoft Visual Studio C++ 2015 (`msvc2015`)
    * 32 bits (`x86`)
    * 64 bits (`x64`)

## Dependencies

The following tables list the dependencies for both runtime and compilation phasis. They do not take into account the compilers themselves. These dependencies are listed in the `dependencies.sh` dependencies base file or in the one generated after stand alone setup (see *Development/Installation*).

### Runtime

#### Core

| Dependency       | Version |
|:----------------:|:-------:|
| [LIBXML2][1]     | 2.9.9   |

[1]: https://github.com/GNOME/libxml2

#### Tests

| Dependency       | Version |
|:----------------:|:-------:|
| [GTEST][2]       | 1.8.1   |

[2]: https://github.com/google/googletest

### Compilation

*In order to compile on Windows, it is mandatory to use a Cygwin terminal, such as [GitBash][9].*

[9]: https://gitforwindows.org/

#### Core

| Dependency       | Version |
|:----------------:|:-------:|
| [CMAKE][3]       | 3.13.4  |
| [NINJA][4]       | 1.8.2   |

[3]: https://github.com/Kitware/CMake
[4]: https://github.com/ninja-build/ninja

#### Tests

| Dependency       | Version |
|:----------------:|:-------:|
| [LCOV][6]        | 1.13.0  |

[6]: https://github.com/linux-test-project/lcov

#### Documentation

| Dependency       | Version |
|:----------------:|:-------:|
| [DOXYGEN][8]     | 1.8.11  |

[8]: https://github.com/doxygen/doxygen

### Setup

The developper must update the `dependencies.sh` file with the right location of dependencies, taking into account OS type, compiler & wordsize as mentionned in each line.

## Development

All the installation, build and test process is made using the `mgr.sh` script. More help with `mgr.sh help`.

### Installation

The developer can perform a stand alone copy of almost all dependencies with the `mgr.sh setup` command.
For example `mgr.sh setup mingw4.9.2 x64` will copy main dependencies in a *deps/new* directory where *new* stands for the name of the dependency configuration. It can by customized with the `-n` command line option. After the copy, this command will also generate a *new.sh* configuration which is a duplicate of the *dependencies.sh* file with links to the newly copied depedencies at the end.

### Build

In order to build the ED247 LIBRARY, the developer shall use `mgr.sh build *compiler* *wordsize*` where `*compiler*` and `*wordsize*` are listed in the section *Operating system and compilers* or in the displayed help `mgr.sh help`. Additionally, the developer can use the `-d` option to use a local setup as detailed in the previous section. More help with `mgr.sh help`.

### Tests

The test environment of the ED247 LIBRARY is internally based on a combination of the iotest.sh script (for functional tests) and GTest (for unit tests). The LCOV code coverage tool is used to map the code executable by those ones. The tests run are independant from the building infrastructure, meaning they can be run outside of the compilation environment.

#### Test suite

The test suite can be run with the following `mgr.sh tests *compiler* *wordsize*` where `*compiler*` and `*wordsize*` are listed in the section *Operating system and compilers* or in the displayed help `mgr.sh help`. More help with `mgr.sh help`.

#### Single run

In addition to the test suite, it is possible to run a single test executable with the command line `mgr.sh test *compiler* *wordsize* *test_executable*` where `*compiler*` and `*wordsize*` are listed in the section *Operating system and compilers* or in the displayed help `mgr.sh help` and the `*test_executable*` specifies the test to run (without any suffix such as *.exe*).

## Limitations

- ED247 Packetization strategy mechanism is not implemented.

- ED247 Error handling is not implemented.

- The implementation is compliant only with A664, A429, A825, SERIAL, ANALOG, DISCRETE, NAD & VNAD protocols.

- FrameFormatRevision & StandardRevision options are not entirely supported. This library implements only the "A" revision. Asking for a different revision will lead to a loading error.

- The regular expressions are not fully supported by gcc4.8.x. Therefore the default linux compilation does not authorize to use complex requests in getters. In particular the use of square brackets has inevitably provoked many failures. Because the behaviour is unspecified the symptoms may vary from a user to an other.

## Miscellaneous

### Logging

The logging strategy can be controlled through the API with `ed247_set_log_level()` or with following environment variables

| Variable            | Purpose                                            |
|:-------------------:|:--------------------------------------------------:|
|`ED247_LOG_LEVEL`    | Set the level of logs (see `ed247_log_level_t()`)  |
|`ED247_LOG_FILEPATH` | Set the filepath of the logging file, if necessary |

## License

The license is detailed in the License section.

## Test between two computers

The test FT_PERFOS can be adapted to be launched on two separate computers (with different IP addresses).
Indeed, the FT_PERFOS test contains a simple ECIC file generator that can be used to generate ECIC configuration files using the `./install/linux/gcc4.8.5/x86/tests/bin/prepare_ft_perfos.sh` script (run `sh ./install/linux/gcc4.8.5/x86/tests/bin/prepare_ft_perfos.sh -h` for more info about the arguments). Example of use: `sh ./install/linux/gcc4.8.5/x86/tests/bin/prepare_ft_perfos.sh 10000 1 1 1 1 192.169.0.1 192.169.0.2` (`192.169.0.1` = Master IP address & `192.169.0.2` = Slave IP address).

Next, in order to run the executables, use `sh ./install/linux/gcc4.8.5/x86/tests/bin/run_ft_perfos_single.sh [master/slave] [remote IP address]` on each computer (for example `sh ./install/linux/gcc4.8.5/x86/tests/bin/run_ft_perfos_single.sh master 192.169.0.2` on the computer with IP address `192.169.0.1` which is considered as the `master`).

