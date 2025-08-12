# GoogleTest'i dahil et
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Platform tanımı
add_definitions(-DMREQ_PLATFORM_POSIX)

# Test ana dosyası
set(TEST_MAIN_FILE
    test_main.cpp
)

# Test kaynak dosyalarını otomatik olarak bul
file(GLOB TEST_FILES "*.cpp")

# Test ana dosyasını listeden çıkar (çünkü ayrı ekleyeceğiz)
list(REMOVE_ITEM TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp")

# Test executable
add_executable(mreq_tests ${TEST_MAIN_FILE} ${TEST_FILES})

# Kütüphaneleri bağla
target_link_libraries(mreq_tests mreq GTest::gtest_main)

# Testleri dahil et
include(GoogleTest)
gtest_discover_tests(mreq_tests)