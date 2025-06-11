cmake_minimum_required(VERSION 3.13)
project(mreq_examples LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include mreq cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(GenerateTopicRegistry)
include(GenerateProtobuf)

# Proto files
set(PROTO_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/sensor.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/sensor_baro.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/sensor_accel.proto
)

# Generate topic registry code
generate_topic_registry("${PROTO_FILES}")

# Example executables
add_executable(basic_example basic_example.cpp)

# Generate protobuf files
generate_protobuf("${PROTO_FILES}" basic_example)

# Link all examples with mreq library
target_link_libraries(basic_example PRIVATE mreq)

add_executable(multi_subscriber_example multi_subscriber_example.cpp)
add_executable(sensor_examples sensor_examples.cpp)
add_executable(sequential_example sequential_example.cpp)

# Link all examples with mreq library
target_link_libraries(multi_subscriber_example PRIVATE mreq)
target_link_libraries(sensor_examples PRIVATE mreq)
target_link_libraries(sequential_example PRIVATE mreq)

# Add dependencies for all examples
add_dependencies(basic_example generate_topic_registry)
add_dependencies(multi_subscriber_example generate_topic_registry)
add_dependencies(sensor_examples generate_topic_registry)
add_dependencies(sequential_example generate_topic_registry)

# Set include directories for all examples
target_include_directories(basic_example PRIVATE ${MREQ_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/autogen)
target_include_directories(multi_subscriber_example PRIVATE ${MREQ_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/autogen)
target_include_directories(sensor_examples PRIVATE ${MREQ_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/autogen)
target_include_directories(sequential_example PRIVATE ${MREQ_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/autogen) 