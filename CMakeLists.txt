cmake_minimum_required(VERSION 3.18)
project(mod_video_stream
        VERSION 1.0.0
        DESCRIPTION "video streaming module for FreeSWITCH."
        HOMEPAGE_URL "https://github.com/voxcom-us/mod_video_stream")

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

option(ENABLE_LOCAL "Enable local compile/debug specific" OFF)
if(ENABLE_LOCAL)
    set(ENV{PKG_CONFIG_PATH} "/usr/local/freeswitch/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
endif()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PkgConfig REQUIRED)
find_package(SpeexDSP REQUIRED)

pkg_check_modules(FreeSWITCH REQUIRED IMPORTED_TARGET freeswitch)
pkg_get_variable(FS_MOD_DIR freeswitch modulesdir)
message(STATUS "FreeSWITCH modules dir: ${FS_MOD_DIR}")

add_subdirectory(libs/libwsc)

add_library(mod_video_stream SHARED 
    mod_video_stream.c
    mod_video_stream.h
    video_streamer_glue.h
    video_streamer_glue.cpp
    base64.cpp
)

set_property(TARGET mod_video_stream PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(mod_video_stream PRIVATE 
    PkgConfig::FreeSWITCH 
    pthread
    libwsc
)

if(CMAKE_BUILD_TYPE MATCHES "Release")
    set_target_properties(${PROJECT_NAME} 
        PROPERTIES 
        LINK_FLAGS_RELEASE "-s")
endif()

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libspeexdsp1, openssl, zlib1g, libfreeswitch1")
set(CPACK_PACKAGE_NAME "mod-video-stream")
set(CMAKE_INSTALL_DOCDIR "share/doc/${CPACK_PACKAGE_NAME}")

install(TARGETS ${PROJECT_NAME}
        COMPONENT ${PROJECT_NAME}
        DESTINATION ${FS_MOD_DIR})

message(STATUS "Components to pack: ${CPACK_COMPONENTS_ALL}")

include(Packing)
