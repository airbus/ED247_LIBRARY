# Introduction

The ED247 LIBRARY / LIBED247 library is an example of implementation of the ED247A communication standard, standardized by the [EUROCAE WG-97][7].

# Operating system and compilers

The ED247 LIBRARY / LIBED247 is natively built with the following operating systems and compilers:

-   _Linux (RHEL7)_
    -   GCC 4.8.5 (32/64 bits)
-   _Windows (10)_
    -   MINGW 4.9.2 (32/64 bits)
    -   Microsoft Visual Studio C++ 2019 (32/64 bits)
    -   Microsoft Visual Studio C++ 2017 (32/64 bits)
    -   Microsoft Visual Studio C++ 2015 (32/64 bits)
-   _Others_
    -   QNX 7.1.0

# Dependencies

The repository contains links to automatically build Libxml2 library from its official repository. Moreover, when tests are enabled, Google Test is also automatically built from its repository.

**CAUTION**: As the source code support C/C++ 11, please make sure you clone the Google Test release `release-1.8.1`, as backward compatibility with C/C++ 11 has been dropped in the next release.

Recommended libraries and associated releases:

|   Library    |         Purpose         | Release |
| :----------: | :---------------------: | :-----: |
| [LIBXML2][1] |        Required         | 2.9.10  |
|  [GTEST][2]  |       Tests only        |  1.8.1  |
|  [LCOV][3]   | Tests only (Linux only) |  1.8.1  |

Recommended libraries and associated releases:

|     Tool     |         Purpose          | Release |
| :----------: | :----------------------: | :-----: |
|  [CMAKE][4]  |  Compilation framework   | 3.13.4  |
|  [NINJA][5]  |       Build system       |  1.8.2  |
| [DOXYGEN][6] | Documentation generation |  1.8.1  |

# Compilation

The library can be compiled with CMake >= 3.13. The first line of the main `CMakeLists.txt`file contains all options for configuration.
The folder `scripts` contains compilation scripts examples for both Linux & Windows environments.

**WARNING**: Do not forget to run `git submodule init` & `git submodule update` while clonning the repository.

**INFO**: In order to compile only the library you can use the scripts with the `_prod_min` suffix.

The following tables list the dependencies for both runtime and compilation phasis. They do not take into account the compilers themselves. These dependencies are listed in the `dependencies.sh` dependencies base file or in the one generated after stand alone setup (see _Development/Installation_).

# Limitations

-   ED247 Packetization strategy mechanism is not implemented.

-   ED247 Error handling is not implemented.

-   The implementation is compliant only with A664, A429, A825, SERIAL, ANALOG, DISCRETE, NAD & VNAD protocols.

-   FrameFormatRevision & StandardRevision options are not entirely supported. This library implements only the "A" revision. Asking for a different revision will lead to a loading error.

-   The regular expressions are not fully supported by gcc4.8.x. Therefore the default linux compilation does not authorize to use complex requests in getters. In particular the use of square brackets has inevitably provoked many failures. Because the behaviour is unspecified the symptoms may vary from a user to an other.

# Miscellaneous

## Logging

The logging strategy can be controlled through the API with `ed247_set_log_level()` or with following environment variables

| Environment Variable |                      Purpose                       |
| :------------------: | :------------------------------------------------: |
|  `ED247_LOG_LEVEL`   | Set the level of logs (see `ed247_log_level_t()`)  |
| `ED247_LOG_FILEPATH` | Set the filepath of the logging file, if necessary |

## License

The license is detailed in the License section.

[1]: https://github.com/GNOME/libxml2
[2]: https://github.com/google/googletest
[3]: https://github.com/linux-test-project/lcov
[4]: https://github.com/Kitware/CMake
[5]: https://github.com/ninja-build/ninja
[6]: https://github.com/doxygen/doxygen
[7]: https://www.eurocae.net/
