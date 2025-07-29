cmake_minimum_required(VERSION 3.16)
project(mreq VERSION 1.0.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include mreq cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(MreqConfig)

message(WARNING ${MREQ_INCLUDE_DIRS})

# Platform seçimi
if(NOT DEFINED MREQ_PLATFORM_POSIX)
    set(MREQ_PLATFORM_POSIX ON)
    add_compile_definitions(MREQ_PLATFORM_POSIX)
endif()

# Test option
option(MREQ_BUILD_TESTS "Build unit tests" OFF)

# Include dosyalarını bul
file(GLOB INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/mreq/*.hpp")

# Ana MREQ kütüphanesi
add_library(mreq STATIC src/mreq.cpp ${INCLUDE_FILES})
target_include_directories(mreq PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
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
