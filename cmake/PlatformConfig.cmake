# Platform selection options with baremetal as default
option(MREQ_USE_BAREMETAL "Use baremetal implementation" OFF)  # Changed to ON
option(MREQ_USE_FREERTOS "Use FreeRTOS implementation" OFF)
option(MREQ_USE_POSIX "Use POSIX implementation" OFF)

# Validate platform selection
set(PLATFORM_COUNT 0)
if(MREQ_USE_BAREMETAL)
    math(EXPR PLATFORM_COUNT "${PLATFORM_COUNT}+1")
endif()
if(MREQ_USE_FREERTOS)
    math(EXPR PLATFORM_COUNT "${PLATFORM_COUNT}+1")
endif()
if(MREQ_USE_POSIX)
    math(EXPR PLATFORM_COUNT "${PLATFORM_COUNT}+1")
endif()

if(PLATFORM_COUNT EQUAL 0)
    # If no platform selected, default to baremetal
    set(MREQ_USE_BAREMETAL ON CACHE BOOL "Use baremetal implementation" FORCE)
elseif(PLATFORM_COUNT GREATER 1)
    message(FATAL_ERROR "Multiple platforms selected. Please enable only one platform.")
endif()

# Configure platform-specific settings
function(configure_platform TARGET)
    if(MREQ_USE_BAREMETAL)
        target_compile_definitions(${TARGET} INTERFACE MREQ_PLATFORM_BAREMETAL)
        message(STATUS "Using baremetal platform")
    elseif(MREQ_USE_FREERTOS)
        target_compile_definitions(${TARGET} INTERFACE MREQ_PLATFORM_FREERTOS)
        message(STATUS "Using FreeRTOS platform")
    elseif(MREQ_USE_POSIX)
        target_compile_definitions(${TARGET} INTERFACE MREQ_PLATFORM_POSIX)
        message(STATUS "Using POSIX platform")
    endif()
endfunction()