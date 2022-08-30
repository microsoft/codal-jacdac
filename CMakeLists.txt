project(codal-jacdac)

include("${CODAL_UTILS_LOCATION}")

# find sources and headers
RECURSIVE_FIND_FILE(SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source" "*.c??")

list(APPEND INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/jacdac-c"
    "${CMAKE_CURRENT_SOURCE_DIR}/jacdac-c/inc"
)

# list(APPEND SOURCE_FILES "asm/CortexContextSwitch.s")
include_directories(${INCLUDE_DIRS})
add_library(codal-jacdac ${SOURCE_FILES})

target_link_libraries(
    codal-jacdac
    codal-core
)

target_include_directories(codal-jacdac PUBLIC ${INCLUDE_DIRS})
