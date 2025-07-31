cmake_minimum_required(VERSION 3.30)
project(mreq_examples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include mreq cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(GenerateTopicRegistry)
include(GenerateProtobuf)

# Proto dosyalarını bul
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto")

# Proto dosyalarından nanopb kodu üret
generate_protobuf_configure_time("${PROTO_FILES}")

# Topic registry üretimi
generate_topic_registry("${PROTO_FILES}")

# Ana MREQ kütüphanesi (zaten ana CMakeLists.txt'de tanımlı)
# Burada sadece link ediyoruz

# Örnekler
add_executable(basic_example basic_example.cpp
    ${CMAKE_BINARY_DIR}/autogen/topic_registry_autogen.cpp ${PROTOBUF_GENERATED_SOURCE_FILES})
target_link_libraries(basic_example PRIVATE mreq)

add_executable(multi_subscriber_example multi_subscriber_example.cpp)
target_include_directories(multi_subscriber_example PRIVATE ${CMAKE_BINARY_DIR}/autogen)
target_link_libraries(multi_subscriber_example mreq pthread)

add_executable(sequential_example sequential_example.cpp)
target_include_directories(sequential_example PRIVATE ${CMAKE_BINARY_DIR}/autogen)
target_link_libraries(sequential_example mreq pthread)

# Add dependencies for all examples
add_dependencies(basic_example generate_topic_registry)
add_dependencies(multi_subscriber_example generate_topic_registry)
add_dependencies(sequential_example generate_topic_registry)

# Set include directories for all examples
target_include_directories(basic_example
    PRIVATE
    ${PROTOBUF_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}/autogen
    ${CMAKE_CURRENT_BINARY_DIR}
    ${MREQ_INCLUDE_DIRS})

target_include_directories(multi_subscriber_example PRIVATE ${CMAKE_BINARY_DIR}/autogen ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(sequential_example PRIVATE ${CMAKE_BINARY_DIR}/autogen ${CMAKE_CURRENT_BINARY_DIR})