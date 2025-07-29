# Mreq library configuration
include(CMakeFindDependencyMacro)

# Set include directories
set(MREQ_INCLUDE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/../include
)

# Set library properties
set(MREQ_LIBRARIES mreq)
set(MREQ_VERSION 1.0.0)

# Export variables
set(MREQ_FOUND TRUE)
set(MREQ_INCLUDE_DIRS ${MREQ_INCLUDE_DIR})
set(MREQ_LIBRARY ${MREQ_LIBRARIES})

list(APPEND MREQ_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/../external/nanopb)

