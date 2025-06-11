# Mreq library configuration
include(CMakeFindDependencyMacro)

# Set include directories
set(MREQ_INCLUDE_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/../include
)

# Set library properties
set(MREQ_LIBRARIES mreq)
set(MREQ_VERSION 1.0.0)

# Export variables
set(MREQ_FOUND TRUE)
set(MREQ_INCLUDE_DIR ${MREQ_INCLUDE_DIRS})
set(MREQ_LIBRARY ${MREQ_LIBRARIES})

# Include nanopb if available
if(APPLE)
    find_path(NANOPB_INCLUDE_DIR nanopb.h
        PATHS /opt/homebrew/include/nanopb
        NO_DEFAULT_PATH
    )
else()
    find_path(NANOPB_INCLUDE_DIR nanopb.h
        PATHS ${CMAKE_CURRENT_LIST_DIR}/../external/nanopb
        NO_DEFAULT_PATH
    )
endif()

if(NANOPB_INCLUDE_DIR)
    list(APPEND MREQ_INCLUDE_DIRS ${NANOPB_INCLUDE_DIR})
endif() 