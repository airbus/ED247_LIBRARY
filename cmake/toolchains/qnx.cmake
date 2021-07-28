set(CMAKE_SYSTEM_NAME QNX)

set(arch gcc_ntox86_64_gpp)
set(ntoarch x86_64)

set(CMAKE_SIZEOF_VOID_P 8)
set(CMAKE_SYSTEM_PROCESSOR ${ntoarch})

set(CMAKE_C_COMPILER qcc)
set(CMAKE_C_COMPILER_TARGET ${arch})

set(CMAKE_CXX_COMPILER q++)
set(CMAKE_CXX_COMPILER_TARGET ${arch})
	
set(CMAKE_SYSROOT $ENV{QNX_TARGET})

# Add additional paths if required
set(CMAKE_PREFIX_PATH ${CMAKE_SYSROOT}/${ntoarch}/usr)