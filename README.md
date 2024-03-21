# Introduction

The ED247 LIBRARY is an example of implementation of the ED247A communication standard, standardized by the [EUROCAE WG-97][7].

Doxygen documnetation is available here: https://airbus.github.io/ED247_LIBRARY/

## License

Refer to [LICENCE](./LICENSE.md) file.


# Limitations

-   ED247 Packetization strategy mechanism is not implemented.

-   ED247 Error handling is not implemented.

-   The implementation is compliant only with A664, A429, A825, SERIAL, ETH, ANALOG, DISCRETE, NAD & VNAD protocols.


# Operating system and compilers

The ED247 LIBRARY / LIBED247 can be built for the following operating systems and compilers:

-   Linux 32 bits (RHEL7)
-   Linux 64 bits (RHEL7)
-   Windows 10 MinGW 32 bits
-   Windows 10 MinGW 64 bits

# Dependencies
 ## Runtime
|   Library    |         Purpose         | Release |
| :----------: | :---------------------: | :-----: |
| [LIBXML2][1] |        Required         |  2.9.1  |

## Building
|     Tool     |         Purpose          | Release  |
| :----------: | :----------------------: | :------: |
|  [CMAKE][4]  |  Compilation framework   | 3.22.0   |
|  [GTEST][2]  |       Tests only         | 1.10.0   |
| [DOXYGEN][6] | Documentation generation | 1.8.11   |

## Logging

The logging strategy can be controlled through the API with `ed247_set_log_level()` or with following environment variables:

| Environment Variable |                      Purpose                       |
| :------------------: | :------------------------------------------------: |
|  `ED247_LOG_LEVEL`   | Set the level of logs (see `ed247_log_level_t()`)  |
| `ED247_LOG_FILEPATH` | Set the filepath of the logging file, if necessary |

# Compilation

## Useful targets

|   target    |         Purpose        |
| :---------- | :--------------------- |
| all | Build everything. This is the default target. |
| ed247 | Only build the library. |
| utils | Build utils and their dependencies. |
| tests | Build all tests and theirs dependencies. |
| run_tests | Build all tests and excute them. |
| <test_name> | Build only test <test_name>. See tests section below.|
| doc | Build documentation. |
| install | Delivery. |

## Handle version

The target version shall be specified in CMakeLists.txt on project() line. It will be used for Product Delivery Note.<br />
The real version will be computed according to git tag description. This value will be hard-coded in the binary in order to to really know where it come from.<br />
**IMPORTANT: You shall re-run CMake and rebuild before delivering to update the version, especially if you have create a git tag.**

The version format is:
```
<last tag>[-<commit info>][-dirty]
```

So, if you just have create a tag, the version will be exactly the tag name.<br />
Else, the version will contain information to know from which commit the binary come from.

## Tests debugging

Test use multicast IPs and you may need to create a specific route to make them working:
```
sudo route add -net 224.0.0.0 netmask 240.0.0.0 dev lo
```


The tests are the subfolders of `tests/` folder. A single test can be rebuild by calling the target `<test_name>`.

A single test can be ran with:
```
cd <build_dir>
ctest -R '^<test_name>$' -VV
```

Since most tests needs two binaries (client/server) and each one have its own output, there is no "test output". The output of each binary is available in `tests/<test_name>/<binary_name>.log`. <br/>
When a test binary fail, the test will display the path to its log file.

You also can run yourself the two binaries of the test in two separate consoles. You will have their output directly in the consoles. This is the only way to use specific tools like gdb or valgrind.


## Build with CMake (quick CMake help)

First, configure your environment:
```
mkdir my_build
cd my_build
cmake <GIT_ROOT> [cmake options]
```
To force the use of Ninja (it has to be in your path):
```
cmake <GIT_ROOT> -G "Ninja" [cmake options]
```
Cmake options can be provided with the syntax `-D<option>=<value>`

|   CMake option  |         Purpose        |
| :-------------- | :--------------------- |
| CMAKE_BUILD_TYPE | The default is RelWithDebInfo. You may want to use Debug to disable optimization. |
| CMAKE_TOOLCHAIN_FILE | Needed to build in 32-bits or cross-compile. See examples in `cmake/toolchains`. |
| CMAKE_PREFIX_PATH | List of paths to search for dependencies. |
| GTest_ROOT | Path to GTest. |
| Doxygen_ROOT | Path to Doxygen. |
| CMAKE_INSTALL_PREFIX | install path. (Can also be set with option --prefix. See install below.) |


Then, run the target you want (see Useful targets upside):
```
cmake --build <build_root> --target <target>
```

Run tests and install (use CMAKE_INSTALL_PREFIX or overwrite with --prefix):
```
cmake --build <build_root> --target run_tests
cmake --install <build_root> [--prefix /where/to/install ]
```

<!-- Links -->
[1]: https://github.com/GNOME/libxml2
[2]: https://github.com/google/googletest
[4]: https://github.com/Kitware/CMake
[6]: https://github.com/doxygen/doxygen
[7]: https://www.eurocae.net/
