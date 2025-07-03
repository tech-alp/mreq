# Find required programs
find_program(PROTOC_EXECUTABLE protoc)
find_program(NANOPB_PLUGIN protoc-gen-nanopb)

if(NOT PROTOC_EXECUTABLE)
    message(FATAL_ERROR "protoc bulunamadı!")
endif()
if(NOT NANOPB_PLUGIN)
    message(FATAL_ERROR "protoc-gen-nanopb bulunamadı!")
endif()

# ESP-IDF uyumlu fonksiyon - configure time'da çalışır
function(generate_protobuf_configure_time PROTO_FILES)
    # Create gen directory in build
    set(GEN_DIR "${CMAKE_BINARY_DIR}/autogen")
    file(MAKE_DIRECTORY ${GEN_DIR})
    
    set(GENERATED_FILES)
    
    # Generate .pb.h and .pb.c files for each proto file
    foreach(PROTO ${PROTO_FILES})
        get_filename_component(PROTO_WE ${PROTO} NAME_WE)
        get_filename_component(PROTO_DIR ${PROTO} DIRECTORY)
        
        set(PB_H_FILE ${GEN_DIR}/${PROTO_WE}.pb.h)
        set(PB_C_FILE ${GEN_DIR}/${PROTO_WE}.pb.c)
        
        # Proto dosyasının var olup olmadığını kontrol et
        if(NOT EXISTS ${PROTO})
            message(FATAL_ERROR "Proto file not found: ${PROTO}")
        endif()
        
        message(STATUS "Generating nanopb code from ${PROTO}")
        
        # Proto dosyasını generate et
        execute_process(
            COMMAND ${PROTOC_EXECUTABLE}
                --plugin=protoc-gen-nanopb=${NANOPB_PLUGIN}
                --nanopb_out=${GEN_DIR}
                --proto_path=${PROTO_DIR}
                ${PROTO_WE}.proto
            WORKING_DIRECTORY ${PROTO_DIR}
            RESULT_VARIABLE PROTOC_RESULT
            OUTPUT_VARIABLE PROTOC_OUTPUT
            ERROR_VARIABLE PROTOC_ERROR
        )
        
        if(NOT PROTOC_RESULT EQUAL 0)
            message(FATAL_ERROR "${PROTO} dosyasından nanopb kodu üretilemedi!\nError: ${PROTOC_ERROR}\nOutput: ${PROTOC_OUTPUT}")
        endif()
        
        # Generate edilen dosyaları listeye ekle
        list(APPEND GENERATED_FILES ${PB_H_FILE} ${PB_C_FILE})
        
        # Proto dosyasında değişiklik olduğunda yeniden configure et
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${PROTO})
    endforeach()
    
    # Generated dosyaları parent scope'a döndür
    set(PROTOBUF_GENERATED_FILES ${GENERATED_FILES} PARENT_SCOPE)
    set(PROTOBUF_INCLUDE_DIR ${GEN_DIR} PARENT_SCOPE)
endfunction()

# Orijinal fonksiyon - build time'da çalışır (ESP-IDF dışında kullanım için)
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
        set(PB_H_FILE ${CMAKE_BINARY_DIR}/gen/${PROTO_WE}.pb.h)
        set(PB_C_FILE ${CMAKE_BINARY_DIR}/gen/${PROTO_WE}.pb.c)
        add_custom_command(
            OUTPUT ${PB_H_FILE} ${PB_C_FILE}
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