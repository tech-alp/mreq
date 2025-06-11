# Find required programs
find_program(PROTOC_EXECUTABLE protoc)
find_program(NANOPB_PLUGIN protoc-gen-nanopb)

if(NOT PROTOC_EXECUTABLE)
    message(FATAL_ERROR "protoc bulunamadı!")
endif()
if(NOT NANOPB_PLUGIN)
    message(FATAL_ERROR "protoc-gen-nanopb bulunamadı!")
endif()

# Function to generate protobuf files
function(generate_protobuf PROTO_FILES TARGET)
    if(NOT TARGET)
        message(FATAL_ERROR "TARGET parameter is required for generate_protobuf function.")
    endif()

    # Create gen directory in build
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/gen)

    # Generate .pb.h and .pb.c files for each proto file
    foreach(PROTO ${PROTO_FILES})
        get_filename_component(PROTO_WE ${PROTO} NAME_WE)
        get_filename_component(PROTO_DIR ${PROTO} DIRECTORY)
        add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/gen/${PROTO_WE}.pb.h ${CMAKE_BINARY_DIR}/gen/${PROTO_WE}.pb.c
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/gen
            COMMAND ${PROTOC_EXECUTABLE}
                --plugin=protoc-gen-nanopb=${NANOPB_PLUGIN}
                --nanopb_out=${CMAKE_BINARY_DIR}/gen
                --proto_path=${PROTO_DIR}
                ${PROTO_WE}.proto
            DEPENDS ${PROTO}
            WORKING_DIRECTORY ${PROTO_DIR}
            COMMENT "${PROTO} dosyasından nanopb kodu üretiliyor"
        )
        list(APPEND PB_GENERATED ${CMAKE_BINARY_DIR}/gen/${PROTO_WE}.pb.h ${CMAKE_BINARY_DIR}/gen/${PROTO_WE}.pb.c)
    endforeach()

    # Set the generated files to the provided target
    target_sources(${TARGET} PRIVATE ${PB_GENERATED})
    target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}/gen)
endfunction() 