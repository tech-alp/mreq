cmake_minimum_required(VERSION 3.13)
project(mreq LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# gen/ klasörünü build dizininde oluştur
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/gen)

# Proto dosyaları
set(PROTO_FILES proto/example.proto)

# nanopb ve protoc yolu
find_program(PROTOC_EXECUTABLE protoc)
find_program(NANOPB_PLUGIN protoc-gen-nanopb)

if(NOT PROTOC_EXECUTABLE)
    message(FATAL_ERROR "protoc bulunamadı!")
endif()
if(NOT NANOPB_PLUGIN)
    message(FATAL_ERROR "protoc-gen-nanopb bulunamadı!")
endif()

# .proto'dan .pb.h/.pb.c üretimi (build/gen dizinine)
foreach(PROTO ${PROTO_FILES})
    get_filename_component(PROTO_WE ${PROTO} NAME_WE)
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/gen/${PROTO_WE}.pb.h ${CMAKE_BINARY_DIR}/gen/${PROTO_WE}.pb.c
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/gen
        COMMAND ${PROTOC_EXECUTABLE}
            --plugin=protoc-gen-nanopb=${NANOPB_PLUGIN}
            --nanopb_out=${CMAKE_BINARY_DIR}/gen
            --proto_path=${CMAKE_SOURCE_DIR}/proto
            ${PROTO_WE}.proto
        DEPENDS ${PROTO}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/proto
        COMMENT "${PROTO} dosyasından nanopb kodu üretiliyor"
    )
    list(APPEND PB_GENERATED ${CMAKE_BINARY_DIR}/gen/${PROTO_WE}.pb.h ${CMAKE_BINARY_DIR}/gen/${PROTO_WE}.pb.c)
endforeach()

add_custom_target(generate_pb ALL DEPENDS ${PB_GENERATED})

# Trait ve registry dosyalarını elle yönetiyoruz
set(GEN_TRAITS src/mreq/topic_traits.hpp src/mreq/topic_registry.hpp)

# Kütüphane olarak ekle
add_library(mreq INTERFACE)
target_include_directories(mreq INTERFACE src/mreq ${CMAKE_BINARY_DIR}/gen)
add_dependencies(mreq generate_pb)

# FreeRTOS ve nanopb entegrasyonu için örnek (kendi yolunuza göre düzenleyin)
# target_link_libraries(mreq INTERFACE freertos nanopb)

# Kullanıcıya örnek
add_executable(mreq_example main.cpp)
target_link_libraries(mreq_example PRIVATE mreq)
add_dependencies(mreq_example generate_pb)

target_include_directories(mreq_example PRIVATE /opt/homebrew/include/nanopb ${CMAKE_BINARY_DIR}/gen)
