# Platform tanımı
add_definitions(-DMREQ_PLATFORM_POSIX)

# Test dosyaları
set(TEST_SOURCES
    test_metadata.cpp
    test_topic.cpp
    test_registry.cpp
    test_subscriber_table.cpp
    test_integration.cpp
)

# Test executable
add_executable(mreq_tests ${TEST_SOURCES})
target_link_libraries(mreq_tests mreq)

# Test'leri çalıştır
add_test(NAME mreq_unit_tests COMMAND mreq_tests) 