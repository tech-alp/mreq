# Function to generate topic registry code at configure time (ESP için)
function(generate_topic_registry_configure_time PROTO_FILES OUTPUT_DIR)
    # Execute Python script immediately (configure-time)
    execute_process(
        COMMAND ${Python3_EXECUTABLE}
            ${COMPONENT_DIR}/mreq/scripts/generate_topic_registry.py
            ${PROTO_FILES}
            ${OUTPUT_DIR}
        RESULT_VARIABLE RESULT
        OUTPUT_VARIABLE OUTPUT
        ERROR_VARIABLE ERROR
    )
    if(RESULT EQUAL 0)
        message(STATUS "Topic registry generated successfully at configure time")
    else()
        message(FATAL_ERROR "Failed to generate topic registry at configure time:\n${ERROR}")
    endif()
endfunction()

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
            ${CMAKE_BINARY_DIR}/autogen/topic_registry_autogen.cpp
    )
endfunction()