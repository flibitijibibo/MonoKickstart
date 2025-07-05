CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(kick C)

if (APPLE)
    SET(CMAKE_OSX_DEPLOYMENT_TARGET 11.0)
    SET(CMAKE_EXECUTABLE_SUFFIX ".bin.osx")
    SET(CMAKE_EXE_LINKER_FLAGS "-framework Foundation -framework GSS")
    SET(BIN_RPATH "@executable_path/osx")
    SET(KICKLIBS
        iconv z
        "-Wl,-force_load,${CMAKE_SOURCE_DIR}/../mono/mono/native/.libs/libmono-native.a"
    )
else()
    SET(CMAKE_EXECUTABLE_SUFFIX ".bin.${CMAKE_SYSTEM_PROCESSOR}")
    SET(KICKLIBS
        m rt dl libz.a
        "-Wl,--whole-archive ${CMAKE_SOURCE_DIR}/../mono/mono/native/.libs/libmono-native.a -Wl,--no-whole-archive"
    )
    SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--disable-new-dtags")

    if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        # Naming quirk for backward compatibility
        SET(BIN_RPATH "\$ORIGIN/lib64")
    else()
        SET(BIN_RPATH "\$ORIGIN/lib${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endif()

ADD_DEFINITIONS(-DENABLE_BINRELOC)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../mono/)

set(CMAKE_SKIP_BUILD_RPATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH ${BIN_RPATH})
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

ADD_EXECUTABLE(kick
    kick.c
    binreloc.c
)

TARGET_LINK_LIBRARIES(kick
    ${CMAKE_SOURCE_DIR}/../mono/mono/mini/.libs/libmonosgen-2.0.a
    ${KICKLIBS}
    pthread
)
