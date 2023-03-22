CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(kick C)

OPTION(FORCE32 "Force a 32bit compile on 64bit" OFF)
if (APPLE)
    SET(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
elseif(FORCE32)
    SET(CMAKE_C_FLAGS "-m32")
    SET(CMAKE_CXX_FLAGS "-m32")
endif()

if (APPLE)
    SET(BIN_LIBROOT "osx")
    SET(CMAKE_EXECUTABLE_SUFFIX ".bin.osx")
    SET(CMAKE_EXE_LINKER_FLAGS "-framework Foundation -framework GSS")
    SET(BIN_RPATH "@executable_path/osx")
    SET(KICKLIBS
        iconv
        "-Wl,-force_load,${CMAKE_SOURCE_DIR}/../mono/mono/native/.libs/libmono-native-compat.a"
    )
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    SET(BIN_LIBROOT "libaarch64")
    SET(CMAKE_EXECUTABLE_SUFFIX ".bin.aarch64")
    SET(BIN_RPATH "\$ORIGIN/libaarch64")
    SET(KICKLIBS
        m rt dl libz.a
        "-Wl,--whole-archive ${CMAKE_SOURCE_DIR}/../mono/mono/native/.libs/libmoo
no-native.a -Wl,--no-whole-archive"
    )
elseif (CMAKE_SIZEOF_VOID_P MATCHES "8" AND NOT(FORCE32))
    SET(BIN_LIBROOT "lib64")
    SET(CMAKE_EXECUTABLE_SUFFIX ".bin.x86_64")
    SET(BIN_RPATH "\$ORIGIN/lib64")
    SET(KICKLIBS
        m rt dl libz.a
        "-Wl,--whole-archive ${CMAKE_SOURCE_DIR}/../mono/mono/native/.libs/libmono-native.a -Wl,--no-whole-archive"
    )
else()
    SET(BIN_LIBROOT "lib")
    SET(CMAKE_EXECUTABLE_SUFFIX ".bin.x86")
    SET(BIN_RPATH "\$ORIGIN/lib")
    SET(KICKLIBS
        m rt dl libz.a
        "-Wl,--whole-archive ${CMAKE_SOURCE_DIR}/../mono/mono/native/.libs/libmono-native.a -Wl,--no-whole-archive"
    )
endif()

ADD_DEFINITIONS(-DENABLE_BINRELOC)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../mono/)

set(CMAKE_SKIP_BUILD_RPATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH ${BIN_RPATH})
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

SET(SOURCE
    kick.c
    binreloc.c
)

ADD_EXECUTABLE(kick
    ${SOURCE}
)

TARGET_LINK_LIBRARIES(kick
    ${CMAKE_SOURCE_DIR}/../mono/mono/mini/.libs/libmonosgen-2.0.a
    ${KICKLIBS}
    pthread
)
