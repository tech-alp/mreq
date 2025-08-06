cmake_minimum_required(VERSION 3.16)
project(mreq VERSION 1.0.0 LANGUAGES C  CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include mreq cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(MreqConfig)

# Platform seçimi
if(NOT DEFINED MREQ_PLATFORM_POSIX)
    set(MREQ_PLATFORM_POSIX ON)
    add_compile_definitions(MREQ_PLATFORM_POSIX)
endif()

# Test option
option(MREQ_BUILD_TESTS "Build unit tests" OFF)

# Include dosyalarını bul
file(GLOB INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/mreq/*.hpp")

# nanopb source dosyalarını belirt
set(NANOPB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/external/nanopb/pb_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/external/nanopb/pb_decode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/external/nanopb/pb_encode.c
)

#
# Ana MREQ kütüphanesi
add_library(mreq STATIC ${NANOPB_SOURCES})
set_source_files_properties(${NANOPB_SOURCES} PROPERTIES LANGUAGE C)
set_target_properties(mreq PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(mreq PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/nanopb
    ${MREQ_INCLUDE_DIRS}
)

# Platform-specific libraries
if(MREQ_PLATFORM_POSIX)
    target_link_libraries(mreq PUBLIC pthread)
endif()

# Test build
if(MREQ_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# Örnekler
add_subdirectory(example)
