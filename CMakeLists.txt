cmake_minimum_required(VERSION 3.13)
project(mreq LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(GenerateTopicRegistry)

# Kütüphane olarak ekle
add_library(mreq INTERFACE)
target_include_directories(mreq INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/gen>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/autogen>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/nanopb>
)
add_dependencies(mreq generate_pb generate_topic_registry)

add_subdirectory(example)

# FreeRTOS ve nanopb entegrasyonu için örnek (kendi yolunuza göre düzenleyin)
# target_link_libraries(mreq INTERFACE freertos nanopb)

# Install rules
install(TARGETS mreq
    EXPORT mreq-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MreqConfig.cmake
    DESTINATION lib/cmake/mreq
)

# Export targets
install(EXPORT mreq-targets
    FILE mreq-targets.cmake
    NAMESPACE mreq::
    DESTINATION lib/cmake/mreq
)