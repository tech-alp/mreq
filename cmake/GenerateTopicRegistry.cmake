# Find Python interpreter
find_package(Python3 REQUIRED)

# Function to generate topic registry code
function(generate_topic_registry PROTO_FILES)
    # Create autogen directory
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/autogen)

    # Create semicolon-separated list of proto files
    string(REPLACE " " ";" PROTO_LIST "${PROTO_FILES}")

    # Add custom command to generate topic registry code
    add_custom_command(
        OUTPUT 
            ${CMAKE_BINARY_DIR}/autogen/topic_registry_autogen.hpp
            ${CMAKE_BINARY_DIR}/autogen/topic_registry_autogen.cpp
        COMMAND ${Python3_EXECUTABLE} 
                ${CMAKE_SOURCE_DIR}/scripts/generate_topic_registry.py
                ${PROTO_LIST}
                ${CMAKE_BINARY_DIR}/autogen
        DEPENDS
            ${CMAKE_SOURCE_DIR}/scripts/generate_topic_registry.py
            ${PROTO_FILES}
        COMMENT "Generating topic registry code"
    )

    # Create custom target for topic registry generation
    add_custom_target(generate_topic_registry ALL
        DEPENDS 
            ${CMAKE_BINARY_DIR}/autogen/topic_registry_autogen.hpp
            ${CMAKE_BINARY_DIR}/autogen/topic_registry_autogen.cpp
    )
endfunction()